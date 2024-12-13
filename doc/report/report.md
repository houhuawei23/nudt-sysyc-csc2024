## SysYCompiler 编译器设计文档 & 实验报告

实习简介与要求:

学习编译器的基本结构、编译原理，掌握从零开始构建编译器的方法，使用已有的编译前端自动生成工具构建词法及语法分析模块，并生成抽象语法树；设计合适的中间表示，在中间表示层分析控制流、数据流、数据依赖等信息，进行标量优化，识别循环并对其进行变换，进行自动并行化和向量化，生成更高质量的代码；针对ARM/RISCV指令集进行指令选择、寄存器分配，从而生成目标代码。要求对编译方向感兴趣，熟悉C++语言开发。

### 简介

<p align="center"> <img src="../pics/workflow.png" width = 60%/>

1. 前端：
   1. 采用 ANTLR4 C++ Lib 进行词法、语法分析，生成抽象语法树（AST）
2. 中端：自定义 SSA IR
   1. 利用 ANTLR Visitor 语法树遍历模式，发射自定义 SSA IR
   2. 自定义 IR 可 序列化 （Serialization）为与 LLVM IR 兼容的可读格式，并利用 LLVM 相关组件进行测试分析
   3. 在 IR 上进行机器无关优化，包括标量优化、循环优化、过程间优化等
3. 后端：MIR 框架，可兼容 自定义 GenericInst 与 TargetInst
   1. 降级（Lower）：由 IR 到 MIR GenericInst
   2. 指令选择（InstSelect）：由 GenericInst 到 TargetInst
   3. 在 MIR 上进行优化：窥孔优化，指令调度，块调度，块化简等
   4. 寄存器分配与栈分配
   5. MIR 序列化为 汇编代码（AsmCode），在 qemu 与 开发板上测试

### 前端

### 中端

#### 分析遍

#### 优化遍

### 后端
