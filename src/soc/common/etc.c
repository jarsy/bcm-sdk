/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * Common [OS-independent] portion of
 * Broadcom Home Networking Division 10/100 Mbit/s Ethernet
 * Device Driver.
 *
 * $Id: etc.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <shared/et/osl.h>
#include <shared/et/bcmendian.h>
#include <shared/et/bcmutils.h>
#include <shared/et/proto/ethernet.h>
#include <shared/et/bcmenetrxh.h>
#include <shared/et/et_dbg.h>
#include <shared/et/et_export.h>
#include <shared/et/proto/802.1d.h>
#include <soc/etc.h>

#if defined(KEYSTONE) || defined(ROBO_4704) || defined(IPROC_CMICD)

#if defined(VXWORKS) || defined(__ECOS)
extern int et_msg_level;
#else
int et_msg_level =
#ifdef BCMDBG
	1;
#else
	0;
#endif
#endif

/* local prototypes */
static void etc_soc_loopback(etc_soc_info_t *etc, int on);
static void etc_soc_dumpetc(etc_soc_info_t *etc, uchar *buf);

/* 802.1d priority to traffic class mapping. queues correspond one-to-one
 * with traffic classes.
 */
uint32 soc_up2tc[NUMPRIO] = {
    TC_BE,      /* 0    BE    TC_BE    Best Effort */
    TC_BK,      /* 1    BK    TC_BK    Background */
    TC_BK,      /* 2    --    TC_BK    Background */
    TC_BE,      /* 3    EE    TC_BE    Best Effort */
    TC_CL,      /* 4    CL    TC_CL    Controlled Load */
    TC_CL,      /* 5    VI    TC_CL    Controlled Load */
    TC_VO,      /* 6    VO    TC_VO    Voice */
    TC_VO       /* 7    NC    TC_VO    Voice */
};

uint32 soc_priq_selector[] = {
    /* 0x0 */ TC_NONE, /* 0x1 */ TC_BK, /* 0x2 */ TC_BE, /* 0x3 */ TC_BE,
    /* 0x4 */ TC_CL,   /* 0x5 */ TC_CL, /* 0x6 */ TC_CL, /* 0x7 */ TC_CL,
    /* 0x8 */ TC_VO,   /* 0x9 */ TC_VO, /* 0xa */ TC_VO, /* 0xb */ TC_VO,
    /* 0xc */ TC_VO,   /* 0xd */ TC_VO, /* 0xe */ TC_VO, /* 0xf */ TC_VO
};

/* find the chip opsvec for this chip */
struct chops*
etc_soc_chipmatch(uint vendor, uint device)
{
#ifdef BCM4413_CHOPS
    {
    extern struct chops bcm4413_chops;
    if (bcm4413_chops.id(vendor, device))
        return (&bcm4413_chops);
    }
#endif

#if defined(ROBO_4704)
   {
    extern struct chops bcm47xx_et_soc_chops;
    if (bcm47xx_et_soc_chops.id(vendor, device)) {
        return (&bcm47xx_et_soc_chops);
    }
    }
#endif
    
#if defined(KEYSTONE)
    {
    extern struct chops bcmgmac_et_soc_chops;
    if (bcmgmac_et_soc_chops.id(vendor, device)) {
        return (&bcmgmac_et_soc_chops);
    }
    }
#endif
#if defined(IPROC_CMICD)
    {
    extern struct chops bcm_nsgmac_et_soc_chops;
    if (bcm_nsgmac_et_soc_chops.id(vendor, device)) {
        return (&bcm_nsgmac_et_soc_chops);
    }
    }
#endif

    return (NULL);
}

