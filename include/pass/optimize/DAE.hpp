#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include "ir/ir.hpp"
#include "pass/pass.hpp"
namespace pass {
class DAE : public ModulePass {
public:
  void run(ir::Module* md, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "dae"; }
};
}  // namespace pass