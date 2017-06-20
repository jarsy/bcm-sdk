/*
 * Broadcom Gigabit Ethernet MAC (Unimac) core.
 *
 * This file implements the chip-specific routines for the GMAC core.
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $Id: etcgmac.c,v 1.17 Broadcom SDK $
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <bcmenetphy.h>
#include <et_dbg.h>
#include <proto/ethernet.h>
#include <proto/802.1d.h>
#include <siutils.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <et_dbg.h>
#include <hndsoc.h>
#include <bcmgmacmib.h>
#include <gmac0_core.h>
#include <gmac_common.h>
#include <et_export.h>      /* for et_phyxx() routines */
#include <etcgmac.h>

struct bcmgmac; /* forward declaration */
#define ch_t    struct bcmgmac
#include <etc.h>

/* private chip state */
struct bcmgmac {
    void        *et;        /* pointer to et private state */
    etc_info_t  *etc;       /* pointer to etc public state */

    gmac0regs_t *regs;      /* pointer to chip registers */
    osl_t       *osh;       /* os handle */

    void        *etphy;     /* pointer to et for shared mdc/mdio contortion */

    uint32      intstatus;  /* saved interrupt condition bits */
    uint32      intmask;    /* current software interrupt mask */

    bool        mibgood;    /* true once mib registers have been cleared */
    gmacmib_t   mib;        /* mib statistic counters */
    si_t        *sih;       /* si utils handle */

    char        *vars;      /* sprom name=value */
    uint        vars_size;

    void        *adm;       /* optional admtek private data */
    mcfilter_t  mf;     /* multicast filter */
};

/* Using CFP entries to achieve the promiscuous function */
/* Masked it due to reset GMAC cores will impact CFP entries. */
#if 0
#define GMAC_PROMISC_BY_CFP
#endif

/* local prototypes */
static bool chipid(uint vendor, uint device);
static void *chipattach(etc_info_t *etc, void *osh, void *regsva);
static void chipdetach(ch_t *ch);
static void chipreset(ch_t *ch);
static void chipinit(ch_t *ch, uint options);
static bool chiptx(ch_t *ch, void *p);
static void *chiprx(ch_t *ch);
static void chiprxfill(ch_t *ch);
static int chipgetintrevents(ch_t *ch, bool in_isr);
static bool chiperrors(ch_t *ch);
static void chipintrson(ch_t *ch);
static void chipintrsoff(ch_t *ch);
static void chiptxreclaim(ch_t *ch, bool all);
static void chiprxreclaim(ch_t *ch);
static void chipstatsupd(ch_t *ch);
static void chipdumpmib(ch_t *ch, char *b);
static void chipenablepme(ch_t *ch);
static void chipdisablepme(ch_t *ch);
static void chipphyreset(ch_t *ch, uint phyaddr);
static uint16 chipphyrd(ch_t *ch, uint phyaddr, uint reg);
static void chipphywr(ch_t *ch, uint phyaddr, uint reg, uint16 v);
static void chipdump(ch_t *ch, struct bcmstrbuf *b);
static void chiplongname(ch_t *ch, char *buf, uint bufsize);
static void chipduplexupd(ch_t *ch);

static void chipphyinit(ch_t *ch, uint phyaddr);
static void chipphyor(ch_t *ch, uint phyaddr, uint reg, uint16 v);
static void chipphyforce(ch_t *ch, uint phyaddr);
static void chipphyadvertise(ch_t *ch, uint phyaddr);
static int chipcfprd(ch_t *ch, void *buf);
static int chipcfpwr(ch_t *ch, void *buf);
static int chipcfpfldrd(ch_t *ch, void *buf);
static int chipcfpfldwr(ch_t *ch, void *buf);
static int chipcfpudfrd(ch_t *ch, void *buf);
static int chipcfpudfwr(ch_t *ch, void *buf);
static int chiprxrateset(ch_t *ch, uint channel, uint pps);
static int chiprxrateget(ch_t *ch, uint channel, uint *pps);
static int chiptxrateset(ch_t *ch, uint channel, uint rate, uint burst);
static int chiptxrateget(ch_t *ch, uint channel, uint *rate, uint *burst);
static int chipflowctrlmodeset(ch_t *ch, uint mode);
static int chipflowctrlmodeget(ch_t *ch, uint *mode);
static int chipflowctrlautoset(ch_t *ch, uint on_thresh, uint off_thresh);
static int chipflowctrlautoget(ch_t *ch, uint *on_thresh, uint *off_thresh);
static int chipflowctrlcpuset(ch_t *ch, uint pause_on);
static int chipflowctrlcpuget(ch_t *ch, uint *pause_on);
static int chipflowctrlrxchanset(ch_t *ch, uint rxchan, uint on_thresh, uint off_thresh);
static int chipflowctrlrxchanget(ch_t *ch, uint rxchan, uint *on_thresh, uint *off_thresh);
static int chiptpidset(ch_t *ch, uint index, uint tpid);
static int chiptpidget(ch_t *ch, uint index, uint *tpid);
static int chippvtagset(ch_t *ch, uint private_tag);
static int chippvtagget(ch_t *ch, uint *private_tag);
static int chiprxsephdrset(ch_t *ch, uint enable);
static int chiprxsephdrget(ch_t *ch, uint *enable);
static int chiptxqosmodeset(ch_t *ch, uint mode);
static int chiptxqosmodeget(ch_t *ch, uint *mode);
static int chiptxqosweightset(ch_t *ch, uint queue, uint weight);
static int chiptxqosweightget(ch_t *ch, uint queue, uint *weight);

#ifdef CFG_ROBO
static void robo_port_enable(ch_t *ch);
static void robo_port_stop(ch_t *ch);
#define SWAP16(x) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
#endif /* CFG_ROBO */

#ifdef BCMDBG
static void chipdumpregs(ch_t *ch, gmac0regs_t *regs, struct bcmstrbuf *b);
#endif /* BCMDBG */
static void gmac_mf_cleanup(ch_t *ch);
static int gmac_speed(ch_t *ch, uint32 speed);
static void gmac_miiconfig(ch_t *ch);

struct chops bcmgmac_et_chops = {
    chipid,
    chipattach,
    chipdetach,
    chipreset,
    chipinit,
    chiptx,
    chiprx,
    chiprxfill,
    chipgetintrevents,
    chiperrors,
    chipintrson,
    chipintrsoff,
    chiptxreclaim,
    chiprxreclaim,
    chipstatsupd,
    chipdumpmib,
    chipenablepme,
    chipdisablepme,
    chipphyreset,
    chipphyrd,
    chipphywr,
    chipdump,
    chiplongname,
    chipduplexupd,
    chipcfprd,
    chipcfpwr,
    chipcfpfldrd,
    chipcfpfldwr,
    chipcfpudfrd,
    chipcfpudfwr,
    chiprxrateset,
    chiprxrateget,
    chiptxrateset,
    chiptxrateget,
    chipflowctrlmodeset,
    chipflowctrlmodeget,
    chipflowctrlautoset,
    chipflowctrlautoget,
    chipflowctrlcpuset,
    chipflowctrlcpuget,
    chipflowctrlrxchanset,
    chipflowctrlrxchanget,
    chiptpidset,
    chiptpidget,
    chippvtagset,
    chippvtagget,
    chiprxsephdrset,
    chiprxsephdrget,
    chiptxqosmodeset,
    chiptxqosmodeget,
    chiptxqosweightset,
    chiptxqosweightget
};

static uint devices[] = {
    BCM53000_GMAC_ID,
    0x0000
};

#ifdef GMAC_PROMISC_BY_CFP
#define CFP_PROMISC_ENTRY_NUM   12

static uint cfp_promisc[] = {
    0x1,
    0x1
};
#endif /* GMAC_PROMISC_BY_CFP */


static bool
chipid(uint vendor, uint device)
{
    int i;

    if (vendor != VENDOR_BROADCOM)
        return (FALSE);

    for (i = 0; devices[i]; i++) {
        if (device == devices[i])
            return (TRUE);
    }

    return (FALSE);
}

#ifdef ROBODVT
static ch_t *etcgmac_ch = NULL;
static SEM_ID mii_mutex;
#define MII_LOCK()         semTake(mii_mutex, WAIT_FOREVER)
#define MII_UNLOCK()       semGive(mii_mutex)
#endif

/* Check ME : 
 *   - the QT_MDIO_PHYADDR is a force defined value for testing the MDIO 
 *      model in QT 18. Must be removed for the formal release.
 */
static void*
chipattach(etc_info_t *etc, void *osh, void *regsva)
{
    ch_t *ch;
    gmac0regs_t *regs;
    char name[16];
    uint boardflags, boardtype;
#ifndef CFG_QUICKTURN
    char *var;
#endif 

    ET_TRACE(("et%d: chipattach: regsva 0x%lx\n", etc->unit, (ulong)regsva));

    if ((ch = (ch_t *) MALLOC(osh, sizeof(ch_t))) == NULL) {
        ET_ERROR(("et%d: chipattach: out of memory, malloced %d bytes\n", etc->unit,
                  MALLOCED(osh)));
        return (NULL);
    }
#ifdef ROBODVT
    else {
        etcgmac_ch = ch;
    }

    /* initialize private semaphore */
    if ((mii_mutex = semBCreate(SEM_Q_FIFO, SEM_FULL)) == NULL) {
        goto fail;
    }
#endif

    bzero((char *)ch, sizeof(ch_t));

    ch->etc = etc;
    ch->et = etc->et;
    ch->osh = osh;


    /* store the pointer to the sw mib */
    etc->mib = (void *)&ch->mib;

    /* get si handle */
    if ((ch->sih = si_attach(etc->deviceid, ch->osh, regsva, PCI_BUS, NULL, &ch->vars,
                             &ch->vars_size)) == NULL) {
        ET_ERROR(("et%d: chipattach: si_attach error\n", etc->unit));
        goto fail;
    }

    if ((regs = (gmac0regs_t *)si_setcore(ch->sih, GMAC_CORE_ID, etc->unit)) == NULL) {
        ET_ERROR(("et%d: chipattach: Could not setcore to the GMAC core\n", etc->unit));
        goto fail;
    }

    ch->regs = regs;
    etc->chip = ch->sih->chip;
    etc->chiprev = ch->sih->chiprev;
    etc->coreid = si_coreid(ch->sih);
    etc->corerev = si_corerev(ch->sih);
    etc->nicmode = !(ch->sih->bustype == SI_BUS);
    etc->coreunit = si_coreunit(ch->sih);
    etc->boardflags = getintvar(ch->vars, "boardflags");
    etc->allmulti = 1; /* enable all multicasts */
    etc->ext_config = FALSE;

    boardflags = etc->boardflags;
    boardtype = ch->sih->boardtype;

    /* configure pci core */
    si_pci_setup(ch->sih, (1 << si_coreidx(ch->sih)));

    etc->phyaddr = EPHY_NOREG; /* CHECK ME */
    
    /* PED: this should be done before chipreset */
    {
        extern void et_dma_attach(void *eti);
        et_dma_attach(ch->et);
    }
    
    /* reset the gmac core */
    chipreset(ch);

    /* dma attach */
    sprintf(name, "et%d", etc->coreunit);


#ifdef CFG_QUICKTURN

    etc->perm_etheraddr.octet [0] = 0x0; 
    etc->perm_etheraddr.octet [1] = 0x10; 
    etc->perm_etheraddr.octet [2] = 0x18; 
    etc->perm_etheraddr.octet [3] = 0x53; 
    etc->perm_etheraddr.octet [4] = 0x00; 
    etc->perm_etheraddr.octet [5] = 0x00|(uint8)etc->coreunit; 
    bcopy((char*)&etc->perm_etheraddr, (char*)&etc->cur_etheraddr, ETHER_ADDR_LEN);

#else /* !CFG_QUICKTURN */

    /* get our local ether addr */
    sprintf(name, "ETH%d_HWADDR", etc->coreunit);
    var = getvar(ch->vars, name);
    if (var == NULL) {
        ET_ERROR(("et%d: chipattach: getvar(%s) not found, using default value\n", etc->unit, name));
        etc->perm_etheraddr.octet [0] = 0x0; 
        etc->perm_etheraddr.octet [1] = 0x10; 
        etc->perm_etheraddr.octet [2] = 0x18; 
        etc->perm_etheraddr.octet [3] = 0x53; 
        etc->perm_etheraddr.octet [4] = 0x00; 
        etc->perm_etheraddr.octet [5] = 0x00|(uint8)etc->coreunit; 
        bcopy((char*)&etc->perm_etheraddr, (char*)&etc->cur_etheraddr, 
            ETHER_ADDR_LEN);
    } else {
        bcm_ether_atoe(var, &etc->perm_etheraddr);
    }

    if (ETHER_ISNULLADDR(&etc->perm_etheraddr)) {
        ET_ERROR(("et%d: chipattach: invalid format: %s=%s\n", etc->unit, name, var));
        goto fail;
    }
    bcopy((char*)&etc->perm_etheraddr, (char*)&etc->cur_etheraddr, ETHER_ADDR_LEN);

    /*
     * Too much can go wrong in scanning MDC/MDIO playing "whos my phy?" .
     * Instead, explicitly require the environment var "et<coreunit>phyaddr=<val>".
     */

    /* get our phyaddr value */
    sprintf(name, "et%dphyaddr", etc->coreunit);
    var = getvar(ch->vars, name);
    if (var == NULL) {
        ET_ERROR(("et%d: chipattach: getvar(%s) not found, using default value\n", etc->unit, name));
        etc->phyaddr = EPHY_NOREG;
    } else {
        etc->phyaddr = bcm_atoi(var) & EPHY_MASK;
    }

    /* nvram says no phy is present */
    if (etc->phyaddr == EPHY_NONE) {
        ET_ERROR(("et%d: chipattach: phy not present\n", etc->unit));
        goto fail;
    }
#endif /* CFG_QUICKTURN */

    /* set default sofware intmask */
    ch->intmask = DEF_INTMASK;

    /* reset phy: reset it once now */
    chipphyreset(ch, etc->phyaddr);

#ifdef CFG_ROBO
    if (etc->unit == 0){
        robo_port_enable(ch);
    }
#endif /* CFG_ROBO */    

    return ((void*) ch);

fail:
    chipdetach(ch);
    return (NULL);
}

static void
chipdetach(ch_t *ch)
{
    ET_TRACE(("et%d: chipdetach\n", ch->etc->unit));

    if (ch == NULL)
        return;

#ifdef CFG_ROBO
    if (ch->etc->unit == 0){
        robo_port_stop(ch);
    }
#endif /* CFG_ROBO */    


    /* put the core back into reset */
    if (ch->sih)
        si_core_disable(ch->sih, 0);

    ch->etc->mib = NULL;

    /* free si handle */
    si_detach(ch->sih);
    ch->sih = NULL;

    /* free vars */
    if (ch->vars)
        MFREE(ch->osh, ch->vars, ch->vars_size);

    /* free chip private state */
    MFREE(ch->osh, ch, sizeof(ch_t));
}

static void
chiplongname(ch_t *ch, char *buf, uint bufsize)
{
    char *s;

    switch (ch->etc->deviceid) {
        case BCM53000_GMAC_ID:
        default:
            s = "Broadcom BCM53000 10/100/1000 Mbps Ethernet Controller";
            break;
    }

    strncpy(buf, s, bufsize);
    buf[bufsize - 1] = '\0';
}

