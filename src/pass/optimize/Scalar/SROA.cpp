#include "pass/analysis/dependenceAnalysis/DependenceAnalysis.hpp"
#include "pass/optimize/SROA.hpp"
#include "pass/optimize/mem2reg.hpp"
using namespace pass;

static std::unordered_set<ir::GetElementPtrInst*> processedSubAddrs;

// Scalar Replacement of Aggregates
// SROA基本逻辑：
/*
处理的是对于循环内的变量
对于所有的循环内内存存取
1. 只考虑数组，全局由AG2L来考虑
2. 对于循环不变量，直接创建临时变量进行修改(从外到内，尽可能在外层做，将load-store紧贴在对应gep之后)
3. 对于当前循环内的量，只要没有possiblySame，都可以进行替换，从外到内即可
*/
void SROA::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  SROAContext sroaContext;
  sroaContext.run(func, tp);
}
void SROAContext::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  if (func->isOnlyDeclare()) return;
  dpctx = tp->getDepInfo(func);
  domctx = tp->getDomTreeWithoutRefresh(func);
  idvctx = tp->getIndVarInfoWithoutRefresh(func);
  lpctx = tp->getLoopInfoWithoutRefresh(func);
  sectx = tp->getSideEffectInfoWithoutRefresh();
  processedSubAddrs.clear();
  for (auto lp : lpctx->loops()) {
    if (lp->parentloop() != nullptr) continue;
    runOnLoop(lp);
  }
  Mem2Reg m2r = Mem2Reg();
  // m2r.run(func,tp);
}

void SROAContext::runOnLoop(ir::Loop* lp) {
  auto idv = idvctx->getIndvar(lp);

  depInfoForLp = dpctx->getLoopDependenceInfo(lp);
  // 对当前循环一对对subAddr进行分析，对于有重复的将重复的加入到集合中并不再处理
  std::unordered_set<ir::GetElementPtrInst*> hasRepGeps;
  std::unordered_set<ir::GetElementPtrInst*> SROAGeps;
  // 主要是看迭代内是否完全不一致
  for (auto bd : depInfoForLp->getBaseAddrs()) {
    auto& subAddrs = depInfoForLp->baseAddrToSubAddrSet(bd);
    for (auto setIter = subAddrs.begin(); setIter != subAddrs.end(); setIter++) {
      // 如果当前的subAddr已经被处理了，就不再处理continue
      if (hasRepGeps.count(*setIter) or SROAGeps.count(*setIter)) continue;
      if (processedSubAddrs.count(*setIter)) continue;
      bool isCurIterIndependent = true;
      setIter++;
      auto setIter2Begin = setIter;
      setIter--;
      for (auto setIter2 = setIter2Begin; setIter2 != subAddrs.end(); setIter2++) {
        if (processedSubAddrs.count(*setIter2)) continue;
        auto gepIdx1 = depInfoForLp->getGepIdx(*setIter);
        auto gepIdx2 = depInfoForLp->getGepIdx(*setIter2);
        auto outputDep = isTwoGepIdxPossiblySame(gepIdx1, gepIdx2, lp, idv);
        if (outputDep & dTotallyNotSame == 0) {
          hasRepGeps.insert(*setIter);
          hasRepGeps.insert(*setIter2);
          isCurIterIndependent = false;
        }
      }
      // 如果与其他的在当前循环迭代均不一样，就可以进行Sroa
      int memOpCnt = depInfoForLp->getSubAddrInsts(*setIter).size();
      if (memOpCnt < 3) continue;  // 循环体内存取小于三个的就直接不管
      if (isCurIterIndependent) SROAGeps.insert(*setIter);
    }
  }
  for (auto gep : SROAGeps) {
    bool res;
    if (domctx->dominate(gep->block(), lp->header()) and gep->block() != lp->header()) {
      auto newAlloca = createNewLocal(gep->baseType(), gep->block()->function());
      bool isReadGep = depInfoForLp->getIsSubAddrRead(gep);
      bool isWriteGep = depInfoForLp->getIsSubAddrWrite(gep);
      res = replaceAllUseInLpForLpI(gep, lp, newAlloca, isReadGep and not isWriteGep,
                                    isWriteGep and not isReadGep);
      if (res) {
        processedSubAddrs.insert(gep);
        std::cerr << "SROA lift" << std::endl;
      }
    } else {
      auto lpLatch = *lp->latchs().begin();
      if (!domctx->dominate(gep->block(), lpLatch)) continue;
      auto newAlloca = createNewLocal(gep->baseType(), gep->block()->function());
      bool isReadGep = depInfoForLp->getIsSubAddrRead(gep);
      bool isWriteGep = depInfoForLp->getIsSubAddrWrite(gep);
      res = replaceAllUseInLpIdv(gep, lp, newAlloca, isReadGep and not isWriteGep,
                                 isWriteGep and not isReadGep);
      if (res) {
        processedSubAddrs.insert(gep);
        std::cerr << "SROA lift" << std::endl;
      }
    }
  }

  // 从外到内
  for (auto subLoop : lp->subLoops()) {
    runOnLoop(subLoop);
  }
}

