/*
循环标准化:
1. 保证所有的循环条件判断都是左边indvar右边endvar
2. 保证所有的循环头条件跳转都是真继续循环 假退出循环
循环标量计算:
对于所有再循环中每次+-循环不变量的值进行直接计算
对于二阶和indvar*LInv的情况放着先太抽象了
*/
#include "pass/optimize/SCEV.hpp"
#include "pass/optimize/SCCP.hpp"
using namespace pass;

static std::vector<SCEVValue*> SCEVValues;
static std::vector<ir::BinaryInst*> binstStk;  // used in findAddSubChain and getSCEVValue

void SCEV::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  SCEVContext scevctx;
  scevctx.run(func, tp);
}

void SCEVContext::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  if (func->isOnlyDeclare()) return;
  domctx = tp->getDomTree(func);
  lpctx = tp->getLoopInfo(func);
  idvctx = tp->getIndVarInfo(func);
  sectx = tp->getSideEffectInfo();
  func->rename();

  for (auto lp : lpctx->loops()) {
    if (lp->parentloop() != nullptr) continue;  // 只处理顶层循环，底层循环通过顶层循环向下分析
    runOnLoop(lp, tp);
  }
}

void SCEVContext::runOnLoop(ir::Loop* lp, TopAnalysisInfoManager* tp) {
  if (lp == nullptr) return;
  if (lp->exits().size() > 1) return;  // 不处理多出口
  auto defaultIdv = idvctx->getIndvar(lp);
  // if(defaultIdv==nullptr)return;//必须有基础indvar
  if (not lp->isLoopSimplifyForm()) return;
  for (auto subLp : lp->subLoops()) {
    runOnLoop(subLp, tp);
  }
  if (defaultIdv == nullptr) return;   // 要进行分析，必须具有基础indvar形式
  normalizeIcmpAndBr(lp, defaultIdv);  // 标准化br和Icmp
  SCCP sccp = SCCP();
  sccp.run(lp->header()->function(), tp);
  auto lpHeader = lp->header();
  SCEVValues.clear();
  for (auto pinst : lpHeader->phi_insts()) {
    auto phiinst = pinst->dynCast<ir::PhiInst>();
    if (phiinst == defaultIdv->phiinst()) continue;  // 不再分析indvar
    if (not isUsedOutsideLoop(lp, phiinst)) continue;  // 只有在外被使用对应value才有被化简的价值
    visitPhi(lp, phiinst);
  }
  if (SCEVValues.empty()) return;  // 没有可以化简的SCEV,return直接
  std::cerr << "Got " << SCEVValues.size() << " SCEV Values!" << std::endl;
  ir::Value* IterCntVal;
  ir::IRBuilder builder;
  builder.set_pos(*(lp->exits().begin()));
  if (defaultIdv->isEndVarConst()) {
    int iterCnt = getConstantEndvarIndVarIterCnt(lp, defaultIdv);
    IterCntVal = ir::ConstantInteger::gen_i32(iterCnt);
  } else {
    IterCntVal = addCalcIterCntInstructions(lp, defaultIdv, builder);
  }
  for (auto scevval : SCEVValues) {
    SCEVReduceInstr(lp, scevval, IterCntVal, builder);
  }
}