static void
chipdump(ch_t *ch, struct bcmstrbuf *b)
{
#ifdef BCMDBG
    int32 i;

    printf("regs 0x%lx etphy 0x%lx ch->intstatus 0x%x intmask 0x%x\n",
        (ulong)ch->regs, (ulong)ch->etphy, ch->intstatus, ch->intmask);
    printf("\n");

    /* XXX bcm_binit() to move up in chain of functions where b is allocated, when
     * all sprintf's are replaced by bcm_bprintf's
     */


    /* registers */
    chipdumpregs(ch, ch->regs, b);
    printf("\n");

#endif  /* BCMDBG */
}

#define PRREG(name) printf(#name " 0x%x ", R_REG(ch->osh, &regs->name))
#define PRMIB(name) printf(#name " 0x%x ", ch->mib.name)

#ifdef BCMDBG

static void
chipdumpregs(ch_t *ch, gmac0regs_t *regs, struct bcmstrbuf *b)
{
    uint phyaddr;

    phyaddr = ch->etc->phyaddr;

    PRREG(devcontrol); PRREG(devstatus);
    printf("\n");
    PRREG(biststatus);
    printf("\n");
    PRREG(intstatus); PRREG(intmask); PRREG(gptimer);
    printf("\n");
    PRREG(rx_ch0_flow_ctrl); PRREG(rx_ch1_flow_ctrl);
    printf("\n");
    PRREG(rx_ch2_flow_ctrl); PRREG(rx_ch3_flow_ctrl); PRREG(desc_flow_ctrl_ps_stat);
    printf("\n");
    PRREG(intrecvlazy);
    printf("\n");
    PRREG(flowctlthresh); PRREG(txqos); PRREG(gmac_idle_cnt_thresh);
    printf("\n");
    PRREG(fifoaccessaddr); PRREG(fifoaccessbyte); PRREG(fifoaccessdata);
    printf("\n");
    PRREG(irc_cfg);
    printf("\n");
    PRREG(erc0_cfg); PRREG(erc1_cfg); PRREG(erc2_cfg); PRREG(erc3_cfg);
    printf("\n");
    PRREG(txqctl); PRREG(rxqctl);
    printf("\n");
    PRREG(gpioselect); PRREG(gpio_output_en);
    printf("\n");
    PRREG(clk_ctl_st); PRREG(pwrctl);
    printf("\n");

    /* unimac registers */
    PRREG(core_version); PRREG(ipg_hd_bkp_cntl);
    printf("\n");
    PRREG(command_config);
    printf("\n");
    PRREG(mac_0); PRREG(mac_1);
    printf("\n");
    PRREG(frm_length); PRREG(pause_quant); PRREG(mac_mode);
    printf("\n");
    PRREG(tx_ipg_length); PRREG(ts_status_cntrl); PRREG(tx_ts_data); PRREG(pause_control);
    printf("\n");
    PRREG(flush_control); PRREG(rxfifo_stat); PRREG(txfifo_stat);
    printf("\n");

    if (phyaddr != EPHY_NOREG) {
        /* print a few interesting phy registers */
        printf("phy0 0x%x phy1 0x%x phy2 0x%x phy3 0x%x\n",
                       chipphyrd(ch, phyaddr, 0),
                       chipphyrd(ch, phyaddr, 1),
                       chipphyrd(ch, phyaddr, 2),
                       chipphyrd(ch, phyaddr, 3));
        printf("phy4 0x%x phy5 0x%x phy24 0x%x phy25 0x%x\n",
                       chipphyrd(ch, phyaddr, 4),
                       chipphyrd(ch, phyaddr, 5),
                       chipphyrd(ch, phyaddr, 24),
                       chipphyrd(ch, phyaddr, 25));
    }

}
#endif  /* BCMDBG */

static void
gmac_clearmib(ch_t *ch)
{
    volatile uint32 *ptr;

    /* enable clear on read */
    OR_REG(ch->osh, &ch->regs->devcontrol, DEVCONTROL_MIB_RESET_ON_READ_MASK);

    for (ptr = &ch->regs->tx_good_octets; ptr <= &ch->regs->rx_cfp_drop; ptr++) {
        (void)R_REG(ch->osh, ptr);
        if (ptr == &ch->regs->tx_pause_pkts)
            ptr = ptr + 5;
    }

    return;
}

static void
gmac_init_reset(ch_t *ch)
{    
    OR_REG(ch->osh, &ch->regs->command_config, COMMAND_CONFIG_SW_RESET_MASK);
    OSL_DELAY(GMAC_RESET_DELAY);
}

static void
gmac_clear_reset(ch_t *ch)
{
    AND_REG(ch->osh, &ch->regs->command_config, ~COMMAND_CONFIG_SW_RESET_MASK);
    OSL_DELAY(GMAC_RESET_DELAY);
}

static void
gmac_reset(ch_t *ch)
{
    uint32 ocmdcfg, cmdcfg;

    /* put the mac in reset */
    gmac_init_reset(ch);

    /* initialize default config */
    ocmdcfg = cmdcfg = R_REG(ch->osh, &ch->regs->command_config);

    cmdcfg &= ~(COMMAND_CONFIG_TX_ENA_MASK | COMMAND_CONFIG_RX_ENA_MASK | 
        COMMAND_CONFIG_PAUSE_IGNORE_MASK | COMMAND_CONFIG_TX_ADDR_INS_MASK | 
        COMMAND_CONFIG_HD_ENA_MASK | COMMAND_CONFIG_MAC_LOOP_CON_MASK |
        COMMAND_CONFIG_CNTL_FRM_ENA_MASK | COMMAND_CONFIG_LINE_LOOPBACK_MASK | 
        COMMAND_CONFIG_IGNORE_TX_PAUSE_MASK | COMMAND_CONFIG_PAD_EN_MASK);
    cmdcfg |= (COMMAND_CONFIG_PROMIS_EN_MASK | COMMAND_CONFIG_PAUSE_FWD_MASK 
        | COMMAND_CONFIG_NO_LGTH_CHECK_MASK | COMMAND_CONFIG_CNTL_FRM_ENA_MASK);

    /* GNAT: Keystone_DVT/3853 */
    /* The bit of COMMAND_CONFIG_ENA_EXT_CONFIG_MASK should be clear */
    if (ch->etc->flowcntl_mode != FLOW_CTRL_MODE_DISABLE) {
        cmdcfg &= ~COMMAND_CONFIG_ENA_EXT_CONFIG_MASK;
    }
    
    if (cmdcfg & COMMAND_CONFIG_ENA_EXT_CONFIG_MASK) {
        ch->etc->ext_config = TRUE;
    } else {
        ch->etc->ext_config = FALSE;
    }

    if (cmdcfg != ocmdcfg)
        W_REG(ch->osh, &ch->regs->command_config, cmdcfg);

    /* bring mac out of reset */
    gmac_clear_reset(ch);
}

#ifdef GMAC_PROMISC_BY_CFP

#define SET_CFP_FIELD_PARAM(fld_idx, fld_val, ram, l3_fram, s_id, buf_ptr) { \
    buf_ptr->field_idx = fld_idx; buf_ptr->field_value = fld_val; \
    buf_ptr->ram_type = ram; buf_ptr->l3_framing = l3_fram; \
    buf_ptr->slice_id = s_id;}


