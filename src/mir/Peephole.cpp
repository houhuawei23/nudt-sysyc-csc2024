#include <queue>
#include "mir/MIR.hpp"
#include "mir/instinfo.hpp"
#include "mir/utils.hpp"
#include "mir/CFGAnalysis.hpp"
#include "support/Hyperparameters.hpp"

namespace mir {
/* utils function */
std::unordered_map<MIROperand, uint32_t, MIROperandHasher> collectDefCount(MIRFunction& mfunc, CodeGenContext& ctx) {
    constexpr bool verify = true;
    std::unordered_map<MIROperand, uint32_t, MIROperandHasher> defCount;
    for (auto& block : mfunc.blocks()) {
        auto& instructions = block->insts();
        for (auto inst : instructions) {
            auto& instInfo = ctx.instInfo.getInstInfo(inst);
            if (requireFlag(instInfo.inst_flag(), InstFlagLoadConstant)) {
                /* 常量加载 */
                auto& dst = inst->operand(0);
                if (isOperandVReg(dst)) ++defCount[dst];
            } else {
                /* 其他指令 */
                for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
                    if (instInfo.operand_flag(idx) & OperandFlagDef) {
                        auto& def = inst->operand(idx);
                        if (isOperandVReg(def)) ++defCount[def];
                    }
                }
            }
        }
    }
    return defCount;
}

/*
 * @brief: EliminateStackLoads
 * @note: 
 *    消除无用的Load Register from Stack指令
 *    我们可以使用前面已经load之后的虚拟寄存器来增加Copy指令来代替Load指令   
 */
bool EliminateStackLoads(MIRFunction& mfunc, CodeGenContext& ctx) {
    if (!ctx.registerInfo || ctx.flags.preRA) return false;
    bool modified = false;
    for (auto& block : mfunc.blocks()) {
        auto& instructions = block->insts();

        uint32_t versionId = 0;
        /* reg2Version: reg -> version */
        std::unordered_map<MIROperand, uint32_t, MIROperandHasher> reg2Version;
        /* stack2Reg: stack -> (reg, version) */
        std::unordered_map<MIROperand, std::pair<MIROperand, uint32_t>, MIROperandHasher> stack2Reg;
        auto defReg = [&](MIROperand reg) { reg2Version[reg] = ++versionId; };

        for (auto inst : instructions) {
            if (inst->opcode() == InstStoreRegToStack) {
                auto& obj = inst->operand(0);
                auto& reg = inst->operand(1);
                if (auto iter = reg2Version.find(reg); iter != reg2Version.cend()) {
                    stack2Reg[obj] = { reg, iter->second };
                } else {
                    defReg(reg);
                    stack2Reg[obj] = { reg, versionId };
                }
            } else if (inst->opcode() == InstLoadRegFromStack) {
                auto& dst = inst->operand(0);
                auto& obj = inst->operand(1);
                if (auto iter = stack2Reg.find(obj); iter != stack2Reg.cend()) {
                    auto& [reg, ver] = stack2Reg[obj];
                    if (ver == reg2Version[reg]) {
                        // dst <- reg
                        inst->set_opcode(InstCopy);
                        obj = reg;
                        modified = true;
                    }
                }
            }

            auto& instInfo = ctx.instInfo.getInstInfo(inst);
            for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
                if (instInfo.operand_flag(idx) & OperandFlagDef) {
                    defReg(inst->operand(idx));
                }
            }

            /* NOTE: 物理寄存器可能会被重复定义, 此时需要更新物理寄存器相关的versionId */
            if (requireFlag(instInfo.inst_flag(), InstFlagCall)) {
                std::vector<MIROperand> nonVReg;
                for (auto [reg, ver] : reg2Version) {
                    if (isISAReg(reg.reg())) nonVReg.push_back(reg);
                }
                for (auto reg : nonVReg) defReg(reg);
            }

            /* NOTE: 更新 */
            if (inst->opcode() == InstLoadRegFromStack) {
                auto& dst = inst->operand(0);
                auto& obj = inst->operand(1);
                stack2Reg[obj] = { dst, reg2Version.at(dst) };
            }
        }
    }
    
    return modified;
}

