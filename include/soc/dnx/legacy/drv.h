/*
 * $Id: drv.h,v 1.187 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains structure and routine declarations for the
 * Switch-on-a-Chip Driver.
 *
 * This file also includes the more common include files so the
 * individual driver files don't have to include as much.
 */
#ifndef _SOC_DNX_LEGACY_DRV_H
#define _SOC_DNX_LEGACY_DRV_H

#include <sal/types.h>

#include <shared/cyclic_buffer.h>

#include <soc/drv.h>
#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif
#include <soc/scache.h>
#include <soc/mem.h>

#include <soc/dnxc/legacy/dnxc_defs.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>

#include <soc/dnx/legacy/fabric.h>
#include <soc/dnx/legacy/dnx_config_imp_defs.h>
#include <soc/dnx/legacy/ARAD/arad_drv.h>
#include <soc/dnx/legacy/JER/jer_drv.h>

#include <soc/dnx/legacy/TMC/tmc_api_multicast_egress.h>

#include <soc/dnx/legacy/ARAD/arad_api_mgmt.h>
#include <soc/dnx/legacy/TMC/tmc_api_multicast_fabric.h>
#include <soc/dnxc/legacy/error.h>

#define SOC_DNX_MAX_NOF_CHANNELS    (256)
#define SOC_DNX_DRV_TDM_OPT_SIZE    (78)

/* Reset flags */
#define SOC_DNX_RESET_ACTION_IN_RESET                               SOC_DNXC_RESET_ACTION_IN_RESET
#define SOC_DNX_RESET_ACTION_OUT_RESET                              SOC_DNXC_RESET_ACTION_OUT_RESET
#define SOC_DNX_RESET_ACTION_INOUT_RESET                            SOC_DNXC_RESET_ACTION_INOUT_RESET

#define SOC_DNX_RESET_MODE_HARD_RESET                               SOC_DNXC_RESET_MODE_HARD_RESET
#define SOC_DNX_RESET_MODE_BLOCKS_RESET                             SOC_DNXC_RESET_MODE_BLOCKS_RESET
#define SOC_DNX_RESET_MODE_BLOCKS_SOFT_RESET                        SOC_DNXC_RESET_MODE_BLOCKS_SOFT_RESET
#define SOC_DNX_RESET_MODE_BLOCKS_SOFT_INGRESS_RESET                SOC_DNXC_RESET_MODE_BLOCKS_SOFT_INGRESS_RESET
#define SOC_DNX_RESET_MODE_BLOCKS_SOFT_EGRESS_RESET                 SOC_DNXC_RESET_MODE_BLOCKS_SOFT_EGRESS_RESET
#define SOC_DNX_RESET_MODE_INIT_RESET                               SOC_DNXC_RESET_MODE_INIT_RESET
#define SOC_DNX_RESET_MODE_REG_ACCESS                               SOC_DNXC_RESET_MODE_REG_ACCESS
#define SOC_DNX_RESET_MODE_ENABLE_TRAFFIC                           SOC_DNXC_RESET_MODE_ENABLE_TRAFFIC
#define SOC_DNX_RESET_MODE_BLOCKS_AND_FABRIC_SOFT_RESET             SOC_DNXC_RESET_MODE_BLOCKS_AND_FABRIC_SOFT_RESET
#define SOC_DNX_RESET_MODE_BLOCKS_AND_FABRIC_SOFT_INGRESS_RESET     SOC_DNXC_RESET_MODE_BLOCKS_AND_FABRIC_SOFT_INGRESS_RESET
#define SOC_DNX_RESET_MODE_BLOCKS_AND_FABRIC_SOFT_EGRESS_RESET      SOC_DNXC_RESET_MODE_BLOCKS_AND_FABRIC_SOFT_EGRESS_RESET
#define SOC_DNX_RESET_MODE_BLOCKS_SOFT_RESET_DIRECT                 SOC_DNXC_RESET_MODE_BLOCKS_SOFT_RESET_DIRECT


#define SOC_DNX_MAX_PADDING_SIZE 0x7f

/* 0-511 pbmp 512-1023 internal 1024-1535 interface */ 

/* Different port types have different ranges                   */
/* each port range is sized at SOC_DNX_PORT_RANGE_NUM_ENTRIES   */
/* pbmp ports start at 0.                                       */
#define SOC_DNX_PORT_RANGE_NUM_ENTRIES 512 /* 288 required for JER2_ARAD */

/* Internal ports */
#ifdef  BCM_88680_A0
#define SOC_DNX_PORT_INTERNAL_START           560
#define SOC_DNX_PORT_INTERNAL_OLP(core)             (SOC_DNX_PORT_INTERNAL_START + core)
#define SOC_DNX_PORT_INTERNAL_OAMP(core)            (SOC_DNX_PORT_INTERNAL_START + 1*SOC_DNX_DEFS_MAX(NOF_CORES) + core)
#define SOC_DNX_PORT_INTERNAL_ERP(core)             (SOC_DNX_PORT_INTERNAL_START + 2*SOC_DNX_DEFS_MAX(NOF_CORES) + core)
#elif defined BCM_88675_A0
#define SOC_DNX_PORT_INTERNAL_START           548
#define SOC_DNX_PORT_INTERNAL_OLP(core)             (SOC_DNX_PORT_INTERNAL_START + core)
#define SOC_DNX_PORT_INTERNAL_OAMP(core)            (SOC_DNX_PORT_INTERNAL_START + 1*SOC_DNX_DEFS_MAX(NOF_CORES) + core)
#define SOC_DNX_PORT_INTERNAL_ERP(core)             (SOC_DNX_PORT_INTERNAL_START + 2*SOC_DNX_DEFS_MAX(NOF_CORES) + core)
#elif defined BCM_88375_A0
#define SOC_DNX_PORT_INTERNAL_START           548
#define SOC_DNX_PORT_INTERNAL_OLP(core)             (SOC_DNX_PORT_INTERNAL_START + core)
#define SOC_DNX_PORT_INTERNAL_OAMP(core)            (SOC_DNX_PORT_INTERNAL_START + 1*SOC_DNX_DEFS_MAX(NOF_CORES) + core)
#define SOC_DNX_PORT_INTERNAL_ERP(core)             (SOC_DNX_PORT_INTERNAL_START + 2*SOC_DNX_DEFS_MAX(NOF_CORES) + core)
#else
#define SOC_DNX_PORT_INTERNAL_START           324
#define SOC_DNX_PORT_INTERNAL_OLP(core)       (SOC_DNX_PORT_INTERNAL_START + 0)
#define SOC_DNX_PORT_INTERNAL_OAMP(core)      (SOC_DNX_PORT_INTERNAL_START + 1)
#define SOC_DNX_PORT_INTERNAL_ERP(core)       (SOC_DNX_PORT_INTERNAL_START + 2)
#endif

#define SOC_DNX_PORT_INTERNAL_END                   (SOC_DNX_PORT_INTERNAL_START + SOC_DNX_PORT_RANGE_NUM_ENTRIES - 1)

/* Interface ports */
#define SOC_DNX_PORT_INTERFACE_START 1024
#define SOC_DNX_PORT_INTERFACE_END   (SOC_DNX_PORT_INTERFACE_START + (SOC_DNX_PORT_RANGE_NUM_ENTRIES) - 1)

/* Number of MACT limit table ranges to be mapped */
#define SOC_DNX_MAX_NOF_MACT_LIMIT_MAPPED_LIF_RANGES    (2)

