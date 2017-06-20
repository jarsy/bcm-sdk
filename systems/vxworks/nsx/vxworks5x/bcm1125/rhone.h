/* bcm1250_int.h - Broadcom BCM1250 swarm board I/O, CS, & GPIO constants & macros */

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

/* $Id: rhone.h,v 1.3 2011/07/21 16:14:43 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01a,15nov01,agf  written.
*/

#ifndef __INCrhoneh
#define __INCrhoneh

/*
 * I/O Address assignments for the CSWARM board
 *
 * Summary of address map:
 *
 * Address         Size   CSel    Description
 * --------------- ----   ------  --------------------------------
 * 0x1FC00000      2MB     CS0    Boot ROM
 * 0x1EC00000      16MB    CS1    Alternate boot ROM
 *                         CS2    Unused
 *                         CS3    Unused
 *                         CS4    Unused
 *                         CS5    Unused
 * 0x1D0A0000  	   64KB    CS6    LED display
 *                         CS7    Unused
 *
 * GPIO assignments
 *
 * GPIO#    Direction   Description
 * -------  ---------   ------------------------------------------
 * GPIO0    Output      Debug LED
 * GPIO1    N/A         Not used, routed to daughter card
 * GPIO2    N/A         Not used, routed to daughter card
 * GPIO3    N/A         Not used, routed to daughter card
 * GPIO4    N/A         Not used, routed to daughter card
 * GPIO5    N/A         Not used, routed to daughter card
 * GPIO6    N/A         Not used, routed to daughter card
 * GPIO7    N/A         Not used, routed to daughter card
 * GPIO8    N/A         Not used, routed to daughter card
 * GPIO9    N/A         Not used, routed to daughter card
 * GPIO10   N/A         Not used, routed to daughter card
 * GPIO11   N/A         Not used, routed to daughter card
 * GPIO12   N/A         Not used, routed to daughter card
 * GPIO13   N/A         Not used, routed to daughter card
 * GPIO14   N/A         Not used, routed to daughter card
 * GPIO15   N/A         Not used, routed to daughter card
 * GPIO1    N/A         Not used, routed to daughter card
 */


#define MB (1024*1024)
#define K64 65536
#define NUM64K(x) (((x)+(K64-1))/K64)


/*  GPIO pins */

#define GPIO_DEBUG_LED		0
#define GPIO_STURGEON_NMI	1
#define GPIO_PHY_INTERRUPT	2
#define GPIO_NONMASKABLE_INT	3
#define GPIO_IDE_INTERRUPT	4
#define GPIO_TEMP_SENSOR_INT	5

#define M_GPIO_DEBUG_LED	_SB_MAKEMASK1(GPIO_DEBUG_LED)
#define M_GPIO_STURGEON_NMI	_SB_MAKEMASK1(GPIO_STURGEON_NMI)

#define GPIO_OUTPUT_MASK (_SB_MAKEMASK1(GPIO_DEBUG_LED) | \
                          _SB_MAKEMASK1(GPIO_STURGEON_NMI) )

#define GPIO_INTERRUPT_MASK ((V_GPIO_INTR_TYPEX(GPIO_PHY_INTERRUPT,K_GPIO_INTR_LEVEL)) | \
                             (V_GPIO_INTR_TYPEX(GPIO_IDE_INTERRUPT,K_GPIO_INTR_LEVEL)))


/*  Generic Bus */

/*
 * Boot ROM:  non-multiplexed, byte width, no parity, no ack
 * XXX: These are the (very slow) default parameters.   This can be sped up!
 */
#define BOOTROM_CS		0
#define BOOTROM_PHYS		0x1FC00000	/* address of boot ROM (CS0) */
#define BOOTROM_SIZE		NUM64K(2*MB)	/* size of boot ROM */
#define BOOTROM_TIMING0		V_IO_ALE_WIDTH(4) | \
                                V_IO_ALE_TO_CS(2) | \
                                V_IO_CS_WIDTH(24) | \
                                V_IO_RDY_SMPLE(1)
#define BOOTROM_TIMING1		V_IO_ALE_TO_WRITE(7) | \
                                V_IO_WRITE_WIDTH(7) | \
                                V_IO_IDLE_CYCLE(6) | \
                                V_IO_CS_TO_OE(0) | \
                                V_IO_OE_TO_CS(0)
#define BOOTROM_CONFIG		V_IO_WIDTH_SEL(K_IO_WIDTH_SEL_1) | M_IO_NONMUX

/*
 * Alternate Boot ROM:  non-multiplexed, byte width, no parity, no ack
 * XXX: These are the (very slow) default parameters.   This can be sped up!
 */
#define ALT_BOOTROM_CS		1
#define ALT_BOOTROM_PHYS	0x1EC00000	/* address of alternate boot ROM (CS1) */
#define ALT_BOOTROM_SIZE	NUM64K(16*MB)	/* size of alternate boot ROM */
#define ALT_BOOTROM_TIMING0	V_IO_ALE_WIDTH(4) | \
                                V_IO_ALE_TO_CS(2) | \
                                V_IO_CS_WIDTH(24) | \
                                V_IO_RDY_SMPLE(1)
#define ALT_BOOTROM_TIMING1	V_IO_ALE_TO_WRITE(7) | \
                                V_IO_WRITE_WIDTH(7) | \
                                V_IO_IDLE_CYCLE(6) | \
                                V_IO_CS_TO_OE(0) | \
                                V_IO_OE_TO_CS(0)
#define ALT_BOOTROM_CONFIG	V_IO_WIDTH_SEL(K_IO_WIDTH_SEL_1) | M_IO_NONMUX

/*
 * LEDs:  non-multiplexed, byte width, no parity, no ack
 */
#define LEDS_CS			6
#define LEDS_PHYS		0x1D0A0000
#define LEDS_SIZE		NUM64K(4)
#define LEDS_TIMING0		V_IO_ALE_WIDTH(4) | \
                                V_IO_ALE_TO_CS(2) | \
                                V_IO_CS_WIDTH(13) | \
                                V_IO_RDY_SMPLE(1)
#define LEDS_TIMING1		V_IO_ALE_TO_WRITE(2) | \
                                V_IO_WRITE_WIDTH(8) | \
                                V_IO_IDLE_CYCLE(6) | \
                                V_IO_CS_TO_OE(0) | \
                                V_IO_OE_TO_CS(0)
#define LEDS_CONFIG		V_IO_WIDTH_SEL(K_IO_WIDTH_SEL_1) | M_IO_NONMUX
                              
/*  SMBus Devices */

#define TEMPSENSOR_SMBUS_CHAN	0
#define TEMPSENSOR_SMBUS_DEV	0x2A
#define SPDEEPROM_SMBUS_CHAN	0
#define SPDEEPROM_SMBUS_DEV	0x54
#define BIGEEPROM0_SMBUS_CHAN	0
#define BIGEEPROM0_SMBUS_DEV	0x50
#define BIGEEPROM1_SMBUS_CHAN	1
#define BIGEEPROM1_SMBUS_DEV	0x50
#define M41T81_SMBUS_CHAN       1
#define M41T81_SMBUS_DEV        0x68


#endif /* __INCrhoneh */
