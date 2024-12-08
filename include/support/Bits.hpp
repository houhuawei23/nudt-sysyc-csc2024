#pragma once
#include <cstdint>

namespace utils {
/* 功能: 判断数x是否为2的幂次 */
inline bool isPowerOf2(size_t x) {
  return __builtin_popcountll(x) == 1;
}
/* 功能: log2(x) */
inline size_t log2(size_t x) {
  return __builtin_ctzll(x);
}
}  // namespace utils