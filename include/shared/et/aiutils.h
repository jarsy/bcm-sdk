/*
 * $Id: aiutils.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef	_AIUTILS_H_
#define	_AIUTILS_H_
/*
 * Data structure to export all chip specific common variables
 *   public (read-only) portion of siutils handle returned by si_attach()/si_kattach()
 */
struct si_pub {
	uint	socitype;		/* SOCI_SB, SOCI_AI */

	uint	bustype;		/* SI_BUS, PCI_BUS */
	uint	buscoretype;		/* PCI_CORE_ID, PCIE_CORE_ID, PCMCIA_CORE_ID */
	uint	buscorerev;		/* buscore rev */
	uint	buscoreidx;		/* buscore index */
	int	ccrev;			/* chip common core rev */
	uint32	cccaps;			/* chip common capabilities */
	int	pmurev;			/* pmu core rev */
	uint32	pmucaps;		/* pmu capabilities */
	uint	boardtype;		/* board type */
	uint	boardvendor;		/* board vendor */
	uint	boardflags;		/* board flags */
	uint	chip;			/* chip number */
	uint	chiprev;		/* chip revision */
	uint	chippkg;		/* chip package option */
	uint32	chipst;			/* chip status */
	bool	issim;			/* chip is in simulation or emulation */
	uint    socirev;		/* SOC interconnect rev */
	bool	pci_pr32414;		/* XXX whether 432414 WAR applis to the chip */
};


typedef const struct si_pub si_t;
/*
 * Many of the routines below take an 'sih' handle as their first arg.
 * Allocate this by calling si_attach().  Free it by calling si_detach().
 * At any one time, the sih is logically focused on one particular si core
 * (the "current core").
 * Use si_setcore() or si_setcoreidx() to change the association to another core.
 */
#define	SI_OSH		NULL	/* Use for si_kattach when no osh is available */

/* clkctl xtal what flags */
#define	XTAL			0x1	/* primary crystal oscillator (2050) */
#define	PLL			0x2	/* main chip pll */

/* SI routine enumeration: to be used by update function with multiple hooks */
#define	SI_DOATTACH	1
#define SI_PCIDOWN	2
#define SI_PCIUP	3

/* GPIO usage priorities */
#define GPIO_DRV_PRIORITY	0	/* Driver */
#define GPIO_APP_PRIORITY	1	/* Application */
#define GPIO_HI_PRIORITY	2	/* Highest priority. Ignore GPIO reservation */

/* GPIO pull up/down */
#define GPIO_PULLUP		0
#define GPIO_PULLDN		1

/* GPIO event regtype */
#define GPIO_REGEVT		0	/* GPIO register event */
#define GPIO_REGEVT_INTMSK	1	/* GPIO register event int mask */
#define GPIO_REGEVT_INTPOL	2	/* GPIO register event int polarity */

#define	CC_CAP_PMU		0x10000000	/* PMU Present, rev >= 20 */
/* PMU clock/power control */
#if defined(BCMPMUCTL)
#define PMUCTL_ENAB(sih)	(BCMPMUCTL)
#else
#define PMUCTL_ENAB(sih)	((sih)->cccaps & CC_CAP_PMU)
#endif

typedef void (*gpio_handler_t)(uint32 stat, void *arg);

/* bcmdefs.h */
/* Bus types */
#define	SI_BUS			0	/* SOC Interconnenct */
#define	PCI_BUS			1	/* PCI target */
#define	PCMCIA_BUS		2	/* PCMCIA target */


#define BUSTYPE(bus) 	(bus)
#define CHIPID(chip)	(chip)

/* Defines for DMA Address Width - Shared between OSL and HNDDMA */
#define DMADDR_MASK_32 0x0		/* Address mask for 32-bits */
#define DMADDR_MASK_30 0xc0000000	/* Address mask for 30-bits */
#define DMADDR_MASK_0  0xffffffff	/* Address mask for 0-bits (hi-part) */

#define	DMADDRWIDTH_30  30 /* 30-bit addressing capability */
#define	DMADDRWIDTH_32  32 /* 32-bit addressing capability */
#define	DMADDRWIDTH_63  63 /* 64-bit addressing capability */
#define	DMADDRWIDTH_64  64 /* 64-bit addressing capability */

/* packet headroom necessary to accomodate the largest header in the system, (i.e TXOFF).
 * By doing, we avoid the need  to allocate an extra buffer for the header when bridging to WL.
 * There is a compile time check in wlc.c which ensure that this value is at least as big
 * as TXOFF. This value is used in dma_rxfill (hnddma.c).
 */
#define BCMEXTRAHDROOM 160

#define SI_MAXCORES         32          /* Max cores (this is arbitrary, for software
                                         * convenience and could be changed if we
                                         * make any larger chips */
#define GMAC_CORE_ID        0x52d       /* Gigabit MAC core */
#define GMAC_COM_CORE_ID    0x5dc       /* gmac common core */
#define PCIE_CORE_ID        0x820       /* pci express core */
#define DEF_AI_COMP         0xfff       /* Default component, in ai chips it maps all
                                         * unused address ranges
                                         */
#define GMAC_CORE_ID_NS        0x82d       /* Northstar's Gigabit MAC core */


/* === exported functions === */
extern si_t *ai_soc_attach(uint devid, void *osh, void *regs,
                       uint bustype, void *sdh, char **vars, uint *varsz);
extern si_t *ai_soc_kattach(void *osh);
extern void ai_soc_detach(si_t *sih);
extern uint ai_soc_coreid(si_t *sih);
extern uint ai_soc_coreidx(si_t *sih);
extern uint ai_soc_coreunit(si_t *sih);
extern uint ai_soc_corerev(si_t *sih);
extern uint ai_soc_findcoreidx(si_t *sih, uint coreid, uint coreunit);
extern uint ai_soc_clock(si_t *sih);
extern void *ai_soc_setcoreidx(si_t *sih, uint coreidx);
extern void *ai_soc_setcore(si_t *sih, uint coreid, uint coreunit);
extern bool ai_soc_iscoreup(si_t *sih);
extern void ai_soc_core_disable(si_t *sih, uint32 bits);
extern void ai_soc_core_reset(si_t *sih, uint32 bits, uint32 resetbits);
extern int ai_soc_spi_select(si_t * sih, uint8 spi_id, int en);
extern int ai_soc_spi_ccint_enable(si_t * sih, bool en);
extern int ai_soc_spi_modectrl(si_t * sih, uint32 mask, uint32 val);
extern int ai_soc_spi_config(si_t * sih, uint32 mask, uint32 val);
extern uint32 ai_soc_spi_fifo(si_t * sih, uint32 mask, uint32 val);
extern int ai_soc_spi_intr_clear(si_t *sih);
extern int ai_soc_spi_status(si_t *sih);
extern int ai_soc_spi_freq_set(si_t *sih, uint32 speed_hz);

#endif	/* _AIUTILS_H_ */
