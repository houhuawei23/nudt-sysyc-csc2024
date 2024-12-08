
// Automatically generated file, do not edit!
#include "autogen/riscv/ScheduleModelDecl.hpp"
#include <iostream>

RISCV_NAMESPACE_BEGIN

class RISCVScheduleModel_sifive_u74 final : public TargetScheduleModel {
  RISCVScheduleClassIntegerArithmetic mScheduleClass_IntegerArithmetic;

  RISCVScheduleClassSlowLoadImm mScheduleClass_SlowLoadImm;

  RISCVScheduleClassIntegerArithmeticEarlyLateB mScheduleClass_IntegerArithmeticEarlyLateB;

  RISCVScheduleClassBranch mScheduleClass_Branch;

  RISCVScheduleClassLoadStore mScheduleClass_LoadStore;

  RISCVScheduleClassMulti mScheduleClass_Multi;

  RISCVScheduleClassDivRem mScheduleClass_DivRem;

  RISCVScheduleClassSDivRemW mScheduleClass_SDivRemW;

  RISCVScheduleClassFPCycle1 mScheduleClass_FPCycle1;

  RISCVScheduleClassFPCycle2 mScheduleClass_FPCycle2;

  RISCVScheduleClassFPCycle4 mScheduleClass_FPCycle4;

  RISCVScheduleClassFPCycle5 mScheduleClass_FPCycle5;

  RISCVScheduleClassFPDiv mScheduleClass_FPDiv;

  RISCVScheduleClassFPLoadStore mScheduleClass_FPLoadStore;

  RISCVScheduleClassGeneralLoad mScheduleClass_GeneralLoad;

public:
  ScheduleClass& getInstScheClass(uint32_t opcode) override {
    switch (opcode) {
      case ADDI:
      case SLTI:
      case SLTIU:
      case ANDI:
      case ORI:
      case XORI:
      case SLLI:
      case SRLI:
      case SRAI:
      case LUI:
      case AUIPC:
      case ADD:
      case SLT:
      case SLTU:
      case AND:
      case OR:
      case XOR:
      case SLL:
      case SRL:
      case SUB:
      case SRA:
      case ADDIW:
      case SLLIW:
      case SRLIW:
      case SRAIW:
      case ADDW:
      case SUBW:
      case SLLW:
      case SRLW:
      case SRAW:
      case ADD_UW:
      case SLLI_UW:
      case LoadImm12:
      case MV:
      case InstLoadStackObjectAddr:
      case InstCopy:
      case InstCopyFromReg:
      case InstCopyToReg:
      case LLA:
        return mScheduleClass_IntegerArithmetic;

      case InstLoadImm:
      case LoadImm32:
      case LoadImm64:
        return mScheduleClass_SlowLoadImm;

      case SH1ADD:
      case SH1ADD_UW:
      case SH2ADD:
      case SH2ADD_UW:
      case SH3ADD:
      case SH3ADD_UW:
        return mScheduleClass_IntegerArithmeticEarlyLateB;

      case JAL:
      case RET:
      case BEQ:
      case BNE:
      case BLT:
      case BLE:
      case BGT:
      case BGE:
      case BLTU:
      case BLEU:
      case BGTU:
      case BGEU:
      case J:
        return mScheduleClass_Branch;

      case LB:
      case LH:
      case LW:
      case LBU:
      case LHU:
      case SB:
      case SH:
      case SW:
      case LD:
      case SD:
      case InstStoreRegToStack:
      case LR_W:
      case SC_W:
      case AMOSWAP_W:
      case AMOADD_W:
      case AMOAND_W:
      case AMOOR_W:
      case AMOXOR_W:
        return mScheduleClass_LoadStore;

      case MUL:
      case MULH:
      case MULHSU:
      case MULHU:
      case MULW:
        return mScheduleClass_Multi;

      case DIV:
      case REM:
      case REMU:
        return mScheduleClass_DivRem;

      case DIVW:
      case REMW:
        return mScheduleClass_SDivRemW;

      case FMV_X_W:
        return mScheduleClass_FPCycle1;

      case FNEG_S:
      case FCVT_S_W:
      case FCVT_S_WU:
      case FMV_S:
      case FMV_W_X:
      case FMIN_S:
      case FMAX_S:
      case FSGNJ_S:
      case FABS_S:
        return mScheduleClass_FPCycle2;

      case FEQ_S:
      case FLT_S:
      case FLE_S:
      case FCVT_W_S:
      case FCVT_WU_S:
        return mScheduleClass_FPCycle4;

      case FADD_S:
      case FSUB_S:
      case FMUL_S:
      case FMADD_S:
      case FMSUB_S:
      case FNMADD_S:
      case FNMSUB_S:
        return mScheduleClass_FPCycle5;

      case FDIV_S:
        return mScheduleClass_FPDiv;

      case FLW:
      case FSW:
        return mScheduleClass_FPLoadStore;

      case InstLoadRegFromStack:
        return mScheduleClass_GeneralLoad;

      default:
        std::cerr << "getInstScheClass() failed: op: " << opcode << std::endl;
        assert(false && "Invalid opcode");
    }
  }
  MicroArchInfo& getMicroArchInfo() override;
  bool peepholeOpt(MIRFunction& func, CodeGenContext& context) override;
  bool isExpensiveInst(MIRInst* inst, CodeGenContext& context) override;
};

TargetScheduleModel& getRISCVScheduleModel() {
  static RISCVScheduleModel_sifive_u74 model_sifive_u74;
  return model_sifive_u74;
}

RISCV_NAMESPACE_END