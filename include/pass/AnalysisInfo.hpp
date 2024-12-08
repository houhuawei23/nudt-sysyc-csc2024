#pragma once
// add analysis passes
#include "ir/ir.hpp"
#include <unordered_map>
#include <vector>
#include <queue>

using namespace ir;
namespace pass {
template <typename PassUnit>
class AnalysisInfo;

class DomTree;
class PDomTree;
class LoopInfo;
class CallGraph;
class IndVarInfo;
class TopAnalysisInfoManager;
class DependenceAnalysis;
class LoopDependenceInfo;

template <typename PassUnit>
class AnalysisInfo {
protected:
  PassUnit* passUnit;
  TopAnalysisInfoManager* topManager;
  bool isValid;

public:
  AnalysisInfo(PassUnit* mp, TopAnalysisInfoManager* mtp, bool v = false)
    : isValid(v), passUnit(mp), topManager(mtp) {}
  void setOn() { isValid = true; }
  void setOff() { isValid = false; }
  virtual void refresh() = 0;
};
using ModuleACtx = AnalysisInfo<Module>;
using FunctionACtx = AnalysisInfo<Function>;

// add new analysis info of ir here!
// dom Tree
/*
Dominate Tree
idom: immediate dominator
sdom: strict dominator
*/
class DomTree : public FunctionACtx {
protected:
  std::unordered_map<BasicBlock*, BasicBlock*> miDom;
  std::unordered_map<BasicBlock*, BasicBlock*> msDom;
  std::unordered_map<BasicBlock*, int> mDomLevel;
  std::unordered_map<BasicBlock*, std::vector<BasicBlock*>> mDomSon;
  std::unordered_map<BasicBlock*, std::vector<BasicBlock*>> mDomFrontier;
  std::vector<BasicBlock*> mBFSDomTreeVector;
  std::vector<BasicBlock*> mDFSDomTreeVector;

public:
  DomTree(Function* func, TopAnalysisInfoManager* tp) : FunctionACtx(func, tp) {}

  BasicBlock* idom(BasicBlock* bb) { return miDom.at(bb); }
  void set_idom(BasicBlock* bb, BasicBlock* idbb) { miDom[bb] = idbb; }

  BasicBlock* sdom(BasicBlock* bb) { return msDom.at(bb); }
  void set_sdom(BasicBlock* bb, BasicBlock* sdbb) { msDom[bb] = sdbb; }

  int domlevel(BasicBlock* bb) { return mDomLevel.at(bb); }
  void set_domlevel(BasicBlock* bb, int lv) { mDomLevel[bb] = lv; }

  auto& domson(BasicBlock* bb) { return mDomSon.at(bb); }

  auto& domfrontier(BasicBlock* bb) { return mDomFrontier.at(bb); }

  auto& BFSDomTreeVector() { return mBFSDomTreeVector; }

  auto& DFSDomTreeVector() { return mDFSDomTreeVector; }

  void clearAll() {
    miDom.clear();
    msDom.clear();
    mDomSon.clear();
    mDomFrontier.clear();
    mDomLevel.clear();
    mBFSDomTreeVector.clear();
    mDFSDomTreeVector.clear();
  }
  void initialize() {
    clearAll();
    for (auto bb : passUnit->blocks()) {
      mDomSon[bb] = std::vector<BasicBlock*>();
      mDomFrontier[bb] = std::vector<BasicBlock*>();
    }
  }
  void refresh() override;
  bool dominate(BasicBlock* bb1, BasicBlock* bb2);
  void BFSDomTreeInfoRefresh();
  void DFSDomTreeInfoRefresh();
};

class PDomTree : public FunctionACtx {  // also used as pdom
protected:
  std::unordered_map<BasicBlock*, BasicBlock*> mipDom;
  std::unordered_map<BasicBlock*, BasicBlock*> mspDom;
  std::unordered_map<BasicBlock*, int> mpDomLevel;
  std::unordered_map<BasicBlock*, std::vector<BasicBlock*>> mpDomSon;
  std::unordered_map<BasicBlock*, std::vector<BasicBlock*>> mpDomFrontier;

public:
  PDomTree(Function* func, TopAnalysisInfoManager* tp) : FunctionACtx(func, tp) {}
  BasicBlock* ipdom(BasicBlock* bb) {
    assert(bb && "bb is null");
    return mipDom.at(bb);
  }
  void set_ipdom(BasicBlock* bb, BasicBlock* idbb) { mipDom[bb] = idbb; }
  BasicBlock* spdom(BasicBlock* bb) {
    assert(bb && "bb is null");
    return mspDom[bb];
  }
  void set_spdom(BasicBlock* bb, BasicBlock* sdbb) { mspDom[bb] = sdbb; }
  int pdomlevel(BasicBlock* bb) {
    assert(bb && "bb is null");
    return mpDomLevel[bb];
  }
  void set_pdomlevel(BasicBlock* bb, int lv) { mpDomLevel[bb] = lv; }
  auto& pdomson(BasicBlock* bb) { return mpDomSon[bb]; }
  auto& pdomfrontier(BasicBlock* bb) { return mpDomFrontier[bb]; }
  void clearAll() {
    mipDom.clear();
    mspDom.clear();
    mpDomSon.clear();
    mpDomFrontier.clear();
    mpDomLevel.clear();
  }
  void initialize() {
    clearAll();
    for (auto bb : passUnit->blocks()) {
      mpDomSon[bb] = std::vector<BasicBlock*>();
      mpDomFrontier[bb] = std::vector<BasicBlock*>();
    }
  }

