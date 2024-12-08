
## Target-Independent Code Generatator

[doc](https://llvm.org/docs/CodeGenerator.html)

The high-level design of the code generator

The LLVM target-independent code generator is designed to support efficient and quality code generation for standard register-based microprocessors. Code generation in this model is divided into the following stages:

LLVM 与目标无关的代码生成器旨在支持基于标准寄存器的微处理器的高效且高质量的代码生成。
该模型中的代码生成分为以下阶段：

- **Instruction Selection** — This phase determines an efficient way to express the **input LLVM code** in the **target instruction set**. This stage produces the initial code for the program in the target instruction set, then makes use of **virtual registers** in SSA form and **physical registers** that represent any required register assignments due to target constraints or calling conventions. **This step turns the LLVM code into a DAG of target instructions.**
- **Scheduling and Formation** — This phase takes the DAG of target instructions produced by the instruction selection phase, determines an ordering of the instructions, then emits the instructions as **MachineInstr**s with that ordering. Note that we describe this in the instruction selection section because it operates on a SelectionDAG.
- **SSA-based Machine Code Optimizations** — This optional stage consists of a series of machine-code optimizations that operate on the SSA-form produced by the instruction selector. Optimizations like **modulo-scheduling** or **peephole optimization** work here.
- **Register Allocation** — The target code is transformed from an **infinite virtual register file in SSA form** to the **concrete register file used by the target**. This phase introduces **spill code** and eliminates all **virtual register references** from the program.
- **Prolog/Epilog Code Insertion** — Once the machine code has been generated for the function and the amount of stack space required is known (used for LLVM alloca’s and spill slots), the prolog and epilog code for the function can be inserted and **“abstract stack location references”** can be eliminated. This stage is responsible for implementing optimizations like frame-pointer elimination and stack packing.
- **Late Machine Code Optimizations** — Optimizations that operate on “final” machine code can go here, such as spill code scheduling and peephole optimizations.
- **Code Emission** — The final stage actually puts out the code for the current function, either in the target assembler format or in machine code.


- **指令选择**——此阶段确定在 **目标指令集** 中表达输入 LLVM 代码的有效方式。此阶段为目标指令集中的程序生成初始代码，然后使用 SSA 形式的**虚拟寄存器** 和 表示由于目标约束或调用约定而需要的任何寄存器分配的**物理寄存器**。此步骤将 LLVM 代码转换为目标指令的 DAG。
- **调度和形成** - 此阶段采用指令选择阶段生成的目标指令的 DAG，确定指令的顺序，然后按照该顺序将指令作为 **MachineInstr**s 发出。请注意，我们在指令选择部分对此进行了描述，因为它在 SelectionDAG 上运行。
- **基于 SSA 的机器代码优化** — 此可选阶段由一系列机器代码优化组成，这些优化对指令选择器生成的 SSA 形式进行操作。**模调度**或**窥孔优化**等优化在这里发挥作用。
- **寄存器分配** ——目标代码从 **SSA形式的无限虚拟寄存器文件** 转换为 **目标机器使用的具体寄存器文件**。此阶段引入 **溢出代码** 并消除程序中的所有 **虚拟寄存器引用**。
- **Prolog/Epilog 代码插入** - 一旦为函数生成了机器代码 并且 知道了所需的 **堆栈空间量**（用于 LLVM 分配和溢出槽），就可以插入函数的 **Prolog** 和 **Epilog** 代码，并且 **“抽象堆栈位置参考”** 可以被消除。此阶段负责实现 **帧指针消除** 和 **堆栈打包** 等优化。
- **后期机器代码优化** ——对“最终”机器代码进行的优化可以放在此处，例如**溢出代码调度**和**窥孔优化**。**代码发射**——最后阶段实际上以**目标汇编程序格式**或机器代码的形式输出当前函数的代码。

The code generator is based on the assumption that the instruction selector will use an **optimal pattern matching selector** to create high-quality sequences of native instructions. Alternative code generator designs based on **pattern expansion** and **aggressive iterative peephole optimization** are much slower. This design permits efficient compilation (important for JIT environments) and aggressive optimization (used when generating code offline) by allowing components of varying levels of sophistication to be used for any step of compilation.

代码生成器基于这样的假设：指令选择器将使用最佳模式匹配选择器来创建高质量的本机指令序列。基于模式扩展和积极的迭代窥孔优化的替代代码生成器设计要慢得多。这种设计允许将不同复杂程度的组件用于任何编译步骤，从而实现高效编译（对于 JIT 环境很重要）和积极优化（离线生成代码时使用）。

In addition to these stages, target implementations can insert arbitrary target-specific passes into the flow. For example, the X86 target uses a special pass to handle the 80x87 floating point stack architecture. Other targets with unusual requirements can be supported with custom passes as needed.
除了这些阶段之外，目标实现还可以将任意特定于目标的通道插入流程中。例如，X86 目标使用特殊通道来处理 80x87 浮点堆栈架构。其他具有特殊要求的目标可以根据需要通过自定义通道来支持。

- Target Description Classes
  - [The ](https://llvm.org/docs/CodeGenerator.html#the-targetmachine-class)[`TargetMachine`](https://llvm.org/docs/CodeGenerator.html#the-targetmachine-class)[ class](https://llvm.org/docs/CodeGenerator.html#the-targetmachine-class)
  - [The ](https://llvm.org/docs/CodeGenerator.html#the-datalayout-class)[`DataLayout`](https://llvm.org/docs/CodeGenerator.html#the-datalayout-class)[ class](https://llvm.org/docs/CodeGenerator.html#the-datalayout-class)
  - [The ](https://llvm.org/docs/CodeGenerator.html#the-targetlowering-class)[`TargetLowering`](https://llvm.org/docs/CodeGenerator.html#the-targetlowering-class)[ class](https://llvm.org/docs/CodeGenerator.html#the-targetlowering-class)
  - [The ](https://llvm.org/docs/CodeGenerator.html#the-targetregisterinfo-class)[`TargetRegisterInfo`](https://llvm.org/docs/CodeGenerator.html#the-targetregisterinfo-class)[ class](https://llvm.org/docs/CodeGenerator.html#the-targetregisterinfo-class)
  - [The ](https://llvm.org/docs/CodeGenerator.html#the-targetinstrinfo-class)[`TargetInstrInfo`](https://llvm.org/docs/CodeGenerator.html#the-targetinstrinfo-class)[ class](https://llvm.org/docs/CodeGenerator.html#the-targetinstrinfo-class)
  - [The ](https://llvm.org/docs/CodeGenerator.html#the-targetframelowering-class)[`TargetFrameLowering`](https://llvm.org/docs/CodeGenerator.html#the-targetframelowering-class)[ class](https://llvm.org/docs/CodeGenerator.html#the-targetframelowering-class)
  - [The ](https://llvm.org/docs/CodeGenerator.html#the-targetsubtarget-class)[`TargetSubtarget`](https://llvm.org/docs/CodeGenerator.html#the-targetsubtarget-class)[ class](https://llvm.org/docs/CodeGenerator.html#the-targetsubtarget-class)
  - [The ](https://llvm.org/docs/CodeGenerator.html#the-targetjitinfo-class)[`TargetJITInfo`](https://llvm.org/docs/CodeGenerator.html#the-targetjitinfo-class)[ class](https://llvm.org/docs/CodeGenerator.html#the-targetjitinfo-class)
- Machine Code Description Classes
  - MachineInstr
  - MachineBasicBlock
  - MAchineFunciton
  - MAchineInstr_Bundles
- The 'MC' Layer
  - MCStream
  - MCContext
  - MCSymbol
  - MCSection
  - MCInst
  - Object File Format
- Target-Independent Code Generation Algorithms
  - Instructin Selection
  - SSA-base MC Op
  - Live Intervals
  - Register Allocation
  - Prolog/Epilog Code Insertion
  - Compact Unwind
  - Late Machine Code Optimizations
  - Code Emission


## MC layer
MC 层用于在原始机器代码级别表示和处理代码，缺乏“常量池”、“跳转表”、“全局变量”或类似信息等“高级”信息。在此级别，LLVM 处理标签名称、机器指令和目标文件中的部分等内容。该层中的代码有许多重要用途：代码生成器的尾部使用它来编写 .s 或 .o 文件，并且 llvm-mc 工具也使用它来实现独立的机器代码汇编器和反汇编程序。

## Target-independent code generation algorithms¶ 与目标无关的代码生成算法 ¶


### SelectionDAG ISP

SelectionDAG-based instruction selection consists of the following steps:
基于SelectionDAG的指令选择包括以下步骤：

- Build initial DAG — This stage performs a simple translation from the input LLVM code to an illegal SelectionDAG.
- Optimize SelectionDAG — This stage performs simple optimizations on the SelectionDAG to simplify it, and recognize meta instructions (like rotates and div/rem pairs) for targets that support these meta operations. This makes the resultant code more efficient and the select instructions from DAG phase (below) simpler.
- Legalize SelectionDAG Types — This stage transforms SelectionDAG nodes to eliminate any types that are unsupported on the target.
- Optimize SelectionDAG — The SelectionDAG optimizer is run to clean up redundancies exposed by type legalization.
- Legalize SelectionDAG Ops — This stage transforms SelectionDAG nodes to eliminate any operations that are unsupported on the target.
- Optimize SelectionDAG — The SelectionDAG optimizer is run to eliminate inefficiencies introduced by operation legalization.
- Select instructions from DAG — Finally, the target instruction selector matches the DAG operations to target instructions. This process translates the target-independent input DAG into another DAG of target instructions.
- SelectionDAG Scheduling and Formation — The last phase assigns a linear order to the instructions in the target-instruction DAG and emits them into the MachineFunction being compiled. This step uses traditional prepass scheduling techniques.

- **构建初始 DAG** — 此阶段执行从输入 LLVM 代码到非法 SelectionDAG 的简单转换。
- **优化 SelectionDAG** — 此阶段对 SelectionDAG 执行简单的优化以简化它，并识别支持这些元操作的目标的元指令（如旋转和 div / rem 对）。这使得生成的代码更加高效，并且 DAG 阶段（如下）的选择指令更加简单。
- **合法化 SelectionDAG 类型** — 此阶段转换 SelectionDAG 节点以消除目标上不支持的任何类型。
- **优化 SelectionDAG** — 运行 SelectionDAG 优化器来清理类型合法化所暴露的冗余。
- **合法化 SelectionDAG 操作** — 此阶段转换 SelectionDAG 节点以消除目标上不支持的任何操作。
- **优化 SelectionDAG** — 运行 SelectionDAG 优化器是为了消除操作合法化带来的低效率。
- **从DAG中选择指令** ——最后，目标指令选择器将DAG操作与目标指令相匹配。此过程将与目标无关的输入 DAG 转换为目标指令的另一个 DAG。
- **SelectionDAG 调度和形成**——最后一个阶段为目标指令 DAG 中的指令分配线性顺序，并将它们发送到正在编译的 MachineFunction 中。此步骤使用传统的预通过调度技术。


指令选择模式匹配