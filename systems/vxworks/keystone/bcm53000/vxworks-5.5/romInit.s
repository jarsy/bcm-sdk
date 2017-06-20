/* romInit.s - ROM initialization module */

/* Copyright (c) 2000-2007 Wind River Systems, Inc. 
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/* $Id: romInit.s,v 1.3 2011/07/21 16:14:25 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */
	

/*
DESCRIPTION 
This module contains the entry code for the VxWorks bootrom.
The entry point romInit, is the first code executed on power-up.

The routine sysToMonitor() jumps to romInit() to perform a
"warm boot".
*/

#define _ASMLANGUAGE
#define _ASSEMBLER_


/* includes */

#include <vxWorks.h>
#include <sysLib.h>
#include <cacheLib.h>
#include "config.h"

#include <asm.h>
#include "mipsR2Hazards.h"
#include <esf.h>
#include <archMips66.h>	
	
#include <drv/pci/pciConfigLib.h>

/* defines */
#ifndef INITIAL_CFG
#define INITIAL_CFG 3  /* 3 = cached */
#endif
#ifndef INITIAL_SR
#define INITIAL_SR (SR_CU0 | SR_BEV)
#endif

/* Swaps data in registers */

#define SWAPEND32( src, tmp0, tmp1 )\
	and	tmp0, src, 0xff;\
	srl	src,  8;\
	sll	tmp0, 8;\
	and	tmp1, src, 0xff;\
	or	tmp0, tmp1;\
	srl	src,  8;\
	sll	tmp0, 8;\
	and	tmp1, src, 0xff;\
	or	tmp0, tmp1;\
	srl	src,  8;\
	sll	tmp0, 8;\
	or	src,  tmp0

/* 
 * Configure the CONFIG register
 * 
 * The HAZARD_CP_READ macro is blank for the 5kx processors.
 * This causes the default CONFIG_INIT macro to expand to
 * have 2 ;'s in a row. Diab reports an error and will not
 * assemble such syntax. An SPR is against Diab to fix
 * this. Until then, we must maintain a separate CONFIG_INIT
 * explicitly without the HAZARD_CP_READ macro.
 */
#if defined (_WRS_MIPS_5KC) || defined (_WRS_MIPS_5KF)
#define CONFIG_INIT \
	mfc0	t0, C0_CONFIG	; \
	and	t0, ~INITIAL_CFG_MASK	; \
	or	t0, INITIAL_CFG	; \
	mtc0	t0, C0_CONFIG	; \
	HAZARD_CP_WRITE
#endif

/* create a mask of ones */

#define MSK(n)			((1 << (n)) - 1)

/* translate address to kseg1 */
	
#define KSEG1(addr)		(((addr) & 0x1fffffff) | K1BASE)

/* EEPROM Error values */

#define ERROR_SPD		0x8
#define ERROR_MEMBASE_ALIGN	0x9
#define ERROR_SDRAM_BANKSIZE	0xa
#define ERROR_SDRAM_CASLAT	0xb
#define ERROR_SDRAM_BURSTLEN	0xc
#define ERROR_SDRAM_ERRORCHECK	0xd
#define ERROR_SDRAM_DEV_BANKS	0xe
#define ERROR_STRUCTURE		0xf
#define ERROR_SDRAM_CONFIG	0x10
#define ERROR_SDRAM_WIDTH		0x11
#define ERROR_SDRAM_MOD_BANKS           0x12
#define ERROR_SDRAM_SIZE                0x13

/* write characters to the ASCII Display */

#define DELAY(n)					  \
	li	k0, n					; \
	sll	k0, 12					; \
69:							; \
	subu	k0, 1					; \
	bnez	k0, 69b					; \
	nop


	.globl	romTextUpdate		/* do cache-text-update */


#include <romMipsInit.s>	
#include <sbmemc.s>	
#include <bcmcore_l1cache.s>	

 /***************************************************************************   
  *   
  * romClearEdac - clear error detection and correction logic   
  *   
  * This routine clears the memory and error detection logic by   
  * doing word writes to each DRAM location.   
  *   
  *   
  */   
     
          .ent    romClearEdac   
  romClearEdac:   
     
#if 0     
          mfc0    v1,C0_SR        /* disable parity errors */   
          HAZARD_CP_READ   
          or      v0,v1,SR_DE   
          mtc0    v0,C0_SR   
          HAZARD_CP_WRITE   
#endif     
          li      a0, ((RAM_LOW_ADRS & ADDRESS_SPACE_MASK) | K1BASE)   
          li      a1, K1BASE   
  clearloop:   
          sw      zero, -4(a0)   
          sw      zero, -8(a0)   
          sw      zero, -12(a0)   
          sw      zero, -16(a0)   
          sw      zero, -20(a0)   
          sw      zero, -24(a0)   
          sw      zero, -28(a0)   
          sw      zero, -32(a0)   
          sw      zero, -36(a0)   
          sw      zero, -40(a0)   
          sw      zero, -44(a0)   
          sw      zero, -48(a0)   
          sw      zero, -52(a0)   
          sw      zero, -56(a0)   
          sw      zero, -60(a0)   
          sw      zero, -64(a0)   
          subu    a0, 64   
          bne     a0, a1, clearloop   
  done:   
