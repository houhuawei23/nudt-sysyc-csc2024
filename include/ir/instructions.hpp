#pragma once
#include "ir/function.hpp"
#include "ir/global.hpp"
#include "ir/infrast.hpp"
#include "ir/value.hpp"
#include "support/utils.hpp"
#include "ir/ConstantValue.hpp"
#include <initializer_list>

namespace ir {

class AllocaInst;
class LoadInst;
class StoreInst;
class GetElementPtrInst;

class ReturnInst;
class BranchInst;

class UnaryInst;
class BinaryInst;

class ICmpInst;
class FCmpInst;
class CallInst;

class PhiInst;

class IndVar;
/**
 * @brief Stack allocation instruction
 * 
 * AllocaInst represents a stack allocation instruction that allocates memory
 * on the stack for a local variable. The allocated memory has automatic
 * storage duration and is automatically freed when the function returns.
 * 
 * Key features:
 * - Allocates space for a single object or array
 * - Returns a pointer to the allocated space
 * - Supports constant allocation annotation
 * - Memory is automatically managed (stack-based)
 * 
 * Example IR:
 * @code
 * %ptr = alloca i32        ; Allocate space for one i32
 * %arr = alloca [10 x i32] ; Allocate space for 10 i32s
 * @endcode
 */
class AllocaInst : public Instruction {
protected:
  bool mIsConst = false;    ///< Whether this allocation is for a constant

public:
  /**
   * @brief Constructs a new AllocaInst
   * @param base_type The type of object to allocate space for
   * @param is_const Whether the allocation is for a constant
   * @param parent The parent basic block
   * @param name Optional name for the instruction
   */
  AllocaInst(Type* base_type,
             bool is_const = false,
             BasicBlock* parent = nullptr,
             const_str_ref name = "")
    : Instruction(vALLOCA, Type::TypePointer(base_type), parent, name), mIsConst(is_const) {}

public:
  /**
   * @brief Gets the type of the allocated object
   * @return Pointer to the base type (the type being allocated)
   */
  Type* baseType() const {
    assert(mType->dynCast<PointerType>() && "type error");
    return mType->as<PointerType>()->baseType();
  }
  
  /**
   * @brief Gets the number of array dimensions (0 for scalars)
   * @return Number of dimensions, or 0 for non-array types
   */
  auto dimsCnt() const {
    if (baseType()->isArray())
      return dyn_cast<ArrayType>(baseType())->dims_cnt();
    else
      return static_cast<size_t>(0);
  }

public:
  /// @brief Checks if this allocates a scalar (non-array) type
  bool isScalar() const { return !baseType()->isArray(); }
  
  /// @brief Checks if this allocation is marked as constant
  bool isConst() const { return mIsConst; }

public:
  /// @brief Runtime type checking for AllocaInst
  static bool classof(const Value* v) { return v->valueId() == vALLOCA; }
  
  /// @brief Prints this instruction to output stream
  void print(std::ostream& os) const override;
  
  /// @brief Dumps this instruction as an operand
  void dumpAsOpernd(std::ostream& os) const override { os << mName; }
  
  /// @brief Creates a copy of this instruction
  Instruction* copy(std::function<Value*(Value*)> getValue) const override;
};

class StoreInst : public Instruction {
public:
  StoreInst(Value* value, Value* ptr, BasicBlock* parent = nullptr)
    : Instruction(vSTORE, Type::void_type(), parent) {
    addOperand(value);
    addOperand(ptr);
  }

public:
  Value* value() const { return operand(0); }
  Value* ptr() const { return operand(1); }

public:
  static bool classof(const Value* v) { return v->valueId() == vSTORE; }
  void print(std::ostream& os) const override;
  Instruction* copy(std::function<Value*(Value*)> getValue) const override;
};

/*
 * @brief Load Instruction
 * @details:
 *      <result> = load <ty>, ptr <pointer>
 */
class LoadInst : public Instruction {
public:
  LoadInst(Value* ptr, Type* type, BasicBlock* parent = nullptr)
    : Instruction(vLOAD, type, parent) {
    addOperand(ptr);
  }

