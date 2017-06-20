/*
 * Misc utility routines for accessing the SOC Interconnects
 * of Broadcom HNBU chips.
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * $Id: siutils.h,v 1.3 Broadcom SDK $
 */

#ifndef	_siutils_h_
#define	_siutils_h_

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

#if defined(WLC_HIGH) && !defined(WLC_LOW)
typedef struct si_pub si_t;
#else
typedef const struct si_pub si_t;
#endif

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

/* clkctl clk mode */
#define	CLK_FAST		0	/* force fast (pll) clock */
#define	CLK_DYNAMIC		2	/* enable dynamic clock control */

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

/* device path */
#define SI_DEVPATH_BUFSZ	16	/* min buffer size in bytes */

/* SI routine enumeration: to be used by update function with multiple hooks */
#define	SI_DOATTACH	1
#define SI_PCIDOWN	2
#define SI_PCIUP	3

#if defined(BCMINTERNAL) || defined(BCMQT)
#define	ISSIM_ENAB(sih)	((sih)->issim)
#else
#define	ISSIM_ENAB(sih)	0
#endif

/* PMU clock/power control */
#if defined(BCMPMUCTL)
#define PMUCTL_ENAB(sih)	(BCMPMUCTL)
#else
#define PMUCTL_ENAB(sih)	((sih)->cccaps & CC_CAP_PMU)
#endif

/* chipcommon clock/power control (exclusive with PMU's) */
#if defined(BCMPMUCTL) && BCMPMUCTL
#define CCCTL_ENAB(sih)		(0)
#define CCPLL_ENAB(sih)		(0)
#else
#define CCCTL_ENAB(sih)		((sih)->cccaps & CC_CAP_PWR_CTL)
#define CCPLL_ENAB(sih)		((sih)->cccaps & CC_CAP_PLL_MASK)
#endif

typedef void (*gpio_handler_t)(uint32 stat, void *arg);


/* === exported functions === */
extern si_t *si_attach(uint pcidev, osl_t *osh, void *regs, uint bustype,
                       void *sdh, char **vars, uint *varsz);
extern si_t *si_kattach(osl_t *osh);
extern void si_detach(si_t *sih);
extern bool si_pci_war16165(si_t *sih);

extern uint si_corelist(si_t *sih, uint coreid[]);
extern uint si_coreid(si_t *sih);
extern uint si_flag(si_t *sih);
extern uint si_coreidx(si_t *sih);
extern uint si_coreunit(si_t *sih);
extern uint si_corevendor(si_t *sih);
extern uint si_corerev(si_t *sih);
extern void *si_osh(si_t *sih);
extern void si_setosh(si_t *sih, osl_t *osh);
extern uint si_corereg(si_t *sih, uint coreidx, uint regoff, uint mask, uint val);
extern uint8 
    si_corereg8(si_t *sih, uint coreidx, uint regoff, uint8 mask, uint8 val);
extern void *si_coreregs(si_t *sih);
extern uint32 si_core_cflags(si_t *sih, uint32 mask, uint32 val);
extern void si_core_cflags_wo(si_t *sih, uint32 mask, uint32 val);
extern uint32 si_core_sflags(si_t *sih, uint32 mask, uint32 val);
extern bool si_iscoreup(si_t *sih);
extern uint si_findcoreidx(si_t *sih, uint coreid, uint coreunit);
extern void *si_setcoreidx(si_t *sih, uint coreidx);
extern void *si_setcore(si_t *sih, uint coreid, uint coreunit);
extern void *si_switch_core(si_t *sih, uint coreid, uint *origidx, uint *intr_val);
extern void si_restore_core(si_t *sih, uint coreid, uint intr_val);
extern int si_numaddrspaces(si_t *sih);
extern uint32 si_addrspace(si_t *sih, uint asidx);
extern uint32 si_addrspacesize(si_t *sih, uint asidx);
extern int si_corebist(si_t *sih);
extern void si_core_reset(si_t *sih, uint32 bits, uint32 resetbits);
extern void si_core_tofixup(si_t *sih);
extern void si_core_disable(si_t *sih, uint32 bits);
extern char *si_chipname(si_t *sih, char *buf, uint len);
extern uint32 si_clock_rate(uint32 pll_type, uint32 n, uint32 m);
extern uint32 si_clock(si_t *sih);
extern void si_clock_pmu_spuravoid(si_t *sih, bool spuravoid);
extern uint32 si_alp_clock(si_t *sih);
extern uint32 si_ilp_clock(si_t *sih);
extern void si_pci_setup(si_t *sih, uint coremask);
extern void si_pcmcia_init(si_t *sih);
extern void si_setint(si_t *sih, int siflag);
extern bool si_backplane64(si_t *sih);
extern void si_register_intr_callback(si_t *sih, void *intrsoff_fn, void *intrsrestore_fn,
	void *intrsenabled_fn, void *intr_arg);
