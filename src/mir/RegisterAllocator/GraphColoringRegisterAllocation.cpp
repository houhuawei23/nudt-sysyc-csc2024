// #define DEBUG

#include "mir/MIR.hpp"
#include "mir/target.hpp"
#include "mir/CFGAnalysis.hpp"
#include "mir/LiveInterval.hpp"
#include "mir/RegisterAllocator.hpp"
#include "support/StaticReflection.hpp"
#include "target/riscv/RISCV.hpp"
#include <vector>
#include <stack>
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <optional>

namespace mir {

/*
 * @brief: Graph Coloring Register Allocation (图着色寄存器分配算法)
 * @param:
 *      1. MIRFunction& mfunc
 *      2. CodeGenContext& ctx
 *      3. IPRAUsageCache& infoIPRA
 *      4. uint32_t allocationClass (决定此次分配是分配整数寄存器还是浮点寄存器)
 *      5. std::unordered_map<uint32_t, uint32_t>& regMap
 *          (answer,存储虚拟寄存器到物理寄存器之间的映射)
 */

void GraphColoringAllocateContext::colorDefUse(RegNum src, RegNum dst) {
  assert(isVirtualReg(src) && isISAReg(dst));
  if (!fixHazard || !defUseTime.count(src)) return;
  auto& dstInfo = defUseTime[dst];
  auto& srcInfo = defUseTime[src];
  dstInfo.insert(srcInfo.begin(), srcInfo.end());
};
void GraphColoringAllocateContext::updateCopyHint(RegNum dst, RegNum src, double weight) {
  if (isVirtualReg(dst)) {
    copyHint[dst][src] += weight;
  }
};
bool GraphColoringAllocateContext::collectInStackArgumentsRegisters(MIRFunction& mfunc,
                                                                    CodeGenContext& ctx) {
  for (auto& inst : mfunc.blocks().front()->insts()) {
    // entry block
    // mfunc arguments in stack
    if (inst->opcode() == InstLoadRegFromStack) {
      const auto dst = inst->operand(0).reg();
      const auto src = inst->operand(1);
      const auto& obj = mfunc.stackObjs().at(src);
      if (obj.usage == StackObjectUsage::Argument) {
        inStackArguments.emplace(dst, src);
      }
    }
  }
  return true;
}
bool GraphColoringAllocateContext::collectConstantsRegisters(MIRFunction& mfunc,
                                                             CodeGenContext& ctx) {
  for (auto& block : mfunc.blocks()) {
    for (auto& inst : block->insts()) {
      const auto& instInfo = ctx.instInfo.getInstInfo(inst);

      if (requireFlag(instInfo.inst_flag(), InstFlagLoadConstant)) {
        const auto reg = inst->operand(0).reg();
        if (isVirtualReg(reg)) {
          if (!constants.count(reg)) {
            // this reg first load by constant, add to map
            constants[reg] = inst;
          } else {
            // this reg is loaded by another constant, multi define, remove from map
            constants[reg] = nullptr;
          }
        }
      } else {
        // not load constant
        for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx) {
          const auto opflag = instInfo.operand_flag(idx);
          if (not(opflag & OperandFlagDef)) continue;
          // operand is defined by non-constant instruction, remove from map
          auto& op = inst->operand(idx);
          if (isOperandVReg(op)) {
            constants[op.reg()] = nullptr;
          }
        }
      }
    }
  }
  {
    std::vector<uint32_t> eraseKey;
    for (auto [reg, inst] : constants) {
      if (!inst) eraseKey.push_back(reg);
    }
    for (auto reg : eraseKey)
      constants.erase(reg);
  }

  return true;
}

