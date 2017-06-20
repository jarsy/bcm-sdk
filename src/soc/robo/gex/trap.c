/*
 * $Id: trap.c,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>

#define GEX_REG_VALUE_SNOOP  0
#define GEX_REG_VALUE_TRAP   1

/*
 *  Function : drv_gex_trap_set
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
drv_gex_trap_set(int unit, soc_pbmp_t bmp, uint32 trap_mask)
{
    uint32  reg_value, temp;
    uint32  snoop_mask = 0;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_gex_trap_set: unit = %d, trap mask = 0x%x\n"), unit, trap_mask));

    /* Get the snooping mask firet to prevent the unexpect value set causes 
     *  other snoop setting been effected.
     *  - prevent the IGMP/MLD/ICMP/DHCP/ARP/RARP been modified unexpected.
     */
    SOC_IF_ERROR_RETURN(DRV_SNOOP_GET
        (unit, &snoop_mask));
     
    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
        (unit, &reg_value));
    if (trap_mask & DRV_SWITCH_TRAP_BPDU1) {
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
        (unit, &reg_value, RXBPDU_ENf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr
        (unit, &reg_value));
    
    SOC_IF_ERROR_RETURN(REG_READ_HL_PRTC_CTRLr
        (unit, &reg_value));
    
    /* Set IGMP trap ====== */
    /* enable/disable bit for all IGMP pkt type CPU trap(CPU redirect) */
    if (trap_mask & DRV_SWITCH_TRAP_IGMP) {
        trap_mask |= DRV_SWITCH_TRAP_IGMP_UNKNOW | 
                    DRV_SWITCH_TRAP_IGMP_QUERY | 
                    DRV_SWITCH_TRAP_IGMP_REPLEV;
    }
    if (trap_mask & DRV_SWITCH_TRAP_IGMP_DISABLE) {
        temp = 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, IGMP_UKN_ENf, &temp));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, IGMP_QRY_ENf, &temp));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, IGMP_RPTLVE_ENf, &temp));
    } else {        
        /* Set IGMP related forwarding mode : 
         *      - forward_mode = 1 : Redirect to CPU only.
         *      - forward_mode = 0 : Snoop to CPU. (L2 forwarding also)
         */
        temp = ((trap_mask & DRV_SWITCH_TRAP_IGMP_QUERY) || 
                (snoop_mask & DRV_SNOOP_IGMP_QUERY)) ? 1 : 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, IGMP_QRY_ENf, &temp));
        if (temp) {
            if (trap_mask & DRV_SWITCH_TRAP_IGMP_QUERY) {
                temp = GEX_REG_VALUE_TRAP;
                SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                    (unit, &reg_value, IGMP_QRY_FWD_MODEf, &temp));
            }
        }
        
        temp = ((trap_mask & DRV_SWITCH_TRAP_IGMP_REPLEV) || 
                (snoop_mask & DRV_SNOOP_IGMP_REPLEV)) ? 1 : 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, IGMP_RPTLVE_ENf, &temp));
        if (temp) {
            if (trap_mask & DRV_SWITCH_TRAP_IGMP_REPLEV) {
                temp = GEX_REG_VALUE_TRAP;
                SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                    (unit, &reg_value, IGMP_RPTLVE_FWD_MODEf, &temp));
            }
        }
        
        temp = ((trap_mask & DRV_SWITCH_TRAP_IGMP_UNKNOW) || 
                (snoop_mask & DRV_SNOOP_IGMP_UNKNOW)) ? 1 : 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, IGMP_UKN_ENf, &temp));
        if (temp) {
            if (trap_mask & DRV_SWITCH_TRAP_IGMP_UNKNOW) {
                temp = GEX_REG_VALUE_TRAP;
                SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                    (unit, &reg_value, IGMP_UKN_FWD_MODEf, &temp));
            }
        }
    }
    
    /* Set MLD Trap ====== */
    /* enable/disable bit setting on all MLD pkt type snooping */
    if (trap_mask & DRV_SWITCH_TRAP_MLD) {
        trap_mask |= DRV_SWITCH_TRAP_MLD_QUERY | DRV_SWITCH_TRAP_MLD_REPDONE;
    }
    if (trap_mask & DRV_SWITCH_TRAP_MLD_DISABLE) {
        temp = 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, MLD_QRY_ENf, &temp));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, MLD_RPTDONE_ENf, &temp));
    } else {
        /* Set MLD related forwarding mode : 
         *      - forward_mode = 1 : Redirect to CPU only.
         *      - forward_mode = 0 : Snoop to CPU. (L2 forwarding also)
         */
        temp = ((trap_mask & DRV_SWITCH_TRAP_MLD_QUERY) || 
                (snoop_mask & DRV_SNOOP_MLD_QUERY)) ? 1 : 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, MLD_QRY_ENf, &temp));
        if (temp) {
            if (trap_mask & DRV_SWITCH_TRAP_MLD_QUERY) {
                temp = GEX_REG_VALUE_TRAP;
                SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                    (unit, &reg_value, MLD_QRY_FWD_MODEf, &temp));
            }
        }
        
        temp = ((trap_mask & DRV_SWITCH_TRAP_MLD_REPDONE) || 
                (snoop_mask & DRV_SNOOP_MLD_REPDONE)) ? 1 : 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, MLD_RPTDONE_ENf, &temp));
        if (temp) {
            if (trap_mask & DRV_SWITCH_TRAP_MLD_REPDONE) {
                temp = GEX_REG_VALUE_TRAP;
                SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                    (unit, &reg_value, MLD_RPTDONE_FWD_MODEf, &temp));
            }
        }
    }

    /* Set ICMPv4 trap (the same configuration with ICMPv4 snoop setting) */
    /* Marked the ICMPv4 trap section for ICMPv4 in GEX family can be 
     *  snooping mode only.
     */
    /* 
    temp = (trap_mask & DRV_SWITCH_TRAP_ICMP) ? 1 : 0;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, HL_PRTC_CTRLr, &reg_value, ICMPV4_ENf, &temp));
    */

    /* Set ICMPv6 trap :  */
    temp = (trap_mask & DRV_SWITCH_TRAP_ICMPV6_DISABLE) ? 0 : 
               (((trap_mask & DRV_SWITCH_TRAP_ICMPV6) || 
                   (snoop_mask & DRV_SNOOP_ICMPV6)) ? 1 : 0);
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
        (unit, &reg_value, ICMPV6_ENf, &temp));
    if (temp) {
        /* Set working mode */
        temp = (trap_mask & DRV_SWITCH_TRAP_ICMPV6) ? 
                GEX_REG_VALUE_TRAP : GEX_REG_VALUE_SNOOP;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, ICMPV6_FWD_MODEf, &temp));
    }

    /* Set RARP trap (the same configuration with RARP snoop setting) */
    temp = (trap_mask & DRV_SWITCH_TRAP_RARP) ? 1 : 0;
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
        (unit, &reg_value, RARP_ENf, &temp));

    /* Set ARP trap (the same configuration with ARP snoop setting) */
    temp = (trap_mask & DRV_SWITCH_TRAP_ARP) ? 1 : 0;
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
        (unit, &reg_value, ARP_ENf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_HL_PRTC_CTRLr
        (unit, &reg_value));

    if ((trap_mask & DRV_SWITCH_TRAP_8021X) ||
        (trap_mask & DRV_SWITCH_TRAP_IPMC) ||
        (trap_mask & DRV_SWITCH_TRAP_GARP) ||
        (trap_mask & DRV_SWITCH_TRAP_8023AD) ||
        (trap_mask & DRV_SWITCH_TRAP_BPDU2) ||
        (trap_mask & DRV_SWITCH_TRAP_8023AD_DIS) ||
        (trap_mask & DRV_SWITCH_TRAP_BGMP) ||
        (trap_mask & DRV_SWITCH_TRAP_LLDP)) {
        return SOC_E_UNAVAIL;
    }
    
    /* Broadcast packet */
    SOC_IF_ERROR_RETURN(REG_READ_IMP_CTLr
       (unit, CMIC_PORT(unit), &reg_value));
    if (trap_mask & DRV_SWITCH_TRAP_BCST) {
        temp = 1;
    } else {
        temp = 0;
    }
    SOC_IF_ERROR_RETURN(soc_IMP_CTLr_field_set
        (unit, &reg_value, RX_BCST_ENf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_IMP_CTLr
        (unit, CMIC_PORT(unit), &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_trap_get
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
drv_gex_trap_get(int unit, soc_port_t port, uint32 *trap_mask)
{
    uint32  reg_value, temp = 0;
    int  enable_flag;    /* for gex's igmp/mld only */

    /* Clear the mask */
    *trap_mask = 0;

    SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
        (unit, &reg_value));
    SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_get
        (unit, &reg_value, RXBPDU_ENf, &temp));
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_BPDU1;
    } 

    /* Broadcast packet */
    SOC_IF_ERROR_RETURN(REG_READ_IMP_CTLr
        (unit, CMIC_PORT(unit), &reg_value));
    SOC_IF_ERROR_RETURN(soc_IMP_CTLr_field_get
        (unit, &reg_value, RX_BCST_ENf, &temp));
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_BCST;
    }
    
    SOC_IF_ERROR_RETURN(REG_READ_HL_PRTC_CTRLr
        (unit, &reg_value));
    
    /* ---- Get IGMP trap setting ---- */ 
    enable_flag = FALSE;
    /* trap status on IGMP query */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, IGMP_QRY_ENf, &temp));
    if (temp) {
        enable_flag = TRUE;
        
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_value, IGMP_QRY_FWD_MODEf, &temp));
        if (temp == GEX_REG_VALUE_TRAP) {
            *trap_mask |= DRV_SWITCH_TRAP_IGMP_QUERY;
        }
    }
    
    /* trap status on IGMP report/leave */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, IGMP_RPTLVE_ENf, &temp));
    if (temp) {
        enable_flag = TRUE;
        
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_value, IGMP_RPTLVE_FWD_MODEf, &temp));
        if (temp == GEX_REG_VALUE_TRAP) {
            *trap_mask |= DRV_SWITCH_TRAP_IGMP_REPLEV;
        }
    }

    /* trap status on IGMP unknow */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, IGMP_UKN_ENf, &temp));
    if (temp) {
        enable_flag = TRUE;
        
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_value, IGMP_UKN_FWD_MODEf, &temp));
        if (temp == GEX_REG_VALUE_TRAP) {
            *trap_mask |= DRV_SWITCH_TRAP_IGMP_UNKNOW;
        }
    }
    
    /* trap status on IGMP all types information */
    if (!enable_flag) {
        *trap_mask |= DRV_SWITCH_TRAP_IGMP_DISABLE;
    }
    
    if ((*trap_mask & DRV_SWITCH_TRAP_IGMP_QUERY) && 
        (*trap_mask & DRV_SWITCH_TRAP_IGMP_REPLEV) &&
        (*trap_mask & DRV_SWITCH_TRAP_IGMP_UNKNOW)) {
        *trap_mask |= DRV_SWITCH_TRAP_IGMP;
    }

    /* ---- Get MLD trap setting ---- */
    enable_flag = FALSE;
    /* trap status on MLD query */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, MLD_QRY_ENf, &temp));
    if (temp) {
        enable_flag = TRUE;
        
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_value, MLD_QRY_FWD_MODEf, &temp));
        if (temp == GEX_REG_VALUE_TRAP) {
            *trap_mask |= DRV_SWITCH_TRAP_MLD_QUERY;
        }
    }
    
    /* trap status on MLD report */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, MLD_RPTDONE_ENf, &temp));
    if (temp) {
        enable_flag = TRUE;
        
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_value, MLD_RPTDONE_FWD_MODEf, &temp));
        if (temp == GEX_REG_VALUE_TRAP) {
            *trap_mask |= DRV_SWITCH_TRAP_MLD_REPDONE;
        }
    }
    
    /* trap status on MLD all types information */
    if (!enable_flag) {
        *trap_mask |= DRV_SWITCH_TRAP_MLD_DISABLE;
    }
    
    if ((*trap_mask & DRV_SWITCH_TRAP_MLD_QUERY) && 
        (*trap_mask & DRV_SWITCH_TRAP_MLD_REPDONE)) {
        *trap_mask |= DRV_SWITCH_TRAP_MLD;
    }
        
    /* Get ICMPv6 trap setting */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, ICMPV6_ENf, &temp));
    if (temp) {
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_value, ICMPV6_FWD_MODEf, &temp));
        if (temp == GEX_REG_VALUE_TRAP) {
            *trap_mask |= DRV_SWITCH_TRAP_ICMPV6;
        }
    } else {
        *trap_mask |= DRV_SWITCH_TRAP_ICMPV6_DISABLE;
    }
        
    /* Get ICMPv4 trap setting */
    /* Marked the ICMPv4 trap section for ICMPv4 in GEX family can be 
     *  snooping mode only.
     */
    /* 
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, HL_PRTC_CTRLr, &reg_value, ICMPV4_ENf, &temp));
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_ICMP;
    }
    */

    /* Get RARP trap setting */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, RARP_ENf, &temp));
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_RARP;
    }

    /* Get ARP trap setting */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, ARP_ENf, &temp));
    if (temp) {
        *trap_mask |= DRV_SWITCH_TRAP_ARP;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_gex_trap_get: unit = %d, trap mask = 0x%x\n"), unit, *trap_mask));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_snoop_set
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
drv_gex_snoop_set(int unit, uint32 snoop_mask)
{
    uint32  reg_value, temp;
    uint32  trap_mask = 0;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_gex_snoop_set: unit = %d, snoop mask = 0x%x\n"), unit, snoop_mask));

    /* Get the snooping mask firet to prevent the unexpect value set causes 
     *  other snoop setting been effected.
     *  - prevent the IGMP/MLD/ICMP/DHCP/ARP/RARP been modified unexpected.
     */
    SOC_IF_ERROR_RETURN(DRV_TRAP_GET
        (unit, 0, &trap_mask));

    SOC_IF_ERROR_RETURN(REG_READ_HL_PRTC_CTRLr
        (unit, &reg_value));
    
    /* Set IGMP ====== */
    /* enable/disable bit setting on all IGMP pkt type snooping */
    if (snoop_mask & DRV_SNOOP_IGMP) {
        snoop_mask |= DRV_SNOOP_IGMP_UNKNOW | 
                      DRV_SNOOP_IGMP_QUERY | 
                      DRV_SNOOP_IGMP_REPLEV;
    }
    if (snoop_mask & DRV_SNOOP_IGMP_DISABLE) {
        temp = 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, IGMP_UKN_ENf, &temp));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, IGMP_QRY_ENf, &temp));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, IGMP_RPTLVE_ENf, &temp));
    } else {
        /* Set IGMP related forwarding mode : 
         *      - forward_mode = 1 : Redirect to CPU only.
         *      - forward_mode = 0 : Snoop to CPU. (L2 forwarding also)
         */
        temp = ((trap_mask & DRV_SWITCH_TRAP_IGMP_QUERY) || 
                (snoop_mask & DRV_SNOOP_IGMP_QUERY)) ? 1 : 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, IGMP_QRY_ENf, &temp));
        if (temp) {
            if (snoop_mask & DRV_SNOOP_IGMP_QUERY) {
                temp = GEX_REG_VALUE_SNOOP;
                SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                    (unit, &reg_value, IGMP_QRY_FWD_MODEf, &temp));
            }
        }
        
        temp = ((trap_mask & DRV_SWITCH_TRAP_IGMP_REPLEV) || 
                (snoop_mask & DRV_SNOOP_IGMP_REPLEV)) ? 1 : 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, IGMP_RPTLVE_ENf, &temp));
        if (temp) {
            if (snoop_mask & DRV_SNOOP_IGMP_REPLEV) {
                temp = GEX_REG_VALUE_SNOOP;
                SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                    (unit, &reg_value, IGMP_RPTLVE_FWD_MODEf, &temp));
            }
        }
        
        temp = ((trap_mask & DRV_SWITCH_TRAP_IGMP_UNKNOW) || 
                (snoop_mask & DRV_SNOOP_IGMP_UNKNOW)) ? 1 : 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, IGMP_UKN_ENf, &temp));
        if (temp) {
            if (snoop_mask & DRV_SNOOP_IGMP_UNKNOW) {
                temp = GEX_REG_VALUE_SNOOP;
                SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                    (unit, &reg_value, IGMP_UKN_FWD_MODEf, &temp));
            }
        }
    }

    /* Set MLD ====== */
    /* enable/disable bit setting on all MLD pkt type snooping */
    if (snoop_mask & DRV_SNOOP_MLD) {
        snoop_mask |= DRV_SNOOP_MLD_QUERY | DRV_SNOOP_MLD_REPDONE;
    }
    if (snoop_mask & DRV_SNOOP_MLD_DISABLE) {
        temp = 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, MLD_QRY_ENf, &temp));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, MLD_RPTDONE_ENf, &temp));
    } else {
        /* Set MLD related forwarding mode : 
         *      - forward_mode = 1 : Redirect to CPU only.
         *      - forward_mode = 0 : Snoop to CPU. (L2 forwarding also)
         */
        temp = ((trap_mask & DRV_SWITCH_TRAP_MLD_QUERY) || 
                (snoop_mask & DRV_SNOOP_MLD_QUERY)) ? 1 : 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, MLD_QRY_ENf, &temp));
        if (temp) {
            if (snoop_mask & DRV_SNOOP_MLD_QUERY) {
                temp = GEX_REG_VALUE_SNOOP;
                SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                    (unit, &reg_value, MLD_QRY_FWD_MODEf, &temp));
            }
        }
        
        temp = ((trap_mask & DRV_SWITCH_TRAP_MLD_REPDONE) || 
                (snoop_mask & DRV_SNOOP_MLD_REPDONE)) ? 1 : 0;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, MLD_RPTDONE_ENf, &temp));
        if (temp) {
            if (snoop_mask & DRV_SNOOP_MLD_REPDONE) {
                temp = GEX_REG_VALUE_SNOOP;
                SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
                    (unit, &reg_value, MLD_RPTDONE_FWD_MODEf, &temp));
            }
        }
    }
        
    /* Set ICMP ====== */
    /* Set enable bits */
    temp = (snoop_mask & DRV_SNOOP_ICMP) ? 1 : 0;
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
        (unit, &reg_value, ICMPV4_ENf, &temp));
        
    /* Set ICMPv6 trap :  */
    temp = (snoop_mask & DRV_SNOOP_ICMPV6_DISABLE) ? 0 : 
               (((trap_mask & DRV_SWITCH_TRAP_ICMPV6) || 
                   (snoop_mask & DRV_SNOOP_ICMPV6)) ? 1 : 0);
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
        (unit, &reg_value, ICMPV6_ENf, &temp));
    if (temp) {
        /* Set working mode */
        temp = (snoop_mask & DRV_SNOOP_ICMPV6) ? 
                GEX_REG_VALUE_SNOOP : GEX_REG_VALUE_TRAP;
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_value, ICMPV6_FWD_MODEf, &temp));
    }
        
    /* Set DHCP ====== */
    /* Set enable bits */
    temp = (snoop_mask & DRV_SNOOP_DHCP) ? 1 : 0;
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
        (unit, &reg_value, DHCP_ENf, &temp));
    
    /* Set RARP ====== */
    /* Set enable bits */
    temp = (snoop_mask & DRV_SNOOP_RARP) ? 1 : 0;
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
        (unit, &reg_value, RARP_ENf, &temp));
    
    /* Set ARP ====== */
    /* Set enable bits */
    temp = (snoop_mask & DRV_SNOOP_ARP) ? 1 : 0;
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
        (unit, &reg_value, ARP_ENf, &temp));
    
    SOC_IF_ERROR_RETURN(REG_WRITE_HL_PRTC_CTRLr
        (unit, &reg_value));
    
    /* Enable unknow Multicast fowardeing for IGMP/MLD 
     *      1. Enable Multicast unknow forwarding mode. (for application can 
     *          indicate the MLF frame to defined routers.
     *      2. IP_MCf is defined as default set always in Register Spec. and 
     *          suggest not to set to zero. Here we set this field again to 
     *          avoid the improer field value.
     */
    if ((snoop_mask & DRV_SNOOP_IGMP) && 
         (!(SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_BLACKBIRD2(unit) || SOC_IS_POLAR(unit) || 
            SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
            SOC_IS_STARFIGHTER3(unit)))) {
        SOC_IF_ERROR_RETURN(REG_READ_NEW_CTRLr
            (unit, &reg_value));
        temp = 1;
        SOC_IF_ERROR_RETURN(soc_NEW_CTRLr_field_set
            (unit, &reg_value, IP_MCf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NEW_CTRLr
            (unit, &reg_value));
    }
     
    return SOC_E_NONE; 
     
}

