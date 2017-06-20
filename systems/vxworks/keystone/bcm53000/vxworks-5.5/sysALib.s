/* sysALib.s system-dependent assembly routines */

/* Copyright (c) 2000-2006 Wind River Systems, Inc. */

/* $Id: sysALib.s,v 1.5 2011/07/21 16:14:25 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */
	


/*
DESCRIPTION
This module contains system-dependent routines written in assembly
language.

This module must be the first specified in the 'ld' command used to
build the system.  The sysInit() routine is the system start-up code.
*/

#define _ASMLANGUAGE
#include <vxWorks.h>
#include <sysLib.h>
#include "config.h"

#include <asm.h>
#include "mipsR2Hazards.h"
	
#include <esf.h>
#include "archMips66.h"
	
	/* defines */

	/* internals */

	.globl  sysWait			/* wait for keyboard ready */
	.globl	sysInit			/* start of system code */
	.globl	sysGpInit		/* initialize global pointer */
	.globl  sysWbFlush		/* flush write buffers */
	.globl  sysFpaAck		/* flush write buffers */
	.globl	mipsTlbSizeGet
	.globl	_mipsTlbSizeGet
	.globl	mipsTlbClear
	.globl  sysTimerIntClr
	.globl	sysConfigGet
	.globl	sysConfig1Get
	.globl  sysPciBusErrDisable
	.globl  sysPciBusErrEnable

    .globl  vxSpGet
    .globl  vxPcGet

#if defined _WRS_MIPS_5KF
	/* DID bit in CONFIG register */
#define CFG_DID			0x00020000	/* Dual Issue Disable */
#endif
	
	/* externals */

	.extern	usrInit		/* system initialization routine */
	.extern mipsTlbClear	/* clear all TLB entries */

	.text
	
/*******************************************************************************
*
* sysInit - start after boot
*
* This routine is the system start-up entry point for VxWorks in RAM, the
* first code executed after booting.  It disables interrupts, sets up the
* stack, and jumps to the C routine usrInit() in usrConfig.c.
*
* The initial stack is set to grow down from the address of sysInit().  This
* stack is used only by usrInit() and is never used again.  Memory for the
* stack must be accounted for when determining the system load address.
*
* The startType is sent by the bootrom in a0.  This value is saved off and 
* forwarded to usrInit.
*
* NOTE: This routine should not be called by the user.
*
* RETURNS: N/A
*
* sysInit (int startType)	/@ THIS IS NOT A CALLABLE ROUTINE @/
*
*/
#ifndef BUILD_VXBOOT
.extern fw_arg0
.extern fw_arg1
.extern fw_arg2
.extern fw_arg3
#endif

	.ent	sysInit
sysInit:
#ifndef BUILD_VXBOOT
        /* Store parameters (a0~a3) passed by boot loader to fw_arg0~3 */
        la  k0, fw_arg0
        sw  a0, 0(k0)
        la  k0, fw_arg1
        sw  a1, 0(k0)
        la  k0, fw_arg2
        sw  a2, 0(k0)
        la  k0, fw_arg3
        sw  a3, 0(k0)
#endif

	/* disable all interrupts */
	li	t0, SR_CU0
	mtc0	t0, C0_SR

	mtc0	zero, C0_CAUSE		/* clear software interrupts */

	/* clear WIRED register */

	mtc0	zero, C0_WIRED
	HAZARD_CP_WRITE

    /* save sysStartType */
	move	s0, a0
	
	/* clear any TLB setup done by bootrom */
        jal     mipsTlbClear 


	/* set stack to grow down from code, leave room for four parameters */
	la	sp, (sysInit-(4*_RTypeSize))
	
	/* give us as long as possible before a clock interrupt */
	li	v0,1
	mtc0	v0,C0_COUNT
	mtc0	zero,C0_COMPARE

	la	gp,_gp			/* set global ptr from cmplr */

	move	a0, s0 		/* restore start type */

	jal	usrInit			/* never returns - starts up kernel */
	li	ra, R_VEC		/* load prom reset address */
	j	ra			/* just in case */
	.end	sysInit
	
                        
                        
