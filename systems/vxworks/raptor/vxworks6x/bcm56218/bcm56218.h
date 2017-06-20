#ifndef _BCM56218_H_
#define _BCM56218_H_
/*
 *   Copyright 2003, Broadcom Corporation
 *   All Rights Reserved.
 * 
 *   Broadcom BCM56218 SOC Internal Core
 *   and MIPS3301 (R4K) System Address Space
 *
 *   $Id: bcm56218.h,v 1.3 2007/10/08 22:31:11 iakramov Exp $
 * 
 */

/* BCM56218 Address map */
#ifndef KSEG1ADDR
#define KSEG1ADDR(x)    ( (x) | 0xa0000000)
#endif

#ifndef KSEG2BASE
#define KSEG2BASE		0xc0000000
#endif

#define BCM56218_SDRAM		0x00000000 /* 0-128MB Physical SDRAM */
#define BCM56218_SDRAM_SZ	0x08000000 /* 128MB Physical SDRAM */
#define	BCM56218_SDRAM_SWAPPED	0x10000000 /* Byteswapped Physical SDRAM */
#define BCM56218_SDRAM_SWAPPED_SZ	0x08000000 /* Byteswapped window size */

/* BCM56218 Core register space */
#define BCM56218_REG_CHIPC	0xb8000000 /* Chipcommon  registers */
#define BCM56218_REG_GPIO 	0xb8000060 /* GPIO registers */
#define BCM56218_REG_MIPS33	0xb8005000 /* MIPS core registers */
#define BCM56218_REG_MEMC	0xb8008000 /* MEMC core registers */
#define BCM56218_REG_UARTS   (BCM56218_REG_CHIPC + 0x300) /* UART regs */
#define BCM56218_SDRAM_HIGH	0x80000000 /* 0-512MB Physical SDRAM */
#define BCM56218_SDRAM_HIGH_SZ	0x20000000 /* 0-512MB Physical SDRAM */
#define	BCM56218_EJTAG		0xff200000 /* MIPS EJTAG space (2M) */

/* COM Ports 1/2 */
#define	BCM56218_UART		BCM56218_REG_UARTS
#define BCM56218_UART_COM2	(BCM56218_REG_UARTS + 0x00000100)

/* Registers common to MIPS33 Core used in 56218 */
#define MIPS33_FLASH_REGION           0x1fc00000 /* Boot FLASH Region  */
#define MIPS33_FLASH_REGION_AUX       0x1c000000 /* FLASH Region 2*/

#define ICS_CMIC_BASE_ADDR            0xA8000000
#define INTERNAL_UART_COM1            BCM56218_UART
#define INTERNAL_UART_COM2            BCM56218_UART_COM2
#define SB_REG_CHIPC                  BCM56218_REG_CHIPC
#define SB_REG_MIPS                   BCM56218_REG_MIPS33
#define SB_REG_MEMC                   BCM56218_REG_MEMC
#define SB_FLASH_SPACE                MIPS33_FLASH_REGION

#define ICS_CHIP_ID_REG             0xb8000000

/* RESET register */
#define ICS_SRESET_REG              0xb8000110

#define ICS_WATCHDOG_CTR_REG        0xb8000080

/* timer 0 registers */
#define ICS_TIMER0_INT_STATUS      (volatile uint32*) 0xb8005020
#define ICS_TIMER0_INT_MASK        (volatile uint32*) 0xb8005024
#define ICS_TIMER0_REG             (volatile uint32*) 0xb8005028
#define I_TO                        1

#define ICS_DEV_REV_ID_REG         *((volatile uint32*) 0xA8000178)

#define ICS_SB_CLK_FREQ             133000000



#endif /*!_BCM56218_H_ */
