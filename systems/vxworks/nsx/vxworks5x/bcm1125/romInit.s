
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

/* $Id: romInit.s,v 1.3 2011/07/21 16:14:43 yshtil Exp $
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
This module contains the entry code for the VxWorks bootrom.
The entry point romInit, is the first code executed on power-up.

The routine sysToMonitor() jumps to romInit() to perform a
"warm boot".

*/


	/* includes */
#define _ASMLANGUAGE
#include "vxWorks.h"
#include "sysLib.h"
#include "cacheLib.h"
#include "config.h"
#include "asm.h"
#include "esf.h"

#ifdef _SENTOSA_
#include "sentosa.h"
#elif defined _RHONE_
#include "rhone.h"
#else
#include "swarm.h"
#endif
#include "bcm1250Lib.h"

#ifndef _RHONE_
#define  CFG_MULTI_CPUS  1
#endif

	/* defines */
#define ROM_ISP_BASE    0xa0010000

#define LED_CHAR0       (32+8*3)
#define LED_CHAR1       (32+8*2)
#define LED_CHAR2       (32+8*1)
#define LED_CHAR3       (32+8*0)

	/* macros */

/* in archMips.h these macros are not assembler friendly, so fix for here */
#undef	PHYS_TO_K0
#define	PHYS_TO_K0(x)	(K0BASE | (x))
#undef	PHYS_TO_K1
#define	PHYS_TO_K1(x)	(K1BASE | (x))

/* 
 * Relocate an address.
 * 
 * This macro is used to call routines in a position independent
 * manner. That way code can be relocated from ROM to RAM and 
 * still work properly. Note this will overwrite the ra register.
*/
#define	RELOC(toreg,address) \
	bal	9f; \
9:; \
	la	toreg,address; \
	addu	toreg,ra; \
	la	ra,9b; \
	subu	toreg,ra

/*
 * Set a vector in a jump table
 */
#define RVECENT(f,n) \
	b f; nop
#define XVECENT(f,bev) \
	b f; li k0,bev

/*
 * Set the on-board LED display.
 *
 *  Input parameters:
 *      a,b,c,d - four ASCII characters (literal constants)
 */
#ifndef _SENTOSA_
#define SETLEDS(a,b,c,d)                                 \
       li     a0,(((a)<<24)|((b)<<16)|((c)<<8)|(d)) ;    \
               li      t0,PHYS_TO_K1(LEDS_PHYS) ;        \
                rol     a0,a0,8 ;                        \
                and     t1,a0,0xFF ;                     \
                sb      t1,LED_CHAR0(t0) ;               \
                rol     a0,a0,8 ;                        \
                and     t1,a0,0xFF ;                     \
                sb      t1,LED_CHAR1(t0) ;               \
                rol     a0,a0,8 ;                        \
                and     t1,a0,0xFF ;                     \
                sb      t1,LED_CHAR2(t0) ;               \
                rol     a0,a0,8 ;                        \
                and     t1,a0,0xFF ;                     \
                sb      t1,LED_CHAR3(t0)
#else
#define SETLEDS(a,b,c,d)  
#endif

/*
 *     RELOC(k1, sysLedDsply) ;                          \
 *     jal   k1
 */

	/* internals */

	.globl	romInit			/* start of system code */

	/* externals */

	.extern	romStart		/* system initialization routine */
	.extern cacheSb1Reset		/* L1 cache reset */
	.extern sysLedDsply		/* swarm board LED display routine */
 
	.data

	/* ensure data segment is 16-byte aligned */

	.align 	4
_sdata:
	.asciiz	"start of data"

	.text
promEntry:
romInit:
_romInit:
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
	XVECENT(romExcHandle,0x200)	/* bfc00200: R4000 tlbmiss vector */
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
	XVECENT(romExcHandle,0x280)	/* bfc00280: R4000 xtlbmiss vector */
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
	XVECENT(romExcHandle,0x300)	/* bfc00300: R4000 cache vector */
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
	XVECENT(romExcHandle,0x380)	/* bfc00380: R4000 general vector */
	RVECENT(romReserved,113)
	RVECENT(romReserved,114)
	RVECENT(romReserved,115)
	RVECENT(romReserved,116)
	RVECENT(romReserved,116)
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
	
/* XXX - bcm1250 has two additional vectors:
 *     XVECENT(romExcHandle,0x400)	/@ bfc00400: SB1 interrupt vector @/
 *     XVECENT(romExcHandle,0x400)	/@ bfc00480: SB1 EJTAG vector     @/
 */

	.align 4

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

         /* move startType to s6 to keep it 'safe' */
                move  s6, a0

        /*
         * Test to see if which cpu is executing since they
         * both share the 0xbfc0.0000 reset vector. Make the
         * secondary cpu(s) use a different bring-up sequence.
         */
