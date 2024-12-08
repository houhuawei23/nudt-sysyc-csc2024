#pragma once
#include "ir/ir.hpp"
#include "pass/pass.hpp"
#include <set>

namespace pass {
class DCE : public FunctionPass {
public:
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "DCE"; }

private:
  bool isAlive(ir::Instruction* inst);
  void addAlive(ir::Instruction* inst);
  // void DCE_delete(ir::Instruction* inst);
};

}  // namespace pass
