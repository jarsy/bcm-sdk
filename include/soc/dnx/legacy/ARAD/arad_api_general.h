/* $Id: jer2_arad_api_general.h,v 1.39 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER2_ARAD_API_GENERAL_INCLUDED__
/* { */
#define __JER2_ARAD_API_GENERAL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/dnx_config_defs.h>

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/TMC/tmc_api_general.h>
#include <soc/dnx/legacy/ARAD/arad_api_framework.h>
#include <soc/dnx/legacy/ARAD/arad_framework.h>

#include <soc/dnx/legacy/SAND/SAND_FM/sand_user_callback.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_chip_defines.h>

#include <soc/dnx/swstate/types/lag_types.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* 
 * Maximum number of TM-domains in the system.
 */
#define JER2_ARAD_NOF_TM_DOMAIN_IN_SYSTEM     (DNX_TMC_NOF_TM_DOMAIN_IN_SYSTEM)

/*
 * Maximum number of FAPs in the system.
 */
#define JER2_ARAD_NOF_FAPS_IN_SYSTEM     (2048)
#define JER2_ARAD_INVALID_FAP_ID          JER2_ARAD_NOF_FAPS_IN_SYSTEM
#define JER2_ARAD_MAX_FAP_ID             (JER2_ARAD_NOF_FAPS_IN_SYSTEM-1)
#define JER2_ARAD_MAX_DEVICE_ID          (JER2_ARAD_NOF_FAPS_IN_SYSTEM-1)

/*
 *	If Egress MC 16K members mode is enabled,
 *  the FAP-IDs range in the system is limited to 0 - 511.
 */
#define JER2_ARAD_MAX_FAP_ID_IF_MC_16K_EN 511

/*
 *  Typically used when the function demands unit parameter,
 *  but the device id is irrelevant in the given context,
 *  and the value is not used in fact.
 */
#define JER2_ARAD_DEVICE_ID_IRRELEVANT   JER2_ARAD_MAX_DEVICE_ID

/*     Maximal number of physical ports.                       */
#define JER2_ARAD_NOF_SYS_PHYS_PORTS                   DNX_TMC_NOF_SYS_PHYS_PORTS_JER2_ARAD
#define JER2_ARAD_NOF_SYS_PHYS_PORTS_INDIRECT          DNX_TMC_NOF_SYS_PHYS_PORTS

#define JER2_ARAD_NOF_SYS_PHYS_PORTS_GET(unit) (JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit) ? \
    JER2_ARAD_NOF_SYS_PHYS_PORTS_INDIRECT : JER2_ARAD_NOF_SYS_PHYS_PORTS)

#define JER2_ARAD_MAX_SYSTEM_PHYSICAL_PORT_ID          DNX_TMC_MAX_SYSTEM_PHYSICAL_PORT_ID_JER2_ARAD
#define JER2_ARAD_MAX_SYSTEM_PHYSICAL_PORT_ID_INDIRECT DNX_TMC_MAX_SYSTEM_PHYSICAL_PORT_ID

/*     Maximal number of physical ports id is indication for Invalid port */
#define JER2_ARAD_SYS_PHYS_PORT_INVALID_DIRECT   JER2_ARAD_MAX_SYSTEM_PHYSICAL_PORT_ID
#define JER2_ARAD_SYS_PHYS_PORT_INVALID_INDIRECT JER2_ARAD_MAX_SYSTEM_PHYSICAL_PORT_ID_INDIRECT
#define JER2_ARAD_SYS_PHYS_PORT_INVALID(unit) (JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit) ? \
  JER2_ARAD_SYS_PHYS_PORT_INVALID_INDIRECT : JER2_ARAD_SYS_PHYS_PORT_INVALID_DIRECT)

/*     Maximal number of logical ports.                        */
#define JER2_ARAD_NOF_SYSTEM_PORTS              DNX_TMC_NOF_SYSTEM_PORTS_JER2_ARAD
#define JER2_ARAD_MAX_SYSTEM_PORT_ID            (JER2_ARAD_NOF_SYSTEM_PORTS - 1)

/*
 * Maximum number of LBP destination IDs.
 * Range 0:4095
 */
#define JER2_ARAD_NOF_LBP_DESTINATIONS   JER2_ARAD_NOF_SYS_PHYS_PORTS
#define JER2_ARAD_MAX_LBP_DESTINATION    (JER2_ARAD_NOF_LBP_DESTINATIONS - 1)

/*
 * Number of queues in JER2_ARAD - 96K.
 */
#define JER2_ARAD_MAX_QUEUE_ID(unit)           (SOC_DNX_DEFS_GET(unit, nof_queues) - 1)


/*
 * Number of queue types in JER2_ARAD - 16.
 */
#define JER2_ARAD_NOF_QUEUE_TYPES        16

/*
 * Number of drop precedences.
 */
#define JER2_ARAD_NOF_DROP_PRECEDENCE    4
#define JER2_ARAD_MAX_DROP_PRECEDENCE    (JER2_ARAD_NOF_DROP_PRECEDENCE-1)

/* Ingress-Packet-traffic-class: Value 0 */
#define JER2_ARAD_TR_CLS_MIN                0
/* Ingress-Packet-traffic-class: Value 7 */
#define JER2_ARAD_TR_CLS_MAX                JER2_ARAD_NOF_TRAFFIC_CLASSES-1

/* JER2_ARAD number of traffic classes.*/
#define JER2_ARAD_NOF_TRAFFIC_CLASSES        DNX_TMC_NOF_TRAFFIC_CLASSES

/*
 * Number of FAP-data-ports in JER2_ARAD.
 */
#define JER2_ARAD_NOF_FAP_PORTS                  DNX_TMC_NOF_FAP_PORTS_JER2_ARAD
#define JER2_ARAD_MAX_FAP_PORT_ID                (JER2_ARAD_NOF_FAP_PORTS-1)
#define JER2_ARAD_FAP_PORT_ID_INVALID            JER2_ARAD_NOF_FAP_PORTS

/* 
 * When setting CUD 16bit to 17bit extension, there are only 128 fap port avail be.
 * And each tm port should configured the same for <fap_port_id> and <fap_port_id + 128>
 */
#define JER2_ARAD_NOF_FAP_PORTS_EGR_MC_17BIT_CUD    (JER2_ARAD_NOF_FAP_PORTS / 2)
#define JER2_ARAD_MAX_FAP_PORT_ID_EGR_MC_17BIT_CUD (JER2_ARAD_NOF_FAP_PORTS_EGR_MC_17BIT_CUD-1)
#define JER2_ARAD_FAP_PORTS_OFFSET_EGR_MC_17BIT_CUD 128
/*
 * ERP (Egress Replication Port), is responsible of
 *  replicating packets in the egress (Multicast)
 * JER2_ARAD_FAP_EGRESS_REPLICATION_SCH_PORT_ID:
 *  The Port ID When attaching flow / aggregate scheduler to it
 * JER2_ARAD_FAP_EGRESS_REPLICATION_IPS_PORT_ID:
 *  The port ID when mapping the system port to it.
 */
