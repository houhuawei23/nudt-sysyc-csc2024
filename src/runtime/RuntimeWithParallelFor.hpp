// Automatically generated file, do not edit!
// Command: riscv64-linux-gnu-g++-12 -Ofast -DNDEBUG -march=rv64gc_zba_zbb -fno-stack-protector -fomit-frame-pointer -mcpu=sifive-u74 -mabi=lp64d -mcmodel=medlow -ffp-contract=on -w /home/hhw/Desktop/compilers/sys-ycompiler/src/runtime/.merge.cpp -S -o /dev/stdout
R"(	.file	".merge.cpp"
	.option pic
	.attribute arch, "rv64i2p1_m2p0_a2p1_f2p2_d2p2_c2p0_zicsr2p0_zifencei2p0_zba1p0_zbb1p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.align	1
	.type	selectNumberOfThreads, @function
selectNumberOfThreads:
.LFB1230:
	.cfi_startproc
	lla	t3,.LANCHOR0
	mv	t4,a0
	lw	a4,896(t3)
	li	t1,16
	li	a7,16
.L5:
	beq	a4,a7,.L28
	zext.w	a6,a4
	slli.uw	a0,a4,3
	sub	a0,a0,a6
	sh3add	a0,a0,t3
.L2:
	slli	a5,a6,3
	addiw	t1,t1,-1
	sub	a5,a5,a6
	sh3add	a5,a5,t3
	lbu	a6,12(a5)
	beq	a6,zero,.L3
	ld	a6,0(a5)
	beq	t4,a6,.L53
.L3:
	addiw	a4,a4,1
	bne	t1,zero,.L5
	lbu	a5,12(t3)
	beq	a5,zero,.L6
	lbu	a5,68(t3)
	beq	a5,zero,.L29
	lbu	a5,124(t3)
	beq	a5,zero,.L30
	lbu	a5,180(t3)
	beq	a5,zero,.L31
	lbu	a5,236(t3)
	beq	a5,zero,.L32
	lbu	a5,292(t3)
	beq	a5,zero,.L33
	lbu	a5,348(t3)
	beq	a5,zero,.L34
	lbu	a5,404(t3)
	beq	a5,zero,.L35
	lbu	a5,460(t3)
	beq	a5,zero,.L36
	lbu	a5,516(t3)
	beq	a5,zero,.L37
	lbu	a5,572(t3)
	beq	a5,zero,.L38
	lbu	a5,628(t3)
	beq	a5,zero,.L39
	lbu	a5,684(t3)
	beq	a5,zero,.L40
	lbu	a5,740(t3)
	beq	a5,zero,.L41
	lbu	a5,796(t3)
	beq	a5,zero,.L42
	lbu	a5,852(t3)
	li	t1,15
	beq	a5,zero,.L6
	lw	a0,72(t3)
	lw	a5,16(t3)
	lw	a7,128(t3)
	sltu	t1,a0,a5
	lw	a6,184(t3)
	bgeu a0,a5,1f; mv a5,a0; 1: # movcc
	mv	t5,a5
	bgeu a7,a5,1f; mv t5,a7; 1: # movcc
	lw	a4,240(t3)
	bgeu a7,a5,1f; li t1,2; 1: # movcc
	mv	a7,t5
	bleu t5,a6,1f; mv a7,a6; 1: # movcc
	lw	a0,296(t3)
	bleu t5,a6,1f; li t1,3; 1: # movcc
	mv	t5,a7
	bleu a7,a4,1f; mv t5,a4; 1: # movcc
	lw	a5,352(t3)
	bleu a7,a4,1f; li t1,4; 1: # movcc
	mv	a7,t5
	bleu t5,a0,1f; mv a7,a0; 1: # movcc
	lw	a6,408(t3)
	bleu t5,a0,1f; li t1,5; 1: # movcc
	mv	t5,a7
	bleu a7,a5,1f; mv t5,a5; 1: # movcc
	lw	a4,464(t3)
	bleu a7,a5,1f; li t1,6; 1: # movcc
	mv	a7,t5
	bleu t5,a6,1f; mv a7,a6; 1: # movcc
	lw	a0,520(t3)
	bleu t5,a6,1f; li t1,7; 1: # movcc
	mv	t5,a7
	bleu a7,a4,1f; mv t5,a4; 1: # movcc
	lw	a5,576(t3)
	bleu a7,a4,1f; li t1,8; 1: # movcc
	mv	a7,t5
	bleu t5,a0,1f; mv a7,a0; 1: # movcc
	lw	a6,632(t3)
	bleu t5,a0,1f; li t1,9; 1: # movcc
	mv	t5,a7
	bleu a7,a5,1f; mv t5,a5; 1: # movcc
	lw	a4,688(t3)
	bleu a7,a5,1f; li t1,10; 1: # movcc
	mv	a7,t5
	bleu t5,a6,1f; mv a7,a6; 1: # movcc
	lw	a0,744(t3)
	bleu t5,a6,1f; li t1,11; 1: # movcc
	mv	a6,a7
	bleu a7,a4,1f; mv a6,a4; 1: # movcc
	lw	a5,800(t3)
	bleu a7,a4,1f; li t1,12; 1: # movcc
	mv	a4,a6
	bleu a6,a0,1f; mv a4,a0; 1: # movcc
	lw	t5,856(t3)
	bleu a6,a0,1f; li t1,13; 1: # movcc
	mv	a0,a4
	bleu a4,a5,1f; mv a0,a5; 1: # movcc
	bleu a4,a5,1f; li t1,14; 1: # movcc
	bgeu t5,a0,1f; li t1,15; 1: # movcc
	slli	a5,t1,3
	sub	a5,a5,t1
	sh3add	a0,a5,t3
	sw	t1,896(t3)
	li	a5,1
	sd	t4,0(a0)
	sw	a1,8(a0)
	sw	a5,16(a0)