void*
etc_soc_attach(void *et, uint vendor, uint device, uint unit, 
    void *pdev, void *regsva)
{
    etc_soc_info_t *etc;
    int i;

    ET_TRACE(("et%d: etc_soc_attach: vendor 0x%x device 0x%x\n", 
        unit, vendor, device));

    /* some code depends on packed structures */
    ASSERT(sizeof (struct ether_addr) == ETHER_ADDR_LEN);
    ASSERT(sizeof (struct ether_header) == ETHER_HDR_LEN);

    /* allocate etc_soc_info_t state structure */
    if ((etc = (etc_soc_info_t*) ET_MALLOC(sizeof (etc_soc_info_t))) == NULL) {
        ET_ERROR(("et%d: etc_soc_attach malloc failed\n", unit));
        return (NULL);
    }
    bzero((char*)etc, sizeof (etc_soc_info_t));


    /* perform any osl specific init */
    osl_init();

    etc->et = et;
    etc->unit = unit;
    etc->vendorid = (uint16) vendor;
    etc->deviceid = (uint16) device;
#ifdef _BCM5365_FPGA_
    etc->forcespeed = ET_10FULL;
    etc->linkstate = TRUE;
#else
    etc->forcespeed = ET_AUTO;
    etc->linkstate = FALSE;
#endif

    etc->promisc = 1;
    etc->pkt_mem = MEMORY_DDRRAM;
    /* separate header buffer memory type */
    etc->pkthdr_mem = MEMORY_DDRRAM;
    /* flow control */
    etc->flowcntl_mode = FLOW_CTRL_MODE_DISABLE; /* default mode */
    etc->flowcntl_auto_on_thresh = FLWO_CTRL_AUTO_MODE_DEFAULT_ON_THRESH;
    etc->flowcntl_auto_off_thresh = FLWO_CTRL_AUTO_MODE_DEFAULT_OFF_THRESH;
    etc->flowcntl_cpu_pause_on = 0;
    for (i=0; i < NUMRXQ; i++) {
        etc->flowcntl_rx_on_thresh[i] = 0;
        etc->flowcntl_rx_off_thresh[i] = 0;
        etc->en_rxsephdr[i] = 0;
    }

    /* tx qos */
    etc->txqos_mode = TXQOS_MODE_1STRICT_3WRR;
    for (i=0; i < NUMTXQ; i++) {
        etc->txqos_weight[i] = TXQOS_DEFAULT_WEIGHT;
    }

    /* tpid */
    for (i=0; i < NUM_STAG_TPID; i++) {
        etc->tpid[i] = STAG_TPID_DEFAULT;
    }

    /* set chip opsvec */
    etc->chops = etc_soc_chipmatch(vendor, device);
    ASSERT(etc->chops);

    /* chip attach */
    if ((etc->ch = (*etc->chops->attach)(etc, pdev, regsva)) == NULL) {
        ET_ERROR(("et%d: chipattach error\n", unit));
        goto fail;
    }

    return ((void*)etc);

fail:
    etc_soc_detach(etc);
    return (NULL);
}

void
etc_soc_detach(etc_soc_info_t *etc)
{
    if (etc == NULL)
    	return;

    /* free chip private state */
    if (etc->ch) {
        (*etc->chops->detach)(etc->ch);
        etc->chops = etc->ch = NULL;
    }

    ET_MFREE(etc, sizeof (etc_soc_info_t));
}

void
etc_soc_reset(etc_soc_info_t *etc)
{
    ET_TRACE(("et%d: etc_soc_reset\n", etc->unit));

    etc->reset++;

    /* reset the chip */
    (*etc->chops->reset)(etc->ch);

    /* free any posted tx packets */
    (*etc->chops->txreclaim)(etc->ch, TRUE);

    /* free any posted rx packets */
    (*etc->chops->rxreclaim)(etc->ch);
}

void
etc_soc_debug(etc_soc_info_t *etc)
{
    uchar *buf = NULL;

    /* allocate buffer for dump messages */
    buf = sal_alloc(8000, "soc_debug");
    if (buf == NULL) {
        return;
    }
    
    (*etc->chops->dump)(etc->ch, (char *) buf);
    LOG_CLI((BSL_META(" %s"), buf));

    sal_free(buf);
    
}


void
etc_soc_init(etc_soc_info_t *etc, bool full)
{
    ET_TRACE(("et%d: etc_soc_init\n", etc->unit));

    ASSERT(etc->pioactive == NULL);
    ASSERT(!ETHER_ISNULLADDR(&etc->cur_etheraddr));
    ASSERT(!ETHER_ISMULTI(&etc->cur_etheraddr));

    /* init the chip */
    (*etc->chops->init)(etc->ch, full);
}

