#include <iostream>
#include <fstream>
#include <queue>
#include <filesystem>
#include <string_view>
#include "pass/pass.hpp"
#include "pass/AnalysisInfo.hpp"
#include "pass/analysis/dom.hpp"
#include "mir/MIR.hpp"
#include "mir/lowering.hpp"
#include "mir/target.hpp"
#include "mir/iselinfo.hpp"
#include "mir/utils.hpp"
#include "mir/RegisterAllocator.hpp"
#include "mir/RegisterCoalescing.hpp"
#include "target/riscv/RISCVTarget.hpp"
#include "support/StaticReflection.hpp"
#include "support/config.hpp"
#include "support/Profiler.hpp"
#include "support/Graph.hpp"

namespace fs = std::filesystem;

using namespace mir;

static OperandType get_optype(ir::Type* type) {
  if (type->isInt()) {
    switch (type->btype()) {
      case ir::BasicTypeRank::INT1:
        return OperandType::Bool;
      case ir::BasicTypeRank::INT32:
        return OperandType::Int32;
      case ir::BasicTypeRank::INT64:
        return OperandType::Int64;
      default:
        assert(false && "unsupported int type");
    }
  } else if (type->isFloatPoint()) {
    switch (type->btype()) {
      case ir::BasicTypeRank::FLOAT:
        return OperandType::Float32;
      default:
        assert(false && "unsupported float type");
    }
  } else if (type->isPointer()) {
    /* NOTE: rv64 */
    return OperandType::Int64;
  } else {
    return OperandType::Special;
  }
}

MIROperand FloatPointConstantPool::getFloatConstant(class LoweringContext& ctx, float val) {
  uint32_t rep;
  memcpy(&rep, &val, sizeof(float));
  uint32_t offset;
  if (const auto it = mFloatOffsetMap.find(rep); it != mFloatOffsetMap.cend()) {
    offset = it->second;
  } else {
    // not found, materialize
    if (!mFloatDataStorage) {  // create data storage if not exist
      auto storage =
        std::make_unique<MIRDataStorage>(MIRDataStorage::Storage{}, true, "floatConstPool", true);
      mFloatDataStorage = storage.get();
      auto pool = std::make_unique<MIRGlobalObject>(sizeof(float), std::move(storage), nullptr);
      ctx.module.global_objs().push_back(std::move(pool));
    }
    offset = (mFloatOffsetMap[rep] = mFloatDataStorage->append_word(rep) * sizeof(float));
  }

  const auto ptrType = ctx.getPointerType();
  const auto base = ctx.newVReg(ptrType);
  /* LoadGlobalAddress base, reloc */
  ctx.emitMIRInst(InstLoadGlobalAddress, {base, MIROperand::asReloc(mFloatDataStorage)});

  // Add addr, base, offset
  const auto addr = ctx.newVReg(ptrType);
  ctx.emitMIRInst(InstAdd, {addr, base, MIROperand::asImm(offset, ptrType)});

  // Load dst, addr, 4
  const auto dst = ctx.newVReg(OperandType::Float32);
  ctx.emitMIRInst(InstLoad, {dst, addr, MIROperand::asImm(4, OperandType::Special)});
  return dst;
}

MIROperand LoweringContext::newVReg(ir::Type* type) {
  auto optype = get_optype(type);
  return MIROperand::asVReg(codeGenctx->nextId(), optype);
}

MIROperand LoweringContext::newVReg(OperandType type) {
  return MIROperand::asVReg(codeGenctx->nextId(), type);
}

void LoweringContext::addValueMap(ir::Value* ir_val, MIROperand mir_operand) {
  if (valueMap.count(ir_val))
    assert(false && "value already mapped");
  valueMap.emplace(ir_val, mir_operand);
}

MIROperand LoweringContext::map2operand(ir::Value* ir_val) {
  assert(ir_val && "null ir_val");
  /* 1. Local Value: alloca */
  if (auto iter = valueMap.find(ir_val); iter != valueMap.end()) {
    return iter->second;
  }

  /* 2. Global Value */
  if (auto gvar = ir_val->dynCast<ir::GlobalVariable>()) {
    auto ptr = newVReg(pointerType);
    /* LoadGlobalAddress ptr, reloc */
    emitMIRInst(InstLoadGlobalAddress, {ptr, MIROperand::asReloc(gvarMap.at(gvar)->reloc.get())});

    return ptr;
  } 
  // for function ptr
  if(auto func = ir_val->dynCast<ir::Function>()) {
    auto ptr = newVReg(pointerType);
    /* LoadGlobalAddress ptr, reloc */
    emitMIRInst(InstLoadGlobalAddress, {ptr, MIROperand::asReloc(funcMap.at(func))});
    return ptr;
  }

  /* 3. Constant */
  if (!ir_val->dynCast<ir::ConstantValue>()) {
    std::cerr << "error: " << ir_val->name() << " must be constant\n";
    assert(false);
  }

  auto const_val = ir_val->dynCast<ir::ConstantValue>();
  if (const_val->type()->isInt()) {
    auto imm = MIROperand::asImm(const_val->i32(), OperandType::Int32);
    return imm;
  }

  if (const_val->type()->isFloat32()) {
    const float floatval = const_val->f32();
    // find in block cache
    auto& blockCache = mBlockLoadedFloatCache[mCurrBlock];
    if (auto iter = blockCache.find(floatval); iter != blockCache.end()) {
      return iter->second;
    }
    // not found, materialize
    if (auto fpOperand = codeGenctx->iselInfo->materializeFPConstant(floatval, *this);
        fpOperand.isInit()) {
      blockCache.emplace(floatval, fpOperand);
      return fpOperand;
    }
    // not materialized, load from constant pool
    auto fpOperand = mFloatConstantPool.getFloatConstant(*this, floatval);
    blockCache.emplace(floatval, fpOperand);
    return fpOperand;
  }
  std::cerr << "Map2Operand Error: Not Supported IR Value Type: "
            << utils::enumName(static_cast<ir::BasicTypeRank>(ir_val->type()->btype()))
            << std::endl;
  assert(false && "Not Supported Type.");
  return MIROperand{};
}

void LoweringContext::emitCopy(MIROperand dst, MIROperand src) {
  /* copy dst, src */
  emitMIRInst(select_copy_opcode(dst, src), {dst, src});
}