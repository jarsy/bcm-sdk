/* $Id: bmw.h,v 1.4 2011/07/21 16:14:08 yshtil Exp $
 * BMW Board constants and defines
 *
 * Copyright 1998 Broadcom Corp.
 */

/*
modification history
--------------------
01g,09oct02,jmb PLD register definitions for chassis system
01f,01oct02,jmb Support boot printing for systems without LEDs, add
                board rev id's
01e,13jun02,jmb Supply an address for 2nd DOC, delete unneeded FLASH0 macros
01d,29may02,jmb add component include for BCM570x END driver
01c,01may02,jmb serial clock support for variable ram speed
01b,20apr02,jfd added TFFS, LCD support
01a,18apr02,jmb derived from mousse.h

*/


#ifndef BMW_H
#define BMW_H

#include "epic.h"

/* Timer constants */

#define SYS_CLK_RATE_MIN	3		/* min system clock rate */
#define SYS_CLK_RATE_MAX	5000		/* max system clock rate */
#define AUX_CLK_RATE_MIN	3		/* min aux clock rate */
#define AUX_CLK_RATE_MAX	5000		/* max aux clock rate */

#define MHZ 1000000

/* PCI clock */

#define PCI_CLK_FREQ (33*MHZ)

/* RAM clock */

#define SDRAM_CLK_FREQ (66*MHZ)

/* Serial clock  -- either a constant or computed  */

/* #define XTAL SDRAM_CLK_FREQ  */
#define XTAL sysSerialClkGet() 

/* Only one can be selected, FULL overrides BASIC */

#ifdef INCLUDE_MMU_FULL
#   undef INCLUDE_MMU_BASIC
#endif

/*
 * If INCLUDE_ECC is defined, ECC is enabled in the boot ROM.  Since
 * ECC increases RDLAT by 1, there is a performance penalty for ECC
 * that varies by application.
 */

#undef INCLUDE_ECC

/*
 * Miscellaneous definitions go here.
 * For example, macro definitions for various devices.
 */

#define CPU_TYPE		((vxPvrGet() >> 16) & 0xffff)
#define CPU_TYPE_601		0x01		/* PPC 601 CPU */
#define CPU_TYPE_602		0x02		/* PPC 602 CPU */
#define CPU_TYPE_603		0x03		/* PPC 603 CPU */
#define CPU_TYPE_603E		0x06		/* PPC 603e CPU */
#define CPU_TYPE_603P		0x07		/* PPC 603p CPU */
#define CPU_TYPE_604		0x04		/* PPC 604 CPU */
#define CPU_TYPE_604E		0x09		/* PPC 604e CPU */
#define CPU_TYPE_604R		0x0a		/* PPC 604r CPU */
#define CPU_TYPE_750		0x08		/* PPC 750  CPU */
#define CPU_TYPE_8240		0x81		/* PPC 8240 CPU */
#define CPU_TYPE_8245		0x8081		/* PPC 8245 CPU */

/* System addresses */

#define PCI_SPECIAL_BASE	0xfe000000
#define PCI_SPECIAL_SIZE	0x01000000

#define EUMBBAR_VAL		0x80500000	/* Location of EUMB region */
#define EUMBSIZE		0x00100000	/* Size of EUMB region */

/* Extended ROM space devices */

#define XROM_BASE_ADDR          0x7c000000      /* RCS2 (PAL / Satellite IO) */
#define PLD_REG_BASE		XROM_BASE_ADDR
#define LED_REG_BASE		(XROM_BASE_ADDR | 0x2000)
#define TOD_BASE		(XROM_BASE_ADDR | 0x4000)
#define LED_REG(x)              (*(volatile UINT8 *) (LED_REG_BASE + (x)))
#define XROM_DEV_SIZE		0x00006000

#define ENET_DEV_BASE		0x80000000

#define PLD_REG(off)		(*(volatile UINT8 *) (PLD_REG_BASE + (off)))

#define PLD_REVID_B1		0x7f
#define PLD_REVID_B2		0x01