.L4:
	lw	a5,16(a0)
	li	a4,99
	bleu	a5,a4,.L43
.L55:
	li	a4,159
	bleu	a5,a4,.L54
	lw	a5,48(a0)
	bne	a5,zero,.L51
	ld	a4,24(a0)
	ld	a1,32(a0)
	ld	a6,40(a0)
	sgt	a5,a4,a1
	ble a4,a1,1f; mv a4,a1; 1: # movcc
	ble a4,a6,1f; li a5,2; 1: # movcc
	sw	a5,48(a0)
.L51:
	li	a4,0
	sw	a5,0(a2)
	sb	a4,0(a3)
	ret
.L28:
	mv	a0,t3
	li	a6,0
	li	a4,0
	j	.L2
.L53:
	lw	a6,8(a5)
	bne	a6,a1,.L3
	lw	a1,16(a5)
	addiw	a1,a1,1
	sw	a1,16(a5)
	sw	a4,896(t3)
	li	a4,99
	lw	a5,16(a0)
	bgtu	a5,a4,.L55
.L43:
	li	a5,2
	j	.L51
.L29:
	li	t1,1
.L6:
	li	a5,1
	zext.w	a4,t1
	sw	t1,896(t3)
	slli.uw	a0,t1,3
	sub	a0,a0,a4
	sh3add	a0,a0,t3
	sb	a5,12(a0)
	sd	t4,0(a0)
	sw	a1,8(a0)
	sw	a5,16(a0)
	j	.L4
.L54:
	addiw	a5,a5,-100
	li	a1,20
	li	a4,1
	divuw	a5,a5,a1
	sw	a5,0(a2)
	sb	a4,0(a3)
	ret
.L36:
	li	t1,8
	j	.L6
.L30:
	li	t1,2
	j	.L6
.L31:
	li	t1,3
	j	.L6
.L32:
	li	t1,4
	j	.L6
.L33:
	li	t1,5
	j	.L6
.L34:
	li	t1,6
	j	.L6
.L35:
	li	t1,7
	j	.L6
.L37:
	li	t1,9
	j	.L6
.L38:
	li	t1,10
	j	.L6
.L39:
	li	t1,11
	j	.L6
.L40:
	li	t1,12
	j	.L6
.L41:
	li	t1,13
	j	.L6
.L42:
	li	t1,14
	j	.L6
	.cfi_endproc
.LFE1230:
	.size	selectNumberOfThreads, .-selectNumberOfThreads
	.align	1
	.type	_ZN12_GLOBAL__N_110cmmcWorkerEPv, @function
_ZN12_GLOBAL__N_110cmmcWorkerEPv:
.LFB312:
	.cfi_startproc
	addi	sp,sp,-192
	.cfi_def_cfa_offset 192
	sd	s0,176(sp)
	.cfi_offset 8, -16
	mv	s0,a0
	sd	ra,184(sp)
	sd	s1,168(sp)
	sd	s2,160(sp)
	sd	s3,152(sp)
	sd	s4,144(sp)
	sd	s5,136(sp)
	.cfi_offset 1, -8
	.cfi_offset 9, -24
	.cfi_offset 18, -32
	.cfi_offset 19, -40
	.cfi_offset 20, -48
	.cfi_offset 21, -56
	fence	iorw,iorw
	lw	a5,16(a0)
	fence	iorw,iorw
	li	a4,1023
	sext.w	a2,a5
	mv	s1,sp
	zext.w	a5,a5
	bgtu	a5,a4,.L57
	li	a3,1
	srli	a5,a5,6
	sll	a3,a3,a2
	sh3add	a5,a5,s1
	ld	a4,0(a5)
	or	a4,a4,a3
	sd	a4,0(a5)
