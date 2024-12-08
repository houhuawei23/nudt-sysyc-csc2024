#include "pass/analysis/MarkParallel.hpp"
#include "pass/analysis/dependenceAnalysis/DependenceAnalysis.hpp"
using namespace pass;
using namespace ir;
void MarkParallel::run(Function* func, TopAnalysisInfoManager* tp) {
  MarkParallelContext context;
  context.run(func, tp);
}
void MarkParallelContext::run(Function* func, TopAnalysisInfoManager* tp) {
  topmana = tp;
  dpctx = tp->getDepInfo(func);
  domctx = tp->getDomTreeWithoutRefresh(func);
  lpctx = tp->getLoopInfoWithoutRefresh(func);
  cgctx = tp->getCallGraphWithoutRefresh();
  sectx = tp->getSideEffectInfoWithoutRefresh();
  idvctx = tp->getIndVarInfoWithoutRefresh(func);
  parctx = tp->getParallelInfo(func);
  for (auto loop : lpctx->loops()) {
    runOnLoop(loop);
  }
  printParallelInfo(func);
}

void MarkParallelContext::runOnLoop(Loop* lp) {
  auto lpDepInfo = dpctx->getLoopDependenceInfo(lp);
  bool isParallelConcerningArray = lpDepInfo->getIsParallel();
  auto defaultIdv = idvctx->getIndvar(lp);
  if (isParallelConcerningArray == false) {
    std::cerr << "Loop " << lp->header()->name() << " is not parallel concerning array."
              << std::endl;
    parctx->setIsParallel(lp->header(), false);
    return;
  }
  for (auto bb : lp->blocks()) {
    for (auto inst : bb->insts()) {
      if (auto callInst = inst->dynCast<CallInst>()) {
        auto callee = callInst->callee();
        if (not isFuncParallel(lp, callInst)) {
          std::cerr << "Loop " << lp->header()->name() << " is not parallel concerning function "
                    << callee->name() << std::endl;
          parctx->setIsParallel(lp->header(), false);
          return;
        }
      } else if (auto storeInst = inst->dynCast<StoreInst>()) {
        auto ptr = storeInst->ptr();
        if (ptr->dynCast<GlobalVariable>()) {
          std::cerr << "Loop " << lp->header()->name()
                    << " is not parallel concerning global variable " << ptr->name() << std::endl;
          parctx->setIsParallel(lp->header(), false);
          return;
        }
      }
    }
  }
  // if(lp->header()->phi_insts().size()>1){//接下来就开始处理每一个phi
  //     for(auto pi:lp->header()->phi_insts()){
  //         auto phi=pi->dynCast<PhiInst>();
  //         if(phi==defaultIdv->phiinst())continue;
  //         auto res=getResPhi(phi,lp);
  //         if(res==nullptr){
  //             parctx->setIsParallel(lp->header(),false);
  //             return;
  //         }
  //         parctx->setPhi(phi,res->isAdd,res->isSub,res->isMul,res->mod);
  //     }
  // }
  parctx->setIsParallel(lp->header(), true);
  return;
}

void MarkParallelContext::printParallelInfo(Function* func) {
  std::cerr << "In Function " << func->name() << ":" << std::endl;
  for (auto lp : lpctx->loops()) {
    using namespace std;
    cerr << "Parallize Loop whose header is " << lp->header()->name() << " :";
    if (parctx->getIsParallel(lp->header())) {
      cerr << "YES";
    } else {
      cerr << "NO";
    }
    cerr << endl;
  }
}

