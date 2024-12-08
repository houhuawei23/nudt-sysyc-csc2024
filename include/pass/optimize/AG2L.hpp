#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include "ir/ir.hpp"
#include "pass/pass.hpp"
namespace pass {

struct AggressiveG2LContext final {
  void run(ir::Module* md, TopAnalysisInfoManager* tp);

  DomTree* domctx;
  SideEffectInfo* sectx;
  CallGraph* cgctx;
  void replaceReadOnlyGv(ir::GlobalVariable* gv);
  void deleteWriteOnlyGv(ir::GlobalVariable* gv);
  void replaceGvInMain(ir::GlobalVariable* gv, ir::Function* func);        // 配合mem2reg使用
  void replaceGvInNormalFunc(ir::GlobalVariable* gv, ir::Function* func);  // 配合mem2reg使用
  void replaceGvInOneFunc(ir::GlobalVariable* gv, ir::Function* func);     // 配合mem2reg使用
};

class AggressiveG2L : public ModulePass {
public:
  void run(ir::Module* md, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "AG2L"; }
};
}  // namespace pass