```llvm

bb3: ; other
    ; pres: bb2
    ; nexts: bb6
    br label %bb6 ; br while1_judge
    
bb6: ; while1_judge
    ; pres: bb3, bb5
    ; nexts: bb5, bb4
    %3 = phi i32 [ 0, %bb3 ],[ %2, %bb5 ]
    %4 = icmp slt i32 %3, 100 
    br i1 %4, label %bb5, label %bb4 ; br while1_loop, while1_next

bb5: ; while1_loop
    ; pres: bb6
    ; nexts: bb6
    %1 = getelementptr [100 x i32], [100 x i32]* %0, i32 0, i32 %3
    store i32 %3, i32* %1

latch:
    %2 = add i32 %3, 1
    br label %bb6 ; br while1_judge

bb4: ; while1_next
    ; pres: bb6
    ; nexts: bb1
    br label %bb1 ; br exit

```

=>

```llvm

bb6: ; while1_judge
    ; pres: bb3, bb5
    ; nexts: bb5, bb4
    %3 = phi i32 [ 0, %bb3 ],[ %2, %bb5 ]
    %4 = icmp slt i32 %3, 100 
    br i1 %4, label %callblock, label %bb4 ; br callblock, while1_next

callblock: 
    ; pres: bb6
    ; nexts: bb6
    call void @loop_boby(i32 %3, [100 x i32]* %0)
   br label %latch ; br latch
latch:
    %2 = add i32 %3, 1
    br label %bb6 ; br while1_judge

void loop_boby(i32 %3, [100 x i32]* %0){
    %1 = getelementptr [100 x i32], [100 x i32]* %0, i32 0, i32 %3
    store i32 %3, i32* %1
    ret void
}

```


`extractLoopBody`函数的目的是从一个给定的循环中提取循环体，以便进行进一步的优化或转换。这个函数接受多个参数，包括循环本身、支配树分析结果、控制流图分析结果、模块、是否独立、指针基础分析结果、是否允许内循环、是否只添加递归、是否估计未展开的块大小、是否需要子循环、是否将减少操作转换为原子操作、是否复制比较操作等。函数的执行流程如下：

1. **参数检查**:
   - 首先，函数检查是否允许处理内嵌循环的最内层循环，如果不是，则直接返回`false`。

2. **Phi指令计数**:
   - 函数遍历循环头部的指令，计数Phi指令的数量。Phi指令通常用于表示循环变量。如果Phi指令超过2个，则函数返回`false`，因为目前只支持最多一个BIV（Backedge-Taken Variable）和一个GIV（Guiding Variable）。

3. **收集循环体**:
   - 使用`collectLoopBody`函数来收集循环体中的所有基本块，即那些在循环中但不在循环的出口或入口之外的块。

4. **检查循环体的完整性**:
   - 确保循环体中的每个块的所有后继块也都属于循环体。

5. **估计块大小**:
   - 如果需要为未展开的循环估计块大小，则计算循环体中每个块的大小，并检查是否有函数调用或总大小是否超出了限制。

6. **寻找GIV**:
   - 寻找循环中的全局引导变量（GIV），并检查它是否被循环外部的指令使用。

7. **匹配加法递归模式**:
   - 如果只允许加法递归并且找到了GIV，则尝试匹配特定的模式，以确定是否可以将循环转换为更简单的形式。

8. **检查循环内变量的使用**:
   - 确保循环体内的变量仅在循环体内使用，除非它们是循环的循环变量或GIV。

9. **独立性检查**:
   - 如果循环是独立的，即循环内的变量不与循环外的变量共享，则检查循环体内的加载和存储指令，并尝试将它们转换为原子操作。

10. **创建新的循环体函数**:
    - 创建一个新的函数来表示循环体，并将这个新函数添加到模块中。

11. **参数映射和函数调用**:
    - 为循环体内的每个变量创建参数，并映射这些参数到原始循环体内的变量。

12. **复制循环条件和更新控制流**:
    - 复制循环的条件，并更新循环的入口和出口，以使用新的循环体函数。

13. **更新Phi指令**:
    - 更新循环变量和GIV的Phi指令，以反映新的控制流结构。

14. **更新循环外部的变量使用**:
    - 更新循环外部对循环内部变量的引用，以使用新的参数或返回值。

15. **返回结果**:
    - 如果提供了`LoopBodyInfo`结构体，则填充相关信息并返回。

整个`extractLoopBody`函数的执行流程是复杂的，涉及到多个步骤的检查和转换，目的是将循环体提取出来作为一个独立的函数，这样可以对循环体进行更精细的优化。这个过程需要仔细处理循环变量、控制流、以及循环内外变量的交互。
