#pragma once
#include <vector>
#include "ir/ir.hpp"
#include "pass/AnalysisInfo.hpp"

#include <chrono>
namespace pass {

class BasePass {
public:
  virtual void run(void* pass_unit, TopAnalysisInfoManager* tp) = 0;
  virtual std::string name() const = 0;

  // Virtual destructor for proper cleanup
  virtual ~BasePass() = default;
};

//! Pass Template
template <typename PassUnit>
class Pass : public BasePass {
public:
  // Override run to match BasePass, using static_cast to ensure type safety
  void run(void* pass_unit, TopAnalysisInfoManager* tp) override {
    run(static_cast<PassUnit*>(pass_unit), tp);
  }
  // pure virtual function, define the api
  virtual void run(PassUnit* pass_unit, TopAnalysisInfoManager* tp) = 0;
  virtual std::string name() const = 0;
  // Virtual destructor to allow proper cleanup of derived classes
  virtual ~Pass() = default;
};

// Instantiate Pass Class for Module, Function and BB
using ModulePass = Pass<ir::Module>;
using FunctionPass = Pass<ir::Function>;
using BasicBlockPass = Pass<ir::BasicBlock>;

class PassManager {
  ir::Module* irModule;
  pass::TopAnalysisInfoManager* tAIM;

public:
  PassManager(ir::Module* pm, TopAnalysisInfoManager* tp) {
    irModule = pm;
    tAIM = tp;
  }
  void run(ModulePass* mp);
  void run(FunctionPass* fp);
  void run(BasicBlockPass* bp);
  void runPasses(std::vector<std::string> passes);
};

class TopAnalysisInfoManager {
private:
  ir::Module* mModule;
  // ir::Module info
  CallGraph* mCallGraph;
  SideEffectInfo* mSideEffectInfo;
  // ir::Function info
  std::unordered_map<ir::Function*, DomTree*> mDomTree;
  std::unordered_map<ir::Function*, PDomTree*> mPDomTree;
  std::unordered_map<ir::Function*, LoopInfo*> mLoopInfo;
  std::unordered_map<ir::Function*, IndVarInfo*> mIndVarInfo;
  std::unordered_map<ir::Function*, DependenceInfo*> mDepInfo;
  std::unordered_map<ir::Function*, ParallelInfo*> mParallelInfo;
  // bb info
  // add new func
  void addNewFunc(ir::Function* func) {
    auto pnewDomTree = new DomTree(func, this);
    mDomTree[func] = pnewDomTree;
    auto pnewPDomTree = new PDomTree(func, this);
    mPDomTree[func] = pnewPDomTree;
    auto pnewLoopInfo = new LoopInfo(func, this);
    mLoopInfo[func] = pnewLoopInfo;
    auto pnewIndVarInfo = new IndVarInfo(func, this);
    mIndVarInfo[func] = pnewIndVarInfo;
    auto pnewDepInfo = new DependenceInfo(func, this);
    mDepInfo[func] = pnewDepInfo;
    auto pnewParallelInfo = new ParallelInfo(func, this);
    mParallelInfo[func] = pnewParallelInfo;
  }

public:
  TopAnalysisInfoManager(ir::Module* pm = nullptr) : mModule(pm), mCallGraph(nullptr) {}
  DomTree* getDomTree(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto domctx = mDomTree[func];
    if (domctx == nullptr) {
      addNewFunc(func);
    }
    domctx = mDomTree[func];
    domctx->refresh();
    return domctx;
  }
  PDomTree* getPDomTree(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto domctx = mPDomTree[func];
    if (domctx == nullptr) {
      addNewFunc(func);
    }
    domctx = mPDomTree[func];
    domctx->refresh();
    return domctx;
  }
  LoopInfo* getLoopInfo(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto lpctx = mLoopInfo[func];
    if (lpctx == nullptr) {
      addNewFunc(func);
    }
    lpctx = mLoopInfo[func];
    lpctx->refresh();
    return lpctx;
  }
  IndVarInfo* getIndVarInfo(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto idvctx = mIndVarInfo[func];
    if (idvctx == nullptr) {
      addNewFunc(func);
    }
    idvctx = mIndVarInfo[func];
    idvctx->setOff();
    idvctx->refresh();
    return idvctx;
  }
  DependenceInfo* getDepInfo(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto dpctx = mDepInfo[func];
    if (dpctx == nullptr) {
      addNewFunc(func);
    }
    dpctx = mDepInfo[func];
    dpctx->setOff();
    dpctx->refresh();
    return dpctx;
  }

  CallGraph* getCallGraph() {
    mCallGraph->refresh();
    return mCallGraph;
  }
  SideEffectInfo* getSideEffectInfo() {
    mSideEffectInfo->setOff();
    mSideEffectInfo->refresh();
    return mSideEffectInfo;
  }

  DomTree* getDomTreeWithoutRefresh(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto domctx = mDomTree[func];
    if (domctx == nullptr) {
      addNewFunc(func);
      domctx->refresh();
    }
    return domctx;
  }
  PDomTree* getPDomTreeWithoutRefresh(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto domctx = mPDomTree[func];
    if (domctx == nullptr) {
      addNewFunc(func);
      domctx->refresh();
    }
    return domctx;
  }
  LoopInfo* getLoopInfoWithoutRefresh(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto lpctx = mLoopInfo[func];
    if (lpctx == nullptr) {
      addNewFunc(func);
      lpctx->refresh();
    }
    return lpctx;
  }
  IndVarInfo* getIndVarInfoWithoutRefresh(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto idvctx = mIndVarInfo[func];
    if (idvctx == nullptr) {
      addNewFunc(func);
      idvctx->refresh();
    }
    return idvctx;
  }
  DependenceInfo* getDepInfoWithoutRefresh(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto dpctx = mDepInfo[func];
    if (dpctx == nullptr) {
      addNewFunc(func);
      dpctx->refresh();
    }
    return dpctx;
  }

  CallGraph* getCallGraphWithoutRefresh() { return mCallGraph; }
  SideEffectInfo* getSideEffectInfoWithoutRefresh() { return mSideEffectInfo; }
  ParallelInfo* getParallelInfo(ir::Function* func) {
    if (func->isOnlyDeclare()) return nullptr;
    auto dpctx = mParallelInfo[func];
    if (dpctx == nullptr) {
      addNewFunc(func);
    }
    dpctx = mParallelInfo[func];
    return dpctx;
  }

  void initialize();
  void CFGChange(ir::Function* func) {
    if (func->isOnlyDeclare()) return;
    if (mDomTree.find(func) == mDomTree.cend()) {
      std::cerr << "DomTree not found for function " << func->name() << std::endl;
      return;
    }
    mDomTree[func]->setOff();
    mPDomTree[func]->setOff();
    mLoopInfo[func]->setOff();
    mIndVarInfo[func]->setOff();
  }
  void CallChange() { mCallGraph->setOff(); }
  void IndVarChange(ir::Function* func) {
    if (func->isOnlyDeclare()) return;
    mIndVarInfo[func]->setOff();
  }
};

}  // namespace pass