/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC Memory (Table) Utilities
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>

#include <soc/mem.h>
#include <soc/debug.h>

#include <soc/l2x.h>
#include <soc/cmic.h>
#include <soc/error.h>
#include <soc/arl.h>
#include <soc/register.h>
#include <soc/robo/mcm/driver.h>

#define FIX_MEM_ORDER_E(v,m) (((m)->flags & SOC_MEM_FLAG_BE) ? \
    BYTES2WORDS((m)->bytes)-1-(v) : (v))

#define DINO16_ARL_MEMORY  3

#define DINO16_MEM_OP_WRITE    0
#define DINO16_MEM_OP_READ     1

static int
_drv_dino16_search_valid_op(int unit, uint32 *key, uint32 *entry, 
        uint32 *entry_1, uint32 flags)
{
    int     rv = SOC_E_NONE;
    uint32  count, temp = 0;
    uint32  control = 0;
    uint8   temp_mac_addr[6];
    uint64  temp_mac_field;
    uint64  entry0, entry1;
    uint32  result, result1;
    uint32  vid_rw, valid, valid1;
    int     binNum = -1;
    uint32  reg_value;
    uint32  process, search_valid = 0;
    uint32  mcast_index, src_port;
    int     multicast = 0;

    sal_memset(&temp_mac_field, 0, sizeof(temp_mac_field));

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
        /* Set ARL Search Control register */
        if ((rv = REG_READ_ARLA_SRCH_CTLr(unit, &control)) < 0) {
            return rv;
        }
        binNum = -1;
        process = 1;
        soc_ARLA_SRCH_CTLr_field_set(unit, &control, 
            ARLA_SRCH_STDNf, &process);
        if ((rv = REG_WRITE_ARLA_SRCH_CTLr(unit, &control)) < 0) {
            return rv;
        }
    } else if (flags & DRV_MEM_OP_SEARCH_VALID_GET) {
        if (flags & DRV_MEM_OP_SEARCH_PORT) {
            if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_SRC_PORT, key, &src_port)) < 0) {
                goto mem_search_valid_get;
            }
        }
 
        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            control = 0;
            process = 0;        
            search_valid = 0;
            if ((rv = REG_READ_ARLA_SRCH_CTLr(unit, &control)) < 0) {
               goto mem_search_valid_get;
            }
            soc_ARLA_SRCH_CTLr_field_get(unit, &control, 
                ARLA_SRCH_STDNf, &process);
            soc_ARLA_SRCH_CTLr_field_get(unit, &control, 
                ARLA_SRCH_VLIDf, &search_valid);
            /* ARL search operation was done */
            if (!process) {
                break;
            }
            if (!search_valid) {
                continue;
            }
            count = 0;
            if (!(flags & DRV_MEM_OP_SEARCH_PORT)) {
                /* index value */
                if ((rv = REG_READ_ARLA_SRCH_ADRr(unit, &reg_value)) < 0) {
                    goto mem_search_valid_get;
                }
                soc_ARLA_SRCH_ADRr_field_get(unit, &reg_value, 
                    ARLA_SRCH_ADRf, key);
            }

            /* Read ARL Search Result VID Register */
            vid_rw = 0;
            result = 0;
            result1 = 0;
            if ((rv = REG_READ_ARLA_SRCH_RSLT_MACVID_1r(unit, 
                (uint32 *)&entry1)) < 0) {
                goto mem_search_valid_get;
            }
            if ((rv = REG_READ_ARLA_SRCH_RSLT_1r(unit, &result1)) < 0) {
                 goto mem_search_valid_get;
            }              

            if ((rv = REG_READ_ARLA_SRCH_RSLT_MACVID_0r(unit, 
                (uint32 *)&entry0)) < 0) {
                goto mem_search_valid_get;
            }
            if ((rv = REG_READ_ARLA_SRCH_RSLT_0r(unit, &result)) < 0) {
                goto mem_search_valid_get;
            }

            LOG_INFO(BSL_LS_SOC_ARL,
                     (BSL_META_U(unit,
                                 " key %d result0 %x result1 %x entry %x %x entry 1 %x %x\n"),
                      *key, result, result1, 
                      COMPILER_64_LO(entry0),COMPILER_64_HI(entry0),
                      COMPILER_64_LO(entry1),COMPILER_64_HI(entry1)));

            valid = 0;
            /* ENTRY 0 */
            soc_ARLA_SRCH_RSLT_0r_field_get(unit, &result, 
                ARL_VALIDf, &valid);

            if ((valid == 1) && (!COMPILER_64_IS_ZERO(entry0))) {
                binNum = 0;
                temp = 1;
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_VALID, entry, &temp)) < 0) {
                    goto mem_search_valid_get;
                }
                /* MAC Address */
                soc_ARLA_SRCH_RSLT_MACVID_0r_field_get(unit, (uint32 *)&entry0, 
                    ARLA_SRCH_MACADDRf, (uint32 *)&temp_mac_field);
                /* VLAN ID */
                soc_ARLA_SRCH_RSLT_MACVID_0r_field_get(unit, (uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_VIDf, &vid_rw);

                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);

                if (temp_mac_addr[0] & 0x01) {
                    multicast = 1;
                } 
                if (flags & DRV_MEM_OP_SEARCH_PORT) {
                    if (multicast) {
                        continue;
                    }
                    soc_ARLA_SRCH_RSLT_0r_field_get(unit, &result, 
                        PORTID_Rf, &temp);
                    if (temp != src_port) {
                        continue;
                    }
                }
                soc_ARLA_SRCH_RSLT_MACVID_0r_field_get(unit,(uint32 *)&entry0, 
                    ARLA_SRCH_RSLT_VIDf, &vid_rw);
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_VLANID, entry, &vid_rw)) < 0) {
                    goto mem_search_valid_get;
                }
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_MAC, entry, 
                    (uint32 *)&temp_mac_field)) < 0) {
                    goto mem_search_valid_get;
                }
                if (multicast) { /* mcast address */
                    mcast_index = 0;
                    soc_MARLA_SRCH_RSLT_0r_field_get(unit, &result, 
                        FWD_PRT_MAPf, &mcast_index);
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                        DRV_MEM_FIELD_DEST_BITMAP, entry, 
                        (uint32 *)&mcast_index)) < 0) {
                        goto mem_search_valid_get;
                    }
                    soc_MARLA_SRCH_RSLT_0r_field_get(unit, &result, 
                        ARL_AGEf, &temp);
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                        DRV_MEM_FIELD_AGE, entry, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }
                    soc_MARLA_SRCH_RSLT_0r_field_get(unit, &result, 
                        ARL_PRIf, &temp);
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                        DRV_MEM_FIELD_PRIORITY, entry, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }
                } else { /* unicast address */
                    /* Source Port Number */
                    soc_ARLA_SRCH_RSLT_0r_field_get(unit, &result, 
                        PORTID_Rf, &temp);
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_SRC_PORT, entry, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }
                    /* Priority Queue */
                    soc_ARLA_SRCH_RSLT_0r_field_get(unit, &result, 
                        ARL_PRIf, &temp);
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_PRIORITY, entry, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }
                    /* Static Bit */
                    soc_ARLA_SRCH_RSLT_0r_field_get(unit, &result, 
                        ARL_STATICf, &temp);
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_STATIC, entry, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }
                    /* Hit bit */
                    soc_ARLA_SRCH_RSLT_0r_field_get(unit, &result, 
                        ARL_AGEf, &temp);
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_AGE, entry, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }
                }
            }

            valid1 = 0;
            /* ENTRY 1 */
            soc_ARLA_SRCH_RSLT_1r_field_get(unit, &result1, 
                ARL_VALIDf, &valid1);

            if ((valid1 == 1) && (!COMPILER_64_IS_ZERO(entry1))) {
                binNum = 1;
                temp = 1;
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_VALID, entry_1, &temp)) < 0) {
                    goto mem_search_valid_get;
                }
                /* MAC Address */
                soc_ARLA_SRCH_RSLT_MACVID_1r_field_get(unit, (uint32 *)&entry1, 
                    ARLA_SRCH_MACADDRf, (uint32 *)&temp_mac_field);
                /* VLAN ID */
                soc_ARLA_SRCH_RSLT_MACVID_1r_field_get(unit, (uint32 *)&entry1, 
                    ARLA_SRCH_RSLT_VIDf, &vid_rw);

                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                    
                if (temp_mac_addr[0] & 0x01) {
                    multicast = 1;
                } 
                if (flags & DRV_MEM_OP_SEARCH_PORT) {
                    if (multicast) {
                        continue;
                    }
                    soc_ARLA_SRCH_RSLT_1r_field_get(unit, &result1, 
                        PORTID_Rf, &temp);
                    if (temp != src_port) {
                        continue;
                    }
                }
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_VLANID, entry_1, &vid_rw)) < 0) {
                    goto mem_search_valid_get;
                }
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_MAC, entry_1, 
                    (uint32 *)&temp_mac_field)) < 0) {
                    goto mem_search_valid_get;
                }
                if (multicast) { /* mcast address */
                    mcast_index = 0;
                    soc_MARLA_SRCH_RSLT_1r_field_get(unit, &result1, 
                        FWD_PRT_MAPf, &mcast_index);
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                        DRV_MEM_FIELD_DEST_BITMAP, entry_1, 
                        (uint32 *)&mcast_index)) < 0) {
                        goto mem_search_valid_get;
                    }
                    soc_MARLA_SRCH_RSLT_1r_field_get(unit, &result1, 
                        ARL_AGEf, &temp);
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                        DRV_MEM_FIELD_AGE, entry_1, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }
                    soc_MARLA_SRCH_RSLT_1r_field_get(unit, &result1, 
                        ARL_PRIf, &temp);
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                        DRV_MEM_FIELD_PRIORITY, entry_1, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }
                } else { /* unicast address */
                    /* Source Port Number */
                    soc_ARLA_SRCH_RSLT_1r_field_get(unit, &result1, 
                        PORTID_Rf, &temp);
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_SRC_PORT, entry_1, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }
                    /* Priority Queue */
                    soc_ARLA_SRCH_RSLT_1r_field_get(unit, &result1, 
                        ARL_PRIf, &temp);
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_PRIORITY, entry_1, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }
                    /* Static Bit */
                    soc_ARLA_SRCH_RSLT_1r_field_get(unit, &result1, 
                        ARL_STATICf, &temp);
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_STATIC, entry_1, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }
                    /* Hit bit */
                    soc_ARLA_SRCH_RSLT_1r_field_get(unit, &result1, 
                        ARL_AGEf, &temp);
                    if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_AGE, entry_1, &temp)) < 0) {
                        goto mem_search_valid_get;
                    }
                }
            } 
            if (binNum == -1) {
                rv = SOC_E_NOT_FOUND;
                goto mem_search_valid_get;
            }
            rv = SOC_E_EXISTS;
            goto mem_search_valid_get;
        }
        rv = SOC_E_TIMEOUT;
