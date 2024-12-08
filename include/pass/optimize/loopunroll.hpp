#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include "pass/analysis/ControlFlowGraph.hpp"
#include "ir/ir.hpp"
#include "pass/pass.hpp"

using namespace ir;
namespace pass {
struct LoopUnrollContext final {
  LoopInfo* lpctx;
  IndVarInfo* ivctx;
  static std::unordered_map<Value*, Value*> copymap;
  std::vector<Instruction*> headuseouts;
  BasicBlock* nowlatchnext;
  bool definuseout(Instruction* inst, Loop* L);
  void insertremainderloop(Loop* loop, Function* func);
  void copyloop(std::vector<BasicBlock*> bbs, BasicBlock* begin, Loop* L, Function* func);
  void copyloopremainder(std::vector<BasicBlock*> bbs, BasicBlock* begin, Loop* L, Function* func);
  int calunrolltime(Loop* loop, int times);
  void doconstunroll(Loop* loop, IndVar* iv, int times);
  void dodynamicunroll(Loop* loop, IndVar* iv);
  void dynamicunroll(Loop* loop, IndVar* iv);
  void constunroll(Loop* loop, IndVar* iv);
  void dofullunroll(Loop* loop, IndVar* iv, int times);
  bool isconstant(IndVar* iv);
  void getdefinuseout(Loop* L);
  void replaceuseout(Instruction* inst, Instruction* copyinst, Loop* L);

  void loopdivest(Loop* loop, IndVar* iv, Function* func);
  void insertbranchloop(BasicBlock* bb0,
                        BasicBlock* bb1,
                        ValueId id,
                        Loop* L,
                        PhiInst* ivphi,
                        ICmpInst* ivicmp,
                        BinaryInst* iviter,
                        Value* endvar,
                        BasicBlock* condbb,
                        DomTree* domctx,
                        TopAnalysisInfoManager* tp);
  static Value* getValue(Value* val) {
    if (auto c = val->dynCast<ConstantValue>()) {
      return c;
    }
    if (copymap.count(val)) {
      return copymap[val];
    }
    return val;
  }
  LoopUnrollContext() {}
  LoopUnrollContext(LoopInfo* lpinfo, IndVarInfo* ivinfo): lpctx(lpinfo), ivctx(ivinfo) {}
  void run(Function* func, TopAnalysisInfoManager* tp);
};

class LoopUnroll : public FunctionPass {
  std::string name() const override { return "loopunroll"; }
  void run(Function* func, TopAnalysisInfoManager* tp) override;
};
}  // namespace pass