/* mark interface up */
void
etc_soc_up(etc_soc_info_t *etc)
{
    etc->up = TRUE;

    et_soc_init(etc->et, FALSE);
}

/* mark interface down */
uint
etc_soc_down(etc_soc_info_t *etc, int reset)
{
    uint callback;

    callback = 0;

    etc->up = FALSE;
    if (reset)
        et_soc_reset(etc->et);

    /* suppress link state changes during power management mode changes */
    if (etc->linkstate) { 
        etc->linkstate = FALSE;
        if (!etc->pm_modechange)
            et_soc_link_down(etc->et);
    }

    return (callback);
}

/* common ioctl handler.  return: 0=ok, -1=error */
int
etc_soc_ioctl(etc_soc_info_t *etc, int cmd, void *arg)
{
    int error;
    int val;
    int *vec = (int*)arg;

    error = 0;

    val = arg? *(int*)arg: 0;

    ET_TRACE(("et%d: etc_soc_ioctl: cmd 0x%x\n", etc->unit, cmd));

    switch (cmd) {
    case ETCUP:
        et_soc_up(etc->et);
        break;

    case ETCDOWN:
        et_soc_down(etc->et, TRUE);
        break;

    case ETCLOOP:
        etc_soc_loopback(etc, val);
        break;

    case ETCDUMP:
        if (et_msg_level & 0x10000) {
            if (arg) {
                bcmdumplog((uchar*)arg, 4096);
            }
        }
        break;

    case ETCSETMSGLEVEL:
        et_msg_level = val;
        break;

    case ETCPROMISC:
        etc_soc_promisc(etc, val);
        break;

    case ETCQOS:
        etc_soc_qos(etc, val);
        break;

    case ETCSPEED:
        if ((val != ET_AUTO) && (val != ET_10HALF) && (val != ET_10FULL)
            && (val != ET_100HALF) && (val != ET_100FULL))
            goto err;
        etc->forcespeed = val;

        /* explicitly reset the phy */
        (*etc->chops->phyreset)(etc->ch, etc->phyaddr);

        /* request restart autonegotiation if we're reverting to adv mode */
        if ((etc->forcespeed == ET_AUTO) & etc->advertise)
            etc->needautoneg = TRUE;

        et_soc_init(etc->et, FALSE);
        break;

    case ETCPHYRD:
        if (vec)
            vec[1] = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, vec[0]);
        break;

    case ETCPHYWR:
        if (vec)
            (*etc->chops->phywr)(etc->ch, etc->phyaddr, vec[0], 
            (uint16) vec[1]);
        break;

    case ETCCFPRD:
        (*etc->chops->cfprd)(etc->ch, arg);
        break;
    case ETCCFPWR:
        (*etc->chops->cfpwr)(etc->ch, arg);
        break;
    case ETCCFPFIELDRD:
        (*etc->chops->cfpfldrd)(etc->ch, arg);
        break;
    case ETCCFPFIELDWR:
        (*etc->chops->cfpfldwr)(etc->ch, arg);
        break;
    case ETCCFPUDFRD:
        (*etc->chops->cfpudfrd)(etc->ch, arg);
        break;
    case ETCCFPUDFWR:
        (*etc->chops->cfpudfwr)(etc->ch, arg);
        break;
    case ETCPKTMEMSET:
        if ((val != MEMORY_DDRRAM) && (val != MEMORY_SOCRAM) 
            && (val != MEMORY_PCIMEM))
            goto err;
        etc->pkt_mem = val;

        et_soc_init(etc->et, TRUE);
        break;
    case ETCPKTHDRMEMSET:
        if ((val != MEMORY_DDRRAM) && (val != MEMORY_SOCRAM) 
            && (val != MEMORY_PCIMEM))
            goto err;
        etc->pkthdr_mem = val;

        et_soc_init(etc->et, TRUE);
        break;
    case ETCRXRATE:
         if (vec) {
            uint chan, pps;
            chan = vec[0];
            pps = vec[1];
            error = (*etc->chops->rxrateset)(etc->ch, chan, pps);
            if (!error) {
                etc->rx_pps[chan] = pps;
            }
        } else {
            goto err;
        }
        break;
    case ETCTXRATE:
         if (vec) {
            uint chan, rate, burst;
            chan = vec[0];
            rate = vec[1];
            burst = vec[2];
            error = (*etc->chops->txrateset)(etc->ch, chan, rate, burst);
            if (!error) {
                etc->tx_rate[chan] = rate;
                etc->tx_burst[chan] = burst;
            }
        }else {
            goto err;
        }
        break;
    case ETCFLOWCTRLMODE:
        if (vec) {
            uint mode;

            mode = vec[0];
            error = (*etc->chops->flowctrlmodeset)(etc->ch, mode);
            if (!error) {
                etc->flowcntl_mode = mode;
                et_soc_init(etc->et, FALSE);
            }
        }else {
            goto err;
        }
        break;
    case ETCFLOWCTRLCPUSET:
        if (vec) {
            uint pause_on;

            pause_on = vec[0];
            error = (*etc->chops->flowctrlcpuset)(etc->ch, pause_on);
            if (!error) {
                etc->flowcntl_cpu_pause_on= pause_on;
            }
        }else {
            goto err;
        }
        break;
    case ETCFLOWCTRLAUTOSET:
        if (vec) {
            uint on_th, off_th;

            on_th = vec[0];
            off_th = vec[1];
            error = (*etc->chops->flowctrlautoset)(etc->ch, on_th, off_th);
            if (!error) {
                etc->flowcntl_auto_on_thresh= on_th;
                etc->flowcntl_auto_off_thresh= off_th;
            }
        }else {
            goto err;
        }
        break;
    case ETCFLOWCTRLRXCHANSET:
        if (vec) {
            uint rxchan, on_th, off_th;

            rxchan = vec[0];
            if (rxchan >= NUMRXQ) {
                goto err;
            }
            on_th = vec[1];
            off_th = vec[2];
            error = (*etc->chops->flowctrlrxchanset)(etc->ch, rxchan, on_th, off_th);
            if (!error) {
                etc->flowcntl_rx_on_thresh[rxchan]= on_th;
                etc->flowcntl_rx_off_thresh[rxchan]= off_th;
            }
        }else {
            goto err;
        }
        break;
    case ETCTPID:
         if (vec) {
            uint id, tpid;
            id = vec[0];
            tpid = vec[1];
            error = (*etc->chops->tpidset)(etc->ch, id, tpid);
            if (!error) {
                etc->tpid[id] = tpid;
            }
        } else {
            goto err;
        }
        break;
    case ETCPVTAG:
        if (vec) {
            uint ptag;
            ptag = vec[0];
            error = (*etc->chops->pvtagset)(etc->ch, ptag);
            if (!error) {
                etc->ptag = ptag;
            }
        } else {
            goto err;
        }
        break;

    /* RX separate header */
    case ETCRXSEPHDR:
        if (vec) {
            uint enable;
            uint i;
            enable = vec[0];
            error = (*etc->chops->rxsephdrset)(etc->ch, enable);
            if (!error) {
                for (i=0; i < NUMRXQ; i++) {
                    etc->en_rxsephdr[i]= enable;
                }
            }
        } else {
            goto err;
        }
        et_soc_init(etc->et, FALSE);
        break;
    case ETCTXQOSMODE:
        if (vec) {
            uint mode;

            mode = vec[0];
            error = (*etc->chops->txqosmodeset)(etc->ch, mode);
            if (!error) {
                etc->txqos_mode = mode;
            }
        }else {
            goto err;
        }
        break;
    case ETCTXQOSWEIGHTSET:
        if (vec) {
            uint queue, weight;

            queue = vec[0];
            if (queue >= NUMTXQ) {
                goto err;
            }
            weight = vec[1];
            error = (*etc->chops->txqosweightset)(etc->ch, queue, weight);
            if (!error) {
                etc->txqos_weight[queue]= weight;
            }
        }else {
            goto err;
        }
        break;
        
    default:
    err:
        error = -1;
    }

    return (error);
}

