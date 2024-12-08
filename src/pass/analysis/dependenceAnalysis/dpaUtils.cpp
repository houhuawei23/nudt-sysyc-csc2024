#include "pass/analysis/dependenceAnalysis/DependenceAnalysis.hpp"
#include "pass/analysis/dependenceAnalysis/dpaUtils.hpp"

using namespace pass;
// 初始化lpDepInfo
void LoopDependenceInfo::makeLoopDepInfo(ir::Loop* lp, TopAnalysisInfoManager* topmana) {
  parent = lp;
  tp = topmana;
  // 遍历所有语句
  for (auto bb : lp->blocks()) {
    for (auto inst : bb->insts()) {
      ir::Value* baseAddr;
      if (auto ldInst = inst->dynCast<ir::LoadInst>()) {
        addPtr(ldInst->ptr(), ldInst);
      } else if (auto stInst = inst->dynCast<ir::StoreInst>()) {
        addPtr(stInst->ptr(), stInst);
      }
    }
  }
}

void LoopDependenceInfo::getInfoFromSubLoop(ir::Loop* subLoop, LoopDependenceInfo* subLoopDepInfo) {
  for (auto memInst : subLoopDepInfo->memInsts) {
    if (auto ldInst = memInst->dynCast<ir::LoadInst>()) {
      addPtrFromSubLoop(ldInst->ptr(), ldInst, subLoopDepInfo);
    } else if (auto stInst = memInst->dynCast<ir::StoreInst>()) {
      addPtrFromSubLoop(stInst->ptr(), stInst, subLoopDepInfo);
    } else {
      std::cerr << "error in function getInfoFromSubLoop!" << std::endl;
      assert(false);
    }
  }
}

ir::Value* pass::getIntToPtrBaseAddr(ir::UnaryInst* inst) {
  if (auto binary = inst->value()->dynCast<ir::BinaryInst>()) {
    // if(auto lbase = getBaseAddr(binary->lValue())) return lbase;
    // if(auto rbase = getBaseAddr(binary->rValue())) return rbase;
    if (binary->lValue()->valueId() == ir::ValueId::vPTRTOINT) {
      return binary->lValue()->dynCast<ir::UnaryInst>()->value();
    } else if (binary->rValue()->valueId() == ir::ValueId::vPTRTOINT) {
      return binary->rValue()->dynCast<ir::UnaryInst>()->value();
    }
  }
  std::cerr << "getIntToPtrBaseAddr!" << std::endl;
  assert(false);
  return nullptr;
}

ir::Value* pass::getBaseAddr(ir::Value* subAddr, TopAnalysisInfoManager* topmana) {
  if (auto allocainst = subAddr->dynCast<ir::AllocaInst>()) return allocainst;
  if (auto gv = subAddr->dynCast<ir::GlobalVariable>()) return gv;
  if (auto arg = subAddr->dynCast<ir::Argument>()) return arg;
  if (auto gep = subAddr->dynCast<ir::GetElementPtrInst>())
    return getBaseAddr(gep->value(), topmana);
  if (auto phi = subAddr->dynCast<ir::PhiInst>()) {
    auto func = phi->block()->function();
    auto lpctx = topmana->getLoopInfoWithoutRefresh(func);
    auto lp = lpctx->head2loop(phi->block());
    auto preHeaderVal = phi->getvalfromBB(lp->getLoopPreheader());
    return getBaseAddr(preHeaderVal, topmana);
  }
  if (auto unary = subAddr->dynCast<ir::UnaryInst>()) {
    if (unary->valueId() == ir::ValueId::vINTTOPTR) {
      return getIntToPtrBaseAddr(unary);
    }
  }
  // assert("Error! invalid type of input in function getBaseAddr!"&&false);
  return nullptr;
}

// 取出基址的类型
BaseAddrType pass::getBaseAddrType(ir::Value* val) {
  if (val->dynCast<ir::GlobalVariable>()) return globalType;
  if (val->dynCast<ir::Argument>()) return argType;
  if (val->dynCast<ir::AllocaInst>()) return localType;
  std::cerr << "invalid input in function getBaseAddrTtype!" << std::endl;
  assert(false);
}

