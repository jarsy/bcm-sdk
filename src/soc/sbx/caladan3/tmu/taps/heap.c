/*
 * $Id: heap.c,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    heap.c
 * Purpose: Caladan3 heap library (for now, designed for TAPS only)
 * Requires:
 */

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/types.h>
#include <soc/drv.h>
#include <soc/sbx/caladan3/tmu/taps/heap.h>

/*
 *
 * Function:
 *     taps_heap_init
 * Purpose:
 *     Bring up TAPS heap drivers
 */
int taps_heap_init(int unit) 
{
     int status = SOC_E_NONE;

     return status;
}

#endif /* BCM_CALADAN3_SUPPORT */
