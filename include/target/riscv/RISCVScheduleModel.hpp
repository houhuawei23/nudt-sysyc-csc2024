#pragma once
#include "autogen/riscv/InstInfoDecl.hpp"
#include "mir/MIR.hpp"
#include "mir/instinfo.hpp"
#include "mir/ScheduleModel.hpp"
RISCV_NAMESPACE_BEGIN

enum RISCVPipeline : uint32_t { RISCVIDivPipeline, RISCVFPDivPipeline };
enum RISCVIssueMask : uint32_t {
  RISCVPipelineA = 1 << 0,                           // 0b01
  RISCVPipelineB = 1 << 1,                           // 0b10
  RISCVPipelineAB = RISCVPipelineA | RISCVPipelineB  // 0b11
};

// LLVM: RISCVSchedSiFive7.td
// The SiFive7 microarchitecture has three pipelines: A, B, V.
// Pipe A can handle memory, integer alu and vector operations.
// Pipe B can handle integer alu, control flow, integer multiply and divide,
// and floating point computation.
// The V pipeline is modeled by the VCQ, VA, VL, and VS resources.

/*
sifive-u74 S7/U7 Core:
dual-issue, in-order pipeline, 8 stages:
F1/F2 - D1/D2 - AG - M1/M2 - WB

 */

template <uint32_t ValidPipeline, bool Early, bool Late>
class RISCVScheduleClassIntegerArithmeticGeneric final : public ScheduleClass {
  static_assert(ValidPipeline != 0 && (Early || Late));

public:
  bool schedule(ScheduleState& state,
                const MIRInst& inst,
                const InstInfo& instInfo) const override {
    if (not state.isAvailable(ValidPipeline)) return false;
    // Address Genration Unit (AG)
    bool availableInAG = true;

    /*
    if operand RISCVTargetlatency is 0:
       availableInAG = availableInAG & true;
    else if operand latency is 1:
       availableInAG = false;
    else latency > 2:
       return false;

    if all operands' laytency of inst is 0, then the dst of inst will avaliable in AG.
    manual:
    Integer arithmetic and branch instructions can execute in either the AG or M2 pipeline stage.
    If such an instruction’s operands are available when the instruction enters the AG stage,
    then it executes in AG; otherwise, it executes in M2.

    in this cycle, the dst of inst will avalible in which stage?
    if cant avalibel in any stages, schedule false.
    */
    // for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
    //   if (instInfo.operand_flag(idx) & OperandFlagUse) {
    //     const auto latency = state.queryRegisterLatency(inst, idx);
    //     switch (latency) {
    //       case 0:
    //         continue;
    //       case 1:
    //       case 2:
    //         availableInAG = false;
    //         break;
    //       default:  // >2
    //         return false;
    //         break;
    //     }
    //   }
    // }
    for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
      if (instInfo.operand_flag(idx) & OperandFlagUse) {
        const auto latency = state.queryRegisterLatency(inst, idx);
        if (latency <= 2) {
          availableInAG &= (latency == 0);
        } else
          return false;
      }
    }
    // all operands laytency is 0 or 1, issue instruction

    if constexpr (Early) {
      if (availableInAG) {
        // all operands are ready (latency == 0)
        if constexpr (ValidPipeline == RISCVPipelineAB) {
          auto availablePipeline =
            state.isAvailable(RISCVPipelineA) ? RISCVPipelineA : RISCVPipelineB;
          state.setIssued(availablePipeline);
        } else {
          state.setIssued(ValidPipeline);
        }
        // dst operand reg will be ready after next cycle
        state.makeRegisterReady(inst, 0, 1);
        return true;
      }
    }
    if constexpr (Late) {
      if constexpr (ValidPipeline == RISCVPipelineAB) {
        auto availablePipeline =
          state.isAvailable(RISCVPipelineA) ? RISCVPipelineA : RISCVPipelineB;
        state.setIssued(availablePipeline);
      } else {
        state.setIssued(ValidPipeline);
      }
      // dst operand reg will be ready after 3 cycles
      state.makeRegisterReady(inst, 0, 3);
      return true;
    }

    return false;
  }
};

/* Normal Integer Arithmetic, can issued in A/B, early and late scheduling */
using RISCVScheduleClassIntegerArithmetic =
  RISCVScheduleClassIntegerArithmeticGeneric<RISCVPipelineAB, true, true>;