// 加入单个ptr的接口
void LoopDependenceInfo::addPtr(ir::Value* ptr, ir::Instruction* inst) {
  if (memInsts.count(inst) != 0) return;
  auto baseAddr = getBaseAddr(ptr, tp);
  auto subAddr = ptr->dynCast<ir::GetElementPtrInst>();
  if (subAddr == nullptr) return;  // 这实际上排除全局变量
  if (inst->dynCast<ir::LoadInst>() != nullptr and inst->dynCast<ir::StoreInst>() != nullptr)
    return;
  // 基址
  if (baseAddrs.count(baseAddr) == 0) {
    baseAddrIsRead[baseAddr] = false;
    baseAddrIsWrite[baseAddr] = false;
  }
  baseAddrs.insert(baseAddr);
  if (inst->valueId() == ir::vLOAD) {
    baseAddrIsRead[baseAddr] = true;
  } else if (inst->valueId() == ir::vSTORE) {
    baseAddrIsWrite[baseAddr] = true;
  } else {
    std::cerr << "error in functionaddPtr, invalid input inst type!" << std::endl;
    assert(false);
  }
  // 子地址
  if (baseAddrToSubAddrs[baseAddr].count(subAddr) == 0) {
    subAddrIsRead[subAddr] = false;
    subAddrIsRead[subAddr] = false;
  }
  baseAddrToSubAddrs[baseAddr].insert(subAddr);
  if (inst->valueId() == ir::vLOAD) {
    subAddrIsRead[subAddr] = true;
  } else if (inst->valueId() == ir::vSTORE) {
    subAddrIsWrite[subAddr] = true;
  } else {
    std::cerr << "error in functionaddPtr, invalid input inst type!" << std::endl;
    assert(false);
  }
  if (subAddrToGepIdx.count(subAddr) == 0) {
    auto curFunc = subAddr->block()->function();
    constexpr bool Debug = false;
    int curIdx = 0;
    auto pnewGepIdx = new GepIdx;
    auto curSubAddr = subAddr;
    pnewGepIdx->idxList.push_back(nullptr);
    ir::Value* index = nullptr;
    if (Debug) {
      std::cerr << "===================\n";
    }
    while (curSubAddr != nullptr) {
      if (Debug) {
        curSubAddr->print(std::cerr);
        std::cerr << "\n";
      }
      if (curSubAddr->getid() == 0) {
        if (index) {
          auto new_index = curSubAddr->index();
          assert(new_index->isa<ir::ConstantInteger>());
          assert(index->isa<ir::ConstantInteger>());
          auto cons_index = index->dynCast<ir::ConstantValue>();
          auto cons_new_index = new_index->dynCast<ir::ConstantValue>();
          index = ir::ConstantInteger::gen_i32(cons_index->i32() + cons_new_index->i32());
        } else {
          index = curSubAddr->index();
        }
      } else {
        auto new_index = curSubAddr->index();
        if (index) {
          assert(new_index->isa<ir::ConstantInteger>());
          auto cons_new_index = new_index->dynCast<ir::ConstantValue>();
          if (cons_new_index->i32() != 0) {
            assert(index->isa<ir::ConstantInteger>());
            auto cons_index = index->dynCast<ir::ConstantValue>();
            index = ir::ConstantInteger::gen_i32(cons_index->i32() + cons_new_index->i32());
          }
          pnewGepIdx->idxList.push_back(index);
          index = nullptr;
        } else {
          pnewGepIdx->idxList.push_back(new_index);
        }
      }

      auto base = curSubAddr->value();
      if (base->isa<ir::GetElementPtrInst>()) {
        curSubAddr = base->dynCast<ir::GetElementPtrInst>();
      } else if (base->dynCast<ir::PhiInst>()) {
        auto lpctx = tp->getLoopInfoWithoutRefresh(curFunc);
        auto phiBase = base->dynCast<ir::PhiInst>();
        auto phiBaseBB = phiBase->block();
        auto phiBaseLp = lpctx->head2loop(phiBaseBB);
        auto phiBaseLpPreHeader = phiBaseLp->getLoopPreheader();
        curSubAddr = phiBase->getvalfromBB(phiBaseLpPreHeader)->dynCast<ir::GetElementPtrInst>();

      } else {
        curSubAddr = nullptr;
      }
    }
    if (index) {
      pnewGepIdx->idxList.push_back(index);
    }

    std::reverse(pnewGepIdx->idxList.begin(), pnewGepIdx->idxList.end());
    for (auto idxVal : pnewGepIdx->idxList) {
      pnewGepIdx->idxTypes[idxVal] = iELSE;  // 初始值都是ELSE
    }
    subAddrToGepIdx[subAddr] = pnewGepIdx;
  }
  // 具体语句
  subAddrToInst[subAddr].insert(inst);
  memInsts.insert(inst);
}

