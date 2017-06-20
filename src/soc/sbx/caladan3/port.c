/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: port.c,v 1.83.14.22 Broadcom SDK $
 *
 * File:    port.c
 * Purpose: Caladan3 port related drivers
 * Requires:
 * Notes:
 */

 
#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#if defined(BCM_CALADAN3_SUPPORT)

#include <soc/cm.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>
#include <soc/error.h>
#include <soc/sbx/caladan3/port.h>
#include <soc/sbx/caladan3.h>
#include <soc/sbx/caladan3/sws.h>
#include <soc/sbx/caladan3/util.h>
#include <sal/appl/sal.h>
#include <soc/sbx/sbx_drv.h>

/* 7 was a magic number introduced long ago with no documentation. */
STATIC_ASSERT((SOC_SBX_CALADAN3_MAX_LINE_PORT) <= \
              ((SBX_MAX_PORTS) - 7), Static_Assert_For_Max_Line_Port_Check_Failed);

STATIC_ASSERT((SOC_SBX_CALADAN3_SWS_INGRESS_CMIC_SQUEUE) >= \
              (SOC_SBX_CALADAN3_SWS_CMIC_PORT_BASE), Static_Assert_For_CMIC_Check_Failed);

#define SOC_SBX_CALADAN3_1GE_PORTS_PER_WCORE  (4)
#define SOC_SBX_CALADAN3_1GE_MAX_PORTS        (48)
#define SOC_SBX_CALADAN3_MAX_CHANNEL_PORTS    (3)
#define SOC_SBX_CALADAN3_MAX_CHANNELS         (64)


/*
 * This struct is coupled with the
 * sbx_caladan3_port_intf_type_e defined in port.h
 * and tdm database in terms of its ordering. Coder beware.
 * New additions permitted only at the end
 */
static sbx_caladan3_intf_attr_t sbx_caladan3_intf_attr[SOC_SBX_CALADAN3_MAX_PORT_INTF_TYPE] = {
    {"ge",        1, 3, 12, SOC_BLK_CLPORT,  1, 0,   1000,   1488, 100,   2},
    {"xe",       10, 3,  4, SOC_BLK_CLPORT,  1, 0,  10000,  14881,  10,   8},
    {"xle",      40, 3,  1, SOC_BLK_CLPORT,  4, 0,  40000,  59523,   2,  32},
    {"ce",      100, 1,  1, SOC_BLK_CLPORT, 10, 0, 100000, 148809,   1,  96},
    {"hg10",     10, 3,  4, SOC_BLK_CLPORT,  1, 1,  10000,  36157,  10,   8},
    {"hg25",     25, 3, 64, SOC_BLK_CLPORT,  4, 1,  25000,  36157,   4,  16},
    {"hg42",     42, 3, 64, SOC_BLK_CLPORT,  4, 1,  42000,  56818,   2,  34},
    {"hg126",   127, 1, 64, SOC_BLK_CLPORT, 12, 1, 127272, 180785,   1, 102},
    {"ge",        1, 1, 12, SOC_BLK_XTPORT,  1, 0,   1000,   1488, 100,   2},
    {"il50w", SOC_SBX_CALADAN3_ILKN50w_SPEED, 1, 64,     SOC_BLK_IL, 12, 1,  59701,  84803,   2,  76},
    {"il50n", SOC_SBX_CALADAN3_ILKN50n_SPEED, 1, 64,     SOC_BLK_IL,  6, 1,  59104,  84955,   2,  76},
    {"il100", SOC_SBX_CALADAN3_ILKN100_SPEED, 1, 64,     SOC_BLK_IL, 12, 1, 118209, 167910,   1,  96},
    {"cmic",      1, 1,  1,   SOC_BLK_CMIC,  0, 0,   1000,   1488, 100,   2},
    {"xlport",    1, 1,  2, SOC_BLK_XLPORT,  1, 0,   1000,   1488, 100,   2},
    {"xaui",     10, 3,  1, SOC_BLK_CLPORT,  4, 0,  10000,  14881,  10,   8}

};


#define SOC_SBX_CALADAN3_CLIENT_MAX_BLK (3)
typedef struct sbx_caladan3_sws_client_info_s {
    uint8 numblk;
    char *name;
    soc_block_type_t type;
    uint8 max_intf;
} sbx_caladan3_sws_client_info_t;

static sbx_caladan3_sws_client_info_t sbx_caladan3_client_attr[SOC_SBX_CALADAN3_MAX_CLIENT] = {
    {2, "il", SOC_BLK_IL, 1},
    {2, "clport", SOC_BLK_CLPORT, 12},
    {3, "xtport", SOC_BLK_XTPORT, 12},
    {1, "xlport", SOC_BLK_XLPORT, 4},
    {1, "cmic", SOC_BLK_CMIC, 1}
};

#define SOC_SBX_CALADAN3_MAX_IL_INTF (1)
#define SOC_SBX_CALADAN3_LINE_IL_INST (0)
#define SOC_SBX_CALADAN3_FABRIC_IL_INST (1)
#define SOC_SBX_CALADAN3_LINE_CL_INST (0)
#define SOC_SBX_CALADAN3_FABRIC_CL_INST (1)
#define SOC_SBX_CALADAN3_MAX_CL_100GE_INTF (1)

static soc_sbx_caladan3_port_map_t *_caladan3_port_map[SOC_MAX_NUM_DEVICES];

static char *_caladan3_flow_ctrl_str[] = {
     "none", 
     "pause", 
     "safc", 
     "hcfc_oob", 
     "ilkn", 
     "ilkn_oob", 
     "off"
};


static const soc_field_t xmac_control_reset_field[] = {
    XMAC0_RESETf,
    XMAC1_RESETf,
    XMAC2_RESETf
};

static const soc_field_t port_soft_reset_xport_field[] = {
    XPORT_CORE0f,
    XPORT_CORE1f,
    XPORT_CORE2f
};


#define SOC_SBX_CALADAN3_CONTROL_PORTS_BW (2)

#define SOC_SBX_CALADAN3_OVERSUB_THRESHOLD (25)
#define SOC_SBX_CALADAN3_MAX_BW (126)
#define SOC_SBX_CALADAN3_MAX_BLK_INSTANCE (3) /* xtport has max instances */
#define SOC_SBX_CALADAN3_MAX_CHANNELIZED_PORTS (3)

#define SOC_SBX_CALADAN3_IF_ERROR_SINGLE_INST_BLK(unit,type)   \
  do {                                                         \
    if ((type) == SOC_BLK_CMIC || (type) == SOC_BLK_XLPORT) {  \
      LOG_ERROR(BSL_LS_SOC_COMMON, \
                (BSL_META("Unit %d *** ERROR: single"     \
                          " instance block \n"), unit));                        \
      return SOC_E_PARAM;                                      \
    }                                                          \
  } while(0);


int c3_port_init[BCM_MAX_NUM_UNITS] = {0};

#define _40GE_PHY_PORT_OFFSET_  (4)
#define _HG42_PHY_PORT_OFFSET_  (4)
#define _HG25_PHY_PORT_OFFSET_  (4)
#define _HG10_PHY_PORT_OFFSET_  (4)
#define _HG126_PHY_PORT_OFFSET_ (0)


/*#define MAC_PORT_MODE_ACCESS_DEBUG*/


int _soc_sbx_caladan3_mac_in_reset(int unit, int port, int subport, int mac_mode, 
                                   uint32 *current_mac_reset, uint32 *new_mac_reset);
int _soc_sbx_caladan3_mac_out_reset(int unit, int port, int subport, int mac_mode);
extern int (*_phy_wcmod_firmware_set_helper[])(int, int, uint8 *,int);

/*
 * Function to directly access PORT_MODE_REG. Tradtional SOC layer API depends on port
 * being present on a HW block being accessed.  This function is to address the issue 
 * with HW coming out of reset with XMACs enabled instead of disabled. 
 *
 * Note that this function is specifically for accessing the XMACs on C3.  The more 
 * general SOC method for obtaining a HW address is bypassed due to port dependencies 
 * that need to be avoided.  The more general and thus more portable method should be
 * employed unless there is a valid specific reason for doing otherwise.
 */
int soc_sbx_caladan3_xmac_disable(int unit, uint32 xmac_block_type, uint32 xmac_block_num,
                                  uint32 xmac_num)
{
    int block = 0;
    int schan_blk = 0;
    uint32 mac_control = 0;
    uint32 port_mode = 0;
    uint8 acc_type = 0;
    uint32 mac_control_base_addr, port_mode_base_addr;
    int core_mode = SOC_SBX_CALADAN3_CORE_PORT_DISABLE;
    uint32 mac_in_reset = 0;

    uint32 port_mode_core_mode __attribute__((unused))= 0;
    uint32 port_mode_debug = 0;

    if ((block = soc_sbx_block_find(unit, xmac_block_type, xmac_block_num)) != -1 &&
        xmac_block_num < 3) {

        /* We need to bypass some SOC functionality that depends on port being present
         * on a HW block since we need access to the block but a port may not exist 
         * on that block due to TDM configuration specified.
         * For example, can NOT use the following typical approach:
         *     (READ_PORT_MAC_CONTROLr(unit, port, &mac_control));
         * since its port-dependent.
         */

/* Cheryl - copied this from register.h in 6.3.1 SBX */
/*#define SOC_REG_FLAG_ACCSHIFT   17*/      /* Shift corresponding to ACCTYPE */

        acc_type = SOC_REG_ACC_TYPE(unit, PORT_MAC_CONTROLr);

/*        acc_type = (SOC_REG_INFO(unit, PORT_MAC_CONTROLr).flags >> SOC_REG_FLAG_ACCSHIFT) & 7;*/
        mac_control_base_addr = SOC_REG_INFO(unit, PORT_MAC_CONTROLr).offset;
        port_mode_base_addr = SOC_REG_INFO(unit, PORT_MODE_REGr).offset;
        schan_blk = SOC_BLOCK_INFO(unit, block).cmic;

        _soc_reg32_get(unit, schan_blk, acc_type, mac_control_base_addr, &mac_control);

        mac_in_reset = soc_reg_field_get(unit, PORT_MAC_CONTROLr, mac_control, 
                                         xmac_control_reset_field[xmac_num]);

         /* Put the MAC in reset; required */
        if (!mac_in_reset) {

            soc_reg_field_set(unit, PORT_MAC_CONTROLr, &mac_control,
                              xmac_control_reset_field[xmac_num], 1);
            
            _soc_reg32_set(unit, schan_blk, acc_type, mac_control_base_addr, mac_control);
                
            sal_udelay(30);
        }

        _soc_reg32_get(unit, schan_blk, acc_type, port_mode_base_addr, &port_mode);

        if (xmac_num == 0) {
            soc_reg_field_set(unit, PORT_MODE_REGr, &port_mode, XPORT0_CORE_PORT_MODEf, core_mode);
        } else if (xmac_num == 1) {
            soc_reg_field_set(unit, PORT_MODE_REGr, &port_mode, XPORT1_CORE_PORT_MODEf, core_mode);
        } else if (xmac_num == 2) {
            soc_reg_field_set(unit, PORT_MODE_REGr, &port_mode, XPORT2_CORE_PORT_MODEf, core_mode);
        } else {
            assert(0);
        }

        _soc_reg32_set(unit, schan_blk, acc_type, port_mode_base_addr, port_mode);

        _soc_reg32_get(unit, schan_blk, acc_type, port_mode_base_addr, &port_mode_debug);
        if (xmac_num == 0) {
            port_mode_core_mode = soc_reg_field_get(unit, PORT_MODE_REGr, port_mode_debug, 
                                                XPORT0_CORE_PORT_MODEf);
        } else if (xmac_num == 1) {
            port_mode_core_mode = soc_reg_field_get(unit, PORT_MODE_REGr, port_mode_debug, 
                                                XPORT1_CORE_PORT_MODEf);
        } else if (xmac_num == 2) {
            port_mode_core_mode = soc_reg_field_get(unit, PORT_MODE_REGr, port_mode_debug, 
                                                XPORT2_CORE_PORT_MODEf);
        } else {
            assert(0);
        }

/*

        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit:%d  xmac_block_type:%d  block:%d   xmac_block_num:%d  xmac_num:%d  port_mode_reg_core_mode:%d\n"), 
                   unit, xmac_block_type, block, xmac_block_num, xmac_num, port_mode_core_mode));
*/


        /* Take the MAC back out of reset */
        if (!mac_in_reset) {
            soc_reg_field_set(unit, PORT_MAC_CONTROLr, &mac_control, 
                              xmac_control_reset_field[xmac_num], 0);
            
            _soc_reg32_set(unit, schan_blk, acc_type, mac_control_base_addr, mac_control);
            

            sal_udelay(30);
        }

        return SOC_E_NONE;
    }

    return SOC_E_NONE;
}



/* This function returns is_channelized=TRUE if a line side port is channelized and not the first/base
 * channel of the port.  Otherwise, it returns FALSE.
 * requires_all_subport_setup is TRUE if this interface type requires calls to be made into low-level
 * code like wcmod for all subports on the base phy port.
 */
int
soc_sbx_caladan3_port_is_channelized_subport(int unit, soc_port_t port, int *is_channelized,
    int *requires_phy_setup) 
{
    soc_sbx_caladan3_port_map_t *port_map;

    if (_caladan3_port_map[unit] == NULL) {
        return SOC_E_PARAM;
    }

    if (is_channelized == NULL || requires_phy_setup == NULL) {
        return SOC_E_PARAM;
    }
    *requires_phy_setup = FALSE;
    *is_channelized = FALSE;

    port_map = _caladan3_port_map[unit];

    if (port >= SOC_SBX_CALADAN3_PORT_MAP_ENTRIES) {
        return SOC_E_PARAM;
    }

    if (port > (port_map->max_port)*2) {
        *is_channelized = FALSE;

    } else if (!soc_sbx_caladan3_is_line_port(unit, port)) {
        *is_channelized = FALSE;

    } else if (!(port_map->line_port_info[port].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
        *is_channelized = FALSE;

    } else if (port_map->line_port_info[port].flags & SOC_SBX_CALADAN3_IS_CHANNELIZED_SUBPORT) {
        *is_channelized = TRUE;
    }

    if (port_map->line_port_info[port].intftype == SOC_SBX_CALADAN3_PORT_INTF_IL100) {
        *requires_phy_setup = TRUE;
    }

    return SOC_E_NONE;
}

/*
 * Function: soc_sbx_caladan3_port_flow_control_get
 * Purpose:  Get the configured Flow control 
 */    
int
soc_sbx_caladan3_port_flow_control_mode_get(int unit, soc_port_t port, 
                                            soc_sbx_caladan3_flow_control_type_t *mode) 
{
    soc_sbx_caladan3_port_map_t *port_map;
    int fc = -1;

    if (_caladan3_port_map[unit] == NULL) {
        return SOC_E_PARAM;
    }

    if (mode == NULL) {
        return SOC_E_PARAM;
    }

    /* Allow Overrides */
    if (IS_HG_PORT(unit, port)) {
        if (soc_sbx_caladan3_is_line_port(unit, port)) {
            fc = soc_property_port_get(unit, port, "fc_type_il_line", -1);
        } else {
            fc = soc_property_port_get(unit, port, "fc_type_il_fabric", -1);
        }
    }

    if (fc >= 0) {
        switch (fc) {
        case 2:
            *mode = SOC_SBX_CALADAN3_FC_TYPE_HCFC_OOB;
            break;
        case 1:
        case 3:
            *mode = SOC_SBX_CALADAN3_FC_TYPE_ILKN_OOB;
            break;
        case 4:
            *mode = SOC_SBX_CALADAN3_FC_TYPE_NONE;
            break;
        default:
        case 0:
            *mode = SOC_SBX_CALADAN3_FC_TYPE_ILKN;
            break;
        }
    } else {
        /*
         * Another set of Supported values are defined in property.h
         * 0=none/inband,2=ilkn_oob,3=hcfc_oob
         */
        fc = soc_property_port_get(unit, port, spn_FC_OOB_TYPE, -1);
        if (fc >= 0) {
            if (fc == 3) {
                *mode = SOC_SBX_CALADAN3_FC_TYPE_HCFC_OOB;
            } else if (fc == 2) {
                *mode = SOC_SBX_CALADAN3_FC_TYPE_ILKN_OOB;
            }
        } else {
            port_map = _caladan3_port_map[unit];
            if (port >= SOC_SBX_CALADAN3_PORT_MAP_ENTRIES ||
                !(port_map->line_port_info[port].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
                return SOC_E_PARAM;
            }
            if (soc_sbx_caladan3_is_line_port(unit, port)) {
                *mode = port_map->line_port_info[port].flow_control;
            } else {
                *mode = port_map->fabric_port_info[port].flow_control;
            }
        }
    }
    return SOC_E_NONE;
}

/*
 * Function: soc_sbx_caladan3_port_flow_control_set
 * Purpose:  Set the configured Flow control 
 */    
int
soc_sbx_caladan3_port_flow_control_mode_set(int unit, soc_port_t port, 
                                            soc_sbx_caladan3_flow_control_type_t mode) 
{
    soc_sbx_caladan3_port_map_t *port_map;

    if (_caladan3_port_map[unit] == NULL) {
        return SOC_E_PARAM;
    }
    if ((mode < SOC_SBX_CALADAN3_FC_TYPE_NONE) &&
         (mode >= SOC_SBX_CALADAN3_FC_TYPE_MAX)) {
        return SOC_E_PARAM;
    }

    port_map = _caladan3_port_map[unit];

    if (port >= SOC_SBX_CALADAN3_PORT_MAP_ENTRIES ||
        !(port_map->line_port_info[port].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
        return SOC_E_PARAM;
    }

    if (soc_sbx_caladan3_is_line_port(unit, port)) {
        port_map->line_port_info[port].flow_control = mode;
    } else {
        port_map->fabric_port_info[port].flow_control = mode;
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_port_queues_get
 * Purpose:
 *    Get the queues info for a given port
 *    dir = 0 for ingress, 1 for egress
 */
int
soc_sbx_caladan3_port_queues_get(int unit, int port, int dir, soc_sbx_caladan3_queues_t **queueinfo)
{
    soc_sbx_caladan3_port_map_t *port_map;
    int line = 0;

    if (!queueinfo) { 
        return SOC_E_PARAM;
    }

    port_map = _caladan3_port_map[unit];
    if (!port_map) {
        return SOC_E_PARAM;
    }

    line = (soc_sbx_caladan3_is_line_port(unit, port)) ? 1 : 0;

    if ( !line || (port >= SOC_SBX_CALADAN3_PORT_MAP_ENTRIES)) {
        return SOC_E_PARAM;
    }

    port = port_map->pbmp2idx[port].portidx;

    if (!dir) {
        if (!(port_map->line_port_info[port].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            return SOC_E_PARAM;
        }
        *queueinfo = &port_map->line_port_info[port].port_queues;
    } else {
        if (!(port_map->fabric_port_info[port].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            return SOC_E_PARAM;
        }
        *queueinfo = &port_map->fabric_port_info[port].port_queues;
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_port_queues_remove
 * Purpose:
 *    Update software state when queue is removed from a port
 */
int
soc_sbx_caladan3_port_queues_remove(int unit, int port, int qid, int type)
{
    soc_sbx_caladan3_port_map_t *port_map;
    soc_sbx_caladan3_queues_t *qi;
    int line = 0;
    int idx = 0;

    port_map = _caladan3_port_map[unit];
    if (!port_map) {
        return SOC_E_PARAM;
    }

    line = (soc_sbx_caladan3_is_line_port(unit, port)) ? 1 : 0;
    qid %= SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION;

    if (port >= SOC_SBX_CALADAN3_PORT_MAP_ENTRIES) {
        return SOC_E_PARAM;
    }

    if (line) {
        port = port_map->pbmp2idx[port].portidx;
        if (!(port_map->line_port_info[port].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            return SOC_E_PARAM;
        }
        qi = &port_map->line_port_info[port].port_queues;
        if (type == INGRESS_SQUEUE) {
            if (SOC_SBX_C3_BMP_MEMBER(qi->squeue_bmp, qid)) {
                SOC_SBX_C3_BMP_REMOVE(qi->squeue_bmp, qid);
                qi->num_squeue--;
                if (qi->num_squeue < 0) {
                    qi->num_squeue = 0;
                }
            } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: line squeue qid (%d) doesnt belong to port %d\n"),
                           unit, qid, port));
                return (SOC_E_PARAM);
            }
        } else if (type == INGRESS_DQUEUE) {
            if (SOC_SBX_C3_BMP_MEMBER(qi->dqueue_bmp, qid)) {
                SOC_SBX_C3_BMP_REMOVE(qi->dqueue_bmp, qid);
                qi->num_dqueue--;
                if (qi->num_dqueue < 0) {
                    qi->num_dqueue = 0;
                }
            } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** Error: line dqueue qid (%d) doesnt belong to port %d\n"),
                           unit, qid, port));
                return (SOC_E_PARAM);
            }
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** Error: Invalid line queue type for qid (%d)\n"),
                       unit, qid));
            return (SOC_E_PARAM);
        }
    } else {
        /* handle spread out fabric port info */
        for (idx =0; idx < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; idx++) {
            if (!(port_map->fabric_port_info[idx].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
                continue;
            }
            qi = &port_map->fabric_port_info[idx].port_queues;
            if (SOC_SBX_C3_BMP_MEMBER(qi->squeue_bmp, qid)) {
                if (port_map->fabric_port_info[idx].port != port) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d *** Error: Queue (%d) does not belong to Port (%d)\n"),
                               unit, qid, port));
                    return (SOC_E_PARAM);
                }
                if (type == EGRESS_SQUEUE) {
                    SOC_SBX_C3_BMP_REMOVE(qi->squeue_bmp, qid);
                    qi->num_squeue--;
                    if (qi->num_squeue < 0) {
                        qi->num_squeue = 0;
                    }
                } else {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d *** Error: squeue qid (%d) is does not match type %d\n"),
                               unit, qid, type));
                    return (SOC_E_PARAM);
                }
            } else if (SOC_SBX_C3_BMP_MEMBER(qi->dqueue_bmp, qid)) {
                if (port_map->fabric_port_info[idx].port != port) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d *** Error: Queue (%d) does not belong to Port (%d)\n"),
                               unit, qid, port));
                    return (SOC_E_PARAM);
                }
                if (type == EGRESS_DQUEUE) {
                    SOC_SBX_C3_BMP_REMOVE(qi->dqueue_bmp, qid);
                    qi->num_dqueue--;
                    if (qi->num_dqueue < 0) {
                        qi->num_dqueue = 0;
                    }
                } else {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d *** Error: dqueue qid (%d) is does not match type %d\n"),
                               unit, qid, type));
                    return (SOC_E_PARAM);
                }
            }
        }
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_port_queues_add
 * Purpose:
 *    Update software state when queue is added to a port
 */
int
soc_sbx_caladan3_port_queues_add(int unit, int port, int qid, int type)
{
    soc_sbx_caladan3_port_map_t *port_map;
    soc_sbx_caladan3_queues_t *qi;
    int line = 0;

    port_map = _caladan3_port_map[unit];
    if (!port_map) {
        return SOC_E_PARAM;
    }

    line = (soc_sbx_caladan3_is_line_port(unit, port)) ? 1 : 0;
    if (!line) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** Error: Invalid port(%d) for qid (%d), need line side port\n"),
                   unit, port, qid));
        return (SOC_E_PARAM);
    }
    qid %= SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION;

    if (port >= SOC_SBX_CALADAN3_PORT_MAP_ENTRIES) {
        return SOC_E_PARAM;
    }
    port = port_map->pbmp2idx[port].portidx;

    if (!(port_map->line_port_info[port].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
        return SOC_E_PARAM;
    }
    qi = &port_map->line_port_info[port].port_queues;
    if (type == INGRESS_SQUEUE) {
        SOC_SBX_C3_BMP_ADD(qi->squeue_bmp, qid);
        qi->num_squeue++;
    } else if (type == INGRESS_DQUEUE) {
        SOC_SBX_C3_BMP_ADD(qi->dqueue_bmp, qid);
        qi->num_dqueue++;
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** Error: Invalid line queue type for qid (%d)\n"),
                   unit, qid));
        return (SOC_E_PARAM);
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_port_queues_update_bmp
 * Purpose:
 *    Translate to queue bmp format used to specify queue info for a given port
 *    This is used when parsing config.bcm.
 *    Old format of num_squeue/squeue/dqueue/num_dqueue is used to update the
 *    queue bitmap
 */
int
soc_sbx_caladan3_port_queues_update_bmp(int unit, soc_sbx_caladan3_queues_t *queueinfo)
{
    int rv = SOC_E_NONE;
    int start_q = 0, i, num_queues = 0;
    if (!queueinfo) { 
        return SOC_E_PARAM;
    }

    num_queues = queueinfo->num_squeue;
    start_q = queueinfo->squeue_base % SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION;
    if ((num_queues > 1) && (start_q >= 0)) {
        for (i=0; i < num_queues; i++) {
            SOC_SBX_C3_BMP_ADD(queueinfo->squeue_bmp, start_q + i);
        }
    } else {
        SOC_SBX_C3_BMP_ADD(queueinfo->squeue_bmp, start_q);
    }
    num_queues = queueinfo->num_dqueue;
    start_q = queueinfo->dqueue_base % SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION;
    if ((num_queues > 1) && (start_q >= 0)) {
        for (i=0; i < num_queues; i++) {
            SOC_SBX_C3_BMP_ADD(queueinfo->dqueue_bmp, start_q + i);
        }
    } else {
        SOC_SBX_C3_BMP_ADD(queueinfo->dqueue_bmp, start_q);
    }

    return rv;
}

