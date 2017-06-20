/*
 * $Id: nsgmac.c,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$ 
 *
 * Broadcom Gigabit Ethernet MAC (Unimac) core.
 *
 * This file implements the chip-specific routines for the GMAC core.
 */
#include <shared/et/osl.h>
#include <shared/et/bcmendian.h>
#include <shared/et/bcmutils.h>
#include <shared/et/bcmdevs.h>
#include <shared/et/proto/ethernet.h>
#include <shared/et/et_dbg.h>
#include <shared/et/sbconfig.h>
#include <shared/et/sbpci.h>
#include <shared/et/sbutils.h>
#include <shared/et/nvutils.h>
#include <shared/et/aiutils.h>
#include <shared/et/et_export.h>        /* for et_phyxx() routines */
#include <shared/et/bcmgmacmib.h>
#include <soc/nsgmac2reg.h>
#include <soc/hnddma.h>
#include <soc/nsgmac.h> 

#include <soc/drv.h>
#include <soc/knet.h>

#if defined(IPROC_CMICD) 

struct bcmgmac_ns; /* forward declaration */
#define ch_t    struct bcmgmac_ns
#include <soc/etc.h>

#define MAX_GMAC_CORES 3

/* private chip state */
struct bcmgmac_ns {
    void        *et;        /* pointer to et private state */
    etc_soc_info_t  *etc;       /* pointer to etc public state */

    nsgmac2regs_t *regs;      /* pointer to chip registers */

    void       *dev;       /* device handle */

    void        *etphy;     /* pointer to et for shared mdc/mdio contortion */

    uint32      intstatus;  /* saved interrupt condition bits */
    uint32      intmask;    /* current software interrupt mask */

    hnddma_t    *di[NUMTXQ];    /* dma engine software state */

    bool        mibgood;    /* true once mib registers have been cleared */

    nsgmacmib_t   mib;        /* mib statistic counters */
    si_t        *sih;        /* si utils handle */
/* XXX-NS */
	si_t 		*ext_sih[MAX_GMAC_CORES];		/* si utils handle */

    char        *vars;      /* sprom name=value */
    uint        vars_size;

    void        *adm;       /* optional admtek private data */

    mcfilter_t  mf;     /* multicast filter */
};

/* local prototypes */
static bool chipid(uint vendor, uint device);
static void *chipattach(etc_soc_info_t *etc, void *osh, void *regsva);
static void chipdetach(ch_t *ch);
static void chipreset(ch_t *ch);
static void chipinit(ch_t *ch, bool full);
static bool chiptx(ch_t *ch, void *p);
static void *chiprx(ch_t *ch);
static void chiprxfill(ch_t *ch);
static int chipgetintrevents(ch_t *ch);
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
static void chipdump(ch_t *ch, char *b);
static void chiplongname(ch_t *ch, char *buf, uint bufsize);
static void chipduplexupd(ch_t *ch);

static void chipphyinit(ch_t *ch, uint phyaddr);
static void chipphyor(ch_t *ch, uint phyaddr, uint reg, uint16 v);
static void chipphyforce(ch_t *ch, uint phyaddr);
static void chipphyadvertise(ch_t *ch, uint phyaddr);
static void chiprxreset(ch_t *ch, int id);
static void chiprxinit(ch_t *ch, int id);
static bool chiprecover(ch_t *ch);
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

static char* chipdumpregs(ch_t *ch, nsgmac2regs_t *regs, char *b);
static void gmac_mf_cleanup(ch_t *ch);
static int gmac_speed(ch_t *ch, uint32 speed);
static void gmac_miiconfig(ch_t *ch);

#define BCM5301X_ROBO_MDC_PORT         2 
#define BCM5301X_ROBO_PHY_ADDR         EPHY_NOREG

/* one tx dma and one rx dma */
#define BCM5301X_TX_Q_IDX 0
#define BCM5301X_RX_Q_IDX 0

/* CPU mac address */
static char *cpu_mac = "00-10-18-53-00-01";

