/*
 * $Id: field.c,v 1.6 Broadcom SDK $
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
#include "thunderbolt/fp_tb.h"
#include <soc/robo/voyager_service.h>

/*
  * Definition of maximum user flow id number =
  *         (4096 - max(IVM/EVM entry count, CFP entry count))
  * Where,
  * A0 entry counts: IVM/EVM 1024, CFP 1536
  * B0 entry counts: IVM/EVM 2048, CFP 1536
  */
#define MAX_USER_FLOW_ID_A0(unit) \
    soc_property_get(unit, spn_TB_FLOW_ID_SIZE, 2560)
#define MAX_USER_FLOW_ID_B0(unit) \
        soc_property_get(unit, spn_TB_FLOW_ID_SIZE, 2048)

#define MAX_USER_FLOW_ID(unit) \
    (SOC_IS_TB_AX(unit) ?  MAX_USER_FLOW_ID_A0(unit): MAX_USER_FLOW_ID_B0(unit))


typedef struct _tb_qual_udf_slice_map_s{
    int cfp_qual;
    uint32 slice_map;
}_tb_qual_udf_slice_map_t;

static _tb_qual_udf_slice_map_t _tb_qual_udf_map[] = {
    {drvFieldQualifyDstIp, 0xb},
    {drvFieldQualifySrcIp, 0xb},
    {drvFieldQualifySrcIp6, 0xb0},
    {drvFieldQualifySrcIp6High, 0xb0},
    {drvFieldQualifySrcIp6Low, 0xb0},
    {drvFieldQualifyInnerVlanId, 0x20},
    {drvFieldQualifyDstMac, 0xf000},    
    {drvFieldQualifySrcMac, 0xf022}
};
int _tb_frame_any_qset[] =
{
drvFieldQualifyInterfaceClassPort,
drvFieldQualifyInPort,
drvFieldQualifyVlanFormat,
drvFieldQualifyIpType,
drvFieldQualifyOuterVlanCfi,
drvFieldQualifyOuterVlanPri,
drvFieldQualifyOuterVlanId,
drvFieldQualifyInnerVlanCfi,
drvFieldQualifyInnerVlanPri,
drvFieldQualifyInnerVlanId
};



int _drv_tbx_fp_entry_alloc(int unit, int stage_id, void **entry);
int _drv_tbx_fp_entry_copy(int unit, int stage_id, void *src_entry, void *dst_entry );

#ifdef BCM_TB_SUPPORT
int _drv_tb_cfp_qual_value_set(int unit, drv_field_qualify_t qual, void *drv_entry,
                      uint32 *p_data, uint32 *p_mask);
int _drv_tb_cfp_qual_value_get(int unit, drv_field_qualify_t qual, void *drv_entry,
                      uint32 *p_data, uint32 *p_mask);

static int 
_drv_tb_cfp_qual_field_get(int unit, drv_cfp_entry_t *cfp_entry, drv_field_qualify_t qual, int *cfp_field)
{
    int     rv = SOC_E_NONE;
    int slice_id;

    slice_id = cfp_entry->slice_id;

    switch (qual) {
        case drvFieldQualifyClassId:
            *cfp_field = DRV_CFP_FIELD_CHAIN_ID;
            break;
        case drvFieldQualifyDSCP: /* drvFieldQualifyTos */
            if (slice_id >= CFP_53280_SLICE_ID_IPV6_BASE) {
                /* drvFieldQualifyIp6TrafficClass */
                *cfp_field = DRV_CFP_FIELD_IP6_TRAFFIC_CLASS;
            } else {
                *cfp_field = DRV_CFP_FIELD_IP_TOS;
            }
            break;
        case drvFieldQualifyDstIp:
            *cfp_field = DRV_CFP_FIELD_IP_DA;
            break;
        case drvFieldQualifyDstMac:            
            *cfp_field = DRV_CFP_FIELD_MAC_DA;
            break;
        case drvFieldQualifyEtherType:
            *cfp_field = DRV_CFP_FIELD_ETYPE;
            break;            
        case drvFieldQualifyInPort:
            *cfp_field = DRV_CFP_FIELD_SRC_PORT;
            break;            
        case drvFieldQualifyInnerVlanCfi:
            *cfp_field = DRV_CFP_FIELD_USR_CFI;
            break;
        case drvFieldQualifyInnerVlanId:            
            *cfp_field = DRV_CFP_FIELD_USR_VID;
            break;
        case drvFieldQualifyInnerVlanPri:            
            *cfp_field = DRV_CFP_FIELD_USR_PRI;
            break;
        case drvFieldQualifyInterfaceClassPort: /* drvFieldQualifyPortClass */
            *cfp_field = DRV_CFP_FIELD_SRC_PROFILE;
            break;
        case drvFieldQualifyIpAuth:      
            *cfp_field = DRV_CFP_FIELD_IP_AUTH;
            break;            
        case drvFieldQualifyIpFlags:                 
            *cfp_field = DRV_CFP_FIELD_IP_FRAG;
            break;                        
        case drvFieldQualifyIpProtocol:
        case drvFieldQualifyIpProtocolCommon:
            if (slice_id >= CFP_53280_SLICE_ID_IPV6_BASE) {
                /* drvFieldQualifyIp6NextHeader */
                *cfp_field = DRV_CFP_FIELD_IP6_NEXT_HEADER;
            } else {
                *cfp_field = DRV_CFP_FIELD_IP_PROTO;
            }
            break;                        
        case drvFieldQualifyOuterVlanCfi:
            *cfp_field = DRV_CFP_FIELD_SP_CFI;            
            break;            
        case drvFieldQualifyOuterVlanId:            
            *cfp_field = DRV_CFP_FIELD_SP_VID;                        
            break;            
        case drvFieldQualifyOuterVlanPri:           
            *cfp_field = DRV_CFP_FIELD_SP_PRI;                        
            break;            
        case drvFieldQualifySrcIp:                   
            *cfp_field = DRV_CFP_FIELD_IP_SA;            
            break;                        
        case drvFieldQualifySrcIp6:                   
            *cfp_field = DRV_CFP_FIELD_IP6_SA;            
            break;                        
        case drvFieldQualifySrcIp6High:
            *cfp_field = DRV_CFP_FIELD_IP6_SA;            
            break;                        
        case drvFieldQualifySrcMac:            
            *cfp_field = DRV_CFP_FIELD_MAC_SA;            
            break;                        
        case drvFieldQualifyTtl: 
            if (slice_id >= CFP_53280_SLICE_ID_IPV6_BASE) {
                /* drvFieldQualifyIp6HopLimit */
                *cfp_field = DRV_CFP_FIELD_IP6_HOP_LIMIT;
            } else {
                *cfp_field = DRV_CFP_FIELD_IP_TTL;                        
            }
            break;                        
        case drvFieldQualifyIpType:
            *cfp_field = DRV_CFP_FIELD_L3_FRM_FORMAT;
            break;                
        case drvFieldQualifyL2Format:
            *cfp_field = DRV_CFP_FIELD_L2_FRM_FORMAT;
            break;                
        default:
            rv = SOC_E_UNAVAIL;
            break;
    }
    return rv;
}
#endif /* BCM_TB_SUPPORT */



int
_drv_tbx_ivm_qual_value_set(int unit, drv_field_qualify_t qual, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = SOC_E_NONE;
    uint32      fld_index = 0;
    drv_vm_entry_t *drv_entry = (drv_vm_entry_t *)entry;

    switch (qual) {
        case drvFieldQualifyInterfaceClassPort:
            fld_index = DRV_VM_FIELD_IVM_INGRESS_PORT_PROFILE;
            break;
        case drvFieldQualifyInPort:
            fld_index = DRV_VM_FIELD_IVM_INGRESS_PORT_ID;
            break;
        case drvFieldQualifyOuterVlanId:
            fld_index = DRV_VM_FIELD_IVM_INGRESS_SVID;
            break;
        case drvFieldQualifyRangeCheck:
            fld_index = DRV_VM_FIELD_IVM_INGRESS_CVID_RANGE;
            break;
        case drvFieldQualifyInnerVlanId:
            fld_index = DRV_VM_FIELD_IVM_INGRESS_CVID;
            break;
        case drvFieldQualifyOuterVlanPri:
            fld_index = DRV_VM_FIELD_IVM_INGRESS_SPCP;
            break;
        case drvFieldQualifyInnerVlanPri:
            fld_index = DRV_VM_FIELD_IVM_INGRESS_CPCP;
            break;
        case drvFieldQualifyL2Format:
            fld_index = DRV_VM_FIELD_IVM_FRAME_TYPE;
            break;
        case drvFieldQualifyEtherType:
            fld_index = DRV_VM_FIELD_IVM_ETHER_TYPE;
            break;
        case drvFieldQualifyVlanFormat:
            /* 
              * drvFieldQualifyVlanFormat for IVM configures the Ingress 
              * STag/CTag status. The qualifier's value set is done 
              * directly by DRV_VM_FIELD_SET() and should not come to
              * this routine.
              */
        default:
            rv = SOC_E_INTERNAL;
            return rv;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "_drv_tbx_ivm_qual_value_set : fld_index = 0x%x\n"),
                 fld_index));
    SOC_IF_ERROR_RETURN(
        DRV_VM_FIELD_SET(unit, DRV_VM_RAM_IVM_KEY_DATA, fld_index, 
                                          drv_entry, p_data));
    SOC_IF_ERROR_RETURN(
        DRV_VM_FIELD_SET(unit, DRV_VM_RAM_IVM_KEY_MASK, fld_index, 
                                          drv_entry, p_mask));
    
  LOG_VERBOSE(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "_drv_tbx_ivm_qual_value_set : data= 0x%x, 0x%x, \n"),
               drv_entry->key_data[0], drv_entry->key_data[1]));

    return SOC_E_NONE;
}

int
_drv_tbx_ivm_qual_value_get(int unit, drv_field_qualify_t qual, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = SOC_E_NONE;
    uint32      fld_index = 0;
    drv_vm_entry_t *drv_entry = (drv_vm_entry_t *)entry;

    switch (qual) {
        case drvFieldQualifyInterfaceClassPort:
            fld_index = DRV_VM_FIELD_IVM_INGRESS_PORT_PROFILE;
            break;
        case drvFieldQualifyInPort:
            fld_index = DRV_VM_FIELD_IVM_INGRESS_PORT_ID;
            break;
        case drvFieldQualifyOuterVlanId:
            fld_index = DRV_VM_FIELD_IVM_INGRESS_SVID;
            break;
        case drvFieldQualifyRangeCheck:
            fld_index = DRV_VM_FIELD_IVM_INGRESS_CVID_RANGE;
            break;
        case drvFieldQualifyInnerVlanId:
            fld_index = DRV_VM_FIELD_IVM_INGRESS_CVID;
            break;
        case drvFieldQualifyOuterVlanPri:
            fld_index = DRV_VM_FIELD_IVM_INGRESS_SPCP;
            break;
        case drvFieldQualifyInnerVlanPri:
            fld_index = DRV_VM_FIELD_IVM_INGRESS_CPCP;
            break;
        case drvFieldQualifyL2Format:
            fld_index = DRV_VM_FIELD_IVM_FRAME_TYPE;
            break;
        case drvFieldQualifyEtherType:
            fld_index = DRV_VM_FIELD_IVM_ETHER_TYPE;
            break;
        case drvFieldQualifyVlanFormat:
            /* 
              * drvFieldQualifyVlanFormat of IVM configuration is the Ingress 
              * STag/CTag status. The qualifier's value get should be done 
              * directly by DRV_VM_FIELD_GET() at bcm layer.
              */
        default:
            rv = SOC_E_INTERNAL;
            return rv;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "_drv_tbx_ivm_qual_value_get : fld_index = 0x%x\n"),
                 fld_index));
    SOC_IF_ERROR_RETURN(
        DRV_VM_FIELD_GET(unit, DRV_VM_RAM_IVM_KEY_DATA, fld_index, 
                                          drv_entry, p_data));
    SOC_IF_ERROR_RETURN(
        DRV_VM_FIELD_GET(unit, DRV_VM_RAM_IVM_KEY_MASK, fld_index, 
                                          drv_entry, p_mask));
    
  LOG_VERBOSE(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "_drv_tbx_ivm_qual_value_get : data= 0x%x, 0x%x, \n"),
               drv_entry->key_data[0], drv_entry->key_data[1]));

    return SOC_E_NONE;
}

int
_drv_tbx_evm_qual_value_set(int unit, drv_field_qualify_t qual, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = SOC_E_NONE;
    uint32      fld_index = 0;
    drv_vm_entry_t *drv_entry = (drv_vm_entry_t *)entry;

    switch (qual) {
        case drvFieldQualifyInPort:
            fld_index = DRV_VM_FIELD_EVM_INGRESS_PORT_ID;
            break;
        case drvFieldQualifyFlowId:
            fld_index = DRV_VM_FIELD_EVM_FLOW_ID;
            break;
        case drvFieldQualifyInVPort:
            fld_index = DRV_VM_FIELD_EVM_INGRESS_VPORT_ID;
            break;
        case drvFieldQualifyOutPort:
            fld_index = DRV_VM_FIELD_EVM_EGRESS_PORT_ID;
            break;
        case drvFieldQualifyOutVPort:
            fld_index = DRV_VM_FIELD_EVM_EGRESS_VPORT_ID;
            break;
        default:
            rv = SOC_E_INTERNAL;
            return rv;
    }
     


    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "_drv_tbx_evm_qual_value_set : fld_index = 0x%x\n"),
                 fld_index));
    SOC_IF_ERROR_RETURN(
        DRV_VM_FIELD_SET(unit, DRV_VM_RAM_EVM_KEY_DATA, fld_index, 
                                          drv_entry, p_data));
    SOC_IF_ERROR_RETURN(
        DRV_VM_FIELD_SET(unit, DRV_VM_RAM_EVM_KEY_MASK, fld_index, 
                                          drv_entry, p_mask));
    
  LOG_VERBOSE(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "_drv_tbx_evm_qual_value_set : data= 0x%x, 0x%x,\n"),
               drv_entry->key_data[0], drv_entry->key_data[1]));

    return SOC_E_NONE;
}

int
_drv_tbx_evm_qual_value_get(int unit, drv_field_qualify_t qual, void *entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = SOC_E_NONE;
    uint32      fld_index = 0;
    drv_vm_entry_t *drv_entry = (drv_vm_entry_t *)entry;

    switch (qual) {
        case drvFieldQualifyInPort:
            fld_index = DRV_VM_FIELD_EVM_INGRESS_PORT_ID;
            break;
        case drvFieldQualifyFlowId:
            fld_index = DRV_VM_FIELD_EVM_FLOW_ID;
            break;
        case drvFieldQualifyInVPort:
            fld_index = DRV_VM_FIELD_EVM_INGRESS_VPORT_ID;
            break;
        case drvFieldQualifyOutPort:
            fld_index = DRV_VM_FIELD_EVM_EGRESS_PORT_ID;
            break;
        case drvFieldQualifyOutVPort:
            fld_index = DRV_VM_FIELD_EVM_EGRESS_VPORT_ID;
            break;
        default:
            rv = SOC_E_INTERNAL;
            return rv;
    }
     


    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "_drv_tbx_evm_qual_value_get : fld_index = 0x%x\n"),
                 fld_index));
    SOC_IF_ERROR_RETURN(
        DRV_VM_FIELD_GET(unit, DRV_VM_RAM_EVM_KEY_DATA, fld_index, 
                                          drv_entry, p_data));
    SOC_IF_ERROR_RETURN(
        DRV_VM_FIELD_GET(unit, DRV_VM_RAM_EVM_KEY_MASK, fld_index, 
                                          drv_entry, p_mask));
    
  LOG_VERBOSE(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "_drv_tbx_evm_qual_value_get : data= 0x%x, 0x%x,\n"),
               drv_entry->key_data[0], drv_entry->key_data[1]));

    return SOC_E_NONE;
}


#ifdef BCM_TB_SUPPORT
int
_drv_tb_cfp_udf_value_set(int unit, uint32 udf_idx, drv_cfp_entry_t *drv_entry,
                      uint32 *p_data, uint32 *p_mask)
{
    uint32      fld_index = 0;
    uint32      temp;
    uint32      fld_valid_index;
    int rv;


    rv = DRV_CFP_UDF_GET(unit, DRV_CFP_UDF_FIELD_GET, udf_idx, &fld_index, &fld_valid_index);
    if (SOC_FAILURE(rv)) {
        return rv;
    }

    if (drv_entry->flags & _DRV_CFP_SLICE_CHAIN){
        if (udf_idx >= CFP_53280_SLICE_CHAIN_UDF_BASE) {
            drv_entry->flags |= _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN;
        }
    }

    temp = 1;
    rv = DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM, fld_index, 
            drv_entry, p_data);
    if (SOC_FAILURE(rv)){
        drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN);
        return rv;
    }

    rv = DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM_MASK, fld_index, 
            drv_entry, p_mask);        
    if (SOC_FAILURE(rv)){
        drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN);
        return rv;
    }

    rv = DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM, fld_valid_index, 
            drv_entry, &temp);
    if (SOC_FAILURE(rv)){
        drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN);
        return rv;
    }


    rv = DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM_MASK, fld_valid_index, 
            drv_entry, &temp);
    
    drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN);

    drv_entry->flags |= _DRV_CFP_UDF_VALID;
    return  rv;
}
int
_drv_tb_cfp_udf_value_get(int unit, uint32 udf_idx, drv_cfp_entry_t *drv_entry,
                      uint32 *p_data, uint32 *p_mask)
{
    uint32      fld_index = 0;
    uint32      fld_valid_index;
    int rv;


    rv = DRV_CFP_UDF_GET(unit, DRV_CFP_UDF_FIELD_GET, udf_idx, &fld_index, &fld_valid_index);
    if (SOC_FAILURE(rv)) {
        return rv;
    }
    if (drv_entry->flags & _DRV_CFP_SLICE_CHAIN){
        if (udf_idx >= CFP_53280_SLICE_CHAIN_UDF_BASE) {
            drv_entry->flags |= _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN;
        }
    }
    rv = DRV_CFP_FIELD_GET
        (unit, DRV_CFP_RAM_TCAM, fld_index, 
            drv_entry, p_data);
    if (SOC_FAILURE(rv)){
        drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN);
        return rv;
    }

    rv = DRV_CFP_FIELD_GET
        (unit, DRV_CFP_RAM_TCAM_MASK, fld_index, 
            drv_entry, p_mask);        

    drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN);

    return  rv;
}