/*
 * Function:
 *    soc_sbx_caladan3_port_queues_init
 * Purpose:
 *    Init port queues data for a port_info struct.
 *    Used when parsing the config.bcm
 */
void
soc_sbx_caladan3_port_queues_init(int unit, soc_sbx_caladan3_port_map_info_t *info)
{
    soc_sbx_caladan3_queues_t *qi ;
    if (info) {
        qi = &info->port_queues;
        qi->num_squeue = 0;
        qi->num_dqueue = 0;
        qi->squeue_base = -1;
        qi->dqueue_base = -1;
        SOC_SBX_C3_BMP_CLEAR(qi->squeue_bmp, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES);
        SOC_SBX_C3_BMP_CLEAR(qi->dqueue_bmp, SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES);
    }
}

/*
 * Function:
 *    soc_sbx_caladan3_port_is_wc_remapped
 * Purpose:
 *    Return true if WC is borrowed
 */
int
soc_sbx_caladan3_port_is_wc_remapped(int unit)
{
    soc_sbx_caladan3_port_map_t *port_map;

    if (_caladan3_port_map[unit] == NULL) {
        return SOC_E_PARAM;
    }

    port_map = _caladan3_port_map[unit];
    return (port_map->borrowed_wcore_count > 0) ? TRUE : FALSE;
}


/*
 *
 * Function:
 *     _soc_sbx_caladan3_parse_block
 * Purpose:
 *     Parse and return block information type, number, and index (port
 *     offset) for given string of format
 *
 * Format:
 *    ucode_port.<UPNUM>=<LINE_INFO>:<FABRIC_INFO>
 *    
 *   Where,
 *    <LINE_INFO>  = <BT><BN>.<IT>.<INST>.<PORT>-[FC]-[NUMCOS/SQ/DQ/NUMDQ]
 *    <FABRIC_INFO>= <BT><BN>.<IT>.<INST>.<PORT>-[FC]-[NUMCOS/SQ/DQ/NUMDQ]
 *
 *    <UPNUM> = Ucode Port number
 *    <BT>    = Block Type
 *    <BN>    = Block Num
 *    <IT>    = Interface Type
 *    <INST>  = Instance
 *    <PORT>  = Port
 *    <FC>    = Flow Control, on/oob/off, default is off
 *    <SQ>    = Squeue 
 *    <DQ>    = Dqueue
 *    <NUMCOS> = No of ingress queues to assign to this port
 *    <NUMDQ>  = No of  egress queues to assign to this port
 *
 *     eg:
 *     1) ucode_port.port1.0=clport0.hg10.0.0-10:il1.il100.0.0-10
 *     2) ucode_port.port1.0=clport0.hg10.0.0-10/0/192:il1.il100.0.0-10/64/128
 *
 */

STATIC
int _soc_sbx_caladan3_parse_block(int unit, 
                                   char *spn_str,
                                   soc_sbx_caladan3_port_map_info_t *info,
                                   int *parsed)
{
    char  *s, *ts;
    int   value;
    sbx_caladan3_intf_attr_t *intf_attr;
    sbx_caladan3_sws_client_info_t *blk_attr;
    soc_sbx_caladan3_queues_t *qinfo;
    int smallest = 0, cnt = 0, qid = 0, fc = 0;
    int i, index, len;
    uint16              dev_id;
    uint8               rev_id;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    info->base_port = -1;
    info->port = -1;
    info->uport = -1;
    qinfo = &info->port_queues;


    soc_sbx_caladan3_port_queues_init(unit, info);

    
    s = spn_str;

    /* Parse block */
    for (index=0; index < SOC_SBX_CALADAN3_CLIENT_XL; index++) {

        blk_attr = &sbx_caladan3_client_attr[index];
        len = strlen(blk_attr->name);

        if (!sal_strncasecmp(s, blk_attr->name,len)) {
            s += len;
            info->blktype = blk_attr->type;
            info->clienttype = index;
            break;
        }
    }

    /* do not expose XL, CMIC PCI ports for configuration, internally provision them */
    if (index == SOC_SBX_CALADAN3_CLIENT_XL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** ERROR: Bad Block %s \n"),
                   unit, s));
        return SOC_E_PARAM;
    }

    /* Parse block number */
    value = sal_ctoi(s, &ts);
    if (ts != s) {
        info->instance = value;
        s = ts;

        if ((info->blk = soc_sbx_block_find(unit, info->blktype, info->instance)) < 0) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** ERROR: Invalid Block instance %s \n"),
                       unit, s));
            return SOC_E_PARAM;
        }
    } else {
        SOC_SBX_CALADAN3_IF_ERROR_SINGLE_INST_BLK(unit, info->blktype);  
    }

    /* Parse interface type / <instance>.<port>, <instance>.<channel number> */
    if (*s == '.') {
        s++;

    /* certain interface type not supported on 88034 */
    if (dev_id == BCM88034_DEVICE_ID) {
        if (!(sal_strncasecmp(s, "il100", strlen("il100"))) ||
        !(sal_strncasecmp(s, "hg127", strlen("hg127"))) ||
        !(sal_strncasecmp(s, "il50w", strlen("il50w")))) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** ERROR: unsupported Interface type %s on 88034\n"),
                   unit, s));
        return SOC_E_PARAM;
        }
    }

        /* assume interface is specified */
        for (index=0; index < SOC_SBX_CALADAN3_MAX_PORT_INTF_TYPE; index++) {
            intf_attr = &sbx_caladan3_intf_attr[index];
            len = strlen(intf_attr->name);
            
            if ((info->blktype == intf_attr->type) && (!sal_strncasecmp(s, intf_attr->name,len))) {
                s += len;
                info->intftype = index;
                break;
            }
        }

        if (index == SOC_SBX_CALADAN3_MAX_PORT_INTF_TYPE) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** ERROR: Bad Interface type %s \n"),
                       unit, s));
            return SOC_E_PARAM;
        }

        info->intftype = index;
        intf_attr = &sbx_caladan3_intf_attr[index];

        /* parse instance */
        if (*s == '.') {
            s++;
            value = sal_ctoi(s, &ts);
            if (ts != s) {
                if (value <0 || value >= intf_attr->max_instance_per_blk) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d *** ERROR: instance[%d] not supported "
                                          "by interface %s Supported [0 - %d] \n"),
                               unit, value, intf_attr->name, intf_attr->max_instance_per_blk - 1));
                    return SOC_E_PARAM;
                }

                info->intf_instance = value;
                s = ts;
            } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** ERROR: No Instance number specified %s \n"),
                           unit, s));
                return SOC_E_PARAM;
            }            
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** ERROR: No Instance number specified %s \n"),
                       unit, s));
            return SOC_E_PARAM;
        }

        /* port or channel number */
        if (*s == '.') {
            s++;
            value = sal_ctoi(s, &ts);
            if (ts != s) {
                info->bindex = value;
                s = ts;
            } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** ERROR: No port/channel number specified %s \n"),
                           unit, s));
                return SOC_E_PARAM;
            }
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** ERROR: No port/channel number specified %s \n"),
                       unit, s));
            return SOC_E_PARAM;
        }

        /* look for flow control */
        if (*s == '-') {
            s++; 
            if (!sal_strncasecmp(s, "on", 2)) {
                fc = 1;
                s += 2;
            } else if (!sal_strncasecmp(s, "safc", 4)) {
                fc = 3;
                s += 4;
            } else if (!sal_strncasecmp(s, "oobilkn", 7)) {
                fc = 4;
                s += 7;
            } else if (!sal_strncasecmp(s, "oob", 3)) {
                fc = 2;
                s += 3;
            } 
            if (fc > 0) {
                /* OOB where possible */
                switch (info->intftype) {
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_1GE:
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_10GE:
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_XAUI_10GE:
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE:
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE:
                    info->flow_control = (fc == 3) ? SOC_SBX_CALADAN3_FC_TYPE_SAFC : \
                                              SOC_SBX_CALADAN3_FC_TYPE_PAUSE;
                    break;
                    
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10:
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25:
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42:
                case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126:
                    switch (fc) {
                    case 3:
                        info->flow_control = SOC_SBX_CALADAN3_FC_TYPE_SAFC;
                        break;
                    case 4:
                        info->flow_control = SOC_SBX_CALADAN3_FC_TYPE_ILKN_OOB;
                        break;
                    case 2:
                    default:
                        info->flow_control = SOC_SBX_CALADAN3_FC_TYPE_HCFC_OOB;
                        break;
                    }
                    break;
                    
                case SOC_SBX_CALADAN3_PORT_INTF_IL50w:
                case SOC_SBX_CALADAN3_PORT_INTF_IL50n:
                case SOC_SBX_CALADAN3_PORT_INTF_IL100:
                    if (fc == 3) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "Unit %d *** Warning unsupported FC mode %d\n"),
                                   unit, fc));
                    }
                    info->flow_control = (fc == 1) ? SOC_SBX_CALADAN3_FC_TYPE_ILKN : \
                                                     SOC_SBX_CALADAN3_FC_TYPE_ILKN_OOB;
                    break;
                    
                default:
                    assert(0);
                }
            } else if (!sal_strncasecmp(s, "off", 3)) {
                s += 3;
                info->flow_control = SOC_SBX_CALADAN3_FC_TYPE_NONE;
            } else {
                /* rewind pointer for line interface where no flow control value is specified
                 * Set to type max so that the right mode can be setup in code
                 */
                info->flow_control = SOC_SBX_CALADAN3_FC_TYPE_MAX;
                s--;
            }
        }

        /* if front panel port, looks for cos queues */
        if (*s == '-') {
            s++;
            if (*s == ':') {
                /* look for :w0:w1:w2:w3:
                 * Each word is a bitmap of the format
                 *   :<sq63..sq32>:<sq31..sq0>:<dq63..dq32>:<dq31:dq0>:
                 */
                s++;
                value = sal_ctoi(s, &ts);
                smallest = 256;
                for(i=0; i < 32; i++) {
                    if (value & (1 << i)) {
                        qid = (i + 32+ ((info->instance == 0) ? 
                                        SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE:
                                              SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE));
                        smallest = (smallest > qid) ? (qid) : (smallest);
                        qid %= SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION;
                        SOC_SBX_C3_BMP_ADD(qinfo->squeue_bmp, qid);
                        cnt++;
                    }
                }
                qinfo->num_squeue = cnt;
                cnt = 0;
                s = ts;
                if (*s == ':') {
                    s++;
                    value = sal_ctoi(s, &ts);
                    for(i=0; i < 32; i++) {
                        if (value & (1 << i)) {
                            qid = (i + ((info->instance == 0) ? 
                                            SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE:
                                              SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE));
                            smallest = (smallest > qid) ? (qid) : (smallest);
                            qid %= SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION;
                            SOC_SBX_C3_BMP_ADD(qinfo->squeue_bmp, qid);
                            cnt++;
                        }
                    }
                    qinfo->num_squeue += cnt;
                    /* update the smallest qid as squeue base */
                    qinfo->squeue_base = smallest;

                    /* Processing dqueues */
                    cnt = 0;
                    smallest = 256;
                    s = ts;
                    if (*s == ':') {
                        s++;
                        value = sal_ctoi(s, &ts);
                        for(i=0; i < 32; i++) {
                            if (value & (1 << i)) {
                                qid = (i + 32 + ((info->instance == 0) ? 
                                            SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE:
                                              SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE));
                                smallest = (smallest > qid) ? (qid) : (smallest);
                                qid %= SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION;
                                SOC_SBX_C3_BMP_ADD(qinfo->dqueue_bmp, qid);
                                cnt++;
                            }
                        }
                        qinfo->num_dqueue = cnt;
                        s = ts;
                    } else {
                       LOG_ERROR(BSL_LS_SOC_COMMON,
                                 (BSL_META_U(unit,
                                             "Unit %d *** ERROR: Dqueue bitmap not specified %s \n"),
                                  unit, s));
                       return SOC_E_PARAM;
                    }
                    if (*s == ':') {
                        s++;
                        cnt = 0;
                        value = sal_ctoi(s, &ts);
                        for(i=0; i < 32; i++) {
                            if (value & (1 << i)) {
                                qid = (i + ((info->instance == 0) ? 
                                            SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE:
                                              SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE));
                                smallest = (smallest > qid) ? (qid) : (smallest);
                                qid %= SOC_SBX_CALADAN3_SWS_QUEUE_PER_REGION;
                                SOC_SBX_C3_BMP_ADD(qinfo->dqueue_bmp, qid);
                                cnt++;
                            }
                        }
                        qinfo->num_dqueue += cnt;
                        qinfo->dqueue_base = smallest;
                        s = ts;
                    } else {
                       LOG_ERROR(BSL_LS_SOC_COMMON,
                                 (BSL_META_U(unit,
                                             "Unit %d *** ERROR: Dqueue bitmap format error %s \n"),
                                  unit, s));
                       return SOC_E_PARAM;
                    }
                } else {
                   LOG_ERROR(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "Unit %d *** ERROR: Squeue bitmap format error %s \n"),
                              unit, s));
                   return SOC_E_PARAM;
                }
            } else {
                /* look for cos queues format ncos/sq/dq/numdq  */
               value = sal_ctoi(s, &ts);
               if (ts != s) {
                   qinfo->num_squeue = value;
                   /* Assume symmetric */
                   qinfo->num_dqueue = qinfo->num_squeue; 
                   s = ts;
                   if (*s == '/') {
                       s++;
                       value = sal_ctoi(s, &ts);
                       if (ts != s) {
                           qinfo->squeue_base = value;
                           s = ts;
                           if (*s == '/') {
                               s++;
                               value = sal_ctoi(s, &ts);
                               if (ts != s) {
                                   qinfo->dqueue_base = value;
                                   s = ts;
                                   if (*s == '/') {
                                       s++;
                                       value = sal_ctoi(s, &ts);
                                       if (ts != s) {
                                           qinfo->num_dqueue = value;
                                           s = ts;
                                       }
                                   }
                               } else {
                                   LOG_ERROR(BSL_LS_SOC_COMMON,
                                             (BSL_META_U(unit,
                                                         "Unit %d *** ERROR: "
                                                         "SQ/DQ pair not specified %s \n"),
                                              unit, s));
                                   return SOC_E_PARAM;
                               }
                           }
                           /* Update bmp */
                           SOC_IF_ERROR_RETURN(
                               soc_sbx_caladan3_port_queues_update_bmp(unit, &info->port_queues));
                       } else {
                           LOG_ERROR(BSL_LS_SOC_COMMON,
                                     (BSL_META_U(unit,
                                                 "Unit %d *** ERROR: "
                                                 "SQ pair not specified %s \n"),
                                      unit, s));
                       }
                   }
               } else {
                   LOG_ERROR(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "Unit %d *** ERROR: No number of COS specified %s \n"),
                              unit, s));
                   return SOC_E_PARAM;
               }
           }
        }
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** ERROR: No Interface information specified %s \n"),
                   unit, s));
        return SOC_E_PARAM;
    }

    *parsed = s - spn_str;
    return SOC_E_NONE;
}


/*
 *
 * Function:
 *     _soc_sbx_caladan3_ucode_value_get
 * Purpose:
 *     Parse ucode property value string into front-panel and
 *     system-side block information.
 *
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Invalid string value for ucode property
 *
 * Notes:
 *     (a) This routine performs parameter checking for valid block type,
 *         number and port
 *     (b) For CMIC/XL- (CPU) block, only <b-type> and <b-num> are expected.
 */
STATIC int _soc_sbx_caladan3_ucode_get(int unit, 
                                       char *spn_str,
                                       soc_sbx_caladan3_port_map_t *port_map,
                                       int uport,
                                       int lport)
{
    char  *s;
    int   n_chars=0;

    if (!spn_str || !port_map) {
        return SOC_E_PARAM;
    }

    s = spn_str;

    /* Parse Front panel block type/number/port */
    /* Parse block type */
    port_map->line_port_info[lport].flow_control = SOC_SBX_CALADAN3_FC_TYPE_MAX;

    SOC_IF_ERROR_RETURN(_soc_sbx_caladan3_parse_block(unit, s, &port_map->line_port_info[lport], &n_chars));

    port_map->line_port_info[lport].uport = uport+1; /* uports start from index 1 */

    port_map->num_1g_ports += (sbx_caladan3_intf_attr[port_map->line_port_info[lport].intftype].speed == 1) ? 1 : 0;

    if (port_map->line_port_info[lport].blktype == SOC_BLK_CLPORT &&
        port_map->line_port_info[lport].instance > 0) {
        /* fabric side warpcore borrowed */
        port_map->line_port_info[lport].physical_intf = SOC_SBX_CALADAN3_FABRIC_WCORE;
        port_map->borrowed_wcore_count++;
    } else {
        port_map->line_port_info[lport].physical_intf = SOC_SBX_CALADAN3_LINE_WCORE;
    }
    /* Mark line entry as valid */
    port_map->line_port_info[lport].flags |= SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID;


    /* Parse fabric-side block type/number/port */
    s += n_chars;
    if (*s != ':') {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** ERROR: Bad mapping - expecting \":\" %s\n"),
                   unit, s));
        return SOC_E_PARAM;
    }

    s++;
    port_map->fabric_port_info[lport].flow_control = SOC_SBX_CALADAN3_FC_TYPE_MAX;

    SOC_IF_ERROR_RETURN(_soc_sbx_caladan3_parse_block(unit, s, &port_map->fabric_port_info[lport], &n_chars));

    if (port_map->fabric_port_info[lport].blktype == SOC_BLK_CLPORT &&
        port_map->fabric_port_info[lport].instance == 0) {
        /* line side warpcore borrowed */
        port_map->fabric_port_info[lport].physical_intf = SOC_SBX_CALADAN3_LINE_WCORE;
        port_map->borrowed_wcore_count++;
    } else {
        port_map->fabric_port_info[lport].physical_intf = SOC_SBX_CALADAN3_FABRIC_WCORE;
    }

    /* Mark line entry as valid */
    port_map->fabric_port_info[lport].flags |= SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID;
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     _soc_sbx_caladan3_resv_port_info_load
 * Purpose:
 * load reserved port parameters [cmic,xlports]
 *
 */
STATIC 
int _soc_sbx_caladan3_resv_port_info_load(int unit,
                                         soc_sbx_caladan3_port_map_t *port_map,
                                         int port)
{
    int i = 0;
    int last_line_port;

    /* Find the last valid line port */
    for (last_line_port = port; last_line_port >= 0; last_line_port--) {
        if (port_map->line_port_info[last_line_port].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID) {
            break;
        }
    }

    /* Potentially allocate consecutive queues based on squeue parameter */

    if (!port_map) {
        return SOC_E_PARAM;
    } else {

        /* allocate ucode port number */
        port_map->line_port_info[port].blk    = soc_sbx_block_find(unit, SOC_BLK_CMIC, 0);
        port_map->line_port_info[port].bindex = 0;
        port_map->line_port_info[port].instance = 0;
        port_map->line_port_info[port].intf_instance = 0;
        port_map->line_port_info[port].intftype = SOC_SBX_CALADAN3_PORT_INTF_CMIC;
        port_map->line_port_info[port].clienttype = SOC_SBX_CALADAN3_CLIENT_CMIC;
        port_map->line_port_info[port].blktype= SOC_BLK_CMIC;
        port_map->line_port_info[port].uport   = 0;
        port_map->line_port_info[port].port_queues.num_squeue = 0;
        port_map->line_port_info[port].flow_control = SOC_SBX_CALADAN3_FC_TYPE_NONE;
        
        port_map->line_port_info[port].port_queues.squeue_base = -1;
        port_map->line_port_info[port].port_queues.dqueue_base = -1;
        port_map->line_port_info[port].flags |= SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID;

        if (last_line_port >= 0) {
            sal_memcpy(&port_map->fabric_port_info[port], 
                       &port_map->fabric_port_info[last_line_port],
                       sizeof(port_map->line_port_info[port]));
            port_map->line_port_info[port].uport   = 0;
            port_map->fabric_port_info[port].port_queues.num_squeue = 0;
            port_map->fabric_port_info[port].port_queues.num_dqueue = 0;
            port_map->fabric_port_info[port].flags |= SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID;
        }
        soc_sbx_caladan3_port_queues_init(unit, &port_map->line_port_info[port]);
        soc_sbx_caladan3_port_queues_init(unit, &port_map->fabric_port_info[port]);
        

        port++;

        for (i=0; i<SOC_SBX_CALADAN3_MAX_XLPORT_PORT; i++) {
            port_map->line_port_info[port].blk    = soc_sbx_block_find(unit, SOC_BLK_XLPORT, 0);
            port_map->line_port_info[port].bindex = i;
            port_map->line_port_info[port].instance = 0;
            if (SOC_SBX_CALADAN3_MAX_XLPORT_PORT > 2) {
                port_map->line_port_info[port].intf_instance = i / 2;
            } else  {
                port_map->line_port_info[port].intf_instance = i ;
            }
            port_map->line_port_info[port].intftype = SOC_SBX_CALADAN3_PORT_INTF_XLPORT;
            port_map->line_port_info[port].clienttype = SOC_SBX_CALADAN3_CLIENT_XL;
            port_map->line_port_info[port].blktype= SOC_BLK_XLPORT;
            port_map->line_port_info[port].uport   = port+i;
            port_map->line_port_info[port].port_queues.num_squeue = 0;
            port_map->line_port_info[port].flow_control = SOC_SBX_CALADAN3_FC_TYPE_PAUSE;
            port_map->line_port_info[port].physical_intf = SOC_SBX_CALADAN3_LINE_WCORE;
            port_map->line_port_info[port].port_queues.squeue_base = -1;
            port_map->line_port_info[port].port_queues.dqueue_base = -1;
            port_map->line_port_info[port].flags |= SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID;

            if (last_line_port >= 0) {
                /* for now use the first fabric port channel */
                sal_memcpy(&port_map->fabric_port_info[port], 
                           &port_map->fabric_port_info[port-1],
                           sizeof(port_map->fabric_port_info[port]));
                port_map->line_port_info[port].uport   = port+i;
                port_map->fabric_port_info[port].port_queues.num_squeue = 0;
                port_map->fabric_port_info[port].port_queues.num_dqueue = 0;
                port_map->fabric_port_info[port].flags |= SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID;
            }
            soc_sbx_caladan3_port_queues_init(unit, &port_map->line_port_info[port]);
            soc_sbx_caladan3_port_queues_init(unit, &port_map->fabric_port_info[port]);
            port++;
        }
        port_map->num_1g_ports += SOC_SBX_CALADAN3_NUM_RESV_PORTS;
    }
    return SOC_E_NONE;
}



int
soc_sbx_caladan3_base_port_config(int unit, 
                                  soc_sbx_caladan3_port_map_t *port_map)
{
    soc_port_t lport, phy_port;
    int blk, bindex, blktype;
    soc_sbx_caladan3_port_map_info_t *resv_port = NULL;
    soc_sbx_caladan3_port_map_info_t *port_info;
    sbx_caladan3_intf_attr_t *intf_attr;
    int p, intf_type;
    int idx;
    int line = 1;
    soc_info_t *si;

    si = &SOC_INFO(unit);

    SOC_PBMP_ITER(PBMP_ALL(unit),lport) {
        if (soc_feature(unit, soc_feature_logical_port_num)) {
            phy_port = si->port_l2p_mapping[lport];
        } else {
            phy_port = lport;
        }
        if (phy_port < 0) {
            break;
        }
        blk = SOC_PORT_IDX_BLOCK(unit, phy_port, 0);
        bindex = SOC_PORT_IDX_BINDEX(unit, phy_port, 0);
        if (blk < 0 && bindex < 0) { 
            break;
        }
        blktype = SOC_BLOCK_INFO(unit, blk).type;

        for (idx =0; idx < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; idx++) {
            if (!(port_map->line_port_info[idx].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
                continue;
            }
            if (port_map->line_port_info[idx].port == lport) {
                line = 1;
                break;
            } else if (port_map->fabric_port_info[idx].port == lport) {
                line = 0;
                break;
            }
        }
        if (idx >= SOC_SBX_CALADAN3_PORT_MAP_ENTRIES) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, *** ERROR: Failed mapping PBMP port (%d)\n"),
                       unit, lport));
            return SOC_E_NOT_FOUND;
        }


        switch (blktype) {
        case SOC_BLK_CMIC:
            port_info = &port_map->line_port_info[idx];
            port_info->port = lport;
            port_info->base_port = SOC_SBX_CALADAN3_SWS_CMIC_PORT_BASE;
            break;
        case SOC_BLK_CLPORT:
        case SOC_BLK_IL:
            if (line) {
                port_info = &port_map->line_port_info[idx];
                port_info->port = lport;
                port_info->base_port = bindex;
            } else {
                port_info = &port_map->fabric_port_info[idx];
                for (p =idx; p < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; p++) {
                    port_info = &port_map->fabric_port_info[p];
                    if (!(port_info->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
                        continue;
                    }
                    port_info->base_port = bindex;
                }
                /* Use the last XL port */
                resv_port = &port_map->fabric_port_info[SOC_SBX_CALADAN3_MAX_LINE_PORT+4];
            }
            break;
        case SOC_BLK_XTPORT:
            port_info = &port_map->line_port_info[idx];
            intf_type = port_info->intftype;
            intf_attr = &sbx_caladan3_intf_attr[intf_type];
            port_info->port = lport;
            bindex = 12 + bindex + port_info->instance * intf_attr->max_port_per_instance;
            port_info->base_port = bindex;
            break;
        case SOC_BLK_XLPORT:
            port_info = &port_map->line_port_info[idx];
            port_info->port = lport;
            if (SOC_SBX_CALADAN3_MAX_XLPORT_PORT > 2) {
                bindex = SOC_SBX_CALADAN3_SWS_XL_PORT_BASE + bindex/2;
            } else {
                bindex = SOC_SBX_CALADAN3_SWS_XL_PORT_BASE + bindex;
            }
            port_info->base_port = bindex;
            break;
        }
    }

    if (resv_port) {
        for (idx = port_map->first_reserved_port;idx < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; idx++) {
                if (!(port_map->fabric_port_info[idx].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
                    continue;
                }
                port_info = &port_map->fabric_port_info[idx];
                port_info->base_port = resv_port->base_port;
        }
    }
    return SOC_E_NONE;
}

