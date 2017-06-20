#ifndef _BCM4704_H_
#define _BCM4704_H_
/*
 *   Copyright 2003, Broadcom Corporation
 *   All Rights Reserved.
 * 
 *   Broadcom Sentry5 (S5) BCM4704, 53xx, BCM58xx SOC Internal Core
 *   and MIPS3301 (R4K) System Address Space
 *
 *   $Id: bcm4704.h,v 1.3 2006/01/12 22:57:03 sanjayg Exp $
 * 
 */

/* BCM4704 Address map */
#ifndef KSEG1ADDR
#define KSEG1ADDR(x)    ( (x) | 0xa0000000)
#endif

#ifndef KSEG2BASE
#define KSEG2BASE		0xc0000000
#endif

#define BCM4704_SDRAM		0x00000000 /* 0-128MB Physical SDRAM */
#define BCM4704_SDRAM_SZ	0x08000000 /* 128MB Physical SDRAM */
#define BCM4704_PCI_MEM		0x08000000 /* Host Mode PCI mem space (64MB) */
#define BCM4704_PCI_CFG		0x0c000000 /* Host Mode PCI cfg space (64MB) */
#define BCM4704_PCI_DMA		0x40000000 /* Client Mode PCI mem space (1GB)*/
#define	BCM4704_SDRAM_SWAPPED	0x10000000 /* Byteswapped Physical SDRAM */
#define BCM4704_SDRAM_SWAPPED_SZ	0x08000000 /* Byteswapped window size */
#define BCM4704_ENUM		0x18000000 /* Beginning of core enum space */

/* BCM4704 Core register space */
#define BCM4704_REG_CHIPC	0x18000000 /* Chipcommon  registers */
#define BCM4704_REG_EMAC0	0x18001000 /* Ethernet MAC0 core registers */
#define BCM4704_REG_EMAC1	0x18002000 /* Ethernet MAC0 core registers */
#define BCM4704_REG_USB		0x18003000 /* USB core registers */
#define BCM4704_REG_PCI		0x18004000 /* PCI core registers */
#define BCM4704_REG_MIPS33	0x18005000 /* MIPS core registers */
#define BCM4704_REG_CODEC   0x18006000  /* Codec core registers */
#define BCM4704_REG_IPSEC	0x18007000 /* BCM582x CryptoCore registers */
#define BCM4704_REG_MEMC	0x18008000 /* MEMC core registers */
#define BCM4704_REG_UARTS   (BCM4704_REG_CHIPC + 0x300) /* UART regs */
#define BCM4704_SDRAM_HIGH	0x80000000 /* 0-512MB Physical SDRAM */
#define BCM4704_SDRAM_HIGH_SZ	0x20000000 /* 0-512MB Physical SDRAM */
#define	BCM4704_EJTAG		0xff200000 /* MIPS EJTAG space (2M) */

/* COM Ports 1/2 */
#define	BCM4704_UART		(BCM4704_REG_UARTS)
#define BCM4704_UART_COM2	(BCM4704_REG_UARTS + 0x00000100)

/* Registers common to MIPS33 Core used in 4704 */
#define MIPS33_FLASH_REGION           0x1fc00000 /* Boot FLASH Region  */
#define MIPS33_EXTIF_REGION           0x1a000000 /* Chipcommon EXTIF region*/
#define BCM4704_EXTIF                 0x1a000000 /* MISC_CS */
#define MIPS33_FLASH_REGION_AUX       0x1c000000 /* FLASH Region 2*/

/* Internal Core Sonics Backplane Devices */
#define INTERNAL_UART_COM1            BCM4704_UART
#define INTERNAL_UART_COM2            BCM4704_UART_COM2
#define SB_REG_CHIPC                  BCM4704_REG_CHIPC
#define SB_REG_ENET0                  BCM4704_REG_EMAC0
#define SB_REG_ENET1                  BCM4704_REG_EMAC1
#define SB_REG_IPSEC                  BCM4704_REG_IPSEC
#define SB_REG_USB                    BCM4704_REG_USB
#define SB_REG_PCI                    BCM4704_REG_PCI
#define SB_REG_MIPS                   BCM4704_REG_MIPS33
#define SB_REG_MEMC                   BCM4704_REG_MEMC
#define SB_REG_MEMC_OFF               0x6000
#define SB_EXTIF_SPACE                MIPS33_EXTIF_REGION
#define SB_FLASH_SPACE                MIPS33_FLASH_REGION

/*
 * XXX
 * 4704-specific backplane interrupt flag numbers.  This should be done
 * dynamically instead.
 */
#define	SBFLAG_PCI	0
#define	SBFLAG_ENET0	1
#define	SBFLAG_ILINE20	2
#define	SBFLAG_CODEC	3
#define	SBFLAG_USB	4
#define	SBFLAG_EXTIF	5
#define	SBFLAG_ENET1	6

/* BCM94704 Local Bus devices */
#define BCM94704K_RESET_ADDR    	 BCM4704_EXTIF
#define BCM94704K_BOARDID_ADDR  	(BCM4704_EXTIF | 0x4000)
#define BCM94704K_DOC_ADDR      	(BCM4704_EXTIF | 0x6000)
#define BCM94704K_LED_ADDR      	(BCM4704_EXTIF | 0xc000)
#define BCM94704K_TOD_REG_BASE      (BCM94704K_NVRAM_ADDR | 0x1ff0)
#define BCM94704K_NVRAM_ADDR    	(BCM4704_EXTIF | 0xe000)
#define BCM94704K_NVRAM_SIZE        0x1ff0 /* 8K NVRAM : DS1743/STM48txx*/

/* Write to DLR2416 VFD Display character RAM */
#ifdef  MIPSEL
#define LED_REG(x)      \
 (*(volatile unsigned char *) (KSEG1ADDR(BCM94704K_LED_ADDR) + (x)))
#else
#define LED_REG(x)      \
 (*(volatile unsigned char *) (KSEG1ADDR(BCM94704K_LED_ADDR) + (x ^ 3)))
#endif

#ifdef	CONFIG_VSIM
#define	BCM4704_TRACE(trval)        do { *((int *)0xa0002ff8) = (trval); \
                                       } while (0)
#else
#define	BCM4704_TRACE(trval)        do { *((unsigned char *)\
                                         KSEG1ADDR(BCM4704K_LED_ADDR)) = (trval); \
				    *((int *)0xa0002ff8) = (trval); } while (0)
#endif

#define LED_GREEN	0x000000fd
#define LED_RED		0x0000007f
#define LED_YELLOW	0x000000bf		

#endif /*!_S5_H_ */
