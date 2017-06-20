/* hs_rdwr.s - specialized routines for systems w/o 64 bit ptr support */

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

/* $Id: hs_rdwr.s,v 1.3 2011/07/21 16:14:49 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01a,15nov01,agf  Created for the bcm1250 BSP

*/

/*
DESCRIPTION
Functions to read and write through 64 bit addresses in systems
without a 64-bit pointer model.  All reads produce unsigned results
of the indicated size.  These functions are coded assuming that
Status.KX is set and will fail on CPUs that only support 32-bit
addresses.
*/

/* includes */

#define _ASMLANGUAGE
#include "vxWorks.h"
#include "config.h"
#include "asm.h"

/* defines */

/* globals */


	.globl	hs_read8
	.globl	hs_write8
	.globl	hs_read16
	.globl	hs_write16
	.globl	hs_read32
	.globl	hs_write32
	.globl	hs_read64
	.globl	hs_write64

/* Versions for __mips64 (either endian) or !__mips64 (big or little endian) */
	
#ifdef __mips64
	
/***************************************************************************
*
* hs_read8 - read a unsigned byte
*
*  Input parameters:
*      a0 - 64 bit address
*
*  Return value:
*      v0 - unsigned byte read 
*/                                                                          
        .ent    hs_read8
hs_read8:
	lbu	v0, 0(a0)
	jr	ra
	.end	hs_read8

/***************************************************************************
*
* hs_write8 - write a unsigned byte
*
*  Input parameters:
*      a0 - 64 bit address
*      a1 - unsigned byte to write 
*
*  Return value:
*      none 
*/                                                              
 	.ent	hs_write8
hs_write8:
	sb	a1, 0(a0)
	jr	ra
	.end	hs_write8


/***************************************************************************
*
* hs_read16 - read a unsigned 16 bit word
*
*  Input parameters:
*      a0 - 64 bit address
*
*  Return value:
*      v0 - unsigned 16 bit word read
*/
        .ent    hs_read16
hs_read16:
	lhu	v0, 0(a0)
	jr	ra
	.end	hs_read16

/***************************************************************************
*
* hs_write16 - write a unsigned 16 bit word
*
*  Input parameters:
*      a0 - 64 bit address
*      a1 - unsigned 16 bit word to write 
*
*  Return value:
*      none 
*/                                                         
 	.ent	hs_write16
hs_write16:
	sh	a1, 0(a0)
	jr	ra
	.end	hs_write16


/***************************************************************************
*
* hs_read32 - read a unsigned 32 bit long word
*
*  Input parameters:
*      a0 - 64 bit address
*
*  Return value:
*      v0 - unsigned 32 bit long word read
*/
        .ent    hs_read32
hs_read32:
	lwu	v0, 0(a0)
	jr	ra
	.end	hs_read32

/***************************************************************************
*
* hs_write32 - write a unsigned 32 bit long word
*
*  Input parameters:
*      a0 - 64 bit address
*      a1 - unsigned 32 bit long word to write 
*
*  Return value:
*      none 
*/                                                                 
 	.ent	hs_write32
hs_write32:
	sw	a1, 0(a0)
	jr	ra
	.end	hs_write32


/***************************************************************************
*
* hs_read64 - read a unsigned 64 bit quad word
*
*  Input parameters:
*      a0 - 64 bit address
*
*  Return value:
*      v0 - unsigned 64 bit quad word read
*/
        .ent    hs_read64
hs_read64:
	ld	v0, 0(a0)
	jr	ra
	.end	hs_read64

/***************************************************************************
*
* hs_write64 - write a unsigned 64 bit quad word
*
*  Input parameters:
*      a0 - 64 bit address
*      a1 - unsigned 64 bit quad word to write 
*
*  Return value:
*      none 
*/                                                                
 	.ent	hs_write64
hs_write64:
	sd	a1, 0(a0)
	jr	ra
	.end	hs_write64

#else  /* !__mips64 */

#if _BYTE_ORDER == _BIG_ENDIAN

/***************************************************************************
*
* hs_read8 - read a unsigned byte
*
*  Input parameters:
*      a0 - high 32 bit address
*      a1 - low 32 bit address
*
*  Return value:
*      v0 - unsigned byte read 
*/                                                                          
        .ent    hs_read8
hs_read8:
	dsll	a0, a0, 32			# addr high in a0
	dsll	a1, a1, 32			# addr low in a0
	dsrl	a1, a1, 32			# make it zero extended
	or	a0, a0, a1			# merge		
	lbu	v0, 0(a0)
	jr	ra
	.end	hs_read8


