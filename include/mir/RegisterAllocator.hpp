#pragma once
#include <cstring>
#include <unordered_set>
#include <unordered_map>
#include "mir/MIR.hpp"
#include "mir/target.hpp"
#include "mir/LiveInterval.hpp"
#include "mir/CFGAnalysis.hpp"
namespace mir {
/*
 * @brief: IPRAInfo Class
 * @details:
 *      存储每个函数中所使用到的Caller Saved寄存器
 */
using IPRAInfo = std::unordered_set<RegNum>;
class Target;
class IPRAUsageCache final {
  std::unordered_map<std::string, IPRAInfo> mCache;

public:
  void add(const CodeGenContext& ctx, MIRFunction& mfunc);
  void add(std::string symbol, IPRAInfo info);
  const IPRAInfo* query(std::string calleeFunc) const;

public:
  void dump(std::ostream& out, std::string calleeFunc) const;
};

void intraBlockAllocate(MIRFunction& mfunc, CodeGenContext& ctx, IPRAUsageCache& infoIPRA);

void graphColoringAllocate(MIRFunction& mfunc, CodeGenContext& ctx, IPRAUsageCache& infoIPRA);

void mixedRegisterAllocate(MIRFunction& mfunc, CodeGenContext& ctx, IPRAUsageCache& infoIPRA);

using RegWeightMap = std::unordered_map<RegNum, double>;
struct RegNumComparator final {
  const RegWeightMap* weights;

  bool operator()(RegNum lhs, RegNum rhs) const { return weights->at(lhs) > weights->at(rhs); }
};

/*
 * @brief: Interference Graph (干涉图)
 * @note:
 *      成员变量:
 *          1. std::unordered_map<RegNum, std::unordered_set<RegNum>> _adj
 *               干涉图 (存储每个节点的邻近节点)
 *          2. std::unordered_map<RegNum, uint32_t> _degree
 *               存储干涉图中虚拟寄存器节点的度数
 *          3. Queue _queue
 *               优先队列, 存储可分配物理寄存器的虚拟寄存器节点
 */
class InterferenceGraph final {
  std::unordered_map<RegNum, std::unordered_set<RegNum>> mAdj;
  std::unordered_map<RegNum, uint32_t> mDegree;
  using Queue = std::priority_queue<RegNum, std::vector<RegNum>, RegNumComparator>;
  Queue mQueue;

public: /* Get Function */
  auto& adj(RegNum u) {
    assert(isVirtualReg(u));
    return mAdj[u];  // create map pair if key not exits
  }

public: /* About Degree Function */
  void create(RegNum u) {
    if (!mDegree.count(u)) mDegree[u] = 0U;
  }
  auto empty() const { return mDegree.empty(); }
  auto size() const { return mDegree.size(); }

public: /* Util Function */
  /* 功能: 加边 */
  void add_edge(RegNum lhs, RegNum rhs);
  /* 功能: 为图着色寄存器分配做准备 */
  void prepare_for_assign(const RegWeightMap& weights, uint32_t k);
  /* 功能: 选择虚拟寄存器来为其分配物理寄存器 */
  RegNum pick_to_assign(uint32_t k);
  /* 功能: 选择虚拟寄存器来将其spill到栈内存中 */
  RegNum pick_to_spill(const std::unordered_set<RegNum>& blockList,
                       const RegWeightMap& weights,
                       uint32_t k) const;
  /* 功能: 统计干涉图中虚拟寄存器的个数 */
  std::vector<RegNum> collect_nodes() const;

public: /* just for debug */
  void dump(std::ostream& out) const;
};

struct GraphColoringAllocateContext final {
  // live over the allocating process
  IPRAUsageCache& infoIPRA;
  std::unordered_map<uint32_t, uint32_t> regMap;
  std::unordered_map<RegNum, MIROperand> inStackArguments;  // reg to in stack argument inst
  std::unordered_map<RegNum, MIRInst*> constants;           // reg to constant inst
  bool fixHazard;

  bool collectInStackArgumentsRegisters(MIRFunction& mfunc, CodeGenContext& ctx);
  bool collectConstantsRegisters(MIRFunction& mfunc, CodeGenContext& ctx);

  // live for specific allocationClass
  uint32_t allocationClass;
  size_t regCount;
  std::unordered_set<uint32_t> allocableISARegs;
  std::unordered_set<uint32_t> blockList;
  std::unordered_map<RegNum, std::set<InstNum>> defUseTime;
  std::unordered_map<RegNum, RegWeightMap> copyHint;

  void initForAllocationClass(uint32_t idx, CodeGenContext& ctx) {
    allocationClass = idx;

    const auto& list = ctx.registerInfo->get_allocation_list(allocationClass);
    regCount = list.size();

    allocableISARegs.clear();
    allocableISARegs.insert(list.cbegin(), list.cend());

    blockList.clear();
    defUseTime.clear();
    copyHint.clear();
  }

  std::unordered_set<RegNum> collectVirtualRegs(MIRFunction& mfunc, CodeGenContext& ctx);

  bool isAllocatableType(OperandType type, CodeGenContext& ctx) {
    return (type <= OperandType::Float32) &&
           (ctx.registerInfo->getAllocationClass(type) == allocationClass);
  }
  bool isLockedOrUnderRenamedType(OperandType type) { return (type <= OperandType::Float32); };

