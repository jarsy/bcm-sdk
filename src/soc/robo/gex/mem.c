/*
 * $Id: mem.c,v 1.21 Broadcom SDK $
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
#include <soc/arl.h>
#include <soc/error.h>
#include "robo_gex.h"

#define GEX_LEARNED_COUNT_HANDLER 1
#define DESIGN_CHANGE_FOR_PORT_IVLSVL 1

#define FIX_MEM_ORDER_E(v,m) (((m)->flags & SOC_MEM_FLAG_BE) ? \
    BYTES2WORDS((m)->bytes)-1-(v) : (v))

#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT) || \
    defined (BCM_STARFIGHTER3_SUPPORT)
#define STARFIGHTER_8051_ROM_MEMORY 0 
#define STARFIGHTER_8051_RAM_MEMORY 1
#endif /* BCM_STARFIGHTER_SUPPORT || BB2 || SF3 */
#define GEX_VLAN_MEMORY 2 
#define GEX_ARL_MEMORY  3

#define GEX_MEM_OP_WRITE    0
#define GEX_MEM_OP_READ     1
#define GEX_MEM_OP_CLEAR    2

#if defined(BCM_STARFIGHTER3_SUPPORT)
soc_field_info_t sf3_vlan_1q_fields[] = {
    { FORWARD_MAPf_ROBO, 9, 0, SOCF_LE },
    { FWD_MODEf_ROBO, 1, 21, 0 },
    { MSPT_IDf_ROBO, 3, 18, SOCF_LE },
    { RESERVEDf_ROBO, 4, 22, SOCF_LE|SOCF_RES },
    { UNTAG_MAPf_ROBO, 9, 9, SOCF_LE }
};
soc_field_info_t sf3_egr_vid_rmk_fields[] = {
    { INNER_OPf_ROBO, 2, 12, SOCF_LE },
    { INNER_VIDf_ROBO, 12, 0, SOCF_LE },
    { OUTER_OPf_ROBO, 2, 28, SOCF_LE },
    { OUTER_VIDf_ROBO, 12, 16, SOCF_LE },
    { RESERVED_1Rf_ROBO, 2, 14, SOCF_LE|SOCF_RES },
    { RESERVED_2Rf_ROBO, 2, 30, SOCF_LE|SOCF_RES }
};

static int
_drv_gex_sf3_fieldinfo_get(int unit, uint32 mem_type, uint32 field_id, uint32 *fieldinfo_ptr)
{
    int i;
    int rv = SOC_E_NONE;

    if (mem_type ==  DRV_MEM_VLAN) {
        for(i = 0; i < COUNTOF(sf3_vlan_1q_fields) ; i++) {
            if (sf3_vlan_1q_fields[i].field == field_id) {
                *fieldinfo_ptr = (uint32)&sf3_vlan_1q_fields[i];
                break;
            }
        }
        if ( i == COUNTOF(sf3_vlan_1q_fields)) {
            rv = SOC_E_BADID;
        }
    } else if (mem_type == DRV_MEM_EGRVID_REMARK) {
        for(i = 0; i < COUNTOF(sf3_egr_vid_rmk_fields) ; i++) {
            if (sf3_egr_vid_rmk_fields[i].field == field_id) {
                *fieldinfo_ptr = (uint32)&sf3_egr_vid_rmk_fields[i];
                break;
            }
        }
        if ( i == COUNTOF(sf3_egr_vid_rmk_fields)) {
            rv = SOC_E_BADID;
        }
    }
    return rv;
}
#endif



static int
_drv_gex_search_valid_op(int unit, uint32 *key, uint32 *entry, 
        uint32 *entry_1, uint32 flags)
{
    uint32  control, reg_value;
    uint32  temp = 0, process;
    int     rv = SOC_E_NONE, multicast = 0;
    uint32  count, search_valid = 0, entry_valid;
    uint32  result, result1;
    uint64  entry0, entry1, temp_mac_field;
    uint32  vid_rw = 0;
    uint8   temp_mac_addr[6];


    COMPILER_64_ZERO(temp_mac_field);
    /* Check if Search Valid Procedure is running or not */  
    if (flags & DRV_MEM_OP_SEARCH_DONE) {
        if ((rv = REG_READ_ARLA_SRCH_CTLr(unit, &control)) < 0) {
            return rv;
        }
        soc_ARLA_SRCH_CTLr_field_get(unit, &control, 
            ARLA_SRCH_STDNf, &temp);
        if (temp) {
            rv = SOC_E_BUSY;    
        } else {
            rv = SOC_E_NONE;
        }
        
    } else if (flags & DRV_MEM_OP_SEARCH_VALID_START) {
        /* Start Search Valid procedure */
        if ((rv = REG_READ_ARLA_SRCH_CTLr(unit, &control)) < 0) {
            return rv;
        }
        process = 1;
        soc_ARLA_SRCH_CTLr_field_set(unit, &control, 
            ARLA_SRCH_STDNf, &process);
        if ((rv = REG_WRITE_ARLA_SRCH_CTLr(unit, &control)) < 0) {
            return rv;
        }
        return rv;
    } else if (flags & DRV_MEM_OP_SEARCH_VALID_GET) {
        process = 1;
        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = REG_READ_ARLA_SRCH_CTLr(unit, &control)) < 0) {
                goto mem_search_valid_get;
            }
            soc_ARLA_SRCH_CTLr_field_get(unit, &control, 
                ARLA_SRCH_STDNf, &process);
            soc_ARLA_SRCH_CTLr_field_get(unit, &control, 
                ARLA_SRCH_VLIDf, &search_valid);
            /* ARL search operation was done */
            if (!process){
                rv = SOC_E_NOT_FOUND;
                break;
            }
            if (!search_valid){
                continue;
            } else {
                break;
            }
        }
        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_search_valid_get;
        }
        
        if (search_valid) {
            /* Get index value */
            if ((rv = REG_READ_ARLA_SRCH_ADRr(
                unit, &reg_value)) < 0) {
                goto mem_search_valid_get;
            }
            if (SOC_IS_STARFIGHTER3(unit)) {
                soc_ARLA_SRCH_ADRr_field_get(unit, &reg_value,
                    ARLA_SRCH_ADDRESSf, key);
            } else {
                soc_ARLA_SRCH_ADRr_field_get(unit, &reg_value,
                    ARLA_SRCH_ADRf, key);
            }
            /* Read searched entry 0 MAC/VID register */
            if ((rv = REG_READ_ARLA_SRCH_RSLT_0_MACVIDr(unit, 
                (uint32 *)&entry0)) < 0) {
                goto mem_search_valid_get;
            }
                
            /* Read searched entry 1 MAC/VID register */
            if ((rv = REG_READ_ARLA_SRCH_RSLT_1_MACVIDr(unit, 
                (uint32 *)&entry1)) < 0) {
                goto mem_search_valid_get;
            }
            
            /* Read searched entry 0 result register */
            if (SOC_IS_STARFIGHTER3(unit)) {
                if ((rv = REG_READ_ARLA_SRCH_RSLT_0r(unit, &result)) < 0) {
                    goto mem_search_valid_get;
                }
            } else {
                if ((rv = REG_READ_ARLA_SRCH_RSLTr(unit, &result)) < 0) {
                    goto mem_search_valid_get;
                }
            }
            /* Read searched entry 1 result register */
            if ((rv = REG_READ_ARLA_SRCH_RSLT_1r(unit, &result1)) < 0) {
                goto mem_search_valid_get;
            }
            
            /* Check the valid bit of bin 0 entry */
            entry_valid = 0;
            if (SOC_IS_STARFIGHTER3(unit)) {
                soc_ARLA_SRCH_RSLT_0r_field_get(unit, &result,
                    ARLA_SRCH_RSLT_VLID_0f, &entry_valid);
            } else {
                soc_ARLA_SRCH_RSLTr_field_get(unit, &result,
                    ARLA_SRCH_RSLT_VLIDf, &entry_valid);
            }
            if (entry_valid) {
                if (SOC_IS_STARFIGHTER3(unit)) {
                    soc_ARLA_SRCH_RSLT_0_MACVIDr_field_get(unit,
                        (uint32 *)&entry0, ARLA_SRCH_MACADDR_0f,
                        (uint32 *)&temp_mac_field);
                } else {
                    soc_ARLA_SRCH_RSLT_0_MACVIDr_field_get(unit,
                        (uint32 *)&entry0, ARLA_SRCH_MACADDRf,
                        (uint32 *)&temp_mac_field);
                }
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                if ((rv = DRV_MEM_FIELD_SET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                        entry, (uint32 *)&temp_mac_field)) < 0) {
                    goto mem_search_valid_get;
                }
                if (temp_mac_addr[0] & 0x01) {
                    multicast = 1;
                }
                temp = 1;
                 /* Set VALID field */
                if ((rv = DRV_MEM_FIELD_SET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_VALID, entry, &temp)) < 0) {
                    goto mem_search_valid_get;
                }
                /* Read ARL Search Result VID Register */
                if (SOC_IS_STARFIGHTER3(unit)) {
                    soc_ARLA_SRCH_RSLT_0_MACVIDr_field_get(unit, (uint32 *)&entry0,
                        ARLA_SRCH_RSLT_VID_0f, &vid_rw);
                } else {
                    soc_ARLA_SRCH_RSLT_0_MACVIDr_field_get(unit, (uint32 *)&entry0,
                        ARLA_SRCH_RSLT_VIDf, &vid_rw);
                }
                /* Set VLANID field */
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_VLANID, entry, &vid_rw)) < 0) {
                    goto mem_search_valid_get;
                }
                /* Set MACADDR field */
                if (SOC_IS_STARFIGHTER3(unit)) {
                    soc_ARLA_SRCH_RSLT_0_MACVIDr_field_get(unit, (uint32 *)&entry0,
                        ARLA_SRCH_MACADDR_0f, (uint32 *)&temp_mac_field);
                } else {
                    soc_ARLA_SRCH_RSLT_0_MACVIDr_field_get(unit, (uint32 *)&entry0,
                        ARLA_SRCH_MACADDRf, (uint32 *)&temp_mac_field);
                }
                if (multicast) { /* mcast address */
                    /* Multicast Port Bitmap */
                    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result, 
                            PORTID_Rf, &temp);
                    } else if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_0r_field_get(unit, &result,
                            PORTID_0f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result,
                            PORTIDf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                            DRV_MEM_FIELD_DEST_BITMAP, entry, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }
                    /* STATIC */
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_0r_field_get(unit, &result,
                            ARLA_SRCH_RSLT_STATIC_0f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result,
                            ARLA_SRCH_RSLT_STATICf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                            DRV_MEM_FIELD_STATIC, entry, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }
                    /* Priority */
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_0r_field_get(unit, &result,
                            ARLA_SRCH_RSLT_PRI_0f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result,
                            ARLA_SRCH_RSLT_PRIf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                            DRV_MEM_FIELD_PRIORITY, entry, &temp)) < 0) {
                        goto mem_search_valid_get;
                    } 
                    /* age */
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_0r_field_get(unit, &result,
                            ARLA_SRCH_RSLT_AGE_0f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result,
                            ARLA_SRCH_RSLT_AGEf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                            DRV_MEM_FIELD_AGE, entry, &temp)) < 0) {
                        goto mem_search_valid_get;
                    } 
                    /* ARL control */
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_0r_field_get(unit, &result,
                            ARL_CON_0f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result,
                            ARL_CONf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                            DRV_MEM_FIELD_ARL_CONTROL, entry, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }

                } else { /* unicast address */       
                    /* Source Port Number */
                    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result, 
                            PORTID_Rf, &temp);
                    } else if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_0r_field_get(unit, &result,
                            PORTID_0f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result,
                            PORTIDf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_SRC_PORT, entry, &temp)) < 0) {
                        goto mem_search_valid_get;
                    } 
            
                    /* Priority */
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_0r_field_get(unit, &result,
                            ARLA_SRCH_RSLT_PRI_0f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result,
                            ARLA_SRCH_RSLT_PRIf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_PRIORITY, entry, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }

                    /* Static Bit */
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_0r_field_get(unit, &result,
                            ARLA_SRCH_RSLT_STATIC_0f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result,
                            ARLA_SRCH_RSLT_STATICf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_STATIC, entry, &temp)) < 0) {
                        goto mem_search_valid_get;
                    } 

                    /* Hit bit */
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_0r_field_get(unit, &result,
                            ARLA_SRCH_RSLT_AGE_0f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result,
                            ARLA_SRCH_RSLT_AGEf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_AGE, entry, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }

                    /* ARL control */
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_0r_field_get(unit, &result,
                            ARL_CON_0f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result,
                            ARL_CONf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_ARL_CONTROL, entry, &temp)) < 0) {
                        goto mem_search_valid_get;
                    } 
                        
                }
            }

            /* Check the valid bit of bin 1 entry */
            entry_valid = 0;
            if (SOC_IS_STARFIGHTER3(unit)) {
                soc_ARLA_SRCH_RSLT_1r_field_get(unit, &result1,
                    ARLA_SRCH_RSLT_VLID_1f, &entry_valid);
            } else {
                soc_ARLA_SRCH_RSLTr_field_get(unit, &result1,
                    ARLA_SRCH_RSLT_VLIDf, &entry_valid);
            }
            if (entry_valid) {
                if (SOC_IS_STARFIGHTER3(unit)) {
                    soc_ARLA_SRCH_RSLT_0_MACVIDr_field_get(unit,
                            (uint32 *)&entry1, ARLA_SRCH_MACADDR_0f,
                            (uint32 *)&temp_mac_field);
                } else {
                    soc_ARLA_SRCH_RSLT_0_MACVIDr_field_get(unit,
                            (uint32 *)&entry1, ARLA_SRCH_MACADDRf,
                            (uint32 *)&temp_mac_field);
                }

                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                        entry_1, (uint32 *)&temp_mac_field)) < 0) {
                    goto mem_search_valid_get;
                }
                if (temp_mac_addr[0] & 0x01) {
                    multicast = 1;
                }
                temp = 1;
                /* Set VALID field */
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_VALID, entry_1, &temp)) < 0) {
                    goto mem_search_valid_get;
                }
                /* Read ARL Search Result VID Register */
                if (SOC_IS_STARFIGHTER3(unit)) {
                    soc_ARLA_SRCH_RSLT_0_MACVIDr_field_get(unit, (uint32 *)&entry1,
                            ARLA_SRCH_RSLT_VID_0f, &vid_rw);
                } else {
                    soc_ARLA_SRCH_RSLT_0_MACVIDr_field_get(unit, (uint32 *)&entry1,
                            ARLA_SRCH_RSLT_VIDf, &vid_rw);
                }
                /* Set VLANID field */
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_VLANID, entry_1, &vid_rw)) < 0) {
                    goto mem_search_valid_get;
                }
                /* Set MACADDR field */
                if (SOC_IS_STARFIGHTER3(unit)) {
                    soc_ARLA_SRCH_RSLT_0_MACVIDr_field_get(unit, (uint32 *)&entry1,
                        ARLA_SRCH_MACADDR_0f, (uint32 *)&temp_mac_field);
                } else {
                    soc_ARLA_SRCH_RSLT_0_MACVIDr_field_get(unit, (uint32 *)&entry1,
                         ARLA_SRCH_MACADDRf, (uint32 *)&temp_mac_field);
                }

                if (multicast) { /* mcast address */
                    /* Multicast Port Bitmap */
                    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result1, 
                                PORTID_Rf, &temp); 
                    } else if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_1r_field_get(unit, &result1,
                                PORTID_1f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result1,
                                PORTIDf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                            DRV_MEM_FIELD_DEST_BITMAP, entry_1, &temp)) < 0) {
                        goto mem_search_valid_get;
                    } 
                    /* STATIC */
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_1r_field_get(unit, &result1,
                            ARLA_SRCH_RSLT_STATIC_1f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result1,
                            ARLA_SRCH_RSLT_STATICf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                            DRV_MEM_FIELD_STATIC, entry_1, &temp)) < 0) {
                        goto mem_search_valid_get;
                    } 
                    /* Priority */
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_1r_field_get(unit, &result1,
                            ARLA_SRCH_RSLT_PRI_1f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result1,
                             ARLA_SRCH_RSLT_PRIf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                            DRV_MEM_FIELD_PRIORITY, entry_1, &temp)) < 0) {
                        goto mem_search_valid_get;
                    } 
                    /* age */
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_1r_field_get(unit, &result1,
                            ARLA_SRCH_RSLT_AGE_1f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result1,
                            ARLA_SRCH_RSLT_AGEf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                            DRV_MEM_FIELD_AGE, entry_1, &temp)) < 0) {
                        goto mem_search_valid_get;
                    } 
                    /* ARL control */
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_1r_field_get(unit, &result1,
                            ARL_CON_1f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result1,
                            ARL_CONf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                            DRV_MEM_FIELD_ARL_CONTROL, entry_1, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }

                } else { /* unicast address */       
                    /* Source Port Number */
                    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result1, 
                                PORTID_Rf, &temp);
                    } else if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_1r_field_get(unit, &result1,
                                PORTID_1f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result1,
                                PORTIDf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_SRC_PORT, entry_1, &temp)) < 0) {
                        goto mem_search_valid_get;
                    } 

                    /* Priority */
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_1r_field_get(unit, &result1,
                            ARLA_SRCH_RSLT_PRI_1f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result1,
                            ARLA_SRCH_RSLT_PRIf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_PRIORITY, entry_1, &temp)) < 0) {
                        goto mem_search_valid_get;
                    } 

                    /* ARL control */
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_1r_field_get(unit, &result1,
                            ARL_CON_1f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result1,
                            ARL_CONf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_ARL_CONTROL, entry_1, &temp)) < 0) {
                        goto mem_search_valid_get;
                    } 

                    /* Static Bit */
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_1r_field_get(unit, &result1,
                            ARLA_SRCH_RSLT_STATIC_1f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result1,
                            ARLA_SRCH_RSLT_STATICf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_STATIC, entry_1, &temp)) < 0) {
                        goto mem_search_valid_get;
                    } 

                    /* Hit bit */
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        soc_ARLA_SRCH_RSLT_1r_field_get(unit, &result1,
                            ARLA_SRCH_RSLT_AGE_1f, &temp);
                    } else {
                        soc_ARLA_SRCH_RSLTr_field_get(unit, &result1,
                            ARLA_SRCH_RSLT_AGEf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_AGE, entry_1, &temp)) < 0) {
                        goto mem_search_valid_get;
                    } 
                        
                }
            }
            rv = SOC_E_EXISTS;
            goto mem_search_valid_get;
        }
mem_search_valid_get:
        return rv;
    }

    return rv;
}