/*************************************************************************
*
* mipsTlbSizeGet - determine size of TLB
*
* For compliant  MIPS32/MIPS64 processors mipsTlbSizeGet reads the CONFIG1
* register to determine the size of the TLB.	
*
* For non-compliant processors, a hueuristic algorithm is used. 
* The algorithm uses the fact that the C0_RAND register is 
* a random index into the TLB, and that C0_RAND may not go below
* C0_WIRED. The C0_RAND register is read repeatedly in quick succession, 
* which produces a set of values that are tightly related 
* (e.g., 7, 9, 11, 13, ...). The largest of these values is determined, 
* and written to C0_WIRED. Multiple passes through this loop are maintained, 
* with successfuly larger maximum values saved and written to C0_WIRED.  
* When no change is observed in C0_RAND, the maximum value for C0_RAND 
* (and hence the maximum index into the TLB) has been obtained.
*
* ASSUMPTIONS
*
* When the hueuristic algorithm is used, it is assumed that the TLB
* size is a multiple of 8. Hence, the max C0_RAND value determined by the 
* hueuristic algorithm is rounded up. This should not be strictly necessary. 
* It is done to account for the possibility that the heuristic algorithm
* does not converge all the way to a max value.
*
* NOTE:	The _mipsTlbSizeGet entry point is deprecated and should not be used
*	in new code. Use mipsTlbSizeGet instead.
*
* NOTE:	A near-clone of this function exists in the 
*       target/config/mipsCommon/romMipsInit.s file, to avoid the need for a
*       'bal' to a label that can be too far for a 16-bit signed offset.
*       Any changes in this function should be coordinated in romMipsInit.s.
*
* int mipsTlbSizeGet (void)

* RETURNS: size of TLB.
*/

	.ent	mipsTlbSizeGet

mipsTlbSizeGet:
_mipsTlbSizeGet:
	
	/* Read the config1 register to determine the max TLB index. */
	
        mfc0    v0, C0_CONFIG,1  
        HAZARD_CP_READ
	and	v0, CFG1_MMUMASK
	srl	v0, CFG1_MMUSHIFT
	addu    v0, 1           /* Turn max index into count */
	jr	ra
	.end	mipsTlbSizeGet

/***************************************************************************
*
* mipsTlbClear -	 Clear the TLB by invalidating each entry
*
*
* Uses:		 a0, a2, a3, v0, t0-t9, ra
* Stack:	 None
*
* NOTE:	A near-clone of this function exists in the 
*       target/config/mipsCommon/romMipsInit.s file, to avoid the need for a
*       'bal' to a label that can be too far for a 16-bit signed offset.
*       Any changes in this function should be coordinated in romMipsInit.s.
*
*/

	.ent	mipsTlbClear
mipsTlbClear:
	move	a3, ra			/* save return address */
	bal	mipsTlbSizeGet
	move	ra, a3			/* restore return address */

	li	a2,TLB_4K_PAGE_SIZE_MASK
	mtc0	a2,C0_PAGEMASK		/* setup page mask */
	HAZARD_CP_WRITE

	/* Setup loop vars */
	
	li	a3,0			/* first TLB index */

	move	a2, v0			/* use calculated TLB count */
	li	t2, K0BASE		/* use KSEG 0 addresses for TLB */
	mtc0	zero, C0_TLBLO0		/* clear entry lo0, set invalid */
	mtc0	zero, C0_TLBLO1		/* clear entry lo1, set invalid */

1:
	/* make the TLBHI entry unique by using the entry number */
	
	sll	t1, a3, MMU_R4K_PAGE_SHIFT+1 /* Valid bit set to zero */
					/* +1 since VPN2 field maps 2 pages */
	or	t1, t2, t1		/* generate unique tlbhi value */

	/* store the entry */
	
	MTC0	t1, C0_TLBHI		/* clear entry high */
	mtc0	a3, C0_INX		/* set the index */
	HAZARD_CP_WRITE
	
	addiu	a3, 1
	tlbwi				/* write the TLB */
	HAZARD_TLB

	bne	a3, a2, 1b
		
	jr	ra
	.end	mipsTlbClear
	
/*******************************************************************************
*
* sysWbFlush - flush the write buffer
*
* This routine flushes the write buffers, making certain all subsequent
* memory writes have occurred.  It is used during critical periods only, e.g.,
* after memory-mapped I/O register access.
*
* RETURNS:	 N/A
*
* sysWbFlush (void)
*	
*/
        .ent    sysWbFlush
sysWbFlush:	
        sync
        j       ra
        .end    sysWbFlush


	
/*******************************************************************************
*
* sysFpaAck - acknowledge a floating point unit interrupt
*
* This routine writes the floating point unit (FPU) status register to
* acknowledge the appropriate FPU interrupt.  It returns an index to the vector
* table.
*
* RETURNS: An interrupt vector.

* int sysFpaAck (void)

*/

	.ent	sysFpaAck
