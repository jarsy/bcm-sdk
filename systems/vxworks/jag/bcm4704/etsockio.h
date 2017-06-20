/*
    EXTERNAL SOURCE RELEASE on 12/03/2001 3.0 - Subject to change without notice.

*/
/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * Driver-specific socket ioctls
 * used by BSD, Linux, and PSOS
 * Broadcom BCM44XX 10/100Mbps Ethernet Device Driver
 *
 * Copyright (C) 2000 Broadcom Corporation
 *
 * $Id: etsockio.h,v 1.1 2004/02/24 07:47:00 csm Exp $
 */

#ifndef _etsockio_h_
#define _etsockio_h_

/* THESE MUST BE CONTIGUOUS AND CONSISTENT WITH VALUES IN ETC.H */


#if defined(linux) || 0
#define SIOCSETCUP		(SIOCDEVPRIVATE + 0)
#define SIOCSETCDOWN		(SIOCDEVPRIVATE + 1)
#define SIOCSETCLOOP		(SIOCDEVPRIVATE + 2)
#define SIOCGETCDUMP		(SIOCDEVPRIVATE + 3)
#define SIOCSETCSETMSGLEVEL	(SIOCDEVPRIVATE + 4)
#define SIOCSETCPROMISC		(SIOCDEVPRIVATE + 5)
#define SIOCSETCTXDOWN		(SIOCDEVPRIVATE + 6)	/* obsolete */
#define SIOCSETCSPEED		(SIOCDEVPRIVATE + 7)
#define SIOCTXGEN		(SIOCDEVPRIVATE + 8)

#else	/* !linux */

#define SIOCSETCUP		_IOWR('e', 130 + 0, struct ifreq)
#define SIOCSETCDOWN		_IOWR('e', 130 + 1, struct ifreq)
#define SIOCSETCLOOP		_IOWR('e', 130 + 2, struct ifreq)
#define SIOCGETCDUMP		_IOWR('e', 130 + 3, struct ifreq)
#define SIOCSETCSETMSGLEVEL	_IOWR('e', 130 + 4, struct ifreq)
#define SIOCSETCPROMISC		_IOWR('e', 130 + 5, struct ifreq)
#define SIOCSETCTXDOWN		_IOWR('e', 130 + 6, struct ifreq)	/* obsolete */
#define SIOCSETCSPEED		_IOWR('e', 130 + 7, struct ifreq)
#define SIOCTXGEN		_IOWR('e', 130 + 8, struct ifreq)

#endif

/* arg to SIOCTXGEN */
struct txg {
	uint32 num;		/* number of frames to send */
	uint32 delay;		/* delay in microseconds between sending each */
	uint32 size;		/* size of ether frame to send */
	uchar buf[1514];	/* starting ether frame data */
};

#endif