#define SBX_PBMP_PORT_NUMBER(type) si->type.num

/*
 *
 * Function:
 *     _soc_sbx_caladan3_port_pbmp_config_port
 * Purpose:
 * populate port pbmp and other tables for a single port.
 *
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Invalid string value for ucode property
 */

STATIC
int _soc_sbx_caladan3_port_pbmp_config_port(int unit,
                                            int lport,
                                            soc_sbx_caladan3_port_map_t *port_map)
{
    soc_info_t *si;
    int         phy_port;
    int         blk, bindex, idx, blktype;
    char        *pname;
    int         pno = 0, intf_type;
    int         line = 1;

    si  = &SOC_INFO(unit);


    if (soc_feature(unit, soc_feature_logical_port_num)) {
        phy_port = si->port_l2p_mapping[lport];
    } else {
        phy_port = lport;
    }
    if (phy_port < 0) {
        return SOC_E_NONE;
    }

    blk = SOC_PORT_IDX_BLOCK(unit, phy_port, 0);
    bindex = SOC_PORT_IDX_BINDEX(unit, phy_port, 0);
    if (blk < 0 && bindex < 0) { /* end of regsfile port list */
        return SOC_E_NONE;
    }

    blktype = SOC_BLOCK_INFO(unit, blk).type;

    pname = "?";

    for (idx =0; idx < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; idx++) {
        if (!(port_map->line_port_info[idx].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }
        if (port_map->line_port_info[idx].port == lport) {
            line = 1;
            break;
        } else if (port_map->fabric_port_info[idx].port == lport) {
            line = 0;
            break;
        }
    }
    if (idx >= SOC_SBX_CALADAN3_PORT_MAP_ENTRIES) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, *** ERROR: Failed mapping PBMP port (%d)\n"),
                   unit, lport));
        return SOC_E_NOT_FOUND;
    }

    switch (blktype) {
    case SOC_BLK_CMIC:
        pno = 0;
        si->cmic_port = lport;
        si->cmic_block = SOC_BLK_CMIC;
        SOC_PBMP_PORT_ADD((si->cmic_bitmap), lport);
        intf_type = port_map->line_port_info[idx].intftype;
        si->port_speed_max[lport] = sbx_caladan3_intf_attr[intf_type].speed * SOC_SBX_CALADAN3_GIG_SPEED;
        pname = "cpu";
        break;
    case SOC_BLK_CLPORT:
        /* if line ports, set appropriate bit map */
        if (line) {
            if (port_map->line_port_info[idx].flow_control 
                     == SOC_SBX_CALADAN3_FC_TYPE_MAX) {
                port_map->line_port_info[idx].flow_control = 
                         SOC_SBX_CALADAN3_FC_TYPE_PAUSE;
            }
            intf_type = port_map->line_port_info[idx].intftype;
            si->port_num_lanes[lport] = sbx_caladan3_intf_attr[intf_type].wcore_lines_per_port;
            si->port_speed_max[lport] = sbx_caladan3_intf_attr[intf_type].speed * SOC_SBX_CALADAN3_GIG_SPEED;

            switch(port_map->line_port_info[idx].intftype) {
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE:
                pno = SBX_PBMP_PORT_NUMBER(ce);
                SBX_ADD_PORT(ce, lport); 
                /*si->port_speed_max[lport] = 100000;*/
                pname = "ce";
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_10GE:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_XAUI_10GE:
                pno = SBX_PBMP_PORT_NUMBER(xe);
                SBX_ADD_PORT(xe, lport);
                pname = "xe";
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_1GE:
                pno = SBX_PBMP_PORT_NUMBER(ge);
                SBX_ADD_PORT(ge, lport); 
                pname = "ge";
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42:
                pno = SBX_PBMP_PORT_NUMBER(hg);
                SBX_ADD_PORT(hg, lport);
                SOC_HG2_ENABLED_PORT_ADD(unit, lport);
                pname = "hg";
                break;
            default:
                assert(0);
                break;
            } 
            SBX_ADD_PORT(ether, lport);
            SBX_ADD_PORT(cl, lport);
        } else {
            /* fabric interface */
            if (port_map->fabric_port_info[idx].flow_control 
                     == SOC_SBX_CALADAN3_FC_TYPE_MAX) {
                port_map->fabric_port_info[idx].flow_control = 
                         SOC_SBX_CALADAN3_FC_TYPE_PAUSE;
            }
            pno = SBX_PBMP_PORT_NUMBER(hg);
            SBX_ADD_PORT(cl, lport);
            SBX_ADD_PORT(hg, lport);
            SOC_HG2_ENABLED_PORT_ADD(unit, lport);
            intf_type = port_map->fabric_port_info[idx].intftype;
            si->port_num_lanes[lport] = sbx_caladan3_intf_attr[intf_type].wcore_lines_per_port;
            si->port_speed_max[lport] = sbx_caladan3_intf_attr[intf_type].speed * SOC_SBX_CALADAN3_GIG_SPEED;
            pname = "hg";
        }
        break;
    case SOC_BLK_IL:
        pno = SBX_PBMP_PORT_NUMBER(il);

        if (line) {
            intf_type = port_map->line_port_info[idx].intftype;
        } else {
            intf_type = port_map->fabric_port_info[idx].intftype;
        }

        SBX_ADD_PORT(il, lport);
        pname = "il";
        si->port_num_lanes[lport] = sbx_caladan3_intf_attr[intf_type].wcore_lines_per_port;
        si->port_speed_max[lport] = sbx_caladan3_intf_attr[intf_type].speed;
        if (port_map->line_port_info[idx].flow_control == SOC_SBX_CALADAN3_FC_TYPE_MAX) {
            port_map->line_port_info[idx].flow_control = SOC_SBX_CALADAN3_FC_TYPE_ILKN;
        }
        break;
    case SOC_BLK_XTPORT:
        pno = SBX_PBMP_PORT_NUMBER(ge);
        SBX_ADD_PORT(ge, lport); 
        SBX_ADD_PORT(xt, lport); 
        SBX_ADD_PORT(ether, lport);
        pname = "ge";
        intf_type = port_map->line_port_info[idx].intftype;
        si->port_num_lanes[lport] = sbx_caladan3_intf_attr[intf_type].wcore_lines_per_port;
        si->port_speed_max[lport] = sbx_caladan3_intf_attr[intf_type].speed * SOC_SBX_CALADAN3_GIG_SPEED;
        if (port_map->line_port_info[idx].flow_control == SOC_SBX_CALADAN3_FC_TYPE_MAX) {
            port_map->line_port_info[idx].flow_control = SOC_SBX_CALADAN3_FC_TYPE_PAUSE;
        }
        break;
    case SOC_BLK_XLPORT:
        pno = SBX_PBMP_PORT_NUMBER(ge);
        SBX_ADD_PORT(ge, lport); 
        SBX_ADD_PORT(xl, lport); 
        SBX_ADD_PORT(ether, lport);
        intf_type = port_map->line_port_info[idx].intftype;
        si->port_num_lanes[lport] = sbx_caladan3_intf_attr[intf_type].wcore_lines_per_port;
        si->port_speed_max[lport] = sbx_caladan3_intf_attr[intf_type].speed * SOC_SBX_CALADAN3_GIG_SPEED;
        if (port_map->line_port_info[idx].flow_control == SOC_SBX_CALADAN3_FC_TYPE_MAX) {
            port_map->line_port_info[idx].flow_control = SOC_SBX_CALADAN3_FC_TYPE_PAUSE;
        }
        pname = "ge";
        break;
    default:
        pname = "?";
        break;
    }

    si->block_valid[blk] += 1;
    if (si->block_port[blk] < 0) {
        si->block_port[blk] = lport;
    }
    sal_snprintf(si->port_name[lport], sizeof(si->port_name[lport]), "%s%d", pname, pno);
    SOC_PBMP_PORT_ADD(si->block_bitmap[blk], lport);
    SBX_ADD_PORT(all, lport);
    if (blktype != SOC_BLK_CMIC) {
        SBX_ADD_PORT(port, lport);
    }
    si->port_type[lport] = blktype;

    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     _soc_sbx_caladan3_port_pbmp_config
 * Purpose:
 * populate port pbmp and other tables for all ports.
 *
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Invalid string value for ucode property
 */
STATIC
int _soc_sbx_caladan3_port_pbmp_config(int unit, soc_sbx_caladan3_port_map_t *port_map)
{
    soc_info_t  *si;
    int         lport, phy_port;
    soc_pbmp_t  pbmp_seen;
    int         ucode_num_port_override;
    int         blk, bindex;

    si  = &SOC_INFO(unit);

    if (!port_map) {
        return SOC_E_PARAM;
    }

    for (lport = 0; lport < SOC_MAX_NUM_BLKS; lport++) {
        si->block_port[lport] = REG_PORT_ANY;
    }

    SOC_PBMP_CLEAR(pbmp_seen);
    for (lport = 0; ; lport++) {

        if (soc_feature(unit, soc_feature_logical_port_num)) {
            phy_port = si->port_l2p_mapping[lport];
        } else {
            phy_port = lport;
        }
        if (phy_port == -2) {
            continue;
        }
        if (phy_port < 0) {
            break;
        }

        /* if hg10 and multiple channels per port, forget you have seen this physical port */
        ucode_num_port_override = soc_property_get(unit, "ucode_num_port_override", 0);
        if (!ucode_num_port_override) {
            if (SOC_PBMP_MEMBER(pbmp_seen, phy_port)) {
                continue;
            }
        }

        SOC_PBMP_PORT_ADD(pbmp_seen, phy_port);
        blk = SOC_PORT_IDX_BLOCK(unit, phy_port, 0);
        bindex = SOC_PORT_IDX_BINDEX(unit, phy_port, 0);
        if (blk < 0 && bindex < 0) { /* end of regsfile port list */
            break;
        }

        _soc_sbx_caladan3_port_pbmp_config_port(unit, lport, port_map);

    }

    /* CPU Port */
    si->ipic_port = -1;

    for (lport = 0; lport<SOC_MAX_NUM_BLKS; lport++) {
        /* Override number of lanes if rxaui mode - must do after name is set up for lport  so "xe" type works */
        if (soc_property_port_get(unit, lport, spn_SERDES_RXAUI_MODE, 0)) {
            /* remove unused odd numbered xe ports from pbmp which are configured as rxaui */
            si->port_num_lanes[lport] = 2; /* override lanes setting */
        }
    }

    soc_sbx_caladan3_base_port_config(unit, port_map);
    return SOC_E_NONE;

}




/*
 *
 * Function:
 *     _soc_sbx_caladan3_get_phy_port
 * Purpose:
 * Given logical port number return mapped to physical port
 *
 * Returns:
 *     SOC_E_NONE - Success
 */
static
int _soc_sbx_caladan3_get_phy_port(int unit,
                                   soc_sbx_caladan3_port_map_info_t *log_port_info,
                                   int *phy_port)
{
    soc_driver_t *driver=NULL;
    soc_port_info_t *phy_port_info=NULL;
    int index=0;

    driver = SOC_DRIVER(unit);

    if (!soc_feature(unit, soc_feature_logical_port_num)) {
        return SOC_E_UNAVAIL;
    }

    if (!log_port_info || !phy_port) {
        return SOC_E_PARAM;
    }

    *phy_port = -1;

    for (index=0; *phy_port < 0; index++) {
        phy_port_info = &driver->port_info[index];

        if (phy_port_info->blk < 0) { /* End of list */
            break;
        }

        /* HIGIG to physical port number mapping */
        /* - HG-126 - if interface type is HG126 always map to first physical port on
                    the CMAC 
           - HG-42 - map HG42 to first port of XMAC.
           - HG-25 - map them to port 0 or port 2 of XMAC. XMAC are on dual channel mode
        */


        if (SOC_SBX_IS_HG_INTF_TYPE(log_port_info->intftype)) {
            switch(log_port_info->intftype) {

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126:
                if (log_port_info->blk == phy_port_info->blk &&
                    phy_port_info->bindex == 0) {
                    *phy_port = index;
                }
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42:
                if ((log_port_info->blk == phy_port_info->blk) &&
                    (log_port_info->intf_instance *_HG42_PHY_PORT_OFFSET_ == 
                     phy_port_info->bindex)) {
                    *phy_port = index;
                }
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25:
                if ((log_port_info->blk == phy_port_info->blk) &&
                    (log_port_info->intf_instance *_HG25_PHY_PORT_OFFSET_ == 
                     phy_port_info->bindex)) {
                    *phy_port = index;
                }
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10:
                if ((log_port_info->blk == phy_port_info->blk) &&
                    (log_port_info->bindex + 
                        (log_port_info->intf_instance *_HG10_PHY_PORT_OFFSET_) == 
                     phy_port_info->bindex)) {
                    *phy_port = index;
                }
                break;

            default:
                assert(0);
                break;
            }

        } else if (SOC_SBX_IS_ILKN_INTF_TYPE(log_port_info->intftype)) {
            switch(log_port_info->intftype) {

            case SOC_SBX_CALADAN3_PORT_INTF_IL100:
            case SOC_SBX_CALADAN3_PORT_INTF_IL50w:
            case SOC_SBX_CALADAN3_PORT_INTF_IL50n:
                if (log_port_info->blk == phy_port_info->blk &&
                    phy_port_info->bindex == 0) { 
                    *phy_port = index;
                }
                break;  

            default:
                assert(0);
                break;  
            }

        } else {
            switch(log_port_info->intftype) {

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE:
                if ((log_port_info->blk == phy_port_info->blk) && 
                     (log_port_info->intf_instance *
                      _40GE_PHY_PORT_OFFSET_ == 
                      phy_port_info->bindex)) {
                    *phy_port = index;
                }
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_10GE:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_XAUI_10GE:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_1GE:
                if ((log_port_info->blk == phy_port_info->blk) && 
                     (log_port_info->intf_instance *
                      _40GE_PHY_PORT_OFFSET_ + log_port_info->bindex == 
                      phy_port_info->bindex)) {
                    *phy_port = index;
                }
                break;

            default:
                if ((log_port_info->blk == phy_port_info->blk) && 
                    (log_port_info->bindex == phy_port_info->bindex)) {
                    *phy_port = index;
                }
                break;
            }
        }
    }

    if (*phy_port < 0) return SOC_E_NOT_FOUND;

    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_drv_port_map_cfg
 * Purpose:
 * populate port mapping information 
 *
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Invalid string value for ucode property
 */
STATIC
int _soc_sbx_caladan3_drv_port_map_cfg(int unit,
                                       soc_sbx_caladan3_port_map_t *port_map)
{
    soc_info_t          *si;
    int idx, index, fabidx, max_ports;
    int rv=SOC_E_NONE;


    si  = &SOC_INFO(unit);

    if (!port_map) {
        return SOC_E_PARAM;
    }

    for (index = 0; index <  SOC_MAX_NUM_PORTS; index++) {
        si->port_p2l_mapping[index] = -1;
        si->port_l2p_mapping[index] = -1;
    } 

    max_ports = port_map->max_port * 2;


    if (max_ports >= SOC_MAX_NUM_PORTS) return SOC_E_PARAM;

    /* Process the external line ports */
    for (idx=0, index=0; idx < port_map->first_reserved_port; idx++, index++) {

        if (!(port_map->line_port_info[idx].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            si->port_l2p_mapping[index] = -2;
            continue;
        }
        rv = _soc_sbx_caladan3_get_phy_port(unit, 
                                            &port_map->line_port_info[idx],
                                            &si->port_l2p_mapping[index]);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, *** ERROR: _soc_sbx_caladan3_get_phy_port failed for line\n"),
                       unit));
            return rv;
        }

        si->port_speed_max[index] = sbx_caladan3_intf_attr[port_map->line_port_info[idx].intftype].speed *
                                      SOC_SBX_CALADAN3_GIG_SPEED;

        /* Only write if not initialized - if channelized, we only want the P2L mapping for the first channel */
        /* So a physical port is only mapped to the base pbmp port for the channelized port */
        if (si->port_p2l_mapping[si->port_l2p_mapping[index]] == -1) {
            si->port_p2l_mapping[si->port_l2p_mapping[index]] = index;
            port_map->max_line_bw += sbx_caladan3_intf_attr[port_map->line_port_info[idx].intftype].speed;
        }
        si->port_type[index] = port_map->line_port_info[idx].blktype;

        port_map->line_port_info[idx].port = index;
        port_map->pbmp2idx[index].portidx = idx;
        port_map->pbmp2idx[index].line = TRUE;

    }

   /* Mark logical to physical port mappings up to 47, if any, as unused */
    for (; index < port_map->first_reserved_port; index++) {
        si->port_l2p_mapping[index] = -2;
    }

    /* Process the CMIC and XL reserved ports */
    for (idx=port_map->first_reserved_port;
              idx < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; idx++) {

        if (!(port_map->line_port_info[idx].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }
        rv = _soc_sbx_caladan3_get_phy_port(unit, 
                                            &port_map->line_port_info[idx],
                                            &si->port_l2p_mapping[index]);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, *** ERROR: _soc_sbx_caladan3_get_phy_port failed for line\n"),
                       unit));
            return rv;
        }
        si->port_speed_max[index] = sbx_caladan3_intf_attr[port_map->line_port_info[idx].intftype].speed *
                                      SOC_SBX_CALADAN3_GIG_SPEED;

        port_map->max_line_bw += sbx_caladan3_intf_attr[port_map->line_port_info[idx].intftype].speed;
        si->port_p2l_mapping[si->port_l2p_mapping[index]] = index;
        si->port_type[index] = port_map->line_port_info[idx].blktype;

        port_map->line_port_info[idx].port = index;
        port_map->pbmp2idx[index].portidx = idx;
        port_map->pbmp2idx[index].line = TRUE;

        index++;
    }

    /* Process the fabric port(s) */
    for (idx=0; idx < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; idx++) {

        if (!(port_map->fabric_port_info[idx].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }
        fabidx = index + port_map->fabric_port_info[idx].intf_instance;
        rv = _soc_sbx_caladan3_get_phy_port(unit, 
                                            &port_map->fabric_port_info[idx],
                                            &si->port_l2p_mapping[fabidx]);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, *** ERROR: _soc_sbx_caladan3_get_phy_port failed for fabric\n"),
                       unit));
            return rv;
        }
        
        si->port_speed_max[fabidx] =
            sbx_caladan3_intf_attr[port_map->fabric_port_info[idx].intftype].speed *
            SOC_SBX_CALADAN3_GIG_SPEED;

        /* note: for higig ports a give physical port is mapped to multiple logic port.
         * This could be modified if essential */
        if (si->port_p2l_mapping[si->port_l2p_mapping[fabidx]] < 0) {
            si->port_p2l_mapping[si->port_l2p_mapping[fabidx]] = fabidx;
            port_map->max_fabric_bw += sbx_caladan3_intf_attr[port_map->fabric_port_info[idx].intftype].speed;
        }
        si->port_type[fabidx]    = port_map->fabric_port_info[idx].blktype;

        port_map->fabric_port_info[idx].port = fabidx;
        port_map->pbmp2idx[fabidx].portidx = idx;
        port_map->pbmp2idx[fabidx].line = FALSE;

    }

    rv = _soc_sbx_caladan3_port_pbmp_config(unit, port_map);

    /* 
     * IL only case, need mapping to CLPORT for WC access
     * This is internal mapping and should not be exposed to BCM layer
     * (Hint: should not be moved above the pbmp_config line)
     * The Last 2 port indexes are used for this purpose
     */

    if (SOC_PBMP_NOT_NULL(si->il.bitmap)) {
        index = SOC_SBX_CALADAN3_CLPORT0_L2P_IDX;
        if (si->port_p2l_mapping[SOC_SBX_CALADAN3_CLPORT0_PHY_PORT] < 0) {
            si->port_l2p_mapping[index] = SOC_SBX_CALADAN3_CLPORT0_PHY_PORT; 
            si->port_p2l_mapping[si->port_l2p_mapping[index]] = index;
            si->port_type[index] = SOC_BLK_CLPORT;
            si->port_speed_max[index] = 100*SOC_SBX_CALADAN3_GIG_SPEED;
            si->block_port[SOC_PORT_BLOCK(unit, SOC_SBX_CALADAN3_CLPORT0_PHY_PORT)] = index;
        }
        index = SOC_SBX_CALADAN3_CLPORT1_L2P_IDX;
        if (si->port_p2l_mapping[SOC_SBX_CALADAN3_CLPORT1_PHY_PORT] < 0) {
            si->port_l2p_mapping[index] = SOC_SBX_CALADAN3_CLPORT1_PHY_PORT; 
            si->port_p2l_mapping[si->port_l2p_mapping[index]] = index;
            si->port_type[index] = SOC_BLK_CLPORT;
            si->port_speed_max[index] = 100*SOC_SBX_CALADAN3_GIG_SPEED;
            si->block_port[SOC_PORT_BLOCK(unit, SOC_SBX_CALADAN3_CLPORT1_PHY_PORT)] = index;
        }
    }

    return rv;

}

/*
 * Function:
 *     soc_sbx_caladan3_port_oob_fc_config
 * Purpose
 *     Routine to be called to setup HCFC OOB flow control when there is no ILKN port
 *     Enables the OOB part of the ILKN to work in HCFC mode
 */
int
soc_sbx_caladan3_port_oob_fc_config(int unit, int line, int fc_type)
{
    soc_info_t *si;
    int lp = 0, fp = 0;
    int index, port;

    si = &SOC_INFO(unit);

     /* If line is set, then enables ILKN interface for OOB support 
      * Otherwise enables fabric side ILKN, Does nothing if the ilkn is already enabled
      * This routine should be called only when hcfc oob is enabled 
      * and no interlaken port is found
      */
    if (SOC_PBMP_NOT_NULL(si->il.bitmap)) {
        PBMP_ITER(si->il.bitmap, port) {
            if (soc_sbx_caladan3_is_line_port(unit, port)) {
                lp = 1;
            } else {
                fp = 1;
            }
        }
    }
    if (line) {
        if (!lp) {
            index = SOC_SBX_CALADAN3_LINE_OOB_L2P_IDX;
            if (si->port_p2l_mapping[SOC_SBX_CALADAN3_IL0_PHY_PORT] < 0) {
                si->port_l2p_mapping[index] = SOC_SBX_CALADAN3_IL0_PHY_PORT; 
                si->port_p2l_mapping[si->port_l2p_mapping[index]] = index;
                si->port_type[index] = SOC_BLK_IL;
                si->block_port[SOC_PORT_BLOCK(unit, SOC_SBX_CALADAN3_IL0_PHY_PORT)] = index;
            }
        }
        soc_sbx_caladan3_il_oob_init(unit, 0, fc_type);
    } else {
        if (!fp) {
            index = SOC_SBX_CALADAN3_FAB_OOB_L2P_IDX;
            if (si->port_p2l_mapping[SOC_SBX_CALADAN3_IL1_PHY_PORT] < 0) {
                si->port_l2p_mapping[index] = SOC_SBX_CALADAN3_IL1_PHY_PORT; 
                si->port_p2l_mapping[si->port_l2p_mapping[index]] = index;
                si->port_type[index] = SOC_BLK_CLPORT;
                si->block_port[SOC_PORT_BLOCK(unit, SOC_SBX_CALADAN3_IL1_PHY_PORT)] = index;
            }
        }
        soc_sbx_caladan3_il_oob_init(unit, 1, fc_type);
    }
    return SOC_E_NONE;
}


/*
 *
 * Function:
 *     soc_sbx_caladan3_port_info_config
 * Purpose:
 * populate port information 
 *
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Invalid string value for ucode property
 */
