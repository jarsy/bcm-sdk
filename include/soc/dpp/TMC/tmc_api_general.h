/* $Id: tmc_api_general.h,v 1.33 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_TMC_API_GENERAL_INCLUDED__
/* { */
#define __SOC_TMC_API_GENERAL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Utils/sand_integer_arithmetic.h>

#include <soc/dpp/SAND/SAND_FM/sand_user_callback.h>
#include <soc/dpp/SAND/SAND_FM/sand_chip_defines.h>
#include <shared/error.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* 
 * Maximum number of TM-domains in the system.
 */
#define SOC_TMC_NOF_TM_DOMAIN_IN_SYSTEM     (16)

/* 
 * Maximum number of FAPs in the system.
 */
#define SOC_TMC_NOF_FAPS_IN_SYSTEM     (2048)
#define SOC_TMC_MAX_FAP_ID             (SOC_TMC_NOF_FAPS_IN_SYSTEM-1)
#define SOC_TMC_MAX_DEVICE_ID          (SOC_TMC_NOF_FAPS_IN_SYSTEM-1)

/*
 *	If Egress MC 16K members mode is enabled,
 *  the FAP-IDs range in the system is limited to 0 - 511.
 */

/*
 *  Typically used when the function demands unit parameter,
 *  but the device id is irrelevant in the given context,
 *  and the value is not used in fact.
 */
#define SOC_TMC_DEVICE_ID_IRRELEVANT   SOC_TMC_MAX_DEVICE_ID

/*     Maximal number of physical ports.                       */
#define SOC_TMC_NOF_SYS_PHYS_PORTS      (4096)
#define SOC_TMC_NOF_SYS_PHYS_PORTS_ARAD (32*1024)
#define SOC_TMC_MAX_SYSTEM_PHYSICAL_PORT_ID      (SOC_TMC_NOF_SYS_PHYS_PORTS - 1)      /* 12b */
#define SOC_TMC_MAX_SYSTEM_PHYSICAL_PORT_ID_ARAD (SOC_TMC_NOF_SYS_PHYS_PORTS_ARAD - 1) /* 15b */

/*     Maximal number of physical ports id is indication for Invalid port */
#define SOC_TMC_SYS_PHYS_PORT_INVALID   (SOC_TMC_NOF_SYS_PHYS_PORTS - 1)

/*     Maximal number of logical ports.                        */
#define SOC_TMC_NOF_SYSTEM_PORTS_ARAD         (64*1024)

/* In the destination encoding, mask to see if LAG is set */

/*
 * Maximum number of LBP destination IDs.
 * Range 0:4095
 */

/*
 * Number of queue types in SOC_PETRA - 16.
 */

/*
 * Number of drop precedences.
 */
#define SOC_TMC_NOF_DROP_PRECEDENCE    4

/* Ingress-Packet-traffic-class: Value 0 */
/* Ingress-Packet-traffic-class: Value 7 */

/* Soc_petra number of traffic classes.*/
#define SOC_TMC_NOF_TRAFFIC_CLASSES        8

/* TCG: Number of groups */
#define SOC_TMC_NOF_TCGS               8
 
#define SOC_TMC_TCG_MIN                0
#define SOC_TMC_TCG_MAX                SOC_TMC_NOF_TCGS-1

/* NOF unique TCGS */
#define SOC_TMC_NOF_TCG_IDS            (256)

/* Port with the following number of priorities supports TCG */
#define SOC_TMC_TCG_NOF_PRIORITIES_SUPPORT    (8)

/*
 * Number of FAP-data-ports in SOC_PETRA .
 */
#define SOC_TMC_NOF_FAP_PORTS                  80

/*
 * Number of FAP-data-ports in SOC_PETRA .
 */
#define SOC_TMC_NOF_FAP_PORTS_PETRA            80

/*
 * Number of FAP-data-ports in ARAD.
 */
#define SOC_TMC_NOF_FAP_PORTS_ARAD             256

#define SOC_TMC_NOF_FAP_PORTS_MAX              (SOC_TMC_NOF_FAP_PORTS_ARAD)

/*
 * JERICHO - nof fap ports per core
 */
#define SOC_TMC_NOF_FAP_PORTS_PER_CORE         256
#define SOC_TMC_NOF_FAP_PORTS_JERICHO          (SOC_TMC_NOF_FAP_PORTS_PER_CORE * 2)

/*
 *  Get the number of local ports (256 max in Arad) in longs.
 */
#define SOC_TMC_NOF_FAP_PORTS_MAX_IN_LONGS ((SOC_TMC_NOF_FAP_PORTS_MAX + 31) / 32)

/*     Indication for invalid FAP port.                        */
#define SOC_TMC_FAP_PORT_ID_INVALID            81

/*
 * ERP (Egress Replication Port), is responsible of
 *  replicating packets in the egress (Multicast)
 * SOC_TMC_FAP_EGRESS_REPLICATION_SCH_PORT_ID:
 *  The Port ID When attaching flow / aggregate scheduler to it
 * SOC_TMC_FAP_EGRESS_REPLICATION_IPS_PORT_ID:
 *  The port ID when mapping the system port to it.
 */
#define SOC_TMC_FAP_EGRESS_REPLICATION_SCH_PORT_ID       80
#define SOC_TMC_FAP_EGRESS_REPLICATION_IPS_PORT_ID       255


/*
 *  Maximal Number of NIFS
 */

/*
 * Number of links from the Soc_petra device toward the fabric element.
 */
#define SOC_TMC_FBR_NOF_LINKS               36

/*
 *  The coefficient to convert 1Kilo-bit-per-second to bit-per-second (e.g.).
 */
#define SOC_TMC_RATE_1K                           1000
/*
* Maximal interface rate, in Mega-bits per second.
* This is the upper boundary, it can be lower
*  depending on the credit size
*/               
#define SOC_TMC_SCH_MAX_RATE_MBPS_ARAD(unit)            (SOC_TMC_RATE_1K * SOC_DPP_DEFS_GET(unit, max_gbps_rate_sch))

/*
* Maximal interface rate, in Kilo-bits per second.
* This is the upper boundary, it can be lower
*  depending on the credit size
*/
#define SOC_TMC_SCH_MAX_RATE_KBPS_ARAD(unit)             (SOC_TMC_RATE_1K * SOC_TMC_SCH_MAX_RATE_MBPS_ARAD(unit))

/*
 * Maximal interface rate, in Mega-bits per second.
 * This is the upper boundary, it can be lower
 *  depending on the credit size
 */
#define SOC_TMC_IF_MAX_RATE_MBPS_ARAD(unit)              (SOC_TMC_RATE_1K * SOC_DPP_DEFS_GET(unit, max_gbps_rate_egq)) 
/*
 * Maximal interface rate, in Kilo-bits per second.
 * This is the upper boundary, it can be lower
 *  depending on the credit size
 */
#define SOC_TMC_IF_MAX_RATE_KBPS_ARAD(unit)              (SOC_TMC_RATE_1K * SOC_TMC_IF_MAX_RATE_MBPS_ARAD(unit))

/*
 * Default Values
 * {
 */

/*
 * This queue default as no real meaning,
 * just as a value to put in erase.
 */
/*
 * This FAP id default as no real meaning,
 * just as a value to put in erase.
 */
#define SOC_TMC_DEFAULT_DESTINATION_FAP    (SOC_TMC_NOF_FAPS_IN_SYSTEM-1)
/*
 * This id default as no real meaning,
 * just as a value to put in erase.
 */