/***************************************************************************
*
* hs_write8 - write a unsigned byte
*
*  Input parameters:
*      a0 - high 32 bit address
*      a1 - low 32 bit address
*      a2 - byte value to write 
*  Return value:
*      none 
*/                                                                          
	.ent	hs_write8
hs_write8:
	dsll	a0, a0, 32
	dsll	a1, a1, 32
	dsrl	a1, a1, 32
	or	a0, a0, a1
	sb	a2, 0(a0)
	jr	ra
	.end	hs_write8


/***************************************************************************
*
* hs_read16 - read a 16 bit word 
*
*  Input parameters:
*      a0 - high 32 bit address
*      a1 - low 32 bit address
*
*  Return value:
*      v0 - 16 bit word read 
*/                                                                          
        .ent    hs_read16
hs_read16:
	dsll	a0, a0, 32
	dsll	a1, a1, 32
	dsrl	a1, a1, 32
	or	a0, a0, a1	
	lhu	v0, 0(a0)
	jr	ra
	.end	hs_read16

/***************************************************************************
*
* hs_write16 - write a 16bit word
*
*  Input parameters:
*      a0 - high 32 bit address
*      a1 - low 32 bit address
*      a2 - 16 bit word value to write 
*  Return value:
*      none 
*/                                                                          
	.ent	hs_write16
hs_write16:
	dsll	a0, a0, 32
	dsll	a1, a1, 32
	dsrl	a1, a1, 32
	or	a0, a0, a1	
	sh	a2, 0(a0)
	jr	ra
	.end	hs_write16


/***************************************************************************
*
* hs_read32 - read a 32 bit word 
*
*  Input parameters:
*      a0 - high 32 bit address
*      a1 - low 32 bit address
*
*  Return value:
*      v0 - 32 bit word read 
*/                                                                          
        .ent    hs_read32
hs_read32:
	dsll	a0, a0, 32
	dsll	a1, a1, 32
	dsrl	a1, a1, 32
	or	a0, a0, a1	
	lwu	v0, 0(a0)
	jr	ra
	.end	hs_read32

/***************************************************************************
*
* hs_write32 - write a 32bit word
*
*  Input parameters:
*      a0 - high 32 bit address
*      a1 - low 32 bit address
*      a2 - 32 bit word value to write 
*  Return value:
*      none 
*/                                                                          
	.ent	hs_write32
hs_write32:
	dsll	a0, a0, 32
	dsll	a1, a1, 32
	dsrl	a1, a1, 32
	or	a0, a0, a1	
	sw	a2, 0(a0)
	jr	ra
	.end	hs_write32


/***************************************************************************
*
* hs_read64 - read a 64 bit word 
*
*  Input parameters:
*      a0 - high 32 bit address
*      a1 - low 32 bit address
*
*  Return value:
*      v0 - high 32 bit word read 
*      v1 - low 32 bit word read
*/                                                                          
        .ent    hs_read64
hs_read64:
	dsll	a0, a0, 32			# addr high in a0
	dsll	a1, a1, 32			# addr low in a1
	dsrl	a1, a1, 32
	or	a0, a0, a1	
	ld	v0, 0(a0)
	sll	v1, v0, 0			# low word in v1
	dsra	v0, v0, 32			# high word in v0
	jr	ra
	.end	hs_read64

/***************************************************************************
*
* hs_write64 - write a 64bit word
*
*  Input parameters:
*      a0 - high 32 bit address
*      a1 - low 32 bit address
*      a2 - high 32 bit word value to write 
*      a3 - low 32 bit word value to write 
*  Return value:
*      none 
*/                                                                          
	.ent	hs_write64
hs_write64:
	dsll	a0, a0, 32			# addr high in a0
	dsll	a1, a1, 32			# addr low in a1
	dsll	a2, a2, 32			# high word in a2
	dsll	a3, a3, 32			# low word in a3
	dsrl	a1, a1, 32
	dsrl	a3, a3, 32			# (make it zero extended)
	or	a0, a0, a1	
	or	a2, a2, a3
	sd	a2, 0(a0)
	jr	ra
	.end	hs_write64

#else /*_BYTE_ORDER == _LITTLE_ENDIAN*/

/***************************************************************************
*
* hs_read8 - read a unsigned byte
*
*  Input parameters:
*      a0 - low 32 bit address
*      a1 - high 32 bit address
*
*  Return value:
*      v0 - unsigned byte read 
*/                                                                          
        .ent    hs_read8
