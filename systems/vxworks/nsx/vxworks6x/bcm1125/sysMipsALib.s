/* sysMipsALib.s - MIPS system-dependent routines */

/* Copyright 2001 Wind River Systems, Inc. */
#include "copyright_wrs.h"

/* $Id: sysMipsALib.s,v 1.3 2011/07/21 16:14:49 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01i,08apr04,pes  Use M{TF]C0 instead of m{tf}c0 when accessing C0_TLBHI.
01h,01dec03,pes  Correct spelling in conditional compilation of sysWiredSet.
01g,05aug03,agf  add routines for TLB related operations
01f,07jun02,jmt  Fix typo with SYS_CONFIG_SET and SYS_PRID_GET macros
01e,29nov01,agf  fix sysConfigSet to load from a0 instead of v0
01e,29nov01,pes  Correct parameter passed to sysConfigSet()
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
	sysIndexSet		SYS_INDEX_SET
	sysIndexGet		SYS_INDEX_GET
	sysRandomSet		SYS_RANDOM_SET
	sysRandomGet		SYS_RANDOM_GET
	sysEntryLo0Set		SYS_ENTRYLO0_SET
	sysEntryLo0Get		SYS_ENTRYLO0_GET
	sysEntryLo1Set		SYS_ENTRYLO1_SET
	sysEntryLo1Get		SYS_ENTRYLO1_GET
	sysPageMaskSet		SYS_PAGEMASK_SET
	sysPageMaskGet		SYS_PAGEMASK_GET
	sysWiredSet		SYS_WIRED_SET
	sysWiredGet		SYS_WIRED_GET
	sysBadVaddrGet		SYS_BADVADDR_GET
	sysEntryHiSet		SYS_ENTRYHI_SET
	sysEntryHiGet		SYS_ENTRYHI_GET
	sysTlbProbe		SYS_TLB_PROBE
	sysTlbRead		SYS_TLB_READ
	sysTlbWriteIndex	SYS_TLB_WRITE_INDEX
	sysTlbWrteRandom	SYS_TLB_WRITE_RANDOM
*/
	
	.globl	sysGpInit
	.globl  sysCompareSet
	.globl  sysCompareGet
	.globl  sysCountSet
	.globl	sysCountGet
	.globl	sysPridGet
	.globl	sysConfigGet
	.globl	sysConfigSet
	.globl	sysIndexSet
	.globl	sysIndexGet
	.globl	sysRandomSet
	.globl	sysRandomGet
	.globl	sysEntryLo0Set
	.globl	sysEntryLo0Get
	.globl	sysEntryLo1Set
	.globl	sysEntryLo1Get
	.globl	sysPageMaskSet
	.globl	sysPageMaskGet
	.globl	sysWiredSet
	.globl	sysWiredGet
	.globl	sysBadVaddrGet
	.globl	sysEntryHiSet
	.globl	sysEntryHiGet
	.globl	sysTlbProbe
	.globl	sysTlbRead
	.globl	sysTlbWriteIndex
	.globl	sysTlbWrteRandom
	.globl  sysCP0StatusGet

	
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
	
#ifndef SYS_PRID_GET
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

#ifndef SYS_CONFIG_SET
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

#ifndef SYS_INDEX_SET
/******************************************************************************
*
* sysIndexSet - set the MIPS processor INDEX register
*
* RETURNS: N/A
*
* void sysIndexSet
*     (
*     UINT32 index
*     )
*
*/
        .ent        sysIndexSet
sysIndexSet:
        mtc0        a0,C0_INX
        HAZARD_TLB
        j        ra
        .end        sysIndexSet
#endif

#ifndef SYS_INDEX_GET
/******************************************************************************
*
* sysIndexGet - get the MIPS processor INDEX register
*
* RETURNS: N/A
*
* UINT32 sysIndexGet (void)
*
*/
        .ent        sysIndexGet
sysIndexGet:
        mfc0        v0,C0_INX
        HAZARD_CP_READ
        j        ra
        .end        sysIndexGet
