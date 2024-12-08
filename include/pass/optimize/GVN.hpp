#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include "ir/ir.hpp"
#include "pass/pass.hpp"

namespace pass {

struct GVNContext {
  std::vector<ir::BasicBlock*> RPOblocks;
  std::set<ir::BasicBlock*> visited;
  std::set<ir::Instruction*> NeedRemove;
  std::unordered_map<ir::Value*, ir::Value*> _Hashtable;
  DomTree* domctx;
  SideEffectInfo* sectx;

  void run(ir::Function* func, TopAnalysisInfoManager* tp);

  void RPO(ir::Function* F);  // 逆后序遍历
  void dfs(ir::BasicBlock* bb);
  ir::Value* checkHashtable(ir::Value* v);
  ir::Value* getValueNumber(ir::Instruction* inst);
  ir::Value* getValueNumber(ir::BinaryInst* binary);
  ir::Value* getValueNumber(ir::UnaryInst* unary);
  ir::Value* getValueNumber(ir::GetElementPtrInst* getelementptr);
  ir::Value* getValueNumber(ir::LoadInst* load);
  ir::Value* getValueNumber(ir::CallInst* call);
  ir::Value* getValueNumber(ir::PtrCastInst* ptrcast);

  void visitinst(ir::Instruction* inst);
};

class GVN : public FunctionPass {
  void run(ir::Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "GVN"; }
};
}  // namespace pass