int
_drv_tb_cfp_entry_slice_id_change(int unit, drv_cfp_entry_t *drv_entry, int new_id)
{
    int rv;
    int old_id, i, j, k;
    int cfp_qual_remove[3], cfp_qual_add[3];
    uint32 p_data[4], p_mask[4], temp;
    drv_cfp_entry_t *temp_entry;
        
    old_id = drv_entry->slice_id;

    for (i = 0; i < COUNTOF(cfp_qual_remove); i++) {
        cfp_qual_remove[i] = -1;
    }
    for (i = 0; i < COUNTOF(cfp_qual_add); i++) {
        cfp_qual_add[i] = -1;
    }
    j = 0;
    k = 0;
    for (i = 0; i <COUNTOF(_tb_qual_udf_map); i ++) {
        if (TEST_CFP_53280_FIX_UDF(drv_entry, i)) {
            if (_tb_qual_udf_map[i].slice_map & (1 << old_id)){
                    cfp_qual_remove[k] = _tb_qual_udf_map[i].cfp_qual;
                    k++;
                if (_tb_qual_udf_map[i].slice_map & (1 << new_id)){
                    cfp_qual_add[j] = _tb_qual_udf_map[i].cfp_qual;
                    j++;
                }
            }
        }
    }

    if (j > 0) {
        rv = _drv_tbx_fp_entry_alloc(unit, DRV_FIELD_STAGE_INGRESS, 
                        (void *)&temp_entry);
        SOC_IF_ERROR_RETURN(rv);
        rv = _drv_tbx_fp_entry_copy(unit, DRV_FIELD_STAGE_INGRESS, 
                        drv_entry, temp_entry);
        if (SOC_FAILURE(rv)){
            sal_free(temp_entry);
            return rv;
        }
        
        for (i = 0; i < COUNTOF(cfp_qual_remove); i++) {
            if (cfp_qual_remove[i] != -1) {
                sal_memset(p_data, 0, 4 * sizeof(uint32)); 
                sal_memset(p_mask, 0, 4 * sizeof(uint32)); 
                rv = _drv_tb_cfp_qual_value_set(unit, cfp_qual_remove[i], 
                        drv_entry, (uint32 *)&p_data, (uint32 *)&p_mask);
                if (SOC_FAILURE(rv)){
                    sal_free(temp_entry);
                    return rv;
                }
            }
        }

        temp = 0;

        rv = DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_TCAM, 
                DRV_CFP_FIELD_UDF_ALL_VALID,
                drv_entry, &temp);
        if (SOC_FAILURE(rv)){
            sal_free(temp_entry);
            return rv;
        }
        temp = 0;

        rv = DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_TCAM_MASK, 
                DRV_CFP_FIELD_UDF_ALL_VALID, drv_entry, &temp);
        if (SOC_FAILURE(rv)){
            sal_free(temp_entry);
            return rv;
        }

        drv_entry->slice_id = new_id;

        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "%s old_id %d new_id %d\n"),
                     FUNCTION_NAME(), old_id, new_id));        
        for (i = 0; i < COUNTOF(cfp_qual_add); i++) {
            if (cfp_qual_add[i] != -1) {
                sal_memset(p_data, 0, 4 * sizeof(uint32)); 
                sal_memset(p_mask, 0, 4 * sizeof(uint32)); 
                rv = _drv_tb_cfp_qual_value_get(unit, cfp_qual_add[i], temp_entry, 
                        (uint32 *)&p_data, (uint32 *)&p_mask);
                if (SOC_FAILURE(rv)){
                    sal_free(temp_entry);
                    return rv;
                }
                rv = _drv_tb_cfp_qual_value_set(unit, cfp_qual_add[i], drv_entry, 
                        (uint32 *)&p_data, (uint32 *)&p_mask);
                if (SOC_FAILURE(rv)){
                    sal_free(temp_entry);
                    return rv;
                }
            }
        }
        sal_free(temp_entry);        
    } else {
        drv_entry->slice_id = new_id;
    }

    if (CFP_53280_SLICE_ID_MAP(new_id) & CFP_53280_SLICE_MAP_CHAIN){
        temp = 0;
    } else {
        temp = CFP_53280_SLICE_ID(new_id);
    }
    LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s set new slice_id %d to tcam\n"),
               FUNCTION_NAME(),temp));
    SOC_IF_ERROR_RETURN(
        DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_SLICE_ID, 
            drv_entry, &temp));

    temp = 0x3;
    SOC_IF_ERROR_RETURN(
        DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_SLICE_ID, 
            drv_entry, &temp));

    return SOC_E_NONE;
}

int
_drv_tb_cfp_slice_id_update(int unit, drv_field_qualify_t qual,
            drv_cfp_entry_t *drv_entry, int *slice_id)
{
    int rv;
    int update, conflict, i, valid;
    uint32 flags, flags_mask,  match_slice;
    int frame, new_frame, id, new_id, sliceid_map;
    uint32 mask;
    int qual_count;
    drv_field_qset_t temp_qset;
    char flag_name[32];

    if (drv_entry == NULL) {
        return SOC_E_PARAM;
    }

    update = FALSE;
    conflict = FALSE;
    new_id = -1;
    new_frame = -1;
    match_slice = drv_entry->slice_bmp;

    flags = drv_entry->flags;   
    flags_mask = _DRV_CFP_FRAME_ALL;

    sliceid_map = CFP_53280_SLICE_ID_MAP(drv_entry->slice_id);

    id = CFP_53280_SLICE_ID(drv_entry->slice_id);
    frame = CFP_53280_L3_FRAME(drv_entry->slice_id);

    LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP: %s slice_id %d sliceid_map %x flags %x flag_mask %x qual %d\n"),
               FUNCTION_NAME(),drv_entry->slice_id, sliceid_map, flags, flags_mask, qual));    

    if (!(flags & flags_mask)) {
        goto end;
    }

    flags &= flags_mask;

    if (flags  ==  _DRV_CFP_FRAME_ANY) {
        /* 
         * _DRV_CFP_FRAME_IPANY : don't care L3 Framing 
         *  Accept common slice qset only. 
         *  Allow chain slice (slice 3) udf only
         */
        /* check the allowed qset */
        memcpy(&temp_qset, &(drv_entry->drv_qset), sizeof(drv_field_qset_t));
        qual_count = 0;
        valid = -1;
        for ( i = 0; i < COUNTOF(_tb_frame_any_qset); i++){   
             if (_tb_frame_any_qset[i] == qual) {
                valid = 1;
                break;
            }
        }
        if (valid == -1) {
            conflict = TRUE;
            LOG_DEBUG(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "common qual only for _DRV_CFP_FRMAE_ANY\n")));
            goto end;
        } else {            
            for ( i = 0; i < COUNTOF(_tb_frame_any_qset); i++){
                DRV_FIELD_QSET_REMOVE(temp_qset,_tb_frame_any_qset[i] );
            }
            DRV_FIELD_QSET_REMOVE(temp_qset, drvFieldQualifyStageIngress);

            /* check the qset in drv_entry */
            SHR_BITCOUNT_RANGE(temp_qset.w, qual_count, 0, DRV_FIELD_QUALIFY_MAX);

            if (qual_count != 0) {
                conflict = TRUE;
                LOG_DEBUG(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_DRV_CFP_FRMAE_ANY is not allowed since the non-common qual has been existed.\n")));
                goto end;
            }
        }

        for (i = 0; i < CFP_53280_SLICE_CHAIN_UDF_BASE; i++){
            if (SHR_BITGET(temp_qset.udf_map, i) != 0){
                conflict = TRUE;
                LOG_DEBUG(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_DRV_CFP_FRMAE_ANY is not allowed since the udf has been existed.\n")));
                goto end;
            }
        }

        /* check allowed slice map*/
        if (match_slice & CFP_53280_SLICE_MAP_FRAME_ANY){
            match_slice &= CFP_53280_SLICE_MAP_FRAME_ANY;
            if (sliceid_map & match_slice){
                new_id = id;
                new_frame = frame;
            } else {                
                if (id == CFP_53280_SLICE_ID_WITH_CHAIN) {
                    mask = match_slice & CFP_53280_SLICE_MAP_CHAIN;
                } else {
                    mask = match_slice & ~CFP_53280_SLICE_MAP_CHAIN;
                }
                new_id = -1;
                for (i=0; i < CFP_53280_SLICE_MAX_ID; i++) {
                    if (mask & CFP_53280_SLICE_ID_MAP(i)) {
                        new_id = CFP_53280_SLICE_ID(i);
                        new_frame = CFP_53280_L3_FRAME(i);
                        break;
                    }
                }
                if(new_id == -1) {
                    conflict = TRUE;
                    LOG_DEBUG(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "_DRV_CFP_FRMAE_ANY is not allowed since no availabe slice match.\n")));
                    goto end;
                }
                update = TRUE;
            }
        } else {
            conflict = TRUE;
            LOG_DEBUG(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_DRV_CFP_FRMAE_ANY is not allowed since no availabe slice match!\n")));
            goto end;
        }

    LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s flag: _DRV_CFP_FRAME_ANY new_id %d (%d) new_frame %d (%d) slice_map %d\n"),
               FUNCTION_NAME(), new_id, id, new_frame, frame, match_slice));
    } else if (flags & _DRV_CFP_FRAME_IPANY) {
        /* 
         * _DRV_CFP_FRAME_IPANY: L3 framing ignore NON_IP
         *  Accept IP common slice qset only. 
         */
        /* exclude the fix udf qset */
        valid = -1;
        memcpy(&temp_qset, &(drv_entry->drv_qset), sizeof(drv_field_qset_t));        

        for ( i = 0; i < COUNTOF(_tb_qual_udf_map); i++){
            if (_tb_qual_udf_map[i].cfp_qual == drvFieldQualifyInnerVlanId) {
                if (_tb_qual_udf_map[i].slice_map == match_slice){
                    valid = 1;
                    break;
                } else {
                    match_slice &= ~(_tb_qual_udf_map[i].slice_map);
                    continue;
                }
            }
            if (DRV_FIELD_QSET_TEST(drv_entry->drv_qset, 
                _tb_qual_udf_map[i].cfp_qual) != 0){
                valid = 1;
                break;
            }
             if (_tb_qual_udf_map[i].cfp_qual == qual) {
                valid = 1;
                break;
            }
        }
    
        if (valid == 1) {
            LOG_DEBUG(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_DRV_CFP_FRAME_IPANY is not allowed since fix udf qual has been existed.\n")));
            conflict = TRUE;
            goto end;
        }
        for (i = 0; i < CFP_53280_SLICE_CHAIN_UDF_BASE; i++){
            if (SHR_BITGET(temp_qset.udf_map, i) != 0){
                conflict = TRUE;
                LOG_DEBUG(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_DRV_CFP_FRAME_IPANY is not allowed since the udf has been existed.\n")));
                goto end;
            }
        }    

    }

    flags &=~_DRV_CFP_FRAME_IPANY;

    if (flags == _DRV_CFP_FRAME_IP) {
        flags_mask = CFP_53280_SLICE_MAP_FRAME_IP;
        sal_strncpy(flag_name,"_DRV_CFP_FRAME_IP",sizeof(flag_name));
    } else if (flags == _DRV_CFP_FRAME_IP4) {
        flags_mask = CFP_53280_SLICE_MAP_FRAME_IPV4_ALL;
        sal_strncpy(flag_name,"_DRV_CFP_FRAME_IP4",sizeof(flag_name));
    } else if (flags == _DRV_CFP_FRAME_IP6) {
        flags_mask = CFP_53280_SLICE_MAP_FRAME_IPV6_ALL;
        sal_strncpy(flag_name,"_DRV_CFP_FRAME_IP6",sizeof(flag_name));
    } else if (flags == _DRV_CFP_FRAME_NONIP) {
        flags_mask = CFP_53280_SLICE_MAP_FRAME_NONIP_ALL;
        sal_strncpy(flag_name,"_DRV_CFP_FRAME_NONIP",sizeof(flag_name));
    } else if (flags == _DRV_CFP_FRAME_ANY){
        goto end;
    } else {
        return SOC_E_UNAVAIL;
    }
    
    if (match_slice & flags_mask){
        match_slice &= flags_mask;
        if (sliceid_map & match_slice){
            new_id = id;
            new_frame = frame;
        } else {
            if (id == CFP_53280_SLICE_ID_WITH_CHAIN) {
                mask = match_slice & CFP_53280_SLICE_MAP_CHAIN;
            } else {
                mask = match_slice & ~CFP_53280_SLICE_MAP_CHAIN;
            }
            new_id = -1;
            for (i=0; i < CFP_53280_SLICE_MAX_ID; i++) {
                if (mask & CFP_53280_SLICE_ID_MAP(i)) {
                    new_id = CFP_53280_SLICE_ID(i);
                    new_frame = CFP_53280_L3_FRAME(i);
                    break;
                }
            }
            if(new_id == -1) {
                conflict = TRUE;
                LOG_DEBUG(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s is not allowed since no availabe slice match.\n"),
                           flag_name));
                goto end;
            }
        }
        update = TRUE;
    } else {
        conflict = TRUE;
        LOG_DEBUG(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s is not allowed since no availabe slice match!\n"),
                   flag_name));
        goto end;
    }




end:
    if (conflict) {       
        return SOC_E_UNAVAIL;
    }

    for (i = 0; i <COUNTOF(_tb_qual_udf_map); i ++) {        
        if (_tb_qual_udf_map[i].cfp_qual == qual) {
            if (_tb_qual_udf_map[i].slice_map & sliceid_map){
                drv_entry->flags |= _DRV_CFP_FIX_UDF_ALL_VALID;
                drv_entry->flags |= CFP_53280_FIX_UDF_SET(i);
            }
        }
    }

    if (update) {
        *slice_id = (new_frame << 2) | new_id;    
        rv = _drv_tb_cfp_entry_slice_id_change(unit, drv_entry, *slice_id);
        SOC_IF_ERROR_RETURN(rv);
        id = new_id;
        drv_entry->slice_bmp = match_slice;
    } else {
        *slice_id = -1;        
    }

    LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP: %s id %d flags %x \n"),
               FUNCTION_NAME(),id, drv_entry->flags)); 
    return SOC_E_NONE;
}
    
int
_drv_tb_cfp_slice_id_update_udf(int unit, uint32 udf_idex,
            drv_cfp_entry_t *drv_entry, int *slice_id)
{
    int u_slice_id, u_slice_map, new_slice;
    int rv, i, change;
    uint32 temp;

    u_slice_id = -1;
    u_slice_map = -1;
    change = -1;
    rv = DRV_CFP_UDF_GET(unit, DRV_CFP_UDF_SLICE_ID_GET, udf_idex,
        (uint32 *)&u_slice_id, (uint32 *)&u_slice_map);
    SOC_IF_ERROR_RETURN(rv);    

    LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          
                           "DRV_FP: %s user idx %d slice_id %d  drv entry slice_id %d map %x\n"),
               FUNCTION_NAME(), udf_idex, u_slice_id, drv_entry->slice_id, 
               drv_entry->slice_bmp));

    new_slice = -1;

    if (drv_entry->slice_id == u_slice_id){
        drv_entry->slice_bmp &= u_slice_map;
        *slice_id = new_slice;
        return SOC_E_NONE;
    }

    if ((drv_entry->flags & _DRV_CFP_FRAME_ANY) ||
        (drv_entry->flags & _DRV_CFP_FRAME_IPANY) ) {
        if (u_slice_id != CFP_53280_SLICE_ID_CHAIN_SLICE){
            return SOC_E_UNAVAIL;
        }
    }


    if (drv_entry->flags & _DRV_CFP_SLICE_CHAIN) {
        if (CFP_53280_SLICE_ID(u_slice_id) == CFP_53280_SLICE_ID_ALLOW_CHAIN){
            new_slice = (CFP_53280_L3_FRAME(u_slice_id) << 2) |
                CFP_53280_SLICE_ID_WITH_CHAIN;
        } else if (u_slice_id == CFP_53280_SLICE_ID_CHAIN_SLICE){
            *slice_id = new_slice;
            return SOC_E_NONE;
        } else {
            return SOC_E_UNAVAIL;
        }
    } else {
        if (drv_entry->slice_bmp & u_slice_map) {
            new_slice = u_slice_id;            
        } else {
            return SOC_E_UNAVAIL;
        }
    }
    if (drv_entry->flags & _DRV_CFP_FIX_UDF_ALL_VALID) {
        for (i = 0; i <COUNTOF(_tb_qual_udf_map); i ++) {
            if (TEST_CFP_53280_FIX_UDF(drv_entry, i)) {
                LOG_DEBUG(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "DRV_FP: _tb_qual_udf_map %d\n"),
                           i));
                if (_tb_qual_udf_map[i].slice_map & 
                    CFP_53280_SLICE_ID_MAP(drv_entry->slice_id)){
                    if (_tb_qual_udf_map[i].slice_map & 
                        CFP_53280_SLICE_ID_MAP(u_slice_id)){
                        change = 1;
                    } else {
                        change = 0;
                    }
                } else {
                    change = 0;
                }
            }
        }                
    } 
    if (change == 1) {
        rv = _drv_tb_cfp_entry_slice_id_change(unit, drv_entry, new_slice);
        SOC_IF_ERROR_RETURN(rv);
    } else if (change == 0){
        return SOC_E_UNAVAIL;
    } else {    
        if (CFP_53280_SLICE_ID_MAP(new_slice) & CFP_53280_SLICE_MAP_CHAIN){
            temp = 0;
        } else {
            temp = CFP_53280_SLICE_ID(new_slice);
        }
        LOG_DEBUG(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s set new slice_id %d to tcam \n"),
                   FUNCTION_NAME(),temp));
        SOC_IF_ERROR_RETURN(
            DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_SLICE_ID, 
                drv_entry, &temp));

        temp = 0x3;
        SOC_IF_ERROR_RETURN(
            DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_SLICE_ID, 
                drv_entry, &temp));
        drv_entry->slice_id = new_slice;
    }
    drv_entry->slice_bmp = CFP_53280_SLICE_ID_MAP(new_slice);
    *slice_id = new_slice;

    return SOC_E_NONE;
}

int 
_drv_tb_cfp_qual_value_set(int unit, drv_field_qualify_t qual, void *drv_entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = SOC_E_NONE;
    uint32      fld_index = 0;

    rv = _drv_tb_cfp_qual_field_get(unit, drv_entry, qual, (int *)&fld_index);    
    SOC_IF_ERROR_RETURN(rv);

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "_field_qual_value_set : fld_index = 0x%x\n"),
                 fld_index));
    SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM, fld_index, 
            drv_entry, p_data));
    
    SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_SET
        (unit, DRV_CFP_RAM_TCAM_MASK, fld_index, 
            drv_entry, p_mask)); 


    return SOC_E_NONE;
}


int 
_drv_tb_cfp_qual_value_get(int unit, drv_field_qualify_t qual, void *drv_entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int     rv = SOC_E_NONE;
    uint32      fld_index = 0;
    
  
    rv = _drv_tb_cfp_qual_field_get(unit, drv_entry, qual, (int *)&fld_index);    
    if (SOC_FAILURE(rv)) {
        return rv;
    }    

    SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_GET
        (unit, DRV_CFP_RAM_TCAM, fld_index, 
            drv_entry, p_data));
    
    SOC_IF_ERROR_RETURN(DRV_CFP_FIELD_GET
        (unit, DRV_CFP_RAM_TCAM_MASK, fld_index, 
            drv_entry, p_mask)); 
    return rv;
}

