/* swarm.s - swarm board specific initialization routines */

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

/* $Id: swarm.s,v 1.3 2011/07/21 16:14:43 yshtil Exp $
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
This module contains the assembly-language part of the init code for 
this board support package.  The routines "board_earlyinit" and 
"board_draminfo" live here.
*/

        /* includes */
#include "bcm1250Lib.h"


        /* defines */
#define K_EOT 0xFF

        /* macros */
#define DRAMINFO(mcchan,smbusdev,rows,cols,banks,sides) \
      .byte mcchan,smbusdev,rows,cols,banks,sides,0,0


		.text

/******************************************************************************
*
* board_earlyinit - entry point for VxWorks in ROM
*
*  Initialize board registers.  This is the earliest 
*  time the BSP gets control.  This routine cannot assume that
*  memory is operational, and therefore all code in this routine
*  must run from registers only.  The $ra register must not
*  be modified, as it contains the return address.
*
*  This routine will be called from uncached space, before
*  the caches are initialized.  If you want to make
*  subroutine calls from here, you must use the CALLKSEG1 macro.
*
*  Among other things, this is where the GPIO registers get 
*  programmed to make on-board LEDs function, or other startup
*  that has to be done before anything will work.
*  
* void board_earlyinit (void)
*  	   
*  RETURNS: N/A
*/

        .globl  board_earlyinit
        .ent    board_earlyinit
board_earlyinit:

	/*
	 * Reprogram the SCD to make sure UART0 is enabled.
	 * Some CSWARM boards have the SER0 enable bit when
	 * they're not supposed to, which switches the UART
	 * into synchronous mode.  Kill off the SCD bit.
	 * XXX this should be investigated in hardware, as 
	 * XXX it is a strap option on the CPU.
	 */

		li      t0,PHYS_TO_K1(A_SCD_SYSTEM_CFG)
		ld	t1,0(t0)
		dli	t2,~M_SYS_SER0_ENABLE
		and	t1,t1,t2
		sd	t1,0(t0)

       /*
        * Configure the GPIOs
        */

		li	t0,PHYS_TO_K1(A_GPIO_DIRECTION)
		li	t1,GPIO_OUTPUT_MASK
		sd	t1,0(t0)

		li	t0,PHYS_TO_K1(A_GPIO_INT_TYPE)
		li	t1,GPIO_INTERRUPT_MASK
		sd	t1,0(t0)

       #
       # Turn on the diagnostic LED and turn off the sturgeon NMI
       #
		li	t0,PHYS_TO_K1(A_GPIO_PIN_SET)
		li	t1,M_GPIO_DEBUG_LED
		sd	t1,0(t0)

		li	t0,PHYS_TO_K1(A_GPIO_PIN_CLR)
		li	t1,M_GPIO_STURGEON_NMI
		sd	t1,0(t0)


       #
       # Configure the LEDs
       #     

		li	t0,PHYS_TO_K1(A_IO_EXT_CS_BASE(LEDS_CS))
		li	t1,LEDS_PHYS >> S_IO_ADDRBASE
		sd	t1,R_IO_EXT_START_ADDR(t0)

		li	t1,LEDS_SIZE-1	/* Needs to be 1 smaller, se UM for details */
		sd	t1,R_IO_EXT_MULT_SIZE(t0)

		li	t1,LEDS_TIMING0
		sd	t1,R_IO_EXT_TIME_CFG0(t0)

		li	t1,LEDS_TIMING1
		sd	t1,R_IO_EXT_TIME_CFG1(t0)

		li	t1,LEDS_CONFIG
		sd	t1,R_IO_EXT_CFG(t0)



       #
       # Configure the alternate boot ROM
       #     

		li	t0,PHYS_TO_K1(A_IO_EXT_CS_BASE(ALT_BOOTROM_CS))

		li	t1,ALT_BOOTROM_PHYS >> S_IO_ADDRBASE
		sd	t1,R_IO_EXT_START_ADDR(t0)

		li	t1,ALT_BOOTROM_SIZE-1
		sd	t1,R_IO_EXT_MULT_SIZE(t0)

		li	t1,ALT_BOOTROM_TIMING0
		sd	t1,R_IO_EXT_TIME_CFG0(t0)

		li	t1,ALT_BOOTROM_TIMING1
		sd	t1,R_IO_EXT_TIME_CFG1(t0)

		li	t1,ALT_BOOTROM_CONFIG
		sd	t1,R_IO_EXT_CFG(t0)



       #
       # Configure the IDE interface
       #     

		li	t0,PHYS_TO_K1(A_IO_EXT_CS_BASE(IDE_CS))

		li	t1,IDE_PHYS >> S_IO_ADDRBASE
		sd	t1,R_IO_EXT_START_ADDR(t0)

		li	t1,IDE_SIZE-1
		sd	t1,R_IO_EXT_MULT_SIZE(t0)

		li	t1,IDE_TIMING0
		sd	t1,R_IO_EXT_TIME_CFG0(t0)

		li	t1,IDE_TIMING1
		sd	t1,R_IO_EXT_TIME_CFG1(t0)

		li	t1,IDE_CONFIG
		sd	t1,R_IO_EXT_CFG(t0)


       #
       # Configure the PCMCIA
       #     

		li	t0,PHYS_TO_K1(A_IO_EXT_CS_BASE(PCMCIA_CS))

		li	t1,PCMCIA_PHYS >> S_IO_ADDRBASE
		sd	t1,R_IO_EXT_START_ADDR(t0)

		li	t1,PCMCIA_SIZE-1
		sd	t1,R_IO_EXT_MULT_SIZE(t0)

		li	t1,PCMCIA_TIMING0
		sd	t1,R_IO_EXT_TIME_CFG0(t0)

		li	t1,PCMCIA_TIMING1
		sd	t1,R_IO_EXT_TIME_CFG1(t0)

		li	t1,PCMCIA_CONFIG
		sd	t1,R_IO_EXT_CFG(t0)

		j	ra

	.end	board_earlyinit

/***************************************************************************
*
* board_draminfo - return the address of the DRAM information table
*
* int * board_draminfo (void)
*
*  RETURNS: pointer DRAM info table, return 0 to use default in the
*           bcm1250DramInit module
*/

        .globl  board_draminfo
        .ent    board_draminfo
board_draminfo:

                move    t0,ra

#ifdef _HARDWIRED_MEMORY_TABLE
                LOADREL(v0,myinfo)
#else
                move    v0,zero         # auto configure
#endif

                move    ra,t0
                j       ra



myinfo:
        #       Chan SMBusAdr   Rows Cols Bnks ChpSel
        #       ---- ---------  ---- ---- ---- ------
        DRAMINFO(0,  0,         12,  8,   2,   0);
        DRAMINFO(K_EOT,0,0,0,0,0);

        .end    board_draminfo

