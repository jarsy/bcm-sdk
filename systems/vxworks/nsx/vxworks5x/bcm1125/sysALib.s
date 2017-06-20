/* sysALib.s - BCM1250 system-dependent assembly routines */

/* Copyright 1984-2002 Wind River Systems, Inc. */
        .data
        .globl  copyright_wind_river

/* $Id: sysALib.s,v 1.3 2011/07/21 16:14:43 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01b,06dec01,agf  add sysConfig1Set routine
01a,15nov01,agf  written
*/

/*
DESCRIPTION
This module contains system-dependent routines written in assembly
language.

This module must be the first specified in the \f3ld\f1 command used to
build the system.  The sysInit() routine is the system start-up code.
*/

#define _ASMLANGUAGE
#include "vxWorks.h"
#include "sysLib.h"
#include "config.h"
#include "asm.h"

#include "esf.h"
#include "bcm1250Lib.h"
#ifdef _SENTOSA_
#include "sentosa.h"
#elif defined _RHONE_
#include "rhone.h"
#else
#include "swarm.h"
#endif

	/* defines */
#define LED_CHAR0       (32+8*3)
#define LED_CHAR1       (32+8*2)
#define LED_CHAR2       (32+8*1)
#define LED_CHAR3       (32+8*0)

/* in case not defined in installed version of Tornado */
#ifndef TLBHI_VPN2MASK
#define	TLBHI_VPN2MASK	0xffffe000
#endif

	/* internals */

	.globl	sysInit			/* start of system code */
	.globl	sysClearTlbEntry	/* clear entry in tlb */
	.globl	sysWbFlush		/* flush write buffers */
	.globl	sysBusTas		/* atomically test and set a char */
	.globl	sysFpaDeMux		/* which FPA int is bugging us */
	.globl	sysFpaAck		/* clear FPA interrupt */
	.globl  sysConfig1Get
	.globl  sysConfig1Set
#ifndef _SENTOSA_
	.globl  sysLedDsply
#endif
	.globl  sysAltGo

	/* externals */

	.globl	usrInit		/* system initialization routine */

	/* macros */
/* in archMips.h these macros are not assembler friendly, so fix for here */
#undef  PHYS_TO_K0
#define PHYS_TO_K0(x)   (K0BASE | (x))
#undef  PHYS_TO_K1
#define PHYS_TO_K1(x)   (K1BASE | (x))

#ifndef _SENTOSA_
#define SETLEDS(a,b,c,d)                                 \
       li     a0,(((a)<<24)|((b)<<16)|((c)<<8)|(d)) ;    \
       bal    sysLedDsply
#else
#define SETLEDS(a,b,c,d) 
#endif

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
* NOTE: This routine should not be called by the user.
*
* RETURNS: N/A

* sysInit (void)	/@ THIS IS NOT A CALLABLE ROUTINE @/

*/

	.ent	sysInit
sysInit:

	la	gp,_gp			/* set global ptr from cmplr */

	/* disable all interrupts, fpa usable */
	li	t0, SR_CU1|SR_CU0|SR_IE
	mtc0	t0, C0_SR		/* put on processor	*/
	la	sp, sysInit-32		/* set stack to grow down from code,
					   leave room for four parameters */

	/* give us as long as possible before a clock interrupt */
	li	v0,1
	mtc0	v0,C0_COUNT
	mtc0	zero,C0_COMPARE

	jal	sysClearTlb		/* make sure tlb is invalid */

	SETLEDS('v','x','W','o')

	li	a0, BOOT_WARM_AUTOBOOT 	/* push start type arg = WARM_BOOT */
	jal	usrInit			/* never returns - starts up kernel */
	li	ra, R_VEC		/* load prom reset address */
	j	ra			/* just in case */
	.end	sysInit

/*******************************************************************************
*
* sysWbFlush - flush the write buffer
*
* This routine flushes the write buffers, making certain all subsequent
* memory writes have occurred.  It is used during critical periods only, e.g.,
* after memory-mapped I/O register access.
*
* RETURNS: N/A

* sysWbFlush (void)

*/
	.ent	sysWbFlush
