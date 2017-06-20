/*
 * Misc utility routines for accessing PMU corerev specific features
 * of the SiliconBackplane-based Broadcom chips.
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * $Id: hndpmu.c,v 1.2 Broadcom SDK $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmdevs.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <hndpmu.h>

/* debug/trace */
#ifdef BCMDBG_ERR
#define	PMU_ERROR(args)	printf args
#else
#define	PMU_ERROR(args)
#endif	/* BCMDBG_ERR */

#ifdef BCMDBG
#define	PMU_MSG(args)	printf args
#else
#define	PMU_MSG(args)
#endif	/* BCMDBG */

#ifndef	BCMDONGLEHOST
/* PLL controls/clocks */
static void si_pmu0_pllinit0(si_t *sih, osl_t *osh, chipcregs_t *cc, uint32 xtal);
static void si_pmu1_pllinit0(si_t *sih, osl_t *osh, chipcregs_t *cc, uint32 xtal);
static uint32 si_pmu0_alpclk0(si_t *sih, osl_t *osh, chipcregs_t *cc);
static uint32 si_pmu0_cpuclk0(si_t *sih, osl_t *osh, chipcregs_t *cc);
static uint32 si_pmu1_cpuclk0(si_t *sih, osl_t *osh, chipcregs_t *cc);
static uint32 si_pmu1_alpclk0(si_t *sih, osl_t *osh, chipcregs_t *cc);

/* PMU resources */
static bool si_pmu_res_depfltr_bb(si_t *sih);
static bool si_pmu_res_depfltr_nbb(si_t *sih);
static bool si_pmu_res_depfltr_ncb(si_t *sih);
#ifdef UNRELEASEDCHIP
static bool si_pmu_res_depfltr_paldo(si_t *sih);
static bool si_pmu_res_depfltr_npaldo(si_t *sih);
#endif
static uint32 si_pmu_res_deps(si_t *sih, osl_t *osh, chipcregs_t *cc, uint32 rsrcs, bool all);
static uint si_pmu_res_uptime(si_t *sih, osl_t *osh, chipcregs_t *cc, uint8 rsrc);
#endif	/* !BCMDONGLEHOST */

static void si_pmu_res_masks(si_t *sih, uint32 *pmin, uint32 *pmax);

#ifndef	BCMDONGLEHOST
/* FVCO frequency */
#define FVCO_880	880000	/* 880MHz */
#define FVCO_1760	1760000	/* 1760MHz */
#define FVCO_1440	1440000	/* 1440MHz */

/* Read/write a chipcontrol reg */
void
si_pmu_chipcontrol(si_t *sih, uint reg, uint32 mask, uint32 val)
{
	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_addr), ~0, reg);
	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_data), mask, val);
}

/* Setup switcher voltage */
void
BCMINITFN(si_pmu_set_switcher_voltage)(si_t *sih, osl_t *osh,
                                       uint8 bb_voltage, uint8 rf_voltage)
{
	chipcregs_t *cc;
	uint origidx;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	/* XXX: define proper constants in the header */
	W_REG(osh, &cc->regcontrol_addr, 0x01);
	W_REG(osh, &cc->regcontrol_data, (uint32)(bb_voltage & 0x1f) << 22);

	/* XXX: define proper constants in the header */
	W_REG(osh, &cc->regcontrol_addr, 0x00);
	W_REG(osh, &cc->regcontrol_data, (uint32)(rf_voltage & 0x1f) << 14);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

void
BCMINITFN(si_pmu_set_ldo_voltage)(si_t *sih, osl_t *osh, uint8 ldo, uint8 voltage)
{
	uint8 sr_cntl_shift = 0, rc_shift = 0, shift = 0, mask = 0;
	uint8 addr = 0;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	switch (CHIPID(sih->chip)) {
	case BCM4328_CHIP_ID:
	case BCM5354_CHIP_ID:
		switch (ldo) {
		case SET_LDO_VOLTAGE_LDO1:
			addr = 2;
			sr_cntl_shift = 8;
			rc_shift = 17;
			mask = 0xf;
			break;
		case SET_LDO_VOLTAGE_LDO2:
			addr = 3;
			rc_shift = 1;
			mask = 0xf;
			break;
		case SET_LDO_VOLTAGE_LDO3:
			addr = 3;
			rc_shift = 9;
			mask = 0xf;
			break;
		case SET_LDO_VOLTAGE_PAREF:
			addr = 3;
			rc_shift = 17;
			mask = 0x3f;
			break;
		default:
			ASSERT(FALSE);
			return;
		}
		break;
	case BCM4312_CHIP_ID:
		switch (ldo) {
		case SET_LDO_VOLTAGE_PAREF:
			addr = 0;
			rc_shift = 21;
			mask = 0x3f;
			break;
		default:
			ASSERT(FALSE);
			return;
		}
		break;
	case BCM4325_CHIP_ID:
		switch (ldo) {
		case SET_LDO_VOLTAGE_CLDO_PWM:
			addr = 5;
			rc_shift = 9;
			mask = 0xf;
			break;
#if 0
		case SET_LDO_VOLTAGE_CLDO_BURST:
			addr = 5;
			rc_shift = 13;
			mask = 0xf;
			break;
#endif
		case SET_LDO_VOLTAGE_CBUCK_PWM:
			addr = 3;
			rc_shift = 20;
			mask = 0x1f;
			/* Bit 116 & 119 are inverted in CLB for opt 2b */
			if (((sih->chipst & CST4325_PMUTOP_2B_MASK) >>
			     CST4325_PMUTOP_2B_SHIFT) == 1)
				voltage ^= 0x9;
			break;
#if 0
		case SET_LDO_VOLTAGE_CBUCK_BURST:
			addr = 3;
			rc_shift = 25;
			mask = 0x1f;
			break;
#endif
		default:
			ASSERT(FALSE);
			return;
		}
		break;
	default:
		ASSERT(FALSE);
		return;
	}

	shift = sr_cntl_shift + rc_shift;

	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, regcontrol_addr),
	           ~0, addr);
	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, regcontrol_data),
	           mask << shift, (voltage & mask) << shift);
}

void
si_pmu_paref_ldo_enable(si_t *sih, osl_t *osh, bool enable)
{
	uint ldo = 0;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	switch (CHIPID(sih->chip)) {
	case BCM4328_CHIP_ID:
		ldo = RES4328_PA_REF_LDO;
		break;
	case BCM5354_CHIP_ID:
		ldo = RES5354_PA_REF_LDO;
		break;
	case BCM4312_CHIP_ID:
		ldo = RES4312_PA_REF_LDO;
		break;
	default:
		return;
	}

	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, min_res_mask),
	           PMURES_BIT(ldo), enable ? PMURES_BIT(ldo) : 0);
}

/* d11 slow to fast clock transition time in slow clock cycles */
#define D11SCC_SLOW2FAST_TRANSITION	2

uint16
BCMINITFN(si_pmu_fast_pwrup_delay)(si_t *sih, osl_t *osh)
{
	uint delay = PMU_MAX_TRANSITION_DLY;
	chipcregs_t *cc;
	uint origidx;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	/* XXX: Should be calculated based on the PMU updown/depend tables */
	switch (CHIPID(sih->chip)) {
	case BCM4328_CHIP_ID:
		delay = 7000;
		break;
	case BCM4325_CHIP_ID:
		if (ISSIM_ENAB(sih))
			delay = 70;
		else {
			uint32 ilp = si_ilp_clock(sih);
			delay = (si_pmu_res_uptime(sih, osh, cc, RES4325_HT_AVAIL) +
			         D11SCC_SLOW2FAST_TRANSITION) * ((1000000 + ilp - 1) / ilp);
		}
		break;
#ifdef UNRELEASEDCHIP
	case BCM4315_CHIP_ID:
		if (ISSIM_ENAB(sih))
			delay = 70;
		else {
			uint32 ilp = si_ilp_clock(sih);
			delay = (si_pmu_res_uptime(sih, osh, cc, RES4315_HT_AVAIL) +
			         D11SCC_SLOW2FAST_TRANSITION) * ((1000000 + ilp - 1) / ilp);
		}
		break;
	case BCM4319_CHIP_ID:
		delay = ISSIM_ENAB(sih) ? 70 : 3000;
		break;
#endif	/* UNRELEASEDCHIP */
	case BCM4312_CHIP_ID:
		delay = 7000;
		break;
	case BCM4322_CHIP_ID:
		/* XXX TBD */
		delay = 7000;
		break;

	default:
		break;
	}

	PMU_MSG(("si_pmu_fast_pwrup_delay: chip %x rev %d delay %d\n",
	         sih->chip, sih->chiprev, delay));

	/* Return to original core */
	si_setcoreidx(sih, origidx);

	return (uint16)delay;
}

uint32
BCMINITFN(si_pmu_force_ilp)(si_t *sih, osl_t *osh, bool force)
{
	chipcregs_t *cc;
	uint origidx;
	uint32 oldpmucontrol;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	oldpmucontrol = R_REG(osh, &cc->pmucontrol);
	if (force)
		W_REG(osh, &cc->pmucontrol, oldpmucontrol &
			~(PCTL_HT_REQ_EN | PCTL_ALP_REQ_EN));
	else
		W_REG(osh, &cc->pmucontrol, oldpmucontrol |
			(PCTL_HT_REQ_EN | PCTL_ALP_REQ_EN));

	/* Return to original core */
	si_setcoreidx(sih, origidx);

	return oldpmucontrol;
}

/* Setup resource up/down timers */
typedef struct {
	uint8 resnum;
	uint16 updown;
} pmu_res_updown_t;

/* Change resource dependancies masks */
typedef struct {
	uint32 res_mask;		/* resources (chip specific) */
	int8 action;			/* action */
	uint32 depend_mask;		/* changes to the dependancies mask */
	bool (*filter)(si_t *sih);	/* action is taken when filter is NULL or return TRUE */
} pmu_res_depend_t;

/* Resource dependancies mask change action */
#define RES_DEPEND_SET		0	/* Override the dependancies mask */
#define RES_DEPEND_ADD		1	/* Add to the  dependancies mask */
#define RES_DEPEND_REMOVE	-1	/* Remove from the dependancies mask */

/* XXX PMU corerev 0 resource used in BCM4328 */
static const pmu_res_updown_t BCMINITDATA(bcm4328a0_res_updown)[] = {
	{ RES4328_EXT_SWITCHER_PWM, 0x0101 },
	{ RES4328_BB_SWITCHER_PWM, 0x1f01 },
	{ RES4328_BB_SWITCHER_BURST, 0x010f },
	{ RES4328_BB_EXT_SWITCHER_BURST, 0x0101 },
	{ RES4328_ILP_REQUEST, 0x0202 },
	{ RES4328_RADIO_SWITCHER_PWM, 0x0f01 },
	{ RES4328_RADIO_SWITCHER_BURST, 0x0f01 },
	{ RES4328_ROM_SWITCH, 0x0101 },
	{ RES4328_PA_REF_LDO, 0x0f01 },
	{ RES4328_RADIO_LDO, 0x0f01 },
	{ RES4328_AFE_LDO, 0x0f01 },
	{ RES4328_PLL_LDO, 0x0f01 },
	{ RES4328_BG_FILTBYP, 0x0101 },
	{ RES4328_TX_FILTBYP, 0x0101 },
	{ RES4328_RX_FILTBYP, 0x0101 },
	{ RES4328_XTAL_PU, 0x0101 },
	{ RES4328_XTAL_EN, 0xa001 },
	{ RES4328_BB_PLL_FILTBYP, 0x0101 },
	{ RES4328_RF_PLL_FILTBYP, 0x0101 },
	{ RES4328_BB_PLL_PU, 0x0701 }
};

static const pmu_res_depend_t BCMINITDATA(bcm4328a0_res_depend)[] = {
	/* Adjust ILP request resource not to force ext/BB switchers into burst mode */
	{
		PMURES_BIT(RES4328_ILP_REQUEST),
		RES_DEPEND_SET,
		PMURES_BIT(RES4328_EXT_SWITCHER_PWM) | PMURES_BIT(RES4328_BB_SWITCHER_PWM),
		NULL
	}
};

static const pmu_res_updown_t BCMINITDATA(bcm4325a0_res_updown_qt)[] = {
	{ RES4325_HT_AVAIL, 0x0300 },
	{ RES4325_BBPLL_PWRSW_PU, 0x0101 },
	{ RES4325_RFPLL_PWRSW_PU, 0x0101 },
	{ RES4325_ALP_AVAIL, 0x0100 },
	{ RES4325_XTAL_PU, 0x1000 },
	{ RES4325_LNLDO1_PU, 0x0800 },
	{ RES4325_CLDO_CBUCK_PWM, 0x0101 },
	{ RES4325_CBUCK_PWM, 0x0803 }
};

