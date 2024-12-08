#pragma once
#include "pass/AnalysisInfo.hpp"
#include "mir/MIR.hpp"
#include "mir/target.hpp"
#include "mir/iselinfo.hpp"
#include "support/arena.hpp"
namespace mir {

class FloatPointConstantPool {
  MIRDataStorage* mFloatDataStorage = nullptr;
  // (uint32_t) fval -> offset in data storage
  std::unordered_map<uint32_t, size_t> mFloatOffsetMap;

public:
  MIROperand getFloatConstant(class LoweringContext& ctx, float val);
};

/* LoweringContext */
class LoweringContext : public MIRBuilder {
public:
  Target& mTarget;
  MIRModule& module;

  /* global mappings */
  std::unordered_map<ir::Function*, MIRFunction*> funcMap;
  std::unordered_map<ir::GlobalVariable*, MIRGlobalObject*> gvarMap;

  /* local variable mappings */
  std::unordered_map<ir::Value*, MIROperand> valueMap;
  std::unordered_map<ir::BasicBlock*, MIRBlock*> blockMap;

  /* Float Point Constant Pool */
  FloatPointConstantPool mFloatConstantPool;
  // TODO: can use inter-block cache to reduce memory usage?
  std::unordered_map<MIRBlock*, std::unordered_map<float, MIROperand>> mBlockLoadedFloatCache;

  /* Pointer Type for Target Platform */
  OperandType pointerType = OperandType::Int64;

  /* Code Generate Context */
  CodeGenContext* codeGenctx = nullptr;
  MIRFunction* memsetFunc;

public:
  LoweringContext(MIRModule& mir_module, Target& target) : module(mir_module), mTarget(target) {
    module.functions().push_back(std::make_unique<MIRFunction>("_memset", &mir_module));
    memsetFunc = module.functions().back().get();
  }

public:  // set function
  void setCodeGenCtx(CodeGenContext* ctx) { codeGenctx = ctx; }

public:  // get function
  auto getPointerType() { return pointerType; }

public:  // gen function
  MIROperand newVReg(ir::Type* type);
  MIROperand newVReg(OperandType type);

public:  // emit function
  void emitCopy(MIROperand dst, MIROperand src);

  // ir_val -> mir_operand
  void addValueMap(ir::Value* ir_val, MIROperand mir_operand);
  MIROperand map2operand(ir::Value* ir_val);
  MIRBlock* map2block(ir::BasicBlock* ir_block) {
    if (blockMap.find(ir_block) == blockMap.end()) {
      std::cerr << ir_block->name() << " not found in blockMap" << std::endl;
      assert(false);
    }
    return blockMap.at(ir_block);
  }
};

std::unique_ptr<MIRModule> createMIRModule(ir::Module& ir_module,
                                           Target& target,
                                           pass::TopAnalysisInfoManager* tAIM);
}  // namespace mir