#if CFG_MULTI_CPUS
                mfc0  t0, C0_PRID               /* get cpu PRID */
                HAZARD_CP_READ
                and   t0, t0, 0xe000000         /* determine cpu number */
                beq   t0, zero, 1f              /* skip if primary cpu */

                RELOC(k1, alt_romInit)          /* cpu1 will not return */
                jal   k1
                nop
1:
        /*
         * In case cpu0 got here from a sw reboot [as opposed to hw reset]
         * reset cpu1 to make sure cpu0 and cpu1 stay sync-ed.
         *
         * This is may not always be necessary, application developers may
         * wish to implement a more graceful reboot sequence.
         */
                li      a0,PHYS_TO_K1(A_SCD_SYSTEM_CFG)
                ld      t0,0(a0)
                dli     t1,M_SYS_CPU_RESET_1    # Reset mask
                or      t0,t1                   # New value to write
                sd      t0,0(a0)                # cpu1 is now in reset
#endif

        /*
         * Do low-level board initialization.  Only needed
         * once, so have cpu0 do it.
         */

                RELOC(k1, board_earlyinit)
                jal   k1

                SETLEDS('H','E','L','O')

        /*
         * Initialize cpu0's core 
         */

                RELOC(k1, core_init)
                jal   k1

        /*
         * Do low-level processor initialization. Again, only
         * needed once, so have cpu0 do it.
         */

                RELOC(k1, processor_init)
                jal   k1

#if CFG_MULTI_CPUS
        /*
         * Release secondary core(s)
         *
         * cpu0 will take the secondary out of reset; then wait
         * until the secondary signals for it to continue.
         *
         * The secondary will start at _romInit, hit the 
         * 'alt_romInit' routine and go with that bring-up
         * sequence. Eventually, it will get to a stopping point
         * and then will signal for cpu0 to continue.
         */

                SETLEDS('a','l','t','C')
                RELOC(k1, start_altCpu)
                jal   k1
#endif

        /*
         * Switch from KSEG1 to KSEG0
         */

                bal     cpu_kseg0_switch

        /*
         * Now initialize the DRAM
         */
                SETLEDS('D','R','A','M')
                RELOC(k1, board_draminfo)
                jal   k1

                SETLEDS('D','R','A','2')
                move   a0,v0                 /* pass params from draminfo */
                RELOC(k1, bcm1250_dram_init)
                jal   k1
                move  k0,v0                  /* Save in k0 for now */

                SETLEDS('D','R','A','3')
                bal 	dram_check           
                beq 	v0, zero, set_count_compare

                SETLEDS('D','R','A','4')
1:             
                b 	1b
                
         /*
          * Finally, do the typical romInit stuff and go to romStart
          */
set_count_compare:
                SETLEDS('r','o','m','S')
                li      t0, 1
                mtc0    t0, C0_COUNT
                mtc0    zero, C0_COMPARE

                la      gp, _gp
                la      sp, STACK_ADRS-(4*_RTypeSize)   /* set stack to begin of data   */
                move    a0, s6                  /* restore startType */

                sync

                RELOC(t0,romStart)
                jal     t0                      /* should not return */
                nop

                SETLEDS('e','r','r',' ')        /* error in starting kernel */
1:		
		b	1b                      /* should never get here */

		
/***************************************************************************
*
* alt_romInit - initialization sequence for the secondary CPU's
*
*  Do the initialization of the local core and then notify cpu0 that 
*  we're done.
* 
*  This routine is executes in KSEG1.
*
*  Input parameters:
*      t0 - CPU identifier
*
*  Return value:
*      nothing
*/

	.ent	alt_romInit
