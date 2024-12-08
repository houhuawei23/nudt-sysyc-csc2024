#pragma once
#include <pthread.h>
#include <functional>
#include <vector>
#include <stdint.h>

class MultiThreadLib {
  public:
  MultiThreadLib();
  ~MultiThreadLib();

  void parallelFor(int32_t beg, int32_t end, void (*func)(int32_t, int32_t));

  private:
  static void* threadFunc(void* arg);

  struct ThreadData {
    int32_t beg;
    int32_t end;
    void (*func)(int32_t, int32_t);
  };
};