/*
 * This id default as no real meaning,
 * just as a value to put in erase.
 */
/*
 * This id default as no real meaning,
 * just as a value to put in erase.
 */

/*
* CPU Default port identifier is 0
*/

/*
 * End Default Values
 * }
 */

/*
 * This is the maximum ingress queue size available in the device.
 * In granularity of bytes.
 */

/* Multicast-ID min: Value 0 */
/* Multicast-ID max: Value 16,383. */
#define SOC_TMC_MULT_NOF_MULTICAST_GROUPS_ARAD  (64*1024)


/*
 * Number of Snoop Commands.
 * There are 15 Snoop Commands altogether 1-15, 0 means disabling Snooping.
 */
/*
* Number of Signatures.
*/
#define SOC_TMC_NOF_SIGNATURE        4
#define SOC_TMC_MAX_SIGNATURE        (SOC_TMC_NOF_SIGNATURE-1)

/*
 * Packet Size (Bytes)
 */

/*
 * Copy-unique-data (e.g. outlif) index
 */

/*     Maximal number of LAG members.                        */
#define SOC_TMC_MAX_NOF_LAG_MEMBER_IN_GROUP              (256)

/*
 *  Rate configuration calendar sets - A and B
 */

/* Ingress TC Mapping Profiles (UC, Flow) */
#define SOC_TMC_NOF_INGRESS_UC_TC_MAPPING_PROFILES    4
#define SOC_TMC_NOF_INGRESS_FLOW_TC_MAPPING_PROFILES  4
/* } */

/*************
 * MACROS    *
 *************/
/* { */
#define SOC_TMC_DEBUG (SOC_SAND_DEBUG)
#define SOC_TMC_DEBUG_IS_LVL1   (SOC_TMC_DEBUG >= SOC_SAND_DBG_LVL1)

#define SOC_TMC_DO_NOTHING_AND_EXIT                       goto exit

/* Data Ports Macros { */
#define SOC_TMC_OLP_PORT_ID                   79


/* Note: ERP is a virtual port, not a data port */
/* Data Ports Macros } */

/* Interface Ports Macros { */








/* Interface Ports Macros } */





/*
 *  TRUE for network interfaces 0, 4, 8... (first interface in each MAL).
 */






/*
 *	Maximal Number of Network Interfaces
 */
#define SOC_TMC_IF_NOF_NIFS 32
/* } */

#define NUM_OF_COUNTERS_CMDS (2)
#define CONVERT_SIGNED_NUM_TO_TWO_COMPLEMENT_METHOD(num,nof_bits) ((num >= 0) ? num : ((~(0-num) + 1)&((1<<nof_bits)-1)))
#define CONVERT_TWO_COMPLEMENT_INTO_SIGNED_NUM(num,nof_bits) ( ((num & (1<<(nof_bits-1))) == 0) ? num : 0-((~(num-1))&((1<<nof_bits)-1)) )


/*************
 * TYPE DEFS *
 *************/
/* { */

/*
 * Soc_petra Multicast ID: 0-(16k-1)
 */
typedef uint32 SOC_TMC_MULT_ID;

/*
 * Soc_petra IPQ Queue type - Traffic Class. Range: 0-7
 */
typedef uint32 SOC_TMC_TR_CLS;

/*
 * Traffic class groups index. Range: 0-7
 */
typedef uint32 SOC_TMC_TCG_NDX;

/* Port id.
 * Soc_petra range: 0 - 79,
 * ARAD range: 0-255.
 */
typedef uint32 SOC_TMC_FAP_PORT_ID;

/* Tunnel id.
 * ARAD range: 0 - 4K-1.
 */
typedef uint32 SOC_TMC_PON_TUNNEL_ID;

typedef enum
{
  SOC_TMC_OLP_ID       = 0 ,
  SOC_TMC_IRE_ID       = 1 ,
  SOC_TMC_IDR_ID       = 2 ,
  SOC_TMC_IRR_ID       = 3 ,
  SOC_TMC_IHP_ID       = 4 ,
  SOC_TMC_QDR_ID       = 5 ,
  SOC_TMC_IPS_ID       = 6 ,
  SOC_TMC_IPT_ID       = 7 ,
  SOC_TMC_DPI_A_ID     = 8 ,
  SOC_TMC_DPI_B_ID     = 9 ,
  SOC_TMC_DPI_C_ID     = 10,
  SOC_TMC_DPI_D_ID     = 11,
  SOC_TMC_DPI_E_ID     = 12,
  SOC_TMC_DPI_F_ID     = 13,
  SOC_TMC_RTP_ID       = 14,
  SOC_TMC_EGQ_ID       = 15,
  SOC_TMC_SCH_ID       = 16,
  SOC_TMC_CFC_ID       = 17,
  SOC_TMC_EPNI_ID      = 18,
  SOC_TMC_IQM_ID       = 19,
  SOC_TMC_MMU_ID       = 20,
  SOC_TMC_NOF_MODULES  = 21
}SOC_TMC_MODULE_ID;

typedef enum
{
  /*
   *  FE1 device: 3b110
   */
  SOC_TMC_FAR_DEVICE_TYPE_FE1=6,
  /*
   *  FE2 device: 3bx11
   */
  SOC_TMC_FAR_DEVICE_TYPE_FE2=3,
  /*
   *  FE3 device: 3b010
   */
  SOC_TMC_FAR_DEVICE_TYPE_FE3=2,
  /*
   *  FAP device: 3bx0x
   */
  SOC_TMC_FAR_DEVICE_TYPE_FAP=0,
  SOC_TMC_FAR_NOF_DEVICE_TYPES = 8

}SOC_TMC_FAR_DEVICE_TYPE;

typedef enum
{
  /*
   *  default value, undefined
   */
  SOC_TMC_IF_TYPE_NONE=0,
  /*
   *  CPU port interface, channelized
   */
  SOC_TMC_IF_TYPE_CPU=1,
  /*
   *  Recycling port interface, channelized
   */
  SOC_TMC_IF_TYPE_RCY=2,
  /*
   *  Offload processor port interface, not channelized
   */
  SOC_TMC_IF_TYPE_OLP=3,
  /*
   *  Egress replication port is not a real interface,
   *  but this interface-equivalent of the ERP is required for
   *  various configuratios.
   */
  SOC_TMC_IF_TYPE_ERP=4,
  /*
   *  Network interface, channelized or non-channelized
   */
  SOC_TMC_IF_TYPE_NIF=5,
  /* 
   * OAM processor port
   */
  SOC_TMC_IF_TYPE_OAMP=6,

  SOC_TMC_IF_NOF_TYPES = 7
}SOC_TMC_INTERFACE_TYPE;

