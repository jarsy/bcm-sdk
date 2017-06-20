/*
 * $Id: field.c,v 1.5 Broadcom SDK $
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


static int 
_drv_harrier_cfp_qual_value_set(int unit, drv_field_qualify_t qual, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = SOC_E_NONE;
    uint32      fld_index = 0;
    uint32      temp;
    uint32    dtag_mode = 0; /* dtag none */
    drv_cfp_entry_t  *drv_entry = (drv_cfp_entry_t  *)entry;

    SOC_IF_ERROR_RETURN(
        DRV_PORT_GET(unit, 0, DRV_PORT_PROP_DTAG_MODE, &dtag_mode));
    switch(qual) {
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
        case drvFieldQualifyOuterVlanId:
            if (dtag_mode == 0) {
                fld_index = DRV_CFP_FIELD_USR_VID;
            } else {
                fld_index = DRV_CFP_FIELD_SP_VID;
            }
            break;
        case drvFieldQualifyOuterVlanCfi:
            if (dtag_mode == 0) {
                fld_index = DRV_CFP_FIELD_USR_CFI;
            } else {
                fld_index = DRV_CFP_FIELD_SP_CFI;
            }
            break;
        case drvFieldQualifyOuterVlanPri:
            if (dtag_mode == 0) {
                fld_index = DRV_CFP_FIELD_USR_PRI;
            } else {
                fld_index = DRV_CFP_FIELD_SP_PRI;
            }
            break;
        case drvFieldQualifyInnerVlanId:
            fld_index = DRV_CFP_FIELD_USR_VID;
            break;
        case drvFieldQualifyInnerVlanCfi:
            fld_index = DRV_CFP_FIELD_USR_CFI;
            break;
        case drvFieldQualifyInnerVlanPri:
            fld_index = DRV_CFP_FIELD_USR_PRI;
            break;

        case drvFieldQualifyRangeCheck:
            fld_index = DRV_CFP_FIELD_L4SRC_LESS1024;
            break;
        case drvFieldQualifyL4SrcPort:
            fld_index = DRV_CFP_FIELD_L4SRC;
            break;
        case drvFieldQualifyL4DstPort:
            fld_index = DRV_CFP_FIELD_L4DST;
            break;
        case drvFieldQualifyEtherType:
            fld_index = DRV_CFP_FIELD_ETYPE;
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
        case drvFieldQualifyTcpControl:
            fld_index = DRV_CFP_FIELD_TCP_FLAG;
            break;
        case drvFieldQualifySrcIpEqualDstIp:
            fld_index = DRV_CFP_FIELD_SAME_IP;
            break;
        case drvFieldQualifyEqualL4Port:
            fld_index = DRV_CFP_FIELD_SAME_L4PORT;
            break;
        case drvFieldQualifyTcpSequenceZero:
            fld_index = DRV_CFP_FIELD_TCP_SEQ_ZERO;
            break;
        case drvFieldQualifyTcpHeaderSize:
            fld_index = DRV_CFP_FIELD_TCP_HDR_LEN;
            break;
        case drvFieldQualifyIpType:
            fld_index = DRV_CFP_FIELD_L3_FRM_FORMAT;
            break;
        case drvFieldQualifyInPorts:
            fld_index = DRV_CFP_FIELD_IN_PBMP;
            break;
        case drvFieldQualifyIpProtocolCommon:
            fld_index = DRV_CFP_FIELD_L4_FRM_FORMAT;
            break;
        case drvFieldQualifyIp6FlowLabel:
            fld_index = DRV_CFP_FIELD_IP6_FLOW_ID;
            break;
        case drvFieldQualifySrcIp6:
            fld_index = DRV_CFP_FIELD_IP6_SA;
            break;
        case drvFieldQualifyL2Format:
            fld_index = DRV_CFP_FIELD_L2_FRM_FORMAT;
            break;
        case drvFieldQualifyBigIcmpCheck:
            fld_index = DRV_CFP_FIELD_BIG_ICMP_CHECK;
            break;
        case drvFieldQualifyIcmpTypeCode:
        case drvFieldQualifyIgmpTypeMaxRespTime:
            fld_index = DRV_CFP_FIELD_ICMPIGMP_TYPECODE;
            break;
        /* Add specific Qualifies for BCM53242 below */
    default:
        rv = SOC_E_INTERNAL;
        return rv;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "DRV_FP: %s fld_index = 0x%x\n"),
                 FUNCTION_NAME(), fld_index));
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "DRV_FP: data %x %x %x %x mask %x %x %x %x\n"),
                 p_data[0],p_data[1],p_data[2],p_data[3],
                 p_mask[0],p_mask[1],p_mask[2],p_mask[3]));
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



    /* Set L3_FRAME_FORMAT IPv4 or IPv6 */
    switch (qual) {
        case drvFieldQualifyDstIp:
        case drvFieldQualifySrcIp:
        case drvFieldQualifyDSCP:
        case drvFieldQualifyIpProtocol:
        case drvFieldQualifyTtl:
        case drvFieldQualifySrcIpEqualDstIp:
            fld_index = DRV_CFP_FIELD_L3_FRM_FORMAT;
            temp = FP_BCM53242_L3_FRM_FMT_IP4;
            SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM, fld_index,
                    drv_entry, &temp));
            temp = FP_BCM53242_L3_FRM_FMT_MASK;
            SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM_MASK, fld_index,
                    drv_entry, &temp));
            break;
        case drvFieldQualifyIp6FlowLabel:
        case drvFieldQualifySrcIp6:
            fld_index = DRV_CFP_FIELD_L3_FRM_FORMAT;
            temp = FP_BCM53242_L3_FRM_FMT_IP6;
            SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM, fld_index,
                    drv_entry, &temp));
            temp = FP_BCM53242_L3_FRM_FMT_MASK;
            SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM_MASK, fld_index,
                    drv_entry, &temp));
            break;
        default:
            break;
    }

        /* Set L4_FRAME_FORMAT TCP, UDP or ICMPIGMP */
    switch (qual) {
        case drvFieldQualifyIcmpTypeCode:
        case drvFieldQualifyIgmpTypeMaxRespTime:
        case drvFieldQualifyBigIcmpCheck:
            fld_index = DRV_CFP_FIELD_L4_FRM_FORMAT;
            temp = FP_BCM53242_L4_FRM_FMT_ICMPIGMP;
            SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM, fld_index,
                    drv_entry, &temp));
            temp = FP_BCM53242_L4_FRM_FMT_MASK;
            SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM_MASK, fld_index,
                    drv_entry, &temp));

            break;

        case drvFieldQualifyTcpSequenceZero:
        case drvFieldQualifyTcpControl:
        case drvFieldQualifyTcpHeaderSize:
            fld_index = DRV_CFP_FIELD_L4_FRM_FORMAT;
            temp = FP_BCM53242_L4_FRM_FMT_TCP;
            SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM, fld_index,
                    drv_entry, &temp));
            temp = FP_BCM53242_L4_FRM_FMT_MASK;
            SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
                (unit, DRV_CFP_RAM_TCAM_MASK, fld_index,
                    drv_entry, &temp));

            break;
        default : 
            break;
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
                          "DRV_FP: %s mask= 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n"),
               FUNCTION_NAME(),
               drv_entry->tcam_mask[0], drv_entry->tcam_mask[1], 
               drv_entry->tcam_mask[2], drv_entry->tcam_mask[3], 
               drv_entry->tcam_mask[4], drv_entry->tcam_mask[5]));

    return SOC_E_NONE;
}