std::unordered_set<RegNum> GraphColoringAllocateContext::collectVirtualRegs(MIRFunction& mfunc,
                                                                            CodeGenContext& ctx) {
  std::unordered_set<RegNum> vregSet;
  for (auto& block : mfunc.blocks()) {
    for (auto& inst : block->insts()) {
      auto& instInfo = ctx.instInfo.getInstInfo(inst);
      for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx) {
        const auto flag = instInfo.operand_flag(idx);
        if (!((flag & OperandFlagUse) || (flag & OperandFlagDef))) continue;
        auto& op = inst->operand(idx);
        if (!(isOperandVReg(op) && isAllocatableType(op.type(), ctx))) continue;
        vregSet.insert(op.reg());
      }
    }
  }
  return std::move(vregSet);
}

InterferenceGraph GraphColoringAllocateContext::buildGraph(
  MIRFunction& mfunc,
  CodeGenContext& ctx,
  const LiveVariablesInfo& liveInterval,
  const std::unordered_set<RegNum>& vregSet,
  const BlockTripCountResult& blockFreq) {
  InterferenceGraph graph;
  // ISA specific reg
  for (auto& block : mfunc.blocks()) {
    auto& instructions = block->insts();
    std::unordered_set<uint32_t> underRenamedISAReg;
    std::unordered_set<uint32_t> lockedISAReg;

    const auto collectUnderRenamedISARegs = [&](MIRInstList::iterator it) {
      while (it != instructions.end()) {
        const auto& inst = *it;
        auto& instInfo = ctx.instInfo.getInstInfo(inst);
        bool hasReg = false;
        for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx) {
          const auto& op = inst->operand(idx);
          if (isOperandISAReg(op) && !ctx.registerInfo->is_zero_reg(op.reg()) &&
              isLockedOrUnderRenamedType(op.type()) &&
              (instInfo.operand_flag(idx) & OperandFlagUse)) {
            if (isAllocatableType(op.type(), ctx)) underRenamedISAReg.insert(op.reg());
            hasReg = true;
          }
        }
        if (hasReg)
          ++it;
        else
          break;
      }
    };

    collectUnderRenamedISARegs(instructions.begin());
    std::unordered_set<uint32_t> liveVRegs;
    for (auto vreg : liveInterval.block2Info.at(block.get()).ins) {
      assert(isVirtualReg(vreg));
      if (vregSet.count(vreg)) liveVRegs.insert(vreg);
    }
    const auto tripCount = blockFreq.query(block.get());
    for (auto iter = instructions.begin(); iter != instructions.end();) {
      const auto next = std::next(iter);
      auto& inst = *iter;
      auto& instInfo = ctx.instInfo.getInstInfo(inst);
      if (inst->opcode() == InstCopyFromReg && allocableISARegs.count(inst->operand(1).reg())) {
        updateCopyHint(inst->operand(0).reg(), inst->operand(1).reg(), tripCount);
      } else if (inst->opcode() == InstCopyToReg &&
                 allocableISARegs.count(inst->operand(0).reg())) {
        updateCopyHint(inst->operand(1).reg(), inst->operand(0).reg(), tripCount);
      } else if (inst->opcode() == InstCopy) {
        const auto u = inst->operand(0).reg();
        const auto v = inst->operand(1).reg();
        if (u != v) {
          if (isVirtualReg(u)) updateCopyHint(u, v, tripCount);
          if (isVirtualReg(v)) updateCopyHint(v, u, tripCount);
        }
      }

      for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx) {
        if (instInfo.operand_flag(idx) & OperandFlagUse) {
          auto& op = inst->operand(idx);
          if (!isAllocatableType(op.type(), ctx)) continue;
          if (!isOperandVRegORISAReg(op)) continue;
          defUseTime[op.reg()].insert(liveInterval.inst2Num.at(inst));
          if (isOperandISAReg(op) && !ctx.registerInfo->is_zero_reg(op.reg())) {
            underRenamedISAReg.erase(op.reg());
          } else if (isOperandVReg(op)) {
            graph.create(op.reg());
            if (op.reg_flag() & RegisterFlagDead) liveVRegs.erase(op.reg());
          }
        }
      }
      if (requireFlag(instInfo.inst_flag(), InstFlagCall)) {
        const IPRAInfo* calleeUsage = nullptr;
        if (auto symbol = inst->operand(0).reloc()) {
          calleeUsage = infoIPRA.query(symbol->name());
        }

        if (calleeUsage) {
          for (auto isaReg : *calleeUsage)
            if (isAllocatableType(ctx.registerInfo->getCanonicalizedRegisterType(isaReg), ctx)) {
              for (auto vreg : liveVRegs)
                graph.add_edge(vreg, isaReg);
            }
        } else {
          for (auto isaReg : ctx.registerInfo->get_allocation_list(allocationClass))
            if (ctx.frameInfo.isCallerSaved(MIROperand::asISAReg(isaReg, OperandType::Special))) {
              for (auto vreg : liveVRegs)
                graph.add_edge(isaReg, vreg);
            }
        }

        collectUnderRenamedISARegs(next);
        lockedISAReg.clear();
      }
      for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx) {
        if (instInfo.operand_flag(idx) & OperandFlagDef) {
          auto& op = inst->operand(idx);
          if (!isAllocatableType(op.type(), ctx)) continue;
          defUseTime[op.reg()].insert(liveInterval.inst2Num.at(inst));
          if (isOperandISAReg(op) && !ctx.registerInfo->is_zero_reg(op.reg())) {
            lockedISAReg.insert(op.reg());
            for (auto vreg : liveVRegs)
              graph.add_edge(vreg, op.reg());
          } else if (isOperandVReg(op)) {
            liveVRegs.insert(op.reg());
            graph.create(op.reg());
            for (auto isaReg : underRenamedISAReg)
              graph.add_edge(op.reg(), isaReg);
            for (auto isaReg : lockedISAReg)
              graph.add_edge(op.reg(), isaReg);
          }
        }
      }

      iter = next;
    }
  }
  auto vregs = graph.collect_nodes();
  assert(vregs.size() == vregSet.size());

  for (size_t i = 0; i < vregs.size(); ++i) {
    auto u = vregs[i];
    auto& intervalU = liveInterval.reg2Interval.at(u);
    for (size_t j = i + 1; j < vregs.size(); ++j) {
      auto& v = vregs[j];
      auto& intervalV = liveInterval.reg2Interval.at(v);
      if (intervalU.intersectWith(intervalV)) {
        graph.add_edge(u, v);
      }
    }
  }
  return std::move(graph);
}

