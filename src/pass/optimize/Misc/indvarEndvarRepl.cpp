#include "pass/optimize/indvarEndvarRepl.hpp"
using namespace pass;

void IdvEdvRepl::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  IdvEdvReplContext ctx;
  ctx.run(func, tp);
}
void IdvEdvReplContext::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  idvctx = tp->getIndVarInfo(func);
  lpctx = tp->getLoopInfoWithoutRefresh(func);
  domctx = tp->getDomTreeWithoutRefresh(func);
  sectx = tp->getSideEffectInfo();
  for (auto lp : lpctx->loops()) {
    if (lp->parentloop() != nullptr) continue;
    runOnLoop(lp);
  }
}

void IdvEdvReplContext::runOnLoop(ir::Loop* lp) {
  for (auto subLoop : lp->subLoops()) {
    runOnLoop(subLoop);
  }
  auto idv = idvctx->getIndvar(lp);
  if (idv == nullptr) return;
  if (not idv->isStepVarConst()) return;
  auto stepConstVal = idv->getStepI32();
  ir::Value* finalVar = nullptr;
  // 激进做法，直接将indvar的后面使用替换而不限制其循环执行与否
  if (idv->isEndVarConst() and idv->isBeginVarConst()) {
    int iterCnt = getConstantEndvarIndVarIterCnt(lp, idv);
    if (iterCnt != -1) {
      finalVar = ir::ConstantInteger::gen_i32(iterCnt * idv->getStepI32() + idv->getBeginI32());
      replaceIndvarAfterLoop(lp, idv, finalVar);
      return;
    } else {
      return;
    }
  }
  normalizeIcmpAndBr(lp, idv);
  // 直接处理特殊情况：step=1，为<,<=,>,>=,分别替换为
  if (stepConstVal == 1) {
    auto endVar = idv->endValue();
    auto icmpInstId = idv->cmpInst()->valueId();
    if (icmpInstId == ir::vISLE) {
      finalVar = addFinalVarInstInLatchAdd1(endVar, lp);
    } else if (icmpInstId == ir::vISLT) {
      finalVar = endVar;
    } else if (icmpInstId == ir::vISGE) {
      finalVar = addFinalVarInstInLatchSub1(endVar, lp);

    } else if (icmpInstId == ir::vISGT) {
      finalVar = endVar;
    }
    if (finalVar == nullptr) return;
    replaceIndvarAfterLoop(lp, idv, finalVar);
  }
}

// 简单的判断一下对应的value是不是循环不变量
bool IdvEdvReplContext::isSimplyLoopInvariant(ir::Loop* lp, ir::Value* val) {
  if (auto conVal = val->dynCast<ir::ConstantValue>()) return true;
  if (auto inst = val->dynCast<ir::Instruction>()) {
    return domctx->dominate(inst->block(), lp->header()) and inst->block() != lp->header();
  }
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
  return false;
}

