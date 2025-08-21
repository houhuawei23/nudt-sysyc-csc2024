#pragma once

#include "ir/infrast.hpp"
#include "ir/module.hpp"
#include "ir/type.hpp"
#include "ir/value.hpp"
#include "support/utils.hpp"
#include "support/arena.hpp"
#include "ir/Attribute.hpp"

namespace ir {

class Loop;

/**
 * @brief Represents a natural loop in the control flow graph
 * 
 * A Loop represents a natural loop structure within a function. It maintains
 * information about the loop's structure including header, exits, latches,
 * and nested sub-loops.
 * 
 * Key components:
 * - Header: The entry point of the loop (dominates all loop blocks)
 * - Exits: Blocks that have successors outside the loop
 * - Latches: Blocks that branch back to the header
 * - Sub-loops: Nested loops contained within this loop
 * 
 * This class is used extensively by loop optimization passes to analyze
 * and transform loop structures.
 * 
 * @see LoopInfo for loop analysis and detection
 */
class Loop {
protected:
  Function* mParent;                           ///< Function containing this loop
  Loop* mParentLoop;                          ///< Parent loop (if nested)
  std::unordered_set<Loop*> mSubLoops;        ///< Direct sub-loops

  std::unordered_set<BasicBlock*> mBlocks;    ///< All blocks in the loop
  BasicBlock* mHeader;                        ///< Loop header block
  std::unordered_set<BasicBlock*> mExits;     ///< Exit blocks
  std::unordered_set<BasicBlock*> mLatchs;    ///< Latch blocks

public:
  /**
   * @brief Constructs a new Loop
   * @param header The header block of the loop
   * @param parent The function containing this loop
   */
  Loop(BasicBlock* header, Function* parent) {
    mHeader = header;
    mParent = parent;
  }
  /// @brief Gets the loop header block
  auto header() const { return mHeader; }
  
  /// @brief Gets the function containing this loop
  auto function() const { return mParent; }
  
  /// @brief Gets the set of all blocks in the loop
  auto& blocks() { return mBlocks; }
  
  /// @brief Gets the set of exit blocks
  auto& exits() { return mExits; }
  
  /// @brief Gets the set of latch blocks
  auto& latchs() { return mLatchs; }
  
  /// @brief Gets the set of direct sub-loops
  auto& subLoops() { return mSubLoops; }

  /// @brief Gets the parent loop (if nested)
  Loop* parentloop() { return mParentLoop; }
  
  /// @brief Sets the parent loop
  void setParent(Loop* lp) { mParentLoop = lp; }
  
  /**
   * @brief Checks if a block is contained in this loop
   * @param block Block to check
   * @return True if the block is in the loop
   */
  bool contains(BasicBlock* block) const { return mBlocks.find(block) != mBlocks.end(); }

  /**
   * @brief Gets the unique predecessor of the loop header from outside the loop
   * @return The loop predecessor block, or nullptr if none or multiple
   */
  BasicBlock* getloopPredecessor() const;

  /**
   * @brief Gets the preheader block of the loop
   * @return The preheader block, or nullptr if it doesn't exist
   */
  BasicBlock* getLoopPreheader() const;

  /**
   * @brief Gets the unique latch block of the loop
   * @return The latch block, or nullptr if none or multiple
   */
  BasicBlock* getLoopLatch() const;

  /**
   * @brief Gets the unique latch block (asserts there is exactly one)
   * @return The unique latch block
   */
  BasicBlock* getUniqueLatch() const {
    assert(mLatchs.size() == 1);
    return *(mLatchs.begin());
  }

  /**
   * @brief Checks if the loop has dedicated exit blocks
   * @return True if all exits are dedicated (no other predecessors from outside)
   */
  bool hasDedicatedExits() const;

  /**
   * @brief Checks if the loop is in simplified form
   * @return True if the loop has preheader, unique latch, and dedicated exits
   */
  bool isLoopSimplifyForm() const {
    return getLoopPreheader() && getLoopLatch() && hasDedicatedExits();
  }
  
  /**
   * @brief Prints the loop structure to output stream
   * @param os Output stream to print to
   */
  void print(std::ostream& os) const;

  /**
   * @brief Sets a single latch block for the loop
   * @param latch The new latch block
   */
  void setLatch(BasicBlock* latch) {
    mLatchs.clear();
    mLatchs.insert(latch);
  }

