/*
 * $Id: ipfix.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:       ipfix.c
 * Purpose:    IPFIX API
 */

#include <soc/drv.h>
#include <bcm/ipfix.h>
#include <bcm/error.h>

/*
 * Function:
 *     bcm_robo_ipfix_register
 * Description:
 *     To register the callback function for flow info export
 * Parameters:
 *     unit          device number
 *     callback      user callback function
 *     userdata      user data used as argument during callcack
 * Return:
 *     BCM_E_XXX
 */
int
bcm_robo_ipfix_register(int unit,
                       bcm_ipfix_callback_t callback,
                       void *userdata)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcm_robo_ipfix_unregister
 * Description:
 *     To unregister the callback function for flow info export
 * Parameters:
 *     unit          device number
 *     callback      user callback function
 *     userdata      user data used as argument during callcack
 * Return:
 *     BCM_E_XXX
 */
int
bcm_robo_ipfix_unregister(int unit,
                         bcm_ipfix_callback_t callback,
                         void *userdata)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcm_robo_ipfix_config_set function
 * Description:
 *     To set per port IPFIX configuration
 * Parameters:
 *     unit            device number
 *     port            port number
 *     stage           BCM_IPFIX_STAGE_INGRESS or BCM_IPFIX_STAGE_EGRESS
 *     config          pointer to ipfix configuration
 * Return:
 *     BCM_E_XXX
 */
int
bcm_robo_ipfix_config_set(int unit,
                          bcm_port_t port,
                          bcm_ipfix_stage_t stage,
                          bcm_ipfix_config_t *config)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcm_robo_ipfix_config_get function
 * Description:
 *     To get per port IPFIX configuration
 * Parameters:
 *     unit            device number
 *     port            port number
 *     stage           BCM_IPFIX_STAGE_INGRESS or BCM_IPFIX_STAGE_EGRESS
 *     config          pointer to ipfix configuration
 * Return:
 *     BCM_E_XXX
 */
int
bcm_robo_ipfix_config_get(int unit,
                          bcm_port_t port,
                          bcm_ipfix_stage_t stage,
                          bcm_ipfix_config_t *config)
{
    return BCM_E_UNAVAIL;
}

/* Add an IPFIX flow rate meter entry */
int bcm_robo_ipfix_rate_create(int unit, 
                        bcm_ipfix_rate_t *rate_info)
{
    return BCM_E_UNAVAIL;
}

/* Delete an IPFIX flow rate meter entry */
int bcm_robo_ipfix_rate_destroy(int unit, 
                        bcm_ipfix_rate_id_t rate_id)
{
    return BCM_E_UNAVAIL;
}

/* Get IPFIX flow rate meter entry for the specified id */
int bcm_robo_ipfix_rate_get(int unit, 
                        bcm_ipfix_rate_t *rate_info)
{
    return BCM_E_UNAVAIL;
}


/* Traverse through IPFIX flow rate meter entries */
int bcm_robo_ipfix_rate_traverse(int unit, 
                        bcm_ipfix_rate_traverse_cb cb, 
                        void *userdata)
{
    return BCM_E_UNAVAIL;
}

/* Delete all IPFIX flow rate meter entries */
int bcm_robo_ipfix_rate_destroy_all(int unit)
{
    return BCM_E_UNAVAIL;
}

/* Add a mirror destination to the IPFIX flow rate meter entry */
int bcm_robo_ipfix_rate_mirror_add(int unit, 
                        bcm_ipfix_rate_id_t rate_id, 
                        bcm_gport_t mirror_dest_id)
{
    return BCM_E_UNAVAIL;
}

/* Delete a mirror destination from the IPFIX flow rate meter entry */
int bcm_robo_ipfix_rate_mirror_delete(int unit, 
                        bcm_ipfix_rate_id_t rate_id, 
                        bcm_gport_t mirror_dest_id)
{
    return BCM_E_UNAVAIL;
}

/* 
 * Delete all mirror destination associated to the IPFIX flow rate meter
 * entry
 */
int bcm_robo_ipfix_rate_mirror_delete_all(int unit, 
                        bcm_ipfix_rate_id_t rate_id)
{
    return BCM_E_UNAVAIL;
}

/* Get all mirror destination from the IPFIX flow rate meter entry */
int bcm_robo_ipfix_rate_mirror_get(int unit, 
                        bcm_ipfix_rate_id_t rate_id, 
                        int mirror_dest_size, 
                        bcm_gport_t *mirror_dest_id, 
                        int *mirror_dest_count)
{
    return BCM_E_UNAVAIL;
}

/* Set IPFIX mirror control configuration of the specified port */
int bcm_robo_ipfix_mirror_config_set(int unit,
                                     bcm_ipfix_stage_t stage,
                                     bcm_gport_t port,
                                     bcm_ipfix_mirror_config_t *config)
{
    return BCM_E_UNAVAIL;
}

/* Get IPFIX mirror control configuration of the specified port */
int bcm_robo_ipfix_mirror_config_get(int unit,
                                     bcm_ipfix_stage_t stage,
                                     bcm_gport_t port,
                                     bcm_ipfix_mirror_config_t *config)
{
    return BCM_E_UNAVAIL;
}

/* Add an IPFIX mirror destination to the specified port */
int bcm_robo_ipfix_mirror_port_dest_add(int unit,
                                        bcm_ipfix_stage_t stage,
                                        bcm_gport_t port,
                                        bcm_gport_t mirror_dest_id)
{
    return BCM_E_UNAVAIL;
}

/* Delete an IPFIX mirror destination from the specified port */
int bcm_robo_ipfix_mirror_port_dest_delete(int unit,
                                           bcm_ipfix_stage_t stage,
                                           bcm_gport_t port,
                                           bcm_gport_t mirror_dest_id)
{
    return BCM_E_UNAVAIL;
}

/* Delete all IPFIX mirror destination from the specified port */
int bcm_robo_ipfix_mirror_port_dest_delete_all(int unit,
                                               bcm_ipfix_stage_t stage,
                                               bcm_gport_t port)
{
    return BCM_E_UNAVAIL;
}

/* Get all IPFIX mirror destination of the specified port */
int bcm_robo_ipfix_mirror_port_dest_get(int unit,
                                        bcm_ipfix_stage_t stage,
                                        bcm_gport_t port,
                                        int mirror_dest_size,
                                        bcm_gport_t *mirror_dest_id,
                                        int *mirror_dest_count)
{
    return BCM_E_UNAVAIL;
}