void SCEVContext::SCEVReduceInstr(ir::Loop* lp,
                                  SCEVValue* scevVal,
                                  ir::Value* itercnt,
                                  ir::IRBuilder& builder) {
  if (scevVal->addsteps.empty() and scevVal->substeps.empty()) return;
  ir::Value* finalStepVar;
  ir::Value* finalVar;
  // builder.set_pos(*lp->exits().begin());
  if (not scevVal->addsteps.empty()) {
    finalStepVar = *scevVal->addsteps.begin();
    for (auto subval : scevVal->substeps) {
      finalStepVar = builder.makeBinary(ir::SUB, finalStepVar, subval);
    }
    bool tmpBool = false;
    for (auto addval : scevVal->addsteps) {
      if (not tmpBool) {
        tmpBool = true;
        continue;
      }
      finalStepVar = builder.makeBinary(ir::ADD, finalStepVar, addval);
    }
    if (finalStepVar->isFloat32())
      itercnt = builder.makeUnary(ir::vSITOFP, itercnt, ir::Type::TypeFloat32());
    finalVar = builder.makeBinary(ir::MUL, itercnt, finalStepVar);
    finalVar = builder.makeBinary(ir::ADD, finalVar, scevVal->initVal);
  } else {
    finalStepVar = scevVal->substeps.front();
    bool tmpBool = false;
    for (auto subval : scevVal->substeps) {
      if (not tmpBool) {
        tmpBool = true;
        continue;
      }
      finalStepVar = builder.makeBinary(ir::ADD, finalStepVar, subval);
    }
    if (finalStepVar->isFloat32())
      itercnt = builder.makeUnary(ir::vSITOFP, itercnt, ir::Type::TypeFloat32());
    finalVar = builder.makeBinary(ir::MUL, itercnt, finalStepVar);
    finalVar = builder.makeBinary(ir::SUB, scevVal->initVal, finalVar);
  }
  for (auto puseIter = scevVal->phiinst->uses().begin();
       puseIter != scevVal->phiinst->uses().end();) {
    auto puse = *puseIter;
    puseIter++;
    auto userInst = puse->user()->dynCast<ir::Instruction>();
    auto userIdx = puse->index();
    if (lp->blocks().count(userInst->block()) != 0) continue;
    auto lpExit = *lp->exits().begin();  // lp must have only one exit
    if (lpExit != userInst->block()) {
      userInst->setOperand(userIdx, finalVar);
    } else {
      bool isReplace;
      for (auto inst : lpExit->insts()) {
        if (inst == finalVar) {
          isReplace = true;
          break;
        }
        if (inst == userInst) {
          isReplace = false;
          break;
        }
      }
      if (isReplace) {
        userInst->setOperand(userIdx, finalVar);
      }
    }
  }
}

void SCEVContext::visitPhi(ir::Loop* lp, ir::PhiInst* phiinst) {
  if (phiinst->isFloat32()) return;
  std::stack<ir::Instruction*> instStk;
  for (auto puse : phiinst->uses()) {
    auto user = puse->user();
    if (auto inst = user->dynCast<ir::Instruction>()) {
      auto instBlk = inst->block();
      if (lp->blocks().count(instBlk)) instStk.push(inst);  // 只有在循环中的对phi的使用才被插入
    }
  }
  bool isAnalysisOK = false;
  while (not instStk.empty()) {  // 这一段工作表分析是用来展示存在环
    auto inst = instStk.top();
    instStk.pop();
    if (inst == phiinst) {
      isAnalysisOK = true;
      break;
    }
    auto newPhiinst = inst->dynCast<ir::PhiInst>();
    if (newPhiinst != nullptr and newPhiinst != phiinst) {
      break;
    }
    for (auto puse : inst->uses()) {
      auto puser = puse->user();
      auto inst = puser->dynCast<ir::Instruction>();
      if (lp->blocks().count(inst->block())) instStk.push(inst);
    }
  }
  if (not isAnalysisOK) return;
  int res = -1;
  for (auto puse : phiinst->uses()) {
    auto inst = puse->user()->dynCast<ir::BinaryInst>();
    if (inst == nullptr) continue;
    if (inst->valueId() == ir::vADD or inst->valueId() == ir::vFADD or
        inst->valueId() == ir::vSUB or inst->valueId() == ir::vFSUB) {
    } else
      continue;
    if (lp->blocks().count(inst->block()) == 0) continue;
    if (inst != nullptr) {
      binstStk.clear();
      res = findAddSubChain(lp, phiinst, inst);
    }
    if (res == 1) break;
  }
  if (res == 1) {
    getSCEVValue(lp, phiinst, binstStk);
  }
}