  /**
   * @brief Gets the first body block of the loop
   * @return The first block in the loop body after the header
   */
  BasicBlock* getFirstBodyBlock() const {
    for (auto block : mHeader->next_blocks()) {
      if (contains(block)) {
        return block;
      }
    }
    assert(false && "no body block found");
    return nullptr;
  }
};

/**
 * @brief Function attribute flags for optimization and analysis
 * 
 * These attributes provide information about function properties that
 * can be used by optimization passes and code generation.
 */
enum FunctionAttribute : uint32_t {
  NoMemoryRead = 1 << 0,         ///< Function does not read memory
  NoMemoryWrite = 1 << 1,        ///< Function does not write memory
  NoSideEffect = 1 << 2,         ///< Function has no observable side effects
  Stateless = 1 << 3,            ///< Function is stateless (pure)
  NoAlias = 1 << 4,              ///< Function parameters don't alias
  NoReturn = 1 << 5,             ///< Function never returns
  NoRecurse = 1 << 6,            ///< Function is not recursive
  Entry = 1 << 7,                ///< Function is an entry point
  Builtin = 1 << 8,              ///< Function is a compiler builtin
  LoopBody = 1 << 9,             ///< Function represents a loop body
  ParallelBody = 1 << 10,        ///< Function can be executed in parallel
  AlignedParallelBody = 1 << 11, ///< Function is aligned for parallel execution
  InlineWrapped = 1 << 12,       ///< Function has been wrapped for inlining
};

/**
 * @brief Represents a function definition in the IR
 * 
 * Function is a User that represents a function definition, containing a list
 * of basic blocks that form the function body, formal arguments, and various
 * function-level metadata.
 * 
 * Key components:
 * - Basic blocks: The control flow graph of the function
 * - Arguments: Formal parameters of the function
 * - Entry/Exit blocks: Special blocks for function entry and exit
 * - Attributes: Function properties for optimization
 * 
 * Functions are organized in Static Single Assignment (SSA) form where each
 * value is defined exactly once and used zero or more times.
 * 
 * @see BasicBlock for the function's control flow structure
 * @see Argument for function parameters
 * @see Module for the container of functions
 */
class Function : public User {
  friend class Module;

protected:
  Module* mModule = nullptr;           ///< Parent module containing this function
  
  block_ptr_list mBlocks;              ///< List of basic blocks in the function
  arg_ptr_vector mArguments;           ///< Formal arguments of the function

  Value* mRetValueAddr = nullptr;      ///< Address for return value storage
  BasicBlock* mEntry = nullptr;        ///< Entry basic block
  BasicBlock* mExit = nullptr;         ///< Exit basic block
  size_t mVarCnt = 0;                  ///< Counter for local variable naming
  size_t argCnt = 0;                   ///< Number of formal arguments
  Attribute<FunctionAttribute> mAttribute; ///< Function attributes

public:
  /**
   * @brief Constructs a new Function
   * @param TypeFunction The function type (must be FunctionType)
   * @param name Optional function name
   * @param parent Optional parent module
   */
  Function(Type* TypeFunction, const_str_ref name = "", Module* parent = nullptr)
    : User(TypeFunction, vFUNCTION, name), mModule(parent) {
    argCnt = 0;
    mRetValueAddr = nullptr;
  }

public:  // Accessor methods
  /**
   * @brief Gets the function attributes
   * @return Reference to the function attributes
   */
  auto& attribute() { return mAttribute; }
  
  /**
   * @brief Gets the parent module
   * @return Pointer to the parent module
   */
  auto module() const { return mModule; }

  /**
   * @brief Gets the return value address
   * @return Pointer to the return value storage location
   */
  auto retValPtr() const { return mRetValueAddr; }
  
  /**
   * @brief Gets the return type of the function
   * @return Pointer to the return type
   */
  auto retType() const { return mType->as<FunctionType>()->retType(); }

  /**
   * @brief Gets the list of basic blocks (const)
   * @return Const reference to the blocks list
   */
  auto& blocks() const { return mBlocks; }
  
  /**
   * @brief Gets the list of basic blocks (mutable)
   * @return Reference to the blocks list
   */
  auto& blocks() { return mBlocks; }

  /**
   * @brief Gets the entry basic block
   * @return Pointer to the entry block
   */
  auto entry() const { return mEntry; }
  
  /**
   * @brief Gets the exit basic block
   * @return Pointer to the exit block
   */
  auto exit() const { return mExit; }

  /**
   * @brief Gets the list of function arguments
   * @return Const reference to the arguments vector
   */
  auto& args() const { return mArguments; }
  
