#pragma once
#include <cassert>
#include <forward_list>
#include <unordered_map>
#include "ir/value.hpp"

namespace ir {

/**
 * @brief
 * XXScope new_scope(mTables)
 * .lookup(name)
 * .insert(name, value)
 */

class SymbolTable {
  enum ScopeKind {
    Module,
    Function,
    Block,
  };

 public:
  /* @brief 符号表作用域 (Module, Function, Block) */
  struct ModuleScope {
    SymbolTable& tables_ref;
    ModuleScope(SymbolTable& tables) : tables_ref(tables) {
      tables.enter(Module);
    }
    ~ModuleScope() { tables_ref.exit(); }
  };
  struct FunctionScope {
    SymbolTable& tables_ref;
    FunctionScope(SymbolTable& tables) : tables_ref(tables) {
      tables.enter(Function);
    }
    ~FunctionScope() { tables_ref.exit(); }
  };
  struct BlockScope {
    SymbolTable& tables_ref;
    BlockScope(SymbolTable& tables) : tables_ref(tables) {
      tables.enter(Block);
    }
    ~BlockScope() { tables_ref.exit(); }
  };

 private:
  /*
   * @brief 按照作用域范围建立符号表 (作用域小的符号表 -> 作用域大的符号表,
   * forward_list组织, 每个作用域使用map建立符号表)
   */
  std::forward_list<
      std::pair<ScopeKind, std::unordered_map<std::string, Value*>>>
      symbols;

 public:
  SymbolTable() = default;

 private:
  /* @brief 创造 or 销毁 某一作用域的符号表*/
  void enter(ScopeKind kind) {
    symbols.emplace_front();
    symbols.front().first = kind;
  }
  void exit() { symbols.pop_front(); }

 public:
  /*
   * @brief 判断属于哪部分的作用域
   */
  bool isModuleScope() const { return symbols.front().first == Module; }
  bool isFunctionScope() const { return symbols.front().first == Function; }
  bool isBlockScope() const { return symbols.front().first == Block; }

  /* @brief 查表 (从当前作用域开始查, 直至查到全局作用域范围) */
  Value* lookup(const_str_ref name) const {
    for (auto& scope : symbols) {
      auto iter = scope.second.find(name);
      if (iter != scope.second.end())
        return iter->second;
    }
    return nullptr;
  }

  /*
   * @brief 为当前作用域插入表项
   *   Return: pair<map<string, Value*>::iterator, bool>
   */
  auto insert(const_str_ref name, Value* value) {
    assert(not symbols.empty());
    return symbols.front().second.emplace(name, value);
  }
};
}  // namespace ir