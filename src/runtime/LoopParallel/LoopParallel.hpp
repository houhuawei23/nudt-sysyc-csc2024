
#include <cstdint>
#include <atomic>

#include <sys/syscall.h>

#include <linux/futex.h>

#include <sched.h>

#include <unistd.h>

#include <bits/types.h>

#include <stddef.h>

#include <stdio.h>
constexpr uint32_t maxThreads = 4;
constexpr auto stackSize = 1024 * 1024;  // 1MB
constexpr auto threadCreationFlags =
  CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM;
/*
CLONE_VM: share same memory space
CLONE_FS: share same filesystem information
CLONE_FILES: share the same file descriptor table
CLONE_SIGHAND: share the same table of signal handlers
CLONE_THREAD: create a new thread
CLONE_SYSVSEM: share the same System V semaphore table
*/
using CmmcForLoop = void (*)(int32_t beg, int32_t end);

namespace {
class Futex final {
  std::atomic_uint32_t storage;

public:
  /* waiting until a certain condition (storage) becomes true */
  void wait() {
    uint32_t one = 1;
    /* compare storage with 1,
    if equal, set it to 0 and return,
    if not, wait for a signal */
    while (!storage.compare_exchange_strong(one, 0)) {
      one = 1;
      syscall(SYS_futex, reinterpret_cast<long>(&storage), FUTEX_WAIT, 0, nullptr, nullptr, 0);
    }
    /* if storage == 1, wair for storage to become 1;
    if storage == 0, wait for storage to become 1. */
  }

  void post() {
    uint32_t zero = 0;
    if (storage.compare_exchange_strong(zero, 1)) {
      syscall(SYS_futex, reinterpret_cast<long>(&storage), FUTEX_WAKE, 1, nullptr, nullptr, 0);
    }
  }
};

struct Worker final {
  pid_t pid;
  void* stack;
  std::atomic_uint32_t core;
  std::atomic_uint32_t run;  // 1: running, 0: idle
  std::atomic<CmmcForLoop> func;
  std::atomic_int32_t beg;
  std::atomic_int32_t end;

  Futex ready, done;
};

Worker workers[maxThreads];  // NOLINT

static_assert(std::atomic_uint32_t::is_always_lock_free);
static_assert(std::atomic_int32_t::is_always_lock_free);
static_assert(std::atomic<void*>::is_always_lock_free);
static_assert(std::atomic<CmmcForLoop>::is_always_lock_free);

int cmmcWorker(void* ptr) {
  auto& worker = *static_cast<Worker*>(ptr);
  /* set thread's cpu affinity */
  {
    cpu_set_t set;
    CPU_SET(worker.core, &set);
    auto pid = static_cast<pid_t>(syscall(SYS_gettid));
    sched_setaffinity(pid, sizeof(set), &set);
  }
  while (worker.run) {
    // wait for task
    worker.ready.wait();
    if (!worker.run) break;
    // exec task
    std::atomic_thread_fence(std::memory_order_seq_cst);
    worker.func.load()(worker.beg.load(), worker.end.load());
    std::atomic_thread_fence(std::memory_order_seq_cst);

    // fprintf(stderr, "finish %d %d\n", worker.beg.load(), worker.end.load());
    // isignal completion
    worker.done.post();
  }
  return 0;
}
}  // namespace
extern "C" {
void parallelFor(int32_t beg, int32_t end, CmmcForLoop func);
}