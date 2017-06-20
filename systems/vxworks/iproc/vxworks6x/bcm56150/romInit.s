/* romInit.s - Hurricane 2  ROM initialization module */

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
#include <arch/arm/cacheArmArch7.h>
#include <arch/arm/mmuCortexA8Lib.h>

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

    .ascii   "nCopyright 2010 Wind River Systems, Inc."
    .balign 4

start:
    MOV  r12, r0

    /*
     * For cold boot, we need perform "Flush + Invalidate D-caches", as vxWorks
     * bootloader always boot from UBoot.
     */

    _CORTEX_AR_ENTIRE_DATA_CACHE_OP(c14)

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

    LDR    r2, =IPROC_GICCPU_CONTROL
    MOV    r1, #0
    STR    r1, [r2, #0]       

    /* disable ints for distributor */

    LDR    r2, =(IPROC_GICDIST_REG_BASE)
    STR    r1, [r2, #0]

    /* disable timers */

    LDR    r2, =IPROC_PERIPH_GLB_TIM_REG_BASE
    STR    r1, [r2, #0]
    
    LDR     sp, L$_STACK_ADDR
    MOV     fp, #0                  /* zero frame pointer */


    LDR     pc, L$_rStrtInRom /* CPU0 will directly run this bootloader */

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

