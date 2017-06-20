/*
 * $Id: trap.c,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>

/*
 *  Function : drv_tbx_trap_set
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
drv_tbx_trap_set(int unit, soc_pbmp_t bmp, uint32 trap_mask)
{
    uint32  reg_value, temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_tbx_trap_set: unit = %d, trap mask = 0x%x\n"), unit, trap_mask));

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
        (unit, &reg_value));

    /* Retreive the current set first for IGMP/MLD snoop field within the 
     * value between 0 | 0x1 | 0x3 (not only on/off).
     *  - This is to avoid IGMP/MLD setting been override unexpected result  
     *     after called both trap_set and snoop_set
     */
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
        (unit, &reg_value, IGMP_MLD_CHKf, &temp));
    if (trap_mask & (DRV_SWITCH_TRAP_IGMP | DRV_SWITCH_TRAP_MLD) && 
        (trap_mask & (DRV_SWITCH_TRAP_IGMP_DISABLE | 
            DRV_SWITCH_TRAP_MLD_DISABLE))) {
        /* Means user set enable and disable together on IGMP/MLD */
        return SOC_E_CONFIG;
    } else if (trap_mask & (DRV_SWITCH_TRAP_IGMP | DRV_SWITCH_TRAP_MLD)) {
        temp = 0x1;
    } else if (trap_mask & (DRV_SWITCH_TRAP_IGMP_DISABLE | 
            DRV_SWITCH_TRAP_MLD_DISABLE)) {
        temp = 0x0;
    } else {
        /* Processing flow here means user doesn't request IGMP/MLD enable
         *  or disable. Leave original configuration on IGMP/MLD.
         */
    }
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
        (unit, &reg_value, IGMP_MLD_CHKf, &temp));

    /* @@@ CHECK_ME : Check if the VLAN bypass IGMP/MLD requirred @@@ 
     *  1. IGMP/MLD is native a VLAN aware protocol.
     *  2. Original deisgn may causes existed VLAN bypass IGMP/MLD been 
     *      overrided once this SOC driver is called.
     */
    /*
     * When IGMP/MLD snooping is enabled, 
     * need to bypass VLAN Ingress/Egress Filter for IGMP/MLD frame.
     */
    SOC_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
        (unit, DRV_VLAN_PROP_BYPASS_IGMP_MLD, (temp) ? 1 : 0));

    /* To indicate BPDU CPUCopy is enabled or not 
      * 1 = Enables all ports to receive BPDUs to CPU
      * 0 = Drop BPDU packet
      */
    if (trap_mask & DRV_SWITCH_TRAP_BPDU1) {    
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
        (unit, &reg_value, RX_BPDU_ENf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr
        (unit, &reg_value));

    /* To indicate EAP PDU CPUCopy is enabled or not
      * 1 = Enables all ports to receive EAP packet and forward to CPU
      * 0 = Drop EAP packet
      */    
    if (trap_mask & DRV_SWITCH_TRAP_8021X) {
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(REG_READ_SECURITY_GLOBAL_CTLr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_SECURITY_GLOBAL_CTLr_field_set
        (unit, &reg_value, RX_EAP_ENf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_SECURITY_GLOBAL_CTLr
        (unit, &reg_value));

    /* Broadcast packet */
    SOC_IF_ERROR_RETURN(REG_READ_IMP_PCTLr
        (unit, CMIC_PORT(unit), &reg_value));
    if (trap_mask & DRV_SWITCH_TRAP_BCST) {
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(soc_IMP_PCTLr_field_set
        (unit, &reg_value, RX_BC_ENf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_IMP_PCTLr
        (unit, CMIC_PORT(unit), &reg_value));
        
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
    
    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_trap_get
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
drv_tbx_trap_get(int unit, soc_port_t port, uint32 *trap_mask)
{
    uint32  reg_value, temp;

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
        (unit, &reg_value, IGMP_MLD_CHKf, &temp));

    /* Report the Trap status only */
    *trap_mask = 0;
    if (temp == 0x1) {
        *trap_mask |= DRV_SWITCH_TRAP_IGMP | DRV_SWITCH_TRAP_MLD;
    } else if (temp == 0) {
        /* DO NOT reutrn DRV_SWITCH_TRAP_IGMP_DISABLE or 
         *  DRV_SWITCH_TRAP_MLD_DISABLE. These two are for set only.
         *  - the reason is to avoid unexpect disable once user called 
         *  snoop_set and trap_set.
         */
    }

    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
        (unit, &reg_value, RX_BPDU_ENf, &temp));
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_BPDU1;
    }

    SOC_IF_ERROR_RETURN(REG_READ_SECURITY_GLOBAL_CTLr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_SECURITY_GLOBAL_CTLr_field_get
        (unit, &reg_value, RX_EAP_ENf, &temp));
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_8021X;
    }

    /* Broadcast packet */
    SOC_IF_ERROR_RETURN(REG_READ_IMP_PCTLr
        (unit, CMIC_PORT(unit), &reg_value));
    SOC_IF_ERROR_RETURN(soc_IMP_PCTLr_field_get
        (unit, &reg_value, RX_BC_ENf, &temp));
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_BCST;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_tbx_trap_get: unit = %d, trap mask = 0x%x\n"), unit, *trap_mask));

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_snoop_set
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
drv_tbx_snoop_set(int unit, uint32 snoop_mask)
{
    uint32  reg_value, temp;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_tbx_snoop_set: unit = %d, snoop mask = 0x%x\n"), unit, snoop_mask));

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
        (unit, &reg_value));
    
    /* Retreive the current set first for IGMP/MLD snoop field within the 
     * value between 0 | 0x1 | 0x3 (not only on/off).
     *  - This is to avoid IGMP/MLD setting been override unexpected result
     *     after called both trap_set and snoop_set
     */
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
        (unit, &reg_value, IGMP_MLD_CHKf, &temp));
    if (snoop_mask & (DRV_SNOOP_IGMP | DRV_SNOOP_MLD) && 
        (snoop_mask & (DRV_SNOOP_IGMP_DISABLE | 
            DRV_SNOOP_MLD_DISABLE))) {
        /* Means user set enable and disable together on IGMP/MLD */
        return SOC_E_CONFIG;
    } else if (snoop_mask & (DRV_SNOOP_IGMP | DRV_SNOOP_MLD)) {
        temp = 0x3;
    } else if (snoop_mask & (DRV_SNOOP_IGMP_DISABLE | 
            DRV_SNOOP_MLD_DISABLE)) {
        temp = 0x0;
    } else {
        /* processing flow here means user doesn't request IGMP/MLD enable
         *  or disable. Leave original configuration on IGMP/MLD.
         */
    }
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
        (unit, &reg_value, IGMP_MLD_CHKf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr
        (unit, &reg_value));

    /* @@@ CHECK_ME : Check if the VLAN bypass IGMP/MLD requirred @@@ 
     *  1. IGMP/MLD is native a VLAN aware protocol.
     *  2. Original deisgn may causes existed VLAN bypass IGMP/MLD been 
     *      overrided once this SOC driver is called.
     */
    /*
      * When IGMP/MLD snooping is enabled, 
      * need to bypass VLAN Ingress/Egress Filter for IGMP/MLD frame.
      */
    SOC_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
        (unit, DRV_VLAN_PROP_BYPASS_IGMP_MLD, (temp) ? 1 : 0));

    if ((snoop_mask & DRV_SNOOP_ARP) ||
        (snoop_mask & DRV_SNOOP_RARP) ||
        (snoop_mask & DRV_SNOOP_ICMP) ||
        (snoop_mask & DRV_SNOOP_ICMPV6)) {
        return SOC_E_UNAVAIL;
    }

    return SOC_E_NONE; 
}