struct chops bcm_nsgmac_et_soc_chops = {
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
    chiprxreset,
    chiprxinit,
    chiprecover,
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

static uint gmac_devices[] = {
	BCM53010_GMAC_ID,
	0x0000
};

static bool
chipid(uint vendor, uint device)
{
    int i;

    if (vendor == VENDOR_BROADCOM_ID) {
	    for (i = 0; gmac_devices[i]; i++) {
            if (device == gmac_devices[i]) {
                return (TRUE);
            }
        }
    }

    return (FALSE);
}

static void*
chipattach(etc_soc_info_t *etc, void *dev, void *regsva)
{
    ch_t *ch;
    nsgmac2regs_t *regs;
    uint coreunit = 0;
    char name[16];

    ET_TRACE(("et%d: chipattach: regsva 0x%lx\n", etc->unit, (ulong)regsva));

    if ((ch = (ch_t *) ET_MALLOC(sizeof(ch_t))) == NULL) {
        ET_ERROR(("et%d: chipattach: out of memory\n", etc->unit));
        return (NULL);
    }
    bzero((char *)ch, sizeof(ch_t));

    ch->etc = etc;
    ch->et = etc->et;
    ch->dev = dev;


    /* store the pointer to the sw mib */
    etc->mib = (void *)&ch->mib;

    etc->hwrxoff = ETCGMAC_HWRXOFF;

    /* get si handle */
    if ((ch->sih = ai_soc_attach(etc->deviceid, ch->dev, regsva, SI_BUS, NULL, &ch->vars,
                             &ch->vars_size)) == NULL) {
        ET_ERROR(("et%d: chipattach: si_attach error\n", etc->unit));
        goto fail;
    }

    if ((ch->sih->chip == BCM53020_CHIP_ID) && (ch->sih->chiprev == 0x4)) {
    /* BCM53022 NS+ XMC card : Use GMAC 1 */
        coreunit = 1;
    } else {
        coreunit = 2;
    }

    /* Starting offset of GMAC 2, base address(0x18000000) is assigned during kernel BDE */
	if ((regs = (nsgmac2regs_t *)ai_soc_setcore(ch->sih, GMAC_CORE_ID_NS, coreunit)) == NULL) {
		ET_ERROR(("et%d: chipattach: Could not setcore to GMAC\n", etc->unit));
		goto fail;
	}

    ET_TRACE(("et%d: chipattach: regs 0x%x\n", etc->unit, (uint32)regs));
    ch->regs = regs;

    etc->chip = ch->sih->chip;
    etc->chiprev = ch->sih->chiprev;
    etc->coreid = ai_soc_coreid(ch->sih);

    etc->corerev = ai_soc_corerev(ch->sih);
    etc->nicmode = !(ch->sih->bustype == SI_BUS);
    etc->coreunit = ai_soc_coreunit(ch->sih);

    etc->allmulti = 1; /* enable all multicasts */
    etc->ext_config = FALSE;

    etc->phyaddr = BCM5301X_ROBO_PHY_ADDR; 
    /* reset the gmac core */
    chipreset(ch);

#ifdef INCLUDE_KNET
    if (!SOC_KNET_MODE(etc->unit)) 
#endif        
    {                
        /* dma attach */
        sprintf(name, "et%d", etc->coreunit);

        /* allocate dma resources for tx/rx queue */
        /* TX: TC_BK, RX: RX_Q0 */
        ch->di[BCM5301X_TX_Q_IDX] = dma_soc_attach(etc->et, name, 0, ch->dev, 1,
                               DMAREG(ch, DMA_TX, TX_Q0),
                               DMAREG(ch, DMA_RX, RX_Q0),
                               NTXD, NRXD, RXBUFSZ, NRXBUFPOST, ETCGMAC_HWRXOFF,
                               (uint *)&et_msg_level, etc->pkt_mem, etc->pkthdr_mem,
                               etc->en_rxsephdr[0]);

        if (ch->di[BCM5301X_TX_Q_IDX] == NULL) {
            ET_ERROR(("et%d: chipattach: dma_soc_attach failed\n", etc->unit));
            goto fail;
        }

        if (ch->di[BCM5301X_TX_Q_IDX] != NULL)
            etc->txavail[BCM5301X_TX_Q_IDX] = \
                (uint*)dma_soc_getvar(ch->di[BCM5301X_TX_Q_IDX], "&txavail");

     }

    bcm_ether_atoe(cpu_mac, (char*) &etc->perm_etheraddr);
    bcopy((char*)&etc->perm_etheraddr, (char*)&etc->cur_etheraddr, ETHER_ADDR_LEN);


    /* set default sofware intmask */
    ch->intmask = DEF_INTMASK;
    
    etc->mdcport = BCM5301X_ROBO_MDC_PORT;

    /* reset phy: reset it once now */
    if (etc->mdcport == etc->coreunit)
        chipphyreset(ch, etc->phyaddr);

    return ((void*) ch);

fail:
    chipdetach(ch);
    return (NULL);
}

static void
chipdetach(ch_t *ch)
{
    int32 i;

    ET_TRACE(("et%d: chipdetach\n", ch->etc->unit));

    if (ch == NULL)
        return;

#ifdef INCLUDE_KNET
    if (!SOC_KNET_MODE(ch->etc->unit)) 
#endif  
    {
        /* free dma state */
        for (i = 0; i < NUMTXQ; i++) {
            if (ch->di[i] != NULL) {
                dma_soc_detach(ch->di[i]);
                ch->di[i] = NULL;
            }
        }
    }

    /* put the core back into reset */
    if (ch->sih){
        ai_soc_core_disable(ch->sih, 0);
    }

    ch->etc->mib = NULL;

    /* free si handle */
    ai_soc_detach(ch->sih);
    ch->sih = NULL;

    /* free vars */
    if (ch->vars)
        ET_MFREE(ch->vars, ch->vars_size);

    /* free chip private state */
    ET_MFREE(ch, sizeof(ch_t));
}

static void
chiplongname(ch_t *ch, char *buf, uint bufsize)
{
    char *s;

    switch (ch->etc->deviceid) {
    /* CHECKME: need to confirm the id value */
        case BCM53010_GMAC_ID: 
        default:
            s = "Broadcom BCM53010 Ethernet Controller";
            break;
    }

    strncpy(buf, s, bufsize);
    buf[bufsize - 1] = '\0';
}

static void
chipdump(ch_t *ch, char *b)
{
    int32 i;

    b += sprintf(b, 
        "regs 0x%x etphy 0x%x ch->intstatus 0x%x intmask 0x%x\n",
        (uint)ch->regs, (uint)ch->etphy, ch->intstatus, ch->intmask);
    b += sprintf(b, "\n");

    /* XXX bcm_binit() to move up in chain of functions where b is allocated, when
     * all sprintf's are replaced by bcm_bprintf's
     */
#ifdef INCLUDE_KNET
    if (!SOC_KNET_MODE((int)(ch->dev)))
#endif
    {
        /* dma engine state */
        for (i = 0; i < NUMTXQ; i++) {
		    if (ch->di[i]) {
                b = dma_soc_dump(ch->di[i], b, TRUE);
                b += sprintf(b, "\n");
		    }
        }
    }
    /* registers */
    b = chipdumpregs(ch, ch->regs, b);
    b += sprintf(b, "\n");
    
}

/* #define PRREG(name) printf(#name " 0x%x ", R_REG(ch->osh, &regs->name)) */

#define PRREG(name) \
    buf += sprintf(buf, #name " 0x%x ", R_REG(ch->dev, &regs->name))

static char*
chipdumpregs(ch_t *ch, nsgmac2regs_t *regs, char *buf)
{
    uint phyaddr;

    phyaddr = ch->etc->phyaddr;

    PRREG(gmac2_devcontrol); PRREG(gmac2_devstatus);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_biststatus);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_intstatus); PRREG(gmac2_intmask); PRREG(gmac2_gptimer);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_intrcvlazy);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_flowcntl_th); PRREG(gmac2_txarb_wrr_th); PRREG(gmac2_gmacidle_cnt_th);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_fifoaccessaddr); PRREG(gmac2_fifoaccessbyte); PRREG(gmac2_fifoaccessdata);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_phyaccess); PRREG(gmac2_phycontrol);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_txqcontrol); PRREG(gmac2_rxqcontrol);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_gpioselect); PRREG(gmac2_gpiooutputen);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_txqrxqmemorycontrol); PRREG(gmac2_memoryeccstatus);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_clockcontrolstatus); PRREG(gmac2_powercontrol);
    buf += sprintf(buf, "\n");

    /* unimac registers */
    PRREG(unimac2_ipg_hd_bkp_cntl);
    buf += sprintf(buf, "\n");
    PRREG(unimac2_command_config);
    buf += sprintf(buf, "\n");
    PRREG(unimac2_mac_0); PRREG(unimac2_mac_1);
    buf += sprintf(buf, "\n");
    PRREG(unimac2_frm_length); PRREG(unimac2_pause_quant); 
    buf += sprintf(buf, "\n");
    PRREG(unimac2_tx_ts_seq_id); PRREG(unimac2_mac_mode); 
    buf += sprintf(buf, "\n");
    PRREG(unimac2_tag_0); PRREG(unimac2_tag_1); 
    buf += sprintf(buf, "\n");
    PRREG(unimac2_rx_pause_quanta_scale); PRREG(unimac2_tx_preamble); 
    buf += sprintf(buf, "\n");
    PRREG(unimac2_tx_ipg_length); PRREG(unimac2_pfc_xoff_timer);
    buf += sprintf(buf, "\n");
    PRREG(unimac2_umac_eee_ctrl); PRREG(unimac2_mii_eee_delay_entry_timer);
    buf += sprintf(buf, "\n");
    PRREG(unimac2_gmii_eee_delay_entry_timer); PRREG(unimac2_umac_eee_ref_count);
    buf += sprintf(buf, "\n");
    PRREG(unimac2_umac_rx_pkt_drop_status); PRREG(unimac2_umac_symmetric_idle_threshold);
    buf += sprintf(buf, "\n");
    PRREG(unimac2_mii_eee_wake_timer); PRREG(unimac2_gmii_eee_wake_timer);
    buf += sprintf(buf, "\n");
    PRREG(unimac2_umac_rev_id);
    buf += sprintf(buf, "\n");
    PRREG(unimac2_mac_pfc_type); PRREG(unimac2_mac_pfc_opcode);
    buf += sprintf(buf, "\n");
    PRREG(unimac2_mac_pfc_da_0); PRREG(unimac2_mac_pfc_da_1);
    buf += sprintf(buf, "\n");
    PRREG(unimac2_macsec_cntrl);
    buf += sprintf(buf, "\n");
    PRREG(unimac2_ts_status_cntrl); PRREG(unimac2_tx_ts_data);
    buf += sprintf(buf, "\n");
    PRREG(unimac2_pause_control); PRREG(unimac2_flush_control);
    buf += sprintf(buf, "\n");
    PRREG(unimac2_rxfifo_stat); PRREG(unimac2_txfifo_stat);
    buf += sprintf(buf, "\n");
    PRREG(unimac2_mac_pfc_ctrl); PRREG(unimac2_mac_pfc_refresh_ctrl);
    buf += sprintf(buf, "\n");

    /* mib registers */
    PRREG(gmac2_tx_gd_octets_lo); PRREG(gmac2_tx_gd_octets_hi); PRREG(gmac2_tx_gd_pkts);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_tx_all_octets_lo); PRREG(gmac2_tx_all_octets_hi); PRREG(gmac2_tx_all_pkts);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_tx_brdcast); PRREG(gmac2_tx_mult);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_tx_64); PRREG(gmac2_tx_65_127); PRREG(gmac2_tx_128_255);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_tx_256_511); PRREG(gmac2_tx_512_1023); PRREG(gmac2_tx_1024_1522);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_tx_1523_2047); PRREG(gmac2_tx_2048_4095); PRREG(gmac2_tx_4096_8191);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_tx_8192_max);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_tx_jab); PRREG(gmac2_tx_over);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_tx_frag); PRREG(gmac2_tx_underrun); PRREG(gmac2_tx_col);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_tx_1_col); PRREG(gmac2_tx_m_col); PRREG(gmac2_tx_ex_col);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_tx_late); PRREG(gmac2_tx_def); PRREG(gmac2_tx_crs);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_tx_paus); PRREG(gmac2_txunicastpkt);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_txqosq0pkt); PRREG(gmac2_txqosq0octet_lo); PRREG(gmac2_txqosq0octet_hi);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_txqosq1pkt); PRREG(gmac2_txqosq1octet_lo); PRREG(gmac2_txqosq1octet_hi);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_txqosq2pkt); PRREG(gmac2_txqosq2octet_lo); PRREG(gmac2_txqosq2octet_hi);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_txqosq3pkt); PRREG(gmac2_txqosq3octet_lo); PRREG(gmac2_txqosq3octet_hi);
    buf += sprintf(buf, "\n");

    PRREG(gmac2_rx_gd_octets_lo); PRREG(gmac2_rx_gd_octets_hi); PRREG(gmac2_rx_gd_pkts);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_rx_all_octets_lo); PRREG(gmac2_rx_all_octets_hi); PRREG(gmac2_rx_all_pkts);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_rx_brdcast); PRREG(gmac2_rx_mult);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_rx_64); PRREG(gmac2_rx_65_127); PRREG(gmac2_rx_128_255);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_rx_256_511); PRREG(gmac2_rx_512_1023); PRREG(gmac2_rx_1024_1522);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_rx_1523_2047); PRREG(gmac2_rx_2048_4095); PRREG(gmac2_rx_4096_8191);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_rx_8192_max); PRREG(gmac2_rx_jab); PRREG(gmac2_rx_ovr);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_rx_frag); PRREG(gmac2_rx_drop); PRREG(gmac2_rx_crc_align);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_rx_und); PRREG(gmac2_rx_crc); PRREG(gmac2_rx_align);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_rx_sym); PRREG(gmac2_rx_paus); PRREG(gmac2_rx_cntrl);
    buf += sprintf(buf, "\n");
    PRREG(gmac2_rxsachanges); PRREG(gmac2_rxunicastpkts);
    buf += sprintf(buf, "\n");

    if (phyaddr != EPHY_NOREG) {
        /* print a few interesting phy registers */
        buf += sprintf(buf, "phy0 0x%x phy1 0x%x phy2 0x%x phy3 0x%x\n",
                   chipphyrd(ch, phyaddr, 0),
                   chipphyrd(ch, phyaddr, 1),
                   chipphyrd(ch, phyaddr, 2),
                   chipphyrd(ch, phyaddr, 3));
        buf += sprintf(buf, "phy4 0x%x phy5 0x%x phy24 0x%x phy25 0x%x\n",
                   chipphyrd(ch, phyaddr, 4),
                   chipphyrd(ch, phyaddr, 5),
                   chipphyrd(ch, phyaddr, 24),
                   chipphyrd(ch, phyaddr, 25));
    }

    return buf;

}