ir::AllocaInst* SROAContext::createNewLocal(ir::Type* allocaType, ir::Function* func) {
  // 创建一个新的局部变量（alloca）
  auto funcEntry = func->entry();
  auto newAlloca = new ir::AllocaInst(allocaType, false, funcEntry);
  funcEntry->emplace_lastbutone_inst(newAlloca);
  return newAlloca;
}

// 对迭代内起作用,如果是真正的LpI指针,还有另外的做法
bool SROAContext::replaceAllUseInLpIdv(ir::GetElementPtrInst* gep,
                                       ir::Loop* lp,
                                       ir::AllocaInst* newAlloca,
                                       bool isOnlyRead,
                                       bool isOnlyWrite) {
  auto gepBB = gep->block();
  auto gepPos = std::find(gepBB->insts().begin(), gepBB->insts().end(), gep);
  gepPos++;
  if (isOnlyRead) {
    // 将gep load出来,store到alloca中，然后将所有的lp中的load替代之
    for (auto puseIter = gep->uses().begin(); puseIter != gep->uses().end();) {
      auto puse = *puseIter;
      puseIter++;
      auto puser = puse->user();
      auto puseIdx = puse->index();
      auto userInst = puser->dynCast<ir::Instruction>();
      if (lp->blocks().count(userInst->block())) {
        userInst->setOperand(puseIdx, newAlloca);
      }
    }
    auto gepLoad = new ir::LoadInst(gep, gep->baseType(), gepBB);
    auto storeToAlloca = new ir::StoreInst(gepLoad, newAlloca, gepBB);
    gepBB->emplace_inst(gepPos, gepLoad);
    gepBB->emplace_inst(gepPos, storeToAlloca);
    return true;
  } else if (isOnlyWrite) {
    auto lpLatch = *lp->latchs().begin();
    if (lp->latchs().size() > 1) return false;
    // 直接将所有lp中的store替换成给alloca store，最后load一下alloca，然后store到gep（循环迭代末尾）
    for (auto puseIter = gep->uses().begin(); puseIter != gep->uses().end();) {
      auto puse = *puseIter;
      puseIter++;
      auto puser = puse->user();
      auto puseIdx = puse->index();
      auto userInst = puser->dynCast<ir::Instruction>();
      if (lp->blocks().count(userInst->block())) {
        userInst->setOperand(puseIdx, newAlloca);
      }
    }

    auto loadAlloca = new ir::LoadInst(newAlloca, newAlloca->baseType(), lpLatch);
    auto storeLoadToGep = new ir::StoreInst(loadAlloca, gep, lpLatch);
    lpLatch->emplace_lastbutone_inst(loadAlloca);
    lpLatch->emplace_lastbutone_inst(storeLoadToGep);
    return true;
  } else {
    auto lpLatch = *lp->latchs().begin();
    if (lp->latchs().size() > 1) return false;
    // 将所有对其的读写变为对alloca的读写
    for (auto puseIter = gep->uses().begin(); puseIter != gep->uses().end();) {
      auto puse = *puseIter;
      puseIter++;
      auto puser = puse->user();
      auto puseIdx = puse->index();
      auto userInst = puser->dynCast<ir::Instruction>();
      if (lp->blocks().count(userInst->block())) {
        userInst->setOperand(puseIdx, newAlloca);
      }
    }
    // 将gep load出来,store到alloca中
    auto gepLoad = new ir::LoadInst(gep, gep->baseType(), gepBB);
    auto storeToAlloca = new ir::StoreInst(gepLoad, newAlloca, gepBB);
    gepBB->emplace_inst(gepPos, gepLoad);
    gepBB->emplace_inst(gepPos, storeToAlloca);
    // 将alloca load出来，将其值store进去

    auto loadAlloca = new ir::LoadInst(newAlloca, newAlloca->baseType(), lpLatch);
    auto storeLoadToGep = new ir::StoreInst(loadAlloca, gep, lpLatch);
    lpLatch->emplace_lastbutone_inst(loadAlloca);
    lpLatch->emplace_lastbutone_inst(storeLoadToGep);
    return true;
  }
}

