/*
 * Misc utility routines for accessing chip-specific features
 * of the SiliconBackplane-based Broadcom chips.
 *
 * $Id: sbutils.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/et/typedefs.h>
#include <shared/et/osl.h>
#include <shared/et/bcmdevs.h>
#include <shared/et/sbconfig.h>
#include <shared/et/sbchipc.h>
#include <shared/et/sbpci.h>
#include <shared/et/pcicfg.h>
#include <shared/et/sbpcmcia.h>
#include <shared/et/sbextif.h>
#include <shared/et/sbutils.h>
#include <shared/et/bcmendian.h>
#include <shared/et/bcmutils.h>
/* #include <bcmnvram.h> */

/* debug/trace */
#ifdef	BCMDBG
#define	SB_SOC_ERROR(args)	bsl_printf args
#else
#define	SB_SOC_ERROR(args)
#endif

/* misc sb info needed by some of the routines */
typedef struct sb_soc_info {
    uint	chip;			/* chip number */
    uint	chiprev;		/* chip revision */
    uint	boardtype;		/* board type */
    uint	boardvendor;		/* board vendor id */
    uint	bus;			/* what bus type we are going through */

    void	*osh;			/* osl os handle */

    void	*curmap;		/* current regs va */
    void	*origregs;		/* initial core registers va */

    uint	coreid;			/* id of each core */
} sb_soc_info_t;

/* local prototypes */
static void* sb_soc_doattach(uint devid, void *osh, void *regs, uint bustype);

#define	SB_SOC_INFO(sbh)	((sb_soc_info_t*)sbh)
#define	SOC_SET_SBREG(sbh, r, mask, val)	\
    SOC_W_SBREG((sbh), (r), ((SOC_R_SBREG(sbh, r) & ~(mask)) | (val)));
#define	REGS2SB(va)	(sbconfig_t*) ((uint)(va) + SBCONFIGOFF)

/*
 * Macros to read/write sbconfig registers. On the PCMCIA core rev 0
 * we need to treat them differently from other registers. See PR 3863.
 */
#define	SOC_R_SBREG(sbh, sbr)	R_REG(SB_SOC_INFO(sbh)->osh, sbr)
#define	SOC_W_SBREG(sbh, sbr, v)	W_REG(SB_SOC_INFO(sbh)->osh, sbr, v)
#define	SOC_AND_SBREG(sbh, sbr, v)	SOC_W_SBREG((sbh), (sbr), (SOC_R_SBREG(sbh, sbr) & (v)))
#define	SOC_OR_SBREG(sbh, sbr, v)	SOC_W_SBREG((sbh), (sbr), (SOC_R_SBREG(sbh, sbr) | (v)))

/*
 * Allocate a sb handle.
 * devid - pci device id (used to determine chip#)
 * osh - opaque OS handle
 * regs - virtual address of initial core registers
 * bustype - pci/pcmcia/sb/etc
 * vars - pointer to a pointer area for "environment" variables
 * varsz - pointer to int to return the size of the vars
 */
void*
sb_soc_attach(uint devid, void *osh, void *regs, uint bustype, char **vars, 
    int *varsz)
{
    return (sb_soc_doattach(devid, osh, regs, bustype));
}

static void*
sb_soc_doattach(uint devid, void *osh, void *regs, uint bustype)
{
    sb_soc_info_t *si;
    uint32 w;

    /* alloc sb_soc_info_t */
    if ((si = ET_MALLOC(sizeof (sb_soc_info_t))) == NULL) {
    	SB_SOC_ERROR(("sb_attach: malloc failed!\n"));
    	return (NULL);
    }
    bzero((uchar*)si, sizeof (sb_soc_info_t));

    si->osh = osh;
    si->origregs = si->curmap = regs;

    /* check to see if we are a sb core mimic'ing a pci core */
    si->bus = bustype;
    if (si->bus == PCI_BUS) {
        if (OSL_ROBO_PCI_READ_CONFIG(osh, PCI_SPROM_CONTROL, sizeof (uint32)) 
            == 0xffffffff)
            si->bus = SB_BUS;
        else
            si->bus = PCI_BUS;
    }

    /* clear any previous epidiag-induced target abort */
    sb_soc_taclear((void*)si);

    si->chip = BCM4710_DEVICE_ID;

    si->chiprev = 0;

    /* get boardtype and boardrev */
    switch (si->bus) {
    case PCI_BUS:
        /* do a pci config read to get subsystem id and subvendor id */
        w = OSL_ROBO_PCI_READ_CONFIG(osh, PCI_CFG_SVID, sizeof (uint32));
        si->boardvendor = w & 0xffff;
        si->boardtype = (w >> 16) & 0xffff;
        break;

    case SB_BUS:
    si->boardvendor = VENDOR_BROADCOM_ID;

    /* boardtype format is a hex string */
    si->boardtype = BCM95380RR_BOARD; 
    break;
    }

    return ((void*)si);
}

