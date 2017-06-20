/* romMipsInit.s - MIPS ROM initialization module */

/* $Id: romMipsInit.s,v 1.3 2011/07/21 16:14:25 yshtil Exp $
 * Copyright 2001-2002,2005-2007 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

		
/*
DESCRIPTION
This module contains the common MIPS entry code for the VxWorks bootrom.
The entry point romInit, is the first code executed on power-up.  All MIPS
BSPs can utilize this source code for the romInit routine.  To do so 
requires the following steps.

1) Determine if the CONFIG_INIT macro is suitable for the target BSP.  It
   simply loads the macro INITIAL_CFG into the CONFIG register.  BSPs
   requiring other means of CONFIG register initialization should #define
   an alternative CONFIG_INIT macro in their romInit.s files.
2) Determine if the CONFIG1_INIT macro is suitable for the target BSP.  Some
   MIPS targets have a CONFIG 1 coprocessor register that must be initialized
   in romInit.s.  The default simply performs a noop.  BSPs requiring CONFIG 1
   initialization should #define an alternative CONFIG1_INIT macro in their
   romInit.s files.
3) Determine if the default vector table is suitable for the target BSP.  Some
   MIPS targets have a larger vector table or different vector requirements.
   To create a different vector table, define the MIPS_VECTOR_TABLE macro in
   romInit.s.  The macro should be of the form:
      #define MIPS_VECTOR_TABLE \
              RVECENT(__romInit,0)            /@ PROM entry point @/ \
              RVECENT(romReboot,1)            /@ software reboot @/ \
              RVECENT(romReserved,2) \
              RVECENT(romReserved,3) \
              ...
   If no MIPS_VECTOR_TABLE macro is defined, the default table is included.
4) Create a routine, sysMemInit(), which is called by the common romInit()
   routine and initializes memory for the system.  This routine should be
   placed in the BSP's romInit.s file.
5) Create a routine, sysCacheInit(), which is called by the common romInit()
   routine and initializes the cache for the system.  This routine should be
   placed in the BSP's romInit.s file. If your board supports auto cache
   size detection using the config1 register, then you may #define 
   ROM_AUTO_CACHE_DETECT and call romAutoCacheDetect() from sysCacheInit().
6) Create a macro, INITIAL_SR, which defines the initial Status Register
   value for the system.  This may be placed in <bsp>.h.
7) Optionally create a macro, ROM_WARM_INIT, which defines code to be
   inserted at the romWarm entry.	
8) Include the file "romMipsInit.s" at the *top* of the BSP's romInit.s file;
   just after header and macro definitions.	
*/

/* 
 * Configure the CONFIG register
 * 
 * This macro configures the config register using the INITIAL_CFG
 * value in <bsp>.h.
 *                      
 * NOTE: This macro may have already been defined by the BSP.
 */

#ifndef INITIAL_CFG_MASK
#define INITIAL_CFG_MASK	CFG_K0MASK
#endif

#ifndef CONFIG_INIT
#define CONFIG_INIT\
	.set	noreorder	; \
	mfc0	t0, C0_CONFIG	; \
	HAZARD_CP_READ		; \
	and	t0, ~INITIAL_CFG_MASK	; \
	or	t0, INITIAL_CFG	; \
	mtc0	t0, C0_CONFIG	; \
	HAZARD_CP_WRITE		; \
	.set	reorder		
#endif

/* Configure the CONFIG 1 register
 *
 * This macro configures the config 1 register available on only
 * some MIPS devices.  The default macro performs no operation.
 *
 * NOTE: This macro may have already been defined by the BSP.
 */
#ifndef	CONFIG1_INIT
#define CONFIG1_INIT ssnop
#endif

#ifdef __GNUC__
# if _WRS_MIPS_64BIT_CPU
#  if defined (_WRS_MIPS_ENABLE_R2_ISA)
#   define SET_MIPS64()   .set mips64r2
#  else
#   define SET_MIPS64()   .set mips64
#  endif
# else
#  if defined (_WRS_MIPS_ENABLE_R2_ISA)
#   define SET_MIPS32()   .set mips32r2
#  else
#   define SET_MIPS32()   .set mips32
#  endif
# endif
# define SET_MIPS0()   .set mips0
#else
# define SET_MIPS64()
# define SET_MIPS32()
# define SET_MIPS0()
#endif

