/* $Id: jer2_jer2_jer2_tmc_api_general.h,v 1.33 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __DNX_TMC_API_GENERAL_INCLUDED__
/* { */
#define __DNX_TMC_API_GENERAL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>

#include <soc/dnx/legacy/SAND/SAND_FM/sand_user_callback.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_chip_defines.h>
#include <shared/error.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* 
 * Maximum number of TM-domains in the system.
 */
#define DNX_TMC_NOF_TM_DOMAIN_IN_SYSTEM     (16)

/* 
 * Maximum number of FAPs in the system.
 */
#define DNX_TMC_NOF_FAPS_IN_SYSTEM     (2048)
#define DNX_TMC_MAX_FAP_ID             (DNX_TMC_NOF_FAPS_IN_SYSTEM-1)
#define DNX_TMC_MAX_DEVICE_ID          (DNX_TMC_NOF_FAPS_IN_SYSTEM-1)

/*
 *	If Egress MC 16K members mode is enabled,
 *  the FAP-IDs range in the system is limited to 0 - 511.
 */

/*
 *  Typically used when the function demands unit parameter,
 *  but the device id is irrelevant in the given context,
 *  and the value is not used in fact.
 */
#define DNX_TMC_DEVICE_ID_IRRELEVANT   DNX_TMC_MAX_DEVICE_ID

/*     Maximal number of physical ports.                       */
#define DNX_TMC_NOF_SYS_PHYS_PORTS      (4096)
#define DNX_TMC_NOF_SYS_PHYS_PORTS_JER2_ARAD (32*1024)
#define DNX_TMC_MAX_SYSTEM_PHYSICAL_PORT_ID      (DNX_TMC_NOF_SYS_PHYS_PORTS - 1)      /* 12b */
#define DNX_TMC_MAX_SYSTEM_PHYSICAL_PORT_ID_JER2_ARAD (DNX_TMC_NOF_SYS_PHYS_PORTS_JER2_ARAD - 1) /* 15b */

/*     Maximal number of physical ports id is indication for Invalid port */
#define DNX_TMC_SYS_PHYS_PORT_INVALID   (DNX_TMC_NOF_SYS_PHYS_PORTS - 1)

/*     Maximal number of logical ports.                        */
#define DNX_TMC_NOF_SYSTEM_PORTS_JER2_ARAD         (64*1024)

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
#define DNX_TMC_NOF_DROP_PRECEDENCE    4

/* Ingress-Packet-traffic-class: Value 0 */
/* Ingress-Packet-traffic-class: Value 7 */

/* Soc_petra number of traffic classes.*/
#define DNX_TMC_NOF_TRAFFIC_CLASSES        8

/* TCG: Number of groups */
#define DNX_TMC_NOF_TCGS               8
 
#define DNX_TMC_TCG_MIN                0
#define DNX_TMC_TCG_MAX                DNX_TMC_NOF_TCGS-1

/* NOF unique TCGS */
#define DNX_TMC_NOF_TCG_IDS            (256)

/* Port with the following number of priorities supports TCG */
#define DNX_TMC_TCG_NOF_PRIORITIES_SUPPORT    (8)

/*
 * Number of FAP-data-ports in SOC_PETRA .
 */
#define DNX_TMC_NOF_FAP_PORTS                  80

/*
 * Number of FAP-data-ports in SOC_PETRA .
 */
#define DNX_TMC_NOF_FAP_PORTS_PETRA            80

/*
 * Number of FAP-data-ports in JER2_ARAD.
 */
#define DNX_TMC_NOF_FAP_PORTS_JER2_ARAD             256

#define DNX_TMC_NOF_FAP_PORTS_MAX              (DNX_TMC_NOF_FAP_PORTS_JER2_ARAD)

/*
 * JER2_JERICHO - nof fap ports per core
 */
#define DNX_TMC_NOF_FAP_PORTS_PER_CORE         256
#define DNX_TMC_NOF_FAP_PORTS_JER2_JERICHO          (DNX_TMC_NOF_FAP_PORTS_PER_CORE * 2)

/*
 *  Get the number of local ports (256 max in Arad) in longs.
 */
#define DNX_TMC_NOF_FAP_PORTS_MAX_IN_LONGS ((DNX_TMC_NOF_FAP_PORTS_MAX + 31) / 32)

/*     Indication for invalid FAP port.                        */
#define DNX_TMC_FAP_PORT_ID_INVALID            81

/*
 * ERP (Egress Replication Port), is responsible of
 *  replicating packets in the egress (Multicast)
 * DNX_TMC_FAP_EGRESS_REPLICATION_SCH_PORT_ID:
 *  The Port ID When attaching flow / aggregate scheduler to it
 * DNX_TMC_FAP_EGRESS_REPLICATION_IPS_PORT_ID:
 *  The port ID when mapping the system port to it.
 */
#define DNX_TMC_FAP_EGRESS_REPLICATION_SCH_PORT_ID       80
#define DNX_TMC_FAP_EGRESS_REPLICATION_IPS_PORT_ID       255


/*
 *  Maximal Number of NIFS
 */

/*
 * Number of links from the Soc_petra device toward the fabric element.
 */
#define DNX_TMC_FBR_NOF_LINKS               36

/*
 *  The coefficient to convert 1Kilo-bit-per-second to bit-per-second (e.g.).
 */
#define DNX_TMC_RATE_1K                           1000
/*
* Maximal interface rate, in Mega-bits per second.
* This is the upper boundary, it can be lower
*  depending on the credit size
*/               
#define DNX_TMC_SCH_MAX_RATE_MBPS_JER2_ARAD(unit)            (DNX_TMC_RATE_1K * SOC_DNX_DEFS_GET(unit, max_gbps_rate_sch))

/*
* Maximal interface rate, in Kilo-bits per second.
* This is the upper boundary, it can be lower
*  depending on the credit size
*/
#define DNX_TMC_SCH_MAX_RATE_KBPS_JER2_ARAD(unit)             (DNX_TMC_RATE_1K * DNX_TMC_SCH_MAX_RATE_MBPS_JER2_ARAD(unit))

/*
 * Maximal interface rate, in Mega-bits per second.
 * This is the upper boundary, it can be lower
 *  depending on the credit size
 */
#define DNX_TMC_IF_MAX_RATE_MBPS_JER2_ARAD(unit)              (DNX_TMC_RATE_1K * SOC_DNX_DEFS_GET(unit, max_gbps_rate_egq)) 
/*
 * Maximal interface rate, in Kilo-bits per second.
 * This is the upper boundary, it can be lower
 *  depending on the credit size
 */
#define DNX_TMC_IF_MAX_RATE_KBPS_JER2_ARAD(unit)              (DNX_TMC_RATE_1K * DNX_TMC_IF_MAX_RATE_MBPS_JER2_ARAD(unit))

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
#define DNX_TMC_DEFAULT_DESTINATION_FAP    (DNX_TMC_NOF_FAPS_IN_SYSTEM-1)
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
#define DNX_TMC_MULT_NOF_MULTICAST_GROUPS_JER2_ARAD  (64*1024)