/* drvFieldQualifyxxx support list for TB */
static int
_drv_tb_ingress_qualify_support(int unit, drv_field_qualify_t qual)    
{
    int rv = SOC_E_UNAVAIL;

    switch(qual) {
        case drvFieldQualifyClassId:
        case drvFieldQualifyDSCP:          /*drvFieldQualifyTos, drvFieldQualifyIp6TrafficClass*/
        case drvFieldQualifyDstIp:
        case drvFieldQualifyDstMac:     
        case drvFieldQualifyEtherType:
        case drvFieldQualifyInPort:     
        case drvFieldQualifyInnerVlan:        
        case drvFieldQualifyInnerVlanCfi:
        case drvFieldQualifyInnerVlanId:     
        case drvFieldQualifyInnerVlanPri:     
        case drvFieldQualifyInterfaceClassPort: /* drvFieldQualifyPortClass */
        case drvFieldQualifyIp4:
        case drvFieldQualifyIp6:              
        case drvFieldQualifyIpAuth:
        case drvFieldQualifyIpFlags:          
        case drvFieldQualifyIpFrag:          
        case drvFieldQualifyIpInfo:
        case drvFieldQualifyIpProtocol: /*drvFieldQualifyIp6NextHeader */
        case drvFieldQualifyIpProtocolCommon:
        case drvFieldQualifyIpType:
        case drvFieldQualifyL2Format:
        case drvFieldQualifyLlc:    
        case drvFieldQualifyOuterVlan:
        case drvFieldQualifyOuterVlanCfi:
        case drvFieldQualifyOuterVlanId:
        case drvFieldQualifyOuterVlanPri:     
        case drvFieldQualifyPacketFormat:      
        case drvFieldQualifySrcIp:            
        case drvFieldQualifySrcIp6:
        case drvFieldQualifySrcIp6High:
        case drvFieldQualifySrcIp6Low:
        case drvFieldQualifySrcMac:     
        case drvFieldQualifyStage:          
        case drvFieldQualifyStageIngress:
        case drvFieldQualifyTtl: /* drvFieldQualifyIp6HopLimit */
        case drvFieldQualifyVlanFormat:
            rv =  SOC_E_NONE;
            break;
        default:
            rv =  SOC_E_UNAVAIL;
            break;
    }
    return rv;
}
static int
_drv_tb_qualify_to_cfp(int unit, drv_field_qualify_t qual, 
                    drv_cfp_entry_t * drv_entry)
{
    int rv = SOC_E_NONE;

    if ((qual == drvFieldQualifyStageIngress)||
        (qual == drvFieldQualifyStage)) {
        return rv;
    }        
    switch (qual) {
        case drvFieldQualifyClassId:
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_CLASS_ID, drv_entry, 1);
            break;
        case drvFieldQualifyDSCP:    /* drvFieldQualifyTos, drvFieldQualifyIp6TrafficClass */                
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_IP6_TRAFFIC_CLASS, 
                        drv_entry, 1);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_IP_TOS, drv_entry, 1);
            break;
        case drvFieldQualifyDstIp:
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_IP_DA, drv_entry, 1);
            break;
        case drvFieldQualifyDstMac:            
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_MAC_DA, drv_entry, 1);
            break;
        case drvFieldQualifyEtherType:
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_ETYPE, drv_entry, 1);
            break;            
        case drvFieldQualifyInPort:
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_SRC_PORT, drv_entry, 1);
            break;            
        case drvFieldQualifyInnerVlan:            
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_USR_VID, drv_entry, 1);
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_USR_PRI, drv_entry, 1);
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_USR_CFI, drv_entry, 1);
            break;            
        case drvFieldQualifyInnerVlanCfi:
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_USR_CFI, drv_entry, 1);
            break;
        case drvFieldQualifyInnerVlanId:            
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_USR_VID, drv_entry, 1);
            break;
        case drvFieldQualifyInnerVlanPri:            
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_USR_PRI, drv_entry, 1);
            break;
        case drvFieldQualifyInterfaceClassPort: /* drvFieldQualifyPortClass */
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_SRC_PROFILE, drv_entry, 1);
            break;
        case drvFieldQualifyIp4:
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_L3_FRM_FORMAT, drv_entry, 1);
            break;            
        case drvFieldQualifyIp6:                     
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_L3_FRM_FORMAT, drv_entry, 1);            
            break;                
        case drvFieldQualifyIpAuth:      
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_IP_AUTH, drv_entry, 1);            
            break;            
        case drvFieldQualifyIpFlags:                 
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_IP_FRGA, drv_entry, 1);            
            break;                        
        case drvFieldQualifyIpFrag:                
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_IP_FRGA, drv_entry, 1);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_IP_NON_FIRST_FRGA, drv_entry, 1);            
            break;            
        case drvFieldQualifyIpInfo:                 
            /* only support SOC_FIELD_IP_HDR_OFFSET_ZERO,SOC_FIELD_IP_HDR_FLAGS_MF*/
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_IP_FRGA, drv_entry, 1);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_IP_NON_FIRST_FRGA, drv_entry, 1);            
            break;                                    
        case drvFieldQualifyIpProtocol: /* drvFieldQualifyIp6NextHeader */
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_IP6_NEXT_HEADER, drv_entry, 1);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_IP_PROTO, drv_entry, 1);            
            break;                        
        case drvFieldQualifyIpProtocolCommon:
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_L3_FRM_FORMAT, drv_entry, 1);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_IP_PROTO, drv_entry, 1);            
            break;            
        case drvFieldQualifyIpType:             
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_L3_FRM_FORMAT, drv_entry, 1);            
            break;            
        case drvFieldQualifyL2Format:
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_L2_FRM_FORMAT, drv_entry, 1);            
            break;            
        case drvFieldQualifyLlc:           
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_L2_FRM_FORMAT, drv_entry, 1);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_L3_FRM_FORMAT, drv_entry, 1);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_ETYPE, drv_entry, 1);            
            break;                        
        case drvFieldQualifyOuterVlan:
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_SP_VID, drv_entry, 1);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_SP_CFI, drv_entry, 1);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_SP_PRI, drv_entry, 1);            
            break;            
        case drvFieldQualifyOuterVlanCfi:
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_SP_CFI, drv_entry, 1);            
            break;            
        case drvFieldQualifyOuterVlanId:            
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_SP_VID, drv_entry, 1);            
            break;            
        case drvFieldQualifyOuterVlanPri:           
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_SP_PRI, drv_entry, 1);            
            break;            
        case drvFieldQualifyPacketFormat:             
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_L2_FRM_FORMAT, drv_entry, 1);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_L3_FRM_FORMAT, drv_entry, 1);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_SPTAG, drv_entry, 1);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_1QTAG, drv_entry, 1);            
            break;            
        case drvFieldQualifySrcIp:                   
            DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_IP_SA, drv_entry, 1);            
            break;                        
        case drvFieldQualifySrcIp6:                   
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_IP6_SA, drv_entry, 1);            
            break;                        
        case drvFieldQualifySrcIp6High:
        case drvFieldQualifySrcIp6Low:
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_IP6_SA, drv_entry, 1);            
            break;                        
        case drvFieldQualifySrcMac:            
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_MAC_SA, drv_entry, 1);            
            break;                        
        case drvFieldQualifyTtl:     /* drvFieldQualifyIp6HopLimit */
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_IP6_HOP_LIMIT, drv_entry, 1);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_IP_TTL, drv_entry, 1);            
            break;                        
        case drvFieldQualifyVlanFormat:
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_1QTAG, drv_entry, 1);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_QSET_SET(unit, DRV_CFP_QUAL_SPTAG, drv_entry, 1);            
            break;                                       
        default:
            rv = SOC_E_UNAVAIL;
            break;
    }
    return rv;
}

/* convert the qset to drv_entry->w */
static int
_drv_tb_qset_to_cfp(int unit, drv_field_qset_t qset, drv_cfp_entry_t * drv_entry)
{
    int     idx;
    int     retval = SOC_E_UNAVAIL;
    uint32 udf_num;
    uint32 cfp_qual, temp;

    retval = DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_CFP_UDFS_NUM, &udf_num);
    SOC_IF_ERROR_RETURN(retval);
    for (idx = 0; idx < udf_num; idx++) {
        retval = DRV_CFP_UDF_GET(unit, DRV_CFP_UDF_QUAL_GET, 
            idx, &cfp_qual, &temp);
        SOC_IF_ERROR_RETURN(retval);

        if (SHR_BITGET(qset.udf_map, idx)) {
            retval = DRV_CFP_QSET_SET
                (unit, cfp_qual, drv_entry, 1);
            if (SOC_FAILURE(retval)) {
                return retval;
            }
        }
    }
    for (idx = 0; idx < drvFieldQualifyCount; idx++) {
        /* Skip unused qualifiers. */
        if (0 == DRV_FIELD_QSET_TEST((qset), idx)) {
            continue;
        }
        retval = _drv_tb_ingress_qualify_support(unit, idx);
        if (SOC_SUCCESS(retval)){
            retval = _drv_tb_qualify_to_cfp(unit, idx, drv_entry);
            if (SOC_FAILURE(retval)) {
                return retval;
            }            
        } else {
            return retval;
        }
    }
    return retval;
}
#endif /* BCM_TB_SUPPORT */


static int
_drv_tbx_cfp_selcode_to_qset(int unit, 
    int slice_id, drv_field_qset_t *qset)
{
    int  frame_type;
    
    frame_type = (slice_id & 0xc0) >> 2;
    
    switch (frame_type) {
        case FP_TB_L3_FRM_FMT_IP4:
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyIp4);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyDSCP);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyTos);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyIpProtocol);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyIpFlags);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyIpFrag);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyIpInfo);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyIpAuth);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyTtl);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyOuterVlan);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyOuterVlanPri);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyOuterVlanCfi);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyOuterVlanId);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInnerVlan);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInnerVlanPri);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInnerVlanCfi);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInnerVlanId);
            break;            

        case FP_TB_L3_FRM_FMT_IP6:
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyIp6);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyIp6TrafficClass);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyIp6NextHeader);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyIpFlags);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyIpFrag);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyIpInfo);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyIpAuth);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyIp6HopLimit);
            break;            

        case FP_TB_L3_FRM_FMT_NON_IP:
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyEtherType);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyLlc);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyOuterVlan);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyOuterVlanPri);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyOuterVlanCfi);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyOuterVlanId);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInnerVlan);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInnerVlanPri);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInnerVlanCfi);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInnerVlanId);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyDstMac);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifySrcMac);    
            break;            

        case FP_TB_L3_FRM_FMT_CHAIN:
            SHR_BITSET(qset->udf_map, 35);
            SHR_BITSET(qset->udf_map, 36);
            SHR_BITSET(qset->udf_map, 37);
            SHR_BITSET(qset->udf_map, 38);
            SHR_BITSET(qset->udf_map, 39);
            SHR_BITSET(qset->udf_map, 40);
            SHR_BITSET(qset->udf_map, 41);
            SHR_BITSET(qset->udf_map, 42);
            SHR_BITSET(qset->udf_map, 43);
            SHR_BITSET(qset->udf_map, 44);
            SHR_BITSET(qset->udf_map, 45);
            SHR_BITSET(qset->udf_map, 46);
            break;            

    }
    if (frame_type != FP_TB_L3_FRM_FMT_CHAIN) {
        DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyIpType);        
        DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyL2Format);
        DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyVlanFormat);
        DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyPacketFormat);
    }

    DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInterfaceClassPort);
    DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInPort);

    switch (slice_id) {
        case (0):
        case (3):
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyDstIp);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifySrcIp);
            SHR_BITSET(qset->udf_map, 0);
            SHR_BITSET(qset->udf_map, 1);
            SHR_BITSET(qset->udf_map, 2);
            SHR_BITSET(qset->udf_map, 3);
            SHR_BITSET(qset->udf_map, 4);
            break;
        case (1):
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifySrcMac);
            SHR_BITSET(qset->udf_map, 5);
            SHR_BITSET(qset->udf_map, 6);
            break;
        case (2):
            SHR_BITSET(qset->udf_map, 7);
            SHR_BITSET(qset->udf_map, 8);
            SHR_BITSET(qset->udf_map, 9);
            SHR_BITSET(qset->udf_map, 10);
            SHR_BITSET(qset->udf_map, 11);
            SHR_BITSET(qset->udf_map, 12);
            SHR_BITSET(qset->udf_map, 13);
            SHR_BITSET(qset->udf_map, 14);
            SHR_BITSET(qset->udf_map, 15);
            break;

        case (4):
        case (7):
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyOuterVlan);            
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyOuterVlanPri);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyOuterVlanCfi);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyOuterVlanId);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInnerVlan);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInnerVlanPri);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInnerVlanCfi);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInnerVlanId);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifySrcIp6);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifySrcIp6High);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifySrcIp6Low);
            SHR_BITSET(qset->udf_map, 16);            
            break;

        case (5):
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInnerVlanId);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifySrcIp6);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifySrcIp6High);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifySrcIp6Low);
            break;

        case (6):
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyOuterVlan);            
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyOuterVlanPri);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyOuterVlanCfi);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyOuterVlanId);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInnerVlan);            
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInnerVlanPri);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInnerVlanCfi);
            DRV_FIELD_QSET_ADD(*qset, drvFieldQualifyInnerVlanId);
            SHR_BITSET(qset->udf_map, 16);
            SHR_BITSET(qset->udf_map, 17);
            SHR_BITSET(qset->udf_map, 18);
            SHR_BITSET(qset->udf_map, 19);
            SHR_BITSET(qset->udf_map, 20);
            SHR_BITSET(qset->udf_map, 21);
            SHR_BITSET(qset->udf_map, 22);
            SHR_BITSET(qset->udf_map, 23);
            SHR_BITSET(qset->udf_map, 24);
            SHR_BITSET(qset->udf_map, 25);
            break;

          case (12):
          case (15):
            SHR_BITSET(qset->udf_map, 26);
            SHR_BITSET(qset->udf_map, 27);
            SHR_BITSET(qset->udf_map, 28);
            break;
          case (13):
            SHR_BITSET(qset->udf_map, 29);
            SHR_BITSET(qset->udf_map, 30);
            SHR_BITSET(qset->udf_map, 31);
            break;
          case (14):
            SHR_BITSET(qset->udf_map, 32);
            SHR_BITSET(qset->udf_map, 33);
            SHR_BITSET(qset->udf_map, 34);
            break;
    }
    return SOC_E_NONE;
}

STATIC int
_drv_tbx_vm_qset_mapping(drv_vm_entry_t *entry, drv_field_qset_t *qset,
        int drv_qual, int bcm_qual)
{
    int wp, bp;

    if (drv_qual < 0 || drv_qual > DRV_VM_QUAL_COUNT) {
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
_drv_tbx_ivm_selcode_to_qset(int unit, 
    int format_id, drv_field_qset_t *qset)
{
    int     rv = SOC_E_NONE;
    drv_vm_entry_t drv_entry;

    rv = DRV_VM_FORMAT_TO_QSET(unit, DRV_VM_RAM_IVM, format_id, &drv_entry);
    SOC_IF_ERROR_RETURN(rv);
    
    SOC_IF_ERROR_RETURN(
    _drv_tbx_vm_qset_mapping(&drv_entry, qset, 
        DRV_VM_QUAL_IVM_PORT_PROFILE, drvFieldQualifyInterfaceClassPort));
    SOC_IF_ERROR_RETURN(
    _drv_tbx_vm_qset_mapping(&drv_entry, qset, 
        DRV_VM_QUAL_IVM_PORT_ID, drvFieldQualifyInPort));
    SOC_IF_ERROR_RETURN(
    _drv_tbx_vm_qset_mapping(&drv_entry, qset, 
        DRV_VM_QUAL_IVM_INGRESS_STAG_STAUS, drvFieldQualifyVlanFormat));
    SOC_IF_ERROR_RETURN(
    _drv_tbx_vm_qset_mapping(&drv_entry, qset, 
        DRV_VM_QUAL_IVM_INGRESS_CTAG_STAUS, drvFieldQualifyVlanFormat));
    SOC_IF_ERROR_RETURN(
    _drv_tbx_vm_qset_mapping(&drv_entry, qset, 
        DRV_VM_QUAL_IVM_INGRESS_SVID, drvFieldQualifyOuterVlanId));
    SOC_IF_ERROR_RETURN(
    _drv_tbx_vm_qset_mapping(&drv_entry, qset, 
        DRV_VM_QUAL_IVM_INGRESS_CVID, drvFieldQualifyInnerVlanId));
    SOC_IF_ERROR_RETURN(
    _drv_tbx_vm_qset_mapping(&drv_entry, qset, 
        DRV_VM_QUAL_IVM_INGRESS_CVID_RANGE, drvFieldQualifyRangeCheck));
    SOC_IF_ERROR_RETURN(
    _drv_tbx_vm_qset_mapping(&drv_entry, qset, 
        DRV_VM_QUAL_IVM_INGRESS_SPCP, drvFieldQualifyOuterVlanPri));
    SOC_IF_ERROR_RETURN(
    _drv_tbx_vm_qset_mapping(&drv_entry, qset, 
        DRV_VM_QUAL_IVM_INGRESS_CPCP, drvFieldQualifyInnerVlanPri));
    SOC_IF_ERROR_RETURN(
    _drv_tbx_vm_qset_mapping(&drv_entry, qset, 
        DRV_VM_QUAL_IVM_INGRESS_FRAME_TYPE, drvFieldQualifyL2Format));
    SOC_IF_ERROR_RETURN(
    _drv_tbx_vm_qset_mapping(&drv_entry, qset, 
        DRV_VM_QUAL_IVM_INGRESS_ETHER_TYPE, drvFieldQualifyEtherType));

    return rv;
}

static int
_drv_tbx_evm_selcode_to_qset(int unit, 
     int format_id, drv_field_qset_t *qset)
{
    int     rv = SOC_E_NONE;
    drv_vm_entry_t drv_entry;
       
    rv = DRV_VM_FORMAT_TO_QSET(unit, DRV_VM_RAM_EVM, format_id, &drv_entry);
    SOC_IF_ERROR_RETURN(rv);

    SOC_IF_ERROR_RETURN(
    _drv_tbx_vm_qset_mapping(&drv_entry, qset, 
        DRV_VM_QUAL_EVM_INGRESS_PORT_ID, drvFieldQualifyInPort));
    SOC_IF_ERROR_RETURN(
    _drv_tbx_vm_qset_mapping(&drv_entry, qset, 
        DRV_VM_QUAL_EVM_INGRESS_VPORT_ID, drvFieldQualifyInVPort));
    SOC_IF_ERROR_RETURN(
    _drv_tbx_vm_qset_mapping(&drv_entry, qset, 
        DRV_VM_QUAL_EVM_FLOW_ID, drvFieldQualifyFlowId));
    SOC_IF_ERROR_RETURN(
    _drv_tbx_vm_qset_mapping(&drv_entry, qset, 
        DRV_VM_QUAL_EVM_EGRESS_PORT_ID, drvFieldQualifyOutPort));
    SOC_IF_ERROR_RETURN(
    _drv_tbx_vm_qset_mapping(&drv_entry, qset, 
        DRV_VM_QUAL_EVM_EGRESS_VPORT_ID, drvFieldQualifyOutVPort));

    return rv;
}

int
_drv_tbx_ivm_action_add(int unit,
             void *      drv_entry,
             drv_field_action_t action,
             uint32 param0,
             uint32 param1)
{
    uint32 temp;
    int rv = SOC_E_NONE;

    switch (action) {
        case drvFieldActionVlanNew:
            rv = DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_VLAN_ID, 
                drv_entry, param0);
            break;
        case drvFieldActionPrioPktNew:
            /* Process outer pcp */
            switch(param1 & DRV_FIELD_SPCP_MARK_MASK) {
                case DRV_FIELD_SPCP_MARK_MAPPED:
                    temp = DRV_VM_ACTION_PCP_MARK_MAPPED;
                    break;
                case DRV_FIELD_SPCP_MARK_USE_INNER_PCP:
                    temp = DRV_VM_ACTION_PCP_MARK_INGRESS_INNER_PCP;
                    break;
                case DRV_FIELD_SPCP_MARK_USE_OUTER_PCP:
                    temp = DRV_VM_ACTION_PCP_MARK_INGRESS_OUTER_PCP;
                    break;
                case DRV_FIELD_SPCP_MARK_USE_PORT_DEFAULT:
                    temp = DRV_VM_ACTION_PCP_MARK_PORT_DEFAULT;
                    break;
                default:
                    return SOC_E_PARAM;
                    break;
            }
            rv = DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_SPCP_MARKING_POLICY, 
                drv_entry, temp);

            SOC_IF_ERROR_RETURN(rv);
            /* Process inner pcp */
            switch(param1 & DRV_FIELD_CPCP_MARK_MASK) {
                case DRV_FIELD_CPCP_MARK_MAPPED:
                    temp = DRV_VM_ACTION_PCP_MARK_MAPPED;
                    break;
                case DRV_FIELD_CPCP_MARK_USE_INNER_PCP:
                    temp = DRV_VM_ACTION_PCP_MARK_INGRESS_INNER_PCP;
                    break;
                case DRV_FIELD_CPCP_MARK_USE_OUTER_PCP:
                    temp = DRV_VM_ACTION_PCP_MARK_INGRESS_OUTER_PCP;
                    break;
                case DRV_FIELD_CPCP_MARK_USE_PORT_DEFAULT:
                    temp = DRV_VM_ACTION_PCP_MARK_PORT_DEFAULT;
                    break;
                default:
                    return SOC_E_PARAM;
                    break;
            }
            rv = DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_CPCP_MARKING_POLICY, 
                drv_entry, temp);
            break;
        case drvFieldActionNewClassId:           
            if (param1 == _DRV_FP_ACTION_PRIVATE){
                if (param0 <= MAX_USER_FLOW_ID(unit)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "DRV_FP: private used flow id %d is out of range\n"),
                               param0));
                    return SOC_E_PARAM;
                }
            } else {
                if (param0 > MAX_USER_FLOW_ID(unit)){
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "DRV_FP: flow id %d is out of range\n"),
                               param0));
                    return SOC_E_PARAM;
                }
            }
            DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_FLOW_ID, 
                drv_entry, param0);
            break;
        case drvFieldActionVportNew:           
            DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_VPORT_ID, 
                drv_entry, param0);
            break;
        case drvFieldActionVportSpcpNew:           
            DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_VPORT_SPCP, 
                drv_entry, param0);
            break;
        case drvFieldActionVportCpcpNew:           
            DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_VPORT_CPCP, 
                drv_entry, param0);
            break;
        case drvFieldActionVportTcNew:           
            DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_VPORT_TC, 
                drv_entry, param0);
            break;
        case drvFieldActionVportDpNew:           
            DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_VPORT_DP, 
                drv_entry, param0);
            break;
         default:
            return SOC_E_UNAVAIL;
    }
    return rv;
}
int
_drv_tbx_evm_action_add(int unit,
             void *      drv_entry,
             drv_field_action_t action,
             uint32 param0,
             uint32 param1)
{

    uint32 temp;

    switch (action) {
        case drvFieldActionInnerVlanNew:
                DRV_VM_ACTION_SET(unit, DRV_VM_ACT_EVM_NEW_CVID, 
                    drv_entry, param0);

                switch(param1) {
                    case DRV_FIELD_TAG_AS_RECEIVED:
                        temp = DRV_VM_ACTION_TAG_AS_RECEIVED;
                        break;
                    case DRV_FIELD_TAG_AS_NORMALIZED:
                        temp = DRV_VM_ACTION_TAG_AS_NORMALIZED;
                        break;
                    case DRV_FIELD_TAG_COPY:
                        temp = DRV_VM_ACTION_TAG_AS_COPY;
                        break;
                    case DRV_FIELD_TAG_REMOVE:
                        temp = DRV_VM_ACTION_TAG_AS_REMOVE;
                        break;
                    case DRV_FIELD_TAG_REPLACE:
                        temp = DRV_VM_ACTION_TAG_AS_REPLACE;
                        break;
                    default:
                        return SOC_E_PARAM;
                        break;
                }
                DRV_VM_ACTION_SET(unit, DRV_VM_ACT_EVM_CTAG, 
                    drv_entry, temp);
                break;
        case drvFieldActionOuterVlanNew:
              DRV_VM_ACTION_SET(unit, DRV_VM_ACT_EVM_NEW_SVID, 
                    drv_entry, param0);

                switch(param1) {
                    case DRV_FIELD_TAG_AS_RECEIVED:
                        temp = DRV_VM_ACTION_TAG_AS_RECEIVED;
                        break;
                    case DRV_FIELD_TAG_AS_NORMALIZED:
                        temp = DRV_VM_ACTION_TAG_AS_NORMALIZED;
                        break;
                    case DRV_FIELD_TAG_COPY:
                        temp = DRV_VM_ACTION_TAG_AS_COPY;
                        break;
                    case DRV_FIELD_TAG_REMOVE:
                        temp = DRV_VM_ACTION_TAG_AS_REMOVE;
                        break;
                    case DRV_FIELD_TAG_REPLACE:
                        temp = DRV_VM_ACTION_TAG_AS_REPLACE;
                        break;
                    default:
                        return SOC_E_PARAM;
                        break;
                }
                DRV_VM_ACTION_SET(unit, DRV_VM_ACT_EVM_STAG, 
                    drv_entry, temp);
                break;
        case drvFieldActionOuterVlanPrioNew:
            if (SOC_IS_TB_AX(unit)) {
                return SOC_E_UNAVAIL;
            } else {
                temp = 1;
                DRV_VM_ACTION_SET(unit, DRV_VM_ACT_EVM_OUTER_PCP_REPLACE, 
                    drv_entry, temp);
                DRV_VM_ACTION_SET(unit, DRV_VM_ACT_EVM_NEW_OUTER_PCP, 
                    drv_entry, param0);
            }
            break;
        case drvFieldActionInnerVlanPrioNew:
            if (SOC_IS_TB_AX(unit)) {
                return SOC_E_UNAVAIL;
            } else {
                temp = 1;
                DRV_VM_ACTION_SET(unit, DRV_VM_ACT_EVM_INNER_PCP_REPLACE, 
                    drv_entry, temp);
                DRV_VM_ACTION_SET(unit, DRV_VM_ACT_EVM_NEW_INNER_PCP, 
                    drv_entry, param0);
            }
            break;
         default:
            return SOC_E_UNAVAIL;
    }
    return SOC_E_NONE;
}

