/*
 * $Id: aiutils.c,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifdef __KERNEL__
#include <linux-bde.h>
#endif
#include <typedefs.h>
#include <shared/et/linux_osl.h>
#include <shared/et/pcicfg.h>
#include <shared/et/bcmdevs.h>
#include <shared/et/bcmendian.h>
#include <shared/et/bcmutils.h>
#include <shared/et/bcmdevs.h>
#include <shared/et/aiutils.h>
#include <shared/et/aiutils_priv.h>
#include <shared/et/sbchipc.h>
#include <shared/et/aidmp.h>


/* debug/trace */
#ifdef BCMDBG_ERR
#define SI_ERROR(args)  printf args
#else
#define SI_ERROR(args)
#endif  /* BCMDBG_ERR */


#ifdef BCMDBG
#define SI_MSG(args)    printf args
#else
#define SI_MSG(args)      
#endif  /* BCMDBG */

#define SI_ENUM_BASE        0x18000000  /* Enumeration space base */
#define SI_CORE_SIZE        0x1000      /* each core gets 4Kbytes for registers */

#define SI_CC_IDX       0
/* SOC Interconnect types (aka chip types) */
#define SOCI_SB         0
#define SOCI_AI         1
#define SOCI_NS         0x3 /* CHECKME: Northstr chip type */

#define SICF_FGC            0x0002
#define SICF_CLOCK_EN       0x0001

static si_info_t *
ai_soc_doattach(uint devid, void *osh, void *regs,
                       uint bustype, void *sdh, char **vars, uint *varsz);

void *ai_soc_osh(si_t *sih);


/* EROM parsing */
/* define the ChipCommon and GMAC cores descriptors of NS+ */ 
static uint32  nsp_erom[] = {
    /* ChipCommon */
    0x4bf80001, 0x2a004201, 0x18000005, 0x181300c5,
    /* GMAC cores, uint 0 ~ 3 */
    0x4bf82d01, 0x13004211, 0x00000103, 0x18022005, 0x181100c5,
    0x4bf82d01, 0x13004211, 0x00000203, 0x18023005, 0x181110c5, 
    0x4bf82d01, 0x13004211, 0x00000303, 0x18024005, 0x181120c5, 
    0x4bf82d01, 0x13004211, 0x00000403, 0x18025005, 0x181130c5,
    /* END */
    0x0000000f
    
};


static uint32
get_erom_ent(si_t *sih, uint32 *eromptr, uint32 mask, uint32 match)
{
    uint32 ent;
    uint inv = 0, nom = 0;

    while (TRUE) {
        ent = R_REG(ai_soc_osh(sih), (uint32 *)(uint*)(*eromptr));
        *eromptr += sizeof(uint32);

        if (mask == 0)
            break;

        if ((ent & ER_VALID) == 0) {
            inv++;
            continue;
        }

        if (ent == (ER_END | ER_VALID))
            break;

        if ((ent & mask) == match)
            break;

        nom++;
    }

    SI_MSG(("%s: Returning ent 0x%08x, mask = 0x%x, match = 0x%x\n", 
        FUNCTION_NAME(), ent, mask, match));
    if (inv + nom)
        SI_MSG(("  after %d invalid and %d non-matching entries\n", inv, nom));
    return ent;
}

static uint32
get_asd(si_t *sih, uint32 *eromptr, uint sp, uint ad, uint st, uint32 *addrl, uint32 *addrh,
        uint32 *sizel, uint32 *sizeh)
{
    uint32 asd, sz, szd;

    asd = get_erom_ent(sih, eromptr, ER_VALID, ER_VALID);
    if (((asd & ER_TAG1) != ER_ADD) ||
        (((asd & AD_SP_MASK) >> AD_SP_SHIFT) != sp) ||
        ((asd & AD_ST_MASK) != st)) {
        /* This is not what we want, "push" it back */
        *eromptr -= sizeof(uint32);
        return 0;
    }
    *addrl = asd & AD_ADDR_MASK;
    if (asd & AD_AG32)
        *addrh = get_erom_ent(sih, eromptr, 0, 0);
    else
        *addrh = 0;
    *sizeh = 0;
    sz = asd & AD_SZ_MASK;
    if (sz == AD_SZ_SZD) {
        szd = get_erom_ent(sih, eromptr, 0, 0);
        *sizel = szd & SD_SZ_MASK;
        if (szd & SD_SG32)
            *sizeh = get_erom_ent(sih, eromptr, 0, 0);
    } else
        *sizel = AD_SZ_BASE << (sz >> AD_SZ_SHIFT);

    SI_MSG(("  SP %d, ad %d: st = %d, 0x%08x_0x%08x @ 0x%08x_0x%08x\n",
            sp, ad, st, *sizeh, *sizel, *addrh, *addrl));

    return asd;
}


