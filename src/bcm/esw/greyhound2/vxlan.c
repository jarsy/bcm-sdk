/*
 * $Id: $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * VXLAN API
 */

#include <shared/bsl.h>

#include <soc/defs.h>
#include <sal/core/libc.h>
#include <soc/drv.h>
#include <soc/scache.h>
#include <soc/util.h>
#include <soc/hash.h>
#include <soc/debug.h>
#include <bcm/types.h>
#include <bcm/error.h>

#include <bcm/types.h>
#include <bcm/l3.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/virtual.h>
#include <bcm_int/esw/vxlan.h>
#include <bcm_int/esw/greyhound2.h>

#if defined(BCM_GREYHOUND2_SUPPORT) && defined(INCLUDE_L3)

/*
 * Function:
 *    bcmi_gh2_vxlan_lock
 * Purpose:
 *    Take VXLAN Lock Sempahore
 * Parameters:
 *    unit - (IN) Device Number
 * Returns:
 *    BCM_E_XXX
 */
int
bcmi_gh2_vxlan_lock(int unit)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_unlock
 * Purpose:
 *    Release  VXLAN Lock Semaphore
 * Parameters:
 *    unit - (IN) Device Number
 * Returns:
 *    BCM_E_XXX
 */
void
bcmi_gh2_vxlan_unlock(int unit)
{
    /* TBD */
}

/*
 * Function:
 *    bcmi_gh2_vxlan_udp_dest_port_set
 * Purpose:
 *    Set UDP Dest port for VXLAN
 * Parameters:
 *    unit - (IN) Device Number
 *    type - (IN) The desired configuration parameter to modify
 *    udp_destport - (IN) UDP Dest port (Non-zero value)
 * Returns:
 *    BCM_E_XXX.
 */