static int 
_drv_harrier_cfp_qual_value_get(int unit, drv_field_qualify_t qual, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = SOC_E_NONE;
    uint32      fld_index = 0;
    uint32    dtag_mode = 0; /* dtag none */
    drv_cfp_entry_t  *drv_entry = (drv_cfp_entry_t  *)entry;

    SOC_IF_ERROR_RETURN(
        DRV_PORT_GET(unit, 0, DRV_PORT_PROP_DTAG_MODE, &dtag_mode));
    switch(qual) {
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
        case drvFieldQualifyOuterVlanId:
            if (dtag_mode == 0) {
                fld_index = DRV_CFP_FIELD_USR_VID;
            } else {
                fld_index = DRV_CFP_FIELD_SP_VID;
            }
            break;
        case drvFieldQualifyOuterVlanCfi:
            if (dtag_mode == 0) {
                fld_index = DRV_CFP_FIELD_USR_CFI;
            } else {
                fld_index = DRV_CFP_FIELD_SP_CFI;
            }
            break;
        case drvFieldQualifyOuterVlanPri:
            if (dtag_mode == 0) {
                fld_index = DRV_CFP_FIELD_USR_PRI;
            } else {
                fld_index = DRV_CFP_FIELD_SP_PRI;
            }
            break;
        case drvFieldQualifyInnerVlanId:
            fld_index = DRV_CFP_FIELD_USR_VID;
            break;
        case drvFieldQualifyInnerVlanCfi:
            fld_index = DRV_CFP_FIELD_USR_CFI;
            break;
        case drvFieldQualifyInnerVlanPri:
            fld_index = DRV_CFP_FIELD_USR_PRI;
            break;

        case drvFieldQualifyRangeCheck:
            fld_index = DRV_CFP_FIELD_L4SRC_LESS1024;
            break;
        case drvFieldQualifyL4SrcPort:
            fld_index = DRV_CFP_FIELD_L4SRC;
            break;
        case drvFieldQualifyL4DstPort:
            fld_index = DRV_CFP_FIELD_L4DST;
            break;
        case drvFieldQualifyEtherType:
            fld_index = DRV_CFP_FIELD_ETYPE;
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
        case drvFieldQualifyTcpControl:
            fld_index = DRV_CFP_FIELD_TCP_FLAG;
            break;
        case drvFieldQualifySrcIpEqualDstIp:
            fld_index = DRV_CFP_FIELD_SAME_IP;
            break;
        case drvFieldQualifyEqualL4Port:
            fld_index = DRV_CFP_FIELD_SAME_L4PORT;
            break;
        case drvFieldQualifyTcpSequenceZero:
            fld_index = DRV_CFP_FIELD_TCP_SEQ_ZERO;
            break;
        case drvFieldQualifyTcpHeaderSize:
            fld_index = DRV_CFP_FIELD_TCP_HDR_LEN;
            break;
        case drvFieldQualifyIpType:
            fld_index = DRV_CFP_FIELD_L3_FRM_FORMAT;
            break;
        case drvFieldQualifyInPorts:
            fld_index = DRV_CFP_FIELD_IN_PBMP;
            break;
        case drvFieldQualifyIpProtocolCommon:
            fld_index = DRV_CFP_FIELD_L4_FRM_FORMAT;
            break;
        case drvFieldQualifyIp6FlowLabel:
            fld_index = DRV_CFP_FIELD_IP6_FLOW_ID;
            break;
        case drvFieldQualifySrcIp6:
            fld_index = DRV_CFP_FIELD_IP6_SA;
            break;
        case drvFieldQualifyL2Format:
            fld_index = DRV_CFP_FIELD_L2_FRM_FORMAT;
            break;
        case drvFieldQualifyBigIcmpCheck:
            fld_index = DRV_CFP_FIELD_BIG_ICMP_CHECK;
            break;
        case drvFieldQualifyIcmpTypeCode:
        case drvFieldQualifyIgmpTypeMaxRespTime:
            fld_index = DRV_CFP_FIELD_ICMPIGMP_TYPECODE;
            break;
        /* Add specific Qualifies for BCM53242 below */
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


static int
_drv_harrier_cfp_udf_value_set(int unit, uint32 udf_idx, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    uint32      fld_index = 0;
    uint32      temp;
    int rv;
    drv_cfp_entry_t  *drv_entry = (drv_cfp_entry_t *)entry;
    
    switch(udf_idx) {
        case 0:
            fld_index = DRV_CFP_FIELD_UDFA0;
            break;
        case 1:
            fld_index = DRV_CFP_FIELD_UDFA1;
            break;
        case 2:
            fld_index = DRV_CFP_FIELD_UDFA2;
            break;
        case 3:
            fld_index = DRV_CFP_FIELD_UDFB0;
            break;
        case 4:
            fld_index = DRV_CFP_FIELD_UDFB1;
            break;
        case 5:
            fld_index = DRV_CFP_FIELD_UDFB2;
            break;
        case 6:
            fld_index = DRV_CFP_FIELD_UDFB3;
            break;
        case 7:
            fld_index = DRV_CFP_FIELD_UDFB4;
            break;
        case 8:
            fld_index = DRV_CFP_FIELD_UDFB5;
            break;
        case 9:
            fld_index = DRV_CFP_FIELD_UDFB6;
            break;
        case 10:
            fld_index = DRV_CFP_FIELD_UDFB7;
            break;
        case 11:
            fld_index = DRV_CFP_FIELD_UDFB8;
            break;
        case 12:
            fld_index = DRV_CFP_FIELD_UDFB9;
            break;
        case 13:
            fld_index = DRV_CFP_FIELD_UDFB10;
            break;
        case 14:
            fld_index = DRV_CFP_FIELD_UDFC0;
            break;
        case 15:
            fld_index = DRV_CFP_FIELD_UDFC1;
            break;
        case 16:
            fld_index = DRV_CFP_FIELD_UDFC2;
            break;
        case 17:
            fld_index = DRV_CFP_FIELD_UDFD0;
            break;
        case 18:
            fld_index = DRV_CFP_FIELD_UDFD1;
            break;
        case 19:
            fld_index = DRV_CFP_FIELD_UDFD2;
            break;
        case 20:
            fld_index = DRV_CFP_FIELD_UDFD3;
            break;
        case 21:
            fld_index = DRV_CFP_FIELD_UDFD4;
            break;
        case 22:
            fld_index = DRV_CFP_FIELD_UDFD5;
            break;
        case 23:
            fld_index = DRV_CFP_FIELD_UDFD6;
            break;
        case 24:
            fld_index = DRV_CFP_FIELD_UDFD7;
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

    /*
     * Enable related UDF valid bits of BCM53242 extended UDFs.
     * (drvFieldQualifyUserDefined10~24)
     */
    if (udf_idx < 25) {
            switch (udf_idx) {
                case 0:
                    fld_index = DRV_CFP_FIELD_UDFA0_VALID;
                    break;
                case 1:
                    fld_index = DRV_CFP_FIELD_UDFA1_VALID;
                    break;
                case 2:
                    fld_index = DRV_CFP_FIELD_UDFA2_VALID;
                    break;
                case 3:
                    fld_index = DRV_CFP_FIELD_UDFB0_VALID;
                    break;
                case 4:
                    fld_index = DRV_CFP_FIELD_UDFB1_VALID;
                    break;
                case 5:
                    fld_index = DRV_CFP_FIELD_UDFB2_VALID;
                    break;
                case 6:
                    fld_index = DRV_CFP_FIELD_UDFB3_VALID;
                    break;
                case 7:
                    fld_index = DRV_CFP_FIELD_UDFB4_VALID;
                    break;
                case 8:
                    fld_index = DRV_CFP_FIELD_UDFB5_VALID;
                    break;
                case 9:
                    fld_index = DRV_CFP_FIELD_UDFB6_VALID;
                    break;
                case 10:
                    fld_index = DRV_CFP_FIELD_UDFB7_VALID;
                    break;
                case 11:
                    fld_index = DRV_CFP_FIELD_UDFB8_VALID;
                    break;
                case 12:
                    fld_index = DRV_CFP_FIELD_UDFB9_VALID;
                    break;
                case 13:
                    fld_index = DRV_CFP_FIELD_UDFB10_VALID;
                    break;
                case 14:
                    fld_index = DRV_CFP_FIELD_UDFC0_VALID;
                    break;
                case 15:
                    fld_index = DRV_CFP_FIELD_UDFC1_VALID;
                    break;
                case 16:
                    fld_index = DRV_CFP_FIELD_UDFC2_VALID;
                    break;
                case 17:
                    fld_index = DRV_CFP_FIELD_UDFD0_VALID;
                    break;
                case 18:
                    fld_index = DRV_CFP_FIELD_UDFD1_VALID;
                    break;
                case 19:
                    fld_index = DRV_CFP_FIELD_UDFD2_VALID;
                    break;
                case 20:
                    fld_index = DRV_CFP_FIELD_UDFD3_VALID;
                    break;
                case 21:
                    fld_index = DRV_CFP_FIELD_UDFD4_VALID;
                    break;
                case 22:
                    fld_index = DRV_CFP_FIELD_UDFD5_VALID;
                    break;
                case 23:
                    fld_index = DRV_CFP_FIELD_UDFD6_VALID;
                    break;
                case 24:
                    fld_index = DRV_CFP_FIELD_UDFD7_VALID;
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
_drv_harrier_cfp_udf_value_get(int unit, uint32 udf_idx, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    uint32      fld_index = 0;
    uint32      temp = 0, temp_mask = 0;
    int rv = SOC_E_NONE;
    drv_cfp_entry_t  *drv_entry = (drv_cfp_entry_t *)entry;

    /* Check the valid bit first */
    if (udf_idx < 25) {
            switch (udf_idx) {
                case 0:
                    fld_index = DRV_CFP_FIELD_UDFA0_VALID;
                    break;
                case 1:
                    fld_index = DRV_CFP_FIELD_UDFA1_VALID;
                    break;
                case 2:
                    fld_index = DRV_CFP_FIELD_UDFA2_VALID;
                    break;
                case 3:
                    fld_index = DRV_CFP_FIELD_UDFB0_VALID;
                    break;
                case 4:
                    fld_index = DRV_CFP_FIELD_UDFB1_VALID;
                    break;
                case 5:
                    fld_index = DRV_CFP_FIELD_UDFB2_VALID;
                    break;
                case 6:
                    fld_index = DRV_CFP_FIELD_UDFB3_VALID;
                    break;
                case 7:
                    fld_index = DRV_CFP_FIELD_UDFB4_VALID;
                    break;
                case 8:
                    fld_index = DRV_CFP_FIELD_UDFB5_VALID;
                    break;
                case 9:
                    fld_index = DRV_CFP_FIELD_UDFB6_VALID;
                    break;
                case 10:
                    fld_index = DRV_CFP_FIELD_UDFB7_VALID;
                    break;
                case 11:
                    fld_index = DRV_CFP_FIELD_UDFB8_VALID;
                    break;
                case 12:
                    fld_index = DRV_CFP_FIELD_UDFB9_VALID;
                    break;
                case 13:
                    fld_index = DRV_CFP_FIELD_UDFB10_VALID;
                    break;
                case 14:
                    fld_index = DRV_CFP_FIELD_UDFC0_VALID;
                    break;
                case 15:
                    fld_index = DRV_CFP_FIELD_UDFC1_VALID;
                    break;
                case 16:
                    fld_index = DRV_CFP_FIELD_UDFC2_VALID;
                    break;
                case 17:
                    fld_index = DRV_CFP_FIELD_UDFD0_VALID;
                    break;
                case 18:
                    fld_index = DRV_CFP_FIELD_UDFD1_VALID;
                    break;
                case 19:
                    fld_index = DRV_CFP_FIELD_UDFD2_VALID;
                    break;
                case 20:
                    fld_index = DRV_CFP_FIELD_UDFD3_VALID;
                    break;
                case 21:
                    fld_index = DRV_CFP_FIELD_UDFD4_VALID;
                    break;
                case 22:
                    fld_index = DRV_CFP_FIELD_UDFD5_VALID;
                    break;
                case 23:
                    fld_index = DRV_CFP_FIELD_UDFD6_VALID;
                    break;
                case 24:
                    fld_index = DRV_CFP_FIELD_UDFD7_VALID;
                    break;
            }
    
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

    /* Get UDF data and mask */
    switch(udf_idx) {
        case 0:
            fld_index = DRV_CFP_FIELD_UDFA0;
            break;
        case 1:
            fld_index = DRV_CFP_FIELD_UDFA1;
            break;
        case 2:
            fld_index = DRV_CFP_FIELD_UDFA2;
            break;
        case 3:
            fld_index = DRV_CFP_FIELD_UDFB0;
            break;
        case 4:
            fld_index = DRV_CFP_FIELD_UDFB1;
            break;
        case 5:
            fld_index = DRV_CFP_FIELD_UDFB2;
            break;
        case 6:
            fld_index = DRV_CFP_FIELD_UDFB3;
            break;
        case 7:
            fld_index = DRV_CFP_FIELD_UDFB4;
            break;
        case 8:
            fld_index = DRV_CFP_FIELD_UDFB5;
            break;
        case 9:
            fld_index = DRV_CFP_FIELD_UDFB6;
            break;
        case 10:
            fld_index = DRV_CFP_FIELD_UDFB7;
            break;
        case 11:
            fld_index = DRV_CFP_FIELD_UDFB8;
            break;
        case 12:
            fld_index = DRV_CFP_FIELD_UDFB9;
            break;
        case 13:
            fld_index = DRV_CFP_FIELD_UDFB10;
            break;
        case 14:
            fld_index = DRV_CFP_FIELD_UDFC0;
            break;
        case 15:
            fld_index = DRV_CFP_FIELD_UDFC1;
            break;
        case 16:
            fld_index = DRV_CFP_FIELD_UDFC2;
            break;
        case 17:
            fld_index = DRV_CFP_FIELD_UDFD0;
            break;
        case 18:
            fld_index = DRV_CFP_FIELD_UDFD1;
            break;
        case 19:
            fld_index = DRV_CFP_FIELD_UDFD2;
            break;
        case 20:
            fld_index = DRV_CFP_FIELD_UDFD3;
            break;
        case 21:
            fld_index = DRV_CFP_FIELD_UDFD4;
            break;
        case 22:
            fld_index = DRV_CFP_FIELD_UDFD5;
            break;
        case 23:
            fld_index = DRV_CFP_FIELD_UDFD6;
            break;
        case 24:
            fld_index = DRV_CFP_FIELD_UDFD7;
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

/* Translate to  driver qualifier set */
int
_drv_harrier_fp_qset_to_cfp(int unit, drv_field_qset_t qset, drv_cfp_entry_t * drv_entry)
{
    int                 retval = SOC_E_UNAVAIL;
    uint32    dtag_mode = 0; /* dtag none */

    if (SHR_BITGET(qset.udf_map, 0)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFA0, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 1)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFA1, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 2)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFA2, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 3)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFB0, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 4)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFB1, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 5)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFB2, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 6)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFB3, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 7)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFB4, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 8)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFB5, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);            
    }
    if (SHR_BITGET(qset.udf_map, 9)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFB6, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 10)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFB7, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 11)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFB8, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 12)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFB9, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 13)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFB10, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }  
    if (SHR_BITGET(qset.udf_map, 14)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFC0, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 15)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFC1, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 16)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFC2, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 17)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFD0, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 18)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFD1, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 19)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFD2, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }   
    if (SHR_BITGET(qset.udf_map, 20)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFD3, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 21)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFD4, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 22)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFD5, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }   
    if (SHR_BITGET(qset.udf_map, 23)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFD6, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (SHR_BITGET(qset.udf_map, 24)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_UDFD7, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
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
    
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyInPort)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_SRC_PBMP, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
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
    }

    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyIpProtocolCommon)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_L4_FRM_FORMAT, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }

    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyIpType)){
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_L3_FRM_FORMAT, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
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
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_L3_FRM_FORMAT, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyIp6)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_L3_FRM_FORMAT, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
    }
    if (DRV_FIELD_QSET_TEST(qset, drvFieldQualifyL2Format)) {
        retval = DRV_CFP_QSET_SET
            (unit, DRV_CFP_QUAL_L2_FRM_FORMAT, drv_entry, 1);
        SOC_IF_ERROR_RETURN(retval);
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

STATIC int
_drv_harrier_cfp_qset_mapping(drv_cfp_entry_t *entry, drv_field_qset_t *qset,
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
STATIC int
_drv_harrier_fp_udf_mapping(drv_cfp_entry_t *entry, drv_field_qset_t *qset,
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
_drv_harrier_cfp_selcode_to_qset(int unit, 
    int slice_id, drv_field_qset_t *qset)
{
    int     rv = SOC_E_NONE;
    drv_cfp_entry_t drv_entry;
    uint32 dtag_mode = 0; /* mode none */

    sal_memset(&drv_entry, 0, sizeof(drv_cfp_entry_t));
    
     rv = DRV_CFP_SLICE_TO_QSET(unit, slice_id, &drv_entry);

    DRV_PORT_GET(unit, 0, DRV_PORT_PROP_DTAG_MODE, &dtag_mode);

    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_SRC_PORT, drvFieldQualifySrcPort));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_SRC_PORT, drvFieldQualifyInPort));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_1QTAG, drvFieldQualifyVlanFormat));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_SPTAG, drvFieldQualifyVlanFormat));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_MAC_DA, drvFieldQualifyDstMac));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_MAC_SA, drvFieldQualifySrcMac));
    if (dtag_mode == 0 ) {
        SOC_IF_ERROR_RETURN(
        _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_USR_VID, drvFieldQualifyOuterVlanId));
        SOC_IF_ERROR_RETURN(
        _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_USR_PRI, drvFieldQualifyOuterVlanPri));
        SOC_IF_ERROR_RETURN(
        _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_USR_CFI, drvFieldQualifyOuterVlanCfi));
    } else {
        SOC_IF_ERROR_RETURN(
        _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_SP_VID, drvFieldQualifyOuterVlanId));
        SOC_IF_ERROR_RETURN(
        _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_SP_PRI, drvFieldQualifyOuterVlanPri));
        SOC_IF_ERROR_RETURN(
        _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_SP_CFI, drvFieldQualifyOuterVlanCfi));
        SOC_IF_ERROR_RETURN(
        _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_USR_VID, drvFieldQualifyInnerVlanId));
        SOC_IF_ERROR_RETURN(
        _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_USR_PRI, drvFieldQualifyInnerVlanPri));
        SOC_IF_ERROR_RETURN(
        _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
            DRV_CFP_QUAL_USR_CFI, drvFieldQualifyInnerVlanCfi));
    }
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_ETYPE, drvFieldQualifyEtherType));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP_DA, drvFieldQualifyDstIp));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP_SA, drvFieldQualifySrcIp));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP_SAME, drvFieldQualifySrcIpEqualDstIp));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IPV4, drvFieldQualifyIp4));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_L4_DST, drvFieldQualifyL4DstPort));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_L4_SRC, drvFieldQualifyL4SrcPort));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_L4SRC_LESS1024, drvFieldQualifyRangeCheck));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_L4_SAME, drvFieldQualifyEqualL4Port));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_TCP_SEQ_ZERO, drvFieldQualifyTcpSequenceZero));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_TCP_HDR_LEN, drvFieldQualifyTcpHeaderSize));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_TCP_FLAG, drvFieldQualifyTcpControl));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP_PROTO, drvFieldQualifyIpProtocol));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP_TOS, drvFieldQualifyDSCP));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP_TTL, drvFieldQualifyTtl));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_SRC_PBMP, drvFieldQualifyInPorts));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_SRC_PBMP, drvFieldQualifyInPort));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_L2_FRM_FORMAT, drvFieldQualifyL2Format));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_L3_FRM_FORMAT, drvFieldQualifyIpType));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_L4_FRM_FORMAT, drvFieldQualifyIpProtocolCommon));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_VLAN_RANGE, drvFieldQualifyRangeCheck));    
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP6_FLOW_ID, drvFieldQualifyIp6FlowLabel));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP6_SA, drvFieldQualifySrcIp6));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP6_DA, drvFieldQualifyDstIp6));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IPV6, drvFieldQualifyIp6));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP_FRGA, drvFieldQualifyIpFrag));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_IP_AUTH, drvFieldQualifyIpAuth));

    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_ICMPIGMP_TYPECODE, drvFieldQualifyIcmpTypeCode));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_ICMPIGMP_TYPECODE, drvFieldQualifyIgmpTypeMaxRespTime));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_cfp_qset_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_BIG_ICMP_CHECK, drvFieldQualifyBigIcmpCheck));

    if (DRV_FIELD_QSET_TEST(*qset, drvFieldQualifyL2Format) &&
        DRV_FIELD_QSET_TEST(*qset, drvFieldQualifyIp4) && 
        DRV_FIELD_QSET_TEST(*qset, drvFieldQualifyIp6)) {
        DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyPacketFormat);
    }

    DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyStage);
    DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyStageIngress);

    /* Add BCM53242 new qualify sets here*/
    /* Add BCM53242 new qualify sets */
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFA0, 0));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFA1, 1));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFA2, 2));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFB0, 3));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFB1, 4));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFB2, 5));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFB3, 6));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFB4, 7));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFB5, 8));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFB6, 9));
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFB7, 10));            
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFB8, 11));  
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFB9, 12));                          
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFB10, 13));            
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFC0, 14));  
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFC1, 15));  
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFC2, 16));            
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFD0, 17));  
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFD1, 18));  
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFD2, 19));            
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFD3, 20));  
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFD4, 21));                                      
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFD5, 22));            
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFD6, 23));  
    SOC_IF_ERROR_RETURN(
    _drv_harrier_fp_udf_mapping(&drv_entry, qset, 
        DRV_CFP_QUAL_UDFD7, 24));                                        

    return rv;
}