extern void si_deregister_intr_callback(si_t *sih);
extern void si_clkctl_init(si_t *sih);
extern uint16 si_clkctl_fast_pwrup_delay(si_t *sih);
extern bool si_clkctl_cc(si_t *sih, uint mode);
extern int si_clkctl_xtal(si_t *sih, uint what, bool on);
extern uint32 si_gpiotimerval(si_t *sih, uint32 mask, uint32 val);
extern bool si_backplane64(si_t *sih);
extern void si_btcgpiowar(si_t *sih);
extern bool si_deviceremoved(si_t *sih);
extern uint32 si_socram_size(si_t *sih);

extern void si_watchdog(si_t *sih, uint nticks);
extern void si_watchdog_ms(si_t *sih, uint32 ms);
extern void *si_gpiosetcore(si_t *sih);
extern uint32 si_gpiocontrol(si_t *sih, uint32 mask, uint32 val, uint8 priority);
extern uint32 si_gpioouten(si_t *sih, uint32 mask, uint32 val, uint8 priority);
extern uint32 si_gpioout(si_t *sih, uint32 mask, uint32 val, uint8 priority);
extern uint32 si_gpioin(si_t *sih);
extern uint32 si_gpiointpolarity(si_t *sih, uint32 mask, uint32 val, uint8 priority);
extern uint32 si_gpiointmask(si_t *sih, uint32 mask, uint32 val, uint8 priority);
extern uint32 si_gpioled(si_t *sih, uint32 mask, uint32 val);
extern uint32 si_gpioreserve(si_t *sih, uint32 gpio_num, uint8 priority);
extern uint32 si_gpiorelease(si_t *sih, uint32 gpio_num, uint8 priority);
extern uint32 si_gpiopull(si_t *sih, bool updown, uint32 mask, uint32 val);
extern uint32 si_gpioevent(si_t *sih, uint regtype, uint32 mask, uint32 val);
extern uint32 si_gpio_int_enable(si_t *sih, bool enable);
extern uint32 si_gpiotimer_outmask(si_t *sih, uint32 mask, uint32 val);

/* GPIO event handlers */
extern void *si_gpio_handler_register(si_t *sih, uint32 e, bool lev, gpio_handler_t cb, void *arg);
extern void si_gpio_handler_unregister(si_t *sih, void* gpioh);
extern void si_gpio_handler_process(si_t *sih);

extern int si_spi_select(si_t * sih, uint8 spi_id, int en);
extern uint32 si_spi_ccint_enable(si_t * sih, bool en);
extern uint32 si_spi_modectrl(si_t * sih, uint32 mask, uint32 val);
extern uint32 si_spi_config(si_t * sih, uint32 mask, uint32 val);
extern uint32 si_spi_fifo(si_t * sih, uint32 mask, uint32 val);
extern uint32 si_spi_intr_clear(si_t * sih);
extern uint32 si_spi_status(si_t * sih);

/* Wake-on-wireless-LAN (WOWL) */
extern bool si_pci_pmecap(si_t *sih);
struct osl_info;
extern bool si_pci_fastpmecap(struct osl_info *osh);
extern bool si_pci_pmeclr(si_t *sih);
extern void si_pci_pmeen(si_t *sih);
extern uint si_pcie_readreg(void *sih, uint addrtype, uint offset);