#define SOC_DNX_FABRIC_LOGICAL_PORT_BASE_DEFAULT  (256)
#define FABRIC_LOGICAL_PORT_BASE(unit)            (SOC_INFO(unit).fabric_logical_port_base)

#define SOC_DNX_NUM_OF_ROUTES (2048)
#define SOC_DNX_MAX_NUM_OF_ROUTE_GROUPS           (32)

#define _DNX_MAX_NUM_OF_LIFS           (SOC_DNX_DEFS_MAX(NOF_LOCAL_LIFS) - 2) /* highest lif (16*1024-1) is used only for MiM */

#define SOC_DNX_NOF_DIRECT_PORTS  (32)

#define SOC_DNX_FIRST_DIRECT_PORT(unit)   (FABRIC_LOGICAL_PORT_BASE(unit) + SOC_DNX_DEFS_GET(unit, nof_fabric_links))

#define SOC_DNX_FABRIC_LINKS_TO_CORE_MAP_SHARED     (0)
#define SOC_DNX_FABRIC_LINKS_TO_CORE_MAP_DEDICATED  (1)

#define SOC_DNX_INVALID_NOF_REMOTE_CORES            (0xffff)
#define SOC_DNX_INVALID_MAPPING_MODE                (0xffff)

#define SOC_DNX_MAX_RIF_ID                          (SOC_DNX_DEFS_GET(unit, max_nof_rifs) - 1)

#define SOC_DNX_PHY_GPORT_LANE_VALID(unit, gport)                                                                                                               \
                                        (                                                                                                                       \
                                            BCM_PHY_GPORT_IS_LANE(gport)                                                                                    &&  \
                                            SOC_PORT_VALID(unit, BCM_PHY_GPORT_LANE_PORT_PORT_GET(port))                                                    &&  \
                                            SOC_INFO(unit).port_num_lanes!=NULL                                                                            &&  \
                                            SOC_INFO(unit).port_num_lanes[BCM_PHY_GPORT_LANE_PORT_PORT_GET(port)]  > BCM_PHY_GPORT_LANE_PORT_LANE_GET(port)    \
                                        )

#define SOC_DNX_FABRIC_PORT_TO_LINK(unit, port) ((port) - FABRIC_LOGICAL_PORT_BASE(unit))
#define SOC_DNX_FABRIC_LINK_TO_PORT(unit, port) ((port) + FABRIC_LOGICAL_PORT_BASE(unit))

/*
 * Definitions of various SOC properties
 */
#define SOC_DNX_FC_CAL_MODE_DISABLE                             0
#define SOC_DNX_FC_CAL_MODE_RX_ENABLE                           0x1
#define SOC_DNX_FC_CAL_MODE_TX_ENABLE                           0x2

#define SOC_DNX_FC_INBAND_INTLKN_CAL_LLFC_MODE_DISABLE          0
#define SOC_DNX_FC_INBAND_INTLKN_CAL_LLFC_MODE1                 1 /* calender 0 */
#define SOC_DNX_FC_INBAND_INTLKN_CAL_LLFC_MODE2                 2 /* calender 0, 16, ... */

#define SOC_DNX_FC_INBAND_INTLKN_LLFC_MUB_DISABLE               0
#define SOC_DNX_FC_INBAND_INTLKN_LLFC_MUB_MASK                  0xFF

#define SOC_DNX_FC_INBAND_INTLKN_CHANNEL_MUB_DISABLE            0
#define SOC_DNX_FC_INBAND_INTLKN_CHANNEL_MUB_MASK               0xFF

#define SOC_DNX_FABRIC_TDM_PRIORITY_NONE                        7 /* system does not support TDM */

/*
 * dnx tm config declaration
 */

#define SOC_DNX_MAX_INTERLAKEN_PORTS                            SOC_DNX_DEFS_MAX(NOF_INTERLAKEN_PORTS)
#define SOC_DNX_MAX_CAUI_PORTS                                  SOC_DNX_DEFS_MAX(NOF_CAUI_PORTS)
#define SOC_DNX_MAX_OOB_PORTS                                   2
#define SOC_DNX_NOF_LINKS                                      36 /* for JER2_ARAD only */
#define SOC_DNX_NOF_INSTANCES_MAC                               9 /* for JER2_ARAD only */

#define JER2_ARAD_MULTICAST_TABLE_MODE         JER2_ARAD_MULT_NOF_MULTICAST_GROUPS
#define JER2_QAX_NOF_MCDB_ENTRIES              (SOC_IS_QUX(unit) ? (48*1024) : (96*1024))
#define JER2_JER_NOF_MCDB_ENTRIES              (256*1024)

/* DNX MC Modes */
#define DNX_MC_ALLOW_DUPLICATES_MODE            1  /* allow duplicate replications in multicast groups */
#define DNX_MC_EGR_17B_CUDS_127_PORTS_MODE      4  /* Arad+ encoding mode */
#define DNX_MC_CUD_EXTENSION_MODE               8  /* CUD extension node */
#define DNX_MC_EGR_CORE_FDA_MODE                16 /* FDA egress hardware controls whether each egress group core is replicated to */
#define DNX_MC_EGR_CORE_MESH_MODE               64 /* Mesh ingress MC hardware controls whether each egress group core is replicated to */

#define JER2_ARAD_Q_PAIRS_ILKN                 8


/* Macros for multicast CUD (outlif) and destination encoding */

#define JER2_ARAD_MC_DEST_ENCODING_0 0 /* supported on all Arads, 16bit CUDs, 96K flows, 32K sysports, 32K MCIDs */
#define JER2_ARAD_MC_DEST_ENCODING_1 1 /* supported on Arad+, 17bit CUDs, 64K-1 flows, 32K sysports */
#define JER2_ARAD_MC_DEST_ENCODING_2 2 /* supported on Arad+, 18bit CUDs, 32K-1 flows, 16K sysports */
#define JER2_ARAD_MC_DEST_ENCODING_3 3 /* supported on Arad+, 17bit CUDs, 96K flows */

/* maximum number of queue that the encoding supports (also limited by no more than 96K queues) */
#define JER2_ARAD_MC_DEST_ENCODING_0_MAX_QUEUE 0x17fff
#define JER2_ARAD_MC_DEST_ENCODING_1_MAX_QUEUE 0xfffe /* 0xffff may cause no replication */
#define JER2_ARAD_MC_DEST_ENCODING_2_MAX_QUEUE 0x7ffe /* 0x7fff may cause no replication */
#define JER2_ARAD_MC_DEST_ENCODING_3_MAX_QUEUE 0x17fff

/* maximum CUD value that the ingress encoding supports (also limited by no more than 96K queues) */
#define JER2_ARAD_MC_DEST_ENCODING_0_MAX_ING_CUD  0xffff
#define JER2_ARAD_MC_DEST_ENCODING_1_MAX_ING_CUD 0x1ffff
#define JER2_ARAD_MC_DEST_ENCODING_2_MAX_ING_CUD 0x3ffff
#define JER2_ARAD_MC_DEST_ENCODING_3_MAX_ING_CUD 0x1ffff
/* maximum CUD value that the egress encoding supports (also limited by no more than 96K queues) */
#define JER2_ARAD_MC_16B_MAX_EGR_CUD  0xffff
#define JER2_ARAD_MC_17B_MAX_EGR_CUD 0x1ffff

#define JER2_JER_MC_MAX_EGR_CUD  0x3ffff
#define JER2_JER_MC_MAX_ING_CUD  0x7ffff