.L57:
	li	a0,178
	addi	s2,s0,20
	call	syscall@plt
	addi	s3,s0,40
	mv	a2,s1
	sext.w	a0,a0
	li	a1,128
	addi	s1,s0,44
	call	sched_setaffinity@plt
	li	s4,1
	j	.L61
.L75:
	fence iorw,ow;  1: lr.w.aq a5,0(s3); bne a5,s4,1f; sc.w.aq a4,zero,0(s3); bnez a4,1b; 1:
	addiw	a5,a5,-1
	li	s5,1
	bne	a5,zero,.L59
.L62:
	fence	iorw,iorw
	lw	a5,0(s2)
	fence	iorw,iorw
	beq	a5,zero,.L60
	fence	iorw,iorw
	fence	iorw,iorw
	ld	a5,24(s0)
	fence	iorw,iorw
	fence	iorw,iorw
	lw	a0,32(s0)
	fence	iorw,iorw
	fence	iorw,iorw
	sext.w	a0,a0
	lw	a1,36(s0)
	fence	iorw,iorw
	jalr	a5
	fence	iorw,iorw
	fence iorw,ow;  1: lr.w.aq a5,0(s1); bne a5,zero,1f; sc.w.aq a4,s4,0(s1); bnez a4,1b; 1:
	sext.w	a5,a5
	bne	a5,zero,.L61
	mv	a1,s1
	li	a6,0
	li	a4,0
	li	a3,1
	li	a2,1
	li	a0,98
	call	syscall@plt
.L61:
	fence	iorw,iorw
	lw	a5,0(s2)
	fence	iorw,iorw
	bne	a5,zero,.L75
.L60:
	ld	ra,184(sp)
	.cfi_remember_state
	.cfi_restore 1
	li	a0,0
	ld	s0,176(sp)
	.cfi_restore 8
	ld	s1,168(sp)
	.cfi_restore 9
	ld	s2,160(sp)
	.cfi_restore 18
	ld	s3,152(sp)
	.cfi_restore 19
	ld	s4,144(sp)
	.cfi_restore 20
	ld	s5,136(sp)
	.cfi_restore 21
	addi	sp,sp,192
	.cfi_def_cfa_offset 0
	jr	ra
.L59:
	.cfi_restore_state
	mv	a1,s3
	li	a6,0
	li	a5,0
	li	a4,0
	li	a3,0
	li	a2,0
	li	a0,98
	call	syscall@plt
	fence iorw,ow;  1: lr.w.aq a5,0(s3); bne a5,s5,1f; sc.w.aq a4,zero,0(s3); bnez a4,1b; 1:
	addiw	a5,a5,-1
	beq	a5,zero,.L62
	j	.L59
	.cfi_endproc
.LFE312:
	.size	_ZN12_GLOBAL__N_110cmmcWorkerEPv, .-_ZN12_GLOBAL__N_110cmmcWorkerEPv
	.align	1
	.globl	_memset
	.type	_memset, @function
_memset:
.LFB0:
	.cfi_startproc
	ble	a1,zero,.L76
	addiw	a2,a1,-1
	li	a1,0
	srliw	a2,a2,2
	addiw	a2,a2,1
	slli.uw	a2,a2,2
	tail	memset@plt
.L76:
	ret
	.cfi_endproc
.LFE0:
	.size	_memset, .-_memset
	.align	1
	.globl	sysycCacheLookup
	.type	sysycCacheLookup, @function
sysycCacheLookup:
.LFB3:
	.cfi_startproc
	slli	a1,a1,32
	li	a5,1021
	or	a2,a1,a2
	remu	a5,a2,a5
	slli	a5,a5,4
	add	a0,a0,a5
	lw	a5,12(a0)
	beq	a5,zero,.L81
	ld	a5,0(a0)
	beq	a5,a2,.L78
	sw	zero,12(a0)
.L81:
	sd	a2,0(a0)
.L78:
	ret
	.cfi_endproc
.LFE3:
	.size	sysycCacheLookup, .-sysycCacheLookup
	.section	.text.startup,"ax",@progbits
	.align	1
	.globl	cmmcInitRuntime
	.type	cmmcInitRuntime, @function
