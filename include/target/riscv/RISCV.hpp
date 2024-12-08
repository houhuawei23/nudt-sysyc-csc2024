#pragma once
#include "mir/MIR.hpp"
#include "mir/instinfo.hpp"
#include "autogen/riscv/InstInfoDecl.hpp"
#include "support/arena.hpp"

namespace mir::RISCV {
/*
 * @brief: RISCVRegister enum
 * @note:
 *      Risc-V架构 64位
 * @param:
 *      1. GRBegin - General Register
 *      2. FRBegin - Float Register
 */
//! do not delete following line
// clang-format off
enum RISCVRegister : uint32_t {
    GPRBegin,
    X0=GPRBegin, X1, X2, X3, X4, X5, X6, X7,
    X8, X9, X10, X11, X12, X13, X14, X15,
    X16, X17, X18, X19, X20, X21, X22, X23,
    X24, X25, X26, X27, X28, X29, X30, X31,
    GPREnd,
    FPRBegin,
    F0=FPRBegin, F1, F2, F3, F4, F5, F6, F7,
    F8, F9, F10, F11, F12, F13, F14, F15,
    F16, F17, F18, F19, F20, F21, F22, F23,
    F24, F25, F26, F27, F28, F29, F30, F31,
    FPREnd,
};
// clang-format on
/* 保存返回地址 */
static auto ra = MIROperand::asISAReg(RISCVRegister::X1, OperandType::Int64);

// stack pointer
static auto sp = MIROperand::asISAReg(RISCVRegister::X2, OperandType::Int64);

// utils function
constexpr bool isOperandGR(const MIROperand& operand) {
  if (not operand.isReg() || not isIntType(operand.type())) return false;
  auto reg = operand.reg();
  return GPRBegin <= reg && reg < GPREnd;
}

constexpr bool isOperandFPR(const MIROperand& operand) {
  if (not operand.isReg() || not isFloatType(operand.type())) return false;
  if (isVirtualReg(operand.reg())) {
    return true;
  }
  auto reg = operand.reg();
  return FPRBegin <= reg && reg < FPREnd;
}

/*
 * @note: 相关寄存器功能:
 *      1. a1 -- return value
 */
static std::string_view getRISCVGPRTextualName(uint32_t idx) noexcept {
  constexpr std::string_view name[] = {
    // clang-format off
    "zero", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2",
    "s0",   "s1", "a0",  "a1",  "a2", "a3", "a4", "a5",
    "a6",   "a7", "s2",  "s3",  "s4", "s5", "s6", "s7",
    "s8",   "s9", "s10", "s11", "t3", "t4", "t5", "t6",
    // clang-format on
  };
  return name[idx];
}

struct OperandDumper {
  MIROperand operand;
};

static std::ostream& operator<<(std::ostream& os, OperandDumper opdp) {
  auto operand = opdp.operand;
  if (operand.isReg()) {
    if (isVirtualReg(operand.reg())) {
      dumpVirtualReg(os, operand);
    } else if (isStackObject(operand.reg())) {
      os << "so" << (operand.reg() ^ stackObjectBegin);
    } else if (isOperandGR(operand)) {
      os << getRISCVGPRTextualName(operand.reg());
    } else if (isOperandFPR(operand)) {
      os << "f" << (operand.reg() - FPRBegin);
    } else {
      os << "[reg]";
    }
  } else if (operand.isImm()) {
    os << operand.imm();
  } else if (operand.isProb()) {
    os << " prob " << operand.prob();
  } else if (operand.isReloc()) {
    if (operand.type() == OperandType::HighBits) {
      os << "%pcrel_hi(";
    } else if (operand.type() == OperandType::LowBits) {
      os << "%pcrel_lo(";
    }
    os << operand.reloc()->name();
    if (operand.type() != OperandType::Special) {
      os << ")";
    }
  } else {
    os << "unknown ";
  }
  return os;
}

constexpr bool isOperandImm12(const MIROperand& operand) {
  if (operand.isReloc() && operand.type() == OperandType::LowBits) return true;
  return operand.isImm() && isSignedImm<12>(operand.imm());
}
constexpr bool isOperandImm32(const MIROperand& operand) {
  return operand.isImm() && isSignedImm<32>(operand.imm());
}

constexpr bool isOperandNonZeroImm12(const MIROperand& operand) {
  return isOperandImm12(operand) && operand.imm() != 0;
}

constexpr bool isOperandNonZeroImm32(const MIROperand& operand) {
  return isOperandImm32(operand) && operand.imm() != 0;
}

constexpr bool isOperandUImm5(const MIROperand& operand) {
    return operand.isImm() && isUnsignedImm<5>(operand.imm());
}

constexpr bool isOperandUImm6(const MIROperand& operand) {
    return operand.isImm() && isUnsignedImm<6>(operand.imm());
}

constexpr bool isOperandUImm20(const MIROperand& operand) {
    if(operand.isReloc() && operand.type() == OperandType::HighBits)
        return true;
    return operand.isImm() && isUnsignedImm<20>(operand.imm());
}

static auto scratch = MIROperand::asISAReg(RISCV::X5, OperandType::Int64);

static void legalizeAddrBaseOffsetPostRA(MIRInstList& instructions,
                                         MIRInstList::iterator iter,
                                         MIROperand& base,
                                         int64_t& imm) {
  if (-2048 <= imm and imm <= 2047) {
    return;
  }
  /**
   * imm(base)
   * ->
   * LoadImm32 scratch, imm
   * ADD scratch, base, scratch
   * &imm = 0
   * &base = scratch
   */
  // auto inst = *iter;
  auto loadInst = utils::make<MIRInst>(
    LoadImm32, {scratch, MIROperand::asImm(imm, OperandType::Int32)});
  instructions.insert(iter, loadInst);

  auto addInst = utils::make<MIRInst>(RISCVInst::ADD, {scratch, base, scratch});
  instructions.insert(iter, addInst);

  imm = 0;
  base = scratch;
  return;
}

static void legalizeAddrBaseOffsetPostRA(MIRInstList& instructions,
                                         MIRInstList::iterator iter,
                                         MIROperand& dst,
                                         MIROperand& base,
                                         int64_t& imm) {
  if (-2048 <= imm and imm <= 2047) {
    return;
  }
  /**
   * origin:
   * dst = base + imm
   * after adjust:
   * dst = adjust, new_imm = imm - adjust
   * dst = base + new_imm
   */
  if (-4096 <= imm and imm <= 4094) {
    const auto adjust = imm < 0 ? -2048 : 2047;
    auto newInst = utils::make<MIRInst>(
      RISCVInst::LoadImm32,
      {dst, MIROperand::asImm(adjust, OperandType::Int32)});
    instructions.insert(iter, newInst);
    imm -= adjust;
    return;
  } 
  std::cerr << "adjust_reg: imm out of range: " << imm << std::endl;
  assert(false);
}

// dst = src + imm
static void adjust_reg(MIRInstList& instructions,
                       MIRInstList::iterator it,
                       MIROperand dst,
                       MIROperand src,
                       int64_t imm) {
  if (dst == src && imm == 0) return;
  MIROperand base = src;
  legalizeAddrBaseOffsetPostRA(instructions, it, base, imm);
  // addi dst, base, imm
  auto inst = utils::make<MIRInst>(
    RISCVInst::ADDI, {dst, base, MIROperand::asImm(imm, OperandType::Int64)});
  instructions.insert(it, inst);
}

// static void adjust
}  // namespace mir::RISCV
