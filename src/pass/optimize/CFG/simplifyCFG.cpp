#include "pass/optimize/simplifyCFG.hpp"

/*
Performs dead code elimination and basic block merging. Specifically
 - Removes basic blocks with no predecessors.----1
 - Merges a basic block into its predecessor if there is only one and the predecessor only has one
 successor.----2
 - Eliminates PHI nodes for basic blocks with a single predecessor.----3
 - Eliminates a basic block that only contains an unconditional branch.----4
*/

namespace pass {

void SimplifyCFG::run(ir::Function* func, TopAnalysisInfoManager* tp) {
  if (func->isOnlyDeclare()) return;
  // func->print(std::cout);
  bool isChange = false;
  bool isWhile = true;
  while (isWhile) {
    isWhile = false;
    isWhile = isWhile or removeNoPreBlock(func);
    isWhile = isWhile or MergeBlock(func);
    isWhile = isWhile or removeSingleIncomingPhi(func);
    // func->rename();
    // func->print(std::cerr);
    isWhile = isWhile or removeSingleBrBlock(func);
    // func->rename();
    // func->print(std::cerr);
    isChange = isWhile or isChange;
  }

  if (isChange) {
    tp->CFGChange(func);
    tp->CallChange();
  }
}
// condition 1
// 移除所有没有从前驱和从末尾不可达的块
bool SimplifyCFG::removeNoPreBlock(ir::Function* func) {
  bool ischanged = false;
  std::stack<ir::BasicBlock*> bbStack;
  std::unordered_map<ir::BasicBlock*, bool> vis1;
  std::unordered_map<ir::BasicBlock*, bool> vis2;
  for (auto bb : func->blocks()) {  // 初始化vis和Stack
    vis1[bb] = vis2[bb] = false;
  }
  assert(func->entry());
  bbStack.push(func->entry());   // 从entry开始搜索
  while (not bbStack.empty()) {  // DFS算法
    auto curBB = bbStack.top();
    bbStack.pop();
    if (vis1[curBB]) continue;
    vis1[curBB] = true;
    for (auto curBBnext : curBB->next_blocks()) {
      bbStack.push(curBBnext);
    }
  }
  assert(bbStack.empty());
  assert(func->exit());
  bbStack.push(func->exit());  // 从exit开始搜索
  while (not bbStack.empty()) {
    auto curBB = bbStack.top();  // DFS
    bbStack.pop();
    if (vis2[curBB]) continue;
    vis2[curBB] = true;
    for (auto curBBPre : curBB->pre_blocks()) {
      bbStack.push(curBBPre);
    }
  }
  // std::cout<<"In Fucntion"<<func->name()<<""<<std::endl;
  // for(auto bb:func->blocks()){
  //     using namespace std;
  //     std::cout<<"bb "<<bb->name()<<": "<<vis1[bb]<<' '<<vis2[bb]<<endl;
  // }
  for (auto bbIter = func->blocks().begin(); bbIter != func->blocks().end();) {
    auto bb = *bbIter;
    bbIter++;
    if (not(vis1[bb] and vis2[bb])) {
      ischanged = true;
      for (auto puseIter = bb->uses().begin(); puseIter != bb->uses().end();) {
        auto puse = *puseIter;
        puseIter++;
        auto bbUser = puse->user();
        auto phiInstUser = dyn_cast<ir::PhiInst>(bbUser);
        if (phiInstUser) {
          phiInstUser->delBlock(bb);
        }
      }
      func->forceDelBlock(bb);
    }
  }
  return ischanged;
}
// condition 2 合并只有前驱和只有后继的块

ir::BasicBlock* SimplifyCFG::getMergeBlock(ir::BasicBlock* bb) {  // condition 2
  if (bb->next_blocks().size() == 1)
    if ((*(bb->next_blocks().begin()))->pre_blocks().size() == 1)
      return *(bb->next_blocks().begin());

  return nullptr;
}

bool SimplifyCFG::MergeBlock(ir::Function* func) {
  bool ischanged = false;
  for (auto bb : func->blocks()) {
    // func->print(std::cout);
    if (bb == func->entry()) continue;
    auto mergeBlock = getMergeBlock(bb);
    while (mergeBlock) {
      if (mergeBlock == func->exit()) func->setExit(bb);
      if (not ischanged) ischanged = true;
      // 去掉两个块的联系
      ir::BasicBlock::delete_block_link(bb, mergeBlock);
      // 删除最后一条跳转指令
      bb->delete_inst(bb->insts().back());
      // 将下一个bb的所有语句复制
      for (auto inst : mergeBlock->insts()) {
        inst->setBlock(bb);
        bb->emplace_inst(bb->insts().end(), inst);
      }
      // 将下一个bb的所有后继与当前进行连接
      for (auto mergeBBNextIter = mergeBlock->next_blocks().begin();
           mergeBBNextIter != mergeBlock->next_blocks().end();) {
        auto mergeBBNext = *mergeBBNextIter;
        mergeBBNextIter++;
        ir::BasicBlock::delete_block_link(mergeBlock, mergeBBNext);
        ir::BasicBlock::block_link(bb, mergeBBNext);
      }
      // 将所有的对mergeBB的使用进行替换
      mergeBlock->replaceAllUseWith(bb);

      // 将merge块删掉
      // 因为这些语句没有消失,不能使用一般的delete接口直接删除use
      mergeBlock->insts().clear();
      func->blocks().remove(mergeBlock);
      mergeBlock = getMergeBlock(bb);
    }
  }
  return ischanged;
}
// condition 3 removing phiNodes with only one incoming
bool SimplifyCFG::removeSingleIncomingPhi(ir::Function* func) {
  bool ischanged = false;
  for (auto bb : func->blocks()) {
    for (auto instIter = bb->phi_insts().begin(); instIter != bb->phi_insts().end();) {
      auto inst = *instIter;
      instIter++;
      auto phiInst = dyn_cast<ir::PhiInst>(inst);
      if (phiInst->getsize() == 1) {
        phiInst->replaceAllUseWith(phiInst->getValue(0));
        bb->delete_inst(phiInst);
        ischanged = true;
      }
    }
  }
  return ischanged;
}
// condition 4 处理只含有一个br uncond 的块
// 判断符合条件的块--只有一条指令, 是无条件跳转, 不是entry
bool SimplifyCFG::getSingleDest(ir::BasicBlock* bb) {  // condition 4
  if (bb->insts().size() != 1) return false;
  if (bb == bb->function()->entry()) return false;
  auto brInst = dyn_cast<ir::BranchInst>(bb->terminator());
  if (brInst == nullptr) return false;
  return not brInst->is_cond();
}

bool SimplifyCFG::removeSingleBrBlock(ir::Function* func) {
  // 这里移除只有一条跳转指令的block
  // 原则如下:
  /*
  1. curBB跳转到destBB
  2. 如果二者没有同样的Pre, 就直接把curBB删除
      - 删除curBB之后要注意phi, 并且相应设置destBB的phi
      - 设置相应的新的block_link关系
  3. 如果二者有相同的Pre:
      - 如果curBB到达destBB的phi和这些preBB到达destBB的phi均一致就可以把curBB直接删除,
  并且将preBB的跳转全部设置为无条件的
      - 重置phi
      - 反之, 则不能将curBB删除!
  */
  bool ischanged = false;
  std::vector<ir::BasicBlock*> worklist;
  for (auto bb : func->blocks()) {  // 找出所有的单br块
    if (getSingleDest(bb)) worklist.push_back(bb);
  }
  while (not worklist.empty()) {
    auto curBB = worklist.back();
    worklist.pop_back();
    auto destBB = curBB->next_blocks().front();
    auto curBBpreBBs =
      std::set<ir::BasicBlock*>(curBB->pre_blocks().begin(), curBB->pre_blocks().end());
    auto destBBpreBBs =
      std::set<ir::BasicBlock*>(destBB->pre_blocks().begin(), destBB->pre_blocks().end());
    std::vector<ir::BasicBlock*> curBBdestBBPreBBIntersection;
    std::set_intersection(curBBpreBBs.begin(), curBBpreBBs.end(), destBBpreBBs.begin(),
                          destBBpreBBs.end(),
                          std::insert_iterator<std::vector<ir::BasicBlock*>>(
                            curBBdestBBPreBBIntersection, curBBdestBBPreBBIntersection.begin()));
    if (curBBdestBBPreBBIntersection.size() == 0) {
      // func->print(std::cerr);
      // 添加新的边, 删除旧的边
      for (auto curBBpreBB : curBBpreBBs) {
        ir::BasicBlock::delete_block_link(curBBpreBB, curBB);
        ir::BasicBlock::block_link(curBBpreBB, destBB);
        // 改变brInst的地址
        auto brInst = dyn_cast<ir::BranchInst>(curBBpreBB->terminator());
        if (brInst->is_cond()) {
          if (brInst->iftrue() == curBB) brInst->set_iftrue(destBB);
          if (brInst->iffalse() == curBB) brInst->set_iffalse(destBB);
        } else {
          if (brInst->dest() == curBB) brInst->set_dest(destBB);
        }
        // 在destBB的phi中加入到达定值
        for (auto pinst : destBB->phi_insts()) {
          auto phiinst = dyn_cast<ir::PhiInst>(pinst);
          // func->print(std::cerr);
          // for(auto moperand:phiinst->operands()){
          //     std::cerr<<moperand->value()->name()<<std::endl;
          // }
          // std::cerr<<"Done"<<std::endl;
          auto valFromCurBB = phiinst->getvalfromBB(curBB);
          phiinst->addIncoming(valFromCurBB, curBBpreBB);
        }
      }
      // 所有东西做完之后的检查:
      assert(curBB->pre_blocks().size() == 0);
      // 删掉curBB
      // 删掉phi到达定值
      for (auto pinst : destBB->phi_insts()) {
        auto phiinst = dyn_cast<ir::PhiInst>(pinst);
        phiinst->delBlock(curBB);
      }
      // 去除blocklink
      ir::BasicBlock::delete_block_link(curBB, destBB);
      // func->rename();
      // func->print(std::cerr);
      // std::cerr<<std::endl;
      // 检查没有curBB的使用
      assert(curBB->uses().size() == 0);
      func->delBlock(curBB);
      ischanged = true;
    } else {
      // 判断这样的块是否可以删除
      // 遍历curBB和preBB的destBB Incoming,查看对应value是不是一致, 如果能够一致就可以进一步
      bool isExchange = true;
      for (auto pinst : destBB->phi_insts()) {
        auto phiinst = dyn_cast<ir::PhiInst>(pinst);
        auto curBBIncomingVal = phiinst->getvalfromBB(curBB);
        for (auto bbIncoming : curBBdestBBPreBBIntersection) {
          auto nowIncomingVal = phiinst->getvalfromBB(bbIncoming);
          if (nowIncomingVal != curBBIncomingVal) {
            isExchange = false;
            break;
          }
        }
        if (not isExchange) break;
      }
      if (not isExchange) continue;
      /*
          1. curBBdestBBPreBBIntersection中的块, 直接变为无条件跳转到destBB, 删除它们到curBB的链接
          2. 对于不在curBBdestBBPreBBIntersection中的但是在curBBPreBBs中的, 删除到curBB的链接,
         替换跳转目标到curBB, 添加对应块的到达定值
          3. 删除curBB到destBB的链接, 删除curBB的到达定值, 删除curBB
      */
      for (auto curBBPreBB : curBBpreBBs) {
        if (destBBpreBBs.count(curBBPreBB)) {
          // 修改terminator为无条件跳转
          curBBPreBB->delete_inst(curBBPreBB->terminator());
          auto newBrInst = new ir::BranchInst(destBB, curBBPreBB);
          curBBPreBB->emplace_back_inst(newBrInst);
          // 修改链接
          ir::BasicBlock::delete_block_link(curBBPreBB, curBB);
        } else {
          // 修改链接
          ir::BasicBlock::delete_block_link(curBBPreBB, curBB);
          ir::BasicBlock::block_link(curBBPreBB, destBB);
          // 修改跳转目标
          auto brTerminator = dyn_cast<ir::BranchInst>(curBBPreBB->terminator());
          if (brTerminator->is_cond()) {
            if (brTerminator->iffalse() == curBB) brTerminator->set_iffalse(destBB);
            if (brTerminator->iftrue() == curBB) brTerminator->set_iftrue(destBB);
          } else {
            if (brTerminator->dest() == curBB) brTerminator->set_dest(destBB);
          }
          // 添加到达定值
          for (auto pinst : destBB->phi_insts()) {
            auto phiinst = dyn_cast<ir::PhiInst>(pinst);
            phiinst->addIncoming(phiinst->getvalfromBB(curBB), curBBPreBB);
          }
        }
      }
      // 删除curBB
      ir::BasicBlock::delete_block_link(curBB, destBB);
      for (auto pinst : destBB->phi_insts()) {
        auto phiinst = dyn_cast<ir::PhiInst>(pinst);
        phiinst->delBlock(curBB);
      }
      func->delBlock(curBB);
      ischanged = true;
    }
  }
  return ischanged;
}
}  // namespace pass