  void colorDefUse(RegNum src, RegNum dst);

  void updateCopyHint(RegNum dst, RegNum src, double weight);
  InterferenceGraph buildGraph(MIRFunction& mfunc,
                               CodeGenContext& ctx,
                               const LiveVariablesInfo& liveInterval,
                               const std::unordered_set<RegNum>& vregSet,
                               const BlockTripCountResult& blockFreq);

  RegWeightMap computeRegWeight(MIRFunction& mfunc,
                                CodeGenContext& ctx,
                                std::vector<uint32_t>& vregs,
                                BlockTripCountResult& blockFreq,
                                LiveVariablesInfo& liveInterval,
                                std::vector<std::pair<InstNum, double>>& freq);

  bool assignRegisters(MIRFunction& mfunc,
                       CodeGenContext& ctx,
                       const InterferenceGraph& graph,
                       const RegWeightMap& weights,
                       std::stack<uint32_t>& assignStack);

  bool allocateRegisters(MIRFunction& mfunc,
                         CodeGenContext& ctx,
                         std::vector<uint32_t>& vregs,
                         std::stack<uint32_t>& assignStack,
                         InterferenceGraph& graph,
                         std::vector<std::pair<InstNum, double>>& freq);

  bool spillRegisters(MIRFunction& mfunc,
                      CodeGenContext& ctx,
                      InterferenceGraph& graph,
                      RegWeightMap& weights);
};

struct VirtualRegUseInfo final {
  std::unordered_set<MIRBlock*> uses;
  std::unordered_set<MIRBlock*> defs;
};
struct FastAllocatorContext final {
  MIRFunction& mfunc;
  CodeGenContext& ctx;
  LiveVariablesInfo& liveInterval;
  IPRAUsageCache& infoIPRA;

  std::unordered_map<MIROperand, VirtualRegUseInfo, MIROperandHasher> useDefInfo;
  std::unordered_map<MIROperand, MIROperand, MIROperandHasher> isaRegHint;

  // find all cross-block vregs and allocate stack slots for them
  std::unordered_map<MIROperand, MIROperand, MIROperandHasher> stackMap;
  std::unordered_map<MIROperand, MIROperand, MIROperandHasher> isaRegStackMap;

  void collectUseDefInfo(MIRFunction& mfunc, CodeGenContext& ctx);
  void collectStackMap(MIRFunction& mfunc, CodeGenContext& ctx);
};

struct BlockAllocator final : public MIRBuilder {
  CodeGenContext& ctx;
  FastAllocatorContext& allocateCtx;
  // 存储当前块内需要spill到内存的虚拟寄存器
  std::unordered_map<MIROperand, MIROperand, MIROperandHasher> localStackMap;
  // 当前块中, 每个虚拟寄存器的映射 -> 栈 or 物理寄存器
  std::unordered_map<MIROperand, std::vector<MIROperand>, MIROperandHasher> currentMap;
  // 当前块中, [物理寄存器, MIROperand] (注意: 同一个物理寄存器可以分配给多个MIROperand)
  std::unordered_map<MIROperand, MIROperand, MIROperandHasher> physMap;
  // 分为两类: int registers and float registers
  std::unordered_map<uint32_t, std::queue<MIROperand>> allocationQueue;
  // retvals/callee arguments
  std::unordered_set<MIROperand, MIROperandHasher> protectedLockedISAReg;
  // callee retvals/arguments
  std::unordered_set<MIROperand, MIROperandHasher> underRenamedISAReg;

  MultiClassRegisterSelector selector;

  std::unordered_set<MIROperand, MIROperandHasher> dirtyVRegs;

  LiveVariablesBlockInfo& liveIntervalInfo;
  BlockAllocator(FastAllocatorContext& allocateCtx,
                 MIRBlock* block,
                 LiveVariablesBlockInfo& liveInfo)
    : allocateCtx(allocateCtx),
      ctx(allocateCtx.ctx),
      selector(*allocateCtx.ctx.registerInfo),
      liveIntervalInfo(liveInfo) {
    mCurrBlock = block;
    mInsertPoint = block->insts().begin();
  }

  const auto getStackStorage(const MIROperand& op);
  auto& getDataMap(const MIROperand& op);

  const auto isAllocatableType(OperandType type);
  const auto isProtected(const MIROperand& isaReg,
                         std::unordered_set<MIROperand, MIROperandHasher>& protectedRegs);

  const auto collectUnderRenamedISARegs(MIRInstList::iterator it);

  const auto evictVReg(MIROperand operand);
  auto getFreeReg(const MIROperand& operand,
                  std::unordered_set<MIROperand, MIROperandHasher>& protectedRegs);

  const auto use(MIROperand& op,
                 std::unordered_set<MIROperand, MIROperandHasher>& protectedRegs,
                 std::unordered_set<MIROperand, MIROperandHasher>& releaseVRegs);

  const auto def(MIROperand& op, std::unordered_set<MIROperand, MIROperandHasher>& protectedRegs);

  const auto spillBeforeBranch(MIRInst* inst);
  const auto saveCallerSavedRegsForCall(MIRInst* inst);
};
};  // namespace mir