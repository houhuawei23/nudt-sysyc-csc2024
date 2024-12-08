#pragma once
#include "pass/pass.hpp"

namespace pass {

class DomInfoAnalysis : public FunctionPass {
public:
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "DomInfoPass"; }
};

class PreProcDom : public FunctionPass {
private:
  DomTree* domctx;

public:
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "PreProcDom"; }
};

class IDomGen : public FunctionPass {
private:
  DomTree* domctx;

private:
  void dfsBlocks(ir::BasicBlock* bb);
  ir::BasicBlock* eval(ir::BasicBlock* bb);
  void link(ir::BasicBlock* v, ir::BasicBlock* w);
  void compress(ir::BasicBlock* bb);

public:
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "IdomGen"; }
};

class DomFrontierGen : public FunctionPass {
private:
  DomTree* domctx;

private:
  void getDomTree(ir::Function* func);
  void getDomFrontier(ir::Function* func);
  void getDomInfo(ir::BasicBlock* bb, int level);

public:
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "DomFrontierGen"; }
};

class DomInfoCheck : public FunctionPass {
private:
  DomTree* domctx;

public:
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "DomInfoCheck"; }
};

}  // namespace pass