/* number of system ports that the encoding supports */
#define JER2_ARAD_MC_DEST_ENCODING_0_NOF_SYSPORT 0x8000
#define JER2_ARAD_MC_DEST_ENCODING_1_NOF_SYSPORT 0x8000
#define JER2_ARAD_MC_DEST_ENCODING_2_NOF_SYSPORT 0x4000
#define JER2_ARAD_MC_DEST_ENCODING_3_NOF_SYSPORT 0

#define JER2_ARAD_MC_MAX_BITMAPS 8191

/* base queue to module x port mapping modes: */
#define VOQ_MAPPING_DIRECT   0
#define VOQ_MAPPING_INDIRECT 1

/* Reserve Unique Device ID for TDM settings. All FAP-IDs must be different than this ID */
#define DNX_TDM_FAP_DEVICE_ID_UNIQUE      (0x80)

/* supported values for the action_type_source_mode field */
#define DNX_ACTION_TYPE_FROM_QUEUE_SIGNATURE 0   /* get action type from queue signature (default) */
#define DNX_ACTION_TYPE_FROM_FORWARDING_ACTION 1 /* get action type from packet header */

#define JER2_ARAD_IS_VOQ_MAPPING_DIRECT(unit) (SOC_DNX_CONFIG(unit)->jer2_arad->voq_mapping_mode == VOQ_MAPPING_DIRECT)
#define JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit) (SOC_DNX_CONFIG(unit)->jer2_arad->voq_mapping_mode != VOQ_MAPPING_DIRECT)
#define JER2_ARAD_IS_HQOS_MAPPING_ENABLE(unit) (SOC_DNX_CONFIG(unit)->jer2_arad->hqos_mapping_enable == TRUE)

#define DNX_IS_DNX_ACTION_TYPE_FROM_QUEUE_SIGNATURE(unit) (SOC_DNX_CONFIG(unit)->jer2_arad->action_type_source_mode == DNX_ACTION_TYPE_FROM_QUEUE_SIGNATURE)

/* VLAN Translation */
#define SOC_DNX_VLAN_TRANSLATE_NORMAL_MODE_NORMAL   (0)
#define SOC_DNX_VLAN_TRANSLATE_NORMAL_MODE_ADVANCED (1)

#define SOC_DNX_IS_VLAN_TRANSLATE_MODE_NORMAL(unit)    (SOC_DNX_CONFIG(unit)->pp.vlan_translate_mode == SOC_DNX_VLAN_TRANSLATE_NORMAL_MODE_NORMAL)
#define SOC_DNX_IS_VLAN_TRANSLATE_MODE_ADVANCED(unit)  (SOC_DNX_CONFIG(unit)->pp.vlan_translate_mode == SOC_DNX_VLAN_TRANSLATE_NORMAL_MODE_ADVANCED)

#define SOC_DNX_NOF_INGRESS_VLAN_EDIT_ACTION_IDS(unit)             (SOC_DNX_CONFIG(unit)->pp.nof_ive_action_ids)
#define SOC_DNX_NOF_INGRESS_VLAN_EDIT_RESERVED_ACTION_IDS(unit)    (SOC_DNX_CONFIG(unit)->pp.nof_ive_reserved_action_ids)

#define SOC_DNX_NOF_EGRESS_VLAN_EDIT_ACTION_MAPPINGS(unit)         (SOC_DNX_CONFIG(unit)->pp.nof_eve_action_mappings)
#define SOC_DNX_NOF_EGRESS_VLAN_EDIT_ACTION_IDS(unit)              (SOC_DNX_CONFIG(unit)->pp.nof_eve_action_ids)
#define SOC_DNX_NOF_EGRESS_VLAN_EDIT_RESERVED_ACTION_IDS(unit)     (SOC_DNX_CONFIG(unit)->pp.nof_eve_reserved_action_ids)

/* Protection */
#define SOC_DNX_IS_PROTECTION_INGRESS_COUPLED(unit)                 (SOC_DNX_JER2_JER_CONFIG(unit)->pp.protection_ingress_coupled_mode)
#define SOC_DNX_IS_PROTECTION_EGRESS_COUPLED(unit)                  (SOC_DNX_JER2_JER_CONFIG(unit)->pp.protection_egress_coupled_mode)
#define SOC_DNX_IS_PROTECTION_FEC_ACCELERATED_REROUTE_MODE(unit)    (SOC_DNX_JER2_JER_CONFIG(unit)->pp.protection_fec_accelerated_reroute_mode)

#define SOC_DNX_IS_SYS_RSRC_MGMT_ADVANCED(unit)  (SOC_DNX_CONFIG(unit)->pp.sys_rsrc_mgmt == 1)


#define DNX_IS_CREDIT_WATCHDOG_FAST_STATUS_MESSAGE_MODE(unit, mode) (mode == CREDIT_WATCHDOG_FAST_STATUS_MESSAGE_MODE)
#define DNX_IS_CREDIT_WATCHDOG_COMMON_STATUS_MESSAGE_MODE(unit, mode) (mode >= CREDIT_WATCHDOG_COMMON_STATUS_MESSAGE_MODE)
#define DNX_IS_CREDIT_WATCHDOG_UNINITIALIZED(unit, mode) (mode == CREDIT_WATCHDOG_UNINITIALIZED)
#define DNX_GET_CREDIT_WATCHDOG_MODE_BASE(mode) ((mode) < CREDIT_WATCHDOG_COMMON_STATUS_MESSAGE_MODE ? (mode) : CREDIT_WATCHDOG_COMMON_STATUS_MESSAGE_MODE)
#define DNX_GET_CREDIT_WATCHDOG_MODE(unit, mode) (sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.credit_watchdog.credit_watchdog_mode.get(unit, &(mode)))
#define DNX_IS_CREDIT_WATCHDOG_MODE(unit, mode, credit_watchdog_mode) (DNX_GET_CREDIT_WATCHDOG_MODE_BASE(mode) == DNX_GET_CREDIT_WATCHDOG_MODE_BASE(credit_watchdog_mode))
#define DNX_SET_CREDIT_WATCHDOG_MODE(unit, mode) (sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.credit_watchdog.credit_watchdog_mode.set(unit, (mode)))
#define DNX_GET_EXACT_CREDIT_WATCHDOG_SCAN_TIME_NANO(unit, scan_time_nano) \
    (sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.credit_watchdog.exact_credit_watchdog_scan_time_nano.get(unit, &(scan_time_nano)))
#define DNX_SET_EXACT_CREDIT_WATCHDOG_SCAN_TIME_NANO(unit, scan_time_nano) \
    (sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.credit_watchdog.exact_credit_watchdog_scan_time_nano.set(unit, (scan_time_nano)))


#define JER2_ARAD_CREDIT_WATCHDOG_COMMON_MAX_SCAN_TIME_MS 2
#define JER2_ARAD_CREDIT_WATCHDOG_COMMON_MAX_SCAN_TIME_NS (1000000 * JER2_ARAD_CREDIT_WATCHDOG_COMMON_MAX_SCAN_TIME_MS)
#define JER2_ARAD_CREDIT_WATCHDOG_COMMON_MAX_DELETE_EXP 12
#define JER2_ARAD_CREDIT_WATCHDOG_COMMON_SCAN_TIME_MAX_HALVES 4
#define JER2_ARAD_CREDIT_WATCHDOG_COMMON_MIN_SCAN_TIME_NS (JER2_ARAD_CREDIT_WATCHDOG_COMMON_MAX_SCAN_TIME_NS >> JER2_ARAD_CREDIT_WATCHDOG_COMMON_SCAN_TIME_MAX_HALVES)

/* Using this number for the start and end of the queue scan range in jer2_arad_itm_cr_wd_set/get,
 * marks to update the delete time exponents for all profiles, in the common FSM mode */
