#pragma once
#include "mir/MIR.hpp"
#include "mir/target.hpp"
#include "target/riscv/RISCV.hpp"
#include "mir/CFGAnalysis.hpp"
#include "support/Graph.hpp"

namespace mir {
using NodeIndex = uint32_t;
using BlockSeq = std::vector<NodeIndex>;

struct BranchEdge final {
    NodeIndex src, dst;
    double prob;
};

static BlockSeq solvePettisHansen(const std::vector<uint32_t>& weights, 
                                  const std::vector<double>& freq, 
                                  const std::vector<BranchEdge>& edges);

void optimizeBlockLayout(MIRFunction* mfunc, CodeGenContext& ctx);
}