/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/* $Id: sysALib.s,v 1.2 2004/12/17 23:50:51 dkelley Exp $ */

/* sysALib.s - IDT S134 system-dependent assembly routines */

/* Copyright 1984-1996 Wind River Systems, Inc. */
        .data
        .globl  copyright_wind_river

/*
modification history
--------------------
01b,12nov99,hsm    cleaned
01a,17june99,hsm   created.
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
#include "arch/mips/ivMips.h"
#include "arch/mips/asmMips.h"
#include "arch/mips/esfMips.h"
#include "arch/mips/archMips.h"
#include "sysLib.h"
#include "config.h"
#include "cacheLib.h"             /* where CACHE_DISABLED, etc are defined; very
                                   * important for #if expression evaluation!
                                   */
	/* internals */

	.globl	sysInit			/* start of system code */
	.globl	sysGpInit		/* initialize global pointer */
	.globl	sysClearTlbEntry	/* clear entry in tlb */
	.globl	sysWbFlush		/* flush write buffers */
	.globl  sysSetCompare
	.globl  sysGetCompare
	.globl  sysSetCount
	.globl  sysGetCount
	.globl	sysGetPrid
        .globl  sysGetConfig
        .globl  setTlbEntry
        .globl  setTlbWired
        .globl  getTlbWired
        .globl  setPageSize
        .globl  sysinbyte
        .globl  sysinword
        .globl  sysinlong
        .globl  sysoutbyte
        .globl  sysoutword
        .globl  sysoutlong
        .globl EnableCache
	.globl DisableCache
   /*     .globl _writeasm */
        .globl ReadCP0_16
        .globl ReadCP0_22
        .globl ReadSR
        .globl SetSR
	.globl SetCause
	.globl GetCause
        .globl  vxSpGet
        .globl  vxPcGet


	/* externals */

	.globl	usrInit		/* system initialization routine */

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

	/* disable all interrupts */
	li	t0, SR_CU0              /* |SR_IE */
        li      t1, ~SR_IE
        and     t0, t1       
	or	t0, SR_DE		/* disable cache errors on R4200 rev1.1 */
	mtc0	t0, C0_SR		/* put on processor	*/
	la	sp, sysInit-32		/* set stack to grow down from code,
					   leave room for four parameters */

              
	/* give us as long as possible before a clock interrupt */
	li	v0,1
	mtc0	v0,C0_COUNT
	mtc0	zero,C0_COMPARE

	li	a0, BOOT_WARM_AUTOBOOT 	/* push start type arg = WARM_BOOT */
	jal	usrInit			/* never returns - starts up kernel */
	li	ra, R_VEC		/* load prom reset address */
	j	ra			/* just in case */
	.end	sysInit

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
	nop
        la      v0,sysWbFlush
        or      v0,K1BASE
        sw      zero,0(v0)
        lw      v0,0(v0)
        j       ra
	nop
	.set reorder
	.end	sysWbFlush

/*******************************************************************************
*
* sysSetCompare - set the RC32364 timer compare register
*
* RETURNS: N/A

* int sysSetCompare (void)

*/

	.ent	sysSetCompare
sysSetCompare:
	mtc0	a0,C0_COMPARE
	j	ra
	.end	sysSetCompare

/*******************************************************************************
*
* sysGetCompare - get the RC32364 timer compare register
*
* RETURNS: N/A

* int sysGetCompare (void)

*/

	.ent	sysGetCompare
sysGetCompare:
	mfc0	v0,C0_COMPARE
	j	ra
	.end	sysGetCompare

/*******************************************************************************
*
* sysSetCount - set the RC32364 timer count register
*
* RETURNS: N/A

* int sysSetCount (void)

*/

	.ent	sysSetCount
sysSetCount:
	mtc0	a0,C0_COUNT
	j	ra
	.end	sysSetCount

/*******************************************************************************
*
* sysGetCount - get the RC32364 timer count register
*
* RETURNS: N/A

* int sysGetCount (void)

*/

	.ent	sysGetCount
sysGetCount:
	mfc0	v0,C0_COUNT
	j	ra
	.end	sysGetCount

/*******************************************************************************
*
* sysGetPrid - get the RC32364 processor ID register
*
* RETURNS: N/A

* int sysGetPrid (void)

*/
	.ent	sysGetPrid
sysGetPrid:
	mfc0	v0,C0_PRID
	j	ra
	.end	sysGetPrid


#define DELAY1	300000
#define DELAY2	600000


/* Tlb Related functions */
/*****************************************************
* setPageSize      - Sets the MMU page size
*
* Input parameters -
*               a0 - PageSize
*/

        .ent setPageSize
