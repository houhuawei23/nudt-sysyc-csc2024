#include "ir/ir.hpp"
#include "mir/MIR.hpp"
#include "mir/target.hpp"
#include "mir/instinfo.hpp"
#include "mir/iselinfo.hpp"
#include "mir/utils.hpp"
#include <optional>

namespace mir {

auto collectDefCount(MIRFunction* func, CodeGenContext& ctx) {
  std::unordered_map<MIROperand, uint32_t, MIROperandHasher> defCount;
  for (auto& block : func->blocks()) {
    auto& insts = block->insts();
    for (auto& inst : insts) {
      auto& instInfo = ctx.instInfo.getInstInfo(inst);
      if (requireFlag(instInfo.inst_flag(), InstFlagLoadConstant)) {
        // load constant
        auto& dst = inst->operand(0);
        if (isOperandVReg(dst)) {
          ++defCount[dst];
        }
      } else {
        // other insts
        for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx)
          if (instInfo.operand_flag(idx) & OperandFlagDef) {
            auto& def = inst->operand(idx);
            if (isOperandVReg(def)) {
              ++defCount[def];
            }
          }
      }
    }
  }
  return std::move(defCount);
}
auto collectUseCount(MIRFunction* func, CodeGenContext& ctx) {
  std::unordered_map<MIROperand, uint32_t, MIROperandHasher> useCount;
  for (auto& block : func->blocks()) {
    auto& insts = block->insts();
    for (auto& inst : insts) {
      auto& instInfo = ctx.instInfo.getInstInfo(inst);
      for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx) {
        if (instInfo.operand_flag(idx) & OperandFlagUse) {
          auto& use = inst->operand(idx);
          if (isOperandVReg(use)) {
            ++useCount[use];
          }
        }
      }
    }
  }
  return std::move(useCount);
}

bool ISelContext::hasOneUse(MIROperand op) {
  if (auto iter = mUseCnt.find(op); iter != mUseCnt.end()) {
    return iter->second == 1;
  }
  return true;
}

void ISelContext::remove_inst(MIRInst* inst) {
  assert(inst != nullptr);
  mRemoveWorkList.insert(inst);
}
void ISelContext::replace_operand(MIROperand src, MIROperand dst) {
  assert(src.isReg());
  if (src != dst) {
    mReplaceMap.emplace(src, dst);
  }
}
MIROperand& ISelContext::getInstDefOperand(MIRInst* inst) {
  assert(inst != nullptr);
  auto& instinfo = mCodeGenCtx.instInfo.getInstInfo(inst);
  for (uint32_t idx = 0; idx < instinfo.operand_num(); idx++) {
    if (instinfo.operand_flag(idx) & OperandFlagDef) {
      return inst->operand(idx);
    }
  }
  assert(false && "no def operand found");
  std::cerr << "no def operand found" << std::endl;
}

MIRInst* ISelContext::lookupDefInst(const MIROperand& op) const {
  assert(op.isInit());
  // const auto iter = mDefinedInst.find(op);
  if (const auto iter = mDefinedInst.find(op); iter != mDefinedInst.end()) {
    return iter->second;
  }
  if (const auto iter = mConstantMap.find(op); iter != mConstantMap.end()) {
    return iter->second;
  }

  // std::cerr << "def not found: " << op << std::endl;
  return nullptr;

  // std::cerr << "op address " << op << "\n";
  if (isOperandVReg(op)) {
    std::cerr << "virtual reg v" << (op.reg() ^ virtualRegBegin) << "\n";
  } else if (isOperandISAReg(op)) {
    std::cerr << "physical reg i" << op.reg() << "\n";
  } else {
    std::cerr << "satck\n";
  }

  assert(false && "def not found");
}
//! 定义和使用计数收集: 遍历所有指令，收集每个定义的计数和使用情况。
// get def count
void ISelContext::calConstantMap(MIRFunction* func) {
  auto defCount = collectDefCount(func, mCodeGenCtx);
  for (auto& block : func->blocks()) {
    for (auto& inst : block->insts()) {
      // for all insts
      auto& instinfo = mCodeGenCtx.instInfo.getInstInfo(inst);
      if (requireFlag(instinfo.inst_flag(), InstFlagLoadConstant)) {
        // load constant, and def once, can view as constant
        auto& def = getInstDefOperand(inst);
        if (isOperandVReg(def) and defCount[def] <= 1) {
          mConstantMap.emplace(def, inst);
        }
      }
    }
  }

  // std::cerr << "constant map size: " << mConstantMap.size() << std::endl;
}
void ISelContext::collectDefinedInst(MIRBlock* block) {
  for (auto& inst : block->insts()) {
    auto& instinfo = mCodeGenCtx.instInfo.getInstInfo(inst);
    for (uint32_t idx = 0; idx < instinfo.operand_num(); idx++) {
      if (instinfo.operand_flag(idx) & OperandFlagDef) {
        auto def = inst->operand(idx);
        if (def.isReg() && isVirtualReg(def.reg())) {
          mDefinedInst.emplace(def, inst);
        }
      }
    }
  }
}
template <typename Func>
void traverseBlocks(MIRFunction& func,
                    Func funcBlock,
                    std::ostream& os = std::cerr,
                    bool reverse = false,
                    bool debug = false) {
  for (auto& block : func.blocks()) {
    if (debug) {
      os << "Traversing block: " << block->name() << std::endl;
    }
    funcBlock(block.get());
  }
}

