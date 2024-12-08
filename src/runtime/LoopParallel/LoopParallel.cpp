#include "LoopParallel.hpp"

#include <array>
#include <algorithm>
#include <limits>
#include <ctime> /* time functions */

#include <sys/mman.h>
#include <sys/wait.h>

#include <bits/types/struct_timespec.h>

/* make sure names inside 'extern "C"' are not changed by mangling */
extern "C" {
/* execute before main() */
__attribute((constructor)) void cmmcInitRuntime() {
  for (uint32_t i = 0; i < maxThreads; ++i) {
    auto& worker = workers[i];
    worker.run = 1;
    worker.stack = mmap(nullptr, stackSize, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    worker.core = i;
    worker.pid = clone(cmmcWorker, static_cast<uint8_t*>(worker.stack) + stackSize,
                       threadCreationFlags, &worker);
  }
}
/* execute after main() */
__attribute((destructor)) void cmmcUninitRuntime() {
  for (auto& worker : workers) {
    worker.run = 0;
    worker.ready.post();
    waitpid(worker.pid, nullptr, 0);
  }
  // FIXME
  // for(auto& worker : workers)
  //     munmap(worker.stack, stackSize);
}
using Time = int64_t;
struct ParallelForEntry final {
  CmmcForLoop func;
  uint32_t size;
  bool valid;
  uint32_t hitCount;
  static constexpr uint32_t sampleThreshold = 100;
  static constexpr uint32_t sampleCount = 20;
  static constexpr uint32_t stopSampleThreshold = sampleThreshold + 3 * sampleCount;
  Time times[3];  // 1T 2T 4T
  uint32_t bestThreads;
};
constexpr uint32_t entryCount = 16;
static ParallelForEntry parallelCache[entryCount];  // NOLINT
static uint32_t lookupPtr;                          // NOLINT
static ParallelForEntry& selectEntry(CmmcForLoop func, uint32_t size) {
  // fprintf(stderr, "lookup %p %d\n", func, size);
  for (uint32_t i = 0; i < entryCount; ++i, ++lookupPtr) {
    if (lookupPtr == entryCount) lookupPtr = 0;
    auto& entry = parallelCache[lookupPtr];
    if (entry.valid && entry.func == func && entry.size == size) {
      entry.hitCount++;
      return entry;
    }
  }
  // select an empty slot
  for (uint32_t i = 0; i < entryCount; ++i) {
    auto& entry = parallelCache[i];
    if (!entry.valid) {
      entry.valid = true;
      entry.func = func;
      entry.size = size;
      entry.hitCount = 1;
      lookupPtr = i;
      return entry;
    }
  }
  // evict
  uint32_t minHitCount = std::numeric_limits<uint32_t>::max();
  uint32_t best = 0;
  for (uint32_t i = 0; i < entryCount; ++i) {
    auto& entry = parallelCache[i];
    if (entry.hitCount < minHitCount) {
      best = i;
      minHitCount = entry.hitCount;
    }
  }

  auto& entry = parallelCache[best];
  entry.func = func;
  entry.size = size;
  entry.hitCount = 1;
  lookupPtr = best;
  return entry;
}
static Time getTimePoint() {
  timespec tp;
  clock_gettime(CLOCK_MONOTONIC, &tp);
  return tp.tv_sec * 1'000'000'000LL + tp.tv_nsec;
}
static ParallelForEntry& selectNumberOfThreads(CmmcForLoop func,
                                               uint32_t size,
                                               uint32_t& threads,
                                               bool& sample) {
  auto& entry = selectEntry(func, size);
  // fprintf(stderr, "hitCount %d\n", entry.hitCount);
  if (entry.hitCount < ParallelForEntry::sampleThreshold) {
    threads = 2;
    sample = false;
    return entry;
  }
  // fprintf(stdout, "here\n");
  if (entry.hitCount < ParallelForEntry::stopSampleThreshold) {
    threads =
      ((entry.hitCount - ParallelForEntry::sampleThreshold) / ParallelForEntry::sampleCount);
    sample = true;
    return entry;
  }
  // fprintf(stderr, "hitCount %d\n", entry.hitCount);
  if (!entry.bestThreads) {
    uint32_t best = 0;
    Time minTime = std::numeric_limits<Time>::max();
    for (uint32_t i = 0; i < 3; ++i)
      if (entry.times[i] < minTime) {
        best = i;
        minTime = entry.times[i];
      }
    entry.bestThreads = best;
  }
  threads = entry.bestThreads;
  sample = false;
  return entry;
}

void parallelFor(int32_t beg, int32_t end, CmmcForLoop func) {
  // Handle the case where end <= beg
  if (end == beg) {
    return;
  }

  // Determine if we are iterating forwards or backwards
  const bool isForward = end > beg;

  // Calculate the size of the range
  const auto size = static_cast<uint32_t>(isForward ? end - beg : beg - end);
  constexpr uint32_t smallTask = 16;

  // If the range is too small, execute it directly in the main thread
  if (size < smallTask) {
    func(beg, end);
    return;
  }

  auto spawnAndJoin = [&](uint32_t threads) {
    if (threads == 1) {
      func(beg, end);
      return;
    }

    // fprintf(stderr, "parallel for %d %d\n", beg, end);
    std::atomic_thread_fence(std::memory_order_seq_cst);

    constexpr uint32_t alignment = 4;
    const auto inc =
      static_cast<int32_t>(((size / threads) + alignment - 1) / alignment * alignment);
    std::array<bool, maxThreads> assigned{};

    for (int32_t i = 0; i < static_cast<int32_t>(threads); ++i) {
      int32_t subBeg, subEnd;

      if (isForward) {
        subBeg = beg + i * inc;
        subEnd = std::min(subBeg + inc, end);
      } else {
        subBeg = beg - i * inc;
        subEnd = std::max(subBeg - inc, end);
      }

      if (static_cast<uint32_t>(i) == threads - 1) subEnd = end;

      if (isForward ? subBeg >= subEnd : subBeg <= subEnd) continue;

      // fprintf(stderr, "launch %d %d\n", subBeg, subEnd);
      auto& worker = workers[static_cast<size_t>(i)];
      worker.func = func;
      worker.beg = subBeg;
      worker.end = subEnd;

      // Signal worker
      worker.ready.post(); /* run sub threads */
      assigned[static_cast<size_t>(i)] = true;
    }

    for (uint32_t i = 0; i < threads; ++i) {
      if (assigned[i]) workers[i].done.wait();
    }

    std::atomic_thread_fence(std::memory_order_seq_cst);
  };

  bool sample;
  uint32_t threads;
  auto& entry = selectNumberOfThreads(func, size, threads, sample);
  // fprintf(stderr, "threads %d\n", threads);
  Time start;

  if (sample) start = getTimePoint();

  spawnAndJoin(1 << threads);

  if (sample) {
    const auto stop = getTimePoint();
    const auto diff = stop - start;
    entry.times[threads] += diff;
  }
}

// constexpr uint32_t m1 = 1021, m2 = 1019;
// struct LUTEntry final {
//   uint64_t key;
//   int val;
//   int hasVal;
// };
// static_assert(sizeof(LUTEntry) == sizeof(uint32_t) * 4);
// LUTEntry* cmmcCacheLookup(LUTEntry* table, int key1, int key2) {
//   const auto key =
//       static_cast<uint64_t>(key1) << 32 | static_cast<uint64_t>(key2);
//   const auto ha = key % m1, hb = 1 + key % m2;
//   auto cur = ha;
//   constexpr uint32_t maxLookupCount = 5;
//   uint32_t count = maxLookupCount;
//   while (true) {
//     auto& ref = table[cur];
//     if (!ref.hasVal) {
//       ref.key = key;
//       return &ref;
//     }
//     if (ref.key == key) {
//       return &ref;
//     }
//     if (++count >= maxLookupCount)
//       break;
//     cur += hb;
//     if (cur >= m1)
//       cur -= m1;
//   }
//   // evict, FIFO
//   auto& ref = table[ha];
//   ref.hasVal = 0;
//   ref.key = key;
//   return &ref;
// }
// int32_t cmmcAddRec3SRem(int32_t x, int32_t rem) {
//   const auto n64 = static_cast<int64_t>(x);
//   return static_cast<int32_t>(n64 * (n64 - 1) / 2 % rem);
// }
// void cmmcReduceAddI32(std::atomic_int32_t& x, int32_t val) {
//   x += val;
// }
// void cmmcReduceAddF32(std::atomic<float>& x, float val) {
//   float base = x.load();
//   while (!x.compare_exchange_weak(base, base + val))
//     ;
// }
}