bool SROAContext::replaceAllUseInLpForLpI(ir::GetElementPtrInst* gep,
                                          ir::Loop* lp,
                                          ir::AllocaInst* newAlloca,
                                          bool isOnlyRead,
                                          bool isOnlyWrite) {
  auto gepBB = gep->block();
  auto gepPos = std::find(gepBB->insts().begin(), gepBB->insts().end(), gep);
  gepPos++;
  if (isOnlyRead) {
    // 将gep load出来,store到alloca中，然后将所有的lp中的load替代之
    for (auto puseIter = gep->uses().begin(); puseIter != gep->uses().end();) {
      auto puse = *puseIter;
      puseIter++;
      auto puser = puse->user();
      auto puseIdx = puse->index();
      auto userInst = puser->dynCast<ir::Instruction>();
      if (lp->blocks().count(userInst->block())) {
        userInst->setOperand(puseIdx, newAlloca);
      }
    }
    auto gepLoad = new ir::LoadInst(gep, gep->baseType(), gepBB);
    auto storeToAlloca = new ir::StoreInst(gepLoad, newAlloca, gepBB);
    gepBB->emplace_inst(gepPos, gepLoad);
    gepBB->emplace_inst(gepPos, storeToAlloca);
    return true;
  } else if (isOnlyWrite) {
    auto lpExit = *lp->exits().begin();
    if (lp->exits().size() > 1) return false;
    // 直接将所有lp中的store替换成给alloca store，最后load一下alloca，然后store到gep（循环迭代末尾）
    for (auto puseIter = gep->uses().begin(); puseIter != gep->uses().end();) {
      auto puse = *puseIter;
      puseIter++;
      auto puser = puse->user();
      auto puseIdx = puse->index();
      auto userInst = puser->dynCast<ir::Instruction>();
      if (lp->blocks().count(userInst->block())) {
        userInst->setOperand(puseIdx, newAlloca);
      }
    }

    auto loadAlloca = new ir::LoadInst(newAlloca, newAlloca->baseType(), lpExit);
    auto storeLoadToGep = new ir::StoreInst(loadAlloca, gep, lpExit);
    lpExit->emplace_lastbutone_inst(loadAlloca);
    lpExit->emplace_lastbutone_inst(storeLoadToGep);
    return true;
  } else {
    auto lpExit = *lp->exits().begin();
    if (lp->exits().size() > 1) return false;
    // 将所有对其的读写变为对alloca的读写
    for (auto puseIter = gep->uses().begin(); puseIter != gep->uses().end();) {
      auto puse = *puseIter;
      puseIter++;
      auto puser = puse->user();
      auto puseIdx = puse->index();
      auto userInst = puser->dynCast<ir::Instruction>();
      if (lp->blocks().count(userInst->block())) {
        userInst->setOperand(puseIdx, newAlloca);
      }
    }
    // 将gep load出来,store到alloca中
    auto gepLoad = new ir::LoadInst(gep, gep->baseType(), gepBB);
    auto storeToAlloca = new ir::StoreInst(gepLoad, newAlloca, gepBB);
    gepBB->emplace_inst(gepPos, gepLoad);
    gepBB->emplace_inst(gepPos, storeToAlloca);
    // 将alloca load出来，将其值store进去
    auto loadAlloca = new ir::LoadInst(newAlloca, newAlloca->baseType(), lpExit);
    auto storeLoadToGep = new ir::StoreInst(loadAlloca, gep, lpExit);
    lpExit->emplace_first_inst(loadAlloca);
    lpExit->emplace_first_inst(storeLoadToGep);
    return true;
  }
}

