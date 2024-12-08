#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include "ir/ir.hpp"
#include "pass/pass.hpp"

using namespace ir;
namespace pass {
class SCEV;
struct SCEVValue {
  Value* initVal;
  std::vector<Value*> addsteps;
  std::vector<Value*> substeps;
  PhiInst* phiinst;
  bool isFloat = false;
};

struct SCEVContext final {
  LoopInfo* lpctx;
  IndVarInfo* idvctx;
  SideEffectInfo* sectx;
  DomTree* domctx;

  void run(Function* func, TopAnalysisInfoManager* tp);

  void runOnLoop(Loop* lp, TopAnalysisInfoManager* tp);
  bool isSimplyLoopInvariant(Loop* lp, Value* val);
  bool isSimplyNotInLoop(Loop* lp, Value* val);
  bool isUsedOutsideLoop(Loop* lp, Value* val);
  int getConstantEndvarIndVarIterCnt(Loop* lp, IndVar* idv);
  Value* addCalcIterCntInstructions(Loop* lp, IndVar* idv, IRBuilder& builder);
  void normalizeIcmpAndBr(Loop* lp, IndVar* idv);
  void exchangeIcmpOp(ICmpInst* icmpInst);
  void reverseIcmpOp(ICmpInst* icmpInst);
  void exchangeBrDest(BranchInst* brInst);
  void visitPhi(Loop* lp, PhiInst* phiinst);
  int findAddSubChain(Loop* lp, PhiInst* phiinst, BinaryInst* nowInst);
  void getSCEVValue(Loop* lp, PhiInst* phiinst, std::vector<BinaryInst*>& instsChain);
  void SCEVReduceInstr(Loop* lp, SCEVValue* scevVal, Value* itercnt, IRBuilder& builder);
};

class SCEV : public FunctionPass {
public:
  void run(Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "scev"; }
};
}  // namespace pass