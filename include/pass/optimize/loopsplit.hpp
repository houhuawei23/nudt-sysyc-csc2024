#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include "ir/ir.hpp"
#include "pass/pass.hpp"
using namespace ir;
namespace pass {

struct LoopSplitContext final {
  DomTree* domctx;
  SideEffectInfo* sectx;
  LoopInfo* lpctx;
  IndVarInfo* ivctx;
  TopAnalysisInfoManager* tpctx;
  BranchInst* brinst = nullptr;
  ICmpInst* icmpinst = nullptr;
  Value* endval = nullptr;
  BasicBlock* condbb = nullptr;

  PhiInst* ivphi = nullptr;
  ICmpInst* ivicmp = nullptr;
  BinaryInst* iviter = nullptr;

  void splitloop(Loop* L);
  bool dosplit(Function* func, TopAnalysisInfoManager* tp);
  bool couldsplit(Loop* loop);
  void run(Function* func, TopAnalysisInfoManager* tp);
};

class LoopSplit : public FunctionPass {
  std::string name() const override { return "loopsplit"; }
  void run(Function* func, TopAnalysisInfoManager* tp) override;
};
}  // namespace pass