  auto ptr() const { return operand(0); }
  static bool classof(const Value* v) { return v->valueId() == vLOAD; }
  void print(std::ostream& os) const override;
  Instruction* copy(std::function<Value*(Value*)> getValue) const override;
};

/*
 * @brief Return Instruction
 * @details:
 *      ret <type> <value>
 *      ret void
 */
class ReturnInst : public Instruction {
public:
  ReturnInst(Value* value = nullptr, BasicBlock* parent = nullptr, const_str_ref name = "")
    : Instruction(vRETURN, Type::void_type(), parent, name) {
    if (value) {
      addOperand(value);
    }
  }

public:
  bool hasRetValue() const { return not mOperands.empty(); }
  Value* returnValue() const { return hasRetValue() ? operand(0) : nullptr; }

public:
  static bool classof(const Value* v) { return v->valueId() == vRETURN; }
  void print(std::ostream& os) const override;
  Instruction* copy(std::function<Value*(Value*)> getValue) const override;
};

/*
 * @brief Unary Instruction
 * @details:
 *    trunc, zext, sext, fptrunc, fptosi, sitofp,
 *    bitcast, ptrtoint, inttoptr
 */
class UnaryInst : public Instruction {
public:
  UnaryInst(ValueId kind,
            Type* type,
            Value* operand,
            BasicBlock* parent = nullptr,
            const_str_ref name = "")
    : Instruction(kind, type, parent, name) {
    addOperand(operand);
  }

public:  // get function
  auto value() const { return operand(0); }

public:  // utils function
  void print(std::ostream& os) const override;
  Value* getConstantRepl(bool recursive = false) override;
  static bool classof(const Value* v) {
    return v->valueId() >= vUNARY_BEGIN && v->valueId() <= vUNARY_END;
  }
  Instruction* copy(std::function<Value*(Value*)> getValue) const override;
  Instruction* clone() const override;
};

/*
 * @brief: Binary Instruction
 * @note:
 *    1. exp (MUL | DIV | MODULO) exp
 *    2. exp (ADD | SUB) exp
 */
class BinaryInst : public Instruction {
public:
  BinaryInst(ValueId kind,
             Type* type,
             Value* lvalue,
             Value* rvalue,
             BasicBlock* parent = nullptr,
             const std::string name = "")
    : Instruction(kind, type, parent, name) {
    addOperand(lvalue);
    addOperand(rvalue);
  }

public:  // check function
  static bool classof(const Value* v) {
    return v->valueId() >= vBINARY_BEGIN && v->valueId() <= vBINARY_END;
  }
  bool isCommutative() const {
    return valueId() == vADD || valueId() == vFADD || valueId() == vMUL || valueId() == vFMUL;
  }

public:  // get function
  auto lValue() const { return operand(0); }
  auto rValue() const { return operand(1); }

public:  // utils function
  void print(std::ostream& os) const override;
  Value* getConstantRepl(bool recursive = false) override;
  Instruction* copy(std::function<Value*(Value*)> getValue) const override;
  Instruction* clone() const override;
};
/* CallInst */
class CallInst : public Instruction {
  Function* mCallee = nullptr;
  bool mIsTail = false;

public:
  CallInst(Function* callee,
           const_value_ptr_vector rargs = {},
           BasicBlock* parent = nullptr,
           const_str_ref name = "")
    : Instruction(vCALL, callee->retType(), parent, name), mCallee(callee), mIsTail(false) {
    addOperands(rargs);
  }
  CallInst(Function* callee,
           std::initializer_list<Value*> rargs = {},
           BasicBlock* parent = nullptr,
           const_str_ref name = "")
    : Instruction(vCALL, callee->retType(), parent, name), mCallee(callee), mIsTail(false) {
    addOperands(rargs);
  }

