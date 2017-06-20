/*
 * $Id: bist.c,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Built-in Self Test for KAPS TCAM
 */

#include <soc/defs.h>
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
#include <soc/dpp/ARAD/arad_kbp.h>
#include <soc/dpp/JER/JER_PP/jer_pp_kaps.h>
#include <soc/dpp/JER/JER_PP/jer_pp_kaps_diag.h>
#include <sal/types.h>
#include <appl/diag/test.h>

int
kaps_bist_test(int u, args_t *a, void *p)
{
    int rv = 0;

    rv = jer_pp_kaps_tcam_bist(u);;

    if (rv < 0) 
    {
        test_error(u, "KAPS TCAM BIST Failed!!!\n");
        return rv;
    }

    return rv;
}

#endif