#endif

#ifndef SYS_RANDOM_SET
/******************************************************************************
*
* sysRandomSet - set the MIPS processor RANDOM register
*
* RETURNS: N/A
*
* void sysRandomSet
*     (
*     UINT32 random
*     )
*
*/
        .ent        sysRandomSet
sysRandomSet:
        mtc0        a0,C0_RAND
        HAZARD_TLB
        .end        sysRandomSet
#endif

#ifndef SYS_RANDOM_GET
/******************************************************************************
*
* sysRandomGet - get the MIPS processor RANDOM register
*
* RETURNS: N/A
*
* UINT32 sysRandomGet (void)
*
*/
        .ent        sysRandomGet
sysRandomGet:
        mfc0        v0,C0_RAND
        HAZARD_CP_READ
        j        ra
        .end        sysRandomGet
#endif

#ifndef SYS_ENTRYLO0_SET
/******************************************************************************
*
* sysEntryLo0Set - set the MIPS processor ENTRYLO0 register
*
* RETURNS: N/A
*
* void sysEntryLo0Set
*     (
*     UINT32 entrylo
*     )
*
*/
        .ent        sysEntryLo0Set
sysEntryLo0Set:
        mtc0        a0,C0_TLBLO0
        HAZARD_TLB
        j        ra
        .end        sysEntryLo0Set
#endif

#ifndef SYS_ENTRYLO0_GET
/******************************************************************************
*
* sysEntryLo0Get - get the MIPS processor ENTRYLO0 register
*
* RETURNS: N/A
*
* UINT32 sysEntryLo0Get (void)
*
*/
        .ent        sysEntryLo0Get
sysEntryLo0Get:
        mfc0        v0,C0_TLBLO0
        HAZARD_CP_READ
        j        ra
        .end        sysEntryLo0Get
#endif

#ifndef SYS_ENTRYLO1_SET
/******************************************************************************
*
* sysEntryLo0Set - set the MIPS processor ENTRYLO1 register
*
* RETURNS: N/A
*
* void sysEntryLo1Set
*     (
*     UINT32 entrylo
*     )
*
*/
        .ent        sysEntryLo1Set
sysEntryLo1Set:
        mtc0        a0,C0_TLBLO1
        HAZARD_TLB
        j        ra
        .end        sysEntryLo1Set
#endif

#ifndef SYS_ENTRYLO1_GET
/******************************************************************************
*
* sysEntryLo1Get - get the MIPS processor ENTRYLO1 register
*
* RETURNS: N/A
*
* UINT32 sysEntryLo1Get (void)
*
*/
        .ent        sysEntryLo1Get
sysEntryLo1Get:
        mfc0        v0,C0_TLBLO1
        HAZARD_CP_READ
        j        ra
        .end        sysEntryLo1Get
#endif

#ifndef SYS_PAGEMASK_SET
/******************************************************************************
*
* sysPageMaskSet - set the MIPS processor PAGEMASK register
*
* RETURNS: N/A
*
* void sysPageMaskSet
*     (
*     UINT32 pagemask
*     )
*
*/
        .ent        sysPageMaskSet
sysPageMaskSet:
        mtc0        a0,C0_PAGEMASK
        HAZARD_TLB
        j        ra
        .end        sysPageMaskSet
#endif

#ifndef SYS_PAGEMASK_GET
/******************************************************************************
*
* sysPageMaskGet - get the MIPS processor PAGEMASK register
*
* RETURNS: N/A
*
* UINT32 sysPageMaskGet (void)
*
*/
        .ent        sysPageMaskGet
sysPageMaskGet:
        mfc0        v0,C0_PAGEMASK
        HAZARD_CP_READ
        j        ra
        .end        sysPageMaskGet
#endif

#ifndef SYS_WIRED_SET
/******************************************************************************
*
* sysWiredSet - set the MIPS processor WRIRED register
*
* RETURNS: N/A
*
* void sysWiredSet
*     (
*     UINT32 wired
*     )
*
*/
        .ent        sysWiredSet
