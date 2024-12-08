// Automatically generated file, do not edit!

#include "autogen/riscv/ISelInfoDecl.hpp"

RISCV_NAMESPACE_BEGIN

static bool matchInstJump(MIRInst* inst, MIROperand& target) {
  if (inst->opcode() != InstJump) return false;
  target = inst->operand(0);
  return true;
}

static bool matchInstBranch(MIRInst* inst, MIROperand& cond, MIROperand& target, MIROperand& prob) {
  if (inst->opcode() != InstBranch) return false;
  cond = inst->operand(0);
  target = inst->operand(1);
  prob = inst->operand(2);
  return true;
}

static bool matchInstUnreachable(MIRInst* inst) {
  if (inst->opcode() != InstUnreachable) return false;

  return true;
}

static bool matchInstLoad(MIRInst* inst, MIROperand& dst, MIROperand& addr, MIROperand& align) {
  if (inst->opcode() != InstLoad) return false;
  dst = inst->operand(0);
  addr = inst->operand(1);
  align = inst->operand(2);
  return true;
}

static bool matchInstStore(MIRInst* inst, MIROperand& addr, MIROperand& src, MIROperand& align) {
  if (inst->opcode() != InstStore) return false;
  addr = inst->operand(0);
  src = inst->operand(1);
  align = inst->operand(2);
  return true;
}

static bool matchInstAdd(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstAdd) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstSub(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstSub) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstMul(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstMul) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstUDiv(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstUDiv) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstURem(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstURem) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstAnd(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstAnd) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstOr(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstOr) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstXor(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstXor) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstShl(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstShl) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstLShr(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstLShr) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstAShr(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstAShr) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstSDiv(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstSDiv) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstSRem(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstSRem) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstSMin(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstSMin) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstSMax(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstSMax) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstNeg(MIRInst* inst, MIROperand& dst, MIROperand& src) {
  if (inst->opcode() != InstNeg) return false;
  dst = inst->operand(0);
  src = inst->operand(1);
  return true;
}

static bool matchInstAbs(MIRInst* inst, MIROperand& dst, MIROperand& src) {
  if (inst->opcode() != InstAbs) return false;
  dst = inst->operand(0);
  src = inst->operand(1);
  return true;
}

static bool matchInstFAdd(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstFAdd) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstFSub(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstFSub) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstFMul(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstFMul) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstFDiv(MIRInst* inst, MIROperand& dst, MIROperand& src1, MIROperand& src2) {
  if (inst->opcode() != InstFDiv) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  return true;
}

static bool matchInstFNeg(MIRInst* inst, MIROperand& dst, MIROperand& src) {
  if (inst->opcode() != InstFNeg) return false;
  dst = inst->operand(0);
  src = inst->operand(1);
  return true;
}

static bool matchInstFAbs(MIRInst* inst, MIROperand& dst, MIROperand& src) {
  if (inst->opcode() != InstFAbs) return false;
  dst = inst->operand(0);
  src = inst->operand(1);
  return true;
}

static bool matchInstFFma(MIRInst* inst,
                          MIROperand& dst,
                          MIROperand& src1,
                          MIROperand& src2,
                          MIROperand& acc) {
  if (inst->opcode() != InstFFma) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  acc = inst->operand(3);
  return true;
}

static bool matchInstICmp(MIRInst* inst,
                          MIROperand& dst,
                          MIROperand& src1,
                          MIROperand& src2,
                          MIROperand& op) {
  if (inst->opcode() != InstICmp) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  op = inst->operand(3);
  return true;
}

static bool matchInstFCmp(MIRInst* inst,
                          MIROperand& dst,
                          MIROperand& src1,
                          MIROperand& src2,
                          MIROperand& op) {
  if (inst->opcode() != InstFCmp) return false;
  dst = inst->operand(0);
  src1 = inst->operand(1);
  src2 = inst->operand(2);
  op = inst->operand(3);
  return true;
}

static bool matchInstSExt(MIRInst* inst, MIROperand& dst, MIROperand& src) {
  if (inst->opcode() != InstSExt) return false;
  dst = inst->operand(0);
  src = inst->operand(1);
  return true;
}

static bool matchInstZExt(MIRInst* inst, MIROperand& dst, MIROperand& src) {
  if (inst->opcode() != InstZExt) return false;
  dst = inst->operand(0);
  src = inst->operand(1);
  return true;
}

static bool matchInstTrunc(MIRInst* inst, MIROperand& dst, MIROperand& src) {
  if (inst->opcode() != InstTrunc) return false;
  dst = inst->operand(0);
  src = inst->operand(1);
  return true;
}

static bool matchInstF2U(MIRInst* inst, MIROperand& dst, MIROperand& src) {
  if (inst->opcode() != InstF2U) return false;
  dst = inst->operand(0);
  src = inst->operand(1);
  return true;
}

static bool matchInstF2S(MIRInst* inst, MIROperand& dst, MIROperand& src) {
  if (inst->opcode() != InstF2S) return false;
  dst = inst->operand(0);
  src = inst->operand(1);
  return true;
}

static bool matchInstU2F(MIRInst* inst, MIROperand& dst, MIROperand& src) {
  if (inst->opcode() != InstU2F) return false;
  dst = inst->operand(0);
  src = inst->operand(1);
  return true;
}

static bool matchInstS2F(MIRInst* inst, MIROperand& dst, MIROperand& src) {
  if (inst->opcode() != InstS2F) return false;
  dst = inst->operand(0);
  src = inst->operand(1);
  return true;
}

static bool matchInstFCast(MIRInst* inst, MIROperand& dst) {
  if (inst->opcode() != InstFCast) return false;
  dst = inst->operand(0);
  return true;
}

static bool matchInstCopy(MIRInst* inst, MIROperand& dst, MIROperand& src) {
  if (inst->opcode() != InstCopy) return false;
  dst = inst->operand(0);
  src = inst->operand(1);
  return true;
}

static bool matchInstSelect(MIRInst* inst,
                            MIROperand& dst,
                            MIROperand& cond,
                            MIROperand& src1,
                            MIROperand& src2) {
  if (inst->opcode() != InstSelect) return false;
  dst = inst->operand(0);
  cond = inst->operand(1);
  src1 = inst->operand(2);
  src2 = inst->operand(3);
  return true;
}

static bool matchInstLoadGlobalAddress(MIRInst* inst, MIROperand& dst, MIROperand& addr) {
  if (inst->opcode() != InstLoadGlobalAddress) return false;
  dst = inst->operand(0);
  addr = inst->operand(1);
  return true;
}

static bool matchInstLoadImm(MIRInst* inst, MIROperand& dst, MIROperand& imm) {
  if (inst->opcode() != InstLoadImm) return false;
  dst = inst->operand(0);
  imm = inst->operand(1);
  return true;
}

static bool matchInstLoadStackObjectAddr(MIRInst* inst, MIROperand& dst, MIROperand& obj) {
  if (inst->opcode() != InstLoadStackObjectAddr) return false;
  dst = inst->operand(0);
  obj = inst->operand(1);
  return true;
}

static bool matchInstCopyFromReg(MIRInst* inst, MIROperand& dst, MIROperand& src) {
  if (inst->opcode() != InstCopyFromReg) return false;
  dst = inst->operand(0);
  src = inst->operand(1);
  return true;
}