static const pmu_res_updown_t BCMINITDATA(bcm4325a0_res_updown)[] = {
	{ RES4325_XTAL_PU, 0x1501 }
};

static const pmu_res_depend_t BCMINITDATA(bcm4325a0_res_depend)[] = {
	/* Adjust LNLDO2 PU resource dependencies - remove BB BURST if power topology
	 * does not use BB to provide LNLDO2 voltage.
	 */
	{
		PMURES_BIT(RES4325_LNLDO2_PU),
		RES_DEPEND_REMOVE,
		PMURES_BIT(RES4325_BUCK_BOOST_BURST),
		si_pmu_res_depfltr_nbb
	},
	/* Adjust ALP/HT Avail resource dependencies - bring up BB along if it is used. */
	{
		PMURES_BIT(RES4325_ALP_AVAIL) | PMURES_BIT(RES4325_HT_AVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4325_BUCK_BOOST_BURST) | PMURES_BIT(RES4325_BUCK_BOOST_PWM),
		si_pmu_res_depfltr_bb
	},
	/* Adjust HT Avail resource dependencies - bring up RF switches along with HT. */
	{
		PMURES_BIT(RES4325_HT_AVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4325_RX_PWRSW_PU) | PMURES_BIT(RES4325_TX_PWRSW_PU) |
		PMURES_BIT(RES4325_LOGEN_PWRSW_PU) | PMURES_BIT(RES4325_AFE_PWRSW_PU),
		NULL
	},
	/* Adjust ALL resource dependencies - remove CBUCK dependancies if it is not used. */
	{
		PMURES_BIT(RES4325_ILP_REQUEST) | PMURES_BIT(RES4325_ABUCK_BURST) |
		PMURES_BIT(RES4325_ABUCK_PWM) | PMURES_BIT(RES4325_LNLDO1_PU) |
		PMURES_BIT(RES4325_LNLDO4_PU) | PMURES_BIT(RES4325_XTAL_PU) |
		PMURES_BIT(RES4325_ALP_AVAIL) | PMURES_BIT(RES4325_RX_PWRSW_PU) |
		PMURES_BIT(RES4325_TX_PWRSW_PU) | PMURES_BIT(RES4325_RFPLL_PWRSW_PU) |
		PMURES_BIT(RES4325_LOGEN_PWRSW_PU) | PMURES_BIT(RES4325_AFE_PWRSW_PU) |
		PMURES_BIT(RES4325_BBPLL_PWRSW_PU) | PMURES_BIT(RES4325_HT_AVAIL),
		RES_DEPEND_REMOVE,
		PMURES_BIT(RES4325B0_CBUCK_LPOM) | PMURES_BIT(RES4325B0_CBUCK_BURST) |
		PMURES_BIT(RES4325B0_CBUCK_PWM),
		si_pmu_res_depfltr_ncb
	}
};

#ifdef UNRELEASEDCHIP
static const pmu_res_updown_t BCMINITDATA(bcm4315a0_res_updown_qt)[] = {
	{ RES4315_HT_AVAIL, 0x0101 },
	{ RES4315_XTAL_PU, 0x0100 },
	{ RES4315_LNLDO1_PU, 0x0100 },
	{ RES4315_PALDO_PU, 0x0100 },
	{ RES4315_CLDO_PU, 0x0100 },
	{ RES4315_CBUCK_PWM, 0x0100 },
	{ RES4315_CBUCK_BURST, 0x0100 },
	{ RES4315_CBUCK_LPOM, 0x0100 }
};

static const pmu_res_updown_t BCMINITDATA(bcm4315a0_res_updown)[] = {
	{ RES4315_XTAL_PU, 0x2501 }
};

static const pmu_res_depend_t BCMINITDATA(bcm4315a0_res_depend)[] = {
	/* Adjust OTP PU resource dependencies - not need PALDO unless write */
	{
		PMURES_BIT(RES4315_OTP_PU),
		RES_DEPEND_REMOVE,
		PMURES_BIT(RES4315_PALDO_PU),
		si_pmu_res_depfltr_npaldo
	},
	/* Adjust ALP/HT Avail resource dependencies - bring up PALDO along if it is used. */
	{
		PMURES_BIT(RES4315_ALP_AVAIL) | PMURES_BIT(RES4315_HT_AVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4315_PALDO_PU),
		si_pmu_res_depfltr_paldo
	},
	/* Adjust HT Avail resource dependencies - bring up RF switches along with HT. */
	{
		PMURES_BIT(RES4315_HT_AVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4315_RX_PWRSW_PU) | PMURES_BIT(RES4315_TX_PWRSW_PU) |
		PMURES_BIT(RES4315_LOGEN_PWRSW_PU) | PMURES_BIT(RES4315_AFE_PWRSW_PU),
		NULL
	},
	/* Adjust ALL resource dependencies - remove CBUCK dependancies if it is not used. */
	{
		PMURES_BIT(RES4315_CLDO_PU) | PMURES_BIT(RES4315_ILP_REQUEST) |
		PMURES_BIT(RES4315_LNLDO1_PU) | PMURES_BIT(RES4315_OTP_PU) |
		PMURES_BIT(RES4315_LNLDO2_PU) | PMURES_BIT(RES4315_XTAL_PU) |
		PMURES_BIT(RES4315_ALP_AVAIL) | PMURES_BIT(RES4315_RX_PWRSW_PU) |
		PMURES_BIT(RES4315_TX_PWRSW_PU) | PMURES_BIT(RES4315_RFPLL_PWRSW_PU) |
		PMURES_BIT(RES4315_LOGEN_PWRSW_PU) | PMURES_BIT(RES4315_AFE_PWRSW_PU) |
		PMURES_BIT(RES4315_BBPLL_PWRSW_PU) | PMURES_BIT(RES4315_HT_AVAIL),
		RES_DEPEND_REMOVE,
		PMURES_BIT(RES4315_CBUCK_LPOM) | PMURES_BIT(RES4315_CBUCK_BURST) |
		PMURES_BIT(RES4315_CBUCK_PWM),
		si_pmu_res_depfltr_ncb
	}
};
#endif	/* UNRELEASEDCHIP */

#ifdef UNRELEASEDCHIP
static const pmu_res_updown_t BCMINITDATA(bcm4319a0_res_updown_qt)[] = {
	{ RES4319_HT_AVAIL, 0x0101 },
	{ RES4319_XTAL_PU, 0x0100 },
	{ RES4319_LNLDO1_PU, 0x0100 },
	{ RES4319_PALDO_PU, 0x0100 },
	{ RES4319_CLDO_PU, 0x0100 },
	{ RES4319_CBUCK_PWM, 0x0100 },
	{ RES4319_CBUCK_BURST, 0x0100 },
	{ RES4319_CBUCK_LPOM, 0x0100 }
};

static const pmu_res_updown_t BCMINITDATA(bcm4319a0_res_updown)[] = {
	{ RES4319_XTAL_PU, 0x1501 }
};

static const pmu_res_depend_t BCMINITDATA(bcm4319a0_res_depend)[] = {
#ifndef BCMUSBDEV
	/* Adjust OTP PU resource dependencies - not need PALDO unless write */
	{
		PMURES_BIT(RES4319_OTP_PU),
		RES_DEPEND_REMOVE,
		PMURES_BIT(RES4319_PALDO_PU),
		NULL
	},
#endif
	/* Adjust HT Avail resource dependencies - bring up RF switches along with HT. */
	{
		PMURES_BIT(RES4319_HT_AVAIL),
		RES_DEPEND_ADD,
		PMURES_BIT(RES4319_RX_PWRSW_PU) | PMURES_BIT(RES4319_TX_PWRSW_PU) |
		PMURES_BIT(RES4319_LOGEN_PWRSW_PU) | PMURES_BIT(RES4319_AFE_PWRSW_PU),
		NULL
	}
};
#endif	/* UNRELEASEDCHIP */

/* TRUE if the power topology uses the buck boost to provide 3.3V to VDDIO_RF and WLAN PA */
static bool
BCMINITFN(si_pmu_res_depfltr_bb)(si_t *sih)
{
	return (sih->boardflags & BFL_BUCKBOOST) != 0;
}

/* TRUE if the power topology doesn't use the buck boost */
static bool
BCMINITFN(si_pmu_res_depfltr_nbb)(si_t *sih)
{
	return (sih->boardflags & BFL_BUCKBOOST) == 0;
}

/* TRUE if the power topology doesn't use the cbuck. Key on chiprev also if the chip is BCM4325. */
static bool
BCMINITFN(si_pmu_res_depfltr_ncb)(si_t *sih)
{
	if (CHIPID(sih->chip) == BCM4325_CHIP_ID)
		return (sih->chiprev >= 2) && ((sih->boardflags & BFL_NOCBUCK) != 0);
	return ((sih->boardflags & BFL_NOCBUCK) != 0);
}

#ifdef UNRELEASEDCHIP
/* TRUE if the power topology uses the PALDO */
static bool
BCMINITFN(si_pmu_res_depfltr_paldo)(si_t *sih)
{
	return (sih->boardflags & BFL_PALDO) != 0;
}

/* TRUE if the power topology doesn't use the PALDO */
static bool
BCMINITFN(si_pmu_res_depfltr_npaldo)(si_t *sih)
{
	return (sih->boardflags & BFL_PALDO) == 0;
}
#endif /* UNRELEASEDCHIP */
#endif /* !BCMDONGLEHOST */

/* XXX A list of BCM4325 boards that use buck boost to power VDDIO_SD. We can't turn off
 * buck boost for these boards.
 */
#define BCM94325_BBVDDIOSD_BOARDS(sih) (sih->boardtype == BCM94325DEVBU_BOARD || \
					sih->boardtype == BCM94325BGABU_BOARD)

/* Determine min/max rsrc masks. Value 0 leaves hardware at default. */
static void
si_pmu_res_masks(si_t *sih, uint32 *pmin, uint32 *pmax)
{
	uint32 min_mask = 0, max_mask = 0;
	uint rsrcs;
	char *val;

	/* # resources */
	rsrcs = (sih->pmucaps & PCAP_RC_MASK) >> PCAP_RC_SHIFT;

	/* determine min/max rsrc masks */
	switch (CHIPID(sih->chip)) {
	case BCM4328_CHIP_ID:
		/* Down to ILP request */
		min_mask = PMURES_BIT(RES4328_EXT_SWITCHER_PWM) |
		        PMURES_BIT(RES4328_BB_SWITCHER_PWM) |
		        PMURES_BIT(RES4328_XTAL_EN);
#ifdef BCMROMOFFLOAD
		min_mask |= PMURES_BIT(RES4328_ROM_SWITCH);
#endif
		/* Allow (but don't require) PLL to turn on */
		max_mask = 0xfffff;
		break;
	case BCM5354_CHIP_ID:
		/* Allow (but don't require) PLL to turn on */
		max_mask = 0xfffff;
		break;
	case BCM4325_CHIP_ID:
		/* Leave OTP powered up and power it down later when there is no one needs it */
		if (sih->chiprev < 2) {
			min_mask = PMURES_BIT(RES4325_CBUCK_BURST) |
			        PMURES_BIT(RES4325_LNLDO2_PU);
			if (((sih->chipst & CST4325_PMUTOP_2B_MASK) >>
			     CST4325_PMUTOP_2B_SHIFT) == 1)
				min_mask |= PMURES_BIT(RES4325_CLDO_CBUCK_BURST);
		}
		/* Power down the OTP as well and everything we need from it are cacehed */
		else {
			if (!(sih->boardflags & BFL_NOCBUCK))
				min_mask = PMURES_BIT(RES4325B0_CBUCK_LPOM);
			if (((sih->chipst & CST4325_PMUTOP_2B_MASK) >>
			     CST4325_PMUTOP_2B_SHIFT) == 1)
			        min_mask |= PMURES_BIT(RES4325B0_CLDO_PU);
		}
		/* Leave buck boost on in burst mode for certain boards */
		if ((sih->boardflags & BFL_BUCKBOOST) && (BCM94325_BBVDDIOSD_BOARDS(sih)))
			min_mask |= PMURES_BIT(RES4325_BUCK_BOOST_BURST);
		/* Allow all resources to be turned on upon requests */
		max_mask = ~(~0 << rsrcs);
		break;
#ifdef UNRELEASEDCHIP
	case BCM4329_CHIP_ID:
		 /* min_mask = 0x2fe63e; */
		min_mask =
		 PMURES_BIT(RES4329_CBUCK_LPOM) |
		 PMURES_BIT(RES4329_CBUCK_BURST) |
		 PMURES_BIT(RES4329_CBUCK_PWM) |
		 PMURES_BIT(RES4329_CLDO_PU) |
		 PMURES_BIT(RES4329_PALDO_PU) |
		 PMURES_BIT(RES4329_LNLDO1_PU) |
		 PMURES_BIT(RES4329_OTP_PU) |
		 PMURES_BIT(RES4329_XTAL_PU) |
		 PMURES_BIT(RES4329_ALP_AVAIL) |
		 PMURES_BIT(RES4329_RX_PWRSW_PU) |
		 PMURES_BIT(RES4329_TX_PWRSW_PU) |
		 PMURES_BIT(RES4329_RFPLL_PWRSW_PU) |
		 PMURES_BIT(RES4329_LOGEN_PWRSW_PU) |
		 PMURES_BIT(RES4329_AFE_PWRSW_PU) |
		 PMURES_BIT(RES4329_HT_AVAIL);
		/* Allow (but don't require) PLL to turn on */
		max_mask = 0x3fffff;
		break;
	case BCM4315_CHIP_ID:
		/* We only need a few resources to be kept on all the time */
		if (!(sih->boardflags & BFL_NOCBUCK))
			min_mask = PMURES_BIT(RES4315_CBUCK_LPOM);
		min_mask |= PMURES_BIT(RES4315_CLDO_PU);
		/* Allow everything else to be turned on upon requests */
		max_mask = ~(~0 << rsrcs);
		break;
	case BCM4319_CHIP_ID:
		/* We only need a few resources to be kept on all the time */
		min_mask = PMURES_BIT(RES4319_CBUCK_LPOM);
		min_mask |= PMURES_BIT(RES4319_CLDO_PU);
		min_mask |= PMURES_BIT(RES4319_PALDO_PU);
		/* Allow everything else to be turned on upon requests */
		max_mask = ~(~0 << rsrcs);
		break;
#endif	/* UNRELEASEDCHIP */
	case BCM4312_CHIP_ID:
		/* default min_mask = 0x80000cbb is wrong */
		min_mask = 0xcbb;
		/*
		 * max_mask = 0x7fff;
		 * pmu_res_updown_table_sz = 0;
		 * pmu_res_depend_table_sz = 0;
		 */
		break;
	case BCM4322_CHIP_ID:
		if (sih->chiprev < 2) {
			/* clear OTP_PU as it's not applicable, request ALP(can skip for A1) */
			min_mask = PMURES_BIT(RES4322_RF_LDO) |	PMURES_BIT(RES4322_XTAL_PU)
				| PMURES_BIT(RES4322_ALP_AVAIL);
			max_mask = 0xff;
		}
		break;

	default:
		break;
	}

	/* Apply nvram override to min mask */
	if ((val = getvar(NULL, "rmin")) != NULL) {
		PMU_MSG(("Applying rmin=%s to min_mask\n", val));
		min_mask = (uint32)bcm_strtoul(val, NULL, 0);
	}
	/* Apply nvram override to max mask */
	if ((val = getvar(NULL, "rmax")) != NULL) {
		PMU_MSG(("Applying rmax=%s to max_mask\n", val));
		max_mask = (uint32)bcm_strtoul(val, NULL, 0);
	}

	*pmin = min_mask;
	*pmax = max_mask;
}

