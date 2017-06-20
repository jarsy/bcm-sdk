/*
 * $Id: mem.c,v 1.14 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <sal/core/libc.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/drv.h>

#include <soc/l2x.h>
#include <soc/error.h>
#include "robo_tbx.h"

#include <soc/robo/arl.h>
#ifdef BCM_VO_SUPPORT
#include <soc/robo/voyager_service.h>
#endif /* BCM_VO_SUPPORT */

#define FIX_MEM_ORDER_E(v,m) (((m)->flags & SOC_MEM_FLAG_BE) ? \
    BYTES2WORDS((m)->bytes)-1-(v) : (v))

/* Local defintion for TBX devices memory control */
/* Memory index */
#define ARL_TABLE                   0x01 
#define MCAST_TABLE                 0x02
#define VLAN_TABLE                  0x03
#define MSPT_TABLE                  0x04
#define CFP_DATA_TABLE              0x10
#define CFP_ACTION_TABLE            0x11
#define CFP_METER_TABLE             0x12
#define CFP_STAT_TABLE              0x13
#define IVM_KEY_TABLE               0x20
#define IVM_ACTION_TABLE            0x21
#define EVM_KEY_TABLE               0x30
#define EVM_ACTION_TABLE            0x31
#define IRC_CONTROL_TABLE           0x40
#define ERC_CONTROL_TABLE           0x41
#define PCPTOTCDP_MAP_TABLE         0x50
#define TCDPTO1P_MAP_TABLE          0x51
#define PORTMASK_TABLE              0x52
#define SA_LERAN_CONF_TABLE         0x53
#define DSCPECN2TCDP_MAP_TABLE  0x54    
#define TCDP2DSCPECN_MAP_TABLE  0x55
#define MCAST_GROUP_VPORT_MAP_TABLE 0x60
#define VPORT_VID_MAP_TABLE         0x61

/* Memory Indication */
#define CFP_INDICATION_DATA_TABLE       (0x100 |CFP_DATA_TABLE)
#define CFP_INDICATION_MASK_TABLE       (0x200 | CFP_DATA_TABLE)
#define CFP_INDICATION_DATA_MASK_TABLE       (0x300 | CFP_DATA_TABLE)

#define TABLE(t) ( t & 0xFF)
#define TABLE_INDICATION(t)    (t & 0x300) 

/* Memory op code */
#define MEM_OP_NONE        0x00
#define MEM_OP_READ        0x01
#define MEM_OP_WRITE       0x02
#define MEM_OP_IDX_READ    0x03
#define MEM_OP_IDX_WRITE   0x04
#define MEM_OP_SCAN_VALID   0x5
#define MEM_OP_FILL        0x06
#define MEM_OP_MOVE        0x07
#define MEM_OP_WRITE_LIMIT 0x08
#define MEM_OP_RESET       0x09
#define MEM_OP_INC_ONE     0x0a
#define MEM_OP_DEC_ONE     0x0b


typedef struct drv_mem_info_s {
    uint32 drv_mem;
    int soc_mem;
    uint32 table_index;
    uint32 table_valid_mask;
}drv_mem_info_t;

static drv_mem_info_t drv_TB_mem_info[]={
        {DRV_MEM_VLAN, INDEX(VLAN_1Qm), VLAN_TABLE, -1},
        {DRV_MEM_MSTP, INDEX(MSPT_TABm), MSPT_TABLE, -1},
        {DRV_MEM_IRC_PORT, INDEX(IRC_PORTm), IRC_CONTROL_TABLE, -1},
        {DRV_MEM_ERC_PORT, INDEX(ERC_PORTm),ERC_CONTROL_TABLE, -1},   
        {DRV_MEM_1P_TO_TCDP, INDEX(PCP2DPTCm), PCPTOTCDP_MAP_TABLE, -1},
        {DRV_MEM_TCDP_TO_1P, INDEX(DPTC2PCPm),TCDPTO1P_MAP_TABLE, -1},   
        {DRV_MEM_PORTMASK, INDEX(PORT_MASKm), PORTMASK_TABLE, -1},
        {DRV_MEM_SALRN_CNT_CTRL, INDEX(SA_LRN_CNTm), SA_LERAN_CONF_TABLE, -1},
        {DRV_MEM_MCAST_VPORT_MAP, INDEX(MCAST_VPORT_MAPm), MCAST_GROUP_VPORT_MAP_TABLE, -1},
        {DRV_MEM_VPORT_VID_MAP, INDEX(VPORT_VID_MAPm), VPORT_VID_MAP_TABLE, -1},
        {DRV_MEM_ARL_HW, INDEX(L2_ARLm), ARL_TABLE, -1},
        {DRV_MEM_MARL_HW, INDEX(L2_MARLm), ARL_TABLE, -1},
        {DRV_MEM_ARL, INDEX(L2_ARL_SWm), ARL_TABLE, -1},
        {DRV_MEM_MARL, INDEX(L2_MARL_SWm), ARL_TABLE, -1},
        {DRV_MEM_MCAST, INDEX(MARL_PBMPm), MCAST_TABLE, -1},
        {DRV_MEM_IVM_KEY_DATA_MASK, INDEX(IVM_KEY_DATA_MASKm), IVM_KEY_TABLE, -1},
        {DRV_MEM_EVM_KEY_DATA_MASK, INDEX(EVM_KEY_DATA_MASKm), EVM_KEY_TABLE, -1},
        {DRV_MEM_IVM_ACT, INDEX(IVM_ACTm), IVM_ACTION_TABLE, -1},
        {DRV_MEM_EVM_ACT, INDEX(EVM_ACTm), EVM_ACTION_TABLE, -1},
        {DRV_MEM_CFP_DATA_MASK, INDEX(CFP_DATA_MASKm), CFP_DATA_TABLE, -1},
        {DRV_MEM_CFP_ACT, INDEX(CFP_ACT_POLm), CFP_ACTION_TABLE, -1},
        {DRV_MEM_CFP_METER, INDEX(CFP_METERm), CFP_METER_TABLE, -1},        
        {DRV_MEM_CFP_STAT, INDEX(CFP_STATm), CFP_STAT_TABLE, -1},
        {INDEX(CFP_TCAM_CHAIN_MASKm), INDEX(CFP_DATA_MASKm), CFP_DATA_TABLE, 0xFF00},
        {INDEX(CFP_TCAM_CHAIN_SCm), INDEX(CFP_DATA_MASKm),  CFP_DATA_TABLE, 0xFF},
        {INDEX(CFP_TCAM_IPV4_S0m), INDEX(CFP_DATA_MASKm),  CFP_DATA_TABLE, 0xFF},
        {INDEX(CFP_TCAM_IPV4_S1m), INDEX(CFP_DATA_MASKm),  CFP_DATA_TABLE, 0xFF},
        {INDEX(CFP_TCAM_IPV4_S2m), INDEX(CFP_DATA_MASKm),  CFP_DATA_TABLE, 0xFF},
        {INDEX(CFP_TCAM_IPV4_S0_MASKm), INDEX(CFP_DATA_MASKm),  CFP_DATA_TABLE, 0xFF00},
        {INDEX(CFP_TCAM_IPV4_S0_MASKm), INDEX(CFP_DATA_MASKm),  CFP_DATA_TABLE, 0xFF00},        
        {INDEX(CFP_TCAM_IPV4_S0_MASKm), INDEX(CFP_DATA_MASKm),  CFP_DATA_TABLE, 0xFF00},        
        {INDEX(CFP_TCAM_IPV6_S0m), INDEX(CFP_DATA_MASKm),  CFP_DATA_TABLE, 0xFF},
        {INDEX(CFP_TCAM_IPV6_S1m), INDEX(CFP_DATA_MASKm),  CFP_DATA_TABLE, 0xFF},
        {INDEX(CFP_TCAM_IPV6_S2m), INDEX(CFP_DATA_MASKm),  CFP_DATA_TABLE, 0xFF},
        {INDEX(CFP_TCAM_IPV6_S0_MASKm), INDEX(CFP_DATA_MASKm),  CFP_DATA_TABLE, 0xFF00},
        {INDEX(CFP_TCAM_IPV6_S0_MASKm), INDEX(CFP_DATA_MASKm),  CFP_DATA_TABLE, 0xFF00},        
        {INDEX(CFP_TCAM_IPV6_S0_MASKm), INDEX(CFP_DATA_MASKm),  CFP_DATA_TABLE, 0xFF00},        
        {INDEX(CFP_TCAM_NONIP_MASKm), INDEX(CFP_DATA_MASKm),  CFP_DATA_TABLE, 0xFF},
        {INDEX(CFP_TCAM_NONIP_SCm), INDEX(CFP_DATA_MASKm),  CFP_DATA_TABLE, 0xFF},
        {INDEX(IVM_KEY_DATA_0m), INDEX(IVM_KEY_DATA_MASKm), IVM_KEY_TABLE, 0x3},
        {INDEX(IVM_KEY_DATA_1m), INDEX(IVM_KEY_DATA_MASKm), IVM_KEY_TABLE, 0x3},
        {INDEX(IVM_KEY_DATA_2m), INDEX(IVM_KEY_DATA_MASKm), IVM_KEY_TABLE, 0x3},
        {INDEX(IVM_KEY_DATA_3m), INDEX(IVM_KEY_DATA_MASKm), IVM_KEY_TABLE, 0x3},
        {INDEX(IVM_KEY_MASK_0m), INDEX(IVM_KEY_DATA_MASKm), IVM_KEY_TABLE, 0xC},
        {INDEX(IVM_KEY_MASK_1m), INDEX(IVM_KEY_DATA_MASKm), IVM_KEY_TABLE, 0xC},
        {INDEX(IVM_KEY_MASK_2m), INDEX(IVM_KEY_DATA_MASKm), IVM_KEY_TABLE, 0xC},
        {INDEX(IVM_KEY_MASK_3m), INDEX(IVM_KEY_DATA_MASKm), IVM_KEY_TABLE, 0xC},
        {INDEX(EVM_KEY_DATAm), INDEX(IVM_KEY_DATA_MASKm), EVM_KEY_TABLE, 0x3},
        {INDEX(EVM_KEY_MASKm), INDEX(IVM_KEY_DATA_MASKm), EVM_KEY_TABLE, 0xC},
        {INDEX(DSCPECN2TCDPm), INDEX(DSCPECN2TCDPm),DSCPECN2TCDP_MAP_TABLE, -1},
        {INDEX(TCDP2DSCPECNm), INDEX(DSCPECN2TCDPm),TCDP2DSCPECN_MAP_TABLE, -1}
};

static drv_mem_info_t drv_TB_cfp_mem_info_B0[]={
        {DRV_MEM_CFP_METER, INDEX(CFP_RATE_METERm), CFP_METER_TABLE, -1}
};


#ifdef BCM_VO_SUPPORT
static drv_mem_info_t drv_VO_cfp_mem_info[]={
        {DRV_MEM_CFP_DATA_MASK, INDEX(CFP_DATA_MASKm), CFP_INDICATION_DATA_MASK_TABLE, -1},
        {DRV_MEM_TCAM_DATA, INDEX(CFP_DATAm), CFP_INDICATION_DATA_TABLE, -1},
        {DRV_MEM_TCAM_MASK, INDEX(CFP_MASKm), CFP_INDICATION_MASK_TABLE, -1},
        {INDEX(CFP_DATAm), INDEX(CFP_DATAm), CFP_INDICATION_DATA_TABLE, -1},            
        {INDEX(CFP_MASKm), INDEX(CFP_MASKm), CFP_INDICATION_MASK_TABLE, -1},            
        {INDEX(CFP_TCAM_SCm), INDEX(CFP_DATAm), CFP_INDICATION_DATA_TABLE, -1},
        {INDEX(CFP_TCAM_MASKm), INDEX(CFP_MASKm), CFP_INDICATION_MASK_TABLE, -1},
        {INDEX(CFP_TCAM_CHAIN_SCm), INDEX(CFP_DATAm), CFP_INDICATION_DATA_TABLE, -1},
        {INDEX(CFP_TCAM_CHAIN_MASKm), INDEX(CFP_MASKm), CFP_INDICATION_MASK_TABLE, -1}
};
#endif

static int _DATA[]={
INDEX(MEM_DATA_0r),
INDEX(MEM_DATA_1r),
INDEX(MEM_DATA_2r),
INDEX(MEM_DATA_3r),
INDEX(MEM_DATA_4r),
INDEX(MEM_DATA_5r),
INDEX(MEM_DATA_6r),
INDEX(MEM_DATA_7r)
};
static int _KEY[]={
INDEX(MEM_KEY_0r),
INDEX(MEM_KEY_1r),
INDEX(MEM_KEY_2r),
INDEX(MEM_KEY_3r),
INDEX(MEM_KEY_4r),
INDEX(MEM_KEY_5r),
INDEX(MEM_KEY_6r),
INDEX(MEM_KEY_7r)
};

#define MEM_DATA_READ(unit, index, val) \
    DRV_REG_READ(unit, DRV_REG_ADDR(unit, _DATA[index], 0, 0), val, DRV_REG_LENGTH_GET(unit, _DATA[index]))

#define MEM_DATA_WRITE(unit, index, val) \
    DRV_REG_WRITE(unit, DRV_REG_ADDR(unit, _DATA[index], 0, 0), val, DRV_REG_LENGTH_GET(unit, _DATA[index]))

#define MEM_KEY_READ(unit, index, val) \
    DRV_REG_READ(unit, DRV_REG_ADDR(unit, _KEY[index], 0, 0), val, DRV_REG_LENGTH_GET(unit, _KEY[index]))

#define MEM_KEY_WRITE(unit, index, val) \
    DRV_REG_WRITE(unit, DRV_REG_ADDR(unit,_KEY[index], 0, 0), val, DRV_REG_LENGTH_GET(unit, _KEY[index]))

/* MAC_ADDR field in l2 entry keeps bit12-bit47 only. 
 *  >> bit0-bit11 were mist!
 */
#define _TB_MIST_MACADDR_LENGTH     12
#define _TB_MIST_MACADDR_SHIFT      _TB_MIST_MACADDR_LENGTH
#define _TB_MIST_MACADDR_MASK       0xFFF       /* mask at bit0-bit11 */
#define _TB_KEPT_MACADDR_MASK       0xFFFFFFFFF /* mask at bit12-bit47 */

/* define for TB valid search */
#define _TB_ARL_BIN_NUMBER          4
#define _TB_ARL_VALID_SEARCH_OP     0x5     /* HW defined valid search op */
#define _TB_ARL_VALID_SEARCH_REPORT_VALID_MASK  0x1
#define _TB_ARL_VALID_SEARCH_REPORT_VALID_SHIFT 15
#define _TB_ARL_VALID_SEARCH_REPORT_TBID_MASK   0xFFF
#define _TB_ARL_VALID_SEARCH_REPORT_TBID_SHIFT  2
#define _TB_ARL_VALID_SEARCH_REPORT_BINID_MASK  0x3

/* the static bit position in the MEM_DATA0r after the ARL index_read */
#define _TB_ARL_KEY_SEARCH_RESULT_REGSHIFT_STATIC   60        
/* the valid field position in the MEM_DATA1r after the ARL index_read */
#define _TB_ARL_KEY_SEARCH_RESULT_REGMASK_VALID     0x3

/* the PORT field position in the MEM_DATA0r after the ARL index_read */
#define _TB_ARL_KEY_SEARCH_RESULT_SHIFT_PORT    36        
/* the PORT field length in the MEM_DATA0r after the ARL index_read */
#define _TB_ARL_KEY_SEARCH_RESULT_LENGTH_PORT   5        

/* the position in the MEM_DATA0r for indicating the Mcast MAC-address */
#define _TB_ARL_KEY_SEARCH_RESULT_MACADDR_BIT0  28        

/* supported fast aging feature mask */
#define _TB_ARL_FAST_AGING_TYPE_MASK         \
        (DRV_MEM_OP_DELETE_BY_PORT | DRV_MEM_OP_DELETE_BY_VLANID |  \
        DRV_MEM_OP_DELETE_BY_SPT | DRV_MEM_OP_DELETE_BY_TRUNK)
        
/* supported fast aging control mask */
#define _TB_ARL_FAST_AGING_CONTROL_MASK         \
        (DRV_MEM_OP_DELETE_BY_STATIC_ONLY | DRV_MEM_OP_DELETE_BY_STATIC | \
        DRV_MEM_OP_DELETE_BY_MCAST_ONLY | DRV_MEM_OP_DELETE_BY_MCAST)

#define _TB_ARL_FAST_AGE_MODE_PORT   0
#define _TB_ARL_FAST_AGE_MODE_VLAN   1
#define _TB_ARL_FAST_AGE_MODE_SPT    2
#define _TB_ARL_FAST_AGE_MODE_TRUNK  3

/* TB's new feature about Dual hash used "Least Full Selection" which 
 *      defined in TB's Appendix Spec.
 *      =============================================
 *      CRC32:D(bin3),C(bin2);  CRC16:B(bin1),A(bin0)
 *      #######################
 *      DC/BA   00  01  10  11
 *       00     A   C   C   C
 *       01     A   B   A   D
 *       10     A   B   A   C
 *       11     A   B   A   --
 *      #######################
 *  P.S. the formula below for "BA" will be revert to "ab" and "DC" will be  
 *      revert to "cd"
 */
#define _TB_ARL_DUAL_HASH_SELECTION(ab, cd)     \
        (((ab) == 0x0) ? 0 : (((cd) == 0) ? 2 :     \
        ((ab) == 0x1) ? 0 : ((ab) == 0x2) ? 1 :     \
        ((cd) == 0x1) ? 2 : 3))


int _tb_mem_modify(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 entry_mask, uint32 *entry);

#ifdef BCM_VO_SUPPORT 
static int 
_vo_cfp_mem_modify(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int i, j, mem_id;
    uint32 retry, index_min, index_max;
    uint32 table_index, temp, reg_val;
    uint32 field_change=0;
    int rv = SOC_E_NONE;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    soc_control_t           *soc = SOC_CONTROL(unit);
    int mem_len;
    uint64 temp_data;
    uint32 *temp_entry, *gmem_entry, *unchange_entry;
    uint32 val32_hi, val32_lo;
    int k=0; 
    int write_data =0;

    mem_id = -1;
    table_index = -1;
    for (i = 0; i < COUNTOF(drv_VO_cfp_mem_info); i++) {
        if ((drv_VO_cfp_mem_info[i].drv_mem == mem) || 
            (drv_VO_cfp_mem_info[i].soc_mem == mem)) {
            mem_id = drv_VO_cfp_mem_info[i].soc_mem;
            table_index = drv_VO_cfp_mem_info[i].table_index;
            break;
        }    
    }
    if ((mem == INDEX(CFP_TCAM_SCm)) || 
        (mem == INDEX(CFP_TCAM_MASKm)) ||
        (mem == INDEX(CFP_TCAM_CHAIN_SCm)) ||
        (mem == INDEX(CFP_TCAM_CHAIN_MASKm))) {
        field_change = 1;
    }
    if ((mem_id == -1) ||(table_index == -1)){
        return SOC_E_PARAM;
    }

    /* add code here to check addr */
    index_min = soc_robo_mem_index_min(unit, INDEX(CFP_DATA_MASKm));
    index_max = soc_robo_mem_index_max(unit, INDEX(CFP_DATA_MASKm));
    /* check the valid entry_id and the requested count */    
    if (((entry_id) < index_min) || 
            ((entry_id + count - 1) > index_max)) {
        if ((count < 1) || (((entry_id + count - 1) > index_max) && 
                ((entry_id) < index_max))){
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,mem_id=0x%x,entry_id=0x%x, invlaid count=%d\n"),
                      FUNCTION_NAME(), mem_id, entry_id, count));
        } else {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,mem_id=0x%x, invalid entry_id=0x%x\n"),
                      FUNCTION_NAME(), mem_id, entry_id));
        }
        return SOC_E_PARAM;
    }

    mem_len = soc_mem_entry_words(unit, INDEX(CFP_DATA_MASKm));

    entry_size = soc_mem_entry_bytes(unit, INDEX(CFP_DATAm));

    unchange_entry = (uint32 *)sal_alloc(entry_size,"mem_mod temp entry");
    if (unchange_entry == NULL) {
        return SOC_E_RESOURCE;
    }


    if (field_change == 1) {
        temp_entry = (uint32 *)sal_alloc(entry_size,"mem_read temp entry");
        if (temp_entry == NULL) {
            sal_free(unchange_entry);
            return SOC_E_RESOURCE;
        }
        rv = _drv_vo_cfp_mem_sw2hw(unit, entry, temp_entry);
        if (SOC_FAILURE(rv)) {
            sal_free(unchange_entry);
            sal_free(temp_entry);
            return rv;
        }
    } else {
        temp_entry = entry;
    }
    gmem_entry = entry;


    /* process write action */
    MEM_RWCTRL_REG_LOCK(soc);
    VO_ARL_SEARCH_LOCK(unit,soc);
    for (i = 0; i < count; i++) {
        /* Decide which table to be written. */
        reg_val = 0;
        temp = TABLE(table_index);
        soc_MEM_INDEXr_field_set(unit, &reg_val, INDEXf, &temp);
        rv = REG_WRITE_MEM_INDEXr(unit, &reg_val);
        if (rv < 0) {
            goto cfp_mem_mod_exit;
        }

        /* Set memory index. */
        reg_val = 0;        
        temp = entry_id + i;


        soc_MEM_ADDR_0r_field_set(unit, &reg_val, MEM_ADDR_OFFSETf, &temp);
        rv = REG_WRITE_MEM_ADDR_0r(unit, &reg_val);
            if (rv < 0) {
                goto cfp_mem_mod_exit;
            }            

        /* Read memory control register */
        reg_val = 0;
        temp = MEM_OP_READ;
        soc_MEM_CTRLr_field_set(unit, &reg_val, OP_CMDf, &temp);
        temp = 1;
        soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_STDNf, &temp);
        rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
        if (rv < 0) {
            goto cfp_mem_mod_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            rv = REG_READ_MEM_CTRLr(unit, &reg_val);
            if (rv < 0) {
                goto cfp_mem_mod_exit;
            }            
            soc_MEM_CTRLr_field_get(unit, &reg_val, MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto cfp_mem_mod_exit;
        }

        /* Decide which table to be written. */
        reg_val = 0;
        temp = TABLE(table_index);
        soc_MEM_INDEXr_field_set(unit, &reg_val, INDEXf, &temp);
        rv = REG_WRITE_MEM_INDEXr(unit, &reg_val);
        if (rv < 0) {
            goto cfp_mem_mod_exit;
        }

        val32_hi = 0;
        val32_lo = 0;

        /* write data */
        for (j = 0; j <  (mem_len + 1)/2; j++) {           
            if (j == 0){
                reg_val = 0;
                temp = 0;
                k = 0;
                soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_INDICATIONf, &temp);
                rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
                if (rv < 0) {
                    goto cfp_mem_mod_exit;
                }                      
            }
            if (j == 8){
                reg_val = 0;
                temp = 1;
                k = 0;
                soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_INDICATIONf, &temp);
                rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
                if (rv < 0) {
                    goto cfp_mem_mod_exit;
                }      
            }
            write_data = 1;
            if(table_index == CFP_INDICATION_MASK_TABLE){
                if (j < 8) {
                    write_data = 0;
                } else {
                    val32_lo = temp_entry[k];
                    k++;
                    val32_hi = temp_entry[k];
                    k++;
                }
            }else if(table_index == CFP_INDICATION_DATA_TABLE){
                if (j < 8) {
                    val32_lo = temp_entry[k];
                    k++;
                    val32_hi = temp_entry[k];
                    k++;
                } else {
                    write_data = 0;
                }
            } else {
                val32_lo = *entry;
                entry ++;
                val32_hi = *entry;
                entry ++;
            }
            COMPILER_64_SET(temp_data, val32_hi, val32_lo);

            temp = j%8;
            if (write_data) {
                rv = MEM_DATA_WRITE(unit, temp, &temp_data);
                if (rv < 0) {
                    goto cfp_mem_mod_exit;
                }
            }
        }

        /* Decide which table to be written. */
        reg_val = 0;
        temp = TABLE(table_index);
        soc_MEM_INDEXr_field_set(unit, &reg_val, INDEXf, &temp);
        rv = REG_WRITE_MEM_INDEXr(unit, &reg_val);
        if (rv < 0) {
            goto cfp_mem_mod_exit;
        }
        /* Write memory control register */
        reg_val = 0;
        temp = MEM_OP_WRITE;
        soc_MEM_CTRLr_field_set(unit, &reg_val, OP_CMDf, &temp);
        temp = 1;
        soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_STDNf, &temp);
        rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
        if (rv < 0) {
            goto cfp_mem_mod_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            rv = REG_READ_MEM_CTRLr(unit, &reg_val);
            if (rv < 0) {
                goto cfp_mem_mod_exit;
            }
            soc_MEM_CTRLr_field_get(unit, &reg_val, MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto cfp_mem_mod_exit;
        }
        
        /* Write back to cache if active */
        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL) {
            entry_size = soc_mem_entry_bytes(unit, mem_id);
            sal_memcpy(cache + (entry_id + i) * entry_size, gmem_entry, 
                    entry_size);
            CACHE_VMAP_SET(vmap, (entry_id + i));
        }

    }

    reg_val = 0;
    rv = REG_WRITE_MEM_INDEXr(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto cfp_mem_mod_exit;
    }

