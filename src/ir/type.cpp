#include <assert.h>
#include <variant>
#include <string_view>
using namespace std::string_view_literals;

#include "ir/type.hpp"
#include "support/arena.hpp"
#include "support/StaticReflection.hpp"

namespace ir {
Type* Type::void_type() {
  static Type voidType(BasicTypeRank::VOID);
  return &voidType;
}

Type* Type::TypeBool() {
  static Type i1Type(BasicTypeRank::INT1);
  return &i1Type;
}

// return static Type instance of size_t
Type* Type::TypeInt8() {
  static Type intType(BasicTypeRank::INT8);
  return &intType;
}
Type* Type::TypeInt32() {
  static Type intType(BasicTypeRank::INT32);
  return &intType;
}
Type* Type::TypeInt64() {
  static Type intType(BasicTypeRank::INT64, 8);
  return &intType;
}
Type* Type::TypeFloat32() {
  static Type floatType(BasicTypeRank::FLOAT);
  return &floatType;
}
Type* Type::TypeDouble() {
  static Type doubleType(BasicTypeRank::DOUBLE);
  return &doubleType;
}
Type* Type::TypeLabel() {
  static Type labelType(BasicTypeRank::LABEL);
  return &labelType;
}
Type* Type::TypeUndefine() {
  static Type undefineType(BasicTypeRank::UNDEFINE);
  return &undefineType;
}
Type* Type::TypePointer(Type* baseType) {
  return PointerType::gen(baseType);
}
Type* Type::TypeArray(Type* baseType, std::vector<size_t> dims, size_t capacity) {
  return ArrayType::gen(baseType, dims, capacity);
}
Type* Type::TypeFunction(Type* ret_type, const type_ptr_vector& arg_types) {
  return FunctionType::gen(ret_type, arg_types);
}

//! type check
bool Type::is(Type* type) const {
  return this == type;
}
bool Type::isnot(Type* type) const {
  return this != type;
}

bool Type::isVoid() const {
  return mBtype == BasicTypeRank::VOID;
}

bool Type::isBool() const {
  return mBtype == BasicTypeRank::INT1;
}
bool Type::isInt32() const {
  return mBtype == BasicTypeRank::INT32;
}
bool Type::isInt64() const {
  return mBtype == BasicTypeRank::INT64;
}

bool Type::isFloat32() const {
  return mBtype == BasicTypeRank::FLOAT;
}
bool Type::isDouble() const {
  return mBtype == BasicTypeRank::DOUBLE;
}

bool Type::isUndef() const {
  return mBtype == BasicTypeRank::UNDEFINE;
}

bool Type::isLabel() const {
  return mBtype == BasicTypeRank::LABEL;
}
bool Type::isPointer() const {
  return mBtype == BasicTypeRank::POINTER;
}
bool Type::isFunction() const {
  return mBtype == BasicTypeRank::FUNCTION;
}
bool Type::isArray() const {
  return mBtype == BasicTypeRank::ARRAY;
}

static std::string_view getTypeName(BasicTypeRank btype) {
  switch (btype) {
    case BasicTypeRank::INT1:
      return "i1"sv;
    case BasicTypeRank::INT8:
      return "i8"sv;
    case BasicTypeRank::INT32:
      return "i32"sv;
    case BasicTypeRank::INT64:
      return "i64"sv;
    case BasicTypeRank::FLOAT:
      return "float"sv;
    case BasicTypeRank::DOUBLE:
      return "double"sv;
    case BasicTypeRank::VOID:
      return "void"sv;
    case BasicTypeRank::LABEL:
      return "label"sv;
    case BasicTypeRank::POINTER:
      return "pointer"sv;
    case BasicTypeRank::FUNCTION:
      return "function"sv;
    case BasicTypeRank::ARRAY:
      return "array"sv;
    case BasicTypeRank::UNDEFINE:
      return "undefine"sv;
    default:
      std::cerr << "unknown BasicTypeRank: " << utils::enumName(btype) << std::endl;
      assert(false);
      return "";
  }
}

//! print
void Type::print(std::ostream& os) const {
  os << getTypeName(mBtype);
}

bool Type::isSame(Type* rhs) const {
  // only base type will jump in this function
  // complex type will override this function
  return this == rhs;
}

PointerType* PointerType::gen(Type* base_type) {
  return utils::make<PointerType>(base_type);
}

void PointerType::print(std::ostream& os) const {
  mBaseType->print(os);
  os << "*";
}

bool PointerType::isSame(Type* rhs) const {
  if (rhs == this) return true;
  return rhs->isPointer() && mBaseType->isSame(rhs->as<PointerType>()->baseType());
}

ArrayType* ArrayType::gen(Type* baseType, std::vector<size_t> dims, size_t capacity) {
  return utils::make<ArrayType>(baseType, dims, capacity);
}

void ArrayType::print(std::ostream& os) const {
  for (size_t i = 0; i < mDims.size(); i++) {
    size_t value = mDims.at(i);
    os << "[" << value << " x ";
  }
  mBaseType->print(os);
  for (size_t i = 0; i < mDims.size(); i++)
    os << "]";
}

bool ArrayType::isSame(Type* rhs) const {
  if (rhs == this) return true;
  if (not(rhs->isArray() && mBaseType->isSame(rhs->as<ArrayType>()->baseType()) &&
          mDims.size() == rhs->as<ArrayType>()->dims().size()))
    return false;
  for (size_t idx = 0; idx < mDims.size(); idx++) {
    if (mDims.at(idx) != rhs->as<ArrayType>()->dims().at(idx)) return false;
  }
  return true;
}

FunctionType* FunctionType::gen(Type* ret_type, const type_ptr_vector& arg_types) {
  return utils::make<FunctionType>(ret_type, arg_types);
}
/** void (i32, i32) */
void FunctionType::print(std::ostream& os) const {
  mRetType->print(os);
  os << " (";
  bool isFirst = true;
  for (auto arg_type : mArgTypes) {
    if (isFirst)
      isFirst = false;
    else
      os << ", ";
    arg_type->print(os);
  }
  os << ")*"; // function is also a pointer
}

bool FunctionType::isSame(Type* rhs) const {
  if (rhs == this) return true;
  if (not(rhs->isFunction() && mRetType->isSame(rhs->as<FunctionType>()->retType()) &&
          mArgTypes.size() == rhs->as<FunctionType>()->argTypes().size()))
    return false;
  for (size_t idx = 0; idx < mArgTypes.size(); idx++) {
    if (not mArgTypes.at(idx)->isSame(rhs->as<FunctionType>()->argTypes().at(idx))) return false;
  }
  return true;
}
}  // namespace ir