int
_drv_tbx_cfp_action_add(int unit,
             void *      drv_entry,
             drv_field_action_t action,
             uint32 param0,
             uint32 param1)
{
    int rv = SOC_E_NONE;    
    uint32 valid_val;
    uint32 *action_mem;

    if (!(((drv_cfp_entry_t *)drv_entry)->flags & _DRV_CFP_ACTION_INIT)){
        /* BYPASS all the FILTER as default */
        rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_ALL, drv_entry, 0, 0);
        SOC_IF_ERROR_RETURN(rv);
        ((drv_cfp_entry_t *)drv_entry)->flags |= _DRV_CFP_ACTION_INIT;
    }

    if (((drv_cfp_entry_t *)drv_entry)->flags & _DRV_CFP_SLICE_CHAIN){
        if (((drv_cfp_entry_t *)drv_entry)->flags & _DRV_CFP_SLICE_CONFIG_SLICE_MAIN){
            action_mem = ((drv_cfp_entry_t *)drv_entry)->cfp_chain->act_data;
        } else {
            action_mem = ((drv_cfp_entry_t *)drv_entry)->act_data;
        }
    } else {
            action_mem = ((drv_cfp_entry_t *)drv_entry)->act_data;
    }


    switch (action)
    {
        case drvFieldActionNewClassId:
            if (param1 == _DRV_FP_ACTION_PRIVATE){
                if (param0 <= MAX_USER_FLOW_ID(unit)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "DRV_FP: private used flow id %d is out of range\n"),
                               param0));
                return SOC_E_PARAM;            
                }
            } else {
                if (param0 > MAX_USER_FLOW_ID(unit)){
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "DRV_FP: flow id %d is out of range\n"),
                               param0));
                    return SOC_E_PARAM;
                }
            }           
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CLASSFICATION_ID, 
                drv_entry, param0, 0);
            break;
        case drvFieldActionVlanNew:
            if (param0 > 4095) 
                return SOC_E_PARAM;
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_NEW_VLAN, 
                drv_entry, param0, 0);
            break;
        case drvFieldActionNewTc:
            if (param0 > 15) 
                return SOC_E_PARAM;
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CHANGE_TC, 
                drv_entry, 0, 0);
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_NEW_TC, 
                drv_entry, param0, 0);

            break;
        case drvFieldActionDropPrecedence:
            if (param0 > DRV_FIELD_COLOR_RED) 
                return SOC_E_PARAM;
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_IB_CHANGE_DP, 
                drv_entry, 0, 0);
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_OB_CHANGE_DP, 
                drv_entry, 0, 0);
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_IB_NEW_DP, 
                drv_entry, param0, 0);
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_OB_NEW_DP, 
                drv_entry, param0, 0);
            break;
        case drvFieldActionGpDropPrecedence:
          if (param0 > DRV_FIELD_COLOR_RED) 
                return SOC_E_PARAM; 
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_IB_CHANGE_DP, 
                drv_entry, 0, 0);
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_IB_NEW_DP, 
                drv_entry, param0, 0);
            break;
        case drvFieldActionRpDropPrecedence:
        case drvFieldActionYpDropPrecedence:
          if (param0 > DRV_FIELD_COLOR_RED) 
                return SOC_E_PARAM; 
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_OB_CHANGE_DP, 
                drv_entry, 0, 0);
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_OB_NEW_DP, 
                drv_entry, param0, 0);
            break;
        case drvFieldActionRedirectMcast:
            rv = DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_MCAST_NUM, &valid_val);
            SOC_IF_ERROR_RETURN(rv);

            if (param0 > valid_val) {
                return SOC_E_PARAM;
            }
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CHANGE_FWD, 
                drv_entry, 0, 0);
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_REDIRECT_MGID, 
                drv_entry, param0, 0);
            break;
        case drvFieldActionRedirectPort:
            if ((param0 != 0)||(!SOC_PORT_VALID(unit, (soc_port_t)param1))){
                return SOC_E_PARAM;
            }
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CHANGE_FWD, 
                drv_entry, 0, 0);
            if (SOC_FAILURE(rv)) {
                break;
            }
            /* param0: Destination modid for robo param0=0 ; param1:Destination port. */
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_REDIRECT_VPORT_PORT, 
                drv_entry, 0, param1);            
            break;
        case drvFieldActionRedirectVportPort:
            if ((param0 > 15)||(!SOC_PORT_VALID(unit, (soc_port_t)param1))) {
                return SOC_E_PARAM;
            }
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CHANGE_FWD, 
                drv_entry, 0, 0);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            /* param0: Destination vport; param1:Destination port. */
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_REDIRECT_VPORT_PORT, 
                drv_entry, param0, param1);            
            break;
        case drvFieldActionDrop:
        case drvFieldActionGpDrop:            
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CHANGE_FWD, 
                drv_entry, 0, 0);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_DROP, 
                drv_entry, 0, 0);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            if(action == drvFieldActionDrop) {
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_RATE_VIOLATE_DROP, 
                    drv_entry, 0, 0);    
            }
            break;
        case drvFieldActionLoopback:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CHANGE_FWD, 
                drv_entry, 0, 0);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_LOOPBACK, 
                drv_entry, 0, 0);            
            break;
        case drvFieldActionRpDrop:
        case drvFieldActionYpDrop:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_RATE_VIOLATE_DROP, 
                drv_entry, 0, 0);            
            break;
        case drvFieldActionCopyToCpu:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CPU_COPY, 
                drv_entry, 0, 0);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_COSQ_CPU_NEW, 
                drv_entry, 7, 0);            
            break;
        case drvFieldActionCosQCpuNew:
            valid_val = NUM_COS(unit);
            SOC_IF_ERROR_RETURN(rv);
            if (param0 > valid_val) {
                return SOC_E_PARAM;
            }
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CPU_COPY, 
                drv_entry, 0, 0);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_COSQ_CPU_NEW, 
                drv_entry, param0, 0);            
            break;
        case drvFieldActionMirrorIngress:
            if ((param0 != 0)||(!SOC_PORT_VALID(unit, (soc_port_t)param1)))
                return SOC_E_PARAM;
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_MIRROR_COPY, 
                drv_entry, 0, 0);
            if (SOC_FAILURE(rv)) {
                break;
            }
            break;
        case drvFieldActionMacDaKnown:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_DA_KNOWN, 
                drv_entry, 0, 0);            
            break;
        case drvFieldActionDoNotLearn:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_DIS_LRN, 
                drv_entry, 0, 0);            
            break;
        case drvFieldActionBypassStp:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_STP, 
                drv_entry, 0, 0);            
            break;
        case drvFieldActionBypassEap:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_EAP, 
                drv_entry, 0, 0);            
            break;
        case drvFieldActionBypassVlan:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_INGRESS_VLAN, 
                drv_entry, 0, 0);            
            break;
        case drvFieldActionFilters:
            if (param0 & DRV_FIELD_FILTER_EAP) {
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_EAP, 
                    drv_entry, 1, 0);            
                if (SOC_FAILURE(rv)) {
                    break;
                }
            }
            if (param0 & DRV_FIELD_FILTER_EGRESS_VLAN) {
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_EGRESS_VLAN, 
                    drv_entry, 1, 0);            
                if (SOC_FAILURE(rv)) {
                    break;
                }
            }
            if (param0 & DRV_FIELD_FILTER_INGRESS_VLAN) {
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_INGRESS_VLAN, 
                    drv_entry, 1, 0);            
                if (SOC_FAILURE(rv)) {
                    break;
                }
            }
            if (param0 & DRV_FIELD_FILTER_LAG) {
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_LAG, 
                    drv_entry, 1, 0);            
                if (SOC_FAILURE(rv)) {
                    break;
                }
            }
            if (param0 & DRV_FIELD_FILTER_PORT_MASK) {
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_PORT_MASK, 
                    drv_entry, 1, 0);            
                if (SOC_FAILURE(rv)) {
                    break;
                }
            }
            if (param0 & DRV_FIELD_FILTER_SA) {
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_SA, 
                    drv_entry, 1, 0);            
                if (SOC_FAILURE(rv)) {
                    break;
                }
            }
            if (param0 & DRV_FIELD_FILTER_STP) {                
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_STP, 
                    drv_entry, 1, 0);            
                if (SOC_FAILURE(rv)) {
                    break;
                }
            }
            if (param0 & DRV_FIELD_FILTER_TAGGED) {                            
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_TAGGED, 
                    drv_entry, 1, 0);               
                if (SOC_FAILURE(rv)) {
                    break;
                }
            }
            break;
        case drvFieldActionUpdateCounter:
            rv = SOC_E_NONE;
            break;
        case drvFieldActionDoNotModify:
            param0 = 1;
            rv = soc_CFP_ACT_POLm_field_set(unit, action_mem, DNMf, &param0);
            break;
        case drvFieldActionVportNew:
            rv = soc_CFP_ACT_POLm_field_set(unit, action_mem, VPORT_IDf, &param0);
            break;
        default:
            rv = SOC_E_UNAVAIL;
            break;
    }
    return rv;
}

int
_drv_tbx_ivm_action_remove(int unit, 
        void *drv_entry,  drv_field_action_t action)
{
    int rv = SOC_E_NONE;    

    switch (action) {
        case drvFieldActionVlanNew:
            rv = DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_VLAN_ID, 
                drv_entry, 0);
            break;
        case drvFieldActionPrioPktNew:
            /* If the action is removed, set port default pcp temporary. */
            SOC_IF_ERROR_RETURN(
                DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_SPCP_MARKING_POLICY, 
                    drv_entry, DRV_VM_ACTION_PCP_MARK_PORT_DEFAULT));
            SOC_IF_ERROR_RETURN(
                DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_CPCP_MARKING_POLICY, 
                    drv_entry, DRV_VM_ACTION_PCP_MARK_PORT_DEFAULT));
           break;
        case drvFieldActionNewClassId:           
            rv = DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_FLOW_ID, 
                drv_entry, 0);
            break;
        case drvFieldActionVportNew:           
            rv = DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_VPORT_ID, 
                drv_entry, 0);
            break;
        case drvFieldActionVportSpcpNew:           
            rv = DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_VPORT_SPCP, 
                drv_entry, 0);
            break;
        case drvFieldActionVportCpcpNew:           
            rv = DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_VPORT_CPCP, 
                drv_entry, 0);
            break;
        case drvFieldActionVportTcNew:           
            rv = DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_VPORT_TC, 
                drv_entry, 0);
            break;
        case drvFieldActionVportDpNew:           
            rv = DRV_VM_ACTION_SET(unit, DRV_VM_ACT_IVM_VPORT_DP, 
                drv_entry, 0);
            break;
        default:
            rv = SOC_E_UNAVAIL;
            break;
    }

    return rv;
}

int
_drv_tbx_evm_action_remove(int unit, 
        void *drv_entry,  drv_field_action_t action)
{
    int rv = SOC_E_NONE;    
    switch (action) {
        case drvFieldActionInnerVlanNew:
            SOC_IF_ERROR_RETURN(
                DRV_VM_ACTION_SET(unit, DRV_VM_ACT_EVM_NEW_CVID, 
                    drv_entry, 0));
            SOC_IF_ERROR_RETURN(
                DRV_VM_ACTION_SET(unit, DRV_VM_ACT_EVM_CTAG, 
                    drv_entry, 0));
            break;
        case drvFieldActionOuterVlanNew:
            SOC_IF_ERROR_RETURN(
                DRV_VM_ACTION_SET(unit, DRV_VM_ACT_EVM_NEW_SVID, 
                    drv_entry, 0));
            SOC_IF_ERROR_RETURN(
                DRV_VM_ACTION_SET(unit, DRV_VM_ACT_EVM_STAG, 
                    drv_entry, 0));
            break;
        case drvFieldActionOuterVlanPrioNew:
            if (SOC_IS_TB_AX(unit)) {
                return SOC_E_UNAVAIL;
            } else {
                SOC_IF_ERROR_RETURN(
                    DRV_VM_ACTION_SET(unit, DRV_VM_ACT_EVM_OUTER_PCP_REPLACE, 
                        drv_entry, 0));
                SOC_IF_ERROR_RETURN(
                    DRV_VM_ACTION_SET(unit, DRV_VM_ACT_EVM_NEW_OUTER_PCP, 
                        drv_entry, 0));
            }
            break;
        case drvFieldActionInnerVlanPrioNew:
            if (SOC_IS_TB_AX(unit)) {
                return SOC_E_UNAVAIL;
            } else {
                SOC_IF_ERROR_RETURN(
                    DRV_VM_ACTION_SET(unit, DRV_VM_ACT_EVM_INNER_PCP_REPLACE, 
                        drv_entry, 0));
                SOC_IF_ERROR_RETURN(
                    DRV_VM_ACTION_SET(unit, DRV_VM_ACT_EVM_NEW_INNER_PCP, 
                        drv_entry, 0));
            }
            break;
        default:
            rv = SOC_E_UNAVAIL;
            break;
    }

    return rv;
}


