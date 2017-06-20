/*
 * $Id: imfswap32.h,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        imfswap32.h
 * Purpose:
 */

#ifndef _BCM_APPL_SBX_IMFSWAP_H
#define _BCM_APPL_SBX_IMFSWAP_H

#define IMFSWAP32(val) \
        ((uint32)( \
                (((uint32)(val) & (uint32)0x000000ffUL) << 24) | \
                (((uint32)(val) & (uint32)0x0000ff00UL) <<  8) | \
                (((uint32)(val) & (uint32)0x00ff0000UL) >>  8) | \
                (((uint32)(val) & (uint32)0xff000000UL) >> 24) ))

#endif /* _BCM_APPL_SBX_IMFSWAP_H */