cmmcInitRuntime:
.LFB1226:
	.cfi_startproc
	addi	sp,sp,-64
	.cfi_def_cfa_offset 64
	sd	s2,32(sp)
	.cfi_offset 18, -32
	li	s2,331776
	sd	s3,24(sp)
	.cfi_offset 19, -40
	li	s3,131072
	addi	s3,s3,34
	addi	s2,s2,-256
	sd	s0,48(sp)
	.cfi_offset 8, -16
	lla	s0,.LANCHOR0+904
	sd	s1,40(sp)
	.cfi_offset 9, -24
	li	s1,0
	sd	s4,16(sp)
	.cfi_offset 20, -48
	li	s4,4
	sd	s5,8(sp)
	.cfi_offset 21, -56
	lla	s5,_ZN12_GLOBAL__N_110cmmcWorkerEPv
	sd	s6,0(sp)
	.cfi_offset 22, -64
	li	s6,1
	sd	ra,56(sp)
	.cfi_offset 1, -8
.L83:
	addi	a5,s0,20
	fence iorw,ow; amoswap.w.aq zero,s6,0(a5)
	mv	a3,s3
	li	a5,0
	li	a4,-1
	li	a2,3
	li	a1,1048576
	li	a0,0
	call	mmap@plt
	sd	a0,8(s0)
	addi	a5,s0,16
	fence iorw,ow; amoswap.w.aq zero,s1,0(a5)
	li	a5,1048576
	mv	a3,s0
	ld	a1,8(s0)
	mv	a2,s2
	add	a1,a1,a5
	mv	a0,s5
	addi	s0,s0,48
	addiw	s1,s1,1
	call	clone@plt
	sw	a0,-48(s0)
	bne	s1,s4,.L83
	ld	ra,56(sp)
	.cfi_restore 1
	ld	s0,48(sp)
	.cfi_restore 8
	ld	s1,40(sp)
	.cfi_restore 9
	ld	s2,32(sp)
	.cfi_restore 18
	ld	s3,24(sp)
	.cfi_restore 19
	ld	s4,16(sp)
	.cfi_restore 20
	ld	s5,8(sp)
	.cfi_restore 21
	ld	s6,0(sp)
	.cfi_restore 22
	addi	sp,sp,64
	.cfi_def_cfa_offset 0
	jr	ra
	.cfi_endproc
.LFE1226:
	.size	cmmcInitRuntime, .-cmmcInitRuntime
	.section	.init_array,"aw"
	.align	3
	.dword	cmmcInitRuntime
	.section	.text.exit,"ax",@progbits
	.align	1
	.globl	cmmcUninitRuntime
	.type	cmmcUninitRuntime, @function
cmmcUninitRuntime:
.LFB1227:
	.cfi_startproc
	addi	sp,sp,-32
	.cfi_def_cfa_offset 32
	sd	s0,16(sp)
	.cfi_offset 8, -16
	lla	s0,.LANCHOR0+944
	sd	s1,8(sp)
	.cfi_offset 9, -24
	li	s1,1
	sd	s2,0(sp)
	.cfi_offset 18, -32
	lla	s2,.LANCHOR0+1136
	sd	ra,24(sp)
	.cfi_offset 1, -8
.L90:
	addi	a5,s0,-20
	fence iorw,ow; amoswap.w.aq zero,zero,0(a5)
	fence iorw,ow;  1: lr.w.aq a7,0(s0); bne a7,zero,1f; sc.w.aq a5,s1,0(s0); bnez a5,1b; 1:
	sext.w	a7,a7
	mv	a1,s0
	li	a6,0
	li	a5,0
	li	a4,0
	li	a3,1
	li	a2,1
	li	a0,98
	bne	a7,zero,.L87
	addi	s0,s0,48
	call	syscall@plt
	li	a2,0
	li	a1,0
	lw	a0,-88(s0)
	call	waitpid@plt
	bne	s2,s0,.L90
.L86:
	ld	ra,24(sp)
	.cfi_remember_state
	.cfi_restore 1
	ld	s0,16(sp)
	.cfi_restore 8
	ld	s1,8(sp)
	.cfi_restore 9
	ld	s2,0(sp)
	.cfi_restore 18
	addi	sp,sp,32
	.cfi_def_cfa_offset 0
	jr	ra
.L87:
	.cfi_restore_state
	lw	a0,-40(s0)
	li	a2,0
	addi	s0,s0,48
	li	a1,0
	call	waitpid@plt
	bne	s0,s2,.L90
	j	.L86
	.cfi_endproc
.LFE1227:
	.size	cmmcUninitRuntime, .-cmmcUninitRuntime
	.section	.fini_array,"aw"
	.align	3
	.dword	cmmcUninitRuntime
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align	3
.LC0:
	.string	"threads %d\n"
	.align	3
.LC1:
	.string	"parallel for %d %d\n"
	.align	3
