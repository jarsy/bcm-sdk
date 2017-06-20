/*
 * $Id: l2.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * L2 - Broadcom StrataSwitch Layer-2 switch API.
 */

#include <shared/bsl.h>

#include <soc/cm.h>

#include <bcm/types.h>
#include <bcm/error.h>

#include <soc/l2x.h>
#include <bcm/l2.h>

#include <bcm_int/sbx/mbcm.h>

/*
 * Function:
 *     bcm_l2_key_dump
 * Purpose:
 *     Dump the key (VLAN+MAC) portion of a hardware-independent
 *     L2 address for debugging.
 * Parameters:
 *     unit  - Unit number
 *     pfx   - String to print before output
 *     entry - Hardware-independent L2 entry to dump
 *     sfx   - String to print after output
 * Returns:
 *     BCM_E_NONE  - Success
 *     BCM_E_PARAM - Failure, null param
 */
int
bcm_sbx_l2_key_dump(int unit, char *pfx, bcm_l2_addr_t *entry, char *sfx)
{
    if ((pfx == NULL) || (entry == NULL) || (sfx == NULL)) {
        return BCM_E_PARAM;
    }

    LOG_CLI((BSL_META_U(unit,
                        "l2: %sVLAN=0x%03x MAC=0x%02x%02x%02x"
                        "%02x%02x%02x%s"), pfx, entry->vid,
             entry->mac[0], entry->mac[1], entry->mac[2],
             entry->mac[3], entry->mac[4], entry->mac[5], sfx));

    return BCM_E_NONE;
}
