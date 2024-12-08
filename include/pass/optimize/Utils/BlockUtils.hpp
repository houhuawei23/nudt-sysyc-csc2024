#pragma once
#include "ir/ir.hpp"
#include "pass/AnalysisInfo.hpp"
#include <functional>

using namespace ir;
using BlockReducer = std::function<ir::Value*(ir::Instruction* inst)>;

namespace pass {

void dumpInst(std::ostream& os, Instruction* inst);
void dumpAsOperand(std::ostream& os, Value* val);

bool reduceBlock(IRBuilder& builder, BasicBlock& block, const BlockReducer& reducer);

BasicBlock* splitBlock(BasicBlockList& blocks,
                       BasicBlockList::iterator blockIt,
                       InstructionList::iterator instIt);

bool fixAllocaInEntry(Function& func);

bool blockSortBFS(Function& func, TopAnalysisInfoManager* tAIM);
bool blockSortDFS(Function& func, TopAnalysisInfoManager* tAIM);

bool fixPhiIncomingBlock(BasicBlock* target, BasicBlock* oldBlock, BasicBlock* newBlock);
}  // namespace pass