cfp_mem_mod_exit:
VO_ARL_SEARCH_UNLOCK(unit,soc);

MEM_RWCTRL_REG_UNLOCK(soc);
if (field_change == 1) {
    sal_free(temp_entry);
}
sal_free(unchange_entry);

return rv;
}

static int
_vo_cfp_mem_fill(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int j, mem_id;
    uint32 retry, index_min, index_max;
    uint32 table_index, temp, reg_val;
    int rv = SOC_E_NONE;
    soc_control_t           *soc = SOC_CONTROL(unit);
    int mem_len;
    uint64 temp_data, reg64;
    uint32 val32_hi, val32_lo;

    mem_id = -1;
    table_index = -1;
    if (mem != DRV_MEM_CFP_DATA_MASK) {
        return SOC_E_UNAVAIL;
    }
    table_index = CFP_INDICATION_DATA_MASK_TABLE;
    mem_id = INDEX(CFP_DATA_MASKm);

    /* add code here to check addr */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    /* check the valid entry_id and the requested count */    
    if (((entry_id) < index_min) || 
            ((entry_id + count - 1) > index_max)) {
        if ((count < 1) || (((entry_id + count - 1) > index_max) && 
                ((entry_id) < index_max))){
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,mem_id=0x%x,entry_id=0x%x, invlaid count=%d\n"),
                      FUNCTION_NAME(), mem_id, entry_id, count));
        } else {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,mem_id=0x%x, invalid entry_id=0x%x\n"),
                      FUNCTION_NAME(), mem_id, entry_id));
        }
        return SOC_E_PARAM;
    }

    mem_len = soc_mem_entry_words(unit, INDEX(CFP_DATA_MASKm));
    MEM_RWCTRL_REG_LOCK(soc);
    VO_ARL_SEARCH_LOCK(unit,soc);    
    /* Decide which table. */
    reg_val = 0;
    temp = TABLE(table_index);
    soc_MEM_INDEXr_field_set(unit, &reg_val, INDEXf, &temp);
    rv = REG_WRITE_MEM_INDEXr(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto cfp_mem_fill_exit;
    }
    /* Set memory index. */
    reg_val = 0;
    temp = entry_id;
    soc_MEM_ADDR_0r_field_set(unit, &reg_val, MEM_ADDR_OFFSETf, &temp);
    rv = REG_WRITE_MEM_ADDR_0r(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto cfp_mem_fill_exit;
    }
    /* how many entries to fill*/
    COMPILER_64_ZERO(temp_data);
    COMPILER_64_ZERO(reg64);
    val32_hi = 0;
    val32_lo = count;
    COMPILER_64_SET(temp_data, val32_hi, val32_lo);
    soc_MEM_KEY_0r_field_set(unit, (void *)&reg64, MEM_KEYf, (void *)&temp_data);
    rv = REG_WRITE_MEM_KEY_0r(unit, &reg64);
    if (SOC_FAILURE(rv)) {
        goto cfp_mem_fill_exit;
    }

    val32_hi = 0;
    val32_lo = 0;
    /* Fill the patterns */
    for (j = 0; j <  (mem_len + 1)/2; j++) {
        if (j == 0){
            reg_val = 0;
            temp = 0;
            soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_INDICATIONf, &temp);
            rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
            if (rv < 0) {
                goto cfp_mem_fill_exit;
            }                      
        }
        if (j == 8){
            reg_val = 0;
            temp = 1;
            soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_INDICATIONf, &temp);
            rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
            if (rv < 0) {
                goto cfp_mem_fill_exit;
            }      
        }

        val32_lo = *entry;
        if(mem_len > (2*j+1) ){
            entry ++;
            val32_hi = *entry;
        } else {
            val32_hi = 0;
        }

        COMPILER_64_SET(temp_data, val32_hi, val32_lo);
        entry++;            
        temp = j%8;

        rv = MEM_DATA_WRITE(unit, temp, &temp_data);
        if (SOC_FAILURE(rv)) {
            goto cfp_mem_fill_exit;
        }
    }

    /* Write memory control register */
    reg_val = 0;
    temp = MEM_OP_FILL;
    soc_MEM_CTRLr_field_set(unit, &reg_val, OP_CMDf, &temp);
    temp = 1;
    soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_STDNf, &temp);

    rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto cfp_mem_fill_exit;
    }
    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        rv = REG_READ_MEM_CTRLr(unit, &reg_val);
        if (SOC_FAILURE(rv)) {
            goto cfp_mem_fill_exit;
        }        
        soc_MEM_CTRLr_field_get(unit, &reg_val, MEM_STDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        if (SOC_FAILURE(rv)) {
            goto cfp_mem_fill_exit;
        }
    }

    reg_val = 0;
    rv = REG_WRITE_MEM_INDEXr(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto cfp_mem_fill_exit;
    }

cfp_mem_fill_exit:
VO_ARL_SEARCH_UNLOCK(unit,soc);
MEM_RWCTRL_REG_UNLOCK(soc);
return rv;

}

#endif /* BCM_VO_SUPPORT */

static int 
_drv_tb_arl_entry_sw2hw(int unit, 
        l2_arl_sw_entry_t *sw_arl, l2_arl_entry_t *hw_arl)
{
    int     rv = SOC_E_NONE, is_marl = 0;
    uint32  temp = 0;
    uint64  full_mac, short_mac;
    sal_mac_addr_t  temp_mac;
    
    assert(sw_arl && hw_arl);
    
    sal_memset(hw_arl, 0, sizeof(l2_arl_entry_t));
    COMPILER_64_ZERO(full_mac);
    COMPILER_64_ZERO(short_mac);
    
    /* set MAC */
    rv = soc_L2_ARL_SWm_field_get(unit, (void *)sw_arl, 
        MACADDRf, (void *)&full_mac);
    SOC_IF_ERROR_RETURN(rv);

    SAL_MAC_ADDR_FROM_UINT64(temp_mac, full_mac);
    is_marl = (*(uint8 *)&temp_mac) & 0x1;  /* check mac_addr bit0 */
                
    sal_memcpy(&short_mac, &full_mac, sizeof(uint64));
    COMPILER_64_SHR(short_mac, _TB_MIST_MACADDR_LENGTH);
                
    rv = soc_L2_ARLm_field_set(unit, (void *)hw_arl, 
        MACADDRf, (void *)&short_mac);
    SOC_IF_ERROR_RETURN(rv);

    /* set vport/port or MGID */
    if (is_marl){
        rv = soc_L2_MARL_SWm_field_get(unit, (void *)sw_arl, 
            MGIDf, &temp);
        SOC_IF_ERROR_RETURN(rv);        
        rv = soc_L2_MARLm_field_set(unit, (void *)hw_arl, 
            MGIDf, &temp);
        SOC_IF_ERROR_RETURN(rv);        

        SOC_IF_ERROR_RETURN(rv);
    } else {
        rv = soc_L2_ARL_SWm_field_get(unit, (void *)sw_arl, 
            V_PORTIDf, &temp);
        SOC_IF_ERROR_RETURN(rv);
        rv = soc_L2_ARLm_field_set(unit, (void *)hw_arl, 
            V_PORTIDf, &temp);
        SOC_IF_ERROR_RETURN(rv);
        rv = soc_L2_ARL_SWm_field_get(unit, (void *)sw_arl, 
            PORTIDf, &temp);
        SOC_IF_ERROR_RETURN(rv);
        rv = soc_L2_ARLm_field_set(unit, (void *)hw_arl, 
            PORTIDf, &temp);
        SOC_IF_ERROR_RETURN(rv);
    }
    
    /* vid */
    rv = soc_L2_ARL_SWm_field_get(unit, (void *)sw_arl, 
        VIDf, &temp);
    SOC_IF_ERROR_RETURN(rv);
    rv = soc_L2_ARLm_field_set(unit, (void *)hw_arl, 
        VIDf, &temp);
    SOC_IF_ERROR_RETURN(rv);
    /* static */
    rv = soc_L2_ARL_SWm_field_get(unit, (void *)sw_arl, 
        STATICf, &temp);
    SOC_IF_ERROR_RETURN(rv);
    rv = soc_L2_ARLm_field_set(unit, (void *)hw_arl, 
        STATICf, &temp);
    SOC_IF_ERROR_RETURN(rv);
    /* age */  
    rv = soc_L2_ARL_SWm_field_get(unit, (void *)sw_arl, 
        AGEf, &temp);
    SOC_IF_ERROR_RETURN(rv);
    rv = soc_L2_ARLm_field_set(unit, (void *)hw_arl, 
        AGEf, &temp);
    SOC_IF_ERROR_RETURN(rv);
    /* set valid */
    rv = soc_L2_ARL_SWm_field_get(unit, (void *)sw_arl, 
        VAf, &temp);
    SOC_IF_ERROR_RETURN(rv);
    rv = soc_L2_ARLm_field_set(unit, (void *)hw_arl, 
        VAf, &temp);
    SOC_IF_ERROR_RETURN(rv);
    /* set USER_REV */
    rv = soc_L2_ARL_SWm_field_get(unit, (void *)sw_arl, 
        USERf, &temp);
    SOC_IF_ERROR_RETURN(rv);
    rv = soc_L2_ARLm_field_set(unit, (void *)hw_arl, 
        USERf, &temp);
    if (soc_feature(unit, soc_feature_arl_mode_control)) {
        rv = soc_L2_ARL_SWm_field_get(unit, (void *)sw_arl, 
            CONf, &temp);
        SOC_IF_ERROR_RETURN(rv);
        rv = soc_L2_ARLm_field_set(unit, (void *)hw_arl, 
            CONf, &temp);
    }
    return rv;
}

#define _TB_ARL_SEARCH_MATCH        0
#define _TB_ARL_SEARCH_CONFLICT     1
/*
 *  Function : _drv_tbx_search_key_op
 *
 *  Purpose :
 *      Performing the ARL search operation about match based or conflict based 
 *      search processes. Both search mechanism use MAC+VID to be the searching 
 *      key in ARL table.
 *      - The match based search : to find the matched MAV+VID l2 entry.
 *      - The conflict based search : to find all valid entries in the MAC+VID 
 *              hashed bucket.
 *
 *  Return :
 *      Normal return in those two search operations are :
 *      - The match based search : SOC_E_EXISTS, SOC_E_FULL, SOCE_NOT_FOUND
 *          >> SOC_E_FULL: all entries in this bucket are valid but not matched
 *          >> SOCE_NOT_FOUND : bucket not full and not matched
 *      - The conflict based search : SOC_E_EXISTS, SOCE_NOT_FOUND
 *          >> SOCE_NOT_FOUND : no valid entry in the hashed bucket.
 *
 *  Note :
 *  1. No SOC_E_NONE return!
 *  2. result_entry for conflict search is the entry array pointer.
 *  3. arl_index return value : 
 *      - for 'Match based search'
 *          a. match entry found (matched MAC+VID)    
 *              >> table_index(16K) = entry_id(4K) | bin_id(matched one)
 *          b. No match entry found (no matched MAC+VID)
 *              >> table_index(16K) = entry_id(4K) | bin_id(SW choice)
 *      - for 'Conflict based search' : will be array pointer
 *          a. Conflict entry found : keep the table_index(16K basis)
 *          b. No conflict entry found : keep value at -1
 */
static int
_drv_tbx_search_key_op(int unit, uint32 *sw_arl, uint32 int_flag,
        uint32 *result_entry, int  *arl_index)
{
    int         rv = SOC_E_NOT_FOUND;
    uint64      mac_key, mac_result, temp_reg_val, vid_reg_val;
    uint32      vid_key = 0, vid_result = 0, reg_val = 0, temp = 0;
    uint32      eid_crc16 = 0, eid_crc32 = 0;
    uint32      temp_hi = 0 ,temp_lo = 0;
    int         i, table_index = 0, valid_cnt = 0, is_conflict = 0;
    uint32      mac_lsb = 0;
    uint8       ab_status = 0, cd_status = 0;   /* bin0~bin3 valid status */
    l2_arl_sw_entry_t   temp_sw_arl, *rep_entry;
    
    COMPILER_64_ZERO(mac_key);
    COMPILER_64_ZERO(mac_result);
    COMPILER_64_ZERO(temp_reg_val);
    COMPILER_64_ZERO(vid_reg_val);
    
    /* valid check */
    assert(sw_arl && result_entry && arl_index);
    if (int_flag == _TB_ARL_SEARCH_CONFLICT){
        for (i = 0; i < ROBO_TBX_L2_BUCKET_SIZE; i++){
            rep_entry = (l2_arl_sw_entry_t *)result_entry + i;
            if (rep_entry == NULL){
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s,entries buffer not allocated!\n"), 
                          FUNCTION_NAME()));
                return SOC_E_PARAM;
            }
        }
    }

    /* -- get the MAC and VID for search -- 
     *  1. check SVL/IVL mode first for the VID desicion.
     */
    rv = soc_L2_ARL_SWm_field_get(unit, (void *)sw_arl, 
        MACADDRf, (void *)&mac_key);
    SOC_IF_ERROR_RETURN(rv);

    rv = soc_L2_ARL_SWm_field_get(unit, (void *)sw_arl, 
        VIDf, (uint32 *)&vid_key);
    SOC_IF_ERROR_RETURN(rv);
    
    rv = REG_READ_VLAN_GLOBAL_CTLr(unit, &reg_val);
    SOC_IF_ERROR_RETURN(rv);
    
    rv = soc_VLAN_GLOBAL_CTLr_field_get(
            unit, &reg_val, VID_MAC_CTRLf, &temp);
    SOC_IF_ERROR_RETURN(rv);    
    if (!temp){
        /* means device is at SVL mode, VID will be zero */
        LOG_INFO(BSL_LS_SOC_ARL,
                 (BSL_META_U(unit,
                             "%s,%d,SVL mode,vid=%d reset to vid=0\n"),
                  FUNCTION_NAME(),__LINE__,vid_key));
        vid_key = 0;
    }

    /* -- preparing the TB's "index_read" op -- */
    /* set tb_index */
    rv = REG_READ_MEM_INDEXr(unit, &reg_val);
    SOC_IF_ERROR_RETURN(rv);    
    
    table_index = ARL_TABLE;
    rv = soc_MEM_INDEXr_field_set(unit, &reg_val, 
            INDEXf, (uint32 *)&table_index);
    SOC_IF_ERROR_RETURN(rv);        

    rv = REG_WRITE_MEM_INDEXr(unit, &reg_val);
    SOC_IF_ERROR_RETURN(rv);        

    /* set searching keys */
    rv = REG_WRITE_MEM_KEY_0r(unit, &mac_key);
    SOC_IF_ERROR_RETURN(rv);    
    
    COMPILER_64_ZERO(vid_reg_val);
    temp_lo = COMPILER_64_LO(vid_reg_val);
    COMPILER_64_SET(vid_reg_val, 0, temp_lo | vid_key);
    rv = REG_WRITE_MEM_KEY_1r(unit, &vid_reg_val);
    SOC_IF_ERROR_RETURN(rv);    
    
    /* -- start index_read op and check if read is done -- */
    reg_val = 0;
    temp = MEM_OP_IDX_READ;
    rv = soc_MEM_CTRLr_field_set(unit, &reg_val, 
            OP_CMDf, &temp);
    SOC_IF_ERROR_RETURN(rv);    

    temp = 1;
    rv = soc_MEM_CTRLr_field_set(unit, &reg_val, 
            MEM_STDNf, &temp);
    SOC_IF_ERROR_RETURN(rv);    

    REG_WRITE_MEM_CTRLr(unit, &reg_val);
    SOC_IF_ERROR_RETURN(rv);    
    
    /* wait for complete */
    for (i = 0; i < SOC_TIMEOUT_VAL; i++) {
        rv = REG_READ_MEM_CTRLr(unit, &reg_val);
        SOC_IF_ERROR_RETURN(rv);    

        rv = soc_MEM_CTRLr_field_get(unit, &reg_val, 
                MEM_STDNf, &temp);
        SOC_IF_ERROR_RETURN(rv);    
        if (!temp) {
            break;
        }
    }
    if (i >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto mem_search_exit;
    }

    /* Entry index(4k basis) from dual hashed index read */
    rv = REG_READ_MEM_ADDR_0r(unit, &reg_val);
    SOC_IF_ERROR_RETURN(rv);    
    rv = soc_MEM_ADDR_0r_field_get(unit, &reg_val, 
            MEM_ADDR_OFFSETf, &eid_crc16);
    SOC_IF_ERROR_RETURN(rv);    
    rv = REG_READ_MEM_ADDR_1r(unit, &reg_val);
    SOC_IF_ERROR_RETURN(rv);    
    rv = soc_MEM_ADDR_1r_field_get(unit, &reg_val, 
            MEM_ADDR_OFFSETf, &eid_crc32);
    SOC_IF_ERROR_RETURN(rv);        
    LOG_INFO(BSL_LS_SOC_ARL,
             (BSL_META_U(unit,
                         "%s,%d,eid_crc16=%d,eid_crc32=%d\n"),
              FUNCTION_NAME(), __LINE__,eid_crc16, eid_crc32));
    assert((eid_crc16 <= _TB_ARL_INDEX_TABLE_ID_MASK) && 
            (eid_crc32 <= _TB_ARL_INDEX_TABLE_ID_MASK));
    
    /* -- check entry valid bit and the MAC+VID on all 4 bin's entries --
     *  1. check sequence bin0 -> bin1 -> bin2 -> bin3.
     *  2. if valid and MAC+VID match than skip the seach process
     *      2.1 get the table_id and bin_id to form a 16K basis ARL index.
     *      2.2 retreive the entry value for returning the result.
     *      2.3 return value will be SOC_E_EXISTS
     *  3. if all 4 bins are valid but no matched MAC+VID >> return FULL.
     *  4. if not FULL condition and no matched MAC+VID >> return NOT_FOUND.
     *
     * Note : SW choice on reporting the table index for the not EXISTS cases
     *  1. FULL condition : choose bin0 index.
     *  2. NOT_FOUND condition : choosed by "Least Full Selection" which 
     *      defined in TB's Appendix Spec.
     */
    for (i = 0; i < ROBO_TBX_L2_BUCKET_SIZE; i++){

        sal_memset(&temp_sw_arl, 0, sizeof(l2_arl_sw_entry_t));
        if (int_flag == _TB_ARL_SEARCH_CONFLICT){
            arl_index[i] = -1;
            rep_entry = (l2_arl_sw_entry_t *)result_entry + i;
        } else {
            rep_entry = (l2_arl_sw_entry_t *)result_entry;
        }
        sal_memset(rep_entry, 0, sizeof(l2_arl_sw_entry_t));
       
        /* the result on key register is started on KEY_2 */
        rv = MEM_KEY_READ(unit, i + 2, &temp_reg_val);
        SOC_IF_ERROR_RETURN(rv);    

        mac_lsb = COMPILER_64_LO(temp_reg_val) & _TB_MIST_MACADDR_MASK;
        
        /* each arl entry uses 2 DATA registers */
        rv = MEM_DATA_READ(unit, i * 2, &temp_reg_val);
        SOC_IF_ERROR_RETURN(rv);    

        temp = (COMPILER_64_HI(temp_reg_val) >> 
                (32 - _TB_MIST_MACADDR_SHIFT)) & _TB_MIST_MACADDR_MASK;
        COMPILER_64_SHL(temp_reg_val, _TB_MIST_MACADDR_SHIFT);
        temp_hi = COMPILER_64_HI(temp_reg_val);
        temp_lo = COMPILER_64_LO(temp_reg_val) | mac_lsb;
        COMPILER_64_SET(temp_reg_val, temp_hi, temp_lo);

        *((uint32 *)&temp_sw_arl) =  COMPILER_64_LO(temp_reg_val);
        *(((uint32 *)&temp_sw_arl) + 1) = COMPILER_64_HI(temp_reg_val);

        rv = MEM_DATA_READ(unit, (i * 2) + 1, &temp_reg_val);
        SOC_IF_ERROR_RETURN(rv);    

        COMPILER_64_SHL(temp_reg_val, _TB_MIST_MACADDR_LENGTH);
        temp_hi = COMPILER_64_HI(temp_reg_val);
        temp_lo = COMPILER_64_LO(temp_reg_val);
        COMPILER_64_SET(temp_reg_val, temp_hi, temp_lo | temp);
        
        *(((uint32 *)&temp_sw_arl) + 2) =  COMPILER_64_LO(temp_reg_val);
        LOG_INFO(BSL_LS_SOC_ARL,
                 (BSL_META_U(unit,
                             "%s,%d,temp_sw_arl[2-0]=%08x-%08x-%08x\n"),
                  FUNCTION_NAME(), __LINE__, *((uint32 *)&temp_sw_arl+2),
                  *((uint32 *)&temp_sw_arl+1), *(uint32 *)&temp_sw_arl));

        /* check valid : accept valid(b11) and pending(b01) status */
        rv = soc_L2_ARL_SWm_field_get(unit, (uint32 *)&temp_sw_arl, 
            VAf, &temp);
        SOC_IF_ERROR_RETURN(rv);    

        if (int_flag == _TB_ARL_SEARCH_MATCH){
            /* keep the entry status(valid and pending) bit for each bin :
             *  - ab_status and cd_status is for the "Least Full Selection"
             *      in TB's dual hash.
             */
            if (i < 2){
                ab_status |= (temp) ? ((i == 0) ? 0x2 : 0x1) : 0;
            } else {
                cd_status |= (temp) ? ((i == 2) ? 0x2 : 0x1) : 0;
            }
            LOG_INFO(BSL_LS_SOC_ARL,
                     (BSL_META_U(unit,
                                 "%s,%d, bin%d ab_status=%d, cd_status=%d!!\n"),
                      FUNCTION_NAME(), __LINE__, i, ab_status, cd_status));
        }

        if (temp) {     /* valid or pending status */
            if (int_flag == _TB_ARL_SEARCH_CONFLICT){
                /* check valid is enough for the conflict search */
                is_conflict = TRUE;

                /* report this index */
                temp = (i < 2) ? eid_crc16 : eid_crc32;
                arl_index[i] = (temp << _TB_ARL_INDEX_TABLE_ID_SHIFT) | 
                        (i & _TB_ARL_INDEX_BIN_ID_MASK);
                
                /* report this entry */
                sal_memcpy(rep_entry, &temp_sw_arl, sizeof(l2_arl_sw_entry_t));

                continue;
            }

            if (temp == _TB_ARL_STATUS_VALID) {
                valid_cnt++;
            }

            /* performing match search operation */
            /* valid(and pending) entry status, than check the MAC+VID */
            rv = soc_L2_ARL_SWm_field_get(unit, (void *)&temp_sw_arl, 
                MACADDRf, (void *)&mac_result);
            SOC_IF_ERROR_RETURN(rv);    

            rv = soc_L2_ARL_SWm_field_get(unit, (uint32 *)&temp_sw_arl, 
                VIDf, &vid_result);
            SOC_IF_ERROR_RETURN(rv);    

            LOG_INFO(BSL_LS_SOC_ARL,
                     (BSL_META_U(unit,
                                 "%s,%d, MAC[1-0](%08x-%08x)+VID(%d) checking...\n"),
                      FUNCTION_NAME(), __LINE__, COMPILER_64_HI(mac_result), 
                      COMPILER_64_LO(mac_result), vid_result));
            if (!sal_memcmp(&mac_key, &mac_result ,8) && 
                    (vid_key == vid_result)){
                /* this entry is the one to be the searched request */
                sal_memcpy(result_entry, &temp_sw_arl, 
                        sizeof(l2_arl_sw_entry_t));
            
                /* bin0/bin1 is hashed by CRC16 and bin2/bin3 is hashed by
                 * CRC32.
                 */
                temp = (i < 2) ? eid_crc16 : eid_crc32;
                *arl_index = (temp << _TB_ARL_INDEX_TABLE_ID_SHIFT) | 
                        (i & _TB_ARL_INDEX_BIN_ID_MASK);
                        
                rv = SOC_E_EXISTS;
                goto mem_search_exit;
            } else {
                /* check next bin */
                continue;
            }
        } else {
            /* no valid or pending. check next bin */
            continue;
        }
    }

    if (int_flag == _TB_ARL_SEARCH_CONFLICT){
        if (is_conflict == TRUE){
            rv = SOC_E_EXISTS;
        } else {
            rv = SOC_E_NOT_FOUND;
        }
    } else {
        /* for match based search */
        if (valid_cnt == _TB_ARL_BIN_NUMBER){
            rv = SOC_E_FULL;
            /* report the table_index for FULL result (bin0 selected) */
            temp = 0;
        } else {
            rv = SOC_E_NOT_FOUND;
            
            /* report the table_index per dual hashed result */
            temp = _TB_ARL_DUAL_HASH_SELECTION(ab_status, cd_status);
        }
        
        if (temp < 2){
            *arl_index = (eid_crc16 << _TB_ARL_INDEX_TABLE_ID_SHIFT) |
                    (temp & _TB_ARL_INDEX_BIN_ID_MASK);
        } else {
            *arl_index = (eid_crc32 << _TB_ARL_INDEX_TABLE_ID_SHIFT) |
                    (temp & _TB_ARL_INDEX_BIN_ID_MASK);
        } 
    }
    
mem_search_exit:
    return rv;
}

/*
 *  Function : _drv_tbx_search_valid_op
 *
 *  Purpose :
 *      Support the search of the valid entry in ARL table.
 *
 *  Parameters :
 *      unit    :   unit id
 *      key     :   memory entry type.
 *      entry   :   entry data pointer.
 *      flags   :   insert flags (no use now).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. For TB device, the search can report the search result in one arl 
 *      entry, thus only "entry" parameter will carry the search result.
 *  2. entry and entyr_1 are asserted to be well allocated.
 */
