/*
 * $Id: port.c,v 1.16 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/robo/mcm/memregs.h>
#include <soc/phy.h>
#include <soc/phy/phyctrl.h>
#include <soc/phyreg.h>

#include "robo_tbx.h"
#include "../common/robo_common.h"



#define DOT1P_PRI_MASK  0x7
#define DOT1P_CFI_MASK  0x1
#define TC_VALID_MASK  0xf
#define DP_VALID_MASK  0x3

/* The loopback set on ROBO's internal FE PHY can't work at speed=10.
 *  - tested on bcm5347 and found 2 cases with speed=10 but loopback still 
 *      working properly.
 *      a. set loopback at speed=10 and Auto-Neg is on.
 *      b. set loopback at speed=100 with AN=on and than set loopback to 
 *          speed=10 with AN=off
 */
#define ROBO_INTFE_NO_FE10_LOOPBACK_CONFIRMED   0

#define PORT_CFG_EN_SA_MOVE_DROP        _TB_PMASK_PCONFIG_EN_SA_MOVE_DROP
#define PORT_CFG_EN_SA_MOVE_CPUCOPY     _TB_PMASK_PCONFIG_EN_SA_MOVE_CPU
#define PORT_CFG_DIS_LRN_SA_MOVE        _TB_PMASK_PCONFIG_DIS_LRN_SA_MOVE
#define PORT_CFG_EN_SA_UNKNOWN_DROP     _TB_PMASK_PCONFIG_EN_SA_UNKNOWN_DROP
#define PORT_CFG_EN_SA_UNKNOWN_CPUCOPY  _TB_PMASK_PCONFIG_EN_SA_UNKNOWN_CPU
#define PORT_CFG_EN_SA_OVERLIMIT_DROP   _TB_PMASK_PCONFIG_EN_SA_OVERLIMIT_DROP
#define PORT_CFG_EN_SA_OVERLIMIT_CPUCOPY  \
                _TB_PMASK_PCONFIG_EN_SA_OVERLIMIT_CPU

#define PORT_CFG_EN_SA_MASK     \
        (PORT_CFG_EN_SA_MOVE_DROP | PORT_CFG_EN_SA_MOVE_CPUCOPY |   \
        PORT_CFG_DIS_LRN_SA_MOVE | PORT_CFG_EN_SA_UNKNOWN_DROP |    \
        PORT_CFG_EN_SA_UNKNOWN_CPUCOPY | PORT_CFG_EN_SA_OVERLIMIT_DROP | \
        PORT_CFG_EN_SA_OVERLIMIT_CPUCOPY)
#define SA_LRN_CNT_LIMIT_MAX    0x4000

#define DEFAULT_COSQ_PRIO_VALID(unit, val) \
        (SOC_IS_TBX(unit) ? ((val) >= 0 && (val < 16)) : \
                                          ((val) >= 0 && (val < 8)))
#define DEFAULT_DROP_PRECEDENCE_VALID(val) \
        ((val) >= 0 && (val <= 3))

#define _ERROR_RETURN_UNLOCK_MEM(_unit, op, mem) \
        do { int __rv__; if ((__rv__ = (op)) < 0) { _SHR_ERROR_TRACE(__rv__);  \
            MEM_UNLOCK(_unit, mem); return(__rv__); } } while(0)

/* the port_mask table configuring interface to port driver services */
static int
_drv_tbx_portmask_config_get(int unit, int port, 
        _drv_tb_pmask_config_type_t config_type, uint32 *val)
{
    uint32  fld32_data, shift_cnt;
    int rv;    
    port_mask_entry_t   pmask_entry;
    
    /* read pmask entry and get the config field */
    sal_memset(&pmask_entry, 0, sizeof(port_mask_entry_t));
    MEM_LOCK(unit, INDEX(PORT_MASKm));
    rv = MEM_READ_PORT_MASKm(unit, port, (uint32 *)&pmask_entry);
    _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));


    rv = soc_PORT_MASKm_field_get(unit, (uint32 *)&pmask_entry, 
        PORT_CONFIGf, &fld32_data);
    _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));

    
    /* get config of the type bit */
    if (config_type < _TB_PMASK_CONFIG_TYPE_COUNT){
        if (config_type == _TB_PMASK_CONFIG_TYPE_RAW_VALUE){
            *val = fld32_data;
        } else {
            shift_cnt = _TB_PMASK_CONFIG_TYPE_SHIFT_BIT_GET(config_type);
            *val = (fld32_data & ((uint32)(1 << shift_cnt))) ? 1 : 0;
        }
    } else {
        MEM_UNLOCK(unit, INDEX(PORT_MASKm));
        return SOC_E_PARAM;
    }
    MEM_UNLOCK(unit, INDEX(PORT_MASKm));    
    return  SOC_E_NONE;
}


/* the port_mask table configuring interface to port driver services */
static int
_drv_tbx_portmask_config_set(int unit, int port, 
        _drv_tb_pmask_config_type_t config_type, uint32 val)
{
    uint32  fld32_data;
    uint32  shift_cnt;
    int rv;
    port_mask_entry_t   pmask_entry;
    

    MEM_LOCK(unit, INDEX(PORT_MASKm));
    /* read pmask entry and get the config field */
    sal_memset(&pmask_entry, 0, sizeof(port_mask_entry_t));
    rv = MEM_READ_PORT_MASKm(unit, port, (uint32 *)&pmask_entry);
    _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));

    /* set config of the type bit */
    if (config_type < _TB_PMASK_CONFIG_TYPE_COUNT){
        if (config_type == _TB_PMASK_CONFIG_TYPE_RAW_VALUE){
            fld32_data = val;
        } else {
            rv = soc_PORT_MASKm_field_get(unit, (uint32 *)&pmask_entry, 
                PORT_CONFIGf, &fld32_data);
            _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));

            shift_cnt = _TB_PMASK_CONFIG_TYPE_SHIFT_BIT_GET(config_type);
            
            if(val){
                fld32_data |= ((uint32)(1 << shift_cnt));
            } else {
                fld32_data &= ~((uint32)(1 << shift_cnt));
            }
        }
    } else {
        MEM_UNLOCK(unit, INDEX(PORT_MASKm));
        return SOC_E_PARAM;
    }
    rv = soc_PORT_MASKm_field_set(unit, (uint32 *)&pmask_entry, 
        PORT_CONFIGf, &fld32_data);
    _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));

    rv = MEM_WRITE_PORT_MASKm(unit, port, (uint32 *)&pmask_entry);
    _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));

    MEM_UNLOCK(unit, INDEX(PORT_MASKm));
    return  SOC_E_NONE;
}

/* the port_mask table configuring interface to port driver services */
static int
_drv_tbx_portmask_mask_get(int unit, int port, 
        _drv_tb_pmask_config_type_t mask_type, uint32 *val)
{
    uint32  fld32_data;
    int rv;    
    port_mask_entry_t   pmask_entry;
    
    MEM_LOCK(unit, INDEX(PORT_MASKm));

    /* read pmask entry and get the config field */
    sal_memset(&pmask_entry, 0, sizeof(port_mask_entry_t));           
    rv = MEM_READ_PORT_MASKm(unit, port, (uint32 *)&pmask_entry);
    _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));


    switch(mask_type){
        case _TB_PMASK_MASK_TYPE_ANY:
            rv = soc_PORT_MASKm_field_get(unit, (uint32 *)&pmask_entry, 
                MASK_ANYf, &fld32_data);
            break;
        case _TB_PMASK_MASK_TYPE_DLF_UCAST :
            rv = soc_PORT_MASKm_field_get(unit, (uint32 *)&pmask_entry, 
                MASK_DLF_UCSTf, &fld32_data);
            break;
        case _TB_PMASK_MASK_TYPE_DLF_L2MCAST : 
            rv = soc_PORT_MASKm_field_get(unit, (uint32 *)&pmask_entry, 
                MASK_DLF_L2MCSTf, &fld32_data);
            break;
        case _TB_PMASK_MASK_TYPE_DLF_L3MCAST :
            rv = soc_PORT_MASKm_field_get(unit, (uint32 *)&pmask_entry, 
                MASK_DLF_L3MCASTf, &fld32_data);
            break;
        case _TB_PMASK_MASK_TYPE_BCAST :
            rv = soc_PORT_MASKm_field_get(unit, (uint32 *)&pmask_entry, 
                MASK_BCSTf, &fld32_data);
            break;
        default :
            MEM_UNLOCK(unit, INDEX(PORT_MASKm));
            return SOC_E_PARAM;
    }
    
    _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));

    *val = fld32_data;
    MEM_UNLOCK(unit, INDEX(PORT_MASKm));
    return  SOC_E_NONE;
}

/* the port_mask table configuring interface to port driver services */
static int
_drv_tbx_portmask_mask_set(int unit, int port, 
        _drv_tb_pmask_config_type_t mask_type, uint32 val)
{
    int rv;
    port_mask_entry_t   pmask_entry;
    
    MEM_LOCK(unit, INDEX(PORT_MASKm));

    /* read pmask entry and get the config field */
    sal_memset(&pmask_entry, 0, sizeof(port_mask_entry_t));           
    rv = MEM_READ_PORT_MASKm(unit, port, (uint32 *)&pmask_entry);
    _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));

    switch(mask_type){
        case _TB_PMASK_MASK_TYPE_ANY:
            rv = soc_PORT_MASKm_field_set(unit, (uint32 *)&pmask_entry, 
                MASK_ANYf, &val);
            break;
        case _TB_PMASK_MASK_TYPE_DLF_UCAST :
            rv = soc_PORT_MASKm_field_set(unit, (uint32 *)&pmask_entry, 
                MASK_DLF_UCSTf, &val);
            break;
        case _TB_PMASK_MASK_TYPE_DLF_L2MCAST : 
            rv = soc_PORT_MASKm_field_set(unit, (uint32 *)&pmask_entry, 
                MASK_DLF_L2MCSTf, &val);
            break;
        case _TB_PMASK_MASK_TYPE_DLF_L3MCAST :
            rv = soc_PORT_MASKm_field_set(unit, (uint32 *)&pmask_entry, 
                MASK_DLF_L3MCASTf, &val);
            break;
        case _TB_PMASK_MASK_TYPE_BCAST :
            rv = soc_PORT_MASKm_field_set(unit, (uint32 *)&pmask_entry, 
                MASK_BCSTf, &val);
            break;
        default :
            MEM_UNLOCK(unit, INDEX(PORT_MASKm));
            return SOC_E_PARAM;
    }
    _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));

    rv = MEM_WRITE_PORT_MASKm(unit, port, (uint32 *)&pmask_entry);
    _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));

    MEM_UNLOCK(unit, INDEX(PORT_MASKm));    
    return  SOC_E_NONE;
}

STATIC int
_drv_tbx_port_security_mode_set(int unit, uint32 port, uint32 mode, 
    uint32 value)
{
    uint32  reg_value32 = 0;
    uint32  field_val32 = 0, temp = 0;
    int rv = SOC_E_NONE;

    /* The security mode */
    switch (mode) {
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            MEM_LOCK(unit, INDEX(PORT_MASKm));

            temp = (~PORT_CFG_EN_SA_MASK);
            rv = _drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_RAW_VALUE, &field_val32);
            _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));

            field_val32 &= temp;
            rv = _drv_tbx_portmask_config_set(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_RAW_VALUE, field_val32);
            _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));

            MEM_UNLOCK(unit, INDEX(PORT_MASKm));
            break;
        case DRV_PORT_PROP_EAP_EN_CHK_OPT:
            /* 
              * Option to control EAP packet checking
              * 0 : Only check DA=01-80-c2-00-00-03. (default)
              * 1 : Full check, including DA, EtherType, Packet Type, non-1Q.
              */
            if (value) {
                temp = 1;
            } else {
                temp = 0;
            }
            SOC_IF_ERROR_RETURN(REG_READ_SECURITY_GLOBAL_CTLr
                (unit, &reg_value32));
            SOC_IF_ERROR_RETURN(soc_SECURITY_GLOBAL_CTLr_field_get
                (unit, &reg_value32, EN_CHK_OPTf, &field_val32));
            /* check original setting */
            if (temp != field_val32) {
                SOC_IF_ERROR_RETURN(soc_SECURITY_GLOBAL_CTLr_field_set
                    (unit, &reg_value32, EN_CHK_OPTf, &temp));
    
                SOC_IF_ERROR_RETURN(REG_WRITE_SECURITY_GLOBAL_CTLr
                    (unit, &reg_value32));
            }
            break;
        default:
            return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