  /**
   * @brief Gets the argument types from the function type
   * @return Const reference to the argument types vector
   */
  auto& argTypes() const { return mType->as<FunctionType>()->argTypes(); }
  
  /**
   * @brief Gets a specific argument by index
   * @param idx Index of the argument
   * @return Pointer to the argument
   */
  auto arg_i(size_t idx) {
    assert(idx < argCnt && "idx out of args vector");
    return mArguments.at(idx);
  }

public:  // Mutator methods
  /**
   * @brief Sets the return value address
   * @param value The return value storage location
   */
  void setRetValueAddr(Value* value) {
    assert(mRetValueAddr == nullptr && "new_ret_value can not call 2th");
    mRetValueAddr = value;
  }
  
  /**
   * @brief Sets the entry basic block
   * @param bb The basic block to use as entry
   */
  void setEntry(ir::BasicBlock* bb) {
    mEntry = bb;
    bb->set_parent(this);
  }
  
  /**
   * @brief Sets the exit basic block
   * @param bb The basic block to use as exit
   */
  void setExit(ir::BasicBlock* bb) {
    mExit = bb;
    bb->set_parent(this);
  }

  /**
   * @brief Creates a new basic block in this function
   * @return Pointer to the newly created basic block
   */
  BasicBlock* newBlock();
  
  /**
   * @brief Creates and sets a new entry basic block
   * @param name Optional name for the entry block
   * @return Pointer to the newly created entry block
   */
  BasicBlock* newEntry(const_str_ref name = "");
  
  /**
   * @brief Creates and sets a new exit basic block
   * @param name Optional name for the exit block
   * @return Pointer to the newly created exit block
   */
  BasicBlock* newExit(const_str_ref name = "");

  /**
   * @brief Removes a basic block from the function
   * @param bb Basic block to remove
   */
  void delBlock(BasicBlock* bb);
  
  /**
   * @brief Forcibly removes a basic block (unsafe)
   * @param bb Basic block to remove
   */
  void forceDelBlock(BasicBlock* bb);

  /**
   * @brief Creates a new function argument
   * @param btype Type of the argument
   * @param name Optional name for the argument
   * @return Pointer to the newly created argument
   */
  auto new_arg(Type* btype, const_str_ref name = "") {
    auto arg = utils::make<Argument>(btype, argCnt, this, name);
    argCnt++;
    mArguments.emplace_back(arg);
    return arg;
  }

  /**
   * @brief Increments and returns the variable counter
   * @return The incremented variable counter
   */
  auto varInc() { return mVarCnt++; }
  
  /**
   * @brief Sets the variable counter
   * @param x New value for the variable counter
   */
  void setVarCnt(size_t x) { mVarCnt = x; }

  /**
   * @brief Checks if this is only a function declaration (no body)
   * @return True if the function has no basic blocks
   */
  bool isOnlyDeclare() const { return mBlocks.empty(); }

  /**
   * @brief Removes an argument by index
   * @param idx Index of the argument to remove
   */
  void delArgumant(size_t idx) {
    assert(idx < argCnt && "idx out of args vector");
    mArguments.erase(mArguments.begin() + idx);
  }

public:  // Utility functions
  /**
   * @brief Runtime type checking for Function class
   * @param v Value to check
   * @return True if the value is a Function
   */
  static bool classof(const Value* v) { return v->valueId() == vFUNCTION; }
  
  /**
   * @brief Creates a copy of this function
   * @return Pointer to the copied function
   */
  ir::Function* copy_func();

  /**
   * @brief Renames all values in the function for readability
   */
  void rename();
  
  /**
   * @brief Dumps this function as an operand (function name)
   * @param os Output stream to dump to
   */
  void dumpAsOpernd(std::ostream& os) const override;
  
  /**
   * @brief Prints the entire function in readable IR format
   * @param os Output stream to print to
   */
  void print(std::ostream& os) const override;

  /**
   * @brief Verifies the integrity of the function
   * @param os Output stream for error messages
   * @return True if the function is valid, false otherwise
   */
  bool verify(std::ostream& os) const;

  /**
   * @brief Updates the function type based on current arguments
   * 
   * Reconstructs the function type from the current argument list.
   * This is useful after argument modifications.
   */
  void updateTypeFromArgs() {
    std::vector<Type*> argTypes;
    for (auto arg : mArguments) {
      argTypes.push_back(arg->type());
    }
    auto newType = FunctionType::gen(retType(), std::move(argTypes));
    mType = newType;
  }
};

}  // namespace ir