alt_romInit:

        /*
         * cpu0 just entered start_altCpu and released us from RESET. It will
         * wait there until we ring its doorbell.
         *
         * Initialize core
         */

                RELOC(k1, core_init)
                jal   k1

        /*
         * Notify the SCD that we're done initializing.  Do this by
         * ringing cpu0's doorbell.
         */

                la      a0,PHYS_TO_K1(A_IMR_REGISTER(0,R_IMR_MAILBOX_SET_CPU))

                mfc0    t0,C0_PRID              # get processor number
                srl     t0,t0,25                # shift CPU bits into low
                and     t0,t0,7                 # keep only low 3 bits
                li      t1,1                    # make a mask unique for a cpu
                sll     t1,t1,t0                # calc t1 = 1 shl cpu num
                sd      t1,0(a0)                # set corrsp bit in mailbox

        /*
         * Now wait for cpu0 to ring *our* doorbell
         * The doorbell is our signal that it's safe to go to application code.
         *
         * Until cpu0 rings our doorbell, we can't use memory (but we can use 
         * the cache).
         *
         * Very BCM1250 specific here.
         */

        /*
         * Switch from KSEG1 to KSEG0
         */

                bal     cpu_kseg0_switch

        /*
         * Set the cache mode to COHERENT
         */

                mfc0    t1,C0_CONFIG
                HAZARD_CP_READ
                srl     t1,3
                sll     t1,3
                or      t1,CFG_K0_COHERENT
                mtc0    t1,C0_CONFIG


        /*
         * Wait on doorbell
         */

1:              la      a0,PHYS_TO_K1(A_IMR_REGISTER(1,R_IMR_MAILBOX_CPU))
                ld      t2,(a0)                 # Read mailbox
                beq     t2,zero,1b              # Loop till the bit is set

        /*
         * Clear all the bits in the mailbox register to dismiss the
         * pending interrupt
         */
                la      a0,PHYS_TO_K1(A_IMR_REGISTER(1,R_IMR_MAILBOX_CLR_CPU))
                li      t1,-1
                sd      t1,0(a0)

        /* t2 has the kernel vector, do some final init then jump to it */

                SETLEDS('a','l','t','G')

                jal     t2
                nop

                SETLEDS('a','l','t','F')
1:
                b       1b


	.end	alt_romInit


/***************************************************************************
*
* start_altCpu - initialization sequence for the secondary CPU's
*
* This routine is executed by the primary core. It releases the secondary(s)
* from RESET then waits for them to finish their init sequence to the 
* point of L1 cache initalization. Once the secondary(s) are finished, they
* will ring the primary's door bell so it can return.
* 
*  Input parameters:
*      nothing
*
*  Return value:
*      nothing
*/

	.ent	start_altCpu
start_altCpu:
        /*
         * Clear out our mailbox registers (both CPUs)
         */

                la      a0,PHYS_TO_K1(A_IMR_REGISTER(0,R_IMR_MAILBOX_CLR_CPU))
                dli     t0,-1                   # clear all 64 bits
                sd      t0,(a0)
                la      a0,PHYS_TO_K1(A_IMR_REGISTER(1,R_IMR_MAILBOX_CLR_CPU))
                sd      t0,(a0)

        /*
         * Let the secondary CPU(s) out of reset
         *
         * This is very BCM1250-specific at the moment.
         */

                la      a0,PHYS_TO_K1(A_SCD_SYSTEM_CFG)
                ld      t0,0(a0)
                dli     t1,M_SYS_CPU_RESET_1    # Reset mask
                not     t1                      # clear this bit
                and     t0,t1                   # New value to write
                sd      t0,0(a0)                # CPU1 is now running

        /*
         * Wait for the other CPU to ring our doorbell
         */

1:              la      a0,PHYS_TO_K1(A_IMR_REGISTER(0,R_IMR_MAILBOX_CPU))
                ld      t0,(a0)                 # Read mailbox
                beq     t0,zero,1b              # Loop till the bit is set

        /*
         * Clear the mailbox to dismiss the pending interrupts
         */
                la      a0,PHYS_TO_K1(A_IMR_REGISTER(0,R_IMR_MAILBOX_CLR_CPU))
                dli     t0,-1                   # clear all 64 bits
                sd      t0,(a0)

        /*
         * Okay, it's safe to return
         */

                j       ra

	.end	start_altCpu

/***************************************************************************
*
* cpu_kseg0_switch - manipulate the return address so the program counter 
*                  returns to the KSEG0 region
*
*  Input parameters:
*      nothing
*
*  Return value:
*      nothing
*/

	.ent	cpu_kseg0_switch
cpu_kseg0_switch:

                and     ra,(K0SIZE-1)
                or      ra,K0BASE
                j       ra

	.end	cpu_kseg0_switch

/***************************************************************************
*
* dram_check: simple dram setting sanity check, write/read low and high addr
*
*/

	.ent	dram_check
dram_check:
        li	t0, 0
        li	t1, -1
	li	t2, 0xA0000000
        li	t3, 0xA1000000
        sd 	t0, (t2)
        sd	t1, (t3)
        sync 
        ld      t4, (t2)
        bne 	t4, t0, dram_check_fail	
        ld      t4, (t3)
        bne 	t4, t1, dram_check_fail	
        li	v0, 0
	j	ra