/*
 * @brief: EliminateIndirectCopy
 * @note: 
 *    消除间接的Copy指令
 *    我们将使用到Copy指令dst的指令直接替换为src
 */
bool EliminateIndirectCopy(MIRFunction& mfunc, CodeGenContext& ctx) {
    if (ctx.flags.dontForward) return false;  // TODO: ???
    
    bool modified = false;
    for (auto& block : mfunc.blocks()) {
        auto& instructions = block->insts();

        uint32_t versionId = 0;
        /* regValue: copy指令中dst -> (src, src version) */
        std::unordered_map<uint32_t, std::pair<uint32_t, uint32_t>> regValue;
        /* version: reg -> version */
        std::unordered_map<uint32_t, uint32_t> version;
        const auto getVersion = [&](const uint32_t reg) {
            assert(isVirtualReg(reg) || isISAReg(reg));
            if (auto it = version.find(reg); it != version.cend()) return it->second;
            return version[reg] = ++versionId;
        };
        const auto defReg = [&](MIROperand& op) {
            if (!isOperandVRegORISAReg(op)) return;
            version[op.reg()] = ++versionId;
            regValue.erase(op.reg());
        };
        const auto replaceUse = [&](MIRInst* inst, MIROperand& reg) {
            if (!isOperandVRegORISAReg(reg)) return;
            if (auto it = regValue.find(reg.reg()); it != regValue.cend() && it->second.second == getVersion(it->second.first)) {
                if (ctx.flags.preRA && (!isVirtualReg(it->second.first) && !(ctx.registerInfo && ctx.registerInfo->is_zero_reg(it->second.first)))) {
                    return;  // should be handled after RA
                }
                auto backup = reg;
                reg = MIROperand{ MIRRegister{ it->second.first }, backup.type() };
                if (reg == backup) return;
                if (inst->opcode() == InstCopy) {
                    inst->set_opcode(select_copy_opcode(inst->operand(0), reg));
                }
                modified = true;
            }
        };

        for (auto iter = instructions.begin(); iter != instructions.end();) {
            auto& inst = *iter;
            auto next = std::next(iter);

            auto& instInfo = ctx.instInfo.getInstInfo(inst);
            for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
                if (instInfo.operand_flag(idx) & OperandFlagUse) {
                    auto& operand = inst->operand(idx);
                    replaceUse(inst, operand);
                }
            }

            MIROperand src, dst;
            if (ctx.instInfo.matchCopy(inst, dst, src)) {
                /* Copy Instruction */
                assert(isOperandVRegORISAReg(src) && isOperandVRegORISAReg(dst));
                if (auto it = regValue.find(dst.reg()); it != regValue.cend() && it->second.second == getVersion(it->second.first)) {
                    instructions.erase(iter);
                    modified = true;
                } else {
                    defReg(dst);
                    regValue[dst.reg()] = { src.reg(), getVersion(src.reg()) };
                }
            } else {
                /* OperandFlagDef */
                for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
                    if (instInfo.operand_flag(idx) & OperandFlagDef) {
                        defReg(inst->operand(idx));
                    }
                }

                /* Call Instruction */
                if (requireFlag(instInfo.inst_flag(), InstFlagCall)) {
                    std::vector<uint32_t> nonVReg;
                    for (auto [reg, ver] : version) {
                        if (isISAReg(reg)) nonVReg.push_back(reg);
                    }
                    for (auto reg : nonVReg) {
                        version[reg] = ++versionId;
                        regValue.erase(reg);
                    }
                }
            }

            iter = next;
        }
    }
    return modified;
}

/*
 * @brief: EliminateUnuseCopy
 * @note: 
 *     删除无用复制
 */
