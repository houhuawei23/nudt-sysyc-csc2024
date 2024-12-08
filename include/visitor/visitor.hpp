#pragma once
#include <any>
#include <typeinfo>
#include <vector>
#include <sstream>
#include <iomanip>

#include "SysYBaseVisitor.h"
#include "ir/ir.hpp"

#include "support/utils.hpp"
#include "support/Profiler.hpp"
namespace sysy {
class SysYIRGenerator : public SysYBaseVisitor {
private:
  ir::Module* mModule = nullptr;
  ir::IRBuilder mBuilder;
  ir::SymbolTable mTables;
  antlr4::ParserRuleContext* mAstRoot;

  /* for array */
  size_t _d = 0, _n = 0;
  ir::Type* _current_type = nullptr;
  std::vector<size_t> _path;
  bool _is_alloca = false;

public:
  SysYIRGenerator() {};
  SysYIRGenerator(ir::Module* module, antlr4::ParserRuleContext* root)
    : mModule(module), mAstRoot(root) {}

public:
  auto module() const { return mModule; }
  auto& builder() { return mBuilder; }

public:
  ir::Module* buildIR() {
    visit(mAstRoot);
    mModule->rename();
    return mModule;
  }

  //! Override all visit methods
  std::any visitCompUnit(SysYParser::CompUnitContext* ctx) override;

  //! function
  std::any visitFuncType(SysYParser::FuncTypeContext* ctx) override;

  std::any visitFuncDef(SysYParser::FuncDefContext* ctx) override;

  ir::Function* getFunctionSignature(SysYParser::FuncDefContext* ctx);

  std::any visitBlockStmt(SysYParser::BlockStmtContext* ctx) override;

  //! visitDecl
  std::any visitDecl(SysYParser::DeclContext* ctx) override;

  bool visitInitValue_Array(SysYParser::InitValueContext* ctx,
                            const size_t capacity,
                            const std::vector<size_t> dims,
                            std::vector<ir::Value*>& init);
  // 局部变量
  ir::Value* visitLocalArray(SysYParser::VarDefContext* ctx,
                             ir::Type* btype,
                             bool is_const,
                             std::vector<size_t> dims,
                             size_t capacity);
  ir::Value* visitLocalScalar(SysYParser::VarDefContext* ctx, ir::Type* btype, bool is_const);

  ir::Value* visitVarDef(SysYParser::VarDefContext* ctx, ir::Type* btype, bool is_const);
  // 全局变量
  ir::Value* visitGlobalArray(SysYParser::VarDefContext* ctx,
                              ir::Type* btype,
                              const bool is_const,
                              const std::vector<size_t>& dims,
                              size_t capacity);
  ir::Value* visitGlobalScalar(SysYParser::VarDefContext* ctx, ir::Type* btype, bool is_const);

  std::any visitBtype(SysYParser::BtypeContext* ctx) override;

  std::any visitLValue(SysYParser::LValueContext* ctx) override;

  //! visit Stmt
  std::any visitReturnStmt(SysYParser::ReturnStmtContext* ctx) override;

  std::any visitAssignStmt(SysYParser::AssignStmtContext* ctx) override;

  std::any visitIfStmt(SysYParser::IfStmtContext* ctx) override;

  std::any visitWhileStmt(SysYParser::WhileStmtContext* ctx) override;

  std::any visitBreakStmt(SysYParser::BreakStmtContext* ctx) override;

  std::any visitContinueStmt(SysYParser::ContinueStmtContext* ctx) override;

  //! visit EXP
  std::any visitVarExp(SysYParser::VarExpContext* ctx) override;

  std::any visitParenExp(SysYParser::ParenExpContext* ctx) override;

  std::any visitNumberExp(SysYParser::NumberExpContext* ctx) override;

  std::any visitUnaryExp(SysYParser::UnaryExpContext* ctx) override;

  std::any visitMultiplicativeExp(SysYParser::MultiplicativeExpContext* ctx) override;

  std::any visitAdditiveExp(SysYParser::AdditiveExpContext* ctx) override;

  std::any visitRelationExp(SysYParser::RelationExpContext* ctx) override;

  std::any visitEqualExp(SysYParser::EqualExpContext* ctx) override;

  ir::Value* visitBinaryExp(ir::BinaryOp op, ir::Value* lhs, ir::Value* rhs);

  ir::Value* visitCmpExp(ir::CmpOp op, ir::Value* lhs, ir::Value* rhs);

  std::any visitAndExp(SysYParser::AndExpContext* ctx) override;

  std::any visitOrExp(SysYParser::OrExpContext* ctx) override;

  //! call
  std::any visitCall(SysYParser::CallContext* ctx) override;
};
}  // namespace sysy