STATIC
int _soc_sbx_caladan3_port_info_config(int unit, soc_sbx_caladan3_port_map_t *port_map)
{
    soc_port_info_t    *port_info;
    soc_driver_t        *driver;
    soc_info_t          *si;
    static soc_port_info_t *default_port_info = NULL;
    int index, max_ports;
    int pidx;

    driver = SOC_DRIVER(unit);
    si  = &SOC_INFO(unit);

    if (!port_map) {
        return SOC_E_PARAM;
    }

    if (soc_feature(unit, soc_feature_logical_port_num)) {
        return _soc_sbx_caladan3_drv_port_map_cfg(unit, port_map);
    }

    max_ports = port_map->max_port * 2;

    port_info = sal_alloc(sizeof(soc_port_info_t) * SOC_SBX_CALADAN3_PORT_MAP_ENTRIES,
        "c3_port_info_config");
    if (port_info == NULL) {
        return SOC_E_MEMORY;
    }
    /* coverity[bad_sizeof] */
    sal_memset(port_info, 0, sizeof(sizeof(soc_port_info_t) * SOC_SBX_CALADAN3_PORT_MAP_ENTRIES));

    for (index = 0; index < SOC_MAX_NUM_BLKS; index++) {
        si->block_port[index] = REG_PORT_ANY;
    }

    for(index=0; index < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; index++) {

        if (!(port_map->line_port_info[index].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }
        port_info[index].blk = port_map->line_port_info[index].blk;
        port_info[index].bindex = port_map->line_port_info[index].bindex;
        si->port_type[index]    = port_map->line_port_info[index].blktype;

        si->port_speed_max[index] = 
            sbx_caladan3_intf_attr[port_map->line_port_info[index].intftype].speed *
                SOC_SBX_CALADAN3_GIG_SPEED;

        if (si->block_port[port_info[index].blk] == REG_PORT_ANY) {
            si->block_port[port_info[index].blk] = index;
        }

        pidx = index+port_map->max_port;
        port_info[pidx].blk = port_map->fabric_port_info[index].blk;
        port_info[pidx].bindex = port_map->fabric_port_info[index].bindex;
        si->port_type[pidx]    = port_map->fabric_port_info[index].blktype;
        if (si->block_port[port_info[pidx].blk] == REG_PORT_ANY) {
            si->block_port[port_info[pidx].blk] = pidx;
        }
    } 

    /* end of list node */
    port_info[max_ports-1].blk = -1;
    port_info[max_ports-1].bindex = -1;

    

    
    /* allocate a new port infor strucutre */
    if (default_port_info == NULL) {
        default_port_info = driver->port_info;
    }

    /* Load port mapping into 'port_info' and 'block_info' device driver */
    /* free old array if its not the default */
    if (driver->port_info &&
        (driver->port_info != default_port_info))
    {
        sal_free(driver->port_info);
        driver->port_info = NULL;
    }

    /* set defaults first */
    driver->port_info = default_port_info;

    driver->port_info = port_info;

    return _soc_sbx_caladan3_port_pbmp_config(unit, port_map);
}


/*
 *
 * Function:
 *     soc_sbx_caladan3_reconfig_port_cleanup
 * Purpose:
 *     During reconfiguration clean up port state for ports
 * that are no longer present. Three things are done:
 *  1) Reset the warp core
 *  2) Clear the appropriate port enable bit
 *  3) Set the appropriate core port mode to disabled.
 * This function must be called before the new TDM
 * is processed by the recnfiguration process.
 *
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_INTERNAL  - Called before port map table exists
 */
int soc_sbx_caladan3_reconfig_port_cleanup(int unit)
{
    soc_sbx_caladan3_port_map_t         *port_map = NULL;
    soc_port_t                          port;
    int                                 subport;
    char                                *spn_str;
    soc_sbx_caladan3_port_map_info_t    port_info;
    int                                 nchar;

    port_map = _caladan3_port_map[unit];
    if (port_map == NULL) {
        return SOC_E_INTERNAL;
    }

    for (port = 0; port < SOC_SBX_CALADAN3_MAX_LINE_PORT; port++) {

        if (!(port_map->line_port_info[port].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }

        if (port_map->line_port_info[port].intftype ==
                    SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE) {
            break;
        }

        spn_str = soc_property_port_get_str(unit, port, spn_UCODE_PORT);

        if (spn_str != NULL) {
            sal_memset(&port_info, 0, sizeof(soc_sbx_caladan3_port_map_info_t));
            _soc_sbx_caladan3_parse_block(unit, spn_str, &port_info, &nchar);
        }

        if (spn_str == NULL ||
            port_info.clienttype != port_map->line_port_info[port].clienttype ||
            port_info.instance != port_map->line_port_info[port].instance ||
            port_info.intftype != port_map->line_port_info[port].intftype ||
            port_info.intf_instance != port_map->line_port_info[port].intf_instance ||
            port_info.bindex != port_map->line_port_info[port].bindex) {

            subport = (port_map->line_port_info[port].intf_instance << 2) +
                port_map->line_port_info[port].bindex;
            soc_sbx_caladan3_port_enable(unit, port, subport, 0);

        }
    }

    return SOC_E_NONE;
}


/*
 *
 * Function:
 *     soc_sbx_caladan3_calc_phy_mac_skip_ports
 * Purpose:
 *     Calculate the set of ports that are on warp
 *  cores that are on the CL0 block and are in use
 *  both before and after the hotswap. Set the port
 *  bit for all ports on these warp cores.
 *
 * Returns:
 *     SOC_E_NONE - Success
 */
int soc_sbx_caladan3_calc_phy_mac_skip_ports(int unit)
{
    int             port;
    int             pport;
    int             blk, cl0_blk;
    int             wcore;
    unsigned int    wcbits = 0;

    cl0_blk = soc_sbx_block_find(unit, SOC_BLK_CLPORT, 0);

    /* Find common set of warp cores in use on CL0 block */
    SOC_PBMP_ITER(SOC_CONTROL(unit)->all_skip_pbm, port) {
        if (soc_feature(unit, soc_feature_logical_port_num)) {
            pport = SOC_INFO(unit).port_l2p_mapping[port];
        } else {
            pport = port;
        }
        blk = SOC_PORT_BLOCK(unit, pport);
        if (blk == cl0_blk) {
            wcore = SOC_PORT_BINDEX(unit, pport) >> 2;
            wcbits |= (1 << wcore);
        } else {
            SOC_PBMP_PORT_ADD(SOC_CONTROL(unit)->mac_phy_skip_pbm, port);
        }

    }

    /* Add all ports on the common warp core(s) to mac phy skip list */
    PBMP_ALL_ITER(unit, port) { 
        if (soc_feature(unit, soc_feature_logical_port_num)) {
            pport = SOC_INFO(unit).port_l2p_mapping[port];
        } else {
            pport = port;
        }
        blk = SOC_PORT_BLOCK(unit, pport);
        if (blk == cl0_blk) {
            wcore = SOC_PORT_BINDEX(unit, pport) >> 2;
            if ((1 << wcore) & wcbits) {
                SOC_PBMP_PORT_ADD(SOC_CONTROL(unit)->mac_phy_skip_pbm, port);
            }
        }
    }

    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_port_info_load
 * Purpose:
 *     Parse ucode property value string into front-panel and
 *     fabric-side block information.
 *
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Invalid string value for ucode property
 */
int soc_sbx_caladan3_port_info_load(int unit)
{
    int  status = SOC_E_NONE;
    char *spn_str;
    int  port, ucode_num_ports, max_port, num_ports_found, ucode_num_port_override __attribute__((unused));
    soc_sbx_caladan3_port_map_t *port_map = NULL;
    uint8 *bindex_in_use = NULL;
    int bindex_intf_inst = 0;
    int lport;
    int port_inc;
    int pport;
    int blk, cl0_blk;
    int cl0_reset;
#if RECONFIG_DEBUG
    char                pfmt[SOC_PBMP_FMT_LEN];
#endif
 
    bindex_in_use = sal_alloc(sizeof(uint8) * SOC_SBX_CALADAN3_MAX_WCORE_LINE_COUNT, "bindex_in_use");
    if (bindex_in_use == NULL) {
        status = SOC_E_MEMORY;
        goto err;
    }

    sal_memset(bindex_in_use, FALSE, sizeof(uint8) * SOC_SBX_CALADAN3_MAX_WCORE_LINE_COUNT);

    port_map = sal_alloc(sizeof(soc_sbx_caladan3_port_map_t),"c3_port_map");
    if (port_map == NULL) {
        status = SOC_E_MEMORY;
        goto err;
    }

    sal_memset(port_map, 0, sizeof(soc_sbx_caladan3_port_map_t));

    port = 0;
    max_port = 0;
    num_ports_found = 0;
    ucode_num_ports = soc_property_get(unit, spn_UCODE_NUM_PORTS, 0);
    ucode_num_port_override = soc_property_get(unit, "ucode_num_port_override", 0);
    if (ucode_num_ports > 0) {
        for (port = 0; port < SBX_MAX_PORTS; port++) {
            spn_str = soc_property_port_get_str(unit, port, spn_UCODE_PORT);
            if (spn_str == NULL) {
                /* not found */
                continue;
            }
            max_port = port + 1;
            num_ports_found++;
        }

        if (num_ports_found != ucode_num_ports) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, *** ERROR: Found %d ucode_port connections, "
                                  "config.bcm has ucode_num_ports=%d\n"), 
                       unit, num_ports_found, ucode_num_ports));
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, *** ERROR: Ignoring ucode_num_ports configuration\n"),
                       unit));
            max_port = 1;
            ucode_num_ports = 0;
        } 
    }

    /* Allocate for port_info structure */
    /* Note: Add space in array for CMIC, XL0, XL1, EOF */
    port_map->num_ports_found = num_ports_found;

    if (max_port > 0) {
        max_port += SOC_SBX_CALADAN3_NUM_RESV_PORTS;
    }

    port_map->line_port_info = 
        sal_alloc((sizeof(soc_sbx_caladan3_port_map_info_t) *
            (SOC_SBX_CALADAN3_PORT_MAP_ENTRIES)), "line_port_map_info");
    if (port_map->line_port_info == NULL) {
        sal_free(port_map);
        status = SOC_E_MEMORY;
        goto err;
    }

    sal_memset(port_map->line_port_info, 0, 
         ((SOC_SBX_CALADAN3_PORT_MAP_ENTRIES) * sizeof(soc_sbx_caladan3_port_map_info_t)));

    port_map->fabric_port_info = 
        sal_alloc((sizeof(soc_sbx_caladan3_port_map_info_t) *
            (SOC_SBX_CALADAN3_PORT_MAP_ENTRIES)), "fabric_port_map_info");
    if (port_map->fabric_port_info == NULL) {
        sal_free(port_map->line_port_info);
        sal_free(port_map);
        status = SOC_E_MEMORY;
        goto err;
    }

    sal_memset(port_map->fabric_port_info, 0, 
         ((SOC_SBX_CALADAN3_PORT_MAP_ENTRIES) * sizeof(soc_sbx_caladan3_port_map_info_t)));

    port_map->max_port = max_port;
    port_map->first_reserved_port = SOC_SBX_CALADAN3_MAX_LINE_PORT;
    port_map->max_line_bw = 0;
    port_map->max_fabric_bw = 0;

    port_inc = soc_property_get(unit, "bcm88030_port_increment", 1);

    if (ucode_num_ports > 0) {
        /* parse configuration info */
        for (port = 0, lport = 0;
             port < SOC_SBX_CALADAN3_MAX_LINE_PORT && SOC_SUCCESS(status);
             port++, lport += port_inc) {

            if ((spn_str = soc_property_port_get_str(unit, port, spn_UCODE_PORT)) == NULL) {
                continue;
            }

            status = _soc_sbx_caladan3_ucode_get(unit, spn_str, port_map, port, lport);


            /* If we have already set up this block index and it is HG10, this is a channel on the port */
            if (port_map->line_port_info[lport].intftype == SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10) {

                bindex_intf_inst = ((port_map->line_port_info[lport].intf_instance) << 2) + 
                    (port_map->line_port_info[lport].bindex);

                if (bindex_in_use[bindex_intf_inst] == TRUE) {
                    port_map->line_port_info[lport].flags |= SOC_SBX_CALADAN3_IS_CHANNELIZED_SUBPORT;
                } else  {
                    port_map->line_port_info[lport].flags &= ~SOC_SBX_CALADAN3_IS_CHANNELIZED_SUBPORT;
                    bindex_in_use[bindex_intf_inst] = TRUE;
                }
                /* Each channel is always 1 cos level */
                port_map->line_port_info[lport].port_queues.num_squeue = 1;
            }

            /*Mark all but the first lport as a channelized port*/
            if (port_map->line_port_info[lport].intftype == SOC_SBX_CALADAN3_PORT_INTF_IL100 &&
                port > 0) {
                port_map->line_port_info[lport].flags |= SOC_SBX_CALADAN3_IS_CHANNELIZED_SUBPORT;
            }

        }

        if (SOC_FAILURE(status))
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, *** ERROR: Port Map Parsing failed\n"), unit));
    }

    if (num_ports_found > 0) {

        if (SOC_SUCCESS(status)) {
            /* Provision CMIC, XL, resv info */
            status = _soc_sbx_caladan3_resv_port_info_load(unit, port_map, port_map->first_reserved_port);
            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d, *** ERROR: Reserved Port load failed\n"), unit));
            } else {
                port_map->reserved_ports_configured = TRUE;
            }
        }

        if (SOC_SUCCESS(status)) {
            status = _soc_sbx_caladan3_port_info_config(unit, port_map);
            if (SOC_FAILURE(status))
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d, *** ERROR: PBMP setup failed\n"), unit));
        }

    }

    if (SOC_FAILURE(status)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, *** ERROR: Failed to initialize"
                              " Port mapping [FATAL] !!!!\n"), unit));
        goto err;
    } else {
        /* successful, update pointers & port map info */
        if(_caladan3_port_map[unit] != NULL) {
            sal_free(_caladan3_port_map[unit]->fabric_port_info);
            sal_free(_caladan3_port_map[unit]->line_port_info);
            sal_free(_caladan3_port_map[unit]);
        }
        port_map->intf_attr = &sbx_caladan3_intf_attr[0];
        _caladan3_port_map[unit] = port_map;
        SOC_SBX_CFG_CALADAN3(unit)->port_map = _caladan3_port_map[unit];
        if (SOC_SUCCESS(status)) {
#if 0
            /* Update queue mapping */
            SOC_PBMP_ITER(PBMP_ALL(unit),lport) {
                status = soc_sbx_caladan3_allocate_port_default_queues(unit, lport);
                if (SOC_FAILURE(status))
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d, *** ERROR: PBMP default queue alloc failed\n"), unit));
                status = soc_sbx_caladan3_update_queue_mapping(unit, lport);
                if (SOC_FAILURE(status))
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d, *** ERROR: PBMP queue mapping failed\n"), unit));
            }
#endif
        }
    }

    if (SOC_RECONFIG_TDM) {

#if RECONFIG_DEBUG
        LOG_CLI((BSL_META_U(unit,
                            "%s: Old ports:      %s\n"), __FUNCTION__, SOC_PBMP_FMT(SOC_CONTROL(unit)->all_skip_pbm, pfmt)));
#endif
        SOC_PBMP_AND(SOC_CONTROL(unit)->all_skip_pbm, PBMP_ALL(unit));
        SOC_PBMP_ITER(SOC_CONTROL(unit)->all_skip_pbm, port) {
            if (!soc_sbx_caladan3_is_line_port(unit, port)) {
                continue;
            }
            if (SOC_CONTROL(unit)->port_l2p_mapping[port] !=
                SOC_INFO(unit).port_l2p_mapping[port] ||
                SOC_CONTROL(unit)->intftype[port] !=
                port_map->line_port_info[port].intftype) {
                SOC_PBMP_PORT_REMOVE(SOC_CONTROL(unit)->all_skip_pbm, port);
#if RECONFIG_DEBUG
                LOG_CLI((BSL_META_U(unit,
                                    "%s: Port %d not the same - pruning\n"), __FUNCTION__, port));
#endif
            }
        }

        cl0_blk = soc_sbx_block_find(unit, SOC_BLK_CLPORT, 0);
        cl0_reset = TRUE;
        SOC_PBMP_ITER(SOC_CONTROL(unit)->all_skip_pbm, port) {
            if (soc_feature(unit, soc_feature_logical_port_num)) {
                pport = SOC_INFO(unit).port_l2p_mapping[port];
            } else {
                pport = port;
            }
            blk = SOC_PORT_BLOCK(unit, pport);
            if (blk == cl0_blk) {
                cl0_reset = FALSE;
                break;
            }
        }
        if (cl0_reset) {
            SOC_CONTROL(unit)->cl0_reset = TRUE;

            /* Remove all ports that are on the CL0 block */
            SOC_PBMP_ITER(SOC_CONTROL(unit)->all_skip_pbm, port) {
                if (soc_feature(unit, soc_feature_logical_port_num)) {
                    pport = SOC_INFO(unit).port_l2p_mapping[port];
                } else {
                    pport = port;
                }
                blk = SOC_PORT_BLOCK(unit, pport);
                if (blk == cl0_blk) {
                    SOC_PBMP_PORT_REMOVE(SOC_CONTROL(unit)->all_skip_pbm, port);
#if RECONFIG_DEBUG
                    LOG_CLI((BSL_META_U(unit,
                                        "%s: Pruning CL0 port %d\n"), __FUNCTION__, port));
#endif
                }
            }
        } else {
            SOC_CONTROL(unit)->cl0_reset = FALSE;

            soc_sbx_caladan3_calc_phy_mac_skip_ports(unit);
        }
#if RECONFIG_DEBUG
        LOG_CLI((BSL_META_U(unit,
                            "%s: Current ports:  %s\n"), __FUNCTION__, SOC_PBMP_FMT(PBMP_ALL(unit), pfmt)));
        LOG_CLI((BSL_META_U(unit,
                            "%s: Common ports:   %s\n"), __FUNCTION__, SOC_PBMP_FMT(SOC_CONTROL(unit)->all_skip_pbm, pfmt)));
        LOG_CLI((BSL_META_U(unit,
                            "%s: Phy skip ports: %s\n"), __FUNCTION__, SOC_PBMP_FMT(SOC_CONTROL(unit)->all_skip_pbm, pfmt)));
#endif

    }
    if (SOC_SUCCESS(status)) {
        goto cleanup;
    }
err:
    if (SOC_FAILURE(status)) {
        if (port_map) {
            if (port_map->fabric_port_info) {
                sal_free(port_map->fabric_port_info);            
            }
            if (port_map->line_port_info) {
                sal_free(port_map->line_port_info);           
            }
            sal_free(port_map);
        }
    }
cleanup:
    if (bindex_in_use) {
        sal_free(bindex_in_use);
    }
    return status;
}

/*
 * Function:
 *    soc_sbx_caladan3_get_max_ports
 * Purpose
 *    returns the total number of ucode ports
 */
int 
soc_sbx_caladan3_get_max_ports(int unit) 
{
    soc_sbx_caladan3_port_map_t *port_map;

    port_map = _caladan3_port_map[unit];
    return (port_map->max_port);
}

/*
 * Function:
 *    soc_sbx_caladan3_is_line_port
 * Purpose
 *    returns True for a Line port
 *    returns False for a Fabric port
 */
int 
soc_sbx_caladan3_is_line_port(int unit, soc_port_t port) 
{
    soc_sbx_caladan3_port_map_t *port_map;

    port_map = _caladan3_port_map[unit];
    return (port_map->pbmp2idx[port].line);
}

int soc_sbx_caladan3_fabric_port_client_type(int unit, soc_port_t port) 
{
    int i;
    soc_sbx_caladan3_port_map_t *port_map;

    port_map = _caladan3_port_map[unit];
    if (port_map->pbmp2idx[port].line == FALSE) {
        i = port_map->pbmp2idx[port].portidx;
        return port_map->fabric_port_info[i].clienttype;
    }
    return SOC_SBX_CALADAN3_MAX_CLIENT;
}


/*
 *
 * Function:
 *     soc_sbx_caladan3_port_info_dump
 * Purpose:
 *     dump port info config
 */
