#include "pass/optimize/optimize.hpp"
#include "pass/optimize/loopsimplify.hpp"
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>
using namespace ir;

namespace pass {
BasicBlock* LoopSimplify::insertUniqueBackedgeBlock(Loop* L,
                                                    BasicBlock* preheader,
                                                    TopAnalysisInfoManager* tp) {
  BasicBlock* header = L->header();
  Function* F = header->function();

  if (!preheader) return nullptr;

  std::vector<BasicBlock*> BackedgeBBs;
  for (BasicBlock* BB : header->pre_blocks()) {
    if (BB != preheader)  // L->contains(BB)
      BackedgeBBs.push_back(BB);
  }

  BasicBlock* BEBB = F->newBlock();
  BranchInst* jmp = new BranchInst(header, BEBB);
  BEBB->emplace_back_inst(jmp);
  BasicBlock::block_link(BEBB, header);

  for (auto& inst : header->insts()) {
    if (PhiInst* phiinst = dyn_cast<PhiInst>(inst)) {
      PhiInst* BEphi = new PhiInst(BEBB, phiinst->type());
      BEBB->emplace_first_inst(BEphi);
      bool hasuniqueval = true;
      Value* uniqueval = nullptr;
      for (BasicBlock* BB : BackedgeBBs) {
        Value* val = phiinst->getvalfromBB(BB);
        BEphi->addIncoming(val, BB);
        if (hasuniqueval) {
          if (!uniqueval) {
            uniqueval = val;
          } else if (uniqueval != val) {
            hasuniqueval = false;
          }
        }
      }
      for (BasicBlock* BB : BackedgeBBs) {
        phiinst->delBlock(BB);
      }
      phiinst->addIncoming(BEphi, BEBB);
      if (hasuniqueval) {
        BEphi->replaceAllUseWith(uniqueval);
        BEBB->delete_inst(BEphi);
      }
    }
  }

  for (BasicBlock* BB : BackedgeBBs) {
    auto inst = BB->insts().back();
    BasicBlock::delete_block_link(BB, header);
    BasicBlock::block_link(BB, BEBB);
    if (BranchInst* brinst = dyn_cast<BranchInst>(inst)) {
      brinst->replaceDest(header, BEBB);
    }
  }
  L->blocks().insert(BEBB);
  return BEBB;
}

BasicBlock* LoopSimplify::insertUniquePreheader(Loop* L, TopAnalysisInfoManager* tp) {
  BasicBlock* header = L->header();
  Function* F = header->function();
  BasicBlock* preheader = L->getloopPredecessor();
  if (!preheader) {  // 有多个循环外的preheader，则插入一个新的汇合块
    std::vector<BasicBlock*> preBBs;
    for (BasicBlock* BB : header->pre_blocks()) {
      if (!L->contains(BB)) preBBs.push_back(BB);
    }
    BasicBlock* BEBB = F->newBlock();
    BranchInst* jmp = new BranchInst(header, BEBB);
    BEBB->emplace_back_inst(jmp);
    BasicBlock::block_link(BEBB, header);
    for (auto& inst : header->insts()) {
      if (PhiInst* phiinst = dyn_cast<PhiInst>(inst)) {
        PhiInst* BEphi = new PhiInst(BEBB, phiinst->type());
        BEBB->emplace_first_inst(BEphi);
        bool hasuniqueval = true;
        Value* uniqueval = nullptr;
        for (BasicBlock* BB : preBBs) {
          Value* val = phiinst->getvalfromBB(BB);
          BEphi->addIncoming(val, BB);
          // if (hasuniqueval) {
          //   if (!uniqueval) {
          //     uniqueval = val;
          //   } else if (uniqueval != val) {
          //     hasuniqueval = false;
          //   }
          // }
        }
        for (BasicBlock* BB : preBBs) {
          phiinst->delBlock(BB);
        }
        phiinst->addIncoming(BEphi, BEBB);
        // if (hasuniqueval) {
        //   BEphi->replaceAllUseWith(uniqueval);
        //   BEBB->delete_inst(BEphi);
        // }
      }
    }
    for (BasicBlock* BB : preBBs) {
      auto inst = BB->insts().back();
      BasicBlock::delete_block_link(BB, header);
      BasicBlock::block_link(BB, BEBB);
      if (BranchInst* brinst = dyn_cast<BranchInst>(inst)) {
        brinst->replaceDest(header, BEBB);
      }
    }
    L->blocks().insert(BEBB);
    return BEBB;
  }

  BasicBlock* newpre = F->newBlock();
  BranchInst* jmp = new BranchInst(header, newpre);
  newpre->emplace_back_inst(jmp);
  BranchInst* br = dyn_cast<BranchInst>(preheader->insts().back());
  br->replaceDest(header, newpre);
  BasicBlock::delete_block_link(preheader, header);
  BasicBlock::block_link(newpre, header);
  BasicBlock::block_link(preheader, newpre);

  for (auto inst : header->insts()) {
    if (PhiInst* phiinst = dyn_cast<PhiInst>(inst)) {
      phiinst->replaceoldtonew(preheader, newpre);
    }
  }
  L->blocks().insert(newpre);
  return newpre;
}

void LoopSimplify::insertUniqueExitBlock(Loop* L, TopAnalysisInfoManager* tp) {
  Function* F = L->header()->function();
  std::vector<BasicBlock*> InLoopPred;
  for (BasicBlock* exit : L->exits()) {
    if (exit->pre_blocks().size() > 1) {
      InLoopPred.clear();
      for (BasicBlock* pred : exit->pre_blocks()) {
        if (L->contains(pred)) {
          InLoopPred.push_back(pred);
        }
      }

      for (BasicBlock* pred : InLoopPred) {
        BasicBlock* newBB = F->newBlock();
        BranchInst* jmp = new BranchInst(exit, newBB);
        newBB->emplace_back_inst(jmp);
        BranchInst* br = dyn_cast<BranchInst>(pred->insts().back());
        br->replaceDest(exit, newBB);
        BasicBlock::delete_block_link(pred, exit);
        BasicBlock::block_link(pred, newBB);
        BasicBlock::block_link(newBB, exit);
        for (auto inst : exit->insts()) {
          if (PhiInst* phiinst = dyn_cast<PhiInst>(inst)) {
            phiinst->replaceoldtonew(pred, newBB);
          }
        }
      }
      // 需要更新_exits
    }
  }
  return;
}

bool LoopSimplify::simplifyOneLoop(Loop* L, TopAnalysisInfoManager* tp) {
  bool changed = false;
  // if (L->isLoopSimplifyForm()) return false;
  // 如果有多条回边
  // preheader不能为F的entry
  BasicBlock* entry = L->header()->function()->entry();
  BasicBlock* preheader = L->getLoopPreheader();
  BasicBlock* LoopLatch = L->getLoopLatch();
  if (!preheader || preheader == entry) {
    preheader = insertUniquePreheader(L, tp);
    if (preheader) changed = true;
  }

  if (!LoopLatch) {
    LoopLatch = insertUniqueBackedgeBlock(L, preheader, tp);
    if (LoopLatch) changed = true;
  }

  if (!L->hasDedicatedExits()) {
    insertUniqueExitBlock(L, tp);
    changed = true;
  }

  return changed;
}

void LoopSimplify::run(Function* func, TopAnalysisInfoManager* tp) {
  if (func->isOnlyDeclare()) return;
  // func->rename();
  // func->print(std::cerr);
  LoopInfo* LI = tp->getLoopInfo(func);
  func->rename();
  // func->rename();
  // func->print(std::cerr);
  auto loops = LI->loops();
  bool changed = false;
  for (auto L : loops) {
    changed |= simplifyOneLoop(L, tp);
    if (!L->isLoopSimplifyForm()) {
      assert("loop is not in simplify form");
    }
  }
  if (changed) {
    // update loopinfo
    tp->CFGChange(func);
  }
  // func->rename();
  // func->print(std::cerr);

  return;
}
}  // namespace pass