#pragma once
#include "ir/ir.hpp"
#include "pass/pass.hpp"
using namespace ir;

namespace pass {
class StatelessCache : public FunctionPass {
  static Function* getLookupFunction(Module* module, ArrayType* entryType, ArrayType* lutType);
  static bool has2MoreRecursiveCalls(Function* func);
  bool runImpl(ir::Function* func, TopAnalysisInfoManager* tp);

public:
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "StatelessCache"; }
};
}  // namespace pass