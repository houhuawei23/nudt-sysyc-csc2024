#include <cmath>
#include <limits>
#include "mir/CFGAnalysis.hpp"

namespace mir {
void CFGAnalysis::dump(std::ostream& out) {
    for (auto [bb, info] : mBlock2CFGInfo) {
        out << bb->name() << ": \n";

        out << "the predecessors are as follows: \n";
        for (auto pre : info.predecessors) {
            out << "\tblock: " << pre.block->name() << ", prob: " << pre.prob << "\n";
        }

        out << "the successors are as follows: \n";
        for (auto suc : info.successors) {
            out << "\tblock: " << suc.block->name() << ", prob: " << suc.prob << "\n";
        }
    }
}

CFGAnalysis calcCFG(MIRFunction& mfunc, CodeGenContext& ctx) {
    /* 确保该函数以终止指令结束 */
    assert(ctx.flags.endsWithTerminator);
    constexpr bool DebugCFG = false;

    CFGAnalysis res;
    auto& CFGInfo = res.block2CFGInfo();
    auto& blocks = mfunc.blocks();
    auto connect = [&](MIRBlock* src, MIRBlock* dst, double prob) {
        CFGInfo[src].successors.push_back({dst, prob});
        CFGInfo[dst].predecessors.push_back({src, prob});
    };

    for (auto it = blocks.begin(); it != blocks.end(); it++) {
        auto& block = *it;
        auto next = std::next(it);
        auto terminator = block->insts().back();
        
        MIRBlock* targetBlock; double prob;
        if (ctx.instInfo.matchBranch(terminator, targetBlock, prob)) {  // Match Jump Branch
            if (requireFlag(ctx.instInfo.getInstInfo(terminator).inst_flag(), InstFlagNoFallThrough)) {
                /* unconditional branch */
                connect(block.get(), targetBlock, 1.0);
            } else {
                /* conditional branch */
                if (next != blocks.end()) {  // 非exit块 
                    if (next->get() == targetBlock) {
                        connect(block.get(),  targetBlock, 1.0);
                    } else {
                        connect(block.get(), targetBlock, prob);
                        connect(block.get(), next->get(), 1.0 - prob);
                    }
                } else {  // exit块
                    connect(block.get(), targetBlock, prob);
                }
            }
        } else if (requireFlag(ctx.instInfo.getInstInfo(terminator).inst_flag(), InstFlagIndirectJump)) {  // jump register
            const auto jumpTable = dynamic_cast<MIRJumpTable*>(terminator->operand(1).reloc());
            auto& table = jumpTable->data();
            prob = 1.0 / static_cast<double>(table.size());
            for (auto item : table) connect(block.get(), dynamic_cast<MIRBlock*>(item), prob);
        }
    }
    if (DebugCFG) {
        std::cerr << "function " << mfunc.name() << "\n";
        res.dump(std::cerr);
    }

    return res;
}

double BlockTripCountResult::query(MIRBlock* block) const {
    if (auto it = _freq.find(block); it != _freq.end()) return it->second;
    return 1.0;
}
void BlockTripCountResult::dump(std::ostream& out) {
    out << "begin dump the frequency of blocks. \n";
    for (auto [block, freq] : _freq) {
        out << "block " << block->name() << " prob: " << freq << "\n";
    }
}
/*
 * @brief: LUP分解
 * @note: 将一个矩阵分解成一个下三角矩阵, 一个上三角矩阵以及一个置换矩阵的乘积
 * @param: 
 *      size_t n: 矩阵的大小 (n * n)
 *      std::vector<double>& a: 矩阵, 基本块之间的跳转关系
 */
static std::vector<double> LUP(size_t n, std::vector<double>& a) {
    const auto mat = [&](uint32_t i, uint32_t j) -> double& { return a[i * n + j]; };
    constexpr double eps = 1e-8;  // 常量 --> 判断浮点数常值的零
    std::vector<uint32_t> p(n);   // 置换向量 --> 记录行交换的信息
    for (uint32_t i = 0; i < n; i++) p[i] = i;

    /* LUP分解 */
    for (uint32_t i = 0; i < n; i++) {
        uint32_t x = std::numeric_limits<uint32_t>::max();
        double maxV = eps;
        // 找到当前行和之后行中绝对值最大的元素
        for (uint32_t j = i; j < n; j++) {
            const auto pivot = std::fabs(mat(i, j));  // 主元
            if (pivot > maxV) {
                maxV = pivot;
                x = j;
                break;
            }
        }
        if (maxV == eps) return {{}};  // 矩阵是奇异的, 无法进行分解

        if (i != x) {  // 主元不在当前行, 更新矩阵和置换向量
            std::swap(p[i], p[x]);
            for (uint32_t j = 0; j < n; j++) std::swap(mat(i, j), mat(x, j));
        }
        const auto pivot = mat(i, i);
        for (uint32_t j = i + 1; j < n; j++) {
            mat(j, i) /= pivot;
            const auto scale = mat(j, i);
            for (uint32_t k = i + 1; k < n; k++) {
                mat(j, k) -= scale * mat(i, k);
            }
        }
    }

    std::vector<double> c(n), d(n);
    for (uint32_t i = 0; i < n; i++) {
        double sum = (p[i] == 0 ? 1.0 : 0.0);
        for (uint32_t j = 0; j < i; j++) {
            sum -= mat(i, j) * c[j];
        }
        c[i] = sum;
    }
    for (auto i = static_cast<int32_t>(n - 1); i >= 0; i--) {
        const auto ui = static_cast<uint32_t>(i);
        auto sum = c[ui];
        for (uint32_t j = ui + 1; j < n; j++) {
            sum -= mat(ui, j) * d[j];
        }
        d[ui] = std::max(1e-4, sum / mat(ui, ui));
    }
    return d;
}
/* calcFreq: 计算基本块的执行频率 */
BlockTripCountResult calcFreq(MIRFunction& mfunc, CFGAnalysis& cfg) {
    constexpr bool DebugFreq = false;
    BlockTripCountResult res;
    auto& storage = res.storage();

    constexpr size_t maxSupportedBlockSize = 1000;  // 函数最大支持块数
    if (mfunc.blocks().size() > maxSupportedBlockSize) return res;

    /* 1. 块编号 */
    uint32_t allocateID = 0;
    std::unordered_map<MIRBlock*, uint32_t> nodeMap;
    for (auto& block : mfunc.blocks()) nodeMap.emplace(block.get(), allocateID++);

    /* 2. 构造求解问题 */
    const auto n = mfunc.blocks().size();    
    std::vector<double> a(n * n);  // a表示基本块之间的跳转关系
    const auto mat = [&](uint32_t i, uint32_t j) -> double& { return a[i * n + j]; };
    for (uint32_t i = 0; i < n; i++) mat(i, i) = 1.0;  // 初始化为1表示每个基本块至少被执行一次

    const auto addEdge = [&](uint32_t u, MIRBlock* v, double prob) { mat(nodeMap.at(v), u) -= prob; };
    for (auto& block : mfunc.blocks()) {
        const auto u = nodeMap.at(block.get());
        for (auto [succ, prob] : cfg.successors(block.get())) {
            addEdge(u, succ, prob);
        }
    }

    /* 3. 计算基本块的执行频率 */
    auto d = LUP(n, a);
    for (auto& block : mfunc.blocks()) {
        storage.emplace(block.get(), d[nodeMap.at(block.get())]);
    }
    if (DebugFreq) res.dump(std::cerr);
    return res;
}
}