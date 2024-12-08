#pragma once
#include "ir/ir.hpp"
#include "pass/pass.hpp"
#include <vector>
#include <set>

namespace pass {
struct CallGraphBuildContext final {
  std::vector<ir::Function*> funcStack;
  std::set<ir::Function*> funcSet;
  std::map<ir::Function*, bool> vis;

  CallGraph* cgctx;
  
  void dfsFuncCallGraph(ir::Function* func);
  void run(ir::Module* ctx, TopAnalysisInfoManager* tp);
};
class CallGraphBuild : public ModulePass {
public:
  std::string name() const override { return "callGraphBuild"; }
  void run(ir::Module* ctx, TopAnalysisInfoManager* tp) override;
};

class CallGraphCheck : public ModulePass {
public:
  std::string name() const override { return "callGraphCheck"; }
  void run(ir::Module* ctx, TopAnalysisInfoManager* tp) override;
};
}  // namespace pass