// Automatically generated file, do not edit!
// Command: riscv64-linux-gnu-g++-12 -Ofast -DNDEBUG -march=rv64gc_zba_zbb -fno-stack-protector -fomit-frame-pointer -mcpu=sifive-u74 -mabi=lp64d -mcmodel=medlow -ffp-contract=on -w /home/hhw/Desktop/compilers/sys-ycompiler/src/runtime/.merge.cpp -S -o /dev/stdout
R"(	.file	".merge.cpp"
	.option pic
	.attribute arch, "rv64i2p1_m2p0_a2p1_f2p2_d2p2_c2p0_zicsr2p0_zifencei2p0_zba1p0_zbb1p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
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
	bgtu	a5,a4,.L2
	li	a3,1
	srli	a5,a5,6
	sll	a3,a3,a2
	sh3add	a5,a5,s1
	ld	a4,0(a5)
	or	a4,a4,a3
	sd	a4,0(a5)
.L2:
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
	j	.L6
.L21:
	fence iorw,ow;  1: lr.w.aq a5,0(s3); bne a5,s4,1f; sc.w.aq a4,zero,0(s3); bnez a4,1b; 1:
	addiw	a5,a5,-1
	li	s5,1
	bne	a5,zero,.L4
.L7:
	fence	iorw,iorw
	lw	a5,0(s2)
	fence	iorw,iorw
	beq	a5,zero,.L5
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
	bne	a5,zero,.L6
	mv	a1,s1
	li	a6,0
	li	a4,0
	li	a3,1
	li	a2,1
	li	a0,98
	call	syscall@plt
.L6:
	fence	iorw,iorw
	lw	a5,0(s2)
	fence	iorw,iorw
	bne	a5,zero,.L21
.L5:
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
.L4:
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
	beq	a5,zero,.L7
	j	.L4
	.cfi_endproc
.LFE312:
	.size	_ZN12_GLOBAL__N_110cmmcWorkerEPv, .-_ZN12_GLOBAL__N_110cmmcWorkerEPv
	.align	1
	.globl	_memset
	.type	_memset, @function
_memset:
.LFB0:
	.cfi_startproc
	ble	a1,zero,.L22
	addiw	a2,a1,-1
	li	a1,0
	srliw	a2,a2,2
	addiw	a2,a2,1
	slli.uw	a2,a2,2
	tail	memset@plt
.L22:
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
	beq	a5,zero,.L27
	ld	a5,0(a0)
	beq	a5,a2,.L24
	sw	zero,12(a0)
.L27:
	sd	a2,0(a0)
.L24:
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
	lla	s0,.LANCHOR0
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
.L29:
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
	bne	s1,s4,.L29
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
	lla	s0,.LANCHOR0+40
	sd	s1,8(sp)
	.cfi_offset 9, -24
	li	s1,1
	sd	s2,0(sp)
	.cfi_offset 18, -32
	lla	s2,.LANCHOR0+232
	sd	ra,24(sp)
	.cfi_offset 1, -8
.L36:
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
	bne	a7,zero,.L33
	addi	s0,s0,48
	call	syscall@plt
	li	a2,0
	li	a1,0
	lw	a0,-88(s0)
	call	waitpid@plt
	bne	s2,s0,.L36
.L32:
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
.L33:
	.cfi_restore_state
	lw	a0,-40(s0)
	li	a2,0
	addi	s0,s0,48
	li	a1,0
	call	waitpid@plt
	bne	s0,s2,.L36
	j	.L32
	.cfi_endproc
.LFE1227:
	.size	cmmcUninitRuntime, .-cmmcUninitRuntime
	.section	.fini_array,"aw"
	.align	3
	.dword	cmmcUninitRuntime
	.text
	.align	1
	.globl	parallelFor
	.type	parallelFor, @function
