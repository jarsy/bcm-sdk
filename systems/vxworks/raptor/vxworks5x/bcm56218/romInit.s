/* $Id: romInit.s,v 1.5 2011/07/21 16:14:55 yshtil Exp $
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/

/* romInit.s - BCM56218 ROM initialization module */

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

#include "vxWorks.h"
#include "arch/mips/ivMips.h"
#include "arch/mips/asmMips.h"
#include "arch/mips/esfMips.h"
#include "sysLib.h"
#include "config.h"
#include "bcm56218.h"
#include "sbmemc.h"

#define MEMC_BASE      BCM56218_REG_MEMC
#define BCM56218_CAP_REG    0x18000004
#define BCM56218_FLASH_CONTROL_REG  0x1800012c

#if (MIPSEB)
#define BYTE_LANE(n)  ((n)^3)
#else
#define BYTE_LANE(n)  (n)
#endif


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
#define BCM56218_GPIO_BASE  0x18000060

#if 1
#define BDBGINIT            \
	    li	t0, (BCM56218_GPIO_BASE | K1BASE)  ;\
            li  t1, 0xFF                            ;\
            sb  t1, BYTE_LANE(0xc)(t0)               ;\
            li  t1, 0                               ;\
            sb  t1, BYTE_LANE(8)(t0)                 ;\
            nop


#define BPRINT_ASM(msg, ledcode)                    \
	li	t0,(BCM56218_GPIO_BASE | K1BASE)    ;\
	sb	a1,BYTE_LANE(4)(t0); nop



#else
#define BDBGINIT            

#define BPRINT_ASM(msg, ledcode)         \
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

    /* set SR and CAUSE to something sensible */
    li      v0,SR_BEV

    .set noreorder
    .set noat

    mtc0    v0,C0_SR
    mtc0    zero,C0_CAUSE
    mtc0    zero,$18        /* C0_WATCHLO */
    mtc0    zero,$19        /* C0_WATCHHI */


    /***********
     * Initialize the hardware
     ***********/

    li      t0, 2
    mtc0    t0, $16
    nop
    
    .set	reorder
    .set	at
    .set mips32

    BDBGINIT

    BPRINT_ASM(Init_msg, 0x01)

    /* switch from async mode to sync mode if already in async mode */
            li      t0, 0x400000
            mfc0    t1,$22,4
            and     t1, t1, t0
            beqz    t1, done_pll
            nop
	    li	    t0, (BCM56218_CAP_REG | K1BASE)
            lw      t1, 0(t0)
            andi    t0, t1, 0x4
            beqz    t0, done_pll
            nop
            andi    t0, t1, 0x30    /* Read the clk ratio strap values */
            beqz    t0, strap_266
            nop
            beq     t0, 0x30, strap_133
            nop

            /* Strap set to either 6:4 or 9:4 ratio */
            beq     t0, 0x10, strap_300
            nop

strap_200:
            li      t0, 0x016D016D
            b       enable_dll
            nop
strap_300:
            li      t0, 0x012A00A9
            b       enable_dll
            nop
strap_266:
            li      t0, 0x00AA0055
            b       enable_dll
            nop
strap_133:
            li      t0, 0xFFFFFFFF
            b       enable_dll
            nop

enable_dll:
            li      t1, 0xff400010
            li      t2, 0x80000008
            lw      t3, 0(t1)
            or      t3, t3, t2
            sw      t3, 0(t1)

            sw      t0, 0x8(t1)

            mfc0    t2,$22,2
            ori     t2, t2, 0x8
            mtc0    t2,$22,2
            nop
            nop
            nop
            nop

start_hw_dll:
            li      t0, 0x1
            li      t1, 0xff400008  /* DLL reg address          */
            li      t3, 0x0041e021  /* setmask to be 1E         */
            lw      t2, 0(t1)
            or      t2, t2, t3
            sw      t2, 0(t1)       /* enable dll by write 1 to bit[0] */
            li      t0, 0x80        /* enable aggressive hardware mode */
            lw      t2, 0(t1)
            or      t2, t2, t0
            sw      t2, 0(t1)
            sync