/* Construct a valid entry with drop action */
static void
gmac_cfp_entry_init(ch_t *ch, uint l3_fram, uint slice_id, void *arg)
{
    int i;
    cfp_ioctl_buf_t *cfp_buf_ptr;

    cfp_buf_ptr = (cfp_ioctl_buf_t *)arg;

    for (i=0; i < CFP_TCAM_ENTRY_WORD; i++) {
        cfp_buf_ptr->cfp_entry.tcam[i] = 0;
        cfp_buf_ptr->cfp_entry.tcam_mask[i] = 0;
    }
    cfp_buf_ptr->cfp_entry.action= 0;

    /* Create valid entry and drop action */
    cfp_buf_ptr->entry_idx = 0;
    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_VALID, 3, CFP_RAM_TYPE_TCAM, 
        CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    chipcfpfldwr(ch, (void *)(cfp_buf_ptr));

    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_VALID, 3, CFP_RAM_TYPE_TCAM_MASK, 
        CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    chipcfpfldwr(ch, (void *)(cfp_buf_ptr));

    /* Configure the L3 Framming value */
    SET_CFP_FIELD_PARAM(CFP_FIELD_NONIP_L3_FRAMING, l3_fram, 
        CFP_RAM_TYPE_TCAM, CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    chipcfpfldwr(ch, (void *)(cfp_buf_ptr));
    SET_CFP_FIELD_PARAM(CFP_FIELD_NONIP_L3_FRAMING, 0x3, 
        CFP_RAM_TYPE_TCAM_MASK, CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    chipcfpfldwr(ch, (void *)(cfp_buf_ptr));

    /* set slice id */
    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_SLICE_ID, slice_id, 
            CFP_RAM_TYPE_TCAM, CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    chipcfpfldwr(ch, (void *)(cfp_buf_ptr));
    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_SLICE_ID, 3, 
            CFP_RAM_TYPE_TCAM_MASK, CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    chipcfpfldwr(ch, (void *)(cfp_buf_ptr));
    
}


/*
 * Using CFP entries to meet the promiscous mode
 */
static void
gmac_cfp_promisc(ch_t *ch, bool on)
{
    cfp_ioctl_buf_t *cfp_buffer;
    uint slice_id, start_entry_idx;
    uint udf_field_idx = 0, field_value, field_mask;
    uint udf_valid_field_idx = 0;
    uint i, l3_fram;
        
    ET_TRACE(("et%d: gamc_cfp_promisc: %d\n", ch->etc->unit, on));

    if (!(cfp_buffer = (cfp_ioctl_buf_t *)MALLOC(ch->osh, sizeof(cfp_ioctl_buf_t)))) {
        printf("Error : gamc_cfp_promisc() KMALLOC failed!");
        return;
    }


    /* Using the last 12 entries */
    start_entry_idx = CFP_TCAM_NUM - CFP_PROMISC_ENTRY_NUM;

    /* Choose the UDF 0, 1, 2 of SLICE 2 to use */
    slice_id = 0;

    for (l3_fram = 0; l3_fram <= CFP_L3_FRAMING_NONIP; l3_fram++) { 
        /* Check L3 framing */
        if ((l3_fram != CFP_L3_FRAMING_IPV4) && 
            (l3_fram != CFP_L3_FRAMING_IPV6) &&
            (l3_fram != CFP_L3_FRAMING_NONIP)) {
            continue;
        }
        gmac_cfp_entry_init(ch, l3_fram, slice_id, cfp_buffer);

        /* Configure the UDF offset value and base */
        for (i= 0; i < 3; i++) {
            cfp_buffer->field_idx = i; /* UDF index */
            cfp_buffer->l3_framing = l3_fram;
            cfp_buffer->slice_id= slice_id;
            cfp_buffer->field_value = (2 * i); /* offset value */
            cfp_buffer->flags = CFP_UDF_OFFSET_BASE_STARTOFFRAME;
            chipcfpudfwr(ch, (void *)(cfp_buffer));
         }
     }

    /* CFP entries */
    for (l3_fram = 0; l3_fram <= CFP_L3_FRAMING_NONIP; l3_fram++) { 
        /* Check L3 framing */
        if ((l3_fram != CFP_L3_FRAMING_IPV4) && 
            (l3_fram != CFP_L3_FRAMING_IPV6) &&
            (l3_fram != CFP_L3_FRAMING_NONIP)) {
            continue;
        }

        switch (l3_fram) {
            case CFP_L3_FRAMING_IPV4:
                udf_field_idx = CFP_FIELD_IPV4_UDF0;
                udf_valid_field_idx = CFP_FIELD_IPV4_UDF0_VLD;
                break;
            case CFP_L3_FRAMING_IPV6:
                udf_field_idx = CFP_FIELD_IPV6_UDF0;
                udf_valid_field_idx = CFP_FIELD_IPV6_UDF0_VLD;
                break;
            case CFP_L3_FRAMING_NONIP:
                udf_field_idx = CFP_FIELD_NONIP_UDF0;
                udf_valid_field_idx = CFP_FIELD_NONIP_UDF0_VLD;
                break;
        }

        /* 1. First entry for broadcast and multicast packets pass to CPU */
        /* Configure the TCAM DATA and MASK to 0x0100 of UDF0*/
        gmac_cfp_entry_init(ch, l3_fram, slice_id, cfp_buffer);
        cfp_buffer->entry_idx = start_entry_idx++;
        /* UDF value */
        SET_CFP_FIELD_PARAM(udf_field_idx, 0x0100, 
            CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
        chipcfpfldwr(ch, (void *)(cfp_buffer));
        SET_CFP_FIELD_PARAM(udf_field_idx, 0x0100, 
            CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
        chipcfpfldwr(ch, (void *)(cfp_buffer));
        /* UDF valid */
        SET_CFP_FIELD_PARAM(udf_valid_field_idx, 0x1, 
            CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
        chipcfpfldwr(ch, (void *)(cfp_buffer));
        SET_CFP_FIELD_PARAM(udf_valid_field_idx, 0x1, 
            CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
        chipcfpfldwr(ch, (void *)(cfp_buffer));
        /* write this entry to CFP */
        chipcfpwr(ch, (void *)(cfp_buffer));

        /* 2. DA match my MAC address packets will be pass to CPU */
        if (ch->etc->unit == 1) {
            /* Bypass the entry for GMAC 0 */
            start_entry_idx++;
        }
        gmac_cfp_entry_init(ch, l3_fram, slice_id, cfp_buffer);
        cfp_buffer->entry_idx = start_entry_idx++;
        for (i= 0; i < 3; i++) {
            field_value = ((ch->etc->perm_etheraddr.octet[(2 * i)] << 8) |
                (ch->etc->perm_etheraddr.octet[(2 * i) + 1]));
            /* UDF value */
            SET_CFP_FIELD_PARAM(udf_field_idx + i, field_value, 
                CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
            chipcfpfldwr(ch, (void *)(cfp_buffer));
            SET_CFP_FIELD_PARAM(udf_field_idx + i, 0xffff, 
                CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
            chipcfpfldwr(ch, (void *)(cfp_buffer));
            /* UDF valid */
            SET_CFP_FIELD_PARAM(udf_valid_field_idx + i, 0x1, 
                CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
            chipcfpfldwr(ch, (void *)(cfp_buffer));
            SET_CFP_FIELD_PARAM(udf_valid_field_idx + i, 0x1, 
                CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
            chipcfpfldwr(ch, (void *)(cfp_buffer));
        }
        /* Since the source port bitmap is the same for all l3 framing types */
        /* We use IPv4 format to configure it */
        SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_SRC_PMAP, 0x1 << ch->etc->unit, 
            CFP_RAM_TYPE_TCAM, CFP_L3_FRAMING_IPV4, slice_id, cfp_buffer);
        chipcfpfldwr(ch, (void *)(cfp_buffer));

        SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_SRC_PMAP, 0x3, 
            CFP_RAM_TYPE_TCAM_MASK, CFP_L3_FRAMING_IPV4, slice_id, cfp_buffer);
        chipcfpfldwr(ch, (void *)(cfp_buffer));
        /* write this entry to CFP */
        chipcfpwr(ch, (void *)(cfp_buffer));
        if (ch->etc->unit == 0) {
            /* Bypass the entry for GMAC 1 */
            start_entry_idx++;
        }

        /* 3. Default entry to dop all packets */
        gmac_cfp_entry_init(ch, l3_fram, slice_id, cfp_buffer);
        cfp_buffer->entry_idx = start_entry_idx++;
        
        if (!cfp_promisc[0] && !cfp_promisc[1]) {
            /* Alwaye hit */
            field_value = 0x3;
            field_mask = 0x0;
        } else if (!cfp_promisc[0]) {
            /* port 0 */
            field_value = 0x1;
            field_mask = 0x3;
        } else if (!cfp_promisc[1]) {
            /* port 1 */ 
            field_value = 0x2;
            field_mask = 0x3;
        } else {
            /* Not port 0 and not port 1 */
            field_value = 0x0;
            field_mask = 0x3;
        }

        
        SET_CFP_FIELD_PARAM(CFP_FIELD_ACT_DROP, 1, CFP_RAM_TYPE_ACTION, 
            l3_fram, slice_id, cfp_buffer);
            
        chipcfpfldwr(ch, (void *)(cfp_buffer));
        /* write this entry to CFP */
        chipcfpwr(ch, (void *)(cfp_buffer));
        
    } 
    
    MFREE(ch->osh, cfp_buffer, sizeof(cfp_ioctl_buf_t));
}

#endif /* GMAC_PROMISC_BY_CFP */

static void
gmac_promisc(ch_t *ch, bool mode)
{
#ifndef GMAC_PROMISC_BY_CFP
    uint32 cmdcfg;

    cmdcfg = R_REG(ch->osh, &ch->regs->command_config);
#endif /* GMAC_PROMISC_BY_CFP */    

    /* put the mac in reset */
    gmac_init_reset(ch);

#ifdef GMAC_PROMISC_BY_CFP
    cfp_promisc[ch->etc->unit] = mode;
    gmac_cfp_promisc(ch, mode);
    
#else /* !GMAC_PROMISC_BY_CFP*/

    /* enable or disable promiscuous mode */
    if (mode)
        cmdcfg |= COMMAND_CONFIG_PROMIS_EN_MASK;
    else
        cmdcfg &= ~COMMAND_CONFIG_PROMIS_EN_MASK;

    W_REG(ch->osh, &ch->regs->command_config, cmdcfg);
#endif /* GMAC_PROMISC_BY_CFP */

    /* bring mac out of reset */
    gmac_clear_reset(ch);
}

static int
gmac_speed(ch_t *ch, uint32 speed)
{
    uint32 cmdcfg;
    uint32 hd_ena = 0;

    switch (speed) {
        case ET_10HALF:
            hd_ena = COMMAND_CONFIG_HD_ENA_MASK;
            /* FALLTHRU */

        case ET_10FULL:
            speed = 0;
            break;

        case ET_100HALF:
            hd_ena = COMMAND_CONFIG_HD_ENA_MASK;
            /* FALLTHRU */

        case ET_100FULL:
            speed = 1;
            break;

        case ET_1000FULL:
            speed = 2;
            break;

        case ET_1000HALF:
            ET_ERROR(("et%d: gmac_speed: supports 1000 mbps full duplex only\n",
                      ch->etc->unit));
            return (FAILURE);

        default:
            ET_ERROR(("et%d: gmac_speed: speed %d not supported\n",
                      ch->etc->unit, speed));
            return (FAILURE);
    }

    cmdcfg = R_REG(ch->osh, &ch->regs->command_config);

    /* put mac in reset */
    gmac_init_reset(ch);

    /* set the speed */
    cmdcfg &= ~(COMMAND_CONFIG_ETH_SPEED_MASK | COMMAND_CONFIG_HD_ENA_MASK);
    cmdcfg |= ((speed << COMMAND_CONFIG_ETH_SPEED_SHIFT) | hd_ena);
    W_REG(ch->osh, &ch->regs->command_config, cmdcfg);

    /* bring mac out of reset */
    gmac_clear_reset(ch);

    return (SUCCESS);
}

static void
gmac_macloopback(ch_t *ch, bool on)
{
    uint32 ocmdcfg, cmdcfg;

    ocmdcfg = cmdcfg = R_REG(ch->osh, &ch->regs->command_config);

    /* put mac in reset */
    gmac_init_reset(ch);

    /* set/clear the mac loopback mode */
    if (on)
        cmdcfg |= COMMAND_CONFIG_LOOP_ENA_MASK;
    else
        cmdcfg &= ~COMMAND_CONFIG_LOOP_ENA_MASK;

    if (cmdcfg != ocmdcfg)
        W_REG(ch->osh, &ch->regs->command_config, cmdcfg);

    /* bring mac out of reset */
    gmac_clear_reset(ch);
}

static int
gmac_loopback(ch_t *ch, uint32 mode)
{
    switch (mode) {
        case LOOPBACK_MODE_DMA:
            break;

        case LOOPBACK_MODE_MAC:
            gmac_macloopback(ch, TRUE);
            break;

        case LOOPBACK_MODE_NONE:
            gmac_macloopback(ch, FALSE);
            break;

        default:
            ET_ERROR(("et%d: gmac_loopaback: Unknown loopback mode %d\n",
                      ch->etc->unit, mode));
            return (FAILURE);
    }

    return (SUCCESS);
}

static void
gmac_enable(ch_t *ch)
{
    uint32 ocmdcfg, cmdcfg, rxqctl;
    gmac0regs_t *regs;

    regs = ch->regs;

    ocmdcfg = cmdcfg = R_REG(ch->osh, &ch->regs->command_config);

    /* put mac in reset */
    gmac_init_reset(ch);

    cmdcfg |= COMMAND_CONFIG_SW_RESET_MASK;

    /* first deassert rx_ena and tx_ena while in reset */
    cmdcfg &= ~(COMMAND_CONFIG_RX_ENA_MASK | COMMAND_CONFIG_TX_ENA_MASK);
    W_REG(ch->osh, &regs->command_config, cmdcfg);

    /* bring mac out of reset */
    gmac_clear_reset(ch);

    /* enable the mac transmit and receive paths now */
    OSL_DELAY(2);
    cmdcfg &= ~COMMAND_CONFIG_SW_RESET_MASK;
    cmdcfg |= (COMMAND_CONFIG_RX_ENA_MASK | COMMAND_CONFIG_TX_ENA_MASK);

    /* assert rx_ena and tx_ena when out of reset to enable the mac */
    W_REG(ch->osh, &regs->command_config, cmdcfg);

    /* request ht clock */
    W_REG(ch->osh, &regs->clk_ctl_st, 0x2);

    /* init the mac data period to 9. this is the optimal value for
     * gmac to work at all backplane clock speeds
     */
    rxqctl = R_REG(ch->osh, &regs->rxqctl);
    rxqctl &= ~RXQCTL_RESERVED0_MASK;
    W_REG(ch->osh, &regs->rxqctl, rxqctl | (RC_MAC_DATA_PERIOD << RXQCTL_RESERVED0_SHIFT));

    return;
}

static void
gmac_txflowcontrol(ch_t *ch, bool on)
{
    uint32 cmdcfg;

    cmdcfg = R_REG(ch->osh, &ch->regs->command_config);

    /* put the mac in reset */
    gmac_init_reset(ch);

    /* to enable tx flow control clear the rx pause ignore bit */
    if (on)
        cmdcfg &= ~(COMMAND_CONFIG_PAUSE_IGNORE_MASK |
            COMMAND_CONFIG_IGNORE_TX_PAUSE_MASK);
    else
        cmdcfg |= COMMAND_CONFIG_PAUSE_IGNORE_MASK |
            COMMAND_CONFIG_IGNORE_TX_PAUSE_MASK; 

    W_REG(ch->osh, &ch->regs->command_config, cmdcfg);

    /* bring mac out of reset */
    gmac_clear_reset(ch);
}

static void
gmac_miiconfig(ch_t *ch)
{
    uint32 devstatus, mode;
    gmac0regs_t *regs;

    regs = ch->regs;

    if (ch->etc->phyaddr != EPHY_NOREG)
        return;

    /* Read the devstatus to figure out the configuration
     * mode of the interface.
     */
    devstatus = R_REG(ch->osh, &regs->devstatus);
    mode = ((devstatus & 
            DEVSTATUS_MII_MODE_MASK) >> DEVSTATUS_MII_MODE_SHIFT);

    /* Set the speed to 100 if the switch interface is 
     * using mii/rev mii.
     */
    if ((mode == 0) || (mode == 1)) {
        if (ch->etc->forcespeed == ET_AUTO)
            gmac_speed(ch, ET_100FULL);
        else
            gmac_speed(ch, ch->etc->forcespeed);
    } else if ((mode == 2) || (mode == 3)) {
        if (ch->etc->forcespeed == ET_AUTO)
            gmac_speed(ch, ET_1000FULL);
        else
            gmac_speed(ch, ch->etc->forcespeed);
    }
}

static void
chipreset(ch_t *ch)
{
    gmac0regs_t *regs;
    gmac_commonregs_t *com_regs;
    uint origidx;

    ET_TRACE(("et%d: chipreset\n", ch->etc->unit));

    regs = ch->regs;

    if (!si_iscoreup(ch->sih)) {
        if (!ch->etc->nicmode)
            si_pci_setup(ch->sih, (1 << si_coreidx(ch->sih)));
        /* power on reset: reset the enet core */
        goto chipinreset;
    }

    /* update software counters before resetting the chip */
    if (ch->mibgood)
        chipstatsupd(ch);

    {
        extern void et_dma_txreset(void *eti);
        et_dma_txreset(ch->et);
    }

    /* set gmac into loopback mode to ensure no rx traffic */
    gmac_loopback(ch, LOOPBACK_MODE_MAC);
    OSL_DELAY(1);

    {
        extern void et_dma_rxreset(void *eti);
        et_dma_rxreset(ch->et);
       }

    /* clear the multicast filter table */
    gmac_mf_cleanup(ch);

chipinreset:
    /* reset core */
    si_core_reset(ch->sih, 0, 0);

    /* reset gmac */
    gmac_reset(ch);

    /* clear mib */
    gmac_clearmib(ch);
    ch->mibgood = TRUE;

    /* Drive mdc clock */
    origidx = si_coreidx(ch->sih);
    com_regs = si_setcore(ch->sih, GMAC_COM_CORE_ID, 0);
    ASSERT(regs != NULL);
    OR_REG(ch->osh, &com_regs->phycontrol, PHYCONTROL_MDC_TRANSITION_EN_MASK);
    si_setcoreidx(ch->sih, origidx);
    

    /* Read the devstatus to figure out the configuration mode of
     * the interface. Set the speed to 100 if the switch interface
     * is mii/rmii.
     */
    gmac_miiconfig(ch);

    /* gmac doesn't have internal phy */
    chipphyinit(ch, ch->etc->phyaddr);

    /* clear persistent sw intstatus */
    ch->intstatus = 0;
}

/*
 * Lookup a multicast address in the filter hash table.
 */
static int
gmac_mf_lkup(ch_t *ch, struct ether_addr *mcaddr)
{
    mflist_t *ptr;

    /* find the multicast address */
    for (ptr = ch->mf.bucket[GMAC_MCADDR_HASH(mcaddr)]; ptr != NULL; ptr = ptr->next) {
        if (!ETHER_MCADDR_CMP(&ptr->mc_addr, mcaddr))
            return (SUCCESS);
    }

    return (FAILURE);
}

/*
 * Add a multicast address to the filter hash table.
 */
static int
gmac_mf_add(ch_t *ch, struct ether_addr *mcaddr)
{
    uint32 hash;
    mflist_t *entry;
    uint8 mac[ETHER_ADDR_STR_LEN];
    
    /* Avoid compiler warning if ET_ERROR does nothing */
    mac[0] = 0;

    /* add multicast addresses only */
    if (!ETHER_ISMULTI(mcaddr)) {
        ET_ERROR(("et%d: adding invalid multicast address %s\n",
                  ch->etc->unit, bcm_ether_ntoa(mcaddr, (char *)mac)));
        return (FAILURE);
    }

    /* discard duplicate add requests */
    if (gmac_mf_lkup(ch, mcaddr) == SUCCESS) {
        ET_ERROR(("et%d: adding duplicate mcast filter entry\n", ch->etc->unit));
        return (FAILURE);
    }

    /* allocate memory for list entry */
    entry = MALLOC(ch->osh, sizeof(mflist_t));
    if (entry == NULL) {
        ET_ERROR(("et%d: out of memory allocating mcast filter entry\n", ch->etc->unit));
        return (FAILURE);
    }

    /* add the entry to the hash bucket */
    ether_copy(mcaddr, &entry->mc_addr);
    hash = GMAC_MCADDR_HASH(mcaddr);
    entry->next = ch->mf.bucket[hash];
    ch->mf.bucket[hash] = entry;

    return (SUCCESS);
}

/*
 * Cleanup the multicast filter hash table.
 */
static void
gmac_mf_cleanup(ch_t *ch)
{
    mflist_t *ptr, *next;
    int32 i;

    for (i = 0; i < GMAC_HASHT_SIZE; i++) {
        for (ptr = ch->mf.bucket[i]; ptr != NULL; ptr = next) {
            next = ptr->next;
            MFREE(ch->osh, ptr, sizeof(mflist_t));
        }
        ch->mf.bucket[i] = NULL;
    }
}

/*
 * Initialize all the chip registers.  If dma mode, init tx and rx dma engines
 * but leave the devcontrol tx and rx (fifos) disabled.
 */
static void
chipinit(ch_t *ch, uint options)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint idx;
    uint i;

    regs = ch->regs;
    etc = ch->etc;
    idx = 0;

    ET_TRACE(("et%d: chipinit\n", etc->unit));;

    /* enable one rx interrupt per received frame */
    W_REG(ch->osh, &regs->intrecvlazy, (1 << INTRECVLAZY_FRAME_COUNT_SHIFT));



    /* flow control mode */
    chipflowctrlmodeset(ch, etc->flowcntl_mode);

    chipflowctrlautoset(ch, etc->flowcntl_auto_on_thresh, 
        etc->flowcntl_auto_off_thresh);

    chipflowctrlcpuset(ch, etc->flowcntl_cpu_pause_on);

    for (i=0; i < NUMRXQ; i++) {
        chipflowctrlrxchanset(ch, i, etc->flowcntl_rx_on_thresh[i], 
            etc->flowcntl_rx_off_thresh[i]);
    }

    /* tx qos mode */
    chiptxqosmodeset(ch, etc->txqos_mode);
    for (i=0; i < (NUMTXQ - 1); i++) {
        chiptxqosweightset(ch, i, etc->txqos_weight[i]);
    }

    for (i=0; i < NUM_STAG_TPID; i++) {
        chiptpidset(ch, i, etc->tpid[i]);
    }
    
    /* RX separate header is global for Keystone */
    if (etc->en_rxsephdr[0]) {
        chiprxsephdrset(ch, 1);
    } else {
        chiprxsephdrset(ch, 0);
    }

    /* enable/disable promiscuous mode */
    gmac_promisc(ch, etc->promisc);

    /* Set TX IPG length */
    W_REG(ch->osh, &regs->tx_ipg_length, 12);

    if (!etc->promisc) {
        /* set our local address */
        W_REG(ch->osh, &regs->mac_0,
              hton32(*(uint32 *)&etc->cur_etheraddr.octet[0]));
        W_REG(ch->osh, &regs->mac_1,
              hton16(*(uint16 *)&etc->cur_etheraddr.octet[4]));

        /* gmac doesn't have a cam, hence do the multicast address filtering
         * in the software
         */
        /* allmulti or a list of discrete multicast addresses */
        if (!etc->allmulti && etc->nmulticast)
            for (i = 0; i < etc->nmulticast; i++)
                (void)gmac_mf_add(ch, &etc->multicast[i]);
    }

    /* optionally enable mac-level loopback */
    if (etc->loopbk == LOOPBACK_MODE_MAC)
        gmac_loopback(ch, LOOPBACK_MODE_MAC);
    else if (etc->loopbk == LOOPBACK_MODE_DMA) 
        gmac_loopback(ch, LOOPBACK_MODE_DMA);
    else
        gmac_loopback(ch, LOOPBACK_MODE_NONE);

#ifndef ROBODVT
    /* set max frame lengths - account for possible vlan tag */
    W_REG(ch->osh, &regs->frm_length, ETHER_MAX_LEN + 32);
#else
    /* Enable maximum possible frame length for DVT test env. */
    W_REG(ch->osh, &regs->frm_length, 9720);
#endif

    /*
     * Optionally, disable phy autonegotiation and force our speed/duplex
     * or constrain our advertised capabilities.
     */
    if (etc->forcespeed != ET_AUTO) {
        gmac_speed(ch, etc->forcespeed);
        chipphyforce(ch, etc->phyaddr);
    } else if (etc->advertise && etc->needautoneg)
        chipphyadvertise(ch, etc->phyaddr);

    if (options & ET_INIT_FULL) {
        extern void et_dma_txinit(void *eti);
        extern void et_dma_rxinit(void *eti);

        et_dma_txinit(etc->et);
        et_dma_rxinit(etc->et);

        /* lastly, enable interrupts */
        if (options & ET_INIT_INTRON)
            et_intrson(etc->et);
    } else {
        extern void et_dma_rxenable(void *eti);
        et_dma_rxenable(etc->et);
    }

    /* turn on the emac */
    gmac_enable(ch);
}

/* dma transmit */
static bool
chiptx(ch_t *ch, void *p0)
{

    return TRUE;
}

/* reclaim completed transmit descriptors and packets */
static void
chiptxreclaim(ch_t *ch, bool forceall)
{
    extern void et_dma_txreclaim(void *eti);
    et_dma_txreclaim(ch->etc->et);
}

/* dma receive: returns a pointer to the next frame received, or NULL if there are no more */
static void*
chiprx(ch_t *ch)
{
    return (NULL);
}

/* reclaim completed dma receive descriptors and packets */
static void
chiprxreclaim(ch_t *ch)
{
    extern void et_dma_rxreclaim(void *eti);
    et_dma_rxreclaim(ch->etc->et);
}

/* allocate and post dma receive buffers */
static void
chiprxfill(ch_t *ch)
{
}

/* get current and pending interrupt events */
static int
chipgetintrevents(ch_t *ch, bool in_isr)
{
    uint32 intstatus;
    int events;

    events = 0;

    /* Check if the core is during reset stage */
    if (!si_iscoreup(ch->sih)) {
        return (0);
    }

    /* read the interrupt status register */
    intstatus = R_REG(ch->osh, &ch->regs->intstatus);

    /* defer unsolicited interrupts */
    intstatus &= (in_isr ? ch->intmask : DEF_INTMASK);

    if (intstatus != 0)
        events = INTR_NEW;

    /* or new bits into persistent intstatus */
    intstatus = (ch->intstatus |= intstatus);

    /* return if no events */
    if (intstatus == 0)
        return (0);

    /* convert chip-specific intstatus bits into generic intr event bits */
    if (intstatus & (INTMASK_RCVINTERRUPT_0_MASK | INTMASK_RCVINTERRUPT_1_MASK |
            INTMASK_RCVINTERRUPT_2_MASK | INTMASK_RCVINTERRUPT_3_MASK))
        events |= INTR_RX;
    if (intstatus & (INTMASK_XMT0INTERRUPT_MASK | INTMASK_XMT1INTERRUPT_MASK
            | INTMASK_XMT2INTERRUPT_MASK | INTMASK_XMT3INTERRUPT_MASK))
        events |= INTR_TX;
    if (intstatus & I_ERRORS)
        events |= INTR_ERROR;

    return (events);
}

/* enable chip interrupts */
static void
chipintrson(ch_t *ch)
{
    ch->intmask = DEF_INTMASK;
    W_REG(ch->osh, &ch->regs->intmask, ch->intmask);
}

/* disable chip interrupts */
static void
chipintrsoff(ch_t *ch)
{
    /* disable further interrupts from gmac */
    W_REG(ch->osh, &ch->regs->intmask, 0);
    (void) R_REG(ch->osh, &ch->regs->intmask);  /* sync readback */
    ch->intmask = 0;

    /* clear the interrupt conditions */
    W_REG(ch->osh, &ch->regs->intstatus, ch->intstatus);
}

/* return true of caller should re-initialize, otherwise false */
static bool
chiperrors(ch_t *ch)
{
    uint32 intstatus;
    etc_info_t *etc;

    etc = ch->etc;

    intstatus = ch->intstatus;
    ch->intstatus &= ~(I_ERRORS);

    ET_TRACE(("et%d: chiperrors: intstatus 0x%x\n", etc->unit, intstatus));

    if (intstatus & INTMASK_PCIDESCERROR_MASK) {
        ET_ERROR(("et%d: descriptor error\n", etc->unit));
        etc->dmade++;
    }

    if (intstatus & INTMASK_PCIDATAERROR_MASK) {
        ET_ERROR(("et%d: data error\n", etc->unit));
        etc->dmada++;
    }

    if (intstatus & INTMASK_DESCERROR_MASK) {
        ET_ERROR(("et%d: descriptor protocol error\n", etc->unit));
        etc->dmape++;
    }

    if (intstatus & (INTMASK_RCVDESCUF_0_MASK |INTMASK_RCVDESCUF_1_MASK 
        | INTMASK_RCVDESCUF_2_MASK | INTMASK_RCVDESCUF_3_MASK)) {
        ET_ERROR(("et%d: receive descriptor underflow\n", etc->unit));
        etc->rxdmauflo++;
    }

    if (intstatus & INTMASK_RCVFIFOOVERFLOW_MASK) {
        ET_ERROR(("et%d: receive fifo overflow\n", etc->unit));
        etc->rxoflo++;
    }

    if (intstatus & INTMASK_XMTFIFOUNDERFLOW_MASK) {
        ET_ERROR(("et%d: transmit fifo underflow\n", etc->unit));
        etc->txuflo++;
    }

    /* if overflows or decriptors underflow, don't report it
     * as an error and  provoque a reset
     */
    /* if (intstatus & ~(I_RDU) & I_ERRORS) */
    if (intstatus & I_ERRORS)
        return (TRUE);

    return FALSE;
}

static void
chipstatsupd(ch_t *ch)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    volatile uint32 *s;
    uint32 *d;

    etc = ch->etc;
    regs = ch->regs;

    /* read the mib counters and update the driver maintained software
     * counters.
     */
    OR_REG(ch->osh, &regs->devcontrol, DEVCONTROL_MIB_RESET_ON_READ_MASK);
    for (s = &regs->tx_good_octets, d = &ch->mib.tx_good_octets;
         s <=  &regs->rx_cfp_drop; s++, d++) {
        *d += R_REG(ch->osh, s);
        if (s == &ch->regs->tx_pause_pkts) {
            s = s + 5;
            d = d + 5;
        }
    }


    /*
     * Aggregate transmit and receive errors that probably resulted
     * in the loss of a frame are computed on the fly.
     *
     * We seem to get lots of tx_carrier_lost errors when flipping
     * speed modes so don't count these as tx errors.
     *
     * Arbitrarily lump the non-specific dma errors as tx errors.
     */
    etc->txerror = ch->mib.tx_jabber_pkts + ch->mib.tx_oversize_pkts
        + ch->mib.tx_underruns + ch->mib.tx_excessive_cols
        + ch->mib.tx_late_cols + etc->txnobuf + etc->dmade
        + etc->dmada + etc->dmape + etc->txuflo;
    etc->rxerror = ch->mib.rx_jabber_pkts + ch->mib.rx_oversize_pkts
        + ch->mib.rx_missed_pkts + ch->mib.rx_align_errs
        + ch->mib.rx_undersize + ch->mib.rx_crc_errs
        + ch->mib.rx_symbol_errs
        + etc->rxnobuf + etc->rxdmauflo + etc->rxoflo + etc->rxbadlen;
}

static void
chipdumpmib(ch_t *ch, char *buf)
{
    /* mib registers */
    PRMIB(tx_good_octets); PRMIB(tx_good_octets_high); PRMIB(tx_good_pkts); 
    printf("\n");
    PRMIB(tx_octets); PRMIB(tx_octets_high); PRMIB(tx_pkts);
    printf("\n");
    PRMIB(tx_broadcast_pkts); PRMIB(tx_multicast_pkts); PRMIB(tx_uni_pkts);
    printf("\n");
    PRMIB(tx_len_64); PRMIB(tx_len_65_to_127); PRMIB(tx_len_128_to_255);
    printf("\n");
    PRMIB(tx_len_256_to_511); PRMIB(tx_len_512_to_1023); PRMIB(tx_len_1024_to_max);
    printf("\n");
    PRMIB(tx_len_max_to_jumbo);
    printf("\n");
    PRMIB(tx_jabber_pkts); PRMIB(tx_oversize_pkts); PRMIB(tx_fragment_pkts);
    printf("\n");
    PRMIB(tx_underruns); PRMIB(tx_total_cols); PRMIB(tx_single_cols);
    printf("\n");
    PRMIB(tx_multiple_cols); PRMIB(tx_excessive_cols); PRMIB(tx_late_cols);
    printf("\n");
    PRMIB(tx_defered); PRMIB(tx_pause_pkts);
    printf("\n");

    PRMIB(rx_good_octets); PRMIB(rx_good_octets_high);PRMIB(rx_good_pkts);
    printf("\n");
    PRMIB(rx_octets); PRMIB(rx_octets_high); PRMIB(rx_pkts);
    printf("\n");
    PRMIB(rx_broadcast_pkts); PRMIB(rx_multicast_pkts); PRMIB(rx_uni_pkts);
    printf("\n");
    PRMIB(rx_len_64); PRMIB(rx_len_65_to_127); PRMIB(rx_len_128_to_255);
    printf("\n");
    PRMIB(rx_len_256_to_511); PRMIB(rx_len_512_to_1023); PRMIB(rx_len_1024_to_max);
    printf("\n");
    PRMIB(rx_len_max_to_jumbo);
    printf("\n");
    PRMIB(rx_jabber_pkts); PRMIB(rx_oversize_pkts); PRMIB(rx_fragment_pkts);
    printf("\n");
    PRMIB(rx_missed_pkts);PRMIB(rx_undersize);
    printf("\n");
    PRMIB(rx_crc_errs); PRMIB(rx_align_errs); PRMIB(rx_symbol_errs);
    printf("\n");
    PRMIB(rx_pause_pkts); PRMIB(rx_nonpause_pkts);
    printf("\n");
    PRMIB(rxq0_irc_drop); PRMIB(rxq1_irc_drop);PRMIB(rxq2_irc_drop); PRMIB(rxq3_irc_drop);
    printf("\n");
    PRMIB(rx_cfp_drop);
    printf("\n");
}

static void
chipenablepme(ch_t *ch)
{
    return;
}

static void
chipdisablepme(ch_t *ch)
{
    return;
}

static void
chipduplexupd(ch_t *ch)
{
    uint32 cmdcfg;
    int32 duplex, speed;

    cmdcfg = R_REG(ch->osh, &ch->regs->command_config);

    /* check if duplex mode changed */
    if (ch->etc->duplex && (cmdcfg & COMMAND_CONFIG_HD_ENA_MASK))
        duplex = 0;
    else if (!ch->etc->duplex && ((cmdcfg & COMMAND_CONFIG_HD_ENA_MASK) == 0))
        duplex = COMMAND_CONFIG_HD_ENA_MASK;
    else
        duplex = -1;

    /* check if the speed changed */
    speed = ((cmdcfg & COMMAND_CONFIG_ETH_SPEED_MASK) 
        >> COMMAND_CONFIG_ETH_SPEED_SHIFT);
    if ((ch->etc->speed == 1000) && (speed != 2))
        speed = 2;
    else if ((ch->etc->speed == 100) && (speed != 1))
        speed = 1;
    else if ((ch->etc->speed == 10) && (speed != 0))
        speed = 0;
    else
        speed = -1;

    /* no duplex or speed change required */
    if ((speed == -1) && (duplex == -1))
        return;

    /* update the speed */
    if (speed != -1) {
        cmdcfg &= ~COMMAND_CONFIG_ETH_SPEED_MASK;
        cmdcfg |= (speed << COMMAND_CONFIG_ETH_SPEED_SHIFT);
    }

    /* update the duplex mode */
    if (duplex != -1) {
        cmdcfg &= ~COMMAND_CONFIG_HD_ENA_MASK;
        cmdcfg |= duplex;
    }

    ET_TRACE(("chipduplexupd: updating speed & duplex %x\n", cmdcfg));

    /* put mac in reset */
    gmac_init_reset(ch);

    W_REG(ch->osh, &ch->regs->command_config, cmdcfg);

    /* bring mac out of reset */
    gmac_clear_reset(ch);
}

static uint16
chipphyrd(ch_t *ch, uint phyaddr, uint reg)
{
    uint32 tmp;
    uint origidx;
    gmac_commonregs_t *regs;

    ASSERT(phyaddr < MAXEPHY);
    ASSERT(reg < MAXPHYREG);

#ifdef ROBODVT
    MII_LOCK();
#else
    if (phyaddr == EPHY_NOREG) {
        ET_ERROR(("et%d: chipphyrd: no phy\n", ch->etc->unit));
        return 0xffff;
    }
#endif

    /* Remember original core before switch to gmac common */
    origidx = si_coreidx(ch->sih);
    regs = si_setcore(ch->sih, GMAC_COM_CORE_ID, 0);
    ASSERT(regs != NULL);

    /* set phyaccess for read/write */
    W_REG(ch->osh, &regs->phyaccess,
          (PHYACCESS_TRIGGER_MASK | (phyaddr << PHYACCESS_CPU_PHY_ADDR_SHIFT) | 
          (reg << PHYACCESS_CPU_REG_ADDR_SHIFT)));

    /* wait for it to complete */
    SPINWAIT((R_REG(ch->osh, &regs->phyaccess) & PHYACCESS_TRIGGER_MASK), 1000);
    tmp = R_REG(ch->osh, &regs->phyaccess);
    if (tmp & PHYACCESS_TRIGGER_MASK) {
        ET_ERROR(("et%d: chipphyrd: did not complete\n", ch->etc->unit));
        tmp = 0xffff;
    }

    /* Return to original core */
    si_setcoreidx(ch->sih, origidx);

#ifdef ROBODVT
    MII_UNLOCK();
#endif

    return (tmp & PHYACCESS_ACC_DATA_MASK);
}

static void
chipphywr(ch_t *ch, uint phyaddr, uint reg, uint16 v)
{
    uint origidx;
    gmac_commonregs_t *regs;

    ASSERT(phyaddr < MAXEPHY);
    ASSERT(reg < MAXPHYREG);

#ifdef ROBODVT
    MII_LOCK();
#else
    if (phyaddr == EPHY_NOREG)
        return;
#endif

    /* Remember original core before switch to gmac common */
    origidx = si_coreidx(ch->sih);
    regs = si_setcore(ch->sih, GMAC_COM_CORE_ID, 0);
    ASSERT(regs != NULL);

    /* set phyaccess for read/write */
    W_REG(ch->osh, &regs->phyaccess,
          (PHYACCESS_TRIGGER_MASK | PHYACCESS_WR_CMD_MASK | 
          (phyaddr << PHYACCESS_CPU_PHY_ADDR_SHIFT) | 
          (reg << PHYACCESS_CPU_REG_ADDR_SHIFT) | v));

    /* wait for it to complete */
    SPINWAIT((R_REG(ch->osh, &regs->phyaccess) & PHYACCESS_TRIGGER_MASK), 1000);
    if (R_REG(ch->osh, &regs->phyaccess) & PHYACCESS_TRIGGER_MASK) {
        ET_ERROR(("et%d: chipphywr: did not complete\n", ch->etc->unit));
    }

    /* Return to original core */
    si_setcoreidx(ch->sih, origidx);

#ifdef ROBODVT
    MII_UNLOCK();
#endif

}

static void
chipphyor(ch_t *ch, uint phyaddr, uint reg, uint16 v)
{
    uint16 tmp;

    tmp = chipphyrd(ch, phyaddr, reg);
    tmp |= v;
    chipphywr(ch, phyaddr, reg, tmp);
}

static void
chipphyreset(ch_t *ch, uint phyaddr)
{
    ASSERT(phyaddr < MAXEPHY);

    if (phyaddr == EPHY_NOREG)
        return;

    chipphywr(ch, phyaddr, 0, CTL_RESET);
    OSL_DELAY(100);
    if (chipphyrd(ch, phyaddr, 0) & CTL_RESET) {
        ET_ERROR(("et%d: chipphyreset: reset not complete\n", ch->etc->unit));
    }

    chipphyinit(ch, phyaddr);
}

static void
chipphyinit(ch_t *ch, uint phyaddr)
{
    uint16 reg_val;
    if (phyaddr == EPHY_NOREG)
        return;

    /* 
     * VO-280 : remove the advertisement of half-duplex of Voyager CSS GMAC0.
     */

    if ((ch->sih->chiprev == GMAC_VOYAGER_REVISION_ID) && 
        (ch->etc->coreunit == 0)) {
        /* Modify advertisement */
        reg_val = chipphyrd(ch, phyaddr, 4);
        reg_val &= ~(0xa0); /* Remove 100H and 10H */
        chipphywr(ch, phyaddr, 4, reg_val);
        reg_val = chipphyrd(ch, phyaddr, 9);
        reg_val &= ~(0x100); /* Remove 1000H */
        chipphywr(ch, phyaddr, 9, reg_val);
        
        /* re-autoneg */
        reg_val = chipphyrd(ch, phyaddr, 0);
        reg_val |= 0x200;
        chipphywr(ch, phyaddr, 0, reg_val);
    }

    ET_TRACE(("et%d: chipphyinit: phyaddr %d\n", ch->etc->unit, phyaddr));
}

static void
chipphyforce(ch_t *ch, uint phyaddr)
{
    etc_info_t *etc;
    uint16 ctl;

    ASSERT(phyaddr < MAXEPHY);

    if (phyaddr == EPHY_NOREG)
        return;

    etc = ch->etc;

    if (etc->forcespeed == ET_AUTO)
        return;

    ctl = chipphyrd(ch, phyaddr, 0);
    ctl &= ~(CTL_SPEED | CTL_SPEED_MSB | CTL_ANENAB | CTL_DUPLEX);

    switch (etc->forcespeed) {
        case ET_10HALF:
            break;

        case ET_10FULL:
            ctl |= CTL_DUPLEX;
            break;

        case ET_100HALF:
            ctl |= CTL_SPEED_100;
            break;

        case ET_100FULL:
            ctl |= (CTL_SPEED_100 | CTL_DUPLEX);
            break;

        case ET_1000FULL:
            ctl |= (CTL_SPEED_1000 | CTL_DUPLEX);
            break;
    }

    chipphywr(ch, phyaddr, 0, ctl);
}

/* set selected capability bits in autonegotiation advertisement */
static void
chipphyadvertise(ch_t *ch, uint phyaddr)
{
    etc_info_t *etc;
    uint16 adv, adv2;

    ASSERT(phyaddr < MAXEPHY);

    if (phyaddr == EPHY_NOREG)
        return;

    etc = ch->etc;

    if ((etc->forcespeed != ET_AUTO) || !etc->needautoneg)
        return;

    ASSERT(etc->advertise);

    /* reset our advertised capabilitity bits */
    adv = chipphyrd(ch, phyaddr, 4);
    adv &= ~(ADV_100FULL | ADV_100HALF | ADV_10FULL | ADV_10HALF);
    adv |= etc->advertise;
    chipphywr(ch, phyaddr, 4, adv);
    adv2 = chipphyrd(ch, phyaddr, 9);
    adv2 &= ~(ADV_1000FULL | ADV_1000HALF);
    adv2 |= etc->advertise2;
    chipphywr(ch, phyaddr, 9, adv2);

    ET_TRACE(("et%d: chipphyadvertise: phyaddr %d adv %x adv2 %x phyad0 %x\n",
        ch->etc->unit, phyaddr, adv, adv2, chipphyrd(ch, phyaddr, 0)));

    /* restart autonegotiation */
    chipphyor(ch, phyaddr, 0, CTL_RESTART);

    etc->needautoneg = FALSE;
}

static int
chipcfprd(ch_t *ch, void *arg)
{
    uint32 tmp, i;
    uint origidx;
    gmac_commonregs_t *regs;
    cfp_ioctl_buf_t *buf;

    ET_TRACE(("et%d: chipcfprd\n", ch->etc->unit));
    
    buf = (cfp_ioctl_buf_t *)arg;
    ASSERT(buf != NULL);

    /* Remember original core before switch to gmac common */
    origidx = si_coreidx(ch->sih);
    regs = si_setcore(ch->sih, GMAC_COM_CORE_ID, 0);
    ASSERT(regs != NULL);

    ASSERT(buf->entry_idx < CFP_TCAM_NUM);

    /* Read tcam */
    tmp = R_REG(ch->osh, &regs->cfp_access);
    tmp &= ~(CFP_ACCESS_OP_START_DONE_MASK | CFP_ACCESS_OP_SEL_MASK |
        CFP_ACCESS_RAM_SEL_MASK| CFP_ACCESS_XCESS_ADDR_MASK);
    tmp |= (buf->entry_idx << CFP_ACCESS_XCESS_ADDR_SHIFT) |
        (CFP_RAM_HW_TCAM << CFP_ACCESS_RAM_SEL_SHIFT) |
        (CFP_TCAM_OP_READ << CFP_ACCESS_OP_SEL_SHIFT) | 
        CFP_ACCESS_OP_START_DONE_MASK;
    W_REG(ch->osh, &regs->cfp_access, tmp);

    /* wait complete */
    for (i=0; i < CFP_RETRY_TIME; i++) {
        tmp = R_REG(ch->osh, &regs->cfp_access);
        if (!(tmp & CFP_ACCESS_OP_START_DONE_MASK)) {
            break;
        }
    }
    if (i >= CFP_RETRY_TIME) {
        /* Return to original core */
        si_setcoreidx(ch->sih, origidx);
        return TIMEOUT;
    }
        
    
    for (i=0; i < CFP_TCAM_ENTRY_WORD; i++) {
        buf->cfp_entry.tcam[i] = R_REG(ch->osh, (&(regs->cfp_tcam_data0) + i ));
        buf->cfp_entry.tcam_mask[i] = R_REG(ch->osh, (&(regs->cfp_tcam_mask0) + i ));
    }

    /* Read Action ram */
    tmp = R_REG(ch->osh, &regs->cfp_access);
    tmp &= ~(CFP_ACCESS_OP_START_DONE_MASK | CFP_ACCESS_OP_SEL_MASK |
        CFP_ACCESS_RAM_SEL_MASK| CFP_ACCESS_XCESS_ADDR_MASK);
    tmp |= (buf->entry_idx << CFP_ACCESS_XCESS_ADDR_SHIFT) |
        (CFP_RAM_HW_ACTION<< CFP_ACCESS_RAM_SEL_SHIFT) |
        (CFP_TCAM_OP_READ << CFP_ACCESS_OP_SEL_SHIFT) | 
        CFP_ACCESS_OP_START_DONE_MASK;
    W_REG(ch->osh, &regs->cfp_access, tmp);

    /* wait complete */
    for (i=0; i < CFP_RETRY_TIME; i++) {
        tmp = R_REG(ch->osh, &regs->cfp_access);
        if (!(tmp & CFP_ACCESS_OP_START_DONE_MASK)) {
            break;
        }
    }
    if (i >= CFP_RETRY_TIME) {
        /* Return to original core */
        si_setcoreidx(ch->sih, origidx);
        return TIMEOUT;
    }

    buf->cfp_entry.action= R_REG(ch->osl, &regs->cfp_action_data);

    /* Return to original core */
    si_setcoreidx(ch->sih, origidx);

    return SUCCESS;
}

static int
chipcfpwr(ch_t *ch, void *arg)
{
    uint32 tmp, i;
    uint origidx;
    gmac_commonregs_t *regs;
    cfp_ioctl_buf_t *buf;

    ET_TRACE(("et%d: chipcfpwr\n", ch->etc->unit));
    buf = (cfp_ioctl_buf_t *)arg;
    ASSERT(buf != NULL);

    /* Remember original core before switch to gmac common */
    origidx = si_coreidx(ch->sih);
    regs = si_setcore(ch->sih, GMAC_COM_CORE_ID, 0);
    ASSERT(regs != NULL);

    ASSERT(buf->entry_idx < CFP_TCAM_NUM);

    /* Write tcam */
    
    for (i=0; i < CFP_TCAM_ENTRY_WORD; i++) {
        W_REG(ch->osh, (&(regs->cfp_tcam_data0) + i), buf->cfp_entry.tcam[i]);
        W_REG(ch->osh, (&(regs->cfp_tcam_mask0) + i), buf->cfp_entry.tcam_mask[i]);
    }
    
    tmp = R_REG(ch->osh, &regs->cfp_access);
    tmp &= ~(CFP_ACCESS_OP_START_DONE_MASK | CFP_ACCESS_OP_SEL_MASK |
        CFP_ACCESS_RAM_SEL_MASK| CFP_ACCESS_XCESS_ADDR_MASK);
    tmp |= (buf->entry_idx << CFP_ACCESS_XCESS_ADDR_SHIFT) |
        (CFP_RAM_HW_TCAM << CFP_ACCESS_RAM_SEL_SHIFT) |
        (CFP_TCAM_OP_WRITE << CFP_ACCESS_OP_SEL_SHIFT) | 
        CFP_ACCESS_OP_START_DONE_MASK;
    W_REG(ch->osh, &regs->cfp_access, tmp);

    /* wait complete */
    for (i=0; i < CFP_RETRY_TIME; i++) {
        tmp = R_REG(ch->osh, &regs->cfp_access);
        if (!(tmp & CFP_ACCESS_OP_START_DONE_MASK)) {
            break;
        }
    }
    if (i >= CFP_RETRY_TIME) {
        /* Return to original core */
        si_setcoreidx(ch->sih, origidx);
        return TIMEOUT;
    }

    /* Write Action ram */
    W_REG(ch->osl, &regs->cfp_action_data, buf->cfp_entry.action);
    
    tmp = R_REG(ch->osh, &regs->cfp_access);
    tmp &= ~(CFP_ACCESS_OP_START_DONE_MASK | CFP_ACCESS_OP_SEL_MASK |
        CFP_ACCESS_RAM_SEL_MASK| CFP_ACCESS_XCESS_ADDR_MASK);
    tmp |= (buf->entry_idx << CFP_ACCESS_XCESS_ADDR_SHIFT) |
        (CFP_RAM_HW_ACTION<< CFP_ACCESS_RAM_SEL_SHIFT) |
        (CFP_TCAM_OP_WRITE << CFP_ACCESS_OP_SEL_SHIFT) | 
        CFP_ACCESS_OP_START_DONE_MASK;
    W_REG(ch->osh, &regs->cfp_access, tmp);

    /* wait complete */
    for (i=0; i < CFP_RETRY_TIME; i++) {
        tmp = R_REG(ch->osh, &regs->cfp_access);
        if (!(tmp & CFP_ACCESS_OP_START_DONE_MASK)) {
            break;
        }
    }
    if (i >= CFP_RETRY_TIME) {
        /* Return to original core */
        si_setcoreidx(ch->sih, origidx);
        return TIMEOUT;
    }

    /* Return to original core */
    si_setcoreidx(ch->sih, origidx);

    return SUCCESS;
}


#define GET_FIELD_REG32(r, shift, len) (r & (((0x1 << len) - 1) << shift))
#define SET_FIELD_REG32(r, shift, len, v) { \
    r &= ~(((0x1 << len) - 1) << shift); \
    r |= (v << shift); \
}

#define GET_FIELD_LEN(f)   (cfp_field_info[f].len)
#define GET_FIELD_SHIFT(f)  (cfp_field_info[f].shift) 

static cfp_field_info_t cfp_field_info[CFP_FIELD_COUNT+1] = CFP_FIELD_INFO;
static uint rx_rate_map[RX_RATE_MAX_MAP_VALUE+1] = RX_RATE_VALUE_MAPPING;

static int
chipcfpfldrd(ch_t *ch, void *arg)
{
    cfp_ioctl_buf_t *buf;
    uint32  fld_len, fld_shift, fld_base, temp;
    int     cross_32bit = 0;

    ET_TRACE(("et%d: chipcfpfldrd\n", ch->etc->unit));
    buf = (cfp_ioctl_buf_t *)arg;
    ASSERT(buf != NULL);

    /* Get the length, shift and base location of the specified field */
    fld_len = GET_FIELD_LEN(buf->field_idx);
    fld_shift = GET_FIELD_SHIFT(buf->field_idx) % 32;
    fld_base = GET_FIELD_SHIFT(buf->field_idx) / 32;

    /* Check if this field cross over 32 bit boundary */
    if ((fld_len + fld_shift) > 32) {
        cross_32bit = 1;
    }

    if (cross_32bit == 0) {
        switch (buf->ram_type) {
            case CFP_RAM_TYPE_TCAM:
                /* Get the value of field */
                buf->field_value = GET_FIELD_REG32(
                    buf->cfp_entry.tcam[fld_base], fld_shift, fld_len);
                buf->field_value = buf->field_value >> fld_shift;
                break;
            case CFP_RAM_TYPE_TCAM_MASK:
                /* Get the value of field */
                buf->field_value = GET_FIELD_REG32(
                    buf->cfp_entry.tcam_mask[fld_base], fld_shift, fld_len);
                buf->field_value = buf->field_value >> fld_shift;
                break;
            case CFP_RAM_TYPE_ACTION:
                /* Get the value of field */
                buf->field_value = GET_FIELD_REG32(
                    buf->cfp_entry.action, fld_shift, fld_len);
                buf->field_value = buf->field_value >> fld_shift;
                break;
            default:
                return FAILURE;
        }
    }else { /* cross over 32 bit boundary */
        switch (buf->ram_type) {
            case CFP_RAM_TYPE_TCAM:
                /* Get the value of in the base location */
                temp = GET_FIELD_REG32(
                    buf->cfp_entry.tcam[fld_base], fld_shift, 
                    (32 - fld_shift));
                temp = temp >> fld_shift;
                /* Get the value of in the (base+1) location */
                buf->field_value = GET_FIELD_REG32(
                    buf->cfp_entry.tcam[fld_base + 1], 0, 
                    (fld_len + fld_shift - 32));
                buf->field_value = buf->field_value << (32 - fld_shift);
                /* Combine them */
                buf->field_value |= temp;
                break;
            case CFP_RAM_TYPE_TCAM_MASK:
                /* Get the value of in the base location */
                 temp = GET_FIELD_REG32(
                    buf->cfp_entry.tcam_mask[fld_base], fld_shift, 
                    (32 - fld_shift));
                temp = temp >> fld_shift;
                /* Get the value of in the (base+1) location */
                buf->field_value = GET_FIELD_REG32(
                    buf->cfp_entry.tcam_mask[fld_base + 1], 0, 
                    (fld_len + fld_shift - 32));
                buf->field_value = buf->field_value << (32 - fld_shift);
                /* Combine them */
                buf->field_value |= temp;
                break;
            default:
                return FAILURE;
        }
    }

    return SUCCESS;
}

static int
chipcfpfldwr(ch_t *ch, void *arg)
{
    cfp_ioctl_buf_t *buf;
    uint32  fld_len, fld_shift, fld_base, temp;
    int     cross_32bit = 0;

    ET_TRACE(("et%d: chipcfpfldwr\n", ch->etc->unit));
    buf = (cfp_ioctl_buf_t *)arg;
    ASSERT(buf != NULL);


    /* Get the length, shift and base location of the specified field */
    fld_len = GET_FIELD_LEN(buf->field_idx);
    fld_shift = GET_FIELD_SHIFT(buf->field_idx) % 32;
    fld_base = GET_FIELD_SHIFT(buf->field_idx) / 32;

    /* Check if this field cross over 32 bit boundary */
    if ((fld_len + fld_shift) > 32) {
        cross_32bit = 1;
    }

    if (cross_32bit == 0) {
        switch (buf->ram_type) {
            case CFP_RAM_TYPE_TCAM:
                /* Set the value of field */
                SET_FIELD_REG32(buf->cfp_entry.tcam[fld_base], 
                    fld_shift, fld_len, buf->field_value);
                break;
            case CFP_RAM_TYPE_TCAM_MASK:
                /* Set the value of field */
                SET_FIELD_REG32(buf->cfp_entry.tcam_mask[fld_base], 
                    fld_shift, fld_len, buf->field_value);
                break;
            case CFP_RAM_TYPE_ACTION:
                /* Set the value of field */
                SET_FIELD_REG32(buf->cfp_entry.action, 
                    fld_shift, fld_len, buf->field_value);
                break;
            default:
                return FAILURE;
        }   
    } else { /* cross over 32-bit bounary */
        switch (buf->ram_type) {
            case CFP_RAM_TYPE_TCAM:
                /* Set the value of field in base location */
                temp = ((0x1 << (32 - fld_shift)) - 1) & buf->field_value;
                SET_FIELD_REG32(buf->cfp_entry.tcam[fld_base], 
                    fld_shift, (32 - fld_shift), temp);
                /* Set the value of field in (base +1) location */
                temp = buf->field_value >> (32 - fld_shift);
                SET_FIELD_REG32(buf->cfp_entry.tcam[fld_base + 1], 
                    0, (fld_len + fld_shift - 32), 
                    temp);
                break;
            case CFP_RAM_TYPE_TCAM_MASK:
                /* Set the value of field in base location */
                temp = ((0x1 << (32 - fld_shift)) - 1) & buf->field_value;
                SET_FIELD_REG32(buf->cfp_entry.tcam_mask[fld_base], 
                    fld_shift, (32 - fld_shift), temp);
                /* Set the value of field in (base + 1) location */
                temp = buf->field_value >> (32 - fld_shift);
                SET_FIELD_REG32(buf->cfp_entry.tcam_mask[fld_base + 1], 
                    0, (fld_len + fld_shift - 32), 
                    temp);
                break;
            default:
                return FAILURE;
        }   
    }

    return SUCCESS;
}

static int
chipcfpudfrd(ch_t *ch, void *arg)
{
    uint32 tmp, shift;
    uint origidx;
    gmac_commonregs_t *regs;
    cfp_ioctl_buf_t *buf;

    ET_TRACE(("et%d: chipcfpudfrd\n", ch->etc->unit));
    buf = (cfp_ioctl_buf_t *)arg;
    ASSERT(buf != NULL);

    /* Remember original core before switch to gmac common */
    origidx = si_coreidx(ch->sih);
    regs = si_setcore(ch->sih, GMAC_COM_CORE_ID, 0);
    ASSERT(regs != NULL);

    /* Check L3 Framing and slice ID */
    switch (buf->l3_framing) {
        case CFP_L3_FRAMING_IPV4:
            if (buf->slice_id > CFP_IPV4_MAX_SLICE_ID) {
                /* Return to original core */
                si_setcoreidx(ch->sih, origidx);
                return FAILURE;
            }
            tmp = R_REG(ch->osh, &regs->udf_0_a3_a0 + 
                (buf->slice_id * 4) + (buf->field_idx /4));
            shift = (buf->field_idx % 4) * 8;
            buf->field_value = (tmp >> shift) & CFP_UDF_OFFSET_BASE_VALUE_MASK;
            buf->flags = (tmp >> (shift + CFP_UDF_OFFSET_BASE_FLAGS_SHIFT)) 
                & CFP_UDF_OFFSET_BASE_FLAGS_MASK;
            break;
        case CFP_L3_FRAMING_IPV6:
             /* slice id 3 means Chain slice */
            if (buf->slice_id > CFP_IPV6_MAX_SLICE_ID) {
                /* Return to original core */
                si_setcoreidx(ch->sih, origidx);
                return FAILURE;
            }
            if (buf->slice_id == CFP_IPV6_MAX_SLICE_ID) {
                tmp = R_REG(ch->osh, &regs->udf_0_d3_d0 + 
                    (buf->field_idx /4));
            } else {
                tmp = R_REG(ch->osh, &regs->udf_0_b3_b0 + 
                    (buf->slice_id * 4) + (buf->field_idx /4));
            }
            shift = (buf->field_idx % 4) * 8;
            buf->field_value = (tmp >> shift) & CFP_UDF_OFFSET_BASE_VALUE_MASK;
            buf->flags = (tmp >> (shift + CFP_UDF_OFFSET_BASE_FLAGS_SHIFT)) 
                & CFP_UDF_OFFSET_BASE_FLAGS_MASK;
            break;
        case CFP_L3_FRAMING_NONIP:
            if (buf->slice_id > CFP_NONIP_MAX_SLICE_ID) {
                /* Return to original core */
                si_setcoreidx(ch->sih, origidx);
                return FAILURE;
            }
            tmp = R_REG(ch->osh, &regs->udf_0_c3_c0 + 
                (buf->slice_id * 4) + (buf->field_idx /4));
            shift = (buf->field_idx % 4) * 8;
            buf->field_value = (tmp >> shift) & CFP_UDF_OFFSET_BASE_VALUE_MASK;
            buf->flags = (tmp >> (shift + CFP_UDF_OFFSET_BASE_FLAGS_SHIFT)) 
                & CFP_UDF_OFFSET_BASE_FLAGS_MASK;
            break;
        default:
            /* Return to original core */
            si_setcoreidx(ch->sih, origidx);
            return FAILURE;
    }
    /* Return to original core */
    si_setcoreidx(ch->sih, origidx);

    return SUCCESS;
}

static int
chipcfpudfwr(ch_t *ch, void *arg)
{
    uint32 tmp, shift, mask;
    uint origidx;
    gmac_commonregs_t *regs;
    cfp_ioctl_buf_t *buf;

    ET_TRACE(("et%d: chipcfpudfwr\n", ch->etc->unit));
    buf = (cfp_ioctl_buf_t *)arg;
    ASSERT(buf != NULL);

    /* Remember original core before switch to gmac common */
    origidx = si_coreidx(ch->sih);
    regs = si_setcore(ch->sih, GMAC_COM_CORE_ID, 0);
    ASSERT(regs != NULL);

    /* Check L3 Framing and slice ID */
    switch (buf->l3_framing) {
        case CFP_L3_FRAMING_IPV4:
            if (buf->slice_id > CFP_IPV4_MAX_SLICE_ID) {
                /* Return to original core */
                si_setcoreidx(ch->sih, origidx);
                return FAILURE;
            }
            tmp = R_REG(ch->osh, &regs->udf_0_a3_a0 + 
                (buf->slice_id * 4) + (buf->field_idx /4));
            shift = (buf->field_idx % 4) * 8;
            mask = UDF_0_A3_A0_CFG_UDF_0_A0_MASK << shift;
            tmp &= ~mask;
            tmp |= (((buf->field_value/2) | buf->flags) & UDF_0_A3_A0_CFG_UDF_0_A0_MASK) << shift;
            W_REG(ch->osh, &regs->udf_0_a3_a0 + 
                (buf->slice_id * 4) + (buf->field_idx /4), tmp);
            break;
        case CFP_L3_FRAMING_IPV6:
            /* slice id 3 means Chain slice */
            if (buf->slice_id > CFP_IPV6_MAX_SLICE_ID) { 
                /* Return to original core */
                si_setcoreidx(ch->sih, origidx);
                return FAILURE;
            }
            if (buf->slice_id == CFP_IPV6_MAX_SLICE_ID) {
                tmp = R_REG(ch->osh, &regs->udf_0_d3_d0 + 
                    (buf->slice_id * 4) + (buf->field_idx /4));
            } else {
                tmp = R_REG(ch->osh, &regs->udf_0_b3_b0 + 
                    (buf->slice_id * 4) + (buf->field_idx /4));
            }
            shift = (buf->field_idx % 4) * 8;
            mask = 0xff << shift;
            tmp &= ~mask;
            tmp |= (((buf->field_value/2) | buf->flags) & 0xff) << shift;
            if (buf->slice_id == CFP_IPV6_MAX_SLICE_ID) {
                W_REG(ch->osh, &regs->udf_0_d3_d0 + 
                    (buf->slice_id * 4) + (buf->field_idx /4), tmp);
            } else {
                W_REG(ch->osh, &regs->udf_0_b3_b0 + 
                    (buf->slice_id * 4) + (buf->field_idx /4), tmp);
            }
            break;
        case CFP_L3_FRAMING_NONIP:
            if (buf->slice_id > CFP_NONIP_MAX_SLICE_ID) {
                /* Return to original core */
                si_setcoreidx(ch->sih, origidx);
                return FAILURE;
            }
            tmp = R_REG(ch->osh, &regs->udf_0_c3_c0 + 
                (buf->slice_id * 4) + (buf->field_idx /4));
            shift = (buf->field_idx % 4) * 8;
            mask = 0xff << shift;
            tmp &= ~mask;
            tmp |= (((buf->field_value/2) | buf->flags) & 0xff) << shift;
            W_REG(ch->osh, &regs->udf_0_c3_c0 + 
                (buf->slice_id * 4) + (buf->field_idx /4), tmp);
            break;
        default:
            /* Return to original core */
            si_setcoreidx(ch->sih, origidx);
            return FAILURE;
    }
    /* Return to original core */
    si_setcoreidx(ch->sih, origidx);

    return SUCCESS;
}

static uint chiprxratemapping(uint pps)
{
    uint i;
    for (i = 0; i <= RX_RATE_MAX_MAP_VALUE; i++) {
        if (pps <= rx_rate_map[i]) {
            return i;
        }
    }

    /* return the max value */
    return RX_RATE_MAX_MAP_VALUE;
}

static int chiprxrateset(ch_t *ch, uint channel, uint pps)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint reg_val, tmp;
    uint map_val;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chiprxrateset\n", etc->unit));
    if (channel >= NUMRXQ) {
        return FAILURE;
    }

    /* Get the PPS of the specific channel from the register */ 
    reg_val = R_REG(ch->osh, &regs->irc_cfg);
    tmp = (reg_val >> (channel * IRC_CFG_IRC1_IDX_SHIFT)) & 
        IRC_CFG_IRC0_IDX_MASK;

    map_val = chiprxratemapping(pps);

    /* Configure the new PPS value */
    if (tmp != map_val) {
        reg_val &= ~(IRC_CFG_IRC0_IDX_MASK << 
            (channel * IRC_CFG_IRC1_IDX_SHIFT));
        reg_val |= ((map_val & IRC_CFG_IRC0_IDX_MASK) << 
            (channel * IRC_CFG_IRC1_IDX_SHIFT));
        W_REG(ch->osh, &regs->irc_cfg, reg_val);
    }

    return SUCCESS;
}

static int chiprxrateget(ch_t *ch, uint channel, uint *pps)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint reg_val, tmp;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chiprxrateget\n", etc->unit));
    if (channel >= NUMRXQ) {
        return FAILURE;
    }

    /* Get the PPS of the specific channel from the register */ 
    reg_val = R_REG(ch->osh, &regs->irc_cfg);
    tmp = (reg_val >> (channel * IRC_CFG_IRC1_IDX_SHIFT)) & 
        IRC_CFG_IRC0_IDX_MASK;

    *pps = rx_rate_map[tmp];

    return SUCCESS;
}

static int chiptxrateset(ch_t *ch, uint channel, uint rate, uint burst)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val, tmp;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chiptxrateset\n", etc->unit));
    if (channel >= NUMTXQ) {
        return FAILURE;
    }

    /* Get the TX rate value from chip */
    reg_val = R_REG(ch->osh, &regs->erc0_cfg + channel);
    if (rate){
        /* Set enable bit */
        reg_val |= ERC0_CFG_ENABLE_MASK;
        /* Set refres count */
        tmp = (rate + (TX_RATE_VALUE_BASE -1)) / TX_RATE_VALUE_BASE;
        reg_val &= ~ERC1_CFG_RFSHCNT_MASK;
        reg_val |= tmp << ERC1_CFG_RFSHCNT_SHIFT;

        /* Set burst size */
        tmp = (burst + (TX_RATE_BURST_BASE - 1)) / TX_RATE_BURST_BASE;
        reg_val &= ~ERC1_CFG_BKTSIZE_MASK;
        reg_val |= tmp << ERC1_CFG_BKTSIZE_SHIFT;
    } else {
        /* Disable Egress rate */
        reg_val &= ~ERC0_CFG_ENABLE_MASK;
    }
    W_REG(ch->osh, &regs->erc0_cfg + channel, reg_val);

    return SUCCESS;
}

static int chiptxrateget(ch_t *ch, uint channel, uint *rate, uint *burst)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val, tmp;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chiptxrateget\n", etc->unit));
    if (channel >= NUMTXQ) {
        return FAILURE;
    }

    /* Get the TX rate value from chip */
    reg_val = R_REG(ch->osh, &regs->erc0_cfg + channel);
    if (reg_val & ERC0_CFG_ENABLE_MASK) {
        /* Get the refresh count value */
        tmp = (reg_val & ERC1_CFG_RFSHCNT_MASK) >> ERC1_CFG_RFSHCNT_SHIFT;
        *rate = tmp * TX_RATE_VALUE_BASE;
        /* Get the bucket size */
         tmp = (reg_val & ERC1_CFG_BKTSIZE_MASK) >> ERC1_CFG_BKTSIZE_SHIFT;
        * burst = tmp * TX_RATE_BURST_BASE;
    } else {
        /* Egress rate control wasn't enabled */
        *rate = 0;
        *burst = 0;
    }

    return SUCCESS;
}

static int chipflowctrlmodeset(ch_t *ch, uint mode)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chipflowctrlmodeset\n", etc->unit));

    reg_val = R_REG(ch->osh, &regs->devcontrol);
    reg_val &= ~DEVCONTROL_FLOW_CTRL_MODE_MASK;
    switch (mode) {
        case FLOW_CTRL_MODE_DISABLE:
            gmac_txflowcontrol(ch, FALSE);
            return SUCCESS;
        case FLOW_CTRL_MODE_AUTO:
            reg_val |= 0x0 << DEVCONTROL_FLOW_CTRL_MODE_SHIFT;
            break;
        case FLOW_CTRL_MODE_CPU:
            reg_val |= 0x1 << DEVCONTROL_FLOW_CTRL_MODE_SHIFT;
            break;
        case FLOW_CTRL_MODE_MIX:
            reg_val |= 0x2 << DEVCONTROL_FLOW_CTRL_MODE_SHIFT;
            break;
        default:
            return FAILURE;
    }
    gmac_txflowcontrol(ch, TRUE);
    W_REG(ch->osh, &regs->devcontrol, reg_val);

    return SUCCESS;
}

