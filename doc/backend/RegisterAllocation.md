## SSA Book Ch 22 Register Allocation

1. Introduction
2. Spilling
3. Coloring cand coalescing


```bash
# def: a1, a2
# def: X11, X12
	.globl	_memset
	.type	_memset, @function
_memset:
.LFB0:
	.cfi_startproc
	blez	a1,.L1
	addiw	a2,a1,-1
	srliw	a2,a2,2
	addi	a2,a2,1
	slli	a2,a2,2
	li	a1,0
	tail	memset@plt
.L1:
	ret
```


sysylib.s

-O0

```bash

# def: a0, a1, a5;
# def: s0, ra;
# X10, X11, X15
.LC0:
	.string	"%d"
	.text
	.align	1
	.globl	getint
	.type	getint, @function
getint:
	addi	sp,sp,-32
	sd	ra,24(sp)****
	sd	s0,16(sp)
	addi	s0,sp,32
	addi	a5,s0,-20
	mv	a1,a5
	lla	a0,.LC0
	call	__isoc99_scanf@plt
	lw	a5,-20(s0)
	mv	a0,a5
	ld	ra,24(sp)
	ld	s0,16(sp)
	addi	sp,sp,32
	jr	ra
	.size	getint, .-getint
	.section	.rodata
	.align	3
```

-O2

```bash
# def: a0, a1, sp, ra
# def: X10, X11, X2, X1
.LC0:
	.string	"%d"
	.text
	.align	1
	.globl	getint
	.type	getint, @function
getint:
	addi	sp,sp,-32 # def sp
	addi	a1,sp,12  # def a1
	lla	a0,.LC0     # def a0
	sd	ra,24(sp)
	call	__isoc99_scanf@plt
	ld	ra,24(sp)   # def ra
	lw	a0,12(sp)   # def a0
	addi	sp,sp,32
	jr	ra
	.size	getint, .-getint
	.section	.rodata.str1.8
	.align	3
```


```bash
# def: sp, s0, a5, a1, a0, ra
# def: X10, X11, X15
	.globl	getch
	.type	getch, @function
getch:
	addi	sp,sp,-32
	sd	ra,24(sp)
	sd	s0,16(sp)
	addi	s0,sp,32
	addi	a5,s0,-17
	mv	a1,a5
	lla	a0,.LC1
	call	__isoc99_scanf@plt
	lbu	a5,-17(s0)
	sext.w	a5,a5
	mv	a0,a5
	ld	ra,24(sp)
	ld	s0,16(sp)
	addi	sp,sp,32
	jr	ra
	.size	getch, .-getch
	.section	.rodata
	.align	3
```


```bash
# -O0
# def: X10, X11, X15, F10, F15
.LC2:
	.string	"%f"
	.text
	.align	1
	.globl	getfloat
	.type	getfloat, @function
getfloat:
	addi	sp,sp,-32
	sd	ra,24(sp)
	sd	s0,16(sp)
	addi	s0,sp,32
	addi	a5,s0,-20
	mv	a1,a5
	lla	a0,.LC2
	call	__isoc99_scanf@plt
	flw	fa5,-20(s0)
	fmv.s	fa0,fa5
	ld	ra,24(sp)
	ld	s0,16(sp)
	addi	sp,sp,32
	jr	ra
	.size	getfloat, .-getfloat

  ```