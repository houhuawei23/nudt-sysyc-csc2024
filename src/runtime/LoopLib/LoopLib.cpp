#include "LoopLib.hpp"
#include <iostream>
#include <thread>
MultiThreadLib::MultiThreadLib() {}

MultiThreadLib::~MultiThreadLib() {}

void MultiThreadLib::parallelFor(int32_t beg,
                                 int32_t end,
                                 void (*func)(int32_t, int32_t)) {
  int numThreads = std::thread::hardware_concurrency();
  int32_t range = end - beg;
  int32_t chunkSize = (range + numThreads - 1) / numThreads;

  std::vector<pthread_t> threads(numThreads);
  std::vector<ThreadData> threadData(numThreads);

  for (int i = 0; i < numThreads; ++i) {
    int32_t chunkBeg = beg + i * chunkSize;
    int32_t chunkEnd = std::min(chunkBeg + chunkSize, end);

    if (chunkBeg >= end)
      break;

    threadData[i].beg = chunkBeg;
    threadData[i].end = chunkEnd;
    threadData[i].func = func;

    pthread_create(&threads[i], nullptr, threadFunc, &threadData[i]);
  }

  for (int i = 0; i < numThreads; ++i) {
    if (threads[i]) {
      pthread_join(threads[i], nullptr);
    }
  }
}

void* MultiThreadLib::threadFunc(void* arg) {
  ThreadData* data = static_cast<ThreadData*>(arg);
  data->func(data->beg, data->end);
  return nullptr;
}