static int chipflowctrlmodeget(ch_t *ch, uint *mode)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val;
    uint32 cmdcfg;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chipflowctrlmodeget\n", etc->unit));

    cmdcfg = R_REG(ch->osh, &ch->regs->command_config);
    if (cmdcfg & COMMAND_CONFIG_PAUSE_IGNORE_MASK) {
        *mode = FLOW_CTRL_MODE_DISABLE;
        return SUCCESS;
    }

    reg_val = R_REG(ch->osh, &regs->devcontrol);
    switch ((reg_val & DEVCONTROL_FLOW_CTRL_MODE_MASK) >>
        DEVCONTROL_FLOW_CTRL_MODE_SHIFT) {
        case 0:
            *mode = FLOW_CTRL_MODE_AUTO;
            break;
        case 1:
            *mode = FLOW_CTRL_MODE_CPU;
            break;
        case 2:
            *mode = FLOW_CTRL_MODE_MIX;
            break;
        default:
            return FAILURE;
    }

    return SUCCESS;
}

static int chipflowctrlautoset(ch_t *ch, uint on_th, uint off_th)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chipflowctrlautoset\n", etc->unit));

    if ((on_th > 0xfff) || (off_th > 0xfff)) {
        return FAILURE;
    }

    /* Set the value of Pause on/off threashod */
    reg_val = R_REG(ch->osh, &regs->flowctlthresh);
    reg_val &= ~FLOWCTLTHRESH_ON_THRESH_MASK;
    reg_val |= on_th << FLOWCTLTHRESH_ON_THRESH_SHIFT;
    reg_val &= ~FLOWCTLTHRESH_OFF_THRESH_MASK;
    reg_val |= off_th << FLOWCTLTHRESH_OFF_THRESH_SHIFT;
    W_REG(ch->osh, &regs->flowctlthresh, reg_val);

    return SUCCESS;
}

