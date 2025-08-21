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
 * @brief Represents the relationship between a Value and its User
 * 
 * The Use class encapsulates the connection between a value being used and the user
 * that uses it. It maintains the index position in the user's operand list and 
 * provides a bidirectional link between values and their users.
 * 
 * This is a fundamental component of the def-use chain system that enables
 * efficient tracking of value usage throughout the IR.
 * 
 * @see Value::mUses for the list of uses of a value
 * @see User::mOperands for the operands used by a user
 */
class Use {
protected:
  size_t mIndex;
  User* mUser;
  Value* mValue;

public:
  /**
   * @brief Constructs a Use relationship
   * @param index The index of this use in the user's operand list
   * @param user The User that uses the value
   * @param value The Value being used
   */
  Use(size_t index, User* user, Value* value) : mIndex(index), mUser(user), mValue(value) {};

  /**
   * @brief Gets the index of this use in the user's operand list
   * @return The operand index
   */
  size_t index() const;
  
  /**
   * @brief Gets the User that uses this value
   * @return Pointer to the User
   */
  User* user() const;
  
  /**
   * @brief Gets the Value being used
   * @return Pointer to the Value
   */
  Value* value() const;
  
  /**
   * @brief Sets the index of this use
   * @param index New index value
   */
  void set_index(size_t index);
  
  /**
   * @brief Sets the value being used
   * @param value New value to be used
   */
  void set_value(Value* value);
  
  /**
   * @brief Sets the user of this value
   * @param user New user
   */
  void set_user(User* user);

  /**
   * @brief Prints the use relationship to output stream
   * @param os Output stream to print to
   */
  void print(std::ostream& os) const;
};

SYSYC_ARENA_TRAIT(Use, IR)

/**
 * @brief Comparison operation types for ICmp and FCmp instructions
 * 
 * Defines the various comparison operations supported by the compiler.
 * These are used in conditional branches and comparison instructions.
 */
enum CmpOp {
  EQ,  ///< Equal (==)
  NE,  ///< Not equal (!=)
  GT,  ///< Greater than (>)
  GE,  ///< Greater than or equal (>=)
  LT,  ///< Less than (<)
  LE,  ///< Less than or equal (<=)
};

/**
 * @brief Binary arithmetic operation types
 * 
 * Defines the supported binary arithmetic operations for integer and
 * floating-point values.
 */
enum BinaryOp {
  ADD, ///< Addition (+)
  SUB, ///< Subtraction (-)
  MUL, ///< Multiplication (*)
  DIV, ///< Division (/)
  REM, ///< Remainder/Modulo (%)
};

/**
 * @brief Unary operation types
 * 
 * Defines the supported unary operations.
 */
enum UnaryOp {
  NEG, ///< Negation (-)
};

/**
 * @brief Value type identifiers for runtime type identification
 * 
 * This enum provides unique identifiers for each Value subclass, enabling
 * efficient runtime type checking and casting through the classof() pattern.
 * The values are organized by category for easier maintenance.
 */
enum ValueId {
  // Base types
  vValue,        ///< Base Value class
  vFUNCTION,     ///< Function definition
  vCONSTANT,     ///< Constant value
  vARGUMENT,     ///< Function argument
  vBASIC_BLOCK,  ///< Basic block
  vGLOBAL_VAR,   ///< Global variable

  // Memory operations
  vMEMSET,       ///< Memory set operation
  vINSTRUCTION,  ///< Base instruction class
  vALLOCA,       ///< Stack allocation
  vLOAD,         ///< Memory load
  vSTORE,        ///< Memory store
  vGETELEMENTPTR,///< Get element pointer instruction

  // Terminator instructions
  vRETURN,       ///< Return instruction
  vBR,           ///< Branch instruction
  vCALL,         ///< Function call instruction

  // Integer comparison instructions
  vICMP_BEGIN,   ///< Begin marker for integer comparisons
  vIEQ,          ///< Integer equality
  vINE,          ///< Integer inequality
  vISGT,         ///< Integer signed greater than
  vISGE,         ///< Integer signed greater than or equal
  vISLT,         ///< Integer signed less than
  vISLE,         ///< Integer signed less than or equal
  vICMP_END,     ///< End marker for integer comparisons

  // Floating-point comparison instructions
  vFCMP_BEGIN,   ///< Begin marker for floating-point comparisons
  vFOEQ,         ///< Float ordered equality
  vFONE,         ///< Float ordered inequality
  vFOGT,         ///< Float ordered greater than
  vFOGE,         ///< Float ordered greater than or equal
  vFOLT,         ///< Float ordered less than
  vFOLE,         ///< Float ordered less than or equal
  vFCMP_END,     ///< End marker for floating-point comparisons

  // Unary instructions
  vUNARY_BEGIN,  ///< Begin marker for unary instructions
  vFNEG,         ///< Floating-point negation