RegWeightMap GraphColoringAllocateContext::computeRegWeight(
  MIRFunction& mfunc,
  CodeGenContext& ctx,
  std::vector<uint32_t>& vregs,
  BlockTripCountResult& blockFreq,
  LiveVariablesInfo& liveInterval,
  std::vector<std::pair<InstNum, double>>& freq) {
  RegWeightMap weights;
  auto getBlockFreq = [&](InstNum inst) {
    const auto it = std::lower_bound(freq.begin(), freq.end(), inst,
                                     [](const auto& a, const auto& b) { return a.first < b; });
    assert(it != freq.end());
    return it->second;
  };
  for (auto vreg : vregs) {
    auto& liveRange = liveInterval.reg2Interval.at(vreg);
    double weight = 0;
    for (auto& [beg, end] : liveRange.segments) {
      weight += static_cast<double>(end - beg) * getBlockFreq(end);
    }
    if (auto iter = copyHint.find(vreg); iter != copyHint.end())
      weight += 100.0 * static_cast<double>(iter->second.size());
    if (constants.count(vreg)) weight -= 1.0;
    weights.emplace(vreg, weight);
  }
  for (auto& block : mfunc.blocks()) {
    const auto w = blockFreq.query(block.get());
    for (auto& inst : block->insts()) {
      auto& instInfo = ctx.instInfo.getInstInfo(inst);
      for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx) {
        const auto flag = instInfo.operand_flag(idx);
        if (!((flag & OperandFlagUse) || (flag & OperandFlagDef))) continue;
        auto& op = inst->operand(idx);
        if (!(isOperandVReg(op) && isAllocatableType(op.type(), ctx))) continue;
        weights[op.reg()] += w;
      }
    }
  }
  return std::move(weights);
}