static int
_drv_tbx_search_valid_op(int unit, uint32 *key, uint32 *entry, 
        uint32 *entry_1, uint32 flags)
{
    uint32  reg_value = 0, temp = 0, srch_ctrl = 0;
    int     rv = SOC_E_NONE, multicast = 0;
    uint32  search_valid = 0, table_index = 0;
    uint64  temp_mac_field, temp_data;
    uint32  temp_hi = 0, temp_lo = 0;
    uint8   temp_mac_addr[6];
    uint16  mac_lsb = 0;
    l2_arl_entry_t  hw_entry;
    int retry;
    
    COMPILER_64_ZERO(temp_mac_field);
    COMPILER_64_ZERO(temp_data);

    /* assigning table_id */
    table_index = ARL_TABLE;
    
    rv = REG_WRITE_MEM_SRCH_INDEXr(unit, &table_index);
    SOC_IF_ERROR_RETURN(rv);       

    /* start / done / valid_check :
     *  
     * There are 3 flags defined for valid search,
     *  - DRV_MEM_OP_SEARCH_DONE
     *  - DRV_MEM_OP_SEARCH_VALID_START
     *  - DRV_MEM_OP_SEARCH_VALID_GET
     *
     * These 3 flags in this routine are optional flag(only one flag can be 
     *  executed). If there are two more flag were set, the process priority
     *  is : DRV_MEM_OP_SEARCH_DONE > DRV_MEM_OP_SEARCH_VALID_START >
     *      DRV_MEM_OP_SEARCH_VALID_GET
     */
    if (flags & DRV_MEM_OP_SEARCH_DONE) {
        /* check if the valid search is DONE! */
        rv = REG_READ_MEM_SRCH_CTRLr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);   
        temp = 1;
        soc_MEM_SRCH_CTRLr_field_get(
                unit, &reg_value, MEM_SRCH_STDNf, &temp);
        if (temp) {
            rv = SOC_E_BUSY;
        } else {
            rv = SOC_E_NONE;
        }
        
    } else if (flags & DRV_MEM_OP_SEARCH_VALID_START) {

        if (SOC_IS_TBX(unit) && !SOC_IS_TB_AX(unit)) {
            if (flags & DRV_MEM_OP_SEARCH_VALID_GET) {
                /* flags == DRV_MEM_OP_SEARCH_VALID_START |
                    DRV_MEM_OP_SEARCH_VALID_GET */
                    /* trigger to find the next valid*/
                rv = REG_READ_MEM_SRCH_KEY_2r(unit, &temp_data);
                SOC_IF_ERROR_RETURN(rv); 
                return SOC_E_NONE;
            }
        }

        /* request valid search OP */
        temp = _TB_ARL_VALID_SEARCH_OP;
        rv = REG_READ_MEM_SRCH_CTRLr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);   

        soc_MEM_SRCH_CTRLr_field_set(
                unit, &reg_value, MEM_SRCH_OP_CMDf, &temp);
        temp = 1;
        soc_MEM_SRCH_CTRLr_field_set(
                unit, &reg_value, MEM_SRCH_STDNf, &temp);
        rv = REG_WRITE_MEM_SRCH_CTRLr(unit, &reg_value);
        SOC_IF_ERROR_RETURN(rv);   

    } else if (flags & DRV_MEM_OP_SEARCH_VALID_GET) {

        rv = REG_READ_MEM_SRCH_CTRLr(unit, &srch_ctrl);
        SOC_IF_ERROR_RETURN(rv); 

        soc_MEM_SRCH_CTRLr_field_get(
                unit, &srch_ctrl, MEM_SRCH_STDNf, &temp);
        if (!temp) {
            rv = SOC_E_NONE;
            goto mem_search_valid_exit;
        }

        soc_MEM_SRCH_CTRLr_field_get(
                unit, &srch_ctrl, MEM_SRCH_OP_VALIDf, &search_valid);
        
        /* TB's ARL valid search report the result as "valid" on the l2 entry 
         *  at valid or pending status.
         */
        if (search_valid){
            /* Get index value */
            for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
                rv = REG_READ_MEM_SRCH_ADDR_0r(unit, &reg_value);
                SOC_IF_ERROR_RETURN(rv);   
                /* valid check again */
                temp = reg_value & 
                    (_TB_ARL_VALID_SEARCH_REPORT_VALID_MASK << 
                    _TB_ARL_VALID_SEARCH_REPORT_VALID_SHIFT);                
                if (temp) {
                    if (retry) {
                        LOG_INFO(BSL_LS_SOC_ARL,
                                 (BSL_META_U(unit,
                                             "=search_vallid (%x), table address valid (%x) retry: %d\n"), 
                                  srch_ctrl, reg_value,retry));
                    }                
                    break;
                } else {
                    LOG_INFO(BSL_LS_SOC_ARL,
                             (BSL_META_U(unit,
                                         "=search_vallid (%x), but table address not valid(%x) retry: %d\n"), 
                              srch_ctrl, reg_value,retry));
                }
            }
            if (temp == 0) {
                /* unexpect condition here, caller(ARL thread) must handle it. */
                rv = SOC_E_FAIL;
                goto mem_search_valid_exit;
            }
            /* get the table_id + bin_id >> 16k basis arl_index */
            *key = reg_value & (_TB_ARL_VALID_SEARCH_REPORT_BINID_MASK | 
                    (_TB_ARL_VALID_SEARCH_REPORT_TBID_MASK << 
                    _TB_ARL_VALID_SEARCH_REPORT_TBID_SHIFT));

            /* --- retrieve searched l2 entry --- 
             *  1. report this rearch result as l2_arl_sw entry format.
             *  2. TB assertion is "The difference between l2_arl and 
             *      l2_arl_sw is only on MAC field."
             *      - l2_arl.MACADDR contain bit12-bit47 MAC address only
             *
             */
            assert(entry);
            sal_memset(&hw_entry, 0, sizeof(l2_arl_entry_t));
            
            
            /* get the searched l2_arl_hw_entry on bit0-bit63 */
            rv = REG_READ_MEM_SRCH_DATA_0r(unit, &temp_data);
            SOC_IF_ERROR_RETURN(rv);   
            temp_lo = COMPILER_64_LO(temp_data);
            temp_hi = COMPILER_64_HI(temp_data);
            *(uint32 *)&hw_entry = temp_lo;
            *((uint32 *)&hw_entry + 1) = temp_hi;
            
            /* Get MAC field */
            rv = soc_L2_ARLm_field_get(unit, (void *)&hw_entry, 
                MACADDRf, (void *)&temp_mac_field);
            SOC_IF_ERROR_RETURN(rv);   

            rv = REG_READ_MEM_SRCH_DATA_1r(unit, &temp_data);
            SOC_IF_ERROR_RETURN(rv);               
            /* get l2 entry bit64-bit68 */
            /* copy l2_arl_hw bit64-bit68 to entry (5 bits >> copy 1 byte) */
            temp = COMPILER_64_LO(temp_data);
            *((uint32 *)&hw_entry + 2) = temp;

            if (SOC_IS_TB_AX(unit)) {            
                /* get the MAC_ADDR bit0-bit11 (mist MAC_ADDR)*/
                rv = REG_READ_MEM_SRCH_KEY_2r(unit, &temp_data);
                mac_lsb = COMPILER_64_LO(temp_data) & _TB_MIST_MACADDR_MASK;
            } else {
                mac_lsb = (temp & 0xFFF0000) >> 16;
            }
            
            /* retrieve MAC addr for multicast check 
             *  - no need to recover full mac for this check.
             */
            COMPILER_64_SHL(temp_mac_field, _TB_MIST_MACADDR_LENGTH);
            temp_hi = COMPILER_64_HI(temp_mac_field);
            temp_lo = COMPILER_64_LO(temp_mac_field);
            COMPILER_64_SET(temp_mac_field, temp_hi, temp_lo | mac_lsb);
            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
            if (temp_mac_addr[0] & 0x01) {
                multicast = 1;
            }
                        
            /* set valid field :
             *  - valid and pending status will be treate as valid here 
             */
            rv = soc_L2_ARLm_field_get(unit, (uint32 *)&hw_entry, 
                VAf, &temp);
            SOC_IF_ERROR_RETURN(rv);   
            rv = soc_L2_ARL_SWm_field_set(unit, entry, 
                VAf, &temp);
            SOC_IF_ERROR_RETURN(rv);               
            /* set user field */
            rv = soc_L2_ARLm_field_get(unit, (uint32 *)&hw_entry, 
                USERf, &temp);
            SOC_IF_ERROR_RETURN(rv);   
            rv = soc_L2_ARL_SWm_field_set(unit, entry, 
                USERf, &temp);
            SOC_IF_ERROR_RETURN(rv);               
                    
            /* set age field */
            rv = soc_L2_ARLm_field_get(unit, (uint32 *)&hw_entry, 
                AGEf, &temp);
            SOC_IF_ERROR_RETURN(rv);   
            rv = soc_L2_ARL_SWm_field_set(unit, entry, 
                AGEf, &temp);
            SOC_IF_ERROR_RETURN(rv);               
                    
            /* set static field */
            rv = soc_L2_ARLm_field_get(unit, (uint32 *)&hw_entry, 
                STATICf, &temp);
            SOC_IF_ERROR_RETURN(rv);   
            rv = soc_L2_ARL_SWm_field_set(unit, entry, 
                STATICf, &temp);
            SOC_IF_ERROR_RETURN(rv);               

            if (soc_feature(unit, soc_feature_arl_mode_control)) {
                /* set arl control field */
                rv = soc_L2_ARLm_field_get(unit, (uint32 *)&hw_entry, 
                    CONf, &temp);
                SOC_IF_ERROR_RETURN(rv);   
                rv = soc_L2_ARL_SWm_field_set(unit, entry, 
                    CONf, &temp);
                SOC_IF_ERROR_RETURN(rv);               

            }

            /* set vid field */
            rv = soc_L2_ARLm_field_get(unit, (uint32 *)&hw_entry, 
                VIDf, &temp);
            SOC_IF_ERROR_RETURN(rv);   
            rv = soc_L2_ARL_SWm_field_set(unit, entry, 
                VIDf, &temp);
            SOC_IF_ERROR_RETURN(rv);               

            /* set MAC field */
            rv = soc_L2_ARL_SWm_field_set(unit, entry, 
                MACADDRf, (void *)&temp_mac_field);
            SOC_IF_ERROR_RETURN(rv);
            
            if (multicast){
                /* set mgid fields for mcast*/
                rv = soc_L2_MARLm_field_get(unit, (uint32 *)&hw_entry, 
                    MGIDf, &temp);
                SOC_IF_ERROR_RETURN(rv);   
                rv = soc_L2_MARL_SWm_field_set(unit, entry, 
                    MGIDf, &temp);
                SOC_IF_ERROR_RETURN(rv);               

            } else {
                /* set port and vport fields for unicast*/
                rv = soc_L2_ARLm_field_get(unit, (uint32 *)&hw_entry, 
                    V_PORTIDf, &temp);
                SOC_IF_ERROR_RETURN(rv);   
                rv = soc_L2_ARL_SWm_field_set(unit, entry, 
                    V_PORTIDf, &temp);
                SOC_IF_ERROR_RETURN(rv);               

                rv = soc_L2_ARLm_field_get(unit, (uint32 *)&hw_entry, 
                    PORTIDf, &temp);
                SOC_IF_ERROR_RETURN(rv);   
                rv = soc_L2_ARL_SWm_field_set(unit, entry, 
                    PORTIDf, &temp);
                SOC_IF_ERROR_RETURN(rv);               
            }
            rv = SOC_E_EXISTS;

        } else {
            rv = SOC_E_NOT_FOUND;
        }
    }
mem_search_valid_exit:
    return rv;

}
        
/*
 *  Function : _drv_bcm53242_mem_table_reset
 *
 *  Purpose :
 *
 *  Parameters :
 *      unit        :   unit id
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
static int
_drv_tbx_mem_table_reset(int unit, uint32 mem)
{
    int rv = SOC_E_NONE;
    uint32 retry;
    uint32  temp, reg_value;


    rv = REG_READ_RST_TABLE_MEMr(unit, &reg_value);
    if (rv < 0) {
        goto mem_table_reset_exit;
    }
    
    temp = 1;
    switch(mem) {
        case DRV_MEM_VLAN:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_VTf, &temp);
            break;
        case DRV_MEM_MSTP:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_MSTPf, &temp);
            break;
        case DRV_MEM_IRC_PORT:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_IRCf, &temp);
            break;
        case DRV_MEM_ERC_PORT:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_ERCf, &temp);
            break;
        case DRV_MEM_1P_TO_TCDP:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_1P_TO_TCDP_MAPPINGf, &temp);
            break;
        case DRV_MEM_TCDP_TO_1P:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_TCDP_TO_1P_MAPPINGf, &temp);
            break;
        case DRV_MEM_IVM_KEY:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_IVM_KEY_TCAMf, &temp);
            break;
        case DRV_MEM_IVM_ACT:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_IVM_ACTIONf, &temp);
            break;
        case DRV_MEM_EVM_KEY:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_EVM_KEY_TCAMf, &temp);
            break;
        case DRV_MEM_EVM_ACT:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_EVM_ACTIONf, &temp);
            break;
        case DRV_MEM_PORTMASK:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_PORT_MASKf, &temp);
            break;
        case DRV_MEM_MCAST_VPORT_MAP:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_MGVP_ID_MAPPINGf, &temp);
            break;
        case DRV_MEM_VPORT_VID_MAP:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_MCSTf, &temp);
            break;
        case DRV_MEM_MCAST:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_VP_VID_MAPPINGf, &temp);
            break;
        case DRV_MEM_ARL:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_ARLf, &temp);
            break;
        case DRV_MEM_CFP_DATA_MASK:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_CFP_TCAMf, &temp);
            break;
        case DRV_MEM_CFP_ACT:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_CFP_ACTIONf, &temp);
            break;
        case DRV_MEM_CFP_METER:
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_CFP_RATE_METERf, &temp);
            break;
        case DRV_MEM_CFP_STAT:            
            soc_RST_TABLE_MEMr_field_set(unit, &reg_value, RST_CFP_STATISTICf, &temp);
            break;
        /* coverity[dead_error_begin] */
        default:
            rv = SOC_E_UNAVAIL;
            goto mem_table_reset_exit;
            break;
    }

    rv = REG_WRITE_RST_TABLE_MEMr(unit, &reg_value);
    if (rv < 0) {
        goto mem_table_reset_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        rv = REG_READ_RST_TABLE_MEMr(unit, &reg_value);
        if (rv < 0) {
            goto mem_table_reset_exit;
        }

        switch(mem) {
            case DRV_MEM_VLAN:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_VTf, &temp);
                break;
            case DRV_MEM_MSTP:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_MSTPf, &temp);
                break;
            case DRV_MEM_IRC_PORT:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_IRCf, &temp);
                break;
            case DRV_MEM_ERC_PORT:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_ERCf, &temp);
                break;
            case DRV_MEM_1P_TO_TCDP:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_1P_TO_TCDP_MAPPINGf, &temp);
                break;
            case DRV_MEM_TCDP_TO_1P:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_TCDP_TO_1P_MAPPINGf, &temp);
                break;
            case DRV_MEM_IVM_KEY:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_IVM_KEY_TCAMf, &temp);
                break;
            case DRV_MEM_IVM_ACT:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_IVM_ACTIONf, &temp);
                break;
            case DRV_MEM_EVM_KEY:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_EVM_KEY_TCAMf, &temp);
                break;
            case DRV_MEM_EVM_ACT:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_EVM_ACTIONf, &temp);
                break;
            case DRV_MEM_PORTMASK:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_PORT_MASKf, &temp);
                break;
            case DRV_MEM_MCAST_VPORT_MAP:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_MGVP_ID_MAPPINGf, &temp);
                break;
            case DRV_MEM_VPORT_VID_MAP:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_VP_VID_MAPPINGf, &temp);
                break;
            case DRV_MEM_MCAST:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_VP_VID_MAPPINGf, &temp);
                break;
            case DRV_MEM_ARL:
            case DRV_MEM_ARL_HW:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_ARLf, &temp);
                break;
            case DRV_MEM_CFP_DATA_MASK:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_CFP_TCAMf, &temp);
                break;
            case DRV_MEM_CFP_ACT:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_CFP_ACTIONf, &temp);
                break;
            case DRV_MEM_CFP_METER:
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_CFP_RATE_METERf, &temp);
                break;
            case DRV_MEM_CFP_STAT:            
                soc_RST_TABLE_MEMr_field_get(unit, &reg_value, RST_CFP_STATISTICf, &temp);
                break;
        }

        if (!temp) {
            break;
        }
    }

    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
    }

    /* The SA learn count is observed not been clear to 0 after ARL table 
     *  reset. Thus we need to force port basis SA learn count here.
     */
    if ((mem == DRV_MEM_ARL) || (mem == DRV_MEM_ARL_HW)){
        soc_port_t  port = 0;
        
        PBMP_E_ITER(unit, port) {
            rv = DRV_ARL_LEARN_COUNT_SET
                (unit, port, DRV_PORT_SA_LRN_CNT_RESET, 0);
            if (rv != SOC_E_NONE){
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "Failed on reset ARL LRN_CNT on port %d\n"), port));
                return SOC_E_INTERNAL;
            }
        }
        
        /* reset to init status while l2 thaw is proceeded. */
        soc_arl_frozen_sync_init(unit);
    }

mem_table_reset_exit:   

    return rv;
}

/*  Report TB's hash index with indicated flag to specify the reported hash is 
 *  CRC16 or CRC32 basis.
 *
 *  This routine will be referenced by other processes and can't be 'static' function.
 */
int 
_drv_tbx_arl_hash_index_get(int unit, int is_crc16, uint32 *sw_arl, int *index)
{
    int         rv = SOC_E_NONE;
    uint64      mac_key, vid_reg_val;
    uint32      vid_key = 0, reg_val = 0, temp = 0;
    uint32      temp_lo = 0, i;
    soc_control_t   *soc = SOC_CONTROL(unit);

    COMPILER_64_ZERO(mac_key);
    COMPILER_64_ZERO(vid_reg_val);

    *index = -1;

    if (!SOC_IS_TBX(unit)){
        return SOC_E_UNAVAIL;
    }

    /* -- get the MAC and VID for search -- 
     *  1. check SVL/IVL mode first for the VID desicion.
     */
    SOC_IF_ERROR_RETURN(soc_L2_ARL_SWm_field_get(unit, sw_arl, 
            MACADDRf, (void *)&mac_key));

    SOC_IF_ERROR_RETURN(soc_L2_ARL_SWm_field_get(unit, sw_arl, 
            VIDf, (uint32 *)&vid_key));
    
    SOC_IF_ERROR_RETURN(REG_READ_VLAN_GLOBAL_CTLr(unit, &reg_val));
    
    SOC_IF_ERROR_RETURN(soc_VLAN_GLOBAL_CTLr_field_get(
            unit, &reg_val, VID_MAC_CTRLf, &temp));
    if (!temp){
        /* means device is at SVL mode, VID will be zero */
        LOG_INFO(BSL_LS_SOC_ARLMON,
                 (BSL_META_U(unit,
                             "%s,%d,SVL mode,vid=%d reset to vid=0\n"),
                  FUNCTION_NAME(),__LINE__,vid_key));
        vid_key = 0;
    }

    /* -- preparing the TB's "index_read" op -- */
    MEM_RWCTRL_REG_LOCK(soc);
    /* set tb_index */
    rv = REG_READ_MEM_INDEXr(unit, &reg_val);
    if (rv){
        goto arl_dual_hash_id_get_exist;
    }
    
    rv = REG_READ_MEM_INDEXr(unit, &reg_val);
    if (rv){
        goto arl_dual_hash_id_get_exist;
    }
    temp = 1;
    rv = soc_MEM_INDEXr_field_set(unit, &reg_val, 
            INDEXf, (uint32 *)&temp);
    if (rv){
        goto arl_dual_hash_id_get_exist;
    }
    rv = REG_WRITE_MEM_INDEXr(unit, &reg_val);
    if (rv){
        goto arl_dual_hash_id_get_exist;
    }

    /* set searching keys */
    rv = REG_WRITE_MEM_KEY_0r(unit, &mac_key);
    if (rv){
        goto arl_dual_hash_id_get_exist;
    }
    
    COMPILER_64_ZERO(vid_reg_val);
    temp_lo = COMPILER_64_LO(vid_reg_val);
    COMPILER_64_SET(vid_reg_val, 0, temp_lo | vid_key);
    rv = REG_WRITE_MEM_KEY_1r(unit, &vid_reg_val);
    
    /* -- start index_read op and check if read is done -- */
    reg_val = 0;
    temp = 3;
    rv = soc_MEM_CTRLr_field_set(unit, &reg_val, OP_CMDf, &temp);
    temp = 1;
    rv = soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_STDNf, &temp);
    rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
    
    /* wait for complete */
    for (i = 0; i < SOC_TIMEOUT_VAL; i++) {
        rv = REG_READ_MEM_CTRLr(unit, &reg_val);
        if (rv){
            goto arl_dual_hash_id_get_exist;
        }

        rv = soc_MEM_CTRLr_field_get(unit, &reg_val, MEM_STDNf, &temp);
        if (rv){
            goto arl_dual_hash_id_get_exist;
        }
        if (!temp) {
            break;
        }
    }
    if (i >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto arl_dual_hash_id_get_exist;
    }

    /* Report entry index(4k basis) */
    if (is_crc16){
        rv = REG_READ_MEM_ADDR_0r(unit, &reg_val);
        if (rv){
            goto arl_dual_hash_id_get_exist;
        }
        rv = soc_MEM_ADDR_0r_field_get(unit, &reg_val, 
                MEM_ADDR_OFFSETf, &temp);
    } else {
        rv = REG_READ_MEM_ADDR_1r(unit, &reg_val);
        if (rv){
            goto arl_dual_hash_id_get_exist;
        }
        rv = soc_MEM_ADDR_1r_field_get(unit, &reg_val, 
                MEM_ADDR_OFFSETf, &temp);
    }
    if (rv){
        goto arl_dual_hash_id_get_exist;
    }
    *index = (int)temp;

arl_dual_hash_id_get_exist :
    MEM_RWCTRL_REG_UNLOCK(soc);

    return rv;
}


/*
 *  Function : drv_tbx_mem_clear
 *
 *  Purpose :
 *      Clear whole memory entries.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory indication.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 *
 */
int 
drv_tbx_mem_clear(int unit, uint32 mem)
{
    int rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_mem_clear : mem=0x%x\n"), mem));


    switch(mem) {
        case DRV_MEM_VLAN:
        case DRV_MEM_MSTP:
        case DRV_MEM_IRC_PORT:
        case DRV_MEM_ERC_PORT:
        case DRV_MEM_1P_TO_TCDP:
        case DRV_MEM_TCDP_TO_1P:
        case DRV_MEM_IVM_KEY:
        case DRV_MEM_IVM_ACT:
        case DRV_MEM_EVM_KEY:
        case DRV_MEM_EVM_ACT:
        case DRV_MEM_PORTMASK:
        case DRV_MEM_MCAST_VPORT_MAP:
        case DRV_MEM_VPORT_VID_MAP:
        case DRV_MEM_MCAST:
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
        /* allowed ARL and ARL_HW for reset only!! 
        case DRV_MEM_MARL_HW:
        */
        case DRV_MEM_CFP_DATA_MASK:
        case DRV_MEM_CFP_ACT:
        case DRV_MEM_CFP_METER:
        case DRV_MEM_CFP_STAT:            
            rv = _drv_tbx_mem_table_reset(unit, mem);
            break;
            
        case DRV_MEM_MARL:
            rv = DRV_MEM_DELETE(unit, DRV_MEM_MARL, NULL, 
                    DRV_MEM_OP_DELETE_ALL_ARL);
            break;
        default:
            rv = SOC_E_UNAVAIL;
            break;
    }


    return rv;
}

/*
 *  Function : drv_tbx_mem_delete
 *
 *  Purpose :
 *      Remove an entry to specific memory or remove entries by flags 
 *
 *  Parameters :
 *      unit    :   unit id
 *      mem     :   memory entry type.
 *      entry   :   entry data pointer.
 *      flags   :   delete flags.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. Only for ARL memory now.
 *  2. Handled delete Operations
 *      a. normal ARL deletion (with MAC+VID key delete)
 *      b. fast aging deletion 
 *  P.S 
 *      1. normal deletion and fast aging deletion can not be requested 
 *          together.
 *      2. Fast aging deletion will be proceeded once both deletions of 
 *          normal and fast aging requested.
 *  3. For the fast aging process, the "entry" input parameter is used to 
 *      carry the aging key.
 */
