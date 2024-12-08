#include "pass/optimize/DLE.hpp"
using namespace pass;

static std::unordered_map<ir::Value*, ir::LoadInst*> loadedPtrSet;
static std::unordered_set<ir::LoadInst*> removeInsts;
static std::unordered_map<ir::Value*, ir::Value*> ptrToValue;

void SimpleDLE::run(ir::BasicBlock* bb, TopAnalysisInfoManager* tp) {
  auto sectx = tp->getSideEffectInfo();
  loadedPtrSet.clear();
  removeInsts.clear();
  ptrToValue.clear();
  for (auto inst : bb->insts()) {
    if (auto loadInst = dyn_cast<ir::LoadInst>(inst)) {
      if (ptrToValue.count(loadInst->ptr())) {
        loadInst->replaceAllUseWith(ptrToValue[loadInst->ptr()]);
        removeInsts.insert(loadInst);
      } else if (loadedPtrSet.count(loadInst->ptr())) {
        auto oldLoadInst = loadedPtrSet[loadInst->ptr()];
        loadInst->replaceAllUseWith(oldLoadInst);
        removeInsts.insert(loadInst);
      } else {
        loadedPtrSet[loadInst->ptr()] = loadInst;
      }
    } else if (auto storeInst = dyn_cast<ir::StoreInst>(inst)) {
      loadedPtrSet.erase(storeInst->ptr());
      ptrToValue[storeInst->ptr()] = storeInst->value();
    } else if (auto callInst = inst->dynCast<ir::CallInst>()) {
      if (not sectx->hasSideEffect(callInst->callee())) continue;
      loadedPtrSet.clear();
      ptrToValue.clear();
    }
  }
  if (removeInsts.size() == 0) return;
  // std::cerr<<"Delete "<<removeInsts.size()<<" load insts."<<std::endl;
  for (auto inst : removeInsts) {
    bb->delete_inst(inst);
  }
}