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
class SimplifyCFG : public FunctionPass {
public:
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "simplifyCFG"; }

private:
  bool getSingleDest(ir::BasicBlock* bb);
  ir::BasicBlock* getMergeBlock(ir::BasicBlock* bb);
  bool MergeBlock(ir::Function* func);
  bool removeNoPreBlock(ir::Function* func);
  bool removeSingleBrBlock(ir::Function* func);
  bool removeSingleIncomingPhi(ir::Function* func);
};
}  // namespace pass