DLL_LOCK_CLEAR: 
            lw      t2, 0(t1)        /* check for lock */
            li      t0, 0x2
            and     t2, t2, t0
            beqz    t2, DLL_LOCK_CLEAR
            nop
            lw      t2, 0(t1)        /*  clear sticky bit */
            sw      t2, 0(t1)
     
            li      t3, 0xfffc1fff
            and     t2, t3, t2
            li      t3, 0x00022000   /* set mask to be 10001 */
            or      t2, t3, t2
     
            sw      t2, 0(t1)

            mfc0    t2,$22,2
            ori     t2, t2, 0x8
            mtc0    t2,$22,2
            nop
            nop
            nop
            nop

            li      t2, 0x000000
            mtc0    t2,$22,4
            sync

done_pll:
            nop
    

       /* Adjust flash timing. */
        li	t0, (BCM56218_FLASH_CONTROL_REG | K1BASE )
        li  t1, 0x01010110
        sw  t1, 0(t0)

    BPRINT_ASM(ddrm_init, 0x02)


    /* Initialize DDR/SDR */
    bal board_draminit

    BPRINT_ASM(ddrm_msg, 0x03)
		
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
    andi    a0, s0, BOOT_CLEAR

    /*initialise caches */	
    bal	romCacheReset

    andi    t0, s0, BOOT_CLEAR
    beqz    t0, romWarm

romWarm:

/* Memory Test */
#if 0
    li	a0,0xa0000000
    li	a1,0xa3FFFFF0
    subu	a1,a0
    srl	a1,2

    bal	_mtest
    nop
    BPRINT_ASM(chkp_msg, 0x04)

    /* Flash Read Test */
    bal	_ftest
    nop
#endif

    BPRINT_ASM(bss_msg, 0x05)
    /* zap the BSS */
    la	a0,_edata
    la	a1,_end
    subu    a1,a0
    srl     a1,2
    li      a2,0xfffffff0
    and     a1,a1,a2
    li      a2,0xfffffff0
    and     a0,a0,a2
    move	a2,zero

    bal	_bfillLongs
    nop

6:
    BPRINT_ASM(stak_msg, 0x06)
    /* give us as long as possible before a clock interrupt */ 
    li	t0, 1
    mtc0	t0, C0_COUNT
    mtc0	zero, C0_COMPARE 

    /* set stack to grow down from beginning of data and call init */
    la	gp, _gp             /* set global ptr from compiler */
    la	sp, STACK_ADRS      /* set stack to begin of data */
    subu	sp, 16          /* give me some room */

    BPRINT_ASM(roms_msg, 0x07)

    move    a0, s0          /* push arg = start type */
    RELOC(t0, romStart)

    jal	t0                  /* never returns - starts up kernel  */

    j	ra                  /* just in case */

1:  b	1b

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

    subu    a1,1
    addu    a0,4

    bgeu    a1,1,1b

#endif
    beqz    a1,2f

4: nop
    li	t0,0x80000000
    sw	a0,0(t0)
    BPRINT_ASM(r_Fail, 0x08)
    b   4b
    /* byte write and compare */

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
    b   4b

2:  j	v0

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
#define _mincache(size, maxsize)    \
    .set noat ;                     \
    sltu    AT,size,maxsize;        \
    bnez    AT,9f     ;             \
	move   size,maxsize ;	    \
    .set at ;                       \
9:

#define _align(minaddr, maxaddr, linesize) \
    .set noat;                          \
	subu	AT,linesize,1 ;	        \
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

/*	mtc0	v1,C0_SR */
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

	/* Save return address */
	move	t6,ra

        li      v1, 0 
	li	a0, (MEMC_BASE | K1BASE)

	b	init			# No NVRAM
	li	a2, 0