static int chipflowctrlautoget(ch_t *ch, uint *on_th, uint *off_th)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chipflowctrlautoget\n", etc->unit));

    /* Get the value of Pause on/off threashod */
    reg_val = R_REG(ch->osh, &regs->flowctlthresh);
    *on_th = (reg_val & FLOWCTLTHRESH_ON_THRESH_MASK) >> 
        FLOWCTLTHRESH_ON_THRESH_SHIFT;
    *off_th = (reg_val & FLOWCTLTHRESH_OFF_THRESH_MASK) >> 
        FLOWCTLTHRESH_OFF_THRESH_SHIFT;

    return SUCCESS;
}

static int chipflowctrlcpuset(ch_t *ch, uint pause_on)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chipflowctrlcpuset\n", etc->unit));

    /* Set the pause on value */
    reg_val = R_REG(ch->osh, &regs->devcontrol);
    reg_val &= ~DEVCONTROL_CPU_FLOW_CTRL_ON_MASK;
    if (pause_on) {
        reg_val |= 1 << DEVCONTROL_CPU_FLOW_CTRL_ON_SHIFT;
    }
    W_REG(ch->osh, &regs->devcontrol, reg_val);

    return SUCCESS;
}

static int chipflowctrlcpuget(ch_t *ch, uint *pause_on)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chipflowctrlcpuget\n", etc->unit));

    /* Get the pause on value */
    reg_val = R_REG(ch->osh, &regs->devcontrol);
    *pause_on = (reg_val & DEVCONTROL_CPU_FLOW_CTRL_ON_MASK) >> 
        DEVCONTROL_CPU_FLOW_CTRL_ON_SHIFT;;

    return SUCCESS;
}

