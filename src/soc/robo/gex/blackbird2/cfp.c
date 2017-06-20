/*
 * $Id: cfp.c,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <assert.h>
#include <soc/types.h>
#include <soc/error.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/drv_if.h>
#include <soc/cfp.h>


#define CFP_53128_FWD_MODE_NO_CHANGE    0x0
#define CFP_53128_FWD_MODE_NEW_DEST    0x2
#define CFP_53128_FWD_MODE_ADD_DEST    0x3

#define CFP_53128_MATCH_L4SRCPORT       0x2
#define CFP_53128_MATCH_L4DESTPORT      0x1
#define CFP_53128_MATCH_L4PORTS         0x3


/* Flood and drop value for BCM53128 */
#define CFP_53128_DEST_FLOOD 0x1ff
#define CFP_53128_DEST_DROP 0x0

#define CFP_53128_SLICE_MAX_ID 2 /* only one format */


#define BCM53128_AUTO_VOIP_NUM  16

typedef struct drv_slice_info_s {
    int slice_id;
    SHR_BITDCL  qset[_SHR_BITDCLSIZE(DRV_CFP_QUAL_COUNT)];
    int slice_udf_free;
}drv_slice_info_t;

static drv_slice_info_t drv53128_slice_info[CFP_53128_SLICE_MAX_ID +1]; 


/* SLICEs */
static int s0_qset[] = { DRV_CFP_QUAL_L4_SRC,
                                 DRV_CFP_QUAL_INVALID};

static int s1_qset[] = { DRV_CFP_QUAL_L4_DST,
                                 DRV_CFP_QUAL_INVALID};

static int s2_qset[] = { DRV_CFP_QUAL_L4_SRC,
                                 DRV_CFP_QUAL_L4_DST,
                                 DRV_CFP_QUAL_INVALID};


/*
 * Function: _drv_blackbird2_cfp_read
 *
 * Purpose:
 *     Read the CFP raw data by ram type from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     ram_type - ram type (TCAM/METER/ACT/POLICY)
 *     index -entry index
 *     entry(OUT) -CFP entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv_blackbird2_cfp_read(int unit, uint32 ram_type, 
                         uint32 index, drv_cfp_entry_t *entry)
{
    int rv = SOC_E_NONE;
    uint32 reg_val = 0;

    assert(entry);

    if (index >= BCM53128_AUTO_VOIP_NUM) {
        rv = SOC_E_PARAM;
        return rv;
    }
    
    switch (ram_type) {
        case DRV_CFP_RAM_ACT:
            SOC_IF_ERROR_RETURN(
                REG_READ_TCP_UDP_ACTION_REGr(unit, index, &reg_val));
            entry->act_data[0] = reg_val;
            break;
        case DRV_CFP_RAM_TCAM:
            SOC_IF_ERROR_RETURN(
                REG_READ_TCP_UDP_KEY_REGr(unit, index, &reg_val));
            entry->tcam_data[0] = reg_val;
            break;
        default:
            rv = SOC_E_PARAM;
            return rv;
    }

    return rv;
}


/*
 * Function: _drv_blackbird2_cfp_write
 *
 * Purpose:
 *     Write the CFP raw data by ram type to chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     ram_type - ram type (TCAM/METER/ACT/POLICY)
 *     index -entry index
 *     entry -CFP entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 */
int
_drv_blackbird2_cfp_write(int unit, uint32 ram_type, 
                              uint32 index, drv_cfp_entry_t *entry)
{
    int rv = SOC_E_NONE;
    uint32 reg_val;

    assert(entry);

    if (index >= BCM53128_AUTO_VOIP_NUM) {
        rv = SOC_E_PARAM;
        return rv;
    }
    
    switch (ram_type) {
        case DRV_CFP_RAM_ACT:
            reg_val = entry->act_data[0];
            SOC_IF_ERROR_RETURN(
                REG_WRITE_TCP_UDP_ACTION_REGr(unit, index, &reg_val));
            break;
        case DRV_CFP_RAM_TCAM:
            reg_val = entry->tcam_data[0];
            SOC_IF_ERROR_RETURN(
                REG_WRITE_TCP_UDP_KEY_REGr(unit, index, &reg_val));
            break;
        default:
            rv = SOC_E_PARAM;
            return rv;
    }

    return rv;
}


