/*
 * Misc utility routines for accessing chip-specific features
 * of the SiliconBackplane-based Broadcom chips.
 *
 * Copyright(c) 2001 Broadcom Corp.
 * $Id: sbutils.c,v 1.2 2006/06/30 22:15:30 korchakd Exp $
 */

#include <hnbutypedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <sbconfig.h>
#include <sbchipc.h>
#include <sbpci.h>
#include <sbpcmcia.h>
#include <sbextif.h>
#include <sbutils.h>
#include <bcmsrom.h>
#include <bcmendian.h>
#ifndef BROADCOM_BSP
#include <bcmnvram.h>
#endif

#define BCMDBG 0
/* debug/trace */
#ifdef	BCMDBG
#define	SB_ERROR(args)	printf args
#else
#define	SB_ERROR(args)
#endif


/* misc sb info needed by some of the routines */
typedef struct sb_info {
	uint	chip;			/* chip number */
	uint	chiprev;		/* chip revision */
	uint16	board;			/* board id */
	uint16	boardvendor;		/* board vendor id */
	uint	bus;			/* what bus type we are going through */

	void	*osh;			/* osl os handle */

	void	*curmap;		/* current regs va */
	void	*origregs;		/* initial core registers va */
	uint	origcoreidx;		/* initial core index */

	uint	pciidx;			/* pci core index */
	uint	pcirev;			/* pci core rev */

        uint    ccrev;                  /* chipc core rev */

	uint	gpioidx;		/* gpio control core index */
	uint	gpioid;			/* gpio control coretype */

	uint	numcores;		/* # discovered cores */
	uint	coreid[SB_MAXCORES];	/* id of each core */
} sb_info_t;

/* local prototypes */
static void sb_scan(sb_info_t *si);
static uint sb_corereg(void *sbh, uint coreidx, uint regoff, uint mask, uint val);
static uint sb_findcoreidx(void *sbh, uint coreid, uint coreunit);
static uint sb_pcidev2chip(uint pcidev);
static int sb_initvars(sb_info_t *si, char **vars, int *count);
static int srominitvars(sb_info_t *si, char **vars, int *count);
static int cisinitvars(sb_info_t *si, char **vars, int *count);
/* A cis parsing routine that can be called externally */
int parsecis(uint8 *cis, char **vars, int *count);
static void msecs(uint ms);

#define	SB_INFO(sbh)	(sb_info_t*)sbh
#define	SET_SBREG(sbh, r, mask, val)	W_SBREG((sbh), (r), ((R_SBREG(r) & ~(mask)) | (val)));
#define	GOODCOREADDR(x)	(((x) >= SB_ENUM_BASE) && ((x) <= SB_ENUM_LIM) \
				&& ISALIGNED((x), SB_CORE_SIZE))
#define	GOODREGS(regs)	(regs && ISALIGNED(regs, SB_CORE_SIZE))
#define	REGS2SB(va)	(sbconfig_t*) ((uint)(va) + SBCONFIGOFF)
#define	GOODIDX(idx)	(((uint)idx) < SB_MAXCORES)
#define	BADIDX		(SB_MAXCORES+1)

/*
 * Macros to read/write sbconfig registers. On the PCMCIA core rev 0
 * we need to treat them differently from other registers. See PR 3863.
 */
#define	R_SBREG(sbr)		R_REG(sbr)
#define	W_SBREG(sbh, sbr, v)	sb_write_sbreg((sbh), (sbr), (v))
#define	AND_SBREG(sbh, sbr, v)	W_SBREG((sbh), (sbr), (R_SBREG(sbr) & (v)))
#define	OR_SBREG(sbh, sbr, v)	W_SBREG((sbh), (sbr), (R_SBREG(sbr) | (v)))

/* power control defines */
#define	PLL_DELAY	150			/* 150us pll on delay */
#define	FREF_DELAY	15			/* 15us fref change delay */
#define	LPOMINFREQ	25000			/* low power oscillator min */
#define	LPOMAXFREQ	43000			/* low power oscillator max */
#define	XTALMINFREQ	19800000		/* 20mhz - 1% */
#define	XTALMAXFREQ	20200000		/* 20mhz + 1% */
#define	PCIMINFREQ	25000000		/* 25mhz */
#define	PCIMAXFREQ	34000000		/* 33mhz + fudge */

/* implement PR3863 workaround for the PCMCIA core rev 0 */
static void
sb_write_sbreg(void *sbh, volatile uint32 *sbr, uint32 v)
{
	sb_info_t *si;
	volatile uint32 dummy;

	si = SB_INFO(sbh);

	if ((si->bus == PCMCIA_BUS) && (sb_corerev(sbh) == 0)) {
#ifdef IL_BIGENDIAN
		dummy = R_REG(sbr);
		W_REG((volatile uint16 *)((uint32)sbr + 2), (uint16)((v >> 16) & 0xffff));
		dummy = R_REG(sbr);
		W_REG((volatile uint16 *)sbr, (uint16)(v & 0xffff));
#else
		dummy = R_REG(sbr);
		W_REG((volatile uint16 *)sbr, (uint16)(v & 0xffff));
		dummy = R_REG(sbr);
		W_REG((volatile uint16 *)((uint32)sbr + 2), (uint16)((v >> 16) & 0xffff));
#endif
	} else
		W_REG(sbr, v);
}

#if 0

/*
 * Allocate a sb handle.
 * devid - pci device id (used to determine chip#)
 * osh - opaque OS handle
 * regs - virtual address of initial core registers
 * vars - pointer to a pointer area for "environment" variables
 * varsz - pointer to int to return the size of the vars
 */
void*
sb_attach(uint devid, void *osh, void *regs, char **vars, int *varsz)
{
	sb_info_t *si;
	uint ccidx;
	uint chipid;
	uint32 w;

	ASSERT(GOODREGS(regs));

	/* alloc sb_info_t */
	if ((si = MALLOC(sizeof (sb_info_t))) == NULL) {
		SB_ERROR(("sb_attach: malloc failed!\n"));
		return (NULL);
	}
	bzero((uchar*)si, sizeof (sb_info_t));

	si->pciidx = si->gpioidx = BADIDX;

	si->osh = osh;
	si->origregs = si->curmap = regs;

	/* determine bus type we're operating over */
	if (sb_coreid((void *)si) == SB_PCMCIA) {
		si->bus = PCMCIA_BUS;
	} else {
        if (osh == NULL) {
			si->bus = SB_BUS;
        } else {
		/* check to see if we are a sb core mimic'ing a pci core */
		if (OSL_PCI_READ_CONFIG(osh, PCI_SPROM_CONTROL, sizeof (uint32)) == 0xffffffff)
			si->bus = SB_BUS;
		else
			si->bus = PCI_BUS;
        }
	}

	if (si->bus == SB_BUS)
		printf("SB_BUS\n");
	else
		printf("PCI_BUS %d\n", si->bus);

	/* keep and reuse the initial register mapping */
	si->origcoreidx = sb_coreidx((void*)si);

	/* scan for cores */
	sb_scan(si);

	/* pci core is required */
	if (!GOODIDX(si->pciidx)) {
		SB_ERROR(("sb_attach: pci core not found\n"));
		goto bad;
	}

	/* gpio control core is required */
	if (!GOODIDX(si->gpioidx)) {
		SB_ERROR(("sb_attach: gpio control core not found\n"));
		goto bad;
	}

	/* initialize the vars */
	if (sb_initvars(si, vars, varsz) != 0) {
		SB_ERROR(("sb_attach: sb_initvars failed\n"));
		goto bad;
	}

	/* determine chip id and rev */
	ccidx = sb_findcoreidx((void*)si, SB_CC, 0);
	if (GOODIDX(ccidx)) {
		/* easy - found chipcommon core */
		chipid = sb_corereg((void*)si, ccidx, OFFSETOF(chipcregs_t, chipid), 0, 0);
		si->chip = chipid & CID_ID_MASK;
		si->chiprev = (chipid & CID_REV_MASK) >> CID_REV_SHIFT;

		/* PR10654: The chipid register on 4306a1 lies */
		if ((si->chip == 0x4306) && (si->chiprev == 0))
			goto bad4306;
	} else {
		/* hard - no chipcommon core - must convert device id to chip id */
		if ((si->chip = sb_pcidev2chip(devid)) == 0) {
			SB_ERROR(("sb_attach: unrecognized device id 0x%04x\n", devid));
			goto bad;
		}

		/*
		 * The chip revision number is hardwired into all
		 * of the pci function config rev fields and is
		 * independent from the individual core revision numbers.
		 * For example, the "A0" silicon of each chip is chip rev 0.
		 * For PCMCIA we get it from the CIS instead.
		 */
		if (si->bus == PCMCIA_BUS)
#ifndef BROADCOM_BSP  /* we don't use PCMCIA */
			si->chiprev = (uint)getintvar(*vars, "chiprev");
#else
                        ;
#endif
		else {
bad4306:		w = OSL_PCI_READ_CONFIG(osh, 8, sizeof (uint32));
			si->chiprev = w & 0xff;
		}
	}

	/* sanity checks */
	ASSERT(si->chip);
    /* 4704A1 is chiprev 8 :-( */
    ASSERT((si->chiprev < 8) ||
           ((si->chip == BCM4704_DEVICE_ID) && ((si->chiprev == 8))));

	/* get boardtype and boardrev */
	if (si->bus == PCMCIA_BUS) {
#ifndef BROADCOM_BSP
		si->board = (uint16)getintvar(*vars, "prodid");
		si->boardvendor = (uint16)getintvar(*vars, "manfid");
#endif
	} else {
		/* do a pci config read to get subsystem id and subvendor id */
		w = OSL_PCI_READ_CONFIG(osh, 0x2c, sizeof (uint32));
		si->board = (w >> 16) & 0xffff;
		si->boardvendor = w & 0xffff;
	}

	/* XXX temp d11 kludge */
	if (si->bus == PCMCIA_BUS) {
		/* set the d11 core instead of pcmcia */
		regs = sb_setcore((void *)si, SB_D11, 0);

		/* reset origregs, curmap and origcoreidx */
		si->origregs = si->curmap = regs;
		si->origcoreidx = sb_coreidx((void*)si);
	}

	return ((void*)si);

bad:
	MFREE(si, sizeof (sb_info_t));
	return (NULL);
}
#endif

/* kernel caller variant of sb_attach() */
void*
sb_kattach(uint chip, uint chiprev)
{
	sb_info_t *si;
	uint ccidx, chipid;

	/* alloc sb_info_t */
	if ((si = MALLOC(sizeof (sb_info_t))) == NULL) {
		SB_ERROR(("sb_kattach: malloc failed!\n"));
		return (NULL);
	}
	bzero((uchar*)si, sizeof (sb_info_t));

	si->pciidx = si->gpioidx = BADIDX;
	si->bus = SB_BUS;

	/* core0 is default */
	si->origcoreidx = 0;
	si->origregs = si->curmap = (void*)REG_MAP(SB_ENUM_BASE, SB_CORE_SIZE);

	/* scan for cores */
	sb_scan(si);

	/* pci core is required */
	if (!GOODIDX(si->pciidx)) {
		SB_ERROR(("sb_attach: pci core not found\n"));
		goto bad;
	}

	/* gpio control core is required */
	if (!GOODIDX(si->gpioidx)) {
		SB_ERROR(("sb_attach: gpio control core not found\n"));
		goto bad;
	}

	/* determine chip id and rev */
	ccidx = sb_findcoreidx((void*)si, SB_CC, 0);
	if (GOODIDX(ccidx)) {
		/* easy - chipcommon core exists */
		chipid = sb_corereg((void*)si, ccidx, OFFSETOF(chipcregs_t, chipid), 0, 0);
		si->chip = chipid & CID_ID_MASK;
		si->chiprev = (chipid & CID_REV_MASK) >> CID_REV_SHIFT;
	}
	else {
		/* no chipcommon core - just use what we're told */
		si->chip = chip;
		si->chiprev = chiprev;
	}

	/* sanity checks */
	ASSERT(chip);
	ASSERT(chiprev < 8);

	return ((void*)si);

bad:
	MFREE(si, sizeof (sb_info_t));
	return (NULL);
}