  // Type conversion instructions
  vTRUNC,        ///< Truncate integer
  vZEXT,         ///< Zero extend integer
  vSEXT,         ///< Sign extend integer
  vFPTRUNC,      ///< Truncate floating-point
  vFPTOSI,       ///< Float to signed integer
  vSITOFP,       ///< Signed integer to float
  vBITCAST,      ///< Bitwise cast
  vPTRTOINT,     ///< Pointer to integer
  vINTTOPTR,     ///< Integer to pointer
  vUNARY_END,    ///< End marker for unary instructions

  // Binary arithmetic instructions
  vBINARY_BEGIN, ///< Begin marker for binary instructions
  vADD,          ///< Integer addition
  vFADD,         ///< Floating-point addition
  vSUB,          ///< Integer subtraction
  vFSUB,         ///< Floating-point subtraction
  vMUL,          ///< Integer multiplication
  vFMUL,         ///< Floating-point multiplication
  vUDIV,         ///< Unsigned integer division
  vSDIV,         ///< Signed integer division
  vFDIV,         ///< Floating-point division
  vUREM,         ///< Unsigned integer remainder
  vSREM,         ///< Signed integer remainder
  vFREM,         ///< Floating-point remainder
  vBINARY_END,   ///< End marker for binary instructions

  // Special instructions
  vPHI,          ///< Phi node for SSA form
  vFUNCPTR,      ///< Function pointer
  vPTRCAST,      ///< Pointer cast
  vATOMICRMW,    ///< Atomic read-modify-write
  vInvalid,      ///< Invalid/uninitialized value
};

/**
 * @brief Base class for all values in the IR
 * 
 * Value is the fundamental base class for all entities that can be used as
 * operands in the IR. It maintains a type, name, and tracks all uses of this
 * value through a def-use chain.
 * 
 * Key features:
 * - Type system: Every value has an associated Type
 * - Naming: Values can have symbolic names for readability
 * - Use tracking: Maintains a list of all uses for efficient def-use analysis
 * - Runtime type identification: Uses ValueId for efficient type checking
 * 
 * The Value class is the root of the IR hierarchy and is inherited by:
 * - Constants (ConstantValue)
 * - Instructions (Instruction via User)
 * - Functions (Function via User) 
 * - Basic blocks (BasicBlock)
 * - Arguments (Argument)
 * - Global variables (GlobalVariable)
 * 
 * @see User for values that use other values as operands
 * @see Type for the type system
 */
class Value {
protected:
  Type* mType;          ///< The type of this value
  ValueId mValueId;     ///< Runtime type identifier
  std::string mName;    ///< Optional symbolic name
  use_ptr_list mUses;   ///< List of all uses of this value
  std::string mComment; ///< Optional comment for debugging

public:
  /// Arena source for memory allocation
  static constexpr auto arenaSource = utils::Arena::Source::IR;

  /**
   * @brief Constructs a new Value
   * @param type The type of this value
   * @param scid The ValueId for runtime type identification
   * @param name Optional symbolic name
   */
  Value(Type* type, ValueId scid = vValue, const_str_ref name = "")
    : mType(type), mValueId(scid), mName(name), mUses() {}
    
  /// Virtual destructor for proper cleanup
  virtual ~Value() = default;
  
  /**
   * @brief Runtime type checking for Value base class
   * @param v Value to check
   * @return Always true since this is the base class
   */
  static bool classof(const Value* v) { return true; }

  /**
   * @brief Gets the type of this value
   * @return Pointer to the Type object
   */
  Type* type() const { return mType; }
  
  /**
   * @brief Gets the name of this value
   * @return The symbolic name as a string
   */
  virtual std::string name() const { return mName; }
  
  /**
   * @brief Sets the name of this value
   * @param name New symbolic name
   */
  void set_name(const_str_ref name) { mName = name; }

  /**
   * @brief Gets the list of all uses of this value
   * @return Reference to the uses list for def-use chain management
   */
  auto& uses() { return mUses; }

  /**
   * @brief Replaces all uses of this value with another value
   * 
   * This is a fundamental operation for IR transformations. It updates
   * all users of this value to use the replacement value instead.
   * 
   * @param mValue The replacement value
   */
  void replaceAllUseWith(Value* mValue);
  
  /**
   * @brief Gets the comment associated with this value
   * @return The comment string
   */
  virtual std::string comment() const { return mComment; }

  /**
   * @brief Sets the comment for this value
   * @param comment The comment string
   * @return This value for method chaining
   */
  Value* setComment(const_str_ref comment);

  /**
   * @brief Adds to the existing comment for this value
   * @param comment Additional comment text
   * @return This value for method chaining
   */
  Value* addComment(const_str_ref comment);

public:  // Type checking convenience methods
  /// @brief Checks if this value has boolean type
  bool isBool() const { return mType->isBool(); }
  
  /// @brief Checks if this value has 32-bit integer type
  bool isInt32() const { return mType->isInt32(); }
  
  /// @brief Checks if this value has 64-bit integer type
  bool isInt64() const { return mType->isInt64(); }
  
  /// @brief Checks if this value has 32-bit floating-point type
  bool isFloat32() const { return mType->isFloat32(); }
  
