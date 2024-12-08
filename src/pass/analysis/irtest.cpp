#include "pass/analysis/irtest.hpp"
#include "support/config.hpp"
using std::cerr, std::endl;
using namespace ir;
namespace pass {
void IRCheck::run(ir::Module* ctx, TopAnalysisInfoManager* tp) {
  const auto& config = sysy::Config::getInstance();
  const bool debug = config.logLevel >= sysy::LogLevel::DEBUG;
  ctx->rename();
  bool isPass = true;
  for (auto func : ctx->funcs()) {
    if (func->isOnlyDeclare()) continue;
    if (debug) cerr << "Testing function " << func->name() << " ..." << endl;

    isPass &= runDefUseTest(func);
    isPass &= runCFGTest(func);
    isPass &= runPhiTest(func);
    isPass &= checkFuncInfo(func);
    isPass &= checkAllocaOnlyInEntry(func);
    isPass &= checkOnlyOneExit(func);
    isPass &= checkParentRelationship(func);
    isPass &= checkOperands(func);
  }
  if (not isPass) assert(false && "didn't pass irCheck!");
}

bool IRCheck::checkDefUse(ir::Value* val) {
  bool isPass = true;
  for (auto puse : val->uses()) {
    auto muser = puse->user();
    auto mindex = puse->index();
    auto mvalue = puse->value();
    if (muser->operands()[mindex]->value() != mvalue) {
      cerr << "Value " << muser->name() << " use index " << mindex << " operand " << mvalue->name()
           << " got an error!" << endl;
      isPass = false;
    }
  }
  auto user = dyn_cast<ir::User>(val);
  if (user == nullptr) return isPass;
  int realIdx = 0;
  for (auto op : user->operands()) {
    auto muser = op->user();
    auto mvalue = op->value();
    auto mindex = op->index();
    if (mindex != realIdx) {
      cerr << "User " << muser->name() << " real idx=" << realIdx
           << " op got a wrong idx=" << mindex << endl;
      isPass = false;
    }
    int isfound = 0;
    for (auto value_s_use : mvalue->uses()) {
      if (op == value_s_use) {
        isfound++;
      }
    }
    if (not isfound) {
      cerr << "User " << muser->name() << " operand idx=" << realIdx << " not found in value "
           << mvalue->name() << " 's uses()." << endl;
      isPass = false;
    }
    if (isfound > 1) {
      cerr << "User " << muser->name() << " operand idx=" << realIdx
           << " got multiple found in value " << mvalue->name() << " 's uses()." << endl;
      isPass = false;
    }
    realIdx++;
  }
  return isPass;
}

bool IRCheck::runDefUseTest(ir::Function* func) {
  bool isPass = true;
  for (auto bb : func->blocks()) {
    bool bbOK = checkDefUse(bb);
    for (auto inst : bb->insts()) {
      bbOK = bbOK and checkDefUse(inst);
      if (bb == func->entry() and (inst->valueId() != ir::vALLOCA and inst->valueId() != ir::vBR)) {
        isPass = false;
        cerr << "Entry block has non-alloca inst!" << endl;
      }
      if (inst->valueId() == ir::vALLOCA and bb != func->entry()) {
        isPass = false;
        cerr << "AllocaInst occur in BB:" << bb->name() << " but it's not entry block!" << endl;
      }
      if (inst->block() != bb) {
        isPass = false;
        cerr << "Inst in BB:" << bb->name() << " and can't match its parent block!" << endl;
      }
      if (not bbOK) {
        isPass = false;
        cerr << "Error occur in BB:" << bb->name() << "!" << endl;
      }
    }
  }
  return isPass;
}

bool IRCheck::checkPhi(ir::PhiInst* phi) {
  bool isPass = true;
  int lim = phi->getsize();
  auto operandSize = phi->operands().size();
  if (lim != operandSize / 2) {
    cerr << "In phi " << phi->name() << ", operandsize/2 is uneqaul to phi Incoming size." << endl;
    isPass = false;
  }
  for (size_t i = 0; i < lim; i++) {
    if (phi->getvalfromBB(phi->getBlock(i)) != phi->getValue(i)) {
      cerr << "In phi " << phi->name() << " incoming has a mismatch!" << endl;
      isPass = false;
    }
  }
  return isPass;
}

bool IRCheck::runPhiTest(ir::Function* func) {
  bool isPass = true;
  for (auto bb : func->blocks()) {
    int lim = bb->phi_insts().size();
    auto instIter = bb->insts().begin();
    auto phiIter = bb->phi_insts().begin();
    while (1) {
      if (phiIter == bb->phi_insts().end()) break;
      if (*instIter != *phiIter) {
        cerr << "In BB" << bb->name() << ", we got a phiinst list error!" << endl;
        isPass = false;
      }
      isPass = isPass and checkPhi(dyn_cast<ir::PhiInst>(*phiIter));
      phiIter++;
      instIter++;
    }
    if (dyn_cast<ir::PhiInst>(*instIter) != nullptr) {
      cerr << "In BB" << bb->name() << ", we got a phiinst not in phiinst list!" << endl;
      isPass = false;
    }
  }
  // check incoming block and preblock
  for (auto block : func->blocks()) {
    for (auto inst : block->insts()) {
      if (auto phi = inst->dynCast<ir::PhiInst>()) {
        for (auto [pre, val] : phi->incomings()) {
          const auto iter = std::find(pre->next_blocks().begin(), pre->next_blocks().end(), block);
          if (iter == pre->next_blocks().end()) {
            std::cerr << "phi incoming block not in pre block list!" << std::endl;
            phi->print(std::cerr);
            std::cerr << std::endl;
            std::cerr << "block: " << block->name() << std::endl;
            std::cerr << "pre block: " << pre->name() << std::endl;
            isPass = false;
          }
        }
      }
    }
  }
  return isPass;
}

bool IRCheck::runCFGTest(ir::Function* func) {
  std::unordered_map<ir::BasicBlock*, int> bbPreSize;
  for (auto bb : func->blocks())
    bbPreSize.emplace(bb, 0);
  bool isPass = true;
  // check succ
  for (auto bb : func->blocks()) {
    auto succSet = std::set<ir::BasicBlock*>();
    auto terminator = dyn_cast<ir::BranchInst>(bb->terminator());
    if (terminator) {
      if (terminator->is_cond()) {
        succSet.insert(terminator->iftrue());
        succSet.insert(terminator->iffalse());
      } else {
        succSet.insert(terminator->dest());
      }
    }
    if (bb->next_blocks().size() != succSet.size()) {
      cerr << "Block " << bb->name() << " got invalid succBlock size!" << endl;
      isPass = false;
    }
    for (auto bbnext : bb->next_blocks()) {
      if (succSet.count(bbnext) == 0) {
        cerr << "Block " << bb->name() << " have a wrong succsecor" << endl;
        isPass = false;
      } else if (succSet.count(bbnext) == 2) {
        cerr << "Block " << bb->name() << " have a multiple succsecor" << endl;
        isPass = false;
      } else {
        bbPreSize[bbnext]++;
      }
    }
  }
  for (auto bb : func->blocks()) {
    if (bb->pre_blocks().size() != bbPreSize.at(bb)) {
      cerr << "Block " << bb->name() << " got invalid preBlock size!" << endl;
      std::cerr << "pre.size() = " << bb->pre_blocks().size()
                << ", bbPreSize.at(bb) = " << bbPreSize.at(bb) << std::endl;
      for (auto bbpre : bb->pre_blocks()) {
        std::cerr << bbpre->name() << " ";
      }
      std::cerr << std::endl;
      isPass = false;
    }
  }
  for (auto block : func->blocks()) {
    if (not block->verify(std::cerr)) {
      std::cerr << "block->verify() failed" << std::endl;
    }
    const auto backInst = block->insts().back();
    if (auto brInst = backInst->dynCast<ir::BranchInst>()) {
      if (brInst->is_cond()) {
        // block -> block.iftrue / block.iffalse
        auto trueIter = std::find(func->blocks().begin(), func->blocks().end(), brInst->iftrue());
        auto falseIter = std::find(func->blocks().begin(), func->blocks().end(), brInst->iffalse());

        if (trueIter == func->blocks().end() and falseIter == func->blocks().end()) {
          std::cerr << "BranchInst's succ block not in function!" << std::endl;
          std::cerr << "BranchInst: ";
          brInst->print(std::cerr);
          std::cerr << std::endl;
        }
      } else {
        // block -> block.next
        auto nextIter = std::find(func->blocks().begin(), func->blocks().end(), brInst->dest());
        if (nextIter == func->blocks().end()) {
          std::cerr << "BranchInst's dest block not in function!" << std::endl;
          std::cerr << "BranchInst: ";
          brInst->print(std::cerr);
          std::cerr << std::endl;
        }
      }
    }
  }

  return isPass;
}

bool IRCheck::checkFuncInfo(ir::Function* func) {
  bool isPass = true;
  if (func->entry()->pre_blocks().size() != 0) {
    isPass = false;
    cerr << "Entry block got predecessors!" << endl;
  }
  if (func->exit()->next_blocks().size() != 0) {
    isPass = false;
    cerr << "Exit block got successors!" << endl;
  }
  return isPass;
}

bool IRCheck::checkAllocaOnlyInEntry(ir::Function* func) {
  bool isPass = true;

  for (auto bb : func->blocks()) {
    for (auto inst : bb->insts()) {
      if (inst->isa<ir::AllocaInst>() and bb != func->entry()) {
        isPass = false;
        cerr << "AllocaInst occur in BB:" << bb->name() << " but it's not entry block!" << endl;
      }
    }
  }
  return isPass;
}

bool IRCheck::checkOnlyOneExit(ir::Function* func) {
  bool isPass = true;
  int exitNum = 0;
  for (auto bb : func->blocks()) {
    for (auto inst : bb->insts()) {
      if (inst->isa<ir::ReturnInst>()) {
        if (bb != func->exit()) {
          std::cerr << "Funtion " << func->name() << " return inst not in function exit block"
                    << std::endl;
          std::cerr << "Return inst in block " << bb->name() << std::endl;
          std::cerr << "Function exit block is " << func->exit()->name() << std::endl;
          std::cerr << "return inst: ";
          inst->print(std::cerr);
          std::cerr << std::endl;
          isPass = false;
          return isPass;
        }
        exitNum++;
      }
    }
  }
  if (exitNum != 1) {
    std::cerr << "Funtion " << func->name() << " has " << exitNum << " exit inst" << std::endl;
    isPass = false;
  }
  return isPass;
}

bool IRCheck::checkParentRelationship(ir::Function* func) {
  bool isPass = true;
  for (auto block : func->blocks()) {
    if (block->function() != func) {
      std::cerr << "block father wrong!" << std::endl;
      std::cerr << "block: " << block->name()
                << ", block->function(): " << block->function()->name()
                << ", actual in function: " << func->name() << std::endl;
      isPass = false;
      assert(false);
    }
  }

  for (auto block : func->blocks()) {
    for (auto inst : block->insts()) {
      if (inst->block() != block) {
        std::cerr << "inst father wrong!" << std::endl;
        std::cerr << "inst: ";
        inst->print(std::cerr);
        std::cerr << std::endl;
        std::cerr << "inst->block(): " << inst->block()->name();
        std::cerr << ", actual in block: " << block->name() << std::endl;
        isPass = false;
        assert(false);
      }
    }
  }

  for (auto block : func->blocks()) {
    for (auto inst : block->insts()) {
      for (auto operandUse : inst->operands()) {
        auto operand = operandUse->value();
        if (operandUse->user() != inst) {
          std::cerr << "operand father wrong!" << std::endl;
          std::cerr << "operand: ";
          operand->dumpAsOpernd(std::cerr);
          std::cerr << std::endl;
          std::cerr << "operand->user(): ";
          operandUse->user()->dumpAsOpernd(std::cerr);
          isPass = false;
        }
        if (auto operandInst = operand->dynCast<Instruction>()) {
          if (operandInst->block()->function() != func) {
            std::cerr << "operand Inst not in same function!" << std::endl;
            std::cerr << "user inst: ";
            inst->print(std::cerr);
            std::cerr << std::endl;

            std::cerr << "operand Inst: ";
            operandInst->print(std::cerr);
            std::cerr << std::endl;
            std::cerr << "user inst in function: " << func->name();
            std::cerr << ", operand Inst->block()->function(): "
                      << operandInst->block()->function()->name() << std::endl;
            isPass = false;
          }
        }
      }
    }
  }

  return isPass;
}
bool IRCheck::checkOperands(ir::Function* func) {
  bool isPass = true;
  for (auto block : func->blocks()) {
    for (auto inst : block->insts()) {
      for (auto use : inst->operands()) {
        if (use == nullptr) {
          std::cerr << "Operand is null!" << std::endl;
          std::cerr << "block: " << block->name() << ", inst: ";
          inst->print(std::cerr);
          std::cerr << std::endl;
          isPass = false;
          assert(false);
        }
        if (use->value() == nullptr) {
          std::cerr << "Operand value is null!" << std::endl;
          std::cerr << "block: " << block->name() << ", inst: ";
          inst->print(std::cerr);
          std::cerr << std::endl;
          isPass = false;
          assert(false);
        }
        if (use->user() == nullptr) {
          std::cerr << "Operand user is null!" << std::endl;
          std::cerr << "block: " << block->name() << ", inst: ";
          inst->print(std::cerr);
          std::cerr << std::endl;
          isPass = false;
          assert(false);
        }
      }
    }
  }
  return isPass;
}
}  // namespace pass