#define JER2_ARAD_CREDIT_WATCHDOG_Q_COMMON_FSM_ADJUST_EXP ((uint32)(-2))

#define DNX_VARIOUS_BM_FORCE_MBIST_TEST 1

/* VLAN match DB mode */
#define SOC_DNX_VLAN_DB_MODE_DEFAULT         (0)
#define SOC_DNX_VLAN_DB_MODE_FULL_DB         (1)
#define SOC_DNX_VLAN_DB_MODE_PCP             (3)

/* IP Tunnel termination bitmap */
/* IP tunnel */
#define SOC_DNX_IP_TUNNEL_TERM_DB_DEFAULT     (1) /* IP tunnel without L2 above DIP only */
#define SOC_DNX_IP_TUNNEL_TERM_DB_DIP_SIP     (2) /* IP tunnel without L2 above (DIP,SIP) */
#define SOC_DNX_IP_TUNNEL_TERM_DB_DIP_SIP_NEXT_PROTOCOL (32)    /* IP tunnel with {dip, sip, port property, next protocol} as key. */
#define SOC_DNX_IP_TUNNEL_TERM_DB_DIP_SIP_VRF (64) /* IP tunnel with {dip, sip, vrf, IP.protocol, GRE.ethertype, is-tunnel-gre, port_property} as key. */
#define SOC_DNX_IP_TUNNEL_TERM_DB_TUNNEL_MASK (SOC_DNX_IP_TUNNEL_TERM_DB_DEFAULT | SOC_DNX_IP_TUNNEL_TERM_DB_DIP_SIP | SOC_DNX_IP_TUNNEL_TERM_DB_DIP_SIP_NEXT_PROTOCOL | SOC_DNX_IP_TUNNEL_TERM_DB_DIP_SIP_VRF) /* tunnel terminations */

/* L2oIP enable*/
#define SOC_DNX_IP_TUNNEL_TERM_DB_NVGRE       (4)
#define SOC_DNX_IP_TUNNEL_TERM_DB_VXLAN       (8)
#define SOC_DNX_IP_TUNNEL_TERM_DB_ETHER       (16) /* EoIP */

/* L2oIP lookup mode */
#define SOC_DNX_IP_TUNNEL_L2_LKUP_MODE_DISABLE           (0)
#define SOC_DNX_IP_TUNNEL_L2_LKUP_MODE_SIP_DIP_SEPERATED (1)
#define SOC_DNX_IP_TUNNEL_L2_LKUP_MODE_SIP_DIP_JOIN      (2)
#define SOC_DNX_IP_TUNNEL_L2_LKUP_MODE_DIP_SIP_VRF_SEM_ONLY_AND_DEFAULT_TCAM (3) /* IP tunnel with {DIP} that returns my-vtep-index. and {my-vtep-index, sip, vrf} lookup. in ISEM only. 
                                                                                      In this mode, the user can also configure a default IP tunnel termination in case there was no hit: {DIP, SIP, VRF} in TCAM */



#define SOC_DNX_GUARANTEED_Q_RESOURCE_MEMORY  0 /* memory amount will be guaranteed. This leads to waste of resources due to buffer fragmentation */
#define SOC_DNX_GUARANTEED_Q_RESOURCE_BUFFERS 1 /* number of buffers (DBs) will be guaranteed. */
#define SOC_DNX_GUARANTEED_Q_RESOURCE_BDS     2 /* number of buffer descriptors (BDs) will be guaranteed. This is the recommended mode */

#define JER2_ARAD_PLUS_MIN_ALPHA -7
#define JER2_ARAD_PLUS_MAX_ALPHA 7
#define JER2_ARAD_PLUS_ALPHA_NOT_SUPPORTED 255

#define JER2_ARAD_PLUS_CREDIT_VALUE_MODES_PER_WORD (sizeof(uint32) * 8)
#define JER2_ARAD_PLUS_CREDIT_VALUE_MODE_WORDS (JER2_ARAD_NOF_FAPS_IN_SYSTEM / JER2_ARAD_PLUS_CREDIT_VALUE_MODES_PER_WORD)

/* L3 source bind mode */
#define SOC_DNX_L3_SOURCE_BIND_MODE_DISABLE  (0)
#define SOC_DNX_L3_SOURCE_BIND_MODE_IPV4     (1)
#define SOC_DNX_L3_SOURCE_BIND_MODE_IPV6     (2)
#define SOC_DNX_L3_SOURCE_BIND_MODE_IP       (3) /* Both IPv4 and IPv6 */
#define SOC_DNX_L3_SOURCE_BIND_SUBNET_MODE_DISABLE  (0)
#define SOC_DNX_L3_SOURCE_BIND_SUBNET_MODE_IPV4     (1)
#define SOC_DNX_L3_SOURCE_BIND_SUBNET_MODE_IPV6     (2)
#define SOC_DNX_L3_SOURCE_BIND_SUBNET_MODE_IP       (3) /* Both IPv4 and IPv6 subnet */
#define SOC_DNX_L3_SRC_BIND_FOR_ARP_RELAY_DOWN      (0x1)
#define SOC_DNX_L3_SRC_BIND_FOR_ARP_RELAY_UP        (0x2)
#define SOC_DNX_L3_SRC_BIND_FOR_ARP_RELAY_BOTH      (0x3)
#define SOC_DNX_L3_SRC_BIND_ARP_RELAY_ENABLE(unit) (SOC_DNX_CONFIG(unit)->pp.l3_src_bind_arp_relay)
#define SOC_DNX_L3_SRC_BIND_IPV4_SUBNET_ENABLE(unit)\
           ((SOC_DNX_CONFIG(unit)->pp.l3_source_bind_subnet_mode == SOC_DNX_L3_SOURCE_BIND_SUBNET_MODE_IPV4) || \
            (SOC_DNX_CONFIG(unit)->pp.l3_source_bind_subnet_mode == SOC_DNX_L3_SOURCE_BIND_SUBNET_MODE_IP))      
#define SOC_DNX_L3_SRC_BIND_IPV6_SUBNET_ENABLE(unit)\
           ((SOC_DNX_CONFIG(unit)->pp.l3_source_bind_subnet_mode == SOC_DNX_L3_SOURCE_BIND_SUBNET_MODE_IPV6) || \
            (SOC_DNX_CONFIG(unit)->pp.l3_source_bind_subnet_mode == SOC_DNX_L3_SOURCE_BIND_SUBNET_MODE_IP)) 
#define SOC_DNX_L3_SRC_BIND_IPV4_HOST_ENABLE(unit)\
           ((SOC_DNX_CONFIG(unit)->pp.l3_source_bind_mode == SOC_DNX_L3_SOURCE_BIND_MODE_IPV4) || \
            (SOC_DNX_CONFIG(unit)->pp.l3_source_bind_mode == SOC_DNX_L3_SOURCE_BIND_MODE_IP))      
#define SOC_DNX_L3_SRC_BIND_IPV6_HOST_ENABLE(unit)\
           ((SOC_DNX_CONFIG(unit)->pp.l3_source_bind_mode == SOC_DNX_L3_SOURCE_BIND_MODE_IPV6) || \
            (SOC_DNX_CONFIG(unit)->pp.l3_source_bind_mode == SOC_DNX_L3_SOURCE_BIND_MODE_IP)) 
#define SOC_DNX_L3_SRC_BIND_IPV4_ENABLE(unit) \
           (SOC_DNX_L3_SRC_BIND_IPV4_SUBNET_ENABLE(unit) || SOC_DNX_L3_SRC_BIND_IPV4_HOST_ENABLE(unit))
