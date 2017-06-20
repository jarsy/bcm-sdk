/* $Id: romInit.s,v 1.10 2011/07/21 16:14:21 yshtil Exp $
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/

/* romInit.s - BCM47XX ROM initialization module */
/* modified from IDT S134 ROM initialization module */

/* Copyright 1984-1996 Wind River Systems, Inc. */
        .data
        .globl  copyright_wind_river
	
/*
DESCRIPTION
This module contains the entry code for the VxWorks bootrom.
The entry point romInit, is the first code executed on power-up.

The routine sysToMonitor() jumps to romInit() to perform a
"warm boot".

*/

#define _ASMLANGUAGE            /* used by vxworks include files */
#define DDR16MX16X2             /* DDR memory config */

#include "vxWorks.h"
#include "arch/mips/ivMips.h"
#include "arch/mips/asmMips.h"
#include "arch/mips/esfMips.h"
#include "sysLib.h"
#include "config.h"
#include "bcm4704.h"
#include "sbmemc.h"
#include "sbconfig.h"
	
#define   RAM_DST_ADRS  RAM_HIGH_ADRS

/* Generic Sentry5 parameters */
#define CFG_SERIAL_BAUD_RATE	115200	/* normal console speed */
#define BASE_BAUD  CFG_SERIAL_BAUD_RATE
#define UART_CLOCK 1850000
#define UART_BASE   0xb8000300 /* COM 1 */
#define UART_BASE2  0xb8000400
#define CHIPC_BASE  0xb8000000
#define DIV_LO     ((UART_CLOCK/16)/BASE_BAUD)
#define DIV_HI     (DIV_LO>>8)	

/* System console LEDs and the GPIO pin state */
#define LED_GREEN	0x000000fd
#define LED_RED		0x0000007f
#define LED_YELLOW	0x000000bf		

#if defined(_BCM95365P_) || defined(_BCM95365R_)
/* Set GPIO pin state:	 uses t5 ($13) and t6 ($14) */		
#define SYSLED(val)	        \
	li	$13, CHIPC_BASE ;\
	li	$14, val        ;\
	sw	$14, 0x64($13)   ; 
#else
#define SYSLED(val)
#endif		
	
	
#define UART_WAR \
	lb	t5, 0(t4)	
	
#define SENDCHAR(reg) \
	li t0,UART_BASE ; \
1:	lb t1,5(t0) ; \
	and t1,0x20 ; \
	beq t1,zero,1b ; \
	nop            ; \
	sb  reg,0(t0)  ; \
	UART_WAR

	/* internals */

	.globl	romInit			/* start of system code */
    .globl  romReboot       /* sw reboot address */
	/* externals */

	.globl	romStart		/* system initialization routine */
	/* external defs */

	.extern	end     0		/* end of program */
	.extern	etext	0		/* end of text section */
	.extern	_edata	0		/* start of bss section , Suggest by Wind River */
	.extern	_fdata	0		/* start of data section */

	.data

	/* ensure data segment is 16-byte aligned */

	.align 	4
_sdata:
	.asciiz	"start of data"

	.text

/*
 * relocate an address
 * Make the address an uncached address
 * This will trash ra!
 */
#define RELOC(toreg,address)      \
        bal     9f              ; \
9:                              ; \
        la      toreg,address   ; \
        addu    toreg,ra        ; \
        la      ra,9b           ; \
        subu    toreg,ra

/*
 * Mark(#if 0) temporarily for compiler error on vxWorks 5.5.
 * Suggest by WindRiver.
 */
#define BPRINT_ASM(msg)
#if 0
#define BPRINT_ASM(msg)         \
        RELOC(a0,msg)           ;\
        bal     displaymsg      ;\
        nop
#endif

#define HANG    \
1:      nop     ;\
        b 1b

#define RVECENT(f,n) \
	b f; nop
#define XVECENT(f,bev) \
	b f; li k0,bev


	.text
promEntry:
romInit:
_romInit:
	.set	noreorder
	RVECENT(__romInit,0)		/* PROM entry point */
	RVECENT(romReboot,1)		/* software reboot */
	RVECENT(romReserved,2)
	RVECENT(romReserved,3)
	RVECENT(romReserved,4)
	RVECENT(romReserved,5)
	RVECENT(romReserved,6)
	RVECENT(romReserved,7)
	RVECENT(romReserved,8)
	RVECENT(romReserved,9)
	RVECENT(romReserved,10)
	RVECENT(romReserved,11)
	RVECENT(romReserved,12)
	RVECENT(romReserved,13)
	RVECENT(romReserved,14)
	RVECENT(romReserved,15)
	RVECENT(romReserved,16)
	RVECENT(romReserved,17) 
	RVECENT(romReserved,18)
	RVECENT(romReserved,19)
	RVECENT(romReserved,20)
	RVECENT(romReserved,21)
	RVECENT(romReserved,22)
	RVECENT(romReserved,23)
	RVECENT(romReserved,24)
	RVECENT(romReserved,25)
	RVECENT(romReserved,26)
	RVECENT(romReserved,27)
	RVECENT(romReserved,28)
	RVECENT(romReserved,29)
	RVECENT(romReserved,30)
	RVECENT(romReserved,31)
	RVECENT(romReserved,32)
	RVECENT(romReserved,33)
	RVECENT(romReserved,34)
	RVECENT(romReserved,35)
	RVECENT(romReserved,36)
	RVECENT(romReserved,37)
	RVECENT(romReserved,38)
	RVECENT(romReserved,39)
	RVECENT(romReserved,40)
	RVECENT(romReserved,41)
	RVECENT(romReserved,42)
	RVECENT(romReserved,43)
	RVECENT(romReserved,44)
	RVECENT(romReserved,45)
	RVECENT(romReserved,46)
	RVECENT(romReserved,47)
