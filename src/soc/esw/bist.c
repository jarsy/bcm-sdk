/*
 * $Id: bist.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#include <soc/error.h>
#include <soc/drv.h>

/* No longer used - preserved for historical reasons */
int
soc_bist(int unit, soc_mem_t *mems, int num_mems, int timeout_msec)
{
    return SOC_E_NONE;
}
