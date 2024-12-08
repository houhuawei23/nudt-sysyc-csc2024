#include "pass/optimize/GCM.hpp"
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>

namespace pass {
// 通过指令类型判断该指令是否固定
bool GlobalCodeMotionContext::ispinned(ir::Instruction* instruction) {
  if (dyn_cast<ir::BinaryInst>(instruction)) {
    if (instruction->valueId() == ir::vSDIV || instruction->valueId() == ir::vSREM ||
        instruction->valueId() == ir::vFDIV ||
        instruction->valueId() ==
          ir::vFREM)  // 二元运算指令不固定(除法取余固定，因为除法取余指令可能产生除零错误)
      return true;
    return false;
  } else if (dyn_cast<ir::UnaryInst>(instruction))  // 一元运算指令不固定
    return false;
  else if (dyn_cast<ir::GetElementPtrInst>(instruction))
    return false;  // GEP指令不固定
  else if (dyn_cast<ir::PtrCastInst>(instruction))
    return false;
  else  // 其他指令固定在自己的BB上
    return true;
}
// 提前调度:思想是如果把一个指令尽量往前提，那么应该在提之前将该指令参数来自的指令前提
void GlobalCodeMotionContext::scheduleEarly(ir::Instruction* instruction, ir::BasicBlock* entry) {
  if (insts_visited.count(instruction))  // 如果已经访问过，则不进行提前调度
    return;

  insts_visited.insert(instruction);
  auto destBB = entry;  // 初始化放置块为entry块,整棵树的root
  ir::BasicBlock* opBB = nullptr;
  for (auto opiter = instruction->operands().begin(); opiter != instruction->operands().end();) {
    auto op = *opiter;
    opiter++;
    if (auto opInst = dyn_cast<ir::Instruction>(op->value())) {
      if (opInst->block() != entry) scheduleEarly(opInst, entry);
      opBB = opInst->block();
      if (domctx->domlevel(opBB) > domctx->domlevel(destBB)) {
        destBB = opBB;
      }
    }
  }
  if (!ispinned(instruction)) {
    if (lpctx->looplevel(instruction->block()) == 0) return;
    auto instbb = instruction->block();
    if (destBB == instbb) return;

    auto bestBB = instbb;
    auto curBB = instbb;

    while (domctx->domlevel(curBB) > domctx->domlevel(destBB)) {
      if (lpctx->looplevel(curBB) <= lpctx->looplevel(bestBB)) bestBB = curBB;
      curBB = domctx->idom(curBB);
      if ((!curBB) || (curBB == entry)) break;
    }

    if (bestBB == instbb) return;
    instbb->move_inst(instruction);                // 将指令从bb中移除
    bestBB->emplace_lastbutone_inst(instruction);  // 将指令移入destBB
  }
}

void GlobalCodeMotionContext::run(ir::Function* F, TopAnalysisInfoManager* tp) {
  domctx = tp->getDomTree(F);
  lpctx = tp->getLoopInfo(F);
  sectx = tp->getSideEffectInfo();
  std::vector<ir::Instruction*> pininsts;
  insts_visited.clear();

  for (auto bb : F->blocks()) {
    for (auto institer = bb->insts().begin(); institer != bb->insts().end();) {
      auto inst = *institer;
      institer++;
      if (ispinned(inst)) pininsts.push_back(inst);
    }
  }

  for (auto inst : pininsts) {
    insts_visited.clear();
    for (auto opiter = inst->operands().begin(); opiter != inst->operands().end();) {
      auto op = *opiter;
      opiter++;
      if (ir::Instruction* opinst = dyn_cast<ir::Instruction>(op->value()))
        scheduleEarly(opinst, F->entry());
    }
  }
}

void GCM::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  GlobalCodeMotionContext gcmctx;
  gcmctx.run(func, tp);
}
}  // namespace pass