/* $Id: sysLib.s,v 1.4 2011/07/21 16:14:58 yshtil Exp $ */
	.file	1 "sysLib.c"
gcc2_compiled.:
__gnu_compiled_c:
	.text
	.align	2
	.globl	sysNvRamGet
	.file	2 "/projects/ntsw-tools/wrs/tornado/t2.1.1/target/src/drv/mem/nullNvRam.c"
	.ent	sysNvRamGet
sysNvRamGet:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	j	$31
	li	$2,-1			# 0xffffffff
	.set	macro
	.set	reorder

	.end	sysNvRamGet
	.align	2
	.globl	sysNvRamSet
	.ent	sysNvRamSet
sysNvRamSet:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	j	$31
	li	$2,-1			# 0xffffffff
	.set	macro
	.set	reorder

	.end	sysNvRamSet
	.data
	.align	2
	.type	 timerregs,@object
	.size	 timerregs,4
timerregs:
	.word	0
	.globl	sysCpu
	.align	2
	.type	 sysCpu,@object
	.size	 sysCpu,4
sysCpu:
	.word	41
	.globl	sysMemTopAdr
	.align	2
	.type	 sysMemTopAdr,@object
	.size	 sysMemTopAdr,4
sysMemTopAdr:
	.word	0
	.globl	sysBootLine
	.align	2
	.type	 sysBootLine,@object
	.size	 sysBootLine,4
sysBootLine:
	.word	-2147481856
	.globl	sysExcMsg
	.align	2
	.type	 sysExcMsg,@object
	.size	 sysExcMsg,4
sysExcMsg:
	.word	-2147481600
	.globl	sysVectorIRQ0
	.align	2
	.type	 sysVectorIRQ0,@object
	.size	 sysVectorIRQ0,4
sysVectorIRQ0:
	.word	65
	.globl	sysMemTopDebug
	.align	2
	.type	 sysMemTopDebug,@object
	.size	 sysMemTopDebug,4
sysMemTopDebug:
	.word	0
	.globl	sysSoftCompare
	.align	2
	.type	 sysSoftCompare,@object
	.size	 sysSoftCompare,4
sysSoftCompare:
	.word	0
	.globl	sysHashOrder
	.align	2
	.type	 sysHashOrder,@object
	.size	 sysHashOrder,4
sysHashOrder:
	.word	ffsMsbTbl
	.globl	intPrioTable
	.align	2
	.type	 intPrioTable,@object
	.size	 intPrioTable,128
intPrioTable:
	.word	256
	.word	32
	.word	256
	.word	0
	.word	512
	.word	33
	.word	512
	.word	0
	.word	1024
	.word	65
	.word	1024
	.word	0
	.word	2048
	.word	66
	.word	2048
	.word	0
	.word	4096
	.word	67
	.word	4096
	.word	0
	.word	8192
	.word	68
	.word	8192
	.word	0
	.word	16384
	.word	69
	.word	16384
	.word	0
	.word	32768
	.word	70
	.word	32768
	.word	0
	.globl	bringupPrintRtn
	.align	2
	.type	 bringupPrintRtn,@object
	.size	 bringupPrintRtn,4
bringupPrintRtn:
	.word	0
	.globl	sysCacheLibInit
	.align	2
	.type	 sysCacheLibInit,@object
	.size	 sysCacheLibInit,4
sysCacheLibInit:
	.word	cacheBcm47xxLibInit
	.align	2
	.type	 sysClkTicksPerSecond,@object
	.size	 sysClkTicksPerSecond,4
sysClkTicksPerSecond:
	.word	100
	.align	2
	.type	 sysClkArg,@object
	.size	 sysClkArg,4
sysClkArg:
	.word	0
	.align	2
	.type	 sysClkConnected,@object
	.size	 sysClkConnected,4
sysClkConnected:
	.word	0
	.align	2
	.type	 sysClkRunning,@object
	.size	 sysClkRunning,4
sysClkRunning:
	.word	0
	.align	2
	.type	 sysClkRoutine,@object
	.size	 sysClkRoutine,4
sysClkRoutine:
	.word	0
	.align	2
	.type	 sysAuxClkRunning,@object
	.size	 sysAuxClkRunning,4
sysAuxClkRunning:
	.word	0
	.align	2
	.type	 sysAuxClkConnected,@object
	.size	 sysAuxClkConnected,4
sysAuxClkConnected:
	.word	0
	.align	2
	.type	 sysAuxClkTicksPerSecond,@object
	.size	 sysAuxClkTicksPerSecond,4
sysAuxClkTicksPerSecond:
	.word	200
	.align	2
	.type	 sysAuxClkArg,@object
	.size	 sysAuxClkArg,4
sysAuxClkArg:
	.word	0
	.align	2
	.type	 sysAuxClkRoutine,@object
	.size	 sysAuxClkRoutine,4
sysAuxClkRoutine:
	.word	0
	.rdata
	.align	2
$LC0:
	.ascii	"Error: et device failed et_load routine.\n\000"
	.text
	.align	2
	.globl	sysEtEndLoad
	.file	3 "sysEtEnd.c"
	.ent	sysEtEndLoad
sysEtEndLoad:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, extra= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$16,16($sp)
	sw	$31,20($sp)
	.set	noreorder
	.set	nomacro
	jal	strlen
	move	$16,$4
	.set	macro
	.set	reorder

	bne	$2,$0,$L5
	.set	noreorder
	.set	nomacro
	jal	et_load
	move	$4,$16
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L6
	move	$16,$2
	.set	macro
	.set	reorder

$L5:
	la	$2,sysEnetAddrGet
	sw	$2,_func_etEnetAddrGet
	.set	noreorder
	.set	nomacro
	jal	et_load
	move	$4,$16
	.set	macro
	.set	reorder

	move	$16,$2
	li	$2,-1			# 0xffffffff
	.set	noreorder
	.set	nomacro
	bne	$16,$2,$L8
	move	$2,$16
	.set	macro
	.set	reorder

	la	$4,$LC0
	jal	printf
$L6:
	move	$2,$16
$L8:
	lw	$31,20($sp)
	lw	$16,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysEtEndLoad
	.align	2
	.globl	sysTimeZoneGood
	.file	4 "sysLib.c"
	.ent	sysTimeZoneGood
sysTimeZoneGood:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	move	$7,$0
	li	$10,13			# 0xd
	addu	$6,$4,64
	.set	noreorder
	.set	nomacro
	j	$L10
	move	$9,$6
	.set	macro
	.set	reorder

$L14:
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L17
	andi	$2,$5,0x80
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L16
	xori	$2,$5,0x3a
	.set	macro
	.set	reorder

$L17:
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$0
	.set	macro
	.set	reorder

$L16:
	sltu	$2,$2,1
	addu	$7,$7,$2
	addu	$4,$4,1
$L10:
	slt	$2,$4,$9
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L11
	slt	$8,$4,$6
	.set	macro
	.set	reorder

	lb	$5,0($4)
	#nop
	sltu	$3,$5,1
	xori	$2,$5,0xa
	sltu	$2,$2,1
	or	$3,$3,$2
	.set	noreorder
	.set	nomacro
	bnel	$3,$0,$L11
	sb	$0,0($4)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bnel	$5,$10,$L14
	slt	$2,$5,32
	.set	macro
	.set	reorder

	sb	$0,0($4)
$L11:
	xori	$2,$7,0x4
	sltu	$2,$2,1
	.set	noreorder
	.set	nomacro
	j	$31
	and	$2,$8,$2
	.set	macro
	.set	reorder

	.end	sysTimeZoneGood
	.align	2
	.globl	test_me
	.ent	test_me
test_me:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	j	$31
	.end	test_me
	.align	2
	.globl	test_me1
	.ent	test_me1
test_me1:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	j	$31
	.end	test_me1
	.rdata
	.align	2
$LC1:
	.ascii	"BCM5365 (MIPS32)\n\000"
	.align	2
$LC2:
	.ascii	"Board:  BCM95365R/P\n\000"
	.text
	.align	2
	.globl	sysModel
	.ent	sysModel
sysModel:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, extra= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$16,16($sp)
	la	$16,sysModelStr
	la	$5,$LC1
	sw	$31,20($sp)
	.set	noreorder
	.set	nomacro
	jal	strcpy
	move	$4,$16
	.set	macro
	.set	reorder

	la	$5,$LC2
	.set	noreorder
	.set	nomacro
	jal	strcat
	move	$4,$16
	.set	macro
	.set	reorder

	move	$2,$16
	lw	$31,20($sp)
	lw	$16,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysModel
	.rdata
	.align	2
$LC3:
	.ascii	"1.0/0\000"
	.text
	.align	2
	.globl	sysBspRev
	.ent	sysBspRev
sysBspRev:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	la	$2,$LC3
	j	$31
	.end	sysBspRev
	.rdata
	.align	2
$LC4:
	.ascii	"BFIX\000"
	.text
	.align	2
	.globl	sysBindFix
	.ent	sysBindFix
sysBindFix:
	.frame	$sp,104,$31		# vars= 72, regs= 4/0, args= 16, extra= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	lw	$2,bringupPrintRtn
	subu	$sp,$sp,104
	sw	$31,100($sp)
	sw	$18,96($sp)
	sw	$17,92($sp)
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L27
	sw	$16,88($sp)
	.set	macro
	.set	reorder

	la	$4,$LC4
	jal	$31,$2
