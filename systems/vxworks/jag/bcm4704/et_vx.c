/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.

    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * vxWorks 5.x END device driver for
 * Broadcom BCM44XX/BCM47XX family 10/100 Mbps Ethernet Controller
 *
 * Copyright(c) 2000 Broadcom Corp.
 * All Rights Reserved.
 * $Id: et_vx.c,v 1.1 2004/02/24 07:47:00 csm Exp $
 */

#ifndef lint
static const char cvsid[] = "$Id: et_vx.c,v 1.1 2004/02/24 07:47:00 csm Exp $";
#endif

#define	__INCunixLibh		/* XXX temp defeat unixLib.h MALLOC() definition */

#include <vx_osl.h>
#include <endLib.h>
#include <intLib.h>
#include <ioctl.h>
#include <netLib.h>
#include <wdLib.h>

#ifndef MSI
#include <drv/pci/pciConfigLib.h>
#endif

#undef	ETHER_MAP_IP_MULTICAST
#include <etherMultiLib.h>
#include <config.h>

#include <epivers.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <ethernet.h>
#include <bcmenetmib.h>
#include <bcmenetrxh.h>
#include <etsockio.h>
#include <et_dbg.h>
#include <etc.h>

#ifdef INCLUDE_ETV0_END
/* info associated with virtual device */
extern END_OBJ *pVirtDev;
#endif /* INCLUDE_ETV0_END */

/* mblk queue */
struct mblkq {
	M_BLK_ID	head;
	M_BLK_ID	tail;
	uint		len;
};

/*
 * Principle vx-specific, per-device software state.
 *
 * The END_OBJ must be the first field in this structure since
 * we overload entry point END_OBJ* with et_info_t* .
 */
typedef struct et_info {
	END_OBJ		endobj;		/* end object */
	etc_info_t	*etc;		/* pointer to common os-independent data */
	SEM_ID		mutex;		/* private lock */
	bool		netjobpend;	/* netjob(et_intr) is pending */
	WDOG_ID		watchdogid;	/* 1 sec et_watchdog timer id */
	bool		dogtimerset;	/* if true: watchdog is running */
	void		*descmem;	/* uncached dma descriptor memory */
    pciinfo_t   pciinfo;
	int		    ivec;		/* interrupt vector */
	int		    irq;		/* interrupt vector */

	struct mblkq	txq;		/* private transmit software queue */

	long		flags;		/* our local flags. */
	M_CL_CONFIG	mclconfig;	/* packet pool */
	CL_DESC		cldesc[1];	/* cluster buffer pool descriptors */
	struct et_info	*next;		/* point to next et_info_t in chain */
} et_info_t;

#define	MAXUNITS	8
static et_info_t *et_global[MAXUNITS];
static et_info_t *et_list = NULL;

#define ONE_SECOND	sysClkRateGet()

#ifdef __i386__

/* perf logging */
#define rdtscl(low) \
	__asm__ __volatile__("rdtsc" : "=a" (low) : : "edx")
#define rdmsr(msr,val1,val2) \
	__asm__ __volatile__("rdmsr" \
		: "=a" (val1), "=d" (val2) \
		: "c" (msr))
#define wrmsr(msr,val1,val2) \
	__asm__ __volatile__("wrmsr" \
		: /* no outputs */ \
		: "c" (msr), "a" (val1), "d" (val2))

#define	INT_NUM(_a)		(_a)
#define	INT_NUM_IRQ0		0x20
#define	INT_ENABLE(_a)		sysIntEnablePIC(_a)

#elif __mips__

#include <vxbsp.h>
#define	INT_NUM(_a)		(((_a) >= IV_RTIME_VEC)? INT_LVL_IORQ0: \
					(INT_LVL_IORQ0 << ((_a) - IV_IORQ0_VEC)))
#undef	INT_NUM_IRQ0
#define	INT_NUM_IRQ0		0
#define	INT_ENABLE(_a)		intEnable(_a)

#endif	/* __mips__ */

#define CLASS_NET_ETH		0x000200    /* 24 bit class value */
#define	DATAHIWAT		    50

/* functions called by etc.c */
void et_init(et_info_t *et);
void et_reset(et_info_t *et);
void et_link_up(et_info_t *et);
void et_link_down(et_info_t *et);
void et_up(et_info_t *et);
void et_down(et_info_t *et, int reset);
int et_dump(et_info_t *et, uchar *buf, uint len);

/* local functions */
END_OBJ *et_load(char *initString, void *unused);
static int et_find_device(int unit, int *bus, int *dev, int *func,
	uint16 *vendorid, uint16 *deviceid);
