#include "ir/ir.hpp"
#include "pass/AnalysisInfo.hpp"
#include "pass/analysis/dom.hpp"
#include "pass/analysis/pdom.hpp"
#include "pass/analysis/callgraph.hpp"
#include "pass/analysis/loop.hpp"
#include "pass/analysis/indvar.hpp"
#include "pass/analysis/sideEffectAnalysis.hpp"
#include "pass/analysis/dependenceAnalysis/DependenceAnalysis.hpp"
#include "pass/analysis/dependenceAnalysis/dpaUtils.hpp"
using namespace ir;
using namespace pass;

void TopAnalysisInfoManager::initialize() {
  mCallGraph = new CallGraph(mModule, this);
  mSideEffectInfo = new SideEffectInfo(mModule, this);
  for (auto func : mModule->funcs()) {
    if (func->blocks().empty()) continue;
    mDomTree[func] = new DomTree(func, this);
    mPDomTree[func] = new PDomTree(func, this);
    mLoopInfo[func] = new LoopInfo(func, this);
    mIndVarInfo[func] = new IndVarInfo(func, this);
    mDepInfo[func] = new DependenceInfo(func, this);
    mParallelInfo[func] = new ParallelInfo(func, this);
  }
}

bool DomTree::dominate(BasicBlock* bb1, BasicBlock* bb2) {
  if (bb1 == bb2) return true;
  auto bbIdom = miDom[bb2];
  while (bbIdom != nullptr) {
    if (bbIdom == bb1) return true;
    bbIdom = miDom[bbIdom];
  }
  return false;
}
void DomTree::BFSDomTreeInfoRefresh() {
  std::queue<BasicBlock*> bbqueue;
  std::unordered_map<BasicBlock*, bool> vis;
  for (auto bb : passUnit->blocks())
    vis[bb] = false;

  mBFSDomTreeVector.clear();
  bbqueue.push(passUnit->entry());

  while (!bbqueue.empty()) {
    auto bb = bbqueue.front();
    bbqueue.pop();
    if (!vis[bb]) {
      mBFSDomTreeVector.push_back(bb);
      vis[bb] = true;
      for (auto bbDomSon : mDomSon[bb])
        bbqueue.push(bbDomSon);
    }
  }
}
void DomTree::DFSDomTreeInfoRefresh() {
  std::stack<BasicBlock*> bbstack;
  std::unordered_map<BasicBlock*, bool> vis;
  for (auto bb : passUnit->blocks())
    vis[bb] = false;

  mDFSDomTreeVector.clear();
  bbstack.push(passUnit->entry());

  while (!bbstack.empty()) {
    auto bb = bbstack.top();
    bbstack.pop();
    if (!vis[bb]) {
      mDFSDomTreeVector.push_back(bb);
      vis[bb] = true;
      for (auto bbDomSon : mDomSon[bb])
        bbstack.push(bbDomSon);
    }
  }
}
void DomTree::refresh() {
  using namespace pass;
  auto dip = DomInfoAnalysis();
  dip.run(passUnit, topManager);
  setOn();
}

bool PDomTree::pdominate(BasicBlock* bb1, BasicBlock* bb2) {
  if (bb1 == bb2) return true;
  auto bbIdom = mipDom[bb2];
  while (bbIdom != nullptr) {
    if (bbIdom == bb1) return true;
    bbIdom = mipDom[bbIdom];
  }
  return false;
}

void PDomTree::refresh() {
  using namespace pass;
  auto pdi = PostDomInfoPass();
  pdi.run(passUnit, topManager);
  setOn();
}

void LoopInfo::refresh() {
  using namespace pass;
  auto la = LoopAnalysis();
  // loopInfoCheck lic = loopInfoCheck();
  la.run(passUnit, topManager);
  // lic.run(passUnit,topManager);
  setOn();
}
void LoopInfo::print(std::ostream& os) const {
  os << "Loop Info:\n";
  for (auto loop : mLoops) {
    std::cerr << "level: " << mLoopLevel.at(loop->header()) << std::endl;
    loop->print(os);
  }
  std::cerr << std::endl;
}
// looplevel small to big
std::vector<ir::Loop*> LoopInfo::sortedLoops(bool reverse) {
  auto loops = mLoops;
  std::sort(loops.begin(), loops.end(), [&](Loop* lhs, Loop* rhs) {
    return mLoopLevel.at(lhs->header()) < mLoopLevel.at(rhs->header());
  });
  if (reverse) std::reverse(loops.begin(), loops.end());
  return std::move(loops);
}

Loop* LoopInfo::getinnermostLoop(BasicBlock* bb) {  // 返回最内层的循环
  Loop* innermost = nullptr;
  for (auto L : mLoops) {
    if (L->contains(bb)) {
      if (innermost == nullptr)
        innermost = L;
      else {
        if (mLoopLevel[L->header()] < mLoopLevel[innermost->header()]) innermost = L;
      }
    }
  }
  return innermost;
}
void CallGraph::refresh() {
  using namespace pass;
  auto cgb = CallGraphBuild();
  cgb.run(passUnit, topManager);

  setOn();
}

void IndVarInfo::refresh() {
  using namespace pass;
  // PassManager pm = PassManager(passUnit->module(), topManager);
  auto iva = IndVarAnalysis();
  // indVarInfoCheck ivc = indVarInfoCheck();
  iva.run(passUnit, topManager);
  // ivc.run(passUnit,topManager);
  setOn();
}

void SideEffectInfo::refresh() {
  using namespace pass;
  // PassManager pm = PassManager(passUnit, topManager);
  SideEffectAnalysis sea = SideEffectAnalysis();
  sea.run(passUnit, topManager);
  setOn();
}

void DependenceInfo::refresh() {
  using namespace pass;
  auto da = DependenceAnalysis();
  da.run(passUnit, topManager);
  setOn();
}
