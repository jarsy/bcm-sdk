/*
 * $Id: field.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 *  Field driver service.
 *  Purpose: Handle the chip variant design for Field Processor
 * 
 */

#include <shared/bsl.h>

#include <soc/error.h>
#include <soc/types.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/drv_if.h>

#define CFP_53115_L3_FRAME_IPV4 0x0
#define CFP_53115_L3_FRAME_IPV6 0x1
#define CFP_53115_L3_FRAME_NONIP 0x3
#define CFP_53115_L3_FRAME_CHAIN 0x2

#define CFP_53115_L3_FRAME(slice_id) \
    (((slice_id) & 0xc) >> 2)

#define CFP_53115_SLICE_ID(slice_id) \
    ((slice_id) & 0x3)


#define CFP_53115_SLICE_ID_WITH_CHAIN       3

#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
static int 
_drv_vulcan_cfp_qual_value_set(int unit, drv_field_qualify_t qual, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = SOC_E_NONE;
    uint32      fld_index = 0;
    uint32    dtag_mode = 0; /* dtag none */
    drv_cfp_entry_t  *drv_entry = (drv_cfp_entry_t  *)entry;

    
   SOC_IF_ERROR_RETURN(
        DRV_PORT_GET(unit, 0, DRV_PORT_PROP_DTAG_MODE, &dtag_mode));
    switch (qual) {
        case drvFieldQualifyInPorts:
            fld_index = DRV_CFP_FIELD_IN_PBMP;
            break;
        case drvFieldQualifyOuterVlanId:
            if (dtag_mode == 0) {
                fld_index = DRV_CFP_FIELD_USR_VID;
            } else {
                fld_index = DRV_CFP_FIELD_SP_VID;
            }
            break;
        case drvFieldQualifyOuterVlanPri:
            if (dtag_mode == 0) {
                fld_index = DRV_CFP_FIELD_USR_PRI;
            } else {
                fld_index = DRV_CFP_FIELD_SP_PRI;
            }
            break;
        case drvFieldQualifyOuterVlanCfi:
            if (dtag_mode == 0) {
                fld_index = DRV_CFP_FIELD_USR_CFI;
            } else {
                fld_index = DRV_CFP_FIELD_SP_CFI;
            }
            break;
        case drvFieldQualifyInnerVlanId:
            fld_index = DRV_CFP_FIELD_USR_VID;
            break;
        case drvFieldQualifyInnerVlanPri:
            fld_index = DRV_CFP_FIELD_USR_PRI;
            break;
        case drvFieldQualifyInnerVlanCfi:
            fld_index = DRV_CFP_FIELD_USR_CFI;
            break;
        case drvFieldQualifyL2Format:
            fld_index = DRV_CFP_FIELD_L2_FRM_FORMAT;
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                /* Check if bit 2 is set for PPPoE Session format */
                if (*p_mask & 0x4) {
                    fld_index = DRV_CFP_FIELD_PPPOE_SESSION_FRM;
                    *p_mask = (*p_mask >> 2);
                    *p_data = (*p_data >> 2);
                }
            }
 #endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
            break;
        case drvFieldQualifyIpType:
            fld_index = DRV_CFP_FIELD_L3_FRM_FORMAT;
            break;
        case drvFieldQualifyIpProtocol:
            fld_index = DRV_CFP_FIELD_IP_PROTO;
            break;
        case drvFieldQualifyDSCP:
            fld_index = DRV_CFP_FIELD_IP_TOS;
            break;
        case drvFieldQualifyTtl:
            fld_index = DRV_CFP_FIELD_IP_TTL;
            break;
        case drvFieldQualifyEtherType:
            fld_index = DRV_CFP_FIELD_ETYPE;
            break;

        /* Add qualify which can be support by UDFs */
        case drvFieldQualifySrcMac:
            fld_index = DRV_CFP_FIELD_MAC_SA;
            break;
        case drvFieldQualifyDstMac:
            fld_index = DRV_CFP_FIELD_MAC_DA;
            break;
        case drvFieldQualifySrcIp:
            fld_index = DRV_CFP_FIELD_IP_SA;
            break;
        case drvFieldQualifyDstIp:
            fld_index = DRV_CFP_FIELD_IP_DA;
            break;
        case drvFieldQualifySrcIp6:
            fld_index = DRV_CFP_FIELD_IP6_SA;
            break;
        case drvFieldQualifyDstIp6:
            fld_index = DRV_CFP_FIELD_IP6_DA;
            break;
        case drvFieldQualifyL4SrcPort:
            fld_index = DRV_CFP_FIELD_L4SRC;
            break;
        case drvFieldQualifyL4DstPort:
            fld_index = DRV_CFP_FIELD_L4DST;
            break;
        case drvFieldQualifySnap:
            fld_index = DRV_CFP_FIELD_SNAP_HEADER;
            break;
        case drvFieldQualifyLlc:
            fld_index = DRV_CFP_FIELD_LLC_HEADER;
            break;
        case drvFieldQualifyTcpControl:
            fld_index = DRV_CFP_FIELD_TCP_FLAG;
            break;
        case drvFieldQualifyIp6FlowLabel:
            fld_index = DRV_CFP_FIELD_IP6_FLOW_ID;
            break;
        case drvFieldQualifyIpAuth:
            fld_index = DRV_CFP_FIELD_IP_AUTH;
            break;
        case drvFieldQualifyClassId:
            fld_index = DRV_CFP_FIELD_CHAIN_ID;
            break;

        default:
            rv = SOC_E_INTERNAL;
            return rv;
    }


    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "DRV_FP: %s fld_index = 0x%x\n"),
                 FUNCTION_NAME(), fld_index));
    SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM, fld_index, 
            drv_entry, p_data));
    
    SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM_MASK, fld_index, 
            drv_entry, p_mask)); 

  LOG_VERBOSE(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP: %s data= 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n"),
               FUNCTION_NAME(),
               drv_entry->tcam_data[0],drv_entry->tcam_data[1], 
               drv_entry->tcam_data[2], drv_entry->tcam_data[3], 
               drv_entry->tcam_data[4], drv_entry->tcam_data[5]));
  LOG_VERBOSE(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP: %s mask= 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n"),
               FUNCTION_NAME(),
               drv_entry->tcam_mask[0], drv_entry->tcam_mask[1], 
               drv_entry->tcam_mask[2], drv_entry->tcam_mask[3], 
               drv_entry->tcam_mask[4], drv_entry->tcam_mask[5]));

    return SOC_E_NONE;
}

static int 
_drv_vulcan_cfp_qual_value_get(int unit, drv_field_qualify_t qual, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = SOC_E_NONE;
    uint32      fld_index = 0;
    uint32    dtag_mode = 0; /* dtag none */
    drv_cfp_entry_t  *drv_entry = (drv_cfp_entry_t  *)entry;
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    uint32      tmp_mask, tmp_data;
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
    
   SOC_IF_ERROR_RETURN(
        DRV_PORT_GET(unit, 0, DRV_PORT_PROP_DTAG_MODE, &dtag_mode));
    switch (qual) {
        case drvFieldQualifyInPorts:
            fld_index = DRV_CFP_FIELD_IN_PBMP;
            break;
        case drvFieldQualifyOuterVlanId:
            if (dtag_mode == 0) {
                fld_index = DRV_CFP_FIELD_USR_VID;
            } else {
                fld_index = DRV_CFP_FIELD_SP_VID;
            }
            break;
        case drvFieldQualifyOuterVlanPri:
            if (dtag_mode == 0) {
                fld_index = DRV_CFP_FIELD_USR_PRI;
            } else {
                fld_index = DRV_CFP_FIELD_SP_PRI;
            }
            break;
        case drvFieldQualifyOuterVlanCfi:
            if (dtag_mode == 0) {
                fld_index = DRV_CFP_FIELD_USR_CFI;
            } else {
                fld_index = DRV_CFP_FIELD_SP_CFI;
            }
            break;
        case drvFieldQualifyInnerVlanId:
            fld_index = DRV_CFP_FIELD_USR_VID;
            break;
        case drvFieldQualifyInnerVlanPri:
            fld_index = DRV_CFP_FIELD_USR_PRI;
            break;
        case drvFieldQualifyInnerVlanCfi:
            fld_index = DRV_CFP_FIELD_USR_CFI;
            break;
        case drvFieldQualifyL2Format:
            fld_index = DRV_CFP_FIELD_L2_FRM_FORMAT;
            break;
        case drvFieldQualifyIpType:
            fld_index = DRV_CFP_FIELD_L3_FRM_FORMAT;
            break;
        case drvFieldQualifyIpProtocol:
            fld_index = DRV_CFP_FIELD_IP_PROTO;
            break;
        case drvFieldQualifyDSCP:
            fld_index = DRV_CFP_FIELD_IP_TOS;
            break;
        case drvFieldQualifyTtl:
            fld_index = DRV_CFP_FIELD_IP_TTL;
            break;
        case drvFieldQualifyEtherType:
            fld_index = DRV_CFP_FIELD_ETYPE;
            break;

        /* Add qualify which can be support by UDFs */
        case drvFieldQualifySrcMac:
            fld_index = DRV_CFP_FIELD_MAC_SA;
            break;
        case drvFieldQualifyDstMac:
            fld_index = DRV_CFP_FIELD_MAC_DA;
            break;
        case drvFieldQualifySrcIp:
            fld_index = DRV_CFP_FIELD_IP_SA;
            break;
        case drvFieldQualifyDstIp:
            fld_index = DRV_CFP_FIELD_IP_DA;
            break;
        case drvFieldQualifySrcIp6:
            fld_index = DRV_CFP_FIELD_IP6_SA;
            break;
        case drvFieldQualifyDstIp6:
            fld_index = DRV_CFP_FIELD_IP6_DA;
            break;
        case drvFieldQualifyL4SrcPort:
            fld_index = DRV_CFP_FIELD_L4SRC;
            break;
        case drvFieldQualifyL4DstPort:
            fld_index = DRV_CFP_FIELD_L4DST;
            break;
        case drvFieldQualifySnap:
            fld_index = DRV_CFP_FIELD_SNAP_HEADER;
            break;
        case drvFieldQualifyLlc:
            fld_index = DRV_CFP_FIELD_LLC_HEADER;
            break;
        case drvFieldQualifyTcpControl:
            fld_index = DRV_CFP_FIELD_TCP_FLAG;
            break;
        case drvFieldQualifyIp6FlowLabel:
            fld_index = DRV_CFP_FIELD_IP6_FLOW_ID;
            break;
        case drvFieldQualifyIpAuth:
            fld_index = DRV_CFP_FIELD_IP_AUTH;
            break;
        case drvFieldQualifyClassId:
            fld_index = DRV_CFP_FIELD_CHAIN_ID;
            break;

        default:
            rv = SOC_E_INTERNAL;
            return rv;
    }


    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "DRV_FP: %s fld_index = 0x%x\n"),
                 FUNCTION_NAME(), fld_index));
    SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_GET
        (unit, DRV_CFP_RAM_TCAM, fld_index, 
            drv_entry, p_data));
    
    SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_GET
        (unit, DRV_CFP_RAM_TCAM_MASK, fld_index, 
            drv_entry, p_mask)); 
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    /* Check the PPPoE session format */
    if ((SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) &&
            (qual == drvFieldQualifyL2Format)) {
        tmp_mask = 0;
        tmp_data = 0;
        SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_PPPOE_SESSION_FRM, 
                drv_entry, &tmp_data));
        SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_GET
            (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_PPPOE_SESSION_FRM, 
                drv_entry, &tmp_mask));
        *p_data |= (tmp_data << 2);
        *p_mask |= (tmp_mask << 2);
        
    }
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHGTER3_SUPPORT */

    return SOC_E_NONE;
}
#endif /* BCM_VULCAN_SUPPORT || STARFIGHTER || POLAR || NORTHSTAR || NS+ || SF3 */

#ifdef BCM_BLACKBIRD2_SUPPORT
static int 
_drv_blackbird2_cfp_qual_value_set(int unit, drv_field_qualify_t qual, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = SOC_E_NONE;
    uint32      fld_index = 0;
    drv_cfp_entry_t  *drv_entry = (drv_cfp_entry_t  *)entry;

    switch (qual) {
        case drvFieldQualifyL4SrcPort:
            fld_index = DRV_CFP_FIELD_L4SRC;
            break;
        case drvFieldQualifyL4DstPort:
            fld_index = DRV_CFP_FIELD_L4DST;
            break;
        case drvFieldQualifyL4Ports:
            fld_index = DRV_CFP_FIELD_L4PORTS;
            break;
        default:
            rv = SOC_E_INTERNAL;
            return rv;
    }


    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "DRV_FP: %s fld_index = 0x%x\n"),
                 FUNCTION_NAME(), fld_index));
    SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM, fld_index, 
            drv_entry, p_data));

  LOG_VERBOSE(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP: %s data= 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n"),
               FUNCTION_NAME(),
               drv_entry->tcam_data[0],drv_entry->tcam_data[1], 
               drv_entry->tcam_data[2], drv_entry->tcam_data[3], 
               drv_entry->tcam_data[4], drv_entry->tcam_data[5]));

    return SOC_E_NONE;
}

static int 
_drv_blackbird2_cfp_qual_value_get(int unit, drv_field_qualify_t qual, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = SOC_E_NONE;
    uint32      fld_index = 0;
    drv_cfp_entry_t  *drv_entry = (drv_cfp_entry_t  *)entry;

    switch (qual) {
        case drvFieldQualifyL4SrcPort:
            fld_index = DRV_CFP_FIELD_L4SRC;
            break;
        case drvFieldQualifyL4DstPort:
            fld_index = DRV_CFP_FIELD_L4DST;
            break;
        case drvFieldQualifyL4Ports:
            fld_index = DRV_CFP_FIELD_L4PORTS;
            break;
        default:
            rv = SOC_E_INTERNAL;
            return rv;
    }


    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "DRV_FP: %s fld_index = 0x%x\n"),
                 FUNCTION_NAME(), fld_index));
    SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_GET
        (unit, DRV_CFP_RAM_TCAM, fld_index, 
            drv_entry, p_data));

    return SOC_E_NONE;
}
#endif /* BCM_BLACKBIRD2_SUPPORT */



int 
_drv_gex_cfp_qual_value_set(int unit, drv_field_qualify_t qual, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = SOC_E_NONE;

    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        return _drv_vulcan_cfp_qual_value_set(unit, qual, entry, p_data, p_mask);
#endif
    } else if (SOC_IS_BLACKBIRD2(unit)) {
#ifdef BCM_BLACKBIRD2_SUPPORT
        return _drv_blackbird2_cfp_qual_value_set(unit, qual, entry, p_data, p_mask);
#endif
    }else {
        rv =  SOC_E_UNAVAIL;
    }

    return rv;
}

int 
_drv_gex_cfp_qual_value_get(int unit, drv_field_qualify_t qual, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = SOC_E_NONE;

    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        return _drv_vulcan_cfp_qual_value_get(unit, qual, entry, p_data, p_mask);
#endif
    } else if (SOC_IS_BLACKBIRD2(unit)) {
#ifdef BCM_BLACKBIRD2_SUPPORT    
        return _drv_blackbird2_cfp_qual_value_get(unit, qual, entry, p_data, p_mask);
#endif
    }else {
        rv =  SOC_E_UNAVAIL;
    }

    return rv;
}


