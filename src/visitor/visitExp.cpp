#include <any>
#include "visitor/visitor.hpp"
#include "ir/ConstantValue.hpp"
using namespace ir;
namespace sysy {
/*
 * @brief: visitNumberExp
 * @details:
 *    number: ILITERAL | FLITERAL; (即: int or float)
 */
std::any SysYIRGenerator::visitNumberExp(SysYParser::NumberExpContext* ctx) {
  Value* res = nullptr;
  if (auto iLiteral = ctx->number()->ILITERAL()) {
    /* 基数 (8, 10, 16) */
    const auto text = iLiteral->getText();
    int base = 10;
    if (text.find("0x") == 0 || text.find("0X") == 0) {
      base = 16;
    } else if (text.find("0b") == 0 || text.find("0B") == 0) {
      base = 2;
    } else if (text.find("0") == 0) {
      base = 8;
    }
    res = ConstantInteger::gen_i32(std::stol(text, 0, base));
  } else if (auto fLiteral = ctx->number()->FLITERAL()) {
    const auto text = fLiteral->getText();
    res = ConstantFloating::gen_f32(std::stof(text));
  }
  return dyn_cast_Value(res);
}

/*
 * @brief: visitVarExp
 * @details:
 *    var: ID (LBRACKET exp RBRACKET)*;
 */
std::any SysYIRGenerator::visitVarExp(SysYParser::VarExpContext* ctx) {
  const auto varname = ctx->var()->ID()->getText();
  auto ptr = mTables.lookup(varname);
  assert(ptr && "use undefined variable");

  bool isArray = false;
  if (auto ptype = ptr->type()->dynCast<PointerType>()) {
    isArray = ptype->baseType()->isArray() || ptype->baseType()->isPointer();
  }
  if (!isArray) {
    //! 1. scalar
    if (auto gptr = ptr->dynCast<GlobalVariable>()) {  // 全局
      if (gptr->isConst()) {                           // 常量
        ptr = gptr->scalarValue();
      } else {  // 变量
        ptr = mBuilder.makeLoad(ptr);
      }
    } else {  // 局部 (变量 - load, 常量 - ignore)
      if (not ptr->isa<ConstantValue>()) {
        ptr = mBuilder.makeLoad(ptr);
      }
    }
  } else {  //! 2. array
    auto type = ptr->type()->as<PointerType>()->baseType();
    if (type->isArray()) {
      /* 数组 (eg. int a[2][3]) -> 常规使用 */
      auto atype = dyn_cast<ArrayType>(type);
      auto base_type = atype->baseType();
      auto dims = atype->dims(), cur_dims(dims);

      int delta = dims.size() - ctx->var()->exp().size();
      for (auto expr : ctx->var()->exp()) {
        auto idx = any_cast_Value(visit(expr));
        dims.erase(dims.begin());
        ptr = mBuilder.makeGetElementPtr(base_type, ptr, idx, dims, cur_dims);
        cur_dims.erase(cur_dims.begin());
      }
      if (ctx->var()->exp().empty()) {
        dims.erase(dims.begin());
        ptr = mBuilder.makeGetElementPtr(base_type, ptr, ConstantInteger::gen_i32(0), dims, cur_dims);
      } else if (delta > 0) {
        dims.erase(dims.begin());
        ptr = mBuilder.makeGetElementPtr(base_type, ptr, ConstantInteger::gen_i32(0), dims, cur_dims);
      }

      if (delta == 0) {
        ptr = mBuilder.makeLoad(ptr);
      }
    } else if (type->isPointer()) {  // 一级及指针 (eg. int a[] OR int
                                     // a[][5]) -> 函数参数
      ptr = mBuilder.makeLoad(ptr);
      type = dyn_cast<PointerType>(type)->baseType();
      if (type->isArray()) {  // 二级及以上指针 (eg. int a[][5])
                              // Pointer<Array>
        if (ctx->var()->exp().size()) {
          auto expr_vec = ctx->var()->exp();

          auto idx = any_cast_Value(visit(expr_vec[0]));
          ptr = mBuilder.makeGetElementPtr(type, ptr, idx);
          auto base_type = dyn_cast<ArrayType>(type)->baseType();
          auto dims = dyn_cast<ArrayType>(type)->dims(), cur_dims(dims);
          int delta = dims.size() + 1 - expr_vec.size();
          for (int i = 1; i < expr_vec.size(); i++) {
            idx = any_cast_Value(visit(expr_vec[i]));
            dims.erase(dims.begin());
            ptr = mBuilder.makeGetElementPtr(base_type, ptr, idx, dims, cur_dims);
            cur_dims.erase(cur_dims.begin());
          }
          if (delta > 0) {  // 参数传递
            dims.erase(dims.begin());
            ptr = mBuilder.makeGetElementPtr(base_type, ptr, ConstantInteger::gen_i32(0), dims,
                                             cur_dims);
            cur_dims.erase(cur_dims.begin());
          } else if (delta == 0) {
            ptr = mBuilder.makeLoad(ptr);
          } else {
            assert(false && "the array dimensions error");
          }
        }
      } else {  // 一级指针
        for (auto expr : ctx->var()->exp()) {
          auto idx = any_cast_Value(visit(expr));
          ptr = mBuilder.makeGetElementPtr(type, ptr, idx);
        }
        if (ctx->var()->exp().size())
          ptr = mBuilder.makeLoad(ptr);
        // ptr = mBuilder.makeInst<LoadInst>(ptr);
      }
    } else {
      assert(false && "type error");
    }
  }
  return dyn_cast_Value(ptr);
}

/*
 * @brief: visit lvalue
 * @details:
 *      lValue: ID (LBRACKET exp RBRACKET)*
 */
std::any SysYIRGenerator::visitLValue(SysYParser::LValueContext* ctx) {
  //! lvalue must be a pointer
  const auto name = ctx->ID()->getText();
  auto ptr = mTables.lookup(name);
  assert(ptr && "use undefined variable");

  bool isArray = false;
  if (auto ptype = dyn_cast<PointerType>(ptr->type())) {
    isArray = ptype->baseType()->isArray() || ptype->baseType()->isPointer();
  }
  if (!isArray) {  //! 1. scalar
    return dyn_cast_Value(ptr);
  } else {  //! 2. array
    Type* type = dyn_cast<PointerType>(ptr->type())->baseType();
    if (type->isArray()) {  // 数组 (eg. int a[2][3]) -> 常规使用
      auto atype = dyn_cast<ArrayType>(type);
      auto base_type = atype->baseType();
      auto dims = atype->dims(), cur_dims(dims);
      for (auto expr : ctx->exp()) {
        auto idx = any_cast_Value(visit(expr));
        dims.erase(dims.begin());
        ptr = mBuilder.makeGetElementPtr(base_type, ptr, idx, dims, cur_dims);
        cur_dims.erase(cur_dims.begin());
      }
    } else if (type->isPointer()) {  // 指针 (eg. int a[] OR int a[][5]) ->
                                     // 函数参数
      ptr = mBuilder.makeLoad(ptr);
      type = dyn_cast<PointerType>(type)->baseType();

      if (type->isArray()) {
        auto expr_vec = ctx->exp();

        auto idx = any_cast_Value(visit(expr_vec[0]));
        ptr = mBuilder.makeGetElementPtr(type, ptr, idx);
        auto base_type = dyn_cast<ArrayType>(type)->baseType();
        auto dims = dyn_cast<ArrayType>(type)->dims(), cur_dims(dims);

        for (int i = 1; i < expr_vec.size(); i++) {
          idx = any_cast_Value(visit(expr_vec[i]));
          dims.erase(dims.begin());
          ptr = mBuilder.makeGetElementPtr(base_type, ptr, idx, dims, cur_dims);
          cur_dims.erase(cur_dims.begin());
        }
      } else {
        for (auto expr : ctx->exp()) {
          auto idx = any_cast_Value(visit(expr));
          ptr = mBuilder.makeGetElementPtr(type, ptr, idx);
        }
      }
    } else {
      assert(false && "type error");
    }
  }

  return dyn_cast_Value(ptr);
}

/*
 * @brief Visit Unary Expression
 * @details: + - ! exp
 */
std::any SysYIRGenerator::visitUnaryExp(SysYParser::UnaryExpContext* ctx) {
  Value* res = nullptr;
  auto exp = any_cast_Value(visit(ctx->exp()));
  if (ctx->ADD()) {
    res = exp;
  } else if (ctx->NOT()) {
    const auto true_target = mBuilder.false_target();
    const auto false_target = mBuilder.true_target();
    mBuilder.pop_tf();
    mBuilder.push_tf(true_target, false_target);
    res = exp;
  } else if (ctx->SUB()) {
    if (auto cexp = exp->dynCast<ConstantValue>()) {
      switch (cexp->type()->btype()) {
        case BasicTypeRank::INT32:
          res = cexp->dynCast<ConstantInteger>()->getNeg();
          break;
        case BasicTypeRank::FLOAT:
          res = cexp->dynCast<ConstantFloating>()->getNeg();
          break;
        case BasicTypeRank::DOUBLE:
          assert(false && "Unsupport Double");
          break;
        default:
          assert(false && "Unsupport btype");
      }
    } else  {
      switch (exp->type()->btype()) {
        case BasicTypeRank::INT32:
          res = mBuilder.makeBinary(BinaryOp::SUB, ConstantInteger::gen_i32(0), exp);
          break;
        case BasicTypeRank::FLOAT:
          res = mBuilder.makeUnary(ValueId::vFNEG, exp);
          break;
        default:
          assert(false && "Unsupport btype");
      }
    } 
  } else {
    assert(false && "invalid expression");
  }

  return dyn_cast_Value(res);
}

std::any SysYIRGenerator::visitParenExp(SysYParser::ParenExpContext* ctx) {
  return any_cast_Value(visit(ctx->exp()));
}

ir::Value* SysYIRGenerator::visitBinaryExp(ir::BinaryOp op, ir::Value* lhs, ir::Value* rhs) {
  lhs = mBuilder.promoteTypeBeta(lhs, rhs->type());
  rhs = mBuilder.promoteTypeBeta(rhs, lhs->type());
  return mBuilder.makeBinary(op, lhs, rhs);
}

/*
 * @brief:  Visit Multiplicative Expression
 *      exp (MUL | DIV | MODULO) exp
 * @details:
 *      1. mul: 整型乘法
 *      2. fmul: 浮点型乘法
 *
 *      3. udiv: 无符号整型除法 ???
 *      4. sdiv: 有符号整型除法
 *      5. fdiv: 有符号浮点型除法
 *
 *      6. urem: 无符号整型取模 ???
 *      7. srem: 有符号整型取模1
 *      8. frem: 有符号浮点型取模
 */
std::any SysYIRGenerator::visitMultiplicativeExp(SysYParser::MultiplicativeExpContext* ctx) {
  auto lhs = any_cast_Value(visit(ctx->exp(0)));
  auto rhs = any_cast_Value(visit(ctx->exp(1)));
  auto opcode = [ctx] {
    if (ctx->MUL())
      return BinaryOp::MUL;
    if (ctx->DIV())
      return BinaryOp::DIV;
    if (ctx->MODULO())
      return BinaryOp::REM;
    assert(false && "Unknown Binary Operator");
  }();
  auto res = visitBinaryExp(opcode, lhs, rhs);
  return res;
}

/*
 * @brief: Visit Additive Expression
 * @details: exp (ADD | SUB) exp
 */
std::any SysYIRGenerator::visitAdditiveExp(SysYParser::AdditiveExpContext* ctx) {
  auto lhs = any_cast_Value(visit(ctx->exp()[0]));
  auto rhs = any_cast_Value(visit(ctx->exp()[1]));
  auto opcode = [ctx] {
    if (ctx->ADD())
      return BinaryOp::ADD;
    if (ctx->SUB())
      return BinaryOp::SUB;
    assert(false && "Unknown Binary Operator");
  }();
  auto res = visitBinaryExp(opcode, lhs, rhs);
  return res;
}

ir::Value* SysYIRGenerator::visitCmpExp(ir::CmpOp op, ir::Value* lhs, ir::Value* rhs) {
  lhs = mBuilder.promoteTypeBeta(lhs, rhs->type());
  rhs = mBuilder.promoteTypeBeta(rhs, lhs->type());
  if(lhs->type() != rhs->type()) {
    std::cerr << "E: lhs type: " << *lhs->type() << ", rhs type: " << *rhs->type() << std::endl;
  }
  return mBuilder.makeCmp(op, lhs, rhs);
}

//! exp (LT | GT | LE | GE) exp
std::any SysYIRGenerator::visitRelationExp(SysYParser::RelationExpContext* ctx) {
  auto lhs = any_cast_Value(visit(ctx->exp()[0]));
  auto rhs = any_cast_Value(visit(ctx->exp()[1]));
  auto opcode = [ctx] {
    if (ctx->LT())
      return CmpOp::LT;
    if (ctx->GT())
      return CmpOp::GT;
    if (ctx->LE())
      return CmpOp::LE;
    if (ctx->GE())
      return CmpOp::GE;
    assert(false && "Unknown Cmp Operator");
  }();

  auto res = visitCmpExp(opcode, lhs, rhs);

  return res;
}

//! exp (EQ | NE) exp
/**
 * i1  vs i1     -> i32 vs i32       (zext)
 * i1  vs i32    -> i32 vs i32       (zext)
 * i1  vs float  -> float vs float   (zext, sitofp)
 * i32 vs float  -> float vs float   (sitofp)
 */
std::any SysYIRGenerator::visitEqualExp(SysYParser::EqualExpContext* ctx) {
  auto lhs = any_cast_Value(visit(ctx->exp()[0]));
  auto rhs = any_cast_Value(visit(ctx->exp()[1]));
  auto opcode = [ctx] {
    if (ctx->EQ())
      return CmpOp::EQ;
    if (ctx->NE())
      return CmpOp::NE;
    assert(false && "Unknown Cmp Operator");
  }();

  auto res = visitCmpExp(opcode, lhs, rhs);
  return res;
}

/*
 * @brief visit And Expressions
 * @details:
 *      exp: exp AND exp;
 * @note:
 *       - before you visit one exp, you must prepare its true and false
 * target
 *       1. push the thing you protect
 *       2. call the function
 *       3. pop to reuse OR use tmp var to log
 * // exp: lhs AND rhs
 * // know exp's true/false target block
 * // lhs's true target = rhs block
 * // lhs's false target = exp false target
 * // rhs's true target = exp true target
 * // rhs's false target = exp false target
 */
std::any SysYIRGenerator::visitAndExp(SysYParser::AndExpContext* ctx) {
  const auto cur_func = mBuilder.curBlock()->function();

  auto rhs_block = cur_func->newBlock();
  rhs_block->addComment("rhs_block");

  {
    //! 1 visit lhs exp to get its value
    //! diff with OrExp
    mBuilder.push_tf(rhs_block, mBuilder.false_target());
    auto lhs_value = any_cast_Value(visit(ctx->exp(0)));  // recursively visit
    //! may chage by visit, need to re get
    const auto lhs_t_target = mBuilder.true_target();
    const auto lhs_f_target = mBuilder.false_target();
    mBuilder.pop_tf();  // match with push_tf

    lhs_value = mBuilder.castToBool(lhs_value);

    mBuilder.makeInst<BranchInst>(lhs_value, lhs_t_target, lhs_f_target);
  }

  //! 3 visit and generate code for rhs block
  mBuilder.set_pos(rhs_block);

  auto rhs_value = any_cast_Value(visit(ctx->exp(1)));

  return rhs_value;
}

//! exp OR exp
// lhs OR rhs
// know exp's true/false target block (already in builder's stack)
// lhs true target = exp true target
// lhs false target = rhs block
// rhs true target = exp true target
// rhs false target = exp false target
std::any SysYIRGenerator::visitOrExp(SysYParser::OrExpContext* ctx) {
  auto cur_func = mBuilder.curBlock()->function();

  auto rhs_block = cur_func->newBlock();
  rhs_block->addComment("rhs_block");

  {
    //! 1 visit lhs exp to get its value
    mBuilder.push_tf(mBuilder.true_target(), rhs_block);
    auto lhs_value = any_cast_Value(visit(ctx->exp(0)));
    const auto lhs_t_target = mBuilder.true_target();
    const auto lhs_f_target = mBuilder.false_target();
    mBuilder.pop_tf();  // match with push_tf

    lhs_value = mBuilder.castToBool(lhs_value);

    mBuilder.makeInst<BranchInst>(lhs_value, lhs_t_target, lhs_f_target);
  }

  //! 3 visit and generate code for rhs block
  mBuilder.set_pos(rhs_block);

  auto rhs_value = any_cast_Value(visit(ctx->exp(1)));

  return rhs_value;
}

}  // namespace sysy