#pragma once
#include "ir/ir.hpp"
#include "pass/pass.hpp"
#include "pass/optimize/Utils/BlockUtils.hpp"
using namespace ir;

namespace pass {
class BlockSort : public FunctionPass {
  bool runImpl(Function* func, TopAnalysisInfoManager* tp) { return blockSortDFS(*func, tp); }

public:
  void run(Function* func, TopAnalysisInfoManager* tp) override { runImpl(func, tp); }
  std::string name() const override { return "BlockSort"; }
};
}  // namespace pass