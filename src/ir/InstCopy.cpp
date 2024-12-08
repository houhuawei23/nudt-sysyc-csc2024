#include "ir/instructions.hpp"

using namespace ir;

Instruction* AllocaInst::copy(std::function<Value*(Value*)> getValue) const {
  auto inst = utils::make<AllocaInst>(baseType(), mIsConst);
  inst->setComment(mComment);
  return inst;
};

Instruction* StoreInst::copy(std::function<Value*(Value*)> getValue) const {
  auto inst = utils::make<StoreInst>(getValue(operand(0)), getValue(operand(1)));
  inst->setComment(mComment);
  return inst;
}

Instruction* LoadInst::copy(std::function<Value*(Value*)> getValue) const {
  auto inst = utils::make<LoadInst>(getValue(operand(0)), mType);
  inst->setComment(mComment);
  return inst;
}

Instruction* ReturnInst::copy(std::function<Value*(Value*)> getValue) const {
  return utils::make<ReturnInst>(getValue(returnValue()));
}

Instruction* UnaryInst::copy(std::function<Value*(Value*)> getValue) const {
  return utils::make<UnaryInst>(mValueId, mType, getValue(operand(0)));
}

Instruction* UnaryInst::clone() const {
  return utils::make<UnaryInst>(mValueId, mType, operand(0));
}

Instruction* BinaryInst::copy(std::function<Value*(Value*)> getValue) const {
  return utils::make<BinaryInst>(mValueId, mType, getValue(operand(0)), getValue(operand(1)));
}

Instruction* BinaryInst::clone() const {
  return utils::make<BinaryInst>(mValueId, mType, operand(0), operand(1));
}

Instruction* CallInst::copy(std::function<Value*(Value*)> getValue) const {
  std::vector<Value*> args;
  for (auto arg : mOperands) {
    auto val = getValue(arg->value());
    assert(val);
    args.push_back(val);
  }
  return utils::make<CallInst>(mCallee, args);
}
Instruction* CallInst::clone() const {
  std::vector<Value*> args;
  for (auto arg : mOperands) {
    args.push_back(arg->value());
  }
  return utils::make<CallInst>(mCallee, args);
}
Instruction* BranchInst::copy(std::function<Value*(Value*)> getValue) const {
  if (is_cond()) {
    return utils::make<BranchInst>(getValue(operand(0)), getValue(iftrue())->as<BasicBlock>(),
                                   getValue(iffalse())->as<BasicBlock>());

  } else {
    return utils::make<BranchInst>(getValue(dest())->as<BasicBlock>());
  }
}
Instruction* ICmpInst::copy(std::function<Value*(Value*)> getValue) const {
  return utils::make<ICmpInst>(mValueId, getValue(operand(0)), getValue(operand(1)));
}

Instruction* FCmpInst::copy(std::function<Value*(Value*)> getValue) const {
  return utils::make<FCmpInst>(mValueId, getValue(operand(0)), getValue(operand(1)));
}

Instruction* MemsetInst::copy(std::function<Value*(Value*)> getValue) const {
  return utils::make<MemsetInst>(getValue(operand(0)), getValue(operand(1)), getValue(operand(2)),
                                 getValue(operand(3)));
}

Instruction* GetElementPtrInst::copy(std::function<Value*(Value*)> getValue) const {
  auto newvalue = getValue(value());
  auto newidx = getValue(index());
  if (getid() == 0) {
    return utils::make<GetElementPtrInst>(baseType(), newvalue, newidx);
  } else if (getid() == 1) {
    auto basetype = baseType()->dynCast<ArrayType>();
    auto dims = basetype->dims();
    auto curdims = cur_dims();
    return utils::make<GetElementPtrInst>(basetype->baseType(), newvalue, newidx, dims, curdims);
  } else {
    return utils::make<GetElementPtrInst>(baseType(), newvalue, newidx, cur_dims());
  }
}
Instruction* PhiInst::copy(std::function<Value*(Value*)> getValue) const {
  return utils::make<PhiInst>(nullptr, mType);
}