.LC2:
	.string	"launch %d %d\n"
	.text
	.align	1
	.globl	parallelForOld
	.type	parallelForOld, @function
parallelForOld:
.LFB1231:
	.cfi_startproc
	bge	a0,a1,.L124
	addi	sp,sp,-192
	.cfi_def_cfa_offset 192
	subw	a5,a1,a0
	li	a4,15
	sd	s0,176(sp)
	.cfi_offset 8, -16
	mv	s0,a2
	sd	s2,160(sp)
	.cfi_offset 18, -32
	mv	s2,a5
	sd	s9,104(sp)
	.cfi_offset 25, -88
	mv	s9,a1
	sd	s11,88(sp)
	.cfi_offset 27, -104
	mv	s11,a0
	sd	ra,184(sp)
	sd	s1,168(sp)
	sd	s3,152(sp)
	sd	s4,144(sp)
	sd	s5,136(sp)
	sd	s6,128(sp)
	sd	s7,120(sp)
	sd	s8,112(sp)
	sd	s10,96(sp)
	.cfi_offset 1, -8
	.cfi_offset 9, -24
	.cfi_offset 19, -40
	.cfi_offset 20, -48
	.cfi_offset 21, -56
	.cfi_offset 22, -64
	.cfi_offset 23, -72
	.cfi_offset 24, -80
	.cfi_offset 26, -96
	bgt	a5,a4,.L94
	ld	ra,184(sp)
	.cfi_remember_state
	.cfi_restore 1
	ld	s0,176(sp)
	.cfi_restore 8
	ld	s1,168(sp)
	.cfi_restore 9
	ld	s2,160(sp)
	.cfi_restore 18
	ld	s3,152(sp)
	.cfi_restore 19
	ld	s4,144(sp)
	.cfi_restore 20
	ld	s5,136(sp)
	.cfi_restore 21
	ld	s6,128(sp)
	.cfi_restore 22
	ld	s7,120(sp)
	.cfi_restore 23
	ld	s8,112(sp)
	.cfi_restore 24
	ld	s9,104(sp)
	.cfi_restore 25
	ld	s10,96(sp)
	.cfi_restore 26
	ld	s11,88(sp)
	.cfi_restore 27
	addi	sp,sp,192
	.cfi_def_cfa_offset 0
	jr	a2
.L94:
	.cfi_restore_state
	mv	a1,a5
	addi	a3,sp,59
	addi	a2,sp,60
	mv	a0,s0
	la	s6,stderr
	call	selectNumberOfThreads
	lw	s5,60(sp)
	mv	a5,a0
	mv	a2,s5
	ld	a0,0(s6)
	lla	a1,.LC0
	sd	a5,16(sp)
	call	fprintf@plt
	lbu	a5,59(sp)
	sd	a5,8(sp)
	bne	a5,zero,.L127
.L95:
	li	s3,1
	li	a5,1
	sllw	s4,s3,s5
	beq	s4,a5,.L128
	ld	a0,0(s6)
	mv	a3,s9
	mv	a2,s11
	lla	a1,.LC1
	call	fprintf@plt
	fence	iorw,iorw
	srlw	s2,s2,s5
	addiw	s2,s2,3
	andi	s2,s2,-4
	sw	zero,64(sp)
	sext.w	s2,s2
	ble	s4,zero,.L129
	addiw	s8,s4,-1
	addi	s10,sp,64
	sext.w	s11,s11
	lla	s3,.LANCHOR0+944
	sw	s8,28(sp)
	li	s7,0
	sd	s10,32(sp)
.L104:
	sext.w	s1,s11
	addw	s11,s11,s2
	min	a5,s11,s9
	bne s8,s7,1f; mv a5,s9; 1: # movcc
	sd	a5,0(sp)
	mv	a2,s1
	mv	a3,a5
	lla	a1,.LC2
	bge	s1,a5,.L102
	ld	a0,0(s6)
	call	fprintf@plt
	addi	a5,s3,-16
	fence iorw,ow; amoswap.d.aq zero,s0,0(a5)
	addi	a5,s3,-8
	fence iorw,ow; amoswap.w.aq zero,s1,0(a5)
	ld	a5,0(sp)
	addi	a4,s3,-4
	fence iorw,ow; amoswap.w.aq zero,a5,0(a4)
	li	a4,1
	fence iorw,ow;  1: lr.w.aq t6,0(s3); bne t6,zero,1f; sc.w.aq a5,a4,0(s3); bnez a5,1b; 1:
	sext.w	t6,t6
	mv	a1,s3
	li	a6,0
	li	a5,0
	li	a4,0
	li	a3,1
	li	a2,1
	li	a0,98
	bne	t6,zero,.L103
	call	syscall@plt
