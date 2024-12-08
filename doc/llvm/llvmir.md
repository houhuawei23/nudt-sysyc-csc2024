
## High Level Structure

### Module Structure

### Functions

### Runtime Preemption Specifiers 运行时抢占说明符
Global variables, functions and aliases may have an optional runtime preemption specifier. If a preemption specifier isn’t given explicitly, then a symbol is assumed to be dso_preemptable.
全局变量、函数和别名可能有一个可选的运行时抢占说明符。如果未明确给出抢占说明符，则假定符号为 dso_preemptable 。

dso_preemptable
Indicates that the function or variable may be replaced by a symbol from outside the linkage unit at runtime.
指示函数或变量可以在运行时被链接单元外部的符号替换。

dso_local
The compiler may assume that a function or variable marked as dso_local will resolve to a symbol within the same linkage unit. Direct access will be generated even if the definition is not within this compilation unit.
编译器可能会假设标记为 dso_local 的函数或变量将解析为同一链接单元内的符号。即使定义不在该编译单元内，也会生成直接访问。

### Attribute Groups  属性组 
Attribute groups are groups of attributes that are referenced by objects within the IR. They are important for keeping .ll files readable, because a lot of functions will use the same set of attributes. In the degenerative case of a .ll file that corresponds to a single .c file, the single attribute group will capture the important command line flags used to build that file.

属性组是 IR 中的对象引用的一组属性。它们对于保持 .ll 文件的可读性非常重要，因为许多函数将使用同一组属性。在与单个 .c 文件对应的 .ll 文件的退化情况下，单个属性组将捕获用于构建该文件的重要命令行标志。

