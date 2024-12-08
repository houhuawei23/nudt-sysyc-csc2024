#include "pass/optimize/AG2L.hpp"
#include "pass/optimize/mem2reg.hpp"
#include "pass/optimize/simplifyCFG.hpp"

using namespace pass;

/*
激进的global2local（仅仅对标量进行替换）
对函数内部的global变量进行local变化，以试图消去冗余的loadstore，将全局变量以尽可能长的时间留在虚拟寄存器中
把全局变量分为三种：
1 只读的和常数的--直接替换
2 只写的--删掉
3 又读又写的
其中第三种再细分，又分为：
a 只是被main函数使用的
b 被一个其他的函数使用的
c 被多个函数使用的
其中第三种又再细分：
i) 在当前函数中被读的
ii) 在当前函数中被写的
iii) 在当前函数中又读又写的

在结束之后内部调用ADCE-mem2reg
*/

std::unordered_map<ir::Function*, bool> isFuncInsertBB;

void AggressiveG2L::run(ir::Module* md, TopAnalysisInfoManager* tp) {
  AggressiveG2LContext ctx;
  ctx.run(md, tp);
}

void AggressiveG2LContext::run(ir::Module* md, TopAnalysisInfoManager* tp) {
  // std::cerr<<"Here"<<std::endl;
  cgctx = tp->getCallGraph();
  isFuncInsertBB.clear();
  // 1. 分析读写情况;
  std::unordered_map<ir::GlobalVariable*, std::unordered_set<ir::Function*>> funcGvRead;
  std::unordered_map<ir::GlobalVariable*, std::unordered_set<ir::Function*>> funcGvWrite;
  std::unordered_set<ir::GlobalVariable*> readAndWriteGvs;
  std::unordered_set<ir::GlobalVariable*> readOnlyGvs;
  std::unordered_set<ir::GlobalVariable*> writeOnlyGvs;
  std::unordered_set<ir::GlobalVariable*> noUseGvs;
  for (auto gv : md->globalVars()) {
    if (gv->isArray()) continue;
    funcGvRead[gv] = std::unordered_set<ir::Function*>();
    funcGvWrite[gv] = std::unordered_set<ir::Function*>();
  }
  sectx = tp->getSideEffectInfo();
  for (auto func : md->funcs()) {
    isFuncInsertBB[func] = false;
    for (auto readGv : sectx->funcReadGlobals(func)) {
      if (readGv->isArray()) continue;
      funcGvRead[readGv].insert(func);
    }
    for (auto writeGv : sectx->funcWriteGlobals(func)) {
      if (writeGv->isArray()) continue;
      funcGvWrite[writeGv].insert(func);
    }
  }
  for (auto gv : md->globalVars()) {
    if (gv->isArray()) continue;
    if (funcGvRead[gv].size() and not funcGvWrite[gv].size())
      readOnlyGvs.insert(gv);
    else if (funcGvWrite[gv].size() and not funcGvRead[gv].size())
      writeOnlyGvs.insert(gv);
    else if (not funcGvWrite[gv].size() and not funcGvRead[gv].size())
      noUseGvs.insert(gv);
    else
      readAndWriteGvs.insert(gv);
  }
  for (auto nuGv : noUseGvs)
    md->delGlobalVariable(nuGv);
  for (auto ROGv : readOnlyGvs)
    replaceReadOnlyGv(ROGv);
  for (auto WOGv : writeOnlyGvs)
    deleteWriteOnlyGv(WOGv);
  // 处理cond 3
  std::unordered_set<ir::GlobalVariable*> multipleRWGvs;
  for (auto rwGv : readAndWriteGvs) {
    auto& readfnset = funcGvRead[rwGv];
    auto& writefnset = funcGvWrite[rwGv];
    // std::cerr<<rwGv->name()<<std::endl;
    // cond a and cond b
    if (rwGv->isArray()) assert(false);
    if (readfnset.size() == 1 and writefnset.size() == 1) {
      auto onlyUseFunc = *readfnset.begin();
      if (onlyUseFunc->name() == "main") {  // cond a
        // std::cerr<<rwGv->name()<<" local to main!"<<std::endl;
        replaceGvInMain(rwGv, onlyUseFunc);
      } else {  // cond b
        // std::cerr<<rwGv->name()<<" local to "<<onlyUseFunc->name()<<"!"<<std::endl;
        replaceGvInNormalFunc(rwGv, onlyUseFunc);
      }
    } else {  // cond c
      std::unordered_set<ir::Function*> readAndWrite;
      for (auto func : md->funcs()) {
        if (sectx->funcDirectReadGvs(func).count(rwGv) or
            sectx->funcDirectWriteGvs(func).count(rwGv))
          readAndWrite.insert(func);
      }
      int funcUseSize = readAndWrite.size();
      int gvUseSize = rwGv->uses().size();
      if (gvUseSize / funcUseSize < 4) continue;
      // std::cerr<<rwGv->name()<<" local to all funcs!"<<std::endl;
      for (auto func : md->funcs()) {
        if (func->isOnlyDeclare()) continue;
        if (sectx->funcReadGlobals(func).count(rwGv) == 0 and
            sectx->funcWriteGlobals(func).count(rwGv) == 0)
          continue;
        if (sectx->funcDirectReadGvs(func).count(rwGv) == 0 and
            sectx->funcDirectWriteGvs(func).count(rwGv) == 0)
          continue;
        replaceGvInOneFunc(rwGv, func);
      }
    }
    // call mem2reg
  }
}

