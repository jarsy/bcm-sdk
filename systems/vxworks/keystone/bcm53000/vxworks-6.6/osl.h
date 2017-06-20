/*
 * OS Abstraction Layer
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $Id: osl.h,v 1.1 Broadcom SDK $
 */

#ifndef _osl_h_
#define _osl_h_

/* osl handle type forward declaration */
typedef struct osl_info osl_t;
typedef struct osl_dmainfo osldma_t;

#define OSL_PKTTAG_SZ	32 /* Size of PktTag */

/* Drivers use PKTFREESETCB to register a callback function when a packet is freed by OSL */
typedef void (*pktfree_cb_fn_t)(void *ctx, void *pkt, unsigned int status);

#ifdef OSLREGOPS
/* Drivers use REGOPSSET() to register register read/write funcitons */
typedef unsigned int (*osl_rreg_fn_t)(void *ctx, void *reg, unsigned int size);
typedef void  (*osl_wreg_fn_t)(void *ctx, void *reg, unsigned int val, unsigned int size);
#endif

#if defined(vxworks)
#include "vx_osl.h"
#elif defined(__ECOS)
#include "ecos_osl.h"
#elif  defined(DOS)
#include <dos_osl.h>
#elif defined(PCBIOS)
#include <pcbios_osl.h>
#elif defined(__IOPOS__)
#include <iopos_osl.h>
#elif defined(linux)
#include <asm/bcmsi/linux_osl.h>
#elif defined(NDIS)
#include <ndis_osl.h>
#elif defined(_CFE_)
#include <cfe_osl.h>
#elif defined(_HNDRTE_)
#include <hndrte_osl.h>
#elif defined(_MINOSL_)
#include <min_osl.h>
#elif defined(MACOSX)
#include <macosx_osl.h>
#elif defined(__NetBSD__)
#include <bsd_osl.h>
#elif defined(EFI)
#include <efi_osl.h>
#else
#error "Unsupported OSL requested"
#endif /* defined(vxworks) */

/* handy */
#define	SET_REG(osh, r, mask, val)	W_REG((osh), (r), ((R_REG((osh), r) & ~(mask)) | (val)))

#endif	/* _osl_h_ */