init:
memc_init:
	bnez	a2,1f		# Already have the parms in t0, t1, t2, t3
	nop

	/* No nvram parms: get configured values (sbmemc.h) */

#ifdef  SDRAM_MODE 
	li	t0,MEMC_SDR_INIT
	li	t1,MEMC_SDR_MODE
	li	t3,MEMC_SDR_NCDL	# If rev0, 2:
	bne	v1,1,1f
	nop
	li	t3,MEMC_SDR1_NCDL	# rev1:
1:
#else
	li	t0,MEMC_DDR_INIT
	li	t1,MEMC_DDR_MODE
	li	t3,MEMC_DDR_NCDL	# If rev0, 2:
	bne	v1,1,1f
	nop
	li	t3,MEMC_DDR1_NCDL	# rev1:
1:
#endif
	andi	a3,t0,MEMC_CONFIG_DDR	# Low bit of init selects ddr or sdr
	beqz	a3,memc_sdr_init
	nop


/*
 * Routines for initializing DDR SDRAM
 */
	
memc_ddr_init:
	beqz	t3,ddr_find_ncdl	/* Do we have ncdl values? (0s) */
	nop
	li	t4,-1			/* or ffs */
	bne	t3,t4,break_ddr_ncdl
	nop

ddr_find_ncdl:

/* Register usage */
#define	pass_count	s0
#define	wrsum		s1
#define	rdsum		s2
#define	gsum		s3
#define	wrlim		s4
#define	rdlim		s5
#define	glim		s6
#define	dll		s7
#define	step		s8
#define	wr		t2
#define	rd		t3
#define	g		t4

	/* Initialize counter & accumulators */
	move	pass_count,zero
	move	wrsum,zero
	move	rdsum,zero
	move	gsum,zero

	/* Initialize with default values */
	li	wr,5
	li	rd,5
	bal	ddr_do_init
	li	g,10

	/* Read dll value */
	lw	dll,MEMC_NCDLCTL(a0)
	andi	dll,dll,0xfe
	srl	dll,dll,1
	beqz	dll,szmem		# If zero, leave the default values
	nop

	move	wrlim,dll		# dll value is lim for wr, rd and g
	move	rdlim,dll
	move	glim,dll

	addi	step,dll,15		# step = (dll + 16 - 1) / 16
	srl	step,step,4

	sub	wr,zero,dll		# Negate dll as initial value
	move	rd,wr
	move	g,wr

	/* Inner loop:	call ddr_do_init to re-initialize and the test mem */
loop:
	bal	ddr_do_init
	nop

	bal	test_mem
	nop

	beqz	v0,nextg
	nop

	/* Memory is ok */

	addi	pass_count,1
	add	wrsum,wrsum,wr
	add	rdsum,rdsum,rd
	add	gsum,gsum,g

	bne	wr,dll,1f
	nop
	sll	wrlim,dll,1
1:
	bne	rd,dll,2f
	nop
	sll	rdlim,dll,1
2:
	bne	g,dll,nextg
	nop
	sll	glim,dll,1

nextg:
	add	g,g,step
	ble	g,glim,loop
	nop
	sub	g,zero,dll
	move	glim,dll

	/* nextrd: */
	add	rd,rd,step
	ble	rd,rdlim,loop
	nop
	sub	rd,zero,dll
	move	rdlim,dll

	/* nextwr: */
	add	wr,wr,step
	ble	wr,wrlim,loop
	nop

	/* All done, calculate average values and program them */
	
	beqz	pass_count,1f
	nop

	div	zero,wrsum,pass_count
	mflo	wr

	div	zero,rdsum,pass_count
	mflo	rd

	div	zero,gsum,pass_count
	mflo	g

	b	ddr_got_ncdl
	nop

	/* No passing values, panic! (use defaults) */
