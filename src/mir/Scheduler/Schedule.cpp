

#include "mir/utils.hpp"
#include "mir/ScheduleModel.hpp"

#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace mir {
bool BlockScheduleContext::celloctInfo(MIRBlock& block, const CodeGenContext& ctx) {
#ifdef DEBUG
  std::cerr << "Scheduling block: " << block.name() << std::endl;
#endif
  auto dumpInst = [&](MIRInst* inst, std::ostream& os) {
    auto& instInfo = ctx.instInfo.getInstInfo(inst);
    instInfo.print(os, *inst, true);
  };
  /** insts that 'touch'(use/def) the register: reg -> insts
   * lastTouch[i]: {def, use, use, ...}
   */
  std::unordered_map<uint32_t, std::vector<MIRInst*>> lastTouch;

  /* the last inst define a register: reg -> inst */
  std::unordered_map<uint32_t, MIRInst*> lastDef;

  /** u depends on v , v anti-depends on u
   * add u to antiDeps[v] and increment degrees[u]
   */
  auto addDep = [&](MIRInst* u, MIRInst* v) {
    if (u == v) return;
    if (antiDeps[v].insert(u).second) {
      ++degrees[u];
    }
#ifdef DEBUG
    dumpInst(u, std::cerr);
    std::cerr << "-> depends on -> ";
    dumpInst(v, std::cerr);
    std::cerr << std::endl;
#endif
  };
  /**
   *
   * iy: add a[Def], b[Use], c[Use]
   * iz: add b[Def], x[Use], c[Use]
   * i1: sub a[Def], b[Use], c[Use]
   * i2: add d[Def], a[Use], b[Use]
   * i3: mul b[Def], d[Use], e[Use]
   * - i2 depends on i1 (i1 anti-depends on i2), addDep(i2, i1)
   * - WAR, i1, i2 anti-depends on i3, addDep(i3, i1), addDep(i3, i2)
   * - when i3: lastTouch[b] = {iz, i1, i2}, except iy
   */
  MIRInst* lastSideEffect = nullptr;
  MIRInst* lastInOrder = nullptr;
  for (auto& inst : block.insts()) {
    auto& instInfo = ctx.instInfo.getInstInfo(inst);
    /* for all operands */
    for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx) {
      auto op = inst->operand(idx);
      auto opflag = instInfo.operand_flag(idx);
      if (op.isReg()) {
        /** before stack allocate, after sa, sobj is replaced by reg */
        if (isOperandStackObject(op)) {
          op = ctx.registerInfo->get_stack_pointer_register();
          opflag = OperandFlagUse;
        }
        const auto reg = op.reg();
        renameMap[inst][idx] = reg;

        if (opflag & OperandFlagUse) {
          /* RAW: read after write (use after def) */
          if (auto it = lastDef.find(reg); it != lastDef.end()) {
            addDep(inst, it->second);
          }
          lastTouch[reg].push_back(inst);
        }
      }
    }  // end for all operands

    for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx) {
      auto op = inst->operand(idx);
      auto opflag = instInfo.operand_flag(idx);
      if (op.isReg()) {
        /** before stack allocate, after sa, sobj is replaced by reg */
        if (isOperandStackObject(op)) {
          op = ctx.registerInfo->get_stack_pointer_register();
        }
        const auto reg = op.reg();
        if (opflag & OperandFlagDef) {
          /** WAR: write after read (def after use)
           * use anti-depends on def, def depends on use,
           * addDep(def, use)
           * must execute 'use' inst before 'def' inst
           */
          for (auto use : lastTouch[reg]) {
            addDep(inst, use);
          }
          lastTouch[reg] = {inst};
          lastDef[reg] = inst;
        }
      }
    }  // end for all operands
    if (lastInOrder) {
      addDep(inst, lastInOrder);
    }

    /** SideEffect Inst */
    /**
     * store r1[Use], imm[Metadata](r2[Use]): SideEffect
     * add r1[Def], r2[USe], r3[Use]
     * call fxx: SideEffect, InOrder, Call, Terminator
     */
    if (requireOneFlag(instInfo.inst_flag(), InstFlagSideEffect)) {
      /** this SideEffect inst depends on the last SideEffect inst */
      if (lastSideEffect) {
        addDep(inst, lastSideEffect);
      }
      lastSideEffect = inst;
      if (requireOneFlag(instInfo.inst_flag(),
                         InstFlagInOrder | InstFlagCall | InstFlagTerminator)) {
        /** sideeffect and inorder inst,
         * then the inst dependes on all previous insts
         * this inst must execute after all previous insts
         * */
        for (auto& prev : block.insts()) {
          if (prev == inst) break;
          addDep(inst, prev);
        }
        lastInOrder = inst;
      }
    }  // end if SideEffect Inst
  }  // end for each inst

  auto dumpDebug = [&](std::ostream& os) {
    os << "block: " << block.name() << std::endl;
    for (auto inst : block.insts()) {
      auto& instInfo = ctx.instInfo.getInstInfo(inst);
      os << "[" << instInfo.name() << "] ";
      instInfo.print(os, *inst, false);
      os << std::endl;
      os << "- rank: " << rank[inst] << std::endl;
      if (auto it = degrees.find(inst); it != degrees.end()) {
        os << "- degree: " << it->second << std::endl;
      } else {
        os << "- degree: 0" << std::endl;
      }
      os << "- antiDeps: \n";
      for (auto target : antiDeps[inst]) {
        auto& targetInfo = ctx.instInfo.getInstInfo(target);
        targetInfo.print(os << "  - ", *target, false);
        os << std::endl;
      }
      os << "- renameMap: ";
      for (auto [idx, renamedReg] : renameMap[inst]) {
        os << " " << idx << "->" << renamedReg << ", ";
      }
      os << std::endl;
    }
  };