  bool istail() { return mIsTail; }
  bool isgetarrayorfarray() {
    return (mCallee->name() == "getarray") || (mCallee->name() == "getfarray");
  }
  bool isputarrayorfarray() {
    return (mCallee->name() == "putarray") || (mCallee->name() == "putfarray");
  }

  void setIsTail(bool b) { mIsTail = b; }

  Function* callee() const { return mCallee; }
  /* real arguments */
  auto& rargs() const { return mOperands; }

  static bool classof(const Value* v) { return v->valueId() == vCALL; }
  void print(std::ostream& os) const override;

  Instruction* copy(std::function<Value*(Value*)> getValue) const override;
  Instruction* clone() const override;
};

/*
 * @brief: Conditional or Unconditional Branch Instruction
 * @note:
 *    1. br i1 <cond>, label <iftrue>, label <iffalse>
 *    2. br label <dest>
 */
class BranchInst : public Instruction {
  bool mIsCond = false;

public:
  /* Condition Branch */
  BranchInst(Value* cond,
             BasicBlock* iftrue,
             BasicBlock* iffalse,
             BasicBlock* parent = nullptr,
             const_str_ref name = "")
    : Instruction(vBR, Type::void_type(), parent, name), mIsCond(true) {
    addOperand(cond);
    addOperand(iftrue);
    addOperand(iffalse);
  }
  /* UnCondition Branch */
  BranchInst(BasicBlock* dest, BasicBlock* parent = nullptr, const_str_ref name = "")
    : Instruction(vBR, Type::void_type(), parent, name), mIsCond(false) {
    addOperand(dest);
  }

public:  // get function
  bool is_cond() const { return mIsCond; }
  auto cond() const {
    assert(mIsCond && "not a conditional branch");
    return operand(0);
  }
  auto iftrue() const {
    assert(mIsCond && "not a conditional branch");
    return operand(1)->as<BasicBlock>();
  }
  auto iffalse() const {
    assert(mIsCond && "not a conditional branch");
    return operand(2)->as<BasicBlock>();
  }
  auto dest() const {
    assert(!mIsCond && "not an unconditional branch");
    return operand(0)->as<BasicBlock>();
  }

public:  // set function
  void replaceDest(BasicBlock* olddest, BasicBlock* newdest);
  void set_iftrue(BasicBlock* bb) {
    assert(mIsCond and "not a conditional branch");
    setOperand(1, bb);
  }
  void set_iffalse(BasicBlock* bb) {
    assert(mIsCond and "not a conditional branch");
    setOperand(2, bb);
  }
  void set_dest(BasicBlock* bb) {
    assert(not mIsCond and "not an unconditional branch");
    setOperand(0, bb);
  }

public:  // utils function
  static bool classof(const Value* v) { return v->valueId() == vBR; }
  void print(std::ostream& os) const override;
  Instruction* copy(std::function<Value*(Value*)> getValue) const override;
};

/*
 * @brief: ICmpInst
 * @note:
 *    <result> = icmp <cond> <ty> <op1>, <op2>
 */
class ICmpInst : public Instruction {
public:
  ICmpInst(ValueId itype,
           Value* lhs,
           Value* rhs,
           BasicBlock* parent = nullptr,
           const_str_ref name = "")
    : Instruction(itype, Type::TypeBool(), parent, name) {
    addOperand(lhs);
    addOperand(rhs);
  }

public:  // get function
  auto lhs() const { return operand(0); }
  auto rhs() const { return operand(1); }

public:  // check function
  bool isReverse(ICmpInst* y);

public:  // set function
  void setCmpOp(ValueId newv) {
    assert(newv >= vICMP_BEGIN and newv <= vICMP_END);
    mValueId = newv;
  }
  void setlhs(Value* v) { setOperand(0, v); }
  void setrhs(Value* v) { setOperand(1, v); }

public:  // utils function
  static bool classof(const Value* v) {
    return v->valueId() >= vICMP_BEGIN && v->valueId() <= vICMP_END;
  }
  void print(std::ostream& os) const override;
  Value* getConstantRepl(bool recursive = false) override;
  Instruction* copy(std::function<Value*(Value*)> getValue) const override;
};

/* FCmpInst */
class FCmpInst : public Instruction {
public:
  FCmpInst(ValueId itype,
           Value* lhs,
           Value* rhs,
           BasicBlock* parent = nullptr,
           const_str_ref name = "")
    : Instruction(itype, Type::TypeBool(), parent, name) {
    addOperand(lhs);
    addOperand(rhs);
  }

public:  // get function
  auto lhs() const { return operand(0); }
  auto rhs() const { return operand(1); }

public:  // check function
  bool isReverse(FCmpInst* y);

public:  // utils function
  static bool classof(const Value* v) {
    return v->valueId() >= vFCMP_BEGIN && v->valueId() <= vFCMP_END;
  }
  void print(std::ostream& os) const override;
  Value* getConstantRepl(bool recursive = false) override;
  // Value* getConstantReplaceRecursive() override;
  Instruction* copy(std::function<Value*(Value*)> getValue) const override;
};

/*
 * @brief: MemsetInst
 * @details:
 *    memset(i8* <dest>, i8 <val>, i64 <len>, i1 <isvolatile>)
 */
class MemsetInst : public Instruction {
public:
  MemsetInst(Value* dst, Value* val, Value* len, Value* isVolatile, BasicBlock* parent = nullptr)
    : Instruction(vMEMSET, Type::void_type(), parent) {
    addOperand(dst);
    addOperand(val);
    addOperand(len);
    addOperand(isVolatile);
  }

public:  // get function
  auto dst() const { return operand(0); }
  auto val() const { return operand(1); }
  auto len() const { return operand(2); }
  auto isVolatile() const { return operand(3); }

public:  // utils function
  static bool classof(const Value* v) { return v->valueId() == vMEMSET; }
  void print(std::ostream& os) const override;
  Instruction* copy(std::function<Value*(Value*)> getValue) const override;
};

/*
 * @brief GetElementPtr Instruction
 * @details:
 *      数组: <result> = getelementptr <type>, <type>* <ptrval>, i32 0, i32
 * <idx> 指针: <result> = getelementptr <type>, <type>* <ptrval>, i32 <idx>
 * @param:
 *      1. mIdx: 数组各个维度的下标索引
 *      2. _id : calculate array address OR pointer address
 */
class GetElementPtrInst : public Instruction {
protected:
  size_t mId = 0;
  std::vector<size_t> mCurDims = {};

public:
  //! 1. Pointer <result> = getelementptr <type>, <type>* <ptrval>, i32 <idx>
  GetElementPtrInst(Type* base_type, Value* ptr, Value* idx, BasicBlock* parent = nullptr)
    : Instruction(vGETELEMENTPTR, Type::TypePointer(base_type), parent) {
    mId = 0;
    addOperand(ptr);
    addOperand(idx);
  }