.L103:
	li	a5,1
	sb	a5,0(s10)
.L102:
	addiw	s7,s7,1
	addi	s10,s10,1
	addi	s3,s3,48
	bne	s4,s7,.L104
.L99:
	ld	s1,32(sp)
	lla	s0,.LANCHOR0+948
	li	s2,1
	addi	a5,sp,65
	lw	a4,28(sp)
	add.uw	s3,a4,a5
.L106:
	lbu	a5,0(s1)
	bne	a5,zero,.L105
.L107:
	addi	s1,s1,1
	addi	s0,s0,48
	bne	s3,s1,.L106
.L100:
	fence	iorw,iorw
	ld	a5,8(sp)
	bne	a5,zero,.L130
.L92:
	ld	ra,184(sp)
	.cfi_restore 1
	ld	s0,176(sp)
	.cfi_restore 8
	ld	s1,168(sp)
	.cfi_restore 9
	ld	s2,160(sp)
	.cfi_restore 18
	ld	s3,152(sp)
	.cfi_restore 19
	ld	s4,144(sp)
	.cfi_restore 20
	ld	s5,136(sp)
	.cfi_restore 21
	ld	s6,128(sp)
	.cfi_restore 22
	ld	s7,120(sp)
	.cfi_restore 23
	ld	s8,112(sp)
	.cfi_restore 24
	ld	s9,104(sp)
	.cfi_restore 25
	ld	s10,96(sp)
	.cfi_restore 26
	ld	s11,88(sp)
	.cfi_restore 27
	addi	sp,sp,192
	.cfi_def_cfa_offset 0
	jr	ra
.L124:
	ret
.L105:
	.cfi_def_cfa_offset 192
	.cfi_offset 1, -8
	.cfi_offset 8, -16
	.cfi_offset 9, -24
	.cfi_offset 18, -32
	.cfi_offset 19, -40
	.cfi_offset 20, -48
	.cfi_offset 21, -56
	.cfi_offset 22, -64
	.cfi_offset 23, -72
	.cfi_offset 24, -80
	.cfi_offset 25, -88
	.cfi_offset 26, -96
	.cfi_offset 27, -104
	fence iorw,ow;  1: lr.w.aq a5,0(s0); bne a5,s2,1f; sc.w.aq a4,zero,0(s0); bnez a4,1b; 1:
	addiw	a5,a5,-1
	beq	a5,zero,.L107
	li	s4,1
.L108:
	mv	a1,s0
	li	a6,0
	li	a5,0
	li	a4,0
	li	a3,0
	li	a2,0
	li	a0,98
	call	syscall@plt
	fence iorw,ow;  1: lr.w.aq a5,0(s0); bne a5,s4,1f; sc.w.aq a4,zero,0(s0); bnez a4,1b; 1:
	addiw	a5,a5,-1
	beq	a5,zero,.L107
	j	.L108
.L128:
	mv	a1,s9
	mv	a0,s11
	jalr	s0
	ld	a5,8(sp)
	beq	a5,zero,.L92
.L130:
	addi	a1,sp,64
	li	a0,1
	call	clock_gettime@plt
	ld	a5,16(sp)
	li	a4,1000001536
	sh3add.uw	s5,s5,a5
	ld	a5,64(sp)
	addi	a4,a4,-1536
	ld	a3,72(sp)
	mul	a5,a5,a4
	ld	a4,24(s5)
	add	a5,a5,a3
	ld	a3,40(sp)
	sub	a5,a5,a3
	add	a5,a4,a5
	sd	a5,24(s5)
	j	.L92
.L127:
	addi	a1,sp,64
	li	a0,1
	call	clock_gettime@plt
	ld	a5,64(sp)
	li	a4,1000001536
	addi	a4,a4,-1536
	mul	a5,a5,a4
	ld	a4,72(sp)
	add	a5,a5,a4
	sd	a5,40(sp)
	j	.L95
.L129:
	beq	s4,zero,.L100
	addi	a5,sp,64
	sd	a5,32(sp)
	addiw	a5,s4,-1
	sw	a5,28(sp)
	j	.L99
	.cfi_endproc
.LFE1231:
	.size	parallelForOld, .-parallelForOld
	.align	1
	.globl	parallelFor
	.type	parallelFor, @function