int
bcmi_gh2_vxlan_udp_dest_port_set(
    int unit,
    bcm_switch_control_t type,
    int udp_destport)
{
    soc_reg_t reg = INVALIDr ;
    soc_field_t reg_field = INVALIDf;
    uint64 regval64;
    uint32 fieldval;

    COMPILER_64_ZERO(regval64);

    /* Given control type select register. */
    switch (type) {
        case bcmSwitchVxlanUdpDestPortSet:
            reg = ING_VXLAN_CONTROLr;
            reg_field = UDP_DST_PORT_NUMBER_1f;
            break;
        case bcmSwitchVxlanUdpDestPortSet1:
            reg = ING_VXLAN_CONTROLr;
            reg_field = UDP_DST_PORT_NUMBER_2f;
            break;
        default:
            return BCM_E_PARAM;
    }

    if (!SOC_REG_FIELD_VALID(unit, reg, reg_field)) {
        return BCM_E_UNAVAIL;
    }

    /* input parameter range check */
    if ((udp_destport < 0) || (udp_destport > 0xFFFF) ) {
         return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(
        soc_reg64_get(unit, reg, REG_PORT_ANY, 0, &regval64));
    fieldval = soc_reg64_field32_get(unit, reg, regval64, reg_field);
    if (udp_destport != fieldval) {
        soc_reg64_field32_set(unit, reg, &regval64, reg_field, udp_destport);
        BCM_IF_ERROR_RETURN(
            soc_reg64_set(unit, reg, REG_PORT_ANY, 0, regval64));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_udp_dest_port_get
 * Purpose:
 *    Get UDP Dest port for VXLAN
 * Parameters:
 *    unit - (IN) Device Number
 *    type - (IN) The desired configuration parameter to modify
 *    udp_destport - (OUT) UDP Dest port (Non-zero value)
 * Returns:
 *    BCM_E_XXX.
 */
int
bcmi_gh2_vxlan_udp_dest_port_get(
    int unit,
    bcm_switch_control_t type,
    int *udp_destport)
{
    soc_reg_t reg = INVALIDr ;
    soc_field_t reg_field = INVALIDf;
    uint64 regval64;
    uint32 fieldval;

    COMPILER_64_ZERO(regval64);

    /* Given control type select register. */
    switch (type) {
        case bcmSwitchVxlanUdpDestPortSet:
            reg = ING_VXLAN_CONTROLr;
            reg_field = UDP_DST_PORT_NUMBER_1f;
            break;
        case bcmSwitchVxlanUdpDestPortSet1:
            reg = ING_VXLAN_CONTROLr;
            reg_field = UDP_DST_PORT_NUMBER_2f;
            break;
        default:
            return BCM_E_PARAM;
    }

    if (!SOC_REG_FIELD_VALID(unit, reg, reg_field)) {
        return BCM_E_UNAVAIL;
    }

    BCM_IF_ERROR_RETURN(
        soc_reg64_get(unit, reg, REG_PORT_ANY, 0, &regval64));
    fieldval = soc_reg64_field32_get(unit, reg, regval64, reg_field);

    *udp_destport = fieldval;

    return BCM_E_NONE;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_udp_source_port_set
 * Purpose:
 *    Enable UDP Source port based HASH for VXLAN
 * Parameters:
 *    unit - (IN) Device Number
 *    type - (IN) The desired configuration parameter to modify
 *    hash_enable - (IN) Enable Hash for UDP SourcePort
 * Returns:
 *    BCM_E_XXX.
 */
int
bcmi_gh2_vxlan_udp_source_port_set(
    int unit,
    bcm_switch_control_t type,
    int hash_enable)
{
    soc_reg_t reg = INVALIDr ;
    soc_field_t reg_field = INVALIDf;
    uint32 regval, fieldval;

    /* Given control type select register. */
    switch (type) {
        case bcmSwitchVxlanEntropyEnable:
            reg = EGR_VXLAN_CONTROLr;
            reg_field = UDP_SOURCE_PORT_SELf;
            break;
        case bcmSwitchVxlanEntropyEnableIP6:
            reg = EGR_VXLAN_CONTROLr;
            reg_field = IPV6_FLOW_LABEL_SELf;
            break;
        default:
            return BCM_E_PARAM;
    }

    if (!SOC_REG_FIELD_VALID(unit, reg, reg_field)) {
        return BCM_E_UNAVAIL;
    }

    if ((hash_enable < 0) || (hash_enable > 1) ) {
         return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(
        soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &regval));
    fieldval = soc_reg_field_get(unit, reg, regval, reg_field);
    if (hash_enable != fieldval) {
        soc_reg_field_set(unit, reg, &regval, reg_field, hash_enable);
        BCM_IF_ERROR_RETURN(
            soc_reg32_set(unit, reg, REG_PORT_ANY, 0, regval));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_udp_source_port_get
 * Purpose:
 *    Get UDP Source port based HASH for VXLAN
 * Parameters:
 *    unit - (IN) Device Number
 *    type - (IN) The desired configuration parameter to modify
 *    hash_enable - (OUT) Enable Hash for UDP SourcePort
 * Returns:
 *    BCM_E_XXX.
 */
int
bcmi_gh2_vxlan_udp_source_port_get(
    int unit,
    bcm_switch_control_t type,
    int *hash_enable)
{
    soc_reg_t reg = INVALIDr ;
    soc_field_t reg_field = INVALIDf;
    uint32 regval, fieldval;

    /* Given control type select register. */
    switch (type) {
        case bcmSwitchVxlanEntropyEnable:
            reg = EGR_VXLAN_CONTROLr;
            reg_field = UDP_SOURCE_PORT_SELf;
            break;
        case bcmSwitchVxlanEntropyEnableIP6:
            reg = EGR_VXLAN_CONTROLr;
            reg_field = IPV6_FLOW_LABEL_SELf;
            break;
        default:
            return BCM_E_PARAM;
    }

    if (!SOC_REG_FIELD_VALID(unit, reg, reg_field)) {
        return BCM_E_UNAVAIL;
    }

    BCM_IF_ERROR_RETURN(
        soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &regval));
    fieldval = soc_reg_field_get(unit, reg, regval, reg_field);

    *hash_enable = fieldval;

    return BCM_E_NONE;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_cleanup
 * Purpose:
 *    DeInit VXLAN software module
 * Parameters:
 *    unit - (IN) Device Number
 * Returns:
 *    BCM_E_XXX
 */
int
bcmi_gh2_vxlan_cleanup(int unit)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_init
 * Purpose:
 *    Initialize the VXLAN software module
 * Parameters:
 *    unit - (IN) Device Number
 * Returns:
 *    BCM_E_XXX
 */
int
bcmi_gh2_vxlan_init(int unit)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_vpn_create
 * Purpose:
 *    Create a VPN instance
 * Parameters:
 *    unit - (IN) Device Number
 *    info - (IN/OUT) VPN configuration info
 * Returns:
 *    BCM_E_XXX
 */
int
bcmi_gh2_vxlan_vpn_create(int unit, bcm_vxlan_vpn_config_t *info)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_vpn_destroy
 * Purpose:
 *    Delete a VPN instance
 * Parameters:
 *    unit  - (IN) Device Number
 *    l2vpn - (IN) VPN instance ID
 * Returns:
 *    BCM_E_XXX
 */
int
bcmi_gh2_vxlan_vpn_destroy(int unit, bcm_vpn_t l2vpn)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

 /* Function:
 *    bcmi_gh2_vxlan_vpn_id_destroy_all
 * Purpose:
 *    Delete all L2-VPN instances
 * Parameters:
 *    unit - (IN) Device Number
 * Returns:
 *    BCM_E_XXXX
 */
int
bcmi_gh2_vxlan_vpn_destroy_all(int unit)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

 /* Function:
 *    bcmi_gh2_vxlan_vpn_get
 * Purpose:
 *    Get L2-VPN instance
 * Parameters:
 *    unit  - (IN) Device Number
 *    l2vpn - (IN) VXLAN VPN
 *    info  - (OUT) VXLAN VPN Config
 * Returns:
 *    BCM_E_XXXX
 */
int
bcmi_gh2_vxlan_vpn_get(
    int unit,
    bcm_vpn_t l2vpn,
    bcm_vxlan_vpn_config_t *info)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_vpn_traverse
 * Purpose:
 *    Get information about a VPN instance
 * Parameters:
 *    unit - (IN) Device Number
 *    cb   - (IN)  User-provided callback
 *    user_data  - (IN/OUT) callback user data
 * Returns:
 *    BCM_E_XXX
 */
int
bcmi_gh2_vxlan_vpn_traverse(
    int unit,
    bcm_vxlan_vpn_traverse_cb cb,
    void *user_data)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_port_traverse
 * Purpose:
 *    Get information about a port instance
 * Parameters:
 *    unit - (IN) Device Number
 *    cb   - (IN) User-provided callback
 *    user_data  - (IN/OUT) callback user data
 * Returns:
 *    BCM_E_XXX
 */
int
bcmi_gh2_vxlan_port_traverse(
    int unit,
    bcm_vxlan_port_traverse_cb cb,
    void *user_data)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_tunnel_terminator_create
 * Purpose:
 *    Set VXLAN Tunnel terminator
 * Parameters:
 *    unit - (IN) Device Number
 *    tnl_info - (IN) Tunnel terminator info data structure
 * Returns:
 *    BCM_E_XXX
 */
int
bcmi_gh2_vxlan_tunnel_terminator_create(
    int unit,
    bcm_tunnel_terminator_t *tnl_info)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_tunnel_terminator_update
 * Purpose:
 *    Update VXLAN Tunnel terminator multicast state
 * Parameters:
 *    unit - (IN) Device Number
 *    tnl_info - (IN) Tunnel terminator info data structure
 * Returns:
 *    BCM_E_XXX
 */
int
bcmi_gh2_vxlan_tunnel_terminator_update(
    int unit,
    bcm_tunnel_terminator_t *tnl_info)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_tunnel_terminate_destroy
 * Purpose:
 *    Destroy VXLAN tunnel_terminate Entry
 * Parameters:
 *    unit - (IN) Device Number
 *    vxlan_tunnel_id - (IN) vxlan_tunnel_id
 * Returns:
 *    BCM_E_XXX
 */
int
bcmi_gh2_vxlan_tunnel_terminator_destroy(
    int unit,
    bcm_gport_t vxlan_tunnel_id)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_tunnel_terminator_destroy_all
 * Purpose:
 *    Destroy all incoming VXLAN tunnel
 * Parameters:
 *    unit - (IN) Device Number
 */
int
bcmi_gh2_vxlan_tunnel_terminator_destroy_all(int unit)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_tunnel_initiator_get
 * Purpose:
 *    Get VXLAN  tunnel_terminate Entry
 * Parameters:
 *    unit - (IN) Device Number
 *    info - (IN/OUT) Tunnel terminator structure
 */
int
bcmi_gh2_vxlan_tunnel_terminator_get(
    int unit,
    bcm_tunnel_terminator_t *tnl_info)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_tunnel_terminator_traverse
 * Purpose:
 *    Traverse VXLAN tunnel terminator entries
 * Parameters:
 *    unit - (IN) Device Number
 *    cb   - (IN) User callback function, called once per entry
 *    user_data - (IN) User supplied cookie used in parameter in callback function
 */
int
bcmi_gh2_vxlan_tunnel_terminator_traverse(
    int unit,
    bcm_tunnel_terminator_traverse_cb cb,
    void *user_data)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_tunnel_initiator_create
 * Purpose:
 *    Set VXLAN Tunnel initiator
 * Parameters:
 *    unit - (IN) Device Number
 * Returns:
 *    BCM_E_XXX
 */
int
bcmi_gh2_vxlan_tunnel_initiator_create(int unit, bcm_tunnel_initiator_t *info)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_tunnel_initiator_destroy
 * Purpose:
 *    Destroy outgoing VXLAN tunnel
 * Parameters:
 *    unit - (IN) Device Number
 *    vxlan_tunnel_id - (IN) Tunnel ID (Gport)
 */
int
bcmi_gh2_vxlan_tunnel_initiator_destroy(
    int unit,
    bcm_gport_t vxlan_tunnel_id)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_tunnel_initiator_destroy_all
 * Purpose:
 *    Destroy all outgoing VXLAN tunnel
 * Parameters:
 *    unit - (IN) Device Number
 */
int
bcmi_gh2_vxlan_tunnel_initiator_destroy_all(int unit)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_tunnel_initiator_get
 * Purpose:
 *    Get an outgong VXLAN tunnel info
 * Parameters:
 *    unit - (IN) Device Number
 *    info - (IN/OUT) Tunnel initiator structure
 */
int
bcmi_gh2_vxlan_tunnel_initiator_get(int unit, bcm_tunnel_initiator_t *info)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_tunnel_initiator_traverse
 * Purpose:
 *    Traverse VXLAN tunnel initiator entries
 * Parameters:
 *    unit - (IN) Device Number
 *    cb   - (IN) User callback function, called once per entry
 *    user_data - (IN) User supplied cookie used in parameter in callback function
 */
int
bcmi_gh2_vxlan_tunnel_initiator_traverse(
    int unit,
    bcm_tunnel_initiator_traverse_cb cb,
    void *user_data)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_port_add
 * Purpose:
 *    Add VXLAN port to a VPN
 * Parameters:
 *    unit - (IN) Device Number
 *    vpn  - (IN) VPN instance ID
 *    vxlan_port - (IN/OUT) vxlan_port information (OUT : vxlan_port_id)
 * Returns:
 *    BCM_E_XXX
 */
int
bcmi_gh2_vxlan_port_add(
    int unit,
    bcm_vpn_t vpn,
    bcm_vxlan_port_t  *vxlan_port)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_port_delete
 * Purpose:
 *    Delete VXLAN port from a VPN
 * Parameters:
 *    unit - (IN) Device Number
 *    vpn  - (IN) VPN instance ID
 *    vxlan_port_id - (IN) vxlan port information
 * Returns:
 *    BCM_E_XXX
 */
int
bcmi_gh2_vxlan_port_delete(
    int unit,
    bcm_vpn_t vpn,
    bcm_gport_t vxlan_port_id)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_port_delete_all
 * Purpose:
 *    Delete all VXLAN ports from a VPN
 * Parameters:
 *    unit - (IN) Device Number
 *    vpn  - (IN) VPN instance ID
 * Returns:
 *    BCM_E_XXX
 */
int
bcmi_gh2_vxlan_port_delete_all(int unit, bcm_vpn_t vpn)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_port_get
 * Purpose:
 *    Get an VXLAN port from a VPN
 * Parameters:
 *    unit - (IN) Device Number
 *    vpn  - (IN) VPN instance ID
 *    vxlan_port - (IN/OUT) VXLAN port information
 */
int
bcmi_gh2_vxlan_port_get(
    int unit,
    bcm_vpn_t vpn,
    bcm_vxlan_port_t *vxlan_port)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmi_gh2_vxlan_port_get_all
 * Purpose:
 *    Get all VXLAN ports from a VPN
 * Parameters:
 *    unit - (IN) Device Number
 *    vpn  - (IN) VPN instance ID
 *    port_max   - (IN) Maximum number of interfaces in array
 *    port_array - (OUT) Array of VXLAN ports
 *    port_count - (OUT) Number of interfaces returned in array
 */
int
bcmi_gh2_vxlan_port_get_all(
    int unit,
    bcm_vpn_t vpn,
    int port_max,
    bcm_vxlan_port_t *port_array,
    int *port_count)
{
    /* TBD */
    return BCM_E_UNAVAIL;
}

#endif /* BCM_GREYHOUND2_SUPPORT */