/*
 *  Function : drv_tbx_snoop_get
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
drv_tbx_snoop_get(int unit, uint32 *snoop_mask)
{
    uint32  reg_value, temp;
    
    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
        (unit, &reg_value, IGMP_MLD_CHKf, &temp));

    /* Report the Trap status only */
    *snoop_mask = 0;
    if (temp == 0x3) {
        *snoop_mask |= DRV_SNOOP_IGMP | DRV_SNOOP_MLD;
    } else if (temp == 0) {
        /* DO NOT reutrn DRV_SNOOP_IGMP_DISABLE or 
         *  DRV_SNOOP_MLD_DISABLE. These two are for set only.
         *  - the reason is to avoid unexpect disable once user called 
         *  snoop_set and trap_set.
         */
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_tbx_snoop_get: unit = %d, snoop mask = 0x%x\n"), unit, *snoop_mask));

    return SOC_E_NONE;
}

/* the filed value in IGMP/MLD related register 
 *  - 0x2 is invalid.
 */
#define TBX_REG_SNOOP_DISABLE  0x0
#define TBX_REG_SNOOP_MODE     0x1
#define TBX_REG_TRAP_MODE      0x3

/*
 *  Function : drv_tbx_igmp_mld_snoop_mode_get
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
drv_tbx_igmp_mld_snoop_mode_get(int unit, int type, int *mode)
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

    *mode = (temp == TBX_REG_SNOOP_MODE) ? DRV_IGMP_MLD_MODE_SNOOP : 
            (temp == TBX_REG_TRAP_MODE) ? DRV_IGMP_MLD_MODE_TRAP : 
                DRV_IGMP_MLD_MODE_DISABLE;

    return SOC_E_NONE; 
}

/*
 *  Function : drv_tbx_igmp_mld_snoop_mode_set
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
 *  1. TB's special design that IGMP/MLD check will subject to any filter.
 *      That is CPU will not receive the Trap or Snoop packet once the IMP 
 *      is not the vlan member.
 *      >> VLAN_BYPASS_IGMP_MLD may causes the security issue due to the 
 *          VLAN ingress and egress filter been ignored.
 *          - the IGMP join/leave/query will flood to all linked port.
 *            no matter the ingress VID is registered or not and no matter 
 *            the egress port is VLAN member or not.
 *  2. TBX in HW spec. is to turn on/off IGMP and MLD together.
 *      Thus the proper type to contorl IGMP/MLD snooping mode is  
 *      'DRV_IGMP_MLD_TYPE_IGMP_MLD' and two special cases still be allowed 
 *      are 'DRV_IGMP_MLD_TYPE_IGMP' and 'DRV_IGMP_MLD_TYPE_MLD'.
 *    - 'DRV_IGMP_MLD_TYPE_IGMP' and 'DRV_IGMP_MLD_TYPE_MLD' will effects on 
 *      each other.
 */
int 
drv_tbx_igmp_mld_snoop_mode_set(int unit, int type, int mode)
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
        temp = TBX_REG_SNOOP_DISABLE;
    } else {
        if ((mode == DRV_IGMP_MLD_MODE_TRAP) || 
            (mode == DRV_IGMP_MLD_MODE_ENABLE)) {
            temp = TBX_REG_TRAP_MODE;
        } else if (mode == DRV_IGMP_MLD_MODE_SNOOP) {
            temp = TBX_REG_SNOOP_MODE;
        } else {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d, Unexpect mode assigned!\n"), FUNCTION_NAME(), __LINE__));
            return SOC_E_PARAM;
        }
    }
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
        (unit, &reg_val, IGMP_MLD_CHKf, &temp));
    /* Special process for TB only :
     *  - When IGMP/MLD snooping is enabled, the bypass VLAN Ingress/Egress 
     *      Filter for IGMP/MLD frame will be asserted for the expecting of 
     *      proper behavior.
     */
    SOC_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
        (unit, DRV_VLAN_PROP_BYPASS_IGMP_MLD, 
        (mode != DRV_IGMP_MLD_MODE_DISABLE) ? 1 : 0));
        
    SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr
        (unit, &reg_val));

    return SOC_E_NONE; 
}