bool EliminateUnusedCopy(MIRFunction& mfunc, CodeGenContext& ctx) {
    bool modified = false;
    for (auto& block : mfunc.blocks()) {
        block->insts().remove_if([&](MIRInst* inst) {
            MIROperand dst, src;
            const auto remove = ctx.instInfo.matchCopy(inst, dst, src) && dst.reg() == src.reg();
            modified |= remove;
            return remove;
        });
    }
    return modified;
}

/*
 * @brief: EliminateUnusedInst
 * @note: 
 *     删除未被使用的指令
 */
bool EliminateUnusedInst(MIRFunction& mfunc, CodeGenContext& ctx) {
    /* writers - def -> vec<inst> */
    std::unordered_map<MIROperand, std::vector<MIRInst*>, MIROperandHasher> writers;
    std::queue<MIRInst*> q;

    auto isAllocableType = [](OperandType type) { return type <= OperandType::Float32; };

    for (auto& block : mfunc.blocks()) {
        for (auto inst : block->insts()) {
            const auto& instInfo = ctx.instInfo.getInstInfo(inst);
            bool special = false;
            if (requireOneFlag(instInfo.inst_flag(), InstFlagSideEffect)) special = true;
            for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
                if (instInfo.operand_flag(idx) & OperandFlagDef) {
                    auto op = inst->operand(idx);
                    writers[op].push_back(inst);
                    if (op.isReg() && isISAReg(op.reg()) && isAllocableType(op.type())) {
                        special = true;
                    }
                }
            }
            if (special) q.push(inst);
        }
    }
    while (!q.empty()) {
        auto inst = q.front(); 
        q.pop();
        const auto& instInfo = ctx.instInfo.getInstInfo(inst);
        for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
            if (instInfo.operand_flag(idx) & OperandFlagUse) {
                auto op = inst->operand(idx);
                if (auto iter = writers.find(op); iter != writers.end()) {
                    for (auto writer : iter->second) {
                        q.push(writer);
                    }
                    writers.erase(iter);
                }
            }
        }
    }
    /* 移除候选集 */
    std::unordered_set<MIRInst*> remove;  // 存储需要移除的指令
    for (auto [op, writerList] : writers) {
        if (isISAReg(op.reg()) && isAllocableType(op.type())) continue;
        for (auto writer : writerList) {
            const auto& instInfo = ctx.instInfo.getInstInfo(writer);
            if (requireOneFlag(instInfo.inst_flag(), InstFlagSideEffect | InstFlagMultiDef)) continue;
            remove.insert(writer);
        }
    }
    /* 执行移除操作 */
    for (auto& block : mfunc.blocks()) {
        block->insts().remove_if([&](MIRInst* inst) { return remove.count(inst); });
    }
    return !remove.empty();
}

/*
 * @brief: ApplySSAPropagation
 * @note: 
 *    注意, 这是针对于MIR级别的优化
 *    对于加载常数的虚拟寄存器, 在后续指令中, 可能会有多个copy指令, 我们以此消除中间商
 * @details: 
 *    SSA是一种常见的中间表示形式, 它使得变量在每个函数中只被赋值一次
 *    这有助于进行更有效的优化, 比如常量传播和死代码消除等等
 */
