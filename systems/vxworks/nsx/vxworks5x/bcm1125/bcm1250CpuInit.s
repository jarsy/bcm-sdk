/* bcm1250Cpu.s - MIPS BCM1250 rom initialization support module */

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

/* $Id: bcm1250CpuInit.s,v 1.3 2011/07/21 16:14:43 yshtil Exp $
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
This module contains helper functions required by the bcm1250 romInit.s
They are broken out into this module to keep romInit reasonable to 
navigate.

*/

	.text

/***************************************************************************
*
* core_init - initialization sequence for an individual core
*
*  Input parameters:
*      none
*
*  Return value:
*      none
*/

        .ent    core_init
core_init:

		move	s1,ra			/* will be trashing RA */

       /*
        * Init the CP0 registers
        */
		SETLEDS('C','P','0','I')
		RELOC(k1, cp0_init)
		jal   k1


	/*
	 * Init the L1 cache.  
	 */

		SETLEDS('L','1','C','I')
		RELOC(k1, cacheSb1Reset)
		jal   k1
	
		move	ra,s1			/* restore return address */
		j	ra

        .end    core_init

/***************************************************************************
*
* processor_init - initialization sequence for on-chip peripherials that
*                  are required during boot-up
*
*  Input parameters:
*      none
*
*  Return value:
*      none
*/

        .ent    processor_init
processor_init:

                move    s1,ra                   /* will be trashing RA */

        /*
         * Init the L2 cache.  We don't init L2 on
         * secondary CPU(s), since it is shared by all CPUs.  We'll
         * do it here by cpu0 only.
         */

                SETLEDS('L','2','C','I')
                RELOC(k1, bcm1250_l2cache_init)
                jal   k1

		move	ra,s1			/* restore return address */
		j	ra

        .end    processor_init


/***************************************************************************
*
* cp0_init - initialize CP0 registers for an SB1 core
*
*  Input parameters:
*      none
*
*  Return value:
*      none
*/

        .ent    cp0_init
cp0_init:
                .set noreorder                  # so as to minimize HAZARDS
                mtc0    zero,C0_WATCHLO         # Clear out the watch regs.
                mtc0    zero,C0_WATCHHI

                mfc0    v0,C0_SR                # Get status register
                HAZARD_CP_READ
                and     v0,SR_SR                # preserve soft reset
                or      v0,INITIAL_SR           # initial SR settings

                mfc0    v1,C0_CONFIG            # get current CONFIG register
                mtc0    zero,C0_CAUSE           # must clear before writing SR
                mtc0    v0,C0_SR                # set up the status register

                srl     v1,v1,3                 # strip out K0 bits
                sll     v1,v1,3                 # k0 bits now zero
                or      v1,v1,CFG_K0_CACHEABLE  # K0 is cached, non-coherent
                # or      v1,v1,2                 # K0 is uncached [for testing]
                mtc0    v1,C0_CONFIG

                # mtc0  zero,C0_WATCHLO,0       # Watch registers.
                # mtc0  zero,C0_WATCHHI,0
                # mtc0  zero,C0_WATCHLO,1
                # mtc0  zero,C0_WATCHHI,1
                mtc0    zero,C0_WATCHLO
                mtc0    zero,C0_WATCHHI
                .word   0x40809001
                .word   0x40809801

                mtc0    zero,C0_TLBHI           # TLB entry (high half)

        #
        # This is probably not the right init value for C0_COMPARE,
        # but it seems to be necessary for the sim model right now.
        #

                li      v0,-1
                mtc0    v0,C0_COMPARE

        #
        # Initialize all the TLB entries to some invalid value
        #

                mtc0    zero,C0_TLBLO0          /* tlblo0 = invalid */
                mtc0    zero,C0_TLBLO1          /* tlblo1 = invalid */
                mtc0    zero,C0_PAGEMASK
                li      t0,K1BASE               /* tlbhi  = impossible vpn */
                li      t1,(TLB_ENTRIES-1)      /* index */

                nop
1:              mtc0    t0,C0_TLBHI
                mtc0    t1,C0_INX
                addu    t0,0x2000               /* inc vpn */
                tlbwi
                bnez    t1,1b
                subu    t1,1                    # BDSLOT
                .set reorder

                HAZARD_CP_WRITE                 /* for good measure */
                j	ra

        .end    cp0_init