/*
 *  Function : drv_gex_snoop_get
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
drv_gex_snoop_get(int unit, uint32 *snoop_mask)
{
    uint32  reg_value, temp = 0;
    int  enable_flag;    /* for igmp/mld only */
    
    SOC_IF_ERROR_RETURN(REG_READ_HL_PRTC_CTRLr
        (unit, &reg_value));
    
    /* ---- Get IGMP snoop setting ---- */ 
    *snoop_mask = 0;
    enable_flag = FALSE;
    /* snoop status on IGMP query */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, IGMP_QRY_ENf, &temp));
    if (temp) {
        enable_flag = TRUE;

        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_value, IGMP_QRY_FWD_MODEf, &temp));
        if (temp == GEX_REG_VALUE_SNOOP) {
            *snoop_mask |= DRV_SNOOP_IGMP_QUERY;
        }
    }
    
    /* snoop status on IGMP report/leave */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, IGMP_RPTLVE_ENf, &temp));
    if (temp) {
        enable_flag = TRUE;

        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_value, IGMP_RPTLVE_FWD_MODEf, &temp));
        if (temp == GEX_REG_VALUE_SNOOP) {
            *snoop_mask |= DRV_SNOOP_IGMP_REPLEV;
        }
    }

    /* snoop status on IGMP unknow */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, IGMP_UKN_ENf, &temp));
    if (temp) {
        enable_flag = TRUE;

        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_value, IGMP_UKN_FWD_MODEf, &temp));
        if (temp == GEX_REG_VALUE_SNOOP) {
            *snoop_mask |= DRV_SNOOP_IGMP_UNKNOW;
        }
    }
    
    /* snoop status on IGMP all types information */
    if (!enable_flag) {
        *snoop_mask |= DRV_SNOOP_IGMP_DISABLE;
    }
    
    if ((*snoop_mask & DRV_SNOOP_IGMP_QUERY) && 
        (*snoop_mask & DRV_SNOOP_IGMP_REPLEV) &&
        (*snoop_mask & DRV_SNOOP_IGMP_UNKNOW)) {
        *snoop_mask |= DRV_SNOOP_IGMP;
    }

    /* ---- Get MLD trap setting ---- */
    enable_flag = FALSE;
    /* snoop status on MLD query */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, MLD_QRY_ENf, &temp));
    if (temp) {
        enable_flag = TRUE;

        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_value, MLD_QRY_FWD_MODEf, &temp));
        if (temp == GEX_REG_VALUE_SNOOP) {
            *snoop_mask |= DRV_SNOOP_MLD_QUERY;
        }
    }
    
    /* snoop status on MLD report */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, MLD_RPTDONE_ENf, &temp));
    if (temp) {
        enable_flag = TRUE;

        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_value, MLD_RPTDONE_FWD_MODEf, &temp));
        if (temp == GEX_REG_VALUE_SNOOP) {
            *snoop_mask |= DRV_SNOOP_MLD_REPDONE;
        }
    }
    
    /* snoop status on MLD all types information */
    if (!enable_flag) {
        *snoop_mask |= DRV_SNOOP_MLD_DISABLE;
    }
    
    if ((*snoop_mask & DRV_SNOOP_MLD_QUERY) && 
        (*snoop_mask & DRV_SNOOP_MLD_REPDONE)) {        
        *snoop_mask |= DRV_SNOOP_MLD;
    }
        
    /* Get ICMPv6 snoop setting */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, ICMPV6_ENf, &temp));
    if (temp) {
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_value, ICMPV6_FWD_MODEf, &temp));
        if (temp == GEX_REG_VALUE_SNOOP) {
            *snoop_mask |= DRV_SNOOP_ICMPV6;
        }
    } else {
        *snoop_mask |= DRV_SNOOP_ICMPV6_DISABLE;
    }

    
    /* Get ICMP */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, ICMPV4_ENf, &temp));
    if (temp) {
        *snoop_mask |= DRV_SNOOP_ICMP;
    }

    /* Get DHCP */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, DHCP_ENf, &temp));
    if (temp) {
        *snoop_mask |= DRV_SNOOP_DHCP;
    }
    
    /* Get RARP */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, RARP_ENf, &temp));
    if (temp) {
        *snoop_mask |= DRV_SNOOP_RARP;
    }
    
    /* Get ARP */
    SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
        (unit, &reg_value, ARP_ENf, &temp));
    if (temp) {
        *snoop_mask |= DRV_SNOOP_ARP;
    }
    
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_gex_snoop_get: unit = %d, snoop mask = 0x%x\n"), unit, *snoop_mask));

    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_igmp_mld_snoop_mode_get
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
 *      1. This routine can serve GEX family due to the same chip spec.
 */
