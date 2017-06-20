/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.

    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * vxWorks 5.x END virtual device driver for
 * Broadcom BCM44XX/BCM47XX family 10/100 Mbps Ethernet Controller
 *
 * Copyright(c) 2000 Broadcom Corp.
 * All Rights Reserved.
 * $Id: et_virt.c,v 1.1 2004/02/24 07:47:00 csm Exp $
 */

#ifndef lint
static const char cvsid[] = "$Id: et_virt.c,v 1.1 2004/02/24 07:47:00 csm Exp $";
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
#include "swapi.h"
#ifdef BROADCOM_BSP
IMPORT STATUS (* _func_etEnetAddrGet)
    (char *dev,int unit,unsigned char *pMac);
#else
#define NVRAM_GET(name, vars) nvram_get(name)
#endif


/* info associated with real device */
#define REAL_DEV "et"
END_OBJ *pRealDev = NULL;
END_OBJ *pVirtDev = NULL;

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
void et_down(et_info_t *et, int reset);

/* local functions */
END_OBJ *etv_load(char *initString, void *unused);
static STATUS et_unload(et_info_t *et);
static void et_free(et_info_t *et);
static int et_ioctl(et_info_t *et, int cmd, caddr_t data);
static STATUS et_end_start(et_info_t *et);
static STATUS et_end_stop(et_info_t *et);
static STATUS et_send(et_info_t *et, M_BLK_ID pBuf);
static STATUS et_mcastAdd(et_info_t *et, char* pAddress);
static STATUS et_mcastDel(et_info_t *et, char* pAddress);
static STATUS et_mcastGet(et_info_t *et, MULTI_TABLE* pTable);
static void et_config(et_info_t *et);
static M_BLK_ID
virt_endEtherAddressForm(M_BLK_ID p, M_BLK_ID s, M_BLK_ID d, BOOL bcastFlag);

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
	virt_endEtherAddressForm,		/* put address info into a NET_BUFFER */
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

M_BLK_ID
virt_endEtherAddressForm(M_BLK_ID pMblk, M_BLK_ID pSrc, M_BLK_ID pDst ,
		BOOL bcastFlag)
{
    uint16 encProto;
    struct vlan_hdr *vhdr;

    /* add ethernet header followed by vlan ID */

    /* add LAN vlan id */
    M_PREPEND(pMblk,VLAN_FIELDS_SIZE,M_WAIT);
    encProto = pDst->mBlkHdr.reserved;
    vhdr = (struct vlan_hdr *)pMblk->mBlkHdr.mData;
    vhdr->h_vlan_encapsulated_proto = ntoh16(encProto);
    vhdr->h_vlan_TCI = ntoh16(DEFAULT_LAN_VLAN);

    /* add ethernet header */
    pDst->mBlkHdr.reserved = ntoh16(VLAN_TAG);
    return(endEtherAddressForm(pMblk, pSrc, pDst, bcastFlag));
}

