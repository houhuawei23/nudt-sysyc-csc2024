#pragma once
#include <array>
#include <list>
#include <variant>
#include <vector>
#include "ir/ir.hpp"
#include "support/arena.hpp"

namespace mir {
class MIRRelocable;
class MIRModule;
class MIRFunction;
class MIRBlock;
class MIRInst;
class MIRRegister;
class MIROperand;
struct MIRGlobalObject;
class MIRZeroStorage;
class MIRDataStorage;
struct StackObject;
struct CodeGenContext;

enum CompareOp : uint32_t {
  ICmpEqual,
  ICmpNotEqual,
  ICmpSignedLessThan,
  ICmpSignedLessEqual,
  ICmpSignedGreaterThan,
  ICmpSignedGreaterEqual,
  ICmpUnsignedLessThan,
  ICmpUnsignedLessEqual,
  ICmpUnsignedGreaterThan,
  ICmpUnsignedGreaterEqual,

  FCmpOrderedEqual,
  FCmpOrderedNotEqual,
  FCmpOrderedLessThan,
  FCmpOrderedLessEqual,
  FCmpOrderedGreaterThan,
  FCmpOrderedGreaterEqual,
  FCmpUnorderedEqual,
  FCmpUnorderedNotEqual,
  FCmpUnorderedLessThan,
  FCmpUnorderedLessEqual,
  FCmpUnorderedGreaterThan,
  FCmpUnorderedGreaterEqual
};

/* MIRRelocable */
class MIRRelocable {
  std::string mName;
public:
  MIRRelocable(const std::string& name="") : mName(name) {}
  virtual ~MIRRelocable() = default;
public:  // get function
  auto name() const { return mName; }
public:  // utils function
  virtual void print(std::ostream& os, CodeGenContext& ctx) = 0;
  template <typename T> const T* dynCast() const {
    static_assert(std::is_base_of_v<MIRRelocable, T>);
    return dynamic_cast<const T*>(this);
  }
};

/* utils function */
constexpr uint32_t virtualRegBegin = 0b0101U << 28;
constexpr uint32_t stackObjectBegin = 0b1010U << 28;
constexpr uint32_t invalidReg = 0b1100U << 28;

constexpr bool isISAReg(uint32_t x) { return x < virtualRegBegin; }
constexpr bool isVirtualReg(uint32_t x) { return (x & virtualRegBegin) == virtualRegBegin; }
constexpr bool isStackObject(uint32_t x) { return (x & stackObjectBegin) == stackObjectBegin; }

enum class OperandType : uint32_t {
  Bool,
  Int8,
  Int16,
  Int32,
  Int64,
  Float32,
  Special,
  HighBits,
  LowBits,
  Alignment
};

constexpr bool isIntType(OperandType type) { return type <= OperandType::Int64; }
constexpr bool isFloatType(OperandType type) { return type == OperandType::Float32; }

constexpr uint32_t getOperandSize(const OperandType type) {
  /* NOTE: RISC-V 64 */
  switch (type) {
    case OperandType::Int8: return 1;
    case OperandType::Int16: return 2;
    case OperandType::Int32: return 4;
    case OperandType::Int64: return 8;
    case OperandType::Float32: return 4;
    default: assert(false && "invalid operand type");
  }
}

/* MIRRegisterFlag */
enum MIRRegisterFlag : uint32_t {
  RegisterFlagNone = 0,
  RegisterFlagDead = 1 << 1,
};

/* MIRRegister */
class MIRRegister {
  uint32_t mReg;
  MIRRegisterFlag mFlag = RegisterFlagNone;

public:
  MIRRegister() = default;
  MIRRegister(uint32_t reg) : mReg(reg) {}

public:  // operator
  bool operator==(const MIRRegister& rhs) const { return mReg == rhs.mReg; }
  bool operator!=(const MIRRegister& rhs) const { return mReg != rhs.mReg; }

public:  // get function
  auto reg() const { return mReg; }
  auto flag() const { return mFlag; }
  MIRRegisterFlag* flag_ptr() { return &mFlag; }

public:  // set function
  void set_flag(MIRRegisterFlag flag) { mFlag = flag; }

public:
  void print(std::ostream& os);
};

}  // namespace mir