int 
drv_gex_igmp_mld_snoop_mode_get(int unit, int type, int *mode)
{
    uint32  reg_val = 0, temp_en = 0, temp_fwd = 0;
    
    if (!(SOC_IS_ROBO_ARCH_VULCAN(unit))) {
        /* improper for other chips */
        return SOC_E_INTERNAL;
    }
    
    SOC_IF_ERROR_RETURN(REG_READ_HL_PRTC_CTRLr
        (unit, &reg_val));
    temp_fwd = GEX_REG_VALUE_SNOOP;    /* preassign to value = 0 */
    
    if (type == DRV_IGMP_MLD_TYPE_IGMP_MLD) {
        /* No support on this type due to IGMP and MLD on this chip can be 
         * configured independently.
         */
        return SOC_E_UNAVAIL;
    } else if ((type == DRV_IGMP_MLD_TYPE_IGMP) || 
               (type == DRV_IGMP_MLD_TYPE_IGMP_REPLEV)) {
        /* to report the IGMP status, if the snooping function is enabled,
         *  we check the IGMP report/leave item to reflect the snooping mode
         *  on this type. The designing reason is the major IGMP snooping 
         *  process is for handling report/leave request. Also, we observed 
         *  that ESW chps have the same design.
         */
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_val, IGMP_RPTLVE_ENf, &temp_en));
        if (temp_en) {
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
                (unit, &reg_val, IGMP_RPTLVE_FWD_MODEf, &temp_fwd));
        }
    } else if ((type == DRV_IGMP_MLD_TYPE_MLD) || 
               (type == DRV_IGMP_MLD_TYPE_MLD_REPDONE)) {
        /* to report the MLD status, if the snooping function is enabled,
         *  we check the MLD report/leave item to reflect the snooping mode
         *  on this type. The designing reason is the major MLD snooping 
         *  process is for handling report/leave request. Also, we observed 
         *  that ESW chips have the same design.
         */
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_val, MLD_RPTDONE_ENf, &temp_en));
        if (temp_en) {
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
                (unit, &reg_val, MLD_RPTDONE_FWD_MODEf, &temp_fwd));
        }
    } else if (type == DRV_IGMP_MLD_TYPE_IGMP_QUERY) {
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_val, IGMP_QRY_ENf, &temp_en));
        if (temp_en) {
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
                (unit, &reg_val, IGMP_QRY_FWD_MODEf, &temp_fwd));
        }
    } else if (type == DRV_IGMP_MLD_TYPE_IGMP_UNKNOWN) {
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_val, IGMP_UKN_ENf, &temp_en));
        if (temp_en) {
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
                (unit, &reg_val, IGMP_UKN_FWD_MODEf, &temp_fwd));
        }
    } else if (type == DRV_IGMP_MLD_TYPE_MLD_QUERY) {
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
            (unit, &reg_val, MLD_QRY_ENf, &temp_en));
        if (temp_en) {
            SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_get
                (unit, &reg_val, MLD_QRY_FWD_MODEf, &temp_fwd));
        }
    } else {
        return SOC_E_UNAVAIL;
    }

    *mode = (!temp_en) ? DRV_IGMP_MLD_MODE_DISABLE : 
            ((temp_fwd == GEX_REG_VALUE_SNOOP) ? 
                DRV_IGMP_MLD_MODE_SNOOP : DRV_IGMP_MLD_MODE_TRAP);

    return SOC_E_NONE; 
}