bool GraphColoringAllocateContext::assignRegisters(MIRFunction& mfunc,
                                                   CodeGenContext& ctx,
                                                   const InterferenceGraph& graph,
                                                   const RegWeightMap& weights,
                                                   std::stack<uint32_t>& assignStack) {
  // try to assign registers, if failed, spill registers
  const auto k = static_cast<uint32_t>(regCount);
  bool spillRegister = false;
  auto dynamicGraph = graph;
  dynamicGraph.prepare_for_assign(weights, k);
  while (!dynamicGraph.empty()) {
    auto u = dynamicGraph.pick_to_assign(k);
    if (u == invalidReg) {
      spillRegister = true;
      break;
    }
#ifdef DEBUG
    std::cerr << "push: " << dumpVirtualReg(u) << std::endl;
#endif
    assignStack.push(u);
  }
  return spillRegister;
}

bool GraphColoringAllocateContext::allocateRegisters(
  MIRFunction& mfunc,
  CodeGenContext& ctx,
  std::vector<uint32_t>& vregs,
  std::stack<uint32_t>& assignStack,
  InterferenceGraph& graph,
  std::vector<std::pair<InstNum, double>>& freq) {
  const auto& list = ctx.registerInfo->get_allocation_list(allocationClass);
  auto getBlockFreq = [&](InstNum inst) {
    const auto it = std::lower_bound(freq.begin(), freq.end(), inst,
                                     [](const auto& a, const auto& b) { return a.first < b; });
    assert(it != freq.end());
    return it->second;
  };
  const auto calcCopyFreeProposal =
    [&](RegNum u, std::unordered_set<uint32_t>& exclude) -> std::optional<RegNum> {
    auto iter = copyHint.find(u);
    if (iter == copyHint.cend()) return std::nullopt;
    RegWeightMap map;
    for (auto [reg, v] : iter->second) {
      if (isVirtualReg(reg)) {
        if (auto it = regMap.find(reg); it != regMap.cend() && !exclude.count(it->second))
          map[it->second] += v;
      } else if (!exclude.count(reg))
        map[reg] += v;
    }
    if (map.empty()) return std::nullopt;
    double maxWeight = -1e10;
    RegNum best = invalidReg;
    for (auto [reg, v] : map) {
      if (v > maxWeight) {
        maxWeight = v;
        best = reg;
      }
    }
    if (best == invalidReg) assert(false && "invalidReg");
    return best;
  };

  assert(assignStack.size() == vregs.size());
  while (!assignStack.empty()) {
    const auto u = assignStack.top();
    assignStack.pop();

    std::unordered_set<uint32_t> exclude;
    for (auto v : graph.adj(u)) {
      if (isVirtualReg(v)) {
        if (auto iter = regMap.find(v); iter != regMap.cend()) {
          exclude.insert(iter->second);
        }
      } else {
        exclude.insert(v);
      }
    }

    bool assigned = false;
    if (auto isaReg = calcCopyFreeProposal(u, exclude)) {
      assert(allocableISARegs.count(*isaReg));
      regMap[u] = *isaReg;
      colorDefUse(u, *isaReg);
      assigned = true;
    } else {
      if (!fixHazard) {
        for (auto reg : list) {
          if (!exclude.count(reg)) {
            regMap[u] = reg;
            assigned = true;
            break;
          }
        }
      } else {
        RegNum bestReg = invalidReg;
        double cost = 1e20;
        auto evalCost = [&](RegNum reg) {
          assert(isISAReg(reg));
          constexpr double calleeSavedCost = 5.0;

          double maxFreq = -1.0;
          constexpr InstNum infDist = 10;
          InstNum minDist = infDist;

          auto& defTimeOfColored = defUseTime[reg];
          auto evalDist = [&](InstNum curDefTime) {
            if (defTimeOfColored.empty()) return infDist;
            const auto it = defTimeOfColored.lower_bound(curDefTime);
            if (it == defTimeOfColored.begin()) return std::min(infDist, *it - curDefTime);
            if (it == defTimeOfColored.end()) return std::min(infDist, curDefTime - *std::prev(it));
            return std::min(infDist, std::min(curDefTime - *std::prev(it), *it - curDefTime));
          };

          for (auto instNum : defUseTime[u]) {
            const auto f = getBlockFreq(instNum);
            if (f > maxFreq) {
              minDist = evalDist(instNum);
              maxFreq = f;
            } else if (f == maxFreq)
              minDist = std::min(minDist, evalDist(instNum));
          }

          double curCost = -static_cast<double>(minDist);
          if (ctx.frameInfo.isCalleeSaved(MIROperand::asISAReg(reg, OperandType::Special))) {
            curCost += calleeSavedCost;
          }

          return curCost;
        };
        for (auto reg : list) {
          if (!exclude.count(reg)) {
            const auto curCost = evalCost(reg);
            if (curCost < cost) {
              cost = curCost;
              bestReg = reg;
            }
          }
        }

        regMap[u] = bestReg;
        colorDefUse(u, bestReg);
        assigned = true;
      }
    }
    if (!assigned) assert(false);
#ifdef DEBUG
    std::cerr << "assign " << (u ^ virtualRegBegin) << " -> " << regMap.at(u) << std::endl;
#endif
  }

  // mfunc.dump(std::cerr, ctx);
  return true;
}