#if CPU==RC32364
        XVECENT(romExcHandle,0x380)
#else
	RVECENT(romReserved,48)
#endif
	RVECENT(romReserved,49)
	RVECENT(romReserved,50)
	RVECENT(romReserved,51)
	RVECENT(romReserved,52)
	RVECENT(romReserved,53)
	RVECENT(romReserved,54)
	RVECENT(romReserved,55)
	RVECENT(romReserved,56)
	RVECENT(romReserved,57)
	RVECENT(romReserved,58)
	RVECENT(romReserved,59)
	RVECENT(romReserved,60)
	RVECENT(romReserved,61)
	RVECENT(romReserved,62)
	RVECENT(romReserved,63)	
#if CPU_VAR==RC32364
	XVECENT(romExcHandle,0x200)	/* bfc00200: RC32364 tlbmiss vector */
#else
	RVECENT(romReserved,64)
#endif
	RVECENT(romReserved,65)
	RVECENT(romReserved,66)
	RVECENT(romReserved,67)
	RVECENT(romReserved,68)
	RVECENT(romReserved,69)
	RVECENT(romReserved,70)
	RVECENT(romReserved,71)
	RVECENT(romReserved,72)
	RVECENT(romReserved,73)
	RVECENT(romReserved,74)
	RVECENT(romReserved,75)
	RVECENT(romReserved,76)
	RVECENT(romReserved,77)
	RVECENT(romReserved,78)
	RVECENT(romReserved,79)	
	RVECENT(romReserved,80)
	RVECENT(romReserved,81)
	RVECENT(romReserved,82)
	RVECENT(romReserved,83)
	RVECENT(romReserved,84)
	RVECENT(romReserved,85)
	RVECENT(romReserved,86)
	RVECENT(romReserved,87)
	RVECENT(romReserved,88)
	RVECENT(romReserved,89)
	RVECENT(romReserved,90)
	RVECENT(romReserved,91)
	RVECENT(romReserved,92)
	RVECENT(romReserved,93)
	RVECENT(romReserved,94)
	RVECENT(romReserved,95)	
#if CPU_VAR==RC32364
	XVECENT(romExcHandle,0x300)	/* bfc00300: RC32364 cache vector */
#else
	RVECENT(romReserved,96)
#endif
	RVECENT(romReserved,97)
	RVECENT(romReserved,98)
	RVECENT(romReserved,99)
	RVECENT(romReserved,100)
	RVECENT(romReserved,101)
	RVECENT(romReserved,102)
	RVECENT(romReserved,103)
	RVECENT(romReserved,104)
	RVECENT(romReserved,105)
	RVECENT(romReserved,106)
	RVECENT(romReserved,107)
	RVECENT(romReserved,108)
	RVECENT(romReserved,109)
	RVECENT(romReserved,110)
	RVECENT(romReserved,111)
#if CPU_VAR==RC32364
	XVECENT(romExcHandle,0x380)	/* bfc00380: RC32364 general vector */
#else
	RVECENT(romReserved,112)
#endif
	RVECENT(romReserved,113)
	RVECENT(romReserved,114)
	RVECENT(romReserved,115)
	RVECENT(romReserved,116)
	RVECENT(romReserved,116)
	RVECENT(romReserved,118)
	RVECENT(romReserved,119)
	RVECENT(romReserved,120)
	RVECENT(romReserved,121)
	RVECENT(romReserved,122)
	RVECENT(romReserved,123)
	RVECENT(romReserved,124)
	RVECENT(romReserved,125)
	RVECENT(romReserved,126)
	RVECENT(romReserved,127)

	/* We hope there are no more reserved vectors!
	 * 128 * 8 == 1024 == 0x400
	 * so this is address R_VEC+0x400 == 0xbfc00400
	 */
	.set reorder
	.align 4

/********************************************************************************
*
* romInit - entry point for VxWorks in ROM
*
* romInit 
*     (
*     int startType
*     )
*/

__romInit:			/* force power-on startType        */
    .set    nomove

    # set SR and CAUSE to something sensible
    li      v0,SR_BEV

    .set noreorder
    .set noat

    mtc0    v0,C0_SR
    mtc0    zero,C0_CAUSE
    mtc0    zero,$18        # C0_WATCHLO
    mtc0    zero,$19        # C0_WATCHHI


    /***********
     * Initialize the hardware
     ***********/

    li      t0, 2
    mtc0    t0, $16
    nop
	.set	reorder
	.set	at

	li	a2, CHIPC_BASE
	li	t4, CHIPC_BASE
#if 0
	/* Program FLASH wait count, enable Asynch I/f */
	li	a3, 0x11        /* Enable Parallel FLASH Mode on FLASH_CS */
	sw	a3, 0x128(a2)   /* Write to ParalleFLASHConfig to enable */
