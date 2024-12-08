
#include "mir/MIR.hpp"
#include "mir/target.hpp"
#include "mir/LiveInterval.hpp"
#include "mir/RegisterAllocator.hpp"
#include <queue>
#include <unordered_set>
#include <iostream>

namespace mir {

void FastAllocatorContext::collectUseDefInfo(MIRFunction& mfunc, CodeGenContext& ctx) {
  const auto collect = [&](MIRBlock* block, MIRInst* inst) {
    auto& instInfo = ctx.instInfo.getInstInfo(inst);

    /* CopyFromReg $Dst:VRegOrISAReg[Def], $Src:ISAReg[Use] */
    if (inst->opcode() == InstCopyFromReg) {
      isaRegHint[inst->operand(0)] = inst->operand(1);
    }
    if (inst->opcode() == InstCopyToReg) {
      isaRegHint[inst->operand(1)] = inst->operand(0);
    }

    /* 统计虚拟寄存器相关定义和使用情况 */
    for (size_t idx = 0; idx < instInfo.operand_num(); ++idx) {
      const auto& op = inst->operand(idx);
      if (!isOperandVReg(op)) continue;

      if (instInfo.operand_flag(idx) & OperandFlagUse) {
        useDefInfo[op].uses.insert(block);
      }
      if (instInfo.operand_flag(idx) & OperandFlagDef) {
        useDefInfo[op].defs.insert(block);
      }
    }
  };

  for (auto& block : mfunc.blocks()) {
    for (auto inst : block->insts()) {
      collect(block.get(), inst);
    }
  }
}

void FastAllocatorContext::collectStackMap(MIRFunction& mfunc, CodeGenContext& ctx) {
  // find all cross-block vregs and allocate stack slot for them
  for (auto& [reg, info] : useDefInfo) {
    /* 1. reg未在块中被定义或定义后未被使用 -> invalid */
    if (info.uses.empty() || info.defs.empty()) {
      continue;  // invalid
    }
    /* 2. reg定义和使用在同一块中 */
    if (info.uses.size() == 1 && info.defs.size() == 1 &&
        *info.uses.cbegin() == *info.defs.cbegin()) {
      continue;  // local
    }

    /* 3. reg的定义和使用跨多个块 -> 防止占用寄存器过久, spill到内存 */
    const auto size = getOperandSize(ctx.registerInfo->getCanonicalizedRegisterType(reg.type()));
    const auto storage =
      mfunc.newStackObject(ctx.nextId(), size, size, 0, StackObjectUsage::RegSpill);
    stackMap[reg] = storage;
  }
}

/* 局部块内需要spill到内存的虚拟寄存器 */
const auto BlockAllocator::getStackStorage(const MIROperand& op) {
  auto& mfunc = allocateCtx.mfunc;
  auto& ctx = allocateCtx.ctx;
  const auto& stackMap = allocateCtx.stackMap;

  if (const auto iter = localStackMap.find(op); iter != localStackMap.cend()) {
    return iter->second;
  }
  auto& ref = localStackMap[op];
  if (const auto iter = stackMap.find(op); iter != stackMap.cend()) {
    return ref = iter->second;
  }
  const auto size = getOperandSize(ctx.registerInfo->getCanonicalizedRegisterType(op.type()));
  const auto storage =
    mfunc.newStackObject(ctx.nextId(), size, size, 0, StackObjectUsage::RegSpill);
  return ref = storage;
};

/* 操作数的相关映射 */
auto& BlockAllocator::getDataMap(const MIROperand& op) {
  auto& map = currentMap[op];
  if (map.empty()) map.push_back(getStackStorage(op));
  return map;
};

const auto BlockAllocator::isAllocatableType(OperandType type) {
  return type <= OperandType::Float32;
};

/* collect underRenamed ISARegisters (寄存器重命名) */
const auto BlockAllocator::collectUnderRenamedISARegs(MIRInstList::iterator it) {
  auto& ctx = allocateCtx.ctx;
  while (it != mCurrBlock->insts().end()) {
    const auto inst = *it;
    auto& instInfo = ctx.instInfo.getInstInfo(inst);
    bool hasReg = false;
    for (size_t idx = 0; idx < instInfo.operand_num(); ++idx) {
      const auto& op = inst->operand(idx);
      if (isOperandISAReg(op) && !ctx.registerInfo->is_zero_reg(op.reg()) &&
          isAllocatableType(op.type()) && (instInfo.operand_flag(idx) & OperandFlagUse)) {
        underRenamedISAReg.insert(op);
        hasReg = true;
      }
    }
    if (hasReg)
      ++it;
    else
      break;
  }
};

const auto BlockAllocator::isProtected(const MIROperand& isaReg,
                                       std::unordered_set<MIROperand, MIROperandHasher>& protect) {
  assert(isOperandISAReg(isaReg));
  return protect.count(isaReg) || protectedLockedISAReg.count(isaReg) ||
         underRenamedISAReg.count(isaReg);
};

const auto BlockAllocator::evictVReg(MIROperand operand) {
  assert(isOperandVReg(operand));
  auto& map = getDataMap(operand);
  MIROperand isaReg;
  bool alreadyInStack = false;
  for (auto& reg : map) {
    if (isStackObject(reg.reg())) {
      alreadyInStack = true;
    }
    if (isISAReg(reg.reg())) isaReg = reg;
  }
  if (isaReg.isUnused()) return;
  physMap.erase(isaReg);
  const auto stackStorage = getStackStorage(operand);
  if (!alreadyInStack) {
    // spill to stack
    insertMIRInst(InstStoreRegToStack, {stackStorage, isaReg});
  }
  map = {stackStorage};
};

auto BlockAllocator::getFreeReg(const MIROperand& operand,
                                std::unordered_set<MIROperand, MIROperandHasher>& protect) {
  const auto& isaRegHint = allocateCtx.isaRegHint;
  const auto regClass = ctx.registerInfo->getAllocationClass(operand.type());
  auto& q = allocationQueue[regClass];
  MIROperand isaReg;

  const auto getFreeRegister = [&] {
    std::vector<MIROperand> temp;
    do {
      auto reg = selector.getFreeRegister(operand.type());
      if (reg.isUnused()) {
        for (auto op : temp)
          selector.markAsDiscarded(op);
        return MIROperand{};
      }
      if (isProtected(reg, protect)) {
        temp.push_back(reg);
        selector.markAsUsed(reg);
      } else {
        for (auto op : temp)
          selector.markAsDiscarded(op);
        return reg;
      }
    } while (true);
  };

  if (auto hintIter = isaRegHint.find(operand); hintIter != isaRegHint.end() &&
                                                selector.isFree(hintIter->second) &&
                                                !isProtected(hintIter->second, protect)) {
    isaReg = hintIter->second;
  } else if (auto reg = getFreeRegister(); !reg.isUnused()) {
    isaReg = reg;
  } else {
    // evict
    assert(!q.empty());
    isaReg = q.front();
    while (isProtected(isaReg, protect)) {
      assert(q.size() != 1);
      q.pop();
      q.push(isaReg);
      isaReg = q.front();
    }
    q.pop();
    selector.markAsDiscarded(isaReg);
  }
  if (auto it = physMap.find(isaReg); it != physMap.cend()) {
    evictVReg(it->second);
  }
  assert(!isProtected(isaReg, protect));

  // std::cerr << (operand.reg() - virtualRegBegin) << " -> " <<
  // isaReg.reg() << std::endl;

  q.push(isaReg);
  physMap[isaReg] = operand;
  selector.markAsUsed(isaReg);
  return isaReg;
};

const auto BlockAllocator::use(MIROperand& op,
                               std::unordered_set<MIROperand, MIROperandHasher>& protect,
                               std::unordered_set<MIROperand, MIROperandHasher>& releaseVRegs) {
  if (!isOperandVReg(op)) {
    if (isOperandISAReg(op) && !ctx.registerInfo->is_zero_reg(op.reg()) &&
        isAllocatableType(op.type())) {
      underRenamedISAReg.erase(op);
    }
    return;
  }
  if (op.reg_flag() & RegisterFlagDead) {
    releaseVRegs.insert(op);
  }

  auto& map = getDataMap(op);
  MIROperand stackStorage;
  for (auto& reg : map) {
    if (!isStackObject(reg.reg())) {
      // loaded
      op = reg;
      protect.insert(reg);
      return;
    }
    stackStorage = reg;
  }
  // load from stack
  assert(!stackStorage.isUnused());
  const auto reg = getFreeReg(op, protect);
  insertMIRInst(InstLoadRegFromStack, {reg, stackStorage});

  map.push_back(reg);
  op = reg;
  protect.insert(reg);
};

const auto BlockAllocator::def(MIROperand& op,
                               std::unordered_set<MIROperand, MIROperandHasher>& protect) {
  const auto& stackMap = allocateCtx.stackMap;
  if (!isOperandVReg(op)) {
    if (isOperandISAReg(op) && !ctx.registerInfo->is_zero_reg(op.reg()) &&
        isAllocatableType(op.type())) {
      protectedLockedISAReg.insert(op);
      if (auto it = physMap.find(op); it != physMap.cend()) {
        evictVReg(it->second);
      }
    }
    return;
  }

  if (stackMap.count(op)) {
    dirtyVRegs.insert(op);
  }

  auto& map = getDataMap(op);
  MIROperand stackStorage;
  for (auto& reg : map) {
    if (!isStackObject(reg.reg())) {
      op = reg;
      map = {reg};  // mark other storage dirty
      protect.insert(reg);
      return;
    }
    stackStorage = reg;
  }
  const auto reg = getFreeReg(op, protect);
  map = {reg};
  protect.insert(reg);
  op = reg;
};

const auto BlockAllocator::spillBeforeBranch(MIRInst* inst) {
  assert(requireFlag(ctx.instInfo.getInstInfo(inst).inst_flag(), InstFlagBranch));
  // write back all out dirty vregs into stack slots before branch
  for (auto dirty : dirtyVRegs)
    if (liveIntervalInfo.outs.count(dirty.reg())) evictVReg(dirty);
}

const auto BlockAllocator::saveCallerSavedRegsForCall(MIRInst* inst) {
  assert(requireFlag(ctx.instInfo.getInstInfo(inst).inst_flag(), InstFlagCall));
  std::vector<MIROperand> savedVRegs;
  const IPRAInfo* calleeUsage = nullptr;
  if (auto symbol = inst->operand(0).reloc()) {
    calleeUsage = allocateCtx.infoIPRA.query(symbol->name());
  }
  // check all allocated (phyreg, verg) pairs in current block,
  // if used in callee, save them to stack
  for (auto& [p, v] : physMap) {
    if (ctx.frameInfo.isCallerSaved(p)) {
      if (calleeUsage && !calleeUsage->count(p.reg())) continue;
      savedVRegs.push_back(v);
    }
  }
  // spill saved vregs to stack
  for (auto v : savedVRegs)
    evictVReg(v);
  protectedLockedISAReg.clear();
}

bool allocateInBlock(MIRBlock& block, FastAllocatorContext& allocateCtx) {
  /* 局部寄存器分配 -- 考虑在每个块内对其进行单独的分析 */
  auto& ctx = allocateCtx.ctx;
  auto blockAllocator =
    BlockAllocator(allocateCtx, &block, allocateCtx.liveInterval.block2Info.at(&block));
  auto& insts = block.insts();
  // insts.insert()
  blockAllocator.collectUnderRenamedISARegs(insts.begin());

  for (auto iter = insts.begin(); iter != insts.end();) {
    blockAllocator.setInsertPoint(iter);
    const auto next = std::next(iter);
    // used or defined ISA reg in inst
    std::unordered_set<MIROperand, MIROperandHasher> protect;
    // dead vregs to release in inst
    std::unordered_set<MIROperand, MIROperandHasher> releaseVRegs;

    auto inst = *iter;
    auto& instInfo = ctx.instInfo.getInstInfo(inst);
    // use or def IAS reg, add to protect set
    for (size_t idx = 0; idx < instInfo.operand_num(); ++idx) {
      auto flag = instInfo.operand_flag(idx);
      if ((flag & OperandFlagUse) || (flag & OperandFlagDef)) {
        const auto& op = inst->operand(idx);
        if (!isOperandVReg(op) and isOperandISAReg(op)) protect.insert(op);
      }
    }
    // use xreg, add use info
    for (size_t idx = 0; idx < instInfo.operand_num(); ++idx)
      if (instInfo.operand_flag(idx) & OperandFlagUse)
        blockAllocator.use(inst->operand(idx), protect, releaseVRegs);

    // call inst
    if (requireFlag(instInfo.inst_flag(), InstFlagCall)) {
      blockAllocator.saveCallerSavedRegsForCall(inst);
      blockAllocator.collectUnderRenamedISARegs(next);
    }

    protect.clear();

    // release dead vregs
    for (auto operand : releaseVRegs) {
      auto& map = blockAllocator.getDataMap(operand);
      for (auto& reg : map)
        if (isISAReg(reg.reg())) {
          blockAllocator.physMap.erase(reg);
          blockAllocator.selector.markAsDiscarded(reg);
        }
      map.clear();
    }

    for (size_t idx = 0; idx < instInfo.operand_num(); ++idx)
      if (instInfo.operand_flag(idx) & OperandFlagDef)
        blockAllocator.def(inst->operand(idx), protect);

    if (requireFlag(instInfo.inst_flag(), InstFlagBranch)) {
      blockAllocator.spillBeforeBranch(inst);
    }

    iter = next;
  }

  assert(block.verify(std::cerr, ctx));
  return true;
}

void intraBlockAllocate(MIRFunction& mfunc, CodeGenContext& ctx, IPRAUsageCache& infoIPRA) {
  auto liveInterval = calcLiveIntervals(mfunc, ctx);
  // mfunc.print(std::cerr, ctx);
  auto allocateCtx = FastAllocatorContext{mfunc, ctx, liveInterval, infoIPRA};

  allocateCtx.collectUseDefInfo(mfunc, ctx);
  allocateCtx.collectStackMap(mfunc, ctx);

  for (auto& block : mfunc.blocks()) {
    allocateInBlock(*block, allocateCtx);
  }
  // mfunc.print(std::cerr, ctx);
}

}  // namespace mir
