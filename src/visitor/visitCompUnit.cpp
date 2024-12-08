#include "visitor/visitor.hpp"

#include <stdbool.h>
using namespace ir;
namespace sysy {
std::any SysYIRGenerator::visitCompUnit(SysYParser::CompUnitContext* ctx) {
  SymbolTable::ModuleScope scope(mTables);
  // add runtime lib functions
  auto type_i32 = Type::TypeInt32();
  auto type_f32 = Type::TypeFloat32();
  auto type_void = Type::void_type();
  auto type_i32p = Type::TypePointer(type_i32);
  auto type_f32p = Type::TypePointer(type_f32);

  //! 外部函数
  mModule->addFunction(Type::TypeFunction(type_i32, {}), "getint");
  mModule->addFunction(Type::TypeFunction(type_i32, {}), "getch");
  mModule->addFunction(Type::TypeFunction(type_f32, {}), "getfloat");

  mModule->addFunction(Type::TypeFunction(type_i32, {type_i32p}), "getarray");
  mModule->addFunction(Type::TypeFunction(type_i32, {type_f32p}), "getfarray");

  mModule->addFunction(Type::TypeFunction(type_void, {type_i32}), "putint");
  mModule->addFunction(Type::TypeFunction(type_void, {type_i32}), "putch");
  mModule->addFunction(Type::TypeFunction(type_void, {type_f32}), "putfloat");

  mModule->addFunction(Type::TypeFunction(type_void, {type_i32, type_i32p}), "putarray");
  mModule->addFunction(Type::TypeFunction(type_void, {type_i32, type_f32p}), "putfarray");

  mModule->addFunction(Type::TypeFunction(type_void, {}), "putf");

  mModule->addFunction(Type::TypeFunction(type_void, {type_i32}), "starttime");
  mModule->addFunction(Type::TypeFunction(type_void, {type_i32}), "stoptime");

  mModule->addFunction(Type::TypeFunction(type_void, {type_i32}), "_sysy_starttime");
  mModule->addFunction(Type::TypeFunction(type_void, {type_i32}), "_sysy_stoptime");

  // memset
  // const auto memsetName = "sysycMemset";
  // const auto memsetType = 

  // visitChildren(ctx);
  for (auto declCtx : ctx->decl()) {
    visit(declCtx);
  }
  for (auto funcDeclCtx : ctx->funcDecl()) {
    mBuilder = IRBuilder();
    visit(funcDeclCtx);
  }
  for (auto funcDefCtx : ctx->funcDef()) {
    mBuilder = IRBuilder();
    visit(funcDefCtx);
  }

  return nullptr;
}
}  // namespace sysy
