/*
 * $Id: et_linux.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Linux device driver tunables for
 * Broadcom BCM44XX and BCM47XX 10/100Mbps Ethernet Device Driver
 */

#ifndef _et_linux_h_
#define _et_linux_h_

/* tunables */
#define	NTXD		64		/* # tx dma ring descriptors (must be ^2) */
#define	NRXD		512		/* # rx dma ring descriptors (must be ^2) */
#define	NRXBUFPOST	32		/* try to keep this # rbufs posted to the chip */
#define	BUFSZ		2048		/* packet data buffer size */
#define	RXBUFSZ		(BUFSZ - 256)	/* receive buffer size */

#if defined(ILSIM) || defined(__arch_um__)
#undef	NTXD
#define	NTXD		16
#undef	NRXD
#define	NRXD		16
#undef	NRXBUFPOST
#define	NRXBUFPOST	2
#endif

#endif	/* _et_linux_h_ */