#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
static int
_drv_vulcan_cfp_udf_value_set(int unit, uint32 udf_idx, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    uint32      fld_index = 0;
    uint32      temp;
    int rv;
    drv_cfp_entry_t  *drv_entry = (drv_cfp_entry_t *)entry;
    
    switch (udf_idx) {
        case 0:
        case 9:
        case 18:
            fld_index = DRV_CFP_FIELD_UDFA0;
            break;
        case 1:
        case 10:
        case 19:
            fld_index = DRV_CFP_FIELD_UDFA1;
            break;
        case 2:
        case 11:
        case 20:
            fld_index = DRV_CFP_FIELD_UDFA2;
            break;
        case 3:
        case 12:
        case 21:
            fld_index = DRV_CFP_FIELD_UDFA3;
            break;
        case 4:
        case 13:
        case 22:
            fld_index = DRV_CFP_FIELD_UDFA4;
            break;
        case 5:
        case 14:
        case 23:
            fld_index = DRV_CFP_FIELD_UDFA5;
            break;
        case 6:
        case 15:
        case 24:
            fld_index = DRV_CFP_FIELD_UDFA6;
            break;
        case 7:
        case 16:
        case 25:
            fld_index = DRV_CFP_FIELD_UDFA7;
            break;
        case 8:
        case 17:
        case 26:
            fld_index = DRV_CFP_FIELD_UDFA8;
            break;
            
        case 27:
        case 36:
        case 45:
            fld_index = DRV_CFP_FIELD_UDFB0;
            break;
        case 28:
        case 37:
        case 46:
            fld_index = DRV_CFP_FIELD_UDFB1;
            break;
        case 29:
        case 38:
        case 47:
            fld_index = DRV_CFP_FIELD_UDFB2;
            break;
        case 30:
        case 39:
        case 48:
            fld_index = DRV_CFP_FIELD_UDFB3;
            break;
        case 31:
        case 40:
        case 49:
            fld_index = DRV_CFP_FIELD_UDFB4;
            break;
        case 32:
        case 41:
        case 50:
            fld_index = DRV_CFP_FIELD_UDFB5;
            break;
        case 33:
        case 42:
        case 51:
            fld_index = DRV_CFP_FIELD_UDFB6;
            break;
        case 34:
        case 43:
        case 52:
            fld_index = DRV_CFP_FIELD_UDFB7;
            break;
        case 35:
        case 44:
        case 53:
            fld_index = DRV_CFP_FIELD_UDFB8;
            break;
            
        case 54:
        case 63:
        case 72:
            fld_index = DRV_CFP_FIELD_UDFC0;
            break;
        case 55:
        case 64:
        case 73:
            fld_index = DRV_CFP_FIELD_UDFC1;
            break;
        case 56:
        case 65:
        case 74:
            fld_index = DRV_CFP_FIELD_UDFC2;
            break;
        case 57:
        case 66:
        case 75:
            fld_index = DRV_CFP_FIELD_UDFC3;
            break;
        case 58:
        case 67:
        case 76:
            fld_index = DRV_CFP_FIELD_UDFC4;
            break;
        case 59:
        case 68:
        case 77:
            fld_index = DRV_CFP_FIELD_UDFC5;
            break;
        case 60:
        case 69:
        case 78:
            fld_index = DRV_CFP_FIELD_UDFC6;
            break;
        case 61:
        case 70:
        case 79:
            fld_index = DRV_CFP_FIELD_UDFC7;
            break;
        case 62:
        case 71:
        case 80:
            fld_index = DRV_CFP_FIELD_UDFC8;
            break;
            
        case 81:
            fld_index = DRV_CFP_FIELD_UDFD0;
            break;
        case 82:
            fld_index = DRV_CFP_FIELD_UDFD1;
            break;
        case 83:
            fld_index = DRV_CFP_FIELD_UDFD2;
            break;
        case 84:
            fld_index = DRV_CFP_FIELD_UDFD3;
            break;
        case 85:
            fld_index = DRV_CFP_FIELD_UDFD4;
            break;
        case 86:
            fld_index = DRV_CFP_FIELD_UDFD5;
            break;
        case 87:
            fld_index = DRV_CFP_FIELD_UDFD6;
            break;
        case 88:
            fld_index = DRV_CFP_FIELD_UDFD7;
            break;
        case 89:
            fld_index = DRV_CFP_FIELD_UDFD8;
            break;
        case 90:
            fld_index = DRV_CFP_FIELD_UDFD9;
            break;
        case 91:
            fld_index = DRV_CFP_FIELD_UDFD10;
            break;
        case 92:
            fld_index = DRV_CFP_FIELD_UDFD11;
            break;
        /* coverity[dead_error_begin] */
        default:
            rv = SOC_E_INTERNAL;
            return rv;
    } 


    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "DRV_FP: %s fld_index = 0x%x\n"),
                 FUNCTION_NAME(), fld_index));
    SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM, fld_index, 
            drv_entry, p_data));
    
    SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM_MASK, fld_index, 
           drv_entry, p_mask));
    
    /*
     * Enable related UDF valid bits.
     */
    if (udf_idx < 93) {
            switch (udf_idx) {
                case 0:
                case 9:
                case 18:
                    fld_index = DRV_CFP_FIELD_UDFA0_VALID;
                    break;
                case 1:
                case 10:
                case 19:
                    fld_index = DRV_CFP_FIELD_UDFA1_VALID;
                    break;
                case 2:
                case 11:
                case 20:
                    fld_index = DRV_CFP_FIELD_UDFA2_VALID;
                    break;
                case 3:
                case 12:
                case 21:
                    fld_index = DRV_CFP_FIELD_UDFA3_VALID;
                    break;
                case 4:
                case 13:
                case 22:
                    fld_index = DRV_CFP_FIELD_UDFA4_VALID;
                    break;
                case 5:
                case 14:
                case 23:
                    fld_index = DRV_CFP_FIELD_UDFA5_VALID;
                    break;
                case 6:
                case 15:
                case 24:
                    fld_index = DRV_CFP_FIELD_UDFA6_VALID;
                    break;
                case 7:
                case 16:
                case 25:
                    fld_index = DRV_CFP_FIELD_UDFA7_VALID;
                    break;
                case 8:
                case 17:
                case 26:
                    fld_index = DRV_CFP_FIELD_UDFA8_VALID;
                    break;
                    
                case 27:
                case 36:
                case 45:
                    fld_index = DRV_CFP_FIELD_UDFB0_VALID;
                    break;
                case 28:
                case 37:
                case 46:
                    fld_index = DRV_CFP_FIELD_UDFB1_VALID;
                    break;
                case 29:
                case 38:
                case 47:
                    fld_index = DRV_CFP_FIELD_UDFB2_VALID;
                    break;
                case 30:
                case 39:
                case 48:
                    fld_index = DRV_CFP_FIELD_UDFB3_VALID;
                    break;
                case 31:
                case 40:
                case 49:
                    fld_index = DRV_CFP_FIELD_UDFB4_VALID;
                    break;
                case 32:
                case 41:
                case 50:
                    fld_index = DRV_CFP_FIELD_UDFB5_VALID;
                    break;
                case 33:
                case 42:
                case 51:
                    fld_index = DRV_CFP_FIELD_UDFB6_VALID;
                    break;
                case 34:
                case 43:
                case 52:
                    fld_index = DRV_CFP_FIELD_UDFB7_VALID;
                    break;
                case 35:
                case 44:
                case 53:
                    fld_index = DRV_CFP_FIELD_UDFB8_VALID;
                    break;
                    
                case 54:
                case 63:
                case 72:
                    fld_index = DRV_CFP_FIELD_UDFC0_VALID;
                    break;
                case 55:
                case 64:
                case 73:
                    fld_index = DRV_CFP_FIELD_UDFC1_VALID;
                    break;
                case 56:
                case 65:
                case 74:
                    fld_index = DRV_CFP_FIELD_UDFC2_VALID;
                    break;
                case 57:
                case 66:
                case 75:
                    fld_index = DRV_CFP_FIELD_UDFC3_VALID;
                    break;
                case 58:
                case 67:
                case 76:
                    fld_index = DRV_CFP_FIELD_UDFC4_VALID;
                    break;
                case 59:
                case 68:
                case 77:
                    fld_index = DRV_CFP_FIELD_UDFC5_VALID;
                    break;
                case 60:
                case 69:
                case 78:
                    fld_index = DRV_CFP_FIELD_UDFC6_VALID;
                    break;
                case 61:
                case 70:
                case 79:
                    fld_index = DRV_CFP_FIELD_UDFC7_VALID;
                    break;
                case 62:
                case 71:
                case 80:
                    fld_index = DRV_CFP_FIELD_UDFC8_VALID;
                    break;
                    
                case 81:
                    fld_index = DRV_CFP_FIELD_UDFD0_VALID;
                    break;
                case 82:
                    fld_index = DRV_CFP_FIELD_UDFD1_VALID;
                    break;
                case 83:
                    fld_index = DRV_CFP_FIELD_UDFD2_VALID;
                    break;
                case 84:
                    fld_index = DRV_CFP_FIELD_UDFD3_VALID;
                    break;
                case 85:
                    fld_index = DRV_CFP_FIELD_UDFD4_VALID;
                    break;
                case 86:
                    fld_index = DRV_CFP_FIELD_UDFD5_VALID;
                    break;
                case 87:
                    fld_index = DRV_CFP_FIELD_UDFD6_VALID;
                    break;
                case 88:
                    fld_index = DRV_CFP_FIELD_UDFD7_VALID;
                    break;
                case 89:
                    fld_index = DRV_CFP_FIELD_UDFD8_VALID;
                    break;
                case 90:
                    fld_index = DRV_CFP_FIELD_UDFD9_VALID;
                    break;
                case 91:
                    fld_index = DRV_CFP_FIELD_UDFD10_VALID;
                    break;
                case 92:
                    fld_index = DRV_CFP_FIELD_UDFD11_VALID;
                    break;
            }

            temp = 1;
            SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM, fld_index, 
                    drv_entry, &temp));
            
            SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM_MASK, fld_index, 
                    drv_entry, &temp));
    }
        
  LOG_VERBOSE(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP:  %s data= 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n"),
               FUNCTION_NAME(),
               drv_entry->tcam_data[0],drv_entry->tcam_data[1], 
               drv_entry->tcam_data[2], drv_entry->tcam_data[3], 
               drv_entry->tcam_data[4], drv_entry->tcam_data[5]));
  LOG_VERBOSE(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP:  %s mask= 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n"),
               FUNCTION_NAME(),
               drv_entry->tcam_mask[0], drv_entry->tcam_mask[1], 
               drv_entry->tcam_mask[2], drv_entry->tcam_mask[3], 
               drv_entry->tcam_mask[4], drv_entry->tcam_mask[5]));
    return SOC_E_NONE;
}

static int
_drv_vulcan_cfp_udf_value_get(int unit, uint32 udf_idx, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    uint32      fld_index = 0;
    uint32      temp = 0, temp_mask = 0;
    int rv = SOC_E_NONE;
    drv_cfp_entry_t  *drv_entry = (drv_cfp_entry_t *)entry;

    /* Check the valid bit first */
    if (udf_idx < 93) {
            switch (udf_idx) {
                case 0:
                case 9:
                case 18:
                    fld_index = DRV_CFP_FIELD_UDFA0_VALID;
                    break;
                case 1:
                case 10:
                case 19:
                    fld_index = DRV_CFP_FIELD_UDFA1_VALID;
                    break;
                case 2:
                case 11:
                case 20:
                    fld_index = DRV_CFP_FIELD_UDFA2_VALID;
                    break;
                case 3:
                case 12:
                case 21:
                    fld_index = DRV_CFP_FIELD_UDFA3_VALID;
                    break;
                case 4:
                case 13:
                case 22:
                    fld_index = DRV_CFP_FIELD_UDFA4_VALID;
                    break;
                case 5:
                case 14:
                case 23:
                    fld_index = DRV_CFP_FIELD_UDFA5_VALID;
                    break;
                case 6:
                case 15:
                case 24:
                    fld_index = DRV_CFP_FIELD_UDFA6_VALID;
                    break;
                case 7:
                case 16:
                case 25:
                    fld_index = DRV_CFP_FIELD_UDFA7_VALID;
                    break;
                case 8:
                case 17:
                case 26:
                    fld_index = DRV_CFP_FIELD_UDFA8_VALID;
                    break;
                    
                case 27:
                case 36:
                case 45:
                    fld_index = DRV_CFP_FIELD_UDFB0_VALID;
                    break;
                case 28:
                case 37:
                case 46:
                    fld_index = DRV_CFP_FIELD_UDFB1_VALID;
                    break;
                case 29:
                case 38:
                case 47:
                    fld_index = DRV_CFP_FIELD_UDFB2_VALID;
                    break;
                case 30:
                case 39:
                case 48:
                    fld_index = DRV_CFP_FIELD_UDFB3_VALID;
                    break;
                case 31:
                case 40:
                case 49:
                    fld_index = DRV_CFP_FIELD_UDFB4_VALID;
                    break;
                case 32:
                case 41:
                case 50:
                    fld_index = DRV_CFP_FIELD_UDFB5_VALID;
                    break;
                case 33:
                case 42:
                case 51:
                    fld_index = DRV_CFP_FIELD_UDFB6_VALID;
                    break;
                case 34:
                case 43:
                case 52:
                    fld_index = DRV_CFP_FIELD_UDFB7_VALID;
                    break;
                case 35:
                case 44:
                case 53:
                    fld_index = DRV_CFP_FIELD_UDFB8_VALID;
                    break;
                    
                case 54:
                case 63:
                case 72:
                    fld_index = DRV_CFP_FIELD_UDFC0_VALID;
                    break;
                case 55:
                case 64:
                case 73:
                    fld_index = DRV_CFP_FIELD_UDFC1_VALID;
                    break;
                case 56:
                case 65:
                case 74:
                    fld_index = DRV_CFP_FIELD_UDFC2_VALID;
                    break;
                case 57:
                case 66:
                case 75:
                    fld_index = DRV_CFP_FIELD_UDFC3_VALID;
                    break;
                case 58:
                case 67:
                case 76:
                    fld_index = DRV_CFP_FIELD_UDFC4_VALID;
                    break;
                case 59:
                case 68:
                case 77:
                    fld_index = DRV_CFP_FIELD_UDFC5_VALID;
                    break;
                case 60:
                case 69:
                case 78:
                    fld_index = DRV_CFP_FIELD_UDFC6_VALID;
                    break;
                case 61:
                case 70:
                case 79:
                    fld_index = DRV_CFP_FIELD_UDFC7_VALID;
                    break;
                case 62:
                case 71:
                case 80:
                    fld_index = DRV_CFP_FIELD_UDFC8_VALID;
                    break;
                    
                case 81:
                    fld_index = DRV_CFP_FIELD_UDFD0_VALID;
                    break;
                case 82:
                    fld_index = DRV_CFP_FIELD_UDFD1_VALID;
                    break;
                case 83:
                    fld_index = DRV_CFP_FIELD_UDFD2_VALID;
                    break;
                case 84:
                    fld_index = DRV_CFP_FIELD_UDFD3_VALID;
                    break;
                case 85:
                    fld_index = DRV_CFP_FIELD_UDFD4_VALID;
                    break;
                case 86:
                    fld_index = DRV_CFP_FIELD_UDFD5_VALID;
                    break;
                case 87:
                    fld_index = DRV_CFP_FIELD_UDFD6_VALID;
                    break;
                case 88:
                    fld_index = DRV_CFP_FIELD_UDFD7_VALID;
                    break;
                case 89:
                    fld_index = DRV_CFP_FIELD_UDFD8_VALID;
                    break;
                case 90:
                    fld_index = DRV_CFP_FIELD_UDFD9_VALID;
                    break;
                case 91:
                    fld_index = DRV_CFP_FIELD_UDFD10_VALID;
                    break;
                case 92:
                    fld_index = DRV_CFP_FIELD_UDFD11_VALID;
                    break;
            }

            temp = 1;
            SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_GET
                (unit, DRV_CFP_RAM_TCAM, fld_index, 
                    drv_entry, &temp));
            
            SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_GET
                (unit, DRV_CFP_RAM_TCAM_MASK, fld_index, 
                    drv_entry, &temp_mask));
    }

    if (!(temp & temp_mask)) {
        *p_data = 0;
        *p_mask = 0;
        return rv;
    }

    /* Get UDF data and mask value */
    switch (udf_idx) {
        case 0:
        case 9:
        case 18:
            fld_index = DRV_CFP_FIELD_UDFA0;
            break;
        case 1:
        case 10:
        case 19:
            fld_index = DRV_CFP_FIELD_UDFA1;
            break;
        case 2:
        case 11:
        case 20:
            fld_index = DRV_CFP_FIELD_UDFA2;
            break;
        case 3:
        case 12:
        case 21:
            fld_index = DRV_CFP_FIELD_UDFA3;
            break;
        case 4:
        case 13:
        case 22:
            fld_index = DRV_CFP_FIELD_UDFA4;
            break;
        case 5:
        case 14:
        case 23:
            fld_index = DRV_CFP_FIELD_UDFA5;
            break;
        case 6:
        case 15:
        case 24:
            fld_index = DRV_CFP_FIELD_UDFA6;
            break;
        case 7:
        case 16:
        case 25:
            fld_index = DRV_CFP_FIELD_UDFA7;
            break;
        case 8:
        case 17:
        case 26:
            fld_index = DRV_CFP_FIELD_UDFA8;
            break;
            
        case 27:
        case 36:
        case 45:
            fld_index = DRV_CFP_FIELD_UDFB0;
            break;
        case 28:
        case 37:
        case 46:
            fld_index = DRV_CFP_FIELD_UDFB1;
            break;
        case 29:
        case 38:
        case 47:
            fld_index = DRV_CFP_FIELD_UDFB2;
            break;
        case 30:
        case 39:
        case 48:
            fld_index = DRV_CFP_FIELD_UDFB3;
            break;
        case 31:
        case 40:
        case 49:
            fld_index = DRV_CFP_FIELD_UDFB4;
            break;
        case 32:
        case 41:
        case 50:
            fld_index = DRV_CFP_FIELD_UDFB5;
            break;
        case 33:
        case 42:
        case 51:
            fld_index = DRV_CFP_FIELD_UDFB6;
            break;
        case 34:
        case 43:
        case 52:
            fld_index = DRV_CFP_FIELD_UDFB7;
            break;
        case 35:
        case 44:
        case 53:
            fld_index = DRV_CFP_FIELD_UDFB8;
            break;
            
        case 54:
        case 63:
        case 72:
            fld_index = DRV_CFP_FIELD_UDFC0;
            break;
        case 55:
        case 64:
        case 73:
            fld_index = DRV_CFP_FIELD_UDFC1;
            break;
        case 56:
        case 65:
        case 74:
            fld_index = DRV_CFP_FIELD_UDFC2;
            break;
        case 57:
        case 66:
        case 75:
            fld_index = DRV_CFP_FIELD_UDFC3;
            break;
        case 58:
        case 67:
        case 76:
            fld_index = DRV_CFP_FIELD_UDFC4;
            break;
        case 59:
        case 68:
        case 77:
            fld_index = DRV_CFP_FIELD_UDFC5;
            break;
        case 60:
        case 69:
        case 78:
            fld_index = DRV_CFP_FIELD_UDFC6;
            break;
        case 61:
        case 70:
        case 79:
            fld_index = DRV_CFP_FIELD_UDFC7;
            break;
        case 62:
        case 71:
        case 80:
            fld_index = DRV_CFP_FIELD_UDFC8;
            break;
            
        case 81:
            fld_index = DRV_CFP_FIELD_UDFD0;
            break;
        case 82:
            fld_index = DRV_CFP_FIELD_UDFD1;
            break;
        case 83:
            fld_index = DRV_CFP_FIELD_UDFD2;
            break;
        case 84:
            fld_index = DRV_CFP_FIELD_UDFD3;
            break;
        case 85:
            fld_index = DRV_CFP_FIELD_UDFD4;
            break;
        case 86:
            fld_index = DRV_CFP_FIELD_UDFD5;
            break;
        case 87:
            fld_index = DRV_CFP_FIELD_UDFD6;
            break;
        case 88:
            fld_index = DRV_CFP_FIELD_UDFD7;
            break;
        case 89:
            fld_index = DRV_CFP_FIELD_UDFD8;
            break;
        case 90:
            fld_index = DRV_CFP_FIELD_UDFD9;
            break;
        case 91:
            fld_index = DRV_CFP_FIELD_UDFD10;
            break;
        case 92:
            fld_index = DRV_CFP_FIELD_UDFD11;
            break;

        default:
            rv = SOC_E_INTERNAL;
            return rv;
    } 


    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "DRV_FP: %s fld_index = 0x%x\n"),
                 FUNCTION_NAME(), fld_index));
    SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_GET
        (unit, DRV_CFP_RAM_TCAM, fld_index, 
            drv_entry, p_data));
    
    SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_GET
        (unit, DRV_CFP_RAM_TCAM_MASK, fld_index, 
           drv_entry, p_mask));
    
    return SOC_E_NONE;
}
#endif /* BCM_VULCAN_SUPPORT || STARFIGHTER || POLAR || NORTHSTAR || NS+ || SF3 */


