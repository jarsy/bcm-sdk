/* $Id: arad_api_nif.h,v 1.47 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __ARAD_API_NIF_INCLUDED__
/* { */
#define __ARAD_API_NIF_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/ARAD/arad_api_general.h>
#include <soc/dpp/SAND/Utils/sand_pp_mac.h>
#include <soc/dpp/SAND/Utils/sand_64cnt.h>
#include <soc/dpp/SAND/Utils/sand_integer_arithmetic.h>

#include <soc/dpp/ARAD/arad_sw_db_tcam_mgmt.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */
#define ARAD_NIF_ID_XAUI_FIRST    ARAD_NIF_ID_XAUI_0
#define ARAD_NIF_ID_XAUI_LAST     ARAD_NIF_ID_XAUI_7

#define ARAD_NIF_ID_RXAUI_FIRST   ARAD_NIF_ID_RXAUI_0
#define ARAD_NIF_ID_RXAUI_LAST    ARAD_NIF_ID_RXAUI_15

#define ARAD_NIF_ID_SGMII_FIRST   ARAD_NIF_ID_SGMII_0
#define ARAD_NIF_ID_SGMII_LAST    ARAD_NIF_ID_SGMII_31


#define ARAD_NIF_ID_ILKN_FIRST    ARAD_NIF_ID_ILKN_0
#define ARAD_NIF_ID_ILKN_LAST     ARAD_NIF_ID_ILKN_1

#define ARAD_NIF_ID_ILKN_TDM_FIRST  ARAD_NIF_ID_ILKN_TDM_0 
#define ARAD_NIF_ID_ILKN_TDM_LAST  ARAD_NIF_ID_ILKN_TDM_1 

#define ARAD_NIF_ID_CGE_FIRST    ARAD_NIF_ID_CGE_0
#define ARAD_NIF_ID_CGE_LAST     ARAD_NIF_ID_CGE_1

#define ARAD_NIF_ID_XLGE_FIRST    ARAD_NIF_ID_XLGE_0
#define ARAD_NIF_ID_XLGE_LAST     ARAD_NIF_ID_XLGE_7

#define ARAD_NIF_ID_TM_INTERNAL_PKT_FIRST    ARAD_IF_ID_TM_INTERNAL_PKT
#define ARAD_NIF_ID_TM_INTERNAL_PKT_LAST     ARAD_IF_ID_TM_INTERNAL_PKT

#define ARAD_NIF_ID_CPU_FIRST    ARAD_IF_ID_CPU
#define ARAD_NIF_ID_CPU_LAST     ARAD_IF_ID_CPU

#define ARAD_NIF_ID_10GBASE_R_FIRST    ARAD_NIF_ID_10GBASE_R_0
#define ARAD_NIF_ID_10GBASE_R_LAST     ARAD_NIF_ID_10GBASE_R_31

/*     Total number of NIF types                               */
#define  ARAD_NIF_NOF_TYPES (5)

#define ARAD_NIF_NOF_NIFS               32
/* Just different naming */
#define ARAD_NIF_NOF_NIFS_MAX           ARAD_NIF_NOF_NIFS
#define ARAD_MAX_NIF_ID                 (ARAD_NIF_NOF_NIFS_MAX-1)
/* Just different naming */
#define ARAD_NIF_ID_MAX                 ARAD_MAX_NIF_ID
/*
 *    An indication of invalid internal NIF index (0..63 are valid values).
 */
#define ARAD_NIF_INVALID_VAL_INTERN ARAD_NIF_ID_NONE

/* 
 *  Number of Wrap core
 */
#define ARAD_NOF_WRAP_CORE              8

#define ARAD_MAX_NIFS_PER_WC            4
#define ARAD_MAX_RXAUIS_PER_WC          2
#define ARAD_NIF2WC_GLBL_ID(nif_id)      ((nif_id)/ARAD_MAX_NIFS_PER_WC)
#define ARAD_NIF_WC2NIF_BASE_ID(wc_id)  ((wc_id)*ARAD_MAX_NIFS_PER_WC)

#define ARAD_SGMII_NIFS_PER_WC          ARAD_MAX_NIFS_PER_WC
#define ARAD_RXAUI_NIFS_PER_WC          (ARAD_MAX_NIFS_PER_WC/2)
/* Maximal number of channels in channelized NIF. */

#define ARAD_ARAD_STYLE_NOF_IF_CHANNELS_MAX (256)

#define ARAD_NOF_IF_CHANNELS_MAX (ARAD_ARAD_STYLE_NOF_IF_CHANNELS_MAX)

#define ARAD_NIF_IS_TYPE_GMII(nif_type) \
  SOC_SAND_NUM2BOOL((((nif_type) == ARAD_NIF_TYPE_SGMII)))

#define ARAD_NIF_IS_TYPE_XAUI_LIKE(nif_type) \
  SOC_SAND_NUM2BOOL(((nif_type) == ARAD_NIF_TYPE_XAUI) || ((nif_type) == ARAD_NIF_TYPE_RXAUI))