/* called once per second */
void
etc_soc_watchdog(etc_soc_info_t *etc)
{
    uint16 status;
    uint16 lpa;
    
    etc->now++;

    if (etc->phyaddr == PHY_NOMDC) {	    
        etc->linkstate = TRUE;
        etc->speed = 100;
        etc->duplex = 1;
        return;
    }

    status = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, 1);
    /* check for bad mdio read */
    if (status == 0xffff) {
        ET_ERROR(("et%d: etc_soc_watchdog: bad mdio read: phyaddr %d mdcport %d\n",
            etc->unit, etc->phyaddr, etc->mdcport));
        return;
    }

    if ((etc->forcespeed == ET_AUTO) && (!etc->ext_config)) {
    	uint16 adv, adv2 = 0, status2 = 0, estatus;

    	adv = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, 4);
    	lpa = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, 5);

    	/* read extended status register. if we are 1000BASE-T
    	 * capable then get our advertised capabilities and the
    	 * link partner capabilities from 1000BASE-T control and
    	 * status registers.
    	 */
    	estatus = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, 15);
    	if ((estatus != 0xffff) && (estatus & EST_1000TFULL)) {
    		/* read 1000BASE-T control and status registers */
    		adv2 = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, 9);
    		status2 = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, 10);
    	}

    	/* update current speed and duplex */
    	if ((adv2 & ADV_1000FULL) && (status2 & LPA_1000FULL)) {
    		etc->speed = 1000;
    		etc->duplex = 1;
    	} else if ((adv2 & ADV_1000HALF) && (status2 & LPA_1000HALF)) {
    		etc->speed = 1000;
    		etc->duplex = 0;
    	} else if ((adv & ADV_100FULL) && (lpa & LPA_100FULL)) {
    		etc->speed = 100;
    		etc->duplex = 1;
    	} else if ((adv & ADV_100HALF) && (lpa & LPA_100HALF)) {
    		etc->speed = 100;
    		etc->duplex = 0;
    	} else if ((adv & ADV_10FULL) && (lpa & LPA_10FULL)) {
    		etc->speed = 10;
    		etc->duplex = 1;
    	} else {
    		etc->speed = 10;
    		etc->duplex = 0;
    	}
    }

    /* monitor link state */
    if (!etc->linkstate && (status & STAT_LINK)) {
        etc->linkstate = TRUE;

        if (etc->pm_modechange)
            etc->pm_modechange = FALSE;
        else
            et_soc_link_up(etc->et);
    }
    else if (etc->linkstate && !(status & STAT_LINK)) {
        etc->linkstate = FALSE;
        if (!etc->pm_modechange)
            et_soc_link_down(etc->et);
    }

    /* keep emac txcontrol duplex bit consistent with current phy duplex */
    (*etc->chops->duplexupd)(etc->ch);

    /* check for remote fault error */
    if (status & STAT_REMFAULT) {
        ET_ERROR(("et%d: remote fault\n", etc->unit));
    }

    /* check for jabber error */
    if (status & STAT_JAB) {
        ET_ERROR(("et%d: jabber\n", etc->unit));
    }

    /*
     * Read chip mib counters occationally before the 16bit ones can wrap.
     * We don't use the high-rate mib counters.
     */
    if ((etc->now % 30) == 0)
        (*etc->chops->statsupd)(etc->ch);
}

