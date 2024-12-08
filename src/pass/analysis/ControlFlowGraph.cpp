#include "pass/analysis/ControlFlowGraph.hpp"

#include <iostream>
#include <algorithm>

namespace pass {
void CFGAnalysisHHW::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  // TODO: Implement CFG analysis
  /* clear the CFG: clear pres/nexts of all blocks */
  for (auto block : func->blocks()) {
    block->pre_blocks().clear();
    block->next_blocks().clear();
  }

  for (auto block : func->blocks()) {
    if (not block->verify(std::cerr)) {
      std::cerr << "block->verify() failed" << std::endl;
    }
    const auto backInst = block->insts().back();
    if (auto brInst = backInst->dynCast<ir::BranchInst>()) {
      if (brInst->is_cond()) {
        // block -> block.iftrue / block.iffalse
        block->next_blocks().push_back(brInst->iftrue());
        block->next_blocks().push_back(brInst->iffalse());
        brInst->iftrue()->pre_blocks().push_back(block);
        brInst->iffalse()->pre_blocks().push_back(block);
      } else {
        // block -> block.next
        block->next_blocks().push_back(brInst->dest());
        brInst->dest()->pre_blocks().push_back(block);
      }
    }
  }
  assert(check(std::cerr, func));
  // std::cerr << func->name() << " CFG analysis done" << std::endl;
}
bool CFGAnalysisHHW::check(std::ostream& os, ir::Function* func) const {
  auto findBlock = [](ir::BasicBlock* block,
                      std::list<ir::BasicBlock*>& blockList) {
    return std::find(blockList.begin(), blockList.end(), block) !=
           blockList.end();
  };
  for (auto block : func->blocks()) {
    for (auto preBlock : block->pre_blocks()) {
      if (not findBlock(block, preBlock->next_blocks())) {
        os << "check failed: " << block->name() << " has " << preBlock->name()
           << " as predecessor but it is not in preBlock.next_blocks list"
           << std::endl;
        return false;
      }
    }
    for (auto nextBlock : block->next_blocks()) {
      if (not findBlock(block, nextBlock->pre_blocks())) {
        os << "check failed: " << block->name() << " has " << nextBlock->name()
           << " as successor but it is not in nextBlock.pre_blocks list"
           << std::endl;
        return false;
      }
    }
  }
  return true;
}

}  // namespace pass
