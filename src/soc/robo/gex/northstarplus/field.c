/*
 * $Id: field.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 *  Field driver service.
 *  Purpose: Handle the chip variant design for Field Processor
 * 
 */

#include <soc/error.h>
#include <soc/types.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/drv_if.h>


/* Policer _Mode */
#define FIELD_53020_POLICER_MODE_RFC2698    (0x0)
#define FIELD_53020_POLICER_MODE_RFC4115    (0x1)
#define FIELD_53020_POLICER_MODE_MEF        (0x2)
#define FIELD_53020_POLICER_MODE_DISABLE    (0x3)



STATIC int
_drv_nsp_fp_policer_support_check(int unit, int stage_id, drv_policer_config_t *policer_cfg)
{
    int rv;

    if (stage_id != DRV_FIELD_STAGE_INGRESS) {
        return SOC_E_PARAM;    
    }
    switch (policer_cfg->mode) {
        case drvPolicerModeCommitted:
        case drvPolicerModePassThrough:
        case drvPolicerModeSrTcm: /* RFC 2697 */
        case drvPolicerModeTrTcm: /* RFC 2698 */
        case drvPolicerModeTrTcmDs: /* RFC 4115 */
        case drvPolicerModeCoupledTrTcmDs: /* MEF */
            rv = SOC_E_NONE;
            break;
        default:
            rv = SOC_E_PARAM;
            break;
    }
    return (rv);

}


STATIC int
_drv_nsp_fp_policer_config(int unit,  int stage_id, void *entry, drv_policer_config_t *policer_cfg) 
{
    drv_cfp_entry_t *drv_entry;
    uint32  cir_en, eir_en, bucket_size, ref_cnt, mode, fld_val;
    uint32  action, coupling;
    uint32 *meter;

    if (stage_id != DRV_FIELD_STAGE_INGRESS) {
        return SOC_E_PARAM;
    }

    drv_entry = (drv_cfp_entry_t *)entry;
    drv_entry->pl_cfg = policer_cfg;

    cir_en = 0;
    eir_en = 0;
    mode = 0;
    action = 0;
    coupling = 0;

    meter = drv_entry->meter_data;
    sal_memset(meter, 0, sizeof(drv_entry->meter_data));
    
    switch(policer_cfg->mode){
        case drvPolicerModeCommitted:
            cir_en = 1;
            mode = FIELD_53020_POLICER_MODE_RFC4115;
            action = 1;
            break;
        case drvPolicerModePassThrough:
            mode = FIELD_53020_POLICER_MODE_DISABLE;
            break;
        case drvPolicerModeSrTcm: /* RFC 2697 */
            policer_cfg->pkbits_sec = 0; /* clear it */
            cir_en = 1;
            eir_en = 1;
            mode = FIELD_53020_POLICER_MODE_MEF;
            coupling = 1;
            break;
        case drvPolicerModeTrTcm: /* RFC 2698 */
            cir_en = 1;
            eir_en = 1;
            mode = FIELD_53020_POLICER_MODE_RFC2698;
            action = 1;
            break;
        case drvPolicerModeTrTcmDs: /* RFC 4115 */
            cir_en = 1;
            eir_en = 1;
            mode = FIELD_53020_POLICER_MODE_RFC4115;
            action = 1;
            break;
        case drvPolicerModeCoupledTrTcmDs: /* MEF */
            cir_en = 1;
            eir_en = 1;
            mode = FIELD_53020_POLICER_MODE_MEF;
            coupling = 1;
            break;
        default:
            return SOC_E_PARAM;
    }

    DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
        DRV_CFP_FIELD_POLICER_MODE, drv_entry, &mode);
    DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
        DRV_CFP_FIELD_COUPLING_EN, drv_entry, &coupling);
    if (cir_en) {
        SOC_IF_ERROR_RETURN(
            DRV_CFP_METER_RATE_TRANSFORM(unit, policer_cfg->ckbits_sec, 
            policer_cfg->ckbits_burst, &bucket_size, &ref_cnt, NULL));
        DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_CIR_BUCKET_SIZE, drv_entry, &bucket_size);
        DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_CIR_RATE, drv_entry, &ref_cnt);
    }
    if (eir_en) {
        SOC_IF_ERROR_RETURN(
            DRV_CFP_METER_RATE_TRANSFORM(unit, policer_cfg->pkbits_sec, 
            policer_cfg->pkbits_burst, &bucket_size, &ref_cnt, NULL));
        DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_EIR_BUCKET_SIZE, drv_entry, &bucket_size);
        DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
            DRV_CFP_FIELD_EIR_RATE, drv_entry, &ref_cnt);
    }

    
    /* Check flags */
    if (policer_cfg->flags & DRV_POLICER_DROP_RED) {
        action = 0; /* Red packets are dropped */
    }
    DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
        DRV_CFP_FIELD_POLICER_ACT, drv_entry, &action);

    if (policer_cfg->flags & DRV_POLICER_COLOR_BLIND) {
        fld_val = 1;
    } else {
        fld_val = 0;
    }
    DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
        DRV_CFP_FIELD_COLOR_MODE, drv_entry, &fld_val);
    
    return SOC_E_NONE;
}