template <typename Func>
void traverseInsts(MIRBlock* block,
                   CodeGenContext& ctx,
                   Func funcInst,
                   std::ostream& os = std::cerr,
                   bool debug = false) {
  for (auto& inst : block->insts()) {
    if (debug) {
      const auto& instInfo = ctx.instInfo.getInstInfo(inst);
      instInfo.print(os << "Traversing inst: ", *inst, false);
      os << std::endl;
    }
    funcInst(inst);
  }
}

bool ISelContext::runInstSelectImpl(MIRFunction* func) {
  auto dumpInst = [&](MIRInst* inst) {
    auto& instInfo = mCodeGenCtx.instInfo.getInstInfo(inst);
    instInfo.print(std::cerr << "match&select: ", *inst, false);
    std::cerr << std::endl;
  };
  constexpr bool debugISel = false;

  auto& isel_info = mCodeGenCtx.iselInfo;
  genericPeepholeOpt(*func, mCodeGenCtx);

  bool modified = false;
  bool has_illegal = false;
  clearInfo();
  calConstantMap(func);
  mUseCnt = collectUseCount(func, mCodeGenCtx);

  //! 指令遍历和分析: 对每个基本块的指令进行遍历，执行指令选择和替换。
  auto matchSelectOnBlock = [&](MIRBlock* block) {
    collectDefinedInst(block);

    mCurrBlock = block;
    auto& insts = block->insts();

    for (auto it = insts.rbegin(); it != insts.rend(); ++it) {
      // Convert reverse iterator to normal iterator
      mInsertPoint = std::prev(it.base());
      auto& inst = *it;

      if (mRemoveWorkList.count(inst)) continue;

      if (debugISel) dumpInst(inst);

      if (isel_info->match_select(inst, *this)) {
        modified = true;
      }
    }
  };

  //! 指令移除和替换: 根据之前的分析结果，移除和替换旧的指令。
  auto removeAndReplaceOnBlock = [&](MIRBlock* block) {
    // remove old insts
    block->insts().remove_if([&](auto inst) { return mRemoveWorkList.count(inst); });

    // replace defs
    for (auto& inst : block->insts()) {
      if (mReplaceBlockList.count(inst)) {
        // in replace block list, jump
        continue;
      }
      auto& info = mCodeGenCtx.instInfo.getInstInfo(inst);

      for (uint32_t idx = 0; idx < info.operand_num(); idx++) {
        auto op = inst->operand(idx);
        if (!op.isReg()) continue;
        if (auto iter = mReplaceMap.find(op); iter != mReplaceMap.end()) {
          inst->set_operand(idx, iter->second);
        }
      }
    }
  };

  traverseBlocks(*func, matchSelectOnBlock, std::cerr, false, false);
  traverseBlocks(*func, removeAndReplaceOnBlock, std::cerr, false, false);
  return modified;
}

void ISelContext::runInstSelect(MIRFunction* func) {
  //! fix point algorithm: 循环执行指令选择和替换，直到不再变化。
  while (runInstSelectImpl(func))
    ;
}

uint32_t select_copy_opcode(MIROperand dst, MIROperand src) {
  if (dst.isReg() && isISAReg(dst.reg())) {
    /* NOTE: dst is a ISAReg */
    if (src.isImm()) return InstLoadImmToReg;
    return InstCopyToReg;
  }
  if (src.isImm()) return InstLoadImmToReg;
  if (src.isReg() && isISAReg(src.reg())) return InstCopyFromReg;
  assert(isOperandVRegORISAReg(src) && isOperandVRegORISAReg(dst));
  return InstCopy;
}

void postLegalizeFunc(MIRFunction& func, CodeGenContext& ctx) {
  /* legalize stack operands */
  for (auto& block : func.blocks()) {
    auto& insts = block->insts();
    for (auto it = insts.begin(); it != insts.end();) {
      auto next = std::next(it);
      auto& inst = *it;
      auto& info = ctx.instInfo.getInstInfo(inst);
      for (uint32_t idx = 0; idx < info.operand_num(); idx++) {
        auto op = inst->operand(idx);
        if (isOperandStackObject(op)) {
          if (func.stackObjs().find(op) == func.stackObjs().end()) {
            std::cerr << "stack object not found in function " << func.name() << "\n";
            std::cerr << "stack object so" << GENERIC::OperandDumper{op} << "\n";
            std::cerr << "instruction: ";
            info.print(std::cerr, *inst, false);
            std::cerr << "\n";
            assert(false);
          }
          ctx.iselInfo->legalizeInstWithStackOperand(
            InstLegalizeContext{inst, insts, it, ctx, std::nullopt, func}, op,
            func.stackObjs().at(op));
        }
      }
      it = next;
    }
  }

  /* iselInfo postLegaliseInst */
  for (auto& block : func.blocks()) {
    auto& insts = block->insts();
    for (auto iter = insts.begin(); iter != insts.end(); iter++) {
      auto& inst = *iter;
      if (inst->opcode() < ISASpecificBegin) {
        ctx.iselInfo->postLegalizeInst(
          InstLegalizeContext{inst, insts, iter, ctx, std::nullopt, func});
      }
    }
  }

  ctx.target.postLegalizeFunc(func, ctx);
}

}  // namespace mir