parallelFor:
.LFB1233:
	.cfi_startproc
	beq	a0,a1,.L172
	addi	sp,sp,-176
	.cfi_def_cfa_offset 176
	subw	a5,a1,a0
	sd	s1,152(sp)
	.cfi_offset 9, -24
	subw	s1,a0,a1
	bge a0,a1,1f; mv s1,a5; 1: # movcc
	sd	s0,160(sp)
	li	a5,15
	sd	s2,144(sp)
	.cfi_offset 8, -16
	.cfi_offset 18, -32
	mv	s0,a1
	sd	s3,136(sp)
	mv	s2,a0
	sd	ra,168(sp)
	.cfi_offset 19, -40
	.cfi_offset 1, -8
	mv	s3,a2
	sd	s4,128(sp)
	sd	s5,120(sp)
	sd	s6,112(sp)
	sd	s7,104(sp)
	sd	s8,96(sp)
	sd	s9,88(sp)
	sd	s10,80(sp)
	sd	s11,72(sp)
	.cfi_offset 20, -48
	.cfi_offset 21, -56
	.cfi_offset 22, -64
	.cfi_offset 23, -72
	.cfi_offset 24, -80
	.cfi_offset 25, -88
	.cfi_offset 26, -96
	.cfi_offset 27, -104
	bgtu	s1,a5,.L135
	ld	ra,168(sp)
	.cfi_remember_state
	.cfi_restore 1
	ld	s0,160(sp)
	.cfi_restore 8
	ld	s1,152(sp)
	.cfi_restore 9
	ld	s2,144(sp)
	.cfi_restore 18
	ld	s3,136(sp)
	.cfi_restore 19
	ld	s4,128(sp)
	.cfi_restore 20
	ld	s5,120(sp)
	.cfi_restore 21
	ld	s6,112(sp)
	.cfi_restore 22
	ld	s7,104(sp)
	.cfi_restore 23
	ld	s8,96(sp)
	.cfi_restore 24
	ld	s9,88(sp)
	.cfi_restore 25
	ld	s10,80(sp)
	.cfi_restore 26
	ld	s11,72(sp)
	.cfi_restore 27
	addi	sp,sp,176
	.cfi_def_cfa_offset 0
	jr	a2
.L135:
	.cfi_restore_state
	addi	a3,sp,43
	addi	a2,sp,44
	mv	a1,s1
	mv	a0,s3
	call	selectNumberOfThreads
	lbu	s7,43(sp)
	sd	a0,16(sp)
	bne	s7,zero,.L175
.L136:
	li	s4,1
	li	a5,1
	lw	s5,44(sp)
	sllw	s9,s4,s5
	mv	s4,s9
	beq	s9,a5,.L176
	fence	iorw,iorw
	srlw	s1,s1,s5
	addiw	s1,s1,3
	andi	s1,s1,-4
	sw	zero,48(sp)
	sext.w	s1,s1
	ble	s9,zero,.L177
	addi	a5,sp,48
	sext.w	a3,s2
	subw	s10,s2,s1
	addiw	s6,s9,-1
	sd	a5,8(sp)
	lla	s8,.LANCHOR0+944
	li	s11,0
.L150:
	addw	a2,s1,a3
	sext.w	a0,s11
	mv	t4,a2
	max	a5,s10,s0
	addw	a1,s10,s1
	min	a2,a2,s0
	bne s6,a0,1f; mv a5,s0; 1: # movcc
	bge	s2,s0,.L142
	mv	a5,a2
	sext.w	a1,a3
	bne s6,a0,1f; mv a5,s0; 1: # movcc
	ble	a5,a3,.L147
.L148:
	addi	a4,s8,-16
	fence iorw,ow; amoswap.d.aq zero,s3,0(a4)
	addi	a4,s8,-8
	fence iorw,ow; amoswap.w.aq zero,a1,0(a4)
	addi	a4,s8,-4
	fence iorw,ow; amoswap.w.aq zero,a5,0(a4)
	li	a4,1
	fence iorw,ow;  1: lr.w.aq t0,0(s8); bne t0,zero,1f; sc.w.aq a5,a4,0(s8); bnez a5,1b; 1:
	sext.w	t0,t0
	mv	a1,s8
	li	a6,0
	li	a5,0
	li	a4,0
	li	a3,1
	li	a2,1
	li	a0,98
	bne	t0,zero,.L149
	sw	t4,4(sp)
	call	syscall@plt
	lw	t4,4(sp)
.L149:
	ld	a5,8(sp)
	li	a4,1
	add	a5,a5,s11
	sb	a4,0(a5)
.L147:
	addi	s11,s11,1
	sext.w	a3,t4
	addi	s8,s8,48
	subw	s10,s10,s1
	bne	s9,s11,.L150
.L140:
	ld	s3,8(sp)
	lla	s0,.LANCHOR0+948
	li	s1,1
	addiw	s4,s4,-1
	addi	a5,sp,49
	add.uw	s4,s4,a5
.L152:
	lbu	a5,0(s3)
	bne	a5,zero,.L151
.L153:
	addi	s3,s3,1
	addi	s0,s0,48
	bne	s3,s4,.L152