STATIC int
_drv_tbx_port_security_mode_get(int unit, uint32 port, uint32 mode, 
    uint32 *prop_val)
{
    uint32  reg_value32 = 0;
    uint32  field_val32 = 0, temp = 0;
    int rv = SOC_E_NONE;

    /* the security mode */
    switch (mode) {
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            /* Port Mask Table is indexed with port (= 0 ~ 28) */
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_RAW_VALUE, &field_val32));
            temp = field_val32 & PORT_CFG_EN_SA_MASK;
            *prop_val = FALSE;
            if (!temp) {
                *prop_val = TRUE;
            }
            break;
        case DRV_PORT_PROP_EAP_EN_CHK_OPT:
            /* 
              * Option to control EAP packet checking
              * 0 : Only check DA=01-80-c2-00-00-03. (default)
              * 1 : Full check, including DA, EtherType, Packet Type, non-1Q.
              */
            SOC_IF_ERROR_RETURN(REG_READ_SECURITY_GLOBAL_CTLr
                (unit, &reg_value32));
            SOC_IF_ERROR_RETURN(soc_SECURITY_GLOBAL_CTLr_field_get
                (unit, &reg_value32, EN_CHK_OPTf, &field_val32));
  
            if (field_val32) {
                *prop_val = TRUE;
            } else {
                *prop_val = FALSE;
            }
            break;
        default:
            return SOC_E_PARAM;
    }

    return rv;
}

/*
 *  Function : _drv_tbx_port_default_qos_config_set
 *
 *  Purpose :
 *      Set the port based default qos configuration.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port    :   port id.
 *      property  :   port property type.
 *      prop_val  :   property value.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
STATIC int 
_drv_tbx_port_default_qos_config_set(int unit, int port, int property, uint32 prop_val)
{
    uint64  reg_value64;
    int     temp = 0;

    SOC_IF_ERROR_RETURN(REG_READ_DEF_PORT_QOS_CFGr
            (unit, port, (void *)&reg_value64));

    temp = prop_val;
    /* These features currently are supported in TB chip only */
    switch (property) {
        case DRV_PORT_PROP_DEFAULT_TC_PRIO:
        case DRV_PORT_PROP_UNTAG_DEFAULT_TC:
            /* It indicates the TC priority attributes assoiated with the port for TB. */
            if (!DEFAULT_COSQ_PRIO_VALID(unit, temp)) {
                return SOC_E_PARAM;
            }

            soc_DEF_PORT_QOS_CFGr_field_set
                (unit, (void *)&reg_value64, TCf, (uint32 *)&temp);

            SOC_IF_ERROR_RETURN(REG_WRITE_DEF_PORT_QOS_CFGr
                (unit, port, (void *)&reg_value64));
            break;

        case DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE:
            /* It indicates the DP(Drop precedence) attributes assoiated with the port for TB. */
            if (!DEFAULT_DROP_PRECEDENCE_VALID(temp)) {
                return SOC_E_PARAM;
            }
            soc_DEF_PORT_QOS_CFGr_field_set
                (unit, (void *)&reg_value64, DPf, (uint32 *)&temp);

            break;

        default:
            return SOC_E_PARAM;
    }
    
    SOC_IF_ERROR_RETURN(REG_WRITE_DEF_PORT_QOS_CFGr
        (unit, port, (void *)&reg_value64));

    return SOC_E_NONE;
}

/*
 *  Function : _drv_tbx_port_default_qos_config_get
 *
 *  Purpose :
 *      Set the port based default qos configuration.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port    :   port id.
 *      property  :   port property type.
 *      prop_val  :   property value.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
STATIC int 
_drv_tbx_port_default_qos_config_get(int unit, int port, int property, uint32 *prop_val)
{
    uint64  reg_value64;
    uint32  temp = 0;

    COMPILER_64_ZERO(reg_value64);
    
    /* It indicates the TC priority attributes assoiated with the port for TB. */
    SOC_IF_ERROR_RETURN(REG_READ_DEF_PORT_QOS_CFGr
        (unit, port, (void *)&reg_value64));
        
    /* These features currently are supported in TB chip only */
    switch (property) {
        case DRV_PORT_PROP_DEFAULT_TC_PRIO:
        case DRV_PORT_PROP_UNTAG_DEFAULT_TC:

            soc_DEF_PORT_QOS_CFGr_field_get
                (unit, (void *)&reg_value64, TCf, &temp);

            *prop_val = temp;
            break;

        case DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE:

            soc_DEF_PORT_QOS_CFGr_field_get
                (unit, (void *)&reg_value64, DPf, &temp);

            *prop_val = temp;
            break;

        default:
            return SOC_E_PARAM;
            break;
    }
    return SOC_E_NONE;
}

/*
 *  Function : _drv_tbx_port_property_enable_set
 *
 *  Purpose :
 *      Set the port based properties enable/disable.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port    :   port id.
 *      property  :   port property type.
 *      enable  :   enable/disable
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
STATIC int 
_drv_tbx_port_property_enable_set(int unit, int port, 
        int property, uint32 enable)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value = 0, temp = 0;
    soc_pbmp_t  pbmp;

    switch (property) {
        case DRV_PORT_PROP_ENABLE_RX:
            if (enable) {
                temp = 0;
            } else {
                temp = 1;
            }
            if (IS_FE_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                        REG_READ_TH_PCTLr(unit, port, &reg_value));
                soc_TH_PCTLr_field_set(unit, &reg_value, RX_DISf, &temp);
                SOC_IF_ERROR_RETURN(
                        REG_WRITE_TH_PCTLr(unit, port, &reg_value));
            } else if (IS_GE_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                        REG_READ_G_PCTLr(unit, port, &reg_value));
                soc_G_PCTLr_field_set(unit, &reg_value, RX_DISf, &temp);
                SOC_IF_ERROR_RETURN(
                        REG_WRITE_G_PCTLr(unit, port, &reg_value));
            } else if (IS_CPU_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                        REG_READ_IMP_PCTLr(unit, port, &reg_value));
                soc_IMP_PCTLr_field_set(unit, &reg_value, RX_DISf, &temp);
                SOC_IF_ERROR_RETURN(
                        REG_WRITE_IMP_PCTLr(unit, port, &reg_value));
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_TX:
            if (enable) {
                temp = 0;
            } else {
                temp = 1;
            }
            if (IS_FE_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                        REG_READ_TH_PCTLr(unit, port, &reg_value));
                soc_TH_PCTLr_field_set(unit, &reg_value, TX_DISf, &temp);
                SOC_IF_ERROR_RETURN(
                        REG_WRITE_TH_PCTLr(unit, port, &reg_value));
            } else if (IS_GE_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                        REG_READ_G_PCTLr(unit, port, &reg_value));
                soc_G_PCTLr_field_set(unit, &reg_value, TX_DISf, &temp);
                SOC_IF_ERROR_RETURN(
                        REG_WRITE_G_PCTLr(unit, port, &reg_value));
            } else if (IS_CPU_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                        REG_READ_IMP_PCTLr(unit, port, &reg_value));
                soc_IMP_PCTLr_field_set(unit, &reg_value, TX_DISf, &temp);
                SOC_IF_ERROR_RETURN(
                        REG_WRITE_IMP_PCTLr(unit, port, &reg_value));
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_TXRX:
            if (enable) {
                temp = 0;
            } else {
                temp = 1;
            }
            if (IS_FE_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                        REG_READ_TH_PCTLr(unit, port, &reg_value));
                soc_TH_PCTLr_field_set(unit, &reg_value, TX_DISf, &temp);
                soc_TH_PCTLr_field_set(unit, &reg_value, RX_DISf, &temp);
                SOC_IF_ERROR_RETURN(
                        REG_WRITE_TH_PCTLr(unit, port, &reg_value));
            } else if (IS_GE_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                        REG_READ_G_PCTLr(unit, port, &reg_value));
                soc_G_PCTLr_field_set(unit, &reg_value, TX_DISf, &temp);
                soc_G_PCTLr_field_set(unit, &reg_value, RX_DISf, &temp);
                SOC_IF_ERROR_RETURN(
                        REG_WRITE_G_PCTLr(unit, port, &reg_value));
            } else if (IS_CPU_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                        REG_READ_IMP_PCTLr(unit, port, &reg_value));
                soc_IMP_PCTLr_field_set(unit, &reg_value, RX_DISf, &temp);
                soc_IMP_PCTLr_field_set(unit, &reg_value, TX_DISf, &temp);
                SOC_IF_ERROR_RETURN(
                        REG_WRITE_IMP_PCTLr(unit, port, &reg_value));
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
            MEM_LOCK(unit, INDEX(PORT_MASKm));
            rv = _drv_tbx_portmask_config_set(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_UNTAG_DROP, enable);
            _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));            
            rv = _drv_tbx_portmask_config_set(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_PRITAG_DROP, enable);
            _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));            
            MEM_UNLOCK(unit, INDEX(PORT_MASKm));
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
            MEM_LOCK(unit, INDEX(PORT_MASKm));
            rv = _drv_tbx_portmask_config_set(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_1QTAG_DROP, enable);
            _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));            
            rv = _drv_tbx_portmask_config_set(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_MIS_TAG_DROP, enable);
            _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));
            MEM_UNLOCK(unit, INDEX(PORT_MASKm));
            break;
        case DRV_PORT_PROP_ENABLE_DROP_UNTAG:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_set(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_UNTAG_DROP, enable));
            break;
        case DRV_PORT_PROP_ENABLE_DROP_PRITAG:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_set(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_PRITAG_DROP, enable));
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1QTAG_IVM_HIT:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_set(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_1QTAG_DROP, enable));
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1QTAG_IVM_MISS:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_set(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_MIS_TAG_DROP, enable));
            break;
        case DRV_PORT_PROP_DISABLE_LEARN:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_set(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_SA_LRN_DISABLE, enable));
            break;
        case DRV_PORT_PROP_SW_LEARN_MODE:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_set(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_SA_LRN_MODE, enable));
            break;
        case DRV_PORT_PROP_SA_MOVE_DROP:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_set(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_SA_MOVE_DROP, enable));
            break;
        case DRV_PORT_PROP_SA_MOVE_CPUCOPY:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_set(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_SA_MOVE_CPU, enable));
            break;
        case DRV_PORT_PROP_SA_UNKNOWN_DROP:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_set(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_SA_UNKNOWN_DROP, enable));
            break;
        case DRV_PORT_PROP_SA_UNKNOWN_CPUCOPY:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_set(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_SA_UNKNOWN_CPU, enable));
            break;
        case DRV_PORT_PROP_SA_OVERLIMIT_DROP:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_set(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_SA_OVERLIMIT_DROP, enable));
            break;
        case DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_set(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_SA_OVERLIMIT_CPU, enable));
            break;
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_RX:
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_TX:
            /* TB have no such feature */
            return SOC_E_UNAVAIL;
        case DRV_PORT_PROP_DTAG_MODE:
            SOC_IF_ERROR_RETURN(DRV_VLAN_PROP_SET
                (unit, DRV_VLAN_PROP_TRANSLATE_MODE, enable));
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            SOC_PBMP_CLEAR(pbmp);
            SOC_PBMP_PORT_ADD(pbmp, port);
            
            rv = DRV_VLAN_PROP_PORT_ENABLE_SET
                    (unit, DRV_VLAN_PROP_ISP_PORT, pbmp, enable);                 
            break;
            
        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
            /* 
              * Enable/disable DSCP remarking for TB (Global-based)
              */
            SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr(unit, &reg_value));

            if (enable) {
                temp = 1;
            } else {
                temp = 0;
            }
            soc_QOS_CTLr_field_set(unit, &reg_value, EN_DSCP_REMARKf, &temp);
    
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_CTLr(unit, &reg_value)); 
            break;
        case DRV_PORT_PROP_EGRESS_ECN_REMARK:            
            if(SOC_IS_TB_AX(unit)){
                return SOC_E_UNAVAIL;
            } else {
                /* 
                  * Enable/disable ECN remarking for TB (Global-based)
                  */
                SOC_IF_ERROR_RETURN(REG_READ_PKT_MARK_CTLr(unit, &reg_value));

                if (enable) {
                    temp = 1;
                } else {
                    temp = 0;
                }

                soc_PKT_MARK_CTLr_field_set(unit, &reg_value, 
                    EN_ECN_REMARKf, &temp);

                SOC_IF_ERROR_RETURN(REG_WRITE_PKT_MARK_CTLr(unit, &reg_value));
            }
            break;
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
            /* 
              * Enable/disable PCP/DEI remarking for TB (Global-based)
              */
            SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr(unit, &reg_value));

            if (enable) {
                temp = 1;
            } else {
                temp = 0;
            }
            soc_QOS_CTLr_field_set(unit, &reg_value, EN_PCP_REMARKf, &temp);
    
            SOC_IF_ERROR_RETURN(REG_WRITE_QOS_CTLr(unit, &reg_value)); 
             
            break;
            
        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
            /* This feature currently is supported in 53115 robo chip only */
        case DRV_PORT_PROP_EGRESS_C_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_S_PCP_REMARK:
                return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
            break;
    }
    return rv;
}