#ifndef	BCMDONGLEHOST
/* initialize PMU resources */
void
BCMINITFN(si_pmu_res_init)(si_t *sih, osl_t *osh)
{
	chipcregs_t *cc;
	uint origidx;
	const pmu_res_updown_t *pmu_res_updown_table = NULL;
	uint pmu_res_updown_table_sz = 0;
	const pmu_res_depend_t *pmu_res_depend_table = NULL;
	uint pmu_res_depend_table_sz = 0;
	uint32 min_mask = 0, max_mask = 0;
	char name[8], *val;
	uint i, rsrcs;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	switch (CHIPID(sih->chip)) {
	case BCM4328_CHIP_ID:
		/* XXX: For now always use rev0 table */
		pmu_res_updown_table = bcm4328a0_res_updown;
		pmu_res_updown_table_sz = ARRAYSIZE(bcm4328a0_res_updown);
		pmu_res_depend_table = bcm4328a0_res_depend;
		pmu_res_depend_table_sz = ARRAYSIZE(bcm4328a0_res_depend);
		break;
	case BCM4325_CHIP_ID:
		/* Optimize resources up/down timers */
		if (ISSIM_ENAB(sih)) {
			pmu_res_updown_table = bcm4325a0_res_updown_qt;
			pmu_res_updown_table_sz = ARRAYSIZE(bcm4325a0_res_updown_qt);
		} else {
			pmu_res_updown_table = bcm4325a0_res_updown;
			pmu_res_updown_table_sz = ARRAYSIZE(bcm4325a0_res_updown);
		}
		/* Optimize resources dependancies */
		pmu_res_depend_table = bcm4325a0_res_depend;
		pmu_res_depend_table_sz = ARRAYSIZE(bcm4325a0_res_depend);
		break;
#ifdef UNRELEASEDCHIP
	case BCM4315_CHIP_ID:
		/* Optimize resources up/down timers */
		if (ISSIM_ENAB(sih)) {
			pmu_res_updown_table = bcm4315a0_res_updown_qt;
			pmu_res_updown_table_sz = ARRAYSIZE(bcm4315a0_res_updown_qt);
		}
		else {
			pmu_res_updown_table = bcm4315a0_res_updown;
			pmu_res_updown_table_sz = ARRAYSIZE(bcm4315a0_res_updown);
		}
		/* Optimize resources dependancies masks */
		pmu_res_depend_table = bcm4315a0_res_depend;
		pmu_res_depend_table_sz = ARRAYSIZE(bcm4315a0_res_depend);
		break;
	case BCM4319_CHIP_ID:
		/* Optimize resources up/down timers */
		if (ISSIM_ENAB(sih)) {
			pmu_res_updown_table = bcm4319a0_res_updown_qt;
			pmu_res_updown_table_sz = ARRAYSIZE(bcm4319a0_res_updown_qt);
		}
		else {
			pmu_res_updown_table = NULL;
			pmu_res_updown_table_sz = 0;
		}
		/* Optimize resources dependancies masks */
		pmu_res_depend_table = bcm4319a0_res_depend;
		pmu_res_depend_table_sz = ARRAYSIZE(bcm4319a0_res_depend);
		break;
#endif	/* UNRELEASEDCHIP */

	default:
		break;
	}

	/* # resources */
	rsrcs = (sih->pmucaps & PCAP_RC_MASK) >> PCAP_RC_SHIFT;

	/* Program up/down timers */
	while (pmu_res_updown_table_sz--) {
		ASSERT(pmu_res_updown_table != NULL);
		PMU_MSG(("Changing rsrc %d res_updn_timer to 0x%x\n",
		         pmu_res_updown_table[pmu_res_updown_table_sz].resnum,
		         pmu_res_updown_table[pmu_res_updown_table_sz].updown));
		W_REG(osh, &cc->res_table_sel,
		      pmu_res_updown_table[pmu_res_updown_table_sz].resnum);
		W_REG(osh, &cc->res_updn_timer,
		      pmu_res_updown_table[pmu_res_updown_table_sz].updown);
	}
	/* Apply nvram overrides to up/down timers */
	for (i = 0; i < rsrcs; i ++) {
		snprintf(name, sizeof(name), "r%dt", i);
		if ((val = getvar(NULL, name)) == NULL)
			continue;
		PMU_MSG(("Applying %s=%s to rsrc %d res_updn_timer\n", name, val, i));
		W_REG(osh, &cc->res_table_sel, (uint32)i);
		W_REG(osh, &cc->res_updn_timer, (uint32)bcm_strtoul(val, NULL, 0));
	}

	/* Program resource dependencies table */
	while (pmu_res_depend_table_sz--) {
		ASSERT(pmu_res_depend_table != NULL);
		if (pmu_res_depend_table[pmu_res_depend_table_sz].filter != NULL &&
		    !(pmu_res_depend_table[pmu_res_depend_table_sz].filter)(sih))
			continue;
		for (i = 0; i < rsrcs; i ++) {
			if ((pmu_res_depend_table[pmu_res_depend_table_sz].res_mask &
			     PMURES_BIT(i)) == 0)
				continue;
			W_REG(osh, &cc->res_table_sel, i);
			switch (pmu_res_depend_table[pmu_res_depend_table_sz].action) {
			case RES_DEPEND_SET:
				PMU_MSG(("Changing rsrc %d res_dep_mask to 0x%x\n", i,
				    pmu_res_depend_table[pmu_res_depend_table_sz].depend_mask));
				W_REG(osh, &cc->res_dep_mask,
				      pmu_res_depend_table[pmu_res_depend_table_sz].depend_mask);
				break;
			case RES_DEPEND_ADD:
				PMU_MSG(("Adding 0x%x to rsrc %d res_dep_mask\n",
				    pmu_res_depend_table[pmu_res_depend_table_sz].depend_mask, i));
				OR_REG(osh, &cc->res_dep_mask,
				       pmu_res_depend_table[pmu_res_depend_table_sz].depend_mask);
				break;
			case RES_DEPEND_REMOVE:
				PMU_MSG(("Removing 0x%x from rsrc %d res_dep_mask\n",
				    pmu_res_depend_table[pmu_res_depend_table_sz].depend_mask, i));
				AND_REG(osh, &cc->res_dep_mask,
				        ~pmu_res_depend_table[pmu_res_depend_table_sz].depend_mask);
				break;
			default:
				ASSERT(0);
				break;
			}
		}
	}
	/* Apply nvram overrides to dependancies masks */
	for (i = 0; i < rsrcs; i ++) {
		snprintf(name, sizeof(name), "r%dd", i);
		if ((val = getvar(NULL, name)) == NULL)
			continue;
		PMU_MSG(("Applying %s=%s to rsrc %d res_dep_mask\n", name, val, i));
		W_REG(osh, &cc->res_table_sel, (uint32)i);
		W_REG(osh, &cc->res_dep_mask, (uint32)bcm_strtoul(val, NULL, 0));
	}

	/* Determine min/max rsrc masks */
	si_pmu_res_masks(sih, &min_mask, &max_mask);

	/* Program min resource mask */
	if (min_mask) {
		PMU_MSG(("Changing min_res_mask to 0x%x\n", min_mask));
		W_REG(osh, &cc->min_res_mask, min_mask);
	}
	/* Program max resource mask */
	if (max_mask) {
		PMU_MSG(("Changing max_res_mask to 0x%x\n", max_mask));
		W_REG(osh, &cc->max_res_mask, max_mask);
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

/* XXX PMU corerev 0 pll programming for BCM4328 BCM5354 */
/* setup pll and query clock speed */
typedef struct {
	uint16	freq;
	uint8	xf;
	uint8	wbint;
	uint32	wbfrac;
} pmu0_xtaltab0_t;

/* the following table is based on 880Mhz fvco */
static const pmu0_xtaltab0_t BCMINITDATA(pmu0_xtaltab0)[] = {
	{ 12000,	1,	73,	349525 },
	{ 13000,	2,	67,	725937 },
	{ 14400,	3,	61,	116508 },
	{ 15360,	4,	57,	305834 },
	{ 16200,	5,	54,	336579 },
	{ 16800,	6,	52,	399457 },
	{ 19200,	7,	45,	873813 },
	{ 19800,	8,	44,	466033 },
	{ 20000,	9,	44,	0 },
	{ 25000,	10,	70,	419430 },
	{ 26000,	11,	67,	725937 },
	{ 30000,	12,	58,	699050 },
	{ 38400,	13,	45,	873813 },
	{ 40000,	14,	45,	0 },
	{ 0,		0,	0,	0 }
};

#ifdef BCMUSBDEV
#define	PMU0_XTAL0_DEFAULT	11
#else
#define PMU0_XTAL0_DEFAULT	8
#endif

#ifdef BCMUSBDEV
/*
 * Set new backplane PLL clock frequency
 */
static void
BCMINITFN(si_pmu0_sbclk4328)(si_t *sih, int freq)
{
	uint32 tmp, oldmax, oldmin, origidx;
	chipcregs_t *cc;

	/* Remember original core before switch to chipc */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc);

	/* Set new backplane PLL clock */
	W_REG(osh, &cc->pllcontrol_addr, PMU0_PLL0_PLLCTL0);
	tmp = R_REG(osh, &cc->pllcontrol_data);
	tmp &= ~(PMU0_PLL0_PC0_DIV_ARM_MASK);
	tmp |= freq << PMU0_PLL0_PC0_DIV_ARM_SHIFT;
	W_REG(osh, &cc->pllcontrol_data, tmp);

	/* Power cycle BB_PLL_PU by disabling/enabling it to take on new freq */
	/* Disable PLL */
	oldmin = R_REG(osh, &cc->min_res_mask);
	oldmax = R_REG(osh, &cc->max_res_mask);
	W_REG(osh, &cc->min_res_mask, oldmin & ~PMURES_BIT(RES4328_BB_PLL_PU));
	W_REG(osh, &cc->max_res_mask, oldmax & ~PMURES_BIT(RES4328_BB_PLL_PU));

	/* It takes over several hundred usec to re-enable the PLL since the
	 * sequencer state machines run on ILP clock. Set delay at 450us to be safe.
	 *
	 * Be sure PLL is powered down first before re-enabling it.
	 */

	/* While in ILP, need delay (min 32us) before accessing pmu.res_state
	 *  register (PR 41610)
	 */
	OSL_DELAY(PLL_DELAY);
	SPINWAIT((R_REG(osh, &cc->res_state) & PMURES_BIT(RES4328_BB_PLL_PU)), PLL_DELAY*3);
	if (R_REG(osh, &cc->res_state) & PMURES_BIT(RES4328_BB_PLL_PU)) {
		/* If BB_PLL not powered down yet, new backplane PLL clock
		 *  may not take effect.
		 *
		 * Still early during bootup so no serial output here.
		 */
		PMU_ERROR(("Fatal: BB_PLL not power down yet!\n"));
		ASSERT(!(R_REG(osh, &cc->res_state) & PMURES_BIT(RES4328_BB_PLL_PU)));
	}

	/* Enable PLL */
	W_REG(osh, &cc->max_res_mask, oldmax);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}
#endif /* BCMUSBDEV */

/* Set up PLL registers in the PMU as per the crystal speed.
 * Uses xtalfreq variable, or passed-in default.
 */
static void
BCMINITFN(si_pmu0_pllinit0)(si_t *sih, osl_t *osh, chipcregs_t *cc, uint32 xtal)
{
	uint32 tmp;
	const pmu0_xtaltab0_t *xt;

	/* Find the frequency in the table */
	for (xt = pmu0_xtaltab0; xt->freq; xt ++)
		if (xt->freq == xtal)
			break;
	if (xt->freq == 0)
		xt = &pmu0_xtaltab0[PMU0_XTAL0_DEFAULT];

	PMU_MSG(("XTAL %d.%d MHz (%d)\n", xtal / 1000, xtal % 1000, xt->xf));

	/* Check current PLL state */
	tmp = (R_REG(osh, &cc->pmucontrol) & PCTL_XTALFREQ_MASK) >>
	        PCTL_XTALFREQ_SHIFT;
	if (tmp == xt->xf) {
		PMU_MSG(("PLL already programmed for %d.%d MHz\n",
		         xt->freq / 1000, xt->freq % 1000));
#ifdef BCMUSBDEV
		/* PR 46012: USB core non-responsive on 4328
		 *  HW synchronization failure between USB app_clk and phy_clk causing USB
		 *  device core to become non-responsive to all host requests; Need a
		 *   core reset to recover from this hung state.
		 *  The app_clk runs at backplane PLL clk (80Mhz) instead of recommended 90Mhz.
		 *
		 *  Work-around is to increase the backplane PLL clk from 80Mhz to 88Mhz to align
		 *     more closely with recommended speed.
		 */
		if (CHIPID(sih->chip) == BCM4328_CHIP_ID)
			si_pmu0_sbclk4328(sih, PMU0_PLL0_PC0_DIV_ARM_88MHZ);
#endif	/* BCMUSBDEV */
		return;
	}

	if (tmp) {
		PMU_MSG(("Reprogramming PLL for %d.%d MHz (was %d.%dMHz)\n",
		         xt->freq / 1000, xt->freq % 1000,
		         pmu0_xtaltab0[tmp-1].freq / 1000, pmu0_xtaltab0[tmp-1].freq % 1000));
	} else {
		PMU_MSG(("Programming PLL for %d.%d MHz\n",
		         xt->freq / 1000, xt->freq % 1000));
	}

	/* Make sure the PLL is off */
	switch (CHIPID(sih->chip)) {
	case BCM4328_CHIP_ID:
		AND_REG(osh, &cc->min_res_mask, ~PMURES_BIT(RES4328_BB_PLL_PU));
		AND_REG(osh, &cc->max_res_mask, ~PMURES_BIT(RES4328_BB_PLL_PU));
		break;
	case BCM5354_CHIP_ID:
		AND_REG(osh, &cc->min_res_mask, ~PMURES_BIT(RES5354_BB_PLL_PU));
		AND_REG(osh, &cc->max_res_mask, ~PMURES_BIT(RES5354_BB_PLL_PU));
		break;
	default:
		ASSERT(0);
	}
	SPINWAIT(R_REG(osh, &cc->clk_ctl_st) & CCS0_HTAVAIL, PMU_MAX_TRANSITION_DLY);
	ASSERT(!(R_REG(osh, &cc->clk_ctl_st) & CCS0_HTAVAIL));

	PMU_MSG(("Done masking\n"));

	/* Write PDIV in pllcontrol[0] */
	W_REG(osh, &cc->pllcontrol_addr, PMU0_PLL0_PLLCTL0);
	tmp = R_REG(osh, &cc->pllcontrol_data);
	if (xt->freq >= PMU0_PLL0_PC0_PDIV_FREQ)
		tmp |= PMU0_PLL0_PC0_PDIV_MASK;
	else
		tmp &= ~PMU0_PLL0_PC0_PDIV_MASK;
	W_REG(osh, &cc->pllcontrol_data, tmp);

	/* Write WILD in pllcontrol[1] */
	W_REG(osh, &cc->pllcontrol_addr, PMU0_PLL0_PLLCTL1);
	tmp = R_REG(osh, &cc->pllcontrol_data);
	tmp = ((tmp & ~(PMU0_PLL0_PC1_WILD_INT_MASK | PMU0_PLL0_PC1_WILD_FRAC_MASK)) |
	       (((xt->wbint << PMU0_PLL0_PC1_WILD_INT_SHIFT) &
	         PMU0_PLL0_PC1_WILD_INT_MASK) |
	        ((xt->wbfrac << PMU0_PLL0_PC1_WILD_FRAC_SHIFT) &
	         PMU0_PLL0_PC1_WILD_FRAC_MASK)));
	if (xt->wbfrac == 0)
		tmp |= PMU0_PLL0_PC1_STOP_MOD;
	else
		tmp &= ~PMU0_PLL0_PC1_STOP_MOD;
	W_REG(osh, &cc->pllcontrol_data, tmp);

	/* Write WILD in pllcontrol[2] */
	W_REG(osh, &cc->pllcontrol_addr, PMU0_PLL0_PLLCTL2);
	tmp = R_REG(osh, &cc->pllcontrol_data);
	tmp = ((tmp & ~PMU0_PLL0_PC2_WILD_INT_MASK) |
	       ((xt->wbint >> PMU0_PLL0_PC2_WILD_INT_SHIFT) &
	        PMU0_PLL0_PC2_WILD_INT_MASK));
	W_REG(osh, &cc->pllcontrol_data, tmp);

	PMU_MSG(("Done pll\n"));

	/* Write XtalFreq. Set the divisor also. */
	tmp = R_REG(osh, &cc->pmucontrol);
	tmp = ((tmp & ~PCTL_ILP_DIV_MASK) |
	       (((((xt->freq + 127) / 128) - 1) << PCTL_ILP_DIV_SHIFT) & PCTL_ILP_DIV_MASK));
	tmp = ((tmp & ~PCTL_XTALFREQ_MASK) |
	       ((xt->xf << PCTL_XTALFREQ_SHIFT) & PCTL_XTALFREQ_MASK));
	W_REG(osh, &cc->pmucontrol, tmp);
}

/* query alp/xtal clock frequency */
static uint32
BCMINITFN(si_pmu0_alpclk0)(si_t *sih, osl_t *osh, chipcregs_t *cc)
{
	const pmu0_xtaltab0_t *xt;
	uint32 xf;

	/* Find the frequency in the table */
	xf = (R_REG(osh, &cc->pmucontrol) & PCTL_XTALFREQ_MASK) >>
	        PCTL_XTALFREQ_SHIFT;
	for (xt = pmu0_xtaltab0; xt->freq; xt++)
		if (xt->xf == xf)
			break;
	/* PLL must be configured before */
	ASSERT(xt->freq);

	return xt->freq * 1000;
}

/* query CPU clock frequency */
static uint32
BCMINITFN(si_pmu0_cpuclk0)(si_t *sih, osl_t *osh, chipcregs_t *cc)
{
	uint32 tmp, divarm;
#ifdef BCMDBG
	uint32 pdiv, wbint, wbfrac, fvco;
	uint32 freq;
#endif
	uint32 FVCO = FVCO_880;

	/* Read divarm from pllcontrol[0] */
	W_REG(osh, &cc->pllcontrol_addr, PMU0_PLL0_PLLCTL0);
	tmp = R_REG(osh, &cc->pllcontrol_data);
	divarm = (tmp & PMU0_PLL0_PC0_DIV_ARM_MASK) >> PMU0_PLL0_PC0_DIV_ARM_SHIFT;

#ifdef BCMDBG
	/* Calculate fvco based on xtal freq, pdiv, and wild */
	pdiv = tmp & PMU0_PLL0_PC0_PDIV_MASK;

	W_REG(osh, &cc->pllcontrol_addr, PMU0_PLL0_PLLCTL1);
	tmp = R_REG(osh, &cc->pllcontrol_data);
	wbfrac = (tmp & PMU0_PLL0_PC1_WILD_FRAC_MASK) >> PMU0_PLL0_PC1_WILD_FRAC_SHIFT;
	wbint = (tmp & PMU0_PLL0_PC1_WILD_INT_MASK) >> PMU0_PLL0_PC1_WILD_INT_SHIFT;

	W_REG(osh, &cc->pllcontrol_addr, PMU0_PLL0_PLLCTL2);
	tmp = R_REG(osh, &cc->pllcontrol_data);
	wbint += (tmp & PMU0_PLL0_PC2_WILD_INT_MASK) << PMU0_PLL0_PC2_WILD_INT_SHIFT;

	freq = si_pmu0_alpclk0(sih, osh, cc) / 1000;

	fvco = (freq * wbint) << 8;
	fvco += (freq * (wbfrac >> 10)) >> 2;
	fvco += (freq * (wbfrac & 0x3ff)) >> 10;
	fvco >>= 8;
	fvco >>= pdiv;
	fvco /= 1000;
	fvco *= 1000;

	PMU_MSG(("si_pmu0_cpuclk0: wbint %u wbfrac %u fvco %u\n",
	         wbint, wbfrac, fvco));

	FVCO = fvco;
#endif	/* BCMDBG */

	/* Return ARM/SB clock */
	return FVCO / (divarm + PMU0_PLL0_PC0_DIV_ARM_BASE) * 1000;
}

/* XXX PMU corerev 1 pll programming for BCM4325 BCM4329 BCM4315 */
/* setup pll and query clock speed */
typedef struct {
	uint16	fref;
	uint8	xf;
	uint8	p1div;
	uint8	p2div;
	uint8	ndiv_int;
	uint32	ndiv_frac;
} pmu1_xtaltab0_t;

/* the following table is based on 880Mhz fvco */
static const pmu1_xtaltab0_t BCMINITDATA(pmu1_xtaltab0_880)[] = {
	{12000,	1,	3,	22,	0x9,	0xFFFFEF},
	{13000,	2,	1,	6,	0xb,	0x483483},
	{14400,	3,	1,	10,	0xa,	0x1C71C7},
	{15360,	4,	1,	5,	0xb,	0x755555},
	{16200,	5,	1,	10,	0x5,	0x6E9E06},
	{16800,	6,	1,	10,	0x5,	0x3Cf3Cf},
	{19200,	7,	1,	9,	0x5,	0x17B425},
	{19800,	8,	1,	11,	0x4,	0xA57EB},
	{20000,	9,	1,	11,	0x4,	0x0},
	{24000,	10,	3,	11,	0xa,	0x0},
	{25000,	11,	5,	16,	0xb,	0x0},
	{26000,	12,	1,	2,	0x10,	0xEC4EC4},
	{30000,	13,	3,	8,	0xb,	0x0},
	{38400,	14,	1,	2,	0xb,	0x755555},
	{40000,	15,	1,	2,	0xb,	0},
	{0,	0,	0,	0,	0,	0}
};

#define PMU1_XTALTAB0_880_12000K	0
#define PMU1_XTALTAB0_880_13000K	1
#define PMU1_XTALTAB0_880_14400K	2
#define PMU1_XTALTAB0_880_15360K	3
#define PMU1_XTALTAB0_880_16200K	4
#define PMU1_XTALTAB0_880_16800K	5
#define PMU1_XTALTAB0_880_19200K	6
#define PMU1_XTALTAB0_880_19800K	7
#define PMU1_XTALTAB0_880_20000K	8
#define PMU1_XTALTAB0_880_24000K	9
#define PMU1_XTALTAB0_880_25000K	10
#define PMU1_XTALTAB0_880_26000K	11
#define PMU1_XTALTAB0_880_30000K	12
#define PMU1_XTALTAB0_880_38400K	13
#define PMU1_XTALTAB0_880_40000K	14

/* the following table is based on 1760Mhz fvco */
static const pmu1_xtaltab0_t BCMINITDATA(pmu1_xtaltab0_1760)[] = {
	{12000,	1,	3,	44,	0x9,	0xFFFFEF},
	{13000,	2,	1,	12,	0xb,	0x483483},
	{14400,	3,	1,	20,	0xa,	0x1C71C7},
	{15360,	4,	1,	10,	0xb,	0x755555},
	{16200,	5,	1,	20,	0x5,	0x6E9E06},
	{16800,	6,	1,	20,	0x5,	0x3Cf3Cf},
	{19200,	7,	1,	18,	0x5,	0x17B425},
	{19800,	8,	1,	22,	0x4,	0xA57EB},
	{20000,	9,	1,	22,	0x4,	0x0},
	{24000,	10,	3,	22,	0xa,	0x0},
	{25000,	11,	5,	32,	0xb,	0x0},
	{26000,	12,	1,	4,	0x10,	0xEC4EC4},
	{30000,	13,	3,	16,	0xb,	0x0},
	{38400,	14,	1,	10,	0x4,	0x955555},
	{40000,	15,	1,	4,	0xb,	0},
	{0,	0,	0,	0,	0,	0}
};

/* table index */
#define PMU1_XTALTAB0_1760_12000K	0
#define PMU1_XTALTAB0_1760_13000K	1
#define PMU1_XTALTAB0_1760_14400K	2
#define PMU1_XTALTAB0_1760_15360K	3
#define PMU1_XTALTAB0_1760_16200K	4
#define PMU1_XTALTAB0_1760_16800K	5
#define PMU1_XTALTAB0_1760_19200K	6
#define PMU1_XTALTAB0_1760_19800K	7
#define PMU1_XTALTAB0_1760_20000K	8
#define PMU1_XTALTAB0_1760_24000K	9
#define PMU1_XTALTAB0_1760_25000K	10
#define PMU1_XTALTAB0_1760_26000K	11
#define PMU1_XTALTAB0_1760_30000K	12
#define PMU1_XTALTAB0_1760_38400K	13
#define PMU1_XTALTAB0_1760_40000K	14

/* the following table is based on 1440Mhz fvco */
static const pmu1_xtaltab0_t BCMINITDATA(pmu1_xtaltab0_1440)[] = {
	{12000,	1,	3,	44,	0x9,	0xFFFFEF},
	{13000,	2,	1,	12,	0xb,	0x483483},
	{14400,	3,	1,	20,	0xa,	0x1C71C7},
	{15360,	4,	1,	10,	0xb,	0x755555},
	{16200,	5,	1,	20,	0x5,	0x6E9E06},
	{16800,	6,	1,	20,	0x5,	0x3Cf3Cf},
	{19200,	7,	1,	18,	0x5,	0x17B425},
	{19800,	8,	1,	22,	0x4,	0xA57EB},
	{20000,	9,	1,	22,	0x4,	0x0},
	{24000,	10,	3,	22,	0xa,	0x0},
	{25000,	11,	5,	32,	0xb,	0x0},
	{26000,	12,	1,	4,	0x10,	0xEC4EC4},
	{30000,	13,	1,	2,	0x18,	0x0},
	{38400,	14,	1,	10,	0x4,	0x955555},
	{40000,	15,	1,	4,	0xb,	0},
	{0,	0,	0,	0,	0,	0}
};

/* table index */
#define PMU1_XTALTAB0_1440_12000K	0
#define PMU1_XTALTAB0_1440_13000K	1
#define PMU1_XTALTAB0_1440_14400K	2
#define PMU1_XTALTAB0_1440_15360K	3
#define PMU1_XTALTAB0_1440_16200K	4
#define PMU1_XTALTAB0_1440_16800K	5
#define PMU1_XTALTAB0_1440_19200K	6
#define PMU1_XTALTAB0_1440_19800K	7
#define PMU1_XTALTAB0_1440_20000K	8
#define PMU1_XTALTAB0_1440_24000K	9
#define PMU1_XTALTAB0_1440_25000K	10
#define PMU1_XTALTAB0_1440_26000K	11
#define PMU1_XTALTAB0_1440_30000K	12
#define PMU1_XTALTAB0_1440_38400K	13
#define PMU1_XTALTAB0_1440_40000K	14

/* select xtal table for each chip */
static const pmu1_xtaltab0_t *
BCMINITFN(si_pmu1_xtaltab0)(si_t *sih)
{
	switch (CHIPID(sih->chip)) {
	case BCM4325_CHIP_ID:
		return pmu1_xtaltab0_880;
#ifdef UNRELEASEDCHIP
	case BCM4329_CHIP_ID:
		return pmu1_xtaltab0_880;
	case BCM4315_CHIP_ID:
		return pmu1_xtaltab0_1760;
	case BCM4319_CHIP_ID:
		return pmu1_xtaltab0_1440;
#endif	/* UNRELEASEDCHIP */
	default:
		PMU_MSG(("si_pmu1_xtaltab0: Unknown chipid %04X\n", sih->chip));
		break;
	}
	ASSERT(0);
	return NULL;
}

/* select default xtal frequency for each chip */
static const pmu1_xtaltab0_t *
BCMINITFN(si_pmu1_xtaldef0)(si_t *sih)
{
	switch (CHIPID(sih->chip)) {
	case BCM4325_CHIP_ID:
		/* Default to 26000Khz */
		return &pmu1_xtaltab0_880[PMU1_XTALTAB0_880_26000K];
#ifdef UNRELEASEDCHIP
	case BCM4329_CHIP_ID:
		/* Default to 38400Khz */
		return &pmu1_xtaltab0_880[PMU1_XTALTAB0_1760_38400K];
	case BCM4315_CHIP_ID:
#ifdef BCMUSBDEV
		/* Default to 30000Khz */
		return &pmu1_xtaltab0_1760[PMU1_XTALTAB0_1760_30000K];
#else
		/* Default to 26000Khz */
		return &pmu1_xtaltab0_1760[PMU1_XTALTAB0_1760_26000K];
#endif
	case BCM4319_CHIP_ID:
		/* Default to 30000Khz */
		return &pmu1_xtaltab0_1440[PMU1_XTALTAB0_1440_30000K];
#endif	/* UNRELEASEDCHIP */
	default:
		PMU_MSG(("si_pmu1_xtaldef0: Unknown chipid %04X\n", sih->chip));
		break;
	}
	ASSERT(0);
	return NULL;
}

/* select default pll fvco for each chip */
static uint32
BCMINITFN(si_pmu1_pllfvco0)(si_t *sih)
{
	switch (CHIPID(sih->chip)) {
	case BCM4325_CHIP_ID:
		return FVCO_880;
#ifdef UNRELEASEDCHIP
	case BCM4329_CHIP_ID:
		return FVCO_880;
	case BCM4315_CHIP_ID:
		return FVCO_1760;
	case BCM4319_CHIP_ID:
		return FVCO_1440;
#endif	/* UNRELEASEDCHIP */
	default:
		PMU_MSG(("si_pmu1_pllfvco0: Unknown chipid %04X\n", sih->chip));
		break;
	}
	ASSERT(0);
	return 0;
}

/* query alp/xtal clock frequency */
static uint32
BCMINITFN(si_pmu1_alpclk0)(si_t *sih, osl_t *osh, chipcregs_t *cc)
{
	const pmu1_xtaltab0_t *xt;
	uint32 xf;

	/* Find the frequency in the table */
	xf = (R_REG(osh, &cc->pmucontrol) & PCTL_XTALFREQ_MASK) >>
	        PCTL_XTALFREQ_SHIFT;
	for (xt = si_pmu1_xtaltab0(sih); xt != NULL && xt->fref != 0; xt ++)
		if (xt->xf == xf)
			break;
	/* Could not find it so assign a default value */
	if (xt == NULL || xt->fref == 0)
		xt = si_pmu1_xtaldef0(sih);
	ASSERT(xt != NULL && xt->fref != 0);

	return xt->fref * 1000;
}

/* Set up PLL registers in the PMU as per the crystal speed.
 * XtalFreq field in pmucontrol register being 0 indicates the PLL
 * is not programmed and the h/w default is assumed to work, in which
 * case the xtal frequency is unknown to the s/w so we need to call
 * si_pmu1_xtaldef0() wherever it is needed to return a default value.
 */
static void
BCMINITFN(si_pmu1_pllinit0)(si_t *sih, osl_t *osh, chipcregs_t *cc, uint32 xtal)
{
	const pmu1_xtaltab0_t *xt;
	uint32 tmp;
	uint32 buf_strength = 0;

	/* Use h/w default PLL config */
	if (xtal == 0) {
		PMU_MSG(("Unspecified xtal frequency, skip PLL configuration\n"));
		return;
	}

	/* Find the frequency in the table */
	for (xt = si_pmu1_xtaltab0(sih); xt != NULL && xt->fref != 0; xt ++)
		if (xt->fref == xtal)
			break;

	/* Check current PLL state, bail out if it has been programmed or
	 * we don't know how to program it.
	 */
	if (xt == NULL || xt->fref == 0) {
		PMU_MSG(("Unsupported xtal frequency %d.%d MHz, skip PLL configuration\n",
		         xtal / 1000, xtal % 1000));
		return;
	}
	if (((R_REG(osh, &cc->pmucontrol) & PCTL_XTALFREQ_MASK) >>
	     PCTL_XTALFREQ_SHIFT) == xt->xf) {
		PMU_MSG(("PLL already programmed for %d.%d MHz\n",
			xt->fref / 1000, xt->fref % 1000));
		return;
	}

	PMU_MSG(("XTAL %d.%d MHz (%d)\n", xtal / 1000, xtal % 1000, xt->xf));
	PMU_MSG(("Programming PLL for %d.%d MHz\n", xt->fref / 1000, xt->fref % 1000));

	switch (CHIPID(sih->chip)) {
	case BCM4325_CHIP_ID:
		/* Change the BBPLL drive strength to 2 for all channels */
		buf_strength = 0x222222;
		if (sih->pmurev >= 2)
			break;
		/* Make sure the PLL is off */
		AND_REG(osh, &cc->min_res_mask,
		        ~(PMURES_BIT(RES4325_BBPLL_PWRSW_PU) | PMURES_BIT(RES4325_HT_AVAIL)));
		AND_REG(osh, &cc->max_res_mask,
		        ~(PMURES_BIT(RES4325_BBPLL_PWRSW_PU) | PMURES_BIT(RES4325_HT_AVAIL)));
		SPINWAIT(R_REG(osh, &cc->clk_ctl_st) & CCS_HTAVAIL, PMU_MAX_TRANSITION_DLY);
		ASSERT(!(R_REG(osh, &cc->clk_ctl_st) & CCS_HTAVAIL));
		break;
#ifdef UNRELEASEDCHIP
	case BCM4329_CHIP_ID:
		/* Change the BBPLL drive strength to 8 for all channels */
		buf_strength = 0x888888;
		AND_REG(osh, &cc->min_res_mask,
		        ~(PMURES_BIT(RES4329_BBPLL_PWRSW_PU) | PMURES_BIT(RES4329_HT_AVAIL)));
		AND_REG(osh, &cc->max_res_mask,
		        ~(PMURES_BIT(RES4329_BBPLL_PWRSW_PU) | PMURES_BIT(RES4329_HT_AVAIL)));
		break;
	case BCM4315_CHIP_ID:
		/* Change the BBPLL drive strength to 2 for all channels */
		buf_strength = 0x222222;
		break;
	case BCM4319_CHIP_ID:
		/* Change the BBPLL drive strength to 2 for all channels */
		buf_strength = 0x222222;
		break;
#endif /* UNRELEASEDCHIP */
	default:
		ASSERT(0);
	}

	PMU_MSG(("Done masking\n"));

	/* Write p1div and p2div to pllcontrol[0] */
	W_REG(osh, &cc->pllcontrol_addr, PMU1_PLL0_PLLCTL0);
	tmp = R_REG(osh, &cc->pllcontrol_data) &
	        ~(PMU1_PLL0_PC0_P1DIV_MASK | PMU1_PLL0_PC0_P2DIV_MASK);
	tmp |= ((xt->p1div << PMU1_PLL0_PC0_P1DIV_SHIFT) & PMU1_PLL0_PC0_P1DIV_MASK) |
	        ((xt->p2div << PMU1_PLL0_PC0_P2DIV_SHIFT) & PMU1_PLL0_PC0_P2DIV_MASK);
	W_REG(osh, &cc->pllcontrol_data, tmp);

#ifdef	UNRELEASEDCHIP
	if ((CHIPID(sih->chip) == BCM4329_CHIP_ID)) {
		/* This is to reduce the MAC clock frequency. 
		 * MAC clk Frequency = 880/DOT11MAC_880MHZ_CLK_DIVISOR_VAL
		 * This is a part of the workaround for PR54092.
		 * Currently the MAC clock to OCP clock ratio must be in the range of .6 to 1.66.
		 * So for PR54092 the MAC clock is kept at 62.5 MHz for ratio of 1.64.
		 * Higher MAC clock frequency wouold take the ratio higher
		 * and lower MAC clk fequency would cause high uCode latencies
		 */

		W_REG(osh, &cc->pllcontrol_addr, PMU1_PLL0_PLLCTL1);
		tmp = R_REG(osh, &cc->pllcontrol_data);
		tmp = tmp & (~DOT11MAC_880MHZ_CLK_DIVISOR_MASK);
		tmp = tmp | DOT11MAC_880MHZ_CLK_DIVISOR_VAL;
		W_REG(osh, &cc->pllcontrol_data, tmp);
	}
#endif /* UNRELEASEDCHIP */

	/* Write ndiv_int and ndiv_mode to pllcontrol[2] */
	W_REG(osh, &cc->pllcontrol_addr, PMU1_PLL0_PLLCTL2);
	tmp = R_REG(osh, &cc->pllcontrol_data) &
	        ~(PMU1_PLL0_PC2_NDIV_INT_MASK | PMU1_PLL0_PC2_NDIV_MODE_MASK);
	tmp |= ((xt->ndiv_int << PMU1_PLL0_PC2_NDIV_INT_SHIFT) & PMU1_PLL0_PC2_NDIV_INT_MASK) |
	        ((1 << PMU1_PLL0_PC2_NDIV_MODE_SHIFT) & PMU1_PLL0_PC2_NDIV_MODE_MASK);
	W_REG(osh, &cc->pllcontrol_data, tmp);

	/* Write ndiv_frac to pllcontrol[3] */
	W_REG(osh, &cc->pllcontrol_addr, PMU1_PLL0_PLLCTL3);
	tmp = R_REG(osh, &cc->pllcontrol_data) & ~PMU1_PLL0_PC3_NDIV_FRAC_MASK;
	tmp |= ((xt->ndiv_frac << PMU1_PLL0_PC3_NDIV_FRAC_SHIFT) &
	        PMU1_PLL0_PC3_NDIV_FRAC_MASK);
	W_REG(osh, &cc->pllcontrol_data, tmp);

	/* Write clock driving strength to pllcontrol[5] */
	if (buf_strength) {
		PMU_MSG(("Adjusting PLL buffer drive strength: %x\n", buf_strength));

		W_REG(osh, &cc->pllcontrol_addr, PMU1_PLL0_PLLCTL5);
		tmp = R_REG(osh, &cc->pllcontrol_data) & ~PMU1_PLL0_PC5_CLK_DRV_MASK;
		tmp |= (buf_strength << PMU1_PLL0_PC5_CLK_DRV_SHIFT);
		W_REG(osh, &cc->pllcontrol_data, tmp);
	}

	PMU_MSG(("Done pll\n"));

	/* Flush deferred pll control registers writes */
	if (sih->pmurev >= 2)
		OR_REG(osh, &cc->pmucontrol, PCTL_PLL_PLLCTL_UPD);

	/* Write XtalFreq. Set the divisor also. */
	tmp = R_REG(osh, &cc->pmucontrol) &
	        ~(PCTL_ILP_DIV_MASK | PCTL_XTALFREQ_MASK);
	tmp |= (((((xt->fref + 127) / 128) - 1) << PCTL_ILP_DIV_SHIFT) &
	        PCTL_ILP_DIV_MASK) |
	       ((xt->xf << PCTL_XTALFREQ_SHIFT) & PCTL_XTALFREQ_MASK);

#ifdef UNRELEASEDCHIP
	/* PR54092 : 4329 dual backplane serror WAR. Don't give HT even if asked for */
	if ((CHIPID(sih->chip) == BCM4329_CHIP_ID) && sih->chiprev == 0)
		tmp &= ~PCTL_HT_REQ_EN;
#endif /* UNRELEASEDCHIP */

	W_REG(osh, &cc->pmucontrol, tmp);
}

/* query the CPU clock frequency */
static uint32
BCMINITFN(si_pmu1_cpuclk0)(si_t *sih, osl_t *osh, chipcregs_t *cc)
{
	uint32 tmp, m1div;
#ifdef BCMDBG
	uint32 ndiv_int, ndiv_frac, p2div, p1div, fvco;
	uint32 fref;
#endif
	uint32 FVCO = si_pmu1_pllfvco0(sih);

	/* Read m1div from pllcontrol[1] */
	W_REG(osh, &cc->pllcontrol_addr, PMU1_PLL0_PLLCTL1);
	tmp = R_REG(osh, &cc->pllcontrol_data);
	m1div = (tmp & PMU1_PLL0_PC1_M1DIV_MASK) >> PMU1_PLL0_PC1_M1DIV_SHIFT;

#ifdef BCMDBG
	/* Read p2div/p1div from pllcontrol[0] */
	W_REG(osh, &cc->pllcontrol_addr, PMU1_PLL0_PLLCTL0);
	tmp = R_REG(osh, &cc->pllcontrol_data);
	p2div = (tmp & PMU1_PLL0_PC0_P2DIV_MASK) >> PMU1_PLL0_PC0_P2DIV_SHIFT;
	p1div = (tmp & PMU1_PLL0_PC0_P1DIV_MASK) >> PMU1_PLL0_PC0_P1DIV_SHIFT;

	/* Calculate fvco based on xtal freq and ndiv and pdiv */
	W_REG(osh, &cc->pllcontrol_addr, PMU1_PLL0_PLLCTL2);
	tmp = R_REG(osh, &cc->pllcontrol_data);
	ndiv_int = (tmp & PMU1_PLL0_PC2_NDIV_INT_MASK) >> PMU1_PLL0_PC2_NDIV_INT_SHIFT;

	W_REG(osh, &cc->pllcontrol_addr, PMU1_PLL0_PLLCTL3);
	tmp = R_REG(osh, &cc->pllcontrol_data);
	ndiv_frac = (tmp & PMU1_PLL0_PC3_NDIV_FRAC_MASK) >> PMU1_PLL0_PC3_NDIV_FRAC_SHIFT;

	fref = si_pmu1_alpclk0(sih, osh, cc) / 1000;

	fvco = (fref * ndiv_int) << 8;
	fvco += (fref * (ndiv_frac >> 12)) >> 4;
	fvco += (fref * (ndiv_frac & 0xfff)) >> 12;
	fvco >>= 8;
	fvco *= p2div;
	fvco /= p1div;
	fvco /= 1000;
	fvco *= 1000;

	PMU_MSG(("si_pmu1_cpuclk0: ndiv_int %u ndiv_frac %u p2div %u p1div %u fvco %u\n",
	         ndiv_int, ndiv_frac, p2div, p1div, fvco));

	FVCO = fvco;
#endif	/* BCMDBG */

	/* Return ARM/SB clock */
	return FVCO / m1div * 1000;
}

/* initialize PLL */
void
BCMINITFN(si_pmu_pll_init)(si_t *sih, osl_t *osh, uint xtalfreq)
{
	chipcregs_t *cc;
	uint origidx;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	switch (CHIPID(sih->chip)) {
	case BCM4328_CHIP_ID:
		si_pmu0_pllinit0(sih, osh, cc, xtalfreq);
		break;
	case BCM5354_CHIP_ID:
		if (xtalfreq == 0)
			xtalfreq = 25000;
		si_pmu0_pllinit0(sih, osh, cc, xtalfreq);
		break;
	case BCM4325_CHIP_ID:
		si_pmu1_pllinit0(sih, osh, cc, xtalfreq);
		break;
#ifdef UNRELEASEDCHIP
	case BCM4329_CHIP_ID:
		si_pmu1_pllinit0(sih, osh, cc, xtalfreq);
		break;
	case BCM4315_CHIP_ID:
		si_pmu1_pllinit0(sih, osh, cc, xtalfreq);
		break;
	case BCM4319_CHIP_ID:
		si_pmu1_pllinit0(sih, osh, cc, xtalfreq);
		break;
#endif	/* UNRELEASEDCHIP */
	case BCM4312_CHIP_ID:
		/* assume default works */
		break;
	case BCM4322_CHIP_ID:
		/* PR 52182 WAR : set 4322A0/A1 PMU SB PLL control register 10 to 0x380005c0
		 * for better jitter performance
		 */
		if (sih->pmurev == 2) {
			W_REG(osh, &cc->pllcontrol_addr, PMU2_SI_PLL_PLLCTL);
			W_REG(osh, &cc->pllcontrol_data, 0x380005c0);
		}
		break;

	default:
		PMU_MSG(("No PLL init done for chip %x rev %d pmurev %d\n",
		         sih->chip, sih->chiprev, sih->pmurev));
		break;
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

/* query alp/xtal clock frequency */
uint32
BCMINITFN(si_pmu_alp_clock)(si_t *sih, osl_t *osh)
{
	chipcregs_t *cc;
	uint origidx;
	uint32 clock = ALP_CLOCK;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	switch (CHIPID(sih->chip)) {
	case BCM4328_CHIP_ID:
		clock = si_pmu0_alpclk0(sih, osh, cc);
		break;
	case BCM5354_CHIP_ID:
		clock = si_pmu0_alpclk0(sih, osh, cc);
		break;
	case BCM4325_CHIP_ID:
		clock = si_pmu1_alpclk0(sih, osh, cc);
		break;
#ifdef UNRELEASEDCHIP
	case BCM4329_CHIP_ID:
		clock = si_pmu1_alpclk0(sih, osh, cc);
		break;
	case BCM4315_CHIP_ID:
		clock = si_pmu1_alpclk0(sih, osh, cc);
		break;
	case BCM4319_CHIP_ID:
		clock = si_pmu1_alpclk0(sih, osh, cc);
		break;
#endif	/* UNRELEASEDCHIP */
	case BCM4312_CHIP_ID:
	case BCM4322_CHIP_ID:
	case BCM4716_CHIP_ID:
		/* always 20Mhz */
		clock = 20000 * 1000;
		break;

	default:
		PMU_MSG(("No ALP clock specified "
			"for chip %x rev %d pmurev %d, using default %d Hz\n",
			sih->chip, sih->chiprev, sih->pmurev, clock));
		break;
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);
	return clock;
}

/* Find the output of the "m" pll divider given pll controls that start with
 * pllreg "pll0" i.e. 12 for main 6 for phy, 0 for misc.
 */
static uint32
BCMINITFN(si_pmu5_clock)(si_t *sih, osl_t *osh, chipcregs_t *cc, uint pll0, uint m)
{
	uint32 tmp, div, ndiv, p1, p2, fc;

	if ((pll0 & 3) || (pll0 > PMU5_MAINPLL_PLL0)) {
		PMU_ERROR(("%s: Bad pll0: %d\n", __FUNCTION__, pll0));
		return 0;
	}

	/* Strictly there is an m5 divider, but I'm not sure we use it */
	if ((m == 0) || (m > 4)) {
		PMU_ERROR(("%s: Bad m divider: %d\n", __FUNCTION__, m));
		return 0;
	}

	W_REG(osh, &cc->pllcontrol_addr, pll0 + PMU5_PLL_P1P2_OFF);
	(void)R_REG(osh, &cc->pllcontrol_addr);
	tmp = R_REG(osh, &cc->pllcontrol_data);
	p1 = (tmp & PMU5_PLL_P1_MASK) >> PMU5_PLL_P1_SHIFT;
	p2 = (tmp & PMU5_PLL_P2_MASK) >> PMU5_PLL_P2_SHIFT;

	W_REG(osh, &cc->pllcontrol_addr, pll0 + PMU5_PLL_M14_OFF);
	(void)R_REG(osh, &cc->pllcontrol_addr);
	tmp = R_REG(osh, &cc->pllcontrol_data);
	div = (tmp >> ((m - 1) * PMU5_PLL_MDIV_WIDTH)) & PMU5_PLL_MDIV_MASK;

	W_REG(osh, &cc->pllcontrol_addr, pll0 + PMU5_PLL_NM5_OFF);
	(void)R_REG(osh, &cc->pllcontrol_addr);
	tmp = R_REG(osh, &cc->pllcontrol_data);
	ndiv = (tmp & PMU5_PLL_NDIV_MASK) >> PMU5_PLL_NDIV_SHIFT;

	/* Do calculation in Mhz */
	fc = si_pmu_alp_clock(sih, osh) / 1000000;
	fc = (p1 * ndiv * fc) / p2;

	PMU_MSG(("%s: p1=%d, p2=%d, ndiv=%d(0x%x), m%d=%d; fc=%d, clock=%d\n",
	         __FUNCTION__, p1, p2, ndiv, ndiv, m, div, fc, fc / div));

	/* Return clock in Hertz */
	return ((fc / div) * 1000000);
}

/* query backplane clock frequency */
/* For designs that feed the same clock to both backplane
 * and CPU just return the CPU clock speed.
 */
uint32
BCMINITFN(si_pmu_si_clock)(si_t *sih, osl_t *osh)
{
	chipcregs_t *cc;
	uint origidx;
	uint32 clock = HT_CLOCK;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	switch (CHIPID(sih->chip)) {

    case BCM53000_CHIP_ID:
        clock = si_pmu_cpu_clock(sih, osh) / 4;
        break;
	case BCM4716_CHIP_ID:
		clock = si_pmu5_clock(sih, osh, cc, PMU5_MAINPLL_PLL0, PMU5_MAINPLL_SI);
		break;
	case BCM4328_CHIP_ID:
		clock = si_pmu0_cpuclk0(sih, osh, cc);
		break;
	case BCM5354_CHIP_ID:
		clock = 120000000;
		break;
	case BCM4325_CHIP_ID:
		clock = si_pmu1_cpuclk0(sih, osh, cc);
		break;
#ifdef UNRELEASEDCHIP
	case BCM4329_CHIP_ID:
		if (sih->chiprev == 0)
			clock = 38400 * 1000;
		else
			clock = si_pmu1_cpuclk0(sih, osh, cc);
		break;
	case BCM4315_CHIP_ID:
		clock = si_pmu1_cpuclk0(sih, osh, cc);
		break;
	case BCM4319_CHIP_ID:
		clock = si_pmu1_cpuclk0(sih, osh, cc);
		break;
#endif	/* UNRELEASEDCHIP */
	case BCM4322_CHIP_ID:
		/* 96MHz backplane clock */
		clock = 96000 * 1000;
		break;

	default:
		PMU_MSG(("No backplane clock specified "
			"for chip %x rev %d pmurev %d, using default %d Hz\n",
			sih->chip, sih->chiprev, sih->pmurev, clock));
		break;
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);
	return clock;
}

/* query CPU clock frequency */
uint32
BCMINITFN(si_pmu_cpu_clock)(si_t *sih, osl_t *osh)
{
	chipcregs_t *cc;
	uint origidx;
	uint32 clock;

	ASSERT(sih->cccaps & CC_CAP_PMU);

    /* Programmable CPU frequency (by strap pin) of 5300x chip */
    if (sih->chip == BCM53000_CHIP_ID) {
        uint32 adj;

#ifdef CFG_QUICKTURN	
        return 200000000;
#endif /* CFG_QUICKTURN */

        /* Remember original core before switch to chipc */
        origidx = si_coreidx(sih);
        cc = si_setcoreidx(sih, SI_CC_IDX);
        ASSERT(cc != NULL);
        
        /* check if it's a low cost bonding */
        if (R_REG(osh, &cc->chipstatus) & CHIPSTATUS_LOWCOST_BONDING) {
            clock = 400000000;
        } else {
            /* Get N divider */
            W_REG(osh, &cc->pllcontrol_addr, 4);
            adj = (R_REG(osh, &cc->pllcontrol_data) & 0x00000fff) >> 3;

            /* Fixed reference clock 25MHz and m = 2 */
            clock = adj * 25000000 / 2; 
        }
        
        /* Return to original core */
        si_setcoreidx(sih, origidx);
        
        return clock;
    }

	/* 5354 chip uses a non programmable PLL of frequency 240MHz */
	if (sih->chip == BCM5354_CHIP_ID)
		return 240000000;

	if (sih->pmurev == 5) {
		/* Remember original core before switch to chipc */
		origidx = si_coreidx(sih);
		cc = si_setcoreidx(sih, SI_CC_IDX);
		ASSERT(cc != NULL);

		clock = si_pmu5_clock(sih, osh, cc, PMU5_MAINPLL_PLL0, PMU5_MAINPLL_CPU);

		/* Return to original core */
		si_setcoreidx(sih, origidx);
	} else
		clock = si_pmu_si_clock(sih, osh);

	return clock;
}

/* query Memroy clock frequency */
uint32
BCMINITFN(si_pmu_mem_clock)(si_t *sih, osl_t *osh)
{
	chipcregs_t *cc;
	uint origidx;
	uint32 clock;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	if (sih->pmurev == 5) {
		/* Remember original core before switch to chipc */
		origidx = si_coreidx(sih);
		cc = si_setcoreidx(sih, SI_CC_IDX);
		ASSERT(cc != NULL);

		clock = si_pmu5_clock(sih, osh, cc, PMU5_MAINPLL_PLL0, PMU5_MAINPLL_MEM);

		/* Return to original core */
		si_setcoreidx(sih, origidx);
	} else
		clock = si_pmu_si_clock(sih, osh);

	return clock;
}

/* query ILP clock frequency */
/* XXX The variable ilpcycles_per_sec is used to store the ILP clock speed. The value
 * is calculated when the function is called the first time using PMU.timer ellapsed
 * during the time defined by ILP_CALC_DUR and then cached. Before first time calling
 * the function one must make sure the HT clock is turned on and used to feed the CPU
 * and OSL_DELAY() is calibrated and available.
 */
#define ILP_CALC_DUR	10	/* ms, make sure 1000 can be divided by it. */
uint32
BCMINITFN(si_pmu_ilp_clock)(si_t *sih, osl_t *osh)
{
	static uint32 ilpcycles_per_sec = 0;

	if (ISSIM_ENAB(sih))
		return ILP_CLOCK;

	if (ilpcycles_per_sec == 0) {
		uint32 start, end, delta;
		uint32 origidx = si_coreidx(sih);
		chipcregs_t *cc = si_setcoreidx(sih, SI_CC_IDX);
		ASSERT(cc != NULL);
		start = R_REG(osh, &cc->pmutimer);
		OSL_DELAY(ILP_CALC_DUR * 1000);
		end = R_REG(osh, &cc->pmutimer);
		delta = end >= start ? end - start : ~0 - start + 1 + end;
		ilpcycles_per_sec = delta * (1000 / ILP_CALC_DUR);
		si_setcoreidx(sih, origidx);
	}

	return ilpcycles_per_sec;
}
#endif /* !BCMDONGLEHOST */

/* SDIO Pad drive strength to select value mappings */
typedef struct {
	uint8 strength;			/* Pad Drive Strength in mA */
	uint8 sel;			/* Chip-specific select value */
} sdiod_drive_str_t;

/* SDIO Drive Strength to sel value table for PMU Rev 1 */
static const sdiod_drive_str_t BCMINITDATA(sdiod_drive_strength_tab1)[] = {
	{4, 0x2},
	{2, 0x3},
	{1, 0x0},
	{0, 0x0} };

/* SDIO Drive Strength to sel value table for PMU Rev 2, 3 */
static const sdiod_drive_str_t BCMINITDATA(sdiod_drive_strength_tab2)[] = {
	{12, 0x7},
	{10, 0x6},
	{8, 0x5},
	{6, 0x4},
	{4, 0x2},
	{2, 0x1},
	{0, 0x0} };

#define SDIOD_DRVSTR_KEY(chip, pmu)	(((chip) << 16) | (pmu))

void
BCMINITFN(si_sdiod_drive_strength_init)(si_t *sih, osl_t *osh, uint drivestrength)
{
	chipcregs_t *cc;
	uint origidx, intr_val;
	sdiod_drive_str_t *str_tab = NULL;
	uint32 str_mask = 0;
	uint32 str_shift = 0;

	if (!(sih->cccaps & CC_CAP_PMU)) {
		return;
	}

	/* Remember original core before switch to chipc */
	cc = (chipcregs_t *)si_switch_core(sih, CC_CORE_ID, &origidx, &intr_val);

	switch (SDIOD_DRVSTR_KEY(sih->chip, sih->pmurev)) {
	case SDIOD_DRVSTR_KEY(BCM4325_CHIP_ID, 1):
		str_tab = (sdiod_drive_str_t *)&sdiod_drive_strength_tab1;
		str_mask = 0x30000000;
		str_shift = 28;
		break;
	case SDIOD_DRVSTR_KEY(BCM4325_CHIP_ID, 2):
	case SDIOD_DRVSTR_KEY(BCM4325_CHIP_ID, 3):
#ifdef UNRELEASEDCHIP
	case SDIOD_DRVSTR_KEY(BCM4315_CHIP_ID, 4):
#endif
		str_tab = (sdiod_drive_str_t *)&sdiod_drive_strength_tab2;
		str_mask = 0x00003800;
		str_shift = 11;
		break;

	default:
		PMU_MSG(("No SDIO Drive strength init done for chip %x rev %d pmurev %d\n",
		         sih->chip, sih->chiprev, sih->pmurev));

		break;
	}

	if (str_tab != NULL) {
		uint32 drivestrength_sel = 0;
		uint32 cc_data_temp;
		int i;

		for (i = 0; str_tab[i].strength != 0; i ++) {
			if (drivestrength >= str_tab[i].strength) {
				drivestrength_sel = str_tab[i].sel;
				break;
			}
		}

		W_REG(osh, &cc->chipcontrol_addr, 1);
		cc_data_temp = R_REG(osh, &cc->chipcontrol_data);
		cc_data_temp &= ~str_mask;
		drivestrength_sel <<= str_shift;
		cc_data_temp |= drivestrength_sel;
		W_REG(osh, &cc->chipcontrol_data, cc_data_temp);

		PMU_MSG(("SDIO: %dmA drive strength selected, set to 0x%08x\n",
		         drivestrength, cc_data_temp));
	}

	/* Return to original core */
	si_restore_core(sih, origidx, intr_val);
}

#ifndef	BCMDONGLEHOST
/* initialize PMU */
void
BCMINITFN(si_pmu_init)(si_t *sih, osl_t *osh)
{
	chipcregs_t *cc;
	uint origidx;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	/* XXX Feature is added in PMU rev. 1 but doesn't work until rev. 2 */
	if (sih->pmurev == 1)
		AND_REG(osh, &cc->pmucontrol, ~PCTL_NOILP_ON_WAIT);
	else if (sih->pmurev >= 2)
		OR_REG(osh, &cc->pmucontrol, PCTL_NOILP_ON_WAIT);

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

/* Return up time in ILP cycles for the given resource. */
/* XXX need to check circular dependancies and prevent dead recursion. */
static uint
BCMINITFN(si_pmu_res_uptime)(si_t *sih, osl_t *osh, chipcregs_t *cc, uint8 rsrc)
{
	uint32 deps;
	uint up, i, dup, dmax;
	uint32 min_mask = 0, max_mask = 0;

	/* uptime of resource 'rsrc' */
	W_REG(osh, &cc->res_table_sel, rsrc);
	up = (R_REG(osh, &cc->res_updn_timer) >> 8) & 0xff;

	/* direct dependancies of resource 'rsrc' */
	deps = si_pmu_res_deps(sih, osh, cc, PMURES_BIT(rsrc), FALSE);
	for (i = 0; i <= PMURES_MAX_RESNUM; i ++) {
		if (!(deps & PMURES_BIT(i)))
			continue;
		deps &= ~si_pmu_res_deps(sih, osh, cc, PMURES_BIT(i), TRUE);
	}
	si_pmu_res_masks(sih, &min_mask, &max_mask);
	deps &= ~min_mask;

	/* max uptime of direct dependancies */
	dmax = 0;
	for (i = 0; i <= PMURES_MAX_RESNUM; i ++) {
		if (!(deps & PMURES_BIT(i)))
			continue;
		dup = si_pmu_res_uptime(sih, osh, cc, (uint8)i);
		if (dmax < dup)
			dmax = dup;
	}

	PMU_MSG(("si_pmu_res_uptime: rsrc %u uptime %u(deps 0x%08x uptime %u)\n",
	         rsrc, up, deps, dmax));

	return up + dmax + PMURES_UP_TRANSITION;
}
#endif /* !BCMDONGLEHOST */

/* Return dependancies (direct or all/indirect) for the given resources */
/* XXX need to check circular dependancies and prevent dead recursion */
static uint32
si_pmu_res_deps(si_t *sih, osl_t *osh, chipcregs_t *cc, uint32 rsrcs, bool all)
{
	uint32 deps = 0;
	uint32 i;

	for (i = 0; i <= PMURES_MAX_RESNUM; i ++) {
		if (!(rsrcs & PMURES_BIT(i)))
			continue;
		W_REG(osh, &cc->res_table_sel, i);
		deps |= R_REG(osh, &cc->res_dep_mask);
	}

	return !all ? deps : (deps ? (deps | si_pmu_res_deps(sih, osh, cc, deps, TRUE)) : 0);
}

/* power up/down OTP through PMU resources */
void
si_pmu_otp_power(si_t *sih, osl_t *osh, bool on)
{
	chipcregs_t *cc;
	uint origidx;
	uint32 rsrcs = 0;	/* rsrcs to turn on/off OTP power */

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Don't do anything if OTP is disabled */
	if (si_is_otp_disabled(sih)) {
		PMU_MSG(("si_pmu_otp_power: OTP is disabled\n"));
		return;
	}

	/* Remember original core before switch to chipc */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	switch (CHIPID(sih->chip)) {
	case BCM4325_CHIP_ID:
		rsrcs = PMURES_BIT(RES4325_LNLDO2_PU);
		break;
#ifdef UNRELEASEDCHIP
	case BCM4315_CHIP_ID:
		rsrcs = PMURES_BIT(RES4315_OTP_PU);
		break;
	case BCM4319_CHIP_ID:
		rsrcs = PMURES_BIT(RES4319_OTP_PU);
		break;
#endif	/* UNRELEASEDCHIP */
	default:
		break;
	}

	if (rsrcs != 0) {
		/* Figure out the dependancies (exclude min_res_mask) */
		uint32 deps = si_pmu_res_deps(sih, osh, cc, rsrcs, TRUE);
		uint32 min_mask = 0, max_mask = 0;
		si_pmu_res_masks(sih, &min_mask, &max_mask);
		deps &= ~min_mask;
		/* Turn on/off the power */
		if (on) {
			PMU_MSG(("Adding rsrc 0x%x to min_res_mask\n", rsrcs | deps));
			OR_REG(osh, &cc->min_res_mask, (rsrcs | deps));
			SPINWAIT(!(R_REG(osh, &cc->res_state) & rsrcs), PMU_MAX_TRANSITION_DLY);
			ASSERT(R_REG(osh, &cc->res_state) & rsrcs);
		}
		else {
			PMU_MSG(("Removing rsrc 0x%x from min_res_mask\n", rsrcs | deps));
			AND_REG(osh, &cc->min_res_mask, ~(rsrcs | deps));
		}
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

#ifndef BCMDONGLEHOST
void
si_pmu_rcal(si_t *sih, osl_t *osh)
{
	chipcregs_t *cc;
	uint origidx;
	uint rcal_done, BT_out_of_reset;

	ASSERT(sih->cccaps & CC_CAP_PMU);

	/* Remember original core before switch to chipc */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	switch (CHIPID(sih->chip)) {
	case BCM4325_CHIP_ID: {
		uint8 rcal_code;
		uint32 val;

		/* Kick RCal */
		W_REG(osh, &cc->chipcontrol_addr, 1);

		/* Power Down RCAL Block */
		AND_REG(osh, &cc->chipcontrol_data, ~0x04);

		/* Check if RCAL is already done by BT */
		rcal_done = ((R_REG(osh, &cc->chipstatus)) & 0x8) >> 3;

		/* If RCAL already done, note that BT is out of reset */
		if (rcal_done == 1) {
			BT_out_of_reset = 1;
		} else {
			BT_out_of_reset = 0;
		}

		/* Power Up RCAL block */
		OR_REG(osh, &cc->chipcontrol_data, 0x04);

		/* Wait for completion */
		SPINWAIT(0 == (R_REG(osh, &cc->chipstatus) & 0x08), 10 * 1000 * 1000);
		ASSERT(R_REG(osh, &cc->chipstatus) & 0x08);

		/* RCAL does not work with A0/A1 chips because BT runs RCAL and the 
		 * RCAL code is 0. So RCAL code is hardcoded when BT is out of reset
		 * To be removed in BCM4325B0.
		 * PR 49738 (TCL) and PR 49739 (Driver)
		 */
		if (BT_out_of_reset) {
			rcal_code = 0x6;
		} else {
			/* Drop the LSB to convert from 5 bit code to 4 bit code */
			rcal_code =  (uint8)(R_REG(osh, &cc->chipstatus) >> 5) & 0x0f;
		}

		PMU_MSG(("RCal completed, status 0x%x, code 0x%x\n",
			R_REG(osh, &cc->chipstatus), rcal_code));

		/* Write RCal code into pmu_vreg_ctrl[32:29] */
		W_REG(osh, &cc->regcontrol_addr, 0);
		val = R_REG(osh, &cc->regcontrol_data) & ~((uint32)0x07 << 29);
		val |= (uint32)(rcal_code & 0x07) << 29;
		W_REG(osh, &cc->regcontrol_data, val);
		W_REG(osh, &cc->regcontrol_addr, 1);
		val = R_REG(osh, &cc->regcontrol_data) & ~(uint32)0x01;
		val |= (uint32)((rcal_code >> 3) & 0x01);
		W_REG(osh, &cc->regcontrol_data, val);

		/* Write RCal code into pmu_chip_ctrl[33:30] */
		W_REG(osh, &cc->chipcontrol_addr, 0);
		val = R_REG(osh, &cc->chipcontrol_data) & ~((uint32)0x03 << 30);
		val |= (uint32)(rcal_code & 0x03) << 30;
		W_REG(osh, &cc->chipcontrol_data, val);
		W_REG(osh, &cc->chipcontrol_addr, 1);
		val = R_REG(osh, &cc->chipcontrol_data) & ~(uint32)0x03;
		val |= (uint32)((rcal_code >> 2) & 0x03);
		W_REG(osh, &cc->chipcontrol_data, val);

		/* Set override in pmu_chip_ctrl[29] */
		W_REG(osh, &cc->chipcontrol_addr, 0);
		OR_REG(osh, &cc->chipcontrol_data, (0x01 << 29));

		/* Power off RCal block */
		W_REG(osh, &cc->chipcontrol_addr, 1);
		AND_REG(osh, &cc->chipcontrol_data, ~0x04);

		break;
	}
#ifdef UNRELEASEDCHIP
	case BCM4329_CHIP_ID: {
		uint8 rcal_code;
		uint32 val;

		/* Kick RCal */
		W_REG(osh, &cc->chipcontrol_addr, 1);

		/* Power Down RCAL Block */
		AND_REG(osh, &cc->chipcontrol_data, ~0x04);

		/* Power Up RCAL block */
		OR_REG(osh, &cc->chipcontrol_data, 0x04);

		/* Wait for completion */
		SPINWAIT(0 == (R_REG(osh, &cc->chipstatus) & 0x08), 10 * 1000 * 1000);
		ASSERT(R_REG(osh, &cc->chipstatus) & 0x08);

		/* Drop the LSB to convert from 5 bit code to 4 bit code */
		rcal_code =  (uint8)(R_REG(osh, &cc->chipstatus) >> 5) & 0x0f;

		PMU_MSG(("RCal completed, status 0x%x, code 0x%x\n",
			R_REG(osh, &cc->chipstatus), rcal_code));

		/* Write RCal code into pmu_vreg_ctrl[32:29] */
		W_REG(osh, &cc->regcontrol_addr, 0);
		val = R_REG(osh, &cc->regcontrol_data) & ~((uint32)0x07 << 29);
		val |= (uint32)(rcal_code & 0x07) << 29;
		W_REG(osh, &cc->regcontrol_data, val);
		W_REG(osh, &cc->regcontrol_addr, 1);
		val = R_REG(osh, &cc->regcontrol_data) & ~(uint32)0x01;
		val |= (uint32)((rcal_code >> 3) & 0x01);
		W_REG(osh, &cc->regcontrol_data, val);

		/* Write RCal code into pmu_chip_ctrl[33:30] */
		W_REG(osh, &cc->chipcontrol_addr, 0);
		val = R_REG(osh, &cc->chipcontrol_data) & ~((uint32)0x03 << 30);
		val |= (uint32)(rcal_code & 0x03) << 30;
		W_REG(osh, &cc->chipcontrol_data, val);
		W_REG(osh, &cc->chipcontrol_addr, 1);
		val = R_REG(osh, &cc->chipcontrol_data) & ~(uint32)0x03;
		val |= (uint32)((rcal_code >> 2) & 0x03);
		W_REG(osh, &cc->chipcontrol_data, val);

		/* Set override in pmu_chip_ctrl[29] */
		W_REG(osh, &cc->chipcontrol_addr, 0);
		OR_REG(osh, &cc->chipcontrol_data, (0x01 << 29));

		/* Power off RCal block */
		W_REG(osh, &cc->chipcontrol_addr, 1);
		AND_REG(osh, &cc->chipcontrol_data, ~0x04);

		break;
	}
#endif	/* UNRELEASEDCHIP */
	default:
		break;
	}

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

void
si_pmu_spuravoid(si_t *sih, osl_t *osh, bool spuravoid)
{
	chipcregs_t *cc;
	uint origidx;
	uint32 tmp;

	if (sih->pmurev == 5) {
		/* PMU_ERROR(("Avoiding PLL programming on pmu rev 5\n")); */
		return;
	}

	/* Remember original core before switch to chipc */
	origidx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	/* change PHY default PLL mode to be integer-N mode (PR52185) */
	W_REG(osh, &cc->pllcontrol_addr, PMU1_PLL0_PLLCTL0);
	W_REG(osh, &cc->pllcontrol_data, 0x11100070);
	W_REG(osh, &cc->pllcontrol_addr, PMU1_PLL0_PLLCTL1);
	W_REG(osh, &cc->pllcontrol_data, 0x1014140a);
	W_REG(osh, &cc->pllcontrol_addr, PMU1_PLL0_PLLCTL5);
	W_REG(osh, &cc->pllcontrol_data, 0x88888854);

	if (spuravoid) { /* spur_avoid ON, enable 41/82/164Mhz clock mode */
		W_REG(osh, &cc->pllcontrol_addr, PMU1_PLL0_PLLCTL2);
		W_REG(osh, &cc->pllcontrol_data, 0x05201828);
	} else { /* enable 40/80/160Mhz clock mode */
		W_REG(osh, &cc->pllcontrol_addr, PMU1_PLL0_PLLCTL2);
		W_REG(osh, &cc->pllcontrol_data, 0x05001828);
	}

	tmp = R_REG(osh, &cc->pmucontrol);
	W_REG(osh, &cc->pmucontrol, tmp | (1 << 10));  

	/* Return to original core */
	si_setcoreidx(sih, origidx);
}

bool
si_pmu_is_otp_powered(si_t *sih, osl_t *osh)
{
	uint idx;
	chipcregs_t *cc;
	bool st;

	/* Retrieve OTP region info */
	idx = si_coreidx(sih);
	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc != NULL);

	switch (CHIPID(sih->chip)) {
	case BCM4325_CHIP_ID:
		st = (R_REG(osh, &cc->res_state) & PMURES_BIT(RES4325_LNLDO2_PU)) != 0;
		break;
#if defined(UNRELEASEDCHIP)
	case BCM4329_CHIP_ID:
		st = (R_REG(osh, &cc->res_state) & PMURES_BIT(RES4329_LNLDO2_PU)) != 0;
		break;
	case BCM4315_CHIP_ID:
		st = (R_REG(osh, &cc->res_state) & PMURES_BIT(RES4315_OTP_PU)) != 0;
		break;
	case BCM4319_CHIP_ID:
		st = (R_REG(osh, &cc->res_state) & PMURES_BIT(RES4319_OTP_PU)) != 0;
		break;
#endif	/* UNRELEASEDCHIP */
	default:
		st = TRUE;
		break;
	}

	si_setcoreidx(sih, idx);
	return st;
}

/* initialize PMU chip controls */
void
BCMINITFN(si_pmu_chip_init)(si_t *sih, osl_t *osh)
{
	ASSERT(sih->cccaps & CC_CAP_PMU);
}

/* initialize PMU switch/regulators */
void
BCMINITFN(si_pmu_swreg_init)(si_t *sih, osl_t *osh)
{
	ASSERT(sih->cccaps & CC_CAP_PMU);

	switch (CHIPID(sih->chip)) {
	case BCM4325_CHIP_ID:
		if ((sih->chiprev >= 3) &&
		    (((sih->chipst & CST4325_PMUTOP_2B_MASK) >>
		      CST4325_PMUTOP_2B_SHIFT) == 1)) {
			/* Bump CLDO PWM output voltage to 1.25V */
			si_pmu_set_ldo_voltage(sih, osh, SET_LDO_VOLTAGE_CLDO_PWM, 0xf);
			/* Bump CBUCK PWM output voltage to 1.5V */
			si_pmu_set_ldo_voltage(sih, osh, SET_LDO_VOLTAGE_CBUCK_PWM, 0xb);
		}
		break;
	default:
		break;
	}
}
#endif /* !BCMDONGLEHOST */
