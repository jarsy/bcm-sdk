/*
 * $Id: trunk.c,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>

#define HARRIER_TRUNK_HASH_FIELD_MACDA_VALUE  0x1
#define HARRIER_TRUNK_HASH_FIELD_MACSA_VALUE  0x2
#define HARRIER_TRUNK_HASH_FIELD_IP_MACDA_VALUE  0x4
#define HARRIER_TRUNK_HASH_FIELD_IP_MACSA_VALUE  0x8

#define HARRIER_TRUNK_HASH_FIELD_VALID_VALUE  (DRV_TRUNK_HASH_FIELD_MACDA | \
                                               DRV_TRUNK_HASH_FIELD_MACSA | \
                                               DRV_TRUNK_HASH_FIELD_IP_MACDA | \
                                               DRV_TRUNK_HASH_FIELD_IP_MACSA)

static uint32  harrier_default_trunk_seed = \
    (HARRIER_TRUNK_HASH_FIELD_MACDA_VALUE | \
     HARRIER_TRUNK_HASH_FIELD_MACSA_VALUE);

/*
 *  Function : _drv_harrier_trunk_enable_set
 *
 *  Purpose :
 *      Enable trunk function.
 *
 *  Parameters :
 *      unit    :  RoboSwitch unit number.
 *      tid     :  trunk id. If tid == -1, globle trunk enable/disable
 *      enable  :  status of the trunk id.  
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 */
static int 
_drv_harrier_trunk_enable_set(int unit, int tid, uint32 enable)
{
    uint32  reg_value, temp;
    uint64  reg_value64;

    if (tid == -1) {
        /* Enable LOCAL TRUNK */ 
        SOC_IF_ERROR_RETURN(REG_READ_GLOBAL_TRUNK_CTLr
            (unit, &reg_value));

        if (enable) {
            temp = 1;
        } else {
            temp = 0;
        }
        SOC_IF_ERROR_RETURN(soc_GLOBAL_TRUNK_CTLr_field_set
            (unit, &reg_value, EN_TRUNK_LOCALf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_GLOBAL_TRUNK_CTLr
            (unit, &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_TRUNK_GRP_CTLr
            (unit, tid, (uint32 *)&reg_value64));

        if (enable) {
            temp = 1;
        } else {
            temp = 0;
        }
        SOC_IF_ERROR_RETURN(soc_TRUNK_GRP_CTLr_field_set
            (unit, (uint32 *)&reg_value64, EN_TRUNK_GRPf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_TRUNK_GRP_CTLr
            (unit, tid, (uint32 *)&reg_value64));
    }

    return SOC_E_NONE;
}

/*
 *  Function : _drv_harrier_trunk_enable_get
 *
 *  Purpose :
 *      Get the status of trunk function.
 *
 *  Parameters :
 *      unit    :  RoboSwitch unit number.
 *      tid     :  trunk id. If tid == -1, globle trunk enable/disable
 *      enable  :  status of the trunk id.  
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 */
static int 
_drv_harrier_trunk_enable_get(int unit, int tid, uint32 *enable)
{
    uint32  reg_value, temp;
    uint64  reg_value64;

    if (tid == -1) { 
        /* Get LOCAL TRUNK*/
        SOC_IF_ERROR_RETURN(REG_READ_GLOBAL_TRUNK_CTLr
            (unit, &reg_value));

        temp = 0;
        SOC_IF_ERROR_RETURN(soc_GLOBAL_TRUNK_CTLr_field_get
            (unit, &reg_value, EN_TRUNK_LOCALf, &temp));
        if (temp) {
            *enable = TRUE;
        } else {
            *enable = FALSE;
        }
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_TRUNK_GRP_CTLr
            (unit, tid, (uint32 *)&reg_value64));

        temp = 0;
        SOC_IF_ERROR_RETURN(soc_TRUNK_GRP_CTLr_field_get
            (unit, (uint32 *)&reg_value64, EN_TRUNK_GRPf, &temp));
        if (temp) {
            *enable = TRUE;
        } else {
            *enable = FALSE;
        }
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_trunk_set
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
drv_harrier_trunk_set(int unit, int tid, soc_pbmp_t bmp, 
    uint32 flag, uint32 hash_op)
{
    uint32  enable, temp, trunk_prop;
    uint64  reg_value64;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_harrier_trunk_set: \
                         unit = %d, trunk id = %d, bmp = 0x%x 0x%x, flag = 0x%x, hash_op = 0x%x\n"),
              unit, tid, SOC_PBMP_WORD_GET(bmp, 0), SOC_PBMP_WORD_GET(bmp, 1), 
              flag, hash_op));

    /* handle default trunk hash seed */
    if (flag & DRV_TRUNK_FLAG_HASH_DEFAULT) {
        temp = 0;
        if (hash_op & DRV_TRUNK_HASH_FIELD_MACDA) {  /* DA */
            temp |= HARRIER_TRUNK_HASH_FIELD_MACDA_VALUE;
        }
        if (hash_op & DRV_TRUNK_HASH_FIELD_MACSA) {  /* SA */
            temp |= HARRIER_TRUNK_HASH_FIELD_MACSA_VALUE;
        }
        if (hash_op & DRV_TRUNK_HASH_FIELD_IP_MACDA) {  /* IP DA */
            temp |= HARRIER_TRUNK_HASH_FIELD_IP_MACDA_VALUE;
        }
        if (hash_op & DRV_TRUNK_HASH_FIELD_IP_MACSA) {  /* IP SA */
            temp |= HARRIER_TRUNK_HASH_FIELD_IP_MACSA_VALUE;
        }

        if (temp == 0) {
            return SOC_E_PARAM;
        }
        harrier_default_trunk_seed = temp;

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
            _drv_harrier_trunk_enable_get(unit, -1, &enable));
        if (!enable) {
            SOC_IF_ERROR_RETURN(
                _drv_harrier_trunk_enable_set(unit, -1, TRUE));
        }    
        SOC_IF_ERROR_RETURN(
            _drv_harrier_trunk_enable_set(unit, tid, TRUE));
    } else if (flag & DRV_TRUNK_FLAG_DISABLE) {
        SOC_IF_ERROR_RETURN(
            _drv_harrier_trunk_enable_set(unit, tid, FALSE));
    }

    if (flag & DRV_TRUNK_FLAG_BITMAP) {
        SOC_IF_ERROR_RETURN(REG_READ_TRUNK_GRP_CTLr
            (unit, tid, (uint32 *)&reg_value64));

        temp = SOC_PBMP_WORD_GET(bmp, 0);
        SOC_IF_ERROR_RETURN(soc_TRUNK_GRP_CTLr_field_set
            (unit, (uint32 *)&reg_value64, TRUNK_PORT_MAPf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_TRUNK_GRP_CTLr
            (unit, tid, (uint32 *)&reg_value64));
    }    

    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_trunk_get
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
drv_harrier_trunk_get(int unit, int tid, soc_pbmp_t *bmp, 
    uint32 flag, uint32 *hash_op)
{
    uint32  enable, temp, trunk_prop;
    uint64  reg_value64;

    /* handle default trunk hash seed */
    if (flag & DRV_TRUNK_FLAG_HASH_DEFAULT) {
        *hash_op = 0;
        if (harrier_default_trunk_seed & 
                HARRIER_TRUNK_HASH_FIELD_MACDA_VALUE) {  /* DA */
            *hash_op |= DRV_TRUNK_HASH_FIELD_MACDA;
        }
        if (harrier_default_trunk_seed & 
                HARRIER_TRUNK_HASH_FIELD_MACSA_VALUE) {  /* SA */
            *hash_op |= DRV_TRUNK_HASH_FIELD_MACSA;
        }
        if (harrier_default_trunk_seed & 
                HARRIER_TRUNK_HASH_FIELD_IP_MACDA_VALUE) {  /* IP DA */
            *hash_op |= DRV_TRUNK_HASH_FIELD_IP_MACDA;
        }
        if (harrier_default_trunk_seed & 
                HARRIER_TRUNK_HASH_FIELD_IP_MACSA_VALUE) {  /* IP SA */
            *hash_op |= DRV_TRUNK_HASH_FIELD_IP_MACSA;
        }

        if (*hash_op == 0) {
            return SOC_E_INTERNAL;
        }

        LOG_INFO(BSL_LS_SOC_PORT, \
                 (BSL_META_U(unit, \
                             "drv_harrier_trunk_get: \
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
            _drv_harrier_trunk_enable_get(unit, -1, &enable));
        if (enable) {
            SOC_IF_ERROR_RETURN(
                _drv_harrier_trunk_enable_get(unit, tid, &enable));            
        }
        if (!enable) {
            SOC_PBMP_CLEAR(*bmp);
            return SOC_E_NONE;
        }
    }

    /* Get group member port bitmap */
    if (flag & DRV_TRUNK_FLAG_BITMAP) {
        SOC_IF_ERROR_RETURN(REG_READ_TRUNK_GRP_CTLr
            (unit, tid, (uint32 *)&reg_value64));
        SOC_IF_ERROR_RETURN(soc_TRUNK_GRP_CTLr_field_get
            (unit, (uint32 *)&reg_value64, TRUNK_PORT_MAPf, &temp));
        SOC_PBMP_WORD_SET(*bmp, 0, temp);
    }

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_harrier_trunk_get: \
                         unit = %d, trunk id = %d, flag = 0x%x, *bmp = 0x%x 0x%x\n"),
              unit, tid, flag, SOC_PBMP_WORD_GET(*bmp, 0), SOC_PBMP_WORD_GET(*bmp, 1)));

    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_trunk_hash_field_add
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
 */
int 
drv_harrier_trunk_hash_field_add(int unit, uint32 field_type)
{
    uint32  reg_value, temp;

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_harrier_trunk_hash_field_add: unit = %d, field type = 0x%x\n"),
              unit, field_type));

    /* check the valid trunk hash field types */
    if (field_type & ~HARRIER_TRUNK_HASH_FIELD_VALID_VALUE) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "drv_harrier_trunk_hash_field_add: hash type = 0x%x, is invalid!\n"),
                  field_type));
        return SOC_E_UNAVAIL;
    }

    SOC_IF_ERROR_RETURN(REG_READ_GLOBAL_TRUNK_CTLr
        (unit, &reg_value));

    temp = 0;
    if (field_type & DRV_TRUNK_HASH_FIELD_MACDA) {  /* DA */
        temp |= HARRIER_TRUNK_HASH_FIELD_MACDA_VALUE;
    }
    if (field_type & DRV_TRUNK_HASH_FIELD_MACSA) {  /* SA */
        temp |= HARRIER_TRUNK_HASH_FIELD_MACSA_VALUE;
    }
    if (field_type & DRV_TRUNK_HASH_FIELD_IP_MACDA) {  /* IP DA */
        temp |= HARRIER_TRUNK_HASH_FIELD_IP_MACDA_VALUE;
    }
    if (field_type & DRV_TRUNK_HASH_FIELD_IP_MACSA) {  /* IP SA */
        temp |= HARRIER_TRUNK_HASH_FIELD_IP_MACSA_VALUE;
    }

    if (!temp) {
        temp = harrier_default_trunk_seed;
    }
            
    SOC_IF_ERROR_RETURN(soc_GLOBAL_TRUNK_CTLr_field_set
        (unit, &reg_value, TRUNK_SEEDf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_GLOBAL_TRUNK_CTLr
        (unit, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_trunk_hash_field_remove
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
 */
int 
drv_harrier_trunk_hash_field_remove(int unit, uint32 field_type)
{
    uint32  reg_value, temp;

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_harrier_trunk_hash_field_remove: unit = %d, field type = 0x%x\n"),
              unit, field_type));

    SOC_IF_ERROR_RETURN(REG_READ_GLOBAL_TRUNK_CTLr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_GLOBAL_TRUNK_CTLr_field_get
        (unit, &reg_value, TRUNK_SEEDf, &temp));
     
    if (field_type & DRV_TRUNK_HASH_FIELD_MACDA) {  /* DA */
        temp &= ~HARRIER_TRUNK_HASH_FIELD_MACDA_VALUE;
    }
    if (field_type & DRV_TRUNK_HASH_FIELD_MACSA) {  /* SA */
        temp &= ~HARRIER_TRUNK_HASH_FIELD_MACSA_VALUE;
    }
    if (field_type & DRV_TRUNK_HASH_FIELD_IP_MACDA) {  /* IP DA */
        temp &= ~HARRIER_TRUNK_HASH_FIELD_IP_MACDA_VALUE;
    }
    if (field_type & DRV_TRUNK_HASH_FIELD_IP_MACSA) {  /* IP SA */
        temp &= ~HARRIER_TRUNK_HASH_FIELD_IP_MACSA_VALUE;
    }

    if (!temp) {
        temp = harrier_default_trunk_seed;
    }

    SOC_IF_ERROR_RETURN(soc_GLOBAL_TRUNK_CTLr_field_set
        (unit, &reg_value, TRUNK_SEEDf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_GLOBAL_TRUNK_CTLr
        (unit, &reg_value));

    return SOC_E_NONE;
}