bool ApplySSAPropagation(MIRFunction& mfunc, CodeGenContext& ctx) {
    if (!ctx.flags.inSSAForm) return false;
    bool modified = false;
    auto DefCount = collectDefCount(mfunc, ctx);
    
    /* LoadConstant Instruction */
    std::unordered_set<MIROperand, MIROperandHasher> constants;
    for (auto& block : mfunc.blocks()) {
        for (auto inst : block->insts()) {
            auto& instInfo = ctx.instInfo.getInstInfo(inst);
            if (requireFlag(instInfo.inst_flag(), InstFlagLoadConstant)) {
                auto& dst = inst->operand(0);
                if (isOperandVReg(dst) && DefCount[dst] <= 1) {
                    constants.insert(dst);
                }
            }
        }
    }

    /* Copy Instruction */
    /* copy: dst -> src (加载常数的虚拟寄存器) */
    std::unordered_map<MIROperand, MIROperand, MIROperandHasher> copy;
    for (auto& block : mfunc.blocks()) {
        for (auto inst : block->insts()) {
            MIROperand dst, src;
            if (ctx.instInfo.matchCopy(inst, dst, src) && isOperandVReg(dst) && isOperandVReg(src) && constants.count(src)) {
                if (DefCount[dst] <= 1 && DefCount[src] <= 1) {
                    copy[dst] = src;
                }
            }
        }
    }

    /* 消除指令中的中间商 */
    if (copy.empty()) return false;
    for (auto& block : mfunc.blocks()) {
        auto& instructions = block->insts();
        for (auto inst : instructions) {
            auto& instInfo = ctx.instInfo.getInstInfo(inst);
            for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
                if (instInfo.operand_flag(idx) & OperandFlagUse) {
                    auto& op = inst->operand(idx);
                    if (auto iter = copy.find(op); iter != copy.cend()) {
                        op = iter->second;
                        modified = true;
                    }
                }
            }
        }
    }

    return modified;
}

/*
 * @brief: EliminateConstantLoads / machineConstantCSE
 * @note: 
 *      在MIR级别上进行常量传播和消除冗余的常量加载
 *      NOTE: 在SSA形式上的后端代码进行相关优化
 */
bool EliminateConstantLoads(MIRFunction& mfunc, CodeGenContext& ctx) {
    if (!ctx.flags.inSSAForm) return false;
    constexpr bool Debug = false;
    bool modified = false;
    
    auto defCount = collectDefCount(mfunc, ctx);
    auto& entryBlockInst = mfunc.blocks().front()->insts();
    /* beginConstants: opcode -> (src, dst) */
    std::unordered_map<uint32_t, std::unordered_map<MIROperand, MIROperand, MIROperandHasher>> beginConstants;
    for (auto inst : entryBlockInst) {
        auto& instInfo = ctx.instInfo.getInstInfo(inst);
        if (requireFlag(instInfo.inst_flag(), InstFlagLoadConstant)) {
            auto& dst = inst->operand(0);
            if (isOperandVReg(dst) && defCount[dst] <= 1) {
                auto& src = inst->operand(1);
                beginConstants[inst->opcode()].emplace(src, dst);
            }
        }
    }

    for (auto& block : mfunc.blocks()) {
        auto& instructions = block->insts();
        /* constants: opcode -> (src, dst) */
        std::unordered_map<uint32_t, std::unordered_map<MIROperand, MIROperand, MIROperandHasher>> constants;
        for (auto& inst : instructions) {
            auto& instInfo = ctx.instInfo.getInstInfo(inst);
            if (requireFlag(instInfo.inst_flag(), InstFlagLoadConstant)) {
                auto& dst = inst->operand(0);
                auto& src = inst->operand(1);
                auto& beginMap = beginConstants[inst->opcode()];
                auto& map = constants[inst->opcode()];
                if (auto it = beginMap.find(src); it != beginMap.end() && &block != &mfunc.blocks().front()) {
                    auto& lastDef = it->second;
                    inst = new MIRInst{ select_copy_opcode(dst, lastDef)};
                    inst->set_operand(0, dst); inst->set_operand(1, lastDef);
                    modified = true;
                } else if (auto iter = map.find(src); iter != map.end()) {
                    auto& lastDef = iter->second;
                    inst = new MIRInst{ select_copy_opcode(dst, lastDef) };
                    inst->set_operand(0, dst); inst->set_operand(1, lastDef);
                    modified = true;
                } else if (!isISAReg(dst.reg())) {
                    map.emplace(src, dst);
                }
            }
        }
    }
    
    return modified;
}

/*
 * @brief: ConstantHoist
 * @note: 
 *      常量提升, 将常量加载操作移动到程序的更高层次, 以减少重复的常量加载
 *      NOTE: 
 *          1. 在SSA形式上的后端代码进行相关优化
 *          2. 无需将所有的常量都提升, 可以提升部分优先级高的常量
 */