mem_search_valid_get:
        return rv;
    }
    return rv;
}

/*
 *  Function : _drv_dino16_mem_search
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
 *      index   :   entry index in this memory bank (0 or 1).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. bcm5396 has 2 banks(bins) in each ARL hash bucket.
 *  2. DRV_MEM_OP_SEARCH_CONFLICT
 *      a. will be performed in the MAC+VID hash search section.
 *      b. (*entry_1) not be used but (entry) will be used as l2_entry array
 *          and this l2_entry array size should not smaller than ARL table  
 *          hash bucket size(bin number).
 *      c. all valid entries in the MAC+VID hash bucket will be reported.
 *
 */
int 
_drv_dino16_mem_search(int unit, uint32 mem, uint32 *key, 
        uint32 *entry, uint32 *entry_1, uint32 flags, int *index)
{
    int     rv = SOC_E_NONE;
    soc_control_t  *soc = SOC_CONTROL(unit);
    uint32  count, temp;
    uint32  control = 0;
    uint8   mac_addr_rw[6], temp_mac_addr[6];
    uint64  rw_mac_field, temp_mac_field;
    uint64  entry0, entry1, *mac_vid;
    uint32  result, result1, *input;
    uint32  vid_rw, vid1 = 0, vid2 = 0;
    int     binNum = -1, existed = 0;
    uint32  reg_value;
    l2_arl_entry_t  l2_arl_entry;

    COMPILER_64_ZERO(temp_mac_field);

    if ((flags & DRV_MEM_OP_SEARCH_DONE) ||
        (flags & DRV_MEM_OP_SEARCH_VALID_START) ||
        (flags & DRV_MEM_OP_SEARCH_VALID_GET)) {
        rv = _drv_dino16_search_valid_op(unit, key, entry, entry_1, flags);
        return rv;
    } else if (flags & DRV_MEM_OP_BY_INDEX) {
        /* The entry index is inside the input parameter "key". */ 
        /* Read the memory raw data */
        SOC_IF_ERROR_RETURN(DRV_MEM_READ
            (unit, mem, *key, 1, (uint32 *)&l2_arl_entry));
        /* Read VALID bit */
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
            (uint32 *)&l2_arl_entry, &temp));
        if (!temp) {
            return rv;
        }
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, entry, &temp));

        /* Read VLAN ID value */
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
            (uint32 *)&l2_arl_entry, &vid_rw));
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &vid_rw));
        
        /* Read MAC address bit 12~47 */
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
            (uint32 *)&l2_arl_entry, (uint32 *)&rw_mac_field));
        SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, rw_mac_field);

        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
             entry, (uint32 *)&temp_mac_field));
        if (temp_mac_addr[0] & 0x01) { /* multicast entry */
            /* Multicast index */
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                (uint32 *)&l2_arl_entry, &temp));
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                 entry, &temp));
             /* Age bit */
			 SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                 (unit, DRV_MEM_MARL, DRV_MEM_FIELD_AGE, 
                 (uint32 *)&l2_arl_entry, &temp));
			 SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                 (unit, DRV_MEM_MARL, DRV_MEM_FIELD_AGE, 
                  entry, &temp));
        } else { /* unicast entry */
            /* Source port number */
			SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                (uint32 *)&l2_arl_entry, &temp));
			SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                 entry, &temp));
            /* Priority queue value */
			SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
                (uint32 *)&l2_arl_entry, &temp));
			SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
                 entry, &temp));
            /* Age bit */
			SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
                (uint32 *)&l2_arl_entry, &temp));
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
                 entry, &temp));
        }
        /* Static bit */
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
            (uint32 *)&l2_arl_entry, &temp));
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
             entry, &temp));
        
        return rv;

    /* delete by MAC */    
    } else if (flags & DRV_MEM_OP_BY_HASH_BY_MAC) {    
        l2_arl_sw_entry_t   *rep_entry;
        int i, is_conflict[ROBO_DINO_L2_BUCKET_SIZE];

        if (flags & DRV_MEM_OP_SEARCH_CONFLICT) {
            if (entry == NULL) {
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s,entries buffer not allocated!\n"), FUNCTION_NAME()));
                return SOC_E_PARAM;
            }

            for (i = 0; i < ROBO_DINO_L2_BUCKET_SIZE; i++) {
                /* check the parameter for output entry buffer */
                rep_entry = (l2_arl_sw_entry_t *)entry + i;
                sal_memset(rep_entry, 0, sizeof(l2_arl_sw_entry_t));
                is_conflict[i] = FALSE;
            }
        }
        
        MEM_LOCK(unit, INDEX(L2_ARLm));
        ARL_MEM_SEARCH_LOCK(soc);
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
		
        if (temp == 3) {     /* VLAN is at IVL mode */
            if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                temp = 3;
            } else {
                temp = 0;
            }

            soc_VLAN_CTRL0r_field_set(unit, &reg_value, 
                VLAN_LEARN_MODEf, &temp);
        }
        if ((rv = REG_WRITE_VLAN_CTRL0r(unit,&reg_value)) < 0) {
            goto mem_search_exit;
        }

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
                DRV_MEM_FIELD_VLANID, key, &vid_rw)) < 0) {
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

        /* Clear the ARL Entry 0/1 Register */
        sal_memset(&result, 0, sizeof(result));
        sal_memset(&result1, 0, sizeof(result1));
        if ((rv = REG_WRITE_ARLA_FWD_ENTRY0r(unit, &result)) < 0) {
            goto mem_search_exit;
        }
        if ((rv = REG_WRITE_ARLA_FWD_ENTRY1r(unit, &result1)) < 0) {
            goto mem_search_exit;
        }

        temp = DINO16_MEM_OP_READ;
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
            if (!temp) {
                break;
            }
        }

        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_search_exit;
        }

        /* Read Operation sucessfully */
        /* Get the ARL Entry 0/1 Register */
        if ((rv = REG_READ_ARLA_FWD_ENTRY0r(unit, &result)) < 0) {
            goto mem_search_exit;
        }
        if ((rv = REG_READ_ARLA_FWD_ENTRY1r(unit, &result1)) < 0) {
            goto mem_search_exit;
        }

        /* check DRV_MEM_OP_REPLACE */
        if (flags & DRV_MEM_OP_REPLACE) {
            uint32 temp_valid1 = 0;
            uint32 temp_valid2 = 0;
            uint32 temp_static1 = 0;
            uint32 temp_static2 = 0;

            LOG_INFO(BSL_LS_SOC_ARL,
                     (BSL_META_U(unit,
                                 "DRV_MEM_OP_REPLACE\n")));

            if (flags & DRV_MEM_OP_SEARCH_CONFLICT) {
                LOG_INFO(BSL_LS_SOC_ARL,
                         (BSL_META_U(unit,
                                     "%s, Conflict search will not be performed in REPLACE!\n"),
                          FUNCTION_NAME()));
            }

            /* Check the ARL Entry 0 Register */
            soc_ARLA_FWD_ENTRY0r_field_get(unit, (uint32 *)&result, 
                ARL_VALIDf, &temp_valid1);

            /* Check the ARL Entry 0 Register if Static*/
            soc_ARLA_FWD_ENTRY0r_field_get(unit, (uint32 *)&result, 
                ARL_STATICf, &temp_static1);

            /* Check the ARL Entry 1 Register */
            soc_ARLA_FWD_ENTRY1r_field_get(unit, (uint32 *)&result1, 
                ARL_VALIDf, &temp_valid2);

            /* Check the ARL Entry 1 Register if Static*/
            soc_ARLA_FWD_ENTRY1r_field_get(unit, (uint32 *)&result1, 
                ARL_STATICf, &temp_static2);

            if (temp_valid1) {
                /* bin 0 valid, check mac or mac+vid */
                if ((rv = REG_READ_ARLA_MACVID_ENTRY0r
                        (unit, (uint32 *)&entry0)) < 0) {
                        goto mem_search_exit;
                }
                COMPILER_64_ZERO(temp_mac_field);
                soc_ARLA_MACVID_ENTRY0r_field_get(unit, (uint32 *)&entry0, 
                    ARL_MACADDRf, (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                soc_ARLA_MACVID_ENTRY0r_field_get(unit, (uint32 *)&entry0, 
                    VID_Rf, &vid1);
                    
                /* check if we have to overwrite this valid bin0 */
                if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                    /* check mac + vid */
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6) && 
                        (vid1 == vid_rw)) {
                        /* select bin 0 to overwrite it */
                        binNum = 0;
                    }
                } else {
                    /* check mac */
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6)) {
                        /* select bin 0 to overwrite it */              
                        binNum = 0;
                    }                
                } 
            } 

            if (temp_valid2) {
                /* bin 0 valid, check mac or mac+vid */
                if ((rv = REG_READ_ARLA_MACVID_ENTRY1r
                        (unit, (uint32 *)&entry1)) < 0) {
                    goto mem_search_exit;
                }
                COMPILER_64_ZERO(temp_mac_field);
                soc_ARLA_MACVID_ENTRY1r_field_get(unit, (uint32 *)&entry1, 
                    ARL_MACADDRf, (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                soc_ARLA_MACVID_ENTRY1r_field_get(unit, (uint32 *)&entry1, 
                    VID_Rf, &vid2);
                    
                /* check if we have to overwrite this valid bin1 */
                if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                    /* check mac + vid */
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6) && 
                        (vid2 == vid_rw)) {
                        /* select bin 1 to overwrite it */
                        binNum = 1;
                    }
                } else {
                    /* check mac */
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6)) {
                        /* select bin 1 to overwrite it */               
                        binNum = 1;
                    }                
                } 
            } 

            /* can not find a entry to overwrite based on same mac + vid */
            if (binNum == -1) {
                if (temp_valid1 == 0) {
                    binNum = 0;
                } else if (temp_valid2 == 0) {
                    binNum = 1;
                } else {
                    /* both valid, pick non-static one */
                    if (temp_static1 == 0) {
                        binNum = 0;
                    } else if (temp_static2 == 0) {
                        binNum = 1;
                    } else {
                        /* table full */
                        rv = SOC_E_FULL;
                        goto mem_search_exit;                    
                    }
                }
            }
        /* Not DRV_MEM_OP_REPLACE */
        } else {
            /* Check the ARL Entry 0 Register */
            soc_ARLA_FWD_ENTRY0r_field_get(unit, &result, 
                ARL_VALIDf, &temp);
            if (temp) {
                if (flags & DRV_MEM_OP_SEARCH_CONFLICT) {
                    is_conflict[0] = TRUE;
                }

                /* this entry if valid, check to see if this is the MAC */
                if ((rv = REG_READ_ARLA_MACVID_ENTRY0r
                        (unit, (uint32 *)&entry0)) < 0) {
                    goto mem_search_exit;
                }
                COMPILER_64_ZERO(temp_mac_field);
                soc_ARLA_MACVID_ENTRY0r_field_get(unit, (uint32 *)&entry0, 
                    ARL_MACADDRf, (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                soc_ARLA_MACVID_ENTRY0r_field_get(unit, (uint32 *)&entry0, 
                    VID_Rf, &vid1);
                if (!memcmp(temp_mac_addr, mac_addr_rw, 6)) {
                    if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                        if (vid1 == vid_rw) {
                            binNum = 0;
                            if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                                existed = 1;
                            }
                        }
                    } else {
                        binNum = 0;
                        if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                            existed = 1;
                        }
                    }
                }
            } else {
                binNum = 0;
            }

            /* Check the ARL Entry 1 Register */
            soc_ARLA_FWD_ENTRY1r_field_get(unit, &result1, 
                ARL_VALIDf, &temp);

            if (temp) {
                if (flags & DRV_MEM_OP_SEARCH_CONFLICT) {
                    binNum = 1;
                    is_conflict[1] = TRUE;
                }

                /* this entry if valid, check to see if this is the MAC */
                if ((rv = REG_READ_ARLA_MACVID_ENTRY1r
                        (unit, (uint32 *)&entry1)) < 0) {
                        goto mem_search_exit;
                }
                COMPILER_64_ZERO(temp_mac_field);
                soc_ARLA_MACVID_ENTRY1r_field_get(unit, (uint32 *)&entry1, 
                    ARL_MACADDRf, (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                soc_ARLA_MACVID_ENTRY1r_field_get(unit, (uint32 *)&entry1, 
                    VID_Rf, &vid2);
                if (!memcmp(temp_mac_addr, mac_addr_rw, 6)) {
                    if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                        if (vid2 == vid_rw) {
                            binNum = 1;
                            if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                                existed = 1;
                            }
                        }
                    } else {
                        binNum = 1;
                        if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                            existed = 1;
                        }
                    }
                }
            } else {
                if (binNum == -1) binNum = 1;
            }

            if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)) {
                /* if no entry found, fail */
                if (binNum == -1) {
                    rv = SOC_E_FULL;
                    goto mem_search_exit;
                }
            }
        }

        for (i = 0; i < ROBO_DINO_L2_BUCKET_SIZE; i++) {
            if (flags & DRV_MEM_OP_SEARCH_CONFLICT) {
                if (is_conflict[i] == FALSE) {
                    continue;
                }
                existed = 1;
                rep_entry = (l2_arl_sw_entry_t *)entry + i;
            } else {
                /* match basis search */
                if (i != binNum) {
                    continue;
                }
                rep_entry = (l2_arl_sw_entry_t *)entry;
            }

            /* assign the processing hw arl entry */
            if (i == 0) {
                mac_vid = &entry0;
                input = &result;
                vid_rw = vid1;
            } else {
                mac_vid = &entry1;
                input = &result1;
                vid_rw = vid2;
            }
            *index = i;

            /* Only need write the selected Entry Register */
            COMPILER_64_ZERO(temp_mac_field);
            soc_ARLA_MACVID_ENTRY0r_field_get(unit, (uint32 *)mac_vid, 
                ARL_MACADDRf, (uint32 *)&temp_mac_field);
            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
            if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                (uint32 *)rep_entry, (uint32 *)&temp_mac_field)) < 0) {
                goto mem_search_exit;
            }

            if (temp_mac_addr[0] & 0x01) { /* The input is the mcast address */
                soc_MARLA_FWD_ENTRY0r_field_get(unit, input, 
                    FWD_PRT_MAPf, &temp);
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                        DRV_MEM_FIELD_DEST_BITMAP, (uint32 *)rep_entry, 
                        &temp)) < 0) {
                    goto mem_search_exit;
                }

            } else { /* The input is the unicast address */
                soc_ARLA_FWD_ENTRY0r_field_get(unit, (uint32 *)input, 
                    PORTID_Rf, &temp);
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_SRC_PORT, (uint32 *)rep_entry, 
                        &temp)) < 0) {
                    goto mem_search_exit;
                }

                soc_ARLA_FWD_ENTRY0r_field_get(unit, (uint32 *)input, 
                    ARL_STATICf, &temp);
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_STATIC, (uint32 *)rep_entry, 
                        &temp)) < 0) {
                    goto mem_search_exit;
                }
            }

            if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_VLANID, (uint32 *)rep_entry, 
                    &vid_rw)) < 0) {
                goto mem_search_exit;
            }
            temp = 1;
            if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_VALID, (uint32 *)rep_entry, 
                    &temp)) < 0) {
                goto mem_search_exit;
            }
            soc_ARLA_FWD_ENTRY0r_field_get(unit, (uint32 *)input, 
                ARL_PRIf, &temp);
            if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_PRIORITY, (uint32 *)rep_entry, 
                    &temp)) < 0) {
                goto mem_search_exit;
            }
    
            soc_ARLA_FWD_ENTRY0r_field_get(unit, (uint32 *)input, 
                ARL_AGEf, &temp);
            if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_AGE, (uint32 *)rep_entry, 
                    &temp)) < 0) {
                goto mem_search_exit;
            }
        }


        if (flags & DRV_MEM_OP_REPLACE) {
            rv = SOC_E_NONE;
        } else {
            if (existed) {
                rv = SOC_E_EXISTS;    
            } else {
                rv = SOC_E_NOT_FOUND;
            }
        }