static bool matchInstCopyToReg(MIRInst* inst, MIROperand& dst, MIROperand& src) {
  if (inst->opcode() != InstCopyToReg) return false;
  dst = inst->operand(0);
  src = inst->operand(1);
  return true;
}

static bool matchInstLoadImmToReg(MIRInst* inst, MIROperand& dst, MIROperand& imm) {
  if (inst->opcode() != InstLoadImmToReg) return false;
  dst = inst->operand(0);
  imm = inst->operand(1);
  return true;
}

static bool matchInstLoadRegFromStack(MIRInst* inst, MIROperand& dst, MIROperand& obj) {
  if (inst->opcode() != InstLoadRegFromStack) return false;
  dst = inst->operand(0);
  obj = inst->operand(1);
  return true;
}

static bool matchInstStoreRegToStack(MIRInst* inst, MIROperand& obj, MIROperand& src) {
  if (inst->opcode() != InstStoreRegToStack) return false;
  obj = inst->operand(0);
  src = inst->operand(1);
  return true;
}

static bool matchInstReturn(MIRInst* inst) {
  if (inst->opcode() != InstReturn) return false;

  return true;
}

static bool matchInstAtomicAdd(MIRInst* inst, MIROperand& dst, MIROperand& addr, MIROperand& src) {
  if (inst->opcode() != InstAtomicAdd) return false;
  dst = inst->operand(0);
  addr = inst->operand(1);
  src = inst->operand(2);
  return true;
}

/* InstLoadGlobalAddress matchAndSelectPatternInstLoadGlobalAddress begin */
static bool matchAndSelectPattern1(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstLoadGlobalAddress;
  /** Match Inst **/
  /* match inst InstLoadGlobalAddress */
  MIROperand op1;
  MIROperand op2;
  if (not matchInstLoadGlobalAddress(inst1, op1, op2)) {
    return false;
  }

  /** Select Inst **/
  auto op4 = (getVRegAs(ctx, op1));
  auto op5 = (op2);
  /* select inst LLA */
  auto inst2 = ctx.insertMIRInst(LLA, {op4, op5});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern2(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstLoadGlobalAddress;
  /** Match Inst **/
  /* match inst InstLoadGlobalAddress */
  MIROperand op1;
  MIROperand op2;
  if (not matchInstLoadGlobalAddress(inst1, op1, op2)) {
    return false;
  }

  /** Select Inst **/
  auto op4 = (op1);
  auto op6 = (getVRegAs(ctx, op1));
  auto op7 = (getHighBits(op2));
  /* select inst AUIPC */
  auto inst2 = ctx.insertMIRInst(AUIPC, {op6, op7});

  auto op5 = ctx.getInstDefOperand(inst2);

  auto op8 = (getLowBits(op2));
  /* select inst ADDI */
  auto inst3 = ctx.insertMIRInst(ADDI, {op4, op5, op8});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst3));
  ctx.remove_inst(inst1);
  return true;
}

/* InstLoadGlobalAddress matchAndSelectPatternInstLoadGlobalAddressend */

