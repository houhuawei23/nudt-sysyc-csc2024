#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include "ir/ir.hpp"
#include "pass/pass.hpp"
#include "pass/analysis/dependenceAnalysis/dpaUtils.hpp"
using namespace ir;
namespace pass {
struct SROAContext final {
public:
  void run(Function* func, TopAnalysisInfoManager* tp);

private:
  DomTree* domctx;
  LoopInfo* lpctx;
  SideEffectInfo* sectx;
  DependenceInfo* dpctx;
  IndVarInfo* idvctx;
  LoopDependenceInfo* depInfoForLp;
  void runOnLoop(Loop* lp);
  AllocaInst* createNewLocal(Type* allocaType, Function* func);
  bool replaceAllUseInLpIdv(GetElementPtrInst* gep,
                            Loop* lp,
                            AllocaInst* newAlloca,
                            bool isOnlyRead,
                            bool isOnlyWrite);
  bool replaceAllUseInLpForLpI(GetElementPtrInst* gep,
                               Loop* lp,
                               AllocaInst* newAlloca,
                               bool isOnlyRead,
                               bool isOnlyWrite);
  int isTwoGepIdxPossiblySame(GepIdx* gepidx1, GepIdx* gepidx2, Loop* lp, IndVar* idv);
  int isTwoIdxPossiblySame(Value* val1,
                           Value* val2,
                           IdxType type1,
                           IdxType type2,
                           Loop* lp,
                           IndVar* idv);
  bool isSimplyLoopInvariant(Loop* lp, Value* val);
};

class SROA : public FunctionPass {
public:
  void run(Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "SROA"; }
};
}  // namespace pass