int
drv_tbx_mem_delete(int unit, uint32 mem, uint32 *entry, uint32 flags)
{
    int         rv = SOC_E_NONE, sw_arl_update = 0;
    int         i, table_index, bin_id;
    int         single_fastaging = FALSE;
    int         is_dynamic = FALSE, is_ucast = FALSE;
    uint64      temp_mem_data;
    uint32      temp = 0, temp_key, reg_val, ageout_reg;
    uint32      src_port= 0, vlanid = 0, fastaging_flags = 0;
    uint32      temp_flags = 0, fastaging_control = 0;
    uint8       temp_mac[6];
    l2_arl_sw_entry_t   temp_sw_arl;
    soc_control_t       *soc = SOC_CONTROL(unit);
    int         search_arl_index[ROBO_TBX_L2_BUCKET_SIZE];
    l2_arl_sw_entry_t   search_sw_arl[ROBO_TBX_L2_BUCKET_SIZE];
    
    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_mem_delete : mem=0x%x, flags = 0x%x)\n"),
              mem, flags));
    
    switch(mem) {
        case DRV_MEM_ARL:
            /* supported */
            break;
        case DRV_MEM_MARL:
            if (flags & DRV_MEM_OP_DELETE_ALL_ARL){
                /* current design to reset all ARL as well */
                rv = _drv_tbx_mem_table_reset(unit, DRV_MEM_ARL);                
                return rv;
            }            
            /* no break here */
            /* break;*/ 
        case DRV_MEM_ARL_HW:
        case DRV_MEM_MARL_HW:
        case DRV_MEM_VLAN:
        case DRV_MEM_MSTP:
        case DRV_MEM_1P_TO_TCDP:
        case DRV_MEM_TCDP_TO_1P:
        case DRV_MEM_IVM_KEY:
        case DRV_MEM_IVM_ACT:
        case DRV_MEM_EVM_KEY:
        case DRV_MEM_EVM_ACT:
        case DRV_MEM_PORTMASK:
        case DRV_MEM_MCAST_VPORT_MAP:
        case DRV_MEM_VPORT_VID_MAP:
        case DRV_MEM_MCAST:
        case DRV_MEM_CFP_DATA_MASK:
        case DRV_MEM_CFP_ACT:
        case DRV_MEM_CFP_METER:
        case DRV_MEM_CFP_STAT:            
        default:            
            return SOC_E_UNAVAIL;
    }
    
    /* DRV_MEM_OP_DELETE_ALL_ARL is not implemented for TB :
     *  1. the same process is handled in mem_clear()
     *  2. no uper layer called mem_delete for ARL delete by this flag.
     */
    if (flags & DRV_MEM_OP_DELETE_ALL_ARL){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s, DRV_MEM_OP_DELETE_ALL_ARL is not implemented for TB\n"),
                   FUNCTION_NAME()));        
        return SOC_E_PARAM;
    }
    
    fastaging_flags = flags & _TB_ARL_FAST_AGING_TYPE_MASK;
    if (fastaging_flags) {
        /*  -- fast aging deletion -- 
         *  1. Fast aging filter. (basis the fast aging mode)
         *      - DRV_MEM_OP_DELETE_BY_DYN_MCAST 
         *      - DRV_MEM_OP_DELETE_BY_ST_MCAST
         *      - DRV_MEM_OP_DELETE_BY_DYN_UCAST
         *      - DRV_MEM_OP_DELETE_BY_ST_UCAST
         *  2. Fast aging mode with key : different key is not allowed to 
         *      proceed at the same time.
         *      - DRV_MEM_OP_DELETE_BY_PORT | DRV_MEM_OP_DELETE_BY_VLANID |
         *          DRV_MEM_OP_DELETE_BY_SPT | DRV_MEM_OP_DELETE_BY_TRUNK
         *      ### HW allowed all those fast aging been proceeded at the same
         *          time but our SW design allowed one time fast aging for one
         *          key only.
         *  3. Fast aging in TB won't serve the flag on "PENDING"
         */
        rv = REG_READ_FAST_AGING_CTLr(unit, &reg_val);
        soc_FAST_AGING_CTLr_field_get(
                unit, &reg_val, FAST_AGE_STDNf, &temp);
        assert(temp == 0);
        
        if (flags & DRV_MEM_OP_PENDING){
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "  FastAging can't serve 'PENDING' flag\n")));
            return SOC_E_UNAVAIL;
        }
        
        /* default fast aging control :
         *  1. Dynamic : default ON
         *  2. Unicast : default ON
         *  3. Static : default OFF
         *  4. Multicast : default OFF
         *  >> DYN_MC will be default ON for dynamic factor(HW default)
         *
         * ---- The fast aging control scenario matrix will be :
         *  (1):ST_ONLY; (2):ST; (3):MC_ONLY; (4):MC
         *
         *          (none)  1    2    3    4   1+3  1+4  2+3  2+4
         *  ======================================================
         *  ST_UC  :       x    x                   x         x 
         *  ST_MC  :                           x    x    x    x
         *  DYN_UC :   x        x         x                   x
         *  DYN_MC :                 x    x              x    x
         *  ======================================================
         *  P.S :
         *   - (1+2) and (3+4) sets are conflict configuration >> ignore it.
         */
        temp_flags = flags & _TB_ARL_FAST_AGING_CONTROL_MASK;
        
        /* fast aging control scenario matrix result */
        fastaging_control = 0;
        if (temp_flags){
            if (temp_flags == DRV_MEM_OP_DELETE_BY_STATIC_ONLY){
                /* case (1) */
                fastaging_control = DRV_MEM_OP_DELETE_BY_ST_UCAST;
            } else if (temp_flags == DRV_MEM_OP_DELETE_BY_STATIC){
                /* case (2) */
                fastaging_control = DRV_MEM_OP_DELETE_BY_ST_UCAST | 
                        DRV_MEM_OP_DELETE_BY_DYN_UCAST;
            } else if (temp_flags == DRV_MEM_OP_DELETE_BY_MCAST_ONLY){
                /* case (3) */
                fastaging_control = DRV_MEM_OP_DELETE_BY_DYN_MCAST;
            } else if (temp_flags == DRV_MEM_OP_DELETE_BY_MCAST){
                /* case (4) */
                fastaging_control = DRV_MEM_OP_DELETE_BY_DYN_MCAST |
                        DRV_MEM_OP_DELETE_BY_DYN_UCAST;
            } else if (temp_flags == (DRV_MEM_OP_DELETE_BY_STATIC_ONLY | 
                    DRV_MEM_OP_DELETE_BY_MCAST_ONLY)){
                /* case (1)+(3) */
                fastaging_control = DRV_MEM_OP_DELETE_BY_ST_MCAST;
            } else if (temp_flags == (DRV_MEM_OP_DELETE_BY_STATIC_ONLY | 
                    DRV_MEM_OP_DELETE_BY_MCAST)){
                /* case (1)+(4) */
                fastaging_control = DRV_MEM_OP_DELETE_BY_ST_UCAST |
                        DRV_MEM_OP_DELETE_BY_ST_MCAST;
            } else if (temp_flags == (DRV_MEM_OP_DELETE_BY_STATIC | 
                    DRV_MEM_OP_DELETE_BY_MCAST_ONLY)){
                /* case (2)+(3) */
                fastaging_control = DRV_MEM_OP_DELETE_BY_DYN_MCAST |
                        DRV_MEM_OP_DELETE_BY_ST_MCAST;
            } else if (temp_flags == (DRV_MEM_OP_DELETE_BY_STATIC | 
                    DRV_MEM_OP_DELETE_BY_MCAST)){
                /* case (2)+(4) */
                fastaging_control = DRV_MEM_OP_DELETE_BY_DYN_MCAST |
                        DRV_MEM_OP_DELETE_BY_DYN_UCAST |
                        DRV_MEM_OP_DELETE_BY_ST_MCAST |
                        DRV_MEM_OP_DELETE_BY_ST_UCAST;
            } else {
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     " Conflict fast aging control!\n")));                
                return SOC_E_CONFIG;
            }
        } else {
            /* so specific fast aging control >> use default behavior */
            fastaging_control = DRV_MEM_OP_DELETE_BY_DYN_UCAST;
        }
        temp = (fastaging_control & DRV_MEM_OP_DELETE_BY_DYN_MCAST) ? 1 : 0;
        soc_FAST_AGING_CTLr_field_set(
                unit, &reg_val, EN_AGE_DYNAMIC_MCf, &temp);
        temp = (fastaging_control & DRV_MEM_OP_DELETE_BY_ST_MCAST) ? 1 : 0;
        soc_FAST_AGING_CTLr_field_set(
                unit, &reg_val, EN_AGE_STATIC_MCf, &temp);
        temp = (fastaging_control & DRV_MEM_OP_DELETE_BY_DYN_UCAST) ? 1 : 0;
        soc_FAST_AGING_CTLr_field_set(
                unit, &reg_val,EN_AGE_DYNAMIC_UCf, &temp);
        temp = (fastaging_control & DRV_MEM_OP_DELETE_BY_ST_UCAST) ? 1 : 0;
        soc_FAST_AGING_CTLr_field_set(
                unit, &reg_val, EN_AGE_STATIC_UCf, &temp);

        /* for fast aging key value assignment */
        rv = REG_READ_AGEOUT_CTLr(unit, &ageout_reg);
        SOC_IF_ERROR_RETURN(rv);
        
        /* key to proceed fast aging : key type {PORT|VLAN|SPT|TRUNK}
         *  1. for port and vid fast aging, the current design in BCM API 
         *      will carry port_id and vid in arl_entry(sw arl entry).
         *  2. for SPT and Trunk fast aging, there is no reflect filed in 
         *      arl_entry to carry with the id of SPT and trunk. Thus the 
         *      design below will treat the entry[0] as the key id for 
         *      fast aging process.
         */
        if ((fastaging_flags == DRV_MEM_OP_DELETE_BY_PORT) ||
                (fastaging_flags == DRV_MEM_OP_DELETE_BY_VLANID)||
                (fastaging_flags == DRV_MEM_OP_DELETE_BY_SPT)||
                (fastaging_flags == DRV_MEM_OP_DELETE_BY_TRUNK)){
            single_fastaging = TRUE;
        } else {
            single_fastaging = FALSE;
        }

        if (flags & DRV_MEM_OP_DELETE_BY_PORT){
            if (single_fastaging == FALSE){
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "  Multi-FastAging requested! "
                                     "Performing Port-FastAging mode only.\n")));
            }
            
            temp = _TB_ARL_FAST_AGE_MODE_PORT;
            rv = soc_L2_ARL_SWm_field_get(unit, (uint32 *)entry, 
                PORTIDf, &temp_key);
            SOC_IF_ERROR_RETURN(rv);
            
            src_port = temp_key;
            soc_AGEOUT_CTLr_field_set(
                unit, &ageout_reg, AGE_EN_PORTf, &temp_key);
        } else if (flags & DRV_MEM_OP_DELETE_BY_VLANID){
            if (single_fastaging == FALSE){
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "  Multi-FastAging requested! "
                                     "Performing VLAN-FastAging mode only.\n")));
            }
            
            temp = _TB_ARL_FAST_AGE_MODE_VLAN;
            rv = soc_L2_ARL_SWm_field_get(unit, (uint32 *)entry, 
                VIDf, &temp_key);
            SOC_IF_ERROR_RETURN(rv);
            vlanid = temp_key;
            soc_AGEOUT_CTLr_field_set(
                unit, &ageout_reg, AGE_EN_VIDf, &temp_key);
        } else if (flags & DRV_MEM_OP_DELETE_BY_SPT){
            if (single_fastaging == FALSE){
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "  Multi-FastAging requested! "
                                     "Performing SPT-FastAging mode only.\n")));
            }
            
            temp_key = *(uint32 *)entry;
            temp = _TB_ARL_FAST_AGE_MODE_SPT;
            
            soc_AGEOUT_CTLr_field_set(
                unit, &ageout_reg, SPT_AGE_ENf, &temp_key);
        } else if (flags & DRV_MEM_OP_DELETE_BY_TRUNK){
            /* process flow in this section should be single_fastaging==TURE.
             *  Keep the warning message below in case the if-else-if 
             *  processing flow changed.
             */
            if (single_fastaging == FALSE){
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "  Multi-FastAging requested! "
                                     "Performing TRUNK-FastAging mode only.\n")));
            }
            
            temp_key = *(uint32 *)entry;
            temp = _TB_ARL_FAST_AGE_MODE_TRUNK;
            
            soc_AGEOUT_CTLr_field_set(
                unit, &ageout_reg, AGE_EN_LAGf, &temp_key);
        }
        /* write to AGEOUT_CTLr for fast age */
        rv = REG_WRITE_AGEOUT_CTLr(unit, &ageout_reg);
        SOC_IF_ERROR_RETURN(rv);           
        soc_FAST_AGING_CTLr_field_set(
                unit, &reg_val, AGE_MODE_CTRLf, &temp);
        temp = 1;
        soc_FAST_AGING_CTLr_field_set(
                unit, &reg_val, FAST_AGE_STDNf, &temp);
        
        /* write FAST_AGING_CTLr to start fast age */
        rv = REG_WRITE_FAST_AGING_CTLr(unit, &reg_val);
        SOC_IF_ERROR_RETURN(rv);
        
        /* wait for the fast aging DONE */
        for (i = 0; i < SOC_TIMEOUT_VAL; i++) {
            rv = REG_READ_FAST_AGING_CTLr(unit, &reg_val);
            SOC_IF_ERROR_RETURN(rv);
            soc_FAST_AGING_CTLr_field_get(
                    unit, &reg_val, FAST_AGE_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (i >= SOC_TIMEOUT_VAL) {
            return SOC_E_TIMEOUT;
        }
        
        /* reset to init status while l2 thaw is proceeded. */
        soc_arl_frozen_sync_init(unit);
        
        /* Remove entries from software table by port/vlan */
        rv = _drv_arl_database_delete_by_fastage(unit, src_port,
                vlanid, flags);     
        return rv;
    } else {
        /* -- Normal ARL deletion -- 
         *  1. existence check
         *      a. search first for check the existence.
         *          - the SOC_E_EXISTS is the only return value allowed 
         *              delete operation.
         *  2. delete op
         *      a. SOC_E_EXISTS means an ARL entry is found with the same 
         *          MAC+VID at valid or pending entry status.
         *      b. DRV_MEM_OP_PENDING is for the request on delete pending 
         *          arl entry only.
         *      c. Override the existed bins' entry data by all zero.
         */
        
        assert(entry);
        if (!(flags & (DRV_MEM_OP_BY_HASH_BY_MAC | 
                DRV_MEM_OP_BY_HASH_BY_VLANID))){
            return SOC_E_PARAM;
        }
       
        MEM_RWCTRL_REG_LOCK(soc);
        /* search MAC+VID first :
         *  - for the LOCK control, the search result assumed will be kept  
         *      after search OP returned.
         */
        sal_memset(search_arl_index, 0, 
                sizeof(int) * ROBO_TBX_L2_BUCKET_SIZE);
        sal_memset(search_sw_arl, 0, 
                sizeof(l2_arl_sw_entry_t) * ROBO_TBX_L2_BUCKET_SIZE);
        rv = _drv_tbx_search_key_op(unit, entry, _TB_ARL_SEARCH_MATCH,
                (uint32 *)&search_sw_arl, search_arl_index);
        /* for op = _TB_ARL_SEARCH_MATCH, the reported index and entry will be 
        * placed in the first item in searched array.
        */
        table_index = search_arl_index[0];
        sal_memcpy(&temp_sw_arl, &search_sw_arl, sizeof(l2_arl_sw_entry_t));
        
        bin_id = -1;
        if (rv == SOC_E_EXISTS){
            bin_id = table_index & _TB_ARL_INDEX_BIN_ID_MASK;
            
        } else {
            rv = SOC_E_NOT_FOUND;
            goto mem_arl_delete_exist;
        }
        
        /* assert MEM_INDEX is not modified unexpected */
        rv = REG_READ_MEM_INDEXr(unit, &reg_val);
        soc_MEM_INDEXr_field_get(unit, &reg_val, INDEXf, &temp);
        assert(temp == ARL_TABLE);
        
        /* the bin entry here after search op is assert on staying at valid 
         * or pending status */
        rv = soc_L2_ARL_SWm_field_get(unit, (uint32 *)&temp_sw_arl, 
            VAf, &temp);
        if (SOC_FAILURE(rv)){
            goto mem_arl_delete_exist;
        }

        assert(temp == _TB_ARL_STATUS_PENDING || 
                temp == _TB_ARL_STATUS_VALID);
        
        /* BCM_L2_REPLACE_PENDING means replace pending entries only. */ 
        /* SDK-33848 : 
         *  Redesign request on DELETE + PENDING process.
         *
         *  1. earily edition op was : 
         *      >> remove {(pending only) v.s. (non-pending only)}
         *  2. previous edition op was :
         *      >> remove {(pending + non-pending) v.s. (non-pending only)}
         *  3. this edition op is : draw back to earily edition op.
         *      >> remove {(pending only) v.s. (non-pending only)}
         */
        if (flags & DRV_MEM_OP_PENDING){
            /* PENDING flag is set, remove pending l2 entry only.*/
            if (temp != _TB_ARL_STATUS_PENDING){
                rv = SOC_E_NOT_FOUND;
                goto mem_arl_delete_exist;
            }
        } else {
            /* PENDING flag isn't set, remove non-pending l2 entry only. */
            if (temp == _TB_ARL_STATUS_PENDING){
                rv = SOC_E_NOT_FOUND;
                goto mem_arl_delete_exist;
            }
        }
        
        rv = soc_L2_ARL_SWm_field_get(unit, (uint32 *)&temp_sw_arl, 
            STATICf, &temp);
        if (SOC_FAILURE(rv)){
            goto mem_arl_delete_exist;
        }


        is_dynamic = (!temp) ? TRUE : FALSE; 
        
        if (!(flags & DRV_MEM_OP_DELETE_BY_STATIC)){
            /* if no such flag, static entry not allow to be deleted */
            if (!is_dynamic){
                rv = SOC_E_NOT_FOUND;
                goto mem_arl_delete_exist;
            }
        }
        
        if (flags & DRV_MEM_OP_DELETE_BY_STATIC_ONLY){
            /* allow status entry be delete only */
            if (is_dynamic){
                rv = SOC_E_NOT_FOUND;
                goto mem_arl_delete_exist;
            }
        }

        /* clear target bin entry data for delete */
        sal_memset(&temp_mem_data, 0, sizeof(uint64));
        rv = MEM_DATA_WRITE(unit, bin_id * 2, &temp_mem_data);
        if (SOC_FAILURE(rv)){
            goto mem_arl_delete_exist;
        }

        rv = MEM_DATA_WRITE(unit, bin_id * 2 + 1, &temp_mem_data);
        if (SOC_FAILURE(rv)){
            goto mem_arl_delete_exist;
        }
        
        /* index_write OP start */
        reg_val = 0;
        temp = MEM_OP_IDX_WRITE;
        soc_MEM_CTRLr_field_set(unit, &reg_val, OP_CMDf, &temp);
        temp = 1;
        soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_STDNf, &temp);
        rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
        if (SOC_FAILURE(rv)){
            goto mem_arl_delete_exist;
        }
        
        /* wait for complete */
        for (i = 0; i < SOC_TIMEOUT_VAL; i++) {
            rv = REG_READ_MEM_CTRLr(unit, &reg_val);
            if (SOC_FAILURE(rv)){
                goto mem_arl_delete_exist;
            }
            soc_MEM_CTRLr_field_get(unit, &reg_val, MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (i >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_arl_delete_exist;
        }
        
        /* SA Learning Count handler :
         *  - decrease one for the ARL deletion process
         *      (for Dynamic and Unicast entry only)
         */
        /* ucast entry check */
        rv = soc_L2_ARL_SWm_field_get(unit, (void *)&temp_sw_arl, 
            MACADDRf, (void *)&temp_mem_data);
        if (SOC_FAILURE(rv)){
            goto mem_arl_delete_exist;
        }
        
        SAL_MAC_ADDR_FROM_UINT64(temp_mac, temp_mem_data);
        is_ucast = !((*(uint8 *)&temp_mac) & 0x1);  /* check mac_addr bit0 */

        if (is_dynamic && is_ucast){
            /* retrieve the port_id to increase one in SA_LRN_CNT.port */
            rv = soc_L2_ARL_SWm_field_get(unit, (uint32 *)&temp_sw_arl, 
                PORTIDf, &temp);
            if (SOC_FAILURE(rv)){
                goto mem_arl_delete_exist;
            }

            rv = DRV_ARL_LEARN_COUNT_SET(unit, temp, 
                    DRV_PORT_SA_LRN_CNT_DECREASE, 0);
            if (SOC_FAILURE(rv)){
                goto mem_arl_delete_exist;
            }
            LOG_INFO(BSL_LS_SOC_ARL,
                     (BSL_META_U(unit,
                                 "%s,port%d, SA_LRN_CNT decreased one!\n"),
                      FUNCTION_NAME(), temp));
        }

        sw_arl_update = 1;
        
mem_arl_delete_exist :
        MEM_RWCTRL_REG_UNLOCK(soc);

        if (sw_arl_update){
            /* Remove the entry from sw database :
             *  - sync the SW ARL for HW delete request and the delete callback 
             *      will be executed in _drv_arl_database_delete().
             *
             *  Important :
             *  1. the 2nd parameter is index. For all eariler ROBO devices before
             *      TB, the ARL table index is not allowed to be retrieved. 
             *      Thus the original design past this index as bin id to  
             *      indicate the real bin id is the target entry to delete.
             *      
             *  2. New Design for TB is past the 2nd parameter as the full table 
             *      index(16K basis entry_id)
             */
            _drv_arl_database_delete(unit, table_index, &temp_sw_arl);        
        }
        return rv;
    }
}

/*
 *  Function : _tbx_mem_mapping
 *
 *  Purpose :
 *      memory mapping from DRV_MEM_xxx to real enumeration
 *
 *  Parameters :
 *      unit            (IN)    :   unit id
 *      mem             (IN)    :   memory indication. (DRV_MEM_xxx)
 *      field_index     (IN)    :  field type. (DRV_MEM_FIELD_xxx),
 *                              : -1 means ignore the field_index mapping
 *      map_mem_id      (OUT)   :   mapped xxxm
 *      map_field_id    (OUT)   :   mapped xxxf
 *
 *  Return :
 *      SOC_E_XXX
 */
static int
_tbx_mem_mapping(int unit, uint32 mem, uint32 field_index, int *map_mem_id, int *map_field_id)
{
    int i;
    int mem_id, field_id;
    
    mem_id = -1;
    for (i = 0; i < COUNTOF(drv_TB_mem_info); i++) {
        if (drv_TB_mem_info[i].drv_mem == mem){
            mem_id = drv_TB_mem_info[i].soc_mem;
            break;
        }    
    }

    if (!SOC_IS_TB_AX(unit)){
        for (i = 0; i < COUNTOF(drv_TB_cfp_mem_info_B0); i++) {
            if (drv_TB_cfp_mem_info_B0[i].drv_mem == mem){
                mem_id = drv_TB_cfp_mem_info_B0[i].soc_mem;
                break;
            }    
        }
    }

#ifdef BCM_VO_SUPPORT
    if (SOC_IS_VO(unit)) {
        for (i = 0; i < COUNTOF(drv_VO_cfp_mem_info); i++) {
            if (drv_VO_cfp_mem_info[i].drv_mem == mem){
                mem_id = drv_VO_cfp_mem_info[i].soc_mem;
                break;
            }   
        }    
    }        
#endif
    if (mem_id == -1){
        return SOC_E_UNAVAIL;
    }
    
    if (field_index == -1){
        *map_mem_id = mem_id;
        return SOC_E_NONE;
    }

    switch (mem)
    {
        case DRV_MEM_VLAN:
            if (field_index == DRV_MEM_FIELD_PORT_BITMAP) {
                field_id = INDEX(FORWARD_MAPf);
            } else if (field_index == DRV_MEM_FIELD_SPT_GROUP_ID) {
                field_id = INDEX(MSPT_IDf);
            } else if (field_index == DRV_MEM_FIELD_FWD_MODE) {
                field_id = INDEX(DIS_LRNf);
            } else if (field_index == DRV_MEM_FIELD_DIR_FWD) {
                field_id = INDEX(DIR_FWDf);
            } else if (field_index == DRV_MEM_FIELD_UCAST_DROP) {
                field_id = INDEX(ULF_DROPf);
            } else if (field_index == DRV_MEM_FIELD_MCAST_DROP) {
                field_id = INDEX(MLF_DROPf);
            } else if (field_index == DRV_MEM_FIELD_ISO_MAP) {
                field_id = INDEX(ISO_MAPf);
            } else if (field_index == DRV_MEM_FIELD_OUTPUT_UNTAG) {
                field_id = INDEX(V_UNTAG_MAPf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_IRC_PORT:
            if (field_index == DRV_MEM_FIELD_IRC_BKT0_REF_CNT) {
                field_id = INDEX(BKT0_REF_CNTf);
            } else if (field_index == DRV_MEM_FIELD_IRC_BKT0_BKT_SIZE) {
                field_id = INDEX(BKT0_BKT_SIZEf);
            } else if (field_index == DRV_MEM_FIELD_IRC_BKT0_PKT_MASK) {
                field_id = INDEX(BKT0_PKT_MASKf);
            } else if (field_index == DRV_MEM_FIELD_IRC_BKT0_IRC_EN) {
                field_id = INDEX(BKT0_IRC_ENf);
            } else if (field_index == DRV_MEM_FIELD_IRC_BKT1_REF_CNT) {
                field_id = INDEX(BKT1_REF_CNTf);
            } else if (field_index == DRV_MEM_FIELD_IRC_BKT1_BKT_SIZE) {
                field_id = INDEX(BKT1_BKT_SIZEf);
            } else if (field_index == DRV_MEM_FIELD_IRC_BKT1_PKT_MASK) {
                field_id = INDEX(BKT1_PKT_MASKf);
            } else if (field_index == DRV_MEM_FIELD_IRC_BKT1_IRC_EN) {
                field_id = INDEX(BKT1_IRC_ENf);
            } else if (field_index == DRV_MEM_FIELD_IRC_BKT2_REF_CNT) {
                field_id = INDEX(BKT2_REF_CNTf);
            } else if (field_index == DRV_MEM_FIELD_IRC_BKT2_BKT_SIZE) {
                field_id = INDEX(BKT2_BKT_SIZEf);
            } else if (field_index == DRV_MEM_FIELD_IRC_BKT2_PKT_MASK) {
                field_id = INDEX(BKT2_PKT_MASKf);
            } else if (field_index == DRV_MEM_FIELD_IRC_BKT2_IRC_EN) {
                field_id = INDEX(BKT2_IRC_ENf);
            } else if (field_index == DRV_MEM_FIELD_IRC_DROP_EN) {
                field_id = INDEX(EN_DROPf);
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_ERC_PORT:
            if (field_index == DRV_MEM_FIELD_ERC_BKT_Q0_REF_CNT_MIN) {
                field_id = INDEX(BKT_Q0_REF_CNT_MINf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q0_REF_CNT_MAX) {
                field_id = INDEX(BKT_Q0_REF_CNT_MAXf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q0_BKT_SIZE) {
                field_id = INDEX(BKT_Q0_BKT_SIZEf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q0_ERC_Q_EN) {
                field_id = INDEX(BKT_Q0_ERC_Q_ENf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q1_REF_CNT_MIN) {
                field_id = INDEX(BKT_Q1_REF_CNT_MINf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q1_REF_CNT_MAX) {
                field_id = INDEX(BKT_Q1_REF_CNT_MAXf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q1_BKT_SIZE) {
                field_id = INDEX(BKT_Q1_BKT_SIZEf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q1_ERC_Q_EN) {
                field_id = INDEX(BKT_Q1_ERC_Q_ENf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q2_REF_CNT_MIN) {
                field_id = INDEX(BKT_Q2_REF_CNT_MINf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q2_REF_CNT_MAX) {
                field_id = INDEX(BKT_Q2_REF_CNT_MAXf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q2_BKT_SIZE) {
                field_id = INDEX(BKT_Q2_BKT_SIZEf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q2_ERC_Q_EN) {
                field_id = INDEX(BKT_Q2_ERC_Q_ENf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q3_REF_CNT_MIN) {
                field_id = INDEX(BKT_Q3_REF_CNT_MINf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q3_REF_CNT_MAX) {
                field_id = INDEX(BKT_Q3_REF_CNT_MAXf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q3_BKT_SIZE) {
                field_id = INDEX(BKT_Q3_BKT_SIZEf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q3_ERC_Q_EN) {
                field_id = INDEX(BKT_Q3_ERC_Q_ENf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q4_REF_CNT_MIN) {
                field_id = INDEX(BKT_Q4_REF_CNT_MINf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q4_REF_CNT_MAX) {
                field_id = INDEX(BKT_Q4_REF_CNT_MAXf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q4_BKT_SIZE) {
                field_id = INDEX(BKT_Q4_BKT_SIZEf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q4_ERC_Q_EN) {
                field_id = INDEX(BKT_Q4_ERC_Q_ENf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q5_REF_CNT_MIN) {
                field_id = INDEX(BKT_Q5_REF_CNT_MINf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q5_REF_CNT_MAX) {
                field_id = INDEX(BKT_Q5_REF_CNT_MAXf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q5_BKT_SIZE) {
                field_id = INDEX(BKT_Q5_BKT_SIZEf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q5_ERC_Q_EN) {
                field_id = INDEX(BKT_Q5_ERC_Q_ENf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q6_REF_CNT_MIN) {
                field_id = INDEX(BKT_Q6_REF_CNT_MINf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q6_REF_CNT_MAX) {
                field_id = INDEX(BKT_Q6_REF_CNT_MAXf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q6_BKT_SIZE) {
                field_id = INDEX(BKT_Q6_BKT_SIZEf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q6_ERC_Q_EN) {
                field_id = INDEX(BKT_Q6_ERC_Q_ENf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q7_REF_CNT_MIN) {
                field_id = INDEX(BKT_Q7_REF_CNT_MINf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q7_REF_CNT_MAX) {
                field_id = INDEX(BKT_Q7_REF_CNT_MAXf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q7_BKT_SIZE) {
                field_id = INDEX(BKT_Q7_BKT_SIZEf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_Q7_ERC_Q_EN) {
                field_id = INDEX(BKT_Q7_ERC_Q_ENf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_T_REF_CNT_MAX) {
                field_id = INDEX(BKT_T_REF_CNT_MAXf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_T_BKT_SIZE) {
                field_id = INDEX(BKT_T_BKT_SIZEf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_T_TYPE) {
                field_id = INDEX(BKT_T_TYPEf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_T_ERC_T_EN) {
                field_id = INDEX(BKT_T_ERC_T_ENf);
            } else if (field_index == DRV_MEM_FIELD_ERC_BKT_BAC) {
                field_id = INDEX(BKT_BACf);
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_1P_TO_TCDP:
            if (field_index == DRV_MEM_FIELD_TC) {
                field_id = INDEX(PCP2TCf);
            } else if (field_index == DRV_MEM_FIELD_DP) {
                field_id = INDEX(PCP2DPf);
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_TCDP_TO_1P:
            if (field_index == DRV_MEM_FIELD_C_DEI) {
                field_id = INDEX(C_DEIf);
            } else if (field_index == DRV_MEM_FIELD_C_PCP) {
                field_id = INDEX(C_PCPf);
            } else if (field_index == DRV_MEM_FIELD_S_DEI) {
                field_id = INDEX(S_DEIf);
            } else if (field_index == DRV_MEM_FIELD_S_PCP) {
                field_id = INDEX(S_PCPf);
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_PORTMASK:
            if (field_index == DRV_MEM_FIELD_MASK_ANY) {
                field_id = INDEX(MASK_ANYf);
            } else if (field_index == DRV_MEM_FIELD_MASK_DLF_UCST) {
                field_id = INDEX(MASK_DLF_UCSTf);
            } else if (field_index == DRV_MEM_FIELD_MASK_DLF_L2MCST) {
                field_id = INDEX(MASK_DLF_L2MCSTf);
            } else if (field_index == DRV_MEM_FIELD_MASK_DLF_L3MCAST) {
                field_id = INDEX(MASK_DLF_L3MCASTf);
            } else if (field_index == DRV_MEM_FIELD_MASK_BCST) {
                field_id = INDEX(MASK_BCSTf);
            } else if (field_index == DRV_MEM_FIELD_PORT_CONFIG) {
                field_id = INDEX(PORT_CONFIGf);
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_SALRN_CNT_CTRL:

            if (field_index == DRV_MEM_FIELD_SA_LRN_CNT_LIM) {
                field_id = INDEX(SA_LRN_CNT_LIMf);
            } else if (field_index == DRV_MEM_FIELD_SA_LRN_CNT_NO) {
                field_id = INDEX(SA_LRN_CNT_NOf);
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MCAST_VPORT_MAP:

            /* Not implemented here for this memory is a bit map actually 
             *  - The definition of those fileds in this table is actually a 
             *      vport bitmap(bit0 to bit15 refelct the vp0 to vp15 )
             *  >> bitmap access will be proper in SW design. 
             */
            return SOC_E_UNAVAIL;
            
        case DRV_MEM_VPORT_VID_MAP:            
            if (field_index == DRV_MEM_FIELD_VPORT_UNTAG){
                field_id = INDEX(VPORT_UNTAGf);
            } else if (field_index == DRV_MEM_FIELD_VPORT_VID0){
                field_id = INDEX(VPORT_VID_0f);
            } else if (field_index == DRV_MEM_FIELD_VPORT_VID1){
                field_id = INDEX(VPORT_VID_1f);
            } else if (field_index == DRV_MEM_FIELD_VPORT_VID2){
                field_id = INDEX(VPORT_VID_2f);
            } else if (field_index == DRV_MEM_FIELD_VPORT_VID3){
                field_id = INDEX(VPORT_VID_3f);
            } else if (field_index == DRV_MEM_FIELD_VPORT_VID4){
                field_id = INDEX(VPORT_VID_4f);
            } else if (field_index == DRV_MEM_FIELD_VPORT_VID5){
                field_id = INDEX(VPORT_VID_5f);
            } else if (field_index == DRV_MEM_FIELD_VPORT_VID6){
                field_id = INDEX(VPORT_VID_6f);
            } else if (field_index == DRV_MEM_FIELD_VPORT_VID7){
                field_id = INDEX(VPORT_VID_7f);
            } else if (field_index == DRV_MEM_FIELD_VPORT_VID8){
                field_id = INDEX(VPORT_VID_8f);
            } else if (field_index == DRV_MEM_FIELD_VPORT_VID9){
                field_id = INDEX(VPORT_VID_9f);
            } else if (field_index == DRV_MEM_FIELD_VPORT_VID10){
                field_id = INDEX(VPORT_VID_10f);
            } else if (field_index == DRV_MEM_FIELD_VPORT_VID11){
                field_id = INDEX(VPORT_VID_11f);
            } else if (field_index == DRV_MEM_FIELD_VPORT_VID12){
                field_id = INDEX(VPORT_VID_12f);
            } else if (field_index == DRV_MEM_FIELD_VPORT_VID13){
                field_id = INDEX(VPORT_VID_13f);
            } else if (field_index == DRV_MEM_FIELD_VPORT_VID14){
                field_id = INDEX(VPORT_VID_14f);
            } else if (field_index == DRV_MEM_FIELD_VPORT_VID15){
                field_id = INDEX(VPORT_VID_15f);
            } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s,%d,TBD!(UNAVAIL)\n"), 
                           FUNCTION_NAME(), __LINE__));
                return SOC_E_UNAVAIL;
            }
            break;
        case DRV_MEM_MCAST:
            if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = INDEX(PBMPf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_ARL_HW:
        case DRV_MEM_MARL_HW:
        case DRV_MEM_ARL:
        case DRV_MEM_MARL:
            /* Note : both ARL/MARL table parsing is targeted at SW table:
             *  >> the only difference with HW table is MAC_ADDR format.
             *      (HW: keeps 36bits MAC_ADDR ; SW: keeps 48 bits MAC_ADDR)
             */
            if (field_index == DRV_MEM_FIELD_MAC) {
                /* for ARL_HW table, MACADDf will return bit12-bit47 only */
                field_id = INDEX(MACADDRf);
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = INDEX(VIDf);
            }else if (field_index == DRV_MEM_FIELD_AGE) {
                field_id = INDEX(AGEf);
            }else if (field_index == DRV_MEM_FIELD_STATIC) {
                field_id = INDEX(STATICf);
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VAf);
            }else if (field_index == DRV_MEM_FIELD_USER) {
                field_id = INDEX(USERf);
            }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
                field_id = INDEX(CONf);                
            }else {
                if (mem == DRV_MEM_ARL_HW || mem == DRV_MEM_ARL) {
                    if (field_index == DRV_MEM_FIELD_VPORT) {
                        field_id = INDEX(V_PORTIDf);
                    } else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
                        field_id = INDEX(PORTIDf);
                    } else {
                        /* special case to process the MARL MGID filed :
                         *  1. to get the MGID on a MARL entry.  (the ROBO 
                         *      chip on BCM design before TB all use 
                         *      DRV_MEM_ARL to retrieve MGID information.)
                         *  2. Another usage for TB is providing the short cut
                         *      to get l2_SW table entry bit48-bit59 raw data
                         *      by one time field get.
                         */
                        if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                            if (mem == DRV_MEM_ARL) {
                                mem_id = INDEX(L2_MARL_SWm);
                            } else if(mem == DRV_MEM_ARL_HW) {
                                mem_id = INDEX(L2_MARLm);
                            }
                            field_id = INDEX(MGIDf);
                        } else {
                            return SOC_E_PARAM;
                        }
                    }
                } else {    /* MARL || MARL_SW */
                    if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                        field_id = INDEX(MGIDf);
                    } else {
                        return SOC_E_PARAM;
                    }
                }
            }
            break;
        case DRV_MEM_IVM_KEY_DATA_MASK:
            if (field_index == DRV_MEM_FIELD_VM_KEY_DATA){
                field_id = INDEX(VM_KEY_DATAf);
            } else if (field_index == DRV_MEM_FIELD_VM_KEY_MASK){
                field_id = INDEX(VM_KEY_MASKf);
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_EVM_KEY_DATA_MASK:
            if (field_index == DRV_MEM_FIELD_VM_KEY_DATA){
                field_id = INDEX(VM_KEY_DATAf);
            } else if (field_index == DRV_MEM_FIELD_VM_KEY_MASK){
                field_id = INDEX(VM_KEY_MASKf);
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_IVM_ACT:
            if (field_index == DRV_VM_FIELD_IVM_SPCP_MARKING_POLICY){
                field_id = INDEX(SPCP_MARKf);
            } else if (field_index == DRV_VM_FIELD_IVM_CPCP_MARKING_POLICY){
                field_id = INDEX(CPCP_MARKf);
            } else if (field_index == DRV_VM_FIELD_IVM_VLAN_ID){
                field_id = INDEX(NEW_VIDf);
             } else if (field_index == DRV_VM_FIELD_IVM_FLOW_ID){
                field_id = INDEX(FLOW_IDf);
            } else if (field_index == DRV_VM_FIELD_IVM_VPORT_ID){
                field_id = INDEX(VPORT_IDf);
            } else if (field_index == DRV_VM_FIELD_IVM_VPORT_SPCP){
                field_id = INDEX(VPORT_SPCPf);
            } else if (field_index == DRV_VM_FIELD_IVM_VPORT_CPCP){
                field_id = INDEX(VPORT_CPCPf);
            } else if (field_index == DRV_VM_FIELD_IVM_VPORT_DP){
                field_id = INDEX(VPORT_DPf);
            } else if (field_index == DRV_VM_FIELD_IVM_VPORT_TC){
                field_id = INDEX(VPORT_TCf);
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_EVM_ACT:
            if (field_index == DRV_VM_FIELD_EVM_STAG_ACT){
                field_id = INDEX(STAG_ACTf);
            } else if (field_index == DRV_VM_FIELD_EVM_CTAG_ACT){
                field_id = INDEX(CTAG_ACTf);
            } else if (field_index == DRV_VM_FIELD_EVM_NEW_SVID){
                field_id = INDEX(NEW_SVIDf);
             } else if (field_index == DRV_VM_FIELD_EVM_NEW_CVID){
                field_id = INDEX(NEW_CVIDf);
            } else {
                return SOC_E_PARAM;
            }
            break;
        default:
            return SOC_E_UNAVAIL;
    }    
    *map_mem_id = mem_id;
    *map_field_id = field_id;
    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_mem_field_get
 *
 *  Purpose :
 *      Extract the value of a field from a memory entry value.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory indication.
 *      field_index    :  field type.
 *      entry   :   entry value pointer.
 *      fld_data   :   field value pointer.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      1.For DRV_MEM_MSTP, because there is no port information 
 *          on the parameter list, so it will return all the ports' state.
 *      2. For DRV_MEM_ARL, the entry type will be l2_arl_sw_entry_t 
 *          and the mem_id is L2_ARL_SWm.
 */
int
drv_tbx_mem_field_get(int unit, uint32 mem, 
    uint32 field_index, uint32 *entry, uint32 *fld_data)
{
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    uint32      mask = -1;
    uint32      mask_hi, mask_lo;
    int         mem_id = 0, field_id = INVALID_Rf;
    int         i, wp, bp, len;
#ifdef BE_HOST
    uint32              val32;
#endif
    int rv;

    if ((mem == INDEX(MSPT_TABm)) || (mem == DRV_MEM_MSTP)) {
        if (field_index == DRV_MEM_FIELD_MSTP_PORTST) {
                sal_memcpy(fld_data, entry, sizeof(mspt_tab_entry_t));
                return SOC_E_NONE;
        }
    }

    mem_id = -1;
    if (mem < DRV_MEM_ARL){
        for (i = 0; i < COUNTOF(drv_TB_mem_info); i++) {
            if ((drv_TB_mem_info[i].drv_mem == mem) || 
                (drv_TB_mem_info[i].soc_mem == mem)) {
                mem_id = mem;
                field_id = field_index;
                break;
            }    
        }
        if (!SOC_IS_TB_AX(unit)){
            for (i = 0; i < COUNTOF(drv_TB_cfp_mem_info_B0); i++) {
                if ((drv_TB_cfp_mem_info_B0[i].drv_mem == mem) || 
                    (drv_TB_cfp_mem_info_B0[i].soc_mem == mem)) {
                    mem_id = mem;
                    field_id = field_index;
                    break;
                }    
            }
        }
#ifdef BCM_VO_SUPPORT
        if (SOC_IS_VO(unit)) {
            for (i = 0; i < COUNTOF(drv_VO_cfp_mem_info); i++) {
                if ((drv_VO_cfp_mem_info[i].drv_mem == mem) || 
                    (drv_VO_cfp_mem_info[i].soc_mem == mem)) {
                    mem_id = mem;
                    field_id = field_index;
                    break;
                }    
            }
        }
#endif
        if (mem_id == -1){
            return SOC_E_UNAVAIL;
        }

    }
    else {
        rv = _tbx_mem_mapping(unit, mem, field_index, &mem_id, &field_id);
        SOC_IF_ERROR_RETURN(rv);
    }    
    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);

    assert(entry);
    assert(fld_data);

    SOC_FIND_FIELD(field_id,
         meminfo->fields,
         meminfo->nFields,
         fieldinfo);
    assert(fieldinfo);

    if (!fieldinfo){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s mem %d: no matched field %d\n"), 
                   FUNCTION_NAME(),mem_id, field_id));
        return SOC_E_PARAM;
    }

    bp = fieldinfo->bp;

    wp = bp / 32;
    bp = bp & (32 - 1);
    len = fieldinfo->len;

    /* field is 1-bit wide */
    if (len == 1) {
        fld_data[0] = ((entry[FIX_MEM_ORDER_E(wp, meminfo)] >> bp) & 1);
    } else {

    if (fieldinfo->flags & SOCF_LE) {
        for (i = 0; len > 0; len -= 32) {
            /* mask covers all bits in field. */
            /* if the field is wider than 32, takes 32 bits in each iteration */
            if (len >= 32) {
                mask = 0xffffffff;
            } else {
                mask = (1 << len) - 1;
            }
            /* the field may be splited across a 32-bit word boundary. */
            /* assume bp=0 to start with. Therefore, mask for higer word is 0 */
            mask_lo = mask;
            mask_hi = 0;
            /* if field is not aligned with 32-bit word boundary */
            /* adjust hi and lo masks accordingly. */
            if (bp) {
                mask_lo = mask << bp;
                mask_hi = mask >> (32 - bp);
            }
            /* get field value --- 32 bits each time */
            fld_data[i] = (entry[FIX_MEM_ORDER_E(wp++, meminfo)] 
                & mask_lo) >> bp;
            if (mask_hi) {
                fld_data[i] |= (entry[FIX_MEM_ORDER_E(wp, meminfo)] 
                    & mask_hi) << (32 - bp);
            }
            i++;
        }
    } else {
        i = (len - 1) / 32;

        while (len > 0) {
            assert(i >= 0);
            fld_data[i] = 0;
            do {
                fld_data[i] = (fld_data[i] << 1) |
                ((entry[FIX_MEM_ORDER_E(bp / 32, meminfo)] >>
                (bp & (32 - 1))) & 1);
                len--;
                bp++;
            } while (len & (32 - 1));
            i--;
        }
    }
    }
    if ((mem_id != INDEX(CFP_TCAM_SCm)) &&
        (mem_id != INDEX(CFP_TCAM_MASKm)) &&
        (mem_id != INDEX(CFP_TCAM_CHAIN_SCm)) &&
        (mem_id != INDEX(CFP_TCAM_CHAIN_MASKm))) {
#ifdef BE_HOST
        if ((fieldinfo->len > 32) && (fieldinfo->len <= 64)){
            val32 = fld_data[0];
            fld_data[0] = fld_data[1];
            fld_data[1] = val32;
        }
#endif
    }
    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_mem_field_set
 *
 *  Purpose :
 *      Set the value of a field in a 8-, 16-, 32, and 64-bit memory's value.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory indication.
 *      field_index    :  field type.
 *      entry   :   entry value pointer.
 *      fld_data   :   field value pointer.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      1.For DRV_MEM_MSTP, because there is no port information 
 *          on the parameter list, so it will set the value on the 
 *          fld_data to memory entry.
 *      2. For DRV_MEM_ARL, the entry type will be l2_arl_sw_entry_t 
 *          and the mem_id is L2_ARL_SWm.
 */
int
drv_tbx_mem_field_set(int unit, uint32 mem, 
    uint32 field_index, uint32 *entry, uint32 *fld_data)
{
    soc_mem_info_t      *meminfo;
    soc_field_info_t    *fieldinfo;
    uint32              mask, mask_hi, mask_lo;
    int                 mem_id, field_id = INVALID_Rf;
    int                 i, wp, bp, len;
#ifdef BE_HOST
    uint32              val32;
#endif
    int rv;

    if ((mem == INDEX(MSPT_TABm)) ||(mem == DRV_MEM_MSTP)) {
        if (field_index == DRV_MEM_FIELD_MSTP_PORTST) {
                sal_memcpy(entry, fld_data, sizeof(mspt_tab_entry_t));
                return SOC_E_NONE;
        }
    }

    mem_id = -1;
    if(mem < DRV_MEM_ARL){
        for (i = 0; i < COUNTOF(drv_TB_mem_info); i++) {
            if ((drv_TB_mem_info[i].drv_mem == mem) || 
                (drv_TB_mem_info[i].soc_mem == mem)) {
                mem_id = mem;
                field_id = field_index;
                break;
            }    
        }
        if (!SOC_IS_TB_AX(unit)){    
            for (i = 0; i < COUNTOF(drv_TB_cfp_mem_info_B0); i++) {
                if ((drv_TB_cfp_mem_info_B0[i].drv_mem == mem) || 
                    (drv_TB_cfp_mem_info_B0[i].soc_mem == mem)) {
                    mem_id = mem;
                    field_id = field_index;
                    break;
                }    
            }
        }
#ifdef BCM_VO_SUPPORT
        if (SOC_IS_VO(unit)) {
            for (i = 0; i < COUNTOF(drv_VO_cfp_mem_info); i++) {
                if ((drv_VO_cfp_mem_info[i].drv_mem == mem) || 
                    (drv_VO_cfp_mem_info[i].soc_mem == mem)) {
                    mem_id = mem;
                    field_id = field_index;
                    break;
                }    
            }
        }
#endif        
        if (mem_id == -1){
            return SOC_E_UNAVAIL;
        }
    }
    else {
        rv = _tbx_mem_mapping(unit, mem, field_index, &mem_id, &field_id);
        SOC_IF_ERROR_RETURN(rv);
    }    

    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);

    assert(meminfo);
    assert(entry);
    assert(fld_data);
    SOC_FIND_FIELD(field_id,
         meminfo->fields,
         meminfo->nFields,
         fieldinfo);

    assert(fieldinfo);
    if (!fieldinfo){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s mem %d: no matched field %d\n"), 
                   FUNCTION_NAME(),mem_id, field_id));
        return SOC_E_PARAM;
    }


    if ((mem_id != INDEX(CFP_TCAM_SCm)) &&
        (mem_id != INDEX(CFP_TCAM_MASKm)) &&
        (mem_id != INDEX(CFP_TCAM_CHAIN_SCm)) &&
        (mem_id != INDEX(CFP_TCAM_CHAIN_MASKm))) {
#ifdef BE_HOST
        if ((fieldinfo->len > 32) && (fieldinfo->len <= 64)){
            val32 = fld_data[0];
            fld_data[0] = fld_data[1];
            fld_data[1] = val32;
        }
#endif
    }
    bp = fieldinfo->bp;
    if (fieldinfo->flags & SOCF_LE) {
        wp = bp / 32;
        bp = bp & (32 - 1);
        i = 0;

        for (len = fieldinfo->len; len > 0; len -= 32) {
            /* mask covers all bits in field. */
            /* if the field is wider than 32, takes 32 bits in each iteration */
            if (len >= 32) {
                mask = 0xffffffff;
            } else {
                mask = (1 << len) - 1;
            }
            /* the field may be splited across a 32-bit word boundary. */
            /* assume bp=0 to start with. Therefore, mask for higer word is 0 */
            mask_lo = mask;
            mask_hi = 0;

            /* if field is not aligned with 32-bit word boundary */
            /* adjust hi and lo masks accordingly. */
            if (bp) {
                mask_lo = mask << bp;
                mask_hi = mask >> (32 - bp);
            }

            /* set field value --- 32 bits each time */
            entry[FIX_MEM_ORDER_E(wp, meminfo)] &= ~mask_lo;
            entry[FIX_MEM_ORDER_E(wp++, meminfo)] |= 
                ((fld_data[i] << bp) & mask_lo);
            if (mask_hi) {
                entry[FIX_MEM_ORDER_E(wp, meminfo)] &= ~(mask_hi);
                entry[FIX_MEM_ORDER_E(wp, meminfo)] |= 
                    ((fld_data[i] >> (32 - bp)) & mask_hi);
            }

            i++;
        }
    } else {                   
        /* Big endian: swap bits */
        len = fieldinfo->len;

        while (len > 0) {
            len--;
            entry[FIX_MEM_ORDER_E(bp / 32, meminfo)] &= ~(1 << (bp & (32-1)));
            entry[FIX_MEM_ORDER_E(bp / 32, meminfo)] |=
            (fld_data[len / 32] >> (len & (32-1)) & 1) << (bp & (32-1));
            bp++;
        }
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_mem_length_get
 *
 *  Purpose :
 *      Get the number of entries of the selected memory.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      data   :   total number entries of this memory type.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_tbx_mem_length_get(int unit, uint32 mem, uint32 *data)
{
    int mem_id;
    soc_mem_info_t *meminfo;
    int rv;

    rv = _tbx_mem_mapping(unit, mem, -1, &mem_id, NULL);
    SOC_IF_ERROR_RETURN(rv);

    meminfo = &SOC_MEM_INFO(unit, mem_id);    
    *data = meminfo->index_max - meminfo->index_min + 1;

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_mem_read
 *
 *  Purpose :
 *      Get the width of selected memory. 
 *      The value returned in data is width in bits.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   the memory type to access.
 *      entry_id    :  the entry's index of the memory to be read.
 *      count   :   one or more netries to be read.
 *      entry_data   :   pointer to a buffer of 32-bit words 
 *                              to contain the read result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 *  1. ARL/MARL read OP is working on reading the 4K basis etnry_id and chip 
 *      will report all 4 bins entry data on this table address.
 *      - the entry_id input is 16K basis, SW must translate this id to 
 *          proper index format for ARL table read.
 *  2. TB new feature for ARL read is the complete MAC address can be repoted 
 *      as well. This routine for ARL read will report the l2 entry with full 
 *      MAC address(48 bits length).
 *      ### the full MAC addr is 
 */
int
drv_tbx_mem_read(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int i, j, k;
    uint32 retry, index_min, index_max;
    int mem_id, table_index;
    uint32 reg_val, temp;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    soc_control_t           *soc = SOC_CONTROL(unit);
    int mem_len;
    uint64 temp_data, temp_key;
    int this_read_id, prev_read_id = -1;
    int real_read = TRUE;   
    uint16  mac_lsb = 0;      /* for TB's ARL read */
    uint32  tmp_buf = 0;
    uint32  temp_hi, temp_lo;
    char *s;
    uint32 table_mask=-1;
    uint32 *temp_entry = NULL;
#ifdef BCM_VO_SUPPORT
    uint32 field_change = 0;
#endif

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_mem_read(mem=0x%x,entry_id=0x%x,count=%d)\n"),
              mem, entry_id, count));

    COMPILER_64_ZERO(temp_data);
    COMPILER_64_ZERO(temp_key);

    mem_id = -1;
    table_index = -1;

    if(mem < DRV_MEM_ARL){
        for (i = 0; i < COUNTOF(drv_TB_mem_info); i++) {
            if ((drv_TB_mem_info[i].drv_mem == mem) 
                || (drv_TB_mem_info[i].soc_mem == mem)) {
                mem_id = drv_TB_mem_info[i].soc_mem;
                table_index = drv_TB_mem_info[i].table_index;
                table_mask = drv_TB_mem_info[i].table_valid_mask;
                break;
            }
        }
        if (!SOC_IS_TB_AX(unit)){
            for (i = 0; i < COUNTOF(drv_TB_cfp_mem_info_B0); i++) {
                if ((drv_TB_cfp_mem_info_B0[i].drv_mem == mem) 
                    || (drv_TB_cfp_mem_info_B0[i].soc_mem == mem)) {
                    mem_id = drv_TB_cfp_mem_info_B0[i].soc_mem;
                    table_index = drv_TB_cfp_mem_info_B0[i].table_index;
                    table_mask = drv_TB_cfp_mem_info_B0[i].table_valid_mask;
                    break;
                }    
            }
        }
#ifdef BCM_VO_SUPPORT
        if (SOC_IS_VO(unit)) {
            for (i = 0; i < COUNTOF(drv_VO_cfp_mem_info); i++) {
                if ((drv_VO_cfp_mem_info[i].drv_mem == mem) || 
                    (drv_VO_cfp_mem_info[i].soc_mem == mem)) {
                    mem_id = drv_VO_cfp_mem_info[i].soc_mem;
                    table_index = drv_VO_cfp_mem_info[i].table_index;
                    table_mask = drv_VO_cfp_mem_info[i].table_valid_mask;
                    break;
                }    
            }

        }
#endif  
    }
    else {
        for (i = 0; i < COUNTOF(drv_TB_mem_info); i++) {
            if (drv_TB_mem_info[i].drv_mem == mem){
                mem_id = drv_TB_mem_info[i].soc_mem;
                table_index = drv_TB_mem_info[i].table_index;
                break;
            }    
        }
        if (!SOC_IS_TB_AX(unit)){
            for (i = 0; i < COUNTOF(drv_TB_cfp_mem_info_B0); i++) {
                if (drv_TB_cfp_mem_info_B0[i].drv_mem == mem){
                    mem_id = drv_TB_cfp_mem_info_B0[i].soc_mem;
                    table_index = drv_TB_cfp_mem_info_B0[i].table_index;
                    break;
                }    
            }
        }
#ifdef BCM_VO_SUPPORT
        if (SOC_IS_VO(unit)) {
            for (i = 0; i < COUNTOF(drv_VO_cfp_mem_info); i++) {
                if ((drv_VO_cfp_mem_info[i].drv_mem == mem) || 
                    (drv_VO_cfp_mem_info[i].soc_mem == mem)) {
                    mem_id = drv_VO_cfp_mem_info[i].soc_mem;
                    table_index = drv_VO_cfp_mem_info[i].table_index;
                    break;
                }    
            }

        }
#endif  

    }    

        if (mem_id == -1){
            return SOC_E_UNAVAIL;
        }
#ifdef BCM_VO_SUPPORT
    if (SOC_IS_VO(unit)){
        if ((mem == INDEX(CFP_TCAM_SCm)) || 
            (mem == INDEX(CFP_TCAM_MASKm)) ||
            (mem == INDEX(CFP_TCAM_CHAIN_SCm)) ||
            (mem == INDEX(CFP_TCAM_CHAIN_MASKm))) {
            field_change = 1;
        }    
    }
#endif        

    s = soc_property_get_str(unit, "board_name");
    if( (s != NULL) && (sal_strcmp(s, "bcm53280_fpga") == 0)) {
        if ((TABLE(table_index) == CFP_DATA_TABLE)||
            (table_index == CFP_ACTION_TABLE)||
            (table_index == CFP_METER_TABLE)||
            (table_index == CFP_STAT_TABLE)){
            entry_id = (entry_id/2) *256 + entry_id%2;
            entry_id = entry_id % 1536;
        }
    }
    /* add code here to check addr */
    mem_len = soc_mem_entry_words(unit, mem_id);
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);

    /* check the valid entry_id and the requested count */    
    if (((entry_id) < index_min) || 
            ((entry_id + count - 1) > index_max)) {
        if ((count < 1) || (((entry_id + count - 1) > index_max) && 
                ((entry_id) < index_max))){
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,mem_id=0x%x,entry_id=0x%x, invlaid count=%d\n"),
                      FUNCTION_NAME(), mem_id, entry_id, count));
        } else {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,mem_id=0x%x, invalid entry_id=0x%x\n"),
                      FUNCTION_NAME(), mem_id, entry_id));
        }
        return SOC_E_PARAM;
    }
#ifdef BCM_VO_SUPPORT
    if (SOC_IS_VO(unit) && (field_change == 1)){
        temp_entry = (uint32 *)sal_alloc(entry_size,"mem_read temp entry");
        if (temp_entry == NULL) {
            return SOC_E_RESOURCE;
        }
    } else 
#endif    
    {
        temp_entry = entry;
    }
        
    /* process read action */
    MEM_RWCTRL_REG_LOCK(soc);
    if (TABLE_INDICATION(table_index)) {
        VO_ARL_SEARCH_LOCK(unit,soc);
    }        
    for (i = 0;i < count; i++ ) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            rv = SOC_E_PARAM;
            goto mem_read_exit;
        }

        /* Return data from cache if active */
        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL && CACHE_VMAP_TST(vmap, (entry_id + i))) {
            sal_memcpy(entry, cache + (entry_id + i) * entry_size, entry_size);
            continue;
        }

        reg_val = 0;
        temp = TABLE(table_index);
        soc_MEM_INDEXr_field_set(unit, &reg_val, INDEXf, &temp);
        rv = REG_WRITE_MEM_INDEXr(unit, &reg_val);
        if (rv < 0) {
            goto mem_read_exit;
        }
        /* Set memory index. */
        if (table_index == ARL_TABLE) {
            this_read_id  = _TB_ARL_INDEX_TABLE_ID_GET(entry_id + i);
            /* TB's ARL table index read can retrieve all 4 bins entry data.
             *  if user request a read with read_count > 0, we can have the 
             *  process below to prevent the redundant ARL_READ operation.
             */
            if (prev_read_id == this_read_id){
                real_read = FALSE;
            } else {
                real_read = TRUE;
            }
        } else {
            this_read_id = entry_id + i;
        }

        if (real_read){
            reg_val = 0;
            soc_MEM_ADDR_0r_field_set(unit, 
                    &reg_val, MEM_ADDR_OFFSETf, (uint32 *)&this_read_id);
            rv = REG_WRITE_MEM_ADDR_0r(unit, &reg_val);
            if (rv < 0) {
                goto mem_read_exit;
            }
    
            /* Read memory control register */
            reg_val = 0;
            temp = MEM_OP_READ;
            soc_MEM_CTRLr_field_set(unit, &reg_val, OP_CMDf, &temp);
            temp = 1;
            soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_STDNf, &temp);
            rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
            if (rv < 0) {
                goto mem_read_exit;
            }
    
            /* wait for complete */
            for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
                rv = REG_READ_MEM_CTRLr(unit, &reg_val);
                if (rv < 0) {
                    goto mem_read_exit;
                }
                soc_MEM_CTRLr_field_get(unit, &reg_val, MEM_STDNf, &temp);
                if (!temp) {
                    break;
                }
            }
            if (retry >= SOC_TIMEOUT_VAL) {
                rv = SOC_E_TIMEOUT;
                goto mem_read_exit;
            }
            
            if (table_index == ARL_TABLE) {
                prev_read_id = this_read_id;
            }
        }
        k = 0;
        for (j = 0; j < (mem_len + 1)/2; j++) {
            if (table_index == ARL_TABLE) {
                /* retrieve the bin id, each arl read get 4 bins' data.
                 *  >>  data0/1 for bin0; data2/3 for bin1; 
                 *  >>  data4/5 for bin2; data6/7 for bin3
                 *
                 * besides, the arl entry on MAC_ADDR only keeps bit12~48 for 
                 *  the hash. TB provide MEM_KEY to retrieve bit0-bit11 for 
                 *  complete the MAC address on each bin.
                 *  >> key2 for bin0; key3 for bin1
                 *  >> key4 for bin2; key5 for bin3
                 */
                temp  = (entry_id + i) & _TB_ARL_INDEX_BIN_ID_MASK;
                rv = MEM_DATA_READ(unit, j + (temp * 2), &temp_data);
                /* read ARL/MARL will report full MAC.
                 * if the read is on ARL_HW, no MAC completing process.
                 */
                if (mem == DRV_MEM_ARL || mem == DRV_MEM_MARL ||
                    mem == INDEX(L2_ARL_SWm)||mem == INDEX(L2_MARL_SWm)){
                    if (j == 0){     /* retrieve the the LSB of MAC_Addr here */
                        /* fixing l2 entry processes : 
                         *  1. insert the mist MAC_ADDR 
                         *  2. keep the 12 bits overflow entry data.
                         */
                        rv = MEM_KEY_READ(unit, j + temp + 2, &temp_key);
                        if (SOC_FAILURE(rv)){
                            goto mem_read_exit;
                        }
                        mac_lsb = COMPILER_64_LO(temp_key) &  _TB_MIST_MACADDR_MASK;

                        tmp_buf = (COMPILER_64_HI(temp_data) >> 
                                (32 - _TB_MIST_MACADDR_SHIFT)) & 
                                _TB_MIST_MACADDR_MASK;
                        COMPILER_64_SHL(temp_data, _TB_MIST_MACADDR_SHIFT);
                        temp_hi = COMPILER_64_HI(temp_data);
                        temp_lo = COMPILER_64_LO(temp_data);
                        COMPILER_64_SET(temp_data, temp_hi, 
                                temp_lo | mac_lsb);
                    } else {    /* j==1 */
                        /* when j==1, there are 5 bits data in temp_data */
    
                        /* get l2 entry bit64-bit68, and 
                         *  1. insert 12 bits overflow entry data.
                         */
                        COMPILER_64_SHL(temp_data, _TB_MIST_MACADDR_LENGTH);
                        temp_hi = COMPILER_64_HI(temp_data);
                        temp_lo = COMPILER_64_LO(temp_data);
                        COMPILER_64_SET(temp_data, temp_hi, 
                                temp_lo | tmp_buf);
                    }
                }
                if (rv < 0) {
                    goto mem_read_exit;
                }
            } else {
#ifdef BCM_VO_SUPPORT
            if (SOC_IS_VO(unit) && TABLE_INDICATION(table_index)) {
                 if(table_index == CFP_INDICATION_MASK_TABLE){
                    temp = 1;
                } else {
                    if (j == 0) {
                        temp = 0;
                    } 
                    if (j == 8) {
                        temp = 1;
                    } 
                }
                if ((j % 8) == 0){
                    reg_val = 0;
                    soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_INDICATIONf, &temp);
                    rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
                }
            }
#endif 
                rv = MEM_DATA_READ(unit, j%8, &temp_data);
            }

            if (rv < 0) {
                goto mem_read_exit;
            }
            
            /* endian handler */
            if (table_mask & (1<< (2*j))){            
                *(temp_entry+(2*k)) =  COMPILER_64_LO(temp_data);

            }
            if(mem_len > (2*j+1) ){
                if (table_mask & (1<< (2*j+1))){
                     *(temp_entry+(2*k+1)) = COMPILER_64_HI(temp_data);
                        k++;
                }
           }
        }
#ifdef BCM_VO_SUPPORT
        if (SOC_IS_VO(unit) && (field_change == 1)){
            rv = _drv_vo_cfp_mem_hw2sw(unit, temp_entry, entry);
            if (rv < 0) {
                goto mem_read_exit;
            }            
        } 
#endif        
    }
    if (TABLE_INDICATION(table_index)) {
        reg_val = 0;
        rv = REG_WRITE_MEM_INDEXr(unit, &reg_val);
        if (SOC_FAILURE(rv)) {
            goto mem_read_exit;
        }
    }
 mem_read_exit:
    if (TABLE_INDICATION(table_index)) {
        VO_ARL_SEARCH_UNLOCK(unit,soc);
    }        

    MEM_RWCTRL_REG_UNLOCK(soc);
#ifdef BCM_VO_SUPPORT
    if (SOC_IS_VO(unit) && (field_change == 1)){
        sal_free(temp_entry);
    }
#endif    
    return rv;

}

/*
 *  Function : drv_tbx_mem_width_get
 *
 *  Purpose :
 *      Get the width of selected memory. 
 *      The value returned in data is width in bits.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      data   :   total number bits of entry.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_tbx_mem_width_get(int unit, uint32 mem, uint32 *data)
{
    int mem_id, memlen;
    int rv;

    rv = _tbx_mem_mapping(unit, mem, -1, &mem_id, NULL);
    SOC_IF_ERROR_RETURN(rv);


    memlen = soc_mem_entry_words(unit, mem_id);

    *data = memlen * sizeof(uint32);

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_mem_write
 *
 *  Purpose :
 *      Writes an internal memory.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   the memory type to access.
 *      entry_id    :  the entry's index of the memory to be written.
 *      count   :   one or more netries to be written.
 *      entry_data   :   pointer to a buffer of 32-bit words 
 *                              to contain the writting result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
drv_tbx_mem_write(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    uint32 retry, index_min, index_max;
    uint32 i;
    int mem_id, table_index;
    uint32 reg_val = 0;
    uint32  temp;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    soc_control_t           *soc = SOC_CONTROL(unit);
    int mem_len;
    int this_arl_id = 0, temp_arl_id = 0, bin_id = 0;
    int is_arl = 0, arl_id_change = 0;
    uint64 temp_data;
    int j;
    uint32 *gmem_entry;
    uint32 val32_hi,val32_lo;
    char *s;
    uint32 table_mask=-1;
    
    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_mem_write(mem=0x%x,entry_id=0x%x,count=%d)\n"),
              mem, entry_id, count));

    mem_id = -1;
    table_index = -1;
    COMPILER_64_ZERO(temp_data);
    
    if(mem < DRV_MEM_ARL){
        for (i = 0; i < COUNTOF(drv_TB_mem_info); i++) {
            if ((drv_TB_mem_info[i].drv_mem == mem) 
                || (drv_TB_mem_info[i].soc_mem == mem)) {
                mem_id = drv_TB_mem_info[i].soc_mem;
                table_index = drv_TB_mem_info[i].table_index;
                table_mask = drv_TB_mem_info[i].table_valid_mask;
                break;
            }
        }
        if (!SOC_IS_TB_AX(unit)){
            for (i = 0; i < COUNTOF(drv_TB_cfp_mem_info_B0); i++) {
                if ((drv_TB_cfp_mem_info_B0[i].drv_mem == mem) 
                    || (drv_TB_cfp_mem_info_B0[i].soc_mem == mem)) {
                    mem_id = drv_TB_cfp_mem_info_B0[i].soc_mem;
                    table_index = drv_TB_cfp_mem_info_B0[i].table_index;
                    table_mask = drv_TB_cfp_mem_info_B0[i].table_valid_mask;
                    break;
                }    
            }
        }
#ifdef BCM_VO_SUPPORT
        if (SOC_IS_VO(unit)) {
            for (i = 0; i < COUNTOF(drv_VO_cfp_mem_info); i++) {
                if ((drv_VO_cfp_mem_info[i].drv_mem == mem) || 
                    (drv_VO_cfp_mem_info[i].soc_mem == mem)) {
                    mem_id = drv_VO_cfp_mem_info[i].soc_mem;
                    table_index = drv_VO_cfp_mem_info[i].table_index;
                    table_mask = drv_VO_cfp_mem_info[i].table_valid_mask;
                    break;
                }    
            }
        }
#endif  

        if (mem_id == -1){
            return SOC_E_UNAVAIL;
        }

    }
    else {

        /* ============== No Write Table =============== */
        if (mem == DRV_MEM_SALRN_CNT_CTRL){     /* reserved for engineering */
            return SOC_E_UNAVAIL;
        }
            
        for (i = 0; i < COUNTOF(drv_TB_mem_info); i++) {
            if (drv_TB_mem_info[i].drv_mem == mem){
                mem_id = drv_TB_mem_info[i].soc_mem;
                table_index = drv_TB_mem_info[i].table_index;
                break;
            }    
        }
        if (!SOC_IS_TB_AX(unit)){
            for (i = 0; i < COUNTOF(drv_TB_cfp_mem_info_B0); i++) {
                if (drv_TB_cfp_mem_info_B0[i].drv_mem == mem){
                    mem_id = drv_TB_cfp_mem_info_B0[i].soc_mem;
                    table_index = drv_TB_cfp_mem_info_B0[i].table_index;
                    break;
                }    
            }
        }
#ifdef BCM_VO_SUPPORT
        if (SOC_IS_VO(unit)) {
            for (i = 0; i < COUNTOF(drv_VO_cfp_mem_info); i++) {
                if ((drv_VO_cfp_mem_info[i].drv_mem == mem) || 
                    (drv_VO_cfp_mem_info[i].soc_mem == mem)) {
                    mem_id = drv_VO_cfp_mem_info[i].soc_mem;
                    table_index = drv_VO_cfp_mem_info[i].table_index;
                    table_mask = drv_VO_cfp_mem_info[i].table_valid_mask;
                    break;
                }    
            }
        }
#endif /* BCM_VO_SUPPORT */
        if (mem_id == -1){
            return SOC_E_UNAVAIL;
        }

    }

    if (table_index == ARL_TABLE) {
        is_arl = 1;
        this_arl_id  = _TB_ARL_INDEX_TABLE_ID_GET(entry_id);
        arl_id_change = 1;
    }

    s = soc_property_get_str(unit, "board_name");
    if( (s != NULL) && (sal_strcmp(s, "bcm53280_fpga") == 0)) {
        if ((TABLE(table_index) == CFP_DATA_TABLE)||
            (table_index == CFP_ACTION_TABLE)||
            (table_index == CFP_METER_TABLE)||
            (table_index == CFP_STAT_TABLE)){
            entry_id = (entry_id/2) *256 + entry_id%2;
            entry_id = entry_id % 1536;
        }
    }

#ifdef BCM_VO_SUPPORT
    if (TABLE_INDICATION(table_index)) {
        return _vo_cfp_mem_modify(unit, mem, entry_id, count, entry);
    }
#endif /* BCM_VO_SUPPORT */

    if (table_mask != -1){
        return  _tb_mem_modify(unit, mem, 
            entry_id, count, table_mask, entry);
    }

    /* add code here to check addr */
    mem_len = soc_mem_entry_words(unit, mem_id);
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);

    /* check the valid entry_id and the requested count */    
    if (((entry_id) < index_min) || 
            ((entry_id + count - 1) > index_max)) {
        if ((count < 1) || (((entry_id + count - 1) > index_max) && 
                ((entry_id) < index_max))){
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,mem_id=0x%x,entry_id=0x%x, invlaid count=%d\n"),
                      FUNCTION_NAME(), mem_id, entry_id, count));
        } else {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,mem_id=0x%x, invalid entry_id=0x%x\n"),
                      FUNCTION_NAME(), mem_id, entry_id));
        }
        return SOC_E_PARAM;
    }

    gmem_entry = entry;
    
    /* process write action */
    MEM_RWCTRL_REG_LOCK(soc);
    for (i = 0; i < count; i++) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            rv = SOC_E_PARAM;
            goto mem_write_exit;            
        }

        /* Decide which table to be written. */
        reg_val = 0;
        temp = TABLE(table_index);
        soc_MEM_INDEXr_field_set(unit, &reg_val, INDEXf, &temp);
        rv = REG_WRITE_MEM_INDEXr(unit, &reg_val);
        if (rv < 0) {
            goto mem_write_exit;
        }

        if(is_arl){
            /* retrieve the ARL bin id from the given entry index */
            bin_id = (entry_id + i) & _TB_ARL_INDEX_BIN_ID_MASK;

            /* there are 4 bins' ARL entries in Thunderbolt for every HW entry index.
             * 
             * For performance issue, no new READ op will be performed if the 
             *  target arl entry is observed at bin id changed only.
             */
            if (i > 0) {    /* multiple entry write preocess */
                temp_arl_id = _TB_ARL_INDEX_TABLE_ID_GET(entry_id + i);
                if (this_arl_id == temp_arl_id){
                    arl_id_change = 0;
                } else {
                    arl_id_change = 0;
                    this_arl_id = temp_arl_id;
                }
            }
        
            /* read OP required first for each write operation will overried 
             * 4 bins arl entries.
             */
            if (arl_id_change) {
                reg_val = 0;
                temp = MEM_OP_READ;
                soc_MEM_CTRLr_field_set(unit, &reg_val, OP_CMDf, &temp);
                temp = 1;
                soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_STDNf, &temp);
                rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
                if (rv < 0) {
                    MEM_RWCTRL_REG_UNLOCK(soc);
                    goto mem_write_exit;
                }
            
                /* wait for complete */
                for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
                    rv = REG_READ_MEM_CTRLr(unit, &reg_val);
                    if (rv < 0) {
                        goto mem_write_exit;
                    }
                    soc_MEM_CTRLr_field_get(unit, &reg_val, MEM_STDNf, &temp);
                    if (!temp) {
                        break;
                    }
                }

                if (retry >= SOC_TIMEOUT_VAL) {
                    rv = SOC_E_TIMEOUT;
                    goto mem_write_exit;
                }
            }
        }

        /* Set memory index. */
        reg_val = 0;
        if (is_arl){
            temp = this_arl_id + i;
        } else {
            temp = entry_id + i;
        }
        soc_MEM_ADDR_0r_field_set(unit, &reg_val, MEM_ADDR_OFFSETf, &temp);
        rv = REG_WRITE_MEM_ADDR_0r(unit, &reg_val);
        if (rv < 0) {
            goto mem_write_exit;
        }

        val32_hi = 0;
        val32_lo = 0;
        /* write data */
        for (j = 0; j <  (mem_len + 1)/2; j++) {
            val32_lo = *entry;
            if(mem_len > (2*j+1) ){
                entry ++;
                val32_hi = *entry;
            } else {
                val32_hi = 0;
            }

            COMPILER_64_SET(temp_data, val32_hi, val32_lo);
            entry++;            
            if (is_arl){
                temp = j + (bin_id * 2);
            } else {
                temp = j;
            }
            rv = MEM_DATA_WRITE(unit, temp, &temp_data);
            if (rv < 0) {
                goto mem_write_exit;
            }
        }

        /* Write memory control register */
        reg_val = 0;
        temp = MEM_OP_WRITE;
        soc_MEM_CTRLr_field_set(unit, &reg_val, OP_CMDf, &temp);
        temp = 1;
        soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_STDNf, &temp);
        rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
        if (rv < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_write_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            rv = REG_READ_MEM_CTRLr(unit, &reg_val);
            if (rv < 0) {
                goto mem_write_exit;
            }
            soc_MEM_CTRLr_field_get(unit, &reg_val, MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_write_exit;
        }
        
        /* Write back to cache if active */
        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL) {
            sal_memcpy(cache + (entry_id + i) * entry_size, gmem_entry, 
                    entry_size);
            CACHE_VMAP_SET(vmap, (entry_id + i));
        }

    }

mem_write_exit:
    MEM_RWCTRL_REG_UNLOCK(soc);
    return rv;
}

/*
 *  Function : drv_tbx_mem_search
 *
 *  Purpose :
 *      Search selected memory for the key value
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      key   :   the pointer of the data to be search.
 *      entry     :   entry data pointer (if found).
 *      flags     :   search flags.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. the flag on "DRV_MEM_OP_BY_INDEX" for all ROBO device before TB were 
 *      implemented to report the l2_ARL raw data(HW data entry). This flag 
 *      for TB will not be implemented. Such request may be approach through 
 *      mem_read on L2_ARL_HW table indicated with entry_id assigned.
 *
 */
int 
drv_tbx_mem_search(int unit, uint32 mem, 
    uint32 *key, uint32 *entry, uint32 *entry_1, uint32 flags)
{
    soc_control_t   *soc = SOC_CONTROL(unit);
    int     rv = SOC_E_NONE;
    int     search_arl_index[ROBO_TBX_L2_BUCKET_SIZE];

    switch(mem) {
    case DRV_MEM_ARL:
    case DRV_MEM_MARL:
        break;
    default:
        return SOC_E_PARAM;
    }

    if ((flags & DRV_MEM_OP_SEARCH_DONE) ||
        (flags & DRV_MEM_OP_SEARCH_VALID_START) ||
        (flags & DRV_MEM_OP_SEARCH_VALID_GET)){
            
        /* the entry and entry_1 parameter for valid search must be L2_ARL_SW
         *  format. (full MAC address in L2_ARL_SW entry)
         */
        rv = _drv_tbx_search_valid_op(unit, key, entry, entry_1, flags);
    } else if (flags & DRV_MEM_OP_BY_INDEX) {
        /* This is Not implemented for TB. 
         * "mem_read" on DRV_MEM_ARL_HW with entry_id assigned can approach  
         *  this request.
         */
        rv = SOC_E_UNAVAIL;
    /* TB have no feature on searching OP by MAC key only without VID key */
    } else if ((flags & DRV_MEM_OP_BY_HASH_BY_MAC)  && 
            (flags & DRV_MEM_OP_BY_HASH_BY_VLANID)){

        MEM_RWCTRL_REG_LOCK(soc);
        sal_memset(search_arl_index, 0, 
                sizeof(int) * ROBO_TBX_L2_BUCKET_SIZE);
        if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
            /* this flag is used to retrieve all valid l2 entries in the 
             *  ARL hashed bucket.
             *  - TB has dual hash mechanism, thus the conflict seach need to 
             *      report all those valid and conflict entries on those dual 
             *      hashed buckets.
             */
            rv = _drv_tbx_search_key_op(unit, key, 
                    _TB_ARL_SEARCH_CONFLICT, entry, search_arl_index);
        } else {
            /* TB specific read operation, "index read" for MAC+VID search */
            
            /* For index read op :
             *   1. the "key" must be at the format of sw_arl 
             *   2. and the "key" must contains the seaching key on MAC+VID to 
             *      call _drv_tbx_index_read_op() 
             */
            rv = _drv_tbx_search_key_op(unit, key, 
                    _TB_ARL_SEARCH_MATCH, entry, search_arl_index);
        }
        MEM_RWCTRL_REG_UNLOCK(soc);
    } else {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s, flags=0x%x indicated no major search operation!\n"),
                  FUNCTION_NAME(), flags));
        rv = SOC_E_PARAM;
    }
    
    return rv;
}

/* for the condition id to represent the entry static status change */
#define _DRV_ST_OVERRIDE_NO_CHANGE  0
#define _DRV_ST_OVERRIDE_DYN2ST     1
#define _DRV_ST_OVERRIDE_ST2DYN     2

/*
 *  Function : drv_tbx_mem_insert
 *
 *  Purpose :
 *      Insert an entry to specific memory
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      entry     :   entry data pointer.
 *      flags     :   insert flags (no use now).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      1. Only for ARL memory now.
 *      2. The DRV_MEM_OP_REPLACE from BCM API to call is based on 
 *          BCM_L2_REPLACE_DYNAMIC. The "BCM_L2_REPLACE_DYNAMIC" is defined 
 *          to indicate the replacing is will replace the dynamic entry when 
 *          the ARL table FULL. Thus the DRV_MEM_OP_REPLACE in this routine 
 *          will be more proper to handle the insert by replacing 
 *          non-static entry.
 *      3.  DRV_MEM_OP_PENDING is used to insert an arl entry with pending 
 *          statsu.
 *  
 */
int 
drv_tbx_mem_insert(int unit, uint32 mem, uint32 *entry, 
        uint32 flags)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    l2_arl_sw_entry_t   temp_sw_arl;
    l2_arl_entry_t   temp_hw_arl;
    int         rv = SOC_E_NONE, sw_arl_update = 0;
    int         i, table_index, bin_id;
    int         is_dynamic = FALSE, is_ucast = FALSE, is_override = FALSE;
    int         ori_port = -1, ori_dynamic = TRUE, ori_ucast = FALSE;
    uint8       temp_mac[6];
    uint32      reg_val = 0, temp = 0;
    uint32      temp_hi = 0, temp_lo = 0;
    uint32      st_override_status = 0, src_port = 0 ;
    uint64      temp_mem_data, mac_key;
    uint64      data64_port_mask, temp64;
    int         search_arl_index[ROBO_TBX_L2_BUCKET_SIZE];
    l2_arl_sw_entry_t   search_sw_arl[ROBO_TBX_L2_BUCKET_SIZE];
    
    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_mem_insert : mem=0x%x, flags = 0x%x\n"),
              mem, flags));

    COMPILER_64_ZERO(temp_mem_data);
    COMPILER_64_ZERO(temp64);
    COMPILER_64_MASK_CREATE(data64_port_mask, 
            _TB_ARL_KEY_SEARCH_RESULT_LENGTH_PORT, 
            _TB_ARL_KEY_SEARCH_RESULT_SHIFT_PORT);
    
    switch(mem) {
    case DRV_MEM_ARL:
    case DRV_MEM_MARL:
        break;
    default:
        return SOC_E_PARAM;
    }
    
    /* valid check */
    assert(entry);
    if (!(flags & (DRV_MEM_OP_BY_HASH_BY_MAC | 
            DRV_MEM_OP_BY_HASH_BY_VLANID))){
        return SOC_E_PARAM;
    }
    rv = soc_L2_ARL_SWm_field_get(unit, entry, VAf, &temp);
    if (rv < 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s,%d,SOC problem!\n"),FUNCTION_NAME(),__LINE__));
        return rv;
    }
    /* only valid/pending entry can be inserted. (reject invalid entry) */
    assert(temp == _TB_ARL_STATUS_VALID || temp == _TB_ARL_STATUS_PENDING);

    /* MAC */
    SOC_IF_ERROR_RETURN(soc_L2_ARL_SWm_field_get(unit, 
            entry, MACADDRf, (void *)&temp_mem_data));
    sal_memcpy(&mac_key, &temp_mem_data, sizeof(uint64));
    SAL_MAC_ADDR_FROM_UINT64(temp_mac, temp_mem_data);
    is_ucast = !((*(uint8 *)&temp_mac) & 0x1);  /* check mac_addr bit0 */

    /* get the source port */
    if (is_ucast){
        SOC_IF_ERROR_RETURN(soc_L2_ARL_SWm_field_get(unit, 
                entry, PORTIDf, &src_port));
    } else {
        src_port = (uint32) -1;
    }

    /* -- existence check -- 
     *  1. use search OP to retrieve all 4 bins' entry data and also the 
     *      existence/full status.
     *  2. TB index_write must be implemented as Read-Modify-Write.
     *  3. Insert OP allowed when :
     *      a. existed entry with the same MAC+VID.
     *          - if REPLACE flag set, override this existed entry.
     *          - if no REPLACE flag set, only pending status entry can be 
     *              overried.
     *      b. No existed entries(4 bins) with the same MAC+VID
     *          - choose invalid entry first
     *          - Than choose pending entry (no matter REPLACE flag is set or
     *              not set)
     *          >> No other process case for there is no other condition out 
     *              of those two condition above.
     *      c. 4 bins entries all valid already(Full condition)
     *          - If REPLACE flag is set 
     *              >> choose non-static entry first 
     *              >> than choose Unicast entry
     *              >> than choose MCast entry(bin4 will be the one if all
     *                  4 entries are static and valid Mcast entry.)
     *  
     *  Note : 
     *      1. Return SOC_E_NONE instead of SOC_E_EXISTS to fit DV test.
     */
    
    MEM_RWCTRL_REG_LOCK(soc);
    /* search MAC+VID first :
     *  - for the LOCK control, the search result assumed will be kept after 
     *      search OP returned.
     */
    sal_memset(search_arl_index, 0, 
            sizeof(int) * ROBO_TBX_L2_BUCKET_SIZE);
    sal_memset(search_sw_arl, 0, 
            sizeof(l2_arl_sw_entry_t) * ROBO_TBX_L2_BUCKET_SIZE);
    rv = _drv_tbx_search_key_op(unit, entry, _TB_ARL_SEARCH_MATCH, 
            (uint32 *)&search_sw_arl, search_arl_index);
    /* for op = _TB_ARL_SEARCH_MATCH, the reported index and entry will be
     * placed in the first item in searched array.
     */
    table_index = search_arl_index[0];
    sal_memcpy(&temp_sw_arl, &search_sw_arl, sizeof(l2_arl_sw_entry_t));
    LOG_INFO(BSL_LS_SOC_ARL,
             (BSL_META_U(unit,
                         "%s,%d,rv=%d, id=%d\n"), 
              FUNCTION_NAME(), __LINE__, rv, table_index));
   
    bin_id = -1;
    if (rv == SOC_E_FULL) {
        if (flags & DRV_MEM_OP_REPLACE){
            /* choose non-static entry first */
            for (i = 0; i < _TB_ARL_BIN_NUMBER; i++){
                rv = MEM_DATA_READ(unit, i * 2, &temp_mem_data);
                if (rv){
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s,%d,SOC driver problem!\n"),
                               FUNCTION_NAME(),__LINE__));
                    goto mem_insert_exit;
                }
                temp = COMPILER_64_BITTEST(temp_mem_data, 
                        _TB_ARL_KEY_SEARCH_RESULT_REGSHIFT_STATIC);
                ori_dynamic = (!temp) ? TRUE : FALSE;

                if (ori_dynamic) {     /* dynamic entry */
                    bin_id = i;
                    is_override = TRUE;

                    /* check original mac is ucast or mcast */
                    temp = COMPILER_64_BITTEST(temp_mem_data, 
                            _TB_ARL_KEY_SEARCH_RESULT_MACADDR_BIT0);
                    ori_ucast = (!temp) ? TRUE : FALSE;

                    /* retrieve the original port for unicast condition */
                    if (ori_ucast){
                        COMPILER_64_ALLONES(temp64);
                        COMPILER_64_AND(temp64, temp_mem_data);
                        COMPILER_64_AND(temp64, data64_port_mask);
                        COMPILER_64_SHR(temp64, 
                                _TB_ARL_KEY_SEARCH_RESULT_SHIFT_PORT);
                        ori_port = (uint32)(COMPILER_64_LO(temp64));
                    }
                    break;
                }
            }

            if (bin_id == -1 && is_override == FALSE) {
                /* means no dynamic entry was found for replace operation */
                rv = SOC_E_FULL;
                goto mem_insert_exit;
                
            }

        } else {
            rv = SOC_E_FULL;
            goto mem_insert_exit;
        }
    } else if (rv == SOC_E_EXISTS){
        /* means MAC+VID is the existed */
        ori_ucast = is_ucast;
        is_override = TRUE;
    
        /* For the legacy support, MODIFY flag is allowed here for insert 
         *  to support the case from L2 Replace BCM APIs.
         */ 
        LOG_INFO(BSL_LS_SOC_ARL,
                 (BSL_META_U(unit,
                             "%s,%d, entry existed already!!!\n"),
                  FUNCTION_NAME(), __LINE__));
        bin_id = table_index & _TB_ARL_INDEX_BIN_ID_MASK;

        rv = soc_L2_ARL_SWm_field_get(unit, (uint32 *)&temp_sw_arl, 
            STATICf, &temp);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }
        ori_dynamic = (!temp) ? TRUE : FALSE;

        rv = soc_L2_ARL_SWm_field_get(unit, (uint32 *)&temp_sw_arl, 
            PORTIDf, &temp);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }
        ori_port = temp;
        
        /* 
        *   The EXISTS case (the same MAC+VID is found) in this function is 
        * designed to perform the entry override process. User call bcm API at 
        * bcm_l2_addr_add() to update some entry fields (for TB, those fields 
        * are port, vport, static, valid/pending and hit)
        *
        *   The only early return condition is this inserting entry is completely 
        * the same with this existed entry.
        */
        if (!sal_memcmp(&temp_sw_arl, entry, sizeof(l2_arl_sw_entry_t)) ){
            bin_id = -1;
            rv = SOC_E_NONE;
            goto mem_insert_exit;
        }

    } else if (rv == SOC_E_NOT_FOUND) {
        /* after search_key_op(), a 16K basis entry_id will returned to 
         *  indicate which bin_id is available for insert if the rv=NOT_FOUND.
         *  (Use the dual hashed algorithm with "Least Full Selection")
         */
        bin_id = table_index & _TB_ARL_INDEX_BIN_ID_MASK;
        assert(bin_id != -1);
                
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s,%d,ARL search problem!\n"), 
                   FUNCTION_NAME(),__LINE__));
        goto mem_insert_exit;
    }

    /* -- insert the arl entry into the chosen bin_id entry -- 
     *  1. assert MEM_CTRL(for table_id), MEM_KEY0(for MAC) and MEM_KEY1(for 
     *      VID) all the same with our insertion MAC+VID.
     *  2. Modify the insertion arl_sw_entry into the target bin entry.
     *  3. request the OP start and wait the OP DONE.
     */
    if (bin_id == -1){
        rv = SOC_E_INTERNAL;
        goto mem_insert_exit;
    } else {
        rv = REG_READ_MEM_INDEXr(unit, &reg_val);
        soc_MEM_INDEXr_field_get(unit, &reg_val, INDEXf, &temp);
        assert(temp == ARL_TABLE);
        
        if (rv < 0) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s,%d,SOC driver problem!\n"),
                       FUNCTION_NAME(),__LINE__));
            goto mem_insert_exit;
        }
        
        sal_memset(&temp_sw_arl, 0, sizeof(l2_arl_sw_entry_t));
        /* valid status (valid or pending) :
         *  - to keep the uper layer API with chip independent design, the 
         *      pending/valid entry status value is reasigned here through
         *      flags definition. (So far, all ROBO device before TB have no 
         *      such feature.
         */
        temp = (flags & DRV_MEM_OP_PENDING) ? 
                _TB_ARL_STATUS_PENDING : _TB_ARL_STATUS_VALID;
        rv = soc_L2_ARL_SWm_field_set(unit, (uint32 *)&temp_sw_arl, 
            VAf, &temp);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }                

        /* MAC */
        rv = soc_L2_ARL_SWm_field_get(unit, entry, 
            MACADDRf, (void *)&temp_mem_data);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }    
        rv = soc_L2_ARL_SWm_field_set(unit, (void *)&temp_sw_arl, 
            MACADDRf, (void *)&temp_mem_data);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }                

        
        /* VID */
        rv = soc_L2_ARL_SWm_field_get(unit, entry, 
            VIDf, &temp);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }                
        rv = soc_L2_ARL_SWm_field_set(unit, (uint32 *)&temp_sw_arl, 
            VIDf, &temp);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }                

        /* age :
         *  - to keep the uper layer API with chip independent design, the 
         *      arl age value(for TB's age has 3 bits length is reasigned here 
         *      through flags definition.  (So far, all ROBO device before TB 
         *      have one bit for age filed only).
         *  - retreive the user assigned age value first.
         *  - Age field for new insert L2 entry use b011 if no HIT flag 
         *      requested by user. This value for a dynamic l2 entry will be 
         *      age-out within 3/4 ~ 1 aging time. 
         */
        rv = soc_L2_ARL_SWm_field_get(unit, entry, AGEf, &temp);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }                
        temp = (temp == _TB_ARL_AGE_HIT_VAL) ? temp : _TB_ARL_AGE_DEF_VAL;
        rv = soc_L2_ARL_SWm_field_set(unit, (uint32 *)&temp_sw_arl, 
                AGEf, &temp);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }                
        
        /* static : 
         *  - the original design to insert an arl entry on all ROBO chip 
         *      through BCM API is assinged in the the entry field.
         */
        rv = soc_L2_ARL_SWm_field_get(unit, entry, STATICf, &temp);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }        
        is_dynamic = (!temp) ? TRUE : FALSE;
        rv = soc_L2_ARL_SWm_field_set(unit, (uint32 *)&temp_sw_arl, 
            STATICf, &temp);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }                
        
        /* port_id+vp_id/MGID :
         *  - get MGID and set MGID for both Mcast and Ucast entry.
         *      (Ucast entry with MGID get/set can cover the port_id and vp_id
         *      as well)
         */
        rv = soc_L2_MARL_SWm_field_get(unit, entry, MGIDf, &temp);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }     

        rv = soc_L2_MARL_SWm_field_set(unit, (uint32 *)&temp_sw_arl, 
            MGIDf, &temp);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }   

        if (soc_feature(unit, soc_feature_arl_mode_control)) {
            rv = soc_L2_ARL_SWm_field_get(unit, entry, 
                CONf, &temp);
            if (SOC_FAILURE(rv)){
                goto mem_insert_exit;
            }                

            rv = soc_L2_ARL_SWm_field_set(unit, (uint32 *)&temp_sw_arl, 
                CONf, &temp);
            if (SOC_FAILURE(rv)){
                goto mem_insert_exit;
            }                
            /* static :
             *  - ROBO chip arl_control_mode at none-zero value can't work 
             *    without static setting.
             */
            if (0 != temp){
                temp = 1;
                rv = soc_L2_ARL_SWm_field_set(unit, (uint32 *)&temp_sw_arl, 
                            STATICf, &temp);
            }
        }

        LOG_INFO(BSL_LS_SOC_ARL,
                 (BSL_META_U(unit,
                             "%s,%d,sw_arl[2-0]=%08x-%08x-%08x\n"),
                  FUNCTION_NAME(), __LINE__, *(((uint32 *)&temp_sw_arl)+2),
                  *(((uint32 *)&temp_sw_arl)+1), *(((uint32 *)&temp_sw_arl)+0)));
         
        /* retrieve the arl_hw_entry from arl_sw_entry */
        rv = _drv_tb_arl_entry_sw2hw(unit, &temp_sw_arl, &temp_hw_arl);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }   
        LOG_INFO(BSL_LS_SOC_ARL,
                 (BSL_META_U(unit,
                             "%s,%d,HW_ARL[2-0]=%08x-%08x-%08x\n"),
                  FUNCTION_NAME(), __LINE__, *(((uint32 *)&temp_hw_arl)+2),
                  *(((uint32 *)&temp_hw_arl)+1), *(((uint32 *)&temp_hw_arl)+0)));

        /* copy to target data entry */
        temp_hi = *(uint32 *)&temp_hw_arl;
        temp_lo = *((uint32 *)&temp_hw_arl + 1);
        COMPILER_64_SET(temp_mem_data, temp_lo, temp_hi);
        
        rv = MEM_DATA_WRITE(unit, bin_id * 2, &temp_mem_data);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }   

        temp_hi = *((uint32 *)&temp_hw_arl + 2);
        temp_lo = 0;
        COMPILER_64_SET(temp_mem_data, temp_lo, temp_hi);
        rv = MEM_DATA_WRITE(unit, bin_id * 2 + 1, &temp_mem_data);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }           
        /* mac_key and vid_key is assumed been filled properly already 
         *  >> yes, confirmed already!!
         */        

        /* index_write OP start */
        reg_val = 0;
        temp = MEM_OP_IDX_WRITE;
        soc_MEM_CTRLr_field_set(unit, &reg_val, OP_CMDf, &temp);
        temp = 1;
        soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_STDNf, &temp);
        rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }   
        
        /* wait for complete */
        for (i = 0; i < SOC_TIMEOUT_VAL; i++) {
            rv = REG_READ_MEM_CTRLr(unit, &reg_val);
            if (rv < 0) {
                goto mem_insert_exit;
            }
            soc_MEM_CTRLr_field_get(unit, &reg_val, MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (i >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_insert_exit;
        }
        
        /* SA Learning Count handler :
         *  - increase one for the ARL insertion process
         *      (for Dynamic and Unicast entry only)
         *
         *  SW SA learn count handling process :
         *  ------------------------------------------
         *  If the L2 insert entry is new created on a empty entry space.
         *      1. Dynamic + Unicast 
         *          >> INCREASE one in SA_LRN_CNT
         *  else ( L2 entry insert to a existed entry)
         *      1. No port movement
         *          a. If both of original and this entries are uicast
         *              1). ST changed from Dynamic to Static
         *                  >> DECREASE one in SA_LRN_CNT
         *              2). ST changed from Static to Dynamic
         *                  >> INCREASE one in SA_LRN_CNT
         *          b. else if original entry is MCast and this entry is unicast
         *              1). This entry is Dynamic
         *                  >> INCREASE one in SA_LRN_CNT
         *          c. else if original entry is Unicast and this entry is MCast
         *              1). Original entry is Dynamic
         *                  >> DECREASE one in original port's SA_LRN_CNT
         *      2. Port movement
         *          a. Original entry is dynamic and unicast
         *              >> DECREASE one in original port's SA_LRN_CNT
         *          b. This entry is dynamic and unicast
         *              >> INCREASE entry in SA_LRN_CNT
         *
         */
        temp = 0;
        if (is_override == FALSE){
            if (is_dynamic && is_ucast){
                temp = DRV_PORT_SA_LRN_CNT_INCREASE;
            }
        } else {
            if (ori_dynamic == is_dynamic) {
                st_override_status = _DRV_ST_OVERRIDE_NO_CHANGE;
            } else if (ori_dynamic){
                st_override_status = _DRV_ST_OVERRIDE_DYN2ST;
            } else {
                st_override_status = _DRV_ST_OVERRIDE_ST2DYN;
            }
        
            /* L2 entry override condition */
            if (ori_port == src_port){
                if ((is_ucast == TRUE) && (ori_ucast == TRUE)){
                    if (st_override_status == _DRV_ST_OVERRIDE_ST2DYN){
                        temp = DRV_PORT_SA_LRN_CNT_INCREASE;
                    } else if (st_override_status == _DRV_ST_OVERRIDE_DYN2ST){
                        temp = DRV_PORT_SA_LRN_CNT_DECREASE;
                    }
                } else if ((is_ucast == TRUE) && (ori_ucast == FALSE)){
                    if (is_dynamic){
                        temp = DRV_PORT_SA_LRN_CNT_INCREASE;
                    }
                } else if ((is_ucast == FALSE) && (ori_ucast == TRUE)){
                    if (ori_dynamic == TRUE){
                        temp = DRV_PORT_SA_LRN_CNT_DECREASE;
                    }
                } else {
                    /* both not ucast >> No SA learn count handling action!! */
                }
            } else {
                /* station removment(port changed)! */
                if (is_ucast && is_dynamic){
                    temp = DRV_PORT_SA_LRN_CNT_INCREASE;
                }
        
                if (ori_dynamic && ori_ucast){
                    /* decrease original port's SA learn count  */
                    rv = DRV_ARL_LEARN_COUNT_SET(unit, 
                            ori_port, DRV_PORT_SA_LRN_CNT_DECREASE, 0);
                    if (SOC_FAILURE(rv)){
                        goto mem_insert_exit;
                    }   
                    LOG_INFO(BSL_LS_SOC_ARL,
                             (BSL_META_U(unit,
                                         "%s,port%d,SA_LRN_CNT decrease one!\n"),
                              FUNCTION_NAME(), ori_port));
                }
            }
        }
        /* performing SW SA learn count handling process on THIS PORT */
        if (temp){
            rv = DRV_ARL_LEARN_COUNT_SET(unit, src_port, temp, 0);
            if (SOC_FAILURE(rv)){
                goto mem_insert_exit;
            }   
            LOG_INFO(BSL_LS_SOC_ARL,
                     (BSL_META_U(unit,
                                 "%s,port%d,SA_LRN_CNT %s one!\n"),
                      FUNCTION_NAME(), src_port, 
                      (temp == DRV_PORT_SA_LRN_CNT_INCREASE) ? 
                      "increase" : "decrease"));
        }

        sw_arl_update = 1;
    }
    
mem_insert_exit :
    MEM_RWCTRL_REG_UNLOCK(soc);    

    if (sw_arl_update){
        /* Add the entry to sw database :
         *  - sync the SW ARL for HW delete request and the insert callback 
         *      will be executed in _drv_arl_database_insert().
         *
         *  Important :
         *  1. the 2nd parameter is index. For all ROBO devices before TB 
         *      can't retrieve the read ARL table index. The original design
         *      past this index as bin id to tell this routine that the real
         *      bin id is the target entry to insert.
         *  2. New Design for TB is past the 2nd parameter as the full table 
         *      index(16K basis entry_id)
         */
        _drv_arl_database_insert(unit, table_index, &temp_sw_arl);
    }

    return rv;
}