  //! 2. 高维 Array <result> = getelementptr <type>, <type>* <ptrval>, i32 0, i32 <idx>
  GetElementPtrInst(Type* base_type,
                    Value* ptr,
                    Value* idx,
                    std::vector<size_t> dims,
                    std::vector<size_t> cur_dims,
                    BasicBlock* parent = nullptr)
    : Instruction(vGETELEMENTPTR, Type::TypePointer(Type::TypeArray(base_type, dims)), parent),
      mCurDims(cur_dims) {
    mId = 1;
    addOperand(ptr);
    addOperand(idx);
  }

  //! 3. 一维 Array <result> = getelementptr <type>, <type>* <ptrval>, i32 0, i32 <idx>
  GetElementPtrInst(Type* base_type,
                    Value* ptr,
                    Value* idx,
                    std::vector<size_t> cur_dims,
                    BasicBlock* parent = nullptr)
    : Instruction(vGETELEMENTPTR, Type::TypePointer(base_type), parent), mCurDims(cur_dims) {
    mId = 2;
    addOperand(ptr);
    addOperand(idx);
  }

public:  // get function
  auto value() const { return operand(0); }
  auto index() const { return operand(1); }
  auto getid() const { return mId; }
  Type* baseType() const {
    assert(dyn_cast<PointerType>(type()) && "type error");
    return dyn_cast<PointerType>(type())->baseType();
  }
  auto cur_dims_cnt() const { return mCurDims.size(); }
  auto cur_dims() const { return mCurDims; }

public:  // check function
  bool is_arrayInst() const { return mId != 0; }

public:  // utils function
  static bool classof(const Value* v) { return v->valueId() == vGETELEMENTPTR; }
  void print(std::ostream& os) const override;
  Instruction* copy(std::function<Value*(Value*)> getValue) const override;
};

class PhiInst : public Instruction {
protected:
  size_t mSize;
  std::unordered_map<BasicBlock*, Value*> mbbToVal;

public:
  PhiInst(BasicBlock* parent,
          Type* type,
          const std::vector<Value*>& vals = {},
          const std::vector<BasicBlock*>& bbs = {})
    : Instruction(vPHI, type, parent), mSize(vals.size()) {
    assert(vals.size() == bbs.size() and "number of vals and bbs in phi must be equal!");
    for (size_t i = 0; i < mSize; i++) {
      addOperand(vals.at(i));
      addOperand(bbs.at(i));
      mbbToVal[bbs.at(i)] = vals.at(i);
    }
  }
  auto getValue(size_t k) const { return operand(2 * k); }
  auto getBlock(size_t k) const { return operand(2 * k + 1)->dynCast<BasicBlock>(); }