static STATUS et_unload(et_info_t *et);
static STATUS et_netpool_alloc(et_info_t *et);
static void et_netpool_free(et_info_t *et);
static void et_free(et_info_t *et);
static void et_vxwatchdog(et_info_t *et);
static void et_vxintr(et_info_t *et);
static void et_sendnext(et_info_t *et);
static void et_watchdog(et_info_t *et);
static void et_sendup(et_info_t *et, struct mbuf *m);
static void et_loginit(void);
static STATUS et_end_start(et_info_t *et);
static STATUS et_end_stop(et_info_t *et);
static int et_ioctl(et_info_t *et, int cmd, caddr_t data);
static STATUS et_send(et_info_t *et, M_BLK_ID pBuf);
static STATUS et_mcastAdd(et_info_t *et, char* pAddress);
static STATUS et_mcastDel(et_info_t *et, char* pAddress);
static STATUS et_mcastGet(et_info_t *et, MULTI_TABLE* pTable);
static void et_config(et_info_t *et);
static void et_enq(struct mblkq *q, M_BLK_ID m);
static M_BLK_ID et_deq(struct mblkq *q);
#ifdef BCMDBG
static void et_dumpet(et_info_t *et, uchar *buf, uint len);
#endif
#ifdef ROBO 
static M_BLK_ID
et_endEtherAddressForm(M_BLK_ID p, M_BLK_ID s, M_BLK_ID d, BOOL bcastFlag);
#else
#define et_endEtherAddressForm endEtherAddressForm
#endif

static NET_FUNCS etFuncTable = {
	(FUNCPTR) et_end_start,		/* function to start the device. */
	(FUNCPTR) et_end_stop,		/* function to stop the device. */
	(FUNCPTR) et_unload,		/* unloading function for the driver. */
	(FUNCPTR) et_ioctl,		/* ioctl function for the driver. */
	(FUNCPTR) et_send,		/* send function for the driver. */
	(FUNCPTR) et_mcastAdd,		/* multicast add function for the driver. */
	(FUNCPTR) et_mcastDel,		/* multicast delete function for the driver. */
	(FUNCPTR) et_mcastGet,		/* multicast retrieve function for the driver. */
	(FUNCPTR) NULL,			/* polling send function -  not supported*/
	(FUNCPTR) NULL,			/* polling receive function  -  not supported */
	et_endEtherAddressForm,		/* put address info into a NET_BUFFER */
	endEtherPacketDataGet,		/* get pointer to data in NET_BUFFER */
	endEtherPacketAddrGet		/* get packet addresses. */
};

#define ET_LOCK(et)		    semTake((et)->mutex, WAIT_FOREVER)
#define ET_UNLOCK(et)		semGive((et)->mutex)

/*
 * vxWorks 5.x does not directly support shared interrupts.  Unbelievable.
 * Redefine intConnect().
 */
#ifndef __mips__
#if defined(INCLUDE_IL_END) || defined(INCLUDE_IL)
extern STATUS cmn_int_connect(VOIDFUNCPTR *ivec, VOIDFUNCPTR dev_vxintr, int param);
#define intConnect  cmn_int_connect
#endif
#endif