/* Perform initialization at romWarm entry point */
	
#ifndef ROM_WARM_INIT
#define ROM_WARM_INIT
#endif
			
/* Relocate an address.
 * 
 * This macro is used to call routines from romInit() that are
 * outside the romInit.s file.  This routine trashes the ra
 * register.
 *
 * NOTE: This macro may have already been defined by the BSP.
 */
#ifndef RELOC
#define	RELOC(toreg,address) \
	bal	9f; \
9:; \
	la	toreg,address; \
	addu	toreg,ra; \
	la	ra,9b; \
	subu	toreg,ra
#endif

#define RVECENT(f,n) \
	b f; nop
#define XVECENT(f,bev) \
	b f; li k0,bev

	/* internals */

	.globl	romInit			/* start of system code */
	.globl	romWatchInit		/* watchpoint register initializtion */

	/* externals */

	.extern	romStart		/* system initialization routine */
 
	.data

	/* ensure data segment is 16-byte aligned */

	.align 	4
_sdata:
	.asciiz	"start of data"

	.text
	.set	noreorder
promEntry:
romInit:
_romInit:
#ifdef MIPS_VECTOR_TABLE
	MIPS_VECTOR_TABLE

#else /* MIPS_VECTOR_TABLE */
	RVECENT(__romInit,0)		/* PROM entry point */
	RVECENT(romReboot,1)		/* software reboot */
	RVECENT(romReserved,2)
	RVECENT(romReserved,3)
	RVECENT(romReserved,4)
	RVECENT(romReserved,5)
	RVECENT(romReserved,6)
	RVECENT(romReserved,7)
	RVECENT(romReserved,8)
	RVECENT(romReserved,9)
	RVECENT(romReserved,10)
	RVECENT(romReserved,11)
	RVECENT(romReserved,12)
	RVECENT(romReserved,13)
	RVECENT(romReserved,14)
	RVECENT(romReserved,15)
	RVECENT(romReserved,16)
	RVECENT(romReserved,17) 
	RVECENT(romReserved,18)
	RVECENT(romReserved,19)
	RVECENT(romReserved,20)
	RVECENT(romReserved,21)
	RVECENT(romReserved,22)
	RVECENT(romReserved,23)
	RVECENT(romReserved,24)
	RVECENT(romReserved,25)
	RVECENT(romReserved,26)
	RVECENT(romReserved,27)
	RVECENT(romReserved,28)
	RVECENT(romReserved,29)
	RVECENT(romReserved,30)
	RVECENT(romReserved,31)
	RVECENT(romReserved,32)
	RVECENT(romReserved,33)
	RVECENT(romReserved,34)
	RVECENT(romReserved,35)
	RVECENT(romReserved,36)
	RVECENT(romReserved,37)
	RVECENT(romReserved,38)
	RVECENT(romReserved,39)
	RVECENT(romReserved,40)
	RVECENT(romReserved,41)
	RVECENT(romReserved,42)
	RVECENT(romReserved,43)
	RVECENT(romReserved,44)
	RVECENT(romReserved,45)
	RVECENT(romReserved,46)
	RVECENT(romReserved,47)
	RVECENT(romReserved,48)
	RVECENT(romReserved,49)
	RVECENT(romReserved,50)
	RVECENT(romReserved,51)
	RVECENT(romReserved,52)
	RVECENT(romReserved,53)
	RVECENT(romReserved,54)
	RVECENT(romReserved,55)
	RVECENT(romReserved,56)
	RVECENT(romReserved,57)
	RVECENT(romReserved,58)
	RVECENT(romReserved,59)
	RVECENT(romReserved,60)
	RVECENT(romReserved,61)
	RVECENT(romReserved,62)
	RVECENT(romReserved,63)	
	XVECENT(romExcHandle,0x200)	/* bfc00200: tlbmiss vector */
	RVECENT(romReserved,65)
	RVECENT(romReserved,66)
	RVECENT(romReserved,67)
	RVECENT(romReserved,68)
	RVECENT(romReserved,69)
	RVECENT(romReserved,70)
	RVECENT(romReserved,71)
	RVECENT(romReserved,72)
	RVECENT(romReserved,73)
	RVECENT(romReserved,74)
	RVECENT(romReserved,75)
	RVECENT(romReserved,76)
	RVECENT(romReserved,77)
	RVECENT(romReserved,78)
	RVECENT(romReserved,79)	
	XVECENT(romExcHandle,0x280)	/* bfc00280: xtlbmiss vector */
	RVECENT(romReserved,81)
	RVECENT(romReserved,82)
	RVECENT(romReserved,83)
	RVECENT(romReserved,84)
	RVECENT(romReserved,85)
	RVECENT(romReserved,86)
	RVECENT(romReserved,87)
	RVECENT(romReserved,88)
	RVECENT(romReserved,89)
	RVECENT(romReserved,90)
	RVECENT(romReserved,91)
	RVECENT(romReserved,92)
	RVECENT(romReserved,93)
	RVECENT(romReserved,94)
	RVECENT(romReserved,95)	
	XVECENT(romExcHandle,0x300)	/* bfc00300: cache vector */
	RVECENT(romReserved,97)
	RVECENT(romReserved,98)
	RVECENT(romReserved,99)
	RVECENT(romReserved,100)
	RVECENT(romReserved,101)
	RVECENT(romReserved,102)
	RVECENT(romReserved,103)
	RVECENT(romReserved,104)
	RVECENT(romReserved,105)
	RVECENT(romReserved,106)
	RVECENT(romReserved,107)
	RVECENT(romReserved,108)
	RVECENT(romReserved,109)
	RVECENT(romReserved,110)
	RVECENT(romReserved,111)
	XVECENT(romExcHandle,0x380)	/* bfc00380: general vector */
	RVECENT(romReserved,113)
	RVECENT(romReserved,114)
	RVECENT(romReserved,115)
	RVECENT(romReserved,116)
	RVECENT(romReserved,117)
	RVECENT(romReserved,118)
	RVECENT(romReserved,119)
	RVECENT(romReserved,120)
	RVECENT(romReserved,121)
	RVECENT(romReserved,122)
	RVECENT(romReserved,123)
	RVECENT(romReserved,124)
	RVECENT(romReserved,125)
	RVECENT(romReserved,126)
	RVECENT(romReserved,127)

	/* We hope there are no more reserved vectors!
	 * 128 * 8 == 1024 == 0x400
	 * so this is address R_VEC+0x400 == 0xbfc00400
	 */

