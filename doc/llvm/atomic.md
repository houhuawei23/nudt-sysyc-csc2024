%old = atomicrmw add ptr %ptr, i32 1 acquire                        ; yields i32

The ‘atomicrmw’ instruction is used to atomically modify memory.

atomicrmw [volatile] <operation> ptr <pointer>, <ty> <value> [syncscope("<target-scope>")] <ordering>[, align <alignment>]  ; yields ty

Arguments:
There are three arguments to the ‘atomicrmw’ instruction: an operation to apply, an address whose value to modify, an argument to the operation. The operation must be one of the following keywords:

xchg

add

sub

and

nand

or

xor

max

min

umax

umin

fadd

fsub

fmax

fmin

uinc_wrap

udec_wrap

For most of these operations, the type of ‘<value>’ must be an integer type whose bit width is a power of two greater than or equal to eight and less than or equal to a target-specific size limit. For xchg, this may also be a floating point or a pointer type with the same size constraints as integers. For fadd/fsub/fmax/fmin, this must be a floating-point or fixed vector of floating-point type. The type of the ‘<pointer>’ operand must be a pointer to that type. If the atomicrmw is marked as volatile, then the optimizer is not allowed to modify the number or order of execution of this atomicrmw with other volatile operations.

Note: if the alignment is not greater or equal to the size of the <value> type, the atomic operation is likely to require a lock and have poor performance.

The alignment is only optional when parsing textual IR; for in-memory IR, it is always present. If unspecified, the alignment is assumed to be equal to the size of the ‘<value>’ type. Note that this default alignment assumption is different from the alignment used for the load/store instructions when align isn’t specified.

A atomicrmw instruction can also take an optional “syncscope” argument.