END_OBJ*
et_load(char *initString, void *unused)
{
	et_info_t *et;
	char *tok;
	int unit;
	uint16 vendorid, deviceid;
	uint32 regaddr;
	char irq;
	int bus, dev, func;

	et = NULL;
	bus = dev = func = 0;
	regaddr = 0;
	irq = 0;

	if (initString == NULL)
		return (NULL);

	/* end driver first pass init */
	if (initString[0] == '\0') {
		bcopy("et", initString, 3);
		return (NULL);
	}

	/* get our unit number from the init string */
	if ((tok = strtok(initString, ":")) == NULL) {
		ET_ERROR(("et_load: garbled initString (%s) - unit number not found\n",
			initString));
		goto fail;
	}
	unit = bcm_atoi(tok);

	ET_TRACE(("et%d: et_load\n", unit));


#ifndef MSI
	if (et_find_device(unit, &bus, &dev, &func, &vendorid, &deviceid) == ERROR) {
		ET_ERROR(("et%d: et_load: et_find_device() failed \n", unit));
		goto fail;
	}

	pciConfigInLong(bus, dev, func, PCI_CFG_BASE_ADDRESS_0, &regaddr);
	pciConfigInByte(bus, dev, func, PCI_CFG_DEV_INT_LINE, &irq);

	/* map chip registers */
	regaddr = REG_MAP(regaddr, 0);
#endif

	/* allocate our private device structure */
	if ((et = (et_info_t*) MALLOC(sizeof (et_info_t))) == NULL) {
		ET_ERROR(("et%d: et_load: malloc et_info_t failed!\n", unit));
		goto fail;
	}
	bzero((char*) et, sizeof (et_info_t));

	et->irq = INT_NUM(irq);
	et->ivec = (int) INUM_TO_IVEC(irq + INT_NUM_IRQ0);
	et->pciinfo.bus = bus;
	et->pciinfo.dev = dev;
	et->pciinfo.func = func;

	ET_TRACE(("et%d: et_load: regaddr %x ivec %d irq %d\n",
		unit, regaddr, et->ivec, et->irq));

	/* initialize private semaphore */
	if ((et->mutex = semBCreate(SEM_Q_FIFO, SEM_FULL)) == NULL) {
		ET_ERROR(("et%d: et_load: semBCreate fail\n", unit));
		goto fail;
	}

	/* common load-time initialization */
	if ((et->etc = etc_attach((void*)et, vendorid, deviceid, unit, &et->pciinfo,
		(void*)regaddr)) == NULL) {
		ET_ERROR(("et%d: etc_attach failed\n", unit));
		goto fail;
	}

	/* create a watchdog timer */
	if ((et->watchdogid = wdCreate()) == NULL ) {
		ET_ERROR(("et%d: error creating watchdog timer\n", unit));
		goto fail;
	}
	et->dogtimerset = FALSE;

	/* initialize the END and MIB2 parts of the structure */
	if (END_OBJ_INIT(&et->endobj, (DEV_OBJ*)et, "et", unit, &etFuncTable, "END et Driver")
		== ERROR)
		goto fail;

	if (END_MIB_INIT(&et->endobj, M2_ifType_ethernet_csmacd, (char *)&et->etc->cur_etheraddr,
		ETHER_ADDR_LEN, ETHERMTU, 100000000) == ERROR)
		goto fail;

	/* allocate and initialize our private mblk/clblk netpool */
	if (et_netpool_alloc(et) == ERROR)
		goto fail;

	/* set the flags to indicate readiness */
	END_OBJ_READY(&et->endobj, (IFF_NOTRAILERS|IFF_BROADCAST|IFF_MULTICAST));

	/* register our interrupt handler */
	if (intConnect((VOIDFUNCPTR*)et->ivec, (VOIDFUNCPTR)et_vxintr, (int)et) == ERROR) {
		ET_ERROR(("et%d: et_load: interrupt connect error\n", unit));
		goto fail;
	}

	et_global[unit] = et;

	/* add us to the global linked list */
	et->next = et_list;
	et_list = et;

	return (&et->endobj);

fail:
	et_free(et);
	return (NULL);
}

/*
 * et_unload - unload a driver from the system
 */
static STATUS
et_unload(et_info_t *et)
{
	ET_TRACE(("et%d: et_unload\n", et->etc->unit));

	ASSERT(!et->etc->up);

	END_OBJECT_UNLOAD(&et->endobj);

	et_global[et->etc->unit] = NULL;

	et_free(et);

	return (OK);
}

static int
et_find_device(int unit, int *bus, int *dev, int *func, uint16 *vendorid, uint16 *deviceid)
{
	int found;
	int i;

	found = 0;

	/* find the unit'th instance of a recognized et device */
	for (i = 0; i < 32; i++) {
		if (pciFindClass(CLASS_NET_ETH, i, bus, dev, func) != OK)
			continue;
		if (pciConfigInWord(*bus, *dev, *func, 0, vendorid) != OK)
			continue;
		if (pciConfigInWord(*bus, *dev, *func, 2, deviceid) != OK)
			continue;
		if (!etc_chipmatch(*vendorid, *deviceid))
			continue;

		if (found == unit)
			return (OK);

		found++;
	}

	ET_ERROR(("et%d: et_find_device:  error finding device\n", unit));

	return (ERROR);
}

static void
et_free(et_info_t *et)
{
	et_info_t **prev;

	if (et == NULL)
		return;

	ET_TRACE(("et: et_free\n"));

	/* delete the watchdog timer */
	if (et->watchdogid) {
		wdDelete(et->watchdogid);
		et->watchdogid = NULL;
	}

	/* free common resources */
	if (et->etc) {
		etc_detach(et->etc);
		et->etc = NULL;
	}

	/* free private netpool */
	et_netpool_free(et);

	/* remove us from the global linked list */
	for (prev = &et_list; *prev; prev = &(*prev)->next)
		if (*prev == et) {
			*prev = et->next;
			break;
		}

	free((char*)et);
}

