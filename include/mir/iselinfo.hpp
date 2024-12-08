#pragma once
#include <unordered_set>
#include "mir/MIR.hpp"
#include "mir/LiveInterval.hpp"
#include "mir/instinfo.hpp"
#include "mir/lowering.hpp"
#include "support/arena.hpp"
#include "support/Bits.hpp"
#include <optional>

namespace mir {
class CodeGenContext;
class LoweringContext;
class ISelContext : public MIRBuilder {
  CodeGenContext& mCodeGenCtx;
  std::unordered_map<MIROperand, MIRInst*, MIROperandHasher> mDefinedInst, mConstantMap;

  // mReplaceList
  std::unordered_map<MIROperand, MIROperand, MIROperandHasher> mReplaceMap;

  std::unordered_set<MIRInst*> mRemoveWorkList, mReplaceBlockList;

  std::unordered_map<MIROperand, uint32_t, MIROperandHasher> mUseCnt;

public:
  ISelContext(CodeGenContext& ctx) : mCodeGenCtx(ctx) {}

  void runInstSelect(MIRFunction* func);
  bool runInstSelectImpl(MIRFunction* func);
  bool hasOneUse(MIROperand op);

  /* lookup the inst that defines the operand */
  MIRInst* lookupDefInst(const MIROperand& op) const;

  void remove_inst(MIRInst* inst);
  void replace_operand(MIROperand src, MIROperand dst);

  MIROperand& getInstDefOperand(MIRInst* inst);

  void insert_inst(MIRInst* inst) {
    assert(inst != nullptr);
    mCurrBlock->insts().emplace(mInsertPoint, inst);
  }
  CodeGenContext& codegen_ctx() { return mCodeGenCtx; }
  MIRBlock* curr_block() { return mCurrBlock; }

  void clearInfo() {
    mRemoveWorkList.clear();
    mReplaceBlockList.clear();
    mReplaceMap.clear();

    mConstantMap.clear();
    mUseCnt.clear();
    mDefinedInst.clear();
  }
  void calConstantMap(MIRFunction* func);
  void collectDefinedInst(MIRBlock* block);
};

class InstLegalizeContext final : public MIRBuilder {
public:
  MIRInst*& inst;
  MIRInstList& instructions;
  MIRInstList::iterator iter;
  CodeGenContext& codeGenCtx;
  std::optional<std::list<std::unique_ptr<MIRBlock>>::iterator> blockIter;
  MIRFunction& func;

public:
  InstLegalizeContext(MIRInst*& i,
                      MIRInstList& insts,
                      MIRInstList::iterator iter,
                      CodeGenContext& ctx,
                      std::optional<std::list<std::unique_ptr<MIRBlock>>::iterator> biter,
                      MIRFunction& f)
    : inst(i), instructions(insts), iter(iter), codeGenCtx(ctx), blockIter(biter), func(f) {}
};

class TargetISelInfo {
public:
  virtual ~TargetISelInfo() = default;
  virtual bool isLegalInst(uint32_t opcode) const = 0;

  virtual bool match_select(MIRInst* inst, ISelContext& ctx) const = 0;

  virtual void legalizeInstWithStackOperand(const InstLegalizeContext& ctx,
                                            MIROperand op,
                                            StackObject& obj) const = 0;

  virtual void postLegalizeInst(const InstLegalizeContext& ctx) const = 0;
  virtual MIROperand materializeFPConstant(float fpVal, LoweringContext& loweringCtx) const = 0;
  virtual bool lowerInst(ir::Instruction* inst, LoweringContext& loweringCtx) const = 0;
};

static bool isCompareOp(MIROperand operand, CompareOp cmpOp) {
  auto op = static_cast<uint32_t>(operand.imm());
  return op == static_cast<uint32_t>(cmpOp);
}

static bool isICmpEqualityOp(MIROperand operand) {
  const auto op = static_cast<CompareOp>(operand.imm());
  switch (op) {
    case CompareOp::ICmpEqual:
    case CompareOp::ICmpNotEqual:
      return true;
    default:
      return false;
  }
}

// utils function
uint32_t select_copy_opcode(MIROperand dst, MIROperand src);

inline MIROperand getNeg(MIROperand operand) {
  return MIROperand::asImm(-operand.imm(), operand.type());
}

inline MIROperand getHighBits(MIROperand operand) {
  assert(isOperandReloc(operand));
  return MIROperand(operand.storage(), OperandType::HighBits);
}
inline MIROperand getLowBits(MIROperand operand) {
  assert(isOperandReloc(operand));
  return MIROperand(operand.storage(), OperandType::LowBits);
}

/* 关于整数除法/取模运算的优化 */
static bool isOperandSDiv32ByConstantDivisor(const MIROperand& rhs) {
  return isOperandImm(rhs) && rhs.type() == OperandType::Int32 && isSignedImm<32>(rhs.imm()) && !(-1 <= rhs.imm() && rhs.imm() <= 1);
}
static bool select_sdiv32_by_cconstant_divisor(const MIROperand& rhs, MIROperand& magic, MIROperand& shift, MIROperand& factor) {
  /* only available for [-2^31, -2) U [2, 2^31) */
  if (!isOperandSDiv32ByConstantDivisor(rhs)) return false;
  const auto d = static_cast<int32_t>(rhs.imm());
  constexpr uint32_t two31 = 0x80000000;

  const auto ad = static_cast<uint32_t>(std::abs(d));
  const auto t = two31 + (static_cast<uint32_t>(d) >> 31);
  const auto anc = t - 1 - t % ad;
  int32_t p = 31;
  auto q1 = two31 / anc;
  auto r1 = two31 - q1 * anc;
  auto q2 = two31 / ad;
  auto r2 = two31 - q2 * ad;
  uint32_t delta;

  do {
    ++p;
    q1 *= 2;
    r1 *= 2;
    if (r1 >= anc) {
      ++q1;
      r1 -= anc;
    }

    q2 *= 2;
    r2 *= 2;
    if (r2 >= ad) {
      q2 += 1;
      r2 -= ad;
    }
    delta = ad - r2;
  } while (q1 < delta || (q1 == delta && r1 == 0));

  const auto mUnsigned = q2 + 1;
  auto m = static_cast<intmax_t>(static_cast<uint32_t>(mUnsigned));
  int32_t factorVal = 0;
  if (d < 0) {
    m = -m;
    if (m > 0) factorVal = -1;
  } else if (d > 0 && m < 0) {
    factorVal = 1;
  }
  magic = MIROperand::asImm(m, OperandType::Int32);
  shift = MIROperand::asImm(p - 32, OperandType::Int32);
  factor = MIROperand::asImm(factorVal, OperandType::Int32);
  return true;
}

/* NOTE: 被除数是2的幂次常数 -> 移位指令 */
static bool isPowerOf2Divisor(const MIROperand& rhs) {
  return isOperandImm(rhs) && rhs.type() == OperandType::Int32 && isSignedImm<32>(rhs.imm()) && rhs.imm() > 1 && utils::isPowerOf2(static_cast<size_t>(rhs.imm()));
}
static bool select_sdiv32_by_powerof2(const MIROperand& rhs, MIROperand& shift) {
  if (!isPowerOf2Divisor(rhs)) return false;
  shift = MIROperand::asImm(utils::log2(static_cast<size_t>(rhs.imm())), OperandType::Int32);
  return true;
}
}  // namespace mir