#define JER2_ARAD_FAP_EGRESS_REPLICATION_BASE_Q_PAIR       248
#define JER2_ARAD_FAP_EGRESS_REPLICATION_IPS_PORT_ID       255

#define JER2_ARAD_FAP_OAMP_PORT_ID                         232

#define JER2_ARAD_WARP_CORE_0               0
#define JER2_ARAD_WARP_CORE_1               1
#define JER2_ARAD_WARP_CORE_2               2
#define JER2_ARAD_WARP_CORE_3               3
#define JER2_ARAD_WARP_CORE_4               4
#define JER2_ARAD_WARP_CORE_5               5
#define JER2_ARAD_WARP_CORE_6               6
#define JER2_ARAD_WARP_CORE_STAT            7
#define JER2_ARAD_NOF_WARP_CORES            8

/*
 *  Maximal Number of NIFS
 */
#define JER2_ARAD_MAX_NOF_NIFS           (JER2_ARAD_NOF_MAC_LANES * JER2_ARAD_MAX_NIFS_PER_MAL)

/*
 *  The coefficient to convert 1Kilo-bit-per-second to bit-per-second (e.g.).
 */
#define JER2_ARAD_RATE_1K                           1000
/*
* Maximal interface rate, in Mega-bits per second.
* This is the upper boundary, it can be lower
*  depending on the credit size
*/
/* #define JER2_ARAD_SCH_MAX_RATE_MBPS                 (JER2_ARAD_RATE_1K * 105) */

/*
 * Maximal interface rate, in Mega-bits per second.
 * This is the upper boundary, it can be lower
 *  depending on the credit size
 */
#define JER2_ARAD_SCH_MAX_RATE_MBPS(unit)        (DNX_TMC_SCH_MAX_RATE_KBPS_JER2_ARAD(unit))
#define JER2_ARAD_IF_MAX_RATE_MBPS(unit)         (DNX_TMC_IF_MAX_RATE_MBPS_JER2_ARAD(unit))

/*
 * Maximal interface rate, in Kilo-bits per second.
 * This is the upper boundary, it can be lower
 *  depending on the credit size
 */
#define JER2_ARAD_IF_MAX_RATE_KBPS(unit)         (DNX_TMC_IF_MAX_RATE_KBPS_JER2_ARAD(unit))

/*
 * Default Values
 * {
 */

/*
 * This queue default as no real meaning,
 * just as a value to put in erase.
 */
#define JER2_ARAD_DEFAULT_QUEUE_TYPE         (0)
/*
 * This FAP id default as no real meaning,
 * just as a value to put in erase.
 */
#define JER2_ARAD_DEFAULT_DESTINATION_FAP    (JER2_ARAD_NOF_FAPS_IN_SYSTEM-1)
/*
 * This id default as no real meaning,
 * just as a value to put in erase.
 */
#define JER2_ARAD_DEFAULT_FAP_PORT_ID       (JER2_ARAD_NOF_FAP_PORTS-1)

/*
* CPU Default port identifier is 0
*/
#define JER2_ARAD_DEFAULT_CPU_PORT_ID       (0)

/*
 * End Default Values
 * }
 */

/*
 * This is the maximum ingress queue size available in the device.
 * In granularity of bytes.
 */
#define JER2_ARAD_MAX_QUEUE_SIZE_BYTES        (2048UL * 1024 * 1024)

#define JER2_ARAD_MAX_QUEUE_SIZE_BDS        	  (3 * 512 * 1024)

#define JER2_ARAD_B0_MAX_ECN_QUEUE_BYTES 0x7e00000 /* maximum queue size in bytes that JER2_ARAD Bo/B1 supports */

/* Multicast-ID min: Value 0 */
#define JER2_ARAD_MULT_ID_MIN                 0
/* Multicast-ID max: */
#define JER2_ARAD_MAX_MC_ID(unit)         ((SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids > SOC_DNX_CONFIG(unit)->tm.nof_mc_ids) ? \
                                     (SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids - 1) : (SOC_DNX_CONFIG(unit)->tm.nof_mc_ids - 1))

#define JER2_ARAD_MULT_NOF_MULTICAST_GROUPS       DNX_TMC_MULT_NOF_MULTICAST_GROUPS_JER2_ARAD

#define JER2_ARAD_MULT_NOF_MULT_VLAN_ERP_GROUPS   (8*1024) /* 4K in Petra */

/*
 * Number of Snoop Commands.
 * There are 15 Snoop Commands altogether 1-15, 0 means disabling Snooping.
 */
#define JER2_ARAD_NOF_SNOOP_COMMAND_INDEX   15
#define JER2_ARAD_MAX_SNOOP_COMMAND_INDEX   (JER2_ARAD_NOF_SNOOP_COMMAND_INDEX)
/*
* Number of Signatures.
*/
#define JER2_ARAD_NOF_SIGNATURE        4
#define JER2_ARAD_MAX_SIGNATURE        (JER2_ARAD_NOF_SIGNATURE-1)

/*
 * Packet Size (Bytes)
 */
#define JER2_ARAD_NOF_PACKET_BYTES     (16*1024)
#define JER2_ARAD_MAX_PACKET_BYTES     (JER2_ARAD_NOF_PACKET_BYTES-1)

/*
 * Copy-unique-data (e.g. outlif) index
 */
#define JER2_ARAD_NOF_CUD_ID         65536
#define JER2_ARAD_MAX_CUD_ID         (JER2_ARAD_NOF_CUD_ID-1)

/*     Maximal number of LAG groups.                        */
#define JER2_ARAD_NOF_LAG_GROUPS              (jer2_arad_ports_lag_nof_lag_groups_get_unsafe(unit))
#define JER2_ARAD_MAX_LAG_GROUP_ID            (JER2_ARAD_NOF_LAG_GROUPS - 1)

/*     Maximal number of LAG entries.                        */
#define JER2_ARAD_NOF_LAG_ENTRIES             (jer2_arad_ports_lag_nof_lag_entries_get_unsafe(unit))
#define JER2_ARAD_MAX_LAG_ENTRY_ID            (JER2_ARAD_NOF_LAG_ENTRIES - 1)

/*
 *  Rate configuration calendar sets - A and B
 */
#define JER2_ARAD_NOF_CALENDAR_SETS           2

#define JER2_ARAD_CONNECTION_DIRECTION_NDX_MAX         (DNX_TMC_NOF_CONNECTION_DIRECTIONS-1)
#define JER2_ARAD_PORT_DIRECTION_NDX_MAX               (DNX_TMC_NOF_CONNECTION_DIRECTIONS-1)

/* } */

/*************
 * MACROS    *
 *************/
/* { */

#define JER2_ARAD_DO_NOTHING_AND_EXIT                       goto exit

/* Data Ports Macros { */
#define JER2_ARAD_FRST_CPU_PORT_ID              0