uint
sb_coreid(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);


	return ((R_SBREG(&(sb)->sbidhigh) & SBIDH_CC_MASK) >> SBIDH_CC_SHIFT);
}

/* return current index of core */
uint
sb_coreidx(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;
	uint32 sbaddr = 0;

	si = SB_INFO(sbh);
	ASSERT(si);

	switch (si->bus) {
	case SB_BUS:
		sb = REGS2SB(si->curmap);
		sbaddr = sb_base(R_SBREG(&sb->sbadmatch0));
		break;

	case PCI_BUS:
		sbaddr = OSL_PCI_READ_CONFIG(si->osh, PCI_BAR0_WIN, sizeof (uint32));
		break;

	case PCMCIA_BUS: {
		uint8 tmp;

		OSL_PCMCIA_READ_ATTR(si->osh, PCMCIA_ADDR0, &tmp, 1);
		sbaddr  = (uint)tmp << 12;
		OSL_PCMCIA_READ_ATTR(si->osh, PCMCIA_ADDR1, &tmp, 1);
		sbaddr |= (uint)tmp << 16;
		OSL_PCMCIA_READ_ATTR(si->osh, PCMCIA_ADDR2, &tmp, 1);
		sbaddr |= (uint)tmp << 24;
		break;
	}

	default:
		ASSERT(0);
	}

	ASSERT(GOODCOREADDR(sbaddr));
	return ((sbaddr - SB_ENUM_BASE)/SB_CORE_SIZE);
}

uint
sb_corevendor(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);

	return ((R_SBREG(&(sb)->sbidhigh) & SBIDH_VC_MASK) >> SBIDH_VC_SHIFT);
}

uint
sb_corerev(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);

	return (R_SBREG(&(sb)->sbidhigh) & SBIDH_RC_MASK);
}

#define	SBTML_ALLOW	(SBTML_PE | SBTML_FGC | SBTML_FL_MASK)

/* set/clear one or more sbtmstatelow core-specific flag or pme bits */
uint32
sb_coreflags(void *sbh, uint32 mask, uint32 val)
{
	sb_info_t *si;
	sbconfig_t *sb;
	uint32 w;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);

	ASSERT((val & ~mask) == 0);
	ASSERT((mask & ~SBTML_ALLOW) == 0);

	/* mask and set */
	if (mask || val) {
		w = (R_SBREG(&sb->sbtmstatelow) & ~mask) | val;
		W_SBREG(sbh, &sb->sbtmstatelow, w);
	}

	/* return the new value */
	return (R_SBREG(&sb->sbtmstatelow) & SBTML_ALLOW);
}

bool
sb_iscoreup(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);

	return ((R_SBREG(&(sb)->sbtmstatelow) & (SBTML_RESET | SBTML_REJ | SBTML_CLK)) == SBTML_CLK);
}

/*
 * Switch to 'coreidx', issue a single arbitrary 32bit register mask&set operation,
 * switch back to the original core, and return the new value.
 */
static uint
sb_corereg(void *sbh, uint coreidx, uint regoff, uint mask, uint val)
{
	sb_info_t *si;
	uint origidx;
	uint32 *r;
	uint w;

	ASSERT(GOODIDX(coreidx));
	ASSERT(regoff < SB_CORE_SIZE);
	ASSERT((val & ~mask) == 0);

	si = SB_INFO(sbh);

	/* save current core index */
	origidx = sb_coreidx(sbh);

	/* switch core */
	if (origidx != coreidx)
		r = (uint32*) ((uint) sb_setcoreidx(sbh, coreidx) + regoff);
	else
		r = (uint32*) ((uint) si->curmap + regoff);

	/* mask and set */
	if (mask || val) {
		w = (R_SBREG(r) & ~mask) | val;
		W_SBREG(sbh, r, w);
	}

	/* readback */
	w = R_SBREG(r);

	/* restore core index */
	if (origidx != coreidx)
		sb_setcoreidx(sbh, origidx);

	return (w);
}

/* scan the sb enumerated space to find all cores */
static void
sb_scan(sb_info_t *si)
{
	void *sbh;
	sbconfig_t *sb;
	uint32 sbidhigh;
	void *regs;
	uint busidx;
	uint idx;
	uint id;
	int i;

	sbh = (void*) si;

	/* busidx is the index of our "active" pci or pcmcia core */
	busidx = BADIDX;

	/* required by sb_setcoreidx() */
	si->numcores = SB_MAXCORES;

	/* save current core index */
	idx = sb_coreidx(sbh);

	for (i = 0; i < SB_MAXCORES; i++) {
		regs = sb_setcoreidx(sbh, i);
		sb = REGS2SB(regs);

		/* target abort at end of enumerated cores */
		sbidhigh = 0;
		BUSPROBE(sbidhigh, (&sb->sbidhigh));
		if ((sbidhigh == 0xffffffff) || (sbidhigh == 0))
			break;

		/*
		 * Some PC chipsets don't return all ones or all zeros on target abort.
		 * Check pci/pcmcia for sbimstate.inbanderror.
		 */
		if (GOODIDX(busidx) && (sb_corereg(sbh, busidx,
			(SBCONFIGOFF + SBIMSTATE), 0, 0) & SBIM_IBE))
				break;

		/* found a core */
		id = sb_coreid(sbh);

		si->coreid[i] = id;

		/* locate our bus adapter core and clear sbimstate.inbanderror */
		if (((si->bus == PCI_BUS) && (id == SB_PCI))
			|| ((si->bus == PCMCIA_BUS) && (id == SB_PCMCIA))) {
			busidx = i;
			SET_SBREG(sbh, &sb->sbimstate, SBIM_IBE, 0);
		}

		/* save pci core rev */
		if (id == SB_PCI)
			si->pcirev = sb_corerev(sbh);
	}

	si->numcores = i;

	/* find pci core */
	si->pciidx = sb_findcoreidx(sbh, SB_PCI, 0);

	/*
	 * Find the gpio "controlling core" type and index.
	 * Precedence:
	 * - if there's a chip common core - use that
	 * - else if there's a pci core (rev >= 2) - use that
	 * - else there had better be an extif core (4710 only)
	 */
	if (GOODIDX(sb_findcoreidx(sbh, SB_CC, 0))) {
		si->gpioidx = sb_findcoreidx(sbh, SB_CC, 0);
		si->gpioid = SB_CC;
	} else if (GOODIDX(si->pciidx) && (si->pcirev >= 2)) {
		si->gpioidx = si->pciidx;
		si->gpioid = SB_PCI;
	} else if (sb_findcoreidx(sbh, SB_EXTIF, 0)) {
		si->gpioidx = sb_findcoreidx(sbh, SB_EXTIF, 0);
		si->gpioid = SB_EXTIF;
	}

	/* restore core index */
	sb_setcoreidx(sbh, idx);
}

void
sb_detach(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);

	if (si == NULL)
		return;

	if ((si->bus == SB_BUS) && si->curmap && (si->curmap != si->origregs)) {
		ASSERT(GOODREGS(si->curmap));
		REG_UNMAP(si->curmap);
		si->curmap = NULL;
	}

	MFREE(si, sizeof (sb_info_t));
}

/* use pci dev id to determine chip id for chips not having a chipcommon core */
static uint
sb_pcidev2chip(uint pcidev)
{
	if ((pcidev >= BCM4710_DEVICE_ID) && (pcidev <= BCM47XX_USB_ID))
		return (BCM4710_DEVICE_ID);
	if ((pcidev >= BCM4610_DEVICE_ID) && (pcidev <= BCM4610_USB_ID))
		return (BCM4610_DEVICE_ID);
	if ((pcidev >= BCM4402_DEVICE_ID) && (pcidev <= BCM4402_V90_ID))
		return (BCM4402_DEVICE_ID);
	if ((pcidev >= BCM4307_V90_ID) && (pcidev <= BCM4307_D11B_ID))
		return (BCM4307_DEVICE_ID);
	if (pcidev == BCM4301_DEVICE_ID)
		return (BCM4301_DEVICE_ID);
	if ((pcidev >= BCM4309_DEVICE_ID) && (pcidev <= BCM4309_D11DUAL_ID))
		return (BCM4309_DEVICE_ID);

	return (0);
}

/* return index of coreid or BADIDX if not found */
static uint
sb_findcoreidx(void *sbh, uint coreid, uint coreunit)
{
	sb_info_t *si;
	uint found;
	uint i;

	si = SB_INFO(sbh);
	found = 0;

	for (i = 0; i < si->numcores; i++)
		if (si->coreid[i] == coreid) {
			if (found == coreunit)
				return (i);
			found++;
		}

	return (BADIDX);
}

/* change logical "focus" to the indiciated core */
void*
sb_setcoreidx(void *sbh, uint coreidx)
{
	sb_info_t *si;
	uint32 sbaddr;
	uint8 tmp;

	si = SB_INFO(sbh);

	if (coreidx >= si->numcores) {
		printf("sb_setcoreidx: coreidx %d >= si->numcores %d failed\n", coreidx, si->numcores);
		return (NULL);
	}

	sbaddr = SB_ENUM_BASE + (coreidx * SB_CORE_SIZE);

	switch (si->bus) {
	case SB_BUS:
		/* unmap any previous one */
		if (si->curmap && (si->curmap != si->origregs)) {
			ASSERT(GOODREGS(si->curmap));
			REG_UNMAP(si->curmap);
			si->curmap = NULL;
		}

		/* keep and reuse the initial register mapping */
		if (coreidx == si->origcoreidx) {
			si->curmap = si->origregs;
			return (si->curmap);
		}

		/* map new one */
		si->curmap = (void*)REG_MAP(sbaddr, SB_CORE_SIZE);
		ASSERT(GOODREGS(si->curmap));
		break;

	case PCI_BUS:
		/* point bar0 window */
		OSL_PCI_WRITE_CONFIG(si->osh, PCI_BAR0_WIN, 4, sbaddr);
		break;

	case PCMCIA_BUS:
		tmp = (sbaddr >> 12) & 0x0f;
		OSL_PCMCIA_WRITE_ATTR(si->osh, PCMCIA_ADDR0, &tmp, 1);
		tmp = (sbaddr >> 16) & 0xff;
		OSL_PCMCIA_WRITE_ATTR(si->osh, PCMCIA_ADDR1, &tmp, 1);
		tmp = (sbaddr >> 24) & 0xff;
		OSL_PCMCIA_WRITE_ATTR(si->osh, PCMCIA_ADDR2, &tmp, 1);
		break;
	}

	return (si->curmap);
}

/* change logical "focus" to the indicated core */
void*
sb_setcore(void *sbh, uint coreid, uint coreunit)
{
	sb_info_t *si;
	uint idx;

	si = SB_INFO(sbh);

	idx = sb_findcoreidx(sbh, coreid, coreunit);
	if (!GOODIDX(idx))
		return (NULL);

	return (sb_setcoreidx(sbh, idx));
}