typedef enum
{
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 0 First
   *  interface in MAL 0
   */
  SOC_TMC_IF_ID_0 = 0,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 1
   */
  SOC_TMC_IF_ID_1 = 1,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 2
   */
  SOC_TMC_IF_ID_2 = 2,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 3
   */
  SOC_TMC_IF_ID_3 = 3,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 4 First
   *  interface in MAL 1
   */
  SOC_TMC_IF_ID_4 = 4,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 5
   */
  SOC_TMC_IF_ID_5 = 5,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 6
   */
  SOC_TMC_IF_ID_6 = 6,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 7
   */
  SOC_TMC_IF_ID_7 = 7,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 8 First
   *  interface in MAL 2
   */
  SOC_TMC_IF_ID_8 = 8,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 9
   */
  SOC_TMC_IF_ID_9 = 9,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 10
   */
  SOC_TMC_IF_ID_10 = 10,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 11
   */
  SOC_TMC_IF_ID_11 = 11,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 12 First
   *  interface in MAL 3
   */
  SOC_TMC_IF_ID_12 = 12,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 13
   */
  SOC_TMC_IF_ID_13 = 13,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 14
   */
  SOC_TMC_IF_ID_14 = 14,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 15
   */
  SOC_TMC_IF_ID_15 = 15,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 16 First
   *  interface in MAL 4
   */
  SOC_TMC_IF_ID_16 = 16,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 17
   */
  SOC_TMC_IF_ID_17 = 17,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 18
   */
  SOC_TMC_IF_ID_18 = 18,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 19
   */
  SOC_TMC_IF_ID_19 = 19,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 20 First
   *  interface in MAL 5
   */
  SOC_TMC_IF_ID_20 = 20,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 21
   */
  SOC_TMC_IF_ID_21 = 21,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 22
   */
  SOC_TMC_IF_ID_22 = 22,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 23
   */
  SOC_TMC_IF_ID_23 = 23,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 24 First
   *  interface in MAL 6
   */
  SOC_TMC_IF_ID_24 = 24,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 25
   */
  SOC_TMC_IF_ID_25 = 25,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 26
   */
  SOC_TMC_IF_ID_26 = 26,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 27
   */
  SOC_TMC_IF_ID_27 = 27,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 28 First
   *  interface in MAL 7
   */
  SOC_TMC_IF_ID_28 = 28,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 29
   */
  SOC_TMC_IF_ID_29 = 29,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 30
   */
  SOC_TMC_IF_ID_30 = 30,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 31
   */
  SOC_TMC_IF_ID_31 = 31,
  /*
   *  XAUI NIF Id 0
   */
  SOC_TMC_NIF_ID_XAUI_0 = 1000,
  /*
   *  XAUI NIF Id 1
   */
  SOC_TMC_NIF_ID_XAUI_1 = 1001,
  /*
   *  XAUI NIF Id 2
   */
  SOC_TMC_NIF_ID_XAUI_2 = 1002,
  /*
   *  XAUI NIF Id 3
   */
  SOC_TMC_NIF_ID_XAUI_3 = 1003,
  /*
   *  XAUI NIF Id 4
   */
  SOC_TMC_NIF_ID_XAUI_4 = 1004,
  /*
   *  XAUI NIF Id 5
   */
  SOC_TMC_NIF_ID_XAUI_5 = 1005,
  /*
   *  XAUI NIF Id 6
   */
  SOC_TMC_NIF_ID_XAUI_6 = 1006,
  /*
   *  XAUI NIF Id 7
   */
  SOC_TMC_NIF_ID_XAUI_7 = 1007,
  /*
   *  RXAUI NIF Id 0
   */
  SOC_TMC_NIF_ID_RXAUI_0 = 2000,
  /*
   *  RXAUI NIF Id 1
   */
  SOC_TMC_NIF_ID_RXAUI_1 = 2001,
  /*
   *  RXAUI NIF Id 2
   */
  SOC_TMC_NIF_ID_RXAUI_2 = 2002,
  /*
   *  RXAUI NIF Id 3
   */
  SOC_TMC_NIF_ID_RXAUI_3 = 2003,
  /*
   *  RXAUI NIF Id 4
   */
  SOC_TMC_NIF_ID_RXAUI_4 = 2004,
  /*
   *  RXAUI NIF Id 5
   */
  SOC_TMC_NIF_ID_RXAUI_5 = 2005,
  /*
   *  RXAUI NIF Id 6
   */
  SOC_TMC_NIF_ID_RXAUI_6 = 2006,
  /*
   *  RXAUI NIF Id 7
   */
  SOC_TMC_NIF_ID_RXAUI_7 = 2007,
  /*
   *  RXAUI NIF Id 8
   */
  SOC_TMC_NIF_ID_RXAUI_8 = 2008,
  /*
   *  RXAUI NIF Id 9
   */
  SOC_TMC_NIF_ID_RXAUI_9 = 2009,
  /*
   *  RXAUI NIF Id 10
   */
  SOC_TMC_NIF_ID_RXAUI_10 = 2010,
  /*
   *  RXAUI NIF Id 11
   */
  SOC_TMC_NIF_ID_RXAUI_11 = 2011,
  /*
   *  RXAUI NIF Id 12
   */
  SOC_TMC_NIF_ID_RXAUI_12 = 2012,
  /*
   *  RXAUI NIF Id 13
   */
  SOC_TMC_NIF_ID_RXAUI_13 = 2013,
  /*
   *  RXAUI NIF Id 14
   */
  SOC_TMC_NIF_ID_RXAUI_14 = 2014,
  /*
   *  RXAUI NIF Id 15
   */
  SOC_TMC_NIF_ID_RXAUI_15 = 2015,
  /*
   *  SGMII NIF Id 0
   */
  SOC_TMC_NIF_ID_SGMII_0 = 3000,
  /*
   *  SGMII NIF Id 1
   */
  SOC_TMC_NIF_ID_SGMII_1 = 3001,
  /*
   *  SGMII NIF Id 2
   */
  SOC_TMC_NIF_ID_SGMII_2 = 3002,
  /*
   *  SGMII NIF Id 3
   */
  SOC_TMC_NIF_ID_SGMII_3 = 3003,
  /*
   *  SGMII NIF Id 4
   */
  SOC_TMC_NIF_ID_SGMII_4 = 3004,
  /*
   *  SGMII NIF Id 5
   */
  SOC_TMC_NIF_ID_SGMII_5 = 3005,
  /*
   *  SGMII NIF Id 6
   */
  SOC_TMC_NIF_ID_SGMII_6 = 3006,
  /*
   *  SGMII NIF Id 7
   */
  SOC_TMC_NIF_ID_SGMII_7 = 3007,
  /*
   *  SGMII NIF Id 8
   */
  SOC_TMC_NIF_ID_SGMII_8 = 3008,
  /*
   *  SGMII NIF Id 9
   */
  SOC_TMC_NIF_ID_SGMII_9 = 3009,
  /*
   *  SGMII NIF Id 10
   */
  SOC_TMC_NIF_ID_SGMII_10 = 3010,
  /*
   *  SGMII NIF Id 11
   */
  SOC_TMC_NIF_ID_SGMII_11 = 3011,
  /*
   *  SGMII NIF Id 12
   */
  SOC_TMC_NIF_ID_SGMII_12 = 3012,
  /*
   *  SGMII NIF Id 13
   */
  SOC_TMC_NIF_ID_SGMII_13 = 3013,
  /*
   *  SGMII NIF Id 14
   */
  SOC_TMC_NIF_ID_SGMII_14 = 3014,
  /*
   *  SGMII NIF Id 15
   */
  SOC_TMC_NIF_ID_SGMII_15 = 3015,
  /*
   *  SGMII NIF Id 16
   */
  SOC_TMC_NIF_ID_SGMII_16 = 3016,
  /*
   *  SGMII NIF Id 17
   */
  SOC_TMC_NIF_ID_SGMII_17 = 3017,
  /*
   *  SGMII NIF Id 18
   */
  SOC_TMC_NIF_ID_SGMII_18 = 3018,
  /*
   *  SGMII NIF Id 19
   */
  SOC_TMC_NIF_ID_SGMII_19 = 3019,
  /*
   *  SGMII NIF Id 20
   */
  SOC_TMC_NIF_ID_SGMII_20 = 3020,
  /*
   *  SGMII NIF Id 21
   */
  SOC_TMC_NIF_ID_SGMII_21 = 3021,
  /*
   *  SGMII NIF Id 22
   */
  SOC_TMC_NIF_ID_SGMII_22 = 3022,
  /*
   *  SGMII NIF Id 23
   */
  SOC_TMC_NIF_ID_SGMII_23 = 3023,
  /*
   *  SGMII NIF Id 24
   */
  SOC_TMC_NIF_ID_SGMII_24 = 3024,
  /*
   *  SGMII NIF Id 25
   */
  SOC_TMC_NIF_ID_SGMII_25 = 3025,
  /*
   *  SGMII NIF Id 26
   */
  SOC_TMC_NIF_ID_SGMII_26 = 3026,
  /*
   *  SGMII NIF Id 27
   */
  SOC_TMC_NIF_ID_SGMII_27 = 3027,
  /*
   *  SGMII NIF Id 28
   */
  SOC_TMC_NIF_ID_SGMII_28 = 3028,
  /*
   *  SGMII NIF Id 29
   */
  SOC_TMC_NIF_ID_SGMII_29 = 3029,
  /*
   *  SGMII NIF Id 30
   */
  SOC_TMC_NIF_ID_SGMII_30 = 3030,
  /*
   *  SGMII NIF Id 31
   */
  SOC_TMC_NIF_ID_SGMII_31 = 3031,
  /*
   *  QSGMII NIF Id 0
   */
  SOC_TMC_NIF_ID_QSGMII_0 = 4000,
  /*
   *  QSGMII NIF Id 1
   */
  SOC_TMC_NIF_ID_QSGMII_1 = 4001,
  /*
   *  QSGMII NIF Id 2
   */
  SOC_TMC_NIF_ID_QSGMII_2 = 4002,
  /*
   *  QSGMII NIF Id 3
   */
  SOC_TMC_NIF_ID_QSGMII_3 = 4003,
  /*
   *  QSGMII NIF Id 4
   */
  SOC_TMC_NIF_ID_QSGMII_4 = 4004,
  /*
   *  QSGMII NIF Id 5
   */
  SOC_TMC_NIF_ID_QSGMII_5 = 4005,
  /*
   *  QSGMII NIF Id 6
   */
  SOC_TMC_NIF_ID_QSGMII_6 = 4006,
  /*
   *  QSGMII NIF Id 7
   */
  SOC_TMC_NIF_ID_QSGMII_7 = 4007,
  /*
   *  QSGMII NIF Id 8
   */
  SOC_TMC_NIF_ID_QSGMII_8 = 4008,
  /*
   *  QSGMII NIF Id 9
   */
  SOC_TMC_NIF_ID_QSGMII_9 = 4009,
  /*
   *  QSGMII NIF Id 10
   */
  SOC_TMC_NIF_ID_QSGMII_10 = 4010,
  /*
   *  QSGMII NIF Id 11
   */
  SOC_TMC_NIF_ID_QSGMII_11 = 4011,
  /*
   *  QSGMII NIF Id 12
   */
  SOC_TMC_NIF_ID_QSGMII_12 = 4012,
  /*
   *  QSGMII NIF Id 13
   */
  SOC_TMC_NIF_ID_QSGMII_13 = 4013,
  /*
   *  QSGMII NIF Id 14
   */
  SOC_TMC_NIF_ID_QSGMII_14 = 4014,
  /*
   *  QSGMII NIF Id 15
   */
  SOC_TMC_NIF_ID_QSGMII_15 = 4015,
  /*
   *  QSGMII NIF Id 16
   */
  SOC_TMC_NIF_ID_QSGMII_16 = 4016,
  /*
   *  QSGMII NIF Id 17
   */
  SOC_TMC_NIF_ID_QSGMII_17 = 4017,
  /*
   *  QSGMII NIF Id 18
   */
  SOC_TMC_NIF_ID_QSGMII_18 = 4018,
  /*
   *  QSGMII NIF Id 19
   */
  SOC_TMC_NIF_ID_QSGMII_19 = 4019,
  /*
   *  QSGMII NIF Id 20
   */
  SOC_TMC_NIF_ID_QSGMII_20 = 4020,
  /*
   *  QSGMII NIF Id 21
   */
  SOC_TMC_NIF_ID_QSGMII_21 = 4021,
  /*
   *  QSGMII NIF Id 22
   */
  SOC_TMC_NIF_ID_QSGMII_22 = 4022,
  /*
   *  QSGMII NIF Id 23
   */
  SOC_TMC_NIF_ID_QSGMII_23 = 4023,
  /*
   *  QSGMII NIF Id 24
   */
  SOC_TMC_NIF_ID_QSGMII_24 = 4024,
  /*
   *  QSGMII NIF Id 25
   */
  SOC_TMC_NIF_ID_QSGMII_25 = 4025,
  /*
   *  QSGMII NIF Id 26
   */
  SOC_TMC_NIF_ID_QSGMII_26 = 4026,
  /*
   *  QSGMII NIF Id 27
   */
  SOC_TMC_NIF_ID_QSGMII_27 = 4027,
  /*
   *  QSGMII NIF Id 28
   */
  SOC_TMC_NIF_ID_QSGMII_28 = 4028,
  /*
   *  QSGMII NIF Id 29
   */
  SOC_TMC_NIF_ID_QSGMII_29 = 4029,
  /*
   *  QSGMII NIF Id 30
   */
  SOC_TMC_NIF_ID_QSGMII_30 = 4030,
  /*
   *  QSGMII NIF Id 31
   */
  SOC_TMC_NIF_ID_QSGMII_31 = 4031,
  /*
   *  QSGMII NIF Id 32
   */
  SOC_TMC_NIF_ID_QSGMII_32 = 4032,
  /*
   *  QSGMII NIF Id 33
   */
  SOC_TMC_NIF_ID_QSGMII_33 = 4033,
  /*
   *  QSGMII NIF Id 34
   */
  SOC_TMC_NIF_ID_QSGMII_34 = 4034,
  /*
   *  QSGMII NIF Id 35
   */
  SOC_TMC_NIF_ID_QSGMII_35 = 4035,
  /*
   *  QSGMII NIF Id 36
   */
  SOC_TMC_NIF_ID_QSGMII_36 = 4036,
  /*
   *  QSGMII NIF Id 37
   */
  SOC_TMC_NIF_ID_QSGMII_37 = 4037,
  /*
   *  QSGMII NIF Id 38
   */
  SOC_TMC_NIF_ID_QSGMII_38 = 4038,
  /*
   *  QSGMII NIF Id 39
   */
  SOC_TMC_NIF_ID_QSGMII_39 = 4039,
  /*
   *  QSGMII NIF Id 40
   */
  SOC_TMC_NIF_ID_QSGMII_40 = 4040,
  /*
   *  QSGMII NIF Id 41
   */
  SOC_TMC_NIF_ID_QSGMII_41 = 4041,
  /*
   *  QSGMII NIF Id 42
   */
  SOC_TMC_NIF_ID_QSGMII_42 = 4042,
  /*
   *  QSGMII NIF Id 43
   */
  SOC_TMC_NIF_ID_QSGMII_43 = 4043,
  /*
   *  QSGMII NIF Id 44
   */
  SOC_TMC_NIF_ID_QSGMII_44 = 4044,
  /*
   *  QSGMII NIF Id 45
   */
  SOC_TMC_NIF_ID_QSGMII_45 = 4045,
  /*
   *  QSGMII NIF Id 46
   */
  SOC_TMC_NIF_ID_QSGMII_46 = 4046,
  /*
   *  QSGMII NIF Id 47
   */
  SOC_TMC_NIF_ID_QSGMII_47 = 4047,
  /*
   *  QSGMII NIF Id 48
   */
  SOC_TMC_NIF_ID_QSGMII_48 = 4048,
  /*
   *  QSGMII NIF Id 49
   */
  SOC_TMC_NIF_ID_QSGMII_49 = 4049,
  /*
   *  QSGMII NIF Id 50
   */
  SOC_TMC_NIF_ID_QSGMII_50 = 4050,
  /*
   *  QSGMII NIF Id 51
   */
  SOC_TMC_NIF_ID_QSGMII_51 = 4051,
  /*
   *  QSGMII NIF Id 52
   */
  SOC_TMC_NIF_ID_QSGMII_52 = 4052,
  /*
   *  QSGMII NIF Id 53
   */
  SOC_TMC_NIF_ID_QSGMII_53 = 4053,
  /*
   *  QSGMII NIF Id 54
   */
  SOC_TMC_NIF_ID_QSGMII_54 = 4054,
  /*
   *  QSGMII NIF Id 55
   */
  SOC_TMC_NIF_ID_QSGMII_55 = 4055,
  /*
   *  QSGMII NIF Id 56
   */
  SOC_TMC_NIF_ID_QSGMII_56 = 4056,
  /*
   *  QSGMII NIF Id 57
   */
  SOC_TMC_NIF_ID_QSGMII_57 = 4057,
  /*
   *  QSGMII NIF Id 58
   */
  SOC_TMC_NIF_ID_QSGMII_58 = 4058,
  /*
   *  QSGMII NIF Id 59
   */
  SOC_TMC_NIF_ID_QSGMII_59 = 4059,
  /*
   *  QSGMII NIF Id 60
   */
  SOC_TMC_NIF_ID_QSGMII_60 = 4060,
  /*
   *  QSGMII NIF Id 61
   */
  SOC_TMC_NIF_ID_QSGMII_61 = 4061,
  /*
   *  QSGMII NIF Id 62
   */
  SOC_TMC_NIF_ID_QSGMII_62 = 4062,
  /*
   *  QSGMII NIF Id 63
   */
  SOC_TMC_NIF_ID_QSGMII_63 = 4063,
  /*
   *  Interlaken NIF Id 0
   */
  SOC_TMC_NIF_ID_ILKN_0 = 5000,
  /*
   *  Interlaken NIF Id 1
   */
  SOC_TMC_NIF_ID_ILKN_1 = 5001,
  /*
   *  Interlaken NIF Id 0
   */
  SOC_TMC_NIF_ID_ILKN_TDM_0 = 5002,
  /*
   *  Interlaken NIF Id 1
   */
  SOC_TMC_NIF_ID_ILKN_TDM_1 = 5003,
  /*
   *  100G CGE NIF Id 0
   */
  SOC_TMC_NIF_ID_CGE_0 = 7000,
  /*
   *  100G CGE NIF Id 1
   */
  SOC_TMC_NIF_ID_CGE_1 = 7001,
  /*
   *  40G XLGE NIF Id 0
   */
  SOC_TMC_NIF_ID_XLGE_0 = 8000,
  /*
   *  40G XLGE NIF Id 1
   */
  SOC_TMC_NIF_ID_XLGE_1 = 8001,
  /*
   *  40G XLGE NIF Id 2
   */
  SOC_TMC_NIF_ID_XLGE_2 = 8002,
  /*
   *  40G XLGE NIF Id 3
   */
  SOC_TMC_NIF_ID_XLGE_3 = 8003,
  /*
   *  40G XLGE NIF Id 4
   */
  SOC_TMC_NIF_ID_XLGE_4 = 8004,
  /*
   *  40G XLGE NIF Id 5
   */
  SOC_TMC_NIF_ID_XLGE_5 = 8005,
  /*
   *  40G XLGE NIF Id 6
   */
  SOC_TMC_NIF_ID_XLGE_6 = 8006,
  /*
   *  40G XLGE NIF Id 7
   */
  SOC_TMC_NIF_ID_XLGE_7 = 8007,
  /*
   *  10G Base-R NIF Id 0
   */
  SOC_TMC_NIF_ID_10GBASE_R_0 = 9000,
  /*
   *  10G Base-R NIF Id 1
   */
  SOC_TMC_NIF_ID_10GBASE_R_1 = 9001,
  /*
   *  10G Base-R NIF Id 2
   */
  SOC_TMC_NIF_ID_10GBASE_R_2 = 9002,
  /*
   *  10G Base-R NIF Id 3
   */
  SOC_TMC_NIF_ID_10GBASE_R_3 = 9003,
  /*
   *  10G Base-R NIF Id 4
   */
  SOC_TMC_NIF_ID_10GBASE_R_4 = 9004,
  /*
   *  10G Base-R NIF Id 5
   */
  SOC_TMC_NIF_ID_10GBASE_R_5 = 9005,
  /*
   *  10G Base-R NIF Id 6
   */
  SOC_TMC_NIF_ID_10GBASE_R_6 = 9006,
  /*
   *  10G Base-R NIF Id 7
   */
  SOC_TMC_NIF_ID_10GBASE_R_7 = 9007,
  /*
   *  10G Base-R NIF Id 8
   */
  SOC_TMC_NIF_ID_10GBASE_R_8 = 9008,
  /*
   *  10G Base-R NIF Id 9
   */
  SOC_TMC_NIF_ID_10GBASE_R_9 = 9009,
  /*
   *  10G Base-R NIF Id 10
   */
  SOC_TMC_NIF_ID_10GBASE_R_10 = 9010,
  /*
   *  10G Base-R NIF Id 11
   */
  SOC_TMC_NIF_ID_10GBASE_R_11 = 9011,
  /*
   *  10G Base-R NIF Id 12
   */
  SOC_TMC_NIF_ID_10GBASE_R_12 = 9012,
  /*
   *  10G Base-R NIF Id 13
   */
  SOC_TMC_NIF_ID_10GBASE_R_13 = 9013,
  /*
   *  10G Base-R NIF Id 14
   */
  SOC_TMC_NIF_ID_10GBASE_R_14 = 9014,
  /*
   *  10G Base-R NIF Id 15
   */
  SOC_TMC_NIF_ID_10GBASE_R_15 = 9015,
  /*
   *  10G Base-R NIF Id 16
   */
  SOC_TMC_NIF_ID_10GBASE_R_16 = 9016,
  /*
   *  10G Base-R NIF Id 17
   */
  SOC_TMC_NIF_ID_10GBASE_R_17 = 9017,
  /*
   *  10G Base-R NIF Id 18
   */
  SOC_TMC_NIF_ID_10GBASE_R_18 = 9018,
  /*
   *  10G Base-R NIF Id 19
   */
  SOC_TMC_NIF_ID_10GBASE_R_19 = 9019,
  /*
   *  10G Base-R NIF Id 20
   */
  SOC_TMC_NIF_ID_10GBASE_R_20 = 9020,
  /*
   *  10G Base-R NIF Id 21
   */
  SOC_TMC_NIF_ID_10GBASE_R_21 = 9021,
  /*
   *  10G Base-R NIF Id 22
   */
  SOC_TMC_NIF_ID_10GBASE_R_22 = 9022,
  /*
   *  10G Base-R NIF Id 23
   */
  SOC_TMC_NIF_ID_10GBASE_R_23 = 9023,
  /*
   *  10G Base-R NIF Id 24
   */
  SOC_TMC_NIF_ID_10GBASE_R_24 = 9024,
  /*
   *  10G Base-R NIF Id 25
   */
  SOC_TMC_NIF_ID_10GBASE_R_25 = 9025,
  /*
   *  10G Base-R NIF Id 26
   */
  SOC_TMC_NIF_ID_10GBASE_R_26 = 9026,
  /*
   *  10G Base-R NIF Id 27
   */
  SOC_TMC_NIF_ID_10GBASE_R_27 = 9027,
  /*
   *  10G Base-R NIF Id 28
   */
  SOC_TMC_NIF_ID_10GBASE_R_28 = 9028,
  /*
   *  10G Base-R NIF Id 29
   */
  SOC_TMC_NIF_ID_10GBASE_R_29 = 9029,
  /*
   *  10G Base-R NIF Id 30
   */
  SOC_TMC_NIF_ID_10GBASE_R_30 = 9030,
  /*
   *  10G Base-R NIF Id 31
   */
  SOC_TMC_NIF_ID_10GBASE_R_31 = 9031,
  /*
   *  CPU port interface
   */
  SOC_TMC_IF_ID_CPU = 200,
  /*
   *  OLP port interface
   */
  SOC_TMC_IF_ID_OLP = 300,
  /*
   *  Recycling port interface
   */
  SOC_TMC_IF_ID_RCY = 400,
  /*
   *  Egress Replication Port (Device Virtual Port)
   */
  SOC_TMC_IF_ID_ERP = 500,
  /*
   *
   */
  SOC_TMC_IF_ID_OAMP = 600,
  /*
   * Synchronous Packet interfaces between ATMF and Spider NPU .
   */
  SOC_TMC_IF_ID_TM_INTERNAL_PKT = 700,
  /*
   * Mirror enabled port(logical port and tm port) .
   */
  SOC_TMC_IF_ID_RESERVED = 800,
  /*
   *  Invalid Interface
   */
   SOC_TMC_IF_ID_NONE = 65535
}SOC_TMC_INTERFACE_ID;