/*
 * Number of Snoop Commands.
 * There are 15 Snoop Commands altogether 1-15, 0 means disabling Snooping.
 */
/*
* Number of Signatures.
*/
#define DNX_TMC_NOF_SIGNATURE        4
#define DNX_TMC_MAX_SIGNATURE        (DNX_TMC_NOF_SIGNATURE-1)

/*
 * Packet Size (Bytes)
 */

/*
 * Copy-unique-data (e.g. outlif) index
 */

/*     Maximal number of LAG groups.                        */
#define DNX_TMC_NOF_LAG_GROUPS              (256)
#define DNX_TMC_MAX_LAG_GROUP_ID            (DNX_TMC_NOF_LAG_GROUPS - 1)

/*
 *  Rate configuration calendar sets - A and B
 */

/* Ingress TC Mapping Profiles (UC, Flow) */
#define DNX_TMC_NOF_INGRESS_UC_TC_MAPPING_PROFILES    4
#define DNX_TMC_NOF_INGRESS_FLOW_TC_MAPPING_PROFILES  4
/* } */

/*************
 * MACROS    *
 *************/
/* { */
#define DNX_TMC_DEBUG (DNX_SAND_DEBUG)
#define DNX_TMC_DEBUG_IS_LVL1   (DNX_TMC_DEBUG >= DNX_SAND_DBG_LVL1)

#define DNX_TMC_DO_NOTHING_AND_EXIT                       goto exit

/* Data Ports Macros { */
#define DNX_TMC_OLP_PORT_ID                   79


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
#define DNX_TMC_IF_NOF_NIFS 32
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
typedef uint32 DNX_TMC_MULT_ID;

/*
 * Soc_petra IPQ Queue type - Traffic Class. Range: 0-7
 */
typedef uint32 DNX_TMC_TR_CLS;

/*
 * Traffic class groups index. Range: 0-7
 */
typedef uint32 DNX_TMC_TCG_NDX;

/* Port id.
 * Soc_petra range: 0 - 79,
 * JER2_ARAD range: 0-255.
 */
typedef uint32 DNX_TMC_FAP_PORT_ID;

/* Tunnel id.
 * JER2_ARAD range: 0 - 4K-1.
 */
typedef uint32 DNX_TMC_PON_TUNNEL_ID;

typedef enum
{
  DNX_TMC_OLP_ID       = 0 ,
  DNX_TMC_IRE_ID       = 1 ,
  DNX_TMC_IDR_ID       = 2 ,
  DNX_TMC_IRR_ID       = 3 ,
  DNX_TMC_IHP_ID       = 4 ,
  DNX_TMC_QDR_ID       = 5 ,
  DNX_TMC_IPS_ID       = 6 ,
  DNX_TMC_IPT_ID       = 7 ,
  DNX_TMC_DPI_A_ID     = 8 ,
  DNX_TMC_DPI_B_ID     = 9 ,
  DNX_TMC_DPI_C_ID     = 10,
  DNX_TMC_DPI_D_ID     = 11,
  DNX_TMC_DPI_E_ID     = 12,
  DNX_TMC_DPI_F_ID     = 13,
  DNX_TMC_RTP_ID       = 14,
  DNX_TMC_EGQ_ID       = 15,
  DNX_TMC_SCH_ID       = 16,
  DNX_TMC_CFC_ID       = 17,
  DNX_TMC_EPNI_ID      = 18,
  DNX_TMC_IQM_ID       = 19,
  DNX_TMC_MMU_ID       = 20,
  DNX_TMC_NOF_MODULES  = 21
}DNX_TMC_MODULE_ID;

typedef enum
{
  /*
   *  FE1 device: 3b110
   */
  DNX_TMC_FAR_DEVICE_TYPE_FE1=6,
  /*
   *  FE2 device: 3bx11
   */
  DNX_TMC_FAR_DEVICE_TYPE_FE2=3,
  /*
   *  FE3 device: 3b010
   */
  DNX_TMC_FAR_DEVICE_TYPE_FE3=2,
  /*
   *  FAP device: 3bx0x
   */
  DNX_TMC_FAR_DEVICE_TYPE_FAP=0,
  DNX_TMC_FAR_NOF_DEVICE_TYPES = 8

}DNX_TMC_FAR_DEVICE_TYPE;

typedef enum
{
  /*
   *  default value, undefined
   */
  DNX_TMC_IF_TYPE_NONE=0,
  /*
   *  CPU port interface, channelized
   */
  DNX_TMC_IF_TYPE_CPU=1,
  /*
   *  Recycling port interface, channelized
   */
  DNX_TMC_IF_TYPE_RCY=2,
  /*
   *  Offload processor port interface, not channelized
   */
  DNX_TMC_IF_TYPE_OLP=3,
  /*
   *  Egress replication port is not a real interface,
   *  but this interface-equivalent of the ERP is required for
   *  various configuratios.
   */
  DNX_TMC_IF_TYPE_ERP=4,
  /*
   *  Network interface, channelized or non-channelized
   */
  DNX_TMC_IF_TYPE_NIF=5,
  /* 
   * OAM processor port
   */
  DNX_TMC_IF_TYPE_OAMP=6,

  DNX_TMC_IF_NOF_TYPES = 7
}DNX_TMC_INTERFACE_TYPE;

