/*
 * $Id: bist.c,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Built-in Self Test for KAPS TCAM
 */

#include <soc/defs.h>
#include <sal/types.h>
#include <appl/diag/test.h>
#ifdef BCM_PETRA_SUPPORT
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>

int
lif_learn_info_bist_test(int u, args_t *a, void *p)
{
    int rv = 0;

    rv = arad_pp_fwd_decision_learn_buffer_bist(u);

    if (rv < 0) 
    {
        test_error(u, "lif learn info bist test Failed!!!\n");
        return rv;
    }

    return rv;
}

#endif
