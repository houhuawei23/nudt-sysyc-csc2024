// #define DEBUG
#include "pass/optimize/Utils/BlockUtils.hpp"

#include "pass/analysis/dom.hpp"
using namespace ir;

namespace pass {
void dumpInst(std::ostream& os, Instruction* inst) {
#ifdef DEBUG
  if (not inst) {
    os << "nullptr";
    return;
  }
  inst->print(os);
  os << std::endl;
#endif
}
void dumpAsOperand(std::ostream& os, Value* val) {
#ifdef DEBUG
  val->dumpAsOpernd(os);
  os << std::endl;
#endif
}

void dumpFunction(Function& func) {
  func.rename();
  func.print(std::cerr);
}

bool fixPhiIncomingBlock(BasicBlock* target, BasicBlock* oldBlock, BasicBlock* newBlock) {
  for (auto inst : target->insts()) {
    if (auto phi = inst->dynCast<PhiInst>()) {
      phi->replaceoldtonew(oldBlock, newBlock);
    }
  }
  return true;
}

bool blockSortBFS(Function& func, TopAnalysisInfoManager* tAIM) {
  auto domCtx = tAIM->getDomTree(&func);
  domCtx->setOff();
  domCtx->refresh();
  domCtx->BFSDomTreeInfoRefresh();
  auto irBlocks = domCtx->BFSDomTreeVector();
  // std::cerr << "func Blocks: " << std::endl;
  // for(auto block : func.blocks()) {
  //   block->dumpAsOpernd(std::cerr);
  //   std::cerr << " ";
  // }
  // std::cerr << std::endl;
  // std::cerr << "IRBlocks: " << std::endl;
  // for(auto block : irBlocks) {
  //   block->dumpAsOpernd(std::cerr);
  //   std::cerr << " ";
  // }
  // std::cerr << std::endl;
  // assert(irBlocks.size() == func.blocks().size());
  if (irBlocks.size() != func.blocks().size()) {
    std::cerr << "Error: blockSortBFS: irBlocks.size() != func.blocks().size()" << std::endl;
    return false;
  }
  func.blocks().clear();
  for (auto block : irBlocks) {
    func.blocks().push_back(block);
  }
  return true;
}
bool blockSortDFS(Function& func, TopAnalysisInfoManager* tAIM) {
  auto domCtx = tAIM->getDomTree(&func);
  domCtx->setOff();
  domCtx->refresh();
  domCtx->DFSDomTreeInfoRefresh();
  auto irBlocks = domCtx->DFSDomTreeVector();
  // std::cerr << "func Blocks: " << std::endl;
  // for(auto block : func.blocks()) {
  //   block->dumpAsOpernd(std::cerr);
  //   std::cerr << " ";
  // }
  // std::cerr << std::endl;
  // std::cerr << "IRBlocks: " << std::endl;
  // for(auto block : irBlocks) {
  //   block->dumpAsOpernd(std::cerr);
  //   std::cerr << " ";
  // }
  // std::cerr << std::endl;
  // assert(irBlocks.size() == func.blocks().size());
  if (irBlocks.size() != func.blocks().size()) {
    std::cerr << "Error: blockSortDFS: irBlocks.size() != func.blocks().size()" << std::endl;
    return false;
  }
  func.blocks().clear();
  for (auto block : irBlocks) {
    func.blocks().push_back(block);
  }
  return true;
}

bool fixAllocaInEntry(Function& func) {
  // dumpFunction(func);
  std::unordered_set<Instruction*> allocas;
  for (auto block : func.blocks()) {
    for (auto inst : block->insts()) {
      if (auto allocaInst = inst->dynCast<AllocaInst>()) {
        allocas.insert(allocaInst);
      }
    }
  }
  // remove
  for (auto block : func.blocks()) {
    block->insts().remove_if([&](Instruction* inst) { return allocas.count(inst); });
  }
  const auto oldEntry = func.entry();
  // new entry block"
  auto newEntry = utils::make<BasicBlock>("new_entry");
  func.blocks().push_front(newEntry);
  func.setEntry(newEntry);
  // add allocas to new entry
  for (auto allocaInst : allocas) {
    newEntry->emplace_back_inst(allocaInst);
  }
  // link new entry to old entry
  IRBuilder builder;
  builder.set_pos(newEntry, newEntry->insts().end());
  builder.makeInst<BranchInst>(oldEntry);

  // dumpFunction(func);
  return true;
}

bool reduceBlock(IRBuilder& builder, BasicBlock& block, const BlockReducer& reducer) {
  auto& insts = block.insts();
  bool modified = false;
  const auto oldSize = insts.size();
  for (auto iter = insts.begin(); iter != insts.end(); iter++) {
    auto inst = *iter;

    builder.set_pos(&block, iter);
    if (auto value = reducer(inst)) {
      // assert(value != inst);
      if(value == inst) {
        std::cerr << "Warning: reduceBlock: value == inst" << std::endl;
        assert(false);
      }
      // modified |= ins
      inst->replaceAllUseWith(value);
      modified = true;
      // std::cerr << "Replaced!" << std::endl;
    }
  }
  const auto newSize = insts.size();
  modified |= (newSize != oldSize);
  return modified;
}
/*
block:
  ...
  inst1
  after
  inst2
  ...

otherblock:

=>
preBlock:
  ...
  inst1
  after

postBlock:
  inst2
  ...

otherblock:

*/
BasicBlock* splitBlock(BasicBlockList& blocks,
                       BasicBlockList::iterator blockIter,
                       InstructionList::iterator after) {
  auto preBlock = *blockIter;
  auto postBlock = utils::make<BasicBlock>("", preBlock->function());
  // auto postBlock = preBlock->function()->newBlock();
  const auto beg = std::next(after);
  const auto end = preBlock->insts().end();

  for (auto iter = beg; iter != end;) {
    auto next = std::next(iter);
    postBlock->emplace_back_inst(*iter);
    preBlock->insts().erase(iter);
    iter = next;
  }
  // insert postBlock after preBlock
  blocks.insert(std::next(blockIter), postBlock);
  return postBlock;
}

}  // namespace pass