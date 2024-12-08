#include "mir/MIR.hpp"
#include "mir/instinfo.hpp"
#include "autogen/generic/InstInfoDecl.hpp"
#include "support/StaticReflection.hpp"

namespace mir {

static bool isOperandFReg(MIROperand operand) {
  return operand.isReg() and isFloatType(operand.type());
}

uint32_t offset = GENERIC::GENERICInstBegin + 1;
const InstInfo& TargetInstInfo::getInstInfo(uint32_t opcode) const {
  return GENERIC::getGENERICInstInfo().getInstInfo(opcode + offset);
}

bool TargetInstInfo::matchBranch(MIRInst* inst, MIRBlock*& target, double& prob) const {
  auto oldOpcode = inst->opcode();
  inst->set_opcode(oldOpcode + offset);
  bool res = GENERIC::getGENERICInstInfo().matchBranch(inst, target, prob);
  inst->set_opcode(oldOpcode);
  return res;
}

bool TargetInstInfo::matchUnconditionalBranch(MIRInst* inst, MIRBlock*& Target) const {
  double prob = 0.0;
  return matchBranch(inst, Target, prob) &&
         requireFlag(getInstInfo(inst).inst_flag(), InstFlagNoFallThrough);
}

bool TargetInstInfo::matchConditionalBranch(MIRInst* inst, MIRBlock*& target, double& prob) const {
    return matchBranch(inst, target, prob) && 
           !requireFlag(getInstInfo(inst).inst_flag(), InstFlagNoFallThrough);
}

void TargetInstInfo::redirectBranch(MIRInst* inst, MIRBlock* target) const {
  auto oldOpcode = inst->opcode();
  inst->set_opcode(oldOpcode + offset);
  GENERIC::getGENERICInstInfo().redirectBranch(inst, target);
  inst->set_opcode(oldOpcode);
}

bool TargetInstInfo::matchCopy(MIRInst* inst, MIROperand& dst, MIROperand& src) const {
  const auto& info = getInstInfo(inst);
  if (requireFlag(info.inst_flag(), InstFlagRegCopy)) {
    if (info.operand_num() != 2) {
      std::cerr << "Error: invalid operand number for copy instruction: \n";
      info.print(std::cerr, *inst, false);
      std::cerr << std::endl;
    }
    assert(info.operand_num() == 2);
    dst = inst->operand(0);
    src = inst->operand(1);
    return (isOperandIReg(dst) and isOperandIReg(src)) or
           (isOperandFReg(dst) and isOperandFReg(src));
  }
  return false;
}

}  // namespace mir

#include "autogen/generic/InstInfoImpl.hpp"
