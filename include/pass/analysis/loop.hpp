#pragma once
#include "pass/pass.hpp"

namespace pass {

struct LoopAnalysisContext final {
  LoopInfo* lpctx;
  DomTree* domctx;
  // std::unordered_map<ir::BasicBlock*, int> stLoopLevel;
  void addLoopBlocks(ir::Function* func, ir::BasicBlock* header, ir::BasicBlock* tail);
  void loopGetExits(ir::Loop* plp);
  void run(ir::Function* func, TopAnalysisInfoManager* tp);
};
class LoopAnalysis : public FunctionPass {
public:
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "Loop Analysis"; }
};

class LoopInfoCheck : public FunctionPass {
public:
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "Loop Info Check"; }
};
}  // namespace pass