/*
 *  Function : drv_tbx_mem_move
 *
 *  Purpose :
 *      Move amounts of mem entries  from src_index to dst_indx
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   the memory type to access.
 *      src_index    :  the entry's src index of the memory.
 *      dest_index    :  the entry's dest index of the memory.
 *      count   :   one or more netries will be moved.
 *      flag   :  flag =1 means: move all the related tables
 *                  flag =0 means: move  the specific mem only
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      CFP TCAM
 *      IVM/EVM TCAM
 */
 int
drv_tbx_mem_move(int unit, uint32 mem,int src_index, 
    int dest_index, int count, int flag)
{
    int rv = SOC_E_NONE;
    uint32  temp, reg_val;
    uint32 retry;
    int mem_id, table_index, index_min, index_max;
    soc_control_t           *soc = SOC_CONTROL(unit);
    int i;
    uint64 reg64, temp_data;
    uint32 val32_hi, val32_lo;
    char *s;
    
    /* support check, not all mem table can do "move" operation*/
    switch(mem){
        case DRV_MEM_CFP_DATA_MASK:
        case DRV_MEM_IVM_KEY_DATA_MASK:
        case DRV_MEM_EVM_KEY_DATA_MASK:            
        case DRV_MEM_CFP_ACT:
        case DRV_MEM_CFP_METER:
        case DRV_MEM_CFP_STAT:
        case DRV_MEM_IVM_ACT:
        case DRV_MEM_EVM_ACT:
            break;
        case DRV_MEM_MCAST:
            if (flag) {
                return SOC_E_UNAVAIL;
            }
            break;
        default:
            rv = SOC_E_UNAVAIL;
            return rv;
    }
    mem_id = -1;
    table_index = -1;
    for (i = 0; i < COUNTOF(drv_TB_mem_info); i++) {
        if (drv_TB_mem_info[i].drv_mem == mem){
            mem_id = drv_TB_mem_info[i].soc_mem;
            table_index = drv_TB_mem_info[i].table_index;
            break;
        }    
    }
    if (!SOC_IS_TB_AX(unit)){
        for (i = 0; i < COUNTOF(drv_TB_cfp_mem_info_B0); i++) {
            if (drv_TB_cfp_mem_info_B0[i].drv_mem == mem){
                mem_id = drv_TB_cfp_mem_info_B0[i].soc_mem;
            table_index = drv_TB_cfp_mem_info_B0[i].table_index;
                break;
            }    
        }
    }
#ifdef BCM_VO_SUPPORT
    if (SOC_IS_VO(unit)) {
        for (i = 0; i < COUNTOF(drv_VO_cfp_mem_info); i++) {
            if (drv_VO_cfp_mem_info[i].drv_mem == mem){
                mem_id = drv_VO_cfp_mem_info[i].soc_mem;
                table_index = drv_VO_cfp_mem_info[i].table_index;
                break;
            }    
        }    
    }        
#endif
    if ((mem_id == -1) ||(table_index == -1)){
        return SOC_E_UNAVAIL;
    }

    s = soc_property_get_str(unit, "board_name");
    if( (s != NULL) && (sal_strcmp(s, "bcm53280_fpga") == 0)) {
        if ((TABLE(table_index) == CFP_DATA_TABLE)||
            (table_index == CFP_ACTION_TABLE)||
            (table_index == CFP_METER_TABLE)||
            (table_index == CFP_STAT_TABLE)){
            src_index = (src_index/2) *256 + src_index%2;
            dest_index = (dest_index/2) *256 + dest_index%2;
        }
    }
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);

    if ((src_index < index_min) ||(dest_index < index_min) ||
            (src_index > index_max) ||(dest_index > index_max)){
        return SOC_E_PARAM;
    }

    MEM_RWCTRL_REG_LOCK(soc);
    if (TABLE_INDICATION(table_index)) {
        VO_ARL_SEARCH_LOCK(unit,soc);
    }  

    reg_val = 0;
    temp = TABLE(table_index);
    soc_MEM_INDEXr_field_set(unit, &reg_val, INDEXf, &temp);       
    rv = REG_WRITE_MEM_INDEXr(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto mem_move_exit;
    }
    /* set group control */
    reg_val = 0;
    temp = flag;
    soc_GROUP_CTRLr_field_set(unit, &reg_val, ALL_INDf, &temp);
    rv = REG_WRITE_GROUP_CTRLr(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto mem_move_exit;
    }
    /* Set src & dest index. */
    reg_val = 0;
    temp = src_index;
    soc_MEM_ADDR_0r_field_set(unit, &reg_val, MEM_ADDR_OFFSETf, &temp);
    rv = REG_WRITE_MEM_ADDR_0r(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto mem_move_exit;
    }
    reg_val = 0;    
    temp = dest_index;
    soc_MEM_ADDR_1r_field_set(unit, &reg_val, MEM_ADDR_OFFSETf, &temp);
    rv = REG_WRITE_MEM_ADDR_1r(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto mem_move_exit;
    }
    /* how many entries to move*/
    COMPILER_64_ZERO(temp_data);
    COMPILER_64_ZERO(reg64);
    val32_hi = 0;
    val32_lo = count;
    COMPILER_64_SET(temp_data, val32_hi, val32_lo);
    soc_MEM_KEY_0r_field_set(unit, (void *)&reg64, MEM_KEYf, (void *)&temp_data);
    rv = REG_WRITE_MEM_KEY_0r(unit, &reg64);

    if (SOC_FAILURE(rv)) {
        goto mem_move_exit;
    }
    /* set the MOVE & START op */
    reg_val = 0;    
    temp = MEM_OP_MOVE;
    soc_MEM_CTRLr_field_set(unit, &reg_val, OP_CMDf, &temp);
    temp = 1;
    soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_STDNf, &temp);
    rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto mem_move_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        rv = REG_READ_MEM_CTRLr(unit, &reg_val);
        if (SOC_FAILURE(rv)) {
            goto mem_move_exit;
        }
        soc_MEM_CTRLr_field_get(unit, &reg_val, MEM_STDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        if (SOC_FAILURE(rv)) {
            goto mem_move_exit;
        }
    }

    if (TABLE_INDICATION(table_index)) {
        reg_val = 0;
        rv = REG_WRITE_MEM_INDEXr(unit, &reg_val);
        if (SOC_FAILURE(rv)) {
            goto mem_move_exit;
        }
    }