parallelFor:
.LFB1231:
	.cfi_startproc
	beq	a0,a1,.L127
	addi	sp,sp,-160
	.cfi_def_cfa_offset 160
	subw	a5,a1,a0
	sd	s3,120(sp)
	.cfi_offset 19, -40
	subw	s3,a0,a1
	bge a0,a1,1f; mv s3,a5; 1: # movcc
	sd	s1,136(sp)
	li	a5,15
	sd	s4,112(sp)
	.cfi_offset 9, -24
	.cfi_offset 20, -48
	mv	s1,a1
	sd	s5,104(sp)
	mv	s4,a0
	sd	ra,152(sp)
	.cfi_offset 21, -56
	.cfi_offset 1, -8
	mv	s5,a2
	sd	s0,144(sp)
	sd	s2,128(sp)
	sd	s6,96(sp)
	sd	s7,88(sp)
	sd	s8,80(sp)
	sd	s9,72(sp)
	sd	s10,64(sp)
	sd	s11,56(sp)
	.cfi_offset 8, -16
	.cfi_offset 18, -32
	.cfi_offset 22, -64
	.cfi_offset 23, -72
	.cfi_offset 24, -80
	.cfi_offset 25, -88
	.cfi_offset 26, -96
	.cfi_offset 27, -104
	bgtu	s3,a5,.L42
	ld	ra,152(sp)
	.cfi_remember_state
	.cfi_restore 1
	ld	s0,144(sp)
	.cfi_restore 8
	ld	s1,136(sp)
	.cfi_restore 9
	ld	s2,128(sp)
	.cfi_restore 18
	ld	s3,120(sp)
	.cfi_restore 19
	ld	s4,112(sp)
	.cfi_restore 20
	ld	s5,104(sp)
	.cfi_restore 21
	ld	s6,96(sp)
	.cfi_restore 22
	ld	s7,88(sp)
	.cfi_restore 23
	ld	s8,80(sp)
	.cfi_restore 24
	ld	s9,72(sp)
	.cfi_restore 25
	ld	s10,64(sp)
	.cfi_restore 26
	ld	s11,56(sp)
	.cfi_restore 27
	addi	sp,sp,160
	.cfi_def_cfa_offset 0
	jr	a2
.L42:
	.cfi_restore_state
	lla	a7,.LANCHOR0
	li	a3,16
	lw	a4,1088(a7)
	lla	a0,.LANCHOR0+192
	li	a2,16
.L46:
	beq	a4,a2,.L92
	zext.w	a6,a4
	slli.uw	a5,a4,3
	sub	a5,a5,a6
	sh3add	s0,a5,a0
.L43:
	slli	a5,a6,3
	addiw	a3,a3,-1
	sub	a5,a5,a6
	sh3add	a5,a5,a7
	lbu	a1,204(a5)
	beq	a1,zero,.L44
	ld	a1,192(a5)
	beq	a1,s5,.L130
