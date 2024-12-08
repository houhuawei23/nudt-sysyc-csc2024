#include "mir/BlockLayoutOpt.hpp"
#include <limits>
namespace mir {
static std::string blockPlacementAlgo = "Pettis-Hansen";

static BlockSeq solvePettisHansen(const std::vector<uint32_t>& weights, 
                                  const std::vector<double>& freq, 
                                  const std::vector<BranchEdge>& edges) {
    const auto blockCnt = weights.size();
    constexpr bool Debug = false;

    /* Stage1: chain composition */
    std::vector<uint32_t> fa;  // 存储每个基本块的父亲节点
    fa.reserve(blockCnt);
    std::vector<std::pair<uint32_t, std::list<uint32_t>>> chains;
    chains.reserve(blockCnt);
    for (uint32_t idx = 0; idx < blockCnt; ++idx) {
        chains.push_back({ std::numeric_limits<uint32_t>::max(), {idx} });
        fa.push_back(idx);
    }

    std::vector<std::pair<BranchEdge, double>> edgeInfo;
    edgeInfo.reserve(edges.size());
    for (auto& edge : edges) {
        edgeInfo.emplace_back(edge, freq[edge.src] * edge.prob);
    }

    /* 并查集 - 查找基本块的父节点以及进行路径压缩 */
    const auto findFaImpl = [&](uint32_t u, auto&& self) {
        if (fa[u] == u) return u;
        return fa[u] = self(fa[u], self);
    };
    const auto findFa = [&](uint32_t u) { return findFaImpl(u, findFaImpl); };

    std::sort(edgeInfo.begin(), edgeInfo.end(), // 按照递减的顺序进行排序
              [](auto& lhs, auto& rhs) { return lhs.second > rhs.second; });
    
    utils::Graph graph(blockCnt);  // 邻接矩阵构造图
    uint32_t p = 0;
    if (Debug) {
        std::cerr << "begin debug the algorithm\n";
    }

    for (auto& [edge, info] : edgeInfo) {
        auto& [src, dst, prob] = edge;
        if (Debug) {
            std::cerr << src << " ->" << dst;
            std::cerr << "[weights = " << info;
            std::cerr << ", color = ";
        }
        if (src == dst) {
            if (Debug) std::cerr << "blue]\n";
            continue;
        }
        
        graph[src].push_back(dst);

        auto& [parentDst, sonsDst] = chains[dst];
        if (findFa(dst) != dst) {
            if (Debug) std::cerr << "blue]\n";
            continue;  // merged
        }

        const auto faSrc = findFa(src);
        if (faSrc == dst) {
            if (Debug) std::cerr << "blue]\n";
            continue;  // merged
        }

        auto& [parentSrc, sonsSrc] = chains[faSrc];
        if (sonsSrc.back() != src) {
            if (Debug) std::cerr << "blue]\n";
            continue;  // merged
        }

        parentSrc = std::min(std::min(parentSrc, parentDst), ++p);
        sonsSrc.splice(sonsSrc.cend(), sonsDst);
        fa[dst] = faSrc;
        if (Debug) std::cerr << "red]\n";
    }

    /* Stage2: code layout */
    assert(findFa(0) == 0);

    std::priority_queue<uint32_t, std::vector<uint32_t>, std::function<bool(uint32_t, uint32_t)>> workList {
        [&](uint32_t lhs, uint32_t rhs) { return chains[lhs].first > chains[rhs].first; }
    };
    workList.push(0);

    std::unordered_set<uint32_t> inserted;
    std::unordered_set<uint32_t> insertedWorkList;
    insertedWorkList.insert(0);
    
    std::vector<uint32_t> seq; seq.reserve(blockCnt);
    while (!workList.empty()) {
        auto k = workList.top();
        workList.pop();

        for (auto u : chains[k].second) {
            if (inserted.insert(u).second) {
                seq.push_back(u);
            }
        }
        for (auto u : chains[k].second) {
            for (auto v : graph[u]) {
                if (inserted.count(v)) continue;
                auto head = findFa(v);
                if (insertedWorkList.insert(head).second) {
                    workList.push(head);
                }
            }
        }
    }
    
    return seq;
}

/*
 * @brief: optimizeBlockLayout function
 * @note: 
 *    Pettis-Hansen基本块调度算法
 *    核心: 重新排列基本块的顺序, 以减少指令缓存失效和分支预测错误
 *    核心思想: 将程序中的热路径紧密排列在一起, 以最大化指令缓存命中率
 * @details: 
 *    1. 控制流图构建: 分析程序的控制流, 生成控制流图
 *    2. 概率分析: 通过静态分析或动态分析确定每条边的执行频率
 *    3. 路径选择: 选择频率最高的路径作为主路径
 *    4. 基本块排列: 将主路径上的基本块紧密排列并尽量减少跨越冷路径的跳转
 */
void optimizeBlockLayout(MIRFunction* mfunc, CodeGenContext& ctx) {
    if (mfunc->blocks().size() <= 2) return;

    /* 1. Build Graph */
    auto cfg = calcCFG(*mfunc, ctx);
    auto blockFreq = calcFreq(*mfunc, cfg);

    /* weights - 存储每个MIRBlock大小 (代码体积) */
    std::vector<uint32_t> weights; weights.reserve(mfunc->blocks().size());
    /* edges - 存储BranchEdge (跳转边 src --prob--> dst) */
    std::vector<BranchEdge> edges;
    /* idxMap - 给每个MIRBlock编号 */
    std::unordered_map<MIRBlock*, uint32_t> idxMap;
    /* freq - 存储每个MIRBlock的执行频率 */
    std::vector<double> freq;
    {
        uint32_t idx = 0;
        for (auto& block : mfunc->blocks()) {
            idxMap[block.get()] = idx++;
            weights.emplace_back(block->insts().size());  // estimate code size
        }

        idx = 0;
        freq.reserve(weights.size());
        for (auto& block : mfunc->blocks()) {
            const auto blockIdx = idx++;
            freq.emplace_back(blockFreq.query(block.get()));
            for (auto [suc, prob] : cfg.successors(block.get())) {
                assert(idxMap.count(suc));
                edges.push_back({ blockIdx, idxMap.at(suc), prob });
            }
        }
    }

    /* 2. Sort Graph */
    BlockSeq seq;
    if (blockPlacementAlgo == "Pettis-Hansen") {
        seq = solvePettisHansen(weights, freq, edges);
    } else {
        return;
    }
    assert(seq[0] == 0);  // entry block

    /* 3. Apply Changes */
    std::vector<std::unique_ptr<MIRBlock>> newBlocks;
    newBlocks.reserve(mfunc->blocks().size());
    for (auto& block : mfunc->blocks()) {
        newBlocks.emplace_back(std::move(block));
    }

    mfunc->blocks().clear();
    for (auto idx : seq) {
        mfunc->blocks().emplace_back(std::move(newBlocks[idx]));
    }

    for (auto iter = mfunc->blocks().cbegin(); iter != mfunc->blocks().cend(); ) {
        auto& block = *iter;
        const auto nextIter = std::next(iter);

        auto& terminator = block->insts().back();
        const auto ensureNext = [&](MIRBlock* next) {
            if (nextIter == mfunc->blocks().cend() || nextIter->get() != next) {
                auto newBlock = std::make_unique<MIRBlock>(mfunc, "label" + std::to_string(ctx.nextLabelId()));
                auto inst = new MIRInst(RISCV::J); inst->set_operand(0, MIROperand::asReloc(next));
                newBlock->insts().emplace_back(inst);
                mfunc->blocks().insert(nextIter, std::move(newBlock));
            }
        };

        MIRBlock* targetBlock;
        double prob;
        if (ctx.instInfo.matchConditionalBranch(terminator, targetBlock, prob)) {
            const auto& successors = cfg.successors(block.get());
            if (successors.size() == 2) {
                if (targetBlock == successors[0].block) {
                    ensureNext(successors[1].block);
                } else {
                    ensureNext(successors[0].block);
                }
            } else if (successors.size() == 1) {
                ensureNext(successors[0].block);
            } else {
                assert(false);
            }
        }

        iter = nextIter;
    }
}
}