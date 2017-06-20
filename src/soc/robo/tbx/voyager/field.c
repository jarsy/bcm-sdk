/*
 * $Id: field.c,v 1.7 Broadcom SDK $
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
#include "fp_vo.h"
#include <soc/robo/voyager_service.h>

int
_drv_vo_cfp_qualify_conflict_check(int unit, drv_field_qualify_t qual, 
            drv_cfp_entry_t *drv_entry, uint32* p_data, 
            uint32* p_mask);
int
_drv_vo_ingress_qualify_support(int unit, drv_field_qualify_t qual);

#define VO_CFP_FRAME_L4_TCP               _DRV_CFP_FRAME_L4_TYPE0
#define VO_CFP_FRAME_L4_UDP               _DRV_CFP_FRAME_L4_TYPE1
#define VO_CFP_FRAME_L4_TCPUDP        (_DRV_CFP_FRAME_L4_TYPE0 |_DRV_CFP_FRAME_L4_TYPE1)
#define VO_CFP_FRAME_L4_ICMPIGMP      _DRV_CFP_FRAME_L4_TYPE2
#define VO_CFP_FRAME_L4_OTHER             _DRV_CFP_FRAME_L4_TYPE3


#define VO_CFP_FRAME_L2_ANY            0
#define VO_CFP_FRAME_L2_DIXv2       _DRV_CFP_FRAME_L2_TYPE0
#define VO_CFP_FRAME_L2_SNAP_PUBLIC     _DRV_CFP_FRAME_L2_TYPE1
#define VO_CFP_FRAME_L2_LLC _DRV_CFP_FRAME_L2_TYPE2
#define VO_CFP_FRAME_L2_SNAP_PRIVATE    _DRV_CFP_FRAME_L2_TYPE3


typedef struct _vo_qual_info_s {
    int qual_id;
    uint32 field_id;
    int slice_id;
}_vo_qual_info_t;

static _vo_qual_info_t _vo_qaul_mapping_tbl[] = {
{drvFieldQualifyL2Format,INDEX(L2_FRAMINGf), -1},
{drvFieldQualifyClassId, INDEX(CHAIN_IDf), CFP_53600_SLICE_ID_WITH_CHAIN},
{drvFieldQualifyDSCP, INDEX(IP_TOSf), -1},
{drvFieldQualifyEtherType, INDEX(IP_TOSf), -1},
{drvFieldQualifyInPort, INDEX(SRC_PBMPf), -1},
{drvFieldQualifyInPorts, INDEX(SRC_PBMPf), -1},
{drvFieldQualifyInnerVlanCfi,INDEX(USR_CFIf), -1},
{drvFieldQualifyInnerVlanId, INDEX(USR_VIDf), -1},
{drvFieldQualifyInnerVlanPri, INDEX(USR_PRIf), -1},
{drvFieldQualifyIpAuth, INDEX(IP_AUTHf), -1},
{drvFieldQualifyIpProtocol,INDEX(IP_PROTOf), -1},
{drvFieldQualifyIpProtocolCommon,INDEX(IP_PROTOf), -1},
{drvFieldQualifyOuterVlanCfi,INDEX(SP_CFIf), -1},
{drvFieldQualifyOuterVlanId, INDEX(SP_VIDf), -1},
{drvFieldQualifyOuterVlanPri, INDEX(SP_PRIf), -1},
{drvFieldQualifyTtl, INDEX(TTL_RANGEf), -1},
{drvFieldQualifyL4SrcPort, INDEX(L4_FIRST_TWO_BYTESf), -1},
{drvFieldQualifyIcmpTypeCode, INDEX(L4_FIRST_TWO_BYTESf), -1},
{drvFieldQualifyIgmpTypeMaxRespTime, INDEX(L4_FIRST_TWO_BYTESf), -1},
{drvFieldQualifyL4DstPort, INDEX(L4_SECOND_TWO_BYTESf), -1},
{drvFieldQualifyRangeCheck, INDEX(RANGE_CHECKf), -1}
};

typedef struct _vo_udf_reg_info_s {
    int udf_id;
    int udf_field;
    uint32 reg_id;
    uint32 reg_index;
    int slice_id;
    int udf_len;
}_vo_udf_reg_info_t;

_vo_udf_reg_info_t   _vo_udf_mapping_tbl[] = {
{0, INDEX(UDF_A0_LOf), INDEX(CFP_UDF_0_Ar), 0, 0, 4},
{1, INDEX(UDF_A0_HIf), INDEX(CFP_UDF_0_Ar), 0, 0, 2},
{2, INDEX(UDF_A1_LOf), INDEX(CFP_UDF_0_Ar), 1, 0, 4},
{3, INDEX(UDF_A1_HIf), INDEX(CFP_UDF_0_Ar), 1, 0, 2},
{4, INDEX(UDF_A2f), INDEX(CFP_UDF_0_Ar), 2, 0, 4},
{5, INDEX(UDF_A3f), INDEX(CFP_UDF_0_Ar), 3, 0, 4},
{6, INDEX(UDF_A4f), INDEX(CFP_UDF_0_Ar), 4, 0, 4},
{7, INDEX(UDF_A5f), INDEX(CFP_UDF_0_Ar), 5, 0, 4},
{8, INDEX(UDF_A6f), INDEX(CFP_UDF_0_Ar), 6, 0, 2},
{9, INDEX(UDF_A7f), INDEX(CFP_UDF_0_Ar), 7, 0, 2},
{10, INDEX(UDF_A8f), INDEX(CFP_UDF_0_Ar), 8, 0, 2},
{11, INDEX(UDF_A9f), INDEX(CFP_UDF_0_Ar), 9, 0, 2},
{12, INDEX(UDF_A10f), INDEX(CFP_UDF_0_Ar), 10, 0, 2},
{13, INDEX(UDF_A11f), INDEX(CFP_UDF_0_Ar), 11, 0, 1},
{14, INDEX(UDF_A0_LOf), INDEX(CFP_UDF_1_Ar), 0, 1, 4},
{15, INDEX(UDF_A0_HIf), INDEX(CFP_UDF_1_Ar), 0, 1, 2},
{16, INDEX(UDF_A1_LOf), INDEX(CFP_UDF_1_Ar), 1, 1, 4},
{17, INDEX(UDF_A1_HIf), INDEX(CFP_UDF_1_Ar), 1, 1, 2},
{18, INDEX(UDF_A2f), INDEX(CFP_UDF_1_Ar), 2, 1, 4},
{19, INDEX(UDF_A3f), INDEX(CFP_UDF_1_Ar), 3, 1, 4},
{20, INDEX(UDF_A4f), INDEX(CFP_UDF_1_Ar), 4, 1, 4},
{21, INDEX(UDF_A5f), INDEX(CFP_UDF_1_Ar), 5, 1, 4},
{22, INDEX(UDF_A6f), INDEX(CFP_UDF_1_Ar), 6, 1, 2},
{23, INDEX(UDF_A7f), INDEX(CFP_UDF_1_Ar), 7, 1, 2},
{24, INDEX(UDF_A8f), INDEX(CFP_UDF_1_Ar), 8, 1, 2},
{25, INDEX(UDF_A9f), INDEX(CFP_UDF_1_Ar), 9, 1, 2},
{26, INDEX(UDF_A10f), INDEX(CFP_UDF_1_Ar), 10, 1, 2},
{27, INDEX(UDF_A11f), INDEX(CFP_UDF_1_Ar), 11, 1, 1},
{28, INDEX(UDF_A0_LOf), INDEX(CFP_UDF_2_Ar), 0, 2, 4},
{29, INDEX(UDF_A0_HIf), INDEX(CFP_UDF_2_Ar), 0, 2, 2},
{30, INDEX(UDF_A1_LOf), INDEX(CFP_UDF_2_Ar), 1, 2, 4},
{31, INDEX(UDF_A1_HIf), INDEX(CFP_UDF_2_Ar), 1, 2, 2},
{32, INDEX(UDF_A2f), INDEX(CFP_UDF_2_Ar), 2, 2, 4},
{33, INDEX(UDF_A3f), INDEX(CFP_UDF_2_Ar), 3, 2, 4},
{34, INDEX(UDF_A4f), INDEX(CFP_UDF_2_Ar), 4, 2, 4},
{35, INDEX(UDF_A5f), INDEX(CFP_UDF_2_Ar), 5, 2, 4},
{36, INDEX(UDF_A6f), INDEX(CFP_UDF_2_Ar), 6, 2, 2},
{37, INDEX(UDF_A7f), INDEX(CFP_UDF_2_Ar), 7, 2, 2},
{38, INDEX(UDF_A8f), INDEX(CFP_UDF_2_Ar), 8, 2, 2},
{39, INDEX(UDF_A9f), INDEX(CFP_UDF_2_Ar), 9, 2, 2},
{40, INDEX(UDF_A10f), INDEX(CFP_UDF_2_Ar), 10, 2, 2},
{41, INDEX(UDF_A11f), INDEX(CFP_UDF_2_Ar),11, 2, 2},
{42, INDEX(UDF_D0_LOf), INDEX(CFP_UDF_3_Dr), 0, 4, 4},
{43, INDEX(UDF_D0_HIf), INDEX(CFP_UDF_3_Dr), 0, 4, 4},
{44, INDEX(UDF_D1_LOf), INDEX(CFP_UDF_3_Dr), 1, 4, 4},
{45, INDEX(UDF_D1_HIf), INDEX(CFP_UDF_3_Dr), 1, 4, 4},
{46, INDEX(UDF_D2f), INDEX(CFP_UDF_3_Dr), 2, 4, 4},
{47, INDEX(UDF_D3f), INDEX(CFP_UDF_3_Dr), 3, 4, 4},
{48, INDEX(UDF_D4f), INDEX(CFP_UDF_3_Dr), 4, 4, 4},
{49, INDEX(UDF_D5f), INDEX(CFP_UDF_3_Dr), 5, 4, 4},
{50, INDEX(UDF_D6f), INDEX(CFP_UDF_3_Dr), 6, 4, 4},
{51, INDEX(UDF_D7f), INDEX(CFP_UDF_3_Dr), 7, 4, 4},
{52, INDEX(UDF_D8f), INDEX(CFP_UDF_3_Dr), 8, 4, 4},
{53, INDEX(UDF_D9f), INDEX(CFP_UDF_3_Dr), 9, 4, 4},
{54, INDEX(UDF_D10f), INDEX(CFP_UDF_3_Dr), 10, 4, 2},
{55, INDEX(UDF_D11f), INDEX(CFP_UDF_3_Dr), 11, 4, 2}
};

#define VO_CFP_UDF_GET(unit, index, val) \
    DRV_REG_READ(unit, DRV_REG_ADDR(unit, \
        _vo_udf_mapping_tbl[index].reg_id, 0, \
        _vo_udf_mapping_tbl[index].reg_index),\
        val, DRV_REG_LENGTH_GET(unit, _vo_udf_mapping_tbl[index].reg_id))

#define VO_CFP_UDF_SET(unit, index, val) \
    DRV_REG_WRITE(unit, DRV_REG_ADDR(unit, \
        _vo_udf_mapping_tbl[index].reg_id, 0, \
        _vo_udf_mapping_tbl[index].reg_index),\
        val, DRV_REG_LENGTH_GET(unit, _vo_udf_mapping_tbl[index].reg_id))


typedef struct _vo_qual_slice_info_s {
    int qual_id;
    int slice_bmp;
    int conflict_qid[2];
}_vo_qual_slice_info_t;


_vo_qual_slice_info_t   _vo_qual_slice_mapping_tbl[] = {
{drvFieldQualifyInPort, 0x1f, {-1, -1}},
{drvFieldQualifyInPorts, 0x1f, {-1, -1}},
{drvFieldQualifyVlanFormat, 0x1f, {-1, -1}},
{drvFieldQualifyIpType, 0x1f, {-1, -1}},
{drvFieldQualifyL2Format, 0x1f, {-1, -1}},
{drvFieldQualifyIp4, 0x1f, {-1, -1}},
{drvFieldQualifyIp6, 0x1f, {-1, -1}},  
{drvFieldQualifyIpProtocolCommon, 0x1f, {-1, -1}},
{drvFieldQualifyPacketFormat, 0x1f, {-1, -1}},
{drvFieldQualifyIpProtocol, 0xf, {drvFieldQualifyEtherType, drvFieldQualifyLlc}},
{drvFieldQualifyDSCP, 0xf, {drvFieldQualifyEtherType, drvFieldQualifyLlc}},
{drvFieldQualifyLlc, 0xf, {drvFieldQualifyDSCP, drvFieldQualifyIpProtocol}}, 
{drvFieldQualifyEtherType, 0xf, {drvFieldQualifyDSCP, drvFieldQualifyIpProtocol}},
{drvFieldQualifyInnerVlan, 0xf, {-1, -1}},
{drvFieldQualifyInnerVlanCfi, 0xf, {-1, -1}},
{drvFieldQualifyInnerVlanId, 0xf, {-1, -1}},
{drvFieldQualifyInnerVlanPri, 0xf, {-1, -1}},
{drvFieldQualifyIpAuth, 0xf, {-1, -1}},
{drvFieldQualifyIpFlags, 0xf, {-1, -1}},
{drvFieldQualifyIpFrag, 0xf, {-1, -1}},     
{drvFieldQualifyIpInfo, 0xf, {-1, -1}},
{drvFieldQualifyOuterVlan, 0xf, {-1, -1}},
{drvFieldQualifyOuterVlanCfi, 0xf, {-1, -1}},
{drvFieldQualifyOuterVlanId, 0xf, {-1, -1}},
{drvFieldQualifyOuterVlanPri, 0xf, {-1, -1}},
{drvFieldQualifyTtl, 0xf, {-1, -1}},
{drvFieldQualifyL4Ports, 0x1f, {-1, -1}},
{drvFieldQualifyL4SrcPort, 0xf, {-1, -1}},
{drvFieldQualifyL4DstPort, 0xf, {-1, -1}},
{drvFieldQualifyIcmpTypeCode, 0xf, {-1, -1}},
{drvFieldQualifyIgmpTypeMaxRespTime, 0xf, {-1, -1}},
{drvFieldQualifyRangeCheck, 0xf, {-1, -1}},
{drvFieldQualifyClassId, 0x13, {-1, -1}}
};

static drv_cfp_field_map_t  vo_cfp_field_map_table[] = {
    {DRV_CFP_FIELD_VALID, INDEX(VALIDf), -1},
    {DRV_CFP_FIELD_SPTAGGED, INDEX(S_TAGGEDf), -1},
    {DRV_CFP_FIELD_1QTAGGED, INDEX(C_TAGGEDf), -1},
    {DRV_CFP_FIELD_L2_FRM_FORMAT, INDEX(L2_FRAMINGf), -1},
    {DRV_CFP_FIELD_L3_FRM_FORMAT, INDEX(L3_FRAMINGf), -1},
    {DRV_CFP_FIELD_IP_FRAG, INDEX(IP_FRAGf), -1},
    {DRV_CFP_FIELD_IP_NON_FIRST_FRAG, INDEX(NON_FIRST_FRAGf), -1},
};


#define _cfp_field_data_set   0x1
#define _cfp_field_mask_set   0x2
#define _cfp_field_data_get  0x4
#define _cfp_field_mask_get  0x8
#define _cfp_field_data_mask_set   (_cfp_field_data_set | _cfp_field_mask_set)
#define _cfp_field_data_mask_get  (_cfp_field_data_get | _cfp_field_mask_get)

int
_drv_vo_cfp_field_control(int unit, drv_cfp_entry_t *drv_entry, int mem, int field,
    uint32 *p_data, uint32 *p_mask, int flag) {

    uint32 *tcam_data, *tcam_mask;    
    int rv = SOC_E_CONFIG;

    if (drv_entry->flags & _DRV_CFP_SLICE_CHAIN){
        if (drv_entry->flags & _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN){
            tcam_data = drv_entry->tcam_data;
            tcam_mask =drv_entry->tcam_mask;
        } else {
            tcam_data = drv_entry->cfp_chain->tcam_data;
            tcam_mask = drv_entry->cfp_chain->tcam_mask;
        }
    } else {
        tcam_data = drv_entry->tcam_data;
        tcam_mask = drv_entry->tcam_mask;
    }
    if (flag &_cfp_field_data_set) {
        rv = DRV_MEM_FIELD_SET(unit, mem, field, tcam_data, p_data);
        SOC_IF_ERROR_RETURN(rv);
    }
    if (flag & _cfp_field_mask_set) {
        rv = DRV_MEM_FIELD_SET(unit, mem, field, tcam_mask, p_mask);
        SOC_IF_ERROR_RETURN(rv);

    } 
    
    if (flag & _cfp_field_data_get) {
        rv = DRV_MEM_FIELD_GET(unit, mem, field, tcam_data, p_data);
        SOC_IF_ERROR_RETURN(rv);
    }
    if (flag &  _cfp_field_mask_get) {
        rv = DRV_MEM_FIELD_GET(unit, mem, field, tcam_mask, p_mask);
        SOC_IF_ERROR_RETURN(rv);
    } 

    return rv;
}


int 
_drv_vo_cfp_qual_value_set(int unit, drv_field_qualify_t qual, drv_cfp_entry_t *drv_entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int rv = SOC_E_UNAVAIL;
    int i;
    uint32 field, mem;
    uint32 temp_data, temp_mask;
    
    rv =_drv_vo_ingress_qualify_support(unit, qual);
    SOC_IF_ERROR_RETURN(rv);
    
    rv = _drv_vo_cfp_qualify_conflict_check(unit, qual, drv_entry, p_data, p_mask);
    SOC_IF_ERROR_RETURN(rv);

    if ((qual == drvFieldQualifyIpProtocolCommon) &&  
        (drv_entry->slice_id == CFP_53600_SLICE_ID_CHAIN_SLICE))  {
        if ((*p_data == 6)  || (*p_data == 17) || (*p_data == 2) || 
            (*p_data == 1) ||(*p_data == 58)) {
            /* program the l4 framing */
            return SOC_E_NONE;
        } else {
            return SOC_E_UNAVAIL;
        }        
    }     

    field = -1;
    mem = 0;
    for (i=0; i < COUNTOF(_vo_qaul_mapping_tbl); i++) {
        if (_vo_qaul_mapping_tbl[i].qual_id == qual) {
            field = _vo_qaul_mapping_tbl[i].field_id;
            if (_vo_qaul_mapping_tbl[i].slice_id == 
                    CFP_53600_SLICE_ID_WITH_CHAIN){
                mem= INDEX(CFP_TCAM_CHAIN_SCm);
            } else {
                mem= INDEX(CFP_TCAM_SCm);
            }
            break;
        }        
    }
  
    if ((-1 == field) ||(0 == mem)){
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "indirect qual field setting.\n")));
        return SOC_E_NONE;
    }    

    if (qual == drvFieldQualifyEtherType){
        temp_data = *p_data & 0xff;
        temp_mask = *p_mask & 0xff;
        rv = _drv_vo_cfp_field_control(unit, drv_entry, mem,  INDEX(IP_PROTOf), &temp_data, &temp_mask, 
            _cfp_field_data_mask_set);
        *p_data = (*p_data & 0xff00) >> 8;
        *p_mask = (*p_mask & 0xff00) >> 8;
    }
    rv = _drv_vo_cfp_field_control(unit, drv_entry, mem, field, p_data, p_mask, 
            _cfp_field_data_mask_set);


    return SOC_E_NONE;
}