/*
 * Function: drv_blackbird2_cfp_init
 *
 * Purpose:
 *     Initialize the CFP module. 
 *
 * Parameters:
 *     unit - BCM device number
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 */
int 
drv_blackbird2_cfp_init(int unit)
{
    int port, i, slice_id;
    pbmp_t pbm;
    uint32  slice[(DRV_CFP_QUAL_COUNT / 32) + 1];
    int     *qset;

    /* assigne a default value */
    qset = s0_qset;

    /* Enable CFP */
    pbm = PBMP_E_ALL(unit);
    _SHR_PBMP_PORT_REMOVE(pbm, CMIC_PORT(unit));
    PBMP_ITER(pbm, port) {
        DRV_CFP_CONTROL_SET(unit, 
            DRV_CFP_ENABLE, port, TRUE);
    }
    /* Clear TCAM Table */
    DRV_CFP_CONTROL_SET(unit, 
            DRV_CFP_TCAM_RESET, 0, 0);


    /* Slice information initialization */
    for (slice_id = 0; slice_id <= CFP_53128_SLICE_MAX_ID; slice_id++) {
        switch(slice_id) {
            case 0:
                qset = s0_qset;
                break;
            case 1:
                qset = s1_qset;
                break;
            case 2:
                qset = s2_qset;
                break;
        }
        for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
            slice[i] = 0;
        }
        i = 0;
        while (qset[i] != DRV_CFP_QUAL_INVALID) {
            slice[(qset[i]/32)] |= (0x1 << (qset[i] % 32));
            i++;
        }
        for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
            drv53128_slice_info[slice_id].qset[i] = slice[i];
        }

        drv53128_slice_info[slice_id].slice_id = slice_id;
        
    }
    
    return SOC_E_NONE;
}

/*
 * Function: drv_blackbird2_cfp_action_get
 *
 * Purpose:
 *     Get the CFP action type and parameters value from 
 *     the raw data of ACTION/POLICY ram.
 *
 * Parameters:
 *     unit - BCM device number
 *     action(IN/OUT) - driver action type
 *     entry -cfp entry
 *     act_param(OUT) - action paramter (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown action type
 *
 * Note:
 */
int
drv_blackbird2_cfp_action_get(int unit, uint32* action, 
            drv_cfp_entry_t* entry, uint32* act_param)
{
    int rv = SOC_E_NONE;
    uint32  fld_val = 0;

    assert(entry);

    switch(*action) {
        case DRV_CFP_ACT_OB_NONE:
        case DRV_CFP_ACT_OB_REDIRECT:
        case DRV_CFP_ACT_OB_APPEND:
        case DRV_CFP_ACT_OB_FLOOD:
        case DRV_CFP_ACT_OB_DROP:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val));
            switch(fld_val) {
                case CFP_53128_FWD_MODE_NO_CHANGE:
                    *action = DRV_CFP_ACT_OB_NONE;
                    break;
                case CFP_53128_FWD_MODE_NEW_DEST:
                    SOC_IF_ERROR_RETURN(
                        DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                        DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val));
                    if (fld_val == CFP_53128_DEST_DROP) {
                        *action = DRV_CFP_ACT_OB_DROP;
                    } else if (fld_val == CFP_53128_DEST_FLOOD) {
                        *action = DRV_CFP_ACT_OB_FLOOD;
                    } else {
                        *action = DRV_CFP_ACT_OB_REDIRECT;
                        *act_param = fld_val;
                    }
                    break;
                case CFP_53128_FWD_MODE_ADD_DEST:
                    *action = DRV_CFP_ACT_OB_APPEND;
                    SOC_IF_ERROR_RETURN(
                        DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                        DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val));
                    *act_param = fld_val;
                    break;
                default:
                    rv = SOC_E_INTERNAL;
            }
            break;
        case DRV_CFP_ACT_CHANGE_TC:
        case DRV_CFP_ACT_CHANGE_TC_CANCEL:
            SOC_IF_ERROR_RETURN(
                DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_TC, entry, &fld_val));
            if (fld_val == 0) {
                *action = DRV_CFP_ACT_CHANGE_TC_CANCEL; 
            } else {
                *action = DRV_CFP_ACT_CHANGE_TC;
                SOC_IF_ERROR_RETURN(
                    DRV_CFP_FIELD_GET(unit, DRV_CFP_RAM_ACT, 
                    DRV_CFP_FIELD_NEW_TC, entry, &fld_val));
                *act_param = fld_val;
            }
            break;
        default:
            rv = SOC_E_PARAM;
    }
    
    return rv;
}