int SCEVContext::findAddSubChain(ir::Loop* lp, ir::PhiInst* phiinst, ir::BinaryInst* nowInst) {
  bool isPushStack = false;
  if (nowInst->valueId() == ir::vADD or nowInst->valueId() == ir::vFADD) {
    if (isSimplyLoopInvariant(lp, nowInst->lValue())) {
      binstStk.push_back(nowInst);
    } else if (isSimplyLoopInvariant(lp, nowInst->rValue())) {
      binstStk.push_back(nowInst);
    } else
      return -1;
  } else if (nowInst->valueId() == ir::vSUB or nowInst->valueId() == ir::vFSUB) {
    if (isSimplyLoopInvariant(lp, nowInst->rValue())) {
      binstStk.push_back(nowInst);
    } else
      return -1;
  } else
    return -1;
  for (auto puse : nowInst->uses()) {
    auto inst = puse->user()->dynCast<ir::Instruction>();
    if (inst == nullptr) continue;
    if (inst == phiinst) return 1;
    if (lp->blocks().count(inst->block()) == 0) continue;
    auto binst = inst->dynCast<ir::BinaryInst>();
    if (binst == nullptr) continue;
    auto binstid = binst->valueId();
    if (not(binstid == ir::vADD or binstid == ir::vFADD or binstid == ir::vSUB or
            binstid == ir::vFSUB))
      continue;
    int res = findAddSubChain(lp, phiinst, binst);
    if (res == 1) return res;
  }
  binstStk.pop_back();
  return -1;
}

void SCEVContext::getSCEVValue(ir::Loop* lp,
                               ir::PhiInst* phiinst,
                               std::vector<ir::BinaryInst*>& instsChain) {
  auto initVal = phiinst->getvalfromBB(lp->getLoopPreheader());
  if (not isSimplyLoopInvariant(lp, initVal)) return;
  auto pnewSCEV = new SCEVValue;
  pnewSCEV->initVal = initVal;
  if (initVal->isFloat32()) pnewSCEV->isFloat = true;
  pnewSCEV->phiinst = phiinst;
  for (auto binst : instsChain) {
    // std::cerr << "binst->isFloat32(): " << binst->isFloat32() << std::endl;
    // std::cerr << "pnewSCEV->isFloat: " << pnewSCEV->isFloat << std::endl;
    assert(binst->isFloat32() == pnewSCEV->isFloat and "Different types of SCEV steps!");
    if (binst->valueId() == ir::vADD or binst->valueId() == ir::vFADD) {
      if (isSimplyLoopInvariant(lp, binst->lValue()))
        pnewSCEV->addsteps.push_back(binst->lValue());
      else
        pnewSCEV->addsteps.push_back(binst->rValue());
    } else {  // SUB
      pnewSCEV->substeps.push_back(binst->rValue());
    }
  }
  SCEVValues.push_back(pnewSCEV);
  instsChain.clear();
}