int 
_drv_vo_cfp_qual_value_get(int unit, drv_field_qualify_t qual, drv_cfp_entry_t *drv_entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int rv = SOC_E_UNAVAIL;
    int i;
    int field, mem;
    uint32 l4_frame, l4_frame_mask;
    
    mem = -1;
    field = -1;
    for (i=0; i < COUNTOF(_vo_qaul_mapping_tbl); i++) {
        if (_vo_qaul_mapping_tbl[i].qual_id == qual) {
            field = _vo_qaul_mapping_tbl[i].field_id;
            if (_vo_qaul_mapping_tbl[i].slice_id == 
                    CFP_53600_SLICE_ID_WITH_CHAIN){
                mem= INDEX(CFP_TCAM_CHAIN_SCm);
            } else {
                mem= INDEX(CFP_TCAM_SCm);
            }
            break;
        } 
    }
    if ((mem == -1) || (field == -1)) {
        return SOC_E_INTERNAL;
    }
    if (qual == drvFieldQualifyL4Ports) {
        rv = _drv_vo_cfp_field_control(unit, drv_entry, INDEX(CFP_TCAM_SCm), 
            INDEX(L4_FRAMINGf), &l4_frame, &l4_frame_mask, 
            _cfp_field_data_mask_get);
        SOC_IF_ERROR_RETURN(rv);
        if (l4_frame_mask) {
            if (l4_frame & l4_frame_mask){
                *p_data = 1;
            } else {
                *p_data = 0;
            }
            *p_mask = 1;        
        } else {
            *p_data = 0;        
            *p_mask = 0;        
        }
        return SOC_E_NONE;
    }

    if ((qual == drvFieldQualifyIpProtocolCommon) &&  
        (drv_entry->slice_id == CFP_53600_SLICE_ID_CHAIN_SLICE))  {
        rv = _drv_vo_cfp_field_control(unit, drv_entry, INDEX(CFP_TCAM_SCm), 
            INDEX(L4_FRAMINGf), &l4_frame, &l4_frame_mask, 
            _cfp_field_data_mask_get);
        SOC_IF_ERROR_RETURN(rv);
        if (l4_frame_mask) {
            if ((l4_frame & l4_frame_mask) == 0){
                *p_data = 6;
            } else if ((l4_frame & l4_frame_mask) == 1){ 
                *p_data = 17;
            } else if ((l4_frame & l4_frame_mask) == 2){ 
                *p_data = 1;
            } else if ((l4_frame & l4_frame_mask) == 3){ 
                return SOC_E_UNAVAIL;
            }
        } else {
            return SOC_E_UNAVAIL;
        }
        return SOC_E_NONE;
    }


    rv = _drv_vo_cfp_field_control(unit, drv_entry, mem, field, p_data, p_mask, 
            _cfp_field_data_mask_get);
    return rv;
}