/* return chip number */
uint
sb_chip(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->chip);
}

/* return chip revision number */
uint
sb_chiprev(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->chiprev);
}

/* return board type */
uint16
sb_board(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->board);
}

/* return board vendor id */
uint16
sb_boardvendor(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->boardvendor);
}

/* return boolean if sbh device is in pci hostmode or client mode */
uint
sb_bus(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	return (si->bus);
}

/* return list of found cores */
uint
sb_corelist(void *sbh, uint coreid[])
{
	sb_info_t *si;

	si = SB_INFO(sbh);

	bcopy((uchar*)si->coreid, (uchar*)coreid, (si->numcores * sizeof (uint)));
	return (si->numcores);
}

/* do buffered registers update */
void
sb_commit(void *sbh)
{
	sb_info_t *si;
	sbpciregs_t *pciregs;
	uint idx;

	si = SB_INFO(sbh);
	idx = BADIDX;

	/* switch over to the pci core if necessary */
	if (sb_coreid(sbh) == SB_PCI)
		pciregs = (sbpciregs_t*) si->curmap;
	else {
		/* save current core index */
		idx = sb_coreidx(sbh);
		ASSERT(GOODIDX(idx));

		/* switch over to pci core */
		pciregs = (sbpciregs_t*) sb_setcore(sbh, SB_PCI, 0);
	}

	/* do the buffer registers update */
	W_REG(&pciregs->bcastaddr, SB_COMMIT);
	W_REG(&pciregs->bcastdata, 0x0);

	/* restore core index */
	if (GOODIDX(idx))
		sb_setcoreidx(sbh, idx);
}

/* reset and re-enable a core */
void
sb_core_reset(void *sbh, uint32 bits)
{
	sb_info_t *si;
	sbconfig_t *sb;
	volatile uint32 dummy;

	si = SB_INFO(sbh);
	ASSERT(GOODREGS(si->curmap));
	sb = REGS2SB(si->curmap);

	/*
	 * Must do the disable sequence first to work for arbitrary current core state.
	 */
	sb_core_disable(sbh, bits);

	/*
	 * Now do the initialization sequence.
	 */

	/* set reset while enabling the clock and forcing them on throughout the core */
	W_SBREG(sbh, &sb->sbtmstatelow, (SBTML_FGC | SBTML_CLK | SBTML_RESET | bits));
	dummy = R_SBREG(&sb->sbtmstatelow);

	/*
	 * PR6594 - 4610 iline100 overloads the clock enable bit with the
	 * oscillator/pll power up function which needs a much longer delay.
	 */
	if (sb_coreid(sbh) == SB_ILINE100) {
		msecs(50);
	} else {
		OSL_DELAY(1);
	}

	/* PR3158 - clear any serror */
	if (R_SBREG(&sb->sbtmstatehigh) & SBTMH_SERR) {
		W_SBREG(sbh, &sb->sbtmstatehigh, 0);
	}
	if ((dummy = R_SBREG(&sb->sbimstate)) & (SBIM_IBE | SBIM_TO)) {
		AND_SBREG(sbh, &sb->sbimstate, ~(SBIM_IBE | SBIM_TO));
	}

	/* clear reset and allow it to propagate throughout the core */
	W_SBREG(sbh, &sb->sbtmstatelow, (SBTML_FGC | SBTML_CLK | bits));
	dummy = R_SBREG(&sb->sbtmstatelow);
	OSL_DELAY(1);

	/* leave clock enabled */
	W_SBREG(sbh, &sb->sbtmstatelow, (SBTML_CLK | bits));
	dummy = R_SBREG(&sb->sbtmstatelow);
	OSL_DELAY(1);
}

/*
 * Do the PR4755 workaround which require a call
 * to sb_commit. sb_commit needs to access PCI
 * configuration space which is not allowed in some
 * versions of windows when going down. That is why
 * this is not part of sb_core_reset above.
 */
void
sb_core_tofixup(void *sbh)
{
	sb_info_t *si;
	sbconfig_t *sb;

	si = SB_INFO(sbh);
	ASSERT(GOODREGS(si->curmap));
	sb = REGS2SB(si->curmap);

	/* PR 9962/4708: Set initiator timeouts. */
	if (si->bus == SB_BUS) {
		SET_SBREG(sbh, &sb->sbimconfiglow,
			  SBIMCL_RTO_MASK | SBIMCL_STO_MASK,
			  (0x5 << SBIMCL_RTO_SHIFT) | 0x3);
	} else {
		if (sb_coreid(sbh) == SB_PCI) {
			SET_SBREG(sbh, &sb->sbimconfiglow,
				  SBIMCL_RTO_MASK | SBIMCL_STO_MASK,
				  (0x3 << SBIMCL_RTO_SHIFT) | 0x2);
		} else {
			SET_SBREG(sbh, &sb->sbimconfiglow, (SBIMCL_RTO_MASK | SBIMCL_STO_MASK), 0);
		}
	}

	sb_commit(sbh);
}

void
sb_core_disable(void *sbh, uint32 bits)
{
	sb_info_t *si;
	volatile uint32 dummy;
	sbconfig_t *sb;

	si = SB_INFO(sbh);

	ASSERT(GOODREGS(si->curmap));
	sb = REGS2SB(si->curmap);

	/* must return if core is already in reset */
	if (R_SBREG(&sb->sbtmstatelow) & SBTML_RESET)
		return;

	/* set the reject bit */
	W_SBREG(sbh, &sb->sbtmstatelow, (SBTML_CLK | SBTML_REJ));

	/* spin until reject is set */
	while ((R_SBREG(&sb->sbtmstatelow) & SBTML_REJ) == 0)
		OSL_DELAY(1);

	/* spin until sbtmstatehigh.busy is clear */
	while (R_SBREG(&sb->sbtmstatehigh) & SBTMH_BUSY)
		OSL_DELAY(1);

	/* set reset and reject while enabling the clocks */
	W_SBREG(sbh, &sb->sbtmstatelow, (bits | SBTML_FGC | SBTML_CLK | SBTML_REJ | SBTML_RESET));
	dummy = R_SBREG(&sb->sbtmstatelow);
	OSL_DELAY(10);

	/* leave reset and reject asserted */
	W_SBREG(sbh, &sb->sbtmstatelow, (bits | SBTML_REJ | SBTML_RESET));
	OSL_DELAY(1);
}

void
sb_chip_reset(void *sbh)
{
	sb_info_t *si = SB_INFO(sbh);

	/* instant NMI */
	switch (si->gpioid) {
	case SB_CC:
		sb_corereg(sbh, si->gpioidx, OFFSETOF(chipcregs_t, watchdog), ~0, 1);
		break;
	case SB_EXTIF:
		sb_corereg(sbh, si->gpioidx, OFFSETOF(extifregs_t, watchdog), ~0, 1);
		break;
	}
}

/* initialize the pcmcia core */
void
sb_pcmcia_init(void *sbh)
{
	sb_info_t *si;
	void *regs;
	uint8 cor;

	si = SB_INFO(sbh);

	/* Set the d11 core instead of pcmcia */
	regs = sb_setcore(sbh, SB_D11, 0);

	/* Reset origregs, curmap and origcoreidx */
	si->origregs = si->curmap = regs;
	si->origcoreidx = sb_coreidx((void*)si);

	/* Enable interrupts via function 2 FCR */
	OSL_PCMCIA_READ_ATTR(si->osh, PCMCIA_FCR0 + PCMCIA_COR, &cor, 1);
	OSL_PCMCIA_READ_ATTR(si->osh, PCMCIA_FCR2 + PCMCIA_COR, &cor, 1);
	cor |= COR_IRQEN | COR_FUNEN;
	OSL_PCMCIA_WRITE_ATTR(si->osh, PCMCIA_FCR2 + PCMCIA_COR, &cor, 1);

	/* XXX What else */
}

/*
 * Configure the pci core for pci client (NIC) action
 * and get appropriate dma offset value.
 * coremask is the bitvec of cores by index to be enabled.
 */
void
sb_pci_setup(void *sbh, uint32 *dmaoffset, uint coremask)
{
	sb_info_t *si;
	sbconfig_t *sb;
	sbpciregs_t *pciregs;
	uint32 sbflag;
	uint32 w;
	uint idx;

	ASSERT(coremask);

	si = SB_INFO(sbh);

	if (dmaoffset)
		*dmaoffset = 0;

	/* if not pci bus, we're done */
	if (si->bus != PCI_BUS)
		return;

	ASSERT(si->pciidx);

	/* get current core index */
	idx = sb_coreidx(sbh);

	/* we interrupt on this backplane flag number */
	ASSERT(GOODREGS(si->curmap));
	sb = REGS2SB(si->curmap);
	sbflag = R_SBREG(&sb->sbtpsflag) & SBTPS_NUM0_MASK;

	/* switch over to pci core */
	pciregs = (sbpciregs_t*) sb_setcoreidx(sbh, si->pciidx);
	sb = REGS2SB(pciregs);

	/*
	 * Enable sb->pci interrupts.  Assume
	 * PCI rev 2.3 support was added in pci core rev 6 and things changed..
	 */
	if (si->pcirev < 6) {
		/* set sbintvec bit for our flag number */
		/* PR6075 - both d11 cores on the 4309 use the same flag */
		OR_SBREG(sbh, &sb->sbintvec, (1 << sbflag));
	} else {
		/* pci config write to set this core bit in PCIIntMask */
		w = OSL_PCI_READ_CONFIG(si->osh, PCI_INT_MASK, sizeof(uint32));
		w |= (coremask << SBIM_SHIFT);
		OSL_PCI_WRITE_CONFIG(si->osh, PCI_INT_MASK, sizeof(uint32), w);
	}

	/* enable prefetch and bursts for sonics-to-pci translation 2 */
	OR_REG(&pciregs->sbtopci2, (SBTOPCI_PREF|SBTOPCI_BURST));

	/* PR 9962/4708: Set initiator timeouts. */
	SET_SBREG(sbh, &sb->sbimconfiglow, SBIMCL_RTO_MASK | SBIMCL_STO_MASK,
		  (0x3 << SBIMCL_RTO_SHIFT) | 0x2);
	sb_commit(sbh);

	/* switch back to previous core */
	sb_setcoreidx(sbh, idx);

	/* use large sb pci dma window */
	if (dmaoffset)
		*dmaoffset = SB_PCI_DMA;
}

uint32
sb_base(uint32 admatch)
{
	uint32 base;
	uint type;

	type = admatch & SBAM_TYPE_MASK;
	ASSERT(type < 3);

	base = 0;

	if (type == 0) {
		base = admatch & SBAM_BASE0_MASK;
	} else if (type == 1) {
		ASSERT(!(admatch & SBAM_ADNEG));	/* neg not supported */
		base = admatch & SBAM_BASE1_MASK;
	} else if (type == 2) {
		ASSERT(!(admatch & SBAM_ADNEG));	/* neg not supported */
		base = admatch & SBAM_BASE2_MASK;
	}

	return (base);
}