mem_search_exit:
        ARL_MEM_SEARCH_UNLOCK(soc);
        MEM_UNLOCK(unit, INDEX(L2_ARLm));
        return rv;
            
    } else {
        return SOC_E_PARAM;
    }

}
/*
 *  Function : _drv_dino16_vlan_mem_read
 *
 *  Purpose :
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry_id    :  the index of entry in the memory to be read.
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
_drv_dino16_vlan_mem_read(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int  rv = SOC_E_NONE;
    int  i, index;
    uint32  retry;
    uint32  temp, reg_value;
    uint64  reg_v64;
    vlan_1q_entry_t  *vlan_mem = 0;

    vlan_mem = (vlan_1q_entry_t *)entry;

    for (i = 0; i < count; i++) {        
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

        temp = DINO16_MEM_OP_READ;
        soc_ARLA_VTBL_RWCTRLr_field_set(unit, &reg_value, 
            ARLA_VTBL_RWf, &temp);

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
        if ((rv = REG_READ_ARLA_VTBL_ENTRYr(unit, (uint32 *)&reg_v64)) < 0) {
            goto mem_read_exit;
        }

        soc_ARLA_VTBL_ENTRYr_field_get(unit, (uint32 *)&reg_v64, 
            VALID_Rf, &temp);
        if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_VLAN, 
            DRV_MEM_FIELD_VALID, (uint32 *)vlan_mem, &temp)) < 0) {
            goto mem_read_exit;
        }

        soc_ARLA_VTBL_ENTRYr_field_get(unit, (uint32 *)&reg_v64, 
            MSPD_IDf, &temp);
        if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_VLAN, 
            DRV_MEM_FIELD_SPT_GROUP_ID, (uint32 *)vlan_mem, &temp)) < 0) {
            goto mem_read_exit;
        }

        soc_ARLA_VTBL_ENTRYr_field_get(unit, (uint32 *)&reg_v64, 
            FWD_MAPf, &temp);
        if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_VLAN, 
            DRV_MEM_FIELD_PORT_BITMAP, (uint32 *)vlan_mem, &temp)) < 0) {
            goto mem_read_exit;
        }

        soc_ARLA_VTBL_ENTRYr_field_get(unit, (uint32 *)&reg_v64, 
            UNTAG_MAPf, &temp);
        if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_VLAN, 
            DRV_MEM_FIELD_OUTPUT_UNTAG, (uint32 *)vlan_mem, &temp)) < 0) {
            goto mem_read_exit;
        }

        vlan_mem++;
    }

mem_read_exit:  
    return rv;
}

/*
 *  Function : _drv_dino16_vlan_mem_write
 *
 *  Purpose :
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry_id    :  the index of entry in the memory to be read.
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
int
_drv_dino16_vlan_mem_write(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int  rv = SOC_E_NONE;
    int  i, index;
    uint32  reg_value;
    uint64  reg_v64;
    uint32  retry, temp;
    vlan_1q_entry_t  *vlan_mem = 0;

    vlan_mem = (vlan_1q_entry_t *)entry;

    for (i = 0; i < count; i++) {
        index = entry_id + i;

        COMPILER_64_ZERO(reg_v64);
        if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_VLAN, 
            DRV_MEM_FIELD_VALID, (uint32 *)vlan_mem, &temp)) < 0) {
            goto mem_write_exit;
        }
        soc_ARLA_VTBL_ENTRYr_field_set(unit, (uint32 *)&reg_v64, 
            VALID_Rf, &temp);

        if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_VLAN, 
            DRV_MEM_FIELD_SPT_GROUP_ID, (uint32 *)vlan_mem, &temp)) < 0) {
            goto mem_write_exit;
        }
        soc_ARLA_VTBL_ENTRYr_field_set(unit, (uint32 *)&reg_v64, 
            MSPD_IDf, &temp);

        if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_VLAN, 
            DRV_MEM_FIELD_PORT_BITMAP, (uint32 *)vlan_mem, &temp)) < 0) {
            goto mem_write_exit;
        }
        soc_ARLA_VTBL_ENTRYr_field_set(unit, (uint32 *)&reg_v64, 
            FWD_MAPf, &temp);

        if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_VLAN, 
            DRV_MEM_FIELD_OUTPUT_UNTAG, (uint32 *)vlan_mem, &temp)) < 0) {
            goto mem_write_exit;
        }
        soc_ARLA_VTBL_ENTRYr_field_set(unit, (uint32 *)&reg_v64, 
            UNTAG_MAPf, &temp);

        if ((rv = REG_WRITE_ARLA_VTBL_ENTRYr(unit, (uint32 *)&reg_v64)) < 0) {
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

        temp = DINO16_MEM_OP_WRITE;
        soc_ARLA_VTBL_RWCTRLr_field_set(unit, &reg_value, 
            ARLA_VTBL_RWf, &temp);

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
 *  Function : _drv_dino16_mstp_mem_read
 *
 *  Purpose :
 *      Get the width of mstp memory. 
 *      The value returned in data is width in bits.
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry_id    :  the index of entry in the memory to be read.
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
_drv_dino16_mstp_mem_read(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int  rv = SOC_E_NONE;
    int  i, index;

    for (i = 0; i < count; i++) {        
        index = entry_id + i;
        if ((rv = REG_READ_MST_TBLr(unit, index, entry)) < 0) {
            goto mem_read_exit;
        }
        entry++;
    }

mem_read_exit:  
    return rv;
}

/*
 *  Function : _drv_dino16_mstp_mem_write
 *
 *  Purpose :
 *      Get the width of mstp memory. 
 *      The value returned in data is width in bits.
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry_id    :  the index of entry in the memory to be read.
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
int
_drv_dino16_mstp_mem_write(int unit,
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int  rv = SOC_E_NONE;
    int  i, index;

    for (i = 0;i < count; i++) {
        index = entry_id + i;
        if ((rv = REG_WRITE_MST_TBLr(unit, index, entry)) < 0) {
            goto mem_write_exit;
        }
        entry++;
    }

mem_write_exit:     
    return rv;
} 

static int 
_drv_dino16_mem_arl_entry_delete(int unit, uint8 *mac_addr, 
    uint32 vid, int static_entry)
{
    uint32  reg_val32, count, temp;
    uint32  result0, result1;
    uint64  entry0, entry1;
    uint8  temp_mac_addr[6];
    uint64  temp_mac_field;
    uint64  reg_val64, mac_field;
    uint32  vid0 = 0, vid1 = 0;
    int  binNum = -1;
    int  rv = SOC_E_NONE;
    uint32  temp_valid0 = 0, temp_valid1 = 0;
    uint32  temp_static0 = 0, temp_static1 = 0;

    COMPILER_64_ZERO(temp_mac_field);
    MEM_LOCK(unit, INDEX(L2_ARLm));

    /* write MAC Addr */
    if ((rv = REG_READ_ARLA_MACr(unit, (uint32 *)&reg_val64)) < 0) {
        goto arl_entry_delete_exit;
    }
    SAL_MAC_ADDR_TO_UINT64(mac_addr, mac_field);
    soc_ARLA_MACr_field_set(unit, (uint32 *)&reg_val64, 
        MAC_ADDR_INDXf, (uint32 *)&mac_field);
    if ((rv = REG_WRITE_ARLA_MACr(unit, (uint32 *)&reg_val64)) < 0) {
        goto arl_entry_delete_exit;
    }

    /* write VID */
    if ((rv = REG_READ_ARLA_VIDr(unit, &reg_val32)) < 0) {
        goto arl_entry_delete_exit;
    }
    soc_ARLA_VIDr_field_set(unit, &reg_val32, 
        ARLA_VIDTAB_INDXf, &vid);
    if ((rv = REG_WRITE_ARLA_VIDr(unit, &reg_val32)) < 0) {
        goto arl_entry_delete_exit;
    }        
        
    /* Write ARL Read/Write Control Register */
    reg_val32 = 0;

    /* Read Operation */
    temp = DINO16_MEM_OP_READ;
    soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
        ARL_RWf, &temp);

    temp = 1;
    soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
        ARL_STRTDNf, &temp);
    if ((rv = REG_WRITE_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
        goto arl_entry_delete_exit;
    }

    /* wait for complete */
    for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
        if ((rv = REG_READ_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
            goto arl_entry_delete_exit;
        }
        soc_ARLA_RWCTLr_field_get(unit, &reg_val32, 
            ARL_STRTDNf, &temp);
        if (!temp) {
            break;
        }
    }

    if (count >= SOC_TIMEOUT_VAL) {
         rv = SOC_E_TIMEOUT;
        goto arl_entry_delete_exit;
    }

    /* Read Operation sucessfully */
    /* Get the ARL Entry 0/1 Register */
    if ((rv = REG_READ_ARLA_FWD_ENTRY0r(unit, &result0)) < 0) {
        goto arl_entry_delete_exit;
    }
    if ((rv = REG_READ_ARLA_FWD_ENTRY1r(unit, &result1)) < 0) {
        goto arl_entry_delete_exit;
    }
         
    /* Check the ARL Entry 0 Register */
    soc_ARLA_FWD_ENTRY0r_field_get(unit, (uint32 *)&result0, 
        ARL_VALIDf, &temp_valid0);

    /* Check the ARL Entry 0 Register if Static*/
    soc_ARLA_FWD_ENTRY0r_field_get(unit, (uint32 *)&result0, 
        ARL_STATICf, &temp_static0);

    /* Check the ARL Entry 1 Register */
    soc_ARLA_FWD_ENTRY1r_field_get(unit, (uint32 *)&result1, 
        ARL_VALIDf, &temp_valid1);

    /* Check the ARL Entry 1 Register if Static*/
    soc_ARLA_FWD_ENTRY1r_field_get(unit, (uint32 *)&result1, 
        ARL_STATICf, &temp_static1);

    if (temp_valid0) {
        /* bin 0 valid, check mac or mac+vid */
        if ((rv = REG_READ_ARLA_MACVID_ENTRY0r(unit, &entry0)) < 0) {
                goto arl_entry_delete_exit;
        }
        COMPILER_64_ZERO(temp_mac_field);
        soc_ARLA_MACVID_ENTRY0r_field_get(unit, (uint32 *)&entry0, 
            ARL_MACADDRf, (uint32 *)&temp_mac_field);

        SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
        soc_ARLA_MACVID_ENTRY0r_field_get(unit, (uint32 *)&entry0, 
            VID_Rf, &vid0);
            
        /* check mac + vid */
        if (!memcmp(temp_mac_addr, mac_addr, 6) && (vid0 == vid)) {
            /* select bin 0 to overwrite it */
            binNum = 0;
        }
    }

    if (binNum == 0) {
        if (!((static_entry == 0) && (temp_static0 != static_entry))) {
            /* clear this entry*/                            
            temp = 0;
            soc_ARLA_FWD_ENTRY0r_field_set(unit, &result0, 
                ARL_VALIDf, &temp);

            if ((rv = REG_WRITE_ARLA_FWD_ENTRY0r(unit, &result0)) < 0) {
                goto arl_entry_delete_exit;
            }

            /* Write ARL Read/Write Control Register */
            reg_val32 = 0;
            /* Write Operation */
            temp = DINO16_MEM_OP_WRITE;
            soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
                ARL_RWf, &temp);

            temp = 1;
            soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
                ARL_STRTDNf, &temp);
            if ((rv = REG_WRITE_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
                goto arl_entry_delete_exit;
            }

            /* wait for complete */
            for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
                if ((rv = REG_READ_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
                    goto arl_entry_delete_exit;
                }
                soc_ARLA_RWCTLr_field_get(unit, &reg_val32, 
                    ARL_STRTDNf, &temp);
                if (!temp) {
                    break;
                }
            }

            if (count >= SOC_TIMEOUT_VAL) {
                rv =  SOC_E_TIMEOUT;
                goto arl_entry_delete_exit;
            }
        }
        goto arl_entry_delete_exit;
    }

    /* Check the ARL Entry 1 Register */
    soc_ARLA_FWD_ENTRY1r_field_get(unit, (uint32 *)&result1, 
        ARL_VALIDf, &temp_valid1);

    /* Check the ARL Entry 1 Register if Static*/
    soc_ARLA_FWD_ENTRY1r_field_get(unit, (uint32 *)&result1, 
        ARL_STATICf, &temp_static1);

    if (temp_valid1) {
        /* bin 1 valid, check mac or mac+vid */
        if ((rv = REG_READ_ARLA_MACVID_ENTRY1r(unit, &entry1)) < 0) {
                goto arl_entry_delete_exit;
        }
        COMPILER_64_ZERO(temp_mac_field);
        soc_ARLA_MACVID_ENTRY1r_field_get(unit, (uint32 *)&entry1, 
            ARL_MACADDRf, (uint32 *)&temp_mac_field);

        SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
        soc_ARLA_MACVID_ENTRY1r_field_get(unit, (uint32 *)&entry1, 
            VID_Rf, &vid1);

        /* check mac + vid */
        if (!memcmp(temp_mac_addr, mac_addr, 6) && (vid1 == vid)) {
            /* select bin 1 to overwrite it */
            binNum = 1;
        }
    } 

    if (binNum == 1) {
        if (!((static_entry == 0) && (temp_static1 != static_entry))) {
            /* clear this entry*/                            
            temp = 0;
            soc_ARLA_FWD_ENTRY1r_field_set(unit, &result1, 
                ARL_VALIDf, &temp);

            if ((rv = REG_WRITE_ARLA_FWD_ENTRY1r(unit, &result1)) < 0) {
                goto arl_entry_delete_exit;
            }

            /* Write ARL Read/Write Control Register */
            reg_val32 = 0;
            /* Write Operation */
            temp = DINO16_MEM_OP_WRITE;
            soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
                ARL_RWf, &temp);

            temp = 1;
            soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
                ARL_STRTDNf, &temp);
            if ((rv = REG_WRITE_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
                goto arl_entry_delete_exit;
            }

            /* wait for complete */
            for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
                if ((rv = REG_READ_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
                    goto arl_entry_delete_exit;
                }
                soc_ARLA_RWCTLr_field_get(unit, &reg_val32, 
                    ARL_STRTDNf, &temp);
                if (!temp) {
                    break;
                }
            }

            if (count >= SOC_TIMEOUT_VAL) {
                rv =  SOC_E_TIMEOUT;
                goto arl_entry_delete_exit;
            }
        }                        
        goto arl_entry_delete_exit;
    }

    /* can not find a entry to overwrite based on same mac + vid */
    if (binNum == -1) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "entry_not found\n")));
        rv = SOC_E_NOT_FOUND;
    }    
