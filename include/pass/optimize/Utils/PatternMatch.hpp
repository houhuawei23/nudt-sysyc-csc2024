#pragma once
#include "ir/ir.hpp"
#include <cstdint>
#include <type_traits>
#include <cassert>
using namespace ir;
namespace pass {

template <typename ValueType>
struct MatchContext final {
  ValueType* value;

  explicit MatchContext(ValueType* val) : value{val} {};

  template <typename T = ValueType>
  MatchContext<Value> getOperand(uint32_t idx) const {
    return MatchContext<Value>{value->operand(idx)};
  }
};

template <typename T, typename Derived>
class GenericMatcher {
  public:
  bool operator()(const MatchContext<Value>& ctx) const noexcept {
    if (auto val = dynamic_cast<T*>(ctx.value)) {
      return (static_cast<const Derived*>(this))->handle(MatchContext<T>{val});
    }
    return false;
  }
};

class AnyMatcher {
  Value*& mValue;

  public:
  explicit AnyMatcher(Value*& value) noexcept : mValue{value} {}
  bool operator()(const MatchContext<Value>& ctx) const noexcept {
    mValue = ctx.value;
    return true;
  }
};

inline auto any(Value*& val) {
  return AnyMatcher{val};
}

class BooleanMatcher {
  Value*& mValue;

  public:
  explicit BooleanMatcher(Value*& value) noexcept : mValue{value} {}
  bool operator()(const MatchContext<Value>& ctx) const noexcept {
    mValue = ctx.value;
    return mValue->type()->isBool();
  }
};

inline auto boolean(Value*& val) {
  return BooleanMatcher{val};
}

class ConstantIntMatcher {
  int64_t& mVal;

  public:
  ConstantIntMatcher(int64_t& value) : mVal{value} {}
  bool operator()(const MatchContext<Value>& ctx) const {
    if (auto value = ctx.value->dynCast<ConstantValue>()) {
      if (value->type()->isInt32()) {
        mVal = value->i32();
        return true;
      }
    }
    return false;
  }
};

class ConstantIntValMatcher {
  int64_t mVal;

  public:
  ConstantIntValMatcher(int64_t value) : mVal{value} {}
  bool operator()(const MatchContext<Value>& ctx) const {
    if (auto value = ctx.value->dynCast<ConstantValue>()) {
      if (value->type()->isInt32()) {
        return mVal == value->i32();
      }
    }
    return false;
  }
};

inline auto int_(int64_t& val) noexcept {
  return ConstantIntMatcher{val};
}

inline auto intval_(int64_t val) {
  return ConstantIntValMatcher{val};
}

class ConstantFloatMatcher {
  float& mVal;

  public:
  ConstantFloatMatcher(float& value) : mVal{value} {}
  bool operator()(const MatchContext<Value>& ctx) const {
    if (auto value = ctx.value->dynCast<ConstantValue>()) {
      if (value->type()->isFloat32()) {
        mVal = value->f32();
        return true;
      }
    }
    return false;
  }
};

class ConstantFloatValMatcher {
  float mVal;