#define SOC_TMC_NIF_ID_NONE SOC_TMC_IF_ID_NONE

/*
 *  The MAC Lane-equivalents for non-network interface identifiers.
 *  Some Network interfaces configuration are per-MAC Lane, and not per-NIF.
 *  These equivalents are used to identify non-network interfaces in this case.
 */

typedef enum
{
  /*
   *  default value, undefined
   */
  SOC_TMC_MAL_TYPE_NONE=0,
  /*
   *  MAL-equivalent of CPU interface
   */
  SOC_TMC_MAL_TYPE_CPU=1,
  /*
   *  MAL-equivalent of Recycling interface
   */
  SOC_TMC_MAL_TYPE_RCY=2,
  /*
   *  MAL-equivalent of OLP interface
   */
  SOC_TMC_MAL_TYPE_OLP=3,
  /*
   *  MAL-equivalent of OLP interface
   */
  SOC_TMC_MAL_TYPE_ERP=4,
  /*
   *  MAL-equivalent of network interface
   */
  SOC_TMC_MAL_TYPE_NIF=5,

  SOC_TMC_MAL_TYPE_NOF_TYPES=6
}SOC_TMC_MAL_EQUIVALENT_TYPE;

typedef enum
{
  /*
  *  Flow Control direction  generation.
  */
  SOC_TMC_FC_DIRECTION_GEN=0,
  /*
  *  Flow Control direction  reception.
  */
  SOC_TMC_FC_DIRECTION_REC=1,
  /*
  *  Total number of Flow Control directions.
  */
  SOC_TMC_FC_NOF_DIRECTIONS=2
}SOC_TMC_FC_DIRECTION;