bool ConstantHoist(MIRFunction& mfunc, CodeGenContext& ctx) {
    if (!ctx.flags.inSSAForm) return false;

    constexpr bool Debug = false;
    bool modified = false;
    const uint32_t maxCnt = static_cast<uint32_t>(utils::ConstantHoistNum);
    uint32_t cnt = 0;
    for (auto& inst : mfunc.blocks().front()->insts()) {
        auto& instInfo = ctx.instInfo.getInstInfo(inst);
        if (requireFlag(instInfo.inst_flag(), InstFlagLoadConstant)) {
            auto& dst = inst->operand(0);
            if (isOperandVReg(dst)) cnt++;
        }
    }
    if (maxCnt <= cnt) {
        std::cerr << "load constant instructions: " << cnt << "\n";
        return false;
    }

    auto cfg = calcCFG(mfunc, ctx);
    auto freq = calcFreq(mfunc, cfg);
    auto defCount = collectDefCount(mfunc, ctx);
    auto& entryBlockInsts = mfunc.blocks().front()->insts();
    std::vector<std::pair<MIRInst**, double>> constants;
    for (auto& block : mfunc.blocks()) {
        if (&block == &mfunc.blocks().front()) continue;

        const auto blockFreq = freq.query(block.get());
        if (blockFreq <= utils::PrimaryPathThreshold) {
            if (Debug) {
                std::cerr << "low frequency block. \n";
                std::cerr << block->name() << " frequency: " << blockFreq << "\n";
            }
            continue;
        }

        auto& instructions = block->insts();
        for (auto& inst : instructions) {
            auto& instInfo = ctx.instInfo.getInstInfo(inst);
            if (requireFlag(instInfo.inst_flag(), InstFlagLoadConstant)) {
                auto& dst = inst->operand(0);
                if (isOperandVReg(dst) && defCount[dst] <= 1) {
                    constants.emplace_back(&inst, blockFreq);
                }
            }
        }
    }
    if (constants.empty()) return false;

    std::stable_sort(constants.begin(), constants.end(), 
                     [](const auto& lhs, const auto& rhs) { return lhs.second > rhs.second; });
    for (auto [inst_ptr, blockFreq] : constants) {
        auto inst = *inst_ptr;
        auto& dst = inst->operand(0);
        entryBlockInsts.insert(std::prev(entryBlockInsts.end()), inst);
        *inst_ptr = new MIRInst{ InstCopy };
        (*inst_ptr)->set_operand(0, MIROperand::asVReg(ctx.nextId(), dst.type()));
        (*inst_ptr)->set_operand(1, dst);
        
        modified = true;
        ++cnt;
        if (cnt >= maxCnt) break;
    }
    return modified;
}

/*
 * @brief: EliminateRedundantInst
 * @note: 
 *     在MIR级别上消除程序中重复的指令模式
 *     NOTE: 在SSA形式上的后端代码进行相关优化
 */