typedef enum
{
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 0 First
   *  interface in MAL 0
   */
  DNX_TMC_IF_ID_0 = 0,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 1
   */
  DNX_TMC_IF_ID_1 = 1,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 2
   */
  DNX_TMC_IF_ID_2 = 2,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 3
   */
  DNX_TMC_IF_ID_3 = 3,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 4 First
   *  interface in MAL 1
   */
  DNX_TMC_IF_ID_4 = 4,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 5
   */
  DNX_TMC_IF_ID_5 = 5,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 6
   */
  DNX_TMC_IF_ID_6 = 6,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 7
   */
  DNX_TMC_IF_ID_7 = 7,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 8 First
   *  interface in MAL 2
   */
  DNX_TMC_IF_ID_8 = 8,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 9
   */
  DNX_TMC_IF_ID_9 = 9,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 10
   */
  DNX_TMC_IF_ID_10 = 10,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 11
   */
  DNX_TMC_IF_ID_11 = 11,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 12 First
   *  interface in MAL 3
   */
  DNX_TMC_IF_ID_12 = 12,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 13
   */
  DNX_TMC_IF_ID_13 = 13,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 14
   */
  DNX_TMC_IF_ID_14 = 14,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 15
   */
  DNX_TMC_IF_ID_15 = 15,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 16 First
   *  interface in MAL 4
   */
  DNX_TMC_IF_ID_16 = 16,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 17
   */
  DNX_TMC_IF_ID_17 = 17,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 18
   */
  DNX_TMC_IF_ID_18 = 18,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 19
   */
  DNX_TMC_IF_ID_19 = 19,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 20 First
   *  interface in MAL 5
   */
  DNX_TMC_IF_ID_20 = 20,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 21
   */
  DNX_TMC_IF_ID_21 = 21,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 22
   */
  DNX_TMC_IF_ID_22 = 22,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 23
   */
  DNX_TMC_IF_ID_23 = 23,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 24 First
   *  interface in MAL 6
   */
  DNX_TMC_IF_ID_24 = 24,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 25
   */
  DNX_TMC_IF_ID_25 = 25,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 26
   */
  DNX_TMC_IF_ID_26 = 26,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 27
   */
  DNX_TMC_IF_ID_27 = 27,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 28 First
   *  interface in MAL 7
   */
  DNX_TMC_IF_ID_28 = 28,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 29
   */
  DNX_TMC_IF_ID_29 = 29,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 30
   */
  DNX_TMC_IF_ID_30 = 30,
  /*
   *  (Soc_petra-A-Compatible Index) NIF ID 31
   */
  DNX_TMC_IF_ID_31 = 31,
  /*
   *  XAUI NIF Id 0
   */
  DNX_TMC_NIF_ID_XAUI_0 = 1000,
  /*
   *  XAUI NIF Id 1
   */
  DNX_TMC_NIF_ID_XAUI_1 = 1001,
  /*
   *  XAUI NIF Id 2
   */
  DNX_TMC_NIF_ID_XAUI_2 = 1002,
  /*
   *  XAUI NIF Id 3
   */
  DNX_TMC_NIF_ID_XAUI_3 = 1003,
  /*
   *  XAUI NIF Id 4
   */
  DNX_TMC_NIF_ID_XAUI_4 = 1004,
  /*
   *  XAUI NIF Id 5
   */
  DNX_TMC_NIF_ID_XAUI_5 = 1005,
  /*
   *  XAUI NIF Id 6
   */
  DNX_TMC_NIF_ID_XAUI_6 = 1006,
  /*
   *  XAUI NIF Id 7
   */
  DNX_TMC_NIF_ID_XAUI_7 = 1007,
  /*
   *  RXAUI NIF Id 0
   */
  DNX_TMC_NIF_ID_RXAUI_0 = 2000,
  /*
   *  RXAUI NIF Id 1
   */
  DNX_TMC_NIF_ID_RXAUI_1 = 2001,
  /*
   *  RXAUI NIF Id 2
   */
  DNX_TMC_NIF_ID_RXAUI_2 = 2002,
  /*
   *  RXAUI NIF Id 3
   */
  DNX_TMC_NIF_ID_RXAUI_3 = 2003,
  /*
   *  RXAUI NIF Id 4
   */
  DNX_TMC_NIF_ID_RXAUI_4 = 2004,
  /*
   *  RXAUI NIF Id 5
   */
  DNX_TMC_NIF_ID_RXAUI_5 = 2005,
  /*
   *  RXAUI NIF Id 6
   */
  DNX_TMC_NIF_ID_RXAUI_6 = 2006,
  /*
   *  RXAUI NIF Id 7
   */
  DNX_TMC_NIF_ID_RXAUI_7 = 2007,
  /*
   *  RXAUI NIF Id 8
   */
  DNX_TMC_NIF_ID_RXAUI_8 = 2008,
  /*
   *  RXAUI NIF Id 9
   */
  DNX_TMC_NIF_ID_RXAUI_9 = 2009,
  /*
   *  RXAUI NIF Id 10
   */
  DNX_TMC_NIF_ID_RXAUI_10 = 2010,
  /*
   *  RXAUI NIF Id 11
   */
  DNX_TMC_NIF_ID_RXAUI_11 = 2011,
  /*
   *  RXAUI NIF Id 12
   */
  DNX_TMC_NIF_ID_RXAUI_12 = 2012,
  /*
   *  RXAUI NIF Id 13
   */
  DNX_TMC_NIF_ID_RXAUI_13 = 2013,
  /*
   *  RXAUI NIF Id 14
   */
  DNX_TMC_NIF_ID_RXAUI_14 = 2014,
  /*
   *  RXAUI NIF Id 15
   */
  DNX_TMC_NIF_ID_RXAUI_15 = 2015,
  /*
   *  SGMII NIF Id 0
   */
  DNX_TMC_NIF_ID_SGMII_0 = 3000,
  /*
   *  SGMII NIF Id 1
   */
  DNX_TMC_NIF_ID_SGMII_1 = 3001,
  /*
   *  SGMII NIF Id 2
   */
  DNX_TMC_NIF_ID_SGMII_2 = 3002,
  /*
   *  SGMII NIF Id 3
   */
  DNX_TMC_NIF_ID_SGMII_3 = 3003,
  /*
   *  SGMII NIF Id 4
   */
  DNX_TMC_NIF_ID_SGMII_4 = 3004,
  /*
   *  SGMII NIF Id 5
   */
  DNX_TMC_NIF_ID_SGMII_5 = 3005,
  /*
   *  SGMII NIF Id 6
   */
  DNX_TMC_NIF_ID_SGMII_6 = 3006,
  /*
   *  SGMII NIF Id 7
   */
  DNX_TMC_NIF_ID_SGMII_7 = 3007,
  /*
   *  SGMII NIF Id 8
   */
  DNX_TMC_NIF_ID_SGMII_8 = 3008,
  /*
   *  SGMII NIF Id 9
   */
  DNX_TMC_NIF_ID_SGMII_9 = 3009,
  /*
   *  SGMII NIF Id 10
   */
  DNX_TMC_NIF_ID_SGMII_10 = 3010,
  /*
   *  SGMII NIF Id 11
   */
  DNX_TMC_NIF_ID_SGMII_11 = 3011,
  /*
   *  SGMII NIF Id 12
   */
  DNX_TMC_NIF_ID_SGMII_12 = 3012,
  /*
   *  SGMII NIF Id 13
   */
  DNX_TMC_NIF_ID_SGMII_13 = 3013,
  /*
   *  SGMII NIF Id 14
   */
  DNX_TMC_NIF_ID_SGMII_14 = 3014,
  /*
   *  SGMII NIF Id 15
   */
  DNX_TMC_NIF_ID_SGMII_15 = 3015,
  /*
   *  SGMII NIF Id 16
   */
  DNX_TMC_NIF_ID_SGMII_16 = 3016,
  /*
   *  SGMII NIF Id 17
   */
  DNX_TMC_NIF_ID_SGMII_17 = 3017,
  /*
   *  SGMII NIF Id 18
   */
  DNX_TMC_NIF_ID_SGMII_18 = 3018,
  /*
   *  SGMII NIF Id 19
   */
  DNX_TMC_NIF_ID_SGMII_19 = 3019,
  /*
   *  SGMII NIF Id 20
   */
  DNX_TMC_NIF_ID_SGMII_20 = 3020,
  /*
   *  SGMII NIF Id 21
   */
  DNX_TMC_NIF_ID_SGMII_21 = 3021,
  /*
   *  SGMII NIF Id 22
   */
  DNX_TMC_NIF_ID_SGMII_22 = 3022,
  /*
   *  SGMII NIF Id 23
   */
  DNX_TMC_NIF_ID_SGMII_23 = 3023,
  /*
   *  SGMII NIF Id 24
   */
  DNX_TMC_NIF_ID_SGMII_24 = 3024,
  /*
   *  SGMII NIF Id 25
   */
  DNX_TMC_NIF_ID_SGMII_25 = 3025,
  /*
   *  SGMII NIF Id 26
   */
  DNX_TMC_NIF_ID_SGMII_26 = 3026,
  /*
   *  SGMII NIF Id 27
   */
  DNX_TMC_NIF_ID_SGMII_27 = 3027,
  /*
   *  SGMII NIF Id 28
   */
  DNX_TMC_NIF_ID_SGMII_28 = 3028,
  /*
   *  SGMII NIF Id 29
   */
  DNX_TMC_NIF_ID_SGMII_29 = 3029,
  /*
   *  SGMII NIF Id 30
   */
  DNX_TMC_NIF_ID_SGMII_30 = 3030,
  /*
   *  SGMII NIF Id 31
   */
  DNX_TMC_NIF_ID_SGMII_31 = 3031,
  /*
   *  QSGMII NIF Id 0
   */
  DNX_TMC_NIF_ID_QSGMII_0 = 4000,
  /*
   *  QSGMII NIF Id 1
   */
  DNX_TMC_NIF_ID_QSGMII_1 = 4001,
  /*
   *  QSGMII NIF Id 2
   */
  DNX_TMC_NIF_ID_QSGMII_2 = 4002,
  /*
   *  QSGMII NIF Id 3
   */
  DNX_TMC_NIF_ID_QSGMII_3 = 4003,
  /*
   *  QSGMII NIF Id 4
   */
  DNX_TMC_NIF_ID_QSGMII_4 = 4004,
  /*
   *  QSGMII NIF Id 5
   */
  DNX_TMC_NIF_ID_QSGMII_5 = 4005,
  /*
   *  QSGMII NIF Id 6
   */
  DNX_TMC_NIF_ID_QSGMII_6 = 4006,
  /*
   *  QSGMII NIF Id 7
   */
  DNX_TMC_NIF_ID_QSGMII_7 = 4007,
  /*
   *  QSGMII NIF Id 8
   */
  DNX_TMC_NIF_ID_QSGMII_8 = 4008,
  /*
   *  QSGMII NIF Id 9
   */
  DNX_TMC_NIF_ID_QSGMII_9 = 4009,
  /*
   *  QSGMII NIF Id 10
   */
  DNX_TMC_NIF_ID_QSGMII_10 = 4010,
  /*
   *  QSGMII NIF Id 11
   */
  DNX_TMC_NIF_ID_QSGMII_11 = 4011,
  /*
   *  QSGMII NIF Id 12
   */
  DNX_TMC_NIF_ID_QSGMII_12 = 4012,
  /*
   *  QSGMII NIF Id 13
   */
  DNX_TMC_NIF_ID_QSGMII_13 = 4013,
  /*
   *  QSGMII NIF Id 14
   */
  DNX_TMC_NIF_ID_QSGMII_14 = 4014,
  /*
   *  QSGMII NIF Id 15
   */
  DNX_TMC_NIF_ID_QSGMII_15 = 4015,
  /*
   *  QSGMII NIF Id 16
   */
  DNX_TMC_NIF_ID_QSGMII_16 = 4016,
  /*
   *  QSGMII NIF Id 17
   */
  DNX_TMC_NIF_ID_QSGMII_17 = 4017,
  /*
   *  QSGMII NIF Id 18
   */
  DNX_TMC_NIF_ID_QSGMII_18 = 4018,
  /*
   *  QSGMII NIF Id 19
   */
  DNX_TMC_NIF_ID_QSGMII_19 = 4019,
  /*
   *  QSGMII NIF Id 20
   */
  DNX_TMC_NIF_ID_QSGMII_20 = 4020,
  /*
   *  QSGMII NIF Id 21
   */
  DNX_TMC_NIF_ID_QSGMII_21 = 4021,
  /*
   *  QSGMII NIF Id 22
   */
  DNX_TMC_NIF_ID_QSGMII_22 = 4022,
  /*
   *  QSGMII NIF Id 23
   */
  DNX_TMC_NIF_ID_QSGMII_23 = 4023,
  /*
   *  QSGMII NIF Id 24
   */
  DNX_TMC_NIF_ID_QSGMII_24 = 4024,
  /*
   *  QSGMII NIF Id 25
   */
  DNX_TMC_NIF_ID_QSGMII_25 = 4025,
  /*
   *  QSGMII NIF Id 26
   */
  DNX_TMC_NIF_ID_QSGMII_26 = 4026,
  /*
   *  QSGMII NIF Id 27
   */
  DNX_TMC_NIF_ID_QSGMII_27 = 4027,
  /*
   *  QSGMII NIF Id 28
   */
  DNX_TMC_NIF_ID_QSGMII_28 = 4028,
  /*
   *  QSGMII NIF Id 29
   */
  DNX_TMC_NIF_ID_QSGMII_29 = 4029,
  /*
   *  QSGMII NIF Id 30
   */
  DNX_TMC_NIF_ID_QSGMII_30 = 4030,
  /*
   *  QSGMII NIF Id 31
   */
  DNX_TMC_NIF_ID_QSGMII_31 = 4031,
  /*
   *  QSGMII NIF Id 32
   */
  DNX_TMC_NIF_ID_QSGMII_32 = 4032,
  /*
   *  QSGMII NIF Id 33
   */
  DNX_TMC_NIF_ID_QSGMII_33 = 4033,
  /*
   *  QSGMII NIF Id 34
   */
  DNX_TMC_NIF_ID_QSGMII_34 = 4034,
  /*
   *  QSGMII NIF Id 35
   */
  DNX_TMC_NIF_ID_QSGMII_35 = 4035,
  /*
   *  QSGMII NIF Id 36
   */
  DNX_TMC_NIF_ID_QSGMII_36 = 4036,
  /*
   *  QSGMII NIF Id 37
   */
  DNX_TMC_NIF_ID_QSGMII_37 = 4037,
  /*
   *  QSGMII NIF Id 38
   */
  DNX_TMC_NIF_ID_QSGMII_38 = 4038,
  /*
   *  QSGMII NIF Id 39
   */
  DNX_TMC_NIF_ID_QSGMII_39 = 4039,
  /*
   *  QSGMII NIF Id 40
   */
  DNX_TMC_NIF_ID_QSGMII_40 = 4040,
  /*
   *  QSGMII NIF Id 41
   */
  DNX_TMC_NIF_ID_QSGMII_41 = 4041,
  /*
   *  QSGMII NIF Id 42
   */
  DNX_TMC_NIF_ID_QSGMII_42 = 4042,
  /*
   *  QSGMII NIF Id 43
   */
  DNX_TMC_NIF_ID_QSGMII_43 = 4043,
  /*
   *  QSGMII NIF Id 44
   */
  DNX_TMC_NIF_ID_QSGMII_44 = 4044,
  /*
   *  QSGMII NIF Id 45
   */
  DNX_TMC_NIF_ID_QSGMII_45 = 4045,
  /*
   *  QSGMII NIF Id 46
   */
  DNX_TMC_NIF_ID_QSGMII_46 = 4046,
  /*
   *  QSGMII NIF Id 47
   */
  DNX_TMC_NIF_ID_QSGMII_47 = 4047,
  /*
   *  QSGMII NIF Id 48
   */
  DNX_TMC_NIF_ID_QSGMII_48 = 4048,
  /*
   *  QSGMII NIF Id 49
   */
  DNX_TMC_NIF_ID_QSGMII_49 = 4049,
  /*
   *  QSGMII NIF Id 50
   */
  DNX_TMC_NIF_ID_QSGMII_50 = 4050,
  /*
   *  QSGMII NIF Id 51
   */
  DNX_TMC_NIF_ID_QSGMII_51 = 4051,
  /*
   *  QSGMII NIF Id 52
   */
  DNX_TMC_NIF_ID_QSGMII_52 = 4052,
  /*
   *  QSGMII NIF Id 53
   */
  DNX_TMC_NIF_ID_QSGMII_53 = 4053,
  /*
   *  QSGMII NIF Id 54
   */
  DNX_TMC_NIF_ID_QSGMII_54 = 4054,
  /*
   *  QSGMII NIF Id 55
   */
  DNX_TMC_NIF_ID_QSGMII_55 = 4055,
  /*
   *  QSGMII NIF Id 56
   */
  DNX_TMC_NIF_ID_QSGMII_56 = 4056,
  /*
   *  QSGMII NIF Id 57
   */
  DNX_TMC_NIF_ID_QSGMII_57 = 4057,
  /*
   *  QSGMII NIF Id 58
   */
  DNX_TMC_NIF_ID_QSGMII_58 = 4058,
  /*
   *  QSGMII NIF Id 59
   */
  DNX_TMC_NIF_ID_QSGMII_59 = 4059,
  /*
   *  QSGMII NIF Id 60
   */
  DNX_TMC_NIF_ID_QSGMII_60 = 4060,
  /*
   *  QSGMII NIF Id 61
   */
  DNX_TMC_NIF_ID_QSGMII_61 = 4061,
  /*
   *  QSGMII NIF Id 62
   */
  DNX_TMC_NIF_ID_QSGMII_62 = 4062,
  /*
   *  QSGMII NIF Id 63
   */
  DNX_TMC_NIF_ID_QSGMII_63 = 4063,
  /*
   *  Interlaken NIF Id 0
   */
  DNX_TMC_NIF_ID_ILKN_0 = 5000,
  /*
   *  Interlaken NIF Id 1
   */
  DNX_TMC_NIF_ID_ILKN_1 = 5001,
  /*
   *  Interlaken NIF Id 0
   */
  DNX_TMC_NIF_ID_ILKN_TDM_0 = 5002,
  /*
   *  Interlaken NIF Id 1
   */
  DNX_TMC_NIF_ID_ILKN_TDM_1 = 5003,
  /*
   *  100G CGE NIF Id 0
   */
  DNX_TMC_NIF_ID_CGE_0 = 7000,
  /*
   *  100G CGE NIF Id 1
   */
  DNX_TMC_NIF_ID_CGE_1 = 7001,
  /*
   *  40G XLGE NIF Id 0
   */
  DNX_TMC_NIF_ID_XLGE_0 = 8000,
  /*
   *  40G XLGE NIF Id 1
   */
  DNX_TMC_NIF_ID_XLGE_1 = 8001,
  /*
   *  40G XLGE NIF Id 2
   */
  DNX_TMC_NIF_ID_XLGE_2 = 8002,
  /*
   *  40G XLGE NIF Id 3
   */
  DNX_TMC_NIF_ID_XLGE_3 = 8003,
  /*
   *  40G XLGE NIF Id 4
   */
  DNX_TMC_NIF_ID_XLGE_4 = 8004,
  /*
   *  40G XLGE NIF Id 5
   */
  DNX_TMC_NIF_ID_XLGE_5 = 8005,
  /*
   *  40G XLGE NIF Id 6
   */
  DNX_TMC_NIF_ID_XLGE_6 = 8006,
  /*
   *  40G XLGE NIF Id 7
   */
  DNX_TMC_NIF_ID_XLGE_7 = 8007,
  /*
   *  10G Base-R NIF Id 0
   */
  DNX_TMC_NIF_ID_10GBASE_R_0 = 9000,
  /*
   *  10G Base-R NIF Id 1
   */
  DNX_TMC_NIF_ID_10GBASE_R_1 = 9001,
  /*
   *  10G Base-R NIF Id 2
   */
  DNX_TMC_NIF_ID_10GBASE_R_2 = 9002,
  /*
   *  10G Base-R NIF Id 3
   */
  DNX_TMC_NIF_ID_10GBASE_R_3 = 9003,
  /*
   *  10G Base-R NIF Id 4
   */
  DNX_TMC_NIF_ID_10GBASE_R_4 = 9004,
  /*
   *  10G Base-R NIF Id 5
   */
  DNX_TMC_NIF_ID_10GBASE_R_5 = 9005,
  /*
   *  10G Base-R NIF Id 6
   */
  DNX_TMC_NIF_ID_10GBASE_R_6 = 9006,
  /*
   *  10G Base-R NIF Id 7
   */
  DNX_TMC_NIF_ID_10GBASE_R_7 = 9007,
  /*
   *  10G Base-R NIF Id 8
   */
  DNX_TMC_NIF_ID_10GBASE_R_8 = 9008,
  /*
   *  10G Base-R NIF Id 9
   */
  DNX_TMC_NIF_ID_10GBASE_R_9 = 9009,
  /*
   *  10G Base-R NIF Id 10
   */
  DNX_TMC_NIF_ID_10GBASE_R_10 = 9010,
  /*
   *  10G Base-R NIF Id 11
   */
  DNX_TMC_NIF_ID_10GBASE_R_11 = 9011,
  /*
   *  10G Base-R NIF Id 12
   */
  DNX_TMC_NIF_ID_10GBASE_R_12 = 9012,
  /*
   *  10G Base-R NIF Id 13
   */
  DNX_TMC_NIF_ID_10GBASE_R_13 = 9013,
  /*
   *  10G Base-R NIF Id 14
   */
  DNX_TMC_NIF_ID_10GBASE_R_14 = 9014,
  /*
   *  10G Base-R NIF Id 15
   */
  DNX_TMC_NIF_ID_10GBASE_R_15 = 9015,
  /*
   *  10G Base-R NIF Id 16
   */
  DNX_TMC_NIF_ID_10GBASE_R_16 = 9016,
  /*
   *  10G Base-R NIF Id 17
   */
  DNX_TMC_NIF_ID_10GBASE_R_17 = 9017,
  /*
   *  10G Base-R NIF Id 18
   */
  DNX_TMC_NIF_ID_10GBASE_R_18 = 9018,
  /*
   *  10G Base-R NIF Id 19
   */
  DNX_TMC_NIF_ID_10GBASE_R_19 = 9019,
  /*
   *  10G Base-R NIF Id 20
   */
  DNX_TMC_NIF_ID_10GBASE_R_20 = 9020,
  /*
   *  10G Base-R NIF Id 21
   */
  DNX_TMC_NIF_ID_10GBASE_R_21 = 9021,
  /*
   *  10G Base-R NIF Id 22
   */
  DNX_TMC_NIF_ID_10GBASE_R_22 = 9022,
  /*
   *  10G Base-R NIF Id 23
   */
  DNX_TMC_NIF_ID_10GBASE_R_23 = 9023,
  /*
   *  10G Base-R NIF Id 24
   */
  DNX_TMC_NIF_ID_10GBASE_R_24 = 9024,
  /*
   *  10G Base-R NIF Id 25
   */
  DNX_TMC_NIF_ID_10GBASE_R_25 = 9025,
  /*
   *  10G Base-R NIF Id 26
   */
  DNX_TMC_NIF_ID_10GBASE_R_26 = 9026,
  /*
   *  10G Base-R NIF Id 27
   */
  DNX_TMC_NIF_ID_10GBASE_R_27 = 9027,
  /*
   *  10G Base-R NIF Id 28
   */
  DNX_TMC_NIF_ID_10GBASE_R_28 = 9028,
  /*
   *  10G Base-R NIF Id 29
   */
  DNX_TMC_NIF_ID_10GBASE_R_29 = 9029,
  /*
   *  10G Base-R NIF Id 30
   */
  DNX_TMC_NIF_ID_10GBASE_R_30 = 9030,
  /*
   *  10G Base-R NIF Id 31
   */
  DNX_TMC_NIF_ID_10GBASE_R_31 = 9031,
  /*
   *  CPU port interface
   */
  DNX_TMC_IF_ID_CPU = 200,
  /*
   *  OLP port interface
   */
  DNX_TMC_IF_ID_OLP = 300,
  /*
   *  Recycling port interface
   */
  DNX_TMC_IF_ID_RCY = 400,
  /*
   *  Egress Replication Port (Device Virtual Port)
   */
  DNX_TMC_IF_ID_ERP = 500,
  /*
   *
   */
  DNX_TMC_IF_ID_OAMP = 600,
  /*
   * Synchronous Packet interfaces between ATMF and Spider NPU .
   */
  DNX_TMC_IF_ID_TM_INTERNAL_PKT = 700,
  /*
   * Mirror enabled port(logical port and tm port) .
   */
  DNX_TMC_IF_ID_RESERVED = 800,
  /*
   *  Invalid Interface
   */
   DNX_TMC_IF_ID_NONE = 65535
}DNX_TMC_INTERFACE_ID;