int
_drv_vo_cfp_udf_value_set(int unit, uint32 udf_idx, drv_cfp_entry_t *drv_entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int rv = SOC_E_UNAVAIL;
    int i;
    uint32 field, mem;
    uint32 valid_index, valid_field, valid_data, valid_mask;

    field = -1;
    valid_index = 0;
    for (i=0; i < COUNTOF(_vo_udf_mapping_tbl); i++) {
        if (_vo_udf_mapping_tbl[i].udf_id== udf_idx) {
            if (drv_entry->slice_id == CFP_53600_SLICE_ID_CHAIN_SLICE) {
                if (drv_entry->flags & _DRV_CFP_SLICE_CHAIN){
                    if ((_vo_udf_mapping_tbl[i].slice_id & 0x3) 
                        != CFP_53600_SLICE_ID_BASE) {
                        LOG_WARN(BSL_LS_SOC_COMMON,
                                 (BSL_META_U(unit,
                                             "udf idex %d is not supported for current slice id %d\n"),
                                  udf_idx, drv_entry->slice_id));
                        return SOC_E_UNAVAIL;
                    }  
                } else {
                    if (_vo_udf_mapping_tbl[i].slice_id != drv_entry->slice_id) {
                        LOG_WARN(BSL_LS_SOC_COMMON,
                                 (BSL_META_U(unit,
                                             "udf idex %d is not supported for current slice id %d\n"),
                                  udf_idx, drv_entry->slice_id));
                        return SOC_E_UNAVAIL;
                    }
                }

            } else if (drv_entry->slice_id == CFP_53600_SLICE_ID_WITH_CHAIN){
                if ((_vo_udf_mapping_tbl[i].slice_id & 0x3) 
                    != CFP_53600_SLICE_ID_BASE) {
                        LOG_WARN(BSL_LS_SOC_COMMON,
                                 (BSL_META_U(unit,
                                             "udf idex %d is not supported for current slice id %d\n"),
                                  udf_idx, drv_entry->slice_id));
                    return SOC_E_UNAVAIL;
                }            
            } else {
                if (_vo_udf_mapping_tbl[i].slice_id != drv_entry->slice_id) {
                    if (drv_entry->slice_bmp & (1<<_vo_udf_mapping_tbl[i].slice_id)) {
                        drv_entry->slice_id = _vo_udf_mapping_tbl[i].slice_id;
                        drv_entry->slice_bmp = (1<<_vo_udf_mapping_tbl[i].slice_id);
                        if (drv_entry->slice_id == CFP_53600_SLICE_ID_CHAIN_SLICE) {
                            valid_data = 3;
                        } else {
                            valid_data = drv_entry->slice_id;
                        }
                        soc_CFP_TCAM_SCm_field_set(unit, drv_entry->tcam_data, 
                            SLICEIDf, &valid_data);

                    } else {
                        LOG_WARN(BSL_LS_SOC_COMMON,
                                 (BSL_META_U(unit,
                                             "udf idex %d is not supported for current slice id %d\n"),
                                  udf_idx, drv_entry->slice_id));
                        return SOC_E_UNAVAIL;
                    }
                }
            }
            field = _vo_udf_mapping_tbl[i].udf_field;
            valid_index = _vo_udf_mapping_tbl[i].reg_index;
            break;
        }    
    }
    if (-1 == field) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "no matched udf idex\n")));
        return SOC_E_INTERNAL;
    }

    if (drv_entry->flags & _DRV_CFP_SLICE_CHAIN){
        if (udf_idx >= CFP_53600_SLICE_CHAIN_UDF_BASE) {
            drv_entry->flags |= _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN;
        }
    }
    if (udf_idx >= CFP_53600_SLICE_CHAIN_UDF_BASE) {
        mem= INDEX(CFP_TCAM_CHAIN_SCm);
        valid_field = INDEX(UDFD_VLDf);
    } else {
        mem= INDEX(CFP_TCAM_SCm);
        valid_field = INDEX(UDFA_VLDf);
    }

    rv = _drv_vo_cfp_field_control(unit, drv_entry, mem, field, 
        p_data, p_mask, _cfp_field_data_mask_set);
    SOC_IF_ERROR_RETURN(rv);
    
    rv = _drv_vo_cfp_field_control(unit, drv_entry, mem, valid_field, 
        &valid_data, &valid_mask, _cfp_field_data_mask_get);    
   SOC_IF_ERROR_RETURN(rv);

    valid_data |= (1 << valid_index);
    valid_mask |= (1 << valid_index);

    rv = _drv_vo_cfp_field_control(unit, drv_entry, mem, valid_field, 
        &valid_data, &valid_mask, _cfp_field_data_mask_set);    

    drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN);
    return rv;
}

int
_drv_vo_cfp_udf_value_get(int unit, uint32 udf_idx, drv_cfp_entry_t *drv_entry,
                      uint32 *p_data, uint32 *p_mask)
{
    int rv = SOC_E_UNAVAIL;
    int i;
    int field, mem;


    if (drv_entry->flags & _DRV_CFP_SLICE_CHAIN){
        if (udf_idx >= CFP_53600_SLICE_CHAIN_UDF_BASE) {
            drv_entry->flags |= _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN;
        }
    }
    field = -1;
    mem = -1;
    if (udf_idx >= CFP_53600_SLICE_CHAIN_UDF_BASE) {
        mem= INDEX(CFP_TCAM_CHAIN_SCm);        
    } else {
        mem= INDEX(CFP_TCAM_SCm);
    }

    for (i=0; i < COUNTOF(_vo_udf_mapping_tbl); i++) {
        if (_vo_udf_mapping_tbl[i].udf_id== udf_idx) {
            field = _vo_udf_mapping_tbl[i].udf_field;
            break;
        }
    }

    if ((field == -1) || (mem == -1)){
        return SOC_E_INTERNAL;
    }

    rv = _drv_vo_cfp_field_control(unit, drv_entry, mem, field, p_data, p_mask, 
            _cfp_field_data_mask_get);
    drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN);
    return rv;
}

int
_drv_vo_cfp_qualify_conflict_check(int unit, drv_field_qualify_t qual, 
            drv_cfp_entry_t *drv_entry, uint32* p_data, 
            uint32* p_mask)