namespace std {
template <>
struct hash<mir::MIRRegister> {
  size_t operator()(const mir::MIRRegister& reg) const noexcept {
    return hash<uint32_t>{}(reg.reg());
  }
};
}  // namespace std

namespace mir {
enum MIRGenericInst : uint32_t {
  // Jump
  InstJump,
  InstBranch,
  InstUnreachable,

  // Memory
  InstLoad,
  InstStore,

  // Arithmetic
  InstAdd,
  InstSub,
  InstMul,
  InstUDiv,
  InstURem,  // 模运算 (无符号运算)

  // Bitwise
  InstAnd,
  InstOr,
  InstXor,
  InstShl,
  InstLShr,  // logic shift right
  InstAShr,  // arth shift right

  // Signed Div/Rem
  InstSDiv,
  InstSRem,  // 模运算 (有符号运算)

  // MinMax
  InstSMin,
  InstSMax,

  // Unary
  InstNeg,
  InstAbs,

  // Float
  InstFAdd,
  InstFSub,
  InstFMul,
  InstFDiv,
  InstFNeg,
  InstFAbs,
  InstFFma,

  // Cmp
  InstICmp,
  InstFCmp,

  // Conversion
  InstSExt,
  InstZExt,
  InstTrunc,
  InstF2U,
  InstF2S,
  InstU2F,
  InstS2F,
  InstFCast,

  // Misc
  InstCopy,
  InstSelect,
  InstLoadGlobalAddress,
  InstLoadImm,
  InstLoadStackObjectAddr,
  InstCopyFromReg,
  InstCopyToReg,  // 45
  InstLoadImmToReg,
  InstLoadRegFromStack,
  InstStoreRegToStack,

  // Bitcast
  InstBitCast, 

  // Return
  InstReturn,

  // Atomic inst
  InstAtomicAdd,
  InstAtomicSub,