#define JER2_ARAD_OLP_PORT_ID                   240

#define JER2_ARAD_ERP_PORT_ID                   JER2_ARAD_FAP_EGRESS_REPLICATION_IPS_PORT_ID

#define JER2_ARAD_OAMP_PORT_ID                  JER2_ARAD_FAP_OAMP_PORT_ID

#define JER2_ARAD_IS_OLP_FAP_PORT_ID(port_id) ((port_id) == JER2_ARAD_OLP_PORT_ID)

/* Note: ERP is a virtual port, not a data port */
/* ERP TM port must be 255. In egress perspective the base-q-pair must be 248 */
#define JER2_ARAD_IS_ERP_PORT_ID(port_id) ((port_id) == JER2_ARAD_ERP_PORT_ID)
/* Data Ports Macros } */

#define JER2_ARAD_IS_OAMP_FAP_PORT_ID(port_id) ((port_id) == JER2_ARAD_OAMP_PORT_ID)

#define JER2_ARAD_IS_ERR_RES(res_)                       \
  (dnx_sand_get_error_code_from_error_word(res_) != DNX_SAND_OK)

/* Interface Ports Macros { */
#define JER2_ARAD_IS_NIF_ID(if_id) \
  ((if_id) <= JER2_ARAD_MAX_NIF_ID)

#define JER2_ARAD_IS_NON_NIF_ID(if_id) \
  (!JER2_ARAD_IS_NIF_ID(if_id))

#define JER2_ARAD_IS_CPU_IF_ID(if_id) \
  ((if_id) == JER2_ARAD_IF_ID_CPU)

#define JER2_ARAD_IS_OLP_IF_ID(if_id) \
  ((if_id) == JER2_ARAD_IF_ID_OLP)

#define JER2_ARAD_IS_OAMP_IF_ID(if_id) \
  ((if_id) == JER2_ARAD_IF_ID_OAMP)

#define JER2_ARAD_IS_RCY_IF_ID(if_id) \
  ((if_id) == JER2_ARAD_IF_ID_RCY)

#define JER2_ARAD_IS_TM_INTERNAL_PKT_IF_ID(if_id) \
  ((if_id) == JER2_ARAD_IF_ID_TM_INTERNAL_PKT)

#define JER2_ARAD_IS_ERP_IF_ID(if_id) \
  ((if_id) == JER2_ARAD_IF_ID_ERP)

#define JER2_ARAD_IS_ECI_IF_ID(if_id) \
  ( JER2_ARAD_IS_CPU_IF_ID(if_id) || JER2_ARAD_IS_OLP_IF_ID(if_id) || JER2_ARAD_IS_OAMP_IF_ID(if_id) )

#define JER2_ARAD_IS_NONE_IF_ID(if_id) \
  ((if_id) == JER2_ARAD_IF_ID_NONE)

#define JER2_ARAD_IS_MAL_EQUIVALENT_ID(mal_id) \
  (JER2_ARAD_IS_ECI_IF_ID(mal_id) || JER2_ARAD_IS_RCY_IF_ID(mal_id) || JER2_ARAD_IS_ERP_IF_ID(mal_id))

/* Interface Ports Macros } */
#define JER2_ARAD_NIF2WC_NDX(if_id) \
  ((uint32)((if_id) / JER2_ARAD_MAX_NIFS_PER_WC))

#define JER2_ARAD_WC2NIF_NDX(wc_id) \
  ((wc_id) * JER2_ARAD_MAX_NIFS_PER_WC)

#define JER2_ARAD_IF2WC_NDX(if_id) \
  (JER2_ARAD_IS_NIF_ID(if_id)?JER2_ARAD_NIF2WC_NDX(if_id):(uint32)(if_id))

#define JER2_ARAD_WC2IF_NDX(wc_id) \
  ((wc_id)<JER2_ARAD_NOF_WRAP_CORE?JER2_ARAD_WC2NIF_NDX(wc_id):(wc_id))

/*
 *  TRUE for network interfaces 0, 4, 8... (first interface in each MAL).
 */
#define JER2_ARAD_IS_WC_NIF_ID(if_id) \
  (uint8)( (if_id <= JER2_ARAD_INTERN_MAX_NIFS) && \
   (JER2_ARAD_WC2NIF_NDX(JER2_ARAD_NIF2WC_NDX(if_id)) == (uint32)(if_id)) )

#define JER2_ARAD_IS_DIRECTION_RX(dir) \
  DNX_SAND_NUM2BOOL(((dir) == JER2_ARAD_CONNECTION_DIRECTION_RX  ) || ((dir) == JER2_ARAD_CONNECTION_DIRECTION_BOTH))

#define JER2_ARAD_IS_DIRECTION_TX(dir) \
  DNX_SAND_NUM2BOOL(((dir) == JER2_ARAD_CONNECTION_DIRECTION_TX  ) || ((dir) == JER2_ARAD_CONNECTION_DIRECTION_BOTH))

#define JER2_ARAD_IS_DIRECTION_REC(dir) \
  DNX_SAND_NUM2BOOL((dir) == JER2_ARAD_FC_DIRECTION_REC  )

#define JER2_ARAD_IS_DIRECTION_GEN(dir) \
  DNX_SAND_NUM2BOOL((dir) == JER2_ARAD_FC_DIRECTION_GEN  )

#define JER2_ARAD_MULTICAST_DEST_INDICATION    (DNX_SAND_BIT(14))

/*
 *	Maximal Number of Network Interfaces
 */
#define JER2_ARAD_IF_NOF_NIFS 32
/*
 *  Maximal number of Network devices for
 *  JER2_ARAD family devices JER2_ARAD-A, JER2_ARAD-B...,
 *  using internal representation
 */
#define JER2_ARAD_INTERN_MAX_NIFS 64
/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

#define JER2_ARAD_FAR_DEVICE_TYPE_FE1                          DNX_TMC_FAR_DEVICE_TYPE_FE1
#define JER2_ARAD_FAR_DEVICE_TYPE_FE2                          DNX_TMC_FAR_DEVICE_TYPE_FE2
#define JER2_ARAD_FAR_DEVICE_TYPE_FE3                          DNX_TMC_FAR_DEVICE_TYPE_FE3
#define JER2_ARAD_FAR_DEVICE_TYPE_FAP                          DNX_TMC_FAR_DEVICE_TYPE_FAP
typedef DNX_TMC_FAR_DEVICE_TYPE                                JER2_ARAD_FAR_DEVICE_TYPE;