static int chipflowctrlrxchanset(ch_t *ch, uint chan, uint on_th, uint off_th)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chipflowctrlrxchanset\n", etc->unit));

    if (chan >= NUMRXQ) {
        return FAILURE;
    }

    /* Value Checking */ 
    /* 
     * The pause off should be less than the value (NRXBUFPOST - 1).
     * Otherwise, the pause frame will keep sending occasionally.
     */
    if ((on_th > NRXBUFPOST) || (off_th >=  (NRXBUFPOST - 1)) ||
        (on_th > off_th)) {
        return FAILURE;
    }

    /* Set the pause on/off threshold of specific channel */
    reg_val = R_REG(ch->osh, &regs->rx_ch0_flow_ctrl + chan);
    reg_val &= ~RX_CH0_FLOW_CTRL_FLOWCNTLONTH_MASK;
    reg_val |= on_th << RX_CH0_FLOW_CTRL_FLOWCNTLONTH_SHIFT;
    reg_val &= ~RX_CH0_FLOW_CTRL_FLOWCNTLOFFTH_MASK;
    reg_val |= off_th << RX_CH0_FLOW_CTRL_FLOWCNTLOFFTH_SHIFT;
    W_REG(ch->osh, &regs->rx_ch0_flow_ctrl + chan, reg_val);

    return SUCCESS;
}