/* allocate drv_entry for CFP driver */
int
_drv_harrier_fp_entry_alloc(int unit, int stage_id, void **drv_entry)
{
    int memsize;
   
    memsize = sizeof(drv_cfp_entry_t);            
    *drv_entry = sal_alloc(memsize, "field drv entry");

    if (*drv_entry == NULL) {
       return SOC_E_MEMORY;
    }
    sal_memset (*drv_entry, 0, memsize);
    return SOC_E_NONE;
}

/* copy drv_entry from src to dest*/
int
_drv_harrier_fp_entry_copy(int unit, int stage_id, void *src_entry, void *dst_entry )
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
_drv_harrier_fp_entry_clear(int unit, int stage_id, void* entry, int op)
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
_drv_harrier_fp_entry_tcam_slice_id_set(int unit, int stage_id, void *entry, int sliceId)
{

    uint32 temp;
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;

    temp = sliceId & 0x7;
    SOC_IF_ERROR_RETURN(
        DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_SLICE_ID, 
            drv_entry, &temp));

    temp = 0x7;
    SOC_IF_ERROR_RETURN(
        DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_SLICE_ID, 
            drv_entry, &temp));

    drv_entry->slice_id = sliceId;
    return SOC_E_NONE;
}


/* move drv_entry(TCAM) only or both TCAM&Counter  with amount*/
int
_drv_harrier_fp_entry_tcam_move(int unit, int stage_id, void *entry, int amount, 
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
    assert(tcam_idx_old <= tcam_idx_max);
    assert(tcam_idx_new <= tcam_idx_max);

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
_drv_harrier_fp_entry_tcam_valid_control(int unit, int stage_id, void *entry, int tcam_idx, uint32 *valid)
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
_drv_harrier_fp_entry_tcam_policy_install(int unit, int stage_id, void *entry, int tcam_idx,
    int *tcam_chain_idx)
{
    uint32 temp;
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    int retval;   

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

    return retval;
}
int
_drv_harrier_fp_entry_tcam_meter_install(int unit, int stage_id, void *entry, 
        int tcam_idx, int *tcam_chain_idx)
{
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    int retval;

    retval = DRV_CFP_ENTRY_WRITE
        (unit, tcam_idx, DRV_CFP_RAM_METER, drv_entry);
    if (SOC_FAILURE(retval)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "FP Error: Write Tcam entry index = %d fail.\n"),  
                   tcam_idx));
        return retval;
    }
    return retval;
}

