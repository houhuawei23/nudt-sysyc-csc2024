#pragma once
#include <cassert>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>
#include "ir/type.hpp"

#include "support/arena.hpp"
// #include <queue>      // for block list priority queue
// #include <algorithm>  // for block list sort
namespace ir {
class Use;
class User;
class Value;

class ConstantValue;
class Instruction;
class BasicBlock;
class Argument;

class Function;
class Module;

//* string
// use as function formal param type for name
using const_str_ref = const std::string&;

//* Value
using value_ptr_vector = std::vector<Value*>;

// use as function formal param for dims or indices
using const_value_ptr_vector = const std::vector<Value*>;

// symbol table, look up value based on name
using str_value_map = std::map<std::string, Value*>;

//* Use
// Value mUses
using use_ptr_list = std::list<Use*>;
using use_ptr_vector = std::vector<Use*>;

//* BasicBlock
using block_ptr_list = std::list<BasicBlock*>;
using block_ptr_vector = std::vector<BasicBlock*>;
using BasicBlockList = std::list<BasicBlock*>;
// true or false targets stack
using block_ptr_stack = std::stack<BasicBlock*>;

//* Argument
// function args
using arg_ptr_list = std::list<Argument*>;
using arg_ptr_vector = std::vector<Argument*>;

//* Instruction
// basicblock insts
using inst_list = std::list<Instruction*>;
using InstructionList = std::list<Instruction*>;
// iterator for add/del/traverse inst list
using inst_iterator = inst_list::iterator;
using reverse_iterator = inst_list::reverse_iterator;

//* Function
// look up function in function table
using str_fun_map = std::map<std::string, Function*>;

/**
 * @brief 表征操作数本身的信息, 连接 value 和 user
 * index in the mOperands, mUser, mValue
 *
 */
class Use {
protected:
  size_t mIndex;
  User* mUser;
  Value* mValue;

public:
  Use(size_t index, User* user, Value* value) : mIndex(index), mUser(user), mValue(value) {};

  // get
  size_t index() const;
  User* user() const;
  Value* value() const;
  // set
  void set_index(size_t index);
  void set_value(Value* value);
  void set_user(User* user);

  void print(std::ostream& os) const;
};

SYSYC_ARENA_TRAIT(Use, IR)
/**
 * @brief Base Class for all classes having 'value' to be used.?
 * @attention
 * - Value 是除了 Type， Module 之外几乎所有数据结构的基类。
 * - Value 表示一个“值”，它有名字 _name，有类型 _type，可以被使用 mUses。
 * - 派生类继承 Value，添加自己所需的 数据成员 和 方法。
 * - Value 的派生类可以重载 print() 方法，以打印出可读的 IR。
 */
enum CmpOp {
  EQ,  // ==
  NE,  // !=
  GT,  // >
  GE,  // >=
  LT,  // <
  LE,  // <=
};
enum BinaryOp {
  ADD, /* + */
  SUB, /* - */
  MUL, /* * */
  DIV, /* / */
  REM,  /* % */
};
enum UnaryOp {
  NEG,
};

enum ValueId {
  vValue,
  vFUNCTION,
  vCONSTANT,
  vARGUMENT,
  vBASIC_BLOCK,
  vGLOBAL_VAR,

  vMEMSET,
  vINSTRUCTION,
  // vMEM_BEGIN,
  vALLOCA,
  vLOAD,
  vSTORE,
  vGETELEMENTPTR,  // GetElementPtr Instruction
  // vMEM_END,

  // vTERMINATOR_BEGIN
  vRETURN,
  vBR,
  vCALL,
  // vTERMINATOR_END

  // icmp
  vICMP_BEGIN,
  vIEQ,
  vINE,
  vISGT,
  vISGE,
  vISLT,
  vISLE,
  vICMP_END,
  // fcmp
  vFCMP_BEGIN,
  vFOEQ,
  vFONE,
  vFOGT,
  vFOGE,
  vFOLT,
  vFOLE,
  vFCMP_END,
  // Unary Instruction
  vUNARY_BEGIN,
  vFNEG,
  // Conversion Insts
  vTRUNC,
  vZEXT,
  vSEXT,
  vFPTRUNC,
  vFPTOSI,
  vSITOFP,
  vBITCAST,
  vPTRTOINT, 
  vINTTOPTR, 
  vUNARY_END,
  // Binary Instruction
  vBINARY_BEGIN,
  vADD,
  vFADD,
  vSUB,
  vFSUB,