uint32
sb_size(uint32 admatch)
{
	uint32 size;
	uint type;

	type = admatch & SBAM_TYPE_MASK;
	ASSERT(type < 3);

	size = 0;

	if (type == 0) {
		size = 1 << (((admatch & SBAM_ADINT0_MASK) >> SBAM_ADINT0_SHIFT) + 1);
	} else if (type == 1) {
		ASSERT(!(admatch & SBAM_ADNEG));	/* neg not supported */
		size = 1 << (((admatch & SBAM_ADINT1_MASK) >> SBAM_ADINT1_SHIFT) + 1);
	} else if (type == 2) {
		ASSERT(!(admatch & SBAM_ADNEG));	/* neg not supported */
		size = 1 << (((admatch & SBAM_ADINT2_MASK) >> SBAM_ADINT2_SHIFT) + 1);
	}

	return (size);
}

/* return the core-type instantiation # of the current core */
uint
sb_coreunit(void *sbh)
{
	sb_info_t *si;
	uint idx;
	uint coreid;
	uint coreunit;
	uint i;

	si = SB_INFO(sbh);
	coreunit = 0;

	idx = sb_coreidx(sbh);

	ASSERT(GOODREGS(si->curmap));
	coreid = sb_coreid(sbh);

	/* count the cores of our type */
	for (i = 0; i < idx; i++)
		if (si->coreid[i] == coreid)
			coreunit++;

	return (coreunit);
}

static INLINE uint32
factor6(uint32 x)
{
	switch (x) {
	case CC_F6_2:	return 2;
	case CC_F6_3:	return 3;
	case CC_F6_4:	return 4;
	case CC_F6_5:	return 5;
	case CC_F6_6:	return 6;
	case CC_F6_7:	return 7;
	default:	return 0;
	}
}

/* calculate the speed the SB would run at given a set of clockcontrol values */
uint32
sb_clock_rate(uint32 n, uint32 m)
{
	uint32 n1, n2, fastclock, m1, m2, m3;

	n1 = factor6(n & CN_N1_MASK);
	n2 = ((n & CN_N2_MASK) >> CN_N2_SHIFT) + CC_F5_BIAS;

	fastclock = CC_CLOCK_BASE * n1 * n2;

	if (fastclock == 0)
		return 0;

	m1 = factor6(m & CC_M1_MASK);
	m2 = ((m & CC_M2_MASK) >> CC_M2_SHIFT) + CC_F5_BIAS;
	m3 = factor6((m & CC_M3_MASK) >> CC_M3_SHIFT);

	switch ((m & CC_MC_MASK) >> CC_MC_SHIFT) {
	case CC_MC_BYPASS:	return (fastclock);
	case CC_MC_M1:		return (fastclock / m1);
	case CC_MC_M1M2:	return (fastclock / (m1 * m2));
	case CC_MC_M1M2M3:	return (fastclock / (m1 * m2 * m3));
	case CC_MC_M1M3:	return (fastclock / (m1 * m3));
	default:		return (0);
	}
}

/* returns the current speed the SB is running at */
uint32
sb_clock(void *sbh)
{
/* #ifndef BCM5365*/
#if 0
        extifregs_t *eir;
	chipcregs_t *cc;
	volatile uint32 *clockcontrol_n, *clockcontrol_sb;
	uint idx;
	uint32 rate;

	/* get index of the current core */
	idx = sb_coreidx(sbh);

	/* switch to extif or chipc core */
	if ((eir = (extifregs_t *) sb_setcore(sbh, SB_EXTIF, 0))) {
		clockcontrol_n = &eir->clockcontrol_n;
		clockcontrol_sb = &eir->clockcontrol_sb;
	} else if ((cc = (chipcregs_t *) sb_setcore(sbh, SB_CC, 0))) {
		clockcontrol_n = &cc->clockcontrol_n;
		clockcontrol_sb = &cc->clockcontrol_sb;
	} else
		return 0;

	/* calculate rate */
	rate = sb_clock_rate(R_REG(clockcontrol_n), R_REG(clockcontrol_sb));

	/* switch back to previous core */
	sb_setcoreidx(sbh, idx);

	return rate;
#else
	return 100000000; /* 5365, usually has 100M backplane */
#endif	
}

/* set the current speed of the SB to the desired rate (as closely as possible) */
bool
sb_setclock(void *sbh, uint32 sb, uint32 pci)
{
	extifregs_t *eir;
	chipcregs_t *cc;
	volatile uint32 *clockcontrol_n, *clockcontrol_sb, *clockcontrol_pci;
	uint32 orig_n, orig_pci, orig_sb, pci_clock=0;
	uint idx, i;
	struct {
		uint32 clock;
		uint16 n;
		uint32 sb;
		uint32 pci33;
		uint32 pci25;
	} sb_clock_table[] = {
		{  96000000, 0x0303, 0x04020011, 0x11030011, 0x11050011 }, /*  96.000 32.000 24.000 */
		{ 100000000, 0x0009, 0x04020011, 0x11030011, 0x11050011 }, /* 100.000 33.333 25.000 */
		{ 104000000, 0x0802, 0x04020011, 0x11050009, 0x11090009 }, /* 104.000 31.200 24.960 */
		{ 108000000, 0x0403, 0x04020011, 0x11050009, 0x02000802 }, /* 108.000 32.400 24.923 */
		{ 112000000, 0x0205, 0x04020011, 0x11030021, 0x02000403 }, /* 112.000 32.000 24.889 */
		{ 115200000, 0x0303, 0x04020009, 0x11030011, 0x11050011 }, /* 115.200 32.000 24.000 */
		{ 120000000, 0x0011, 0x04020011, 0x11050011, 0x11090011 }, /* 120.000 30.000 24.000 */
		{ 124800000, 0x0802, 0x04020009, 0x11050009, 0x11090009 }, /* 124.800 31.200 24.960 */
		{ 128000000, 0x0305, 0x04020011, 0x11050011, 0x02000305 }, /* 128.000 32.000 24.000 */
		{ 132000000, 0x0603, 0x04020011, 0x11050011, 0x02000305 }, /* 132.000 33.000 24.750 */
		{ 136000000, 0x0c02, 0x04020011, 0x11090009, 0x02000603 }, /* 136.000 32.640 24.727 */
		{ 140000000, 0x0021, 0x04020011, 0x11050021, 0x02000c02 }, /* 140.000 30.000 24.706 */
		{ 144000000, 0x0405, 0x04020011, 0x01020202, 0x11090021 }, /* 144.000 30.857 24.686 */
		{ 150857142, 0x0605, 0x04020021, 0x02000305, 0x02000605 }, /* 150.857 33.000 24.000 */
		{ 152000000, 0x0e02, 0x04020011, 0x11050021, 0x02000e02 }, /* 152.000 32.571 24.000 */
		{ 156000000, 0x0802, 0x04020005, 0x11050009, 0x11090009 }, /* 156.000 31.200 24.960 */
		{ 160000000, 0x0309, 0x04020011, 0x11090011, 0x02000309 }, /* 160.000 32.000 24.000 */
		{ 163200000, 0x0c02, 0x04020009, 0x11090009, 0x02000603 }, /* 163.200 32.640 24.727 */
		{ 168000000, 0x0205, 0x04020005, 0x11030021, 0x02000403 }, /* 168.000 32.000 24.889 */
		{ 176000000, 0x0602, 0x04020003, 0x11050005, 0x02000602 }, /* 176.000 33.000 24.000 */
	};

	/* get index of the current core */
	idx = sb_coreidx(sbh);

	/* switch to extif or chipc core */
	if ((eir = (extifregs_t *) sb_setcore(sbh, SB_EXTIF, 0))) {
		clockcontrol_n = &eir->clockcontrol_n;
		clockcontrol_sb = &eir->clockcontrol_sb;
		clockcontrol_pci = &eir->clockcontrol_pci;
	} else if ((cc = (chipcregs_t *) sb_setcore(sbh, SB_CC, 0))) {
		clockcontrol_n = &cc->clockcontrol_n;
		clockcontrol_sb = &cc->clockcontrol_sb;
		clockcontrol_pci = &cc->clockcontrol_pci;
	} else
		return 0;

	/* Store the current clock reg values */
	orig_n = R_REG(clockcontrol_n);
	orig_sb = R_REG(clockcontrol_sb);
	orig_pci = R_REG(clockcontrol_pci);

	/* keep current pci clock if not specified */
	if (!pci) {
		pci = sb_clock_rate(R_REG(clockcontrol_n), R_REG(clockcontrol_pci));
		pci = (pci <= 25000000) ? 25000000 : 33000000;
	}

	for (i = 1; i < (sizeof(sb_clock_table)/sizeof(sb_clock_table[0])); i++) {
		if ((sb >= sb_clock_table[i-1].clock) && (sb < sb_clock_table[i].clock)) {
			ASSERT(sb_clock_table[i-1].clock ==
				sb_clock_rate(sb_clock_table[i-1].n, sb_clock_table[i-1].sb));
			W_REG(clockcontrol_n, sb_clock_table[i-1].n);
			W_REG(clockcontrol_sb, sb_clock_table[i-1].sb);
			if (pci == 25000000)
				pci_clock = sb_clock_table[i-1].pci25;
			else
				pci_clock = sb_clock_table[i-1].pci33;
			W_REG(clockcontrol_pci, pci_clock);
			break;
		}
	}

	/* switch back to previous core */
	sb_setcoreidx(sbh, idx);

	/* Were the clock rates in the table */
	if (i < (sizeof(sb_clock_table)/sizeof(sb_clock_table[0])))	{
		/* reset if any clocks changed */
		if ((orig_n != sb_clock_table[i-1].n) ||
			(orig_sb != sb_clock_table[i-1].sb) ||
			(orig_pci != pci_clock))
			sb_chip_reset(sbh);
		else
			return TRUE;
	}

	return FALSE;
}

#include <ethernet.h>	/* for sprom content groking */

/*
 * Read in and validate sprom.
 * Return 0 on success, nonzero on error.
 */
static int
spromread(uint16 *sprom, uint byteoff, uint16 *buf, uint nbytes, bool check_crc)
{
	int off, nw;
	uint8 chk8;
	int i;

	off = byteoff / 2;
	nw = ROUNDUP(nbytes, 2) / 2;

	/* read the sprom */
	for (i = 0; i < nw; i++)
		buf[i] = R_REG(&sprom[off + i]);

	if (check_crc) {
		/* fixup the endianness so crc8 will pass */
		htol16_buf(buf, nw * 2);
		if ((chk8 = crc8((uchar*)buf, nbytes, CRC8_INIT_VALUE)) != CRC8_GOOD_VALUE)
			return (1);
		/* now correct the endianness of the byte array */
		ltoh16_buf(buf, nw * 2);
	}

	return (0);
}

#define	VARS_MAX	4096

/*
 * Initialize nonvolatile variable table from sprom.
 * Return 0 on success, nonzero on error.
 */

