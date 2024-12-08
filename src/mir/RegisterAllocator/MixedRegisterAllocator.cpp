#include "mir/MIR.hpp"
#include "mir/RegisterAllocator.hpp"

namespace mir {

static size_t collectVregNumber(MIRFunction& mfunc, CodeGenContext& ctx) {
  size_t vregNum = 0;
  for (auto& block : mfunc.blocks()) {
    for (auto inst : block->insts()) {
      auto& info = ctx.instInfo.getInstInfo(inst);
      for (size_t idx = 0; idx < info.operand_num(); idx++) {
        const auto& op = inst->operand(idx);
        if (op.isReg() and isVirtualReg(op.reg())) {
          vregNum++;
        }
      }
    }
  }
  return vregNum;
}

static size_t VregNumThreshold = 3000;

void mixedRegisterAllocate(MIRFunction& mfunc, CodeGenContext& ctx, IPRAUsageCache& infoIPRA) {
  const auto vregNum = collectVregNumber(mfunc, ctx);
  // std::cerr << "vregNum: " << vregNum;
  if (vregNum > VregNumThreshold) {
    // std::cerr << ", using fast allocator beta" << std::endl;
    intraBlockAllocate(mfunc, ctx, infoIPRA);
  } else {
    // std::cerr << ", using graph coloring allocator" << std::endl;
    graphColoringAllocate(mfunc, ctx, infoIPRA);
  }
}

}  // namespace mir