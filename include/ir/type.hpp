#pragma once
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cassert>
#include "support/arena.hpp"
namespace ir {
class DataLayout;
class Type;
class PointerType;
class FunctionType;
using type_ptr_vector = std::vector<Type*>;

/* ir base type */
enum class BasicTypeRank : size_t {
  VOID,
  INT1,
  INT8,
  INT32,
  INT64,   // Address
  FLOAT,   // represent f32 in C
  DOUBLE,  // represent f64 in C
  LABEL,   // BasicBlock
  POINTER,
  FUNCTION,
  ARRAY,
  UNDEFINE
};

/* Type */
// NOTE: complex type cant compare by Type*, need to fix
class Type {
protected:
  BasicTypeRank mBtype;
  size_t mSize;

public:
  static constexpr auto arenaSource = utils::Arena::Source::IR;
  Type(BasicTypeRank btype, size_t size = 4) : mBtype(btype), mSize(size) {}
  virtual ~Type() = default;

public:  // static method for construct Type instance
  static Type* void_type();

  static Type* TypeBool();
  static Type* TypeInt8();
  static Type* TypeInt32();
  static Type* TypeInt64();
  static Type* TypeFloat32();
  static Type* TypeDouble();

  static Type* TypeLabel();
  static Type* TypeUndefine();
  static Type* TypePointer(Type* baseType);
  static Type* TypeArray(Type* baseType, std::vector<size_t> dims, size_t capacity = 1);
  static Type* TypeFunction(Type* ret_type, const type_ptr_vector& param_types);

public:  // check function
  bool is(Type* type) const;
  bool isnot(Type* type) const;
  bool isVoid() const;

  bool isBool() const;
  bool isInt32() const;
  bool isInt64() const;
  bool isInt() const { return BasicTypeRank::INT1 <= mBtype and mBtype <= BasicTypeRank::INT64; }

  bool isFloat32() const;
  bool isDouble() const;
  bool isFloatPoint() const {
    return BasicTypeRank::FLOAT <= mBtype and mBtype <= BasicTypeRank::DOUBLE;
  }
  bool isUndef() const;

  bool isLabel() const;
  bool isPointer() const;
  bool isArray() const;
  bool isFunction() const;

public:  // get function
  auto btype() const { return mBtype; }
  auto size() const { return mSize; }

public:  // utils function
  virtual void print(std::ostream& os) const;
  template <typename T>
  T* as() {
    static_assert(std::is_base_of_v<Type, T>);
    auto ptr = dynamic_cast<T*>(this);
    assert(ptr);
    return ptr;
  }
  template <typename T>
  const T* dynCast() const {
    static_assert(std::is_base_of_v<Type, T>);
    return dynamic_cast<const T*>(this);
  }
  virtual bool isSame(Type* rhs) const;
};

SYSYC_ARENA_TRAIT(Type, IR);

/* PointerType */
class PointerType : public Type {
  Type* mBaseType;

public:
  PointerType(Type* baseType) : Type(BasicTypeRank::POINTER, 8), mBaseType(baseType) {}
  static PointerType* gen(Type* baseType);

public:  // get function
  auto baseType() const { return mBaseType; }
  void print(std::ostream& os) const override;
  bool isSame(Type* rhs) const override;
};

/* ArrayType */
class ArrayType : public Type {
protected:
  std::vector<size_t> mDims;  // dimensions
  Type* mBaseType;            // int or float
public:
  // capacity: by words
  ArrayType(Type* baseType, std::vector<size_t> dims, size_t capacity = 1)
    : Type(BasicTypeRank::ARRAY, capacity * 4), mBaseType(baseType), mDims(dims) {}

public:  // generate function
  static ArrayType* gen(Type* baseType, std::vector<size_t> dims, size_t capacity = 1);

public:  // get function
  auto dims_cnt() const { return mDims.size(); }
  auto dim(size_t index) const {
    assert(index < mDims.size());
    return mDims.at(index);
  }
  auto& dims() const { return mDims; }
  auto baseType() const { return mBaseType; }
  void print(std::ostream& os) const override;
  bool isSame(Type* rhs) const override;
};

/* FunctionType */
class FunctionType : public Type {
protected:
  Type* mRetType;
  std::vector<Type*> mArgTypes;

public:
  FunctionType(Type* ret_type, const type_ptr_vector& arg_types = {})
    : Type(BasicTypeRank::FUNCTION, 8), mRetType(ret_type), mArgTypes(arg_types) {}

public:  // generate function
  static FunctionType* gen(Type* ret_type, const type_ptr_vector& arg_types);

public:  // get function
  auto retType() const { return mRetType; }
  auto& argTypes() const { return mArgTypes; }
  void print(std::ostream& os) const override;
  bool isSame(Type* rhs) const override;
};
}  // namespace ir