/*
 * $Id:  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * ECN - Broadcom StrataSwitch ECN API
 */

#include <soc/drv.h>
#include <bcm/ecn.h>

/*
 * Function:
 *      bcm_ecn_traffic_map_info_t_init
 * Purpose:
 *      Initialize an ECN traffic map information structure 
 * Parameters:
 *      ecn_map - (OUT) pointer to bcm_ecn_traffic_map_info_t structure
 */
void 
bcm_ecn_traffic_map_info_t_init(
    bcm_ecn_traffic_map_info_t *ecn_map)
{
    if (NULL != ecn_map) {
        sal_memset(ecn_map, 0, sizeof (*ecn_map));
    }
    return;
}

/*
 * Function:
 *      bcm_ecn_traffic_action_config_t_init
 * Purpose:
 *      Initialize an ECN traffic action config structure
 * Parameters:
 *      ecn_action - (OUT) pointer to bcm_ecn_traffic_action_config_t structure
 */
void 
bcm_ecn_traffic_action_config_t_init(
    bcm_ecn_traffic_action_config_t *ecn_action)
{
    if (NULL != ecn_action) {
        sal_memset(ecn_action, 0, sizeof (*ecn_action));
    }
    return;

}
#if defined(INCLUDE_L3)
/*
 * Function:
 *      bcm_ecn_map_t_init
 * Purpose:
 *      Initialize the MPLS ECN map structure
 * Parameters:
 *      info - Pointer to the struct to be init'ed
 */

void
bcm_ecn_map_t_init(bcm_ecn_map_t *ecn_map)
{
    if (ecn_map != NULL) {
        sal_memset(ecn_map, 0, sizeof(*ecn_map));
    }
    return;
}

/*
 * Function:
 *      bcm_ecn_port_map_t_init
 * Purpose:
 *      Initialize the MPLS ECN port map structure
 * Parameters:
 *      info - Pointer to the struct to be init'ed
 */

void
bcm_ecn_port_map_t_init(bcm_ecn_port_map_t *ecn_map)
{
    if (ecn_map != NULL) {
        sal_memset(ecn_map, 0, sizeof(*ecn_map));
        ecn_map->ecn_map_id = -1;
    }    
    return;
}
#endif /*INCLUDE_L3*/
