#include "pass/optimize/optimize.hpp"
#include "pass/optimize/licm.hpp"
#include "ir/value.hpp"
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>

namespace pass {
ir::Value* LICMContext::getIntToPtrBaseAddr(ir::UnaryInst* inst) {
  if (auto binary = inst->value()->dynCast<ir::BinaryInst>()) {
    if (binary->lValue()->valueId() == ir::ValueId::vPTRTOINT) {
      return binary->lValue()->dynCast<ir::UnaryInst>()->value();
    } else if (binary->rValue()->valueId() == ir::ValueId::vPTRTOINT) {
      return binary->rValue()->dynCast<ir::UnaryInst>()->value();
    }
  }
  assert(false);
  return nullptr;
}

ir::Value* LICMContext::getbase(ir::Value* subAddr) {
  if (auto allocainst = subAddr->dynCast<ir::AllocaInst>()) return allocainst;
  if (auto gv = subAddr->dynCast<ir::GlobalVariable>()) return gv;
  if (auto arg = subAddr->dynCast<ir::Argument>()) return arg;
  if (auto gep = subAddr->dynCast<ir::GetElementPtrInst>()) return getbase(gep->value());
  if (auto phi = subAddr->dynCast<ir::PhiInst>()) {
    auto func = phi->block()->function();
    auto lpctx = tpm->getLoopInfoWithoutRefresh(func);
    auto lp = lpctx->head2loop(phi->block());
    auto preHeaderVal = phi->getvalfromBB(lp->getLoopPreheader());
    return getbase(preHeaderVal);
  }
  if (auto unary = subAddr->dynCast<ir::UnaryInst>()) {
    if (unary->valueId() == ir::ValueId::vINTTOPTR) {
      return getIntToPtrBaseAddr(unary);
    }
  }
  assert("Error! invalid type of input in function getBaseAddr!" && false);
  return nullptr;
}

ir::Value* getsubaddr(ir::Instruction* inst) {
  if (inst->dynCast<ir::StoreInst>())
    return inst->dynCast<ir::StoreInst>()->ptr();
  else if (inst->dynCast<ir::LoadInst>())
    return inst->dynCast<ir::LoadInst>()->ptr();
  else
    return nullptr;
}

bool LICMContext::alias(ir::Instruction* inst0, ir::Instruction* inst1) {
  if (inst0->dynCast<ir::CallInst>()) {
    return true;
  }
  if (inst0->dynCast<ir::LoadInst>() || inst0->dynCast<ir::StoreInst>()) {
    auto instbase = getbase(getsubaddr(inst0));
    auto storebase = getbase(getsubaddr(inst1));
    if (instbase == storebase) return true;
    return false;
  } else {
    return false;
  }
}

bool LICMContext::isinvariantop(ir::Instruction* inst, ir::Loop* loop) {
  for (auto op : inst->operands()) {
    if (auto opinst = op->value()->dynCast<ir::Instruction>()) {
      if (loop->contains(opinst->block())) return false;
    }
  }
  return true;
}

bool LICMContext::iswrite(ir::Value* ptr, ir::CallInst* callinst) {
  auto baseaddr = getbase(ptr);
  auto callee = callinst->callee();
  for (auto gv : sectx->funcWriteGlobals(callee)) {
    if (baseaddr == gv) return true;
  }

  for (auto rarg : callinst->rargs()) {
    if (baseaddr == rarg->value()) {
      auto para = callee->arg_i(rarg->index());
      if (sectx->getArgWrite(para)) return true;
    }
  }
  return false;
}

bool LICMContext::isread(ir::Value* ptr, ir::CallInst* callinst) {
  auto baseaddr = getbase(ptr);
  auto callee = callinst->callee();
  for (auto gv : sectx->funcReadGlobals(callee)) {
    if (baseaddr == gv) return true;
  }

  for (auto rarg : callinst->rargs()) {
    if (baseaddr == rarg->value()) {
      auto para = callee->arg_i(rarg->index());
      if (sectx->getArgRead(para)) return true;
    }
  }
  return false;
}

void LICMContext::collectStorePtrs(ir::CallInst* call, ir::Loop* loop) {
  loopStorePtrs.clear();
  for (auto bb : loop->blocks()) {
    for (auto inst : bb->insts()) {
      if (inst->dynCast<ir::StoreInst>()) {
        auto ptr = getbase(getsubaddr(inst));
        loopStorePtrs.insert(ptr);
      } else if (inst->dynCast<ir::CallInst>()) {
        auto callinst = inst->dynCast<ir::CallInst>();
        if (call == callinst) continue;
        auto callee = callinst->callee();
        for (auto larg : sectx->funcArgSet(callee)) {
          auto rarg = callinst->rargs()[larg->index()]->value();
          if (sectx->getArgWrite(larg)) {
            auto ptr = getbase(rarg);
            loopStorePtrs.insert(ptr);
          }
        }
        for (auto gv : sectx->funcWriteGlobals(callee)) {
          loopStorePtrs.insert(gv);
        }
      }
    }
  }
}

bool LICMContext::isinvariantcall(ir::CallInst* callinst, ir::Loop* loop) {
  if (!isinvariantop(callinst, loop)) return false;

  collectStorePtrs(callinst, loop);
  auto callee = callinst->callee();
  for (auto larg : sectx->funcArgSet(callee)) {
    auto rarg = callinst->rargs()[larg->index()]->value();
    auto ptr = getbase(rarg);
    if (loopStorePtrs.count(ptr)) {
      if (sectx->getArgRead(larg)) return false;
    }
  }
  return true;
}

bool LICMContext::checkload(ir::StoreInst* storeinst, ir::Loop* loop) {
  auto ptr = getbase(getsubaddr(storeinst));
  for (auto bb : loop->blocks()) {
    for (auto inst : bb->insts()) {
      if (inst->dynCast<ir::LoadInst>()) {
        if (alias(inst, storeinst)) {
          auto storebb = storeinst->block();
          auto loadbb = inst->block();
          if (storebb != loadbb) {
            if (domctx->dominate(loadbb, storebb)) return false;
          } else {
            auto storeit = std::find(storebb->insts().begin(), storebb->insts().end(), storeinst);
            auto loadit = std::find(loadbb->insts().begin(), loadbb->insts().end(), inst);
            int storeidx = std::distance(storebb->insts().begin(), storeit);
            int loadidx = std::distance(loadbb->insts().begin(), loadit);
            if (storeidx > loadidx) return false;
          }
        }
      } else if (inst->dynCast<ir::CallInst>()) {
        auto callee = inst->dynCast<ir::CallInst>()->callee();
        if (sectx->getPotentialSideEffect(callee))
          return false;
        else {
          // if (isread(ptr, inst->dynCast<ir::CallInst>())) {
          //     return false;
          // }
          if (sectx->hasSideEffect(callee)) {
            return false;
          }
        }
      }
    }
  }
  return true;
}

bool LICMContext::checkstore(ir::LoadInst* loadinst, ir::Loop* loop) {
  auto ptr = getbase(getsubaddr(loadinst));
  for (auto bb : loop->blocks()) {
    for (auto inst : bb->insts()) {
      if (inst->dynCast<ir::StoreInst>()) {
        if (alias(inst, loadinst)) return false;
      } else if (inst->dynCast<ir::CallInst>()) {
        auto callee = inst->dynCast<ir::CallInst>()->callee();
        if (sectx->getPotentialSideEffect(callee))
          return false;
        else {
          // if (iswrite(ptr, inst->dynCast<ir::CallInst>())) {
          //     return false;
          // }
          if (sectx->hasSideEffect(callee)) {
            return false;
          }
        }
      }
    }
  }
  return true;
}

std::vector<ir::Instruction*> LICMContext::getinvariant(ir::BasicBlock* bb, ir::Loop* loop) {
  std::vector<ir::Instruction*> res;
  auto head = loop->header();
  ir::BasicBlock* headnext;
  for (auto ibb : head->next_blocks()) {
    if (loop->contains(ibb)) {
      headnext = ibb;
      break;
    }
  }
  if (bb != head && !pdomcctx->pdominate(bb, headnext)) return res;
  for (auto inst : bb->insts()) {
    if (auto storeinst = inst->dynCast<ir::StoreInst>()) {
      if (isinvariantop(storeinst, loop))
        if (checkload(storeinst, loop)) {  // 接下来检查循环体里有无对本数组的load
          res.push_back(storeinst);
          // std::cerr << "lift store" << std::endl;
        }

    } else if (auto loadinst = inst->dynCast<ir::LoadInst>()) {
      if (isinvariantop(loadinst, loop))
        if (checkstore(loadinst, loop)) {  // 接下来检查循环体里有无对本数组的store
          res.push_back(loadinst);
          // std::cerr << "lift load" << std::endl;
        }
    } else if (auto callinst = inst->dynCast<ir::CallInst>()) {
      auto callee = callinst->callee();
      if (sectx->isInputOnlyFunc(callee)) {
        if (isinvariantcall(callinst, loop)) {
          res.push_back(callinst);
          // std::cerr << "lift call" << std::endl;
        }
      }
    } else if (auto UnaryInst = inst->dynCast<ir::UnaryInst>()) {
      if (isinvariantop(UnaryInst, loop)) {
        res.push_back(UnaryInst);
        // std::cerr << "lift Unary" << std::endl;
      }
    } else if (auto BinaryInst = inst->dynCast<ir::BinaryInst>()) {
      if (isinvariantop(BinaryInst, loop)) {
        if (BinaryInst->valueId() != ir::vSDIV && BinaryInst->valueId() != ir::vSREM &&
            BinaryInst->valueId() != ir::vFDIV && BinaryInst->valueId() != ir::vFREM) {
          res.push_back(BinaryInst);
          // std::cerr << "lift Binary" << std::endl;
        }
      }
    } else if (auto GepInst = inst->dynCast<ir::GetElementPtrInst>()) {
      if (isinvariantop(GepInst, loop)) {
        res.push_back(GepInst);
        // std::cerr << "lift Gep" << std::endl;
      }
    }
  }
  return res;
}

void LICMContext::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  // std::cout<<"In Function "<<func->name()<<": "<<std::endl;
  loopctx = tp->getLoopInfo(func);
  loopctx->refresh();
  domctx = tp->getDomTree(func);
  domctx->refresh();
  pdomcctx = tp->getPDomTree(func);
  pdomcctx->refresh();
  sectx = tp->getSideEffectInfo();
  sectx->setOff();
  sectx->refresh();
  tpm = tp;
  for (auto loop : loopctx->loops()) {
    auto preheader = loop->getLoopPreheader();
    bool changed = true;
    while (changed) {
      std::vector<ir::Instruction*> inststoremove;
      for (auto bb : loop->blocks()) {
        std::vector<ir::Instruction*> inststoremoveinbb;
        inststoremoveinbb = getinvariant(bb, loop);
        inststoremove.insert(inststoremove.end(), inststoremoveinbb.begin(),
                             inststoremoveinbb.end());
      }
      changed = (inststoremove.size() > 0);
      for (auto inst : inststoremove) {
        auto instbb = inst->block();
        instbb->move_inst(inst);
        preheader->emplace_lastbutone_inst(inst);
      }
    }
  }
}
void LICM::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  LICMContext licmctx;
  licmctx.run(func, tp);
}
}  // namespace pass