#include "mir/RegisterAllocator.hpp"
#include "mir/LiveInterval.hpp"
#include "mir/target.hpp"
#include "support/StaticReflection.hpp"
#include "target/riscv/RISCV.hpp"

namespace mir {
void IPRAUsageCache::add(const CodeGenContext& ctx, MIRFunction& mfunc) {
  constexpr bool Debug = false;
  IPRAInfo info;

  const auto collect = [&](MIRInst* inst) {
    auto& instInfo = ctx.instInfo.getInstInfo(inst);

    /* 判断该函数中是否使用到Caller Saved Register */
    for (size_t idx = 0; idx < instInfo.operand_num(); idx++) {
      auto op = inst->operand(idx);
      if (!isOperandISAReg(op)) continue;
      if (ctx.frameInfo.isCallerSaved(op)) info.emplace(op.reg());
    }

    /* 遇到Call指令 */
    if (requireFlag(instInfo.inst_flag(), InstFlagCall)) {
      auto callee = inst->operand(0).reloc();
      if (callee->name() != mfunc.name()) {  // 非递归的情况
        auto calleeInfo = query(callee->name());
        if (calleeInfo)
          for (auto reg : *calleeInfo)
            info.emplace(reg);
        else
          return false;
      }
    }
    return true;
  };

  for (auto& block : mfunc.blocks()) {
    for (auto inst : block->insts()) {
      // if collect false, means the callee function is not analyzed yet,
      // so we should return and wait for the next time
      if(not collect(inst)) return;
    }
  }
  mCache.emplace(mfunc.name(), std::move(info));
  if (Debug) dump(std::cerr, mfunc.name());
}

void IPRAUsageCache::add(std::string symbol, IPRAInfo info) {
  mCache.emplace(symbol, std::move(info));
}

const IPRAInfo* IPRAUsageCache::query(std::string calleeFunc) const {
  if (auto iter = mCache.find(calleeFunc); iter != mCache.cend()) return &(iter->second);
  return nullptr;
}
void IPRAUsageCache::dump(std::ostream& out, std::string calleeFunc) const {
  std::cerr << "Debug function " << calleeFunc << ": \n";
  for (auto reg : mCache.at(calleeFunc)) {
    std::cerr << "\t" << utils::enumName(static_cast<RISCV::RISCVRegister>(reg)) << "\n";
  }
  std::cerr << "\n";
}
};  // namespace mir