  // ISA Specific
  ISASpecificBegin,
};

/* MIROperand */
class MIROperand {
private:
  std::variant<std::monostate, MIRRegister, MIRRelocable*, intmax_t, double> mStorage{std::monostate{}};
  OperandType mType = OperandType::Special;
public:
  MIROperand() = default;
  template <typename T> MIROperand(T x, OperandType type) : mStorage(x), mType(type) {}
public:  // get function
  auto& storage() { return mStorage; }
  auto type() const { return mType; }
  auto imm() const {
    assert(isImm());
    return std::get<intmax_t>(mStorage);
  }
  auto prob() const {
    assert(isProb());
    return std::get<double>(mStorage);
  }
  auto reg() const {
    assert(isReg());
    return std::get<MIRRegister>(mStorage).reg();
  }
  auto reloc() const {
    assert(isReloc());
    return std::get<MIRRelocable*>(mStorage);
  }
  auto reg_flag() {
    assert(isReg() && "the operand is not a register");
    return std::get<MIRRegister>(mStorage).flag();
  }
public:  // operator
  bool operator==(const MIROperand& rhs) const {
    return mStorage == rhs.mStorage;
  }
  bool operator!=(const MIROperand& rhs) const {
    return mStorage != rhs.mStorage;
  }
public:  // check function
  constexpr bool isUnused() const {
    return std::holds_alternative<std::monostate>(mStorage);
  }
  constexpr bool isImm() const {
    return std::holds_alternative<intmax_t>(mStorage);
  }
  constexpr bool isReg() const {
    return std::holds_alternative<MIRRegister>(mStorage);
  }
  constexpr bool isReloc() const {
    return std::holds_alternative<MIRRelocable*>(mStorage);
  }
  constexpr bool isProb() const {
    return std::holds_alternative<double>(mStorage);
  }
  constexpr bool isInit() const {
    return !std::holds_alternative<std::monostate>(mStorage);
  }
  template <typename T> bool is() const {
    return std::holds_alternative<T>(mStorage);
  }
public:  // gen function
  template <typename T> static auto asImm(T val, OperandType type) {
    return MIROperand(static_cast<intmax_t>(val), type);
  }
  // FIXME: have static instance, cant ue utils::make
  static auto asISAReg(uint32_t reg, OperandType type) {
    return MIROperand(MIRRegister(reg), type);
  }
  static auto asVReg(uint32_t reg, OperandType type) {
    return MIROperand(MIRRegister(reg + virtualRegBegin), type);
  }
  static auto asStackObj(uint32_t reg, OperandType type) {
    return MIROperand(MIRRegister(reg + stackObjectBegin), type);
  }
  static auto asReloc(MIRRelocable* reloc) {
    return MIROperand(reloc, OperandType::Special);
  }
  static auto asProb(double prob) {
    return MIROperand(prob, OperandType::Special);
  }
public:
  size_t hash() const {
    return std::hash<std::decay_t<decltype(mStorage)>>{}(mStorage);
  }
};

SYSYC_ARENA_TRAIT(MIROperand, MIR)

/* MIROperandHasher */
struct MIROperandHasher final {
  size_t operator()(const MIROperand& operand) const { return operand.hash(); }
};

#include <initializer_list>
/* MIRInst */
class MIRInst {
public:
  static constexpr int max_operand_num = 7;
protected:
  uint32_t mOpcode;  // 标明指令的类型
  MIRBlock* mBlock;  // 标明指令所在的块
  std::array<MIROperand, max_operand_num> mOperands;  // 指令操作数
public:
  static constexpr auto arenaSource = utils::Arena::Source::MIR;
  MIRInst(uint32_t opcode) : mOpcode(opcode) {}
  MIRInst(uint32_t opcode, std::initializer_list<MIROperand> operands) {
    mOpcode = opcode;
    for (auto it = operands.begin(); it != operands.end(); ++it) {
      assert(it->isInit());
      mOperands[it - operands.begin()] = *it;
    }
  }
public:  // operator
  bool operator==(const MIRInst& rhs) const {
    return mOpcode == rhs.mOpcode && mOperands == rhs.mOperands;
  }
public:  // get function
  uint32_t opcode() const { return mOpcode; }
  auto operand(int idx) const {
    assert(idx < max_operand_num);
    return mOperands.at(idx);
  }
  MIROperand& operand(int idx) {
    assert(idx < max_operand_num);
    return mOperands.at(idx);
  }

public:  // set function
  auto set_opcode(uint32_t opcode) {
    mOpcode = opcode;
    return this;
  }
  MIRInst* set_operand(int idx, MIROperand operand) {
    // assert(idx < max_operand_num && opeand != nullptr);
    assert(idx < max_operand_num && operand.isInit());
    mOperands[idx] = operand;
    return this;
  }
  auto resetOperands(std::initializer_list<MIROperand> operands) {
    for (auto it = operands.begin(); it != operands.end(); ++it) {
      // assert(*it != nullptr);
      // mOperands[it - operands.begin()] = *it;
      assert(it->isInit());
      mOperands[it - operands.begin()] = *it;
    }
    return this;
  }

public:
  void print(std::ostream& os);
  bool verify(std::ostream& os, CodeGenContext& ctx) const;
};
using MIRInstList = std::list<MIRInst*>;

SYSYC_ARENA_TRAIT(MIRInst, MIR)

/* MIRBlock Class */
class MIRBlock : public MIRRelocable {
private:
  MIRFunction* mFunction;
  MIRInstList mInsts;
  double mTripCount = 0.0;

public:
  MIRBlock() = default;
  MIRBlock(MIRFunction* parent, const std::string& name = "")
    : MIRRelocable(name), mFunction(parent) {}

public:
  void add_inst(MIRInst* inst) { mInsts.push_back(inst); }

public:  // get function
  auto parent() const { return mFunction; }
  auto& insts() { return mInsts; }
  auto trip_count() { return mTripCount; }

public:  // set function
  void set_trip_count(double trip_count) { mTripCount = trip_count; }

public:
  void print(std::ostream& os, CodeGenContext& ctx) override;
  bool verify(std::ostream& os, CodeGenContext& ctx) const;
};

/* StackObjectUsage */
// in a function (my)
enum class StackObjectUsage {
  Argument,        // my Argument
  CalleeArgument,  // for my callee func argument
  Local,           // my local variable
  RegSpill,        // my register spill
  CalleeSaved      // funct is a callee, register it should save and restore
};

/* StackObject */
struct StackObject final {
  uint32_t size;
  uint32_t alignment;
  int32_t offset;
  StackObjectUsage usage;
};

/* MIRJumpTable */
class MIRJumpTable final : public MIRRelocable {
private:
  std::vector<MIRRelocable*> mData;

public:
  explicit MIRJumpTable(std::string symbol) : MIRRelocable(symbol) {}

public:  // get function
  auto& data() { return mData; }
  void print(std::ostream& os, CodeGenContext& ctx) {}
};

/* MIRFunction */
class MIRFunction : public MIRRelocable {
private:
  ir::Function* mIRFunc;
  MIRModule* mModule;
  std::list<std::unique_ptr<MIRBlock>> mBlocks;
  std::unordered_map<MIROperand, StackObject, MIROperandHasher> mStackObjects;
  std::vector<MIROperand> mArguments;

public:
  MIRFunction(ir::Function* ir_func, MIRModule* parent);
  MIRFunction(const std::string& name, MIRModule* parent)
    : MIRRelocable(name), mModule(parent) {}

