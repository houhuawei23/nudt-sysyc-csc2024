#pragma once
#include "ir/ir.hpp"
#include "pass/pass.hpp"
#include <set>
#include <queue>

using namespace ir;
namespace pass {

struct ADCEContext final {
  PDomTree* pdctx;
  void ADCEInfoCheck(ir::Function* func);
  ir::BasicBlock* getTargetBB(ir::BasicBlock* bb);

  void run(ir::Function* func, TopAnalysisInfoManager* tp);
};

class ADCE : public FunctionPass {
public:
  std::string name() const override { return "ADCE"; }
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
};

}  // namespace pass
