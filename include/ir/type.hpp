/**
 * @file type.hpp
 * @brief IR Type System Implementation
 * 
 * This header defines the type system for the NUDT SysY compiler's IR.
 * It provides a hierarchy of types including primitive types, composite types,
 * and function types that form the foundation of the type-safe IR representation.
 * 
 * The type system supports:
 * - Primitive types (integers, floats, void, boolean)
 * - Composite types (arrays, pointers)
 * - Function types with parameter and return type information
 * - Type comparison and casting utilities
 * 
 * All types are managed through an arena allocator for efficient memory management.
 */

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

/// Vector of Type pointers for function parameters and other type collections
using type_ptr_vector = std::vector<Type*>;

/**
 * @brief Enumeration of fundamental type categories in the IR
 * 
 * This enumeration defines the basic type categories that form the foundation
 * of the IR type system. Each type has a unique rank that determines its
 * properties and behavior within the compiler.
 */
enum class BasicTypeRank : size_t {
  VOID,      ///< Void type (no value)
  INT1,      ///< Boolean type (1-bit integer)
  INT8,      ///< 8-bit signed integer
  INT32,     ///< 32-bit signed integer (default int)
  INT64,     ///< 64-bit signed integer (address size)
  FLOAT,     ///< 32-bit IEEE 754 floating point (f32)
  DOUBLE,    ///< 64-bit IEEE 754 floating point (f64)
  LABEL,     ///< BasicBlock label type
  POINTER,   ///< Pointer to another type
  FUNCTION,  ///< Function type
  ARRAY,     ///< Array type
  UNDEFINE   ///< Undefined/unknown type
};

/**
 * @brief Base class for all types in the IR type system
 * 
 * Type represents the base class for all types in the IR. It provides common
 * functionality for type identification, size calculation, and type comparison.
 * 
 * All types are immutable once created and are managed through an arena allocator
 * for efficient memory management. Complex types (arrays, functions) should use
 * structural comparison rather than pointer equality.
 * 
 * Key features:
 * - Immutable type objects
 * - Arena-based memory management
 * - Virtual dispatch for type-specific operations
 * - Type hierarchy with runtime type identification
 * 
 * @note Complex types cannot be compared by pointer equality - use isSame() instead
 */
class Type {
protected:
  BasicTypeRank mBtype;  ///< The fundamental type category
  size_t mSize;          ///< Size of the type in bytes

public:
  /// Arena source for type allocation
  static constexpr auto arenaSource = utils::Arena::Source::IR;
  
  /**
   * @brief Constructs a new Type
   * @param btype The basic type rank/category
   * @param size Size of the type in bytes (default 4)
   */
  Type(BasicTypeRank btype, size_t size = 4) : mBtype(btype), mSize(size) {}
  
  /// Virtual destructor for proper cleanup
  virtual ~Type() = default;

public:  // Static factory methods for type construction
  /// @brief Creates the void type (represents no value)
  static Type* void_type();

  /// @brief Creates a boolean type (1-bit integer)
  static Type* TypeBool();
  
  /// @brief Creates an 8-bit signed integer type
  static Type* TypeInt8();
  
  /// @brief Creates a 32-bit signed integer type (default integer)
  static Type* TypeInt32();
  
  /// @brief Creates a 64-bit signed integer type
  static Type* TypeInt64();
  
  /// @brief Creates a 32-bit IEEE 754 floating point type
  static Type* TypeFloat32();
  
  /// @brief Creates a 64-bit IEEE 754 floating point type
  static Type* TypeDouble();

  /// @brief Creates a basic block label type
  static Type* TypeLabel();
  
  /// @brief Creates an undefined/unknown type
  static Type* TypeUndefine();
  
  /**
   * @brief Creates a pointer type
   * @param baseType The type being pointed to
   * @return Pointer type pointing to baseType
   */
  static Type* TypePointer(Type* baseType);
  
  /**
   * @brief Creates an array type
   * @param baseType The element type of the array
   * @param dims Dimensions of the array (supports multi-dimensional arrays)
   * @param capacity Total capacity in words (default 1)
   * @return Array type with specified dimensions and element type
   */
  static Type* TypeArray(Type* baseType, std::vector<size_t> dims, size_t capacity = 1);
  
  /**
   * @brief Creates a function type
   * @param ret_type Return type of the function
   * @param param_types Parameter types of the function
   * @return Function type with specified signature
   */
  static Type* TypeFunction(Type* ret_type, const type_ptr_vector& param_types);

public:  // Type checking methods
  /// @brief Checks if this type is the same as another type
  bool is(Type* type) const;
  
  /// @brief Checks if this type is different from another type
  bool isnot(Type* type) const;
  
  /// @brief Checks if this is the void type
  bool isVoid() const;

  /// @brief Checks if this is a boolean type
  bool isBool() const;
  
  /// @brief Checks if this is a 32-bit integer type
  bool isInt32() const;
  
  /// @brief Checks if this is a 64-bit integer type
  bool isInt64() const;
  
  /// @brief Checks if this is any integer type
  bool isInt() const { return BasicTypeRank::INT1 <= mBtype and mBtype <= BasicTypeRank::INT64; }

  /// @brief Checks if this is a 32-bit floating point type
  bool isFloat32() const;
  
  /// @brief Checks if this is a 64-bit floating point type
  bool isDouble() const;
  
  /// @brief Checks if this is any floating point type
  bool isFloatPoint() const {
    return BasicTypeRank::FLOAT <= mBtype and mBtype <= BasicTypeRank::DOUBLE;
  }
  
  /// @brief Checks if this is an undefined type
  bool isUndef() const;

  /// @brief Checks if this is a basic block label type
  bool isLabel() const;
  
  /// @brief Checks if this is a pointer type
  bool isPointer() const;
  
  /// @brief Checks if this is an array type
  bool isArray() const;
  
  /// @brief Checks if this is a function type
  bool isFunction() const;

public:  // Accessor methods
  /// @brief Gets the basic type rank/category
  auto btype() const { return mBtype; }
  
  /// @brief Gets the size of this type in bytes
  auto size() const { return mSize; }

public:  // Utility methods
  /// @brief Prints this type to an output stream
  virtual void print(std::ostream& os) const;
  
  /**
   * @brief Safely casts this type to a derived type
   * @tparam T The target type (must be derived from Type)
   * @return Pointer to the casted type
   * @throws Assertion failure if the cast is invalid
   */
  template <typename T>
  T* as() {
    static_assert(std::is_base_of_v<Type, T>);
    auto ptr = dynamic_cast<T*>(this);
    assert(ptr);
    return ptr;
  }
  
  /**
   * @brief Dynamically casts this type to a derived type
   * @tparam T The target type (must be derived from Type)
   * @return Pointer to the casted type, or nullptr if invalid
   */
  template <typename T>
  const T* dynCast() const {
    static_assert(std::is_base_of_v<Type, T>);
    return dynamic_cast<const T*>(this);
  }
  
  /**
   * @brief Checks if this type is structurally equivalent to another
   * @param rhs The type to compare against
   * @return True if the types are structurally equivalent
   * @note This performs deep structural comparison for complex types
   */
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