#endif
    /* Enable M_CS1 CS01 Enable*/
	li	a3, 0x1
	sw	a3, 0x100(a2)   /* Enable EBI for Parallel/Async mode*/

        li      t1, 0xba004000  /* board ID Addr */
        lbu     t1, 0(t1)       /* board ID */
        bne     t1, 0x1C, 1f    /* BOARD_ID (BCM95836EB) = 0x1c */
        nop
        /* Enable CS2 for async flash interface for EB bus */
	li	a3, 0x70        /* Enable async mode */
	sw	a3, 0x110(a2)   /* CS23Config, Enable */
	li	a3, 0x01020a0c	/* CS23 Timing parameters */
	sw	a3, 0x114(a2)	/* Set timing for Local bus */
	li	a3, 0x71        /* Enable CS2 */
	sw	a3, 0x110(a2)   /* CS23Config, Enable */
        b       2f
        nop
1:
    /* Enable M_CS2 */
	li	a3, 0x2        /* Enable sync clock used by FPGA */
	sw	a3, 0x008(a2)   /* Enable EBI for Parallel/Async mode*/
	li	a3, 0x83        /* Enable sync clock used by FPGA */
	sw	a3, 0x110(a2)   /* CS23Config, Enable */

    /* Enable MCS3 */
	li	a3, 0x1
	sw	a3, 0x120(a2)   /* CS4Config, Enable Expansion bus */
	li	a3, 0x01020a0c	/* CS4 Timing parameters */
	sw	a3, 0x124(a2)	/* Set timing for Local bus */

    /* Set timing for CS01 CS23 Flashwait count */
    li  a3, 0x100010a
	sw	a3, 0x104(a2)	/* Set timing for Local bus */
	sw	a3, 0x114(a2)	/* Set timing for Local bus */
	sw	a3, 0x124(a2)	/* Set timing for Local bus */
	sw	a3, 0x12c(a2)	/* Set timing for Local bus */
2:
	/* Change clock div on 4704 to 0x36 (54) */
	li	a2, CHIPC_BASE
	li	a3, 0x00000036 /* @ 100 Mhz backplane clock UART Freq = 1.85 Mhz */
	sw	a3, 0xa4(a2)
	lw	t5, 0(a2)       /* Read Chip ID */

    BPRINT_ASM(Init_msg)

/* Setup GPIO state so we can turn on the  LEDs */		
/* #define _BCM95365R_ 1	*/
#if defined(_BCM95365P_) || defined(_BCM95365R_)
	/* Negate reset */	
	li	a3, 0x000000ff   
	sw	a3, 0x64(a2)
    /* Set GPIO 0,1,6,7 as output for LEDs */
	/* Set as an output and enable on all GPIO's */
	li	a2, CHIPC_BASE
	li	a3, 0x000000c3
	sw	a3, 0x68(a2)
#endif		
		
	SYSLED(LED_GREEN)		
	
    BPRINT_ASM(ddrm_init)

	/* Initialize DDR/SDR */
	bal board_draminit

    BPRINT_ASM(ddrm_msg)
		
	SYSLED(LED_YELLOW)

	/* Ram init done */

	li      a0, BOOT_CLEAR	/* Says to clear mem.BOOT_NORMAL says don't. */

	/*
 	 * If there was some way to distinguish between a cold and warm
	 * restart AND the memory system is guaranteed to be intact then
     * we could load BOOT_NORMAL instead
 	 */

romReboot:                          /* sw reboot inherits a0 startType */
    move    s0, a0                  /* save startType */
        
	.set	nomove
	/* disable all interrupts,  */

    li      t1, 0xfffffffe
	li	    t0, SR_BEV           /* select boot exception handlers */ 
    and     t0, t1           /* IE bit in status reg becomes 0 */
    li      t1, ~SR_IE
    and     t0, t1
	mtc0	t0, C0_SR
	/* clear software interrupts */
	mtc0	zero, C0_CAUSE
	mtc0	zero, C0_IWATCH
	mtc0	zero, C0_DWATCH
    .set    move

#if 1
	/* On MIPS 33xx, use CP0 diagnostic to turn on the caches */
	mfc0	v0,$22
    nop
    nop
    nop
	or      v0,0xc0000000		/* Enable both I$ and D$ */
	mtc0	v0,$22
    nop
    nop
    /* Enable Prefetch Cache (RAC) */
    lui     v0, 0xff40
    lui     t1, 0x0400
    sw      t1, 4(v0)
    li      t1, 0x0000          /* Disable Prefectch for I$ & D$ */
    sw      t1, 0(v0)
#endif
    andi    a0, s0, BOOT_CLEAR

	/*initialise caches */	
	bal	romCacheReset

    andi    t0, s0, BOOT_CLEAR
	beqz    t0, romWarm

#ifdef ECCMODE
#error "ECCMODE defined"
	bal	romClearEdac
	nop
	b 	6f
	nop
#endif
	
romWarm:

	SYSLED(LED_RED)

/* Memory Test */
#if 0
    li	a0,0xa0000000
	li	a1,0xa3FFFFF0
	subu	a1,a0
	srl	a1,2

	bal	_mtest
    nop

    BPRINT_ASM(chkp_msg)
#endif