$L27:
	addu	$4,$sp,16
	li	$5,1			# 0x1
	.set	noreorder
	.set	nomacro
	jal	sysNvRamGet
	li	$6,1032			# 0x408
	.set	macro
	.set	reorder

	addu	$4,$sp,16
	lbu	$2,16($sp)
	li	$5,1			# 0x1
	li	$6,1032			# 0x408
	addu	$2,$2,-16
	.set	noreorder
	.set	nomacro
	jal	sysNvRamSet
	sb	$2,16($sp)
	.set	macro
	.set	reorder

	lbu	$3,16($sp)
	li	$2,255			# 0xff
	addu	$3,$3,-1
	sb	$3,16($sp)
	andi	$3,$3,0x00ff
	.set	noreorder
	.set	nomacro
	beq	$3,$2,$L29
	li	$18,2			# 0x2
	.set	macro
	.set	reorder

	li	$17,255			# 0xff
	li	$4,2			# 0x2
$L33:
	li	$5,1			# 0x1
	move	$6,$0
	sw	$0,28($sp)
	sh	$0,26($sp)
	.set	noreorder
	.set	nomacro
	jal	socket
	sb	$18,25($sp)
	.set	macro
	.set	reorder

	move	$16,$2
	move	$4,$16
	addu	$5,$sp,24
	.set	noreorder
	.set	nomacro
	jal	bind
	li	$6,16			# 0x10
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	close
	move	$4,$16
	.set	macro
	.set	reorder

	lbu	$3,16($sp)
	#nop
	addu	$3,$3,-1
	sb	$3,16($sp)
	andi	$3,$3,0x00ff
	.set	noreorder
	.set	nomacro
	bnel	$3,$17,$L33
	li	$4,2			# 0x2
	.set	macro
	.set	reorder