static int
_drv_gex_arl_mac_vid_match(int unit, uint32 *mac, uint32 vid, 
        uint32 *entry, uint32 flags)
{
    uint64  entry_mac_field;
    uint32  entry_vid = 0;
    int match = 0;

    soc_ARLA_MACVID_ENTRY0r_field_get(unit, entry, 
        ARL_MACADDRf, (uint32 *)&entry_mac_field);
    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
        soc_ARLA_MACVID_ENTRY0r_field_get(unit, entry, 
            VID_Rf, &entry_vid);
    } else {
        soc_ARLA_MACVID_ENTRY0r_field_get(unit, entry, 
            VIDf, &entry_vid);
    }
    /* check if we have to overwrite this valid bin0 */
    if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
        /* check mac + vid */
        if (!sal_memcmp((uint32 *)&entry_mac_field, mac, 
                    sizeof(entry_mac_field)) && (vid == entry_vid)) {
            match = 1;
       }
    } else {
        /* check mac */
        if (!sal_memcmp((uint32 *)&entry_mac_field, mac, 
                sizeof(entry_mac_field))) {
           match = 1;
        }
    }
    return match;
}

/*
 *  Function : _drv_gex_mem_search
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
 *      index   :   bin_id in this entry bucket (-1 ~ 3).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. DRV_MEM_OP_SEARCH_CONFLICT
 *      a. will be performed in the MAC+VID hash search section.
 *      b. '*entry_1' not be used but 'entry' will be used as l2_entry array
 *          and this l2_entry array size should not smaller than ARL table  
 *          hash bucket size(bin number).
 *      c. all valid entries in the MAC+VID hash bucket will be reported.
 */
static int 
_drv_gex_mem_search(int unit, uint32 mem, uint32 *key, 
                uint32 *entry, uint32 *entry_1, uint32 flags, int *index)
{
    int         rv = SOC_E_NONE;
    soc_control_t   *soc = SOC_CONTROL(unit);
    uint32      count, temp, i;
    uint32      control = 0;
    uint8       mac_addr_rw[6], temp_mac_addr[6];
    uint64      rw_mac_field, temp_mac_field;
    uint64      entry0;
    uint32      vid_rw;
    int         binNum = -1, matched_id = -1, dynamic_index = -1;
    int         valid[ROBO_GEX_L2_BUCKET_SIZE], is_conflict = 0;
    uint32      reg_value;
    gen_memory_entry_t      gmem_entry;
    uint8       hash_value[6];
    uint16      hash_result;
    uint32      result, mcast_pbmp;
    uint32      macvid_reg, result_reg;
    
    if ((flags & DRV_MEM_OP_SEARCH_DONE) ||
        (flags & DRV_MEM_OP_SEARCH_VALID_START) ||
        (flags & DRV_MEM_OP_SEARCH_VALID_GET)){
        rv = _drv_gex_search_valid_op(unit, key, entry, entry_1, flags);
        return rv;
    } else if (flags & DRV_MEM_OP_BY_INDEX) {
        /* The entry index is inside the input parameter "key". */ 
        /* Read the memory raw data */
        SOC_IF_ERROR_RETURN(DRV_MEM_READ
                (unit, mem, *key, 1, (uint32 *)&gmem_entry));
        /* Read VALID bit */
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_VALID, 
                (uint32 *)&gmem_entry, &temp));
        if (!temp) {
            return SOC_E_NONE;
        }
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, entry, &temp));

        /* Read VLAN ID value */
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_VLANID, 
                (uint32 *)&gmem_entry, &vid_rw));
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &vid_rw));
        
        /* Read MAC address bit 10~47 */
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_MAC, 
                (uint32 *)&gmem_entry, (uint32 *)&rw_mac_field));
        SAL_MAC_ADDR_FROM_UINT64(mac_addr_rw, rw_mac_field);
        /* check HASH enabled ? */
        /* check 802.1q enable */
        if ((rv = REG_READ_VLAN_CTRL0r(unit, &reg_value)) < 0) {
            return rv;
        }

        soc_VLAN_CTRL0r_field_get(unit, &reg_value, 
            VLAN_ENf, &temp);
        sal_memset(hash_value, 0, sizeof(hash_value));
        if (temp) {
            soc_VLAN_CTRL0r_field_get(unit, &reg_value, 
                VLAN_LEARN_MODEf, &temp);
            if (temp == 3) {
                /* hash value = VID + MAC */
                hash_value[0] = (vid_rw >> 4) & 0xff;
                hash_value[1] = ((vid_rw & 0xf) << 4) + (mac_addr_rw[1] & 0xf);
                hash_value[2] = mac_addr_rw[2];
                hash_value[3] = mac_addr_rw[3];
                hash_value[4] = mac_addr_rw[4];
                hash_value[5] = mac_addr_rw[5];
            } else if (temp == 0){
                /* hash value = MAC */
                hash_value[0] = ((mac_addr_rw[1] & 0xf) << 4) 
                    + ((mac_addr_rw[2] & 0xf0) >> 4);
                hash_value[1] = ((mac_addr_rw[2] & 0xf) << 4) 
                    + ((mac_addr_rw[3] & 0xf0) >> 4);
                hash_value[2] = ((mac_addr_rw[3] & 0xf) << 4) 
                    + ((mac_addr_rw[4] & 0xf0) >> 4);
                hash_value[3] = ((mac_addr_rw[4] & 0xf) << 4) 
                    + ((mac_addr_rw[5] & 0xf0) >> 4);
                hash_value[4] = ((mac_addr_rw[5] & 0xf) << 4);
            } else {
                return SOC_E_CONFIG;
            }
        } else {
            /* hash value = MAC value */
            hash_value[0] = ((mac_addr_rw[1] & 0xf) << 4) 
                    + ((mac_addr_rw[2] & 0xf0) >> 4);
            hash_value[1] = ((mac_addr_rw[2] & 0xf) << 4) 
                + ((mac_addr_rw[3] & 0xf0) >> 4);
            hash_value[2] = ((mac_addr_rw[3] & 0xf) << 4) 
                + ((mac_addr_rw[4] & 0xf0) >> 4);
            hash_value[3] = ((mac_addr_rw[4] & 0xf) << 4) 
                + ((mac_addr_rw[5] & 0xf0) >> 4);
            hash_value[4] = ((mac_addr_rw[5] & 0xf) << 4);
        }
        /* Get the hash revalue */
        _drv_arl_hash(hash_value, 48, &hash_result);
        /* Recover the MAC bit 0~11 */
        temp = *key;
        temp = temp >> 1; 
        hash_result = (hash_result ^ temp) & 0xfff;
        
        temp_mac_addr[0] = ((mac_addr_rw[1] & 0xf) << 4) 
            + ((mac_addr_rw[2] & 0xf0) >> 4);
        temp_mac_addr[1] = ((mac_addr_rw[2] & 0xf) << 4) 
            + ((mac_addr_rw[3] & 0xf0) >> 4);
        temp_mac_addr[2] = ((mac_addr_rw[3] & 0xf) << 4) 
            + ((mac_addr_rw[4] & 0xf0) >> 4);
        temp_mac_addr[3] = ((mac_addr_rw[4] & 0xf) << 4) 
            + ((mac_addr_rw[5] & 0xf0) >> 4);
        temp_mac_addr[4] = ((mac_addr_rw[5] & 0xf) << 4) 
            + (hash_result >> 8);
        temp_mac_addr[5] = hash_result & 0xff;
        SAL_MAC_ADDR_TO_UINT64(temp_mac_addr, temp_mac_field);
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                entry, (uint32 *)&temp_mac_field));
        if (MAC_IS_MCAST(temp_mac_addr)) { /* multicast entry */
            /* Multicast index */
            mcast_pbmp = 0;
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                    (unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_SRC_PORT, 
                    (uint32 *)&gmem_entry, &temp));
            mcast_pbmp = temp;
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
                    unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_DEST_BITMAP, 
                    (uint32 *)&gmem_entry, &temp));
            mcast_pbmp += (temp << 4);
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
                    unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                    entry, &mcast_pbmp));
        } else { /* unicast entry */
            /* Source port number */
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
                    unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_SRC_PORT, 
                    (uint32 *)&gmem_entry, &temp));
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
                    unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp));
            /* Priority value */
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
                    unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_PRIORITY, 
                    (uint32 *)&gmem_entry, &temp));
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
                    unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, entry, &temp));
            
        }
        /* Age bit */
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
                unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_AGE, 
                (uint32 *)&gmem_entry, &temp));
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, entry, &temp));
        /* ARL_control */
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
                unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_ARL_CONTROL, 
                (uint32 *)&gmem_entry, &temp));
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, entry, &temp));

        /* Static bit */
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
                unit, DRV_MEM_ARL_HW, DRV_MEM_FIELD_STATIC, 
                (uint32 *)&gmem_entry, &temp));
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
                unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, entry, &temp));
        
        return rv;
     
    /* delete by MAC/MAC+VID */
    } else if (flags & DRV_MEM_OP_BY_HASH_BY_MAC) {
        l2_arl_sw_entry_t   *rep_entry;
        int     entry_len = sizeof(l2_arl_sw_entry_t);

        sal_memset(valid, 0, sizeof(valid));
        if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
            for (i = 0; i < ROBO_GEX_L2_BUCKET_SIZE; i++){
                /* check the parameter for output entry buffer */
                rep_entry = (l2_arl_sw_entry_t *)entry + i;
                if (rep_entry == NULL){
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s,entries buffer not allocated!\n"), 
                              FUNCTION_NAME()));
                    return SOC_E_PARAM;
                }

            }
        }
    
        ARL_MEM_SEARCH_LOCK(soc);
#if DESIGN_CHANGE_FOR_PORT_IVLSVL
        /* original deisgn is not proper to change IVL/SVL setting while 
         * performing ARL search operation.
         */

        /* this new design won't check the IVL/SVL setting. API user need give 
         * proper VID for ARL add/delete/etc..
         */ 
            
#else /* DESIGN_CHANGE_FOR_PORT_IVLSVL */
        /* enable 802.1Q and set VID+MAC to hash */
        if ((rv = REG_READ_VLAN_CTRL0r(unit, &reg_value)) < 0) {
            goto mem_search_exit;
        }
        temp = 1;
        soc_VLAN_CTRL0r_field_set(unit, &reg_value, 
            VLAN_ENf, &temp);
        /* check IVL or SVL */
        soc_VLAN_CTRL0r_field_get(unit, &reg_value, 
            VLAN_LEARN_MODEf, &temp);
        if (temp == 3){     /* VLAN is at IVL mode */
            if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                temp = 3;
            } else {
                temp = 0;
            }
            soc_VLAN_CTRL0r_field_set(unit, &reg_value, 
                VLAN_LEARN_MODEf, &temp);
        }
        if ((rv = REG_WRITE_VLAN_CTRL0r(unit, &reg_value)) < 0) {
            goto mem_search_exit;
        }
#endif /* DESIGN_CHANGE_FOR_PORT_IVLSVLc */

        /* Write MAC Address Index Register */
        if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                key, (uint32 *)&rw_mac_field)) < 0) {
            goto mem_search_exit;
        }
        SAL_MAC_ADDR_FROM_UINT64(mac_addr_rw, rw_mac_field);
        if ((rv = REG_WRITE_ARLA_MACr(unit, &rw_mac_field)) < 0) {
            goto mem_search_exit;
        }

        if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
            /* Write VID Table Index Register */
            if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_VLANID, key, &vid_rw))< 0) {
                goto mem_search_exit;
            }
            if ((rv = REG_WRITE_ARLA_VIDr(unit, &vid_rw)) < 0) {
                goto mem_search_exit;
            }
        }

        /* Write ARL Read/Write Control Register */
        /* Read Operation */
        if ((rv = REG_READ_ARLA_RWCTLr(unit, &control)) < 0) {
            goto mem_search_exit;
        }
        temp = MEM_TABLE_READ;
        soc_ARLA_RWCTLr_field_set(unit, &control, 
            ARL_RWf, &temp);
        
        temp = 1;
        soc_ARLA_RWCTLr_field_set(unit, &control, 
            ARL_STRTDNf, &temp);
        
        if ((rv = REG_WRITE_ARLA_RWCTLr(unit, &control)) < 0) {
            goto mem_search_exit;
        }

        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = REG_READ_ARLA_RWCTLr(unit, &control)) < 0) {
                goto mem_search_exit;
            }

            soc_ARLA_RWCTLr_field_get(unit, &control, 
                ARL_STRTDNf, &temp);
            if (!temp)
                break;
        }

        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_search_exit;
        }

        /* vulcan read operation will index to a 4 bins buckets */
        *index = -1;    /* assigning the initial value */
        for (i = 0 ;i < ROBO_GEX_L2_BUCKET_SIZE; i++) {
            switch(i) {
            case 0:
                macvid_reg = INDEX(ARLA_MACVID_ENTRY0r);
                result_reg = INDEX(ARLA_FWD_ENTRY0r);
                break;
            case 1:
                macvid_reg = INDEX(ARLA_MACVID_ENTRY1r);
                result_reg = INDEX(ARLA_FWD_ENTRY1r);
                break;
            case 2:
                macvid_reg = INDEX(ARLA_MACVID_ENTRY2r);
                result_reg = INDEX(ARLA_FWD_ENTRY2r);
                break;
            case 3:
                macvid_reg = INDEX(ARLA_MACVID_ENTRY3r);
                result_reg = INDEX(ARLA_FWD_ENTRY3r);
                break;
            default:
                ARL_MEM_SEARCH_UNLOCK(soc);
                return SOC_E_INTERNAL;
            }
            if ((rv = DRV_REG_READ(unit, DRV_REG_ADDR(unit, result_reg, 0, 0),
                    &result, 4)) < 0) {
                goto mem_search_exit;
            }
            soc_ARLA_FWD_ENTRY0r_field_get(unit, &result,
                ARL_VALIDf, &temp);
            /* Check valid bit */
            if (!temp) {
                if (binNum == -1) {
                    binNum = i;     /* assigning the free bin_id */
                }
                continue;
            }

            /* valid[] updating process :
             *  a. CONFLICT search is asserted :
             *      - valid[] will be updated for each bin
             *  b. CONFLICT search is not asserted :
             *      - valid[] may not be all updated once the match occurred.
             */
            valid[i] = TRUE;
            if (flags & DRV_MEM_OP_SEARCH_CONFLICT){

                /* for conflict search, to know the valid status is enough */
                continue;
            }

            /* Read MAC/VID entry register */
            if ((rv = DRV_REG_READ(unit, DRV_REG_ADDR(unit, macvid_reg, 0, 0),
                    &entry0, 8)) < 0) {
                goto mem_search_exit;
            }
            
            /* Compare the MAC and VID value */
            /* coverity[incompatible_cast] */
            if (_drv_gex_arl_mac_vid_match
                    (unit, (uint32*)&rw_mac_field, 
                    vid_rw, (uint32 *)&entry0, flags)) {
                    
                binNum = i; /* update binNum to report the existed bin_id */
                matched_id = binNum;

                break;
            } else {

                /* DRV_MEM_OP_REPLACE processing :
                 *  a. REPLACE is set and non-static entry found
                 *      - dynamic_index = 0~3
                 *  b. REPLACE is set but no non-static entry
                 *      - dynamic_index = -1
                 *  c. REPLACE is not set 
                 *      - dynamic_index = -1
                 */
                if (flags & DRV_MEM_OP_REPLACE){
                    /* Check static valid */
                    soc_ARLA_FWD_ENTRY0r_field_get(unit, &result,
                        ARL_STATICf, &temp);
                    if (!temp) {
                        dynamic_index = i;
                    }
                }
            }
        }

        /* After the process above, the possible condition will be :
         *  1. FOUND : the MAC+VID matched and valid entry is found.
         *      - binNum = 0~3
         *      - matched_id = binNum
         *  2. FULL : bucket FULL(all bins are valid)
         *      - binNum = -1
         *      - matched_id = -1
         *  3. NOT_FOUND : no valid bin with MAC+VID matched.
         *      - binNum = 0~3
         *      - matched_id = -1
         */
        if (binNum == -1){
            /* Condition here :
             *  - every entry in those 4 bins are valid and no matached
             *      MAC+VID
             */
            
            if ((flags & DRV_MEM_OP_REPLACE) && (dynamic_index != -1)){
                binNum = dynamic_index;
            }
        }

        /* check the searching operation :
         *  1. Normal search : search the entry with MAC+VID been matched
         *    a. SOC_E_EXISTS : found the matched entry
         *      - '*entry' : report this matched entry(binNum)
         *      - '*index' : report the matched entry index(binNum)
         *    b. SOC_E_FULL : all 4 bins are valid but no MAC+VID matched.
         *      - '*entry' : 
         *          1). if REPLACE flag is not set, report all zero entry
         *          2). if REPLACE flag is set, report a found entry which 
         *              is non-static entry in this hash bucket.(binNum)
         *      - '*index' : 
         *          1). if REPLACE flag is not set, report -1
         *          2). if REPLACE flag is set, report a found bin_id(binNum)
         *              of an entry which is non-static entry.
         *              (allow to be replaced)
         *    c. SOC_E_NOT_FOUND : no MAC+VID matched entry and the hash 
         *          bucket is not full(some bin is invalid)
         *      - '*entry' : report all zero entry
         *      - '*index' : report a found bin_id which is invalid.(binNum)
         *  2. Conflict search : search all valid entries in this MAC_VID hash
         *      bucket.
         *      - '*index' : report -1 (for ignor)
         *    a. SOC_E_EXISTS : found the valid entry/entries in this bucket
         *      - '*entry' : report those conflicted entries
         *    b. SOC_E_NOT_FOUND : hash bucket has no valid entry (EMPTY)
         *      - '*entry' : report all zero entry
         */

        /* reporting entry/entries and the index process :
         *  1. for conflict search,
         *      >> report all valid but not matched entries.
         *  2. for normal search,
         *      >> report the valid entry with bin_id=binNum.
         */
        for (i = 0; i < ROBO_GEX_L2_BUCKET_SIZE; i++){
            rep_entry = (l2_arl_sw_entry_t *)entry + i;
            /* init the reporting entry and index buffer */
            memset(rep_entry, 0, entry_len);
            
            if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
                if (valid[i] == 0){
                    /* no report on this bin */
                    continue;
                } else {
                    /* report this entry */
                    is_conflict = TRUE;
                }
            } else {
                if (binNum == -1){
                    /* FULL condition, no reported entry and index */
                    break;
                } else {
                    /* report this entry */
                    *index = binNum;
                    /* force one time report process only */
                    i = binNum;
                }
            }

            /* ------ entries report process ------ */
            if (valid[i] == TRUE){
                switch(i) {
                case 0:
                    macvid_reg = INDEX(ARLA_MACVID_ENTRY0r);
                    result_reg = INDEX(ARLA_FWD_ENTRY0r);
                    break;
                case 1:
                    macvid_reg = INDEX(ARLA_MACVID_ENTRY1r);
                    result_reg = INDEX(ARLA_FWD_ENTRY1r);
                    break;
                case 2:
                    macvid_reg = INDEX(ARLA_MACVID_ENTRY2r);
                    result_reg = INDEX(ARLA_FWD_ENTRY2r);
                    break;
                case 3:
                    macvid_reg = INDEX(ARLA_MACVID_ENTRY3r);
                    result_reg = INDEX(ARLA_FWD_ENTRY3r);
                    break;
                default:
                    ARL_MEM_SEARCH_UNLOCK(soc);
                    return SOC_E_INTERNAL;
                }

                /* Read MAC/VID entry register */
                if ((rv = DRV_REG_READ(unit, 
                        DRV_REG_ADDR(unit, macvid_reg, 0, 0), 
                        (uint32 *)&entry0, 8)) < 0) {
                    goto mem_search_exit;
                }
                COMPILER_64_ZERO(temp_mac_field);
                soc_ARLA_MACVID_ENTRY0r_field_get(unit, (uint32 *)&entry0,
                    ARL_MACADDRf, (uint32 *)&temp_mac_field);

                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                
                /* Only need write the selected Entry Register */
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_MAC, (uint32 *)rep_entry, 
                        (uint32 *)&temp_mac_field)) < 0){
                    goto mem_search_exit;
                }

                /* VID */
                if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                    soc_ARLA_MACVID_ENTRY0r_field_get(unit, (uint32 *)&entry0,
                        VID_Rf, &temp);
                } else {
                    soc_ARLA_MACVID_ENTRY0r_field_get(unit, (uint32 *)&entry0,
                        VIDf, &temp);
                }
                if ((rv = DRV_MEM_FIELD_SET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                        (uint32 *)rep_entry, &temp)) < 0){
                    goto mem_search_exit;
                }

                if ((rv = DRV_REG_READ(unit, 
                        DRV_REG_ADDR(unit, result_reg, 0, 0), 
                        (uint32 *)&result, 4)) < 0) {
                    goto mem_search_exit;
                }

                if (MAC_IS_MCAST(temp_mac_addr)) { /* is mcast address */
                    mcast_pbmp = 0;
                    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                        soc_ARLA_FWD_ENTRY0r_field_get(unit, &result, 
                            PORTID_Rf, &temp);
                    } else {
                        soc_ARLA_FWD_ENTRY0r_field_get(unit, &result, 
                            PORTIDf, &temp);
                    }
                    mcast_pbmp = temp;
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                            DRV_MEM_FIELD_DEST_BITMAP, (uint32 *)rep_entry, 
                            &temp)) < 0){
                        goto mem_search_exit;
                    }
                } else { /* The input is the unicast address */
                    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                        soc_ARLA_FWD_ENTRY0r_field_get(unit, &result, 
                            PORTID_Rf, &temp);
                    } else {
                        soc_ARLA_FWD_ENTRY0r_field_get(unit, &result, 
                            PORTIDf, &temp);
                    }
                    if ((rv = DRV_MEM_FIELD_SET(unit, 
                            DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                            (uint32 *)rep_entry, &temp)) < 0){
                        goto mem_search_exit;
                    }
                }

                soc_ARLA_FWD_ENTRY0r_field_get(unit, &result, 
                    ARL_AGEf, &temp);
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_AGE, (uint32 *)rep_entry, &temp)) < 0){
                    goto mem_search_exit;
                }

                soc_ARLA_FWD_ENTRY0r_field_get(unit, &result, 
                    ARL_VALIDf, &temp);
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_VALID, (uint32 *)rep_entry, 
                        &temp)) < 0){
                    goto mem_search_exit;
                }

                soc_ARLA_FWD_ENTRY0r_field_get(unit, &result, 
                    ARL_PRIf, &temp);
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_PRIORITY, (uint32 *)rep_entry, 
                        &temp)) < 0){
                    goto mem_search_exit;
                }

                soc_ARLA_FWD_ENTRY0r_field_get(unit, &result, 
                    ARL_STATICf, &temp);
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_STATIC, (uint32 *)rep_entry, 
                        &temp)) < 0){
                    goto mem_search_exit;
                }

                soc_ARLA_FWD_ENTRY0r_field_get(unit, &result, 
                    ARL_CONf, &temp);
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_ARL_CONTROL, (uint32 *)rep_entry, 
                        &temp)) < 0){
                    goto mem_search_exit;
                }
            }

            /* force one time report process only */
            if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                break;
            }
        }

        /* reporting the search result */
        if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){     /* normal search */
            if (matched_id != -1){
                /* assume binNum = matched_id in this condition  */
                if (binNum < 0 || binNum >= ROBO_GEX_L2_BUCKET_SIZE){
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s, unexpected search result!\n"), 
                              FUNCTION_NAME()));
                    rv = SOC_E_INTERNAL;
                } else {
                    /* matched entry founded */
                    rv = SOC_E_EXISTS;
                }
            } else if (binNum == -1){ 
                /* no matched entry and 4 bins all valid */
                rv = SOC_E_FULL;
            } else {
                /* processing here is no matched entry but a free bin_id 
                 * will be indicated to accept new insert entry.
                 */
                rv = SOC_E_NOT_FOUND;
            }
        } else {        /* conflict search */
            if (is_conflict == TRUE){
                rv = SOC_E_EXISTS;
            } else {
                /* no conflict entry. (no valid entry in this hash bucket */
                rv = SOC_E_NOT_FOUND;
            }
        }