int
_drv_harrier_fp_qualify_support(int unit, drv_field_qset_t qset)
{
    int rv = SOC_E_NONE;
    int i;

    /* Qualifier checking */
    for (i = 0; i < drvFieldQualifyCount; i++) {
        if (DRV_FIELD_QSET_TEST(qset, i)) {

            switch (i) {
                case drvFieldQualifySrcIp6:
                case drvFieldQualifySrcMac:
                case drvFieldQualifyDstMac:
                case drvFieldQualifySrcIp:
                case drvFieldQualifyDstIp:
                case drvFieldQualifyInPort:
                case drvFieldQualifyInPorts:
                case drvFieldQualifyIp6FlowLabel:
                case drvFieldQualifyOuterVlan:
                case drvFieldQualifyOuterVlanId:
                case drvFieldQualifyOuterVlanPri:
                case drvFieldQualifyOuterVlanCfi:
                case drvFieldQualifyInnerVlan:
                case drvFieldQualifyInnerVlanId:
                case drvFieldQualifyInnerVlanCfi:
                case drvFieldQualifyInnerVlanPri:
                case drvFieldQualifyRangeCheck: /* For L4 src port less than 1024 */
                case drvFieldQualifyL4SrcPort:
                case drvFieldQualifyL4DstPort:
                case drvFieldQualifyEtherType:
                case drvFieldQualifyIpProtocol: /* same as Ip6NextHeader */
                case drvFieldQualifyIpProtocolCommon:
                case drvFieldQualifyDSCP: /* same as ToS, Ip6TrafficClass */
                case drvFieldQualifyTtl: /* same as Ip6HopLimit */
                case drvFieldQualifyTcpControl:
                case drvFieldQualifyStageIngress:
                case drvFieldQualifySrcIpEqualDstIp:
                case drvFieldQualifyEqualL4Port:
                case drvFieldQualifyTcpSequenceZero:
                case drvFieldQualifyTcpHeaderSize:
                case drvFieldQualifyVlanFormat:
                case drvFieldQualifyL2Format:
                case drvFieldQualifyBigIcmpCheck:
                case drvFieldQualifyIcmpTypeCode:
                case drvFieldQualifyIgmpTypeMaxRespTime:
                case drvFieldQualifyIpType:
                case drvFieldQualifyIp4:
                case drvFieldQualifyIp6:
                case drvFieldQualifyPacketFormat:
                    break;
                default:
                    return SOC_E_UNAVAIL;
            }
        }
    }
    return rv;
}