#define JER2_ARAD_IF_TYPE_NONE                                 DNX_TMC_IF_TYPE_NONE
#define JER2_ARAD_IF_TYPE_CPU                                  DNX_TMC_IF_TYPE_CPU
#define JER2_ARAD_IF_TYPE_RCY                                  DNX_TMC_IF_TYPE_RCY
#define JER2_ARAD_IF_TYPE_OLP                                  DNX_TMC_IF_TYPE_OLP
#define JER2_ARAD_IF_TYPE_NIF                                  DNX_TMC_IF_TYPE_NIF
#define JER2_ARAD_IF_TYPE_ERP                                  DNX_TMC_IF_TYPE_ERP
#define JER2_ARAD_IF_TYPE_OAMP                                 DNX_TMC_IF_TYPE_OAMP
typedef DNX_TMC_INTERFACE_TYPE                                 JER2_ARAD_INTERFACE_TYPE;

#define JER2_ARAD_IF_ID_0                                      DNX_TMC_IF_ID_0
#define JER2_ARAD_IF_ID_1                                      DNX_TMC_IF_ID_1
#define JER2_ARAD_IF_ID_2                                      DNX_TMC_IF_ID_2
#define JER2_ARAD_IF_ID_3                                      DNX_TMC_IF_ID_3
#define JER2_ARAD_IF_ID_4                                      DNX_TMC_IF_ID_4
#define JER2_ARAD_IF_ID_5                                      DNX_TMC_IF_ID_5
#define JER2_ARAD_IF_ID_6                                      DNX_TMC_IF_ID_6
#define JER2_ARAD_IF_ID_7                                      DNX_TMC_IF_ID_7
#define JER2_ARAD_IF_ID_8                                      DNX_TMC_IF_ID_8
#define JER2_ARAD_IF_ID_9                                      DNX_TMC_IF_ID_9
#define JER2_ARAD_IF_ID_10                                     DNX_TMC_IF_ID_10
#define JER2_ARAD_IF_ID_11                                     DNX_TMC_IF_ID_11
#define JER2_ARAD_IF_ID_12                                     DNX_TMC_IF_ID_12
#define JER2_ARAD_IF_ID_13                                     DNX_TMC_IF_ID_13
#define JER2_ARAD_IF_ID_14                                     DNX_TMC_IF_ID_14
#define JER2_ARAD_IF_ID_15                                     DNX_TMC_IF_ID_15
#define JER2_ARAD_IF_ID_16                                     DNX_TMC_IF_ID_16
#define JER2_ARAD_IF_ID_17                                     DNX_TMC_IF_ID_17
#define JER2_ARAD_IF_ID_18                                     DNX_TMC_IF_ID_18
#define JER2_ARAD_IF_ID_19                                     DNX_TMC_IF_ID_19
#define JER2_ARAD_IF_ID_20                                     DNX_TMC_IF_ID_20
#define JER2_ARAD_IF_ID_21                                     DNX_TMC_IF_ID_21
#define JER2_ARAD_IF_ID_22                                     DNX_TMC_IF_ID_22
#define JER2_ARAD_IF_ID_23                                     DNX_TMC_IF_ID_23
#define JER2_ARAD_IF_ID_24                                     DNX_TMC_IF_ID_24
#define JER2_ARAD_IF_ID_25                                     DNX_TMC_IF_ID_25
#define JER2_ARAD_IF_ID_26                                     DNX_TMC_IF_ID_26
#define JER2_ARAD_IF_ID_27                                     DNX_TMC_IF_ID_27
#define JER2_ARAD_IF_ID_28                                     DNX_TMC_IF_ID_28
#define JER2_ARAD_IF_ID_29                                     DNX_TMC_IF_ID_29
#define JER2_ARAD_IF_ID_30                                     DNX_TMC_IF_ID_30
#define JER2_ARAD_IF_ID_31                                     DNX_TMC_IF_ID_31
#define JER2_ARAD_NIF_ID_XAUI_0                                DNX_TMC_NIF_ID_XAUI_0
#define JER2_ARAD_NIF_ID_XAUI_1                                DNX_TMC_NIF_ID_XAUI_1
#define JER2_ARAD_NIF_ID_XAUI_2                                DNX_TMC_NIF_ID_XAUI_2
#define JER2_ARAD_NIF_ID_XAUI_3                                DNX_TMC_NIF_ID_XAUI_3
#define JER2_ARAD_NIF_ID_XAUI_4                                DNX_TMC_NIF_ID_XAUI_4
#define JER2_ARAD_NIF_ID_XAUI_5                                DNX_TMC_NIF_ID_XAUI_5
#define JER2_ARAD_NIF_ID_XAUI_6                                DNX_TMC_NIF_ID_XAUI_6
#define JER2_ARAD_NIF_ID_XAUI_7                                DNX_TMC_NIF_ID_XAUI_7
#define JER2_ARAD_NIF_ID_RXAUI_0                               DNX_TMC_NIF_ID_RXAUI_0
#define JER2_ARAD_NIF_ID_RXAUI_1                               DNX_TMC_NIF_ID_RXAUI_1
#define JER2_ARAD_NIF_ID_RXAUI_2                               DNX_TMC_NIF_ID_RXAUI_2
#define JER2_ARAD_NIF_ID_RXAUI_3                               DNX_TMC_NIF_ID_RXAUI_3
#define JER2_ARAD_NIF_ID_RXAUI_4                               DNX_TMC_NIF_ID_RXAUI_4
#define JER2_ARAD_NIF_ID_RXAUI_5                               DNX_TMC_NIF_ID_RXAUI_5
#define JER2_ARAD_NIF_ID_RXAUI_6                               DNX_TMC_NIF_ID_RXAUI_6
#define JER2_ARAD_NIF_ID_RXAUI_7                               DNX_TMC_NIF_ID_RXAUI_7
#define JER2_ARAD_NIF_ID_RXAUI_8                               DNX_TMC_NIF_ID_RXAUI_8
#define JER2_ARAD_NIF_ID_RXAUI_9                               DNX_TMC_NIF_ID_RXAUI_9
#define JER2_ARAD_NIF_ID_RXAUI_10                              DNX_TMC_NIF_ID_RXAUI_10
#define JER2_ARAD_NIF_ID_RXAUI_11                              DNX_TMC_NIF_ID_RXAUI_11
#define JER2_ARAD_NIF_ID_RXAUI_12                              DNX_TMC_NIF_ID_RXAUI_12
#define JER2_ARAD_NIF_ID_RXAUI_13                              DNX_TMC_NIF_ID_RXAUI_13
#define JER2_ARAD_NIF_ID_RXAUI_14                              DNX_TMC_NIF_ID_RXAUI_14
#define JER2_ARAD_NIF_ID_RXAUI_15                              DNX_TMC_NIF_ID_RXAUI_15
#define JER2_ARAD_NIF_ID_SGMII_0                               DNX_TMC_NIF_ID_SGMII_0
#define JER2_ARAD_NIF_ID_SGMII_1                               DNX_TMC_NIF_ID_SGMII_1
#define JER2_ARAD_NIF_ID_SGMII_2                               DNX_TMC_NIF_ID_SGMII_2
#define JER2_ARAD_NIF_ID_SGMII_3                               DNX_TMC_NIF_ID_SGMII_3
#define JER2_ARAD_NIF_ID_SGMII_4                               DNX_TMC_NIF_ID_SGMII_4
#define JER2_ARAD_NIF_ID_SGMII_5                               DNX_TMC_NIF_ID_SGMII_5
#define JER2_ARAD_NIF_ID_SGMII_6                               DNX_TMC_NIF_ID_SGMII_6
#define JER2_ARAD_NIF_ID_SGMII_7                               DNX_TMC_NIF_ID_SGMII_7
#define JER2_ARAD_NIF_ID_SGMII_8                               DNX_TMC_NIF_ID_SGMII_8
#define JER2_ARAD_NIF_ID_SGMII_9                               DNX_TMC_NIF_ID_SGMII_9
#define JER2_ARAD_NIF_ID_SGMII_10                              DNX_TMC_NIF_ID_SGMII_10
#define JER2_ARAD_NIF_ID_SGMII_11                              DNX_TMC_NIF_ID_SGMII_11
#define JER2_ARAD_NIF_ID_SGMII_12                              DNX_TMC_NIF_ID_SGMII_12
#define JER2_ARAD_NIF_ID_SGMII_13                              DNX_TMC_NIF_ID_SGMII_13
#define JER2_ARAD_NIF_ID_SGMII_14                              DNX_TMC_NIF_ID_SGMII_14
#define JER2_ARAD_NIF_ID_SGMII_15                              DNX_TMC_NIF_ID_SGMII_15
#define JER2_ARAD_NIF_ID_SGMII_16                              DNX_TMC_NIF_ID_SGMII_16
#define JER2_ARAD_NIF_ID_SGMII_17                              DNX_TMC_NIF_ID_SGMII_17
#define JER2_ARAD_NIF_ID_SGMII_18                              DNX_TMC_NIF_ID_SGMII_18
#define JER2_ARAD_NIF_ID_SGMII_19                              DNX_TMC_NIF_ID_SGMII_19
#define JER2_ARAD_NIF_ID_SGMII_20                              DNX_TMC_NIF_ID_SGMII_20
#define JER2_ARAD_NIF_ID_SGMII_21                              DNX_TMC_NIF_ID_SGMII_21
#define JER2_ARAD_NIF_ID_SGMII_22                              DNX_TMC_NIF_ID_SGMII_22
#define JER2_ARAD_NIF_ID_SGMII_23                              DNX_TMC_NIF_ID_SGMII_23
#define JER2_ARAD_NIF_ID_SGMII_24                              DNX_TMC_NIF_ID_SGMII_24
#define JER2_ARAD_NIF_ID_SGMII_25                              DNX_TMC_NIF_ID_SGMII_25
#define JER2_ARAD_NIF_ID_SGMII_26                              DNX_TMC_NIF_ID_SGMII_26
#define JER2_ARAD_NIF_ID_SGMII_27                              DNX_TMC_NIF_ID_SGMII_27
#define JER2_ARAD_NIF_ID_SGMII_28                              DNX_TMC_NIF_ID_SGMII_28
#define JER2_ARAD_NIF_ID_SGMII_29                              DNX_TMC_NIF_ID_SGMII_29
#define JER2_ARAD_NIF_ID_SGMII_30                              DNX_TMC_NIF_ID_SGMII_30
#define JER2_ARAD_NIF_ID_SGMII_31                              DNX_TMC_NIF_ID_SGMII_31
#define JER2_ARAD_NIF_ID_ILKN_0                                DNX_TMC_NIF_ID_ILKN_0
#define JER2_ARAD_NIF_ID_ILKN_1                                DNX_TMC_NIF_ID_ILKN_1
#define JER2_ARAD_NIF_ID_ILKN_TDM_0                            DNX_TMC_NIF_ID_ILKN_TDM_0
#define JER2_ARAD_NIF_ID_ILKN_TDM_1                            DNX_TMC_NIF_ID_ILKN_TDM_1
#define JER2_ARAD_NIF_ID_CGE_0                                 DNX_TMC_NIF_ID_CGE_0
#define JER2_ARAD_NIF_ID_CGE_1                                 DNX_TMC_NIF_ID_CGE_1
#define JER2_ARAD_NIF_ID_10GBASE_R_0                           DNX_TMC_NIF_ID_10GBASE_R_0
#define JER2_ARAD_NIF_ID_10GBASE_R_1                           DNX_TMC_NIF_ID_10GBASE_R_1
#define JER2_ARAD_NIF_ID_10GBASE_R_2                           DNX_TMC_NIF_ID_10GBASE_R_2
#define JER2_ARAD_NIF_ID_10GBASE_R_3                           DNX_TMC_NIF_ID_10GBASE_R_3
#define JER2_ARAD_NIF_ID_10GBASE_R_4                           DNX_TMC_NIF_ID_10GBASE_R_4
#define JER2_ARAD_NIF_ID_10GBASE_R_5                           DNX_TMC_NIF_ID_10GBASE_R_5
#define JER2_ARAD_NIF_ID_10GBASE_R_6                           DNX_TMC_NIF_ID_10GBASE_R_6
#define JER2_ARAD_NIF_ID_10GBASE_R_7                           DNX_TMC_NIF_ID_10GBASE_R_7
#define JER2_ARAD_NIF_ID_10GBASE_R_8                           DNX_TMC_NIF_ID_10GBASE_R_8
#define JER2_ARAD_NIF_ID_10GBASE_R_9                           DNX_TMC_NIF_ID_10GBASE_R_9
#define JER2_ARAD_NIF_ID_10GBASE_R_10                          DNX_TMC_NIF_ID_10GBASE_R_10
#define JER2_ARAD_NIF_ID_10GBASE_R_11                          DNX_TMC_NIF_ID_10GBASE_R_11
#define JER2_ARAD_NIF_ID_10GBASE_R_12                          DNX_TMC_NIF_ID_10GBASE_R_12
#define JER2_ARAD_NIF_ID_10GBASE_R_13                          DNX_TMC_NIF_ID_10GBASE_R_13
#define JER2_ARAD_NIF_ID_10GBASE_R_14                          DNX_TMC_NIF_ID_10GBASE_R_14
#define JER2_ARAD_NIF_ID_10GBASE_R_15                          DNX_TMC_NIF_ID_10GBASE_R_15
#define JER2_ARAD_NIF_ID_10GBASE_R_16                          DNX_TMC_NIF_ID_10GBASE_R_16
#define JER2_ARAD_NIF_ID_10GBASE_R_17                          DNX_TMC_NIF_ID_10GBASE_R_17
#define JER2_ARAD_NIF_ID_10GBASE_R_18                          DNX_TMC_NIF_ID_10GBASE_R_18
#define JER2_ARAD_NIF_ID_10GBASE_R_19                          DNX_TMC_NIF_ID_10GBASE_R_19
#define JER2_ARAD_NIF_ID_10GBASE_R_20                          DNX_TMC_NIF_ID_10GBASE_R_20
#define JER2_ARAD_NIF_ID_10GBASE_R_21                          DNX_TMC_NIF_ID_10GBASE_R_21
#define JER2_ARAD_NIF_ID_10GBASE_R_22                          DNX_TMC_NIF_ID_10GBASE_R_22
#define JER2_ARAD_NIF_ID_10GBASE_R_23                          DNX_TMC_NIF_ID_10GBASE_R_23
#define JER2_ARAD_NIF_ID_10GBASE_R_24                          DNX_TMC_NIF_ID_10GBASE_R_24
#define JER2_ARAD_NIF_ID_10GBASE_R_25                          DNX_TMC_NIF_ID_10GBASE_R_25
#define JER2_ARAD_NIF_ID_10GBASE_R_26                          DNX_TMC_NIF_ID_10GBASE_R_26
#define JER2_ARAD_NIF_ID_10GBASE_R_27                          DNX_TMC_NIF_ID_10GBASE_R_27
#define JER2_ARAD_NIF_ID_10GBASE_R_28                          DNX_TMC_NIF_ID_10GBASE_R_28
#define JER2_ARAD_NIF_ID_10GBASE_R_29                          DNX_TMC_NIF_ID_10GBASE_R_29
#define JER2_ARAD_NIF_ID_10GBASE_R_30                          DNX_TMC_NIF_ID_10GBASE_R_30
#define JER2_ARAD_NIF_ID_10GBASE_R_31                          DNX_TMC_NIF_ID_10GBASE_R_31
#define JER2_ARAD_NIF_ID_XLGE_0                                DNX_TMC_NIF_ID_XLGE_0
#define JER2_ARAD_NIF_ID_XLGE_1                                DNX_TMC_NIF_ID_XLGE_1
#define JER2_ARAD_NIF_ID_XLGE_2                                DNX_TMC_NIF_ID_XLGE_2
#define JER2_ARAD_NIF_ID_XLGE_3                                DNX_TMC_NIF_ID_XLGE_3
#define JER2_ARAD_NIF_ID_XLGE_4                                DNX_TMC_NIF_ID_XLGE_4
#define JER2_ARAD_NIF_ID_XLGE_5                                DNX_TMC_NIF_ID_XLGE_5
#define JER2_ARAD_NIF_ID_XLGE_6                                DNX_TMC_NIF_ID_XLGE_6
#define JER2_ARAD_NIF_ID_XLGE_7                                DNX_TMC_NIF_ID_XLGE_7