uint
sb_soc_coreid(void *sbh)
{
    sb_soc_info_t *si;
    sbconfig_t *sb;

    si = SB_SOC_INFO(sbh);
    sb = REGS2SB(si->curmap);

    return ((SOC_R_SBREG(sbh, &(sb)->sbidhigh) & SBIDH_CC_MASK) >> SBIDH_CC_SHIFT);
}

uint
sb_soc_corevendor(void *sbh)
{
    sb_soc_info_t *si;
    sbconfig_t *sb;

    si = SB_SOC_INFO(sbh);
    sb = REGS2SB(si->curmap);

    return ((SOC_R_SBREG(sbh, &(sb)->sbidhigh) & SBIDH_VC_MASK) >> SBIDH_VC_SHIFT);
}

uint
sb_soc_corerev(void *sbh)
{
    sb_soc_info_t *si;
    sbconfig_t *sb;

    si = SB_SOC_INFO(sbh);
    sb = REGS2SB(si->curmap);

    return (SOC_R_SBREG(sbh, &(sb)->sbidhigh) & SBIDH_RC_MASK);
}

#define	SBTML_ALLOW	(SBTML_PE | SBTML_FGC | SBTML_FL_MASK)

/* set/clear sbtmstatelow core-specific flags */
uint32
sb_soc_coreflags(void *sbh, uint32 mask, uint32 val)
{
    sb_soc_info_t *si;
    sbconfig_t *sb;
    uint32 w;

    si = SB_SOC_INFO(sbh);
    sb = REGS2SB(si->curmap);

    ASSERT((val & ~mask) == 0);
    ASSERT((mask & ~SBTML_ALLOW) == 0);

    /* mask and set */
    if (mask || val) {
        w = (SOC_R_SBREG(sbh, &sb->sbtmstatelow) & ~mask) | val;
        SOC_W_SBREG(sbh, &sb->sbtmstatelow, w);
    }

    /* return the new value */
    return (SOC_R_SBREG(sbh, &sb->sbtmstatelow) & SBTML_ALLOW);
}

/* set/clear sbtmstatehigh core-specific flags */
uint32
sb_soc_coreflagshi(void *sbh, uint32 mask, uint32 val)
{
    sb_soc_info_t *si;
    sbconfig_t *sb;
    uint32 w;

    si = SB_SOC_INFO(sbh);
    sb = REGS2SB(si->curmap);

    ASSERT((val & ~mask) == 0);
    ASSERT((mask & ~SBTMH_FL_MASK) == 0);

    /* mask and set */
    if (mask || val) {
        w = (SOC_R_SBREG(sbh, &sb->sbtmstatehigh) & ~mask) | val;
        SOC_W_SBREG(sbh, &sb->sbtmstatehigh, w);
    }

    /* return the new value */
    return (SOC_R_SBREG(sbh, &sb->sbtmstatehigh) & SBTMH_FL_MASK);
}

bool
sb_soc_iscoreup(void *sbh)
{
    sb_soc_info_t *si;
    sbconfig_t *sb;

    si = SB_SOC_INFO(sbh);
    sb = REGS2SB(si->curmap);

    return ((SOC_R_SBREG(sbh, &(sb)->sbtmstatelow) 
        & (SBTML_RESET | SBTML_REJ | SBTML_CLK)) == SBTML_CLK);
}

/* may be called with core in reset */
void
sb_soc_detach(void *sbh)
{
    sb_soc_info_t *si;

    si = SB_SOC_INFO(sbh);

    if (si == NULL)
        return;

    ET_MFREE(si, sizeof (sb_soc_info_t));
}

/* return chip number */
uint
sb_soc_chip(void *sbh)
{
    sb_soc_info_t *si;

    si = SB_SOC_INFO(sbh);
    return (si->chip);
}

/* return chip revision number */
uint
sb_soc_chiprev(void *sbh)
{
    sb_soc_info_t *si;

    si = SB_SOC_INFO(sbh);
    return (si->chiprev);
}

/* return board vendor id */
uint
sb_soc_boardvendor(void *sbh)
{
    sb_soc_info_t *si;

    si = SB_SOC_INFO(sbh);
    return (si->boardvendor);
}

/* return boardtype */
uint
sb_soc_boardtype(void *sbh)
{
    sb_soc_info_t *si;

    si = SB_SOC_INFO(sbh);
    return (si->boardtype);
}

/* return board bus style */
uint
sb_soc_boardstyle(void *sbh)
{
    sb_soc_info_t *si;

    si = SB_SOC_INFO(sbh);

    if (si->bus == SB_BUS)
        return (BOARDSTYLE_SOC);

    /* bus is PCI */
    return (BOARDSTYLE_PCI);
}

/* return boolean if sbh device is in pci hostmode or client mode */
uint
sb_soc_bus(void *sbh)
{
    sb_soc_info_t *si;

    si = SB_SOC_INFO(sbh);
    return (si->bus);
}

