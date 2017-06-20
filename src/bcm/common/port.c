/*
 * $Id: port.c,v 1.19 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/register.h>
#include <soc/memory.h>
#include <soc/ll.h>
#include <soc/ptable.h>
#ifdef BCM_ESW_SUPPORT
#include <soc/tucana.h>
#endif
#include <bcm/port.h>
#include <bcm/vlan.h>
#include <bcm/error.h>
#include <bcm/link.h>
#include <bcm/stack.h>
#include <bcm/stg.h>
#include <bcm/rate.h>


/*
* Function:
*      bcm_port_tpid_class_t_init
* Purpose:
*      Initialize the bcm_port_tpid_class_t structure
* Parameters:
*      info - Pointer to the structure which should be initialized
* Returns:
*      NONE
*/
void 
bcm_port_tpid_class_t_init(bcm_port_tpid_class_t *tpid_class)
{
    if (NULL != tpid_class) {
        sal_memset(tpid_class, 0, sizeof (*tpid_class));
    }
    return;
}

/*
* Function:
*      bcm_port_info_t_init
* Purpose:
*      Initialize the bcm_port_info_t structure
* Parameters:
*      info - Pointer to structure which should be initialized
* Returns:
*      NONE
*/
void 
bcm_port_info_t_init(bcm_port_info_t *info)
{
    if (NULL != info) {
        sal_memset(info, 0, sizeof (*info));
    }
    return;
}

/*
 * Function:
 *      bcm_port_config_t_init
 * Purpose:
 *      Initialize a Port config object struct.
 * Parameters:
 *      pconfig - Pointer to the Port Config object struct.
 * Returns:
 *      NONE
 */
void
bcm_port_config_t_init(bcm_port_config_t *pconfig)
{
    if (pconfig != NULL) {
        sal_memset(pconfig, 0, sizeof (*pconfig));
    }
    return;
}


/*
 * Function:
 *      bcm_port_interface_config_t_init
 * Purpose:
 *      Initialize a Port interface config object struct.
 * Parameters:
 *      pconfig - Pointer to the Port Interface Config object struct.
 * Returns:
 *      NONE
 */
void
bcm_port_interface_config_t_init(bcm_port_interface_config_t *pconfig)
{
    if (pconfig != NULL) {
        sal_memset(pconfig, 0, sizeof (*pconfig));
    }
    return;
}


/*
 * Function:
 *      bcm_port_ability_t_init
 * Purpose:
 *      Initialize a Port ability object struct.
 * Parameters:
 *      pconfig - Pointer to the Port ability object struct.
 * Returns:
 *      NONE
 */
void
bcm_port_ability_t_init(bcm_port_ability_t *pability)
{
    if (pability != NULL) {
        sal_memset(pability, 0, sizeof (*pability));
    }
    return;
}

/*
 * Function:
 *      bcm_phy_config_t_init
 * Purpose:
 *      Initialize a PHY config object struct.
 * Parameters:
 *      pconfig - Pointer to the Port ability object struct.
 * Returns:
 *      NONE
 */
void
bcm_phy_config_t_init(bcm_phy_config_t *pconfig)
{
    if (pconfig != NULL) {
        sal_memset(pconfig, 0, sizeof (*pconfig));
    }
    return;
}

/*
 * Function:
 *      bcm_port_encap_config_t_init
 * Purpose:
 *      Initialize a Port encapsulation config object struct.
 * Parameters:
 *      encap_config - Pointer to the Port encapsultation config object struct.
 * Returns:
 *      NONE
 */
void
bcm_port_encap_config_t_init(bcm_port_encap_config_t *encap_config)
{
    if (encap_config != NULL) {
        sal_memset(encap_config, 0, sizeof (*encap_config));
    }
    return;
}

/*
 * Function:
 *      bcm_port_congestion_config_t_init
 * Purpose:
 *      Initialize a Port congestion config object struct.
 * Parameters:
 *      config - Pointer to the Port congestion config object struct.
 * Returns:
 *      NONE
 */
void
bcm_port_congestion_config_t_init(bcm_port_congestion_config_t *config)
{
    if (config != NULL) {
        sal_memset(config, 0, sizeof (*config));
    }
    return;
}

/* 
 * Function:
 *      bcm_port_match_info_t_init
 * Purpose:
 *      Initialize the port_match_info struct
 * Parameters: 
 *      port_match_info - Pointer to the struct to be init'ed
 */
void
bcm_port_match_info_t_init(bcm_port_match_info_t *port_match_info)
{   
    if (port_match_info != NULL) {
        sal_memset(port_match_info, 0, sizeof(*port_match_info));
    }
    return;
}

/*
 * Function:
 *      bcm_port_timesync_config_t_init
 * Purpose:
 *      Initialize a Port config object struct.
 * Parameters:
 *      pconfig - Pointer to the Port Config object struct.
 * Returns:
 *      NONE
 */