setPageSize:
        .set noreorder
         nop
         nop
         mtc0  a0,C0_PAGEMASK
         nop
         nop
        .set reorder
         j    ra
         nop
        .end setPageSize

         
/***************************************************
* setTlbEntry      - sets one MMU entry
*
* Input parameters -
*                a0- Tlb_Inx
*                a1- Tlb_Hi
*                a2- Tlb_Lo0
*                a3- Tlb_Lo1
*/

         .ent setTlbEntry
setTlbEntry:
         .set noreorder
          nop
          nop
          mtc0 a0,C0_INX
          nop
          nop
          mtc0 a1,C0_TLBHI
          nop
          nop
          mtc0 a2,C0_TLBLO0
          nop
          nop
          mtc0 a3,C0_TLBLO1
          nop
          nop
          nop
          nop
          nop
          nop
          tlbwi
          nop
          nop
          nop
          .set reorder
 
         j      ra
         nop
         .end setTlbEntry


/***************************************************
* setTlbWired      - sets CP0 Wired register
*
* Input parameters -
*                a0- Wired
*/

         .ent setTlbWired
setTlbWired:
         .set noreorder
          nop
          nop
          mtc0 a0,C0_WIRED
          nop
          nop
          nop
          nop
          nop
          nop
          .set reorder
 
         j      ra
         nop
         .end setTlbWired


/***************************************************
* getTlbWired      - gets CP0 Wired register
*
* RETURNS: Value of CP0 Wired register
*
* INT getTlbWired (void)
*/

         .ent getTlbWired
getTlbWired:
         .set noreorder
          nop
          nop
          mfc0 v0,C0_WIRED
          nop /* I have no idea how many nops are appropriate here. */
          nop
          nop
          nop
          nop
          nop
          .set reorder
 
         j      ra
         nop
         .end getTlbWired


/*******************************************************************************
*
* sysInByte - input one byte from I/O space
*
* RETURNS: Byte data from the I/O port.
 
* UCHAR sysInByte (address)
*     int address;      /@ I/O port address @/
 
*/

          .ent sysinbyte
sysinbyte:
          .set noreorder
           lb   v0, 0x0(a0)
           nop
           j    ra
           nop
          .set reorder
          .end sysinbyte

/*******************************************************************************
*
* sysInWord - input one word from I/O space
*
* RETURNS: Word data from the I/O port.
 
* USHORT sysInWord (address)
*     int address;      /@ I/O port address @/
 
*/
           .ent  sysinword
sysinword:
          .set noreorder
           lh   v0,0x0(a0)
           j    ra
           nop
          .set reorder
          .end sysinword
/*******************************************************************************
*
* sysInLong - input one long-word from I/O space
*
* RETURNS: Long-Word data from the I/O port.
 
* USHORT sysInLong (address)
*     int address;      /@ I/O port address @/
 
*/
           .ent sysinlong
sysinlong:
           .set noreorder
            lw  v0 ,0(a0)
            nop
            j   ra
            nop
           .set reorder
           .end sysinlong
/*******************************************************************************
*
* sysoutByte - output one byte to I/O space
*
* RETURNS: N/A
 
* void sysOutByte (address, data)
*     int address;      /@ I/O port address @/
*     char data;        /@ data written to the port @/
 
*/
            .ent sysoutbyte
sysoutbyte: 
             .set noreorder
             li   t0, 0x3
             xor  a0, t0
             sb   a1,0(a0)
             nop
             j    ra
             nop
            .set reorder
            .end sysoutbyte

/*******************************************************************************
*
* sysoutWord - output one word to I/O space
*
* RETURNS: N/A
 
* void sysOutWord (address, data)
*     int address;      /@ I/O port address @/
*     short data;       /@ data written to the port @/
 
*/
             .ent sysoutword
sysoutword:
             .set noreorder
              sh  a1,0(a0)
              nop
              j   ra
              nop
             .set reorder
             .end sysoutword
/*******************************************************************************
*
* sysOutLong - output one long-word to I/O space
*
* RETURNS: N/A
 
* void sysOutLong (address, data)
*     int address;      /@ I/O port address @/
*     long data;        /@ data written to the port @/
 
*/
             .ent sysoutlong
sysoutlong:
             .set noreorder
              sw  a1,0(a0)
              nop
              j   ra
              nop
             .set reorder
             .end sysoutlong

/*****************************************************************************
*
* sysgetConfig - Returns the Config register contents
*
*
*/
             .ent sysGetConfig
sysGetConfig:
             .set noreorder
              mfc0 v0,C0_CONFIG
              nop
              nop
              j   ra
              nop
             .set reorder
             .end sysGetConfig
             