/* parse the enumeration rom to identify all cores */
static void
ai_soc_scan(si_t *sih, void *regs, uint devid)
{
    si_info_t *sii = SI_INFO(sih);
    chipcregs_t *cc = (chipcregs_t *)REG_MAP(SI_ENUM_BASE, SI_CORE_SIZE);
    uint32 erombase, eromptr, eromlim;
    erombase = R_REG(sii->osh, &cc->eromptr);

    switch (BUSTYPE(sih->bustype)) {
    case SI_BUS:
        if (CHIPID(sih->chip) == BCM53020_CHIP_ID) {
            eromptr = (uint32)nsp_erom;
        } else {
            eromptr = (uint32)REG_MAP(erombase, SI_CORE_SIZE);
        }
        break;

    case PCMCIA_BUS:
    default:
        SI_ERROR(("Don't know how to do AXI enumertion on bus %d\n", sih->bustype));
        ASSERT(0);
        return;
    }
    eromlim = eromptr + ER_REMAPCONTROL;

    SI_MSG(("ai_scan: regs = 0x%p, erombase = 0x%08x, eromptr = 0x%08x, eromlim = 0x%08x\n",
            regs, erombase, eromptr, eromlim));
    while (eromptr < eromlim) {
        uint32 cia, cib, base, cid, mfg, crev, nmw, nsw, nmp, nsp;
        uint32 mpd, asd, addrl, addrh, sizel, sizeh;
        uint i, j, idx;
        bool br;

        COMPILER_REFERENCE(base);
        COMPILER_REFERENCE(crev);

        br = FALSE;

        /* Grok a component */
        cia = get_erom_ent(sih, &eromptr, ER_TAG, ER_CI);
        if (cia == (ER_END | ER_VALID)) {
            SI_MSG(("Found END of erom after %d cores\n", sii->numcores));
            return;
        }
        base = eromptr - sizeof(uint32);
        cib = get_erom_ent(sih, &eromptr, 0, 0);

        if ((cib & ER_TAG) != ER_CI) {
            SI_ERROR(("CIA not followed by CIB\n"));
            goto error;
        }

        cid = (cia & CIA_CID_MASK) >> CIA_CID_SHIFT;
        mfg = (cia & CIA_MFG_MASK) >> CIA_MFG_SHIFT;
        crev = (cib & CIB_REV_MASK) >> CIB_REV_SHIFT;
        nmw = (cib & CIB_NMW_MASK) >> CIB_NMW_SHIFT;
        nsw = (cib & CIB_NSW_MASK) >> CIB_NSW_SHIFT;
        nmp = (cib & CIB_NMP_MASK) >> CIB_NMP_SHIFT;
        nsp = (cib & CIB_NSP_MASK) >> CIB_NSP_SHIFT;

        SI_MSG(("Found component 0x%04x/0x%4x rev %d at erom addr 0x%08x, with nmw = %d, "
                "nsw = %d, nmp = %d & nsp = %d\n",
                mfg, cid, crev, base, nmw, nsw, nmp, nsp));

        if (cid != GMAC_COM_CORE_ID) {
            if (((mfg == MFGID_ARM) && (cid == DEF_AI_COMP)) ||
                (nmw + nsw == 0) || (nsp == 0)) {
                /* A component which is not a core */
                /* XXX: Should record some info */
                continue;
            }
        }

        idx = sii->numcores;
        sii->cia[idx] = cia;
        sii->cib[idx] = cib;
        sii->coreid[idx] = cid;

        for (i = 0; i < nmp; i++) {
            mpd = get_erom_ent(sih, &eromptr, ER_VALID, ER_VALID);
            if ((mpd & ER_TAG) != ER_MP) {
                SI_ERROR(("Not enough MP entries for component 0x%x\n", cid));
                goto error;
            }
            /* XXX: Record something? */
            SI_MSG(("  Master port %d, mp: %d id: %d\n", i,
                    (mpd & MPD_MP_MASK) >> MPD_MP_SHIFT,
                    (mpd & MPD_MUI_MASK) >> MPD_MUI_SHIFT));
        }

        /* First Slave Address Descriptor should be port 0:
         * the main register space for the core
         */
        asd = get_asd(sih, &eromptr, 0, 0, AD_ST_SLAVE, &addrl, &addrh, &sizel, &sizeh);
        if (asd == 0) {
            /* Try again to see if it is a bridge */
            asd = get_asd(sih, &eromptr, 0, 0, AD_ST_BRIDGE, &addrl, &addrh,
                          &sizel, &sizeh);
            if (asd != 0)
                br = TRUE;
            else
                if ((addrh != 0) || (sizeh != 0) || (sizel != SI_CORE_SIZE)) {
                    /* XXX: Could we have sizel != 4KB? */
                    SI_ERROR(("First Slave ASD for core 0x%04x malformed "
                              "(0x%08x)\n", cid, asd));
                    goto error;
                }
        }
        sii->coresba[idx] = addrl;
        sii->coresba_size[idx] = sizel;
        /* Get any more ASDs in port 0 */
        j = 1;
        do {
            asd = get_asd(sih, &eromptr, 0, j, AD_ST_SLAVE, &addrl, &addrh,
                          &sizel, &sizeh);
            if ((asd != 0) && (j == 1) && (sizel == SI_CORE_SIZE))
                sii->coresba2[idx] = addrl;
                sii->coresba2_size[idx] = sizel;
            j++;
        } while (asd != 0);

        /* Go through the ASDs for other slave ports */
        for (i = 1; i < nsp; i++) {
            j = 0;
            do {
                asd = get_asd(sih, &eromptr, i, j++, AD_ST_SLAVE, &addrl, &addrh,
                              &sizel, &sizeh);
                /* XXX: Should record them so we can do error recovery later */
            } while (asd != 0);
            if (j == 0) {
                SI_ERROR((" SP %d has no address descriptors\n", i));
                goto error;
            }
        }

        /* Now get master wrappers */
        for (i = 0; i < nmw; i++) {
            asd = get_asd(sih, &eromptr, i, 0, AD_ST_MWRAP, &addrl, &addrh,
                          &sizel, &sizeh);
            if (asd == 0) {
                SI_ERROR(("Missing descriptor for MW %d\n", i));
                goto error;
            }
            if ((sizeh != 0) || (sizel != SI_CORE_SIZE)) {
                SI_ERROR(("Master wrapper %d is not 4KB\n", i));
                goto error;
            }
            if (i == 0)
                sii->wrapba[idx] = addrl;
        }

        /* And finally slave wrappers */
        for (i = 0; i < nsw; i++) {
            uint fwp = (nsp == 1) ? 0 : 1;
            asd = get_asd(sih, &eromptr, fwp + i, 0, AD_ST_SWRAP, &addrl, &addrh,
                          &sizel, &sizeh);
            if (asd == 0) {
                SI_ERROR(("Missing descriptor for SW %d\n", i));
                goto error;
            }
            if ((sizeh != 0) || (sizel != SI_CORE_SIZE)) {
                SI_ERROR(("Slave wrapper %d is not 4KB\n", i));
                goto error;
            }
            if ((nmw == 0) && (i == 0))
                sii->wrapba[idx] = addrl;
        }

        if (CHIPID(sih->chip) == BCM53000_CHIP_ID) {
            /* Check if it's a low cost package */
            i = (R_REG(sii->osh, &cc->chipid) & CID_PKG_MASK) >> CID_PKG_SHIFT;
            if (i == 1) {
                /* BCM53001: only one GMAC */
                if (cid == GMAC_CORE_ID) {
                    for(j=0; j<sii->numcores; j++) {
                        if (sii->coreid[j] == GMAC_CORE_ID) {
                            break;
                        }
                    }
                    if (j != sii->numcores) {
                        /* Found one GMAC already, ignore this one */
                        continue;
                    }
                }
            } else if (i == 2) {
                /* BCM53002: only one PCIE */
                if (cid == PCIE_CORE_ID) {
                    for(j=0; j<sii->numcores; j++) {
                        if (sii->coreid[j] == PCIE_CORE_ID) {
                            break;
                        }
                    }
                    if (j != sii->numcores) {
                        /* Found one PCIE already, ignore this one */
                        continue;
                    }
                }
            }
        }

        if ((CHIPID(sih->chip) == BCM53010_CHIP_ID) || \
            (CHIPID(sih->chip) == BCM53018_CHIP_ID)) {
            /* Check if it's a low cost package */
            i = (R_REG(sii->osh, &cc->chipid) & CID_PKG_MASK) >> CID_PKG_SHIFT;
            /* 
             * Add necessary code for Northstar here.
             * For example, low cost package may have to skip i
             * some cores that does not exist.
             */
        }

        /* Don't record bridges */
        if (br)
            continue;

        /* Done with core */
        sii->numcores++;
    }

    SI_ERROR(("Reached end of erom without finding END"));

error:
    sii->numcores = 0;
    return;
}

