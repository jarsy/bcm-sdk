/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * BCM47XX Sonics SiliconBackplane DDR/SDRAM controller core hardware definitions.
 *
 * $Id: sbmemc.h,v 1.4 2011/01/19 19:44:18 hamilton Exp $
 */

#ifndef	_SBMEMC_H
#define	_SBMEMC_H

#ifdef _LANGUAGE_ASSEMBLY

#define	MEMC_CONTROL		0x00
#define	MEMC_CONFIG		0x04
#define	MEMC_REFRESH		0x08
#define	MEMC_BISTSTAT		0x0c
#define	MEMC_MODEBUF		0x10
#define	MEMC_BKCLS		0x14
#define	MEMC_PRIORINV		0x18
#define	MEMC_DRAMTIM		0x1c
#define	MEMC_INTSTAT		0x20
#define	MEMC_INTMASK		0x24
#define	MEMC_INTINFO		0x28
#define	MEMC_NCDLCTL		0x30
#define	MEMC_RDNCDLCOR		0x34
#define	MEMC_WRNCDLCOR		0x38
#define	MEMC_MISCDLYCTL		0x3c
#define	MEMC_DQSGATENCDL	0x40
#define	MEMC_SPARE		0x44
#define	MEMC_TPADDR		0x48
#define	MEMC_TPDATA		0x4c
#define	MEMC_BARRIER		0x50
#define	MEMC_CORE		0x54


#else

/* Sonics side: MEMC core registers */
typedef volatile struct sbmemcregs {
	uint32	control;
	uint32	config;
	uint32	refresh;
	uint32	biststat;
	uint32	modebuf;
	uint32	bkcls;
	uint32	priorinv;
	uint32	dramtim;
	uint32	intstat;
	uint32	intmask;
	uint32	intinfo;
	uint32	reserved1;
	uint32	ncdlctl;
	uint32	rdncdlcor;
	uint32	wrncdlcor;
	uint32	miscdlyctl;
	uint32	dqsgatencdl;
	uint32	spare;
	uint32	tpaddr;
	uint32	tpdata;
	uint32	barrier;
	uint32	core;
} sbmemcregs_t;

#endif

/* MEMC Core Init values (OCP ID 0x80f) */

#ifdef _BCM5365_FPGA_
/* This is for SDRM16MX32X2 */
#define MEMC_SD_CONFIG_INIT	0x0004900c
#define MEMC_SD_DRAMTIM_INIT	0x000754d8
#define MEMC_SD_RDNCDLCOR_INIT	0x14141414
#define MEMC_SD_WRNCDLCOR_INIT	0x00000000
#define MEMC_SD_MISCDLYCTL_INIT	0x30000000
#define MEMC_SD_CONTROL_INIT0	0x00000002
#define MEMC_SD_CONTROL_INIT1	0x00000008
#define MEMC_SD_CONTROL_INIT2	0x00000004
#define MEMC_SD_CONTROL_INIT3	0x00000010
#define MEMC_SD_CONTROL_INIT4	0x00000001
#define MEMC_SD_MODEBUF_INIT	0x00000022
#define MEMC_SD_REFRESH_INIT	0x0000840f
#else 
/* This is for SDRM16MX16X2 - Micron M48LC16M16A2 -75A */
/* Default SDRAM type on BCM5365P, BCM5365R */
#define MEMC_SD_CONFIG_INIT	0x0004b000 /* + */
#define MEMC_SD_DRAMTIM_INIT	0x000754d8
#define MEMC_SD_RDNCDLCOR_INIT	0x14141414
#define MEMC_SD_WRNCDLCOR_INIT	0x00000000
#define MEMC_SD_MISCDLYCTL_INIT	0x20000000 /* + */
#define MEMC_SD_CONTROL_INIT0	0x00000002
#define MEMC_SD_CONTROL_INIT1	0x00000008
#define MEMC_SD_CONTROL_INIT2	0x00000004
#define MEMC_SD_CONTROL_INIT3	0x00000010
#define MEMC_SD_CONTROL_INIT4	0x00000001
/*#define MEMC_SD_MODEBUF_INIT	0x00000022 */ /* Burst=010/4 word */
#define MEMC_SD_MODEBUF_INIT	0x00000023 /* Burst=011/8 word, max perf */
#define MEMC_SD_REFRESH_INIT	0x0000840f
#endif /* !_BCM5365_FPGA_ */

/* For ddr: */
#define MEMC_CONFIG_INIT	0x00048000
#define MEMC_DRAMTIM_INIT	0x000754d9
#define MEMC_RDNCDLCOR_INIT	0x05050505
#define MEMC_WRNCDLCOR_INIT	0x49351205
#define MEMC_DQSGATENCDL_INIT	0x0003000a
#define MEMC_MISCDLYCTL_INIT	0x21061c1b
#define MEMC_NCDLCTL_INIT	0x00002001
#define MEMC_CONTROL_INIT0	0x00000002
#define MEMC_CONTROL_INIT1	0x00000008
#define MEMC_MODEBUF_INIT0	0x00004000
#define MEMC_CONTROL_INIT2	0x00000010
#define MEMC_MODEBUF_INIT1	0x00000100
#define MEMC_CONTROL_INIT3	0x00000010
#define MEMC_CONTROL_INIT4	0x00000008
#define MEMC_REFRESH_INIT	0x00008300 /* was 0x0000840f */
#define MEMC_CONTROL_INIT5	0x00000004
#define MEMC_MODEBUF_INIT2	0x00000000
#define MEMC_CONTROL_INIT6	0x00000010
#define MEMC_CONTROL_INIT7	0x00000001

#ifdef DDR16MX16X2 
#warning "Memory configured for DDR16MX16X2"
/* This is for DDRM16X16X2 (BU4704) */
#define	MEMC_DDR_INIT		0x0009 /* 0x48009 */
#define	MEMC_DDR_MODE		0x62
/* This is for DDR64MX16X2 (CPCI) */
#define	MEMC_DDR_INIT_256MB		0x0011 /* 0x48011 */
#else
/* This is for DDRM32MX8X4 (95365K) */
/* Micron MT46V16M8 DDR */
#define	MEMC_DDR_INIT		0x0011  /* Becomes 0x48011 */
/*#define MEMC_DDR_MODE		0x62 */ /* Burst = 4 */
#define MEMC_DDR_MODE		0x63  /* Burst = 8 */
#endif

/* Micron 256Mb x16: SDRM16MX16X2: BCM95365P/BCM95365R */
#define MEMC_SDR_INIT           0x000c /* Becomes 4b00c */
#define MEMC_SDR_MODE           0x0023 /* Burst = 8 */

#endif	/* _SBMEMC_H */
