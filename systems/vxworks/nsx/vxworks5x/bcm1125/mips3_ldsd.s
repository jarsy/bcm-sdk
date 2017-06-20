/* mips3_ldsd.s - commonized 64 bit load/store routines */

/* Copyright 2002 Wind River Systems, Inc. */

/*********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
********************************************************************* */

/* $Id: mips3_ldsd.s,v 1.3 2011/07/21 16:14:43 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01b,07dec01,agf  apply coding standard fix-ups
01a,15nov01,agf  Created for the bcm1250 BSP

*/

/*
DESCRIPTION
This file consists of routines to load and store 64 bit quad words for 
either mips3 [non-64 bit] or mips64 [64 bit] ISA's.
*/

/* includes */

#define _ASMLANGUAGE
#include "vxWorks.h"
#include "config.h"
#include "asm.h"

/* defines */

/* globals */

        .globl  mips3_ld
        .globl  mips3_sd

/***************************************************************************
*
* mips3_ld - common function to load a 64 bit quad word in either mips3 
*            or mips64 fashion 
*
* mips3_ld((volatile unsigned long long *) (addr))
*
*  Input parameters:
*      a0 - 32 bit address
*
*  Return value:
*      if mips 64 architecture
*          v0 - 64 bit quad word read
*      else
*          v0, v1 - low and high 32 bit long word value of the 64 bit
*                   order depends on endianess  
*/         
        .ent    mips3_ld
mips3_ld:
#ifdef __mips64
	ld	v0, 0(a0)
	jr	ra
#else
	ld	v0, 0(a0)
#if _BYTE_ORDER == _BIG_ENDIAN
	sll	v1, v0, 0			# low word in v1
	dsra	v0, v0, 32			# high word in v0
#else
	dsra	v1, v0, 32			# high word in v1
	sll	v0, v0, 0			# low word in v0
#endif
	jr	ra
#endif /* __mips64 */
	.end	mips3_ld


/***************************************************************************
*
* mips3_sd - commnon function to store a 64 bit quad word in either mips3 
*            or mips64 fashion 
*
* mips3_sd((volatile unsigned long long *) (addr), (value))
*
*  Input parameters:
*      a0 - 32 bit address
*      if mips 64 architecture
*          a1 - 64 bit quad word to write
*      else
*          a2, a3 - low and high 32 bit long word value of the 64 bit
*                   order of a2 and a3 depends on endianess  
*          note: a2 is padding
*
*  Return value:
*      none
*/         
	.ent	mips3_sd
mips3_sd:
#ifdef __mips64
	sd	a1, 0(a0)
	jr	ra
#else
#if _BYTE_ORDER == _BIG_ENDIAN
	dsll	a2, a2, 32			# high word in a2
	dsll	a3, a3, 32			# low word in a3
	dsrl	a3, a3, 32			# (make it zero extended)
#else
	dsll	a2, a2, 32			# low word in a2
	dsrl	a2, a2, 32			# (make it zero extended)
	dsll	a3, a3, 32			# high word in a3
#endif
	or	a2, a2, a3
	sd	a2, 0(a0)
	jr	ra
#endif /* __mips64 */
	.end	mips3_sd