int
_drv_gex_cfp_udf_value_set(int unit, uint32 udf_idx, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int rv = SOC_E_NONE;
    
    if (SOC_IS_VULCAN(unit) ||SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        return _drv_vulcan_cfp_udf_value_set(
            unit, udf_idx, entry, p_data, p_mask);
#endif
    } else {
        rv = SOC_E_UNAVAIL;
    }
    
    return rv;
}

int
_drv_gex_cfp_udf_value_get(int unit, uint32 udf_idx, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int rv = SOC_E_NONE;
    
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        return _drv_vulcan_cfp_udf_value_get(
            unit, udf_idx, entry, p_data, p_mask);
#endif
    } else {
        rv = SOC_E_UNAVAIL;
    }
    
    return rv;
}

#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
STATIC int
_drv_vulcan_udf_qset_mapping(int unit, drv_field_qset_t qset, drv_cfp_entry_t *drv_entry)
{
    int i;
    for (i = 0; i < DRV_FIELD_USER_NUM_UDFS; i++) {
        /* A0 ~ A26 */
        if ((i >= 0) &&  (i < 27)) {
            if (SHR_BITGET(qset.udf_map, i)) {
                 SOC_IF_ERROR_RETURN(DRV_CFP_QSET_SET              
                    (unit, (DRV_CFP_QUAL_UDFA0 + i), drv_entry, 1));
            }
        }
        /* B0 ~ B26 */
        if ((i >= 27) &&  (i < 54)) {
            if (SHR_BITGET(qset.udf_map, i)) {
                 SOC_IF_ERROR_RETURN(DRV_CFP_QSET_SET
                    (unit, (DRV_CFP_QUAL_UDFB0 + (i-27)), drv_entry, 1));
            }
        }
        /* C0 ~ C26 */
        if ((i >= 54) &&  (i < 81)) {
            if (SHR_BITGET(qset.udf_map, i)) {
                 SOC_IF_ERROR_RETURN(DRV_CFP_QSET_SET                
                    (unit, (DRV_CFP_QUAL_UDFC0 + (i-54)), drv_entry, 1));
            }
        }
        /* D0 ~ D11 */
        if ((i >= 81) &&  (i < 93)) {
            if (SHR_BITGET(qset.udf_map, i)) {
                 SOC_IF_ERROR_RETURN(DRV_CFP_QSET_SET                
                    (unit, (DRV_CFP_QUAL_UDFD0 + (i-81)), drv_entry, 1));
            }
        }
    }
    return SOC_E_NONE;
}
#endif /* BCM_VULCAN_SUPPORT || SF || POLAR || NS || NS+ || SF3 */


/* Translate to  driver qualifier set */
int
_drv_gex_fp_qset_to_cfp(int unit, drv_field_qset_t qset, drv_cfp_entry_t * drv_entry)
{
    int                 retval = SOC_E_UNAVAIL;
    uint32    dtag_mode = 0; /* dtag none */

    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        retval = _drv_vulcan_udf_qset_mapping(unit, qset, drv_entry);
        if (SOC_FAILURE(retval)){
            return retval;
        }
#endif        
    }
    
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifySrcIp6)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_IP6_SA, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyDstIp6)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_IP6_DA, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifySrcMac)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_MAC_SA, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyDstMac)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_MAC_DA, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifySnap)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_SNAP_HEADER, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifySrcIp)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_IP_SA, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyDstIp)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_IP_DA, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyLlc)) {
         return SOC_E_UNAVAIL;
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyInPort)) {
        if (SOC_IS_VULCAN(unit) || 
            SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) ||
            SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) || 
            SOC_IS_STARFIGHTER3(unit)) {
            retval = DRV_CFP_QSET_SET
                (unit, DRV_CFP_QUAL_SRC_PBMP, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);
        } else {
            retval = DRV_CFP_QSET_SET
                (unit, DRV_CFP_QUAL_SRC_PORT, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);
        }
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyInPorts)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_SRC_PBMP, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyIp6FlowLabel)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_IP6_FLOW_ID, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }

    /* VLAN TAG */
    SOC_IF_ERROR_RETURN(
        DRV_PORT_GET(unit, 0, DRV_PORT_PROP_DTAG_MODE, &dtag_mode));
    if (dtag_mode == 0) {
        /* mode none */
        if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyOuterVlan)) {
            retval = DRV_CFP_QSET_SET
                (unit, DRV_CFP_QUAL_USR_VID, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);
            retval = DRV_CFP_QSET_SET
                (unit, DRV_CFP_QUAL_USR_PRI, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);
            retval = DRV_CFP_QSET_SET
                (unit, DRV_CFP_QUAL_USR_CFI, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);
        } else {
            if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyOuterVlanId)) {
                retval = DRV_CFP_QSET_SET
                    (unit, DRV_CFP_QUAL_USR_VID, drv_entry, 1);
                SOC_IF_ERROR_RETURN(retval);
            }
            if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyOuterVlanPri)) {
                retval = DRV_CFP_QSET_SET
                    (unit, DRV_CFP_QUAL_USR_PRI, drv_entry, 1);
                SOC_IF_ERROR_RETURN(retval);
            }
            if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyOuterVlanCfi)) {
                retval = DRV_CFP_QSET_SET
                    (unit, DRV_CFP_QUAL_USR_CFI, drv_entry, 1);
                SOC_IF_ERROR_RETURN(retval);
            }
        }
    } else { 
        /* Double tagging Mode */
        if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyOuterVlan)) {
            retval = DRV_CFP_QSET_SET
                (unit, DRV_CFP_QUAL_SP_VID, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);
            retval = DRV_CFP_QSET_SET
                (unit, DRV_CFP_QUAL_SP_PRI, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);
            retval = DRV_CFP_QSET_SET
                (unit, DRV_CFP_QUAL_SP_CFI, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);
        } else {
            if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyOuterVlanId)) {
                retval = DRV_CFP_QSET_SET
                    (unit, DRV_CFP_QUAL_SP_VID, drv_entry, 1);
                SOC_IF_ERROR_RETURN(retval);
            }
            if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyOuterVlanPri)) {
                retval = DRV_CFP_QSET_SET
                    (unit, DRV_CFP_QUAL_SP_PRI, drv_entry, 1);
                SOC_IF_ERROR_RETURN(retval);
            }
            if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyOuterVlanCfi)) {
                retval = DRV_CFP_QSET_SET
                    (unit, DRV_CFP_QUAL_SP_CFI, drv_entry, 1);
                SOC_IF_ERROR_RETURN(retval);
            }
        }
    }
    
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyInnerVlan)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_USR_VID, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_USR_PRI, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_USR_CFI, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyInnerVlanId)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_USR_VID, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyInnerVlanPri)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_USR_PRI, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyInnerVlanCfi)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_USR_CFI, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }    
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyRangeCheck)) {
        /* robo chip only support L4 soure port < 1024 */
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_L4SRC_LESS1024, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyL4SrcPort)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_L4_SRC, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyL4DstPort)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_L4_DST, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyL4Ports)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_L4_PORTS, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyEtherType)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_ETYPE, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyIpProtocol)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_IP_PROTO, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyDSCP) ||
        DRV_FIELD_QSET_TEST(qset, drvFieldQualifyTos)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_IP_TOS, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyTtl)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_IP_TTL, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyIp6NextHeader)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_IP6_NEXT_HEADER, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyIp6TrafficClass)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_IP6_TRAFFIC_CLASS, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyIp6HopLimit)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_IP6_HOP_LIMIT, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifySrcPort)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_SRC_PORT, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyTcpControl)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_TCP_FLAG, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyPacketFormat)) {
        if (SOC_IS_VULCAN(unit)||SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            retval = DRV_CFP_QSET_SET
                (unit, DRV_CFP_QUAL_L2_FRM_FORMAT, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);
            retval = DRV_CFP_QSET_SET
                (unit, DRV_CFP_QUAL_L3_FRM_FORMAT, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);
            retval = DRV_CFP_QSET_SET
                (unit, DRV_CFP_QUAL_SPTAG, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);
            retval = DRV_CFP_QSET_SET
                (unit, DRV_CFP_QUAL_1QTAG, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);            
         } else {
            return SOC_E_UNAVAIL;
         }
    }

    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyIpProtocolCommon)) {
        return SOC_E_UNAVAIL;
    }

    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyIpType)){
        if (SOC_IS_VULCAN(unit)|| SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            retval = DRV_CFP_QSET_SET
                (unit, DRV_CFP_QUAL_L3_FRM_FORMAT, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);
        } else {
            return SOC_E_UNAVAIL;
        }
    }

    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifySrcIpEqualDstIp)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_IP_SAME, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyEqualL4Port)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_L4_SAME, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyTcpSequenceZero)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_TCP_SEQ_ZERO, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyTcpHeaderSize)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_TCP_HDR_LEN, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyIpFrag)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_IP_FRGA, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_IP_NON_FIRST_FRGA, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyIp4)) {
        if (SOC_IS_VULCAN(unit)|| SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            retval = DRV_CFP_QSET_SET
                (unit, DRV_CFP_QUAL_L3_FRM_FORMAT, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);
            SOC_IF_ERROR_RETURN(DRV_CFP_QSET_SET
                    (unit, DRV_CFP_QUAL_IPV4, drv_entry, 1));
        } else {
            return SOC_E_UNAVAIL;
        }
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyIp6)) {
        if (SOC_IS_VULCAN(unit)|| SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            retval = DRV_CFP_QSET_SET
                (unit, DRV_CFP_QUAL_L3_FRM_FORMAT, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);
            SOC_IF_ERROR_RETURN(DRV_CFP_QSET_SET
                    (unit, DRV_CFP_QUAL_IPV6, drv_entry, 1));
        } else {
            return SOC_E_UNAVAIL;
        }
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyL2Format)) {
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            retval = DRV_CFP_QSET_SET
                (unit, DRV_CFP_QUAL_L2_FRM_FORMAT, drv_entry, 1);
            SOC_IF_ERROR_RETURN(retval);
        } else {
            return SOC_E_UNAVAIL;
        }
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyVlanFormat)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_1QTAG, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);        
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_SPTAG, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);        
    }

    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyIpAuth)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_IP_AUTH, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);        
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyClassId)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_CLASS_ID, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);        
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyBigIcmpCheck)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_BIG_ICMP_CHECK, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);        
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyIcmpTypeCode)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_ICMPIGMP_TYPECODE, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);        
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyIgmpTypeMaxRespTime)) {
         retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_ICMPIGMP_TYPECODE, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    
    return retval;
}

int
_drv_gex_cfp_qset_mapping(drv_cfp_entry_t *entry, drv_field_qset_t *qset,
        int drv_qual, int bcm_qual)
{
    int wp, bp;

    if (drv_qual < 0 || drv_qual > DRV_CFP_QUAL_COUNT) {
        return SOC_E_PARAM;
    }
    if (bcm_qual < 0 || bcm_qual > drvFieldQualifyCount) {
        return SOC_E_PARAM;
    }
    
    wp = drv_qual / 32;
    bp = drv_qual & (32-1);
    if (entry->w[wp] & (0x1 << bp)) {
        DRV_FIELD_QSET_ADD(*qset, bcm_qual);
    }

    return SOC_E_NONE;
    
}
int
_drv_gex_fp_udf_mapping(drv_cfp_entry_t *entry, drv_field_qset_t *qset,
        int drv_qual, int udf_id)
{
    int wp, bp;

    if (drv_qual < 0 || drv_qual > DRV_CFP_QUAL_COUNT) {
        return SOC_E_PARAM;
    }
    if (udf_id < 0 || udf_id >= DRV_FIELD_USER_NUM_UDFS) {
        return SOC_E_PARAM;
    }
    
    wp = drv_qual / 32;
    bp = drv_qual & (32-1);
    if (entry->w[wp] & (0x1 << bp)) {
        SHR_BITSET(qset->udf_map, udf_id);
    }

    return SOC_E_NONE;
    
}