// 如果endvar是常数，就直接计算出对应的迭代次数
int IdvEdvReplContext::getConstantEndvarIndVarIterCnt(ir::Loop* lp, ir::IndVar* idv) {
  //-1 不能计算
  assert(idv->isEndVarConst());
  auto beginVar = idv->getBeginI32();
  auto endVar = idv->getEndVarI32();
  auto stepVar = idv->getStepI32();
  auto icmpinst = idv->cmpInst();
  auto iterinst = idv->iterInst();
  if (stepVar == 0) return -1;
  // 对icmp进行标准化
  normalizeIcmpAndBr(lp, idv);
  switch (icmpinst->valueId()) {
    case ir::vIEQ:
      if (beginVar == endVar)
        return 1;
      else
        return 0;
      break;
    case ir::vINE:
      if (iterinst->valueId() == ir::vADD) {
        if ((endVar - beginVar) % stepVar != 0) return -1;
        auto cnt = (endVar - beginVar) / stepVar;
        if (cnt < 0) return -1;
        return cnt;
      } else if (iterinst->valueId() == ir::vSUB) {
        if ((beginVar - endVar) % stepVar != 0) return -1;
        auto cnt = -(endVar - beginVar) / stepVar;
        if (cnt < 0) return -1;
        return cnt;
      } else {      // MUL
        return -1;  // TODO: do not support != with MUL
      }
      break;
    case ir::vISGT:
      if (iterinst->valueId() == ir::vADD) {
        if (stepVar > 0) return -1;
        if (endVar >= beginVar) return -1;
        auto cnt = (endVar - beginVar) / stepVar;
        if ((endVar - beginVar) % stepVar == 0)
          return cnt;
        else
          return cnt + 1;
      } else if (iterinst->valueId() == ir::vSUB) {
        if (stepVar < 0) return -1;
        if (beginVar <= endVar) return -1;
        auto cnt = (beginVar - endVar) / stepVar;
        if ((beginVar - endVar) % stepVar == 0)
          return cnt;
        else
          return cnt + 1;
      } else if (iterinst->valueId() == ir::vMUL) {
        return -1;  // TODO: do not support != with MUL
      } else {
        assert(false and "invalid operator in IndVar!");
      }
      break;
    case ir::vISGE:
      if (iterinst->valueId() == ir::vADD) {
        if (stepVar > 0) return -1;
        if (endVar >= beginVar) return -1;
        auto cnt = (endVar - beginVar) / stepVar;
        return cnt + 1;
      } else if (iterinst->valueId() == ir::vSUB) {
        if (stepVar < 0) return -1;
        if (beginVar <= endVar) return -1;
        auto cnt = (beginVar - endVar) / stepVar;
        return cnt + 1;
      } else if (iterinst->valueId() == ir::vMUL) {
        return -1;  // TODO: do not support != with MUL
      } else {
        assert(false and "invalid operator in IndVar!");
      }
      break;
    case ir::vISLT:
      if (iterinst->valueId() == ir::vADD) {
        if (stepVar < 0) return -1;
        if (endVar <= beginVar) return -1;
        auto cnt = (endVar - beginVar) / stepVar;
        if ((endVar - beginVar) % stepVar == 0)
          return cnt;
        else
          return cnt + 1;
      } else if (iterinst->valueId() == ir::vSUB) {
        if (stepVar > 0) return -1;
        if (beginVar <= endVar) return -1;
        auto cnt = (beginVar - endVar) / stepVar;
        if ((beginVar - endVar) % stepVar == 0)
          return cnt;
        else
          return cnt + 1;
      } else if (iterinst->valueId() == ir::vMUL) {
        return -1;  // TODO: do not support != with MUL
      } else {
        assert(false and "invalid operator in IndVar!");
      }
      break;
    case ir::vISLE:
      if (iterinst->valueId() == ir::vADD) {
        if (stepVar < 0) return -1;
        if (endVar <= beginVar) return -1;
        auto cnt = (endVar - beginVar) / stepVar;
        return cnt + 1;
      } else if (iterinst->valueId() == ir::vSUB) {
        if (stepVar > 0) return -1;
        if (beginVar <= endVar) return -1;
        auto cnt = (beginVar - endVar) / stepVar;
        return cnt + 1;
      } else if (iterinst->valueId() == ir::vMUL) {
        return -1;  // TODO: do not support != with MUL
      } else {
        assert(false and "invalid operator in IndVar!");
      }
      break;
    default:
      break;
  }
  return -1;
}