sysWiredSet:
        mtc0        a0,C0_WIRED
        HAZARD_TLB
        j        ra
        .end        sysWiredSet
#endif

#ifndef SYS_WIRED_GET
/******************************************************************************
*
* sysWiredGet - get the MIPS processor WIRED register
*
* RETURNS: N/A
*
* UINT32 sysWiredGet (void)
*
*/
        .ent        sysWiredGet
sysWiredGet:
        mfc0        v0,C0_WIRED
        HAZARD_CP_READ
        j        ra
        .end        sysWiredGet
#endif

#ifndef SYS_BADVADDR_GET
/******************************************************************************
*
* sysBadVaddrGet - get the MIPS processor BADVADDR register
*
* RETURNS: N/A
*
* UINT32 sysBadVaddrGet (void)
*
*/
        .ent        sysBadVaddrGet
sysBadVaddrGet:
        mfc0        v0,C0_BADVADDR
        HAZARD_CP_READ
        j        ra
        .end        sysBadVaddrGet
#endif

#ifndef SYS_ENTRYHI_SET
/******************************************************************************
*
* sysEntryHiSet - set the MIPS processor ENTRYHI register
*
* RETURNS: N/A
*
* void sysEntryHiSet
*     (
*     UINT32 entryhi
*     )
*
*/
        .ent        sysEntryHiSet
sysEntryHiSet:
        MTC0        a0,C0_TLBHI
        HAZARD_TLB
        j        ra
        .end        sysEntryHiSet
#endif

#ifndef SYS_ENTRYHI_GET
/******************************************************************************
*
* sysEntryHiGet - get the MIPS processor ENTRYHI register
*
* RETURNS: N/A
*
* UINT32 sysEntryHiGet (void)
*
*/
        .ent        sysEntryHiGet
sysEntryHiGet:
        MFC0        v0,C0_TLBHI
        HAZARD_CP_READ
        j        ra
        .end        sysEntryHiGet
#endif

#ifndef SYS_TLBPROBE
/******************************************************************************
*
* sysTlbProbe - TLBP instruction
*
* RETURNS: N/A
*
* void sysTlbProbe (void)
*
*/
        .ent        sysTlbProbe
sysTlbProbe:
        tlbp
        HAZARD_CP_READ
        j        ra
        .end        sysTlbProbe
#endif

#ifndef SYS_TLBREAD
/******************************************************************************
*
* sysTlbRead - TLBR instruction
*
* RETURNS: N/A
*
* void sysTlbRead (void)
*
*/
        .ent        sysTlbRead
sysTlbRead:
        tlbr
        HAZARD_CP_READ
        j        ra
        .end        sysTlbRead
#endif

#ifndef SYS_TLB_WRITE_INDEX
/******************************************************************************
*
* sysTlbWriteIndex - TLBWI instruction
*
* RETURNS: N/A
*
* void sysTlbWriteIndex (void)
*
*/
        .ent        sysTlbWriteIndex
sysTlbWriteIndex:
        tlbwi
        HAZARD_CP_WRITE
        j        ra
        .end        sysTlbWriteIndex
#endif

#ifndef SYS_TLB_WRITE_RANDOM
/******************************************************************************
*
* sysTlbWrteRandom - TLBWR instruction
*
* RETURNS: N/A
*
* void sysTlbWrteRandom (void)
*
*/
        .ent        sysTlbWrteRandom
sysTlbWrteRandom:
        tlbwr
        HAZARD_CP_WRITE
        j        ra
        .end        sysTlbWrteRandom
#endif

/*******************************************************************************
*
* sysCP0StatusGet - get the processor STATUS register
*
* RETURNS: value of CO_CONFIG,1 register

* int sysCP0StatusGet(void)

*/
        .ent    sysCP0StatusGet
sysCP0StatusGet:
        mfc0    v0,C0_SR
        HAZARD_CP_WRITE
        j       ra
        .end    sysCP0StatusGet

