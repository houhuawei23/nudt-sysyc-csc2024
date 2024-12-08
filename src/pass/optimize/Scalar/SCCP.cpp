#include "pass/optimize/SCCP.hpp"

static std::set<ir::Instruction*> worklist;
static std::unordered_set<ir::BasicBlock*> liveBB;
static std::unordered_set<ir::BasicBlock*> visBB;
namespace pass {
void SCCP::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  bool isChange = false;
  bool isCFGChange = false;
  do {
    isChange = false;
    isCFGChange = false;
    isChange |= SCPrun(func, tp);
    isCFGChange |= cleanCFG(func);
    isChange = isCFGChange or isChange;
  } while (isChange);
  if (isCFGChange) {
    tp->CFGChange(func);
    tp->CallChange();
  }
}

bool SCCP::cleanCFG(ir::Function* func) {
  bool reb = false;
  liveBB.clear();
  visBB.clear();
  searchCFG(func->entry());
  for (auto bbIter = func->blocks().begin(); bbIter != func->blocks().end();) {
    auto bb = *bbIter;
    bbIter++;
    if (liveBB.count(bb)) continue;
    for (auto bbnext : bb->next_blocks()) {
      for (auto pinst : bbnext->phi_insts()) {
        auto phiinst = pinst->dynCast<ir::PhiInst>();
        phiinst->delBlock(bb);
      }
    }
    func->forceDelBlock(bb);
    reb = true;
  }
  return reb;
}

void SCCP::searchCFG(ir::BasicBlock* bb) {
  if (visBB.count(bb)) return;
  visBB.insert(bb);
  liveBB.insert(bb);
  auto terminator = bb->terminator();
  auto terBrInst = dyn_cast<ir::BranchInst>(terminator);
  if (terminator->valueId() == ir::vRETURN) return;
  assert(terBrInst != nullptr);
  if (terBrInst->is_cond()) {
    if (terBrInst->cond()->valueId() == ir::vCONSTANT) {
      auto constCond = terBrInst->cond()->dynCast<ir::ConstantValue>();
      if (constCond->i1()) {
        searchCFG(terBrInst->iftrue());
        for (auto pinst : terBrInst->iffalse()->phi_insts()) {
          auto phiinst = dyn_cast<ir::PhiInst>(pinst);
          phiinst->delBlock(bb);
        }
        ir::BasicBlock::delete_block_link(bb, terBrInst->iffalse());
        auto newBrInst = new ir::BranchInst(terBrInst->iftrue(), bb);
        bb->delete_inst(terBrInst);
        bb->emplace_back_inst(newBrInst);

      } else {
        searchCFG(terBrInst->iffalse());
        for (auto pinst : terBrInst->iftrue()->phi_insts()) {
          auto phiinst = dyn_cast<ir::PhiInst>(pinst);
          phiinst->delBlock(bb);
        }
        ir::BasicBlock::delete_block_link(bb, terBrInst->iftrue());
        auto newBrInst = new ir::BranchInst(terBrInst->iffalse(), bb);
        bb->delete_inst(terBrInst);
        bb->emplace_back_inst(newBrInst);
      }
    } else {
      searchCFG(terBrInst->iftrue());
      searchCFG(terBrInst->iffalse());
    }
  } else {
    searchCFG(terBrInst->dest());
  }
}

bool SCCP::SCPrun(ir::Function* func, TopAnalysisInfoManager* tp) {
  bool isChange = false;
  if (func->isOnlyDeclare()) return false;
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
    isChange = isChange or addConstFlod(*curInst);
  }
  return isChange;
}

bool SCCP::addConstFlod(ir::Instruction* inst) {
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
  return true;
}
}  // namespace pass