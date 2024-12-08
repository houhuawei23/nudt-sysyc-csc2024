#pragma once
#include "ir/type.hpp"
#include "ir/utils_ir.hpp"
#include "ir/value.hpp"
#include "support/arena.hpp"
#include <variant>
namespace ir {

using ConstantValVariant = std::variant<intmax_t, float>;
using ConstantValueKey = std::pair<Type*, ConstantValVariant>;

struct ConstantValueHash {
  std::size_t operator()(const ConstantValueKey& key) const {
    std::size_t typeHash = std::hash<Type*>{}(key.first);
    std::size_t valHash = std::hash<ConstantValVariant>{}(key.second);
    return typeHash ^ (valHash << 1);
  }
};

struct ConstantValueEqual {
  bool operator()(const ConstantValueKey& lhs, const ConstantValueKey& rhs) const {
    return lhs.first->isSame(rhs.first) && lhs.second == rhs.second;
  }
};

class ConstantValue : public Value {
protected:
  static std::unordered_map<ConstantValueKey, ConstantValue*, ConstantValueHash> mConstantPool;

public:
  ConstantValue(Type* type) : Value(type, ValueId::vCONSTANT) {}
  virtual size_t hash() const = 0;
  static ConstantValue* findCacheByHash(size_t hash);
  virtual ConstantValVariant getValue() const = 0;
public:
  int64_t i64() const;
  int32_t i32() const;
  float f32() const;
  bool i1() const;
public:
  virtual bool isZero();
  virtual bool isOne();

  static ConstantValue* get(Type* type, ConstantValVariant val);
};

class ConstantInteger : public ConstantValue {
  intmax_t mVal;
public:
  ConstantInteger(Type* type, intmax_t val) : ConstantValue(type), mVal(val) {}
  size_t hash() const override;
  intmax_t getVal() const { return mVal; }

  ConstantValVariant getValue() const override { return mVal; }

  static ConstantInteger* get(Type* type, intmax_t val);
  static ConstantInteger* getTrue();
  static ConstantInteger* getFalse();
public:
  static ConstantInteger* gen_i64(intmax_t val);
  static ConstantInteger* gen_i32(intmax_t val);
  static ConstantInteger* gen_i1(bool val);
  ConstantInteger* getNeg() const;
public:
  void print(std::ostream& os) const override;
  void dumpAsOpernd(std::ostream& os) const override;
  bool isZero() override { return mVal == 0; }
  bool isOne() override { return mVal == 1; }
};

class ConstantFloating : public ConstantValue {
  float mVal;
public:
  ConstantFloating(Type* type, float val) : ConstantValue(type), mVal(val) {}
  size_t hash() const override;
  float getVal() const { return mVal; }
  ConstantValVariant getValue() const override { return mVal; }

  static ConstantFloating* get(Type* type, float val);
  static ConstantFloating* gen_f32(float val);
  ConstantFloating* getNeg() const;
public:
  void print(std::ostream& os) const override;
  void dumpAsOpernd(std::ostream& os) const override;
  bool isZero() override { return mVal == 0.0; }
  bool isOne() override { return mVal == 1.0; }
};

class UndefinedValue : public ConstantValue {
public:
  explicit UndefinedValue(Type* type) : ConstantValue(type) { assert(not type->isVoid()); }
public:
  static UndefinedValue* get(Type* type);
public:
  void print(std::ostream& os) const override { os << "undef"; }
  void dumpAsOpernd(std::ostream& os) const override;
  size_t hash() const override { return 0; }
  ConstantValVariant getValue() const override { return static_cast<intmax_t>(0); }
};

}  // namespace ir