#define SYS_HARD_RESET()	{ for (;;) PLD_REG(0) = 0x80; }
#define SYS_REVID_GET()		((int) PLD_REG(0) & 0x7f)
#define SYS_LED_OFF()		(PLD_REG(1) |= 0x80)
#define SYS_LED_ON()		(PLD_REG(1) &= ~0x80)
#define SYS_WATCHDOG_IRQ3()	(PLD_REG(2) |= 0x80)
#define SYS_WATCHDOG_RESET()	(PLD_REG(2) &= ~0x80)
#define SYS_TOD_PROTECT()	(PLD_REG(3) |= 0x80)
#define SYS_TOD_UNPROTECT()	(PLD_REG(3) &= ~0x80)

/* Some PLD registers in the CFM */
#define CFM_REG_STATUS_LEDS  0x1
#define CFM_REG_LM_LED_MODE  0x4
#define CFM_REG_INTMASK      0x5  
#define CFM_REG_CFM_RESET    0x7
#define CFM_REG_LM_RESET     0x6
#define CFM_REG_LM_PRESENCE  0x10
#define CFM_REG_CFM_PRESENCE 0x11
#define CFM_REG_PS_STATUS    0x12      /* power supply status */
#define CFM_REG_LM_SWITCH    0x13
#define CFM_REG_FAN          0x14
#define CFM_REG_INTCAUSE     0x15
#define CFM_REG_INTACK       0x16
#define CFM_REG_PLD_REVID    0x17
#define CFM_REG_PS_MASK      0x18      /* power supply interrupt mask */
#define CFM_REG_SPI_COM0     0x8       /* SPI command reg for ROBO 0 */
#define CFM_REG_SPI_COM1     0x9       /* SPI command reg for ROBO 1 */
#define CFM_REG_SPI_STATUS   0xa
#define CFM_REG_SPI_READ_ACK 0xb
#define CFM_REG_SPI_EOC      0xc       /* SPI end-of-command register */

/* Bitfields in some CFM registers */
/* CFM_REG_STATUS_LEDS */
#define CFM_STATUS_MASTER_RED (1 << 0)
#define CFM_STATUS_MASTER_GREEN (1 << 1)
#define CFM_STATUS_STATUS_RED (1 << 2)
#define CFM_STATUS_STATUS_GREEN (1 << 3)
#define CFM_STATUS_POWER_RED  (1 << 4)
#define CFM_STATUS_POWER_GREEN  (1 << 5)
#define CFM_STATUS_ALL_GREEN (CFM_STATUS_MASTER_GREEN | \
                              CFM_STATUS_STATUS_GREEN | \
                              CFM_STATUS_POWER_GREEN )

/* CFM_REG_LM_LED_MODE */
#define CFM_LM_LED_STATUS_RED (1 << 0)
#define CFM_LM_LED_STATUS_GREEN (1 << 1)
#define CFM_LM_LED_DIAG_RED   (1 << 2)
#define CFM_LM_LED_DIAG_GREEN   (1 << 3)
#define CFM_LM_LED_SPEED_RED (1 << 4)
#define CFM_LM_LED_SPEED_GREEN (1 << 5)
#define CFM_LM_LED_DUPLEX_RED (1 << 6)
#define CFM_LM_LED_DUPLEX_GREEN (1 << 7)

/* CFM_REG_INTMASK */
#define CFM_INTMASK_IRQ0 0x80
#define CFM_INTMASK_BUTTON 0x01
#define CFM_INTMASK_FAN 0x02
#define CFM_INTMASK_POWER 0x04
#define CFM_INTMASK_TEMPERATURE 0x08
#define CFM_INTMASK_LM_PRESENCE 0x10
#define CFM_INTMASK_CFM_PRESENCE 0x20

/* CFM_REG_CFM_RESET */
#define CFM_RESET_SLOT_ID   (1 << 1) /* (1)set slot-2 (0)reset slot-1 */

/* CFM_REG_CFM_PRESENCE */
#define CFM_PRESENCE_OTHER  (1 << 0) 


/* REVID values expected in PLD */