/************************************************************************/
/*     ReadCP0_16 :                                                     */
/*                                                                      */
/*      SYNTAX: unsigned long ReadCP0_16(void);                         */
/*     RETURNS: CP0 value                                               */
/*                                                                      */
/************************************************************************/
	.ent	ReadCP0_16
ReadCP0_16:
	.set	noreorder
	mfc0	v0, $16
	nop
	j	ra
	nop
	.set	reorder
	.end	ReadCP0_16


/************************************************************************
*/
/*     ReadCP0_22 :
/*                                                                      *
*/
/*      SYNTAX: unsigned long ReadCP0_22(void);                         *
*/
/*     RETURNS: CP0 value                                               *
*/
/*                                                                      *
*/
/************************************************************************
*/
	.ent	ReadCP0_22
ReadCP0_22:
	.set	noreorder
	mfc0	v0, $22
	nop
	j	ra
	nop
	.set	reorder
	.end	ReadCP0_22
        
/************************************************************************	*/
/*     ReadSR : Read the SR (Status Register)                           *	*/
/*                                                                      *	*/
/*      SYNTAX: unsigned long ReadSR(void);                             *	*/
/*     RETURNS: SR value                                                *	*/
/*                                                                      *	*/
/************************************************************************	*/
	.ent	ReadSR
ReadSR:	
	.set	noreorder
	mfc0	v0, C0_SR
	nop
	j	ra
	nop
	.set	reorder
	.end	ReadSR

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

/************************************************************************	*/
/*     SetSR  : Set the SR (Status Register)                            *	*/
/*                                                                      *	*/
/*      SYNTAX: unsigned long ReadSR(unsigned long);                    *	*/
/*     RETURNS: SR value                                                *	*/
/*                                                                      *	*/
/************************************************************************	*/
	.ent	SetSR
SetSR:	
	.set    noreorder
	mfc0    v0, C0_SR
	nop
	mtc0    a0, C0_SR
	nop
	j       ra
	nop
	.set    reorder
	.end	SetSR
        



/************************************************************************	*/
/*     ReadSR : Read the SR (Status Register)                           *	*/
/*                                                                      *	*/
/*      SYNTAX: unsigned long ReadSR(void);                             *	*/
/*     RETURNS: SR value                                                *	*/
/*                                                                      *	*/
/************************************************************************	*/
	.ent	ReadCause
ReadCause:	
	.set	noreorder
	mfc0	v0, $13
	nop
	j	ra
	nop
	.set	reorder
	.end	ReadCause

/************************************************************************	*/
/*     SetSR  : Set the SR (Status Register)                            *	*/
/*                                                                      *	*/
/*      SYNTAX: unsigned long ReadSR(unsigned long);                    *	*/
/*     RETURNS: SR value                                                *	*/
/*                                                                      *	*/
/************************************************************************	*/
	.ent	SetCause
SetCause:	
	.set    noreorder
	mfc0    v0, $13
	nop
	mtc0    a0, $13
	nop
	j       ra
	nop
	.set    reorder
	.end	SetCause

	
#define CACHEBITS       3
#define WRITETHROUGH    0
#define CACHEDISABLE    2
#define CACHEWRITEBACK  3  

/***********************************************************************/
/*                                                                     */
/*     EnableCache: enable the caches				       */
/*                                                                     */
/***********************************************************************/
	.ent	EnableCache
EnableCache:
	
	.set	noreorder

        /* Caches must be enabled from KSEG1 */        
        la      t0, 1f
        li      t1, K1BASE             
        or      t0, t1                 
        j       t0
        nop
1:

	mfc0	t1, $16
	nop

        and	t1, t1, ~(CACHEBITS)
        
#if USER_D_CACHE_MODE == CACHE_DISABLED
        or	t1, t1, CACHEDISABLE
#else        
#if USER_D_CACHE_MODE == CACHE_COPYBACK
	or	    t1, t1, CACHEWRITEBACK 
#endif
#endif

	mtc0	t1, $16	# enable D Cache
        nop
	j	ra
	nop
	.set	reorder
	.end	EnableCache
                  
/************************************************************************/
/*                                                                      */
/*     DisableCache: disable the caches				        */
/*                                                                      */
/************************************************************************/
	.ent	DisableCache
DisableCache:
	
	.set	noreorder
	mfc0	t1, $16
	nop
    and	t1, t1, ~(CACHEBITS)
    or      t1, t1, CACHEDISABLE
	mtc0	t1, $16 	# disable D Cache
	j	ra
	nop
	.set	reorder
	.end	DisableCache
        
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

