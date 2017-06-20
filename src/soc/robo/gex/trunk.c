/*
 * $Id: trunk.c,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>

#define GEX_TRUNK_HASH_FIELD_MACDA_VALUE  1
#define GEX_TRUNK_HASH_FIELD_MACSA_VALUE  2
#define GEX_TRUNK_HASH_FIELD_MACDASA_VALUE  0

/* trunk seed in gex is named as trunk hash */
static uint32  gex_default_trunk_seed = GEX_TRUNK_HASH_FIELD_MACDASA_VALUE;

#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
#define NORTHSTAR_TRUNK_HASH_FIELD_VID_MACDA_VALUE  1
#define NORTHSTAR_TRUNK_HASH_FIELD_VID_MACSA_VALUE  2
#define NORTHSTAR_TRUNK_HASH_FIELD_VID_MACDASA_VALUE  0

/* trunk seed in gex is named as trunk hash */
static uint32  northstar_default_trunk_seed = \
                                   NORTHSTAR_TRUNK_HASH_FIELD_VID_MACDASA_VALUE;
#endif /* BCM_NORTHSTAR_SUPPORT || BCM_NORTHSTARPLUS_SUPPORT */

/*
 *  Function : _drv_gex_trunk_enable_set
 *
 *  Purpose :
 *      Enable trunk function (global).
 *
 *  Parameters :
 *      unit    :  RoboSwitch unit number.
 *      enable  :  status of the trunk id (global).
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 */
static int 
_drv_gex_trunk_enable_set(int unit, uint32 enable)
{
    uint32  reg_value, temp;

    /* Enable LOCAL TRUNK */ /* should move to somewhere to initialize it */
    SOC_IF_ERROR_RETURN(REG_READ_MAC_TRUNK_CTLr
        (unit, &reg_value));

    if (enable) {
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(soc_MAC_TRUNK_CTLr_field_set
        (unit, &reg_value, EN_TRUNK_LOCALf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_MAC_TRUNK_CTLr
        (unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : _drv_gex_trunk_enable_get
 *
 *  Purpose :
 *      Get the status of the selected GE trunk.
 *
 *  Parameters :
 *      unit    :  RoboSwitch unit number.
 *      tid     :   trunk id.
 *      enable  :  status of the trunk id.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 */
static int 
_drv_gex_trunk_enable_get(int unit, int tid, uint32 *enable)
{
    uint32  reg_value, temp, bmp;

    SOC_IF_ERROR_RETURN(REG_READ_MAC_TRUNK_CTLr
        (unit, &reg_value));

    temp = 0;
    SOC_IF_ERROR_RETURN(soc_MAC_TRUNK_CTLr_field_get
        (unit, &reg_value, EN_TRUNK_LOCALf, &temp));

    if (temp) {
        SOC_IF_ERROR_RETURN(REG_READ_TRUNK_GRP_CTLr
            (unit, tid, &reg_value));

        bmp = 0;
        SOC_IF_ERROR_RETURN(soc_TRUNK_GRP_CTLr_field_get
            (unit, &reg_value, EN_TRUNK_GRPf, &bmp));

        if(bmp) {
            *enable = 1;
        } else {
            *enable = 0;
        }
    } else {
        *enable = 0;
    }

    return SOC_E_NONE;
}


/*
 *  Function : drv_gex_trunk_set
 *
 *  Purpose :
 *      Set the member ports to a trunk group.
 *
 *  Parameters :
 *      unit    :  RoboSwitch unit number.
 *      tid     :  trunk id.
 *      bmp     :  trunk member port bitmap.
 *      flag    :  trunk flag.
 *      hash_op :  trunk hash seed.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 */
int 
drv_gex_trunk_set(int unit, int tid, soc_pbmp_t bmp, 
    uint32 flag, uint32 hash_op)
{
    uint32  reg_value, temp, c_temp;
    uint32  bmp_value = 0, trunk_prop;
    int  port;

    bmp_value = SOC_PBMP_WORD_GET(bmp, 0);
    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_gex_trunk_set: \
                         unit = %d, trunk id = %d, bmp = %x, flag = 0x%x, hash_op = 0x%x\n"),
              unit, tid, bmp_value, flag, hash_op));

    /* handle default trunk hash seed */
    if (flag & DRV_TRUNK_FLAG_HASH_DEFAULT) {
        if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
            if ((hash_op & DRV_TRUNK_HASH_FIELD_VLANID) &&
                (hash_op & DRV_TRUNK_HASH_FIELD_MACDA)) {
                if (hash_op & DRV_TRUNK_HASH_FIELD_MACSA) {
                    northstar_default_trunk_seed = \
                        NORTHSTAR_TRUNK_HASH_FIELD_VID_MACDASA_VALUE;
                } else {
                    northstar_default_trunk_seed = \
                        NORTHSTAR_TRUNK_HASH_FIELD_VID_MACDA_VALUE;
                }
            } else if ((hash_op & DRV_TRUNK_HASH_FIELD_VLANID) &&
                (hash_op & DRV_TRUNK_HASH_FIELD_MACSA)) {
                northstar_default_trunk_seed = \
                    NORTHSTAR_TRUNK_HASH_FIELD_VID_MACSA_VALUE;
            }
#endif /* BCM_NORTHSTAR_SUPPORT || BCM_NORTHSTARPLUS_SUPPORT */
        } else {
            if (hash_op & DRV_TRUNK_HASH_FIELD_MACDA) {
                if (hash_op & DRV_TRUNK_HASH_FIELD_MACSA) {
                    gex_default_trunk_seed = GEX_TRUNK_HASH_FIELD_MACDASA_VALUE;
                } else {
                    gex_default_trunk_seed = GEX_TRUNK_HASH_FIELD_MACDA_VALUE;
                }
            } else if (hash_op & DRV_TRUNK_HASH_FIELD_MACSA) {
                gex_default_trunk_seed = GEX_TRUNK_HASH_FIELD_MACSA_VALUE;
            }
        }
        return SOC_E_NONE;
    }

    /* Check TRUNK MAX ID */
    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_TRUNK_NUM, &trunk_prop));
    if (tid > (trunk_prop - 1)) {
        return SOC_E_PARAM;
    }        

    if (flag & DRV_TRUNK_FLAG_ENABLE) {
        SOC_IF_ERROR_RETURN(
            _drv_gex_trunk_enable_set(unit, TRUE));
    } else if (flag & DRV_TRUNK_FLAG_DISABLE) {
        SOC_IF_ERROR_RETURN(
            _drv_gex_trunk_enable_set(unit, FALSE));
    }

    if (flag & DRV_TRUNK_FLAG_BITMAP) {   
        /* Check per trunk member port number */
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_TRUNK_MAX_PORT_NUM, &trunk_prop));

        /* check pbmp parameter (port count = 0 is allowed) */
        SOC_PBMP_COUNT(bmp, c_temp); 
        if (c_temp) {
            if (c_temp > trunk_prop) {
                return SOC_E_PARAM;
            }
        }
        
        /* check valid port bitmap */
        SOC_PBMP_ITER(bmp, port) {
            if (!SOC_PORT_VALID(unit, port)) {
                return SOC_E_PARAM;
            }
            
            if (IS_CPU_PORT(unit, port)) {
                return SOC_E_PARAM;
            }
        }

        SOC_IF_ERROR_RETURN(REG_READ_TRUNK_GRP_CTLr
            (unit, tid, &reg_value));

        temp = bmp_value;
        SOC_IF_ERROR_RETURN(soc_TRUNK_GRP_CTLr_field_set
            (unit, &reg_value, EN_TRUNK_GRPf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_TRUNK_GRP_CTLr
            (unit, tid, &reg_value));

    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_trunk_get
 *
 *  Purpose :
 *      Get the member ports to a trunk group.
 *
 *  Parameters :
 *      unit    :  RoboSwitch unit number.
 *      tid     :  trunk id.
 *      bmp     :  trunk member port bitmap.
 *      flag    :  trunk flag.
 *      hash_op :  trunk hash seed.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 */
int 
drv_gex_trunk_get(int unit, int tid, soc_pbmp_t *bmp, 
    uint32 flag, uint32 *hash_op)
{
    uint32  reg_value, temp, enable;
    uint32  trunk_prop;

    /* handle default trunk hash seed */
    if (flag & DRV_TRUNK_FLAG_HASH_DEFAULT) {
        if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
            switch (northstar_default_trunk_seed) {
                case NORTHSTAR_TRUNK_HASH_FIELD_VID_MACDASA_VALUE:
                    *hash_op = DRV_TRUNK_HASH_FIELD_VLANID | 
                        DRV_TRUNK_HASH_FIELD_MACDA |
                        DRV_TRUNK_HASH_FIELD_MACSA;
                    break;
                case NORTHSTAR_TRUNK_HASH_FIELD_VID_MACDA_VALUE:
                    *hash_op = DRV_TRUNK_HASH_FIELD_VLANID | 
                        DRV_TRUNK_HASH_FIELD_MACDA;
                    break;
                case NORTHSTAR_TRUNK_HASH_FIELD_VID_MACSA_VALUE:
                    *hash_op = DRV_TRUNK_HASH_FIELD_VLANID | 
                        DRV_TRUNK_HASH_FIELD_MACSA;
                    break;
                default:
                    return SOC_E_INTERNAL;
            }
#endif /* BCM_NORTHSTAR_SUPPORT || BCM_NORTHSTARPLUS_SUPPORT*/
        } else {
            switch (gex_default_trunk_seed) {
                case GEX_TRUNK_HASH_FIELD_MACDASA_VALUE:
                    *hash_op = DRV_TRUNK_HASH_FIELD_MACDA | 
                        DRV_TRUNK_HASH_FIELD_MACSA;
                    break;
                case GEX_TRUNK_HASH_FIELD_MACDA_VALUE:
                    *hash_op = DRV_TRUNK_HASH_FIELD_MACDA;
                    break;
                case GEX_TRUNK_HASH_FIELD_MACSA_VALUE:
                    *hash_op = DRV_TRUNK_HASH_FIELD_MACSA;
                    break;
                default:
                    return SOC_E_INTERNAL;
            }
        }
        LOG_INFO(BSL_LS_SOC_PORT, \
                 (BSL_META_U(unit, \
                             "drv_gex_trunk_get: \
                             unit = %d, trunk id = %d, flag = 0x%x, *hash_op = 0x%x\n"),
                  unit, tid, flag, *hash_op));

        return SOC_E_NONE;
    }

    /* Check TRUNK MAX ID */
    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_TRUNK_NUM, &trunk_prop));
    if (tid > (trunk_prop - 1)) {
        return SOC_E_PARAM;
    }

    if (flag & DRV_TRUNK_FLAG_ENABLE) {
        SOC_IF_ERROR_RETURN(
            _drv_gex_trunk_enable_get(unit, tid, &enable));
        if (!enable) {
            SOC_PBMP_CLEAR(*bmp);
            return SOC_E_NONE;
        }
    }
    
    /* Get group member port bitmap */
    if (flag & DRV_TRUNK_FLAG_BITMAP) {
        SOC_IF_ERROR_RETURN(REG_READ_TRUNK_GRP_CTLr
            (unit, tid, &reg_value));
        SOC_IF_ERROR_RETURN(soc_TRUNK_GRP_CTLr_field_get
            (unit, &reg_value, EN_TRUNK_GRPf, &temp));
        SOC_PBMP_WORD_SET(*bmp, 0, temp);
    }
	
    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_gex_trunk_get: unit = %d, trunk id = %d, flag = 0x%x, *bmp = 0x%x\n"),
              unit, tid, flag, SOC_PBMP_WORD_GET(*bmp, 0)));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_trunk_hash_field_add
 *
 *  Purpose :
 *      Add trunk hash field type.
 *
 *  Parameters :
 *      unit      :  RoboSwitch unit number.
 *      field_type:  trunk hash field type to be add.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. For the hash type field in gex family allowed 3 hash types only.
 *          (ie. MAC_DASA=0; MAC_DA=1; MAC_SA=2), here the final type value 
 *          to set to register will be :
 *          - no change : when add new hash is the same with current hash.
 *          - MAC_SADA : add to DASA.
 *          - MAC_DA : add DA. (no matter what current type is)
 *          - MAC_SA : add SA. (no matter what current type is)
 */