1:
#ifdef SDRAM_MODE
	li	t3,MEMC_SDR_NCDL		# If rev0, 2:
	bne	v1,1,2f
	nop
	li	t3,MEMC_SDR1_NCDL		# rev1:
#else
	li	t3,MEMC_DDR_NCDL		# If rev0, 2:
	bne	v1,1,2f
	nop
	li	t3,MEMC_DDR1_NCDL		# rev1:
#endif
2:

break_ddr_ncdl:
	andi	t4,t3,0xff			# t4:	g
	srl	t2,t3,16			# t2:	wr
	andi	t2,t2,0xff
	srl	t3,t3,8				# t3:	rd
	andi	t3,t3,0xff

ddr_got_ncdl:
	bal	ddr_do_init
	nop

	b	szmem
	nop


	/* Do an init of the memc core for ddr
	 *	a0:	memc core pointer
	 *	t0:	memc config value
	 *	t1:	memc mode value
	 *	t2:	memc wr ncdl value
	 *	t3:	memc rd ncdl value
	 *	t4:	memc g ncdl value
	 *
	 * Uses a1, t7, t8, t9 (here and by calling memc_reset)
	 */
ddr_do_init:

	/* Save return address */
	move	t7,ra
#if 0
	bal	memc_reset
	li	a1,0
#endif
	li	a1,MEMC_CONFIG_INIT
	or	a1,a1,t0
	sw	a1,MEMC_CONFIG(a0)

	li	a1,MEMC_DRAMTIM25_INIT		# Assume CAS latency of 2.5
	andi	t8,t1,0xf0			# Find out the CAS latency
	bne	t8,0x20,1f
	nop
	li	a1,MEMC_DRAMTIM2_INIT		# CAS latency is 2
1:	
	sw	a1,MEMC_DRAMTIM(a0)

	andi	t8,t3,0xff
	sll	a1,t8,8				/* Replicate rd ncdl 4 times */
	or	a1,a1,t8
	sll	t8,a1,16
	or	t8,t8,a1
	li	a1,MEMC_RDNCDLCOR_INIT
	or	a1,a1,t8
	sw	a1,MEMC_RDNCDLCOR(a0)

	li	a1,MEMC_WRNCDLCOR_INIT		/* If rev0, 2: */
	bne	v1,1,1f
	nop
	li	a1,MEMC_1_WRNCDLCOR_INIT	/* rev1 */
1:
	andi	t8,t2,0xff
	or	a1,a1,t8
	sw	a1,MEMC_WRNCDLCOR(a0)

	li	a1,MEMC_DQSGATENCDL_INIT
	andi	t8,t4,0xff
	or	a1,a1,t8
	sw	a1,MEMC_DQSGATENCDL(a0)

	li	a1,MEMC_MISCDLYCTL_INIT		/* If rev0, 2: */
	bne	v1,1,2f
	nop
	li	a1,MEMC_1_MISCDLYCTL_INIT	/* rev1 */
2:
	sw	a1,MEMC_MISCDLYCTL(a0)

	li	a1,MEMC_NCDLCTL_INIT
	sw	a1,MEMC_NCDLCTL(a0)

	li	a1,MEMC_CONTROL_INIT0
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_CONTROL_INIT1
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_MODEBUF_INIT0
	sw	a1,MEMC_MODEBUF(a0)

	li	a1,MEMC_CONTROL_INIT2
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_MODEBUF_INIT1
	or	a1,a1,t1
	sw	a1,MEMC_MODEBUF(a0)

	li	a1,MEMC_CONTROL_INIT3
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_CONTROL_INIT4
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_CONTROL_INIT5
	sw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_CONTROL_INIT5
	sw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_REFRESH_INIT
	sw	a1,MEMC_REFRESH(a0)
	
	li	a1,MEMC_MODEBUF_INIT2
	or	a1,a1,t1
	sw	a1,MEMC_MODEBUF(a0)

	li	a1,MEMC_CONTROL_INIT6
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_CONTROL_INIT7
	sw	a1,MEMC_CONTROL(a0)

	jr	t7
	nop


