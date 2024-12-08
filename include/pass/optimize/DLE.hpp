#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <algorithm>
#include <queue>
#include "ir/ir.hpp"
#include "pass/pass.hpp"

namespace pass {
class SimpleDLE : public BasicBlockPass {
public:
  void run(ir::BasicBlock* bb, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "simpleDLE"; }
};
}  // namespace pass