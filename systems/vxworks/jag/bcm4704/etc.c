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
 * Copyright(c) 2000 Broadcom, Inc.
 * $Id: etc.c,v 1.4 2007/10/08 22:30:43 iakramov Exp $
 */

#include <osl.h>
#include <bcmendian.h>
#include <ethernet.h>
#include <bcmenetmib.h>
#include <bcmenetrxh.h>
#include <et_dbg.h>
#include <etc.h>
#include <et_export.h>
#include <bcmutils.h>
#include <mbz.h>

int et_msg_level =
#ifdef BCMDBG
	0xffffffff;   /* This is normally "1" */
#else
	0;
#endif

/* local prototypes */
static void etc_loopback(etc_info_t *etc, int on);
static void etc_dumpetc(etc_info_t *etc, uchar *buf);

/* find the chip opsvec for this chip */
struct chops*
etc_chipmatch(uint vendor, uint device)
{
#ifdef BCM4413_CHOPS
	{
	extern struct chops bcm4413_chops;
	if (bcm4413_chops.id(vendor, device))
		return (&bcm4413_chops);
	}
#endif
#ifdef BCM47XX_CHOPS
	{
	extern struct chops bcm47xx_et_chops;
	if (bcm47xx_et_chops.id(vendor, device))
		return (&bcm47xx_et_chops);
	}
#endif
	return (NULL);
}

void*
etc_attach(void *et, uint vendor, uint device, uint unit, void *pdev, void *regsva)
{
    extern int sysIsLM();
	etc_info_t *etc;

	ET_TRACE(("et%d: etc_attach: vendor 0x%x device 0x%x\n", unit, vendor, device));

	/* some code depends on packed structures */
	ASSERT(sizeof (struct ether_addr) == ETHER_ADDR_LEN);
	ASSERT(sizeof (struct ether_header) == ETHER_HDR_LEN);

	/* allocate etc_info_t state structure */
	if ((etc = (etc_info_t*) MALLOC(sizeof (etc_info_t))) == NULL) {
		ET_ERROR(("et%d: etc_attach malloc failed\n", unit));
		return (NULL);
	}
	bzero((char*)etc, sizeof (etc_info_t));

	etc->et = et;
	etc->unit = unit;
	etc->vendorid = (uint16) vendor;
	etc->deviceid = (uint16) device;

        if (SYS_REVID_GET() == BOARD_ID_JAG_BCM56218_EB) {
	    etc->forcespeed = (unit == 1) ? ET_100FULL : ET_AUTO;
            etc->duplex = 1;
            etc->speed = 100;
        } else {
	    etc->forcespeed = (sysIsLM()) ? ET_100HALF : ET_AUTO;
        }
	etc->linkstate = FALSE;

	/* set chip opsvec */
	etc->chops = etc_chipmatch(vendor, device);
	ASSERT(etc->chops);

	/* chip attach */
	if ((etc->ch = (*etc->chops->attach)(etc, pdev, regsva)) == NULL) {
		ET_ERROR(("et%d: chipattach error\n", unit));
		goto fail;
	}

	return ((void*)etc);

fail:
	etc_detach(etc);
	return (NULL);
}

void
etc_detach(etc_info_t *etc)
{
	if (etc == NULL)
		return;

	/* free chip private state */
	if (etc->ch) {
		(*etc->chops->detach)(etc->ch);
		etc->chops = etc->ch = NULL;
	}

	MFREE(etc, sizeof (etc_info_t));
}

void
etc_reset(etc_info_t *etc)
{
	ET_TRACE(("et%d: etc_reset\n", etc->unit));

	etc->reset++;

	/* reset the chip */
	(*etc->chops->reset)(etc->ch);

	/* free any posted tx packets */
	(*etc->chops->txreclaim)(etc->ch, TRUE);

#ifdef DMA
	/* free any posted rx packets */
	(*etc->chops->rxreclaim)(etc->ch);
#endif
}

