/*
 * $Id: $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>

#define DINO16_TRUNK_HASH_FIELD_MACDA_VALUE    1
#define DINO16_TRUNK_HASH_FIELD_MACSA_VALUE    2
#define DINO16_TRUNK_HASH_FIELD_MACDASA_VALUE  0

/* trunk seed in dino16 is named as trunk hash */
static uint32 dino16_default_trunk_seed = DINO16_TRUNK_HASH_FIELD_MACDASA_VALUE;

/*
 *  Function : _drv_dino16_trunk_enable_set
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
_drv_dino16_trunk_enable_set(int unit, uint32 enable)
{
    uint32  reg_value, temp;

    /* Enable LOCAL TRUNK */ /* should move to somewhere to initialize it */
    SOC_IF_ERROR_RETURN(REG_READ_GLOBAL_TRUNK_CTLr
        (unit, &reg_value));

    if (enable) {
        temp = 1;
    } else {
        temp = 0;
    }
    soc_GLOBAL_TRUNK_CTLr_field_set(unit, &reg_value, 
        EN_TRUNK_LOCALf, &temp);
    SOC_IF_ERROR_RETURN(REG_WRITE_GLOBAL_TRUNK_CTLr
        (unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : _drv_dino16_trunk_enable_get
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
_drv_dino16_trunk_enable_get(int unit, int tid, uint32 *enable)
{
    uint32  reg_value, temp, bmp;

    SOC_IF_ERROR_RETURN(REG_READ_GLOBAL_TRUNK_CTLr
        (unit, &reg_value));

    temp = 0;
    soc_GLOBAL_TRUNK_CTLr_field_get(unit, &reg_value, 
        EN_TRUNK_LOCALf, &temp);

    if (temp) {
        SOC_IF_ERROR_RETURN(REG_READ_TRUNK_GRP_CTLr
            (unit, 0, &reg_value));

        bmp = 0;
        soc_TRUNK_GRP_CTLr_field_get(unit, &reg_value, 
            TRUNK_PORT_MAP0f, &bmp);

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
 *  Function : drv_dino16_trunk_set
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
drv_dino16_trunk_set(int unit, int tid, soc_pbmp_t bmp, 
    uint32 flag, uint32 hash_op)
{
    uint32  reg_value, temp;
    uint32  bmp_value = 0, trunk_prop;

    bmp_value = SOC_PBMP_WORD_GET(bmp, 0);
    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_dino16_trunk_set: \
                         unit = %d, trunk id = %d, bmp = %x, flag = 0x%x, hash_op = 0x%x\n"),
              unit, tid, bmp_value, flag, hash_op));

    /* handle default trunk hash seed */
    if (flag & DRV_TRUNK_FLAG_HASH_DEFAULT) {
        if (hash_op & DRV_TRUNK_HASH_FIELD_MACDA) {
            if (hash_op & DRV_TRUNK_HASH_FIELD_MACSA) {
                dino16_default_trunk_seed = 
                    DINO16_TRUNK_HASH_FIELD_MACDASA_VALUE;
            } else {
                dino16_default_trunk_seed = 
                    DINO16_TRUNK_HASH_FIELD_MACDA_VALUE;
            }
        } else if (hash_op & DRV_TRUNK_HASH_FIELD_MACSA) {
            dino16_default_trunk_seed = 
                DINO16_TRUNK_HASH_FIELD_MACSA_VALUE;
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
            _drv_dino16_trunk_enable_set(unit, TRUE));
    } else if (flag & DRV_TRUNK_FLAG_DISABLE) {
        SOC_IF_ERROR_RETURN(
            _drv_dino16_trunk_enable_set(unit, FALSE));
    }

    if (flag & DRV_TRUNK_FLAG_BITMAP) {   
        SOC_IF_ERROR_RETURN(REG_READ_TRUNK_GRP_CTLr
            (unit, tid, &reg_value));

        if (bmp_value & ~(SOC_PBMP_WORD_GET(PBMP_PORT_ALL(unit), 0))) {
            return SOC_E_PARAM;
        }
        temp = bmp_value;

        soc_TRUNK_GRP_CTLr_field_set(unit, &reg_value, 
            TRUNK_PORT_MAP0f, &temp);
        SOC_IF_ERROR_RETURN(REG_WRITE_TRUNK_GRP_CTLr
            (unit, tid, &reg_value));
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_dino16_trunk_get
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
drv_dino16_trunk_get(int unit, int tid, soc_pbmp_t *bmp, 
    uint32 flag, uint32 *hash_op)
{
    uint32  reg_value, temp = 0, enable;
    uint32  trunk_prop;

    /* handle default trunk hash seed */
    if (flag & DRV_TRUNK_FLAG_HASH_DEFAULT) {
        switch (dino16_default_trunk_seed) {
            case DINO16_TRUNK_HASH_FIELD_MACDASA_VALUE:
                *hash_op = DRV_TRUNK_HASH_FIELD_MACDA | 
                    DRV_TRUNK_HASH_FIELD_MACSA;
                break;
            case DINO16_TRUNK_HASH_FIELD_MACDA_VALUE:
                *hash_op = DRV_TRUNK_HASH_FIELD_MACDA;
                break;
            case DINO16_TRUNK_HASH_FIELD_MACSA_VALUE:
                *hash_op = DRV_TRUNK_HASH_FIELD_MACSA;
                break;
            default:
                return SOC_E_INTERNAL;
        }
        LOG_INFO(BSL_LS_SOC_PORT, \
                 (BSL_META_U(unit, \
                             "drv_dino16_trunk_get: \
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
            _drv_dino16_trunk_enable_get(unit, tid, &enable));
        if (!enable) {
            SOC_PBMP_CLEAR(*bmp);
            return SOC_E_NONE;
        }
    }
    
    /* Get group member port bitmap */
    if (flag & DRV_TRUNK_FLAG_BITMAP) {
        SOC_IF_ERROR_RETURN(REG_READ_TRUNK_GRP_CTLr
            (unit, tid, &reg_value));

        soc_TRUNK_GRP_CTLr_field_get(unit, &reg_value, 
            TRUNK_PORT_MAP0f, &temp);
        SOC_PBMP_WORD_SET(*bmp, 0, temp);
    }
	
    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_dino16_trunk_get: \
                         unit = %d, trunk id = %d, flag = 0x%x, *bmp = 0x%x\n"),
              unit, tid, flag, SOC_PBMP_WORD_GET(*bmp, 0)));

    return SOC_E_NONE;
}

/*
 *  Function : drv_dino16_trunk_hash_field_add
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
 *      1. For the hash type field in dino16 family allowed 3 hash types only.
 *          (ie. MAC_DASA=0; MAC_DA=1; MAC_SA=2), here the final type value 
 *          to set to register will be :
 *          - no change : when add new hash is the same with current hash.
 *          - MAC_SADA : add to DASA.
 *          - MAC_DA : add DA. (no matter what current type is)
 *          - MAC_SA : add SA. (no matter what current type is)
 */
int 
drv_dino16_trunk_hash_field_add(int unit, uint32 field_type)
{
    uint32  reg_value, temp = 0;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_dino16_trunk_hash_field_add: unit = %d, field type = 0x%x\n"),
                 unit, field_type));

    SOC_IF_ERROR_RETURN(REG_READ_GLOBAL_TRUNK_CTLr
        (unit, &reg_value));
    soc_GLOBAL_TRUNK_CTLr_field_get(unit, &reg_value, 
        TRUNK_SEEDf, &temp);

    if ((field_type & DRV_TRUNK_HASH_FIELD_MACDA) && 
        (field_type & DRV_TRUNK_HASH_FIELD_MACSA)) {
        if (temp == DINO16_TRUNK_HASH_FIELD_MACDASA_VALUE) {
            return SOC_E_NONE;
        } else {
            temp = DINO16_TRUNK_HASH_FIELD_MACDASA_VALUE;
        }
    } else if (field_type & DRV_TRUNK_HASH_FIELD_MACDA) {
        if (temp == DINO16_TRUNK_HASH_FIELD_MACDA_VALUE) {
            return SOC_E_NONE;
        } else {
            temp = DINO16_TRUNK_HASH_FIELD_MACDA_VALUE;
        }
    } else if (field_type & DRV_TRUNK_HASH_FIELD_MACSA) {
        if (temp == DINO16_TRUNK_HASH_FIELD_MACSA_VALUE) {
            return SOC_E_NONE;
        } else {
            temp = DINO16_TRUNK_HASH_FIELD_MACSA_VALUE;
        }
    } else {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "drv_dino16_trunk_hash_field_add: hash type = 0x%x, is invalid!\n"),
                  field_type));
        return SOC_E_UNAVAIL;
    }

    soc_GLOBAL_TRUNK_CTLr_field_set(unit, &reg_value, 
        TRUNK_SEEDf, &temp);
    SOC_IF_ERROR_RETURN(REG_WRITE_GLOBAL_TRUNK_CTLr
        (unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_dino16_trunk_hash_field_remove
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
 *      1. For the hash type field in dino16 family allowed 3 hash types only.
 *          (ie. MAC_DASA=0; MAC_DA=1; MAC_SA=2), here the final type value 
 *          to set to register will be :
 *          - MAC_DA : when remove DA+SA
 *          - MAC_SADA : when current is SA only and remove SA.
 *          - MAC_SADA : when current is DA only and remove DA.
 *          - MAC_SADA : when other hask key applying but no SA or DA.
 */
int 
drv_dino16_trunk_hash_field_remove(int unit, uint32 field_type)
{
    uint32  reg_value, temp = 0, current_hash;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_dino16_trunk_hash_field_remove: unit = %d, field type = 0x%x\n"),
                 unit, field_type));

    SOC_IF_ERROR_RETURN(REG_READ_GLOBAL_TRUNK_CTLr
        (unit, &reg_value));
    soc_GLOBAL_TRUNK_CTLr_field_get(unit, &reg_value, 
        TRUNK_SEEDf, &temp);

    /* hash key with 2 bits length but "3" is not a valid value : 
     *  - in such case, the hash key is treat as DASA.
     */
    current_hash = (temp == 3) ? DINO16_TRUNK_HASH_FIELD_MACDASA_VALUE : temp;
    
    if ((field_type & DRV_TRUNK_HASH_FIELD_MACDA) && 
        (field_type & DRV_TRUNK_HASH_FIELD_MACSA)) {
        temp = DINO16_TRUNK_HASH_FIELD_MACDA_VALUE;
    } else if (field_type & DRV_TRUNK_HASH_FIELD_MACDA) {
        if (current_hash == DINO16_TRUNK_HASH_FIELD_MACDA_VALUE) {
            temp = DINO16_TRUNK_HASH_FIELD_MACDASA_VALUE;
        }
    } else if (field_type & DRV_TRUNK_HASH_FIELD_MACSA) {
        if (current_hash == DINO16_TRUNK_HASH_FIELD_MACSA_VALUE) {
            temp = DINO16_TRUNK_HASH_FIELD_MACDASA_VALUE;
        }
    }

    soc_GLOBAL_TRUNK_CTLr_field_set(unit, &reg_value, 
        TRUNK_SEEDf, &temp);
    SOC_IF_ERROR_RETURN(REG_WRITE_GLOBAL_TRUNK_CTLr
        (unit, &reg_value));

    return SOC_E_NONE;
}