typedef enum
{
  /*
  *  Destination type: Incoming Queue. Matching Index Range: 0
  *   32K-1
  */
  SOC_TMC_DEST_TYPE_QUEUE=0,
  /*
  *  Destination type: Multicast Group. Matching Index Range:
  *  0  16K-1
  */
  SOC_TMC_DEST_TYPE_MULTICAST=1,
  /*
  *  System Physical FAP Port. Matching Index Range: 0  4095.
  */
  SOC_TMC_DEST_TYPE_SYS_PHY_PORT=2,
  /*
  *  System LAG Id. Matching Index Range: 0  255.
  */
  SOC_TMC_DEST_TYPE_LAG=3,
  /*
  *  Total number of destination types.
  */
  SOC_TMC_NOF_DEST_TYPES_PETRA=4,
  /*
  *  Ingress Shaping Queue flow id
  */
  SOC_TMC_DEST_TYPE_ISQ_FLOW_ID=4,
  /*
  *  OutLif
  */
  SOC_TMC_DEST_TYPE_OUT_LIF=5,
  /*
  *  Multicast Flow ID.
  */
  SOC_TMC_DEST_TYPE_MULTICAST_FLOW_ID=6,
  /*
  *  Total number of destination types.
  */
  SOC_TMC_NOF_DEST_TYPES_ARAD=7,
  /*
    *  FEC pointer id
  */
  SOC_TMC_DEST_TYPE_FEC_PTR = SOC_TMC_NOF_DEST_TYPES_ARAD,
  /*
  *  Total number of destination types.
  */
  SOC_TMC_NOF_DEST_TYPES_JER=8

}SOC_TMC_DEST_TYPE;