#define SOC_DNX_L3_SRC_BIND_IPV6_ENABLE(unit) \
           (SOC_DNX_L3_SRC_BIND_IPV6_SUBNET_ENABLE(unit) || SOC_DNX_L3_SRC_BIND_IPV6_HOST_ENABLE(unit))
#define SOC_DNX_L3_SRC_BIND_IPV4_SUBNET_OR_ARP_ENABLE(unit) \
           (SOC_DNX_L3_SRC_BIND_IPV4_SUBNET_ENABLE(unit) || SOC_DNX_L3_SRC_BIND_ARP_RELAY_ENABLE(unit))
#define SOC_DNX_L3_SOURCE_BIND_DISABLE(unit) \
           ((SOC_DNX_CONFIG(unit)->pp.l3_source_bind_mode == SOC_DNX_L3_SOURCE_BIND_MODE_DISABLE) && \
             (SOC_DNX_CONFIG(unit)->pp.l3_source_bind_subnet_mode == SOC_DNX_L3_SOURCE_BIND_SUBNET_MODE_DISABLE) && \
             !SOC_DNX_L3_SRC_BIND_ARP_RELAY_ENABLE(unit))
#define SOC_DNX_L3_SOURCE_BIND_USE_FEC_LEARN_DATA(unit) \
            soc_property_suffix_num_get((unit), -1, spn_CUSTOM_FEATURE, "l3_source_bind_use_fec_learn_data", 0)
            
/* XGS Diffserv, HQOS system port encoding mode */
#define SOC_DNX_XGS_TM_7_MODID_8_PORT        (0)
#define SOC_DNX_XGS_TM_8_MODID_7_PORT        (1)
#define SOC_DNX_WARMBOOT_TIME_OUT            (5000000) /*5 sec*/

#define SOC_DNX_JER2_ARAD_INJECTED_HEADER_SIZE_BYTES_PTCH    (3) /* PTCH-1: 3 bytes */
#define SOC_DNX_JER2_ARAD_INJECTED_HEADER_SIZE_BYTES_PTCH_2  (2) /* PTCH-2: 2 bytes */
#define SOC_DNX_JER2_ARAD_INJECTED_HEADER_SIZE_BYTES_FRC_PPD (16) 
#define SOC_DNX_JER2_ARAD_INJECTED_HEADER_SIZE_BYTES_HIGIG_FB (1) /* NIF-Higig 0xFB prefix: 1 byte */
#define SOC_DNX_JER2_ARAD_INJECTED_HEADER_SIZE_BYTES_VENDOR_PP (12)
#define SOC_DNX_JER2_ARAD_INJECTED_HEADER_SIZE_BYTES_VENDOR_PP_2 (16)
#define SOC_DNX_JER2_ARAD_INJECTED_HEADER_SIZE_BYTES_COE       (4)
#define SOC_DNX_JER2_ARAD_INJECTED_HEADER_SIZE_BYTES_PON       (4)

/* To be used as the value of SOC_DNX_CONFIG(unit)->pp.micro_bfd_support*/
#define SOC_DNX_JER2_ARAD_MICRO_BFD_SUPPORT_NONE 0
#define SOC_DNX_JER2_ARAD_MICRO_BFD_SUPPORT_IPv4 1
#define SOC_DNX_JER2_ARAD_MICRO_BFD_SUPPORT_IPv6 2
#define SOC_DNX_JER2_ARAD_MICRO_BFD_SUPPORT_IPv4_6 3

/* To be used as the value of SOC_DNX_CONFIG(unit)->pp.bfd_ipv6_enable*/
#define SOC_DNX_JER2_ARAD_BFD_IPV6_SUPPORT_NONE 0
#define SOC_DNX_JER2_ARAD_BFD_IPV6_SUPPORT_WITH_LEM 1
#define SOC_DNX_JER2_ARAD_BFD_IPV6_SUPPORT_CLASSIFIER 2 /* Available only in Jericho*/

#define SOC_DNX_JER2_ARAD_IS_HG_SPEED_ONLY(speed) ((speed == 21000) || (speed == 42000)|| (speed == 106000) || (speed == 127000))

/* put these defenitions under #if 0 and defined those above because of mutex deadlock they cause */    \

#define SOC_DNX_ALLOW_WARMBOOT_WRITE(operation, _rv) \
            SOC_ALLOW_WB_WRITE(unit, operation, _rv)

#define SOC_DNX_ALLOW_WARMBOOT_WRITE_NO_ERR(operation, _rv) \
        do { \
            SOC_ALLOW_WB_WRITE(unit, operation, _rv); \
            if (_rv != SOC_E_UNIT) { \
                _rv = SOC_E_NONE; \
            } \
        } while(0)

#define SOC_DNX_WARMBOOT_RELEASE_HW_MUTEX(_rv)\
        do {\
            _rv = soc_schan_override_disable(unit); \
        } while(0)

/* iterate over all cores */
#define SOC_DNX_CORES_ITER(core_id, index) \
    for(index = ((core_id == _SHR_CORE_ALL) ? 0 : core_id);\
        index < ((core_id == _SHR_CORE_ALL) ?  SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores : (core_id + 1));\
        index++)

#define SOC_DNX_ASSYMETRIC_CORES_ITER(core_id, index)\
    for(index = ((core_id == _SHR_CORE_ALL || SOC_DNX_CORE_MODE_IS_SYMMETRIC(unit)) ? 0 : core_id);\
        index < ((core_id == _SHR_CORE_ALL) ?  \
                    (SOC_DNX_CORE_MODE_IS_SYMMETRIC(unit) ? 1 : SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) : \
                    (core_id + 1));\
        index++)

/* validate core in range */
#define SOC_DNX_CORE_VALIDATE(unit, core, allow_all) \
        if((core >= SOC_DNX_DEFS_GET(unit, nof_cores) || (core < 0)) && \
           (!allow_all || core != _SHR_CORE_ALL)) { \
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_BCM_MSG("Invalid Core %d"), core)); \
        }

/* converts modid to base modid */
#define SOC_DNX_IS_MODID_AND_BASE_MODID_ON_SAME_FAP(unit, modid, base_modid) \
           ((base_modid <= modid) && (base_modid + SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores > modid))

#define SOC_DNX_CORE_TO_MODID(base_modid, core) (base_modid + core)

#define SOC_DNX_MODID_TO_CORE(unit, base_modid, modid) \
           modid - base_modid; \
           SOC_DNX_CORE_VALIDATE(unit, modid - base_modid, FALSE) 

#define _SOC_DNX_HTONS_CVT_SET(pkt, val, posn)  \
    do { \
         uint16 _tmp; \
         _tmp = soc_htons(val); \
         sal_memcpy((pkt)->_pkt_data.data + (posn), &_tmp, 2); \
    } while (0) 

/* Helper macro for single buffer Ethernet packet with PTCH header (not HiGig). */
#define SOC_DNX_PKT_PTCH_HDR_SET(pkt, mac)  \
    sal_memcpy((pkt)->_pkt_data.data, (mac), 2) 

/* Helper macro for single buffer Ethernet packet with PTCH header (not HiGig). */
#define SOC_DNX_PKT_HDR_DMAC_SET_WITH_PTCH(pkt, mac)  \
    sal_memcpy((pkt)->_pkt_data.data + 2, (mac), 6) 

/* Helper macro for single buffer Ethernet packet with PTCH header(not HiGig). */
#define SOC_DNX_PKT_HDR_SMAC_SET_WITH_PTCH(pkt, mac)  \
    sal_memcpy((pkt)->_pkt_data.data + 8, (mac), 6) 