sysWbFlush:
	.set noreorder
        j       ra
	sync
	.set reorder
	.end	sysWbFlush


/*******************************************************************************
*
* sysBusTas - test and set a location across the bus
*
* This routine performs an atomic test-and-set operation.
*
* RETURNS: TRUE if the value had not been set but is now, or FALSE if the
* value was set already.
*
* NOTE: Other MIPS implementations return FALSE if the address is not in
* KSEG0 or KSEG1.  This implementation does not do that, but instead has
* the requirement that the address be in cacheable coherent space.  There's
* no easy way to test for that.  (Typically, it will be true if using KSEG0
* with Config:K0 set properly, but there are other conditions in which it
* is true which should not be excluded by this implementation.)
*
* SEE ALSO: sysAuxClkEnable(), sysAuxClkRateGet()
*/

        .ent    sysBusTas
sysBusTas:
	.set noreorder

	ori	t0, a0, 0x3
	andi	t1, a0, 0x3

	xori	t0, t0, 0x3		# t0 = aligned address
	xori	t1, t1, 0x3

	li	t2, 0xff
	sll	t1, t1, 0x3		# t1 = bit shift

	sll	t2, t2, t1		# t2 = byte mask

	sync				# Pass1 bug workaround.

	ll	t3, 0(t0)		# t3 = word containing flag
	and	t4, t2, t3		# t4 = flag
	bne	t4, zero, 1f		# if already set, just fail.

	or	t4, t2, t3		# t4 = word with flag set.
	sc	t4, 0(t0)		# store it back, t0 == sc flag
	beq	t4, zero, 1f
	nop

	# success: flag was previously unset, and new value written.
	jr	ra
	li	v0, TRUE


	# failure: flag was previously set.
1:	jr	ra
	li	v0, FALSE

	.set reorder
        .end    sysBusTas

/*******************************************************************************
*
* sysClearTlbEntry - clear translation lookaside buffer entry
*
* This routine clears a specified translation lookaside buffer (TLB)
* entry by writing a zero to the virtual page number and valid bit.
*
* RETURNS: N/A

* void sysClearTlbEntry
*     (
*     int entry
*     )

*/

	.ent	sysClearTlbEntry
sysClearTlbEntry:
	subu	t0, a0, TLB_ENTRIES - 1 /* how many tlb entries are there */
	bgtz	t0, invalidEntry	/* is my index bad ? */
	li	t2,K0BASE&TLBHI_VPN2MASK
	sll	t1, a0, 13
	addu	t2, t1, t2
	mtc0	t2,C0_TLBHI		/* zero tlbhi entry */
	mtc0	zero,C0_TLBLO0		/* set valid bit to zero */
	mtc0	zero,C0_TLBLO1		/* set valid bit to zero */
	mtc0	a0,C0_INX		/* set up index for write     */
	c0	C0_WRITEI		/* write entry */
invalidEntry:
	j	ra
	.end	sysClearTlbEntry

/*******************************************************************************
*
* sysFpaDeMux - determine which FPA exception is pending
*
* This routine reads the floating point unit (FPU) status to determine which
* FPU exception generated an interrupt to the processor.  It returns an
* index to the vector table.
*
* This routine is loaded into the static interrupt priority table.
* It is called by jumping to the address in this table, not by
* user calls.
*
* RETURNS: An interrupt vector.

* int sysFpaDeMux
*     (
*     int vecbase	/@ base location of FPA vectors in excBsrTbl @/
*     )

*/

	.ent	sysFpaDeMux
