#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include "ir/ir.hpp"
#include "pass/pass.hpp"
namespace pass {

struct DeadLoopEliminationContext final {
  DomTree* domctx;
  SideEffectInfo* sectx;
  LoopInfo* lpctx;
  IndVarInfo* ivctx;

  bool isDeadLoop(ir::IndVar* iv, ir::Loop* loop);
  void deleteDeadLoop(ir::Loop* loop);
  void run(ir::Function* func, TopAnalysisInfoManager* tp);
};

class DeadLoopElimination : public FunctionPass {
public:
  std::string name() const override { return "DeadLoop"; }
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
};
}  // namespace pass