#define ID_CFM_1 0x24   
#define ID_LANCELOT 17 /* 0x11 */
#define ID_BMW_1 110   /* 0x6E */
#define ID_BMW_2 97    /* 0x61 */
#define ID_BMW_3 37    /* 0x25 */
#define ID_WHITE_KNIGHT 110 /* 0x6E */
#define ID_MERLIN 0x3
#define ID_BCM56215R26T 0x55

#define	TOD_NVRAM_BASE		TOD_BASE
#define	TOD_NVRAM_SIZE		0x1ff0
#define	TOD_NVRAM_LIMIT		(TOD_NVRAM_BASE + TOD_NVRAM_SIZE)

#undef TOD_NONE   /* temp for debugging Mukul's board */

#ifdef TOD_NONE
#undef	TOD_NVRAM_BASE	
#define	TOD_NVRAM_BASE	0x00010000
#define DEC_CLOCK_FREQ	50000000
#endif

#define DS1743_BASE_ADDR	TOD_BASE

#define	SERIAL_BASE(_x)		(EUMBBAR_VAL | 0x4500 | ((_x) ? 0x100 : 0))

#define UART_SIMPLE 1
#define UART_MODEM 0
#define DUART_CONFIG UART_SIMPLE

/* Only one serial connector on board, even though PPC has 2 UARTs */

#define N_SIO_CHANNELS		1	

#if (N_SIO_CHANNELS > 1)
#if (DUART_CONFIG == UART_MODEM)
#error Only 1 UART when modem control enabled
#endif
#endif


/*
 * On-board PCI Ethernet
 */

#define INCLUDE_BCM570XEND

#define	PCI_ENET_IOADDR		0x00800000
#define	PCI_ENET_MEMADDR	0x80000000

/*
 * FLASH memory address space: 8-bit wide FLASH memory spaces.
 */
#define FLASH0_SEG0_START       0xffe00000       /* Baby 32Kb segment */
#define FLASH0_SEG0_END         0xffe07fff       /* 16 kB + 8 kB + 8 kB */
#define FLASH0_SEG0_SIZE        0x00008000       /*   (sectors SA0-SA2) */

#define FLASH0_SEG1_START       0xffe10000       /* 1MB - 64Kb FLASH0 seg */
#define FLASH0_SEG1_END         0xffefffff       /* 960 kB */
#define FLASH0_SEG1_SIZE        0x000f0000

#define FLASH0_SEG2_START       0xfff00000       /* Boot Loader stored here */
#define FLASH0_SEG2_END         0xfff7ffff       /* 512 kB FLASH0/PLCC seg */
#define FLASH0_SEG2_SIZE        0x00080000

#define FLASH0_SEG3_START       0xfff80000       /* 512 kB FLASH0 seg */
#define FLASH0_SEG3_END         0xffffffff
#define FLASH0_SEG3_SIZE        0x00080000

#define FLASH0_START		0xff800000	 /* 8 MB of address space */
#define FLASH0_END		0xffffffff	 /* for 2 MB of actual PROM */
#define FLASH0_SIZE		0x00800000

#define FLASH1_START		0xff000000	 /* 8 MB of address space */
#define FLASH1_END		0xff7fffff	 /* for 2 MB of actual PROM */
#define FLASH1_SIZE		0x00800000

/*
 * FLASH memory address space: 8-bit wide this is for 32MB flash
 */
#define FLASH2_START		0x70000000	 /* this is RCS3 */
#define FLASH2_END		0x71ffffff	 
#define FLASH2_SIZE		0x02000000       /* 16 MB */

#define FLASH_RESET_VECT	0xfff00100	 /* Where Kahlua starts */

/*
 * CHRP / PREP (MAP A/B) definitions.
 */

#define PREP_REG_ADDR		0x80000cf8	/* MPC107 Config, Map A */
#define PREP_REG_DATA		0x80000cfc	/* MPC107 Config, Map A */

#define CHRP_REG_ADDR		0xfec00000	/* MPC106 Config, Map B */
#define CHRP_REG_DATA		0xfee00000	/* MPC106 Config, Map B */

