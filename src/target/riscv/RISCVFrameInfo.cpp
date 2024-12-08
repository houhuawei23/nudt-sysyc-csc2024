#include "mir/MIR.hpp"
#include "mir/utils.hpp"
#include "target/riscv/RISCVTarget.hpp"
#include "autogen/riscv/InstInfoDecl.hpp"
#include "autogen/riscv/ISelInfoDecl.hpp"
#include "support/StaticReflection.hpp"
#include "support/arena.hpp"
namespace mir {
constexpr int32_t passingByRegBase = 0x100000;

/*
 * insert_prologue_epilogue: insert prologue and epilogue for a function
 * - using for allocateStackObjects, after register allocation, we know the
 * register to spill, so save callee-saved registers after jump to function, and
 * restore callee-saved registers before return.
 *
 * input:
 * - callee_saved_regs: a set of callee-saved registers
 * - return_addr_reg: the register used to store the return address
 *
 * process:
 * - allocate stack space for callee-saved registers
 * - allocate stack space for return address register
 * - prologue: save callee-saved registers to stack, store return address to
 * stack
 * - epilogue: load callee-saved registers from stack, restore return address
 * from stack
 *
 * return: 0?
 */
int32_t RISCVFrameInfo::insertPrologueEpilogue(MIRFunction* func,
                                               std::unordered_set<MIROperand, MIROperandHasher>& callee_saved_regs,
                                               CodeGenContext& ctx, MIROperand return_addr_reg) {
  // op -> stack
  std::vector<std::pair<MIROperand, MIROperand>> saved;

  /* find the callee saved registers */
  /* allocate stack space for callee-saved registers */
  {
    for (auto op : callee_saved_regs) {
      auto size = getOperandSize(
        ctx.registerInfo->getCanonicalizedRegisterType(op.type()));
      auto alignment = size;
      auto stack = func->newStackObject(ctx.nextId(), size, alignment, 0,
                                        StackObjectUsage::CalleeSaved);

      saved.emplace_back(op, stack);
    }
  }
  /* return address register */
  auto size = getOperandSize(return_addr_reg.type());
  auto alignment = size;
  auto stack = func->newStackObject(ctx.nextId(), size, alignment, 0,
                                    StackObjectUsage::CalleeSaved);
  saved.emplace_back(return_addr_reg, stack);

  /* insert the prologue and epilogue */
  {
    for (auto& block : func->blocks()) {
      auto& instructions = block->insts();

      // 1. 开始执行指令之前保存相关的调用者维护寄存器
      if (&block == &func->blocks().front()) {
        for (auto [op, stack] : saved) {
          auto inst = utils::make<MIRInst>(InstStoreRegToStack, {stack, op});
          instructions.push_front(inst);
        }
      }

      // 2. 函数返回之前将相关的调用者维护寄存器释放
      auto exit = instructions.back();
      auto& instInfo = ctx.instInfo.getInstInfo(exit);
      if (requireFlag(instInfo.inst_flag(), InstFlagReturn)) {
        auto pos = std::prev(instructions.end());
        for (auto [op, stack] : saved) {
          auto inst = utils::make<MIRInst>(InstLoadRegFromStack, {op, stack});
          instructions.insert(pos, inst);
        }
      }
    }
  }
  return 0;
}

/*
 * emit_call: emit instructions for a call instruction
 * using when lowering a ir call instruction to riscv
 *
 * input:
 * - inst: the call instruction
 * - lowering_ctx: the lowering context
 *
 * process:
 * - prepare args:
 *  - < 8 args: pass by registers
 *  - >= 8 args: pass by stack
 * - generate instructions to store arguments passed by stack
 * - generate instructions to assign arguments passed by registers
 * - generate instructions to call the callee function
 * - generate instructions to handle return value
 */
void RISCVFrameInfo::emitCall(ir::CallInst* inst, LoweringContext& lowering_ctx) {
  constexpr bool Debug = false;
  /* 1. 相关被调用函数 */
  auto irCalleeFunc = inst->callee();
  auto mirCalleeFunc = lowering_ctx.funcMap.at(irCalleeFunc);
  assert(mirCalleeFunc);

  /* 2. 计算参数的偏移量, 确定哪些参数通过寄存器传递, 哪些通过栈传递 */
  int32_t curOffset = 0;
  std::vector<int32_t> offsets;
  int32_t gprCount = 0, fprCount = 0;
  for (auto use : inst->rargs()) {
    auto arg = use->value();
    if (not arg->type()->isFloatPoint()) {
      if (gprCount < 8) {
        if (Debug) std::cerr << "gprCount: " << gprCount << std::endl;
        offsets.push_back(passingByRegBase + gprCount++);
        continue;
      }
    } else {
      if (fprCount < 8) {
        if (Debug) std::cerr << "fprCount: " << fprCount << std::endl;
        offsets.push_back(passingByRegBase + fprCount++);
        continue;
      }
    }

    /* 2.2 如果寄存器已满, 则计算栈上的位置, 更新当前栈偏移curOffset */
    int32_t size = arg->type()->size();
    int32_t align = 4;
    int32_t minimumSize = sizeof(uint64_t);
    size = std::max(size, minimumSize);
    align = std::max(align, minimumSize);

    /*
    栈对齐:
        1. curOffset + alignment - 1：
            首先, 将当前的栈偏移curOffset与参数的对齐字节数alignment相加,
    然后减去1 目的是为了确保即使在加上当前参数的大小后, 栈地址仍然在对齐边界上
        2. (curOffset + alignment - 1) / alignment:
            接着, 将上一步的结果除以alignment
            这个操作会得到一个大于或等于原栈偏移的值,
    且这个值是alignment的整数倍 这意味着, 无论当前栈偏移是多少,
    除以对齐字节数后, 都会得到一个对齐的地址
        3. / alignment * alignment:
        最后, 将上一步的结果再乘以alignment
        这样做是为了确保栈偏移量是alignment的整数倍, 满足参数的对齐要求。
    */
    curOffset = (curOffset + align - 1) / align * align;
    offsets.push_back(curOffset);
    curOffset += size;
  }

  auto mirFunc = lowering_ctx.currBlock()->parent();
  /* 为通过栈传递的参数分配栈空间, 并生成相应的存储指令 */
  for (uint32_t idx = 0; idx < inst->rargs().size(); idx++) {
    auto arg = inst->operand(idx);
    const auto offset = offsets[idx];
    auto val = lowering_ctx.map2operand(arg);
    auto size = arg->type()->size();
    const auto align = 4;

    if (offset < passingByRegBase) { /* pass by stack */
      auto obj =
        mirFunc->newStackObject(lowering_ctx.codeGenctx->nextId(), size, align,
                                offset, StackObjectUsage::CalleeArgument);
      // copy val to reg, then store reg to stack
      if (!isOperandVRegORISAReg(val)) {
        auto reg = lowering_ctx.newVReg(val.type());
        lowering_ctx.emitCopy(reg, val);
        val = reg;
      }
      lowering_ctx.emitMIRInst(InstStoreRegToStack, {obj, val});
    }
  }
  /* prepare args: 为通过寄存器传递的参数生成相应的寄存器赋值指令 */
  for (uint32_t idx = 0; idx < inst->rargs().size(); idx++) {
    auto arg = inst->operand(idx);
    const auto offset = offsets[idx];
    auto val = lowering_ctx.map2operand(arg);
    if (offset >= passingByRegBase) { /* pass by reg */
      MIROperand dst;
      if (isFloatType(val.type())) {
        dst = MIROperand::asISAReg(
          RISCV::F10 + static_cast<uint32_t>(offset - passingByRegBase),
          OperandType::Float32);
      } else {
        dst = MIROperand::asISAReg(
          RISCV::X10 + static_cast<uint32_t>(offset - passingByRegBase),
          OperandType::Int64);
      }
      assert(dst.isInit());
      lowering_ctx.emitCopy(dst, val);
    }
  }
  /* 生成跳转至被调用函数的指令。*/
  lowering_ctx.emitMIRInst(RISCV::JAL, {MIROperand::asReloc(mirCalleeFunc)});

  const auto irRetType = inst->callee()->retType();

  /* 函数返回值的处理 */
  if (irRetType->isVoid()) return;
  auto retReg = lowering_ctx.newVReg(irRetType);
  MIROperand val;
  if (irRetType->isFloatPoint()) { /* return by $fa0 */
    val = MIROperand::asISAReg(RISCV::F10, OperandType::Float32);
  } else { /* return by $a0 */
    val = MIROperand::asISAReg(RISCV::X10, OperandType::Int64);
  }
  lowering_ctx.emitCopy(retReg, val);
  lowering_ctx.addValueMap(inst, retReg);
}

/*
 * FrameInfo::emit_prologue: emit instructions for function prologue
 * - using for lowering from ir to mir funtion, after ir args are mapped to
 * vreg, these args(vregs) is passed by isa register or stack, we need to
 * determine.
 *
 * then generate instructions:
 * - by isa regs: copy isa reg to corresponding vreg
 * - by stack: load vreg from stack
 */
void RISCVFrameInfo::emitPrologue(MIRFunction* func, LoweringContext& lowering_ctx) {
  const auto& args = func->args();
  int32_t curOffset = 0;
  /* off >= passingByGPR: passing by reg[off - passingByRegBase] */
  std::vector<int32_t> offsets;
  int32_t gprCount = 0, fprCount = 0;

  /* traverse args, split into reg/stack */
  for (auto& arg : args) {
    if (isIntType(arg.type())) {
      if (gprCount < 8) {
        offsets.push_back(passingByRegBase + gprCount++);
        continue;
      }
    } else {
      if (fprCount < 8) {
        offsets.push_back(passingByRegBase + fprCount++);
        continue;
      }
    }

    /* pass by stack */
    int32_t size = getOperandSize(arg.type());
    int32_t align = 4;  // TODO: check alignment
    int32_t minimumSize = sizeof(uint64_t);
    size = std::max(size, minimumSize);
    align = std::max(align, minimumSize);
    curOffset = (curOffset + align - 1) / align * align;
    offsets.push_back(curOffset);
    curOffset += size;
  }
  /* pass by register, generate copy from isa reg to vreg */
  for (uint32_t idx = 0; idx < args.size(); idx++) {
    const auto offset = offsets[idx];
    const auto& arg = args[idx];
    if (offset >= passingByRegBase) {
      /* $a0-$a7, $f0-$f7 */
      MIROperand src;
      if (isFloatType(arg.type())) {
        src = MIROperand::asISAReg(
          RISCV::F10 + static_cast<uint32_t>(offset - passingByRegBase),
          OperandType::Float32);
      } else {
        src = MIROperand::asISAReg(
          RISCV::X10 + static_cast<uint32_t>(offset - passingByRegBase),
          OperandType::Int64);
      }
      assert(src.isInit());
      // copy isa reg(src) to vreg(arg)
      lowering_ctx.emitCopy(arg, src);
    }
  }
  /* pass by stack, generate load from stack */
  for (uint32_t idx = 0; idx < args.size(); idx++) {
    const auto offset = offsets[idx];
    const auto& arg = args[idx];
    const auto size = getOperandSize(arg.type());
    const auto align = 4;  // TODO: check alignment
    if (offset < passingByRegBase) {
      auto obj =
        func->newStackObject(lowering_ctx.codeGenctx->nextId(), size, align,
                             offset, StackObjectUsage::Argument);
      // load vreg(arg) from stack(obj)
      lowering_ctx.emitMIRInst(InstLoadRegFromStack, {arg, obj});
    }
  }
}

/*
 * FrameInfo::emit_epilogue: emit instructions for function epilogue
 * using for lowering ir return inst to riscv return inst.
 * gen riscv return inst:
 * - return void: ret
 * - return int: move $a0/$f0, retval; ret
 */
void RISCVFrameInfo::emitReturn(ir::ReturnInst* ir_inst, LoweringContext& lowering_ctx) {
  if (not ir_inst->operands().empty()) {
    // has return value
    auto retval = ir_inst->returnValue();
    if (retval->type()->isFloatPoint()) {
      /* return by $fa0 */
      lowering_ctx.emitCopy(
        MIROperand::asISAReg(RISCV::F10, OperandType::Float32),
        lowering_ctx.map2operand(retval));
    } else if (retval->type()->isInt()) {
      /* return by $a0 */
      lowering_ctx.emitCopy(
        MIROperand::asISAReg(RISCV::X10, OperandType::Int64),
        lowering_ctx.map2operand(retval));
    }
  }
  lowering_ctx.emitMIRInst(RISCV::RET);
}

}  // namespace mir