hs_read8:
	dsll	a1, a1, 32			# addr high in a1
	dsll	a0, a0, 32			# addr low in a0
	dsrl	a0, a0, 32			# sign extend
	or	a0, a0, a1			# merge	
	lbu	v0, 0(a0)
	jr	ra
	.end	hs_read8


/***************************************************************************
*
* hs_write8 - write a unsigned byte
*
*  Input parameters:
*      a0 - low 32 bit address
*      a1 - high 32 bit address
*      a2 - byte value to write 
*  Return value:
*      none 
*/                                                                          
	.ent	hs_write8
hs_write8:
	dsll	a1, a1, 32
	dsll	a0, a0, 32
	dsrl	a0, a0, 32
	or	a0, a0, a1
	sb	a2, 0(a0)
	jr	ra
	.end	hs_write8


/***************************************************************************
*
* hs_read16 - read a 16 bit word 
*
*  Input parameters:
*      a0 - low 32 bit address
*      a1 - high 32 bit address
*
*  Return value:
*      v0 - 16 bit word read 
*/                                                                          
        .ent    hs_read16
hs_read16:
	dsll	a1, a1, 32
	dsll	a0, a0, 32
	dsrl	a0, a0, 32
	or	a0, a0, a1
	lhu	v0, 0(a0)
	jr	ra
	.end	hs_read16

/***************************************************************************
*
* hs_write16 - write a 16bit word
*
*  Input parameters:
*      a0 - low 32 bit address
*      a1 - high 32 bit address
*      a2 - 16 bit word value to write 
*  Return value:
*      none 
*/                                                                          
	.ent	hs_write16
hs_write16:
	dsll	a1, a1, 32
	dsll	a0, a0, 32
	dsrl	a0, a0, 32
	or	a0, a0, a1
	sh	a2, 0(a0)
	jr	ra
	.end	hs_write16


/***************************************************************************
*
* hs_read32 - read a 32 bit word 
*
*  Input parameters:
*      a0 - low 32 bit address
*      a1 - high 32 bit address
*
*  Return value:
*      v0 - 32 bit word read 
*/                                                                          
        .ent    hs_read32
hs_read32:
	dsll	a1, a1, 32
	dsll	a0, a0, 32
	dsrl	a0, a0, 32
	or	a0, a0, a1
	lwu	v0, 0(a0)
	jr	ra
	.end	hs_read32

/***************************************************************************
*
* hs_write32 - write a 32bit word
*
*  Input parameters:
*      a0 - low 32 bit address
*      a1 - high 32 bit address
*      a2 - 32 bit word value to write 
*  Return value:
*      none 
*/                                                                          
	.ent	hs_write32
hs_write32:
	dsll	a1, a1, 32
	dsll	a0, a0, 32
	dsrl	a0, a0, 32
	or	a0, a0, a1
	sw	a2, 0(a0)
	jr	ra
	.end	hs_write32


/***************************************************************************
*
* hs_read64 - read a 64 bit word 
*
*  Input parameters:
*      a0 - low 32 bit address
*      a1 - high 32 bit address
*
*  Return value:
*      v0 - low 32 bit word read 
*      v1 - high 32 bit word read
*/                                                                          
        .ent    hs_read64
hs_read64:
	dsll	a1, a1, 32
	dsll	a0, a0, 32
	dsrl	a0, a0, 32
	or	a0, a0, a1
	ld	v0, 0(a0)
	dsra	v1, v0, 32			# high word in v1
	sll	v0, v0, 0			# low word in v0
	jr	ra
	.end	hs_read64

/***************************************************************************
*
* hs_write64 - write a 64bit word
*
*  Input parameters:
*      a0 - low 32 bit address
*      a1 - high 32 bit address
*      a2 - low 32 bit word value to write 
*      a3 - high 32 bit word value to write 
*  Return value:
*      none 
*/                                                                          
	.ent	hs_write64
hs_write64:
	dsll	a1, a1, 32
	dsll	a0, a0, 32
	dsrl	a0, a0, 32
	or	a0, a0, a1
	dsll	a2, a2, 32			# low word in a2
	dsrl	a2, a2, 32			# (make it zero extended)
	dsll	a3, a3, 32			# high word in a3
	or	a2, a2, a3
	sd	a2, 0(a0)
	jr	ra
	.end	hs_write64

#endif /*_ENDIAN*/
	
#endif /*__mips64*/
