/* bcm1250IntLib.h - BCM1250 Interrupt support public constants header file */

/* Copyright 2001 Wind River Systems, Inc. */

/* $Id: bcm1250IntLib.h,v 1.3 2011/07/21 16:14:48 yshtil Exp $
********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and conditions
*  of this license.  No title or ownership is transferred hereby.
********************************************************************* */

/*
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01b,11apr05,kab  fix comments for apiGen (SPR 107842)
01a,15nov01,agf  written
*/

#ifndef __INCbcm1250IntLibh
#define __INCbcm1250IntLibh

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ASMLANGUAGE

#if defined(__STDC__) || defined(__cplusplus)

STATUS bcm1250IntLibInit	(void);
STATUS bcm1250IntConnect	(int intrSource, int intrVecNum, 
			 VOIDFUNCPTR routine, int parameter);
STATUS bcm1250IntDisconnect (int intrSource);
STATUS bcm1250IntEnable	(int intrSource);
STATUS bcm1250IntDisable	(int intrSource);

#else	/* __STDC__ */

STATUS bcm1250IntLibInit	();
STATUS bcm1250IntConnect	();
STATUS bcm1250IntDisconnect ();
STATUS bcm1250IntEnable	();
STATUS bcm1250IntDiable	();

#endif	/* __STDC__ */

#endif	/* _ASMLANGUAGE */

#ifdef __cplusplus
}
#endif

#endif /* __INCbcm1250IntLibh */