ResPhi* MarkParallelContext::getResPhi(PhiInst* phi, Loop* lp) {
  assert(phi->block() == lp->header());
  if (phi->isFloat32()) return nullptr;
  auto lpPreheader = lp->getLoopPreheader();
  auto preheaderIncoming = phi->getvalfromBB(lpPreheader);
  if (lp->latchs().size() > 1) return nullptr;
  auto latchIncoming = phi->getvalfromBB(*lp->latchs().begin());
  if (auto binaryLatchIncoming = latchIncoming->dynCast<BinaryInst>()) {
    auto pnewResPhi = new ResPhi;
    pnewResPhi->phi = phi;
    pnewResPhi->isAdd = false;
    pnewResPhi->isSub = false;
    pnewResPhi->isMul = false;
    pnewResPhi->isModulo = false;
    auto binaryLatchIncomingInstId = binaryLatchIncoming->valueId();
    Instruction* curInst;
    if (binaryLatchIncomingInstId == vADD or binaryLatchIncomingInstId == vMUL or
        binaryLatchIncomingInstId == vSUB) {
      pnewResPhi->isModulo = false;
      pnewResPhi->mod = nullptr;
      curInst = binaryLatchIncoming->dynCast<Instruction>();
      if (binaryLatchIncomingInstId == vADD) {
        if (binaryLatchIncoming->lValue() == phi or binaryLatchIncoming->rValue() == phi) {
          pnewResPhi->isAdd = true;
          return pnewResPhi;
        }
        return nullptr;
      }
      if (binaryLatchIncomingInstId == vMUL) {
        if (binaryLatchIncoming->lValue() == phi or binaryLatchIncoming->rValue() == phi) {
          pnewResPhi->isMul = true;
          return pnewResPhi;
        }
        return nullptr;
      }
      if (binaryLatchIncomingInstId == vSUB) {
        if (binaryLatchIncoming->lValue() == phi) {
          pnewResPhi->isSub = true;
          return pnewResPhi;
        }
        return nullptr;
      }
    } else if (binaryLatchIncomingInstId == vSREM) {
      pnewResPhi->isModulo = true;
      pnewResPhi->mod = binaryLatchIncoming->rValue();
      curInst = binaryLatchIncoming->lValue()->dynCast<Instruction>();
      auto curInstId = curInst->valueId();
      if (curInstId == vADD) {
        if (binaryLatchIncoming->lValue() == phi or binaryLatchIncoming->rValue() == phi) {
          pnewResPhi->isAdd = true;
          return pnewResPhi;
        }
        return nullptr;
      }
      if (curInstId == vMUL) {
        if (binaryLatchIncoming->lValue() == phi or binaryLatchIncoming->rValue() == phi) {
          pnewResPhi->isMul = true;
          return pnewResPhi;
        }
        return nullptr;
      }
      if (curInstId == vSUB) {
        if (binaryLatchIncoming->lValue() == phi) {
          pnewResPhi->isSub = true;
          return pnewResPhi;
        }
        return nullptr;
      }
      return nullptr;

    } else {
      delete pnewResPhi;
      return nullptr;
    }
  } else {
    return nullptr;
  }
  return nullptr;
}

bool MarkParallelContext::isSimplyLpInvariant(Loop* lp, Value* val) {
  if (auto constVal = val->dynCast<ConstantInteger>()) {
    return true;
  }
  if (auto arg = val->dynCast<Argument>()) {
    return true;
  }
  if (auto inst = val->dynCast<Instruction>()) {
    return domctx->dominate(inst->block(), lp->header()) and inst->block() != lp->header();
  }
  return false;
}

bool MarkParallelContext::isFuncParallel(Loop* lp, CallInst* callinst) {
  auto func = callinst->callee();
  auto lpDepInfo = dpctx->getLoopDependenceInfo(lp);
  if (sectx->hasSideEffect(func)) return false;
  // 只读内存不写的函数，看读的和人家写的一不一样
  std::set<Value*> readLocs;
  // 读取的几个部分：数组，只会读取arg数组和全局数组，分别使用副作用来判断
  for (auto arrArg : sectx->funcArgSet(func)) {
    auto arrArgIdx = arrArg->index();
    auto arrRArg = callinst->rargs().at(arrArgIdx)->value();
    readLocs.insert(getBaseAddr(arrRArg));  // getBaseAddr;
  }
  for (auto readGv : sectx->funcReadGlobals(func)) {
    readLocs.insert(readGv);
  }
  // 查看当前循环写的地址中有无对应的地址
  for (auto writeGv : lpDepInfo->getBaseAddrs()) {
    if (lpDepInfo->getIsBaseAddrWrite(writeGv)) {
      if (readLocs.count(writeGv)) return false;
    }
  }
  return true;
}

Value* MarkParallelContext::getBaseAddr(Value* subAddr) {
  if (auto allocainst = subAddr->dynCast<AllocaInst>()) return allocainst;
  if (auto gv = subAddr->dynCast<GlobalVariable>()) return gv;
  if (auto arg = subAddr->dynCast<Argument>()) return arg;
  if (auto gep = subAddr->dynCast<GetElementPtrInst>()) return getBaseAddr(gep->value());
  if (auto phi = subAddr->dynCast<PhiInst>()) {
    auto func = phi->block()->function();
    auto lpctx = topmana->getLoopInfo(func);
    auto lp = lpctx->head2loop(phi->block());
    auto preHeaderVal = phi->getvalfromBB(lp->getLoopPreheader());
    return getBaseAddr(preHeaderVal);
  }
  // assert("Error! invalid type of input in function getBaseAddr!"&&false);
  return nullptr;
}