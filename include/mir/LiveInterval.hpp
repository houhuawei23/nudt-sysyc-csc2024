#pragma once
#include <iostream>
#include <deque>
#include <unordered_set>
#include "mir/MIR.hpp"
#include "mir/instinfo.hpp"

namespace mir {
using InstNum = uint64_t;
using RegNum = uint32_t;
constexpr uint64_t defaultIncrement = 4;

/*
 * @brief: LiveSegment Struct
 * @note:
 *      i in [begin, end):
 *          this register should be available just before issuing instruction i
 */
struct LiveSegment final {
    InstNum begin, end;
    bool operator<(const LiveSegment& rhs) const { return begin < rhs.begin; }
};

/*
 * @brief: LiveInterval Struct
 * @note: 
 *      1. Live Interval (生命间隔)
 *      2. segments -> 按照结束位置从小到大进行排序
 */
struct LiveInterval final {
    std::deque<LiveSegment> segments;

    /* utils function */
    void addSegment(const LiveSegment& segment);
    InstNum nextUse(InstNum begin) const;
    bool intersectWith(const LiveInterval& rhs) const;
    void optimize();

    /* Just for Debug */
    bool verify() const;
    void dump(std::ostream& out) const;
};

/* LiveVariableInfo */
struct LiveVariablesBlockInfo final {
    std::unordered_set<RegNum> uses;  // defined in other block, but used in this block
    std::unordered_set<RegNum> defs;  // defined in this block

    std::unordered_set<RegNum> ins;   // block inputs
    std::unordered_set<RegNum> outs;  // block outputs
};

/* LiveVariablesInfo */
/*
 * @brief: LiveVariablesInfo
 * @note: 
 *      1. 功能: 相关变量的活跃信息
 * @param:
 *      1. inst2Num: 指令编号
 *      2. block2Info: 基本块内活跃变量相关信息
 *      3. reg2Interval: 寄存器活跃信息
 */
struct LiveVariablesInfo final {
    std::unordered_map<MIRInst*, InstNum> inst2Num;
    std::unordered_map<MIRBlock*, LiveVariablesBlockInfo> block2Info;
    std::unordered_map<RegNum, LiveInterval> reg2Interval;
};

/* utils function */
inline RegNum regNum(MIROperand& operand) {
    assert(isOperandVRegORISAReg(operand));
    return static_cast<RegNum>(operand.reg());
}
LiveVariablesInfo calcLiveIntervals(MIRFunction& mfunc, CodeGenContext& ctx);
void cleanupRegFlags(MIRFunction& mfunc, CodeGenContext& ctx);
}