/*
 * BMW PCI IDSEL Assignments (Device Number)
 */

#define BMW_IDSEL_ENET	13		/* On-board 21143 Ethernet */
#define BMW_IDSEL_LPCI	14		/* On-board PCI slot */
#define BMW_IDSEL_82371	15		/* That other thing */
#define BMW_IDSEL_CPCI2	31		/* CPCI slot 2 */
#define BMW_IDSEL_CPCI3	30		/* CPCI slot 3 */
#define BMW_IDSEL_CPCI4	29		/* CPCI slot 4 */
#define BMW_IDSEL_CPCI5	28		/* CPCI slot 5 */
#define BMW_IDSEL_CPCI6	27		/* CPCI slot 6 */

/*
 * BMW Interrupt Mapping:
 *
 *	IRQ1	Enet (intA|intB|intC|intD)
 *	IRQ2	CPCI intA (See below)
 *	IRQ3	Local PCI slot intA|intB|intC|intD
 *	IRQ4	COM1 Serial port (Actually higher addressed port on duart)
 *
 * PCI Interrupt Mapping in Broadcom Test Chassis:
 *
 *                 |           CPCI Slot
 * 		   | 1 (CPU)	2	3	4       5       6
 *      -----------+--------+-------+-------+-------+-------+-------+
 *	  intA 	   |    X               X		X
 *	  intB     |            X               X		X
 *	  intC     |    X               X		X
 *	  intD	   |            X               X		X
 */

#define INT_VEC_IRQ0		0
#define INT_NUM_IRQ0		INT_VEC_IRQ0

#define BMW_IRQ_ENET		EPIC_VECTOR_EXT1	/* Hardwired */
#define BMW_IRQ_CPCI		EPIC_VECTOR_EXT2	/* Hardwired */
#define BMW_IRQ_LPCI		EPIC_VECTOR_EXT3	/* Hardwired */
#define BMW_IRQ_DUART	EPIC_VECTOR_EXT4	/* Hardwired */

/* DEC/Intel 21143 Ethernet Chips */
#define DC21140_VENDOR_ID		0x1011
#define DC21140_DEVICE_ID		0x0009

#define DC21143_VENDOR_ID		0x1011
#define DC21143_DEVICE_ID		0x0019
#define MAX_NUM_DEC_CHIP  5 /* Max 2114x */

/* Generic PCI-PCI (P2P) Bridge configruation parameters */
#define P2P_CLR_STATUS           0xFFFF0000
#define P2P_SEC_BUS_RESET        (0x0040 << 16)
#define P2P_CLK_ENABLE           0x00       /* enable clocks on all slots */
#define P2P_PMC_DISABLE          0
#define P2P_PMC_ENABLE           6

/* MPC8240 Architecture specific settings for PCI-PCI Bridge support */
/* See MPC8240UM Address Map B */
#define	P2P_IO_BASE              0x81100000  /* PCI I/O window */
#define	P2P_IO_SIZE              0x00800000  /* Size of IO space */
#define	P2P_PREF_MEM_BASE        0x81100000  /* PCI prefetch mem window */
#define	P2P_PREF_MEM_SIZE        0x01000000  /* Size of prefetchable region */
#define	P2P_PREF_HI32_BASE       0x00000000  /* hi 32 bits of address */
#define	P2P_NONPREF_MEM_BASE     0x81100000  /* PCI non-prefetch mem window */
#define P2P_NONPREF_MEM_SIZE	 0x01000000  /* PCI non-prefetch size */
#define P2P_CACHE_LINE_SIZE      8           /* cache line size */
#define P2P_PRIM_LATENCY         0           /* latency */
/*
 * Segment PCI-PCI bridge memory space into 4K windows
 * which are inclusive of the mapping setup above.
 */
#define P2P_4K_PCI_MEM_ADDR(x)    (P2P_PREF_MEM_BASE + 0x1000*(x))