static int
_drv_gex_cfp_selcode_to_qset(int unit, 
    int slice_id, drv_field_qset_t *qset)
{
    int     rv = SOC_E_NONE;
    drv_cfp_entry_t drv_entry;
    uint32 dtag_mode = 0; /* mode none */

    sal_memset(&drv_entry, 0, sizeof(drv_cfp_entry_t));
    
     rv = DRV_CFP_SLICE_TO_QSET(unit, slice_id, &drv_entry);

    DRV_PORT_GET(unit, 0, DRV_PORT_PROP_DTAG_MODE, &dtag_mode);

    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_SRC_PORT, drvFieldQualifySrcPort));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_SRC_PORT, drvFieldQualifyInPort));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_1QTAG, drvFieldQualifyVlanFormat));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_SPTAG, drvFieldQualifyVlanFormat));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_MAC_DA, drvFieldQualifyDstMac));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_MAC_SA, drvFieldQualifySrcMac));
    if (dtag_mode == 0 ) {
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_USR_VID, drvFieldQualifyOuterVlanId));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_USR_PRI, drvFieldQualifyOuterVlanPri));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_USR_CFI, drvFieldQualifyOuterVlanCfi));
    } else {
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_SP_VID, drvFieldQualifyOuterVlanId));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_SP_PRI, drvFieldQualifyOuterVlanPri));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_SP_CFI, drvFieldQualifyOuterVlanCfi));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_USR_VID, drvFieldQualifyInnerVlanId));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_USR_PRI, drvFieldQualifyInnerVlanPri));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_USR_CFI, drvFieldQualifyInnerVlanCfi));
    }
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_ETYPE, drvFieldQualifyEtherType));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP_DA, drvFieldQualifyDstIp));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP_SA, drvFieldQualifySrcIp));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP_SAME, drvFieldQualifySrcIpEqualDstIp));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IPV4, drvFieldQualifyIp4));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_L4_DST, drvFieldQualifyL4DstPort));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_L4_SRC, drvFieldQualifyL4SrcPort));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_L4SRC_LESS1024, drvFieldQualifyRangeCheck));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_L4_SAME, drvFieldQualifyEqualL4Port));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_TCP_SEQ_ZERO, drvFieldQualifyTcpSequenceZero));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_TCP_HDR_LEN, drvFieldQualifyTcpHeaderSize));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_TCP_FLAG, drvFieldQualifyTcpControl));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP_PROTO, drvFieldQualifyIpProtocol));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP_TOS, drvFieldQualifyDSCP));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP_TTL, drvFieldQualifyTtl));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_SRC_PBMP, drvFieldQualifyInPorts));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_SRC_PBMP, drvFieldQualifyInPort));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_L2_FRM_FORMAT, drvFieldQualifyL2Format));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_L3_FRM_FORMAT, drvFieldQualifyIpType));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_L4_FRM_FORMAT, drvFieldQualifyIpProtocolCommon));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_VLAN_RANGE, drvFieldQualifyRangeCheck));    
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP6_FLOW_ID, drvFieldQualifyIp6FlowLabel));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP6_SA, drvFieldQualifySrcIp6));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP6_DA, drvFieldQualifyDstIp6));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IPV6, drvFieldQualifyIp6));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP_FRGA, drvFieldQualifyIpFrag));
    SOC_IF_ERROR_RETURN(
    _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP_AUTH, drvFieldQualifyIpAuth));

    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        SOC_IF_ERROR_RETURN(
            _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_L3_FRM_FORMAT, drvFieldQualifyIp4));
        SOC_IF_ERROR_RETURN(
            _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_L3_FRM_FORMAT, drvFieldQualifyIp6));

        if (DRV_FIELD_QSET_TEST(*qset, drvFieldQualifyL2Format) &&
            DRV_FIELD_QSET_TEST(*qset, drvFieldQualifyIpType)) {
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyPacketFormat);
        }
    }

    DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyStage);
    DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyStageIngress);

    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA0, 0));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA1, 1));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA2, 2));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA3, 3));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA4, 4));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA5, 5));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA6, 6));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA7, 7));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA8, 8));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA9, 9));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA10, 10));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA11, 11));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA12, 12));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA13, 13));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA14, 14));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA15, 15));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA16, 16));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA17, 17));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA18, 18));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA19, 19));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA20, 20));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA21, 21));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA22, 22));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA23, 23));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA24, 24));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA25, 25));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFA26, 26));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB0, 27));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB1, 28));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB2, 29));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB3, 30));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB4, 31));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB5, 32));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB6, 33));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB7, 34));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB8, 35));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB9, 36));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB10, 37));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB11, 38));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB12, 39));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB13, 40));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB14, 41));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB15, 42));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB16, 43));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB17, 44));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB18, 45));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB19, 46));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB0, 47));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB21, 48));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB22, 49));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB23, 50));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB24, 51));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB25, 52));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFB26, 53));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC0, 54));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC1, 55));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC2, 56));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC3, 57));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC4, 58));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC5, 59));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC6, 60));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC7, 61));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC8, 62));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC9, 63));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC10, 64));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC11, 65));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC12, 66));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC13, 67));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC14, 68));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC15, 69));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC16, 70));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC17, 71));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC18, 72));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC19, 73));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC20, 74));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC21, 75));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC22, 76));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC23, 77));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC24, 78));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC25, 79));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFC26, 80));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFD0, 81));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFD1, 82));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFD2, 83));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFD3, 84));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFD4, 85));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFD5, 86));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFD6, 87));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFD7, 88));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFD8, 89));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFD9, 90));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFD10, 91));
        SOC_IF_ERROR_RETURN(
        _drv_gex_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_UDFD11, 92));
#endif  /* VULCAN || POLAR || SF || NS || NS+ || SF3 */      
        
    }

    return rv;
}


/* allocate drv_entry for CFP driver */
int
_drv_gex_fp_entry_alloc(int unit, int stage_id, void **drv_entry)
{
    int memsize;
#ifdef BCM_NORTHSTARPLUS_SUPPORT
    int rv = SOC_E_NONE;   
#endif /* BCM_NORTHSTARPLUS_SUPPORT */

    memsize = sizeof(drv_cfp_entry_t);            
    *drv_entry = sal_alloc(memsize, "field drv entry");

    if (*drv_entry == NULL) {
       return SOC_E_MEMORY;
    }
    sal_memset (*drv_entry, 0, memsize);
#ifdef BCM_NORTHSTARPLUS_SUPPORT
    if (SOC_IS_NORTHSTARPLUS(unit)) {
        /* Disable the meter by default */
        rv = DRV_FP_POLICER_CONTROL(unit, stage_id, DRV_FIELD_POLICER_FREE, 
            *drv_entry, NULL);
        if (rv != 0){
            sal_free(*drv_entry);
            return SOC_E_INTERNAL;
        }
    }
    
#endif /* BCM_NORTHSTARPLUS_SUPPORT */
    return SOC_E_NONE;
}

/* copy drv_entry from src to dest*/
int
_drv_gex_fp_entry_copy(int unit, int stage_id, void *src_entry, void *dst_entry )
{
    int memsize;
    

    memsize = sizeof(drv_cfp_entry_t);            

    sal_memcpy(dst_entry, src_entry, 
        memsize);

  LOG_VERBOSE(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP: %s dst data= 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n"),
               FUNCTION_NAME(),
               ((drv_cfp_entry_t *)dst_entry)->tcam_data[0],((drv_cfp_entry_t *)dst_entry)->tcam_data[1], 
               ((drv_cfp_entry_t *)dst_entry)->tcam_data[2], ((drv_cfp_entry_t *)dst_entry)->tcam_data[3], 
               ((drv_cfp_entry_t *)dst_entry)->tcam_data[4], ((drv_cfp_entry_t *)dst_entry)->tcam_data[5]));
      
  LOG_VERBOSE(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP: %s src data= 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n"),
               FUNCTION_NAME(),
               ((drv_cfp_entry_t *)src_entry)->tcam_data[0], ((drv_cfp_entry_t *)src_entry)->tcam_data[1], 
               ((drv_cfp_entry_t *)src_entry)->tcam_data[2],  ((drv_cfp_entry_t *)src_entry)->tcam_data[3], 
               ((drv_cfp_entry_t *)src_entry)->tcam_data[4],  ((drv_cfp_entry_t *)src_entry)->tcam_data[5]));
  
    return SOC_E_NONE;
}
int
_drv_gex_fp_entry_clear(int unit, int stage_id, void* entry, int op)
{
    int memsize = 0;
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;    

    if (op == DRV_FIELD_ENTRY_MEM_CLEAR_DATAMASK) {
        memsize = sizeof(drv_entry->tcam_data);            
        sal_memset(drv_entry->tcam_data, 0, memsize);
        memsize = sizeof(drv_entry->tcam_mask);            
        sal_memset(drv_entry->tcam_mask, 0, memsize);    
    }

    return  SOC_E_NONE;
}

/* set sliceId to drv_entry->sliceId and TCAM */
int
_drv_gex_fp_entry_tcam_slice_id_set(int unit, int stage_id, void *entry, int sliceId)
{

    uint32 temp;
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    uint32 id;    
    int rv;

    if (SOC_IS_BLACKBIRD2(unit)) {
#ifdef BCM_BLACKBIRD2_SUPPORT
        /* No need to set the slice id */
        drv_entry->slice_id = sliceId;
        return SOC_E_NONE;
#endif /* BCM_BLACKBIRD2_SUPPORT */        
    }
    /* 
     * The slice format of BCM53115 determined by slice id and L3 framing value,
     * others are determioned by slice id value
     */
    temp = sliceId & 0x7;
    if (drv_entry->flags & _DRV_CFP_SLICE_CHAIN) {
        drv_entry->flags |= _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN;    
        id = CFP_53115_SLICE_ID_WITH_CHAIN;    
        rv = DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_SLICE_ID, 
                drv_entry, &id);
        if (SOC_FAILURE(rv)){
            drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN);
            return rv;
        }
        temp = 0x3;
        rv = DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_SLICE_ID, 
                drv_entry, &temp);
        if (SOC_FAILURE(rv)){
            drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN);
            return rv;
        }
        drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN);    

        id = 0;
        SOC_IF_ERROR_RETURN(
            DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_SLICE_ID, 
                drv_entry, &id));

        temp = 0x3;
        SOC_IF_ERROR_RETURN(
            DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_SLICE_ID, 
                drv_entry, &temp));
    } else {
        SOC_IF_ERROR_RETURN(
            DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_SLICE_ID, 
                drv_entry, &temp));

        temp = 0x7;
        SOC_IF_ERROR_RETURN(
            DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_SLICE_ID, 
                drv_entry, &temp));
    }

    drv_entry->slice_id = sliceId;
    return SOC_E_NONE;
}

#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
int
_drv_gex_fp_entry_tcam_chain_mode_get(int unit, int stage_id, void *drv_entry, int sliceId, void *mode)    
{
    uint32 id;
    drv_cfp_tcam_t *cfp_chain;

    *((int *)mode) = 0;

    id = CFP_53115_SLICE_ID(sliceId);

    if ((id == CFP_53115_SLICE_ID_WITH_CHAIN)) {
        ((drv_cfp_entry_t *)(drv_entry))->flags |= _DRV_CFP_SLICE_CHAIN;
        if (((drv_cfp_entry_t *)(drv_entry))->cfp_chain == NULL) {
            cfp_chain = sal_alloc(sizeof(drv_cfp_tcam_t),"cfp chain tcam");
            if (cfp_chain == NULL) {
                return SOC_E_MEMORY;
            }
            sal_memset(cfp_chain, 0, sizeof(drv_cfp_tcam_t));
            cfp_chain->chain_id = -1;
            ((drv_cfp_entry_t *)(drv_entry))->cfp_chain = cfp_chain;
        }
        *((int *)mode) = _DRV_CFP_SLICE_CHAIN;
    }

    return SOC_E_NONE;
}
#endif /* BCM_VULCAN_SUPPORT || STARFIGHTER || POLAR || NORTHSTAR || NS+ || SF3*/


/* move drv_entry(TCAM) only or both TCAM&Counter  with amount*/
int
_drv_gex_fp_entry_tcam_move(int unit, int stage_id, void *entry, int amount, 
        void *counter)
{
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    uint32      tcam_idx_old, tcam_idx_new;
    int   tcam_idx_max;
    uint32      temp;
    drv_cfp_entry_t     cfp_entry;

    
    tcam_idx_old = drv_entry->id;
    tcam_idx_new = tcam_idx_old + amount;

    SOC_IF_ERROR_RETURN(
        DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_CFP_TCAM_SIZE, 
        (uint32 *)&tcam_idx_max));
    tcam_idx_max = tcam_idx_max -1;

    LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP: %s old:%d new:%d total(max):%d\n"),
               FUNCTION_NAME(), tcam_idx_old, tcam_idx_new, tcam_idx_max));
    assert(0 <= (int)tcam_idx_old && (int)tcam_idx_old <= (int)tcam_idx_max);
    assert(0 <= (int)tcam_idx_new && (int)tcam_idx_new <= (int)tcam_idx_max);

    sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
    
    /* Move the hardware entry.*/
    SOC_IF_ERROR_RETURN(
        DRV_CFP_ENTRY_READ
        (unit, tcam_idx_old, DRV_CFP_RAM_ALL, &cfp_entry));
    SOC_IF_ERROR_RETURN(
        DRV_CFP_ENTRY_WRITE
        (unit, tcam_idx_new, DRV_CFP_RAM_ALL, &cfp_entry));
    /* Sync the counter values */
    if (counter != NULL) {
        SOC_IF_ERROR_RETURN(
            DRV_CFP_STAT_GET
            (unit, DRV_CFP_STAT_INBAND, tcam_idx_old, &temp));
        SOC_IF_ERROR_RETURN(
            DRV_CFP_STAT_SET
            (unit, DRV_CFP_STAT_INBAND, tcam_idx_new, temp));
        SOC_IF_ERROR_RETURN(
            DRV_CFP_STAT_GET
            (unit, DRV_CFP_STAT_OUTBAND, tcam_idx_old, &temp));
        SOC_IF_ERROR_RETURN(
            DRV_CFP_STAT_SET
            (unit, DRV_CFP_STAT_OUTBAND, tcam_idx_new, temp));
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            SOC_IF_ERROR_RETURN(
                DRV_CFP_STAT_GET
                (unit, DRV_CFP_STAT_YELLOW, tcam_idx_old, &temp));
            SOC_IF_ERROR_RETURN(
                DRV_CFP_STAT_SET
                (unit, DRV_CFP_STAT_YELLOW, tcam_idx_new, temp));
        }
#endif /* BCM_NORTHSTARPLUS_SUPPORT */
    }


    if (SOC_IS_BLACKBIRD2(unit)) {
#ifdef BCM_BLACKBIRD2_SUPPORT
        /* Write all zero to the TCP UDP Key register */
        cfp_entry.tcam_data[0] = 0;
        SOC_IF_ERROR_RETURN(
            DRV_CFP_ENTRY_WRITE
                (unit, tcam_idx_old, DRV_CFP_RAM_ALL, &cfp_entry));
        return SOC_E_NONE;
#endif        
    }
    
    /* Clear original one */
    sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
    temp = 0;
    SOC_IF_ERROR_RETURN(
        DRV_CFP_ENTRY_WRITE
            (unit, tcam_idx_old, DRV_CFP_RAM_ALL, &cfp_entry));

    SOC_IF_ERROR_RETURN(
        DRV_CFP_STAT_SET
        (unit, DRV_CFP_STAT_ALL, tcam_idx_old, temp));
    
    return SOC_E_NONE;
}

