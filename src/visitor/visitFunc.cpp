#include "visitor/visitor.hpp"

using namespace std;

using namespace ir;
namespace sysy {
/*
 * @brief: visit function type
 * @details:
 *      funcType: VOID | INT | FLOAT;
 */
std::any SysYIRGenerator::visitFuncType(SysYParser::FuncTypeContext* ctx) {
  if (ctx->INT()) {
    return Type::TypeInt32();
  } else if (ctx->FLOAT()) {
    return Type::TypeFloat32();
  } else if (ctx->VOID()) {
    return Type::void_type();
  }
  std::cerr << "invalid function type: " << ctx->getText() << std::endl;
  assert(false && "invalid return type");
}

/*
 * @brief: create function
 * @details:
 *      funcDef: funcType ID LPAREN funcFParams? RPAREN blockStmt;
 *      funcFParams: funcFParam (COMMA funcFParam)*;
 *      funcFParam: btype ID (LBRACKET RBRACKET (LBRACKET exp RBRACKET)*)?;
 */
Function* SysYIRGenerator::getFunctionSignature(SysYParser::FuncDefContext* ctx) {
  const auto func_name = ctx->ID()->getText();
  if (auto func = mModule->findFunction(func_name))
    return func;

  std::vector<Type*> param_types;

  auto collectParamTypes = [&]() {
    auto params = ctx->funcFParams()->funcFParam();

    for (auto param : params) {
      bool isArray = not param->LBRACKET().empty();

      auto base_type = any_cast_Type(visit(param->btype()));

      if (!isArray) {
        param_types.push_back(base_type);
        continue;
      }

      // array type
      std::vector<size_t> dims;

      for (auto expr : param->exp()) {
        auto value = any_cast_Value(visit(expr));
        size_t dim = 0;
        if (auto cint = value->dynCast<ConstantInteger>()) {
          dim = cint->getVal();
        } else if (auto cfloat = value->dynCast<ConstantFloating>()) {
          dim = static_cast<size_t>(cfloat->getVal());
        }
        assert(dim > 0 && "array dimension must be positive");
        dims.push_back(dim);
      }

      if (dims.size() != 0)
        base_type = Type::TypeArray(base_type, dims);

      param_types.push_back(Type::TypePointer(base_type));
    }
  };

  if (ctx->funcFParams())
    collectParamTypes();

  const auto ret_type = any_cast_Type(visit(ctx->funcType()));
  const auto TypeFunction = Type::TypeFunction(ret_type, param_types);
  auto func = mModule->addFunction(TypeFunction, func_name);

  return func;
}

/*
 * @brief: visit function define
 * @details:
 *      funcDef: funcType ID LPAREN funcFParams? RPAREN blockStmt;
 *      funcFParams: funcFParam (COMMA funcFParam)*;
 *      funcFParam: btype ID (LBRACKET RBRACKET (LBRACKET exp RBRACKET)*)?;
 * entry -> next -> other -> .... -> exit
 * entry: allocas, br
 * next: retval, params, br
 * other: blockStmt init block
 * exit: load retval, ret
 */

std::any SysYIRGenerator::visitFuncDef(SysYParser::FuncDefContext* ctx) {
  const auto funcName = ctx->ID()->getText();
  auto func = getFunctionSignature(ctx);

  if (not ctx->blockStmt())
    return func;

  SymbolTable::FunctionScope scope(mTables);
  func->newEntry()->addComment("entry");
  func->newExit()->addComment("exit");

  const auto next = func->newBlock();
  next->addComment("next");

  func->entry()->emplace_back_inst(mBuilder.makeIdenticalInst<BranchInst>(next));

  const auto retType = func->retType();

  // build next
  mBuilder.set_pos(next);
  // create return value alloca
  if (not retType->isVoid()) {
    auto ret_value_ptr = mBuilder.makeAlloca(retType);
    ret_value_ptr->addComment("retval");
    func->setRetValueAddr(ret_value_ptr);
    switch (retType->btype()) {
      case BasicTypeRank::INT32:
        mBuilder.makeInst<StoreInst>(ConstantInteger::gen_i32(0), ret_value_ptr);
        break;
      case BasicTypeRank::FLOAT:
        mBuilder.makeInst<StoreInst>(ConstantFloating::gen_f32(0.0), ret_value_ptr);
        break;
      default:
        assert(false && "not valid type");
    }
  }
  // func already has argsTypes
  auto allocaParams = [&] {
    auto params = ctx->funcFParams()->funcFParam();
    assert(func->argTypes().size() == params.size());
    for (size_t idx = 0; idx < params.size(); idx++) {
      const auto argCtx = params[idx];
      const auto argName = argCtx->ID()->getText();
      auto argType = func->argTypes()[idx];

      auto arg = func->new_arg(argType);
      auto alloca = mBuilder.makeAlloca(argType);
      alloca->addComment(argName);
      mBuilder.makeInst<StoreInst>(arg, alloca);
      mTables.insert(argName, alloca);
    }
  };

  if (ctx->funcFParams())
    allocaParams();

  const auto other = func->newBlock();
  other->addComment("other");
  next->emplace_back_inst(mBuilder.makeIdenticalInst<BranchInst>(other));
  // build other
  mBuilder.set_pos(other);
  visitBlockStmt(ctx->blockStmt());
  if (not mBuilder.curBlock()->isTerminal()) {
    mBuilder.makeInst<BranchInst>(func->exit());
  }
  // build exit
  mBuilder.set_pos(func->exit());
  if (not retType->isVoid()) {
    mBuilder.makeInst<ReturnInst>(mBuilder.makeLoad(func->retValPtr()));
  } else {
    mBuilder.makeInst<ReturnInst>(nullptr);
  }
  mBuilder.reset();
  return dyn_cast_Value(func);
}


}  // namespace sysy
