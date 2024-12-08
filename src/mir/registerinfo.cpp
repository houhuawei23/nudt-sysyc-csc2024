#include "mir/registerinfo.hpp"

namespace mir {

RegisterSelector::RegisterSelector(const std::vector<uint32_t>& list) {
  assert(list.size() <= maxRegisterCount);
  mFree = (static_cast<int64_t>(1) << list.size()) - 1;
  for (uint32_t idx = 0; idx < list.size(); ++idx) {
    const auto reg = list[idx];
    mIdx2Reg[idx] = reg;
    if (reg >= mReg2Idx.size()) mReg2Idx.resize(reg + 1, invalidReg);
    mReg2Idx[reg] = idx;
  }
}

void RegisterSelector::markAsDiscarded(uint32_t reg) {
  assert(mReg2Idx[reg] != invalidReg);
  const auto mask = static_cast<int64_t>(1) << mReg2Idx[reg];
  assert((mFree & mask) == 0);
  mFree ^= mask;
}
void RegisterSelector::markAsUsed(uint32_t reg) {
  assert(mReg2Idx[reg] != invalidReg);
  const auto mask = static_cast<int64_t>(1) << mReg2Idx[reg];
  assert((mFree & mask) == mask);
  mFree ^= mask;
}

bool RegisterSelector::isFree(uint32_t reg) const {
  if (mReg2Idx[reg] == invalidReg) return false;
  const auto mask = static_cast<int64_t>(1) << mReg2Idx[reg];
  return (mFree & mask) == mask;
}

uint32_t RegisterSelector::getFreeRegister() const {
  if (mFree == 0) return invalidReg;
  return mIdx2Reg[static_cast<uint32_t>(__builtin_ctzll(static_cast<uint64_t>(mFree)))];
}

void MultiClassRegisterSelector::markAsDiscarded(MIROperand reg) {
  assert(isISAReg(reg.reg()));
  const auto classId = mRegisterInfo.getAllocationClass(reg.type());
  auto& selector = *mSelectors[classId];
  selector.markAsDiscarded(reg.reg());
}
void MultiClassRegisterSelector::markAsUsed(MIROperand reg) {
  assert(isISAReg(reg.reg()));
  const auto classId = mRegisterInfo.getAllocationClass(reg.type());
  auto& selector = *mSelectors[classId];
  selector.markAsUsed(reg.reg());
}
bool MultiClassRegisterSelector::isFree(MIROperand reg) const {
  assert(isISAReg(reg.reg()));
  const auto classId = mRegisterInfo.getAllocationClass(reg.type());
  auto& selector = *mSelectors[classId];
  return selector.isFree(reg.reg());
}
MIROperand MultiClassRegisterSelector::getFreeRegister(OperandType type) {
  const auto classId = mRegisterInfo.getAllocationClass(type);
  const auto& selector = *mSelectors[classId];
  const auto reg = selector.getFreeRegister();
  if (reg == invalidReg) return MIROperand{};
  return MIROperand::asISAReg(reg, mRegisterInfo.getCanonicalizedRegisterType(type));
}

}  // namespace mir