int
drv_nsp_fp_policer_control(int unit,  int stage_id, int op, void *entry, drv_policer_config_t *policer_cfg) 
{

    int rv = SOC_E_NONE;
    drv_cfp_entry_t *drv_entry;
    uint32  temp;

    switch (op) {
        case DRV_FIELD_POLICER_MODE_SUPPORT:
            rv = _drv_nsp_fp_policer_support_check(unit, stage_id, policer_cfg);
            break;
        case DRV_FIELD_POLICER_CONFIG:
            rv =  _drv_nsp_fp_policer_config(unit, stage_id, entry, policer_cfg);
            break;                
        case DRV_FIELD_POLICER_FREE:
            drv_entry = (drv_cfp_entry_t *)entry;
            sal_memset(drv_entry->meter_data, 0, sizeof(drv_entry->meter_data));
            temp = FIELD_53020_POLICER_MODE_DISABLE; 
            rv = DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_METER, 
                DRV_CFP_FIELD_POLICER_MODE, drv_entry, &temp);
            rv =  SOC_E_NONE;
            break;                
    }
    return rv;
}

int
_drv_nsp_fp_counter_mode_support(int unit, int param0, void *mode) 
{
    int policer_valid;

    policer_valid = *((int *)mode);

    if (policer_valid) {
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
    } else {
        if (param0 != DRV_FIELD_COUNTER_MODE_NO_YES){
            return SOC_E_UNAVAIL;
        }
    }
    return SOC_E_NONE;
}

int
drv_nsp_fp_stat_type_get(int unit, int stage_id, drv_policer_mode_t policer_mode,
        drv_field_stat_t stat, int *type1, int *type2, int *type3)
{
    *type1 = -1;
    *type2 = -1;
    *type3 = -1;
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
        case drvFieldStatRedPackets:
            *type1 = DRV_CFP_STAT_RED;
            break;
        case drvFieldStatYellowPackets:
            *type1 = DRV_CFP_STAT_YELLOW;
            break;
        case drvFieldStatNotGreenPackets:
            *type1 = DRV_CFP_STAT_YELLOW;
            *type2 = DRV_CFP_STAT_RED;
            break;
        case drvFieldStatNotRedPackets:
            *type2 = DRV_CFP_STAT_YELLOW;
            *type1 = DRV_CFP_STAT_GREEN;
            break;
        case drvFieldStatNotYellowPackets:
            *type1 = DRV_CFP_STAT_RED;
            *type2 = DRV_CFP_STAT_GREEN;
            break;
        default:
            return SOC_E_PARAM;
    }        
    return SOC_E_NONE;
}


