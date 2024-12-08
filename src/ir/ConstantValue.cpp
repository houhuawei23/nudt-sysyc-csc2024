#include "ir/ConstantValue.hpp"
#include "support/arena.hpp"

using namespace ir;

std::unordered_map<ConstantValueKey, ConstantValue*, ConstantValueHash>
  ConstantValue::mConstantPool;

int64_t ConstantValue::i64() const {
  const auto val = getValue();
  if (std::holds_alternative<intmax_t>(val)) {
    return static_cast<int64_t>(std::get<intmax_t>(val));
  } else if (std::holds_alternative<float>(val)) {
    return static_cast<int64_t>(std::get<float>(val));
  }
  assert(false);
}
int32_t ConstantValue::i32() const {
  const auto val = getValue();
  if (std::holds_alternative<intmax_t>(val)) {
    return static_cast<int32_t>(std::get<intmax_t>(val));
  } else if (std::holds_alternative<float>(val)) {
    return static_cast<int32_t>(std::get<float>(val));
  }
  assert(false);
}
float ConstantValue::f32() const {
  const auto val = getValue();
  if (std::holds_alternative<float>(val)) {
    return static_cast<float>(std::get<float>(val));
  } else if (std::holds_alternative<intmax_t>(val)) {
    return static_cast<float>(std::get<intmax_t>(val));
  }
  assert(false);
}
bool ConstantValue::i1() const {
  const auto val = getValue();
  if (std::holds_alternative<intmax_t>(val)) {
    return static_cast<bool>(std::get<intmax_t>(val));
  }
  assert(false);
}

bool ConstantValue::isZero() {
  if (auto cint = dynCast<ConstantInteger>()) {
    return cint->isZero();
  } else if (auto cfloat = dynCast<ConstantFloating>()) {
    return cfloat->isZero();
  }
  return false;
}
bool ConstantValue::isOne() {
  if (auto cint = dynCast<ConstantInteger>()) {
    return cint->isOne();
  } else if (auto cfloat = dynCast<ConstantFloating>()) {
    return cfloat->isOne();
  }
  return false;
}

ConstantValue* ConstantValue::get(Type* type, ConstantValVariant val) {
  const auto key = std::make_pair(type, val);
  if (const auto iter = mConstantPool.find(key); iter != mConstantPool.cend()) {
    return iter->second;
  }

  if (type->isInt()) {
    intmax_t intval = 0;
    if (std::holds_alternative<intmax_t>(val)) {
      intval = std::get<intmax_t>(val);
    } else if (std::holds_alternative<float>(val)) {
      intval = std::get<float>(val);
    }
    return ConstantInteger::get(type, intval);
  } else if (type->isFloatPoint()) {
    float floatval = 0.0;
    if (std::holds_alternative<intmax_t>(val)) {
      floatval = std::get<intmax_t>(val);
    } else if (std::holds_alternative<float>(val)) {
      floatval = std::get<float>(val);
    }
    return ConstantFloating::get(type, floatval);
  }
  assert(false && "Unsupported type for constant value");
}

size_t ConstantInteger::hash() const {
  return std::hash<intmax_t>{}(mVal);
}

ConstantInteger* ConstantInteger::gen_i64(intmax_t val) {
  return ConstantInteger::get(Type::TypeInt64(), val);
}
ConstantInteger* ConstantInteger::gen_i32(intmax_t val) {
  return ConstantInteger::get(Type::TypeInt32(), val);
}
ConstantInteger* ConstantInteger::gen_i1(bool val) {
  return ConstantInteger::get(Type::TypeBool(), val);
}

ConstantInteger* ConstantInteger::get(Type* type, intmax_t val) {
  const auto key = std::make_pair(type, val);
  if (const auto iter = mConstantPool.find(key); iter != mConstantPool.cend()) {
    return iter->second->dynCast<ConstantInteger>();
  }
  if (type->isBool()) {
    return (val & 1) ? getTrue() : getFalse();
  }
  const auto cintValue = utils::make<ConstantInteger>(type, val);
  mConstantPool.emplace(key, cintValue);
  return cintValue;
}

ConstantInteger* ConstantInteger::getTrue() {
  static auto i1True = utils::make<ConstantInteger>(Type::TypeBool(), 1);
  return i1True;
}

ConstantInteger* ConstantInteger::getFalse() {
  static auto i1False = utils::make<ConstantInteger>(Type::TypeBool(), 0);
  return i1False;
}

ConstantInteger* ConstantInteger::getNeg() const {
  return ConstantInteger::get(mType, -mVal);
}

void ConstantInteger::print(std::ostream& os) const {
  os << mVal;
}
void ConstantInteger::dumpAsOpernd(std::ostream& os) const {
  os << mVal;
}

ConstantFloating* ConstantFloating::gen_f32(float val) {
  return ConstantFloating::get(Type::TypeFloat32(), val);
}

ConstantFloating* ConstantFloating::get(Type* type, float val) {
  const auto key = std::make_pair(type, val);
  if (const auto iter = mConstantPool.find(key); iter != mConstantPool.cend()) {
    return iter->second->dynCast<ConstantFloating>();
  }
  const auto cfloatValue = utils::make<ConstantFloating>(type, val);
  mConstantPool.emplace(key, cfloatValue);

  return cfloatValue;
}
ConstantFloating* ConstantFloating::getNeg() const {
  return ConstantFloating::get(mType, -mVal);
}

void ConstantFloating::print(std::ostream& os) const {
  os << getMC(mVal);  // implicit conversion to float
}
void ConstantFloating::dumpAsOpernd(std::ostream& os) const {
  os << getMC(mVal);  // implicit conversion to float
}

size_t ConstantFloating::hash() const {
  return std::hash<float>{}(mVal);
}

UndefinedValue* UndefinedValue::get(Type* type) {
  static auto undefined = utils::make<UndefinedValue>(type);
  return undefined;
}
void UndefinedValue::dumpAsOpernd(std::ostream& os) const {
  print(os);
}