#define JER2_ARAD_IF_ID_CPU                                    DNX_TMC_IF_ID_CPU
#define JER2_ARAD_IF_ID_OLP                                    DNX_TMC_IF_ID_OLP
#define JER2_ARAD_IF_ID_RCY                                    DNX_TMC_IF_ID_RCY
#define JER2_ARAD_IF_ID_ERP                                    DNX_TMC_IF_ID_ERP
#define JER2_ARAD_IF_ID_OAMP                                   DNX_TMC_IF_ID_OAMP
#define JER2_ARAD_IF_ID_TM_INTERNAL_PKT                        DNX_TMC_IF_ID_TM_INTERNAL_PKT
#define JER2_ARAD_IF_ID_RESERVED                               DNX_TMC_IF_ID_RESERVED
typedef DNX_TMC_INTERFACE_ID                              JER2_ARAD_INTERFACE_ID;

#define JER2_ARAD_FC_DIRECTION_GEN                             DNX_TMC_FC_DIRECTION_GEN
#define JER2_ARAD_FC_DIRECTION_REC                             DNX_TMC_FC_DIRECTION_REC
#define JER2_ARAD_FC_NOF_DIRECTIONS                            DNX_TMC_FC_NOF_DIRECTIONS
typedef DNX_TMC_FC_DIRECTION                                   JER2_ARAD_FC_DIRECTION;