/*
 * Allocate a si handle.
 * devid - pci device id (used to determine chip#)
 * osh - opaque OS handle
 * regs - virtual address of initial core registers
 * bustype - pci/pcmcia/sb/sdio/etc
 * vars - pointer to a pointer area for "environment" variables
 * varsz - pointer to int to return the size of the vars
 */
si_t *
ai_soc_attach(uint devid, void *osh, void *regs,
                       uint bustype, void *sdh, char **vars, uint *varsz)
{
    si_info_t *sii;  

    if ((sii = ai_soc_doattach(devid, osh, regs, bustype, sdh, vars, varsz)) == NULL) {
        MFREE(osh, sii, sizeof(si_info_t));
        return (NULL);
    }
    sii->vars = vars ? *vars : NULL;
    sii->varsz = varsz ? *varsz : 0;

    return (si_t *)sii;
}


si_t *
ai_soc_kattach(void *osh)
{
    si_info_t *sii;
    char *unused;
    uint varsz;
#if defined(KEYSTONE)	
    sii = ai_soc_doattach(BCM53000_GMAC_ID, osh, 
                        (void *)REG_MAP(SI_ENUM_BASE, SI_CORE_SIZE),
                        SI_BUS, NULL,
                        &unused,
                        &varsz);
#endif

#if defined(IPROC_CMICD)	
    sii = ai_soc_doattach(BCM53010_GMAC_ID, osh, 
                        (void *)REG_MAP(SI_ENUM_BASE, SI_CORE_SIZE),
                        SI_BUS, NULL,
                        &unused,
                        &varsz);
#endif

    return (si_t *)sii;
}


