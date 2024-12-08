# Pass


## Pass Manager

LLVM: PassManager

[doxygen](https://llvm.org/doxygen/classllvm_1_1PassManager.html#:~:text=A%20pass%20manager%20contains%20a,block%20of%20a%20pass%20pipeline.)

Manages a sequence of passes over a particular unit of IR.

A pass manager contains **a sequence of passes** to run over a particular unit of IR (e.g. Functions, Modules). It is itself a valid **pass** over that unit of IR, and when run over some given IR will run each of its contained passes in sequence. Pass managers are the primary and most basic building block of a pass pipeline.

When you run a pass manager, you provide an AnalysisManager<IRUnitT> argument. The pass manager will propagate that analysis manager to each pass it runs, and will call the analysis manager's invalidation routine with the PreservedAnalyses of each pass it runs.


## Pass

[doxygen](https://llvm.org/doxygen/classllvm_1_1Pass.html#details)

### LLVM: Pass

Pass interface - Implemented by all 'passes'.

Subclass this if you are an interprocedural optimization or you do not fit into any of the more constrained passes described below.



Derived classes:
- **CallGraphSCCPass**: Strongly Connected Components
  - Pass that operates BU on call graph (BU: Bottom Up)
- **FunctionPass**: Implement most global optimizations.
  - Optimizations should subclass this class if they meet the following constraints:
    - Optimizations are organized globally, i.e., a function at a time
    - Optimizing a function does not cause the addition or removal of any functions in the module
- **LoopPass**: All loop optimization and transformation passes are derived from LoopPass.
- **ModulePass**: Used to implement unstructured interprocedural optimizations and analyses.
  - ModulePasses may do anything they want to the program.
- **RegionPass**: A pass that runs on each Region in a function.


### FunctionPass


### CallGraphSCCPass

This file defines the CallGraphSCCPass class, which is used for passes which
are implemented as bottom-up traversals on the call graph.  Because there may
be cycles in the call graph, passes of this type operate on the call-graph in
SCC order: that is, they process function bottom-up, except for recursive
functions, which they process all at once.

These passes are inherently interprocedural, and are required to keep the
call graph up-to-date if they do anything which could modify it.


### RegionPass
RegionInfo.h

Calculate a program structure tree built out of single entry single exit
regions.
The basic ideas are taken from "The Program Structure Tree - Richard Johnson,
David Pearson, Keshav Pingali - 1994", however enriched with ideas from "The
Refined Process Structure Tree - Jussi Vanhatalo, Hagen Voelyer, Jana
Koehler - 2009".
The algorithm to calculate these data structures however is completely
different, as it takes advantage of existing information already available
in (Post)dominace tree and dominance frontier passes. This leads to a simpler
and in practice hopefully better performing algorithm. The runtime of the
algorithms described in the papers above are both linear in graph size,
O(V+E), whereas this algorithm is not, as the dominance frontier information
itself is not, but in practice runtime seems to be in the order of magnitude
of dominance tree calculation.



## Analysis and Transform in Passes


本文档是 LLVM 提供的优化功能的高级摘要。优化被实现为遍历程序的某些部分以收集信息或转换程序的通道。下表将 LLVM 提供的 pass 分为三类。

- 分析遍 Analysis Pass 计算其他过程可以使用或用于调试或程序可视化目的的信息。
- 变换遍 Transform passes 可以使用分析遍，或导致分析遍无效。转换过程都会以某种方式改变程序。
- Utility passes 提供了一些实用性，但不适合分类。例如，将函数提取到位码或将模块写入位码的过程既不是分析过程，也不是转换过程。

### Analysis Passes

- 别名分析
- 调用图构建
- 依赖分析
- Dominance Frontier Construction¶
- globals-aa: Simple mod/ref analysis for globals¶
- instcount: Counts the various types of Instructions
- loops: Natural Loop Information
- memdep: Memory Dependence Analysis
- postdomtree: Post-Dominator Tree Construction
- scalar-evolution: Scalar Evolution Analysis
  - 用于对循环中的标量表达式进行分析和分类。
  - This analysis is primarily useful for induction variable substitution and strength reduction.
- scev-aa: ScalarEvolution-based Alias Analysis
  

### Transform Passes

- adce: Aggressive Dead Code Elimination
- always-inline: Inliner for always_inline functions
- argpromotion: Promote ‘by reference’ arguments to scalars
- codegenprepare: Optimize for code generation
- constmerge: Merge Duplicate Global Constants
- dce: Dead Code Elimination¶
  - 死代码消除与死指令消除类似，但它会重新检查已删除指令所使用的指令，以查看它们是否是新的死指令。
- deadargelim: Dead Argument Elimination
- dse: Dead Store Elimination
- globaldce: Dead Global Elimination
- inline: Function Integration/Inlining
- instcombine: Combine redundant instructions
- licm: Loop Invariant Code Motion
- loop-deletion: Delete dead loops¶
- loop-extract: Extract loops into new functions
- loop-unroll: Unroll loops
- **mem2reg**: Promote Memory to Register
- reg2mem: Demote all values to stack slots¶
- 

#### mem2reg

This file promotes memory references to be register references. It promotes alloca instructions which only have loads and stores as uses. An alloca is transformed by using dominator frontiers to place phi nodes, then traversing the function in depth-first order to rewrite loads and stores as appropriate. This is just the standard SSA construction algorithm to construct “pruned” SSA form.

该文件将内存引用提升为寄存器引用。它提倡仅将加载和存储用作用途的分配指令。 alloca 通过使用支配边界放置 phi 节点进行转换，然后以深度优先顺序遍历函数以根据需要重写加载和存储。这只是构建“修剪”SSA 形式的标准 SSA 构建算法。



### Utility Passes

- verify: Module Verifier

#### verify: Module Verifier

Verifies an LLVM IR code. This is useful to run after an optimization which is undergoing testing. Note that llvm-as verifies its input before emitting bitcode, and also that malformed bitcode is likely to make LLVM crash. All language front-ends are therefore encouraged to verify their output before performing optimizing transformations.
验证 LLVM IR 代码。这对于在正在进行测试的优化之后运行非常有用。请注意，llvm-as 在发出位码之前会验证其输入，而且格式错误的位码可能会导致 LLVM 崩溃。因此，鼓励所有语言前端在执行优化转换之前验证其输出。

Both of a binary operator’s parameters are of the same type.
二元运算符的两个参数具有相同的类型。

Verify that the indices of mem access instructions match other operands.
验证内存访问指令的索引是否与其他操作数匹配。

Verify that arithmetic and other things are only performed on first-class types. Verify that shifts and logicals only happen on integrals f.e.
验证算术和其他操作仅在第一类类型上执行。验证移位和逻辑仅发生在积分上

All of the constants in a switch statement are of the correct type.
switch 语句中的所有常量都是正确的类型。

The code is in valid SSA form.
该代码采用有效的 SSA 形式。

It is illegal to put a label into any other type (like a structure) or to return one.
将标签放入任何其他类型（如结构）或返回标签是非法的。

Only phi nodes can be self referential: %x = add i32 %x, %x is invalid.
只有 phi 节点可以自引用： %x = add i32 %x 、 %x 无效。

PHI nodes must have an entry for each predecessor, with no extras.
PHI 节点必须为每个前驱节点都有一个条目，没有额外的条目。

PHI nodes must be the first thing in a basic block, all grouped together.
PHI 节点必须是基本块中的第一个节点，所有节点都分组在一起。

PHI nodes must have at least one entry.
PHI 节点必须至少有一个条目。

All basic blocks should only end with terminator insts, not contain them.
所有基本块只能以终止符实例结束，而不是包含它们。

The entry node to a function must not have predecessors.
函数的入口节点不得有前驱节点。

All Instructions must be embedded into a basic block.
所有指令必须嵌入到基本块中。

Functions cannot take a void-typed parameter.
函数不能采用 void 类型的参数。

Verify that a function’s argument list agrees with its declared type.
验证函数的参数列表与其声明的类型一致。

It is illegal to specify a name for a void value.
为 void 值指定名称是非法的。

It is illegal to have an internal global value with no initializer.
拥有不带初始值设定项的内部全局值是非法的。

It is illegal to have a ret instruction that returns a value that does not agree with the function return value type.
ret 指令返回与函数返回值类型不一致的值是非法的。

Function call argument types match the function prototype.
函数调用参数类型与函数原型匹配。

All other things that are tested by asserts spread about the code.
由断言测试的所有其他内容都在代码中传播。

Note that this does not provide full security verification (like Java), but instead just tries to ensure that code is well-formed.
请注意，这并不提供完整的安全验证（如 Java），而只是尝试确保代码格式良好。

```C


```