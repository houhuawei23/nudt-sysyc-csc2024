#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include "ir/ir.hpp"
#include "pass/pass.hpp"

namespace pass {

struct InlineContext {
  CallGraph* cgctx;
  void callinline(ir::CallInst* call);
  std::vector<ir::CallInst*> getcall(ir::Module* module,
                                     ir::Function* function);  // 找出调用了function的call指令
  std::vector<ir::Function*> getinlineFunc(ir::Module* module);
  void run(ir::Module* module, TopAnalysisInfoManager* tp);
};
class Inline : public ModulePass {
public:
  std::string name() const override { return "Inline"; }

  void run(ir::Module* module, TopAnalysisInfoManager* tp) override;
};
}  // namespace pass