mem_search_exit:
        ARL_MEM_SEARCH_UNLOCK(soc);
        LOG_INFO(BSL_LS_SOC_TESTS,
                 (BSL_META_U(unit,
                             "%s,%d,binNum=%d, matched_id=%d, isConflict=%d\n"),
                  FUNCTION_NAME(), rv, binNum, matched_id, is_conflict));
        return rv;
    }else {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s,%d, Unexpect process here!\n"),
                  FUNCTION_NAME(), __LINE__));
        return SOC_E_INTERNAL;
    }
}

#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
/*
 *  Function : _drv_gex_egr_v2v_mem_clear
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
_drv_gex_egr_v2v_mem_clear(int unit)
{
    int rv = SOC_E_NONE;
    uint32 retry;
    uint32  temp, reg_value;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s\n"), FUNCTION_NAME()));

    /* read control setting */
    if ((rv = REG_READ_EGRESS_VID_RMK_TBL_ACSr(unit, &reg_value)) < 0) {
        goto egrv2v_mem_clear_exit;
    }

    temp = 1;   /* set reset bit */
    soc_EGRESS_VID_RMK_TBL_ACSr_field_set(unit, &reg_value, 
        RESET_EVTf, &temp);

    temp = 1;
    soc_EGRESS_VID_RMK_TBL_ACSr_field_set(unit, &reg_value, 
        START_DONEf, &temp);

    
    if ((rv = REG_WRITE_EGRESS_VID_RMK_TBL_ACSr(unit, &reg_value)) < 0) {
        goto egrv2v_mem_clear_exit;        
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_EGRESS_VID_RMK_TBL_ACSr(unit, &reg_value)) < 0) {
            goto egrv2v_mem_clear_exit;
        }
        soc_EGRESS_VID_RMK_TBL_ACSr_field_get(unit, &reg_value, 
            START_DONEf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto egrv2v_mem_clear_exit;
    }

egrv2v_mem_clear_exit:     
    return rv;
}

/*
 *  Function : _drv_gex_egr_v2v_mem_read
 *
 *  Purpose :
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry_id    :   the entry's index (include port and entry_id).
 *      count       :   one or more netries to be read.
 *      entry_data  :   pointer to a buffer of 32-bit words 
 *                              to contain the read result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
static int
_drv_gex_egr_v2v_mem_read(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int     rv = SOC_E_NONE;
    int     i, port_id, index, temp_id;
    uint32  retry;
    uint32  temp, reg_value;
    egress_vid_remark_entry_t *v2v_mem = 0;
#ifdef BCM_POLAR_SUPPORT
    uint32 imp1_port_num;
#endif /* BCM_POLAR_SUPPORT */
#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
    uint32 skip_port = 6; /* port 6 is not existed in Northstar */
#endif /* BCM_NORTHSTAR_SUPPORT || NS+ */

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s,entry_id=%d,count=%d\n"), 
              FUNCTION_NAME(), entry_id, count));
    v2v_mem = (egress_vid_remark_entry_t *)entry;
    
    /* get port id from original entry id */
    port_id = (entry_id & DRV_EGR_V2V_PORT_ADDR_MASK) >> 
                DRV_EGR_V2V_PORT_ADDR_OFFSET;
    index = (entry_id & DRV_EGR_V2V_ENTRY_ADDR_MASK) >> 
                DRV_EGR_V2V_ENTRY_ADDR_OFFSET;
    
    if (SOC_IS_POLAR(unit)) {
#ifdef BCM_POLAR_SUPPORT
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_IMP1_PORT_NUM, &imp1_port_num));
#endif /* BCM_POLAR_SUPPORT */
    }

    if (SOC_IS_POLAR(unit)) {
#ifdef BCM_POLAR_SUPPORT
        if (port_id > DRV_EGR_V2V_IMP_PORT_ADDR_ID){
            rv = SOC_E_FULL;
            goto mem_read_exit;
        } else {
            if (port_id > imp1_port_num ) {
                /* port_id = 6 doesn't exist in Polar, 
                 * So port_id need to increase by 1 after port_id = 6 
                 */
                port_id++;
            }
        }
#endif /* BCM_POLAR_SUPPORT */
    } else if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
        if (port_id > DRV_EGR_V2V_IMP_PORT_ADDR_ID){
            rv = SOC_E_FULL;
            goto mem_read_exit;
        } else {
            if (port_id >= skip_port ) {
                /* port_id = 6 doesn't exist in Northstar, 
                * So port_id need to increase by 1 after port_id = 6 
                */
                port_id++;
            }
        }
#endif /* BCM_NORTHSTAR_SUPPORT || NS+ */
    } else
    {
        /* check if the port_id is not in the valid range */
        if (port_id > DRV_EGR_V2V_IMP_PORT_ADDR_ID){
            rv = SOC_E_FULL;
            goto mem_read_exit;
        } else if (port_id == DRV_EGR_V2V_IMP_PORT_ADDR_ID){
            port_id = DRV_EGR_V2V_IMP_PORT_SEARCH_ID;
        }
    }

    /* 1. per port containing up to 1~255 entries.
     * 2. the counter number might large enough to force SW to process at next 
     *      port's entryies.
     * 3. read process will be stopped if the reading count over the max entry.
     */
    for (i = 0, temp_id = index; i < count; i++ ) {
        
        /* ---- reading process ---- */
        
        /* set index key(port) */
        reg_value = 0;
        temp = port_id;
        if (SOC_IS_VULCAN(unit)) {
            soc_EGRESS_VID_RMK_TBL_ACSr_field_set(unit, &reg_value, 
                EGRESS_PORT_Rf, &temp);
        } else {
            soc_EGRESS_VID_RMK_TBL_ACSr_field_set(unit, &reg_value, 
                EGRESS_PORTf, &temp);
        }
        /* set index key(index) */
        temp = temp_id;
        soc_EGRESS_VID_RMK_TBL_ACSr_field_set(unit, &reg_value, 
            TBL_ADDRf, &temp);
                
        /* set read/write OP */
        temp = 0;   /* 0 means read OP in this table */
        soc_EGRESS_VID_RMK_TBL_ACSr_field_set(unit, &reg_value, 
            OPf, &temp);
        
        /* Start to read */
        temp = 1;
        soc_EGRESS_VID_RMK_TBL_ACSr_field_set(unit, &reg_value, 
            START_DONEf, &temp);

        
        if ((rv = REG_WRITE_EGRESS_VID_RMK_TBL_ACSr(unit, &reg_value)) < 0) {
            goto mem_read_exit;       
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_EGRESS_VID_RMK_TBL_ACSr(unit, &reg_value)) < 0) {
                goto mem_read_exit;       
            }
            soc_EGRESS_VID_RMK_TBL_ACSr_field_get(unit, &reg_value, 
                START_DONEf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_read_exit;
        }
        
        /* get result */

        if ((rv = REG_READ_EGRESS_VID_RMK_TBL_DATAr(unit, &reg_value)) < 0) {
            goto mem_read_exit;
        }

        sal_memset(v2v_mem, 0, sizeof(egress_vid_remark_entry_t));

            soc_EGRESS_VID_RMK_TBL_DATAr_field_get(unit, &reg_value, 
                OUTER_OPf, &temp);
            if ((rv = DRV_MEM_FIELD_SET
                (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_OUTER_OP, 
                    (uint32 *)v2v_mem, &temp)) < 0) {
                goto mem_read_exit;
            }

            soc_EGRESS_VID_RMK_TBL_DATAr_field_get(unit, &reg_value, 
                OUTER_VIDf, &temp);
            if ((rv = DRV_MEM_FIELD_SET
                (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_OUTER_VID, 
                    (uint32 *)v2v_mem, &temp)) < 0) {
                goto mem_read_exit;
            }

            soc_EGRESS_VID_RMK_TBL_DATAr_field_get(unit, &reg_value, 
                INNER_OPf, &temp);
            if ((rv = DRV_MEM_FIELD_SET
                (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_INNER_OP, 
                   (uint32 *)v2v_mem, &temp)) < 0) {
                goto mem_read_exit;
            }

            soc_EGRESS_VID_RMK_TBL_DATAr_field_get(unit, &reg_value, 
                INNER_VIDf, &temp);
            if ((rv = DRV_MEM_FIELD_SET
                (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_INNER_VID, 
                   (uint32 *)v2v_mem, &temp)) < 0) {
               goto mem_read_exit;
            }
        v2v_mem++;
        
        /* ---- index to next for count assignment ---- */
        temp_id++ ;
        if (temp_id >= DRV_EGRESS_V2V_NUM_PER_PORT){
            if ((port_id == DRV_EGR_V2V_IMP_PORT_SEARCH_ID) && 
                ((count - i) > 1)) {
                rv = SOC_E_FULL;
                goto mem_read_exit;
            } else {
                if (SOC_IS_POLAR(unit)) {
#ifdef BCM_POLAR_SUPPORT
                    if (port_id == imp1_port_num) {
                        /* port_id = 6 doesn't exist in Polar */
                        port_id = port_id + 2;
                    } else {
                        port_id ++;
                    }
#endif /* BCM_POLAR_SUPPORT */
               	} else if (SOC_IS_NORTHSTAR(unit) || 
               	    SOC_IS_NORTHSTARPLUS(unit)) {
#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
                    if (port_id == (skip_port - 1)) {
                        /* port_id = 6 doesn't exist in Northstar */
                        port_id = port_id + 2;
                    } else {
                        port_id ++;
                    }
#endif /* BCM_NORTHSTAR_SUPPORT || NS+ */
                } else
                {
                    port_id = ((port_id + 1) < DRV_EGR_V2V_IMP_PORT_ADDR_ID) ? 
                                (port_id + 1) : DRV_EGR_V2V_IMP_PORT_SEARCH_ID;
                }
                temp_id = 0;
            }
        }
    }
    
mem_read_exit:     
    return rv;
}

/*
 *  Function : _drv_gex_egr_v2v_mem_write
 *
 *  Purpose :
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry_id    :   the entry's index (include port and entry_id).
 *      count       :   one or more netries to be writen.
 *      entry_data  :   pointer to a buffer of 32-bit words 
 *                              to contain the writing result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
static int
_drv_gex_egr_v2v_mem_write(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int     rv = SOC_E_NONE;
    int     i, port_id, index, temp_id;
    uint32  retry;
    uint32  temp, reg_value;
    egress_vid_remark_entry_t *v2v_mem = 0;
#ifdef BCM_POLAR_SUPPORT
    uint32 imp1_port_num;
#endif /* BCM_POLAR_SUPPORT */    
#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
    uint32 skip_port = 6; /* port 6 is not existed in Northstar */
#endif /* BCM_NORTHSTAR_SUPPORT || NS+ */

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s,entry_id=%d,count=%d\n"), 
              FUNCTION_NAME(), entry_id, count));
    v2v_mem = (egress_vid_remark_entry_t *)entry;
    
    /* get port id from original entry id */
    port_id = (entry_id & DRV_EGR_V2V_PORT_ADDR_MASK) >> 
                DRV_EGR_V2V_PORT_ADDR_OFFSET;
    index = (entry_id & DRV_EGR_V2V_ENTRY_ADDR_MASK) >> 
                DRV_EGR_V2V_ENTRY_ADDR_OFFSET;
    
    if (SOC_IS_POLAR(unit)) {
#ifdef BCM_POLAR_SUPPORT
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_IMP1_PORT_NUM, &imp1_port_num));
#endif /* BCM_POLAR_SUPPORT */
    }

    if (SOC_IS_POLAR(unit)) {
#ifdef BCM_POLAR_SUPPORT
        if (port_id > DRV_EGR_V2V_IMP_PORT_ADDR_ID){
            rv = SOC_E_FULL;
            goto mem_write_exit;
        } else {
            if (port_id > imp1_port_num ) {
                /* port_id = 6 doesn't exist in Polar, 
                 * So port_id need to increase by 1 after port_id = 6 
                 */
                port_id++;
            }
        }
#endif /* BCM_POLAR_SUPPORT */
    } else if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
        if (port_id > DRV_EGR_V2V_IMP_PORT_ADDR_ID){
            rv = SOC_E_FULL;
            goto mem_write_exit;
        } else {
            if (port_id >= skip_port ) {
                /* port_id = 6 doesn't exist in Northstar, 
                 * So port_id need to increase by 1 after port_id = 6 
                 */
                port_id++;
            }
        }
