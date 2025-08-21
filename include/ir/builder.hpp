#pragma once
#include <any>
#include "ir/infrast.hpp"
#include "ir/instructions.hpp"
#include "support/arena.hpp"
namespace ir {
/**
 * @brief High-level interface for constructing IR
 * 
 * IRBuilder provides a convenient interface for constructing IR instructions
 * and managing the insertion context. It maintains state about the current
 * insertion point and provides helper methods for common IR construction
 * patterns.
 * 
 * Key features:
 * - Manages current insertion point (basic block and position)
 * - Provides stack-based context for nested constructs (loops, conditionals)
 * - Automatic instruction insertion and basic block management
 * - Type promotion and casting utilities
 * - Naming counters for generated constructs
 * 
 * The builder pattern allows for fluent IR construction while maintaining
 * proper SSA form and control flow relationships.
 * 
 * @see BasicBlock for instruction containers
 * @see Instruction for the instructions being built
 */
class IRBuilder {
private:
  BasicBlock* mBlock = nullptr;     ///< Current basic block for instruction insertion
  inst_iterator mInsertPos;         ///< Current insertion position within the block
  block_ptr_stack _headers, _exits; ///< Stack of loop headers and exits
  size_t ifNum, whileNum, rhsNum, funcNum, varNum; ///< Naming counters

  // Stack-based context for short-circuit evaluation
  block_ptr_stack _true_targets, _false_targets; ///< True/false branch targets
  size_t blockNum; ///< Basic block counter

public:
  /**
   * @brief Constructs a new IRBuilder with default state
   */
  IRBuilder() {
    ifNum = 0;
    whileNum = 0;
    rhsNum = 0;
    funcNum = 0;
    varNum = 0;
    blockNum = 0;
  }
  /**
   * @brief Resets the builder to initial state
   */
  void reset() {
    ifNum = 0;
    whileNum = 0;
    rhsNum = 0;
    funcNum = 0;
    varNum = 0;
    blockNum = 0;
    mBlock = nullptr;
    mInsertPos = inst_iterator();
  }

public:  // State query methods
  /**
   * @brief Gets the current insertion basic block
   * @return Pointer to the current basic block
   */
  auto curBlock() const { return mBlock; }
  
  /**
   * @brief Gets the current insertion position
   * @return Iterator pointing to the insertion position
   */
  auto position() const { return mInsertPos; }
  
  /**
   * @brief Gets the current loop header (if any)
   * @return Pointer to the current loop header, or nullptr
   */
  BasicBlock* header() {
    if (!_headers.empty())
      return _headers.top();
    else
      return nullptr;
  }
  
  /**
   * @brief Gets the current loop exit (if any)
   * @return Pointer to the current loop exit, or nullptr
   */
  BasicBlock* exit() {
    if (!_exits.empty())
      return _exits.top();
    else
      return nullptr;
  }

public:  // Position management methods
  /**
   * @brief Sets the insertion position to a specific location
   * @param block The basic block to insert into
   * @param pos Iterator pointing to the insertion position
   */
  void set_pos(BasicBlock* block, inst_iterator pos) {
    assert(block != nullptr);
    mBlock = block;
    mInsertPos = pos;
  }
  
  /**
   * @brief Sets the insertion position to the beginning of a block
   * @param block The basic block to insert into
   */
  void set_pos(BasicBlock* block) {
    mBlock = block;
    mInsertPos = block->insts().begin();
  }
  
  /**
   * @brief Sets the insertion position to the end of a block
   * @param block The basic block to insert into
   */
  void setInsetPosEnd(BasicBlock* block) {
    mBlock = block;
    mInsertPos = block->insts().end();
  }
  
  /**
   * @brief Pushes a loop header onto the context stack
   * @param block The loop header block
   */
  void push_header(BasicBlock* block) { _headers.push(block); }
  
  /**
   * @brief Pushes a loop exit onto the context stack
   * @param block The loop exit block
   */
  void push_exit(BasicBlock* block) { _exits.push(block); }

  /**
   * @brief Pushes both loop header and exit onto the context stack
   * @param header_block The loop header block
   * @param exit_block The loop exit block
   */
  void push_loop(BasicBlock* header_block, BasicBlock* exit_block) {
    push_header(header_block);
    push_exit(exit_block);
  }

  /**
   * @brief Pops the current loop context from the stack
   */
  void pop_loop() {
    _headers.pop();
    _exits.pop();
  }

  /// @brief Increments the if statement counter
  void if_inc() { ifNum++; }
  
  /// @brief Increments the while loop counter
  void while_inc() { whileNum++; }
  
  /// @brief Increments the right-hand side counter
  void rhs_inc() { rhsNum++; }
  
  /// @brief Increments the function counter
  void func_inc() { funcNum++; }

  /// @brief Gets the current if statement count
  auto if_cnt() const { return ifNum; }
  
  /// @brief Gets the current while loop count
  auto while_cnt() const { return whileNum; }
  
  /// @brief Gets the current right-hand side count
  auto rhs_cnt() const { return rhsNum; }
  