/*static void*/
void
gmac_clearmib(ch_t *ch)
{
    volatile uint32 *ptr; 

    /* enable clear on read */
    OR_REG(ch->dev, &ch->regs->gmac2_devcontrol, GMAC2_DEVCONTROL_MIB_RD_RESET_EN_MASK);

    for (ptr = &ch->regs->gmac2_tx_gd_octets_lo; ptr <= &ch->regs->gmac2_rxunicastpkts; ptr++) {
        (void)R_REG(ch->dev, ptr);
        if (ptr == &ch->regs->gmac2_txqosq3octet_hi)
            ptr = ptr + 1;
    }

    return;
}

static void
gmac_init_reset(ch_t *ch)
{    
    OR_REG(ch->dev, &ch->regs->unimac2_command_config, UNIMAC2_COMMAND_CONFIG_SW_RESET_MASK);
    OSL_DELAY(GMAC_RESET_DELAY);
}

static void
gmac_clear_reset(ch_t *ch)
{
    AND_REG(ch->dev, &ch->regs->unimac2_command_config, ~UNIMAC2_COMMAND_CONFIG_SW_RESET_MASK);
    OSL_DELAY(GMAC_RESET_DELAY);
}

static void
gmac_reset(ch_t *ch)
{
    uint32 ocmdcfg, cmdcfg;

    /* put the mac in reset */
    gmac_init_reset(ch);

    /* initialize default config */
    ocmdcfg = cmdcfg = R_REG(ch->dev, &ch->regs->unimac2_command_config);

    /* Clear the ENA_EXT_CONFIG bit for software flow control */
    cmdcfg &= ~(UNIMAC2_COMMAND_CONFIG_TX_ENA_MASK | \
	            UNIMAC2_COMMAND_CONFIG_RX_ENA_MASK | \
                UNIMAC2_COMMAND_CONFIG_PAUSE_IGNORE_MASK | \
                UNIMAC2_COMMAND_CONFIG_TX_ADDR_INS_MASK | \
                UNIMAC2_COMMAND_CONFIG_HD_ENA_MASK | \
                UNIMAC2_COMMAND_CONFIG_MAC_LOOP_CON_MASK | \
                UNIMAC2_COMMAND_CONFIG_CNTL_FRM_ENA_MASK | \
                UNIMAC2_COMMAND_CONFIG_LINE_LOOPBACK_MASK | \
                UNIMAC2_COMMAND_CONFIG_IGNORE_TX_PAUSE_MASK | \
                UNIMAC2_COMMAND_CONFIG_PAD_EN_MASK | \
                UNIMAC2_COMMAND_CONFIG_ENA_EXT_CONFIG_MASK);
    
    cmdcfg |= (UNIMAC2_COMMAND_CONFIG_PROMIS_EN_MASK | \
               UNIMAC2_COMMAND_CONFIG_PAUSE_FWD_MASK | \
               UNIMAC2_COMMAND_CONFIG_NO_LGTH_CHECK_MASK | \
               UNIMAC2_COMMAND_CONFIG_CNTL_FRM_ENA_MASK);

    if (cmdcfg & UNIMAC2_COMMAND_CONFIG_ENA_EXT_CONFIG_MASK) {
        ch->etc->ext_config = TRUE;
    } else {
        ch->etc->ext_config = FALSE;
    }

    if (cmdcfg != ocmdcfg)
        W_REG(ch->dev, &ch->regs->unimac2_command_config, cmdcfg);

    /* bring mac out of reset */
    gmac_clear_reset(ch);
}


static void
gmac_promisc(ch_t *ch, bool mode)
{
    uint32 cmdcfg;

    cmdcfg = R_REG(ch->dev, &ch->regs->unimac2_command_config);

    /* put the mac in reset */
    gmac_init_reset(ch);


    /* enable or disable promiscuous mode */
    if (mode)
        cmdcfg |= UNIMAC2_COMMAND_CONFIG_PROMIS_EN_MASK;
    else
        cmdcfg &= ~UNIMAC2_COMMAND_CONFIG_PROMIS_EN_MASK;

    W_REG(ch->dev, &ch->regs->unimac2_command_config, cmdcfg);

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
            hd_ena = UNIMAC2_COMMAND_CONFIG_HD_ENA_MASK;
            /* FALLTHRU */

        case ET_10FULL:
            speed = 0;
            break;

        case ET_100HALF:
            hd_ena = UNIMAC2_COMMAND_CONFIG_HD_ENA_MASK;
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

        case ET_2000FULL:
            /* CHECKME: 
                   * In addition to configure unimac to 2Gbps, also check the IDM configuration.
                   * 
                   * IDM GMAC2 iocontrol: 0x18112408
                   * bit 6: CLK_250_SEL. 
                   *     Set this field 1 to select 250MHz reference clock and hence unimac line rate will be 2G.
                   */

            speed = 0x3;
            break;

        default:
            ET_ERROR(("et%d: gmac_speed: speed %d not supported\n",
                      ch->etc->unit, speed));
            return (FAILURE);
    }

    cmdcfg = R_REG(ch->dev, &ch->regs->unimac2_command_config);

    /* put mac in reset */
    gmac_init_reset(ch);

    /* set the speed */
    cmdcfg &= ~(UNIMAC2_COMMAND_CONFIG_ETH_SPEED_MASK | \
                UNIMAC2_COMMAND_CONFIG_HD_ENA_MASK);
    cmdcfg |= ((speed << UNIMAC2_COMMAND_CONFIG_ETH_SPEED_SHIFT) | hd_ena);
    W_REG(ch->dev, &ch->regs->unimac2_command_config, cmdcfg);

    /* bring mac out of reset */
    gmac_clear_reset(ch);

    return (SUCCESS);
}

static void
gmac_macloopback(ch_t *ch, bool on)
{
    uint32 ocmdcfg, cmdcfg;

    ocmdcfg = cmdcfg = R_REG(ch->dev, &ch->regs->unimac2_command_config);

    /* put mac in reset */
    gmac_init_reset(ch);

    /* set/clear the mac loopback mode */
    if (on)
        cmdcfg |= UNIMAC2_COMMAND_CONFIG_LOOP_ENA_MASK;
    else
        cmdcfg &= ~UNIMAC2_COMMAND_CONFIG_LOOP_ENA_MASK;

    if (cmdcfg != ocmdcfg)
        W_REG(ch->dev, &ch->regs->unimac2_command_config, cmdcfg);

    /* bring mac out of reset */
    gmac_clear_reset(ch);
}

