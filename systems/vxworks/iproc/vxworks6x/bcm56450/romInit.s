/* romInit.s - ARM Versatile Express A9x4 board ROM initialization module */

/*
 * Copyright (c) 2010 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */
 
/*
modification history
--------------------
01d,20aug10,my_  fix warm boot issue in BE8 mode (WIND00229371)
01c,05aug10,j_b  branch prediction now enabled in MMU_INIT_VALUE (CQID: 222896)
01b,20jul10,my_  add be8 support
01a,17may10,my_  created from arm_pbxa9 rev 01a
*/

/*
DESCRIPTION
This module contains the entry code for vxWorks images that start
running from ROM, such as 'bootrom' and 'vxWorks_rom'.  The entry
point, romInit(), is the first code executed on power-up.  It performs
the minimal setup needed to call the generic C routine romStart() with
parameter BOOT_COLD.

romInit() masks interrupts in the processor and the interrupt
controller and sets the initial stack pointer (to STACK_ADRS which is
defined in configAll.h).  Other hardware and device initialisation is
performed later in the sysHwInit routine in sysLib.c.

The routine sysToMonitor() jumps to a location after the beginning of
romInit, (defined by ROM_WARM_ADRS) to perform a "warm boot".  This
entry point allows a parameter to be passed to romStart().

The routines in this module don't use the "C" frame pointer %r11@ ! or
establish a stack frame.
*/

#define _ASMLANGUAGE
#include <vxWorks.h>
#include <sysLib.h>
#include <asm.h>
#include <regs.h>
#include <config.h>
#include <arch/arm/mmuArmLib.h>

    /* macros */

    #define CACHE_DC_ENABLE (0x1 << 2)
    #define CACHE_IC_ENABLE (0x1 << 12)

    /* internals */

    .globl  FUNC(romInit)     /* start of system code */
    .globl  VAR(sdata)        /* start of data */
    .globl  _sdata
    .globl  FUNC(bfillLongs)
    .globl  FUNC(bspfilllong)

    /* externals */

    .extern FUNC(romStart)    /* system initialization routine */
    .extern FUNC(bfill)       /* architecture optimized fill */

    .data
_sdata:
VAR_LABEL(sdata)
    .asciz    "start of data"
    .balign    4

    .text
    .balign 4

/*******************************************************************************
*
* romInit - entry point for VxWorks in ROM
*

* romInit
*     (
*     int startType    /@ only used by 2nd entry point @/
*     )

* INTERNAL
* sysToMonitor examines the ROM for the first instruction and the string
* "Copy" in the third word so if this changes, sysToMonitor must be updated.
*/

_ARM_FUNCTION(romInit)
_romInit:
   
cold:
    MOV  r0, #BOOT_COLD

#ifdef ARMBE8
    SETEND BE
#endif

warm:
    B    start

    /* copyright notice appears at beginning of ROM (in TEXT segment) */

    .ascii   "\nCopyright 2010 Wind River Systems, Inc."
    .balign 4

