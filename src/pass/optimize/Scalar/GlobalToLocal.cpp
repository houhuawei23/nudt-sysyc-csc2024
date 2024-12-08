#include "pass/optimize/GlobalToLocal.hpp"
#include "pass/optimize/mem2reg.hpp"

using namespace pass;
// 对该全局变量直接使用的函数
static std::unordered_map<ir::GlobalVariable*, std::unordered_set<ir::Function*>>
  globalDirectUsedFunc;
// 该函数直接使用的全局变量
static std::unordered_map<ir::Function*, std::unordered_set<ir::GlobalVariable*>>
  funcDirectUseGlobal;
// 该函数间接使用的全局变量
static std::unordered_map<ir::Function*, std::unordered_set<ir::GlobalVariable*>>
  funcIndirectUseGlobal;
static std::unordered_map<ir::GlobalVariable*, bool> globalHasStore;
static std::unordered_set<ir::Function*> funcToMem2Reg;

void Global2Local::run(ir::Module* md, TopAnalysisInfoManager* tp) {
  Global2LocalContext ctx;
  ctx.run(md, tp);
}

void Global2LocalContext::run(ir::Module* md, TopAnalysisInfoManager* tp) {
  bool isChange = false;
  cgctx = tp->getCallGraph();
  globalCallAnalysis(md);
  funcToMem2Reg.clear();
  for (auto gvIter = md->globalVars().begin(); gvIter != md->globalVars().end();) {
    auto gv = *gvIter;
    // std::cerr<<gv->name()<<std::endl;
    bool tmpBool = processGlobalVariables(gv, md, tp);
    isChange = isChange or tmpBool;
    if (tmpBool) continue;
    gvIter++;
    // std::cerr<<"del."<<std::endl;
  }
  // std::cerr<<"Here"<<std::endl;
  if (funcToMem2Reg.size()) {
    Mem2Reg m2r;
    for (auto func : funcToMem2Reg) {
      tp->CFGChange(func);
      m2r.run(func, tp);
    }
    for (auto func : funcToMem2Reg) {
      auto newEntry = func->entry();
      auto oldEntry = newEntry->next_blocks().front();
      for (auto instIter = newEntry->insts().begin(); instIter != newEntry->insts().end();) {
        auto inst = *instIter;
        instIter++;
        if (inst->dynCast<ir::AllocaInst>()) {
          newEntry->move_inst(inst);
          oldEntry->emplace_first_inst(inst);
        }
      }
      func->forceDelBlock(newEntry);
      func->setEntry(oldEntry);
    }
  }
}

void Global2LocalContext::globalCallAnalysis(ir::Module* md) {
  /*
  这里进行分析, 判断每一个函数是否调用了对应的全局变量
  如果func A B C存在这样的调用链：A call B call C,
  而C调用了global var a,那么A和B也要视为调用了这个global var
  需要得到的信息：
  1. 每一个全局变量分别被使用了几次
  2. 每一个函数分别使用了（或者间接使用）哪些全局变量
  */
  // 清理对应信息
  globalDirectUsedFunc.clear();
  funcDirectUseGlobal.clear();
  funcIndirectUseGlobal.clear();
  globalHasStore.clear();
  for (auto func : md->funcs()) {
    funcDirectUseGlobal[func] = std::unordered_set<ir::GlobalVariable*>();
    funcIndirectUseGlobal[func] = std::unordered_set<ir::GlobalVariable*>();
  }
  for (auto gv : md->globalVars()) {
    globalHasStore[gv] = false;
    globalDirectUsedFunc[gv] = std::unordered_set<ir::Function*>();
    for (auto puse : gv->uses()) {
      auto gvUser = puse->user();
      auto gvUserInst = dyn_cast<ir::Instruction>(gvUser);
      if (gvUserInst) {
        auto directUseFunc = gvUserInst->block()->function();
        globalDirectUsedFunc[gv].insert(directUseFunc);
        funcDirectUseGlobal[directUseFunc].insert(gv);
      }
      auto gvUserStoreInst = dyn_cast<ir::StoreInst>(gvUser);
      if (gvUserStoreInst != nullptr) globalHasStore[gv] = true;
    }
  }
}

// 对于间接调用结果，需要基于直接调用进行传播
void Global2LocalContext::addIndirectGlobalUseFunc(ir::GlobalVariable* gv, ir::Function* func) {
  funcIndirectUseGlobal[func].insert(gv);
  for (auto callerfunc : cgctx->callers(func)) {
    if (funcIndirectUseGlobal[func].count(gv) == 0) addIndirectGlobalUseFunc(gv, callerfunc);
  }
}