static int
gmac_loopback(ch_t *ch, uint32 mode)
{
    switch (mode) {
        case LOOPBACK_MODE_DMA:
            /* to enable loopback for any channel set the loopback
             * enable bit in xmt0control register.
             */
#ifdef INCLUDE_KNET
            if (SOC_KNET_MODE((int)(ch->dev))) {
                soc_eth_knet_hw_config((int)(ch->dev), KCOM_ETH_HW_T_OTHER,
                    KCOM_ETH_HW_C_ALL, KCOM_ETH_HW_OTHER_F_FIFO_LOOPBACK, TRUE);
            } else           
#endif
            {
                dma_soc_fifoloopbackenable(ch->di[BCM5301X_TX_Q_IDX], TRUE);
            }
            break;

        case LOOPBACK_MODE_MAC:
            gmac_macloopback(ch, TRUE);
            break;

        case LOOPBACK_MODE_NONE:
            gmac_macloopback(ch, FALSE);
#ifdef INCLUDE_KNET
            if (SOC_KNET_MODE((int)(ch->dev))) {
                soc_eth_knet_hw_config((int)(ch->dev), KCOM_ETH_HW_T_OTHER,
                    KCOM_ETH_HW_C_ALL, KCOM_ETH_HW_OTHER_F_FIFO_LOOPBACK, FALSE);
            } else
#endif
            {
                /* Disable DMA lookback */
                dma_soc_fifoloopbackenable(ch->di[BCM5301X_TX_Q_IDX], FALSE);
            }
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
    uint32 cmdcfg;
    nsgmac2regs_t *regs;

    regs = ch->regs;

    cmdcfg = R_REG(ch->dev, &ch->regs->unimac2_command_config);

    /* put mac in reset */
    gmac_init_reset(ch);

    cmdcfg |= UNIMAC2_COMMAND_CONFIG_SW_RESET_MASK;

    /* first deassert rx_ena and tx_ena while in reset */
    cmdcfg &= ~(UNIMAC2_COMMAND_CONFIG_RX_ENA_MASK | \
                UNIMAC2_COMMAND_CONFIG_TX_ENA_MASK);
    W_REG(ch->dev, &regs->unimac2_command_config, cmdcfg);

    /* bring mac out of reset */
    gmac_clear_reset(ch);

    /* enable the mac transmit and receive paths now */
    OSL_DELAY(2);
    cmdcfg &= ~UNIMAC2_COMMAND_CONFIG_SW_RESET_MASK;
    cmdcfg |= (UNIMAC2_COMMAND_CONFIG_RX_ENA_MASK | \
	           UNIMAC2_COMMAND_CONFIG_TX_ENA_MASK);

    /* assert rx_ena and tx_ena when out of reset to enable the mac */
    W_REG(ch->dev, &regs->unimac2_command_config, cmdcfg);

    /* request ht clock */
    W_REG(ch->dev, &regs->gmac2_clockcontrolstatus, \
          GMAC2_CLOCKCONTROLSTATUS_FORCEHTREQUEST_MASK);

    return;
}

static void
gmac_txflowcontrol(ch_t *ch, bool on)
{
    uint32 cmdcfg;

    cmdcfg = R_REG(ch->dev, &ch->regs->unimac2_command_config);

    /* put the mac in reset */
    gmac_init_reset(ch);

    /* to enable tx flow control clear the rx pause ignore bit */
    if (on) {
        cmdcfg &= ~(UNIMAC2_COMMAND_CONFIG_PAUSE_IGNORE_MASK |
            UNIMAC2_COMMAND_CONFIG_IGNORE_TX_PAUSE_MASK); 
    } else {
        cmdcfg |= UNIMAC2_COMMAND_CONFIG_PAUSE_IGNORE_MASK |
            UNIMAC2_COMMAND_CONFIG_IGNORE_TX_PAUSE_MASK; 
    }

    W_REG(ch->dev, &ch->regs->unimac2_command_config, cmdcfg);

    /* bring mac out of reset */
    gmac_clear_reset(ch);
}

static void
gmac_miiconfig(ch_t *ch)
{
    /* BCM53010 GMAC DevStatus register has different definition of "Interface Mode"
     * Bit 12:8  "interface_mode"  This field is programmed through IDM control bits [6:2]
     *
     * Bit 0 : SOURCE_SYNC_MODE_EN - If set, Rx line clock input will be used by Unimac for
     *          sampling data.If this is reset, PLL reference clock (Clock 250 or Clk 125 based
     *          on CLK_250_SEL) will be used as receive side line clock.
     * Bit 1 : DEST_SYNC_MODE_EN - If this is reset, PLL reference clock input (Clock 250 or
     *          Clk 125 based on CLK_250_SEL) will be used as transmit line clock.
     *          If this is set, TX line clock input (from external switch/PHY) is used as
     *          transmit line clock.
     * Bit 2 : TX_CLK_OUT_INVERT_EN - If set, this will invert the TX clock out of AMAC.
     * Bit 3 : DIRECT_GMII_MODE - If direct gmii is set to 0, then only 25 MHz clock needs to
     *          be fed at 25MHz reference clock input, for both 10/100 Mbps speeds.
     *          Unimac will internally divide the clock to 2.5 MHz for 10 Mbps speed
     * Bit 4 : CLK_250_SEL - When set, this selects 250Mhz reference clock input and hence
     *          Unimac line rate will be 2G.
     *          If reset, this selects 125MHz reference clock input.
     */
    if (ch->etc->forcespeed == ET_AUTO) {
        gmac_speed(ch, ET_2000FULL);
    } else {
        gmac_speed(ch, ch->etc->forcespeed);
    }
}

static void
chipreset(ch_t *ch)
{
    nsgmac2regs_t *regs;
    uint32 i;
    int ns_gmac = 0;
    uint32 force_reset = 0;
    char *s;

    ET_TRACE(("et%d: chipreset\n", ch->etc->unit));

    regs = ch->regs;

    if (!ai_soc_iscoreup(ch->sih)) {
        /* power on reset: reset the enet core */
        goto chipinreset;
    }

    /* update software counters before resetting the chip */
    if (ch->mibgood)
        chipstatsupd(ch);

#ifdef INCLUDE_KNET
    if (SOC_KNET_MODE((int)(ch->dev))) {
        /* reset the tx dma engines */
        soc_eth_knet_hw_config((int)(ch->dev), KCOM_ETH_HW_T_RESET,
            KCOM_ETH_HW_C_ALL, KCOM_ETH_HW_RESET_F_TX, 0);

        /* set gmac into loopback mode to ensure no rx traffic */
        gmac_loopback(ch, LOOPBACK_MODE_MAC);

        OSL_DELAY(1);

        /* reset the rx dma engine */  
        soc_eth_knet_hw_config((int)(ch->dev), KCOM_ETH_HW_T_RESET,
            KCOM_ETH_HW_C_ALL, KCOM_ETH_HW_RESET_F_RX, 0);

        OSL_DELAY(1);
    } else    
#endif
    {
        /* reset the tx dma engines */
        for (i = 0; i < NUMTXQ; i++) {
            if (ch->di[i]) {
                dma_soc_txreset(ch->di[i]);
            }
        }

        /* set gmac into loopback mode to ensure no rx traffic */
        gmac_loopback(ch, LOOPBACK_MODE_MAC);
        OSL_DELAY(1);

        /* reset the rx dma engine */  
        if (ch->di[BCM5301X_RX_Q_IDX]) {
            dma_soc_rxreset(ch->di[BCM5301X_RX_Q_IDX]);
        }

        /* set gmac out of loopback mode */
        /* 
         * only need to program the bit in unimac cmd_cfg reg 
         * don't call gmac_loopback(ch, LOOPBACK_MODE_NONE)
         * since it also configure dma but dma driver may not
         * attached yet.
         */
        gmac_macloopback(ch, FALSE);
        OSL_DELAY(1);
    }
    /* clear the multicast filter table */
    gmac_mf_cleanup(ch);

chipinreset:
    force_reset = 0;
    s = soc_property_get_str(0, "board_name");
    if((s != NULL) && (sal_strcmp(s, "bcm5301x_gmac_reset") == 0)) {
        force_reset = 1;
    }
    if (force_reset) {
        ET_TRACE(("DO GMAC CORE RESET\n"));
        ai_soc_core_reset(ch->sih, 0x44, 0);
    }

    /* reset all GMAC cores, if necessary */
    for (ns_gmac = 0; ns_gmac < 4; ns_gmac++) {
        /* In case GMAC cores aren't power on. */
        ai_soc_setcore(ch->sih, GMAC_CORE_ID_NS, ns_gmac);
        if (!ai_soc_iscoreup(ch->sih)) {
            ET_TRACE(("et%d: reset NorthStar GMAC[%d] core\n", ch->etc->unit, ns_gmac));
            ai_soc_core_reset(ch->sih, 0x04, 0);
        }
    }
    ai_soc_setcore(ch->sih, GMAC_CORE_ID_NS, ch->etc->coreunit);

    /* reset gmac */
    gmac_reset(ch);

    /* clear mib */
    gmac_clearmib(ch);

    ch->mibgood = TRUE;

    ASSERT(regs != NULL);
    OR_REG(ch->dev, &regs->gmac2_phycontrol, \
	       GMAC2_PHYCONTROL_MDC_TRANSITION_EN_MASK);

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
    uint8 mac[18];
    
    /* Avoid compiler warning if ET_ERROR does nothing */
    mac[0] = 0;

    COMPILER_REFERENCE(mac);

    /* add multicast addresses only */
    if (!ETHER_ISMULTI(mcaddr)) {
        ET_ERROR(("et%d: adding invalid multicast address %s\n",
                  ch->etc->unit, bcm_ether_ntoa((char *)mcaddr, (char *)mac)));
        return (FAILURE);
    }

    /* discard duplicate add requests */
    if (gmac_mf_lkup(ch, mcaddr) == SUCCESS) {
        ET_ERROR(("et%d: adding duplicate mcast filter entry\n", ch->etc->unit));
        return (FAILURE);
    }

    /* allocate memory for list entry */
    entry = ET_MALLOC(sizeof(mflist_t));
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
            ET_MFREE(ptr, sizeof(mflist_t));
        }
        ch->mf.bucket[i] = NULL;
    }
}


/*
 * Initialize all the chip registers.  If dma mode, init tx and rx dma engines
 * but leave the devcontrol tx and rx (fifos) disabled.
 */
