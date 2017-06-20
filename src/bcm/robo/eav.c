/*
 * $Id: eav.c,v 1.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Ethernet AV BCM APIs
 */

#include <shared/bsl.h>

#include <bcm/error.h>
#include <bcm/eav.h>
#include <soc/drv.h>
#include <soc/debug.h>

/*
* Function:
*      bcm_eav_init
* Description:
*      Initialize the Residential Ethernet module and enable
*      the Ethernet AV (EAV) support.
* Parameters:
*      unit - device unit number.
* Returns:
*      BCM_E_XXX
*      
* Notes:
*      1. This function will enable the global EAV functionality
*      2. Decide the way to report egress timestamp info to CPU
*         Either loopback reporting packets
*         or CPU directly read register later.
*/
int 
bcm_robo_eav_init(int unit)
{
    int rv = BCM_E_NONE;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    
    /* 1. MMU settings provided by ASIC*/
    rv = DRV_EAV_CONTROL_SET(unit, 
        DRV_EAV_CONTROL_MMU_INIT, 1);

    /* 2. Enable time stamped to IMP port */
    rv = DRV_EAV_CONTROL_SET(unit, 
        DRV_EAV_CONTROL_TIME_STAMP_TO_IMP, 1);

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "EAV Error: failure in bcm_robo_eav_init()\n")));
        return rv;
    }
    return rv;
}

/*
* Function:
*      bcm_eav_port_enable_get
* Description:
*      Get enable status of per port Ethernet AV functionality
* Parameters:
*      unit - device unit number.
*      port - port number
*      enable - (OUT) TRUE, port is enabled for Ethernet AV
*                     FALSE, port is disabled for Ethernet AV
* Returns:
*      BCM_E_NONE
*      BCM_E_XXX
* Notes:
*/

int 
bcm_robo_eav_port_enable_get(int unit, bcm_port_t port, int *enable)
{
    int rv = BCM_E_NONE;
    uint32  temp;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (!SOC_PORT_VALID(unit, port)) { 
        return BCM_E_PORT; 
    }
    
    rv = DRV_EAV_ENABLE_GET(unit, port, &temp);

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "EAV Error: failure in bcm_robo_eav_port_enable_get()\n")));
        return rv;
    }
    if (temp) {
        *enable = TRUE;
    } else {
        *enable = FALSE;
    }

    return rv;
    
}

/*
* Function:
*      bcm_eav_port_enable_set
* Description:
*      Enable or disable per port Ethernet AV functionality
* Parameters:
*      unit - device unit number.
*      port - port number
*      enable - TRUE, port is enabled for Ethernet AV
*               FALSE, port is disabled for Ethernet AV
* Returns:
*      BCM_E_NONE
*      BCM_E_XXX
*
* Notes:
*      Need to disable the per port flow control
*/

int 
bcm_robo_eav_port_enable_set(int unit, bcm_port_t port, int enable)
{
    int rv = BCM_E_NONE;
    uint32  temp;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (!SOC_PORT_VALID(unit, port)) { 
        return BCM_E_PORT; 
    }
    
    if (enable) {
        temp = 1;
    } else {
        temp = 0;
    }
    
    rv = DRV_EAV_ENABLE_SET(unit, port, temp);

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "EAV Error: failure in bcm_robo_eav_port_enable_set()\n")));
        return rv;
    }

    return rv;
}

/*
* Function:
*      bcm_eav_link_status_get
* Description:
*      Get link status of per port Ethernet AV functionality
* Parameters:
*      unit - device unit number.
*      port - port number
*      link - (OUT) TRUE, Ethernet AV led is light on
*                     FALSE, Ethernet AV led is light off
* Returns:
*      BCM_E_NONE
*      BCM_E_XXX
* Notes:
*/

