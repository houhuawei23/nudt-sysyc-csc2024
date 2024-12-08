// #define DEBUG
#include "pass/optimize/Utils/BlockUtils.hpp"
#include "pass/optimize/Misc/StatelessCache.hpp"
#include "pass/analysis/ControlFlowGraph.hpp"

using namespace ir;

namespace pass {
Function* StatelessCache::getLookupFunction(Module* module,
                                            ArrayType* entryType,
                                            ArrayType* lutType) {
  const auto funcName = "sysycCacheLookup";
  if (auto func = module->findFunction(funcName)) {
    return func;
  }
  const auto i32 = Type::TypeInt32();

  const auto funcType =
    FunctionType::gen(PointerType::gen(entryType), {PointerType::gen(lutType), i32, i32});
  const auto func = module->addFunction(funcType, funcName);
  func->attribute().addAttr(FunctionAttribute::Builtin);
  return func;
}
bool StatelessCache::has2MoreRecursiveCalls(Function* func) {
  size_t count = 0;
  for (auto block : func->blocks()) {
    for (auto inst : block->insts()) {
      if (auto call = inst->dynCast<CallInst>()) {
        if (call->callee() == func) {
          count++;
        } else {
          return false;
        }
      }
    }  // for each instruction in block
  }  // for each block
  return count >= 2;
}

bool checkArgs(Function* func) {
  if (func->args().empty() or func->args().size() > 2) return false;
  for (auto argType : func->argTypes()) {
    if (not(argType->isInt32() or argType->isFloat32())) return false;
  }
  return true;
}


bool StatelessCache::runImpl(ir::Function* func, TopAnalysisInfoManager* tp) {
  auto sideEffectInfo = tp->getSideEffectInfo();
  if (sideEffectInfo->hasSideEffect(func)) {
#ifdef DEBUG
    std::cerr << "StatelessCache: " << func->name() << " has side effect" << std::endl;
#endif
    return false;
  }

#ifdef DEBUG
    std::cerr << "StatelessCache: " << func->name() << std::endl;
    // func->print(std::cerr);
#endif

  // check if function is Stateless: NoMemoryRead, NoSideEffect
  // check if function has 2 more recursive calls
  if (not has2MoreRecursiveCalls(func)) return false;
  const auto i32 = Type::TypeInt32();
  const auto f32 = Type::TypeFloat32();
  const auto retType = func->retType();

  // only support i32 and f32 return types
  if (not(retType->isInt32() or retType->isFloat32())) return false;

  if (not checkArgs(func)) return false;

  /* split entry block to: alloca block, eval block */
  const auto entry = func->entry();
  const auto branchInst = entry->terminator()->as<BranchInst>();

  BasicBlock* evalBloack = nullptr;
  for (auto iter = entry->insts().begin(); iter != entry->insts().end(); iter++) {
    const auto inst = *iter;
    if (not inst->isa<AllocaInst>()) {
      evalBloack = splitBlock(func->blocks(), func->blocks().begin(), std::prev(iter));
      break;
    }
  }
  // fix phi inst: replace incoming block from entry to evalBlock
  std::vector<BasicBlock*> workList;
  if (branchInst->is_cond()) {
    workList.push_back(branchInst->iftrue());
    workList.push_back(branchInst->iffalse());
  } else {
    workList.push_back(branchInst->dest());
  }
  for (auto block : workList) {
    for (auto inst : block->insts()) {
      if (auto phi = inst->dynCast<PhiInst>()) {
        phi->replaceoldtonew(entry, evalBloack);
      }
    }
  }
  IRBuilder builder;
  auto next = func->newBlock();
  next->setComment("StatelessCache next block");

  builder.set_pos(entry, entry->insts().end());
  builder.makeInst<BranchInst>(next);

  builder.set_pos(next);
  // prepare lookup function, lookup table (lut)
  const size_t tableSize = 1021, tableWords = tableSize * 4;
  // totally tableSize lut entries, each entry is 4 words (i32)
  const auto lutType = ArrayType::gen(i32, {tableWords}, tableWords);
  const auto entryType = ArrayType::gen(i32, {4}, 4);
  const auto lut = utils::make<GlobalVariable>(lutType, "lut_" + func->name());
  func->module()->addGlobalVar(lut->name(), lut);
  // const auto lut = func->module()->addGlobalVar("lut_" + func->name(), lutType)
  // prepare lookup function arguments: (table, i32 key1, i32 key2)
  std::vector<Value*> lookupFuncArgs;
  lookupFuncArgs.push_back(lut);

  for (auto arg : func->args()) {
    if (arg->isInt32())
      lookupFuncArgs.push_back(arg);
    else if (arg->isFloat32())
      lookupFuncArgs.push_back(builder.makeUnary(ir::ValueId::vFPTOSI, arg, i32));
    // lookupFuncArgs.push_back(builder.makeInst<UnaryInst>(ir::ValueId::vBITCAST, i32, arg));
  }
  while (lookupFuncArgs.size() < 3) {
    lookupFuncArgs.push_back(ConstantInteger::gen_i32(0));
  }
  const auto lookupFunc = getLookupFunction(func->module(), entryType, lutType);

  // ptr = call lookup(table, key1, key2), return ptr is the pointer to the lookuped entry
  const auto entryPtr = builder.makeInst<CallInst>(lookupFunc, lookupFuncArgs);
  /*
  struct LUTEntry final {
      uint64_t key;
      int val;
      int hasVal;
  };
  */
  // load the value from the lookuped entry: *(ptr + 2) by words (i32), LUTEntry.val ptr
  auto valPtr = builder.makeGetElementPtr(i32, entryPtr, ConstantInteger::gen_i32(2), {}, {4});
  if (not(valPtr->type()->as<PointerType>()->baseType()->isSame(retType))) {
    // cast val pointer to the return type pointer
    valPtr = builder.makeInst<UnaryInst>(ir::ValueId::vBITCAST, PointerType::gen(retType), valPtr);
  }
  // LUTEntry.hasVal ptr: PointerType::gen(i32)
  auto hasValPtr = builder.makeGetElementPtr(i32, entryPtr, ConstantInteger::gen_i32(3), {}, {4});
  // load the hasVal from the lookuped entry: *(ptr + 3) by words (i32), LUTEntry.hasVal ptr
  auto hasVal = builder.makeLoad(hasValPtr);
  // cmp inst
  auto cmp = builder.makeCmp(CmpOp::NE, hasVal, ConstantInteger::gen_i32(0));
  // if true, jump to earlyExit block; else, jump to evalBlock
  auto earlyExitBlock = func->newBlock();
  earlyExitBlock->addComment("early exit block");

  builder.makeInst<BranchInst>(cmp, earlyExitBlock, evalBloack);

  // build entry end
  // earlyExitBlock: return the lookuped value directly
  // only one exit, modified cfg
  const auto originalExit = func->exit();

  // remove retInst from originalExit
  const auto retInst = originalExit->terminator()->as<ReturnInst>();
  const auto originalRetVal = retInst->returnValue();
  // assert(retInst and originalRetVal);
  if(!retInst or !originalRetVal) {
    std::cerr << "StatelessCache: " << func->name() << " has no return value" << std::endl;
    return false;
  }
  originalExit->insts().remove(retInst);

  builder.set_pos(originalExit, originalExit->insts().end());
  // store hasVal to the lookuped entry
  builder.makeInst<StoreInst>(ConstantInteger::gen_i32(1), hasValPtr);
  // store val to the lookuped entry
  builder.makeInst<StoreInst>(originalRetVal, valPtr);

  // build newExit
  const auto newExit = func->newBlock();
  newExit->setComment("StatelessCache new exit block");
  func->setExit(newExit);

  // originalExit -> newExit
  builder.set_pos(originalExit, originalExit->insts().end());
  builder.makeInst<BranchInst>(newExit);

  // earlyExitBlock -> newExit
  builder.set_pos(earlyExitBlock, earlyExitBlock->insts().end());
  auto earlyExitVal = builder.makeLoad(valPtr);
  builder.makeInst<BranchInst>(newExit);

  // newExit:
  // insert phi inst to merge the lookuped value and original return value
  builder.set_pos(newExit, newExit->insts().begin());
  auto phi = utils::make<PhiInst>(nullptr, retType);
  newExit->emplace_first_inst(phi);
  // return phi
  phi->addIncoming(earlyExitVal, earlyExitBlock);
  phi->addIncoming(originalRetVal, originalExit);
  builder.makeInst<ReturnInst>(phi);

  // func->rename();
  // func->print(std::cerr);
  tp->CallChange();
  tp->CFGChange(func);
  tp->IndVarChange(func);
  // fix cfg
  CFGAnalysisHHW().run(func, tp);
  return true;
}

void StatelessCache::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  runImpl(func, tp);
}
}  // namespace pass