bool GraphColoringAllocateContext::spillRegisters(MIRFunction& mfunc,
                                                  CodeGenContext& ctx,
                                                  InterferenceGraph& graph,
                                                  RegWeightMap& weights) {
  const auto canonicalizedType =
    ctx.registerInfo->getCanonicalizedRegisterTypeForClass(allocationClass);
  auto u = graph.pick_to_spill(blockList, weights, regCount);
  blockList.insert(u);
#ifdef DEBUG
  std::cerr << "spill: ";
  dumpVirtualReg(u) << std::endl;
  std::cerr << "block list " << blockList.size() << ' ' << graph.size() << '\n';
#endif
  if (!isVirtualReg(u)) {
    assert(false);
  }
  const auto size = getOperandSize(canonicalizedType);
  bool alreadyInStack = inStackArguments.count(u);
  bool rematerializeConstant = constants.count(u);
  MIROperand stackStorage;
  MIRInst* copyInst = nullptr;
  if (alreadyInStack) {
    stackStorage = inStackArguments.at(u);
  } else if (rematerializeConstant) {
    copyInst = constants.at(u);
  } else {
    stackStorage = mfunc.newStackObject(ctx.nextId(), size, size, 0, StackObjectUsage::RegSpill);
  }

  std::unordered_set<MIRInst*> newInsts;
  const uint32_t minimizeIntervalThreshold = 8;
  const auto fallback = blockList.size() >= minimizeIntervalThreshold;

  for (auto& block : mfunc.blocks()) {
    auto& instructions = block->insts();

    bool loaded = false;
    for (auto iter = instructions.begin(); iter != instructions.end();) {
      const auto next = std::next(iter);
      auto& inst = *iter;
      if (newInsts.count(inst)) {
        iter = next;
        continue;
      }
      auto& instInfo = ctx.instInfo.getInstInfo(inst);
      bool hasUse = false, hasDef = false;
      for (uint32_t idx = 0; idx < instInfo.operand_num(); ++idx) {
        auto& op = inst->operand(idx);
        if (!isOperandVReg(op)) continue;
        if (op.reg() != u) continue;

        const auto flag = instInfo.operand_flag(idx);
        if (flag & OperandFlagUse) {
          hasUse = true;
        } else if (flag & OperandFlagDef) {
          hasDef = true;
        }
      }

      if (hasUse && !loaded) {
        // should be placed before locked inst block
        auto it = iter;
        while (it != instructions.begin()) {
          auto& lockedInst = *std::prev(it);
          auto& lockedInstInfo = ctx.instInfo.getInstInfo(lockedInst);
          bool hasReg = false;
          for (uint32_t idx = 0; idx < lockedInstInfo.operand_num(); ++idx) {
            const auto& op = lockedInst->operand(idx);
            if (isOperandISAReg(op) && !ctx.registerInfo->is_zero_reg(op.reg()) &&
                isLockedOrUnderRenamedType(op.type()) &&
                (instInfo.operand_flag(idx) & OperandFlagDef)) {
              hasReg = true;
              break;
            }
          }
          if (!hasReg) break;
          --it;
        }

        if (rematerializeConstant) {
          // auto& copyInstInfo = ctx.instInfo.getInstInfo(*copyInst);
          // copyInstInfo.print(std::cerr, *copyInst, true);
          // std::cerr << '\n';
          auto tmpInst = new MIRInst(*copyInst);
          instructions.insert(it, tmpInst);
        } else {
          auto tmpInst = new MIRInst(InstLoadRegFromStack);
          tmpInst->set_operand(0, MIROperand::asVReg(u - virtualRegBegin, canonicalizedType));
          tmpInst->set_operand(1, stackStorage);
          instructions.insert(it, tmpInst);
          // instructions.insert(it,
          //                     MIRInst{ InstLoadRegFromStack }
          //                         .setOperand<0>(MIROperand::asVReg(u -
          //                         virtualRegBegin, canonicalizedType))
          //                         .setOperand<1>(stackStorage));
        }
        if (!fallback) loaded = true;
      }
      if (hasDef) {
        if (alreadyInStack || rematerializeConstant) {
          instructions.erase(iter);
          loaded = false;
        } else {
          // should be placed after rename inst block
          auto it = next;
          while (it != instructions.end()) {
            auto& renameInst = *it;
            auto& renameInstInfo = ctx.instInfo.getInstInfo(renameInst);
            bool hasReg = false;
            for (uint32_t idx = 0; idx < renameInstInfo.operand_num(); ++idx) {
              const auto& op = renameInst->operand(idx);
              if (isOperandISAReg(op) && !ctx.registerInfo->is_zero_reg(op.reg()) &&
                  isLockedOrUnderRenamedType(op.type()) &&
                  (instInfo.operand_flag(idx) & OperandFlagUse)) {
                hasReg = true;
                break;
              }
            }
            if (!hasReg) break;
            ++it;
          }
          auto tmpInst = new MIRInst(InstStoreRegToStack);
          tmpInst->set_operand(0, stackStorage);
          tmpInst->set_operand(1, MIROperand::asVReg(u - virtualRegBegin, canonicalizedType));
          auto newInst = instructions.insert(it, tmpInst);
          newInsts.insert(*newInst);
          loaded = false;
        }
      }

      iter = next;
    }

    // TODO: update live interval instead of recomputation?
  }
  cleanupRegFlags(mfunc, ctx);
  return true;
}
/**
 * 1. calculate live intervals for virtual registers
 * 2. collect all virtual registers
 * 3. construct interference graph

 */