int 
_drv_harrier_fp_entry_tcam_enable_set(int unit, int stage_id, 
        int tcam_idx, int *tcam_chain_idx, int enable)
{
    int                 rv = SOC_E_UNAVAIL;
    drv_cfp_entry_t     *cfp_entry = NULL;
    uint32      temp;

    rv = _drv_harrier_fp_entry_alloc(unit, stage_id, (void **)&cfp_entry);
    if (SOC_FAILURE(rv)) {
        return rv;
    }

    rv = DRV_CFP_ENTRY_READ
        (unit, tcam_idx, DRV_CFP_RAM_TCAM, cfp_entry);

    if (SOC_FAILURE(rv)) {
       sal_free(cfp_entry);
        return rv;
    }

    if (enable) {
        temp = 1;
    } else {
        temp = 0;
    }
    rv = _drv_harrier_fp_entry_tcam_valid_control(
        unit, stage_id, cfp_entry, tcam_idx, &temp);

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
_drv_harrier_fp_entry_tcam_clear(int unit, int stage_id, int tcam_idx, 
        int *tcam_chain_idx, int op)
{
    int                 rv = SOC_E_UNAVAIL;
    drv_cfp_entry_t     *cfp_entry = NULL;
    int mem_id;

    rv = _drv_harrier_fp_entry_alloc(unit, stage_id, (void **)&cfp_entry);
    if (SOC_FAILURE(rv)) {
        return rv;
    }
    if (op == DRV_FIELD_ENTRY_TCAM_DATAMASK_CLEAR) {
        mem_id = DRV_CFP_RAM_TCAM;
    } else {
        mem_id = DRV_CFP_RAM_ALL;
    }
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
_drv_harrier_fp_entry_tcam_reinstall(int unit, int stage_id, void *entry, int tcam_idx, 
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


int 
drv_harrier_fp_init(int unit,int stage_id)
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

    return rv;
}
int 
drv_harrier_fp_deinit(int unit,int stage_id)
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
drv_harrier_fp_qual_value_set(int unit, int stage_id, drv_field_qualify_t qual, 
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
    rv = _drv_harrier_cfp_qual_value_set 
                (unit, qual, drv_entry, p_data, p_mask);
    return rv;
}

int
drv_harrier_fp_udf_value_set(int unit, int stage_id, uint32 udf_idex, 
        void *entry, uint32* p_data, uint32* p_mask)
{
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    int rv;

    switch(stage_id) {
        case DRV_FIELD_STAGE_INGRESS:
            rv = _drv_harrier_cfp_udf_value_set 
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
drv_harrier_fp_udf_value_get(int unit, int stage_id, uint32 udf_idex, void *entry, uint32* p_data, uint32* p_mask)
{
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    int rv;

    switch(stage_id) {
        case DRV_FIELD_STAGE_INGRESS:
            rv = _drv_harrier_cfp_udf_value_get 
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
drv_harrier_fp_qual_value_get(int unit, int stage_id, drv_field_qualify_t qual, 
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
    rv = _drv_harrier_cfp_qual_value_get 
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
drv_harrier_fp_entry_mem_control (int unit, int stage_id, int op, void *src_entry, void *dst_entry, 
        void **alloc_entry)
{
    int rv = SOC_E_NONE;

    switch (op) {
        case DRV_FIELD_ENTRY_MEM_ALLOC:
            rv = _drv_harrier_fp_entry_alloc(unit, stage_id, alloc_entry);        
            break;    
        case DRV_FIELD_ENTRY_MEM_COPY:
            rv = _drv_harrier_fp_entry_copy(unit, stage_id, src_entry, dst_entry);
            break;                
        case DRV_FIELD_ENTRY_MEM_CLEAR_DATAMASK:
            rv = _drv_harrier_fp_entry_clear(unit, stage_id, src_entry, op);
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
drv_harrier_fp_entry_tcam_control(int unit, int stage_id, void* drv_entry, int op, 
        int param1, void *param2)
{
    int rv = SOC_E_NONE;

    if (DRV_FIELD_STAGE_INGRESS != stage_id)
        return SOC_E_PARAM;

    switch (op) {
        case DRV_FIELD_ENTRY_TCAM_SLICE_ID_SET:
            rv = _drv_harrier_fp_entry_tcam_slice_id_set(unit, stage_id, drv_entry, param1);
            break;
        case DRV_FIELD_ENTRY_TCAM_MOVE:
            rv = _drv_harrier_fp_entry_tcam_move(unit, stage_id, drv_entry, param1, param2);
            break;
        case DRV_FIELD_ENTRY_TCAM_REMOVE:
            rv = _drv_harrier_fp_entry_tcam_enable_set(unit, stage_id, param1, param2, FALSE);
            break;
        case DRV_FIELD_ENTRY_TCAM_ENABLE:
            rv = _drv_harrier_fp_entry_tcam_enable_set(unit, stage_id, param1, param2, TRUE);
            break;
        case DRV_FIELD_ENTRY_TCAM_CLEAR:
        case DRV_FIELD_ENTRY_TCAM_DATAMASK_CLEAR:
            rv = _drv_harrier_fp_entry_tcam_clear(unit, stage_id, param1, param2, op);
            break;
        case DRV_FIELD_ENTRY_TCAM_POLICY_INSTALL:
            rv = _drv_harrier_fp_entry_tcam_policy_install(unit, stage_id, drv_entry, param1, param2);
            break;
        case DRV_FIELD_ENTRY_TCAM_ENTRY_REINSTALL:
            rv = _drv_harrier_fp_entry_tcam_reinstall(unit, stage_id, drv_entry, param1, param2);
            break;
        case DRV_FIELD_ENTRY_TCAM_METER_INSTALL:
            rv = _drv_harrier_fp_entry_tcam_meter_install(unit, stage_id, drv_entry, param1, param2);
            break;   
        case DRV_FIELD_ENTRY_TCAM_SW_INDEX_SET:   
            ((drv_cfp_entry_t *)drv_entry)->id = param1;            
            break;
        case DRV_FIELD_ENTRY_TCAM_SW_FLAGS_SET:
            if (DRV_FIELD_STAGE_INGRESS == stage_id) {
                *((uint32 *)param2) = ((drv_cfp_entry_t *)drv_entry)->flags;
            }
            break;           
    }

    return rv;
}


int
drv_harrier_fp_action_conflict(int unit, int stage_id, 
        drv_field_action_t act1, drv_field_action_t act2)
{
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
}
int
drv_harrier_fp_action_support_check(int unit, int stage_id, 
            drv_field_action_t action)
{
    int                 retval = SOC_E_NONE;

    if (DRV_FIELD_STAGE_INGRESS != stage_id) {
        return SOC_E_PARAM;
    }

    switch (action) {
        case drvFieldActionDscpNew: 
        case drvFieldActionGpDscpNew:
        case drvFieldActionRpDscpNew:
        case drvFieldActionDrop:
        case drvFieldActionGpDrop:
        case drvFieldActionRpDrop:
        case drvFieldActionCopyToCpu:
        case drvFieldActionGpCopyToCpu:
        case drvFieldActionRpCopyToCpu:
        case drvFieldActionRedirect:
        case drvFieldActionGpRedirectPort:                
        case drvFieldActionRpRedirectPort:                
        case drvFieldActionMirrorIngress:
        case drvFieldActionGpMirrorIngress:
        case drvFieldActionRpMirrorIngress:
        case drvFieldActionVlanNew:
        case drvFieldActionInnerVlanNew:
        case drvFieldActionMeterConfig:
        case drvFieldActionUpdateCounter:
        case drvFieldActionCosQNew:
        case drvFieldActionCosQCpuNew:
        case drvFieldActionPrioPktNew:                
            retval = SOC_E_NONE;
            break;
        default:
            retval = SOC_E_UNAVAIL;
            break;
    }
    return retval;
}

int
drv_harrier_fp_action_add(int unit, int stage_id, void *drv_entry, 
        drv_field_action_t action, uint32 param0, uint32 param1)
{
    uint32          temp;
    uint32          explicit_dest = 0;
    uint32 get_action = 0;

    if (DRV_FIELD_STAGE_INGRESS != stage_id)
        return SOC_E_PARAM;

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
        /*
         * "param1=0x3f" of actions redirec or mirror-to of BCM53242 
         * is a special value that represents redirect or mirror-to all
         * ports. Although it's not a valid port number.
         */
        if (!(param1 == 0x3f)) {
            return SOC_E_PARAM;
        }
    }

    switch (action) {
        case drvFieldActionPrioPktNew:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_PCP_NEW, 
                drv_entry, param0, 0));
            break;
        case drvFieldActionCopyToCpu:
            /* should use symbol */
            temp = CMIC_PORT(unit);           
             /*
             * If the port field is bitmap format,
             * translate the port number to bitmap format
             */
            /* Check if copy to mirror-to port is configured */
            explicit_dest = 0;
            get_action = DRV_CFP_ACT_OB_APPEND;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_GET
                    (unit, &get_action, drv_entry, &temp));
            if ((get_action != DRV_CFP_ACT_OB_NONE) && 
                (temp & DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE)) {
                /* 
                 * Copy to mirror-to port is already configured. 
                 * Flag to represent copy to both cpu and mirror-to port 
                 * is acceptable .
                 */
                explicit_dest = 1;
            }
            get_action = DRV_CFP_ACT_IB_APPEND;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_GET
                    (unit, &get_action, drv_entry, &temp));
            if ((get_action != DRV_CFP_ACT_IB_NONE) &&
                (temp & DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE)) {
                /* 
                 * Copy to mirror-to port is already configured. 
                 * Flag to represent copy to both cpu and mirror-to port 
                 * is acceptable .
                 */
                explicit_dest = 1;
            } else {
                explicit_dest = 0;
            }

            if (explicit_dest) {
                temp = DRV_FIELD_ACT_CHANGE_FWD_CPU;
                temp |= DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_APPEND, 
                        drv_entry, 0, temp));
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_APPEND, 
                        drv_entry, 0, temp));
            } else {
                temp = CMIC_PORT(unit);           
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_APPEND, 
                        drv_entry, temp, 0));
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_APPEND, 
                        drv_entry, temp, 0));
            }
            break;
        case drvFieldActionMirrorIngress:           
            /*
             * If the port field is bitmap format,
             * translate the port number to bitmap format
             */
            /* Check if copy to cpu port is configured */
            explicit_dest = 0;
            if (param1 == FP_ACT_CHANGE_FWD_MIRROT_TO_PORT) {
                get_action = DRV_CFP_ACT_OB_APPEND;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_GET
                        (unit, &get_action, drv_entry, &temp));
                if ((get_action != DRV_CFP_ACT_OB_NONE) && 
                    (temp & DRV_FIELD_ACT_CHANGE_FWD_CPU)) {
                    /* 
                     * Copy to cpu is already configured. 
                     * Flag to represent copy to both cpu and mirror-to port 
                     * is acceptable .
                     */
                    explicit_dest = 1;
                }
                get_action = DRV_CFP_ACT_IB_APPEND;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_GET
                        (unit, &get_action, drv_entry, &temp));
                if ((get_action != DRV_CFP_ACT_IB_NONE) &&
                    (temp & DRV_FIELD_ACT_CHANGE_FWD_CPU)) {
                    /* 
                     * Copy to cpu is already configured. 
                     * Flag to represent copy to both cpu and mirror-to port 
                     * is acceptable .
                     */
                    explicit_dest = 1;
                } else {
                    explicit_dest = 0;
                }
            }
            
            if (explicit_dest) {
                temp = DRV_FIELD_ACT_CHANGE_FWD_CPU;
                temp |= DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_APPEND, 
                        drv_entry, 0, temp));
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_APPEND, 
                        drv_entry, 0, temp));
            } else if (param1 == FP_ACT_CHANGE_FWD_MIRROT_TO_PORT) {
                temp = DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_APPEND, 
                        drv_entry, 0, temp));
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_APPEND, 
                        drv_entry, 0, temp));
            } else {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_APPEND, 
                        drv_entry, param1, 0));
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_APPEND, 
                        drv_entry, param1, 0));
            }
            break;
        case drvFieldActionRedirect:
            /*
             * If the port field is bitmap format,
             * translate the port number to bitmap format
             */
            explicit_dest = 0;
            get_action = DRV_CFP_ACT_OB_REDIRECT;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_GET
                    (unit, &get_action, drv_entry, &temp));
            if ((param1 == CMIC_PORT(unit)) &&
                (get_action != DRV_CFP_ACT_OB_NONE) &&
                (temp & DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE)) {
                explicit_dest = 1;
            } else if ((param1 == FP_ACT_CHANGE_FWD_MIRROT_TO_PORT) &&
                (get_action != DRV_CFP_ACT_OB_NONE) &&
                (temp & DRV_FIELD_ACT_CHANGE_FWD_CPU)) {
                explicit_dest = 1;
            }

            get_action = DRV_CFP_ACT_IB_REDIRECT;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_GET
                    (unit, &get_action, drv_entry, &temp));
            if ((param1 == CMIC_PORT(unit)) &&
                (get_action != DRV_CFP_ACT_IB_NONE) &&
                (temp & DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE)) {
                explicit_dest = 1;
            } else if ((param1 == FP_ACT_CHANGE_FWD_MIRROT_TO_PORT) &&
                (get_action != DRV_CFP_ACT_IB_NONE) &&
                (temp & DRV_FIELD_ACT_CHANGE_FWD_CPU)) {
                explicit_dest = 1;
            } else {
                explicit_dest = 0;
            }

            if (explicit_dest) {
                /* Redirect to both cpu and mirror-to port */
                temp = DRV_FIELD_ACT_CHANGE_FWD_CPU;
                temp |= DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_REDIRECT, 
                        drv_entry, 0, temp));
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_REDIRECT, 
                        drv_entry, 0, temp));
            } else if (param1 == FP_ACT_CHANGE_FWD_MIRROT_TO_PORT) { 
                temp = DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_REDIRECT, 
                        drv_entry, 0, temp));
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_REDIRECT, 
                        drv_entry, 0, temp));
            } else {
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_REDIRECT, 
                        drv_entry, param1, 0));
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_REDIRECT, 
                        drv_entry, param1, 0));
            }
            break;
        case drvFieldActionDrop:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_OB_DROP, 
                    drv_entry, 0, 0));
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_IB_DROP, 
                drv_entry, 0, 0));
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
            temp = CMIC_PORT(unit);
            /*
             * If the port field is bitmap format,
             * translate the port number to bitmap format
             */
            /* Check if copy to mirror-to port is configured */
            explicit_dest = 0;
            get_action = DRV_CFP_ACT_OB_APPEND;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_GET
                    (unit, &get_action, drv_entry, &temp));
            if ((get_action != DRV_CFP_ACT_OB_NONE) &&
                (temp & DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE)) {
                /* 
                 * Copy to mirror-to port is already configured. 
                 * Flag to represent copy to both cpu and mirror-to port 
                 * is acceptable .
                 */
                explicit_dest = 1;
            }
            
            if (explicit_dest) {
                temp = DRV_FIELD_ACT_CHANGE_FWD_CPU;
                temp |= DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_APPEND, 
                        drv_entry, 0, temp));
            } else {
                temp = CMIC_PORT(unit);
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_APPEND, 
                        drv_entry, temp, 0));
            }           
            break;
        case drvFieldActionRpDrop:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_OB_DROP, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionRpDscpNew:
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
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_IB_APPEND, 
                    drv_entry, temp, 0));
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
             * param1 carries the desired port bitmap.
             */
            if (param1 == 0) {
                temp = SOC_PBMP_WORD_GET(PBMP_E_ALL(unit), 0);
            } else {
                temp = param1;
            }
            SOC_IF_ERROR_RETURN(DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_CHANGE_CVID, 
                drv_entry, param0, temp));
            break;
        case drvFieldActionOuterVlanNew:
            /*
             * param0 carries the value of new vid.
             * param1 carries the desired port bitmap.
             */
            if (param1 == 0) {
                temp = SOC_PBMP_WORD_GET(PBMP_E_ALL(unit), 0);
            } else {
                temp = param1;
            }
            SOC_IF_ERROR_RETURN(DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_CHANGE_SPVID, 
                drv_entry, param0, temp));
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
            /*
             * If the port field is bitmap format,
             * translate the port number to bitmap format
             */
            explicit_dest = 0;
            get_action = DRV_CFP_ACT_OB_REDIRECT;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_GET
                    (unit, &get_action, drv_entry, &temp));
            
            if ((param1 == CMIC_PORT(unit)) &&
                (get_action != DRV_CFP_ACT_OB_NONE) &&
                (temp & DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE)) {
                explicit_dest = 1;
            } else if ((param1 == FP_ACT_CHANGE_FWD_MIRROT_TO_PORT) &&
                (get_action != DRV_CFP_ACT_OB_NONE) &&
                (temp & DRV_FIELD_ACT_CHANGE_FWD_CPU)) {
                explicit_dest = 1;
            }
            
            if (explicit_dest) {
                /* Redirect to both cpu and mirror-to port */
                temp = DRV_FIELD_ACT_CHANGE_FWD_CPU;
                temp |= DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_REDIRECT, 
                        drv_entry, 0, temp));
            } else if (param1 == FP_ACT_CHANGE_FWD_MIRROT_TO_PORT) { 
                temp = DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_REDIRECT, 
                        drv_entry, 0, temp));
            } else {
                SOC_IF_ERROR_RETURN(                
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_REDIRECT, 
                        drv_entry, param1, 0));
            }
            break;
        case drvFieldActionRpMirrorIngress:           
            /*
             * If the port field is bitmap format,
             * translate the port number to bitmap format
             */
            /* Check if copy to cpu port is configured */
            explicit_dest = 0;
            if (param1 == FP_ACT_CHANGE_FWD_MIRROT_TO_PORT) {
                get_action = DRV_CFP_ACT_OB_APPEND;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_GET
                        (unit, &get_action, drv_entry, &temp));
                if ((get_action != DRV_CFP_ACT_OB_NONE) &&
                    (temp & DRV_FIELD_ACT_CHANGE_FWD_CPU)) {
                    /* 
                     * Copy to cpu is already configured. 
                     * Flag to represent copy to both cpu and mirror-to port 
                     * is acceptable .
                     */
                    explicit_dest = 1;
                }
            }
            
            if (explicit_dest) {
                temp = DRV_FIELD_ACT_CHANGE_FWD_CPU;
                temp |= DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_APPEND, 
                        drv_entry, 0, temp));
            } else if (param1 == FP_ACT_CHANGE_FWD_MIRROT_TO_PORT) {
                temp = DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_OB_APPEND, 
                        drv_entry, 0, temp));
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
            explicit_dest = 0;
            get_action = DRV_CFP_ACT_IB_REDIRECT;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_GET
                    (unit, &get_action, drv_entry, &temp));
            
            if ((param1 == CMIC_PORT(unit)) &&
                (get_action != DRV_CFP_ACT_IB_NONE) &&
                (temp & DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE)) {
                explicit_dest = 1;
            } else if ((param1 == FP_ACT_CHANGE_FWD_MIRROT_TO_PORT) &&
                (get_action != DRV_CFP_ACT_IB_NONE) &&
                (temp & DRV_FIELD_ACT_CHANGE_FWD_CPU)) {
                explicit_dest = 1;
            }
            
            if (explicit_dest) {
                /* Redirect to both cpu and mirror-to port */
                temp = DRV_FIELD_ACT_CHANGE_FWD_CPU;
                temp |= DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_REDIRECT, 
                        drv_entry, 0, temp));
            } else if (param1 == FP_ACT_CHANGE_FWD_MIRROT_TO_PORT) { 
                temp = DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_REDIRECT, 
                        drv_entry, 0, temp));
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
            /* Check if copy to cpu port is configured */
            explicit_dest = 0;
            if (param1 == FP_ACT_CHANGE_FWD_MIRROT_TO_PORT) {
                get_action = DRV_CFP_ACT_IB_APPEND;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_GET
                        (unit, &get_action, drv_entry, &temp));
                if ((get_action != DRV_CFP_ACT_IB_NONE) &&
                    (temp & DRV_FIELD_ACT_CHANGE_FWD_CPU)) {
                    /* 
                     * Copy to cpu is already configured. 
                     * Flag to represent copy to both cpu and mirror-to port 
                     * is acceptable .
                     */
                    explicit_dest = 1;
                }
            }
            
            if (explicit_dest) {
                temp = DRV_FIELD_ACT_CHANGE_FWD_CPU;
                temp |= DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_APPEND, 
                        drv_entry, 0, temp));
            } else if (param1 == FP_ACT_CHANGE_FWD_MIRROT_TO_PORT) {
                temp = DRV_FIELD_ACT_CHANGE_FWD_MIRROR_CAPTURE;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_ACTION_SET
                        (unit, DRV_CFP_ACT_IB_APPEND, 
                        drv_entry, 0, temp));
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
        default:
            return SOC_E_UNAVAIL;
    }
    return SOC_E_NONE;
}