  auto& incomings() const { return mbbToVal; }

  Value* getvalfromBB(BasicBlock* bb);
  BasicBlock* getbbfromVal(Value* val);

  auto getsize() { return mSize; }
  void addIncoming(Value* val, BasicBlock* bb) {
    if (mbbToVal.count(bb)) {
      assert(false && "Trying to add a duplicated basic block!");
    }
    addOperand(val);
    addOperand(bb);
    // 更新操作数的数量
    mbbToVal[bb] = val;
    mSize++;
  }
  void delValue(Value* val);
  void delBlock(BasicBlock* bb);
  void replaceBlock(BasicBlock* newBB, size_t k);
  void replaceoldtonew(BasicBlock* oldBB, BasicBlock* newBB);
  void refreshMap();

  void print(std::ostream& os) const override;
  Value* getConstantRepl(bool recursive = false) override;
  Instruction* copy(std::function<Value*(Value*)> getValue) const override;
};

class FunctionPtrInst : public Instruction {
public:
  explicit FunctionPtrInst(Function* func, Type* dstType, BasicBlock* parent = nullptr)
    : Instruction(vFUNCPTR, dstType, parent) {
    addOperand(func);
  }
  Function* getFunc() const { return operand(0)->as<Function>(); }
  static bool classof(const Value* v) { return v->valueId() == vFUNCPTR; }
  void print(std::ostream& os) const override;
  Instruction* copy(std::function<Value*(Value*)> getValue) const override { return nullptr; }
};

/*
 * @brief: PtrCastInst
 * @note:
 */
class PtrCastInst : public Instruction {
public:
  explicit PtrCastInst(Value* src, Type* dstType, BasicBlock* parent = nullptr)
    : Instruction(vPTRCAST, dstType, parent) {
    addOperand(src);
  }

public:  // utils function
  static bool classof(const Value* v) { return v->valueId() == vPTRCAST; }
  Value* src() { return operand(0); }
  void print(std::ostream& os) const override;
  Instruction* copy(std::function<Value*(Value*)> getValue) const override { return nullptr; }
};

// Atomic orderings
enum class AtomicOrdering {
  NotAtomic,
  Unordered,
  Monotonic,
  Acquire,
  Release,
  AcquireRelease,
  SequentiallyConsistent
};

class AtomicrmwInst : public Instruction {
  BinaryOp mOpcode;
  AtomicOrdering mOrdering;

public:
  AtomicrmwInst(BinaryOp opcode,
                Value* ptr,
                Value* val,
                AtomicOrdering ordering = AtomicOrdering::SequentiallyConsistent,
                BasicBlock* parent = nullptr)
    : Instruction(vATOMICRMW, ptr->type()->dynCast<PointerType>()->baseType(), parent),
      mOpcode(opcode),
      mOrdering(ordering) {
    addOperand(ptr);
    addOperand(val);
  }

public:
  static bool classof(const Value* v) { return v->valueId() == vATOMICRMW; }
  auto ptr() const { return operand(0); }
  auto val() const { return operand(1); }
  auto opcode() const { return mOpcode; }
  auto ordering() const { return mOrdering; }
  void print(std::ostream& os) const override;
  Instruction* copy(std::function<Value*(Value*)> getValue) const override { return nullptr; }
};

class IndVar {
public:  // only for constant beginvar and stepvar
  ConstantInteger* mbeginVar;
  Value* mbeginVarValue;

