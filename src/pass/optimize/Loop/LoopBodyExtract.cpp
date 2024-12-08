
// #define DEBUG
#include "ir/ir.hpp"
#include "pass/pass.hpp"
#include "pass/analysis/ControlFlowGraph.hpp"
#include "pass/analysis/indvar.hpp"

#include "pass/optimize/Loop/LoopBodyExtract.hpp"
#include "pass/optimize/Utils/BlockUtils.hpp"
#include "pass/analysis/MarkParallel.hpp"
#include "pass/optimize/Loop/LoopUtils.hpp"

#include "pass/optimize/Utils/PatternMatch.hpp"

#include <cassert>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <algorithm>  // For std::erase_if(but c++20)
using namespace ir;
namespace pass {

void LoopBodyInfo::print(std::ostream& os) const {
  os << "LoopBodyInfo: " << std::endl;
  std::cout << "callInst: ";
  callInst->print(os);
  os << std::endl;
  std::cout << "indVar: ";
  indVar->print(os);
  std::cout << std::endl;
  std::cout << "preHeader: ";
  preHeader->dumpAsOpernd(os);
  os << std::endl;
  std::cout << "header: ";
  header->dumpAsOpernd(os);
  os << std::endl;
  std::cout << "body: ";
  body->dumpAsOpernd(os);
  os << std::endl;
  std::cout << "latch: ";
  latch->dumpAsOpernd(os);
  os << std::endl;

  //   PhiInst* giv;
  // bool givUsedByOuter;
  // bool givUsedByInner;
  if (giv) {
    os << "giv: ";
    giv->print(os);
    os << std::endl;
    os << "givUsedByOuter: " << givUsedByOuter << std::endl;
    os << "givUsedByInner: " << givUsedByInner << std::endl;
  }
}
void LoopBodyExtract::run(Function* func, TopAnalysisInfoManager* tp) {
  runImpl(func, tp);
}

bool hasCall(Loop* loop) {
  for (auto block : loop->blocks()) {
    for (auto inst : block->insts()) {
      if (auto call = inst->dynCast<CallInst>()) {
        return true;
      }
    }
  }
  return false;
}

bool LoopBodyExtract::runImpl(Function* func, TopAnalysisInfoManager* tp) {
  auto sideEffectInfo = tp->getSideEffectInfo();
  CFGAnalysisHHW().run(func, tp);  // refresh CFG
  MarkParallel().run(func, tp);

  auto lpctx = tp->getLoopInfoWithoutRefresh(func);        // fisrt loop analysis
  auto indVarctx = tp->getIndVarInfoWithoutRefresh(func);  // then indvar analysis
  auto parallelctx = tp->getParallelInfo(func);

  bool modified = false;

  auto loops = lpctx->sortedLoops();

  std::unordered_set<Loop*> extractedLoops;

  for (auto loop : loops) {
    // std::cerr << "loop level: " << lpctx->looplevel(loop->header());
    // loop->print(std::cerr);

    if (not checkLoopParallel(loop, lpctx, indVarctx, parallelctx, extractedLoops)) continue;

    const auto indVar = indVarctx->getIndvar(loop);
    LoopBodyInfo loopBodyInfo;
    if (not extractLoopBody(func, loop, indVar, tp, loopBodyInfo /* ret */)) continue;
    modified = true;
    extractedLoops.insert(loop);
#ifdef DEBUG
    std::cerr << "extracted loop body: " << loopBodyInfo.callInst->callee()->name() << std::endl;
#endif
    // break;
  }
  tp->CallChange();
  tp->CFGChange(func);
  tp->IndVarChange(func);
  // fix cfg
  CFGAnalysisHHW().run(func, tp);  // refresh CFG
  return modified;
}

/**
      other
        |
        v
  |-> loop header --> loop next
  |     |
  |     v
  |   loop body
  |     |
  |     v
  --- loop latch

- loop->header:
  - phi = phi [v1, other], [i.next, loop->latch] ; phi inst (for indvar),
  - cond = imcp op phi, endVar
  - br cond, loop->body, loop->next

- loop->body:
  - real body of the loop

- loop->latch:
  - i.next = i + step
  - br loop->header

==> after extractLoopBody:

      other
        |
        v
  --> loop header --> loop->next
  |     |
  |     v
  |  callBlock
  |     |
  |     v
  --  loop latch


newLoop:
  - i = phi [i0, other], [i.next, newLoop]
  -
 */
// need iterInst in loop->latch
static auto getUniqueID() {
  static size_t id = 0;
  const auto base = "sysyc_loop_body";
  return base + std::to_string(id++);
}

#include "pass/optimize/Utils/PatternMatch.hpp"

static size_t count;

static bool matchAddRec(Value* giv, BasicBlock* latch, std::unordered_set<Value*>& values) {
  // count++;
  // if (count > 10)  {
  //   std::cerr << "matchAddRec: too many iterations" << std::endl;
  //   assert(false);
  // }
  if (not giv->isa<PhiInst>()) return false;
  if (not giv->dynCast<PhiInst>()->incomings().count(latch)) return false;
  const auto givNext = giv->dynCast<PhiInst>()->getvalfromBB(latch);
  // dumpAsOperand(std::cerr << "givNext: ", givNext);
  if (givNext->isa<Instruction>()) {
    Value* v2;
    if (givNext->isa<PhiInst>()) {
      for (auto& [pre, val] : givNext->dynCast<PhiInst>()->incomings()) {
        if (val == giv) continue;
        PhiInst* base;
        // val = phi + v2
        if (!add(phi(base), any(v2))(MatchContext<Value>{val})) {
          return false;
        }
        // recursive match base
        if (!matchAddRec(base, pre, values)) return false;
        values.insert(val);
        values.insert(base);
      }
      values.insert(giv);
      values.insert(givNext);
      return true;
    }

    if (add(exactly(giv), any(v2))(MatchContext<Value>{givNext})) {
      values.insert(giv);
      values.insert(givNext);
      return true;
    }

    // if()
  }

  return false;
}

bool checkAndFixLoopLatch(Function* func,
                          Loop* loop,
                          IndVar* indVar,
                          TopAnalysisInfoManager* tp,
                          BasicBlock*& oldLatch) {
  if (func->attribute().hasAttr(FunctionAttribute::LoopBody | FunctionAttribute::ParallelBody)) {
    return false;
  }

  if (loop->latchs().size() != 1) return false;
  // header == latch, no loop body
  // only support loop with one exit
  if (loop->header() == loop->getUniqueLatch() and loop->exits().size() != 1) {
    return false;
  }
  // make sure loop is correct
  oldLatch = loop->getUniqueLatch();

  if (not fixLoopLatch(func, loop, indVar, tp)) return false;

  // only support 2 phi insts: 1 for indvar, 1 for giv
  // biv: basic indvar
  // giv: general indvar (and = ans + 1)
  // return next giv as loop_body return value
  size_t phiCount = 0;
  for (auto inst : loop->header()->insts()) {
    if (inst->isa<PhiInst>()) {
      phiCount++;
    }
  }
  if (phiCount > 2) return false;

  for (auto block : loop->blocks()) {
    if (block == loop->header()) continue;
    for (auto next : block->next_blocks()) {
      if (not loop->contains(next)) {
        // std::cerr << block->name() << "->" << next->name() << " is not in loop" << std::endl;
        return false;
      }
    }
  }
  return true;
}

auto checkGivUsedByOuter(Value* GIndVar, Loop* loop) {
  if (not GIndVar) return false;
  // dumpAsOperand(std::cerr << "checkGivUsedByOuter: ", GIndVar);

  for (auto inst : {GIndVar, GIndVar->dynCast<PhiInst>()->getvalfromBB(loop->getUniqueLatch())}) {
    // dumpInst(std::cerr << "inst: ", inst->dynCast<Instruction>());
    if (not inst->isa<Instruction>()) continue;
    for (auto userUse : inst->uses()) {
      auto userInst = userUse->user()->dynCast<Instruction>();
      // dumpInst(std::cerr << "userInst: ", userInst);
      if (not loop->blocks().count(userInst->block())) {
        return true;
      }
    }
  }
  return false;
};

const auto checkGivUsedByOtherInner(Value* GIndVar, Loop* loop, bool& error) {
  if (not GIndVar) return false;
  // dumpAsOperand(std::cerr << "checkGivUsedByOtherInner: ", GIndVar);

  bool usedByOtherInner = false;
  std::unordered_set<Value*> values;
  if (!matchAddRec(GIndVar, loop->getUniqueLatch(), values)) {
    error = true;
    return true;
  }
  for (auto inst : values) {
    // dumpInst(std::cerr << "inst: ", inst->dynCast<Instruction>());
    for (auto userUse : inst->uses()) {
      auto user = userUse->user();
      // dumpInst(std::cerr << "user: ", user->dynCast<Instruction>());
      if (not user->isa<Instruction>()) return false;
      if (!values.count(user) and loop->blocks().count(user->dynCast<Instruction>()->block())) {
        usedByOtherInner = true;
        return usedByOtherInner;
      }
    }
  }
  return usedByOtherInner;
};

Value* getGeneralIndVar(Loop* loop, IndVar* indVar) {
  for (auto inst : loop->header()->insts())
    if (inst->isa<PhiInst>() and inst != indVar->phiinst())
      return inst;
    else
      continue;
  return nullptr;
}

auto convertToAtomic(Loop* loop) {
  // load-store pair to atomic add
  std::unordered_map<Value*, uint32_t> loadStoreMap;
  for (auto block : loop->blocks()) {
    for (auto inst : block->insts()) {
      if (inst->isTerminator()) continue;
      if (auto loadInst = inst->dynCast<LoadInst>()) {
        const auto ptr = loadInst->ptr();
      } else if (auto storeInst = inst->dynCast<StoreInst>()) {
        const auto ptr = storeInst->ptr();
      }
      // TODO:
    }
  }
  std::vector<std::pair<Instruction*, Instruction*>> workList;
  for (auto [k, v] : loadStoreMap) {
    if (v == 3) {
      // TODO:
    }
  }
};

static bool buildBodyFunc(Function* func,
                          Loop* loop,
                          IndVar* indVar,
                          Value* giv,
                          TopAnalysisInfoManager* tp,
                          Function*& bodyFunc,
                          std::unordered_map<Value*, Value*>& arg2val) {
  auto funcType = FunctionType::gen(giv ? giv->type() : Type::void_type(), {});

  bodyFunc = func->module()->addFunction(funcType, getUniqueID());
  bodyFunc->attribute().addAttr(FunctionAttribute::LoopBody);

  // some operand used in loop must be passed by function arg, add to val2arg
  std::unordered_map<Value*, Value*> val2arg;

  // indvar phi -> body func first arg
  val2arg.emplace(indVar->phiinst(), bodyFunc->new_arg(indVar->phiinst()->type(), "indvar_arg"));

  // if giv, giv -> body func second arg
  if (giv) val2arg.emplace(giv, bodyFunc->new_arg(giv->type()));

  // duplicate cmp, true
  // if cmp cond not generated by loop body, duplicate it in loop body
  for (auto block : loop->blocks()) {
    auto branchInst = block->terminator()->dynCast<BranchInst>();
    if (!branchInst) {
      std::cerr << "loop body has no branch inst" << std::endl;
      assert(branchInst);
    }
    if (not branchInst->is_cond()) continue;

    auto cond = branchInst->cond()->dynCast<Instruction>();
    // if cond is in loop, skip
    if (loop->blocks().count(cond->block())) continue;
    // if not, duplicate it in loop body
    std::cerr << "cond inst is not in loop blocks, need to duplicate it in loop body!" << std::endl;
    assert(false);
    return false;
  }

  const auto mapOperandInLoopToLoopBodyArguemnt = [&] {
    // std::cerr << "for all operands in loop, add to val2arg or pass by function arg" << std::endl;
    for (auto block : loop->blocks()) {
      // dumpAsOperand(std::cerr << "block: ", block);
      if (block == loop->header() or block == loop->getUniqueLatch()) continue;
      for (auto inst : block->insts()) {
        // dumpInst(std::cerr << "inst: ", inst);
        for (auto opuse : inst->operands()) {
          auto op = opuse->value();
          // dumpAsOperand(std::cerr << "op: ", op);
          if (op->type()->isLabel()) continue;  // block label
          if (val2arg.count(op)) continue;      // already mapped
          if (op->dynCast<ConstantValue>() or op->dynCast<GlobalVariable>()) {
            continue;  // constants and global variables can be used directly
          }
          if (auto opInst = op->dynCast<Instruction>()) {
            if (loop->blocks().count(opInst->block())) continue;
          }
          // else, this op must pass by function arg, add to val2arg
          val2arg.emplace(op, bodyFunc->new_arg(op->type(), op->name() + "_arg"));
        }
      }
    }
  };

  mapOperandInLoopToLoopBodyArguemnt();

#ifdef DEBUG
  for (auto [val, arg] : val2arg) {
    std::cerr << "val: ";
    val->dumpAsOpernd(std::cerr);
    std::cerr << " -> ";
    std::cerr << "arg: ";
    arg->dumpAsOpernd(std::cerr);
    std::cerr << std::endl;
  }
#endif

  bodyFunc->updateTypeFromArgs();

  // std::unordered_map<Value*, Value*> arg2val;

  // replace operands used in loop with corresponding args
  // update use
  const auto replaceOperandInLoopWithArg = [&] {
    // std::cerr << "replace operands used in loop with corresponding args" << std::endl;
    for (auto [val, arg] : val2arg) {
      arg2val.emplace(arg, val);
      auto uses = val->uses();  // avoid invalidating use iterator
#ifdef DEBUG
      std::cerr << "val: ";
      val->dumpAsOpernd(std::cerr);
      std::cerr << ", with uses size: " << uses.size() << std::endl;
#endif
      for (auto use : uses) {
        const auto userInst = use->user()->dynCast<Instruction>();
        // dumpInst(std::cerr << "userInst: ", userInst);
        // exclude head and iterInst
        if (userInst->block() == loop->header() or userInst->block() == loop->getUniqueLatch())
          continue;
        if (userInst == indVar->iterInst()) continue;
        if (loop->blocks().count(userInst->block())) {
#ifdef DEBUG
          std::cerr << "replace operand " << val->name() << " with arg " << arg->name()
                    << std::endl;
#endif
          // userInst is in loop, replace operand with arg
          const auto idx = use->index();
          userInst->setOperand(idx, arg);
        }
        // std::cerr << "after replace, val: " << val->uses().size() << std::endl;
      }
    }
  };
  replaceOperandInLoopWithArg();

  // construct bodyFunc blocks
  // push header.next as loop_body's entry
  std::unordered_set<BasicBlock*> removeWorkList;
  const auto bodyEntry = loop->getFirstBodyBlock();
  bodyFunc->setEntry(bodyEntry);
  bodyFunc->blocks().push_back(bodyEntry);
  removeWorkList.insert(bodyEntry);

  // other blocks in loop
  for (auto block : loop->blocks()) {
    // exclue head and latch
    if (block == loop->header() or block == loop->getUniqueLatch()) continue;
    if (block != bodyFunc->entry()) {
      block->set_parent(bodyFunc);
      bodyFunc->blocks().push_back(block);
      removeWorkList.insert(block);
    }
  }
  if (bodyFunc->blocks().size() != (loop->blocks().size() - 2)) {
    std::cerr << "bodyFunc has wrong number of blocks" << std::endl;
    assert(false);
  }
  // remove loop blocks from func
  func->blocks().remove_if([&](BasicBlock* block) { return removeWorkList.count(block); });
  // FIXME: update loop itself? dont do because loop->blocks() is used by next iter
  // remove loop body blocks from loop

  IRBuilder builder;
  // oldLatch now is the new loop_body's exit
  // assert(loop->getUniqueLatch()->pre_blocks().size() == 1);
  if (loop->getUniqueLatch()->pre_blocks().size() != 1) {
    std::cerr << "loop latch has more than one pre block" << std::endl;
    assert(false);
  }
  const auto oldLatch = loop->getUniqueLatch()->pre_blocks().front();
  // assert(oldLatch->terminator()->isa<BranchInst>());
  if (not oldLatch->terminator()->isa<BranchInst>()) {
    std::cerr << "loop latch has no branch inst" << std::endl;
    assert(false);
  }
  oldLatch->insts().pop_back();
  // just return, caller will call next iter
  builder.set_pos(oldLatch, oldLatch->insts().end());
  // return next giv
  if (giv)
    builder.makeInst<ReturnInst>(giv->dynCast<PhiInst>()->getvalfromBB(loop->getUniqueLatch()));
  else
    builder.makeInst<ReturnInst>();

  bodyFunc->setExit(oldLatch);

  return true;
}

static bool rebuildFunc(Function* func,
                        Loop* loop,
                        Function* bodyFunc,
                        Value* giv,
                        const std::unordered_map<Value*, Value*>& arg2val,
                        LoopBodyInfo& info) {
  // header -> callBlock -> latch
  // realArgs used by call inst
  std::vector<Value*> RealArgs4Call;
  for (auto arg : bodyFunc->args()) {
    RealArgs4Call.push_back(arg2val.at(arg));
  }

  IRBuilder builder;
  // fix branch relation
  auto callBlock = func->newBlock();
  // loop->blocks().insert(callBlock);
  auto headerBranch = loop->header()->terminator()->dynCast<BranchInst>();
  {
    // assert(loop->contains(headerBranch->iftrue()));  // true is jump in loop
    // or:
    const auto iter =
      std::find(bodyFunc->blocks().begin(), bodyFunc->blocks().end(), headerBranch->iftrue());
    // assert(iter != bodyFunc->blocks().end());
    if (iter == bodyFunc->blocks().end()) {
      std::cerr << "headerBranch->iftrue() not in bodyFunc->blocks()" << std::endl;
      assert(false);
    }
  }
  headerBranch->set_iftrue(callBlock);

  // buid callBlock: call loop_body + jump to newlatch
  builder.set_pos(callBlock, callBlock->insts().end());
  const auto callInst = builder.makeInst<CallInst>(bodyFunc, RealArgs4Call);
  builder.makeInst<BranchInst>(loop->getUniqueLatch());

  // fix phiGiv
  if (giv) {
    auto phiGiv = giv->dynCast<PhiInst>();
    // phiGiv->print(std::cerr);
    // std::cerr << std::endl;
    // assert(phiGiv->incomings().count(loop->getUniqueLatch()));
    if (not phiGiv->incomings().count(loop->getUniqueLatch())) {
      std::cerr << "giv not incomings with loop latch" << std::endl;
      assert(false);
    }
    phiGiv->delBlock(loop->getUniqueLatch());

    phiGiv->addIncoming(callInst, loop->getUniqueLatch());
  }
  // LoopBodyInfo
  info.callInst = callInst;
  info.body = callBlock;
  return true;
}

static bool checkInstUsedByOuter(Function* func,
                                 Loop* loop,
                                 IndVar* indVar,
                                 Value* giv,
                                 std::unordered_set<Value*>& allowedToBeUsedByOuter) {
  // only indvar, next indvar, giv, next giv, allowed to be used by outer
  // other inst in loop should not be used by outer
  // std::unordered_set<Value*> allowedToBeUsedByOuter;

  // allowedToBeUsedByOuter.insert(indVar->phiinst());
  // allowedToBeUsedByOuter.insert(indVar->iterInst());
  if (giv) {
    allowedToBeUsedByOuter.insert(giv);
    // next_giv
    allowedToBeUsedByOuter.insert(giv->dynCast<PhiInst>()->getvalfromBB(loop->getUniqueLatch()));
  }

  for (auto block : loop->blocks()) {
    for (auto inst : block->insts()) {
      if (allowedToBeUsedByOuter.count(inst)) continue;
      for (auto user_use : inst->uses()) {
        auto userInst = user_use->user()->dynCast<Instruction>();
        // used in loop: header - body - latch
        if (loop->blocks().count(userInst->block())) {
          continue;
        }
        // else
        // std::cerr << "inst: ";
        // inst->print(std::cerr);
        // std::cerr << std::endl;
        // std::cerr << "userInst: ";
        // userInst->print(std::cerr);
        // std::cerr << std::endl;
        // std::cerr << "userInst block: ";
        // std::cerr << userInst->block()->name() << std::endl;
        return false;
      }
    }
  }
  return true;
}

bool extractLoopBody(Function* func,
                     Loop* loop,
                     IndVar* indVar,
                     TopAnalysisInfoManager* tp,
                     LoopBodyInfo& info) {
#ifdef DEBUG
  std::cerr << "extract loop body for: " << func->name() << std::endl;
  func->rename();
  func->print(std::cerr);
  loop->print(std::cerr);
#endif
  BasicBlock* oldLatch = nullptr;
  if (not checkAndFixLoopLatch(func, loop, indVar, tp, oldLatch)) return false;

  // first phi inst != loop->inductionVar, giv = that phi inst
  // global induction var, such as n
  Value* giv = getGeneralIndVar(loop, indVar);

  bool givUsedByOuter = checkGivUsedByOuter(giv, loop);
  Value* givAddRecInnerStep = nullptr;

  bool error = false;
  bool givUsedByOtherInner = checkGivUsedByOtherInner(giv, loop, error);
  if (error) return false;
  if (givUsedByOtherInner and givUsedByOuter) return false;
  std::unordered_set<Value*> allowedToBeUsedByOuter;
  if (not checkInstUsedByOuter(func, loop, indVar, giv, allowedToBeUsedByOuter /* ret */))
    return false;

  // independent
  std::unordered_map<Value*, Value*> arg2val;
  Function* bodyFunc = nullptr;

  if (not buildBodyFunc(func, loop, indVar, giv, tp, bodyFunc /* ret */, arg2val /* ret */))
    return false;

  if (not rebuildFunc(func, loop, bodyFunc, giv, arg2val, info)) {
    return false;
  }

  // fix constraints on entry and exit
  fixAllocaInEntry(*bodyFunc);

  // fix cfg
  const auto fixFunction = [&](Function* function) {
    CFGAnalysisHHW().run(function, tp);
    blockSortDFS(*function, tp);
    // function->rename();
    // function->print(std::cerr);
  };

  // std::cerr << "after extractLoopBody, func: " << func->name() << std::endl;
  fixFunction(func);
  fixFunction(bodyFunc);

  {
    // return LoopBodyInfo
    info.indVar = indVar;
    info.header = loop->header();
    info.latch = loop->getUniqueLatch();
    info.preHeader = loop->getLoopPreheader();
    info.exit = *(loop->exits().begin());

    // giv
    info.giv = giv->dynCast<PhiInst>();
    info.givUsedByOuter = givUsedByOuter;
    info.givUsedByInner = givUsedByOtherInner;
  }

  tp->CallChange();
  tp->CFGChange(func);
  tp->IndVarChange(func);
  return true;
}

}  // namespace pass