static int
srominitvars(sb_info_t *si, char **vars, int *count)
{
	uint16 w, b[64];
	uint8 spromversion;
	struct ether_addr ea;
	char eabuf[32];
	int c, woff, i;
	char *vp, *base;

	if (spromread((void *)((uint)si->curmap + PCI_BAR0_SPROM_OFFSET), 0, b, sizeof (b), TRUE))
		return (-1);

	/* top word of sprom contains version and crc8 */
	spromversion = b[63] & 0xff;
	if (spromversion != 1) {
		return (-2);
	}

	ASSERT(vars);
	ASSERT(count);

	base = vp = MALLOC(VARS_MAX);
	ASSERT(vp);

	/* parameter section of sprom starts at byte offset 72 */
	woff = 72/2;

	/* first 6 bytes are il0macaddr */
	ea.octet[0] = (b[woff] >> 8) & 0xff;
	ea.octet[1] = b[woff] & 0xff;
	ea.octet[2] = (b[woff+1] >> 8) & 0xff;
	ea.octet[3] = b[woff+1] & 0xff;
	ea.octet[4] = (b[woff+2] >> 8) & 0xff;
	ea.octet[5] = b[woff+2] & 0xff;
	woff += ETHER_ADDR_LEN/2 ;
	bcm_ether_ntoa((uchar*)&ea, eabuf);
	vp += sprintf(vp, "il0macaddr=%s", eabuf);
	vp++;

	/* next 6 bytes are et0macaddr */
	ea.octet[0] = (b[woff] >> 8) & 0xff;
	ea.octet[1] = b[woff] & 0xff;
	ea.octet[2] = (b[woff+1] >> 8) & 0xff;
	ea.octet[3] = b[woff+1] & 0xff;
	ea.octet[4] = (b[woff+2] >> 8) & 0xff;
	ea.octet[5] = b[woff+2] & 0xff;
	woff += ETHER_ADDR_LEN/2 ;
	bcm_ether_ntoa((uchar*)&ea, eabuf);
	vp += sprintf(vp, "et0macaddr=%s", eabuf);
	vp++;

	/* next 6 bytes are et1macaddr */
	ea.octet[0] = (b[woff] >> 8) & 0xff;
	ea.octet[1] = b[woff] & 0xff;
	ea.octet[2] = (b[woff+1] >> 8) & 0xff;
	ea.octet[3] = b[woff+1] & 0xff;
	ea.octet[4] = (b[woff+2] >> 8) & 0xff;
	ea.octet[5] = b[woff+2] & 0xff;
	woff += ETHER_ADDR_LEN/2 ;
	bcm_ether_ntoa((uchar*)&ea, eabuf);
	vp += sprintf(vp, "et1macaddr=%s", eabuf);
	vp++;

	/*
	 * Enet phy settings one or two singles or a dual
	 * Bits 4-0 : MII address for enet0 (0x1f for not there)
	 * Bits 9-5 : MII address for enet1 (0x1f for not there)
	 * Bit 14   : Mdio for enet0
	 * Bit 15   : Mdio for enet1
	 */
	w = b[woff];
	vp += sprintf(vp, "et0phyaddr=%d", (w & 0x1f));
	vp++;
	vp += sprintf(vp, "et1phyaddr=%d", ((w >> 5) & 0x1f));
	vp++;
	vp += sprintf(vp, "et0mdcport=%d", ((w >> 14) & 0x1));
	vp++;
	vp += sprintf(vp, "et1mdcport=%d", ((w >> 15) & 0x1));
	vp++;

	/* Word 46 has board rev, antennas 0/1 & Country code */
	w = b[46];
	vp += sprintf(vp, "boardrev=%d", w & 0xff);
	vp++;

	vp += sprintf(vp, "cc=%d", (w >> 8) & 0xf);
	vp++;

	vp += sprintf(vp, "aa0=%d", (w >> 12) & 0x3);
	vp++;

	vp += sprintf(vp, "aa1=%d", (w >> 14) & 0x3);
	vp++;

	/* Word 52 is max power 0/1 */
	w = b[52];
	vp += sprintf(vp, "pa0maxpwr=%d", w & 0xff);
	vp++;
	vp += sprintf(vp, "pa1maxpwr=%d", (w >> 8) & 0xff);
	vp++;

	/* set the (wl) pa settings */
	woff = 47; /* start of pa param's section */

	for (i = 0; i < 5; i++) {
		vp += sprintf(vp, "pa0b%d=%d", i, b[woff+i]);
		vp++;
		vp += sprintf(vp, "pa1b%d=%d", i, b[woff+i+6]);
		vp++;
	}

	/* Word 58 is antenna gain 0/1 */
	w = b[58];
	vp += sprintf(vp, "ag0=%d", w & 0xff);
	vp++;

	vp += sprintf(vp, "ag1=%d", (w >> 8) & 0xff);
	vp++;

	/* set the oem string */
	vp += sprintf(vp, "oem=%02x%02x%02x%02x%02x%02x%02x%02x",
		((b[woff] >> 8) & 0xff), (b[woff] & 0xff),
		((b[woff+1] >> 8) & 0xff), (b[woff+1] & 0xff),
		((b[woff+2] >> 8) & 0xff), (b[woff+2] & 0xff),
		((b[woff+3] >> 8) & 0xff), (b[woff+3] & 0xff));
	vp++;

	/* final nullbyte terminator */
	*vp++ = '\0';

	c = vp - base;
	ASSERT(c <= VARS_MAX);

	if (c == VARS_MAX) {
		*vars = base;
	} else {
		vp = MALLOC(c);
		ASSERT(vp);
		bcopy(base, vp, c);
		MFREE(base, VARS_MAX);
		*vars = vp;
	}
	*count = c;

	return (0);
}

/* set PCMCIA sprom command register */
static int
pcsr_cmd(sb_info_t *si, uint8 cmd)
{
	uint8 status;
	uint wait_cnt = 1000;

	/* write sprom command register */
	OSL_PCMCIA_WRITE_ATTR(si->osh, SROM_CS, &cmd, 1);

	/* wait status */
	while (wait_cnt--) {
		OSL_PCMCIA_READ_ATTR(si->osh, SROM_CS, &status, 1);
		if (status & SROM_DONE)
			return 0;
	}
	return 1;
}

/* read a word from the PCMCIA srom */
static int
pcsr_read(sb_info_t *si, uint16 addr, uint16 *data)
{
	uint8 addr_l, addr_h, data_l, data_h;

	addr_l = (uint8)((addr * 2) & 0xff);
	addr_h = (uint8)(((addr * 2) >> 8) & 0xff);

	/* set address */
	OSL_PCMCIA_WRITE_ATTR(si->osh, SROM_ADDRH, &addr_h, 1);
	OSL_PCMCIA_WRITE_ATTR(si->osh, SROM_ADDRL, &addr_l, 1);

	/* do read */
	if (pcsr_cmd(si, SROM_READ))
		return 1;

	/* read data */
	OSL_PCMCIA_READ_ATTR(si->osh, SROM_DATAH, &data_h, 1);
	OSL_PCMCIA_READ_ATTR(si->osh, SROM_DATAL, &data_l, 1);

	*data = (data_h << 8) | data_l;
	return 0;
}

/* write a word to the PCMCIA srom */
static int
pcsr_write(sb_info_t *si, uint16 addr, uint16 data)
{
	uint8 addr_l, addr_h, data_l, data_h;

	addr_l = (uint8)((addr * 2) & 0xff);
	addr_h = (uint8)(((addr * 2) >> 8) & 0xff);
	data_l = (uint8)(data & 0xff);
	data_h = (uint8)((data >> 8) & 0xff);

	/* set address */
	OSL_PCMCIA_WRITE_ATTR(si->osh, SROM_ADDRH, &addr_h, 1);
	OSL_PCMCIA_WRITE_ATTR(si->osh, SROM_ADDRL, &addr_l, 1);

	/* write data */
	OSL_PCMCIA_WRITE_ATTR(si->osh, SROM_DATAH, &data_h, 1);
	OSL_PCMCIA_WRITE_ATTR(si->osh, SROM_DATAL, &data_l, 1);

	/* do write */
	return pcsr_cmd(si, SROM_WRITE);
}

/* support only 16-bit word read from srom */
int
sromread(void *sbh, uint byteoff, uint nbytes, uint16 *buf)
{
	void *srom;
	uint i, off, nw;
	sb_info_t *si = SB_INFO(sbh);

	/* check input - 16-bit access only */
	if (byteoff & 1 || nbytes & 1 || (byteoff + nbytes) > (SPROM_SIZE * 2))
		return 1;

	if (si->bus == PCI_BUS) {
		srom = (void *)((uint)si->curmap + PCI_BAR0_SPROM_OFFSET);
		if (spromread(srom, byteoff, buf, nbytes, FALSE))
			return 1;
	} else if (si->bus == PCMCIA_BUS) {
		off = byteoff / 2;
		nw = nbytes / 2;
		for (i = 0; i < nw; i++) {
			if (pcsr_read(si, off + i, buf + i))
				return 1;
		}
	} else {
		return 1;
	}

	return 0;
}

/* support only 16-bit word write into srom */
int
sromwrite(void *sbh, uint byteoff, uint nbytes, uint16 *buf)
{
	uint16 *srom;
	sb_info_t *si = SB_INFO(sbh);
	uint i, off, nw, crc_range;
	uint16 image[SPROM_SIZE], *p;
	uint8 crc;
	volatile uint32 val32;

	/* check input - 16-bit access only */
	if (byteoff & 1 || nbytes & 1 || (byteoff + nbytes) > (SPROM_SIZE * 2))
		return 1;

	crc_range = ((si->bus == PCMCIA_BUS) ? SPROM_SIZE : SPROM_CRC_RANGE) * 2;

	/* if changes made inside crc cover range */
	if (byteoff < crc_range) {
		nw = (((byteoff + nbytes) > crc_range) ? byteoff + nbytes : crc_range) / 2;
		/* read data including entire first 64 words from srom */
		if (sromread(sbh, 0, nw * 2, image))
			return 1;
		/* make changes */
		bcopy((void*)buf, (void*)&image[byteoff / 2], nbytes);
		/* calculate crc */
		htol16_buf(image, crc_range);
		crc = ~crc8((uint8 *)image, crc_range - 1, CRC8_INIT_VALUE);
		ltoh16_buf(image, crc_range);
		image[(crc_range / 2) - 1] = (crc << 8) | (image[(crc_range / 2) - 1] & 0xff);
		p = image;
		off = 0;
	} else {
		p = buf;
		off = byteoff / 2;
		nw = nbytes / 2;
	}

	if (si->bus == PCI_BUS) {
		srom = (uint16*)((uint)si->curmap + PCI_BAR0_SPROM_OFFSET);
		/* enable writes to the SPROM */
		val32 = OSL_PCI_READ_CONFIG(si->osh, PCI_SPROM_CONTROL, sizeof(uint32));
		val32 |= SPROM_WRITEEN;
		OSL_PCI_WRITE_CONFIG(si->osh, PCI_SPROM_CONTROL, sizeof(uint32), val32);
		msecs(500);
		/* write srom */
		for (i = 0; i < nw; i++) {
			W_REG(&srom[off + i], p[i]);
			msecs(20);
		}
		/* disable writes to the SPROM */
		OSL_PCI_WRITE_CONFIG(si->osh, PCI_SPROM_CONTROL, sizeof(uint32), val32 & ~SPROM_WRITEEN);
	} else if (si->bus == PCMCIA_BUS) {
		/* enable writes to the SPROM */
		if (pcsr_cmd(si, SROM_WEN))
			return 1;
		msecs(500);
		/* write srom */
		for (i = 0; i < nw; i++) {
			pcsr_write(si, off + i, p[i]);
			msecs(20);
		}
		/* disable writes to the SPROM */
		if (pcsr_cmd(si, SROM_WDS))
			return 1;
	} else {
		return 1;
	}

	msecs(500);
	return 0;
}

/*
 * "The sprom" in PCMCIA cards is simply the standard PCMCIA
 * CIS (Card Information Structure); so we have to parse the
 * CIS and extract from it into name=value pairs the information
 * we need: the mac address is a standard tuple; plus we add
 * vendor specific tuples for chip/revision ids, board revision,
 * country code lock, PA parameters and OEM space.
 * XXX: Should check a crc (there is a crc tuple that we could add).
 *
 * Return 0 on success, nonzero on error.
 */
