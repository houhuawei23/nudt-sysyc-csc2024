#include "visitor/visitor.hpp"
using namespace ir;
namespace sysy {
/*
 * @brief visitBtype (变量类型)
 * @details
 *    btype: INT | FLOAT;
 */
std::any SysYIRGenerator::visitBtype(SysYParser::BtypeContext* ctx) {
  if (ctx->INT()) {
    return Type::TypeInt32();
  } else if (ctx->FLOAT()) {
    return Type::TypeFloat32();
  }
  return nullptr;
}

/*
 * @brief: visitDecl (变量定义 && 声明)
 * @details
 *    Global OR Local (全局 OR 局部)
 */
std::any SysYIRGenerator::visitDecl(SysYParser::DeclContext* ctx) {
  auto btype = any_cast_Type(visit(ctx->btype()));
  bool is_const = ctx->CONST();
  for (auto varDef : ctx->varDef()) {
    visitVarDef(varDef, btype, is_const);
  }

  return std::any();
}

Value* SysYIRGenerator::visitVarDef(SysYParser::VarDefContext* ctx, Type* btype, bool is_const) {
  // 获得数组的各个维度 (常量)
  std::vector<size_t> dims;
  size_t capacity = 1;
  for (auto dimCtx : ctx->lValue()->exp()) {
    auto dimValue = any_cast_Value(visit(dimCtx));

    if (auto instdim = dimValue->dynCast<Instruction>())
      dimValue = instdim->getConstantRepl(true);

    assert(dimValue->isa<ConstantValue>() && "dimension must be a constant");
    const auto dimConstant = dimValue->dynCast<ConstantValue>();
    const auto dimVal = dimConstant->i32();

    capacity *= dimVal;
    dims.push_back(dimVal);
  }
  bool isArray = dims.size() > 0;

  if (mTables.isModuleScope()) {
    if (isArray) {
      return visitGlobalArray(ctx, btype, is_const, dims, capacity);
    } else {
      return visitGlobalScalar(ctx, btype, is_const);
    }
  } else {
    if (isArray) {
      return visitLocalArray(ctx, btype, is_const, dims, capacity);
    } else {
      return visitLocalScalar(ctx, btype, is_const);
    }
  }
}

/*
 * @brief: visit global array
 * @details:
 *    varDef: lValue (ASSIGN initValue)?;
 *    lValue: ID (LBRACKET exp RBRACKET)*;
 *    initValue: exp | LBRACE (initValue (COMMA initValue)*)? RBRACE;
 * @note: global variable
 *    1. const
 *    2. variable
 */
Value* SysYIRGenerator::visitGlobalArray(SysYParser::VarDefContext* ctx,
                                         Type* btype,
                                         const bool is_const,
                                         const std::vector<size_t>& dims,
                                         size_t capacity) {
  const auto name = ctx->lValue()->ID()->getText();
  std::vector<Value*> Arrayinit(capacity, ConstantValue::get(btype, static_cast<intmax_t>(0)));
  bool is_init = false;

  //! get initial value (将数组元素的初始化值存储在Arrayinit中)
  if (ctx->ASSIGN()) {
    _d = 0; _n = 0; _path.clear();
    _path = std::vector<size_t>(dims.size(), 0);
    _current_type = btype;
    _is_alloca = true;
    for (auto expr : ctx->initValue()->initValue()) {
      is_init |= visitInitValue_Array(expr, capacity, dims, Arrayinit);
    }
  }

  //! generate global variable and assign
  auto global_var = GlobalVariable::gen(btype, Arrayinit, mModule, name, is_const, dims, is_init, capacity);
  mTables.insert(name, global_var);
  mModule->addGlobalVar(name, global_var);

  return dyn_cast_Value(global_var);
}
/*
 * @brief: visit global scalar
 * @details:
 *    varDef: lValue (ASSIGN initValue)?;
 *    lValue: ID (LBRACKET exp RBRACKET)*;
 *    initValue: exp | LBRACE (initValue (COMMA initValue)*)? RBRACE;
 * @note: global variable
 *    1. const
 *    2. variable
 */
Value* SysYIRGenerator::visitGlobalScalar(SysYParser::VarDefContext* ctx,
                                          Type* btype,
                                          bool is_const) {
  const auto name = ctx->lValue()->ID()->getText();

  Value* init = nullptr;
  bool is_init = false;
  
  init = ConstantValue::get(btype, static_cast<intmax_t>(0));

  if (ctx->ASSIGN()) {
    is_init = true;
    init = any_cast_Value(visit(ctx->initValue()->exp()));
    if (auto initInst = init->dynCast<Instruction>())
      init = initInst->getConstantRepl(true);
    assert(init->isa<ConstantValue>() && "global must be initialized by constant");
    init = mBuilder.castConstantType(init, btype);
  }

  //! generate global variable and assign
  auto global_var = GlobalVariable::gen(btype, {init}, mModule, name, is_const, {}, is_init);
  mTables.insert(name, global_var);
  mModule->addGlobalVar(name, global_var);

  return dyn_cast_Value(global_var);
}

/*
 * @brief: visitLocalArray
 * @details:
 *    varDef: lValue (ASSIGN initValue)?;
 *    lValue: ID (LBRACKET exp RBRACKET)*;
 *    initValue: exp | LBRACE (initValue (COMMA initValue)*)? RBRACE;
 */
Value* SysYIRGenerator::visitLocalArray(SysYParser::VarDefContext* ctx,
                                        Type* btype,
                                        bool is_const,
                                        std::vector<size_t> dims,
                                        size_t capacity) {
  const auto name = ctx->lValue()->ID()->getText();
  size_t dimensions = dims.size();
  std::vector<size_t> cur_dims(dims);
  std::vector<Value*> Arrayinit;
  bool isAssign = false;

  //! alloca
  const auto arraytype = ArrayType::gen(btype, cur_dims, capacity);
  // auto alloca_ptr = mBuilder.makeInst<AllocaInst>(arraytype, nullptr, name, is_const);
  auto alloca_ptr = mBuilder.makeAlloca(arraytype, is_const, name);
  // std::cerr << "alloca_ptr: " << alloca_ptr->type()->size() << std::endl;
  // std::cerr << "capacity: " << capacity << std::endl;
  // std::cerr << "baseType: " << alloca_ptr->type()->dynCast<PointerType>()->baseType()->size()
  //           << std::endl;
  mTables.insert(name, alloca_ptr);

  //! get initial value (将数组元素的初始化值存储在Arrayinit中)
  if (ctx->ASSIGN()) {
    for (int i = 0; i < capacity; i++) {
      Arrayinit.push_back(nullptr);
    }

    auto ptr = mBuilder.makeInst<UnaryInst>(ir::ValueId::vBITCAST,
                                            PointerType::gen(Type::TypeInt8()), alloca_ptr);
                                            
    const auto len = alloca_ptr->type()->dynCast<PointerType>()->baseType()->size();

    mBuilder.makeInst<MemsetInst>(ptr,
                                  ConstantInteger::get(Type::TypeInt8(), 0),
                                  ConstantInteger::get(Type::TypeInt64(), len),
                                  ConstantInteger::getFalse());

    _d = 0; _n = 0; _path.clear();
    _path = std::vector<size_t>(dims.size(), 0);
    _current_type = btype;
    _is_alloca = true;
    for (auto expr : ctx->initValue()->initValue()) {
      isAssign |= visitInitValue_Array(expr, capacity, dims, Arrayinit);
    }
  }

  //! assign
  if (!isAssign) return dyn_cast_Value(alloca_ptr);
  Value* element_ptr = dyn_cast<Value>(alloca_ptr);
  for (size_t cur = 1; cur <= dimensions; cur++) {
    dims.erase(dims.begin());
    element_ptr =
      mBuilder.makeGetElementPtr(btype, element_ptr, ConstantInteger::gen_i32(0), dims, cur_dims);
    cur_dims.erase(cur_dims.begin());
  }

  size_t cnt = 0;
  for (size_t i = 0; i < Arrayinit.size(); i++) {
    if (Arrayinit[i] != nullptr) {
      element_ptr = mBuilder.makeGetElementPtr(btype, element_ptr, ConstantInteger::gen_i32(cnt));
      mBuilder.makeInst<StoreInst>(Arrayinit[i], element_ptr);
      cnt = 0;
    }
    cnt++;
  }

  return dyn_cast_Value(alloca_ptr);
}

/*
 * @brief: visitLocalScalar
 * @details:
 *    varDef: lValue (ASSIGN initValue)?;
 *    lValue: ID (LBRACKET exp RBRACKET)*;
 *    initValue: exp | LBRACE (initValue (COMMA initValue)*)? RBRACE;
 * @note:
 *    1. const     ignore
 *    2. variable  alloca
 */
Value* SysYIRGenerator::visitLocalScalar(SysYParser::VarDefContext* ctx,
                                         Type* btype, bool is_const) {
  const auto name = ctx->lValue()->ID()->getText();

  if (is_const) {  //! const qulifier
    if (!ctx->ASSIGN())
      assert(false && "const without initialization");
    auto init = any_cast_Value(visit(ctx->initValue()->exp()));
    if(auto inst = init->dynCast<Instruction>()) {
      init = inst->getConstantRepl(true);
    }
    assert(init->isa<ConstantValue>() && "const must be initialized by constant");

    // init is Constant
    init = mBuilder.castConstantType(init, btype);
    mTables.insert(name, init);

    return init;
  } else {  //! not const qulifier
    auto alloca = mBuilder.makeAlloca(btype, is_const, name);
    mTables.insert(name, alloca);

    if (not ctx->ASSIGN())
      return alloca;

    // has init
    auto init = any_cast_Value(visit(ctx->initValue()->exp()));
    if (init->isa<ConstantValue>())
      init = mBuilder.castConstantType(init, btype);
    else
      init = mBuilder.makeTypeCast(init, btype);

    mBuilder.makeInst<StoreInst>(init, alloca);

    return alloca;
  }
}

/*
 * @brief: visitInitValue_Array
 * @details:
 *    varDef: lValue (ASSIGN initValue)?;
 *    initValue: exp | LBRACE (initValue (COMMA initValue)*)? RBRACE;
 */
bool SysYIRGenerator::visitInitValue_Array(SysYParser::InitValueContext* ctx,
                                           const size_t capacity,
                                           const std::vector<size_t> dims,
                                           std::vector<Value*>& init) {
  bool res = false;
  if (ctx->exp()) {
    auto value = any_cast_Value(visit(ctx->exp()));

    //! 类型转换 (匹配左值与右值的数据类型)
    if (value->isa<ConstantValue>()) {
      value = mBuilder.castConstantType(value, _current_type);
    } else {
      value = mBuilder.makeTypeCast(value, _current_type);
    }
    //! 获取当前数组元素的位置
    while (_d < dims.size() - 1) {
      _path[_d++] = _n;
      _n = 0;
    }
    std::vector<Value*> indices;  // 大小为数组维度 (存储当前visit的元素的下标)
    for (size_t i = 0; i < dims.size() - 1; i++) {
      indices.push_back(ConstantInteger::gen_i32(_path[i]));
    }
    indices.push_back(ConstantInteger::gen_i32(_n));

    //! 将特定位置的数组元素存入init数组中
    size_t factor = 1, offset = 0;
    for (int32_t i = indices.size() - 1; i >= 0; i--) { // careful int32_t
      offset += factor * indices[i]->dynCast<ConstantInteger>()->getVal();
      factor *= dims[i];
    }
    if (auto cvalue = value->dynCast<ConstantInteger>()) {  // 1. 常值 (global OR local)
      res = true;
      init[offset] = value;
    } else {  // 2. 变量 (just for local)
      res = true;
      if (_is_alloca) {
        init[offset] = value;
      } else {
        assert(false && "global variable must be initialized by constant");
      }
    }
  } else {
    size_t cur_d = _d, cur_n = _n;
    for (auto expr : ctx->initValue()) {
      res |= visitInitValue_Array(expr, capacity, dims, init);
    }
    _d = cur_d, _n = cur_n;
  }

  // goto next element
  _n++;
  while (_d >= 0 && _n >= dims[_d]) {
    _n = _path[--_d] + 1;
  }
  return res;
}

}  // namespace sysy