dram_check_fail:
        li	v0, -1
	j	ra
	.end 	dram_check

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


/******************************************************************************
*
* romExcHandle - rom based exception/interrupt handler
*
* This routine is invoked on an exception or interrupt while
* the status register is using the bootstrap exception vectors.
* It saves a state frame to a known uncached location so someone
* can examine the data over the VME.
*
* THIS ROUTIINE IS NOT CALLABLE FROM "C"
*
*/

        .ent    romExcHandle
romExcHandle:

        .set    noat
        move    k1, sp                  /* save fault sp */
        li      sp, ROM_ISP_BASE        /* sp to known uncached location */
        SW      sp, E_STK_SP-ESTKSIZE(sp) /* save sp in new intstk frame */
        subu    sp, ESTKSIZE            /* make new exc stk frame       */
        SW      k0, E_STK_K0(sp)        /* save k0, (exception type)    */
        SW      k1, E_STK_SP(sp)        /* save SP at fault */
        SW      AT, E_STK_AT(sp)        /* save asmbler resvd reg       */
        .set    at
        SW      v0, E_STK_V0(sp)        /* save func return 0, used
                                           to hold masked cause         */
        mfc0    k1, C0_BADVADDR         /* read bad VA reg      */
        sw      k1, E_STK_BADVADDR(sp)  /* save bad VA on stack */
        mfc0    k1, C0_EPC              /* read exception pc    */
        sw      k1, E_STK_EPC(sp)       /* save EPC on stack    */
        mfc0    v0, C0_CAUSE            /* read cause register  */
        sw      v0, E_STK_CAUSE(sp)     /* save cause on stack  */
        mfc0    k1, C0_SR               /* read status register */
        sw      k1, E_STK_SR(sp)        /* save status on stack */

        .set    noat
        mflo    AT                      /* read entry lo reg    */
        SW      AT,E_STK_LO(sp)         /* save entry lo reg    */
        mfhi    AT                      /* read entry hi reg    */
        SW      AT,E_STK_HI(sp)         /* save entry hi reg    */
        .set    at
        SW      zero, E_STK_ZERO(sp)    /* save zero ?!         */
        SW      v1,E_STK_V1(sp)         /* save func return 1   */
        SW      a0,E_STK_A0(sp)         /* save passed param 0  */
        SW      a1,E_STK_A1(sp)         /* save passed param 1  */
        SW      a2,E_STK_A2(sp)         /* save passed param 2  */
        SW      a3,E_STK_A3(sp)         /* save passed param 3  */
        SW      t0,E_STK_T0(sp)         /* save temp reg 0      */
        SW      t1,E_STK_T1(sp)         /* save temp reg 1      */
        SW      t2,E_STK_T2(sp)         /* save temp reg 2      */
        SW      t3,E_STK_T3(sp)         /* save temp reg 3      */
        SW      t4,E_STK_T4(sp)         /* save temp reg 4      */
        SW      t5,E_STK_T5(sp)         /* save temp reg 5      */
        SW      t6,E_STK_T6(sp)         /* save temp reg 6      */
        SW      t7,E_STK_T7(sp)         /* save temp reg 7      */
        SW      t8,E_STK_T8(sp)         /* save temp reg 8      */
        SW      t9,E_STK_T9(sp)         /* save temp reg 9      */
        SW      s0,E_STK_S0(sp)         /* save saved reg 0     */
        SW      s1,E_STK_S1(sp)         /* save saved reg 1     */
        SW      s2,E_STK_S2(sp)         /* save saved reg 2     */
        SW      s3,E_STK_S3(sp)         /* save saved reg 3     */
        SW      s4,E_STK_S4(sp)         /* save saved reg 4     */
        SW      s5,E_STK_S5(sp)         /* save saved reg 5     */
        SW      s6,E_STK_S6(sp)         /* save saved reg 6     */
        SW      s7,E_STK_S7(sp)         /* save saved reg 7     */
        SW      s8,E_STK_FP(sp)         /* save saved reg 8     */
        SW      gp,E_STK_GP(sp)         /* save global pointer? */
        SW      ra,E_STK_RA(sp)         /* save return address  */
blink:

        b       blink

        .end    romExcHandle            /* that's all folks */

/***************************************************************************
*
* initialization 'helper' code
*
*/

#include "bcm1250CpuInit.s"
#ifdef _SENTOSA_
#include "sentosa.s"
#elif defined _RHONE_
#include "rhone.s"
#else
#include "swarm.s"
#endif