int
parsecis(uint8 *cis, char **vars, int *count)
{
	char eabuf[32];
	char *vp, *base;
	uint8 tup, tlen;
	int i, j;
	uint varsize;
	bool ag_init = FALSE;

	ASSERT(vars);
	ASSERT(count);

	base = vp = MALLOC(VARS_MAX);
	ASSERT(vp);

	i = 0;
	do {
		tup = cis[i++];
		tlen = cis[i++];

		switch (tup) {
		case CISTPL_MANFID:
			vp += sprintf(vp, "manfid=%d", (cis[i + 1] << 8) + cis[i]);
			vp++;
			vp += sprintf(vp, "prodid=%d", (cis[i + 3] << 8) + cis[i + 2]);
			vp++;
			break;

		case CISTPL_FUNCE:
			if (cis[i] == LAN_NID) {
				ASSERT(cis[i + 1] == ETHER_ADDR_LEN);
				bcm_ether_ntoa((uchar*)&cis[i + 2], eabuf);
				vp += sprintf(vp, "il0macaddr=%s", eabuf);
				vp++;
			}
			break;

		case CISTPL_BRCM_HNBU:
			switch (cis[i]) {
			case HNBU_CHIPID:
				vp += sprintf(vp, "vendid=%d", (cis[i + 2] << 8) + cis[i + 1]);
				vp++;
				vp += sprintf(vp, "devid=%d", (cis[i + 4] << 8) + cis[i + 3]);
				vp++;
				vp += sprintf(vp, "chiprev=%d", (cis[i + 6] << 8) + cis[i + 5]);
				vp++;
				break;

			case HNBU_BOARDREV:
				vp += sprintf(vp, "boardrev=%d", cis[i + 1]);
				vp++;
				break;

			case HNBU_AA:
				vp += sprintf(vp, "aa0=%d", cis[i + 1]);
				vp++;
				break;

			case HNBU_AG:
				vp += sprintf(vp, "ag0=%d", cis[i + 1]);
				vp++;
				ag_init = TRUE;
				break;

			case HNBU_CC:
				vp += sprintf(vp, "cc=%d", cis[i + 1]);
				vp++;
				break;

			case HNBU_PAPARMS:
				for (j = 0; j < 5; j++) {
					vp += sprintf(vp, "pa0b%d=%d", j,
						(cis[i + (j * 2) + 2] << 8) + cis[i + (j * 2) + 1]);
					vp++;
				}
				vp += sprintf(vp, "pa0maxpwr=%d", cis[i + 11]);
				vp++;
				break;

			case HNBU_OEM:
				vp += sprintf(vp, "oem=%02x%02x%02x%02x%02x%02x%02x%02x",
					cis[i + 1], cis[i + 2], cis[i + 3], cis[i + 4],
					cis[i + 5], cis[i + 6], cis[i + 7], cis[i + 8]);
				vp++;
				break;
			}
			break;

		}
		i += tlen;
	} while (tup != 0xff);

	/* if there is no antenna gain field, set default */
	if (ag_init == FALSE) {
		vp += sprintf(vp, "ag0=%d", 0xff);
		vp++;
	}

	/* final nullbyte terminator */
	*vp++ = '\0';
	varsize = (uint)vp - (uint)base;

	ASSERT(varsize < VARS_MAX);

	if (varsize == VARS_MAX) {
		*vars = base;
	} else {
		vp = MALLOC(varsize);
		ASSERT(vp);
		bcopy(base, vp, varsize);
		MFREE(base, VARS_MAX);
		*vars = vp;
	}
	*count = varsize;

	return (0);
}

/*
 * Read the cis and call parsecis to initialize the vars.
 * Return 0 on success, nonzero on error.
 */
static int
cisinitvars(sb_info_t *si, char **vars, int *count)
{
	uint8 *cis = NULL;
	int rc;

	if ((cis = MALLOC(CIS_SIZE)) == NULL)
		return (-1);

	OSL_PCMCIA_READ_ATTR(si->osh, 0, cis, CIS_SIZE);

	rc = parsecis(cis, vars, count);

	MFREE(cis, CIS_SIZE);

	return (rc);
}

/*
 * Initialize the vars from the right source for this platform.
 * Return 0 on success, nonzero on error.
 */
static int
sb_initvars(sb_info_t *si, char **vars, int *count)
{
	if (vars == NULL)
		return (0);

	switch (si->bus) {
	case SB_BUS:
		/* These two could be asserts ... */
		*vars = NULL;
		*count = 0;
		return(0);

	case PCI_BUS:
		return(srominitvars(si, vars, count));

	case PCMCIA_BUS:
		return(cisinitvars(si, vars, count));

	default:
		ASSERT(0);
	}
	return (-1);
}

#ifndef BROADCOM_BSP
/*
 * Search the vars for a specific one and return its value.
 * Returns NULL if not found.
 */
char*
getvar(char *vars, char *name)
{
	char *s;
	int len;

	len = strlen(name);

	if (vars == NULL)
		return(nvram_get(name));

	for (s = vars; s && *s; ) {
		if ((bcmp(s, name, len) == 0) && (s[len] == '='))
			return (&s[len+1]);

		while (*s++)
			;
	}
	return (NULL);
}

/*
 * Search the vars for a specific one and return its value as
 * an integer. Returns 0 if not found.
 */
int
getintvar(char *vars, char *name)
{
	char *val;

	if ((val = getvar(vars, name)) == NULL)
		return (0);

	return (bcm_strtoul(val, NULL, 0));
}
#endif

/* change logical "focus" to the gpio core for optimized access */
void*
sb_gpiosetcore(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);

	return (sb_setcoreidx(sbh, si->gpioidx));
}

/* mask&set gpiocontrol bits */
uint32
sb_gpiocontrol(void *sbh, uint32 mask, uint32 val)
{
	sb_info_t *si;
	uint regoff;

	si = SB_INFO(sbh);
	regoff = 0;

	switch (si->gpioid) {
	case SB_CC:
		regoff = OFFSETOF(chipcregs_t, gpiocontrol);
		break;

	case SB_PCI:
		regoff = OFFSETOF(sbpciregs_t, gpiocontrol);
		break;

	case SB_EXTIF:
		return (0);
	}

	return (sb_corereg(sbh, si->gpioidx, regoff, mask, val));
}

/* mask&set gpio output enable bits */
uint32
sb_gpioouten(void *sbh, uint32 mask, uint32 val)
{
	sb_info_t *si;
	uint regoff;

	si = SB_INFO(sbh);
	regoff = 0;

	switch (si->gpioid) {
	case SB_CC:
		regoff = OFFSETOF(chipcregs_t, gpioouten);
		break;

	case SB_PCI:
		regoff = OFFSETOF(sbpciregs_t, gpioouten);
		break;

	case SB_EXTIF:
		regoff = OFFSETOF(extifregs_t, gpio[0].outen);
		break;
	}

	return (sb_corereg(sbh, si->gpioidx, regoff, mask, val));
}

/* mask&set gpio output bits */
uint32
sb_gpioout(void *sbh, uint32 mask, uint32 val)
{
	sb_info_t *si;
	uint regoff;

	si = SB_INFO(sbh);
	regoff = 0;

	switch (si->gpioid) {
	case SB_CC:
		regoff = OFFSETOF(chipcregs_t, gpioout);
		break;

	case SB_PCI:
		regoff = OFFSETOF(sbpciregs_t, gpioout);
		break;

	case SB_EXTIF:
		regoff = OFFSETOF(extifregs_t, gpio[0].out);
		break;
	}

	return (sb_corereg(sbh, si->gpioidx, regoff, mask, val));
}

/* return the current gpioin register value */
uint32
sb_gpioin(void *sbh)
{
	sb_info_t *si;
	uint regoff;

	si = SB_INFO(sbh);
	regoff = 0;

	switch (si->gpioid) {
	case SB_CC:
		regoff = OFFSETOF(chipcregs_t, gpioin);
		break;

	case SB_PCI:
		regoff = OFFSETOF(sbpciregs_t, gpioin);
		break;

	case SB_EXTIF:
		regoff = OFFSETOF(extifregs_t, gpioin);
		break;
	}

	return (sb_corereg(sbh, si->gpioidx, regoff, 0, 0));
}

void
sb_dump(void *sbh, char *buf)
{
	sb_info_t *si;
	uint i;

	si = SB_INFO(sbh);

	buf += sprintf(buf, "si 0x%x chip 0x%x chiprev 0x%x boardtype 0x%x boardvendor 0x%x bus %d\n",
		(uint)si, si->chip, si->chiprev, si->board, si->boardvendor, si->bus); /* changed boardtype to board */
	buf += sprintf(buf, "osh 0x%x curmap 0x%x\n",
		(uint)si->osh, (uint)si->curmap);
	buf += sprintf(buf, "ccrev %d pcirev %d gpioidx %d gpioid 0x%x\n",
		si->ccrev, si->pcirev, si->gpioidx, si->gpioid);
	buf += sprintf(buf, "cores:  ");
	for (i = 0; i < si->numcores; i++)
		buf += sprintf(buf, "0x%x ", si->coreid[i]);
	buf += sprintf(buf, "\n");
}

/* Doing really long delays with DELAY breaks Linux:
 * it claims they would not be accurate on fast machines.
 */
static void
msecs(uint ms)
{
	uint i;

	for (i = 0; i < ms; i++) {
		OSL_DELAY(1000);
	}
}





/* return current register mapping */
void *
sb_coreregs(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
	ASSERT(GOODREGS(si->curmap));

	return (si->curmap);
}


/* Returns the SB interrupt flag of the current core. */
uint32
sb_flag(void *sbh)
{
	void *regs;
	sbconfig_t *sb;

	regs = sb_coreregs(sbh);
	sb = (sbconfig_t *)((ulong) regs + SBCONFIGOFF);

	return (R_REG(&sb->sbtpsflag) & SBTPS_NUM0_MASK);
}


static const uint32 sbips_int_mask[] = {
	0,
	SBIPS_INT1_MASK,
	SBIPS_INT2_MASK,
	SBIPS_INT3_MASK,
	SBIPS_INT4_MASK
};

static const uint32 sbips_int_shift[] = {
	0,
	0,
	SBIPS_INT2_SHIFT,
	SBIPS_INT3_SHIFT,
	SBIPS_INT4_SHIFT
};

/* 
 * Returns the MIPS IRQ assignment of the current core. If unassigned,
 * 0 is returned.
 */
uint
sb_irq(void *sbh)
{
	uint idx;
	void *regs;
	sbconfig_t *sb;
	uint32 flag, sbipsflag;
	uint irq = 0;

	flag = sb_flag(sbh);

	idx = sb_coreidx(sbh);

	if ((regs = sb_setcore(sbh, SB_MIPS, 0)) ||
	    (regs = sb_setcore(sbh, SB_MIPS33, 0))) {
		sb = (sbconfig_t *)((ulong) regs + SBCONFIGOFF);

		/* sbipsflag specifies which core is routed to interrupts 1 to 4 */
		sbipsflag = R_REG(&sb->sbipsflag);
		for (irq = 1; irq <= 4; irq++) {
			if (((sbipsflag & sbips_int_mask[irq]) >> sbips_int_shift[irq]) == flag)
				break;
		}
		if (irq == 5)
			irq = 0;
	}

	sb_setcoreidx(sbh, idx);

	return irq;
}

