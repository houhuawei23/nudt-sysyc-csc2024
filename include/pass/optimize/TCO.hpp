#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <algorithm>
#include <queue>
#include "ir/ir.hpp"
#include "pass/pass.hpp"

using namespace ir;
namespace pass {
class TailCallOpt : public FunctionPass {
public:
  std::string name() const override { return "tailCallOpt"; }
  void run(Function* func, TopAnalysisInfoManager* tp) override;

private:
  bool is_tail_rec(Instruction* inst, Function* func);
  bool is_tail_call(Instruction* inst, Function* func);
  void recursiveDeleteInst(Instruction* inst);
  void recursiveDeleteBB(BasicBlock* bb);
};
}  // namespace pass