typedef enum
{
  /*
  *  System Physical FAP Port. Matching Index Range: 0  4095.
  */
  SOC_TMC_DEST_SYS_PORT_TYPE_SYS_PHY_PORT=0,
  /*
  *  System LAG Id. Matching Index Range: 0  255.
  */
  SOC_TMC_DEST_SYS_PORT_TYPE_LAG=1,
  /*
  *  Total number of system-ports types.
  */
  SOC_TMC_DEST_SYS_PORT_NOF_TYPES=2
}SOC_TMC_DEST_SYS_PORT_TYPE;

typedef enum
{
    SOC_TMC_CMD_TYPE_MIRROR = 0,
    SOC_TMC_CMD_TYPE_SNOOP = 1
}SOC_TMC_CMD_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
    *  LAG id or system-physical-port id.
    */
  SOC_TMC_DEST_SYS_PORT_TYPE type;
 /*
  *  According to the System-Port type, one of the
  *  following:LAG id. Range: 0  255. System Physical Port.
  *  Range: 0  4K-1.
  */
  uint32 id;
  /*
   * In case of LAG, the Member-ID.
   * Range: 0 - 15.
   */
  uint32 member_id;
}SOC_TMC_DEST_SYS_PORT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Queue, multicast group or system port / LAG id (Unicast
   *  flow, System multicast, Unicast Port accordingly).
   */
  SOC_TMC_DEST_TYPE type;
  /*
   *  According to destination type, one of the
   *  following:Queue id (Unicast flow), Range: 0 -
   *  32K-1. Multicast id (System Multicast), Range: 0 -
   *  16K-1. System Physical Port, Range: 0  4K-1. LAG id., Range:
   *  0-255.
   */
  uint32 id;
}SOC_TMC_DEST_INFO;