#define JER2_ARAD_COMBO_QRTT_0                                 DNX_TMC_COMBO_QRTT_0
#define JER2_ARAD_COMBO_QRTT_1                                 DNX_TMC_COMBO_QRTT_1
#define JER2_ARAD_COMBO_NOF_QRTTS                              DNX_TMC_COMBO_NOF_QRTTS
typedef DNX_TMC_COMBO_QRTT                                     JER2_ARAD_COMBO_QRTT;

#define JER2_ARAD_DEST_TYPE_QUEUE                              DNX_TMC_DEST_TYPE_QUEUE
#define JER2_ARAD_DEST_TYPE_MULTICAST                          DNX_TMC_DEST_TYPE_MULTICAST
#define JER2_ARAD_DEST_TYPE_SYS_PHY_PORT                       DNX_TMC_DEST_TYPE_SYS_PHY_PORT
#define JER2_ARAD_DEST_TYPE_LAG                                DNX_TMC_DEST_TYPE_LAG
#define JER2_ARAD_DEST_TYPE_ISQ_FLOW_ID                   DNX_TMC_DEST_TYPE_ISQ_FLOW_ID
#define JER2_ARAD_DEST_TYPE_OUT_LIF                           DNX_TMC_DEST_TYPE_OUT_LIF
#define JER2_ARAD_DEST_TYPE_MULTICAST_FLOW_ID       DNX_TMC_DEST_TYPE_MULTICAST_FLOW_ID
#define JER2_ARAD_NOF_DEST_TYPES                               DNX_TMC_NOF_DEST_TYPES_JER2_ARAD
#define JER2_ARAD_DEST_TYPE_FEC_PTR                            DNX_TMC_DEST_TYPE_FEC_PTR
typedef DNX_TMC_DEST_TYPE                                      JER2_ARAD_DEST_TYPE;

#define JER2_ARAD_DEST_SYS_PORT_TYPE_SYS_PHY_PORT              DNX_TMC_DEST_SYS_PORT_TYPE_SYS_PHY_PORT
#define JER2_ARAD_DEST_SYS_PORT_TYPE_LAG                       DNX_TMC_DEST_SYS_PORT_TYPE_LAG
#define JER2_ARAD_DEST_SYS_PORT_NOF_TYPES                      DNX_TMC_DEST_SYS_PORT_NOF_TYPES
typedef DNX_TMC_DEST_SYS_PORT_TYPE                             JER2_ARAD_DEST_SYS_PORT_TYPE;

#define JER2_ARAD_CONNECTION_DIRECTION_RX                      DNX_TMC_CONNECTION_DIRECTION_RX
#define JER2_ARAD_CONNECTION_DIRECTION_TX                      DNX_TMC_CONNECTION_DIRECTION_TX
#define JER2_ARAD_CONNECTION_DIRECTION_BOTH                    DNX_TMC_CONNECTION_DIRECTION_BOTH
#define JER2_ARAD_NOF_CONNECTION_DIRECTIONS                    DNX_TMC_NOF_CONNECTION_DIRECTIONS
typedef DNX_TMC_CONNECTION_DIRECTION                           JER2_ARAD_CONNECTION_DIRECTION;