static bool runAllocate(MIRFunction& mfunc,
                        CodeGenContext& ctx,
                        GraphColoringAllocateContext& allocateCtx) {
  /* return true if success; false if need to reAllocate */

  auto liveInterval = calcLiveIntervals(mfunc, ctx);
  // Collect virtual registers
  auto vregSet = allocateCtx.collectVirtualRegs(mfunc, ctx);

  // Construct interference graph
  // InterferenceGraph graph;
  auto cfg = calcCFG(mfunc, ctx);
  auto blockFreq = calcFreq(mfunc, cfg);

  auto graph = allocateCtx.buildGraph(mfunc, ctx, liveInterval, vregSet, blockFreq);
  auto vregs = graph.collect_nodes();  // all virtual registers need to be assigned
  assert(vregs.size() == vregSet.size());

  if (graph.empty()) return true;

  // Calculate weights for virtual registers
  // Weight = \sum (number of use/def) * Freq
  std::vector<std::pair<InstNum, double>> freq;
  for (auto& block : mfunc.blocks()) {
    auto endInst = liveInterval.inst2Num.at(block->insts().back());
    freq.emplace_back(endInst + 2, blockFreq.query(block.get()));
  }
  auto weights = allocateCtx.computeRegWeight(mfunc, ctx, vregs, blockFreq, liveInterval, freq);

  // Assign registers
  std::stack<uint32_t> assignStack;
  auto spillRegister =
    allocateCtx.assignRegisters(mfunc, ctx, graph, weights, assignStack /* ret */);

  // no spill register, allocate registers directly
  if (!spillRegister and
      allocateCtx.allocateRegisters(mfunc, ctx, vregs, assignStack, graph, freq)) {
    return true;
  }

  // Spill register
  if (allocateCtx.spillRegisters(mfunc, ctx, graph, weights)) {
    return false;
  }
  return false;
}