{
    uint32 flags, l2_flags, l3_flags, l4_flags;
    int conflict;
    uint32 data, mask;
    int rv = SOC_E_NONE;

    conflict = 0;

    if (drv_entry == NULL) {
        return SOC_E_PARAM;
    }
    flags = drv_entry->flags;   
    
    l2_flags = flags & _DRV_CFP_FRAME_L2_ALL;
    l3_flags = flags & _DRV_CFP_FRAME_ALL;
    l4_flags = flags & _DRV_CFP_FRAME_L4_ALL;
    data = *p_data;
    mask = *p_mask;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "(1)flags l2 %x l3 %x l4 %x\n"),
                 l2_flags, l3_flags, l4_flags));
    switch (qual) {
        /* Non-IP */            
        case drvFieldQualifyEtherType:
        {
            if ( _DRV_CFP_FRAME_NONIP !=l3_flags ) {
                conflict = 1;
            }
            break;
        }
        case drvFieldQualifyL2Format:
        {
            if ((data & mask) == VO_CFP_FRAME_L2_ANY) {
                if (0 == l2_flags) {
                    l2_flags = VO_CFP_FRAME_L2_ANY;   
                }
            }
            if ((data & mask) == VO_CFP_FRAME_L2_DIXv2) {
                if (0 == l2_flags) {
                    l2_flags = VO_CFP_FRAME_L2_DIXv2;
                }
                if (VO_CFP_FRAME_L2_DIXv2 != l2_flags) {
                    conflict = 1;
                }
            }
            if ((data & mask) == VO_CFP_FRAME_L2_SNAP_PUBLIC) {
                if (0 == l2_flags) {
                    l2_flags = VO_CFP_FRAME_L2_SNAP_PUBLIC;
                }
                if (VO_CFP_FRAME_L2_SNAP_PUBLIC!= l2_flags) {
                    conflict = 1;
                }
            }
            if ((data & mask) == VO_CFP_FRAME_L2_LLC) {
                if (0 == l2_flags) {
                    l2_flags = VO_CFP_FRAME_L2_LLC;
                }
                if (VO_CFP_FRAME_L2_LLC != l2_flags) {
                    conflict = 1;
                }
            }
            if ((data & mask) == VO_CFP_FRAME_L2_SNAP_PRIVATE) {
                if (0 == l2_flags) {
                    l2_flags = VO_CFP_FRAME_L2_SNAP_PRIVATE;
                }
                if (VO_CFP_FRAME_L2_SNAP_PRIVATE != l2_flags) {
                    conflict = 1;
                }
            }
            break;
        }
        case drvFieldQualifyLlc:    
        /* non-IP, L2-LLC */
        {
            if (0 == l2_flags) {
                l2_flags = VO_CFP_FRAME_L2_LLC;
            }
            if (VO_CFP_FRAME_L2_LLC != l2_flags) {
                    conflict = 1;
            }
            break;
        }
        case drvFieldQualifyIp4:
        /* IPv4*/
        {
            if (0 == l3_flags) {
                l3_flags = _DRV_CFP_FRAME_IP4;
            }
            if ( 0 ==  (_DRV_CFP_FRAME_IP4 & l3_flags)) {
                conflict = 1;
            } else {
                l3_flags = _DRV_CFP_FRAME_IP4;
            }
            break;
        }
        case drvFieldQualifyIp6:              
        /* IPv6*/
        {
            if (0 == l3_flags) {
                l3_flags = _DRV_CFP_FRAME_IP6;
            }
            if ( 0 ==  (_DRV_CFP_FRAME_IP6 & l3_flags)) {
                conflict = 1;
            } else {
                l3_flags = _DRV_CFP_FRAME_IP6;
            }
            break;

        }
        case drvFieldQualifyIpAuth:
        case drvFieldQualifyIpFlags:          
        case drvFieldQualifyIpFrag:          
        case drvFieldQualifyIpInfo:
        /* IP type */
        {
            if ( _DRV_CFP_FRAME_NONIP == l3_flags ) {
                conflict = 1;
            }
            break;
        }

        case drvFieldQualifyIpProtocolCommon:
        {
            if ( _DRV_CFP_FRAME_NONIP == l3_flags ) {
                conflict = 1;
            }

            if (data == 6) {
            /* bcmFieldIpProtocolCommonTcp*/ 
                if (0 == l4_flags) {                    
                    l4_flags  = VO_CFP_FRAME_L4_TCP;
                }
                if (l4_flags & VO_CFP_FRAME_L4_TCP) {
                    l4_flags  = VO_CFP_FRAME_L4_TCP;
                } else {
                    conflict = 1;
                }
            }
            if (data == 17) {
            /* bcmFieldIpProtocolCommonUdp */
                if (0 == l4_flags) {                    
                    l4_flags  = VO_CFP_FRAME_L4_UDP;
                }
                if (l4_flags & VO_CFP_FRAME_L4_UDP) {
                    l4_flags  = VO_CFP_FRAME_L4_UDP;
                } else {
                    conflict = 1;
                }
            }

            if ((data == 2) || (data == 1) ||(data == 58)){
                /* bcmFieldIpProtocolCommonIgmp / 
                    bcmFieldIpProtocolCommonIcmp /
                    bcmFieldIpProtocolCommonIp6Icmp*/
                if (0 == l4_flags) {                    
                    l4_flags  = VO_CFP_FRAME_L4_ICMPIGMP;
                }
                if (l4_flags & VO_CFP_FRAME_L4_ICMPIGMP) {
                    l4_flags  = VO_CFP_FRAME_L4_ICMPIGMP;
                } else {
                    conflict = 1;
                }
            }
            break;
        }         
        case drvFieldQualifyL4SrcPort:
        case drvFieldQualifyL4DstPort:
            /* L4 TCP/UDP*/
        {
            if ( _DRV_CFP_FRAME_NONIP == l3_flags ) {
                conflict = 1;
            } else {
                l3_flags = _DRV_CFP_FRAME_IP;
            }
            if (0 == l4_flags) {
                l4_flags = VO_CFP_FRAME_L4_TCPUDP |VO_CFP_FRAME_L4_ICMPIGMP|
                    VO_CFP_FRAME_L4_OTHER;
            } 
            break;
        }

        case drvFieldQualifyIcmpTypeCode:
        case drvFieldQualifyIgmpTypeMaxRespTime:            
            /* L4 ICMP/IGMP */
        {
            if ( _DRV_CFP_FRAME_NONIP == l3_flags ) {
                conflict = 1;
            }
            if (0 == l4_flags) {
                l4_flags = VO_CFP_FRAME_L4_ICMPIGMP;
            }
            if ( 0 == (VO_CFP_FRAME_L4_ICMPIGMP & l4_flags)){
                conflict = 1;
            }            
            break;
        }
        case drvFieldQualifyL4Ports:
            /* L4 TCP/UDP/ICMP/IGMP or L4 others */
        {
            if ( _DRV_CFP_FRAME_NONIP == l3_flags ) {
                conflict = 1;
            } else {
                l3_flags = _DRV_CFP_FRAME_IP;
            }
            if (data & mask) {
                /* valid */
                if ( 0 == l4_flags) {
                    l4_flags = VO_CFP_FRAME_L4_TCPUDP 
                        | VO_CFP_FRAME_L4_ICMPIGMP;
                }
                if ( VO_CFP_FRAME_L4_OTHER == l4_flags) {
                    conflict = 1;
                } else {
                    l4_flags &= ~VO_CFP_FRAME_L4_OTHER;
                }                    

            }else {
                if (mask) {
                    if ( 0 == l4_flags) {
                        l4_flags = VO_CFP_FRAME_L4_OTHER;
                    }
                    if (0 == (VO_CFP_FRAME_L4_OTHER & l4_flags)){
                        conflict = 1;
                    } else {
                        l4_flags = VO_CFP_FRAME_L4_OTHER;
                    }
                } else {
                    l4_flags = 0;
                }
            }
            break;
        }
        default:
            break;
    }

    if (1 == conflict) {
        rv = SOC_E_UNAVAIL;
    } else {            
        flags =( flags & ~(_DRV_CFP_FRAME_L2_ALL)) | l2_flags;
        flags = (flags & ~(_DRV_CFP_FRAME_ALL)) | l3_flags;
        flags = (flags & ~(_DRV_CFP_FRAME_L4_ALL)) | l4_flags;
        drv_entry->flags = flags;   
        rv = SOC_E_NONE;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s: conflict %d qual %d, l2_flags %x l3_flags %x l4_flags %x\n"), 
                 FUNCTION_NAME(), conflict, qual, l2_flags, l3_flags, l4_flags));
    return rv;
}

int
_drv_vo_ingress_qualify_support(int unit, drv_field_qualify_t qual)    
{
    int rv = SOC_E_UNAVAIL;

    switch(qual) {
        case drvFieldQualifyClassId:
        case drvFieldQualifyDSCP:          /*drvFieldQualifyTos, drvFieldQualifyIp6TrafficClass*/
        case drvFieldQualifyEtherType:
        case drvFieldQualifyInPort:     
        case drvFieldQualifyInPorts:     
        case drvFieldQualifyInnerVlan:        
        case drvFieldQualifyInnerVlanCfi:
        case drvFieldQualifyInnerVlanId:     
        case drvFieldQualifyInnerVlanPri:     
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
        case drvFieldQualifyStage:          
        case drvFieldQualifyStageIngress:
        case drvFieldQualifyTtl: /* drvFieldQualifyIp6HopLimit */
        case drvFieldQualifyVlanFormat:
        case drvFieldQualifyL4Ports:
        case drvFieldQualifyL4SrcPort:
        case drvFieldQualifyL4DstPort:
        case drvFieldQualifyIcmpTypeCode:
        case drvFieldQualifyIgmpTypeMaxRespTime:            
        case drvFieldQualifyRangeCheck:
            rv =  SOC_E_NONE;
            break;
        default:
            rv =  SOC_E_UNAVAIL;
            break;
    }
    return rv;
}

int
_drv_vo_fp_cfp_qualify_support(int unit, drv_field_qset_t qset)
{
    int rv = SOC_E_NONE;
    int i;

    /* Qualifier checking */
    for (i = 0; i < drvFieldQualifyCount; i++) {
        if (DRV_FIELD_QSET_TEST(qset, i)) {
            rv = _drv_vo_ingress_qualify_support(unit, i);
            if (SOC_FAILURE(rv)){
                return rv;
            }
        }            
    }
    return rv;
}

int
_drv_vo_qset_to_cfp(int unit, drv_field_qset_t qset, drv_cfp_entry_t * drv_entry, int mode)
{
    int     idx, i, slice_id;
    int     retval = SOC_E_UNAVAIL;
    uint32 udf_num, slice_bmp, support_bmp;
    
    retval = DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_CFP_UDFS_NUM, &udf_num);
    SOC_IF_ERROR_RETURN(retval);


    slice_id = -1;
    slice_bmp = 0;
    drv_entry->slice_id = -1;
    drv_entry->slice_bmp = 0;

    /* udf idx determin the slice_id and slice_bmp*/
    for (idx = 0; idx < udf_num; idx++) {
        if (SHR_BITGET(qset.udf_map, idx)) {            
            for (i=0; i < COUNTOF(_vo_udf_mapping_tbl); i++) {
                if (_vo_udf_mapping_tbl[i].udf_id== idx) {
                    if(slice_id == -1){
                        slice_id = _vo_udf_mapping_tbl[i].slice_id;
                    }
                    if (mode == DRV_FIELD_GROUP_MODE_DOUBLE) {
                        if ((_vo_udf_mapping_tbl[i].slice_id & 0x3) != CFP_53600_SLICE_ID_BASE) {
                            LOG_WARN(BSL_LS_SOC_COMMON,
                                     (BSL_META_U(unit,
                                                 "udf conflict for double-wide mode!\n")));
                            return SOC_E_UNAVAIL;
                        } 
                        slice_id = CFP_53600_SLICE_ID_WITH_CHAIN;
                        slice_bmp = CFP_53600_SLICE_ID_MAP(slice_id);
                    } else if (mode == DRV_FIELD_GROUP_MODE_AUTO) {
                        if ((_vo_udf_mapping_tbl[i].slice_id & 0x3) != (slice_id & 0x3)) {
                            LOG_WARN(BSL_LS_SOC_COMMON,
                                     (BSL_META_U(unit,
                                                 "udf conflict for auto mode!\n")));
                            return SOC_E_UNAVAIL;
                        } 
                        /* slice_bmp bit count > 1 : double wide case*/
                        slice_bmp |= 1 << (_vo_udf_mapping_tbl[i].slice_id);
                    } else {
                        if (_vo_udf_mapping_tbl[i].slice_id != slice_id) {
                            LOG_WARN(BSL_LS_SOC_COMMON,
                                     (BSL_META_U(unit,
                                                 "udf conflict !\n")));
                            return SOC_E_UNAVAIL;
                        } 
                        slice_bmp = CFP_53600_SLICE_ID_MAP(slice_id);
                    }
                }   
            }            
        }
    }
    drv_entry->slice_id = slice_id;
    drv_entry->slice_bmp = slice_bmp;
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "after udf sets slice id %d bmp %x\n"),
                 drv_entry->slice_id, drv_entry->slice_bmp));

    support_bmp = CFP_53600_SLICE_MAP_ALL;
    for (idx = 0; idx < drvFieldQualifyCount; idx++) {
        /* Skip unused qualifiers. */
        if (0 == DRV_FIELD_QSET_TEST((qset), idx)) {
            continue;
        }
        retval = _drv_vo_ingress_qualify_support(unit, idx);
        SOC_IF_ERROR_RETURN(retval);

    
        for (i=0; i < COUNTOF(_vo_qual_slice_mapping_tbl); i++) {
            if (_vo_qual_slice_mapping_tbl[i].qual_id == idx) {

                if(slice_id != -1){
                    if (_vo_qual_slice_mapping_tbl[i].conflict_qid[0] != -1){
                        if (1 == DRV_FIELD_QSET_TEST((qset), 
                            _vo_qual_slice_mapping_tbl[i].conflict_qid[0])){
                            LOG_WARN(BSL_LS_SOC_COMMON,
                                     (BSL_META_U(unit,
                                                 "qset (EtherType/LLC ) conflict with (DSCP/IpProtocol) \n")));
                            return SOC_E_UNAVAIL; 
                        }
                        if (1 == DRV_FIELD_QSET_TEST((qset), 
                            _vo_qual_slice_mapping_tbl[i].conflict_qid[1])){
                            LOG_WARN(BSL_LS_SOC_COMMON,
                                     (BSL_META_U(unit,
                                                 "qset (EtherType/LLC ) conflict with (DSCP/IpProtocol) \n")));
                            return SOC_E_UNAVAIL; 
                        }
                    }
                    if (mode == DRV_FIELD_GROUP_MODE_AUTO) {
                        if (_shr_popcount(slice_bmp) == 1) {   
                            /* udf_id decide the slice_bmp. 
                                 should add the qual info to detremin single or double-wide */
                            if (slice_id == CFP_53600_SLICE_ID_CHAIN_SLICE) {
                                if (!(_vo_qual_slice_mapping_tbl[i].slice_bmp &  
                                        CFP_53600_SLICE_ID_MAP(4))) {
                                    slice_id = CFP_53600_SLICE_ID_WITH_CHAIN;
                                    drv_entry->slice_id = slice_id;
                                    drv_entry->slice_bmp = 
                                        CFP_53600_SLICE_ID_MAP(slice_id);
                                } 
                            } else {
                                if (!(_vo_qual_slice_mapping_tbl[i].slice_bmp & (1<< slice_id))) {
                                    LOG_WARN(BSL_LS_SOC_COMMON,
                                             (BSL_META_U(unit,
                                                         "qset conflict for AUTO mode!\n")));
                                    return SOC_E_UNAVAIL; 
                                }
                            }
                        } else {
                            if  (!(_vo_qual_slice_mapping_tbl[i].slice_bmp & 0x1)) {
                                LOG_WARN(BSL_LS_SOC_COMMON,
                                         (BSL_META_U(unit,
                                                     "qset conflict for AUTO mode!\n")));
                                return SOC_E_UNAVAIL; 
                            } 
                        }
                    }
                    if (mode == DRV_FIELD_GROUP_MODE_SINGLE) {                        
                        if (!(_vo_qual_slice_mapping_tbl[i].slice_bmp & (1<< slice_id))) {
                            LOG_WARN(BSL_LS_SOC_COMMON,
                                     (BSL_META_U(unit,
                                                 "qset conflict for SINGLE mode!\n")));
                            return SOC_E_UNAVAIL; 
                        }
                    }
                } else {
                    support_bmp &= _vo_qual_slice_mapping_tbl[i].slice_bmp;
                }
                break;
            }
        }                
    }
    if (slice_id == -1) {
        drv_entry->slice_bmp = support_bmp;
    }
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "after qual sets slice id %d bmp %x\n"),
                 drv_entry->slice_id, drv_entry->slice_bmp));
    /* copy qset to drv_entry to provide the qset info for drv_cfp_slice_id_select*/
    sal_memcpy(&drv_entry->drv_qset, &qset, sizeof(drv_field_qset_t));
    return retval;
}



