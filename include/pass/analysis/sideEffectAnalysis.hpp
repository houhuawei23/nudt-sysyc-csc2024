#pragma once
#include "pass/pass.hpp"
using namespace pass;
namespace pass {

struct SideEffectAnalysisContext final {
  void run(ir::Module* md, TopAnalysisInfoManager* tp);

  void infoCheck(ir::Module* md);
  bool propogateSideEffect(ir::Module* md);
  TopAnalysisInfoManager* topmana;
  ir::Value* getIntToPtrBaseAddr(ir::UnaryInst* inst);
  ir::Value* getBaseAddr(ir::Value* subAddr);

  CallGraph* cgctx;
  SideEffectInfo* sectx;
};

class SideEffectAnalysis : public ModulePass {
public:
  std::string name() const override { return "sideEffectAnalysis"; }
  void run(ir::Module* md, TopAnalysisInfoManager* tp) override;

};
}  // namespace pass