int
_drv_tbx_cfp_action_remove(int unit, void *drv_entry,  
        drv_field_action_t action, uint32 param0, uint32 param1)
{
    int rv = SOC_E_NONE;
    uint32 *action_mem;
    if (((drv_cfp_entry_t *)drv_entry)->flags & _DRV_CFP_SLICE_CHAIN){
        if (((drv_cfp_entry_t *)drv_entry)->flags & _DRV_CFP_SLICE_CONFIG_SLICE_MAIN){
            action_mem = ((drv_cfp_entry_t *)drv_entry)->cfp_chain->act_data;
        } else {
            action_mem = ((drv_cfp_entry_t *)drv_entry)->act_data;
        }
    } else {
            action_mem = ((drv_cfp_entry_t *)drv_entry)->act_data;
    }
    
    switch (action)
    {
        case drvFieldActionNewClassId:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CLASSFICATION_ID, 
                drv_entry, 0, 0);
            break;
        case drvFieldActionVlanNew:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_NEW_VLAN, 
                drv_entry, 0, 0);
            break;
        case drvFieldActionNewTc:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CHANGE_TC_CANCEL, 
                drv_entry, 0, 0);
            break;
        case drvFieldActionDropPrecedence:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_IB_CHANGE_DP_CANCEL, 
                drv_entry, 0, 0);
            if (SOC_FAILURE(rv)) {
                break;
            }
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_OB_CHANGE_DP_CANCEL, 
                drv_entry, 0, 0);
            break;
        case drvFieldActionGpDropPrecedence:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_IB_CHANGE_DP_CANCEL, 
                drv_entry, 0, 0);
            break;
        case drvFieldActionRpDropPrecedence:
        case drvFieldActionYpDropPrecedence:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_OB_CHANGE_DP_CANCEL, 
                drv_entry, 0, 0);
            break;
        case drvFieldActionRedirectMcast:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CHANGE_FWD_CANCEL, 
                drv_entry, 0, 0);
            break;
        case drvFieldActionRedirectPort:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CHANGE_FWD_CANCEL, 
                drv_entry, 0, 0);
            break;
        case drvFieldActionRedirectVportPort:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CHANGE_FWD_CANCEL, 
                drv_entry, 0, 0);            
            break;
        case drvFieldActionDrop:
        case drvFieldActionGpDrop:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CHANGE_FWD_CANCEL, 
                drv_entry, 0, 0);            
            if (SOC_FAILURE(rv)) {
                break;
            }
            if (action == drvFieldActionDrop) {
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_RATE_VIOLATE_DROP_CANCEL, 
                    drv_entry, 0, 0);            
            }
            break;
        case drvFieldActionLoopback:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CHANGE_FWD_CANCEL, 
                drv_entry, 0, 0);            
            break;
        case drvFieldActionRpDrop:
        case drvFieldActionYpDrop:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_RATE_VIOLATE_DROP_CANCEL, 
                drv_entry, 0, 0);            
            break;
        case drvFieldActionCopyToCpu:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CPU_COPY_CANCEL, 
                drv_entry, 0, 0);            
            break;
        case drvFieldActionCosQCpuNew:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CPU_COPY_CANCEL, 
                drv_entry, 0, 0);            
            break;
        case drvFieldActionMirrorIngress:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_MIRROR_COPY_CANCEL, 
                drv_entry, 0, 0);            
            break;
        case drvFieldActionMacDaKnown:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_DA_KNOWN_CANCEL, 
                drv_entry, 0, 0);            
            break;
        case drvFieldActionDoNotLearn:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_DIS_LRN_CANCEL, 
                drv_entry, 0, 0);            
            break;
        case drvFieldActionBypassStp:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_STP, 
                drv_entry, 1, 0);            
            break;
        case drvFieldActionBypassEap:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_EAP, 
                drv_entry, 1, 0);            
            break;
        case drvFieldActionBypassVlan:
            rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_INGRESS_VLAN, 
                drv_entry, 1, 0);            
            break;

        case drvFieldActionFilters:
            if (param0 & DRV_FIELD_FILTER_EAP) {
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_EAP, 
                    drv_entry, 0, 0);            
                if (SOC_FAILURE(rv)) {
                    break;
                }
            }
            if (param0 & DRV_FIELD_FILTER_EGRESS_VLAN) {
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_EGRESS_VLAN, 
                    drv_entry, 0, 0);            
                if (SOC_FAILURE(rv)) {
                    break;
                }
            }
            if (param0 & DRV_FIELD_FILTER_INGRESS_VLAN) {
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_INGRESS_VLAN, 
                    drv_entry, 0, 0);            
                if (SOC_FAILURE(rv)) {
                    break;
                }
            }
            if (param0 & DRV_FIELD_FILTER_LAG) {
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_LAG, 
                    drv_entry, 0, 0);            
                if (SOC_FAILURE(rv)) {
                    break;
                }
            }
            if (param0 & DRV_FIELD_FILTER_PORT_MASK) {
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_PORT_MASK, 
                    drv_entry, 0, 0);            
                if (SOC_FAILURE(rv)) {
                    break;
                }
            }
            if (param0 & DRV_FIELD_FILTER_SA) {
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_SA, 
                    drv_entry, 0, 0);            
                if (SOC_FAILURE(rv)) {
                    break;
                }
            }
            if (param0 & DRV_FIELD_FILTER_STP) {                
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_STP, 
                    drv_entry, 0, 0);            
                if (SOC_FAILURE(rv)) {
                    break;
                }
            }
            if (param0 & DRV_FIELD_FILTER_TAGGED) {                            
                rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_TAGGED, 
                    drv_entry, 0, 0);               
                if (SOC_FAILURE(rv)) {
                    break;
                }
            }
            break;
        case drvFieldActionUpdateCounter:
            rv = SOC_E_NONE;
            break;
        case drvFieldActionDoNotModify:
            param0 = 0;
            rv = soc_CFP_ACT_POLm_field_set(unit, action_mem, DNMf, &param0);
            break;
        case drvFieldActionVportNew:
            param0 = 0;
            rv = soc_CFP_ACT_POLm_field_set(unit, action_mem, VPORT_IDf, &param0);
            break;
        default:
            rv = SOC_E_UNAVAIL;
            break;
    }
    return rv;

}

int
_drv_tbx_fp_entry_alloc(int unit, int stage_id, void **entry)
{
    int memsize;

    if (stage_id == DRV_FIELD_STAGE_INGRESS){
        memsize = sizeof(drv_cfp_entry_t);            
    } else {
        memsize = sizeof(drv_vm_entry_t);
    }
    *entry = sal_alloc(memsize, "field drv entry");

    if (*entry == NULL) {
       return SOC_E_MEMORY;
    }
    sal_memset (*entry, 0, memsize);
    LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP: _tb_fp_entry_alloc %p\n"),
               *entry));
    return SOC_E_NONE;
}

int
_drv_tbx_fp_entry_copy(int unit, int stage_id, void *src_entry, void *dst_entry )
{
    int memsize;
    drv_cfp_tcam_t *cfp_chain;

    if (stage_id == DRV_FIELD_STAGE_INGRESS){
        memsize = sizeof(drv_cfp_entry_t);            
    } else {
        memsize = sizeof(drv_vm_entry_t);
    }
    sal_memcpy(dst_entry, src_entry, 
        memsize);


    if (stage_id == DRV_FIELD_STAGE_INGRESS) {
        if (((drv_cfp_entry_t *)src_entry)->flags & _DRV_CFP_SLICE_CHAIN){
            cfp_chain = sal_alloc(sizeof(drv_cfp_tcam_t),"cfp chain tcam");
            if (cfp_chain == NULL) {
                return SOC_E_MEMORY;
            }
            sal_memset(cfp_chain, 0, sizeof(drv_cfp_tcam_t));
            ((drv_cfp_entry_t *)dst_entry)->cfp_chain = cfp_chain;

            sal_memcpy(((drv_cfp_entry_t *)dst_entry)->cfp_chain, 
                ((drv_cfp_entry_t *)src_entry)->cfp_chain, 
                sizeof(drv_cfp_tcam_t));
            ((drv_cfp_entry_t *)dst_entry)->cfp_chain->chain_id  = -1;
        }
    }

    return SOC_E_NONE;
}

int
_drv_tbx_fp_entry_clear(int unit, int stage_id, void* entry, int op)
{
    int memsize = 0;


    if (op == DRV_FIELD_ENTRY_MEM_CLEAR_DATAMASK) {
        if (stage_id == DRV_FIELD_STAGE_INGRESS){
            memsize = sizeof(((drv_cfp_entry_t *)(entry))->tcam_data);
            sal_memset(((drv_cfp_entry_t *)(entry))->tcam_data, 0, memsize);
            memsize = sizeof(((drv_cfp_entry_t *)(entry))->tcam_mask);
            sal_memset(((drv_cfp_entry_t *)(entry))->tcam_mask, 0, memsize);    
#ifdef BCM_TB_SUPPORT            
            if (SOC_IS_TB(unit)) {
                ((drv_cfp_entry_t *)entry)->flags &= ~(_DRV_CFP_FIX_UDF_ALL_VALID \
                                                        |_DRV_CFP_FRAME_ALL|_DRV_CFP_UDF_VALID);
                ((drv_cfp_entry_t *)entry)->flags &= (CFP_53280_FIX_UDF_SET(0)-1);
            } 
#endif /* BCM_TB_SUPPORT */            
#ifdef BCM_VO_SUPPORT
            if (SOC_IS_VO(unit)) {
                ((drv_cfp_entry_t *)entry)->flags &= ~(_DRV_CFP_FRAME_L4_ALL \
                                                        |_DRV_CFP_FRAME_ALL|_DRV_CFP_FRAME_L2_ALL);
            }
#endif /* BCM_VO_SUPPORT */
            if (((drv_cfp_entry_t *)entry)->flags & _DRV_CFP_SLICE_CHAIN){
                memsize = 
                    sizeof(((drv_cfp_entry_t *)(entry))->cfp_chain->tcam_data);
                sal_memset(((drv_cfp_entry_t *)(entry))->cfp_chain->tcam_data,
                    0, memsize);
                memsize = 
                    sizeof(((drv_cfp_entry_t *)(entry))->cfp_chain->tcam_mask);
                sal_memset(((drv_cfp_entry_t *)(entry))->cfp_chain->tcam_mask, 
                    0, memsize);    
            }
        }else {
            memsize = sizeof(((drv_vm_entry_t *)(entry))->key_data);
            sal_memset(((drv_vm_entry_t *)(entry))->key_data, 0, memsize);
            memsize = sizeof(((drv_vm_entry_t *)(entry))->key_mask);
            sal_memset(((drv_vm_entry_t *)(entry))->key_mask, 0, memsize);    
        }
    }

    return  SOC_E_NONE;
}

#ifdef BCM_TB_SUPPORT
int
_drv_tb_fp_entry_tcam_slice_id_set(int unit, int stage_id, void *entry, int sliceId, void *slice_map)
{
    uint32 temp;
    uint32 id;
    drv_cfp_entry_t *drv_entry;
    int rv;

    if (DRV_FIELD_STAGE_INGRESS != stage_id)
        return SOC_E_PARAM;

    drv_entry = (drv_cfp_entry_t *)entry;

    drv_entry->slice_id = sliceId;
    drv_entry->slice_bmp = *((uint32 *)slice_map);

    if (drv_entry->flags & _DRV_CFP_SLICE_CHAIN) {
        drv_entry->flags |= _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN;    
        id = CFP_53280_SLICE_ID_WITH_CHAIN;    
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
        id = CFP_53280_SLICE_ID(sliceId);
        SOC_IF_ERROR_RETURN(
            DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_SLICE_ID, 
                drv_entry, &id));

        temp = 0x3;
        SOC_IF_ERROR_RETURN(
            DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_SLICE_ID, 
                drv_entry, &temp));
    }
    return SOC_E_NONE;
}
#endif /* BCM_TB_SUPPORT */

int
_drv_tbx_fp_entry_tcam_chain_mode_get(int unit, int stage_id, void *drv_entry, int sliceId, void *mode)    
{
    uint32 frame, id;
    drv_cfp_tcam_t *cfp_chain;

    *((int *)mode) = 0;

    frame = CFP_53280_L3_FRAME(sliceId);
    id = CFP_53280_SLICE_ID(sliceId);

    if (sliceId == CFP_53280_SLICE_ID_CHAIN_SLICE){
        *((int *)mode) = _DRV_CFP_SLICE_CHAIN_SINGLE;
        return SOC_E_NONE;
    }
    if ((id == CFP_53280_SLICE_ID_WITH_CHAIN) && 
        (frame != FP_TB_L3_FRM_FMT_CHAIN)) {
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

int
_drv_tbx_fp_entry_tcam_valid_control(int unit, int stage_id, void * drv_entry, int tcam_idx, uint32 *valid)
{
    uint32 temp;

    temp = *valid;
    switch (stage_id) {
        case DRV_FIELD_STAGE_INGRESS:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET
                    (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_VALID, 
                        drv_entry, &temp));

            SOC_IF_ERROR_RETURN(
                DRV_CFP_ENTRY_WRITE
                    (unit, tcam_idx, DRV_CFP_RAM_TCAM, drv_entry));
        break;

        case DRV_FIELD_STAGE_LOOKUP:
            SOC_IF_ERROR_RETURN(
                DRV_VM_FIELD_SET
                    (unit, DRV_VM_RAM_IVM_KEY_DATA, DRV_VM_FIELD_IVM_VALID, 
                        drv_entry, &temp));

            SOC_IF_ERROR_RETURN(
                DRV_VM_ENTRY_WRITE
                    (unit, tcam_idx, DRV_VM_RAM_IVM_KEY_DATA, drv_entry));
        break;

        case DRV_FIELD_STAGE_EGRESS:            
            SOC_IF_ERROR_RETURN(
                DRV_VM_FIELD_SET
                    (unit, DRV_VM_RAM_EVM_KEY_DATA, DRV_VM_FIELD_EVM_VALID, 
                        drv_entry, &temp));

            SOC_IF_ERROR_RETURN(
                DRV_VM_ENTRY_WRITE
                    (unit, tcam_idx, DRV_VM_RAM_EVM_KEY_DATA, drv_entry));
        break;
    }
    return SOC_E_NONE;
}

int
_drv_tbx_fp_entry_tcam_block_move(int unit, int stage_id, void *tcam_idx_old, int dest_index, void *amount)
{
    int mem_id;
    int rv;
    
    switch (stage_id){
        case DRV_FIELD_STAGE_INGRESS:
            mem_id = DRV_MEM_CFP_DATA_MASK;
            break;
        case DRV_FIELD_STAGE_LOOKUP:
            mem_id = DRV_MEM_IVM_KEY_DATA_MASK;
            break;
        case DRV_FIELD_STAGE_EGRESS:
            mem_id = DRV_MEM_EVM_KEY_DATA_MASK;
            break;
        default:
            return SOC_E_PARAM;
    }

    LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP: %s slice_idx src: %d  dst:%d amount %d\n"),
               FUNCTION_NAME(),*((int *)tcam_idx_old), dest_index, *((int *)amount)));
    rv = DRV_MEM_MOVE(unit, mem_id, *((int *)tcam_idx_old), dest_index, *((int *)amount), 1);
    SOC_IF_ERROR_RETURN(rv);

    return SOC_E_NONE;
}

int
_drv_tbx_fp_entry_tcam_move(int unit, int stage_id, void *drv_entry, int amount, void *counter)
{

    uint32      *tcam_idx_old, tcam_idx_new;
    int   tcam_idx_max;
    int tcam_type, mem_id;
    int rv;

    switch (stage_id){
        case DRV_FIELD_STAGE_INGRESS:
            tcam_type = DRV_DEV_PROP_CFP_TCAM_SIZE;
            mem_id = DRV_MEM_CFP_DATA_MASK;
            tcam_idx_old = &(((drv_cfp_entry_t *)drv_entry)->id);
            break;
        case DRV_FIELD_STAGE_LOOKUP:
            tcam_type = DRV_DEV_PROP_IVM_TCAM_SIZE;
            mem_id = DRV_MEM_IVM_KEY_DATA_MASK;
            tcam_idx_old = &(((drv_vm_entry_t *)drv_entry)->id);
            break;
        case DRV_FIELD_STAGE_EGRESS:
            tcam_type = DRV_DEV_PROP_EVM_TCAM_SIZE;
            mem_id = DRV_MEM_EVM_KEY_DATA_MASK;
            tcam_idx_old = &(((drv_vm_entry_t *)drv_entry)->id);
            break;
        default:
            return SOC_E_PARAM;
    }

    tcam_idx_new = *tcam_idx_old + amount;

    LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP: %s slice_idx old: %d  new:%d amount %d\n"),
               FUNCTION_NAME(),*tcam_idx_old, tcam_idx_new, amount));

    rv = DRV_DEV_PROP_GET(unit, tcam_type, (uint32 *)&tcam_idx_max);
    SOC_IF_ERROR_RETURN(rv);

    tcam_idx_max = tcam_idx_max -1;
    assert(0 <= (int)*tcam_idx_old && (int)*tcam_idx_old <= (int)tcam_idx_max);
    assert(0 <= (int)tcam_idx_new && (int)tcam_idx_new <= (int)tcam_idx_max);

    rv = DRV_MEM_MOVE(unit, mem_id, *tcam_idx_old, tcam_idx_new, 1, 1);
    SOC_IF_ERROR_RETURN(rv);

    *tcam_idx_old = tcam_idx_new;

    return SOC_E_NONE;
}

int
_drv_tbx_fp_entry_ivm_tcam_policy_install(int unit, void *entry, int tcam_idx)
{
    uint32 temp;
    drv_vm_entry_t *drv_entry = (drv_vm_entry_t *)entry;
    int retval;
    /* 
     * Get the value to set into each entry's valid field. 
     * The valid value is depend on chips. Let the driver service to handle it.
     */
    temp = 1;
   DRV_VM_FIELD_SET
        (unit, DRV_VM_RAM_IVM_KEY_DATA, 
        DRV_VM_FIELD_IVM_VALID, drv_entry, &temp);
   DRV_VM_FIELD_SET
       (unit, DRV_VM_RAM_IVM_KEY_MASK, 
       DRV_VM_FIELD_IVM_VALID, drv_entry, &temp);

    retval = DRV_VM_ENTRY_WRITE
        (unit, tcam_idx, DRV_VM_RAM_IVM_ACT, drv_entry);
    if (SOC_FAILURE(retval)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "FP Error: Write IVM Act entry index = %d fail.\n"),  
                   tcam_idx));
        return retval;
    }
    retval = DRV_VM_ENTRY_WRITE
        (unit, tcam_idx, DRV_VM_RAM_IVM_ALL, drv_entry);
    if (SOC_FAILURE(retval)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "FP Error: Write IVM Key entry index = %d fail.\n"),  
                   tcam_idx));
        return retval;
    }

    return retval;
}
int
_drv_tbx_fp_entry_evm_tcam_policy_install(int unit, void *entry, int tcam_idx)
{
    uint32 temp;
    drv_vm_entry_t *drv_entry = (drv_vm_entry_t *)entry;
    int retval;

    /* 
     * Get the value to set into each entry's valid field. 
     * The valid value is depend on chips. Let the driver service to handle it.
     */
    temp = 1;
   DRV_VM_FIELD_SET
        (unit, DRV_VM_RAM_EVM_KEY_DATA, 
        DRV_VM_FIELD_EVM_VALID, drv_entry, &temp);
   DRV_VM_FIELD_SET
       (unit, DRV_VM_RAM_EVM_KEY_MASK, 
       DRV_VM_FIELD_EVM_VALID, drv_entry, &temp);

    retval = DRV_VM_ENTRY_WRITE
        (unit, tcam_idx, DRV_VM_RAM_EVM_ACT, drv_entry);
    if (SOC_FAILURE(retval)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "FP Error: Write EVM Act entry index = %d fail.\n"),  
                   tcam_idx));
        return retval;
    }
    retval = DRV_VM_ENTRY_WRITE
        (unit, tcam_idx, DRV_VM_RAM_EVM_ALL, drv_entry);
    if (SOC_FAILURE(retval)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "FP Error: Write EVM Key entry index = %d fail.\n"),  
                   tcam_idx));
        return retval;
    }

    return retval;
}

#ifdef BCM_TB_SUPPORT
int
_drv_tb_fp_entry_cfp_tcam_policy_install(int unit, void *entry, int tcam_idx, 
        int tcam_chain_idx)
{
    uint32 temp;
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    int retval;
    int chain_id;

    /* 
     * Get the value to set into each entry's valid field. 
     * The valid value is depend on chips. Let the driver service to handle it.
     */
    temp = 1;
    SOC_IF_ERROR_RETURN(
        DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_VALID, drv_entry, &temp));
    SOC_IF_ERROR_RETURN(
        DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM_MASK, 
            DRV_CFP_FIELD_VALID, drv_entry, &temp));

   if (drv_entry->flags & _DRV_CFP_FIX_UDF_ALL_VALID) {
        /* any fix udf is qualified, valid bit should be set */
        temp = 1;
        SOC_IF_ERROR_RETURN(
            DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_TCAM, 
                DRV_CFP_FIELD_UDF_ALL_VALID,
                drv_entry, &temp));

        temp = 1;
        SOC_IF_ERROR_RETURN(
            DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_TCAM_MASK, 
                DRV_CFP_FIELD_UDF_ALL_VALID, drv_entry, &temp));
    }

    if ((drv_entry->flags & _DRV_CFP_FRAME_IP) ||        
        (drv_entry->flags & _DRV_CFP_FRAME_NONIP) ||
        (drv_entry->flags & _DRV_CFP_FIX_UDF_ALL_VALID) ||
        (drv_entry->flags & _DRV_CFP_UDF_VALID) ){
        temp = CFP_53280_L3_FRAME(drv_entry->slice_id);
        if (temp != CFP_53280_L3_FRAME_CHAIN) {
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

    if (!(drv_entry->flags & _DRV_CFP_ACTION_INIT)){
        /* BYPASS all the FILTER as default */
        retval = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_ALL, drv_entry, 0, 0);
        SOC_IF_ERROR_RETURN(retval);
        drv_entry->flags |= _DRV_CFP_ACTION_INIT;
    }

    if (tcam_chain_idx != -1) {
    /* entry with chain {slice0,slice3}*/        

        /* configure slice0 framing*/
        drv_entry->flags |= _DRV_CFP_SLICE_CONFIG_SLICE_MAIN;
        temp = CFP_53280_L3_FRAME(drv_entry->slice_id);
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
        retval = _drv_tb_cfp_qual_value_set(unit, drvFieldQualifyClassId, 
                        drv_entry, (uint32 *)&chain_id, &temp);
        if (SOC_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: set qualify chain_id %d fail.\n"),  
                       chain_id));
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
            (unit, tcam_chain_idx, DRV_CFP_RAM_ACT, drv_entry);
        if (SOC_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Write Act/Pol entry index = %d fail.\n"),  
                       tcam_idx));
            return retval;
        }
        retval = DRV_CFP_ENTRY_WRITE
            (unit, tcam_chain_idx, DRV_CFP_RAM_TCAM, drv_entry);
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
#endif /* BCM_TB_SUPPORT */