/* alloc and init a private mblk/clblk netpool */
STATUS
et_netpool_alloc(et_info_t *et)
{
	ET_TRACE(("et%d: et_netpool_alloc\n", et->etc->unit));

	/* alloc the netpool structure itself */
	if ((et->endobj.pNetPool = (NET_POOL_ID) MALLOC(sizeof (NET_POOL))) == NULL)
		goto error;

	et->mclconfig.mBlkNum = MIN_PKTS;
	et->mclconfig.clBlkNum = MIN_PKTS;

	/* sanity check */
	ASSERT(BUFSZ == VXCLSIZE);

	/* create a single 2Kbyte cluster buffer pool */
	et->cldesc[0].clNum = MIN_CLUSTERS;
	et->cldesc[0].clSize = VXCLSIZE;

	/* calculate the total memory for all the M_BLKs and CL_BLKs */
	et->mclconfig.memSize = (et->mclconfig.mBlkNum * (MSIZE + sizeof (long))) +
		(et->mclconfig.clBlkNum * (CL_BLK_SZ + sizeof(long)));

	/* allocate the M_BLK&CL_BLK pool */
	if ((et->mclconfig.memArea = MALLOC(et->mclconfig.memSize)) == NULL)
		goto error;

	/* calculate the memory size of all the clusters. */
	et->cldesc[0].memSize = (et->cldesc[0].clNum * (VXCLSIZE + 8)) + sizeof (int);

	/* allocate the cluster buffer pool */
	if ((et->cldesc[0].memArea = memalign(2048, et->cldesc[0].memSize)) == NULL)
		goto error;
	ASSERT(ISALIGNED(et->cldesc[0].memArea, 2048));

	/* now init the netpool */
	if (netPoolInit(et->endobj.pNetPool, &et->mclconfig, &et->cldesc[0], 1, NULL) == ERROR)
		goto error;

	return (OK);

error:
	ET_ERROR(("et%d: et_netpool_alloc: malloc failed!\n", et->etc->unit));
	et_netpool_free(et);
	return (ERROR);
}

static void
et_netpool_free(et_info_t *et)
{
	ET_TRACE(("et%d: et_netpool_free\n", et->etc->unit));

	if (et->cldesc[0].memArea) {
		MFREE(et->cldesc[0].memArea, et->cldesc[0].memSize);
		et->cldesc[0].memSize = 0;
		et->cldesc[0].memArea = NULL;
	}

	if (et->mclconfig.memArea) {
		MFREE(et->mclconfig.memArea, et->mclconfig.memSize);
		et->mclconfig.memSize = 0;
		et->mclconfig.memArea = NULL;
	}

	if (et->endobj.pNetPool) {
		netPoolDelete(et->endobj.pNetPool);
		et->endobj.pNetPool = NULL;
	}
}

void
et_init(et_info_t *et)
{
	ET_TRACE(("et%d: et_init\n", et->etc->unit));
	ET_LOG("et_init", 0);

	et_loginit();

	/* reset the interface */
	et_reset(et);

	/* initialize the driver and chip */
	etc_init(et->etc);

	et->endobj.flags |= (IFF_RUNNING | IFF_UP);
}

void
et_reset(et_info_t *et)
{
	ET_TRACE(("et%d: et_reset\n", et->etc->unit));

	/* common reset code */
	etc_reset(et->etc);

	et->endobj.flags &= ~IFF_RUNNING;
}

void
et_link_up(et_info_t *et)
{
	et->endobj.mib2Tbl.ifOperStatus = 1;
	ET_ERROR(("et%d: link up\n", et->etc->unit));
}

void
et_link_down(et_info_t *et)
{
	et->endobj.mib2Tbl.ifOperStatus = 0;
	ET_ERROR(("et%d: link down\n", et->etc->unit));
}

void
et_up(et_info_t *et)
{
	ET_TRACE(("et%d: et_up\n", et->etc->unit));

	etc_up(et->etc);

	END_FLAGS_SET (&et->endobj, IFF_UP | IFF_RUNNING);

	/* start a one second the watchdog timer */
	if (et->dogtimerset == FALSE) {
		if (wdStart(et->watchdogid, ONE_SECOND, (FUNCPTR)et_vxwatchdog, (int)et) != OK) {
			ET_ERROR(("et%d: et_up: Error starting the watchdog\n", et->etc->unit));
		}
		et->dogtimerset = TRUE;
	}

}

void
et_down(et_info_t *et, int reset)
{
	etc_info_t *etc;
	M_BLK_ID m;

	etc = et->etc;

	ET_TRACE(("et%d: et_down\n", etc->unit));

	/* cancel the watchdog timer */
	if (et->dogtimerset) {
		wdCancel(et->watchdogid);
		et->dogtimerset = FALSE;
	}

	etc_down(etc, reset);

    /* mark the driver as down */
    END_FLAGS_CLR (&et->endobj, IFF_UP | IFF_RUNNING);

	/* flush the txq */
	while ((m = (M_BLK_ID) et_deq(&et->txq)))
		netMblkClChainFree(m);
}

int
et_dump(et_info_t *et, uchar *buf, uint len)
{
	/* big enough? */
	if (len < 4096)
		return (4096);

	sprintf(buf, "et%d: %s %s version %s\n", et->etc->unit,
		__DATE__, __TIME__, EPI_VERSION_STR);

#ifdef BCMDBG
	et_dumpet(et, &buf[strlen(buf)], len - strlen(buf));
	etc_dump(et->etc, &buf[strlen(buf)], len - strlen(buf));
#endif

	return (strlen(buf));
}

static void
et_vxwatchdog(et_info_t *et)
{
	netJobAdd((FUNCPTR)et_watchdog, (int)et, 0, 0, 0, 0);
}

