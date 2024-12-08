#include "mir/MIR.hpp"
#include "mir/target.hpp"
#include "mir/CFGAnalysis.hpp"
#include "mir/LiveInterval.hpp"
#include "mir/RegisterAllocator.hpp"
#include "support/StaticReflection.hpp"
#include "target/riscv/RISCV.hpp"
#include <vector>
#include <stack>
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <optional>

namespace mir {

void InterferenceGraph::add_edge(RegNum lhs, RegNum rhs) {
  assert(lhs != rhs);
  assert((isVirtualReg(lhs) || isISAReg(lhs)) && (isVirtualReg(rhs) || isISAReg(rhs)));
  if (mAdj[lhs].count(rhs)) return;
  mAdj[lhs].insert(rhs);
  mAdj[rhs].insert(lhs);

  /*
   NOTE: 干涉图的节点可以为虚拟寄存器 OR 物理寄存器
   但是我们仅仅只考虑对虚拟寄存器进行相关物理寄存器的指派和分配
   故: 我们仅仅只计算虚拟寄存器节点的度数, 不考虑计算物理寄存器节点的度数
   */
  if (isVirtualReg(lhs)) ++mDegree[lhs];
  if (isVirtualReg(rhs)) ++mDegree[rhs];
}
void InterferenceGraph::prepare_for_assign(const RegWeightMap& weights, uint32_t k) {
  mQueue = Queue{RegNumComparator{&weights}};
  for (auto& [reg, degree] : mDegree) {
    if (degree < k) { /* 度数小于k, 可进行图着色分配 */
      assert(isVirtualReg(reg));
      mQueue.push(reg);
    }
  }
}

RegNum InterferenceGraph::pick_to_assign(uint32_t k) {
  if (mQueue.empty()) return invalidReg;
  auto u = mQueue.top();
  mQueue.pop();
  assert(isVirtualReg(u));
  assert(adj(u).size() < k);

  if (auto iter = mAdj.find(u); iter != mAdj.cend()) {
    for (auto v : mAdj.at(u)) {
      if (isVirtualReg(v)) {
        if (mDegree[v] == k) mQueue.push(v);
        --mDegree[v];
      }
      mAdj[v].erase(u);
    }
    mAdj.erase(iter);
  }
  mDegree.erase(u);
  return u;
}

RegNum InterferenceGraph::pick_to_spill(const std::unordered_set<RegNum>& blockList,
                                        const RegWeightMap& weights,
                                        uint32_t k) const {
  constexpr uint32_t fallbackThreshold = 0;  // 根据blockList的大小选择不同的策略
  RegNum best = invalidReg;
  double minWeight = 1e40;  // 最小权值
  if (blockList.size() >= fallbackThreshold) {
    // 策略1: 选择度数最大且权值最小的节点来将其spill到内存中
    uint32_t maxDegree = 0;  // 最大度数
    for (auto& [reg, degree] : mDegree) {
      if (degree >= maxDegree && !blockList.count(reg)) {
        if (maxDegree == degree && weights.at(reg) >= minWeight) continue;
        maxDegree = degree;
        minWeight = weights.at(reg);
        best = reg;
      }
    }
  } else {  // 策略2: 选择权值最小的大于k的节点来将其spill到内存
    for (auto& [reg, degree] : mDegree) {
      if (degree >= k && !blockList.count(reg) && weights.at(reg) < minWeight) {
        best = reg;
        minWeight = weights.at(reg);
      }
    }
  }
  assert(best != invalidReg && isVirtualReg(best));
  return best;
}

std::vector<RegNum> InterferenceGraph::collect_nodes() const {
  std::vector<RegNum> vregs;
  vregs.reserve(mDegree.size());
  for (auto [reg, degree] : mDegree)
    vregs.push_back(reg);
  return vregs;
}

void InterferenceGraph::dump(std::ostream& out) const {
  for (auto& [vreg, degree] : mDegree) {
    out << (vreg ^ virtualRegBegin) << "[" << degree << "]: ";
    for (auto adj : mAdj.at(vreg)) {
      if (isVirtualReg(adj))
        out << "v";
      else
        out << "i";
      out << (isVirtualReg(adj) ? adj ^ virtualRegBegin : adj) << " ";
    }
    out << "\n";
  }
}
}  // namespace mir