typedef enum
{
  /*
   */
  SOC_TMC_MULTICAST_CLASS_0=0,
  /*
   */
  SOC_TMC_MULTICAST_CLASS_1=1,
  /*
   */
  SOC_TMC_MULTICAST_CLASS_2=2,
  /*
   */
  SOC_TMC_MULTICAST_CLASS_3=3,
  /*
   */
  SOC_TMC_NOF_MULTICAST_CLASSES=4,
  /*
   *  Must be the last value
   */
  SOC_TMC_MULTICAST_CLASS_LAST

}SOC_TMC_MULTICAST_CLASS;

typedef enum
{
  /*
  *  Connection direction  receive (RX).
  */
  SOC_TMC_CONNECTION_DIRECTION_RX=0,
  /*
  *  Connection direction  transmit (TX).
  */
  SOC_TMC_CONNECTION_DIRECTION_TX=1,
  /*
  *  Connection direction  both receive and transmit
  *  (RX/TX).
  */
  SOC_TMC_CONNECTION_DIRECTION_BOTH=2,
  /*
  *  Total number of connection direction configurations.
  */
  SOC_TMC_NOF_CONNECTION_DIRECTIONS=3
}SOC_TMC_CONNECTION_DIRECTION;

typedef enum
{
  /*
   *  packet is bridged according to outer most LL header
   */
  SOC_TMC_PKT_FRWRD_TYPE_BRIDGE = 0,
  /*
   *  packet is IPv4 UC routed
   */
  SOC_TMC_PKT_FRWRD_TYPE_IPV4_UC = 1,
  /*
   *  packet is IPv4 MC routed
   */
  SOC_TMC_PKT_FRWRD_TYPE_IPV4_MC = 2,
  /*
   *  packet is IPv6 UC routed
   */
  SOC_TMC_PKT_FRWRD_TYPE_IPV6_UC = 3,
  /*
   *  packet is IPv6 MC routed
   */
  SOC_TMC_PKT_FRWRD_TYPE_IPV6_MC = 4,
  /*
   *  packet is forwarded according to MPLS label,
   *  Ingress-label-mapping
   */
  SOC_TMC_PKT_FRWRD_TYPE_MPLS = 5,
  /*
   *  packet is forwarded according to TRILL
   */
  SOC_TMC_PKT_FRWRD_TYPE_TRILL = 6,
  /*
   *  packet is trapped
   */
  SOC_TMC_PKT_FRWRD_TYPE_CPU_TRAP = 7,
  /*
   *  packet is bridged according to inner LL header, outer LL
   *  header is terminated. For example in VPLS application.
   */
  SOC_TMC_PKT_FRWRD_TYPE_BRIDGE_AFTER_TERM = 8,
  /*
   *  Custom1 - used in Arad for FCoE for FCF
   */
  SOC_TMC_PKT_FRWRD_TYPE_CUSTOM1 = 9,
  /*
   *  Packet is snooped.
   */
   SOC_TMC_PKT_FRWRD_TYPE_SNOOP = 0xB,
  /*
   *  Packet comes for a TM processing.
   */
   SOC_TMC_PKT_FRWRD_TYPE_TM = 0xE,
  /*
   *  Number of types in SOC_TMC_PKT_FRWRD_TYPE
   */
  SOC_TMC_NOF_PKT_FRWRD_TYPES = 10
}SOC_TMC_PKT_FRWRD_TYPE;

typedef enum
{
  /*
  *  No header is terminated.
  */
  SOC_TMC_TUNNEL_TERM_CODE_NONE = 0,
  /*
  *  The Ethernet header is terminated.
  */
  SOC_TMC_TUNNEL_TERM_CODE_ETH = 1,
  /*
  *  The IPv4 and Ethernet headers are terminated.
  */
  SOC_TMC_TUNNEL_TERM_CODE_IPV4_ETH = 2,
  /*
  *  The MPLS and Ethernet headers are terminated.
  */
  SOC_TMC_TUNNEL_TERM_CODE_MPLS_ETH = 3,
  /*
  *  The MPLS and Ethernet headers are terminated with
  *  Control Word.
  */
  SOC_TMC_TUNNEL_TERM_CODE_MPLS_ETH_CW = 4,
  /*
  *  The 2 MPLS and Ethernet headers are terminated.
  */
  SOC_TMC_TUNNEL_TERM_CODE_MPLS2_ETH = 5,
  /*
  *  The 2 MPLS and Ethernet headers are terminated with
  *  Control Word.
  */
  SOC_TMC_TUNNEL_TERM_CODE_MPLS2_ETH_CW = 6,
  /*
  *  The 3 MPLS and Ethernet headers are terminated.
  */
  SOC_TMC_TUNNEL_TERM_CODE_MPLS3_ETH = 7,
  /*
  *  The 3 MPLS and Ethernet headers are terminated with
  *  Control Word.
  */
  SOC_TMC_TUNNEL_TERM_CODE_MPLS3_ETH_CW = 8,
  /*
  *  The Trill and Ethernet headers are terminated.
  */
  SOC_TMC_TUNNEL_TERM_CODE_TRILL_ETH = 9,
  /*
  *  Number of types in SOC_TMC_TUNNEL_TERM_CODE
  */
  SOC_TMC_NOF_TUNNEL_TERM_CODES = 10
}SOC_TMC_TUNNEL_TERM_CODE;

typedef enum
{
  /*
   *  Push new label. The payload of the command is a push
   *  profile to build the TTL and EXP of the pushed label.
   */
  SOC_TMC_MPLS_COMMAND_TYPE_PUSH = 0,
  /*
   *  Pop MPLS header, next header, processing type
   *  and CW passed by other attributes
   *  Arad only.
   *  cannot be used for Soc_petra-B.
   */
  SOC_TMC_MPLS_COMMAND_TYPE_POP = 1,
  /*
   *  Pop into MPLS. TTL and EXP of the exposed label not
   *  affected by the popped label. May perform mapping of the
   *  EXP field.
   */
  SOC_TMC_MPLS_COMMAND_TYPE_POP_INTO_MPLS_PIPE = 8,
  /*
   *  Pop into MPLS and inherent TTL and EXP from the popped
   *  label with optional mapping of the EXP field.
   */
  SOC_TMC_MPLS_COMMAND_TYPE_POP_INTO_MPLS_UNIFORM = 9,
  /*
   *  Pop into MPLS. TTL and TOS of the exposed IPv4 header
   *  not affected by the popped label. May perform mapping of
   *  the TOS field.
   */
  SOC_TMC_MPLS_COMMAND_TYPE_POP_INTO_IPV4_PIPE = 10,
  /*
   *  Pop into IPv4 and inherent TTL and EXP from the popped
   *  label with mapping of the EXP field to TOS.
   */
  SOC_TMC_MPLS_COMMAND_TYPE_POP_INTO_IPV4_UNIFORM = 11,
  /*
   *  Pop into MPLS. TTL and TC of the exposed IPv6 header not
   *  affected by the popped label. May perform mapping of the
   *  TC field.
   */
  SOC_TMC_MPLS_COMMAND_TYPE_POP_INTO_IPV6_PIPE = 12,
  /*
   *  Pop into IPv4 and inherent TTL and EXP from the popped
   *  label with mapping of the EXP field to TC.
   */
  SOC_TMC_MPLS_COMMAND_TYPE_POP_INTO_IPV6_UNIFORM = 13,
  /*
   *  Pop into Ethernet. For Drop and Continue applications.
   */
  SOC_TMC_MPLS_COMMAND_TYPE_POP_INTO_ETHERNET = 14,
  /*
   *  Swap the label of the MPLS header.
   */
  SOC_TMC_MPLS_COMMAND_TYPE_SWAP = 15,
  /*
   *  Number of types in SOC_TMC_MPLS_COMMAND_TYPE
   */
  SOC_TMC_NOF_MPLS_COMMAND_TYPES = 16
}SOC_TMC_MPLS_COMMAND_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Passing this threshold from above means activation.
   */
  uint32 set;
  /*
   *  Passing this threshold from below means deactivation.
   */
  uint32 clear;
}SOC_TMC_THRESH_WITH_HYST_INFO;