// 简单的判断一下对应的value是不是循环不变量
bool SCEVContext::isSimplyLoopInvariant(ir::Loop* lp, ir::Value* val) {
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

// 在循环外对这个值(phi)有使用这就说明了这个常数是值得被化简计算的
bool SCEVContext::isUsedOutsideLoop(ir::Loop* lp, ir::Value* val) {
  for (auto puse : val->uses()) {
    auto user = puse->user();
    if (auto inst = user->dynCast<ir::Instruction>()) {
      if (lp->blocks().count(inst->block()) == 0) return true;
    }
  }
  return false;
}

// 如果endvar是常数，就直接计算出对应的迭代次数
int SCEVContext::getConstantEndvarIndVarIterCnt(ir::Loop* lp, ir::IndVar* idv) {
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

// 如果不是常数，就要在必要的时候生成计算迭代次数的指令
// return nullptr if cannot calcuate
ir::Value* SCEVContext::addCalcIterCntInstructions(ir::Loop* lp,
                                                   ir::IndVar* idv,
                                                   ir::IRBuilder& builder) {  //-1 for cannot calc
  assert(not idv->isEndVarConst());
  auto beginVar = idv->getBeginI32();
  auto stepVar = idv->getStepI32();
  auto icmpinst = idv->cmpInst();
  auto iterinst = idv->iterInst();
  if (stepVar == 0) return nullptr;
  if (lp->exits().size() != 1) return nullptr;
  auto lpExit = *lp->exits().begin();
  auto endVal = idv->endValue();
  auto beginVal = idv->getBegin();
  auto stepVal = idv->getStep();
  // 对icmp进行标准化
  normalizeIcmpAndBr(lp, idv);
  // 对于不能确定具体cnt的，只生成stepVar==1的情况，否则可以生成所有的情况
  //  builder.set_pos(lpExit,lpExit->insts().begin());
  switch (icmpinst->valueId()) {
    case ir::vIEQ:
      return nullptr;
      break;
    case ir::vINE:
      return nullptr;
      break;
    case ir::vISGT:
      if (iterinst->valueId() == ir::vADD) {
        // if(stepVar>0)return -1;
        // if(endVar>=beginVar)return -1;
        // auto cnt=(endVar-beginVar)/stepVar;
        // if((endVar-beginVar)%stepVar==0)return cnt;
        // else return cnt+1;
        if (stepVar != -1) return nullptr;

        // TODO: makeInst here: sub i32 %beginVal, i32 %endVal;
        return builder.makeBinary(ir::SUB, beginVal, endVal);
      } else if (iterinst->valueId() == ir::vSUB) {
        // if(stepVar<0)return -1;
        // if(beginVar<=endVar)return -1;
        // auto cnt=(beginVar-endVar)/stepVar;
        // if((beginVar-endVar)%stepVar==0)return cnt;
        // else return cnt+1;
        if (stepVar != 1) return nullptr;

        // TODO: makeInst here: sub i32 %beginVal, i32 %endVal;
        return builder.makeBinary(ir::SUB, beginVal, endVal);

      } else if (iterinst->valueId() == ir::vMUL) {
        return nullptr;  // TODO: do not support != with MUL
      } else {
        assert(false and "invalid operator in IndVar!");
      }
      break;
    case ir::vISGE:
      if (iterinst->valueId() == ir::vADD) {
        if (stepVar > 0) return nullptr;
        // if(endVar>=beginVar)return -1;
        // auto cnt=(endVar-beginVar)/stepVar;
        // return cnt+1;

        // TODO: makeInst here: %newVal = sub i32 %beginVal, i32 %endVal;
        // TODO: makeInst here: %newVal2 = sdiv i32 %newVal, i32 %stepVal;
        // TODO: makeInst here: %newVal2 = add i32 %newVal2, 1
        auto newVal1 = builder.makeBinary(ir::SUB, beginVal, endVal);
        ir::Value* newVal2;
        if (stepVar != 1)
          newVal2 = builder.makeBinary(ir::DIV, newVal1, stepVal);
        else
          newVal2 = newVal1;
        auto const1 = ir::ConstantInteger::gen_i32(1);
        return builder.makeBinary(ir::ADD, newVal2, const1);
      } else if (iterinst->valueId() == ir::vSUB) {
        if (stepVar < 0) return nullptr;
        // if(beginVar<=endVar)return -1;
        // auto cnt=(beginVar-endVar)/stepVar;
        // return cnt+1;

        // TODO: makeInst here: %newVal = sub i32 %beginVal, i32 %endVal;
        // TODO: makeInst here: %newVal2 = sdiv i32 %newVal, i32 %stepVal;
        // TODO: makeInst here: %newVal2 = add i32 %newVal2, 1
        ir::Value* newVal2;
        if (stepVar == -1) {
          newVal2 = builder.makeBinary(ir::SUB, endVal, beginVal);
        } else {
          auto newVal1 = builder.makeBinary(ir::SUB, beginVal, endVal);
          newVal2 = builder.makeBinary(ir::DIV, newVal1, stepVal);
        }
        auto const1 = ir::ConstantInteger::gen_i32(1);
        return builder.makeBinary(ir::ADD, newVal2, const1);
      } else if (iterinst->valueId() == ir::vMUL) {
        return nullptr;  // TODO: do not support != with MUL
      } else {
        assert(false and "invalid operator in IndVar!");
      }
      break;
    case ir::vISLT:
      if (iterinst->valueId() == ir::vADD) {
        // if(stepVar<0)return -1;
        // if(endVar<=beginVar)return -1;
        // auto cnt=(endVar-beginVar)/stepVar;
        // if((endVar-beginVar)%stepVar==0)return cnt;
        // else return cnt+1;
        if (stepVar != 1) return nullptr;

        // TODO: makeInst here: sub i32 %endVal, i32 %beginVal;
        return builder.makeBinary(ir::SUB, endVal, beginVal);
      } else if (iterinst->valueId() == ir::vSUB) {
        // if(stepVar>0)return -1;
        // if(beginVar<=endVar)return -1;
        // auto cnt=(beginVar-endVar)/stepVar;
        // if((beginVar-endVar)%stepVar==0)return cnt;
        // else return cnt+1;
        if (stepVar != -1) return nullptr;

        // TODO: makeInst here: sub i32 %endVal, i32 %beginVal;
        return builder.makeBinary(ir::SUB, endVal, beginVal);
      } else if (iterinst->valueId() == ir::vMUL) {
        return nullptr;  // TODO: do not support != with MUL
      } else {
        assert(false and "invalid operator in IndVar!");
      }
      break;
    case ir::vISLE:
      if (iterinst->valueId() == ir::vADD) {
        if (stepVar < 0) return nullptr;
        // if(endVar<=beginVar)return -1;
        // auto cnt=(endVar-beginVar)/stepVar;
        // return cnt+1;

        // TODO: makeInst here: %newVal = sub i32 %endVal, i32 %beginVal;
        // TODO: makeInst here: %newVal2 = sdiv i32 %newVal, i32 %stepVal;
        // TODO: makeInst here: %newVal2 = add i32 %newVal2, 1
        ir::Value* newVal2;
        auto newVal1 = builder.makeBinary(ir::SUB, endVal, beginVal);
        if (stepVar == 1)
          newVal2 = newVal1;
        else
          newVal2 = builder.makeBinary(ir::DIV, newVal1, stepVal);
        auto const1 = ir::ConstantInteger::gen_i32(1);
        return builder.makeBinary(ir::ADD, newVal2, const1);

      } else if (iterinst->valueId() == ir::vSUB) {
        if (stepVar > 0) return nullptr;
        // if(beginVar<=endVar)return -1;
        // auto cnt=(beginVar-endVar)/stepVar;
        // return cnt+1;

        // TODO: makeInst here: %newVal = sub i32 %beginVal, i32 %endVal;
        // TODO: makeInst here: %newVal2 = sdiv i32 %newVal, i32 %stepVal;
        // TODO: makeInst here: %newVal2 = add i32 %newVal2, 1
        ir::Value* newVal2;
        if (stepVar == -1) {
          newVal2 = builder.makeBinary(ir::SUB, endVal, beginVal);
        } else {
          auto newVal1 = builder.makeBinary(ir::SUB, beginVal, endVal);
          newVal2 = builder.makeBinary(ir::DIV, newVal1, stepVal);
        }
        auto const1 = ir::ConstantInteger::gen_i32(1);
        return builder.makeBinary(ir::ADD, newVal2, const1);

      } else if (iterinst->valueId() == ir::vMUL) {
        return nullptr;  // TODO: do not support != with MUL
      } else {
        assert(false and "invalid operator in IndVar!");
      }
      break;
    default:
      break;
  }
  assert(false and "something error happened in func addCalCntInstuctions ");
  return nullptr;
}

// 标准化:把idv放在op1 把endvar放在op2,icmp true就保持循环,false就跳出
void SCEVContext::normalizeIcmpAndBr(ir::Loop* lp, ir::IndVar* idv) {
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
void SCEVContext::exchangeIcmpOp(ir::ICmpInst* icmpInst) {
  auto LHS = icmpInst->lhs();
  auto RHS = icmpInst->rhs();
  // 改变ValueId
  reverseIcmpOp(icmpInst);
  // 交换op
  icmpInst->setOperand(0, RHS);
  icmpInst->setOperand(1, LHS);
}

// 翻转这个Icmp的符号使得原意不变
void SCEVContext::reverseIcmpOp(ir::ICmpInst* icmpInst) {
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
void SCEVContext::exchangeBrDest(ir::BranchInst* brInst) {
  assert(brInst->is_cond());
  auto trueTarget = brInst->iftrue();
  auto falseTarget = brInst->iffalse();
  brInst->set_iftrue(falseTarget);
  brInst->set_iffalse(trueTarget);
}

bool SCEVContext::isSimplyNotInLoop(ir::Loop* lp, ir::Value* val) {
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