#endif /* BCM_NORTHSTAR_SUPPORT || NS+ */
    } else
    {
        /* check if the port_id is not in the valid range */
        if (port_id > DRV_EGR_V2V_IMP_PORT_ADDR_ID){
            rv = SOC_E_FULL;
            goto mem_write_exit;
        } else if (port_id == DRV_EGR_V2V_IMP_PORT_ADDR_ID){
            port_id = DRV_EGR_V2V_IMP_PORT_SEARCH_ID;
        }
    }
    
    /* 1. per port containing up to 1~255 entries.
     * 2. the counter number might large enough to force SW to process at next 
     *      port's entryies.
     * 3. read process will be stopped if the reading count over the max entry.
     */
    for (i = 0, temp_id = index; i < count; i++ ) {
        
        /* set user data */

        reg_value = 0;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_OUTER_OP, 
                (uint32 *)v2v_mem, &temp));
        soc_EGRESS_VID_RMK_TBL_DATAr_field_set(unit, &reg_value, 
            OUTER_OPf, &temp);
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_OUTER_VID, 
                (uint32 *)v2v_mem, &temp));
        soc_EGRESS_VID_RMK_TBL_DATAr_field_set(unit, &reg_value, 
            OUTER_VIDf, &temp);
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_INNER_OP, 
                (uint32 *)v2v_mem, &temp));
        soc_EGRESS_VID_RMK_TBL_DATAr_field_set(unit, &reg_value, 
            INNER_OPf, &temp);
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_INNER_VID, 
                (uint32 *)v2v_mem, &temp));
        soc_EGRESS_VID_RMK_TBL_DATAr_field_set(unit, &reg_value, 
            INNER_VIDf, &temp);

        if ((rv = REG_WRITE_EGRESS_VID_RMK_TBL_DATAr(unit, &reg_value)) < 0) {
                goto mem_write_exit;
        }

        /* ---- writing process ---- */
        
        /* set index key(port) */
        reg_value = 0;
        temp = port_id;
        if (SOC_IS_VULCAN(unit)) {
            soc_EGRESS_VID_RMK_TBL_ACSr_field_set(unit, &reg_value, 
                EGRESS_PORT_Rf, &temp);
        } else {
            soc_EGRESS_VID_RMK_TBL_ACSr_field_set(unit, &reg_value, 
                EGRESS_PORTf, &temp);
        }
        /* set index key(index) */
        temp = temp_id;
        soc_EGRESS_VID_RMK_TBL_ACSr_field_set(unit, &reg_value, 
                TBL_ADDRf, &temp);
                
        /* set read/write OP */
        temp = 1;   /* 1 means write OP in this table */
        soc_EGRESS_VID_RMK_TBL_ACSr_field_set(unit, &reg_value, 
                OPf, &temp);
        
        /* Start to write */
        temp = 1;
        soc_EGRESS_VID_RMK_TBL_ACSr_field_set(unit, &reg_value, 
                START_DONEf, &temp);
        if ((rv = REG_WRITE_EGRESS_VID_RMK_TBL_ACSr(unit, &reg_value)) < 0) {
            goto mem_write_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_EGRESS_VID_RMK_TBL_ACSr(unit, &reg_value)) < 0) {
                goto mem_write_exit;
            }
            soc_EGRESS_VID_RMK_TBL_ACSr_field_get(unit, &reg_value, 
                START_DONEf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_write_exit;
        }
        

        v2v_mem++;
        
        /* ---- index to next for count assignment ---- */
        temp_id++ ;
        if (temp_id >= DRV_EGRESS_V2V_NUM_PER_PORT){
            if ((port_id == DRV_EGR_V2V_IMP_PORT_SEARCH_ID) && 
               ((count - i) > 1)) {
                rv = SOC_E_FULL;
                goto mem_write_exit;
            } else {
                if (SOC_IS_POLAR(unit)) {
#ifdef BCM_POLAR_SUPPORT
                    if (port_id == imp1_port_num) {
                        /* port_id = 6 doesn't exist in Polar */
                        port_id = port_id + 2;
                    } else {
                        port_id ++;
                    }
#endif /* BCM_POLAR_SUPPORT */
               	} else if (SOC_IS_NORTHSTAR(unit) || 
               	    SOC_IS_NORTHSTARPLUS(unit)) {
#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
                    if (port_id == (skip_port - 1)) {
                        /* port_id = 6 doesn't exist in Northstar */
                        port_id = port_id + 2;
                    } else {
                        port_id ++;
                    }
#endif /* BCM_NORTHSTAR_SUPPORT || NS+ */
                } else
                {
                    port_id = ((port_id + 1) < DRV_EGR_V2V_IMP_PORT_ADDR_ID) ? 
                                (port_id + 1) : DRV_EGR_V2V_IMP_PORT_SEARCH_ID;
                }
                temp_id = 0;
            }
        }
    }
    
mem_write_exit:     
    return rv;
}
#endif /* BCM_VULCAN_SUPPORT||BCM_STARFIGHTER_SUPPORT||BCM_POLAR_SUPPORT||
        * BCM_NORTHSTAR_SUPPORT ||  BCM_NORTHSTARPLUS_SUPPORT ||
        * BCM_STARFIGHTER3_SUPPORT
        */



/*
 *  Function : _drv_vlan_mem_read
 *
 *  Purpose :
 *
 *  Parameters :
 *      unit        :   unit id
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
 */
static int
_drv_gex_vlan_mem_read(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int     rv = SOC_E_NONE;
    int     i, index;
    uint32  retry;
    uint32  temp, reg_value;
    vlan_1q_entry_t *vlan_mem = 0;

    vlan_mem = (vlan_1q_entry_t *)entry;

    for (i = 0;i < count; i++ ) {        
        index = entry_id + i;

        /* fill index */
        reg_value = 0;
        temp = index;
        soc_ARLA_VTBL_ADDRr_field_set(unit, &reg_value, 
            VTBL_ADDR_INDEXf, &temp);

        if ((rv = REG_WRITE_ARLA_VTBL_ADDRr(unit, &reg_value)) < 0) {
            goto mem_read_exit;
        }

        /* read control setting */
        if ((rv = REG_READ_ARLA_VTBL_RWCTRLr(unit, &reg_value)) < 0) {
            goto mem_read_exit;
        }

        temp = GEX_MEM_OP_READ;
        soc_ARLA_VTBL_RWCTRLr_field_set(unit, &reg_value, 
            ARLA_VTBL_RW_CLRf, &temp);

        temp = 1;
        soc_ARLA_VTBL_RWCTRLr_field_set(unit, &reg_value, 
            ARLA_VTBL_STDNf, &temp);

        if ((rv = REG_WRITE_ARLA_VTBL_RWCTRLr(unit, &reg_value)) < 0) {
            goto mem_read_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_ARLA_VTBL_RWCTRLr(unit, &reg_value)) < 0) {
                goto mem_read_exit;
            }
            soc_ARLA_VTBL_RWCTRLr_field_get(unit, &reg_value, 
                ARLA_VTBL_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_read_exit;
        }

        /* get result */
        if ((rv = REG_READ_ARLA_VTBL_ENTRYr(unit, &reg_value)) < 0) {
            goto mem_read_exit;
        }

        soc_ARLA_VTBL_ENTRYr_field_get(unit, &reg_value, 
            MSPT_INDEXf, &temp);
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_VLAN, 
                DRV_MEM_FIELD_SPT_GROUP_ID, (uint32 *)vlan_mem, &temp));

        soc_ARLA_VTBL_ENTRYr_field_get(unit, &reg_value, 
            FWD_MAPf, &temp);
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_VLAN, 
                DRV_MEM_FIELD_PORT_BITMAP, (uint32 *)vlan_mem, &temp));
        soc_ARLA_VTBL_ENTRYr_field_get(unit, &reg_value, 
            UNTAG_MAPf, &temp);
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_VLAN, 
                DRV_MEM_FIELD_OUTPUT_UNTAG, (uint32 *)vlan_mem, &temp));
        soc_ARLA_VTBL_ENTRYr_field_get(unit, &reg_value, 
            FWD_MODEf, &temp);
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_VLAN, 
                DRV_MEM_FIELD_FWD_MODE, (uint32 *)vlan_mem, &temp));
#if defined(BCM_LOTUS_SUPPORT)
        if (SOC_IS_LOTUS(unit)) {
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_POLICER_EN, 
                (uint32 *)vlan_mem, &temp));
            soc_ARLA_VTBL_ENTRYr_field_get(unit, &reg_value, 
                POLICER_IDf, &temp);
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_POLICER_ID, 
                (uint32 *)vlan_mem, &temp));     
        }
#endif  /* (BCM_LOTUS_SUPPORT) */
        
        vlan_mem++;
    }

mem_read_exit:     
    return rv;
}

/*
 *  Function : _drv_gex_vlan_mem_write
 *
 *  Purpose :
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry_id    :  the entry's index of the memory to be read.
 *      count   :   one or more netries to be read.
 *      entry_data   :   pointer to a buffer of 32-bit words 
 *                              to contain the write result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
static int
_drv_gex_vlan_mem_write(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int i, index;
    uint32  reg_value;
    uint32  retry, temp;
    vlan_1q_entry_t *vlan_mem = 0;

    vlan_mem = (vlan_1q_entry_t *)entry;

    for (i = 0;i < count; i++ ) {
        index = entry_id + i;

        reg_value = 0;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(unit, DRV_MEM_VLAN, DRV_MEM_FIELD_SPT_GROUP_ID, 
             (uint32 *)vlan_mem, &temp));
        soc_ARLA_VTBL_ENTRYr_field_set(unit, &reg_value, 
            MSPT_INDEXf, &temp);
       
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(unit, DRV_MEM_VLAN, DRV_MEM_FIELD_PORT_BITMAP, 
             (uint32 *)vlan_mem, &temp));
        soc_ARLA_VTBL_ENTRYr_field_set(unit, &reg_value, 
            FWD_MAPf, &temp);
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG, 
             (uint32 *)vlan_mem, &temp));
        soc_ARLA_VTBL_ENTRYr_field_set(unit, &reg_value, 
            UNTAG_MAPf, &temp);
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(unit, DRV_MEM_VLAN, DRV_MEM_FIELD_FWD_MODE, 
             (uint32 *)vlan_mem, &temp));
        soc_ARLA_VTBL_ENTRYr_field_set(unit, &reg_value, 
            FWD_MODEf, &temp);
#if defined(BCM_LOTUS_SUPPORT)
        if (SOC_IS_LOTUS(unit)) {
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                    (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_POLICER_EN, 
                    (uint32 *)vlan_mem, &temp));    
            soc_ARLA_VTBL_ENTRYr_field_set(unit, &reg_value, 
                POLICER_ENf, &temp);
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                    (unit, DRV_MEM_VLAN, DRV_MEM_FIELD_POLICER_ID, 
                    (uint32 *)vlan_mem, &temp));   
            soc_ARLA_VTBL_ENTRYr_field_set(unit, &reg_value, 
                POLICER_IDf, &temp);
        }
#endif  /* (BCM_LOTUS_SUPPORT) */

        if ((rv = REG_WRITE_ARLA_VTBL_ENTRYr(unit, &reg_value)) < 0) {
                goto mem_write_exit;
        }

        /* fill index */
        reg_value = 0;
        temp = index;
        soc_ARLA_VTBL_ADDRr_field_set(unit, &reg_value, 
            VTBL_ADDR_INDEXf, &temp);

        if ((rv = REG_WRITE_ARLA_VTBL_ADDRr(unit, &reg_value)) < 0) {
            goto mem_write_exit;
        }

        /* read control setting */
        if ((rv = REG_READ_ARLA_VTBL_RWCTRLr(unit, &reg_value)) < 0) {
            goto mem_write_exit;
        }

        temp = GEX_MEM_OP_WRITE;
        soc_ARLA_VTBL_RWCTRLr_field_set(unit, &reg_value, 
            ARLA_VTBL_RW_CLRf, &temp);

        temp = 1;
        soc_ARLA_VTBL_RWCTRLr_field_set(unit, &reg_value, 
            ARLA_VTBL_STDNf, &temp);

        if ((rv = REG_WRITE_ARLA_VTBL_RWCTRLr(unit, &reg_value)) < 0) {
            goto mem_write_exit;
        }
        
        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_ARLA_VTBL_RWCTRLr(unit, &reg_value)) < 0) {
                goto mem_write_exit;
            }
            soc_ARLA_VTBL_RWCTRLr_field_get(unit, &reg_value, 
                ARLA_VTBL_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_write_exit;
        }

            vlan_mem ++;
    }

mem_write_exit:     
    return rv;
}


/*
 *  Function : _drv_vlan_mem_clear
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
_drv_gex_vlan_mem_clear(int unit)
{
    int rv = SOC_E_NONE;
    uint32 retry;
    uint32  temp, reg_value;


    /* read control setting */
    if ((rv = REG_READ_ARLA_VTBL_RWCTRLr(unit, &reg_value)) < 0) {
        goto vlan_mem_clear_exit;
    }

    temp = GEX_MEM_OP_CLEAR;
    soc_ARLA_VTBL_RWCTRLr_field_set(unit, &reg_value, 
            ARLA_VTBL_RW_CLRf, &temp);

    temp = 1;
    soc_ARLA_VTBL_RWCTRLr_field_set(unit, &reg_value, 
            ARLA_VTBL_STDNf, &temp);

    if ((rv = REG_WRITE_ARLA_VTBL_RWCTRLr(unit, &reg_value)) < 0) {
        goto vlan_mem_clear_exit;
    }

    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_ARLA_VTBL_RWCTRLr(unit, &reg_value)) < 0) {
            goto vlan_mem_clear_exit;
        }
        soc_ARLA_VTBL_RWCTRLr_field_get(unit, &reg_value, 
            ARLA_VTBL_STDNf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto vlan_mem_clear_exit;
    }

vlan_mem_clear_exit:     
    return rv;
}

/*
 *  Function : _drv_mstp_mem_read
 *
 *  Purpose :
 *      Get the width of mstp memory. 
 *      The value returned in data is width in bits.
 *
 *  Parameters :
 *      unit        :   unit id
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
 */
static int
_drv_gex_mstp_mem_read(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int i, index;
    uint32  reg_len, reg_addr;

    for (i = 0;i < count; i++ ) {        
        index = entry_id + i;
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT)
        if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)) {
            reg_addr = DRV_REG_ADDR(unit, INDEX(MST_TBLr), 0, index);
            reg_len = DRV_REG_LENGTH_GET(unit, INDEX(MST_TBLr));
        } else {
            reg_addr = DRV_REG_ADDR(unit, INDEX(MST_TABr), 0, index);
            reg_len = DRV_REG_LENGTH_GET(unit, INDEX(MST_TABr));
        }
#else   /* (BCM_VULCAN_SUPPORT) || (BCM_BLACKBIRD_SUPPORT) */
        reg_addr = DRV_REG_ADDR(unit, INDEX(MST_TABr), 0, index);
        reg_len = DRV_REG_LENGTH_GET(unit, INDEX(MST_TABr));
#endif  /* (BCM_VULCAN_SUPPORT) || (BCM_BLACKBIRD_SUPPORT) */

        if ((rv = DRV_REG_READ(unit, reg_addr, entry, reg_len)) < 0) {
            goto mem_read_exit;
        }
        entry++;
    }

mem_read_exit:     
    return rv;
}

/*
 *  Function : _drv_gex_mstp_mem_write
 *
 *  Purpose :
 *      Get the width of mstp memory. 
 *      The value returned in data is width in bits.
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry_id    :  the entry's index of the memory to be read.
 *      count   :   one or more netries to be read.
 *      entry_data   :   pointer to a buffer of 32-bit words 
 *                              to contain the write result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
static int
_drv_gex_mstp_mem_write(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int i, index;
    uint32  reg_len, reg_addr;

    for (i = 0;i < count; i++ ) {
        index = entry_id + i;
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT)
        if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)) {
            reg_addr = DRV_REG_ADDR(unit, INDEX(MST_TBLr), 0, index);
            reg_len = DRV_REG_LENGTH_GET(unit, INDEX(MST_TBLr));
        } else {
            reg_addr = DRV_REG_ADDR(unit, INDEX(MST_TABr), 0, index);
            reg_len = DRV_REG_LENGTH_GET(unit, INDEX(MST_TABr));
        }
#else   /* (BCM_VULCAN_SUPPORT) || (BCM_BLACKBIRD_SUPPORT) */
        reg_addr = DRV_REG_ADDR(unit, INDEX(MST_TABr), 0, index);
        reg_len = DRV_REG_LENGTH_GET(unit, INDEX(MST_TABr));
#endif  /* (BCM_VULCAN_SUPPORT) || (BCM_BLACKBIRD_SUPPORT) */

        if ((rv = DRV_REG_WRITE(unit, reg_addr, entry, reg_len)) < 0) {
            goto mem_write_exit;
        }
        entry++;
    }

mem_write_exit:     
    return rv;
}

/*
 *  Function : _drv_gex_mem_delete_all
 *
 *  Purpose :
 *      Delete all arl entries by fast aging all ports. 
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
_drv_gex_arl_mem_clear(int unit, int mcast)
{
    uint32            reg_value;
    uint32            temp, count;
    int rv = SOC_E_NONE;

        /* start fast aging process */
        reg_value = 0;

        /* Fast aging static and dynamic entries */
        temp = 1;
        soc_FAST_AGE_CTRLr_field_set(unit, &reg_value,
            EN_FAST_AGE_STATICf, &temp);
        soc_FAST_AGE_CTRLr_field_set(unit, &reg_value,
            EN_AGE_DYNAMICf, &temp);
        if (mcast) {
            soc_FAST_AGE_CTRLr_field_set(unit, &reg_value,
                EN_AGE_MCASTf, &temp);
        }

        /* Start Aging */
        soc_FAST_AGE_CTRLr_field_set(unit, &reg_value,
            FAST_AGE_STR_DONEf, &temp);
        if ((rv = REG_WRITE_FAST_AGE_CTRLr(unit, &reg_value)) < 0) {
            return rv;
        }
        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = REG_READ_FAST_AGE_CTRLr(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_FAST_AGE_CTRLr_field_get(unit, &reg_value,
                FAST_AGE_STR_DONEf, &temp);
            if (!temp) {
                break;
            }
        }
        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            return rv;
        }

        /* reset to init status while l2 thaw is proceeded. */
        soc_arl_frozen_sync_init(unit);

    /*
     * Restore register value to 0x2, otherwise aging will fail
     */
    reg_value = 0x2;
    REG_WRITE_FAST_AGE_CTRLr(unit, &reg_value);

    if (soc_feature(unit, soc_feature_mac_learn_limit)) {
        /* reset the SA learn limit related counters */
        rv = DRV_ARL_LEARN_COUNT_SET(unit, -1, DRV_PORT_SA_LRN_CNT_RESET, 0);
        if (SOC_FAILURE(rv)){
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "Failed on reset ARL LRN_CNT!\n")));
            return SOC_E_INTERNAL;
        }
    }

    return rv;
}