typedef struct
{
  /* Maximum FADT threshold */
  uint32 max_threshold;
  /* Minimum FADT threshold */
  uint32 min_threshold;
  /* 
   * If AdjustFactor3 is set,  
   *    Dynamic-Max-Th = Free-Resource << AdjustFactor2:0 
   * Otherwise, 
   *    Dynamic-Max-Th = Free-Resource >> AdjustFactor2:0
   */
  int   alpha;
} SOC_TMC_FADT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Passing this threshold from above means activation.
   *  The threshold is dynamic (FADT).
   */
  SOC_TMC_FADT_INFO set;
  /*
   *  Passing this threshold from below means deactivation.
   *  The threshold is an offset below the FADT set threshold.
   */
  uint32 clear_offset;
} SOC_TMC_THRESH_WITH_FADT_HYST_INFO;

typedef enum
{
  /*
   *  Combo Quartet 0 - the SerDes combo quartet in STAR 0
   */
  SOC_TMC_COMBO_QRTT_0=0,
  /*
   *  Combo Quartet 1 - the SerDes combo quartet in STAR 1
   */
  SOC_TMC_COMBO_QRTT_1=1,
  /*
   *  Total number of SerDes combo quartets
   */
  SOC_TMC_COMBO_NOF_QRTTS=2
}SOC_TMC_COMBO_QRTT;


typedef enum
{
  /*
   *  SerDes is powered down.
   */
  SOC_TMC_SRD_POWER_STATE_DOWN=0,
  /*
   *  SerDes is powered up.
   */
  SOC_TMC_SRD_POWER_STATE_UP=1,
  /*
   *  SerDes is powered up. When setting this state, the
   *  SerDes is validated after power-up. If needed. a re-lock
   *  sequence is performed to verify SerDes is active. Note:
   *  this is the recommended value for powering-up the
   *  SerDes.
   */
  SOC_TMC_SRD_POWER_STATE_UP_AND_RELOCK=2,
  /*
   *  Total Number of SerDes power states.
   */
  SOC_TMC_SRD_NOF_POWER_STATES=3
}SOC_TMC_SRD_POWER_STATE;

typedef _shr_error_t SOC_TMC_ERROR;


typedef enum
{
  /*
   * SWAP operation 4 bytes after DA to the start of the packet.
   * Applicable for PON Channel 
   */
  SOC_TMC_SWAP_MODE_4_BYTES = 0,
  /*
   * SWAP operation 8 bytes after DA to the start of the packet. 
   * Applicable for E-TAG 
   */
  SOC_TMC_SWAP_MODE_8_BYTES,
  /*
   * Total number of arad init swap modes
   */
  SOC_TMC_SWAP_MODES
} SOC_TMC_SWAP_MODE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Device swap mode for incoming packets
   */
  SOC_TMC_SWAP_MODE mode;
  /*
   *  Device swap offset to the Prepend Tag. Range: 0-15.
   *  The Prepend Tag is located at (12 + 2*offset) bytes from the start
   *  of the packet. HW Default: 0.
   */
  uint32 offset;
} SOC_TMC_SWAP_INFO;

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

void
  SOC_TMC_DEST_SYS_PORT_INFO_clear(
    SOC_SAND_OUT SOC_TMC_DEST_SYS_PORT_INFO *info
  );

void
  SOC_TMC_DEST_INFO_clear(
    SOC_SAND_OUT SOC_TMC_DEST_INFO *info
  );

void
  SOC_TMC_THRESH_WITH_HYST_INFO_clear(
    SOC_SAND_OUT SOC_TMC_THRESH_WITH_HYST_INFO *info
  );

void
  SOC_TMC_THRESH_WITH_FADT_HYST_INFO_clear(
    SOC_SAND_OUT SOC_TMC_THRESH_WITH_FADT_HYST_INFO *info
  );

void
  SOC_TMC_SWAP_INFO_clear(
    SOC_SAND_OUT SOC_TMC_SWAP_INFO *info
  );

#if SOC_TMC_DEBUG_IS_LVL1

const char*
  SOC_TMC_COMBO_QRTT_to_string(
    SOC_SAND_IN SOC_TMC_COMBO_QRTT enum_val
  );

const char*
  SOC_TMC_FAR_DEVICE_TYPE_to_string(
    SOC_SAND_IN SOC_TMC_FAR_DEVICE_TYPE enum_val
  );

const char*
  SOC_TMC_INTERFACE_TYPE_to_string(
    SOC_SAND_IN SOC_TMC_INTERFACE_TYPE enum_val
  );

void
  SOC_TMC_INTERFACE_ID_print(
    SOC_SAND_IN SOC_TMC_INTERFACE_ID if_ndx
  );

const char*
  SOC_TMC_INTERFACE_ID_to_string(
    SOC_SAND_IN SOC_TMC_INTERFACE_ID enum_val
  );

const char*
  SOC_TMC_FC_DIRECTION_to_string(
    SOC_SAND_IN SOC_TMC_FC_DIRECTION enum_val
  );

const char*
  SOC_TMC_CONNECTION_DIRECTION_to_string(
    SOC_SAND_IN SOC_TMC_CONNECTION_DIRECTION enum_val
  );

const char*
  SOC_TMC_DEST_TYPE_to_string(
    SOC_SAND_IN SOC_TMC_DEST_TYPE enum_val,
    SOC_SAND_IN uint8       short_name
  );

const char*
  SOC_TMC_DEST_SYS_PORT_TYPE_to_string(
    SOC_SAND_IN SOC_TMC_DEST_SYS_PORT_TYPE enum_val
    );

const char*
  SOC_TMC_PKT_FRWRD_TYPE_to_string(
    SOC_SAND_IN  SOC_TMC_PKT_FRWRD_TYPE enum_val
  );

const char*
  SOC_TMC_TUNNEL_TERM_CODE_to_string(
    SOC_SAND_IN  SOC_TMC_TUNNEL_TERM_CODE enum_val
  );


void
  SOC_TMC_DEST_SYS_PORT_INFO_print(
    SOC_SAND_IN SOC_TMC_DEST_SYS_PORT_INFO *info
  );

void
  SOC_TMC_DEST_INFO_print(
    SOC_SAND_IN SOC_TMC_DEST_INFO *info
  );

void
  SOC_TMC_THRESH_WITH_HYST_INFO_print(
    SOC_SAND_IN SOC_TMC_THRESH_WITH_HYST_INFO *info
  );

void
  SOC_TMC_DEST_SYS_PORT_INFO_table_format_print(
    SOC_SAND_IN SOC_TMC_DEST_SYS_PORT_INFO *info
  );

const char*
  SOC_TMC_SWAP_MODE_to_string(
    SOC_SAND_IN  SOC_TMC_SWAP_MODE enum_val
  );

void
  SOC_TMC_SWAP_INFO_print(
    SOC_SAND_IN SOC_TMC_SWAP_INFO *info
  );

#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_GENERAL_INCLUDED__*/
#endif
