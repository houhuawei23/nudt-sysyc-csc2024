#include "pass/optimize/ADCE.hpp"

static std::queue<ir::Instruction*> workList;
static std::unordered_map<ir::BasicBlock*, bool> liveBB;
static std::unordered_map<ir::Instruction*, bool> liveInst;

namespace pass {
void ADCE::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  ADCEContext ctx;
  ctx.run(func, tp);
}

void ADCEContext::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  bool isCFGChange = false;
  bool isCallGraphChange = false;
  // func->print(std::cerr);
  if (func->isOnlyDeclare()) return;
  // std::cerr << "ADCEContext running on function " << func->name() << std::endl;

  pdctx = tp->getPDomTree(func);
  pdctx->setOff();
  pdctx->refresh();
  liveBB.clear();
  liveInst.clear();
  auto sectx = tp->getSideEffectInfo();

  assert(workList.empty() and "ADCEContext WorkList not empty before running!");

  // 初始化所有的inst和BB的live信息
  for (auto bb : func->blocks()) {
    assert(bb != nullptr);
    liveBB[bb] = false;
    for (auto inst : bb->insts()) {
      liveInst[inst] = false;
      if (inst->hasSideEffect()) {
        workList.push(inst);
      }
      if (auto callInst = inst->dynCast<ir::CallInst>()) {
        if (sectx->hasSideEffect(callInst->callee())) {
          workList.push(inst);
        }
      }
    }
  }
  // std::cerr << "ADCEContext worklist size: " << workList.size() << std::endl;
  // 工作表算法
  while (not workList.empty()) {
    // std::cerr << "ADCEContext worklist size: " << workList.size() << std::endl;
    auto curInst = workList.front();
    auto curBB = curInst->block();
    assert(curInst != nullptr and curBB != nullptr);
    workList.pop();
    if (liveInst[curInst]) continue;
    // 设置当前的inst为活, 以及其块
    liveInst[curInst] = true;
    liveBB[curBB] = true;
    auto curInstPhi = dyn_cast<ir::PhiInst>(curInst);
    // 如果是phi,就要将其所有前驱BB的terminal置为活
    if (curInstPhi) {
      for (int idx = 0; idx < curInstPhi->getsize(); idx++) {
        auto phibb = curInstPhi->getBlock(idx);
        auto phibbTerminator = phibb->terminator();
        if (phibbTerminator and not liveInst[phibbTerminator]) {
          workList.push(phibbTerminator);
          assert(phibb != nullptr and phibbTerminator != nullptr);
          liveBB[phibb] = true;
        }
      }
    }
    for (auto cdgpredBB : pdctx->pdomfrontier(curBB)) {  // curBB->pdomFrontier
      auto cdgpredBBTerminator = cdgpredBB->terminator();
      if (cdgpredBBTerminator and not liveInst[cdgpredBBTerminator]) {
        workList.push(cdgpredBBTerminator);
      }
    }
    for (auto op : curInst->operands()) {
      auto opInst = dyn_cast<ir::Instruction>(op->value());
      if (opInst and not liveInst[opInst]) workList.push(opInst);
    }
  }
  // delete useless insts
  // std::cerr << "Delete useless insts" << std::endl;
  for (auto bb : func->blocks()) {
    for (auto instIter = bb->insts().begin(); instIter != bb->insts().end();) {
      auto inst = *instIter;
      instIter++;
      if (not liveInst[inst] and not dyn_cast<ir::BranchInst>(inst)) {
        if (dyn_cast<ir::CallInst>(inst)) isCallGraphChange = true;
        bb->force_delete_inst(inst);
      }
    }
  }
  // delete bb
  // std::cerr << "Delete useless bb" << std::endl;
  for (auto bbIter = func->blocks().begin(); bbIter != func->blocks().end();) {
    auto bb = *bbIter;
    bbIter++;
    if ((not liveBB[bb]) and (bb != func->entry())) {
      func->forceDelBlock(bb);
      isCFGChange = true;
    }
  }
  // rebuild jmp
  // unnecessary to rebuild phi, because:
  // if a phi inst is alive, all its incoming bbs are alive
  // std::cerr << "Rebuild jmp" << std::endl;
  for (auto bb : func->blocks()) {
    auto terInst = dyn_cast<ir::BranchInst>(bb->terminator());
    if (terInst == nullptr) continue;
    if (terInst->is_cond()) {
      // std::cerr<<"isCond:"<<terInst->is_cond()<<std::endl;
      // std::cerr<<"Operands size:"<<terInst->operands().size()<<std::endl;
      auto trueTarget = terInst->iftrue();
      auto falseTarget = terInst->iffalse();
      assert(trueTarget != nullptr and falseTarget != nullptr);
      if (not liveBB[trueTarget]) {
        auto newTarget = getTargetBB(trueTarget);
        assert(newTarget != nullptr);
        terInst->set_iftrue(newTarget);
      }
      if (not liveBB[terInst->iffalse()]) {
        auto newTarget = getTargetBB(falseTarget);
        assert(newTarget != nullptr);
        terInst->set_iffalse(newTarget);
      }
      if (terInst->iffalse() == terInst->iftrue()) {
        auto dest = terInst->iftrue();
        auto newBr = new ir::BranchInst(dest);
        newBr->setBlock(bb);
        bb->force_delete_inst(terInst);
        bb->emplace_back_inst(newBr);
      }
    } else {
      // std::cerr<<"isCond:"<<terInst->is_cond()<<std::endl;
      // std::cerr<<"Operands size:"<<terInst->operands().size()<<std::endl;
      auto jmpTarget = terInst->dest();
      if (not liveBB[jmpTarget]) {
        auto newTarget = getTargetBB(jmpTarget);
        assert(newTarget != nullptr);
        terInst->set_dest(newTarget);
      }
    }
  }

  // std::cerr << "isCFGChange: " << isCFGChange << std::endl;
  if (isCFGChange) {
    tp->CFGChange(func);
    // rebuild CFG
    for (auto bb : func->blocks()) {
      bb->pre_blocks().clear();
      bb->next_blocks().clear();
    }
    for (auto bb : func->blocks()) {
      auto terInst = bb->terminator();
      auto brTerInst = dyn_cast<ir::BranchInst>(terInst);
      if (brTerInst) {
        if (brTerInst->is_cond()) {
          ir::BasicBlock::block_link(bb, brTerInst->iftrue());
          ir::BasicBlock::block_link(bb, brTerInst->iffalse());
        } else {
          ir::BasicBlock::block_link(bb, brTerInst->dest());
        }
      }
    }
  }
  // std::cerr << "isCallGraphChange: " << isCallGraphChange << std::endl;
  if (isCallGraphChange) tp->CallChange();
}

ir::BasicBlock* ADCEContext::getTargetBB(ir::BasicBlock* bb) {
  // std::cerr << "Get target BB for " << bb->name() << std::endl;
  auto targetBB = bb;
  while (not liveBB[targetBB]) {
    targetBB = pdctx->ipdom(targetBB);
    // std::cerr << "Target BB is not live, find its ipdom: " << targetBB->name() << std::endl;
    assert(targetBB && "Target BB is null!");
  }
  // std::cerr << "Target BB is " << targetBB->name() << std::endl;
  return targetBB;
}

void ADCEContext::ADCEInfoCheck(ir::Function* func) {
  using namespace std;
  cout << "In Function " << func->name() << " :" << endl;
  for (auto bb : func->blocks()) {
    cout << bb->name() << " alive: " << liveBB[bb] << endl;
  }
}
}  // namespace pass