/* LateB Integer Arithmetic, can issued in B, late scheduling */
using RISCVScheduleClassIntegerArithmeticLateB =
  RISCVScheduleClassIntegerArithmeticGeneric<RISCVPipelineB, false, true>;

using RISCVScheduleClassIntegerArithmeticEarlyB =
  RISCVScheduleClassIntegerArithmeticGeneric<RISCVPipelineB, true, false>;

using RISCVScheduleClassIntegerArithmeticLateAB =
  RISCVScheduleClassIntegerArithmeticGeneric<RISCVPipelineAB, false, true>;

using RISCVScheduleClassIntegerArithmeticEarlyLateB =
  RISCVScheduleClassIntegerArithmeticGeneric<RISCVPipelineB, true, true>;

class RISCVScheduleClassSlowLoadImm final : public ScheduleClass {
public:
  bool schedule(ScheduleState& state,
                const MIRInst& inst,
                const InstInfo& instInfo) const override {
    const auto& imm = inst.operand(1);
    if (isOperandImm12(imm)) {
      state.makeRegisterReady(inst, 0, 1);
    } else {
      // LUI + ADDI
      state.makeRegisterReady(inst, 0, 3);
    }
    return true;
  }
};

class RISCVScheduleClassBranch final : public ScheduleClass {
public:
  bool schedule(ScheduleState& state,
                const MIRInst& inst,
                const InstInfo& instInfo) const override {
    if (not state.isAvailable(RISCVPipelineB)) return false;
    for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx) {
      if (instInfo.operand_flag(idx) & OperandFlagUse) {
        if (state.queryRegisterLatency(inst, idx) > 2) return false;
      }
    }

    state.setIssued(RISCVPipelineB);
    return true;
  }
};
class RISCVScheduleClassLoadStore final : public ScheduleClass {
public:
  bool schedule(ScheduleState& state,
                const MIRInst& inst,
                const InstInfo& instInfo) const override {
    if (not state.isAvailable(RISCVPipelineA)) return false;
    // require effective address ready in AG stage
    for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx) {
      if (instInfo.operand_flag(idx) & OperandFlagUse) {
        if (state.queryRegisterLatency(inst, idx) > 0) return false;
      }
    }
    /* def operand reg will be ready after 3 cycles */
    if (instInfo.operand_flag(0) & OperandFlagDef) {
      state.makeRegisterReady(inst, 0, 3);
    }
    state.setIssued(RISCVPipelineA);
    return true;
  }
};

class RISCVScheduleClassMulti final : public ScheduleClass {
public:
  bool schedule(ScheduleState& state,
                const MIRInst& inst,
                const InstInfo& instInfo) const override {
    if (not state.isAvailable(RISCVPipelineB)) return false;

    for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx) {
      if (instInfo.operand_flag(idx) & OperandFlagUse) {
        if (state.queryRegisterLatency(inst, idx) > 0) return false;
      }
    }
    /* def operand reg will be ready after 3 cycles */
    if (instInfo.operand_flag(0) & OperandFlagDef) {
      state.makeRegisterReady(inst, 0, 3);
    }
    state.setIssued(RISCVPipelineB);
    return true;
  }
};

class RISCVScheduleClassDivRem final : public ScheduleClass {
public:
  bool schedule(ScheduleState& state,
                const MIRInst& inst,
                const InstInfo& instInfo) const override {
    if (!state.isAvailable(RISCVPipelineB)) return false;
    if (!state.isPipelineReady(RISCVIDivPipeline)) return false;

    // consumes operands in the AG stage
    for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx) {
      if (instInfo.operand_flag(idx) & OperandFlagUse) {
        if (state.queryRegisterLatency(inst, idx) > 0) return false;
      }
    }

    state.resetPipeline(RISCVIDivPipeline, 65);
    state.makeRegisterReady(inst, 0, 68);
    state.setIssued(RISCVPipelineB);
    return true;
  }
};

static uint32_t estimateDivRemLatency(const MIROperand& logDividend,
                                      const MIROperand& logDivisor,
                                      const MIROperand& nonNegativeHint) {
  //   const auto imm = nonNegativeHint.imm();
  const auto imm = 0b1010;
  const auto signDividend = imm & 0b100;
  const auto signDivisor = imm & 0b010;
  const auto signRes = imm & 0b001;
  const auto mayHaveNegativeInput = !(signDividend & signDivisor);
  const auto mayHaveNagativeResult = !signRes;
  const auto sdivLatency =
    2U +
    static_cast<uint32_t>(std::max(4, static_cast<int32_t>(logDividend.imm() - logDivisor.imm()))) +
    (mayHaveNegativeInput ? 1 : 0) + (mayHaveNagativeResult ? 1 : 0);
  return sdivLatency;
}