static int
_drv_gex_cfp_read(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    drv_cfp_entry_t cfp_entry;
    int entry_len, rv = SOC_E_NONE;
    uint8 *data_ptr;
    uint32 mem_id, counter;
    uint32 i, index_min, index_max;
    soc_mem_info_t *meminfo;

    data_ptr = (uint8 *)entry;
    /* Get the length of entry */
    switch (mem) {
    case DRV_MEM_TCAM_DATA:
    case DRV_MEM_TCAM_MASK:
        mem_id = INDEX(CFP_TCAM_IPV4_SCm);
        meminfo = &SOC_MEM_INFO(unit, mem_id);
        entry_len = meminfo->bytes;
        break;
    case DRV_MEM_CFP_ACT:
        if (SOC_IS_STARFIGHTER3(unit)) {
            mem_id = INDEX(CFP_ACTm);
        } else {
            mem_id = INDEX(CFP_ACT_POLm);
        }
        meminfo = &SOC_MEM_INFO(unit, mem_id);
        entry_len = meminfo->bytes;
        break;
    case DRV_MEM_CFP_METER:
        mem_id = INDEX(CFP_METERm);
        meminfo = &SOC_MEM_INFO(unit, mem_id);
        entry_len = meminfo->bytes;
        break;
    case DRV_MEM_CFP_STAT_IB:
    case DRV_MEM_CFP_STAT_OB:
        mem_id = INDEX(CFP_STAT_IBm);
        meminfo = &SOC_MEM_INFO(unit, mem_id);
        entry_len = meminfo->bytes;
        break;
    default:
        return SOC_E_PARAM;
    }

    /* check count */
    if (count < 1) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /* check index */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_len = (((entry_len - 1) / 4) + 1) * 4;

    for (i = 0; i < count; i++) {
        if (((entry_id + i) < index_min) || 
            ((entry_id + i) > index_max)) {
            return SOC_E_PARAM;
        }

        switch (mem) {
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
            sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
            if ((rv = DRV_CFP_ENTRY_READ(unit, (entry_id + i), 
                    DRV_CFP_RAM_TCAM, &cfp_entry)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_gex_cfp_read(mem=0x%x,entry_id=0x%x)\n"),
                           mem, entry_id + i));
                return rv;
            }
            if (mem == DRV_MEM_TCAM_DATA) {
                sal_memcpy(data_ptr, cfp_entry.tcam_data, entry_len);
            } else {
                sal_memcpy(data_ptr, cfp_entry.tcam_mask, entry_len);
            }
            break;
        case DRV_MEM_CFP_ACT:
            sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
            if ((rv = DRV_CFP_ENTRY_READ(unit, (entry_id + i), 
                    DRV_CFP_RAM_ACT, &cfp_entry)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_gex_cfp_read(mem=0x%x,entry_id=0x%x)\n"),
                           mem, entry_id + i));
                return rv;
            }
            sal_memcpy(data_ptr, cfp_entry.act_data, entry_len);
            break;
        case DRV_MEM_CFP_METER:
            sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
            if ((rv = DRV_CFP_ENTRY_READ(unit, (entry_id + i), 
                    DRV_CFP_RAM_METER, &cfp_entry)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_gex_cfp_read(mem=0x%x,entry_id=0x%x)\n"),
                           mem, entry_id + i));
                return rv;
            }
            sal_memcpy(data_ptr, cfp_entry.meter_data, entry_len);
            break;
        case DRV_MEM_CFP_STAT_IB:
            sal_memset(&counter, 0, sizeof(uint32));
            if ((rv = DRV_CFP_STAT_GET(unit, DRV_CFP_STAT_INBAND, 
                    (entry_id + i), &counter)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_gex_cfp_read(mem=0x%x,entry_id=0x%x)\n"),
                           mem, entry_id + i));
                return rv;
            }
            sal_memcpy(data_ptr, &counter, entry_len);
            break;
        case DRV_MEM_CFP_STAT_OB:
            sal_memset(&counter, 0, sizeof(uint32));
            if ((rv = DRV_CFP_STAT_GET(unit, DRV_CFP_STAT_OUTBAND, 
                    (entry_id + i), &counter)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_gex_cfp_read(mem=0x%x,entry_id=0x%x)\n"),
                           mem, entry_id + i));
                return rv;
            }
            sal_memcpy(data_ptr, &counter, entry_len);
            break;
        }
        data_ptr = data_ptr + entry_len;
    }

    return rv;
}


static int
_drv_gex_cfp_write(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    drv_cfp_entry_t cfp_entry;
    int entry_len, rv = SOC_E_NONE;
    uint8 *data_ptr;
    uint32 mem_id, counter;
    uint32 i, index_min, index_max;
    soc_mem_info_t *meminfo;

    data_ptr = (uint8 *)entry;
    /* Get the length of entry */
    switch (mem) {
    case DRV_MEM_TCAM_DATA:
    case DRV_MEM_TCAM_MASK:
        mem_id = INDEX(CFP_TCAM_IPV4_SCm);
        meminfo = &SOC_MEM_INFO(unit, mem_id);
        entry_len = meminfo->bytes;
        break;
    case DRV_MEM_CFP_ACT:
        if (SOC_IS_STARFIGHTER3(unit)) {
            mem_id = INDEX(CFP_ACTm);
        } else {
            mem_id = INDEX(CFP_ACT_POLm);
        }
        meminfo = &SOC_MEM_INFO(unit, mem_id);
        entry_len = meminfo->bytes;
        break;
    case DRV_MEM_CFP_METER:
        mem_id = INDEX(CFP_METERm);
        meminfo = &SOC_MEM_INFO(unit, mem_id);
        entry_len = meminfo->bytes;
        break;
    case DRV_MEM_CFP_STAT_IB:
    case DRV_MEM_CFP_STAT_OB:
        mem_id = INDEX(CFP_STAT_IBm);
        meminfo = &SOC_MEM_INFO(unit, mem_id);
        entry_len = meminfo->bytes;
        break;
    default:
        return SOC_E_PARAM;
    }

    /* check count */
    if (count < 1) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /* check index */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_len = (((entry_len - 1) / 4) + 1) * 4;

    for (i = 0; i < count; i++) {
        if (((entry_id + i) < index_min) || 
            ((entry_id + i) > index_max)) {
            return SOC_E_PARAM;
        }

        switch (mem) {
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
            sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
            if ((rv = DRV_CFP_ENTRY_READ(unit, (entry_id + i), 
                    DRV_CFP_RAM_TCAM, &cfp_entry)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_gex_cfp_read(mem=0x%x,entry_id=0x%x)\n"),
                           mem, entry_id + i));
                return rv;
            }
            if (mem == DRV_MEM_TCAM_DATA) {
                sal_memcpy(cfp_entry.tcam_data, data_ptr, entry_len);
            } else {
                sal_memcpy(cfp_entry.tcam_mask, data_ptr, entry_len);
            }
            if ((rv = DRV_CFP_ENTRY_WRITE(unit, (entry_id + i), 
                    DRV_CFP_RAM_TCAM, &cfp_entry)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_gex_cfp_write(mem=0x%x,entry_id=0x%x)\n"),
                           mem, entry_id + i));
                return rv;
            }
            break;
        case DRV_MEM_CFP_ACT:
            sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
            sal_memcpy(cfp_entry.act_data, data_ptr, entry_len);
            if ((rv = DRV_CFP_ENTRY_WRITE(unit, (entry_id + i), 
                DRV_CFP_RAM_ACT, &cfp_entry)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_gex_cfp_write(mem=0x%x,entry_id=0x%x)\n"),
                           mem, entry_id + i));
                return rv;
            }
            break;
        case DRV_MEM_CFP_METER:
            sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
            sal_memcpy(cfp_entry.meter_data, data_ptr, entry_len);
            if ((rv = DRV_CFP_ENTRY_WRITE(unit, (entry_id + i), 
                DRV_CFP_RAM_METER, &cfp_entry)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_gex_cfp_write(mem=0x%x,entry_id=0x%x)\n"),
                           mem, entry_id + i));
                return rv;
            }
            break;
        case DRV_MEM_CFP_STAT_IB:
            sal_memset(&counter, 0, sizeof(uint32));
            sal_memcpy(&counter, data_ptr, entry_len);
            if ((rv = DRV_CFP_STAT_SET(unit, DRV_CFP_STAT_INBAND, 
                (entry_id + i), counter)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_gex_cfp_read(mem=0x%x,entry_id=0x%x)\n"),
                           mem, entry_id + i));
                return rv;
            }
            break;
        case DRV_MEM_CFP_STAT_OB:
            sal_memset(&counter, 0, sizeof(uint32));
            sal_memcpy(&counter, data_ptr, entry_len);
            if ((rv = DRV_CFP_STAT_SET(unit, DRV_CFP_STAT_OUTBAND, 
                (entry_id + i), counter)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_gex_cfp_read(mem=0x%x,entry_id=0x%x)\n"),
                           mem, entry_id + i));
                return rv;
            }
            break;
        }
        data_ptr = data_ptr + entry_len;
    }

    return rv;
}

static int
_drv_gex_cfp_field_get(int unit, uint32 mem, uint32 field_index, 
        uint32 *entry, uint32 *fld_data)
{
    int rv = SOC_E_NONE;
    drv_cfp_entry_t cfp_entry;
    int entry_len;
    soc_mem_info_t *meminfo;
    
    sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
    switch (mem) {
    case DRV_MEM_TCAM_DATA:
        meminfo = &SOC_MEM_INFO(unit, INDEX(CFP_TCAM_S0m));
        entry_len = meminfo->bytes;
        sal_memcpy(cfp_entry.tcam_data, entry, entry_len);
        if ((rv = DRV_CFP_FIELD_GET(unit, 
                DRV_CFP_RAM_TCAM, field_index, &cfp_entry, fld_data)) < 0){
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_drv_gex_cfp_field_get(mem=0x%x,field=0x%x)\n"),
                       mem, field_index));
            return rv;
        }
        break;
    case DRV_MEM_TCAM_MASK:
        meminfo = &SOC_MEM_INFO(unit, INDEX(CFP_TCAM_MASKm));
        entry_len = meminfo->bytes;
        sal_memcpy(cfp_entry.tcam_mask, entry, entry_len);
        if ((rv = DRV_CFP_FIELD_GET(unit, 
                DRV_CFP_RAM_TCAM_MASK, field_index, &cfp_entry, fld_data)) < 0){
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_drv_gex_cfp_field_get(mem=0x%x,field=0x%x)\n"),
                       mem, field_index));
            return rv;
        }
        break;
    case DRV_MEM_CFP_ACT:
        meminfo = &SOC_MEM_INFO(unit, INDEX(CFP_ACT_POLm));
        entry_len = meminfo->bytes;
        sal_memcpy(cfp_entry.act_data, entry, entry_len);
        if ((rv = DRV_CFP_FIELD_GET(unit, 
                DRV_CFP_RAM_ACT, field_index, &cfp_entry, fld_data)) < 0){
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_drv_gex_cfp_field_get(mem=0x%x,field=0x%x)\n"),
                       mem, field_index));
            return rv;
        }
        break;
    case DRV_MEM_CFP_METER:
        meminfo = &SOC_MEM_INFO(unit, INDEX(CFP_METERm));
        entry_len = meminfo->bytes;
        sal_memcpy(cfp_entry.meter_data, entry, entry_len);
        if ((rv = DRV_CFP_FIELD_GET(unit, 
                DRV_CFP_RAM_METER, field_index, &cfp_entry, fld_data)) < 0){
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_drv_gex_cfp_field_get(mem=0x%x,field=0x%x)\n"),
                       mem, field_index));
            return rv;
        }
        break;
    default:
        rv = SOC_E_PARAM;
        break;
    }

    return rv;
}


static int
_drv_gex_cfp_field_set(int unit, uint32 mem, uint32 field_index, 
        uint32 *entry, uint32 *fld_data)
{
    int rv = SOC_E_NONE;
    drv_cfp_entry_t cfp_entry;
    int entry_len;
    soc_mem_info_t *meminfo;
    
    sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
    switch (mem) {
    case DRV_MEM_TCAM_DATA:
        meminfo = &SOC_MEM_INFO(unit, INDEX(CFP_TCAM_S0m));
        entry_len = meminfo->bytes;
        sal_memcpy(cfp_entry.tcam_data, entry, entry_len);
        if ((rv = DRV_CFP_FIELD_SET(unit, 
                DRV_CFP_RAM_TCAM, field_index, &cfp_entry, fld_data)) < 0){
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_drv_gex_cfp_field_set(mem=0x%x,field=0x%x)\n"),
                       mem, field_index));
            return rv;
        }
        sal_memcpy(entry, cfp_entry.tcam_data, entry_len);
        break;
    case DRV_MEM_TCAM_MASK:
        meminfo = &SOC_MEM_INFO(unit, INDEX(CFP_TCAM_MASKm));
        entry_len = meminfo->bytes;
        sal_memcpy(cfp_entry.tcam_mask, entry, entry_len);
        if ((rv = DRV_CFP_FIELD_SET(unit, 
                DRV_CFP_RAM_TCAM_MASK, field_index, &cfp_entry, fld_data)) < 0){
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_drv_gex_cfp_field_set(mem=0x%x,field=0x%x)\n"),
                       mem, field_index));
            return rv;
        }
        sal_memcpy(entry, cfp_entry.tcam_mask, entry_len);
        break;
    case DRV_MEM_CFP_ACT:
        meminfo = &SOC_MEM_INFO(unit, INDEX(CFP_ACT_POLm));
        entry_len = meminfo->bytes;
        sal_memcpy(cfp_entry.act_data, entry, entry_len);
        if ((rv = DRV_CFP_FIELD_SET(unit, 
                DRV_CFP_RAM_ACT, field_index, &cfp_entry, fld_data)) < 0){
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_drv_gex_cfp_field_set(mem=0x%x,field=0x%x)\n"),
                       mem, field_index));
            return rv;
        }
        sal_memcpy(entry, cfp_entry.act_data, entry_len);
        break;
    case DRV_MEM_CFP_METER:
        meminfo = &SOC_MEM_INFO(unit, INDEX(CFP_METERm));
        entry_len = meminfo->bytes;
        sal_memcpy(cfp_entry.meter_data, entry, entry_len);
        if ((rv = DRV_CFP_FIELD_SET(unit, 
                DRV_CFP_RAM_METER, field_index, &cfp_entry, fld_data)) < 0){
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "_drv_gex_cfp_field_set(mem=0x%x,field=0x%x)\n"),
                       mem, field_index));
            return rv;
        }
        sal_memcpy(entry, cfp_entry.meter_data, entry_len);
        break;
    default:
        rv = SOC_E_PARAM;
        break;
    }

    return rv;
}

#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
static int
_drv_starfighter_8051_mem_read(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int     i, index;
    uint32  retry;
    uint32  temp, reg_value;
    uint64  mem_data;
    int_8051_ram_entry_t    *mem_8051 = (int_8051_ram_entry_t *)entry;

    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    if ( (rv = REG_READ_MEM_CTRLr(unit, &reg_value)) < 0) {
        goto mem_8051_read_exit;
    }

    if (mem == DRV_MEM_8051_RAM) {
        temp = STARFIGHTER_8051_RAM_MEMORY;
    } else if (mem == DRV_MEM_8051_ROM) {
        /* 8051 ROM */
        temp = STARFIGHTER_8051_ROM_MEMORY;
    } else {
        rv = SOC_E_UNAVAIL;
        goto mem_8051_read_exit;
    }
    soc_MEM_CTRLr_field_set(unit, &reg_value, MEM_TYPEf, &temp);

    if ( (rv = REG_WRITE_MEM_CTRLr(unit, &reg_value)) < 0) {
        goto mem_8051_read_exit;
    }
      

    for (i = 0;i < count; i++ ) {        
        index = entry_id + i;

        /* fill index */
        reg_value = 0;
        temp = index;
        soc_MEM_ADDRr_field_set(unit, &reg_value, 
            MEM_ADRf, &temp);
        /* READ operation */
        temp  = GEX_MEM_OP_READ;
        soc_MEM_ADDRr_field_set(unit, &reg_value, 
            MEM_RWf, &temp);
        /* Kick off */
        temp = 1;
        soc_MEM_ADDRr_field_set(unit, &reg_value, 
            MEM_STDNf, &temp);
        
        if ( (rv = REG_WRITE_MEM_ADDRr(unit, &reg_value)) < 0) {
            goto mem_8051_read_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ( (rv = REG_READ_MEM_ADDRr(unit, &reg_value)) < 0) {
                goto mem_8051_read_exit;
            }
            soc_MEM_ADDRr_field_get(unit, &reg_value, 
                MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_8051_read_exit;
        }

        /* get result */
        COMPILER_64_ZERO(mem_data);
        if ( (rv = REG_READ_MEM_DEBUG_DATA_0_0r(
            unit, (uint32 *)&mem_data)) < 0) {
            goto mem_8051_read_exit;
        }
        sal_memcpy(mem_8051, &mem_data, sizeof(int_8051_ram_entry_t));
        
        mem_8051++;
    }

mem_8051_read_exit:    
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
}

static int
_drv_starfighter_8051_mem_write(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int     i, index;
    uint32  retry;
    uint32  temp, reg_value;
    uint64  mem_data;
    int_8051_ram_entry_t    *mem_8051 = (int_8051_ram_entry_t *)entry;

    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    if ( (rv = REG_READ_MEM_CTRLr(unit, &reg_value)) < 0) {
        goto mem_8051_write_exit;
    }

    if (mem == DRV_MEM_8051_RAM) {
        temp = STARFIGHTER_8051_RAM_MEMORY;
     } else {
        rv = SOC_E_UNAVAIL;
        goto mem_8051_write_exit;
    }
    soc_MEM_CTRLr_field_set(unit, &reg_value, MEM_TYPEf, &temp);

    if ( (rv = REG_WRITE_MEM_CTRLr(unit, &reg_value)) < 0) {
        goto mem_8051_write_exit;
    }
      

    for (i = 0;i < count; i++ ) {   
        COMPILER_64_ZERO(mem_data);
        sal_memcpy(&mem_data, mem_8051, sizeof(int_8051_ram_entry_t));
        if ( (rv = REG_WRITE_MEM_DEBUG_DATA_0_0r(
            unit, (uint32 *)&mem_data)) < 0) {
            goto mem_8051_write_exit;
        }
        
        index = entry_id + i;

        /* fill index */
        reg_value = 0;
        temp = index;
        soc_MEM_ADDRr_field_set(unit, &reg_value, 
            MEM_ADRf, &temp);
        /* READ operation */
        temp  = GEX_MEM_OP_WRITE;
        soc_MEM_ADDRr_field_set(unit, &reg_value, 
            MEM_RWf, &temp);
        /* Kick off */
        temp = 1;
        soc_MEM_ADDRr_field_set(unit, &reg_value, 
            MEM_STDNf, &temp);
        
        if ( (rv = REG_WRITE_MEM_ADDRr(unit, &reg_value)) < 0) {
            goto mem_8051_write_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ( (rv = REG_READ_MEM_ADDRr(unit, &reg_value)) < 0) {
                goto mem_8051_write_exit;
            }
            soc_MEM_ADDRr_field_get(unit, &reg_value, 
                MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_8051_write_exit;
        }
        
        mem_8051++;
    }

mem_8051_write_exit:    
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
}


#endif /* BCM_STARFIGHTER_SUPPORT || BCM_BLACKBIRD2_SUPPORT ||
        * BCM_STARFIGHTER3_SUPPORT
        */

/*
 *  Function : drv_gex_mem_length_get
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
drv_gex_mem_length_get(int unit, uint32 mem, uint32 *data)
{

    soc_mem_info_t *meminfo;
    
    switch (mem)
    {
    case DRV_MEM_ARL:
        meminfo = &SOC_MEM_INFO(unit, INDEX(L2_ARLm));
        break;
    case DRV_MEM_MARL:
        meminfo = &SOC_MEM_INFO(unit, INDEX(L2_MARL_SWm));
        break;            
    case DRV_MEM_VLAN:
        meminfo = &SOC_MEM_INFO(unit, INDEX(VLAN_1Qm));
        break;
    case DRV_MEM_MSTP:
        if (SOC_IS_STARFIGHTER3(unit)) {
            return SOC_E_UNAVAIL;
        } else {
            meminfo = &SOC_MEM_INFO(unit, INDEX(MSPT_TABm));
        }
        break;
    case DRV_MEM_EGRVID_REMARK:
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            meminfo = &SOC_MEM_INFO(unit, INDEX(EGRESS_VID_REMARKm));
        } else {
            return SOC_E_UNAVAIL;
        }
        break;
    case DRV_MEM_GEN:
        meminfo = &SOC_MEM_INFO(unit, INDEX(GEN_MEMORYm));
        break;
    case DRV_MEM_VLANVLAN:
    default:
        return SOC_E_PARAM;
    }

  *data = meminfo->index_max - meminfo->index_min + 1;

  return SOC_E_NONE;
}


/*
 *  Function : drv_gex_mem_width_get
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
drv_gex_mem_width_get(int unit, uint32 mem, uint32 *data)
{

    switch (mem)
    {
    case DRV_MEM_ARL:
        *data = sizeof(l2_arl_entry_t);
        break;
     case DRV_MEM_MARL:
        *data = sizeof(l2_marl_sw_entry_t);
        break;
    case DRV_MEM_VLAN:
        *data = sizeof(vlan_1q_entry_t);
        break;
    case DRV_MEM_MSTP:
        *data = sizeof(mspt_tab_entry_t);
        break;
    case DRV_MEM_EGRVID_REMARK:
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            *data = sizeof(egress_vid_remark_entry_t);
        } else {
            return SOC_E_UNAVAIL;
        }
        break;
    case DRV_MEM_GEN:
        *data = sizeof(gen_memory_entry_t);
        break;
    case DRV_MEM_VLANVLAN:
    default:
        return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}


 /*
 *  Function : drv_gex_mem_read
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
 */
int
drv_gex_mem_read(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    
    int rv = SOC_E_NONE;
    int i;
    uint32 retry, index_min, index_max, index;
    uint32 mem_id = 0;
    uint32 acc_ctrl = 0;
    uint32    temp;
    gen_memory_entry_t *gmem_entry = (gen_memory_entry_t *)entry;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    uint64 value64;
    uint32 *entry32;

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_mem_read(mem=0x%x,entry_id=0x%x,count=%d)\n"),
              mem, entry_id, count));
    switch (mem)
    {
    case DRV_MEM_ARL:
    case DRV_MEM_ARL_HW:
    case DRV_MEM_MARL:
        mem_id = INDEX(L2_ARLm);
        break;
    case DRV_MEM_VLAN:
        mem_id = INDEX(VLAN_1Qm);
        break;
    case DRV_MEM_MSTP:
        if (SOC_IS_STARFIGHTER3(unit)) {
            return SOC_E_UNAVAIL;
        } else {
            mem_id = INDEX(MSPT_TABm);
        }
        break;
    case DRV_MEM_EGRVID_REMARK:
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            mem_id = INDEX(EGRESS_VID_REMARKm);
        } else {
            return SOC_E_UNAVAIL;
        }
        break;
    case DRV_MEM_TCAM_DATA:
    case DRV_MEM_TCAM_MASK:
    case DRV_MEM_CFP_ACT:
    case DRV_MEM_CFP_METER:
    case DRV_MEM_CFP_STAT_IB:
    case DRV_MEM_CFP_STAT_OB:
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            rv = _drv_gex_cfp_read(
                unit, mem, entry_id, count, entry);
        } else {
            rv = SOC_E_UNAVAIL;
        }
        return rv;
    case DRV_MEM_8051_RAM:
    case DRV_MEM_8051_ROM:
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
        if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
            SOC_IS_STARFIGHTER3(unit)) {
            _drv_starfighter_8051_mem_read(
                unit, mem, entry_id, count, entry);
        } else {
            rv = SOC_E_UNAVAIL;
        }