#if 0
          mtc0    v1,C0_SR   
          HAZARD_CP_WRITE   
#endif
          jr      ra   
          .end    romClearEdac 

#if defined (_WRS_MIPS_5261)
/***************************************************************************
*
* romTextUpdate - do cache-text-update from bootApp
*
* Tail-call to R5k cache routine, as we don't need to allocate a stack.
* 
*/

	.ent	romTextUpdate
romTextUpdate:
	li	a0, 0x8000 /* I-cache size */
	li	a1, 32 /* I-cache line size */
	li	a2, 0x8000 /* D-cache size */
	li	a3, 32 /* D-cache line size */
	la	t0, cacheR5kRomTextUpdate
	jr	t0
	.end	romTextUpdate
#endif

/***************************************************************************
*
* romExcHandle - rom based exception/interrupt handler
*
* This routine is invoked on an exception or interrupt while
* the status register is using the bootstrap exception vectors.
* It saves a state frame to a known uncached location. 
*
* THIS ROUTIINE IS NOT CALLABLE FROM "C"
*
*/

#define ROM_ISP_BASE	0xa0010000

.ent	romExcHandle

romExcHandle:

	.set	noat
	li	sp, ROM_ISP_BASE	/* sp to known uncached location */
	SW	sp, E_STK_SP-ESTKSIZE(sp) /* save sp in new intstk frame */
	subu	sp, ESTKSIZE		/* make new exc stk frame	*/
	SW	k0, E_STK_K0(sp)	/* save k0, (exception type)	*/
	SW	AT, E_STK_AT(sp)	/* save asmbler resvd reg	*/
	.set	at
	
	SW	v0, E_STK_V0(sp)	/* save func return 0, used */
					/* to hold masked cause	    */
	mfc0	k1, C0_BADVADDR		/* read bad VA reg	*/
	sw	k1, E_STK_BADVADDR(sp)	/* save bad VA on stack */
	mfc0	k1, C0_EPC		/* read exception pc	*/
	sw	k1, E_STK_EPC(sp)	/* save EPC on stack	*/
	mfc0	v0, C0_CAUSE		/* read cause register	*/
	sw	v0, E_STK_CAUSE(sp)	/* save cause on stack	*/
	mfc0	k1, C0_SR		/* read status register */
	sw	k1, E_STK_SR(sp)	/* save status on stack */

	.set	noat
	mflo	AT			/* read entry lo reg	*/
	SW	AT,E_STK_LO(sp)		/* save entry lo reg	*/
	mfhi	AT			/* read entry hi reg	*/
	SW	AT,E_STK_HI(sp)		/* save entry hi reg	*/
	.set	at
	SW	zero, E_STK_ZERO(sp)	/* save zero ?!	    */
	SW	v1, E_STK_V1(sp)	/* save func return 1	*/
	SW	a0, E_STK_A0(sp)	/* save passed param 0	*/
	SW	a1, E_STK_A1(sp)	/* save passed param 1	*/
	SW	a2, E_STK_A2(sp)	/* save passed param 2	*/
	SW	a3, E_STK_A3(sp)	/* save passed param 3	*/
	SW	t0, E_STK_T0(sp)	/* save temp reg 0	*/
	SW	t1, E_STK_T1(sp)	/* save temp reg 1	*/
	SW	t2, E_STK_T2(sp)	/* save temp reg 2	*/
	SW	t3, E_STK_T3(sp)	/* save temp reg 3	*/
	SW	t4, E_STK_T4(sp)	/* save temp reg 4	*/ 
	SW	t5, E_STK_T5(sp)	/* save temp reg 5	*/
	SW	t6, E_STK_T6(sp)	/* save temp reg 6	*/
	SW	t7, E_STK_T7(sp)	/* save temp reg 7	*/
	SW	t8, E_STK_T8(sp)	/* save temp reg 8	*/
	SW	t9, E_STK_T9(sp)	/* save temp reg 9	*/
	SW	s0, E_STK_S0(sp)	/* save saved reg 0	 */
	SW	s1, E_STK_S1(sp)	/* save saved reg 1	 */
	SW	s2, E_STK_S2(sp)	/* save saved reg 2	 */
	SW	s3, E_STK_S3(sp)	/* save saved reg 3	 */
	SW	s4, E_STK_S4(sp)	/* save saved reg 4	 */
	SW	s5, E_STK_S5(sp)	/* save saved reg 5	 */
	SW	s6, E_STK_S6(sp)	/* save saved reg 6	 */
	SW	s7, E_STK_S7(sp)	/* save saved reg 7	 */
	SW	s8, E_STK_FP(sp)	/* save saved reg 8	 */
	SW	gp, E_STK_GP(sp)	/* save global pointer?	 */
	SW	ra, E_STK_RA(sp)	/* save return address	 */

	.end	romExcHandle		/* that's all folks */


	
/************************************************************************
 *  Definitions
 ************************************************************************/
	.set	noreorder


	
/* include system controller support functions here */
	
	
	