static si_info_t *
ai_soc_doattach(uint devid, void *osh, void *regs,
                       uint bustype, void *sdh, char **vars, uint *varsz)
{
    struct si_pub *sih = NULL;
    uint32 w;
    chipcregs_t *cc;
    si_info_t *sii;

    /* alloc si_info_t */
    if ((sii = MALLOC(osh, sizeof (si_info_t))) == NULL) {
        SI_ERROR(("si_attach: malloc failed!\n"));
        return (NULL);
    }
    sih = &sii->pub;

    ASSERT(GOODREGS(regs));

    bzero((uchar*)sii, sizeof(si_info_t));

    sih->buscoreidx = BADIDX;

    sii->curmap = regs;
    sii->sdh = sdh;
    sii->osh = osh;

    /* find Chipcommon address */
    cc = (chipcregs_t *)REG_MAP(SI_ENUM_BASE, SI_CORE_SIZE);

    sih->bustype = bustype;

    /* ChipID recognition.
     *   We assume we can read chipid at offset 0 from the regs arg.
     *   If we add other chiptypes (or if we need to support old sdio hosts w/o chipcommon),
     *   some way of recognizing them needs to be added here.
     */
    w = R_REG(osh, &cc->chipid);
    sih->socitype = (w & CID_TYPE_MASK) >> CID_TYPE_SHIFT;
    /* Might as wll fill in chip id rev & pkg */
    sih->chip = w & CID_ID_MASK;
    sih->chiprev = (w & CID_REV_MASK) >> CID_REV_SHIFT;
    sih->chippkg = (w & CID_PKG_MASK) >> CID_PKG_SHIFT;

    /* scan for cores */
    if (sii->pub.socitype == SOCI_SB) {
        SI_MSG(("Found chip type SB (0x%08x)\n", w));
        /* sb_scan(&sii->pub, regs, devid); */
    } else if ((sii->pub.socitype == SOCI_AI) || (sii->pub.socitype == SOCI_NS)) {
        SI_MSG(("Found chip type AI (0x%08x)\n", w));
        /* Replace regs as cc */
        ai_soc_scan(&sii->pub, regs, devid);
    } else {
        SI_ERROR(("Found chip of unkown type (0x%08x)\n", w));
        return NULL;
    }
    /* no cores found, bail out */
    if (sii->numcores == 0) {
        SI_ERROR(("si_doattach: could not find any cores\n"));
        return NULL;
    }
    
    return (sii);
}

