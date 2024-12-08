// idom create algorithm from paper by Lengauer and Tarjan
// paper name: A fast algorithm for finding dominators in a flowgraph
// by: Thomas Lengauer and Robert Endre Tarjan
#include "pass/analysis/dom.hpp"
#include <set>
#include <map>
#include <algorithm>

static std::unordered_map<ir::BasicBlock*, ir::BasicBlock*> parent;
static std::unordered_map<ir::BasicBlock*, int> semi;
static std::vector<ir::BasicBlock*> vertex;
using bbset = std::set<ir::BasicBlock*>;
static std::unordered_map<ir::BasicBlock*, bbset> bucket;
static std::unordered_map<ir::BasicBlock*, ir::BasicBlock*> idom;
static std::unordered_map<ir::BasicBlock*, ir::BasicBlock*> ancestor;
static std::unordered_map<ir::BasicBlock*, ir::BasicBlock*> child;
static std::unordered_map<ir::BasicBlock*, int> size;
static std::unordered_map<ir::BasicBlock*, ir::BasicBlock*> label;
static int dfc;

namespace pass {
// pre process for dom calc
void PreProcDom::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  if (func->isOnlyDeclare()) return;
  auto blocklist = func->blocks();
  std::vector<ir::BasicBlock*> worklist;
  for (auto bbiter = blocklist.begin(); bbiter != blocklist.end();) {
    auto bb = *bbiter;
    if (bb->pre_blocks().empty() and bb != func->entry()) {
      bbiter++;
      worklist.push_back(bb);
    } else {
      bbiter++;
    }
  }
  while (not worklist.empty()) {
    auto bbcur = worklist.back();
    worklist.pop_back();
    for (auto bbcurnext : bbcur->next_blocks()) {
      if (bbcurnext->pre_blocks().size() == 1) worklist.push_back(bbcurnext);
    }
    func->forceDelBlock(bbcur);
  }
}
// LT algorithm to get idom and sdom
void IDomGen::compress(ir::BasicBlock* bb) {
  auto ancestorBB = ancestor.at(bb);
  if (ancestor[ancestorBB]) {
    compress(ancestorBB);
    // if (semi[label[ancestorBB]] < semi[label.at(bb)]) {
    if (semi.at(label.at(ancestorBB)) < semi.at(label.at(bb))) {
      label[bb] = label[ancestorBB];
    }
    ancestor[bb] = ancestor[ancestorBB];
  }
}

void IDomGen::link(ir::BasicBlock* v, ir::BasicBlock* w) {
  auto s = w;
  while (semi[label[w]] < semi[label[child[s]]]) {
    if (size[s] + size[child[child[s]]] >= 2 * size[child[s]]) {
      ancestor[child[s]] = s;
      child[s] = child[child[s]];
    } else {
      size[child[s]] = size[s];
      s = ancestor[s] = child[s];
    }
  }
  label[s] = label[w];
  size[v] = size[v] + size[w];
  if (size[v] < 2 * size[w]) {
    auto tmp = s;
    s = child[v];
    child[v] = tmp;
  }
  while (s) {
    ancestor[s] = v;
    s = child[s];
  }
}

ir::BasicBlock* IDomGen::eval(ir::BasicBlock* bb) {
  if (ancestor[bb] == 0) {
    return label[bb];
  }
  compress(bb);
  return (semi[label[ancestor[bb]]] >= semi[label[bb]]) ? label[bb] : label[ancestor[bb]];
}

void IDomGen::dfsBlocks(ir::BasicBlock* bb) {
  semi[bb] = dfc++;
  vertex.push_back(bb);
  for (auto bbnext : bb->next_blocks()) {
    if (semi[bbnext] == 0) {
      parent[bbnext] = bb;
      dfsBlocks(bbnext);
    }
  }
}

void IDomGen::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  if (func->isOnlyDeclare()) return;
  domctx = tp->getDomTreeWithoutRefresh(func);
  domctx->clearAll();
  domctx->initialize();

  parent.clear();
  semi.clear();
  vertex.clear();
  bucket.clear();
  idom.clear();
  ancestor.clear();
  child.clear();
  size.clear();
  label.clear();
  // step 1
  // initialize all arrays and maps
  for (auto bb : func->blocks()) {
    semi[bb] = 0;
    ancestor[bb] = nullptr;
    child[bb] = nullptr;
    label[bb] = bb;
    size[bb] = 1;
    // bb->idom=nullptr;
    // bb->sdom=nullptr;
    domctx->set_idom(bb, nullptr);
    domctx->set_sdom(bb, nullptr);
  }
  semi[nullptr] = 0;
  label[nullptr] = nullptr;
  size[nullptr] = 0;
  // dfs
  dfc = 0;  // can't static def in dfs func, think about why
  dfsBlocks(func->entry());
  // step2 and 3
  for (auto bbIter = vertex.rbegin(); bbIter != vertex.rend(); bbIter++) {
    auto w = *bbIter;
    if (!parent[w]) continue;

    for (auto v : w->pre_blocks()) {
      auto u = eval(v);
      if (semi[u] < semi[w]) semi[w] = semi[u];
    }
    bucket[vertex[semi[w]]].insert(w);
    link(parent[w], w);
    auto tmp = bucket[parent[w]];
    for (auto v : tmp) {
      bucket[parent[w]].erase(v);
      auto u = eval(v);
      idom[v] = (semi[u] < semi[v]) ? u : parent[w];
    }
  }

  // step4
  for (auto bbIter = vertex.begin(); bbIter != vertex.end(); bbIter++) {
    auto w = *bbIter;
    if (idom[w] != vertex[semi[w]]) idom[w] = idom[idom[w]];
  }
  idom[func->entry()] = nullptr;

  // extra step, store informations into BasicBlocks
  for (auto bb : func->blocks()) {
    // bb->idom=idom[bb];
    // bb->sdom=vertex[semi[bb]];
    domctx->set_idom(bb, idom[bb]);
    domctx->set_sdom(bb, vertex[semi[bb]]);
  }
}