/*
 * Function: drv_blackbird2_cfp_action_set
 *
 * Purpose:
 *     Set the CFP action type and parameters value to 
 *     the raw data of ACTION/POLICY ram.
 *
 * Parameters:
 *     unit - BCM device number
 *     action - driver action type
 *     entry(OUT) -cfp entry
 *     act_param - action paramter (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown action type
 *
 * Note:
 */
int
drv_blackbird2_cfp_action_set(int unit, uint32 action, 
            drv_cfp_entry_t* entry, uint32 act_param1, uint32 act_param2)
{
    int rv = SOC_E_NONE;
    uint32  fld_val;

    assert(entry);

    switch(action) {
        case DRV_CFP_ACT_CHANGE_TC:
            fld_val = 1;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_TC, entry, &fld_val));
            fld_val = act_param1;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_TC, entry, &fld_val));
            break;
        case DRV_CFP_ACT_CHANGE_TC_CANCEL:
            fld_val = 0;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_TC, entry, &fld_val));
            break;
        case DRV_CFP_ACT_OB_NONE:
            fld_val = CFP_53128_FWD_MODE_NO_CHANGE;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val));
            break;
        case DRV_CFP_ACT_OB_APPEND:
            fld_val = CFP_53128_FWD_MODE_ADD_DEST;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val));
            fld_val = act_param1;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val));
            break;
        case DRV_CFP_ACT_OB_DROP:
            fld_val = CFP_53128_FWD_MODE_NEW_DEST;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val));
            fld_val = CFP_53128_DEST_DROP;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val));
            break;
        case DRV_CFP_ACT_OB_FLOOD:
            fld_val = CFP_53128_FWD_MODE_NEW_DEST;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val));
            fld_val = CFP_53128_DEST_FLOOD;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val));
            break;
        case DRV_CFP_ACT_OB_REDIRECT:
            fld_val = CFP_53128_FWD_MODE_NEW_DEST;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_CHANGE_FWD_OB_EN, entry, &fld_val));
            fld_val = act_param1;
            SOC_IF_ERROR_RETURN(
                DRV_CFP_FIELD_SET(unit, DRV_CFP_RAM_ACT, 
                DRV_CFP_FIELD_NEW_FWD_OB, entry, &fld_val));
            break;
        default:
            rv = SOC_E_UNAVAIL;
    }
    
    return rv;
}