void LoopDependenceInfo::addPtrFromSubLoop(ir::Value* ptr,
                                           ir::Instruction* inst,
                                           LoopDependenceInfo* subLoopDepInfo) {
  // 基地址
  if (memInsts.count(inst) != 0) return;
  auto baseAddr = getBaseAddr(ptr, tp);
  auto subAddr = ptr->dynCast<ir::GetElementPtrInst>();
  if (subAddr == nullptr) return;  // 这实际上排除全局变量
  if (inst->dynCast<ir::LoadInst>() != nullptr and inst->dynCast<ir::StoreInst>() != nullptr)
    return;
  // 基址
  if (baseAddrs.count(baseAddr) == 0) {
    baseAddrIsRead[baseAddr] = false;
    baseAddrIsWrite[baseAddr] = false;
  }
  baseAddrs.insert(baseAddr);
  if (inst->valueId() == ir::vLOAD) {
    baseAddrIsRead[baseAddr] = true;
  } else if (inst->valueId() == ir::vSTORE) {
    baseAddrIsWrite[baseAddr] = true;
  } else {
    std::cerr << "error in functionaddPtr, invalid input inst type!" << std::endl;
    assert(false);
  }
  // 子地址
  if (baseAddrToSubAddrs[baseAddr].count(subAddr) == 0) {
    subAddrIsRead[subAddr] = false;
    subAddrIsRead[subAddr] = false;
  }
  baseAddrToSubAddrs[baseAddr].insert(subAddr);
  if (inst->valueId() == ir::vLOAD) {
    subAddrIsRead[subAddr] = true;
  } else if (inst->valueId() == ir::vSTORE) {
    subAddrIsWrite[subAddr] = true;
  } else {
    std::cerr << "error in functionaddPtr, invalid input inst type!" << std::endl;
    assert(false);
  }
  if (subAddrToGepIdx.count(subAddr) == 0) {
    auto oldGepIdx = subLoopDepInfo->getGepIdx(subAddr);
    auto pnewGepIdx = new GepIdx;
    for (auto indexValOld : oldGepIdx->idxList) {
      pnewGepIdx->idxList.push_back(indexValOld);
      auto valType = oldGepIdx->idxTypes[indexValOld];
      if (valType != iIDV and valType != iIDVPLUSMINUSFORMULA and valType != iLOOPINVARIANT)
        pnewGepIdx->idxTypes[indexValOld] = valType;
      else if (valType == iIDV)
        pnewGepIdx->idxTypes[indexValOld] = iINNERIDV;
      else if (valType == iIDVPLUSMINUSFORMULA)
        pnewGepIdx->idxTypes[indexValOld] = iINNERIDVPLUSMINUSFORMULA;
      else if (valType == iLOOPINVARIANT)
        pnewGepIdx->idxTypes[indexValOld] = iELSE;
    }
    subAddrToGepIdx[subAddr] = pnewGepIdx;
  }
  subAddrToInst[subAddr].insert(inst);
  memInsts.insert(inst);
}