int
_drv_vo_fp_entry_tcam_chain_mode_get(int unit, int stage_id, void *drv_entry, int sliceId, void *mode)    
{

    drv_cfp_tcam_t *cfp_chain;

    *((int *)mode) = 0;

    if (sliceId == CFP_53600_SLICE_ID_CHAIN_SLICE) {
        *((int *)mode) = _DRV_CFP_SLICE_CHAIN_SINGLE;
        return SOC_E_NONE;
    }
    if (sliceId  == CFP_53600_SLICE_ID_WITH_CHAIN) {
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
_drv_vo_fp_entry_tcam_slice_id_set(int unit, int stage_id, void *entry, int sliceId, void *slice_map)
{
    uint32 temp;
    uint32 id;
    drv_cfp_entry_t *drv_entry;

    if (DRV_FIELD_STAGE_INGRESS != stage_id)
        return SOC_E_PARAM;

    drv_entry = (drv_cfp_entry_t *)entry;

    drv_entry->slice_id = sliceId;
    drv_entry->slice_bmp = *((uint32 *)slice_map);


    if (drv_entry->flags & _DRV_CFP_SLICE_CHAIN) {
        id = CFP_53600_SLICE_ID_WITH_CHAIN;    
        temp = 0x3;
        soc_CFP_TCAM_SCm_field_set(unit, drv_entry->tcam_data, 
            SLICEIDf, &id);
        soc_CFP_TCAM_SCm_field_set(unit, drv_entry->tcam_mask, 
            SLICEIDf, &temp);
        id = 0;
        soc_CFP_TCAM_SCm_field_set(unit, drv_entry->cfp_chain->tcam_data, 
            SLICEIDf, &id);
        soc_CFP_TCAM_SCm_field_set(unit, drv_entry->cfp_chain->tcam_mask, 
            SLICEIDf, &temp);

    } else {
        id = sliceId;
        if (id == CFP_53600_SLICE_ID_CHAIN_SLICE) {
            id = 3;
        }
        temp = 0x3;
        soc_CFP_TCAM_SCm_field_set(unit, drv_entry->tcam_data, 
            SLICEIDf, &id);
        soc_CFP_TCAM_SCm_field_set(unit, drv_entry->tcam_mask, 
            SLICEIDf, &temp);    
    }
    return SOC_E_NONE;

}


#define config_main_entry   0   /* normal entry exclude chain{slice0,slice3} case */
#define config_chain_main   1   /* slice0 in {slice0, slice3}*/
int
_drv_vo_cfp_framing_configure(int unit, drv_cfp_entry_t *drv_entry, int flag)    
{
    int l2_flag;
    uint32 l2_frame, l2_frame_mask;
    int l3_flag;
    uint32 l3_frame, l3_frame_mask;
    int l4_flag;
    uint32 l4_frame, l4_frame_mask;
    int rv = SOC_E_NONE;

    l2_flag = drv_entry->flags & _DRV_CFP_FRAME_L2_ALL;    
    l4_flag = drv_entry->flags & _DRV_CFP_FRAME_L4_ALL;    
    l3_flag = drv_entry->flags & _DRV_CFP_FRAME_ALL;    

    if (flag == config_chain_main) {
        drv_entry->flags |= _DRV_CFP_SLICE_CONFIG_SLICE_MAIN;
    }
    if (l2_flag) {
        rv = _drv_vo_cfp_field_control(unit, drv_entry, INDEX(CFP_TCAM_SCm), 
            INDEX(L2_FRAMINGf), &l2_frame, &l2_frame_mask, 
            _cfp_field_data_mask_get);
        if (SOC_FAILURE(rv)) {
           goto framing_err; 
        }

        if (l2_flag == VO_CFP_FRAME_L2_DIXv2) {
            if (l2_frame_mask) {
                if ((l2_frame & l2_frame_mask) != 0){
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s l2 framing conflict (VO_CFP_FRAME_L2_DIXv2)!\n"),
                              FUNCTION_NAME()));
                    rv = SOC_E_UNAVAIL;
                    goto framing_err; 
                }
            } else {
                l2_frame = 0;
                l2_frame_mask = 0x3;
            }
        } else if (l2_flag == VO_CFP_FRAME_L2_SNAP_PUBLIC) {
            if (l2_frame_mask) {
                if ((l2_frame & l2_frame_mask) != 0x1){
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s l2 framing conflict (VO_CFP_FRAME_L2_SNAP_PUBLIC)!\n"),
                              FUNCTION_NAME()));
                    rv = SOC_E_UNAVAIL;
                    goto framing_err; 
                }
            } else {
                l2_frame = 0x1;
                l2_frame_mask = 0x3;
            }
        } else if (l2_flag == VO_CFP_FRAME_L2_LLC) {
            if (l2_frame_mask) {
                if ((l2_frame & l2_frame_mask) != 0x2){
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s l2 framing conflict (VO_CFP_FRAME_L2_LLC)!\n"),
                              FUNCTION_NAME()));
                    rv = SOC_E_UNAVAIL;
                    goto framing_err; 
                }
            } else {
                l2_frame = 0x2;
                l2_frame_mask = 0x3;
            }
        } else if (l2_flag == VO_CFP_FRAME_L2_SNAP_PRIVATE) {
            if (l2_frame_mask) {
                if ((l2_frame & l2_frame_mask) != 0x3){
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s l2 framing conflict (VO_CFP_FRAME_L2_SNAP_PRIVATE)!\n"),
                              FUNCTION_NAME()));
                    rv = SOC_E_UNAVAIL;
                    goto framing_err; 
                }
            }
            l2_frame = 0x3;
            l2_frame_mask = 0x3;
        } else {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s l2 framing conflict (non-handle case )!\n"),
                      FUNCTION_NAME()));  
            rv = SOC_E_UNAVAIL;
            goto framing_err; 
        }
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "%s l2 frame %x %x\n"),
                     FUNCTION_NAME(), 
                     l2_frame, l2_frame_mask));
        rv = _drv_vo_cfp_field_control(unit, drv_entry, INDEX(CFP_TCAM_SCm), 
            INDEX(L2_FRAMINGf), &l2_frame, &l2_frame_mask, 
            _cfp_field_data_mask_set);
        if (SOC_FAILURE(rv)) {
           goto framing_err; 
        }
    }
    if (l3_flag) {
        rv = _drv_vo_cfp_field_control(unit, drv_entry, INDEX(CFP_TCAM_SCm), 
            INDEX(L3_FRAMINGf), &l3_frame, &l3_frame_mask, 
            _cfp_field_data_mask_get);
        if (SOC_FAILURE(rv)) {
           goto framing_err; 
        };
        if (l3_flag == _DRV_CFP_FRAME_IP4) {
            if (l3_frame_mask) {
                if ((l3_frame & l3_frame_mask) != 0x0 ){
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s l3 framing conflict (_DRV_CFP_FRAME_IP4)!\n"),
                              FUNCTION_NAME()));                
                    rv = SOC_E_UNAVAIL;
                    goto framing_err; 
                }
            } else {
                l3_frame = 0;
                l3_frame_mask = 0x3;
            }
        } else if (l3_flag == _DRV_CFP_FRAME_IP6) {
            if (l3_frame_mask) {
                if ((l3_frame & l3_frame_mask) != 0x1 ){
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s l3 framing conflict (_DRV_CFP_FRAME_IP6)!\n"),
                              FUNCTION_NAME()));                
                    rv = SOC_E_UNAVAIL;
                    goto framing_err; 
                }
            } else {
                l3_frame = 0x1;
                l3_frame_mask = 0x3;
            }        
        } else if (l3_flag == _DRV_CFP_FRAME_NONIP) {
            if (l3_frame_mask) {
                if ((l3_frame & l3_frame_mask) != 0x3 ){
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s l3 framing conflict (_DRV_CFP_FRAME_NONIP)!\n"),
                              FUNCTION_NAME()));                
                    rv = SOC_E_UNAVAIL;
                    goto framing_err; 
                }
            } else {
                l3_frame = 0x3;
                l3_frame_mask = 0x3;
            }        
        } else if (l3_flag &  _DRV_CFP_FRAME_IPANY) {
            if (l3_frame_mask) {
                if ((l3_frame == 0x3 )|| (l3_frame_mask != 0x2)) {
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s l3 framing conflict (_DRV_CFP_FRAME_IPANY)!\n"),
                              FUNCTION_NAME()));                
                    rv = SOC_E_UNAVAIL;
                    goto framing_err; 
                }
            } else {
                l3_frame = 0;
                l3_frame_mask = 0x2;
            }
        } else if (l3_flag & _DRV_CFP_FRAME_IP) {
            if (l3_frame_mask) {
                if ((l3_frame & l3_frame_mask) == 0x3 ){
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s l3 framing conflict (_DRV_CFP_FRAME_IP)!\n"),
                              FUNCTION_NAME()));                
                    rv = SOC_E_UNAVAIL;
                    goto framing_err; 
                }
            } else {
                l3_frame = 0;
                l3_frame_mask = 0x2;
            }
        } else {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s l3 framing conflict (non-handle case )!\n"),
                      FUNCTION_NAME()));  
            rv = SOC_E_UNAVAIL;
            goto framing_err; 
        }
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "%s l3 frame %x %x\n"),
                     FUNCTION_NAME(), 
                     l3_frame, l3_frame_mask));
        rv = _drv_vo_cfp_field_control(unit, drv_entry, INDEX(CFP_TCAM_SCm), 
            INDEX(L3_FRAMINGf), &l3_frame, &l3_frame_mask, 
            _cfp_field_data_mask_set);
        if (SOC_FAILURE(rv)) {
           goto framing_err; 
        }

    }

    if ((l4_flag != 0) && (l4_flag != _DRV_CFP_FRAME_L4_ALL)){

        rv = _drv_vo_cfp_field_control(unit, drv_entry, INDEX(CFP_TCAM_SCm), 
            INDEX(L4_FRAMINGf), &l4_frame, &l4_frame_mask, 
            _cfp_field_data_mask_get);
        if (SOC_FAILURE(rv)) {
           goto framing_err; 
        }
        if (l4_flag == VO_CFP_FRAME_L4_TCP) {
            if (l4_frame_mask) {
                if ((l4_frame & l4_frame_mask) != 0){
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s l4 framing conflict (VO_CFP_FRAME_L4_TCP)!\n"),
                              FUNCTION_NAME()));
                    rv = SOC_E_UNAVAIL;
                    goto framing_err; 
                }
            } else {
                l4_frame = 0;
                l4_frame_mask = 0x3;
            }
        } else if (l4_flag == VO_CFP_FRAME_L4_UDP) {
            if (l4_frame_mask) {
                if ((l4_frame & l4_frame_mask) != 1){
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s l4 framing conflict (VO_CFP_FRAME_L4_UDP)!\n"),
                              FUNCTION_NAME()));
                    rv = SOC_E_UNAVAIL;
                    goto framing_err; 
                }
            } else {
                l4_frame = 1;
                l4_frame_mask = 0x3;
            }        
        } else if (l4_flag == VO_CFP_FRAME_L4_ICMPIGMP) {
            if (l4_frame_mask) {
                if ((l4_frame & l4_frame_mask) != 2){
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s l4 framing conflict (VO_CFP_FRAME_L4_ICMPIGMP)!\n"),
                              FUNCTION_NAME()));
                    rv = SOC_E_UNAVAIL;
                    goto framing_err; 
                }
            } else {
                l4_frame = 2;
                l4_frame_mask = 0x3;
            }        
        } else if (l4_flag == VO_CFP_FRAME_L4_OTHER) {
            if (l4_frame_mask) {
                if ((l4_frame & l4_frame_mask) != 3){
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s l4 framing conflict (VO_CFP_FRAME_L4_OTHER)!\n"),
                              FUNCTION_NAME()));
                    rv = SOC_E_UNAVAIL;
                    goto framing_err; 
                }
            } else {
                l4_frame = 3;
                l4_frame_mask = 0x3;
            }       
        } else if (l4_flag & VO_CFP_FRAME_L4_TCPUDP) {
            if (l4_frame_mask) {
                if (((l4_frame & l4_frame_mask) != 3) || 
                        ((l4_frame & l4_frame_mask) != 2)){
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s l4 framing conflict (VO_CFP_FRAME_L4_TCPUDP)!\n"),
                              FUNCTION_NAME()));                
                    rv = SOC_E_UNAVAIL;
                    goto framing_err; 
                }
            } else {
                l4_frame = 0;
                l4_frame_mask = 0x2;
            }
        } else {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s l4 framing conflict (non-handle case )!\n"),
                      FUNCTION_NAME()));  
            rv = SOC_E_UNAVAIL;
            goto framing_err; 
        }
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "%s l4 frame %x %x\n"),
                     FUNCTION_NAME(), 
                     l4_frame, l4_frame_mask));
        rv = _drv_vo_cfp_field_control(unit, drv_entry, INDEX(CFP_TCAM_SCm), 
            INDEX(L4_FRAMINGf), &l4_frame, &l4_frame_mask, 
            _cfp_field_data_mask_set);
        if (SOC_FAILURE(rv)) {
           goto framing_err; 
        }
    }