int
_drv_gex_fp_entry_tcam_valid_control(int unit, int stage_id, void *entry, int tcam_idx, uint32 *valid)
{
    uint32 temp;
    drv_cfp_entry_t *drv_entry =(drv_cfp_entry_t *)entry;
    

    temp = *valid;
    SOC_IF_ERROR_RETURN(
        DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_VALID, 
            drv_entry, &temp));

    SOC_IF_ERROR_RETURN(
        DRV_CFP_ENTRY_WRITE
            (unit, tcam_idx, DRV_CFP_RAM_TCAM, drv_entry));

    LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV FP: set tcam idx %d valid %d\n"),
               tcam_idx,temp));
    return SOC_E_NONE;
}

int
_drv_gex_fp_entry_tcam_policy_install(int unit, int stage_id, void *entry, int tcam_idx,
    int *tcam_chain_idx)
{
    uint32 temp;
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    int retval;
    int chain_id = 0;
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    int chain_slice = 0;
#endif /* BCM_VULCAN_SUPPORT || STARFIGHTER || POLAR || NORTHSTAR || NS+ */    

    if (!SOC_IS_BLACKBIRD2(unit)) {
        /* 
         * Get the value to set into each entry's valid field. 
         * The valid value is depend on chips. Let the driver service to handle it.
         */
        temp = 1;
        retval = DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_VALID, drv_entry, &temp);
        SOC_IF_ERROR_RETURN(retval);
        retval = DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM_MASK, 
                DRV_CFP_FIELD_VALID, drv_entry, &temp);
        SOC_IF_ERROR_RETURN(retval);

#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        /* Auto configure the L3 Frame type */
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            if ((drv_entry->flags & _DRV_CFP_FRAME_IP) ||        
                (drv_entry->flags & _DRV_CFP_FRAME_NONIP)){
                
                if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
                    SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                    SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                    temp = CFP_53115_L3_FRAME(drv_entry->slice_id);
                    if (temp == 
                        CFP_53115_L3_FRAME_CHAIN) {
                        chain_slice = 1;
                    }
                }
                if (!chain_slice) {
                    SOC_IF_ERROR_RETURN(
                        DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_TCAM, 
                        DRV_CFP_FIELD_L3_FRM_FORMAT,
                        drv_entry, &temp));
                    if (drv_entry->flags & _DRV_CFP_FRAME_IPANY) {
                        temp = 0x2;
                    }else if (drv_entry->flags & _DRV_CFP_FRAME_ANY) {
                        temp = 0;
                    } else {
                        temp = 0x3;
                    }
                    SOC_IF_ERROR_RETURN(
                        DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_TCAM_MASK, 
                        DRV_CFP_FIELD_L3_FRM_FORMAT, drv_entry, &temp));        
                }
            }
        }
#endif /* BCM_53115 || BCM_53125 || POLAR || NORTHSTAR || NS+ */   

    }
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "DRV_FP: %s data= 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n"),
                 FUNCTION_NAME(),
                 drv_entry->tcam_data[0],drv_entry->tcam_data[1], 
                 drv_entry->tcam_data[2], drv_entry->tcam_data[3], 
                 drv_entry->tcam_data[4], drv_entry->tcam_data[5]));
  
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "DRV_FP: %s action= 0x%x, 0x%x\n"),
                 FUNCTION_NAME(),drv_entry->act_data[0],drv_entry->act_data[1]));
    if (*tcam_chain_idx != -1) {
    /* entry with chain {slice0,slice3}*/        

        /* configure slice0 framing*/
        drv_entry->flags |= _DRV_CFP_SLICE_CONFIG_SLICE_MAIN;
        temp = CFP_53115_L3_FRAME(drv_entry->slice_id);
        retval = DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_TCAM, 
            DRV_CFP_FIELD_L3_FRM_FORMAT,
            drv_entry, &temp);
        if (SOC_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Write Tcam entry index = %d fail.\n"),  
                       tcam_idx));
            drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
            return retval;
        }

        if (drv_entry->flags & _DRV_CFP_FRAME_IPANY){
            temp = 0x2;
        }else if (drv_entry->flags & _DRV_CFP_FRAME_ANY) {
            temp = 0;
        } else {
            temp = 0x3;
        }

        retval = DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_TCAM_MASK, 
            DRV_CFP_FIELD_L3_FRM_FORMAT, drv_entry, &temp);   
        if (SOC_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Write Tcam entry index = %d fail.\n"),  
                       tcam_idx));
            drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
            return retval;
        }
        /* write the slice0 data tcam*/
        retval = DRV_CFP_ENTRY_WRITE
            (unit, tcam_idx, DRV_CFP_RAM_TCAM, drv_entry);
        if (SOC_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Write Tcam entry index = %d fail.\n"),  
                       tcam_idx));
            drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
            return retval;
        }

        /* internally handle the classification id used to chain slice3*/
        if (drv_entry->cfp_chain->chain_id == -1) {
            retval = DRV_FP_ID_CONTROL(unit, _DRV_FP_ID_CFP_CHAIN_ID, 
                _DRV_FP_ID_CTRL_ALLOC, 0, &chain_id, NULL);

            if (SOC_FAILURE(retval)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "FP Error: Can't get chain_id for Tcam entry index = %d fail.\n"),  
                           tcam_idx));
                drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
                return retval;
            }
            drv_entry->cfp_chain->chain_id  = chain_id;
        } else {
            chain_id = drv_entry->cfp_chain->chain_id;
        }

        LOG_DEBUG(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "DRV_FP: %s get chain_id %d\n"),
                   FUNCTION_NAME(), chain_id));

        /*
         * configure the slice0's chain id (classification id), 
         * enable the chain and give a chain_id 
         */
        retval = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CHAIN_ID, 
                        drv_entry, 0, 0);
        if (SOC_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Can't set chain_id action for Tcam entry %d..\n"),  
                       tcam_idx));
            drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
            return retval;
        }

        retval = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CLASSFICATION_ID, 
                        drv_entry, chain_id, 0);
        if (SOC_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: set chain_id %d action fail.\n"),  
                       chain_id));
            drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
            return retval;
        }

        /* write the slice0 action */
        retval = DRV_CFP_ENTRY_WRITE
            (unit, tcam_idx, DRV_CFP_RAM_ACT, drv_entry);
        if (SOC_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Write Act/Pol entry index = %d fail.\n"),  
                       tcam_idx));
            return retval;
        }

        drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_MAIN);


        /* qualify slice3 classification id */     
        drv_entry->flags |= _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN;
        temp = 0xfff;
        retval = _drv_gex_cfp_qual_value_set(unit, drvFieldQualifyClassId, 
                        drv_entry, (uint32 *)&chain_id, &temp);
        if (SOC_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: set qualify chain_id %d fail.\n"),  
                       chain_id));
            drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN);
            return retval;
        }

        temp = 1;
        DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_VALID, drv_entry, &temp);

        DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM_MASK, 
                DRV_CFP_FIELD_VALID, drv_entry, &temp);

        drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN);

        /* write the slice3 data and action tcam*/
        retval = DRV_CFP_ENTRY_WRITE
            (unit, *tcam_chain_idx, DRV_CFP_RAM_ACT, drv_entry);
        if (SOC_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Write Act/Pol entry index = %d fail.\n"),  
                       tcam_idx));
            return retval;
        }
        retval = DRV_CFP_ENTRY_WRITE
            (unit, *tcam_chain_idx, DRV_CFP_RAM_TCAM, drv_entry);
        if (SOC_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Write Tcam entry index = %d fail.\n"),  
                       tcam_idx));
            return retval;
        }
    } else {
        /* normal entry */
        retval = DRV_CFP_ENTRY_WRITE
            (unit, tcam_idx, DRV_CFP_RAM_ACT, drv_entry);
        if (SOC_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Write Act/Pol entry index = %d fail.\n"),  
                       tcam_idx));
            return retval;
        }
        retval = DRV_CFP_ENTRY_WRITE
            (unit, tcam_idx, DRV_CFP_RAM_TCAM, drv_entry);
        if (SOC_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Write Tcam entry index = %d fail.\n"),  
                       tcam_idx));
            return retval;
        }
    }

    return retval;
}
int
_drv_gex_fp_entry_tcam_meter_install(int unit, int stage_id, void *entry, 
        int tcam_idx, int *tcam_chain_idx)
{
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    uint32 temp_meter[2];
    int retval;

    
    if (*tcam_chain_idx != -1) {
        /* write the slice3 meter tcam*/
        retval = DRV_CFP_ENTRY_WRITE
            (unit, *tcam_chain_idx, DRV_CFP_RAM_METER, drv_entry);
        /* clean slice0 meter tcam */
        memcpy(&temp_meter, drv_entry->meter_data, sizeof(temp_meter));
        memset(drv_entry->meter_data, 0, sizeof(temp_meter));
        retval = DRV_CFP_ENTRY_WRITE
            (unit, tcam_idx, DRV_CFP_RAM_METER, drv_entry);
        memcpy(drv_entry->meter_data, &temp_meter, sizeof(temp_meter));
    } else {
        retval = DRV_CFP_ENTRY_WRITE
            (unit, tcam_idx, DRV_CFP_RAM_METER, drv_entry);
        if (SOC_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Write Tcam entry index = %d fail.\n"),  
                       tcam_idx));
            return retval;
        }
    }
    return retval;
}

int
_drv_gex_fp_qualify_support(int unit, drv_field_qset_t qset)
{
    int rv = SOC_E_NONE;
    int i;

    /* Qualifier checking */
    for (i = 0; i < drvFieldQualifyCount; i++) {
        if (DRV_FIELD_QSET_TEST(qset, i)) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            
                switch (i) {
                    case drvFieldQualifySrcIp6: /* use UDF */
                    case drvFieldQualifyDstIp6: /* use UDF */
                    case drvFieldQualifySrcMac: /* use UDF */
                    case drvFieldQualifyDstMac: /* use UDF */
                    case drvFieldQualifySnap: /* use UDF */
                    case drvFieldQualifySrcIp: /* use UDF */
                    case drvFieldQualifyDstIp: /* use UDF */
                    case drvFieldQualifyInPort:
                    case drvFieldQualifyInPorts:
                    case drvFieldQualifyIp6FlowLabel: /* use UDF */
                    case drvFieldQualifyOuterVlan:
                    case drvFieldQualifyOuterVlanId:
                    case drvFieldQualifyOuterVlanPri:
                    case drvFieldQualifyOuterVlanCfi:
                    case drvFieldQualifyInnerVlan:
                    case drvFieldQualifyInnerVlanId:
                    case drvFieldQualifyInnerVlanCfi:
                    case drvFieldQualifyInnerVlanPri:
                    case drvFieldQualifyL4SrcPort: /* use UDF */
                    case drvFieldQualifyL4DstPort: /* use UDF */
                    case drvFieldQualifyEtherType: 
                    case drvFieldQualifyIpProtocol: /* same as Ip6NextHeader */
                    case drvFieldQualifyDSCP:   /* same as ToS, Ip6TrafficClass */
                    case drvFieldQualifyTtl: /* same as Ip6HopLimit */
                    case drvFieldQualifyTcpControl: /* use UDF */
                    case drvFieldQualifyPacketFormat:
                    case drvFieldQualifyIpType:
                    case drvFieldQualifyStageIngress:
                    case drvFieldQualifyIpFrag:
                    case drvFieldQualifyL2Format:
                    case drvFieldQualifyVlanFormat:
                    case drvFieldQualifyIpAuth:
                    case drvFieldQualifyClassId:
                    case drvFieldQualifyIp4:
                    case drvFieldQualifyIp6:
                        break;
                    default:
                        return SOC_E_UNAVAIL;
                }                
            } else if (SOC_IS_BLACKBIRD2(unit)) {
            
                switch (i) {
                    case drvFieldQualifyL4SrcPort:
                    case drvFieldQualifyL4DstPort:
                    case drvFieldQualifyL4Ports: 
                    case drvFieldQualifyStageIngress:
                        break;
                    default:
                        return SOC_E_UNAVAIL;
                }
            } else { /* other chips */
                return SOC_E_UNAVAIL;
            }
        }
    }
    return rv;
}

int 
_drv_gex_fp_entry_tcam_enable_set(int unit, int stage_id, 
        int tcam_idx, int *tcam_chain_idx, int enable)
{
    int                 rv = SOC_E_UNAVAIL;
    drv_cfp_entry_t     *cfp_entry = NULL;
    uint32      temp;
    drv_cfp_tcam_t *cfp_chain;

    rv = _drv_gex_fp_entry_alloc(unit, stage_id, (void **)&cfp_entry);
    if (SOC_FAILURE(rv)) {
        return rv;
    }

    if (*tcam_chain_idx != -1) {
        ((drv_cfp_entry_t *)cfp_entry)->flags |=_DRV_CFP_SLICE_CHAIN;
        ((drv_cfp_entry_t *)cfp_entry)->flags |= 
                    _DRV_CFP_SLICE_CONFIG_SLICE_MAIN;
        if (((drv_cfp_entry_t *)(cfp_entry))->cfp_chain == NULL) {
            cfp_chain = sal_alloc(sizeof(drv_cfp_tcam_t),"cfp chain tcam");
            if (cfp_chain == NULL) {
                sal_free(cfp_entry);
                return SOC_E_MEMORY;
            }
            sal_memset(cfp_chain, 0, sizeof(drv_cfp_tcam_t));
            cfp_chain->chain_id = -1;
            ((drv_cfp_entry_t *)(cfp_entry))->cfp_chain = cfp_chain;
        }
        rv = DRV_CFP_ENTRY_READ
            (unit, *tcam_chain_idx, DRV_CFP_RAM_TCAM, cfp_entry);
        if (SOC_FAILURE(rv)) {
            sal_free(((drv_cfp_entry_t *)cfp_entry)->cfp_chain);
            ((drv_cfp_entry_t *)cfp_entry)->cfp_chain = NULL;
            sal_free(cfp_entry);
            return rv;
        }
       ((drv_cfp_entry_t *)cfp_entry)->slice_id =
            (CFP_53115_L3_FRAME_IPV6 << 2) |
            CFP_53115_SLICE_ID_WITH_CHAIN;
        if (enable) {
            temp = 1;
        } else {
            temp = 0;
        }
        rv = _drv_gex_fp_entry_tcam_valid_control(unit, stage_id, 
                cfp_entry, *tcam_chain_idx, &temp);
        if (SOC_FAILURE(rv)) {
            sal_free(((drv_cfp_entry_t *)cfp_entry)->cfp_chain);
            ((drv_cfp_entry_t *)cfp_entry)->cfp_chain = NULL;
            sal_free(cfp_entry);
            return rv;
        }
        ((drv_cfp_entry_t *)cfp_entry)->flags = 0;
        sal_free(((drv_cfp_entry_t *)cfp_entry)->cfp_chain);
        ((drv_cfp_entry_t *)cfp_entry)->cfp_chain = NULL;
    }
    rv = DRV_CFP_ENTRY_READ
        (unit, tcam_idx, DRV_CFP_RAM_TCAM, cfp_entry);

    if (SOC_FAILURE(rv)) {
       sal_free(cfp_entry);
        return rv;
    }

    if (SOC_IS_BLACKBIRD2(unit)) {
#ifdef BCM_BLACKBIRD2_SUPPORT
        /* Write all zero to the TCP UDP Key register */
        if (!enable) {
            cfp_entry->tcam_data[0] = 0;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ENTRY_WRITE
                    (unit, tcam_idx, DRV_CFP_RAM_TCAM, cfp_entry));
            rv = SOC_E_NONE;
        }
#endif        
    } else {
        if (enable) {
            temp = 1;
        } else {
            temp = 0;
        }
        rv = _drv_gex_fp_entry_tcam_valid_control(
            unit, stage_id, cfp_entry, tcam_idx, &temp);
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "DRV_FP %s : data= 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n"),
                 FUNCTION_NAME(),
                 cfp_entry->tcam_data[0],cfp_entry->tcam_data[1], 
                 cfp_entry->tcam_data[2], cfp_entry->tcam_data[3], 
                 cfp_entry->tcam_data[4], cfp_entry->tcam_data[5]));
   sal_free(cfp_entry);
   return rv;
}

