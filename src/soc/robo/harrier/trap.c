/*
 * $Id: trap.c,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

 #include <soc/drv.h>
 #include <soc/debug.h>

/* the filed value in Harrier IGMP/MLD related register 
 *  - GMNGCFGr.IGMP_MLD_CHKf (0x2 is invalid)
 */
#define HARRIER_REG_IGMP_MLD_DISABLE   0x0
#define HARRIER_REG_IGMP_MLD_TRAP      0x1
#define HARRIER_REG_IGMP_MLD_SNOOP     0x3

/*
 *  Function : drv_harrier_trap_set
 *
 *  Purpose :
 *      Set the trap frame type to CPU.
 *
 *  Parameters :
 *      unit      :  unit id.
 *      bmp       :  port bitmap.
 *      trap_mask :  the mask of trap type.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
int 
drv_harrier_trap_set(int unit, soc_pbmp_t bmp, uint32 trap_mask)
{
    uint32  reg_value, temp;
    soc_port_t port = 0;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_harrier_trap_set: unit = %d, trap mask = 0x%x\n"), unit, trap_mask));

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
        (unit, &reg_value));
    if (trap_mask & DRV_SWITCH_TRAP_IGMP) {
            temp = HARRIER_REG_IGMP_MLD_TRAP;
    } else {
        temp = 0;
        if (trap_mask & DRV_SWITCH_TRAP_MLD) {
            temp = HARRIER_REG_IGMP_MLD_TRAP;
        }
    }

    /* temp =  1b'01: IGMP/MLD forward to IMP.
               1b'11: IGMP/MLD forward to original pbmp and IMP. */
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
        (unit, &reg_value, IGMP_MLD_CHKf, &temp));

    if (trap_mask & DRV_SWITCH_TRAP_BPDU1) {    
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
        (unit, &reg_value, RXBPDU_ENf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr
        (unit, &reg_value));
    
    if (trap_mask & DRV_SWITCH_TRAP_8021X) {
        temp = 1;
    } else {
        temp = 0;
    }

    PBMP_ITER(bmp, port) {
        SOC_IF_ERROR_RETURN(REG_READ_PORT_SEC_CONr
            (unit, port, &reg_value));
        SOC_IF_ERROR_RETURN(soc_PORT_SEC_CONr_field_set
            (unit, &reg_value, SA_VIO_OPTf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_PORT_SEC_CONr
            (unit, port, &reg_value));
    }
    
    if ((trap_mask & DRV_SWITCH_TRAP_IPMC) || 
        (trap_mask & DRV_SWITCH_TRAP_GARP) || 
        (trap_mask & DRV_SWITCH_TRAP_ARP) ||
        (trap_mask & DRV_SWITCH_TRAP_8023AD) ||
        (trap_mask & DRV_SWITCH_TRAP_ICMP) ||
        (trap_mask & DRV_SWITCH_TRAP_BPDU2) ||
        (trap_mask & DRV_SWITCH_TRAP_RARP) ||
        (trap_mask & DRV_SWITCH_TRAP_8023AD_DIS) ||
        (trap_mask & DRV_SWITCH_TRAP_BGMP) ||
        (trap_mask & DRV_SWITCH_TRAP_LLDP)) {
        return SOC_E_UNAVAIL;
    }

    /* Broadcast packet */
    SOC_IF_ERROR_RETURN(REG_READ_MII_PCTLr
        (unit, CMIC_PORT(unit), &reg_value));
    if (trap_mask & DRV_SWITCH_TRAP_BCST) {
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(soc_MII_PCTLr_field_set
        (unit, &reg_value, MIRX_BC_ENf, &temp));    
    SOC_IF_ERROR_RETURN(REG_WRITE_MII_PCTLr
        (unit, CMIC_PORT(unit), &reg_value));
    
    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_trap_get
 *
 *  Purpose :
 *      Get the trap frame type to CPU.
 *
 *  Parameters :
 *      unit      :  unit id.
 *      port      :  port id.
 *      trap_mask :  the mask of trap type.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
int 
drv_harrier_trap_get(int unit, soc_port_t port, uint32 *trap_mask)
{
    uint32  reg_value, temp = 0;

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
        (unit, &reg_value, IGMP_MLD_CHKf, &temp));
    if (temp == HARRIER_REG_IGMP_MLD_TRAP) {
        *trap_mask |= DRV_SWITCH_TRAP_IGMP | DRV_SWITCH_TRAP_MLD;
    }

    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
        (unit, &reg_value, RXBPDU_ENf, &temp));
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_BPDU1;
    }

    SOC_IF_ERROR_RETURN(REG_READ_PORT_SEC_CONr
        (unit, port, &reg_value));

    SOC_IF_ERROR_RETURN(soc_PORT_SEC_CONr_field_get
        (unit, &reg_value, SA_VIO_OPTf, &temp));
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_8021X;
    }

    /* Broadcast packet */
    SOC_IF_ERROR_RETURN(REG_READ_MII_PCTLr
        (unit, CMIC_PORT(unit), &reg_value));
    SOC_IF_ERROR_RETURN(soc_MII_PCTLr_field_get
        (unit, &reg_value, MIRX_BC_ENf, &temp));

    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_BCST;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_harrier_trap_get: unit = %d, trap mask = 0x%x\n"), unit, *trap_mask));

    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_snoop_set
 *
 *  Purpose :
 *      Set the Snoop type.
 *
 *  Parameters :
 *      unit      :  unit id.
 *      snoop_mask:  the mask of snoop type.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
