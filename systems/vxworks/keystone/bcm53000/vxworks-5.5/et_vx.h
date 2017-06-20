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
 * Broadcom BCM47XX 10/100Mbps Ethernet Device Driver
 *
 * Copyright(c) 2000 Broadcom Corp.
 * $Id: et_vx.h,v 1.3 2010/01/13 06:13:07 alai Exp $
 */

#ifndef _et_vx_h_
#define _et_vx_h_

#include <ioctl.h>

/* PED: net buffer allocation tunables */
#define PED_USE_LINK_BUFFER_POOL    (0) /* Should be clear for non-vxbridge */
#define PED_USE_UNCACHED_BUFFER     (1)

/* PED: change NTXD/NRXD/NRXBUFPOST  */

/* tunables */
#define NTXD	        128
#define NRXD	        128
#define NRXBUFPOST	    32   
#define	BUFSZ		2044		/* packet data buffer size */
#define	RXBUFSZ		(BUFSZ - 256)	/* receive buffer size */
#define	MAXMULTILIST	32		/* max # multicast addresses */
#define	MIN_CLUSTERS	256     /* PED: shortcut netTupleGet */
#define	MIN_PKTS	256

/* Vx prototype */
extern int sysClkRateGet(void);
#define	M_HEADROOM(m)	((m)->mBlkHdr.mData - (m)->pClBlk->clNode.pClBuf)

#endif	/* _et_vx_h_ */