void AggressiveG2LContext::replaceReadOnlyGv(ir::GlobalVariable* gv) {
  auto gvInitVal = gv->init(0);
  for (auto puseiter = gv->uses().begin(); puseiter != gv->uses().end();) {
    auto puse = *puseiter;
    puseiter++;
    auto puser = puse->user();
    if (auto ldinst = puser->dynCast<ir::LoadInst>()) {
      ldinst->replaceAllUseWith(gvInitVal);
    }
    if (auto stInst = puser->dynCast<ir::StoreInst>()) {
      assert(false and "Trying to replace a gv with value while it's not Read only!");
    }
  }
}

void AggressiveG2LContext::deleteWriteOnlyGv(ir::GlobalVariable* gv) {
  for (auto puseiter = gv->uses().begin(); puseiter != gv->uses().end();) {
    auto puse = *puseiter;
    puseiter++;
    auto puser = puse->user();
    if (auto stInst = puser->dynCast<ir::StoreInst>()) {
      stInst->block()->delete_inst(stInst);
    }
    if (auto ldinst = puser->dynCast<ir::LoadInst>()) {
      assert(false and "Trying to delete a non-WirteOnly gv!");
    }
  }
}

void AggressiveG2LContext::replaceGvInMain(ir::GlobalVariable* gv, ir::Function* func) {
  // 在entry和唯一后继之间插入一个bb
  ir::BasicBlock* newBB;
  ir::BasicBlock* funcEntry;
  if (isFuncInsertBB[func]) {
    funcEntry = func->entry();
    newBB = func->entry()->next_blocks().front();
  } else {
    isFuncInsertBB[func] = true;
    newBB = new ir::BasicBlock("", func);
    funcEntry = func->entry();
    auto funcEntryNext = funcEntry->next_blocks().front();
    ir::BasicBlock::delete_block_link(funcEntry, funcEntryNext);
    ir::BasicBlock::block_link(funcEntry, newBB);
    ir::BasicBlock::block_link(newBB, funcEntryNext);
    auto funcEntryTerminator = funcEntry->terminator()->dynCast<ir::BranchInst>();
    funcEntryTerminator->set_dest(newBB);
    auto newBrInNewBB = new ir::BranchInst(funcEntryNext, newBB, "");
    newBB->emplace_back_inst(newBrInNewBB);
    func->blocks().push_back(newBB);
    for (auto entryUseIter = funcEntry->uses().begin(); entryUseIter != funcEntry->uses().end();) {
      auto puse = *entryUseIter;
      auto useIdx = puse->index();
      entryUseIter++;
      auto puserInst = puse->user()->dynCast<ir::PhiInst>();
      if (puserInst != nullptr) {
        puserInst->setOperand(useIdx, newBB);
      }
    }
  }

  // 构造一个同类型的alloca在entry
  auto gvType = gv->type();
  auto newAlloca = new ir::AllocaInst(gv->baseType(), false, funcEntry, "");
  funcEntry->emplace_lastbutone_inst(newAlloca);
  // 在函数中所有对于这个gv的store都转化成对alloca的，load也是一样
  gv->replaceAllUseWith(newAlloca);
  // 在新的bb中加入对当前gv的初始值的store,如果没有就不加
  if (gv->isInit()) {
    auto newStore = new ir::StoreInst(gv->init(0), newAlloca, newBB);
    newBB->emplace_lastbutone_inst(newStore);
  } else {
    ir::Value* initVal;
    if (gv->baseType()->isInt32()) {
      initVal = ir::ConstantInteger::gen_i32(0);
    }
    if (gv->baseType()->isFloat32()) {
      initVal = ir::ConstantFloating::gen_f32(0.0);
    }

    auto newStore = new ir::StoreInst(initVal, newAlloca, newBB);
    newBB->emplace_first_inst(newStore);
  }

  // 最后将gv删除
  auto md = gv->parent();
  md->delGlobalVariable(gv);
}

