#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include "ir/ir.hpp"
#include "pass/pass.hpp"

namespace pass {

struct GlobalCodeMotionContext final {
  std::set<ir::Instruction*> insts_visited;
  DomTree* domctx;
  LoopInfo* lpctx;
  SideEffectInfo* sectx;

  void run(ir::Function* func, TopAnalysisInfoManager* tp);

  void scheduleEarly(ir::Instruction* instruction, ir::BasicBlock* entry);
  // void scheduleLate(ir::Instruction *instruction, ir::BasicBlock* exit);
  ir::BasicBlock* LCA(ir::BasicBlock* lhs, ir::BasicBlock* rhs);
  bool ispinned(ir::Instruction* instruction);
};

class GCM : public FunctionPass {
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "GCM"; }
};
}  // namespace pass