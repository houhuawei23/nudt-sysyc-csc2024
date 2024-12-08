#include "pass/optimize/optimize.hpp"
#include "pass/optimize/inline.hpp"
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>
namespace pass {

void Inline::run(ir::Module* module, TopAnalysisInfoManager* tp) {
  InlineContext ctx;
  ctx.run(module, tp);
}

void InlineContext::callinline(ir::CallInst* call) {
  auto callee = call->callee();         // 被调用的需要被展开的函数
  auto copyfunc = callee->copy_func();  // callee的复制，展开的是这个函数而不是callee
  auto nowBB = call->block();
  auto caller = nowBB->function();
  auto retBB = caller->newBlock();
  auto calleeAllocaBB = copyfunc->entry();
  auto callerAllocaBB = caller->entry();

  // std::cerr << "caller: " << caller->name() << std::endl;
  // std::cerr << "callerAllocaBB: " << callerAllocaBB->name() << ", size: " <<
  // callerAllocaBB->insts().size()
  //           << std::endl;

  // std::cerr << "callee: " << callee->name() << std::endl;
  // std::cerr << "calleeAllocaBB: " << calleeAllocaBB->name() << ", size: " <<
  // calleeAllocaBB->insts().size()
  //           << std::endl;

  if (nowBB == caller->exit()) {
    caller->setExit(retBB);
  }
  // 将call之后的指令移动到retBB中
  auto it = std::find(nowBB->insts().begin(), nowBB->insts().end(), call);
  if (it != nowBB->insts().end()) {
    ++it;
    while (it != nowBB->insts().end()) {
      ir::Instruction* inst = *it;
      inst->setBlock(retBB);
      retBB->emplace_back_inst(inst);
      it = nowBB->insts().erase(it);
    }
  }

  // 将nowBB的后继变为retBB的后继
  auto succList = nowBB->next_blocks();
  for (auto it = succList.begin(); it != succList.end();) {
    ir::BasicBlock* succBB = *it;
    ir::BasicBlock::delete_block_link(nowBB, succBB);
    ir::BasicBlock::block_link(retBB, succBB);
    for (auto phi : succBB->phi_insts()) {  // 修改phi指令中的nowBB为retBB
      ir::PhiInst* phiinst = dyn_cast<ir::PhiInst>(phi);
      for (size_t i = 0; i < phiinst->getsize(); i++) {
        ir::BasicBlock* phiBB = phiinst->getBlock(i);
        if (phiBB == nowBB) {
          phiinst->replaceBlock(retBB, i);
        }
      }
    }
    it = succList.erase(it);
  }

  // 被调用函数的返回块无条件跳转到retBB
  std::vector<std::pair<ir::ReturnInst*, ir::BasicBlock*>> retmap;
  auto blocklist = copyfunc->blocks();
  for (auto it = blocklist.begin(); it != blocklist.end();) {
    ir::BasicBlock* bb = *it;
    bb->set_parent(caller);
    caller->blocks().emplace_back(bb);
    it = copyfunc->blocks().erase(it);
    if (bb->next_blocks().empty()) {
      auto lastinst = bb->insts().back();
      if (auto ret = dyn_cast<ir::ReturnInst>(lastinst)) {
        retmap.push_back(std::make_pair(ret, bb));
      }
    }
  }
  // 如果函数的返回值不是void，则需要把call的使用全部替换为返回值的使用
  if ((!copyfunc->retType()->isVoid()) && (!retmap.empty())) {
    if (retmap.size() == 1) {  // 如果只有一个返回值
      call->replaceAllUseWith(retmap[0].first->returnValue());
    } else {
      ir::PhiInst* newphi = new ir::PhiInst(retBB, call->type());
      retBB->emplace_first_inst(newphi);
      for (auto [ret, bb] : retmap) {
        newphi->addIncoming(ret->returnValue(), bb);
      }
      call->replaceAllUseWith(newphi);
    }
  }
  // 在copyfunc的retbb中插入无条件跳转指令到caller的retBB
  for (auto [ret, bb] : retmap) {
    ir::BasicBlock::block_link(bb, retBB);
    auto jmprettobb = new ir::BranchInst(retBB, bb);
    bb->delete_inst(ret);
    bb->emplace_back_inst(jmprettobb);
  }
  // 被调用函数的参数
  for (size_t i = 0; i < copyfunc->args().size(); i++) {
    auto realArg = call->operand(i);
    auto formalArg = copyfunc->arg_i(i);
    if (!formalArg->type()
           ->isPointer()) {  // 如果传递参数不是高维数组等指针，直接替换TODO 加入一维数组的判断
      formalArg->replaceAllUseWith(realArg);
    } else {  // 如果传递参数是数组等指针
      auto baseType = formalArg->type()->dynCast<ir::PointerType>()->baseType();
      if (!baseType->isPointer()) {
        formalArg->replaceAllUseWith(realArg);
      } else {
        std::vector<ir::StoreInst*> storetomove;
        for (auto use : formalArg->uses()) {
          if (ir::StoreInst* storeinst = dyn_cast<ir::StoreInst>(use->user())) {
            storetomove.push_back(storeinst);
            auto allocainst = storeinst->ptr();
            std::vector<ir::LoadInst*> loadtoremove;
            for (auto allocause : allocainst->uses()) {
              if (ir::LoadInst* loadinst = dyn_cast<ir::LoadInst>(allocause->user())) {
                loadtoremove.push_back(loadinst);
              }
            }
            for (auto rmloadinst : loadtoremove) {
              rmloadinst->replaceAllUseWith(realArg);
              auto loadBB = rmloadinst->block();
              loadBB->delete_inst(rmloadinst);
            }
          }
        }
        for (auto rmstoreinst : storetomove) {
          auto storeBB = rmstoreinst->block();
          storeBB->delete_inst(rmstoreinst);
        }
      }
    }
  }
  // 删除caller中调用copyfunc的call指令
  nowBB->delete_inst(call);
  // 连接nowBB和callee的entry,在nowBB末尾插入无条件跳转指令到copyfunc的entry
  ir::BasicBlock::block_link(nowBB, calleeAllocaBB);
  auto jmpnowtoentry = new ir::BranchInst(calleeAllocaBB, nowBB);
  nowBB->emplace_back_inst(jmpnowtoentry);
  // 将callee的alloca提到caller
  auto& calleeAllocaBBinst = calleeAllocaBB->insts();
  for (auto it = calleeAllocaBBinst.begin(); it != calleeAllocaBBinst.end();) {
    ir::Instruction* inst = *it;
    if (auto allocainst = dyn_cast<ir::AllocaInst>(inst)) {
      callerAllocaBB->emplace_first_inst(allocainst);
      // callerAllocaBB->emplace_lastbutone_inst(allocainst);
      it = calleeAllocaBBinst.erase(it);
    } else {
      ++it;
    }
  }
}

std::vector<ir::CallInst*> InlineContext::getcall(ir::Module* module, ir::Function* function) {
  std::vector<ir::CallInst*> calllist;
  for (auto func : module->funcs()) {
    for (auto bb : func->blocks()) {
      for (auto inst : bb->insts()) {
        if (auto callinst = dyn_cast<ir::CallInst>(inst)) {
          if (function == callinst->callee()) {
            calllist.push_back(callinst);
          }
        }
      }
    }
  }

  return calllist;
}

std::vector<ir::Function*> InlineContext::getinlineFunc(ir::Module* module) {
  std::vector<ir::Function*> functiontoremove;
  // std::cerr << "inline" << std::endl;
  for (auto func : module->funcs()) {
    if (func->name() != "main" && !cgctx->isLib(func) && cgctx->isNoCallee(func) &&
        !func->attribute().hasAttr(ir::FunctionAttribute::ParallelBody |
                                   ir::FunctionAttribute::Builtin)) {
      //&& !func->get_is_inline() TODO
      // 分析哪些函数可以被内联优化展开
      // std::cerr << func->name() << " ";
      functiontoremove.push_back(func);
    }
  }
  // std::cerr << std::endl;
  return functiontoremove;
}
void InlineContext::run(ir::Module* module, TopAnalysisInfoManager* tp) {
  cgctx = tp->getCallGraph();
  // cgctx->setOff();
  // cgctx->refresh();
  std::vector<ir::Function*> functiontoremove = getinlineFunc(module);
  bool isFuncInline = false;
  while (!functiontoremove.empty()) {  // 找到所有调用了可以被内联优化展开的函数的call指令
    auto func = functiontoremove.back();
    // std::cerr << "inline function: " << func->name() << std::endl;
    std::vector<ir::CallInst*> callList = getcall(module, func);
    for (auto call : callList) {
      callinline(call);
      isFuncInline = true;
      tp->CFGChange(call->block()->function());
    }
    module->delFunction(func);

    functiontoremove.pop_back();
    if (functiontoremove.empty()) {
      cgctx = tp->getCallGraph();
      functiontoremove = getinlineFunc(module);
    }
  }
  tp->CallChange();
}

}  // namespace pass