int SROAContext::isTwoGepIdxPossiblySame(GepIdx* gepidx1,
                                         GepIdx* gepidx2,
                                         ir::Loop* lp,
                                         ir::IndVar* idv) {
  std::vector<DependenceType> compareAns;
  size_t lim = gepidx1->idxList.size();
  int res = 0;
  for (size_t i = 0; i < lim; i++) {
    auto val1 = gepidx1->idxList.at(i);
    auto val2 = gepidx2->idxList.at(i);
    auto type1 = gepidx1->idxTypes[val1];
    auto type2 = gepidx2->idxTypes[val2];
    int outputDepInfo = isTwoIdxPossiblySame(val1, val2, type1, type2, lp, idv);
    res = res | outputDepInfo;
  }
  return res;
}

int SROAContext::isTwoIdxPossiblySame(ir::Value* val1,
                                      ir::Value* val2,
                                      IdxType type1,
                                      IdxType type2,
                                      ir::Loop* lp,
                                      ir::IndVar* idv) {
  if (val1 == val2) {  // 自己跟自己进行比较
    switch (type1) {
      case iLOOPINVARIANT:
        return dTotallySame | dCrossIterTotallySame;
        break;
      case iCALL:
        return dTotallySame | dCrossIterPossiblySame;
        break;
      case iIDV:
        return dTotallySame | dCrossIterTotallyNotSame;
        break;
      case iIDVPLUSMINUSFORMULA:
        return dTotallySame | dCrossIterTotallyNotSame;
        break;
      case iINNERIDV:
        return dTotallySame | dCrossIterPossiblySame;
        break;
      case iINNERIDVPLUSMINUSFORMULA:
        return dTotallySame | dCrossIterPossiblySame;
        break;
      case iLOAD:
        return dTotallySame | dCrossIterPossiblySame;  // TODO, 依赖于副作用等分析
        break;
      case iELSE:
        return dPossiblySame | dCrossIterPossiblySame;
        break;
      default:
        break;
    }
  }
  if (type1 == type2) {
    switch (type1) {
      case iCALL: {
        auto callInst1 = val1->dynCast<ir::CallInst>();
        auto callInst2 = val2->dynCast<ir::CallInst>();
        auto callFunc1 = callInst1->callee();
        auto callFunc2 = callInst2->callee();
        if (callFunc1 != callFunc2) return dPossiblySame | dCrossIterPossiblySame;
        // TODO:副作用分析判断这里是纯函数
        if (sectx->isPureFunc(callFunc1))
          return dTotallySame | dCrossIterPossiblySame;
        else
          return dPossiblySame | dCrossIterPossiblySame;
        break;
      }

      case iLOOPINVARIANT: {
        auto constval1 = val1->dynCast<ir::ConstantInteger>();
        auto constval2 = val2->dynCast<ir::ConstantInteger>();
        if (constval1 != nullptr and constval2 != nullptr) {
          if (constval1->i32() == constval2->i32())
            return dTotallySame | dCrossIterTotallySame;
          else
            return dTotallyNotSame | dCrossIterTotallyNotSame;
        } else {
          return dPossiblySame | dCrossIterPossiblySame;
        }
        break;
      }

      case iIDV: {
        if (val1 != val2) {
          assert(false and "Error: indvar in a same loop is not same!");
        }
        return dTotallySame | dCrossIterTotallyNotSame;
        break;
      }

      case iIDVPLUSMINUSFORMULA: {
        std::unordered_set<ir::Value*> val1Add;
        std::unordered_set<ir::Value*> val1Sub;
        auto curVal1 = val1;
        while (curVal1 != idv->phiinst()) {
          if (auto BInst = curVal1->dynCast<ir::BinaryInst>()) {
            auto lval = BInst->lValue();
            auto rval = BInst->rValue();
            ir::Value* LpIVal;
            bool isLVAL = false;
            if (isSimplyLoopInvariant(lp, lval)) {
              LpIVal = lval;
              curVal1 = rval;
              isLVAL = true;
            } else if (isSimplyLoopInvariant(lp, rval)) {
              LpIVal = rval;
              curVal1 = lval;
            } else {
              assert(false and "Error:GepIdx is not IDVPLUSMINUSFORMULA!");
            }
            if (BInst->valueId() == ir::vADD) {
              val1Add.insert(LpIVal);
            } else if (BInst->valueId() == ir::vSUB) {
              if (isLVAL) {
                assert(false and "Error:GepIdx is a-i formula!");
              }
              val1Sub.insert(LpIVal);
            }
          } else {
            assert(false and "this is not a idvplusminus formula!");
          }
        }
        auto curVal2 = val2;
        while (curVal2 != idv->phiinst()) {
          if (auto BInst = curVal2->dynCast<ir::BinaryInst>()) {
            auto lval = BInst->lValue();
            auto rval = BInst->rValue();
            ir::Value* LpIVal;
            bool isLVAL = false;
            if (isSimplyLoopInvariant(lp, lval)) {
              LpIVal = lval;
              curVal1 = rval;
              isLVAL = true;
            } else if (isSimplyLoopInvariant(lp, rval)) {
              LpIVal = rval;
              curVal1 = lval;
            } else {
              assert(false and "Error:GepIdx is not IDVPLUSMINUSFORMULA!");
            }
            if (BInst->valueId() == ir::vADD) {
              if (val1Add.count(LpIVal)) {
                val1Add.erase(LpIVal);
              } else {
                return dPossiblySame | dCrossIterPossiblySame;
              }
            } else if (BInst->valueId() == ir::vSUB) {
              if (isLVAL) {
                assert(false and "Error:GepIdx is a-i formula!");
              }
              if (val1Sub.count(LpIVal)) {
                val1Sub.erase(LpIVal);
              } else {
                return dPossiblySame | dCrossIterPossiblySame;
              }
            }
          } else {
            assert(false and "this is not a idvplusminus formula!");
          }
        }
        return dTotallySame | dCrossIterTotallySame;

        break;
      }

      case iINNERIDV: {
        return dPossiblySame | dCrossIterPossiblySame;
      }

      case iINNERIDVPLUSMINUSFORMULA: {
        return dPossiblySame | dCrossIterPossiblySame;
      }

      case iLOAD: {
        auto loadInst1 = val1->dynCast<ir::LoadInst>();
        auto loadInst2 = val2->dynCast<ir::LoadInst>();
        auto ptr1 = loadInst1->ptr();
        auto ptr2 = loadInst2->ptr();
        if (ptr1 != ptr2) {
          return dPossiblySame | dCrossIterPossiblySame;
        } else {
          return dPossiblySame | dCrossIterPossiblySame;  // 进行副作用分析可以进一步细化
        }
      }

      case iELSE: {
        return dPossiblySame | dCrossIterPossiblySame;
      }

      default:
        break;
    }
  }
  if ((type1 == iIDV and type2 == iIDVPLUSMINUSFORMULA) or
      (type1 == iIDVPLUSMINUSFORMULA and type2 == iIDV)) {
    return dTotallyNotSame | dCrossIterPossiblySame;
  }
  return dPossiblySame | dCrossIterPossiblySame;
}

bool SROAContext::isSimplyLoopInvariant(ir::Loop* lp, ir::Value* val) {
  if (auto constVal = val->dynCast<ir::ConstantValue>()) return true;  // 常数
  if (auto argVal = val->dynCast<ir::Argument>()) return true;         // 参数
  if (auto instVal = val->dynCast<ir::Instruction>()) {
    return domctx->dominate(instVal->block(), lp->header()) and instVal->block() != lp->header();
  }
  return false;
}