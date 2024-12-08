#pragma once
#include "pass/optimize/optimize.hpp"
#include "pass/optimize/Loop/LoopParallel.hpp"
#include "pass/analysis/ControlFlowGraph.hpp"
#include "pass/optimize/Loop/LoopBodyExtract.hpp"
#include "pass/optimize/Loop/ParallelBodyExtract.hpp"
#include "pass/optimize/Utils/BlockUtils.hpp"
#include "pass/analysis/MarkParallel.hpp"

using namespace ir;

namespace pass {
bool checkLoopParallel(Loop* loop,
                       LoopInfo* lpctx,
                       IndVarInfo* indVarctx,
                       ParallelInfo* parallelctx,
                       std::unordered_set<Loop*>& extractedLoops);

bool fixLoopLatch(Function* func, Loop* loop, IndVar* indVar, TopAnalysisInfoManager* tp);
}  // namespace pass