arl_entry_delete_exit:
    MEM_UNLOCK(unit, INDEX(L2_ARLm));
    return rv;
}

static int
 _drv_dino16_mem_delete_ARL_by_port(int unit, uint32 src_port, 
    int static_entry)
{
    int  rv = SOC_E_NONE;
    soc_control_t  *soc = SOC_CONTROL(unit);
    l2_arl_sw_entry_t  output;
    uint64  temp_mac_field;
    uint8  temp_mac_addr[6];
    int  index_min, index_count;
    int  idx, temp_vid;
    uint32  valid;
    uint32  port;

    index_min = SOC_MEM_BASE(unit, INDEX(L2_ARLm));
    index_count = SOC_MEM_SIZE(unit, INDEX(L2_ARLm));
    if (soc->arl_table != NULL) {
        ARL_SW_TABLE_LOCK(soc); 
        for (idx = index_min; idx < index_count; idx++) {
            sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
            if (!ARL_ENTRY_NULL(&soc->arl_table[idx])) {
                sal_memcpy(&output, &soc->arl_table[idx], 
                    sizeof(l2_arl_sw_entry_t));
            } else {
                continue;
            }
            if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_VALID, (uint32 *)&output, &valid)) < 0) {
                ARL_SW_TABLE_UNLOCK(soc);
                return rv;
            }
            if (valid) {
                if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_SRC_PORT, (uint32 *)&output, &port)) < 0) {
                    ARL_SW_TABLE_UNLOCK(soc);
                    return rv;
                }
                if (port != src_port) {
                    continue;
                }
                if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_MAC, (uint32 *)&output, 
                    (uint32 *)&temp_mac_field)) < 0) {
                    ARL_SW_TABLE_UNLOCK(soc);
                    return rv;
                }
                if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_VLANID, (uint32 *)&output, 
                    (uint32 *)&temp_vid)) < 0) {
                    ARL_SW_TABLE_UNLOCK(soc);
                    return rv;
                }
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);

                if (!(temp_mac_addr[0] & 0x01)) { /* unicast address */
                    rv = _drv_dino16_mem_arl_entry_delete
                            (unit, temp_mac_addr, temp_vid, static_entry);

                    if (rv == SOC_E_NONE) {
                        soc_robo_arl_callback
                            (unit, (l2_arl_sw_entry_t *)&output, NULL);
                        /* Remove the entry from sw database */
                        sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
                        sal_memcpy(&soc->arl_table[idx], &output,
                            sizeof(l2_arl_sw_entry_t));
                    }
                }
            }
        } 
        ARL_SW_TABLE_UNLOCK(soc); 
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc arl table not allocated")));
        rv = SOC_E_FAIL;
    }

    return rv;
}