An attribute group is a module-level object. To use an attribute group, an object references the attribute group’s ID (e.g. #37). An object may refer to more than one attribute group. In that situation, the attributes from the different groups are merged.

属性组是模块级对象。要使用属性组，对象引用属性组的 ID（例如 #37 ）。一个对象可以引用多个属性组。在这种情况下，来自不同组的属性将被合并。

Here is an example of attribute groups for a function that should always be inlined, has a stack alignment of 4, and which shouldn’t use SSE instructions:

以下是函数的属性组示例，该函数应始终内联、堆栈对齐为 4，并且不应使用 SSE 指令：

```LLVM
; Target-independent attributes:
attributes #0 = { alwaysinline alignstack=4 }

; Target-dependent attributes:
attributes #1 = { "no-sse" }

; Function @f has attributes: alwaysinline, alignstack=4, and "no-sse".
define void @f() #0 #1 { ... }
```

## Instruction Reference
[docs-llvmir-reference](https://llvm.org/docs/LangRef.html#instruction-reference)
### Terminator Instruction

Every basic block ends with a terminator instruction, which indicates which block should be executed next after the current block.

Yield a 'void' type: produce control flow not values. (except invoke)

- ret:
  - `ret <type> <value>` or `ret void`
- br
  - `br i1 <cond>, label <iftrue>, label <iffalse>`
- invoke


#### BR
The conditional branch form of the ‘br’ instruction takes a single ‘i1’ value and two ‘label’ values. 
The unconditional form of the ‘br’ instruction takes a single ‘label’ value as a target.
“ br ”指令的条件分支形式采用单个“ i1 ”值和两个“ label ”值。 
“ br ”指令的无条件形式采用单个“ label ”值作为目标。

执行条件“ br ”指令后，将对“ i1 ”参数进行求值。
如果值为 true ，控制流向“ iftrue ” label 参数。
如果“cond”为 false ，控制流向‘ iffalse ’ label 参数。
如果“ cond ”是 poison 或 undef ，则该指令具有未定义的行为。

```llvm
; ret <type> <value>
; ret void
ret i32 5                       ; Return an integer value of 5
ret void                        ; Return from a void function
ret { i32, i8 } { i32 4, i8 2 } ; Return a struct of values 4 and 2  

; br i1 <cond>, label <iftrue>, label <iffalse>
; br label <dest>
Test:
  %cond = icmp eq i32 %a, %b
  br i1 %cond, label %IfEqual, label %IfUnequal
IfEqual:
  ret i32 1
IfUnequal:
  ret i32 0

;
```

### Unary Operations

```llvm
; fneg: return the negation of op
; <result> = fneg [fast-math flags]* <ty> <op1>   ; yields ty:result
```
### Binary Operations

- add
- fadd
- sub
- fsub
- mul
- fmul
- udiv
- sdiv
- fdiv
- urem
- srem
- frem

### Bitwise Binary Operations

- shl
- lshr
- ashr
- and
- or
- xor

### Vector Operations



### Aggregate Operations


- extracvalue
- insertvalue




### Memory Access and Addressing Operations

- alloca
- load
- store

```llvm
<result> = alloca [inalloca] <type> [, <ty> <NumElements>] [, align <alignment>] [, addrspace(<num>)]     
; yields type addrspace(num)*:result
%ptr = alloca i32                             ; yields ptr
%ptr = alloca i32, i32 4                      ; yields ptr
%ptr = alloca i32, i32 4, align 1024          ; yields ptr
%ptr = alloca i32, align 1024                 ; yields ptr

%ptr = alloca i32                               ; yields ptr
store i32 3, ptr %ptr                           ; yields void
%val = load i32, ptr %ptr                       ; yields i32:val = i32 3

<result> = load [volatile] <ty>, ptr <pointer>[, align <alignment>][, !nontemporal !<nontemp_node>][, !invariant.load !<empty_node>][, !invariant.group !<empty_node>][, !nonnull !<empty_node>][, !dereferenceable !<deref_bytes_node>][, !dereferenceable_or_null !<deref_bytes_node>][, !align !<align_node>][, !noundef !<empty_node>]
<result> = load atomic [volatile] <ty>, ptr <pointer> [syncscope("<target-scope>")] <ordering>, align <alignment> [, !invariant.group !<empty_node>]
!<nontemp_node> = !{ i32 1 }
!<empty_node> = !{}
!<deref_bytes_node> = !{ i64 <dereferenceable_bytes> }
!<align_node> = !{ i64 <value_alignment> }


store [volatile] <ty> <value>, ptr <pointer>[, align <alignment>][, !nontemporal !<nontemp_node>][, !invariant.group !<empty_node>]        ; yields void
store atomic [volatile] <ty> <value>, ptr <pointer> [syncscope("<target-scope>")] <ordering>, align <alignment> [, !invariant.group !<empty_node>] ; yields void
!<nontemp_node> = !{ i32 1 }
!<empty_node> = !{}



```
### Conversion Operations

- `trunc .. to`:  truncates its operand to the type ty2.
  - `<result> = trunc <ty> <value> to <ty2>`  yields ty2
- `zext .. to`: zero extends its operand to type ty2.
  - `<result> = zext <ty> <value> to <ty2>   `
- `sext .. to`: sign extends value to the type ty2.
  - `<result> = sext <ty> <value> to <ty2>`
- `fptrunc .. to`
  - `<result> = fptrunc <ty> <value> to <ty2>`
- `fpext .. to`
- `fptoui .. to`: converts a floating-point value to its unsigned integer equivalent of type ty2.
  - `<result> = fptoui <ty> <value> to <ty2>`
- `fptosi .. to`: converts floating-point value to type ty2
  - `<result> = fptosi <ty> <value> to <ty2>`
- `uitofp .. to`: regards value as an unsigned integer and converts that value to the ty2 type.
- `sitofp .. to`: regards value as a signed integer and converts that value to the ty2 type.
- `ptrtoint .. to`
- `inttoptr .. to`
- `bitcast .. to`: converts value to type ty2 without changing any bits.
- `addrspacecast .. to`

```LLVM
; trunc .. to 
; trunc 接受一个要截断的值和一个截断到的类型。
; 两种类型都必须是整数类型，或者是具有相同数量整数的向量。 
; value 的位大小必须大于目标类型 ty2 的位大小。
; 不允许使用相同大小的类型。

; 截断 value 中的高位并将剩余位转换为 ty2 。
; 由于源大小必须大于目标大小，因此 trunc 不能是无操作强制转换。它总是会截断位。
; <result> = trunc <ty> <value> to <ty2>        ; yields ty2
%X = trunc i32 257 to i8                        ; yields i8:1
%Y = trunc i32 123 to i1                        ; yields i1:true
%Z = trunc i32 122 to i1                        ; yields i1:false
%W = trunc <2 x i16> <i16 8, i16 7> to <2 x i8> ; yields <i8 8, i8 7>


; <result> = zext <ty> <value> to <ty2>   
; 将其操作数零扩展为类型 ty2 。
%X = zext i32 257 to i64              ; yields i64:257
%Y = zext i1 true to i32              ; yields i32:1
%Z = zext <2 x i16> <i16 8, i16 7> to <2 x i32> ; yields <i32 8, i32 7>

%a = zext nneg i8 127 to i16 ; yields i16 127
%b = zext nneg i8 -1 to i16  ; yields i16 poison

; <result> = sext <ty> <value> to <ty2>
%X = sext i8  -1 to i16              ; yields i16   :65535
%Y = sext i1 true to i32             ; yields i32:-1
%Z = sext <2 x i16> <i16 8, i16 7> to <2 x i32> ; yields <i32 8, i32 7>

; <result> = fptrunc <ty> <value> to <ty2> 
; 将 value 从较大的浮点类型转换为较小的浮点类型。假定该指令在默认浮点环境中执行。
%X = fptrunc double 16777217.0 to float    ; yields float:16777216.0
%Y = fptrunc double 1.0E+300 to half       ; yields half:+infinity


; <result> = fpext <ty> <value> to <ty2> 
%X = fpext float 3.125 to double         ; yields double:3.125000e+00
%Y = fpext double %X to fp128            ; yields fp128:0xL00000000000000004000900000000000


; <result> = fptoui <ty> <value> to <ty2>
; 将其浮点操作数转换为最接近的（向零舍入）无符号整数值。
; 如果该值无法放入 ty2 中，则结果是有毒值 poison value。

%X = fptoui double 123.0 to i32      ; yields i32:123
%Y = fptoui float 1.0E+300 to i1     ; yields undefined:1
%Z = fptoui float 1.04E+17 to i8     ; yields undefined:1

; <result> = fptosi <ty> <value> to <ty2>
; 将其浮点操作数转换为最接近的（向零舍入）有符号整数值。
; 如果该值无法放入 ty2 中，则结果是有毒值。
%X = fptosi double -123.0 to i32      ; yields i32:-123
%Y = fptosi float 1.0E-247 to i1      ; yields undefined:1
%Z = fptosi float 1.04E+17 to i8      ; yields undefined:1

; 接受一个要转换的值，该值必须是标量或向量整数值，以及将其转换为 ty2 的类型，该类型必须是浮点类型。
; 如果 ty 是向量整数类型，则 ty2 必须是向量浮点类型，且元素数量与 ty 相同
; 将其操作数解释为无符号整数，并将其转换为相应的浮点值。
; 如果无法精确表示该值，则使用默认舍入模式对其进行舍入。
%X = uitofp i32 257 to float         ; yields float:257.0
%Y = uitofp i8 -1 to double          ; yields double:255.0

; <result> = sitofp <ty> <value> to <ty2>
; 将其操作数解释为有符号整数，并将其转换为相应的浮点值。
; 如果无法精确表示该值，则使用默认舍入模式对其进行舍入。
%X = sitofp i32 257 to float         ; yields float:257.0
%Y = sitofp i8 -1 to double          ; yields double:-1.0


%X = bitcast i8 255 to i8         ; yields i8 :-1
%Y = bitcast i32* %x to i16*      ; yields i16*:%x
%Z = bitcast <2 x i32> %V to i64; ; yields i64: %V (depends on endianness)
%Z = bitcast <2 x i32*> %V to <2 x i64*> ; yields <2 x i64*>



```

### Other Operations

- `icmp`: returns a boolean value or a vector of boolean values based on comparison of its 
  - two integer, integer vector, pointer, or pointer vector operands.
- `fcmp`: float comparison
- `phi`
- `call`

#### ICMP

1. `eq`: yields `true` if the operands are equal, `false` otherwise. No sign interpretation is necessary or performed.
2. `ne`: yields `true` if the operands are unequal, `false` otherwise. No sign interpretation is necessary or performed.
3. `ugt`: interprets the operands as unsigned values and yields `true` if `op1` is greater than `op2`.
4. `uge`: interprets the operands as unsigned values and yields `true` if `op1` is greater than or equal to `op2`.
5. `ult`: interprets the operands as unsigned values and yields `true` if `op1` is less than `op2`.
6. `ule`: interprets the operands as unsigned values and yields `true` if `op1` is less than or equal to `op2`.
7. `sgt`: interprets the operands as signed values and yields `true` if `op1` is greater than `op2`.
8. `sge`: interprets the operands as signed values and yields `true` if `op1` is greater than or equal to `op2`.
9. `slt`: interprets the operands as signed values and yields `true` if `op1` is less than `op2`.
10. `sle`: interprets the operands as signed values and yields `true` if `op1` is less than or equal to `op2`.


#### FCMP

1. `false`: no comparison, always returns false
2. `oeq`: ordered and equal
3. `ogt`: ordered and greater than
4. `oge`: ordered and greater than or equal
5. `olt`: ordered and less than
6. `ole`: ordered and less than or equal
7. `one`: ordered and not equal
8. `ord`: ordered (no nans)
9. `ueq`: unordered or equal
10. `ugt`: unordered or greater than
11. `uge`: unordered or greater than or equal
12. `ult`: unordered or less than
13. `ule`: unordered or less than or equal
14. `une`: unordered or not equal
15. `uno`: unordered (either nans)
16. `true`: no comparison, always returns true

#### PHI


#### CALL

This instruction requires several arguments:
该指令需要几个参数：

The optional tail and musttail markers indicate that the optimizers should perform tail call optimization. The tail marker is a hint that can be ignored. The musttail marker means that the call must be tail call optimized in order for the program to be correct. This is true even in the presence of attributes like “disable-tail-calls”. The musttail marker provides these guarantees:
可选的 tail 和 musttail 标记指示优化器应该执行尾调用优化。 tail 标记是可以忽略的提示。 musttail 标记意味着调用必须经过尾部调用优化才能使程序正确。即使存在“disable-tail-calls”等属性也是如此。 musttail 标记提供以下保证：

The call will not cause unbounded stack growth if it is part of a recursive cycle in the call graph.
如果该调用是调用图中递归循环的一部分，则不会导致堆栈无限增长。

Arguments with the inalloca or preallocated attribute are forwarded in place.
具有 inalloca 或 preallocated 属性的参数将就地转发。

If the musttail call appears in a function with the "thunk" attribute and the caller and callee both have varargs, then any unprototyped arguments in register or memory are forwarded to the callee. Similarly, the return value of the callee is returned to the caller’s caller, even if a void return type is in use.
如果 Musttail 调用出现在具有 "thunk" 属性的函数中，并且调用者和被调用者都具有可变参数，则寄存器或内存中的任何非原型参数都会转发给被调用者。类似地，即使正在使用 void 返回类型，被调用者的返回值也会返回给调用者的调用者。

Both markers imply that the callee does not access allocas from the caller. The tail marker additionally implies that the callee does not access varargs from the caller. Calls marked musttail must obey the following additional rules:
这两个标记都意味着被调用者不会访问调用者的分配。 tail 标记还暗示被调用者不会从调用者访问可变参数。标记为 musttail 的调用必须遵守以下附加规则：

The call must immediately precede a ret instruction, or a pointer bitcast followed by a ret instruction.
该调用必须紧接在 ret 指令之前，或者紧跟在 ret 指令后面的指针位转换。

The ret instruction must return the (possibly bitcasted) value produced by the call, undef, or void.
ret 指令必须返回由调用、undef 或 void 生成的（可能是位转换的）值。

The calling conventions of the caller and callee must match.
调用者和被调用者的调用约定必须匹配。

The callee must be varargs iff the caller is varargs. Bitcasting a non-varargs function to the appropriate varargs type is legal so long as the non-varargs prefixes obey the other rules.
当且仅当调用者是可变参数时，被调用者必须是可变参数。只要非可变参数前缀遵守其他规则，将非可变参数函数位转换为适当的可变参数类型就是合法的。

The return type must not undergo automatic conversion to an sret pointer.
返回类型不得自动转换为 sret 指针。

In addition, if the calling convention is not swifttailcc or tailcc:
另外，如果调用约定不是 swifttailcc 或 tailcc：

All ABI-impacting function attributes, such as sret, byval, inreg, returned, and inalloca, must match.
所有影响 ABI 的函数属性（例如 sret、byval、inreg、returned 和 inalloca）都必须匹配。

The caller and callee prototypes must match. Pointer types of parameters or return types may differ in pointee type, but not in address space.
调用者和被调用者原型必须匹配。参数或返回类型的指针类型在指针对象类型中可能不同，但在地址空间中不会不同。

On the other hand, if the calling convention is swifttailcc or swiftcc:
另一方面，如果调用约定是 swifttailcc 或 swiftcc：

Only these ABI-impacting attributes attributes are allowed: sret, byval, swiftself, and swiftasync.
仅允许这些影响 ABI 的属性：sret、byval、swiftself 和 swiftasync。

Prototypes are not required to match.
原型不需要匹配。

Tail call optimization for calls marked tail is guaranteed to occur if the following conditions are met:
如果满足以下条件，则保证会发生标记为 tail 的尾部调用优化：

Caller and callee both have the calling convention fastcc or tailcc.
调用者和被调用者都具有调用约定 fastcc 或 tailcc 。

The call is in tail position (ret immediately follows call and ret uses value of call or is void).
调用位于尾部位置（ret 紧跟在调用之后，并且 ret 使用调用的值或者为 void）。

Option -tailcallopt is enabled, llvm::GuaranteedTailCallOpt is true, or the calling convention is tailcc
选项 -tailcallopt 已启用， llvm::GuaranteedTailCallOpt 为 true ，或者调用约定为 tailcc

Platform-specific constraints are met.
满足特定于平台的约束。

The optional notail marker indicates that the optimizers should not add tail or musttail markers to the call. It is used to prevent tail call optimization from being performed on the call.
可选的 notail 标记指示优化器不应向调用添加 tail 或 musttail 标记。它用于防止对调用执行尾调用优化。

The optional fast-math flags marker indicates that the call has one or more fast-math flags, which are optimization hints to enable otherwise unsafe floating-point optimizations. Fast-math flags are only valid for calls that return a floating-point scalar or vector type, or an array (nested to any depth) of floating-point scalar or vector types.
可选的 fast-math flags 标记指示该调用具有一个或多个快速数学标志，这些标志是优化提示，用于启用其他不安全的浮点优化。快速数学标志仅对返回浮点标量或向量类型或浮点标量或向量类型的数组（嵌套到任意深度）的调用有效。

The optional “cconv” marker indicates which calling convention the call should use. If none is specified, the call defaults to using C calling conventions. The calling convention of the call must match the calling convention of the target function, or else the behavior is undefined.
可选的“cconv”标记指示调用应使用哪种调用约定。如果未指定，则调用默认使用 C 调用约定。调用的调用约定必须与目标函数的调用约定相匹配，否则行为未定义。

The optional Parameter Attributes list for return values. Only ‘zeroext’, ‘signext’, and ‘inreg’ attributes are valid here.
返回值的可选参数属性列表。只有“ zeroext ”、“ signext ”和“ inreg ”属性在这里有效。

The optional addrspace attribute can be used to indicate the address space of the called function. If it is not specified, the program address space from the datalayout string will be used.
可选的 addrspace 属性可用于指示被调用函数的地址空间。如果未指定，则将使用数据布局字符串中的程序地址空间。

‘ty’: the type of the call instruction itself which is also the type of the return value. Functions that return no value are marked void.
‘ ty ’：调用指令本身的类型，也是返回值的类型。不返回值的函数被标记为 void 。

‘fnty’: shall be the signature of the function being called. The argument types must match the types implied by this signature. This type can be omitted if the function is not varargs.
‘ fnty ’：应是被调用函数的签名。参数类型必须与此签名隐含的类型匹配。如果函数不是可变参数，则可以省略此类型。

‘fnptrval’: An LLVM value containing a pointer to a function to be called. In most cases, this is a direct function call, but indirect call’s are just as possible, calling an arbitrary pointer to function value.
‘ fnptrval ’：一个 LLVM 值，包含指向要调用的函数的指针。在大多数情况下，这是直接函数调用，但间接 call 也是可能的，调用指向函数值的任意指针。

‘function args’: argument list whose types match the function signature argument types and parameter attributes. All arguments must be of first class type. If the function signature indicates the function accepts a variable number of arguments, the extra arguments can be specified.
‘ function args ’：类型与函数签名参数类型和参数属性匹配的参数列表。所有参数必须是第一类类型。如果函数签名指示该函数接受可变数量的参数，则可以指定额外的参数。

The optional function attributes list.
可选功能属性列表。

The optional operand bundles list.
可选操作数捆绑列表。

#### Example

```LLVM
; <result> = icmp <cond> <ty> <op1>, <op2>   ; yields i1 or <N x i1>:result

<result> = icmp eq i32 4, 5          ; yields: result=false
<result> = icmp ne ptr %X, %X        ; yields: result=false
<result> = icmp ult i16  4, 5        ; yields: result=true
<result> = icmp sgt i16  4, 5        ; yields: result=false
<result> = icmp ule i16 -4, 5        ; yields: result=false
<result> = icmp sge i16  4, 5        ; yields: result=false

; fcmp
<result> = fcmp oeq float 4.0, 5.0    ; yields: result=false
<result> = fcmp one float 4.0, 5.0    ; yields: result=true
<result> = fcmp olt float 4.0, 5.0    ; yields: result=true
<result> = fcmp ueq double 1.0, 2.0   ; yields: result=false

; phi
; <result> = phi [fast-math-flags] <ty> [ <val0>, <label0>], ...
; implement the phi node in the SSA graph representing the function.
; 在运行时，“ phi ”指令逻辑上采用与在当前块之前执行的前驱基本块相对应的对指定的值。

Loop:       ; Infinite loop that counts from 0 on up...
  %indvar = phi i32 [ 0, %LoopHeader ], [ %nextindvar, %Loop ]
  %nextindvar = add i32 %indvar, 1
  br label %Loop


; call
<result> = [tail | musttail | notail ] call [fast-math flags] [cconv] [ret attrs] [addrspace(<num>)]
           <ty>|<fnty> <fnptrval>(<function args>) [fn attrs] [ operand bundles ]

%retval = call i32 @test(i32 %argc)
call i32 (ptr, ...) @printf(ptr %msg, i32 12, i8 42)        ; yields i32
%X = tail call i32 @foo()                                    ; yields i32
%Y = tail call fastcc i32 @foo()  ; yields i32
call void %foo(i8 signext 97)

%struct.A = type { i32, i8 }
%r = call %struct.A @foo()                        ; yields { i32, i8 }
%gr = extractvalue %struct.A %r, 0                ; yields i32
%gr1 = extractvalue %struct.A %r, 1               ; yields i8
%Z = call void @foo() noreturn                    ; indicates that %foo never returns normally
%ZZ = call zeroext i32 @bar()                     ; Return value is %zero extended

```

The type of the incoming values is specified with the first type field. After this, the ‘phi’ instruction takes a list of pairs as arguments, with one pair for each predecessor basic block of the current block. Only values of first class type may be used as the value arguments to the PHI node. Only labels may be used as the label arguments.
传入值的类型由第一个类型字段指定。此后，“ phi ”指令将一系列对作为参数，当前块的每个前驱基本块都有一对。只有第一类类型的值可以用作 PHI 节点的值参数。只有标签可以用作标签参数。

There must be no non-phi instructions between the start of a basic block and the PHI instructions: i.e. PHI instructions must be first in a basic block.
基本块的开始和 PHI 指令之间不得有非 phi 指令：即 PHI 指令必须位于基本块中的第一个。

For the purposes of the SSA form, the use of each incoming value is deemed to occur on the edge from the corresponding predecessor block to the current block (but after any definition of an ‘invoke’ instruction’s return value on the same edge).
出于 SSA 形式的目的，每个传入值的使用被视为发生在从相应前驱块到当前块的边缘上（但在“ invoke ”指令的返回值的任何定义之后）相同的边）。

The optional fast-math-flags marker indicates that the phi has one or more fast-math-flags. These are optimization hints to enable otherwise unsafe floating-point optimizations. Fast-math-flags are only valid for phis that return a floating-point scalar or vector type, or an array (nested to any depth) of floating-point scalar or vector types.
可选的 fast-math-flags 标记表示 phi 有一个或多个快速数学标志。这些是优化提示，用于启用其他不安全的浮点优化。 Fast-math-flags 仅对返回浮点标量或向量类型，或浮点标量或向量类型的数组（嵌套到任意深度）的 phi 有效。

## GetElementPtr

https://llvm.org/docs/LangRef.html#getelementptr-instruction

https://llvm.org/docs/GetElementPtr.html#i-m-writing-a-backend-for-a-target-which-needs-custom-lowering-for-gep-how-do-i-do-this

```llvm
<result> = getelementptr <ty>, ptr <ptrval>{, <ty> <idx>}*
<result> = getelementptr inbounds <ty>, ptr <ptrval>{, <ty> <idx>}*
<result> = getelementptr nusw <ty>, ptr <ptrval>{, <ty> <idx>}*
<result> = getelementptr nuw <ty>, ptr <ptrval>{, <ty> <idx>}*
<result> = getelementptr inrange(S,E) <ty>, ptr <ptrval>{, <ty> <idx>}*
<result> = getelementptr <ty>, <N x ptr> <ptrval>, <vector index type> <idx>

```

The first argument is always a type used as the basis for the calculations. The second argument is always a pointer or a vector of pointers, and is the base address to start from. The remaining arguments are indices that indicate which of the elements of the aggregate object are indexed. The interpretation of each index is dependent on the type being indexed into. The first index always indexes the pointer value given as the second argument, the second index indexes a value of the type pointed to (not necessarily the value directly pointed to, since the first index can be non-zero), etc. The first type indexed into must be a pointer value, subsequent types can be arrays, vectors, and structs. Note that subsequent types being indexed into can never be pointers, since that would require loading the pointer before continuing calculation.

第一个参数始终是用作计算基础的类型。第二个参数始终是指针或指针向量，并且是起始基地址。其余参数是索引，指示聚合对象的哪些元素已建立索引。每个索引的解释取决于索引的类型。第一个索引始终索引作为第二个参数给出的指针值，第二个索引索引指向的类型的值（不一定是直接指向的值，因为第一个索引可以是非零），等等。索引必须是指针值，后续类型可以是数组、向量和结构体。请注意，后续被索引的类型永远不能是指针，因为这需要在继续计算之前加载指针。

The type of each index argument depends on the type it is indexing into. When indexing into a (optionally packed) structure, only i32 integer constants are allowed (when using a vector of indices they must all be the same i32 integer constant). When indexing into an array, pointer or vector, integers of any width are allowed, and they are not required to be constant. These integers are treated as signed values where relevant.

每个索引参数的类型取决于它索引到的类型。当索引到（可选打包）结构时，仅允许使用i32整型常量（当使用索引向量时，它们必须都是相同的i32整型常量）。当索引到数组、指针或向量时，允许任何宽度的整数，并且它们不需要是常量。这些整数在相关时被视为有符号值。

For example, let’s consider a C code fragment and how it gets compiled to LLVM:

例如，让我们考虑一个 C 代码片段以及它如何编译为 LLVM：