#ifdef BCMSDIO
extern void si_sdio_init(si_t *sih);
#endif

extern uint16 si_d11_devid(si_t *sih);
extern int si_corepciid(si_t *sih, uint func, uint16 *pcivendor, uint16 *pcidevice,
	uint8 *pciclass, uint8 *pcisubclass, uint8 *pciprogif, uint8 *pciheader);

#ifdef BCMECICOEX
extern void *si_eci_init(si_t *sih);
extern void si_eci_notify_bt(si_t *sih, uint32 mask, uint32 val);
#else
#define si_eci_init(sih) (0)
#define si_eci_notify_bt(sih, type, val)  (0)
#endif /* BCMECICOEX */

/* OTP status */
extern bool si_is_otp_disabled(si_t *sih);
extern bool si_is_otp_powered(si_t *sih);
extern void si_otp_power(si_t *sih, bool on);

/* SPROM availability */
extern bool si_is_sprom_available(si_t *sih);

/*
 * Build device path. Path size must be >= SI_DEVPATH_BUFSZ.
 * The returned path is NULL terminated and has trailing '/'.
 * Return 0 on success, nonzero otherwise.
 */
extern int si_devpath(si_t *sih, char *path, int size);
/* Read variable with prepending the devpath to the name */
extern char *si_getdevpathvar(si_t *sih, const char *name);
extern int si_getdevpathintvar(si_t *sih, const char *name);


/* === XXX HW PR WARs === */
extern uint8 si_pcieclkreq(si_t *sih, uint32 mask, uint32 val);
extern void si_war42780_clkreq(si_t *sih, bool clkreq);
extern void si_pci_sleep(si_t *sih);
extern void si_pci_down(si_t *sih);
extern void si_pci_up(si_t *sih);
extern void si_pcie_war_ovr_disable(si_t *sih);
extern void si_pcie_extendL1timer(si_t *sih, bool extend);
extern int si_pci_fixcfg(si_t *sih);
#ifndef BCMDONGLEHOST
extern bool si_ldo_war(si_t *sih, uint devid);
#endif

/* === debug routines === */


#if defined(BCMINTERNAL) || defined(BCMDBG_ASSERT) || defined(BCMDBG_ERR) || defined(BCMDBG_DUMP)
extern bool si_taclear(si_t *sih, bool details);
#endif

#ifdef BCMDBG
extern void si_view(si_t *sih, bool verbose);
extern void si_viewall(si_t *sih, bool verbose);
extern uint32 si_pcielcreg(si_t *sih, uint32 mask, uint32 val);
#endif

#if defined(BCMINTERNAL) && defined(BCMDBG) || defined(BCMDBG_DUMP)
extern void si_dump(si_t *sih, struct bcmstrbuf *b);
extern void si_ccreg_dump(si_t *sih, struct bcmstrbuf *b);
extern void si_clkctl_dump(si_t *sih, struct bcmstrbuf *b);
extern int si_gpiodump(si_t *sih, struct bcmstrbuf *b);
#endif
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
extern void si_dumpregs(si_t *sih, struct bcmstrbuf *b);
#endif
#if defined(BCMINTERNAL) && defined(BCMDBG)
extern int si_dump_pcieregs(si_t *sih, struct bcmstrbuf *b);
#endif

#ifdef BCMINTERNAL
extern uint32 si_pciereg(si_t *sih, uint32 offset, uint32 mask, uint32 val, uint type);
extern uint32 si_pciesbreg(si_t *sih, uint32 offset, uint32 mask, uint32 val);
extern uint32 si_pcieserdesreg(si_t *sih, uint32 mdioslave, uint32 offset, uint32 mask, uint32 val);
extern uint32 si_ccreg(si_t *sih, uint32 offset, uint32 mask, uint32 val);
#endif

#endif	/* _siutils_h_ */