  Value* mendVar;

  ConstantInteger* mstepVar;
  Value* mstepVarValue;

  bool mendIsConst;

  BinaryInst* miterInst;
  Instruction* mcmpInst;
  PhiInst* mphiinst;

  bool isBeginAndStepConst;

public:
  IndVar(Value* mbegin,
         Value* mend,
         Value* mstep,
         BinaryInst* bininst,
         Instruction* cmpinst,
         PhiInst* phiinst)
    : mbeginVarValue(mbegin),
      mendVar(mend),
      mstepVarValue(mstep),
      miterInst(bininst),
      mcmpInst(cmpinst),
      mphiinst(phiinst) {
    mendIsConst = mendVar->isa<ConstantInteger>();
    if (auto constBeginVar = mbeginVarValue->dynCast<ConstantInteger>()) {
      mbeginVar = constBeginVar;
    } else {
      mbeginVar = nullptr;
    }
    if (auto constStepVar = mstepVarValue->dynCast<ConstantInteger>()) {
      mstepVar = constStepVar;
    } else {
      mstepVar = nullptr;
    }
    isBeginAndStepConst = mstepVar != nullptr and mbeginVar != nullptr;
  }
  int getBeginI32() {
    if (mbeginVar == nullptr) assert(false and "BeginVar is not i32!");
    return mbeginVar->i32();
  }
  int getStepI32() {
    if (mstepVar == nullptr) assert(false and "StepVar is not i32!");
    return mstepVar->i32();
  }
  bool isEndVarConst() { return mendIsConst; }
  bool isBeginVarConst() { return mbeginVar != nullptr; }
  bool isStepVarConst() { return mstepVar != nullptr; }
  int getEndVarI32() {
    if (mendIsConst) {
      return mendVar->dynCast<ConstantValue>()->i32();
    } else
      assert(false && "endVar is not constant");
  }
  Value* endValue() { return mendVar; }
  Value* beginValue() { return mbeginVarValue; }
  Value* stepValue() { return mstepVarValue; }
  BinaryInst* iterInst() { return miterInst; }
  Instruction* cmpInst() { return mcmpInst; }
  PhiInst* phiinst() { return mphiinst; }
  ConstantInteger* getBegin() { return mbeginVar; }
  ConstantInteger* getStep() { return mstepVar; }
  Value* getEnd() { return mendVar; }
  bool getIsBeginAndStepConst() { return isBeginAndStepConst; }
  void print(std::ostream& os) const {
    os << "beginVar: ";
    mbeginVarValue->print(os);

    os << "\n endVar: ";
    mendVar->print(os);

    os << "\n stepVar: ";
    mstepVarValue->print(os);

    os << "\n iterInst: ";
    miterInst->print(os);

    os << "\n cmpInst: ";
    mcmpInst->print(os);

    os << "\n phiInst: ";
    mphiinst->print(os);

    os << "\n";
  }
};

}  // namespace ir