END_OBJ*
etv_load(char *initString, void *unused)
{
	et_info_t *et;
	char *tok;
	int unit;
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
		bcopy("etv", initString, 4);
		return (NULL);
	}

	/* get our unit number from the init string */
	if ((tok = strtok(initString, ":")) == NULL) {
		ET_ERROR(("etv_load: garbled initString (%s) - unit number not found\n",
			initString));
		goto fail;
	}
	unit = bcm_atoi(tok);

	ET_TRACE(("etv%d: etv_load\n", unit));

	/* make sure real device that this virtual device is */
	/* associated with exists.  if not, fail */
	if ((pRealDev = endFindByName(REAL_DEV,0)) == NULL) {
		ET_ERROR(("etv%d: etv_load: unable to find device %s%d!\n",
				unit,REAL_DEV,0));
		goto fail;
	}

	/* allocate our private device structure */
	if ((et = (et_info_t*) MALLOC(sizeof (et_info_t))) == NULL) {
		ET_ERROR(("etv%d: etv_load: malloc et_info_t failed!\n", unit));
		goto fail;
	}
	bzero((char*) et, sizeof (et_info_t));

	/* initialize private semaphore */
	if ((et->mutex = semBCreate(SEM_Q_FIFO, SEM_FULL)) == NULL) {
		ET_ERROR(("etv%d: etv_load: semBCreate fail\n", unit));
		goto fail;
	}

	/* allocate etc_info_t state structure */
	if ((et->etc = (etc_info_t*) MALLOC(sizeof (etc_info_t))) == NULL) {
		ET_ERROR(("et%d: etv_load: etc malloc failed\n", unit));
		return (NULL);
	}
	bzero((char*)et->etc, sizeof (etc_info_t));

	et->etc->et = et;
	et->etc->unit = unit;
	et->dogtimerset = FALSE;

	/* get MAC address */
    if (_func_etEnetAddrGet == NULL || _func_etEnetAddrGet ("etv", et->etc->unit,
            (unsigned char *) &et->etc->cur_etheraddr) != OK)
    {
        ET_ERROR(("etv%d: etv_load: MAC address not found\n", et->etc->unit));
        goto fail;
    }
    
	/* initialize the END part of the structure */
	if (END_OBJ_INIT(&et->endobj, (DEV_OBJ*)et, "etv", unit, &etFuncTable, "END etv Driver")
		== ERROR)
		goto fail;

	if (END_MIB_INIT(&et->endobj, M2_ifType_ethernet_csmacd, (char *)&et->etc->cur_etheraddr,
		ETHER_ADDR_LEN, ETHERMTU, 100000000) == ERROR)
		goto fail;

    /* share the packet pool of the real device */
	et->endobj.pNetPool = pRealDev->pNetPool;

	/* set the flags to indicate readiness */
	END_OBJ_READY(&et->endobj, (IFF_NOTRAILERS|IFF_BROADCAST|IFF_MULTICAST));

	et_global[unit] = et;

	/* add us to the global linked list */
	et->next = et_list;
	et_list = et;

	pVirtDev = &et->endobj;
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
	ET_TRACE(("etv%d: et_unload\n", et->etc->unit));

	pRealDev = NULL;
	pVirtDev = NULL;

	ASSERT(!et->etc->up);

	END_OBJECT_UNLOAD(&et->endobj);

	et_global[et->etc->unit] = NULL;

	et_free(et);

	return (OK);
}

static void
et_free(et_info_t *et)
{
	et_info_t **prev;

	if (et == NULL)
		return;

	ET_TRACE(("etv: et_free\n"));

	/* free common resources */
	if (et->etc) {
	    MFREE(et->etc, sizeof (etc_info_t));
		et->etc = NULL;
	}

	/* remove us from the global linked list */
	for (prev = &et_list; *prev; prev = &(*prev)->next)
		if (*prev == et) {
			*prev = et->next;
			break;
		}

	free((char*)et);
}

void
etv_up(et_info_t *et)
{
	ET_TRACE(("etv%d: etv_up\n", et->etc->unit));

	et->etc->up = TRUE;

	END_FLAGS_SET (&et->endobj, IFF_UP | IFF_RUNNING);

}

void
etv_down(et_info_t *et, int reset)
{
	etc_info_t *etc;

	etc = et->etc;

	ET_TRACE(("etv%d: etv_down\n", etc->unit));

	etc->up = FALSE;

    /* mark the driver as down */
    END_FLAGS_CLR (&et->endobj, IFF_UP | IFF_RUNNING);

}

/*
 * et_end_start - start the device
 */
static STATUS
et_end_start(et_info_t *et)
{
	ET_TRACE(("et%d: et_end_start\n", et->etc->unit));
	ET_LOCK(et);
	etv_up(et);
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
	etv_down(et, 1);
	ET_UNLOCK(et);
	return (OK);
}

/*
 * et_send - the driver send routine
 */
