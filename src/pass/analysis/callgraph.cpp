#include "pass/analysis/callgraph.hpp"

namespace pass {
void CallGraphBuild::run(ir::Module* module, TopAnalysisInfoManager* tp) {
  CallGraphBuildContext ctx;
  ctx.run(module, tp);
}
void CallGraphBuildContext::run(ir::Module* module, TopAnalysisInfoManager* tp) {
  cgctx = tp->getCallGraphWithoutRefresh();
  cgctx->clearAll();
  cgctx->initialize();

  for (auto func : module->funcs()) {  // initialize call info for functions
    if (func->isOnlyDeclare())
      // func->set_is_lib(true);
      cgctx->set_isLib(func, true);
    else if (func->name() == "_sysy_starttime" or func->name() == "_sysy_stoptime")
      cgctx->set_isLib(func, true);
    else
      // func->set_is_lib(false);
      cgctx->set_isLib(func, false);
    // func->set_is_called(false);
    // func->set_is_inline(true);
    // func->callees().clear();
    cgctx->set_isCalled(func, false);
    cgctx->set_isInline(func, true);
    vis.emplace(func, false);
  }

  for (auto func : module->funcs()) {  // travel all inst and collect call information
    for (auto bb : func->blocks()) {
      for (auto inst : bb->insts()) {
        auto instCall = dyn_cast<ir::CallInst>(inst);
        if (instCall) {
          auto calleePtr = instCall->callee();
          // if(calleePtr->get_is_lib())continue;// lib function don't need call info
          // if (cgctx->isLib(calleePtr)) continue;
          // func->callees().insert(calleePtr);
          // func->set_is_called(true);
          assert(std::find(instCall->block()->insts().begin(), instCall->block()->insts().end(),
                           instCall) != instCall->block()->insts().end());
          cgctx->callees(func).insert(calleePtr);
          cgctx->calleeCallInsts(func).insert(instCall);
          cgctx->callers(calleePtr).insert(func);
          cgctx->callerCallInsts(calleePtr).insert(instCall);
          cgctx->set_isCalled(func, true);
        }
      }
    }
  }
  // assert(funcStack.empty());
  // assert(funcSet.empty());
  // dfsFuncCallGraph(ctx->mainFunction());
}
void CallGraphBuildContext::dfsFuncCallGraph(ir::Function* func) {
  funcStack.push_back(func);
  funcSet.insert(func);
  for (auto calleeFunc : cgctx->callees(func)) {
    if (funcSet.count(calleeFunc)) {  // find a back edge
      // calleeFunc->set_is_inline(false);
      cgctx->set_isInline(calleeFunc, false);
      for (auto funcIter = funcStack.rbegin(); *funcIter != calleeFunc; funcIter++) {
        // (*funcIter)->set_is_inline(false);
        cgctx->set_isInline(*funcIter, false);
      }
    } else {  // normal edge, and we continue recursive
      //   if (not vis[calleeFunc]) dfsFuncCallGraph(calleeFunc);
      if (not vis.at(calleeFunc)) dfsFuncCallGraph(calleeFunc);
    }
  }
  funcStack.pop_back();
  funcSet.erase(func);
}

void CallGraphCheck::run(ir::Module* module, TopAnalysisInfoManager* tp) {
  auto cgctx = tp->getCallGraphWithoutRefresh();
  using namespace std;
  for (auto func : module->funcs()) {
    if (cgctx->isLib(func)) continue;
    cout << "Function " << func->name() << "(" << cgctx->isInline(func)
         << " for inline) called :" << endl;
    for (auto funccallee : cgctx->callees(func)) {
      cout << funccallee->name() << "\t";
    }
    cout << endl;
  }
}
}  // namespace pass