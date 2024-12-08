#pragma once
#include "mir/MIR.hpp"
#include "mir/utils.hpp"
#include "mir/target.hpp"
#include "mir/datalayout.hpp"
#include "mir/registerinfo.hpp"
#include "mir/lowering.hpp"
#include "target/riscv/RISCV.hpp"
#include "autogen/riscv/InstInfoDecl.hpp"
#include "autogen/riscv/ISelInfoDecl.hpp"
#include "autogen/riscv/ScheduleModelDecl.hpp"
// clang-format on

namespace mir {
/*
 * @brief: RISCVDataLayout Class
 * @note: RISC-V数据信息 (64位)
 */
static const std::string SysYRuntime =
#include "autogen/riscv/RuntimeWithParallelFor.hpp"
  ;
class RISCVDataLayout final : public DataLayout {
public:
  Endian edian() const override { return Endian::Little; }
  size_t typeAlign(ir::Type* type) const override { return 4; }
  size_t pointerSize() const override { return 8; }
  size_t codeAlign() const override { return 4; }
  size_t memAlign() const override { return 8; }
};

/*
 * @brief: RISCVFrameInfo Class
 * @note: RISC-V帧相关信息
 */
class RISCVFrameInfo : public TargetFrameInfo {
public:  // lowering stage
  void emitCall(ir::CallInst* inst, LoweringContext& lowering_ctx) override;
  // 在函数调用前生成序言代码，用于设置栈帧和保存寄存器状态。
  void emitPrologue(MIRFunction* func, LoweringContext& lowering_ctx) override;
  void emitReturn(ir::ReturnInst* ir_inst, LoweringContext& lowering_ctx) override;

public:  // ra stage (register allocation stage)
  // 调用者保存寄存器
  bool isCallerSaved(const MIROperand& op) override {
    const auto reg = op.reg();
    // $ra $t0-$t6 $a0-$a7 $ft0-$ft11 $fa0-$fa7
    return reg == RISCV::X1 || (RISCV::X5 <= reg && reg <= RISCV::X7) ||
           (RISCV::X10 <= reg && reg <= RISCV::X17) || (RISCV::X28 <= reg && reg <= RISCV::X31) ||
           (RISCV::F0 <= reg && reg <= RISCV::F7) || (RISCV::F10 <= reg && reg <= RISCV::F17) ||
           (RISCV::F28 <= reg && reg <= RISCV::F31);
  }
  // 被调用者保存寄存器
  bool isCalleeSaved(const MIROperand& op) override {
    const auto reg = op.reg();
    // $sp $s0-$s7 $f20-$f30 $gp
    return reg == RISCV::X2 || (RISCV::X8 <= reg && reg <= RISCV::X9) ||
           (RISCV::X18 <= reg && reg <= RISCV::X27) || (RISCV::F8 <= reg && reg <= RISCV::F9) ||
           (RISCV::F18 <= reg && reg <= RISCV::F27) || reg == RISCV::X3;
  }

public:  // sa stage (stack allocation stage)
  int stackPointerAlign() override { return 8; }
  void emitPostSAPrologue(MIRBlock* entry, int32_t stack_size) override {
    auto& insts = entry->insts();
    RISCV::adjust_reg(insts, insts.begin(), RISCV::sp, RISCV::sp, -stack_size);
  }
  void emitPostSAEpilogue(MIRBlock* exit, int32_t stack_size) override {
    auto& insts = exit->insts();
    RISCV::adjust_reg(insts, std::prev(insts.end()), RISCV::sp, RISCV::sp, stack_size);
  }
  int32_t insertPrologueEpilogue(
    MIRFunction* func,
    std::unordered_set<MIROperand, MIROperandHasher>& callee_saved_regs,
    CodeGenContext& ctx,
    MIROperand return_addr_reg) override;
};

/*
 * @brief: RISCVRegisterInfo Class
 * @note: RISC-V寄存器相关信息
 */
class RISCVRegisterInfo : public TargetRegisterInfo {
public:  // get function
  /* GPR(General Purpose Registers)/FPR(Floating Point Registers) */
  uint32_t get_alloca_class_cnt() { return 2; }
  uint32_t getAllocationClass(OperandType type) {
    switch (type) {
      case OperandType::Bool:
      case OperandType::Int8:
      case OperandType::Int16:
      case OperandType::Int32:
      case OperandType::Int64:
        return 0;
      case OperandType::Float32:
        return 1;
      default:
        assert(false && "invalid alloca class");
    }
  }
  std::vector<uint32_t>& get_allocation_list(uint32_t classId);
  OperandType getCanonicalizedRegisterTypeForClass(uint32_t classId) {
    return classId == 0 ? OperandType::Int64 : OperandType::Float32;
  }
  OperandType getCanonicalizedRegisterType(OperandType type) {
    switch (type) {
      case OperandType::Bool:
      case OperandType::Int8:
      case OperandType::Int16:
      case OperandType::Int32:
      case OperandType::Int64:
        return OperandType::Int64;
      case OperandType::Float32:
        return OperandType::Float32;
      default:
        assert(false && "valid operand type");
    }
  }
  OperandType getCanonicalizedRegisterType(uint32_t reg) {
    if (reg >= RISCV::GPRBegin and reg <= RISCV::GPREnd) {
      return OperandType::Int64;
    } else if (reg >= RISCV::FPRBegin and reg <= RISCV::FPREnd) {
      return OperandType::Float32;
    } else {
      assert(false && "valid operand type");
    }
  }
  MIROperand get_return_address_register() { return RISCV::ra; }
  MIROperand get_stack_pointer_register() { return RISCV::sp; }

public:  // check function
  bool is_legal_isa_reg_operand(MIROperand& op) {
    std::cerr << "Not Impl is_legal_isa_reg_operand" << std::endl;
    return false;
  }
  bool is_zero_reg(const uint32_t x) const { return x == RISCV::RISCVRegister::X0; }
};

/*
 * @brief: RISCVTarget Class
 * @note: RISC-V架构后端
 */
class RISCVTarget : public Target {
  RISCVDataLayout mDatalayout;
  RISCVFrameInfo mFrameInfo;
  RISCVRegisterInfo mRegisterInfo;

public:
  RISCVTarget() = default;

public:  // get function
  DataLayout& getDataLayout() override { return mDatalayout; }
  TargetFrameInfo& getTargetFrameInfo() override { return mFrameInfo; }
  TargetRegisterInfo& getRegisterInfo() override { return mRegisterInfo; }
  TargetInstInfo& getTargetInstInfo() override { return RISCV::getRISCVInstInfo(); }
  TargetISelInfo& getTargetIselInfo() override { return RISCV::getRISCVISelInfo(); }
  TargetScheduleModel& getScheduleModel() override { return RISCV::getRISCVScheduleModel(); }

public:  // emit_assembly
  void postLegalizeFunc(MIRFunction& func, CodeGenContext& ctx) override;
  void emit_assembly(std::ostream& out, MIRModule& module) override;

  bool verify(MIRModule& module) override;
  bool verify(MIRFunction& func) override;
};

void addExternalIPRAInfo(class IPRAUsageCache& infoIPRA);
}  // namespace mir