static void
et_watchdog(et_info_t *et)
{
#if 0
	ET_TRACE(("et%d: watchdog\n", et->etc->unit));
	ET_LOG("et_watchdog", 0);
#endif

	ET_LOCK(et);

	etc_watchdog(et->etc);

	/* restart timer */
	wdStart(et->watchdogid, ONE_SECOND, (FUNCPTR)et_vxwatchdog, (int)et);

	ET_UNLOCK(et);

	ET_LOG("et_watchdog ret\n", 0);
}

static void
et_sendnext(et_info_t *et)
{
	M_BLK_ID m;
	etc_info_t *etc;

	etc = et->etc;

	ET_TRACE(("et%d: et_sendnext\n", etc->unit));
	ET_LOG("et_sendnext", 0);

	/* dequeue and send each packet */
	while ((*etc->txavail > 8)
		&& (etc->pioactive == NULL)
		&& (m = et_deq(&et->txq))) {

		ET_PRHDR("tx", (struct ether_header*) m->mBlkHdr.mData, etc_totlen(etc, (void*)m));
		ET_PRPKT("txpkt", m->mBlkHdr.mData, m->mBlkHdr.mLen);

		/* transmit the frame */
		(*etc->chops->tx)(etc->ch, (void*)m);

		etc->txframe++;
		etc->txbyte += m->mBlkHdr.mLen;
	}

	ET_LOG("et_sendnext ret\n", 0);
}

static void
et_intr(et_info_t *et, uint events)
{
	etc_info_t *etc;
	struct chops *chops;
	void *ch;
	M_BLK_ID m;

	etc = et->etc;
	chops = etc->chops;
	ch = etc->ch;

	ASSERT(events);

	ET_TRACE(("et%d: et_intr: events 0x%x\n", etc->unit, events));
	ET_LOG("et_intr: events 0x%x", events);

	ET_LOCK(et);

	if (events & INTR_RX) {
		while ((m = (M_BLK_ID) (*chops->rx)(ch)))
			et_sendup(et, m);
		/* post more rx bufs */
		(*chops->rxfill)(ch);
	}

	if (events & INTR_TX)
		(*chops->txreclaim)(ch, FALSE);

	if (events & INTR_ERROR)
		if ((*chops->errors)(ch))
			et_init(et);

	/* run the tx queue */
	if (et->txq.len)
		et_sendnext(et);

	et->netjobpend = FALSE;

	/* re-enable interrupts */
	(*chops->intrson)(ch);

	ET_UNLOCK(et);
	ET_LOG("et_intr ret\n", 0);
}

/* just a wrapper for et_intr() */
static void
et_vxintr(et_info_t *et)
{
	uint events;

	/* guard against shared interrupts */
	if (!et->etc->up)
		goto done;

	events = (*et->etc->chops->getintrevents)(et->etc->ch);

	ET_LOG("et_vxintr: events 0x%x", events);

	if (!(events & INTR_NEW) || et->netjobpend)
		goto done;

	/* disable device interrupts */
	(*et->etc->chops->intrsoff)(et->etc->ch);

	et->netjobpend = TRUE;
	netJobAdd((FUNCPTR)et_intr, (int)et, events, 0, 0 ,0);

done:
	ET_LOG("et_vxintr ret\n", 0);
}

