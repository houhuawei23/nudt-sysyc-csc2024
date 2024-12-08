#include "support/Graph.hpp"
#include <vector>
#include <queue>
#include <unordered_set>

namespace utils {
std::pair<NodeIndex, std::vector<NodeIndex>> calcSCC(const Graph& graph) {
  const auto size = graph.size();
  std::vector<NodeIndex> dfn(size), low(size), st(size), col(size);
  NodeIndex top = 0, ccnt = 0, icnt = 0;
  std::vector<bool> flag(size);
  const auto dfs = [&](auto&& self, NodeIndex u) -> void {
    dfn[u] = low[u] = ++icnt;
    flag[u] = true;
    st[top++] = u;
    for (auto v : graph[u]) {
      if (dfn[v]) {
        if (flag[v]) low[u] = std::min(low[u], dfn[v]);
      } else {
        self(self, v);
        low[u] = std::min(low[u], low[v]);
      }
    }
    if (dfn[u] == low[u]) {
      NodeIndex c = ccnt++, v;
      do {
        v = st[--top];
        flag[v] = false;
        col[v] = c;
      } while (u != v);
    }
  };

  for (NodeIndex i = 0; i < size; ++i)
    if (!dfn[i]) dfs(dfs, i);
  return {ccnt, std::move(col)};
}

std::vector<uint32_t> topologicalSort(const Graph& graph,
                                      NodeIndex ccnt,
                                      std::vector<NodeIndex> col) {
  std::vector<std::unordered_set<NodeIndex>> dag(ccnt);
  std::vector<std::vector<utils::NodeIndex>> groups(ccnt);
  std::vector<uint32_t> in(ccnt);
  /* topological sort */
  for (uint32_t u = 0; u < graph.size(); ++u) {
    const auto cu = col[u];
    groups[cu].push_back(u);
    for (auto v : graph[u]) {
      const auto cv = col[v];
      if (cu != cv && dag[cu].emplace(cv).second) {
        ++in[cv];
      }
    }
  }

  std::queue<uint32_t> q;
  for (uint32_t u = 0; u < ccnt; ++u)
    if (in[u] == 0) q.push(u);
  std::vector<uint32_t> order;
  order.reserve(graph.size());
  while (!q.empty()) {
    const auto u = q.front();
    q.pop();

    const auto& group = groups[u];
    order.insert(order.end(), group.cbegin(), group.cend());

    for (auto v : dag[u]) {
      if (--in[v] == 0) {
        q.push(v);
      }
    }
  }
  return std::move(order);
}

}  // namespace utils