/*
 *  Function : _drv_tbx_port_property_enable_get
 *
 *  Purpose :
 *      Get the status of port related properties
 *
 *  Parameters :
 *      unit        :   unit id
 *      port    :   port id.
 *      property  :   port property type.
 *      enable  :   status of this property.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
STATIC int 
_drv_tbx_port_property_enable_get(int unit, int port, 
        int property, uint32 *enable)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value = 0, temp = 0, temp2 = 0;

    switch (property) {
        case DRV_PORT_PROP_ENABLE_RX:
            if (IS_FE_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                        REG_READ_TH_PCTLr(unit, port, &reg_value));
                soc_TH_PCTLr_field_get(unit, &reg_value, RX_DISf, &temp);
            } else if (IS_GE_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                        REG_READ_G_PCTLr(unit, port, &reg_value));
                soc_G_PCTLr_field_get(unit, &reg_value, RX_DISf, &temp);
            } else if (IS_CPU_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                        REG_READ_IMP_PCTLr(unit, port, &reg_value));
                soc_IMP_PCTLr_field_get(unit, &reg_value, RX_DISf, &temp);
            } else {
                return SOC_E_PARAM;
            }
            if (temp) {
                *enable = FALSE;
            } else {
                *enable = TRUE;
            }
            break;
        case DRV_PORT_PROP_ENABLE_TX:
            if (IS_FE_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                        REG_READ_TH_PCTLr(unit, port, &reg_value));
                soc_TH_PCTLr_field_get(unit, &reg_value, TX_DISf, &temp);
            } else if (IS_GE_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                        REG_READ_G_PCTLr(unit, port, &reg_value));
                soc_G_PCTLr_field_get(unit, &reg_value, TX_DISf, &temp);
            } else if (IS_CPU_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                        REG_READ_IMP_PCTLr(unit, port, &reg_value));
                soc_IMP_PCTLr_field_get(unit, &reg_value, TX_DISf, &temp);
            } else {
                return SOC_E_PARAM;
            }
            if (temp) {
                *enable = FALSE;
            } else {
                *enable = TRUE;
            }
            break;
        case DRV_PORT_PROP_ENABLE_TXRX:
            if (IS_FE_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(REG_READ_TH_PCTLr(unit, port, &reg_value));
                soc_TH_PCTLr_field_get(unit, &reg_value, RX_DISf, &temp);
               if (temp) {
                    *enable = FALSE;
                    return rv;
                }
                soc_TH_PCTLr_field_get(unit, &reg_value, TX_DISf, &temp);
            } else if (IS_GE_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(REG_READ_G_PCTLr(unit, port, &reg_value));
                soc_G_PCTLr_field_get(unit, &reg_value, RX_DISf, &temp);
                if (temp) {
                    *enable = FALSE;
                    return rv;
                }
                soc_G_PCTLr_field_get(unit, &reg_value, TX_DISf, &temp);
            } else if (IS_CPU_PORT(unit, port)) {
                SOC_IF_ERROR_RETURN(
                        REG_READ_IMP_PCTLr(unit, port, &reg_value));
                soc_IMP_PCTLr_field_get(unit, &reg_value, RX_DISf, &temp);
                if (temp) {
                    *enable = FALSE;
                } else {
                    *enable = TRUE;
                }
                soc_IMP_PCTLr_field_get(unit, &reg_value, TX_DISf, &temp);
            } else {
                return SOC_E_PARAM;
            }
            if (temp) {
                *enable = FALSE;
            } else {
                *enable = TRUE;
            }
            break;
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
            MEM_LOCK(unit, INDEX(PORT_MASKm));
            rv = _drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_UNTAG_DROP, &temp);
            _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));

            rv = _drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_PRITAG_DROP, &temp2);
            _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));
            MEM_UNLOCK(unit, INDEX(PORT_MASKm));
            if (temp == temp2) {
                *enable = (temp & temp2) ? TRUE : FALSE;
            } else {
                *enable = FALSE;
                return SOC_E_FAIL;
            }
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
            MEM_LOCK(unit, INDEX(PORT_MASKm));
            rv = _drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_1QTAG_DROP, &temp);
            _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));

            rv = _drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_MIS_TAG_DROP, &temp2);
            _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));
            MEM_UNLOCK(unit, INDEX(PORT_MASKm));
            if (temp == temp2) {
                *enable = (temp & temp2) ? TRUE : FALSE;
            } else {
                *enable = FALSE;
                return SOC_E_FAIL;
            }
            break;
        case DRV_PORT_PROP_ENABLE_DROP_UNTAG:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_UNTAG_DROP, enable));
            break;
        case DRV_PORT_PROP_ENABLE_DROP_PRITAG:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_PRITAG_DROP, enable));
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1QTAG_IVM_HIT:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_1QTAG_DROP, enable));
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1QTAG_IVM_MISS:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_MIS_TAG_DROP, enable));
            break;
        case DRV_PORT_PROP_DISABLE_LEARN:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_SA_LRN_DISABLE, enable));
            break;
        case DRV_PORT_PROP_SW_LEARN_MODE:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_SA_LRN_MODE, enable));
            break;
        case DRV_PORT_PROP_SA_MOVE_DROP:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_SA_MOVE_DROP, enable));
            break;
        case DRV_PORT_PROP_SA_MOVE_CPUCOPY:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_SA_MOVE_CPU, enable));
            break;
        case DRV_PORT_PROP_SA_UNKNOWN_DROP:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_SA_UNKNOWN_DROP, enable));
            break;
        case DRV_PORT_PROP_SA_UNKNOWN_CPUCOPY:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_SA_UNKNOWN_CPU, enable));
            break;
        case DRV_PORT_PROP_SA_OVERLIMIT_DROP:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_SA_OVERLIMIT_DROP, enable));
            break;
        case DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY:
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_SA_OVERLIMIT_CPU, enable));
            break;
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_RX:
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_TX:
            /* TB have no such feature */
            return SOC_E_UNAVAIL;
        case DRV_PORT_PROP_DTAG_MODE:
            SOC_IF_ERROR_RETURN(DRV_VLAN_PROP_GET
                (unit, DRV_VLAN_PROP_TRANSLATE_MODE, &temp));
            *enable = temp;
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            if ((rv = DRV_VLAN_PROP_PORT_ENABLE_GET
                (unit, DRV_VLAN_PROP_ISP_PORT, port, &temp)) < 0) {
                return rv;
            }
            *enable = temp;
            break;

        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
            /* 
              * Enable/disable DSCP remarking for TB (Global-based)
              */
            SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr(unit, &reg_value));

            soc_QOS_CTLr_field_get(unit, &reg_value, EN_DSCP_REMARKf, &temp);

            if (temp) {
                *enable = TRUE;
            } else {
                *enable = FALSE;
            }
            break;
            
        case DRV_PORT_PROP_EGRESS_ECN_REMARK:
            if(SOC_IS_TB_AX(unit)){
                return SOC_E_UNAVAIL;
            } else {                /* 
                  * Enable/disable ECN remarking for TB (Global-based)
                  */
                SOC_IF_ERROR_RETURN(REG_READ_PKT_MARK_CTLr(unit, &reg_value));

                soc_PKT_MARK_CTLr_field_get(unit, &reg_value, EN_ECN_REMARKf, &temp);

                if (temp) {
                    *enable = TRUE;
                } else {
                    *enable = FALSE;
                }
                break;
            }
            break;
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
            /* 
              * Enable/disable PCP/DEI remarking for TB (Global-based)
              */
            SOC_IF_ERROR_RETURN(REG_READ_QOS_CTLr(unit, &reg_value));

            soc_QOS_CTLr_field_get(unit, &reg_value, EN_PCP_REMARKf, &temp);

            if (temp) {
                *enable = TRUE;
            } else {
                *enable = FALSE;
            }
            break;

        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
            /* This feature currently is supported in bcm53115 and bcm53118 
             * chip only.
             */
        case DRV_PORT_PROP_EGRESS_C_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_S_PCP_REMARK:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
            break;
    }
    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_port_set
 *
 *  Purpose :
 *      Set the property to the specific ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      prop_type  :   port property type.
 *      prop_val    :   port property value.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_tbx_port_set(int unit, soc_pbmp_t bmp, 
        uint32 prop_type, uint32 prop_val)
{
    int     rv = SOC_E_NONE;
    uint32  reg32_value = 0, fld32_value = 0, temp = 0;
    uint64  reg64_value;
    int     port = 0, conf_port = 0;
    soc_pbmp_t tmp_pbmp;
    mac_driver_t *p_mac = NULL;    

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_tbx_port_set: unit=%d bmp=%x %x\n"), \
              unit, SOC_PBMP_WORD_GET(bmp, 0), SOC_PBMP_WORD_GET(bmp, 1)));

    SOC_ROBO_PORT_INIT(unit);
    switch (prop_type) {
        case DRV_PORT_PROP_SPEED:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_SPEED\n")));

            temp = 0;
            switch (prop_val) {
                case DRV_PORT_STATUS_SPEED_10M:
                    temp = 10;
                    break;
                case DRV_PORT_STATUS_SPEED_100M:
                    temp =100;
                    break;
                case DRV_PORT_STATUS_SPEED_1G:
                    temp = 1000;
                    break;
                case DRV_PORT_STATUS_SPEED_2500M:
                    temp = 2500;
                    break;
                default:
                    return SOC_E_PARAM;
            }
            PBMP_ITER(bmp, port) {
                /* set PHY and MAC auto-negotiation OFF */
                rv = soc_phyctrl_auto_negotiate_set(unit, port, FALSE);
             
                /* Set PHY registers anyway. */
                rv = soc_phyctrl_speed_set(unit, port, temp);
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    /* Set MAC registers anyway. */
                    if (p_mac->md_speed_set != NULL) {
                        rv = MAC_SPEED_SET(
                            p_mac, unit, port, temp);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;

        case DRV_PORT_PROP_DUPLEX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_DUPLEX\n")));
            switch (prop_val) {
                case DRV_PORT_STATUS_DUPLEX_HALF:
                    temp = FALSE;
                    break;
                case DRV_PORT_STATUS_DUPLEX_FULL:
                    temp =TRUE;
                    break;
                default:
                    return SOC_E_PARAM;
            }
            PBMP_ITER(bmp, port) {
                /* set PHY auto-negotiation OFF */
                rv = soc_phyctrl_auto_negotiate_set(unit, port, FALSE);

                /* Set PHY registers anyway. */
                rv = soc_phyctrl_duplex_set(unit, port, temp);

                /* set MAC duplex anyway*/
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    /* Set PHY registers anyway. */
                    if (p_mac->md_duplex_set != NULL) {
                        rv = MAC_DUPLEX_SET(
                           p_mac, unit, port, temp);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_RESTART_AUTONEG:
        case DRV_PORT_PROP_AUTONEG:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_AUTONEG\n")));
            PBMP_ITER(bmp, port) {
                /* set PHY auto-negotiation */
                rv = soc_phyctrl_auto_negotiate_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_TX_PAUSE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_TX_PAUSE\n")));
            PBMP_ITER(bmp, port) {
                /* set TX PAUSE */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_pause_set != NULL) {
                        rv = MAC_PAUSE_SET(
                            p_mac, unit, port, prop_val, -1);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_RX_PAUSE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_RX_PAUSE\n")));
            PBMP_ITER(bmp, port) {
                 /* set TX PAUSE */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_pause_set != NULL) {
                        rv = MAC_PAUSE_SET(
                            p_mac, unit, port, -1, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_LOCAL_ADVERTISE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: LOCAL_ADVER\n")));
            PBMP_ITER(bmp, port) {
                /* set advertise to PHY accordingly */
                rv = soc_phyctrl_adv_local_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_REMOTE_ADVERTISE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: REMOTE_ADVER not support\n")));
            /* can not set remote advert */
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_PORT_ABILITY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_PORT_ABILITY not support\n")));
            /* can not be set */
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_MAC_ABILITY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_MAC_ABILITY not support\n")));
            /* can not be set */
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_PHY_ABILITY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_PHY_ABILITY not support\n")));
            /* can not be set */
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_INTERFACE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_INTERFACE\n")));
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_interface_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_MAC_ENABLE:
            /* This case is called for _bcm_robo_port_update() only. */
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_MAC_ENABLE\n")));
            PBMP_ITER(bmp, port) {
                /* MAC register(s) should be set also */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_enable_set != NULL) {
                        rv = MAC_ENABLE_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_ENABLE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_ENABLE\n")));
            PBMP_ITER(bmp, port) {
                /* for enable, set MAC first and than PHY.
                 * for disable, set PHY first and than MAC
                 */
                if (prop_val) {
                    /* MAC register(s) should be set also */
                    p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                    if (p_mac != NULL) {
                        if (p_mac->md_enable_set != NULL) {
                            rv = MAC_ENABLE_SET(
                                p_mac, unit, port, prop_val);
                        } else {
                            rv = SOC_E_UNAVAIL;
                        }    
                    } else {
                        rv = SOC_E_PARAM;
                    }
                    SOC_IF_ERROR_RETURN(rv);
                    /* Set PHY registers anyway. */
                    rv = soc_phyctrl_enable_set(unit, port, prop_val);
                } else {
                    /* Set PHY registers anyway. */
                    rv = soc_phyctrl_enable_set(unit, port, prop_val);
                    /* MAC register(s) should be set also */
                    p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                    if (p_mac != NULL) {
                        if (p_mac->md_enable_set != NULL) {
                            rv = MAC_ENABLE_SET(
                                p_mac, unit, port, prop_val);
                        } else {
                            rv = SOC_E_UNAVAIL;
                        }
                    } else {
                        rv = SOC_E_PARAM;
                    }
                }

            }
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
        case DRV_PORT_PROP_ENABLE_DROP_UNTAG:
        case DRV_PORT_PROP_ENABLE_DROP_PRITAG:
        case DRV_PORT_PROP_ENABLE_DROP_1QTAG_IVM_HIT:
        case DRV_PORT_PROP_ENABLE_DROP_1QTAG_IVM_MISS:
        case DRV_PORT_PROP_DISABLE_LEARN:
        case DRV_PORT_PROP_SW_LEARN_MODE:
        case DRV_PORT_PROP_ENABLE_RX:
        case DRV_PORT_PROP_ENABLE_TX:
        case DRV_PORT_PROP_ENABLE_TXRX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_ENABLE_SET\n")));
            PBMP_ITER(bmp, port) {
                rv = _drv_tbx_port_property_enable_set(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
        case DRV_PORT_PROP_EGRESS_ECN_REMARK:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_EGRESS REMARKING\n")));
            PBMP_ITER(bmp, port) {
                rv = _drv_tbx_port_property_enable_set(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_IPG_FE:
        case DRV_PORT_PROP_IPG_GE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_IPG_FE/GE\n")));
            if (SOC_PBMP_EQ(bmp, PBMP_ALL(unit))) { /* per system */
                SOC_IF_ERROR_RETURN(REG_READ_SWMODEr(unit, &reg32_value));
                if (prop_val > 92) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                soc_SWMODEr_field_set(unit, &reg32_value, IPGf, &temp);
                SOC_IF_ERROR_RETURN(REG_WRITE_SWMODEr(unit, &reg32_value));
            }
            break;
        case DRV_PORT_PROP_JAM:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_JAM\n")));
            
            /* Robo chip use binding Jamming/Pause, Jamming/Pause can't be set
             * independent.
             */
            PBMP_ITER(bmp, port) {
                /* set TX PAUSE */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_pause_set != NULL) {
                        rv = MAC_PAUSE_SET(
                            p_mac, unit, port, prop_val, -1);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_BPDU_RX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_BPDU_RX\n")));
            if (SOC_PBMP_EQ(bmp, PBMP_ALL(unit))) {
                SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr(unit, &reg32_value));
                if (prop_val) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                soc_GMNGCFGr_field_set(unit, &reg32_value, 
                        RX_BPDU_ENf, &temp);
                SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr(unit, &reg32_value));
            }
            break;
        case DRV_PORT_PROP_MAC_LOOPBACK:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_MAC_LOOPBACK\n")));
            PBMP_ITER(bmp, port) {
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_lb_set != NULL) {
                        rv = MAC_LOOPBACK_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_PHY_LOOPBACK:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_PHY_LOOPBACK\n")));
            PBMP_ITER(bmp, port) {
                /* Special case :
                 *  - Design team confirmed that integrated FE PHY after 
                 *      robo5348 device will be limited to serve phy loopback
                 *      at 10MB speed.
                 */
#if ROBO_INTFE_NO_FE10_LOOPBACK_CONFIRMED
                if (IS_FE_PORT(unit, port)){
                    SOC_IF_ERROR_RETURN(
                            REG_READ_MIICTLr(unit, port, &reg32_value));
                    soc_MIICTLr_field_get(unit, &reg32_value, F_SPD_SELf, 
                            &temp);
                    if (!temp) {    /* F_SPD_SEL == b0 means speed=10 */
                        LOG_WARN(BSL_LS_SOC_COMMON,
                                 (BSL_META_U(unit,
                                             "No loopback on port%d for speed=10!\n"),
                                  port));
                        return SOC_E_UNAVAIL;
                    }
                }
#endif /* ROBO_INTFE_NO_FE10_LOOPBACK_CONFIRMED */
                rv = soc_phyctrl_loopback_set(unit, port, prop_val, TRUE);
            }
            break;
        case DRV_PORT_PROP_PHY_MEDIUM:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_PHY_MEDIUM not support\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_PHY_MDIX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_PHY_MDIX\n")));
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_mdix_set(unit, port, prop_val); 
            }
            break;
        case DRV_PORT_PROP_PHY_MDIX_STATUS:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_PHY_MDIX_STATUS not support\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MS:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_MS not support\n")));
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_master_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_SEC_MODE_NONE\n")));
            PBMP_ITER(bmp, port) {
                rv = _drv_tbx_port_security_mode_set(
                    unit, port, prop_type, 0);
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_ACCEPT:
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_REJECT:
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH:
        case DRV_PORT_PROP_SEC_MAC_MODE_EXTEND:
        case DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY:
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM:
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_ROAMING_OPT:
            if (prop_val & ~(DRV_SA_MOVE_ARL | DRV_SA_MOVE_CPU | 
                    DRV_SA_MOVE_DROP)){
                /* TB chips for Satation movement support ARL|CPU|DROP only */
                return SOC_E_UNAVAIL;
            }

            SOC_IF_ERROR_RETURN(REG_READ_SECURITY_GLOBAL_CTLr
                (unit, &reg32_value));
            SOC_IF_ERROR_RETURN(soc_SECURITY_GLOBAL_CTLr_field_get
                (unit, &reg32_value, ROAMING_OPTf, &fld32_value));
            /* 
             * Option to be used in the SA_OverLimit Filtering checking 
             * 0 : Not support address roaming. (default)
             * 1 : Support address roaming.
             *
             * P.S. SA learn count can't be reported properly once the 
             *     ROAMING_OPTf in SECURITY_GLOBAL_CTLr is not set when SA 
             *     move occurred.
             */
            if (prop_val & DRV_SA_MOVE_ARL) {
                temp = 1;
            } else {
                /* once the user assigned all ports to have no SA_MOVE_ARL 
                 * feature, clear the ROAMING_OPTf in SECURITY_GLOBAL_CTLr.
                 *
                 * P.S leave ROAMING_OPTf=1 won't impact the SA Move operation. 
                 *      the SA Move feature in TB is controled by port based.
                 */
                if (SOC_PBMP_EQ(bmp, PBMP_ALL(unit))){
                    temp = 0;
                }
            }
            /* check original setting */
            if (temp != fld32_value) {
                SOC_IF_ERROR_RETURN(soc_SECURITY_GLOBAL_CTLr_field_set
                    (unit, &reg32_value, ROAMING_OPTf, &temp));
    
                SOC_IF_ERROR_RETURN(REG_WRITE_SECURITY_GLOBAL_CTLr
                    (unit, &reg32_value));
            }

            temp = (~(PORT_CFG_EN_SA_MOVE_DROP | PORT_CFG_EN_SA_MOVE_CPUCOPY |
                    PORT_CFG_DIS_LRN_SA_MOVE));

            MEM_LOCK(unit, INDEX(PORT_MASKm));
            PBMP_ITER(bmp, port){
                rv = _drv_tbx_portmask_config_get(unit, port, 
                        _TB_PMASK_CONFIG_TYPE_RAW_VALUE, &fld32_value);
                _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));

                fld32_value &= temp;

                if (!(prop_val & DRV_SA_MOVE_ARL)) {
                    fld32_value |= PORT_CFG_DIS_LRN_SA_MOVE;
                }
                if (prop_val & DRV_SA_MOVE_CPU){
                    fld32_value |= PORT_CFG_EN_SA_MOVE_CPUCOPY;
                }
                
                if (prop_val & DRV_SA_MOVE_DROP){
                    fld32_value |= PORT_CFG_EN_SA_MOVE_DROP;
                }
                rv = _drv_tbx_portmask_config_set(unit, port, 
                        _TB_PMASK_CONFIG_TYPE_RAW_VALUE, fld32_value);
                _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));
            }
            MEM_UNLOCK(unit, INDEX(PORT_MASKm));

            break;
        case DRV_PORT_PROP_EAP_EN_CHK_OPT:
            /* System base, not per-port based (ingore port variable) */
            rv = _drv_tbx_port_security_mode_set(
                unit, -1, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_SA_MOVE_DROP:
        case DRV_PORT_PROP_SA_MOVE_CPUCOPY:
        case DRV_PORT_PROP_SA_UNKNOWN_DROP:
        case DRV_PORT_PROP_SA_UNKNOWN_CPUCOPY:
        case DRV_PORT_PROP_SA_OVERLIMIT_DROP:
        case DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PORT PROPERTY : SA VIOLATION DROP or CPUCOPY\n")));
            PBMP_ITER(bmp, port) {
                rv = _drv_tbx_port_property_enable_set(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP\n")));
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_set
                        (unit, port, DRV_PORT_PROP_SA_OVERLIMIT_DROP, TRUE));
                temp = ((int)prop_val < 0) ? 0 : prop_val;
                SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_COUNT_SET
                        (unit, port, DRV_PORT_SA_LRN_CNT_LIMIT, (int)temp));
            }
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU\n")));
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_set
                        (unit, port, 
                        DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY,TRUE));
                temp = ((int)prop_val < 0) ? 0 : prop_val;
                SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_COUNT_SET
                        (unit, port, DRV_PORT_SA_LRN_CNT_LIMIT, (int)temp));
            }
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_NONE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_L2_LEARN_LIMIT_PORT_ACTION_NONE\n")));
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_set
                        (unit, port, 
                        DRV_PORT_PROP_SA_OVERLIMIT_DROP, FALSE));
                SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_set
                        (unit, port, 
                        DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY, FALSE));
                /* clear learn limit */
                SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_COUNT_SET
                        (unit, port, DRV_PORT_SA_LRN_CNT_LIMIT, 0));
            }
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION\n"))); 
            /* tbx can be at action in one of NONE, CPU-COPY, CPU-REDIRECT and DROP */
            temp = (prop_val & DRV_PORT_LEARN_LIMIT_ACTION_MASK);
            PBMP_ITER(bmp, port) {
                if (temp == DRV_PORT_LEARN_LIMIT_ACTION_NONE) {
                    SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_set
                            (unit, port,
                            DRV_PORT_PROP_SA_OVERLIMIT_DROP, FALSE));
                    SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_set
                            (unit, port,
                            DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY, FALSE));
                    /* clear learn limit : 
                    *  - set limit to 0 to ensure the action-none can works 
                    *       properly.
                    */
                    SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_COUNT_SET
                            (unit, port, DRV_PORT_SA_LRN_CNT_LIMIT, 0));
                } else {
                    /* tbx in this section can only be drop or CPU-ONLY */
                    if (temp == DRV_PORT_LEARN_LIMIT_ACTION_DROP) {
                        SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_set
                                (unit, port, DRV_PORT_PROP_SA_OVERLIMIT_DROP, TRUE));
                        SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_set
                                (unit, port,
                                DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY, FALSE));
                    } else if (temp == DRV_PORT_LEARN_LIMIT_ACTION_COPY2CPU) {
                        SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_set
                                (unit, port, DRV_PORT_PROP_SA_OVERLIMIT_DROP, FALSE));
                        SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_set
                                (unit, port,
                                DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY, TRUE));
                    } else if (temp == DRV_PORT_LEARN_LIMIT_ACTION_REDIRECT2CPU) {
                        SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_set
                                (unit, port, DRV_PORT_PROP_SA_OVERLIMIT_DROP, TRUE));
                        SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_set
                                (unit, port,
                                DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY, TRUE));
                    }
                }
            }
            break; 
        case DRV_PORT_PROP_PHY_LINKUP_EVT:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_PHY_LINKUP_EVT\n")));
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_linkup_evt(unit, port);
                if (SOC_FAILURE(rv) && (rv != SOC_E_UNAVAIL)) {
                    return rv;
                }
                rv = SOC_E_NONE;
            }
            break;
        case DRV_PORT_PROP_PHY_LINKDN_EVT:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_PHY_LINKDN_EVT\n")));
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_linkdn_evt(unit, port);
                if (SOC_FAILURE(rv) && (rv != SOC_E_UNAVAIL)) {
                    return rv;
                }
                rv = SOC_E_NONE;
            }
            break;
        case DRV_PORT_PROP_PHY_RESET:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_PHY_RESET\n")));
            PBMP_ITER(bmp, port) {
                rv = soc_phy_reset(unit, port);
            }
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_DTAG_MODE\n")));
            rv = _drv_tbx_port_property_enable_set(
                unit, 0, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_DTAG_ISP_PORT\n")));
            PBMP_ITER(bmp, port) {
                rv = _drv_tbx_port_property_enable_set(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_DTAG_TPID:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_DTAG_TPID\n")));
            SOC_IF_ERROR_RETURN(REG_READ_ISP_TPIDr(unit, &reg32_value));
            soc_ISP_TPIDr_field_set(unit, &reg32_value, 
                    ISP_VLAN_DELIMITERf, &prop_val);
            SOC_IF_ERROR_RETURN(REG_WRITE_ISP_TPIDr(unit, &reg32_value));
            break;
        case DRV_PORT_PROP_802_1X_MODE :
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_802_1X_MODE\n")));
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_802_1X_BLK_RX :
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_802_1X_BLK_RX\n")));
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_MAC_BASE_VLAN:
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MAX_FRAME_SZ:
            PBMP_ITER(bmp, port) {
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_frame_max_set != NULL) {
                        rv = MAC_FRAME_MAX_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_INGRESS_VLAN_CHK:
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_set(unit, port, 
                        _TB_PMASK_CONFIG_TYPE_NON_MEMBER_DROP, prop_val));
            }
            break;
        case DRV_PORT_PROP_SFLOW_INGRESS_RATE:
            /* in TB's HW Spec. Ingress SFlow can be enabled/disabled 
             *  per port but all those enabled port referenced the same 
             *  ingress sflow rate configuration.
             *
             *  For the cross chip supporting and following the API guide's 
             *  definition, 0 will still been treat as the value to represent
             *  the disable value.
             *
             *  The CPU port for ingress SFLOW is not allowed to indicated.
             */
            
            PBMP_ITER(bmp, port) {
                if (IS_CPU_PORT(unit, port)){
                    return SOC_E_PARAM;
                }
            }
                
            rv = REG_READ_INGRESS_SFLOW_PORTr(unit, &reg32_value);
            rv |= soc_INGRESS_SFLOW_PORTr_field_get(unit, &reg32_value,
                    EN_INGRESS_PORTMAPf, &temp);
                    
            SOC_PBMP_WORD_SET(tmp_pbmp, 0, temp);
            
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "%s,current ing_sflow_en pbm=0x%x\n"),
                      FUNCTION_NAME(), temp));
            if ((int)prop_val > 0){
                /* enable the port's sFlow */
                SOC_PBMP_OR(tmp_pbmp, bmp);
            } else {
                /* disable the port's sFlow */
                SOC_PBMP_REMOVE(tmp_pbmp, bmp);
            }
            
            /* set ingress sflow enable/disable of the port */
            temp = SOC_PBMP_WORD_GET(tmp_pbmp, 0);
            rv |= soc_INGRESS_SFLOW_PORTr_field_set(unit, &reg32_value,
                    EN_INGRESS_PORTMAPf, &temp);
            rv |= REG_WRITE_INGRESS_SFLOW_PORTr(unit, &reg32_value);
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "%s,last ing_sflow_en pbm=0x%x\n"),
                      FUNCTION_NAME(), reg32_value));
            if (rv){
                break;
            }
               
            /* set ingress sflow rate */
            if (prop_val) {
                temp = drv_robo_port_sample_rate_get(unit, prop_val);

                LOG_INFO(BSL_LS_SOC_PORT,
                         (BSL_META_U(unit,
                                     "%s,setting user rate=%d, trans_rate=%d\n"),
                          FUNCTION_NAME(), prop_val, temp));
                if (!temp) {
                    /* 1. sample rate 1/1 is not supported. 
                     * 2. For chip no support case the temp will be 0.
                     */
                    return SOC_E_UNAVAIL;
                } else {
                    temp -= 1;
                }
                rv = REG_READ_INGRESS_SFLOWr(unit, &reg32_value);
                rv |= soc_INGRESS_SFLOWr_field_set(unit, &reg32_value,
                        INGRESS_CFGf, &temp);
                rv |= REG_WRITE_INGRESS_SFLOWr(unit, &reg32_value);
                
                /* update software copy of enabled ports. */
                PBMP_ITER(bmp, port) {
                    SOC_ROBO_PORT_INFO(unit, port).ing_sample_rate = prop_val;
                }        
                LOG_INFO(BSL_LS_SOC_PORT,
                         (BSL_META_U(unit,
                                     "%s, chip rate=%x\n"),
                          FUNCTION_NAME(), temp));

                /* 
                 * Then, update software copy of other enabled ports. 
                 * since all enabled ports share one configured value.
                 */
                PBMP_ITER(PBMP_ALL(unit), port) {
                    if (SOC_ROBO_PORT_INFO(unit, port).ing_sample_rate) {
                        SOC_ROBO_PORT_INFO(unit, port).ing_sample_rate = prop_val;
                    }
                }
            } else {
                /* update software copy of disabled ports. */
                PBMP_ITER(bmp, port) {
                    SOC_ROBO_PORT_INFO(unit, port).ing_sample_rate = prop_val;
                }
            }
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "%s, ingress sample rate=%d\n"),
                      FUNCTION_NAME(), prop_val));

            break;
        case DRV_PORT_PROP_SFLOW_EGRESS_RATE:
            /* in TB's HW Spec. Egress SFlow can be enabled/disabled */ 
            rv = REG_READ_EGRESS_SFLOWr(unit, &reg32_value);
            rv |= soc_EGRESS_SFLOWr_field_get(unit, &reg32_value,
                    EGRESS_PORTf, &temp);

            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "%s,current egr_sflow_port=%d, user rate=%x\n"),
                      FUNCTION_NAME(), temp, prop_val));
            /* 
             * If the port that desired to disable is not 
             * the previous enabled port, 
             * do nothing.
             */
            if ((!prop_val) && !SOC_PBMP_MEMBER(bmp, temp)) {
                return SOC_E_NONE;
            }
        
            if (!prop_val) {
                /* disable the port's sFlow */
                temp = 0;
            } else {
                /* enable the port's sFlow */
                temp = 1;
            }
            rv |= soc_EGRESS_SFLOWr_field_set(unit, &reg32_value,
                    EN_EGRESS_SFLOWf, &temp);
            
            /* set the egress sflow port (one port allowed only) */
            PBMP_ITER(bmp, port) {
                conf_port = port;
            }
            /* assign the latest port for egress sflow */
            rv |= soc_EGRESS_SFLOWr_field_set(unit, &reg32_value,
                    EGRESS_PORTf, (uint32 *)&conf_port);
             
            /* set the sflow rate */
            if (prop_val) {
                temp = drv_robo_port_sample_rate_get(unit, prop_val);
                if (!temp) {
                    /* 1. sample rate 1/1 is not supported. 
                     * 2. For chip no support case the temp will be 0.
                     */
                    return SOC_E_UNAVAIL;
                } else {
                    temp -= 1;
                }
                rv |= soc_EGRESS_SFLOWr_field_set(unit, &reg32_value,
                        EGRESS_CFGf, &temp);

                LOG_INFO(BSL_LS_SOC_PORT,
                         (BSL_META_U(unit,
                                     "%s,last egr_sflow_port=%d, chip rate=%x\n"),
                          FUNCTION_NAME(), conf_port, temp));
                /* 
                 * Clear software copy of all ports. 
                 * Since there is only one port can be 
                 * egress sample port at one time.
                 */
                PBMP_ITER(PBMP_ALL(unit), port) {
                    SOC_ROBO_PORT_INFO(unit, port).eg_sample_rate = 0;
                }

                /* Only update software copy of the configured port. */
                SOC_ROBO_PORT_INFO(unit, conf_port).eg_sample_rate = prop_val;
            } else {
                /* 
                 * Clear software copy of all ports. 
                 * Since there is only one port can be 
                 * egress sample port at one time.
                 */
                PBMP_ITER(PBMP_ALL(unit), port) {
                    SOC_ROBO_PORT_INFO(unit, port).eg_sample_rate = 0;
                }
            }
            rv |= REG_WRITE_EGRESS_SFLOWr(unit, &reg32_value);
            
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "%s, egress sample rate=%d\n"),
                      FUNCTION_NAME(), SOC_ROBO_PORT_INFO(unit, conf_port).eg_sample_rate));
            break;
        case DRV_PORT_PROP_SFLOW_INGRESS_PRIO:
        case DRV_PORT_PROP_SFLOW_EGRESS_PRIO:
        case DRV_PORT_PROP_MIB_CLEAR:
        case DRV_PORT_PROP_PPPOE_PARSE_EN:
            return SOC_E_UNAVAIL;

        case DRV_PORT_PROP_DEFAULT_TC_PRIO:
        case DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE:
        case DRV_PORT_PROP_UNTAG_DEFAULT_TC:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_set: PROP_DEFAULT_QOS CONFIGURATION\n")));
            PBMP_ITER(bmp, port) {
                rv = _drv_tbx_port_default_qos_config_set(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_PROFILE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: DRV_PORT_PROP_PROFILE\n")));
            /* valid check */
            SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET(
                    unit, DRV_DEV_PROP_PROFILE_NUM, &temp));
            if (prop_val >= temp){
                return SOC_E_PARAM;
            }
            
            PBMP_ITER(bmp, port) {
                rv = REG_READ_DEF_PORT_QOS_CFGr(unit, port, (void *)&reg64_value);
                temp = prop_val;
                rv |= soc_DEF_PORT_QOS_CFGr_field_set(unit, (void *)&reg64_value,
                        PORT_PROFILEf, &temp);
                rv |= REG_WRITE_DEF_PORT_QOS_CFGr(unit, port, (void *)&reg64_value);
                if (rv){
                    break;
                }
            }
            break;
        default: 
            rv = SOC_E_PARAM; 
        break;
    }

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_tbx_port_set: Exit\n")));
    return rv;
}