/* may be called with core in reset */
void
ai_soc_detach(si_t *sih)
{
    si_info_t *sii;
    uint idx;

    sii = SI_INFO(sih);

    if (sii == NULL)
        return;

    if (BUSTYPE(sih->bustype) == SI_BUS)
        for (idx = 0; idx < SI_MAXCORES; idx++)
            if (sii->regs[idx]) {
                REG_UNMAP(sii->regs[idx]);
                sii->regs[idx] = NULL;
            }

    MFREE(sii->osh, sii, sizeof(si_info_t));
}

void *
ai_soc_osh(si_t *sih)
{
    si_info_t *sii;

    sii = SI_INFO(sih);
    return sii->osh;
}

unsigned int
ai_soc_clock(si_t *sih)
{
    chipcregs_t *cc;
    uint32  w, cpu_freq;
    
    /* The function will be called before si_attach() */
    cc = (chipcregs_t *)REG_MAP(SI_ENUM_BASE, SI_CORE_SIZE);
    w = R_REG(NULL, &cc->chipstatus);
    if (w & CHIPSTAT_PO_MASK) {
        return (400000000 / 4);
    } else {
#if defined(QUICK_TURN)
        return (60000000); /* shorten the delay */
#else
        /* Get N divider to determine CPU clock */
        W_REG(NULL, &cc->pllcontrol_addr, 4);
        w = (R_REG(NULL, &cc->pllcontrol_data) & 0x00000fff) >> 3;
        
        /* Fixed reference clock 25MHz and m = 2 */
        cpu_freq = w * 25000000 / 2;
        cpu_freq = cpu_freq /4;
        return cpu_freq;
#endif
    }
}


uint
ai_soc_coreid(si_t *sih)
{
    si_info_t *sii;

    sii = SI_INFO(sih);
    return sii->coreid[sii->curidx];
}

uint
ai_soc_coreidx(si_t *sih)
{
    si_info_t *sii;

    sii = SI_INFO(sih);
    return sii->curidx;
}

/* return the core-type instantiation # of the current core */
uint
ai_soc_coreunit(si_t *sih)
{
    si_info_t *sii;
    uint idx;
    uint coreid;
    uint coreunit;
    uint i;

    sii = SI_INFO(sih);
    coreunit = 0;

    idx = sii->curidx;

    ASSERT(GOODREGS(sii->curmap));
    coreid = ai_soc_coreid(sih);

    /* count the cores of our type */
    for (i = 0; i < idx; i++)
        if (sii->coreid[i] == coreid)
            coreunit++;

    return (coreunit);
}


