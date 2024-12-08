#pragma once

#include "ir/function.hpp"
#include "ir/global.hpp"
#include "ir/value.hpp"

#include "support/arena.hpp"

namespace ir {
class Module {
 private:
  utils::Arena mArena;
  std::vector<Function*> mFunctions;
  std::unordered_map<std::string, Function*> mFuncTable;

  std::vector<GlobalVariable*> mGlobalVariables;
  std::unordered_map<std::string, GlobalVariable*> mGlobalVariableTable;

 public:
  Module() : mArena{utils::Arena::Source::IR} {};

  //! get
  auto& funcs() const { return mFunctions; }
  auto& globalVars() const { return mGlobalVariables; }

  Function* mainFunction() const { return findFunction("main"); }

  Function* findFunction(const_str_ref name) const;
  Function* addFunction(Type* type, const_str_ref name);
  void delFunction(ir::Function* func);
  void delGlobalVariable(ir::GlobalVariable*gv);

  void addGlobalVar(const_str_ref name, GlobalVariable* gv);

  void rename();
  // readable ir print
  void print(std::ostream& os) const;
  bool verify(std::ostream& os) const;
  GlobalVariable* findGlobalVariable(const_str_ref name);
};

SYSYC_ARENA_TRAIT(Module, IR);
}