typedef DNX_TMC_DEST_SYS_PORT_INFO                             JER2_ARAD_DEST_SYS_PORT_INFO;
typedef DNX_TMC_DEST_INFO                                      JER2_ARAD_DEST_INFO;

#define JER2_ARAD_IF_ID_NONE                                   DNX_TMC_IF_ID_NONE
#define JER2_ARAD_NIF_ID_NONE                                  DNX_TMC_NIF_ID_NONE

typedef DNX_TMC_THRESH_WITH_HYST_INFO                          JER2_ARAD_THRESH_WITH_HYST_INFO;

#define JER2_ARAD_NOF_TCGS                                      DNX_TMC_NOF_TCGS                
#define JER2_ARAD_TCG_MIN                                       DNX_TMC_TCG_MIN
#define JER2_ARAD_TCG_MAX                                       DNX_TMC_TCG_MAX                

/* Port with the following number of priorities supports TCG */
#define JER2_ARAD_TCG_NOF_PRIORITIES_SUPPORT                    (DNX_TMC_TCG_NOF_PRIORITIES_SUPPORT)

/* NOF unique TCGS */
#define JER2_ARAD_NOF_TCG_IDS                                   DNX_TMC_NOF_TCG_IDS

/*
 * JER2_ARAD Multicast ID: 0-(16k-1)
 */
typedef DNX_TMC_MULT_ID JER2_ARAD_MULT_ID;

/*
 * JER2_ARAD IPQ Queue type - Traffic Class. Range: 0-7
 */
typedef DNX_TMC_TR_CLS JER2_ARAD_TR_CLS;

/* 
 * JER2_ARAD TCG Traffic class groups. Range: 0-7
 */
typedef DNX_TMC_TCG_NDX JER2_ARAD_TCG_NDX;

/* 
 * Fap Port id.range: 0 - 255.
 */
typedef DNX_TMC_FAP_PORT_ID JER2_ARAD_FAP_PORT_ID;

/* 
 *  Tunnel ID. Range: 0 ... 4K-1.
 */
typedef DNX_TMC_PON_TUNNEL_ID JER2_ARAD_PON_TUNNEL_ID;

typedef enum
{
  JER2_ARAD_OLP_ID       = 0 ,
  JER2_ARAD_IRE_ID       = 1 ,
  JER2_ARAD_IDR_ID       = 2 ,
  JER2_ARAD_IRR_ID       = 3 ,
  JER2_ARAD_IHP_ID       = 4 ,
  JER2_ARAD_QDR_ID       = 5 ,
  JER2_ARAD_IPS_ID       = 6 ,
  JER2_ARAD_IPT_ID       = 7 ,
  JER2_ARAD_DPI_A_ID     = 8 ,
  JER2_ARAD_DPI_B_ID     = 9 ,
  JER2_ARAD_DPI_C_ID     = 10,
  JER2_ARAD_DPI_D_ID     = 11,
  JER2_ARAD_DPI_E_ID     = 12,
  JER2_ARAD_DPI_F_ID     = 13,
  JER2_ARAD_RTP_ID       = 14,
  JER2_ARAD_EGQ_ID       = 15,
  JER2_ARAD_SCH_ID       = 16,
  JER2_ARAD_CFC_ID       = 17,
  JER2_ARAD_EPNI_ID      = 18,
  JER2_ARAD_IQM_ID       = 19,
  JER2_ARAD_MMU_ID       = 20,
  JER2_ARAD_IHB_ID       = 21,
  JER2_ARAD_OAMP_ID      = 22,
  JER2_ARAD_NOF_MODULES  = 23
}JER2_ARAD_MODULE_ID;

/*
 *  The MAC Lane-equivalents for non-network interface identifiers.
 *  Some Network interfaces configuration are per-MAC Lane, and not per-NIF.
 *  These equivalents are used to identify non-network interfaces in this case.
 */
#define JER2_ARAD_MAL_ID_CPU JER2_ARAD_IF_ID_CPU
#define JER2_ARAD_MAL_ID_OLP JER2_ARAD_IF_ID_OLP
#define JER2_ARAD_MAL_ID_RCY JER2_ARAD_IF_ID_RCY
#define JER2_ARAD_MAL_ID_ERP JER2_ARAD_IF_ID_ERP
#define JER2_ARAD_MAL_ID_OAMP JER2_ARAD_IF_ID_OAMP
#define JER2_ARAD_MAL_ID_NONE JER2_ARAD_IF_ID_NONE

#define JER2_ARAD_MAL_TYPE_NONE         DNX_TMC_MAL_TYPE_NONE
#define JER2_ARAD_MAL_TYPE_CPU          DNX_TMC_MAL_TYPE_CPU
#define JER2_ARAD_MAL_TYPE_RCY          DNX_TMC_MAL_TYPE_RCY
#define JER2_ARAD_MAL_TYPE_OLP          DNX_TMC_MAL_TYPE_OLP
#define JER2_ARAD_MAL_TYPE_ERP          DNX_TMC_MAL_TYPE_ERP
#define JER2_ARAD_MAL_TYPE_NIF          DNX_TMC_MAL_TYPE_NIF
#define JER2_ARAD_MAL_TYPE_OAMP         DNX_TMC_MAL_TYPE_OAMP
#define JER2_ARAD_MAL_TYPE_NOF_TYPES    DNX_TMC_MAL_TYPE_NOF_TYPES
typedef DNX_TMC_MAL_EQUIVALENT_TYPE     JER2_ARAD_MAL_EQUIVALENT_TYPE;

/*#define JER2_ARAD_MAX_NIF_ID (JER2_ARAD_IF_NOF_NIFS - 1)*/
#define JER2_ARAD_MAX_IF_ID JER2_ARAD_IF_ID_ERP

typedef enum
{
  /*
   */
  JER2_ARAD_MULTICAST_CLASS_0=0,
  /*
   */
  JER2_ARAD_MULTICAST_CLASS_1=1,
  /*
   */
  JER2_ARAD_MULTICAST_CLASS_2=2,
  /*
   */
  JER2_ARAD_MULTICAST_CLASS_3=3,
  /*
   */
  JER2_ARAD_NOF_MULTICAST_CLASSES=4,
  /*
   *  Must be the last value
   */
  JER2_ARAD_MULTICAST_CLASS_LAST

}JER2_ARAD_MULTICAST_CLASS;


#define JER2_ARAD_COMBO2NIF_GRP_IDX(combo_id) (combo_id)

/* Arad Packet swap mode defines that are DNX_TMC equivalent */
#define JER2_ARAD_SWAP_MODE_4_BYTES          DNX_TMC_SWAP_MODE_4_BYTES
#define JER2_ARAD_SWAP_MODE_8_BYTES          DNX_TMC_SWAP_MODE_8_BYTES
#define JER2_ARAD_SWAP_MODES                 DNX_TMC_SWAP_MODES