int 
bcm_robo_eav_link_status_get(int unit, bcm_port_t port, int *link)
{
    int rv = BCM_E_NONE;
    uint32  temp;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (!SOC_PORT_VALID(unit, port)) { 
        return BCM_E_PORT; 
    }
    
    rv = DRV_EAV_LINK_STATUS_GET(unit, port, &temp);

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "EAV Error: failure in bcm_robo_eav_link_status_get()\n")));
        return rv;
    }
    if (temp) {
        *link = TRUE;
    } else {
        *link = FALSE;
    }

    return rv;
    
}

/*
* Function:
*      bcm_eav_link_status_set
* Description:
*      Set the EAV link status of  per port Ethernet AV functionality
* Parameters:
*      unit - device unit number.
*      port - port number
*      link - TRUE, Ethernet AV led is light on
*               FALSE, Ethernet AV led is light off
* Returns:
*      BCM_E_NONE
*      BCM_E_XXX
*
* Notes:
*/

int 
bcm_robo_eav_link_status_set(int unit, bcm_port_t port, int link)
{
    int rv = BCM_E_NONE;
    uint32  temp;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (!SOC_PORT_VALID(unit, port)) { 
        return BCM_E_PORT; 
    }
    
    if (link) {
        temp = 1;
    } else {
        temp = 0;
    }
    
    rv = DRV_EAV_LINK_STATUS_SET(unit, port, temp);

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "EAV Error: failure in bcm_robo_eav_link_status_set()\n")));
        return rv;
    }

    return rv;
}

/*
* Function:
*      bcm_eav_control_get
* Description:
*      Get the configuration for specific type
* Parameters:
*      unit - device unit number.
*      type - configuration type
*      arg1 - (OUT) the pointer buffer to store the returned configuration
*      arg2 - (OUT) the pointer buffer to store the returned configuration
* Returns:
*      BCM_E_NONE
*      BCM_E_XXX
* Notes:
*/

int 
bcm_robo_eav_control_get(int unit, bcm_eav_control_t type, 
        uint32 *arg1, uint32 *arg2)
{
    int rv = BCM_E_NONE;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    switch (type) {
        case bcmEAVControlMaxFrameSize:
            rv = DRV_EAV_CONTROL_GET
                (unit, DRV_EAV_CONTROL_MAX_AV_SIZE, arg1);
            break;
        case bcmEAVControlTimeBase:
            rv = DRV_EAV_TIME_SYNC_GET
                (unit, DRV_EAV_TIME_SYNC_TIME_BASE, arg1, arg2);
            break;
        case bcmEAVControlTimeAdjust:
            rv = DRV_EAV_TIME_SYNC_GET
                (unit, DRV_EAV_TIME_SYNC_TIME_ADJUST, arg1, arg2);
            break;
        case bcmEAVControlTickCounter:
            rv = DRV_EAV_TIME_SYNC_GET
                (unit, DRV_EAV_TIME_SYNC_TICK_COUNTER, arg1, arg2);
            break;
        case bcmEAVControlSlotNumber:
            rv = DRV_EAV_TIME_SYNC_GET
                (unit, DRV_EAV_TIME_SYNC_SLOT_NUMBER, arg1, arg2);
            break;
        case bcmEAVControlMacroSlotTime:
            rv = DRV_EAV_TIME_SYNC_GET
                (unit, DRV_EAV_TIME_SYNC_MACRO_SLOT_PERIOD, arg1, arg2);
            break;
         case bcmEAVControlSlotAdjust:
            rv = DRV_EAV_TIME_SYNC_GET
                (unit, DRV_EAV_TIME_SYNC_SLOT_ADJUST, arg1, arg2);
            break;
        case bcmEAVControlStreamClassAPCP:
            rv = DRV_EAV_CONTROL_GET
                (unit, DRV_EAV_CONTROL_STREAM_CLASSA_PCP, arg1);
            break;
        case bcmEAVControlStreamClassBPCP:
            rv = DRV_EAV_CONTROL_GET
                (unit, DRV_EAV_CONTROL_STREAM_CLASSB_PCP, arg1);
            break;
        default:
            return BCM_E_PARAM;
    }
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "EAV Error: failure in bcm_eav_control_get()\n")));
        return rv;
    }

    return rv;
}

