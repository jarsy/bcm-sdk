/*
 * $Id: bist.c,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Test for verifying KBP sdk ver compatibility with the SW sdk ver
 */

#include <soc/defs.h>
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
#include <soc/dpp/ARAD/arad_kbp.h>
#include <soc/dpp/JER/JER_PP/jer_pp_kaps.h>
#include <soc/dpp/JER/JER_PP/jer_pp_kaps_diag.h>
#include <sal/types.h>
#include <appl/diag/test.h>

int
kbp_sdk_ver_test(int u, args_t *a, void *p)
{
    int rv = 0;

    rv = jer_pp_kbp_sdk_ver_test(u);

    if (rv != 0)
    {
        test_error(u, "KBP SDK version test failed!!!\n");
        return rv;
    }

    return rv;
}

#endif