void soc_sbx_caladan3_port_info_dump(int unit, int verbose, int port)
{
    int index, lcos, fcos, lport, fport;
    int nldq, nfdq;
    int i, j, qid;
    soc_sbx_caladan3_port_map_t *port_map = _caladan3_port_map[unit];
    soc_sbx_caladan3_port_map_info_t *port_info = NULL;
    soc_sbx_caladan3_queues_t *lqinfo = NULL, *fqinfo = NULL;
    char buffer1[512], buffer2[512];
    int buffer1_len, buffer2_len;
    char buffer[50];

    LOG_CLI((BSL_META_U(unit,
                        "\n+ Port mapping Information +\n")));
    if (port_map && !verbose) {
        LOG_CLI((BSL_META_U(unit,
                            "\n+--------------------------------------------------------------------------------------------+")));
        LOG_CLI((BSL_META_U(unit,
                            "\n|             Line                            |            Fabric                            |")));
        LOG_CLI((BSL_META_U(unit,
                            "\n+--------------------------------------------------------------------------------------------+")));
        LOG_CLI((BSL_META_U(unit,
                            "\n|  Port       SQ    NSQ    DQ    NDQ     FC   |   Port      SQ    NSQ    DQ    NDQ     FC    |")));
        LOG_CLI((BSL_META_U(unit,
                            "\n+--------------------------------------------------------------------------------------------+")));
        for (index=0; index < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; index++) {
            if (!(port_map->line_port_info[index].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
                continue;
            }
            lport = port_map->line_port_info[index].port;
            lqinfo = &port_map->line_port_info[index].port_queues;
            fport = port_map->fabric_port_info[index].port;
            fqinfo = &port_map->fabric_port_info[index].port_queues;
            if ((port >= 0) && (lport != port)) {continue;}
            lcos = (lqinfo->num_squeue > 0) ? lqinfo->num_squeue  : 1;
            nldq = (lqinfo->num_dqueue > 0) ? lqinfo->num_dqueue  : 1;
            fcos = (fqinfo->num_squeue > 0) ? fqinfo->num_squeue  : 1;
            nfdq = (fqinfo->num_dqueue > 0) ? fqinfo->num_dqueue  : 1;
            LOG_CLI((BSL_META_U(unit,
                                "\n| %2d(%4s) %3d(%2x)  %2d  %4d(%2x) %2d  %8s | %2d(%4s) %3d(%2x)  %2d %4d(%2x)  %2d %8s   |"),
                     lport, 
                     SOC_PORT_NAME(unit, lport),
                     port_map->line_port_info[index].port_queues.squeue_base,
                     port_map->line_port_info[index].port_queues.squeue_base, lcos,
                     port_map->line_port_info[index].port_queues.dqueue_base,
                     port_map->line_port_info[index].port_queues.dqueue_base, nldq,
                     _caladan3_flow_ctrl_str[port_map->line_port_info[index].flow_control],
                     fport, 
                     SOC_PORT_NAME(unit, fport),
                     port_map->fabric_port_info[index].port_queues.squeue_base,
                     port_map->fabric_port_info[index].port_queues.squeue_base, fcos,
                     port_map->fabric_port_info[index].port_queues.dqueue_base,
                     port_map->fabric_port_info[index].port_queues.dqueue_base, nfdq,
                     _caladan3_flow_ctrl_str[port_map->fabric_port_info[index].flow_control]));
            if ((lcos <= 1) && (fcos <= 1) && (nldq <= 1) && (nfdq <= 1)) {
                continue;
            }
            LOG_CLI((BSL_META_U(unit,
                                "\n|Line Squeues:                                |Fabric Dqueues:                               |")));
            i = 0;
            SOC_SBX_C3_BMP_ITER_RANGE(lqinfo->squeue_bmp, SOC_SBX_CALADAN3_SWS_LINE_SQUEUE_BASE,
                                      qid, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES) {
                i += sal_snprintf(buffer1 + i, 260-i, "%2d,", qid);
            }
            buffer1[i-1] = ' ';
            i = 0;
            SOC_SBX_C3_BMP_ITER_RANGE(fqinfo->dqueue_bmp, SOC_SBX_CALADAN3_SWS_FABRIC_DQUEUE_BASE,
                                      qid, SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES) {
                i += sal_snprintf(buffer2 + i, 260-i, "%2d,", qid);
            }
            buffer2[i-1] = ' ';
            i = 0; j =0;
            buffer1_len = strlen(buffer1);
            buffer2_len = strlen(buffer2);
            do {
                if (i <= buffer1_len) {
                    sal_snprintf(buffer, 45, "%45s", buffer1 + i);
                } else {
                    sal_snprintf(buffer, 45, "%45s", "                                                  ");
                }
                LOG_CLI((BSL_META_U(unit,
                                    "\n|%-45s"), buffer));
                i += 45;
                if (j <= buffer2_len) {
                    sal_snprintf(buffer, 45, "%44s", buffer2 + j);
                } else {
                    sal_snprintf(buffer, 45, "%45s", "                                                  ");
                }
                LOG_CLI((BSL_META_U(unit,
                                    "|%-46s|"), buffer));
                j += 44;
            } while ((i < buffer1_len) || (j < buffer2_len));

            LOG_CLI((BSL_META_U(unit,
                                "\n|Line Dqueues:                                |Fabric Squeues:                               |")));
            i = 0;
            SOC_SBX_C3_BMP_ITER_RANGE(lqinfo->dqueue_bmp, SOC_SBX_CALADAN3_SWS_LINE_DQUEUE_BASE,
                                      qid, SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES) {
                i += sal_snprintf(buffer1 + i, 260-i, "%2d,", qid);
            }
            buffer1[i-1] = ' ';
            i = 0;
            SOC_SBX_C3_BMP_ITER_RANGE(fqinfo->squeue_bmp, SOC_SBX_CALADAN3_SWS_FABRIC_SQUEUE_BASE,
                                      qid, SOC_SBX_CALADAN3_NUM_WORDS_FOR_SQUEUES) {
                i += sal_snprintf(buffer2 + i, 260-i, "%2d,", qid);
            }
            buffer2[i-1] = ' ';
            i = 0; j =0;
            buffer1_len = strlen(buffer1);
            buffer2_len = strlen(buffer2);
            do {
                if (i <= buffer1_len) {
                    sal_snprintf(buffer, 45, "%44s", buffer1 + i);
                } else {
                    sal_snprintf(buffer, 45, "%45s", "                                                  ");
                }
                LOG_CLI((BSL_META_U(unit,
                                    "\n|%-45s"), buffer));
                i += 44;
                if (j <= buffer2_len) {
                    sal_snprintf(buffer, 45, "%43s", buffer2 + j);
                } else {
                    sal_snprintf(buffer, 45, "%45s", "                                                  ");
                }
                LOG_CLI((BSL_META_U(unit,
                                    "|%-46s|"), buffer));
                j += 42;
            } while ((i <= buffer1_len) || (j <= buffer2_len));

            LOG_CLI((BSL_META_U(unit,
                                "\n|---------------------------------------------|----------------------------------------------|")));
        }
        LOG_CLI((BSL_META_U(unit,
                            "\n+--------------------------------------------------------------------------------------------+\n")));

    } else if (port_map && verbose) {

        soc_port_t lport, pport;
        int blk, idx, status;
        LOG_CLI((BSL_META_U(unit,
                            "\n+Port to Block Dump +\n")));
        SOC_PBMP_ITER(PBMP_ALL(unit), lport) {
            if ((port >= 0) && (lport != port)) {continue;}
            if (soc_feature(unit, soc_feature_logical_port_num)) {
                pport = SOC_INFO(unit).port_l2p_mapping[lport];
            } else {
                pport = lport;
            }
            blk = SOC_PORT_BLOCK(unit, pport);
            status = soc_sbx_caladan3_sws_pbmp_to_line_port(unit, lport, &idx);
            if (status == SOC_E_NONE) {
                port_info = &port_map->line_port_info[idx];
            } else {
                status = soc_sbx_caladan3_sws_pbmp_to_fab_port(unit, lport, &idx);
                if (status == SOC_E_NONE) {
                    port_info = &port_map->fabric_port_info[idx];
                }
            }
            if (SOC_FAILURE(status)) {
                LOG_CLI((BSL_META_U(unit,
                                    "Unit %d, Unmapped port %d\n"), unit, lport));
            } else {
                LOG_CLI((BSL_META_U(unit,
                                    "LPort: %2d (%4s)  Block: %7s(%2d)  Block-Index: %2d  Base Port: %2d  PhyPort: %2d\n"),
                         lport, SOC_PORT_NAME(unit, lport),
                         SOC_INFO(unit).block_name[blk], blk,
                         SOC_PORT_BINDEX(unit, pport),
                         port_info->base_port, pport));
            }
        }
    } else {
        LOG_CLI((BSL_META_U(unit,
                            "\n!!!! No port mapping available \n")));
    }
}

int
soc_sbx_caladan3_100G_to_phyid[] = {
    0x1,
    0x15
};

int
soc_sbx_caladan3_40G_to_phyid[] = {
    0x1,
    0x1d,
    0x35,
    0x15,
    0x19,
    0x31
};

int
soc_sbx_caladan3_10G_to_fab_phyid[] = {
    0x15,
    0x16,
    0x17,
    0x18,
    0x19,
    0x1a,
    0x1b,
    0x1c,
    0x31,
    0x32,
    0x33,
    0x34
};

int
soc_sbx_caladan3_1G_to_phyid[] = {
    0x5,
    0x6,
    0x7,
    0x8,
    0x9,
    0xa,
    0xb,
    0xc,
    0xd,
    0xe,
    0xf,
    0x10,
    0x11,
    0x12,
    0x13,
    0x14,
    0x21,
    0x22,
    0x23,
    0x24,
    0x25,
    0x26,
    0x27,
    0x28,
    0x29,
    0x2a,
    0x2b,
    0x2c,
    0x2d,
    0x2e,
    0x2f,
    0x30,
    0x39,
    0x3a,
    0x3b,
    0x3c,
    0x3d,
    0x3e,
    0x3f,
    0x40,
    0x41,
    0x42,
    0x43,
    0x44,
    0x45,
    0x46,
    0x47,
    0x48,
    0x4b,
    0x4b,
    0x4b,
    0x4b
};

int
soc_sbx_caladan3_10G_to_phyid[] = {
    0x1,
    0x2,
    0x3,
    0x4,
    0x1d,
    0x1e,
    0x1f,
    0x20,
    0x35,
    0x36,
    0x37,
    0x38,
};

int
soc_sbx_caladan3_port_to_phyid(int unit, int port, int *phyidx) 
{
    int speed;
    int offset = 0;
    int blk = 0;
    soc_info_t *si = &SOC_INFO(unit);
    volatile int max_ports, index; 
    volatile int phy_port, bindex;

    if (!si) 
        return SOC_E_INIT;

    if (soc_feature(unit, soc_feature_logical_port_num)) {
        phy_port = SOC_INFO(unit).port_l2p_mapping[port];
    } else {
        phy_port = port;
    }
    bindex = SOC_PORT_BINDEX(unit, phy_port);
    speed = si->port_speed_max[port];
    max_ports = soc_sbx_caladan3_get_max_ports(unit);
    index = port % max_ports;
    if ((speed > 100000) || IS_IL_PORT(unit, port)) {
        if (soc_sbx_caladan3_is_line_port(unit, port)) {
            offset = 0;
        } else {
            offset = 1;
        }
        index = bindex;
        *phyidx = soc_sbx_caladan3_100G_to_phyid[index + offset];
    } else if (speed >= 25000) {    
        blk =  SOC_PORT_BLOCK(unit, phy_port);
        if (blk == soc_sbx_block_find(unit, SOC_BLK_CLPORT, 0)) {
            offset = 0;
        } else if (blk == soc_sbx_block_find(unit, SOC_BLK_CLPORT, 1)) {
            offset = 3;
        } else {
            if (soc_sbx_caladan3_is_line_port(unit, port)) {
            offset = 0;
            } else {
            offset = 3;
            }
        }
        index = bindex >> 2 ;
        *phyidx = soc_sbx_caladan3_40G_to_phyid[index + offset];
    } else if ((speed > 1000) ||
               ((si->port_speed_max[port] <= 1000) &&
                 (soc_property_port_get(unit, port, spn_SERDES_QSGMII_SGMII_OVERRIDE, 1) == 2))) {
        index = bindex;
        *phyidx = soc_sbx_caladan3_10G_to_phyid[index];
         blk =  SOC_PORT_BLOCK(unit, phy_port);
         if (blk == soc_sbx_block_find(unit, SOC_BLK_CLPORT, 1)) {
             *phyidx = soc_sbx_caladan3_10G_to_fab_phyid[index];
         }
    } else {
        if (SOC_PBMP_MEMBER(si->cl.bitmap, port)) {
            offset = 0;
        } else if (SOC_PBMP_MEMBER(si->xt.bitmap, port)) {
            blk =  SOC_PORT_BLOCK(unit, phy_port);
            if (blk == soc_sbx_block_find(unit, SOC_BLK_XTPORT, 0)) {
                offset = 12;
            } else if (blk == soc_sbx_block_find(unit, SOC_BLK_XTPORT, 1)) {
                offset = 24;
            } else if (blk == soc_sbx_block_find(unit, SOC_BLK_XTPORT, 2)) {
                offset = 36;
            } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** ERROR: phy port mapping in 1G XTPORT failed, port %d\n"), unit, port));
                return SOC_E_PARAM;
            }
        } else if (SOC_PBMP_MEMBER(si->xl.bitmap, port)) {
            offset = 48;
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** ERROR: phy port mapping in 1G mode failed, index %d"), unit, index));
            return SOC_E_PARAM;
        }
        index = bindex;
        *phyidx = soc_sbx_caladan3_1G_to_phyid[index + offset];
    }

    return SOC_E_NONE;
} 

/*
 * Function:
 *    soc_sbx_caladan3_wc_image_download_helper
 * Purpose
 *    provide faster means to download wc image
 */
int
soc_sbx_caladan3_wc_image_download_helper(int unit, int port, uint8 *arr, int len)
{
    int rv = SOC_E_NONE;
    int ge_port = 0;
    int p, phy_port = 0, subport = 0;
    soc_info_t *si;
    int i, j, k, excess;
    uint32 nentries = 0, regval = 0, lastw = 0;
    uint32 *wcimage;
    int idx, nbytes;
    int blk = 0;

    /* Entry size of UCMEM */
    nbytes = 16;
    
    si  = &SOC_INFO(unit);
    wcimage = (uint32*)soc_cm_salloc(unit, len, "wc image");
    if (!wcimage) {
        LOG_CLI((BSL_META_U(unit,
                            "\nWC Image download failed, no memory allocated")));
        return SOC_E_PARAM;
    }
    if (si->port_num_lanes[port]==1) {
        ge_port = 1;
        subport = 0;
        if (SOC_PBMP_MEMBER(si->xt.bitmap, port)) {
            if (soc_feature(unit, soc_feature_logical_port_num)) {
                phy_port = SOC_INFO(unit).port_l2p_mapping[port];
            } else {
                phy_port = port;
            }

            blk = SOC_PORT_BLOCK(unit, phy_port);
            if (blk == soc_sbx_block_find(unit, SOC_BLK_XTPORT, 0)) {
                subport = 12;
            } else if (blk == soc_sbx_block_find(unit, SOC_BLK_XTPORT, 1)) {
                subport = 24;
            } else if (blk == soc_sbx_block_find(unit, SOC_BLK_XTPORT, 2)) {
                subport = 36;
            } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** ERROR: phy port mapping in 1G XTPORT failed, port %d\n"), unit, port));
                return SOC_E_PARAM;
            }
        }
        subport += SOC_PORT_BINDEX(unit, phy_port);
        /* WCs are attached to the CLPORT only, so remap to CLPORT after extracting instance */
        SOC_PBMP_ITER(si->cl.bitmap, p) {
            if (IS_GE_PORT(unit, p)) {
                port = p;
                break;
            }
        }
    }

    /* 
     * IL Case override
     */
    if (SOC_PBMP_NOT_NULL(si->il.bitmap)) {
        if (soc_sbx_caladan3_is_line_port(unit, port)) {
            port = SOC_SBX_CALADAN3_CLPORT0_L2P_IDX;
        } else {
            port = SOC_SBX_CALADAN3_CLPORT1_L2P_IDX;
        }
    }

    if (soc_feature(unit, soc_feature_logical_port_num)) {
        phy_port = SOC_INFO(unit).port_l2p_mapping[port];
    } else {
        phy_port = port;
    }

    rv = READ_PORT_WC_UCMEM_CTRLr(unit, port, &regval);
    if (SOC_SUCCESS(rv)) {
        /* enable parallel bus access */
        soc_reg_field_set(unit, PORT_WC_UCMEM_CTRLr, &regval, ACCESS_MODEf, 1);
        if (ge_port) {
            subport /= 16;
            soc_reg_field_set(unit, PORT_WC_UCMEM_CTRLr, &regval, INST_SELECTf, subport);
        } else if ((si->port_num_lanes[port] >= 10) || (si->port_type[port] == SOC_BLK_IL)) {
            /* enable broadcast */
            soc_reg_field_set(unit, PORT_WC_UCMEM_CTRLr, &regval, WR_BROADCASTf, 1);
            soc_reg_field_set(unit, PORT_WC_UCMEM_CTRLr, &regval, INST_SELECTf, 0);
        } else {
            subport = SOC_PORT_BINDEX(unit, phy_port) % 3;
            soc_reg_field_set(unit, PORT_WC_UCMEM_CTRLr, &regval, INST_SELECTf, subport);
       }
    }
    rv = WRITE_PORT_WC_UCMEM_CTRLr(unit, port, regval);

    if (SOC_SUCCESS(rv)) {
        excess = len % 4;
        idx = nbytes/sizeof(int);
        for (k=0, j=idx-1, i=0; i < (len-excess); i+=4, j--) {
            wcimage[k*idx+j] = (arr[i] << 24) | (arr[i+1] << 16) | (arr[i+2] << 8) | arr[i+3];
            if (j == 0) { k++; j=idx; }
        }
        if (excess) {
            if (excess > 2)
                lastw = (arr[i] << 24) | (arr[i+1] << 16) | (arr[i+2] << 8);
            else if (excess > 1)
                lastw = (arr[i] << 24) | (arr[i+1] << 16);
            else if (excess > 0)
                lastw = (arr[i] << 24);
            wcimage[k*idx+j] =  lastw;
        }
        nentries = (len + nbytes - 1) / nbytes   ;
        rv = soc_mem_write_range(unit, PORT_WC_UCMEM_DATAm, SOC_PORT_BLOCK(unit, phy_port), 0, nentries, wcimage);
        soc_cm_sfree(unit, wcimage);
    }
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "Downloading wcimage to port %d using download helper: %s\n"), 
                 port, SOC_SUCCESS(rv) ? "OK" : "FAIL"));

    /* switch back to mdio access */
    soc_reg_field_set(unit, PORT_WC_UCMEM_CTRLr, &regval, ACCESS_MODEf, 0);
    rv = WRITE_PORT_WC_UCMEM_CTRLr(unit, port, regval);

    return rv;
}


int soc_sbx_caladan3_port_reset_mib_counters(int unit, int port, int subport)
{
    uint32 rval;
    int rv = SOC_E_NONE;

    rv = READ_PORT_MIB_RESETr(unit, port, &rval);
    if (SOC_SUCCESS(rv)) {
        rval = rval | (1 << subport);
        rv = WRITE_PORT_MIB_RESETr(unit, port, rval);
        if (SOC_FAILURE(rv)) {
            LOG_CLI((BSL_META_U(unit,
                                "\nError clearing mib counters")));
            return SOC_E_INIT;
        }
        rval = rval & ~(1 << subport);
        rv = WRITE_PORT_MIB_RESETr(unit, port, rval);
        if (SOC_FAILURE(rv)) {
            LOG_CLI((BSL_META_U(unit,
                                "\nError clearing mib counters")));
            return SOC_E_INIT;
        }
    }
    return SOC_E_NONE;
}


/*
 * Function:
 *    soc_sbx_caladan3_port_enable
 * Purpose:
 *    Enable ports in the mac core
 */
int
soc_sbx_caladan3_port_enable(int unit, int port, int subport, int enable)
{
    uint32 rval;
    int rv = SOC_E_NONE;
    int portf[] = { PORT0f, PORT1f, PORT2f, PORT3f, PORT4f, PORT5f,
                    PORT6f, PORT7f, PORT8f, PORT9f, PORT10f, PORT11f };

#ifdef MAC_PORT_MODE_ACCESS_DEBUG
    cli_out("\n%s  port:%d  subport/idx:%d  en:%d\n",__FUNCTION__, port, subport, enable);
#endif

    rv = READ_PORT_ENABLE_REGr(unit, port, &rval);
    if (soc_reg_field_get(unit, PORT_ENABLE_REGr, rval, portf[subport]) == enable) {
        /* Already in that mode */
        return SOC_E_NONE;
    }

    if (SOC_SUCCESS(rv)) {
        rv = soc_reg_field32_modify(unit, PORT_ENABLE_REGr, port, portf[subport], enable);
    }
    return rv;
}

int
soc_sbx_caladan3_xgxs_reset(int unit, int port, int wcidx) 
{

    const static soc_reg_t caladan3_regs[] = {
        PORT_XGXS0_CTRL_REGr,
        PORT_XGXS1_CTRL_REGr,
        PORT_XGXS2_CTRL_REGr
    };
    soc_reg_t   reg;
    uint64      rval64;
    int         reset_sleep_usec = SAL_BOOT_QUICKTURN ? 500000 : 1100;

    reg = caladan3_regs[wcidx];

    /*
     * Reference clock selection
     *    LCPLL -> WC0, WC3 = 0
     *    REFCLK -> WC0, WC3 = 1
     *    LCPLL -> WC1,2 WC4,5 = 1
     */
    SOC_IF_ERROR_RETURN(soc_reg_get(unit, reg, port, 0, &rval64));
    if (wcidx == 0) {
        soc_reg64_field32_set(unit, reg, &rval64, LCREF_ENf, 0);
        soc_reg64_field32_set(unit, reg, &rval64, REFOUT_ENf, 1);
    } else {
        soc_reg64_field32_set(unit, reg, &rval64, LCREF_ENf, 1);
        soc_reg64_field32_set(unit, reg, &rval64, REFOUT_ENf, 0);
    }
    SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));

    /*
     * XGXS MAC initialization steps.
     *
     * A minimum delay is required between various initialization steps.
     * There is no maximum delay.  The values given are very conservative
     * including the timeout for PLL lock.
     */
    /* Release reset (if asserted) to allow xmac/cmac to initialize */
    soc_reg64_field32_set(unit, reg, &rval64, IDDQf, 0);
    soc_reg64_field32_set(unit, reg, &rval64, PWRDWNf, 0);
    soc_reg64_field32_set(unit, reg, &rval64, PWRDWN_PLLf, 0);
    soc_reg64_field32_set(unit, reg, &rval64, RSTB_HWf, 1);
    SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));
    sal_usleep(reset_sleep_usec);

    /* Power down and reset */
    soc_reg64_field32_set(unit, reg, &rval64, PWRDWNf, 1);
    soc_reg64_field32_set(unit, reg, &rval64, PWRDWN_PLLf, 1);
    soc_reg64_field32_set(unit, reg, &rval64, IDDQf, 1);
    soc_reg64_field32_set(unit, reg, &rval64, RSTB_HWf, 0);
    soc_reg64_field32_set(unit, reg, &rval64, RSTB_MDIOREGSf, 0);
    soc_reg64_field32_set(unit, reg, &rval64, RSTB_PLLf, 0);
    soc_reg64_field32_set(unit, reg, &rval64, TXD1G_FIFO_RSTBf, 0);
    soc_reg64_field32_set(unit, reg, &rval64, TXD10G_FIFO_RSTBf, 0);
    SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));
    sal_usleep(reset_sleep_usec);

    /*
     * Bring up both digital and analog clocks
     *
     * NOTE: Many MAC registers are not accessible until the PLL is locked.
     * An S-Channel timeout will occur before that.
     */
    soc_reg64_field32_set(unit, reg, &rval64, PWRDWNf, 0);
    soc_reg64_field32_set(unit, reg, &rval64, PWRDWN_PLLf, 0);
    soc_reg64_field32_set(unit, reg, &rval64, IDDQf, 0);
    SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));
    sal_usleep(reset_sleep_usec);

    /* Bring XGXS out of reset */
    soc_reg64_field32_set(unit, reg, &rval64, RSTB_HWf, 1);
    SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));
    sal_usleep(reset_sleep_usec);

    /* Bring MDIO registers out of reset */
    soc_reg64_field32_set(unit, reg, &rval64, RSTB_MDIOREGSf, 1);
    SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));

    /* Activate all clocks */
    soc_reg64_field32_set(unit, reg, &rval64, RSTB_PLLf, 1);
    SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));

    /* Bring Tx FIFO out of reset */
    soc_reg64_field32_set(unit, reg, &rval64, TXD1G_FIFO_RSTBf, 0xf);
    soc_reg64_field32_set(unit, reg, &rval64, TXD10G_FIFO_RSTBf, 1);
    SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));

    /* Get MLD Out of reset */
    WRITE_PORT_MLD_CTRL_REGr(unit, port, 3);

    /* 
     * XL Port requires clock from the pads
     * If refsel = 2b00, refclk comes from external chip: LCREFP/N
     * If refsel = 2b01, refclk comes from pad_refclkp/n
     * LCREF is not applicable in this case
     */
   if (IS_XL_PORT(unit, port)) {
        soc_reg64_field32_set(unit, reg, &rval64, REFSELf, 1);
#ifdef SGMII_MODE
        if (soc_property_port_get(unit, port, "xgxs_lcpll_xtal_refclk", 156) == 25) {
            soc_reg64_field32_set(unit, reg, &rval64, REFDIVf, 0);
        } else {
            soc_reg64_field32_set(unit, reg, &rval64, REFDIVf, 3);
        }
#else
        soc_reg64_field32_set(unit, reg, &rval64, REFDIVf, 0);
#endif
        /* enable internal 100 ohm termination */
        soc_reg64_field32_set(unit, reg, &rval64, REF_TERM_SELf, 1);
        SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));
   }

    return SOC_E_NONE;

}

int
_soc_sbx_caladan3_clport_gmii_mode(int unit, int cl_gmii)
{
    uint32 cx_mode = 0;
    int rv = SOC_E_NONE;
    rv = READ_CX_PHY_PORT_CONFIGr(unit, &cx_mode);
    if (SOC_SUCCESS(rv)) {
        soc_reg_field_set(unit, CX_PHY_PORT_CONFIGr, &cx_mode, CL_PORT_QSGMII_SELf, cl_gmii);
        soc_reg_field_set(unit, CX_PHY_PORT_CONFIGr, &cx_mode, WC0_MD_STf, 1);
        soc_reg_field_set(unit, CX_PHY_PORT_CONFIGr, &cx_mode, WC1_MD_STf, 1);
        soc_reg_field_set(unit, CX_PHY_PORT_CONFIGr, &cx_mode, WC2_MD_STf, 1);
        soc_reg_field_set(unit, CX_PHY_PORT_CONFIGr, &cx_mode, WC3_MD_STf, 1);
        soc_reg_field_set(unit, CX_PHY_PORT_CONFIGr, &cx_mode, WC4_MD_STf, 1);
        soc_reg_field_set(unit, CX_PHY_PORT_CONFIGr, &cx_mode, WC5_MD_STf, 1);
        WRITE_CX_PHY_PORT_CONFIGr(unit, cx_mode);
    }
    return rv;
}

int
_soc_sbx_caladan3_wc_gmii_config(int unit, int wc0, int wc1, int wc2)
{
    uint32 cx_mode = 0;
    int rv = SOC_E_NONE;
    rv = READ_CX_PHY_PORT_CONFIGr(unit, &cx_mode);
    if (SOC_SUCCESS(rv)) {
        soc_reg_field_set(unit, CX_PHY_PORT_CONFIGr, &cx_mode, WC0_QSGMII_SELf, wc0);
        soc_reg_field_set(unit, CX_PHY_PORT_CONFIGr, &cx_mode, WC1_QSGMII_SELf, wc1);
        soc_reg_field_set(unit, CX_PHY_PORT_CONFIGr, &cx_mode, WC2_QSGMII_SELf, wc2);
        WRITE_CX_PHY_PORT_CONFIGr(unit, cx_mode);
    }
    return rv;
}

int
_soc_sbx_caladan3_wc_gmii_mode(int unit, int core, int gmii_mode)
{
    uint32 cx_mode = 0;
    int rv = SOC_E_NONE;
    rv = READ_CX_PHY_PORT_CONFIGr(unit, &cx_mode);
    if (SOC_SUCCESS(rv)) {
        if (core == 0) {
            soc_reg_field_set(unit, CX_PHY_PORT_CONFIGr, &cx_mode, WC0_QSGMII_SELf, gmii_mode);
        } else if (core == 1) {
            soc_reg_field_set(unit, CX_PHY_PORT_CONFIGr, &cx_mode, WC1_QSGMII_SELf, gmii_mode);
        } else if (core == 2) {
            soc_reg_field_set(unit, CX_PHY_PORT_CONFIGr, &cx_mode, WC2_QSGMII_SELf, gmii_mode);
        }
        WRITE_CX_PHY_PORT_CONFIGr(unit, cx_mode);
    }
    return rv;
}

