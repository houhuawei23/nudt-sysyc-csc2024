#include "pass/optimize/optimize.hpp"
#include "pass/optimize/reg2mem.hpp"
#include "support/arena.hpp"
#include <set>
#include <cassert>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>

using namespace ir;
namespace pass {
void Reg2MemContext::getallphi(Function* func) {
  for (BasicBlock* bb : func->blocks()) {
    if (bb->phi_insts().empty()) {
      continue;
    } else {
      phiblocks.push_back(bb);
      for (auto inst : bb->phi_insts()) {
        PhiInst* phiinst = dyn_cast<PhiInst>(inst);
        bbphismap[bb].push_back(phiinst);
        allphi.push_back(phiinst);
      }
    }
  }
}

void Reg2MemContext::run(Function* func, TopAnalysisInfoManager* tp) {
  getallphi(func);
  BasicBlock* entry = func->entry();
  for (PhiInst* phiinst : allphi) {
    auto var = utils::make<AllocaInst>(phiinst->type());
    allocasToinsert.push_back(var);
    phiweb[phiinst] = var;
  }
  // for (auto alloca : allocasToinsert) {

  //     entry->emplace_lastbutone_inst(alloca);
  // }
  for (auto BB : phiblocks) {
    std::unordered_map<BasicBlock*, BasicBlock*> repalceBBmap;
    for (PhiInst* phiinst : bbphismap[BB]) {
      // AllocaInst* variable = phiweb[phiinst];
      // assert(variable!=nullptr&&"nullptr val");
      // LoadInst* phiload = new LoadInst(variable, phiinst->type(), nullptr);  //
      // 用这条load替换所有对phiinst的使用 philoadmap[phiinst] = phiload; // 记录phiinst与load的映射

      for (size_t i = 0; i < phiinst->getsize(); i++) {
        BasicBlock* prebb = phiinst->getBlock(i);
        Value* phival = phiinst->getValue(i);
        // if (phival->type()->isUndef()) {  // 如果是phi或则undef则不插入store
        //     continue;
        // }

        if (repalceBBmap.count(prebb)) {
          prebb = repalceBBmap[prebb];
        }
        // StoreInst* phistore = new StoreInst(phival, variable);
        if ((prebb->next_blocks().size() == 1) &&
            (prebb != entry)) {  // 如果前驱块只有一个后继，直接在前驱块末尾插入store
          // prebb->emplace_lastbutone_inst(phistore);
          // phistore->setBlock(prebb);
          ;
        } else {  // 有多个后继则需要在前驱块与当前块中插入一个新的块，在新块中插入store与br指令
          BasicBlock* newbb = func->newBlock();
          repalceBBmap[prebb] = newbb;
          BranchInst* br = new BranchInst(BB);
          // newbb->emplace_back_inst(phistore);
          newbb->emplace_back_inst(br);
          Instruction* lastinst = prebb->insts().back();
          BranchInst* oldbr = dyn_cast<BranchInst>(lastinst);
          oldbr->replaceDest(BB, newbb);
          BasicBlock::delete_block_link(prebb, BB);
          BasicBlock::block_link(prebb, newbb);
          BasicBlock::block_link(newbb, BB);
          prebb = newbb;
        }
        phiinst->replaceBlock(prebb, i);
      }
    }
  }

  // // 删除phi，并将对phi的使用替换为load
  // for (PhiInst* phitoremove : allphi) {
  //     BasicBlock* phibb = phitoremove->block();
  //     LoadInst* loadinst = philoadmap[phitoremove];
  //     phitoremove->replaceAllUseWith(loadinst);
  //     phibb->delete_inst(phitoremove);
  //     phibb->emplace_first_inst(loadinst);
  // }
}

void Reg2MemContext::DisjSet() {
  for (int i = 0; i < allphi.size(); i++) {
    parent.push_back(i);
    rank.push_back(1);
  }
  for (ir::PhiInst* phiinst : allphi) {
    for (size_t i = 0; i < phiinst->getsize(); i++) {
      ir::Value* val = phiinst->getValue(i);
      if (ir::PhiInst* phival = dyn_cast<ir::PhiInst>(val)) {
        int id0 = getindex(phiinst);
        int id1 = getindex(phival);
        if (issame(id0, id1)) {
          continue;
        } else {
          tounion(id0, id1);
        }
      }
    }
  }
}

void Reg2Mem::run(Function* func, TopAnalysisInfoManager* tp) {
  Reg2MemContext ctx;
  ctx.run(func, tp);
}

}  // namespace pass