  /// @brief Gets the current function count
  auto func_cnt() const { return funcNum; }

  /**
   * @brief Pushes a true branch target for short-circuit evaluation
   * @param block The target block for true conditions
   */
  void push_true_target(BasicBlock* block) { _true_targets.push(block); }
  
  /**
   * @brief Pushes a false branch target for short-circuit evaluation
   * @param block The target block for false conditions
   */
  void push_false_target(BasicBlock* block) { _false_targets.push(block); }
  
  /**
   * @brief Pushes both true and false branch targets
   * @param true_block The target block for true conditions
   * @param false_block The target block for false conditions
   */
  void push_tf(BasicBlock* true_block, BasicBlock* false_block) {
    _true_targets.push(true_block);
    _false_targets.push(false_block);
  }

  /**
   * @brief Gets the current true branch target
   * @return The current true target block
   */
  auto true_target() const { return _true_targets.top(); }
  
  /**
   * @brief Gets the current false branch target
   * @return The current false target block
   */
  auto false_target() const { return _false_targets.top(); }

  /**
   * @brief Pops the current true/false branch targets
   */
  void pop_tf() {
    _true_targets.pop();
    _false_targets.pop();
  }

  /**
   * @brief Casts a value to boolean type
   * @param val The value to cast
   * @return The casted boolean value
   */
  Value* castToBool(Value* val);
  
  /**
   * @brief Promotes a value to a target type (default i32)
   * @param val The value to promote
   * @param target_tpye The target type
   * @param base_type The base type for promotion (default i32)
   * @return The promoted value
   */
  Value* promoteType(Value* val, Type* target_tpye, Type* base_type = Type::TypeInt32());

  /**
   * @brief Alternative type promotion method
   * @param val The value to promote
   * @param target_tpye The target type
   * @return The promoted value
   */
  Value* promoteTypeBeta(Value* val, Type* target_tpye);

  /**
   * @brief Casts a constant to a target type
   * @param val The constant value to cast
   * @param target_tpye The target type
   * @return The casted constant value
   */
  Value* castConstantType(Value* val, Type* target_tpye);

  /**
   * @brief Creates a type cast instruction
   * @param val The value to cast
   * @param target_type The target type
   * @return The cast instruction result
   */
  Value* makeTypeCast(Value* val, Type* target_type);

  /**
   * @brief Creates an alloca instruction
   * @param base_type The type to allocate
   * @param is_const Whether the allocation is constant
   * @param comment Optional comment
   * @return The alloca instruction
   */
  Value* makeAlloca(Type* base_type, bool is_const = false, const_str_ref comment = "");

  /**
   * @brief Creates a comparison instruction
   * @param op The comparison operation
   * @param lhs Left-hand side operand
   * @param rhs Right-hand side operand
   * @return The comparison instruction result
   */
  Value* makeCmp(CmpOp op, Value* lhs, Value* rhs);

  /**
   * @brief Creates a binary arithmetic instruction
   * @param op The binary operation
   * @param lhs Left-hand side operand
   * @param rhs Right-hand side operand
   * @return The binary instruction result
   */
  Value* makeBinary(BinaryOp op, Value* lhs, Value* rhs);

  /**
   * @brief Creates a unary instruction
   * @param vid The value ID for the unary operation
   * @param val The operand value
   * @param ty Optional result type
   * @return The unary instruction result
   */
  Value* makeUnary(ValueId vid, Value* val, Type* ty = nullptr);

  /**
   * @brief Creates a load instruction
   * @param ptr The pointer to load from
   * @return The load instruction result
   */
  Value* makeLoad(Value* ptr);

  /**
   * @brief Creates a getelementptr instruction
   * @param base_type The base type for indexing
   * @param value The base pointer value
   * @param idx The index value
   * @param dims Optional dimension information
   * @param cur_dims Optional current dimension information
   * @return The getelementptr instruction result
   */
  Value* makeGetElementPtr(Type* base_type,
                           Value* value,
                           Value* idx,
                           std::vector<size_t> dims = {},
                           std::vector<size_t> cur_dims = {});

  /**
   * @brief Creates and inserts an instruction of type T
   * @tparam T The instruction type to create
   * @tparam Args Argument types for the constructor
   * @param args Arguments to forward to the constructor
   * @return Pointer to the created instruction
   */
  template <typename T, typename... Args>
  auto makeInst(Args&&... args) {
    auto inst = utils::make<T>(std::forward<Args>(args)...);
    if (mBlock) {
      inst->setBlock(mBlock);
      mBlock->insts().insert(mInsertPos, inst);
    }
    return inst;
  }

  /**
   * @brief Creates an instruction without inserting it
   * @tparam T The instruction type to create
   * @tparam Args Argument types for the constructor
   * @param args Arguments to forward to the constructor
   * @return Pointer to the created instruction
   */
  template <typename T, typename... Args>
  auto makeIdenticalInst(Args&&... args) {
    return utils::make<T>(std::forward<Args>(args)...);
  }
};

}  // namespace ir