bool EliminateRedundantInst(MIRFunction& mfunc, CodeGenContext& ctx) {
    if (!ctx.flags.inSSAForm) return false;
    bool modified = false;
    for (auto& block : mfunc.blocks()) {
        auto& instructions = block->insts();
        std::unordered_map<MIROperand, uint32_t, MIROperandHasher> version;
        uint32_t versionId = 0;
        auto getVersion = [&](MIROperand& op) ->uint32_t {
            if (!isOperandVRegORISAReg(op)) return 0;
            if (auto it = version.find(op); it != version.end()) return it->second;
            version[op] = ++versionId;
            return versionId;
        };
        using VersionArray = std::array<uint32_t, MIRInst::max_operand_num>;
        std::unordered_map<MIRInst*, VersionArray> verArray;
        std::unordered_map<uint32_t, std::vector<MIRInst*>> lastDef;  /* opcode -> insts */

        for (auto& inst : instructions) {
            auto& instInfo = ctx.instInfo.getInstInfo(inst);
            auto equal = [&](MIRInst* rhs, VersionArray& rhsVer, uint32_t defIdx, VersionArray& ver) {
                if (inst->opcode() != rhs->opcode()) return false;
                for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
                    if (idx == defIdx) {
                        if (rhsVer[idx] != version.at(rhs->operand(idx))) return false;
                        continue;
                    }
                    if (inst->operand(idx) != rhs->operand(idx) || ver[idx] != rhsVer[idx]) return false;
                }
                return true;
            };

            if (requireFlag(instInfo.inst_flag(), InstFlagCall)) {
                // TODO: use IPRA info
                lastDef.clear();
                continue;
            }

            if (requireOneFlag(instInfo.inst_flag(), 
                               InstFlagSideEffect | InstFlagPCRel | InstFlagMultiDef | InstFlagRegCopy | InstFlagLoadConstant)) {
                for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
                    if (instInfo.operand_flag(idx) & OperandFlagDef) {
                        version[inst->operand(idx)] = ++versionId;
                    }
                }
            } else {
                auto& ver = verArray[inst];
                for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
                    ver[idx] = getVersion(inst->operand(idx));
                }

                // auto& cache = lastDef
                // for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
                //     if (instInfo.operand_flag(idx) & OperandFlagDef) {
                //         auto& op = inst->operand(idx);
                //         if (isOperandVRegORISAReg(op)) {
                //             bool changed = false;
                //             for (rhs : cache)
                //         }
                //     }
                // }
            }
        }
    }
    return modified;
}

/*
 * @brief: DeadInstElimination
 * @note: 
 *    删除冗余代码
 *    NOTE: 需要按照BFS dom tree的顺序来遍历基本块
 *    优化范围: 针对于preRA和postRA的代码优化
 */
bool DeadInstElimination(MIRFunction& mfunc, CodeGenContext& ctx) {
    bool modified = false;
    for (auto& block : mfunc.blocks()) {
        auto& instructions = block->insts();

        /* version: operand -> id, 同一个操作数, 被不同的指令定义, 其版本号也就不一样s */
        std::unordered_map<MIROperand, uint32_t, MIROperandHasher> version;
        uint32_t versionIdx = 0;
        auto getVersion = [&](MIROperand& op) ->uint32_t {
            if (!isOperandVRegORISAReg(op)) return 0;
            if (auto iter = version.find(op); iter != version.cend()) return iter->second;
            return version[op] = ++versionIdx;
        };
    
        /* lastDef: operand -> (inst: 当前指令, id_array: 当前指令中被使用的操作数的版本号), 存储每个操作数最后定义的指令和其操作数的版本号数组 */
        using VersionArray = std::array<uint32_t, MIRInst::max_operand_num>;
        std::unordered_map<MIROperand, std::pair<MIRInst*, VersionArray>, MIROperandHasher> lastDef;

        /* 删除死代码 */
        instructions.remove_if([&](MIRInst* inst) {
            auto& instInfo = ctx.instInfo.getInstInfo(inst);
            /* 首先得到当前指令中被使用的操作数的版本号 */
            VersionArray ver{};
            for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
                if (instInfo.operand_flag(idx) & OperandFlagUse) {
                    ver[idx] = getVersion(inst->operand(idx));
                }
            }

            /*
             NOTE: Call Instruction一定不会被删除
             同时, 由于Call指令会在被调用函数中使用到相关的物理寄存器 (包括Caller-Saved和Callee-Saved)
             为了方便处理, 我们将在Call指令分析出来的lastDef清空重新进行分析
            */
            if (requireFlag(instInfo.inst_flag(), InstFlagCall)) {
                lastDef.clear();
                return false;
            }

            /* 其次根据计算得到的版本号和指令来删除冗余代码 */
            for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
                if (instInfo.operand_flag(idx) & OperandFlagDef) {
                    auto& op = inst->operand(idx);
                    if (isOperandVRegORISAReg(op)) {
                        if (auto it = lastDef.find(op); it != lastDef.cend()) {
                            if (!requireOneFlag(instInfo.inst_flag(), InstFlagSideEffect | InstFlagPCRel | InstFlagMultiDef)
                                && *(it->second.first) == *inst && it->second.second == ver) {
                                modified = true;
                                return true;
                            }
                            it->second = { inst, ver };
                        } else {
                            lastDef[op] = { inst, ver };
                        }
                        
                        version[op] = ++versionIdx;
                    }
                    return false;
                }
            }
            return false;
        });
    }
    
    return modified;
}

