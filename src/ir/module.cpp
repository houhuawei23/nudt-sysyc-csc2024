#include "ir/module.hpp"
#include <cassert>
#include <iostream>
#include <algorithm>

#include "ir/utils_ir.hpp"
#include "support/arena.hpp"

namespace ir {

void Module::addGlobalVar(const_str_ref name, GlobalVariable* gv) {
  auto iter = mGlobalVariableTable.find(name);  // find the name in _globals
  assert(iter == mGlobalVariableTable.end() && "Redeclare! global variable already exists");
  mGlobalVariableTable.emplace(name, gv);
  mGlobalVariables.emplace_back(gv);
}

Function* Module::findFunction(const_str_ref name) const {
  auto iter = mFuncTable.find(name);
  if (iter != mFuncTable.end()) {
    return iter->second;
  }
  return nullptr;
}
Function* Module::addFunction(Type* type, const_str_ref name) {
  // re-def name
  assert(findFunction(name) == nullptr && "re-add function!");
  auto func = utils::make<Function>(type, name, this);
  mFuncTable.emplace(name, func);
  mFunctions.emplace_back(func);
  return func;
}

void Module::delFunction(ir::Function* func) {
  assert(findFunction(func->name()) != nullptr && "delete unexisted function!");
  mFuncTable.erase(func->name());
  mFunctions.erase(std::find(mFunctions.begin(), mFunctions.end(), func));
  for (auto bbiter = func->blocks().begin(); bbiter != func->blocks().end();) {
    auto bb = *bbiter;
    bbiter++;
    for (auto institer = bb->insts().begin(); institer != bb->insts().end();) {
      auto inst = *institer;
      institer++;
      bb->force_delete_inst(inst);
    }
  }
  for (auto bbiter = func->blocks().begin(); bbiter != func->blocks().end();) {
    auto bb = *bbiter;
    bbiter++;
    func->forceDelBlock(bb);
  }
}

void Module::delGlobalVariable(ir::GlobalVariable* gv) {
  auto pos = std::find(mGlobalVariables.begin(), mGlobalVariables.end(), gv);
  mGlobalVariables.erase(pos);
  mGlobalVariableTable.erase(gv->name());
}

// readable ir print
void Module::print(std::ostream& os) const {
  //! print all global values
  for (auto gv : mGlobalVariables) {
    os << *gv << std::endl;
  }

  // llvm inline function
  os << "declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)\n"
     << std::endl;

  //! print all functions
  for (auto func : mFunctions) {
    if (func->isOnlyDeclare()) {
      func->print(os);
      os << std::endl;
    }
  }
  for (auto func : mFunctions) {
    if (not func->isOnlyDeclare()) {
      func->print(os);
      os << std::endl;
    }
  }
}

void Module::rename() {
  for (auto func : mFunctions) {
    func->rename();
  }
}

bool Module::verify(std::ostream& os) const {
  for (auto func : mFunctions) {
    if (not func->verify(os)) {
      os << "func: " << func->name() << " failed" << std::endl;
      return false;
    }
  }
  return true;
}

GlobalVariable* Module::findGlobalVariable(const_str_ref name) {
  // if (mGlobalVariableTable.count(name) == 0)
  //   return nullptr;
  // return mGlobalVariableTable.at(name);
  return mGlobalVariableTable[name];  // create nullptr if not found
}
}  // namespace ir
