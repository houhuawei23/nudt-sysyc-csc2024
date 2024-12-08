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

class LoopInterChange : public FunctionPass {
public:
  void run(Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "LoopInterChange"; }

private:
  bool runImpl(Function* func, TopAnalysisInfoManager* tp);
};

}  // namespace pass