int
soc_sbx_caladan3_wc_mode_set(int unit) 
{
    int rv = SOC_E_NONE;
    int wc0_gmii = 0, wc0_used = 0;
    int wc1_gmii = 0, wc1_used = 0;
    int wc2_gmii = 0, wc2_used = 0;
    int ucode_num_port_override = 0;
    soc_port_t port, phy_port;
    int block, bindex, count, max;
    int mode;
    soc_pbmp_t pbmp;
    soc_info_t *si;

    si  = &SOC_INFO(unit);
    SOC_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
    SOC_PBMP_COUNT(pbmp, max);
    count = 0;
    SOC_PBMP_ITER(pbmp, port) {
        if (!soc_sbx_caladan3_is_line_port(unit, port) || 
                  IS_XL_PORT(unit, port)) {
            count++;
            continue;
        }
        if (soc_feature(unit, soc_feature_logical_port_num)) {
            phy_port = si->port_l2p_mapping[port];
        } else {
            phy_port = port;
        }

        bindex = SOC_PORT_BINDEX(unit, phy_port);

        if (si->port_speed_max[port] >= 100*SOC_SBX_CALADAN3_GIG_SPEED) {
            count++;
            switch (bindex) {
            case 0:
                wc0_gmii = wc1_gmii = wc2_gmii = 0;
                wc0_used += 16;
                wc1_used += 16;
                wc2_used += 16;
                break;
            default:
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** ERROR: port %d mapped incorrectly\n"),
                           unit, port));
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** ERROR: Port can be mapped to 0 only\n"), unit));
                return SOC_E_PARAM;
            }
        } else if (si->port_speed_max[port] >= 40*SOC_SBX_CALADAN3_GIG_SPEED) {
            count++;
            switch (bindex) {
            case 0:
                wc0_gmii = 0;
                wc0_used += 16;
                break;
            case 4:
                wc1_gmii = 0;
                wc1_used += 16;
                break;
            case 8:
                wc2_gmii = 0;
                wc2_used += 16;
                break;
            default:
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** ERROR: port %d mapped incorrectly\n"), 
                           unit, port));
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** ERROR: Port can be mapped to 0/4/8 only\n"), unit));
                return SOC_E_PARAM;
            }
        } else {
            count++;
            mode = (si->port_speed_max[port] >= 
                        10*SOC_SBX_CALADAN3_GIG_SPEED) ? 1 : 0;
            if (soc_property_port_get(unit, port, spn_SERDES_QSGMII_SGMII_OVERRIDE, 1) == 2) {
                mode = 1;
            }
            
            block = SOC_PORT_BLOCK(unit, phy_port);
            if (soc_sbx_block_find(unit, SOC_BLK_CLPORT, 0) == block) {
                switch (bindex) {
                case 0:
                case 1:
                case 2:
                case 3:
                    wc0_gmii = (mode) ? 0 : 1;
                    wc0_used += (mode) ? 4 : 1;
                    break;
                case 4:
                case 5:
                case 6:
                case 7:
                    if (mode) {
                        wc1_gmii = 0;
                        wc1_used += 4;
                    } else {
                        wc0_gmii = 1;
                        wc0_used += 1;
                    }
                    break;
                case 8:
                case 9:
                case 10:
                case 11:
                    if (mode) {
                        wc2_gmii = 0;
                        wc2_used += 4;
                    } else {
                        wc0_gmii = 1;
                        wc0_used += 1;
                    }
                    break;
                }
            } else if (soc_sbx_block_find(unit, SOC_BLK_XTPORT, 0) == block) {
                switch (bindex) {
                case 0:
                case 1:
                case 2:
                case 3:
                    if ((wc0_used > 0) && (!wc0_gmii)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "Unit %d *** ERROR: "
                                              "Cannot mix GMII/Non GMII ports WC0\n"), unit));
                        return SOC_E_PARAM;
                    }
                    wc0_gmii =  1;
                    wc0_used += 1;
                    break;
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                    if ((wc1_used > 0) && (!wc1_gmii)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "Unit %d *** ERROR: "
                                              "Cannot mix Non GMII/GMII ports WC1\n"), unit));
                        return SOC_E_PARAM;
                    }
                    wc1_gmii = 1;
                    wc1_used += 1;
                    break;
                }
            } else if (soc_sbx_block_find(unit, SOC_BLK_XTPORT, 1) == block) {
                switch (bindex) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                    if ((wc1_used > 0) && (!wc1_gmii)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "Unit %d *** ERROR: "
                                              "Cannot mix GMII/Non GMII ports WC1\n"), unit));
                        return SOC_E_PARAM;
                    }
                    wc1_gmii = 1;
                    wc1_used += 1;
                    break;
                case 8:
                case 9:
                case 10:
                case 11:
                    if ((wc2_used > 0) && (!wc2_gmii)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "Unit %d *** ERROR: "
                                              "Cannot mix Non GMII/GMII ports WC2\n"), unit));
                        return SOC_E_PARAM;
                    }
                    wc2_gmii = 1;
                    wc2_used += 1;
                    break;
                }
            } else if (soc_sbx_block_find(unit, SOC_BLK_XTPORT, 2) == block) {
                if ((wc2_used > 0) && (!wc2_gmii)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "Unit %d *** ERROR: "
                                              "Cannot mix Non GMII/GMII ports WC2\n"), unit));
                        return SOC_E_PARAM;
                }
                wc2_gmii = 1;
                wc2_used += 1;
            }
        }
    }
    if (count != max) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** ERROR: port mapping ok only for %d out of %d ports \n"),
                   unit, count, max));
        return SOC_E_PARAM;
    }

    ucode_num_port_override = soc_property_get(unit, "ucode_num_port_override", 0);

    if (wc0_used > 16) {
        if (!ucode_num_port_override) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** ERROR: port mapping to WC0 exceeds limits\n"), unit));
            return SOC_E_PARAM;
        }
    }

    if (wc1_used > 16) {
        if (!ucode_num_port_override) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** ERROR: port mapping to WC1 exceeds limits\n"), unit));
            return SOC_E_PARAM;
        }
    }

    if (wc2_used > 16) {
        if (!ucode_num_port_override) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d *** ERROR: port mapping to WC2 exceeds limits\n"), unit));
            return SOC_E_PARAM;
        }
    }
    if ((wc0_used % 4) != 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** ERROR: Unsupported port mapping mix detected on WC0\n"),
                   unit));
        return SOC_E_PARAM;
    }
    if ((wc1_used % 4) != 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** ERROR: Unsupported port mapping mix detected on WC1\n"),
                   unit));
        return SOC_E_PARAM;
    }
    if ((wc2_used % 4) != 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d ***  ERROR: Unsupported port mapping mix detected on WC2\n"),
                   unit));
        return SOC_E_PARAM;
    }

    rv = _soc_sbx_caladan3_wc_gmii_config(unit, wc0_gmii, wc1_gmii, wc2_gmii);

    return rv;
}

int
_soc_sbx_caladan3_phy_config(int unit, int port)
{
    int rv = SOC_E_NONE;
    int core;

    /* for now, assume we can access all 3 wc */
    for (core = 0; core < 3; core++) {
        rv = soc_sbx_caladan3_xgxs_reset(unit, port, core);
        if (SOC_SUCCESS(rv)) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "\nInfo:wc %d init on port %d ok"), core, port));
        }
    }  
    return rv;
}

int
_soc_sbx_caladan3_phy_core_config(int unit, int port, int core)
{
    int rv = SOC_E_NONE;

    rv = soc_sbx_caladan3_xgxs_reset(unit, port, core);
    if (SOC_SUCCESS(rv)) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "\nInfo:wc %d init on port %d ok"), core, port));
    }  
    
    return rv;
}

/*
 * Function:
 *    soc_sbx_caladan3_port_speed_get
 * Purpose:
 *    Granular port speed
 */
int
soc_sbx_caladan3_port_speed_get(int unit, int port, int* speed) 
{
    uint32 regval = 0 , portmode = 0;
    int mac_mode = 0, higig = 0;
    
    SOC_IF_ERROR_RETURN(READ_PORT_CONFIGr(unit, port, &regval));
    if (soc_reg_field_get(unit, PORT_CONFIGr, regval, HIGIG2_MODEf) || 
            soc_reg_field_get(unit, PORT_CONFIGr, regval, HIGIG_MODEf)) {
        higig = 1;
    } 
    SOC_IF_ERROR_RETURN(READ_PORT_MODE_REGr(unit, port, &portmode));
    if (soc_reg_field_get(unit, PORT_MODE_REGr, portmode, MAC_MODEf) 
              == SOC_SBX_CALADAN3_INDEPENDENT_MODE) 
    {
        mac_mode = 
            soc_reg_field_get(unit, PORT_MODE_REGr, portmode, 
                              XPORT0_CORE_PORT_MODEf);
        /*XXXTBC Need to account for XAUI*/
        if (mac_mode == SOC_SBX_CALADAN3_CORE_PORT_SINGLE_MODE) {
            *speed = 40000;
            if (higig) 
                *speed= 42000;
        } else if (mac_mode == SOC_SBX_CALADAN3_CORE_PORT_DUAL_MODE) {
            if (soc_property_port_get(unit, port, spn_SERDES_RXAUI_MODE, 0)) {
                *speed = 10000;
            } else {
                *speed = 20000;
            }
            if (higig) 
                *speed= 25000;
        } else {
            *speed = 10000;
        }
    } else {
        if (higig) {
            *speed= 127000;
        } else {
            *speed= 100000;
        }
    }
    return SOC_E_NONE;
}

int
_soc_sbx_caladan3_mac_reconfig(int unit, int port,
                               int mac_mode, int core, 
                               int core_mode, int phy_mode, int gmii_mode)
{
    uint32 portmode = 0;
    uint32 mac_control = 0;
    int mac_in_reset = 0;

#ifdef MAC_PORT_MODE_ACCESS_DEBUG
    uint32 mac_cont = ~0;
    uint32 rval = ~0;
#endif

    READ_PORT_MODE_REGr(unit, port, &portmode);
    if (mac_mode != 
            soc_reg_field_get(unit, PORT_MODE_REGr, portmode, MAC_MODEf)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** ERROR: Invalid use of reconfig. use mac_config\n"), unit));
        return SOC_E_UNAVAIL;
    }

    if (core == 0) {
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPORT0_CORE_PORT_MODEf, core_mode);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPORT0_PHY_PORT_MODEf, phy_mode);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPC0_GMII_MII_ENABLEf, gmii_mode);
    } else if (core == 1) {
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPORT1_CORE_PORT_MODEf, core_mode);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPORT1_PHY_PORT_MODEf, phy_mode);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPC1_GMII_MII_ENABLEf, gmii_mode);
    } else if (core == 2) {
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPORT2_CORE_PORT_MODEf, core_mode);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPORT2_PHY_PORT_MODEf, phy_mode);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPC2_GMII_MII_ENABLEf, gmii_mode);
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** ERROR: Failed in _soc_sbx_caladan3_mac_reconfig\n"), unit));
        return SOC_E_PARAM;
    }

    /* MAC needs to be in reset to modify PORT_MODE_REG */
    SOC_IF_ERROR_RETURN(READ_PORT_MAC_CONTROLr(unit, port, &mac_control));

#ifdef MAC_PORT_MODE_ACCESS_DEBUG
    READ_PORT_ENABLE_REGr(unit, port, &rval); /*XXXTTT*/
    READ_PORT_MAC_CONTROLr(unit, port, &mac_cont); /*XXXTTT*/
    SOC_ERROR_PRINT((DK_ERR,
                     "\n_soc_sbx_caladan3_mac_reconfig Unit %d MAC_CONTROL BEF port %d  core %d mac_cont %x  port_en %x\n", 
                     unit, port, core, mac_cont, rval));
#endif

    mac_in_reset = soc_reg_field_get(unit, PORT_MAC_CONTROLr, mac_control,
                                     xmac_control_reset_field[core]);

    if (!mac_in_reset) {
        soc_reg_field_set(unit, PORT_MAC_CONTROLr, &mac_control,
                          xmac_control_reset_field[core], 1);

        SOC_IF_ERROR_RETURN(WRITE_PORT_MAC_CONTROLr(unit, port, mac_control));
        sal_udelay(30);

#ifdef MAC_PORT_MODE_ACCESS_DEBUG
        READ_PORT_MAC_CONTROLr(unit, port, &mac_cont); /*XXXTTT*/
        SOC_ERROR_PRINT((DK_ERR,
                  "_soc_sbx_caladan3_mac_reconfig  Unit %d MAC_CONTROL  AFT port %d  core %d  mac_cont %x\n", 
                         unit, port, core, mac_cont));
#endif

    }

    SOC_IF_ERROR_RETURN(WRITE_PORT_MODE_REGr(unit, port, portmode));
    /* Take MAC out of reset */
    if (!mac_in_reset) {
        soc_reg_field_set(unit, PORT_MAC_CONTROLr, &mac_control,
                          xmac_control_reset_field[core], 0);
        SOC_IF_ERROR_RETURN(WRITE_PORT_MAC_CONTROLr(unit, port, mac_control)); 
    }

    /* Take MAC out of reset, if it was out of reset per above. */
    if (!mac_in_reset) {
        soc_reg_field_set(unit, PORT_MAC_CONTROLr, &mac_control,
                          xmac_control_reset_field[core], 0);

        SOC_IF_ERROR_RETURN(WRITE_PORT_MAC_CONTROLr(unit, port, mac_control)); 

        sal_udelay(30);

#ifdef MAC_PORT_MODE_ACCESS_DEBUG
        READ_PORT_MAC_CONTROLr(unit, port, &mac_cont); /*XXXTTT*/
        SOC_ERROR_PRINT((DK_ERR,
                         "mac_reconfig    Unit %d MAC_CONTROL  AFT port %d  mac_cont %x\n", 
                         unit, port, mac_cont));
#endif

    }

    return SOC_E_NONE;
}



/*
 * Returns current mac_reset state prior to making any potential changes
 */
int
_soc_sbx_caladan3_mac_in_reset(int unit, int port, int subport, int mac_mode, 
                               uint32 *current_mac_reset, uint32 *new_mac_reset) 
{
    uint32 macctrl=0;
    int     core;
#ifdef MAC_PORT_MODE_ACCESS_DEBUG
    uint32 mac_control = 0;
#endif
    uint32 mac_in_reset = 0;

    core = subport >> 2;

    READ_PORT_MAC_CONTROLr(unit, port, &macctrl);
    if (!current_mac_reset || !new_mac_reset) {
        return SOC_E_PARAM;
    }

    *current_mac_reset = 0;
    *current_mac_reset = (macctrl & 0xF);
    *new_mac_reset = *current_mac_reset;

    mac_in_reset = soc_reg_field_get(unit, PORT_MAC_CONTROLr, macctrl,
                                     xmac_control_reset_field[core]);

#ifdef MAC_PORT_MODE_ACCESS_DEBUG
    READ_PORT_MAC_CONTROLr(unit, port, &mac_control); 

        SOC_ERROR_PRINT((DK_ERR,
                         "_soc_sbx_caladan3_mac_in_reset Unit %d MAC_CONTROL  BEF port %d  mac_control %x\n", 
                         unit, port, mac_control));

#endif

    /* Only set reset if not already in reset */
    if (!mac_in_reset) {
        if (mac_mode == SOC_SBX_CALADAN3_INDEPENDENT_MODE) {
            if (core == 0) {
                soc_reg_field_set(unit, PORT_MAC_CONTROLr, &macctrl, XMAC0_RESETf, 1);
            } else if (core == 1) {
                soc_reg_field_set(unit, PORT_MAC_CONTROLr, &macctrl, XMAC1_RESETf, 1);
            } else if (core == 2) {
                soc_reg_field_set(unit, PORT_MAC_CONTROLr, &macctrl, XMAC2_RESETf, 1);
            }
            else {
                assert(0);
            }
        } else {
            soc_reg_field_set(unit, PORT_MAC_CONTROLr, &macctrl, CMAC_RESETf, 1);
        }

        *new_mac_reset = (macctrl & 0xF);

        SOC_IF_ERROR_RETURN(WRITE_PORT_MAC_CONTROLr(unit, port, macctrl));
        
        sal_udelay(30);
    }

#ifdef MAC_PORT_MODE_ACCESS_DEBUG
    READ_PORT_MAC_CONTROLr(unit, port, &mac_control); /*XXXTTT*/
        SOC_ERROR_PRINT((DK_ERR,
                         "_soc_sbx_caladan3_mac_in_reset Unit %d MAC_CONTROL  AFT port %d  mac_control %x\n", 
                         unit, port, mac_control));
#endif

    return SOC_E_NONE;
}



int soc_sbx_caladan3_take_mac_out_of_reset(int unit, int port)
{
    soc_info_t *si;
    int mac_in_reset;
    int xmac_num;
    int phy_port;
    int subport;
    uint32 mac_control = 0;
    si = &SOC_INFO(unit);

    assert(si);

    if (soc_feature(unit, soc_feature_logical_port_num)) {
        phy_port = SOC_INFO(unit).port_l2p_mapping[port];
    } else {
        phy_port = port;
    }

    subport = SOC_PORT_BINDEX(unit, phy_port);

    SOC_IF_ERROR_RETURN(READ_PORT_MAC_CONTROLr(unit, port, &mac_control));

    if (IS_IL_PORT(unit, port) || IS_CE_PORT(unit, port) ||
        (IS_HG_PORT(unit, port) &&
         (si->port_speed_max[port] > 
          (42*SOC_SBX_CALADAN3_GIG_SPEED))) ) {
        
        mac_in_reset = soc_reg_field_get(unit, PORT_MAC_CONTROLr, mac_control,
                                         CMAC_RESETf);
        if (mac_in_reset) {
            _soc_sbx_caladan3_mac_out_reset(unit, port, subport,
                                            SOC_SBX_CALADAN3_AGGREGATE_MODE);
        }
    } else {
        
        xmac_num = subport >> 2;
        mac_in_reset = soc_reg_field_get(unit, PORT_MAC_CONTROLr, mac_control, 
                                         xmac_control_reset_field[xmac_num]);
        if (mac_in_reset) {
            _soc_sbx_caladan3_mac_out_reset(unit, port, subport, 
                                            SOC_SBX_CALADAN3_INDEPENDENT_MODE);
        }
    }
    
    return SOC_E_NONE;
}


/*
 * This function *unconditionally* sets a particular MAC out of reset.
 */
int
_soc_sbx_caladan3_mac_out_reset(int unit, int port, int subport, int mac_mode) 
{
    uint32 macctrl=0;
    int     core;
#ifdef MAC_PORT_MODE_ACCESS_DEBUG
    uint32 mac_control = 0;
#endif

    core = subport >> 2;
#ifdef MAC_PORT_MODE_ACCESS_DEBUG

    READ_PORT_MAC_CONTROLr(unit, port, &mac_control); /*XXXTTT*/
        SOC_ERROR_PRINT((DK_ERR,
                         "_soc_sbx_caladan3_mac_out_reset Unit %d MAC_CONTROL BEF  port %d  mac_control %x\n",
                         unit, port, mac_control));
#endif

    SOC_IF_ERROR_RETURN(READ_PORT_MAC_CONTROLr(unit, port, &macctrl));
    if (mac_mode == SOC_SBX_CALADAN3_INDEPENDENT_MODE) {
        if (core == 0) {
            soc_reg_field_set(unit, PORT_MAC_CONTROLr, &macctrl, XMAC0_RESETf, 0);
        } else if (core == 1) {
            soc_reg_field_set(unit, PORT_MAC_CONTROLr, &macctrl, XMAC1_RESETf, 0);
        } else if (core == 2) {
            soc_reg_field_set(unit, PORT_MAC_CONTROLr, &macctrl, XMAC2_RESETf, 0);
        } else {
            assert(0);
        }
    } else {
        soc_reg_field_set(unit, PORT_MAC_CONTROLr, &macctrl, CMAC_RESETf, 0);
    }

    SOC_IF_ERROR_RETURN(WRITE_PORT_MAC_CONTROLr(unit, port, macctrl));

    sal_udelay(30);


#ifdef MAC_PORT_MODE_ACCESS_DEBUG
    READ_PORT_MAC_CONTROLr(unit, port, &mac_control); /*XXXTTT*/
        SOC_ERROR_PRINT((DK_ERR,
                         "_soc_sbx_caladan3_mac_out_reset Unit %d MAC_CONTROL  AFT port %d  mac_control %x\n", 
                         unit, port, mac_control));
#endif

    return SOC_E_NONE;
}



int
_soc_sbx_caladan3_mac_aggregated(int unit, int port, int subport)
{
    int rv = SOC_E_NONE;
    uint32 portmode = 0;
    uint32 mac_mode  = SOC_SBX_CALADAN3_AGGREGATE_MODE;
    uint32 core_mode = SOC_SBX_CALADAN3_CORE_PORT_SINGLE_MODE;
    uint32 phy_mode  = SOC_SBX_CALADAN3_CORE_PORT_SINGLE_MODE;
    uint32 gmii_mode = 0;
    int phy_port;
    soc_info_t *si = &SOC_INFO(unit);
    uint32 prev_mac_reset = 0;
    uint32 new_mac_reset = 0;

    /* Reset the mac */
    if (!IS_IL_PORT(unit, port)) {
        rv = _soc_sbx_caladan3_mac_in_reset(unit, port, subport, mac_mode, 
                                            &prev_mac_reset, &new_mac_reset);
        if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "*** ERROR: Mac Reset failed port %d\n"), port));
            return rv;
        }
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPORT0_CORE_PORT_MODEf, core_mode);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPORT1_CORE_PORT_MODEf, core_mode);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPORT2_CORE_PORT_MODEf, core_mode);

        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPORT0_PHY_PORT_MODEf, phy_mode);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPORT1_PHY_PORT_MODEf, phy_mode);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPORT2_PHY_PORT_MODEf, phy_mode);

        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPC0_GMII_MII_ENABLEf, gmii_mode);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPC1_GMII_MII_ENABLEf, gmii_mode);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                          XPC2_GMII_MII_ENABLEf, gmii_mode);

        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, MAC_MODEf, mac_mode);
        SOC_IF_ERROR_RETURN(WRITE_PORT_MODE_REGr(unit, port, portmode));
    }

    phy_port = port;
    if (IS_IL_PORT(unit, port)) {
        if (soc_sbx_caladan3_is_line_port(unit, port)) {
            phy_port = SOC_SBX_CALADAN3_CLPORT0_L2P_IDX;
        } else {
            phy_port = SOC_SBX_CALADAN3_CLPORT1_L2P_IDX;
        }
    }

    rv = _soc_sbx_caladan3_phy_config(unit, phy_port);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** ERROR: Phy Reset failed port %d\n"), unit, port));
        return rv;
    }

    if (IS_HG_PORT(unit, port) && 
           (si->port_speed_max[port] >= 106 * SOC_SBX_CALADAN3_GIG_SPEED)) {
        /* enable HG127 */
        SOC_IF_ERROR_RETURN(WRITE_PORT_CMAC_MODEr(unit, port, 1));
    }

    if (!IS_IL_PORT(unit, port)) {
        if (prev_mac_reset != new_mac_reset) {
            SOC_IF_ERROR_RETURN(_soc_sbx_caladan3_mac_out_reset(unit, port, subport, mac_mode));
        }
    }

    return SOC_E_NONE;
}


/*
 * This function modifies the PORT_MODE_REG and the MAC MUST be held in reset
 * while modifying this register.
 */
int
_soc_sbx_caladan3_mac_independent(int unit, soc_port_t port, int subport, 
                                  int core_mode, int phy_mode, 
                                  int gmii_mode, int reset)
{
    int rv = SOC_E_NONE;
    uint32 portmode = 0;
    uint32 mac_mode;
    int core;

    /* fend */
    if ((subport % 4) != 0) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Unit %d, _soc_sbx_caladan3_mac_independent:Invalid subport %d\n"), unit, subport));
        return SOC_E_NONE;
    }
    core = subport >> 2;
    if ((core > 2) || (core < 0)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, *** ERROR: Invalid core %d\n"), unit, core));
        return SOC_E_PARAM;
    }

    /* Configure */
    mac_mode = SOC_SBX_CALADAN3_INDEPENDENT_MODE;
    READ_PORT_MODE_REGr(unit, port, &portmode);

    if (reset || 
           (mac_mode != 
               soc_reg_field_get(unit, PORT_MODE_REGr, portmode, MAC_MODEf))) {

        uint32 prev_mac_res = 0;
        uint32 new_mac_res = 0;

        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, MAC_MODEf, mac_mode);

        SOC_IF_ERROR_RETURN(_soc_sbx_caladan3_mac_in_reset(unit, port, subport, mac_mode, 
                                                           &prev_mac_res, &new_mac_res));
        WRITE_PORT_MODE_REGr(unit, port, portmode);

        if (prev_mac_res != new_mac_res) {
            SOC_IF_ERROR_RETURN(_soc_sbx_caladan3_mac_out_reset(unit, port, subport, mac_mode));
        }

    }

    rv = _soc_sbx_caladan3_mac_reconfig(unit, port, mac_mode, 
                                        core, core_mode, phy_mode, gmii_mode);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** ERROR: Mac Reconfig failed port %d\n"), unit, port));
    } else {
#ifdef MAC_PORT_MODE_ACCESS_DEBUG

        uint32 mac_cont = 0;

        READ_PORT_MAC_CONTROLr(unit, port, &mac_cont);

        SOC_ERROR_PRINT((DK_ERR,
                         "Before phy_core_config  Unit %d MAC_CONTROL  port %d  core %d  mac_cont %x\n", 
                         unit, port, core, mac_cont));
#endif

        if (!SOC_RECONFIG_TDM || SOC_CONTROL(unit)->cl0_reset) {
            rv = _soc_sbx_caladan3_phy_core_config(unit, port, core);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d *** ERROR: Phy Reset failed port %d\n"), unit, port));
            }
        }

    }

    return rv;
}
    

#ifdef BRCM_NOT_CALLED_ANYWHERE_BUT_STILL_SUPPORTED
int
_soc_sbx_caladan3_mac_config(int unit, int port, 
                             int mac_mode, int *core_mode, 
                             int *phy_mode, int*gmii_mode)
{
    uint32 portmode = 0, macctrl=0;

    /* Put MAC in Reset */
    soc_reg_field_set(unit, PORT_MAC_CONTROLr, &macctrl, XMAC0_RESETf, 1);
    soc_reg_field_set(unit, PORT_MAC_CONTROLr, &macctrl, XMAC1_RESETf, 1);
    soc_reg_field_set(unit, PORT_MAC_CONTROLr, &macctrl, XMAC2_RESETf, 1);
    soc_reg_field_set(unit, PORT_MAC_CONTROLr, &macctrl, CMAC_RESETf, 1);
    SOC_IF_ERROR_RETURN(WRITE_PORT_MAC_CONTROLr(unit, port, macctrl));
    
    sal_udelay(30);

    /* Configure */
    if (mac_mode == SOC_SBX_CALADAN3_INDEPENDENT_MODE) {
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, XPORT0_CORE_PORT_MODEf, core_mode[0]);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, XPORT1_CORE_PORT_MODEf, core_mode[1]);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, XPORT2_CORE_PORT_MODEf, core_mode[2]);

        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, XPORT0_PHY_PORT_MODEf, phy_mode[0]);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, XPORT1_PHY_PORT_MODEf, phy_mode[1]);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, XPORT2_PHY_PORT_MODEf, phy_mode[2]);

        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, XPC0_GMII_MII_ENABLEf, gmii_mode[0]);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, XPC1_GMII_MII_ENABLEf, gmii_mode[1]);
        soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, XPC2_GMII_MII_ENABLEf, gmii_mode[2]);

        soc_reg_field_set(unit, PORT_MAC_CONTROLr, &macctrl, XMAC0_RESETf, 0);
        soc_reg_field_set(unit, PORT_MAC_CONTROLr, &macctrl, XMAC1_RESETf, 0);
        soc_reg_field_set(unit, PORT_MAC_CONTROLr, &macctrl, XMAC2_RESETf, 0);

    } else {
        soc_reg_field_set(unit, PORT_MAC_CONTROLr, &macctrl, CMAC_RESETf, 0);
    }

    soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, MAC_MODEf, mac_mode);

    /* Finally write out config to regs. Ensure MAC is still in reset prior to
     * writing to PORT_MODE_REG.
     */
    SOC_IF_ERROR_RETURN(WRITE_PORT_MODE_REGr(unit, port, portmode));

    SOC_IF_ERROR_RETURN(WRITE_PORT_MAC_CONTROLr(unit, port, macctrl));

    sal_udelay(30);

    return SOC_E_NONE;
}
#endif

