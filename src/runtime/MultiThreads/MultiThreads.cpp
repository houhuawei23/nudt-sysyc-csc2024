#include "MultiThreads.hpp"
#include <cstdio>
#include <unistd.h>
#include <cstdint>
#include <array>
#include <algorithm>
#include <sys/mman.h>
#include <sys/wait.h>
// #include <iostream>
// #include <fstream>
#include <cassert>

#define alignTo(size, align) ((size) + (align) - 1) / (align) * (align)

// void Worker::dump(std::ostream& os) const {
//   os << "worker: \n";
//   os << "  pid: " << pid << "\n";
//   os << "  stackAddr: " << stackAddr << "\n";
//   os << "  core: " << core << "\n";
//   os << "  status: " << status << "\n";
//   os << "  func: " << func << "\n";
//   os << "  beg: " << beg << "\n";
//   os << "  end: " << end << "\n";
// }
// void Worker::dump() const {

// }

Worker workers[maxThreads];

/* wiat for the storage to be 1 */
void Futex::wait() {
  uint32_t one = 1;
  /* compare storage with 1,
  if equal, set it to 0 and return,
  if not, wait for a signal */
  while (!storage.compare_exchange_strong(one, 0)) {
    one = 1;
    syscall(SYS_futex, reinterpret_cast<long>(&storage), FUTEX_WAIT, 0, nullptr, nullptr, 0);
  }
}

/* set the storage to 1 */
void Futex::post() {
  uint32_t zero = 0;
  if (storage.compare_exchange_strong(zero, 1)) {
    syscall(SYS_futex, reinterpret_cast<long>(&storage), FUTEX_WAKE, 1, nullptr, nullptr, 0);
  }
}


int workerRun(void* workerPtr) {
  auto& worker = *static_cast<Worker*>(workerPtr);

  // FIXME: IOT instruction (core dumped)
  // std::ofstream fout("worker" + std::to_string(worker.core) + ".log");
  // if(not fout.is_open()) {
  //   std::cerr << "Failed to open worker log file" << std::endl;
  //   assert(false);
  //   return 1;
  // }
  // worker.dump(fout);
  // fout.close();
  cpu_set_t set;
  CPU_SET(worker.core, &set);
  auto pid = static_cast<pid_t>(syscall(SYS_gettid));
  sched_setaffinity(pid, sizeof(cpu_set_t), &set);

  while (worker.status) {
    // wait for worker to be ready
    worker.ready.wait();
    // run the loop function
    std::atomic_thread_fence(std::memory_order_seq_cst);
    worker.func.load()(worker.beg.load(), worker.end.load());
    std::atomic_thread_fence(std::memory_order_seq_cst);
    fprintf(stderr, "finish %d %d\n", worker.beg.load(), worker.end.load());
    // signal worker is done
    worker.done.post();
  }
  return 0;
}

__attribute((constructor)) void initRuntime() {
  // printf("initRuntime begin\n");
  fprintf(stderr, "initRuntime begin\n");
  for (auto i = 0; i < maxThreads; i++) {
    auto& worker = workers[i];
    worker.status = 1;

    constexpr auto protFlags = PROT_READ | PROT_WRITE;
    constexpr auto mapFlags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK;

    worker.stackAddr = mmap(nullptr, stackSize, protFlags, mapFlags, -1, 0);
    worker.core = i;
    worker.pid =
        clone(workerRun, static_cast<uint8_t*>(worker.stackAddr) + stackSize,
              threadCreationFlags, &worker);
  }
  // printf("initRuntime end\n");
  fprintf(stderr, "initRuntime end\n");
}

__attribute((destructor)) void destroyRuntime() {
  // printf("destroyRuntime begin\n");
  fprintf(stderr, "destroyRuntime begin\n");
  // for (auto& worker : workers) {
  //   worker.status = 0;
  //   worker.ready.post();
  //   waitpid(worker.pid, nullptr, 0);
  //   // munmap(worker.stackAddr, stackSize);
  // }
  // printf("destroyRuntime end\n");
  fprintf(stderr, "destroyRuntime\n");
}

std::size_t getNumThreads() {
  return maxThreads;
}

void parallelFor(int32_t beg, int32_t end, LoopFuncHeader func) {
  const auto size = static_cast<std::size_t>(end - beg);
  constexpr std::size_t smallTaskSize = 32;
  if (size <= smallTaskSize) {
    func(beg, end);
    return;
  }

  auto spawnAndJoin = [&](std::size_t threads) {
    if (threads == 1) {
      func(beg, end);
      return;
    }

    // fprintf(stderr, "parallel for %d %d\n", beg, end);

    std::size_t align = 4;
    const auto inc = static_cast<int32_t>(alignTo(size / threads, align));

    std::array<bool, maxThreads> assigned{};
    for (std::size_t i = 0; i < threads; i++) {
      const auto subBeg = beg + static_cast<int32_t>(i) * inc;
      auto subEnd = std::min(subBeg + inc, end);
      if (i == threads - 1) {
        subEnd = end;
      }
      if (subBeg >= subEnd)
        continue;

      auto& worker = workers[static_cast<size_t>(i)];
      worker.func = func;
      worker.beg = subBeg;
      worker.end = subEnd;
      fprintf(stderr, "worker.ready.post() %ld [%d, %d)\n", i, subBeg, subEnd);
      // signal worker to be ready
      worker.ready.post();
      assigned[i] = true;
    }
    for (std::size_t i = 0; i < threads; i++) {
      if (assigned[i]) {
        workers[i].done.wait();
      }
    }
  };

  std::size_t threads;
  threads = getNumThreads();
  fprintf(stderr, "parallel for %d %d, threads %ld\n", beg, end, threads);
  spawnAndJoin(threads);
}
