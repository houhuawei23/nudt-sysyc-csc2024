#pragma once
#include "ir/ir.hpp"
#include "pass/pass.hpp"

namespace pass {
class CFGPrinter : public FunctionPass {
public:
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "CFGPrinter"; }
};
}  // namespace pass