/* Helper macro for single buffer Ethernet packet with PTCH header(not HiGig). */
#define SOC_DNX_PKT_HDR_TPID_SET_WITH_PTCH(pkt, tpid)  \
    _SOC_DNX_HTONS_CVT_SET(pkt, tpid, 14) 

/* Helper macro for single buffer Ethernet packet with PTCH header(not HiGig). */
#define SOC_DNX_PKT_HDR_UNTAGGED_LEN_SET_WITH_PTCH(pkt, len)  \
    _SOC_DNX_HTONS_CVT_SET(pkt, len, 14) 

/* Helper macro for single buffer Ethernet packet with PTCH header(not HiGig). */
#define SOC_DNX_PKT_HDR_VTAG_CONTROL_SET_WITH_PTCH(pkt, vtag)  \
    _SOC_DNX_HTONS_CVT_SET(pkt, vtag, 16) 

/* Helper macro for single buffer Ethernet packet with PTCH header(not HiGig). */
#define SOC_DNX_PKT_HDR_TAGGED_LEN_SET_WITH_PTCH(pkt, len)  \
    _SOC_DNX_HTONS_CVT_SET(pkt, len, 18) 

#define SOC_DNX_GET_BLOCK_TYPE_SCHAN_ID(unit,block_type,other_value)   soc_dnx_get_block_type_schan_id(unit,block_type)>=0 ? soc_dnx_get_block_type_schan_id(unit,block_type): other_value


typedef enum soc_dnx_core_mode_type_e {
    soc_dnx_core_symmetric = 0x0,
    soc_dnx_core_asymmetric = 0x1
} soc_dnx_core_mode_type_t;

typedef struct soc_dnx_config_core_mode_s {
    uint8 nof_active_cores;
    soc_dnx_core_mode_type_t type;
} soc_dnx_config_core_mode_t;

/*Used to save the PDM-extension info*/
typedef struct soc_dnx_config_pdm_extension_s {
    uint32 max_pp_port;
    uint32 max_st_vsq;
    uint32 max_hdr_comp_ptr;
} soc_dnx_config_pdm_extension_t;


#define SOC_DNX_CORE_MODE_IS_SINGLE_CORE(unit) (SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores == 1)
#define SOC_DNX_CORE_MODE_IS_SYMMETRIC(unit) (SOC_DNX_CONFIG(unit)->core_mode.type == soc_dnx_core_symmetric)
#define SOC_DNX_CORE_ALL(unit) (SOC_DNX_CORE_MODE_IS_SINGLE_CORE(unit) ? 0 : SOC_CORE_ALL)
#define SOC_DNX_CORE_NOF_ACTIVE_CORES(unit)    (SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores)

typedef struct soc_dnx_tm_crps_config_s {
	uint32 sequentialTimerInterval[(SOC_DNX_DEFS_MAX(NOF_COUNTER_PROCESSORS) + SOC_DNX_DEFS_MAX(NOF_SMALL_COUNTER_PROCESSORS))];
}soc_dnx_tm_crps_config_t;

typedef struct soc_dnx_tm_config_s {
  uint32  *base_address;
  int8    max_interlaken_ports;
  int8    max_oob_ports;
  int8    fc_oob_type[SOC_DNX_MAX_OOB_PORTS];
  int8    fc_oob_mode[SOC_DNX_MAX_OOB_PORTS];
  int16   fc_oob_calender_length[SOC_DNX_MAX_OOB_PORTS][DNX_TMC_CONNECTION_DIRECTION_BOTH];
  int8    fc_oob_calender_rep_count[SOC_DNX_MAX_OOB_PORTS][DNX_TMC_CONNECTION_DIRECTION_BOTH];
  int8    fc_oob_tx_speed[SOC_DNX_MAX_OOB_PORTS];
  int8    fc_oob_ilkn_indication_invert[SOC_DNX_MAX_OOB_PORTS][DNX_TMC_CONNECTION_DIRECTION_BOTH];
  int8    fc_oob_spi_indication_invert[SOC_DNX_MAX_OOB_PORTS];
  int8    fc_inband_intlkn_type[SOC_DNX_MAX_INTERLAKEN_PORTS];
  int8    fc_inband_intlkn_mode[SOC_DNX_MAX_INTERLAKEN_PORTS];
  int16   fc_inband_intlkn_calender_length[SOC_DNX_MAX_INTERLAKEN_PORTS][DNX_TMC_CONNECTION_DIRECTION_BOTH];
  int8    fc_inband_intlkn_calender_rep_count[SOC_DNX_MAX_INTERLAKEN_PORTS][DNX_TMC_CONNECTION_DIRECTION_BOTH];
  int8    fc_inband_intlkn_calender_llfc_mode[SOC_DNX_MAX_INTERLAKEN_PORTS];
  int8    fc_inband_intlkn_llfc_mub_enable_mask[SOC_DNX_MAX_INTERLAKEN_PORTS];
  int8    fc_inband_intlkn_channel_mub_enable_mask[SOC_DNX_MAX_INTERLAKEN_PORTS][DNX_TMC_CONNECTION_DIRECTION_BOTH];
  int8    fc_inband_mode[SOC_MAX_NUM_PORTS][DNX_TMC_CONNECTION_DIRECTION_BOTH];
  int8    fc_ilkn_rt_calendar_mode[SOC_DNX_MAX_INTERLAKEN_PORTS][DNX_TMC_CONNECTION_DIRECTION_BOTH];
  int     fc_cl_sch_enable;   /* Enable CL SCHEDULER Flow Control */
  int     fc_calendar_coe_mode;
  int     fc_calendar_pause_resolution;  /* unit: usec */
  int     fc_calendar_e2e_status_of_entries;
  int     fc_calendar_indication_invert;  
  bcm_mac_t fc_coe_mac_address;
  uint16    fc_coe_ethertype;
  int       fc_coe_data_offset;
  int       fc_oob_ilkn_pad_sync_pin;
  int8    queue_level_interface_enable;
  int8    uc_q_pair2channel_id[SOC_DNX_MAX_INTERLAKEN_PORTS][JER2_ARAD_Q_PAIRS_ILKN];
  int8    mc_q_pair2channel_id[SOC_DNX_MAX_INTERLAKEN_PORTS][JER2_ARAD_Q_PAIRS_ILKN];
  int16   wred_packet_size;
  DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE multicast_egress_bitmap_group_range;
  int8    is_petrab_in_system;
  int8    is_immediate_sync;
  uint8   guaranteed_q_mode; /* which resource can be guaranteed for VOQs. must be one of SOC_DNX_GUARANTEED_Q_RESOURCE_* */
  uint8   mc_mode; /* should hold DNX_MC_*_MODE bits. */
  uint32  nof_mc_ids; /* number of egress MC IDs */
  uint32  nof_ingr_mc_ids; /* number of ingress MC IDs */
  uint32  ingress_mc_id_alloc_range_start;
  uint32  ingress_mc_id_alloc_range_end;
  uint32  egress_mc_id_alloc_range_start;
  uint32  egress_mc_id_alloc_range_end;
  uint32  ingress_mc_max_queue;    /* the maximum queue allowed as an ingress multicast destination */
  uint32  ingress_mc_nof_sysports; /* the number of allowed system ports as an ingress multicast destination */
  bcm_if_t ingress_mc_max_cud;     /* the maximum CUD allowed for ingress replications */
  bcm_if_t egress_mc_max_cud;      /* the maximum CUD allowed for egress replications */
  /*
   * Delete FIFO congestion thresholds
   */
  int     delete_fifo_almost_full_multicast_low_priority;
  int     delete_fifo_almost_full_multicast;
  int     delete_fifo_almost_full_all;
  int     delete_fifo_almost_full_except_tdm;
  int32   flow_base_q;
  int8    is_fe1600_in_system;
  uint8   mc_ing_encoding_mode; /* should hold JER2_ARAD_MC_DEST_ENCODING_* values. */
  uint16  max_ses; /* maximum SEs (Scheduling elements) */
  DNX_TMC_SCH_FLOW_IPF_CONFIG_MODE ipf_config_mode;  /*the ipf to inverse/proportinal mode*/
  uint16  cl_se_min;
  uint16  cl_se_max;
  uint16  fq_se_min;
  uint16  fq_se_max;
  uint16  hr_se_min;
  uint16  hr_se_max;
  uint16  port_hr_se_min;
  uint16  port_hr_se_max;
  int32   max_connectors; /* maximim connector resources */
  int8    max_egr_q_prio; /* Number of Egress queue priorities */
  int16   invalid_port_id_num; /* Invalid port id (outside the allowed range) */
  uint16  invalid_se_id_num; /* Invalid scheduling element id (outside the allowed range) */
  int32   invalid_voq_connector_id_num; /* Invalid VOQ connector id (outside the allowed range) */
  int8    nof_vsq_category; /* Number of VSQ categories */
  int16   hr_isq[SOC_DNX_DEFS_MAX(NOF_CORES)]; /* Reserved HR for ISQ */
  int     hr_fmqs[SOC_DNX_DEFS_MAX(NOF_CORES)][DNX_TMC_MULT_FABRIC_NOF_CLS]; /* Reserved HRs for FMQ */
  int8    is_port_tc_enable; /* Set is port TC feature to enable , disable */
  int8    cnm_mode;
  int8    cnm_voq_mapping_mode;
  uint8   various_bm;
  int8    ftmh_outlif_enlarge;
  uint8   otmh_header_used; /* Is the OTMH header used for any port */
  uint8   l2_encap_external_cpu_used; /* Is the L2 encapsulation is used for any port */
  soc_dnx_tm_crps_config_t crps_config;
} soc_dnx_tm_config_t;

