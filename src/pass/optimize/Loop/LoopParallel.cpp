#define DEBUG

#include "pass/optimize/optimize.hpp"
#include "pass/optimize/Loop/LoopParallel.hpp"
#include "pass/analysis/ControlFlowGraph.hpp"
#include "pass/optimize/Loop/LoopBodyExtract.hpp"
#include "pass/optimize/Loop/ParallelBodyExtract.hpp"
#include "pass/optimize/Utils/BlockUtils.hpp"
#include "pass/analysis/MarkParallel.hpp"

#include "pass/optimize/Loop/LoopUtils.hpp"
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>

using namespace ir;
namespace pass {
bool LoopParallel::isConstant(Value* val) {
  if (val->isa<ConstantValue>() or val->isa<GlobalVariable>()) {
    return true;
  }

  // if(auto inst = val->dynCast<Instruction>()){

  // }
  return false;
}
/**
 * void parallelFor(int32_t beg, int32_t end, void (*)(int32_t beg, int32_t end) func);
 *
 * void @parallelFor(i32 %beg, i32 %end, void (i32, i32)* %parallel_body_ptr);
 */
static Function* loopupParallelFor(Module* module) {
  if (auto func = module->findFunction("parallelFor")) {
    return func;
  }
  const auto voidType = Type::void_type();
  const auto i32 = Type::TypeInt32();

  const auto parallelBodyPtrType = FunctionType::gen(voidType, {i32, i32});

  const auto parallelForType = FunctionType::gen(voidType, {i32, i32, parallelBodyPtrType});

  auto parallelFor = module->addFunction(parallelForType, "parallelFor");
  parallelFor->attribute().addAttr(FunctionAttribute::Builtin);

  return parallelFor;
}

void LoopParallel::run(Function* func, TopAnalysisInfoManager* tp) {
  runImpl(func, tp);
}

bool parallelLoop(Function* func, TopAnalysisInfoManager* tp, Loop* loop, IndVar* indVar) {
  ParallelBodyInfo parallelBodyInfo;
  if (not extractParallelBody(func, loop /* modified */, indVar, tp, parallelBodyInfo /* ret */))
    return false;
  // std::cerr << "parallel body extracted" << std::endl;
  // func->print(std::cerr);
  const auto parallelBody = parallelBodyInfo.parallelBody;
  auto parallelFor = loopupParallelFor(func->module());

  IRBuilder builder;
  const auto callBlock = parallelBodyInfo.callBlock;
  auto& insts = parallelBodyInfo.callBlock->insts();
  std::vector<Value*> args = {parallelBodyInfo.beg, parallelBodyInfo.end, parallelBody};

  const auto iter = std::find(insts.begin(), insts.end(), parallelBodyInfo.callInst);
  // assert(iter != insts.end());  // must find
  if (iter == insts.end()) {
    std::cerr << "cannot find call inst" << std::endl;
    return false;
  }

  builder.set_pos(callBlock, iter);
  builder.makeInst<CallInst>(parallelFor, args);
  callBlock->move_inst(parallelBodyInfo.callInst);  // remove call parallel_body

  const auto fixFunction = [&](Function* function) {
    CFGAnalysisHHW().run(function, tp);
    blockSortDFS(*function, tp);
    // function->rename();
    // function->print(std::cerr);
  };
  fixFunction(func);
  return true;
}

bool LoopParallel::runImpl(Function* func, TopAnalysisInfoManager* tp) {
  func->rename();
  // func->print(std::cerr);

  CFGAnalysisHHW().run(func, tp);  // refresh CFG
  MarkParallel().run(func, tp);

  auto lpctx = tp->getLoopInfoWithoutRefresh(func);        // fisrt loop analysis
  auto indVarctx = tp->getIndVarInfoWithoutRefresh(func);  // then indvar analysis
  auto parallelctx = tp->getParallelInfo(func);

  auto loops = lpctx->sortedLoops();

  bool modified = false;

  std::unordered_set<Loop*> extractedLoops;

  // lpctx->print(std::cerr);
  for (auto loop : loops) {  // for all loops
    const auto indVar = indVarctx->getIndvar(loop);
    if (not checkLoopParallel(loop, lpctx, indVarctx, parallelctx, extractedLoops)) continue;
#ifdef DEBUG
    std::cerr << "loop level: " << lpctx->looplevel(loop->header());
    loop->print(std::cerr);
    indVar->print(std::cerr);
#endif
    auto success = parallelLoop(func, tp, loop, indVar);
    modified |= success;
    if (success) {
      extractedLoops.insert(loop);
      // std::cerr << "parallel loop: " << std::endl;
      // loop->print(std::cerr);
      // indVar->print(std::cerr);
    }
  }

  return modified;
}

}  // namespace pass