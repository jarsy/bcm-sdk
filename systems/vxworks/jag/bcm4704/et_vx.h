/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * vxWorks 5.x device driver tunables for
 * Broadcom BCM44XX and BCM47XX 10/100Mbps Ethernet Device Driver
 *
 * Copyright(c) 2000 Broadcom Corp.
 * $Id: et_vx.h,v 1.1 2004/02/24 07:47:00 csm Exp $
 */

#ifndef _et_vx_h_
#define _et_vx_h_

/* tunables */
#define	NTXD		256		/* # tx dma ring descriptors (must be ^2) */
#define	NRXD		256		/* # rx dma ring descriptors (must be ^2) */
#define	NRXBUFPOST	128		/* try to keep this # rbufs posted to the chip */
#define	BUFSZ		2044		/* packet data buffer size */
#define	RXBUFSZ		(BUFSZ - 256)	/* receive buffer size */
#define	MAXMULTILIST	32		/* max # multicast addresses */
#define	MIN_CLUSTERS	1024
#define	MIN_PKTS	1024

#ifdef BROADCOM_BSP
#define BCM47XX_CHOPS
#endif

/* Vx prototype */
extern int sysClkRateGet(void);
#define	M_HEADROOM(m)	((m)->mBlkHdr.mData - (m)->pClBlk->clNode.pClBuf)

#endif	/* _et_vx_h_ */