  bool pdominate(BasicBlock* bb1, BasicBlock* bb2);

  void refresh() override;
};

class LoopInfo : public FunctionACtx {
protected:
  std::vector<Loop*> mLoops;
  std::unordered_map<BasicBlock*, Loop*> mHeader2Loop;
  std::unordered_map<BasicBlock*, size_t> mLoopLevel;

public:
  LoopInfo(Function* fp, TopAnalysisInfoManager* tp) : FunctionACtx(fp, tp) {}
  auto& loops() { return mLoops; }
  Loop* head2loop(BasicBlock* bb) {
    if (mHeader2Loop.count(bb) == 0) return nullptr;
    return mHeader2Loop[bb];
  }
  void set_head2loop(BasicBlock* bb, Loop* lp) { mHeader2Loop[bb] = lp; }
  int looplevel(BasicBlock* bb) { return mLoopLevel[bb]; }
  void set_looplevel(BasicBlock* bb, int lv) { mLoopLevel[bb] = lv; }
  void clearAll() {
    mLoops.clear();
    mHeader2Loop.clear();
    mLoopLevel.clear();
  }
  bool isHeader(BasicBlock* bb) { return mHeader2Loop.count(bb); }
  Loop* getinnermostLoop(BasicBlock* bb);
  void refresh() override;
  void print(std::ostream& os) const;
  std::vector<ir::Loop*> sortedLoops(bool reverse = false);  // looplevel small to big
};

class CallGraph : public ModuleACtx {
protected:
  std::unordered_map<Function*, std::set<Function*>> mCallees;
  std::unordered_map<Function*, std::set<Function*>> mCallers;
  std::unordered_map<Function*, bool> mIsCalled;
  std::unordered_map<Function*, bool> mIsInline;
  std::unordered_map<Function*, bool> mIsLib;
  // func's caller insts are func's callers'
  std::unordered_map<Function*, std::set<CallInst*>> mCallerCallInsts;
  // func's callee insts are func's
  std::unordered_map<Function*, std::set<CallInst*>> mCalleeCallInsts;

public:
  CallGraph(Module* md, TopAnalysisInfoManager* tp) : ModuleACtx(md, tp) {}
  auto& callees(Function* func) { return mCallees[func]; }
  auto& callers(Function* func) { return mCallers[func]; }
  auto& callerCallInsts(Function* func) { return mCallerCallInsts[func]; }
  auto& calleeCallInsts(Function* func) { return mCalleeCallInsts[func]; }
  bool isCalled(Function* func) { return mIsCalled[func]; }
  bool isInline(Function* func) { return mIsInline[func]; }
  bool isLib(Function* func) { return mIsLib[func]; }
  void set_isCalled(Function* func, bool b) { mIsCalled[func] = b; }
  void set_isInline(Function* func, bool b) { mIsInline[func] = b; }
  void set_isLib(Function* func, bool b) { mIsLib[func] = b; }
  void clearAll() {
    mCallees.clear();
    mCallers.clear();
    mIsCalled.clear();
    mIsInline.clear();
    mIsLib.clear();
    mCallerCallInsts.clear();
    mCalleeCallInsts.clear();
  }
  void initialize() {
    for (auto func : passUnit->funcs()) {
      mCallees[func] = std::set<Function*>();
      mCallers[func] = std::set<Function*>();
    }
  }
  bool isNoCallee(Function* func) {
    if (mCallees[func].size() == 0) return true;
    for (auto f : mCallees[func]) {
      if (not isLib(f)) return false;
    }
    return true;
  }
  void refresh() override;
};

class IndVarInfo : public FunctionACtx {
private:
  std::unordered_map<Loop*, IndVar*> _loopToIndvar;

public:
  IndVarInfo(Function* fp, TopAnalysisInfoManager* tp) : FunctionACtx(fp, tp) {}
  IndVar* getIndvar(Loop* loop) {
    if (_loopToIndvar.count(loop) == 0) return nullptr;
    return _loopToIndvar.at(loop);
  }
  void clearAll() { _loopToIndvar.clear(); }
  void refresh() override;
  void addIndVar(Loop* lp, IndVar* idv) { _loopToIndvar[lp] = idv; }
};

class SideEffectInfo : public ModuleACtx {
private:
  // 当前函数读取的全局变量
  std::unordered_map<Function*, std::set<GlobalVariable*>> mFuncReadGlobals;
  // 当前函数写入的全局变量
  std::unordered_map<Function*, std::set<GlobalVariable*>> mFuncWriteGlobals;
  // 对于当前argument函数是否读取（仅限pointer）
  std::unordered_map<Argument*, bool> mIsArgumentRead;
  // 对于当前argument哈数是否写入（仅限pointer）
  std::unordered_map<Argument*, bool> mIsArgumentWrite;
  std::unordered_map<Function*, bool> mIsLib;  // 当前函数是否为lib函数
  // 当前函数的参数中有哪些是指针参数
  std::unordered_map<Function*, std::set<Argument*>> mFuncPointerArgs;
  // 当前函数有无调用库函数或者简介调用库函数
  std::unordered_map<Function*, bool> mIsCallLibFunc;
  // 出现了无法分析基址的情况，含有潜在的副作用
  std::unordered_map<Function*, bool> mHasPotentialSideEffect;
  // 当前函数直接读取的gv
  std::unordered_map<Function*, std::set<GlobalVariable*>> mFuncReadDirectGvs;
  // 当前函数直接写入的gv
  std::unordered_map<Function*, std::set<GlobalVariable*>> mFuncWriteDirectGvs;

public:
  SideEffectInfo(Module* ctx, TopAnalysisInfoManager* tp) : ModuleACtx(ctx, tp) {}
  void clearAll() {
    mFuncReadGlobals.clear();
    mFuncWriteGlobals.clear();
    mIsArgumentRead.clear();
    mIsArgumentWrite.clear();
    mIsLib.clear();
    mFuncPointerArgs.clear();
    mIsCallLibFunc.clear();
    mHasPotentialSideEffect.clear();
    mFuncReadDirectGvs.clear();
    mFuncWriteDirectGvs.clear();
  }
  void refresh() override;
  // get
  bool getArgRead(Argument* arg) { return mIsArgumentRead[arg]; }
  bool getArgWrite(Argument* arg) { return mIsArgumentWrite[arg]; }
  bool getIsLIb(Function* func) { return mIsLib[func]; }
  bool getIsCallLib(Function* func) { return mIsCallLibFunc[func]; }
  bool getPotentialSideEffect(Function* func) { return mHasPotentialSideEffect[func]; }
  // set
  void setArgRead(Argument* arg, bool b) { mIsArgumentRead[arg] = b; }
  void setArgWrite(Argument* arg, bool b) { mIsArgumentWrite[arg] = b; }
  void setFuncIsLIb(Function* func, bool b) { mIsLib[func] = b; }
  void setFuncIsCallLib(Function* func, bool b) { mIsCallLibFunc[func] = b; }
  void setPotentialSideEffect(Function* func, bool b) { mHasPotentialSideEffect[func] = b; }
  // reference
  auto& funcReadGlobals(Function* func) { return mFuncReadGlobals[func]; }
  auto& funcWriteGlobals(Function* func) { return mFuncWriteGlobals[func]; }
  auto& funcArgSet(Function* func) { return mFuncPointerArgs[func]; }
  auto& funcDirectReadGvs(Function* func) { return mFuncReadDirectGvs[func]; }
  auto& funcDirectWriteGvs(Function* func) { return mFuncWriteDirectGvs[func]; }
  // old API
  bool hasSideEffect(Function* func) {
    if (mIsLib[func]) return true;
    if (mIsCallLibFunc[func]) return true;
    if (not mFuncWriteGlobals[func].empty()) return true;
    if (mHasPotentialSideEffect[func]) return true;

    for (auto arg : mFuncPointerArgs[func]) {
      if (getArgWrite(arg)) return true;
    }
    return false;
  }
  bool isPureFunc(Function* func) {
    for (auto arg : funcArgSet(func)) {
      if (getArgRead(arg)) return false;
    }
    return (not hasSideEffect(func) and mFuncReadGlobals[func].empty()) and not mIsLib[func];
  }
  bool isInputOnlyFunc(Function* func) {
    if (hasSideEffect(func)) return false;
    if (not mFuncReadGlobals[func].empty()) return false;
    return true;
  }
  void functionInit(Function* func) {
    mFuncReadGlobals[func] = std::set<GlobalVariable*>();
    mFuncWriteGlobals[func] = std::set<GlobalVariable*>();
    mFuncWriteDirectGvs[func] = std::set<GlobalVariable*>();
    mFuncReadDirectGvs[func] = std::set<GlobalVariable*>();
    for (auto arg : func->args()) {
      if (not arg->isPointer()) continue;
      mFuncPointerArgs[func].insert(arg);
      setArgRead(arg, false);
      setArgWrite(arg, false);
    }
    mIsCallLibFunc[func] = false;
  }
};

class DependenceInfo : public FunctionACtx {
private:
  std::unordered_map<Loop*, LoopDependenceInfo*> mFunc2LoopDepInfo;

public:
  DependenceInfo(Function* func, TopAnalysisInfoManager* tp) : FunctionACtx(func, tp) {}
  LoopDependenceInfo* getLoopDependenceInfo(Loop* lp) {
    if (mFunc2LoopDepInfo.count(lp))
      return mFunc2LoopDepInfo[lp];
    else
      return nullptr;
  }
  void clearAll() { mFunc2LoopDepInfo.clear(); }
  void refresh() override;
  void setDepInfoLp(Loop* lp, LoopDependenceInfo* input) { mFunc2LoopDepInfo[lp] = input; }
};

class ParallelInfo : public FunctionACtx {
  // 你想并行的这里都有!
private:
  std::unordered_map<BasicBlock*, bool> mLpIsParallel;
  std::unordered_map<BasicBlock*, std::set<PhiInst*>> mLpPhis;
  std::unordered_map<PhiInst*, bool> mIsPhiAdd;
  std::unordered_map<PhiInst*, bool> mIsPhiSub;
  std::unordered_map<PhiInst*, bool> mIsPhiMul;
  std::unordered_map<PhiInst*, Value*> mModuloVal;

public:
  ParallelInfo(Function* func, TopAnalysisInfoManager* tp) : FunctionACtx(func, tp) {}
  void setIsParallel(BasicBlock* header, bool b) {
    std::cerr << "set " << header->name() << " is parallel " << b << std::endl;
    mLpIsParallel[header] = b;
  }
  bool getIsParallel(BasicBlock* lp) {
    if (mLpIsParallel.count(lp)) {
      return mLpIsParallel[lp];
    }
    assert(false and "input an unexistend loop in ");
  }
  std::set<PhiInst*>& resPhi(BasicBlock* bb) { return mLpPhis[bb]; }
  void clearAll() {
    mLpIsParallel.clear();
    mLpPhis.clear();
  }
  void refresh() {}
  // set
  void setPhi(PhiInst* phi, bool isadd, bool issub, bool ismul, Value* mod) {
    mIsPhiAdd[phi] = isadd;
    mIsPhiMul[phi] = ismul;
    mIsPhiSub[phi] = issub;
    mModuloVal[phi] = mod;
  }
  // get
  bool getIsAdd(PhiInst* phi) {
    assert(false and "can not use!");
    return mIsPhiAdd.at(phi);
  }
  bool getIsSub(PhiInst* phi) {
    assert(false and "can not use!");
    return mIsPhiSub.at(phi);
  }
  bool getIsMul(PhiInst* phi) {
    assert(false and "can not use!");
    return mIsPhiMul.at(phi);
  }
  Value* getMod(PhiInst* phi) {
    assert(false and "can not use!");
    return mModuloVal.at(phi);
  }
};

};  // namespace pass