.L44:
	addiw	a4,a4,1
	bne	a3,zero,.L46
	lbu	a5,204(a7)
	beq	a5,zero,.L47
	lbu	a5,260(a7)
	beq	a5,zero,.L93
	lbu	a5,316(a7)
	beq	a5,zero,.L94
	lbu	a5,372(a7)
	beq	a5,zero,.L95
	lbu	a5,428(a7)
	beq	a5,zero,.L96
	lbu	a5,484(a7)
	beq	a5,zero,.L97
	lbu	a5,540(a7)
	beq	a5,zero,.L98
	lbu	a5,596(a7)
	beq	a5,zero,.L99
	lbu	a5,652(a7)
	beq	a5,zero,.L100
	lbu	a5,708(a7)
	beq	a5,zero,.L101
	lbu	a5,764(a7)
	beq	a5,zero,.L102
	lbu	a5,820(a7)
	beq	a5,zero,.L103
	lbu	a5,876(a7)
	beq	a5,zero,.L104
	lbu	a5,932(a7)
	beq	a5,zero,.L105
	lbu	a5,988(a7)
	beq	a5,zero,.L106
	lbu	a5,1044(a7)
	li	a3,15
	beq	a5,zero,.L47
	lw	a2,264(a7)
	lw	a5,208(a7)
	lw	a6,320(a7)
	sgtu	a3,a5,a2
	lw	t1,376(a7)
	bleu a5,a2,1f; mv a5,a2; 1: # movcc
	mv	a2,a5
	bgeu a6,a5,1f; mv a2,a6; 1: # movcc
	lw	a4,432(a7)
	bgeu a6,a5,1f; li a3,2; 1: # movcc
	mv	a5,a2
	bgeu t1,a2,1f; mv a5,t1; 1: # movcc
	lw	a1,488(a7)
	bgeu t1,a2,1f; li a3,3; 1: # movcc
	mv	a2,a5
	bgeu a4,a5,1f; mv a2,a4; 1: # movcc
	lw	a6,544(a7)
	bgeu a4,a5,1f; li a3,4; 1: # movcc
	mv	a5,a2
	bgeu a1,a2,1f; mv a5,a1; 1: # movcc
	lw	t1,600(a7)
	bgeu a1,a2,1f; li a3,5; 1: # movcc
	mv	a1,a5
	bgeu a6,a5,1f; mv a1,a6; 1: # movcc
	lw	a4,656(a7)
	bgeu a6,a5,1f; li a3,6; 1: # movcc
	mv	a6,a1
	bgeu t1,a1,1f; mv a6,t1; 1: # movcc
	lw	a2,712(a7)
	bgeu t1,a1,1f; li a3,7; 1: # movcc
	mv	t1,a6
	bleu a6,a4,1f; mv t1,a4; 1: # movcc
	lw	a5,768(a7)
	bleu a6,a4,1f; li a3,8; 1: # movcc
	mv	a6,t1
	bleu t1,a2,1f; mv a6,a2; 1: # movcc
	lw	a1,824(a7)
	bleu t1,a2,1f; li a3,9; 1: # movcc
	mv	t1,a6
	bleu a6,a5,1f; mv t1,a5; 1: # movcc
	lw	a4,880(a7)
	bleu a6,a5,1f; li a3,10; 1: # movcc
	mv	a6,t1
	bleu t1,a1,1f; mv a6,a1; 1: # movcc
	lw	a2,936(a7)
	bleu t1,a1,1f; li a3,11; 1: # movcc
	mv	a1,a6
	bleu a6,a4,1f; mv a1,a4; 1: # movcc
	lw	a5,992(a7)
	bleu a6,a4,1f; li a3,12; 1: # movcc
	mv	a4,a1
	bleu a1,a2,1f; mv a4,a2; 1: # movcc
	lw	t1,1048(a7)
	bleu a1,a2,1f; li a3,13; 1: # movcc
	mv	a2,a4
	bleu a4,a5,1f; mv a2,a5; 1: # movcc
	bleu a4,a5,1f; li a3,14; 1: # movcc
	bgeu t1,a2,1f; li a3,15; 1: # movcc
	slli	a5,a3,3
	sub	a5,a5,a3
	sh3add	a4,a5,a7
	sh3add	s0,a5,a0
	li	a5,1
	sw	a3,1088(a7)
	sd	s5,192(a4)
	sw	s3,200(a4)
	sw	a5,208(a4)
.L45:
	lw	s6,16(s0)
	li	a5,99
	bleu	s6,a5,.L64
	li	a5,159
	bleu	s6,a5,.L131
	lw	s6,48(s0)
	beq	s6,zero,.L67
	li	t5,1
	sllw	s8,t5,s6
	mv	s2,s8
.L68:
	sd	zero,8(sp)
.L66:
	li	a5,1
	bne	s2,a5,.L71
	mv	a1,s1
	mv	a0,s4
	jalr	s5
	ld	a5,8(sp)
	bne	a5,zero,.L132
