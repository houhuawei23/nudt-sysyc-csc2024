#include <iostream>
#include <fstream>
#include <queue>
#include <filesystem>
#include <string_view>
#include "pass/pass.hpp"
#include "pass/AnalysisInfo.hpp"
#include "pass/analysis/dom.hpp"
#include "mir/MIR.hpp"
#include "mir/lowering.hpp"
#include "mir/target.hpp"
#include "mir/iselinfo.hpp"
#include "mir/utils.hpp"
#include "mir/RegisterAllocator.hpp"
#include "mir/RegisterCoalescing.hpp"
#include "mir/BlockLayoutOpt.hpp"
#include "target/riscv/RISCVTarget.hpp"
#include "support/StaticReflection.hpp"
#include "support/config.hpp"
#include "support/Profiler.hpp"
#include "support/Graph.hpp"
#include "support/FileSystem.hpp"
namespace fs = std::filesystem;
namespace mir {

void createMIRModule(ir::Module& ir_module,
                     MIRModule& mir_module,
                     Target& target,
                     pass::TopAnalysisInfoManager* tAIM);

void createMIRFunction(ir::Function* ir_func,
                       MIRFunction* mir_func,
                       CodeGenContext& codegen_ctx,
                       LoweringContext& lowering_ctx,
                       pass::TopAnalysisInfoManager* tAIM);

void createMIRInst(ir::Instruction* ir_inst, LoweringContext& ctx);

void lower_GetElementPtr(ir::inst_iterator begin, ir::inst_iterator end, LoweringContext& ctx);

std::unique_ptr<MIRModule> createMIRModule(ir::Module& ir_module,
                                           Target& target,
                                           pass::TopAnalysisInfoManager* tAIM) {
  auto mir_module_uptr = std::make_unique<MIRModule>(&ir_module, target);
  createMIRModule(ir_module, *mir_module_uptr, target, tAIM);
  return mir_module_uptr;
}

void createMIRModule(ir::Module& ir_module,
                     MIRModule& mir_module,
                     Target& target,
                     pass::TopAnalysisInfoManager* tAIM) {
  auto& config = sysy::Config::getInstance();

  bool debugLowering = config.logLevel >= sysy::LogLevel::DEBUG;

  auto& functions = mir_module.functions();      // uptr vector
  auto& global_objs = mir_module.global_objs();  // uptr vector

  LoweringContext lowering_ctx(mir_module, target);
  auto& func_map = lowering_ctx.funcMap;  // ir func -> mir func
  auto& gvar_map = lowering_ctx.gvarMap;  // ir gvar -> mir gobj

  //! 1. for all functions, create MIRFunction
  for (auto func : ir_module.funcs()) {
    functions.push_back(std::make_unique<MIRFunction>(func->name(), &mir_module));
    func_map.emplace(func, functions.back().get());
  }

  //! 2. for all global variables, create MIRGlobalObject
  for (auto ir_gvar : ir_module.globalVars()) {
    const auto name = ir_gvar->name().substr(1); /* remove '@' */
    /* 基础类型 (int OR float) */
    auto type = ir_gvar->type()->dynCast<ir::PointerType>()->baseType();
    const size_t size = type->size();
    if (type->isArray()) {
      type = dyn_cast<ir::ArrayType>(type)->baseType();
    }
    const bool read_only = ir_gvar->isConst();
    const bool is_float = type->isFloat32();
    const size_t align = 4;

    if (ir_gvar->isInit()) {
      /* .data: 已初始化的、可修改的全局数据 (Array and Scalar) */
      /* NOTE: 全局变量初始化一定为常值表达式 */
      MIRDataStorage::Storage data;
      for (int i = 0; i < ir_gvar->init_cnt(); i++) {
        const auto constValue = dyn_cast<ir::ConstantValue>(ir_gvar->init(i));
        /* NOTE: float to uint32_t, type cast, doesn't change the memory */
        uint32_t word;
        if (type->isInt()) {
          const auto val = constValue->i32();
          memcpy(&word, &val, sizeof(uint32_t));
        } else if (type->isFloat32()) {
          const auto val = constValue->f32();
          memcpy(&word, &val, sizeof(float));
        } else {
          assert(false && "Not Supported Type.");
        }
        data.push_back(word);
      }
      auto mir_storage =
        std::make_unique<MIRDataStorage>(std::move(data), read_only, name, is_float);
      auto mir_gobj = std::make_unique<MIRGlobalObject>(align, std::move(mir_storage), &mir_module);
      mir_module.global_objs().push_back(std::move(mir_gobj));
    } else {
      /* .bss: 未初始化的全局数据 (Just Scalar) */
      auto mir_storage = std::make_unique<MIRZeroStorage>(size, name, is_float);
      auto mir_gobj = std::make_unique<MIRGlobalObject>(align, std::move(mir_storage), &mir_module);
      mir_module.global_objs().push_back(std::move(mir_gobj));
    }
    gvar_map.emplace(ir_gvar, mir_module.global_objs().back().get());
  }

  // TODO: transformModuleBeforeCodeGen

  //! 3. codegen
  CodeGenContext codegen_ctx{target, target.getDataLayout(), target.getTargetInstInfo(),
                             target.getTargetFrameInfo(), MIRFlags{}};
  codegen_ctx.iselInfo = &target.getTargetIselInfo();
  codegen_ctx.scheduleModel = &target.getScheduleModel();
  codegen_ctx.registerInfo = new RISCVRegisterInfo();
  lowering_ctx.codeGenctx = &codegen_ctx;

  IPRAUsageCache infoIPRA; /* 缓存各个函数所用到的Caller-Saved Registers */

  auto dumpStageWithMsg = [&](std::ostream& os, std::string_view stage, std::string_view msg) {
    if (!debugLowering) return;
    enum class Style { RED, BOLD, RESET };
    static std::unordered_map<Style, std::string_view> styleMap = {
      {Style::RED, "\033[0;31m"}, {Style::BOLD, "\033[1m"}, {Style::RESET, "\033[0m"}};

    os << "\n";
    os << styleMap[Style::RED] << styleMap[Style::BOLD];
    os << "[" << stage << "] ";
    os << styleMap[Style::RESET];
    os << msg << std::endl;
  };

  //! 4. Lower all Functions
  addExternalIPRAInfo(infoIPRA);
  for (auto& ir_func : ir_module.funcs()) {
    codegen_ctx.flags = MIRFlags{};
    if (ir_func->blocks().empty()) continue;

    /* Just for Debug */
    size_t stageIdx = 0;
    auto dumpStageResult = [&](std::string stage, MIRFunction* mir_func,
                               CodeGenContext& codegen_ctx) {
      if (!debugLowering) return;
      auto fileName = mir_func->name() + std::to_string(stageIdx) + "_" + stage + ".ll";
      auto path = config.debugDir() / fs::path(fileName);
      std::ofstream fout(path);
      mir_func->print(fout, codegen_ctx);
      stageIdx++;
    };

    if (debugLowering) {
      auto fileName = ir_func->name() + std::to_string(stageIdx) + "_" + "BeforeLowering.ll";
      auto path = config.debugDir() / fs::path(fileName);
      std::ofstream fout(path);
      ir_func->print(fout);
      stageIdx++;
      dumpStageWithMsg(std::cerr, "BeforeLowering", "Lowering " + ir_func->name());
    }

    const auto mir_func = func_map[ir_func];
    /* stage1: lower function body to generic MIR */
    {
      utils::Stage stage{"createMIRFunction"sv};
      createMIRFunction(ir_func, mir_func, codegen_ctx, lowering_ctx, tAIM);
      dumpStageWithMsg(std::cerr, "AfterCreateMIRFunction",
                       "Create MIR Function " + ir_func->name());
      dumpStageResult("AfterLowering", mir_func, codegen_ctx);
      if (!mir_func->verify(std::cerr, codegen_ctx)) {
        std::cerr << "Lowering Error: " << mir_func->name() << " failed to verify.\n";
      }
    }

    /* stage2: instruction selection */
    {
      utils::Stage stage{"instructionSelection"sv};
      ISelContext isel_ctx(codegen_ctx);
      isel_ctx.runInstSelect(mir_func);
      dumpStageWithMsg(std::cerr, "AfterIsel", "Instruction Selection " + ir_func->name());
      dumpStageResult("AfterIsel", mir_func, codegen_ctx);
    }
    /* stage3: register coalescing */
    {
      utils::Stage stage{"registerCoalescing"sv};
      RegisterCoalescing(*mir_func, codegen_ctx);
      dumpStageResult("AfterRegisterCoalescing", mir_func, codegen_ctx);
    }

    /* stage4: Optimize: peephole optimization (窥孔优化) */
    {
      utils::Stage stage{"peepholeOptimization"sv};
      while (genericPeepholeOpt(*mir_func, codegen_ctx))
        ;
      dumpStageWithMsg(std::cerr, "AfterPeephole", "Peephole Optimization " + ir_func->name());
      dumpStageResult("AfterPeephole", mir_func, codegen_ctx);
    }

    /* stage5: pre-RA legalization */
    { codegen_ctx.flags.inSSAForm = false; }

    // /* stage6: Optimize: pre-RA scheduling, minimize register usage */
    {
      preRASchedule(*mir_func, codegen_ctx);
      dumpStageResult("AfterPreRASchedule", mir_func, codegen_ctx);
    }

    /* stage7: register allocation */
    {
      utils::Stage stage{"registerAllocation"sv};
      codegen_ctx.flags.preRA = false;
      if (codegen_ctx.registerInfo) {
        mixedRegisterAllocate(*mir_func, codegen_ctx, infoIPRA);
        dumpStageWithMsg(std::cerr, "AfterRegisterAlloc", "Register Allocation " + ir_func->name());
        dumpStageResult("AfterGraphColoring", mir_func, codegen_ctx);
      }
    }

    /* stage8: stack allocation */
    {
      if (codegen_ctx.registerInfo) {
        utils::Stage stage{"stackAllocation"sv};
        /* after sa, all stack objects are allocated with .offset */
        allocateStackObjects(mir_func, codegen_ctx);
        codegen_ctx.flags.postSA = true;
        dumpStageWithMsg(std::cerr, "AfterStackAlloc", "Stack Allocation " + ir_func->name());
        dumpStageResult("AfterStackAlloc", mir_func, codegen_ctx);
      }
    }

    {
      while (genericPeepholeOpt(*mir_func, codegen_ctx))
        ;
    }

    {
      /* post-RA scheduling, minimize cycles */
      postRASchedule(*mir_func, codegen_ctx);
      dumpStageResult("AfterPostRASchedule", mir_func, codegen_ctx);
    }

    /* stage10: code layout */
    {
      assert(mir_func->verify(std::cerr, codegen_ctx));
      optimizeBlockLayout(mir_func, codegen_ctx);
      dumpStageResult("After Block Schedule", mir_func, codegen_ctx);
      assert(mir_func->verify(std::cerr, codegen_ctx));
    }

    /* stage11: simplify CFG */
    { simplifyCFG(*mir_func, codegen_ctx); }

    /* stage12: post legalization */
    {
      utils::Stage stage{"postLegalization"sv};
      postLegalizeFunc(*mir_func, codegen_ctx);
      dumpStageWithMsg(std::cerr, "AfterPostLegalize", "Post Legalization " + ir_func->name());
    }

    /* Add Function to IPRA cache */
    if (codegen_ctx.registerInfo) {
      infoIPRA.add(codegen_ctx, *mir_func);
    }

    dumpStageResult("AfterCodeGen", mir_func, codegen_ctx);

    if (!target.verify(*mir_func)) {
      std::cerr << "Lowering Error: " << mir_func->name() << " failed to verify." << std::endl;
    }
  }
  /* module verify */
  {
    auto filename = utils::preName(config.infile) + ".s";
    auto path = config.debugDir() / filename;
    std::ofstream fout(path);
    target.emit_assembly(fout, mir_module);
  }
}

void createMIRFunction(ir::Function* ir_func,
                       MIRFunction* mir_func,
                       CodeGenContext& codegen_ctx,
                       LoweringContext& lowering_ctx,
                       pass::TopAnalysisInfoManager* tAIM) {
  if (ir_func->blocks().empty()) return;
  const auto& config = sysy::Config::getInstance();
  lowering_ctx.setCurrFunc(mir_func);
  /* Some Debug Information */
  constexpr bool DebugCreateMirFunction = false;

  // TODO: before lowering, get some analysis pass result

  auto domCtx = tAIM->getDomTree(ir_func);
  domCtx->setOff();
  domCtx->refresh();
  domCtx->BFSDomTreeInfoRefresh();
  auto irBlocks = domCtx->BFSDomTreeVector();
  //! 1. map from ir to mir
  auto& block_map = lowering_ctx.blockMap;
  auto& target = codegen_ctx.target;
  auto& datalayout = target.getDataLayout();

  for (auto ir_block : irBlocks) {
    mir_func->blocks().push_back(
      std::make_unique<MIRBlock>(mir_func, "label" + std::to_string(codegen_ctx.nextLabelId())));
    block_map.emplace(ir_block, mir_func->blocks().back().get());
    for (auto ir_inst : ir_block->insts()) {
      if (ir_inst->isa<ir::PhiInst>()) {
        auto vreg = lowering_ctx.newVReg(ir_inst->type());
        lowering_ctx.addValueMap(ir_inst, vreg);
      }
    }
  }

  //! 2. emitPrologue for function
  {
    /* assign vreg to arg */
    for (auto ir_arg : ir_func->args()) {
      auto vreg = lowering_ctx.newVReg(ir_arg->type());
      lowering_ctx.addValueMap(ir_arg, vreg);
      mir_func->args().push_back(vreg);
    }
    lowering_ctx.setCurrBlock(block_map.at(ir_func->entry()));
    codegen_ctx.frameInfo.emitPrologue(mir_func, lowering_ctx);
  }
  if (DebugCreateMirFunction) {
    std::cerr << "stage 2: emitPrologue for function" << std::endl;
  }
  //! 3. process alloca, new stack object for each alloca
  lowering_ctx.setCurrBlock(block_map.at(ir_func->entry()));  // entry
  for (auto& ir_inst : ir_func->entry()->insts()) {
    // NOTE: all alloca in entry
    if (!ir_inst->isa<ir::AllocaInst>()) continue;

    const auto ir_alloca = dyn_cast<ir::AllocaInst>(ir_inst);
    auto pointee_type = ir_alloca->baseType();
    uint32_t align = 4;  // TODO: align, need bind to ir object
    auto storage = mir_func->newStackObject(codegen_ctx.nextId(),                         // id
                                            static_cast<uint32_t>(pointee_type->size()),  // size
                                            align,                                        // align
                                            0,                                            // offset
                                            StackObjectUsage::Local);
    // emit load stack object addr inst
    auto addr = lowering_ctx.newVReg(lowering_ctx.getPointerType());

    lowering_ctx.emitMIRInst(InstLoadStackObjectAddr, {addr, storage});
    // map
    lowering_ctx.addValueMap(ir_inst, addr);
  }

  //! 4. lowering all blocks
  {
    for (auto& ir_block : irBlocks) {
      auto mir_block = block_map[ir_block];
      lowering_ctx.setCurrBlock(mir_block);

      auto& insts = ir_block->insts();
      for (auto iter = insts.begin(); iter != insts.end();) {
        auto ir_inst = *iter;
        if (ir_inst->isa<ir::AllocaInst>()) {
          iter++;
          continue;
        } else if (ir_inst->isa<ir::GetElementPtrInst>()) {
          auto ir_getelement_inst = dyn_cast<ir::GetElementPtrInst>(ir_inst);
          int id = ir_getelement_inst->getid();
          if (id == 0) {
            createMIRInst(ir_inst, lowering_ctx);
            iter++;
          } else {
            auto end = iter;
            end++;
            while (end != insts.end() && (*end)->isa<ir::GetElementPtrInst>()) {
              auto preInst = std::prev(end);
              auto endInst = dyn_cast<ir::GetElementPtrInst>(*end);
              if (endInst->value() == (*preInst) && endInst->getid() != 0) {
                end++;
              } else {
                break;
              }
            }
            lower_GetElementPtr(iter, end, lowering_ctx);  // [iter, end)
            iter = end;
          }
        } else {
          if (!codegen_ctx.iselInfo->lowerInst(ir_inst, lowering_ctx))
            createMIRInst(ir_inst, lowering_ctx);
          iter++;
        }
        if (DebugCreateMirFunction) {
          ir_inst->print(std::cerr);
          std::cerr << std::endl;
        }
      }
    }
  }
  if (DebugCreateMirFunction) {
    std::cerr << "stage 4: lowering all blocks" << std::endl;
  }
}

void lower(ir::UnaryInst* ir_inst, LoweringContext& ctx);
void lower(ir::BinaryInst* ir_inst, LoweringContext& ctx);
void lower(ir::BranchInst* ir_inst, LoweringContext& ctx);
void lower(ir::LoadInst* ir_inst, LoweringContext& ctx);
void lower(ir::StoreInst* ir_inst, LoweringContext& ctx);
void lower(ir::ICmpInst* ir_inst, LoweringContext& ctx);
void lower(ir::FCmpInst* ir_inst, LoweringContext& ctx);
void lower(ir::CallInst* ir_inst, LoweringContext& ctx);
void lower(ir::ReturnInst* ir_inst, LoweringContext& ctx);
void lower(ir::BranchInst* ir_inst, LoweringContext& ctx);
void lower(ir::MemsetInst* ir_inst, LoweringContext& ctx);
void lower(ir::GetElementPtrInst* ir_inst, LoweringContext& ctx);
void lower(ir::AtomicrmwInst* ir_inst, LoweringContext& ctx);

void createMIRInst(ir::Instruction* ir_inst, LoweringContext& ctx) {
  switch (ir_inst->valueId()) {
    case ir::ValueId::vFNEG:
    case ir::ValueId::vTRUNC:
    case ir::ValueId::vZEXT:
    case ir::ValueId::vSEXT:
    case ir::ValueId::vFPTRUNC:
    case ir::ValueId::vFPTOSI:
    case ir::ValueId::vSITOFP:
    case ir::ValueId::vBITCAST:
    case ir::ValueId::vPTRTOINT:
    case ir::ValueId::vINTTOPTR:
      lower(dyn_cast<ir::UnaryInst>(ir_inst), ctx);
      break;
    case ir::ValueId::vADD:
    case ir::ValueId::vFADD:
    case ir::ValueId::vSUB:
    case ir::ValueId::vFSUB:
    case ir::ValueId::vMUL:
    case ir::ValueId::vFMUL:
    case ir::ValueId::vUDIV:
    case ir::ValueId::vSDIV:
    case ir::ValueId::vFDIV:
    case ir::ValueId::vUREM:
    case ir::ValueId::vSREM:
    case ir::ValueId::vFREM:
      lower(dyn_cast<ir::BinaryInst>(ir_inst), ctx);
      break;
    case ir::ValueId::vIEQ:
    case ir::ValueId::vINE:
    case ir::ValueId::vISGT:
    case ir::ValueId::vISGE:
    case ir::ValueId::vISLT:
    case ir::ValueId::vISLE:
      lower(dyn_cast<ir::ICmpInst>(ir_inst), ctx);
      break;
    case ir::ValueId::vFOEQ:
    case ir::ValueId::vFONE:
    case ir::ValueId::vFOGT:
    case ir::ValueId::vFOGE:
    case ir::ValueId::vFOLT:
    case ir::ValueId::vFOLE:
      lower(dyn_cast<ir::FCmpInst>(ir_inst), ctx);
      break;
    case ir::ValueId::vALLOCA:
      std::cerr << "alloca not supported" << std::endl;
      break;
    case ir::ValueId::vLOAD:
      lower(dyn_cast<ir::LoadInst>(ir_inst), ctx);
      break;
    case ir::ValueId::vSTORE:
      lower(dyn_cast<ir::StoreInst>(ir_inst), ctx);
      break;
    case ir::ValueId::vGETELEMENTPTR:
      lower(dyn_cast<ir::GetElementPtrInst>(ir_inst), ctx);
      break;
    case ir::ValueId::vRETURN:
      lower(dyn_cast<ir::ReturnInst>(ir_inst), ctx);
      break;
    case ir::ValueId::vBR:
      lower(dyn_cast<ir::BranchInst>(ir_inst), ctx);
      break;
    case ir::ValueId::vCALL:
      lower(dyn_cast<ir::CallInst>(ir_inst), ctx);
      break;
    case ir::ValueId::vMEMSET:
      lower(dyn_cast<ir::MemsetInst>(ir_inst), ctx);
      break;
    case ir::ValueId::vPHI:
      break;
    // case ir::ValueId::vATOMICRMW:
    //   lower(dyn_cast<ir::AtomicrmwInst>(ir_inst), ctx);
    // break;
    default:
      const auto valueIdEnumName = utils::enumName(static_cast<ir::ValueId>(ir_inst->valueId()));
      std::cerr << valueIdEnumName << ": not supported inst" << std::endl;
      assert(false && "not supported inst");
  }
}

void lower(ir::AtomicrmwInst* ir_inst, LoweringContext& ctx) {
  // TODO: support more atomicrmw inst and ordering
  assert(ir_inst->ordering() == ir::AtomicOrdering::SequentiallyConsistent);
  auto gcode = [opcode = ir_inst->opcode()] {
    switch (opcode) {
      case ir::BinaryOp::ADD:
        return InstAtomicAdd;
      case ir::BinaryOp::SUB:
        return InstAtomicSub;
      default:
        assert(false && "not supported atomicrmw inst");
        return InstUnreachable;
    }
  }();
  /* amoadd.w.aqrl $rd, $val, $addr*/
  auto dst = ctx.newVReg(ir_inst->type());
  auto addr = ctx.map2operand(ir_inst->ptr());
  auto val = ctx.map2operand(ir_inst->val());
  ctx.emitMIRInst(gcode, {dst, val, addr});
  ctx.addValueMap(ir_inst, dst);
}

/*
 * @brief: lower UnaryInst
 * @note:
 *    IR: ir_inst := value [valueId]
 *    -> MIR: InstXXX dst, src
 */
void lower(ir::UnaryInst* ir_inst, LoweringContext& ctx) {
  auto gc_instid = [scid = ir_inst->valueId()] {
    switch (scid) {
      case ir::ValueId::vFNEG:
        return InstFNeg;
      case ir::ValueId::vTRUNC:
        return InstTrunc;
      case ir::ValueId::vZEXT:
        return InstZExt;
      case ir::ValueId::vSEXT:
        return InstSExt;
        // return InstBitCast;
      case ir::ValueId::vFPTRUNC:
        assert(false && "not supported unary inst");
      case ir::ValueId::vFPTOSI:
        return InstF2S;
      case ir::ValueId::vSITOFP:
        return InstS2F;
      case ir::ValueId::vBITCAST:
      case ir::ValueId::vPTRTOINT:
      case ir::ValueId::vINTTOPTR:
        return InstBitCast;
      default:
        assert(false && "not supported unary inst");
    }
  }();

  if (gc_instid != InstBitCast) {
    auto dst = ctx.newVReg(ir_inst->type());
    ctx.emitMIRInst(gc_instid, {dst, ctx.map2operand(ir_inst->value())});
    ctx.addValueMap(ir_inst, dst);
  } else {
    /* 类型转换 -> 无需生成指令 (因为在lli系统中是强类型, 而在后端不是强类型) */
    ctx.addValueMap(ir_inst, ctx.map2operand(ir_inst->value()));
  }
}

/*
 * @brief: Lowering ICmpInst (int OR float)
 * @note:
 *    1. int
 *      IR: <result> = icmp <cond> <ty> <op1>, <op2>
 *      MIRGeneric: ICmp dst, src1, src2, op
 *    2. float
 *      IR: <result> = fcmp [fast-math flags]* <cond> <ty> <op1>, <op2>
 *      MIRGeneric: FCmp dst, src1, src2, op
 */
void lower(ir::ICmpInst* ir_inst, LoweringContext& ctx) {
  auto op = [scid = ir_inst->valueId()] {
    switch (scid) {
      case ir::ValueId::vIEQ:
        return CompareOp::ICmpEqual;
      case ir::ValueId::vINE:
        return CompareOp::ICmpNotEqual;
      case ir::ValueId::vISGT:
        return CompareOp::ICmpSignedGreaterThan;
      case ir::ValueId::vISGE:
        return CompareOp::ICmpSignedGreaterEqual;
      case ir::ValueId::vISLT:
        return CompareOp::ICmpSignedLessThan;
      case ir::ValueId::vISLE:
        return CompareOp::ICmpSignedLessEqual;
      default:
        assert(false && "not supported icmp inst");
    }
  }();

  const auto dst = ctx.newVReg(ir_inst->type());

  ctx.emitMIRInst(InstICmp, {dst, ctx.map2operand(ir_inst->lhs()), ctx.map2operand(ir_inst->rhs()),
                             MIROperand::asImm(static_cast<uint32_t>(op), OperandType::Special)});

  ctx.addValueMap(ir_inst, dst);
}
void lower(ir::FCmpInst* ir_inst, LoweringContext& ctx) {
  auto op = [scid = ir_inst->valueId()] {
    switch (scid) {
      case ir::ValueId::vFOEQ:
        return CompareOp::FCmpOrderedEqual;
      case ir::ValueId::vFONE:
        return CompareOp::FCmpOrderedNotEqual;
      case ir::ValueId::vFOGT:
        return CompareOp::FCmpOrderedGreaterThan;
      case ir::ValueId::vFOGE:
        return CompareOp::FCmpOrderedGreaterEqual;
      case ir::ValueId::vFOLT:
        return CompareOp::FCmpOrderedLessThan;
      case ir::ValueId::vFOLE:
        return CompareOp::FCmpOrderedLessEqual;
      default:
        assert(false && "not supported fcmp inst");
    }
  }();

  const auto dst = ctx.newVReg(ir_inst->type());
  ctx.emitMIRInst(InstFCmp, {dst, ctx.map2operand(ir_inst->lhs()), ctx.map2operand(ir_inst->rhs()),
                             MIROperand::asImm(static_cast<uint32_t>(op), OperandType::Special)});
  ctx.addValueMap(ir_inst, dst);
}

/* CallInst */
void lower(ir::CallInst* ir_inst, LoweringContext& ctx) {
  ctx.mTarget.getTargetFrameInfo().emitCall(ir_inst, ctx);
}

/*
 * @brief: lower BinaryInst
 * @note:
 *    IR: ir_inst := lValue, rValue [ValueId]
 *    -> MIR: InstXXX dst, src1, src2
 */
void lower(ir::BinaryInst* ir_inst, LoweringContext& ctx) {
  auto code = [scid = ir_inst->valueId()] {
    switch (scid) {
      case ir::ValueId::vADD:
        return InstAdd;
      case ir::ValueId::vFADD:
        return InstFAdd;
      case ir::ValueId::vSUB:
        return InstSub;
      case ir::ValueId::vFSUB:
        return InstFSub;
      case ir::ValueId::vMUL:
        return InstMul;
      case ir::ValueId::vFMUL:
        return InstFMul;
      case ir::ValueId::vUDIV:
        return InstUDiv;
      case ir::ValueId::vSDIV:
        return InstSDiv;
      case ir::ValueId::vFDIV:
        return InstFDiv;
      case ir::ValueId::vUREM:
        return InstURem;
      case ir::ValueId::vSREM:
        return InstSRem;
      default:
        assert(false && "not supported binary inst");
    }
  }();

  auto dst = ctx.newVReg(ir_inst->type());
  auto lhs = ir_inst->lValue();
  auto rhs = ir_inst->rValue();

  if (ir_inst->isCommutative() && lhs->isa<ir::ConstantValue>() && !rhs->isa<ir::ConstantValue>()) {
    std::swap(lhs, rhs);
  }
  /** isCommutative: cc, xc, xx */
  if (const auto crhs = rhs->dynCast<ir::ConstantValue>()) {
    if (crhs->isZero()) {
      switch (code) {
        case InstAdd:
        case InstSub:
        case InstFAdd:
        case InstFSub: {
          /* x + 0 = x */
          auto lhsOpernd = ctx.map2operand(lhs);
          ctx.addValueMap(ir_inst, lhsOpernd);
          return;
        }
        case InstMul:
        case InstFMul: {
          /* x * 0 = 0 */
          auto zeroOperand = MIROperand::asImm(0, OperandType::Int32);
          ctx.addValueMap(ir_inst, zeroOperand);
          return;
        }
        default:
          break;
      }
    } else if (crhs->isOne()) {
      switch (code) {
        case InstMul:
        case InstFMul: {
          /* x * 1 = x */
          auto lhsOpernd = ctx.map2operand(lhs);
          ctx.addValueMap(ir_inst, lhsOpernd);
          return;
        }
        default:
          break;
      }
    }
  }

  ctx.emitMIRInst(code,
                  {dst, ctx.map2operand(ir_inst->lValue()), ctx.map2operand(ir_inst->rValue())});
  ctx.addValueMap(ir_inst, dst);
}

/* BranchInst */
void emitJump(ir::BasicBlock* srcblock, ir::BasicBlock* dstblock, LoweringContext& lctx);

void lower(ir::BranchInst* ir_inst, LoweringContext& ctx) {
  /** mid end can guarantee that phi block can only be the reached by unconditional jump,
   * so conditional branch dont need to process phi insts.
   * unconditional branch use emitJum to process phi insts.
   */
  auto src_block = ir_inst->block();
  auto mir_func = ctx.currFunc();
  const auto codegen_ctx = ctx.codeGenctx;
  if (ir_inst->is_cond()) {  // conditional branch
    /*
        branch cond, iftrue, iffalse
        -> MIR
    preblock:
        ...
        branch cond, iftrue

    nextblock:
        jump iffalse

        ...
    */
    /* branch cond, iftrue */

    ctx.emitMIRInst(InstBranch,
                    {
                      ctx.map2operand(ir_inst->cond()) /* cond */,
                      MIROperand::asReloc(ctx.map2block(ir_inst->iftrue())) /* iftrue */,
                      MIROperand::asProb(0.5) /* prob*/
                    });
    // emitBranch(ir_inst->cond(), src_block, ir_inst->iftrue(), ctx);
    /* nextblock: jump iffalse */
    auto findBlockIter = [mir_func](const MIRBlock* block) {
      return std::find_if(
        mir_func->blocks().begin(), mir_func->blocks().end(),
        [block](const std::unique_ptr<MIRBlock>& mir_block) { return mir_block.get() == block; });
    };
    {
      /* insert new block after current block */
      auto curBlockIter = findBlockIter(ctx.currBlock());
      assert(curBlockIter != mir_func->blocks().end());

      auto newBlock = std::make_unique<MIRBlock>(
        ctx.currFunc(), "label" + std::to_string(codegen_ctx->nextLabelId()));
      auto newBlockPtr = newBlock.get();
      // insert new block after current block
      mir_func->blocks().insert(++curBlockIter, std::move(newBlock));
      ctx.setCurrBlock(newBlockPtr);
    }
    /* emit jump to iffalse */
    ctx.emitMIRInst(InstJump, {MIROperand::asReloc(ctx.map2block(ir_inst->iffalse()))});
    // emitJump(src_block, ir_inst->iffalse(), ctx);
    // emitJump(src_block, ir_inst->iffalse(), ctx);
  } else {  // unconditional branch
    auto dst_block = ir_inst->dest();
    emitJump(src_block, dst_block, ctx);
  }
}
void emitJump(ir::BasicBlock* srcblock, ir::BasicBlock* dstblock, LoweringContext& lctx) {
  // TODO: need to process phi insts
  std::vector<MIROperand> srcOperands;
  std::vector<MIROperand> dstOperands;

  for (auto inst : dstblock->insts()) {
    if (const auto phi = inst->dynCast<ir::PhiInst>()) {
      const auto val = phi->getvalfromBB(srcblock);

      if (val->isUndef()) continue;
      auto dumpOperand = [&](MIROperand op) {
        std::cerr << mir::GENERIC::OperandDumper{op} << std::endl;
      };

      srcOperands.push_back(lctx.map2operand(val));
      dstOperands.push_back(lctx.map2operand(phi));
    }
  }

  if (not srcOperands.empty()) {
    std::unordered_set<MIROperand, MIROperandHasher> needStagingRegister;
    std::unordered_set<MIROperand, MIROperandHasher> dstSet(dstOperands.begin(), dstOperands.end());
    for (auto op : srcOperands)
      if (dstSet.count(op)) needStagingRegister.insert(op);

    // setup phi values
    // calcuates the best order and create temporary variables for args
    // nodeMap: dstOperand -> idx
    std::unordered_map<MIROperand, uint32_t, MIROperandHasher> nodeMap;
    for (size_t idx = 0; idx < dstOperands.size(); ++idx)
      nodeMap.emplace(dstOperands[idx], idx);

    utils::Graph graph(dstOperands.size());  // direct copy graph

    for (size_t idx = 0; idx < dstOperands.size(); ++idx) {
      const auto arg = srcOperands[idx];  // corresponding srcOperand
      if (auto iter = nodeMap.find(arg); iter != nodeMap.cend()) {
        // copy b to a -> a should be resetted before b
        graph[idx].push_back(iter->second);
      }
    }

    const auto [ccnt, col] = utils::calcSCC(graph);
    auto order = utils::topologicalSort(graph, ccnt, col);

    assert(order.size() == dstOperands.size());
    std::unordered_map<MIROperand, MIROperand, MIROperandHasher> dirtyRegRemapping;

    for (size_t i = 0; i < dstOperands.size(); ++i) {
      const auto idx = order[i];
      MIROperand arg;
      if (auto iter = dirtyRegRemapping.find(srcOperands[idx]); iter != dirtyRegRemapping.cend()) {
        arg = iter->second;  // use copy
      } else {
        arg = srcOperands[idx];
      }
      const auto dstArg = dstOperands[idx];

      if (arg == dstArg) continue;  // identical copy

      // create copy
      if (needStagingRegister.count(dstOperands[idx])) {
        const auto intermediate = lctx.newVReg(srcOperands[idx].type());
        lctx.emitCopy(intermediate, dstArg);  // NOLINT
        dirtyRegRemapping.emplace(dstOperands[idx], intermediate);
      }

      // apply reset
      lctx.emitCopy(dstArg, arg);
    }
  }
  lctx.emitMIRInst(InstJump, {MIROperand::asReloc(lctx.map2block(dstblock))});
}

/*
 * @brief: lower LoadInst
 * @note:
 *    IR: inst := ptr [ValueId: vLOAD]
 *    -> MIR: InstLoad dst, src, align
 */
void lower(ir::LoadInst* ir_inst, LoweringContext& ctx) {
  const uint32_t align = 4;

  auto inst =
    ctx.emitMIRInst(InstLoad, {
                                ctx.newVReg(ir_inst->type()),                    /* dst */
                                ctx.map2operand(ir_inst->ptr()),                 /* src */
                                MIROperand::asImm(align, OperandType::Alignment) /* align*/
                              });

  ctx.addValueMap(ir_inst, inst->operand(0));
}

/*
 * @brief: lower StoreInst
 * @note:
 *    IR: inst := value, ptr [ValueId: vSTORE]
 *    -> MIR: InstStore addr, src, align
 */
void lower(ir::StoreInst* ir_inst, LoweringContext& ctx) {
  ctx.emitMIRInst(InstStore,
                  {
                    ctx.map2operand(ir_inst->ptr()),              // addr
                    ctx.map2operand(ir_inst->value()),            // src
                    MIROperand::asImm(4, OperandType::Alignment)  // align
                  });
}

/* ReturnInst */
void lower(ir::ReturnInst* ir_inst, LoweringContext& ctx) {
  ctx.mTarget.getTargetFrameInfo().emitReturn(ir_inst, ctx);
}

/*
 * @brief: lower MemsetInst
 * @note:
 *    memset(i8* <dest>, i8 <val>, i64 <len>, i1 <isvolatile>)
 * @details:
 *    NOTE: only support val = 0 and len is constant
 *    -> MIR: memset(dst, len)
 */
void lower(ir::MemsetInst* ir_inst, LoweringContext& ctx) {
  assert(ir_inst->val()->isa<ir::ConstantInteger>() and ir_inst->len()->isa<ir::ConstantInteger>());
  const auto val = ir_inst->val()->dynCast<ir::ConstantInteger>()->getVal();
  assert(val == 0);

  /* 通过寄存器传递参数 */
  // 1. 指针
  {
    auto val = ctx.map2operand(ir_inst->dst());
    auto dst = MIROperand::asISAReg(RISCV::X10, OperandType::Int64);
    ctx.emitCopy(dst, val);
  }

  // 2. 长度
  {
    auto len = ctx.map2operand(ir_inst->len());
    auto dst = MIROperand::asISAReg(RISCV::X11, OperandType::Int64);
    ctx.emitCopy(dst, len);
  }

  /* 生成跳转至被调用函数的指令 */
  ctx.emitMIRInst(RISCV::JAL, {MIROperand::asReloc(ctx.memsetFunc)});
}

/*
 * @brief: lower GetElementPtrInst for Pointer
 * @note:
 *    Pointer: <result> = getelementptr <type>, <type>* <ptrval>, i32 <idx>
 * @details: 指针运算
 *    1. 生成add AND mul指令来计算偏移量
 *    2. 生成add指令来计算得到目标指针地址
 */
void lower(ir::GetElementPtrInst* ir_inst, LoweringContext& ctx) {
  constexpr bool Debug = false;
  if (Debug) {
    std::cerr << "lower GetElementPtrInst for Pointer. \n";
    ir_inst->print(std::cerr);
    std::cerr << "\n";
  }

  auto base = ctx.map2operand(ir_inst->value());  // 基地址
  MIROperand ptr = base;
  auto btype = ir_inst->baseType();
  int stride = 1;
  if (btype->isArray()) {
    auto dims = dyn_cast<ir::ArrayType>(btype)->dims();
    for (int i = 0; i < dims.size(); i++) {
      stride *= dims[i];
    }
  }
  auto ir_index = ir_inst->index();

  if (auto ir_constant = dyn_cast<ir::ConstantValue>(ir_index)) {
    auto newPtr = ctx.newVReg(OperandType::Int64);
    ctx.emitMIRInst(
      InstAdd,
      {newPtr, ptr, MIROperand::asImm(4 * stride * ir_constant->i32(), OperandType::Int64)});
    ptr = newPtr;
  } else {
    auto newPtr_mul = ctx.newVReg(OperandType::Int64);
    ctx.emitMIRInst(InstMul, {newPtr_mul, ctx.map2operand(ir_index),
                              MIROperand::asImm(4 * stride, OperandType::Int64)});
    auto newPtr_add = ctx.newVReg(OperandType::Int64);
    ctx.emitMIRInst(InstAdd, {newPtr_add, ptr, newPtr_mul});
    ptr = newPtr_add;
  }

  ctx.addValueMap(ir_inst, ptr);
}
/*
 * @brief: lower GetElementPtrInst [begin, end) for Array
 * @note:
 *    Array: <result> = getelementptr <type>, <type>* <ptrval>, i32 0, i32 <idx>
 * @details:
 *    How to compute?
 *      - 我们遍历每一维度的下标索引, 直接在当前维度计算出当前维度的偏移量
 */
void lower_GetElementPtr(ir::inst_iterator begin, ir::inst_iterator end, LoweringContext& ctx) {
  constexpr bool Debug = false;
  if (Debug) {
    std::cerr << "lower GetElementPtrInst for Array. \n";
    auto iter = begin;
    while (iter != end) {
      auto ir_inst = dyn_cast<ir::GetElementPtrInst>(*iter);

      /* Instruction */
      std::cerr << "the instruction is: ";
      ir_inst->print(std::cerr);
      std::cerr << "\n";

      /* Attribute */
      std::cerr << "the attribute: \n";
      std::cerr << "\tindex is: " << ir_inst->index()->name() << "\n";
      std::cerr << "\tid is: " << ir_inst->getid() << "\n";

      std::cerr << "\n";
      iter++;
    }
  }
  auto base = ctx.map2operand(dyn_cast<ir::GetElementPtrInst>(*begin)->value());  // 基地址
  auto iter = begin;
  MIROperand ptr = base;

  auto ir_inst = dyn_cast<ir::GetElementPtrInst>(*iter);
  ir::Value* instEnd = *iter;  // GetElementPtr指令末尾 (包含)

  ir::Value* ir_offset = ir_inst->index();
  // bool is_constant = ir::isa<ir::ConstantValue>(ir_offset);
  bool is_constant = ir_offset->isa<ir::ConstantValue>();
  MIROperand mir_offset = ctx.map2operand(ir_offset);
  auto dims = ir_inst->cur_dims();
  /* 乘法运算 */
  for (int i = 1; i < dims.size(); i++) {
    if (is_constant) {
      auto ir_offset_constant = dyn_cast<ir::ConstantValue>(ir_offset);
      ir_offset = ir::ConstantInteger::gen_i32(ir_offset_constant->i32() * dims[i]);
      mir_offset = ctx.map2operand(ir_offset);
    } else {
      auto newPtr = ctx.newVReg(OperandType::Int64);
      ctx.emitMIRInst(InstMul,
                      {newPtr, mir_offset, MIROperand::asImm(dims[i], OperandType::Int64)});
      mir_offset = newPtr;
    }
  }
  /* 1. 偏移量 */
  {
    if (is_constant) {
      auto ir_offset_constant = dyn_cast<ir::ConstantValue>(ir_offset);
      ir_offset = ir::ConstantInteger::gen_i32(ir_offset_constant->i32() * 4);
      mir_offset = ctx.map2operand(ir_offset);
    } else {
      auto newPtr = ctx.newVReg(OperandType::Int64);
      ctx.emitMIRInst(InstShl, {newPtr, mir_offset, MIROperand::asImm(2, OperandType::Int64)});
      mir_offset = newPtr;
    }
  }

  /* 2. 指针运算 */
  {
    auto newPtr = ctx.newVReg(OperandType::Int64);
    ctx.emitMIRInst(InstAdd, {newPtr, ptr, mir_offset});
    ptr = newPtr;
    ctx.addValueMap(instEnd, ptr);
  }
  iter++;

  while (iter != end) {
    ir_inst = dyn_cast<ir::GetElementPtrInst>(*iter);
    dims = ir_inst->cur_dims();
    instEnd = *iter;
    ir_offset = ir_inst->index();
    is_constant = ir_offset->isa<ir::ConstantValue>();
    mir_offset = ctx.map2operand(ir_offset);

    /* 乘法运算 */
    for (int i = 1; i < dims.size(); i++) {
      if (is_constant) {
        auto ir_offset_constant = dyn_cast<ir::ConstantValue>(ir_offset);
        ir_offset = ir::ConstantInteger::gen_i32(ir_offset_constant->i32() * dims[i]);
        mir_offset = ctx.map2operand(ir_offset);
      } else {
        auto newPtr = ctx.newVReg(OperandType::Int64);
        ctx.emitMIRInst(InstMul,
                        {newPtr, mir_offset, MIROperand::asImm(dims[i], OperandType::Int64)});
        mir_offset = newPtr;
      }
    }

    /* 偏移量 */
    {
      if (is_constant) {
        auto ir_offset_constant = dyn_cast<ir::ConstantValue>(ir_offset);
        ir_offset = ir::ConstantInteger::gen_i32(ir_offset_constant->i32() * 4);
        mir_offset = ctx.map2operand(ir_offset);
      } else {
        auto newPtr = ctx.newVReg(OperandType::Int64);
        ctx.emitMIRInst(InstShl, {newPtr, mir_offset, MIROperand::asImm(2, OperandType::Int64)});
        mir_offset = newPtr;
      }
    }

    /* 指针运算 */
    {
      auto newPtr = ctx.newVReg(OperandType::Int64);
      ctx.emitMIRInst(InstAdd, {newPtr, ptr, mir_offset});
      ptr = newPtr;
      ctx.addValueMap(instEnd, ptr);
    }

    iter++;
  }
}
}  // namespace mir