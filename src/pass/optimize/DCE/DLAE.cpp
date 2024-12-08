#include "pass/optimize/optimize.hpp"
#include "pass/optimize/DLAE.hpp"
#include "ir/value.hpp"
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>
namespace pass {
void DLAE::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  DLAEContext context;
  context.run(func, tp);
}
void DLAEContext::dfs(ir::AllocaInst* alloca, ir::Instruction* inst) {
  if (inst->dynCast<ir::GetElementPtrInst>()) {
    geps.push_back(inst->dynCast<ir::GetElementPtrInst>());
    for (auto use : inst->uses()) {
      dfs(alloca, use->user()->dynCast<ir::Instruction>());
    }
  } else if (inst->dynCast<ir::StoreInst>()) {
    stores.push_back(inst->dynCast<ir::StoreInst>());
  } else if (inst->dynCast<ir::LoadInst>()) {
    loads.push_back(inst->dynCast<ir::LoadInst>());
  } else if (inst->dynCast<ir::CallInst>()) {
    calls.push_back(inst->dynCast<ir::CallInst>());
  } else if (inst->dynCast<ir::MemsetInst>()) {
    memsets.push_back(inst->dynCast<ir::MemsetInst>());
  } else if (inst->dynCast<ir::UnaryInst>()) {
    auto bitcast = inst->dynCast<ir::UnaryInst>();
    if (bitcast->valueId() == ir::vBITCAST) {
      bitcasts.push_back(bitcast);
      for (auto use : bitcast->uses()) {
        dfs(alloca, use->user()->dynCast<ir::Instruction>());
      }
    }
  }
}
void DLAEContext::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  std::vector<ir::AllocaInst*> allocas;
  for (auto inst : func->entry()->insts()) {
    if (auto alloca = inst->dynCast<ir::AllocaInst>()) {
      allocas.push_back(alloca);
    }
  }
  for (auto alloca : allocas) {
    geps.clear();
    stores.clear();
    loads.clear();
    calls.clear();
    memsets.clear();
    bitcasts.clear();
    for (auto use : alloca->uses()) {
      if (use->user()->dynCast<ir::Instruction>()) {
        dfs(alloca, use->user()->dynCast<ir::Instruction>());
      }
    }
    if (!calls.empty()) continue;
    ;
    if (!loads.empty()) continue;
    for (auto inst : stores)
      inst->block()->force_delete_inst(inst);
    for (auto inst : geps)
      inst->block()->force_delete_inst(inst);
    for (auto inst : memsets)
      inst->block()->force_delete_inst(inst);
    for (auto inst : bitcasts)
      inst->block()->force_delete_inst(inst);
    alloca->block()->force_delete_inst(alloca);
  }
  return;
}
}  // namespace pass