int 
drv_gex_trunk_hash_field_add(int unit, uint32 field_type)
{
    uint32	reg_value, temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_gex_trunk_hash_field_add: unit = %d, field type = 0x%x\n"),
                 unit, field_type));

    SOC_IF_ERROR_RETURN(REG_READ_MAC_TRUNK_CTLr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_MAC_TRUNK_CTLr_field_get
        (unit, &reg_value, HASH_SELf, &temp));

    if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
        if ((field_type & DRV_TRUNK_HASH_FIELD_VLANID) && 
            (field_type & DRV_TRUNK_HASH_FIELD_MACDA) && 
            (field_type & DRV_TRUNK_HASH_FIELD_MACSA)) {
            if (temp == NORTHSTAR_TRUNK_HASH_FIELD_VID_MACDASA_VALUE) {
                return SOC_E_NONE;
            } else {
                temp = NORTHSTAR_TRUNK_HASH_FIELD_VID_MACDASA_VALUE;
            }
        } else if ((field_type & DRV_TRUNK_HASH_FIELD_VLANID) &&
            (field_type & DRV_TRUNK_HASH_FIELD_MACDA)) {
            if (temp == NORTHSTAR_TRUNK_HASH_FIELD_VID_MACDA_VALUE) {
                return SOC_E_NONE;
            } else {
                temp = NORTHSTAR_TRUNK_HASH_FIELD_VID_MACDA_VALUE;
            }
        } else if ((field_type & DRV_TRUNK_HASH_FIELD_VLANID) &&
            (field_type & DRV_TRUNK_HASH_FIELD_MACSA)) {
            if (temp == NORTHSTAR_TRUNK_HASH_FIELD_VID_MACSA_VALUE) {
                return SOC_E_NONE;
            } else {
                temp = NORTHSTAR_TRUNK_HASH_FIELD_VID_MACSA_VALUE;
            }
        } else {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "drv_gex_trunk_hash_field_add: hash type = 0x%x, is invalid!\n"),
                      field_type));
            return SOC_E_UNAVAIL;
        }