/*
 *  Function : drv_dino16_mem_read
 *
 *  Purpose :
 *      Get the width of selected memory. 
 *      The value returned in data is width in bits.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   the memory type to access.
 *      entry_id    :  the index of entry in the memory to be read.
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
drv_dino16_mem_read(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int  rv = SOC_E_NONE;
    int  i;
    uint32  retry, index_min, index_max, index;
    uint32  mem_id, temp;
    uint32  acc_ctrl = 0;
    uint32  *cache;
    uint8  *vmap;
    int  entry_size;
    uint64  mem_data0, mem_data1;
    int  reg_low, reg_high;
    uint8  temp_mac_addr[6];
    uint64  temp_mac_field;
    l2_arl_entry_t  *arl_mem = 0;

    sal_memset(&temp_mac_field, 0, sizeof(temp_mac_field));

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_dino16_mem_read(mem=0x%x,entry_id=0x%x,count=%d)\n"),
              mem, entry_id, count));
    switch (mem)
    {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
        case DRV_MEM_MARL:
            mem_id = INDEX(L2_ARLm);
            arl_mem = (l2_arl_entry_t *)entry;
            break;
        case DRV_MEM_VLAN:
            mem_id = INDEX(VLAN_1Qm);
            break;
        case DRV_MEM_MSTP:
            mem_id = INDEX(MSPT_TABm);
            break;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
        case DRV_MEM_MCAST:
        case DRV_MEM_SECMAC:
        case DRV_MEM_GEN:
        default:
            return SOC_E_PARAM;
    }

    
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);

    /* check count */
    if (count < 1) {
        return SOC_E_PARAM;
    }

    /* process read action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    /* add code here to check addr */
    if (mem_id == INDEX(L2_ARLm)) {        
        if ((rv = REG_READ_MEM_CTRLr(unit, &acc_ctrl)) < 0) {
            goto mem_read_exit;
        }

        temp = DINO16_ARL_MEMORY;
        soc_MEM_CTRLr_field_set(unit,&acc_ctrl, 
            MEM_TYPEf, &temp);
        
        if ((rv = REG_WRITE_MEM_CTRLr(unit, &acc_ctrl)) < 0) {
            goto mem_read_exit;
        }
    } else if (mem_id == INDEX(VLAN_1Qm)) {
        rv = _drv_dino16_vlan_mem_read(unit, entry_id, count, entry);
        goto mem_read_exit;
    } else {
        rv = _drv_dino16_mstp_mem_read(unit, entry_id, count, entry);
        goto mem_read_exit;
    }

    for (i = 0; i < count; i++) {
        if (((entry_id + i) < index_min) || ((entry_id + i) > index_max)) {
            MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
            return SOC_E_PARAM;
        }

        /* Return data from cache if active */
        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL && CACHE_VMAP_TST(vmap, (entry_id + i))) {
            sal_memcpy(entry, cache + (entry_id + i) * entry_size, entry_size);
            continue;
        }

        /* Read memory address register */
        if ((rv = REG_READ_MEM_ADDRr(unit, &acc_ctrl)) < 0) {
            goto mem_read_exit;
        }
        temp = DINO16_MEM_OP_READ;
        soc_MEM_ADDRr_field_set(unit, &acc_ctrl, 
            MEM_RWf, &temp);

        index = entry_id + i;
        temp = index / 2;
        /* Set memory entry address */
        soc_MEM_ADDRr_field_set(unit, &acc_ctrl, 
            MEM_ADRf, &temp);
        if ((rv = REG_WRITE_MEM_ADDRr(unit, &acc_ctrl)) < 0) {
            goto mem_read_exit;
        }

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

        if (mem_id == INDEX(L2_ARLm)) {
            /* Read the current generic memory entry */
            if (index % 2) {
                reg_high = MEM_ARL_B1_HIr;
                reg_low = MEM_ARL_B1_LOr;
            } else {
                reg_high = MEM_ARL_B0_HIr;
                reg_low = MEM_ARL_B0_LOr;
            }
    
            if ((rv = DRV_REG_READ(unit, 
                    DRV_REG_ADDR(unit, reg_low, 0, 0), 
                    (uint32 *)&mem_data0, 8)) < 0) {
                goto mem_read_exit;
            }
            if ((rv = DRV_REG_READ(unit, 
                    DRV_REG_ADDR(unit, reg_high, 0, 0), 
                    (uint32 *)&mem_data1, 8)) < 0) {
                goto mem_read_exit;
            }

            DRV_REG_FIELD_GET(unit, reg_low, (uint32 *)&mem_data0, 
                MACADDRf, (uint32 *)&temp_mac_field);                    
            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);                    
            if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_MAC, (uint32 *)arl_mem, 
                (uint32 *)&temp_mac_field)) < 0) {
                goto mem_read_exit;
            }

            DRV_REG_FIELD_GET(unit, reg_low, (uint32 *)&mem_data0, 
                VID_Rf, &temp);                    
            if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_VLANID, (uint32 *)arl_mem, &temp)) < 0) {
                goto mem_read_exit;
            }

            DRV_REG_FIELD_GET(unit, reg_low, (uint32 *)&mem_data0, 
                PRIORITY_Rf, &temp);                    
            if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_PRIORITY, (uint32 *)arl_mem, &temp)) < 0) {
                goto mem_read_exit;
            }

            DRV_REG_FIELD_GET(unit, reg_low, (uint32 *)&mem_data0, 
                VALID_Rf, &temp);                    
            if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_VALID, (uint32 *)arl_mem, &temp)) < 0) {
                goto mem_read_exit;
            }
              
            DRV_REG_FIELD_GET(unit, reg_high, (uint32 *)&mem_data1, 
                AGEf, &temp);
            if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_AGE, (uint32 *)arl_mem, &temp)) < 0) {
                goto mem_read_exit;
            }

            if (temp_mac_addr[0] & 0x01) {
                DRV_REG_FIELD_GET(unit, reg_high, (uint32 *)&mem_data1, 
                    PORTBMPf, &temp);
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_MARL, 
                    DRV_MEM_FIELD_DEST_BITMAP, (uint32 *)arl_mem, &temp)) < 0) {
                    goto mem_read_exit;
                }
            } else {
                DRV_REG_FIELD_GET(unit, reg_high, (uint32 *)&mem_data1, 
                    STATICf, &temp);
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_STATIC, (uint32 *)arl_mem, &temp)) < 0) {
                    goto mem_read_exit;
                }

                DRV_REG_FIELD_GET(unit, reg_high, (uint32 *)&mem_data1, 
                    PORTID_Rf, &temp);
                if ((rv = DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_SRC_PORT, (uint32 *)arl_mem, &temp)) < 0) {
                    goto mem_read_exit;
                }           
            }
            arl_mem ++;
        }
    }

 mem_read_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;

}


 /*
 *  Function : drv_dino16_mem_write
 *
 *  Purpose :
 *      Writes an internal memory.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   the memory type to access.
 *      entry_id    :  the index of entry in the memory to be read.
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
drv_dino16_mem_write(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int  rv = SOC_E_NONE;
    uint32  retry, index_min, index_max, index = 0;
    uint32  i;
    uint32  mem_id, temp;
    uint32  acc_ctrl = 0;
    uint32  *cache;
    uint8  *vmap;
    int  entry_size;
    uint64  mem_data0, mem_data1;
    l2_arl_entry_t  *arl_mem = 0;
    int  reg_low, reg_high;
    uint8   temp_mac_addr[6];
    uint64  temp_mac_field;
    
    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_dino16_mem_write(mem=0x%x,entry_id=0x%x,count=%d)\n"),
              mem, entry_id, count));
         
    switch (mem)
    {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
        case DRV_MEM_MARL:
            mem_id = INDEX(L2_ARLm);
            arl_mem = (l2_arl_entry_t *)entry;
            break;
        case DRV_MEM_VLAN:
            mem_id = INDEX(VLAN_1Qm);
            break;
        case DRV_MEM_MSTP:
            mem_id = INDEX(MSPT_TABm);
            break;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
        case DRV_MEM_MCAST:            
        case DRV_MEM_SECMAC:
        case DRV_MEM_GEN:
            return SOC_E_PARAM;
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

    /* process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    if (mem_id == INDEX(L2_ARLm)) {        
        if ((rv = REG_READ_MEM_CTRLr(unit, &acc_ctrl)) < 0) {
            goto mem_write_exit;
        }
        
        temp = DINO16_ARL_MEMORY;
        soc_MEM_CTRLr_field_set(unit, &acc_ctrl, 
            MEM_TYPEf, &temp);
        
        if ((rv = REG_WRITE_MEM_CTRLr(unit, &acc_ctrl)) < 0) {
            goto mem_write_exit;
        }
    } else if (mem_id == INDEX(VLAN_1Qm)) {
        rv = _drv_dino16_vlan_mem_write(unit, entry_id, count, entry);
        goto mem_write_exit;    
    } else {
        rv = _drv_dino16_mstp_mem_write(unit, entry_id, count, entry);
        goto mem_write_exit;
    }
    
    for (i = 0; i < count; i++) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
            return SOC_E_PARAM;
        }

        /* write data */
        COMPILER_64_ZERO(mem_data0);
        COMPILER_64_ZERO(mem_data1);
        index = entry_id + i;
 
        if (mem_id == INDEX(L2_ARLm)) {
            if (index % 2) {
                reg_high = MEM_ARL_B1_HIr;
                reg_low = MEM_ARL_B1_LOr;                    
            } else {
                reg_high = MEM_ARL_B0_HIr;
                reg_low = MEM_ARL_B0_LOr; 
            }
                    
            if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                (uint32 *)arl_mem, (uint32 *)&temp_mac_field)) < 0) {
                goto mem_write_exit;
            }
            DRV_REG_FIELD_SET(unit, reg_low, (uint32 *)&mem_data0, 
                MACADDRf, (uint32 *)&temp_mac_field);                           
            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);            

            if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_VLANID, (uint32 *)arl_mem, &temp)) < 0) {
                goto mem_write_exit;
            }
            DRV_REG_FIELD_SET(unit, reg_low, (uint32 *)&mem_data0, VID_Rf, 
                (uint32 *)&temp); 

            if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_PRIORITY, (uint32 *)arl_mem, &temp)) < 0) {
                goto mem_write_exit;
            }
            DRV_REG_FIELD_SET(unit, reg_low, (uint32 *)&mem_data0, 
                PRIORITY_Rf, (uint32 *)&temp);

            if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_VALID, (uint32 *)arl_mem, &temp)) < 0) {
                goto mem_write_exit;
            }
            DRV_REG_FIELD_SET(unit, reg_low, (uint32 *)&mem_data0, 
                VALID_Rf, &temp);

            if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_AGE, (uint32 *)arl_mem, &temp)) < 0) {
                goto mem_write_exit;
            }
            DRV_REG_FIELD_SET(unit, reg_high, (uint32 *)&mem_data1, 
                AGEf, &temp);

            if (temp_mac_addr[0] & 0x01) {
                if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_MARL, 
                    DRV_MEM_FIELD_DEST_BITMAP, (uint32 *)arl_mem, 
                    &temp)) < 0) {
                    goto mem_write_exit;
                }
                DRV_REG_FIELD_SET(unit, reg_high, (uint32 *)&mem_data1, 
                    PORTBMPf, &temp);             
            } else {
                if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_STATIC, (uint32 *)arl_mem, &temp)) < 0) {
                    goto mem_write_exit;
                }
                DRV_REG_FIELD_SET(unit, reg_high, (uint32 *)&mem_data1, 
                    STATICf, &temp); 

                if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_STATIC, (uint32 *)arl_mem, &temp)) < 0) {
                    goto mem_write_exit;
                }                
                DRV_REG_FIELD_SET(unit, reg_high, (uint32 *)&mem_data1, 
                    STATICf, (uint32 *)&temp); 

                if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_SRC_PORT, (uint32 *)arl_mem, &temp)) < 0) {
                    goto mem_write_exit;
                } 
                DRV_REG_FIELD_SET(unit, reg_high, (uint32 *)&mem_data1, 
                    PORTID_Rf, (uint32 *)&temp);                           
            }            

            if ((rv = REG_WRITE_MEM_FRM_DATA0r
                (unit, (uint32 *)&mem_data0)) < 0) {
                goto mem_write_exit;
            }              
            if ((rv = REG_WRITE_MEM_FRM_DATA1r
                (unit, (uint32 *)&mem_data1)) < 0) {
                goto mem_write_exit;
            }
            arl_mem ++;
        } 
        /* set memory address */
        temp = index / 2;
        soc_MEM_ADDRr_field_set(unit, &acc_ctrl, 
            MEM_ADRf, &temp);
        if ((rv = REG_WRITE_MEM_ADDRr(unit, &acc_ctrl)) < 0) {
            goto mem_write_exit;
        }
        
        if ((rv = REG_READ_MEM_ADDRr(unit, &acc_ctrl)) < 0) {
            goto mem_write_exit;
        }
        temp = DINO16_MEM_OP_WRITE;
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
            sal_memcpy(cache + (entry_id + i) * entry_size, entry, entry_size);
            CACHE_VMAP_SET(vmap, (entry_id + i));
        }

    }

 mem_write_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
    
}