/* Flash Read Test */
#if 0
	bal	_ftest
    nop
#endif

	SYSLED(LED_YELLOW)
    BPRINT_ASM(bss_msg)
	/* zap the BSS */
#if 1 /* zero out the entire memory */
    la	a0,_edata
	la	a1,_end
#else
    li	a0,0x80000000
	li	a1,0x83FFFFF0
#endif
	subu	a1,a0
	srl     a1,2
    li      a2,0xfffffff0
    and     a1,a1,a2
    li      a2,0xfffffff0
    and     a0,a0,a2
	move	a2,zero

	bal	_bfillLongs
    nop

6:
    BPRINT_ASM(stak_msg)
	SYSLED(LED_GREEN)
	/* give us as long as possible before a clock interrupt */ 
	li	t0, 1
	mtc0	t0, C0_COUNT
	mtc0	zero, C0_COMPARE 
#if 0
    li t0, 0xa0000000
    /* li t1, 0x02000000*/
    li t1, 0x01000000
    li  t2, 0
1:
	sw	t2, (t0); 
	subu	t0,4
	subu	t1,4
	bnez	t1,1b
#endif

	/* set stack to grow down from beginning of data and call init */
	la	gp, _gp             /* set global ptr from compiler */
	la	sp, STACK_ADRS      /* set stack to begin of data */
	subu	sp, 16          /* give me some room */

    BPRINT_ASM(roms_msg)

    move    a0, s0          /* push arg = start type */
	RELOC(t0, romStart)

	jal	t0                  /* never returns - starts up kernel  */

	j	ra                  /* just in case */


1:	b	1b

Init_msg:
	.ascii	"Init"

r_Fail:
	.ascii	"Fail"

clk_set:
    .ascii  "CLKS"

ddrm_init:
    .ascii  "DDIN"

roms_msg:
    .ascii  "ROMS"

sdrm_msg:
	.ascii "SDRM"

ddrm_msg:
	.ascii "DDRM"

bss_msg:
	.ascii "BSS "

chkp_msg:
	.ascii "CHKP"

stak_msg:
	.ascii "STAK"

			
/*******************************************************************************
*
* romReserved -	 Handle a jump to an unknown vector
*
* romReserved ()
*
*/

	.ent	romReserved
romReserved:
	b	romInit		/* just start over */
	.end	romReserved


/*******************************************************************************
*
* _mtest -	 Memory Test
*
* _mtest ()
*
*/

	.ent	_mtest
_mtest:

    move    v0,ra           /* save return address */
    bltu    a1,8,2f

    move    a2,a0           /* save a0 */
    move    a3,a1           /* save a1 */

#if 0 /* word mode mem test */
    RELOC(t0,1f)            /* run the loop from cache */
	nop
	j	t0

/* word write and compare */
1:
	sw	a0,0(a0)

	subu	a1,1
	addu	a0,4

	bgeu	a1,1,1b

2:
    move    a0,a2           /* restore a0 */
    move    a1,a3           /* restore a1 */

1:
	lw	t0,0(a0)
	sw	zero,0(a0)

    bne a0, t0, 4f

	subu	a1,1
	addu	a0,4

	bgeu	a1,1,1b

    nop
#endif

#if 1 /* Byte mode mem test */
    move    a0,a2           /* restore a0 */
    move    a1,a3           /* restore a1 */

1:
    move t0, a0
    andi t0, t0, 0xff
	sb	t0,3(a0)

    srl t0,a0,8
    andi t0, t0, 0xff
	sb	t0,2(a0)

    srl t0,a0,16
    andi t0, t0, 0xff
	sb	t0,1(a0)

    srl t0,a0,24
    andi t0, t0, 0xff
	sb	t0,0(a0)

	subu	a1,1
	addu	a0,4

	bgeu	a1,1,1b

2:
    move    a0,a2           /* restore a0 */
    move    a1,a3           /* restore a1 */

1:
	lw	t0,0(a0)

    bne a0, t0, 4f

	subu	a1,1
	addu	a0,4

	bgeu	a1,1,1b

#endif
	beqz	a1,2f

4: nop
    li	t0,0x80000000
	sw	a0,0(t0)
    BPRINT_ASM(r_Fail)
    SYSLED(LED_GREEN)
    SYSLED(LED_YELLOW)
    SYSLED(LED_RED)
    b   4b
/* byte write and compare */
/* 2:*/
	
2:	j	v0

	.end	_mtest

/* a0 = ROM Start address; a1 RAM copy addr ; a2 size */
	.ent	_ftest
_ftest:

    move    v0,ra           /* save return address */

    li      a0, 0xbfc00000
    li      a1, 0x80100000
    li      a2, 0x80000

    RELOC(t0,1f)            /* run the loop from cache */
	nop
	j	t0

/* word write and compare */
1:
	lb	t0,0(a0)
	sb	t0,0(a1)

	addu	a0,1
	addu	a1,1
	subu	a2,1

	bgeu	a2,1,1b

2:
    li      a0, 0xbfc00000
    li      a1, 0x80100000
    li      a2, 0x80000

1:
	lb	t0,0(a0)
	lb	t1,0(a1)

    bne t1, t0, 4f

	addu	a0,1
	addu	a1,1
	subu	a2,1

	bgeu	a2,1,1b
    nop

	beqz    a2, 2f