#endif /* MIPS_VECTOR_TABLE */
	
	.align 4
	.set	reorder
/******************************************************************************
*
* romWatchInit - clear out the watch register(s)
*
* romWatchInit
*     (
*     void
*     )
*/
    .ent romWatchInit
romWatchInit:
#if _WRS_MIPS_64BIT_CPU
    SET_MIPS64()
#else
    SET_MIPS32()
#endif
    /* clear at least one pair of watch registers (1) */
    mtc0    zero,C0_WATCHHI,0
    MTC0    zero,C0_WATCHLO,0
    HAZARD_CP_WRITE

    /* determine if MIPS32 MIPS64 hw compliant breakpoint */
    mfc0   t3,C0_CONFIG,1
    HAZARD_CP_READ
    li     t2,CFG1_WR
    and    t3,t3,t2
    beqz   t3,1f
    /* determine if have more watch resister pairs (2) */
    mfc0   t3,C0_WATCHHI,0
    HAZARD_CP_READ
    li     t2,CFG_CM
    and    t3,t3,t2
    beqz   t3,1f
    /* clear next watch register pair */
    mtc0    zero,C0_WATCHHI,1
    MTC0    zero,C0_WATCHLO,1
    HAZARD_CP_WRITE
    /* determine if have more watch resister pairs (3)*/
    mfc0   t3,C0_WATCHHI,1
    HAZARD_CP_READ
    li     t2,CFG_CM
    and    t3,t3,t2
    beqz   t3,1f
    /* clear next watch register pair */
    mtc0    zero,C0_WATCHHI,2
    MTC0    zero,C0_WATCHLO,2
    HAZARD_CP_WRITE
    /* determine if have more watch resister pairs (4)*/
    mfc0   t3,C0_WATCHHI,2
    HAZARD_CP_READ
    li     t2,CFG_CM
    and    t3,t3,t2
    beqz   t3,1f
    /* clear next watch register pair */
    mtc0    zero,C0_WATCHHI,3
    MTC0    zero,C0_WATCHLO,3
    HAZARD_CP_WRITE
    /* determine if have more watch resister pairs (5)*/
    mfc0   t3,C0_WATCHHI,3
    HAZARD_CP_READ
    li     t2,CFG_CM
    and    t3,t3,t2
    beqz   t3,1f
    /* clear next watch register pair */
    mtc0    zero,C0_WATCHHI,4
    MTC0    zero,C0_WATCHLO,4
    HAZARD_CP_WRITE
    /* determine if have more watch resister pairs (6)*/
    mfc0   t3,C0_WATCHHI,4
    HAZARD_CP_READ
    li     t2,CFG_CM
    and    t3,t3,t2
    beqz   t3,1f
    /* clear next watch register pair */
    mtc0    zero,C0_WATCHHI,5
    MTC0    zero,C0_WATCHLO,5
    HAZARD_CP_WRITE
    /* determine if have more watch resister pairs (7)*/
    mfc0   t3,C0_WATCHHI,5
    HAZARD_CP_READ
    li     t2,CFG_CM
    and    t3,t3,t2
    beqz   t3,1f
    /* clear next watch register pair */
    mtc0    zero,C0_WATCHHI,6
    MTC0    zero,C0_WATCHLO,6
    HAZARD_CP_WRITE

   /* determine if have more watch resister pairs (8)*/
    mfc0   t3,C0_WATCHHI,6
    HAZARD_CP_READ
    li     t2,CFG_CM
    and    t3,t3,t2
    beqz   t3,1f
    /* clear next watch register pair */
    mtc0    zero,C0_WATCHHI,7
    MTC0    zero,C0_WATCHLO,7
    HAZARD_CP_WRITE

