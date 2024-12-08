// idom create algorithm from paper by Lengauer and Tarjan
// paper name: A fast algorithm for finding dominators in a flowgraph
// by: Thomas Lengauer and Robert Endre Tarjan
#include "pass/analysis/pdom.hpp"
#include <set>
#include <map>
#include <algorithm>

using namespace ir;

static std::unordered_map<BasicBlock*, BasicBlock*> parent;
static std::unordered_map<BasicBlock*, int> semi;
static std::vector<BasicBlock*> vertex;
using bbset = std::set<BasicBlock*>;
static std::unordered_map<BasicBlock*, bbset> bucket;
static std::unordered_map<BasicBlock*, BasicBlock*> idom;
static std::unordered_map<BasicBlock*, BasicBlock*> ancestor;
static std::unordered_map<BasicBlock*, BasicBlock*> child;
static std::unordered_map<BasicBlock*, int> size;
static std::unordered_map<BasicBlock*, BasicBlock*> label;
static int dfc;

namespace pass {
// pre process for dom calc
void PreProcPostDom::run(Function* func, TopAnalysisInfoManager* tp) {
  if (func->isOnlyDeclare()) return;
  auto blocklist = func->blocks();
  for (auto bbiter = blocklist.begin(); bbiter != blocklist.end();) {
    auto bb = *bbiter;
    if (bb->next_blocks().empty() and bb != func->exit()) {
      bbiter++;
      func->forceDelBlock(bb);
    } else {
      bbiter++;
    }
  }
}
// LT algorithm to get idom and sdom
void IPostDomGen::compress(BasicBlock* bb) {
  auto ancestorBB = ancestor[bb];
  if (ancestor[ancestorBB]) {
    compress(ancestorBB);
    if (semi[label[ancestorBB]] < semi[label[bb]]) {
      label[bb] = label[ancestorBB];
    }
    ancestor[bb] = ancestor[ancestorBB];
  }
}

void IPostDomGen::link(BasicBlock* v, BasicBlock* w) {
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

BasicBlock* IPostDomGen::eval(BasicBlock* bb) {
  if (ancestor[bb] == 0) {
    return label[bb];
  }
  compress(bb);
  return (semi[label[ancestor[bb]]] >= semi[label[bb]]) ? label[bb] : label[ancestor[bb]];
}

void IPostDomGen::dfsBlocks(BasicBlock* bb) {
  semi[bb] = dfc++;
  vertex.push_back(bb);
  for (auto bbnext : bb->pre_blocks()) {
    if (semi[bbnext] == 0) {
      parent[bbnext] = bb;
      dfsBlocks(bbnext);
    }
  }
}

void IPostDomGen::run(Function* func, TopAnalysisInfoManager* tp) {
  if (func->isOnlyDeclare()) return;
  pdctx = tp->getPDomTreeWithoutRefresh(func);
  pdctx->clearAll();
  pdctx->initialize();

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
    // bb->ipdom=nullptr;
    // bb->spdom=nullptr;
    // bb->PDomTree.clear();
    // bb->pdomFrontier.clear();
    pdctx->set_ipdom(bb, nullptr);
    pdctx->set_spdom(bb, nullptr);
  }
  semi[nullptr] = 0;
  label[nullptr] = nullptr;
  size[nullptr] = 0;
  // dfs
  dfc = 0;  // can't static def in dfs func, think about why
  dfsBlocks(func->exit());
  // step2 and 3
  for (auto bbIter = vertex.rbegin(); bbIter != vertex.rend(); bbIter++) {
    auto w = *bbIter;
    if (!parent[w]) continue;

    for (auto v : w->next_blocks()) {
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
  idom[func->exit()] = nullptr;

  // extra step, store informations into BasicBlocks
  for (auto bb : func->blocks()) {
    // bb->ipdom=idom[bb];
    // bb->spdom=vertex[semi[bb]];
    pdctx->set_ipdom(bb, idom[bb]);
    pdctx->set_spdom(bb, vertex[semi[bb]]);
  }
}

void PostDomFrontierGen::getDomTree(Function* func) {
  // for(auto bb : func->blocks())
  //     bb->PDomTree.clear();
  for (auto bb : func->blocks()) {
    // if(bb->ipdom)
    //     bb->ipdom->PDomTree.push_back(bb);
    if (pdctx->ipdom(bb)) {
      pdctx->pdomson(pdctx->ipdom(bb)).push_back(bb);
    }
  }
}

void PostDomFrontierGen::getDomInfo(BasicBlock* bb, int level) {
  // bb->pdomLevel=level;
  pdctx->set_pdomlevel(bb, level);
  for (auto bbnext : pdctx->pdomson(bb)) {  // bb->PDomTree
    getDomInfo(bbnext, level + 1);
  }
}

void PostDomFrontierGen::getDomFrontier(Function* func) {
  // for(auto bb : func->blocks())
  //     bb->pdomFrontier.clear();
  for (auto bb : func->blocks()) {
    if (bb->next_blocks().size() > 1) {
      for (auto bbnext : bb->next_blocks()) {
        auto runner = bbnext;
        // while(runner!=bb->ipdom){
        //     runner->pdomFrontier.push_back(bb);
        //     runner=runner->ipdom;
        // }
        while (runner != pdctx->ipdom(bb)) {
          pdctx->pdomfrontier(runner).push_back(bb);
          runner = pdctx->ipdom(runner);
        }
      }
    }
  }
}

// generate dom tree
void PostDomFrontierGen::run(Function* func, TopAnalysisInfoManager* tp) {
  if (func->isOnlyDeclare()) return;
  pdctx = tp->getPDomTreeWithoutRefresh(func);
  getDomTree(func);
  getDomInfo(func->entry(), 0);
  getDomFrontier(func);
}

// debug info print pass
void PostDomInfoCheck::run(Function* func, TopAnalysisInfoManager* tp) {
  if (func->isOnlyDeclare()) return;
  pdctx = tp->getPDomTreeWithoutRefresh(func);
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
    cout << bb->name() << " ipdom: ";
    if (pdctx->ipdom(bb))
      cout << "\t" << pdctx->ipdom(bb)->name();
    else
      cout << "null";
    cout << endl;
  }
  cout << endl;
  for (auto bb : func->blocks()) {
    cout << bb->name() << " spdom: ";
    if (pdctx->spdom(bb))
      cout << "\t" << pdctx->spdom(bb)->name();
    else
      cout << "null";
    cout << endl;
  }
  cout << endl;
  for (auto bb : func->blocks()) {
    cout << bb->name() << " pdomTreeSons: ";
    for (auto bbson : pdctx->pdomson(bb)) {
      cout << bbson->name() << '\t';
    }
    cout << endl;
  }
  cout << endl;
  for (auto bb : func->blocks()) {
    cout << bb->name() << " pdomFrontier: ";
    for (auto bbf : pdctx->pdomson(bb)) {
      cout << bbf->name() << '\t';
    }
    cout << endl;
  }
}

void PostDomInfoPass::run(Function* func, TopAnalysisInfoManager* tp) {
  PreProcPostDom ppd = PreProcPostDom();
  IPostDomGen idg = IPostDomGen();
  PostDomFrontierGen dfg = PostDomFrontierGen();
  ppd.run(func, tp);
  idg.run(func, tp);
  dfg.run(func, tp);
}

}  // namespace pass