typedef struct soc_dnx_config_qos_s {
  int8    nof_ing_elsp;
  int8    nof_ing_lif_cos;
  int8    nof_ing_pcp_vlan;
  int8    nof_ing_dp;
  int8    nof_egr_remark_id;
  int8    nof_egr_mpls_php;
  int8    nof_egr_pcp_vlan;
  int8    nof_ing_cos_opcode;
  int8    nof_egr_l2_i_tag;
  int8    nof_egr_dscp_exp_marking;
  uint32  mpls_elsp_label_range_min;
  uint32  mpls_elsp_label_range_max;
  int8    egr_pcp_vlan_dscp_exp_profile_id;
  int8    egr_pcp_vlan_dscp_exp_enable;
  int8    egr_remark_encap_enable;
  int8    dp_max;
  int8    ecn_mpls_one_bit_mode; /* If set ECN for MPLS encapsulation is in 1-bit mode, else 2-bits mode */
  int8    ecn_mpls_enabled; /* If set, ECN for MPLS encapsulation is enabled, otherwise, ECN is ignored */
  int8    ecn_ip_enabled;   /* If set, ECN for IP encapsulation is enabled, otherwise, ECN is ignored */
} soc_dnx_config_qos_t;

typedef struct soc_dnx_config_am_s {
    uint16 nof_am_resource_ids;
    uint16 nof_am_cosq_resource_ids;
    uint16 nof_am_template_ids;
} soc_dnx_config_am_t;

typedef struct soc_dnx_config_tdm_s {
  int8    max_user_define_bits;
  int16   min_cell_size;
  int16   max_cell_size;
  int8    is_fixed_opt_cell_size;
  int8    is_bypass;
  int8    is_packet;  /* if 0  packet tdm disabled for  device  else  packet tdm  enabled for device */
  int8    is_tdm_over_primary_pipe;  
} soc_dnx_config_tdm_t;

/* 
 * Set the global variable SOC_DNX_CONFIG(_unit)->extender.port_extender_init_status according to these values. 
 */ 
typedef enum {
    soc_dnx_extender_init_status_off,       /* Soc properties are not configured for port extender. */
    soc_dnx_extender_init_status_enabled,   /* Soc properties are configured but module is not initialized */
    soc_dnx_extender_init_status_init,      /* Soc properties are set and module is initialized. */
    soc_dnx_extender_init_status_count
} soc_dnx_extender_init_status;


/* Port extender module global configuration. */
typedef struct soc_dnx_config_extender_s {
  soc_dnx_extender_init_status  port_extender_init_status; /* Port extender init status */
} soc_dnx_config_extender_t;

/* Main Ardon/ATMF configuration structure */
typedef struct soc_dnx_config_ardon_s { 
    int  enable; 
} soc_dnx_config_ardon_t; 

/* Main JER2_QAX configuration structure */
typedef struct soc_dnx_config_jer2_qax_s { 
    /* JER2_QAX specific multicast configuration */
    uint16 nof_egress_bitmaps;
    uint16 nof_ingress_bitmaps;
    uint8 nof_egress_bitmap_bytes;
    uint8 nof_ingress_bitmap_bytes;
    uint8 ipsec;
    uint8 link_bonding_enable; /* If set, Link Bonding should be initialize */
    int per_packet_compensation_legacy_mode;

} soc_dnx_config_jer2_qax_t; 

typedef struct soc_dnx_config_s {
  soc_dnx_config_jer2_arad_t *jer2_arad;
  soc_dnx_config_jer2_jer_t *jer2_jer;
  soc_dnx_config_jer2_qax_t *jer2_qax;
  soc_dnx_config_ardon_t *ardon;
  soc_dnx_tm_config_t  tm;
  soc_dnx_config_tdm_t tdm;
  soc_dnx_config_qos_t qos;
  soc_dnx_config_am_t   am;
  soc_dnx_config_extender_t extender;
  uint8 emulation_system;
  soc_dnx_config_core_mode_t core_mode;
  soc_dnx_config_pdm_extension_t pdm_extension;
} soc_dnx_config_t;

typedef void (*soc_dnx_isr_f)(void *);

typedef struct soc_dnx_route_entry_s { 
  dnxc_soc_fabric_inband_route_t route;
  uint8                     is_valid;
  uint8                     group;
  uint8                     is_group_valid;
} soc_dnx_route_entry_t;

typedef struct soc_dnx_port_params_s { 
  uint32 comma_burst_conf[SOC_DNX_NOF_LINKS]; 
  uint32 control_burst_conf[SOC_DNX_NOF_INSTANCES_MAC]; 
  uint32 cl72_configured[SOC_MAX_NUM_PORTS];
} soc_dnx_port_params_t; 

typedef struct soc_dnx_control_s {
  soc_dnx_config_t *cfg;
  soc_dnx_isr_f     isr;
  soc_dnx_route_entry_t inband_route_entries[SOC_DNX_NUM_OF_ROUTES];
  soc_dnx_port_params_t port_params;
  uint8 is_silent_init;
  uint8 is_modid_set_called;
} soc_dnx_control_t;

#define SOC_IS_DNX_TYPE(dev_type) \
    ((dev_type) == BCM88690_DEVICE_ID)

