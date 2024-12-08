#pragma once
#include "mir/MIR.hpp"
#include <unordered_set>

/*
1. lowering stage
- emit_call
- emit_prologue
- emit_return

2. ra stage - register allocation
- is_caller_saved(MIROperand& op)
- is_callee_saved(MIROperand& op)

3. sa stage - stack allocation
- stack_pointer_align
- emit_postsa_prologue
- emit_postsa_epilogue
- insert_prologue_epilogue
*/
namespace mir {
class LoweringContext;
class TargetFrameInfo {
public:
  TargetFrameInfo() = default;
  virtual ~TargetFrameInfo() = default;

public:  // lowering stage
  // clang-format off
  virtual void emitCall(ir::CallInst* inst, LoweringContext& lowering_ctx) = 0;
  virtual void emitPrologue(MIRFunction* func, LoweringContext& lowering_ctx) = 0;
  virtual void emitReturn(ir::ReturnInst* inst, LoweringContext& lowering_ctx) = 0;
  // clang-format on

public:  // ra stage (register allocation)
  virtual bool isCallerSaved(const MIROperand& op) = 0;
  virtual bool isCalleeSaved(const MIROperand& op) = 0;

public:  // sa stage (stack allocation)
  virtual int stackPointerAlign() = 0;
  virtual void emitPostSAPrologue(MIRBlock* entry, int32_t stack_size) = 0;
  virtual void emitPostSAEpilogue(MIRBlock* exit, int32_t stack_size) = 0;

  /* 插入序言和尾声代码: callee-saved registers 寄存器保护与恢复 */
  virtual int32_t insertPrologueEpilogue(
    MIRFunction* func,
    std::unordered_set<MIROperand, MIROperandHasher>& callee_saved_regs,
    CodeGenContext& ctx,
    MIROperand return_addr_reg) = 0;
};
}  // namespace mir