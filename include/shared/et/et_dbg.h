/*
 * $Id: et_dbg.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Minimal debug/trace/assert driver definitions for
 * Broadcom Home Networking Division 10/100 Mbit/s Ethernet
 * Device Driver.
 */

#ifndef _et_dbg_
#define _et_dbg_

#ifdef	BCMDBG
/*
 * et_msg_level is a bitvector:
 *	0	errors
 *	1	function-level tracing
 *	2	one-line frame tx/rx summary
 *	3	complex frame tx/rx in hex
 */
#include <shared/bsl.h>
#define	ET_ERROR(args)	if (!(et_msg_level & 1)) ; else bsl_printf args
#define	ET_TRACE(args)	if (!(et_msg_level & 2)) ; else bsl_printf args
#define	ET_PRHDR(msg, eh, len)	if (!(et_msg_level & 4)) ; else etc_soc_prhdr(msg, eh, len)
#define	ET_PRPKT(msg, buf, len)	if (!(et_msg_level & 8)) ; else prhex(msg, buf, len)
#define	EB_TRACE(args)	if (!(et_msg_level & 0x10)) ; else bsl_printf args
extern void etc_soc_prhdr(char *msg, struct ether_header *eh, uint len);
#else	/* BCMDBG */
#define	ET_ERROR(args)
#define	ET_TRACE(args)
#define	ET_PRHDR(msg, eh, len)
#define	ET_PRPKT(msg, buf, len)
#define	EB_TRACE(args)
#endif	/* BCMDBG */

extern int et_msg_level;

#ifdef BCMINTERNAL
#define	ET_LOG(fmt, a1, a2)	if (!(et_msg_level & 0x10000)) ; else bcmlog(fmt, a1, a2)
#else
#define	ET_LOG(fmt, a1, a2)
#endif

/* include port-specific tunables */
#ifdef NDIS
#include <et_ndis.h>
#elif defined(vxworks)
#include <et_vx.h>
#elif defined(linux) && defined(__KERNEL__)
#include <soc/et_soc.h>
#elif defined(PMON)
#include <et_pmon.h>
#elif defined(_CFE_)
#include <et_cfe.h>
#else
#include <soc/et_soc.h>
#endif

#endif /* _et_dbg_ */