1:
    SET_MIPS0()

    j	ra

    .end romWatchInit

/******************************************************************************
*
* romInit - entry point for VxWorks in ROM
*

* romInit 
*     (
*     int startType
*     )
*/

__romInit:
	/* force power-on startType */
	li	a0, BOOT_CLEAR

	/*
	 * If there was some way to distinguish between a cold and warm
	 * restart AND the memory system is guaranteed to be intact then
	 * we could load BOOT_NORMAL instead
	*/

romReboot:				/* sw reboot inherits a0 startType */

	move	s0, a0			/* save startType */

	/* clear software interrupts */
	
	mtc0	zero, C0_CAUSE
        bal     romWatchInit

	/* initialize status register */
	
	li	t0, INITIAL_SR
	mtc0	t0, C0_SR

	/* give us as long as possible before a clock interrupt */

	li	t0, 1
	mtc0	t0, C0_COUNT
	mtc0	zero, C0_COMPARE

	/* setup CONFIG registers using pre-defined macro */

	CONFIG_INIT
	CONFIG1_INIT

#ifndef _WRS_MIPS_NO_MMU
	/* clear TLB */

	mtc0	zero, C0_WIRED
	HAZARD_CP_WRITE
/* 
 * Note: Do not replace the 'bal' with 'jal' here. The resulting
 *       target address is not PIC.
 */
	bal	romMipsTlbClear
#endif
	
	/* absolutely basic initialization to allow things to continue */
        bal     board_draminfo

	bal	sysMemInit
	bal	sysCacheInit

        bal     romClearEdac   

	andi	t0, s0, BOOT_CLEAR
	beqz	t0, romWarm

romWarm:
	
#if (ROM_TEXT_ADRS == 0x9fc00000)
	/* Switch to cached space so that copying ROM into RAM is faster */
	RELOC(t0,0f)
	and	t0, ~0x20000000
	jal	t0
0:
#endif

	/* Set stack to grow down from beginning of data and call init          */

	la	gp, _gp				/* set global ptr from compiler */
	la	sp, STACK_ADRS-(4*_RTypeSize)	/* set stack to begin of data   */
	move	a0, s0				/* push arg = start type        */
	sync					/* flush any last-minute writes */
	RELOC(t0, romStart)
	jal	t0				/* starts kernel, never returns */

	j	ra                  /* just in case */

1:		
	b	1b

		
/***************************************************************************
*
* romReserved -	 Handle a jump to an unknown vector
*
*
* 
*/

	.ent	romReserved