static void
etc_soc_loopback(etc_soc_info_t *etc, int on)
{
    ET_TRACE(("et%d: etc_soc_loopback: %d\n", etc->unit, on));

    etc->loopbk = (bool) on;
    et_soc_init(etc->et, FALSE);
}

void
etc_soc_promisc(etc_soc_info_t *etc, uint on)
{
    ET_TRACE(("et%d: etc_soc_promisc: %d\n", etc->unit, on));

    etc->promisc = (bool) on;
    et_soc_init(etc->et, FALSE);
}

void
etc_soc_qos(etc_soc_info_t *etc, uint on)
{
    ET_TRACE(("et%d: etc_soc_qos: %d\n", etc->unit, on));

    etc->qos = (bool) on;
    et_soc_init(etc->et, FALSE);
}


void
etc_soc_dump(etc_soc_info_t *etc, uchar *buf, int size)
{
    /* big enough */
    if (size < 3700)
        return;

    etc_soc_dumpetc(etc, buf);
    (*etc->chops->dump)(etc->ch, (char *)buf + strlen((char *) buf));
}

static void
etc_soc_dumpetc(etc_soc_info_t *etc, uchar *buf)
{
    char perm[32], cur[32];
    uint i;

    buf += sprintf((char *) buf, 
        "etc 0x%x et 0x%x unit %d msglevel %d speed/duplex %d%s\n",
        (uint)etc, (uint)etc->et, etc->unit, et_msg_level,
        etc->speed, (etc->duplex? "full": "half"));
    buf += sprintf((char *) buf, 
    "up %d promisc %d loopbk %d forcespeed %d advertise 0x%x needautoneg %d\n",
        etc->up, etc->promisc, etc->loopbk, 
        etc->forcespeed, etc->advertise, etc->needautoneg);
    buf += sprintf((char *) buf,
                   "piomode %d pioactive 0x%x nmulticast %d allmulti %d\n",
        etc->piomode, (uint)etc->pioactive, etc->nmulticast, etc->allmulti);
    buf += sprintf((char *) buf, 
        "vendor 0x%x device 0x%x rev %d coreunit %d phyaddr %d mdcport %d\n",
        etc->vendorid, etc->deviceid, etc->chiprev,
        etc->coreunit, etc->phyaddr, etc->mdcport);

    buf += sprintf((char *) buf, "perm_etheraddr %s cur_etheraddr %s\n",
        bcm_ether_ntoa((char*)&etc->perm_etheraddr, perm),
        bcm_ether_ntoa((char*)&etc->cur_etheraddr, cur));

    if (etc->nmulticast) {
        buf += sprintf((char *) buf, "multicast: ");
        for (i = 0; i < etc->nmulticast; i++)
            buf += sprintf((char *) buf, "%s ", 
            bcm_ether_ntoa((char*)&etc->multicast[i], cur));
        buf += sprintf((char *) buf, "\n");
    }

    buf += sprintf((char *) buf, "linkstate %d\n", etc->linkstate);
    buf += sprintf((char *) buf, "\n");

    /* refresh stat counters */
    (*etc->chops->statsupd)(etc->ch);

    /* summary stat counter line */
    /* use sw frame and byte counters -- */
    (*etc->chops->dumpmib)(etc->ch, (char *)buf);
    
}