void DomFrontierGen::getDomTree(ir::Function* func) {
  // for(auto bb : func->blocks())
  //     bb->DomTree.clear();
  for (auto bb : func->blocks()) {
    // if(bb->idom)
    //     bb->idom->DomTree.push_back(bb);
    if (domctx->idom(bb)) {
      domctx->domson(domctx->idom(bb)).push_back(bb);
    }
  }
}

void DomFrontierGen::getDomInfo(ir::BasicBlock* bb, int level) {
  // bb->domLevel=level;
  domctx->set_domlevel(bb, level);
  for (auto bbnext : domctx->domson(bb)) {  //: bb->DomTree
    getDomInfo(bbnext, level + 1);
  }
}

void DomFrontierGen::getDomFrontier(ir::Function* func) {
  // for(auto bb : func->blocks())
  //     bb->domFrontier.clear();
  // func->print(std::cout);
  for (auto bb : func->blocks()) {
    if (bb->pre_blocks().size() > 1) {
      for (auto bbnext : bb->pre_blocks()) {
        auto runner = bbnext;
        // while(runner!=bb->idom){
        //     runner->domFrontier.push_back(bb);
        //     runner=runner->idom;
        // }
        while (runner != domctx->idom(bb)) {
          domctx->domfrontier(runner).push_back(bb);
          runner = domctx->idom(runner);
        }
      }
    }
  }
}

// generate dom tree
void DomFrontierGen::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  if (func->isOnlyDeclare()) return;
  domctx = tp->getDomTreeWithoutRefresh(func);
  getDomTree(func);
  getDomInfo(func->entry(), 0);
  getDomFrontier(func);
}

// debug info print pass
void DomInfoCheck::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  if (func->isOnlyDeclare()) return;
  func->rename();
  domctx = tp->getDomTreeWithoutRefresh(func);
  using namespace std;
  cout << "In Function " << func->name() << "" << endl;
  for (auto bb : func->blocks()) {
    cout << bb->name() << " Prec: ";
    for (auto bbpre : bb->pre_blocks()) {
      cout << "\t" << bbpre->name();
    }
    cout << endl;
  }
  cout << endl;
  for (auto bb : func->blocks()) {
    cout << bb->name() << " Succ: ";
    for (auto bbnext : bb->next_blocks()) {
      cout << "\t" << bbnext->name();
    }
    cout << endl;
  }
  cout << endl;
  for (auto bb : func->blocks()) {
    cout << bb->name() << " idom: ";
    if (domctx->idom(bb))
      cout << "\t" << domctx->idom(bb)->name();
    else
      cout << "null";
    cout << endl;
  }
  cout << endl;
  for (auto bb : func->blocks()) {
    cout << bb->name() << " sdom: ";
    if (domctx->sdom(bb))
      cout << "\t" << domctx->sdom(bb)->name();
    else
      cout << "null";
    cout << endl;
  }
  cout << endl;
  for (auto bb : func->blocks()) {
    cout << bb->name() << " domTreeSons: ";
    for (auto bbson : domctx->domson(bb)) {
      cout << bbson->name() << '\t';
    }
    cout << endl;
  }
  cout << endl;
  for (auto bb : func->blocks()) {
    cout << bb->name() << " domFrontier: ";
    for (auto bbf : domctx->domfrontier(bb)) {
      cout << bbf->name() << '\t';
    }
    cout << endl;
  }
  cout << endl;
  domctx->BFSDomTreeInfoRefresh();
  cout << "BFSDomTreeVector:" << endl;
  for (auto bb : domctx->BFSDomTreeVector()) {
    cout << bb->name() << ":" << bb->insts().size() << "\t";
  }
  cout << endl << endl;

  cout << "BBs:" << endl;
  for (auto bb : func->blocks()) {
    cout << bb->name() << ":" << bb->insts().size() << "\t";
  }
  cout << endl << endl;
}

void DomInfoAnalysis::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  PreProcDom ppd = PreProcDom();
  IDomGen idg = IDomGen();
  DomFrontierGen dfg = DomFrontierGen();
  DomInfoCheck dic = DomInfoCheck();
  ppd.run(func, tp);
  idg.run(func, tp);
  dfg.run(func, tp);
  // dic.run(func,tp);
}

}  // namespace pass