static void
chipinit(ch_t *ch, bool full)
{
    etc_soc_info_t *etc;
    nsgmac2regs_t *regs;
    uint i;

    regs = ch->regs;
    etc = ch->etc;

    ET_TRACE(("et%d: chipinit\n", etc->unit));

    /* enable one rx interrupt per received frame */
    W_REG(ch->dev, &regs->gmac2_intrcvlazy, \
          (1 << GMAC2_INTRCVLAZY_FRAMECOUNT_SHIFT));

    /* flow control mode */
    chipflowctrlmodeset(ch, etc->flowcntl_mode);

    chipflowctrlautoset(ch, etc->flowcntl_auto_on_thresh, 
        etc->flowcntl_auto_off_thresh);

    chipflowctrlcpuset(ch, etc->flowcntl_cpu_pause_on);

    /* tx qos mode */
    for (i=0; i < (NUMTXQ - 1); i++) {
        chiptxqosweightset(ch, i, etc->txqos_weight[i]);
    }

    /* RX separate header is global for Keystone */
    if (etc->en_rxsephdr[0]) {
        chiprxsephdrset(ch, 1);
    } else {
        chiprxsephdrset(ch, 0);
    }
#ifdef INCLUDE_KNET
    if (!SOC_KNET_MODE((int)(ch->dev)))
#endif
    {
        dma_soc_rxsephdrctrl(ch->di[BCM5301X_RX_Q_IDX], etc->en_rxsephdr[BCM5301X_RX_Q_IDX], 
            etc->pkthdr_mem, RXSEPHDRSZ);

    }

    /* enable/disable promiscuous mode */
    gmac_promisc(ch, etc->promisc);

    /* Set TX IPG length */
    W_REG(ch->dev, &regs->unimac2_tx_ipg_length, 12);

    if (!etc->promisc) {
        /* set our local address */
        W_REG(ch->dev, &regs->unimac2_mac_0,
              hton32(*(uint32 *)&etc->cur_etheraddr.octet[0]));
        W_REG(ch->dev, &regs->unimac2_mac_1,
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

    /* set max frame lengths - account for possible vlan tag */
    W_REG(ch->dev, &regs->unimac2_frm_length, ETHER_MAX_LEN + 32);

    /*
     * Optionally, disable phy autonegotiation and force our speed/duplex
     * or constrain our advertised capabilities.
     */
    if (etc->forcespeed != ET_AUTO) {
        gmac_speed(ch, etc->forcespeed);
        chipphyforce(ch, etc->phyaddr);
    } else if (etc->advertise && etc->needautoneg) {
        chipphyadvertise(ch, etc->phyaddr);
    }

    /* initialize the tx and rx dma channels */
#ifdef INCLUDE_KNET
    if (SOC_KNET_MODE((int)(ch->dev))) {
        int flags;        

        flags = KCOM_ETH_HW_INIT_F_TX |
            KCOM_ETH_HW_INIT_F_RX;
        soc_eth_knet_hw_config((int)(ch->dev), KCOM_ETH_HW_T_INIT,
            KCOM_ETH_HW_C_ALL, flags, 1);
        if (full) {
            soc_eth_knet_hw_config((int)(ch->dev), KCOM_ETH_HW_T_INIT,
                KCOM_ETH_HW_C_ALL, KCOM_ETH_HW_INIT_F_RX_FILL, 0);
        }

        soc_eth_knet_hw_config((int)(ch->dev), KCOM_ETH_HW_T_OTHER,
            KCOM_ETH_HW_C_ALL, KCOM_ETH_HW_OTHER_F_INTERRUPT, 1);

    } else
#endif
    {

        for (i = 0; i < NUMTXQ; i++) {
		    if (ch->di[i]) {
                dma_soc_txinit(ch->di[i]);
		    }
        }

        for (i = 0; i < NUMRXQ; i++) {
		    if (ch->di[i]) {
                dma_soc_rxinit(ch->di[i]);
                if (full) {
                    /* post dma receive buffers */
                    dma_soc_rxfill(ch->di[i]);
                }
		    }
        }

        /* lastly, enable interrupts */
        W_REG(ch->dev, &regs->gmac2_intmask, ch->intmask);
    }

    /* turn on the emac */
    gmac_enable(ch);

}

/* dma transmit */
static bool
chiptx(ch_t *ch, void *p0)
{
    int error, len;
    uint32 q = TX_Q0;

    ET_TRACE(("et%d: chiptx\n", ch->etc->unit));
    ET_LOG("et%d: chiptx", ch->etc->unit, 0);

    len = ET_PKTLEN(ch->dev, p0);

    /* check tx max length */
    if (len > (ETHER_MAX_LEN + 32)) {
        ET_ERROR(("et%d: chiptx: max frame length exceeded\n",
                  ch->etc->unit));
        return FALSE;
    }

    /* gmac rev 0 workaround:  unimac can only transmit frames of
     * length 17 bytes or greater. so pad the frame and send a
     * 17 byte frame. to do the padding just modify the packet
     * length that we provide to the dma. unimac does the extra
     * padding * required to send 64 byte frames.
     */
    if ((len < GMAC_MIN_FRAMESIZE) && (ch->etc->corerev == 0)) {
        ET_PKTSETLEN(ch->dev, p0, GMAC_MIN_FRAMESIZE);
    }

    /* queue the packet based on its priority */
    if (ch->etc->qos)
        q = etc_soc_up2tc(ET_PKTPRIO(ch->dev, p0));

    ASSERT(q < NUMTXQ);
    error = dma_soc_txfast(ch->di[q], p0, TRUE);

    /* set back the orig length */
    ET_PKTSETLEN(ch->dev, p0, len);

    if (error) {
        ET_ERROR(("et%d: chiptx: out of txds\n", ch->etc->unit));
        ch->etc->txnobuf++;
        return FALSE;
    }

    return TRUE;
}

/* reclaim completed transmit descriptors and packets */
static void
chiptxreclaim(ch_t *ch, bool forceall)
{
    int32 i;

    ET_TRACE(("et%d: chiptxreclaim\n", ch->etc->unit));

    for (i = 0; i < NUMTXQ; i++) {
	    if (ch->di[i]) {
            dma_soc_txreclaim(ch->di[i], forceall);
            ch->intstatus &= ~(GMAC2_INTMASK_XMTINTEN_0_MASK << i);
	    }
    }
}

/* dma receive: returns a pointer to the next frame received, or NULL if there are no more */
static void*
chiprx(ch_t *ch)
{
    void *p;
    struct ether_addr *da;
    uint8 *payload = NULL;

    ET_TRACE(("et%d: chiprx\n", ch->etc->unit));
    ET_LOG("et%d: chiprx", ch->etc->unit, 0);

    /* gmac doesn't have a cam to do address filtering. so we implement
     * the multicast address filtering here.
     */
    while ((p = dma_soc_rx(ch->di[BCM5301X_RX_Q_IDX])) != NULL) {
        /* skip the rx header */
        payload = ET_PKTDATA(ch->dev, p) + ETCGMAC_HWRXOFF;

        /* do filtering only for multicast packets when allmulti is false */
        da = (struct ether_addr *)payload;
        if (!ETHER_ISMULTI(da) || ch->etc->allmulti ||
            (gmac_mf_lkup(ch, da) == SUCCESS) || ETHER_ISBCAST(da)) {
            return (p);
        }

        ET_PKTFREE(ch->dev, p, FALSE);
    }

    ch->intstatus &= ~GMAC2_INTMASK_RCVINTEN_MASK;

    /* post more rx buffers since we consumed a few */
    dma_soc_rxfill(ch->di[BCM5301X_RX_Q_IDX]);

    return (NULL);
}

/* reclaim completed dma receive descriptors and packets */
static void
chiprxreclaim(ch_t *ch)
{
    ET_TRACE(("et%d: chiprxreclaim\n", ch->etc->unit));
    dma_soc_rxreclaim(ch->di[BCM5301X_RX_Q_IDX]);
    ch->intstatus &= ~GMAC2_INTMASK_RCVINTEN_MASK;
}

/* allocate and post dma receive buffers */
static void
chiprxfill(ch_t *ch)
{
    ET_TRACE(("et%d: chiprxfill\n", ch->etc->unit));
    ET_LOG("et%d: chiprx", ch->etc->unit, 0);        
    dma_soc_rxfill(ch->di[BCM5301X_RX_Q_IDX]);
}

/* get current and pending interrupt events */
static int
chipgetintrevents(ch_t *ch)
{
    uint32 intstatus;
    int events;

    events = 0;

    /* Check if the core is during reset stage */
    if (!ai_soc_iscoreup(ch->sih)) {
        return (0);
    }

    /* read the interrupt status register */
    intstatus = R_REG(ch->dev, &ch->regs->gmac2_intstatus);

    /* defer unsolicited interrupts */
    intstatus &= DEF_INTMASK;

    if (intstatus != 0) {
        events = INTR_NEW;
    }

    /* or new bits into persistent intstatus */
    intstatus = (ch->intstatus |= intstatus);

    /* return if no events */
    if (intstatus == 0)
        return (0);

    /* convert chip-specific intstatus bits into generic intr event bits */
    if (intstatus & GMAC2_INTMASK_RCVINTEN_MASK) {
        events |= INTR_RX;
    }
    if (intstatus & (GMAC2_INTMASK_XMTINTEN_0_MASK | \
	                 GMAC2_INTMASK_XMTINTEN_1_MASK | \
	                 GMAC2_INTMASK_XMTINTEN_2_MASK | \
	                 GMAC2_INTMASK_XMTINTEN_3_MASK)) {
        events |= INTR_TX;
    }
    if (intstatus & I_ERRORS) {
        events |= INTR_ERROR;
    }

    return (events);
}

/* enable chip interrupts */
static void
chipintrson(ch_t *ch)
{
    ch->intmask = DEF_INTMASK;
    W_REG(ch->dev, &ch->regs->gmac2_intmask, ch->intmask);
}

/* disable chip interrupts */
static void
chipintrsoff(ch_t *ch)
{
    /* disable further interrupts from gmac */
    W_REG(ch->dev, &ch->regs->gmac2_intmask, 0);
    (void) R_REG(ch->dev, &ch->regs->gmac2_intmask);  /* sync readback */
    ch->intmask = 0;

    /* clear the interrupt conditions */
    W_REG(ch->dev, &ch->regs->gmac2_intstatus, ch->intstatus);
}

/* return true of caller should re-initialize, otherwise false */
static bool
chiperrors(ch_t *ch)
{
    uint32 intstatus;
    etc_soc_info_t *etc;

    etc = ch->etc;

    intstatus = ch->intstatus;
    ch->intstatus &= ~(I_ERRORS);

    ET_TRACE(("et%d: chiperrors: intstatus 0x%x\n", etc->unit, intstatus));

    if (intstatus & GMAC2_INTMASK_DESCRERREN_MASK) {
        ET_ERROR(("et%d: descriptor read error\n", etc->unit));
        etc->dmade++;
    }

    if (intstatus & GMAC2_INTMASK_DATAERREN_MASK) {
        ET_ERROR(("et%d: data error\n", etc->unit));
        etc->dmada++;
    }

    if (intstatus & GMAC2_INTMASK_DESCPROTOERREN_MASK) {
        ET_ERROR(("et%d: descriptor programming error\n", etc->unit));
        etc->dmape++;
    }

    if (intstatus & GMAC2_INTMASK_RCVDESCUFEN_MASK) {
        ET_ERROR(("et%d: receive descriptor underflow\n", etc->unit));
        etc->rxdmauflo++;
    }

    if (intstatus & GMAC2_INTMASK_RCVFIFOOFEN_MASK) {
        ET_ERROR(("et%d: receive fifo overflow\n", etc->unit));
        etc->rxoflo++;
    }

    if (intstatus & GMAC2_INTMASK_XMTUFEN_MASK) {
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
    etc_soc_info_t *etc;
    nsgmac2regs_t *regs;
#if 1
    volatile uint32 *s;
    uint32 *d;
#endif

    etc = ch->etc;
    regs = ch->regs;

    /* read the mib counters and update the driver maintained software
     * counters.
     */
    OR_REG(ch->dev, &regs->gmac2_devcontrol, \
           GMAC2_DEVCONTROL_MIB_RD_RESET_EN_MASK);
#if 1
    for (s = &regs->gmac2_tx_gd_octets_lo, d = &ch->mib.gmac2_tx_gd_octets_lo;
         s <=  &regs->gmac2_rxunicastpkts; s++, d++) {
        *d += R_REG(ch->dev, s);
        if (s == &ch->regs->gmac2_txqosq3octet_hi) {
            s = s + 1;
            d = d + 1;
        }
    }
#endif        
    /*
     * Aggregate transmit and receive errors that probably resulted
     * in the loss of a frame are computed on the fly.
     *
     * We seem to get lots of tx_carrier_lost errors when flipping
     * speed modes so don't count these as tx errors.
     *
     * Arbitrarily lump the non-specific dma errors as tx errors.
     */
#if 1
    etc->txerror = ch->mib.gmac2_tx_jab + ch->mib.gmac2_tx_over
        + ch->mib.gmac2_tx_underrun + ch->mib.gmac2_tx_ex_col
        + ch->mib.gmac2_tx_late + etc->txnobuf + etc->dmade
        + etc->dmada + etc->dmape + etc->txuflo;
    etc->rxerror = ch->mib.gmac2_rx_jab + ch->mib.gmac2_rx_ovr
        + ch->mib.gmac2_rx_drop + ch->mib.gmac2_rx_crc_align
        + ch->mib.gmac2_rx_und + ch->mib.gmac2_rx_crc
        + ch->mib.gmac2_rx_align + ch->mib.gmac2_rx_sym
        + etc->rxnobuf + etc->rxdmauflo + etc->rxoflo + etc->rxbadlen;
#endif        
}

static void
chipdumpmib(ch_t *ch, char *buf)
{
    nsgmacmib_t *m;

    m = &ch->mib;

    buf += sprintf((char *) buf, "tx_good_octets %d tx_good_octets_high %d \
        tx_good_pkts %d\n",
        m->gmac2_tx_gd_octets_lo, \
        m->gmac2_tx_gd_octets_hi, \
        m->gmac2_tx_gd_pkts);
    
    buf += sprintf((char *) buf, "tx_octets %d tx_octets_high %d \
        tx_pkts %d\n",
        m->gmac2_tx_all_octets_lo, \
        m->gmac2_tx_all_octets_hi, \
        m->gmac2_tx_all_pkts);

    buf += sprintf((char *) buf, "tx_broadcast_pkts %d tx_multicast_pkts %d \
        tx_uni_pkts %d\n",
        m->gmac2_tx_brdcast, \
        m->gmac2_tx_mult, \
        m->gmac2_txunicastpkt);

    buf += sprintf((char *) buf, "tx_len_64 %d tx_len_65_to_127 %d \
        tx_len_128_to_255 %d\n",
        m->gmac2_tx_64, \
        m->gmac2_tx_65_127, \
        m->gmac2_tx_128_255);
    
    buf += sprintf((char *) buf, "tx_len_256_to_511 %d tx_len_512_to_1023 %d \
        tx_len_1024_to_1522 %d\n",
        m->gmac2_tx_256_511, \
        m->gmac2_tx_512_1023, \
        m->gmac2_tx_1024_1522);

    buf += sprintf((char *) buf, "tx_len_1523_to_2047 %d \
        tx_len_2048_to_4095 %d tx_len_4096_to_8191 %d\n",
        m->gmac2_tx_1523_2047, \
        m->gmac2_tx_2048_4095, \
        m->gmac2_tx_4096_8191);

    buf += sprintf((char *) buf, "tx_len_8192_to_max %d\n",
        m->gmac2_tx_8192_max);
    
    buf += sprintf((char *) buf, "tx_jabber_pkts %d tx_oversize_pkts %d\n",
        m->gmac2_tx_jab, \
        m->gmac2_tx_over);

    buf += sprintf((char *) buf, "tx_fragment_pkts %d tx_underruns %d\n",
        m->gmac2_tx_frag, \
        m->gmac2_tx_underrun);
    
    buf += sprintf((char *) buf, "tx_total_cols %d tx_single_cols %d\n",
        m->gmac2_tx_col, m->gmac2_tx_1_col);
    
    buf += sprintf((char *) buf, "tx_multiple_cols %d tx_excessive_cols %d \
        tx_late_cols %d\n",
        m->gmac2_tx_m_col, m->gmac2_tx_ex_col, m->gmac2_tx_late);
    
    buf += sprintf((char *) buf, "tx_defered %d tx_crs %d tx_pause_pkts %d\n",
        m->gmac2_tx_def, m->gmac2_tx_crs, m->gmac2_tx_paus);

    buf += sprintf((char *) buf, "tx_qos0_pkts %d tx_qos0_octets_lo %d \
	    tx_qos0_octets_hi %d\n",
        m->gmac2_txqosq0pkt, m->gmac2_txqosq0octet_lo, m->gmac2_txqosq0octet_hi);

    buf += sprintf((char *) buf, "tx_qos1_pkts %d tx_qos1_octets_lo %d \
	    tx_qos1_octets_hi %d\n",
        m->gmac2_txqosq1pkt, m->gmac2_txqosq1octet_lo, m->gmac2_txqosq1octet_hi);

    buf += sprintf((char *) buf, "tx_qos2_pkts %d tx_qos2_octets_lo %d \
	    tx_qos2_octets_hi %d\n",
        m->gmac2_txqosq2pkt, m->gmac2_txqosq2octet_lo, m->gmac2_txqosq2octet_hi);

    buf += sprintf((char *) buf, "tx_qos3_pkts %d tx_qos3_octets_lo %d \
	    tx_qos3_octets_hi %d\n",
        m->gmac2_txqosq3pkt, m->gmac2_txqosq3octet_lo, m->gmac2_txqosq3octet_hi);

    buf += sprintf((char *) buf, "rx_good_octets %d rx_good_octets_high %d \
        rx_good_pkts %d\n",
        m->gmac2_rx_gd_octets_lo, \
        m->gmac2_rx_gd_octets_hi, \
        m->gmac2_rx_gd_pkts);
    
    buf += sprintf((char *) buf, "rx_octets %d rx_octets_high %d \
        rx_pkts %d\n",
        m->gmac2_rx_all_octets_lo, \
        m->gmac2_rx_all_octets_hi, \
        m->gmac2_rx_all_pkts);
        
    buf += sprintf((char *) buf, "rx_broadcast_pkts %d rx_multicast_pkts %d \
        rx_uni_pkts %d\n",
        m->gmac2_rx_brdcast, m->gmac2_rx_mult, m->gmac2_rxunicastpkts);
    
    buf += sprintf((char *) buf, "rx_len_64 %d rx_len_65_to_127 %d \
        rx_len_128_to_255 %d\n",
        m->gmac2_rx_64, m->gmac2_rx_65_127, m->gmac2_rx_128_255);
    
    buf += sprintf((char *) buf, "rx_len_256_to_511 %d rx_len_512_to_1023 %d \
        rx_len_1024_to_1522 %d\n",
        m->gmac2_rx_256_511, m->gmac2_rx_512_1023, m->gmac2_rx_1024_1522);

    buf += sprintf((char *) buf, "rx_len_1523_to_2047 %d \
		rx_len_2048_to_4095 %d rx_len_4096_to_8191 %d\n",
        m->gmac2_rx_1523_2047, m->gmac2_rx_2048_4095, m->gmac2_rx_4096_8191);
    
    buf += sprintf((char *) buf, "rx_len_8192_to_max %d\n",
        m->gmac2_rx_8192_max);
    
    buf += sprintf((char *) buf, "rx_jabber_pkts %d rx_oversize_pkts %d \
        rx_fragment_pkts %d\n",
        m->gmac2_rx_jab, m->gmac2_rx_ovr, m->gmac2_rx_frag);
    
    buf += sprintf((char *) buf, "rx_drop_pkts %d rx_crc_align %d\n",
        m->gmac2_rx_drop, m->gmac2_rx_crc_align);
    
    buf += sprintf((char *) buf, "rx_under_size %d rx_crc_errs %d\n",
        m->gmac2_rx_und, m->gmac2_rx_crc);

    buf += sprintf((char *) buf, "rx_align_errs %d rx_symbol_errs %d\n",
        m->gmac2_rx_align, m->gmac2_rx_sym);
    
    buf += sprintf((char *) buf, "rx_pause_pkts %d rx_nonpause_pkts %d \n",
        m->gmac2_rx_paus, m->gmac2_rx_cntrl);
    
    buf += sprintf((char *) buf, "rx_sa_changes %d\n",
        m->gmac2_rxsachanges);
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
    int32 half_duplex, speed;

    cmdcfg = R_REG(ch->dev, &ch->regs->unimac2_command_config);

    /* check if duplex mode changed */
    if (ch->etc->duplex && (cmdcfg & UNIMAC2_COMMAND_CONFIG_HD_ENA_MASK)) {
	    /* Set full-duplex */
        half_duplex = 0;
    } else if (!ch->etc->duplex && \
	           ((cmdcfg & UNIMAC2_COMMAND_CONFIG_HD_ENA_MASK) == 0)) {
	    /* Set half-duplex */
        half_duplex = 1;
    } else {
        /* same as current configuration */
        half_duplex = -1;
    }

    /* check if the speed changed */
    speed = ((cmdcfg & UNIMAC2_COMMAND_CONFIG_ETH_SPEED_MASK) 
        >> UNIMAC2_COMMAND_CONFIG_ETH_SPEED_SHIFT);

    if ((ch->etc->speed == 1000) && (speed != 2)) {
        speed = 2;
    } else if ((ch->etc->speed == 100) && (speed != 1)) {
        speed = 1;
    } else if ((ch->etc->speed == 10) && (speed != 0)) {
        speed = 0;
    } else {
        speed = -1;
    }

    /* no duplex or speed change required */
    if ((speed == -1) && (half_duplex == -1))
        return;

    /* update the speed */
    if (speed != -1) {
        cmdcfg &= ~UNIMAC2_COMMAND_CONFIG_ETH_SPEED_MASK;
        cmdcfg |= (speed << UNIMAC2_COMMAND_CONFIG_ETH_SPEED_SHIFT);
    }

    /* update the duplex mode */
    if (half_duplex != -1) {
        cmdcfg &= ~UNIMAC2_COMMAND_CONFIG_HD_ENA_MASK;
        cmdcfg |= (half_duplex << UNIMAC2_COMMAND_CONFIG_HD_ENA_SHIFT);
    }

    ET_TRACE(("chipduplexupd: updating speed & duplex %x\n", cmdcfg));

    /* put mac in reset */
    gmac_init_reset(ch);

    W_REG(ch->dev, &ch->regs->unimac2_command_config, cmdcfg);

    /* bring mac out of reset */
    gmac_clear_reset(ch);
}

static uint16
chipphyrd(ch_t *ch, uint phyaddr, uint reg)
{
    uint32 tmp;
    nsgmac2regs_t *regs;
    regs = ch->regs;


    ASSERT(phyaddr < MAXEPHY);
    ASSERT(reg < MAXPHYREG);

    if (phyaddr == EPHY_NOREG) {
        ET_ERROR(("et%d: chipphyrd: no phy\n", ch->etc->unit));
        return 0xffff;
    }

    ASSERT(regs != NULL);

	/* issue the read */
	/* PR59036: Need to write phy address to phycontrol in addition to phyaccess */
	tmp = R_REG(ch->dev, &regs->gmac2_phycontrol);
	tmp &= ~GMAC2_PHYCONTROL_EXT_PHY_ADDR_MASK;
	tmp |= (phyaddr & GMAC2_PHYCONTROL_EXT_PHY_ADDR_MASK);
	W_REG(ch->dev, &regs->gmac2_phycontrol, tmp);

	/* set phyaccess for read/write */
    W_REG(ch->dev, &regs->gmac2_phyaccess,
          (GMAC2_PHYACCESS_TRIGGER_MASK | \
          (phyaddr << GMAC2_PHYACCESS_CPU_PHY_ADDR_SHIFT) | \
          (reg << GMAC2_PHYACCESS_CPU_REG_ADDR_SHIFT)));

    /* wait for it to complete */
    SPINWAIT((R_REG(ch->dev, \
              &regs->gmac2_phyaccess) & GMAC2_PHYACCESS_TRIGGER_MASK), 1000);
    tmp = R_REG(ch->dev, &regs->gmac2_phyaccess);
    if (tmp & GMAC2_PHYACCESS_TRIGGER_MASK) {
        ET_ERROR(("et%d: chipphyrd: did not complete\n", ch->etc->unit));
        tmp = 0xffff;
    }


    return (tmp & GMAC2_PHYACCESS_ACC_DATA_MASK);
}

static void
chipphywr(ch_t *ch, uint phyaddr, uint reg, uint16 v)
{
    uint32 tmp;
    nsgmac2regs_t *regs;
    regs = ch->regs;

    ASSERT(phyaddr < MAXEPHY);
    ASSERT(reg < MAXPHYREG);

    if (phyaddr == EPHY_NOREG)
        return;

    ASSERT(regs != NULL);

	/* clear mdioint bit of intstatus first  */
	/* PR59036: Need to write phy address to phycontrol in addition to phyaccess */
	tmp = R_REG(ch->dev, &regs->gmac2_phycontrol);
	tmp &= ~GMAC2_PHYCONTROL_EXT_PHY_ADDR_MASK;
	tmp |= phyaddr;
	W_REG(ch->dev, &regs->gmac2_phycontrol, tmp);
	W_REG(ch->dev, &regs->gmac2_intstatus, GMAC2_INTSTATUS_MDIOINT_MASK);
	ASSERT((R_REG(ch->dev, &regs->gmac2_intstatus) & GMAC2_INTSTATUS_MDIOINT_MASK) == 0);

    /* set phyaccess for read/write */
    W_REG(ch->dev, &regs->gmac2_phyaccess,
          (GMAC2_PHYACCESS_TRIGGER_MASK | GMAC2_PHYACCESS_WR_CMD_MASK | 
          (phyaddr << GMAC2_PHYACCESS_CPU_PHY_ADDR_SHIFT) | 
          (reg << GMAC2_PHYACCESS_CPU_REG_ADDR_SHIFT) | v));

    /* wait for it to complete */
    SPINWAIT((R_REG(ch->dev, \
              &regs->gmac2_phyaccess) & GMAC2_PHYACCESS_TRIGGER_MASK), 1000);
    tmp = R_REG(ch->dev, &regs->gmac2_phyaccess);
    if (tmp & GMAC2_PHYACCESS_TRIGGER_MASK) {
        ET_ERROR(("et%d: chipphywr: did not complete\n", ch->etc->unit));
    }
    
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
    if (phyaddr == EPHY_NOREG)
        return;

    ET_TRACE(("et%d: chipphyinit: phyaddr %d\n", ch->etc->unit, phyaddr));
}

static void
chipphyforce(ch_t *ch, uint phyaddr)
{
    etc_soc_info_t *etc;
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
    etc_soc_info_t *etc;
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


static void 
chiprxreset(ch_t *ch, int id)
{
    dma_soc_rxreset(ch->di[BCM5301X_RX_Q_IDX]);
    dma_soc_rxreclaim(ch->di[BCM5301X_RX_Q_IDX]);
}

static void 
chiprxinit(ch_t *ch, int id)
{
    if(!dma_soc_rxenabled(ch->di[BCM5301X_RX_Q_IDX])) {
        dma_soc_rxinit(ch->di[BCM5301X_RX_Q_IDX]);
    }
}

static bool 
chiprecover(ch_t *ch)
{      
    return FALSE;
}

static int chipflowctrlmodeset(ch_t *ch, uint mode)
{
    etc_soc_info_t *etc;
    nsgmac2regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    COMPILER_REFERENCE(etc);

    ET_TRACE(("et%d: chipflowctrlmodeset\n", etc->unit));

    reg_val = R_REG(ch->dev, &regs->gmac2_devcontrol);
    reg_val &= ~GMAC2_DEVCONTROL_FLOW_CNTL_MODE_MASK;
    switch (mode) {
        case FLOW_CTRL_MODE_DISABLE:
            gmac_txflowcontrol(ch, FALSE);
            return SUCCESS;
        case FLOW_CTRL_MODE_AUTO:
            reg_val |= 0x0 << GMAC2_DEVCONTROL_FLOW_CNTL_MODE_SHIFT;
            break;
        case FLOW_CTRL_MODE_CPU:
            reg_val |= 0x1 << GMAC2_DEVCONTROL_FLOW_CNTL_MODE_SHIFT;
            break;
        case FLOW_CTRL_MODE_MIX:
            reg_val |= 0x2 << GMAC2_DEVCONTROL_FLOW_CNTL_MODE_SHIFT;
            break;
        default:
            return FAILURE;
    }
    gmac_txflowcontrol(ch, TRUE);
    W_REG(ch->dev, &regs->gmac2_devcontrol, reg_val);

    return SUCCESS;
}

static int chipflowctrlmodeget(ch_t *ch, uint *mode)
{
    etc_soc_info_t *etc;
    nsgmac2regs_t *regs;
    uint32 reg_val;
    uint32 cmdcfg;

    etc = ch->etc;
    regs = ch->regs;

    COMPILER_REFERENCE(etc);

    ET_TRACE(("et%d: chipflowctrlmodeget\n", etc->unit));

    cmdcfg = R_REG(ch->dev, &ch->regs->unimac2_command_config);
    if (cmdcfg & UNIMAC2_COMMAND_CONFIG_PAUSE_IGNORE_MASK) {
        *mode = FLOW_CTRL_MODE_DISABLE;
        return SUCCESS;
    }

    reg_val = R_REG(ch->dev, &regs->gmac2_devcontrol);
    switch ((reg_val & GMAC2_DEVCONTROL_FLOW_CNTL_MODE_MASK) >> 
        GMAC2_DEVCONTROL_FLOW_CNTL_MODE_SHIFT) {
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
    etc_soc_info_t *etc;
    nsgmac2regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    COMPILER_REFERENCE(etc);

    ET_TRACE(("et%d: chipflowctrlautoset\n", etc->unit));

    if ((on_th > 0xffff) || (off_th > 0xffff)) {
        return FAILURE;
    }

    /* Set the value of Pause on/off threashod */
    reg_val = R_REG(ch->dev, &regs->gmac2_flowcntl_th);
    reg_val &= ~GMAC2_FLOWCNTL_TH_REGTORXQ_FLOW_CNTL_ON_TH_MASK;
    reg_val |= on_th << GMAC2_FLOWCNTL_TH_REGTORXQ_FLOW_CNTL_ON_TH_SHIFT;
    reg_val &= ~GMAC2_FLOWCNTL_TH_REGTORXQ_FLOW_CNTL_OFF_TH_MASK;
    reg_val |= off_th << GMAC2_FLOWCNTL_TH_REGTORXQ_FLOW_CNTL_OFF_TH_SHIFT;
    W_REG(ch->dev, &regs->gmac2_flowcntl_th, reg_val);

    return SUCCESS;
}

static int chipflowctrlautoget(ch_t *ch, uint *on_th, uint *off_th)
{
    etc_soc_info_t *etc;
    nsgmac2regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    COMPILER_REFERENCE(etc);

    ET_TRACE(("et%d: chipflowctrlautoget\n", etc->unit));

    /* Get the value of Pause on/off threashod */
    reg_val = R_REG(ch->dev, &regs->gmac2_flowcntl_th);
    *on_th = (reg_val & GMAC2_FLOWCNTL_TH_REGTORXQ_FLOW_CNTL_ON_TH_MASK) >> 
        GMAC2_FLOWCNTL_TH_REGTORXQ_FLOW_CNTL_ON_TH_SHIFT;
    *off_th = (reg_val & GMAC2_FLOWCNTL_TH_REGTORXQ_FLOW_CNTL_OFF_TH_MASK) >> 
        GMAC2_FLOWCNTL_TH_REGTORXQ_FLOW_CNTL_OFF_TH_SHIFT;

    return SUCCESS;
}

static int chipflowctrlcpuset(ch_t *ch, uint pause_on)
{
    etc_soc_info_t *etc;
    nsgmac2regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    COMPILER_REFERENCE(etc);

    ET_TRACE(("et%d: chipflowctrlcpuset\n", etc->unit));

    /* Set the pause on value */
    reg_val = R_REG(ch->dev, &regs->gmac2_devcontrol);
    reg_val &= ~GMAC2_DEVCONTROL_CPU_FLOW_CNTL_ON_MASK;
    if (pause_on) {
        reg_val |= 1 << GMAC2_DEVCONTROL_CPU_FLOW_CNTL_ON_SHIFT;
    }
    W_REG(ch->dev, &regs->gmac2_devcontrol, reg_val);

    return SUCCESS;
}

static int chipflowctrlcpuget(ch_t *ch, uint *pause_on)
{
    etc_soc_info_t *etc;
    nsgmac2regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    COMPILER_REFERENCE(etc);

    ET_TRACE(("et%d: chipflowctrlcpuget\n", etc->unit));

    /* Get the pause on value */
    reg_val = R_REG(ch->dev, &regs->gmac2_devcontrol);
    *pause_on = (reg_val & GMAC2_DEVCONTROL_CPU_FLOW_CNTL_ON_MASK) >> 
        GMAC2_DEVCONTROL_CPU_FLOW_CNTL_ON_SHIFT;;

    return SUCCESS;
}

static int chipflowctrlrxchanset(ch_t *ch, uint chan, uint on_th, uint off_th)
{
    return FALSE;
}

static int chipflowctrlrxchanget(ch_t *ch, uint chan, uint *on_th, uint *off_th)
{
    return FALSE;
}

static int
chiptpidset(ch_t *ch, uint index, uint tpid)
{
    return FALSE;
}

static int
chiptpidget(ch_t *ch, uint index, uint *tpid)
{
    return FALSE;
}

static int chippvtagset(ch_t *ch, uint private_tag)
{
    return FALSE;
}

static int chippvtagget(ch_t *ch, uint *private_tag)
{
    return FALSE;
}

static int chiprxsephdrset(ch_t *ch, uint enable)
{
    etc_soc_info_t *etc;
    nsgmac2regs_t *regs;
    uint32 reg_val, field_val = 0;

    etc = ch->etc;
    regs = ch->regs;

    COMPILER_REFERENCE(etc);

    ET_TRACE(("et%d: chiprxsephdrset\n", etc->unit));

    if (enable) {
        field_val = 1;
    }

    reg_val = R_REG(ch->dev, &regs->gmac2_rcvcontrol);
    reg_val &= ~GMAC2_RCVCONTROL_SEPRXHDRDESCEN_MASK;
    reg_val |= field_val << GMAC2_RCVCONTROL_SEPRXHDRDESCEN_SHIFT;
    W_REG(ch->dev, &regs->gmac2_rcvcontrol, reg_val);

    return SUCCESS;
}

static int chiprxsephdrget(ch_t *ch, uint *enable)
{
    etc_soc_info_t *etc;
    nsgmac2regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    COMPILER_REFERENCE(etc);

    ET_TRACE(("et%d: chiprxsephdrget\n", etc->unit));

    reg_val = R_REG(ch->dev, &regs->gmac2_rcvcontrol);
    *enable = (reg_val & GMAC2_RCVCONTROL_SEPRXHDRDESCEN_MASK) >> 
        GMAC2_RCVCONTROL_SEPRXHDRDESCEN_SHIFT;

    return SUCCESS;
}

static int chiptxqosmodeset(ch_t *ch, uint mode)
{
    etc_soc_info_t *etc;
    nsgmac2regs_t *regs;

    etc = ch->etc;
    regs = ch->regs;

    COMPILER_REFERENCE(etc);
    COMPILER_REFERENCE(regs);

    ET_TRACE(("et%d: chiptxqosmodeset\n", etc->unit));

    /* Northstar: only all WRR mode */
    switch (mode) {
        case TXQOS_MODE_ALL_WRR:
            break;
        default:
            return FAILURE;
    }

    return SUCCESS;
}

static int chiptxqosmodeget(ch_t *ch, uint *mode)
{
    etc_soc_info_t *etc;

    etc = ch->etc;

    COMPILER_REFERENCE(etc);

    ET_TRACE(("et%d: chipflowctrlmodeget\n", etc->unit));

    *mode = TXQOS_MODE_ALL_WRR;

    return SUCCESS;
}

static int chiptxqosweightset(ch_t *ch, uint queue, uint weight)
{
    etc_soc_info_t *etc;
    nsgmac2regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    COMPILER_REFERENCE(etc);

    ET_TRACE(("et%d: chiptxqosweightset\n", etc->unit));

    if (queue >= NUMTXQ) {
        return FAILURE;
    }

    /* Value Checking */ 
    if (weight > 0xff) {
        return FAILURE;
    }

    /* Set the pause on/off threshold of specific channel */
    reg_val = R_REG(ch->dev, &regs->gmac2_txarb_wrr_th);
    reg_val &= ~(GMAC2_TXARB_WRR_TH_REGTOTXARB_WRR_TH_0_MASK << 
        (queue * GMAC2_TXARB_WRR_TH_REGTOTXARB_WRR_TH_1_SHIFT));
    reg_val |= weight << (queue * GMAC2_TXARB_WRR_TH_REGTOTXARB_WRR_TH_1_SHIFT);
    W_REG(ch->dev, &regs->gmac2_txarb_wrr_th, reg_val);

    return SUCCESS;
}

static int chiptxqosweightget(ch_t *ch, uint queue, uint *weight)
{
    etc_soc_info_t *etc;
    nsgmac2regs_t *regs;
    uint32 reg_val;

    etc = ch->etc;
    regs = ch->regs;

    COMPILER_REFERENCE(etc);

    ET_TRACE(("et%d: chiptxqosweightget\n", etc->unit));

    if (queue >= NUMTXQ) {
        return FAILURE;
    }

    /* Get the pause on/off threshold of specific channel */
    reg_val = R_REG(ch->dev, &regs->gmac2_txarb_wrr_th);
    *weight = (reg_val >> (queue * GMAC2_TXARB_WRR_TH_REGTOTXARB_WRR_TH_1_SHIFT)) 
        && GMAC2_TXARB_WRR_TH_REGTOTXARB_WRR_TH_0_MASK;

    return SUCCESS;
}

/* Dummy routines */
static int chiprxrateset(ch_t *ch, uint channel, uint pps)
{
    return FALSE;
}

static int chiprxrateget(ch_t *ch, uint channel, uint *pps)
{
    return FALSE;
}

static int chiptxrateset(ch_t *ch, uint channel, uint rate, uint burst)
{
    return FALSE;
}


static int chiptxrateget(ch_t *ch, uint channel, uint *rate, uint *burst)
{
    return FALSE;
}

static int 
chipcfprd(ch_t *ch, void *buf)
{
    return FALSE;
}

static int 
chipcfpwr(ch_t *ch, void *buf)
{
    return FALSE;
}

static int 
chipcfpfldrd(ch_t *ch, void *buf)
{
    return FALSE;
}
static int 
chipcfpfldwr(ch_t *ch, void *buf)
{
    return FALSE;
}

static int 
chipcfpudfrd(ch_t *ch, void *buf)
{
    return FALSE;
}

static int 
chipcfpudfwr(ch_t *ch, void *buf)
{
    return FALSE;
}


#endif /* defined(IPROC_CMICD) */
