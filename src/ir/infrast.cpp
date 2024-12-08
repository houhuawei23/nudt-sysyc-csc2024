#include "ir/infrast.hpp"
#include "ir/function.hpp"
#include "ir/utils_ir.hpp"
#include "ir/instructions.hpp"
#include "support/arena.hpp"
#include "ir/ConstantValue.hpp"
namespace ir {

void Argument::print(std::ostream& os) const {
  os << *type() << " " << name();
}
bool BasicBlock::isTerminal() const {
  if (mInsts.empty()) return false;
  return mInsts.back()->isTerminator();
}

bool BasicBlock::verify(std::ostream& os) const {
  if (mInsts.empty()) return false;
  for (auto inst : mInsts) {
    if (inst->isTerminator() and inst != mInsts.back()) {
      inst->print(os);
      os << "block have terminator inst not at the end" << std::endl;
      return false;
    }
  }
  // end with a terminator inst
  if (not mInsts.back()->isTerminator()) return false;

  return true;
}

void BasicBlock::print(std::ostream& os) const {
  // print all instructions

  os << name() << ":";
  /* comment begin */
  if (not mComment.empty()) {
    os << " ; " << mComment << std::endl;
  } else {
    os << std::endl;
  }
  if (not mPreBlocks.empty()) {
    os << "    ; " << "pres: ";
    for (auto it = pre_blocks().begin(); it != pre_blocks().end(); it++) {
      os << (*it)->name();
      if (std::next(it) != pre_blocks().end()) {
        os << ", ";
      }
    }
    os << std::endl;
  }
  if (not mNextBlocks.empty()) {
    os << "    ; " << "nexts: ";
    for (auto it = next_blocks().begin(); it != next_blocks().end(); it++) {
      os << (*it)->name();
      if (std::next(it) != next_blocks().end()) {
        os << ", ";
      }
    }
    os << std::endl;
  }
  /* comment end */

  for (auto& inst : mInsts) {
    os << "    " << *inst << std::endl;
  }
}

void BasicBlock::emplace_inst(inst_iterator pos, Instruction* i) {
  // Warning: didn't check _is_terminal
  if (i->isTerminator() and isTerminal()) {
    std::cerr << "[Warning] insert a terminal inst to a terminal bb" << std::endl;
  }
  mInsts.emplace(pos, i);
  i->setBlock(this);
  if (auto phiInst = dyn_cast<PhiInst>(i)) {
    // assume that Phi insts are all at the front of a bb
    size_t index = std::distance(mInsts.begin(), pos);
    mPhiInsts.emplace(std::next(mPhiInsts.begin(), index), phiInst);
  }
}
void BasicBlock::emplace_first_inst(Instruction* inst) {
  // Warning: didn't check _is_terminal
  if (inst->isTerminator() and isTerminal()) {
    std::cerr << "[Warning] insert a terminal inst to a terminal bb" << std::endl;
  }

  if (auto phiInst = dyn_cast<PhiInst>(inst)) {
    mPhiInsts.push_front(inst);
    auto pos = mInsts.begin();
    mInsts.emplace(pos, inst);
  } else {
    auto pos = mInsts.begin();
    for (size_t i = 0; i < mPhiInsts.size(); i++) {
      pos++;
    }
    mInsts.emplace(pos, inst);
  }
  inst->setBlock(this);
}

void BasicBlock::emplace_back_inst(Instruction* i) {
  if (isTerminal()) {
    std::cerr << "[Warning] emplace_back a non-terminal inst to a terminal bb" << std::endl;
    assert(false);
    return;
  }
  mInsts.emplace_back(i);
  i->setBlock(this);
  if (auto phiInst = dyn_cast<PhiInst>(i))
    // assert(false and "a phi can not be inserted at the back of a bb");
    mPhiInsts.emplace_back(phiInst);
}

void BasicBlock::emplace_lastbutone_inst(Instruction* i) {
  if (mInsts.size() == 1) {
    emplace_first_inst(i);
  } else {
    auto iter = mInsts.end();
    iter--;
    emplace_inst(iter, i);
  }
  i->setBlock(this);
}

void Instruction::setvarname() {
  auto cur_func = mBlock->function();
  mName = "%" + std::to_string(cur_func->varInc());
}

void BasicBlock::delete_inst(Instruction* inst) {
  // if inst1 use 2, 2->mUses have use user inst
  // in 2, del use of 1
  // if 3 use inst, 3.operands have use(3, 1)
  // first replace use(3, 1)
  // if you want to delete a inst, all use of it must be deleted in advance
  assert(inst->uses().size() == 0);
  for (auto op_use : inst->operands()) {
    auto op = op_use->value();
    op->uses().remove(op_use);
  }
  mInsts.remove(inst);
  if (auto phiInst = dyn_cast<PhiInst>(inst)) mPhiInsts.remove(phiInst);

  // delete inst;
}

void BasicBlock::force_delete_inst(Instruction* inst) {
  // assert(inst->uses().size()==0);
  for (auto op_use : inst->operands()) {
    auto op = op_use->value();
    op->uses().remove(op_use);
  }
  mInsts.remove(inst);
  if (auto phiInst = dyn_cast<PhiInst>(inst)) mPhiInsts.remove(phiInst);
}

void BasicBlock::move_inst(Instruction* inst) {
  // assert(inst->uses().size()==0);
  mInsts.remove(inst);
  if (auto phiInst = dyn_cast<PhiInst>(inst)) mPhiInsts.remove(phiInst);
}

void BasicBlock::replaceinst(Instruction* old_inst, Value* new_) {
  inst_iterator pos = find(mInsts.begin(), mInsts.end(), old_inst);
  if (pos != mInsts.end()) {
    if (auto inst = dyn_cast<Instruction>(new_)) {
      emplace_inst(pos, inst);
      old_inst->replaceAllUseWith(inst);
      delete_inst(old_inst);
    } else if (auto constant = new_->dynCast<ConstantValue>()) {
      old_inst->replaceAllUseWith(constant);
      delete_inst(old_inst);
    }
  }
}

bool Instruction::isTerminator() {
  return mValueId == vRETURN || mValueId == vBR;
}
bool Instruction::isUnary() {
  return mValueId > vUNARY_BEGIN && mValueId < vUNARY_END;
};
bool Instruction::isBinary() {
  return mValueId > vBINARY_BEGIN && mValueId < vBINARY_END;
};
bool Instruction::isBitWise() {
  return false;
}
bool Instruction::isMemory() {
  return mValueId == vALLOCA || mValueId == vLOAD || mValueId == vSTORE ||
         mValueId == vGETELEMENTPTR;
};
bool Instruction::isNoName() {
  return isTerminator() or mValueId == vSTORE or mValueId == vMEMSET;
}
bool Instruction::isAggressiveAlive() {
  return mValueId == vSTORE or mValueId == vCALL or mValueId == vMEMSET or mValueId == vRETURN or
         mValueId == vATOMICRMW;
}
bool Instruction::hasSideEffect() {
  if (mValueId == vSTORE or mValueId == vMEMSET or mValueId == vRETURN) return true;
  return false;  // 默认call没有
}

// bool Instruction::verify(std::ostream& os) const{
//   for(auto use: mUses) {
//     assert(use);
//     assert(use->value());
//     assert(use->user());
//   }
// }

}  // namespace ir