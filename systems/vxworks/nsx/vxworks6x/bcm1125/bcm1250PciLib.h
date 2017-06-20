/* bcm1250PciLib.h - BCM12500 PCI and LDT support header file */

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

/* $Id: bcm1250PciLib.h,v 1.3 2011/07/21 16:14:49 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01a,15nov01,agf   written
*/

#ifndef __INCbcm1250PciLibh
#define __INCbcm1250PciLibh

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ASMLANGUAGE

#if defined(__STDC__) || defined(__cplusplus)

void sysPciConfig (void);
void sysHostBridgeInit (void);
void sysPciAutoConfig (void);

#else	/* __STDC__ */

void	sysPciConfig ();
void	sysHostBridgeInit ();
void	sysPciAutoConfig ();

#endif	/* __STDC__ */

#endif	/* _ASMLANGUAGE */

#ifdef __cplusplus
}
#endif

#endif /* __INCbcm1250PciLibh */