/*
 *  Function : drv_dino16_mem_field_get
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
 *          on the parameter list, so it will return the state of all the ports.
 *      2. For DRV_MEM_ARL, the entry type will be l2_arl_sw_entry_t 
 *          and the mem_id is L2_ARL_SWm.
 */
int
drv_dino16_mem_field_get(int unit, uint32 mem, 
    uint32 field_index, uint32 *entry, uint32 *fld_data)
{

    soc_mem_info_t  *meminfo;
    soc_field_info_t  *fieldinfo;
    uint32  mask = -1;
    uint32  mask_hi, mask_lo;
    int  mem_id, field_id;
    int  i, wp, bp, len;
#ifdef BE_HOST
    uint32  val32;
#endif

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_dino16_mem_field_get(mem=0x%x,field_index=0x%x)\n"),
              mem, field_index));
         
    switch (mem)
    {
        case DRV_MEM_ARL_HW:        
        case DRV_MEM_ARL:
            mem_id = INDEX(L2_ARLm);
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = INDEX(MACADDRf);
            } else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
                field_id = INDEX(PORTID_Rf);
            } else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = INDEX(PRIORITY_Rf);
            } else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = INDEX(VID_Rf);
            } else if (field_index == DRV_MEM_FIELD_AGE) {
                field_id = INDEX(AGEf);
            } else if (field_index == DRV_MEM_FIELD_STATIC) {
                field_id = INDEX(STATICf);
            } else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VALID_Rf);
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MARL:
            mem_id = INDEX(L2_MARL_SWm);
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = INDEX(MACADDRf);
            } else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = INDEX(PORTBMPf);
            } else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = INDEX(PRIORITY_Rf);
            } else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = INDEX(VID_Rf);
            } else if (field_index == DRV_MEM_FIELD_AGE) {
                field_id = INDEX(AGEf);
            } else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VALID_Rf);
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_VLAN:
            mem_id = INDEX(VLAN_1Qm);
            if (field_index == DRV_MEM_FIELD_SPT_GROUP_ID) {
                field_id = INDEX(MSPT_IDf);
            } else if (field_index == DRV_MEM_FIELD_OUTPUT_UNTAG) {
                field_id = INDEX(UNTAG_MAPf);
            } else if (field_index == DRV_MEM_FIELD_PORT_BITMAP) {
                field_id = INDEX(FORWARD_MAPf);
            } else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VALID_Rf);
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MSTP:
            mem_id = INDEX(MSPT_TABm);
            if (field_index == DRV_MEM_FIELD_MSTP_PORTST) {
                sal_memcpy(fld_data, entry, sizeof(mspt_tab_entry_t));
                return SOC_E_NONE;
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
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

    SOC_FIND_FIELD(field_id,
         meminfo->fields,
         meminfo->nFields,
         fieldinfo);
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
 *  Function : drv_dino16_mem_field_set
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
drv_dino16_mem_field_set(int unit, uint32 mem, 
    uint32 field_index, uint32 *entry, uint32 *fld_data)
{
    soc_mem_info_t  *meminfo;
    soc_field_info_t  *fieldinfo;
    uint32  mask, mask_hi, mask_lo;
    int  mem_id, field_id;
    int  i, wp, bp, len;
#ifdef BE_HOST
    uint32  val32;
#endif

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_dino16_mem_field_set(mem=0x%x,field_index=0x%x)\n"),
              mem, field_index));
         
    switch (mem)
    {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
            mem_id = INDEX(L2_ARLm);
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = INDEX(MACADDRf);
            } else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
                field_id = INDEX(PORTID_Rf);
            } else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = INDEX(MARL_PBMP_IDXf);
                mem_id = INDEX(L2_MARL_SWm);
            } else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = INDEX(PRIORITY_Rf);
            } else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = INDEX(VID_Rf);
            } else if (field_index == DRV_MEM_FIELD_AGE) {
                field_id = INDEX(AGEf);
            } else if (field_index == DRV_MEM_FIELD_STATIC) {
                field_id = INDEX(STATICf);
            } else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VALID_Rf);
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MARL:
            mem_id = INDEX(L2_MARL_SWm);
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = INDEX(MACADDRf);            
            } else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id =INDEX(PORTBMPf);
            } else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = INDEX(PRIORITY_Rf);
            } else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = INDEX(VID_Rf);
            } else if (field_index == DRV_MEM_FIELD_AGE) {
                field_id = INDEX(AGEf);
            } else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VALID_Rf);
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_VLAN:
            mem_id = INDEX(VLAN_1Qm);
            if (field_index == DRV_MEM_FIELD_SPT_GROUP_ID) {
                field_id = INDEX(MSPT_IDf);
            } else if (field_index == DRV_MEM_FIELD_OUTPUT_UNTAG) {
                field_id = INDEX(UNTAG_MAPf);
            } else if (field_index == DRV_MEM_FIELD_PORT_BITMAP) {
                field_id = INDEX(FORWARD_MAPf);
            } else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VALID_Rf);            
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MSTP:
            mem_id = INDEX(MSPT_TABm);
            if (field_index == DRV_MEM_FIELD_MSTP_PORTST) {
                sal_memcpy(entry, fld_data, sizeof(mspt_tab_entry_t));
                return SOC_E_NONE;
            } else {
                return SOC_E_PARAM;
            }
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
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
    SOC_FIND_FIELD(field_id,
         meminfo->fields,
         meminfo->nFields,
         fieldinfo);
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
 *  Function : drv_dino16_mem_clear
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
drv_dino16_mem_clear(int unit, uint32 mem)
{
    int  rv = SOC_E_NONE;
    uint32  count;
    int  mem_id;
    uint32  *entry;
    uint32  del_id;

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_dino16_mem_clear : mem=0x%x\n"), mem));
    switch (mem) {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
            mem_id = INDEX(L2_ARLm);
            /* Clear Dynamic ARL entries */
            rv = DRV_MEM_DELETE(unit, DRV_MEM_ARL, 
                NULL, DRV_MEM_OP_DELETE_ALL_ARL);
            return rv;
        case DRV_MEM_MARL:
            mem_id = INDEX(L2_ARLm);
            rv = DRV_MEM_DELETE(unit, DRV_MEM_MARL, 
                NULL, DRV_MEM_OP_DELETE_ALL_ARL);
            return rv;
        case DRV_MEM_VLAN:
            mem_id = INDEX(VLAN_1Qm);
            break;
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        default:
            return SOC_E_PARAM;
    }
   
    count = soc_robo_mem_index_count(unit, mem_id);

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
        if (rv != SOC_E_NONE) {
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
 *  Function : drv_dino16_mem_search
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
drv_dino16_mem_search(int unit, uint32 mem, 
    uint32 *key, uint32 *entry, uint32 *entry1, uint32 flags)
{
    int  rv = SOC_E_NONE;
    int  index;

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_dino16_mem_search : mem=0x%x, flags = 0x%x)\n"),
              mem, flags));
    switch (mem) {
        case DRV_MEM_ARL:
        case DRV_MEM_MARL:
            break;
        case DRV_MEM_VLAN:
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    rv = _drv_dino16_mem_search(unit, mem, key, entry, 
        entry1, flags, &index);
    
    return rv;
}

