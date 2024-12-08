#include "visitor/visitor.hpp"

using namespace ir;

namespace sysy {

/*
 * @brief visit BlockStmt
 * @details:
 *      blockStmt: LBRACE blockItem* RBRACE;
 *      blockItem: decl | stmt;
 *      stmt:
 *            assignStmt
 *          | expStmt
 *          | ifStmt
 *          | whileStmt
 *          | breakStmt
 *          | continueStmt
 *          | returnStmt
 *          | blockStmt
 *          | emptyStmt;
 */
/*
curBlock is sequencially set as curBlock, so if curBlock is Terminal,
means this blockStmt is over, just break.

当前块如果已经Terminal, 后续的 blockItem 不会创建新的块, 可以直接 break.
*/
std::any SysYIRGenerator::visitBlockStmt(SysYParser::BlockStmtContext* ctx) {
  SymbolTable::BlockScope scope(mTables);
  for (auto item : ctx->blockItem()) {
    visit(item);
    if (mBuilder.curBlock()->isTerminal()) {
      break;
    }
  }
  return nullptr;
}

/*
 * @brief Visit ReturnStmt
 *      returnStmt: RETURN exp? SEMICOLON;
 */
std::any SysYIRGenerator::visitReturnStmt(SysYParser::ReturnStmtContext* ctx) {
  auto value = ctx->exp() ? any_cast_Value(visit(ctx->exp())) : nullptr;

  const auto func = mBuilder.curBlock()->function();

  assert(func && "ret stmt block parent err!");

  // 匹配 返回值类型 与 函数定义类型
  if (func->retType()->isVoid()) {
    if (ctx->exp())
      assert(false && "the returned value is not matching the function");

    if (not mBuilder.curBlock()->isTerminal()) {
      auto br = mBuilder.makeInst<BranchInst>(func->exit());
    }
    return std::any();
  }

  assert(ctx->exp() && "the returned value is not matching the function");

  if (value->isa<ConstantValue>()) {  //! 常值
    value = mBuilder.castConstantType(value, func->retType());
  } else {  //! 变量
    value = mBuilder.makeTypeCast(value, func->retType());
  }

  // store res to ret_value
  auto store = mBuilder.makeInst<StoreInst>(value, func->retValPtr());

  if (not mBuilder.curBlock()->isTerminal()) {  // ?
    auto br = mBuilder.makeInst<BranchInst>(func->exit());
  }

  return std::any();
}

/*
 * @brief visit assign stmt
 * @details:
 *      assignStmt: lValue ASSIGN exp SEMICOLON
 */
std::any SysYIRGenerator::visitAssignStmt(SysYParser::AssignStmtContext* ctx) {
  const auto lvalue = any_cast_Value(visit(ctx->lValue()));  // 左值
  auto exp = any_cast_Value(visit(ctx->exp()));              // 右值

  Type* lvalueType = nullptr;
  if (lvalue->type()->isPointer())
    lvalueType = lvalue->type()->as<PointerType>()->baseType();
  else
    lvalueType = lvalue->type()->as<ArrayType>()->baseType();

  exp = mBuilder.makeTypeCast(exp, lvalueType);

  auto res = mBuilder.makeInst<StoreInst>(exp, lvalue);

  return dyn_cast_Value(res);
}

/*
 * @brief: visit If Stmt
 * @details:
 *      ifStmt: IF LPAREN exp RPAREN stmt (ELSE stmt)?;
 */
std::any SysYIRGenerator::visitIfStmt(SysYParser::IfStmtContext* ctx) {
  mBuilder.if_inc();
  const auto cur_block = mBuilder.curBlock();
  const auto cur_func = cur_block->function();

  const auto then_block = cur_func->newBlock();
  const auto else_block = cur_func->newBlock();
  const auto merge_block = cur_func->newBlock();

  // clang-format off
  then_block->addComment("if" + std::to_string(mBuilder.if_cnt()) + "_then");
  else_block->addComment("if" + std::to_string(mBuilder.if_cnt()) +  "_else");
  merge_block->addComment("if" + std::to_string(mBuilder.if_cnt()) + "_merge");
  // clang-format on

  {  //! 1. VISIT cond
    //* push the then and else block to the stack
    mBuilder.push_tf(then_block, else_block);

    //* visit cond, create icmp and br inst
    auto cond = any_cast_Value(visit(ctx->exp()));  // load
    assert(cond != nullptr && "any_cast result nullptr!");
    //* pop to get lhs t/f target
    const auto lhs_t_target = mBuilder.true_target();
    const auto lhs_f_target = mBuilder.false_target();

    mBuilder.pop_tf();

    cond = mBuilder.castToBool(cond);  //* cast to i1
    //* create cond br inst
    if (not mBuilder.curBlock()->isTerminal()) {
      mBuilder.makeInst<BranchInst>(cond, lhs_t_target, lhs_f_target);

      //! [CFG] link the basic block
    }
  }

  {  //! VISIT then block
    mBuilder.set_pos(then_block);
    visit(ctx->stmt(0));  //* may change the basic block
    if (not mBuilder.curBlock()->isTerminal()) {
      mBuilder.makeInst<BranchInst>(merge_block);
    }
  }

  //! VISIT else block
  {
    mBuilder.set_pos(else_block);
    if (auto elsestmt = ctx->stmt(1))
      visit(elsestmt);
    if (not mBuilder.curBlock()->isTerminal()) {
      mBuilder.makeInst<BranchInst>(merge_block);
    }
  }

  //! SETUP builder fo merge block
  mBuilder.set_pos(merge_block);
  return std::any();
}

/*
这里的while的翻译:
while(judge_block){loop_block}
next_block
*/
std::any SysYIRGenerator::visitWhileStmt(SysYParser::WhileStmtContext* ctx) {
  mBuilder.while_inc();
  const auto cur_func = mBuilder.curBlock()->function();
  // create new blocks
  const auto next_block = cur_func->newBlock();
  const auto loop_block = cur_func->newBlock();
  const auto judge_block = cur_func->newBlock();

  //! block comment
  // clang-format off
  next_block->addComment("while" + std::to_string(mBuilder.while_cnt()) + "_next");
  loop_block->addComment("while" + std::to_string(mBuilder.while_cnt()) + "_loop");
  judge_block->addComment("while" + std::to_string(mBuilder.while_cnt()) + "_judge");
  // clang-format on

  // set header and exit block
  mBuilder.push_loop(judge_block, next_block);

  // jump without cond, directly jump to judge block
  mBuilder.makeInst<BranchInst>(judge_block);

  {  // visit cond
    mBuilder.set_pos(judge_block);

    mBuilder.push_tf(loop_block, next_block);
    auto cond = any_cast_Value((visit(ctx->exp())));

    const auto tTarget = mBuilder.true_target();
    const auto fTarget = mBuilder.false_target();
    mBuilder.pop_tf();

    cond = mBuilder.castToBool(cond);

    mBuilder.makeInst<BranchInst>(cond, tTarget, fTarget);
  }

  {  // visit loop block
    mBuilder.set_pos(loop_block);
    visit(ctx->stmt());

    mBuilder.makeInst<BranchInst>(judge_block);

    // pop header and exit block
    mBuilder.pop_loop();
  }

  // visit next block
  mBuilder.set_pos(next_block);

  return std::any();
}

std::any SysYIRGenerator::visitBreakStmt(SysYParser::BreakStmtContext* ctx) {
  const auto breakDest = mBuilder.exit();
  assert(breakDest);

  const auto res = mBuilder.makeInst<BranchInst>(breakDest);

  // create a basic block
  const auto next_block = mBuilder.curBlock()->function()->newBlock();

  mBuilder.set_pos(next_block);
  return std::any();
}

std::any SysYIRGenerator::visitContinueStmt(SysYParser::ContinueStmtContext* ctx) {
  const auto continueDest = mBuilder.header();
  assert(continueDest);

  const auto res = mBuilder.makeInst<BranchInst>(continueDest);

  const auto next_block = mBuilder.curBlock()->function()->newBlock();

  mBuilder.set_pos(next_block);
  return std::any();
}

}  // namespace sysy