static int chipflowctrlrxchanget(ch_t *ch, uint chan, uint *on_th, uint *off_th)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chipflowctrlrxchanget\n", etc->unit));

    if (chan >= NUMRXQ) {
        return FAILURE;
    }

    /* Get the pause on/off threshold of specific channel */
    reg_val = R_REG(ch->osh, &regs->rx_ch0_flow_ctrl + chan);
    *on_th = (reg_val & RX_CH0_FLOW_CTRL_FLOWCNTLONTH_MASK) >>
        RX_CH0_FLOW_CTRL_FLOWCNTLONTH_SHIFT;
    *off_th = (reg_val & RX_CH0_FLOW_CTRL_FLOWCNTLOFFTH_MASK) >>
        RX_CH0_FLOW_CTRL_FLOWCNTLOFFTH_SHIFT;

    return SUCCESS;
}

static int
chiptpidset(ch_t *ch, uint index, uint tpid)
{
    uint32 reg_val;
    uint origidx;
    gmac_commonregs_t *regs;

    ET_TRACE(("et%d: chiptpidset\n", ch->etc->unit));

    /* Remember original core before switch to gmac common */
    origidx = si_coreidx(ch->sih);
    regs = si_setcore(ch->sih, GMAC_COM_CORE_ID, 0);
    ASSERT(regs != NULL);

    if (index >= NUM_STAG_TPID) {
        return FAILURE;
    }

    /* Set TPID */
    reg_val = R_REG(ch->osh, &regs->stag0 + index);
    reg_val &= ~STAG0_TPID_MASK;
    reg_val |= tpid & STAG0_TPID_MASK;
    W_REG(ch->osh, &regs->stag0 + index, reg_val);

    /* Return to original core */
    si_setcoreidx(ch->sih, origidx);

    return SUCCESS;
}

static int
chiptpidget(ch_t *ch, uint index, uint *tpid)
{
    uint32 reg_val;
    uint origidx;
    gmac_commonregs_t *regs;

    ET_TRACE(("et%d: chiptpidget\n", ch->etc->unit));

    /* Remember original core before switch to gmac common */
    origidx = si_coreidx(ch->sih);
    regs = si_setcore(ch->sih, GMAC_COM_CORE_ID, 0);
    ASSERT(regs != NULL);

    if (index >= NUM_STAG_TPID) {
        si_setcoreidx(ch->sih, origidx);
        return FAILURE;
    }

    /* Get TPID */
    reg_val = R_REG(ch->osh, &regs->stag0 + index);
    *tpid = reg_val & STAG0_TPID_MASK;

    /* Return to original core */
    si_setcoreidx(ch->sih, origidx);
    return SUCCESS;
}

static int chippvtagset(ch_t *ch, uint private_tag)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chippvtagset\n", etc->unit));

    if (private_tag > MAX_PRIVATE_TAG_VALUE) {
        return FAILURE;
    }

    reg_val = R_REG(ch->osh, &regs->devcontrol);
    reg_val &= ~DEVCONTROL_PV_TAG_MASK;
    reg_val |= private_tag << DEVCONTROL_PV_TAG_SHIFT;
    W_REG(ch->osh, &regs->devcontrol, reg_val);

    return SUCCESS;
}

static int chippvtagget(ch_t *ch, uint *private_tag)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chippvtagget\n", etc->unit));

    reg_val = R_REG(ch->osh, &regs->devcontrol);
    *private_tag = (reg_val & DEVCONTROL_PV_TAG_MASK) >> 
        DEVCONTROL_PV_TAG_SHIFT;

    return SUCCESS;
}

static int chiprxsephdrset(ch_t *ch, uint enable)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val, field_val = 0;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chiprxsephdrset\n", etc->unit));

    if (enable) {
        field_val = 1;
    }

    reg_val = R_REG(ch->osh, &regs->d64rcv0control);
    reg_val &= ~D64RCV0CONTROL_SEPDESCRHDREN_MASK;
    reg_val |= field_val << D64RCV0CONTROL_SEPDESCRHDREN_SHIFT;
    W_REG(ch->osh, &regs->d64rcv0control, reg_val);

    return SUCCESS;
}

static int chiprxsephdrget(ch_t *ch, uint *enable)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chiprxsephdrget\n", etc->unit));

    reg_val = R_REG(ch->osh, &regs->devcontrol);
    *enable = (reg_val & D64RCV0CONTROL_SEPDESCRHDREN_MASK) >> 
        D64RCV0CONTROL_SEPDESCRHDREN_SHIFT;

    return SUCCESS;
}

static int chiptxqosmodeset(ch_t *ch, uint mode)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chiptxqosmodeset\n", etc->unit));

    reg_val = R_REG(ch->osh, &regs->txqos);
    reg_val &= ~TXQOS_TXQOS_POLICY_MASK;
    switch (mode) {
        case TXQOS_MODE_1STRICT_3WRR:
            reg_val |= 0x0 << TXQOS_TXQOS_POLICY_SHIFT;
            break;
        case TXQOS_MODE_2STRICT_2WRR:
            reg_val |= 0x1 << TXQOS_TXQOS_POLICY_SHIFT;
            break;
        case TXQOS_MODE_3STRICT_1WRR:
            reg_val |= 0x2 << TXQOS_TXQOS_POLICY_SHIFT;
            break;
        case TXQOS_MODE_ALL_STRICT:
            reg_val |= 0x3 << TXQOS_TXQOS_POLICY_SHIFT;
            break;
        default:
            return FAILURE;
    }
    W_REG(ch->osh, &regs->txqos, reg_val);

    return SUCCESS;
}

static int chiptxqosmodeget(ch_t *ch, uint *mode)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chipflowctrlmodeget\n", etc->unit));

    reg_val = R_REG(ch->osh, &regs->txqos);
    *mode = (reg_val & TXQOS_TXQOS_POLICY_MASK) >> 
        TXQOS_TXQOS_POLICY_SHIFT;

    return SUCCESS;
}

static int chiptxqosweightset(ch_t *ch, uint queue, uint weight)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chiptxqosweightset\n", etc->unit));

    if (queue >= NUMTXQ) {
        return FAILURE;
    }

    /* Value Checking */ 
    if (weight > 0xff) {
        return FAILURE;
    }

    /* highest queue always with strict mode */
    if (queue == TX_Q3) {
        return FAILURE;
    }

    /* Set the pause on/off threshold of specific channel */
    reg_val = R_REG(ch->osh, &regs->txqos);
    reg_val &= ~(TXQOS_TXQOS_WEIGHT0_MASK << 
        (queue * TXQOS_TXQOS_WEIGHT1_SHIFT));
    reg_val |= weight << (queue * TXQOS_TXQOS_WEIGHT1_SHIFT);
    W_REG(ch->osh, &regs->txqos, reg_val);

    return SUCCESS;
}

static int chiptxqosweightget(ch_t *ch, uint queue, uint *weight)
{
    etc_info_t *etc;
    gmac0regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    ET_TRACE(("et%d: chiptxqosweightget\n", etc->unit));

    if (queue >= NUMTXQ) {
        return FAILURE;
    }

    /* highest queue always with strict mode */
    if (queue == TX_Q3) {
        return FAILURE;
    }

    /* Get the pause on/off threshold of specific channel */
    reg_val = R_REG(ch->osh, &regs->txqos);
    *weight = (reg_val >> (queue * TXQOS_TXQOS_WEIGHT1_SHIFT)) 
        && TXQOS_TXQOS_WEIGHT0_MASK;

    return SUCCESS;
}


#ifdef CFG_ROBO
static void
robo_fege_port_enable(int handle, int ge_num)
{
    int fh = handle;
    uint16_t miiaddr;
    uint8_t miireg;
    int i;
    
    /* reset chip */
    miiaddr = 0x37c;
    miireg = 0x1;

    cfe_writeblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));

    /* Wait for chip reset complete */
    for (i=0; i<10000000; i++) {
        miireg = 0;
        cfe_readblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));
        if (!(miireg & 0x1)) {
            /* Reset is complete */
            break;
        }
    }
    miiaddr = 0x140;
    /* MII port state override (page 1 register 40) */
    cfe_readblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));
    /* Disable IMP port link before doing change in 1Q vlan, mstp
     * and management mode
     */
    miireg = 0x0;
    cfe_writeblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));

    /*
     * Make sure 802.1Q VLAN, MSTP and management mode are bothe disabled
     * The system may be reset from runtime...
     */
    miireg = 0;
    /* Disable management mode */
    cfe_writeblk(fh, 0x0300, PTR2HSADDR(&miireg), sizeof(miireg));
    /* Disable 802.1Q VLAN*/
    cfe_writeblk(fh, 0x3400, PTR2HSADDR(&miireg), sizeof(miireg));
    /* Disable MSTP */
    cfe_writeblk(fh, 0x4500, PTR2HSADDR(&miireg), sizeof(miireg));

    miireg = 0x47;

    /* Enable the IMP port link and speed duplex for in-band networking */
    cfe_writeblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));

    miireg = 0;
    /* Read back */
    cfe_readblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));
    if ((miireg & 0x47) != 0x47) {
        printf("robo_rvmii: error enabling mode");
    }

}