void
etc_init(etc_info_t *etc)
{
	ET_TRACE(("et%d: etc_init\n", etc->unit));

	ASSERT(etc->pioactive == NULL);
	ASSERT(!ETHER_ISNULLADDR(&etc->cur_etheraddr));
	ASSERT(!ETHER_ISMULTI(&etc->cur_etheraddr));

	/* init the chip */
	(*etc->chops->init)(etc->ch, TRUE);
}

/* mark interface up */
void
etc_up(etc_info_t *etc)
{
	/* calibration */
	ET_LOG("etc_up1", 0);
	ET_LOG("etc_up2", 0);
	ET_LOG("etc_up3", 0);

	etc->up = TRUE;

	et_init(etc->et);
}

/* mark interface down */
uint
etc_down(etc_info_t *etc, int reset)
{
	uint callback;

	callback = 0;

	etc->up = FALSE;
	if (reset)
		et_reset(etc->et);

	/* suppress link state changes during power management mode changes */
	if (etc->linkstate) { 
		etc->linkstate = FALSE;
		if (!etc->pm_modechange)
			et_link_down(etc->et);
	}

	return (callback);
}

/* common ioctl handler.  return: 0=ok, -1=error */
int
etc_ioctl(etc_info_t *etc, int cmd, void *arg)
{
	int error;
	int val;
	int *vec = (int*)arg;

	error = 0;

	val = arg? *(int*)arg: 0;

	ET_TRACE(("et%d: etc_ioctl: cmd 0x%x\n", etc->unit, cmd));

	switch (cmd) {
	case ETCUP:
		et_up(etc->et);
		break;

	case ETCDOWN:
		et_down(etc->et, TRUE);
		break;

	case ETCLOOP:
		etc_loopback(etc, val);
		break;

	case ETCDUMP:
#ifdef BCMDBG
		/* if trace logging is on, hijack et_dump() */
		if (et_msg_level & 2048)
			etc_dumplog((uchar*) arg);
		else
#endif
			et_dump(etc->et, (uchar*) arg, 4096);
		break;

	case ETCSETMSGLEVEL:
		et_msg_level = val;
		break;

	case ETCPROMISC:
		etc_promisc(etc, val);
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

		et_init(etc->et);
		break;

	case ETCPHYRD:
		if (vec)
			vec[1] = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, vec[0]);
		break;

	case ETCPHYWR:
		if (vec)
			(*etc->chops->phywr)(etc->ch, etc->phyaddr, vec[0], vec[1]);
		break;
		
	default:
	err:
		error = -1;
	}

	return (error);
}

