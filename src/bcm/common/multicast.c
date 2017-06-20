/*
 * $Id: multicast.c,v 1.17 2014/05/11 15:07:11 Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/debug.h>

#include <sal/core/boot.h>

#include <bcm/error.h>
#include <bcm/multicast.h>

/*
 * Function:
 *      bcm_vlan_ip_t_init
 * Purpose:
 *      Initialize the vlan_ip_t structure
 * Parameters:
 *      vlan_ip - Pointer to structure which should be initialized
 * Returns:
 *      NONE
*/
void 
bcm_multicast_replication_t_init(bcm_multicast_replication_t *replication)
{
    if (NULL != replication) {
        sal_memset(replication, 0, sizeof (bcm_multicast_replication_t));
    }
    return;
}

/*
 * Function:
 *      bcm_multicast_multi_info_t_init
 * Purpose:
 *      Initialize multicast multi group struct.
 * Parameters:
 *      ecmp_member - pointer to multicast multi struct.
 * Returns:
 *      NONE
 */
void
bcm_multicast_multi_info_t_init(bcm_multicast_multi_info_t *mc_multi_info)
{
    if (NULL != mc_multi_info) {
        sal_memset(mc_multi_info, 0, sizeof (*mc_multi_info));
    }
    return;
}