int 
_drv_gex_fp_entry_tcam_clear(int unit, int stage_id, int tcam_idx, 
        int *tcam_chain_idx, int op)
{
    int                 rv = SOC_E_UNAVAIL;
    drv_cfp_entry_t     *cfp_entry = NULL;
    int mem_id;
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    uint32 temp;
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */

    rv = _drv_gex_fp_entry_alloc(unit, stage_id, (void **)&cfp_entry);
    if (SOC_FAILURE(rv)) {
        return rv;
    }
    if (op == DRV_FIELD_ENTRY_TCAM_DATAMASK_CLEAR) {
        mem_id = DRV_CFP_RAM_TCAM;
    } else {
        mem_id = DRV_CFP_RAM_ALL;
    }

#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {

        /* Disable all meters */
        temp = 0x3; /* Disable mode */

        /* coverity[callee_ptr_arith] */
        rv = DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
                DRV_CFP_FIELD_POLICER_MODE, cfp_entry, &temp);
        if (SOC_FAILURE(rv)) {
            sal_free(cfp_entry);
            return rv;
        }
    }
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */

    rv =  DRV_CFP_ENTRY_WRITE(unit, tcam_idx, 
        mem_id, cfp_entry);
    
    if (SOC_FAILURE(rv)) {
        sal_free(cfp_entry);
        return rv;
    }
    if (*tcam_chain_idx != -1) {
        rv =  DRV_CFP_ENTRY_WRITE(unit, *tcam_chain_idx, 
        mem_id, cfp_entry);
    }

    sal_free(cfp_entry);
    return rv;
}

int
_drv_gex_fp_entry_tcam_reinstall(int unit, int stage_id, void *entry, int tcam_idx, 
        int *mode)
{
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    int retval;

    if (*mode & _DRV_CFP_SLICE_CHAIN){
        drv_entry->flags |= _DRV_CFP_SLICE_CONFIG_SLICE_MAIN;
    }

    retval = DRV_CFP_ENTRY_WRITE
        (unit, tcam_idx, DRV_CFP_RAM_ALL, drv_entry);
    if (SOC_FAILURE(retval)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "FP Error: Write Act/Pol entry index = %d fail.\n"),  
                   tcam_idx));
        return retval;
    }

    if (*mode & _DRV_FP_ENTRY_CHAIN){
        drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
    }
    
    return SOC_E_NONE;
}

#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
int
_drv_gex_fp_id_control_deinit(int unit)
{

    /* Release chain Id management. */
    if (CFP_CHAIN_USED(unit) != NULL) {
        sal_free(CFP_CHAIN_USED(unit));
        CFP_CHAIN_USED(unit) = NULL;
    }    
    return SOC_E_NONE;
}


int
_drv_gex_fp_id_control_init(int unit)
{
    uint32 temp = 0;
    int rv;
    uint32 alloc_size = 0;

    /* Initialize chain Id management. */
    if (CFP_CHAIN_USED(unit) != NULL) {
        sal_free(CFP_CHAIN_USED(unit));
    }

    rv = DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_CFP_TCAM_SIZE, &temp);
    if (SOC_FAILURE(rv)){
        return SOC_E_FAIL;

    }

    CFP_CHAIN_SIZE(unit) = temp;
    alloc_size = SHR_BITALLOCSIZE(CFP_CHAIN_SIZE(unit));
    CFP_CHAIN_USED(unit) = sal_alloc(alloc_size, "CFP_CHAIN_ID");

    if (CFP_CHAIN_USED(unit) == NULL) {
        _drv_gex_fp_id_control_deinit(unit);
        return SOC_E_MEMORY;
    }
    sal_memset(CFP_CHAIN_USED(unit), 0, alloc_size);

    return SOC_E_NONE;
}
#endif /* BCM_VULCAN_SUPPORT || STARFIGHTER || POLAR || NORTHSTAR || NS+ || SF3 */

int 
drv_gex_fp_init(int unit,int stage_id)
{
    int rv = SOC_E_NONE;
    uint8 init_all, init_cfp;
    
    init_all = 0;
    init_cfp = 0;

    if (stage_id == -1) {
        init_all = 1;
    } else if (stage_id == DRV_FIELD_STAGE_INGRESS){
        init_cfp = 1;
    } else {
        return SOC_E_PARAM;
    }

    if (init_all |init_cfp) {
        rv = DRV_CFP_INIT(unit);
        SOC_IF_ERROR_RETURN(rv);
    }

#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        rv = _drv_gex_fp_id_control_init(unit);
    }
#endif    

    return rv;
}
int 
drv_gex_fp_deinit(int unit,int stage_id)
{
    int rv;
    int port;
    uint8 deinit_all, deinit_cfp;
    
    deinit_all = 0;
    deinit_cfp = 0;

    if (stage_id == -1) {
        deinit_all = 1;
    } else if (stage_id == DRV_FIELD_STAGE_INGRESS){
        deinit_cfp = 1;
    } else {
        return SOC_E_PARAM;
    }

    if (deinit_all |deinit_cfp) {
        /* Disable CFP */
        PBMP_ITER(PBMP_PORT_ALL(unit), port) {
            rv = DRV_CFP_CONTROL_SET
                (unit, DRV_CFP_ENABLE, port, 0);
            SOC_IF_ERROR_RETURN(rv);
        }        
        /* Clear HW TABLE */
        rv = DRV_CFP_CONTROL_SET(unit, DRV_CFP_TCAM_RESET, 0, 0);
        SOC_IF_ERROR_RETURN(rv);

    }    

    return SOC_E_NONE;
}

int
drv_gex_fp_qual_value_set(int unit, int stage_id, drv_field_qualify_t qual, 
            void *entry, uint32* p_data, uint32* p_mask)
{
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    int rv;

    if (DRV_FIELD_STAGE_INGRESS != stage_id)
        return SOC_E_PARAM;

    LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP: %s qual %d \n"),
               FUNCTION_NAME(),qual));
    rv = _drv_gex_cfp_qual_value_set 
                (unit, qual, drv_entry, p_data, p_mask);
    return rv;
}

int
drv_gex_fp_udf_value_set(int unit, int stage_id, uint32 udf_idex, 
        void *entry, uint32* p_data, uint32* p_mask)
{
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    int rv;

    switch(stage_id) {
        case DRV_FIELD_STAGE_INGRESS:
            rv = _drv_gex_cfp_udf_value_set 
                        (unit, udf_idex, drv_entry, p_data, p_mask);
            break;
        case DRV_FIELD_STAGE_LOOKUP:
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_FIELD_STAGE_EGRESS:
            rv = SOC_E_UNAVAIL;
            break;
        default:
            rv = SOC_E_PARAM;
            break;        
    }
    return rv;
}
int
drv_gex_fp_udf_value_get(int unit, int stage_id, uint32 udf_idex, void *entry, uint32* p_data, uint32* p_mask)
{
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    int rv;

    switch(stage_id) {
        case DRV_FIELD_STAGE_INGRESS:
            rv = _drv_gex_cfp_udf_value_get 
                        (unit, udf_idex, drv_entry, p_data, p_mask);
            break;
        case DRV_FIELD_STAGE_LOOKUP:
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_FIELD_STAGE_EGRESS:
            rv = SOC_E_UNAVAIL;
            break;
        default:
            rv = SOC_E_PARAM;
            break;        
    }
    return rv;
}

int
drv_gex_fp_qual_value_get(int unit, int stage_id, drv_field_qualify_t qual, 
        void *entry, uint32* p_data, uint32* p_mask)
{
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    int rv;

    if (DRV_FIELD_STAGE_INGRESS != stage_id)
        return SOC_E_PARAM;

    LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP: %s qual %d \n"),
               FUNCTION_NAME(),qual));
    rv = _drv_gex_cfp_qual_value_get 
                (unit, qual, drv_entry, p_data, p_mask);
    return rv;
}

/*
 * DRV_FIELD_ENTRY_MEM_ALLOC
 *  get the allocated drv_entry pointer from alloc_entry
 * DRV_FIELD_ENTRY_MEM_COPY
 *  entry copy from dst to src
 * DRV_FIELD_ENTRY_MEM_CLEAR_DATAMASK 
 *  clear the drv_entry data & mask 
 */
int
drv_gex_fp_entry_mem_control (int unit, int stage_id, int op, void *src_entry, void *dst_entry, 
        void **alloc_entry)
{
    int rv = SOC_E_NONE;

    switch (op) {
        case DRV_FIELD_ENTRY_MEM_ALLOC:
            rv = _drv_gex_fp_entry_alloc(unit, stage_id, alloc_entry);        
            break;    
        case DRV_FIELD_ENTRY_MEM_COPY:
            rv = _drv_gex_fp_entry_copy(unit, stage_id, src_entry, dst_entry);
            break;                
        case DRV_FIELD_ENTRY_MEM_CLEAR_DATAMASK:
            rv = _drv_gex_fp_entry_clear(unit, stage_id, src_entry, op);
            break;
    }
    return rv;
}

/*
  * DRV_FIELD_ENTRY_TCAM_SLICE_ID_SET    
  *     set param1 as slice id in drv_entry.                       
  * DRV_FIELD_ENTRY_TCAM_MOVE       
  *     move drv_entry only or both entry & param2(counter) by param1(amount)   
  * DRV_FIELD_ENTRY_TCAM_REMOVE     
  *     give an index to unset entry's valid bit, param1:tcam_index     
  * DRV_FIELD_ENTRY_TCAM_CLEAR      
  *     give an index to clear this drv_entry from TCAM. param1:tcam_index   
  * DRV_FIELD_ENTRY_TCAM_DATAMASK_CLEAR 
  *     give an index to clear this drv_entry data and mask only from TCAM. param1:tcam_index   
  * DRV_FIELD_ENTRY_TCAM_POLICY_INSTALL   
  *     write the drv_entry and act to TCAM, param1:tcam_index.   
  */
  
int
drv_gex_fp_entry_tcam_control(int unit, int stage_id, void* drv_entry, int op, 
        int param1, void *param2)
{
    int rv = SOC_E_NONE;

    if (DRV_FIELD_STAGE_INGRESS != stage_id)
        return SOC_E_PARAM;

    switch (op) {
        case DRV_FIELD_ENTRY_TCAM_SLICE_ID_SET:
            rv = _drv_gex_fp_entry_tcam_slice_id_set(unit, stage_id, drv_entry, param1);
            break;
        case DRV_FIELD_ENTRY_TCAM_MOVE:
            rv = _drv_gex_fp_entry_tcam_move(unit, stage_id, drv_entry, param1, param2);
            break;
        case DRV_FIELD_ENTRY_TCAM_REMOVE:
            rv = _drv_gex_fp_entry_tcam_enable_set(unit, stage_id, param1, param2, FALSE);
            break;
        case DRV_FIELD_ENTRY_TCAM_ENABLE:
            rv = _drv_gex_fp_entry_tcam_enable_set(unit, stage_id, param1, param2, TRUE);
            break;
        case DRV_FIELD_ENTRY_TCAM_CLEAR:
        case DRV_FIELD_ENTRY_TCAM_DATAMASK_CLEAR:
            rv = _drv_gex_fp_entry_tcam_clear(unit, stage_id, param1, param2, op);
            break;
        case DRV_FIELD_ENTRY_TCAM_POLICY_INSTALL:
            rv = _drv_gex_fp_entry_tcam_policy_install(unit, stage_id, drv_entry, param1, param2);
            break;
        case DRV_FIELD_ENTRY_TCAM_ENTRY_REINSTALL:
            rv = _drv_gex_fp_entry_tcam_reinstall(unit, stage_id, drv_entry, param1, param2);
            break;
        case DRV_FIELD_ENTRY_TCAM_METER_INSTALL:
            rv = _drv_gex_fp_entry_tcam_meter_install(unit, stage_id, drv_entry, param1, param2);
            break;   
        case DRV_FIELD_ENTRY_TCAM_SW_INDEX_SET:   
            ((drv_cfp_entry_t *)drv_entry)->id = param1;            
            break;
        case DRV_FIELD_ENTRY_TCAM_SW_FLAGS_SET:
            if (DRV_FIELD_STAGE_INGRESS == stage_id) {
                *((uint32 *)param2) = ((drv_cfp_entry_t *)drv_entry)->flags;
                if (param1 != -1) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                        ((drv_cfp_entry_t *)drv_entry)->flags &= ~(_DRV_CFP_FRAME_ALL);
                        ((drv_cfp_entry_t *)drv_entry)->flags |= param1;   
                    }
#endif /* BCM_VULCAN_SUPPORT || STARFIGHTER || POLAR || NORTHSTAR || NS+ || SF3 */                    
                }
            }
            break;
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        case DRV_FIELD_ENTRY_TCAM_CHAIN_MODE_GET:
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                rv = _drv_gex_fp_entry_tcam_chain_mode_get(unit, stage_id, 
                    drv_entry, param1, param2);
            }
            break;                
        case DRV_FIELD_ENTRY_TCAM_CHAIN_DESTROY:            
            if (DRV_FIELD_STAGE_INGRESS == stage_id) {
                int chain_id;                
                if (((drv_cfp_entry_t *)drv_entry)->cfp_chain != NULL){
                    chain_id = ((drv_cfp_entry_t *)drv_entry)->cfp_chain->chain_id;
                    if(chain_id != -1 ){
                        DRV_FP_ID_CONTROL(unit, _DRV_FP_ID_CFP_CHAIN_ID, 
                            _DRV_FP_ID_CTRL_FREE, 0, (int *)&chain_id, NULL);
                    }
                    sal_free(((drv_cfp_entry_t *)drv_entry)->cfp_chain);
                    ((drv_cfp_entry_t *)drv_entry)->cfp_chain = NULL;
                }
            }
            break;
#endif /* BCM_VULCAN_SUPPORT ||STARFIGHTER ||  POLAR || NORTHSTAR || NS+ || SF3 */            
    }

    return rv;
}


