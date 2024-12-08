#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <deque>
#include <chrono>
#include <memory>
#include <cstdint>
using namespace std::string_view_literals;

namespace utils {

using Clock = std::chrono::high_resolution_clock;
using Duration = Clock::duration;
using TimePoint = Clock::time_point;

class Stage final {
public:
  bool mAlive = false;
  explicit Stage(const std::string& name) = delete;
  explicit Stage(const std::string_view& name);
  ~Stage();
  Stage(const Stage&) = delete;
  Stage(Stage&&) = delete;
  Stage& operator=(const Stage&) = delete;
  Stage& operator=(Stage&&) = delete;
};

class StageStorage final {
  TimePoint mCreationTime;
  std::unordered_map<std::string_view, std::unique_ptr<StageStorage>> mNestedStages;
  Duration mTotalDuration;
  uint32_t mCount;

public:
  StageStorage();
  StageStorage* getSubStorage(const std::string_view& name);
  void record(Duration duration);
  TimePoint creationTime() const noexcept { return mCreationTime; }
  Duration duration() const noexcept { return mTotalDuration; }
  uint32_t count() const noexcept { return mCount; }
  void printNested(uint32_t depth, double total) const;
};

class Profiler final {
  StageStorage mRootStage;
  std::deque<std::pair<TimePoint, StageStorage*>> mStageStack;

public:
  Profiler();
  // performance
  void pushStage(const std::string_view& name);
  void popStage();
  void printStatistics();
  // counter

  static Profiler& get();
};

}  // namespace utils