sysFpaDeMux:
	.set	noreorder
	cfc1	v0, C1_SR			/* grab FPA status	*/
	nop
	.set	reorder
	li	a2, FP_EXC_MASK			/* load cause mask	*/
	and	a2, v0				/* look at cause only	*/
	srl	a2, FP_EXC_SHIFT		/* place cause in lsb	*/
	li	a1, FP_ENABLE_MASK		/* load enable mask	*/
	and	a1, a1, v0			/* look at enable only	*/
	srl	a1, FP_ENABLE_SHIFT		/* place enabled in lsb	*/
	li	a3, (FP_EXC_E>>FP_EXC_SHIFT)	/* ld unimp op bit	*/
	or	a1, a3				/* no mask bit for this	*/
	and	a2, a1				/* look at just enabled	*/
	and	v1, v0, ~FP_EXC_MASK		/* clear the exceptions */
	lbu	v0, ffsLsbTbl(a2)		/* lkup first set bit	*/
	addu	v0, a0				/* increment io vector	*/
	ctc1	v1, C1_SR			/* clear fp condition	*/
	j	ra				/* return to caller	*/
	.end	sysFpaDeMux

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
	cfc1	v0, C1_SR		/* read control/status reg	*/
	and	t0, v0, ~FP_EXC_MASK	/* zero bits		*/
	ctc1	t0, C1_SR		/* acknowledge interrupt	*/
	j	ra			/* return to caller		*/
	.end	sysFpaAck


/*******************************************************************************
*
* sysConfig1Get - get the processor Config1 register
*
* RETURNS: value of CO_CONFIG,1 register

* int sysConfig1Get (void)

*/
        .ent    sysConfig1Get
sysConfig1Get:
/*      mfc0    v0,C0_CONFIG,1 */
        .word   0x40028001
        j       ra
        .end    sysConfig1Get


/*******************************************************************************
*
* sysConfig1Set - set the processor Config1 register
*
* RETURNS: N/A

* void sysConfig1Get (int)

*/
        .ent    sysConfig1Set
sysConfig1Set:
/*      mtc0    a0,C0_CONFIG,1 */
        .word   0x40848001
        j       ra
        .end    sysConfig1Set


/**************************************************************************
*
* sysLedDsply -  put 4 ascii characters on LED display
*
* RETURNS : N/A

* void sysLedDsply (int )

*/
#ifndef _SENTOSA_ 
        .ent	sysLedDsply
sysLedDsply:

        li      t0,PHYS_TO_K1(LEDS_PHYS)

        rol     a0,a0,8
        and     t1,a0,0xFF
        sb      t1,LED_CHAR0(t0)

        rol     a0,a0,8
        and     t1,a0,0xFF
        sb      t1,LED_CHAR1(t0)

        rol     a0,a0,8
        and     t1,a0,0xFF
        sb      t1,LED_CHAR2(t0)

        rol     a0,a0,8
        and     t1,a0,0xFF
        sb      t1,LED_CHAR3(t0)

        j       ra

	.end    sysLedDsply
#endif
#ifdef BCM1250_CPU_0
/**************************************************************************
*
* sysAltGo -  give cpu1 an address where it should start executing from 
*
* Cpu1 is in a tight loop monitoring its mailbox register. If the register
* is written to with a non-zero value, cpu1 will leave its tight loop
* and begin executing from the value that was written to the register.
*
* RETURNS : N/A

* void sysAltGo (int )

*/

        .ent	sysAltGo
sysAltGo:

                la      t1,PHYS_TO_K1(A_IMR_REGISTER(1,R_IMR_MAILBOX_SET_CPU))
                sd      a0,0(t1)        # Write to mailbox register

                j       ra

        .end	sysAltGo

#endif /* BCM1250_CPU_0 */

        .globl  bcm1250_sys_reset
        .ent    bcm1250_sys_reset
bcm1250_sys_reset:
                .set noreorder
                li      a0,PHYS_TO_K1(A_SCD_SYSTEM_CFG)
                ld      t0,0(a0)
                dli     t1,M_SYS_SYSTEM_RESET  # Reset mask
                or      t0,t1                   # New value to write
                sd      t0,0(a0)                # system reset is issued
                .set reorder

                j       ra                     # should not come here

        .end    bcm1250_sys_reset

              .globl  vxSpGet
              .ent vxSpGet
vxSpGet:
              .set noreorder
               j ra
               or v0,sp,sp
               .set reorder
               .end vxSpGet


              .globl  vxPcGet
              .ent vxPcGet
vxPcGet:
              .set noreorder
               or v0,ra,ra
               j ra
               nop
               .set reorder
               .end vxPcGet

#include "sysMipsALib.s"