mem_move_exit :
    if (TABLE_INDICATION(table_index)) {
        VO_ARL_SEARCH_UNLOCK(unit,soc);
    } 

    MEM_RWCTRL_REG_UNLOCK(soc);    
    return rv;
}

/*
 *  Function : drv_tbx_mem_fill
 *
 *  Purpose :
 *      Fill an internal memory.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   the memory type to access.
 *      entry_id    :  the entry's index of the memory to be written.
 *      count   :   one or more netries to be written.
 *      entry_data   :   pointer to a buffer of 32-bit words 
 *                              to contain the writting result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
drv_tbx_mem_fill(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int retry, index_min, index_max;
    uint32 i;
    int mem_id, table_index;
    uint32 reg_val = 0;
    uint32  temp;
    soc_control_t           *soc = SOC_CONTROL(unit);
    int mem_len;
    uint64 temp_data, reg64;
    int j;
    uint32 val32_hi, val32_lo;
    char *s;

    mem_id = -1;
    table_index = -1;
    for (i = 0; i < COUNTOF(drv_TB_mem_info); i++) {
        if (drv_TB_mem_info[i].drv_mem == mem){
            mem_id = drv_TB_mem_info[i].soc_mem;
            table_index = drv_TB_mem_info[i].table_index;
            break;
        }    
    }
    if (!SOC_IS_TB_AX(unit)){
        for (i = 0; i < COUNTOF(drv_TB_cfp_mem_info_B0); i++) {
            if (drv_TB_cfp_mem_info_B0[i].drv_mem == mem){
                mem_id = drv_TB_cfp_mem_info_B0[i].soc_mem;
                table_index = drv_TB_cfp_mem_info_B0[i].table_index;
                break;
            }    
        }
    }
#ifdef BCM_VO_SUPPORT
    if (SOC_IS_VO(unit)) {
        for (i = 0; i < COUNTOF(drv_VO_cfp_mem_info); i++) {
            if (drv_VO_cfp_mem_info[i].drv_mem == mem){
                mem_id = drv_VO_cfp_mem_info[i].soc_mem;
                table_index = drv_VO_cfp_mem_info[i].table_index;
                break;
            }    
        }    
    }        
#endif
    if ((mem_id == -1) || (table_index == -1)){
        return SOC_E_UNAVAIL;
    }
    s = soc_property_get_str(unit, "board_name");
    if( (s != NULL) && (sal_strcmp(s, "bcm53280_fpga") == 0)) {
        if ((TABLE(table_index) == CFP_DATA_TABLE)||
            (table_index == CFP_ACTION_TABLE)||
            (table_index == CFP_METER_TABLE)||
            (table_index == CFP_STAT_TABLE)){
            entry_id = (entry_id/2) *256 + entry_id%2;
        }
    }
#ifdef BCM_VO_SUPPORT
    if (TABLE_INDICATION(table_index)) {
        if (CFP_INDICATION_DATA_MASK_TABLE != table_index){
            return SOC_E_UNAVAIL;
        } else {
            return _vo_cfp_mem_fill(unit, mem, entry_id, count, entry);
        }
    }
#endif  /* BCM_VO_SUPPORT */

    /* add code here to check addr */
    mem_len = soc_mem_entry_words(unit, mem_id);
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);

    /* check the valid entry_id and the requested count */    
    if (((entry_id) < index_min) || 
            ((entry_id + count - 1) > index_max)) {
        if ((count < 1) || (((entry_id + count - 1) > index_max) && 
                ((entry_id) < index_max))){
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,mem_id=0x%x,entry_id=0x%x, invlaid count=%d\n"),
                      FUNCTION_NAME(), mem_id, entry_id, count));
        } else {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,mem_id=0x%x, invalid entry_id=0x%x\n"),
                      FUNCTION_NAME(), mem_id, entry_id));
        }
        return SOC_E_PARAM;
    }

    MEM_RWCTRL_REG_LOCK(soc);
    /* Decide which table. */
    reg_val = 0;
    temp = TABLE(table_index);
    soc_MEM_INDEXr_field_set(unit, &reg_val, INDEXf, &temp);
    rv = REG_WRITE_MEM_INDEXr(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto mem_fill_exit;
    }
    /* Set memory index. */
    reg_val = 0;
    temp = entry_id;
    soc_MEM_ADDR_0r_field_set(unit, &reg_val, MEM_ADDR_OFFSETf, &temp);
    rv = REG_WRITE_MEM_ADDR_0r(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto mem_fill_exit;
    }
    /* how many entries to fill*/
    COMPILER_64_ZERO(temp_data);
    COMPILER_64_ZERO(reg64);
    val32_hi = 0;
    val32_lo = count;
    COMPILER_64_SET(temp_data, val32_hi, val32_lo);
    soc_MEM_KEY_0r_field_set(unit, (void *)&reg64, MEM_KEYf, (void *)&temp_data);
    rv = REG_WRITE_MEM_KEY_0r(unit, &reg64);
    if (SOC_FAILURE(rv)) {
        goto mem_fill_exit;
    }
    val32_hi = 0;
    val32_lo = 0;
    /* Fill the patterns */
    for (j = 0; j <  (mem_len + 1)/2; j++) {
        val32_lo = *entry;
        if(mem_len > (2*j+1) ){
            entry ++;
            val32_hi = *entry;
        } else {
            val32_hi = 0;
        }

        COMPILER_64_SET(temp_data, val32_hi, val32_lo);
        entry++;            
        rv = MEM_DATA_WRITE(unit, j, &temp_data);
        if (SOC_FAILURE(rv)) {
            goto mem_fill_exit;
        }
    }
    /* Write memory control register */
    reg_val = 0;
    temp = MEM_OP_FILL;
    soc_MEM_CTRLr_field_set(unit, &reg_val, OP_CMDf, &temp);
    temp = 1;
    soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_STDNf, &temp);
    rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto mem_fill_exit;
    }
    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        rv = REG_READ_MEM_CTRLr(unit, &reg_val);
        if (SOC_FAILURE(rv)) {
            goto mem_fill_exit;
        }        
        soc_MEM_CTRLr_field_get(unit, &reg_val, MEM_STDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        if (SOC_FAILURE(rv)) {
            goto mem_fill_exit;
        }
    }

