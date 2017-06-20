/* sysMipsALib.s - MIPS system-dependent routines */

/* Copyright 2001 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/* $Id: sysMipsALib.s,v 1.3 2011/07/21 16:14:44 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01e,29nov01,agf  fix sysConfigSet to load from a0 instead of v0
01d,16jul01,tlc  Add CofE copyright.
01c,27jun01,tlc  General cleanup.
01b,21jun01,agf  fix typos in comments
01a,15jun01,tlc  Use HAZARD_VR5400 macro.
*/

/*
DESCRIPTION
This library provides board-specific routines that are shared by all MIPS-based
BSPs.  MIPS BSPs utilize this file by creating a symbolic link from their
directory to target/config/mipsCommon/sysMipsALib.s and include the file at the 
*bottom* of sysALib.s using

	#include "sysMipsALib.s"
	
A list of provided routines follows.  If a BSP requires a specialized routine,
then #define the appropriate MACRO corresponding to the routine to be
specialized in the BSPs sysALib.s file.

	 ROUTINE		  MACRO
	------------------------------------------------------
	sysGpInit		SYS_GP_INIT
	sysCompareSet		SYS_COMPARE_SET
	sysCompareGet		SYS_COMPARE_GET
	sysCountSet		SYS_COUNT_SET
	sysCountGet		SYS_COUNT_GET
	sysPridGet		SYS_PRID_GET
	sysConfigGet		SYS_CONFIG_GET
	sysConfigSet		SYS_CONFIG_SET
*/
	
	.globl	sysGpInit
	.globl  sysCompareSet
	.globl  sysCompareGet
	.globl  sysCountSet
	.globl	sysCountGet
	.globl	sysPridGet
	.globl	sysConfigGet
	.globl	sysConfigSet
	
	.text

#ifndef SYS_GP_INIT
/*******************************************************************************
*
* sysGpInit - initialize the MIPS global pointer
*
* The purpose of this routine is to initialize the global pointer (gp).
* It is required in order support compressed ROMs.
*
* RETURNS: N/A
*
* NOMANUAL
*/

	.ent	sysGpInit
sysGpInit:
	la	gp, _gp			/* set global pointer from compiler */
	j	ra
	.end	sysGpInit

#endif

#ifndef SYS_COMPARE_SET	
/******************************************************************************
*
* sysCompareSet - set the MIPS timer compare register
*
* RETURNS: N/A

* void sysCompareSet
*     (
*     int compareValue
*     )

* NOMANUAL
*/

	.ent	sysCompareSet
sysCompareSet:
	HAZARD_VR5400
	mtc0	a0,C0_COMPARE
	j	ra
	.end	sysCompareSet
#endif

#ifndef SYS_COMPARE_GET	
/******************************************************************************
*
* sysCompareGet - get the MIPS timer compare register
*
* RETURNS: The MIPS timer compare register value

* int sysCompareGet (void)

* NOMANUAL	
*/

	.ent	sysCompareGet
sysCompareGet:
	HAZARD_VR5400
	mfc0	v0,C0_COMPARE
	j	ra
	.end	sysCompareGet
#endif

#ifndef SYS_COUNT_SET		
/******************************************************************************
*
* sysCountSet - set the MIPS timer count register
*
* RETURNS: N/A

* void sysCountSet
*     (
*     int countValue
*     )

* NOMANUAL	
*/

	.ent	sysCountSet
sysCountSet:
	HAZARD_VR5400
	mtc0	a0,C0_COUNT
	j	ra
	.end	sysCountSet
#endif

#ifndef SYS_COUNT_GET
/******************************************************************************
*
* sysCountGet - get the MIPS timer count register
*
* RETURNS: The MIPS timer count register value
*
* int sysCountGet (void)
*
* NOMANUAL	
*/

	.ent	sysCountGet
sysCountGet:
	HAZARD_VR5400
	mfc0	v0,C0_COUNT
	j	ra
	.end	sysCountGet
#endif
	
#ifndef SYS_GET_PRID
/******************************************************************************
*
* sysPridGet - get the MIPS processor ID register
*
* RETURNS: N/A

* int sysPridGet (void)

*/
	.ent	sysPridGet
sysPridGet:
	HAZARD_VR5400
	mfc0	v0,C0_PRID
	j	ra
	.end	sysPridGet
#endif

#ifndef SYS_CONFIG_GET				
/******************************************************************************
*
* sysConfigGet - get the MIPS processor CONFIG register
*
* RETURNS: N/A

* int sysConfigGet (void)

*/
	.ent	sysConfigGet
sysConfigGet:
	HAZARD_VR5400
	mfc0	v0,C0_CONFIG
	j	ra
	.end	sysConfigGet
#endif

#ifndef SYS_SET_CONFIG				
/******************************************************************************
*
* sysConfigSet - set the MIPS processor CONFIG register
*
* RETURNS: N/A

* int sysConfigSet (void)

*/
	.ent	sysConfigSet
sysConfigSet:
	HAZARD_VR5400
	mtc0	a0,C0_CONFIG
	j	ra
	.end	sysConfigSet
#endif

