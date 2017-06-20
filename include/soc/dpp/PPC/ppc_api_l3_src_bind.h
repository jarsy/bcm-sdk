/* $Id: ppc_api_lag.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_l3_src_bind.h
*
* MODULE PREFIX:  soc_ppc_src_bind
*
* FILE DESCRIPTION:
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/

#ifndef __SOC_PPC_API_L3_SRC_BIND_INCLUDED__
/* { */
#define __SOC_PPC_API_L3_SRC_BIND_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* } */
/*************
 * MACROS    *
 *************/
/* { */

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

#define SOC_PPC_IP6_COMPRESSION_DIP 0x01 /* indicate DIP compression */
#define SOC_PPC_IP6_COMPRESSION_SIP 0x02 /* indicate SIP compression */
#define SOC_PPC_IP6_COMPRESSION_TCAM 0x04 /* TCAM entry  */
#define SOC_PPC_IP6_COMPRESSION_LEM  0x08 /* LEM entry */

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Source MAC address
   */
  SOC_SAND_PP_MAC_ADDRESS smac;
  /*
   *  If set to FALSE then SMAC is masked.
   */
  uint8 smac_valid;
  /*
   *  Packet SIP (Source IP address). 
   */
  uint32 sip;
  /*
   *  Logical Interface ID. 
   */
  SOC_PPC_LIF_ID lif_ndx;
  /*
   *  Number of bits to consider in the IP address starting 
   *  from the msb. Range: 0 - 32.Example for key ip_address 
   *  192.168.1.0 and prefix_len 24 would match any IP Address 
   *  of the form 192.168.1.x                                 
   */
  uint8 prefix_len;
  /*
   *  Set to TRUE means the entry is for network IP anti-spoofing.
   */
  uint8 is_network;
  
  SOC_PPC_VSI_ID  vsi_id;
} SOC_PPC_SRC_BIND_IPV4_ENTRY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Source MAC address
   */
  SOC_SAND_PP_MAC_ADDRESS smac;
  /*
   *  If set to FALSE then SMAC is masked.
   */
  uint8 smac_valid;
  /*
   *  Packet SIP (Source IPv6 address). 
   */
  SOC_SAND_PP_IPV6_ADDRESS sip6;

  /*
   *  Logical Interface ID. 
   */
  SOC_PPC_LIF_ID lif_ndx;
  /*
   *  Number of bits to consider in the IP address starting 
   *  from the msb.Range: 0 - 128.                            
   */
  uint8 prefix_len;
  /*
   *  Set to TRUE means the entry is for network IP anti-spoofing.
   */
  uint8 is_network;
} SOC_PPC_SRC_BIND_IPV6_ENTRY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /* Indicate the compression IP whether it's SIP or DIP */
  uint32 flags;
  /*  Compression IPv6 address. */
  SOC_SAND_PP_IPV6_SUBNET ip6;
  
  /*
   *  Logical Interface ID, same with FEC ID. 
   */
  SOC_PPC_LIF_ID lif_ndx;

  /* tt result compression result */
  uint16 ip6_tt_compressed_result;

  /* Ip6 comprssion result, it will be used in PMF */
  uint32 ip6_compressed_result;
  /* Indicate it's IP6 spoof compression or DIP compression  */
  uint8 is_spoof;
  
  SOC_PPC_VSI_ID  vsi_id;
} SOC_PPC_IPV6_COMPRESSED_ENTRY;

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
  SOC_PPC_SRC_BIND_IPV4_ENTRY_clear(
    SOC_SAND_OUT SOC_PPC_SRC_BIND_IPV4_ENTRY     *info
  );

void
  SOC_PPC_SRC_BIND_IPV6_ENTRY_clear(
    SOC_SAND_OUT SOC_PPC_SRC_BIND_IPV6_ENTRY     *info
  );

void
  SOC_PPC_SRC_BIND_IPV4_ENTRY_print(
    SOC_SAND_OUT SOC_PPC_SRC_BIND_IPV4_ENTRY     *info
  );

void
  SOC_PPC_SRC_BIND_IPV6_ENTRY_print(
    SOC_SAND_OUT SOC_PPC_SRC_BIND_IPV6_ENTRY     *info
  );


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_L3_SRC_BIND_INCLUDED__*/
#endif