/*
 * @brief: EliminateInvisibleInsts
 * @note: 
 *    功能: 删除无用赋值 (基于基本块内的优化)
 *    NOTE: 需要按照BFS dom tree的顺序来遍历基本块
 *    优化范围: 针对于postRA的代码优化
 */
bool EliminateInvisibleInsts(MIRFunction& mfunc, CodeGenContext& ctx) {
    if (ctx.flags.preRA || !ctx.registerInfo) return false;

    bool modified = false;
    for (auto& block : mfunc.blocks()) {
        std::unordered_set<MIRInst*> UnusedInsts;
        /* writer: operand (指令中被定义的操作数) -> inst (指令) */
        std::unordered_map<MIROperand, MIRInst*, MIROperandHasher> writer;

        auto use = [&](MIROperand& op) {
            if (isOperandISAReg(op)) {
                if (auto it = writer.find(op); it != writer.cend()) {
                    UnusedInsts.erase(it->second);
                }
            }
        };
        auto release = [&]() {
            for (auto [k, v] : writer) {
                UnusedInsts.erase(v);
            }
        };
        
        for (auto inst : block->insts()) {
            auto& instInfo = ctx.instInfo.getInstInfo(inst);
            
            /* OperandFlagUse */
            for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
                if (instInfo.operand_flag(idx) & OperandFlagUse) {
                    use(inst->operand(idx));
                }
            }

            /* 数据流不确定, 采取保险的优化 */
            if (requireOneFlag(instInfo.inst_flag(), InstFlagBranch | InstFlagCall)) {
                release();
            }

            /* OperandFlagDef */
            for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
                if (instInfo.operand_flag(idx) & OperandFlagDef) {
                    auto& op = inst->operand(idx);
                    if (isOperandISAReg(op)) {
                        writer[op] = inst;
                    }
                }
            }

            if (!requireOneFlag(instInfo.inst_flag(), InstFlagSideEffect)) {
                UnusedInsts.insert(inst);
            }
        }

        /* 不确定剩余的指令是否会在后续的块中被使用, 采取保险的优化 */
        release();

        /* 删除无用赋值 */
        if (UnusedInsts.empty()) continue;
        modified = true;
        block->insts().remove_if([&](MIRInst* inst) {
            return UnusedInsts.count(inst);
        });
    }
    return modified;
}

/* 窥孔优化 */
bool genericPeepholeOpt(MIRFunction& mfunc, CodeGenContext& ctx) {
    constexpr bool Debug = false;
    bool modified = false;
    modified |= EliminateStackLoads(mfunc, ctx);
    // modified |= EliminateIndirectCopy(mfunc, ctx);
    modified |= EliminateUnusedCopy(mfunc, ctx);
    modified |= EliminateUnusedInst(mfunc, ctx);
    modified |= ApplySSAPropagation(mfunc, ctx);
    modified |= EliminateConstantLoads(mfunc, ctx);
    modified |= ConstantHoist(mfunc, ctx);
    modified |= EliminateRedundantInst(mfunc, ctx);
    modified |= DeadInstElimination(mfunc, ctx);
    modified |= EliminateInvisibleInsts(mfunc, ctx);
    modified |= ctx.scheduleModel->peepholeOpt(mfunc, ctx);
    if (Debug) {
        std::cerr << "genericPeepholeOpt function" << std::endl;
        std::cerr << "modified = " << modified << std::endl;
    }
    return modified;
}
}