typedef DNX_TMC_SWAP_MODE               JER2_ARAD_SWAP_MODE;
typedef DNX_TMC_SWAP_INFO               JER2_ARAD_SWAP_INFO;

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */
/*********************************************************************
* MANE:
*     jer2_arad_fap_port_id_verify
* TYPE:
*   PROC
* DATE:
*   Aug  9 2007
* FUNCTION:
*     Verifies validity of port id
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of device to access (and check for
*     interrupts).
*  DNX_SAND_IN  JER2_ARAD_FAP_PORT_ID  port_id -
*     The index of the FAP port to verify for validity.
*  DNX_SAND_IN  uint8          is_per_port_if_fatp
*     Only relevant if FAT Pipe is enabled.
*     If FAT Pipe enabled, and is_per_port_if_fatp is TRUE, it
*     is valid to call the API for each FAP port that is a member of Fat Pipe
*     (ports 1-4). If FALSE - the API must only be called on Fat Pipe port 1,
*     and calling it for ports 2 - 4 is an error.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_fap_port_id_verify(
    DNX_SAND_IN  int          unit,
    DNX_SAND_IN  JER2_ARAD_FAP_PORT_ID  port_id
  );

uint32
  jer2_arad_drop_precedence_verify(
    DNX_SAND_IN  uint32      dp_ndx
  );

uint32
  jer2_arad_traffic_class_verify(
    DNX_SAND_IN  uint32      tc_ndx
  );

/*****************************************************
* NAME
*    jer2_arad_interface_id_verify
* TYPE:
*   PROC
* DATE:
*   03/12/2007
* FUNCTION:
*   Input Verification for interface index
* INPUT:
*   DNX_SAND_IN  JER2_ARAD_INTERFACE_ID    if_ndx -
*     Interface index.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  jer2_arad_interface_id_verify(
    DNX_SAND_IN  int              unit,
    DNX_SAND_IN  JER2_ARAD_INTERFACE_ID     if_ndx
  );

void
  jer2_arad_JER2_ARAD_DEST_SYS_PORT_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_DEST_SYS_PORT_INFO *info
  );

void
  jer2_arad_JER2_ARAD_DEST_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_DEST_INFO *info
  );

void
  jer2_arad_JER2_ARAD_THRESH_WITH_HYST_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_THRESH_WITH_HYST_INFO *info
  );

/*********************************************************************
* MANE:
*   jer2_arad_JER2_ARAD_SWAP_INFO_clear
* TYPE:
*   PROC
* DATE:
*   Feb 26 2013
* FUNCTION:
*     Resets the JER2_ARAD SWAP information structure JER2_ARAD_SWAP_INFO.
* INPUT:
*   DNX_SAND_OUT  JER2_ARAD_SWAP_INFO  *info -
*       A pointer to the JER2_ARAD SWAP information structure.
* RETURNS:
*   NA.
*********************************************************************/
void
  jer2_arad_JER2_ARAD_SWAP_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_SWAP_INFO *info
  );

#if JER2_ARAD_DEBUG_IS_LVL1
const char*
  jer2_arad_JER2_ARAD_COMBO_QRTT_to_string(
    DNX_SAND_IN JER2_ARAD_COMBO_QRTT enum_val
  );

const char*
  jer2_arad_JER2_ARAD_FAR_DEVICE_TYPE_to_string(
    DNX_SAND_IN JER2_ARAD_FAR_DEVICE_TYPE enum_val
  );

const char*
  jer2_arad_JER2_ARAD_INTERFACE_TYPE_to_string(
    DNX_SAND_IN JER2_ARAD_INTERFACE_TYPE enum_val
  );

const char*
  jer2_arad_JER2_ARAD_INTERFACE_ID_to_string(
    DNX_SAND_IN JER2_ARAD_INTERFACE_ID enum_val
  );

const char*
  jer2_arad_JER2_ARAD_FC_DIRECTION_to_string(
    DNX_SAND_IN JER2_ARAD_FC_DIRECTION enum_val
  );

const char*
  jer2_arad_JER2_ARAD_CONNECTION_DIRECTION_to_string(
    DNX_SAND_IN JER2_ARAD_CONNECTION_DIRECTION enum_val
  );

const char*
  jer2_arad_JER2_ARAD_DEST_TYPE_to_string(
    DNX_SAND_IN JER2_ARAD_DEST_TYPE enum_val,
    DNX_SAND_IN uint8       short_name
  );

const char*
  jer2_arad_JER2_ARAD_DEST_SYS_PORT_TYPE_to_string(
    DNX_SAND_IN JER2_ARAD_DEST_SYS_PORT_TYPE enum_val
  );

void
  jer2_arad_JER2_ARAD_INTERFACE_ID_print(
    DNX_SAND_IN JER2_ARAD_INTERFACE_ID if_ndx
  );

void
  jer2_arad_JER2_ARAD_DEST_SYS_PORT_INFO_print(
    DNX_SAND_IN JER2_ARAD_DEST_SYS_PORT_INFO *info
  );

void
  jer2_arad_JER2_ARAD_DEST_INFO_print(
    DNX_SAND_IN JER2_ARAD_DEST_INFO *info
  );

void
  jer2_arad_JER2_ARAD_THRESH_WITH_HYST_INFO_print(
    DNX_SAND_IN JER2_ARAD_THRESH_WITH_HYST_INFO *info
  );

void
  jer2_arad_JER2_ARAD_DEST_SYS_PORT_INFO_table_format_print(
    DNX_SAND_IN JER2_ARAD_DEST_SYS_PORT_INFO *info
  );

/*********************************************************************
* MANE:
*   jer2_arad_JER2_ARAD_SWAP_MODE_to_string
* TYPE:
*   PROC
* DATE:
*   Feb 26 2013
* FUNCTION:
*     Converts JER2_ARAD SWAP Mode enum value to a string
* INPUT:
*   DNX_SAND_IN  JER2_ARAD_SWAP_MODE  enum_val -
*       JER2_ARAD SWAP Mode enum value
* RETURNS:
*   JER2_ARAD SWAP Mode enum equivalent string or Unknown.
*********************************************************************/
const char*
  jer2_arad_JER2_ARAD_SWAP_MODE_to_string(
     DNX_SAND_IN JER2_ARAD_SWAP_MODE enum_val
  );

/*********************************************************************
* MANE:
*   jer2_arad_JER2_ARAD_SWAP_INFO_print
* TYPE:
*   PROC
* DATE:
*   Feb 26 2013
* FUNCTION:
*     Prints the data in JER2_ARAD_SWAP_INFO.
* INPUT:
*   DNX_SAND_IN  JER2_ARAD_SWAP_INFO  *info -
*       A pointer to the JER2_ARAD SWAP information structure.
* RETURNS:
*   NA.
*********************************************************************/
void
  jer2_arad_JER2_ARAD_SWAP_INFO_print(
    DNX_SAND_IN JER2_ARAD_SWAP_INFO *info
  );

#endif /* JER2_ARAD_DEBUG_IS_LVL1 */

/* } */


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


/* } __JER2_ARAD_API_GENERAL_INCLUDED__*/
#endif