/*
* Function:
*      bcm_eav_control_set
* Description:
*      Set the configuration for specific type
* Parameters:
*      unit - device unit number.
*      type - configuration type
*      arg1 - the configuration data to set 
*      arg2 - the configuration data to set 
* Returns:
*      BCM_E_NONE
*      BCM_E_XXX
* Notes:
*/

int 
bcm_robo_eav_control_set(int unit, bcm_eav_control_t type, 
       uint32 arg1, uint32 arg2)
{
    int rv = BCM_E_NONE;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    
    switch (type) {
        case bcmEAVControlMaxFrameSize:
            rv = DRV_EAV_CONTROL_SET 
                (unit, DRV_EAV_CONTROL_MAX_AV_SIZE, arg1);
            break;
        case bcmEAVControlTimeBase:
            rv =  DRV_EAV_TIME_SYNC_SET
                (unit, DRV_EAV_TIME_SYNC_TIME_BASE, arg1, arg2);
            break;
        case bcmEAVControlTimeAdjust:
            rv = DRV_EAV_TIME_SYNC_SET 
                (unit, DRV_EAV_TIME_SYNC_TIME_ADJUST, arg1, arg2);
            break;
        case bcmEAVControlTickCounter:
            rv = DRV_EAV_TIME_SYNC_SET 
                (unit, DRV_EAV_TIME_SYNC_TICK_COUNTER, arg1, arg2);
            break;
        case bcmEAVControlSlotNumber:
            rv = DRV_EAV_TIME_SYNC_SET 
                (unit, DRV_EAV_TIME_SYNC_SLOT_NUMBER, arg1, arg2);
            break;
        case bcmEAVControlMacroSlotTime:
            rv = DRV_EAV_TIME_SYNC_SET 
                (unit, DRV_EAV_TIME_SYNC_MACRO_SLOT_PERIOD, arg1, arg2);
            break;
         case bcmEAVControlSlotAdjust:
            rv = DRV_EAV_TIME_SYNC_SET 
                (unit, DRV_EAV_TIME_SYNC_SLOT_ADJUST, arg1, arg2);
            break;
         case bcmEAVControlStreamClassAPCP:
            rv = DRV_EAV_CONTROL_SET
                (unit, DRV_EAV_CONTROL_STREAM_CLASSA_PCP, arg1);
            break;
         case bcmEAVControlStreamClassBPCP:
            rv = DRV_EAV_CONTROL_SET
                (unit, DRV_EAV_CONTROL_STREAM_CLASSB_PCP, arg1);
            break;
        default:
            return BCM_E_PARAM;
    }
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "EAV Error: failure in bcm_eav_control_set()\n")));
        return rv;
    }

    return rv;
}

/*
* Function:
*      bcm_eav_egress_timestamp_get
* Description:
*      Get the per port egress timestamp value
* Parameters:
*      unit - device unit number
*      port - port number
*      timestamp - (OUT) the pointer buffer to store the returned timestamp  
* Returns:
*      BCM_E_NONE
*      BCM_E_XXX
* Notes:
*/

int 
bcm_robo_eav_timestamp_get(int unit, bcm_port_t port, uint32 *timestamp)
{
    int rv = BCM_E_NONE;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (!SOC_PORT_VALID(unit, port)) { 
        return BCM_E_PORT; 
    }
    
     rv = DRV_EAV_EGRESS_TIMESTAMP_GET
        (unit, port, timestamp);

     if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "EAV Error: failure in bcm_eav_timestamp_get()\n")));
        return rv;
    }

     return rv;
}