#define SOC_DNX_CONTROL(unit) ((soc_dnx_control_t *)SOC_CONTROL(unit)->drv)
#define SOC_DNX_CONFIG(unit)  (SOC_DNX_CONTROL(unit)->cfg)
#define SOC_DNX_IS_MESH(unit) ((SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.connect_mode == JER2_ARAD_FABRIC_CONNECT_MODE_BACK2BACK)\
                                || (SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.connect_mode == JER2_ARAD_FABRIC_CONNECT_MODE_MESH))
#define SOC_DNX_PORT_PARAMS(unit)  (SOC_DNX_CONTROL(unit)->port_params)

#define SOC_DNX_SINGLE_FAP(unit)            (SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.connect_mode == JER2_ARAD_FABRIC_CONNECT_MODE_SINGLE_FAP)
#define SOC_IS_DNX_TDM_STANDARD(unit)       (SOC_DNX_CONFIG(unit)->jer2_arad->init.tdm_mode == JER2_ARAD_MGMT_TDM_MODE_TDM_STA)
#define SOC_IS_DNX_TDM_OPTIMIZE(unit)       (SOC_DNX_CONFIG(unit)->jer2_arad->init.tdm_mode == JER2_ARAD_MGMT_TDM_MODE_TDM_OPT)



#define DNX_ADD_PORT(ptype, nport) \
            si->ptype.port[si->ptype.num++] = nport; \
            if ( (si->ptype.min < 0) || (si->ptype.min > nport) ) {     \
                si->ptype.min = nport; \
            } \
            if (nport > si->ptype.max) { \
                si->ptype.max = nport; \
            } \
            SOC_PBMP_PORT_ADD(si->ptype.bitmap, nport);

            
#define DNX_ADD_DISABLED_PORT(ptype, nport) \
            si->ptype.port[si->ptype.num++] = nport; \
            if ( (si->ptype.min < 0) || (si->ptype.min > nport) ) {     \
                si->ptype.min = nport; \
            } \
            if (nport > si->ptype.max) { \
                si->ptype.max = nport; \
            } \
            SOC_PBMP_PORT_ADD(si->ptype.disabled_bitmap, nport);


int soc_dnx_info_config_common_tm(int unit);
int soc_dnx_str_prop_parse_tm_port_header_type(int unit, soc_port_t port, DNX_TMC_PORT_HEADER_TYPE *p_val_incoming, DNX_TMC_PORT_HEADER_TYPE *p_val_outgoing, uint32* is_hg_header);
int soc_dnx_str_prop_parse_tm_port_otmh_extensions_en(int unit, soc_port_t port, DNX_TMC_PORTS_OTMH_EXTENSIONS_EN *p_val);
int soc_dnx_str_prop_parse_flow_control_type(int unit, soc_port_t port, DNX_TMC_PORTS_FC_TYPE *p_val);
int soc_dnx_str_prop_fabric_connect_mode_get(int unit, DNX_TMC_FABRIC_CONNECT_MODE *fabric_connect_mode);
int soc_dnx_str_prop_fabric_ftmh_outlif_extension_get(int unit, DNX_TMC_PORTS_FTMH_EXT_OUTLIF *fabric_ftmh_outlif_extension);
int soc_dnx_common_init(int unit);
int soc_dnx_common_tm_init(int unit, DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE *multicast_egress_bitmap_group_range);
int soc_dnx_device_specific_info_config_direct(int unit);
int soc_dnx_device_specific_info_config_derived(int unit);
int soc_dnx_prop_parse_dtm_nof_remote_cores(int unit, int core, int region);
int soc_dnx_prop_parse_dtm_mapping_mode(int unit, int core, int region, int default_mode, int *mapping_mode);
int soc_dnx_prop_parse_ingress_congestion_management_pdm_extensions_get(int unit, soc_dnx_config_pdm_extension_t *pdm_extensions);
int soc_dnx_info_config(int unit);
int soc_dnx_info_config_blocks(int unit);
int soc_dnx_chip_type_set(int unit, uint16 dev_id);
int soc_dnx_prop_parse_admission_precedence_preference(int unit, uint32* preference);

extern int soc_dnx_device_core_mode(int unit, uint32* is_symmetric, uint32* nof_active_cores);
extern int soc_dnx_attach(int unit);
extern int soc_dnx_detach(int unit);
extern int soc_dnx_init(int unit, int reset);
extern int soc_dnx_deinit(int unit);
extern int soc_dnx_reinit(int unit, int reset);
extern int soc_dnx_dump(int unit, char *pfx);
extern void soc_dnx_chip_dump(int unit, soc_driver_t *d);
extern int soc_dnx_device_reset(int unit, int mdoe, int action);

/* Read/write internal registers */
extern int soc_dnx_reg_read(int unit, soc_reg_t reg, uint32 addr, 
                            uint64 *data);
extern int soc_dnx_reg_write(int unit, soc_reg_t reg, uint32 addr, 
                             uint64 data);

extern int soc_dnx_reg32_read(int unit, uint32 addr, uint32 *data);
extern int soc_dnx_reg32_write(int unit, uint32 addr, uint32 data);

extern int soc_dnx_reg64_read(int unit, uint32 addr, uint64 *data);
extern int soc_dnx_reg64_write(int unit, uint32 addr, uint64 data);

/* Read/Write internal memories */
extern int soc_dnx_mem_read(int unit, soc_mem_t mem, int copyno, int index, 
                            void *entry_data);
/*
extern int soc_dnx_mem_array_read(int unit, soc_mem_t mem, unsigned array_index, int copyno, int index,
                            void *entry_data);
*/
extern int soc_dnx_mem_write(int unit, soc_mem_t mem, int copyno, int index, 
                             void *entry_data);
/*
extern int soc_dnx_mem_array_write(int unit, soc_mem_t mem, unsigned array_index, int copyno, int index,
                             void *entry_data);
*/

/* 
 * Set fap ID 
 */
int
soc_dnx_petra_stk_modid_config_set(int unit, int fap_id);
/* 
 * Set fap ID 
 * Call pb init phase 2 
 */ 
int
soc_dnx_petra_stk_modid_set(int unit, int fap_id);

/* 
 * Enable traffic 
 */ 
int
soc_dnx_petra_stk_module_enable(int unit, int enable);
/*
 * Map block type to number of block instance. 
 */
int
soc_dnx_nof_block_instances(int unit,  soc_block_types_t block_types, int *nof_block_instances) ;

/*
 * Get unit AVS value 
 */
int
soc_dnx_avs_value_get(int unit, uint32* avs_value);

uint32
_soc_dnx_property_port_get(int unit, soc_port_t port,
                      const char *name, uint32 defl);

#ifdef BCM_WARM_BOOT_SUPPORT
uint8 
_bcm_dnx_switch_is_immediate_sync(int unit);
#endif /*BCM_WARM_BOOT_SUPPORT*/

/*init ofp rates configuration*/
int
_soc_dnx_jer2_arad_ofp_rates_set(int unit);

int
soc_dnx_cache_enable_init(int unit);
int soc_dnx_info_config_ports(int unit);

int soc_dnx_get_block_type_schan_id(int unit, soc_block_t block_type);


/* check whether next hop mac extension feature work in jer2_arad mode:
   for compatibility with JER2_ARAD, Disable hardware computation of Host-Index for DA.
   Instead, do it the Arad way: PMF will add pph learn extension (system header ), egress program editor will stamp the DA  
   if this soc property is disabled, then use hardware computation using the chicken bit */ 
#define SOC_IS_NEXT_HOP_MAC_EXT_JER2_ARAD_COMPATIBLE(unit) (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "next_hop_mac_ext_jer2_arad_compatible", 0))


#endif  /* _SOC_DNX_DRV_H */