/*
对于所有的全局变量，分为以下三种情况：
1. 没有被使用过的global
2. 只在一个函数中被使用的global
3. 在多个函数中被使用的global
*/
bool Global2LocalContext::processGlobalVariables(ir::GlobalVariable* gv,
                                                 ir::Module* md,
                                                 TopAnalysisInfoManager* tp) {
  // std::cerr<<gv->name()<<"!!!"<<std::endl;
  auto gvUseFuncSize = globalDirectUsedFunc[gv].size();
  if (gv->isArray()) return false;
  // std::cerr<<gv->name()<<"!!!"<<std::endl;
  if (not globalHasStore[gv]) {
    // std::cerr<<gv->name()<<"no!!!"<<std::endl;
    // 如果一个gv没有store,那么所有的值都可以被初始值直接替换！
    for (auto puseIter = gv->uses().begin(); puseIter != gv->uses().end();) {
      auto puse = *puseIter;
      puseIter++;
      auto userLdInst = dyn_cast<ir::LoadInst>(puse->user());
      assert(userLdInst != nullptr);  // 这里假设所有的对全局的使用都是load
      userLdInst->replaceAllUseWith(gv->init(0));
      userLdInst->block()->delete_inst(userLdInst);
    }
    md->delGlobalVariable(gv);
    return true;
  }
  // 如果对应的gv没有被使用过一次，那么就直接删除了
  if (gvUseFuncSize == 0) {
    // std::cerr<<gv->name()<<"000!!!"<<std::endl;
    md->delGlobalVariable(gv);
    return true;
  }
  if (gvUseFuncSize == 1) {
    // std::cerr<<gv->name()<<"111!!!"<<std::endl;
    auto func = *globalDirectUsedFunc[gv].begin();
    if (cgctx->callees(func).count(func)) return false;  // is recursive
    if (func->name() != "main") return false;  // 目前只支持对main使用的全局变量的tolocal
    if (funcToMem2Reg.count(func) ==
        0) {  // 为有需要的mem2reg的函数提供alloca条件,每一个函数只需要运行一遍
      auto newEntry = new ir::BasicBlock("newbb", func);
      auto oldEntry = func->entry();
      for (auto instIter = oldEntry->insts().begin(); instIter != oldEntry->insts().end();) {
        auto inst = *instIter;
        instIter++;
        if (inst->dynCast<ir::AllocaInst>()) {
          oldEntry->move_inst(inst);
          newEntry->emplace_first_inst(inst);
        }
      }
      func->setEntry(newEntry);
      func->blocks().push_front(newEntry);
      ir::BasicBlock::block_link(newEntry, oldEntry);
      auto newBrInst = new ir::BranchInst(oldEntry, newEntry);
      newEntry->emplace_back_inst(newBrInst);
      funcToMem2Reg.insert(func);
    }
    // std::cerr<<gv->name()<<"!!!"<<std::endl;
    auto gvType = gv->baseType();
    auto funcEntry = func->entry();
    auto newAlloca = new ir::AllocaInst(gvType, funcEntry);
    funcEntry->emplace_lastbutone_inst(newAlloca);
    gv->replaceAllUseWith(newAlloca);
    if (gv->isInit()) {  // 如果有对于gv的初始值就在oldEntry中进行添加即可
      auto oldEntry = func->entry()
                        ->next_blocks()
                        .front();  // 这里已经经过之前的针对mem2reg的条件的转换,所以这里直接取出
      auto newStoreInst = new ir::StoreInst(gv->init(0), newAlloca, oldEntry);
      oldEntry->emplace_first_inst(newStoreInst);
    } else {
      auto oldEntry = func->entry()
                        ->next_blocks()
                        .front();  // 这里已经经过之前的针对mem2reg的条件的转换,所以这里直接取出
      auto newStoreInst = new ir::StoreInst(ir::ConstantInteger::gen_i32(0), newAlloca, oldEntry);
      oldEntry->emplace_first_inst(newStoreInst);
    }
    md->delGlobalVariable(gv);
    return true;
  }
  // std::cerr<<gvUseFuncSize<<std::endl;
  for (auto func : globalDirectUsedFunc[gv]) {
    // std::cerr<<func->name()<<std::endl;
  }
  return false;
}