/*
 *  Routines for initializing SDR SDRAM
 */
	
memc_sdr_init:
	beqz	t3,sdr_find_ncdl	/* Do we have ncdl values? */
	nop

	li	t4,-1
	bne	t3,t4,break_sdr_ncdl
	nop

sdr_find_ncdl:

/* Register usage */
#define	pass_count	s0
#define	clkdsum		s1
#define	pass_countmax	s2
#define	strmmax		s3
#define	strdmax		s4
#define	clkdmax		s5
#define	clkdlim		s6
#define	strm		t2
#define	strd		t3
#define	clkd		t4

#define	STRMLIM		4
#define	STRDLIM		16
#define	CLKDLIM		128
#define	CLKDLIM_IC	256

	/* Initialize counter & saved values */
	move	pass_countmax,zero
	move	strmmax,zero
	move	strdmax,zero
	li	clkdlim,CLKDLIM

	and	strm,t0,0x2000		/* Test for internal clock (Using strm as a temp) */
	beqz	strm,strmloop
	nop

	li	clkdlim,CLKDLIM_IC

	move	strm,zero		/* strm loop */
strmloop:
	move	strd,zero
strdloop:
	move	pass_count,zero
	move	clkdsum,zero
	move	clkd,zero

	/* Inner loop:	call sdr_do_init to re-initialize and the test mem */
clkdloop:
	bal	sdr_do_init
	nop

	bal	test_mem
	nop

	beqz	v0,failclkd
	nop

	/* Memory is ok */

	addi	pass_count,1
	add	clkdsum,clkdsum,clkd
	b	nextclkd
	nop

failclkd:
	bnez	pass_count,clkdout	/* End of passing range, leave clkd loop */
	nop

nextclkd:
	addi	clkd,clkd,1
	blt	clkd,clkdlim,clkdloop
	nop

clkdout:
	/* If no passing values, skip to next strm */
	beqz	pass_count,nextstrm
	nop

	/* If this is a new max, Save the values */
	ble	pass_count,pass_countmax,nextstrd
	nop

	move	pass_countmax,pass_count
	div	zero,clkdsum,pass_count
	mflo	clkdmax
	move	strdmax,strd
	move	strmmax,strm

nextstrd:
	addi	strd,strd,1
	blt	strd,STRDLIM,strdloop
	nop

nextstrm:
	addi	strm,strm,1
	blt	strm,STRMLIM,strmloop
	nop

	/* All done, program the new ncdl values */
	
	beqz	pass_countmax,1f
	nop

	move	clkd,clkdmax
	move	strd,strdmax
	move	strm,strmmax
	b	sdr_got_ncdl
	nop

	/* No passing values, panic! (use defaults) */
1:
#ifdef SDRAM_MODE
	li	t3,MEMC_SDR_NCDL	/* If rev0, 2: */
	bne	v1,1,2f
	nop
	li	t3,MEMC_SDR1_NCDL	/* rev1: */
#else
	li	t3,MEMC_DDR_NCDL		/* If rev0, 2: */
	bne	v1,1,2f
	nop
	li	t3,MEMC_DDR1_NCDL		/* rev1: */
#endif
2:

break_sdr_ncdl:
	andi	t4,t3,0xff		/* t4:	cd */
	srl	t2,t3,16		/* t2:	sm */
	andi	t2,t2,3			/*	sm is 2 bits only*/
	srl	t3,t3,8			/* t3:	sd               */
	andi	t3,t3,0xf		/*	sd is 4 bits     */
                                                                 