4: nop
    SYSLED(LED_GREEN)
    SYSLED(LED_YELLOW)
    b   4b
	
2:	j	v0

	.end	_ftest

/*******************************************************************************
*
* _bfillLongs -	 
*
* _bfillLongs ()
*
*/

	.ent	_bfillLongs
_bfillLongs:

    move    v0,ra           /* save return address */
    bltu    a1,8,2f

    RELOC(t0,1f)            /* run the loop from cache */
	nop
	j	t0

	
1:
	sw	a2,0(a0)
	sw	a2,4(a0)
	sw	a2,8(a0)
	sw	a2,12(a0)
	sw	a2,16(a0)
	sw	a2,20(a0)
	sw	a2,24(a0)
	sw	a2,28(a0)

	subu	a1,8
	addu	a0,32

	bgeu	a1,8,1b

2:	beqz	a1,2f

	
1:	subu	a1,1
	sw	a2,0(a0)
	addu	a0,4
	bnez	a1,1b

2:	j	v0

	.end	_bfillLongs

/*
 * cacheop macro to automate cache operations
 * first some helpers...
 */
#define _mincache(size, maxsize) \
    .set noat ; \
    sltu    AT,size,maxsize;         \
    bnez    AT,9f     ;         \
	move   size,maxsize ;		\
    .set at ; \
9:

#define _align(minaddr, maxaddr, linesize) \
    .set noat;\
	subu	AT,linesize,1 ;	\
	not	AT ;			\
	and	minaddr,AT ;		\
	addu	maxaddr,-1 ;		\
	and	maxaddr,AT ;		\
    .set at

#define CACHELINE_SIZE  16
#define ICACHE_SIZE     (16*1024)
#define DCACHE_SIZE     (16*1024)
#define KSEG0BASE       0x80000000
#define C0_TAGLO        $28
#define C0_TAGHI        $29

#define CACHE_OP( code, type )			( ((code) << 2) | (type) )

#define ICACHE_INDEX_INVALIDATE			CACHE_OP(0x0, 0)
#define ICACHE_INDEX_LOAD_TAG			CACHE_OP(0x1, 0)
#define ICACHE_INDEX_STORE_TAG			CACHE_OP(0x2, 0)
#define DCACHE_INDEX_WRITEBACK_INVALIDATE	CACHE_OP(0x0, 1)
#define DCACHE_INDEX_LOAD_TAG			CACHE_OP(0x1, 1)
#define DCACHE_INDEX_STORE_TAG			CACHE_OP(0x2, 1)

#define ICACHE_ADDR_HIT_INVALIDATE		CACHE_OP(0x4, 0)
#define ICACHE_ADDR_FILL			CACHE_OP(0x5, 0)
#define ICACHE_ADDR_FETCH_LOCK			CACHE_OP(0x7, 0)
#define DCACHE_ADDR_HIT_INVALIDATE		CACHE_OP(0x4, 1)
#define DCACHE_ADDR_HIT_WRITEBACK_INVALIDATE	CACHE_OP(0x5, 1)
#define DCACHE_ADDR_HIT_WRITEBACK		CACHE_OP(0x6, 1)
#define DCACHE_ADDR_FETCH_LOCK			CACHE_OP(0x7, 1)

/*******************************************************************************
*
* romCacheReset - low level initialisation of the primary caches
*
*
* RETURNS: N/A
*
* void romCacheReset
*/
    .set mips3
	.set noreorder
	.ent	romCacheReset
romCacheReset:
	
    /*  a0 = cache size
	 *  a1 = line  size
	 */
    li      a0, ICACHE_SIZE
    li      a1, CACHELINE_SIZE 

    /* CFLUSH */

	mtc0    zero, C0_TAGLO 
	mtc0    zero, C0_TAGHI 	    /* TagHi is not really used */
	
	/* Calc an address that will correspond to the first cache line */
	li	a2, KSEG0BASE

	/* Calc an address that will correspond to the last cache line  */
	addu	a3, a2, a0
	subu    a3, a1

	/* Loop through all lines, invalidating each of them */
1:	
	cache	ICACHE_INDEX_STORE_TAG, 0(a2)	/* clear tag */
	bne	a2, a3, 1b
	addu	a2, a1

    /* DFLUSH INV */
                
	/*  a0 = cache size
	 *  a1 = line  size
	 */
    li      a0, DCACHE_SIZE
    li      a1, CACHELINE_SIZE 

	mtc0    zero, C0_TAGLO 
	mtc0    zero, C0_TAGHI 	    /* TagHi is not really used */
	
	/* Calc an address that will correspond to the first cache line */
	li	a2, KSEG0BASE

	/* Calc an address that will correspond to the last cache line  */
	addu	a3, a2, a0
	subu    a3, a1

	/* Loop through all lines, invalidating each of them */
1:	
	cache	DCACHE_INDEX_STORE_TAG, 0(a2)	/* clear tag */
	bne	a2, a3, 1b
	addu	a2, a1

	j	ra
	.end	romCacheReset
    .set    mips0
	.set 	reorder

/*******************************************************************************
*
* romClearEdac - clear error detection and correction logic
*
* This routine clears the memory and error detection logic by
* doing word writes to each DRAM location.  It sizes memory by
* probing memory locations.
*/

	.ent	romClearEdac