#define SOC_MAX_NUM_CLPORT0_BLKS         (1)
#define SOC_MAX_NUM_XTPORT_BLKS          (3)
#define SOC_MAX_NUM_ILPORT_BLKS          (2)

/*
 * This function puts the specified XMACs into a disabled state as the HW 
 * should have done coming out of reset.
 */
int soc_sbx_caladan3_disable_macs(int unit, soc_block_type_t  *p_block_types, int num_block_types)
{
    int block_type_idx;
    int max_block_type_blocks;
    int skip_xmac_on_this_block;
    int block_number, current_block_number, block_index;
    int block_num, xmac_num;
    int phy_port;
    soc_pbmp_t phy_port_seen;
    soc_port_t port;

    /*
     * Currently only CLPORTS and XTPORTS are supported. If XLPORTS are required add below.
     */
    for (block_type_idx = 0; block_type_idx < num_block_types; block_type_idx++) {

        if (p_block_types[block_type_idx] == SOC_BLK_CLPORT) {
            max_block_type_blocks = SOC_MAX_NUM_CLPORT0_BLKS;
        } else if (p_block_types[block_type_idx] == SOC_BLK_XTPORT) {
            max_block_type_blocks = SOC_MAX_NUM_XTPORT_BLKS;
        }
        else {
            return SOC_E_PARAM;
        }

        for (block_num = 0; block_num < max_block_type_blocks; block_num++) {

            current_block_number = 
                soc_sbx_block_find(unit, p_block_types[block_type_idx], block_num);

            for (xmac_num = 0; xmac_num < SOC_SBX_CALADAN3_CORE_PER_MAC; xmac_num++) {

                skip_xmac_on_this_block = 0;

                if (SOC_RECONFIG_TDM) {
                    SOC_PBMP_CLEAR(phy_port_seen);

                    SOC_PBMP_ITER(SOC_CONTROL(unit)->all_skip_pbm, port) {
                        if (soc_feature(unit, soc_feature_logical_port_num)) {
                            phy_port = SOC_INFO(unit).port_l2p_mapping[port];
                        } else {
                            phy_port = port;
                        }
                        
                        if (SOC_PBMP_MEMBER(phy_port_seen, phy_port)) {
                            continue;
                        }

                        SOC_PBMP_PORT_ADD(phy_port_seen, phy_port);

                        block_number = SOC_PORT_BLOCK(unit, phy_port);
                        block_index = SOC_PORT_BINDEX(unit, phy_port);

                        if (block_number == current_block_number && (block_index >> 2) == xmac_num) {
                            /* 
                             * We found at lease one common port on this block/xmac.
                             * Therefore don't disable this xmac on this block.
                             */
                            skip_xmac_on_this_block = 1;
                            break;
                        }
                    }
                }

                if (!skip_xmac_on_this_block) {
                    soc_sbx_caladan3_xmac_disable(
                        unit, p_block_types[block_type_idx], block_num, xmac_num);
                }
            }
        }
    }
    return SOC_E_NONE;
}



/*
 * Function
 *    soc_sbx_caladan3_mac_init
 * Purpose
 *    Initialize the mac core based on port configuration
 */
int
soc_sbx_caladan3_mac_init(int unit) 
{
    int rv = SOC_E_NONE;
    soc_info_t *si;
    soc_port_t port;
    int core_mode;
    int phy_mode;
    int gmii_mode;
    int numgmii = 0;
    int phy_port, subport, speed;
    int count = 0;
    soc_pbmp_t phy_port_seen;
    int ucode_num_port_override;
    int core;
    uint32 soft_reset;
    soc_block_type_t  p_block_types[] = {SOC_BLK_CLPORT, SOC_BLK_XTPORT}; 

#ifdef BCM_WARM_BOOT_SUPPORT
    if(SOC_WARM_BOOT(unit))
    {
        return rv;
    }
#endif /* BCM_WARM_BOOT_SUPPORT */


    si  = &SOC_INFO(unit);

    rv = soc_sbx_caladan3_disable_macs(unit, p_block_types, 
                                       sizeof(p_block_types)/sizeof(soc_block_type_t));
    if (SOC_FAILURE(rv)) {
        return rv;
    }

    SOC_PBMP_CLEAR(phy_port_seen);

    SOC_PBMP_ITER(PBMP_PORT_ALL(unit), port) {

        if (SOC_RECONFIG_TDM) {
            if (PBMP_MEMBER(SOC_CONTROL(unit)->all_skip_pbm, port)) {
                continue;
            }
        }

        core_mode = phy_mode  = gmii_mode = 0;
        if (soc_feature(unit, soc_feature_logical_port_num)) {
            phy_port = si->port_l2p_mapping[port];
        } else {
            phy_port = port;
        }

        subport = SOC_PORT_BINDEX(unit, phy_port);
        core = subport >> 2;

        ucode_num_port_override = soc_property_get(unit, "ucode_num_port_override", 0);
        if (!ucode_num_port_override) {
            if (SOC_PBMP_MEMBER(phy_port_seen, phy_port)) {
                continue;
            }
        }
        SOC_PBMP_PORT_ADD(phy_port_seen, phy_port);

        if (!PBMP_MEMBER(SOC_CONTROL(unit)->mac_phy_skip_pbm, port)) {

            if (IS_IL_PORT(unit, port) || IS_CE_PORT(unit, port)) {

                rv = _soc_sbx_caladan3_mac_aggregated(unit, port, subport);
                if (SOC_FAILURE(rv)) {
                    return rv;
                }
            } else if (IS_HG_PORT(unit, port) &&
                       (si->port_speed_max[port] > 
                        (42*SOC_SBX_CALADAN3_GIG_SPEED))) {

                rv = _soc_sbx_caladan3_mac_aggregated(unit, port, subport);
                if (SOC_FAILURE(rv)) {
                    return rv;
                }
            } else if (soc_property_port_get(unit, port, spn_SERDES_RXAUI_MODE, 0)) {
                if ((subport % 2) == 0) {

                    speed = si->port_speed_max[port];
                    core_mode = SOC_SBX_CALADAN3_CORE_PORT_DUAL_MODE;
                    phy_mode  = SOC_SBX_CALADAN3_PHY_PORT_QUAD_MODE;
                    gmii_mode = 0;
                    si->port_num_lanes[port] = 2;
                    rv = _soc_sbx_caladan3_mac_independent(unit, port, subport, 
                                                           core_mode, phy_mode, 
                                                           gmii_mode, TRUE);
                    if (SOC_FAILURE(rv)) {
                        return rv;
                    }
                }
            } else {
                if ((subport % 4) == 0) {
                    speed = si->port_speed_max[port];
                    if (speed >= (20*SOC_SBX_CALADAN3_GIG_SPEED)) {
                        /* 40 G */
                        core_mode = SOC_SBX_CALADAN3_CORE_PORT_SINGLE_MODE;
                        phy_mode  = SOC_SBX_CALADAN3_CORE_PORT_SINGLE_MODE;
                        gmii_mode = 0;
                    } else if (speed > (10*SOC_SBX_CALADAN3_GIG_SPEED)) {
                        /* 20 G */
                        core_mode = SOC_SBX_CALADAN3_CORE_PORT_DUAL_MODE;
                        phy_mode  = SOC_SBX_CALADAN3_PHY_PORT_DUAL_MODE;
                        gmii_mode = 0;
                    } else {
                        /* 1G/10G */
                        core_mode = SOC_SBX_CALADAN3_CORE_PORT_QUAD_MODE;
                        phy_mode  = SOC_SBX_CALADAN3_PHY_PORT_QUAD_MODE;
                        gmii_mode = 
                            (speed <= (1*SOC_SBX_CALADAN3_GIG_SPEED)) ? 1 : 0;

                    }
                    rv = _soc_sbx_caladan3_mac_independent(unit, port, subport,
                                                           core_mode, phy_mode, 
                                                           gmii_mode, TRUE);
                    if (SOC_FAILURE(rv)) {
                        return rv;
                    }
                }
            }

            if (SOC_HOTSWAP_TDM && !SOC_CONTROL(unit)->cl0_reset) {

                SOC_IF_ERROR_RETURN(READ_PORT_SOFT_RESETr(unit, port, &soft_reset));
                soc_reg_field_set(unit, PORT_SOFT_RESETr, &soft_reset,
                    port_soft_reset_xport_field[core], 0);
                SOC_IF_ERROR_RETURN(WRITE_PORT_SOFT_RESETr(unit, port, soft_reset));
            }

            /*XXXTBD This needs to be moved to bcm/port.c where port_enable is now done*/
            if (soc_property_port_get(unit, port, spn_SERDES_RXAUI_MODE, 0) && ((subport % 2) != 0)) {
                /* Don't enable RXAUI port if odd subport - not used */
            }


            /* config the higig modes */
            if ((IS_HG_PORT(unit, port)) && (!SAL_BOOT_PLISIM)) {
                uint32 regval = 0;
                READ_PORT_CONFIGr(unit, port, &regval);
                if (soc_property_port_get(unit, port, spn_HIGIG2_HDR_MODE, 0)) {
                    soc_reg_field_set(unit, PORT_CONFIGr, 
                                  &regval, HIGIG2_MODEf, 1);
                } else {
                    soc_reg_field_set(unit, PORT_CONFIGr, 
                                      &regval, HIGIG_MODEf, 1);
                }
                WRITE_PORT_CONFIGr(unit, port, regval);
            }
            
        }
    } 

        /* Enable appropriate modes */
        count = 0;
        SOC_PBMP_ITER(si->cl.bitmap, port) {
            if (soc_sbx_caladan3_is_line_port(unit, port)) {
                if ((si->port_speed_max[port] <= 1000) &&
                    (soc_property_port_get(unit, port, spn_SERDES_QSGMII_SGMII_OVERRIDE, 1) == 1)) {
                    numgmii++;
                }
                count++;
            }
        }

/*    if ((numgmii > 0) && (count != numgmii)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d *** ERROR: Misconfigured CLPORT detected, "
                              "cannot mix gmii and no-gmii modes on CLPORT\n"), unit));
        return SOC_E_PARAM;
    }
*/
    _soc_sbx_caladan3_clport_gmii_mode(unit, (numgmii > 0) ? 1 : 0);
    rv = soc_sbx_caladan3_wc_mode_set(unit);
    
    _phy_wcmod_firmware_set_helper[unit] = 
          soc_sbx_caladan3_wc_image_download_helper;

    return rv;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_port_init
 * Purpose:
 *     port driver init
 */
void soc_sbx_caladan3_port_init(int unit)
{
    _caladan3_port_map[unit] = NULL;
}

int soc_sbx_caladan3_port_info_update(int unit)
{
    int  status = SOC_E_NONE;
    int port, max_port;
    uint8 *bindex_in_use = NULL;
    int bindex_intf_inst = 0;
    int lport;
    soc_sbx_caladan3_port_map_t *port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    soc_sbx_caladan3_port_config_t  *interface_config = SOC_CONTROL(unit)->interface_config;
    soc_sbx_caladan3_port_queue_t   *port_queue_config = SOC_CONTROL(unit)->port_queue_config;
    soc_sbx_caladan3_port_map_info_t    *line_port_info;
    soc_sbx_caladan3_port_map_info_t    *fabric_port_info;
    int fabric_port;
    int fabric_bindex = 0;
    int pport;
    int blk, cl0_blk;
    int cl0_reset;
#if RECONFIG_DEBUG
    char                pfmt[SOC_PBMP_FMT_LEN];
#endif


    bindex_in_use = sal_alloc(sizeof(uint8) * SOC_SBX_CALADAN3_MAX_WCORE_LINE_COUNT, "bindex_in_use");
    if (bindex_in_use == NULL) {
        status = SOC_E_MEMORY;
        goto err;
    }
    sal_memset(bindex_in_use, FALSE, sizeof(uint8) * SOC_SBX_CALADAN3_MAX_WCORE_LINE_COUNT);

    max_port = 0;
    port_map->max_line_bw = 0;
    port_map->max_fabric_bw = 0;

    for(lport = 0; lport < SOC_SBX_CALADAN3_MAX_LINE_PORT; lport++) {
        if (!interface_config[lport].valid) {
            continue;
        }
        max_port++;

        line_port_info = &port_map->line_port_info[lport];
        fabric_port_info = &port_map->fabric_port_info[lport];

        switch(interface_config[lport].if_type) {
            case SOC_PORT_IF_XGMII:
                line_port_info->blktype = SOC_BLK_CLPORT;
                line_port_info->clienttype = SOC_SBX_CALADAN3_CLIENT_CL;
                line_port_info->instance = 0;
                if (interface_config[lport].encaps == SOC_ENCAP_IEEE) {
                    line_port_info->intftype = SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE;
                    line_port_info->intf_instance = 0;
                    line_port_info->bindex = 0;

                    fabric_port_info->bindex = 0;
                } else {
                    line_port_info->intf_instance = interface_config[lport].phy_port / 4;
                    line_port_info->bindex = interface_config[lport].phy_port % 4;

                    switch (interface_config[lport].speed) {
                        case 10000:
                            line_port_info->intftype = SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10;
                            bindex_intf_inst = (line_port_info->intf_instance << 2) +
                                line_port_info->bindex;
                            if (bindex_in_use[bindex_intf_inst] == TRUE) {
                                line_port_info->flags |= SOC_SBX_CALADAN3_IS_CHANNELIZED_SUBPORT;
                            } else {
                                line_port_info->flags &= ~SOC_SBX_CALADAN3_IS_CHANNELIZED_SUBPORT;
                                bindex_in_use[bindex_intf_inst] = TRUE;
                            }
                            break;
                        case 25000:
                            line_port_info->intftype = SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25;
                            break;
                        case 42000:
                            line_port_info->intftype = SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42;
                            break;
                        default:
                            break;
                    }

                    fabric_port_info->bindex = interface_config[lport].phy_port;
                }
                break;

            case SOC_PORT_IF_XLAUI:
                line_port_info->blktype = SOC_BLK_CLPORT;
                line_port_info->clienttype = SOC_SBX_CALADAN3_CLIENT_CL;
                line_port_info->instance = 0;
                line_port_info->intftype = SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE;
                line_port_info->intf_instance = interface_config[lport].phy_port;
                line_port_info->bindex = 0;

                fabric_port_info->bindex = 0;
                break;

            case SOC_PORT_IF_SFI:
                line_port_info->blktype = SOC_BLK_CLPORT;
                line_port_info->clienttype = SOC_SBX_CALADAN3_CLIENT_CL;
                line_port_info->instance = 0;
                line_port_info->intftype = SOC_SBX_CALADAN3_PORT_INTF_CLPORT_10GE;
                line_port_info->intf_instance = interface_config[lport].phy_port / 4;
                line_port_info->bindex = interface_config[lport].phy_port % 4;
                break;

            case SOC_PORT_IF_GMII:
                if (interface_config[lport].phy_port < 12) {
                    line_port_info->blktype = SOC_BLK_CLPORT;
                    line_port_info->clienttype = SOC_SBX_CALADAN3_CLIENT_CL;
                    line_port_info->instance = 0;
                    line_port_info->intftype = SOC_SBX_CALADAN3_PORT_INTF_CLPORT_1GE;
                } else {
                    line_port_info->blktype = SOC_BLK_XTPORT;
                    line_port_info->clienttype = SOC_SBX_CALADAN3_CLIENT_XT;
                    line_port_info->instance = (interface_config[lport].phy_port - 12) /12;
                    line_port_info->intftype = SOC_SBX_CALADAN3_PORT_INTF_XTPORT;
                }
                line_port_info->intf_instance = 0;
                line_port_info->bindex = interface_config[lport].phy_port % 12;

                fabric_port_info->bindex = interface_config[lport].phy_port;
                break;

            case SOC_PORT_IF_DNX_XAUI:
                line_port_info->blktype = SOC_BLK_CLPORT;
                line_port_info->clienttype = SOC_SBX_CALADAN3_CLIENT_CL;
                line_port_info->instance = 0;
                line_port_info->intftype = SOC_SBX_CALADAN3_PORT_INTF_CLPORT_XAUI_10GE;
                line_port_info->intf_instance = interface_config[lport].phy_port / 4;
                line_port_info->bindex = interface_config[lport].phy_port % 4;

                fabric_port_info->bindex = interface_config[lport].phy_port;
                break;

            case SOC_PORT_IF_ILKN:
                line_port_info->blktype = SOC_BLK_IL;
                line_port_info->clienttype = SOC_SBX_CALADAN3_CLIENT_IL;
                line_port_info->instance = 0;
                if (interface_config[lport].speed == 50000) {
                    line_port_info->intftype = SOC_SBX_CALADAN3_PORT_INTF_IL50n;
                } else {
                    line_port_info->intftype = SOC_SBX_CALADAN3_PORT_INTF_IL100;
                }
                line_port_info->intf_instance = 0;
                line_port_info->bindex = 0;
                break;

            default:
                break;
        }
        line_port_info->blk =
            soc_sbx_block_find(unit, line_port_info->blktype,
            line_port_info->instance);
        line_port_info->port = -1;
        line_port_info->uport = lport + 1;
        line_port_info->physical_intf = SOC_SBX_CALADAN3_LINE_WCORE;
        line_port_info->flow_control = SOC_SBX_CALADAN3_FC_TYPE_MAX;
        line_port_info->base_port = -1;
        line_port_info->port_queues.squeue_base = port_queue_config[lport].line_sq_base;
        line_port_info->port_queues.dqueue_base = port_queue_config[lport].line_dq_base;
        line_port_info->port_queues.num_squeue = port_queue_config[lport].sq_count;
        line_port_info->port_queues.num_dqueue = port_queue_config[lport].dq_count;
        /* Update bmp */
        if (soc_sbx_caladan3_port_queues_update_bmp(unit, &line_port_info->port_queues) < 0) {
            sal_free(bindex_in_use);
            LOG_CLI((BSL_META_U(unit,
                                "%s: Line port %d update bmp failed!\n"), __FUNCTION__, lport));
            return SOC_E_PARAM;
        }


        fabric_port = port_queue_config[lport].fabric_port;
        fabric_port_info->flow_control = SOC_SBX_CALADAN3_FC_TYPE_MAX;
        fabric_port_info->port_queues.num_squeue = port_queue_config[lport].sq_count;
        fabric_port_info->port_queues.num_dqueue = port_queue_config[lport].dq_count;
        fabric_port_info->base_port = -1;
        fabric_port_info->port = -1;
        fabric_port_info->uport = -1;
        fabric_port_info->port_queues.squeue_base = port_queue_config[lport].fabric_sq_base;
        fabric_port_info->port_queues.dqueue_base = port_queue_config[lport].fabric_dq_base;
        fabric_port_info->physical_intf = SOC_SBX_CALADAN3_FABRIC_WCORE;
        if (soc_sbx_caladan3_port_queues_update_bmp(unit, &fabric_port_info->port_queues) < 0) {
            sal_free(bindex_in_use);
            LOG_CLI((BSL_META_U(unit,
                                "%s: Fabric port %d update bmp failed!\n"), __FUNCTION__, fabric_port));
            return SOC_E_PARAM;
        }

        if (!interface_config[fabric_port].valid) {
            sal_free(bindex_in_use);
            LOG_CLI((BSL_META_U(unit,
                                "%s: Fabric port %d is not valid!\n"), __FUNCTION__, fabric_port));
            return SOC_E_PARAM;
        }

        if (interface_config[fabric_port].if_type == SOC_PORT_IF_ILKN) {
            fabric_port_info->blktype = SOC_BLK_IL;
            fabric_port_info->clienttype = SOC_SBX_CALADAN3_CLIENT_IL;
            fabric_port_info->instance = 1;
            fabric_port_info->intftype = SOC_SBX_CALADAN3_PORT_INTF_IL100;
            fabric_port_info->intf_instance = 0;
            fabric_port_info->bindex = fabric_bindex++;
        } else if (interface_config[fabric_port].if_type == SOC_PORT_IF_XGMII) {
            fabric_port_info->blktype = SOC_BLK_CLPORT;
            fabric_port_info->clienttype = SOC_SBX_CALADAN3_CLIENT_CL;
            fabric_port_info->instance = 1;
            fabric_port_info->intftype = SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126;
            fabric_port_info->intf_instance = 0;
            fabric_port_info->bindex = fabric_bindex++;
        }
        fabric_port_info->blk =
            soc_sbx_block_find(unit, fabric_port_info->blktype,
            fabric_port_info->instance);

        port_map->num_1g_ports += (sbx_caladan3_intf_attr[line_port_info->intftype].speed == 1) ? 1 : 0;

        line_port_info->flags |= SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID;
        fabric_port_info->flags |= SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID;

    }

    port_map->num_ports_found = max_port;
    if (max_port > 0) {
        max_port += SOC_SBX_CALADAN3_NUM_RESV_PORTS;
    }
    port_map->max_port = max_port;

    if (!port_map->reserved_ports_configured) {
        /* Provision CMIC, XL, resv info */
        status = _soc_sbx_caladan3_resv_port_info_load(unit, port_map, port_map->first_reserved_port);
        if (SOC_FAILURE(status))
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, *** ERROR: Reserved Port load failed\n"), unit));
    }

    if (SOC_SUCCESS(status)) {
        status = _soc_sbx_caladan3_port_info_config(unit, port_map);
        if (SOC_FAILURE(status))
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, *** ERROR: PBMP setup failed\n"), unit));
    }

    if (SOC_FAILURE(status)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d, *** ERROR: Failed to initialize"
                              " Port mapping [FATAL] !!!!\n"), unit));
        goto err;
    }

    if (SOC_RECONFIG_TDM) {

#if RECONFIG_DEBUG
        LOG_CLI((BSL_META_U(unit,
                            "%s: Old ports:      %s\n"), __FUNCTION__, SOC_PBMP_FMT(SOC_CONTROL(unit)->all_skip_pbm, pfmt)));
#endif
        SOC_PBMP_AND(SOC_CONTROL(unit)->all_skip_pbm, PBMP_ALL(unit));
        SOC_PBMP_ITER(SOC_CONTROL(unit)->all_skip_pbm, port) {
            if (!soc_sbx_caladan3_is_line_port(unit, port)) {
                continue;
            }
            if (SOC_CONTROL(unit)->port_l2p_mapping[port] !=
                SOC_INFO(unit).port_l2p_mapping[port] ||
                SOC_CONTROL(unit)->intftype[port] !=
                port_map->line_port_info[port].intftype) {

                SOC_PBMP_PORT_REMOVE(SOC_CONTROL(unit)->all_skip_pbm, port);

#if RECONFIG_DEBUG
                LOG_CLI((BSL_META_U(unit,
                                    "%s: Port %d not the same - pruning\n"), __FUNCTION__, port));
#endif
            }
        }

        cl0_blk = soc_sbx_block_find(unit, SOC_BLK_CLPORT, 0);
        cl0_reset = TRUE;
        SOC_PBMP_ITER(SOC_CONTROL(unit)->all_skip_pbm, port) {
            if (soc_feature(unit, soc_feature_logical_port_num)) {
                pport = SOC_INFO(unit).port_l2p_mapping[port];
            } else {
                pport = port;
            }
            blk = SOC_PORT_BLOCK(unit, pport);
            if (blk == cl0_blk) {
                cl0_reset = FALSE;
                break;
            }
        }
        if (cl0_reset) {
            SOC_CONTROL(unit)->cl0_reset = TRUE;

            /* Remove all ports that are on the CL0 block */
            SOC_PBMP_ITER(SOC_CONTROL(unit)->all_skip_pbm, port) {
                if (soc_feature(unit, soc_feature_logical_port_num)) {
                    pport = SOC_INFO(unit).port_l2p_mapping[port];
                } else {
                    pport = port;
                }
                blk = SOC_PORT_BLOCK(unit, pport);
                if (blk == cl0_blk) {
                    SOC_PBMP_PORT_REMOVE(SOC_CONTROL(unit)->all_skip_pbm, port);
                    LOG_CLI((BSL_META_U(unit,
                                        "%s: Pruning CL0 port %d\n"), __FUNCTION__, port));
                }
            }
        } else {
            SOC_CONTROL(unit)->cl0_reset = FALSE;

            soc_sbx_caladan3_calc_phy_mac_skip_ports(unit);
        }
#if RECONFIG_DEBUG
        LOG_CLI((BSL_META_U(unit,
                            "%s: Current ports:  %s\n"), __FUNCTION__, SOC_PBMP_FMT(PBMP_ALL(unit), pfmt)));
        LOG_CLI((BSL_META_U(unit,
                            "%s: Common ports:   %s\n"), __FUNCTION__, SOC_PBMP_FMT(SOC_CONTROL(unit)->all_skip_pbm, pfmt)));
        LOG_CLI((BSL_META_U(unit,
                            "%s: Phy skip ports: %s\n"), __FUNCTION__, SOC_PBMP_FMT(SOC_CONTROL(unit)->mac_phy_skip_pbm, pfmt)));
#endif

    }

