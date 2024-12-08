#include <algorithm>
#include <limits>

#include "mir/LiveInterval.hpp"
#include "mir/target.hpp"
#include "mir/CFGAnalysis.hpp"

namespace mir {
/* utils function: 为每一条指令编号 */
static void assignInstNum(MIRFunction& mfunc, LiveVariablesInfo& info, CodeGenContext& ctx) {
    constexpr bool DebugAssign = false;
    auto& num = info.inst2Num;
    InstNum current = 4;
    if (DebugAssign) {
        std::cout << "the function is " << mfunc.name() << "\n";
    }
    for (auto& block : mfunc.blocks()) {
        for (auto& inst : block->insts()) {
            if (DebugAssign) {
                std::cout << current << ": ";
                ctx.instInfo.getInstInfo(inst).print(std::cout, *inst, true);
                std::cout << "\n";
            }
            num[inst] = current;
            current += defaultIncrement;
        }
    }
    uint64_t pre = 0;
    for (auto& block : mfunc.blocks()) {
        for (auto& inst : block->insts()) {
            if (num[inst] < pre) {
                ctx.instInfo.getInstInfo(inst).print(std::cerr, *inst, true);
                std::cerr << "\n";
                std::cerr << "num: " << num[inst] << "\n";
                std::cerr << "pre: " << pre << "\n";
            }
            assert(num[inst] > pre);
            pre = num[inst];
        }
    }
    if (DebugAssign) {
        std::cout << "\n\n";
    }
}

void LiveInterval::addSegment(const LiveSegment& segment) { segments.push_back(segment); }
InstNum LiveInterval::nextUse(InstNum begin) const {
    auto it = std::upper_bound(segments.cbegin(), segments.cend(), LiveSegment{0, begin + 1}, 
                               [](const LiveSegment& lhs, const LiveSegment& rhs) { return lhs.end < rhs.end; });
    if (it == segments.cend()) return std::numeric_limits<InstNum>::max();
    assert(it->begin > begin);
    return it->begin;
}
bool LiveInterval::verify() const {
    InstNum val = 0;
    for (auto segment : segments) {
        if (segment.begin >= segment.end) return false;
        if (val >= segment.end) return false;
        val = segment.end;
    }
    return true;
}
void LiveInterval::optimize() {
    constexpr bool Debug = false;
    if (Debug) {
        dump(std::cerr);
        std::cerr << std::endl;
    }
    assert(verify());
    auto cur = segments.begin();
    for (auto it = segments.begin(); it != segments.end(); it++) {
        if (it == cur) continue;
        if (cur->end < it->begin) {
            ++cur; *cur = *it;
        } else {
            cur->end = std::max(cur->end, it->end);
        }
    }
    segments.erase(std::next(cur), segments.end());
}
bool LiveInterval::intersectWith(const LiveInterval& rhs) const {
    /* 判断两个变量的活跃区间是否相交 */
    auto it = rhs.segments.begin();
    for (auto& [begin, end] : segments) {
        while (true) {
            if (it == rhs.segments.end()) return false;
            if (it->end <= begin) ++it;
            else break;
        }
        if (it->begin < end) return true;
    }
    return false;
}

void LiveInterval::dump(std::ostream& out) const {  // 方便调试
    for (auto& [begin, end] : segments) {
        out << "[" << begin << ", " << end << ")";
    }
}

// utils function
void cleanupRegFlags(MIRFunction& mfunc, CodeGenContext& ctx) {
    for (auto& block : mfunc.blocks()) {
        for (auto& inst : block.get()->insts()) {
            auto& instinfo = ctx.instInfo.getInstInfo(inst);
            for (uint32_t idx = 0; idx < instinfo.operand_num(); idx++) {
                auto op = inst->operand(idx);
                if (op.isReg()) {
                    auto reg = std::get<MIRRegister>(op.storage());
                    reg.set_flag(RegisterFlagNone);
                }
            }
        }
    }
}

/* 全局活跃变量分析 */
LiveVariablesInfo calcLiveIntervals(MIRFunction& mfunc, CodeGenContext& ctx) {
    constexpr bool Debug = false;
    LiveVariablesInfo info;
    auto cfg = calcCFG(mfunc, ctx);
    if (Debug) {
        std::cerr << "begin debuging calcLiveIntervals function. \n";
        std::cerr << "the function " << mfunc.name() << std::endl;
        cfg.dump(std::cerr); std::cerr << std::endl;
    }

    // stage 1: collect use/def link of virtual registers
    for (auto& block : mfunc.blocks()) {
        auto& blockInfo = info.block2Info[block.get()];
        for (auto& inst : block->insts()) {
            auto& instInfo = ctx.instInfo.getInstInfo(inst);
            for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
                const auto flag = instInfo.operand_flag(idx);
                auto& operand = inst->operand(idx);
                if (!isOperandVReg(operand)) continue;

                const auto id = regNum(operand);
                if (flag & OperandFlagDef) {
                    /* 变量定义 --> defined in this block */
                    blockInfo.defs.insert(id);
                } else if (flag & OperandFlagUse) {
                    /* 变量使用 --> defined in other block, but used in the block */
                    if (!blockInfo.defs.count(id)) {
                        blockInfo.uses.insert(id);
                    }
                } else {  
                    std::cerr <<"operand idx: " << idx <<  "unknown operand flag: " << flag << std::endl;
                    instInfo.print(std::cerr, *inst, false);
                    std::cerr << std::endl;
                    assert(false && "report unreachable");
                }
            }
        }
    }