#define DNX_TMC_NIF_ID_NONE DNX_TMC_IF_ID_NONE

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
  DNX_TMC_MAL_TYPE_NONE=0,
  /*
   *  MAL-equivalent of CPU interface
   */
  DNX_TMC_MAL_TYPE_CPU=1,
  /*
   *  MAL-equivalent of Recycling interface
   */
  DNX_TMC_MAL_TYPE_RCY=2,
  /*
   *  MAL-equivalent of OLP interface
   */
  DNX_TMC_MAL_TYPE_OLP=3,
  /*
   *  MAL-equivalent of OLP interface
   */
  DNX_TMC_MAL_TYPE_ERP=4,
  /*
   *  MAL-equivalent of network interface
   */
  DNX_TMC_MAL_TYPE_NIF=5,

  DNX_TMC_MAL_TYPE_NOF_TYPES=6
}DNX_TMC_MAL_EQUIVALENT_TYPE;

typedef enum
{
  /*
  *  Flow Control direction  generation.
  */
  DNX_TMC_FC_DIRECTION_GEN=0,
  /*
  *  Flow Control direction  reception.
  */
  DNX_TMC_FC_DIRECTION_REC=1,
  /*
  *  Total number of Flow Control directions.
  */
  DNX_TMC_FC_NOF_DIRECTIONS=2
}DNX_TMC_FC_DIRECTION;




