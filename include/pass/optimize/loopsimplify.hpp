#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include "ir/ir.hpp"
#include "pass/pass.hpp"

using namespace ir;
namespace pass {
class LoopSimplify : public FunctionPass {
public:
  std::string name() const override { return "Loopsimplify"; }
  BasicBlock* insertUniqueBackedgeBlock(Loop* L,
                                            BasicBlock* preheader,
                                            TopAnalysisInfoManager* tp);
  BasicBlock* insertUniquePreheader(Loop* L, TopAnalysisInfoManager* tp);
  void insertUniqueExitBlock(Loop* L, TopAnalysisInfoManager* tp);
  bool simplifyOneLoop(Loop* L, TopAnalysisInfoManager* tp);
  void run(Function* func, TopAnalysisInfoManager* tp) override;
};
}  // namespace pass