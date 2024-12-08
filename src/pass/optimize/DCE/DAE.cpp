#include "pass/optimize/DAE.hpp"
using namespace pass;
static std::unordered_map<ir::Function*, std::unordered_set<ir::CallInst*>> FucntionCallerInsts;

struct PairHash {
  template <class T1, class T2>
  std::size_t operator()(const std::pair<T1, T2>& pair) const {
    return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
  }
};

void DAE::run(ir::Module* md, TopAnalysisInfoManager* tp) {
  for (auto func : md->funcs()) {
    for (auto bb : func->blocks()) {
      for (auto inst : bb->insts()) {
        if (auto callInst = inst->dynCast<ir::CallInst>()) {
          FucntionCallerInsts[callInst->callee()].insert(callInst);
        }
      }
    }
  }
  for (auto func : md->funcs()) {
    std::vector<std::pair<ir::Argument*, size_t>> delArgs;
    // std::unordered_set<std::pair<ir::Argument*, size_t>, PairHash> delArgs;
    size_t idx = 0;
    for (auto arg : func->args()) {
      bool isDead = true;
      for (auto puse : arg->uses()) {
        auto user = puse->user();
        auto callInst = user->dynCast<ir::CallInst>();
        if (callInst == nullptr) {
          isDead = false;
          break;
        }
        if (callInst->callee() != func) {
          isDead = false;
          break;
        }
      }
      if (isDead) {
        delArgs.push_back(std::pair(arg, idx));
      }
      idx++;
    }
    if (delArgs.empty()) continue;
    std::cerr << "Function " << func->name() << " has " << delArgs.size() << " dead args."
              << std::endl;
    for (auto argIter = delArgs.rbegin(); argIter != delArgs.rend(); argIter++) {
      // for(auto argIter=delArgs.begin();argIter!=delArgs.end();argIter++){
      auto arg = argIter->first;
      auto idx = argIter->second;
      func->delArgumant(idx);
      for (auto callInst : FucntionCallerInsts[func]) {
        callInst->delete_operands(idx);
      }
    }
  }
}