romReserved:
	b	romInit	    /* just start over */
	.end	romReserved

/***************************************************************************
* NON-DEFAULTED OPTIONAL UTILITIES
*
* The following utilities are not included by default, and there is
* no default definition for them. You must #define a macro to get them 
* pulled in. This is the opposite of the convention used by other code 
* in this file (such as CONFIG_INIT, ROM_WARM_INIT), where a default
* definition is provided and you have to #define the macro to
* override it.	
*/
			
#if defined ROM_AUTO_CACHE_DETECT
	
/***************************************************************************
*
* romAutoCacheDetect - Determine caches sizes from config1
*
* Recent MIPS processors carry enough information in the
* CONFIG1 register to identify the cache size and line size of the
* data and instruction caches. since this information may change
* from one variant to another, the only safe way to know how to 
* specify the cache sizes is to read the CONFIG1 fields and act 
* accordingly. 
*  
*   The instruction cache information is:	
*	line size  = IL	
*	cache size = IS * IL * IA
*
*   Similarly, the data cache information is:
*	line size  = DL
*	cache size = DS * DL * DA
*
*   The following processing is performed on the fields coming from
*   CONFIG1 to translate the bit patterns into the correct values:
*
*   IS, DS: (number of cache lines)
*	64 shifted left by n, where 'n' is the value from the
*	IS or DS field in CONFIG1. Technically, if 'n' is 7, the
*	value is invalid (reserved), but since this should not happen,
*	we don't test for it. The result implements the following table:
*
*	n	value
*	0	64
*	1	128
*	2	256
*	3	512
*	4	1024
*	5	2048
*	6	4096
*	7	8192	(invalid)
*
*    IL, DL (number of bytes per cache line)
*	2 shifted left by n, where 'n' is the value from the
*	IL or DL field in CONFIG1. If the result is '2', the value
*	is set to 0, because if IL or DL is 0, there is no cache.
*	also, there is an invalid value calculated if 'n' is 7, since
*	that value is reserved. But since it should not happen, we don't
*	test for it. The result implements the following table:
*
*	n	value
*	0	0	(after adjustment)
*	1	4
*	2	8
*	3	16
*	4	32
*	5	64
*	6	128
*	7	256	(invalid)
*
*    IA, DA (number of cache ways)
*	n + 1, where 'n' is the value from the IA or DA field of the
*	CONFIG1 register. This implements the following table:
*
*	n	value
*	0	1	(direct mapped)
*	1	2
*	2	3
*	3	4
*	4	5
*	5	6
*	6	7
*	7	8
*
* RETURNS:
*	
* t0:	  I-cache size 
* t1:	  I-cache line size 
* t2:	  D-cache size 
* t3:	  D-cache line size
*
* NOMANUAL
*/

	.ent romAutoCacheDetect
romAutoCacheDetect:

#define MSK(n)				((1 << (n)) - 1)
	
/******** reg: CONFIG1 *******/
#define MIPS_CONFIG1_DA_SHF		7
#define MIPS_CONFIG1_DA_MSK		(MSK(3) << MIPS_CONFIG1_DA_SHF)
#define MIPS_CONFIG1_DL_SHF		10
#define MIPS_CONFIG1_DL_MSK		(MSK(3) << MIPS_CONFIG1_DL_SHF)
#define MIPS_CONFIG1_DS_SHF		13
#define MIPS_CONFIG1_DS_MSK		(MSK(3) << MIPS_CONFIG1_DS_SHF)

#define MIPS_CONFIG1_IA_SHF		16
#define MIPS_CONFIG1_IA_MSK		(MSK(3) << MIPS_CONFIG1_IA_SHF)
#define MIPS_CONFIG1_IL_SHF		19
#define MIPS_CONFIG1_IL_MSK		(MSK(3) << MIPS_CONFIG1_IL_SHF)
#define MIPS_CONFIG1_IS_SHF		22
#define MIPS_CONFIG1_IS_MSK		(MSK(3) << MIPS_CONFIG1_IS_SHF)

		
	/* read the config1 register to determine the cache parameters */
	/* N.B.: the mfc0 instruction to access config1 is not supported by
	   the assembler, so we have to synthesize it. */