  auto& blocks() { return mBlocks; }
  auto& args() { return mArguments; }
  auto& stackObjs() { return mStackObjects; }

  auto newStackObject(uint32_t id,
                      uint32_t size,
                      uint32_t alignment,
                      int32_t offset,
                      StackObjectUsage usage) {
    auto ref = MIROperand::asStackObj(id, OperandType::Special);
    mStackObjects.emplace(ref, StackObject{size, alignment, offset, usage});
    return ref;
  }

public:  // utils function
  void print(std::ostream& os, CodeGenContext& ctx) override;
  void print_cfg(std::ostream& os);
  bool verify(std::ostream& os, CodeGenContext& ctx) const;
};

/* MIRZeroStorage */
class MIRZeroStorage : public MIRRelocable {
  size_t mSize;  // bytes
  bool mIsFloat;

public:
  MIRZeroStorage(size_t size,
                 const std::string& name = "",
                 bool is_float = false)
    : MIRRelocable(name), mSize(size), mIsFloat(is_float) {}

public:
  auto is_float() const { return mIsFloat; }

public:
  void print(std::ostream& os, CodeGenContext& ctx) override;
};

/* MIRDataStorage */
class MIRDataStorage : public MIRRelocable {
public:
  using Storage = std::vector<uint32_t>;
private:
  Storage mData;
  bool mIsFloat;
  bool mIsReadonly;
public:
  MIRDataStorage(const Storage data, bool readonly,
                 const std::string& name="", bool is_float=false)
    : MIRRelocable(name), mData(data), mIsReadonly(readonly), mIsFloat(is_float) {}
public:  // check function
  auto is_readonly() const { return mIsReadonly; }
  auto is_float() const { return mIsFloat; }
public:  // set function
  // return the index of this word
  size_t append_word(uint32_t word) {
    auto idx = static_cast<size_t>(mData.size());
    mData.push_back(word);
    return idx;
  }
public:  // utils function
  void print(std::ostream& os, CodeGenContext& ctx) override;
};

/* MIRGlobalObject */
using MIRRelocable_UPtr = std::unique_ptr<MIRRelocable>;
struct MIRGlobalObject {
  MIRModule* parent;
  size_t align;
  MIRRelocable_UPtr reloc; /* MIRZeroStorage OR MIRDataStorage */