int 
drv_harrier_snoop_set(int unit, uint32 snoop_mask)
{
    uint32  reg_value, temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_harrier_snoop_set: \
                            unit = %d, snoop mask = 0x%x\n"), unit, snoop_mask));

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
        (unit, &reg_value));
    if (snoop_mask & DRV_SNOOP_IGMP) {    
        temp = HARRIER_REG_IGMP_MLD_SNOOP;
    } else {
        temp = 0;
        if (snoop_mask & DRV_SNOOP_MLD) {
            temp = HARRIER_REG_IGMP_MLD_SNOOP;
        }
    }

    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
        (unit, &reg_value, IGMP_MLD_CHKf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr
        (unit, &reg_value));

    if ((snoop_mask & DRV_SNOOP_ARP) ||
        (snoop_mask & DRV_SNOOP_RARP) ||
        (snoop_mask & DRV_SNOOP_ICMP) ||
        (snoop_mask & DRV_SNOOP_ICMPV6)) {
        return SOC_E_UNAVAIL;
    }
     
    return SOC_E_NONE; 
     
}

/*
 *  Function : drv_harrier_snoop_get
 *
 *  Purpose :
 *      Get the Snoop type.
 *
 *  Parameters :
 *      unit      :  unit id.
 *      snoop_mask:  the mask of snoop type.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
int 
drv_harrier_snoop_get(int unit, uint32 *snoop_mask)
{
    uint32  reg_value, temp = 0;
    
    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
        (unit, &reg_value, IGMP_MLD_CHKf, &temp));

    if (temp == HARRIER_REG_IGMP_MLD_SNOOP) {
       *snoop_mask = DRV_SNOOP_IGMP | DRV_SNOOP_MLD;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON, \
                (BSL_META_U(unit, \
                            "drv_harrier_snoop_get: \
                            unit = %d, snoop mask = 0x%x\n"), unit, *snoop_mask));

    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_igmp_mld_snoop_mode_get
 *
 *  Purpose :
 *      Get the Snoop mode for IGMP/MLD.
 *
 *  Parameters :
 *      unit      :  unit id.
 *      type      :  (IN) indicate a snoop type.
 *      mode      :  (OUT) indicate a snoop mode.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 */
int 
drv_harrier_igmp_mld_snoop_mode_get(int unit, int type, int *mode)
{
    uint32  reg_val = 0, temp = 0;
    
    if (!(type == DRV_IGMP_MLD_TYPE_IGMP_MLD || 
          type == DRV_IGMP_MLD_TYPE_IGMP || 
          type == DRV_IGMP_MLD_TYPE_MLD)) {
        return SOC_E_UNAVAIL;
    }

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
        (unit, &reg_val));
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
        (unit, &reg_val, IGMP_MLD_CHKf, &temp));

    *mode = (temp == HARRIER_REG_IGMP_MLD_SNOOP) ? DRV_IGMP_MLD_MODE_SNOOP : 
            (temp == HARRIER_REG_IGMP_MLD_TRAP) ? DRV_IGMP_MLD_MODE_TRAP : 
                DRV_IGMP_MLD_MODE_DISABLE;

    return SOC_E_NONE; 
}

/*
 *  Function : drv_harrier_igmp_mld_snoop_mode_set
 *
 *  Purpose :
 *      Set the Snoop mode for IGMP/MLD.
 *
 *  Parameters :
 *      unit      :  unit id.
 *      type      :  (IN) indicate a snoop type.
 *      mode      :  (IN) indicate a snoop mode.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note : 
 *  1. Harrier in HW spec. is to turn on/off IGMP and MLD together.
 *      Thus the proper type to contorl IGMP/MLD snooping mode is  
 *      'DRV_IGMP_MLD_TYPE_IGMP_MLD' and two special cases still be allowed 
 *      are 'DRV_IGMP_MLD_TYPE_IGMP' and 'DRV_IGMP_MLD_TYPE_MLD'.
 *    - 'DRV_IGMP_MLD_TYPE_IGMP' and 'DRV_IGMP_MLD_TYPE_MLD' will effects on 
 *      each other.
 */
int 
drv_harrier_igmp_mld_snoop_mode_set(int unit, int type, int mode)
{
    uint32  reg_val = 0, temp = 0;

    if (!(type == DRV_IGMP_MLD_TYPE_IGMP_MLD || 
          type == DRV_IGMP_MLD_TYPE_IGMP || 
          type == DRV_IGMP_MLD_TYPE_MLD)) {
        return SOC_E_UNAVAIL;
    }

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
        (unit, &reg_val));

    if (mode == DRV_IGMP_MLD_MODE_DISABLE) {
        temp = HARRIER_REG_IGMP_MLD_DISABLE;
    } else {
        if ((mode == DRV_IGMP_MLD_MODE_TRAP) || 
            (mode == DRV_IGMP_MLD_MODE_ENABLE)) {
            temp = HARRIER_REG_IGMP_MLD_TRAP;
        } else if (mode == DRV_IGMP_MLD_MODE_SNOOP) {
            temp = HARRIER_REG_IGMP_MLD_SNOOP;
        } else {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Unexpect mode assigned!\n"), FUNCTION_NAME(), __LINE__));
            return SOC_E_PARAM;
        }
    }
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
        (unit, &reg_val, IGMP_MLD_CHKf, &temp));    
    SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr
        (unit, &reg_val));

    return SOC_E_NONE; 
}