/*	mfc0	v0,C0_CONFIG,1 */
	.word	0x40028001

	/* calculate instruction cache parameters */
	lui	v1,(MIPS_CONFIG1_IS_MSK >> 16)
	and	v1,v1,v0
	srl	v1,v1,MIPS_CONFIG1_IS_SHF
	li	t0,64
	sll	t0,v1		/* number of cache lines */

	lui	v1,(MIPS_CONFIG1_IA_MSK >> 16)
	and	v1,v1,v0
	srl	v1,v1,MIPS_CONFIG1_IA_SHF
	add	t1,v1,1		/* number of cache ways */
	mul	t0,t0,t1	/* t0 = IS * IA */

	lui	v1,(MIPS_CONFIG1_IL_MSK >> 16)
	and	v1,v1,v0
	srl	v1,v1,MIPS_CONFIG1_IL_SHF
	li	t1,2
	sll	t1,v1	/* t1 = IL */
	bnez	t1,1f
	li	t1,0
1:
	mul	t0,t0,t1	/* t0 = IS * IA * IL */
	
	/* calculate data cache parameters */
	andi	v1,v0,MIPS_CONFIG1_DS_MSK
	srl	v1,v1,MIPS_CONFIG1_DS_SHF
	li	t2,64
	sll	t2,v1		/* number of cache lines */

	andi	v1,v0,MIPS_CONFIG1_DA_MSK
	srl	v1,v1,MIPS_CONFIG1_DA_SHF
	add	t3,v1,1		/* number of cache ways */
	mul	t2,t2,t3	/* t2 = DS * DA */
	
	andi	v1,v0,MIPS_CONFIG1_DL_MSK
	srl	v1,v1,MIPS_CONFIG1_DL_SHF
	li	t3,2
	sll	t3,v1	/* t3 = DL */
	bnez	t3,1f
	li	t3,0
1:		
	mul	t2,t2,t3	/* t2 = DS * DA * DL */

	/* t0..t3 are now loaded with cache parameters */

	/* return */
	jr	ra
	.end romAutoCacheDetect

#endif /* ROM_AUTO_CACHE_DETECT */


#ifndef _WRS_MIPS_NO_MMU
/***************************************************************************
*
* romMipsTlbClear -	 Clear the TLB by invalidating each entry
*
*
* Uses:		 a0, a2, a3, v0, t0-t9, ra
* Stack:	 None
*
* NOTE:	A near-clone of this function exists in the 
*       target/src/arch/mips/palMipsALib.s file, to avoid the need for a
*       'bal' to a label that can be too far for a 16-bit signed offset.
*       Any changes in this function should be coordinated in palMipsALib.s.
*
*/

	.ent	romMipsTlbClear
	.globl	romMipsTlbClear
romMipsTlbClear:
	move	a3, ra			/* save return address */
	bal	romMipsTlbSizeGet
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
	
	sll	t1, a3, MMU_R4K_PAGE_SHIFT+1	/* Valid bit set to zero */
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
	.end	romMipsTlbClear

/*************************************************************************
*
* romMipsTlbSizeGet - determine size of TLB
*
* A hueuristic algorithm is used. 
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
* It is assumed that the TLB size is a multiple of 8. Hence, the max 
* C0_RAND value determined by the hueuristic algorithm is rounded up. 
* This should not be strictly necessary. It is done to account for 
* the possibility that the heuristic algorithm does not converge all the 
* way to a max value.
*
* NOTE:	A near-clone of this function exists in the 
*       target/src/arch/mips/palMipsALib.s file, to avoid the need for a
*       'bal' to a label that can be too far for a 16-bit signed offset.
*       Any changes in this function should be coordinated in palMipsALib.s.
*
* int romMipsTlbSizeGet (void)

* RETURNS: size of TLB.
*/

	.globl	romMipsTlbSizeGet
	.ent	romMipsTlbSizeGet

romMipsTlbSizeGet:
        /* Read the config1 register to determine the max TLB index. */
        mfc0    v0,C0_CONFIG,1
        HAZARD_CP_READ

        and     v0, CFG1_MMUMASK
        srl     v0, CFG1_MMUSHIFT

exit:
	addu    v0, 1           /* Turn max index into count */
	jr	ra
	.end	romMipsTlbSizeGet
	
#endif /* _WRS_MIPS_NO_MMU */
