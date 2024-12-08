#pragma once
#include "ir/ir.hpp"
#include "pass/pass.hpp"

using namespace ir;
namespace pass {
class GepSplit : public FunctionPass {
  // std::unordered_map<Value>
public:
  void run(Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "GepSplit"; }

private:
  void split_pointer(GetElementPtrInst* inst, BasicBlock* insertBlock, inst_iterator insertPos);
  void split_array(inst_iterator begin, BasicBlock* insertBlock, inst_iterator end);
};
}  // namespace pass