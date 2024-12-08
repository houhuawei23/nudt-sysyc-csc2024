# Loop Definition

[LLVM Loop Terminology (and Canonical Forms)](https://llvm.org/docs/LoopTerminology.html)

Loops are an important concept for a code optimizer. In LLVM, detection of loops in a control-flow graph is done by LoopInfo. It is based on the following definition.

A loop is a subset of nodes from the control-flow graph (CFG; where nodes represent basic blocks) with the following properties:

1. The induced subgraph (which is the subgraph that contains all the edges from the CFG within the loop) is strongly connected (every node is reachable from all others).

2. All edges from outside the subset into the subset point to the same node, called the `header`. As a consequence, the header **dominates** all nodes in the loop (i.e. every execution path to any of the loop’s node will have to pass through the header).

3. The loop is the maximum subset with these properties. That is, no additional nodes from the CFG can be added such that the induced subgraph would still be strongly connected and the header would remain the same.

In computer science literature, this is often called a natural loop. In LLVM, a more generalized definition is called a cycle.

1. 诱导子图（包含循环内CFG中的所有边的子图）是强连通的（每个节点都可以从其他节点到达）。

2. 从子集外部进入子集的所有边都指向同一个节点，称为头节点。因此，头节点支配循环中的所有节点（即，每条到任何循环节点的执行路径都必须经过头节点）。

3. 该循环是具有这些性质的最大子集。也就是说，不能添加CFG中的额外节点，使得诱导子图仍然是强连通的并且头节点保持不变。

- Entering Block (Loop Predecessor)
  - a non-loop node that has an edge into the loop (necessarily the header)
- Latch
  - a loop node that has an edge to the header.
- Backedge
  - an edge from a latch to the header.
- Exiting Block -- Exiting Edge -> Exit Block

## Loop Simplify Form

The Loop Simplify Form is a canonical form that makes several analyses and transformations simpler and more effective. It is ensured by the LoopSimplify (-loop-simplify) pass and is automatically added by the pass managers when scheduling a LoopPass. This pass is implemented in LoopSimplify.h. When it is successful, the loop has:

- A preheader.

- A single backedge (which implies that there is a single latch).

- Dedicated exits. That is, no exit block for the loop has a predecessor that is outside the loop. This implies that all exit blocks are dominated by the loop header.

## Loop Closed SSA (LCSSA)

A program is in Loop Closed SSA Form if it is in SSA form and all values that are defined in a loop are used only inside this loop.
如果程序采用 SSA 形式，并且循环中定义的所有值仅在该循环内使用，则该程序处于循环闭环 SSA 形式。

Programs written in LLVM IR are always in SSA form but not necessarily in LCSSA. To achieve the latter, for each value that is live across the loop boundary, single entry PHI nodes are inserted to each of the exit blocks [1] in order to “close” these values inside the loop. In particular, consider the following loop:
用 LLVM IR 编写的程序始终是 SSA 形式，但不一定是 LCSSA。为了实现后者，对于跨越循环边界的每个值，将单入口 PHI 节点插入到每个出口块 1 中，以便“关闭”循环内的这些值。特别是考虑以下循环：

```LLVM
c = ...;
for (...) {
  if (c)
    X1 = ...
  else
    X2 = ...
  X3 = phi(X1, X2);  // X3 defined
}

... = X3 + 4;  // X3 used, i.e. live
               // outside the loo
```


```LLVM
c = ...;
for (...) {
  if (c)
    X1 = ...
  else
    X2 = ...
  X3 = phi(X1, X2);
}
X4 = phi(X3);

... = X4 + 4;
```