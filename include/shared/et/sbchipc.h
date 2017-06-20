/*
 * $Id: sbchipc.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SiliconBackplane Chipcommon core hardware definitions.
 *
 * The chipcommon core provides chip identification, SB control,
 * jtag, 0/1/2 uarts, clock frequency control, a watchdog interrupt timer,
 * gpio interface, extbus, and support for serial and parallel flashes.
 *
 */

#ifndef	_SBCHIPC_H
#define	_SBCHIPC_H


/* cpp contortions to concatenate w/arg prescan */
#ifndef PAD
#define	_PADLINE(line)	pad ## line
#define	_XSTR(line)	_PADLINE(line)
#define	PAD		_XSTR(__LINE__)
#endif	/* PAD */

typedef volatile struct {
	uint32	chipid;			/* 0x0 */
	uint32	capabilities;
	uint32	corecontrol;		/* corerev >= 1 */
	uint32	PAD[5];

	/* Interrupt control */
	uint32	intstatus;		/* 0x20 */
	uint32	intmask;
	uint32	chipcontrol;        /* 0x28, rev >= 11 */
	uint32	chipstatus;         /* 0x2c, rev >= 11 */
    
	/* Jtag Master */
	uint32	jtagcmd;            /* 0x30, rev >= 10 */
	uint32	jtagir;
	uint32	jtagdr;
	uint32	jtagctrl;    

	/* serial flash interface registers */
	uint32	flashcontrol;		/* 0x40 */
	uint32	flashaddress;
	uint32	flashdata;
	uint32	PAD[1];

	/* Silicon backplane configuration broadcast control */
	uint32	broadcastaddress;
	uint32	broadcastdata;
	uint32	PAD[2];

	/* gpio - cleared only by power-on-reset */
	uint32	gpioin;			/* 0x60 */
	uint32	gpioout;
	uint32	gpioouten;
	uint32	gpiocontrol;
	uint32	gpiointpolarity;
	uint32	gpiointmask;
	uint32	PAD[2];

	/* Watchdog timer */
	uint32	watchdog;		/* 0x80 */
	uint32	PAD[3];

	/* clock control */
	uint32	clockcontrol_n;		/* 0x90 */
	uint32	clockcontrol_sb;	/* aka m0 */
	uint32	clockcontrol_pci;	/* aka m1 */
	uint32	clockcontrol_m2;	/* mii/uart/mem */
	uint32	clockcontrol_mips;	/* aka m3 */
	uint32	uart_clkdiv;		/* corerev >= 3 */
	uint32	PAD[2];

	/* pll delay registers (corerev >= 4) */
	uint16	pllondelay;		/* 0xb0 */
	uint16	PAD;
	uint16	pllfrefselectdelay;
	uint16	PAD;
	uint32	PAD[17];

        /* In AI chips, pointer to erom */
    uint32  eromptr;            /* 0xfc */

	/* ExtBus control registers (corerev >= 3) */
	uint32	cs01config;		/* 0x100 */
	uint32	cs01memwaitcnt;
	uint32	cs01attrwaitcnt;
	uint32	cs01iowaitcnt;
	uint32	cs23config;
	uint32	cs23memwaitcnt;
	uint32	cs23attrwaitcnt;
	uint32	cs23iowaitcnt;
	uint32	cs4config;
	uint32	cs4waitcnt;
	uint32	parallelflashconfig;
	uint32	parallelflashwaitcnt;
	uint32	PAD[84];

	/* SPI registers (for keystone only) */
    uint32    spi_mode_ctrl;    /* 0x280, SPI Mode Control registers */
    uint32    spi_config;       /* 0x284, SPI Configuration registers */
    uint32    spi_fifo_io;      /* 0x288, SPI FIFO IO registers */
    uint32    spi_status;       /* 0x28c, SPI Status registers */
    uint32    PAD[24];

    /* Serial Interface registers */
    uint32    SERIAL_IO_INTMASK; /* 0x2f0 */
    uint32    SERIAL_IO_SEL;     
    uint32    PAD[2];
    
	/* uarts */
	uint8	uart0data;		/* 0x300 */
	uint8	uart0imr;
	uint8	uart0fcr;
	uint8	uart0lcr;
	uint8	uart0mcr;
	uint8	uart0lsr;
	uint8	uart0msr;
	uint8	uart0scratch;
	uint8	PAD[248];		/* corerev >= 1 */

	uint8	uart1data;		/* 0x400 */
	uint8	uart1imr;
	uint8	uart1fcr;
	uint8	uart1lcr;
	uint8	uart1mcr;
	uint8	uart1lsr;
	uint8	uart1msr;
	uint8	uart1scratch;
    uint32  PAD[126];

    /* PMU registers (corerev >= 20) */
    uint32  pmucontrol;         /* 0x600 */
    uint32  pmucapabilities;
    uint32  pmustatus;
    uint32  PAD[2];
    uint32  pmutimer;
    uint32  min_res_mask;       /* legacy */
    uint32  max_res_mask;       /* legacy */
    uint32  res_table_sel;
    uint32  res_dep_mask;
    uint32  res_updn_timer;
    uint32  PAD[2];
    uint32  pmuwatchdog;        /* 0x634 */
    uint32  PAD[6];
    uint32  chipcontrol_addr;   /* 0x650 */
    uint32  chipcontrol_data;   /* 0x654 */
    uint32  PAD[2];
    uint32  pllcontrol_addr;    /* 0x660 */
    uint32  pllcontrol_data;    /* 0x664 */
} chipcregs_t;

