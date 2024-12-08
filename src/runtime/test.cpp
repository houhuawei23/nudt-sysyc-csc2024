#include <iostream>
#include "LoopLib.hpp"

void exampleFunc(int32_t beg, int32_t end) {
  for (int32_t i = beg; i < end; ++i) {
    std::cout << "Processing index: " << i << std::endl;
  }
}

int main() {
  MultiThreadLib mtLib;
  mtLib.parallelFor(0, 100, exampleFunc);
  return 0;
}