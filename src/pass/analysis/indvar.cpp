#include "pass/analysis/indvar.hpp"

using namespace pass;

void IndVarAnalysis::run(Function* func, TopAnalysisInfoManager* tp) {
  IndVarAnalysisContext ctx;
  ctx.run(func, tp);
}

void IndVarAnalysisContext::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  if (func->isOnlyDeclare()) return;
  domctx = tp->getDomTree(func);
  lpctx = tp->getLoopInfo(func);
  sectx = tp->getSideEffectInfo();
  ivctx = tp->getIndVarInfoWithoutRefresh(func);
  ivctx->clearAll();
  func->rename();
  for (auto lp : lpctx->loops()) {
    // std::cerr<<lp<<std::endl;
    auto lpHeader = lp->header();
    // std::cerr<<lp->header()->name()<<std::endl;
    if (not lp->isLoopSimplifyForm()) continue;
    auto lpPreHeader = lp->getLoopPreheader();
    auto lpHeaderTerminator = dyn_cast<ir::BranchInst>(lpHeader->terminator());
    if (lpHeaderTerminator == nullptr) continue;  // header's terminator must be brcond
    if (not lpHeaderTerminator->is_cond()) continue;
    if (lp->exits().size() > 1) continue;
    auto lpCond = lpHeaderTerminator->cond();
    auto lpCondScid = lpCond->valueId();
    ir::PhiInst* keyPhiInst;
    ir::Value* mEndVar;
    if (lpCondScid >= ir::vICMP_BEGIN and lpCondScid <= ir::vICMP_END) {
      auto lpCondIcmp = dyn_cast<ir::ICmpInst>(lpCond);
      auto lpCondIcmpLHSPhi = lpCondIcmp->lhs()->dynCast<ir::PhiInst>();
      auto lpCondIcmpRHSPhi = lpCondIcmp->rhs()->dynCast<ir::PhiInst>();
      // if(not (lpCondIcmpLHSPhi!=nullptr and lpCondIcmpRHSPhi!=nullptr))continue;
      if (lpCondIcmpLHSPhi != nullptr and lpCondIcmpLHSPhi->block() == lpHeader) {
        keyPhiInst = lpCondIcmpLHSPhi;
        mEndVar = lpCondIcmp->rhs();
      } else if (lpCondIcmpRHSPhi != nullptr and lpCondIcmpRHSPhi->block() == lpHeader) {
        keyPhiInst = lpCondIcmpRHSPhi;
        mEndVar = lpCondIcmp->lhs();
      } else
        continue;
    } else
      continue;
    if (not isSimplyLoopInvariant(lp, mEndVar)) continue;
    auto mBeginVar = keyPhiInst->getvalfromBB(lpPreHeader);
    if (not isSimplyLoopInvariant(lp, mBeginVar)) continue;
    auto iterInst = keyPhiInst->getValue(0) == keyPhiInst->getvalfromBB(lpPreHeader)
                      ? keyPhiInst->getValue(1)
                      : keyPhiInst->getValue(0);
    auto iterInstScid = iterInst->valueId();
    ir::Value* mstepVar;
    if (not(iterInstScid == ir::vADD or iterInstScid == ir::vFADD or iterInstScid == ir::vSUB or
            iterInstScid == ir::vFSUB or iterInstScid == ir::vMUL or iterInstScid == ir::vFMUL)) {
      if (not(iterInstScid == ir::vPHI)) continue;
      auto phiinst = iterInst->dynCast<ir::PhiInst>();
      auto phiinstBlock = phiinst->block();
      if (lp->latchs().count(phiinst->block()) == 0) continue;
      iterInst = phiinst->getConstantRepl();
      if (iterInst == nullptr) continue;
      iterInstScid = iterInst->valueId();
      if (not(iterInstScid == ir::vADD or iterInstScid == ir::vFADD or iterInstScid == ir::vSUB or
              iterInstScid == ir::vFSUB or iterInstScid == ir::vMUL or iterInstScid == ir::vFMUL))
        continue;
    }
    auto iterInstBinary = dyn_cast<ir::BinaryInst>(iterInst);
    if (iterInstBinary->lValue()->valueId() == ir::vPHI) {
      if (dyn_cast<ir::PhiInst>(iterInstBinary->lValue()) != keyPhiInst) continue;
      mstepVar = iterInstBinary->rValue();
      if (not isSimplyLoopInvariant(lp, mstepVar)) continue;
    } else if (iterInstBinary->rValue()->valueId() == ir::vPHI) {
      if (dyn_cast<ir::PhiInst>(iterInstBinary->rValue()) != keyPhiInst) continue;
      mstepVar = iterInstBinary->lValue();
      if (not isSimplyLoopInvariant(lp, mstepVar)) continue;
    } else
      continue;
    addIndVar(lp, mBeginVar, mstepVar, mEndVar, iterInstBinary, dyn_cast<ir::Instruction>(lpCond),
              keyPhiInst);
    // using namespace std;
    // auto idv = ivctx->getIndvar(lp);
    // if (idv == nullptr) {
    //     cerr << "No indvar." << endl;
    // } else {
    //     cerr << "BeginVar:\t" << idv->getBeginI32() << endl;
    //     cerr << "StepVar :\t" << idv->getStepI32() << endl;
    //     if(idv->isEndVarConst())
    //     cerr << "EndVar  :\t" << idv->getEndVarI32() << endl;
    // }
  }
}