.L38:
	ld	ra,152(sp)
	.cfi_remember_state
	.cfi_restore 1
	ld	s0,144(sp)
	.cfi_restore 8
	ld	s1,136(sp)
	.cfi_restore 9
	ld	s2,128(sp)
	.cfi_restore 18
	ld	s3,120(sp)
	.cfi_restore 19
	ld	s4,112(sp)
	.cfi_restore 20
	ld	s5,104(sp)
	.cfi_restore 21
	ld	s6,96(sp)
	.cfi_restore 22
	ld	s7,88(sp)
	.cfi_restore 23
	ld	s8,80(sp)
	.cfi_restore 24
	ld	s9,72(sp)
	.cfi_restore 25
	ld	s10,64(sp)
	.cfi_restore 26
	ld	s11,56(sp)
	.cfi_restore 27
	addi	sp,sp,160
	.cfi_def_cfa_offset 0
	jr	ra
.L92:
	.cfi_restore_state
	mv	s0,a0
	li	a6,0
	li	a4,0
	j	.L43
.L130:
	lw	a1,200(a5)
	bne	a1,s3,.L44
	lw	a3,208(a5)
	addiw	a3,a3,1
	sw	a4,1088(a7)
	sw	a3,208(a5)
	j	.L45
.L67:
	ld	a5,24(s0)
	li	s2,1
	ld	a4,32(s0)
	li	s8,1
	bge	a4,a5,.L69
	mv	a5,a4
	li	s2,2
	li	s8,2
	li	s6,1
.L69:
	ld	a4,40(s0)
	ble	a5,a4,.L70
	li	a5,2
	sw	a5,48(s0)
.L64:
	fence	iorw,iorw
	srliw	s3,s3,2
	li	s8,4
	addiw	s3,s3,3
	li	s6,2
	andi	s3,s3,-4
	li	s2,4
	sext.w	s3,s3
	sd	zero,8(sp)
	sw	zero,32(sp)
.L91:
	addi	a5,sp,32
	sext.w	a3,s4
	subw	s7,s4,s3
	addiw	s10,s2,-1
	sd	a5,16(sp)
	lla	s9,.LANCHOR0+40
	li	s11,0
.L84:
	addw	a2,s3,a3
	sext.w	a0,s11
	mv	t3,a2
	max	a5,s7,s1
	addw	a1,s3,s7
	min	a2,a2,s1
	bne s10,a0,1f; mv a5,s1; 1: # movcc
	bge	s4,s1,.L76
	mv	a5,a2
	sext.w	a1,a3
	bne s10,a0,1f; mv a5,s1; 1: # movcc
	ble	a5,a3,.L81
.L82:
	addi	a4,s9,-16
	fence iorw,ow; amoswap.d.aq zero,s5,0(a4)
	addi	a4,s9,-8
	fence iorw,ow; amoswap.w.aq zero,a1,0(a4)
	addi	a4,s9,-4
	fence iorw,ow; amoswap.w.aq zero,a5,0(a4)
	li	a4,1
	fence iorw,ow;  1: lr.w.aq t0,0(s9); bne t0,zero,1f; sc.w.aq a5,a4,0(s9); bnez a5,1b; 1:
	sext.w	t0,t0
	mv	a1,s9
	li	a6,0
	li	a5,0
	li	a4,0
	li	a3,1
	li	a2,1
	li	a0,98
	bne	t0,zero,.L83
	sw	t3,4(sp)
	call	syscall@plt
	lw	t3,4(sp)
.L83:
	ld	a5,16(sp)
	li	a4,1
	add	a5,a5,s11
	sb	a4,0(a5)
.L81:
	addi	s11,s11,1
	sext.w	a3,t3
	addi	s9,s9,48
	subw	s7,s7,s3
	bne	s11,s8,.L84
.L85:
	beq	s2,zero,.L75
	lla	s3,.LANCHOR0+44
	li	s4,1
	addi	s1,sp,32
	add.uw	s2,s2,s1
.L87:
	lbu	a5,0(s1)
	bne	a5,zero,.L86
