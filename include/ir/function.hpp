#pragma once

#include "ir/infrast.hpp"
#include "ir/module.hpp"
#include "ir/type.hpp"
#include "ir/value.hpp"
#include "support/utils.hpp"
#include "support/arena.hpp"
#include "ir/Attribute.hpp"

namespace ir {

class Loop;

class Loop {
protected:
  Function* mParent;
  Loop* mParentLoop;
  std::unordered_set<Loop*> mSubLoops;

  std::unordered_set<BasicBlock*> mBlocks;
  BasicBlock* mHeader;
  std::unordered_set<BasicBlock*> mExits;
  std::unordered_set<BasicBlock*> mLatchs;

public:
  Loop(BasicBlock* header, Function* parent) {
    mHeader = header;
    mParent = parent;
  }
  auto header() const { return mHeader; }
  auto function() const { return mParent; }
  auto& blocks() { return mBlocks; }
  auto& exits() { return mExits; }
  auto& latchs() { return mLatchs; }
  auto& subLoops() { return mSubLoops; }

  Loop* parentloop() { return mParentLoop; }
  void setParent(Loop* lp) { mParentLoop = lp; }
  bool contains(BasicBlock* block) const { return mBlocks.find(block) != mBlocks.end(); }

  BasicBlock* getloopPredecessor() const;

  BasicBlock* getLoopPreheader() const;

  BasicBlock* getLoopLatch() const;

  BasicBlock* getUniqueLatch() const {
    assert(mLatchs.size() == 1);
    return *(mLatchs.begin());
  }

  bool hasDedicatedExits() const;

  bool isLoopSimplifyForm() const {
    return getLoopPreheader() && getLoopLatch() && hasDedicatedExits();
  }
  void print(std::ostream& os) const;

  void setLatch(BasicBlock* latch) {
    mLatchs.clear();
    mLatchs.insert(latch);
  }

  BasicBlock* getFirstBodyBlock() const {
    for (auto block : mHeader->next_blocks()) {
      if (contains(block)) {
        return block;
      }
    }
    assert(false && "no body block found");
    return nullptr;
  }
};

enum FunctionAttribute : uint32_t {
  NoMemoryRead = 1 << 0,
  NoMemoryWrite = 1 << 1,
  NoSideEffect = 1 << 2,
  Stateless = 1 << 3,
  NoAlias = 1 << 4,
  NoReturn = 1 << 5,
  NoRecurse = 1 << 6,
  Entry = 1 << 7,
  Builtin = 1 << 8,
  LoopBody = 1 << 9,
  ParallelBody = 1 << 10,
  AlignedParallelBody = 1 << 11,
  InlineWrapped = 1 << 12,
};

class Function : public User {
  friend class Module;

protected:
  Module* mModule = nullptr;  // parent Module

  block_ptr_list mBlocks;     // blocks of the function
  arg_ptr_vector mArguments;  // formal args

  Value* mRetValueAddr = nullptr;  // return value
  BasicBlock* mEntry = nullptr;    // entry block
  BasicBlock* mExit = nullptr;     // exit block
  size_t mVarCnt = 0;              // for local variables count
  size_t argCnt = 0;               // formal arguments count
  Attribute<FunctionAttribute> mAttribute;

public:
  Function(Type* TypeFunction, const_str_ref name = "", Module* parent = nullptr)
    : User(TypeFunction, vFUNCTION, name), mModule(parent) {
    argCnt = 0;
    mRetValueAddr = nullptr;
  }

public:  // get function
  auto& attribute() { return mAttribute; }
  auto module() const { return mModule; }

  auto retValPtr() const { return mRetValueAddr; }
  auto retType() const { return mType->as<FunctionType>()->retType(); }

  auto& blocks() const { return mBlocks; }
  auto& blocks() { return mBlocks; }

  auto entry() const { return mEntry; }
  auto exit() const { return mExit; }

  auto& args() const { return mArguments; }
  auto& argTypes() const { return mType->as<FunctionType>()->argTypes(); }
  auto arg_i(size_t idx) {
    assert(idx < argCnt && "idx out of args vector");
    return mArguments.at(idx);
  }

public:  // set function
  void setRetValueAddr(Value* value) {
    assert(mRetValueAddr == nullptr && "new_ret_value can not call 2th");
    mRetValueAddr = value;
  }
  void setEntry(ir::BasicBlock* bb) {
    mEntry = bb;
    bb->set_parent(this);
  }
  void setExit(ir::BasicBlock* bb) {
    mExit = bb;
    bb->set_parent(this);
  }

  BasicBlock* newBlock();
  BasicBlock* newEntry(const_str_ref name = "");
  BasicBlock* newExit(const_str_ref name = "");

  void delBlock(BasicBlock* bb);
  void forceDelBlock(BasicBlock* bb);

  auto new_arg(Type* btype, const_str_ref name = "") {
    auto arg = utils::make<Argument>(btype, argCnt, this, name);
    argCnt++;
    mArguments.emplace_back(arg);
    return arg;
  }

  auto varInc() { return mVarCnt++; }
  void setVarCnt(size_t x) { mVarCnt = x; }

  bool isOnlyDeclare() const { return mBlocks.empty(); }

  void delArgumant(size_t idx) {
    assert(idx < argCnt && "idx out of args vector");
    mArguments.erase(mArguments.begin() + idx);
  }

public:  // utils function
  static bool classof(const Value* v) { return v->valueId() == vFUNCTION; }
  ir::Function* copy_func();

  void rename();
  void dumpAsOpernd(std::ostream& os) const override;
  void print(std::ostream& os) const override;

  bool verify(std::ostream& os) const;

  void updateTypeFromArgs() {
    std::vector<Type*> argTypes;
    for (auto arg : mArguments) {
      argTypes.push_back(arg->type());
    }
    auto newType = FunctionType::gen(retType(), std::move(argTypes));
    mType = newType;
  }
};

}  // namespace ir