int
drv_gex_fp_action_conflict(int unit, int stage_id, 
        drv_field_action_t act1, drv_field_action_t act2)
{
    int act1_switch = 0, act2_switch = 0;
    

    if (DRV_FIELD_STAGE_INGRESS != stage_id)
        return SOC_E_PARAM;

    if (act1== act2) {
        /* action duplication */
        return 1;
    }
    /* 
     * Check confilct action types 
     * 2 : switch forwarding actions
     * 3 : vlan modification actions
     * 4 : CosQ or Packet internal priority actions
     */
    switch (act1) {
        case drvFieldActionCopyToCpu:
        case drvFieldActionRpCopyToCpu:
        case drvFieldActionGpCopyToCpu:
        case drvFieldActionRedirect:
        case drvFieldActionRpRedirectPort:
        case drvFieldActionGpRedirectPort:
        case drvFieldActionDrop:
        case drvFieldActionRpDrop:
        case drvFieldActionGpDrop:
        case drvFieldActionMirrorIngress:
        case drvFieldActionRpMirrorIngress:
        case drvFieldActionGpMirrorIngress:
        case drvFieldActionRedirectPbmp:
        case drvFieldActionCopyToCpuCancel:
        case drvFieldActionRedirectCancel:
        case drvFieldActionDropCancel:
        case drvFieldActionEgressMask: 
            act1_switch = 2; /* 2: forwarding actions */
            break;
        case drvFieldActionInnerVlanNew:
        case drvFieldActionOuterVlanNew:
            act1_switch = 3; /* 3: vlan change actions */
            break;
        case drvFieldActionCosQNew:
        case drvFieldActionPrioIntNew:
            act1_switch = 4;
            break;
        default:
            break;
    }
    switch (act2) {
        case drvFieldActionCopyToCpu:
        case drvFieldActionRpCopyToCpu:
        case drvFieldActionGpCopyToCpu:
        case drvFieldActionRedirect:
        case drvFieldActionRpRedirectPort:
        case drvFieldActionGpRedirectPort:
        case drvFieldActionDrop:
        case drvFieldActionRpDrop:
        case drvFieldActionGpDrop:
        case drvFieldActionMirrorIngress:
        case drvFieldActionRpMirrorIngress:
        case drvFieldActionGpMirrorIngress:
        case drvFieldActionRedirectPbmp:
        case drvFieldActionCopyToCpuCancel:
        case drvFieldActionRedirectCancel:
        case drvFieldActionDropCancel:
        case drvFieldActionEgressMask:
            act2_switch = 2; /* 2: forwarding actions */
            break;
        case drvFieldActionInnerVlanNew:
        case drvFieldActionOuterVlanNew:
            act2_switch = 3; /* 3: vlan change actions */
            break;
        case drvFieldActionCosQNew:
        case drvFieldActionPrioIntNew:
            act1_switch = 4;
            break;
        default:
            break;
    }
    if (act1_switch && act2_switch) {
        if (act1_switch == act2_switch) {
            return act1_switch;
        }
    }
    return 0;
}
int
drv_gex_fp_action_support_check(int unit, int stage_id, 
            drv_field_action_t action)
{
    int                 retval = SOC_E_NONE;

    if (DRV_FIELD_STAGE_INGRESS != stage_id) {
        return SOC_E_PARAM;
    }

    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        switch (action) {
            case drvFieldActionPrioIntNew:
            case drvFieldActionCopyToCpu: 
            case drvFieldActionRedirect:
            case drvFieldActionDrop:
            case drvFieldActionRpCopyToCpu:
            case drvFieldActionRpDrop:
            case drvFieldActionBypassEap:
            case drvFieldActionBypassStp:
            case drvFieldActionBypassVlan: 
            case drvFieldActionDscpNew:
            case drvFieldActionLoopback:
            case drvFieldActionMeterConfig:
            case drvFieldActionMirrorIngress: 
            case drvFieldActionNewClassId:
            case drvFieldActionNewReasonCode:
            case drvFieldActionNewTc:
            case drvFieldActionRedirectPbmp:
            case drvFieldActionRpDscpNew:
            case drvFieldActionUpdateCounter:
            case drvFieldActionGpDrop:
            case drvFieldActionGpCopyToCpu:
            case drvFieldActionGpDscpNew:
            case drvFieldActionRpMirrorIngress:
            case drvFieldActionRpRedirectPort:
            case drvFieldActionGpMirrorIngress:
            case drvFieldActionGpRedirectPort:                
                retval = SOC_E_NONE;
                break;
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            case drvFieldActionYpDrop:
            case drvFieldActionYpCopyToCpu:
            case drvFieldActionYpDscpNew:
            case drvFieldActionYpMirrorIngress:
            case drvFieldActionYpRedirectPort:
            case drvFieldActionL2LearnLimitDropCancel:
            case drvFieldActionPrioPktCancel:
            case drvFieldActionOuterVlanCfiCancel:
            case drvFieldActionInnerVlanPriCancel:
            case drvFieldActionPrioIntRemark:
			case drvFieldActionUseDefaultWred:
            case drvFieldActionDropPrecedence:
                if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                    retval = SOC_E_NONE;
                } else {
                    retval = SOC_E_UNAVAIL;
                }
                break;
#endif /* BCM_NORTHSTARPLUS_SUPPORT */
                
            case drvFieldActionEgressMask:
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
                if(SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                    SOC_IS_NORTHSTARPLUS(unit)) {
                    retval = SOC_E_NONE;
                } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ */
                {
                    retval = SOC_E_UNAVAIL;
                }
                break;
            default:
                retval = SOC_E_UNAVAIL;
                break;
        }
    }

    if (SOC_IS_BLACKBIRD2(unit)) {
#ifdef BCM_BLACKBIRD2_SUPPORT
        switch (action) {
            case drvFieldActionNewTc:
            case drvFieldActionCopyToCpu: 
            case drvFieldActionRedirect:
            case drvFieldActionDrop:
            case drvFieldActionMirrorIngress: 
            case drvFieldActionRedirectPbmp:
                retval = SOC_E_NONE;
                break;
            default:
                retval = SOC_E_UNAVAIL;
                break;
        }
#endif /* BCM_BLACKBIRD2_SUPPORT */        
    }
    
    return retval;
}

int
drv_gex_fp_action_add(int unit, int stage_id, void *drv_entry, 
        drv_field_action_t action, uint32 param0, uint32 param1)
{
    uint32          temp;
#ifdef BCM_NORTHSTARPLUS_SUPPORT
    drv_cfp_entry_t *cfp_entry;    
#endif /* BCM_NORTHSTARPLUS_SUPPORT */

    if (DRV_FIELD_STAGE_INGRESS != stage_id) {
        return SOC_E_PARAM;
    }

    LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP: %s action %d p0:%x p1:%x\n"),
               FUNCTION_NAME(),action,param0,param1));
    /* For Redirect-to-port action, range check the port value.  */
    if ((action == drvFieldActionRedirect || 
         action == drvFieldActionRpRedirectPort ||
         action == drvFieldActionMirrorIngress || 
         action == drvFieldActionRpMirrorIngress) &&
        !SOC_PORT_VALID_RANGE(unit, (soc_port_t)param1 & ~(1 << 6))) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "FP ERROR: param1=%d out of range for Redirect.\n"),
                   param1));
        return SOC_E_PARAM;
    }

#ifdef BCM_NORTHSTARPLUS_SUPPORT
    if (SOC_IS_NORTHSTARPLUS(unit)) {
        /* Check if the red actions were support of current policer mode */
        cfp_entry = (drv_cfp_entry_t *)drv_entry;
        if (cfp_entry->pl_cfg) {
            if (((cfp_entry->pl_cfg->mode == drvPolicerModeCoupledTrTcmDs) ||
                (cfp_entry->pl_cfg->mode == drvPolicerModeSrTcm)) ||
                (cfp_entry->pl_cfg->flags & DRV_POLICER_DROP_RED)) {
                /* only drop action is supported for red packets for MEF policer mode */
                switch(action) {
                    case drvFieldActionRpCopyToCpu:
                    case drvFieldActionRpDscpNew:
                    case drvFieldActionRpMirrorIngress:
                    case drvFieldActionRpRedirectPort:
                        return (SOC_E_UNAVAIL);
                    case drvFieldActionRpDrop:
                        return (SOC_E_NONE);
                    default:
                        break;
                }
            }
        }
    }
#endif /* BCM_NORTHSTARPLUS_SUPPORT */

    switch (action) {
        case drvFieldActionPrioIntNew:
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                 SOC_IF_ERROR_RETURN(            
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_CHANGE_TC, 
                        drv_entry, param0, 0));
            }
            break;
        case drvFieldActionPrioPktNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_PCP_NEW, 
                drv_entry, param0, 0));
            break;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
        case drvFieldActionEgressMask:
            if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit)) {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_REMOVE, 
                        drv_entry, ~param0, 0));
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_REMOVE, 
                        drv_entry, ~param0, 0));
            }
            break;
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ */
        case drvFieldActionCopyToCpu:
            /* should use symbol */
            temp = CMIC_PORT(unit);           
             /*
             * If the port field is bitmap format,
             * translate the port number to bitmap format
             */
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_APPEND, 
                        drv_entry, 0x1 << temp, 0));
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_APPEND, 
                        drv_entry, 0x1 << temp, 0));
            } else if (SOC_IS_BLACKBIRD2(unit)) {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_APPEND, 
                        drv_entry, 0x1 << temp, 0));
            }
            break;
        case drvFieldActionMirrorIngress:           
            /*
             * If the port field is bitmap format,
             * translate the port number to bitmap format
             */
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_APPEND, 
                        drv_entry, 0x1 << param1, 0));
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_APPEND, 
                        drv_entry, 0x1 << param1, 0));
            } else if (SOC_IS_BLACKBIRD2(unit)) {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_APPEND, 
                        drv_entry, 0x1 << param1, 0));
            }
            break;
        case drvFieldActionRedirect:
            /*
             * If the port field is bitmap format,
             * translate the port number to bitmap format
             */
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_REDIRECT, 
                        drv_entry, 0x1 << param1, 0));
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_REDIRECT, 
                        drv_entry, 0x1 << param1, 0));
            } else if (SOC_IS_BLACKBIRD2(unit)) {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_REDIRECT, 
                        drv_entry, 0x1 << param1, 0));
            }
            break;
        case drvFieldActionRedirectPbmp:
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_REDIRECT, 
                        drv_entry, param0, 0));
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_REDIRECT, 
                        drv_entry, param0, 0));
            } else if (SOC_IS_BLACKBIRD2(unit)) {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_REDIRECT, 
                        drv_entry, param0, 0));
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
        case drvFieldActionDrop:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_OB_DROP, 
                    drv_entry, 0, 0));
            if (!SOC_IS_BLACKBIRD2(unit)) {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_IB_DROP, 
                    drv_entry, 0, 0));
            }
            break;
        case drvFieldActionDscpNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_OB_DSCP_NEW, 
                    drv_entry, param0, 0));
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_IB_DSCP_NEW, 
                    drv_entry, param0, 0));
            break;
        case drvFieldActionCosQNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_OB_COSQ_NEW, 
                    drv_entry, param0, 0));
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_IB_COSQ_NEW, 
                    drv_entry, param0, 0));                                            
            break;
        case drvFieldActionCosQCpuNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_COSQ_CPU_NEW, 
                    drv_entry, param0, 0));
            break;
        case drvFieldActionRpCopyToCpu:
        case drvFieldActionYpCopyToCpu:
            temp = CMIC_PORT(unit);
            /*
             * If the port field is bitmap format,
             * translate the port number to bitmap format
             */
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_APPEND, 
                        drv_entry, 0x1 << temp, 0));
            } else {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_APPEND, 
                        drv_entry, temp, 0));
            }            
            break;
        case drvFieldActionRpDrop:
        case drvFieldActionYpDrop:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_OB_DROP, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionRpDscpNew:
        case drvFieldActionYpDscpNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_OB_DSCP_NEW, 
                    drv_entry, param0, 0));
            break;
        case drvFieldActionGpCopyToCpu:
            temp = CMIC_PORT(unit);
            /*
             * If the port field is bitmap format,
             * translate the port number to bitmap format
             */
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                SOC_IF_ERROR_RETURN(                
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_APPEND, 
                        drv_entry, 0x1 << temp, 0));
            } else {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_APPEND, 
                        drv_entry, temp, 0));
            }            
            break;
        case drvFieldActionGpDrop:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_IB_DROP, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionGpDscpNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_IB_DSCP_NEW, 
                    drv_entry, param0, 0));
            break;
        case drvFieldActionRpPrioIntNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_OB_MOD_INT_PRI, 
                    drv_entry, param0, 0));
            break;
        case drvFieldActionGpPrioIntNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_IB_MOD_INT_PRI, 
                    drv_entry, param0, 0));
            break;
        case drvFieldActionInnerVlanNew:
                /*
                 * param0 carries the value of new vid.
                 */
            SOC_IF_ERROR_RETURN(DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_CHANGE_CVID, 
                drv_entry, param0, 0));
            break;
        case drvFieldActionOuterVlanNew:
                /*
                 * param0 carries the value of new vid.
                 */
            SOC_IF_ERROR_RETURN(DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_CHANGE_SPVID, 
                drv_entry, param0, 0));
            break;
        case drvFieldActionNewTc:
            SOC_IF_ERROR_RETURN(            
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_CHANGE_TC, 
                    drv_entry, param0, 0));
            break;
        case drvFieldActionNewClassId:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_CLASSFICATION_ID, 
                    drv_entry, param0, 0));
            break;
        case drvFieldActionLoopback:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_LOOPBACK, 
                    drv_entry, param0, 0));
            break;
        case drvFieldActionNewReasonCode:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_REASON_CODE, 
                    drv_entry, param0, 0));
            break;
        case drvFieldActionBypassStp:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_STP_BYPASS, 
                    drv_entry, param0, 0));
            break;
        case drvFieldActionBypassEap:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_EAP_BYPASS, 
                    drv_entry, param0, 0));
            break;
        case drvFieldActionBypassVlan:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_VLAN_BYPASS, 
                    drv_entry, param0, 0));
            break;    
        case drvFieldActionRpRedirectPort:
        case drvFieldActionYpRedirectPort:
            /*
             * If the port field is bitmap format,
             * translate the port number to bitmap format
             */
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_REDIRECT, 
                        drv_entry, 0x1 << param1, 0));
            } else {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_REDIRECT, 
                        drv_entry, param1, 0));
            }
            break;
        case drvFieldActionRpMirrorIngress:
        case drvFieldActionYpMirrorIngress:
            /*
             * If the port field is bitmap format,
             * translate the port number to bitmap format
             */
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_APPEND, 
                        drv_entry, 0x1 << param1, 0));
            } else {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_APPEND, 
                        drv_entry, param1, 0));
            }
            break;
        case drvFieldActionGpRedirectPort:
            /*
             * If the port field is bitmap format,
             * translate the port number to bitmap format
             */
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                    SOC_IF_ERROR_RETURN(
                        DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_REDIRECT, 
                            drv_entry, 0x1 << param1, 0));
            } else {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_REDIRECT, 
                        drv_entry, param1, 0));
            }
            break;
        case drvFieldActionGpMirrorIngress:           
            /*
             * If the port field is bitmap format,
             * translate the port number to bitmap format
             */
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                (DRV_SERVICES(unit)->cfp_action_set)
                    (unit, DRV_CFP_ACT_IB_APPEND, 
                        drv_entry, 0x1 << param1, 0);
            } else {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_IB_APPEND, 
                        drv_entry, param1, 0));
            }
            break;

        case drvFieldActionUpdateCounter:
            /* Do nothing with action ram */
            break;
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        case drvFieldActionDropPrecedence:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_CHANGE_COLOR, 
                    drv_entry, param0, 0));
            break;
        case drvFieldActionL2LearnLimitDropCancel:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_MAC_LIMIT_DROP_BYPASS, 
                    drv_entry, TRUE, 0));
            break;
        case drvFieldActionPrioPktCancel:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_SPCP_RMK_DISABLE, 
                    drv_entry, TRUE, 0));
            break;
        case drvFieldActionOuterVlanCfiCancel:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_DEI_RMK_DISABLE, 
                    drv_entry, TRUE, 0));
            break;
        case drvFieldActionInnerVlanPriCancel:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_CPCP_RMK_DISABLE, 
                    drv_entry, TRUE, 0));
            break;
        case drvFieldActionPrioIntRemark:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_CHANGE_TC_OUTPUT, 
                    drv_entry, param0, 0));
            break;
        case drvFieldActionUseDefaultWred:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_WRED_DEFAULT, 
                    drv_entry, TRUE, 0));
            break;

#endif /* BCM_NORTHSTARPLUS_SUPPORT */            
        default:
            return SOC_E_UNAVAIL;
    }
    return SOC_E_NONE;
}

