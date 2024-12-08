#pragma once
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include "ir/ir.hpp"
#include "pass/pass.hpp"

using namespace ir;
namespace pass {

struct LoopBodyInfo {
  CallInst* callInst;
  IndVar* indVar;
  
  BasicBlock* preHeader;
  BasicBlock* header;
  BasicBlock* body;
  BasicBlock* latch;
  BasicBlock* exit;

  PhiInst* giv;
  // Value* givInit;
  Value* givLoopInit;
  bool givUsedByOuter;
  bool givUsedByInner;

  void print(std::ostream& os) const;
};

class LoopBodyExtract : public FunctionPass {
public:
  void run(Function* func, TopAnalysisInfoManager* tp) override;
  std::string name() const override { return "LoopBodyExtract"; }

private:
  bool runImpl(Function* func, TopAnalysisInfoManager* tp);
};

bool extractLoopBody(Function* func,
                     Loop* loop,
                     IndVar* indVar,
                     TopAnalysisInfoManager* tp,
                     LoopBodyInfo& loopBodyInfo);

}  // namespace pass