/* chipid */
#define	CID_ID_MASK		0x0000ffff		/* Chip Id mask */
#define	CID_REV_MASK		0x000f0000		/* Chip Revision mask */
#define	CID_REV_SHIFT		16			/* Chip Revision shift */
#define	CID_PKG_MASK		0x00f00000		/* Package Option mask */
#define	CID_PKG_SHIFT		20			/* Package Option shift */
#define	CID_CC_MASK		0x0f000000		/* CoreCount (corerev >= 4) */
#define CID_CC_SHIFT		24
#define	CID_TYPE_MASK		0xf0000000	/* Chip Type */
#define CID_TYPE_SHIFT		28

/* capabilities */
#define	CAP_UARTS_MASK		0x00000003		/* Number of uarts */
#define CAP_MIPSEB		0x00000004		/* MIPS is in big-endian mode */
#define CAP_UCLKSEL		0x00000018		/* UARTs clock select */
#define CAP_UINTCLK		0x00000008		/* UARTs are driven by internal divided clock */
#define CAP_UARTGPIO		0x00000020		/* UARTs own Gpio's 15:12 */
#define CAP_EXTBUS		0x00000040		/* External bus present */
#define	CAP_FLASH_MASK		0x00000700		/* Type of flash */
#define	CAP_PLL_MASK		0x00030000		/* Type of PLL */

/* PLL capabilities */
#define PLL_NONE		0x00000000
#define PLL_N3M			0x00010000
#define PLL_N4M			0x00020000

/* corecontrol */
#define CC_UARTCLKO		0x00000001		/* Drive UART with internal clock */
#define	CC_SE			0x00000002		/* sync clk out enable (corerev >= 3) */

/* chipstatus */
#define CHIPSTAT_PO_MASK    0x1 /* Package Option */
#define CHIPSTAT_PO_SHIFT   0
#define CHIPSTAT_PCIE1_DIS_MASK    0x20 /* PCIE1 disable */
#define CHIPSTAT_PCIE1_DIS_SHIFT   5
#define CHIPSTAT_PCIE0_MODE_MASK    0x2000 /* PCIE0 mode */
#define CHIPSTAT_PCIE0_MODE_SHIFT   13
#define CHIPSTAT_CPU_FREQ_MASK    0x1C000000 /* CPU freq adjust */
#define CHIPSTAT_CPU_FREQ_SHIFT   26
#define CHIPSTAT_CPU_FREQ_600M      0x0
#define CHIPSTAT_CPU_FREQ_587M      0x1
#define CHIPSTAT_CPU_FREQ_575M      0x2
#define CHIPSTAT_CPU_FREQ_562M      0x3
#define CHIPSTAT_CPU_FREQ_550M      0x4
#define CHIPSTAT_CPU_FREQ_537M      0x5
#define CHIPSTAT_CPU_FREQ_525M      0x6
#define CHIPSTAT_CPU_FREQ_512M      0x7

/* intstatus/intmask */
#define	CI_EI			0x00000002		/* ro: ext intr pin (corerev >= 3) */

/* clockcontrol_n */
#define	CN_N1_MASK		0x3f			/* n1 control */
#define	CN_N2_MASK		0x1f00			/* n2 control */
#define	CN_N2_SHIFT		8

/* clockcontrol_sb/pci/uart */
#define	CC_M1_MASK		0x3f			/* m1 control */
#define	CC_M2_MASK		0x1f00			/* m2 control */
#define	CC_M2_SHIFT		8
#define	CC_M3_MASK		0x3f0000		/* m3 control */
#define	CC_M3_SHIFT		16
#define	CC_MC_MASK		0x1f000000		/* mux control */
#define	CC_MC_SHIFT		24