romClearEdac:

#if 0
	mfc0	v1,C0_SR	/* disable parity errors */
	or	    v0,v1,SR_DE
	mtc0	v0,C0_SR
#endif
	
	move 	v0,ra		/* save return address */

    RELOC(t0,1f)            /* run the loop from cache */
    and     t0,0x1fffffff
    or      t0,K0BASE
    j       t0
1:


	li     a0,(LOCAL_MEM_SIZE | K1BASE )
        
clearloop:
	sw	zero, -4(a0)
	sw	zero, -8(a0)
	sw	zero, -12(a0)
	sw	zero, -16(a0)
	sw	zero, -20(a0)
	sw	zero, -24(a0)
	sw	zero, -28(a0)
	sw	zero, -32(a0)
	subu	a0, 32
	bne	a0, K1BASE, clearloop 
        nop
done:

#	mtc0	v1,C0_SR
	j	v0
	.end	romClearEdac

/*******************************************************************************
*
* displaymsg - display a 4-character message on the alpha LEDS
*
*/
        .globl  displaymsg
        .ent    displaymsg
displaymsg:
    li  a2, MBZ_LED_ADDR_ASM
    lbu     a1,0(a0)        /* get char from bootrom */
    sb      a1,0(a2)

    lbu     a1,1(a0)
    sb      a1,1(a2)

    lbu     a1,2(a0)
    sb      a1,2(a2)

    lbu     a1,3(a0)
    sb      a1,3(a2)

    j       ra
    nop
    .end   displaymsg

/**********************************************************************
 * board_draminit
 * Setup SDR/DDR ram based ondefine below.
 * NOTE: assumes v0 filled with ra before routine runs
 */	
/*
 * Memory segments (32bit kernel mode addresses)
 */
#define KUSEG			0x00000000
#define KSEG0			0x80000000
#define KSEG1			0xa0000000
#define KSEG2			0xc0000000
#define KSEG3			0xe0000000

	
/*
 * Map an address to a certain kernel segment
 */

#define _KSEG0ADDR(a)		(((a) & 0x1fffffff) | KSEG0)
#define _KSEG1ADDR(a)		(((a) & 0x1fffffff) | KSEG1)
#define _KSEG2ADDR(a)		(((a) & 0x1fffffff) | KSEG2)
#define _KSEG3ADDR(a)		(((a) & 0x1fffffff) | KSEG3)
	.ent	board_draminit
	.set	noreorder
	.align	4
	.text
board_draminit:	

	SYSLED(LED_RED)
	
	/* Save return address */
	move	v0,ra

	/* Scan for an SDRAM controller (a0) */
	li	a0, _KSEG1ADDR(SB_ENUM_BASE)
1:	lw	a1, (SBCONFIGOFF + SBIDHIGH)(a0)
	and	a1, a1, SBIDH_CC_MASK
	srl	a1, a1, SBIDH_CC_SHIFT
	beq	a1,SB_MEMC,read_nvram
	nop
	beq	a1,SB_SDRAM,read_nvram
	nop
	addu	a0, SB_CORE_SIZE
	bne	a1, (SBIDH_CC_MASK >> SBIDH_CC_SHIFT), 1b
	nop

    BPRINT_ASM(chkp_msg)
    HANG;
	/* No SDRAM controller */
	j	v0 /* ra */
	nop	

	/* Special sb_core_reset that makes sure the first time
	 * clock is enabled, address line 6 is in the state specified
	 * by a2.
	 *
	 * a0:	Core pointer
	 * a2:	0x40 if a6 needs to be 1, 0 otherwise
	 * uses t8, t9
	 */
memc_sb_core_reset:

	/* Save return address */
	move	t9,ra

	/* Figure out our address */
	bal	h0
	nop
h0:	add	t8,ra,24		# This is (h1 - h0)
	andi	t8,t8,0x40
	bne	t8,a2,alt_core_reset
	nop

	/* Set reset while enabling the clock */
	li	t8,(SBTML_FGC | SBTML_CLK | SBTML_RESET)
h1:	sw	t8,(SBCONFIGOFF + SBTMSTATELOW)(a0)
	b	cont
	nop

	/* Now pad to 0x40: We want (h2 - h1) == 0x40 and there
	 * are 5 instructions inbetween them.
	 */
	.space	(0x40 - 20)

alt_core_reset:
	/* Set reset while enabling the clock */
	li	t8,(SBTML_FGC | SBTML_CLK | SBTML_RESET)
h2:	sw	t8,(SBCONFIGOFF + SBTMSTATELOW)(a0)

cont:
	/* Read back and delay */
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)

	/* Clear reset */
	li	t8, (SBTML_FGC | SBTML_CLK)
	sw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)

	/* Read back and delay */
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)

	/* Leave clock enabled */
	li	t8, SBTML_CLK
	sw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)

	/* Read back and delay */
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)
	lw	t8, (SBCONFIGOFF + SBTMSTATELOW)(a0)

	jr	t9
	nop

read_nvram:
	/* Don't read from NVRAM */
	b	init			# No NVRAM
	li	a2, 0

init:
	/* Initialize SDRAM controller:	 MEMC core */

#ifndef SDRAM_MODE
	/* Perform DDR Ram initialization */	