start:

    MOV  r12, r0

    /*
     * For cold boot, we need perform "Flush + Invalidate D-caches", as vxWorks
     * bootloader always boot from UBoot.
     */

    _CORTEX_AR_ENTIRE_DATA_CACHE_OP (c14)

    /* disable L1 D-caches */

    MRC     p15, 0, r1, c1, c0, 0       /* Read control register into r1 */
    BIC     r1, r1, # CACHE_DC_ENABLE   /* Clear L1 D-cache enable */
    MCR     p15, 0, r1, c1, c0, 0       /* Write control register */

    /* disable and clear I-cache */

    MRC     p15 , 0, r2, c1, c0, 0      /* Read control register */
    BIC     r2, r2, # CACHE_IC_ENABLE   /* Disable I-cache */
    MCR     p15 , 0, r2, c1, c0, 0      /* Write control register */

    /* Flush the entire BTAC before enabling the Z bit */

    MOV     r1, #0
    MCR     CP_MMU, 0, r1, c7, c5, 6

    LDR     r1, =MMU_INIT_VALUE         /* Defined in mmuCortexA8Lib.h */
    MCR     CP_MMU, 0, r1, c1, c0, 0    /* Write to MMU CR */

    /* perform a Data Synchronization Barrier */

    DSB

    /* detach SMP system */

    MRC     p15, 0, r1, c1, c0, 1       /* Read aux control register */
    BIC     r1,  r1, #(0x1 << 6)        /* Clear SMP bit in aux control register */
    MCR     p15, 0, r1, c1, c0, 1       /* Write aux control register */

    MOV     r1, #0                      /* data SBZ */
    MRC     p15, 0, r1, c0, c0, 5       /* read multiprocessor ID register */
    ANDS    r1, r1, #0x0f
    BNE     cpuWaitForSig

    MOV     r0, r12                     /* restore startType */    
    
    /* disable interrupts in CPU and switch to SVC32 mode */

    MRS     r1, cpsr
    BIC     r1, r1, #MASK_MODE
    ORR     r1, r1, #MODE_SVC32 | I_BIT | F_BIT
    MSR     cpsr, r1

    /*
     * Set Process ID Register to zero, this effectively disables
     * the process ID remapping feature.
     */

    MOV     r1, #0
    MCR     CP_MMU, 0, r1, c13, c0, 0

    /* Set Context ID Register to zero, including Address Space ID */

    MCR     CP_MMU, 0, r1, c13, c0, 1

    MOV     r1, #0xFFFFFFFF         /* Set All 16 domains to mgr access*/
    MCR     p15, 0, r1, c3, c0, 0   /* Set Domain Permissions */

    /* disable individual interrupts in the interrupt controller */    

    /* disable ints to CPU */

    LDR     r2, =PBXA9_GIC_CPU_CONTROL
    MOV     r1, #0
    STR     r1, [r2, #0]       

    /* disable ints for distributor */

    LDR     r2, =(PBXA9_GIC1_BASE + PBXA9_GIC_DIST_CONTROL)
    STR     r1, [r2, #0]

    /* disable Timers */

    LDR     r2, =AMBA_TIMER0_BASE             /* Timer 1 */
    STR     r1, [r2, #8]

    LDR     r2, =AMBA_TIMER1_BASE             /* Timer 2 */
    STR     r1, [r2, #8]

    LDR     r2, =AMBA_TIMER2_BASE             /* Timer 3 */
    STR     r1, [r2, #8]

    LDR     r2, =AMBA_TIMER3_BASE             /* Timer 4 */
    STR     r1, [r2, #8]
    
    LDR     sp, L$_STACK_ADDR
    MOV     fp, #0                  /* zero frame pointer */

    LDR     pc, L$_rStrtInRom /* CPU0 will directly run this bootloader */

cpuWaitForSig: /* non-CPU0 will be here to wait for the awake interrupt */
    LDR     r1, =PBXA9_GIC_CPU_CONTROL
    MOV     r2, #1
    STR     r2, [r1, #0]

    LDR     r1, =PBXA9_SR_FLAGSCLR
    LDR     r2, =0xffffffff
    STR     r2, [r1, #0]
    
    DSB  /* Data Synch Barrier */        

    WFI /* wait for interrupt */

    /* Read flag register to see if address to jump too */
        
jumpaddrget:
    LDR     r1, =PBXA9_SR_FLAGS
    LDR     r2, [r1, #0]
    CMP     r2, #0
    BXNE    r2
    
    B   jumpaddrget

buf  .req    r0
n_f  .req    r1
ch   .req    r2
tmp1 .req    r3
tmp3 .req    r12

_ARM_FUNCTION_CALLED_FROM_C(bspfilllong)

/*******************************************************************************
*
* bspfilllong - fill a buffer with a specified character
*
* This routine fills buffer as fast as possible.
*
* void bspfilllong
*    (
*    char * buf,    /@ r0, pointer to buffer @/
*    int    nums,   /@ r1, number of word to fill @/
*    int    ch      /@ r2, char with which to fill buffer @/
*    ) 
*
* RETURNS: N/A
*/

    stmfd    sp!, {lr}        /* Preserve return address */

    /* convert word number to byte number */
    
    add     r1, r1, r1
    add     r1, r1, r1

    subs    n_f, n_f, #4        /* need at least 4 bytes */
    bmi     TrailingBytes        /* < 4 bytes to go */

    /*
     * word align the dst - first find out how many bytes
     * must be stored to do this.
     */

    ands   tmp3, buf, #3        /* eq means aligned! */
    bne    AlignDst

    /*
     * here when destination is word-aligned,
     * number of bytes to transfer is (n+4), n is >= 0.
     */
DstAligned:
    and    ch, ch, #0xff        /* pad src */
    orr    ch, ch, ch, ASL #8
    orr    ch, ch, ch, ASL #16
    mov    tmp1, ch
    mov    tmp3, ch
    mov    lr, ch

    subs    n_f, n_f, #12-4        /* 12 bytes or more? */
    blt    TrailingWords

    subs    n_f, n_f, #32-12        /* n+32 to go. */
    blt    1f

0:
    stmia    buf!, {ch, tmp1, tmp3, lr}
    stmia    buf!, {ch, tmp1, tmp3, lr}
    subs    n_f, n_f, #32
    bge    0b

    /* see if we can handle another 8 */

    cmn    n_f, #16
    stmgeia buf!, {ch, tmp1, tmp3, lr}
    subge    n_f, n_f, #16

    /* note that we still have (n+32) bytes to go, and this is <16. */

1:
    /* Here when there are fewer than 16 bytes to go. */

    adds    n_f, n_f, #32-12           /* (n-12) to go */

2:
    /* Ok - do three words at a time. */

    stmgeia buf!, {tmp1, tmp3, lr}
    subges    n_f, n_f, #12
    bge    2b

TrailingWords:
    /* (n-12) bytes to go - 0, 1 or 2 words.  Check which. */

    adds    n_f, n_f, #12-4        /* (n-4) to go */
    blt    TrailingBytes        /* < 4 bytes to go */
    subs    n_f, n_f, #4
    strlt    ch, [buf], #4
    stmgeia buf!, {ch, tmp1}
    subge    n_f, n_f, #4

TrailingBytes:
    /* Here with less than 4 bytes to go */

    adds    n_f, n_f, #4

    ldmeqfd    sp!, {pc}        /* 0 bytes, RETURN */

    cmp    n_f, #2            /* 1, 2 or 3 bytes */
    strb    ch, [buf], #1        /* 1 */
    strgeb    ch, [buf], #1        /* 2 */
    strgtb    ch, [buf], #1        /* 3 */

    ldmfd    sp!, {pc}        /* RETURN */

/*
 * word align dst - tmp3 contains current destination
 * alignment.  We can store at least 4 bytes here.
 */
AlignDst:
    rsb    tmp3, tmp3, #4        /* 1-3 bytes to go */
    cmp    tmp3, #2
    strb    ch, [buf], #1        /* 1 */
    strgeb    ch, [buf], #1        /* 2 */
    strgtb    ch, [buf], #1        /* 3 */

    subs    n_f, n_f, tmp3        /* check number to go */
    blt    TrailingBytes        /* less than 4 bytes */
    b    DstAligned

/******************************************************************************/

/*
 * PC-relative-addressable pointers - LDR Rn,=sym is broken
 * note "_" after "$" to stop preprocessor performing substitution
 */

    .balign    4

L$_rStrtInRom:
    .long    ROM_TEXT_ADRS + FUNC(romStart) - FUNC(romInit)

L$_STACK_ADDR:
    .long    STACK_ADRS

L$_bfill:
    .long   ROM_TEXT_ADRS + FUNC(bfill) - FUNC(romInit)