static void
et_sendup(et_info_t *et, M_BLK_ID m)
{
	bcmenetrxh_t *rxh;
	uint16 flags;
	uchar eabuf[32];
#ifdef INCLUDE_ETV0_END
    struct vlan_ethhdr *vhdr;
#endif /* INCLUDE_ETV0_END */

#if 0
sysSerialPrintString("et_sendup\n");
ET_PRPKT("rxpktraw", (uchar*)m->mBlkHdr.mData, m->mBlkHdr.mLen);
#endif
	/* packet buffer starts with rxh */
	rxh = (bcmenetrxh_t*) m->mBlkHdr.mData;

	/* strip off rxh */
	m->mBlkHdr.mData += HWRXOFF;
	m->mBlkHdr.mLen -= HWRXOFF;

	ET_TRACE(("et%d: et_sendup: %d bytes\n", et->etc->unit, m->mBlkHdr.mLen));
	ET_LOG("et_sendup: len %d", m->mBlkHdr.mLen);

	et->etc->rxframe++;
	et->etc->rxbyte += m->m_len;

	/* eh should now be aligned 2-mod-4 */
	ASSERT(((uint)m->mBlkHdr.mData & 3) == 2);

	/* strip off crc32 */
	m->mBlkHdr.mLen -= ETHER_CRC_LEN;

	ET_PRHDR("rx", (struct ether_header*)m->mBlkHdr.mData, m->mBlkHdr.mLen);
	ET_PRPKT("rxpkt", (uchar*)m->mBlkHdr.mData, m->mBlkHdr.mLen);

	/* check for reported frame errors */
	flags = ltoh16(rxh->flags);
	if (flags & (RXF_NO | RXF_RXER | RXF_CRC | RXF_OV))
		goto err;

	m->mBlkHdr.mFlags |= M_PKTHDR;
	m->mBlkPktHdr.len = m->mBlkHdr.mLen;

	/* The vx router 1.0 stack is reentrant so we 
		need to drop the perimeter lock here */
	ET_UNLOCK(et);
#ifdef INCLUDE_ETV0_END
    /* check to see if vlan tag.  if so this is a LAN packet, remove tag */
    /* and use virt driver as parameter of rcv routine.  if no vlan tag */
    /* this is a WAN packet, just send up */
    vhdr = (struct vlan_ethhdr *)m->mBlkHdr.mData;
    if (vhdr->h_vlan_proto == ntoh16(VLAN_TAG))
    {
        memmove(m->mBlkHdr.mData + VLAN_FIELDS_SIZE,m->mBlkHdr.mData,
                2*ETHER_ADDR_LEN);
    	m->mBlkHdr.mData += VLAN_FIELDS_SIZE;
    	m->mBlkHdr.mLen -= VLAN_FIELDS_SIZE;
	    m->mBlkPktHdr.len = m->mBlkHdr.mLen;
    	if (pVirtDev == NULL)
    	    goto err;
    	else
    	{
	        END_ERR_ADD(pVirtDev, MIB2_IN_UCAST, +1);
	        END_RCV_RTN_CALL(pVirtDev, m);   
	    }
    } else {
        END_RCV_RTN_CALL(&et->endobj, m);
	}
#else /* INCLUDE_ETV0_END */
        END_RCV_RTN_CALL(&et->endobj, m);
#endif /* INCLUDE_ETV0_END */
	END_ERR_ADD(&et->endobj, MIB2_IN_UCAST, +1);
	ET_LOCK(et);

	ET_LOG("et_sendup ret", 0);

	return;

err:
	bcm_ether_ntoa(((struct ether_header*)m->mBlkHdr.mData)->ether_shost, eabuf);
	if (flags & RXF_NO) {
		ET_ERROR(("et%d: rx: crc error (odd nibbles) from %s\n", et->etc->unit, eabuf));
	}
	if (flags & RXF_RXER) {
		ET_ERROR(("et%d: rx: symbol error from %s\n", et->etc->unit, eabuf));
	}
	if ((flags & RXF_CRC) == RXF_CRC) {
		ET_ERROR(("et%d: rx: crc error from %s\n", et->etc->unit, eabuf));
	}
	if (flags & RXF_OV) {
		ET_ERROR(("et%d: rx: fifo overflow\n", et->etc->unit));
	}
	netMblkClFree(m);
	return;
}

/*
 * Debugging/Performance Logging Functions
 */
static void
et_loginit(void)
{
#if defined(BCMDBG) && defined(__i386__)
	if (et_msg_level & 2048)
		wrmsr(0x186, 0x4700c0, 0);	/* enable P6 instr counting */
#ifdef notdef
		wrmsr(0x11, 0xd6, 0);		/* enable Pentium instr counting */
#endif
#endif /* BCMDBG && __i386__ */
}

#ifdef BCMDBG
static void
et_dumpet(et_info_t *et, uchar *buf, uint len)
{
	sprintf(&buf[strlen(buf)], "et %p flags 0x%x txq.len %d\n",
		et, (uint)et->flags, et->txq.len);
	sprintf(&buf[strlen(buf)], "\n");
}

/* obtaining cycle and instruction counts is processor specific */
void
et_log(char *fmt, ulong a1)
{
	static uint32 lasttsc = 0;
	static uint32 lastinstr = 0;
	uint32 tsc, instr;

#ifdef __i386__
	uint32 dummy;

	/* read cycle counter */
	rdtscl(tsc);

	/* read ctr1 value */
	rdmsr(0xc1, instr, dummy);	/* read P6 CTR0 register */
#ifdef notdef
	rdmsr(0x12, instr, dummy);	/* read Pentium CTR0 register */
#endif
#endif	/* __i386__ */

#if defined(__mips__)
	/* readCount() and readInstr() must be provided by the bsp */
	tsc = readCount();
	instr = readInstr();
#endif

	/* call common log handler */
	etc_log(tsc - lasttsc, instr - lastinstr, fmt, a1);

	/* update last values */
	lasttsc = tsc;
	lastinstr = instr;
}
#endif	/* BCMDBG */

/*
 * et_end_start - start the device
 */
static STATUS
et_end_start(et_info_t *et)
{
	ET_TRACE(("et%d: et_end_start\n", et->etc->unit));
	ET_LOCK(et);
	et_up(et);

#ifndef MSI
	INT_ENABLE(et->irq);
#endif

	ET_UNLOCK(et);
	return (OK);
}

/*
 * et_end_stop - stop the device
 */
static STATUS
et_end_stop(et_info_t *et)
{
	ET_TRACE(("et%d: et_end_stop\n", et->etc->unit));
	ET_LOCK(et);
	et_down(et, 1);
	ET_UNLOCK(et);
	return (OK);
}

