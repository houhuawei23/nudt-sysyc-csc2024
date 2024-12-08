#pragma once
#include "ir/ir.hpp"
#include "pass/pass.hpp"
#include <vector>
#include <set>
using namespace ir;

namespace pass {
class MarkParallel;
struct ResPhi;
struct ResPhi {
  PhiInst* phi;   // 对应的phi指令指针
  bool isAdd;     // 表示最后的结果汇合将使用+
  bool isMul;     // 表示最后的结果汇合将使用*
  bool isSub;     // 表示最后的结果汇合将使用preVal-gv1-gv2-gv3-gv4
  bool isModulo;  // 在+的基础上，最后的结果每次汇合需要mod
  Value* mod;     // if isModulo,使用这个值进行
};

struct MarkParallelContext final {
  TopAnalysisInfoManager* topmana;
  DependenceInfo* dpctx;
  DomTree* domctx;
  LoopInfo* lpctx;
  SideEffectInfo* sectx;
  CallGraph* cgctx;
  IndVarInfo* idvctx;
  ParallelInfo* parctx;

  void runOnLoop(Loop* lp);
  void printParallelInfo(Function* func);
  ResPhi* getResPhi(PhiInst* phi, Loop* lp);
  bool isSimplyLpInvariant(Loop* lp, Value* val);
  bool isFuncParallel(Loop* lp, CallInst* callinst);
  Value* getBaseAddr(Value* ptr);
  void run(Function* func, TopAnalysisInfoManager* tp);
};

class MarkParallel : public FunctionPass {
public:
  void run(Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "MarkParallel"; }
};
};  // namespace pass