#include "mir/MIR.hpp"
#include "mir/utils.hpp"
#include "target/riscv/RISCVTarget.hpp"
#include "autogen/riscv/InstInfoDecl.hpp"
#include "autogen/riscv/ISelInfoDecl.hpp"
#include "support/StaticReflection.hpp"
#include "mir/RegisterAllocator.hpp"

namespace mir {
/*
 * @brief: get_allocation_list
 * @note:
 *    当进行寄存器分配时, 我们首先考虑指派Caller-Saved Registers
 *    其次再考虑指派Callee-Saved Registers
 * @include:
 */
std::vector<uint32_t>& RISCVRegisterInfo::get_allocation_list(
  uint32_t classId) {
  if (classId == 0) {  // General Purpose Registers
    static std::vector<uint32_t> list{
      // clang-format off
      /* NOTE: Caller-Saved Registers */
      // $a0-$a5
      RISCV::X10, RISCV::X11, RISCV::X12, RISCV::X13, RISCV::X14, RISCV::X15,
      // $t0-$t6 $a6-$a7
      // RISCV::X5,  // t0 - 处理栈内的大数偏移
      RISCV::X6, RISCV::X7, RISCV::X28, RISCV::X29, RISCV::X30, RISCV::X31, RISCV::X16, RISCV::X17,
    
      /* NOTE: Callee-Saved Registers */
      // $s0-$s1
      RISCV::X8, RISCV::X9,
      // $s2-$s11
      RISCV::X18, RISCV::X19, 
      RISCV::X20, RISCV::X21, RISCV::X22, RISCV::X23, 
      RISCV::X24, RISCV::X25, RISCV::X26, RISCV::X27,
      // clang-format on
    };
    return list;
  } else if (classId == 1) {  // Floating Point Registers
    static std::vector<uint32_t> list{
      // clang-format off
      /* NOTE: Caller-Saved Registers */
      // $fa0-$fa5
      RISCV::F10, RISCV::F11, RISCV::F12, RISCV::F13, RISCV::F14, RISCV::F15,
      // $ft0-$ft11 $fa6-$fa7
      RISCV::F0, RISCV::F1, RISCV::F2, RISCV::F3, 
      RISCV::F4, RISCV::F5, RISCV::F6, RISCV::F7,
      RISCV::F28, RISCV::F29, RISCV::F30, RISCV::F31,
      RISCV::F16, RISCV::F17,

      /* NOTE: Callee-Saved Registers */
      // $fs0-$fs1
      RISCV::F8, RISCV::F9,
      // $fs2-$fs11
      RISCV::F18, RISCV::F19, RISCV::F20, RISCV::F21,
      RISCV::F22, RISCV::F23, RISCV::F24, RISCV::F25, 
      RISCV::F26, RISCV::F27,
      // clang-format on
    };
    return list;
  } else {
    assert(false && "invalid type registers");
  }
}

static const std::unordered_set<RegNum> callerSavedRISCVGPR = {
  RISCV::X5,  RISCV::X6,  RISCV::X7,  RISCV::X10, RISCV::X11,
  RISCV::X12, RISCV::X13, RISCV::X14, RISCV::X15, RISCV::X16,
  RISCV::X17, RISCV::X28, RISCV::X29, RISCV::X30, RISCV::X31,
};
static const std::unordered_set<RegNum> callerSavedRISCVFPR = {
  RISCV::F10, RISCV::F11, RISCV::F12, RISCV::F13, RISCV::F14,
  RISCV::F15, RISCV::F0,  RISCV::F1,  RISCV::F2,  RISCV::F3,
  RISCV::F4,  RISCV::F5,  RISCV::F6,  RISCV::F7,  RISCV::F28,
  RISCV::F29, RISCV::F30, RISCV::F31, RISCV::F16, RISCV::F17
};
static const auto externalOnlyGPR = std::vector<std::string>{
  "_memset", "putint", "getch", "getint", "getarray", "putch", "putarray"};
static const auto externalFloat = std::vector<std::string>{
  "getfloat", "putfloat", "getfarray", "putfarray", "putf"};
/* 保存Runtime相关的Caller-Saved Registers */
void addExternalIPRAInfo(IPRAUsageCache& infoIPRA) {
  for (auto name : externalOnlyGPR) {
    infoIPRA.add(name, callerSavedRISCVGPR);
  }

  auto callerSavedRISCVRegs = callerSavedRISCVGPR;
  callerSavedRISCVRegs.insert(callerSavedRISCVFPR.begin(),
                              callerSavedRISCVFPR.end());

  for (auto name : externalFloat) {
    infoIPRA.add(name, callerSavedRISCVRegs);
  }
}

}  // namespace mir