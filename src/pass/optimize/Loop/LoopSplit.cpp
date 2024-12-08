#include "pass/optimize/optimize.hpp"
#include "pass/optimize/loopsplit.hpp"
#include "pass/optimize/loopunroll.hpp"
#include "ir/value.hpp"
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>
namespace pass {

void LoopSplit::run(Function* func, TopAnalysisInfoManager* tp) {
  LoopSplitContext ctx;
  ctx.run(func, tp);
}
bool LoopSplitContext::couldsplit(Loop* loop) {
  if (loop->subLoops().size() != 0) return false;
  auto iv = ivctx->getIndvar(loop);
  if (iv) {
    // std::cerr << "get iv in func" << std::endl;
    iviter = iv->iterInst();
    // auto ivid = iviter->valueId();
    if ((iviter->valueId() != vADD) && (iviter->valueId() != vSUB)) return false;
    ivphi = iv->phiinst();
    ivicmp = iv->cmpInst()->dynCast<ICmpInst>();
  } else {
    // std::cerr << "no iv in func" << std::endl;
    return false;
  }

  auto head = loop->header();
  BasicBlock* headnext;
  for (auto bb : head->next_blocks()) {
    if (loop->contains(bb)) headnext = bb;
  }
  // 找到条件指令
  std::unordered_set<BasicBlock*> vis;
  BasicBlock::BasicBlockDfs(headnext, [&](BasicBlock* bb) -> bool {
    if (vis.count(bb) || (!loop->contains(bb)) || (bb == head)) return true;
    vis.insert(bb);
    if (bb->next_blocks().size() > 1) {
      condbb = bb;
      brinst = bb->insts().back()->dynCast<BranchInst>();
      return true;
    }
    return false;
  });

  std::unordered_set<ValueId> cd = {vISGE, vISLE, vISLT, vISGT};
  if (!brinst) return false;

  std::vector<BasicBlock*> branchture;
  std::vector<BasicBlock*> branchfalse;

  auto ift = brinst->iftrue();
  auto iff = brinst->iffalse();

  for (auto bb : loop->blocks()) {
    if (domctx->dominate(ift, bb)) {
      if (bb->pre_blocks().size() > 1) return false;
      branchture.push_back(bb);
    }
    if (domctx->dominate(iff, bb)) {
      if (bb->pre_blocks().size() > 1) return false;
      branchfalse.push_back(bb);
    }
  }

  auto cond = brinst->cond();
  if (icmpinst = cond->dynCast<ICmpInst>()) {
    if (icmpinst->lhs() == ivphi) {
      if (cd.count(icmpinst->valueId())) {
        endval = icmpinst->rhs();
        return true;
      }
    } else if (icmpinst->rhs() == ivphi) {
      if (cd.count(icmpinst->valueId())) {
        endval = icmpinst->lhs();
        return true;
      }
    }
  }
  return false;
}

void LoopSplitContext::splitloop(Loop* loop) {
  LoopUnrollContext unroll(lpctx, ivctx);
  if (iviter->valueId() == vADD) {
    if (icmpinst->rhs() == ivphi) {
      if (icmpinst->valueId() == vISGE) {
        unroll.insertbranchloop(brinst->iftrue(), brinst->iffalse(), vISLE, loop, ivphi, ivicmp,
                                iviter, endval, condbb, domctx, tpctx);
      } else if (icmpinst->valueId() == vISGT) {
        unroll.insertbranchloop(brinst->iftrue(), brinst->iffalse(), vISLT, loop, ivphi, ivicmp,
                                iviter, endval, condbb, domctx, tpctx);
      } else if (icmpinst->valueId() == vISLE) {
        unroll.insertbranchloop(brinst->iffalse(), brinst->iftrue(), vISLT, loop, ivphi, ivicmp,
                                iviter, endval, condbb, domctx, tpctx);
      } else if (icmpinst->valueId() == vISLT) {
        unroll.insertbranchloop(brinst->iffalse(), brinst->iftrue(), vISLE, loop, ivphi, ivicmp,
                                iviter, endval, condbb, domctx, tpctx);
      }
    } else if (ivicmp->lhs() == ivphi) {
      if (icmpinst->valueId() == vISGE) {
        unroll.insertbranchloop(brinst->iffalse(), brinst->iftrue(), vISLT, loop, ivphi, ivicmp,
                                iviter, endval, condbb, domctx, tpctx);
      } else if (icmpinst->valueId() == vISGT) {
        unroll.insertbranchloop(brinst->iffalse(), brinst->iftrue(), vISLE, loop, ivphi, ivicmp,
                                iviter, endval, condbb, domctx, tpctx);
      } else if (icmpinst->valueId() == vISLE) {
        unroll.insertbranchloop(brinst->iftrue(), brinst->iffalse(), vISLE, loop, ivphi, ivicmp,
                                iviter, endval, condbb, domctx, tpctx);
      } else if (icmpinst->valueId() == vISLT) {
        unroll.insertbranchloop(brinst->iftrue(), brinst->iffalse(), vISLT, loop, ivphi, ivicmp,
                                iviter, endval, condbb, domctx, tpctx);
      }
    } else {
      assert(false && "wrong ivphi");
    }
  } else if (iviter->valueId() == vSUB) {
    if (icmpinst->rhs() == ivphi) {
      if (icmpinst->valueId() == vISGE) {
        unroll.insertbranchloop(brinst->iffalse(), brinst->iftrue(), vISGT, loop, ivphi, ivicmp,
                                iviter, endval, condbb, domctx, tpctx);
      } else if (icmpinst->valueId() == vISGT) {
        unroll.insertbranchloop(brinst->iffalse(), brinst->iftrue(), vISGE, loop, ivphi, ivicmp,
                                iviter, endval, condbb, domctx, tpctx);
      } else if (icmpinst->valueId() == vISLE) {
        unroll.insertbranchloop(brinst->iftrue(), brinst->iffalse(), vISGE, loop, ivphi, ivicmp,
                                iviter, endval, condbb, domctx, tpctx);
      } else if (icmpinst->valueId() == vISLT) {
        unroll.insertbranchloop(brinst->iftrue(), brinst->iffalse(), vISGT, loop, ivphi, ivicmp,
                                iviter, endval, condbb, domctx, tpctx);
      }
    } else if (ivicmp->lhs() == ivphi) {
      if (icmpinst->valueId() == vISGE) {
        unroll.insertbranchloop(brinst->iftrue(), brinst->iffalse(), vISGE, loop, ivphi, ivicmp,
                                iviter, endval, condbb, domctx, tpctx);
      } else if (icmpinst->valueId() == vISGT) {
        unroll.insertbranchloop(brinst->iftrue(), brinst->iffalse(), vISGT, loop, ivphi, ivicmp,
                                iviter, endval, condbb, domctx, tpctx);
      } else if (icmpinst->valueId() == vISLE) {
        unroll.insertbranchloop(brinst->iffalse(), brinst->iftrue(), vISGT, loop, ivphi, ivicmp,
                                iviter, endval, condbb, domctx, tpctx);
      } else if (icmpinst->valueId() == vISLT) {
        unroll.insertbranchloop(brinst->iffalse(), brinst->iftrue(), vISGE, loop, ivphi, ivicmp,
                                iviter, endval, condbb, domctx, tpctx);
      }
    } else {
      assert(false && "wrong ivphi");
    }
  } else
    return;
}

bool LoopSplitContext::dosplit(Function* func, TopAnalysisInfoManager* tp) {
  lpctx = tp->getLoopInfo(func);
  ivctx = tp->getIndVarInfo(func);
  lpctx->sortedLoops(true);
  for (auto loop : lpctx->loops()) {
    if (couldsplit(loop)) {
      std::cerr << "split loop" << std::endl;
      splitloop(loop);
      return true;
    }
  }

  return false;
}

void LoopSplitContext::run(Function* func, TopAnalysisInfoManager* tp) {
  lpctx = tp->getLoopInfo(func);
  ivctx = tp->getIndVarInfo(func);
  domctx = tp->getDomTree(func);
  tpctx = tp;
  // std::cerr << func->name() << std::endl;
  while (dosplit(func, tp))
    ;
}
}  // namespace pass