mem_fill_exit :
    MEM_RWCTRL_REG_UNLOCK(soc);
    return rv;
}


/* SA_LRN_CNT is a TB specific table, thus this funtion is designed to be 
 *  an internal routine insead of designing to be a formal driver service 
 *  interface.
 */
int 
_drv_tbx_mem_sa_lrncnt_control_set(int unit, uint32 entry_id, 
        uint32 control_type, int  value)
{
    int     rv = SOC_E_NONE, retry = 0;
    int     table_index = 0;
    uint32  reg_val = 0, port_id, mem_op, temp;
    uint32  val32_hi,val32_lo;
    uint64  temp64_data;
    soc_control_t       *soc = SOC_CONTROL(unit);
    
    if (entry_id > _TB_SA_LRNCNT_MAX_TABLE_ID){
        return SOC_E_PARAM;
    }

    if (control_type == DRV_PORT_SA_LRN_CNT_LIMIT){
        mem_op = MEM_OP_WRITE_LIMIT;
    } else if (control_type == DRV_PORT_SA_LRN_CNT_INCREASE){
        mem_op = MEM_OP_INC_ONE;
    } else if (control_type == DRV_PORT_SA_LRN_CNT_DECREASE){
        mem_op = MEM_OP_DEC_ONE;
    } else if (control_type == DRV_PORT_SA_LRN_CNT_RESET){
        mem_op = MEM_OP_RESET;
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s, incorrect control type for this internal routine!\n"),
                   FUNCTION_NAME()));
        return SOC_E_PARAM;
    }

    MEM_RWCTRL_REG_LOCK(soc);
    
    /* set table index */
    reg_val = 0;
    table_index = SA_LERAN_CONF_TABLE;
    soc_MEM_INDEXr_field_set(unit, &reg_val, INDEXf, (uint32 *)&table_index);
    rv = REG_WRITE_MEM_INDEXr(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto exit;
    }
    /* set index keys */
    port_id = entry_id;
    reg_val = 0;
    soc_MEM_ADDR_0r_field_set(unit, &reg_val, MEM_ADDR_OFFSETf, &port_id);
    rv = REG_WRITE_MEM_ADDR_0r(unit, &reg_val);
     if (SOC_FAILURE(rv)) {
        goto exit;
    }

    if (control_type == DRV_PORT_SA_LRN_CNT_LIMIT){
        /* set limit value */
        val32_hi = 0;
        val32_lo = value;
        COMPILER_64_SET(temp64_data, val32_hi, val32_lo);
        rv = MEM_DATA_WRITE(unit, 0, &temp64_data);
        if (SOC_FAILURE(rv)) {
            goto exit;
        }
    } 
    
    /* set mem op_code */
    reg_val = 0;
    soc_MEM_CTRLr_field_set(unit, &reg_val, OP_CMDf, &mem_op);
    temp = 1;
    soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_STDNf, &temp);
    rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto exit;
    }
    
    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        rv = REG_READ_MEM_CTRLr(unit, &reg_val);
        if (SOC_FAILURE(rv)) {
            goto exit;
        }
        soc_MEM_CTRLr_field_get(unit, &reg_val, MEM_STDNf, &temp);
        if (!temp) {
            break;
        }
    }
    
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
    }