#ifdef DEBUG
  dumpDebug(std::cerr);
#endif
  return true;
}

uint32_t ScheduleState::queryRegisterLatency(const MIRInst& inst, uint32_t idx) {
  /* 查询寄存器延迟 */

  if (not inst.operand(idx).isReg()) return 0;
#ifdef DEBUG
  std::cerr << "mcycle: " << mCycleCount << ", query: " << idx << ": ";
#endif
  auto reg = mRegRenameMap.at(&inst).at(idx);
  if (auto iter = mRegisterAvailableTime.find(reg); iter != mRegisterAvailableTime.end()) {
#ifdef DEBUG
    std::cerr << "av: " << iter->second << ", la: " << iter->second - mCycleCount << std::endl;
#endif
    if (iter->second > mCycleCount) {
      return iter->second - mCycleCount;
    }
  }
  return 0;
}
bool ScheduleState::isPipelineReady(uint32_t pipelineId) {
  if (auto iter = mNextPipelineAvailable.find(pipelineId); iter != mNextPipelineAvailable.end()) {
    return iter->second <= mCycleCount;
  }
  return true;
}
bool ScheduleState::isAvailable(uint32_t mask) {
  /* check issued flag with mask (can issue bits) */
  return (mIssuedFlag & mask) != mask;
}
// issue
void ScheduleState::setIssued(uint32_t mask) {
  mIssuedFlag |= mask;
}
void ScheduleState::resetPipeline(uint32_t pipelineId, uint32_t duration) {
  /* 更新 mNextPipelineAvailable，
  ** 将指定流水线的下一次可用时间设置为当前周期加上持续时间。*/
  mNextPipelineAvailable[pipelineId] = mCycleCount + duration;
}
void ScheduleState::makeRegisterReady(const MIRInst& inst, uint32_t idx, uint32_t latency) {
  /* 更新 mRegisterAvailableTime，
  ** 将指定寄存器的可用时间设置为当前周期加上延迟。 */
  auto renamedReg = mRegRenameMap.at(&inst).at(idx);
  mRegisterAvailableTime[renamedReg] = mCycleCount + latency;
}

}  // namespace mir