static void graphColoringAllocateImpl(MIRFunction& mfunc,
                                      CodeGenContext& ctx,
                                      GraphColoringAllocateContext& allocateCtx) {
  const auto allocationClass = allocateCtx.allocationClass;
#ifdef DEBUG
  std::cerr << "allocate for class " << allocationClass << std::endl;
#endif

  size_t iterantion = 0;
  while (not runAllocate(mfunc, ctx, allocateCtx)) {
    iterantion++;
#ifdef DEBUG
    std::cerr << "iteration " << iterantion << std::endl;
#endif
  }
#ifdef DEBUG
  std::cerr << "allocate for class " << allocationClass << " success" << std::endl;
#endif
}

void graphColoringAllocate(MIRFunction& mfunc, CodeGenContext& ctx, IPRAUsageCache& infoIPRA) {
  const auto classCount = ctx.registerInfo->get_alloca_class_cnt();
  //   std::unordered_map<uint32_t, uint32_t> regMap;  // -->
  //   存储[虚拟寄存器]到[物理寄存器]之间的映射
  auto allocateCtx = GraphColoringAllocateContext{infoIPRA};
  allocateCtx.collectInStackArgumentsRegisters(mfunc, ctx);
  allocateCtx.collectConstantsRegisters(mfunc, ctx);
  auto microArch = ctx.scheduleModel->getMicroArchInfo();
  allocateCtx.fixHazard = microArch.enablePostRAScheduling and !microArch.hasRegRenaming;

  auto& regMap = allocateCtx.regMap;
  for (uint32_t idx = 0; idx < classCount; idx++) {
    allocateCtx.initForAllocationClass(idx, ctx);
    graphColoringAllocateImpl(mfunc, ctx, allocateCtx);
  }

  for (auto& block : mfunc.blocks()) {
    auto& instructions = block->insts();
    for (auto inst : instructions) {
      auto& instInfo = ctx.instInfo.getInstInfo(inst);
      for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
        auto& op = inst->operand(idx);
        if (op.type() > OperandType::Float32) continue;
        if (isOperandVReg(op)) {
          const auto isaReg = regMap.at(op.reg());
          op = MIROperand::asISAReg(isaReg, op.type());
        }
      }
    }
  }
  // std::cerr << "regMap's size is " << regMap.size() << std::endl;
}
}  // namespace mir