int
drv_harrier_fp_action_remove(int unit, int stage_id, void *entry, 
        drv_field_action_t action, uint32 param0, uint32 param1)
{
    int rv = SOC_E_NONE;
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;

    if (DRV_FIELD_STAGE_INGRESS != stage_id)
        return SOC_E_PARAM;

    switch (action) {
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
        case drvFieldActionCopyToCpu:
        case drvFieldActionDrop:
        case drvFieldActionRedirectPbmp:
        case drvFieldActionRedirect:
        case drvFieldActionMirrorIngress:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                (unit, DRV_CFP_ACT_IB_NONE, 
                drv_entry, 0, 0));
            SOC_IF_ERROR_RETURN(
                DRV_CFP_ACTION_SET
                    (unit, DRV_CFP_ACT_OB_NONE, 
                    drv_entry, 0, 0));
            break;
        case drvFieldActionRpMirrorIngress:
        case drvFieldActionRpRedirectPort:
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
drv_harrier_fp_selcode_mode_get(int unit, int stage_id, 
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

    rv = _drv_harrier_fp_qset_to_cfp(unit, drv_qset, 
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

    if (SOC_FAILURE(rv)) {
        sal_free(drv_entry);
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "fail in DRV_CFP_SLICE_ID_SELECT %d\n"),
                   rv));
        return rv;
    }
    *slice_id = temp;
    *entry = drv_entry;
    *slice_map = 0;
    return rv;
}