.L88:
	addi	s1,s1,1
	addi	s3,s3,48
	bne	s1,s2,.L87
.L75:
	fence	iorw,iorw
	ld	a5,8(sp)
	beq	a5,zero,.L38
.L132:
	addi	a1,sp,32
	li	a0,1
	sh3add.uw	s6,s6,s0
	call	clock_gettime@plt
	ld	a5,32(sp)
	li	a4,1000001536
	addi	a4,a4,-1536
	ld	a3,40(sp)
	mul	a5,a5,a4
	ld	a4,24(s6)
	add	a5,a5,a3
	ld	a3,24(sp)
	sub	a5,a5,a3
	add	a5,a4,a5
	sd	a5,24(s6)
	j	.L38
.L76:
	addw	t3,s3,a3
	ble	a1,a5,.L81
	j	.L82
.L86:
	fence iorw,ow;  1: lr.w.aq a5,0(s3); bne a5,s4,1f; sc.w.aq a4,zero,0(s3); bnez a4,1b; 1:
	addiw	a5,a5,-1
	beq	a5,zero,.L88
	li	s5,1
.L89:
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
	beq	a5,zero,.L88
	j	.L89
.L93:
	li	a3,1
.L47:
	li	a4,1
	zext.w	a2,a3
	sw	a3,1088(a7)
	slli.uw	a5,a3,3
	sub	a5,a5,a2
	sh3add	a7,a5,a7
	sh3add	s0,a5,a0
	sb	a4,204(a7)
	sd	s5,192(a7)
	sw	s3,200(a7)
	sw	a4,208(a7)
	j	.L45
.L131:
	addiw	s6,s6,-100
	li	a5,20
	addi	a1,sp,32
	li	a0,1
	divuw	s6,s6,a5
	li	a5,1
	sd	a5,8(sp)
	call	clock_gettime@plt
	li	a5,1000001536
	li	t5,1
	ld	s2,32(sp)
	addi	a5,a5,-1536
	mul	s2,s2,a5
	ld	a5,40(sp)
	add	a5,s2,a5
	sd	a5,24(sp)
	sllw	s8,t5,s6
	mv	s2,s8
	j	.L66
.L127:
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
.L71:
	.cfi_def_cfa_offset 160
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
	fence	iorw,iorw
	sw	zero,32(sp)
	ble	s8,zero,.L85
	srlw	s3,s3,s6
	addiw	s3,s3,3
	andi	s3,s3,-4
	sext.w	s3,s3
	j	.L91
.L70:
	sw	s6,48(s0)
	j	.L68
.L100:
	li	a3,8
	j	.L47
.L94:
	li	a3,2
	j	.L47
.L95:
	li	a3,3
	j	.L47
.L96:
	li	a3,4
	j	.L47
.L97:
	li	a3,5
	j	.L47
.L98:
	li	a3,6
	j	.L47
.L99:
	li	a3,7
	j	.L47
.L101:
	li	a3,9
	j	.L47
.L102:
	li	a3,10
	j	.L47
.L103:
	li	a3,11
	j	.L47
.L104:
	li	a3,12
	j	.L47
.L105:
	li	a3,13
	j	.L47
.L106:
	li	a3,14
	j	.L47
	.cfi_endproc
.LFE1231:
	.size	parallelFor, .-parallelFor
	.bss
	.align	3
	.set	.LANCHOR0,. + 0
	.type	_ZN12_GLOBAL__N_17workersE, @object
	.size	_ZN12_GLOBAL__N_17workersE, 192
_ZN12_GLOBAL__N_17workersE:
	.zero	192
	.type	_ZL13parallelCache, @object
	.size	_ZL13parallelCache, 896
_ZL13parallelCache:
	.zero	896
	.type	_ZL9lookupPtr, @object
	.size	_ZL9lookupPtr, 4
_ZL9lookupPtr:
	.zero	4
	.ident	"GCC: (Debian 12.2.0-13) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
)"