/* Check if a target abort has happened and clear it */
bool
sb_soc_taclear(void *sbh)
{
    sb_soc_info_t *si;
    bool rc = FALSE;

    si = SB_SOC_INFO(sbh);

    if (si->bus == PCI_BUS) {
        uint32 stcmd;

        stcmd = OSL_ROBO_PCI_READ_CONFIG(si->osh, PCI_CFG_CMD, sizeof(stcmd));
        rc = (stcmd & 0x08000000) != 0;

        if (rc) {
            /* Target abort bit is set, clear it */
            OSL_ROBO_PCI_WRITE_CONFIG(si->osh, PCI_CFG_CMD, sizeof(stcmd), stcmd);
        }
    }
    /* XXX: Other buses?? */

    return (rc);
}


/* reset and re-enable a core */
void
sb_soc_core_reset(void *sbh, uint32 bits)
{
    sb_soc_info_t *si;
    sbconfig_t *sb;
    volatile uint32 dummy;

    si = SB_SOC_INFO(sbh);
    sb = REGS2SB(si->curmap);

    /*
     * Must do the disable sequence first to work for 
     * arbitrary current core state.
     */
    sb_soc_core_disable(sbh, bits);

    /*
     * Now do the initialization sequence.
     */

    /* set reset while enabling the clock and */
    /* forcing them on throughout the core */
    SOC_W_SBREG(sbh, &sb->sbtmstatelow, 
        (SBTML_FGC | SBTML_CLK | SBTML_RESET | bits));
    dummy = SOC_R_SBREG(sbh, &sb->sbtmstatelow);

    OSL_DELAY(1);

    /* PR3158 - clear any serror */
    if (SOC_R_SBREG(sbh, &sb->sbtmstatehigh) & SBTMH_SERR) {
        SOC_W_SBREG(sbh, &sb->sbtmstatehigh, 0);
    }
    if ((dummy = SOC_R_SBREG(sbh, &sb->sbimstate)) & (SBIM_IBE | SBIM_TO)) {
        SOC_AND_SBREG(sbh, &sb->sbimstate, ~(SBIM_IBE | SBIM_TO));
    }

    /* clear reset and allow it to propagate throughout the core */
    SOC_W_SBREG(sbh, &sb->sbtmstatelow, (SBTML_FGC | SBTML_CLK | bits));
    dummy = SOC_R_SBREG(sbh, &sb->sbtmstatelow);
    OSL_DELAY(1);

    /* leave clock enabled */
    SOC_W_SBREG(sbh, &sb->sbtmstatelow, (SBTML_CLK | bits));
    dummy = SOC_R_SBREG(sbh, &sb->sbtmstatelow);
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
sb_soc_core_toadjust(void *sbh)
{
    sb_soc_info_t *si;
    sbconfig_t *sb;

    si = SB_SOC_INFO(sbh);
    sb = REGS2SB(si->curmap);

    /* PR 9962/4708: Set initiator timeouts. */
    if (si->bus == SB_BUS) {
        SOC_SET_SBREG(sbh, &sb->sbimconfiglow,
            SBIMCL_RTO_MASK | SBIMCL_STO_MASK,
            (0x5 << SBIMCL_RTO_SHIFT) | 0x3);
    } else {
        if (sb_soc_coreid(sbh) == SB_PCI) {
            SOC_SET_SBREG(sbh, &sb->sbimconfiglow,
                SBIMCL_RTO_MASK | SBIMCL_STO_MASK,
                (0x3 << SBIMCL_RTO_SHIFT) | 0x2);
        } else {
            SOC_SET_SBREG(sbh, &sb->sbimconfiglow, 
                (SBIMCL_RTO_MASK | SBIMCL_STO_MASK), 0);
        }
    }

}

void
sb_soc_core_disable(void *sbh, uint32 bits)
{
    sb_soc_info_t *si;
    sbconfig_t *sb;

    si = SB_SOC_INFO(sbh);

    sb = REGS2SB(si->curmap);

    /* must return if core is already in reset */
    if (SOC_R_SBREG(sbh, &sb->sbtmstatelow) & SBTML_RESET)
        return;

    /* put into reset and return if clocks are not enabled */
    if ((SOC_R_SBREG(sbh, &sb->sbtmstatelow) & SBTML_CLK) == 0)
        goto disable;

    /* set the reject bit */
    SOC_W_SBREG(sbh, &sb->sbtmstatelow, (SBTML_CLK | SBTML_REJ));

    /* spin until reject is set */
    while ((SOC_R_SBREG(sbh, &sb->sbtmstatelow) & SBTML_REJ) == 0)
        OSL_DELAY(1);

    /* spin until sbtmstatehigh.busy is clear */
    while (SOC_R_SBREG(sbh, &sb->sbtmstatehigh) & SBTMH_BUSY)
        OSL_DELAY(1);

    /* set reset and reject while enabling the clocks */
    SOC_W_SBREG(sbh, &sb->sbtmstatelow, 
        (bits | SBTML_FGC | SBTML_CLK | SBTML_REJ | SBTML_RESET));
    (void)SOC_R_SBREG(sbh, &sb->sbtmstatelow);
    OSL_DELAY(10);

 disable:
    /* leave reset and reject asserted */
    SOC_W_SBREG(sbh, &sb->sbtmstatelow, (bits | SBTML_REJ | SBTML_RESET));
    OSL_DELAY(1);
}


