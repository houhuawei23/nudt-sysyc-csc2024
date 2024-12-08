#pragma once

#include <cstdint>
#include <utility>
#include <vector>

namespace utils {
using NodeIndex = uint32_t;
using Graph = std::vector<std::vector<NodeIndex>>;

std::pair<NodeIndex, std::vector<NodeIndex>> calcSCC(const Graph& graph);

std::vector<uint32_t> topologicalSort(const Graph& graph,
                                      NodeIndex ccnt,
                                      std::vector<NodeIndex> col);
}  // namespace utils