/*
 * Function: drv_blackbird2_cfp_control_get
 *
 * Purpose:
 *     Get the CFP control paramters.
 *
 * Parameters:
 *     unit - BCM device number
 *     control_type - CFP control type
 *     param1 -control paramter 1 (if need)
 *     param2(OUT) -control parameter 2 (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv_blackbird2_cfp_control_get(int unit, uint32 control_type, uint32 param1, 
            uint32 *param2)
{
    int rv = SOC_E_NONE;
    uint32  fld_val = 0, reg_val = 0;

    switch (control_type) {
        case DRV_CFP_ENABLE:
            /* Read SIP control register */
            SOC_IF_ERROR_RETURN(
                REG_READ_SIP_REGr(unit, &reg_val));
            
            /* Set  per PORT SIP enable */
            soc_SIP_REGr_field_get(unit, &reg_val, SIP_REGf, &fld_val);
            if ((fld_val & (0x1 << param1))  != 0) {
                *param2 = TRUE;
            } else {
                *param2 = FALSE;
            }
            break;
        case DRV_CFP_UDP_DEFAULT_ACTION_ENABLE:
            SOC_IF_ERROR_RETURN(
                REG_READ_SIP_REGr(unit, &reg_val));
            soc_SIP_REGr_field_get(unit, &reg_val, UDP_WCM_ENf, &fld_val);
            *param2 =(fld_val)? TRUE : FALSE;
            break;
        case DRV_CFP_UDP_DEFAULT_ACTION:
            SOC_IF_ERROR_RETURN(
                REG_READ_TCP_UDP_WILDCARD_ACTION_REGr(unit, &reg_val));
            /* Check action type */
            switch (param1) {
                case DRV_CFP_ACT_CHANGE_TC:
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_get(
                        unit, &reg_val, CHG_TCf, &fld_val);
                    if (fld_val == FALSE) {
                        /* Change TC action is not set */
                        rv = SOC_E_DISABLED;
                    }
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_get(
                        unit, &reg_val, TCf, &fld_val);
                    *param2 = fld_val;
                    break;
                case DRV_CFP_ACT_CHANGE_TC_CANCEL:
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_get(
                        unit, &reg_val, CHG_TCf, &fld_val);
                    *param2 = (fld_val)? FALSE : TRUE;
                    break;
                case DRV_CFP_ACT_OB_NONE:
                    fld_val = CFP_53128_FWD_MODE_NO_CHANGE;
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_get(
                        unit, &reg_val, FWD_MODf, &fld_val);
                    if (fld_val == CFP_53128_FWD_MODE_NO_CHANGE) {
                        *param2 = TRUE;
                    } else {
                        rv = SOC_E_PARAM;
                    }
                    break;
                case DRV_CFP_ACT_OB_APPEND:
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_get(
                        unit, &reg_val, FWD_MODf, &fld_val);
                    if (fld_val != CFP_53128_FWD_MODE_ADD_DEST) {
                        rv = SOC_E_DISABLED;
                        return rv;
                    }
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_get(
                        unit, &reg_val, DEST_MAPf, &fld_val);
                    *param2 = fld_val;
                    break;
                case DRV_CFP_ACT_OB_DROP:
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_get(
                        unit, &reg_val, FWD_MODf, &fld_val);
                    if (fld_val != CFP_53128_FWD_MODE_NEW_DEST) {
                        rv = SOC_E_DISABLED;
                    }
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_get(
                        unit, &reg_val, DEST_MAPf, &fld_val);
                    if (fld_val != CFP_53128_DEST_DROP) {
                        rv = SOC_E_PARAM;
                    } else {
                        *param2 = TRUE;
                    }
                    break;
                case DRV_CFP_ACT_OB_FLOOD:
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_get(
                        unit, &reg_val, FWD_MODf, &fld_val);
                    if (fld_val != CFP_53128_FWD_MODE_NEW_DEST) {
                        rv = SOC_E_DISABLED;
                    }
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_get(
                        unit, &reg_val, DEST_MAPf, &fld_val);
                    if (fld_val != CFP_53128_DEST_FLOOD) {
                        rv = SOC_E_PARAM;
                    } else {
                        *param2 = TRUE;
                    }
                    break;
                case DRV_CFP_ACT_OB_REDIRECT:
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_get(
                        unit, &reg_val, FWD_MODf, &fld_val);
                    if (fld_val != CFP_53128_FWD_MODE_NEW_DEST) {
                        rv = SOC_E_DISABLED;
                    }
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_get(
                        unit, &reg_val, DEST_MAPf, &fld_val);
                    *param2 = fld_val;
                    break;
                default:
                    rv = SOC_E_UNAVAIL;
            }
            break;
		case DRV_CFP_BYPASS_VLAN_CHECK:
			SOC_IF_ERROR_RETURN(
				REG_READ_CHIP_REVIDr(unit,&reg_val));
			if (reg_val == BCM53128_B0_REV_ID) {
				SOC_IF_ERROR_RETURN(
					REG_READ_SIP_REGr(unit,&reg_val));
				soc_SIP_REGr_field_get(unit,&reg_val,
					SIP_VLAN_BYPASS_ENf,&fld_val);
				*param2 = fld_val;
			} else {
				rv = SOC_E_UNAVAIL;
			}
			break;
        default:
            rv = SOC_E_UNAVAIL;
    }

    return rv;
}

