#include "ir/instructions.hpp"

#include "ir/utils_ir.hpp"
#include "ir/ConstantValue.hpp"

using namespace ir;


Value* BinaryInst::getConstantRepl(bool recursive) {
  auto lval = lValue();
  auto rval = rValue();
  assert(lval->type()->isSame(rval->type()));
  if (recursive) {
    if (auto lvalInst = lval->dynCast<Instruction>()) {
      lval = lvalInst->getConstantRepl(recursive);
    }
    if (auto rvalInst = rval->dynCast<Instruction>()) {
      rval = rvalInst->getConstantRepl(recursive);
    }
  }

  if (not(lval->isa<ConstantValue>() and rval->isa<ConstantValue>()))
    return nullptr;

  int32_t i32val = 0;
  float f32val = 0.0;

  auto clval = lval->dynCast<ConstantValue>();
  auto crval = rval->dynCast<ConstantValue>();

  switch (mValueId) {
    case vADD:
      i32val = clval->i32() + crval->i32();
      break;
    case vSUB:
      i32val = clval->i32() - crval->i32();
      break;
    case vMUL:
      i32val = clval->i32() * crval->i32();
      break;
    case vSDIV:
      i32val = clval->i32() / crval->i32();
      break;
    case vSREM:
      i32val = clval->i32() % crval->i32();
      break;
    case vFADD:
      f32val = clval->f32() + crval->f32();
      break;
    case vFSUB:
      f32val = clval->f32() - crval->f32();
      break;
    case vFMUL:
      f32val = clval->f32() * crval->f32();
      break;
    case vFDIV:
      f32val = clval->f32() / crval->f32();
      break;
    default:
      assert(false and "Error in BinaryInst::getConstantRepl!");
  }
  if (type()->isFloat32())
    return ConstantFloating::gen_f32(f32val);
  else
    return ConstantInteger::gen_i32(i32val);
}

Value* UnaryInst::getConstantRepl(bool recursive) {
  auto val = value();
  if (recursive) {
    if (auto valInst = val->dynCast<Instruction>())
      val = valInst->getConstantRepl(recursive);
  }
  if (not val->isa<ConstantValue>())
    return nullptr;

  auto cval = val->dynCast<ConstantValue>();

  switch (valueId()) {
    case vSITOFP:
      return ConstantFloating::gen_f32(cval->i32());
    case vFPTOSI:
      return ConstantInteger::gen_i32(cval->f32());
    case vZEXT:
      return ConstantInteger::gen_i32(cval->i1());
    case vFNEG:
      return ConstantFloating::gen_f32(-cval->f32());
    default:
      std::cerr << mValueId << std::endl;
      assert(false && "Invalid scid from UnaryInst::getConstantRepl");
  }

  return nullptr;
}

Value* ICmpInst::getConstantRepl(bool recursive) {
  auto lval = lhs();
  auto rval = rhs();
  if (recursive) {
    if (auto lhsInst = lval->dynCast<Instruction>())
      lval = lhsInst->getConstantRepl(recursive);
    if (auto rhsInst = rval->dynCast<Instruction>())
      rval = rhsInst->getConstantRepl(recursive);
  }
  if (not(lval->isa<ConstantValue>() and rval->isa<ConstantValue>()))
    return nullptr;

  auto clval = lval->dynCast<ConstantValue>()->i32();
  auto crval = rval->dynCast<ConstantValue>()->i32();
  switch (valueId()) {
    case vIEQ:
      return ConstantInteger::gen_i1(clval == crval);
    case vINE:
      return ConstantInteger::gen_i1(clval != crval);
    case vISGT:
      return ConstantInteger::gen_i1(clval > crval);
    case vISLT:
      return ConstantInteger::gen_i1(clval < crval);
    case vISGE:
      return ConstantInteger::gen_i1(clval >= crval);
    case vISLE:
      return ConstantInteger::gen_i1(clval <= crval);
    default:
      assert(false and "icmpinst const flod error");
  }
  return nullptr;
}

Value* FCmpInst::getConstantRepl(bool recursive) {
  auto lval = lhs();
  auto rval = rhs();
  if (recursive) {
    if (auto lhsInst = lval->dynCast<Instruction>())
      lval = lhsInst->getConstantRepl(recursive);
    if (auto rhsInst = rval->dynCast<Instruction>())
      rval = rhsInst->getConstantRepl(recursive);
  }
  if (not(lval->isa<ConstantValue>() and rval->isa<ConstantValue>()))
    return nullptr;

  auto clval = lval->dynCast<ConstantValue>()->f32();
  auto crval = rval->dynCast<ConstantValue>()->f32();
  switch (valueId()) {
    case vFOEQ:
      return ConstantInteger::gen_i1(clval == crval);
    case vFONE:
      return ConstantInteger::gen_i1(clval != crval);
    case vFOGT:
      return ConstantInteger::gen_i1(clval > crval);
    case vFOLT:
      return ConstantInteger::gen_i1(clval < crval);
    case vFOGE:
      return ConstantInteger::gen_i1(clval >= crval);
    case vFOLE:
      return ConstantInteger::gen_i1(clval <= crval);
    default:
      assert(false and "icmpinst const flod error");
  }
  return nullptr;
}

Value* PhiInst::getConstantRepl(bool recursive) {
  if (mSize == 0)
    return nullptr;
  auto curval = getValue(0);
  if (mSize == 1)
    return getValue(0);
  for (size_t i = 1; i < mSize; i++) {
    if (getValue(i) != curval)
      return nullptr;
  }
  return curval;
}