uint
ai_soc_corerev(si_t *sih)
{
    si_info_t *sii;
    uint32 cib;

    sii = SI_INFO(sih);
    cib = sii->cib[sii->curidx];
    return ((cib & CIB_REV_MASK) >> CIB_REV_SHIFT);
}

/* return index of coreid or BADIDX if not found */
uint
ai_soc_findcoreidx(si_t *sih, uint coreid, uint coreunit)
{
    si_info_t *sii;
    uint found;
    uint i;

    sii = SI_INFO(sih);

    found = 0;

    for (i = 0; i < sii->numcores; i++) {
        if (sii->coreid[i] == coreid) {
            if (found == coreunit)
                return (i);
            found++;
        }
    }

    return (BADIDX);
}


/*
 * This function changes logical "focus" to the indicated core;
 * must be called with interrupts off.
 * Moreover, callers should keep interrupts off during switching out of and back to d11 core
 */
void *
ai_soc_setcore(si_t *sih, uint coreid, uint coreunit)
{
    uint idx;

    idx = ai_soc_findcoreidx(sih, coreid, coreunit);
    if (!GOODIDX(idx))
        return (NULL);

    return ai_soc_setcoreidx(sih, idx);
}

void *
ai_soc_setcoreidx(si_t *sih, uint coreidx)
{
    si_info_t *sii = SI_INFO(sih);
    uint32 addr = sii->coresba[coreidx];
    uint32 wrap = sii->wrapba[coreidx];
    void *regs;

    if (coreidx >= sii->numcores)
        return (NULL);

    /*
     * If the user has provided an interrupt mask enabled function,
     * then assert interrupts are disabled before switching the core.
     */
    ASSERT((sii->intrsenabled_fn == NULL) || !(*(sii)->intrsenabled_fn)((sii)->intr_arg));

    switch (BUSTYPE(sih->bustype)) {
    case SI_BUS:
        /* map new one */
        if (!sii->regs[coreidx]) {
            sii->regs[coreidx] = (void*)REG_MAP(addr, SI_CORE_SIZE);
            ASSERT(GOODREGS(sii->regs[coreidx]));
        }
        sii->curmap = regs = sii->regs[coreidx];
        if (!sii->wrappers[coreidx]) {
            sii->wrappers[coreidx] = (void*)REG_MAP(wrap, SI_CORE_SIZE);
            ASSERT(GOODREGS(sii->wrappers[coreidx]));
        }
        sii->curwrap = sii->wrappers[coreidx];
        break;     
        
    default:
        ASSERT(0);
        regs = NULL;
        break;
    }

    sii->curmap = regs;
    sii->curidx = coreidx;

    return regs;
}

bool
ai_soc_iscoreup(si_t *sih)
{
    si_info_t *sii;
    aidmp_t *ai;

    sii = SI_INFO(sih);
    ai = sii->curwrap;

    return (((R_REG(sii->osh, &ai->ioctrl) & (SICF_FGC | SICF_CLOCK_EN)) == SICF_CLOCK_EN) &&
            ((R_REG(sii->osh, &ai->resetctrl) & AIRC_RESET) == 0));
}

uint
ai_soc_corereg(si_t *sih, uint coreidx, uint regoff, uint mask, uint val)
{
    uint origidx = 0;
    uint32 *r = NULL;
    uint w;
    uint intr_val = 0;
    bool fast = FALSE;
    si_info_t *sii;

    sii = SI_INFO(sih);

    ASSERT(GOODIDX(coreidx));
    ASSERT(regoff < SI_CORE_SIZE);
    ASSERT((val & ~mask) == 0);

    if (BUSTYPE(sih->bustype) == SI_BUS) {
        /* If internal bus, we can always get at everything */
        fast = TRUE;
        /* map if does not exist */
        if (!sii->wrappers[coreidx]) {
            sii->regs[coreidx] = (void *)REG_MAP(sii->coresba[coreidx],
                                        SI_CORE_SIZE);
            ASSERT(GOODREGS(sii->regs[coreidx]));
        }
        r = (uint32 *)((uchar *)sii->regs[coreidx] + regoff);
    }

    if (!fast) {
        INTR_OFF(sii, intr_val);

        /* save current core index */
        origidx = ai_soc_coreidx(&sii->pub);

        /* switch core */
        r = (uint32*) ((uchar*) ai_soc_setcoreidx(&sii->pub, coreidx) + regoff);
    }
    ASSERT(r != NULL);

    /* mask and set */
    if (mask || val) {
        w = (R_REG(sii->osh, r) & ~mask) | val;
        W_REG(sii->osh, r, w);
    }

    /* readback */
    w = R_REG(sii->osh, r);

    if (!fast) {
        /* restore core index */
        if (origidx != coreidx)
            ai_soc_setcoreidx(&sii->pub, origidx);

        INTR_RESTORE(sii, intr_val);
    }

    return (w);
}