memc_init:

	li	t0,MEMC_DDR_INIT_256MB
        li      t1, 0xba004000  /* board ID Addr */
        lbu     t1, 0(t1)       /* board ID */
        beq     t1, 0x15, lm_256MB /* BOARD_ID_JAG_CPCI2 = 0x15 */
        nop
        beq     t1, 0x16, lm_256MB /* BOARD_ID_LM_P6CX4 = 0x16 */
        nop
        beq     t1, 0x1C, lm_256MB /* BOARD_ID (BCM95836EB) = 0x1c */
        nop
	/* Assume DDRM32MX8X4 */
	li	t0,MEMC_DDR_INIT
lm_256MB:
	li	t1,MEMC_DDR_MODE

memc_ddr_init:
	/* Initialize DDR SDRAM */
	bal	memc_sb_core_reset
	li	a2,0

	li	a3,MEMC_CONFIG_INIT
	or	a3,a3,t0
	sw	a3,MEMC_CONFIG(a0)
	li	a3,MEMC_DRAMTIM_INIT
	sw	a3,MEMC_DRAMTIM(a0)
	li	a3,MEMC_RDNCDLCOR_INIT
	sw	a3,MEMC_RDNCDLCOR(a0)
	li	a3,MEMC_WRNCDLCOR_INIT
	sw	a3,MEMC_WRNCDLCOR(a0)
	li	a3,MEMC_DQSGATENCDL_INIT
	sw	a3,MEMC_DQSGATENCDL(a0)
	li	a3,MEMC_MISCDLYCTL_INIT
	sw	a3,MEMC_MISCDLYCTL(a0)
	li	a3,MEMC_NCDLCTL_INIT
	sw	a3,MEMC_NCDLCTL(a0)
	li	a3,MEMC_CONTROL_INIT0
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_CONTROL_INIT1
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_MODEBUF_INIT0
	sw	a3,MEMC_MODEBUF(a0)
	li	a3,MEMC_CONTROL_INIT2
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_MODEBUF_INIT1
	or	a3,a3,t1
	sw	a3,MEMC_MODEBUF(a0)
	li	a3,MEMC_CONTROL_INIT3
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_CONTROL_INIT4
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_REFRESH_INIT
	sw	a3,MEMC_REFRESH(a0)
	li	a3,MEMC_CONTROL_INIT5
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_CONTROL_INIT5
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_MODEBUF_INIT2
	or	a3,a3,t1
	sw	a3,MEMC_MODEBUF(a0)
	li	a3,MEMC_CONTROL_INIT6
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_CONTROL_INIT7
	sw	a3,MEMC_CONTROL(a0)

	/* XXX Determine memory size */
	b	done
	nop
#else
	/* Perform SDRAM initialization */
memc_init:

	/* Assume SDRM16Mx16X2 */
	li	t0,MEMC_SDR_INIT
	li	t1,MEMC_SDR_MODE
	
memc_sdr_init:
	/* Initialize SDRAM */
	bal	memc_sb_core_reset
	li	a2,0x40
	
	li	a3,MEMC_SD_CONFIG_INIT
	or	a3,a3,t0
	sw	a3,MEMC_CONFIG(a0)
	li	a3,MEMC_SD_DRAMTIM_INIT
	sw	a3,MEMC_DRAMTIM(a0)
	li	a3,MEMC_SD_RDNCDLCOR_INIT
	sw	a3,MEMC_RDNCDLCOR(a0)
	li	a3,MEMC_SD_WRNCDLCOR_INIT
	sw	a3,MEMC_WRNCDLCOR(a0)
	li	a3,MEMC_SD_MISCDLYCTL_INIT
	sw	a3,MEMC_MISCDLYCTL(a0)
	li	a3,MEMC_SD_CONTROL_INIT0
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_SD_CONTROL_INIT1
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_SD_REFRESH_INIT
	sw	a3,MEMC_REFRESH(a0)
	li	a3,MEMC_SD_CONTROL_INIT2
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_SD_CONTROL_INIT2
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_SD_CONTROL_INIT2
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_SD_CONTROL_INIT2
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_SD_CONTROL_INIT2
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_SD_CONTROL_INIT2
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_SD_CONTROL_INIT2
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_SD_CONTROL_INIT2
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_SD_MODEBUF_INIT
	or	a3,a3,t1
	sw	a3,MEMC_MODEBUF(a0)
	li	a3,MEMC_SD_CONTROL_INIT3
	sw	a3,MEMC_CONTROL(a0)
	li	a3,MEMC_SD_CONTROL_INIT4
	sw	a3,MEMC_CONTROL(a0)

	/* XXX Determine memory size */
#endif

memdone:
	/* Wait for SDRAM controller to refresh */
	li	t3,10000
1:	bnez	t3,1b
	subu	t3,1

	j	v0 /* ra */
	nop
	.end	board_draminit
	.set	reorder
	


	

	


	

/*******************************************************************************
*
* romExcHandle - rom based exception/interrupt handler
*
* This routine is invoked on an exception or interrupt while
* the status register is using the bootstrap exception vectors.
* It saves a state frame to a known uncached location so someone
* can examine the data over the VME.  It also displays a summary of the
* error on the boards alphanumeric display.
*
* THIS ROUTIINE IS NOT CALLABLE FROM "C"
*
*/