int
_drv_tbx_fp_entry_cfp_tcam_meter_install(int unit, void *entry, int tcam_idx, 
        int tcam_chain_idx)
{    
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    uint32 temp_meter[2];
    int retval;
   
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s meter %x %x %x\n"),
                 FUNCTION_NAME(),
                 drv_entry->meter_data[0],drv_entry->meter_data[1],
                 drv_entry->meter_data[2]));

    if (tcam_chain_idx != -1) {
        /* write the slice3 meter tcam*/
        retval = DRV_CFP_ENTRY_WRITE
            (unit, tcam_chain_idx, DRV_CFP_RAM_METER, drv_entry);
        /* clean slice0 meter tcam */
        memcpy(&temp_meter, drv_entry->meter_data, sizeof(temp_meter));
        memset(drv_entry->meter_data, 0, sizeof(temp_meter));
        retval = DRV_CFP_ENTRY_WRITE
            (unit, tcam_idx, DRV_CFP_RAM_METER, drv_entry);
        memcpy(drv_entry->meter_data, &temp_meter, sizeof(temp_meter));
    } else {
        /* write the slice0 meter tcam*/
        retval = DRV_CFP_ENTRY_WRITE
            (unit, tcam_idx, DRV_CFP_RAM_METER, drv_entry);
    }
        
    return retval;
}

int
_drv_tbx_fp_entry_cfp_tcam_reinstall(int unit, void *entry, int tcam_idx, 
        int mode)
{
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    int retval;

    if (mode & _DRV_CFP_SLICE_CHAIN){
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

    if (mode & _DRV_FP_ENTRY_CHAIN){
        drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
    }
    
    return SOC_E_NONE;
}

int
_drv_tbx_fp_entry_tcam_policy_install(int unit, int stage_id, void *entry, 
            int tcam_idx, int *tcam_chain_idx)
{
    int rv;
    switch(stage_id) {
        case DRV_FIELD_STAGE_INGRESS:
            rv = SOC_E_UNAVAIL;
#ifdef BCM_TB_SUPPORT            
            if (SOC_IS_TB(unit)) {
                rv = _drv_tb_fp_entry_cfp_tcam_policy_install(unit, entry, tcam_idx, 
                    *tcam_chain_idx);
            }
#endif /* BCM_TB_SUPPORT */            
#ifdef BCM_VO_SUPPORT
            if (SOC_IS_VO(unit)) {
                rv = _drv_vo_fp_entry_cfp_tcam_policy_install(unit, entry, tcam_idx, 
                    *tcam_chain_idx);
            }
#endif /* BCM_VO_SUPPORT */
            break;
        case DRV_FIELD_STAGE_LOOKUP:
            rv = _drv_tbx_fp_entry_ivm_tcam_policy_install(unit, entry, tcam_idx);
            break;
        case DRV_FIELD_STAGE_EGRESS:
            rv = _drv_tbx_fp_entry_evm_tcam_policy_install(unit, entry, tcam_idx);
            break;
        default:
            return SOC_E_PARAM;
    }
    return rv;
}
int
_drv_tbx_fp_entry_tcam_meter_install(int unit, int stage_id, void *entry, 
            int tcam_idx, int *tcam_chain_idx)
{
    int rv;
    switch(stage_id) {
        case DRV_FIELD_STAGE_INGRESS:
            rv = _drv_tbx_fp_entry_cfp_tcam_meter_install(unit, entry, tcam_idx, *tcam_chain_idx);
            break;
        case DRV_FIELD_STAGE_LOOKUP:
        case DRV_FIELD_STAGE_EGRESS:
        default:
            return SOC_E_PARAM;
    }
    return rv;
}

int
_drv_tbx_fp_entry_tcam_reinstall(int unit, int stage_id, void *entry, 
            int tcam_idx, int *mode)
{
    int rv;
    switch(stage_id) {
        case DRV_FIELD_STAGE_INGRESS:
            rv = _drv_tbx_fp_entry_cfp_tcam_reinstall(unit, entry, tcam_idx, *mode);
            break;
        case DRV_FIELD_STAGE_LOOKUP:
            rv = _drv_tbx_fp_entry_ivm_tcam_policy_install(unit, entry, tcam_idx);
            break;
        case DRV_FIELD_STAGE_EGRESS:
            rv = _drv_tbx_fp_entry_evm_tcam_policy_install(unit, entry, tcam_idx);
            break;
        default:
            return SOC_E_PARAM;
    }
    return rv;
}

int 
_drv_tbx_fp_entry_tcam_enable_set(int unit, int stage_id, int tcam_idx, 
                              int *tcam_chain_idx, int enable)
{
    int rv;
    void *drv_entry=NULL;
    uint32 temp;

    rv = _drv_tbx_fp_entry_alloc(unit, stage_id, &drv_entry);
    if (SOC_FAILURE(rv)) {
        return rv;
    }
    LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP: %s tcam_idx %d chain_idx %d\n"),
               FUNCTION_NAME(),
               tcam_idx,*tcam_chain_idx));
    switch(stage_id) {
        case DRV_FIELD_STAGE_INGRESS:
            if (*tcam_chain_idx != -1) {
                rv = _drv_tbx_fp_entry_tcam_chain_mode_get(unit, stage_id, drv_entry, 
                    CFP_53280_SLICE_ID_WITH_CHAIN, &temp);
                if (SOC_FAILURE(rv)) {
                   sal_free(drv_entry);
                    return rv;
                }
                ((drv_cfp_entry_t *)drv_entry)->flags |=_DRV_CFP_SLICE_CHAIN;
                ((drv_cfp_entry_t *)drv_entry)->flags |= 
                            _DRV_CFP_SLICE_CONFIG_SLICE_MAIN;
                rv = DRV_CFP_ENTRY_READ
                    (unit, *tcam_chain_idx, DRV_CFP_RAM_TCAM, drv_entry);
                if (SOC_FAILURE(rv)) {
                    sal_free(((drv_cfp_entry_t *)drv_entry)->cfp_chain);
                    ((drv_cfp_entry_t *)drv_entry)->cfp_chain = NULL;
                    sal_free(drv_entry);
                    return rv;
                }
               ((drv_cfp_entry_t *)drv_entry)->slice_id = 
                    CFP_53280_SLICE_ID_WITH_CHAIN;
                temp = 0;
                rv = _drv_tbx_fp_entry_tcam_valid_control(unit, stage_id, 
                        drv_entry, *tcam_chain_idx, &temp);
                if (SOC_FAILURE(rv)) {
                    sal_free(((drv_cfp_entry_t *)drv_entry)->cfp_chain);
                    ((drv_cfp_entry_t *)drv_entry)->cfp_chain = NULL;
                    sal_free(drv_entry);
                    return rv;
                }
                ((drv_cfp_entry_t *)drv_entry)->flags = 0;
                sal_free(((drv_cfp_entry_t *)drv_entry)->cfp_chain);
                ((drv_cfp_entry_t *)drv_entry)->cfp_chain = NULL;
            }
            rv = DRV_CFP_ENTRY_READ
                (unit, tcam_idx, DRV_CFP_RAM_TCAM, drv_entry);
            if (SOC_FAILURE(rv)) {
               sal_free(drv_entry);
                return rv;
            }
            break;
        case DRV_FIELD_STAGE_LOOKUP:
            rv = DRV_VM_ENTRY_READ
                (unit, tcam_idx, DRV_VM_RAM_IVM_KEY_DATA, drv_entry);
            break;
        case DRV_FIELD_STAGE_EGRESS:
            rv = DRV_VM_ENTRY_READ
                    (unit, tcam_idx, DRV_VM_RAM_EVM_KEY_DATA, drv_entry);
            break;
        default:
            rv = SOC_E_PARAM;
            break;
    }
    if (SOC_FAILURE(rv)) {
       sal_free(drv_entry);
        return rv;
    }

    if (enable) {
        temp = 1;
    } else {
        temp = 0;
    }
    rv = _drv_tbx_fp_entry_tcam_valid_control(unit, stage_id, drv_entry, tcam_idx, &temp);

    sal_free(drv_entry);
    return rv;
}

int 
_drv_tbx_fp_entry_tcam_clear(int unit, int stage_id, int tcam_idx, 
        int *tcam_chain_idx, int op)
{
    int mem_id, i, num;
    uint32 size;
    uint32 *entry;
    uint32 *tcam;    
    uint32    cfp_tcam[4] = {DRV_MEM_CFP_DATA_MASK, DRV_MEM_CFP_ACT, 
                            DRV_MEM_CFP_METER, DRV_MEM_CFP_STAT};
    uint32    ivm_tcam[2] = {DRV_MEM_IVM_KEY_DATA_MASK, DRV_MEM_IVM_ACT};
    uint32    evm_tcam[2] = {DRV_MEM_EVM_KEY_DATA_MASK, DRV_MEM_EVM_ACT};
    int rv = SOC_E_NONE;
    
    switch(stage_id) {
        case DRV_FIELD_STAGE_INGRESS:
            tcam = &cfp_tcam[0];
            num =  COUNTOF(cfp_tcam);
            break;
        case DRV_FIELD_STAGE_LOOKUP:
            tcam = &ivm_tcam[0];
            num =  COUNTOF(ivm_tcam);
            break;
        case DRV_FIELD_STAGE_EGRESS:
            tcam = &evm_tcam[0];
            num =  COUNTOF(evm_tcam);
            break;
        default:
            return SOC_E_PARAM;
    }

    if (op == DRV_FIELD_ENTRY_TCAM_DATAMASK_CLEAR) {
        /* over-write the num to 1 ,DATAMASK only*/
        num = 1;
    }
    
    for (i=0; i < num; i++) {
        mem_id =  *(tcam+i);
        rv = DRV_MEM_WIDTH_GET(unit, mem_id, &size);
        SOC_IF_ERROR_RETURN(rv);
        entry = sal_alloc(size, "tcam entry size");
        if (NULL == entry) {
            return SOC_E_MEMORY;
        }
        sal_memset(entry, 0, size);

        rv = DRV_MEM_FILL(unit, mem_id, tcam_idx, 1, entry);
    
        if (SOC_FAILURE(rv)) {
            sal_free(entry);
            return rv;
        }

        if (*tcam_chain_idx != -1) {
            rv = DRV_MEM_FILL(unit, mem_id, *tcam_chain_idx, 1, entry);
        }

        sal_free(entry);
        SOC_IF_ERROR_RETURN(rv);
    }

    return rv;
}
#ifdef BCM_TB_SUPPORT
int
_drv_tb_fp_cfp_qualify_support(int unit, drv_field_qset_t qset)
{
    int rv = SOC_E_NONE;
    int i;

    /* Qualifier checking */
    for (i = 0; i < drvFieldQualifyCount; i++) {
        if (DRV_FIELD_QSET_TEST(qset, i)) {
            rv = _drv_tb_ingress_qualify_support(unit, i);
            if (SOC_FAILURE(rv)){
                return rv;
            }
        }            
    }
    return rv;
}
#endif /* BCM_TB_SUPPORT */

int
_drv_tbx_fp_ivm_qualify_support(int unit, drv_field_qset_t qset)
{
    int rv = SOC_E_NONE;
    int i;
    /* Qualifier checking */
    for (i = 0; i < drvFieldQualifyCount; i++) {
        if (DRV_FIELD_QSET_TEST(qset, i)) {
            switch (i) {
                /* IVM qualifiers. */
                case drvFieldQualifyStageLookup:
                case drvFieldQualifyInterfaceClassPort:
                case drvFieldQualifyInPort:
                case drvFieldQualifyVlanFormat:
                case drvFieldQualifyOuterVlanId:
                case drvFieldQualifyRangeCheck:
                case drvFieldQualifyInnerVlanId:
                case drvFieldQualifyOuterVlanPri:
                case drvFieldQualifyInnerVlanPri:
                case drvFieldQualifyL2Format:
                case drvFieldQualifyEtherType:
                    break;
                 default:
                    return SOC_E_UNAVAIL;
            }
        }
    }
    return rv;
}

int
_drv_tbx_fp_evm_qualify_support(int unit, drv_field_qset_t qset)
{
    int rv = SOC_E_NONE;
    int i;

    /* Qualifier checking */
    for (i = 0; i < drvFieldQualifyCount; i++) {
        if (DRV_FIELD_QSET_TEST(qset, i)) {
            switch (i) {
                /* EVM qualifiers. */
                case drvFieldQualifyStageEgress:
                case drvFieldQualifyInPort:
                case drvFieldQualifyInVPort:
                case drvFieldQualifyFlowId:
                case drvFieldQualifyOutPort:
                case drvFieldQualifyOutVPort:
                    break;
                 default:
                    return SOC_E_UNAVAIL;
            }
        }
    }
    return rv;
}
int
_drv_tbx_fp_id_control_deinit(int unit)
{

    /* Release chain Id management. */
    if (CFP_CHAIN_USED(unit) != NULL) {
        sal_free(CFP_CHAIN_USED(unit));
        CFP_CHAIN_USED(unit) = NULL;
    }
    if (VM_FLOW_USED(unit) != NULL) {
        sal_free(VM_FLOW_USED(unit));
        VM_FLOW_USED(unit) = NULL;
    }
    if (VM_RANGER_USED(unit) != NULL) {
        sal_free(VM_RANGER_USED(unit));
        VM_RANGER_USED(unit) = NULL;
    }    
    return SOC_E_NONE;
}
int
_drv_tbx_fp_id_control_init(int unit)
{
    uint32 temp = 0;
    uint32 cpu_fid = 0;
    int rv;
    uint32 alloc_size = 0;

    /* Initialize chain Id management. */
    if (CFP_CHAIN_USED(unit) != NULL) {
        sal_free(CFP_CHAIN_USED(unit));
    }

    rv = DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_FLOW_ID_NUM, &temp);
    if (SOC_FAILURE(rv)){
        _drv_tbx_fp_id_control_deinit(unit);
        return SOC_E_MEMORY;

    }
    cpu_fid = temp - 1;

    CFP_CHAIN_SIZE(unit) = temp;
    alloc_size = SHR_BITALLOCSIZE(CFP_CHAIN_SIZE(unit));
    CFP_CHAIN_USED(unit) = sal_alloc(alloc_size, "CFP_CHAIN_ID");

    if (CFP_CHAIN_USED(unit) == NULL) {
        _drv_tbx_fp_id_control_deinit(unit);
        return SOC_E_MEMORY;
    }
    sal_memset(CFP_CHAIN_USED(unit), 0, alloc_size);

    if (VM_FLOW_USED(unit) != NULL) {
        sal_free(VM_FLOW_USED(unit));
    }
    VM_FLOW_SIZE(unit) = temp;
    alloc_size = SHR_BITALLOCSIZE(VM_FLOW_SIZE(unit));
    VM_FLOW_USED(unit) = sal_alloc(alloc_size, "VM_FLOW_ID");

    if (VM_FLOW_USED(unit) == NULL) {
        _drv_tbx_fp_id_control_deinit(unit);
        return SOC_E_MEMORY;
    }
    sal_memset(VM_FLOW_USED(unit), 0, alloc_size);

    /* Initialize cpu default flow id, it should be 4095. */
    VM_FLOW_USED_SET(unit, cpu_fid);
    VM_FLOW_CPU_DEFAULT(unit) = cpu_fid;

    if (VM_RANGER_USED(unit) != NULL) {
        sal_free(VM_RANGER_USED(unit));
    }

    rv = DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_IVM_RNG_NUM, &temp);
    if (SOC_FAILURE(rv)){
        _drv_tbx_fp_id_control_deinit(unit);
        return SOC_E_MEMORY;

    }

    VM_RANGER_SIZE(unit) = temp;
    VM_RANGER_USED(unit) = sal_alloc(sizeof(int) * VM_RANGER_SIZE(unit), "VM_RANGER_ID");
    if (VM_RANGER_USED(unit) == NULL) {
        _drv_tbx_fp_id_control_deinit(unit);
        return SOC_E_MEMORY;
    }
    sal_memset(VM_RANGER_USED(unit), 0, sizeof(int) * VM_RANGER_SIZE(unit));

    if (MAX_USER_FLOW_ID(unit) > (VM_FLOW_SIZE(unit) - 2)){
        /* for ThunderBolt flow id 4095 is reserved for cpu tx */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "TB_FLOW_ID_SIZE should't large than %d\n"),
                   VM_FLOW_SIZE(unit) - 2));
        return SOC_E_CONFIG;
    }
    return SOC_E_NONE;
}

int 
drv_tbx_fp_init(int unit,int stage_id)
{
    int rv;
    uint8 init_all, init_cfp, init_vm;
    
    init_all = 0;
    init_cfp = 0;
    init_vm = 0;

    if (stage_id == -1) {
        init_all = 1;
    } else if (stage_id == DRV_FIELD_STAGE_INGRESS){
        init_cfp = 1;
    } else {
        init_vm = 1;
    }

    if (init_all |init_cfp) {
        rv = DRV_CFP_INIT(unit);
        SOC_IF_ERROR_RETURN(rv);
    }
    if (init_all |init_vm) {
        rv = DRV_VM_INIT(unit);
        SOC_IF_ERROR_RETURN(rv);
    }
    rv = _drv_tbx_fp_id_control_init(unit);

    return rv;
}

