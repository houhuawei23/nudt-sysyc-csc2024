#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include "ir/ir.hpp"
#include "pass/pass.hpp"

namespace pass {
struct IdvEdvReplContext final {
  LoopInfo* lpctx;
  IndVarInfo* idvctx;
  DomTree* domctx;
  SideEffectInfo* sectx;

  void run(ir::Function* func, TopAnalysisInfoManager* tp);

  void runOnLoop(ir::Loop* lp);
  int getConstantEndvarIndVarIterCnt(ir::Loop* lp, ir::IndVar* idv);
  void normalizeIcmpAndBr(ir::Loop* lp, ir::IndVar* idv);
  void exchangeIcmpOp(ir::ICmpInst* icmpInst);
  void reverseIcmpOp(ir::ICmpInst* icmpInst);
  void exchangeBrDest(ir::BranchInst* brInst);
  bool isSimplyNotInLoop(ir::Loop* lp, ir::Value* val);
  bool isSimplyLoopInvariant(ir::Loop* lp, ir::Value* val);
  void replaceIndvarAfterLoop(ir::Loop* lp, ir::IndVar* idv, ir::Value* finalVar);
  ir::Value* addFinalVarInstInLatchSub1(ir::Value* edv, ir::Loop* lp);
  ir::Value* addFinalVarInstInLatchAdd1(ir::Value* edv, ir::Loop* lp);
};

class IdvEdvRepl : public FunctionPass {
public:
  std::string name() const override { return "idvEdvRepl"; }
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
};
}  // namespace pass