#else   /* !BCM_STARFIGHTER_SUPPORT */
        rv = SOC_E_UNAVAIL;
#endif /* BCM_STARFIGHTER_SUPPORT || BB2 || SF3 */
        return rv;
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_PROTOCOLVLAN:
    case DRV_MEM_MACVLAN:
    case DRV_MEM_MCAST:
    case DRV_MEM_SECMAC:
    case DRV_MEM_GEN:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_PARAM;
    }

    /* add code here to check addr */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);

    
    if (((entry_id) < index_min) || 
        ((entry_id + count - 1) > index_max)) {
            return SOC_E_PARAM;
    }

    /* process read action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    /* add code here to check addr */
    if (mem_id == INDEX(L2_ARLm)) {        
        if ((rv = REG_READ_MEM_CTRLr(unit, &acc_ctrl)) < 0) {
            goto mem_read_exit;
        }

        temp = GEX_ARL_MEMORY;
        soc_MEM_CTRLr_field_set(unit, &acc_ctrl,
            MEM_TYPEf, &temp);
        
        if ((rv = REG_WRITE_MEM_CTRLr(unit, &acc_ctrl)) < 0) {
            goto mem_read_exit;
        }
    } else if (mem_id == INDEX(VLAN_1Qm)) {
        rv = _drv_gex_vlan_mem_read(unit, entry_id, count, entry);
        goto mem_read_exit;
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    } else if (mem_id == INDEX(EGRESS_VID_REMARKm)) {
        rv = _drv_gex_egr_v2v_mem_read(unit, entry_id, count, entry);
        goto mem_read_exit;
#endif /* BCM_VULCAN_SUPPORT||BCM_STARFIGHTER_SUPPORT||BCM_POLAR_SUPPORT||
        * BCM_NORTHSTAR_SUPPORT || BCM_NORTHSTARPLUS_SUPPORT ||
        * BCM_STARFIGHTER3_SUPPORT
        */
    } else {
        rv = _drv_gex_mstp_mem_read(unit, entry_id, count, entry);
        goto mem_read_exit;
    }

    /* check count */
    if (count < 1) {
        MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
        return SOC_E_PARAM;
    }

    for (i = 0;i < count; i++ ) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
             MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
            return SOC_E_PARAM;
        }

        /* Return data from cache if active */

        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL && CACHE_VMAP_TST(vmap, (entry_id + i))) {
            sal_memcpy(gmem_entry, 
                cache + (entry_id + i) * entry_size, entry_size);
            continue;
        }

        /* Read memory control register */
        if ((rv = REG_READ_MEM_ADDRr(unit, &acc_ctrl)) < 0) {
            goto mem_read_exit;
        }
        temp = GEX_MEM_OP_READ;
        soc_MEM_ADDRr_field_set(unit, &acc_ctrl, 
            MEM_RWf, &temp);

        index = entry_id + i;
        temp = index / 2;
        /* Set memory entry address */
        soc_MEM_ADDRr_field_set(unit, &acc_ctrl, 
            MEM_ADRf, &temp);

        temp = 1;
        soc_MEM_ADDRr_field_set(unit, &acc_ctrl, 
            MEM_STDNf, &temp);
        if ((rv = REG_WRITE_MEM_ADDRr(unit, &acc_ctrl)) < 0) {
            goto mem_read_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_MEM_ADDRr(unit, &acc_ctrl)) < 0) {
                goto mem_read_exit;
            }
            soc_MEM_ADDRr_field_get(unit, &acc_ctrl, 
                MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_read_exit;
        }

        /* Read the current generic memory entry */
        if ((index % 2) == 0) {
            /* Read bin 0 entry */
            entry32 = (uint32 *)gmem_entry;
            COMPILER_64_ZERO(value64);
            if ((rv = REG_READ_MEM_DEBUG_DATA_0_0r(unit, 
                (uint32 *)&value64)) < 0) {
                goto mem_read_exit;
            }
            entry32[0] = COMPILER_64_LO(value64);
            entry32[1] = COMPILER_64_HI(value64);
            if ((rv = REG_READ_MEM_DEBUG_DATA_0_1r(unit, 
                &(entry32[2]))) < 0) {
                goto mem_read_exit;
            }
            gmem_entry++;
            /* Read bin 1 entry */
            if (++i < count) {
                entry32 = (uint32 *)gmem_entry;
                COMPILER_64_ZERO(value64);
                if ((rv = REG_READ_MEM_DEBUG_DATA_1_0r(unit, 
                    (uint32 *)&value64)) < 0) {
                    goto mem_read_exit;
                }
                entry32[0] = COMPILER_64_LO(value64);
                entry32[1] = COMPILER_64_HI(value64);
                if ((rv = REG_READ_MEM_DEBUG_DATA_1_1r(unit, 
                    &(entry32[2]))) < 0) {
                    goto mem_read_exit;
                }
                gmem_entry++;    
            }
        } else {
            entry32 = (uint32 *)gmem_entry;
            COMPILER_64_ZERO(value64);
            if ((rv = REG_READ_MEM_DEBUG_DATA_1_0r(unit, 
                (uint32 *)&value64)) < 0) {
                goto mem_read_exit;
            }
            entry32[0] = COMPILER_64_LO(value64);
            entry32[1] = COMPILER_64_HI(value64);
            if ((rv = REG_READ_MEM_DEBUG_DATA_1_1r(unit, 
                &(entry32[2]))) < 0) {
                goto mem_read_exit;
            }
            gmem_entry++;  
        }
        

    }

 mem_read_exit:
     MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;

}

 /*
 *  Function : drv_gex_mem_write
 *
 *  Purpose :
 *      Writes an internal memory.
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
 */
int
drv_gex_mem_write(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    uint32 retry, index_min, index_max;
    uint32 i;
    uint32 mem_id = 0;
    uint32 acc_ctrl = 0;
    uint32    temp, index;
    gen_memory_entry_t *gmem_entry = (gen_memory_entry_t *)entry;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    uint64 value64;
    uint32 *entry32;

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_mem_write(mem=0x%x,entry_id=0x%x,count=%d)\n"),
              mem, entry_id, count));
         
    switch (mem)
    {
    case DRV_MEM_ARL:
    case DRV_MEM_ARL_HW:
    case DRV_MEM_MARL:
        mem_id = INDEX(L2_ARLm);
        break;
    case DRV_MEM_VLAN:
        mem_id = INDEX(VLAN_1Qm);
        break;
    case DRV_MEM_MSTP:
        if (SOC_IS_STARFIGHTER3(unit)) {
            return SOC_E_UNAVAIL;
        } else {
            mem_id = INDEX(MSPT_TABm);
        }
        break;
    case DRV_MEM_EGRVID_REMARK:
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) ||SOC_IS_NORTHSTAR(unit) ||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            mem_id = INDEX(EGRESS_VID_REMARKm);
        } else {
            return SOC_E_UNAVAIL;
        }
        break;
    case DRV_MEM_TCAM_DATA:
    case DRV_MEM_TCAM_MASK:
    case DRV_MEM_CFP_ACT:
    case DRV_MEM_CFP_METER:
    case DRV_MEM_CFP_STAT_IB:
    case DRV_MEM_CFP_STAT_OB:
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            rv = _drv_gex_cfp_write(
                unit, mem, entry_id, count, entry);
        } else {
            rv = SOC_E_UNAVAIL;
        }
        return rv;
    case DRV_MEM_8051_RAM:
#if defined(BCM_STARFIGHTER_SUPPORT) || defined(BCM_BLACKBIRD2_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
        if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
            SOC_IS_STARFIGHTER3(unit)) {
            _drv_starfighter_8051_mem_write(
                unit, mem, entry_id, count, entry);
        } else {
            rv = SOC_E_UNAVAIL;
        }
#else   /* !BCM53125 */
        rv = SOC_E_UNAVAIL;
#endif /* BCM_53125  || BB2 || SF3*/
        return rv;
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_PROTOCOLVLAN:
    case DRV_MEM_MACVLAN:
    case DRV_MEM_GEN:
    case DRV_MEM_MCAST:
    case DRV_MEM_SECMAC:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_PARAM;
    }

    /* add code here to check addr */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);
    
    if (count < 1) {
        return SOC_E_PARAM;
    }
    if (((entry_id) < index_min) || 
        ((entry_id + count - 1) > index_max)) {
            return SOC_E_PARAM;
    }

    /* process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    if (mem_id == INDEX(L2_ARLm) ){
        if ((rv = REG_READ_MEM_CTRLr(unit, &acc_ctrl)) < 0) {
            goto mem_write_exit;
        }
        
        temp = GEX_ARL_MEMORY;
        soc_MEM_CTRLr_field_set(unit, &acc_ctrl,
            MEM_TYPEf, &temp);

        if ((rv = REG_WRITE_MEM_CTRLr(unit, &acc_ctrl)) < 0) {
            goto mem_write_exit;
        }
    } else if (mem_id == INDEX(VLAN_1Qm)){
        rv = _drv_gex_vlan_mem_write(unit, entry_id, count, entry);
        goto mem_write_exit;
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    } else if (mem_id == INDEX(EGRESS_VID_REMARKm)) {
        rv = _drv_gex_egr_v2v_mem_write(unit, entry_id, count, entry);
        goto mem_write_exit;
#endif /* BCM_VULCAN_SUPPORT||BCM_STARFIGHTER_SUPPORT||BCM_POLAR_SUPPORT||
        * BCM_NORTHSTAR_SUPPORT || BCM_NORTHSTARPLUS_SUPPORT ||
        * BCM_STARFIGHTER3_SUPPORT
        */
    } else {
        rv = _drv_gex_mstp_mem_write(unit, entry_id, count, entry);
        goto mem_write_exit;
    }
    
    for (i = 0; i < count; i++) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
            return SOC_E_PARAM;
        }

        /* write data */
        index = entry_id + i;
        if ((index % 2) == 0) {
            COMPILER_64_ZERO(value64);
            entry32 = (uint32 *)gmem_entry;
            COMPILER_64_SET(value64, entry32[1], entry32[0]);
            if ((rv = REG_WRITE_MEM_DEBUG_DATA_0_0r(unit, 
                (uint32 *)&value64)) < 0) {
                goto mem_write_exit;
            }
            if ((rv = REG_WRITE_MEM_DEBUG_DATA_0_1r(unit, 
                &(entry32[2]))) < 0) {
                goto mem_write_exit;
            }
            if (++i < count) {
                gmem_entry++;
                COMPILER_64_ZERO(value64);
                entry32 = (uint32 *)gmem_entry;
                COMPILER_64_SET(value64, entry32[1], entry32[0]);
                if ((rv = REG_WRITE_MEM_DEBUG_DATA_1_0r(
                    unit, (uint32 *)&value64)) < 0) {
                    goto mem_write_exit;
                }
                if ((rv = REG_WRITE_MEM_DEBUG_DATA_1_1r(unit, 
                    &(entry32[2]))) < 0) {
                    goto mem_write_exit;
                }
            }
        } else {
            COMPILER_64_ZERO(value64);
            entry32 = (uint32 *)gmem_entry;
            COMPILER_64_SET(value64, entry32[1], entry32[0]);
            if ((rv = REG_WRITE_MEM_DEBUG_DATA_1_0r(unit, 
                (uint32 *)&value64)) < 0) {
                goto mem_write_exit;
            }
            if ((rv = REG_WRITE_MEM_DEBUG_DATA_1_1r(unit, 
                &(entry32[2]))) < 0) {
                goto mem_write_exit;
            }
        }
        
        /* set memory address */
        temp = index / 2;
        soc_MEM_ADDRr_field_set(unit, &acc_ctrl, 
            MEM_ADRf, &temp);

        temp = GEX_MEM_OP_WRITE;
        soc_MEM_ADDRr_field_set(unit, &acc_ctrl, 
            MEM_RWf, &temp);

        temp = 1;
        soc_MEM_ADDRr_field_set(unit, &acc_ctrl, 
            MEM_STDNf, &temp);
        if ((rv = REG_WRITE_MEM_ADDRr(unit, &acc_ctrl)) < 0) {
            goto mem_write_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_MEM_ADDRr(unit, &acc_ctrl)) < 0) {
                goto mem_write_exit;
            }
            soc_MEM_ADDRr_field_get(unit, &acc_ctrl, 
                MEM_STDNf, &temp);
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
            if (((index % 2) == 0) && (i < count)) {
                sal_memcpy(cache + (index) * entry_size, 
                    --gmem_entry, entry_size * 2);
                CACHE_VMAP_SET(vmap, (index));
                CACHE_VMAP_SET(vmap, (index+1));
                gmem_entry++;
            } else {
                sal_memcpy(cache + (index) * entry_size, 
                    gmem_entry, entry_size);
                CACHE_VMAP_SET(vmap, (index));
            }
        }
        gmem_entry++;
    }

 mem_write_exit:
     MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
    
}

/*
 *  Function : drv_gex_mem_field_get
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
drv_gex_mem_field_get(int unit, uint32 mem, 
    uint32 field_index, uint32 *entry, uint32 *fld_data)
{

    soc_mem_info_t    *meminfo;
    soc_field_info_t    *fieldinfo;
    uint32                  mask, mask_hi, mask_lo;
    int            mem_id, field_id;
    int            i, wp, bp, len;
#ifdef BE_HOST
    uint32              val32;
#endif

    switch (mem)
    {
    case DRV_MEM_ARL_HW:
        mem_id = INDEX(L2_ARLm);
        if (field_index == DRV_MEM_FIELD_MAC) {
            if (SOC_IS_STARFIGHTER3(unit)) {
                field_id = INDEX(MAC_ADDRf);
            } else {
                field_id = INDEX(MACADDRf);
            }
        }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
            if (SOC_IS_STARFIGHTER3(unit)) {
                field_id = INDEX(UCAST_PID_MCAST_PORTMAPf);
            } else if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(PORTID_Rf);
            } else {
                field_id = INDEX(MULTCAST_PORTMAPf);
            }
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(PRIORITY_Rf);
            } else {
                field_id = INDEX(TCf);
            }
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(VID_Rf);
            } else {
                field_id = INDEX(VIDf);
            }
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            if (SOC_IS_STARFIGHTER3(unit)) {
                field_id = INDEX(STATIC_STSf);
            } else {
                field_id = INDEX(STATICf);
            }
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(VALID_Rf);
            } else {
                field_id = INDEX(VALIDf);
            }
        }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
            field_id = INDEX(CONTROLf);
        }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(RESERVED_Rf);
            } else {
                field_id = INDEX(MULTCAST_PORTMAPf);
            }
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_ARL:
        mem_id = INDEX(L2_ARL_SWm);
        if (field_index == DRV_MEM_FIELD_MAC) {
            if (SOC_IS_STARFIGHTER3(unit)) {
                field_id = INDEX(MAC_ADDRf);
            } else {
                field_id = INDEX(MACADDRf);
            }
        }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(PORTID_Rf);
            } else {
                field_id = INDEX(PORTIDf);
            }
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(PRIORITY_Rf);
            } else {
                field_id = INDEX(TCf);
            }
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(VID_Rf);
            } else {
                field_id = INDEX(VIDf);
            }
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            field_id = INDEX(STATICf);
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(VALID_Rf);
            } else {
                field_id = INDEX(VALIDf);
            }
        }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
            field_id = INDEX(CONTROLf);
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_MARL:
        mem_id = INDEX(L2_MARL_SWm);
        if (field_index == DRV_MEM_FIELD_MAC) {
            if (SOC_IS_STARFIGHTER3(unit)) {
                field_id = INDEX(MAC_ADDRf);
            } else {
                field_id = INDEX(MACADDRf);            
            }
        }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
            field_id =INDEX(PORTBMPf);
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(PRIORITY_Rf);
            } else {
                field_id = INDEX(TCf);
            }
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(VID_Rf);
            } else {
                field_id = INDEX(VIDf);
            }
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            field_id = INDEX(STATICf);
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(VALID_Rf);
            } else {
                field_id = INDEX(VALIDf);
            }
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
        }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
            field_id = INDEX(CONTROLf);
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_VLAN:
        mem_id = INDEX(VLAN_1Qm);
        if (field_index == DRV_MEM_FIELD_SPT_GROUP_ID) {
            field_id = INDEX(MSPT_IDf);
        }else if (field_index == DRV_MEM_FIELD_OUTPUT_UNTAG) {
            field_id = INDEX(UNTAG_MAPf);
        }else if (field_index == DRV_MEM_FIELD_PORT_BITMAP) {
            field_id = INDEX(FORWARD_MAPf);
        }else if (field_index == DRV_MEM_FIELD_FWD_MODE) {
            field_id = INDEX(FWD_MODEf);
#ifdef BCM_LOTUS_SUPPORT
        }else if (field_index == DRV_MEM_FIELD_POLICER_EN) {
            field_id = INDEX(POLICER_ENf);
            if (!SOC_IS_LOTUS(unit)){
                return SOC_E_PARAM;
            }
        }else if (field_index == DRV_MEM_FIELD_POLICER_ID) {
            field_id = INDEX(POLICER_IDf);
            if (!SOC_IS_LOTUS(unit)){
                return SOC_E_PARAM;
            }
#endif /* BCM_LOTUS_SUPPORT */
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_MSTP:
        if (SOC_IS_STARFIGHTER3(unit)) {
            return SOC_E_UNAVAIL;
        }
        mem_id = INDEX(MSPT_TABm);
        if (field_index == DRV_MEM_FIELD_MSTP_PORTST) {
            sal_memcpy(fld_data, entry, sizeof(mspt_tab_entry_t));
            return SOC_E_NONE;
        } else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_EGRVID_REMARK:
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            mem_id = INDEX(EGRESS_VID_REMARKm);
            if (field_index == DRV_MEM_FIELD_OUTER_OP) {
                field_id = INDEX(OUTER_OPf);
            }else if (field_index == DRV_MEM_FIELD_OUTER_VID) {
                field_id = INDEX(OUTER_VIDf);
            }else if (field_index == DRV_MEM_FIELD_INNER_OP) {
                field_id = INDEX(INNER_OPf);
            }else if (field_index == DRV_MEM_FIELD_INNER_VID) {
                field_id = INDEX(INNER_VIDf);
            }else {
                return SOC_E_PARAM;
            }
        } else {
            return SOC_E_UNAVAIL;
        }
        break;
    case DRV_MEM_TCAM_DATA:
    case DRV_MEM_TCAM_MASK:
    case DRV_MEM_CFP_ACT:
    case DRV_MEM_CFP_METER:
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            return _drv_gex_cfp_field_get
                (unit, mem, field_index, entry, fld_data);
        } else {
            return SOC_E_UNAVAIL;
        }
        break;
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_MACVLAN:
    case DRV_MEM_PROTOCOLVLAN:
        return SOC_E_UNAVAIL;
    case DRV_MEM_MCAST:
    case DRV_MEM_SECMAC:
    case DRV_MEM_GEN:
    default:
        return SOC_E_PARAM;
    }    
    
    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);

    assert(entry);
    assert(fld_data);