void
bcm_port_timesync_config_t_init(bcm_port_timesync_config_t *pconfig)
{
    if (pconfig != NULL) {
        sal_memset(pconfig, 0, sizeof (*pconfig));
    }
    return;
}

/*
 * Function:
 *      bcm_port_phy_timesync_config_t_init
 * Purpose:
 *      Initialize a Port config object struct.
 * Parameters:
 *      pconfig - Pointer to the Port Config object struct.
 * Returns:
 *      NONE
 */
void
bcm_port_phy_timesync_config_t_init(bcm_port_phy_timesync_config_t *pconfig)
{
    if (pconfig != NULL) {
        sal_memset(pconfig, 0, sizeof (*pconfig));
    }
    return;
}

/*
 * Function:
 *      bcm_priority_mapping_t_init
 * Purpose:
 *      Initialize a port vlan priority mapping struct.
 * Parameters:
 *      pri_map - Pointer to the pri_map struct.
 * Returns:
 *      NONE
 */
void
bcm_priority_mapping_t_init(bcm_priority_mapping_t *pri_map)
{
    if (pri_map != NULL) {
        sal_memset(pri_map, 0, sizeof (*pri_map));
    }
    return;
}

/*
 * Function:
 *      bcm_port_extender_mapping_info_t_init
 * Purpose:
 *      Initialize a port extender mapping struct.
 * Parameters:
 *      mapping_info - Pointer to the port extender mapping struct.
 * Returns:
 *      NONE
 */
void
bcm_port_extender_mapping_info_t_init(bcm_port_extender_mapping_info_t *mapping_info)
{
    if (mapping_info != NULL) {
        sal_memset(mapping_info, 0, sizeof (*mapping_info));
    }
    return;
}

/*
 * Function:
 *      bcm_port_resource_t_init
 * Purpose:
 *      Initialize a Port Resource data structure.
 * Parameters:
 *      resource - Pointer to Port Resource data structure to 
 *                 be initialized.
 * Returns:
 *      None
 */
void
bcm_port_resource_t_init(bcm_port_resource_t *resource)
{
    if (resource != NULL) {
        sal_memset(resource, 0, sizeof (*resource));
    }

    return;
}

/*
 * Function:
 *      bcm_port_priority_group_config_t_init
 * Purpose:
 *      Initialize a Port Priority Group configuration struct.
 * Parameters:
 *      prigrp_config - Pointer to the priority group configuration struct.
 * Returns:
 *      None
 */
void
bcm_port_priority_group_config_t_init(bcm_port_priority_group_config_t *prigrp_config)
{
    if (prigrp_config != NULL) {
        sal_memset(prigrp_config, 0, sizeof (*prigrp_config));
    }

    return;
}

/*
 * Function:
 *      bcm_port_redirect_config_t_init
 * Purpose:
 *      Initialize the redirection config structure
 * Parameters:
 *      unit             - (IN) device id.
 *      redirect_config  - (IN) Pointer to the redirection config structure
 * Returns:
 *      None
 */
void
bcm_port_redirect_config_t_init(bcm_port_redirect_config_t *redirect_config)
{
    /* Sanity check */
    if(redirect_config == NULL) {
        return;
    }

    /* Memset the struct */
    sal_memset(redirect_config, 0, sizeof(bcm_port_redirect_config_t));

    /* Intiialize the config struct fields to defaults */
    redirect_config->flags = BCM_PORT_REDIRECT_SOURCE_USE_ORIGINAL_SOURCE;
    redirect_config->buffer_priority = BCM_PORT_REDIRECT_BUFFER_PRIORITY_LOW;
    redirect_config->destination = -1;

    return;
}

/*
 * Description:
 *        Initialize a bcm_port_e2efc_remote_module_config_t structure.
 * Parameter:
 *        e2efc_rmod_cfg - Pointer to bcm_port_e2efc_remote_module_config_t
 */
void
bcm_port_e2efc_remote_module_config_t_init(
    bcm_port_e2efc_remote_module_config_t *e2efc_rmod_cfg)
{
    if (e2efc_rmod_cfg != NULL) {
        sal_memset(e2efc_rmod_cfg, 0, sizeof (*e2efc_rmod_cfg));
    }

    return;
}

/*
 * Description:
 *        Initialize a bcm_port_e2efc_remote_port_config_t structure.
 * Parameter:
 *        e2efc_rport_cfg - Pointer to bcm_port_e2efc_remote_port_config_t
 */
void
bcm_port_e2efc_remote_port_config_t_init(
    bcm_port_e2efc_remote_port_config_t *e2efc_rport_cfg)
{
    if (e2efc_rport_cfg != NULL) {
        sal_memset(e2efc_rport_cfg, 0, sizeof (*e2efc_rport_cfg));
    }

    return;
}