/*
 *  Function : drv_dino16_mem_insert
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
 *      flags = DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID;
 */
int 
drv_dino16_mem_insert(int unit, uint32 mem, uint32 *entry, uint32 flags)
{
    int  rv = SOC_E_NONE, sw_arl_update = 0;
    l2_arl_sw_entry_t  output;
    uint32  temp, count;
    uint64  entry_reg;
    uint32  reg32;            
    uint8   mac_addr[6];
    uint64  mac_field, mac_field_output, mac_field_entry;
    uint32  vid, control, vid_output, vid_entry;
    int  index, reg_macvid, reg_fwd, reg_len;

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_dino16_mem_insert : mem=0x%x, flags = 0x%x)\n"),
              mem, flags));
    switch (mem) {
        case DRV_MEM_ARL:
        case DRV_MEM_MARL:            
            break;
        case DRV_MEM_VLAN:
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }
    MEM_LOCK(unit, INDEX(L2_ARLm));
    /* search entry */
    sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
    if ((rv = _drv_dino16_mem_search
            (unit, DRV_MEM_ARL, entry, (uint32 *)&output, 
            NULL, flags, &index))< 0) {
        if ((rv != SOC_E_NOT_FOUND) && (rv != SOC_E_EXISTS)) {
            /* FULL condition will be handled to return here */
            goto mem_insert_exit;
        }
    }

    if (rv == SOC_E_EXISTS) {
        /* Return SOC_E_NONE instead of SOC_E_EXISTS to fit DV test. */
        if (!sal_memcmp(&output, entry, sizeof(l2_arl_sw_entry_t))) {
            rv = SOC_E_NONE;
            goto mem_insert_exit;
        } else {
            /* MAC Address */
            if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    (uint32 *)&output, (uint32 *)&mac_field_output)) < 0) {
                goto mem_insert_exit;
            }
            if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    entry, (uint32 *)&mac_field_entry)) < 0) {
                goto mem_insert_exit;
            }
            if (!sal_memcmp(&mac_field_entry, &mac_field_output, 
                    sizeof(mac_field_output))) {
                 /*  VLAN ID  */
                if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_VLANID, (uint32 *)&output, 
                        &vid_output)) < 0) {
                    goto mem_insert_exit;
                }
                if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                        DRV_MEM_FIELD_VLANID, entry, &vid_entry)) < 0) {
                    goto mem_insert_exit;
                }
                if (vid_output != vid_entry) {
                    rv = SOC_E_NONE;
                    goto mem_insert_exit;
                }                
            } else {
               rv = SOC_E_NONE;
                goto mem_insert_exit;
            }
        }
    }

    /* write entry if entry does not exist or mem content changed*/
    /* form entry */
    /* VLAN ID */
    if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
            DRV_MEM_FIELD_VLANID, entry, &vid)) < 0) {
        goto mem_insert_exit;
    }
    soc_ARLA_MACVID_ENTRY0r_field_set(unit, (uint32 *)&entry_reg, 
        VID_Rf, &vid);

    /* MAC Address */
    if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
            DRV_MEM_FIELD_MAC, entry, (uint32 *)&mac_field)) < 0) {
        goto mem_insert_exit;
    }
    SAL_MAC_ADDR_FROM_UINT64(mac_addr, mac_field);
    soc_ARLA_MACVID_ENTRY0r_field_set(unit, (uint32 *)&entry_reg, 
        ARL_MACADDRf, (uint32 *)&mac_field);  

    reg32 = 0;
    if (mac_addr[0] & 0x01) { /* The input is the mcast address */
        /* multicast group index */
        if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_MARL, 
                DRV_MEM_FIELD_DEST_BITMAP, entry, &temp)) < 0) {
            goto mem_insert_exit;
        }
        soc_MARLA_FWD_ENTRY0r_field_set(unit, &reg32, 
            FWD_PRT_MAPf, &temp);

        /* age */
        if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_MARL, 
                DRV_MEM_FIELD_AGE, entry, &temp)) < 0) {
            goto mem_insert_exit;
        }
        soc_MARLA_FWD_ENTRY0r_field_set(unit, &reg32, 
            ARL_AGEf, &temp);
        /* valid bit */
        temp = 1;
        soc_MARLA_FWD_ENTRY0r_field_set(unit, &reg32, 
            ARL_VALIDf, &temp);
    } else { /* unicast address */
        /* source port id */
        if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_SRC_PORT, entry, &temp)) < 0) {
            goto mem_insert_exit;
        }
        soc_ARLA_FWD_ENTRY0r_field_set(unit, &reg32, 
            PORTID_Rf, &temp);

        /* priority */
        if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_PRIORITY, entry, &temp)) < 0) {
            goto mem_insert_exit;
        }
        soc_ARLA_FWD_ENTRY0r_field_set(unit, &reg32, 
            ARL_PRIf, &temp);

        /* age */
        if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_AGE, entry, &temp)) < 0) {
            goto mem_insert_exit;
        }
        soc_ARLA_FWD_ENTRY0r_field_set(unit, &reg32, 
            ARL_AGEf, &temp);

        /* static */
        if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_STATIC, entry, &temp)) < 0) {
            goto mem_insert_exit;
        }
        soc_ARLA_FWD_ENTRY0r_field_set(unit, &reg32, 
            ARL_STATICf, &temp);

        temp = 1;
        soc_ARLA_FWD_ENTRY0r_field_set(unit, &reg32, 
            ARL_VALIDf, &temp);
    }

    /* write ARL and MAC/VID entry register*/
    if (index == 0) { /* entry 0 */
        reg_macvid = INDEX(ARLA_MACVID_ENTRY0r);
        reg_fwd = INDEX(ARLA_FWD_ENTRY0r);
    } else {
        reg_macvid = INDEX(ARLA_MACVID_ENTRY1r);
        reg_fwd = INDEX(ARLA_FWD_ENTRY1r);
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
    temp = DINO16_MEM_OP_WRITE;
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
        if (!temp) {
            break;
        }
    }

    if (count >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto mem_insert_exit;
    }

    sw_arl_update = 1;