/*
 *  Function : drv_gex_igmp_mld_snoop_mode_set
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
 *      1. This routine can serve GEX family due to the same chip spec.
 */
int 
drv_gex_igmp_mld_snoop_mode_set(int unit, int type, int mode)
{
    uint32  reg_val = 0, temp_en = 0, temp_fwd = 0;

    if (!(SOC_IS_ROBO_ARCH_VULCAN(unit))) {
        /* improper for other chips */
        return SOC_E_INTERNAL;
    }

    /* there are some speical deisgn for Vulcan : 
     *  1. to set to DISABLE, the Drop will be set to 0.
     *    - This is for the consistent SOC design on all robo chips. 
     *      Such configuration on Drop is actually a dummy configuration 
     *      due to Vulcan have no PURE Drop action on IGMP/MLD relate 
     *      snooping control.
     *    - Such design is consistent with other FE ROBO chips like 
     *      harrier or tbx for those chips in Register Spec. is a 
     *      two bits register filed the DISABLE value is '0' 
     *      (means internal Drop bit is '0')
     *  2. to set to ENABLE, the Drop action will be set to 1.
     *    - Trap is the most reasonable configuration for IGMP snooping 
     *      (no customer port talk)
     */
    
    if (mode == DRV_IGMP_MLD_MODE_DISABLE) {
        temp_en = FALSE;
        temp_fwd = GEX_REG_VALUE_SNOOP;    /* value = 0 */
    } else if (mode == DRV_IGMP_MLD_MODE_SNOOP) {
        temp_en = TRUE;
        temp_fwd = GEX_REG_VALUE_SNOOP;
    } else if (mode == DRV_IGMP_MLD_MODE_TRAP || 
               mode == DRV_IGMP_MLD_MODE_ENABLE) {
        temp_en = TRUE;
        temp_fwd = GEX_REG_VALUE_TRAP;
    } else {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s, snooping mode out of valid value(%d)\n"), FUNCTION_NAME(), mode));
        return SOC_E_PARAM;
    }
    
    SOC_IF_ERROR_RETURN(REG_READ_HL_PRTC_CTRLr
        (unit, &reg_val));
    if (type == DRV_IGMP_MLD_TYPE_IGMP_MLD) {
        /* No support on this type due to IGMP and MLD on this chip can be 
         * configured independently.
         */
        return SOC_E_UNAVAIL;
    } else if (type == DRV_IGMP_MLD_TYPE_IGMP) {
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, IGMP_RPTLVE_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, IGMP_RPTLVE_FWD_MODEf, &temp_fwd));
        
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, IGMP_QRY_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, IGMP_QRY_FWD_MODEf, &temp_fwd));
        
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, IGMP_UKN_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, IGMP_UKN_FWD_MODEf, &temp_fwd));
        
    } else if (type == DRV_IGMP_MLD_TYPE_MLD) {
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, MLD_RPTDONE_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, MLD_RPTDONE_FWD_MODEf, &temp_fwd));
         
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, MLD_QRY_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, MLD_QRY_FWD_MODEf, &temp_fwd));
         
    } else if (type == DRV_IGMP_MLD_TYPE_IGMP_REPLEV) {
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, IGMP_RPTLVE_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, IGMP_RPTLVE_FWD_MODEf, &temp_fwd));
        
    } else if (type == DRV_IGMP_MLD_TYPE_IGMP_QUERY) {
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, IGMP_QRY_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, IGMP_QRY_FWD_MODEf, &temp_fwd));
        
    } else if (type == DRV_IGMP_MLD_TYPE_IGMP_UNKNOWN) {
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, IGMP_UKN_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, IGMP_UKN_FWD_MODEf, &temp_fwd));
        
    } else if (type == DRV_IGMP_MLD_TYPE_MLD_REPDONE) {
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, MLD_RPTDONE_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, MLD_RPTDONE_FWD_MODEf, &temp_fwd));
    } else if (type == DRV_IGMP_MLD_TYPE_MLD_QUERY) {
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, MLD_QRY_ENf, &temp_en));
        SOC_IF_ERROR_RETURN(soc_HL_PRTC_CTRLr_field_set
            (unit, &reg_val, MLD_QRY_FWD_MODEf, &temp_fwd));
    } else {
        return SOC_E_UNAVAIL;
    }
    
    SOC_IF_ERROR_RETURN(REG_WRITE_HL_PRTC_CTRLr
        (unit, &reg_val));

    return SOC_E_NONE; 
}

