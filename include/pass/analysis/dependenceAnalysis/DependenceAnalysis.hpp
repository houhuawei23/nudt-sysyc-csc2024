#pragma once
#include "ir/ir.hpp"
#include "pass/pass.hpp"
#include "pass/analysis/dependenceAnalysis/dpaUtils.hpp"

using namespace ir;
namespace pass {

class DependenceAnalysis;
// class dependenceAnalysisInfoCheck;

struct DependenceAnalysisContext final {
  TopAnalysisInfoManager* topmana;
  DomTree* domctx;
  LoopInfo* lpctx;
  IndVarInfo* idvctx;
  SideEffectInfo* sectx;
  CallGraph* cgctx;
  DependenceInfo* dpctx;
  void runOnLoop(Loop* lp);
  void makeGepIdx(Loop* lp, IndVar* idv, GepIdx* gepidx);
  bool isSimplyLoopInvariant(Loop* lp, Value* val);
  bool isIDVPLUSMINUSFORMULA(IndVar* idv, Value* val, Loop* lp);
  int isTwoGepIdxPossiblySame(GepIdx* gepidx1, GepIdx* gepidx2, Loop* lp, IndVar* idv);
  int isTwoIdxPossiblySame(Value* val1,
                           Value* val2,
                           IdxType type1,
                           IdxType type2,
                           Loop* lp,
                           IndVar* idv);
  void run(Function* func, TopAnalysisInfoManager* tp);
};

class DependenceAnalysis : public FunctionPass {
public:
  void run(Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "DepAnalysis"; }
};

};  // namespace pass