#if defined(BCM_STARFIGHTER3_SUPPORT)
    if((SOC_IS_STARFIGHTER3(unit)) &&((mem == DRV_MEM_VLAN) ||
                (mem == DRV_MEM_EGRVID_REMARK))) {
        _drv_gex_sf3_fieldinfo_get(unit, mem, field_id, (uint32 *) &fieldinfo);
    } else
#endif /* BCM_STARFIGHTER3_SUPPORT  */
    {
        SOC_FIND_FIELD(field_id,
             meminfo->fields,
             meminfo->nFields,
             fieldinfo);
    }

    assert(fieldinfo);
    bp = fieldinfo->bp;
#ifdef BE_HOST
    if ((fieldinfo->len > 32) && (fieldinfo->len <= 64)) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif


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
#ifdef BE_HOST
    if ((fieldinfo->len > 32) && (fieldinfo->len <= 64)) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif
    return SOC_E_NONE;
}

 /*
 *  Function : drv_gex_mem_field_set
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
drv_gex_mem_field_set(int unit, uint32 mem, 
    uint32 field_index, uint32 *entry, uint32 *fld_data)
{
    soc_mem_info_t    *meminfo;
    soc_field_info_t    *fieldinfo;
    uint32                  mask, mask_hi, mask_lo;
    int            mem_id, field_id;
    int            i, wp, bp, len;
#ifdef BE_HOST
    uint32               val32;
#endif

    switch (mem)
    {
    case DRV_MEM_ARL_HW:
        mem_id = INDEX(L2_ARLm);
        if (field_index == DRV_MEM_FIELD_MAC) {
            if (SOC_IS_STARFIGHTER3(unit)) {
                field_id = INDEX(MAC_ADDRf);
            } else {
                field_id = INDEX(MACADDRf);
            }
        }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
            if (SOC_IS_STARFIGHTER3(unit)) {
                field_id = INDEX(UCAST_PID_MCAST_PORTMAPf);
            } else if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(PORTID_Rf);
            } else {
                field_id = INDEX(MULTCAST_PORTMAPf);
            }
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(PRIORITY_Rf);
            } else {
                field_id = INDEX(TCf);
            }
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(VID_Rf);
            } else {
                field_id = INDEX(VIDf);
            }
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            field_id = INDEX(STATICf);
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(VALID_Rf);
            } else {
                field_id = INDEX(VALIDf);
            }
        }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
            field_id = INDEX(CONTROLf);
        }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(RESERVED_Rf);
            } else {
                field_id = INDEX(MULTCAST_PORTMAPf);
            }
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_ARL:
        mem_id = INDEX(L2_ARL_SWm);
        if (field_index == DRV_MEM_FIELD_MAC) {
            if (SOC_IS_STARFIGHTER3(unit)) {
                field_id = INDEX(MAC_ADDRf);
            } else {
                field_id = INDEX(MACADDRf);
            }
        }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(PORTID_Rf);
            } else {
                field_id = INDEX(PORTIDf);
            }
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(PRIORITY_Rf);
            } else {
                field_id = INDEX(TCf);
            }
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(VID_Rf);
            } else {
                field_id = INDEX(VIDf);
            }
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            field_id = INDEX(STATICf);
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(VALID_Rf);
            } else {
                field_id = INDEX(VALIDf);
            }
        }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
            field_id = INDEX(CONTROLf);
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_MARL:
        mem_id = INDEX(L2_MARL_SWm);
        if (field_index == DRV_MEM_FIELD_MAC) {
            if (SOC_IS_STARFIGHTER3(unit)) {
                field_id = INDEX(MAC_ADDRf);
            } else {
                field_id = INDEX(MACADDRf);            
            }
        }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
            field_id =INDEX(PORTBMPf);
        }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(PRIORITY_Rf);
            } else {
                field_id = INDEX(TCf);
            }
        }else if (field_index == DRV_MEM_FIELD_VLANID) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(VID_Rf);
            } else {
                field_id = INDEX(VIDf);
            }
        }else if (field_index == DRV_MEM_FIELD_STATIC) {
            field_id = INDEX(STATICf);
        }else if (field_index == DRV_MEM_FIELD_VALID) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
                field_id = INDEX(VALID_Rf);
            } else {
                field_id = INDEX(VALIDf);
            }
        }else if (field_index == DRV_MEM_FIELD_AGE) {
            field_id = INDEX(AGEf);
        }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
            field_id = INDEX(CONTROLf);
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_VLAN:
        mem_id = INDEX(VLAN_1Qm);
        if (field_index == DRV_MEM_FIELD_SPT_GROUP_ID) {
            field_id = INDEX(MSPT_IDf);
        }else if (field_index == DRV_MEM_FIELD_OUTPUT_UNTAG) {
            field_id = INDEX(UNTAG_MAPf);
        }else if (field_index == DRV_MEM_FIELD_PORT_BITMAP) {
            field_id = INDEX(FORWARD_MAPf);
        }else if (field_index == DRV_MEM_FIELD_FWD_MODE) {
            field_id = INDEX(FWD_MODEf);
#ifdef BCM_LOTUS_SUPPORT
        }else if (field_index == DRV_MEM_FIELD_POLICER_EN) {
            field_id = INDEX(POLICER_ENf);
            if (!SOC_IS_LOTUS(unit)){
                return SOC_E_PARAM;
            }
        }else if (field_index == DRV_MEM_FIELD_POLICER_ID) {
            field_id = INDEX(POLICER_IDf);
            if (!SOC_IS_LOTUS(unit)){
                return SOC_E_PARAM;
            }
#endif  /* BCM_LOTUS_SUPPORT */
        }else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_MSTP:
        if (SOC_IS_STARFIGHTER3(unit)) {
            return SOC_E_UNAVAIL;
        }
        mem_id = INDEX(MSPT_TABm);
        if (field_index == DRV_MEM_FIELD_MSTP_PORTST) {
            sal_memcpy(fld_data, entry, sizeof(mspt_tab_entry_t));
            return SOC_E_NONE;
        } else {
            return SOC_E_PARAM;
        }
        break;
    case DRV_MEM_EGRVID_REMARK:
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            mem_id = INDEX(EGRESS_VID_REMARKm);
            if (field_index == DRV_MEM_FIELD_OUTER_OP) {
                field_id = INDEX(OUTER_OPf);
            }else if (field_index == DRV_MEM_FIELD_OUTER_VID) {
                field_id = INDEX(OUTER_VIDf);
            }else if (field_index == DRV_MEM_FIELD_INNER_OP) {
                field_id = INDEX(INNER_OPf);
            }else if (field_index == DRV_MEM_FIELD_INNER_VID) {
                field_id = INDEX(INNER_VIDf);
            }else {
                return SOC_E_PARAM;
            }
        } else {
            return SOC_E_UNAVAIL;
        }
        break;
    case DRV_MEM_TCAM_DATA:
    case DRV_MEM_TCAM_MASK:
    case DRV_MEM_CFP_ACT:
    case DRV_MEM_CFP_METER:
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            return _drv_gex_cfp_field_set
                (unit, mem, field_index, entry, fld_data);
        } else {
            return SOC_E_UNAVAIL;
        }
        break;
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_MACVLAN:
    case DRV_MEM_PROTOCOLVLAN:
        return SOC_E_UNAVAIL;
    case DRV_MEM_MCAST:
    case DRV_MEM_SECMAC:
    case DRV_MEM_GEN:
    default:
        return SOC_E_PARAM;
    }    
    
    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);

    assert(entry);
    assert(fld_data);
#if defined(BCM_STARFIGHTER3_SUPPORT)
    if((SOC_IS_STARFIGHTER3(unit)) &&((mem == DRV_MEM_VLAN) ||
                (mem == DRV_MEM_EGRVID_REMARK))) {
        _drv_gex_sf3_fieldinfo_get(unit, mem, field_id, (uint32 *) &fieldinfo);
    } else
#endif /* BCM_STARFIGHTER3_SUPPORT  */
    {
        SOC_FIND_FIELD(field_id,
             meminfo->fields,
             meminfo->nFields,
             fieldinfo);
    }
    assert(fieldinfo);
#ifdef BE_HOST
    if ((fieldinfo->len > 32) && (fieldinfo->len <= 64)) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif
    
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
#ifdef BE_HOST
    if ((fieldinfo->len > 32) && (fieldinfo->len <= 64)) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif
    return SOC_E_NONE;
}

/*
 *  Function : drv_gex_mem_clear
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
drv_gex_mem_clear(int unit, uint32 mem)
{
    int rv = SOC_E_NONE;
    uint32 count;
    int mem_id;
    uint32 *entry;
    uint32 del_id;


    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_mem_clear : mem=0x%x\n"), mem));
    switch(mem) {
    case DRV_MEM_ARL:
    case DRV_MEM_ARL_HW:
        rv = _drv_gex_arl_mem_clear(unit, 0);
        return rv;
        break;
    case DRV_MEM_MARL:
        rv = _drv_gex_arl_mem_clear(unit, 1);
        return rv;
        break;
    case DRV_MEM_VLAN:
        rv = _drv_gex_vlan_mem_clear(unit);
        return rv;
    case DRV_MEM_EGRVID_REMARK:
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            _drv_gex_egr_v2v_mem_clear(unit);
            return rv;
        } else {
#endif /* BCM_VULCAN_SUPPORT||BCM_STARFIGHTER_SUPPORT||BCM_POLAR_SUPPORT||
        * BCM_NORTHSTAR_SUPPORT  || BCM_NORTHSTARPLUS_SUPPORT ||
        * BCM_STARFIGHTER3_SUPPORT
        */
            return SOC_E_UNAVAIL;
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_POLAR_SUPPORT)|| defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        }
#endif /* BCM_VULCAN_SUPPORT||BCM_STARFIGHTER_SUPPORT||BCM_POLAR_SUPPORT||
        * BCM_NORTHSTAR_SUPPORT  || BCM_NOTHSTARPLUS_SUPPORT ||
        * BCM_STARFIGHTER3_SUPPORT
        */
        break;
    case DRV_MEM_MSTP:
        if (SOC_IS_STARFIGHTER3(unit)) {
            return SOC_E_UNAVAIL;
        } else {
            mem_id = INDEX(MSPT_TABm);
        }
        break;
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_MACVLAN:
    case DRV_MEM_PROTOCOLVLAN:
        return SOC_E_UNAVAIL;
    case DRV_MEM_MCAST:
    case DRV_MEM_GEN:
    case DRV_MEM_SECMAC:
    default:
        return SOC_E_PARAM;
    }
   
    count = soc_mem_index_count(unit, mem_id);

    /* Prevent the Coverity Check(multiple sizeof(uint32))*/
    entry = (uint32 *)sal_alloc((sizeof(_soc_mem_entry_null_zeroes) / 
            sizeof(uint32)) * sizeof(uint32),
            "null_entry");
    if (entry == NULL) {
        LOG_CLI((BSL_META_U(unit,
                            "Insufficient memory.\n")));
        return SOC_E_MEMORY;
    }
    sal_memset(entry, 0, sizeof(_soc_mem_entry_null_zeroes));

    for (del_id = 0; del_id < count; del_id++) {
        rv = DRV_MEM_WRITE(unit, mem, del_id, 1, entry);
        if (rv != SOC_E_NONE){
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s : failed at mem_id=%d, entry=%d!\n"), 
                      FUNCTION_NAME(), mem, del_id));
            break;
        }
    }
    sal_free(entry);

    return rv;

    
}

/*
 *  Function : drv_gex_mem_search
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
 *      Only for ARL memory now.
 *
 */
int 
drv_gex_mem_search(int unit, uint32 mem, 
    uint32 *key, uint32 *entry, uint32 *entry_1, uint32 flags)
{
    int                 rv = SOC_E_NONE;
    int                 index;

    switch(mem) {
    case DRV_MEM_ARL:
    case DRV_MEM_MARL:
        break;
    case DRV_MEM_VLAN:
    case DRV_MEM_MSTP:
    case DRV_MEM_MCAST:
    case DRV_MEM_GEN:
    case DRV_MEM_SECMAC:
    case DRV_MEM_PROTOCOLVLAN:
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_MACVLAN:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_PARAM;
    }

    rv = _drv_gex_mem_search(unit, mem, key, 
        entry, entry_1, flags, &index);
    
    return rv;

    
}

/* for the condition id to represent the entry static status change */
#define _DRV_ST_OVERRIDE_NO_CHANGE  0
#define _DRV_ST_OVERRIDE_DYN2ST     1
#define _DRV_ST_OVERRIDE_ST2DYN     2
/*
 *  Function : drv_gex_mem_insert
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
 *      Only for ARL memory now.
 *
 */
