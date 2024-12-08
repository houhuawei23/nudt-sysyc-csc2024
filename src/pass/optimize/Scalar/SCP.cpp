#include "pass/optimize/optimize.hpp"
#include "pass/optimize/SCP.hpp"
#include <vector>
// 当前是简单常量传播遍 Simple Constant Propagation
static std::unordered_set<ir::Instruction*> worklist;

namespace pass {
void SCP::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  if (func->isOnlyDeclare()) return;
  // func->print(std::cout);
  worklist.clear();
  for (auto bb : func->blocks()) {
    for (auto instIter = bb->insts().begin(); instIter != bb->insts().end();) {
      auto curInst = *instIter;
      instIter++;
      if (curInst->getConstantRepl()) worklist.insert(curInst);
    }
  }
  while (!worklist.empty()) {
    auto curInst = worklist.begin();
    worklist.erase(curInst);
    addConstFlod(*curInst);
  }
}

void SCP::addConstFlod(ir::Instruction* inst) {
  auto replval = inst->getConstantRepl();
  for (auto puseIter = inst->uses().begin(); puseIter != inst->uses().end();) {
    auto puse = *puseIter;
    puseIter++;
    auto puser = puse->user();
    puser->setOperand(puse->index(), replval);
    auto puserInst = dyn_cast<ir::Instruction>(puser);
    assert(puserInst);
    if (puserInst->getConstantRepl()) {
      worklist.insert(puserInst);
    }
  }
  inst->uses().clear();
  inst->block()->delete_inst(inst);
}

}  // namespace pass