typedef enum
{
  /*
  *  Destination type: Incoming Queue. Matching Index Range: 0
  *   32K-1
  */
  DNX_TMC_DEST_TYPE_QUEUE=0,
  /*
  *  Destination type: Multicast Group. Matching Index Range:
  *  0  16K-1
  */
  DNX_TMC_DEST_TYPE_MULTICAST=1,
  /*
  *  System Physical FAP Port. Matching Index Range: 0  4095.
  */
  DNX_TMC_DEST_TYPE_SYS_PHY_PORT=2,
  /*
  *  System LAG Id. Matching Index Range: 0  255.
  */
  DNX_TMC_DEST_TYPE_LAG=3,
  /*
  *  Total number of destination types.
  */
  DNX_TMC_NOF_DEST_TYPES_PETRA=4,
  /*
  *  Ingress Shaping Queue flow id
  */
  DNX_TMC_DEST_TYPE_ISQ_FLOW_ID=4,
  /*
  *  OutLif
  */
  DNX_TMC_DEST_TYPE_OUT_LIF=5,
  /*
  *  Multicast Flow ID.
  */
  DNX_TMC_DEST_TYPE_MULTICAST_FLOW_ID=6,
  /*
  *  Total number of destination types.
  */
  DNX_TMC_NOF_DEST_TYPES_JER2_ARAD=7,
  /*
    *  FEC pointer id
  */
  DNX_TMC_DEST_TYPE_FEC_PTR = DNX_TMC_NOF_DEST_TYPES_JER2_ARAD,
  /*
  *  Total number of destination types.
  */
  DNX_TMC_NOF_DEST_TYPES_JER2_JER=8

}DNX_TMC_DEST_TYPE;

