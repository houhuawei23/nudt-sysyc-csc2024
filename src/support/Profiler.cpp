#include "support/config.hpp"
#include "support/Profiler.hpp"
#include <vector>
#include <algorithm>
#include <iostream>
using namespace std::string_view_literals;

namespace utils {
Stage::Stage(const std::string_view& name) {
  const auto& config = sysy::Config::getInstance();
  if (config.logLevel >= sysy::LogLevel::DEBUG) {
    mAlive = true;
    Profiler::get().pushStage(name);
  }
}
Stage::~Stage() {
  const auto& config = sysy::Config::getInstance();
  if (config.logLevel >= sysy::LogLevel::DEBUG and mAlive) {
    Profiler::get().popStage();
    mAlive = false;
  }
}

StageStorage::StageStorage() : mCreationTime{Clock::now()}, mTotalDuration{}, mCount{0} {}
StageStorage* StageStorage::getSubStorage(const std::string_view& name) {
  const auto iter = mNestedStages.find(name);
  if (iter != mNestedStages.cend()) return iter->second.get();
  return mNestedStages.emplace(name, std::make_unique<StageStorage>()).first->second.get();
}
void StageStorage::record(Duration duration) {
  ++mCount;
  mTotalDuration += duration;
}
void StageStorage::printNested(uint32_t depth, double total) const {
  const auto self = static_cast<double>(mTotalDuration.count());
  std::vector<std::pair<const std::string_view*, const StageStorage*>> storages;
  for (auto& [k, v] : mNestedStages)
    storages.emplace_back(&k, v.get());
  std::sort(storages.begin(), storages.end(),
            [](auto& lhs, auto& rhs) { return lhs.second->duration() > rhs.second->duration(); });
  double acc = 0.0;
  constexpr double accThreshold = 0.95;    // top 95%
  constexpr double printThreshold = 0.05;  // at least 5%
  for (auto [name, stage] : storages) {
    const auto count = stage->count();
    const auto duraiton = static_cast<double>(stage->duration().count());
    constexpr auto ratio =
      static_cast<double>(Clock::period::num) / static_cast<double>(Clock::period::den);
    const auto selfRatio = duraiton / self;
    if (selfRatio < printThreshold) break;
    for (uint32_t idx = 0; idx < depth; ++idx)
      std::cerr << "    "sv;
    std::cerr << *name << ' ' << (duraiton * ratio * 1000.0) << " ms "sv << count << ' '
              << (selfRatio * 100.0) << "% "sv << (duraiton / total * 100.0) << "% "sv << std::endl;
    stage->printNested(depth + 1, total);
    acc += selfRatio;
    if (acc > accThreshold) break;
  }
}

Profiler::Profiler() {
  pushStage({});
}
// performance
void Profiler::pushStage(const std::string_view& name) {
  const auto current = Clock::now();
  if (!mStageStack.empty())
    mStageStack.emplace_back(current, mStageStack.back().second->getSubStorage(name));
  else
    mStageStack.emplace_back(current, &mRootStage);
}
void Profiler::popStage() {
  const auto end = Clock::now();
  auto [start, stage] = mStageStack.back();
  stage->record(end - start);
  mStageStack.pop_back();
}
void Profiler::printStatistics() {
  popStage();

  const auto& config = sysy::Config::getInstance();
  if (config.logLevel >= sysy::LogLevel::DEBUG) {
    if (!mStageStack.empty()) {
      std::cerr << "Unclosed stages: "sv << mStageStack.size() << std::endl;
    }
    std::cerr.precision(2);
    std::cerr << std::fixed;
    std::cerr << "===================== PERFORMANCE PROFILING RESULT ====================="sv
              << std::endl;
    constexpr auto ratio =
      static_cast<double>(Clock::period::num) / static_cast<double>(Clock::period::den);
    std::cerr << "Total used: "sv
              << (static_cast<double>(mRootStage.duration().count()) * ratio * 1000.0) << " ms"sv
              << std::endl;
    mRootStage.printNested(0, static_cast<double>(mRootStage.duration().count()));
    std::cerr << "========================================================================"sv
              << std::endl;
  }
}

Profiler& Profiler::get() {
  static Profiler profiler;
  return profiler;
}

}  // namespace utils