  public:
  ConstantFloatValMatcher(float value) : mVal{value} {}
  bool operator()(const MatchContext<Value>& ctx) const {
    if (auto value = ctx.value->dynCast<ConstantValue>()) {
      if (value->type()->isFloat32()) {
        return mVal == value->f32();
      }
    }
    return false;
  }
};

inline auto float_(float& val) noexcept {
  return ConstantFloatMatcher{val};
}

inline auto floatval_(float val) {
  return ConstantFloatValMatcher{val};
}

template <bool IsCommutative, typename LhsMatcher, typename RhsMatcher>
class BinaryInstMatcher final
    : public GenericMatcher<
          BinaryInst,
          BinaryInstMatcher<IsCommutative, LhsMatcher, RhsMatcher>> {
  //
  ValueId mValueId;
  LhsMatcher mLhsMatcher;
  RhsMatcher mRhsMatcher;

  public:
  BinaryInstMatcher(ValueId valueId,
                    LhsMatcher lhsMatcher,
                    RhsMatcher rhsMatcher)
      : mValueId{valueId}, mLhsMatcher{lhsMatcher}, mRhsMatcher{rhsMatcher} {}

  bool handle(const MatchContext<BinaryInst>& ctx) const {
    if (mValueId != ValueId::vInvalid and mValueId != ctx.value->valueId()) {
      return false;
    }
    if (mLhsMatcher(ctx.getOperand(0)) and mRhsMatcher(ctx.getOperand(1))) {
      return true;
    }
    if constexpr (IsCommutative) {
      return mLhsMatcher(ctx.getOperand(1)) and mRhsMatcher(ctx.getOperand(0));
    }
    return false;
  }
};
template <typename LhsMatcher, typename RhsMatcher>
auto add(LhsMatcher lhsMatcher, RhsMatcher rhsMatcher) {
  return BinaryInstMatcher<true, LhsMatcher, RhsMatcher>{
      ValueId::vADD, lhsMatcher, rhsMatcher};
}

template <typename LhsMatcher, typename RhsMatcher>
auto sub(LhsMatcher lhsMatcher, RhsMatcher rhsMatcher) {
  return BinaryInstMatcher<false, LhsMatcher, RhsMatcher>{
      ValueId::vSUB, lhsMatcher, rhsMatcher};
}

template <typename LhsMatcher, typename RhsMatcher>
auto mul(LhsMatcher lhsMatcher, RhsMatcher rhsMatcher) {
  return BinaryInstMatcher<true, LhsMatcher, RhsMatcher>{
      ValueId::vMUL, lhsMatcher, rhsMatcher};
}

template <typename LhsMatcher, typename RhsMatcher>
auto sdiv(LhsMatcher lhsMatcher, RhsMatcher rhsMatcher) {
  return BinaryInstMatcher<false, LhsMatcher, RhsMatcher>{
      ValueId::vSDIV, lhsMatcher, rhsMatcher};
}

template <typename LhsMatcher, typename RhsMatcher>
auto srem(LhsMatcher lhsMatcher, RhsMatcher rhsMatcher) {
  return BinaryInstMatcher<false, LhsMatcher, RhsMatcher>{
      ValueId::vSREM, lhsMatcher, rhsMatcher};
}

template <typename LhsMatcher, typename RhsMatcher>
auto fadd(LhsMatcher lhsMatcher, RhsMatcher rhsMatcher) {
  return BinaryInstMatcher<true, LhsMatcher, RhsMatcher>{
      ValueId::vFADD, lhsMatcher, rhsMatcher};
}

template <typename LhsMatcher, typename RhsMatcher>
auto fsub(LhsMatcher lhsMatcher, RhsMatcher rhsMatcher) {
  return BinaryInstMatcher<false, LhsMatcher, RhsMatcher>{
      ValueId::vFSUB, lhsMatcher, rhsMatcher};
}

template <typename LhsMatcher, typename RhsMatcher>
auto fmul(LhsMatcher lhsMatcher, RhsMatcher rhsMatcher) {
  return BinaryInstMatcher<true, LhsMatcher, RhsMatcher>{
      ValueId::vFMUL, lhsMatcher, rhsMatcher};
}
template <typename LhsMatcher, typename RhsMatcher>
auto fdiv(LhsMatcher lhsMatcher, RhsMatcher rhsMatcher) {
  return BinaryInstMatcher<false, LhsMatcher, RhsMatcher>{
      ValueId::vFDIV, lhsMatcher, rhsMatcher};
}

// class CastMatcher

template <typename LhsMatcher, typename RhsMatcher>
class ICmpInstMatcher final
    : public GenericMatcher<ICmpInst, ICmpInstMatcher<LhsMatcher, RhsMatcher>> {
  ValueId mValueId;
  // CmpOp mCmpOp;
  LhsMatcher mLhsMatcher;
  RhsMatcher mRhsMatcher;

  public:
  explicit ICmpInstMatcher(ValueId valueId,
                           LhsMatcher lhsMatcher,
                           RhsMatcher rhsMatcher)
      : mValueId{valueId}, mLhsMatcher{lhsMatcher}, mRhsMatcher{rhsMatcher} {}

  bool handle(const MatchContext<ICmpInst>& ctx) const {
    if (mValueId != ValueId::vInvalid and mValueId != ctx.value->valueId()) {
      return false;
    }
    if (mLhsMatcher(ctx.getOperand(0)) and mRhsMatcher(ctx.getOperand(1))) {
      return true;
    }
    return false;
  }
};

class PhiMatcher final : public GenericMatcher<PhiInst, PhiMatcher> {
  PhiInst*& mPhi;

  public:
  explicit PhiMatcher(PhiInst*& phi) noexcept : mPhi{phi} {}
  bool handle(const MatchContext<PhiInst>& ctx) const {
    mPhi = ctx.value;
    return true;
  }
};

inline auto phi(PhiInst*& phi) noexcept {
  return PhiMatcher{phi};
}

template <typename Matcher>
class OneUseMatcher {
  Matcher mMatcher;

  public:
  explicit OneUseMatcher(Matcher matcher) : mMatcher{matcher} {}
  bool operator()(const MatchContext<Value>& ctx) const {
    if (auto inst = ctx.value->dynCast<Instruction>()) {
      if (not(inst->uses().size() == 1)) {
        return false;
      } else {
        return mMatcher(ctx);
      }
    }
    return false;
  }
};

template <typename Matcher>
auto oneUse(Matcher matcher) {
  return OneUseMatcher<Matcher>{matcher};
}

class ExactlyMatcher final {
  Value*& mVal;

  public:
  explicit ExactlyMatcher(Value*& val) noexcept : mVal{val} {}
  bool operator()(const MatchContext<Value>& ctx) const noexcept {
    return ctx.value == mVal;
  }
};

inline auto exactly(Value*& val) noexcept {
  return ExactlyMatcher{val};
}

}  // namespace pass