int 
drv_tbx_fp_deinit(int unit,int stage_id)
{
    int rv;
    int port;
    uint8 deinit_all, deinit_cfp, deinit_vm;
    
    deinit_all = 0;
    deinit_cfp = 0;
    deinit_vm = 0;

    if (stage_id == -1) {
        deinit_all = 1;
    } else if (stage_id == DRV_FIELD_STAGE_INGRESS){
        deinit_cfp = 1;
    } else {
        deinit_vm = 1;
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

    if (deinit_all |deinit_vm) {
        rv = DRV_VM_DEINIT(unit);
        SOC_IF_ERROR_RETURN(rv);
    }
    rv =_drv_tbx_fp_id_control_deinit(unit);

    return rv;
}

int
drv_tbx_fp_id_control(int unit, int type, int op, int flags, int *id, uint32 *count)
{
    int rv = SOC_E_NONE;
    int loop, idx = -1;

    if (type == _DRV_FP_ID_CFP_CHAIN_ID) {
        switch(op) {
            case    _DRV_FP_ID_CTRL_ALLOC:
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
    } else if (type == _DRV_FP_ID_VM_FLOW_ID){
        switch(op) {
            case    _DRV_FP_ID_CTRL_ALLOC:
                if (flags == _DRV_FP_ID_GET_FROM_HEAD) {
                    /* get a free index from head */
                    for (loop = MAX_USER_FLOW_ID(unit)+1; loop < VM_FLOW_SIZE(unit); loop++) {
                        if (!VM_FLOW_USED_ISSET(unit, loop)) {
                            idx = loop;
                            break;
                        }
                    }
                } else {
                    /* get a free index from tail */
                    for (loop = VM_FLOW_SIZE(unit)-1; loop > MAX_USER_FLOW_ID(unit); loop--) {
                        if (!VM_FLOW_USED_ISSET(unit, loop)) {
                            idx = loop;
                            break;
                        }
                    }
                }
                if (idx == -1){
                    return SOC_E_FULL;
                }
                VM_FLOW_USED_SET(unit, idx);
                *id = idx;
                break;
            case _DRV_FP_ID_CTRL_FREE:
                if (VM_FLOW_CPU_DEFAULT(unit) != *id) {
                    /* 
                     * Flow id that dedicated for cpu default usage
                     * is not necessary to be freed. 
                     */
                    VM_FLOW_USED_CLR(unit, *id);
                }
                break;                
            case    _DRV_FP_ID_CTRL_GET_CPU_DEFAULT:
                *id = VM_FLOW_CPU_DEFAULT(unit);
                break;
            default:
                rv = SOC_E_UNAVAIL;
                break;
        }

    } else if (type == _DRV_FP_ID_VM_RANGE_ID){
        switch(op) {
            case    _DRV_FP_ID_CTRL_INC:
                VM_RANGER_USED_INC(unit, *id);
                break;
            case _DRV_FP_ID_CTRL_DEC:
                if (VM_RANGER_USED_COUNT(unit, *id) > 0) {
                        VM_RANGER_USED_DEC(unit, *id);
                } else {
                    /* Try to decrease the count of a ranger that no one is still used. */
                    rv = SOC_E_NOT_FOUND;
                }
                break;                
            case _DRV_FP_ID_CTRL_COUNT:
                *count = VM_RANGER_USED_COUNT(unit, *id);
                break;                
            default:
                rv = SOC_E_UNAVAIL;
                break;
        }

    }

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
drv_tbx_fp_entry_mem_control (int unit, int stage_id, int op, void *src_entry, void *dst_entry, void **alloc_entry)
{

    int rv = SOC_E_NONE;

    switch (op) {
        case DRV_FIELD_ENTRY_MEM_ALLOC:
            rv = _drv_tbx_fp_entry_alloc(unit, stage_id, alloc_entry);    
            break;    
        case DRV_FIELD_ENTRY_MEM_COPY:
            rv = _drv_tbx_fp_entry_copy(unit, stage_id, src_entry, dst_entry);
            break;                
        case DRV_FIELD_ENTRY_MEM_CLEAR_DATAMASK:
            rv = _drv_tbx_fp_entry_clear(unit, stage_id, src_entry, op);
            break;
    }
    return rv;
}

/*
DRV_FIELD_ENTRY_TCAM_SLICE_ID_SET
    set param1 as slice id ,param2 as slice_bmp in drv_entry.
DRV_FIELD_ENTRY_TCAM_CLEAR     
    give an index to clear this drv_entry from TCAM. param1:tcam_index param2: chain_index
DRV_FIELD_ENTRY_TCAM_REMOVE     
    give an index to unset entry's valid bit, param1:tcam_index param2: chain_index 
DRV_FIELD_ENTRY_TCAM_MOVE       
    move drv_entry only or both entry & param2(counter) by param1(amount) 
DRV_FIELD_ENTRY_TCAM_BLOCK_MOVE 
      block of entries move. drv_entry: src_idx, param1: dst_idx, param2: amount of blocks
DRV_FIELD_ENTRY_TCAM_POLICY_INSTALL 
    write the drv_entry and act to TCAM, param1:tcam_index. param2: chain_index
DRV_FIELD_ENTRY_TCAM_SW_INDEX_SET   
       assign the param1 to drv_entry->id
DRV_FIELD_ENTRY_TCAM_DATAMASK_CLEAR      
    give an index to clear this drv_entry data and mask only from TCAM. param1:tcam_index 
DRV_FIELD_ENTRY_TCAM_SW_FLAGS_SET       
    assign the param1 to drv_entry->flags
DRV_FIELD_ENTRY_TCAM_CHAIN_MODE_GET     
    mode:chain_mode get from the drv_entry by the giving slice_Id
DRV_FIELD_ENTRY_TCAM_CHAIN_DESTROY  
        destroy the memory crated for the chain entry support 
DRV_FIELD_ENTRY_TCAM_ENTRY_REINSTALL 
    reinstall the entry when the parity check error happened. param1: error index, param2: chain mode or not
DRV_FIELD_ENTRY_TCAM_METER_INSTALL
    write the meter to TCAM, param1:tcam_index. param2: chain_index
*/
int
drv_tbx_fp_entry_tcam_control(int unit, int stage_id, void* drv_entry, int op, 
        int param1, void *param2)
{
    int rv = SOC_E_NONE;

    switch (op) {
        case DRV_FIELD_ENTRY_TCAM_SLICE_ID_SET:
            rv = SOC_E_UNAVAIL;
#ifdef BCM_TB_SUPPORT            
            if (SOC_IS_TB(unit)) {
                rv = _drv_tb_fp_entry_tcam_slice_id_set(unit, stage_id, drv_entry, 
                    param1, param2);
            }
#endif /* BCM_TB_SUPPORT */            
#ifdef BCM_VO_SUPPORT
            if (SOC_IS_VO(unit)) {
                rv = _drv_vo_fp_entry_tcam_slice_id_set(unit, stage_id, drv_entry, 
                    param1, param2);
            }
#endif /* BCM_VO_SUPPORT */
            break;
        case DRV_FIELD_ENTRY_TCAM_MOVE:
            rv = _drv_tbx_fp_entry_tcam_move(unit, stage_id, drv_entry, param1, param2);
            break;
        case DRV_FIELD_ENTRY_TCAM_BLOCK_MOVE:
            rv = _drv_tbx_fp_entry_tcam_block_move(unit, stage_id, drv_entry, param1, param2);
            break;
        case DRV_FIELD_ENTRY_TCAM_REMOVE:
            rv = _drv_tbx_fp_entry_tcam_enable_set(unit, stage_id, param1, param2, FALSE);
            break;
        case DRV_FIELD_ENTRY_TCAM_ENABLE:
            rv = _drv_tbx_fp_entry_tcam_enable_set(unit, stage_id, param1, param2, TRUE);
            break;
        case DRV_FIELD_ENTRY_TCAM_CLEAR:
        case DRV_FIELD_ENTRY_TCAM_DATAMASK_CLEAR:            
            rv = _drv_tbx_fp_entry_tcam_clear(unit, stage_id, param1, param2, op);
            break;
        case DRV_FIELD_ENTRY_TCAM_POLICY_INSTALL:
            rv = _drv_tbx_fp_entry_tcam_policy_install(unit, stage_id, drv_entry, param1, param2);
            break;
        case DRV_FIELD_ENTRY_TCAM_METER_INSTALL:
            rv = _drv_tbx_fp_entry_tcam_meter_install(unit, stage_id, drv_entry, param1, param2);
            break;            
        case DRV_FIELD_ENTRY_TCAM_ENTRY_REINSTALL:
            rv = _drv_tbx_fp_entry_tcam_reinstall(unit, stage_id, drv_entry, param1, param2);
            break;                
        case DRV_FIELD_ENTRY_TCAM_SW_INDEX_SET:   
            if (DRV_FIELD_STAGE_INGRESS == stage_id) {
                ((drv_cfp_entry_t *)drv_entry)->id = param1;   
            } else {
                ((drv_vm_entry_t *)drv_entry)->id = param1;   
            }
            break;
        case DRV_FIELD_ENTRY_TCAM_SW_FLAGS_SET:
            if (DRV_FIELD_STAGE_INGRESS == stage_id) {
                *((uint32 *)param2) = ((drv_cfp_entry_t *)drv_entry)->flags;
                if (param1 != -1) {
                    ((drv_cfp_entry_t *)drv_entry)->flags &= ~(_DRV_CFP_FRAME_ALL);
                    ((drv_cfp_entry_t *)drv_entry)->flags |= param1;   
                }
            }
            break;
        case DRV_FIELD_ENTRY_TCAM_CHAIN_MODE_GET:
            rv = SOC_E_UNAVAIL;
#ifdef BCM_TB_SUPPORT            
            if (SOC_IS_TB(unit)){
                rv = _drv_tbx_fp_entry_tcam_chain_mode_get(unit, stage_id, drv_entry, 
                    param1, param2);
            }
#endif /* BCM_TB_SUPPORT */            
#ifdef BCM_VO_SUPPORT
            if(SOC_IS_VO(unit)) {
                rv = _drv_vo_fp_entry_tcam_chain_mode_get(unit, stage_id, drv_entry, 
                    param1, param2);
            }   
#endif /* BCM_VO_SUPPORT*/
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
    }

    return rv;
}

int
drv_tbx_fp_action_conflict(int unit, int stage_id, drv_field_action_t act1, drv_field_action_t act2)
{
    if(act1 != drvFieldActionFilters) {
        /* Two identical actions are forbidden. */
        _FIELD_ACTIONS_CONFLICT(act1);
    }
    /* No conflict actions of lookup and egress stages */

    if (DRV_FIELD_STAGE_INGRESS ==  stage_id) {
        switch(act1) {
            case drvFieldActionRedirectMcast:
            case drvFieldActionRedirectPort:
            case drvFieldActionRedirectVportPort:
            case drvFieldActionDrop:
            case drvFieldActionGpDrop:
            case drvFieldActionLoopback:
                _FIELD_ACTIONS_CONFLICT(drvFieldActionRedirectMcast);
                _FIELD_ACTIONS_CONFLICT(drvFieldActionRedirectPort);
                _FIELD_ACTIONS_CONFLICT(drvFieldActionRedirectVportPort);
                _FIELD_ACTIONS_CONFLICT(drvFieldActionDrop);
                _FIELD_ACTIONS_CONFLICT(drvFieldActionGpDrop);
                _FIELD_ACTIONS_CONFLICT(drvFieldActionLoopback);                
                break;
            case drvFieldActionFilters:
            case drvFieldActionBypassStp:
            case drvFieldActionBypassEap:
            case drvFieldActionBypassVlan:
            case drvFieldActionMeterConfig:
            case drvFieldActionNewClassId:
            case drvFieldActionVlanNew:
            case drvFieldActionNewTc:
            case drvFieldActionDropPrecedence:
            case drvFieldActionGpDropPrecedence:
            case drvFieldActionRpDropPrecedence:
            case drvFieldActionCopyToCpu:
            case drvFieldActionCosQCpuNew:
            case drvFieldActionMirrorIngress:
            case drvFieldActionMacDaKnown:
            case drvFieldActionDoNotLearn:
            case drvFieldActionVportNew:                
                break;
            default:
                break;
        }
    }
    return (SOC_E_NONE);
}

int
drv_tbx_fp_action_support_check(int unit, int stage_id, 
            drv_field_action_t action)
{

    int rv = SOC_E_NONE;

    if (DRV_FIELD_STAGE_LOOKUP ==  stage_id) {
        switch (action) {
            case drvFieldActionPrioPktNew:
            case drvFieldActionVlanNew:
            case drvFieldActionNewClassId:
            case drvFieldActionVportNew:
            case drvFieldActionVportSpcpNew:
            case drvFieldActionVportCpcpNew:
            case drvFieldActionVportTcNew:
            case drvFieldActionVportDpNew:
                rv = SOC_E_NONE;
                break;
            default:
                rv = SOC_E_UNAVAIL;
        }  
    }

    if (DRV_FIELD_STAGE_EGRESS ==  stage_id) {
        switch (action) {
            case drvFieldActionOuterVlanNew:
            case drvFieldActionInnerVlanNew:
                rv = SOC_E_NONE;
                break;
            case drvFieldActionOuterVlanPrioNew:
            case drvFieldActionInnerVlanPrioNew:
                if (SOC_IS_TB_AX(unit)) {
                    return SOC_E_UNAVAIL;
                } else {
                    rv = SOC_E_NONE;
                }
                break;
            default:
                rv = SOC_E_UNAVAIL;
        }  
    }

    if (DRV_FIELD_STAGE_INGRESS ==  stage_id) {
        switch (action) {
            case drvFieldActionUpdateCounter:
            case drvFieldActionMeterConfig:
            case drvFieldActionNewClassId:
            case drvFieldActionVlanNew:
            case drvFieldActionNewTc:
            case drvFieldActionDropPrecedence:
            case drvFieldActionGpDropPrecedence:
            case drvFieldActionRpDropPrecedence:
            case drvFieldActionRedirectMcast:
            case drvFieldActionRedirectPort:
            case drvFieldActionRedirectVportPort:
            case drvFieldActionDrop:
            case drvFieldActionGpDrop:
            case drvFieldActionRpDrop:                
            case drvFieldActionLoopback:
            case drvFieldActionCopyToCpu:
            case drvFieldActionCosQCpuNew:
            case drvFieldActionMirrorIngress:
            case drvFieldActionMacDaKnown:
            case drvFieldActionDoNotLearn:
            case drvFieldActionFilters:
            case drvFieldActionBypassStp:
            case drvFieldActionBypassEap:
            case drvFieldActionBypassVlan:
                rv = SOC_E_NONE;
                break;
            case drvFieldActionYpDropPrecedence:
            case drvFieldActionYpDrop:                
            case drvFieldActionDoNotModify:                
                if (SOC_IS_TB_AX(unit)) {
                    return SOC_E_UNAVAIL;
                } else {
                    rv = SOC_E_NONE;
                }
                break;
            case drvFieldActionVportNew:
                if (SOC_IS_VO(unit)) {
                    rv = SOC_E_NONE;
                } else {
                    return SOC_E_UNAVAIL;
                }
                break;

            default:
                rv = SOC_E_UNAVAIL;
        }  
        return (rv);
    }    
    return (rv);

}

int
drv_tbx_fp_action_add(int unit, int stage_id, void *drv_entry, 
        drv_field_action_t action, uint32 param0, uint32 param1)
{
    int rv = SOC_E_UNAVAIL;
    switch (stage_id){
        case DRV_FIELD_STAGE_INGRESS:  
            rv = _drv_tbx_cfp_action_add(unit, drv_entry, action, param0, param1);
            break;
        case DRV_FIELD_STAGE_LOOKUP:  
            rv = _drv_tbx_ivm_action_add(unit, drv_entry, action, param0, param1);
            break;
        case DRV_FIELD_STAGE_EGRESS:  
            rv = _drv_tbx_evm_action_add(unit, drv_entry, action, param0, param1);
            break;            
        default:
            break;
    }
    return rv;
}
int
drv_tbx_fp_action_remove(int unit, int stage_id, void *drv_entry, 
        drv_field_action_t action, uint32 param0, uint32 param1)
{
    int rv = SOC_E_UNAVAIL;
    switch (stage_id){
        case DRV_FIELD_STAGE_INGRESS:  
            rv = _drv_tbx_cfp_action_remove(unit, drv_entry, action, param0, param1);
            break;
        case DRV_FIELD_STAGE_LOOKUP:  
            rv = _drv_tbx_ivm_action_remove(unit, drv_entry, action);
            break;
        case DRV_FIELD_STAGE_EGRESS:  
            rv = _drv_tbx_evm_action_remove(unit, drv_entry, action);
            break;            
        default:
            break;
    }
    return rv;

}

/* API has the resbosible to  free the entry get from drv_bcm53280_fp_selcode_mode_get() */
int
drv_tbx_fp_selcode_mode_get(int unit, int stage_id, 
        void*  qset,  int mode, int8 *slice_id, uint32 *slice_map, void **entry)
{
    int rv =  SOC_E_NONE;
    void *drv_entry = NULL;
    uint32  temp, flag, map = 0;
    drv_field_qset_t drv_qset;

    sal_memcpy(&drv_qset, qset, sizeof(drv_field_qset_t));
    switch (stage_id){
        case DRV_FIELD_STAGE_INGRESS:  
           drv_entry = sal_alloc(sizeof(drv_cfp_entry_t),"cfp entry");
            if (drv_entry == NULL){
                return SOC_E_MEMORY;
            }

            sal_memset(drv_entry, 0, sizeof(drv_cfp_entry_t));
#ifdef BCM_TB_SUPPORT            
            if (SOC_IS_TB(unit)) {
                rv = _drv_tb_qset_to_cfp(unit, drv_qset, 
                            (drv_cfp_entry_t *)drv_entry);
            }
#endif /* BCM_TB_SUPPORT */            
#ifdef BCM_VO_SUPPORT
            if (SOC_IS_VO(unit)) {
                rv = _drv_vo_qset_to_cfp(unit, drv_qset, 
                            (drv_cfp_entry_t *)drv_entry, mode);
            }
#endif /* BCM_VO_SUPPORT */
            if (SOC_FAILURE(rv)) {
                sal_free(drv_entry);
                return rv;
            }

            if (mode == DRV_FIELD_GROUP_MODE_DOUBLE) {
                flag = DRV_FIELD_QUAL_CHAIN;
            } else if (mode == DRV_FIELD_GROUP_MODE_SINGLE){
                flag = DRV_FIELD_QUAL_SINGLE;
            } else {
                flag = 0;
            }

            rv = DRV_CFP_SLICE_ID_SELECT
                            (unit, drv_entry, &temp, flag);
            if (SOC_FAILURE(rv)) {
                sal_free(drv_entry);
                return rv;
            }
            map = ((drv_cfp_entry_t *)drv_entry)->slice_bmp;
            break;
        case DRV_FIELD_STAGE_LOOKUP:  
            drv_entry = sal_alloc(sizeof(drv_vm_entry_t),"ivm entry");
            if (drv_entry == NULL)
                return SOC_E_MEMORY;
            sal_memset(drv_entry, 0, sizeof(drv_vm_entry_t));

            map = 0;
            rv = DRV_VM_FORMAT_ID_SELECT
                            (unit, drv_entry, &temp, 0);
            if (SOC_FAILURE(rv)) {
                sal_free(drv_entry);
                return rv;
            }
            break;
        case DRV_FIELD_STAGE_EGRESS:  
            drv_entry = sal_alloc(sizeof(drv_vm_entry_t),"evm entry");
            if (drv_entry == NULL)
                return SOC_E_MEMORY;
            sal_memset(drv_entry, 0, sizeof(drv_vm_entry_t));

            map = 0;
            rv = DRV_VM_FORMAT_ID_SELECT
                            (unit, drv_entry, &temp, 0);
            if (SOC_FAILURE(rv)) {
                sal_free(drv_entry);
                return rv;
            }
            break;
        default:
            rv = SOC_E_PARAM;
            break;
    }

    if (SOC_FAILURE(rv)) {
        return rv;
    }

    *slice_id = temp;
    *entry = drv_entry;
    *slice_map = map;
    return rv;

}

int
drv_tbx_fp_selcode_to_qset(int unit,  int stage_id, int slice_id, 
                void *qset) 
{
    int rv;
    drv_field_qset_t *drv_qset = (drv_field_qset_t *)qset;
    switch (stage_id){
        case DRV_FIELD_STAGE_INGRESS:
            rv =_drv_tbx_cfp_selcode_to_qset(unit, slice_id, drv_qset);
            break;
        case DRV_FIELD_STAGE_LOOKUP:
            rv = _drv_tbx_ivm_selcode_to_qset(unit, slice_id, drv_qset);
            break;
        case DRV_FIELD_STAGE_EGRESS:
            rv = _drv_tbx_evm_selcode_to_qset(unit, slice_id, drv_qset);
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

int
drv_tbx_fp_qualify_support(int unit, int stage_id, void* qset)
{
    int rv;
    drv_field_qset_t drv_qset;
    
    sal_memcpy(&drv_qset, qset, sizeof(drv_field_qset_t));

    switch (stage_id){
        case DRV_FIELD_STAGE_INGRESS:
            rv = SOC_E_UNAVAIL;
#ifdef BCM_TB_SUPPORT            
            if (SOC_IS_TB(unit)) {
                rv = _drv_tb_fp_cfp_qualify_support(unit, drv_qset);
            }
#endif /* BCM_TB_SUPPORT */            
#ifdef BCM_VO_SUPPORT
            if (SOC_IS_VO(unit)) {
                rv = _drv_vo_fp_cfp_qualify_support(unit, drv_qset);
            }
#endif /* BCM_VO_SUPPORT */
            break;
        case DRV_FIELD_STAGE_LOOKUP:
            rv = _drv_tbx_fp_ivm_qualify_support(unit, drv_qset);
            break;
        case DRV_FIELD_STAGE_EGRESS:
            rv = _drv_tbx_fp_evm_qualify_support(unit, drv_qset);
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;


}

int
drv_tbx_fp_qual_value_get(int unit, int stage_id, drv_field_qualify_t qual, void *drv_entry, uint32* p_data, uint32* p_mask)
{
    int rv;

    switch(stage_id){
        case DRV_FIELD_STAGE_INGRESS:
            rv = SOC_E_UNAVAIL;
#ifdef BCM_TB_SUPPORT            
            if (SOC_IS_TB(unit)) {
                rv = _drv_tb_cfp_qual_value_get 
                            (unit, qual, drv_entry, p_data, p_mask);
            }
#endif            
#ifdef BCM_VO_SUPPORT
            if (SOC_IS_VO(unit)) {
                rv = _drv_vo_cfp_qual_value_get
                            (unit, qual, drv_entry, p_data, p_mask);            
            }
#endif /* BCM_VO_SUPPORT */            
            break;
        case DRV_FIELD_STAGE_LOOKUP:
            rv = _drv_tbx_ivm_qual_value_get 
                        (unit, qual, drv_entry, p_data, p_mask);
            break;
        case DRV_FIELD_STAGE_EGRESS:
            rv = _drv_tbx_evm_qual_value_get 
                (unit, qual, drv_entry, p_data, p_mask);
            break;
        default:
            rv = SOC_E_UNAVAIL;
            break;
    }
    return rv;
}

int
drv_tbx_fp_qual_value_set(int unit, int stage_id, 
        drv_field_qualify_t qual, void *drv_entry, uint32* p_data, 
            uint32* p_mask)
{
    int rv;
    int slice_id;    

    LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP: %s qual %d \n"),
               FUNCTION_NAME(),qual));
    switch(stage_id) {
        case DRV_FIELD_STAGE_INGRESS:
            rv = SOC_E_UNAVAIL;
#ifdef BCM_TB_SUPPORT            
            if(SOC_IS_TB(unit)){
                rv = _drv_tb_cfp_slice_id_update(unit, qual, drv_entry,&slice_id);
                SOC_IF_ERROR_RETURN(rv);
                
                rv = _drv_tb_cfp_qual_value_set 
                            (unit, qual, drv_entry, p_data, p_mask);
                SOC_IF_ERROR_RETURN(rv);
                DRV_FIELD_QSET_ADD(((drv_cfp_entry_t *)drv_entry)->drv_qset, qual);
                if (slice_id != -1){
                    rv = (SOC_E_NONE |(slice_id+1));
                }            
            }
#endif /* BCM_TB_SUPPORT */           
      
#ifdef BCM_VO_SUPPORT
            if(SOC_IS_VO(unit)) {
                rv = _drv_vo_cfp_qual_value_set
                            (unit, qual, drv_entry, p_data, p_mask);
                SOC_IF_ERROR_RETURN(rv);
                DRV_FIELD_QSET_ADD(((drv_cfp_entry_t *)drv_entry)->drv_qset, qual);

            }
#endif /* BCM_VO_SUPPORT */
            break;
        case DRV_FIELD_STAGE_LOOKUP:
            rv = _drv_tbx_ivm_qual_value_set 
                        (unit, qual, drv_entry, p_data, p_mask);
            break;
        case DRV_FIELD_STAGE_EGRESS:
            rv = _drv_tbx_evm_qual_value_set 
                (unit, qual, drv_entry, p_data, p_mask);
            break;
        default:
            rv = SOC_E_PARAM;
            break;        
    }
    return rv;
}

int
drv_tbx_fp_udf_value_set(int unit, int stage_id, uint32 udf_idex, void *drv_entry, uint32* p_data, uint32* p_mask)
{

    int rv;
    int slice_id;
    switch(stage_id) {
        case DRV_FIELD_STAGE_INGRESS:
            rv = SOC_E_UNAVAIL;
#ifdef BCM_TB_SUPPORT            
            if (SOC_IS_TB(unit)){
                rv = _drv_tb_cfp_slice_id_update_udf(unit, udf_idex, drv_entry,&slice_id);
                SOC_IF_ERROR_RETURN(rv);
                
                rv = _drv_tb_cfp_udf_value_set 
                            (unit, udf_idex, drv_entry, p_data, p_mask);
                SOC_IF_ERROR_RETURN(rv);
                if (slice_id != -1){
                    rv = (SOC_E_NONE |(slice_id+1));
                }                        
                SHR_BITSET(((drv_cfp_entry_t *)drv_entry)->drv_qset.udf_map, udf_idex);
            }
#endif /* BCM_TB_SUPPORT */            
#ifdef BCM_VO_SUPPORT
            if(SOC_IS_VO(unit)) {            
                slice_id = ((drv_cfp_entry_t *)drv_entry)->slice_id;
                rv = _drv_vo_cfp_udf_value_set 
                            (unit, udf_idex, drv_entry, p_data, p_mask);
                SOC_IF_ERROR_RETURN(rv);
                if (((drv_cfp_entry_t *)drv_entry)->slice_id != slice_id){
                    rv = (SOC_E_NONE | (((drv_cfp_entry_t *)drv_entry)->slice_id+1));
                }
                SHR_BITSET(((drv_cfp_entry_t *)drv_entry)->drv_qset.udf_map, udf_idex);
            }            
#endif /* BCM_VO_SUPPORT */
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
drv_tbx_fp_udf_value_get(int unit, int stage_id, uint32 udf_idex, void *drv_entry, uint32* p_data, uint32* p_mask)
{
    int rv;

    switch(stage_id) {
        case DRV_FIELD_STAGE_INGRESS:
            rv = SOC_E_UNAVAIL;
#ifdef BCM_TB_SUPPORT            
            if (SOC_IS_TB(unit)) {
                rv = _drv_tb_cfp_udf_value_get 
                        (unit, udf_idex, drv_entry, p_data, p_mask);
            }
#endif /* BCM_TB_SUPPORT */            
#ifdef BCM_VO_SUPPORT
            if (SOC_IS_VO(unit)) {
                rv = _drv_vo_cfp_udf_value_get 
                        (unit, udf_idex, drv_entry, p_data, p_mask);
            }
#endif /* BCM_VO_SUPPORT */
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
drv_tbx_fp_tcam_parity_check(int unit, drv_fp_tcam_checksum_t *drv_fp_tcam_chksum)
{
    int rv;
    uint64  reg_val;
    uint32 cfp_error, ivm_error, evm_error;

    sal_memset(drv_fp_tcam_chksum, 0, sizeof(drv_fp_tcam_checksum_t));

    rv = REG_READ_TCAM_CHKSUM_STSr(unit, (void *)&reg_val);
    SOC_IF_ERROR_RETURN(rv);
    LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "DRV_FP: %s reg_val hi %x lo %x\n"),
               FUNCTION_NAME(),COMPILER_64_HI(reg_val),COMPILER_64_LO(reg_val)));    

    rv = soc_TCAM_CHKSUM_STSr_field_get(unit, (void *)&reg_val, 
        CFP_TCAM_CHKSUM_ERRf, &cfp_error);
    SOC_IF_ERROR_RETURN(rv);

    rv = soc_TCAM_CHKSUM_STSr_field_get(unit, (void *)&reg_val, 
        IVM_TCAM_CHKSUM_ERRf, &ivm_error);
    SOC_IF_ERROR_RETURN(rv);

    rv = soc_TCAM_CHKSUM_STSr_field_get(unit, (void *)&reg_val, 
        EVM_TCAM_CHKSUM_ERRf, &evm_error);
    SOC_IF_ERROR_RETURN(rv);

   if (cfp_error) {
        rv = soc_TCAM_CHKSUM_STSr_field_get(unit, (void *)&reg_val, 
            CFP_TCAM_CHKSUM_ADDRf, &(drv_fp_tcam_chksum->stage_ingress_addr));
        SOC_IF_ERROR_RETURN(rv);
        drv_fp_tcam_chksum->tcam_error |= 1 << (DRV_FIELD_STAGE_INGRESS);
   }

   if (ivm_error) {
        rv = soc_TCAM_CHKSUM_STSr_field_get(unit, (void *)&reg_val, 
            IVM_TCAM_CHKSUM_ADDRf, &(drv_fp_tcam_chksum->stage_lookup_addr));
        SOC_IF_ERROR_RETURN(rv);
        drv_fp_tcam_chksum->tcam_error |= 1 << (DRV_FIELD_STAGE_LOOKUP);
   }
   
   if (evm_error) {
        rv = soc_TCAM_CHKSUM_STSr_field_get(unit, (void *)&reg_val, 
            EVM_TCAM_CHKSUM_ADDRf, &(drv_fp_tcam_chksum->stage_egress_addr));
        SOC_IF_ERROR_RETURN(rv);
        drv_fp_tcam_chksum->tcam_error |= 1 << (DRV_FIELD_STAGE_EGRESS);
   }

    return SOC_E_NONE;
}

int
_drv_tbx_fp_policer_support_check(int unit, int stage_id, drv_policer_config_t *policer_cfg)
{
    int rv;

    if (stage_id != DRV_FIELD_STAGE_INGRESS) {
        return SOC_E_PARAM;    
    }
    switch (policer_cfg->mode) {
        case drvPolicerModeCommitted:
        case drvPolicerModeGreen:
        case drvPolicerModePassThrough:
            rv = SOC_E_NONE;
            break;
        case drvPolicerModeSrTcm:
        case drvPolicerModeTrTcmDs:
        case drvPolicerModeCoupledTrTcmDs:
            if (SOC_IS_TB_AX(unit)) {
                return SOC_E_UNAVAIL;
            } else {
                rv = SOC_E_NONE;
            }
            break;
        default:
            rv = SOC_E_PARAM;
            break;
    }
    return (rv);

}

int
_drv_tbx_fp_policer_config_b0(int unit, void *entry, drv_policer_config_t *policer_cfg)
{
    int rv = SOC_E_NONE;
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    uint32 cir_en, eir_en, color_aware, coupling, bucket_size, ref_cnt;
    uint32 *meter;
    
    drv_entry->pl_cfg = policer_cfg;

    switch(policer_cfg->mode){
        case drvPolicerModeTrTcmDs:
        case drvPolicerModeCoupledTrTcmDs:
        case drvPolicerModeSrTcm:
            cir_en = 1;
            eir_en = 1;
            break;
        case drvPolicerModeCommitted:
            cir_en = 1;
            eir_en = 0;
            break;
        case drvPolicerModeGreen:
        case drvPolicerModePassThrough:
            cir_en = 0;
            eir_en = 0;
            break;            
        default:
            return SOC_E_PARAM;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s mode %d CIR %s EIR %s \n"),
                 FUNCTION_NAME(), policer_cfg->mode,
                 cir_en ? "enable":"disable", eir_en ? "enable":"disable"));
    meter = drv_entry->meter_data;
    sal_memset(meter, 0, sizeof(*meter));

    soc_CFP_RATE_METERm_field_set(unit, meter, EN_CIR_RMf, &cir_en);
    soc_CFP_RATE_METERm_field_set(unit, meter, EN_EIR_RMf, &eir_en);    

    if (cir_en != 0){
        rv = DRV_CFP_METER_RATE_TRANSFORM(unit, policer_cfg->ckbits_sec, 
            policer_cfg->ckbits_burst, &bucket_size, &ref_cnt, NULL);
        SOC_IF_ERROR_RETURN(rv);
        soc_CFP_RATE_METERm_field_set(unit, meter, CIR_BKT_SIZEf, &bucket_size);
        soc_CFP_RATE_METERm_field_set(unit, meter, CIR_REF_CNTf, &ref_cnt);
    }
    if (eir_en != 0) {    
        rv = DRV_CFP_METER_RATE_TRANSFORM(unit, policer_cfg->pkbits_sec, 
            policer_cfg->pkbits_burst, &bucket_size, &ref_cnt, NULL);
        SOC_IF_ERROR_RETURN(rv);
        if (drvPolicerModeSrTcm == policer_cfg->mode) {
            /* SrTcm set EIR ref_cnt=0. enable rate copling */
            if (0 == bucket_size) {
                bucket_size = CFP_53280_METER_BURST_MAX(unit);
            }
            ref_cnt = 0;
        } 
        soc_CFP_RATE_METERm_field_set(unit, meter, EIR_BKT_SIZEf, &bucket_size);
        soc_CFP_RATE_METERm_field_set(unit, meter, EIR_REF_CNTf, &ref_cnt);

        if ((policer_cfg->flags & DRV_POLICER_COLOR_BLIND) != 0){
            color_aware = 0;
        } else {
            color_aware = 1;
        }
        
        if ((drvPolicerModeCoupledTrTcmDs == policer_cfg->mode)||
           (drvPolicerModeSrTcm == policer_cfg->mode)) {
            coupling = 1;
        } else {
            coupling = 0;
        }

        soc_CFP_RATE_METERm_field_set(unit, meter, CMf, &color_aware);       
        soc_CFP_RATE_METERm_field_set(unit, meter, CFf, &coupling);
    }

    return SOC_E_NONE;
}
int
_drv_tbx_fp_policer_config(int unit,  int stage_id, void *entry, drv_policer_config_t *policer_cfg)
{
    int rv = SOC_E_NONE;
    drv_cfp_entry_t *drv_entry;
    uint32 cir_en, bucket_size, ref_cnt;
    uint32 *meter;

    if (stage_id != DRV_FIELD_STAGE_INGRESS) {
        return SOC_E_PARAM;    
    }

    if (SOC_IS_TB_AX(unit)) {
        drv_entry = (drv_cfp_entry_t *)entry;
        drv_entry->pl_cfg = policer_cfg;

        switch(policer_cfg->mode){
            case drvPolicerModeCommitted:
                cir_en = 1;
                break;
            case drvPolicerModeGreen:
            case drvPolicerModePassThrough:
                cir_en = 0;
                break;            
            default:
                return SOC_E_PARAM;
        }
        meter = drv_entry->meter_data;
        sal_memset(meter, 0, sizeof(drv_entry->meter_data));

        soc_CFP_METERm_field_set(unit, meter, RM_ENf, &cir_en);

        if (cir_en){
            rv = DRV_CFP_METER_RATE_TRANSFORM(unit, policer_cfg->ckbits_sec, 
                policer_cfg->ckbits_burst, &bucket_size, &ref_cnt, NULL);
            SOC_IF_ERROR_RETURN(rv);
            soc_CFP_METERm_field_set(unit, meter, BKTSIZEf, &bucket_size);
            soc_CFP_METERm_field_set(unit, meter, RFSHCNTf, &ref_cnt);
        }
        return SOC_E_NONE;
    } else {
        return _drv_tbx_fp_policer_config_b0(unit, entry, policer_cfg);
    }

}
int
_drv_tbx_fp_policer_free(int unit,  int stage_id, void *entry)
{
    drv_cfp_entry_t *drv_entry;
    
    if (stage_id != DRV_FIELD_STAGE_INGRESS) {
        return SOC_E_PARAM;    
    }

    drv_entry = (drv_cfp_entry_t *)entry;
    sal_memset(drv_entry->meter_data, 0, sizeof(drv_entry->meter_data));

    return SOC_E_NONE;
}
int
drv_tbx_fp_policer_control(int unit,  int stage_id, int op, void *entry, drv_policer_config_t *policer_cfg)
{

    int rv = SOC_E_NONE;

    switch (op) {
        case DRV_FIELD_POLICER_MODE_SUPPORT:
            rv = _drv_tbx_fp_policer_support_check(unit, stage_id, policer_cfg);
            break;
        case DRV_FIELD_POLICER_CONFIG:
            rv =  _drv_tbx_fp_policer_config(unit, stage_id, entry, policer_cfg);
            break;                
        case DRV_FIELD_POLICER_FREE:
            rv =  _drv_tbx_fp_policer_free(unit, stage_id, entry);
            break;                
    }
    return rv;
}

int
drv_tbx_fp_stat_type_get(int unit, int stage_id, drv_policer_mode_t policer_mode,
    drv_field_stat_t stat, int *type1, int *type2, int *type3)
{
    *type1 = -1;
    *type2 = -1;
    *type3 = -1;    

    if (SOC_IS_TB_AX(unit)) {
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
    } else {
        switch (stat) {
            case drvFieldStatCount:
            case drvFieldStatPackets:
                *type1 = DRV_CFP_STAT_GREEN;
                *type2 = DRV_CFP_STAT_RED;
                *type3 = DRV_CFP_STAT_YELLOW;
                break;
            case drvFieldStatGreenPackets:
                *type1 = DRV_CFP_STAT_GREEN;
                break;
            case drvFieldStatYellowPackets:
                if (policer_mode == drvPolicerModeCommitted) {
                    break;
                } else {
                    *type1 = DRV_CFP_STAT_YELLOW;
                }
                break;
            case drvFieldStatRedPackets:
                if (policer_mode == drvPolicerModeCommitted) {
                    *type1 = DRV_CFP_STAT_YELLOW;
                } else {
                    *type1 = DRV_CFP_STAT_RED;
                }
                break;
            case drvFieldStatNotGreenPackets:
                if (policer_mode == drvPolicerModeCommitted) {
                    *type1 = DRV_CFP_STAT_YELLOW;
                } else {
                    *type1 = DRV_CFP_STAT_RED;
                    *type2 = DRV_CFP_STAT_YELLOW;
                }
                break;
            case drvFieldStatNotYellowPackets:
                if (policer_mode == drvPolicerModeCommitted) {
                    break;
                } else {
                    *type1 = DRV_CFP_STAT_RED;
                    *type2 = DRV_CFP_STAT_GREEN;
                }
                break;
            case drvFieldStatNotRedPackets:
                if (policer_mode == drvPolicerModeCommitted) {
                    *type1 = DRV_CFP_STAT_GREEN;
                } else {
                    *type1 = DRV_CFP_STAT_GREEN;
                    *type2 = DRV_CFP_STAT_YELLOW;
                }
                break;
            default:
                return SOC_E_PARAM;
        }
    } 
    return SOC_E_NONE;
}


int
_drv_tbx_counter_mode_support(int unit, int param0, void *mode)
{
    int policer_valid;

    policer_valid = *((int *)mode);

    if (policer_valid) {
        if (SOC_IS_TB_AX(unit)) { 
             if (param0 != DRV_FIELD_COUNTER_MODE_RED_NOTRED){
                return SOC_E_UNAVAIL;
            }
        } else {
            switch (param0) {
                case DRV_FIELD_COUNTER_MODE_RED_NOTRED:
                case DRV_FIELD_COUNTER_MODE_GREEN_NOTGREEN:
                case DRV_FIELD_COUNTER_MODE_GREEN_RED:
                case DRV_FIELD_COUNTER_MODE_GREEN_YELLOW:
                case DRV_FIELD_COUNTER_MODE_RED_YELLOW:                
                case DRV_FIELD_COUNTER_MODE_GREEN:
                case DRV_FIELD_COUNTER_MODE_YELLOW:
                case DRV_FIELD_COUNTER_MODE_RED:
                    return SOC_E_NONE;
                default:
                    return SOC_E_UNAVAIL;
            }
        }
    } else {
        if (param0 != DRV_FIELD_COUNTER_MODE_NO_YES){
            return SOC_E_UNAVAIL;
        }
    }
    return SOC_E_NONE;
}

int
_drv_tbx_stat_mode_support(int unit, int stage_id, drv_policer_mode_t policer_mode, void *mode)    
{
    drv_field_stat_t f_stat;
    int type1, type2, type3;
    int rv;

    f_stat = *((drv_field_stat_t *)mode);
    type1 = type2 = type3 =-1;
    rv = drv_tbx_fp_stat_type_get(unit, stage_id, policer_mode, f_stat, &type1, &type2, &type3);
    return rv;
}


int
drv_tbx_fp_stat_support_check(int unit, int stage_id, int op, int param0, void *mode)
{
        
    switch (op) {
        case _DRV_FP_STAT_OP_COUNTER_MODE:
            return _drv_tbx_counter_mode_support(unit, param0, mode);
        case _DRV_FP_STAT_OP_STAT_MODE:
            return _drv_tbx_stat_mode_support(unit, stage_id, param0, mode);
    }
    return SOC_E_NONE;
}