/*
 * Function: drv_blackbird2_cfp_control_set
 *
 * Purpose:
 *     Set the CFP control paramters.
 *
 * Parameters:
 *     unit - BCM device number
 *     control_type - CFP control type
 *     param1 -control paramter 1 (if need)
 *     param2 -control parameter 2 (if need)
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv_blackbird2_cfp_control_set(int unit, uint32 control_type, uint32 param1, 
            uint32 param2)
{
    int rv = SOC_E_NONE, i;
    uint32  fld_val = 0, reg_val = 0;
    uint32 temp;

    switch (control_type) {
        case DRV_CFP_ENABLE:
            /* Read SIP control register */
            SOC_IF_ERROR_RETURN(
                REG_READ_SIP_REGr(unit, &reg_val));
            
            /* Set  per PORT SIP enable */
            soc_SIP_REGr_field_get(unit, &reg_val, SIP_REGf, &fld_val);
            temp = 0x1 << param1;
             if (param2) {
                fld_val |= temp;
            } else {
                fld_val &= ~temp;
            }
            soc_SIP_REGr_field_set(unit, &reg_val, SIP_REGf, &fld_val);
            
            /* Write SIP control register */
            SOC_IF_ERROR_RETURN(
                REG_WRITE_SIP_REGr(unit, &reg_val));
            break;
        case DRV_CFP_TCAM_RESET:
            /* Clear all the UDP_TCP Key registers */
            reg_val = 0;
            for (i = 0; i < BCM53128_AUTO_VOIP_NUM; i++) {
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_TCP_UDP_KEY_REGr(unit, i, &reg_val));
            }
            break;
        case DRV_CFP_UDP_DEFAULT_ACTION_ENABLE:
            SOC_IF_ERROR_RETURN(
                REG_READ_SIP_REGr(unit, &reg_val));
            temp = (param2)? 1 : 0;
            soc_SIP_REGr_field_set(unit, &reg_val, UDP_WCM_ENf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_SIP_REGr(unit, &reg_val));
            break;
        case DRV_CFP_UDP_DEFAULT_ACTION:
            SOC_IF_ERROR_RETURN(
                REG_READ_TCP_UDP_WILDCARD_ACTION_REGr(unit, &reg_val));
            /* Check action type */
            switch (param1) {
                case DRV_CFP_ACT_CHANGE_TC:
                    fld_val = 1;
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_set(
                        unit, &reg_val, CHG_TCf, &fld_val);
                    fld_val = param2;
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_set(
                        unit, &reg_val, TCf, &fld_val);
                    break;
                case DRV_CFP_ACT_CHANGE_TC_CANCEL:
                    fld_val = 0;
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_set(
                        unit, &reg_val, CHG_TCf, &fld_val);
                    break;
                case DRV_CFP_ACT_OB_NONE:
                    fld_val = CFP_53128_FWD_MODE_NO_CHANGE;
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_set(
                        unit, &reg_val, FWD_MODf, &fld_val);
                    break;
                case DRV_CFP_ACT_OB_APPEND:
                    fld_val = CFP_53128_FWD_MODE_ADD_DEST;
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_set(
                        unit, &reg_val, FWD_MODf, &fld_val);
                    fld_val = param2;
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_set(
                        unit, &reg_val, DEST_MAPf, &fld_val);
                    break;
                case DRV_CFP_ACT_OB_DROP:
                    fld_val = CFP_53128_FWD_MODE_NEW_DEST;
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_set(
                        unit, &reg_val, FWD_MODf, &fld_val);
                    fld_val = CFP_53128_DEST_DROP;
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_set(
                        unit, &reg_val, DEST_MAPf, &fld_val);
                    break;
                case DRV_CFP_ACT_OB_FLOOD:
                    fld_val = CFP_53128_FWD_MODE_NEW_DEST;
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_set(
                        unit, &reg_val, FWD_MODf, &fld_val);
                    fld_val = CFP_53128_DEST_FLOOD;
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_set(
                        unit, &reg_val, DEST_MAPf, &fld_val);
                    break;
                case DRV_CFP_ACT_OB_REDIRECT:
                    fld_val = CFP_53128_FWD_MODE_NEW_DEST;
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_set(
                        unit, &reg_val, FWD_MODf, &fld_val);
                    fld_val = param2;
                    soc_TCP_UDP_WILDCARD_ACTION_REGr_field_set(
                        unit, &reg_val, DEST_MAPf, &fld_val);
                    break;
                default:
                    rv = SOC_E_UNAVAIL;
            }
            SOC_IF_ERROR_RETURN(
                REG_WRITE_TCP_UDP_WILDCARD_ACTION_REGr(unit, &reg_val));
            break;
		case DRV_CFP_BYPASS_VLAN_CHECK:
			SOC_IF_ERROR_RETURN(
				REG_READ_CHIP_REVIDr(unit,&reg_val));
			if (reg_val == BCM53128_B0_REV_ID) {
				SOC_IF_ERROR_RETURN(
					REG_READ_SIP_REGr(unit,&reg_val));
				fld_val = param2;
				soc_SIP_REGr_field_set(unit,&reg_val,
					SIP_VLAN_BYPASS_ENf,&fld_val);
				SOC_IF_ERROR_RETURN(
					REG_WRITE_SIP_REGr(unit,&reg_val));
			} else {
				rv = SOC_E_UNAVAIL;
			}
			break;
        default:
            rv = SOC_E_UNAVAIL;
    }

    return rv;
}