/*
 *  Function : drv_tbx_port_get
 *
 *  Purpose :
 *      Get the property to the specific ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      prop_type  :   port property type.
 *      prop_val    :   port property value.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int drv_tbx_port_get(int unit, int port, 
        uint32 prop_type, uint32 *prop_val)
{
    int  rv = SOC_E_NONE;
    uint32  reg32_value = 0, temp = 0;
    uint64  reg64_value;
    uint32  mac_ability = 0, phy_ability = 0;
    int  pause_tx = 0, pause_rx = 0;
    int  autoneg = 0, done = 0;
    soc_pbmp_t tmp_pbmp;
    mac_driver_t *p_mac = NULL;    

    SOC_ROBO_PORT_INIT(unit);
    p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
    switch (prop_type) {
        case DRV_PORT_PROP_SPEED:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: Speed\n")));
            temp = 0;

            /* redesigned for TB to report speed from MAC layer */
            if (p_mac != NULL) {
                if (p_mac->md_speed_get != NULL) {
                    rv = MAC_SPEED_GET(p_mac, 
                        unit, port, (int *)&temp);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(rv);
                    
            switch(temp) {
                case 10:
                    *prop_val = DRV_PORT_STATUS_SPEED_10M;
                    break;
                case 100:
                    *prop_val = DRV_PORT_STATUS_SPEED_100M;
                    break;
                case 1000:
                    *prop_val = DRV_PORT_STATUS_SPEED_1G;
                    break;
                case 2500:
                    *prop_val = DRV_PORT_STATUS_SPEED_2500M;
                    break;
                default:
                    *prop_val = 0;
                    break;
            }
            break;
        case DRV_PORT_PROP_DUPLEX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: Duplex\n")));
            temp = 0;

            /* redesigned for TB to report speed from MAC layer */
            if (p_mac != NULL) {
                if (p_mac->md_duplex_get != NULL) {
                    rv = MAC_DUPLEX_GET(p_mac, 
                        unit, port, (int *)&temp);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(rv);

            switch(temp) {
                case FALSE:
                    *prop_val = DRV_PORT_STATUS_DUPLEX_HALF;
                    break;
                case TRUE:
                    *prop_val = DRV_PORT_STATUS_DUPLEX_FULL;
                    break;
                default:
                    break;
            }
            break;
        case DRV_PORT_PROP_AUTONEG:     /* get PHY auto-neg */
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: Autoneg\n")));
            
            rv = soc_phyctrl_auto_negotiate_get(unit, port,
                                                    &autoneg, &done);
            *prop_val = (autoneg) ? DRV_PORT_STATUS_AUTONEG_ENABLE :
                        DRV_PORT_STATUS_AUTONEG_DISABLED;
            break;
        case DRV_PORT_PROP_TX_PAUSE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: TX Pause\n")));
            if (p_mac != NULL) {
                if (p_mac->md_pause_get != NULL) {
                    rv = MAC_PAUSE_GET(p_mac, 
                        unit, port, (int *) prop_val, &pause_rx);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_RX_PAUSE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: RX Pause\n")));
            if (p_mac != NULL) {
                if (p_mac->md_pause_get != NULL) {
                    rv = MAC_PAUSE_GET(p_mac, 
                        unit, port, &pause_tx, (int *) prop_val);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_LOCAL_ADVERTISE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: Local Advertise\n")));
            rv = soc_phyctrl_adv_local_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_REMOTE_ADVERTISE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: Remote Advertise\n")));
            /* if auto-negotiation is ON and negotiation is completed */
            /*   get remote advertisement from PHY */
            rv = soc_phyctrl_adv_remote_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PORT_ABILITY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: Port Ability\n")));
            rv = soc_phyctrl_ability_get(unit, port, &phy_ability);
            SOC_IF_ERROR_RETURN(rv);
            if (p_mac != NULL) {
                if (p_mac->md_ability_get != NULL) {
                    rv = MAC_ABILITY_GET(
                        p_mac, unit, port, &mac_ability);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }

            *prop_val  = mac_ability & phy_ability;
            *prop_val |= phy_ability & SOC_PM_ABILITY_PHY;
            break;
        case DRV_PORT_PROP_MAC_ABILITY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: MAC Ability\n")));
            if (p_mac != NULL) {
                if (p_mac->md_ability_get != NULL) {
                    rv = MAC_ABILITY_GET(
                        p_mac, unit, port, &mac_ability);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            *prop_val = mac_ability;
            break;
        case DRV_PORT_PROP_PHY_ABILITY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PHY Ability\n")));
            rv = soc_phyctrl_ability_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_INTERFACE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: Interface\n")));
            if (p_mac != NULL) {
                if (p_mac->md_interface_get != NULL) {
                    rv = MAC_INTERFACE_GET(
                        p_mac, unit, port, prop_val); 
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            } 
            break;
        case DRV_PORT_PROP_ENABLE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: Enable\n")));
            /* get PHY enable status only! (the same design flow with ESW) */
            rv = soc_phyctrl_enable_get(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
        case DRV_PORT_PROP_ENABLE_DROP_UNTAG:
        case DRV_PORT_PROP_ENABLE_DROP_PRITAG:
        case DRV_PORT_PROP_ENABLE_DROP_1QTAG_IVM_HIT:
        case DRV_PORT_PROP_ENABLE_DROP_1QTAG_IVM_MISS:
        case DRV_PORT_PROP_DISABLE_LEARN:
        case DRV_PORT_PROP_SW_LEARN_MODE:
        case DRV_PORT_PROP_ENABLE_RX:
        case DRV_PORT_PROP_ENABLE_TX:
        case DRV_PORT_PROP_ENABLE_TXRX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: Enable Get\n")));
            rv = _drv_tbx_port_property_enable_get(
                unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
        case DRV_PORT_PROP_EGRESS_ECN_REMARK:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_EGRESS REMARKING\n")));
            rv = _drv_tbx_port_property_enable_get(
                unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_IPG_FE:
        case DRV_PORT_PROP_IPG_GE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: IPG FE/GE\n")));
            SOC_IF_ERROR_RETURN(REG_READ_SWMODEr(unit, &reg32_value));
            soc_SWMODEr_field_get(unit, &reg32_value, IPGf, &temp);
            switch (temp) {
                case 0:
                    *prop_val = 92;
                    break;
                case 1:
                    *prop_val = 96;
                    break;
                default:
                    rv = SOC_E_INTERNAL;
                    break;
            }

            break;
        case DRV_PORT_PROP_JAM:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: JAM\n")));
            if (p_mac != NULL) {
                if (p_mac->md_pause_get != NULL) {
                    rv = MAC_PAUSE_GET(
                        p_mac, unit, port, (int *) prop_val,
                        &pause_rx);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            } 
            break;
        case DRV_PORT_PROP_BPDU_RX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: BPDU RX\n")));
            SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr(unit, &reg32_value));
            soc_GMNGCFGr_field_get(unit, &reg32_value, RX_BPDU_ENf, &temp);
            if (temp) {
                *prop_val = TRUE;
            } else {
                *prop_val = FALSE;
            }
            break;
        case DRV_PORT_PROP_RESTART_AUTONEG:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_RESTART_AUTONEG not support\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MAC_LOOPBACK:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_MAC_LOOPBACK\n")));
            if (p_mac != NULL) {
                if (p_mac->md_lb_get != NULL) {
                    rv = MAC_LOOPBACK_GET(
                        p_mac, unit, port, (int *) prop_val);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_PHY_LOOPBACK:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_PHY_LOOPBACK\n")));
            rv = soc_phyctrl_loopback_get(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_PHY_MEDIUM:
            rv = soc_phyctrl_medium_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PHY_MDIX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_PHY_MDIX\n")));
            rv = soc_phyctrl_mdix_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PHY_MDIX_STATUS:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_PHY_MDIX_STATUS\n")));
            rv = soc_phyctrl_mdix_status_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_MS:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_MS\n")));
            rv = soc_phyctrl_master_get(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_SEC_MODE_NONE\n")));
            rv = _drv_tbx_port_security_mode_get(
                unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_ACCEPT:
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_REJECT:
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH:
        case DRV_PORT_PROP_SEC_MAC_MODE_EXTEND:
        case DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY:
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM:
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_ROAMING_OPT:
            /* for TB's Station Move configuration, the major enable/disable
             *  features were controled in Port Mask table. Thus we redesign  
             *  this section for proper reported configuration.
             */
            SOC_IF_ERROR_RETURN(_drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_RAW_VALUE, &reg32_value));
            temp = reg32_value & (PORT_CFG_EN_SA_MOVE_DROP | 
                    PORT_CFG_EN_SA_MOVE_CPUCOPY | PORT_CFG_DIS_LRN_SA_MOVE);

            if (!(temp & PORT_CFG_DIS_LRN_SA_MOVE)) {
                *prop_val |= DRV_SA_MOVE_ARL;
            }
            if (temp & PORT_CFG_EN_SA_MOVE_DROP) {
                *prop_val |= DRV_SA_MOVE_DROP;
            }
            if (temp & PORT_CFG_EN_SA_MOVE_CPUCOPY) {
                *prop_val |= DRV_SA_MOVE_CPU;
            }
            
            break;
        case DRV_PORT_PROP_EAP_EN_CHK_OPT:
            /* System base, not per-port based (ingore port variable) */
            rv = _drv_tbx_port_security_mode_get(
                unit, -1, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_SA_MOVE_DROP:
        case DRV_PORT_PROP_SA_MOVE_CPUCOPY:
        case DRV_PORT_PROP_SA_UNKNOWN_DROP:
        case DRV_PORT_PROP_SA_UNKNOWN_CPUCOPY:
        case DRV_PORT_PROP_SA_OVERLIMIT_DROP:
        case DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PORT PROPERTY SA VIOLATION DROP or CPUCOPY\n")));
            rv = _drv_tbx_port_property_enable_get(
                unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP\n")));
            SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_get
                (unit, port, DRV_PORT_PROP_SA_OVERLIMIT_DROP, prop_val));
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU\n")));
            SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_get
                (unit, port, DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY, prop_val));
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_NONE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_L2_LEARN_LIMIT_PORT_ACTION_NONE\n")));
            /* check if the drop and toCPU action both not been enabled */
            SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_get
                (unit, port, DRV_PORT_PROP_SA_OVERLIMIT_DROP, &temp));
            if (!temp){
                SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_get
                    (unit, port, DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY, &temp));
            }
            /* means no learn limit action */
            *prop_val = (temp) ? FALSE : TRUE;
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION\n")));
            /* tbx can be at action in one of NONE, CPU-COPY, CPU-REDIRECT 
             *  and DROP. 
             */

            /* check if the drop and toCPU action both not been enabled */
            SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_get
                    (unit, port, DRV_PORT_PROP_SA_OVERLIMIT_DROP, &temp));
            if (temp == TRUE) {
                SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_get
                        (unit, port, DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY, &temp));
                if (temp == TRUE) {
                    /* means no learn limit action */
                    *prop_val = DRV_PORT_LEARN_LIMIT_ACTION_REDIRECT2CPU; 
                } else {
                    *prop_val = DRV_PORT_LEARN_LIMIT_ACTION_DROP; 
                }
            } else {
                SOC_IF_ERROR_RETURN(_drv_tbx_port_property_enable_get
                        (unit, port, DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY, &temp));
                if (temp == TRUE) {
                    /* means no learn limit action */
                    *prop_val = DRV_PORT_LEARN_LIMIT_ACTION_NONE;
                } else {
                    *prop_val = DRV_PORT_LEARN_LIMIT_ACTION_COPY2CPU;
                }
            }
            break; 

        case DRV_PORT_PROP_PHY_CABLE_DIAG:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_PHY_CABLE_DIAG\n")));
            rv = soc_phyctrl_cable_diag(unit, port, 
                                        (soc_port_cable_diag_t *)prop_val);
            break;
        case DRV_PORT_PROP_PHY_LINK_CHANGE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_PHY_LINKCHANGE\n")));
            rv = soc_phyctrl_link_change(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_DTAG_MODE\n")));
            rv = _drv_tbx_port_property_enable_get(unit, 0, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_DTAG_ISP_PORT\n")));
            rv = _drv_tbx_port_property_enable_get(unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_DTAG_TPID:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_DTAG_TPID\n")));
            SOC_IF_ERROR_RETURN(REG_READ_ISP_TPIDr(unit, &reg32_value));
            soc_ISP_TPIDr_field_get(unit, &reg32_value, ISP_VLAN_DELIMITERf, &temp);
            *prop_val = temp;
            break;
        case DRV_PORT_PROP_802_1X_MODE :
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_802_1X_MODE\n")));
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_802_1X_BLK_RX :
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_802_1X_BLK_RX\n")));
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_MAC_BASE_VLAN:
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MAX_FRAME_SZ:
            if (p_mac != NULL) {
                if (p_mac->md_frame_max_get != NULL) {
                    rv = MAC_FRAME_MAX_GET(
                        p_mac, unit, port, (int *) prop_val);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_INGRESS_VLAN_CHK:
            rv = _drv_tbx_portmask_config_get(unit, port, 
                    _TB_PMASK_CONFIG_TYPE_NON_MEMBER_DROP, prop_val);
            break;
        case DRV_PORT_PROP_SFLOW_INGRESS_RATE:
            /* in TB's HW Spec. Ingress SFlow can be enabled/disabled 
             *  per port but all those enabled port referenced the same 
             *  ingress sflow rate configuration.
             *
             *  Besides, TB's rate value is valid from 0x0 to 0xF and it is 
             *  differnet from previous ROBO chip(0 is invalud in 5348/53242).
             *
             *  For the cross chip supporting and following the API guide's 
             *  definition, 0 will still been treat as the value to represent
             *  the disable value.
             */
        
            /* get ingress sflow enable/disable of the port */
            if (IS_CPU_PORT(unit, port)){
                return SOC_E_PARAM;
            }
            rv = REG_READ_INGRESS_SFLOW_PORTr(unit, &reg32_value);
            rv |= soc_INGRESS_SFLOW_PORTr_field_get(unit, &reg32_value,
                    EN_INGRESS_PORTMAPf, &temp);
            SOC_PBMP_WORD_SET(tmp_pbmp, 0, temp);
            
            if (SOC_PBMP_MEMBER(tmp_pbmp, port)) {
                *prop_val = SOC_ROBO_PORT_INFO(unit, port).ing_sample_rate;
            } else {
                *prop_val = 0;
            }
            break;
        case DRV_PORT_PROP_SFLOW_EGRESS_RATE:
            /* in TB's HW Spec. Egress SFlow can be enabled/disabled.*/
        
            /* check egress sflow enable/disable of the port */
            rv = REG_READ_EGRESS_SFLOWr(unit, &reg32_value);
            rv |= soc_EGRESS_SFLOWr_field_get(unit, &reg32_value,
                    EN_EGRESS_SFLOWf, &temp);
            if (!temp) {
                /* disabled */
                *prop_val = 0;
                break;
            }
            rv |= soc_EGRESS_SFLOWr_field_get(unit, &reg32_value,
                    EGRESS_PORTf, &temp);
            
            /* get the sflow rate if sflow enabled on this port */
            if (temp != port) {
                /* disabled */
                *prop_val = 0;
            } else {
                /* enabled */
                *prop_val = SOC_ROBO_PORT_INFO(unit, port).eg_sample_rate;
            }
            break;
        case DRV_PORT_PROP_SFLOW_INGRESS_PRIO:
        case DRV_PORT_PROP_SFLOW_EGRESS_PRIO:
        case DRV_PORT_PROP_MIB_CLEAR:
        case DRV_PORT_PROP_PPPOE_PARSE_EN:
            return SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_DEFAULT_TC_PRIO:
        case DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE:
        case DRV_PORT_PROP_UNTAG_DEFAULT_TC:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: PROP_DEFAULT_QOS_CONFIGURATION\n")));
            rv = _drv_tbx_port_default_qos_config_get(
                unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_PROFILE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_get: DRV_PORT_PROP_PROFILE\n")));
            rv = REG_READ_DEF_PORT_QOS_CFGr(unit, port, (void *)&reg64_value);
            rv |= soc_DEF_PORT_QOS_CFGr_field_get(unit, (void *)&reg64_value, 
                    PORT_PROFILEf, &temp);
            *prop_val = temp;
            break;
        default: 
            return SOC_E_PARAM;
    }

    return rv;
}

/*
 *  Function : drv_tbx_port_status_get
 *
 *  Purpose :
 *      Get the status of the port for selected status type.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port number.
 *      status_type  :   port status type.
 *      vla     :   status value.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_tbx_port_status_get(int unit, uint32 port, uint32 status_type, uint32 *val)
{
    int     rv = SOC_E_NONE;
    uint32  reg_addr = 0, reg_value = 0, reg_len = 0;
    int     okay = 0;
    
    uint32  phy_medium = SOC_PORT_MEDIUM_COPPER;
    
    uint32  port_lb_phy = 0;
    int up = 0;
    int speed = 0;
    
    /* int_pd and pd used to prevent the runpacket issue on the GE port.
     * (with Internal SerDes bounded)
     */
    phy_ctrl_t      *int_pc = NULL, *ext_pc = NULL;  
    
    
    /* special process to detach port driver */
    if (status_type == DRV_PORT_STATUS_DETACH){
        *val = TRUE;
        rv = drv_robo_port_sw_detach(unit);
        if (rv < 0) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "Port detach failed!\n")));
            *val = FALSE;
        }
        LOG_INFO(BSL_LS_SOC_PORT,
                 (BSL_META_U(unit,
                             "drv_tbx_port_status_get: DETACH %s\n"),
                  *val ? "OK" : "FAIL"));
        return SOC_E_NONE;
    }
    
    /* To prevent the runpacket issue on the GE port which bounded 
     *  with internal SerDes and connected to an external PHY through SGMII.
     *  The linked information report will be designed to retrieved from   
     *  internal SerDes instead of Ext_PHY.
     *  (original design check external PHY only) 
     */
    if (IS_GE_PORT(unit, port)){
        int_pc = INT_PHY_SW_STATE(unit, port);
        ext_pc = EXT_PHY_SW_STATE(unit, port);
    }
     

    /* remarked for performance issue when debugging LinkScan
    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_port_status_get: unit = %d, port = %d\n"),
              unit, port));
    */
    switch (status_type)
    {
        case DRV_PORT_STATUS_LINK_UP:
            /* get the port loopback status (for Robo Chip)
             */
            rv = soc_phyctrl_loopback_get(unit, port, (int *) &port_lb_phy);

            if (port_lb_phy){
                LOG_INFO(BSL_LS_SOC_PORT,
                         (BSL_META_U(unit,
                                     "port%d at loopback status.\n"), port));
                reg_addr = DRV_REG_ADDR(unit, INDEX(LNKSTSr), 0, 0);
                reg_len = DRV_REG_LENGTH_GET(unit, INDEX(LNKSTSr));

                if ((rv = DRV_REG_READ(unit, reg_addr, &reg_value, 
                        reg_len)) < 0) {
                    return rv;
                }
                if (reg_value & (1 << port)) {
                    *val = TRUE;
                } else {
                    *val = FALSE;
                }
                
                return SOC_E_NONE;
            }
            
            rv = soc_phyctrl_link_get(unit, port, &up);
            
            /* Section to prevent the runpacket issue on the GE port */
            if (IS_GE_PORT(unit, port)){
                if ((int_pc != NULL) && (ext_pc != NULL)  && 
                        (ext_pc != int_pc)){
                    rv = PHY_LINK_GET(int_pc->pd, unit, port, (&up));
                }
            }

            if (rv < 0){
                *val = FALSE;
            } else {
                *val = (up) ? TRUE : FALSE;
            }  

           /* If link down, set default COPPER mode. */
            if (*val) {
                
                /* 1. GE port get the medium from PHY.
                 * 2. FE port on geting medium will got COPPER medium properly 
                 *    - for the PHY_MEDIUM_GET will direct to the mapping 
                 *      routine in phy.c
                 */
                (DRV_SERVICES(unit)->port_get) \
                    (unit, port, 
                    DRV_PORT_PROP_PHY_MEDIUM, &phy_medium);
            }
            else {
                phy_medium = SOC_PORT_MEDIUM_COPPER;
            }

            /* Program MAC if medium mode is changed. */
            if (phy_medium != SOC_ROBO_PORT_MEDIUM_MODE(unit, port)) {
                    
                    /* SW port medium update */
                    SOC_ROBO_PORT_MEDIUM_MODE_SET(unit, port, phy_medium);
            }

            break;
        case DRV_PORT_STATUS_LINK_SPEED:
            /* Section to prevent the runpacket issue on the GE port */
            if (IS_GE_PORT(unit, port)){
                if ((int_pc != NULL) && (ext_pc != NULL) && 
                        (ext_pc != int_pc)){
                    rv = PHY_SPEED_GET(int_pc->pd, unit, port, (&speed));
                } else {
                    rv = soc_phyctrl_speed_get(unit, port, &speed);
                }
            }

            if (rv){
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s, Can't get the PHY speed!\n"), FUNCTION_NAME())); 
                return rv;
            }
            
            *val = (speed == 2500) ? DRV_PORT_STATUS_SPEED_2500M :
                    (speed == 1000) ? DRV_PORT_STATUS_SPEED_1G :
                    (speed == 100) ? DRV_PORT_STATUS_SPEED_100M :
                                    DRV_PORT_STATUS_SPEED_10M;
                
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_status_get: SPEED = %d\n"),
                      *val));
            break;
        case DRV_PORT_STATUS_LINK_DUPLEX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_status_get: DUPLEX\n")));
            return SOC_E_RESOURCE;
        case DRV_PORT_STATUS_PROBE:
            *val = 0;
            rv = drv_robo_port_probe(unit, port, &okay);
            *val = okay;
            if (rv < 0) {
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "Port probe failed on port %s\n"),
                          SOC_PORT_NAME(unit, port)));
            }
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_status_get: PROBE %s\n"),
                      *val ? "OK" : "FAIL"));
            break;
        case DRV_PORT_STATUS_INIT:
            rv = drv_robo_port_sw_init_status_get(unit);
            if (rv == SOC_E_INIT) {
                *val = FALSE;
            } else {
                *val = TRUE;
            }
            break;
        case DRV_PORT_STATUS_PHY_DRV_NAME:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_tbx_port_status_get: PHY_DRV_NAME\n")));
            SOC_ROBO_PORT_INIT(unit);            
            
            *val = PTR_TO_INT(soc_phyctrl_drv_name(unit, port));
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 *  Function : drv_tbx_port_pri_mapop_set
 *
 *  Purpose :
 *      Port basis priority mapping operation configuration set
 *
 *  Parameters :
 *      unit        :   unit id
 *      port        :   port id.
 *      op_type     :   operation type
 *      pri_old     :   old priority.
 *      cfi_old     :   old cfi.
 *      pri_new     :   new priority (TC).
 *      cfi_new     :   new cfi (DP).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. This driver service is designed for priority operation exchange.
 *  2. Priority type could be dot1p, DSCP, port based.
 *
 */
int 
drv_tbx_port_pri_mapop_set(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 pri_new, uint32 cfi_new)
{
    pcp2dptc_entry_t  entry_pcp2dptc;
    dptc2pcp_entry_t  entry_dptc2pcp;
    uint32  field_val32;
    int rv;
    int  index = 0;

    sal_memset(&entry_pcp2dptc, 0, sizeof (entry_pcp2dptc));
    sal_memset(&entry_dptc2pcp, 0, sizeof (entry_dptc2pcp));

    switch (op_type) {
        case DRV_PORT_OP_PCP2TC :
            /* 1P-to-TCDP Port Mapping Table is indexed with
             * port[0:4] incoming priority[2:0] incoming CFI[0]
             */
            index = (port << 4) | 
                        ((pri_old & DOT1P_PRI_MASK) << 1) | 
                        (cfi_old & DOT1P_CFI_MASK);

            rv = MEM_READ_PCP2DPTCm(unit, index,  (uint32 *)&entry_pcp2dptc);
            SOC_IF_ERROR_RETURN(rv);

            field_val32 = pri_new & TC_VALID_MASK;
            rv = soc_PCP2DPTCm_field_set(unit,  (uint32 *)&entry_pcp2dptc, 
                PCP2TCf, &field_val32);
            SOC_IF_ERROR_RETURN(rv);

            field_val32 = cfi_new & DP_VALID_MASK;
            rv = soc_PCP2DPTCm_field_set(unit,  (uint32 *)&entry_pcp2dptc, 
                PCP2DPf, &field_val32);
            SOC_IF_ERROR_RETURN(rv);

            rv = MEM_WRITE_PCP2DPTCm(unit, index,  (uint32 *)&entry_pcp2dptc);
            SOC_IF_ERROR_RETURN(rv);

            break;
        case DRV_PORT_OP_NORMAL_TC2PCP:
            /* TCDP-to-1P Port Mapping Table is indexed with
             * port[0:4] CNG[1:0] priority[3:0] 
             */
            index = (port << 6) | 
                        ((cfi_old & DP_VALID_MASK) << 4) | 
                        (pri_old & TC_VALID_MASK);


            rv = MEM_READ_DPTC2PCPm(unit, index,  (uint32 *)&entry_dptc2pcp);
            SOC_IF_ERROR_RETURN(rv);
            
            field_val32 = cfi_new & DOT1P_CFI_MASK;
            rv = soc_DPTC2PCPm_field_set(unit, (uint32 *)&entry_dptc2pcp, 
                C_DEIf, &field_val32);
            SOC_IF_ERROR_RETURN(rv);


            field_val32 = pri_new & DOT1P_PRI_MASK;
            rv = soc_DPTC2PCPm_field_set(unit, (uint32 *)&entry_dptc2pcp, 
                C_PCPf, &field_val32);
            SOC_IF_ERROR_RETURN(rv);

            rv = MEM_WRITE_DPTC2PCPm(unit, index,  (uint32 *)&entry_dptc2pcp);
            SOC_IF_ERROR_RETURN(rv);

            break;
        case DRV_PORT_OP_OUTBAND_TC2PCP:
            /* TCDP-to-1P Port Mapping Table is indexed with
             * port[0:4] CNG[1:0] priority[3:0] 
             */
            index = (port << 6) | 
                        ((cfi_old & DP_VALID_MASK) << 4) | 
                        (pri_old & TC_VALID_MASK);

            rv = MEM_READ_DPTC2PCPm(unit, index,  (uint32 *)&entry_dptc2pcp);
            SOC_IF_ERROR_RETURN(rv);
            
            field_val32 = cfi_new & DOT1P_CFI_MASK;
            rv = soc_DPTC2PCPm_field_set(unit, (uint32 *)&entry_dptc2pcp, 
                S_DEIf, &field_val32);
            SOC_IF_ERROR_RETURN(rv);

            field_val32 = pri_new & DOT1P_PRI_MASK;
            rv = soc_DPTC2PCPm_field_set(unit, (uint32 *)&entry_dptc2pcp, 
                S_PCPf, &field_val32);
            SOC_IF_ERROR_RETURN(rv);

            rv = MEM_WRITE_DPTC2PCPm(unit, index,  (uint32 *)&entry_dptc2pcp);
            SOC_IF_ERROR_RETURN(rv);

            break;
        default:
            return SOC_E_UNAVAIL;
    }
    
    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_port_pri_mapop_get
 *
 *  Purpose :
 *      Port basis priority mapping operation configuration get
 *
 *  Parameters :
 *      unit        :   unit id
 *      port        :   port id.
 *      pri_old     :   (in)old priority.
 *      cfi_old     :   (in)old cfi.
 *      pri_new     :   (out)new priority (TC).
 *      cfi_new     :   (out)new cfi (DP).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. This driver service is designed for priority operation exchange.
 *  2. Priority type could be dot1p, DSCP, port based.
 *
 */
int 
drv_tbx_port_pri_mapop_get(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 *pri_new, uint32 *cfi_new)
{
    pcp2dptc_entry_t  entry_pcp2dptc;
    dptc2pcp_entry_t  entry_dptc2pcp;
    int  index = 0;
    int rv;

    sal_memset(&entry_pcp2dptc, 0, sizeof (entry_pcp2dptc));
    sal_memset(&entry_dptc2pcp, 0, sizeof (entry_dptc2pcp));

    switch (op_type) {
        case DRV_PORT_OP_PCP2TC :
            /* 1P-to-TCDP Port Mapping Table is indexed with
             * port[0:4] incoming priority[2:0] incoming CFI[0]
             */
            index = (port << 4) | 
                        ((pri_old & DOT1P_PRI_MASK) << 1) | 
                        (cfi_old & DOT1P_CFI_MASK);

            rv = MEM_READ_PCP2DPTCm(unit, index,  (uint32 *)&entry_pcp2dptc);
            SOC_IF_ERROR_RETURN(rv);

            rv = soc_PCP2DPTCm_field_get(unit,  (uint32 *)&entry_pcp2dptc, 
                PCP2TCf, pri_new);
            SOC_IF_ERROR_RETURN(rv);


            rv = soc_PCP2DPTCm_field_get(unit,  (uint32 *)&entry_pcp2dptc, 
                PCP2DPf, cfi_new);
            SOC_IF_ERROR_RETURN(rv);

            break;
        case DRV_PORT_OP_NORMAL_TC2PCP:
            /* TCDP-to-1P Port Mapping Table is indexed with
             * port[0:4] CNG[1:0] priority[3:0] 
             */
            index = (port << 6) | 
                        ((cfi_old & DP_VALID_MASK) << 4) | 
                        (pri_old & TC_VALID_MASK);

            rv = MEM_READ_DPTC2PCPm(unit, index,  (uint32 *)&entry_dptc2pcp);
            SOC_IF_ERROR_RETURN(rv);
            
            rv = soc_DPTC2PCPm_field_get(unit, (uint32 *)&entry_dptc2pcp, 
                C_DEIf, cfi_new);
            SOC_IF_ERROR_RETURN(rv);
            rv = soc_DPTC2PCPm_field_get(unit, (uint32 *)&entry_dptc2pcp, 
                C_PCPf, pri_new);
            SOC_IF_ERROR_RETURN(rv);

            break;
        case DRV_PORT_OP_OUTBAND_TC2PCP:
            /* TCDP-to-1P Port Mapping Table is indexed with
             * port[0:4] CNG[1:0] priority[3:0] 
             */
            index = (port << 6) | 
                        ((cfi_old & DP_VALID_MASK) << 4) | 
                        (pri_old & TC_VALID_MASK);

            rv = MEM_READ_DPTC2PCPm(unit, index,  (uint32 *)&entry_dptc2pcp);
            SOC_IF_ERROR_RETURN(rv);
            
            rv = soc_DPTC2PCPm_field_get(unit, (uint32 *)&entry_dptc2pcp, 
                S_DEIf, cfi_new);
            SOC_IF_ERROR_RETURN(rv);
            rv = soc_DPTC2PCPm_field_get(unit, (uint32 *)&entry_dptc2pcp, 
                S_PCPf, pri_new);
            SOC_IF_ERROR_RETURN(rv);

            break;
        default:
            return SOC_E_UNAVAIL;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_port_block_set
 *
 *  Purpose :
 *      Set the port based block types
 *
 *  Parameters :
 *      unit        :   unit id
 *      port        :   port id.
 *      block_type  :   (IN)port flood block type.
 *      egress_pbmp :   (IN) egress ports bitmap
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_tbx_port_block_set(int unit, int port, uint32 block_type,
        soc_pbmp_t egress_pbmp)
{
    uint32  type, value;
    int rv = SOC_E_NONE;
    
    SOC_ROBO_PORT_INIT(unit);
    
    value = SOC_PBMP_WORD_GET(egress_pbmp, 0);

    MEM_LOCK(unit,INDEX(PORT_MASKm));    

    if (block_type & DRV_BLOCK_ALL) {
        type = _TB_PMASK_MASK_TYPE_ANY;
    } else if (block_type & DRV_BLOCK_DLF_UCAST) {
        type = _TB_PMASK_MASK_TYPE_DLF_UCAST;
    } else if (block_type & DRV_BLOCK_DLF_MCAST) {
        /* special design for TB 
         *  - L2/L3 DLF Mcast to constract DLF Mcast
         */
        rv = _drv_tbx_portmask_mask_set(unit, port, 
                _TB_PMASK_MASK_TYPE_DLF_L2MCAST, value);
        _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));

        rv = _drv_tbx_portmask_mask_set(unit, port, 
                _TB_PMASK_MASK_TYPE_DLF_L3MCAST, value);
        _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));
        MEM_UNLOCK(unit,INDEX(PORT_MASKm));
        return SOC_E_NONE;
    } else if (block_type & DRV_BLOCK_BCAST) {
        type = _TB_PMASK_MASK_TYPE_BCAST;
    } else if (block_type & DRV_BLOCK_DLF_IP_MCAST) {
        type = _TB_PMASK_MASK_TYPE_DLF_L3MCAST;
    } else if (block_type & DRV_BLOCK_DLF_NONIP_MCAST) {
        type = _TB_PMASK_MASK_TYPE_DLF_L2MCAST;
    } else {
        MEM_UNLOCK(unit,INDEX(PORT_MASKm));
        return SOC_E_PARAM;
    }
    
    rv = _drv_tbx_portmask_mask_set(unit, port, 
            type, value);
    _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));

    MEM_UNLOCK(unit,INDEX(PORT_MASKm));

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_port_block_get
 *
 *  Purpose :
 *      Get the port based block types
 *
 *  Parameters :
 *      unit        :   unit id
 *      port        :   port id.
 *      block_type  :   (IN) port flood block type.
 *      egress_pbmp :   (OUT) egress ports bitmap
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_tbx_port_block_get(int unit, int port, uint32 block_type, 
        soc_pbmp_t *egress_pbmp)
{
    uint32  type, value, temp;
    int rv;
   
    SOC_ROBO_PORT_INIT(unit);
    
    SOC_PBMP_CLEAR(*egress_pbmp);

    MEM_LOCK(unit, INDEX(PORT_MASKm));
    if (block_type & DRV_BLOCK_ALL) {
        type = _TB_PMASK_MASK_TYPE_ANY;
    } else if (block_type & DRV_BLOCK_DLF_UCAST) {
        type = _TB_PMASK_MASK_TYPE_DLF_UCAST;
    } else if (block_type & DRV_BLOCK_DLF_MCAST) {
        /* special design for TB 
         *  - L2/L3 DLF Mcast to constract DLF Mcast
         */        
        rv = _drv_tbx_portmask_mask_get(unit, port, 
                _TB_PMASK_MASK_TYPE_DLF_L2MCAST, &value);
        _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));

        temp = value;
        rv = _drv_tbx_portmask_mask_get(unit, port, 
                _TB_PMASK_MASK_TYPE_DLF_L3MCAST, &value);
        _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));        
        SOC_PBMP_WORD_SET(*egress_pbmp, 0, (temp & value));
        MEM_UNLOCK(unit, INDEX(PORT_MASKm));        
        return SOC_E_NONE;
    } else if (block_type & DRV_BLOCK_BCAST) {
        type = _TB_PMASK_MASK_TYPE_BCAST;
    } else if (block_type & DRV_BLOCK_DLF_IP_MCAST) {
        type = _TB_PMASK_MASK_TYPE_DLF_L3MCAST;
    } else if (block_type & DRV_BLOCK_DLF_NONIP_MCAST) {
        type = _TB_PMASK_MASK_TYPE_DLF_L2MCAST;
    } else {
        MEM_UNLOCK(unit, INDEX(PORT_MASKm));        
        return SOC_E_PARAM;
    }
    
    rv = _drv_tbx_portmask_mask_get(unit, port, 
            type, &value);
    _ERROR_RETURN_UNLOCK_MEM(unit, rv, INDEX(PORT_MASKm));
    
    SOC_PBMP_WORD_SET(*egress_pbmp, 0, value);

    MEM_UNLOCK(unit, INDEX(PORT_MASKm));                
    return SOC_E_NONE;
}

