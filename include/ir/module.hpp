#pragma once

#include "ir/function.hpp"
#include "ir/global.hpp"
#include "ir/value.hpp"

#include "support/arena.hpp"

namespace ir {
/**
 * @brief Top-level container for all IR constructs
 * 
 * Module represents a complete compilation unit in the IR. It contains all
 * functions, global variables, and manages memory allocation for IR objects
 * through an arena allocator.
 * 
 * Key responsibilities:
 * - Contains all functions in the compilation unit
 * - Contains all global variables
 * - Manages memory allocation through arena allocator
 * - Provides lookup tables for efficient name-based access
 * - Supports IR verification and serialization
 * 
 * The Module class is the entry point for most IR operations and serves as
 * the root of the IR hierarchy. It owns all IR objects and ensures proper
 * memory management.
 * 
 * @see Function for function definitions
 * @see GlobalVariable for global variable declarations
 */
class Module {
 private:
  utils::Arena mArena;                                          ///< Memory arena for IR objects
  std::vector<Function*> mFunctions;                           ///< All functions in the module
  std::unordered_map<std::string, Function*> mFuncTable;       ///< Function name lookup table

  std::vector<GlobalVariable*> mGlobalVariables;               ///< All global variables
  std::unordered_map<std::string, GlobalVariable*> mGlobalVariableTable; ///< Global variable lookup table

 public:
  /**
   * @brief Constructs a new empty Module
   */
  Module() : mArena{utils::Arena::Source::IR} {};

  /**
   * @brief Gets the list of all functions in the module
   * @return Const reference to the functions vector
   */
  auto& funcs() const { return mFunctions; }
  
  /**
   * @brief Gets the list of all global variables in the module
   * @return Const reference to the global variables vector
   */
  auto& globalVars() const { return mGlobalVariables; }

  /**
   * @brief Gets the main function if it exists
   * @return Pointer to the main function, or nullptr if not found
   */
  Function* mainFunction() const { return findFunction("main"); }

  /**
   * @brief Finds a function by name
   * @param name Name of the function to find
   * @return Pointer to the function, or nullptr if not found
   */
  Function* findFunction(const_str_ref name) const;
  
  /**
   * @brief Adds a new function to the module
   * @param type Function type (must be FunctionType)
   * @param name Name of the function
   * @return Pointer to the newly created function
   */
  Function* addFunction(Type* type, const_str_ref name);
  
  /**
   * @brief Removes a function from the module
   * @param func Function to remove
   */
  void delFunction(ir::Function* func);
  
  /**
   * @brief Removes a global variable from the module
   * @param gv Global variable to remove
   */
  void delGlobalVariable(ir::GlobalVariable* gv);

  /**
   * @brief Adds a global variable to the module
   * @param name Name of the global variable
   * @param gv Global variable to add
   */
  void addGlobalVar(const_str_ref name, GlobalVariable* gv);

  /**
   * @brief Renames all values in the module for readability
   * 
   * Assigns systematic names to all unnamed values in the module,
   * making the IR more readable when printed.
   */
  void rename();
  
  /**
   * @brief Prints the entire module in readable IR format
   * @param os Output stream to print to
   */
  void print(std::ostream& os) const;
  
  /**
   * @brief Verifies the integrity of the module
   * @param os Output stream for error messages
   * @return True if the module is valid, false otherwise
   */
  bool verify(std::ostream& os) const;
  
  /**
   * @brief Finds a global variable by name
   * @param name Name of the global variable to find
   * @return Pointer to the global variable, or nullptr if not found
   */
  GlobalVariable* findGlobalVariable(const_str_ref name);
};

SYSYC_ARENA_TRAIT(Module, IR);
}