framing_err:
    if (flag == config_chain_main) {
        drv_entry->flags &=~( _DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
    }
    return rv;
}


int
_drv_vo_fp_entry_cfp_tcam_policy_install(int unit, void *entry, int tcam_idx, 
        int tcam_chain_idx)
{
    uint32 temp;
    drv_cfp_entry_t *drv_entry = (drv_cfp_entry_t *)entry;
    int rv=SOC_E_NONE;
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
            (unit, DRV_CFP_RAM_TCAM_MASK, DRV_CFP_FIELD_VALID, 
            drv_entry, &temp));

     if (drv_entry->slice_id != CFP_53600_SLICE_ID_WITH_CHAIN) {
        SOC_IF_ERROR_RETURN(
            _drv_vo_cfp_framing_configure(unit, drv_entry, config_main_entry));
     }

    if (!(drv_entry->flags & _DRV_CFP_ACTION_INIT)){
        /* BYPASS all the FILTER as default */
        rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_FILTER_ALL, drv_entry, 0, 0);
        SOC_IF_ERROR_RETURN(rv);
        drv_entry->flags |= _DRV_CFP_ACTION_INIT;
    }

    if (tcam_chain_idx != -1) {
    /* entry with chain {slice0,slice3}*/        
        /* configure slice0 framing*/
        SOC_IF_ERROR_RETURN(
        _drv_vo_cfp_framing_configure(unit, drv_entry, config_chain_main));
        drv_entry->flags |= _DRV_CFP_SLICE_CONFIG_SLICE_MAIN;

        /* write the slice0 data tcam*/
        rv = DRV_CFP_ENTRY_WRITE
            (unit, tcam_idx, DRV_CFP_RAM_TCAM, drv_entry);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Write Tcam entry index = %d fail.\n"),  
                       tcam_idx));
            drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
            return rv;
        }

        /* internally handle the classification id used to chain slice3*/
        if (drv_entry->cfp_chain->chain_id == -1) {
            rv = DRV_FP_ID_CONTROL(unit, _DRV_FP_ID_CFP_CHAIN_ID, 
                _DRV_FP_ID_CTRL_ALLOC, 0, &chain_id, NULL);

            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "FP Error: Can't get chain_id for Tcam entry index = %d fail.\n"),  
                           tcam_idx));
                drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
                return rv;
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
        rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CHAIN_ID, 
                        drv_entry, 0, 0);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Can't set chain_id action for Tcam entry %d..\n"),  
                       tcam_idx));
            drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
            return rv;
        }

        rv = DRV_CFP_ACTION_SET(unit, DRV_CFP_ACT_CLASSFICATION_ID, 
                        drv_entry, chain_id, 0);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: set chain_id %d action fail.\n"),  
                       chain_id));
            drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_MAIN);
            return rv;
        }

        /* write the slice0 action */
        rv = DRV_CFP_ENTRY_WRITE
            (unit, tcam_idx, DRV_CFP_RAM_ACT, drv_entry);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Write Act/Pol entry index = %d fail.\n"),  
                       tcam_idx));
            return rv;
        }

        drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_MAIN);


        /* qualify slice3 classification id */     
        drv_entry->flags |= _DRV_CFP_SLICE_CONFIG_SLICE_CHAIN;
        temp = 0xfff;
        rv = _drv_vo_cfp_qual_value_set(unit, drvFieldQualifyClassId, 
                        drv_entry, (uint32 *)&chain_id, &temp);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: set qualify chain_id %d fail.\n"),  
                       chain_id));
            return rv;
        }

        temp = 1;
        DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM, DRV_CFP_FIELD_VALID, drv_entry, &temp);

        DRV_CFP_FIELD_SET
            (unit, DRV_CFP_RAM_TCAM_MASK, 
                DRV_CFP_FIELD_VALID, drv_entry, &temp);

        drv_entry->flags &= ~(_DRV_CFP_SLICE_CONFIG_SLICE_CHAIN);

        /* write the slice3 data and action tcam*/
        rv = DRV_CFP_ENTRY_WRITE
            (unit, tcam_chain_idx, DRV_CFP_RAM_ACT, drv_entry);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Write Act/Pol entry index = %d fail.\n"),  
                       tcam_idx));
            return rv;
        }
        rv = DRV_CFP_ENTRY_WRITE
            (unit, tcam_chain_idx, DRV_CFP_RAM_TCAM, drv_entry);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Write Tcam entry index = %d fail.\n"),  
                       tcam_idx));
            return rv;
        }

    } else {

    /* normal entry */
        rv = DRV_CFP_ENTRY_WRITE
            (unit, tcam_idx, DRV_CFP_RAM_ACT, drv_entry);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Write Act/Pol entry index = %d fail.\n"),  
                       tcam_idx));
            return rv;
        }
        rv = DRV_CFP_ENTRY_WRITE
            (unit, tcam_idx, DRV_CFP_RAM_TCAM, drv_entry);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "FP Error: Write Tcam entry index = %d fail.\n"),  
                       tcam_idx));
            return rv;
        }
    }
    return rv;
}



