code gen optimization (maybe):
- createMIRFunction: lowering getelementptr inst
- 


InstSelection template:

parse_ise_item



mir:
MIRRelocable: class
    MIRFunction
    MIRBasicBlock
    MIRZeroStorage
    MIRDataStorage
    MIRJumpTable

MIRregister struct, MIRRegisterFlag enum

MIRInst class, MIRGenericInst enum
    MIROperand class, OperandType enum
    

StackObject struct, StackObjectUsage enum

MIRModule class
    Target
    MIRGlobal: MIRRelocable*

target.hpp:

Target:
    DataLayout
    TargetScheduleModel
    TargetInstInfo
    TargetSelInfo
    TargetFrameInfo
    TargetRegisterInfo

CodeGenContext struct:
    Target
    DataLayout
    TargetScheduleModel
    TargetInstInfo
    TargetSelInfo
    TargetFrameInfo
    TargetRegisterInfo
    MIRFlags
    idx, next_idx()

datalayout.hpp:
DataLayout class:
    getEndian(), enum Endian
    getBuiltinAlignment()
    getPointerSize()
    getCodeAlignment()
    getStorageAlignment()

schedulemodel.hpp:

ScheduleClass class
MicroarchitectureInfo struct
TargetScheduleModel class:
    getInstScheClass(opcode)
    getInfo() MicroarchitectureInfo
    peepholeOpt(mirfunc, codegenctx)
    isExpensiveInst()
ScheduleState class:
    ??

instinfo.hpp:

OperandFlag enum: use, def, metadata
InstFlag enum: xxx
InstInfo class:
    print() 
    getOperandNum()
    getOperandFlag()
    getInstFlag()
    getUniqueName()
TargetInstInfo class:
    getInstInfo
    matchBranch
    matchConditionalBranch
    matchUnconditionalBranch
    redirectBranch
    emitGoto
    inverseBranch
    getAddressingImmRange

iselinfo.hpp
ISelContext class
InstLegalizeContext
TargetISelInfo class

frameinfo.hpp
TargeFrameInfo class:
    lowering stage
        emitCall
        emitPrologue
        emitReturn
    ra stage
    sa stage

target.riscv:


RISCV:
- instinfodecl.hpp:
  - enum RISCVInst
  - getRISCVInstInfo()
- instinfoimpl.hpp:
  - RISCVInstInfoXXX classes
  - RISCVInstInfo class 
  - getRISCVInstInfo()
- instselinfodecl.hpp:
  - getRISCVISelInfo()
- instselinfopimpl.hpp:
  - matchInstXXX(): match Generic Inst
  - matchAndSelectPatternXXX(): match Generic Inst and select RISCV isa
  - matchAndSelectImpl(): switch opcode on matchAndSelectPatternXXX()
  - RISCVISelInfo class: public TargetISelInfo
    - matchAndSelect(): call matchAndSelectImpl()
  - getRISCVISelInfo()
- scheduleModeldecl.hpp:
  - getRISCVScheduleModel()
- impl.hpp:
  - class RISCVScheduleModel_sifive_u74: public TargetScheduleModel
    - getInstScheClass(opcode)
    - getInfo()
    - peepholeOpt(mir_func, codegen_ctx)
    - isExpensiveInst()
  - getRISCVScheduleModel()
- riscv.hpp:
  - enum RISCVRegister{}
  - isOperandXXX(): check operand type for isel
- RISCVTarget.cpp:
  - class RISCVDataLayout: public DataLayout
  - class RISCVFrameInfo: public TargetFrameInfo
  - class RISCVRigisterInfo: public TargetRegisterInfo
  - class RISCVTarget: public Target
    - RISCVDataLayout mDataLayout;
    - RISCVFrameInfo mFrameInfo;
    - RISCVRegisterInfo mRegisterInfo;
- RISCVInstInfo.cpp:
  - getRISCVGPRTextualName()
  - operator<<(out, operand)
  - emitGotoImpl()
  - inverseBranchImpl
  - getAddressingImmRangeImpl
- RISCVISelInfo.cpp:
  - getXXXXOpcode()
  - selectXXXX()
  - legalizeInst()
- RISCVScheduleModel.cpp:
  - template class RISCVScheduleClassXXXXX: public ScheduleClass
    - schedule(state, inst, instinfo)
  - 
Global Value:

- has init: .data
- not init: .bss
- const: .rodata

Function:

- register allocation
- 为该function使用的每一个global value建立local label
- 检查是否有子函数调用
- 完成函数entry/头部工作？
- 遍历bb
- 完成尾部 exit?

StackTable符号表：记录每个local value在栈上的位置，每次都load store

Function 头部工作：
- 在.text段生成该Function的FunctionLabel以及.type等汇编伪指令
- 进入该Function后，保存上一级Function的FP
- 通过上一级Function的SP设置该Function的FP
- 更新该Function的SP (即开辟栈空间)

开辟函数栈空间
- sub sp, sp, #n
- 多大空间，需要计算

Function 尾部
- 恢复上一级Function的FP, SP
- 返回上一级Function

为所有basicblock生成汇编
- Label
- ir inst -> mir inst -> asm inst
- one by one translation

初始化局部变量和常数
- 常数在可表示的立即数范围内
- 超出范围
- 3 在text段构建常量池放置常数，使用时加载
  - `LDR <locallabel>`
  - 在LocalLabel处存放了常量，`<LocalLabel>` 需在ldr的寻址范围内（4KB）

## nanke ir->mir

```c++
for (all global in irmodule.globals){
    if (is function) {
        if (not has body): 
        else: 
    }
    else {
        if (has init value) {
            // .data
            // MIRDataStorage:: Storage data init
        }
        else {
            // .bss
            // MIRZeroStorage
        } 
    }

}
        
```

## cmmc Lowering

IR Module -> MIR Module, for Target
- input: IR::Module ir_module
- craete empty MIRModule mir_module
- for all IR::Function, create empty MIRFunction
- for all IR::GlobalVariable, create MIRGlobalObject and add to mir_module.global_objs
- for all IR::Function ir_func:
  - **fullfill corresponding MIRFunction mir_func**: 
    - create_mir_function(ir_func, mir_func)
    - IR::Function -> MIRFunction, MIRInst with GenericMIRInst opcode
    - MIRInst has MIROperand (VirtualRegister, Constant, MIRRelocable[MIRFunction/MIRBlock/Storage])
    - 
  - **Instruction Selection on mir_func**:
    - ISelContex.run_isel(mir_func)
    - MIRFunction (GenericMIRInst opcode) -> MIRFunction (RISCVInst opcode)
  - **Register Allocation on mir_func**:
    - MIROperand(VirtualRegister) -> MIROperand(PhysicalRegister: RISCV)
  - **Stack Allocation on mir_func**:
    - determine stack offset, and leglize all instructions that use stack objects
  - **Post Legalize on mir_func**: 
  - 