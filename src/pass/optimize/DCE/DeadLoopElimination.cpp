#include "pass/optimize/optimize.hpp"
#include "pass/optimize/DeadLoopElimination.hpp"
#include "ir/value.hpp"
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>
namespace pass {
void DeadLoopElimination::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  DeadLoopEliminationContext ctx;
  ctx.run(func, tp);
}
bool DeadLoopEliminationContext::isDeadLoop(ir::IndVar* iv, ir::Loop* loop) {
  auto ivbegin = iv->beginValue();
  auto ivend = iv->endValue();
  auto ivcmp = iv->cmpInst();
  if (ivbegin == ivend) {
    if (ivcmp->valueId() == ir::vISLT || ivcmp->valueId() == ir::vISGT) return true;
  }
  return false;
}
void DeadLoopEliminationContext::deleteDeadLoop(ir::Loop* loop) {
  auto head = loop->header();
  ir::BasicBlock* headnext;
  ir::BasicBlock* exitbb;
  for (auto bb : loop->exits()) {
    exitbb = bb;
  }
  for (auto bb : head->next_blocks()) {
    if (bb != exitbb) {
      headnext = bb;
    }
  }
  auto headbr = head->insts().back();
  head->delete_inst(headbr);
  auto newbr = utils::make<ir::BranchInst>(exitbb);
  head->emplace_back_inst(newbr);
  ir::BasicBlock::delete_block_link(head, headnext);
}

void DeadLoopEliminationContext::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  // std::cerr << func->name() << std::endl;
  lpctx = tp->getLoopInfo(func);
  ivctx = tp->getIndVarInfo(func);
  for (auto loop : lpctx->loops()) {
    if (loop->exits().size() > 1) continue;
    auto iv = ivctx->getIndvar(loop);
    if (!iv) continue;
    if (isDeadLoop(iv, loop)) {
      // std::cerr << "find dead loop!" << std::endl;
      deleteDeadLoop(loop);
    }
  }
}
}  // namespace pass