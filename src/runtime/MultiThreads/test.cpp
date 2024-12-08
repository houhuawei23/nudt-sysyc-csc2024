#include <chrono>
#include <iostream>
#include "MultiThreads.hpp"

using namespace std;

using namespace std::string_view_literals;
using Clock = std::chrono::high_resolution_clock;
using Duration = Clock::duration;
using TimePoint = Clock::time_point;

int globalSum;

void exampleFunc(int32_t beg, int32_t end) {
  int localSum = 0;
  int tmp = 0;
  for (int32_t i = beg; i < end; ++i) {
    for(int32_t j = 0; j < 5000; ++j) {
      tmp += j;
    }

    localSum += 1;
  }
  globalSum += localSum;
}

int main() {
  int32_t beg = 0;
  int32_t end = 1e7;
  TimePoint starttime = Clock::now();

  // for (auto i = 0; i < 200; ++i) {
  //   parallelFor(beg, end, exampleFunc);
  // }
  parallelFor(beg, end, exampleFunc);

  TimePoint endtime = Clock::now();
  std::cout << "Parallel: " << std::endl;
  std::cout << "Global sum: " << globalSum << std::endl;
  Duration parallel = endtime - starttime;
  std::cout << "Elapsed time: "
            << std::chrono::duration_cast<std::chrono::microseconds>(parallel).count()
            << " microseconds" << std::endl;

  globalSum = 0;
  starttime = Clock::now();
  // for (auto i = 0; i < 200; ++i) {
  //   exampleFunc(beg, end);
  // }
  exampleFunc(beg, end);

  endtime = Clock::now();

  std::cout << "Sequential: " << std::endl;
  std::cout << "Global sum: " << globalSum << std::endl;
  Duration sequential = endtime - starttime;
  std::cout << "Elapsed time: "
            << std::chrono::duration_cast<std::chrono::microseconds>(sequential).count()
            << " microseconds" << std::endl;

  auto speedup = sequential * 1.0 / parallel;
  std::cout << "Speedup: " << speedup << std::endl;
  return 0;
}