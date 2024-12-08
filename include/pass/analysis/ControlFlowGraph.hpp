#pragma once
#include "ir/ir.hpp"
#include "pass/pass.hpp"
#include <iostream>
namespace pass {
class CFGAnalysisHHW : public FunctionPass {
  bool check(std::ostream& os, ir::Function* func) const;

 public:
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "CFGAnalysisHHW"; }
};

}  // namespace pass