/*
 * Function: drv_blackbird2_cfp_entry_read
 *
 * Purpose:
 *     Read the TCAM/ACTION/POLICY/METER raw data from chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     index - CFP entry index
 *     ram_type -TCAM, ACTION/POLICT and METER
 *     entry(OUT) - chip entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown ram type
 *
 */
int
drv_blackbird2_cfp_entry_read(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry)
{
    int rv = SOC_E_NONE;

    switch (ram_type) {
        case DRV_CFP_RAM_ALL:
            if ((rv = _drv_blackbird2_cfp_read(unit, DRV_CFP_RAM_TCAM, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_read : failed to read TCAM with index = 0x%x, rv = %d. \n"), 
                           index, rv));
                return rv;
            }
            if ( (rv = _drv_blackbird2_cfp_read(unit, DRV_CFP_RAM_ACT, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_read : failed to read action ram with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_TCAM:
            if ((rv = _drv_blackbird2_cfp_read(unit, DRV_CFP_RAM_TCAM, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_read : failed to read TCAM with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_ACT:
            if ((rv = _drv_blackbird2_cfp_read(unit, DRV_CFP_RAM_ACT, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_read : failed to read action ram with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv_blackbird2_cfp_entry_write
 *
 * Purpose:
 *     Write the TCAM/ACTION/POLICY/METER raw data to chip.
 *
 * Parameters:
 *     unit - BCM device number
 *     index - CFP entry index
 *     ram_type -TCAM, ACTION/POLICT and METER
 *     entry - chip entry raw data
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - unknown ram type
 *
 * Note:
 *     
 */
int
drv_blackbird2_cfp_entry_write(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry)
{
    int rv = SOC_E_NONE;

    switch (ram_type) {
        case DRV_CFP_RAM_ALL:
            if ((rv = _drv_blackbird2_cfp_write(unit, DRV_CFP_RAM_TCAM, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_write : failed to write TCAM with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            if ((rv = _drv_blackbird2_cfp_write(unit, DRV_CFP_RAM_ACT, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_write:failed to write action ram with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index;
            break;
        case DRV_CFP_RAM_TCAM:
             if ((rv = _drv_blackbird2_cfp_write(unit, DRV_CFP_RAM_TCAM, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_write : failed to write TCAM with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index; 
            break;
        case DRV_CFP_RAM_ACT:
             if ((rv = _drv_blackbird2_cfp_write(unit, DRV_CFP_RAM_ACT, index, entry)) 
                != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "drv_cfp_entry_write : failed to write ACT ram with index = 0x%x, rv = %d. \n"),
                           index, rv));
                return rv;
            }
            entry->id = index; 
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 * Function: drv_blackbird2_cfp_field_get
 *
 * Purpose:
 *     Get the field value from the CFP entry raw data.
 *
 * Parameters:
 *     unit - BCM device number
 *     mem_type - driver ram type (TCAM/Meter/Act/Policy)
 *     field_type -driver CFP field type
 *     entry -cfp entry
 *     fld_val(OUT) - field value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv_blackbird2_cfp_field_get(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val)
{
    int rv = SOC_E_NONE;
    uint32  fld_id = 0, reg_id = 0, reg_val = 0;

    assert(entry);
    assert(fld_val);

    if (mem_type == DRV_CFP_RAM_TCAM) {
        reg_id = INDEX(TCP_UDP_KEY_REGr);
        reg_val = entry->tcam_data[0];
        /* The match type should be check before entering */
        switch (field_type) {
            case DRV_CFP_FIELD_L4SRC:
            case DRV_CFP_FIELD_L4DST:
                fld_id = INDEX(TCP_UDP_PORTNUMf);
                break;
            default:
                return SOC_E_BADID;
        }
        
    } else if (mem_type == DRV_CFP_RAM_ACT) {
        reg_id = INDEX(TCP_UDP_ACTION_REGr);
        reg_val = entry->act_data[0];
        switch(field_type) {
            case DRV_CFP_FIELD_CHANGE_TC:
                fld_id = INDEX(CHG_TCf);
                break;
            case DRV_CFP_FIELD_NEW_TC:
                fld_id = INDEX(TCf);
                break;
            case DRV_CFP_FIELD_CHANGE_FWD_OB_EN:
                fld_id = INDEX(FWD_MODf);
                break;
            case DRV_CFP_FIELD_NEW_FWD_OB:
                fld_id = INDEX(DEST_MAPf);
                break;
            default:
                return SOC_E_BADID;
         }
    } else {
        rv = SOC_E_PARAM;
        return rv;
    }
    
    /* Get field value */
    DRV_REG_FIELD_GET(unit, reg_id, &reg_val, fld_id, fld_val);
    
    return rv;
}

/*
 * Function: drv_blackbird2_cfp_field_set
 *
 * Purpose:
 *     Set the field value to the CFP entry raw data.
 *
 * Parameters:
 *     unit - BCM device number
 *     mem_type - driver ram type (TCAM/Meter/Act/Policy)
 *     field_type -driver CFP field type
 *     entry(OUT) -cfp entry
 *     fld_val - field value
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_XXX
 *
 * Note:
 *     
 */
int
drv_blackbird2_cfp_field_set(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val)
{
    int rv = SOC_E_NONE;
    uint32  fld_id = 0, reg_id = 0, reg_val = 0, temp  = 0;

    assert(entry);
    assert(fld_val);

    if (mem_type == DRV_CFP_RAM_TCAM) {
        reg_id = INDEX(TCP_UDP_KEY_REGr);
        reg_val = entry->tcam_data[0];
        /* Set Match control type : src, dest or either */
        switch (field_type) {
            case DRV_CFP_FIELD_L4SRC:
                temp = CFP_53128_MATCH_L4SRCPORT;
                break;
            case DRV_CFP_FIELD_L4DST:
                temp = CFP_53128_MATCH_L4DESTPORT;
                break;
            default:
                return SOC_E_BADID;
        }
        /* The slice to filter the L4 source or destination ports */
        if (entry->slice_id == CFP_53128_SLICE_MAX_ID) {
            temp = CFP_53128_MATCH_L4PORTS;
        }
        fld_id = INDEX(CTRLf);
        DRV_REG_FIELD_SET(unit, reg_id, &reg_val, fld_id, &temp);

        /* Set the value of port number */
        fld_id = INDEX(TCP_UDP_PORTNUMf);
    } else if (mem_type == DRV_CFP_RAM_ACT) {
        reg_id = INDEX(TCP_UDP_ACTION_REGr);
        reg_val = entry->act_data[0];
        switch(field_type) {
            case DRV_CFP_FIELD_CHANGE_TC:
                fld_id = INDEX(CHG_TCf);
                break;
            case DRV_CFP_FIELD_NEW_TC:
                fld_id = INDEX(TCf);
                break;
            case DRV_CFP_FIELD_CHANGE_FWD_OB_EN:
                fld_id = INDEX(FWD_MODf);
                break;
            case DRV_CFP_FIELD_NEW_FWD_OB:
                fld_id = INDEX(DEST_MAPf);
                break;
            default:
                return SOC_E_BADID;
         }
    } else {
        rv = SOC_E_PARAM;
        return rv;
    }

    /* Set field value */
    DRV_REG_FIELD_SET(unit, reg_id, &reg_val, fld_id, fld_val);

    if (mem_type == DRV_CFP_RAM_TCAM) {
        entry->tcam_data[0] = reg_val;
    } else if (mem_type == DRV_CFP_RAM_ACT) {
        entry->act_data[0] = reg_val;
    }

    return rv;
}


/*
 * Function: drv_blackbird2_cfp_slice_id_select
 *
 * Purpose:
 *     According to this entry's fields to select which slice id used for this entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - driver cfp entry
 *     slice_id(OUT) - slice id for this entry
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_RESOURCE - Can't found suitable slice id for this entry.
 */
int
drv_blackbird2_cfp_slice_id_select(int unit, drv_cfp_entry_t *entry, uint32 *slice_id, uint32 flags)
{
    uint32 i, j;
    int match = TRUE;

    for (j=0; j <= CFP_53128_SLICE_MAX_ID; j++) {
        match = TRUE;
        for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
            if (entry->w[i] & ~drv53128_slice_info[j].qset[i]) {
                match = FALSE;
                break;
            }
        }
        if (match) {
            *slice_id = drv53128_slice_info[j].slice_id;
            return SOC_E_NONE;
        }
    }
    
    return SOC_E_RESOURCE;
    
}

/*
 * Function: drv_blackbird2_cfp_slice_to_qset
 *
 * Purpose:
 *     According to slice id used for this entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry(OUT) - driver cfp entry
 *     slice_id - slice id 
 *
 * Returns:
 *     SOC_E_NONE
 *     SOC_E_PARAM - un-support slice id.
 */
int
drv_blackbird2_cfp_slice_to_qset(int unit, uint32 slice_id, drv_cfp_entry_t *entry)
{
    uint32 i;

    if (slice_id > CFP_53128_SLICE_MAX_ID) {
        return SOC_E_PARAM;
    }
    
    for (i = 0; i < (DRV_CFP_QUAL_COUNT / 32) + 1; i++) {
        entry->w[i] = drv53128_slice_info[slice_id].qset[i];
    }
    return SOC_E_NONE;
    
}