    // stage 2: calculate ins and outs for each block
    while (true) {
        bool modified = false;
        if (Debug) {
            for(auto& block : mfunc.blocks()) {
                auto& blockInfo = info.block2Info[block.get()];
                
                block->print(std::cerr, ctx); std::cerr << "\n";

                std::cerr << "uses: ";
                for(auto use : blockInfo.uses) {
                    if (isVirtualReg(use)) std::cerr << "v" << (use ^ virtualRegBegin);
                    else if (isISAReg(use)) std::cerr << "i" << use;
                    else assert(false);
                    std::cerr << " ";
                }
                std::cerr << '\n';

                std::cerr << "defs: ";
                for(auto def : blockInfo.defs) {
                    if (isVirtualReg(def)) std::cerr << "v" << (def ^ virtualRegBegin);
                    else if (isISAReg(def)) std::cerr << "i" << def;
                    else assert(false);
                    std::cerr << ' ';
                }
                std::cerr << '\n';

                std::cerr << "ins: ";
                for(auto in : blockInfo.ins) {
                    if (isVirtualReg(in)) std::cerr << "v" << (in ^ virtualRegBegin);
                    else if (isISAReg(in)) std::cerr << "i" << in;
                    else assert(false);
                    std::cerr << ' ';
                }
                std::cerr << '\n';

                std::cerr << "outs: ";
                for(auto out : blockInfo.outs) {
                    if (isVirtualReg(out)) std::cerr << "v" << (out ^ virtualRegBegin);
                    else if (isISAReg(out)) std::cerr << "i" << out;
                    std::cerr << ' ';
                }
                std::cerr << '\n';
            }
        }
        for (auto& block : mfunc.blocks()) {
            auto b = block.get();
            auto& blockInfo = info.block2Info[b];
            std::unordered_set<RegNum> outs;
            for (auto [succ, prob] : cfg.successors(b)) {  // 计算当前块的输出集合
                for (auto in : info.block2Info[succ].ins) {
                    outs.insert(in);
                }
            }
            std::swap(blockInfo.outs, outs);
            auto ins = blockInfo.uses;  // other block pass the ins to the next block
            for (auto out : blockInfo.outs) {  // 计算当前块的输入集合
                if (!blockInfo.defs.count(out)) {
                    ins.insert(out);
                }
            }
            if (ins != blockInfo.ins) {
                std::swap(blockInfo.ins, ins);
                modified = true;
            }
        }

        if (!modified) break;
    }