static void
bcm5324_port_enable(int handle, int ge_num)
{
    int fh = handle;
    uint16_t miiaddr;
    uint8_t miireg;
    int i;

    /* Check if 5324 chip */
        /* 5324 chip */
    /* do not override port status*/
    for (i = 0; i < 24; i++) {
        /* fe port*/
        miiaddr = 0xa0 + i;
        miireg = 0;
        cfe_writeblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));
    }
    for (i = 0; i < 2; i++) {
        /* ge port*/
        miiaddr = 0xb9 + i;
        miireg = 0;
        cfe_writeblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));
    }   
    miiaddr = 0xb8;

    /* MII port state override (page 0 register 14) */
    cfe_readblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));
    /* Disable IMP port link before doing change in 1Q vlan, mstp
     * and management mode
     */
    miireg &= 0xF0;
    miireg |= 0x80;
    cfe_writeblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));

    /*
     * Make sure 802.1Q VLAN, MSTP and management mode are bothe disabled
     * The system may be reset from runtime...
     */
    miireg = 0;
    /* Disable management mode */
    cfe_writeblk(fh, 0x0200, PTR2HSADDR(&miireg), sizeof(miireg));
    /* Disable 802.1Q VLAN*/
    cfe_writeblk(fh, 0x3400, PTR2HSADDR(&miireg), sizeof(miireg));
    /* Disable MSTP */
    cfe_writeblk(fh, 0x4300, PTR2HSADDR(&miireg), sizeof(miireg));

    /* Enable the IMP port link and speed duplex for in-band networking */
    cfe_readblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));
    miireg |= 0x87;
    cfe_writeblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));

    miireg = 0;
    /* Read back */
    cfe_readblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));

    if ((miireg & 0x87) != 0x87) {
        printf("robo_rvmii: error enabling mode");
    }

}

static void
bcm5398_port_enable(int handle, int ge_num)
{
    int fh = handle;
    uint16_t miiaddr;
    uint8_t miireg;
    int i;


    for (i = 0; i < ge_num; i++) {
        /* ge port*/
        miiaddr = 0x58 + i;
        miireg = 0;
        cfe_writeblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));
    }
    miiaddr = 0xe;

    /* MII port state override (page 0 register 14) */
    cfe_readblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));
    /* Disable IMP port link before doing change in 1Q vlan, mstp
     * and management mode
     */
    miireg &= 0xF0;
    miireg |= 0x80;
    cfe_writeblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));

    /*
     * Make sure 802.1Q VLAN, MSTP and management mode are bothe disabled
     * The system may be reset from runtime...
     */
    miireg = 0;
    /* Disable management mode */
    cfe_writeblk(fh, 0x0200, PTR2HSADDR(&miireg), sizeof(miireg));
    /* Disable 802.1Q VLAN*/
    cfe_writeblk(fh, 0x3400, PTR2HSADDR(&miireg), sizeof(miireg));
    /* Disable MSTP */
    cfe_writeblk(fh, 0x4300, PTR2HSADDR(&miireg), sizeof(miireg));

    /* Enable the IMP port link and speed duplex for in-band networking */
    cfe_readblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));
    miireg |= 0x87;
    cfe_writeblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));

    miireg = 0;
    /* Read back */
    cfe_readblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));

    if ((miireg & 0x87) != 0x87) {
        printf("robo_rvmii: error enabling mode");
    }

}

static void
robo_ge_port_enable(int handle, int ge_num)
{
    int fh = handle;
    uint16_t miiaddr;
    uint8_t miireg;
    int i;

    /* bcm53115 chip */
    
    /* force IMP port link down before all configuration is done. */
    miiaddr = 0x000e;
    miireg = 0x86;
    cfe_writeblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));

    /* set to Management mode */
    miiaddr = 0x000b;
    miireg = 0x7;
    cfe_writeblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));
    
    /* enable IMP port */
    miiaddr = 0x0200;
    miireg = 0x82;
    cfe_writeblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));
    
    /* disable BRCM header */
    miiaddr = 0x0203;
    miireg = 0x0;
    cfe_writeblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));
    
    /* GE port setting */
    for (i = 0; i < ge_num; i++){
        /* clear stateoverride */
        miiaddr = 0x0058 + i;
        cfe_readblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));
        miireg &= ~0x40;
        cfe_writeblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));
        
        /* set forwarding state and enable tx/rx */
        miiaddr = 0x0 + i;
        miireg = 0xa0;
        cfe_writeblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));
    }
   
    /* force IMP port to link at 100FD for in-band connectivity */
    miiaddr = 0x000e;
    miireg = 0x87;
    cfe_writeblk(fh, miiaddr, PTR2HSADDR(&miireg), sizeof(miireg));

}

static void
robo_53242_misc_init(int handle)
{
    int fh = handle;
    uint16_t regaddr;
    uint16_t ledfun;
    uint8_t led_map[8];
    
    
    /* Configure the LED function for BCM53242 */
    regaddr = 0x5c;
    ledfun = 0x120;
#if ENDIAN_BIG
    ledfun = SWAP16(ledfun);
#endif
    cfe_writeblk(fh, regaddr, PTR2HSADDR(&ledfun), sizeof(ledfun));

    regaddr = 0x5e;
    ledfun = 0x1a0;
#if ENDIAN_BIG
    ledfun = SWAP16(ledfun);
#endif    
    cfe_writeblk(fh, regaddr, PTR2HSADDR(&ledfun), sizeof(ledfun));

    regaddr = 0x60;
    bzero(&led_map[0], 8);
    led_map[6] = 0x06;
    cfe_writeblk(fh, regaddr, PTR2HSADDR(&led_map[0]), 8);
    
    return;
}

static void
_robo_port_stop(int handle, uint16_t mii_reg)
{
     int fh = handle;
     uint8_t value;
     
    cfe_readblk(fh, mii_reg, PTR2HSADDR(&value), sizeof(value));
    value &= 0xfe; /* force IMP port link down*/
    cfe_writeblk(fh, mii_reg, PTR2HSADDR(&value), sizeof(value));
}

typedef struct chip_info_s {
    char *name;
    uint16_t    phyidh;
    uint16_t    phyidl;
    uint16_t    ge_num;
    void        (*port_enable)(int handle, int ge_num);
    void        (*misc_init)(int handle);
    uint16_t    mii_reg; /* IMP override reg */
}chip_info_t;    

static chip_info_t _robo_chip_info[] = {
    { "BCM5324",
       0x0143,
       0xbc20,
       2,
       bcm5324_port_enable,
       NULL,
       0x00b8
    },
    { "BCM5398",
        0x0143,
       0xbcd0,
       8, 
       bcm5398_port_enable,
       NULL,
       0x000e
    },
    { "BCM5348/47",
       0x0143,
       0xbe40,
       4,
       robo_fege_port_enable,
       NULL,
       0x0140
    },
    { "BCM5395",
       0x0143,
       0xbcf0,
       5,  
       robo_ge_port_enable,
       NULL,
       0x000e
    },
    { "BCM53242",
       0x0143,
       0xbf10,
       2,
       robo_fege_port_enable,
       robo_53242_misc_init,
       0x0140
    },
    { "BCM53262",
       0x0143,
       0xbf20,
       2,
       robo_fege_port_enable,
       NULL,
       0x0140
    },
    { "BCM53115",
       0x0143,
       0xbf80,
       6, 
       robo_ge_port_enable,
       NULL,
       0x000e       
    },
    { "BCM53118",
       0x0143,
       0xbfe0,
       8, 
       robo_ge_port_enable,
       NULL,
       0x000e
    },
    { NULL, -1  }
};


static void
robo_port_enable(ch_t *ch)
{
    int fh;
    chip_info_t *chip;
    uint16_t phyidl, phyidh;

    /* For now, robo0 is the default for spi channel */
    if ((fh = cfe_open("robo0")) < 0) {
    printf("cfe_open robo0 failed\n");
    return;
    }

    cfe_readblk(fh, ((0x10 << 8) | (0x4 << 0)),
        PTR2HSADDR(&phyidh), sizeof(phyidh));
    cfe_readblk(fh, ((0x10 << 8) | (0x6 << 0)),
        PTR2HSADDR(&phyidl), sizeof(phyidl));

#if ENDIAN_BIG
    phyidh =  SWAP16(phyidh);
    phyidl =  SWAP16(phyidl);
#endif

    chip = _robo_chip_info;

    while(chip->name){
        if ((phyidh == chip->phyidh ) && ((phyidl & 0xfff0) == chip->phyidl)){
            chip->port_enable(fh, chip->ge_num);
            if (chip->misc_init) {
                chip->misc_init(fh);
            }
            break;
        }
        chip ++;
    }
        
    cfe_close(fh);

}

static void
robo_port_stop(ch_t *ch)
{
    int fh;
    chip_info_t *chip;
    uint16_t phyidl, phyidh;

    /* For now, robo0 is the default for spi channel */
    if ((fh = cfe_open("robo0")) < 0) {
    printf("cfe_open robo0 failed\n");
    return;
    }

    cfe_readblk(fh, ((0x10 << 8) | (0x4 << 0)),
        PTR2HSADDR(&phyidh), sizeof(phyidh));
    cfe_readblk(fh, ((0x10 << 8) | (0x6 << 0)),
        PTR2HSADDR(&phyidl), sizeof(phyidl));

#if ENDIAN_BIG
    phyidh =  SWAP16(phyidh);
    phyidl =  SWAP16(phyidl);
#endif

    chip = _robo_chip_info;

    while(chip->name){
        if ((phyidh == chip->phyidh ) && ((phyidl & 0xfff0) == chip->phyidl)){
            _robo_port_stop(fh, chip->mii_reg);
            break;
        }
        chip ++;
    }        
    cfe_close(fh);
}
#endif /* CFG_ROBO */

#ifdef ROBODVT

/* For big endian operation */

static struct mdioCtrlReg1{
    uint8     page;
    uint8     writeAll:1;
    uint8     reserved:4;
    uint8     chipId:2;
    uint8     mdioEnable:1;
}  mdioCtrl1;

static struct mdioCtrlReg2 {
    uint8    regOffset;
    uint8    reserved:6;
    uint8    op:2;   
} mdioCtrl2;

int     gSPIEnable = TRUE;
int 	mdioDelay=100;
static uint16  *pMdioCtrl1 = (uint16 *) &mdioCtrl1;
static uint16  *pMdioCtrl2 = (uint16 *) &mdioCtrl2;
static uint16  mMdioBuf[4];
static 	char mMdioUsed = FALSE;

#define    MII_PAGE_PORTmin	0x10
#define    MII_PAGE_PORTmax	0x18
#define    PSEUDO_PHY		0x1e
#define    MDIO_CTRL1_ADDR	16
#define    MDIO_CTRL2_ADDR	17
#define    MDIO_DATAREG_BASE    24
#define    MDIO_OP_RD		2
#define    MDIO_OP_WR   	1

/*-------------------------------------------------------------
check if this access is at the same page as the previous access
if it is in different page, then need to set the mdio_ctrl1 
---------------------------------------------------------------*/
#define    CHKSET_CHIP_PAGE(cid, page) \
        if ( !mMdioUsed ||mdioCtrl1.page != page || mdioCtrl1.chipId !=cid ) {\
	    mMdioUsed = TRUE; 	   \
            mdioCtrl1.page = page; \
    	    mdioCtrl1.chipId = cid; \
	    chipphywr(etcgmac_ch, PSEUDO_PHY, MDIO_CTRL1_ADDR, *pMdioCtrl1); \
	}

void robo_mdio_reset() 
{
	mMdioUsed = FALSE;
	chipphywr(etcgmac_ch, PSEUDO_PHY, MDIO_CTRL1_ADDR, 0);
}

void setSPI(int fEnable)
{
	gSPIEnable = fEnable;
	mdioCtrl1.mdioEnable = 1-fEnable;
	mMdioUsed = FALSE;
	
	chipphywr(etcgmac_ch, PSEUDO_PHY, MDIO_CTRL1_ADDR, 1-fEnable);
}

void robo_mdio_rreg(uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len)
{
	int i, nWord=(len+1)>>1;	
	uint16 ret;
	uint8  *p = buf;
	
	if (len > 8) {
	    ET_ERROR(("length should be no greater than 8 bytes\n"));
	    return;
	}

	if (page>= MII_PAGE_PORTmin && page <= MII_PAGE_PORTmax) {
	    /* direct access */
	    if (len!=2) {
		ET_ERROR(("All MII registers are 2 bytes long\n"));
		return;
	    }

	    cid = cid<<3 | (page - MII_PAGE_PORTmin); 
	    ret = chipphyrd(etcgmac_ch, cid, addr>>1);
	    buf[0] = ret & 0x00ff;
	    buf[1] = ret >> 8;    
	    return;
	}

	/* if the access is to pages/chips other than the previous one
	   set the mdio_ctrl1 register again  */
	CHKSET_CHIP_PAGE(cid, page); 
	
	mdioCtrl2.op = MDIO_OP_RD;
	mdioCtrl2.regOffset = addr;
	chipphywr(etcgmac_ch, PSEUDO_PHY, MDIO_CTRL2_ADDR, *pMdioCtrl2);

	do {
	    *pMdioCtrl2 = chipphyrd(etcgmac_ch, PSEUDO_PHY, MDIO_CTRL2_ADDR);
	} while (mdioCtrl2.op != 0);

	for (i=0; i<nWord; i++ ) {
	    ret = chipphyrd(etcgmac_ch, PSEUDO_PHY, MDIO_DATAREG_BASE+ i);
            *p++ = ret & 0x00ff;
	    *p++ = ret >> 8;
	}
} 

void robo_mdio_wreg(uint8 cid, uint8 page, uint8 addr, uint8 *buf, uint len)
{
	int i, nWord=(len+1)>>1;

	if (len > 8) {
	    ET_ERROR(("length should be no greater than 8 bytes\n"));
	    return;
	}

	memcpy((uint8 *)mMdioBuf, buf, len);
	
	/*odd number of bytes, move data from hibyte to lobyte for the last 
	  byte */
        /* For BE, byte swap for the 16-bit MDIO access */
        for (i=0; i<nWord; i++) {
	    mMdioBuf[i] = bcmswap16(mMdioBuf[i]);
        }

	if (page>= MII_PAGE_PORTmin && page <= MII_PAGE_PORTmax) {
	    /*direct access */
	    if (len!=2) {
		ET_ERROR(("All MII registers are 2 bytes long\n"));
		return;
	    }

	    cid = cid<<3 |(page - MII_PAGE_PORTmin);
	    chipphywr(etcgmac_ch, cid, addr>>1, mMdioBuf[0]);
	    return;
	}

	for (i=0; i<nWord; i++ )  {
	    chipphywr(etcgmac_ch, PSEUDO_PHY, MDIO_DATAREG_BASE+ i, mMdioBuf[i]);
	}

	CHKSET_CHIP_PAGE(cid, page);
	mdioCtrl2.op = MDIO_OP_WR;
	mdioCtrl2.regOffset = addr;
	chipphywr(etcgmac_ch, PSEUDO_PHY, MDIO_CTRL2_ADDR, *pMdioCtrl2);

	do {
	    *pMdioCtrl2 = chipphyrd(etcgmac_ch, PSEUDO_PHY, MDIO_CTRL2_ADDR);
	} while (mdioCtrl2.op != 0);
	
}

#endif

