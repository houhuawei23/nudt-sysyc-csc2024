// 死本地数组删除
#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <algorithm>
#include <queue>
#include "ir/ir.hpp"
#include "pass/pass.hpp"

namespace pass {
struct DLAEContext final {
  std::vector<ir::GetElementPtrInst*> geps;
  std::vector<ir::StoreInst*> stores;
  std::vector<ir::LoadInst*> loads;
  std::vector<ir::CallInst*> calls;
  std::vector<ir::UnaryInst*> bitcasts;
  std::vector<ir::MemsetInst*> memsets;
  void dfs(ir::AllocaInst* alloca, ir::Instruction* inst);
  void run(ir::Function* func, TopAnalysisInfoManager* tp);
};

class DLAE : public FunctionPass {
public:
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "dlae"; }
};
}  // namespace pass