err:
    if (bindex_in_use) {
        sal_free(bindex_in_use);
    }
    return status;
}
int soc_sbx_caladan3_hotswap(int unit)
{
    int                 status = SOC_E_NONE;
    uint16              dev_id, dev_id_driver;
    uint8               rev_id, rev_id_driver;
    soc_persist_t       *sop;
    bcm_port_t          port;

#if RECONFIG_DEBUG
    char                pfmt[SOC_PBMP_FMT_LEN];
#endif

    soc_cm_get_id(unit, &dev_id, &rev_id);
    soc_cm_get_id_driver(dev_id, rev_id, &dev_id_driver, &rev_id_driver);

    status = soc_counter_detach(unit);
    if (status != SOC_E_NONE) {
        LOG_CLI((BSL_META_U(unit,
                            "%s: soc_counter_detach failed: %s\n"),
                 FUNCTION_NAME(), soc_errmsg(status)));
    }

    /* Get the current port information */
    SOC_PBMP_ASSIGN(SOC_CONTROL(unit)->all_skip_pbm, PBMP_ALL(unit));

#if RECONFIG_DEBUG
        cli_out("%s: Assigning all_skip ports:  %s\n",
                     __FUNCTION__, SOC_PBMP_FMT(SOC_CONTROL(unit)->all_skip_pbm, pfmt));
#endif

    for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
        SOC_CONTROL(unit)->port_l2p_mapping[port] =
            SOC_INFO(unit).port_l2p_mapping[port];
        soc_sbx_caladan3_get_intftype(unit, port,
            &SOC_CONTROL(unit)->intftype[port]);
    }

    status = soc_sbx_init(unit);
    if (status != SOC_E_NONE) {
        LOG_CLI((BSL_META_U(unit,
                            "%s: soc_sbx_init failed: %s\n"),
                 FUNCTION_NAME(), soc_errmsg(status)));
    }

    /* The IPG values used to program the MACs are normally configured 
    * only during the attach. But it must be done again here. 
    */ 
    sop = SOC_PERSIST(unit); 
    PBMP_ALL_ITER(unit, port) { 
        sop->ipg[port].hd_10    = 96; 
        sop->ipg[port].hd_100   = 96; 
        sop->ipg[port].hd_1000  = 96; 
        sop->ipg[port].hd_2500  = 96; 

        sop->ipg[port].fd_10    = 96; 
        sop->ipg[port].fd_100   = 96; 
        sop->ipg[port].fd_1000  = 96; 
        sop->ipg[port].fd_2500  = 96; 
        sop->ipg[port].fd_10000 = 96; 
        sop->ipg[port].fd_xe    = 96; 
        sop->ipg[port].fd_hg    = 64; 
    } 

    status = soc_counter_attach(unit);
    if (status != SOC_E_NONE) {
        LOG_CLI((BSL_META_U(unit,
                            "%s: soc_counter_attach failed: %s\n"),
                 FUNCTION_NAME(), soc_errmsg(status)));
    }

    /* End of tdm reset */

    c3_port_init[unit] = 0;

    return SOC_E_NONE;
}



/*
 *
 * Function:
 *     soc_sbx_caladan3_port_remove
 * Purpose:
 *     Removes a port from the system.
 */
int soc_sbx_caladan3_port_remove(int unit, soc_port_t lport)
{
    int                                 blk;
    soc_block_type_t                    blktype;
    soc_sbx_caladan3_port_map_t         *port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    soc_sbx_caladan3_port_map_info_t    *line_info;
    int                                 status = SOC_E_NONE;
    int                                 subport;
    soc_info_t                          *si;
    soc_port_t                          phy_port;
    soc_port_t                          tmp_port;
    int                                 wcidx;
    int                                 tmp_blk, tmp_wcidx;
    int                                 wc_count;

    uint32                              portmode;
    uint32                              mac_control;
    uint32                              soft_reset;
    uint32                              mac_cont = ~0;

    if (port_map == NULL) {
        return SOC_E_INTERNAL;
    }

    /* Make sure port exists. Cannot remove CMIC, XL and fabric ports */
    if (!(port_map->line_port_info[lport].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID) ||
        lport >= SOC_SBX_CALADAN3_MAX_LINE_PORT) {
        return SOC_E_PARAM;
    }

    /* Clear phy table entry for the port */
    status = soc_phyctrl_software_port_delete(unit, lport);
    if (status != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d soc_phyctrl_software_port_delete port %d failed: %s\n"),
                   unit, lport, soc_errmsg(status)));
    }

    /* Stop the counter collection for the port */
    COUNTER_LOCK(unit);
    SOC_PBMP_PORT_REMOVE(SOC_CONTROL(unit)->counter_pbmp, lport);
    COUNTER_UNLOCK(unit);

    /* Disable port and clear MIB counters */
    subport = (port_map->line_port_info[lport].intf_instance << 2) +
        port_map->line_port_info[lport].bindex;
    soc_sbx_caladan3_port_enable(unit, lport, subport, 0);

    /* Set the credits to zero */
    status = soc_sbx_caladan3_pt_port_update(unit, 0, lport,
        0, 0, 0, FALSE, 0, TRUE);
        
    wcidx = subport >> 2;

    si = &SOC_INFO(unit);

    /* Get physical port */
    if (soc_feature(unit, soc_feature_logical_port_num)) {
        phy_port = si->port_l2p_mapping[lport];
    } else {
        phy_port = lport;
    }

    blk = SOC_PORT_IDX_BLOCK(unit, phy_port, 0);
    blktype = SOC_BLOCK_INFO(unit, blk).type;

    /* If there are no more ports on this warp core then disable it */
    for (tmp_port = 0, wc_count = 0; tmp_port < SOC_SBX_CALADAN3_MAX_LINE_PORT; tmp_port++) {

        line_info = &port_map->line_port_info[tmp_port];

        if (!(line_info->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID) ||
            tmp_port == lport) {
            continue;
        }

        if (soc_feature(unit, soc_feature_logical_port_num)) {
            phy_port = si->port_l2p_mapping[tmp_port];
        } else {
            phy_port = tmp_port;
        }

        subport = (port_map->line_port_info[tmp_port].intf_instance << 2) +
            port_map->line_port_info[tmp_port].bindex;
        tmp_wcidx = subport >> 2;
        
        tmp_blk = SOC_PORT_IDX_BLOCK(unit, phy_port, 0);
        if (tmp_blk == blk && wcidx == tmp_wcidx) {
            wc_count++;
        }
    }

    if (wc_count == 0) {

        if (port_map->line_port_info[lport].intftype !=
            SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE) {
            SOC_IF_ERROR_RETURN(READ_PORT_SOFT_RESETr(unit, lport, &soft_reset));
            soc_reg_field_set(unit, PORT_SOFT_RESETr, &soft_reset,
                port_soft_reset_xport_field[wcidx], 1);
            SOC_IF_ERROR_RETURN(WRITE_PORT_SOFT_RESETr(unit, lport, soft_reset));
        }

        /* MAC needs to be in reset to modify PORT_MODE_REG */
        SOC_IF_ERROR_RETURN(READ_PORT_MAC_CONTROLr(unit, lport, &mac_control));
        soc_reg_field_set(unit, PORT_MAC_CONTROLr, &mac_control,
            xmac_control_reset_field[wcidx], 1);
        SOC_IF_ERROR_RETURN(WRITE_PORT_MAC_CONTROLr(unit, lport, mac_control));

        sal_udelay(30);

        READ_PORT_MAC_CONTROLr(unit, lport, &mac_cont); /*XXXTTT*/
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "MAC_CONTROL AFT port %d  mac_cont %x\n"),
                  lport, mac_cont));


        /* Update PORT_MODE_REG */
        SOC_IF_ERROR_RETURN(READ_PORT_MODE_REGr(unit, lport, &portmode));

        if (wcidx == 0) {
            soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                              XPORT0_CORE_PORT_MODEf, SOC_SBX_CALADAN3_CORE_PORT_DISABLE);
            soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                              XPORT0_PHY_PORT_MODEf, SOC_SBX_CALADAN3_CORE_PORT_SINGLE_MODE);
        } else if (wcidx == 1) {
            soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                              XPORT1_CORE_PORT_MODEf, SOC_SBX_CALADAN3_CORE_PORT_DISABLE);
            soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                              XPORT1_PHY_PORT_MODEf, SOC_SBX_CALADAN3_CORE_PORT_SINGLE_MODE);
        } else if (wcidx == 2) {
            soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                              XPORT2_CORE_PORT_MODEf, SOC_SBX_CALADAN3_CORE_PORT_DISABLE);
            soc_reg_field_set(unit, PORT_MODE_REGr, &portmode, 
                              XPORT2_PHY_PORT_MODEf, SOC_SBX_CALADAN3_CORE_PORT_SINGLE_MODE);
        }
        SOC_IF_ERROR_RETURN(WRITE_PORT_MODE_REGr(unit, lport, portmode));
    }

    /* Clear port speed */
    si->port_speed_max[lport] = 0;

    /* Remove port from port type bitmaps */
    switch(blktype) {
        
        case SOC_BLK_CLPORT:
            switch(port_map->line_port_info[lport].intftype) {
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE:
                SBX_REMOVE_PORT(ce, lport);
                break;
                
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_10GE:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_XAUI_10GE:
                SBX_REMOVE_PORT(xe, lport);
                break;
                
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_1GE:
                SBX_REMOVE_PORT(ge, lport);
                break;
                
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25:
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42:
                SBX_REMOVE_PORT(hg, lport);
                break;
                
            default:
                break;
            }
            SBX_REMOVE_PORT(ether, lport);
            SBX_REMOVE_PORT(cl, lport);
            break;

        case SOC_BLK_XTPORT:
            SBX_REMOVE_PORT(ge, lport);
            SBX_REMOVE_PORT(xt, lport);
            SBX_REMOVE_PORT(ether, lport);
            break;

        default:
            break;
    }

    si->block_valid[blk] -= 1;
    sal_memset(si->port_name[lport], 0, sizeof(si->port_name[lport]));

    /* Clear logical to physical and physical to logical port mappings */
    si->port_l2p_mapping[lport] = -2;
    si->port_p2l_mapping[phy_port] = 0;

    /* Remove port from internal bitmaps */
    SOC_PBMP_PORT_REMOVE(si->block_bitmap[blk], lport);
    SBX_REMOVE_PORT(all, lport);
    SBX_REMOVE_PORT(port, lport);
    si->port_type[lport] = 0;

    port_map->max_line_bw -=
        sbx_caladan3_intf_attr[port_map->line_port_info[lport].intftype].speed;
    port_map->num_1g_ports -=
        (sbx_caladan3_intf_attr[port_map->line_port_info[lport].intftype].speed == 1) ? 1 : 0;

    /* Clear port map table entry */
    sal_memset(&port_map->line_port_info[lport], 0, sizeof(soc_sbx_caladan3_port_map_info_t));
    sal_memset(&port_map->fabric_port_info[lport], 0, sizeof(soc_sbx_caladan3_port_map_info_t));

    /* If necessary update block port */
    if (si->block_port[blk] == lport) {
        si->block_port[blk] = REG_PORT_ANY;
        for(tmp_port = 0; tmp_port < SOC_SBX_CALADAN3_MAX_LINE_PORT; tmp_port++) {
            if ((port_map->line_port_info[tmp_port].flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID) &&
                port_map->line_port_info[tmp_port].blk == blk) {
                si->block_port[blk] = tmp_port;
                break;
            }
        }
    }

    return SOC_E_NONE;
}




/*
 *
 * Function:
 *     soc_sbx_caladan3_update_interface_config
 * Purpose:
 *     Updates the BCM port configuration data.
 */
int soc_sbx_caladan3_update_interface_config(int unit, soc_sbx_caladan3_port_config_t *config)
{
    soc_sbx_caladan3_port_map_t         *port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    soc_port_t                          port;
    soc_sbx_caladan3_port_map_info_t    *line_info;
    soc_sbx_caladan3_port_map_info_t    *fabric_info;
    soc_port_t                          fabric_port;


    for (port = 0; port < SOC_SBX_CALADAN3_PORT_MAP_ENTRIES; port++) {

        line_info = &port_map->line_port_info[port];
        fabric_info = &port_map->fabric_port_info[port];

        if (!(line_info->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
            continue;
        }

        switch(line_info->intftype) {

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_1GE:
                config[port].encaps = SOC_ENCAP_IEEE;
                config[port].if_type = SOC_PORT_IF_GMII;
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_10GE:
                config[port].encaps = SOC_ENCAP_IEEE;
                config[port].if_type = SOC_PORT_IF_SFI;
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_XAUI_10GE:
                config[port].encaps = SOC_ENCAP_IEEE;
                config[port].if_type = SOC_PORT_IF_DNX_XAUI;
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_40GE:
                config[port].encaps = SOC_ENCAP_IEEE;
                config[port].if_type = SOC_PORT_IF_XLAUI;
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_100GE:
                config[port].encaps = SOC_ENCAP_IEEE;
                config[port].if_type = SOC_PORT_IF_XGMII;
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG10:
                config[port].encaps = SOC_ENCAP_HIGIG2;
                config[port].speed = 10000;
                config[port].if_type = SOC_PORT_IF_XGMII;
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25:
                config[port].encaps = SOC_ENCAP_HIGIG2;
                config[port].speed = 25000;
                config[port].if_type = SOC_PORT_IF_XGMII;
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42:
                config[port].encaps = SOC_ENCAP_HIGIG2;
                config[port].speed = 42000;
                config[port].if_type = SOC_PORT_IF_XGMII;
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126:
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_XTPORT:
                config[port].encaps = SOC_ENCAP_IEEE;
                config[port].if_type = SOC_PORT_IF_GMII;
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_IL50w:
            case SOC_SBX_CALADAN3_PORT_INTF_IL50n:
                config[port].encaps = SOC_ENCAP_IEEE;
                config[port].speed = 50000;
                config[port].if_type = SOC_PORT_IF_ILKN;
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_IL100:
                config[port].encaps = SOC_ENCAP_IEEE;
                config[port].if_type = SOC_PORT_IF_ILKN;
                break;

            default:
                break;
        }
        config[port].phy_port = line_info->base_port;
        config[port].valid = TRUE;

        /* Get the fabric port configuration */
        fabric_port = port_map->fabric_port_info[port].port;
        switch(fabric_info->intftype) {
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG126:
                config[fabric_port].encaps = SOC_ENCAP_HIGIG2;
                config[fabric_port].speed = 127000;
                config[fabric_port].if_type = SOC_PORT_IF_XGMII;
                config[fabric_port].valid = TRUE;
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG25:
                config[fabric_port].encaps = SOC_ENCAP_HIGIG2;
                config[fabric_port].speed = 25000;
                config[fabric_port].if_type = SOC_PORT_IF_XGMII;
                config[fabric_port].valid = TRUE;
                break;
            case SOC_SBX_CALADAN3_PORT_INTF_IL100:
            case SOC_SBX_CALADAN3_PORT_INTF_IL50w:
            case SOC_SBX_CALADAN3_PORT_INTF_IL50n:
                config[fabric_port].if_type = SOC_PORT_IF_ILKN;
                config[fabric_port].valid = TRUE;
                break;

            case SOC_SBX_CALADAN3_PORT_INTF_CLPORT_HG42:
                config[fabric_port].encaps = SOC_ENCAP_HIGIG2;
                config[fabric_port].speed = 42000;
                config[fabric_port].if_type = SOC_PORT_IF_XGMII;
                config[fabric_port].valid = TRUE;
                break;

            default:
                LOG_CLI((BSL_META_U(unit,
                                    "%s: Unexpected fabric port(%d) type %d\n"),
                         __FUNCTION__, fabric_port, fabric_info->intftype));
                break;
        }
    }

    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_flush_ports
 * Purpose:
 *     Flush the line destination queue(s) for the specified
 *     port(s).
 */
int soc_sbx_caladan3_flush_ports(int unit, pbmp_t *pbmp)
{
    int                                 status = SOC_E_NONE;
    soc_port_t                          port;
    uint32                              regval;
    int                                 port_phy_state[SOC_MAX_NUM_PORTS];
    soc_sbx_caladan3_port_map_t         *port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    uint32                              safc_cfg0, safc_cfg1;
    uint32                              chan_cfg0, chan_cfg1;
    uint32                              port_bitmap0 = 0xffffffff, port_bitmap1 = 0xffffffff;
    uint32                              chan_bitmap0 = 0xffffffff, chan_bitmap1 = 0xffffffff;
    int                                 index;
    soc_sbx_caladan3_port_map_info_t    *line_port_info;
    soc_sbx_caladan3_queues_t           *qinfo;
    int                                 is_channelized;
    int                                 requires_phy_setup;
    int                                 n_dq = 0;
    char                                dq_list[64];
    int                                 max_checks = 10;
    uint32                              entry[SOC_MAX_MEM_WORDS];
    uint32                              pages_stored;


    sal_memset(dq_list, 0, sizeof(dq_list));
    sal_memset(port_phy_state, 0, sizeof(port_phy_state));
    
    /* Save current port flow control settings */
    SOC_IF_ERROR_RETURN(READ_PT_IPTE_FC_SAFC_CFG0r(unit, &safc_cfg0));
    SOC_IF_ERROR_RETURN(READ_PT_IPTE_FC_SAFC_CFG1r(unit, &safc_cfg1));
    SOC_IF_ERROR_RETURN(READ_PT_IPTE_FC_CHAN_CFG0r(unit, &chan_cfg0));
    SOC_IF_ERROR_RETURN(READ_PT_IPTE_FC_CHAN_CFG1r(unit, &chan_cfg1));
    /*LOG_CLI((BSL_META_U(unit,
                          "safc0: 0x%8.8x, safc1: 0x%8.8x, chan0: 0x%8.8x, chan1: 0x%8.8x\n"),
               safc_cfg0, safc_cfg1, chan_cfg0, chan_cfg1));*/

    regval = 0;
    soc_reg_field_set(unit, PORT_TXFIFO_PKT_DROP_CTLr, &regval, DROP_ENf, 1);
    SOC_PBMP_ITER(*pbmp, port) {

        /* Discard packets instead of sending to MAC */
        SOC_IF_ERROR_RETURN(WRITE_PORT_TXFIFO_PKT_DROP_CTLr(unit, port, regval));

        /* Get port phy loopback state */
        status = soc_phyctrl_loopback_get(unit, port, &port_phy_state[port]);
        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d soc_phyctrl_loopback_get port %d failed: %s\n"),
                       unit, port, soc_errmsg(status)));
            return status;
        }

        /* Put port in phy loopback */
        status = soc_phyctrl_loopback_set(unit, port, TRUE, TRUE);
        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d soc_phyctrl_loopback_set port %d failed: %s\n"),
                       unit, port, soc_errmsg(status)));
            return status;
        }

        status = soc_sbx_caladan3_port_is_channelized_subport(unit, port, 
                                                              &is_channelized, &requires_phy_setup);
        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d soc_sbx_caladan3_port_is_channelized_subport port %d failed: %s\n"),
                       unit, port, soc_errmsg(status)));
            return status;
        }
        /*LOG_CLI((BSL_META_U(unit,
                              "Port %d channelized: %s\n"), port, is_channelized ? "True" : "False"));*/

        line_port_info = &port_map->line_port_info[port];
        if (line_port_info == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d port %d not found in port map table\n"), unit, port));
            return SOC_E_INTERNAL;
        }

        /* Update port flow control port bits */
        if (is_channelized) {
            if (port < 32) {
                chan_bitmap0 &= ~(1 << port);
            } else {
                chan_bitmap1 &= ~(1 << (port-32));
            }
        } else {
            if (port < 32) {
                port_bitmap0 &= ~(1 << port);
            } else {
                /* coverity[large_shift] */
                port_bitmap1 &= ~(1 << (port-32));
            }
        }

        /* Get the destination queue(s) for the port */
        /* The dq variables here are indices into the dq_list array so they
         * are in the range 0...63 however the debug prints show the
         * dq variables in the form expected by the user.
         */
        qinfo = &line_port_info->port_queues;
        SOC_SBX_C3_BMP_ITER(qinfo->dqueue_bmp, index, SOC_SBX_CALADAN3_NUM_WORDS_FOR_DQUEUES) {
            if (dq_list[index] == 0) {
                dq_list[index] = 1;
                n_dq++;
            }
        }
    
    }

    /*LOG_CLI((BSL_META_U(unit,
                          "calc, port: 0x%8.8x:%8.8x, chan: 0x%8.8x:%8.8x\n"),
               port_bitmap0, port_bitmap1, chan_bitmap0, chan_bitmap1));*/
    port_bitmap0 &= safc_cfg0;
    port_bitmap1 &= safc_cfg1;
    chan_bitmap0 &= chan_cfg0;
    chan_bitmap1 &= chan_cfg1;
    /*LOG_CLI((BSL_META_U(unit,
                          "writing port: 0x%8.8x:%8.8x, chan: 0x%8.8x:%8.8x\n"),
               port_bitmap0, port_bitmap1, chan_bitmap0, chan_bitmap1));*/
    SOC_IF_ERROR_RETURN(WRITE_PT_IPTE_FC_SAFC_CFG0r(unit, port_bitmap0));
    SOC_IF_ERROR_RETURN(WRITE_PT_IPTE_FC_SAFC_CFG1r(unit, port_bitmap1));
    SOC_IF_ERROR_RETURN(WRITE_PT_IPTE_FC_CHAN_CFG0r(unit, chan_bitmap0));
    SOC_IF_ERROR_RETURN(WRITE_PT_IPTE_FC_CHAN_CFG1r(unit, chan_bitmap1));

    /* Check for queues being empty */
    while (max_checks > 0 && n_dq > 0) {

        /* Iterate over the dqueues which start at 64 */
        for(index = 0; index < 64; index++) {

            /* If queue is not verified as empty check it */
            if (dq_list[index] != 0) {

                /* The memory block indices are in the range 0...127 with
                 * half for the egress line ports, so add 64 here.
                 */
                SOC_IF_ERROR_RETURN(soc_mem_read(unit, QM_DEST_QUEUE_STATEm,
                    SOC_MEM_BLOCK_MIN(unit,QM_DEST_QUEUE_STATEm), index+64, entry));
                pages_stored = soc_mem_field32_get(unit, QM_DEST_QUEUE_STATEm,
                    entry, PAGES_STOREDf);
                /*LOG_CLI((BSL_META_U(unit,
                                      "dq %d pages stored %u\n"), index+64, pages_stored));*/
                if (pages_stored == 0) {
                    /* This destination queue is empty */
                    dq_list[index] = 0;
                    n_dq--;
                }
            }

        }

        max_checks--;
    }

    /* Restore port state */
    SOC_IF_ERROR_RETURN(WRITE_PT_IPTE_FC_SAFC_CFG0r(unit, safc_cfg0));
    SOC_IF_ERROR_RETURN(WRITE_PT_IPTE_FC_SAFC_CFG1r(unit, safc_cfg1));
    SOC_IF_ERROR_RETURN(WRITE_PT_IPTE_FC_CHAN_CFG0r(unit, chan_cfg0));
    SOC_IF_ERROR_RETURN(WRITE_PT_IPTE_FC_CHAN_CFG1r(unit, chan_cfg1));
    regval = 0;
    SOC_PBMP_ITER(*pbmp, port) {

        /* Restore phy loopback state */
        status = soc_phyctrl_loopback_set(unit, port, port_phy_state[port], TRUE);
        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d soc_phyctrl_loopback_set port %d failed: %s\n"),
                       unit, port, soc_errmsg(status)));
            return status;
        }

        /* Forward packets to MAC */
        SOC_IF_ERROR_RETURN(WRITE_PORT_TXFIFO_PKT_DROP_CTLr(unit, port, regval));

    }

    return SOC_E_NONE;

}

/*
 *
 * Function:
 *     soc_sbx_caladan3_cmic_port_get
 * Purpose:
 *     return cmic-m ucode port number
 */
int soc_sbx_caladan3_cmic_port_get(int unit, int *port)
{
    soc_sbx_caladan3_port_map_t *port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;

    if (!port || !port_map) return SOC_E_PARAM;
    
    *port = port_map->first_reserved_port;
    
    return SOC_E_NONE;
}

int soc_sbx_caladan3_get_intftype(int unit, int port, int *intftype)
{
    soc_sbx_caladan3_port_map_t         *port_map = SOC_SBX_CFG_CALADAN3(unit)->port_map;
    soc_sbx_caladan3_port_map_info_t    *line_port_info;

    if (!port_map) {
        return SOC_E_INTERNAL;
    }
    if (port >= SOC_SBX_CALADAN3_PORT_MAP_ENTRIES) {
        return SOC_E_PARAM;
    }

    line_port_info = &port_map->line_port_info[port];
    if (!(line_port_info->flags & SOC_SBX_CALADAN3_PORT_MAP_ENTRY_VALID)) {
        *intftype = -2;
        return SOC_E_NONE;
    }

    *intftype = line_port_info->intftype;

    return SOC_E_NONE;

}

#endif /* BCM_CALADAN3_SUPPORT */