// 标准化:把idv放在op1 把endvar放在op2,icmp true就保持循环,false就跳出
void IdvEdvReplContext::normalizeIcmpAndBr(ir::Loop* lp, ir::IndVar* idv) {
  auto endvar = idv->endValue();
  auto icmpInst = idv->cmpInst()->dynCast<ir::ICmpInst>();
  auto brInst = lp->header()->terminator()->dynCast<ir::BranchInst>();
  assert(icmpInst != nullptr);
  bool IsIcmpOpNorm = icmpInst->rhs() == endvar;
  bool IsBrDestNorm = lp->blocks().count(brInst->iftrue()) > 0;
  if (not IsBrDestNorm and not IsIcmpOpNorm) {
    std::cerr << "Lp Br and Icmp both not normalized!" << std::endl;
    exchangeIcmpOp(icmpInst);
    exchangeBrDest(brInst);
    reverseIcmpOp(icmpInst);
  } else if (not IsBrDestNorm) {
    std::cerr << "Lp Br not normalized!" << std::endl;
    reverseIcmpOp(icmpInst);
    exchangeBrDest(brInst);

  } else if (not IsIcmpOpNorm) {
    std::cerr << "Lp Icmp both not normalized!" << std::endl;
    exchangeIcmpOp(icmpInst);
  }
}

// 交换两个Icmp中的Op以使得ind在LHS
void IdvEdvReplContext::exchangeIcmpOp(ir::ICmpInst* icmpInst) {
  auto LHS = icmpInst->lhs();
  auto RHS = icmpInst->rhs();
  // 改变ValueId
  reverseIcmpOp(icmpInst);
  // 交换op
  icmpInst->setOperand(0, RHS);
  icmpInst->setOperand(1, LHS);
}

// 翻转这个Icmp的符号使得原意不变
void IdvEdvReplContext::reverseIcmpOp(ir::ICmpInst* icmpInst) {
  switch (icmpInst->valueId()) {
    case ir::vIEQ:
      break;
    case ir::vINE:
      break;
    case ir::vISGE:
      icmpInst->setCmpOp(ir::vISLE);
      break;
    case ir::vISLE:
      icmpInst->setCmpOp(ir::vISGE);
      break;
    case ir::vISLT:
      icmpInst->setCmpOp(ir::vISGT);
      break;
    case ir::vISGT:
      icmpInst->setCmpOp(ir::vISLT);
      break;
    default:
      assert(false and "invalid ICMP Op!");
      break;
  }
}

// 交换brcond的跳转两个目标
void IdvEdvReplContext::exchangeBrDest(ir::BranchInst* brInst) {
  assert(brInst->is_cond());
  auto trueTarget = brInst->iftrue();
  auto falseTarget = brInst->iffalse();
  brInst->set_iftrue(falseTarget);
  brInst->set_iffalse(trueTarget);
}

bool IdvEdvReplContext::isSimplyNotInLoop(ir::Loop* lp, ir::Value* val) {
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

void IdvEdvReplContext::replaceIndvarAfterLoop(ir::Loop* lp, ir::IndVar* idv, ir::Value* finalVar) {
  auto idvPhi = idv->phiinst();
  for (auto puseiter = idvPhi->uses().begin(); puseiter != idvPhi->uses().end();) {
    auto puse = *puseiter;
    puseiter++;
    auto puser = puse->user();
    auto userInst = puser->dynCast<ir::Instruction>();
    if (lp->blocks().count(userInst->block())) continue;
    auto useIdx = puse->index();
    userInst->setOperand(useIdx, finalVar);
  }
}

ir::Value* IdvEdvReplContext::addFinalVarInstInLatchSub1(ir::Value* edv, ir::Loop* lp) {
  if (lp->exits().size() > 1) return nullptr;
  auto exitBB = *lp->exits().begin();
  auto pnewSubInst = new ir::BinaryInst(ir::vSUB, ir::Type::TypeInt32(), edv,
                                        ir::ConstantInteger::gen_i32(1), exitBB);
  exitBB->emplace_first_inst(pnewSubInst);
  return pnewSubInst;
}

ir::Value* IdvEdvReplContext::addFinalVarInstInLatchAdd1(ir::Value* edv, ir::Loop* lp) {
  if (lp->exits().size() > 1) return nullptr;
  auto exitBB = *lp->exits().begin();
  auto pnewSubInst = new ir::BinaryInst(ir::vADD, ir::Type::TypeInt32(), edv,
                                        ir::ConstantInteger::gen_i32(1), exitBB);
  exitBB->emplace_first_inst(pnewSubInst);
  return pnewSubInst;
}