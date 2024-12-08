#pragma once
#include <cstdint>
#include <atomic>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <sched.h>
#include <unistd.h>
#include <bits/types.h>
// #include <iostream>

constexpr std::size_t maxThreads = 4;
constexpr std::size_t stackSize = 1024 * 1024;
constexpr auto threadCreationFlags = CLONE_VM | CLONE_FS | CLONE_FILES |
                                     CLONE_SIGHAND | CLONE_THREAD |
                                     CLONE_SYSVSEM;
using LoopFuncHeader = void (*)(int32_t beg, int32_t end);

class Futex final {
  std::atomic_uint32_t storage;
public:
  void wait();
  void post();
};

struct Worker final {
  pid_t pid;
  void* stackAddr;
  std::atomic_uint32_t core;
  std::atomic_uint32_t status;  // 0: idle, 1: running
  std::atomic<LoopFuncHeader> func;
  std::atomic_int32_t beg, end;

  Futex ready, done;
  // void dump(std::ostream& os) const;
  // void dump() const;
};


int workerRun(void* workerPtr);

extern "C" {
void parallelFor(int32_t beg, int32_t end, LoopFuncHeader func);
}