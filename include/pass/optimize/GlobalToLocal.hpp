#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include "ir/ir.hpp"
#include "pass/pass.hpp"

namespace pass {

struct Global2LocalContext {
  CallGraph* cgctx;
  void globalCallAnalysis(ir::Module* md);
  void addIndirectGlobalUseFunc(ir::GlobalVariable* gv, ir::Function* func);
  bool processGlobalVariables(ir::GlobalVariable* gv, ir::Module* md, TopAnalysisInfoManager* tp);

  void run(ir::Module* md, TopAnalysisInfoManager* tp);
};

class Global2Local : public ModulePass {
  std::string name() const override { return "global2local"; }
  void run(ir::Module* md, TopAnalysisInfoManager* tp) override;
};
}  // namespace pass