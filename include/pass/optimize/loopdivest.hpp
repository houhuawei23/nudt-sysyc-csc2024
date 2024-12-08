#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include "ir/ir.hpp"
#include "pass/pass.hpp"
#include "pass/optimize/loopunroll.hpp"

using namespace ir;
namespace pass {

struct LoopDivestContext final {
  DomTree* domctx;
  SideEffectInfo* sectx;
  LoopInfo* lpctx;
  IndVarInfo* ivctx;

  bool shoulddivest(Loop* loop);
  void runonloop(Loop* loop, Function* func);
  void run(Function* func, TopAnalysisInfoManager* tp);
};

class LoopDivest : public FunctionPass {

public:
  std::string name() const override { return "LoopDivestContext"; }
  void run(Function* func, TopAnalysisInfoManager* tp) override;
};
}  // namespace pass