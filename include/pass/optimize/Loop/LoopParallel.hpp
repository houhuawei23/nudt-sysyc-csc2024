#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include "ir/ir.hpp"
#include "pass/pass.hpp"

using namespace ir;
namespace pass {

class LoopParallel : public FunctionPass {
public:
  void run(Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "LoopParallel"; }

private:
  static bool isConstant(Value* val);
  bool runImpl(Function* func, TopAnalysisInfoManager* tp);
};

}  // namespace pass