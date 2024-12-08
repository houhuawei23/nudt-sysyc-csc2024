#include "ir/function.hpp"
#include "ir/utils_ir.hpp"
#include "ir/type.hpp"
#include "ir/instructions.hpp"
#include "support/arena.hpp"
namespace ir {
void Loop::print(std::ostream& os) const {
  os << "header: " << mHeader->name() << std::endl;
  os << "exits: " << mExits.size() << std::endl;
  for (auto exit : mExits) {
    os << "  " << exit->name() << std::endl;
  }
  os << "latchs: " << mLatchs.size() << std::endl;
  for (auto latch : mLatchs) {
    os << "  " << latch->name() << std::endl;
  }
  os << "blocks: " << mBlocks.size() << std::endl;
  for (auto bb : mBlocks) {
    os << "  " << bb->name() << std::endl;
  }
}

BasicBlock* Loop::getloopPredecessor() const {
  BasicBlock* predecessor = nullptr;
  BasicBlock* Header = header();
  for (auto* pred : Header->pre_blocks()) {
    if (!contains(pred)) {
      if (predecessor && (predecessor != pred)) {
        return nullptr;  // 多个前驱
      }
      predecessor = pred;
    }
  }
  return predecessor;  // 返回唯一的predecessor
}

BasicBlock* Loop::getLoopPreheader() const {
  BasicBlock* preheader = getloopPredecessor();
  if (!preheader) return nullptr;
  if (preheader->next_blocks().size() != 1) return nullptr;
  return preheader;
}

BasicBlock* Loop::getLoopLatch() const {
  BasicBlock* latch = nullptr;
  BasicBlock* Header = header();
  for (auto pred : Header->pre_blocks()) {
    if (contains(pred)) {
      if (latch) return nullptr;
      latch = pred;
    }
  }
  return latch;  // 返回唯一的latch
}

bool Loop::hasDedicatedExits() const {
  for (auto exitbb : mExits) {
    if (exitbb->pre_blocks().size() != 1) return false;
    // for (auto pred : exitbb->pre_blocks()) {
    //   if (!contains(pred)) return false;
    // }
  }
  return true;
}

BasicBlock* Function::newBlock() {
  auto nb = utils::make<BasicBlock>("", this);
  mBlocks.emplace_back(nb);
  return nb;
}

BasicBlock* Function::newEntry(const_str_ref name) {
  assert(mEntry == nullptr);
  mEntry = utils::make<BasicBlock>(name, this);
  mBlocks.emplace_back(mEntry);
  return mEntry;
}
BasicBlock* Function::newExit(const_str_ref name) {
  mExit = utils::make<BasicBlock>(name, this);
  mBlocks.emplace_back(mExit);
  return mExit;
}
void Function::delBlock(BasicBlock* bb) {
  for (auto bbpre : bb->pre_blocks()) {
    bbpre->next_blocks().remove(bb);
  }
  for (auto bbnext : bb->next_blocks()) {
    bbnext->pre_blocks().remove(bb);
  }
  for (auto bbinstIter = bb->insts().begin(); bbinstIter != bb->insts().end();) {
    auto delinst = *bbinstIter;
    bbinstIter++;
    bb->delete_inst(delinst);
  }
  mBlocks.remove(bb);
  // delete bb;
}

void Function::forceDelBlock(BasicBlock* bb) {
  for (auto bbpre : bb->pre_blocks()) {
    bbpre->next_blocks().remove(bb);
  }
  for (auto bbnext : bb->next_blocks()) {
    bbnext->pre_blocks().remove(bb);
  }
  for (auto bbinstIter = bb->insts().begin(); bbinstIter != bb->insts().end();) {
    auto delinst = *bbinstIter;
    bbinstIter++;
    bb->force_delete_inst(delinst);
  }
  mBlocks.remove(bb);
}
void Function::dumpAsOpernd(std::ostream& os) const {
  os << "@" << mName;
}

void Function::print(std::ostream& os) const {
  auto return_type = retType();
  if (not isOnlyDeclare()) {
    os << "define " << *return_type << " @" << name() << "(";
    if (mArguments.size() > 0) {
      auto last_iter = mArguments.end() - 1;
      for (auto iter = mArguments.begin(); iter != last_iter; ++iter) {
        auto arg = *iter;
        os << *(arg->type()) << " " << arg->name();
        os << ", ";
      }
      auto arg = *last_iter;
      os << *(arg->type()) << " " << arg->name();
    }
  } else {
    os << "declare " << *return_type << " @" << name() << "(";
    auto t = type();
    if (auto funcType = t->dynCast<FunctionType>()) {
      auto args_types = funcType->argTypes();
      if (args_types.size() > 0) {
        auto last_iter = args_types.end() - 1;
        for (auto iter = args_types.begin(); iter != last_iter; ++iter) {
          os << **iter << ", ";
        }
        os << **last_iter;
      }
    } else {
      assert(false && "Unexpected type");
    }
  }

  os << ")";

  // print bbloks
  if (blocks().size()) {
    os << " {\n";
    for (auto& bb : mBlocks) {
      os << *bb;
    }
    os << "}";
  } else {
    os << "\n";
  }
}

void Function::rename() {
  if (mBlocks.empty()) return;
  setVarCnt(0);
  for (auto arg : mArguments) {
    std::string argname = "%" + std::to_string(varInc());
    arg->set_name(argname);
  }
  size_t blockIdx = 0;
  for (auto bb : mBlocks) {
    bb->set_idx(blockIdx);
    blockIdx++;
    for (auto inst : bb->insts()) {
      if (inst->isNoName()) continue;
      auto callpt = inst->dynCast<CallInst>();
      if (callpt and callpt->isVoid()) continue;
      inst->setvarname();
    }
  }
}

// func_copy
Function* Function::copy_func() {
  std::unordered_map<Value*, Value*> valueMap;
  // copy global
  for (auto gvalue : mModule->globalVars()) {
    valueMap.emplace(gvalue, gvalue);
  }
  // copy func
  auto copyfunc = utils::make<Function>(type(), name() + "_copy", mModule);
  // copy args
  for (auto arg : mArguments) {
    Value* copyarg = copyfunc->new_arg(arg->type(), "");
    valueMap.emplace(arg, copyarg);
  }
  // copy block
  for (auto bb : mBlocks) {
    BasicBlock* copybb = copyfunc->newBlock();
    if (copyfunc->entry() == nullptr) {
      copyfunc->setEntry(copybb);
    } else if (copyfunc->exit() == nullptr) {
      copyfunc->setExit(copybb);
    }
    // valueMap[bb] = copybb;
    valueMap.emplace(bb, copybb);
  }

  // copy bb's pred and succ
  for (auto BB : mBlocks) {
    auto copyBB = valueMap.at(BB)->dynCast<BasicBlock>();
    for (auto pred : BB->pre_blocks()) {
      copyBB->pre_blocks().emplace_back(valueMap.at(pred)->dynCast<BasicBlock>());
    }
    for (auto succ : BB->next_blocks()) {
      copyBB->next_blocks().emplace_back(valueMap.at(succ)->dynCast<BasicBlock>());
    }
  }
  // if cant find, return itself
  auto getValue = [&](Value* val) -> Value* {
    if (val == nullptr) {
      std::cerr << "getValue(nullptr)" << std::endl;
      return nullptr;
    }
    if (val->isa<ConstantValue>()) return val;
    if (auto iter = valueMap.find(val); iter != valueMap.end()) return iter->second;
    return val;
  };

  // copy inst in bb
  std::vector<PhiInst*> phis;
  std::set<BasicBlock*> vis;

  const auto copyBlock = [&](BasicBlock* bb) -> bool {
    if (vis.count(bb)) return true;
    vis.insert(bb);
    auto bbCpy = valueMap.at(bb)->dynCast<BasicBlock>();
    for (auto inst : bb->insts()) {
      // inst->print(std::cerr);
      // std::cerr << std::endl;
      auto copyinst = inst->copy(getValue);
      copyinst->setBlock(bbCpy);
      valueMap.emplace(inst, copyinst);
      bbCpy->emplace_back_inst(copyinst);
      if (auto phi = inst->dynCast<PhiInst>()) phis.emplace_back(phi);
    }
    return false;
  };
  BasicBlock::BasicBlockDfs(mEntry, copyBlock);

  for (auto phi : phis) {
    auto copyphi = valueMap.at(phi)->dynCast<PhiInst>();
    for (size_t i = 0; i < phi->getsize(); i++) {
      auto phivalue = getValue(phi->getValue(i));
      auto phibb = valueMap.at(phi->getBlock(i))->dynCast<BasicBlock>();
      copyphi->addIncoming(phivalue, phibb);
    }
  }
  return copyfunc;
}

bool Function::verify(std::ostream& os) const {
  for (auto block : mBlocks) {
    if (not block->verify(os)) {
      os << "block: " << block->name() << " falied" << std::endl;
      return false;
    }
  }
  return true;
}

}  // namespace ir