typedef enum
{
  /*
  *  System Physical FAP Port. Matching Index Range: 0  4095.
  */
  DNX_TMC_DEST_SYS_PORT_TYPE_SYS_PHY_PORT=0,
  /*
  *  System LAG Id. Matching Index Range: 0  255.
  */
  DNX_TMC_DEST_SYS_PORT_TYPE_LAG=1,
  /*
  *  Total number of system-ports types.
  */
  DNX_TMC_DEST_SYS_PORT_NOF_TYPES=2
}DNX_TMC_DEST_SYS_PORT_TYPE;

typedef enum
{
    DNX_TMC_CMD_TYPE_MIRROR = 0,
    DNX_TMC_CMD_TYPE_SNOOP = 1
}DNX_TMC_CMD_TYPE;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
    *  LAG id or system-physical-port id.
    */
  DNX_TMC_DEST_SYS_PORT_TYPE type;
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
}DNX_TMC_DEST_SYS_PORT_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Queue, multicast group or system port / LAG id (Unicast
   *  flow, System multicast, Unicast Port accordingly).
   */
  DNX_TMC_DEST_TYPE type;
  /*
   *  According to destination type, one of the
   *  following:Queue id (Unicast flow), Range: 0 -
   *  32K-1. Multicast id (System Multicast), Range: 0 -
   *  16K-1. System Physical Port, Range: 0  4K-1. LAG id., Range:
   *  0-255.
   */
  uint32 id;

  /* 
   * The matching dbal type: 
   * DBAL_FIELD_MC_ID, DBAL_FIELD_FLOW_ID, DBAL_FIELD_PORT_ID
   */ 
  int dbal_type;
}DNX_TMC_DEST_INFO;

