#include "pass/optimize/optimize.hpp"
#include "pass/optimize/loopdivest.hpp"
#include "pass/optimize/loopunroll.hpp"
#include "ir/value.hpp"
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>
namespace pass {

void LoopDivest::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  LoopDivestContext context;
  context.run(func, tp);
}

bool LoopDivestContext::shoulddivest(ir::Loop* loop) {
  if (loop->exits().size() != 1) return false;
  if (loop->subLoops().size() != 1) return false;
  ir::IndVar* iv = ivctx->getIndvar(loop);
  if (!iv) return false;
  ir::Loop* subloop;
  subloop = *(loop->subLoops().begin());
  if (subloop->subLoops().size() != 0) return false;
  ir::IndVar* siv = ivctx->getIndvar(subloop);
  if (!siv) return false;
  if (!iv->isBeginVarConst()) return false;
  if (iv && (!siv->getIsBeginAndStepConst())) return true;
  return false;
}
void LoopDivestContext::runonloop(ir::Loop* loop, ir::Function* func) {
  if (shoulddivest(loop)) {
    std::cerr << "loopdivest" << std::endl;
    ir::IndVar* iv = ivctx->getIndvar(loop);
    LoopUnrollContext loopunroll(lpctx, ivctx);
    loopunroll.loopdivest(loop, iv, func);
  }
}

void LoopDivestContext::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  // std::cerr << func->name() << std::endl;
  lpctx = tp->getLoopInfo(func);
  // lpctx->setOff();
  // lpctx->refresh();
  ivctx = tp->getIndVarInfo(func);
  for (auto loop : lpctx->loops()) {
    runonloop(loop, func);
  }
}
}  // namespace pass