    // stage 3: calculate live intervals
    assignInstNum(mfunc, info, ctx);
    for (auto& block : mfunc.blocks()) {
        auto& blockInfo = info.block2Info[block.get()];
        std::unordered_map<RegNum, LiveSegment> curSegment;  // 当前块内的活跃寄存器集合
        std::unordered_map<RegNum, std::pair<MIRInst*, std::vector<MIRRegisterFlag*>>> lastUse;  // 当前块内寄存器最后一次被使用的指令及其指令标志
        InstNum firstInstNum = 0, lastInstNum = 0;
        for (auto inst : block->insts()) {
            auto& instInfo = ctx.instInfo.getInstInfo(inst);
            const auto instNum = info.inst2Num[inst];

            // update firstInstNum and lastInstNum
            if (inst == block.get()->insts().front()) firstInstNum = instNum;
            lastInstNum = instNum;

            /* OperandFlagUse */
            for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
                const auto flag = instInfo.operand_flag(idx);
                auto& operand = inst->operand(idx);
                if (!isOperandVReg(operand)) continue;
                const auto id = regNum(operand);

                if (flag & OperandFlagUse) {
                    /* update the curSegment */
                    if (auto it = curSegment.find(id); it != curSegment.end()) {
                        it->second.end = instNum + 1;  // [begin, end)
                        assert(it->second.begin < it->second.end); 
                    } else {
                        assert(firstInstNum < instNum + 1);
                        curSegment[id] = {firstInstNum, instNum + 1};  // [begin, end)
                    }

                    /* update the lastUse */
                    if (auto it = lastUse.find(id); it != lastUse.end()) {
                        if (it->second.first == inst) {
                            it->second.second.push_back(std::get<MIRRegister>(operand.storage()).flag_ptr());
                        } else {
                            it->second = {inst, {std::get<MIRRegister>(operand.storage()).flag_ptr()}};
                        }
                    } else {
                        lastUse[id] = {inst, {std::get<MIRRegister>(operand.storage()).flag_ptr()}};
                    }
                }
            }
            
            /* OperandFlagDef */
            for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
                const auto flag = instInfo.operand_flag(idx);
                auto& operand = inst->operand(idx);
                if (!isOperandVReg(operand)) continue;
                const auto id = regNum(operand);

                if (flag & OperandFlagDef) {  // defined
                    if (curSegment.count(id)) {
                        auto& segment = curSegment[id];

                        if (segment.end == instNum + 1) {  // the register is defined and used in one instruction --> dead
                            segment.end = instNum + 2;
                            assert(segment.begin < segment.end);
                        } else {  // the register is not defined and used in one instruction
                            info.reg2Interval[id].addSegment(segment);
                            segment = {instNum + 1, instNum + 2};

                            if (auto it = lastUse.find(id); it != lastUse.end()) {
                                for (auto flagRegflag : it->second.second) {
                                    *flagRegflag = RegisterFlagDead;  // 寄存器的最后一次使用
                                }
                                lastUse.erase(it);
                            }
                        }
                    } else {  // the register is only defined int the block, is used in other block
                        curSegment[id] = {instNum + 1, instNum + 2};
                    }
                }
            }
        }

        for (auto& [id, segment] : curSegment) {
            if (blockInfo.outs.count(id)) {
                segment.end = lastInstNum + 2;
                assert(segment.begin < segment.end);
            }
            info.reg2Interval[id].addSegment(segment);
        }
        
        for (auto& [id, flags] : lastUse) {
            if (blockInfo.outs.count(id)) continue;
            for (auto flagReg : flags.second) {
                *flagReg = RegisterFlagDead;  // 无需输入到下一个块 -> 寄存器的最后一次使用
            }
        }

        for (auto id : blockInfo.outs) {
            if (blockInfo.ins.count(id) && !curSegment.count(id)) {
                info.reg2Interval[id].addSegment({firstInstNum, lastInstNum + 2});
                assert(firstInstNum < lastInstNum + 2);
            }
        }
    }

    // stage 4: optimize
    for (auto& [regNum, liveInterval] : info.reg2Interval) {
        if (Debug) {
            if (isVirtualReg(regNum)) {
                std::cerr << "v" << (regNum ^ virtualRegBegin) << ": ";
            } else if (isISAReg(regNum)) {
                std::cerr << "i" << regNum << ": ";
            } else if (isStackObject(regNum)) {
                std::cerr << "so" << (regNum ^ stackObjectBegin) << ": ";
            }
        }
        liveInterval.optimize();
        assert(liveInterval.verify());
    }

    return info;
}
}