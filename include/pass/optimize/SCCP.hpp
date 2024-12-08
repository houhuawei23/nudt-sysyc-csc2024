#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include "ir/ir.hpp"
#include "pass/pass.hpp"

namespace pass {
class SCCP : public FunctionPass {
public:
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "SCCP"; }

private:
  bool cleanCFG(ir::Function* func);
  bool addConstFlod(ir::Instruction* inst);
  bool SCPrun(ir::Function* func, TopAnalysisInfoManager* tp);
  void searchCFG(ir::BasicBlock* bb);
};
}  // namespace pass