sdr_got_ncdl:
	bal	sdr_do_init
	nop

	b	szmem
	nop

	
	/* Do an init of the memc core for sdr
	 *	a0:	memc core pointer
	 *	t0:	memc config value
	 *	t1:	memc mode value
	 *	t2:	memc strobe mode ncdl value
	 *	t3:	memc strobe delay ncdl value
	 *	t4:	memc clock delay ncdl value
	 *
	 * Uses a1, t7, t8, t9 (here and by calling memc_reset)
	 */
sdr_do_init:

	/* Save return address */
	move	t7,ra
#if 0
	bal	memc_reset
	li	a1,0x40
#endif
	/* Initialize SDRAM */
	li	a1,MEMC_SD_CONFIG_INIT
	or	a1,a1,t0
	sw	a1,MEMC_CONFIG(a0)

	li	a1,MEMC_SD_DRAMTIM3_INIT	/* Assume CAS latency of 3 */
	andi	t8,t1,0xf0			/* Find out the CAS latency*/
	bne	t8,0x20,1f
	nop
	li	a1,MEMC_SD_DRAMTIM2_INIT	/* CAS latency is 2 */
1:	
	sw	a1,MEMC_DRAMTIM(a0)

	andi	t8,t4,0xff
	ble	t8,MEMC_CD_THRESHOLD,1f		/* if (cd <= 128) rd = cd */
	nop

	li	t8,MEMC_CD_THRESHOLD		/* else rd = 128 */

1:						/* t8 is now rd */
	sll	a1,t8,8				/*  .. replicate it 4 times */
	or	a1,a1,t8
	sll	t8,a1,16
	or	t8,t8,a1
	li	a1,MEMC_SD_RDNCDLCOR_INIT
	or	a1,a1,t8
	sw	a1,MEMC_RDNCDLCOR(a0)

	li	a1,MEMC_SD1_WRNCDLCOR_INIT	/* rev1 */
	beq	v1,1,1f
	nop
	li	a1,MEMC_SD_WRNCDLCOR_INIT	/* rev0, 2 */
1:
	li	t8,0
	ble	t4,MEMC_CD_THRESHOLD,2f		/* if (cd <= 128) wr = 0 */
	nop
	
	andi	t8,t4,0xff			/* else wr = cd - 128 */
	sub	t8,t8,MEMC_CD_THRESHOLD
	andi	t8,t8,0xff

2:						/* t8 is now wr, a0 is extra bits */
	or	a1,a1,t8
	sw	a1,MEMC_WRNCDLCOR(a0)

	andi	t8,t2,3
	sll	a1,t8,28
	andi	t8,t3,0xf
	sll	t8,t8,24
	or	t8,t8,a1
	li	a1,MEMC_SD_MISCDLYCTL_INIT
	bne	v1,1,3f				/* If rev0, 2: */
	nop
	li	a1,MEMC_SD1_MISCDLYCTL_INIT	/* rev1: */
3:
	or	a1,a1,t8
	sw	a1,MEMC_MISCDLYCTL(a0)

	li	a1,MEMC_SD_CONTROL_INIT0
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_SD_CONTROL_INIT1
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_SD_CONTROL_INIT2
	sw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_SD_CONTROL_INIT2
	sw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_SD_CONTROL_INIT2
	sw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)
	lw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_SD_REFRESH_INIT
	sw	a1,MEMC_REFRESH(a0)

	li	a1,MEMC_SD_MODEBUF_INIT
	or	a1,a1,t1
	sw	a1,MEMC_MODEBUF(a0)

	li	a1,MEMC_SD_CONTROL_INIT3
	sw	a1,MEMC_CONTROL(a0)

	li	a1,MEMC_SD_CONTROL_INIT4
	sw	a1,MEMC_CONTROL(a0)

	li	t8,50
1:	

	jr	t7
	nop



/*
 *  Common exit code and subroutines shared by SDR and DDR initialization.
 */
		
	/* Determine memory size and return
	 *
	 * Somewhat simplistic, assumes size is a power of 2 and looks
	 * for aliases of location 0.
	 */