/* Clears the specified MIPS IRQ. */
static void
sb_clearirq(void *sbh, uint irq)
{
	void *regs;
	sbconfig_t *sb;

	if (!(regs = sb_setcore(sbh, SB_MIPS, 0)) &&
	    !(regs = sb_setcore(sbh, SB_MIPS33, 0)))
		ASSERT(regs);
	sb = (sbconfig_t *)((ulong) regs + SBCONFIGOFF);

	if (irq == 0)
		W_REG(&sb->sbintvec, 0);
	else
		OR_REG(&sb->sbipsflag, sbips_int_mask[irq]);
}

/* 
 * Assigns the specified MIPS IRQ to the specified core. Shared MIPS
 * IRQ 0 may be assigned more than once.
 */
static void
sb_setirq(void *sbh, uint irq, uint coreid, uint coreunit)
{
	void *regs;
	sbconfig_t *sb;
	uint32 flag;

	regs = sb_setcore(sbh, coreid, coreunit);
	ASSERT(regs);
	flag = sb_flag(sbh);

	if (!(regs = sb_setcore(sbh, SB_MIPS, 0)) &&
	    !(regs = sb_setcore(sbh, SB_MIPS33, 0)))
		ASSERT(regs);
	sb = (sbconfig_t *)((ulong) regs + SBCONFIGOFF);

	if (irq == 0)
		OR_REG(&sb->sbintvec, 1 << flag);
	else {
		flag <<= sbips_int_shift[irq];
		ASSERT(!(flag & ~sbips_int_mask[irq]));
		flag |= R_REG(&sb->sbipsflag) & ~sbips_int_mask[irq];
		W_REG(&sb->sbipsflag, flag);
	}
}	


void
sb_init_irq(void* sbh)
{
    int irq;
    /* Clear interrupt map */
    for (irq = 0; irq <= 4; irq++)
	sb_clearirq(sbh, irq);
    sb_setirq(sbh, 0, SB_CODEC, 0);
    sb_setirq(sbh, 0, SB_EXTIF, 0);
    sb_setirq(sbh, 2, SB_ENET, 1);
    sb_setirq(sbh, 3, SB_ILINE20, 0);
    sb_setirq(sbh, 4, SB_PCI, 0);
}


/* integrated from new sbutils.c */

#include <pcicfg.h>

/* return boardtype */
uint
sb_boardtype(void *sbh)
{
	sb_info_t *si;

	si = SB_INFO(sbh);
#if 0
	return (si->boardtype);
#else
	return (si->board);
#endif
}

/* return board bus style */
uint
sb_boardstyle(void *sbh)
{
	sb_info_t *si;
	uint16 w;

	si = SB_INFO(sbh);

	if (si->bus == PCMCIA_BUS)
		return (BOARDSTYLE_PCMCIA);

	if (si->bus == SB_BUS)
		return (BOARDSTYLE_SOC);

	/* bus is PCI */

	if (OSL_PCI_READ_CONFIG(si->osh, PCI_CFG_CIS, sizeof (uint32)) != 0)
		return (BOARDSTYLE_CARDBUS);

	if ((srom_read(si->bus, si->curmap, si->osh, (SPROM_SIZE - 1) * 2, 2, &w) == 0) &&
	    (w == 0x0313))
		return (BOARDSTYLE_CARDBUS);

	return (BOARDSTYLE_PCI);
}

/* set/clear sbtmstatehigh core-specific flags */
uint32
sb_coreflagshi(void *sbh, uint32 mask, uint32 val)
{
	sb_info_t *si;
	sbconfig_t *sb;
	uint32 w;

	si = SB_INFO(sbh);
	sb = REGS2SB(si->curmap);

	ASSERT((val & ~mask) == 0);
	ASSERT((mask & ~SBTMH_FL_MASK) == 0);

	/* mask and set */
	if (mask || val) {
		w = (R_SBREG(&sb->sbtmstatehigh) & ~mask) | val;
		W_SBREG(sbh, &sb->sbtmstatehigh, w);
	}

	/* return the new value */
	return (R_SBREG(&sb->sbtmstatehigh) & SBTMH_FL_MASK);
}

/* convert chip number to number of i/o cores */
static uint
sb_chip2numcores(uint chip)
{
	if (chip == 0x4710)
		return (9);
	if (chip == 0x4610)
		return (9);
	if (chip == 0x4402)
		return (3);
	if ((chip == 0x4307) || (chip == 0x4301))
		return (5);
	if (chip == 0x4310)
		return (8);
	if (chip == 0x4306)	/* < 4306c0 */
		return (6);
	if (chip == 0x4704)
		return (9);
	if (chip == 0x5365)
		return (7);

	SB_ERROR(("sb_chip2numcores: unsupported chip 0x%x\n", chip));
	ASSERT(0);
	return (1);
}
/* Check if a target abort has happened and clear it */
bool
sb_taclear(void *sbh)
{
	sb_info_t *si;
	bool rc = FALSE;

	si = SB_INFO(sbh);

	if (si->bus == PCI_BUS) {
		uint32 stcmd;

		stcmd = OSL_PCI_READ_CONFIG(si->osh, PCI_CFG_CMD, sizeof(stcmd));
		rc = (stcmd & 0x08000000) != 0;

		if (rc) {
			/* Target abort bit is set, clear it */
			OSL_PCI_WRITE_CONFIG(si->osh, PCI_CFG_CMD, sizeof(stcmd), stcmd);
		}
	}
	/* XXX: Other buses?? */

	return (rc);
}


/* scan the sb enumerated space to identify all cores */
static void
sb_doscan(sb_info_t *si)
{
	void *sbh;
	uint origidx;
	uint i;

	sbh = (void*) si;
	/* numcores should already be set */
	ASSERT((si->numcores > 0) && (si->numcores <= SB_MAXCORES));

	/* save current core index */
	origidx = sb_coreidx(sbh);

	si->pciidx = si->gpioidx = BADIDX;

	for (i = 0; i < si->numcores; i++) {
		sb_setcoreidx(sbh, i);

		si->coreid[i] = sb_coreid(sbh);


		if (si->coreid[i] == SB_PCI) {
			si->pciidx = i;
			si->pcirev = sb_corerev(sbh);
		}
	}

	/*
	 * Find the gpio "controlling core" type and index.
	 * Precedence:
	 * - if there's a chip common core - use that
	 * - else if there's a pci core (rev >= 2) - use that
	 * - else there had better be an extif core (4710 only)
	 */
	if (GOODIDX(sb_findcoreidx(sbh, SB_CC, 0))) {
		si->gpioidx = sb_findcoreidx(sbh, SB_CC, 0);
		si->gpioid = SB_CC;
	} else if (GOODIDX(si->pciidx) && (si->pcirev >= 2)) {
		si->gpioidx = si->pciidx;
		si->gpioid = SB_PCI;
	} else if (sb_findcoreidx(sbh, SB_EXTIF, 0)) {

	    si->gpioidx = sb_findcoreidx(sbh, SB_EXTIF, 0);
		si->gpioid = SB_EXTIF;
	}

	/* return to original core index */
	sb_setcoreidx(sbh, origidx);
}

#if 1

/* sb_doattach is made as sb_attach till */
void*
sb_attach(uint devid, void *osh, void *regs, uint bustype, char **vars, int *varsz)
{
	sb_info_t *si;
	chipcregs_t *cc;
	uint32 w;

	ASSERT(GOODREGS(regs));


	/* alloc sb_info_t */
	if ((si = MALLOC(sizeof (sb_info_t))) == NULL) {
		SB_ERROR(("sb_attach: malloc failed!\n"));
		return (NULL);
	}
	bzero((uchar*)si, sizeof (sb_info_t));

	si->pciidx = si->gpioidx = BADIDX;

	si->osh = osh;
	si->origregs = si->curmap = regs;

	/* check to see if we are a sb core mimic'ing a pci core */
	si->bus = bustype;


	if (si->bus == PCI_BUS) {
		if (OSL_PCI_READ_CONFIG(osh, PCI_SPROM_CONTROL, sizeof (uint32)) == 0xffffffff)
			si->bus = SB_BUS;
		else
			si->bus = PCI_BUS;
	}

	/* clear any previous epidiag-induced target abort */
	sb_taclear((void*)si);

	/* keep and reuse the initial register mapping */
	si->origcoreidx = sb_coreidx((void*)si);

	/* initialize the vars */
	if (srom_var_init(si->bus, si->curmap, osh, vars, varsz)) {
		SB_ERROR(("sb_attach: srom_var_init failed\n"));
		goto bad;
	}

	/* is core-0 a chipcommon core? */
	si->numcores = 1;
	cc = (chipcregs_t*) sb_setcoreidx((void*)si, 0);
	

	if (sb_coreid((void*)si) != SB_CC)
		cc = NULL;


	/* determine chip id and rev */
	if (cc) {
		/* chip common core found! */
		si->chip = R_REG(&cc->chipid) & CID_ID_MASK;
		si->chiprev = (R_REG(&cc->chipid) & CID_REV_MASK) >> CID_REV_SHIFT;
	} else {
		/* no chip common core -- must convert device id to chip id */
		/*printf("Hardwire BCM4710 device id for sb_doattach\n"); */
		if ((si->chip = sb_pcidev2chip(devid)) == 0) {
		/*if ((si->chip = sb_pcidev2chip(BCM4710_DEVICE_ID)) == 0) {*/
			SB_ERROR(("sb_attach: unrecognized device id 0x%04x\n", devid));
			goto bad;
		}

		/*
		 * The chip revision number is hardwired into all
		 * of the pci function config rev fields and is
		 * independent from the individual core revision numbers.
		 * For example, the "A0" silicon of each chip is chip rev 0.
		 * For PCMCIA we get it from the CIS instead.
		 */
		if (si->bus == PCMCIA_BUS) {
			ASSERT(vars);
			si->chiprev = getintvar(*vars, "chiprev");
		} else if (si->bus == PCI_BUS) {
			w = OSL_PCI_READ_CONFIG(osh, PCI_CFG_REV, sizeof (uint32));
			si->chiprev = w & 0xff;
		} else
			si->chiprev = 0;
	}
#if 0
	/* determine numcores */
	if (cc && (sb_corerev((void*)si) >= 4))
		si->numcores = (R_REG(&cc->chipid) & CID_CC_MASK) >> CID_CC_SHIFT;
	else
#endif
	    si->numcores = sb_chip2numcores(si->chip);

	/* return to original core */
	sb_setcoreidx((void*)si, si->origcoreidx);

	/* sanity checks */
	ASSERT(si->chip);
	ASSERT(si->chiprev < 8);

	/* scan for cores */
	/*sb_scan(si);*/
	sb_doscan(si);

	/* pci core is required */
	if (!GOODIDX(si->pciidx)) {
		SB_ERROR(("sb_attach: pci core not found\n"));
		goto bad;
	}

	/* gpio control core is required */
	if (!GOODIDX(si->gpioidx)) {
		SB_ERROR(("sb_attach: gpio control core not found\n"));
		goto bad;
	}

	/* get boardtype and boardrev */
	switch (si->bus) {
	case PCI_BUS:
		/* do a pci config read to get subsystem id and subvendor id */
		w = OSL_PCI_READ_CONFIG(osh, PCI_CFG_SVID, sizeof (uint32));
		si->boardvendor = w & 0xffff;
		si->board = (w >> 16) & 0xffff;
		break;

	case PCMCIA_BUS:
		si->boardvendor = getintvar(*vars, "manfid");
		si->board = getintvar(*vars, "prodid");
		break;

	case SB_BUS:
		si->boardvendor = VENDOR_BROADCOM;

		if (vars == NULL) {
			/* The caller obviously does not care */
			si->board = 0xffff;
			break;
		}

		/* boardtype format is a hex string */
		si->board = getintvar(*vars, "boardtype");

		break;
	}

	if (si->board == 0) {
		SB_ERROR(("sb_attach: unknown board type\n"));
#ifdef _CFE_
		si->board = BU5365_FPGA_BOARD;
#else		
		si->board = BU4704_BOARD;
#endif
	}


	return ((void*)si);

bad:
	MFREE(si, sizeof (sb_info_t));
	return (NULL);
}
#endif