#define	ROM_ISP_BASE	0xa0100000

	.ent	romExcHandle
romExcHandle:
	.set	noat

	li	sp, ROM_ISP_BASE 	/* sp to known uncached location */
	SW	sp, E_STK_SP-ESTKSIZE(sp) /* save sp in new intstk frame */
	subu	sp, ESTKSIZE		/* make new exc stk frame	*/
	SW	k0, E_STK_K0(sp) 	/* save k0, (exception type)	*/
	SW	AT, E_STK_AT(sp)	/* save asmbler resvd reg	*/
	.set	at
	SW	v0, E_STK_V0(sp)	/* save func return 0, used
					   to hold masked cause		*/
	mfc0	k1, C0_BADVADDR		/* read bad VA reg	*/
	sw	k1, E_STK_BADVADDR(sp)	/* save bad VA on stack	*/
	mfc0	k1, C0_EPC		/* read exception pc	*/
	sw	k1, E_STK_EPC(sp)	/* save EPC on stack	*/
	mfc0	v0, C0_CAUSE		/* read cause register	*/
	sw	v0, E_STK_CAUSE(sp)	/* save cause on stack	*/
	mfc0	k1, C0_SR		/* read status register	*/
	sw	k1, E_STK_SR(sp)	/* save status on stack	*/

	.set	noat
	mflo	AT			/* read entry lo reg	*/
	SW	AT,E_STK_LO(sp)		/* save entry lo reg	*/
	mfhi	AT			/* read entry hi reg	*/
	SW	AT,E_STK_HI(sp)		/* save entry hi reg	*/
	.set	at
	SW	zero, E_STK_ZERO(sp)	/* save zero ?!		*/
	SW	v1,E_STK_V1(sp)		/* save func return 1	*/
	SW	a0,E_STK_A0(sp)		/* save passed param 0	*/
	SW	a1,E_STK_A1(sp)		/* save passed param 1	*/
	SW	a2,E_STK_A2(sp)		/* save passed param 2	*/
	SW	a3,E_STK_A3(sp)		/* save passed param 3	*/
	SW	t0,E_STK_T0(sp)		/* save temp reg 0	*/
	SW	t1,E_STK_T1(sp)		/* save temp reg 1	*/
	SW	t2,E_STK_T2(sp)		/* save temp reg 2	*/
	SW	t3,E_STK_T3(sp)		/* save temp reg 3	*/
	SW	t4,E_STK_T4(sp)		/* save temp reg 4	*/
	SW	t5,E_STK_T5(sp)		/* save temp reg 5	*/
	SW	t6,E_STK_T6(sp)		/* save temp reg 6	*/
	SW	t7,E_STK_T7(sp)		/* save temp reg 7	*/
	SW	t8,E_STK_T8(sp)		/* save temp reg 8	*/
	SW	t9,E_STK_T9(sp)		/* save temp reg 9	*/
	SW	s0,E_STK_S0(sp)		/* save saved reg 0	*/
	SW	s1,E_STK_S1(sp)		/* save saved reg 1	*/
	SW	s2,E_STK_S2(sp)		/* save saved reg 2	*/
	SW	s3,E_STK_S3(sp)		/* save saved reg 3	*/
	SW	s4,E_STK_S4(sp)		/* save saved reg 4	*/
	SW	s5,E_STK_S5(sp)		/* save saved reg 5	*/
	SW	s6,E_STK_S6(sp)		/* save saved reg 6	*/
	SW	s7,E_STK_S7(sp)		/* save saved reg 7	*/
	SW	s8,E_STK_FP(sp)		/* save saved reg 8	*/
	SW	gp,E_STK_GP(sp)		/* save global pointer?	*/
	SW	ra,E_STK_RA(sp)		/* save return address	*/
	
	.end    romExcHandle            /* that's all folks */


#define ROMCYCLE	1180	/* 1.18us cycle */
#define ROMMS	1000000/(ROMCYCLE*2)

/*
 * delay for r microseconds
 */
#define ASMRDELAY(r) \
	.set	noreorder; \
	.set	noat; \
	move	AT,r; \
99:; \
	bne	AT,zero,99b; \
	subu	AT,1; \
	.set	at; \
	.set	reorder

/*
 * delay for n milliseconds
 */
#define DELAY(n)			\
	.set	noat ;			\
	li	AT,(n*ROMMS) ;		\
	ASMRDELAY(AT) ;			\
	.set	at

#define PREDELAY	1000
#define ONDELAY		4000
#define OFFDELAY	400
#define POSTDELAY	1000

#define BLIPDISPLAY(bcrrp,bcrrv)	\
	DELAY(ONDELAY) ; 		\
	DELAY(OFFDELAY)


	/* Data for the display of Exception Info */
exctypes:
#if CPU_VAR==RC32364
	.word	0x200;	.ascii	"tlb "
	.word	0x300;	.ascii	"cach"
	.word	0x380;	.ascii	"gen "
#endif	/* CPU_VAR==RC32364 */
	.word	0;	.ascii	"????"

r_epc:	.ascii	"epc "
r_cr:	.ascii	"cr  "
r_sr:	.ascii	"sr  "
r_badvaddr: .ascii	"badv"
r_ra:	.ascii	"ra  "


