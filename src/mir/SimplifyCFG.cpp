#include "mir/MIR.hpp"
#include "mir/utils.hpp"

namespace mir {
  
template <typename Func>
static void traverseInstsInFunction(MIRFunction& func,
                                    Func funcInst,
                                    std::ostream& os = std::cerr,
                                    bool reverse = false,
                                    bool debug = false) {
  for (auto& block : func.blocks()) {
    if (debug) os << "Traversing block: " << block->name() << std::endl;
    for (auto& inst : block->insts()) {
      if (debug) os << "Traversing inst: " << inst << std::endl;
      funcInst(inst);
    }
  }
}

/**
 * block:
 *    ...
 *    br nextBlock
 * nextBlock:
 *    ...
 *    ...
 * ->
 * block:
 *    ...
 * nextBlock:
 *    ...
 *    ...
 *
 */
static bool removeGotoNext(MIRFunction& func, const CodeGenContext& ctx) {
  bool modified = false;
  for (auto iter = func.blocks().begin(); iter != func.blocks().end(); ++iter) {
    const auto next = std::next(iter);
    if (next == func.blocks().end()) {
      break;
    }

    const auto& block = *iter;
    const auto& nextBlock = next->get();

    while (not block->insts().empty()) {
      const auto lastInst = block->insts().back();
      MIRBlock* targetBlock = nullptr;
      if (ctx.instInfo.matchUnconditionalBranch(lastInst, targetBlock) &&
          targetBlock == nextBlock) {
        block->insts().pop_back();
        modified = true;
      } else {
        break;
      }
    }
  }
  return modified;
}

/**
 *
 * emptyBlock:
 * nextBlock:
 *    ...
 *    ...
 *
 *
 *    ...
 *
 * otherBlock:
 *    br emptyBlock
 *
 * ->
 * otherBlock:
 *    br nextBlock

 */

static bool removeEmptyBlocks(MIRFunction& func, const CodeGenContext& ctx) {
  std::unordered_map<MIRBlock*, MIRBlock*> redirects;
  std::vector<MIRBlock*> consecutiveEmptyBlocks;

  const auto rediectTo = [&](MIRBlock* target) {
    for (auto block : consecutiveEmptyBlocks) {
      redirects[block] = target;
    }

    consecutiveEmptyBlocks.clear();
  };

  /* gther bracch infomation */
  for (auto& block : func.blocks()) {
    if (block->insts().empty()) {
      consecutiveEmptyBlocks.push_back(block.get());
    } else {
      rediectTo(block.get());
    }
  }
  traverseInstsInFunction(func, [&](MIRInst* inst) {
    MIRBlock* targetBlock = nullptr;
    double prob;
    if (ctx.instInfo.matchBranch(inst, targetBlock, prob)) {
      /* redirect */
      if (const auto iter = redirects.find(targetBlock); iter != redirects.end()) {
        ctx.instInfo.redirectBranch(inst, iter->second);
      }
    }
  });

  func.blocks().remove_if([&](auto& ptr) { return redirects.count(ptr.get()); });

  return !redirects.empty();
}

static bool removeConsecutiveJump(MIRFunction& func, const CodeGenContext& ctx) {
  std::unordered_map<MIRBlock*, MIRBlock*> redirects;
  for (auto& block : func.blocks()) {
    if (block->insts().size() != 1) continue;
    // only one jump in block
    const auto& inst = block->insts().front();
    MIRBlock* targetBlock = nullptr;
    if (ctx.instInfo.matchUnconditionalBranch(inst, targetBlock)) {
      redirects[block.get()] = targetBlock;
    }
  }

  if (redirects.empty()) return false;

  bool modified = false;

  traverseInstsInFunction(func, [&](MIRInst* inst) {
    MIRBlock* targetBlock = nullptr;
    double prob;
    /**
     * inst: jump targetBlock
     *
     * targetBlock in redirects -> redirectBlock, then redirect jump to
     * redirectBlock
     */
    if (not ctx.instInfo.matchBranch(inst, targetBlock, prob)) return;

    if (const auto iter = redirects.find(targetBlock); iter != redirects.end()) {
      ctx.instInfo.redirectBranch(inst, iter->second);
      modified = true;
    }
  });
  return modified;
}

void simplifyCFG(MIRFunction& func, CodeGenContext& ctx) {
  while (true) {
    bool modified = false;
    modified |= removeGotoNext(func, ctx);
    modified |= removeEmptyBlocks(func, ctx);
    modified |= removeConsecutiveJump(func, ctx);
    modified |= genericPeepholeOpt(func, ctx);
    if (not modified) break;
  }
}

}  // namespace mir