/* InstLoad matchAndSelectPatternInstLoad begin */
static bool matchAndSelectPattern3(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstLoad;
  /** Match Inst **/
  /* match inst InstLoad */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstLoad(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  MIROperand op4;
  MIROperand op5;
  if (not(selectAddrOffset(op2, ctx, op4, op5))) {
    return false;
  }

  /** Select Inst **/
  auto op7 = (op1);
  auto op8 = (op4);
  auto op9 = (op5);
  /* select inst getLoadOpcode(op1) */
  auto inst2 = ctx.insertMIRInst(getLoadOpcode(op1), {op7, op9, op8});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstLoad matchAndSelectPatternInstLoadend */

/* InstStore matchAndSelectPatternInstStore begin */
static bool matchAndSelectPattern4(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstStore;
  /** Match Inst **/
  /* match inst InstStore */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstStore(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  MIROperand op4;
  MIROperand op5;
  if (not(isOperandVRegORISAReg(op2) && selectAddrOffset(op1, ctx, op4, op5))) {
    return false;
  }

  /** Select Inst **/
  auto op7 = (op2);
  auto op8 = (op4);
  auto op9 = (op5);
  /* select inst getStoreOpcode(op2) */
  auto inst2 = ctx.insertMIRInst(getStoreOpcode(op2), {op7, op9, op8});

  ctx.remove_inst(inst1);
  return true;
}

/* InstStore matchAndSelectPatternInstStoreend */

/* InstJump matchAndSelectPatternInstJump begin */
static bool matchAndSelectPattern5(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstJump;
  /** Match Inst **/
  /* match inst InstJump */
  MIROperand op1;
  if (not matchInstJump(inst1, op1)) {
    return false;
  }

  /** Select Inst **/
  auto op3 = (op1);
  /* select inst J */
  auto inst2 = ctx.insertMIRInst(J, {op3});

  ctx.remove_inst(inst1);
  return true;
}

/* InstJump matchAndSelectPatternInstJumpend */

/* InstLoadImm matchAndSelectPatternInstLoadImm begin */
static bool matchAndSelectPattern6(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstLoadImm;
  /** Match Inst **/
  /* match inst InstLoadImm */
  MIROperand op1;
  MIROperand op2;
  if (not matchInstLoadImm(inst1, op1, op2)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isZero(op2))) {
    return false;
  }

  /** Select Inst **/
  auto op4 = (op1);
  auto op5 = (getZero(op1));
  /* select inst MV */
  auto inst2 = ctx.insertMIRInst(MV, {op4, op5});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern8(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstLoadImm;
  /** Match Inst **/
  /* match inst InstLoadImm */
  MIROperand op1;
  MIROperand op2;
  if (not matchInstLoadImm(inst1, op1, op2)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandNonZeroImm12(op2))) {
    return false;
  }

  /** Select Inst **/
  auto op4 = (op1);
  auto op5 = (op2);
  /* select inst LoadImm12 */
  auto inst2 = ctx.insertMIRInst(LoadImm12, {op4, op5});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern10(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstLoadImm;
  /** Match Inst **/
  /* match inst InstLoadImm */
  MIROperand op1;
  MIROperand op2;
  if (not matchInstLoadImm(inst1, op1, op2)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandNonZeroImm32(op2))) {
    return false;
  }

  /** Select Inst **/
  auto op4 = (op1);
  auto op5 = (op2);
  /* select inst LoadImm32 */
  auto inst2 = ctx.insertMIRInst(LoadImm32, {op4, op5});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstLoadImm matchAndSelectPatternInstLoadImmend */

/* InstLoadImmToReg matchAndSelectPatternInstLoadImmToReg begin */
static bool matchAndSelectPattern7(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstLoadImmToReg;
  /** Match Inst **/
  /* match inst InstLoadImmToReg */
  MIROperand op1;
  MIROperand op2;
  if (not matchInstLoadImmToReg(inst1, op1, op2)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isZero(op2))) {
    return false;
  }

  /** Select Inst **/
  auto op4 = (op1);
  auto op5 = (getZero(op1));
  /* select inst MV */
  auto inst2 = ctx.insertMIRInst(MV, {op4, op5});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern9(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstLoadImmToReg;
  /** Match Inst **/
  /* match inst InstLoadImmToReg */
  MIROperand op1;
  MIROperand op2;
  if (not matchInstLoadImmToReg(inst1, op1, op2)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandNonZeroImm12(op2))) {
    return false;
  }

  /** Select Inst **/
  auto op4 = (op1);
  auto op5 = (op2);
  /* select inst LoadImm12 */
  auto inst2 = ctx.insertMIRInst(LoadImm12, {op4, op5});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern11(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstLoadImmToReg;
  /** Match Inst **/
  /* match inst InstLoadImmToReg */
  MIROperand op1;
  MIROperand op2;
  if (not matchInstLoadImmToReg(inst1, op1, op2)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandNonZeroImm32(op2))) {
    return false;
  }

  /** Select Inst **/
  auto op4 = (op1);
  auto op5 = (op2);
  /* select inst LoadImm32 */
  auto inst2 = ctx.insertMIRInst(LoadImm32, {op4, op5});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstLoadImmToReg matchAndSelectPatternInstLoadImmToRegend */

/* InstAdd matchAndSelectPatternInstAdd begin */
static bool matchAndSelectPattern12(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstAdd;
  /** Match Inst **/
  /* match inst InstAdd */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstAdd(inst1, op1, op2, op3)) {
    return false;
  } /* lookup inst that define the operand2 */
  auto inst2 = ctx.lookupDefInst(op2);
  if (not inst2) {
    return false;
  }

  /* match inst InstShl */
  MIROperand op4;
  MIROperand op5;
  MIROperand op6;
  if (not matchInstShl(inst2, op4, op5, op6)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op1) && isOperandIReg(op5) && isOperandIReg(op3) && (op6).isImm() &&
          (op6).imm() == 1)) {
    return false;
  }

  /** Select Inst **/
  auto op8 = (op1);
  auto op9 = (op5);
  auto op10 = (op3);
  /* select inst SH1ADD */
  auto inst3 = ctx.insertMIRInst(SH1ADD, {op8, op9, op10});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst3));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern13(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstAdd;
  /** Match Inst **/
  /* match inst InstAdd */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstAdd(inst1, op1, op2, op3)) {
    return false;
  } /* lookup inst that define the operand3 */
  auto inst2 = ctx.lookupDefInst(op3);
  if (not inst2) {
    return false;
  }

  /* match inst InstShl */
  MIROperand op4;
  MIROperand op5;
  MIROperand op6;
  if (not matchInstShl(inst2, op4, op5, op6)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op1) && isOperandIReg(op2) && isOperandIReg(op5) && (op6).isImm() &&
          (op6).imm() == 1)) {
    return false;
  }

  /** Select Inst **/
  auto op8 = (op1);
  auto op9 = (op5);
  auto op10 = (op2);
  /* select inst SH1ADD */
  auto inst3 = ctx.insertMIRInst(SH1ADD, {op8, op9, op10});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst3));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern14(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstAdd;
  /** Match Inst **/
  /* match inst InstAdd */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstAdd(inst1, op1, op2, op3)) {
    return false;
  } /* lookup inst that define the operand2 */
  auto inst2 = ctx.lookupDefInst(op2);
  if (not inst2) {
    return false;
  }

  /* match inst InstShl */
  MIROperand op4;
  MIROperand op5;
  MIROperand op6;
  if (not matchInstShl(inst2, op4, op5, op6)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op1) && isOperandIReg(op5) && isOperandIReg(op3) && (op6).isImm() &&
          (op6).imm() == 2)) {
    return false;
  }

  /** Select Inst **/
  auto op8 = (op1);
  auto op9 = (op5);
  auto op10 = (op3);
  /* select inst SH2ADD */
  auto inst3 = ctx.insertMIRInst(SH2ADD, {op8, op9, op10});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst3));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern15(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstAdd;
  /** Match Inst **/
  /* match inst InstAdd */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstAdd(inst1, op1, op2, op3)) {
    return false;
  } /* lookup inst that define the operand3 */
  auto inst2 = ctx.lookupDefInst(op3);
  if (not inst2) {
    return false;
  }

  /* match inst InstShl */
  MIROperand op4;
  MIROperand op5;
  MIROperand op6;
  if (not matchInstShl(inst2, op4, op5, op6)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op1) && isOperandIReg(op2) && isOperandIReg(op5) && (op6).isImm() &&
          (op6).imm() == 2)) {
    return false;
  }

  /** Select Inst **/
  auto op8 = (op1);
  auto op9 = (op5);
  auto op10 = (op2);
  /* select inst SH2ADD */
  auto inst3 = ctx.insertMIRInst(SH2ADD, {op8, op9, op10});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst3));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern16(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstAdd;
  /** Match Inst **/
  /* match inst InstAdd */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstAdd(inst1, op1, op2, op3)) {
    return false;
  } /* lookup inst that define the operand2 */
  auto inst2 = ctx.lookupDefInst(op2);
  if (not inst2) {
    return false;
  }

  /* match inst InstShl */
  MIROperand op4;
  MIROperand op5;
  MIROperand op6;
  if (not matchInstShl(inst2, op4, op5, op6)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op1) && isOperandIReg(op5) && isOperandIReg(op3) && (op6).isImm() &&
          (op6).imm() == 3)) {
    return false;
  }

  /** Select Inst **/
  auto op8 = (op1);
  auto op9 = (op5);
  auto op10 = (op3);
  /* select inst SH3ADD */
  auto inst3 = ctx.insertMIRInst(SH3ADD, {op8, op9, op10});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst3));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern17(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstAdd;
  /** Match Inst **/
  /* match inst InstAdd */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstAdd(inst1, op1, op2, op3)) {
    return false;
  } /* lookup inst that define the operand3 */
  auto inst2 = ctx.lookupDefInst(op3);
  if (not inst2) {
    return false;
  }

  /* match inst InstShl */
  MIROperand op4;
  MIROperand op5;
  MIROperand op6;
  if (not matchInstShl(inst2, op4, op5, op6)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op1) && isOperandIReg(op2) && isOperandIReg(op5) && (op6).isImm() &&
          (op6).imm() == 3)) {
    return false;
  }

  /** Select Inst **/
  auto op8 = (op1);
  auto op9 = (op5);
  auto op10 = (op2);
  /* select inst SH3ADD */
  auto inst3 = ctx.insertMIRInst(SH3ADD, {op8, op9, op10});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst3));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern18(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstAdd;
  /** Match Inst **/
  /* match inst InstAdd */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstAdd(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI64(op1) && isOperandIReg(op2) && isOperandImm12(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst ADDI */
  auto inst2 = ctx.insertMIRInst(ADDI, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern19(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstAdd;
  /** Match Inst **/
  /* match inst InstAdd */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstAdd(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI64(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst ADD */
  auto inst2 = ctx.insertMIRInst(ADD, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern20(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstAdd;
  /** Match Inst **/
  /* match inst InstAdd */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstAdd(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI32(op1) && isOperandIReg(op2) && isOperandImm12(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst ADDIW */
  auto inst2 = ctx.insertMIRInst(ADDIW, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern21(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstAdd;
  /** Match Inst **/
  /* match inst InstAdd */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstAdd(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI32(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst ADDW */
  auto inst2 = ctx.insertMIRInst(ADDW, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstAdd matchAndSelectPatternInstAddend */

/* InstSub matchAndSelectPatternInstSub begin */
static bool matchAndSelectPattern22(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstSub;
  /** Match Inst **/
  /* match inst InstSub */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstSub(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI64(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst SUB */
  auto inst2 = ctx.insertMIRInst(SUB, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern23(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstSub;
  /** Match Inst **/
  /* match inst InstSub */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstSub(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI32(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst SUBW */
  auto inst2 = ctx.insertMIRInst(SUBW, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstSub matchAndSelectPatternInstSubend */

/* InstMul matchAndSelectPatternInstMul begin */
static bool matchAndSelectPattern24(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstMul;
  /** Match Inst **/
  /* match inst InstMul */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstMul(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI64(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst MUL */
  auto inst2 = ctx.insertMIRInst(MUL, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern25(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstMul;
  /** Match Inst **/
  /* match inst InstMul */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstMul(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI32(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst MULW */
  auto inst2 = ctx.insertMIRInst(MULW, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstMul matchAndSelectPatternInstMulend */

/* InstUDiv matchAndSelectPatternInstUDiv begin */
static bool matchAndSelectPattern26(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstUDiv;
  /** Match Inst **/
  /* match inst InstUDiv */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstUDiv(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI64(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst DIVU */
  auto inst2 = ctx.insertMIRInst(DIVU, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern27(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstUDiv;
  /** Match Inst **/
  /* match inst InstUDiv */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstUDiv(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI32(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst DIVUW */
  auto inst2 = ctx.insertMIRInst(DIVUW, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstUDiv matchAndSelectPatternInstUDivend */

/* InstURem matchAndSelectPatternInstURem begin */
static bool matchAndSelectPattern28(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstURem;
  /** Match Inst **/
  /* match inst InstURem */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstURem(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI64(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst REMU */
  auto inst2 = ctx.insertMIRInst(REMU, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern29(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstURem;
  /** Match Inst **/
  /* match inst InstURem */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstURem(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI32(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst REMUW */
  auto inst2 = ctx.insertMIRInst(REMUW, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstURem matchAndSelectPatternInstURemend */

/* InstAnd matchAndSelectPatternInstAnd begin */
static bool matchAndSelectPattern30(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstAnd;
  /** Match Inst **/
  /* match inst InstAnd */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstAnd(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst getIntegerBinaryRegOpcode(rootOpcode) */
  auto inst2 = ctx.insertMIRInst(getIntegerBinaryRegOpcode(rootOpcode), {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern33(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstAnd;
  /** Match Inst **/
  /* match inst InstAnd */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstAnd(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2) && isOperandImm12(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst getIntegerBinaryImmOpcode(rootOpcode) */
  auto inst2 = ctx.insertMIRInst(getIntegerBinaryImmOpcode(rootOpcode), {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstAnd matchAndSelectPatternInstAndend */

/* InstOr matchAndSelectPatternInstOr begin */
static bool matchAndSelectPattern31(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstOr;
  /** Match Inst **/
  /* match inst InstOr */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstOr(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst getIntegerBinaryRegOpcode(rootOpcode) */
  auto inst2 = ctx.insertMIRInst(getIntegerBinaryRegOpcode(rootOpcode), {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern34(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstOr;
  /** Match Inst **/
  /* match inst InstOr */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstOr(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2) && isOperandImm12(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst getIntegerBinaryImmOpcode(rootOpcode) */
  auto inst2 = ctx.insertMIRInst(getIntegerBinaryImmOpcode(rootOpcode), {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstOr matchAndSelectPatternInstOrend */

/* InstXor matchAndSelectPatternInstXor begin */
static bool matchAndSelectPattern32(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstXor;
  /** Match Inst **/
  /* match inst InstXor */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstXor(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst getIntegerBinaryRegOpcode(rootOpcode) */
  auto inst2 = ctx.insertMIRInst(getIntegerBinaryRegOpcode(rootOpcode), {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern35(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstXor;
  /** Match Inst **/
  /* match inst InstXor */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstXor(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2) && isOperandImm12(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst getIntegerBinaryImmOpcode(rootOpcode) */
  auto inst2 = ctx.insertMIRInst(getIntegerBinaryImmOpcode(rootOpcode), {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstXor matchAndSelectPatternInstXorend */

/* InstShl matchAndSelectPatternInstShl begin */
static bool matchAndSelectPattern36(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstShl;
  /** Match Inst **/
  /* match inst InstShl */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstShl(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI64(op1) && isOperandIReg(op2) && isOperandUImm6(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst SLLI */
  auto inst2 = ctx.insertMIRInst(SLLI, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern37(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstShl;
  /** Match Inst **/
  /* match inst InstShl */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstShl(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI64(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst SLL */
  auto inst2 = ctx.insertMIRInst(SLL, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern38(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstShl;
  /** Match Inst **/
  /* match inst InstShl */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstShl(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI32(op1) && isOperandIReg(op2) && isOperandUImm6(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst SLLIW */
  auto inst2 = ctx.insertMIRInst(SLLIW, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern39(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstShl;
  /** Match Inst **/
  /* match inst InstShl */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstShl(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI32(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst SLLW */
  auto inst2 = ctx.insertMIRInst(SLLW, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstShl matchAndSelectPatternInstShlend */

/* InstLShr matchAndSelectPatternInstLShr begin */
static bool matchAndSelectPattern40(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstLShr;
  /** Match Inst **/
  /* match inst InstLShr */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstLShr(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI64(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst SRL */
  auto inst2 = ctx.insertMIRInst(SRL, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern41(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstLShr;
  /** Match Inst **/
  /* match inst InstLShr */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstLShr(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI32(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst SRLW */
  auto inst2 = ctx.insertMIRInst(SRLW, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstLShr matchAndSelectPatternInstLShrend */

/* InstAShr matchAndSelectPatternInstAShr begin */
static bool matchAndSelectPattern42(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstAShr;
  /** Match Inst **/
  /* match inst InstAShr */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstAShr(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI64(op1) && isOperandIReg(op2) && isOperandUImm6(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst SRAI */
  auto inst2 = ctx.insertMIRInst(SRAI, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern43(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstAShr;
  /** Match Inst **/
  /* match inst InstAShr */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstAShr(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI64(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst SRA */
  auto inst2 = ctx.insertMIRInst(SRA, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern44(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstAShr;
  /** Match Inst **/
  /* match inst InstAShr */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstAShr(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI32(op1) && isOperandIReg(op2) && isOperandUImm6(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst SRAIW */
  auto inst2 = ctx.insertMIRInst(SRAIW, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern45(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstAShr;
  /** Match Inst **/
  /* match inst InstAShr */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstAShr(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI32(op1) && isOperandIReg(op2) && isOperandI32(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst SRAW */
  auto inst2 = ctx.insertMIRInst(SRAW, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstAShr matchAndSelectPatternInstAShrend */

/* InstSDiv matchAndSelectPatternInstSDiv begin */
static bool matchAndSelectPattern46(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstSDiv;
  /** Match Inst **/
  /* match inst InstSDiv */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstSDiv(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI32(op1) && isOperandIReg(op2) && isOperandI32(op3) && (op3).isImm() &&
          (op3).imm() == 2)) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op7 = (getVRegAs64(ctx, op1));
  auto op8 = (op2);
  auto op10 = (getVRegAs64(ctx, op1));
  auto op11 = (op2);
  auto op12 = (MIROperand::asImm(31, OperandType::Int32));
  /* select inst SRLIW */
  auto inst2 = ctx.insertMIRInst(SRLIW, {op10, op11, op12});

  auto op9 = ctx.getInstDefOperand(inst2);

  /* select inst ADD */
  auto inst3 = ctx.insertMIRInst(ADD, {op7, op8, op9});

  auto op6 = ctx.getInstDefOperand(inst3);

  auto op13 = (MIROperand::asImm(1, OperandType::Int32));
  /* select inst SRAIW */
  auto inst4 = ctx.insertMIRInst(SRAIW, {op5, op6, op13});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst4));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern47(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstSDiv;
  /** Match Inst **/
  /* match inst InstSDiv */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstSDiv(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  MIROperand op4;
  if (not(isOperandI32(op1) && isOperandIReg(op2) && select_sdiv32_by_powerof2(op3, op4))) {
    return false;
  }

  /** Select Inst **/
  auto op6 = (op1);
  auto op8 = (getVRegAs64(ctx, op1));
  auto op9 = (op2);
  auto op11 = (getVRegAs64(ctx, op1));
  auto op13 = (getVRegAs64(ctx, op1));
  auto op14 = (op2);
  auto op15 = (MIROperand::asImm(1, OperandType::Int32));
  /* select inst SLLI */
  auto inst2 = ctx.insertMIRInst(SLLI, {op13, op14, op15});

  auto op12 = ctx.getInstDefOperand(inst2);

  auto op16 = (MIROperand::asImm(64 - (op4).imm(), OperandType::Int32));
  /* select inst SRLI */
  auto inst3 = ctx.insertMIRInst(SRLI, {op11, op12, op16});

  auto op10 = ctx.getInstDefOperand(inst3);

  /* select inst ADD */
  auto inst4 = ctx.insertMIRInst(ADD, {op8, op9, op10});

  auto op7 = ctx.getInstDefOperand(inst4);

  auto op17 = (op4);
  /* select inst SRAIW */
  auto inst5 = ctx.insertMIRInst(SRAIW, {op6, op7, op17});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst5));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern48(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstSDiv;
  /** Match Inst **/
  /* match inst InstSDiv */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstSDiv(inst1, op1, op2, op3)) {
    return false;
  } /* lookup inst that define the operand3 */
  auto inst2 = ctx.lookupDefInst(op3);
  if (not inst2) {
    return false;
  }

  /* match inst InstShl */
  MIROperand op4;
  MIROperand op5;
  MIROperand op6;
  if (not matchInstShl(inst2, op4, op5, op6)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI32(op1) && isOperandIReg(op2) && isOne(op5))) {
    return false;
  }

  /** Select Inst **/
  auto op8 = (op1);
  auto op10 = (getVRegAs64(ctx, op1));
  auto op11 = (op2);
  auto op13 = (getVRegAs64(ctx, op1));
  auto op15 = (getVRegAs64(ctx, op1));
  auto op16 = (op2);
  auto op17 = (MIROperand::asImm(1, OperandType::Int32));
  /* select inst SLLI */
  auto inst3 = ctx.insertMIRInst(SLLI, {op15, op16, op17});

  auto op14 = ctx.getInstDefOperand(inst3);

  auto op19 = (getVRegAs(ctx, op1));
  auto op20 = (MIROperand::asImm(64, OperandType::Int32));
  auto op21 = (op6);
  /* select inst InstSub */
  auto inst4 = ctx.insertMIRInst(InstSub, {op19, op20, op21});

  auto op18 = ctx.getInstDefOperand(inst4);

  /* select inst SRL */
  auto inst5 = ctx.insertMIRInst(SRL, {op13, op14, op18});

  auto op12 = ctx.getInstDefOperand(inst5);

  /* select inst ADD */
  auto inst6 = ctx.insertMIRInst(ADD, {op10, op11, op12});

  auto op9 = ctx.getInstDefOperand(inst6);

  auto op22 = (op6);
  /* select inst SRAW */
  auto inst7 = ctx.insertMIRInst(SRAW, {op8, op9, op22});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst7));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern49(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstSDiv;
  /** Match Inst **/
  /* match inst InstSDiv */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstSDiv(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI64(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst DIV */
  auto inst2 = ctx.insertMIRInst(DIV, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern50(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstSDiv;
  /** Match Inst **/
  /* match inst InstSDiv */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstSDiv(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI32(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst DIVW */
  auto inst2 = ctx.insertMIRInst(DIVW, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstSDiv matchAndSelectPatternInstSDivend */

/* InstSRem matchAndSelectPatternInstSRem begin */
static bool matchAndSelectPattern51(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstSRem;
  /** Match Inst **/
  /* match inst InstSRem */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstSRem(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  MIROperand op4;
  MIROperand op5;
  MIROperand op6;
  if (not(isOperandI32(op1) && isOperandIReg(op2) && isOperandImm(op3) &&
          select_sdiv32_by_cconstant_divisor(op3, op4, op5, op6))) {
    return false;
  }

  /** Select Inst **/
  auto op8 = (op1);
  auto op9 = (op2);
  auto op11 = (getVRegAs(ctx, op1));
  auto op13 = (getVRegAs(ctx, op1));
  auto op14 = (op2);
  auto op15 = (op3);
  /* select inst InstSDiv */
  auto inst2 = ctx.insertMIRInst(InstSDiv, {op13, op14, op15});

  auto op12 = ctx.getInstDefOperand(inst2);

  auto op16 = (op3);
  /* select inst InstMul */
  auto inst3 = ctx.insertMIRInst(InstMul, {op11, op12, op16});

  auto op10 = ctx.getInstDefOperand(inst3);

  /* select inst SUBW */
  auto inst4 = ctx.insertMIRInst(SUBW, {op8, op9, op10});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst4));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern52(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstSRem;
  /** Match Inst **/
  /* match inst InstSRem */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstSRem(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI32(op1) && isOperandIReg(op2) && !isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op8 = (getVRegAs(ctx, op1));
  auto op9 = (op3);
  /* select inst InstLoadImm */
  auto inst2 = ctx.insertMIRInst(InstLoadImm, {op8, op9});

  auto op7 = ctx.getInstDefOperand(inst2);

  /* select inst InstSRem */
  auto inst3 = ctx.insertMIRInst(InstSRem, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst3));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern53(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstSRem;
  /** Match Inst **/
  /* match inst InstSRem */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstSRem(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI64(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst REM */
  auto inst2 = ctx.insertMIRInst(REM, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern54(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstSRem;
  /** Match Inst **/
  /* match inst InstSRem */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstSRem(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandI32(op1) && isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst REMW */
  auto inst2 = ctx.insertMIRInst(REMW, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstSRem matchAndSelectPatternInstSRemend */

/* InstSMin matchAndSelectPatternInstSMin begin */
static bool matchAndSelectPattern55(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstSMin;
  /** Match Inst **/
  /* match inst InstSMin */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstSMin(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst MIN */
  auto inst2 = ctx.insertMIRInst(MIN, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstSMin matchAndSelectPatternInstSMinend */

/* InstSMax matchAndSelectPatternInstSMax begin */
static bool matchAndSelectPattern56(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstSMax;
  /** Match Inst **/
  /* match inst InstSMax */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstSMax(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2) && isOperandIReg(op3))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst MAX */
  auto inst2 = ctx.insertMIRInst(MAX, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstSMax matchAndSelectPatternInstSMaxend */

/* InstAbs matchAndSelectPatternInstAbs begin */
static bool matchAndSelectPattern57(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstAbs;
  /** Match Inst **/
  /* match inst InstAbs */
  MIROperand op1;
  MIROperand op2;
  if (not matchInstAbs(inst1, op1, op2)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2) && isOperandI32(op1))) {
    return false;
  }

  /** Select Inst **/
  auto op4 = (op1);
  auto op6 = (getVRegAs(ctx, op1));
  auto op7 = (getZero(op1));
  auto op8 = (op2);
  /* select inst SUBW */
  auto inst2 = ctx.insertMIRInst(SUBW, {op6, op7, op8});

  auto op5 = ctx.getInstDefOperand(inst2);

  auto op9 = (op2);
  /* select inst MAX */
  auto inst3 = ctx.insertMIRInst(MAX, {op4, op5, op9});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst3));
  ctx.remove_inst(inst1);
  return true;
}

/* InstAbs matchAndSelectPatternInstAbsend */

/* InstICmp matchAndSelectPatternInstICmp begin */
static bool matchAndSelectPattern58(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstICmp;
  /** Match Inst **/
  /* match inst InstICmp */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  MIROperand op4;
  if (not matchInstICmp(inst1, op1, op2, op3, op4)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2) && isOperandIReg(op3) &&
          isCompareOp(op4, CompareOp::ICmpSignedLessThan))) {
    return false;
  }

  /** Select Inst **/
  auto op6 = (op1);
  auto op7 = (op2);
  auto op8 = (op3);
  /* select inst SLT */
  auto inst2 = ctx.insertMIRInst(SLT, {op6, op7, op8});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern59(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstICmp;
  /** Match Inst **/
  /* match inst InstICmp */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  MIROperand op4;
  if (not matchInstICmp(inst1, op1, op2, op3, op4)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2) && isOperandIReg(op3) &&
          isCompareOp(op4, CompareOp::ICmpSignedGreaterEqual))) {
    return false;
  }

  /** Select Inst **/
  auto op6 = (op1);
  auto op8 = (getVRegAs(ctx, op1));
  auto op9 = (op2);
  auto op10 = (op3);
  /* select inst SLT */
  auto inst2 = ctx.insertMIRInst(SLT, {op8, op9, op10});

  auto op7 = ctx.getInstDefOperand(inst2);

  auto op11 = (getOne(op3));
  /* select inst XORI */
  auto inst3 = ctx.insertMIRInst(XORI, {op6, op7, op11});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst3));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern60(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstICmp;
  /** Match Inst **/
  /* match inst InstICmp */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  MIROperand op4;
  if (not matchInstICmp(inst1, op1, op2, op3, op4)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2) && isOperandIReg(op3) &&
          isCompareOp(op4, CompareOp::ICmpUnsignedLessThan))) {
    return false;
  }

  /** Select Inst **/
  auto op6 = (op1);
  auto op7 = (op2);
  auto op8 = (op3);
  /* select inst SLTU */
  auto inst2 = ctx.insertMIRInst(SLTU, {op6, op7, op8});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern61(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstICmp;
  /** Match Inst **/
  /* match inst InstICmp */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  MIROperand op4;
  if (not matchInstICmp(inst1, op1, op2, op3, op4)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2) && isOperandIReg(op3) &&
          isCompareOp(op4, CompareOp::ICmpSignedGreaterThan))) {
    return false;
  }

  /** Select Inst **/
  auto op6 = (op1);
  auto op7 = (op3);
  auto op8 = (op2);
  /* select inst SLT */
  auto inst2 = ctx.insertMIRInst(SLT, {op6, op7, op8});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern62(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstICmp;
  /** Match Inst **/
  /* match inst InstICmp */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  MIROperand op4;
  if (not matchInstICmp(inst1, op1, op2, op3, op4)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2) && isOperandIReg(op3) &&
          isCompareOp(op4, CompareOp::ICmpUnsignedGreaterThan))) {
    return false;
  }

  /** Select Inst **/
  auto op6 = (op1);
  auto op7 = (op3);
  auto op8 = (op2);
  /* select inst SLTU */
  auto inst2 = ctx.insertMIRInst(SLTU, {op6, op7, op8});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern63(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstICmp;
  /** Match Inst **/
  /* match inst InstICmp */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  MIROperand op4;
  if (not matchInstICmp(inst1, op1, op2, op3, op4)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2) && isZero(op3) && isCompareOp(op4, CompareOp::ICmpEqual))) {
    return false;
  }

  /** Select Inst **/
  auto op6 = (op1);
  auto op7 = (op2);
  auto op8 = (getOne(op3));
  /* select inst SLTIU */
  auto inst2 = ctx.insertMIRInst(SLTIU, {op6, op7, op8});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern64(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstICmp;
  /** Match Inst **/
  /* match inst InstICmp */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  MIROperand op4;
  if (not matchInstICmp(inst1, op1, op2, op3, op4)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2) && isZero(op3) && isCompareOp(op4, CompareOp::ICmpNotEqual))) {
    return false;
  }

  /** Select Inst **/
  auto op6 = (op1);
  auto op7 = (getZero(op3));
  auto op8 = (op2);
  /* select inst SLTU */
  auto inst2 = ctx.insertMIRInst(SLTU, {op6, op7, op8});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern65(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstICmp;
  /** Match Inst **/
  /* match inst InstICmp */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  MIROperand op4;
  if (not matchInstICmp(inst1, op1, op2, op3, op4)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2) && isOperandIReg(op3) &&
          isCompareOp(op4, CompareOp::ICmpSignedLessEqual))) {
    return false;
  }

  /** Select Inst **/
  auto op6 = (op1);
  auto op8 = (getVRegAs(ctx, op1));
  auto op9 = (op3);
  auto op10 = (op2);
  /* select inst SLT */
  auto inst2 = ctx.insertMIRInst(SLT, {op8, op9, op10});

  auto op7 = ctx.getInstDefOperand(inst2);

  auto op11 = (getOne(op3));
  /* select inst XORI */
  auto inst3 = ctx.insertMIRInst(XORI, {op6, op7, op11});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst3));
  ctx.remove_inst(inst1);
  return true;
}

/* InstICmp matchAndSelectPatternInstICmpend */

/* InstBranch matchAndSelectPatternInstBranch begin */
static bool matchAndSelectPattern66(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstBranch;
  /** Match Inst **/
  /* match inst InstBranch */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstBranch(inst1, op1, op2, op3)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op1))) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (getZero(op1));
  auto op7 = (op2);
  auto op8 = (op3);
  /* select inst BNE */
  auto inst2 = ctx.insertMIRInst(BNE, {op5, op6, op7, op8});

  ctx.remove_inst(inst1);
  return true;
}

/* InstBranch matchAndSelectPatternInstBranchend */

/* InstF2S matchAndSelectPatternInstF2S begin */
static bool matchAndSelectPattern67(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstF2S;
  /** Match Inst **/
  /* match inst InstF2S */
  MIROperand op1;
  MIROperand op2;
  if (not matchInstF2S(inst1, op1, op2)) {
    return false;
  }

  /** Select Inst **/
  auto op4 = (op1);
  auto op5 = (op2);
  /* select inst FCVT_W_S */
  auto inst2 = ctx.insertMIRInst(FCVT_W_S, {op4, op5});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstF2S matchAndSelectPatternInstF2Send */

/* InstS2F matchAndSelectPatternInstS2F begin */
static bool matchAndSelectPattern68(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstS2F;
  /** Match Inst **/
  /* match inst InstS2F */
  MIROperand op1;
  MIROperand op2;
  if (not matchInstS2F(inst1, op1, op2)) {
    return false;
  }

  /** Select Inst **/
  auto op4 = (op1);
  auto op5 = (op2);
  /* select inst FCVT_S_W */
  auto inst2 = ctx.insertMIRInst(FCVT_S_W, {op4, op5});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstS2F matchAndSelectPatternInstS2Fend */

/* InstFCmp matchAndSelectPatternInstFCmp begin */
static bool matchAndSelectPattern69(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstFCmp;
  /** Match Inst **/
  /* match inst InstFCmp */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  MIROperand op4;
  if (not matchInstFCmp(inst1, op1, op2, op3, op4)) {
    return false;
  }

  /* match predicate for operands  */
  MIROperand op5;
  MIROperand op6;
  MIROperand op7;
  if (not(selectFCmpOpcode(op4, op2, op3, op5, op6, op7))) {
    return false;
  }

  /** Select Inst **/
  auto op9 = (op1);
  auto op10 = (op5);
  auto op11 = (op6);
  /* select inst static_cast<uint32_t>((op7).imm()) */
  auto inst2 = ctx.insertMIRInst(static_cast<uint32_t>((op7).imm()), {op9, op10, op11});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstFCmp matchAndSelectPatternInstFCmpend */

/* InstFNeg matchAndSelectPatternInstFNeg begin */
static bool matchAndSelectPattern70(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstFNeg;
  /** Match Inst **/
  /* match inst InstFNeg */
  MIROperand op1;
  MIROperand op2;
  if (not matchInstFNeg(inst1, op1, op2)) {
    return false;
  }

  /** Select Inst **/
  auto op4 = (op1);
  auto op5 = (op2);
  /* select inst FNEG_S */
  auto inst2 = ctx.insertMIRInst(FNEG_S, {op4, op5});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstFNeg matchAndSelectPatternInstFNegend */

/* InstFAdd matchAndSelectPatternInstFAdd begin */
static bool matchAndSelectPattern71(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstFAdd;
  /** Match Inst **/
  /* match inst InstFAdd */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstFAdd(inst1, op1, op2, op3)) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst FADD_S */
  auto inst2 = ctx.insertMIRInst(FADD_S, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern75(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstFAdd;
  /** Match Inst **/
  /* match inst InstFAdd */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstFAdd(inst1, op1, op2, op3)) {
    return false;
  } /* lookup inst that define the operand2 */
  auto inst2 = ctx.lookupDefInst(op2);
  if (not inst2) {
    return false;
  }

  /* match inst InstFMul */
  MIROperand op4;
  MIROperand op5;
  MIROperand op6;
  if (not matchInstFMul(inst2, op4, op5, op6)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(ctx.hasOneUse(op4))) {
    return false;
  }

  /** Select Inst **/
  auto op8 = (op1);
  auto op9 = (op5);
  auto op10 = (op6);
  auto op11 = (op3);
  /* select inst FMADD_S */
  auto inst3 = ctx.insertMIRInst(FMADD_S, {op8, op9, op10, op11});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst3));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern77(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstFAdd;
  /** Match Inst **/
  /* match inst InstFAdd */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstFAdd(inst1, op1, op2, op3)) {
    return false;
  } /* lookup inst that define the operand2 */
  auto inst2 = ctx.lookupDefInst(op2);
  if (not inst2) {
    return false;
  }

  /* match inst InstFNeg */
  MIROperand op4;
  MIROperand op5;
  if (not matchInstFNeg(inst2, op4, op5)) {
    return false;
  } /* lookup inst that define the operand5 */
  auto inst3 = ctx.lookupDefInst(op5);
  if (not inst3) {
    return false;
  }

  /* match inst InstFMul */
  MIROperand op6;
  MIROperand op7;
  MIROperand op8;
  if (not matchInstFMul(inst3, op6, op7, op8)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(ctx.hasOneUse(op4) && ctx.hasOneUse(op6))) {
    return false;
  }

  /** Select Inst **/
  auto op10 = (op1);
  auto op11 = (op7);
  auto op12 = (op8);
  auto op13 = (op3);
  /* select inst FNMSUB_S */
  auto inst4 = ctx.insertMIRInst(FNMSUB_S, {op10, op11, op12, op13});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst4));
  ctx.remove_inst(inst1);
  return true;
}

/* InstFAdd matchAndSelectPatternInstFAddend */

/* InstFSub matchAndSelectPatternInstFSub begin */
static bool matchAndSelectPattern72(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstFSub;
  /** Match Inst **/
  /* match inst InstFSub */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstFSub(inst1, op1, op2, op3)) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst FSUB_S */
  auto inst2 = ctx.insertMIRInst(FSUB_S, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern76(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstFSub;
  /** Match Inst **/
  /* match inst InstFSub */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstFSub(inst1, op1, op2, op3)) {
    return false;
  } /* lookup inst that define the operand2 */
  auto inst2 = ctx.lookupDefInst(op2);
  if (not inst2) {
    return false;
  }

  /* match inst InstFMul */
  MIROperand op4;
  MIROperand op5;
  MIROperand op6;
  if (not matchInstFMul(inst2, op4, op5, op6)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(ctx.hasOneUse(op4))) {
    return false;
  }

  /** Select Inst **/
  auto op8 = (op1);
  auto op9 = (op5);
  auto op10 = (op6);
  auto op11 = (op3);
  /* select inst FMSUB_S */
  auto inst3 = ctx.insertMIRInst(FMSUB_S, {op8, op9, op10, op11});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst3));
  ctx.remove_inst(inst1);
  return true;
}
static bool matchAndSelectPattern78(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstFSub;
  /** Match Inst **/
  /* match inst InstFSub */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstFSub(inst1, op1, op2, op3)) {
    return false;
  } /* lookup inst that define the operand2 */
  auto inst2 = ctx.lookupDefInst(op2);
  if (not inst2) {
    return false;
  }

  /* match inst InstFNeg */
  MIROperand op4;
  MIROperand op5;
  if (not matchInstFNeg(inst2, op4, op5)) {
    return false;
  } /* lookup inst that define the operand5 */
  auto inst3 = ctx.lookupDefInst(op5);
  if (not inst3) {
    return false;
  }

  /* match inst InstFMul */
  MIROperand op6;
  MIROperand op7;
  MIROperand op8;
  if (not matchInstFMul(inst3, op6, op7, op8)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(ctx.hasOneUse(op4) && ctx.hasOneUse(op6))) {
    return false;
  }

  /** Select Inst **/
  auto op10 = (op1);
  auto op11 = (op7);
  auto op12 = (op8);
  auto op13 = (op3);
  /* select inst FNMADD_S */
  auto inst4 = ctx.insertMIRInst(FNMADD_S, {op10, op11, op12, op13});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst4));
  ctx.remove_inst(inst1);
  return true;
}

/* InstFSub matchAndSelectPatternInstFSubend */

/* InstFMul matchAndSelectPatternInstFMul begin */
static bool matchAndSelectPattern73(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstFMul;
  /** Match Inst **/
  /* match inst InstFMul */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstFMul(inst1, op1, op2, op3)) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst FMUL_S */
  auto inst2 = ctx.insertMIRInst(FMUL_S, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstFMul matchAndSelectPatternInstFMulend */

/* InstFDiv matchAndSelectPatternInstFDiv begin */
static bool matchAndSelectPattern74(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstFDiv;
  /** Match Inst **/
  /* match inst InstFDiv */
  MIROperand op1;
  MIROperand op2;
  MIROperand op3;
  if (not matchInstFDiv(inst1, op1, op2, op3)) {
    return false;
  }

  /** Select Inst **/
  auto op5 = (op1);
  auto op6 = (op2);
  auto op7 = (op3);
  /* select inst FDIV_S */
  auto inst2 = ctx.insertMIRInst(FDIV_S, {op5, op6, op7});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstFDiv matchAndSelectPatternInstFDivend */

/* InstZExt matchAndSelectPatternInstZExt begin */
static bool matchAndSelectPattern79(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstZExt;
  /** Match Inst **/
  /* match inst InstZExt */
  MIROperand op1;
  MIROperand op2;
  if (not matchInstZExt(inst1, op1, op2)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandBoolReg(op2))) {
    return false;
  }

  /** Select Inst **/
  auto op4 = (op1);
  auto op5 = (op2);
  /* select inst InstCopy */
  auto inst2 = ctx.insertMIRInst(InstCopy, {op4, op5});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstZExt matchAndSelectPatternInstZExtend */

/* InstSExt matchAndSelectPatternInstSExt begin */
static bool matchAndSelectPattern80(MIRInst* inst1, ISelContext& ctx) {
  uint32_t rootOpcode = InstSExt;
  /** Match Inst **/
  /* match inst InstSExt */
  MIROperand op1;
  MIROperand op2;
  if (not matchInstSExt(inst1, op1, op2)) {
    return false;
  }

  /* match predicate for operands  */
  if (not(isOperandIReg(op2))) {
    return false;
  }

  /** Select Inst **/
  auto op4 = (op1);
  auto op5 = (op2);
  /* select inst InstCopy */
  auto inst2 = ctx.insertMIRInst(InstCopy, {op4, op5});

  /* Replace Operand */
  ctx.replace_operand(ctx.getInstDefOperand(inst1), ctx.getInstDefOperand(inst2));
  ctx.remove_inst(inst1);
  return true;
}

/* InstSExt matchAndSelectPatternInstSExtend */

static bool matchAndSelectImpl(MIRInst* inst, ISelContext& ctx, bool debugMatchSelect) {
  bool success = false;
  switch (inst->opcode()) {
    case InstLoadGlobalAddress: {
      if (matchAndSelectPattern1(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern2(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstLoad: {
      if (matchAndSelectPattern3(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstStore: {
      if (matchAndSelectPattern4(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstJump: {
      if (matchAndSelectPattern5(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstLoadImm: {
      if (matchAndSelectPattern6(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern8(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern10(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstLoadImmToReg: {
      if (matchAndSelectPattern7(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern9(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern11(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstAdd: {
      if (matchAndSelectPattern12(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern13(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern14(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern15(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern16(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern17(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern18(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern19(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern20(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern21(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstSub: {
      if (matchAndSelectPattern22(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern23(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstMul: {
      if (matchAndSelectPattern24(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern25(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstUDiv: {
      if (matchAndSelectPattern26(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern27(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstURem: {
      if (matchAndSelectPattern28(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern29(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstAnd: {
      if (matchAndSelectPattern30(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern33(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstOr: {
      if (matchAndSelectPattern31(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern34(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstXor: {
      if (matchAndSelectPattern32(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern35(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstShl: {
      if (matchAndSelectPattern36(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern37(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern38(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern39(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstLShr: {
      if (matchAndSelectPattern40(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern41(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstAShr: {
      if (matchAndSelectPattern42(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern43(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern44(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern45(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstSDiv: {
      if (matchAndSelectPattern46(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern47(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern48(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern49(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern50(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstSRem: {
      if (matchAndSelectPattern51(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern52(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern53(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern54(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstSMin: {
      if (matchAndSelectPattern55(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstSMax: {
      if (matchAndSelectPattern56(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstAbs: {
      if (matchAndSelectPattern57(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstICmp: {
      if (matchAndSelectPattern58(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern59(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern60(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern61(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern62(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern63(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern64(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern65(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstBranch: {
      if (matchAndSelectPattern66(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstF2S: {
      if (matchAndSelectPattern67(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstS2F: {
      if (matchAndSelectPattern68(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstFCmp: {
      if (matchAndSelectPattern69(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstFNeg: {
      if (matchAndSelectPattern70(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstFAdd: {
      if (matchAndSelectPattern71(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern75(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern77(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstFSub: {
      if (matchAndSelectPattern72(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern76(inst, ctx)) {
        success = true;
        break;
      }
      if (matchAndSelectPattern78(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstFMul: {
      if (matchAndSelectPattern73(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstFDiv: {
      if (matchAndSelectPattern74(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstZExt: {
      if (matchAndSelectPattern79(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    case InstSExt: {
      if (matchAndSelectPattern80(inst, ctx)) {
        success = true;
        break;
      }
      break;
    }
    default:
      break;
  }
  if (debugMatchSelect) {
    auto& instInfo = ctx.codegen_ctx().instInfo.getInstInfo(inst);
    std::cerr << instInfo.name();
    if (success)
      std::cerr << " success." << std::endl;
    else
      std::cerr << " failed." << std::endl;
  }
  return success;
}

class RISCVISelInfo final : public TargetISelInfo {
public:
  bool isLegalInst(uint32_t opcode) const override;
  bool match_select(MIRInst* inst, ISelContext& ctx) const override;
  void legalizeInstWithStackOperand(const InstLegalizeContext& ctx,
                                    MIROperand op,
                                    StackObject& obj) const override;
  void postLegalizeInst(const InstLegalizeContext& ctx) const override;
  bool lowerInst(ir::Instruction* inst, LoweringContext& loweringCtx) const override;
  MIROperand materializeFPConstant(float fpVal, LoweringContext& loweringCtx) const override;
};

TargetISelInfo& getRISCVISelInfo() {
  static RISCVISelInfo iselInfo;
  return iselInfo;
}

RISCV_NAMESPACE_END