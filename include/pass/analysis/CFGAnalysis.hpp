#pragma once
#include "ir/ir.hpp"
#include "pass/pass.hpp"
using namespace ir;
namespace pass {
class CFGAnalysis : public ModulePass {
public:
  void run(Module* ctx, TopAnalysisInfoManager* tp) override;
  void dump(std::ostream& out, Module* ctx);
};
}  // namespace pass