mem_insert_exit:
    MEM_UNLOCK(unit, INDEX(L2_ARLm));

    if (sw_arl_update) {
        /* Add the entry to sw database */
        _drv_arl_database_insert(unit, index, entry);
    }

    return rv;    
}

/*
 *  Function : drv_dino16_mem_delete
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
drv_dino16_mem_delete(int unit, uint32 mem, uint32 *entry, 
    uint32 flags)
{
    int  rv = SOC_E_NONE, sw_arl_update = 0;
    l2_arl_sw_entry_t  output;
    uint32  entry_reg;
    uint32  temp, count;
    uint32  control;
    int    index;
    uint32  reg_value;
    uint32  ag_port_mode = 0, ag_vlan_mode = 0;
    uint32  ag_static_mode = 0;
    uint32  src_port = 0, vlanid = 0;


    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_dino16_mem_delete : mem=0x%x, flags = 0x%x)\n"),
              mem, flags));
    switch (mem) {
        case DRV_MEM_ARL:
        case DRV_MEM_MARL:
            break;
        case DRV_MEM_VLAN:
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_VLANVLAN:
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
    if (flags & DRV_MEM_OP_DELETE_ALL_ARL) {
        ag_static_mode = 1;
        ag_port_mode = 1;
    }
    
    if ((ag_port_mode) || (ag_vlan_mode)) {
        /* aging port mode */
        if (ag_port_mode) {
            reg_value = 0;
            if ((rv = REG_WRITE_FAST_AGING_PORTr(unit, &reg_value)) < 0) {
                goto mem_delete_exit;
            }            
            if (flags & DRV_MEM_OP_DELETE_ALL_ARL) {
                temp = 1;
                soc_FAST_AGING_PORTr_field_set(unit, &reg_value, 
                    AGE_OUT_ALL_PORTSf, &temp);
            } else { /* by port */
                temp = 0;
                if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_SRC_PORT, entry, &temp)) < 0) {
                   goto mem_delete_exit;
                } 
                src_port = temp;

                /* 5396 port age sw workaround */
                _drv_dino16_mem_delete_ARL_by_port
                    (unit, src_port, ag_static_mode);
                goto mem_delete_exit;
            }
            if ((rv = REG_WRITE_FAST_AGING_PORTr(unit, &reg_value)) < 0) {
                goto mem_delete_exit;
            }

            reg_value = 0;
            if ((rv = REG_READ_FAST_AGING_VIDr(unit, &reg_value)) < 0) {
                goto mem_delete_exit;
            }
            temp = 1;
            soc_FAST_AGING_VIDr_field_set(unit, &reg_value, 
                AGE_OUT_ALL_VIDSf, &temp);
            if ((rv = REG_WRITE_FAST_AGING_VIDr(unit, &reg_value)) < 0) {
                goto mem_delete_exit;
            }

        } else if (ag_vlan_mode) {
        /* aging vlan mode */
            temp = 0;
            reg_value = 0;
            if ((rv = REG_WRITE_FAST_AGING_VIDr(unit, &reg_value)) < 0) {
                goto mem_delete_exit;
            }

            if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_VLANID, entry, &temp)) < 0) {
               goto mem_delete_exit;
            }
            vlanid= temp;
            soc_FAST_AGING_VIDr_field_set(unit, &reg_value, 
                AGE_VIDf, &temp);
            if ((rv = REG_WRITE_FAST_AGING_VIDr(unit, &reg_value)) < 0) {
                goto mem_delete_exit;
            }

            reg_value = 0;
            if ((rv = REG_WRITE_FAST_AGING_PORTr(unit, &reg_value)) < 0) {
                goto mem_delete_exit;
            }
 
            temp = 1;
            soc_FAST_AGING_PORTr_field_set(unit, &reg_value, 
                AGE_OUT_ALL_PORTSf, &temp);
            if ((rv = REG_WRITE_FAST_AGING_PORTr(unit, &reg_value)) < 0) {
                goto mem_delete_exit;
            }

        }

        /* start fast aging process */
        if ((rv = REG_READ_FAST_AGE_CTLr(unit, &reg_value)) < 0) {
            goto mem_delete_exit;
        }

        if (ag_static_mode) {
            temp = 1;    
        } else {
            temp = 0;
        }
        soc_FAST_AGE_CTLr_field_set(unit, &reg_value, 
            EN_FAST_AGE_STATICf, &temp);

        temp = 1;
        soc_FAST_AGE_CTLr_field_set(unit, &reg_value, 
            FAST_AGE_START_DONEf, &temp);
        if ((rv = REG_WRITE_FAST_AGE_CTLr(unit, &reg_value)) < 0) {
            goto mem_delete_exit;
        }
        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = REG_READ_FAST_AGE_CTLr(unit, &reg_value)) < 0) {
                goto mem_delete_exit;
            }
            soc_FAST_AGE_CTLr_field_get(unit, &reg_value, 
                FAST_AGE_START_DONEf, &temp);

            if (!temp) {
                break;
            }
        }
        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_delete_exit;
        }
         /* Remove entries from software table by port/vlan */
        _drv_arl_database_delete_by_fastage(unit, src_port,
            vlanid, flags);

    } else { /* normal deletion */
        LOG_INFO(BSL_LS_SOC_MEM,
                 (BSL_META_U(unit,
                             "\t Normal ARL DEL with Static=%d\n"), ag_static_mode));

        MEM_LOCK(unit, INDEX(L2_ARLm));
        /* search entry */
        sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
        if ((rv =  _drv_dino16_mem_search
                (unit, DRV_MEM_ARL, entry, (uint32 *)&output, 
                NULL, flags, &index)) < 0) {
            if ((rv != SOC_E_NOT_FOUND) && (rv != SOC_E_EXISTS)) {
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
            if (index == 0) { 
                /* entry 0 */
                if ((rv = REG_READ_ARLA_FWD_ENTRY0r(unit, &entry_reg)) < 0) {
                    goto mem_delete_exit;
                }
                /* check static bit if set */
                if (!ag_static_mode) {
                    soc_ARLA_FWD_ENTRY0r_field_get(unit, (uint32 *)&entry_reg, 
                        ARL_STATICf, &temp);
                    if (temp) {
                        LOG_INFO(BSL_LS_SOC_MEM,
                                 (BSL_META_U(unit,
                                             "\t Entry exist with static=%d\n"), temp));
                        rv = SOC_E_NOT_FOUND;
                        goto mem_delete_exit;
                    }
                }

                temp = 0;
                soc_ARLA_FWD_ENTRY0r_field_set(unit, &entry_reg, 
                    ARL_VALIDf, &temp);

                if ((rv = REG_WRITE_ARLA_FWD_ENTRY0r(unit, &entry_reg)) < 0) {
                    goto mem_delete_exit;
                }
            } else { 
                /* entry 1 */
                if ((rv = REG_READ_ARLA_FWD_ENTRY1r(unit, &entry_reg)) < 0) {
                    goto mem_delete_exit;
                }
               /* check static bit if set */
                if (!ag_static_mode) {
                    soc_ARLA_FWD_ENTRY1r_field_get(unit, (uint32 *)&entry_reg, 
                        ARL_STATICf, &temp);

                    if (temp) {
                        LOG_INFO(BSL_LS_SOC_MEM,
                                 (BSL_META_U(unit,
                                             "\t Entry exist with static=%d\n"), temp));
                        rv = SOC_E_NOT_FOUND;
                        goto mem_delete_exit;
                    }
                }

                temp = 0;
                soc_ARLA_FWD_ENTRY1r_field_set(unit, &entry_reg, 
                    ARL_VALIDf, &temp);
                if ((rv = REG_WRITE_ARLA_FWD_ENTRY1r(unit, 
                    (uint32 *)&entry_reg)) < 0) {
                    goto mem_delete_exit;
                }
            }

            /* Write ARL Read/Write Control Register */
            if ((rv = REG_READ_ARLA_RWCTLr(unit, &control)) < 0) {
                goto mem_delete_exit;
            }
            temp = DINO16_MEM_OP_WRITE;
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

            /* Set flag to record the entry is deleted from HW */
            sw_arl_update = 1;
        }
    }

mem_delete_exit:
    if (((ag_port_mode) || (ag_vlan_mode))) {
        if (ag_static_mode) {
            if ((rv = REG_READ_FAST_AGE_CTLr(unit, &reg_value)) < 0) {
                return rv;
            }
            temp = 0;
            soc_FAST_AGE_CTLr_field_set(unit, &reg_value, 
                EN_FAST_AGE_STATICf, &temp);
            if ((rv = REG_WRITE_FAST_AGE_CTLr(unit, &reg_value)) < 0) {
                return rv;
            }
            
        }
    } else {
        MEM_UNLOCK(unit, INDEX(L2_ARLm));
    }

    if (sw_arl_update) {
        /* Remove the entry from sw database */
        _drv_arl_database_delete(unit, index, &output);
    }

    return rv;    
}