static STATUS
et_send(et_info_t *et, M_BLK_ID m)
{
	M_BLK_ID mbuf_new;
	M_BLK_ID mbuf_next;
	M_BLK_ID mbuf_last;
    int padBytesNeeded, newBytesNeeded, availBytes;

    /* send packet to real device (should have already had */
    /* vlan tag added by virt_endEtherAddressForm rtn) */
    if (pRealDev == NULL)
        return (END_ERR_BLOCK);

	/* make sure there's additional pad bytes for when packet gets untagged egressing switch */
	/* ETHER_MIN_LEN is min pkt size with vlan hdr and w/o FCS */
	padBytesNeeded = m->mBlkPktHdr.len - ETHER_MIN_LEN;
	if (padBytesNeeded < 0)
	{
	    padBytesNeeded = -padBytesNeeded;
	    /* see if buffer already has room, otherwise, allocate */
	    /* new buffer and chain to end of packet */
    	mbuf_next = m;
    	mbuf_last = mbuf_next;
    	while (mbuf_next != NULL)
    	{
    	    mbuf_last = mbuf_next;
    	    mbuf_next = mbuf_next->mBlkHdr.mNext;
    	}
    	availBytes = M_TRAILINGSPACE(mbuf_last);
    	newBytesNeeded = availBytes - padBytesNeeded;
	    if (newBytesNeeded < 0)
	    {
	        /* not enough trailing bytes available.  get new buffer */
	        mbuf_last->mBlkHdr.mLen += availBytes;
    	    /* zero the pad bytes added */
       	    memset(mbuf_last->mBlkHdr.mData + mbuf_last->mBlkHdr.mLen
       	            - availBytes,0,availBytes);
	        newBytesNeeded -= newBytesNeeded;
        	if (NULL != (mbuf_new = PKTGET(et,newBytesNeeded,0)))
        	{
        	    mbuf_new->mBlkHdr.mNext = NULL;
        	    mbuf_last->mBlkHdr.mNext = mbuf_new;
        	    /* zero the pad bytes added */
           	    memset(mbuf_new->mBlkHdr.mData,0,newBytesNeeded);
        	}
        	else
        	{
                ET_TRACE(("etv%d: vlan_start_xmit: couldn't expand buffer by %d bytes",
                        et->etc->unit, padBytesNeeded));
                return (END_ERR_BLOCK);
        	}
        } else {
            /* just use available trailing bytes */
	        mbuf_last->mBlkHdr.mLen += padBytesNeeded;
    	    /* zero the pad bytes added */
       	    memset(mbuf_last->mBlkHdr.mData + mbuf_last->mBlkHdr.mLen
       	            - padBytesNeeded,0,padBytesNeeded);
	    }
	    m->mBlkPktHdr.len += padBytesNeeded;
    }

	/* bump the statistic counter. */
	END_ERR_ADD(&et->endobj, MIB2_OUT_UCAST, +1);
    return ((*(pRealDev->pFuncTable->send))(pRealDev, m));
}

static int
et_ioctl(et_info_t *et, int cmd, caddr_t data)
{
	int error;
	long value;
	etc_info_t  *etc, *etc_realdev;

	etc = et->etc;
	error = 0;

	ET_TRACE(("etv%d: et_ioctl\n", et->etc->unit));

    if (pRealDev == NULL)
        return (EINVAL);
    etc_realdev =  ((et_info_t *)pRealDev)->etc;
	
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
	et->endobj.mib2Tbl.ifInErrors = etc_realdev->rxerror;
	et->endobj.mib2Tbl.ifOutErrors = etc_realdev->txerror;
	et->endobj.mib2Tbl.ifInOctets = etc_realdev->rxbyte;
	et->endobj.mib2Tbl.ifOutOctets = etc_realdev->txbyte;
	et->endobj.mib2Tbl.ifInUcastPkts = etc_realdev->rxframe;
	et->endobj.mib2Tbl.ifOutUcastPkts = etc_realdev->txframe;
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

	ET_TRACE(("etv%d: et_config\n", et->etc->unit));

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
			ET_ERROR(("etv%d: et_config: etc.nmulticast %d, endobj.nMulti %d\n",
				et->etc->unit, et->etc->nmulticast, et->endobj.nMulti));
		}
	}

	ET_UNLOCK(et);
}

/*
 * et_mcastAdd - add a multicast address for the device
 */
static STATUS
et_mcastAdd(et_info_t *et, char* pAddress)
{
	int error;

	ET_TRACE(("etv%d: et_mcastAdd\n", et->etc->unit));

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

	ET_TRACE(("etv%d: et_mcastDel\n", et->etc->unit));

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
	ET_TRACE(("etv%d: et_mcastGet\n", et->etc->unit));

	return (etherMultiGet(&et->endobj.multiList, pTable));
}