void IndVarAnalysisContext::addIndVar(ir::Loop* lp,
                                      ir::Value* mbegin,
                                      ir::Value* mstep,
                                      ir::Value* mend,
                                      ir::BinaryInst* iterinst,
                                      ir::Instruction* cmpinst,
                                      ir::PhiInst* phiinst) {
  auto pnewIdv = new ir::IndVar(mbegin, mend, mstep, iterinst, cmpinst, phiinst);
  ivctx->addIndVar(lp, pnewIdv);
}

void IndVarInfoCheck::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  if (func->isOnlyDeclare()) return;
  auto lpctx = tp->getLoopInfoWithoutRefresh(func);
  auto ivctx = tp->getIndVarInfoWithoutRefresh(func);
  using namespace std;
  cerr << "In Function " << func->name() << ":" << endl;
  for (auto lp : lpctx->loops()) {
    cerr << "In loop whose header is " << lp->header()->name() << ":" << endl;
    // cerr<<lp<<endl;
    auto idv = ivctx->getIndvar(lp);
    if (idv == nullptr) {
      cerr << "No indvar." << endl;
    } else {
      cerr << "BeginVar:\t" << idv->getBeginI32() << endl;
      cerr << "StepVar :\t" << idv->getStepI32() << endl;
      if (idv->isEndVarConst()) cerr << "EndVar  :\t" << idv->getEndVarI32() << endl;
    }
  }
}

// ir::ConstantInteger* indVarAnalysis::getConstantBeginVarFromPhi(ir::PhiInst* phiinst,ir::PhiInst*
// oldPhiinst,ir::Loop* lp){
//     if(not lp->isLoopSimplifyForm())return nullptr;
//     if(phiinst->block()!=lp->header())return nullptr;
//     auto lpPreHeader=lp->getLoopPreheader();
//     if(phiinst->getsize()!=2)return nullptr;
//     auto phivalfromlpPreHeader=phiinst->getvalfromBB(lpPreHeader);
//     auto phivalfromLatch=phiinst->getvalfromBB(*lp->latchs().begin());
//     if(lp->latchs().size()!=1)return nullptr;
//     if(phivalfromLatch!=oldPhiinst)return nullptr;
//     auto constVal=phivalfromlpPreHeader->dynCast<ir::ConstantInteger>();
//     if(constVal!=nullptr and constVal->isInt32())return constVal;
//     auto phiVal=phivalfromlpPreHeader->dynCast<ir::PhiInst>();
//     if(phiVal==nullptr)return nullptr;
//     auto outerLp=lp->parent();
//     if(outerLp==nullptr)return nullptr;
//     return getConstantBeginVarFromPhi(phiVal,phiinst,outerLp);
// }

bool IndVarAnalysisContext::isSimplyNotInLoop(ir::Loop* lp, ir::Value* val) {
  if (auto instVal = val->dynCast<ir::Instruction>()) {
    return lp->blocks().count(instVal->block()) == 0;
  }
  if (auto argVal = val->dynCast<ir::Argument>()) {
    return true;
  }
  if (auto constVal = val->dynCast<ir::ConstantValue>()) {
    return true;
  }
  return false;
}

// 简单的判断一下对应的value是不是循环不变量
bool IndVarAnalysisContext::isSimplyLoopInvariant(ir::Loop* lp, ir::Value* val) {
  if (auto instVal = val->dynCast<ir::Instruction>()) {
    if (domctx->dominate(instVal->block(), lp->header()) and instVal->block() != lp->header())
      return true;
  }
  if (auto conVal = val->dynCast<ir::ConstantValue>()) return true;
  if (auto binaryVal = val->dynCast<ir::BinaryInst>()) {
    return isSimplyNotInLoop(lp, binaryVal->lValue()) and
           isSimplyNotInLoop(lp, binaryVal->rValue());
  }
  if (auto unaryVal = val->dynCast<ir::UnaryInst>()) {
    return isSimplyNotInLoop(lp, unaryVal->value());
  }
  if (auto callVal = val->dynCast<ir::CallInst>()) {
    if (not sectx->isPureFunc(callVal->callee())) return false;
    for (auto rarg : callVal->rargs()) {
      if (not isSimplyLoopInvariant(lp, rarg->value())) return false;
    }
    return true;
  }
  if (auto phiinst = val->dynCast<ir::PhiInst>()) {
    return domctx->dominate(phiinst->block(), lp->header()) and phiinst->block() != lp->header();
  }
  if (auto arg = val->dynCast<ir::Argument>()) {
    return true;
  }
  if (auto loadinst = val->dynCast<ir::LoadInst>()) {
    return domctx->dominate(loadinst->block(), lp->header()) and loadinst->block() != lp->header();
  }
  return false;
}