typedef enum
{
  /*
   */
  DNX_TMC_MULTICAST_CLASS_0=0,
  /*
   */
  DNX_TMC_MULTICAST_CLASS_1=1,
  /*
   */
  DNX_TMC_MULTICAST_CLASS_2=2,
  /*
   */
  DNX_TMC_MULTICAST_CLASS_3=3,
  /*
   */
  DNX_TMC_NOF_MULTICAST_CLASSES=4,
  /*
   *  Must be the last value
   */
  DNX_TMC_MULTICAST_CLASS_LAST

}DNX_TMC_MULTICAST_CLASS;

typedef enum
{
  /*
  *  Connection direction  receive (RX).
  */
  DNX_TMC_CONNECTION_DIRECTION_RX=0,
  /*
  *  Connection direction  transmit (TX).
  */
  DNX_TMC_CONNECTION_DIRECTION_TX=1,
  /*
  *  Connection direction  both receive and transmit
  *  (RX/TX).
  */
  DNX_TMC_CONNECTION_DIRECTION_BOTH=2,
  /*
  *  Total number of connection direction configurations.
  */
  DNX_TMC_NOF_CONNECTION_DIRECTIONS=3
}DNX_TMC_CONNECTION_DIRECTION;

typedef enum
{
  /*
   *  packet is bridged according to outer most LL header
   */
  DNX_TMC_PKT_FRWRD_TYPE_BRIDGE = 0,
  /*
   *  packet is IPv4 UC routed
   */
  DNX_TMC_PKT_FRWRD_TYPE_IPV4_UC = 1,
  /*
   *  packet is IPv4 MC routed
   */
  DNX_TMC_PKT_FRWRD_TYPE_IPV4_MC = 2,
  /*
   *  packet is IPv6 UC routed
   */
  DNX_TMC_PKT_FRWRD_TYPE_IPV6_UC = 3,
  /*
   *  packet is IPv6 MC routed
   */
  DNX_TMC_PKT_FRWRD_TYPE_IPV6_MC = 4,
  /*
   *  packet is forwarded according to MPLS label,
   *  Ingress-label-mapping
   */
  DNX_TMC_PKT_FRWRD_TYPE_MPLS = 5,
  /*
   *  packet is forwarded according to TRILL
   */
  DNX_TMC_PKT_FRWRD_TYPE_TRILL = 6,
  /*
   *  packet is trapped
   */
  DNX_TMC_PKT_FRWRD_TYPE_CPU_TRAP = 7,
  /*
   *  packet is bridged according to inner LL header, outer LL
   *  header is terminated. For example in VPLS application.
   */
  DNX_TMC_PKT_FRWRD_TYPE_BRIDGE_AFTER_TERM = 8,
  /*
   *  Custom1 - used in Arad for FCoE for FCF
   */
  DNX_TMC_PKT_FRWRD_TYPE_CUSTOM1 = 9,
  /*
   *  Packet is snooped.
   */
   DNX_TMC_PKT_FRWRD_TYPE_SNOOP = 0xB,
  /*
   *  Packet comes for a TM processing.
   */
   DNX_TMC_PKT_FRWRD_TYPE_TM = 0xE,
  /*
   *  Number of types in DNX_TMC_PKT_FRWRD_TYPE
   */
  DNX_TMC_NOF_PKT_FRWRD_TYPES = 10
}DNX_TMC_PKT_FRWRD_TYPE;

typedef enum
{
  /*
  *  No header is terminated.
  */
  DNX_TMC_TUNNEL_TERM_CODE_NONE = 0,
  /*
  *  The Ethernet header is terminated.
  */
  DNX_TMC_TUNNEL_TERM_CODE_ETH = 1,
  /*
  *  The IPv4 and Ethernet headers are terminated.
  */
  DNX_TMC_TUNNEL_TERM_CODE_IPV4_ETH = 2,
  /*
  *  The MPLS and Ethernet headers are terminated.
  */
  DNX_TMC_TUNNEL_TERM_CODE_MPLS_ETH = 3,
  /*
  *  The MPLS and Ethernet headers are terminated with
  *  Control Word.
  */
  DNX_TMC_TUNNEL_TERM_CODE_MPLS_ETH_CW = 4,
  /*
  *  The 2 MPLS and Ethernet headers are terminated.
  */
  DNX_TMC_TUNNEL_TERM_CODE_MPLS2_ETH = 5,
  /*
  *  The 2 MPLS and Ethernet headers are terminated with
  *  Control Word.
  */
  DNX_TMC_TUNNEL_TERM_CODE_MPLS2_ETH_CW = 6,
  /*
  *  The 3 MPLS and Ethernet headers are terminated.
  */
  DNX_TMC_TUNNEL_TERM_CODE_MPLS3_ETH = 7,
  /*
  *  The 3 MPLS and Ethernet headers are terminated with
  *  Control Word.
  */
  DNX_TMC_TUNNEL_TERM_CODE_MPLS3_ETH_CW = 8,
  /*
  *  The Trill and Ethernet headers are terminated.
  */
  DNX_TMC_TUNNEL_TERM_CODE_TRILL_ETH = 9,
  /*
  *  Number of types in DNX_TMC_TUNNEL_TERM_CODE
  */
  DNX_TMC_NOF_TUNNEL_TERM_CODES = 10
}DNX_TMC_TUNNEL_TERM_CODE;

typedef enum
{
  /*
   *  Push new label. The payload of the command is a push
   *  profile to build the TTL and EXP of the pushed label.
   */
  DNX_TMC_MPLS_COMMAND_TYPE_PUSH = 0,
  /*
   *  Pop MPLS header, next header, processing type
   *  and CW passed by other attributes
   *  Arad only.
   *  cannot be used for Soc_petra-B.
   */
  DNX_TMC_MPLS_COMMAND_TYPE_POP = 1,
  /*
   *  Pop into MPLS. TTL and EXP of the exposed label not
   *  affected by the popped label. May perform mapping of the
   *  EXP field.
   */
  DNX_TMC_MPLS_COMMAND_TYPE_POP_INTO_MPLS_PIPE = 8,
  /*
   *  Pop into MPLS and inherent TTL and EXP from the popped
   *  label with optional mapping of the EXP field.
   */
  DNX_TMC_MPLS_COMMAND_TYPE_POP_INTO_MPLS_UNIFORM = 9,
  /*
   *  Pop into MPLS. TTL and TOS of the exposed IPv4 header
   *  not affected by the popped label. May perform mapping of
   *  the TOS field.
   */
  DNX_TMC_MPLS_COMMAND_TYPE_POP_INTO_IPV4_PIPE = 10,
  /*
   *  Pop into IPv4 and inherent TTL and EXP from the popped
   *  label with mapping of the EXP field to TOS.
   */
  DNX_TMC_MPLS_COMMAND_TYPE_POP_INTO_IPV4_UNIFORM = 11,
  /*
   *  Pop into MPLS. TTL and TC of the exposed IPv6 header not
   *  affected by the popped label. May perform mapping of the
   *  TC field.
   */
  DNX_TMC_MPLS_COMMAND_TYPE_POP_INTO_IPV6_PIPE = 12,
  /*
   *  Pop into IPv4 and inherent TTL and EXP from the popped
   *  label with mapping of the EXP field to TC.
   */
  DNX_TMC_MPLS_COMMAND_TYPE_POP_INTO_IPV6_UNIFORM = 13,
  /*
   *  Pop into Ethernet. For Drop and Continue applications.
   */
  DNX_TMC_MPLS_COMMAND_TYPE_POP_INTO_ETHERNET = 14,
  /*
   *  Swap the label of the MPLS header.
   */
  DNX_TMC_MPLS_COMMAND_TYPE_SWAP = 15,
  /*
   *  Number of types in DNX_TMC_MPLS_COMMAND_TYPE
   */
  DNX_TMC_NOF_MPLS_COMMAND_TYPES = 16
}DNX_TMC_MPLS_COMMAND_TYPE;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Passing this threshold from above means activation.
   */
  uint32 set;
  /*
   *  Passing this threshold from below means deactivation.
   */
  uint32 clear;
}DNX_TMC_THRESH_WITH_HYST_INFO;