$L29:
	lw	$31,100($sp)
	lw	$18,96($sp)
	lw	$17,92($sp)
	lw	$16,88($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,104
	.set	macro
	.set	reorder

	.end	sysBindFix
	.rdata
	.align	2
$LC5:
	.ascii	"HWOK\000"
	.text
	.align	2
	.globl	sysHwInit
	.ent	sysHwInit
sysHwInit:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$31,16($sp)
	jal	sysGetPrid
	li	$4,268435456			# 0x10000000
	.set	noreorder
	.set	nomacro
	jal	taskSRInit
	ori	$4,$4,0xfc01
	.set	macro
	.set	reorder

	li	$4,268435456			# 0x10000000
	.set	noreorder
	.set	nomacro
	jal	intSRSet
	ori	$4,$4,0xfc00
	.set	macro
	.set	reorder

	lw	$2,bringupPrintRtn
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L37
	lw	$31,16($sp)
	.set	macro
	.set	reorder

	la	$4,$LC5
	jal	$31,$2
	lw	$31,16($sp)
$L37:
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysHwInit
	.align	2
	.ent	sharedInt0
sharedInt0:
	.frame	$sp,40,$31		# vars= 8, regs= 4/0, args= 16, extra= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,40
	sw	$17,28($sp)
	li	$17,64			# 0x40
	sw	$18,32($sp)
	li	$18,79			# 0x4f
	sw	$31,36($sp)
	.set	noreorder
	.set	nomacro
	jal	intCRGet
	sw	$16,24($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	intSRGet
	move	$16,$2
	.set	macro
	.set	reorder

	and	$16,$16,$2
	.set	noreorder
	.set	nomacro
	jal	intSRSet
	andi	$4,$16,0xff00
	.set	macro
	.set	reorder

	andi	$2,$17,0x21
$L50:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L42
	sll	$4,$18,2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	intVecGet
	addu	$18,$18,-1
	.set	macro
	.set	reorder

	beq	$2,$0,$L43
	jal	$31,$2
$L43:
	#.set	volatile
	lw	$2,16($sp)
	#.set	novolatile
	#nop
	andi	$2,$2,0x400
	.set	noreorder
	.set	nomacro
	beql	$2,$0,$L49
	srl	$17,$17,1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	intVecGet
	li	$4,312			# 0x138
	.set	macro
	.set	reorder

	beq	$2,$0,$L45
	jal	$31,$2
$L45:
	.set	noreorder
	.set	nomacro
	jal	intVecGet
	li	$4,316			# 0x13c
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beql	$2,$0,$L49
	srl	$17,$17,1
	.set	macro
	.set	reorder

	jal	$31,$2
$L42:
	srl	$17,$17,1
$L49:
	.set	noreorder
	.set	nomacro
	bne	$17,$0,$L50
	andi	$2,$17,0x21
	.set	macro
	.set	reorder

	lw	$31,36($sp)
	lw	$18,32($sp)
	lw	$17,28($sp)
	lw	$16,24($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,40
	.set	macro
	.set	reorder

	.end	sharedInt0
	.align	2
	.ent	dummyISR
dummyISR:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	j	$31
	.end	dummyISR
	.align	2
	.globl	sysled
	.ent	sysled
sysled:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, extra= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,32
	sw	$17,20($sp)
	move	$17,$4
	li	$4,18192			# 0x4710
	move	$5,$0
	sw	$31,24($sp)
	.set	noreorder
	.set	nomacro
	jal	sb_kattach
	sw	$16,16($sp)
	.set	macro
	.set	reorder

	move	$16,$2
	move	$4,$16
	li	$5,255			# 0xff
	.set	noreorder
	.set	nomacro
	jal	sb_gpioout
	li	$6,255			# 0xff
	.set	macro
	.set	reorder

	move	$4,$16
	li	$5,255			# 0xff
	.set	noreorder
	.set	nomacro
	jal	sb_gpioouten
	li	$6,195			# 0xc3
	.set	macro
	.set	reorder

	move	$4,$16
	li	$5,255			# 0xff
	.set	noreorder
	.set	nomacro
	jal	sb_gpioout
	move	$6,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	sb_detach
	move	$4,$16
	.set	macro
	.set	reorder

	lw	$31,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	sysled
	.rdata
	.align	2
$LC6:
	.ascii	"PCI \000"
	.align	2
$LC7:
	.ascii	"SER \000"
	.align	2
$LC8:
	.ascii	"IRQs\n\000"
	.text
	.align	2
	.globl	sysHwInit2
	.ent	sysHwInit2
sysHwInit2:
	.frame	$sp,96,$31		# vars= 64, regs= 3/0, args= 16, extra= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,96
	sw	$31,88($sp)
	sw	$17,84($sp)
	.set	noreorder
	.set	nomacro
	jal	intSRGet
	sw	$16,80($sp)
	.set	macro
	.set	reorder

	lw	$3,bringupPrintRtn
	#nop
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L55
	move	$17,$2
	.set	macro
	.set	reorder

	la	$4,$LC6
	jal	$31,$3
$L55:
	move	$4,$0
	move	$5,$0
	move	$6,$0
	.set	noreorder
	.set	nomacro
	jal	pciConfigLibInit
	move	$7,$0
	.set	macro
	.set	reorder

	lw	$2,bringupPrintRtn
	#nop
	beq	$2,$0,$L57
	la	$4,$LC7
	jal	$31,$2
$L57:
	jal	sysSerialHwInit
	addu	$4,$sp,16
	li	$2,17			# 0x11
	li	$3,10			# 0xa
	sb	$2,16($sp)
	sb	$3,17($sp)
	.set	noreorder
	.set	nomacro
	jal	sysSerialPrintString
	sb	$0,18($sp)
	.set	macro
	.set	reorder

	jal	platform_init
	move	$4,$0
	li	$5,1			# 0x1
	li	$6,2			# 0x2
	li	$2,1			# 0x1
	sw	$2,pciMaxBus
	.set	noreorder
	.set	nomacro
	jal	sysPCIBridgeProbe
	li	$7,2			# 0x2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L58
	li	$5,1			# 0x1
	.set	macro
	.set	reorder

	li	$4,1			# 0x1
	li	$6,3			# 0x3
	.set	noreorder
	.set	nomacro
	jal	sysPCIBridgeProbe
	li	$7,3			# 0x3
	.set	macro
	.set	reorder

$L58:
	.set	noreorder
	.set	nomacro
	jal	pciIntLibInit
	li	$16,65			# 0x41
	.set	macro
	.set	reorder

	ori	$17,$17,0x8000
	sll	$4,$16,2
$L76:
	la	$5,dummyISR
	.set	noreorder
	.set	nomacro
	jal	intConnect
	move	$6,$16
	.set	macro
	.set	reorder

	addu	$6,$16,1
	la	$5,dummyISR
	.set	noreorder
	.set	nomacro
	jal	intConnect
	sll	$4,$6,2
	.set	macro
	.set	reorder

	addu	$6,$16,2
	la	$5,dummyISR
	.set	noreorder
	.set	nomacro
	jal	intConnect
	sll	$4,$6,2
	.set	macro
	.set	reorder

	addu	$6,$16,3
	la	$5,dummyISR
	.set	noreorder
	.set	nomacro
	jal	intConnect
	sll	$4,$6,2
	.set	macro
	.set	reorder

	addu	$6,$16,4
	la	$5,dummyISR
	.set	noreorder
	.set	nomacro
	jal	intConnect
	sll	$4,$6,2
	.set	macro
	.set	reorder

	addu	$16,$16,5
	slt	$2,$16,80
	.set	noreorder
	.set	nomacro
	bnel	$2,$0,$L76
	sll	$4,$16,2
	.set	macro
	.set	reorder

	li	$4,280			# 0x118
	la	$5,sysClkInt
	.set	noreorder
	.set	nomacro
	jal	intConnect
	move	$6,$0
	.set	macro
	.set	reorder

	li	$4,260			# 0x104
	la	$5,sharedInt0
	.set	noreorder
	.set	nomacro
	jal	intConnect
	move	$6,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	sysSerialHwInit2
	ori	$17,$17,0x400
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	intSRSet
	move	$4,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	sysled
	li	$4,191			# 0xbf
	.set	macro
	.set	reorder

	lw	$2,bringupPrintRtn
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L77
	lw	$31,88($sp)
	.set	macro
	.set	reorder

	la	$4,$LC8
	jal	$31,$2
	lw	$31,88($sp)
$L77:
	lw	$17,84($sp)
	lw	$16,80($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,96
	.set	macro
	.set	reorder

	.end	sysHwInit2
	.data
	.align	2
	.type	 memTop.45,@object
	.size	 memTop.45,4
memTop.45:
	.word	0
	.text
	.align	2
	.globl	sysPhysMemTop
	.ent	sysPhysMemTop
sysPhysMemTop:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lw	$2,memTop.45
	#nop
	bne	$2,$0,$L80
	li	$2,-2113929216			# 0x82000000
	sw	$2,memTop.45
	lw	$2,memTop.45
$L80:
	j	$31
	.end	sysPhysMemTop
	.data
	.align	2
	.type	 memTop.49,@object
	.size	 memTop.49,4
memTop.49:
	.word	0
	.text
	.align	2
	.globl	sysMemTop
	.ent	sysMemTop
sysMemTop:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	lw	$2,memTop.49
	subu	$sp,$sp,24
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L82
	sw	$31,16($sp)
	.set	macro
	.set	reorder

	jal	sysPhysMemTop
	sw	$2,memTop.49
$L82:
	lw	$2,memTop.49
	lw	$31,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysMemTop
	.rdata
	.align	2
$LC9:
	.ascii	"0x%x\000"
	.align	2
$LC10:
	.ascii	" + 0x%x\000"
	.align	2
$LC11:
	.ascii	" - 0x%x\000"
	.text
	.align	2
	.ent	sysbt_addr2sym
sysbt_addr2sym:
	.frame	$sp,48,$31		# vars= 8, regs= 3/0, args= 24, extra= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,48
	sw	$17,36($sp)
	move	$17,$5
	sw	$16,32($sp)
	move	$16,$4
	lw	$4,sysSymTbl
	move	$6,$16
	addu	$2,$sp,24
	addu	$7,$sp,28
	sw	$31,40($sp)
	.set	noreorder
	.set	nomacro
	jal	symFindByValue
	sw	$2,16($sp)
	.set	macro
	.set	reorder

	li	$3,-1			# 0xffffffff
	.set	noreorder
	.set	nomacro
	bne	$2,$3,$L84
	lw	$3,28($sp)
	.set	macro
	.set	reorder

	move	$4,$16
	la	$5,$LC9
	.set	noreorder
	.set	nomacro
	jal	sprintf
	move	$6,$17
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L89
	lw	$31,40($sp)
	.set	macro
	.set	reorder

$L84:
	sltu	$2,$3,$17
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L86
	sltu	$2,$17,$3
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	strlen
	move	$4,$16
	.set	macro
	.set	reorder

	lw	$6,28($sp)
	addu	$4,$16,$2
	la	$5,$LC10
	.set	noreorder
	.set	nomacro
	jal	sprintf
	subu	$6,$17,$6
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L89
	lw	$31,40($sp)
	.set	macro
	.set	reorder

$L86:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L89
	lw	$31,40($sp)
	.set	macro
	.set	reorder

	#nop
	.set	noreorder
	.set	nomacro
	jal	strlen
	move	$4,$16
	.set	macro
	.set	reorder

	lw	$6,28($sp)
	addu	$4,$16,$2
	la	$5,$LC11
	.set	noreorder
	.set	nomacro
	jal	sprintf
	subu	$6,$6,$17
	.set	macro
	.set	reorder

	lw	$31,40($sp)
$L89:
	lw	$17,36($sp)
	lw	$16,32($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,48
	.set	macro
	.set	reorder

	.end	sysbt_addr2sym
	.rdata
	.align	2
$LC12:
	.ascii	"FUNC = 0x%x, PC = 0x%x (%s), SP = 0x%x\n\000"
	.align	2
$LC13:
	.ascii	"RA 0x%x out of range\n\000"
	.align	2
$LC14:
	.ascii	"Caller SP 0x%x out of range\n\000"
	.text
	.align	2
	.globl	sysBacktraceMips
	.ent	sysBacktraceMips
sysBacktraceMips:
	.frame	$sp,2248,$31		# vars= 2184, regs= 10/0, args= 24, extra= 0
	.mask	0xc0ff0000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,2248
	sw	$16,2208($sp)
	move	$16,$4
	sw	$31,2244($sp)
	sw	$fp,2240($sp)
	sw	$23,2236($sp)
	sw	$22,2232($sp)
	sw	$21,2228($sp)
	sw	$20,2224($sp)
	sw	$19,2220($sp)
	sw	$18,2216($sp)
	sw	$17,2212($sp)
	sw	$5,2252($sp)
	.set	noreorder
	.set	nomacro
	jal	vxSpGet
	sw	$6,2256($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	vxPcGet
	move	$22,$2
	.set	macro
	.set	reorder

	move	$21,$2
	addu	$17,$sp,24
	move	$4,$17
	.set	noreorder
	.set	nomacro
	jal	strcpy
	move	$5,$16
	.set	macro
	.set	reorder

	lb	$3,24($sp)
	li	$fp,1			# 0x1
	addu	$2,$sp,1944
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L92
	sw	$2,2200($sp)
	.set	macro
	.set	reorder

	addu	$17,$17,1
$L144:
	lb	$2,0($17)
	#nop
	.set	noreorder
	.set	nomacro
	bnel	$2,$0,$L144
	addu	$17,$17,1
	.set	macro
	.set	reorder

$L92:
	lw	$3,2200($sp)
	#nop
	sltu	$2,$17,$3
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L96
	li	$23,536805376			# 0x1fff0000
	.set	macro
	.set	reorder

	ori	$23,$23,0xffff
$L97:
	move	$6,$0
	lw	$2,-4($21)
	li	$3,-32768			# 0xffff8000
	li	$4,-1346437120			# 0xafbf0000
	and	$2,$2,$3
	.set	noreorder
	.set	nomacro
	beq	$2,$4,$L99
	addu	$16,$21,-4
	.set	macro
	.set	reorder

	li	$4,-32768			# 0xffff8000
	li	$3,-1346437120			# 0xafbf0000
	addu	$6,$6,1
$L146:
	slt	$2,$6,4096
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L145
	li	$2,4096			# 0x1000
	.set	macro
	.set	reorder

	addu	$16,$16,-4
	lw	$2,0($16)
	#nop
	and	$2,$2,$4
	.set	noreorder
	.set	nomacro
	bnel	$2,$3,$L146
	addu	$6,$6,1
	.set	macro
	.set	reorder

$L99:
	li	$2,4096			# 0x1000
$L145:
	.set	noreorder
	.set	nomacro
	beq	$6,$2,$L96
	move	$6,$0
	.set	macro
	.set	reorder

	lw	$5,0($16)
	addu	$16,$16,-4
	li	$4,666697728			# 0x27bd0000
	ori	$4,$4,0x8000
	lw	$3,0($16)
	li	$2,-32768			# 0xffff8000
	and	$3,$3,$2
	.set	noreorder
	.set	nomacro
	beq	$3,$4,$L106
	andi	$5,$5,0x7fff
	.set	macro
	.set	reorder

	li	$4,-32768			# 0xffff8000
	li	$3,666697728			# 0x27bd0000
	ori	$3,$3,0x8000
	addu	$6,$6,1
$L148:
	slt	$2,$6,4096
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L147
	li	$2,4096			# 0x1000
	.set	macro
	.set	reorder

	addu	$16,$16,-4
	lw	$2,0($16)
	#nop
	and	$2,$2,$4
	.set	noreorder
	.set	nomacro
	bnel	$2,$3,$L148
	addu	$6,$6,1
	.set	macro
	.set	reorder

$L106:
	li	$2,4096			# 0x1000
$L147:
	.set	noreorder
	.set	nomacro
	beq	$6,$2,$L96
	move	$6,$5
	.set	macro
	.set	reorder

	lw	$3,0($16)
	li	$2,-65536			# 0xffff0000
	.set	noreorder
	.set	nomacro
	bgez	$6,$L112
	or	$3,$3,$2
	.set	macro
	.set	reorder

	addu	$6,$6,3
$L112:
	addu	$20,$sp,2072
	move	$4,$20
	move	$5,$21
	andi	$2,$6,0xfffc
	addu	$2,$2,$22
	lw	$19,0($2)
	addu	$3,$3,3
	sra	$3,$3,2
	sll	$3,$3,2
	.set	noreorder
	.set	nomacro
	jal	sysbt_addr2sym
	subu	$18,$22,$3
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bnel	$fp,$0,$L149
	lb	$2,0($17)
	.set	macro
	.set	reorder

	move	$4,$17
	la	$5,$LC12
	move	$6,$16
	move	$7,$21
	sw	$20,16($sp)
	.set	noreorder
	.set	nomacro
	jal	sprintf
	sw	$22,20($sp)
	.set	macro
	.set	reorder

	lb	$2,0($17)
$L149:
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L116
	nor	$3,$0,$19
	.set	macro
	.set	reorder

	addu	$17,$17,1
$L150:
	lb	$2,0($17)
	#nop
	.set	noreorder
	.set	nomacro
	bnel	$2,$0,$L150
	addu	$17,$17,1
	.set	macro
	.set	reorder

$L116:
	li	$2,-1610678272			# 0x9fff0000
	ori	$2,$2,0xffff
	srl	$3,$3,31
	sltu	$2,$2,$19
	or	$3,$3,$2
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L151
	li	$16,536805376			# 0x1fff0000
	.set	macro
	.set	reorder

	jal	sysPhysMemTop
	and	$3,$19,$23
	and	$2,$2,$23
	sltu	$3,$3,$2
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L152
	li	$3,-1610678272			# 0x9fff0000
	.set	macro
	.set	reorder

	li	$16,536805376			# 0x1fff0000
$L151:
	ori	$16,$16,0xffff
	la $2,1610612736($19)
	sltu	$2,$16,$2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L153
	move	$4,$17
	.set	macro
	.set	reorder

	jal	sysPhysMemTop
	and	$3,$19,$16
	and	$2,$2,$16
	sltu	$3,$3,$2
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L119
	li	$3,-1610678272			# 0x9fff0000
	.set	macro
	.set	reorder

	move	$4,$17
$L153:
	la	$5,$LC13
	.set	noreorder
	.set	nomacro
	jal	sprintf
	move	$6,$19
	.set	macro
	.set	reorder

	lb	$3,0($17)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L142
	lw	$5,2252($sp)
	.set	macro
	.set	reorder

	addu	$17,$17,1
$L154:
	lb	$2,0($17)
	#nop
	.set	noreorder
	.set	nomacro
	bnel	$2,$0,$L154
	addu	$17,$17,1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L142
	lw	$5,2252($sp)
	.set	macro
	.set	reorder

$L119:
$L152:
	ori	$3,$3,0xffff
	nor	$2,$0,$18
	srl	$2,$2,31
	sltu	$3,$3,$18
	or	$2,$2,$3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L155
	li	$16,536805376			# 0x1fff0000
	.set	macro
	.set	reorder

	jal	sysPhysMemTop
	and	$3,$18,$23
	and	$2,$2,$23
	sltu	$3,$3,$2
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L156
	move	$21,$19
	.set	macro
	.set	reorder

	li	$16,536805376			# 0x1fff0000
$L155:
	ori	$16,$16,0xffff
	la $2,1610612736($18)
	sltu	$2,$16,$2
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L157
	move	$4,$17
	.set	macro
	.set	reorder

	jal	sysPhysMemTop
	and	$3,$18,$16
	and	$2,$2,$16
	sltu	$3,$3,$2
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L126
	move	$21,$19
	.set	macro
	.set	reorder

	move	$4,$17
$L157:
	la	$5,$LC14
	.set	noreorder
	.set	nomacro
	jal	sprintf
	move	$6,$18
	.set	macro
	.set	reorder

	lb	$3,0($17)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L142
	lw	$5,2252($sp)
	.set	macro
	.set	reorder

	addu	$17,$17,1
$L158:
	lb	$2,0($17)
	#nop
	.set	noreorder
	.set	nomacro
	bnel	$2,$0,$L158
	addu	$17,$17,1
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L142
	lw	$5,2252($sp)
	.set	macro
	.set	reorder

$L126:
$L156:
	lw	$3,2200($sp)
	move	$22,$18
	sltu	$2,$17,$3
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L97
	move	$fp,$0
	.set	macro
	.set	reorder

$L96:
	lw	$5,2252($sp)
$L142:
	.set	noreorder
	.set	nomacro
	jal	strcpy
	move	$4,$17
	.set	macro
	.set	reorder

	lw	$2,2256($sp)
	#nop
	beq	$2,$0,$L134
	.set	noreorder
	.set	nomacro
	jal	sysSerialPrintString
	addu	$4,$sp,24
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L143
	lw	$31,2244($sp)
	.set	macro
	.set	reorder

$L134:
	#nop
	jal	__stdout
	lw	$5,0($2)
	.set	noreorder
	.set	nomacro
	jal	fputs
	addu	$4,$sp,24
	.set	macro
	.set	reorder

	jal	__stdout
	.set	noreorder
	.set	nomacro
	jal	fflush
	lw	$4,0($2)
	.set	macro
	.set	reorder

	lw	$31,2244($sp)
$L143:
	lw	$fp,2240($sp)
	lw	$23,2236($sp)
	lw	$22,2232($sp)
	lw	$21,2228($sp)
	lw	$20,2224($sp)
	lw	$19,2220($sp)
	lw	$18,2216($sp)
	lw	$17,2212($sp)
	lw	$16,2208($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,2248
	.set	macro
	.set	reorder

	.end	sysBacktraceMips
	.globl	sysToMonitorExcMessage
	.data
	.align	2
	.type	 sysToMonitorExcMessage,@object
	.size	 sysToMonitorExcMessage,4
sysToMonitorExcMessage:
	.word	0
	.globl	sysToMonitorBacktrace
	.align	2
	.type	 sysToMonitorBacktrace,@object
	.size	 sysToMonitorBacktrace,4
sysToMonitorBacktrace:
	.word	1
	.globl	sysToMonitorReboot
	.align	2
	.type	 sysToMonitorReboot,@object
	.size	 sysToMonitorReboot,4
sysToMonitorReboot:
	.word	1
	.rdata
	.align	2
$LC15:
	.ascii	"\n"
	.ascii	"--- Stack Trace ---\n\000"
	.align	2
$LC16:
	.ascii	"\000"
	.text
	.align	2
	.globl	sysToMonitor
	.ent	sysToMonitor
sysToMonitor:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, extra= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	lw	$2,sysToMonitorHook
	subu	$sp,$sp,24
	sw	$16,16($sp)
	move	$16,$4
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L160
	sw	$31,20($sp)
	.set	macro
	.set	reorder

	jal	$31,$2
$L160:
	lw	$2,sysToMonitorBacktrace
	#nop
	beq	$2,$0,$L161
	la	$4,$LC15
	la	$5,$LC16
	.set	noreorder
	.set	nomacro
	jal	sysBacktraceMips
	li	$6,1			# 0x1
	.set	macro
	.set	reorder

$L161:
	lw	$2,sysToMonitorReboot
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L163
	lw	$31,20($sp)
	.set	macro
	.set	reorder

	#nop
	jal	intLock
	.set	noat
	li	$1,-1077936120
	.set	noreorder
	.set	nomacro
	jal	$31,$1
	.set	at
	move	$4,$16
	.set	macro
	.set	reorder

	lw	$31,20($sp)
$L163:
	lw	$16,16($sp)
	move	$2,$0
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysToMonitor
	.align	2
	.globl	sysProcNumGet
	.ent	sysProcNumGet
sysProcNumGet:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lw	$2,sysProcNum
	j	$31
	.end	sysProcNumGet
	.align	2
	.globl	sysProcNumSet
	.ent	sysProcNumSet
sysProcNumSet:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	sw	$4,sysProcNum
	j	$31
	.end	sysProcNumSet
	.align	2
	.globl	sysLedDsply
	.ent	sysLedDsply
sysLedDsply:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	j	$31
	.end	sysLedDsply
	.align	2
	.globl	sysBusEid
	.ent	sysBusEid
sysBusEid:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$0
	.set	macro
	.set	reorder

	.end	sysBusEid
	.align	2
	.globl	sysBusEar
	.ent	sysBusEar
sysBusEar:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	j	$31
	li	$2,-1			# 0xffffffff
	.set	macro
	.set	reorder

	.end	sysBusEar
	.align	2
	.globl	sysAutoAck
	.ent	sysAutoAck
sysAutoAck:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	li	$2,33			# 0x21
	.set	noreorder
	.set	nomacro
	beq	$4,$2,$L173
	sw	$31,16($sp)
	.set	macro
	.set	reorder

	slt	$2,$4,34
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L176
	li	$2,32			# 0x20
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$4,$2,$L172
	li	$2,-1			# 0xffffffff
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L178
	lw	$31,16($sp)
	.set	macro
	.set	reorder

$L176:
	li	$2,70			# 0x46
	.set	noreorder
	.set	nomacro
	bne	$4,$2,$L177
	li	$2,-1			# 0xffffffff
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	sysSetCompare
	move	$4,$0
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L170
	move	$2,$0
	.set	macro
	.set	reorder

$L172:
	jal	sysSw0Ack
	.set	noreorder
	.set	nomacro
	j	$L178
	lw	$31,16($sp)
	.set	macro
	.set	reorder

$L173:
	#nop
	jal	sysSw1Ack
	.set	noreorder
	.set	nomacro
	j	$L178
	lw	$31,16($sp)
	.set	macro
	.set	reorder

$L170:
$L177:
	#nop
	lw	$31,16($sp)
$L178:
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysAutoAck
	.align	2
	.globl	sysSw0Gen
	.ent	sysSw0Gen
sysSw0Gen:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$31,16($sp)
	jal	intCRGet
	.set	noreorder
	.set	nomacro
	jal	intCRSet
	ori	$4,$2,0x100
	.set	macro
	.set	reorder

	lw	$31,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysSw0Gen
	.align	2
	.globl	sysSw1Gen
	.ent	sysSw1Gen
sysSw1Gen:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$31,16($sp)
	jal	intCRGet
	.set	noreorder
	.set	nomacro
	jal	intCRSet
	ori	$4,$2,0x200
	.set	macro
	.set	reorder

	lw	$31,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysSw1Gen
	.align	2
	.ent	sysSw0Ack
sysSw0Ack:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$31,16($sp)
	jal	intCRGet
	li	$4,-257			# 0xfffffeff
	.set	noreorder
	.set	nomacro
	jal	intCRSet
	and	$4,$2,$4
	.set	macro
	.set	reorder

	lw	$31,16($sp)
	move	$2,$0
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysSw0Ack
	.align	2
	.ent	sysSw1Ack
sysSw1Ack:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$31,16($sp)
	jal	intCRGet
	li	$4,-513			# 0xfffffdff
	.set	noreorder
	.set	nomacro
	jal	intCRSet
	and	$4,$2,$4
	.set	macro
	.set	reorder

	lw	$31,16($sp)
	move	$2,$0
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysSw1Ack
	.align	2
	.globl	sysClkEnable
	.ent	sysClkEnable
sysClkEnable:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, extra= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$31,20($sp)
	.set	noreorder
	.set	nomacro
	jal	get_sb_clock
	sw	$16,16($sp)
	.set	macro
	.set	reorder

	lw	$3,sysClkTicksPerSecond
	#nop
	divu	$0,$2,$3
	mflo	$2
	#nop
	.set	noreorder
	beql	$3,$0,1f
	break	7
1:
	.set	reorder
	srl	$2,$2,1
	sw	$2,sysClkProcessorTicks
	jal	intLock
	move	$4,$0
	.set	noreorder
	.set	nomacro
	jal	sysSetCompare
	move	$16,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	sysSetCount
	li	$4,1			# 0x1
	.set	macro
	.set	reorder

	lw	$4,sysClkProcessorTicks
	jal	sysSetCompare
	lw	$3,sysClkProcessorTicks
	li	$2,1			# 0x1
	sw	$2,sysClkRunning
	sw	$3,sysSoftCompare
	.set	noreorder
	.set	nomacro
	jal	intUnlock
	move	$4,$16
	.set	macro
	.set	reorder

	lw	$31,20($sp)
	lw	$16,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysClkEnable
	.align	2
	.globl	sysClkRateGet
	.ent	sysClkRateGet
sysClkRateGet:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lw	$2,sysClkTicksPerSecond
	j	$31
	.end	sysClkRateGet
	.align	2
	.globl	sysClkRateSet
	.ent	sysClkRateSet
sysClkRateSet:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	addu	$2,$4,-1
	sltu	$2,$2,3600
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L186
	sw	$31,16($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L188
	li	$2,-1			# 0xffffffff
	.set	macro
	.set	reorder

$L186:
	lw	$2,sysClkRunning
	sw	$4,sysClkTicksPerSecond
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L188
	move	$2,$0
	.set	macro
	.set	reorder

	jal	sysClkDisable
	jal	sysClkEnable
	move	$2,$0
$L188:
	lw	$31,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysClkRateSet
	.globl	timer_irqs
	.data
	.align	2
	.type	 timer_irqs,@object
	.size	 timer_irqs,4
timer_irqs:
	.word	0
	.globl	timer_irq1
	.align	2
	.type	 timer_irq1,@object
	.size	 timer_irq1,4
timer_irq1:
	.word	0
	.text
	.align	2
	.ent	sysClkInt
sysClkInt:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, extra= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	lw	$2,sysClkRunning
	subu	$sp,$sp,24
	sw	$31,20($sp)
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L190
	sw	$16,16($sp)
	.set	macro
	.set	reorder

	lw	$4,sysSoftCompare
	jal	sysSetCompare
	.set	noreorder
	.set	nomacro
	j	$L198
	lw	$31,20($sp)
	.set	macro
	.set	reorder

$L190:
	#nop
	jal	intLock
	lw	$3,timer_irq1
	#nop
	addu	$3,$3,1
	sw	$3,timer_irq1
	slt	$3,$3,100
	.set	noreorder
	.set	nomacro
	bne	$3,$0,$L191
	move	$16,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	sysled
	li	$4,191			# 0xbf
	.set	macro
	.set	reorder

	sw	$0,timer_irq1
$L191:
	jal	sysGetCount
	lw	$3,sysSoftCompare
	lw	$4,sysClkProcessorTicks
	subu	$5,$2,$3
	addu	$2,$4,-200
	sltu	$2,$5,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L192
	sll	$2,$4,4
	.set	macro
	.set	reorder

	addu	$4,$3,$4
	sw	$4,sysSoftCompare
	jal	sysSetCompare
	j	$L193
$L192:
	sltu	$2,$5,$2
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L194
	addu	$2,$3,$4
	.set	macro
	.set	reorder

	sw	$2,sysSoftCompare
	j	$L193
$L194:
	jal	sysGetCount
	lw	$3,sysClkProcessorTicks
	#nop
	addu	$2,$2,$3
	sw	$2,sysSoftCompare
	.set	noreorder
	.set	nomacro
	jal	sysSetCompare
	move	$4,$2
	.set	macro
	.set	reorder

$L193:
	lw	$2,timer_irqs
	#nop
	addu	$2,$2,1
	sw	$2,timer_irqs
	slt	$2,$2,101
	bne	$2,$0,$L196
	.set	noreorder
	.set	nomacro
	jal	sysled
	li	$4,253			# 0xfd
	.set	macro
	.set	reorder

	sw	$0,timer_irqs
$L196:
	.set	noreorder
	.set	nomacro
	jal	intUnlock
	move	$4,$16
	.set	macro
	.set	reorder

	lw	$2,sysClkRoutine
	#nop
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L198
	lw	$31,20($sp)
	.set	macro
	.set	reorder

	lw	$4,sysClkArg
	jal	$31,$2
	lw	$31,20($sp)
$L198:
	lw	$16,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysClkInt
	.align	2
	.globl	sysClkConnect
	.ent	sysClkConnect
sysClkConnect:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, extra= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	lw	$2,sysClkConnected
	subu	$sp,$sp,32
	sw	$16,16($sp)
	move	$16,$4
	sw	$17,20($sp)
	move	$17,$5
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L200
	sw	$31,24($sp)
	.set	macro
	.set	reorder

	jal	sysHwInit2
	li	$2,1			# 0x1
	sw	$2,sysClkConnected
$L200:
	lw	$31,24($sp)
	sw	$17,sysClkArg
	lw	$17,20($sp)
	sw	$16,sysClkRoutine
	lw	$16,16($sp)
	move	$2,$0
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	sysClkConnect
	.align	2
	.globl	sysClkDisable
	.ent	sysClkDisable
sysClkDisable:
	.frame	$sp,24,$31		# vars= 0, regs= 2/0, args= 16, extra= 0
	.mask	0x80010000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$31,20($sp)
	.set	noreorder
	.set	nomacro
	jal	intLock
	sw	$16,16($sp)
	.set	macro
	.set	reorder

	move	$4,$0
	.set	noreorder
	.set	nomacro
	jal	sysSetCompare
	move	$16,$2
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	sysSetCount
	li	$4,1			# 0x1
	.set	macro
	.set	reorder

	li	$2,-256			# 0xffffff00
	sw	$2,sysClkProcessorTicks
	.set	noreorder
	.set	nomacro
	jal	sysSetCompare
	li	$4,-256			# 0xffffff00
	.set	macro
	.set	reorder

	lw	$3,sysClkProcessorTicks
	sw	$0,sysClkRunning
	sw	$3,sysSoftCompare
	.set	noreorder
	.set	nomacro
	jal	intUnlock
	move	$4,$16
	.set	macro
	.set	reorder

	lw	$31,20($sp)
	lw	$16,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysClkDisable
	.align	2
	.ent	sysAuxClkInt
sysAuxClkInt:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	lw	$4,timerregs
	subu	$sp,$sp,24
	sw	$31,16($sp)
	#.set	volatile
	lw	$2,32($4)
	#.set	novolatile
	#nop
	andi	$2,$2,0x80
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L205
	lw	$31,16($sp)
	.set	macro
	.set	reorder

	lw	$3,sysAuxClkRoutine
	#.set	volatile
	sw	$2,32($4)
	#.set	novolatile
	.set	noreorder
	.set	nomacro
	beq	$3,$0,$L205
	lw	$31,16($sp)
	.set	macro
	.set	reorder

	lw	$4,sysAuxClkArg
	jal	$31,$3
	lw	$31,16($sp)
$L205:
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysAuxClkInt
	.align	2
	.globl	sysAuxClkConnect
	.ent	sysAuxClkConnect
sysAuxClkConnect:
	.frame	$sp,32,$31		# vars= 0, regs= 3/0, args= 16, extra= 0
	.mask	0x80030000,-8
	.fmask	0x00000000,0
	lw	$2,sysAuxClkConnected
	subu	$sp,$sp,32
	sw	$16,16($sp)
	move	$16,$4
	sw	$17,20($sp)
	move	$17,$5
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L207
	sw	$31,24($sp)
	.set	macro
	.set	reorder

	li	$2,1			# 0x1
	sw	$2,sysAuxClkConnected
	li	$4,272			# 0x110
	la	$5,sysAuxClkInt
	.set	noreorder
	.set	nomacro
	jal	intConnect
	move	$6,$0
	.set	macro
	.set	reorder

$L207:
	lw	$31,24($sp)
	sw	$17,sysAuxClkArg
	lw	$17,20($sp)
	sw	$16,sysAuxClkRoutine
	lw	$16,16($sp)
	move	$2,$0
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	sysAuxClkConnect
	.align	2
	.globl	sysAuxClkDisable
	.ent	sysAuxClkDisable
sysAuxClkDisable:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lw	$2,timerregs
	#nop
	#.set	volatile
	sw	$0,40($2)
	#.set	novolatile
	#.set	volatile
	sw	$0,36($2)
	#.set	novolatile
	sw	$0,sysAuxClkRunning
	j	$31
	.end	sysAuxClkDisable
	.rdata
	.align	2
$LC17:
	.ascii	"No core was selected to provide a timer\n\000"
	.align	2
$LC18:
	.ascii	"Use set_auxtimer(base_addr) to select a core.\n\000"
	.text
	.align	2
	.globl	sysAuxClkEnable
	.ent	sysAuxClkEnable
sysAuxClkEnable:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	lw	$2,timerregs
	subu	$sp,$sp,24
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L210
	sw	$31,16($sp)
	.set	macro
	.set	reorder

	la	$4,$LC17
	jal	printf
	la	$4,$LC18
	jal	printf
	sw	$0,sysAuxClkRunning
	.set	noreorder
	.set	nomacro
	j	$L213
	lw	$31,16($sp)
	.set	macro
	.set	reorder

$L210:
	addu	$2,$2,3840
	#.set	volatile
	lw	$3,252($2)
	#.set	novolatile
	li	$2,2049			# 0x801
	andi	$3,$3,0xfff0
	srl	$3,$3,4
	.set	noreorder
	.set	nomacro
	bne	$3,$2,$L211
	li	$3,63963136			# 0x3d00000
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L212
	ori	$3,$3,0x9000
	.set	macro
	.set	reorder

$L211:
	jal	get_sb_clock
	move	$3,$2
$L212:
	lw	$2,sysAuxClkTicksPerSecond
	#nop
	divu	$0,$3,$2
	lw	$4,timerregs
	mflo	$3
	#nop
	.set	noreorder
	beql	$2,$0,1f
	break	7
1:
	.set	reorder
	li	$2,128			# 0x80
	#.set	volatile
	sw	$3,40($4)
	#.set	novolatile
	li	$3,1			# 0x1
	#.set	volatile
	sw	$2,36($4)
	#.set	novolatile
	sw	$3,sysAuxClkRunning
	lw	$31,16($sp)
$L213:
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysAuxClkEnable
	.align	2
	.globl	set_auxtimer
	.ent	set_auxtimer
set_auxtimer:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	sw	$4,timerregs
	j	$31
	.end	set_auxtimer
	.rdata
	.align	2
$LC19:
	.ascii	"Aux timer core base address is 0x%x\n\000"
	.text
	.align	2
	.globl	show_auxtimer
	.ent	show_auxtimer
show_auxtimer:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	lw	$5,timerregs
	subu	$sp,$sp,24
	la	$4,$LC19
	sw	$31,16($sp)
	jal	printf
	lw	$31,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	show_auxtimer
	.align	2
	.globl	sysAuxClkRateGet
	.ent	sysAuxClkRateGet
sysAuxClkRateGet:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lw	$2,sysAuxClkTicksPerSecond
	j	$31
	.end	sysAuxClkRateGet
	.align	2
	.globl	sysAuxClkRateSet
	.ent	sysAuxClkRateSet
sysAuxClkRateSet:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	addu	$2,$4,-1
	sltu	$2,$2,10000
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L218
	sw	$31,16($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L220
	li	$2,-1			# 0xffffffff
	.set	macro
	.set	reorder

$L218:
	lw	$2,sysAuxClkRunning
	sw	$4,sysAuxClkTicksPerSecond
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L220
	move	$2,$0
	.set	macro
	.set	reorder

	jal	sysAuxClkDisable
	jal	sysAuxClkEnable
	move	$2,$0
$L220:
	lw	$31,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysAuxClkRateSet
	.align	2
	.globl	sysReboot
	.ent	sysReboot
sysReboot:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	j	$31
	.end	sysReboot
	.align	2
	.globl	sysMaskVmeErr
	.ent	sysMaskVmeErr
sysMaskVmeErr:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$0
	.set	macro
	.set	reorder

	.end	sysMaskVmeErr
	.align	2
	.globl	sysUnmaskVmeErr
	.ent	sysUnmaskVmeErr
sysUnmaskVmeErr:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$0
	.set	macro
	.set	reorder

	.end	sysUnmaskVmeErr
	.globl	defaultAddr
	.data
	.align	2
	.type	 defaultAddr,@object
defaultAddr:
	.byte	0
	.byte	16
	.byte	24
	.byte	83
	.byte	101
	.byte	0
	.size	 defaultAddr,6
	.text
	.align	2
	.globl	sysEnetAddrGet
	.ent	sysEnetAddrGet
sysEnetAddrGet:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	la	$4,defaultAddr
	lbu	$2,0($4)
	#nop
	sb	$2,0($6)
	lbu	$3,1($4)
	#nop
	sb	$3,1($6)
	lbu	$2,2($4)
	#nop
	sb	$2,2($6)
	lbu	$3,3($4)
	#nop
	sb	$3,3($6)
	lbu	$2,4($4)
	#nop
	sb	$2,4($6)
	lbu	$3,5($4)
	#nop
	sb	$3,5($6)
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$0
	.set	macro
	.set	reorder

	.end	sysEnetAddrGet
	.align	2
	.globl	sysPCIBridgeProbe
	.ent	sysPCIBridgeProbe
sysPCIBridgeProbe:
	.frame	$sp,80,$31		# vars= 16, regs= 9/0, args= 24, extra= 0
	.mask	0x80ff0000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,80
	sw	$20,56($sp)
	move	$20,$4
	li	$4,4113			# 0x1011
	sw	$23,68($sp)
	move	$23,$5
	li	$5,34			# 0x22
	sw	$21,60($sp)
	move	$21,$6
	move	$6,$20
	sw	$22,64($sp)
	move	$22,$7
	sw	$19,52($sp)
	addu	$19,$sp,24
	move	$7,$19
	sw	$18,48($sp)
	addu	$18,$sp,28
	sw	$17,44($sp)
	addu	$17,$sp,32
	sw	$31,72($sp)
	sw	$16,40($sp)
	sw	$18,16($sp)
	.set	noreorder
	.set	nomacro
	jal	pciFindDevice
	sw	$17,20($sp)
	.set	macro
	.set	reorder

	move	$16,$2
	li	$2,-1			# 0xffffffff
	.set	noreorder
	.set	nomacro
	bne	$16,$2,$L246
	lw	$4,24($sp)
	.set	macro
	.set	reorder

	#nop
	li	$4,13192			# 0x3388
	li	$5,34			# 0x22
	move	$6,$20
	move	$7,$19
	sw	$18,16($sp)
	.set	noreorder
	.set	nomacro
	jal	pciFindDevice
	sw	$17,20($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	beq	$2,$16,$L243
	lw	$4,24($sp)
	.set	macro
	.set	reorder

$L246:
	lw	$5,28($sp)
	lw	$6,32($sp)
	lw	$2,pciMaxBus
	sw	$0,16($sp)
	addu	$2,$2,1
	sw	$2,pciMaxBus
	.set	noreorder
	.set	nomacro
	jal	pciConfigOutWord
	li	$7,4			# 0x4
	.set	macro
	.set	reorder

	lw	$4,24($sp)
	lw	$5,28($sp)
	lw	$6,32($sp)
	li	$7,6			# 0x6
	li	$16,65535			# 0xffff
	.set	noreorder
	.set	nomacro
	jal	pciConfigOutWord
	sw	$16,16($sp)
	.set	macro
	.set	reorder

	lw	$4,24($sp)
	lw	$5,28($sp)
	lw	$6,32($sp)
	li	$7,62			# 0x3e
	li	$2,64			# 0x40
	.set	noreorder
	.set	nomacro
	jal	pciConfigOutWord
	sw	$2,16($sp)
	.set	macro
	.set	reorder

	li	$7,24			# 0x18
	sll	$2,$21,8
	or	$2,$23,$2
	lw	$4,24($sp)
	lw	$5,28($sp)
	lw	$6,32($sp)
	sll	$3,$22,16
	or	$2,$2,$3
	.set	noreorder
	.set	nomacro
	jal	pciConfigOutLong
	sw	$2,16($sp)
	.set	macro
	.set	reorder

	lw	$4,24($sp)
	lw	$5,28($sp)
	lw	$6,32($sp)
	li	$7,30			# 0x1e
	.set	noreorder
	.set	nomacro
	jal	pciConfigOutWord
	sw	$16,16($sp)
	.set	macro
	.set	reorder

	lw	$4,24($sp)
	lw	$5,28($sp)
	lw	$6,32($sp)
	li	$7,32			# 0x20
	li	$2,43008			# 0xa800
	.set	noreorder
	.set	nomacro
	jal	pciConfigOutWord
	sw	$2,16($sp)
	.set	macro
	.set	reorder

	lw	$4,24($sp)
	lw	$5,28($sp)
	lw	$6,32($sp)
	li	$7,34			# 0x22
	li	$2,43504			# 0xa9f0
	.set	noreorder
	.set	nomacro
	jal	pciConfigOutWord
	sw	$2,16($sp)
	.set	macro
	.set	reorder

	lw	$4,24($sp)
	lw	$5,28($sp)
	lw	$6,32($sp)
	li	$7,62			# 0x3e
	.set	noreorder
	.set	nomacro
	jal	pciConfigOutWord
	sw	$0,16($sp)
	.set	macro
	.set	reorder

	lw	$4,24($sp)
	lw	$5,28($sp)
	lw	$6,32($sp)
	li	$7,104			# 0x68
	.set	noreorder
	.set	nomacro
	jal	pciConfigOutWord
	sw	$0,16($sp)
	.set	macro
	.set	reorder

	lw	$4,24($sp)
	lw	$5,28($sp)
	lw	$6,32($sp)
	li	$7,106			# 0x6a
	li	$2,255			# 0xff
	.set	noreorder
	.set	nomacro
	jal	pciConfigOutByte
	sw	$2,16($sp)
	.set	macro
	.set	reorder

	lw	$4,24($sp)
	lw	$5,28($sp)
	lw	$6,32($sp)
	li	$7,6			# 0x6
	.set	noreorder
	.set	nomacro
	jal	pciConfigOutWord
	sw	$16,16($sp)
	.set	macro
	.set	reorder

	lw	$4,24($sp)
	lw	$5,28($sp)
	lw	$6,32($sp)
	li	$7,4			# 0x4
	li	$2,6			# 0x6
	.set	noreorder
	.set	nomacro
	jal	pciConfigOutWord
	sw	$2,16($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L245
	move	$2,$0
	.set	macro
	.set	reorder

$L243:
	li	$2,-1			# 0xffffffff
$L245:
	lw	$31,72($sp)
	lw	$23,68($sp)
	lw	$22,64($sp)
	lw	$21,60($sp)
	lw	$20,56($sp)
	lw	$19,52($sp)
	lw	$18,48($sp)
	lw	$17,44($sp)
	lw	$16,40($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,80
	.set	macro
	.set	reorder

	.end	sysPCIBridgeProbe
	.align	2
	.globl	sysBusTas
	.ent	sysBusTas
sysBusTas:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	j	$31
	li	$2,1			# 0x1
	.set	macro
	.set	reorder

	.end	sysBusTas
	.align	2
	.globl	sysBoardRev
	.ent	sysBoardRev
sysBoardRev:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	j	$31
	move	$2,$0
	.set	macro
	.set	reorder

	.end	sysBoardRev
	.align	2
	.globl	sysTodGetSecond
	.ent	sysTodGetSecond
sysTodGetSecond:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	lw	$2,sysTodFuncs+12
	subu	$sp,$sp,24
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L250
	sw	$31,16($sp)
	.set	macro
	.set	reorder

	jal	$31,$2
	.set	noreorder
	.set	nomacro
	j	$L253
	lw	$31,16($sp)
	.set	macro
	.set	reorder

$L250:
	li	$2,-1			# 0xffffffff
	lw	$31,16($sp)
$L253:
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysTodGetSecond
	.align	2
	.globl	sysTodGet
	.ent	sysTodGet
sysTodGet:
	.frame	$sp,32,$31		# vars= 0, regs= 1/0, args= 24, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	lw	$8,sysTodFuncs+4
	subu	$sp,$sp,32
	.set	noreorder
	.set	nomacro
	beq	$8,$0,$L255
	sw	$31,24($sp)
	.set	macro
	.set	reorder

	lw	$2,48($sp)
	lw	$3,52($sp)
	sw	$2,16($sp)
	.set	noreorder
	.set	nomacro
	jal	$31,$8
	sw	$3,20($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L258
	lw	$31,24($sp)
	.set	macro
	.set	reorder

$L255:
	li	$2,-1			# 0xffffffff
	lw	$31,24($sp)
$L258:
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	sysTodGet
	.align	2
	.globl	sysTodSet
	.ent	sysTodSet
sysTodSet:
	.frame	$sp,32,$31		# vars= 0, regs= 1/0, args= 24, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	lw	$8,sysTodFuncs+8
	subu	$sp,$sp,32
	.set	noreorder
	.set	nomacro
	beq	$8,$0,$L260
	sw	$31,24($sp)
	.set	macro
	.set	reorder

	lw	$2,48($sp)
	lw	$3,52($sp)
	sw	$2,16($sp)
	.set	noreorder
	.set	nomacro
	jal	$31,$8
	sw	$3,20($sp)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	j	$L263
	lw	$31,24($sp)
	.set	macro
	.set	reorder

$L260:
	li	$2,-1			# 0xffffffff
	lw	$31,24($sp)
$L263:
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	sysTodSet
	.align	2
	.globl	sysTodInit
	.ent	sysTodInit
sysTodInit:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	lw	$2,sysTodFuncs
	subu	$sp,$sp,24
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L265
	sw	$31,16($sp)
	.set	macro
	.set	reorder

	jal	$31,$2
	.set	noreorder
	.set	nomacro
	j	$L268
	lw	$31,16($sp)
	.set	macro
	.set	reorder

$L265:
	li	$2,-1			# 0xffffffff
	lw	$31,16($sp)
$L268:
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysTodInit
	.align	2
	.globl	sysTodWatchdogArm
	.ent	sysTodWatchdogArm
sysTodWatchdogArm:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	lw	$2,sysTodFuncs+16
	subu	$sp,$sp,24
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L270
	sw	$31,16($sp)
	.set	macro
	.set	reorder

	jal	$31,$2
	.set	noreorder
	.set	nomacro
	j	$L273
	lw	$31,16($sp)
	.set	macro
	.set	reorder

$L270:
	li	$2,-1			# 0xffffffff
	lw	$31,16($sp)
$L273:
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysTodWatchdogArm
	.rdata
	.align	2
$LC20:
	.ascii	"\n\r\000"
	.text
	.align	2
	.globl	sysSerialPrintStringNL
	.ent	sysSerialPrintStringNL
sysSerialPrintStringNL:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$31,16($sp)
	jal	sysSerialPrintString
	la	$4,$LC20
	jal	sysSerialPrintString
	lw	$31,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	sysSerialPrintStringNL
	.align	2
	.globl	pio_uart_inb
	.ent	pio_uart_inb
pio_uart_inb:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	#.set	volatile
	lbu	$2,0($4)
	#.set	novolatile
	j	$31
	.end	pio_uart_inb
	.align	2
	.globl	pio_uart_outb
	.ent	pio_uart_outb
pio_uart_outb:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	andi	$5,$5,0x00ff
	#.set	volatile
	sb	$5,0($4)
	#.set	novolatile
	j	$31
	.end	pio_uart_outb
	.align	2
	.globl	pio_uart_delay
	.ent	pio_uart_delay
pio_uart_delay:
	.frame	$sp,8,$31		# vars= 8, regs= 0/0, args= 0, extra= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	subu	$sp,$sp,8
	#.set	volatile
	sw	$0,0($sp)
	#.set	novolatile
	#.set	volatile
	lw	$3,0($sp)
	#.set	novolatile
	li	$2,65535			# 0xffff
	slt	$2,$2,$3
	bne	$2,$0,$L279
	li	$4,65535			# 0xffff
$L280:
	#.set	volatile
	lw	$2,0($sp)
	#.set	novolatile
	#nop
	addu	$2,$2,1
	#.set	volatile
	sw	$2,0($sp)
	#.set	novolatile
	#.set	volatile
	lw	$3,0($sp)
	#.set	novolatile
	#nop
	slt	$3,$4,$3
	beq	$3,$0,$L280
$L279:
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,8
	.set	macro
	.set	reorder

	.end	pio_uart_delay
	.align	2
	.globl	pio_uart_putc
	.ent	pio_uart_putc
pio_uart_putc:
	.frame	$sp,32,$31		# vars= 0, regs= 4/0, args= 16, extra= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,32
	sw	$16,16($sp)
	li	$16,10000			# 0x2710
	sw	$18,24($sp)
	move	$18,$4
	li	$4,402653184			# 0x18000000
	ori	$4,$4,0x406
	sw	$31,28($sp)
	.set	noreorder
	.set	nomacro
	jal	pio_uart_inb
	sw	$17,20($sp)
	.set	macro
	.set	reorder

	andi	$2,$2,0x40
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L286
	li	$17,-1			# 0xffffffff
	.set	macro
	.set	reorder

	addu	$16,$16,-1
	.set	noreorder
	.set	nomacro
	beq	$16,$17,$L308
	li	$4,402653184			# 0x18000000
	.set	macro
	.set	reorder

$L309:
	.set	noreorder
	.set	nomacro
	jal	pio_uart_inb
	ori	$4,$4,0x406
	.set	macro
	.set	reorder

	andi	$2,$2,0x40
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L286
	li	$4,402653184			# 0x18000000
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	pio_uart_inb
	ori	$4,$4,0x406
	.set	macro
	.set	reorder

	andi	$2,$2,0x40
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L286
	li	$4,402653184			# 0x18000000
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	pio_uart_inb
	ori	$4,$4,0x406
	.set	macro
	.set	reorder

	andi	$2,$2,0x40
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L286
	li	$4,402653184			# 0x18000000
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	pio_uart_inb
	ori	$4,$4,0x406
	.set	macro
	.set	reorder

	andi	$2,$2,0x40
	.set	noreorder
	.set	nomacro
	bne	$2,$0,$L286
	addu	$16,$16,-4
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	bne	$16,$17,$L309
	li	$4,402653184			# 0x18000000
	.set	macro
	.set	reorder

$L286:
	li	$4,402653184			# 0x18000000
$L308:
	ori	$4,$4,0x403
	.set	noreorder
	.set	nomacro
	jal	pio_uart_outb
	andi	$5,$18,0x00ff
	.set	macro
	.set	reorder

	lw	$31,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	pio_uart_putc
	.align	2
	.globl	pio_uart_getc
	.ent	pio_uart_getc
pio_uart_getc:
	.frame	$sp,24,$31		# vars= 0, regs= 1/0, args= 16, extra= 0
	.mask	0x80000000,-8
	.fmask	0x00000000,0
	subu	$sp,$sp,24
	sw	$31,16($sp)
	li	$4,402653184			# 0x18000000
$L315:
	.set	noreorder
	.set	nomacro
	jal	pio_uart_inb
	ori	$4,$4,0x406
	.set	macro
	.set	reorder

	andi	$2,$2,0x1
	.set	noreorder
	.set	nomacro
	beq	$2,$0,$L315
	li	$4,402653184			# 0x18000000
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	pio_uart_inb
	ori	$4,$4,0x403
	.set	macro
	.set	reorder

	lw	$31,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,24
	.set	macro
	.set	reorder

	.end	pio_uart_getc
	.align	2
	.globl	pio_uart_puts
	.ent	pio_uart_puts
pio_uart_puts:
	.frame	$sp,32,$31		# vars= 0, regs= 4/0, args= 16, extra= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,32
	sw	$17,20($sp)
	move	$17,$4
	li	$4,402653184			# 0x18000000
	ori	$4,$4,0x400
	li	$5,131			# 0x83
	sw	$31,28($sp)
	sw	$18,24($sp)
	.set	noreorder
	.set	nomacro
	jal	pio_uart_outb
	sw	$16,16($sp)
	.set	macro
	.set	reorder

	li	$4,402653184			# 0x18000000
	ori	$4,$4,0x403
	.set	noreorder
	.set	nomacro
	jal	pio_uart_outb
	li	$5,12			# 0xc
	.set	macro
	.set	reorder

	li	$4,402653184			# 0x18000000
	ori	$4,$4,0x402
	.set	noreorder
	.set	nomacro
	jal	pio_uart_outb
	move	$5,$0
	.set	macro
	.set	reorder

	li	$4,402653184			# 0x18000000
	ori	$4,$4,0x400
	.set	noreorder
	.set	nomacro
	jal	pio_uart_outb
	li	$5,3			# 0x3
	.set	macro
	.set	reorder

	li	$4,402653184			# 0x18000000
	ori	$4,$4,0x407
	.set	noreorder
	.set	nomacro
	jal	pio_uart_outb
	li	$5,11			# 0xb
	.set	macro
	.set	reorder

	lb	$16,0($17)
	#nop
	.set	noreorder
	.set	nomacro
	beq	$16,$0,$L318
	addu	$17,$17,1
	.set	macro
	.set	reorder

	li	$18,10			# 0xa
$L319:
	bne	$16,$18,$L320
	.set	noreorder
	.set	nomacro
	jal	pio_uart_putc
	li	$4,13			# 0xd
	.set	macro
	.set	reorder

$L320:
	.set	noreorder
	.set	nomacro
	jal	pio_uart_putc
	move	$4,$16
	.set	macro
	.set	reorder

	lb	$16,0($17)
	#nop
	.set	noreorder
	.set	nomacro
	bne	$16,$0,$L319
	addu	$17,$17,1
	.set	macro
	.set	reorder

$L318:
	jal	pio_uart_delay
	lw	$31,28($sp)
	lw	$18,24($sp)
	lw	$17,20($sp)
	lw	$16,16($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,32
	.set	macro
	.set	reorder

	.end	pio_uart_puts
	.rdata
	.align	2
$LC21:
	.ascii	"0123456789abcdef\000"
	.text
	.align	2
	.globl	pio_uart_puthex
	.ent	pio_uart_puthex
pio_uart_puthex:
	.frame	$sp,96,$31		# vars= 64, regs= 4/0, args= 16, extra= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	subu	$sp,$sp,96
	sw	$17,84($sp)
	move	$17,$4
	li	$4,402653184			# 0x18000000
	ori	$4,$4,0x400
	sw	$18,88($sp)
	move	$18,$5
	sw	$31,92($sp)
	sw	$16,80($sp)
	la	$7,$LC21
	lwl	$2,0($7)
	lwr	$2,3($7)
	lwl	$3,4($7)
	lwr	$3,7($7)
	lwl	$6,8($7)
	lwr	$6,11($7)
	swl	$2,16($sp)
	swr	$2,19($sp)
	swl	$3,20($sp)
	swr	$3,23($sp)
	swl	$6,24($sp)
	swr	$6,27($sp)
	lwl	$2,12($7)
	lwr	$2,15($7)
	lb	$3,16($7)
	swl	$2,28($sp)
	swr	$2,31($sp)
	sb	$3,32($sp)
	.set	noreorder
	.set	nomacro
	jal	pio_uart_outb
	li	$5,128			# 0x80
	.set	macro
	.set	reorder

	li	$4,402653184			# 0x18000000
	ori	$4,$4,0x403
	.set	noreorder
	.set	nomacro
	jal	pio_uart_outb
	li	$5,12			# 0xc
	.set	macro
	.set	reorder

	li	$4,402653184			# 0x18000000
	ori	$4,$4,0x402
	.set	noreorder
	.set	nomacro
	jal	pio_uart_outb
	move	$5,$0
	.set	macro
	.set	reorder

	li	$4,402653184			# 0x18000000
	ori	$4,$4,0x400
	.set	noreorder
	.set	nomacro
	jal	pio_uart_outb
	li	$5,3			# 0x3
	.set	macro
	.set	reorder

	addu	$16,$sp,16
	srl	$2,$17,28
	addu	$2,$16,$2
	.set	noreorder
	.set	nomacro
	jal	pio_uart_putc
	lb	$4,0($2)
	.set	macro
	.set	reorder

	srl	$2,$17,24
	andi	$2,$2,0xf
	addu	$2,$16,$2
	.set	noreorder
	.set	nomacro
	jal	pio_uart_putc
	lb	$4,0($2)
	.set	macro
	.set	reorder

	srl	$2,$17,20
	andi	$2,$2,0xf
	addu	$2,$16,$2
	.set	noreorder
	.set	nomacro
	jal	pio_uart_putc
	lb	$4,0($2)
	.set	macro
	.set	reorder

	srl	$2,$17,16
	andi	$2,$2,0xf
	addu	$2,$16,$2
	.set	noreorder
	.set	nomacro
	jal	pio_uart_putc
	lb	$4,0($2)
	.set	macro
	.set	reorder

	srl	$2,$17,12
	andi	$2,$2,0xf
	addu	$2,$16,$2
	.set	noreorder
	.set	nomacro
	jal	pio_uart_putc
	lb	$4,0($2)
	.set	macro
	.set	reorder

	srl	$2,$17,8
	andi	$2,$2,0xf
	addu	$2,$16,$2
	.set	noreorder
	.set	nomacro
	jal	pio_uart_putc
	lb	$4,0($2)
	.set	macro
	.set	reorder

	srl	$2,$17,4
	andi	$2,$2,0xf
	addu	$2,$16,$2
	.set	noreorder
	.set	nomacro
	jal	pio_uart_putc
	lb	$4,0($2)
	.set	macro
	.set	reorder

	andi	$17,$17,0xf
	addu	$16,$16,$17
	.set	noreorder
	.set	nomacro
	jal	pio_uart_putc
	lb	$4,0($16)
	.set	macro
	.set	reorder

	beq	$18,$0,$L329
	.set	noreorder
	.set	nomacro
	jal	pio_uart_putc
	li	$4,13			# 0xd
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	pio_uart_putc
	li	$4,10			# 0xa
	.set	macro
	.set	reorder

$L329:
	jal	pio_uart_delay
	lw	$31,92($sp)
	lw	$18,88($sp)
	lw	$17,84($sp)
	lw	$16,80($sp)
	#nop
	.set	noreorder
	.set	nomacro
	j	$31
	addu	$sp,$sp,96
	.set	macro
	.set	reorder

	.end	pio_uart_puthex

	.comm	sysProcNum,4

	.comm	sysFlags,4

	.comm	sysBootHost,80

	.comm	sysBootFile,80

	.comm	sysTodFuncs,20

	.comm	sysRevId,4

	.lcomm	sysClkProcessorTicks,4

	.lcomm	sysModelStr,200

	.comm	sysToMonitorHook,4