int
_drv_nsp_fp_stat_mode_support(int unit, int stage_id, drv_policer_mode_t policer_mode, void *mode)
{
    drv_field_stat_t f_stat;
    int type1, type2, type3;
    int rv;

    f_stat = *((drv_field_stat_t *)mode);
    type1 = type2 = type3 =-1;
    rv = drv_nsp_fp_stat_type_get(unit, stage_id, policer_mode,
        f_stat, &type1, &type2, &type3);
    return rv;
}

int
drv_nsp_fp_stat_support_check(int unit, int stage_id, int op, int param0, void *mode) 
{
        
    switch (op) {
        case _DRV_FP_STAT_OP_COUNTER_MODE:
            return _drv_nsp_fp_counter_mode_support(unit, param0, mode);
        case _DRV_FP_STAT_OP_STAT_MODE:
            return _drv_nsp_fp_stat_mode_support(unit, stage_id, param0, mode);
    }
    return SOC_E_NONE;
}

int
drv_nsp_fp_action_conflict(int unit, int stage_id, 
        drv_field_action_t act1, drv_field_action_t act2)
{

    if (DRV_FIELD_STAGE_INGRESS != stage_id) {
        return SOC_E_PARAM;
    }

    switch (act1) {
        case drvFieldActionCopyToCpu:
        case drvFieldActionRedirect:
        case drvFieldActionDrop:
        case drvFieldActionMirrorIngress:
        case drvFieldActionRedirectPbmp:
        case drvFieldActionEgressMask: 
            _FIELD_ACTIONS_CONFLICT(drvFieldActionDrop);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionCopyToCpu);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionRedirect);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionMirrorIngress);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionEgressMask);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionRedirectPbmp);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionRpDrop);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionRpRedirectPort);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionRpMirrorIngress);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionRpCopyToCpu);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionYpDrop);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionYpRedirectPort);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionYpMirrorIngress);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionYpCopyToCpu);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionGpDrop);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionGpRedirectPort);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionGpMirrorIngress);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionGpCopyToCpu);
            break;
        case drvFieldActionRpCopyToCpu:
        case drvFieldActionRpRedirectPort:
        case drvFieldActionRpDrop:
        case drvFieldActionRpMirrorIngress:
        case drvFieldActionYpCopyToCpu:
        case drvFieldActionYpRedirectPort:
        case drvFieldActionYpDrop:
        case drvFieldActionYpMirrorIngress:
            /* Both Red actions and Yellow action are mutually exclusive */
            _FIELD_ACTIONS_CONFLICT(drvFieldActionRpDrop);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionRpRedirectPort);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionRpMirrorIngress);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionRpCopyToCpu);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionYpDrop);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionYpRedirectPort);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionYpMirrorIngress);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionYpCopyToCpu);
            break;
        case drvFieldActionGpCopyToCpu:
        case drvFieldActionGpRedirectPort:
        case drvFieldActionGpDrop:
        case drvFieldActionGpMirrorIngress:
            _FIELD_ACTIONS_CONFLICT(drvFieldActionGpDrop);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionGpRedirectPort);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionGpMirrorIngress);
            _FIELD_ACTIONS_CONFLICT(drvFieldActionGpCopyToCpu);
            break;
        case drvFieldActionInnerVlanNew:
            _FIELD_ACTIONS_CONFLICT(drvFieldActionOuterVlanNew);
            break;
        case drvFieldActionOuterVlanNew:
            _FIELD_ACTIONS_CONFLICT(drvFieldActionInnerVlanNew);
            break;
        case drvFieldActionCosQNew:
            _FIELD_ACTIONS_CONFLICT(drvFieldActionPrioIntNew);
            break;
        case drvFieldActionPrioIntNew:
            _FIELD_ACTIONS_CONFLICT(drvFieldActionPrioIntNew);
            break;
        default:
            break;
    }
    return 0;
}


