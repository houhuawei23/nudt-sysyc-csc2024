#pragma once
#include "pass/pass.hpp"

namespace pass {
class IRCheck : public ModulePass {
public:
  std::string name() const override { return "IR Check"; }
  void run(ir::Module* ctx, TopAnalysisInfoManager* tp) override;

private:
  bool runDefUseTest(ir::Function* func);
  bool runPhiTest(ir::Function* func);
  bool runCFGTest(ir::Function* func);
  bool checkDefUse(ir::Value* val);
  bool checkPhi(ir::PhiInst* phi);
  bool checkFuncInfo(ir::Function* func);
  bool checkAllocaOnlyInEntry(ir::Function* func);
  bool checkOnlyOneExit(ir::Function* func);
  bool checkParentRelationship(ir::Function* func);
  bool checkOperands(ir::Function* func);
};
}  // namespace pass