/* Generic Bridge specific settings */
#define P2P_IO_SPACE_BASE_ADRS  ((P2P_IO_BASE & 0x0000F000) >> 8)
#define P2P_IO_SPACE_LIMIT_ADRS ((P2P_IO_BASE + P2P_IO_SIZE - 1) & 0x0000F000)
#define	P2P_IO_HI16_BASE_ADRS   ((P2P_IO_BASE & 0xFFFF0000) >> 16)
#define	P2P_IO_HI16_LIMIT_ADRS  ((P2P_IO_BASE + P2P_IO_SIZE - 1) & 0xFFFF0000)
#define P2P_NPMEM_SPACE_BASE_ADRS ((P2P_NONPREF_MEM_BASE & 0xFFFF0000) >> 16)
#define P2P_NPMEM_SPACE_LIMIT_ADRS ((P2P_NONPREF_MEM_BASE + \
                                     P2P_NONPREF_MEM_SIZE - 1) & 0xFFFF0000)
#define P2P_PREF_MEM_BASE_ADRS  ((P2P_PREF_MEM_BASE & 0xFFFF0000) >> 16)
#define P2P_PREF_MEM_LIMIT_ADRS ((P2P_PREF_MEM_BASE + \
				  P2P_PREF_MEM_SIZE - 1) &  0xFFFF0000)

/* DEC 21150 Specific Registers and Values */
#define DC21150_VENDOR_ID    0x1011
#define	DC21150_DEVICE_ID    0x0022
#define PCI_CFG_DEC21150_CHIP_CTRL 0x40
#define PCI_CFG_DEC21150_DIAG_CTRL 0x41
#define PCI_CFG_DEC21150_ARB_CTRL  0x42
#define PCI_CFG_DEC21150_EVNT_DSBL 0x64
#define PCI_CFG_DEC21150_GPIO_DOUT 0x65
#define PCI_CFG_DEC21150_GPIO_CTRL 0x66
#define PCI_CFG_DEC21150_GPIO_DIN  0x67
#define PCI_CFG_DEC21150_SEC_CLK   0x68     /* secondary clock control reg */
#define PCI_CFG_DEC21150_SERR_STAT 0x6A

/* HINT (R) HB4 PCI-PCI Bridge (21150 clone) */
#define HINT_HB4_VENDOR_ID    0x3388
#define HINT_HB4_DEVICE_ID    0x0022

/* Pericom PCI-PCI Bridge (21150 clone) */
#define PERICOM_VENDOR_ID    0x12D8
#define PERICOM_DEVICE_ID    0x8150

/* 
* This macro selects appropriate debug print routine when
* bootrom build with -DBRINGUP option.  Usually it prints
* 4-byte strings to either UART or serial port.
*/
#define BPRINT(str)  \
        ((bringupPrintRtn == NULL) ? OK :        \
         (bringupPrintRtn) ((str) ))

#ifndef _ASMLANGUAGE

/*
 * BMW-related routines
 */

extern VOIDFUNCPTR	bringupPrintRtn;		/* sysLib.c */
extern UINT32	sysMeasureTimebase(void);		/* sysLib.c */
extern void	sysBindFix(void);			/* sysLib.c */

extern void	SEROUT(int ch);				/* sysALib.s */
extern UINT32	vxHid1Get(void);			/* sysALib.s */

extern void	sysSerialPutc(int c);			/* sysSerial.c */
extern int	sysSerialGetc(void);			/* sysSerial.c */
extern void	sysSerialPrintString(char *s);		/* sysSerial.c */
extern void	sysSerialPrintStringNL(char *s);	/* sysSerial.c */
extern void	sysSerialPrintHex(UINT32 val, int cr);	/* sysSerial.c */
int sysTodGetSecond (void);
STATUS sysTodGet (int *pYear, int *pMonth, int *pDay, int *pHour,
    int *pMin, int *pSec);
STATUS sysTodSet (int year, int month, int day, int hour, int min, int sec);
STATUS sysTodInit (UINT8 *addr);
STATUS sysTodWatchdogArm (int usec);

extern int sysVectorIRQ0;

#endif /* !_ASMLANGUAGE */

#endif /* BMW_H */