int
drv_gex_fp_action_remove(int unit, int stage_id, void *entry, 
        drv_field_action_t action, uint32 param0, uint32 param1)
{
    int rv = SOC_E_NONE;
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;

    if (DRV_FIELD_STAGE_INGRESS != stage_id)
        return SOC_E_PARAM;

#ifdef BCM_NORTHSTARPLUS_SUPPORT
    if (SOC_IS_NORTHSTARPLUS(unit)) {
        /* Check if the red actions were support of current policer mode */
        drv_entry = (drv_cfp_entry_t *)entry;
        if (drv_entry->pl_cfg) {
            if (((drv_entry->pl_cfg->mode == drvPolicerModeCoupledTrTcmDs) ||
                (drv_entry->pl_cfg->mode == drvPolicerModeSrTcm)) ||
                (drv_entry->pl_cfg->flags & DRV_POLICER_DROP_RED)) {
                /* only drop action is supported for red packets for MEF policer mode */
                switch(action) {
                    case drvFieldActionRpDrop:
                        return (SOC_E_NONE);
                    default:
                        break;
                }
            }
        }
    }
#endif /* BCM_NORTHSTARPLUS_SUPPORT */    

    switch (action) {
        case drvFieldActionPrioIntNew:
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                 SOC_IF_ERROR_RETURN(            
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_CHANGE_TC_CANCEL, 
                        drv_entry, param0, 0));
#endif
            }
            break;

        case drvFieldActionRpPrioIntNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_OB_MOD_INT_PRI_CANCEL, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionGpPrioIntNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_IB_MOD_INT_PRI_CANCEL, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionPrioPktNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_PCP_CANCEL, 
                    drv_entry, 0, 0));
            break;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
        case drvFieldActionEgressMask:
            if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit)) {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_IB_NONE, 
                    drv_entry, 0, 0));
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_NONE, 
                        drv_entry, 0, 0));                
            }
            break;
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || NS+ */
        case drvFieldActionCopyToCpu:
        case drvFieldActionDrop:
        case drvFieldActionRedirectPbmp:
        case drvFieldActionRedirect:
        case drvFieldActionMirrorIngress:
            if (!SOC_IS_BLACKBIRD2(unit)) {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_IB_NONE, 
                    drv_entry, 0, 0));
            }
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_OB_NONE, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionRpMirrorIngress:
        case drvFieldActionRpRedirectPort:
        case drvFieldActionYpMirrorIngress:
        case drvFieldActionYpRedirectPort:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_OB_NONE, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionGpMirrorIngress:
        case drvFieldActionGpRedirectPort:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_IB_NONE, 
                    drv_entry, 0, 0));
                    break;
        case drvFieldActionDscpNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_IB_DSCP_CANCEL, 
                    drv_entry, 0, 0));
            SOC_IF_ERROR_RETURN(            
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_OB_DSCP_CANCEL, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionCosQNew:
            SOC_IF_ERROR_RETURN(            
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_IB_COSQ_CANCEL, 
                    drv_entry, 0, 0));
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_OB_COSQ_CANCEL, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionCosQCpuNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_COSQ_CPU_CANCEL, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionRpDrop:
        case drvFieldActionRpCopyToCpu:
        case drvFieldActionYpDrop:
        case drvFieldActionYpCopyToCpu:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_OB_NONE, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionGpDrop:
        case drvFieldActionGpCopyToCpu:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_IB_NONE, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionInnerVlanNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_CHANGE_CVID_CANCEL, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionOuterVlanNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_CHANGE_SPVID_CANCEL, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionNewClassId:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_CLASSFICATION_ID, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionLoopback:    
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_LOOPBACK,
                    drv_entry, 0, 0));
            break;
        case drvFieldActionNewReasonCode:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_REASON_CODE, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionBypassStp:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_STP_BYPASS, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionBypassEap:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_EAP_BYPASS, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionBypassVlan:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_VLAN_BYPASS, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionNewTc:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_CHANGE_TC_CANCEL, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionRpDscpNew:
        case drvFieldActionYpDscpNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_OB_DSCP_CANCEL, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionGpDscpNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_IB_DSCP_CANCEL, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionUpdateCounter:
            /* Do nothing with action ram */
            rv = SOC_E_NONE;
            break;
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        case drvFieldActionDropPrecedence:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_CHANGE_COLOR_CANCEL, 
                    drv_entry, param1, 0));
            break;
        case drvFieldActionL2LearnLimitDropCancel:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_MAC_LIMIT_DROP_BYPASS, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionPrioPktCancel:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_SPCP_RMK_DISABLE, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionOuterVlanCfiCancel:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_DEI_RMK_DISABLE, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionInnerVlanPriCancel:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_CPCP_RMK_DISABLE, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionPrioIntRemark:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_CHANGE_TC_OUTPUT_CANCEL, 
                    drv_entry, 0, 0));
            break;
         case drvFieldActionUseDefaultWred:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_WRED_DEFAULT, 
                    drv_entry, 0, 0));
            break;
#endif /* BCM_NORTHSTARPLUS_SUPPORT */               
        default:
            rv = SOC_E_UNAVAIL;
            break;
        }
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "DRV_FP: %s action(%d)= 0x%x, 0x%x\n"),
                 FUNCTION_NAME(),action, drv_entry->act_data[0],drv_entry->act_data[1]));
    return rv;
}

/* API has the resbosible to  free the entry get from drv_fp_selcode_mode_get() */
int
drv_gex_fp_selcode_mode_get(int unit, int stage_id, 
                    void*  qset,  int mode, int8 *slice_id, uint32 *slice_map, void **entry)
{
    int rv;
    drv_cfp_entry_t *drv_entry = NULL;
    uint32  temp, flag;
    drv_field_qset_t drv_qset;

    if (DRV_FIELD_STAGE_INGRESS != stage_id)
        return SOC_E_PARAM;


    drv_entry = sal_alloc(sizeof(drv_cfp_entry_t),"cfp entry");
    if (drv_entry == NULL){
        return SOC_E_MEMORY;
    }

    sal_memset(drv_entry, 0, sizeof(drv_cfp_entry_t));

    sal_memcpy(&drv_qset, qset, sizeof(drv_field_qset_t));

    rv = _drv_gex_fp_qset_to_cfp(unit, drv_qset, 
                    (drv_cfp_entry_t *)drv_entry);

    if (SOC_FAILURE(rv)) {
        sal_free(drv_entry);
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "fail in _fp_qset_to_cfp %d\n"),
                   rv));
        return rv;
    }

    if (mode == DRV_FIELD_GROUP_MODE_DOUBLE) {
        flag = DRV_FIELD_QUAL_CHAIN;
    } else {
        flag = 0;
    }

    rv = DRV_CFP_SLICE_ID_SELECT
                    (unit, drv_entry, &temp, flag);
    
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        if (rv == SOC_E_UNAVAIL) {
            sal_free(drv_entry);
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "fail in DRV_CFP_SLICE_ID_SELECT %d\n"),
                       rv));
            return rv;
        }
    } else {
        if (SOC_FAILURE(rv)) {
            sal_free(drv_entry);
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "fail in DRV_CFP_SLICE_ID_SELECT %d\n"),
                       rv));
            return rv;
        }
    }
    *slice_id = temp;
    *entry = drv_entry;
    *slice_map = 0;
    return rv;
}

int
drv_gex_fp_selcode_to_qset(int unit,  int stage_id, int slice_id, 
        void *qset) 
{
    int rv;
    drv_field_qset_t *drv_qset = (drv_field_qset_t *)qset;

    if (DRV_FIELD_STAGE_INGRESS != stage_id)
        return SOC_E_PARAM;
    
    rv =_drv_gex_cfp_selcode_to_qset(unit, slice_id,drv_qset);

    return rv;
}

int
drv_gex_fp_qualify_support(int unit, int stage_id,  void *qset)
{
    drv_field_qset_t drv_qset;

    if (DRV_FIELD_STAGE_INGRESS != stage_id)
        return SOC_E_PARAM;

    sal_memcpy(&drv_qset, qset, sizeof(drv_field_qset_t));
    return _drv_gex_fp_qualify_support(unit, drv_qset);
}
int
drv_gex_fp_id_control(int unit, int type, int op, int flags, int *id, uint32 *count)
{
    int rv = SOC_E_NONE;
    int loop, idx = -1;
    
    if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
        if (type == _DRV_FP_ID_CFP_CHAIN_ID) {
            switch(op) {
                case _DRV_FP_ID_CTRL_ALLOC:
                    /* get a free index from head */
                    for (loop = 1; loop <= CFP_CHAIN_SIZE(unit); loop++) {
                        if (!CFP_CHAIN_USED_ISSET(unit, loop)) {
                            idx = loop;
                            break;
                        }
                    }
                    if (idx == -1){
                        return SOC_E_FULL;
                    }
                    CFP_CHAIN_USED_SET(unit, idx);
                    *id = idx;
                    break;
                case _DRV_FP_ID_CTRL_FREE:
                    CFP_CHAIN_USED_CLR(unit, *id);
                    break;                
                default:
                    rv = SOC_E_UNAVAIL;
                    break;
            }
        }
    }
    
    return rv;
}

int
drv_gex_fp_tcam_parity_check(int unit, drv_fp_tcam_checksum_t *drv_fp_tcam_chksum)
{
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
    int rv;
    uint32  reg_val = 0;
    uint32 cfp_error;

    if (SOC_IS_STARFIGHTER(unit)|| SOC_IS_POLAR(unit) || 
        SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) || 
        SOC_IS_STARFIGHTER3(unit)) {
        sal_memset(drv_fp_tcam_chksum, 0, sizeof(drv_fp_tcam_checksum_t));

        rv = REG_READ_TCAM_CHKSUM_STSr(unit, &reg_val);
        SOC_IF_ERROR_RETURN(rv);
        LOG_DEBUG(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "DRV_FP: %s reg_val %x \n"),
                   FUNCTION_NAME(),reg_val));    

        rv = soc_TCAM_CHKSUM_STSr_field_get(unit, &reg_val, 
            CFP_TCAM_CHKSUM_ERRf, &cfp_error);
        SOC_IF_ERROR_RETURN(rv);

        if (cfp_error) {
            rv = soc_TCAM_CHKSUM_STSr_field_get(unit, &reg_val, 
                CFP_TCAM_CHKSUM_ADDRf, &(drv_fp_tcam_chksum->stage_ingress_addr));
            SOC_IF_ERROR_RETURN(rv);
            drv_fp_tcam_chksum->tcam_error |= 1 << (DRV_FIELD_STAGE_INGRESS);
        }
    }
#endif    

    return SOC_E_NONE;
}


int
_drv_gex_fp_policer_support_check(int unit, int stage_id, drv_policer_config_t *policer_cfg)
{
    int rv;

    if (stage_id != DRV_FIELD_STAGE_INGRESS) {
        return SOC_E_PARAM;    
    }
    switch (policer_cfg->mode) {
        case drvPolicerModeCommitted:
        case drvPolicerModePassThrough:
            rv = SOC_E_NONE;
            break;
        default:
            rv = SOC_E_PARAM;
            break;
    }
    return (rv);

}

int
_drv_gex_fp_policer_config(int unit,  int stage_id, void *entry, drv_policer_config_t *policer_cfg) {
    int rv = SOC_E_NONE;
    drv_cfp_entry_t *drv_entry;
    uint32 cir_en, bucket_size, ref_cnt;
    uint32 *meter;

    if (stage_id != DRV_FIELD_STAGE_INGRESS) {
        return SOC_E_PARAM;
    }

    drv_entry = (drv_cfp_entry_t *)entry;
    drv_entry->pl_cfg = policer_cfg;

    switch(policer_cfg->mode){
        case drvPolicerModeCommitted:
            cir_en = 1;
            break;
        case drvPolicerModePassThrough:
            cir_en = 0;
            break;
        default:
            return SOC_E_PARAM;
    }
    meter = drv_entry->meter_data;
    sal_memset(meter, 0, sizeof(drv_entry->meter_data));

    DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_RATE_REFRESH_EN, drv_entry, &cir_en);
    if (cir_en){
        rv = DRV_CFP_METER_RATE_TRANSFORM(unit, policer_cfg->ckbits_sec, 
            policer_cfg->ckbits_burst, &bucket_size, &ref_cnt, NULL);
        SOC_IF_ERROR_RETURN(rv);
        DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_REF_CAP, drv_entry, &bucket_size);
        DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_RATE, drv_entry, &ref_cnt);
    }
    
    return SOC_E_NONE;
}



int
drv_gex_fp_policer_control(int unit,  int stage_id, int op, void *entry, drv_policer_config_t *policer_cfg) {

    int rv = SOC_E_NONE;
    drv_cfp_entry_t *drv_entry;

    switch (op) {
        case DRV_FIELD_POLICER_MODE_SUPPORT:
            rv = _drv_gex_fp_policer_support_check(unit, stage_id, policer_cfg);
            break;
        case DRV_FIELD_POLICER_CONFIG:
            rv =  _drv_gex_fp_policer_config(unit, stage_id, entry, policer_cfg);
            break;                
        case DRV_FIELD_POLICER_FREE:
            drv_entry = (drv_cfp_entry_t *)entry;
            sal_memset(drv_entry->meter_data, 0, sizeof(drv_entry->meter_data));
            rv =  SOC_E_NONE;
            break;                
    }
    return rv;
}

int
_drv_gex_fp_counter_mode_support(int unit, int param0, void *mode) {
    int policer_valid;

    policer_valid = *((int *)mode);

    if (policer_valid) {
        if (param0 == DRV_FIELD_COUNTER_MODE_RED_NOTRED){
            return SOC_E_NONE;
        }
    } else {
        if (param0 == DRV_FIELD_COUNTER_MODE_YES_NO){
            if (SOC_IS_VULCAN(unit) ||
                SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) ||
                SOC_IS_NORTHSTAR(unit) || SOC_IS_STARFIGHTER3(unit)) {
                return SOC_E_NONE;
            }
        }
    }
    return SOC_E_UNAVAIL;
}

int
drv_gex_fp_stat_type_get(int unit, int stage_id, drv_policer_mode_t policer_mode,
        drv_field_stat_t stat, int *type1, int *type2, int *type3)
{
    *type1 = -1;
    *type2 = -1;
    *type3 = -1;
    if(drvPolicerModeCount ==  policer_mode) {
        switch (stat) {
            case drvFieldStatCount:
            case drvFieldStatPackets:
                *type1 = DRV_CFP_STAT_GREEN;
                *type2 = DRV_CFP_STAT_RED;
                break;
            case drvFieldStatGreenPackets:
                /*
                  * 5348/5347/5395/53115/53125:
                  * If meter is not enabled, number of rule hit is count 
                  * at out-band counter.
                  *
                  * 53242/53262:
                  * If meter is not enabled, number of rule hit is count
                  * at in-band counter. But in SDK-30542, the meter 
                  * enable bit is turned on at driver init stage and remains
                  * during the run time, because it is a global control bit
                  * but not a per-rule bit in 53242.
                  * Thus the behavior of "meter not set" of each CFP rule,
                  * the count is at out-band counter since the meter enable
                  * bit is always on.
                  */
                *type1 = DRV_CFP_STAT_RED;
                break;
            case drvFieldStatRedPackets:
                /* See comment at switch-case drvFieldStatGreenPackets*/
                *type1 = DRV_CFP_STAT_GREEN;
                break;
            case drvFieldStatNotGreenPackets:
                /* See comment at switch-case drvFieldStatGreenPackets*/
                *type1 = DRV_CFP_STAT_GREEN;
                break;
            case drvFieldStatNotRedPackets:
                /* See comment at switch-case drvFieldStatGreenPackets*/
                *type1 = DRV_CFP_STAT_RED;
                break;
            default:
                return SOC_E_PARAM;
        }        
    } else {
        switch (stat) {
            case drvFieldStatCount:
            case drvFieldStatPackets:
                *type1 = DRV_CFP_STAT_GREEN;
                *type2 = DRV_CFP_STAT_RED;
                break;
            case drvFieldStatGreenPackets:
                *type1 = DRV_CFP_STAT_GREEN;
                break;
            case drvFieldStatRedPackets:
                *type1 = DRV_CFP_STAT_RED;
                break;
            case drvFieldStatNotGreenPackets:
                *type1 = DRV_CFP_STAT_RED;
                break;
            case drvFieldStatNotRedPackets:
                *type1 = DRV_CFP_STAT_GREEN;
                break;
            default:
                return SOC_E_PARAM;
        }        
    }
    return SOC_E_NONE;
}


int
_drv_gex_fp_stat_mode_support(int unit, int stage_id, void *mode)    
{
    drv_field_stat_t f_stat;
    int type1, type2, type3;
    int rv;

    f_stat = *((drv_field_stat_t *)mode);
    type1 = type2 = type3 =-1;
    rv = drv_gex_fp_stat_type_get(unit, stage_id, drvFieldStatCount,
        f_stat, &type1, &type2, &type3);
    return rv;
}

int
drv_gex_fp_stat_support_check(int unit, int stage_id, int op, int param0, void *mode) {
        
    switch (op) {
        case _DRV_FP_STAT_OP_COUNTER_MODE:
            return _drv_gex_fp_counter_mode_support(unit, param0, mode);
        case _DRV_FP_STAT_OP_STAT_MODE:
            return _drv_gex_fp_stat_mode_support(unit, stage_id,mode);
    }
    return SOC_E_NONE;
}


