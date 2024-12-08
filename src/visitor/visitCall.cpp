#include "visitor/visitor.hpp"
#include "ir/ConstantValue.hpp"
namespace sysy {
/*
 * @brief: visit call
 * @details:
 *      call: ID LPAREN funcRParams? RPAREN;
 *      funcRParams: exp (COMMA exp)*;
 *      var: ID (LBRACKET exp RBRACKET)*;
 *      lValue: ID (LBRACKET exp RBRACKET)*;
 */
std::any SysYIRGenerator::visitCall(SysYParser::CallContext* ctx) {
  const auto lineNumber = ctx->start->getLine();

  auto func_name = ctx->ID()->getText();
  /* macro replace */
  if (func_name.compare("starttime") == 0) {
    func_name = "_sysy_starttime";
  } else if (func_name.compare("stoptime") == 0) {
    func_name = "_sysy_stoptime";
  }
  const auto callee = mModule->findFunction(func_name);
  // function rargs 应该被作为 function 的 operands
  std::vector<ir::Value*> rargs;
  std::vector<ir::Value*> final_rargs;
  
  if (func_name.compare("_sysy_starttime") == 0 or
      func_name.compare("_sysy_stoptime") == 0) {
      rargs.push_back(ir::ConstantInteger::gen_i32(lineNumber));
  } else {
    if (ctx->funcRParams()) {
      for (auto exp : ctx->funcRParams()->exp()) {
        auto rarg = any_cast_Value(visit(exp));
        rargs.push_back(rarg);
      }
    }
  }

  assert(callee->argTypes().size() == rargs.size() && "size not match!");

  int length = rargs.size();
  for (int i = 0; i < length; i++) {
    const auto rarg = rargs[i];
    const auto arg_type = callee->argTypes()[i];
    auto val = mBuilder.makeTypeCast(rargs[i], arg_type);
    final_rargs.push_back(val);
  }
  auto inst = mBuilder.makeInst<ir::CallInst>(callee, final_rargs);
  return dyn_cast_Value(inst);
}
}  // namespace sysy
