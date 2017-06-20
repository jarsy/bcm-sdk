/*
 * $Id: et_soc.h,v 1.3.2.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Broadcom device driver tunables for
 * Broadcom BCM44XX and BCM47XX 10/100Mbps Ethernet Device Driver
 *
 * Note: This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 */

#ifndef _et_soc_h_
#define _et_soc_h_

/* tunables */
#define	NTXD        (16)      /* # tx dma ring descriptors (must be ^2) */
#define	NRXD        (64)     /* # rx dma ring descriptors (must be ^2) */
#define	NRXBUFPOST  ((NRXD > 64)? 64: (NRXD - 1))
                              /* try to keep this # rbufs posted to the chip */
#define	BUFSZ       (2048)    /* packet data buffer size */
#define	RXBUFSZ     (BUFSZ - 256)  /* receive buffer size */

#endif	/* _et_soc_h_ */