/*
 * et_send - the driver send routine
 */
static STATUS
et_send(et_info_t *et, M_BLK_ID m)
{
	ET_TRACE(("et%d: et_send: len %d \n", et->etc->unit, m->mBlkPktHdr.len));

	END_TX_SEM_TAKE(&et->endobj, WAIT_FOREVER);
	ET_LOCK(et);

	/* discard if transmit queue is too full */
	if (et->txq.len > DATAHIWAT)
		goto qfull;

	et_enq(&et->txq, m);
	et_sendnext(et);

	ET_UNLOCK(et);
	END_TX_SEM_GIVE(&et->endobj);

	/* bump the statistic counter. */
	END_ERR_ADD(&et->endobj, MIB2_OUT_UCAST, +1);

	return (OK);

qfull:
	ET_ERROR(("et%d: et_send: txq full\n", et->etc->unit));
	et->etc->txnobuf++;
	et->etc->txerror++;
	ET_UNLOCK(et);
	END_TX_SEM_GIVE(&et->endobj);
	return (END_ERR_BLOCK);
}

static int
et_ioctl(et_info_t *et, int cmd, caddr_t data)
{
	int error;
	long value;
	etc_info_t  *etc;

	etc = et->etc;
	error = 0;

	ET_TRACE(("et%d: et_ioctl\n", et->etc->unit));

	switch (cmd) {
	case EIOCSADDR:
		if (data == NULL)
			return (EINVAL);
		bcopy((char*)data, (char*)et->endobj.mib2Tbl.ifPhysAddress.phyAddress, ETHER_ADDR_LEN);
		bcopy((char*)data, (char*)&etc->cur_etheraddr, ETHER_ADDR_LEN);
		et_config(et);
		break;

	case EIOCGADDR:
		if (data == NULL)
			return (EINVAL);
		bcopy((char*)&etc->cur_etheraddr, (char *)data, ETHER_ADDR_LEN);
		break;

	case EIOCSFLAGS:
		value = (long)data;
		if (value < 0) {
			value = ~value;
			END_FLAGS_CLR(&et->endobj, value);
		}
		else {
			END_FLAGS_SET(&et->endobj, value);
		}
		et_config(et);
		break;

	case EIOCGFLAGS:
		*(int*)data = END_FLAGS_GET(&et->endobj);
		break;

	case EIOCPOLLSTART:
		ASSERT(0);	/* not supported */
		break;

	case EIOCPOLLSTOP:
		ASSERT(0);	/* not supported */
		break;

	case EIOCGMIB2:		/* return MIB information */
		if (data == NULL)
			return (EINVAL);

	/* update some of the mib counters */
	et->endobj.mib2Tbl.ifInErrors = etc->rxerror;
	et->endobj.mib2Tbl.ifOutErrors = etc->txerror;
	et->endobj.mib2Tbl.ifInOctets = etc->rxbyte;
	et->endobj.mib2Tbl.ifOutOctets = etc->txbyte;
	et->endobj.mib2Tbl.ifInUcastPkts = etc->rxframe;
	et->endobj.mib2Tbl.ifOutUcastPkts = etc->txframe;
		bcopy((char *)&et->endobj.mib2Tbl, (char *)data,
			sizeof(et->endobj.mib2Tbl));
		break;

	case EIOCGFBUF:		/* return minimum First Buffer for chaining */
		if (data == NULL)
			return (EINVAL);
		*(int *)data = ETHER_MAX_LEN;
		break;
#ifdef ROBO
	case EIOCGNPT:
        if (et->etc->unit == 1) {
            return(OK);
        } else {
            return(EINVAL);
        }
#endif
	default:
		error = EINVAL;
	}

	return (error);
}

/*
 * et_config - Reconfigure the interface to match the current flag/multicast settings
 */
static void
et_config(et_info_t *et)
{
	int i;
	ETHER_MULTI *pCurr;
	uchar *src, *dst;

	ET_TRACE(("et%d: et_config\n", et->etc->unit));

	ET_LOCK(et);

	/* set promiscuous mode if it's asked for. */
	et->etc->promisc = (et->endobj.flags & IFF_PROMISC)? TRUE: FALSE;
	et->etc->allmulti = (et->endobj.flags & IFF_ALLMULTI)? TRUE: FALSE;

	/* set up address filter for multicasting */
	pCurr = END_MULTI_LST_FIRST(&et->endobj);
	i = 0;
	while (pCurr != NULL) {
		src = (unsigned char *)&pCurr->addr;
		dst = (unsigned char *)&et->etc->multicast[i];
		bcopy(src, dst, ETHER_ADDR_LEN);

		pCurr = END_MULTI_LST_NEXT(pCurr);
		i++;
	}

	et->etc->nmulticast = i;
	if (i) {
		if (et->etc->nmulticast == et->endobj.nMulti) {
			ET_ERROR(("et%d: et_config: etc.nmulticast %d, endobj.nMulti %d\n",
				et->etc->unit, et->etc->nmulticast, et->endobj.nMulti));
		}
	}

	if (et->endobj.flags & IFF_UP)
		et_up(et);
	else if (et->endobj.flags & IFF_RUNNING)
		et_down(et, 1);

	ET_UNLOCK(et);
}