  vMUL,
  vFMUL,

  vUDIV,
  vSDIV,
  vFDIV,

  vUREM,
  vSREM,
  vFREM,
  vBINARY_END,
  // Phi Instruction
  vPHI,
  vFUNCPTR,
  vPTRCAST,
  vATOMICRMW,
  vInvalid,
};
class Value {
protected:
  Type* mType;       // type of the value
  ValueId mValueId;  // subclass id of Value
  std::string mName;
  /* uses list, this value is used by users throw use */
  use_ptr_list mUses;

  std::string mComment;

public:
  static constexpr auto arenaSource = utils::Arena::Source::IR;

  Value(Type* type, ValueId scid = vValue, const_str_ref name = "")
    : mType(type), mValueId(scid), mName(name), mUses() {}
  virtual ~Value() = default;
  // Value is all base, return true
  static bool classof(const Value* v) { return true; }

  // get
  Type* type() const { return mType; }
  virtual std::string name() const { return mName; }
  void set_name(const_str_ref name) { mName = name; }

  /*! manage use-def relation !*/
  auto& uses() { return mUses; }

  /* replace this value with another value,
     for all user use this value */
  void replaceAllUseWith(Value* mValue);
  // manage
  virtual std::string comment() const { return mComment; }

  Value* setComment(const_str_ref comment);

  Value* addComment(const_str_ref comment);

public:  // check function
  bool isBool() const { return mType->isBool(); }
  bool isInt32() const { return mType->isInt32(); }
  bool isInt64() const { return mType->isInt64(); }
  bool isFloat32() const { return mType->isFloat32(); }
  bool isDouble() const { return mType->isDouble(); }
  bool isFloatPoint() const { return mType->isFloatPoint(); }
  bool isUndef() const { return mType->isUndef(); }
  bool isPointer() const { return mType->isPointer(); }
  bool isVoid() const { return mType->isVoid(); }
public:  // utils function
  ValueId valueId() const { return mValueId; }
  virtual void print(std::ostream& os) const = 0;
  virtual void dumpAsOpernd(std::ostream& os) const {
    os << mName;
  }
  template <typename T> T* as() {
    static_assert(std::is_base_of_v<Value, T>);
    auto ptr = dynamic_cast<T*>(this);
    assert(ptr);
    return ptr;
  }
  template <typename T> bool isa() const {
    static_assert(std::is_base_of_v<Value, T>);
    return dynamic_cast<const T*>(this);
  }
  template <typename T> T* dynCast() {
    static_assert(std::is_base_of_v<Value, T>);
    return dynamic_cast<T*>(this);
  }
  // virtual bool verify(std::ostream& os) const = 0;
};

/**
 * @brief 使用“值”，既要使用值，又有返回值，所以继承自 Value
 * @attention mOperands
 * 派生类： Instruction
 *
 * User is the abstract base type of `Value` types which use other `Value` as
 * operands. Currently, there are two kinds of `User`s, `Instruction` and
 * `GlobalValue`.
 *
 */
class User : public Value {
  // mType, mName, mUses
protected:
  use_ptr_vector mOperands;  // 操作数

public:
  User(Type* type, ValueId scid, const_str_ref name = "") : Value(type, scid, name) {}

public:
  // get function

  auto& operands() { return mOperands; }  //! return uses vector
  const auto& operands() const { return mOperands; }
  Value* operand(size_t index) const;  // return value, not use relation

  // manage function
  void addOperand(Value* value);
  void setOperand(size_t index, Value* value);

  template <typename Container>
  void addOperands(const Container& operands) {
    for (auto value : operands) {
      addOperand(value);
    }
  }
  /* del use relation of all operand values,
  ** may do this before delete this user */
  void unuse_allvalue();

  /* delete an operand of a value */
  void delete_operands(size_t index);

  void refresh_operand_index();
  virtual void print(std::ostream& os) const = 0;
};

}  // namespace ir