/* called once per second */
void
etc_watchdog(etc_info_t *etc)
{
	uint16 status;
	uint16 adv;
	uint16 lpa;

	etc->now++;

	if (etc->phyaddr == PHY_NOMDC) {
		etc->linkstate = TRUE;
		etc->speed = 100;
		etc->duplex = 1;
		return;
	}

	status = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, 1);
	adv = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, 4);
	lpa = (*etc->chops->phyrd)(etc->ch, etc->phyaddr, 5);

	/* check for bad mdio read */
	if (status == 0xffff) {
		ET_ERROR(("et%d: etc_watchdog: bad mdio read: phyaddr %d mdcport %d\n",
			etc->unit, etc->phyaddr, etc->mdcport));
		return;
	}

	/* monitor link state */
	if (!etc->linkstate && (status & STAT_LINK)) {
		etc->linkstate = TRUE;

		if (etc->pm_modechange)
			etc->pm_modechange = FALSE;
		else
			et_link_up(etc->et);
	}
	else if (etc->linkstate && !(status & STAT_LINK)) {
		etc->linkstate = FALSE;
		if (!etc->pm_modechange)
			et_link_down(etc->et);
	}

	/* update current speed and duplex */
	if ( (adv & ADV_100FULL) && (lpa & LPA_100FULL) ) {
		etc->speed = 100;
		etc->duplex = 1;
	} else if ( (adv & ADV_100HALF) && (lpa & LPA_100HALF) ) {
		etc->speed = 100;
		etc->duplex = 0;
	} else if ( (adv & ADV_10FULL) && (lpa & LPA_10FULL) ) {
		etc->speed = 10;
		etc->duplex = 1;
	} else {
		etc->speed = 10;
		etc->duplex = 0;
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
etc_loopback(etc_info_t *etc, int on)
{
	ET_TRACE(("et%d: etc_loopback: %d\n", etc->unit, on));

	etc->loopbk = (bool) on;
	et_init(etc->et);
}

void
etc_promisc(etc_info_t *etc, uint on)
{
	ET_TRACE(("et%d: etc_promisc: %d\n", etc->unit, on));

	etc->promisc = (bool) on;
	et_init(etc->et);
}

void
etc_dump(etc_info_t *etc, uchar *buf, int size)
{
	/* big enough */
	if (size < 3700)
		return;

	etc_dumpetc(etc, buf);
	(*etc->chops->dump)(etc->ch, buf + strlen(buf));
}

static void
etc_dumpetc(etc_info_t *etc, uchar *buf)
{
	char perm[32], cur[32];
	uint i;

	buf += sprintf(buf, "etc 0x%x et 0x%x unit %d msglevel %d speed/duplex %d%s\n",
		(uint)etc, (uint)etc->et, etc->unit, et_msg_level,
		etc->speed, (etc->duplex? "full": "half"));
	buf += sprintf(buf, "up %d promisc %d loopbk %d forcespeed %d advertise 0x%x needautoneg %d\n",
		etc->up, etc->promisc, etc->loopbk, etc->forcespeed, etc->advertise, etc->needautoneg);
	buf += sprintf(buf, "piomode %d pioactive 0x%x nmulticast %d allmulti %d\n",
		etc->piomode, (uint)etc->pioactive, etc->nmulticast, etc->allmulti);
	buf += sprintf(buf, "vendor 0x%x device 0x%x rev %d coreunit %d phyaddr %d mdcport %d\n",
		etc->vendorid, etc->deviceid, etc->chiprev,
		etc->coreunit, etc->phyaddr, etc->mdcport);

	buf += sprintf(buf, "perm_etheraddr %s cur_etheraddr %s\n",
		bcm_ether_ntoa((char*)&etc->perm_etheraddr, perm),
		bcm_ether_ntoa((char*)&etc->cur_etheraddr, cur));

	if (etc->nmulticast) {
		buf += sprintf(buf, "multicast: ");
		for (i = 0; i < etc->nmulticast; i++)
			buf += sprintf(buf, "%s ", bcm_ether_ntoa((char*)&etc->multicast[i], cur));
		buf += sprintf(buf, "\n");
	}

	buf += sprintf(buf, "linkstate %d\n", etc->linkstate);
	buf += sprintf(buf, "\n");

	/* refresh stat counters */
	(*etc->chops->statsupd)(etc->ch);

	/* summary stat counter line */
	/* use sw frame and byte counters -- hw mib counters wrap too quickly to be useful */
	buf += sprintf(buf, "txframe %d txbyte %d txerror %d rxframe %d rxbyte %d rxerror %d\n",
		etc->txframe, etc->txbyte, etc->txerror,
		etc->rxframe, etc->rxbyte, etc->rxerror);

	/* transmit stat counters */
	/* hardware mib pkt and octet counters wrap too quickly to be useful */
	buf += sprintf(buf, "tx_broadcast_pkts %d tx_multicast_pkts %d tx_jabber_pkts %d tx_oversize_pkts %d\n",
		etc->mib.tx_broadcast_pkts, etc->mib.tx_multicast_pkts, etc->mib.tx_jabber_pkts,
		etc->mib.tx_oversize_pkts);
	buf += sprintf(buf, "tx_fragment_pkts %d tx_underruns %d\n",
		etc->mib.tx_fragment_pkts, etc->mib.tx_underruns);
	buf += sprintf(buf, "tx_total_cols %d tx_single_cols %d tx_multiple_cols %d tx_excessive_cols %d\n",
		etc->mib.tx_total_cols, etc->mib.tx_single_cols, etc->mib.tx_multiple_cols, etc->mib.tx_excessive_cols);
	buf += sprintf(buf, "tx_late_cols %d tx_defered %d tx_carrier_lost %d tx_pause_pkts %d\n",
		etc->mib.tx_late_cols, etc->mib.tx_defered, etc->mib.tx_carrier_lost, etc->mib.tx_pause_pkts);
	buf += sprintf(buf, "txnobuf %d reset %d dmade %d dmada %d dmape %d\n",
		etc->txnobuf, etc->reset, etc->dmade, etc->dmada, etc->dmape);

	/* receive stat counters */
	/* hardware mib pkt and octet counters wrap too quickly to be useful */
	buf += sprintf(buf, "rx_broadcast_pkts %d rx_multicast_pkts %d rx_jabber_pkts %d rx_oversize_pkts %d\n",
		etc->mib.rx_broadcast_pkts, etc->mib.rx_multicast_pkts, etc->mib.rx_jabber_pkts, etc->mib.rx_oversize_pkts);
	buf += sprintf(buf, "rx_fragment_pkts %d rx_missed_pkts %d rx_crc_align_errs %d rx_undersize %d\n",
		etc->mib.rx_fragment_pkts, etc->mib.rx_missed_pkts, etc->mib.rx_crc_align_errs, etc->mib.rx_undersize);
	buf += sprintf(buf, "rx_crc_errs %d rx_align_errs %d rx_symbol_errs %d rx_pause_pkts %d\n",
		etc->mib.rx_crc_errs, etc->mib.rx_align_errs, etc->mib.rx_symbol_errs, etc->mib.rx_pause_pkts);
	buf += sprintf(buf, "rx_nonpause_pkts %d rxnobuf %d rxdmauflo %d rxoflo %d rxbadlen %d\n",
		etc->mib.rx_nonpause_pkts, etc->rxnobuf, etc->rxdmauflo, etc->rxoflo, etc->rxbadlen);
	buf += sprintf(buf, "\n");
}

#define	LOGSIZE	256

static struct {
	uint32	cycles;
	uint32	instr;
	char	*fmt;
	ulong	a1;
} etc_logtab[LOGSIZE];

static uint etc_logi = 0;

void
etc_log(uint cycles, uint instr, char *fmt, ulong a1)
{
	uint i;

	i = etc_logi;

	/* save values */
	etc_logtab[i].cycles = cycles;
	etc_logtab[i].instr = instr;
	etc_logtab[i].fmt = fmt;
	etc_logtab[i].a1 = a1;

	etc_logi = ++i % LOGSIZE;
}

void
etc_dumplog(uchar *buf)
{
	uint i;

	*buf = '\0';

	/* format the log into the buffer in chronological order */
	for (i = (etc_logi + 1) % LOGSIZE; i != etc_logi; i = (++i % LOGSIZE)) {
		if (etc_logtab[i].fmt == NULL)
			continue;
		buf += sprintf(buf, "%d\t%d\t",
			etc_logtab[i].cycles, etc_logtab[i].instr);
		buf += sprintf(buf, etc_logtab[i].fmt, etc_logtab[i].a1);
		buf += sprintf(buf, "\n");
	}
}

uint
etc_totlen(etc_info_t *etc, void *p)
{
	uint total;

	total = 0;
	for (; p; p = PKTNEXT(etc->et, p))
		total += PKTLEN(etc->et, p);
	return (total);
}

#ifdef BCMDBG
void
etc_prhdr(char *msg, struct ether_header *eh, uint len)
{
	char da[32], sa[32];

	printf("%s: dst %s src %s type 0x%x len %d\n",
		msg,
		bcm_ether_ntoa(eh->ether_dhost, da),
		bcm_ether_ntoa(eh->ether_shost, sa),
		ntoh16(eh->ether_type),
		len);
}
#endif /* BCMDBG */
