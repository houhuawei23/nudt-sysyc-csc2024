#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include "ir/ir.hpp"
#include "pass/pass.hpp"

using namespace ir;
namespace pass {

struct Reg2MemContext final {
  std::vector<ir::PhiInst*> allphi;
  std::vector<size_t> parent;
  std::vector<size_t> rank;
  std::map<ir::PhiInst*, ir::AllocaInst*> phiweb;
  std::vector<ir::AllocaInst*> allocasToinsert;
  std::map<ir::PhiInst*, ir::LoadInst*> philoadmap;
  std::unordered_map<ir::BasicBlock*, std::vector<ir::PhiInst*>> bbphismap;
  std::vector<ir::BasicBlock*> phiblocks;

  void run(Function* func, TopAnalysisInfoManager* tp);
  void getallphi(ir::Function* func);
  // 并查集算法
  int getindex(ir::PhiInst* phiinst) {
    auto it = std::find(allphi.begin(), allphi.end(), phiinst);
    if (it != allphi.end()) {
      size_t index = std::distance(allphi.begin(), it);
      return index;
    } else {
      return -1;
    }
  }
  int find(int x) { return x == parent[x] ? x : (parent[x] = find(parent[x])); }
  bool issame(int x, int y) { return find(x) == find(y); }
  void tounion(int x, int y) {
    int f0 = find(x);
    int f1 = find(y);
    if (rank[f0] > rank[f1]) {
      parent[f1] = f0;
    } else {
      parent[f0] = f1;
      if (rank[f0] == rank[f1]) {
        rank[f1]++;
      }
    }
  }
  void DisjSet();
};

class Reg2Mem : public FunctionPass {
public:
  std::string name() const override { return "Reg2Mem"; }
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
};
}  // namespace pass