exit:
    MEM_RWCTRL_REG_UNLOCK(soc);
    return rv;
}

int
_tb_mem_modify(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 entry_mask, uint32 *entry)
{
    int rv = SOC_E_NONE;
    uint32 retry, index_min, index_max;
    uint32 i;
    int mem_id, table_index;
    uint32 reg_val = 0;
    uint32  temp;
    int entry_size;
    soc_control_t           *soc = SOC_CONTROL(unit);
    int mem_len;
    uint64 temp_data;
    int j;
    uint32 *cache;
    uint8 *vmap;
    uint32 *gmem_entry;
    uint32 val32_hi,val32_lo, hi, lo;
    char *s;

    
    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "_tb_mem_modify(mem=0x%x,entry_id=0x%x,count=%d)\n"),
              mem, entry_id, count));

    mem_id = -1;
    table_index = -1;
    COMPILER_64_ZERO(temp_data);
    
    if(mem < DRV_MEM_ARL){
        for (i = 0; i < COUNTOF(drv_TB_mem_info); i++) {
            if ((drv_TB_mem_info[i].drv_mem == mem) 
                || (drv_TB_mem_info[i].soc_mem == mem)) {
                mem_id = drv_TB_mem_info[i].soc_mem;
                table_index = drv_TB_mem_info[i].table_index;
                break;
            }
        }
        if (!SOC_IS_TB_AX(unit)){
            for (i = 0; i < COUNTOF(drv_TB_cfp_mem_info_B0); i++) {
                if ((drv_TB_cfp_mem_info_B0[i].drv_mem == mem) 
                    || (drv_TB_cfp_mem_info_B0[i].soc_mem == mem)) {
                    mem_id = drv_TB_cfp_mem_info_B0[i].soc_mem;
                    table_index = drv_TB_cfp_mem_info_B0[i].table_index;
                    break;
                }    
            }
        }

        if (mem_id == -1){
            return SOC_E_UNAVAIL;
        }
    }
    else {
        for (i = 0; i < COUNTOF(drv_TB_mem_info); i++) {
            if (drv_TB_mem_info[i].drv_mem == mem){
                mem_id = drv_TB_mem_info[i].soc_mem;
                table_index = drv_TB_mem_info[i].table_index;
                break;
            }    
        }
        if (!SOC_IS_TB_AX(unit)){
            for (i = 0; i < COUNTOF(drv_TB_cfp_mem_info_B0); i++) {
                if (drv_TB_cfp_mem_info_B0[i].drv_mem == mem){
                    mem_id = drv_TB_cfp_mem_info_B0[i].soc_mem;
                    table_index = drv_TB_cfp_mem_info_B0[i].table_index;
                    break;
                }    
            }
        }        
        if (mem_id == -1){
            return SOC_E_UNAVAIL;
        }
    }

    s = soc_property_get_str(unit, "board_name");
    if( (s != NULL) && (sal_strcmp(s, "bcm53280_fpga") == 0)) {
        if ((TABLE(table_index) == CFP_DATA_TABLE)||
            (table_index == CFP_ACTION_TABLE)||
            (table_index == CFP_METER_TABLE)||
            (table_index == CFP_STAT_TABLE)){
            entry_id = (entry_id/2) *256 + entry_id%2;
            entry_id = entry_id % 1536;
        }
    }
    /* add code here to check addr */
    mem_len = soc_mem_entry_words(unit, mem_id);
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);
    gmem_entry = entry;
    /* check the valid entry_id and the requested count */    
    if (((entry_id) < index_min) || 
            ((entry_id + count - 1) > index_max)) {
        if ((count < 1) || (((entry_id + count - 1) > index_max) && 
                ((entry_id) < index_max))){
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,mem_id=0x%x,entry_id=0x%x, invlaid count=%d\n"),
                      FUNCTION_NAME(), mem_id, entry_id, count));
        } else {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,mem_id=0x%x, invalid entry_id=0x%x\n"),
                      FUNCTION_NAME(), mem_id, entry_id));
        }
        return SOC_E_PARAM;
    }

    /* process write action */
    MEM_RWCTRL_REG_LOCK(soc);
    for (i = 0; i < count; i++) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            return SOC_E_PARAM;
        }

        /* Decide which table to be written. */
        reg_val = 0;
        temp = TABLE(table_index);
        soc_MEM_INDEXr_field_set(unit, &reg_val, INDEXf, &temp);
        rv = REG_WRITE_MEM_INDEXr(unit, &reg_val);
        if (rv < 0) {
            goto mem_mod_exit;
        }

        /* Set memory index. */
        reg_val = 0;
    
        temp = entry_id + i;
    
        soc_MEM_ADDR_0r_field_set(unit, &reg_val, MEM_ADDR_OFFSETf, &temp);
        rv = REG_WRITE_MEM_ADDR_0r(unit, &reg_val);
        if (rv < 0) {
            goto mem_mod_exit;
        }

    
        /* Read memory control register */
        reg_val = 0;
        temp = MEM_OP_READ;
        soc_MEM_CTRLr_field_set(unit, &reg_val, OP_CMDf, &temp);
        temp = 1;
        soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_STDNf, &temp);
        rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
        if (rv < 0) {
            goto mem_mod_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            rv = REG_READ_MEM_CTRLr(unit, &reg_val);
            if (rv < 0) {
                goto mem_mod_exit;
            }
            soc_MEM_CTRLr_field_get(unit, &reg_val, MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_mod_exit;
        }


        /* write data */
        for (j = 0; j <  (mem_len + 1)/2; j++) {
            val32_hi = 0;
            val32_lo = 0;
            hi = 0;
            lo = 0;
            if (entry_mask & (1<<(2*j))){
                lo = 1;
            }
            if (entry_mask & (1<<(2*j+1))){
                hi = 1;
            }    

            if (lo|hi) {
                rv = MEM_DATA_READ(unit, j, &temp_data);
                if (rv < 0) {
                    goto mem_mod_exit;
                }
                if (lo){
                    val32_lo = *entry;
                    entry ++;
                } else {
                    val32_lo =  COMPILER_64_LO(temp_data);
                }

                if (hi){
                    val32_hi = *entry;
                    entry ++;
                } else{
                    val32_hi = COMPILER_64_HI(temp_data);
                }
                COMPILER_64_SET(temp_data, val32_hi, val32_lo);
                rv = MEM_DATA_WRITE(unit, j, &temp_data);
                if (rv < 0) {
                    goto mem_mod_exit;
                }   
            }                
        }

        /* Write memory control register */
        reg_val = 0;
        temp = MEM_OP_WRITE;
        soc_MEM_CTRLr_field_set(unit, &reg_val, OP_CMDf, &temp);
        temp = 1;
        soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_STDNf, &temp);
        rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
        if (rv < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_mod_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            rv = REG_READ_MEM_CTRLr(unit, &reg_val);
            if (rv < 0) {
                goto mem_mod_exit;
            }
            soc_MEM_CTRLr_field_get(unit, &reg_val, MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_mod_exit;
        }
        
        /* Write back to cache if active */
        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL) {
            sal_memcpy(cache + (entry_id + i) * entry_size, gmem_entry, 
                    entry_size);
            CACHE_VMAP_SET(vmap, (entry_id + i));
        }
        
    }

mem_mod_exit:
    MEM_RWCTRL_REG_UNLOCK(soc);
    return rv;
}

/*
 * index : (IN) (valid range 0-63) get 256 address update per index
 * valid_list : (OUT) 64 entries with 4 bins vaid status (256 address)
 */
int
_tb_arl_scan_valid(int unit, int index, SHR_BITDCL *valid_list)
{
    soc_control_t *soc = SOC_CONTROL(unit);    
    int rv, table_index, retry, j, k;
    uint32 reg_val, temp;
    uint64 temp_key;

    table_index = ARL_TABLE;

    MEM_RWCTRL_REG_LOCK(soc);
    soc_MEM_INDEXr_field_set(unit, &reg_val, INDEXf, (uint32 *)&table_index);
    rv = REG_WRITE_MEM_INDEXr(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto arl_scan_valid_exit;
    }
    /* Set memory index. */
    reg_val = 0;
    temp = index;
    soc_MEM_ADDR_0r_field_set(unit, &reg_val, MEM_ADDR_OFFSETf, &temp);
    rv = REG_WRITE_MEM_ADDR_0r(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto arl_scan_valid_exit;
    }

    /* start scan */
    reg_val = 0;
    temp = MEM_OP_SCAN_VALID;
    soc_MEM_CTRLr_field_set(unit, &reg_val, OP_CMDf, &temp);
    temp = 1;
    soc_MEM_CTRLr_field_set(unit, &reg_val, MEM_STDNf, &temp);
    rv = REG_WRITE_MEM_CTRLr(unit, &reg_val);
    if (SOC_FAILURE(rv)) {
        goto arl_scan_valid_exit;
    }    
    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        rv = REG_READ_MEM_CTRLr(unit, &reg_val);
        if (SOC_FAILURE(rv)) {
            goto arl_scan_valid_exit;
        }        
        soc_MEM_CTRLr_field_get(unit, &reg_val, MEM_STDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        if (SOC_FAILURE(rv)) {
            goto arl_scan_valid_exit;
        }
    }
    k = 0;
    for (j = 0; j < 8; j++) {
        rv = MEM_KEY_READ(unit, j, &temp_key);
        if (SOC_FAILURE(rv)) {
            goto arl_scan_valid_exit;
        }
        /* endian handler */
        *(valid_list+(2*k)) =  COMPILER_64_LO(temp_key);
        *(valid_list+(2*k+1)) = COMPILER_64_HI(temp_key);
        k++;

    }

arl_scan_valid_exit:
    MEM_RWCTRL_REG_UNLOCK(soc);
    return rv;
}