/* N3M Clock control values for 125Mhz */
#define	CC_125_N		0x0802			/* Default values for bcm4310 */
#define	CC_125_M		0x04020009
#define	CC_125_M25		0x11090009
#define	CC_125_M33		0x11090005

/* N3M Clock control magic field values */
#define	CC_F6_2			0x02			/* A factor of 2 in */
#define	CC_F6_3			0x03			/* 6-bit fields like */
#define	CC_F6_4			0x05			/* N1, M1 or M3 */
#define	CC_F6_5			0x09
#define	CC_F6_6			0x11
#define	CC_F6_7			0x21

#define	CC_F5_BIAS		5			/* 5-bit fields get this added */

#define	CC_MC_BYPASS		0x08
#define	CC_MC_M1		0x04
#define	CC_MC_M1M2		0x02
#define	CC_MC_M1M2M3		0x01
#define	CC_MC_M1M3		0x11

/* N4M Clock control magic field values */
#define	CC_N4M_BIAS		2			/* n1, n2, m1 & m3 bias */
#define	CC_N4M2_BIAS		3			/* m2 bias */

#define	CC_N4MC_M1BYP		1
#define	CC_N4MC_M2BYP		2
#define	CC_N4MC_M3BYP		4

/* Common clock base */
#define	CC_CLOCK_BASE		24000000		/* Half the clock freq */

/* Flash types in the chipcommon capabilities register */
#define FLASH_NONE		0x000		/* No flash */
#define SFLASH_ST		0x100		/* ST serial flash */
#define SFLASH_AT		0x200		/* Atmel serial flash */
#define	PFLASH			0x700		/* Parallel flash */

/* Start/busy bit in flashcontrol */
#define SFLASH_START		0x80000000
#define SFLASH_BUSY		SFLASH_START

/* flashcontrol opcodes for ST flashes */
#define SFLASH_ST_WREN		0x0006		/* Write Enable */
#define SFLASH_ST_WRDIS		0x0004		/* Write Disable */
#define SFLASH_ST_RDSR		0x0105		/* Read Status Register */
#define SFLASH_ST_WRSR		0x0101		/* Write Status Register */
#define SFLASH_ST_READ		0x0303		/* Read Data Bytes */
#define SFLASH_ST_PP		0x0302		/* Page Program */
#define SFLASH_ST_SE		0x02d8		/* Sector Erase */
#define SFLASH_ST_BE		0x00c7		/* Bulk Erase */
#define SFLASH_ST_DP		0x00b9		/* Deep Power-down */
#define SFLASH_ST_RES		0x03ab		/* Read Electronic Signature */

/* Status register bits for ST flashes */
#define SFLASH_ST_WIP		0x01		/* Write In Progress */
#define SFLASH_ST_WEL		0x02		/* Write Enable Latch */
#define SFLASH_ST_BP_MASK	0x1c		/* Block Protect */
#define SFLASH_ST_BP_SHIFT	2
#define SFLASH_ST_SRWD		0x80		/* Status Register Write Disable */

/* flashcontrol opcodes for Atmel flashes */
#define SFLASH_AT_READ				0x07e8
#define SFLASH_AT_PAGE_READ			0x07d2
/* PR9631: impossible to specify Atmel Buffer Read command */
#define SFLASH_AT_BUF1_READ
#define SFLASH_AT_BUF2_READ
#define SFLASH_AT_STATUS			0x01d7
#define SFLASH_AT_BUF1_WRITE			0x0384
#define SFLASH_AT_BUF2_WRITE			0x0387
#define SFLASH_AT_BUF1_ERASE_PROGRAM		0x0283
#define SFLASH_AT_BUF2_ERASE_PROGRAM		0x0286
#define SFLASH_AT_BUF1_PROGRAM			0x0288
#define SFLASH_AT_BUF2_PROGRAM			0x0289
#define SFLASH_AT_PAGE_ERASE			0x0281
#define SFLASH_AT_BLOCK_ERASE			0x0250
#define SFLASH_AT_BUF1_WRITE_ERASE_PROGRAM	0x0382
#define SFLASH_AT_BUF2_WRITE_ERASE_PROGRAM	0x0385
#define SFLASH_AT_BUF1_LOAD			0x0253
#define SFLASH_AT_BUF2_LOAD			0x0255
#define SFLASH_AT_BUF1_COMPARE			0x0260
#define SFLASH_AT_BUF2_COMPARE			0x0261
#define SFLASH_AT_BUF1_REPROGRAM		0x0258
#define SFLASH_AT_BUF2_REPROGRAM		0x0259

