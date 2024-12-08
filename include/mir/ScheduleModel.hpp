#pragma once
#include "mir/MIR.hpp"
#include "mir/instinfo.hpp"
#include <stdint.h>
#include <unordered_map>

namespace mir {
class ScheduleState;
class ScheduleClass {
public:
  virtual ~ScheduleClass() = default;
  virtual bool schedule(ScheduleState& state,
                        const MIRInst& inst,
                        const InstInfo& instInfo) const = 0;
};
struct MicroArchInfo {
  bool enablePostRAScheduling;
  // Front-end
  bool hasRegRenaming;
  bool hasMacroFusion;
  uint32_t issueWidth;
  // Back-end
  bool outOfOrder;
  // Memory system
  bool hardwarePrefetch;
  uint32_t maxDataStreams;
  uint32_t maxStrideByBytes;
};

class TargetScheduleModel {
public:
  virtual ~TargetScheduleModel() = default;
  virtual ScheduleClass& getInstScheClass(uint32_t opcode) = 0;
  virtual MicroArchInfo& getMicroArchInfo() = 0;
  virtual bool peepholeOpt(MIRFunction& func, CodeGenContext& context) { return false; }
  virtual bool isExpensiveInst(MIRInst* inst, CodeGenContext& context) { return false; }
};

class ScheduleState {
  uint32_t mCycleCount;

  // 流水线下一次可用时间: pipelineId -> cycle
  std::unordered_map<uint32_t, uint32_t> mNextPipelineAvailable;
  // 寄存器可用时间: renamedRegIdx -> cycle
  std::unordered_map<uint32_t, uint32_t> mRegisterAvailableTime;

  // inst.idx -> renamedRegIdx, 寄存器重命名映射
  const std::unordered_map<const MIRInst*, std::unordered_map<uint32_t, uint32_t>>& mRegRenameMap;
  // 已发射指令的标记掩码
  uint32_t mIssuedFlag;

public:
  ScheduleState(
    const std::unordered_map<const MIRInst*, std::unordered_map<uint32_t, uint32_t>>& regRenameMap)
    : mCycleCount(0), mRegRenameMap(regRenameMap), mIssuedFlag(0) {}
  // query
  uint32_t queryRegisterLatency(const MIRInst& inst, uint32_t idx);
  bool isPipelineReady(uint32_t pipelineId);
  bool isAvailable(uint32_t mask);
  // issue
  void setIssued(uint32_t mask);
  void resetPipeline(uint32_t pipelineId, uint32_t duration);
  void makeRegisterReady(const MIRInst& inst, uint32_t idx, uint32_t latency);

  uint32_t nextCycle() {
    mCycleCount++;
    mIssuedFlag = 0;
    return mCycleCount;
  }
};

struct BlockScheduleContext final {
  /* build anti-dependencies */
  std::unordered_map<MIRInst*, std::unordered_set<MIRInst*>> antiDeps;

  /* inst -> (operand_idx -> reg_idx) */
  std::unordered_map<const MIRInst*, std::unordered_map<uint32_t, uint32_t>> renameMap;

  /* indegree: number of insts this inst depends on: inst -> degree */
  std::unordered_map<MIRInst*, uint32_t> degrees;

  std::unordered_map<MIRInst*, int32_t> rank;

  int32_t waitPenalty;

public:
  bool celloctInfo(MIRBlock& block, const CodeGenContext& ctx);
};
}  // namespace mir
