#include "pass/optimize/InstCombine/ArithmeticReduce.hpp"
#include "pass/optimize/Utils/PatternMatch.hpp"
#include "pass/optimize/Utils/BlockUtils.hpp"

#include "support/StaticReflection.hpp"
#include <iostream>
#include <cassert>
#include <algorithm>
using namespace ir;
namespace pass {

bool ArithmeticReduce::runOnBlock(ir::IRBuilder& builder,
                                  ir::BasicBlock& block) {
  bool debug = false;
  bool modified = false;
  auto reducer = [&](Instruction* inst) -> Value* {
    if (debug) {
      std::cerr << "Checking: " << inst->valueId() << std::endl;
    }
    // commutative c x -> commutative x c
    auto isConst = [](Value* v) { return v->isa<ConstantValue>(); };

    if (const auto biInst = inst->dynCast<BinaryInst>()) {
      if (biInst->isCommutative()) {
        if (biInst->operand(0)->isa<ConstantValue>() and
            !biInst->operand(1)->isa<ConstantValue>()) {
          // std::cerr << "Swap operands: "
          //           <<
          //           utils::enumName(static_cast<ValueId>(biInst->valueId()))
          //           << std::endl;
          auto& operands = biInst->operands();
          std::swap(operands[0], operands[1]);
          /* remember to maintain the index of operands */
          operands[0]->set_index(0);
          operands[1]->set_index(1);
          modified = true;
        }
      }
    }

    MatchContext<Value> matchCtx{inst};
    Value *v1, *v2, *v3, *v4;
    int64_t i1, i2;
    float f1, f2;
    // add(x, 0) = x
    if (add(any(v1), intval_(0))(matchCtx)) return v1;
    if (fadd(any(v1), floatval_(0.0))(matchCtx)) return v1;
    // sub(x, 0) = x
    if (sub(any(v1), intval_(0))(matchCtx)) return v1;

    if (fsub(any(v1), floatval_(0.0))(matchCtx)) return v1;

    // mul(x, 0) = 0
    if (mul(any(v1), intval_(0))(matchCtx)) return ConstantInteger::gen_i32(0);
    if (fmul(any(v1), floatval_(0.0))(matchCtx)) return ConstantFloating::gen_f32(0.0);
    // mul(x, 1) = x
    if (mul(any(v1), intval_(1))(matchCtx)) return v1;
    if (fmul(any(v1), floatval_(1.0))(matchCtx)) return v1;

    // div(0, x) = 0
    if (sdiv(intval_(0), any(v1))(matchCtx)) return ConstantInteger::gen_i32(0);
    if (fdiv(floatval_(0.0), any(v1))(matchCtx)) return ConstantFloating::gen_f32(0.0);
    // div(x, 1) = x
    if (sdiv(any(v1), intval_(1))(matchCtx)) return v1;
    if (fdiv(any(v1), floatval_(1.0))(matchCtx)) return v1;
    // FIXME: div(x, -1) = -x
    // if (sdiv(any(v1), intval_(-1))(matchCtx)) {
    //   return builder.makeInst<BinaryInst>(ValueId::vNEG, v1->type(), v1);
    // }
    if (fdiv(any(v1), floatval_(-1.0))(matchCtx)) {
      return builder.makeInst<UnaryInst>(ValueId::vFNEG, v1->type(), v1);
    }
    // div(x, x) = 1
    if (sdiv(any(v1), exactly(v1))(matchCtx)) return ConstantInteger::gen_i32(1);
    if (fdiv(any(v1), exactly(v1))(matchCtx)) return ConstantFloating::gen_f32(1.0);
    // (a / c1) / c2 -> a / (c1 * c2)
    if (sdiv(sdiv(any(v1), any(v3)), any(v4))(matchCtx)) {
      const auto mulInst =
          builder.makeInst<BinaryInst>(ValueId::vMUL, v1->type(), v3, v4);

      return builder.makeInst<BinaryInst>(ValueId::vSDIV, v1->type(), v1,
                                          mulInst);
    }

    // TODO: x / (2^k) -> x * 2^(-k)

    /* rem */
    // 0 % x -> 0
    if (srem(intval_(0), any(v1))(matchCtx)) return ConstantInteger::gen_i32(0);
    // a % 1 -> 0
    if (srem(any(v1), intval_(1))(matchCtx)) return ConstantInteger::gen_i32(0);
    // a % a -> 0
    if (srem(any(v1), exactly(v1))(matchCtx)) return ConstantInteger::gen_i32(0);

    // a * b + a * c = a * (b + c)
    if (add(oneUse(mul(any(v1), any(v2))),
            oneUse(mul(any(v3), any(v4))))(matchCtx)) {
      Value *a = nullptr, *b = nullptr, *c = nullptr;
      if (v1 == v3) {
        a = v1, b = v2, c = v4;
      } else if (v1 == v4) {
        a = v1, b = v2, c = v3;
      } else if (v2 == v3) {
        a = v2, b = v1, c = v4;
      } else if (v2 == v4) {
        a = v2, b = v1, c = v3;
      }
      if (a && b && c) {
        const auto addInst =
            builder.makeInst<BinaryInst>(ValueId::vADD, a->type(), b, c);
        return builder.makeInst<BinaryInst>(ValueId::vMUL, a->type(), a,
                                            addInst);
      }
    }
    // a * b - a * c = a * (b - c)
    if (sub(oneUse(mul(any(v1), any(v2))),
            oneUse(mul(any(v3), any(v4))))(matchCtx)) {
      Value *a = nullptr, *b = nullptr, *c = nullptr;
      if (v1 == v3) {
        a = v1, b = v2, c = v4;
      } else if (v1 == v4) {
        a = v1, b = v2, c = v3;
      } else if (v2 == v3) {
        a = v2, b = v1, c = v4;
      } else if (v2 == v4) {
        a = v2, b = v1, c = v3;
      }
      if (a && b && c) {
        const auto subInst =
            builder.makeInst<BinaryInst>(ValueId::vSUB, a->type(), b, c);
        return builder.makeInst<BinaryInst>(ValueId::vMUL, a->type(), a,
                                            subInst);
      }
    }
    // TODO: c % (2^k) = c & (2^k - 1)

    // add(add(x, c1), c2) = add(x, c1+c2)
    if (add(add(any(v1), int_(i1)), int_(i2))(matchCtx)) {
      return builder.makeInst<BinaryInst>(ValueId::vADD, v1->type(), v1,
                                          ConstantInteger::gen_i32(i1 + i2));
    }
    // add(sub(x, c1), c2) = sub(x, c1-c2)
    if (add(sub(any(v1), int_(i1)), int_(i2))(matchCtx)) {
      return builder.makeInst<BinaryInst>(ValueId::vSUB, v1->type(), v1,
                                          ConstantInteger::gen_i32(i1 - i2));
    }
    // sub(sub(x, c1), c2) = sub(x, c1+c2)
    if (sub(sub(any(v1), int_(i1)), int_(i2))(matchCtx)) {
      return builder.makeInst<BinaryInst>(ValueId::vSUB, v1->type(), v1,
                                          ConstantInteger::gen_i32(i1 + i2));
    }
    // sub(add(x, c1), c2) = add(x, c1-c2)
    if (sub(add(any(v1), int_(i1)), int_(i2))(matchCtx)) {
      return builder.makeInst<BinaryInst>(ValueId::vADD, v1->type(), v1,
                                          ConstantInteger::gen_i32(i1 - i2));
    }
    // (a / ci + b / ci) -> (a + b) / ci
    if (add(sdiv(any(v1), int_(i1)), sdiv(any(v2), int_(i2)))(matchCtx)) {
      if (i1 == i2) {
        auto addInst =
            builder.makeInst<BinaryInst>(ValueId::vADD, v1->type(), v1, v2);
        return builder.makeInst<BinaryInst>(ValueId::vSDIV, v1->type(), addInst,
                                            ConstantInteger::gen_i32(i1));
      }
    }
    // (a / cf + b / cf) -> (a + b) / cf
    if (add(fdiv(any(v1), float_(f1)), fdiv(any(v2), float_(f2)))(matchCtx)) {
      if (f1 == f2) {
        auto addInst =
            builder.makeInst<BinaryInst>(ValueId::vADD, v1->type(), v1, v2);
        return builder.makeInst<BinaryInst>(ValueId::vFDIV, v1->type(), addInst,
                                            ConstantFloating::gen_f32(f1));
      }
    }
    // (a / c + b / c) -> (a + b) / c
    if (add(sdiv(any(v1), any(v3)), sdiv(any(v2), exactly(v3)))(matchCtx)) {
      const auto addInst =
          builder.makeInst<BinaryInst>(ValueId::vADD, v1->type(), v1, v2);
      return builder.makeInst<BinaryInst>(ValueId::vSDIV, v1->type(), addInst,
                                          v3);
    }

    // FIXME: reduce single value phiInst
    PhiInst* phiInst = nullptr;
    if (phi(phiInst)(matchCtx)) {
      if (phiInst->operands().size() == 1) {
        return phiInst->operand(0);
      }
    }

    return nullptr;
  };
  const auto ret = reduceBlock(builder, block, reducer);
  return modified | ret;
}
};  // namespace pass