int
drv_harrier_fp_selcode_to_qset(int unit,  int stage_id, int slice_id, 
        void *qset) 
{
    int rv;
    drv_field_qset_t *drv_qset = (drv_field_qset_t *)qset;

    if (DRV_FIELD_STAGE_INGRESS != stage_id)
        return SOC_E_PARAM;
    
    rv =_drv_harrier_cfp_selcode_to_qset(unit, slice_id,drv_qset);

    return rv;
}

int
drv_harrier_fp_qualify_support(int unit, int stage_id,  void *qset)
{
    drv_field_qset_t drv_qset;

    if (DRV_FIELD_STAGE_INGRESS != stage_id)
        return SOC_E_PARAM;

    sal_memcpy(&drv_qset, qset, sizeof(drv_field_qset_t));
    return _drv_harrier_fp_qualify_support(unit, drv_qset);
}
int
drv_harrier_fp_id_control(int unit, int type, int op, int flags, int *id, uint32 *count)
{
    return SOC_E_NONE;
}

int
_drv_harrier_fp_policer_support_check(int unit, int stage_id, drv_policer_config_t *policer_cfg)
{
    int rv;

    if (stage_id != DRV_FIELD_STAGE_INGRESS) {
        return SOC_E_PARAM;    
    }
    switch (policer_cfg->mode) {
        case drvPolicerModeCommitted:
        case drvPolicerModePassThrough:
        case drvPolicerModeGreen:
            rv = SOC_E_NONE;
            break;
        default:
            rv = SOC_E_PARAM;
            break;
    }
    return (rv);

}

