LoopParallel 过程主要步骤:

1. 分析出 Loop 信息
2. 提取出 LoopBody, 封装成 loop_body (new Function)
3. 构造 parallel_body(start, end) 函数
4. 通过 parallelFor(start, end, parallel_body) 调用并行库
5. parallelFor(start, end, parallel_body) 就是后端暴露给中端用于并行化的接口

```cpp
void parallel_body(int start, int end) {
    
}

int main() {
    call ParallelFor(beg, end, parallel_body);

}

```
after loop-extract:
```llvm
define i32 @main() {
bb0: ; entry
    ; nexts: bb2
    %0 = alloca [100 x i32] ; arr*
    br label %bb2 ; br next
bb1: ; exit
    ; pres: bb4
    ret i32 5
bb2: ; next
    ; pres: bb0
    ; nexts: bb3
    br label %bb3 ; br other
bb3: ; other
    ; pres: bb2
    ; nexts: bb5
    br label %bb5 ; br while1_judge
bb4: ; while1_next
    ; pres: bb5
    ; nexts: bb1
    br label %bb1 ; br exit
bb5: ; while1_judge
    ; pres: bb3, bb6
    ; nexts: bb7, bb4
    %1 = phi i32 [ 0, %bb3 ],[ %3, %bb6 ]
    %2 = icmp slt i32 %1, 100 
    br i1 %2, label %bb7, label %bb4
bb7:
    ; pres: bb5
    ; nexts: bb6
    call void @loop_body(i32 %1, [100 x i32]* %0)
    br label %bb6
bb6:
    ; pres: bb7
    ; nexts: bb5
    %3 = add i32 %1, 1
    br label %bb5 ; br while1_judge

}
define void @loop_body(i32 %0, [100 x i32]* %1) {
bb0:
    ; nexts: bb1
    br label %bb1 ; br while1_loop
bb1: ; while1_loop
    ; pres: bb0
    %2 = getelementptr [100 x i32], [100 x i32]* %1, i32 0, i32 %0
    store i32 %0, i32* %2
    ret void
}
```


```llvm
bb3: ; other
    ; pres: bb2
    ; nexts: bb5
    br label %bb5 ; br while1_judge
bb5: ; while1_judge
    ; pres: bb3, bb6
    ; nexts: bb7, bb4
    %1 = phi i32 [ 0, %bb3 ],[ %3, %bb6 ]
    %2 = icmp slt i32 %1, 100 
    br i1 %2, label %bb7, label %bb4
bb7:
    ; pres: bb5
    ; nexts: bb6
    call void @loop_body(i32 %1, [100 x i32]* %0)
    br label %bb6
bb6:
    ; pres: bb7
    ; nexts: bb5
    %3 = add i32 %1, 1
    br label %bb5 ; br while1_judge
bb4: ; while1_next
    ; pres: bb5
    ; nexts: bb1
    br label %bb1 ; br exit

```

```llvm
i32 main() {
bb3: ; other
    br label %bb_call
bb_call:
    call void @parallel_body(i32 0, i32 100)
    br label %bb4 ; br while1_next
bb4: ; while1_next
    br label %bb1 ; br exit
}

void parallel_body(i32 %beg, i32 %end) {
entry:
    br bb5
header: 
    %1 = phi i32 [ %beg, %entry ],[ %3, %latch ]
    %2 = icmp slt i32 %1, %end
    br i1 %2, label %body, label %exit
body:
    call void @loop_body(i32 %1, [100 x i32]* %0)
    br label %latch
latch:
    %3 = add i32 %1, 1
    br label %header
exit:
    ret void
}
```

```llvm
i32 main() {
bb3: ; other
    br label %bb_call
bb_call:
    %ptr = functionptr @parallel_body as i8*
    call parallelFor(i32 0, i32 100, i8* %ptr)
    ; call void @parallel_body(i32 0, i32 100)
    br label %bb4 ; br while1_next
bb4: ; while1_next
    br label %bb1 ; br exit
}

void parallel_body(i32 %beg, i32 %end) {
entry:
    br bb5
header: 
    %1 = phi i32 [ %beg, %entry ],[ %3, %latch ]
    %2 = icmp slt i32 %1, %end
    br i1 %2, label %body, label %exit
body:
    call void @loop_body(i32 %1, [100 x i32]* %0)
    br label %latch
latch:
    %3 = add i32 %1, 1
    br label %header
exit:
    ret void
}
```

Loop Info:

level: 1
header: bb2
exits: 1
  bb7
latchs: 1
  bb6
blocks: 5
  bb2
  bb6
  bb5
  bb4
  bb3
level: 2

header: bb4
exits: 1
  bb6
latchs: 1
  bb5
blocks: 2
  bb5
  bb4


```llvm
define i32 @main() {
bb0: ; entry
    br label %bb1 ; br next
bb1: ; next
    call void @_sysy_starttime(i32 7)
    br label %bb2 ; br while1_judge
bb2: ; while1_judge
    %0 = phi i32 [ 0, %bb1 ],[ %2, %bb4 ]
    %1 = icmp slt i32 %0, 100000 
    br i1 %1, label %bb3, label %bb5
bb3:
    call void @sysyc_loop_body0(i32 %0)
    br label %bb4
bb4:
    %2 = add i32 %0, 1
    br label %bb2 ; br while1_judge
bb5: ; while1_next
    call void @_sysy_stoptime(i32 17)
    %3 = getelementptr [100000 x i32], [100000 x i32]* @arr, i32 0, i32 0
    %4 = load i32, i32* %3
    %5 = getelementptr [100000 x i32], [100000 x i32]* @arr, i32 0, i32 1
    %6 = load i32, i32* %5
    %7 = add i32 %4, %6
    %8 = getelementptr [100000 x i32], [100000 x i32]* @arr, i32 0, i32 2
    %9 = load i32, i32* %8
    %10 = add i32 %7, %9
    %11 = getelementptr [100000 x i32], [100000 x i32]* @arr, i32 0, i32 3
    %12 = load i32, i32* %11
    %13 = add i32 %10, %12
    %14 = getelementptr [100000 x i32], [100000 x i32]* @arr, i32 0, i32 4
    %15 = load i32, i32* %14
    %16 = add i32 %13, %15
    ret i32 %16
}

define void @sysyc_loop_body0(i32 %0) {
bb0:
    br label %bb1
bb1:
    br label %bb2 ; br while2_judge
bb2: ; while2_judge
    %1 = phi i32 [ %5, %bb3 ],[ 0, %bb1 ]
    %2 = icmp slt i32 %1, 100000 
    br i1 %2, label %bb3, label %bb4 ; br while2_loop, while2_next
bb3: ; while2_loop
    %3 = getelementptr [100000 x i32], [100000 x i32]* @arr, i32 0, i32 %0
    %4 = add i32 %1, %1
    store i32 %4, i32* %3
    %5 = add i32 %1, 1
    br label %bb2 ; br while2_judge
bb4: ; while2_next
    ret void
}
```