  /// @brief Checks if this value has 64-bit floating-point type
  bool isDouble() const { return mType->isDouble(); }
  
  /// @brief Checks if this value has any floating-point type
  bool isFloatPoint() const { return mType->isFloatPoint(); }
  
  /// @brief Checks if this value has undefined type
  bool isUndef() const { return mType->isUndef(); }
  
  /// @brief Checks if this value has pointer type
  bool isPointer() const { return mType->isPointer(); }
  
  /// @brief Checks if this value has void type
  bool isVoid() const { return mType->isVoid(); }

public:  // Utility functions
  /**
   * @brief Gets the runtime type identifier
   * @return The ValueId for this value's type
   */
  ValueId valueId() const { return mValueId; }
  
  /**
   * @brief Prints this value to an output stream
   * @param os Output stream to print to
   */
  virtual void print(std::ostream& os) const = 0;
  
  /**
   * @brief Dumps this value as an operand (typically just the name)
   * @param os Output stream to dump to
   */
  virtual void dumpAsOpernd(std::ostream& os) const {
    os << mName;
  }
  
  /**
   * @brief Casts this value to a specific type with assertion
   * @tparam T Target type (must derive from Value)
   * @return Pointer to this value cast as T
   * @throws Assertion failure if cast is invalid
   */
  template <typename T> T* as() {
    static_assert(std::is_base_of_v<Value, T>);
    auto ptr = dynamic_cast<T*>(this);
    assert(ptr);
    return ptr;
  }
  
  /**
   * @brief Checks if this value can be cast to a specific type
   * @tparam T Target type (must derive from Value)
   * @return True if cast is valid, false otherwise
   */
  template <typename T> bool isa() const {
    static_assert(std::is_base_of_v<Value, T>);
    return dynamic_cast<const T*>(this);
  }
  
  /**
   * @brief Dynamically casts this value to a specific type
   * @tparam T Target type (must derive from Value)
   * @return Pointer to this value cast as T, or nullptr if invalid
   */
  template <typename T> T* dynCast() {
    static_assert(std::is_base_of_v<Value, T>);
    return dynamic_cast<T*>(this);
  }
};

/**
 * @brief Base class for Values that use other Values as operands
 * 
 * User represents Values that consume other Values as operands. This is a
 * fundamental concept in the IR where some values (like instructions) use
 * other values to compute their result.
 * 
 * The User class maintains a list of operands and manages the def-use chains
 * by automatically creating Use objects that link operands to this user.
 * 
 * Key responsibilities:
 * - Maintains a vector of operands (as Use objects)
 * - Manages def-use relationships automatically
 * - Provides operand access and manipulation methods
 * 
 * Primary subclasses:
 * - Instruction: All IR instructions that operate on values
 * - GlobalValue: Global variables and functions that may have initializers
 * 
 * @see Use for the operand relationship management
 * @see Instruction for the main user of this class
 */
class User : public Value {
protected:
  use_ptr_vector mOperands;  ///< Vector of operands used by this user

public:
  /**
   * @brief Constructs a new User
   * @param type The type of this user value
   * @param scid The ValueId for runtime type identification
   * @param name Optional symbolic name
   */
  User(Type* type, ValueId scid, const_str_ref name = "") : Value(type, scid, name) {}

public:
  /**
   * @brief Gets the operands vector (mutable)
   * @return Reference to the operands vector
   */
  auto& operands() { return mOperands; }
  
  /**
   * @brief Gets the operands vector (const)
   * @return Const reference to the operands vector
   */
  const auto& operands() const { return mOperands; }
  
  /**
   * @brief Gets a specific operand value by index
   * @param index Index of the operand to retrieve
   * @return Pointer to the operand Value
   */
  Value* operand(size_t index) const;

  /**
   * @brief Adds a new operand to this user
   * 
   * Creates a new Use relationship between this user and the value.
   * The value's use list is automatically updated.
   * 
   * @param value The value to add as an operand
   */
  void addOperand(Value* value);
  
  /**
   * @brief Sets an existing operand to a new value
   * @param index Index of the operand to replace
   * @param value New value for the operand
   */
  void setOperand(size_t index, Value* value);

  /**
   * @brief Adds multiple operands from a container
   * @tparam Container Container type (must be iterable)
   * @param operands Container of values to add as operands
   */
  template <typename Container>
  void addOperands(const Container& operands) {
    for (auto value : operands) {
      addOperand(value);
    }
  }
  
  /**
   * @brief Removes use relationships for all operands
   * 
   * This should be called before deleting this user to properly
   * clean up the def-use chains.
   */
  void unuse_allvalue();

  /**
   * @brief Deletes a specific operand
   * @param index Index of the operand to delete
   */
  void delete_operands(size_t index);

  /**
   * @brief Refreshes the indices of all operands
   * 
   * Should be called after operand vector modifications to ensure
   * Use objects have correct indices.
   */
  void refresh_operand_index();
  
  /**
   * @brief Prints this user to an output stream
   * @param os Output stream to print to
   */
  virtual void print(std::ostream& os) const = 0;
};

}  // namespace ir