int
drv_vo_cfp_slice_id_select(int unit, drv_cfp_entry_t *entry, uint32 *slice_id, uint32 flags)
{
    int qual_num, i;
    uint32 slice_bmp;

    /* slice id from udf */
    if (entry->slice_id != -1){
        if (_shr_popcount(entry->slice_bmp) > 1) {
            *slice_id = CFP_53600_SLICE_ID_WITH_CHAIN;
            entry->slice_id = CFP_53600_SLICE_ID_WITH_CHAIN;
            entry->slice_bmp = CFP_53600_SLICE_MAP_DOUBLE_WIDE;
        } else if (entry->slice_bmp == 
            CFP_53600_SLICE_ID_MAP(CFP_53600_SLICE_ID_WITH_CHAIN)){
            *slice_id = CFP_53600_SLICE_ID_WITH_CHAIN;
        } else {
            *slice_id = entry->slice_id;
        }
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "%s  %d slice_id %d entry slice id %d bmp %x \n"),
                     FUNCTION_NAME(), __LINE__,
                     *slice_id, entry->slice_id, entry->slice_bmp));
        sal_memset(&entry->drv_qset, 0, sizeof(drv_field_qset_t));
        return SOC_E_NONE;
    }
    /* slice id from mode (single/auto/double-wide) */
    if (flags == DRV_FIELD_QUAL_CHAIN) {
        /* double wide must be slice 3*/
        *slice_id = CFP_53600_SLICE_ID_WITH_CHAIN;
        entry->slice_id = CFP_53600_SLICE_ID_WITH_CHAIN;
        entry->slice_bmp = CFP_53600_SLICE_MAP_DOUBLE_WIDE;
    } else {
        /* single or auto mode */
        slice_bmp = entry->slice_bmp;

        qual_num = 0;
        for (i=0; i<_SHR_BITDCLSIZE(DRV_FIELD_QUALIFY_MAX); i++) {
            qual_num += _shr_popcount(entry->drv_qset.w[i]);
        }

        if (_shr_popcount(slice_bmp)  == 1) {
            for (i = 0; i < 2; i++){
                if ((slice_bmp >> i) & 0x1) {
                    *slice_id = i;
                    break;
                }
            }            
        } else {
            if ( qual_num < CFP_53600_SLICE_PRI_LEVEL0) {
                /* choose the lowest id*/
                for (i = 0; i < 2; i++){
                    if ((slice_bmp >> i) & 0x1) {
                        *slice_id = i;
                        break;
                    }
                }
            } else if ((CFP_53600_SLICE_PRI_LEVEL0 <= qual_num )
                && (qual_num < CFP_53600_SLICE_PRI_LEVEL1) ){
            /* choose id = 1 , then 2, then 0*/
                if (slice_bmp & CFP_53600_SLICE_ID_MAP(1)){
                    *slice_id = 1;
                } else if (slice_bmp & CFP_53600_SLICE_ID_MAP(2)) {
                    *slice_id = 2; 
                } else if (slice_bmp & CFP_53600_SLICE_ID_MAP(0)){
                    *slice_id = 0;
                }
            } else {
                /* choose id = 2, then 1, then 0*/
                if (slice_bmp & CFP_53600_SLICE_ID_MAP(2)) {
                    *slice_id = 2; 
                } else if (slice_bmp & CFP_53600_SLICE_ID_MAP(1)){
                    *slice_id = 1;
                } else if (slice_bmp & CFP_53600_SLICE_ID_MAP(0)){
                    *slice_id = 0;
                }
            }
        }        
        entry->slice_id = *slice_id;
    }
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s %d slice_id %d entry slice id %d bmp %x \n"),
                 FUNCTION_NAME(), __LINE__,
                 *slice_id, entry->slice_id, entry->slice_bmp));

    /* clear the drv_qset. the drv_qset will be updated while the actaul qual value is configuerd */
    sal_memset(&entry->drv_qset, 0, sizeof(drv_field_qset_t));

    return SOC_E_NONE;
}

/*
 * Function: drv_vo_cfp_udf_get
 *
 * Purpose:
 *     Get the offset value of the User Defined fields.
 *
 * Parameters:
 *     unit - BCM device number
 *     port - The UDFs are global settings not related to port.
 *               here, we use this parameter to differentiate the result 
 *               we get from the register offset or just the mapping between
 *               the bcm layer and the driver service layer.
 *               port = 0  (DRV_CFP_UDF_OFFSET_GET)
 *                           bcm udf_id <--> register level access
 *               port = DRV_CFP_UDF_QUAL_GET
 *                           bcm udf_id <--> driver service level mapping
 *     udf_index -the index of user defined fields
 *     offset(OUT) - when port = 0, offset value
 *                                  port != 0, DRV_CFP_QUAL_UDFxxx
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 *     
 */
int 
drv_vo_cfp_udf_get(int unit, uint32 port, uint32 udf_index, 
    uint32 *offset, uint32 *base)
{
    int rv = SOC_E_NONE;
    uint32  temp, reg_32;

    if (udf_index >=  CFP_53600_UDF_NUM_MAX) {
        return SOC_E_CONFIG;
    }
    if (port == DRV_CFP_UDF_LENGTH_GET){
        *offset = _vo_udf_mapping_tbl[udf_index].udf_len; 
        return SOC_E_NONE;
    }

    rv = VO_CFP_UDF_GET(unit, udf_index, &reg_32);
    SOC_IF_ERROR_RETURN(rv);

    rv = soc_CFP_UDF_0_Ar_field_get(unit, &reg_32, UDF_N_X_OFFSETf, &temp);
    SOC_IF_ERROR_RETURN(rv);

    *offset = temp * 2;

    soc_CFP_UDF_0_Ar_field_get(unit, &reg_32, UDF_N_X_REFf, &temp);
    switch(temp) {
        case 0:
            *base = DRV_CFP_UDF_OFFSET_BASE_START_OF_FRAME;
            break;
        case 1:
            *base = DRV_CFP_UDF_OFFSET_BASE_END_OF_TAG;
            break;
        case 2:
            *base = DRV_CFP_UDF_OFFSET_BASE_END_OF_L2_HDR;
            break;
        case 3:
            *base = DRV_CFP_UDF_OFFSET_BASE_END_OF_L3_HDR;
            break;
    }

    return rv;
}

int 
drv_vo_cfp_udf_set(int unit, uint32 port, uint32 udf_index, 
    uint32 offset, uint32 base)
{

    int rv = SOC_E_NONE;
    uint32  reg_32, temp;

    if (udf_index >=  CFP_53600_UDF_NUM_MAX) {
        return SOC_E_CONFIG;
    }
    if (offset > CFP_53600_UDF_OFFSET_MAX) {
        return SOC_E_CONFIG;
    }
    rv = VO_CFP_UDF_GET(unit, udf_index, &reg_32);

    SOC_IF_ERROR_RETURN(rv);

    temp = (offset /2 ) & 0x1f; /* the offset is 2N bytes based */

    soc_CFP_UDF_0_Ar_field_set(unit, &reg_32, UDF_N_X_OFFSETf, &temp);
    
    switch(base) {
        case DRV_CFP_UDF_OFFSET_BASE_START_OF_FRAME:
            temp = 0;
            break;
        case DRV_CFP_UDF_OFFSET_BASE_END_OF_TAG:
            temp = 1;
            break;
        case DRV_CFP_UDF_OFFSET_BASE_END_OF_L2_HDR:
            temp = 2;
            break;
        case DRV_CFP_UDF_OFFSET_BASE_END_OF_L3_HDR:
            temp = 3;
            break;
        default:
            return SOC_E_PARAM;
    }

    soc_CFP_UDF_0_Ar_field_set(unit, &reg_32, UDF_N_X_REFf, &temp);

    VO_CFP_UDF_SET(unit, udf_index, &reg_32);    
    return rv;
}

int 
drv_vo_cfp_range_set(int unit, uint32 type, uint32 id, 
        uint32 param1, uint32 param2) {

    uint32 field_select, reg_val;
    int rv = SOC_E_NONE;
    
    switch(type) {
        case DRV_CFP_RANGE_INNER_VLAN:
            field_select = 0;
             if ((param1 & 0xF00) ||(param2 & 0xF00) ) {
                field_select |=0x2;
             }
            break;
        case DRV_CFP_RANGE_OUTER_VLAN:
            field_select = 1;
             if ((param1 & 0xF00) ||(param2 & 0xF00) ) {
                field_select |=0x2;
             }            
            break;     
        case DRV_CFP_RANGE_DSTPORT:
            field_select = 4;
            break;
        case DRV_CFP_RANGE_SRCPORT:
            field_select = 5;
            break;           
        default:
            rv = SOC_E_UNAVAIL;
    }        
    SOC_IF_ERROR_RETURN(rv);
    reg_val = 0;
    soc_RANGE_CHECKER_FIELD_SELr_field_set(unit, &reg_val, FIELD_SELECTf, &field_select);
    rv = REG_WRITE_RANGE_CHECKER_FIELD_SELr(unit, id, &reg_val);
    SOC_IF_ERROR_RETURN(rv);    

    reg_val = 0;
    soc_RANGE_CHECKERr_field_set(unit, &reg_val, LOW_VALf, &param1);
    soc_RANGE_CHECKERr_field_set(unit, &reg_val, HIGH_VALf, &param2);
    rv = REG_WRITE_RANGE_CHECKERr(unit, id, &reg_val);
    SOC_IF_ERROR_RETURN(rv);

    return rv;
}

int 
drv_vo_cfp_range_get(int unit, uint32 type, uint32 id, 
        uint32 *param1, uint32 *param2) {

    uint32 reg_val;
    int rv;
    
    rv = REG_READ_RANGE_CHECKERr(unit, id, &reg_val);
    SOC_IF_ERROR_RETURN(rv);    
    soc_RANGE_CHECKERr_field_get(unit, &reg_val, LOW_VALf, param1);
    soc_RANGE_CHECKERr_field_get(unit, &reg_val, HIGH_VALf, param2);
    return rv;
}

#define _VO_TCAM_L_LEN  229
#define _VO_TCAM_L_WORD    8
int
_drv_vo_cfp_mem_hw2sw(int unit, uint32 *hw_entry, uint32 *sw_entry){

    int len, wp, bp,i;
    uint32 mask, mask_lo, mask_hi;

    sal_memcpy(sw_entry, hw_entry, _VO_TCAM_L_WORD * sizeof(uint32));

    wp = _VO_TCAM_L_WORD - 1; /* 7 */
    bp = (_VO_TCAM_L_LEN % 32) + 1; /* 6 */
    i = _VO_TCAM_L_WORD; /* 8 */
    for (len = _VO_TCAM_L_LEN; len > 0; len-=32) {
        mask = 0xffffffff;
        mask_lo = mask;
        mask_hi = 0;
        mask_lo = mask << bp;
        mask_hi = mask >> (32 -bp);
        sw_entry[wp] &= ~mask_lo;
        sw_entry[wp++] |= ((hw_entry[i] << bp) & mask_lo);
        sw_entry[wp] &=~(mask_hi);
        sw_entry[wp] |= ((hw_entry[i] >> (32-bp)) & mask_hi);
        i++;                
    };

    return SOC_E_NONE;
}


int
_drv_vo_cfp_mem_sw2hw(int unit, uint32 *sw_entry, uint32 *hw_entry){
    int len, wp, bp,i;
    uint32 mask, mask_lo, mask_hi;

    wp = _VO_TCAM_L_WORD - 1; /* 7 */
    bp = 32 - ((_VO_TCAM_L_LEN % 32) + 1); /* 26 */
    i = _VO_TCAM_L_WORD - 1;
    for (len = _VO_TCAM_L_LEN; len > 0; len-=32) {
        mask = 0xffffffff;
        mask_lo = mask;
        mask_hi = 0;
        mask_lo = mask << bp;
        mask_hi = mask >> (32 -bp);
        hw_entry[wp] &= ~mask_lo;
        hw_entry[wp++] |= ((sw_entry[i] << bp) & mask_lo);

        hw_entry[wp] &=~(mask_hi);
        hw_entry[wp] |= ((sw_entry[i] >> (32-bp)) & mask_hi);
        i++;
    };

    sal_memcpy(&hw_entry[0], &sw_entry[0], _VO_TCAM_L_WORD*sizeof(uint32));

    hw_entry[7] &= 0x3f;

    return SOC_E_NONE;
}