#define ARAD_NIF_IS_TYPE_10GBASE_R(nif_type) \
  SOC_SAND_NUM2BOOL((((nif_type) == ARAD_NIF_TYPE_10GBASE_R)))

#define ARAD_NIF_IS_TYPE_CGE(nif_type) \
  SOC_SAND_NUM2BOOL((((nif_type) == ARAD_NIF_TYPE_100G_CGE)))
#define ARAD_NIF_IS_TYPE_XLGE(nif_type) \
  SOC_SAND_NUM2BOOL((((nif_type) == ARAD_NIF_TYPE_40G_XLGE)))

/* 
 * SyncE
 */
#define ARAD_NIF_NOF_SYNCE_CLK_IDS (2)

/* } */
/*************
 * MACROS    *
 *************/
/* { */
/*
 *    Get per-type NIF-index.
 *  For example, ARAD_NIF_ID(XAUI, 3) is
 *  ARAD_NIF_ID_XAUI_3
 */
#define ARAD_NIF_ID(tp,id) \
  (ARAD_NIF_ID_##tp##_FIRST + (id))

/*
 *    Check if a NIF-id belongs to a specific NIF type
 */
#define ARAD_NIF_IS_TYPE_ID(tp,id) \
  SOC_SAND_NUM2BOOL(SOC_SAND_IS_VAL_IN_RANGE(id, ARAD_NIF_ID_##tp##_FIRST, ARAD_NIF_ID_##tp##_LAST))

/*
 *    Get the offset-index inside a certain NIF type
 */
#define ARAD_NIF_ID_OFFSET(tp,id) \
  SOC_SAND_DELTA((int)(id), (int)ARAD_NIF_ID_##tp##_FIRST)

/*
 *    Get the maximal number of interfaces of a certain NIF type
 */
#define ARAD_NIF_TYPE_MAX(tp) \
  SOC_SAND_RNG_COUNT(ARAD_NIF_ID_##tp##_FIRST, ARAD_NIF_ID_##tp##_LAST)

/*
 *    Arad style indexing (type, in-type-id)
 */
#define ARAD_NIF_IS_ARAD_ID(id)\
  SOC_SAND_NUM2BOOL(ARAD_NIF_IS_TYPE_ID(XAUI,id)  || ARAD_NIF_IS_TYPE_ID(RXAUI,id)  || \
      ARAD_NIF_IS_TYPE_ID(SGMII,id) || \
      ARAD_NIF_IS_TYPE_ID(XLGE,id) || ARAD_NIF_IS_TYPE_ID(CGE,id) || \
      ARAD_NIF_IS_TYPE_ID(ILKN,id)|| ARAD_NIF_IS_TYPE_ID(10GBASE_R,id) || ARAD_NIF_IS_TYPE_ID(TM_INTERNAL_PKT,id))

#define ARAD_NIF_IS_VALID_ID(id) \
  SOC_SAND_NUM2BOOL(ARAD_NIF_IS_ARAD_ID(id))

/*
#define ARAD_NIF_IS_VALID_ID(id) \
  SOC_SAND_NUM2BOOL(ARAD_NIF_IS_PA_ID(id) || ARAD_NIF_IS_ARAD_ID(id))
*/


/*
 *    Even (primary) MAL in a Dual MAL (pair of MALs)
 */
#define ARAD_NIF_BASE_MAL(mal_id)         ((mal_id) & ~0x1)
/*
 *    Odd (secondary) MAL in a Dual MAL (pair of MALs)
 */
#define ARAD_NIF_IS_BASE_MAL(mal_id)      SOC_SAND_NUM2BOOL((mal_id) == ARAD_NIF_BASE_MAL(mal_id))

#define ARAD_NIF_SGMII_LANE(if_id) \
  ((if_id) % ARAD_MAX_NIFS_PER_WC)

#define ARAD_NIF_RXAUI_LANE(if_id) \
  (((if_id) % ARAD_MAX_RXAUIS_PER_WC)*2)

typedef enum
{
  /*
   *  Undefined or invalid.
   */
  ARAD_NIF_MODE_NONE = 0,
  /*
   *  XAUI standard 10GE interface (3.125 SerDes). 
   */
  ARAD_NIF_MODE_XAUI_STANDARD,
  /*
   *  XAUI CX4 10GE interface (3.125 SerDes). 
   */
  ARAD_NIF_MODE_XAUI_CX4,
  /*
   *  XAUI KX4 10GE interface (3.125 SerDes). 
   */
  ARAD_NIF_MODE_XAUI_KX4,
  /*
   *  XAUI+ 16GE interface (5 SerDes). 
   */
  ARAD_NIF_MODE_XAUI_PLUS_16G,
  /*
   *  XAUI+ 20GE interface (6.25 SerDes). 
   */
  ARAD_NIF_MODE_XAUI_PLUS_20G,
  /*
   *  XAUI++ 25GE interface (6.5 SerDes). 
   */
  ARAD_NIF_MODE_XAUI_PLUS_PLUS_25G,
  /*
   *  XAUI++ 40GE interface (10.9 SerDes). 
   */
  ARAD_NIF_MODE_XAUI_PLUS_PLUS_40G,
  /*
   *  SGMII 100M interface (1.25SerDes). 
   */
  ARAD_NIF_MODE_SGMII_100M,
  /*
   *  SGMII 1G interface (1.25SerDes). 
   */
  ARAD_NIF_MODE_SGMII_1G,
  /*
   *  SGMII 1000-BaseX 1G interface (1.25SerDes). 
   */
  ARAD_NIF_MODE_SGMII_1000_BASEX,
  /*
   *  SGMII+ 2.5G interface (3.125 SerDes). 
   */
  ARAD_NIF_MODE_SGMII_2_5G,
  /*
   *  RXAUI 10GE (Standard), at 6.25Gbps SerDes.
   */
  ARAD_NIF_MODE_RXAUI_10G_STANDARD,
  /*
   *  RXAUI 11GE (10.5 in fact), at 6.5Gbps SerDes.
   */
  ARAD_NIF_MODE_RXAUI_11G,
  /*
   *  RXAUI 16GE (16.4 in fact), at 10.3Gbps SerDes.
   */
  ARAD_NIF_MODE_RXAUI_16G,
  /*
   *  RXAUI 18GE (18.4 in fact), at 11.5Gbps SerDes.
   */
  ARAD_NIF_MODE_RXAUI_18G,
  /*
   *  RXAUI 20GE, at 12.5Gbps SerDes.
   */
  ARAD_NIF_MODE_RXAUI_20G,

    /* ILKN to start - Not a NIF mode, just for validation */
  ARAD_NIF_MODE_ILKN_FIRST,
  /*
   *  Interlaken standard interface, at 6.25Gbps SerDes.
   */
  ARAD_NIF_MODE_ILKN_SERDES_6_25 = ARAD_NIF_MODE_ILKN_FIRST,
  /*
   *  Interlaken standard interface, at 10.3Gbps SerDes.
   */
  ARAD_NIF_MODE_ILKN_SERDES_10_3,
  /*
   *  Interlaken standard interface, at 10.9Gbps SerDes.
   */
  ARAD_NIF_MODE_ILKN_SERDES_10_9,
  /*
   *  Interlaken standard interface, at 11.5Gbps SerDes.
   */
  ARAD_NIF_MODE_ILKN_SERDES_11_5,
  /*
   *  Interlaken standard interface, at 12.5Gbps SerDes.
   */
  ARAD_NIF_MODE_ILKN_SERDES_12_5,
    /* ILKN to end - Not a NIF mode, just for validation */
  ARAD_NIF_MODE_ILKN_LAST = ARAD_NIF_MODE_ILKN_SERDES_12_5,


    /*
     *  10G Base-R interface - XFI, at 10.3Gbps SerDes.
     */
    ARAD_NIF_MODE_10GBASE_R_XFI_10G,
    /*
     *  10G Base-R interface - SFI, at 10.3Gbps SerDes.
     */
    ARAD_NIF_MODE_10GBASE_R_SFI_10G,
    /*
     *  10G Base-R interface - KR, at 10.3Gbps SerDes.
     */
    ARAD_NIF_MODE_10GBASE_R_KR_10G,
    /*
     *  11G Base-R interface - XFI, at 11.5Gbps SerDes.
     */
    ARAD_NIF_MODE_10GBASE_R_XFI_11G,
    /*
     *  11G Base-R interface - SFI, at 11.5Gbps SerDes.
     */
    ARAD_NIF_MODE_10GBASE_R_SFI_11G,
    /*
     *  11G Base-R interface - KR, at 11.5Gbps SerDes.
     */
    ARAD_NIF_MODE_10GBASE_R_KR_11G,
    /*
     *  12G Base-R interface - XFI, at 12.5Gbps SerDes.
     */
    ARAD_NIF_MODE_10GBASE_R_XFI_12G,
    /*
     *  12G Base-R interface - SFI, at 12.5Gbps SerDes.
     */
    ARAD_NIF_MODE_10GBASE_R_SFI_12G,
    /*
     *  12G Base-R interface - KR, at 12.5Gbps SerDes.
     */
    ARAD_NIF_MODE_10GBASE_R_KR_12G,
    /*
     *  40G XLGE - XLAUI, at 10.3Gbps SerDes.
     */
    ARAD_NIF_MODE_XLGE_XLAUI_40G,
    /*
     *  40G XLGE - CR4, at 10.3Gbps SerDes.
     */
    ARAD_NIF_MODE_XLGE_CR4_40G,
    /*
     *  40G XLGE - KR4, at 10.3Gbps SerDes.
     */
    ARAD_NIF_MODE_XLGE_KR4_40G,
    /*
     *  40G XLGE - XLPPI, at 10.3Gbps SerDes.
     */
    ARAD_NIF_MODE_XLGE_XLPPI_40G,
    /*
     *  42G XLGE - XLPPI, at 10.9Gbps SerDes.
     */
    ARAD_NIF_MODE_XLGE_XLPPI_42G,
    /*
     *  45G XLGE - XLAUI, at 11.5Gbps SerDes.
     */
    ARAD_NIF_MODE_XLGE_XLAUI_45G,
    /*
     *  45G XLGE - CR4, at 11.5Gbps SerDes.
     */
    ARAD_NIF_MODE_XLGE_CR4_45G,
    /*
     *  45G XLGE - KR4, at 11.5Gbps SerDes.
     */
    ARAD_NIF_MODE_XLGE_KR4_45G,
    /*
     *  45G XLGE - XLPPI, at 11.5Gbps SerDes.
     */
    ARAD_NIF_MODE_XLGE_XLPPI_45G,
    /*
     *  48G XLGE - XLAUI, at 12.5Gbps SerDes.
     */
    ARAD_NIF_MODE_XLGE_XLAUI_48G,
    /*
     *  48G XLGE - CR4, at 12.5Gbps SerDes.
     */
    ARAD_NIF_MODE_XLGE_CR4_48G,
    /*
     *  48G XLGE - KR4, at 12.5Gbps SerDes.
     */
    ARAD_NIF_MODE_XLGE_KR4_48G,
    /*
     *  48G XLGE - XLPPI, at 12.5Gbps SerDes.
     */
    ARAD_NIF_MODE_XLGE_XLPPI_48G,


        /* CGE to start - Not a NIF mode, just for validation */
    ARAD_NIF_MODE_CGE_FIRST,
    /*
     *  CGE CAUI interface, at 10.3Gbps SerDes.
     */
    ARAD_NIF_MODE_CGE_CAUI_SERDES_10_3 = ARAD_NIF_MODE_CGE_FIRST,
    /*
     *  CGE KR10 interface, at 10.3Gbps SerDes.
     */
    ARAD_NIF_MODE_CGE_KR10_SERDES_10_3,
    /*
     *  CGE CR10 interface, at 10.3Gbps SerDes.
     */
    ARAD_NIF_MODE_CGE_CR10_SERDES_10_3,
    /*
     *  CGE CAUI interface, at 10.9Gbps SerDes.
     */
    ARAD_NIF_MODE_CGE_CAUI_SERDES_10_9,
    /*
     *  CGE KR10 interface, at 10.9Gbps SerDes.
     */
    ARAD_NIF_MODE_CGE_KR10_SERDES_10_9,
    /*
     *  CGE CR10 interface, at 10.9Gbps SerDes.
     */
    ARAD_NIF_MODE_CGE_CR10_SERDES_10_9,
    /*
     *  CGE CAUI interface, at 11.5Gbps SerDes.
     */
    ARAD_NIF_MODE_CGE_CAUI_SERDES_11_5,
    /*
     *  CGE KR10 interface, at 11.5Gbps SerDes.
     */
    ARAD_NIF_MODE_CGE_KR10_SERDES_11_5,
    /*
     *  CGE CR10 interface, at 11.5Gbps SerDes.
     */
    ARAD_NIF_MODE_CGE_CR10_SERDES_11_5,
    /*
     *  CGE CAUI interface, at 12.5Gbps SerDes.
     */
    ARAD_NIF_MODE_CGE_CAUI_SERDES_12_5,
    /*
     *  CGE KR10 interface, at 12.5Gbps SerDes.
     */
    ARAD_NIF_MODE_CGE_KR10_SERDES_12_5,
    /*
     *  CGE CR10 interface, at 12.5Gbps SerDes.
     */
    ARAD_NIF_MODE_CGE_CR10_SERDES_12_5,
        /* CGE to start - Not a NIF mode, just for validation */
    ARAD_NIF_MODE_CGE_LAST = ARAD_NIF_MODE_CGE_CR10_SERDES_12_5,

    ARAD_NIF_NOF_MODES

}ARAD_NIF_MODE;


typedef enum
{
  /*
   *  Undefined or invalid.
   */
  ARAD_NIF_TYPE_NONE = 0,
  /*
   *  XAUI standard 10GE interface (3.125 SerDes). Up to 16GE
   *  at 5Gbps SerDes.
   *  Rates: 10 / 12 / 16 / 20 / 21_SCR GE
   *  SerDes Rates: 3.125, 3.75, 5.0, 6.25, 6.5625 Gbps
   */
  ARAD_NIF_TYPE_XAUI,
  /*
   *  SGMII standard interface or 1000Base-X interface, 1GE
   *  (1.25Gbps SerDes). Up to 4GE at 5Gbps SerDes.
   *  Rates: 0.1 / 1 / 1-1000BaseX / 2.5 GE
   *  SerDes Rates: 3.125, 3.75, 5.0, 6.25, 6.5625 Gbps
   */
   ARAD_NIF_TYPE_SGMII,
  /*
   *  RXAUI standard 10GE, at 6.25Gbps SerDes.
   *  Rates: 10 / 10.5_SCR / 16 (?)/ 18 (?)/ 20 GE
   */
  ARAD_NIF_TYPE_RXAUI,
  /*
   *  Interlaken standard interface : 8-24 SerDes lanes for
   *  ILKN-A4-12 SerDes lanes for ILKN-BUp to 150GE for
   *  ILKN-24 at 6.25Gbps SerDes
   */
  ARAD_NIF_TYPE_ILKN,
    /*
     *  10G Base-R interface
     *  XFI, CFI, KR 
     */
    ARAD_NIF_TYPE_10GBASE_R,
    /*
     *  40G XLGE interface
     *  XLAUI, CR4, KR4, XLPPI 
     */
    ARAD_NIF_TYPE_40G_XLGE,
    /*
     *  100G CGE interface
     *  CAUI, KR10, CR10. 10/12 lanes 
     */
    ARAD_NIF_TYPE_100G_CGE,
    /*
     *  120G Synchronous Packer interface (Ardon)
     */
    ARAD_NIF_TYPE_TM_INTERNAL_PKT,
    /*
     *  CPU nif type
     */
    ARAD_NIF_TYPE_CPU

}ARAD_NIF_TYPE;

typedef enum
{
  /*
   *  Interlaken Interface A. Corresponds to INTERFCE_ID 5000
   *  (NIF_ID_ILKN_0). Can consume 8 - 24 SerDes lanes.
   */
  ARAD_NIF_ILKN_ID_A = ARAD_NIF_ID_ILKN_0,
  /*
   *  Interlaken Interface B. Corresponds to INTERFCE_ID 5001
   *  (NIF_ID_ILKN_1). Can consume 4 - 12 SerDes lanes
   */
  ARAD_NIF_ILKN_ID_B = ARAD_NIF_ID_ILKN_1,
  /*
   *  Number of types in ARAD_NIF_ILKN_ID
   */
  ARAD_NIF_NOF_ILKN_IDS = 2
}ARAD_NIF_ILKN_ID;

typedef enum
{
  /*
   *  CGE Interface A. Can consume 10 or 12 SerDes lanes.
   *  Corresponds to INTERFCE_ID 7000
   */
  ARAD_NIF_CGE_ID_A = ARAD_NIF_ID_CGE_0,
    /*
     *  CGE Interface B. Can consume 10 or 12 SerDes lanes.
     *  Corresponds to INTERFCE_ID 7001
     */
  ARAD_NIF_CGE_ID_B = ARAD_NIF_ID_CGE_1,
  /*
   *  Number of types in ARAD_NIF_CGE_ID
   */
  ARAD_NIF_NOF_CGE_IDS = 2
}ARAD_NIF_CGE_ID;

typedef enum
{
  /*
   *  Received broadcast packets. . Configurable:
   *  include/exclude bad packets.
   */
  ARAD_NIF_RX_BCAST_PACKETS,
  /*
   *  Received multicast packets. . Configurable:
   *  include/exclude bad packets.
   */
  ARAD_NIF_RX_MCAST_BURSTS,
  
  /*
   *  Received invalid packets ( e.g. CRC errors).
   *  Configurable: only Unicast packets/ All packets.
   */
  ARAD_NIF_RX_ERR_PACKETS,

  /* RX burst length bins, including/excluding bad bursts */

  /*
   *  Received packets, below minimal-packet-size. Units:
   *  Bytes. Configurable: include/exclude bad packets.
   */
  ARAD_NIF_RX_LEN_BELOW_MIN,
  /*
   *  Received packets, minimal-packet-size - 59 Bytes.
   *  Configurable: include/exclude bad packets.
   */
  ARAD_NIF_RX_LEN_MIN_59,
  /*
   *  Received packets, 60. Units: Bytes. Configurable:
   *  include/exclude bad packets.
   */
  ARAD_NIF_RX_LEN_60,
  /*
   *  Received packets, 0 - X Bytes. Configurable:
   *  include/exclude bad packets.
   */
  ARAD_NIF_RX_LEN_61_123,
  /*
   *  Received packets, 0 - X Bytes. Configurable:
   *  include/exclude bad packets.
   */
  ARAD_NIF_RX_LEN_124_251,
  /*
   *  Received packets, 252 - 507 Bytes. Configurable:
   *  include/exclude bad packets.
   */
  ARAD_NIF_RX_LEN_252_507,
  /*
   *  Received packets, 508 - 1019 Bytes. Configurable:
   *  include/exclude bad packets.
   */
  ARAD_NIF_RX_LEN_508_1019,
  /*
   *  Received packets, 1020 - 1514 Bytes.
   *  Configurable: include/exclude bad packets.
   */
  ARAD_NIF_RX_LEN_1020_1514,
  /*
   *  Received packets, 1515 - 1518 Bytes.
   *  Configurable: include/exclude bad packets.
   */
  ARAD_NIF_RX_LEN_1515_1518,
  /*
   *  Received packets, 1519 - 2043 Bytes.
   *  Configurable: include/exclude bad packets.
   */
  ARAD_NIF_RX_LEN_1519_2043,
  /*
   *  Received packets, 2044 - 4091 Bytes.
   *  Configurable: include/exclude bad packets.
   */
  ARAD_NIF_RX_LEN_2044_4091,
  /*
   *  Received packets, 4092 - 9212 Bytes.
   *  Configurable: include/exclude bad packets.
   */
  ARAD_NIF_RX_LEN_4092_9212,
  /*
   *  Received packets, 9213 - maximal-packet-size Bytes.
   *  Configurable: include/exclude bad packets.
   */
  ARAD_NIF_RX_LEN_9213CFG_MAX,

  /*
   *  Received packets, 1515 - maximal-packet-size Bytes.
   *  Configurable: include/exclude bad packets.
   *  The counter is exist at the HW == ARAD_NIF_TX_LEN_9213CFG_MAX + ARAD_NIF_TX_LEN_4092_9212 + ARAD_NIF_TX_LEN_2044_4091
   */
  ARAD_NIF_RX_LEN_1515CFG_MAX,

  /*
   *  Received packets, above maximal packet size.
   *  Configurable: include/exclude bad packets.
   */
  ARAD_NIF_RX_LEN_ABOVE_MAX,


  /* TX burst length bins, including/excluding bad bursts */

  /*
   *  Transmitted packets, below minimal-packet-size. Units:
   *  Bytes. Configurable: include/exclude bad packets.
   */
  ARAD_NIF_TX_LEN_BELOW_MIN,
  /*
   *  Transmitted packets, minimal-packet-size - 59 Bytes.
   *  Configurable: include/exclude bad packets.
   */
  ARAD_NIF_TX_LEN_MIN_59,
  /*
   *  Transmitted packets, 60. Units: Bytes. Configurable:
   *  include/exclude bad packets.
   */
  ARAD_NIF_TX_LEN_60,
  /*
   *  Transmitted packets, 0 - X Bytes. Configurable:
   *  include/exclude bad packets.
   */
  ARAD_NIF_TX_LEN_61_123,
  /*
   *  Transmitted packets, 0 - X Bytes. Configurable:
   *  include/exclude bad packets.
   */
  ARAD_NIF_TX_LEN_124_251,
  /*
   *  Transmitted packets, 252 - 507 Bytes. Configurable:
   *  include/exclude bad packets.
   */
  ARAD_NIF_TX_LEN_252_507,
  /*
   *  Transmitted packets, 508 - 1019 Bytes. Configurable:
   *  include/exclude bad packets.
   */
  ARAD_NIF_TX_LEN_508_1019,
  /*
   *  Transmitted packets, 1020 - 1514 Bytes.
   *  Configurable: include/exclude bad packets.
   */
  ARAD_NIF_TX_LEN_1020_1514,
  /*
   *  Transmitted packets, 1515 - 1518 Bytes.
   *  Configurable: include/exclude bad packets.
   */
  ARAD_NIF_TX_LEN_1515_1518,
  /*
   *  Transmitted packets, 1519 - 2043 Bytes.
   *  Configurable: include/exclude bad packets.
   */
  ARAD_NIF_TX_LEN_1519_2043,
  /*
   *  Transmitted packets, 2044 - 4091 Bytes.
   *  Configurable: include/exclude bad packets.
   */
  ARAD_NIF_TX_LEN_2044_4091,
  /*
   *  Transmitted packets, 4092 - 9212 Bytes.
   *  Configurable: include/exclude bad packets.
   */
  ARAD_NIF_TX_LEN_4092_9212,
  /*
   *  Transmitted packets, 9213 - maximal-packet-size Bytes.
   *  Configurable: include/exclude bad packets.
   */
  ARAD_NIF_TX_LEN_9213CFG_MAX,
  /*
   *  Transmitted broadcast packets. . Configurable:
   *  include/exclude bad packets.
   */
  ARAD_NIF_TX_BCAST_PACKETS,
  /*
   *  Transmitted multicast packets. . Configurable:
   *  include/exclude bad packets.
   */
  ARAD_NIF_TX_MCAST_BURSTS,
  
  /*
   *  Transmitted invalid packets ( e.g. CRC errors).
   *  Configurable: only Unicast packets/ All packets.
   */
  ARAD_NIF_TX_ERR_PACKETS,
  /*
   *  Received valid octets. Configurable: include/exclude bad
   *  packets.
   */
  ARAD_NIF_RX_OK_OCTETS,
  /*
   *  Transmitted valid octets. Configurable: include/exclude
   *  bad packets.
   */
  ARAD_NIF_TX_OK_OCTETS,
  /*
   *  Received valid packets. Configurable: only Unicast
   *  packets/ All packets.
   */
  ARAD_NIF_RX_OK_PACKETS,
  /*
   *  Transmitted valid packets. Configurable: only Unicast
   *  packets/ All packets.
   */
  ARAD_NIF_TX_OK_PACKETS,
  /*
   *  Received valid unicast packets.
   */
  ARAD_NIF_RX_NON_UNICAST_PACKETS,
  /*
   *  Transmitted valid unicast packets.
   */
  ARAD_NIF_TX_NON_UNICAST_PACKETS,

   /*
   *  Received packets per ilkn channel.
   */
  ARAD_NIF_RX_ILKN_PER_CHANNEL,
  /*
   *  Transmitted packets per ilkn channel.
   */
  ARAD_NIF_TX_ILKN_PER_CHANNEL,


  /*
   *  Number of types in ARAD_NIF_COUNTER_TYPE
   */
  ARAD_NIF_NOF_COUNTER_TYPES
}ARAD_NIF_COUNTER_TYPE;

/* 
 *  SyncE
 */
typedef enum
{
   /*
   *  Synchronous Ethernet signal - differential (two signals
   *  per clock) recovered clock, two differential outputs
   */
  ARAD_NIF_SYNCE_MODE_TWO_DIFF_CLK = 0,
  /*
   *  Synchronous Ethernet signal - recovered clock accompanied
   *  by a valid indication, two clk+valid outputs
   */
  ARAD_NIF_SYNCE_MODE_TWO_CLK_AND_VALID = 1,
  /*
   *  Number of types in ARAD_NIF_SYNCE_MODE
   */
  ARAD_NIF_NOF_SYNCE_MODES = 2
}ARAD_NIF_SYNCE_MODE;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
/* 
 *  ELK
 */
typedef enum
{
   /*
   *  NetLogic KBP NL88650
   */
  ARAD_NIF_ELK_TCAM_DEV_TYPE_NONE = 0,
  /*
   *  NetLogic KBP NL88650 Ax
   */
  ARAD_NIF_ELK_TCAM_DEV_TYPE_NL88650A = 1,
  /*
   *  NetLogic KBP NL88650 Bx
   */
  ARAD_NIF_ELK_TCAM_DEV_TYPE_NL88650B = 2,
  
  ARAD_NIF_ELK_TCAM_DEV_TYPE_NL88675 = 3,

  /*
   * Optimus Prime
   */
   ARAD_NIF_ELK_TCAM_DEV_TYPE_BCM52311 = 4,

   /*
   *  Number of types in ARAD_NIF_ELK_TCAM_DEV_TYPE
   */
  ARAD_NIF_ELK_NOF_TCAM_DEV_TYPE = 4
}ARAD_NIF_ELK_TCAM_DEV_TYPE;
#endif

typedef enum
{
  /*
   *  Clock Divider for the selected recovered clock rate
   *  (based on SerDes lane rate) - divide by 1
   */
  ARAD_NIF_SYNCE_CLK_DIV_1 = 0,
  /*
   *  Clock Divider for the selected recovered clock rate
   *  (based on SerDes lane rate) - divide by 2
   */
  ARAD_NIF_SYNCE_CLK_DIV_2 = 1,
  /*
   *  Clock Divider for the selected recovered clock rate
   *  (based on SerDes lane rate) - divide by 80
   */
  ARAD_NIF_SYNCE_CLK_DIV_4 = 2,
  /*
   *  Number of types in SOC_PB_NIF_SYNCE_CLK_DIV
   */
  ARAD_NIF_NOF_SYNCE_CLK_DIVS = 3
}ARAD_NIF_SYNCE_CLK_DIV;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If TRUE, the selected Recovered Clock is active.
   */
  uint8 enable;
  /*
   *  The source NIF port for the recovered clock output of
   *  the MALG. Range: NIF_ID(NIF_TYPE, 0 - 63). Note: the
   *  actual source is a single SerDes lane in the specified
   *  NIF port.
   */
  uint32 port_id;
  /*
   *  Clock Divider for the selected recovered clock.
   */
  ARAD_NIF_SYNCE_CLK_DIV clk_divider;
  /*
   *  If TRUE, automatic squelch function is enabled for the
   *  recovered clock. This function powers down the clock
   *  output whenever the link is not synced, i.e. the clock
   *  is invalid (even if the VALID indication is not present
   *  on the pin).
   */
  uint8 squelch_enable;

} ARAD_NIF_SYNCE_CLK;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If TRUE, the SyncE feature is active.
   */
  uint8 enable;
  /*
   *  Synchronous Ethernet Signal Mode
   */
  ARAD_NIF_SYNCE_MODE mode;
  /*
   *  Synchronous Ethernet configuration
   */
  ARAD_NIF_SYNCE_CLK conf[ARAD_NIF_NOF_SYNCE_CLK_IDS];

}ARAD_INIT_SYNCE;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If TRUE, the ELK feature is active.
   */
  uint8 enable;
  /*
   *  If TRUE(Just for ARAD+) the 2xCAUI +elk 4/8 mode is enabled
   */
#ifdef BCM_88660_A0
  uint8 ext_interface_mode;
  /* 
   * 0x1 : CPU_RECORD_DATA_MSB_TRIGGER=1, CPU_RECORD_DATA_MSB_TRIGGER=0 
   *       Setting IHB_CPU_RECORD_DATA_MSBr will send ROP packet
   * 0x0 : CPU_RECORD_DATA_MSB_TRIGGER=0, CPU_RECORD_DATA_MSB_TRIGGER=1 
   *       Setting IHB_CPU_RECORD_DATA_MSBr and then IHB_CPU_RECORD_DATA_LSBr will send ROP packet
   */
  uint8 msb_trigger_on[2];
	/*
	* this value represents the last set opcode, this way we can skip updating.
	*/
  uint32 last_opcode_set[2];
#endif
  /*
   *  Indicate the External lookup Device type
   */
  ARAD_NIF_ELK_TCAM_DEV_TYPE  tcam_dev_type;
  /*
   *  Forward table sizes.
   *  if 0x0 forwarding done in internal DB, else done External;
   *  Do not allow (-1) for indefinite table size.
   */
  uint32  fwd_table_size[ARAD_KBP_FRWRD_IP_NOF_TABLES];

  uint32 kbp_mdio_id[2];

  uint32 kbp_recover_enable;

  uint32 kbp_recover_iter;

#ifdef BCM_88675_A0
  uint32 kbp_no_fwd_ipv6_dip_sip_sharing_disable;
#endif

}ARAD_INIT_ELK;
#endif

#define ARAD_NIF_NOF_COUNTERS   ARAD_NIF_NOF_COUNTER_TYPES

/*********************************************************************
* NAME:
 *   arad_nif_counter_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets Value of statistics counter of the NIF.
 * INPUT:
 *   SOC_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_INTERFACE_ID             nif_ndx -
 *     NIF Port index. Range: ARAD_NIF_ID(NIF-Type, 0 - 63).
 *   SOC_SAND_IN  ARAD_NIF_COUNTER_TYPE            counter_type -
 *     Counter Type.
 *   SOC_SAND_OUT SOC_SAND_64CNT                     *counter_val -
 *     Counter Value.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_nif_counter_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  soc_port_t                  port,
    SOC_SAND_IN  ARAD_NIF_COUNTER_TYPE       counter_type,
    SOC_SAND_OUT SOC_SAND_64CNT              *counter_val
  );

/*********************************************************************
* NAME:
 *   arad_nif_all_counters_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets Value of statistics counter of the NIF.
 * INPUT:
 *   SOC_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_INTERFACE_ID             nif_ndx -
 *     NIF Port index. Range: ARAD_NIF_ID(NIF-Type, 0 - 63).
 *   SOC_SAND_IN  uint8            non_data_also -
 *     If set, reads also Pause Frames and PTP counters
 *   SOC_SAND_OUT SOC_SAND_64CNT                     counter_val -
 *     ARAD_NIF_NOF_COUNTERS] - Counters Values
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_nif_all_counters_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  soc_port_t             port,
    SOC_SAND_IN  uint8                  non_data_also,
    SOC_SAND_OUT SOC_SAND_64CNT         counter_val[ARAD_NIF_NOF_COUNTERS]
  );

/*********************************************************************
* NAME:
 *   arad_nif_all_nifs_all_counters_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets Statistics Counters for all the NIF-s in the
 *   device.
 * INPUT:
 *   SOC_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8            non_data_also -
 *     If set, reads also Pause Frames and PTP counters
 *   SOC_SAND_OUT SOC_SAND_64CNT                     counters_val -
 *     [ARAD_IF_NOF_NIFS][ARAD_NIF_NOF_COUNTERS] - Counters
 *     Values, all NIF-s.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_nif_all_nifs_all_counters_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint8                      non_data_also,
    SOC_SAND_OUT SOC_SAND_64CNT                     counters_val[ARAD_NIF_NOF_NIFS][ARAD_NIF_NOF_COUNTERS]
  );

/*********************************************************************
*     Set SyncE recovered clocks to drive on primiray recovered
*     clock output of port.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32 arad_nif_synce_clk_sel_port_set(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                      synce_cfg_num,
    SOC_SAND_IN  soc_port_t                  port);

uint32 arad_nif_synce_clk_sel_port_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                      synce_cfg_num,
    SOC_SAND_OUT soc_port_t                  *port);

/*
 *    Derive Interface type
 *  from Arad style index
 */
ARAD_NIF_TYPE
  arad_nif_id2type(
    SOC_SAND_IN  ARAD_INTERFACE_ID  arad_nif_id
  );

/*
 *    Derive Arad Interface ID
 *  from Arad style type, index
 */
ARAD_INTERFACE_ID
  arad_nif_type2id(
    SOC_SAND_IN ARAD_NIF_TYPE arad_nif_type,
    SOC_SAND_IN uint32 internal_id
  );

/*
 *    Converts from a Network interface offset (0 - 63),
 *  to ARAD-style Arad-NIF-id, given the NIF type.
 *  For example ARAD_NIF_TYPE_XAUI with offset 3 is converted to
 *  ARAD_NIF_ID_XAUI_3
 */
ARAD_INTERFACE_ID
  arad_nif_offset2nif_id(
    SOC_SAND_IN  ARAD_NIF_TYPE       nif_type,
    SOC_SAND_IN  uint32         nif_offset
  );


#if ARAD_DEBUG_IS_LVL1
const char*
  ARAD_NIF_TYPE_to_string(
    SOC_SAND_IN  ARAD_NIF_TYPE enum_val
  );

#endif /* ARAD_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


/* } __ARAD_API_NIF_INCLUDED__*/
#endif


