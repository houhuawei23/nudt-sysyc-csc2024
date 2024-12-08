#pragma once
#include "ir/ir.hpp"
#include "pass/pass.hpp"
#include "pass/analysis/dependenceAnalysis/DependenceAnalysis.hpp"

using namespace ir;
namespace pass {
class LoopDependenceInfo;
struct GepIdx;
enum BaseAddrType { globalType, localType, argType };
enum IdxType {           // 单个idx的类型
  iLOOPINVARIANT,        // lpI
  iIDV,                  // i
  iIDVPLUSMINUSFORMULA,  // i+a i-a (a==lpI)
  iINNERIDV,
  iINNERIDVPLUSMINUSFORMULA,
  iCALL,  // f()
  iLOAD,  // a[] or global
  iELSE,
};
enum DependenceType {
  dTotallySame = 1,
  dPossiblySame = 1 << 2,
  dTotallyNotSame = 1 << 3,
  dCrossIterTotallySame = 1 << 4,
  dCrossIterPossiblySame = 1 << 5,
  dCrossIterTotallyNotSame = 1 << 6
};
struct GepIdx {
  std::vector<Value*> idxList;
  std::unordered_map<Value*, IdxType> idxTypes;
};
class LoopDependenceInfo {
private:
  TopAnalysisInfoManager* tp;
  // for outside info
  Loop* parent;                    // 当前循环
  bool isParallelConcerningArray;  // 是否应当并行(仅考虑数组间依赖)
  bool hasSideEffectFunction;      // 后期使用副作用来细化
  // for baseaddr
  std::set<Value*> baseAddrs;                               // 存储该循环中用到的基址
  std::unordered_map<Value*, bool> baseAddrIsRead;          // 当前基址是否读
  std::unordered_map<Value*, bool> baseAddrIsWrite;         // 当前基址是否写
  bool isBaseAddrPossiblySame;                              // 基址别名可能
  std::unordered_map<Value*, bool> baseAddrIsCrossIterDep;  // 当前子地址是否有跨循环依赖
  // for subAddr
  std::unordered_map<Value*, std::set<GetElementPtrInst*>>
    baseAddrToSubAddrs;                                             // 子地址和基地址映射
  std::unordered_map<GetElementPtrInst*, GepIdx*> subAddrToGepIdx;  // 保存gep的Idx信息
  std::unordered_map<GetElementPtrInst*, bool> subAddrIsRead;       // 当前子地址是否读
  std::unordered_map<GetElementPtrInst*, bool> subAddrIsWrite;      // 当前子地址是否写

  // for readwrite
  std::unordered_map<GetElementPtrInst*, std::set<Instruction*>> subAddrToInst;  // 子地址到存取语句
  std::set<Instruction*> memInsts;  // 在当前这个循环中进行了存取的语句

public:
  // utils
  // 直接根据循环相关信息对当前info进行构建
  void makeLoopDepInfo(Loop* lp, TopAnalysisInfoManager* topmana);
  void clearAll() {
    baseAddrs.clear();
    baseAddrToSubAddrs.clear();
    subAddrToGepIdx.clear();
    subAddrToInst.clear();
    memInsts.clear();
    baseAddrIsRead.clear();
    baseAddrIsWrite.clear();
    baseAddrIsCrossIterDep.clear();
    subAddrIsRead.clear();
    subAddrIsWrite.clear();
  }
  void getInfoFromSubLoop(Loop* subLoop, LoopDependenceInfo* subLoopDepInfo);
  void setTp(TopAnalysisInfoManager* topmana) { tp = topmana; }

  // get
  std::set<Value*>& getBaseAddrs() { return baseAddrs; }
  bool getIsBaseAddrPossiblySame() { return isBaseAddrPossiblySame; }
  bool getIsParallel() { return isParallelConcerningArray; }
  bool getIsSubAddrRead(GetElementPtrInst* gep) { return subAddrIsRead[gep]; }
  bool getIsSubAddrWrite(GetElementPtrInst* gep) { return subAddrIsWrite[gep]; }
  bool getIsBaseAddrRead(Value* val) { return baseAddrIsRead[val]; }
  bool getIsBaseAddrWrite(Value* val) { return baseAddrIsWrite[val]; }
  std::set<GetElementPtrInst*>& baseAddrToSubAddrSet(Value* baseaddr) {
    return baseAddrToSubAddrs[baseaddr];
  }
  GepIdx* getGepIdx(GetElementPtrInst* subaddr) { return subAddrToGepIdx[subaddr]; }
  std::set<Instruction*> getSubAddrInsts(GetElementPtrInst* gep) { return subAddrToInst[gep]; }

  // print for dbg
  void print(std::ostream& os);

  // set
  void setIsBaseAddrPossiblySame(bool b) { isBaseAddrPossiblySame = b; }
  void setIsParallel(bool b) { isParallelConcerningArray = b; }
  void setBaseAddrIsCrossIterDep(Value* bd, bool b) { baseAddrIsCrossIterDep[bd] = b; }

private:
  void addPtr(Value* val, Instruction* inst);  // 用于添加一个指针进入
  // 用于从子循环添加一个ptr进入
  void addPtrFromSubLoop(Value* ptr, Instruction* inst, LoopDependenceInfo* subLoopDepInfo);
};

Value* getBaseAddr(Value* val, TopAnalysisInfoManager* tp);
Value* getIntToPtrBaseAddr(UnaryInst* inst);
BaseAddrType getBaseAddrType(Value* val);
bool isTwoBaseAddrPossiblySame(Value* ptr1,
                               Value* ptr2,
                               Function* func,
                               CallGraph* cgctx,
                               TopAnalysisInfoManager* tp);
void printIdxType(IdxType idxtype, std::ostream& os);
};  // namespace pass
