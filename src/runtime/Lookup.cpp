#include <cstdint>

extern "C" {
constexpr uint32_t m1 = 1021, m2 = 1019;
struct LUTEntry final {
  uint64_t key;
  int val;
  int hasVal;
};
static_assert(sizeof(LUTEntry) == sizeof(uint32_t) * 4);
LUTEntry* sysycCacheLookup(LUTEntry* table, int key1, int key2) {
  const auto key = static_cast<uint64_t>(key1) << 32 | static_cast<uint64_t>(key2);
  const auto ha = key % m1, hb = 1 + key % m2;
  auto cur = ha;
  constexpr uint32_t maxLookupCount = 5;
  uint32_t count = maxLookupCount;
  while (true) {
    auto& ref = table[cur];
    if (!ref.hasVal) {
      ref.key = key;
      return &ref;
    }
    if (ref.key == key) {
      return &ref;
    }
    if (++count >= maxLookupCount)
      break;
    cur += hb;
    if (cur >= m1)
      cur -= m1;
  }
  // evict, FIFO
  auto& ref = table[ha];
  ref.hasVal = 0;
  ref.key = key;
  return &ref;
}
}