int 
drv_gex_mem_insert(int unit, uint32 mem, uint32 *entry, 
        uint32 flags)
{
    int         rv = SOC_E_NONE, sw_arl_update = 0;
    int         index, reg_macvid, reg_fwd, reg_len;
    uint8       mac_addr[6];
    uint32      temp, count;
    uint32      vid, control, vid_output, vid_entry;
    uint32      mcast_pbmp, reg32;
    uint64      entry_reg;
    uint64      mac_field, mac_field_output, mac_field_entry;
    l2_arl_sw_entry_t        output, output1;
    int         is_dynamic = FALSE, is_ucast = FALSE, is_override = FALSE;
    int         ori_port = -1, ori_dynamic = TRUE, ori_ucast = FALSE;
    uint32      st_override_status = 0, src_port = 0 ;

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "%s:mem=0x%x,flags=0x%x,entry[0-2]=%08x-%08x-%08x\n"), 
              FUNCTION_NAME(), mem, flags, 
              *(uint32 *)entry, *((uint32 *)entry + 1), *((uint32 *)entry + 2)));
    switch(mem) {
    case DRV_MEM_ARL:
    case DRV_MEM_MARL:
        break;
    case DRV_MEM_VLAN:
    case DRV_MEM_MSTP:
    case DRV_MEM_MCAST:
    case DRV_MEM_GEN:
    case DRV_MEM_SECMAC:
    case DRV_MEM_PROTOCOLVLAN:
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_MACVLAN:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_PARAM;
    }
    MEM_LOCK(unit,INDEX(L2_ARLm));
    /* search entry */
    sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
    if ((rv =  _drv_gex_mem_search
            (unit, DRV_MEM_ARL, entry, (uint32 *)&output, 
            (uint32 *)&output1, flags, &index))< 0) {
        if ((rv != SOC_E_NOT_FOUND) && (rv != SOC_E_EXISTS)) {
            /* FULL condition will be handled to return here */
            goto mem_insert_exit;
        }
    }

    if (rv == SOC_E_EXISTS) {
        /* Return SOC_E_NONE instead of SOC_E_EXISTS to fit DV test. */
        if (!sal_memcmp(&output, entry, 
                sizeof(l2_arl_sw_entry_t)) ){
            rv = SOC_E_NONE;
            goto mem_insert_exit;
        } else {
            /* retrieve SA learn count requirred information in 
             * this section.
             */
            is_override = TRUE;
            if ((rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                    (uint32 *)&output, &temp)) < 0) {
                goto mem_insert_exit;
            }
            ori_dynamic = (temp) ? FALSE : TRUE;

            /* MAC Address */
            if ((rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    (uint32 *)&output, (uint32 *)&mac_field_output)) < 0) {
                goto mem_insert_exit;
            }

            SAL_MAC_ADDR_FROM_UINT64(mac_addr, mac_field_output);
            ori_ucast = (MAC_IS_MCAST(mac_addr)) ? FALSE : TRUE;

            if (ori_ucast){
                /* original port */
                if ((rv = DRV_MEM_FIELD_GET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                        (uint32 *)&output, (uint32 *)&ori_port)) < 0) {
                    goto mem_insert_exit;
                }
            }

            if ((rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    entry, (uint32 *)&mac_field_entry)) < 0) {
                goto mem_insert_exit;
            }
            if (!sal_memcmp(&mac_field_entry, &mac_field_output, 
                    sizeof(mac_field_output)) ){
                /*  VLAN ID  */
                if ((rv = DRV_MEM_FIELD_GET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                        (uint32 *)&output, &vid_output)) < 0) {
                    goto mem_insert_exit;
                }
                if ((rv = DRV_MEM_FIELD_GET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                        entry, &vid_entry)) < 0) {
                    goto mem_insert_exit;
                }
                if (vid_output != vid_entry){
                    rv = SOC_E_NONE;
                    goto mem_insert_exit;
                }                
            } else {
                rv = SOC_E_NONE;
                goto mem_insert_exit;
            }
        }
    }

    /* write entry */
    /* form entry */
    /* VLAN ID */
    if ((rv = DRV_MEM_FIELD_GET(unit, 
            DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &vid)) < 0) {
        goto mem_insert_exit;
    }
    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
        soc_ARLA_MACVID_ENTRY0r_field_set(unit, (uint32 *)&entry_reg,
            VID_Rf, &vid);
    } else {
        soc_ARLA_MACVID_ENTRY0r_field_set(unit, (uint32 *)&entry_reg,
            VIDf, &vid);
    }

    /* MAC Address */
    if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
            entry, (uint32 *)&mac_field)) < 0) {
        goto mem_insert_exit;
    }
    SAL_MAC_ADDR_FROM_UINT64(mac_addr, mac_field);

    soc_ARLA_MACVID_ENTRY0r_field_set(unit, (uint32 *)&entry_reg,
            ARL_MACADDRf, (uint32 *)&mac_field);

    reg32 = 0;
    if (mac_addr[0] & 0x01) { /* The input is the mcast address */
        /* multicast group index */
        if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_MARL, 
                DRV_MEM_FIELD_DEST_BITMAP, entry, &mcast_pbmp)) < 0) {
            goto mem_insert_exit;
        }
        temp = mcast_pbmp;
        if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
            soc_ARLA_FWD_ENTRY0r_field_set(unit, &reg32, 
                PORTID_Rf, &temp);
        } else {
            soc_ARLA_FWD_ENTRY0r_field_set(unit, &reg32, 
                PORTIDf, &temp);
        }
    } else { /* unicast address */
        is_ucast = TRUE;
        /* source port id */
        if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_SRC_PORT, entry, &temp)) < 0) {
            goto mem_insert_exit;
        }
        src_port = temp;
        if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
            soc_ARLA_FWD_ENTRY0r_field_set(unit, &reg32, 
                PORTID_Rf, &temp);
        } else {
            soc_ARLA_FWD_ENTRY0r_field_set(unit, &reg32, 
                PORTIDf, &temp);
        }
    }

    /* age */
    if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
            DRV_MEM_FIELD_AGE, entry, &temp)) < 0) {
        goto mem_insert_exit;
    }
    soc_ARLA_FWD_ENTRY0r_field_set(unit, &reg32, 
                ARL_AGEf, &temp);
    /* priority0 */
    if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
            DRV_MEM_FIELD_PRIORITY, entry, &temp)) < 0) {
        goto mem_insert_exit;
    }
    soc_ARLA_FWD_ENTRY0r_field_set(unit, &reg32, 
                ARL_PRIf, &temp);
    /* ARL_CON */
    if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
            DRV_MEM_FIELD_ARL_CONTROL, entry, &temp)) < 0) {
        goto mem_insert_exit;
    }
    soc_ARLA_FWD_ENTRY0r_field_set(unit, &reg32, 
                ARL_CONf, &temp);            
    /* static :
     *  - ROBO chip arl_control_mode at none-zero value can't work 
     *    without static setting.
     */
    if (temp == 0 ) {
        if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_STATIC, entry, &temp)) < 0) {
            goto mem_insert_exit;
        }
    } else {
        temp = 1;
    }

    is_dynamic = (!temp) ? TRUE : FALSE;
    soc_ARLA_FWD_ENTRY0r_field_set(unit, &reg32, 
                ARL_STATICf, &temp);
    /* valid bit */
    temp = 1;
    soc_ARLA_FWD_ENTRY0r_field_set(unit, &reg32, 
                ARL_VALIDf, &temp);

    if (soc_feature(unit, soc_feature_mac_learn_limit)) {
        /* SA learn counter hander :
        *
        *   1. increase counter for inserting unicast and dynamic entry
        *   2. Check if REPLACE action on original dynalmic entry which the 
        *       entry within different MAC+VID
        *   3. Check if PORT MOVING condition causes counter mismatch.
        *   4. Check if data modify(non MAC+VID+PORT) only and 
        *       a. static is changed : counter must updated.
        *       b. static is not changed : counter must not changed.
        */
        LOG_INFO(BSL_LS_SOC_ARL,
                 (BSL_META_U(unit,
                             "%s,%d, SA learn count must review!\n"),
                  FUNCTION_NAME(), __LINE__));
#if GEX_LEARNED_COUNT_HANDLER
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
#endif  /* GEX_LEARNED_COUNT_HANDLER */
    }

    /* write ARL and MAC/VID entry register*/
    switch(index) {
    case 0:
        reg_macvid = INDEX(ARLA_MACVID_ENTRY0r);
        reg_fwd = INDEX(ARLA_FWD_ENTRY0r);
        break;
    case 1:
        reg_macvid = INDEX(ARLA_MACVID_ENTRY1r);
        reg_fwd = INDEX(ARLA_FWD_ENTRY1r);
        break;
    case 2:
        reg_macvid = INDEX(ARLA_MACVID_ENTRY2r);
        reg_fwd = INDEX(ARLA_FWD_ENTRY2r);
        break;
    case 3:
        reg_macvid = INDEX(ARLA_MACVID_ENTRY3r);
        reg_fwd = INDEX(ARLA_FWD_ENTRY3r);
        break;
    default:
        rv =  SOC_E_PARAM;
        goto mem_insert_exit;
    }
    
    reg_len = DRV_REG_LENGTH_GET(unit, reg_fwd);
    if ((rv = DRV_REG_WRITE(unit, DRV_REG_ADDR(unit, reg_macvid, 0, 0), 
            (uint32 *)&entry_reg, 8)) < 0) {
        goto mem_insert_exit;
    }

    if ((rv = DRV_REG_WRITE(unit, DRV_REG_ADDR(unit, reg_fwd, 0, 0), 
            &reg32, reg_len)) < 0) {
        goto mem_insert_exit;
    }

    /* Write ARL Read/Write Control Register */
    if ((rv = REG_READ_ARLA_RWCTLr(unit, &control)) < 0) {
        goto mem_insert_exit;
    }
    temp = GEX_MEM_OP_WRITE;
    soc_ARLA_RWCTLr_field_set(unit, &control, 
        ARL_RWf, &temp);
    temp = 1;
    soc_ARLA_RWCTLr_field_set(unit, &control, 
        ARL_STRTDNf, &temp);
    if ((rv = REG_WRITE_ARLA_RWCTLr(unit, &control)) < 0) {
        goto mem_insert_exit;
    }

    /* wait for complete */
    for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
        if ((rv = REG_READ_ARLA_RWCTLr(unit, &control)) < 0) {
            goto mem_insert_exit;
        }
        soc_ARLA_RWCTLr_field_get(unit, &control, 
            ARL_STRTDNf, &temp);
        if (!temp)
            break;
    }

    if (count >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto mem_insert_exit;
    }

    sw_arl_update = 1;
    
mem_insert_exit:
    MEM_UNLOCK(unit, INDEX(L2_ARLm));

    if (sw_arl_update){
        /* Add the entry to sw database */
        _drv_arl_database_insert(unit, index, entry);
    }

    return rv;
}

/*
 *  Function : drv_gex_mem_delete
 *
 *  Purpose :
 *      Remove an entry to specific memory or remove entries by flags 
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      entry     :   entry data pointer.
 *      flags     :   delete flags.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Only for ARL memory now.
 *
 */
int 
drv_gex_mem_delete(int unit, uint32 mem, uint32 *entry, uint32 flags)
{
    int         rv = SOC_E_NONE, sw_arl_update = 0;
    l2_arl_sw_entry_t   output, output1;
    uint32      entry_reg;
    uint32      temp, count;
    uint32      control;
    int         index;
    uint32      reg_len, reg_value;
    uint32      ag_port_mode = 0, ag_vlan_mode = 0;
    uint32      ag_static_mode = 0;
    uint32      reg_fwd;
    uint32      src_port= 0, vlanid = 0;
    uint64      temp_mem_data;
    uint8       temp_mac[6];
    int         is_dynamic = FALSE, is_ucast = FALSE;

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_mem_delete : mem=0x%x, flags = 0x%x)\n"),
              mem, flags));
    switch(mem) {
    case DRV_MEM_ARL:
    case DRV_MEM_MARL:
        break;
    case DRV_MEM_VLAN:
    case DRV_MEM_MSTP:
    case DRV_MEM_MCAST:
    case DRV_MEM_GEN:
    case DRV_MEM_SECMAC:
    case DRV_MEM_VLANVLAN:
    case DRV_MEM_PROTOCOLVLAN:
    case DRV_MEM_MACVLAN:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_PARAM;
    }

    if (flags & DRV_MEM_OP_DELETE_BY_PORT) {
        ag_port_mode = 1;
    }
    if (flags & DRV_MEM_OP_DELETE_BY_VLANID) {
        ag_vlan_mode = 1;
    }
    if (flags & DRV_MEM_OP_DELETE_BY_SPT) {
        return SOC_E_UNAVAIL;
    }
    if (flags & DRV_MEM_OP_DELETE_BY_STATIC) {
        ag_static_mode = 1;
    }
    
    if ((ag_port_mode) || (ag_vlan_mode)) {
        if (ag_port_mode) {
        /* aging port mode */
            reg_value = 0;
            if ((rv = REG_WRITE_FAST_AGE_PORTr(unit, &reg_value)) < 0) {
               goto mem_delete_exit;
            }            
            if ((rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp)) < 0) {
               goto mem_delete_exit;
            }            
            src_port = temp;
            soc_FAST_AGE_PORTr_field_set(unit, &reg_value, 
                AGE_PORTf, &temp);

            if ((rv = REG_WRITE_FAST_AGE_PORTr(unit, &reg_value)) < 0) {
               goto mem_delete_exit;
            }
        }  
        if (ag_vlan_mode) {
        /* aging vlan mode */
            temp = 0;
            reg_value = 0;

            if ((rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &temp)) < 0) {
                goto mem_delete_exit;
            }
            vlanid = temp;
            soc_FAST_AGE_VIDr_field_set(unit, &reg_value, 
                AGE_VIDf, &temp);
            if ((rv = REG_WRITE_FAST_AGE_VIDr(unit, &reg_value)) < 0) {
                goto mem_delete_exit;
            }
        }

        /* start fast aging process */
        if ((REG_READ_FAST_AGE_CTRLr(unit, &reg_value)) < 0) {
            goto mem_delete_exit;
        }

        soc_FAST_AGE_CTRLr_field_set(unit, &reg_value, 
            EN_AGE_PORTf, &ag_port_mode);
        soc_FAST_AGE_CTRLr_field_set(unit, &reg_value, 
            EN_AGE_VLANf, &ag_vlan_mode);
        if ((mem == DRV_MEM_MARL) && (!ag_port_mode)) {
            temp = 1;
            soc_FAST_AGE_CTRLr_field_set(unit, &reg_value, 
                EN_AGE_MCASTf, &temp);
        }

        /* Fast aging static and dynamic entries */
        if (ag_static_mode) {
            temp = 1;
            soc_FAST_AGE_CTRLr_field_set(unit, &reg_value, 
                EN_FAST_AGE_STATICf, &temp);
        } else {
            temp = 0;    
            soc_FAST_AGE_CTRLr_field_set(unit, &reg_value, 
                EN_FAST_AGE_STATICf, &temp);
        }
        temp = 1;
        soc_FAST_AGE_CTRLr_field_set(unit, &reg_value, 
                EN_AGE_DYNAMICf, &temp);

        temp = 1;
        soc_FAST_AGE_CTRLr_field_set(unit, &reg_value, 
            FAST_AGE_STR_DONEf, &temp);
        if ((rv = REG_WRITE_FAST_AGE_CTRLr(unit, &reg_value)) < 0) {
            goto mem_delete_exit;
        }
        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = REG_READ_FAST_AGE_CTRLr(unit, &reg_value)) < 0) {
                goto mem_delete_exit;
            }
            soc_FAST_AGE_CTRLr_field_get(unit, &reg_value, 
                FAST_AGE_STR_DONEf, &temp);
            if (!temp) {
                break;
            }
        }
        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_delete_exit;
        }

        /* reset to init status while l2 thaw is proceeded. */
        soc_arl_frozen_sync_init(unit);

        /* Remove entries from software table by port/vlan */
        _drv_arl_database_delete_by_fastage(unit, src_port,
            vlanid, flags);
    } else { /* normal deletion */
        LOG_INFO(BSL_LS_SOC_MEM,
                 (BSL_META_U(unit,
                             "\t Normal ARL DEL with Static=%d\n"),
                  ag_static_mode));

        MEM_LOCK(unit, INDEX(L2_ARLm));
        /* search entry */
        sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
        if ((rv =  _drv_gex_mem_search
            (unit, DRV_MEM_ARL, entry, (uint32 *)&output, 
            (uint32 *)&output1, flags, &index))< 0){
            if((rv != SOC_E_NOT_FOUND) && (rv != SOC_E_EXISTS)) {
                if (rv == SOC_E_FULL) {
                /* For mem_delete if mem_search return SOC_E_FULL it means
                  * the entry is not found. */
                    rv = SOC_E_NOT_FOUND;
                }
                goto mem_delete_exit;              
            }
        }

        /* write entry */
        if (rv == SOC_E_EXISTS) {
            /* clear the VALID bit*/
            temp = 0;
            reg_len = DRV_REG_LENGTH_GET(unit, INDEX(ARLA_FWD_ENTRY0r));
            switch(index) {
                case 0:
                    reg_fwd = INDEX(ARLA_FWD_ENTRY0r);
                    break;
                case 1:
                    reg_fwd = INDEX(ARLA_FWD_ENTRY1r);
                    break;
                case 2:
                    reg_fwd = INDEX(ARLA_FWD_ENTRY2r);
                    break;
                case 3:
                    reg_fwd = INDEX(ARLA_FWD_ENTRY3r);
                    break;
                default:
                    rv =  SOC_E_PARAM;
                    goto mem_delete_exit;
            }
            if ((rv = DRV_REG_READ(unit, DRV_REG_ADDR(unit, reg_fwd, 0, 0), 
                    &entry_reg, reg_len)) < 0){
                goto mem_delete_exit;
            }
            /* check static bit if set */
            if (!ag_static_mode){
                soc_ARLA_FWD_ENTRY0r_field_get(unit, &entry_reg, 
                    ARL_STATICf, &temp);
                if (temp) {
                    LOG_INFO(BSL_LS_SOC_MEM,
                             (BSL_META_U(unit,
                                         "\t Entry exist with static=%d\n"),
                              temp));
                    rv = SOC_E_NOT_FOUND;
                    goto mem_delete_exit;
                }
            }
            
            temp = 0;
            soc_ARLA_FWD_ENTRY0r_field_set(unit, &entry_reg, 
                    ARL_VALIDf, &temp);

            if ((rv = DRV_REG_WRITE(unit, DRV_REG_ADDR(unit, reg_fwd, 0, 0), 
                    &entry_reg, reg_len)) < 0){
                goto mem_delete_exit;
            }

            /* Write ARL Read/Write Control Register */
            if ((rv = REG_READ_ARLA_RWCTLr(unit, &control)) < 0) {
                goto mem_delete_exit;
            }
            temp = GEX_MEM_OP_WRITE;
            soc_ARLA_RWCTLr_field_set(unit, &control,
                ARL_RWf, &temp);
            temp = 1;
            soc_ARLA_RWCTLr_field_set(unit, &control,
                ARL_STRTDNf, &temp);
            if ((rv = REG_WRITE_ARLA_RWCTLr(unit, &control)) < 0) {
                goto mem_delete_exit;
            }

            /* wait for complete */
            for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
                if ((rv = REG_READ_ARLA_RWCTLr(unit, &control)) < 0) {
                    goto mem_delete_exit;
                }
                soc_ARLA_RWCTLr_field_get(unit, &control,
                    ARL_STRTDNf, &temp);
                if (!temp) {
                    break;
                }
            }

            if (count >= SOC_TIMEOUT_VAL) {
                rv = SOC_E_TIMEOUT;
                goto mem_delete_exit;
            }
        
            if (soc_feature(unit, soc_feature_mac_learn_limit)) {
#if GEX_LEARNED_COUNT_HANDLER
                /* SA Learning Count handler :
                *  - decrease one for the ARL deletion process
                *      (for Dynamic and Unicast entry only)
                */
                /* ucast entry check */
                if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_MAC, (uint32 *)&output, 
                        (uint32 *)&temp_mem_data)) < 0) {
                    goto mem_delete_exit;
                }
                
                SAL_MAC_ADDR_FROM_UINT64(temp_mac, temp_mem_data);
                is_ucast = (MAC_IS_MCAST(temp_mac)) ? FALSE : TRUE;
                
                /* static check */
                if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_STATIC, (uint32 *)&output, 
                        &temp)) < 0) {
                    goto mem_delete_exit;
                }
                is_dynamic = (temp) ? FALSE : TRUE;
    
                if (is_dynamic && is_ucast){
                    /* retrieve port_id to increase one in SA_LRN_CNT.port */
                    if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_SRC_PORT, (uint32 *)&output, 
                            &temp)) < 0) {
                        goto mem_delete_exit;
                    }
        
                    rv = DRV_ARL_LEARN_COUNT_SET(unit, temp, 
                            DRV_PORT_SA_LRN_CNT_DECREASE, 0);
                    if (SOC_FAILURE(rv)){
                        goto mem_delete_exit;
                    }
                    LOG_INFO(BSL_LS_SOC_ARL,
                             (BSL_META_U(unit,
                                         "%s,port%d, SA_LRN_CNT decreased one!\n"),
                              FUNCTION_NAME(), temp));
                }
#endif /* GEX_LEARNED_COUNT_HANDLER */
            }

            sw_arl_update = 1;
        }
    }

mem_delete_exit:
    if (((ag_port_mode) || (ag_vlan_mode) )) {
        if(ag_static_mode){
            if ((rv = REG_READ_FAST_AGE_CTRLr(unit, &reg_value)) < 0) {
                return rv;
            }
            temp = 0;
            soc_FAST_AGE_CTRLr_field_set(unit, &reg_value,
                EN_FAST_AGE_STATICf, &temp);
            if ((rv = REG_WRITE_FAST_AGE_CTRLr(unit, &reg_value)) < 0) {
                return rv;
            }
        }
    }else {
        MEM_UNLOCK(unit, INDEX(L2_ARLm));
    }
    /*
     * Restore register value to 0x2, otherwise aging will fail
     */
    reg_value = 0x2;
    REG_WRITE_FAST_AGE_CTRLr(unit, &reg_value);

    if (sw_arl_update){
        /* Remove the entry from sw database */
        _drv_arl_database_delete(unit, index, &output);
    }

    return rv;

    
}