int
_drv_vo_cfp_data_mask_read(int unit, uint32 ram_type, 
                         uint32 index, drv_cfp_entry_t *cfp_entry)
{

    int rv = SOC_E_NONE;
    uint32 *mem_entry, *tcam_data, *tcam_mask;
    int size, mem_len, read_data, read_mask;
        
    assert(cfp_entry);

    read_data = -1;
    read_mask = -1;
    if (ram_type == DRV_MEM_CFP_DATA_MASK) {
        read_data = 1;
        read_mask = 1;
    } 
    if (ram_type == DRV_MEM_TCAM_DATA) {
        read_data = 1;
    }
    if (ram_type == DRV_MEM_TCAM_MASK) {
        read_mask = 1;
    }

    if ((read_data == -1) && (read_mask == -1) ) {
        return SOC_E_CONFIG;
    }
 
    mem_len = soc_mem_entry_words(unit, INDEX(CFP_DATAm));

    size = mem_len * sizeof(uint32);
    mem_entry = (uint32 *) sal_alloc(size, "cfp mem entry");
    if (mem_entry == NULL) {
        return SOC_E_RESOURCE;
    }

    if (cfp_entry->flags & _DRV_CFP_SLICE_CONFIG_SLICE_MAIN) {
        tcam_data = cfp_entry->cfp_chain->tcam_data;
        tcam_mask = cfp_entry->cfp_chain->tcam_mask; 
    } else {
        tcam_data = cfp_entry->tcam_data;
        tcam_mask = cfp_entry->tcam_mask; 
    }

    if (1 == read_data) {
        sal_memset(mem_entry, 0, size);
        sal_memset(tcam_data, 0, sizeof(cfp_entry->tcam_data));

        rv = DRV_MEM_READ(unit, INDEX(CFP_DATAm), index, 1, mem_entry);
        if (SOC_FAILURE(rv)) {
            sal_free(mem_entry);
            return rv;
        }

        rv = _drv_vo_cfp_mem_hw2sw(unit, mem_entry, tcam_data);
        if (SOC_FAILURE(rv)) {
            sal_free(mem_entry);
            return rv;
        }
    }

    if (1 == read_mask) {
        sal_memset(mem_entry, 0, size);
        sal_memset(tcam_mask, 0, sizeof(cfp_entry->tcam_mask));

        rv = DRV_MEM_READ(unit, INDEX(CFP_MASKm), index, 1, mem_entry);
        if (SOC_FAILURE(rv)) {
            sal_free(mem_entry);
            return rv;
        }
        rv = _drv_vo_cfp_mem_hw2sw(unit, mem_entry, tcam_mask);
        if (SOC_FAILURE(rv)) {
            sal_free(mem_entry);
            return rv;
        }        
    }

    sal_free(mem_entry);
    return rv;

}

int
_drv_vo_cfp_data_mask_write(int unit, uint32 ram_type, 
                              uint32 index, drv_cfp_entry_t *cfp_entry)
{
    int rv = SOC_E_NONE;
    uint32 *mem_entry, *tcam_data, *tcam_mask, valid;
    int size, mem_len, write_data, write_mask, mask_offset;
    
    assert(cfp_entry);
    
    write_data = -1;
    write_mask = -1;
    if (ram_type == DRV_MEM_CFP_DATA_MASK) {
        write_data = 1;
        write_mask = 1;
    } 
    if (ram_type == DRV_MEM_TCAM_DATA) {
        write_data = 1;
    }
    if (ram_type == DRV_MEM_TCAM_MASK) {
        write_mask = 1;
    }

    if ((write_data == -1) && (write_mask == -1) ) {
        return SOC_E_CONFIG;
    }
 
	if (ram_type == DRV_MEM_CFP_DATA_MASK) {
		mem_len = soc_mem_entry_words(unit, INDEX(CFP_DATA_MASKm));
	} else{
        mem_len = soc_mem_entry_words(unit, INDEX(CFP_DATAm));
 	}
	mask_offset = soc_mem_entry_words(unit, INDEX(CFP_DATAm));	
    size = mem_len * sizeof(uint32);
    mem_entry = (uint32 *) sal_alloc(size, "cfp mem entry");
    if (mem_entry == NULL) {
        return SOC_E_RESOURCE;
    }

    if (cfp_entry->flags & _DRV_CFP_SLICE_CONFIG_SLICE_MAIN) {
        tcam_data = cfp_entry->cfp_chain->tcam_data;
        tcam_mask = cfp_entry->cfp_chain->tcam_mask; 
    } else {
        tcam_data = cfp_entry->tcam_data;
        tcam_mask = cfp_entry->tcam_mask; 
    }
	if (ram_type == DRV_MEM_CFP_DATA_MASK) {	
        sal_memset(mem_entry, 0, size);
        rv = _drv_vo_cfp_mem_sw2hw(unit, tcam_data, mem_entry);
        if (SOC_FAILURE(rv)) {
            sal_free(mem_entry);
            return rv;
        }
        soc_CFP_DATAm_field_get(unit, mem_entry, CFP_TCAM_DATA_H_VALIDf, &valid);
        soc_CFP_DATAm_field_set(unit, mem_entry, CFP_TCAM_DATA_L_VALIDf, &valid);

        rv = _drv_vo_cfp_mem_sw2hw(unit, tcam_mask, &mem_entry[mask_offset]);
        if (SOC_FAILURE(rv)) {
            sal_free(mem_entry);
            return rv;
        }
		/* There's no need to configure the mask valid bits, 
		since data and mask valid bits are tied together in chip.
		However, there's no harm to set it again. */
        soc_CFP_DATAm_field_get(unit, &mem_entry[mask_offset], 
            CFP_TCAM_DATA_H_VALIDf, &valid);
        soc_CFP_DATAm_field_set(unit, &mem_entry[mask_offset], 
            CFP_TCAM_DATA_L_VALIDf, &valid);		

        rv = DRV_MEM_WRITE(unit, INDEX(CFP_DATA_MASKm),index, 1, mem_entry);
        if (SOC_FAILURE(rv)) {
            sal_free(mem_entry);
            return rv;
        }
	} else {
        if (1 == write_data) {
            sal_memset(mem_entry, 0, size);
            rv = _drv_vo_cfp_mem_sw2hw(unit, tcam_data, mem_entry);
            if (SOC_FAILURE(rv)) {
                sal_free(mem_entry);
                return rv;
            }

            soc_CFP_DATAm_field_get(unit, mem_entry, CFP_TCAM_DATA_H_VALIDf, &valid);
            soc_CFP_DATAm_field_set(unit, mem_entry, CFP_TCAM_DATA_L_VALIDf, &valid);

            rv = DRV_MEM_WRITE(unit, INDEX(CFP_DATAm),index, 1, mem_entry);
            if (SOC_FAILURE(rv)) {
                sal_free(mem_entry);
                return rv;
            }
        }

        if (1 == write_mask) {
            sal_memset(mem_entry, 0, size);
            rv = _drv_vo_cfp_mem_sw2hw(unit, tcam_mask, mem_entry);
            if (SOC_FAILURE(rv)) {
                sal_free(mem_entry);
                return rv;
            }

            soc_CFP_DATAm_field_get(unit, mem_entry, CFP_TCAM_DATA_H_VALIDf, &valid);

            soc_CFP_DATAm_field_set(unit, mem_entry, CFP_TCAM_DATA_L_VALIDf, &valid);

            rv = DRV_MEM_WRITE(unit, INDEX(CFP_MASKm), index, 1, mem_entry);
            if (SOC_FAILURE(rv)) {
                sal_free(mem_entry);
                return rv;
            }
	    }
    }
    sal_free(mem_entry);
    return rv;
}


int
_drv_vo_cfp_field_set(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val)
{
    int i, fld, rv = SOC_E_UNAVAIL;
    
    if (!((mem_type == DRV_CFP_RAM_TCAM) ||
        (mem_type == DRV_CFP_RAM_TCAM_MASK))) {
        return rv;
    }
    fld = -1;
    for (i=0; i < COUNTOF(vo_cfp_field_map_table); i ++) {
        if (vo_cfp_field_map_table[i].cfp_field_type == field_type) {
            fld = vo_cfp_field_map_table[i].field_id;
            break;
        }
    }
    if (fld == -1) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "_vo_cfp_field_set DRV_CFP_FIELD_XXX %d "
                             "can't find the matched field.\n"),
                  field_type));
        return SOC_E_UNAVAIL;
    }

    if ((fld == INDEX(VALIDf)) &&  (*fld_val != 0)) {
        *fld_val = 0x3;
    }

    if (mem_type == DRV_CFP_RAM_TCAM) {
        rv = _drv_vo_cfp_field_control(unit, entry, INDEX(CFP_TCAM_SCm), 
            fld, fld_val, NULL, 
            _cfp_field_data_set);
    }

    if (mem_type == DRV_CFP_RAM_TCAM_MASK) {
        rv = _drv_vo_cfp_field_control(unit, entry, INDEX(CFP_TCAM_SCm), 
            fld, NULL, fld_val, 
            _cfp_field_mask_set);
    }

    return rv;
}

int
_drv_vo_cfp_field_get(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val)
{
    int i, fld, rv = SOC_E_UNAVAIL;
    
    if (!((mem_type == DRV_CFP_RAM_TCAM) ||
        (mem_type == DRV_CFP_RAM_TCAM_MASK))) {
        return rv;
    }

    fld = -1;
    for (i=0; i < COUNTOF(vo_cfp_field_map_table); i ++) {
        if (vo_cfp_field_map_table[i].cfp_field_type == field_type) {
            fld = vo_cfp_field_map_table[i].field_id;
            break;
        }
    }
    if (fld == -1) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "_vo_cfp_field_set DRV_CFP_FIELD_XXX %d "
                             "can't find the matched field.\n"),
                  field_type));
        return SOC_E_UNAVAIL;
    }

    if (mem_type == DRV_CFP_RAM_TCAM) {
        rv = _drv_vo_cfp_field_control(unit, entry, INDEX(CFP_TCAM_SCm), 
            fld, fld_val, NULL, 
            _cfp_field_data_get);
    }

    if (mem_type == DRV_CFP_RAM_TCAM_MASK) {
        rv = _drv_vo_cfp_field_control(unit, entry, INDEX(CFP_TCAM_SCm), 
            fld, NULL, fld_val, 
            _cfp_field_mask_get);
    }

    return rv;
}