void AggressiveG2LContext::replaceGvInNormalFunc(ir::GlobalVariable* gv, ir::Function* func) {
  // 在entry和唯一后继之间插入一个bb
  ir::BasicBlock* newBB;
  ir::BasicBlock* funcEntry;
  if (isFuncInsertBB[func]) {
    funcEntry = func->entry();
    newBB = func->entry()->next_blocks().front();
  } else {
    isFuncInsertBB[func] = true;
    newBB = new ir::BasicBlock("", func);
    funcEntry = func->entry();
    auto funcEntryNext = funcEntry->next_blocks().front();
    ir::BasicBlock::delete_block_link(funcEntry, funcEntryNext);
    ir::BasicBlock::block_link(funcEntry, newBB);
    ir::BasicBlock::block_link(newBB, funcEntryNext);
    auto funcEntryTerminator = funcEntry->terminator()->dynCast<ir::BranchInst>();
    funcEntryTerminator->set_dest(newBB);
    auto newBrInNewBB = new ir::BranchInst(funcEntryNext, newBB, "");
    newBB->emplace_back_inst(newBrInNewBB);
    func->blocks().push_back(newBB);
    for (auto entryUseIter = funcEntry->uses().begin(); entryUseIter != funcEntry->uses().end();) {
      auto puse = *entryUseIter;
      auto useIdx = puse->index();
      entryUseIter++;
      auto puserInst = puse->user()->dynCast<ir::PhiInst>();
      if (puserInst != nullptr) {
        puserInst->setOperand(useIdx, newBB);
      }
    }
  }
  // 构造一个同类型的alloca在entry
  auto gvType = gv->type();
  auto newAlloca = new ir::AllocaInst(gv->type(), false, funcEntry, "");
  funcEntry->emplace_lastbutone_inst(newAlloca);
  // 在函数中所有对于这个gv的store都转化成对alloca的，load也是一样
  gv->replaceAllUseWith(newAlloca);
  // 在新的bb中load gv的值，store到新的alloca中
  auto loadGvInNewBB = new ir::LoadInst(gv, gv->type(), newBB);
  newBB->emplace_first_inst(loadGvInNewBB);
  auto storeGvInNewBB = new ir::StoreInst(loadGvInNewBB, newAlloca, newBB);
  newBB->emplace_lastbutone_inst(storeGvInNewBB);
  // 如果有递归，在call指令的之前：将alloca的load值store给gv，在之后：将gvload的值store给alloca
  for (auto calleeInst : cgctx->calleeCallInsts(func)) {
    auto calleeFunc = calleeInst->callee();
    if (calleeFunc == func) {
      auto curBB = calleeInst->block();
      auto callInstPos = std::find(curBB->insts().begin(), curBB->insts().end(), calleeInst);
      auto newLoadAlloca = new ir::LoadInst(newAlloca, newAlloca->baseType(), curBB);
      auto newStoreToGv = new ir::StoreInst(newLoadAlloca, gv, curBB);
      auto newLoadGv = new ir::LoadInst(gv, gv->baseType(), curBB);
      auto newStoreToAlloca = new ir::StoreInst(newLoadGv, newAlloca, curBB);
      // TODO:这里将前面两句插在call前面，后面两句插在call后面
      curBB->emplace_inst(callInstPos, newLoadAlloca);
      curBB->emplace_inst(callInstPos, newStoreToGv);
      callInstPos++;
      curBB->emplace_inst(callInstPos, newLoadGv);
      curBB->emplace_inst(callInstPos, newStoreToAlloca);
    }
  }
  // 在函数ret之前插入load alloca store gv
  auto funcExit = func->exit();
  auto loadAllocaInExit = new ir::LoadInst(newAlloca, newAlloca->baseType(), funcExit);
  auto storeLoadToGv = new ir::StoreInst(loadAllocaInExit, gv, funcExit);
  funcExit->emplace_lastbutone_inst(loadAllocaInExit);
  funcExit->emplace_lastbutone_inst(storeLoadToGv);

}  // 配合mem2reg使用