void
ai_soc_core_disable(si_t *sih, uint32 bits)
{
    si_info_t *sii;
    aidmp_t *ai;

    sii = SI_INFO(sih);

    ASSERT(GOODREGS(sii->curwrap));
    ai = sii->curwrap;

    /* if core is already in reset, just return */
    if (R_REG(sii->osh, &ai->resetctrl) & AIRC_RESET)
        return;

    W_REG(sii->osh, &ai->ioctrl, bits);
    (void)R_REG(sii->osh, &ai->ioctrl);
    OSL_DELAY(10);

    W_REG(sii->osh, &ai->resetctrl, AIRC_RESET);
    OSL_DELAY(1);
}

void
ai_soc_core_reset(si_t *sih, uint32 bits, uint32 resetbits)
{
    si_info_t *sii;
    aidmp_t *ai;

    sii = SI_INFO(sih);
    ASSERT(GOODREGS(sii->curwrap));
    ai = sii->curwrap;

    /*
     * Must do the disable sequence first to work for arbitrary current core state.
     */
    ai_soc_core_disable(sih, (bits | resetbits));

    /*
     * Now do the initialization sequence.
     */
    W_REG(sii->osh, &ai->ioctrl, (bits | SICF_FGC | SICF_CLOCK_EN));
    (void)R_REG(sii->osh, &ai->ioctrl);
    W_REG(sii->osh, &ai->resetctrl, 0);
    OSL_DELAY(1);

    W_REG(sii->osh, &ai->ioctrl, (bits | SICF_CLOCK_EN));
    (void)R_REG(sii->osh, &ai->ioctrl);
    OSL_DELAY(1);

}

/* enable the SPI I/O port */
int
ai_soc_spi_select(si_t *sih, uint8 spi_id, int en)
{
    si_info_t *sii;
    uint    origidx;
    uint    intr_val = 0;
    uint32  reg_val = 0, temp;
    chipcregs_t *cc;
    
    /* select the I2C port */
    if (!CC_SPI_ID_IS_VALID(spi_id)){
        SI_ERROR(("%s: invalid spi_id\n", FUNCTION_NAME()));
        return -1;
    }
    
    sii = SI_INFO(sih);
    INTR_OFF(sii, intr_val);
    origidx = ai_soc_coreidx(sih);

    cc = (chipcregs_t *)ai_soc_setcoreidx(sih, SI_CC_IDX);
    ASSERT(cc);

    reg_val = R_REG(sii->osh, &cc->SERIAL_IO_SEL);
    temp = (spi_id == CC_SPI_SS0) ? CC_SERIAL_IO_SPI0_MASK : 
            (spi_id == CC_SPI_SS1) ? CC_SERIAL_IO_SPI1_MASK : 
            (spi_id == CC_SPI_SS2) ? CC_SERIAL_IO_SPI2_MASK : 0;
    if (en){
        reg_val |= temp;
    } else {
        reg_val &= ~temp;
    }
    W_REG(sii->osh, &cc->SERIAL_IO_SEL, reg_val);

    /* restore the original index */
    ai_soc_setcoreidx(sih, origidx);

    INTR_RESTORE(sii, intr_val);
    return 0;
    
}