/* Status register bits for Atmel flashes */
#define SFLASH_AT_READY				0x80
#define SFLASH_AT_MISMATCH			0x40
#define SFLASH_AT_ID_MASK			0x38
#define SFLASH_AT_ID_SHIFT			3

#ifndef _LANGUAGE_ASSEMBLY
/* SPI device ID  :
 *  - Keystone has three SPI device selected.
 */
typedef enum cc_spi_id_e {
    CC_SPI_SS0 = 0,
    CC_SPI_SS1,
    CC_SPI_SS2,
    CC_SPI_NUM_DEV
}cc_spi_id_t;

/* SPI Mode :
 *  - Keystone has four SPI mode control.
 */
typedef enum cc_spi_mode_e {
    CC_SPI_MODE_CPOL_0_CPHA_0 = 0,
    CC_SPI_MODE_CPOL_0_CPHA_1,
    CC_SPI_MODE_CPOL_1_CPHA_0,
    CC_SPI_MODE_CPOL_1_CPHA_1,
    CC_SPI_NUM_MODE
}cc_spi_mode_t;
#endif /* _LANGUAGE_ASSEMBLY */

#define CC_SPI_ID_IS_VALID(_id)    ((_id) < CC_SPI_NUM_DEV)

#define CC_SPI_MODE_IS_VALID(_id)    ((_id) < CC_SPI_NUM_MODE)

/* SPI reg-spi_mode_ctrl :field defintiion */
#define CC_SPIMCTRL_MODE_MASK     0x3        /* SPI mode valid bits */
#define CC_SPIMCTRL_MODE_0     0x0        /* SPI mode : CPOL = 0, CPHA = 0 */
#define CC_SPIMCTRL_MODE_1     0x1        /* SPI mode : CPOL = 0, CPHA = 1 */
#define CC_SPIMCTRL_MODE_2     0x2        /* SPI mode : CPOL = 1, CPHA = 0 */
#define CC_SPIMCTRL_MODE_3     0x3        /* SPI mode : CPOL = 1, CPHA = 1 */
#define CC_SPIMCTRL_ACKEN_MASK     0x4        /* SPI RACK enable valid bits */
#define CC_SPIMCTRL_ACKEN     0x4        /* SPI RACK */
#define CC_SPIMCTRL_ENDIAN_MASK     0x8        /* SPI endiag valid bits */
#define CC_SPIMCTRL_BE     0x8        /* SPI endiag : big endian mode */
#define CC_SPIMCTRL_LE     0x0         /* SPI endiag : little endian mode */
#define CC_SPIMCTRL_LSB_MASK     0x100        /* SPI LSB first valid bits */
#define CC_SPIMCTRL_LSB_FIRST     0x100        /* SPI LSB : big endian mode */
#define CC_SPIMCTRL_MSB_FIRST    0x0         /* SPI LSB : little endian mode */
#define CC_SPIMCTRL_CLK_MASK     0xf0        /* SPI clock divider valid bits */

/* SPI reg-spi_config :field defintiion */
#define CC_SPICFG_MASK     0x9f87e1fb        /* SPI config valid bits */   

/* SPI reg-spi_fifo_io :field defintiion */
#define CC_SPIFIFOIO_MASK     0xffffffff        /* SPI FIFO IO valid bits */   

/* SPI reg-spi_status :field defintiion */
#define CC_SPISTS_MASK     0x37        /* SPI status valid bits */
#define CC_SPISTS_READY    0x1        /* SPI ready */
#define CC_SPISTS_RDONE    0x2       /* SPI transaction done and FIFO is empty when reading data */
#define CC_SPISTS_INTFLAG   0x4    /* SPI Interrupt Flag */
#define CC_SPISTS_FIFOE    0x10      /* SPI FIFO empty */
#define CC_SPISTS_FIFOF  0x20       /* SPI FIFO full */

/* Serial IO eanble MASK and bit value definition */
#define CC_SERIAL_IO_SPI_INTR_MASK       0x4
#define CC_SERIAL_IO_SPI0_MASK       0x4
#define CC_SERIAL_IO_SPI1_MASK       0xc
#define CC_SERIAL_IO_SPI2_MASK       0x1c

#endif	/* _SBCHIPC_H */