int
_drv_harrier_fp_policer_config(int unit,  int stage_id, void *entry, drv_policer_config_t *policer_cfg) {
    int rv = SOC_E_NONE;
    drv_cfp_entry_t *drv_entry;
    uint32 cir_en, bucket_size, ref_cnt, ref_unit;
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
        case drvPolicerModeGreen:
            cir_en = 0;
            break;
        default:
            return SOC_E_PARAM;
    }
    meter = drv_entry->meter_data;
    sal_memset(meter, 0, sizeof(drv_entry->meter_data));

    if (cir_en){
        rv = DRV_CFP_METER_RATE_TRANSFORM(unit, policer_cfg->ckbits_sec, 
            policer_cfg->ckbits_burst, &bucket_size, &ref_cnt, &ref_unit);
        SOC_IF_ERROR_RETURN(rv);
        DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_BUCKET_SIZE, drv_entry, &bucket_size);
        DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_REF_UNIT, drv_entry, &ref_unit);
        DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_REF_CNT, drv_entry, &ref_cnt);
    } else {
        ref_cnt = 0;
        ref_unit = 0;
        DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_REF_UNIT, drv_entry, &ref_unit);
        DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_REF_CNT, drv_entry, &ref_cnt);
        }
    return SOC_E_NONE;
}



int
drv_harrier_fp_policer_control(int unit,  int stage_id, int op, void *entry, drv_policer_config_t *policer_cfg) {

    int rv = SOC_E_NONE;
    drv_cfp_entry_t *drv_entry;

    switch (op) {
        case DRV_FIELD_POLICER_MODE_SUPPORT:
            rv = _drv_harrier_fp_policer_support_check(unit, stage_id, policer_cfg);
            break;
        case DRV_FIELD_POLICER_CONFIG:
            rv =  _drv_harrier_fp_policer_config(unit, stage_id, entry, policer_cfg);
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
_drv_harrier_fp_counter_mode_support(int unit, int param0, void *mode) {
    int policer_valid;

    policer_valid = *((int *)mode);

    if (policer_valid) {
        if (param0 == DRV_FIELD_COUNTER_MODE_RED_NOTRED){
            return SOC_E_NONE;
        }
    } else {
        if (param0 == DRV_FIELD_COUNTER_MODE_NO_YES){
            return SOC_E_NONE;
        }
    }
    return SOC_E_UNAVAIL;
}

int
drv_harrier_fp_stat_type_get(int unit, int stage_id, drv_policer_mode_t policer_mode,
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
_drv_harrier_fp_stat_mode_support(int unit, int stage_id, void *mode)    
{
    drv_field_stat_t f_stat;
    int type1, type2, type3;
    int rv;

    f_stat = *((drv_field_stat_t *)mode);
    type1 = type2 = type3 =-1;
    rv = drv_harrier_fp_stat_type_get(unit, stage_id, drvFieldStatCount,
        f_stat, &type1, &type2, &type3);
    return rv;
}

int
drv_harrier_fp_stat_support_check(int unit, int stage_id, int op, int param0, void *mode) {
        
    switch (op) {
        case _DRV_FP_STAT_OP_COUNTER_MODE:
            return _drv_harrier_fp_counter_mode_support(unit, param0, mode);
        case _DRV_FP_STAT_OP_STAT_MODE:
            return _drv_harrier_fp_stat_mode_support(unit, stage_id,mode);
    }
    return SOC_E_NONE;
}