sysFpaAck:
	j	ra			/* return to caller		*/
	.end	sysFpaAck


/******************************************************************************
*
* sysTimerIntClr - clear the intyerrupt generated by the CPU internal timer
*
* This routine clears the interrupt pending bit ,IP7, of the CAUSE register
* using a side effect of writing to the COMPARE register
*
* This routine is loaded into the static interrupt priority table.
* It is called by jumping to the address in this table, not by
* user calls.
*
* SEE ALSO: sysSetCompare()
*
* int sysTimerIntClr ()
*/

	.ent	sysTimerIntClr
sysTimerIntClr:
	/* give us as long as possible before a clock interrupt */

	li	t0, 1
	mtc0	t0, C0_COUNT
	mtc0	zero, C0_COMPARE
	nop
	j	ra			/* return */
	.end	sysTimerIntClr


/*******************************************************************************
*
* sysWait - wait until the input buffer become empty
*
* wait until the input buffer become empty
*
* RETURNS: N/A

* void sysWait (void)
 
*/

	.ent	sysWait
sysWait:
	j	ra			/* dummy routine for now */
	.end	sysWait


#define SYS_CONFIG1_GET
/*******************************************************************************
*
* sysConfig1Get - get the processor Config1 register
*
* NOTE:	 This function should be moved to sysMipsALib.s.
*
* RETURNS: N/A

* int sysConfig1Get (void)

*/
	.ent	sysConfig1Get
sysConfig1Get:
/*	mfc0	v0,C0_CONFIG */
	.word	0x40028001
	j	ra
	.end	sysConfig1Get



/* Include Generic MIPS Support */
		
/*******************************************************************************
*
* sysPciBusErrDisable - disable PCI bus error reporting
*
* RETURNS: N/A

* void sysPciBusErrDisable (void)

*/
	.ent	sysPciBusErrDisable
sysPciBusErrDisable:
	j	ra
	.end	sysPciBusErrDisable

/*******************************************************************************
*
* sysPciBusErrEnable - enable PCI bus error reporting
*
* RETURNS: N/A

* void sysPciBusErrEnable (void)

*/
	.ent	sysPciBusErrEnable
sysPciBusErrEnable:
	j	ra
	.end	sysPciBusErrEnable

/* Some performance and debug tools */
        
        .globl readCount
        .globl readInstr
	.globl readStatus
	.globl readConfig
        
#define MFC0_SEL_OPCODE(dst, src, sel)\
	  	.word (0x40000000 | ((dst)<<16) | ((src)<<11) | (sel))

#define MTC0_SEL_OPCODE(dst, src, sel)\
	  	.word (0x40800000 | ((dst)<<16) | ((src)<<11) | (sel))
                
#define INSTR      4


        .set    noreorder
                
        .ent    readCount
readCount:

	mfc0	v0, C0_COUNT
        nop
        
        jr      ra
        nop
        
        .end    readCount        
        
        .ent    readInstr

readInstr:

        MFC0_SEL_OPCODE(2, 25, INSTR)
        nop
        
        jr      ra
        nop
        
        .end    readInstr

	.ent	readStatus
readStatus:
	mfc0	v0, $12
	nop

	jr	ra
	nop

	.end	readStatus

	.ent	readConfig
readConfig:
	mfc0	v0, $16
	nop

	jr	ra
	nop

	.end	readConfig

/*************************************************************
*
* Support functions for CPU register read and write
*
*/
	.globl	ReadERRPC
	.ent	ReadERRPC
ReadERRPC:	
	.set	noreorder
	mfc0	v0, C0_ERRPC
	nop
	j	ra
	nop
	.set	reorder
	.end	ReadERRPC

	.globl	ReadBVA
	.ent	ReadBVA
ReadBVA:	
	.set	noreorder
	mfc0	v0, C0_BADVADDR
	nop
	j	ra
	nop
	.set	reorder
	.end	ReadBVA

	.globl	ReadEPC
	.ent	ReadEPC
ReadEPC:	
	.set	noreorder
	mfc0	v0, C0_EPC
	nop
	j	ra
	nop
	.set	reorder
	.end	ReadEPC

/*************************************************************
*
* Support functions for sysBackTrace
*
*/
        
             .ent vxSpGet
vxSpGet:
              .set noreorder
               j ra
               or v0,sp,sp
               .set reorder
               .end vxSpGet


              .ent vxPcGet
vxPcGet:
              .set noreorder
               or v0,ra,ra
               j ra
               nop
               .set reorder
               .end vxPcGet


#include <sysMipsALib.s>
#include <tlbmap.s>