  MIRGlobalObject() = default;
  MIRGlobalObject(size_t align,
                  std::unique_ptr<MIRRelocable> reloc,
                  MIRModule* parent = nullptr)
    : parent(parent), align(align), reloc(std::move(reloc)) {}
  void print(std::ostream& os);
};

/* MIRModule */
class Target;
using MIRFunction_UPtrVec = std::vector<std::unique_ptr<MIRFunction>>;
using MIRGlobalObject_UPtrVec = std::vector<std::unique_ptr<MIRGlobalObject>>;
class MIRModule {
private:
  utils::Arena mArena;

  Target& mTarget;
  MIRFunction_UPtrVec mFunctions;
  MIRGlobalObject_UPtrVec mGlobalObjects;
  ir::Module* mIRModule;

public:
  MIRModule(ir::Module* ir_module, Target& target)
    : mIRModule(ir_module), mTarget(target), mArena{utils::Arena::Source::MIR} {}

  MIRFunction_UPtrVec& functions() { return mFunctions; }
  MIRGlobalObject_UPtrVec& global_objs() { return mGlobalObjects; }

public:
  void print(std::ostream& os);
  bool verify() const;
};

class MIRBuilder {
protected:
  MIRFunction* mCurrFunc = nullptr;
  MIRBlock* mCurrBlock = nullptr;
  MIRInstList::iterator mInsertPoint;

public:
  MIRBuilder() = default;

  auto currFunc() const { return mCurrFunc; }
  auto currBlock() const { return mCurrBlock; }
  auto insertPoint() const { return mInsertPoint; }

  void setCurrFunc(MIRFunction* func) { mCurrFunc = func; }
  void setCurrBlock(MIRBlock* block) { mCurrBlock = block; }
  void setInsertPoint(MIRInstList::iterator point) { mInsertPoint = point; }

  /** makeMIRInst: make identical MIRInst */
  template <typename... Args>
  auto makeMIRInst(Args&&... args) {
    auto inst = utils::make<MIRInst>(std::forward<Args>(args)...);
    return inst;
  }
  /** pass initializer_list as operands:
   * makeMIRInst(InstAdd, {dst, src1, src2})
   */
  template <typename T, typename U>
  auto makeMIRInst(T&& arg1, std::initializer_list<U> arg2) {
    return utils::make<MIRInst>(std::forward<T>(arg1),
                                std::forward<std::initializer_list<U>>(arg2));
  }

  /** insert MIRInst before mInsertPoint */
  template <typename T, typename U>
  auto insertMIRInst(T&& arg1, std::initializer_list<U> arg2) {
    auto inst = makeMIRInst(arg1, arg2);
    mCurrBlock->insts().emplace(mInsertPoint, inst);
    return inst;
  }

  template <typename T, typename U>
  auto insertMIRInst(MIRInstList::iterator pos, T&& arg1, std::initializer_list<U> arg2) {
    auto inst = makeMIRInst(arg1, arg2);
    mCurrBlock->insts().emplace(pos, inst);
    return inst;
  }

  /** emit MIRInst at the end of mCurrBlock */
  template <typename T, typename U>
  auto emitMIRInst(T&& arg1, std::initializer_list<U> arg2) {
    auto inst = makeMIRInst(arg1, arg2);
    mCurrBlock->insts().emplace_back(inst);
    return inst;
  }

  template <typename... Args>
  auto emitMIRInst(Args&&... args) {
    auto inst = makeMIRInst(std::forward<Args>(args)...);
    mCurrBlock->insts().emplace_back(inst);
    return inst;
  }
};
}  // namespace mir