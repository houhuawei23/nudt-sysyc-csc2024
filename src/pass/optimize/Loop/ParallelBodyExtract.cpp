// #define DEBUG
#include "pass/optimize/Loop/ParallelBodyExtract.hpp"
#include "pass/optimize/Loop/LoopBodyExtract.hpp"
#include "pass/analysis/ControlFlowGraph.hpp"
#include "pass/optimize/Utils/BlockUtils.hpp"

// #include "pass/analysis/dependenceAnalysis/dpaUtils.hpp"
#include "pass/analysis/dependenceAnalysis/DependenceAnalysis.hpp"
#include "pass/analysis/MarkParallel.hpp"
#include "pass/optimize/Loop/LoopUtils.hpp"

#include "support/arena.hpp"
#include "support/utils.hpp"

using namespace ir;

namespace pass {

static auto getUniqueID() {
  static size_t id = 0;
  const auto base = "sysyc_parallel_body";
  return base + std::to_string(id++);
}

static auto getStorageUniqueID() {
  static size_t id = 0;
  const auto base = "parallel_body_payload";
  return base + std::to_string(id++);
}
/*
after extract loop body:
  preheader -> header -> call_block -> latch -> exit
               header <--------------- latch
  call_block: call loop_body(i, otherargs...)

after extract parallel body:
  preheader -> call_block -> exit
  call parallel_body(beg, end)

  parallel_body(beg, end)
    for (i = beg; i < end; i++) {
      loop_body(i, otherargs...)
    }

*/
// build parallelBody function
/*
               |--> newExit
newEntry -> header -> call_loop_body -> latch
             |__________________________|

*/
auto buildParallelBodyBeta(Module& module,
                           IndVar* indVar,
                           LoopBodyInfo& loopBodyInfo,
                           ParallelBodyInfo& parallelBodyInfo /* ret */) {
  const auto i32 = Type::TypeInt32();
  auto funcType = FunctionType::gen(Type::void_type(), {i32, i32});
  auto parallelBody = module.addFunction(funcType, getUniqueID());
  parallelBody->attribute().addAttr(FunctionAttribute::ParallelBody);
  auto argBeg = parallelBody->new_arg(i32, "beg");
  auto argEnd = parallelBody->new_arg(i32, "end");

  auto newEntry = parallelBody->newEntry("new_entry");
  auto newExit = parallelBody->newExit("new_exit");
  std::unordered_set<BasicBlock*> bodyBlocks = {loopBodyInfo.header, loopBodyInfo.body,
                                                loopBodyInfo.latch};
  // add loop blocks to parallel_body
  for (auto block : bodyBlocks) {
    block->set_parent(parallelBody);
    parallelBody->blocks().push_back(block);
  }
  // build parallel_body
  IRBuilder builder;
  builder.set_pos(newEntry, newEntry->insts().end());
  // non constant, gloabal value used in loop_body, must pass by global payload
  // fix loop_body(i, otherargs...)
  // args must pass through global var, Value* -> offset
  std::vector<std::pair<Value*, size_t>> payload;
  size_t totalSize = 0;  // bytes
  const auto f32 = Type::TypeFloat32();

  std::unordered_set<Value*> inserted;
  // align by 32 bits, 4 bytes
  const size_t align = 4;
  Value* givOffset = nullptr;

  const auto addArgument = [&](Value* arg) {
    if (arg == loopBodyInfo.indVar->phiinst()) return;
    if (arg->isa<ConstantValue>() or arg->isa<GlobalVariable>()) return;
    // giv but not used by inner and outer
    if (arg == loopBodyInfo.giv and !loopBodyInfo.givUsedByOuter and !loopBodyInfo.givUsedByInner) {
      return;
    }
    if (inserted.count(arg)) return;  // already in
    // pass by payload
    const auto size = arg->type()->size();
    totalSize = utils::alignTo(totalSize, align);

    if (arg == loopBodyInfo.giv)
      givOffset = ConstantInteger::gen_i64(totalSize);
    else
      payload.emplace_back(arg, totalSize);  // arg -> offset

    totalSize += size;
  };

  for (auto use : loopBodyInfo.callInst->rargs()) {
    const auto realArg = use->value();
    addArgument(realArg);
  }
  // assert(totalSize % 4 == 0);  // by words
  if (totalSize % 4 != 0) {
    std::cerr << "totalSize not aligned by 4 bytes" << std::endl;
    assert(false);
  }
  const auto totalWords = totalSize / 4;
  const auto payloadType = ArrayType::gen(Type::TypeInt32(), {totalWords}, totalWords);  // by word?
  const auto payloadStorage = utils::make<GlobalVariable>(payloadType, getStorageUniqueID());
  module.addGlobalVar(payloadStorage->name(), payloadStorage);
  const auto payloadBase = builder.makeUnary(ValueId::vPTRTOINT, payloadStorage, Type::TypeInt64());

  // const auto giv = (loopBodyInfo.giv ? )
  // fix call loop_body(i, others)
  // original value -> load payload
  const auto remapArgument = [&](Use* use) {
    const auto user = use->user();
    const auto value = use->value();

    // dumpAsOperand(std::cerr << "remap: ", value);

    if (value == loopBodyInfo.indVar->phiinst()) return;
    // giv
    if (value == loopBodyInfo.giv) {
      return;
    }
    if (value->isa<ConstantValue>() or value->isa<GlobalVariable>()) return;

    bool replaced = false;
    for (auto [arg, offset] : payload) {
      if (arg == value) {
        auto ptr = builder.makeBinary(BinaryOp::ADD, payloadBase, ConstantInteger::gen_i64(offset));
        ptr = builder.makeUnary(ValueId::vINTTOPTR, ptr, Type::TypePointer(arg->type()));
        auto load = builder.makeLoad(ptr);
        user->setOperand(use->index(), load);
        replaced = true;
      }
    }
    if (not replaced) {
      std::cerr << "arg not found in payload" << std::endl;
      assert(false);
    }
    // dumpAsOperand(std::cerr << "-> ", user->operand(use->index()));
  };
  auto uses = loopBodyInfo.callInst->rargs();
  for (auto use : uses) {
    remapArgument(use);
  }
  // nextans = ans + xx, loopinit is 0
  const auto givLoopInit = ConstantInteger::gen_i32(0);

  // fix value in paraplel_body
  const auto fixPhi = [&](PhiInst* phi) {
    // std::cerr << "fix phi inst: ";
    // phi->dumpAsOpernd(std::cerr);
    // std::cerr << std::endl;
    if (phi == indVar->phiinst()) {
      phi->delBlock(loopBodyInfo.preHeader);
      phi->addIncoming(argBeg, newEntry);
      return;
    } else if (phi == loopBodyInfo.giv) {
      phi->delBlock(loopBodyInfo.preHeader);
      phi->addIncoming(givLoopInit, newEntry);
      return;
    }
    // std::cerr << "phi inst not indvar phi inst" << std::endl;
    // phi->print(std::cerr);
    // std::cerr << std::endl;
  };

  // fix cmp inst
  const auto fixCmp = [&](ICmpInst* cmpInst) {
    if (cmpInst == indVar->cmpInst()) {
      for (auto opuse : cmpInst->operands()) {
        auto op = opuse->value();
        if (op == indVar->endValue()) {
          cmpInst->setOperand(opuse->index(), argEnd);
          break;
        }
      }
    }
    // std::cerr << "cmp inst not indvar cmp inst" << std::endl;
    // cmpInst->print(std::cerr);
    // std::cerr << std::endl;
  };
  std::unordered_map<BasicBlock*, BasicBlock*> blockMap;
  blockMap.emplace(loopBodyInfo.exit, newExit);
  const auto fixBranch = [&](BranchInst* branch) {
    for (auto opuse : branch->operands()) {
      auto op = opuse->value();
      if (auto block = op->dynCast<BasicBlock>()) {
        if (auto pair = blockMap.find(block); pair != blockMap.end()) {
          branch->setOperand(opuse->index(), pair->second);
        }
      }
    }
  };
  const auto fixCall = [&](CallInst* call) {
    if (call == loopBodyInfo.callInst) {  // call loop_body(i, otherargs...)
      for (auto opuse : call->operands()) {
        auto op = opuse->value();
        // TODO:
      }
    }
  };
  for (auto block : parallelBody->blocks()) {
    // std::cerr << "block: " << block->name() << std::endl;
    for (auto inst : block->insts()) {
      // inst->print(std::cerr);
      // std::cerr << std::endl;
      if (auto phi = inst->dynCast<PhiInst>()) {
        fixPhi(phi);
      } else if (auto cmpInst = inst->dynCast<ICmpInst>()) {
        fixCmp(cmpInst);
      } else if (auto branch = inst->dynCast<BranchInst>()) {
        fixBranch(branch);
      } else if (auto call = inst->dynCast<CallInst>()) {
        fixCall(call);
      }
    }
  }

  builder.makeInst<BranchInst>(loopBodyInfo.header);  // newEntry -> header
  fixAllocaInEntry(*parallelBody);

  // newExit -> store giv to payload, return
  builder.set_pos(newExit, newExit->insts().end());
  if (loopBodyInfo.giv) {
    auto ptr = builder.makeBinary(BinaryOp::ADD, payloadBase, givOffset);
    ptr = builder.makeUnary(ValueId::vINTTOPTR, ptr, Type::TypePointer(loopBodyInfo.giv->type()));
    // builder.makeInst<StoreInst>(loopBodyInfo.giv, ptr);
    builder.makeInst<AtomicrmwInst>(BinaryOp::ADD, ptr, loopBodyInfo.giv);
  }
  builder.makeInst<ReturnInst>();  // newExit [ret void]

  parallelBodyInfo.parallelBody = parallelBody;
  parallelBodyInfo.payload = payload;
  parallelBodyInfo.payloadStorage = payloadStorage;
  parallelBodyInfo.givOffset = givOffset;
  return parallelBody;
}

auto rebuildFunc(Function* func,
                 IndVar* indVar,
                 LoopBodyInfo& loopBodyInfo,
                 ParallelBodyInfo& parallelBodyInfo) {
  std::unordered_set<BasicBlock*> bodyBlocks = {loopBodyInfo.header, loopBodyInfo.body,
                                                loopBodyInfo.latch};
  func->blocks().remove_if([&](BasicBlock* block) { return bodyBlocks.count(block); });
  IRBuilder builder;
  // add new call block to func
  // preHeader -> callBlock -> exit
  auto callBlock = func->newBlock();
  callBlock->setComment("call_parallel_body");
  loopBodyInfo.preHeader->insts().pop_back();  // remove old br from preHeader
  builder.set_pos(loopBodyInfo.preHeader, loopBodyInfo.preHeader->insts().end());
  builder.makeInst<BranchInst>(callBlock);  // preHeader -> callBlock

  // callBlock
  builder.set_pos(callBlock, callBlock->insts().end());
  // store payload
  const auto base =
    builder.makeUnary(ValueId::vPTRTOINT, parallelBodyInfo.payloadStorage, Type::TypeInt64());
  parallelBodyInfo.payloadStoreInsts.emplace_back(base);

  for (auto [value, offset] : parallelBodyInfo.payload) {
    auto ptr = builder.makeBinary(BinaryOp::ADD, base, ConstantInteger::gen_i64(offset));
    auto typeptr = builder.makeUnary(ValueId::vINTTOPTR, ptr, Type::TypePointer(value->type()));
    auto store = builder.makeInst<StoreInst>(value, typeptr);
    parallelBodyInfo.payloadStoreInsts.insert(parallelBodyInfo.payloadStoreInsts.end(),
                                              {ptr, typeptr, store});
  }

  // call parallel_body(beg, end)
  auto callArgs = std::vector<Value*>{indVar->beginValue(), indVar->endValue()};
  auto callInst = builder.makeInst<CallInst>(parallelBodyInfo.parallelBody, callArgs);

  // builder.set_pos(loopBodyInfo.exit, loopBodyInfo.exit->insts().begin());
  // giv
  Value* newGiv = nullptr;
  if (loopBodyInfo.giv) {
    const auto payloadBase =
      builder.makeUnary(ValueId::vPTRTOINT, parallelBodyInfo.payloadStorage, Type::TypeInt64());
    auto ptr = builder.makeBinary(BinaryOp::ADD, payloadBase, parallelBodyInfo.givOffset);
    ptr = builder.makeUnary(ValueId::vINTTOPTR, ptr, Type::TypePointer(loopBodyInfo.giv->type()));
    newGiv = builder.makeLoad(ptr);
  }

  // fix outuse of inner loop var: giv
  for (auto block : func->blocks()) {
    for (auto inst : block->insts()) {
      auto operands = inst->operands();
      for (auto operandUse : operands) {
        // if (operandUse->value() == indVar->phiinst()) {
        //   inst->setOperand(operandUse->index(), indVar->endValue());
        // }
        if (operandUse->value() == loopBodyInfo.giv) {
          inst->setOperand(operandUse->index(), newGiv);
        }
      }
    }
  }
  builder.makeInst<BranchInst>(loopBodyInfo.exit);

  parallelBodyInfo.callInst = callInst;
  parallelBodyInfo.callBlock = callBlock;
}

bool extractParallelBody(Function* func,
                         Loop* loop,
                         IndVar* indVar,
                         TopAnalysisInfoManager* tp,
                         ParallelBodyInfo& parallelBodyInfo) {
  if (not indVar->stepValue()->isa<ConstantValue>()) {
    return false;
  }
  const auto step = indVar->stepValue()->dynCast<ConstantValue>()->i32();
  // indVar->print(std::cerr);
  if (step != 1) return false;                  // only support step = 1
  if (loop->exits().size() != 1) return false;  // only support single exit loop

  // extact loop body as a new loop_body func from func loop
  LoopBodyInfo loopBodyInfo;
  if (not extractLoopBody(func, loop /* modified */, indVar, tp, loopBodyInfo /* ret */))
    return false;
  // loopBodyInfo.print(std::cerr);
  // func->rename();
  // func->print(std::cerr);
  // const auto loopBodyFunc = loopBodyInfo.callInst->callee();
  // loopBodyFunc->rename();
  // loopBodyFunc->print(std::cerr);
  // if(loopBodyInfo.giv) {
  //   loopBodyInfo.giv->delBlock(loopBodyInfo.)
  // }

  const auto parallelBody =
    buildParallelBodyBeta(*func->module(), indVar, loopBodyInfo, parallelBodyInfo);

  rebuildFunc(func, indVar, loopBodyInfo, parallelBodyInfo);
  const auto fixFunction = [&](Function* function) {
    CFGAnalysisHHW().run(function, tp);
    blockSortDFS(*function, tp);
    // function->rename();
    // function->print(std::cerr);
  };
  // fic function
  fixFunction(func);
  fixFunction(parallelBody);

  // parallelBodyInfo
  parallelBodyInfo.beg = indVar->beginValue();
  parallelBodyInfo.end = indVar->endValue();
  return true;
}

void ParallelBodyExtract::run(Function* func, TopAnalysisInfoManager* tp) {
  runImpl(func, tp);
}

bool ParallelBodyExtract::runImpl(Function* func, TopAnalysisInfoManager* tp) {
  // func->rename();
  // std::cerr << "!! ParallelBodyExtract::runImpl: " << func->name() << std::endl;
  // func->print(std::cerr);

  CFGAnalysisHHW().run(func, tp);  // refresh CFG
  MarkParallel().run(func, tp);

  bool modified = false;

  auto lpctx = tp->getLoopInfoWithoutRefresh(func);        // fisrt loop analysis
  auto indVarctx = tp->getIndVarInfoWithoutRefresh(func);  // then indvar analysis
  auto parallelctx = tp->getParallelInfo(func);

  auto loops = lpctx->sortedLoops();

  std::unordered_set<Loop*> extractedLoops;
  for (auto loop : loops) {
    // std::cerr << "loop level: " << lpctx->looplevel(loop->header());
    // loop->print(std::cerr);

    if (not checkLoopParallel(loop, lpctx, indVarctx, parallelctx, extractedLoops)) continue;

    const auto indVar = indVarctx->getIndvar(loop);
    ParallelBodyInfo info;
    if (not extractParallelBody(func, loop, indVar, tp, info)) {
      // std::cerr << "failed to extract parallel body for loop" << std::endl;
      continue;
    }
    modified = true;
    extractedLoops.insert(loop);
  }
  return modified;
}

}  // namespace pass