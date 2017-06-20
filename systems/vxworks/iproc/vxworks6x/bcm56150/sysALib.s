/* sysALib.s - Katana2 system-dependent routines */

/*
 * Copyright (c) 2013 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */
 
/*
modification history
--------------------
01a,23sep13,dnb  created from arm_a9_ctx revision 01e

*/

/*
DESCRIPTION
This module contains system-dependent routines written in assembly
language. It contains the entry code, sysInit(), for VxWorks images
that start running from RAM, such as 'vxWorks'.  These images are
loaded into memory by some external program (e.g., a boot ROM) and then
started.  The routine sysInit() must come first in the text segment.
Its job is to perform the minimal setup needed to call the generic C
routine usrInit().

sysInit() masks interrupts in the processor and the interrupt
controller and sets the initial stack pointer.  Other hardware and
device initialisation is performed later in the sysHwInit routine in
sysLib.c.

NOTE
The routines in this module don't use the "C" frame pointer %r11@ ! or
establish a stack frame.
*/

#define _ASMLANGUAGE
#include <vxWorks.h>
#include <vsbConfig.h>
#include <asm.h>
#include <regs.h>
#include <arch/arm/arm.h>
#include <arch/arm/mmuArmLib.h>
#include <sysLib.h>
#include "config.h"

    /* internals */

    .globl  FUNC(sysInit)           /* start of system code */


    /* externals */

    .extern FUNC(usrInit)        /* system initialization routine */

    .text
    .balign 4

/*******************************************************************************
*
* sysInit - start after boot
*
* This routine is the system start-up entry point for VxWorks in RAM, the
* first code executed after booting.  It disables interrupts, sets up
* the stack, and jumps to the C routine usrInit() in usrConfig.c.
*
* The initial stack is set to grow down from the address of sysInit().  This
* stack is used only by usrInit() and is never used again.  Memory for the
* stack must be accounted for when determining the system load address.
*
* NOTE: This routine should not be called by the user.
*
* RETURNS: N/A
*
* sysInit ()              /@ THIS IS NOT A CALLABLE ROUTINE @/
*
*/

_ARM_FUNCTION(sysInit)


#ifdef ARMBE8
      SETEND BE
#endif

    /* 
     * SPSR does not have pre-defined reset value. 
     * Here correct endianess (BE bit) in SPSR
     */

    MRS    r0, spsr
#if (_BYTE_ORDER == _LITTLE_ENDIAN)
    BIC    r0, r0, #(0x1 << 9)    /* Little Endian*/
#else
    ORR    r0, r0, #(0x1 << 9)    /* Big Endian */
#endif
    MSR    spsr_x, r0

    /*
     * Set processor and MMU to known state as follows (we may have not
     * been entered from a reset). We must do this before setting the CPU
     * mode as we must set PROG32/DATA32.
     *
     * MMU Control Register layout.
     *
     * bit
     *  0 M 0 MMU disabled
     *  1 A 0 Address alignment fault disabled, initially
     *  2 C 0 Data cache disabled
     *  3 W 0 Write Buffer disabled
     *  4 P 1 PROG32
     *  5 D 1 DATA32
     *  6 L 1 Should Be One (Late abort on earlier CPUs)
     *  7 B ? Endianness (1 => big)
     *  8 S 0 System bit to zero } Modifies MMU protections, not really
     *  9 R 1 ROM bit to one     } relevant until MMU switched on later.
     * 10 F 0 Should Be Zero
     * 11 Z 1 Branch prediction enabled
     * 12 I 0 Instruction cache control
     */

    /* Flush the entire BTAC before enabling the Z bit */

    MOV    r1, #0
    MCR    CP_MMU, 0, r1, c7, c5, 6

    /* Setup MMU Control Register */    

    LDR    r1, =MMU_INIT_VALUE           /* Defined in mmuCortexA8Lib.h */
    MCR    CP_MMU, 0, r1, c1, c0, 0      /* Write to MMU CR */

    /* perform a Data Synchronization Barrier */

    DSB


    /* We only get here with CPU 0 */    

    MOV    r1, #0
    LDR    r2, =IPROC_SCU_CONTROL
    STR    r1, [r2, #0xc]                /* invalidate all registers */

    /*
     * Set Process ID Register to zero, this effectively disables
     * the process ID remapping feature.
     */

    MOV    r1, #0
    MCR    CP_MMU, 0, r1, c13, c0, 0

    /* Set Context ID Register to zero, including Address Space ID */

    MCR    CP_MMU, 0, r1, c13, c0, 1

    /* disable interrupts in CPU and switch to SVC32 mode */

    MRS    r1, cpsr
    BIC    r1, r1, #MASK_MODE
    ORR    r1, r1, #MODE_SVC32 | I_BIT | F_BIT
    MSR    cpsr, r1
    
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

    /* set the interrupt vector base address to 0 */
    MOV    r1, #0
    MCR    p15, 0, r1, c12, c0, 0
	
    /* set initial stack pointer so stack grows down from start of code */

    ADR    sp, FUNC(sysInit)        /* initialize stack pointer */
    MOV    fp, #0                   /* initialize frame pointer */

    /* Make sure Boot type is set correctly. */

    MOV    r1,#BOOT_NORMAL
    CMP    r1,r0
    BEQ    L$_Good_Boot

    MOV    r1,#BOOT_NO_AUTOBOOT
    CMP    r1,r0
    BEQ    L$_Good_Boot

    MOV    r1,#BOOT_CLEAR
    CMP    r1,r0
    BEQ    L$_Good_Boot

    MOV    r1,#BOOT_QUICK_AUTOBOOT
    CMP    r1,r0
    BEQ    L$_Good_Boot

    MOV    r0, #BOOT_NORMAL /* default startType */

L$_Good_Boot:

	/* now call usrInit (startType) */

#if (ARM_THUMB)
    LDR    r12, L$_usrInit
    BX     r12
#else
    B      FUNC(usrInit)
#endif /* (ARM_THUMB) */
