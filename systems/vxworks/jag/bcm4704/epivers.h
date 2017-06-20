/*
 * Copyright 1999 Epigram, Inc.
 *
 * $Id: epivers.h,v 1.1 2004/02/24 07:47:00 csm Exp $
 *
*/

#ifndef _epivers_h_
#define _epivers_h_

#ifdef	linux
#include <linux/config.h>
#endif

/* Vendor Name, ASCII, 32 chars max */
#ifdef COMPANYNAME
#define	HPNA_VENDOR 		COMPANYNAME
#else
#define	HPNA_VENDOR 		"Broadcom Corporation"
#endif

/* Driver Date, ASCII, 32 chars max */
#define HPNA_DRV_BUILD_DATE	__DATE__

/* Hardware Manufacture Date, ASCII, 32 chars max */
#define HPNA_HW_MFG_DATE	"Not Specified"

/* See documentation for Device Type values, 32 values max */
#ifndef	HPNA_DEV_TYPE

#if	defined(BCM94100)
#define HPNA_DEV_TYPE		{ CDCF_V0_DEVICE_ETH_BRIDGE }

#elif	defined(BCM93310) || defined(BCM93350)
#define HPNA_DEV_TYPE		{ CDCF_V0_DEVICE_CM_BRIDGE }

#elif	defined(CONFIG_BRCM_VJ)
#define HPNA_DEV_TYPE		{ CDCF_V0_DEVICE_DISPLAY }

#elif	defined(CONFIG_BCRM_93725)
#define HPNA_DEV_TYPE		{ CDCF_V0_DEVICE_CM_BRIDGE, CDCF_V0_DEVICE_DISPLAY }

#else
#define HPNA_DEV_TYPE		{ CDCF_V0_DEVICE_PCINIC }

#endif

#endif	/* !HPNA_DEV_TYPE */


#define	EPI_MAJOR_VERSION	2002

#define	EPI_MINOR_VERSION	7

#define	EPI_RC_NUMBER		11

#define	EPI_INCREMENTAL_NUMBER	0

#define EPI_BUILD_NUMBER	1

#define	EPI_VERSION		2002,7,11,0

/* Driver Version String, ASCII, 32 chars max */
#ifdef BCMINTERNAL
#define	EPI_VERSION_STR		"2002.7.11.0 (BROADCOM INTERNAL)"
#else
#define	EPI_VERSION_STR		"2002.7.11.0"
#endif

#endif /* _epivers_h_ */