szmem:
	li	t0, K1BASE
	li	t2,0xaa55beef
	sw	t2,0(t0)
	li	v0,4		/* Assume minimum of 4MB */

1:
	sll	t0,v0,20
	or	t0, K1BASE
	lw	t1,0(t0)
	beq	t1,t2,mdone
	nop

	sll	v0,v0,1
	bne	v0,128,1b	/* Fully populated at 128MB, no alias */
	nop

mdone:
	jr	t6	
	nop


	/*
	 * Test memory
	 *
	 * Uses arg in t2(wr/sd), t3(rd/sm) and t4(g/clkd)
	 * Returns success (1) or failure (0) in v0
	 * Uses a1, a2, a3 & t5
	 */
test_mem:
	/* Use t4 to generate a semi-random address in the second KB */
	li	a1,0xa0000000
	addi	a2,t4,255
	sll	a2,a2,2
	add	a1,a1,a2

	/* First set: 0 & its negation */
	li	a2,0
	sw	a2,0(a1)
	not	a3,a2
	sw	a3,4(a1)
	nop
	lw	t5,0(a1)
	bne	a2,t5,bad_mem
	nop
	lw	t5,4(a1)
	bne	a3,t5,bad_mem
	nop

	/* Second set: 0xaaaaaaaa & its negation */
	li	a2,0xaaaaaaaa
	sw	a2,0(a1)
	not	a3,a2
	sw	a3,4(a1)
	nop
	lw	t5,0(a1)
	bne	a2,t5,bad_mem
	nop
	lw	t5,4(a1)
	bne	a3,t5,bad_mem
	nop

	/* Third set: 0x12345678 & its negation */
	li	a2,0x12345678
	sw	a2,0(a1)
	not	a3,a2
	sw	a3,4(a1)
	nop
	lw	t5,0(a1)
	bne	a2,t5,bad_mem
	nop
	lw	t5,4(a1)
	bne	a3,t5,bad_mem
	nop

	/* Fourth set: the ncdl & its negation */
	sll	a2,t2,8
	or	a2,t3
	sll	a2,a2,8
	or	a2,t4
	sw	a2,0(a1)
	not	a3,a2
	sw	a3,4(a1)
	nop
	lw	t5,0(a1)
	bne	a2,t5,bad_mem
	nop
	lw	t5,4(a1)
	bne	a3,t5,bad_mem
	nop

	/* Fifth set: the CPU count register & its negation */
	mfc0	a2,$9
	sw	a2,0(a1)
	not	a3,a2
	sw	a3,4(a1)
	nop
	lw	t5,0(a1)
	bne	a2,t5,bad_mem
	nop
	lw	t5,4(a1)
	bne	a3,t5,bad_mem
	nop

	jr	ra
	li	v0,1

bad_mem:
	jr	ra
	li	v0,0

	
	.align 6
#if 0
memc_reset:

	/* Save return address */
	move	t9,ra
        
	/* run uncached */
	bal     kseg1_switch
	nop                                

        li      t5, 0xb8000114
        li      t8, 0x00
        sw      t8, 0(t5)
        nop
        nop
        nop
        nop
        nop
        li      t8, 0x01
        sw      t8, 0(t5)
        nop
        nop
        nop

	jr	t9
	nop
#endif
        
kseg1_switch:
	and     ra, ra, 0x1fffffff
	or      ra, ra, K1BASE
	jr      ra
	nop 
        
	.set	reorder
	.end	board_draminit

/* end here */

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
#if 0
	.word	0x200;	.ascii	"tlb "
	.word	0x300;	.ascii	"cach"
	.word	0x380;	.ascii	"gen "
#endif
#endif	/* CPU_VAR==RC32364 */
	.word	0;	.ascii	"????"

r_epc:	.ascii	"epc "
r_cr:	.ascii	"cr  "
r_sr:	.ascii	"sr  "
r_badvaddr: .ascii	"badv"
r_ra:	.ascii	"ra  "


