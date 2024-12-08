#pragma once
#include "ir/infrast.hpp"
#include "ir/type.hpp"
#include "ir/value.hpp"
#include "support/utils.hpp"
#include "support/arena.hpp"
namespace ir {
/* GlobalVariable */
/*
 * @brief: GlobalVariable Class
 * @note:
 *      1. Array (全局矢量)
 *      2. Scalar (全局标量)
 *      NOTE: GlobalVariable must be PointerType
 */
class GlobalVariable : public User {
protected:
  Module* mModule = nullptr;
  bool mIsArray = false;
  bool mIsConst = false;
  bool mIsInit = true;
  std::vector<Value*> mInitValues;

public:
  GlobalVariable(Type* base_type,
                 const_str_ref name,
                 bool is_const = false,
                 Module* parent = nullptr)
    : User(Type::TypePointer(base_type), vGLOBAL_VAR, name), mModule(parent), mIsConst(is_const) {
    //
    if (base_type->isArray()) {
      mIsArray = true;
    }
    mIsInit = false;
  }

  //! 1. Array
  GlobalVariable(Type* base_type,
                 const std::vector<Value*>& init,
                 const std::vector<size_t>& dims,
                 Module* parent = nullptr,
                 const_str_ref name = "",
                 bool is_const = false,
                 bool is_init = false,
                 size_t capacity = 1)
    : User(Type::TypePointer(Type::TypeArray(base_type, dims, capacity)), vGLOBAL_VAR, name),
      mModule(parent),
      mInitValues(init),
      mIsConst(is_const),
      mIsInit(is_init) {
    mIsArray = true;
  }

  //! 2. Scalar
  GlobalVariable(Type* base_type,
                 const std::vector<Value*> init,
                 Module* parent = nullptr,
                 const_str_ref name = "",
                 bool is_const = false,
                 bool is_init = false)
    : User(ir::Type::TypePointer(base_type), vGLOBAL_VAR, name),
      mModule(parent),
      mInitValues(init),
      mIsConst(is_const),
      mIsInit(is_init) {
    mIsArray = false;
  }

public:  // generate function
  static GlobalVariable* gen(Type* base_type,
                             const std::vector<Value*>& init,
                             Module* parent = nullptr,
                             const_str_ref name = "",
                             bool is_const = false,
                             const std::vector<size_t> dims = {},
                             bool is_init = false,
                             size_t capacity = 1) {
    GlobalVariable* var = nullptr;
    if (dims.size() == 0) {  // 标量
      var = utils::make<GlobalVariable>(base_type, init, parent, name, is_const, is_init);
    } else {  // 矢量
      var = utils::make<GlobalVariable>(base_type, init, dims, parent, name, is_const, is_init,
                                        capacity);
    }
    return var;
  }

public:  // check function
  bool isArray() const { return mIsArray; }
  bool isConst() const { return mIsConst; }
  bool isInit() const { return mIsInit; }

public:  // get function
  auto parent() const { return mModule; }
  auto dims_cnt() const {
    if (isArray()) {
      return dyn_cast<ArrayType>(baseType())->dims_cnt();
    } else {
      return static_cast<size_t>(0);
    }
  }
  auto init_cnt() const { return mInitValues.size(); }
  auto init(size_t index) const { return mInitValues.at(index); }

  Type* baseType() const {
    assert(dyn_cast<PointerType>(type()) && "type error");
    return dyn_cast<PointerType>(type())->baseType();
  }
  auto scalarValue() const {
    assert(mInitValues.size() == 1 && "scalar value error");
    return mInitValues.at(0);
  }

public:
  void print_ArrayInit(std::ostream& os,
                       const size_t dimension,
                       const size_t begin,
                       size_t* idx) const;

public:
  static bool classof(const Value* v) { return v->valueId() == vGLOBAL_VAR; }
  std::string name() const override { return "@" + mName; }
  void dumpAsOpernd(std::ostream& os) const override { os << "@" << mName; }
  void print(std::ostream& os) const override;
};
}  // namespace ir