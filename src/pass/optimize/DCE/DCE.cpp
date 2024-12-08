#include "pass/optimize/DCE.hpp"

static std::unordered_set<ir::Instruction*> alive;

namespace pass {

void DCE::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  alive.clear();
  if (func->isOnlyDeclare()) return;
  // func->rename();
  // func->print(std::cout);
  for (auto bb : func->blocks()) {
    // 扫描所有的指令,只要是isAlive的就加到alive列表中
    for (auto inst : bb->insts()) {
      if (isAlive(inst)) addAlive(inst);
    }
  }

  for (auto bb : func->blocks()) {
    for (auto instIter = bb->insts().begin(); instIter != bb->insts().end();) {
      auto curIter = *instIter;
      instIter++;
      if (alive.count(curIter) == 0) {
        bb->force_delete_inst(curIter);
      }
    }
  }
  // func->print(std::cout);
}

bool DCE::isAlive(ir::Instruction* inst) {  // 只有store,terminator和call inst是活的
  return inst->isNoName() or dyn_cast<ir::CallInst>(inst) or inst->valueId() == ir::vATOMICRMW;
}

void DCE::addAlive(ir::Instruction* inst) {  // 递归的将活代码和他的依赖加入到alive列表当中
  alive.insert(inst);
  for (auto op : inst->operands()) {
    auto opInst = dyn_cast<ir::Instruction>(op->value());
    if (opInst == nullptr) continue;
    // std::cout<<alive.count(opInst)<<std::endl;
    if (alive.count(opInst) == 0) {
      addAlive(opInst);
    }
  }
  inst->block();
}

}  // namespace pass