/*
 * et_mcastAdd - add a multicast address for the device
 */
static STATUS
et_mcastAdd(et_info_t *et, char* pAddress)
{
	int error;

	ET_TRACE(("et%d: et_mcastAdd\n", et->etc->unit));

	if ((error = etherMultiAdd(&et->endobj.multiList, pAddress)) == ENETRESET)
		et_config(et);

	return(OK);
}

/*
 * et_mcastDel - delete a multicast address for the device
 */
static STATUS
et_mcastDel(et_info_t *et,char* pAddress)
{
	int error;

	ET_TRACE(("et%d: et_mcastDel\n", et->etc->unit));

	if ((error = etherMultiDel(&et->endobj.multiList, (char *)pAddress)) == ENETRESET)
		et_config(et);

	return (OK);
}

/*
 * et_mcastGet - get the multicast address list for the device
 */
static STATUS
et_mcastGet(et_info_t *et, MULTI_TABLE* pTable)
{
	ET_TRACE(("et%d: et_mcastGet\n", et->etc->unit));

	return (etherMultiGet(&et->endobj.multiList, pTable));
}

#ifdef ROBO 
/*
 * et_endEtherAddressForm - src/dst mac updation routine
 */
M_BLK_ID
et_endEtherAddressForm(M_BLK_ID p, M_BLK_ID s, M_BLK_ID d, BOOL bcastFlag)
{
    char zd[] = {0,0,0,0,0,0};
    if (memcmp(d->mBlkHdr.mData,zd,6) == 0) {
        return(p);
    }
    return (endEtherAddressForm(p,s,d,bcastFlag));
}
#endif

/* enqueue mblk on queue */
static void
et_enq(struct mblkq *q, M_BLK_ID m)
{
	ASSERT(m->mBlkHdr.mNextPkt == NULL);

	if (q->tail == NULL) {
		ASSERT(q->head == NULL);
		q->head = q->tail = m;
	}
	else {
		ASSERT(q->head);
		ASSERT(q->tail->mBlkHdr.mNextPkt == NULL);
		q->tail->mBlkHdr.mNextPkt = m;
		q->tail = m;
	}
	q->len++;
}

/* dequeue mblk from queue */
static M_BLK_ID
et_deq(struct mblkq *q)
{
	M_BLK_ID m;

	if ((m = q->head)) {
		ASSERT(q->tail);
		q->head = m->mBlkHdr.mNextPkt;
		m->mBlkHdr.mNextPkt = NULL;
		q->len--;
		if (q->head == NULL)
			q->tail = NULL;
	}
	else {
		ASSERT(q->tail == NULL);
	}

	return (m);
}

#ifdef BCM47XX_CHOPS
/*
 * 47XX-specific shared mdc/mdio contortion:
 * Find the et associated with the same chip as <et>
 * and coreunit matching <coreunit>.
 */
void*
et_phyfind(et_info_t *et, uint coreunit)
{
	et_info_t *tmp;

	/* walk the list et's */
	for (tmp = et_list; tmp; tmp = tmp->next) {
		if (et->etc == NULL)
			continue;
		if (tmp->pciinfo.bus != et->pciinfo.bus)
			continue;  
		if (tmp->etc->coreunit != coreunit)
			continue;
		break;
	}
	return (tmp);
}

/* shared phy read entry point */
uint16
et_phyrd(et_info_t *et, uint phyaddr, uint reg)
{
	uint16 val;

	ET_LOCK(et);
	val = et->etc->chops->phyrd(et->etc->ch, phyaddr, reg);
	ET_UNLOCK(et);

	return (val);
}

/* shared phy write entry point */
void
et_phywr(et_info_t *et, uint phyaddr, uint reg, uint16 val)
{
	ET_LOCK(et);
	et->etc->chops->phywr(et->etc->ch, phyaddr, reg, val);
	ET_UNLOCK(et);
}
#endif	/* BCM47XX_CHOPS */

#ifdef BCMINTERNAL
#ifdef BCMDBG
/* debugging routines callable from the shell */
static uchar buf1[64 * 1024];

void
set_et_msg_level(int level) {
	et_msg_level = level;
}

void
dump_etstats(int unit) {
	et_dump(et_global[unit], buf1, 4096);
	printf("%s", buf1);
}

void dump_etlog(int unit) {
	etc_dumplog(buf1);
	printf("%s", buf1);
}

void up_et(int unit) {
	et_up(et_global[unit]);
}

#endif
#endif
