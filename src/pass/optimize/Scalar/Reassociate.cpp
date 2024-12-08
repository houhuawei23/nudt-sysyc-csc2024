#include "pass/optimize/reassociate.hpp"
using namespace pass;

static std::vector<ir::BasicBlock*> RPOVector;
static std::unordered_map<ir::BasicBlock*, bool> vis;
static std::unordered_map<ir::Value*, int> rankMap;

void Reassociate::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  if (func->isOnlyDeclare()) return;
  // initialize
  RPOVector.clear();
  vis.clear();
  rankMap.clear();
  // get RPO of BBs in func
  for (auto bb : func->blocks())
    vis[bb] = false;
  DFSPostOrderBB(func->entry());
  std::reverse(RPOVector.begin(), RPOVector.end());
  // calculate Rank Map
}

// dfs whole cfg in post order and get post order rank of bbs
void Reassociate::DFSPostOrderBB(ir::BasicBlock* bb) {
  vis[bb] = true;
  for (auto bbNext : bb->next_blocks()) {
    if (not vis[bb]) DFSPostOrderBB(bbNext);
  }
  RPOVector.push_back(bb);
}

// to build rank map
void Reassociate::buildRankMap() {}