/*
* Function:
*      bcm_eav_timesync_mac_get
* Description:
*      Get the Mac address of Time Sync protocol
* Parameters:
*      unit - device unit number
*      eav_mac - the pointer buffer to restorm the mac addrss  
* Returns:
*      BCM_E_NONE
*      BCM_E_XXX
* Notes:
*/

int 
bcm_robo_eav_timesync_mac_get(int unit, bcm_mac_t eav_mac)
{
    uint16  ethertype;
    int rv = BCM_E_NONE;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    rv = DRV_EAV_TIME_SYNC_MAC_GET
        (unit, eav_mac, &ethertype);
    
    return rv;
}

/*
* Function:
*      bcm_eav_timesync_mac_set
* Description:
*      Set the Mac address of Time Sync protocol
* Parameters:
*      unit - device unit number
*      eav_mac - the pointer buffer to restorm the mac addrss  
* Returns:
*      BCM_E_NONE
*      BCM_E_XXX
* Notes:
*/

int 
bcm_robo_eav_timesync_mac_set(int unit, bcm_mac_t eav_mac)
{

    uint16  ethertype;
    int rv = BCM_E_NONE;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    ethertype = 0; /* ignore ethtype value */
    rv = DRV_EAV_TIME_SYNC_MAC_SET
        (unit, eav_mac, ethertype);
    
    return rv;
}