void AggressiveG2LContext::replaceGvInOneFunc(ir::GlobalVariable* gv, ir::Function* func) {
  // std::cerr<<"gv:"<<gv->name()<<" func:"<<func->name()<<std::endl;
  // 在entry和唯一后继之间插入一个bb
  ir::BasicBlock* newBB;
  ir::BasicBlock* funcEntry;
  if (isFuncInsertBB[func]) {
    funcEntry = func->entry();
    newBB = func->entry()->next_blocks().front();
  } else {
    isFuncInsertBB[func] = true;
    newBB = new ir::BasicBlock("", func);
    funcEntry = func->entry();
    auto funcEntryNext = funcEntry->next_blocks().front();
    ir::BasicBlock::delete_block_link(funcEntry, funcEntryNext);
    ir::BasicBlock::block_link(funcEntry, newBB);
    ir::BasicBlock::block_link(newBB, funcEntryNext);
    auto funcEntryTerminator = funcEntry->terminator()->dynCast<ir::BranchInst>();
    funcEntryTerminator->set_dest(newBB);
    auto newBrInNewBB = new ir::BranchInst(funcEntryNext, newBB, "");
    newBB->emplace_back_inst(newBrInNewBB);
    func->blocks().push_back(newBB);
    for (auto entryUseIter = funcEntry->uses().begin(); entryUseIter != funcEntry->uses().end();) {
      auto puse = *entryUseIter;
      auto useIdx = puse->index();
      entryUseIter++;
      auto puserInst = puse->user()->dynCast<ir::PhiInst>();
      if (puserInst != nullptr) {
        puserInst->setOperand(useIdx, newBB);
      }
    }
  }
  // 构造一个同类型的alloca在entry
  auto gvType = gv->type();
  auto newAlloca = new ir::AllocaInst(gv->baseType(), false, funcEntry, "");
  funcEntry->emplace_lastbutone_inst(newAlloca);
  // 在函数中所有对于这个gv的store都转化成对alloca的，load也是一样
  //  gv->replaceAllUseWith(newAlloca);
  for (auto gvUseIter = gv->uses().begin(); gvUseIter != gv->uses().end();) {
    auto gvUse = *gvUseIter;
    gvUseIter++;
    auto user = gvUse->user();
    auto useIdx = gvUse->index();
    auto inst = user->dynCast<ir::Instruction>();
    if (inst->block()->function() == func) {
      inst->setOperand(useIdx, newAlloca);
    }
  }
  // 在新的bb中load gv的值，store到新的alloca中
  auto loadGvInNewBB = new ir::LoadInst(gv, gv->baseType(), newBB);
  newBB->emplace_first_inst(loadGvInNewBB);
  auto storeGvInNewBB = new ir::StoreInst(loadGvInNewBB, newAlloca, newBB);
  newBB->emplace_lastbutone_inst(storeGvInNewBB);
  // 计算当前函数对于当前的gv是否只读 只写
  auto isFuncReadGv = sectx->funcReadGlobals(func).count(gv) != 0;
  auto isFuncWriteGv = sectx->funcWriteGlobals(func).count(gv) != 0;
  if (isFuncReadGv and
      not isFuncWriteGv) {  // 根据副作用分析，这里说的只读和只写分别是在当前函数及其被调用者中
    // 只对gv读取
    return;
  } else if (isFuncWriteGv and not isFuncReadGv) {
    // 只对gv写入
    auto funcExit = func->exit();
    auto loadAllocaInExit = new ir::LoadInst(newAlloca, newAlloca->baseType(), funcExit);
    auto storeLoadToGv = new ir::StoreInst(loadAllocaInExit, gv, funcExit);
    funcExit->emplace_lastbutone_inst(loadAllocaInExit);
    funcExit->emplace_lastbutone_inst(storeLoadToGv);
    return;
  } else {
    // 均有
    for (auto calleeInst : cgctx->calleeCallInsts(func)) {
      auto calleeFunc = calleeInst->callee();
      auto calleeInstBB = calleeInst->block();
      auto isCalleeFuncRead = sectx->funcReadGlobals(calleeFunc).count(gv) != 0;
      auto isCalleeFuncWrite = sectx->funcWriteGlobals(calleeFunc).count(gv) != 0;
      if (not isCalleeFuncRead and not isCalleeFuncWrite) {     // 没有副作用
        continue;                                               // not processed
      } else if (not isCalleeFuncWrite and isCalleeFuncRead) {  // 只读
        // load alloca val
        // store load val
        auto curBB = calleeInst->block();
        auto callInstPos = std::find(curBB->insts().begin(), curBB->insts().end(), calleeInst);
        auto newLoadAlloca = new ir::LoadInst(newAlloca, newAlloca->baseType(), curBB);
        auto newStoreToGv = new ir::StoreInst(newLoadAlloca, gv, curBB);
        curBB->emplace_inst(callInstPos, newLoadAlloca);
        curBB->emplace_inst(callInstPos, newStoreToGv);
        callInstPos++;
      } else if (not isCalleeFuncRead and isCalleeFuncWrite) {  // 只写
        // load gv val
        // store load val
        auto curBB = calleeInst->block();
        auto callInstPos = std::find(curBB->insts().begin(), curBB->insts().end(), calleeInst);
        auto newLoadGv = new ir::LoadInst(gv, gv->baseType(), curBB);
        auto newStoreToAlloca = new ir::StoreInst(newLoadGv, newAlloca, curBB);
        // TODO:这里将前面两句插在call前面，后面两句插在call后面
        callInstPos++;
        curBB->emplace_inst(callInstPos, newLoadGv);
        curBB->emplace_inst(callInstPos, newStoreToAlloca);
      } else {  // 又读又写
        // load alloca val
        // store load val
        // load gv val
        // store load val
        auto curBB = calleeInst->block();
        auto callInstPos = std::find(curBB->insts().begin(), curBB->insts().end(), calleeInst);
        auto newLoadAlloca = new ir::LoadInst(newAlloca, newAlloca->baseType(), curBB);
        auto newStoreToGv = new ir::StoreInst(newLoadAlloca, gv, curBB);
        auto newLoadGv = new ir::LoadInst(gv, gv->baseType(), curBB);
        auto newStoreToAlloca = new ir::StoreInst(newLoadGv, newAlloca, curBB);
        // TODO:这里将前面两句插在call前面，后面两句插在call后面
        curBB->emplace_inst(callInstPos, newLoadAlloca);
        curBB->emplace_inst(callInstPos, newStoreToGv);
        callInstPos++;
        curBB->emplace_inst(callInstPos, newLoadGv);
        curBB->emplace_inst(callInstPos, newStoreToAlloca);
      }
    }
  }
  // 在函数ret之前插入load alloca store gv
  auto funcExit = func->exit();
  auto loadAllocaInExit = new ir::LoadInst(newAlloca, newAlloca->baseType(), funcExit);
  auto storeLoadToGv = new ir::StoreInst(loadAllocaInExit, gv, funcExit);
  funcExit->emplace_lastbutone_inst(loadAllocaInExit);
  funcExit->emplace_lastbutone_inst(storeLoadToGv);

}  // 配合mem2reg使用