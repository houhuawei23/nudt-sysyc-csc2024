#pragma once
#include "pass/pass.hpp"

using namespace ir;
namespace pass {

struct IndVarAnalysisContext {
  LoopInfo* lpctx;
  IndVarInfo* ivctx;
  DomTree* domctx;
  SideEffectInfo* sectx;

  void run(Function* func, TopAnalysisInfoManager* tp);

  void addIndVar(Loop* lp,
                 Value* mbegin,
                 Value* mstep,
                 Value* mend,
                 BinaryInst* iterinst,
                 Instruction* cmpinst,
                 PhiInst* phiinst);
  ConstantInteger* getConstantBeginVarFromPhi(PhiInst* phiinst, PhiInst* oldPhiinst, Loop* lp);
  bool isSimplyNotInLoop(Loop* lp, Value* val);
  bool isSimplyLoopInvariant(Loop* lp, Value* val);
};

class IndVarAnalysis : public FunctionPass {
public:
  std::string name() const override { return "indVarAnalysis"; }
  void run(Function* func, TopAnalysisInfoManager* tp) override;
};

class IndVarInfoCheck : public FunctionPass {
public:
  void run(Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "indVarCheckInfo"; }

};
}  // namespace pass