/*
 * Return the slowclock min or max frequency.
 * Three sources of SLOW CLOCK:
 *	1. On Chip LPO         -     32khz or 160khz
 *	2. On Chip Xtal OSC    -     20mhz/4*(divider+1) 
 *	3. External PCI clock  -     66mhz/4*(divider+1)
 */
static uint
slowfreq(void *sbh, bool max)
{
	sb_info_t *si;
	chipcregs_t *cc;
	uint32 v;
	uint div;

	si = SB_INFO(sbh);

	ASSERT(sb_coreid(sbh) == SB_CC);

	cc = (chipcregs_t*) sb_setcoreidx(sbh, sb_coreidx(sbh));

	ASSERT(R_SBREG(&cc->capabilities) & CAP_PWR_CTL);

	if (si->ccrev < 6) {
		v = OSL_PCI_READ_CONFIG(si->osh, PCI_GPIO_OUT, sizeof (uint32));

		if (v & PCI_CFG_GPIO_SCS)
			return (max? (PCIMAXFREQ/64) : (PCIMINFREQ/64));
		else
			return (max? (XTALMAXFREQ/32) : (XTALMINFREQ/32));
	} else {
		v = R_REG(&cc->slow_clk_ctl) & SCC_SLOW_SEL_MASK;
		div = 4 * (((R_REG(&cc->slow_clk_ctl) & SCC_CLK_DIV_MASK) >> SCC_CLK_DIV_SHF) + 1);
		if (v == SCC_SLOWSEL_LPO)
			return (max? LPOMAXFREQ : LPOMINFREQ);
		else if (v == SCC_SLOWSEL_XTAL)
			return (max? (XTALMAXFREQ/div) : (XTALMINFREQ/div));
		else if (v == SCC_SLOWSEL_PCI)
			return (max? (PCIMAXFREQ/div) : (PCIMINFREQ/div));
		else
			ASSERT(0);
	}
	return (0);
}

/* initialize power control delay registers */
void
sb_pwrctl_init(void *sbh)
{
	sb_info_t *si;
	uint origidx;
	chipcregs_t *cc;
	uint slowmaxfreq;
	uint pll_on_delay, fref_sel_delay;

	si = SB_INFO(sbh);

	/* XXX add support for sdio */
	if (sb_bus(sbh) != PCI_BUS)
		return;

	origidx = sb_coreidx(sbh);

	if ((cc = (chipcregs_t*) sb_setcore(sbh, SB_CC, 0)) == NULL)
		return;

	if (!(R_SBREG(&cc->capabilities) & CAP_PWR_CTL))
		goto done;

	slowmaxfreq = slowfreq(sbh, TRUE);
	pll_on_delay = ((slowmaxfreq * PLL_DELAY) + 999999) / 1000000;
	fref_sel_delay = ((slowmaxfreq * FREF_DELAY) + 999999) / 1000000;

	W_REG(&cc->pll_on_delay, pll_on_delay);
	W_REG(&cc->fref_sel_delay, fref_sel_delay);

done:
	sb_setcoreidx(sbh, origidx);
}

/* return the value suitable for writing to the dot11 core FAST_PWRUP_DELAY register */
uint16
sb_pwrctl_fast_pwrup_delay(void *sbh)
{
	sb_info_t *si;
	uint origidx;
	chipcregs_t *cc;
	uint slowminfreq;
	uint16 fpdelay;

	si = SB_INFO(sbh);
	fpdelay = 0;
	origidx = sb_coreidx(sbh);

	/* XXX add support for sdio */
	if (sb_bus(sbh) != PCI_BUS)
		goto done;

	if ((cc = (chipcregs_t*) sb_setcore(sbh, SB_CC, 0)) == NULL)
		goto done;

	if (!(R_SBREG(&cc->capabilities) & CAP_PWR_CTL))
		goto done;

	slowminfreq = slowfreq(sbh, FALSE);
	fpdelay = (((R_REG(&cc->pll_on_delay) + 2) * 1000000) + (slowminfreq - 1)) / slowminfreq;

done:
	sb_setcoreidx(sbh, origidx);
	return (fpdelay);
}

/* turn both the pll and external oscillator on/off */
int
sb_pwrctl_xtal(void *sbh, uint on)
{
	sb_info_t *si;
	uint32 in, out, outen;

	si = SB_INFO(sbh);

	/* XXX add support for sdio */
	if (si->bus != PCI_BUS)
		return (-1);

	in = OSL_PCI_READ_CONFIG(si->osh, PCI_GPIO_IN, sizeof (uint32));
	out = OSL_PCI_READ_CONFIG(si->osh, PCI_GPIO_OUT, sizeof (uint32));
	outen = OSL_PCI_READ_CONFIG(si->osh, PCI_GPIO_OUTEN, sizeof (uint32));

	/*
	 * We can't actually read the state of the PLLPD so we infer it
	 * by the value of XTAL_PU which *is* readable via gpioin.
	 */
	if (on && (in & PCI_CFG_GPIO_XTAL))
		return (0);

	outen |= (PCI_CFG_GPIO_PLL | PCI_CFG_GPIO_XTAL);

	if (on) {
		out |= PCI_CFG_GPIO_PLL | PCI_CFG_GPIO_XTAL;
		OSL_PCI_WRITE_CONFIG(si->osh, PCI_GPIO_OUT, sizeof (uint32), out);
		OSL_PCI_WRITE_CONFIG(si->osh, PCI_GPIO_OUTEN, sizeof (uint32), outen);
		OSL_DELAY(1000);

		out &= ~PCI_CFG_GPIO_PLL;
		OSL_PCI_WRITE_CONFIG(si->osh, PCI_GPIO_OUT, sizeof (uint32), out);
		OSL_DELAY(5000);
	} else {
		out = PCI_CFG_GPIO_PLL | (out & ~PCI_CFG_GPIO_XTAL);
		OSL_PCI_WRITE_CONFIG(si->osh, PCI_GPIO_OUT, sizeof (uint32), out);
		OSL_PCI_WRITE_CONFIG(si->osh, PCI_GPIO_OUTEN, sizeof (uint32), outen);
	}

	return (0);
}

/* set dynamic power control mode (forceslow, forcefast, dynamic) */
int
sb_pwrctl_clk(void *sbh, uint mode)
{
	sb_info_t *si;
	uint origidx;
	chipcregs_t *cc;
	int error;

	si = SB_INFO(sbh);
	error = 0;

	/* XXX add support for SDIO */
	if (si->bus != PCI_BUS)
		return (-1);

	origidx = sb_coreidx(sbh);

	if ((cc = (chipcregs_t*) sb_setcore(sbh, SB_CC, 0)) == NULL)
		return (-1);

	if (!(R_SBREG(&cc->capabilities) & CAP_PWR_CTL)) {
		error = -1;
		goto done;
	}

	switch (mode) {
	case CLK_FAST:	/* force fast (pll) */

		/* ensure pll and xtal are on */
		sb_pwrctl_xtal(sbh, TRUE);

		if (si->ccrev >= 6)
			SET_REG(&cc->slow_clk_ctl, (SCC_FORCE_SLOW | SCC_FORCE_PLL), SCC_FORCE_PLL);
		break;

	case CLK_SLOW:	/* force slow clock */
		if (si->ccrev >= 6)
			OR_REG(&cc->slow_clk_ctl, SCC_FORCE_SLOW);
		break;

	case CLK_DYNAMIC:	/* enable dynamic power control */
		if (si->ccrev >= 6)
			SET_REG(&cc->slow_clk_ctl, (SCC_FORCE_SLOW | SCC_FORCE_PLL | SCC_DYN_XTAL),
				SCC_DYN_XTAL);
		break;
	}

done:
	sb_setcoreidx(sbh, origidx);
	return (error);
}

#ifdef BCMDBG
/* dump power control related registers */
int
sb_pwrctl_dump(void *sbh, char *buf)
{
	sb_info_t *si;
	chipcregs_t *cc;
	uint origidx;
	char *obuf;

	si = SB_INFO(sbh);
	obuf = buf;
	*buf = '\0';

	origidx = sb_coreidx(sbh);

	if ((cc = (chipcregs_t*) sb_setcore(sbh, SB_CC, 0)) == NULL)
		return (0);

	if (!(R_SBREG(&cc->capabilities) & CAP_PWR_CTL))
		goto done;

	buf += sprintf(buf, "pll_on_delay 0x%x fref_sel_delay 0x%x ",
		cc->pll_on_delay, cc->fref_sel_delay);
	if (si->ccrev >= 6)
		buf += sprintf(buf, "slow_clk_ctl 0x%x ", cc->slow_clk_ctl);
	if (si->bus == PCI_BUS)
		buf += sprintf(buf, "gpioout 0x%x gpioouten 0x%x ",
			OSL_PCI_READ_CONFIG(si->osh, PCI_GPIO_OUT, sizeof (uint32)),
			OSL_PCI_READ_CONFIG(si->osh, PCI_GPIO_OUTEN, sizeof (uint32)));
	buf += sprintf(buf, "\n");

done:
	sb_setcoreidx(sbh, origidx);
	return (buf - obuf);
}

#endif

/* copied from sbutils.h */
/*
* load 16-bit value from unaligned little endian byte array.
*/
uint16
ltoh16_ua(uint8 *bytes)
{
	return (bytes[1]<<8)+bytes[0];
}

/*
* load 32-bit value from unaligned little endian byte array.
*/
uint32
ltoh32_ua(uint8 *bytes)
{
	return (bytes[3]<<24)+(bytes[2]<<16)+(bytes[1]<<8)+bytes[0];
}

/*
* load 16-bit value from unaligned big(network) endian byte array.
*/
uint16
ntoh16_ua(uint8 *bytes)
{
	return (bytes[0]<<8)+bytes[1];
}

/*
* load 32-bit value from unaligned big(network) endian byte array.
*/
uint32
ntoh32_ua(uint8 *bytes)
{
	return (bytes[0]<<24)+(bytes[1]<<16)+(bytes[2]<<8)+bytes[3];
}

uint32
sb_EBbus_enable(void *sbh, int data_type)
{
    sb_info_t *si;
    uint regoff;
    uint32 regval;

    si = SB_INFO(sbh);
    regoff = 0;
    regval = 0;

    switch (si->gpioid) {
    case SB_CC:
        regoff = OFFSETOF(chipcregs_t, cs01config);
        break;
    }
    SB_ERROR(("sb_cs0enable reg offset %x\n",regoff));
    if (data_type == DATA_BIT_16){
        regval = sb_corereg(sbh, si->gpioidx, regoff, 0xffffffff, 0x11);
    } else if (data_type == DATA_BIT_8) {
        regval = sb_corereg(sbh, si->gpioidx, regoff, 0xffffffff, 0x1);
    }

    regoff = OFFSETOF(chipcregs_t, cs01memwaitcnt);
    return (sb_corereg(sbh, si->gpioidx, regoff, 0xffffffff, 0x1f1f1f3f));
}