class RISCVScheduleClassSDivRemW final : public ScheduleClass {
public:
  bool schedule(ScheduleState& state,
                const MIRInst& inst,
                const InstInfo& instInfo) const override {
    if (!state.isAvailable(RISCVPipelineB)) return false;
    if (!state.isPipelineReady(RISCVIDivPipeline)) return false;

    // consumes operands in the AG stage
    if (state.queryRegisterLatency(inst, 1) > 0) return false;
    if (state.queryRegisterLatency(inst, 2) > 0) return false;

    // TODO: estimate latency based on operands
    // const auto logDividend = inst.operand(3);
    // const auto logDivisor = inst.operand(4);
    // const auto hint = inst.operand(5);
    // const auto latency = estimateDivRemLatency(logDividend, logDivisor,
    // hint);
    const auto latency = 30;

    state.resetPipeline(RISCVIDivPipeline, latency - 3);
    state.makeRegisterReady(inst, 0, latency);
    state.setIssued(RISCVPipelineB);
    return true;
  }
};

template <uint32_t Latency>
class RISCVScheduleClassFP final : public ScheduleClass {
public:
  bool schedule(ScheduleState& state,
                const MIRInst& inst,
                const InstInfo& instInfo) const override {
    if (!state.isAvailable(RISCVPipelineB)) return false;

    // consumes operands in the AG stage
    for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx) {
      if (instInfo.operand_flag(idx) & OperandFlagUse) {
        if (state.queryRegisterLatency(inst, idx) > 0) return false;
      }
    }

    state.makeRegisterReady(inst, 0, Latency);
    state.setIssued(RISCVPipelineB);
    return true;
  }
};

using RISCVScheduleClassFPCycle1 = RISCVScheduleClassFP<1>;
using RISCVScheduleClassFPCycle2 = RISCVScheduleClassFP<2>;
using RISCVScheduleClassFPCycle4 = RISCVScheduleClassFP<4>;
using RISCVScheduleClassFPCycle5 = RISCVScheduleClassFP<5>;

class RISCVScheduleClassFPDiv final : public ScheduleClass {
public:
  bool schedule(ScheduleState& state,
                const MIRInst& inst,
                const InstInfo& instInfo) const override {
    if (!state.isAvailable(RISCVPipelineB)) return false;
    if (!state.isPipelineReady(RISCVFPDivPipeline)) return false;

    // consumes operands in the AG stage
    for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx) {
      if (instInfo.operand_flag(idx) & OperandFlagUse) {
        if (state.queryRegisterLatency(inst, idx) > 0) return false;
      }
    }

    state.resetPipeline(RISCVFPDivPipeline, 33);
    state.makeRegisterReady(inst, 0, 36);
    state.setIssued(RISCVPipelineB);
    return true;
  }
};
class RISCVScheduleClassFPLoadStore final : public ScheduleClass {
public:
  bool schedule(ScheduleState& state,
                const MIRInst& inst,
                const InstInfo& instInfo) const override {
    if (!state.isAvailable(RISCVPipelineA)) return false;
    // require effective addresses to be ready in the AG stage
    for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx) {
      if (instInfo.operand_flag(idx) & OperandFlagUse) {
        if (state.queryRegisterLatency(inst, idx) > 0) return false;
      }
    }

    if (instInfo.operand_flag(0) & OperandFlagDef) {
      // 2 cycles to use for FLW
      state.makeRegisterReady(inst, 0, 2);
      auto la = state.queryRegisterLatency(inst, 0);
      // std::cerr << "FLoad laytency: " << la << std::endl;
    }

    state.setIssued(RISCVPipelineA);
    return true;
  }
};

class RISCVScheduleClassGeneralLoad final : public ScheduleClass {
  RISCVScheduleClassLoadStore mLoad;
  RISCVScheduleClassFPLoadStore mFPLoad;

public:
  bool schedule(ScheduleState& state,
                const MIRInst& inst,
                const InstInfo& instInfo) const override {
    if (isOperandGR(inst.operand(0))) return mLoad.schedule(state, inst, instInfo);
    return mFPLoad.schedule(state, inst, instInfo);
  }
};

RISCV_NAMESPACE_END