typedef enum
{
  /*
   *  Combo Quartet 0 - the SerDes combo quartet in STAR 0
   */
  DNX_TMC_COMBO_QRTT_0=0,
  /*
   *  Combo Quartet 1 - the SerDes combo quartet in STAR 1
   */
  DNX_TMC_COMBO_QRTT_1=1,
  /*
   *  Total number of SerDes combo quartets
   */
  DNX_TMC_COMBO_NOF_QRTTS=2
}DNX_TMC_COMBO_QRTT;


typedef enum
{
  /*
   *  SerDes is powered down.
   */
  DNX_TMC_SRD_POWER_STATE_DOWN=0,
  /*
   *  SerDes is powered up.
   */
  DNX_TMC_SRD_POWER_STATE_UP=1,
  /*
   *  SerDes is powered up. When setting this state, the
   *  SerDes is validated after power-up. If needed. a re-lock
   *  sequence is performed to verify SerDes is active. Note:
   *  this is the recommended value for powering-up the
   *  SerDes.
   */
  DNX_TMC_SRD_POWER_STATE_UP_AND_RELOCK=2,
  /*
   *  Total Number of SerDes power states.
   */
  DNX_TMC_SRD_NOF_POWER_STATES=3
}DNX_TMC_SRD_POWER_STATE;

typedef _shr_error_t DNX_TMC_ERROR;


typedef enum
{
  /*
   * SWAP operation 4 bytes after DA to the start of the packet.
   * Applicable for PON Channel 
   */
  DNX_TMC_SWAP_MODE_4_BYTES = 0,
  /*
   * SWAP operation 8 bytes after DA to the start of the packet. 
   * Applicable for E-TAG 
   */
  DNX_TMC_SWAP_MODE_8_BYTES,
  /*
   * Total number of jer2_jer2_jer2_arad init swap modes
   */
  DNX_TMC_SWAP_MODES
} DNX_TMC_SWAP_MODE;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Device swap mode for incoming packets
   */
  DNX_TMC_SWAP_MODE mode;
  /*
   *  Device swap offset to the Prepend Tag. Range: 0-15.
   *  The Prepend Tag is located at (12 + 2*offset) bytes from the start
   *  of the packet. HW Default: 0.
   */
  uint32 offset;
} DNX_TMC_SWAP_INFO;

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
  DNX_TMC_DEST_SYS_PORT_INFO_clear(
    DNX_SAND_OUT DNX_TMC_DEST_SYS_PORT_INFO *info
  );

void
  DNX_TMC_DEST_INFO_clear(
    DNX_SAND_OUT DNX_TMC_DEST_INFO *info
  );

void
  DNX_TMC_THRESH_WITH_HYST_INFO_clear(
    DNX_SAND_OUT DNX_TMC_THRESH_WITH_HYST_INFO *info
  );

void
  DNX_TMC_SWAP_INFO_clear(
    DNX_SAND_OUT DNX_TMC_SWAP_INFO *info
  );

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_COMBO_QRTT_to_string(
    DNX_SAND_IN DNX_TMC_COMBO_QRTT enum_val
  );

const char*
  DNX_TMC_FAR_DEVICE_TYPE_to_string(
    DNX_SAND_IN DNX_TMC_FAR_DEVICE_TYPE enum_val
  );

const char*
  DNX_TMC_INTERFACE_TYPE_to_string(
    DNX_SAND_IN DNX_TMC_INTERFACE_TYPE enum_val
  );

void
  DNX_TMC_INTERFACE_ID_print(
    DNX_SAND_IN DNX_TMC_INTERFACE_ID if_ndx
  );

const char*
  DNX_TMC_INTERFACE_ID_to_string(
    DNX_SAND_IN DNX_TMC_INTERFACE_ID enum_val
  );

const char*
  DNX_TMC_FC_DIRECTION_to_string(
    DNX_SAND_IN DNX_TMC_FC_DIRECTION enum_val
  );

const char*
  DNX_TMC_CONNECTION_DIRECTION_to_string(
    DNX_SAND_IN DNX_TMC_CONNECTION_DIRECTION enum_val
  );

const char*
  DNX_TMC_DEST_TYPE_to_string(
    DNX_SAND_IN DNX_TMC_DEST_TYPE enum_val,
    DNX_SAND_IN uint8       short_name
  );

const char*
  DNX_TMC_DEST_SYS_PORT_TYPE_to_string(
    DNX_SAND_IN DNX_TMC_DEST_SYS_PORT_TYPE enum_val
    );

const char*
  DNX_TMC_PKT_FRWRD_TYPE_to_string(
    DNX_SAND_IN  DNX_TMC_PKT_FRWRD_TYPE enum_val
  );

const char*
  DNX_TMC_TUNNEL_TERM_CODE_to_string(
    DNX_SAND_IN  DNX_TMC_TUNNEL_TERM_CODE enum_val
  );


void
  DNX_TMC_DEST_SYS_PORT_INFO_print(
    DNX_SAND_IN DNX_TMC_DEST_SYS_PORT_INFO *info
  );

void
  DNX_TMC_DEST_INFO_print(
    DNX_SAND_IN DNX_TMC_DEST_INFO *info
  );

void
  DNX_TMC_THRESH_WITH_HYST_INFO_print(
    DNX_SAND_IN DNX_TMC_THRESH_WITH_HYST_INFO *info
  );

void
  DNX_TMC_DEST_SYS_PORT_INFO_table_format_print(
    DNX_SAND_IN DNX_TMC_DEST_SYS_PORT_INFO *info
  );

const char*
  DNX_TMC_SWAP_MODE_to_string(
    DNX_SAND_IN  DNX_TMC_SWAP_MODE enum_val
  );

void
  DNX_TMC_SWAP_INFO_print(
    DNX_SAND_IN DNX_TMC_SWAP_INFO *info
  );

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_TMC_API_GENERAL_INCLUDED__*/
#endif