/* enable CC lever interrupt on the SPI I/O port */
int
ai_soc_spi_ccint_enable(si_t *sih, bool en)
{
    si_info_t *sii;
    uint    origidx;
    uint    intr_val = 0;
    uint32  reg_val = 0, temp;
    chipcregs_t *cc;
    
    sii = SI_INFO(sih);
    INTR_OFF(sii, intr_val);
    origidx = ai_soc_coreidx(sih);

    cc = (chipcregs_t *)ai_soc_setcoreidx(sih, SI_CC_IDX);
    ASSERT(cc);

    reg_val = R_REG(sii->osh, &cc->SERIAL_IO_INTMASK);

    temp = 0x4;
    if (en){
        reg_val |= temp;
    } else {
        reg_val &= ~temp;
    }

    W_REG(sii->osh, &cc->SERIAL_IO_INTMASK, reg_val);

    /* restore the original index */
    ai_soc_setcoreidx(sih, origidx);

    INTR_RESTORE(sii, intr_val);
    return 0;
}

/* mask&set spi_mode_ctrl bits */
int 
ai_soc_spi_modectrl(si_t *sih, uint32 mask, uint32 val)
{
    uint regoff = 0;

    regoff = OFFSETOF(chipcregs_t, spi_mode_ctrl);
    return (ai_soc_corereg(sih, SI_CC_IDX, regoff, mask, val));
}

/* mask&set spi_mode_ctrl bits */
int 
ai_soc_spi_config(si_t *sih, uint32 mask, uint32 val)
{
    uint regoff = 0;

    regoff = OFFSETOF(chipcregs_t, spi_config);
    return (ai_soc_corereg(sih, SI_CC_IDX, regoff, mask, val));
}

/* mask&set si_spi_fifo bits.
  * Note : do not read original value before writing data for SPI FIFO IO register,
  *           since this will cause abnormal FIFO in/out behavior.
  */
uint32
ai_soc_spi_fifo(si_t *sih, uint32 mask, uint32 val)
{
    si_info_t *sii;
    uint    origidx;
    uint    intr_val = 0;
    uint32 reg_val = 0;
    chipcregs_t *cc;
    
    sii = SI_INFO(sih);
    INTR_OFF(sii, intr_val);
    
    /* save current core index */
    origidx = ai_soc_coreidx(sih);

    cc = (chipcregs_t *)ai_soc_setcoreidx(sih, SI_CC_IDX);
    ASSERT(cc);

    /* mask and set */
    if (mask || val) {
        reg_val = val;
        W_REG(sii->osh, &cc->spi_fifo_io, reg_val);
    } else {
        /* readback */
        reg_val = R_REG(sii->osh, &cc->spi_fifo_io);
    }
    
    /* restore the original index */
    ai_soc_setcoreidx(sih, origidx);
    
    INTR_RESTORE(sii, intr_val);
    
    return reg_val;
}

/* mask&set spi_status bits. 
 *  write (0/1) to clear interrupt flag.
 */
int
ai_soc_spi_intr_clear(si_t *sih)
{
    si_info_t *sii;
    uint    origidx;
    uint    intr_val = 0;
    uint32    reg_val = 0;
    chipcregs_t *cc;
    
    sii = SI_INFO(sih);
    INTR_OFF(sii, intr_val);
    
    /* save current core index */
    origidx = ai_soc_coreidx(sih);

    cc = (chipcregs_t *)ai_soc_setcoreidx(sih, SI_CC_IDX);
    ASSERT(cc);

    reg_val = R_REG(sii->osh, &cc->spi_status);
    /* write (0/1) to clear interrupt flag */
    W_REG(sii->osh, &cc->spi_status, reg_val);
    
    /* restore the original index */
    ai_soc_setcoreidx(sih, origidx);
    
    INTR_RESTORE(sii, intr_val);
    
    return 0;
}

/* mask&set spi_status bits. */
int
ai_soc_spi_status(si_t *sih)
{
    uint regoff = 0; 
    regoff = OFFSETOF(chipcregs_t, spi_status);
    return (ai_soc_corereg(sih, SI_CC_IDX, regoff, 0, 0));
}


