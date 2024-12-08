#include "mir/utils.hpp"
#include "target/riscv/RISCV.hpp"
namespace mir {
struct StackObjectInterval final {
  uint32_t begin, end;
};
using Intervals =
  std::unordered_map<MIROperand, StackObjectInterval*, MIROperandHasher>;

struct Slot final {
  uint32_t color;
  uint32_t end;

  bool operator<(const Slot& rhs) const noexcept { return end > rhs.end; }
};

static void remove_unused_spill_stack_objects(MIRFunction* mfunc) {
  std::unordered_set<MIROperand, MIROperandHasher> spill_stack;
  /* find the unused spill_stack */
  {
    for (auto& [op, stack] : mfunc->stackObjs()) {
      assert(isStackObject(op.reg()));
      if (stack.usage == StackObjectUsage::RegSpill) spill_stack.emplace(op);
    }
    for (auto& block : mfunc->blocks()) {
      for (auto inst : block->insts()) {
        if (inst->opcode() == InstLoadRegFromStack) {
          /* LoadRegFromStack dst, obj */
          spill_stack.erase(inst->operand(1));
        }
      }
    }
  }

  /* remove dead store */
  {
    for (auto& block : mfunc->blocks()) {
      block->insts().remove_if([&](auto inst) {
        /* StoreRegToStack obj, src */
        return inst->opcode() == InstStoreRegToStack &&
               spill_stack.count(inst->operand(0));
      });
    }
    for (auto op : spill_stack) {
      mfunc->stackObjs().erase(op);
    }
  }
}
#include "support/StaticReflection.hpp"
/*
 * Stack Layout
 * ------------------------ <----- Last sp
 * Locals & Spills
 * ------------------------
 * Return Address
 * ------------------------
 * Callee Arguments
 * ------------------------ <----- Current sp
 */

/**
 * allocateStackObjects after register allocation
 * process:
 * - Allocate stack objs for callee saved registers
 *  - 1. collect all callee saved registers that defined by the function.
 *  - 2. insert prologue and epilogue: save and restore them
 *  - 3. but corresponding stack objs' offset is not determined yet
 * - Determine stack objs' offset:
 *  - 0. CalleeArgument StackObjs:
 *    - in Lowering from ir to mir function Stage, already determined their
 * offset
 *  - 1. CalleeSaved StackObjs
 *  - 2. Local and RegSpill StackObjs
 *  - 3. Argument StackObjs
 *  - 4
 */
void allocateStackObjects(MIRFunction* func, CodeGenContext& ctx) {

  constexpr bool debugSA = false;
  auto dumpOperand = [&](MIROperand op) {
    std::cerr << mir::RISCV::OperandDumper{op} << std::endl;
  };
  // func is like a callee
  /* 1. callee saved: 被调用者需要保存的寄存器 */
  /* find all callee saved registers that defined by the function, these
   * registers will be saved in the prologue and restored in the epilogue */
  std::unordered_set<uint32_t> calleeSavedRegs;
  for (auto& block : func->blocks()) {
    forEachDefOperand(*block, ctx, [&](MIROperand op) {
      if (op.isUnused()) return;
      if (op.isReg() && isISAReg(op.reg()) &&
          ctx.frameInfo.isCalleeSaved(op)) {
        if (debugSA) dumpOperand(op);
        calleeSavedRegs.insert(op.reg());
      }
    });
  }
  std::unordered_set<MIROperand, MIROperandHasher> calleeSaved;
  for (auto reg : calleeSavedRegs) {
    calleeSaved.emplace(MIROperand::asISAReg(
      reg, ctx.registerInfo->getCanonicalizedRegisterType(reg)));
  }
  // insert prologue and epilogue
  const auto preAllocatedBase = ctx.frameInfo.insertPrologueEpilogue(
    func, calleeSaved, ctx, ctx.registerInfo->get_return_address_register());

  /* 优化: remove dead code */
  remove_unused_spill_stack_objects(func);
  // determine stack objs' offset
  int32_t allocationBase = 0;
  auto sp_alignment = static_cast<int32_t>(ctx.frameInfo.stackPointerAlign());

  auto align_to = [&](int32_t alignment) {
    assert(alignment <= sp_alignment);
    allocationBase = utils::alignTo(allocationBase, alignment);
  };

  /* 2. determine stack layout for callee arguments, calculate offset  */
  for (auto& [ref, stack] : func->stackObjs()) {
    assert(isStackObject(ref.reg()));
    if (stack.usage == StackObjectUsage::CalleeArgument) {
      allocationBase = std::max(
        allocationBase, stack.offset + static_cast<int32_t>(stack.size));
    }
  }

  align_to(sp_alignment);

  /* 3. local variables */

  std::vector<MIROperand> local_callee_saved;
  for (auto& [op, stack] : func->stackObjs()) {
    if (stack.usage == StackObjectUsage::CalleeSaved) {
      local_callee_saved.push_back(op);
    }
  }

  // sort:
  std::sort(local_callee_saved.begin(), local_callee_saved.end(),
            [&](const MIROperand lhs, const MIROperand rhs) {
              return lhs.reg() > rhs.reg();
            });

  auto allocate_for = [&](StackObject& stack) {
    align_to(static_cast<int32_t>(stack.alignment));
    stack.offset = allocationBase;
    allocationBase += static_cast<int32_t>(stack.size);
    if (debugSA) {
      std::cerr << "allocate for: " << utils::enumName(stack.usage)
                << ", offset: " << stack.offset << ", size: " << stack.size
                << ", align: " << stack.alignment << std::endl;
      std::cerr << "allocationBase: " << allocationBase << std::endl;
    }
  };
  // allocate for callee saved registers
  for (auto op : local_callee_saved) {
    allocate_for(func->stackObjs().at(op));
  }
  // allocate for local variables and spills
  for (auto& [op, stack] : func->stackObjs()) {
    if (stack.usage == StackObjectUsage::Local ||
        stack.usage == StackObjectUsage::RegSpill) {
      allocate_for(stack);
    }
  }
  // allocate for my arguments
  /**
   * in Lowering from ir to mir function Stage, already use
   * FrameInfo::emit_prologue() to allocate stack for arguments, and their
   * offset is already set (preAllocated), have preAllocatedBase.
   * their actual offset = original offset + preAllocatedBase + stack_size.
   * stack_size is the size of stack objects excluding arguments.
   */
  align_to(sp_alignment);

  auto gap = utils::alignTo(preAllocatedBase, sp_alignment) - preAllocatedBase;

  auto stack_size = allocationBase + gap;
  assert((stack_size + preAllocatedBase) % sp_alignment == 0);
  if (debugSA) {
    std::cerr << "allocationBase: " << allocationBase << std::endl;
    std::cerr << "gap: " << gap << std::endl;
    std::cerr << "stack_size: " << stack_size << std::endl;
    std::cerr << "preAllocatedBase: " << preAllocatedBase << std::endl;
  }
  for (auto& [op, stack] : func->stackObjs()) {
    assert(isStackObject(op.reg()));
    if (stack.usage == StackObjectUsage::Argument) {
      stack.offset += stack_size + preAllocatedBase;
    }
  }

  /* 4. emit prologue and epilogue */
  ctx.frameInfo.emitPostSAPrologue(func->blocks().front().get(), stack_size);

  for (auto& block : func->blocks()) {
    auto terminator = block->insts().back();
    auto& instInfo = ctx.instInfo.getInstInfo(terminator);
    if (requireFlag(instInfo.inst_flag(), InstFlagReturn)) {
      ctx.frameInfo.emitPostSAEpilogue(block.get(), stack_size);
    }
  }
}

}  // namespace mir