#endif /* BCM_NORTHSTAR_SUPPORT || BCM_NORTHSTARPLUS_SUPPORT */
    } else {
        if ((field_type & DRV_TRUNK_HASH_FIELD_MACDA) && 
            (field_type & DRV_TRUNK_HASH_FIELD_MACSA)) {
            if (temp == GEX_TRUNK_HASH_FIELD_MACDASA_VALUE) {
                return SOC_E_NONE;
            } else {
                temp = GEX_TRUNK_HASH_FIELD_MACDASA_VALUE;
            }
        } else if (field_type & DRV_TRUNK_HASH_FIELD_MACDA) {
            if (temp == GEX_TRUNK_HASH_FIELD_MACDA_VALUE) {
                return SOC_E_NONE;
            } else {
                temp = GEX_TRUNK_HASH_FIELD_MACDA_VALUE;
            }
        } else if (field_type & DRV_TRUNK_HASH_FIELD_MACSA) {
            if (temp == GEX_TRUNK_HASH_FIELD_MACSA_VALUE) {
                return SOC_E_NONE;
            } else {
                temp = GEX_TRUNK_HASH_FIELD_MACSA_VALUE;
            }
        } else {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "drv_gex_trunk_hash_field_add: hash type = 0x%x, is invalid!\n"),
                      field_type));
            return SOC_E_UNAVAIL;
        }
    }

    SOC_IF_ERROR_RETURN(soc_MAC_TRUNK_CTLr_field_set
        (unit, &reg_value, HASH_SELf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_MAC_TRUNK_CTLr
        (unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_trunk_hash_field_remove
 *
 *  Purpose :
 *      Remove trunk hash field type.
 *
 *  Parameters :
 *      unit      :  RoboSwitch unit number.
 *      field_type:  trunk hash field type to be removed.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. For the hash type field in gex family allowed 3 hash types only.
 *          (ie. MAC_DASA=0; MAC_DA=1; MAC_SA=2), here the final type value 
 *          to set to register will be :
 *          - MAC_DA : when remove DA+SA
 *          - MAC_SADA : when current is SA only and remove SA.
 *          - MAC_SADA : when current is DA only and remove DA.
 *          - MAC_SADA : when other hask key applying but no SA or DA.
 */
int 
drv_gex_trunk_hash_field_remove(int unit, uint32 field_type)
{
    uint32	reg_value, temp, current_hash;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_gex_trunk_hash_field_remove: unit = %d, field type = 0x%x\n"),
                 unit, field_type));

    SOC_IF_ERROR_RETURN(REG_READ_MAC_TRUNK_CTLr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_MAC_TRUNK_CTLr_field_get
        (unit, &reg_value, HASH_SELf, &temp));

    /* hash key with 2 bits length but "3" is not a valid value : 
     *  - in such case, the hash key is treat as DASA.
     */
    if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
        current_hash = (temp == 3) ? \
            NORTHSTAR_TRUNK_HASH_FIELD_VID_MACDASA_VALUE : temp;
        
        if ((field_type & DRV_TRUNK_HASH_FIELD_VLANID) && 
            (field_type & DRV_TRUNK_HASH_FIELD_MACDA) &&
            (field_type & DRV_TRUNK_HASH_FIELD_MACSA)) {
            temp = NORTHSTAR_TRUNK_HASH_FIELD_VID_MACDA_VALUE;
        } else if ((field_type & DRV_TRUNK_HASH_FIELD_VLANID) &&
            (field_type & DRV_TRUNK_HASH_FIELD_MACDA)) {
            if (current_hash == NORTHSTAR_TRUNK_HASH_FIELD_VID_MACDA_VALUE) {
                temp = NORTHSTAR_TRUNK_HASH_FIELD_VID_MACDASA_VALUE;
            }
        } else if ((field_type & DRV_TRUNK_HASH_FIELD_VLANID) &&
            (field_type & DRV_TRUNK_HASH_FIELD_MACSA)) {
            if (current_hash == NORTHSTAR_TRUNK_HASH_FIELD_VID_MACSA_VALUE) {
                temp = NORTHSTAR_TRUNK_HASH_FIELD_VID_MACDASA_VALUE;
            }
        }
#endif /* BCM_NORTHSTAR_SUPPORT || BCM_NORTHSTARPLUS_SUPPORT */
    } else {
        current_hash = (temp == 3) ? GEX_TRUNK_HASH_FIELD_MACDASA_VALUE : temp;
        
        if ((field_type & DRV_TRUNK_HASH_FIELD_MACDA) && 
            (field_type & DRV_TRUNK_HASH_FIELD_MACSA)) {
            temp = GEX_TRUNK_HASH_FIELD_MACDA_VALUE;
        } else if (field_type & DRV_TRUNK_HASH_FIELD_MACDA) {
            if (current_hash == GEX_TRUNK_HASH_FIELD_MACDA_VALUE) {
                temp = GEX_TRUNK_HASH_FIELD_MACDASA_VALUE;
            }
        } else if (field_type & DRV_TRUNK_HASH_FIELD_MACSA) {
            if (current_hash == GEX_TRUNK_HASH_FIELD_MACSA_VALUE) {
                temp = GEX_TRUNK_HASH_FIELD_MACDASA_VALUE;
            }
        }
    }

    SOC_IF_ERROR_RETURN(soc_MAC_TRUNK_CTLr_field_set
        (unit, &reg_value, HASH_SELf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_MAC_TRUNK_CTLr
        (unit, &reg_value));

    return SOC_E_NONE;
}