bool pass::isTwoBaseAddrPossiblySame(ir::Value* ptr1,
                                     ir::Value* ptr2,
                                     ir::Function* func,
                                     CallGraph* cgctx,
                                     TopAnalysisInfoManager* tp) {
  auto type1 = getBaseAddrType(ptr1);
  auto type2 = getBaseAddrType(ptr2);
  if (type1 == type2) {
    if (type1 == globalType) {
      return ptr1 == ptr2;
    } else if (type1 == localType) {
      return ptr1 == ptr2;
    } else {  // 分辨两个arg是否一致
      auto arg1 = ptr1->dynCast<ir::Argument>();
      auto arg2 = ptr2->dynCast<ir::Argument>();
      auto idx1 = arg1->index();
      auto idx2 = arg2->index();
      for (auto callInst : cgctx->callerCallInsts(func)) {
        auto rarg1 = callInst->rargs()[idx1]->value();
        auto rarg2 = callInst->rargs()[idx2]->value();
        if (getBaseAddr(rarg1, tp) != getBaseAddr(rarg2, tp))
          continue;
        else
          return true;  // 简单的认为他们一致
      }
      return false;
    }
  } else {
    if (type1 != argType and type2 != argType) {
      return false;
    } else {
      if (type1 == localType or type2 == localType) return false;
      ir::GlobalVariable* gv;
      ir::Argument* arg;
      if (type1 == globalType) {
        gv = ptr1->dynCast<ir::GlobalVariable>();
        arg = ptr2->dynCast<ir::Argument>();
      } else {
        gv = ptr2->dynCast<ir::GlobalVariable>();
        arg = ptr1->dynCast<ir::Argument>();
      }
      auto idx = arg->index();
      for (auto callinst : cgctx->callerCallInsts(func)) {
        auto rarg = callinst->rargs()[idx]->value();
        auto rargBaseAddr = getBaseAddr(rarg, tp);
        if (rargBaseAddr != gv)
          continue;
        else
          return true;
      }
      return false;
    }
  }
  std::cerr << "error occur in function isTwoBaseAddrPossiblySame" << std::endl;
  assert(false);
  return true;
}

void LoopDependenceInfo::print(std::ostream& os) {
  using namespace std;
  os << "In function " << parent->header()->function()->name() << ":" << endl;
  os << "In loop whose header is" << parent->header()->name() << ":\n";
  if (baseAddrs.empty()) {
    os << "No mem read or write." << endl << endl;
    return;
  }
  os << "Base addrs:" << endl;
  for (auto baseaddr : baseAddrs) {
    os << baseaddr->name() << " ";
    os << "has " << baseAddrToSubAddrs[baseaddr].size() << " sub addrs." << endl;
    os << "read:\t";
    if (baseAddrIsRead[baseaddr])
      os << "yes\t";
    else
      os << "no\t";
    os << "write:\t";
    if (baseAddrIsWrite[baseaddr])
      os << "yes\t";
    else
      os << "no\t";
    os << endl;
    auto subAddrset = baseAddrToSubAddrs[baseaddr];
    os << "subAddrs:\t" << endl;
    for (auto subaddr : subAddrset) {
      os << subaddr->name() << ":";
      for (auto idxVal : subAddrToGepIdx[subaddr]->idxList) {
        printIdxType(subAddrToGepIdx[subaddr]->idxTypes[idxVal], os);
        os << "\t";
      }
      os << endl;
    }
  }
  os << "This Loop should ";
  if (not isParallelConcerningArray) {
    os << "not ";
  }
  os << "be parallelize." << endl;
  os << endl;
}

void pass::printIdxType(IdxType idxtype, std::ostream& os) {
  switch (idxtype) {
    case iLOOPINVARIANT:
      os << "LpI";
      break;
    case iIDV:
      os << "idv";
      break;
    case iIDVPLUSMINUSFORMULA:
      os << "idvAddMinusFormula";
      break;
    case iCALL:
      os << "call";
      break;
    case iLOAD:
      os << "load";
      break;
    case iELSE:
      os << "else";
      break;
    case iINNERIDV:
      os << "InnerIdv";
      break;
    case iINNERIDVPLUSMINUSFORMULA:
      os << "InnerIdvAddMinusFormula";
      break;
    default:
      std::cerr << "wrong idx type!" << std::endl;
      assert(false);
      break;
  }
}