.L141:
	fence	iorw,iorw
	bne	s7,zero,.L178
.L131:
	ld	ra,168(sp)
	.cfi_remember_state
	.cfi_restore 1
	ld	s0,160(sp)
	.cfi_restore 8
	ld	s1,152(sp)
	.cfi_restore 9
	ld	s2,144(sp)
	.cfi_restore 18
	ld	s3,136(sp)
	.cfi_restore 19
	ld	s4,128(sp)
	.cfi_restore 20
	ld	s5,120(sp)
	.cfi_restore 21
	ld	s6,112(sp)
	.cfi_restore 22
	ld	s7,104(sp)
	.cfi_restore 23
	ld	s8,96(sp)
	.cfi_restore 24
	ld	s9,88(sp)
	.cfi_restore 25
	ld	s10,80(sp)
	.cfi_restore 26
	ld	s11,72(sp)
	.cfi_restore 27
	addi	sp,sp,176
	.cfi_def_cfa_offset 0
	jr	ra
.L142:
	.cfi_restore_state
	addw	t4,s1,a3
	ble	a1,a5,.L147
	j	.L148
.L151:
	fence iorw,ow;  1: lr.w.aq a5,0(s0); bne a5,s1,1f; sc.w.aq a4,zero,0(s0); bnez a4,1b; 1:
	addiw	a5,a5,-1
	beq	a5,zero,.L153
	li	s2,1
.L154:
	mv	a1,s0
	li	a6,0
	li	a5,0
	li	a4,0
	li	a3,0
	li	a2,0
	li	a0,98
	call	syscall@plt
	fence iorw,ow;  1: lr.w.aq a5,0(s0); bne a5,s2,1f; sc.w.aq a4,zero,0(s0); bnez a4,1b; 1:
	addiw	a5,a5,-1
	beq	a5,zero,.L153
	j	.L154
.L176:
	mv	a1,s0
	mv	a0,s2
	jalr	s3
	beq	s7,zero,.L131
.L178:
	addi	a1,sp,48
	li	a0,1
	call	clock_gettime@plt
	ld	a5,16(sp)
	li	a4,1000001536
	sh3add.uw	s5,s5,a5
	ld	a5,48(sp)
	addi	a4,a4,-1536
	ld	a3,56(sp)
	mul	a5,a5,a4
	ld	a4,24(s5)
	add	a5,a5,a3
	ld	a3,24(sp)
	sub	a5,a5,a3
	add	a5,a4,a5
	sd	a5,24(s5)
	j	.L131
.L175:
	addi	a1,sp,48
	li	a0,1
	call	clock_gettime@plt
	ld	s8,48(sp)
	li	a5,1000001536
	addi	a5,a5,-1536
	mul	s8,s8,a5
	ld	a5,56(sp)
	add	a5,s8,a5
	sd	a5,24(sp)
	j	.L136
.L172:
	.cfi_def_cfa_offset 0
	.cfi_restore 1
	.cfi_restore 8
	.cfi_restore 9
	.cfi_restore 18
	.cfi_restore 19
	.cfi_restore 20
	.cfi_restore 21
	.cfi_restore 22
	.cfi_restore 23
	.cfi_restore 24
	.cfi_restore 25
	.cfi_restore 26
	.cfi_restore 27
	ret
.L177:
	.cfi_def_cfa_offset 176
	.cfi_offset 1, -8
	.cfi_offset 8, -16
	.cfi_offset 9, -24
	.cfi_offset 18, -32
	.cfi_offset 19, -40
	.cfi_offset 20, -48
	.cfi_offset 21, -56
	.cfi_offset 22, -64
	.cfi_offset 23, -72
	.cfi_offset 24, -80
	.cfi_offset 25, -88
	.cfi_offset 26, -96
	.cfi_offset 27, -104
	beq	s9,zero,.L141
	addi	a5,sp,48
	sd	a5,8(sp)
	j	.L140
	.cfi_endproc
.LFE1233:
	.size	parallelFor, .-parallelFor
	.bss
	.align	3
	.set	.LANCHOR0,. + 0
	.type	_ZL13parallelCache, @object
	.size	_ZL13parallelCache, 896
_ZL13parallelCache:
	.zero	896
	.type	_ZL9lookupPtr, @object
	.size	_ZL9lookupPtr, 4
_ZL9lookupPtr:
	.zero	4
	.zero	4
	.type	_ZN12_GLOBAL__N_17workersE, @object
	.size	_ZN12_GLOBAL__N_17workersE, 192
_ZN12_GLOBAL__N_17workersE:
	.zero	192
	.ident	"GCC: (Debian 12.2.0-13) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
)"