uint
etc_soc_control_totlen(etc_soc_info_t *etc, void *p)
{
    uint total;

    total = 0;
    for (; p; p = ET_PKTNEXT(etc->unit, p))
        total += ET_PKTLEN(etc->unit, p);
    return (total);
}

uint32
etc_soc_up2tc(uint32 up)
{
    /* Checking if the priority is overflow , return the max  queue number */ 
    if (up > MAXPRIO) {
        return TC_VO;
    }

    return (soc_up2tc[up]);
}

uint32
etc_soc_priq(uint32 txq_state)
{
    /* Checking if the input value is overflow */
    if (txq_state > ((0x1 << NUMTXQ)-1)) {
        txq_state &= ((0x1 << NUMTXQ)-1); 
    }

    return (soc_priq_selector[txq_state]);
}

#ifdef BCMDBG
void
etc_soc_prhdr(char *msg, struct ether_header *eh, uint len)
{
    char da[32], sa[32];

    LOG_CLI((BSL_META("%s: dst %s src %s type 0x%x len %d\n"),
             msg,
             bcm_ether_ntoa(eh->ether_dhost, da),
             bcm_ether_ntoa(eh->ether_shost, sa),
             ntoh16(eh->ether_type),
             len));
}

void
etc_soc_prhex(char *msg, uchar *buf, uint nbytes, int unit)
{
    if (msg && (msg[0] != '\0'))
        LOG_CLI((BSL_META_U(unit,
                            "et%d: %s:\n"), unit, msg));
    else
        LOG_CLI((BSL_META_U(unit,
                            "et%d:\n"), unit));

    prhex(NULL, buf, nbytes);
}
#endif /* BCMDBG */

#endif /* defined(KEYSTONE) || defined(ROBO_4704)  || defined(IPROC_CMICD)*/