/*
 * Function:
 *      bcm_eav_srp_mac_ethertype_set
 * Description:
 *      Get the Mac address and Ethertype used to trap SRP protocol packets
 * Parameters:
 *      unit - device unit number
 *      mac  - the mac addrss   
 *      ethertype - the EtherType
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_eav_srp_mac_ethertype_set(int unit, bcm_mac_t mac, bcm_port_ethertype_t ethertype)
{
    uint32 reg_val;
    uint64 reg_val64, mac_field;
    uint32 temp;

    if (!soc_feature(unit, soc_feature_eav_support)) {
        return BCM_E_UNAVAIL;
    }

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    /*
     * For srp, the mac should be set in Multi-address 1 register
     */

    /* 1. Set MAC and Ethertype value */
    SAL_MAC_ADDR_TO_UINT64(mac, mac_field);

    COMPILER_64_ZERO(reg_val64);
    BCM_IF_ERROR_RETURN(soc_MULTIPORT_ADDR1r_field_set
        (unit, (void *)&reg_val64, MPORT_ADDRf, (void *)&mac_field));
    
    temp = ethertype;
    BCM_IF_ERROR_RETURN(soc_MULTIPORT_ADDR1r_field_set
        (unit, (void *)&reg_val64, MPORT_E_TYPEf, &temp));

    BCM_IF_ERROR_RETURN(REG_WRITE_MULTIPORT_ADDR1r(unit, (void *)&reg_val64));

    /* 2. Set Forward map to CPU only */
    temp  = SOC_PBMP_WORD_GET(PBMP_CMIC(unit), 0);
    reg_val = 0;
    BCM_IF_ERROR_RETURN(soc_MPORTVEC1r_field_set
        (unit, &reg_val, PORT_VCTRf, &temp));
    BCM_IF_ERROR_RETURN(REG_WRITE_MPORTVEC1r(unit, &reg_val));

    /* 3. Enable Multi-address o */
    BCM_IF_ERROR_RETURN(REG_READ_MULTI_PORT_CTLr(unit, &reg_val));

    /* Set the match condition are MAC/Ethertype */
    temp = DRV_MULTIPORT_CTRL_MATCH_ETYPE_ADDR;
    BCM_IF_ERROR_RETURN(soc_MULTI_PORT_CTLr_field_set
        (unit, &reg_val, MPORT_CTRL1f, &temp));

    BCM_IF_ERROR_RETURN(REG_WRITE_MULTI_PORT_CTLr(unit, &reg_val));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_eav_srp_mac_ethertype_get
 * Description:
 *      Get the Mac address and Ethertype used to trap SRP protocol packets
 * Parameters:
 *      unit - device unit number
 *      mac  - (OUT)the mac addrss   
 *      ethertype - (OUT)the EtherType
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */                               
int 
bcm_robo_eav_srp_mac_ethertype_get(int unit, bcm_mac_t mac, bcm_port_ethertype_t *ethertype)
{
    uint32 reg_val;
    uint64 reg_val64, mac_field;
    uint32 temp; 

    if (!soc_feature(unit, soc_feature_eav_support)) {
        return BCM_E_UNAVAIL;
    }

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    
    BCM_IF_ERROR_RETURN(REG_READ_MULTI_PORT_CTLr(unit, &reg_val));

    /* Get the Multi-address control value */
    BCM_IF_ERROR_RETURN(soc_MULTI_PORT_CTLr_field_get
        (unit, &reg_val, MPORT_CTRL1f, &temp));
    if (temp == 0) { 
        return SOC_E_DISABLED;
    }

    /* Get the MAC and Ethertype value */
    COMPILER_64_ZERO(reg_val64);
    BCM_IF_ERROR_RETURN(REG_READ_MULTIPORT_ADDR1r(unit, (void *)&reg_val64));

    BCM_IF_ERROR_RETURN(soc_MULTIPORT_ADDR1r_field_get
        (unit, (void *)&reg_val64, MPORT_ADDRf, (void *)&mac_field));
    SAL_MAC_ADDR_FROM_UINT64(mac, mac_field);

    BCM_IF_ERROR_RETURN(soc_MULTIPORT_ADDR1r_field_get
        (unit, (void *)&reg_val64, MPORT_E_TYPEf, &temp));   
 
    *ethertype = temp;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_eav_pcp_mapping_set
 * Description:
 *      Set the PCP(priority) value mapping for each EAV class packets
 * Parameters:
 *      unit - device unit number
 *      type - Class A or Class B stream
 *      pcp  - Priority for the Class   
 *      rempapped_pcp - For NonEAV traffic with PCP=ClassA_PCP or ClassB_PCP
 *      exiting through an egress port configured in EAV mode must be remapped
 *      to another pcp.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_eav_pcp_mapping_set(
    int unit, 
    bcm_eav_stream_class_t type, 
    int pcp, 
    int remapped_pcp)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_eav_pcp_mapping_get
 * Description:
 *      Set the PCP(priority) value mapping for each EAV class packets
 * Parameters:
 *      unit - device unit number
 *      type - Class A or Class B stream
 *      pcp  - Priority for the Class   
 *      rempapped_pcp - For NonEAV traffic with PCP=ClassA_PCP or ClassB_PCP
 *      exiting through an egress port configured in EAV mode must be remapped
 *      to another pcp.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_eav_pcp_mapping_get(
    int unit, 
    bcm_eav_stream_class_t type, 
    int *pcp, 
    int *remapped_pcp)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_eav_bandwidth_set
 * Description:
 *      Set the reserved bandwidth for Class A or B stream traffic
 * Parameters:
 *      unit - device unit number
 *      port - port number   
 *      type - Class A or Class B stream
 *      bytes_sec - bytes per second.
 *      bytes_burst - maximum burst size in bytes.  
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_eav_bandwidth_set(
    int unit, 
    bcm_port_t port, 
    bcm_eav_stream_class_t type, 
    uint32 bytes_sec,
    uint32 bytes_burst)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_eav_bandwidth_get
 * Description:
 *      Get the reserved bandwidth for Class A or B stream traffic
 * Parameters:
 *      unit - device unit number
 *      port - port number   
 *      type - Class A or Class B stream
 *      bytes_sec - bytes per second.
 *      bytes_burst - maximum burst size in bytes.  
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_robo_eav_bandwidth_get(
    int unit, 
    bcm_port_t port, 
    bcm_eav_stream_class_t type, 
    uint32 *bytes_sec,
    uint32 *bytes_burst)
{
    return BCM_E_UNAVAIL;
}



