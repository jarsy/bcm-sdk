/* $Id: ppc_api_lag.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_lag.h
*
* MODULE PREFIX:  soc_ppc_lag
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

#ifndef __SOC_PPC_API_LAG_INCLUDED__
/* { */
#define __SOC_PPC_API_LAG_INCLUDED__

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

/*     Maximal LAG ID                                          */
#define  SOC_PPC_LAG_MAX_ID (255)

/*     Maximal number of members in LAG                        */
/* for PB: 16
   for Arad 256 */
#define  SOC_PPC_LAG_MEMBERS_MAX (256)

#define SOC_PPC_LAG_LB_CRC_0x14D         0x0
#define SOC_PPC_LAG_LB_CRC_0x1C3         0x1
#define SOC_PPC_LAG_LB_CRC_0x1CF         0x2
#define SOC_PPC_LAG_LB_KEY               0x3   /* Use LB-Key-Packet-Data directly */          
#define SOC_PPC_LAG_LB_ROUND_ROBIN       0x4   /* Use counter incremented every packet */
#define SOC_PPC_LAG_LB_2_CLOCK           0x5    /* User counter incremented every two clocks */
#define SOC_PPC_LAG_LB_CRC_0x10861       0x6
#define SOC_PPC_LAG_LB_CRC_0x10285       0x7
#define SOC_PPC_LAG_LB_CRC_0x101A1       0x8
#define SOC_PPC_LAG_LB_CRC_0x12499       0x9
#define SOC_PPC_LAG_LB_CRC_0x1F801       0xA
#define SOC_PPC_LAG_LB_CRC_0x172E1       0xB
#define SOC_PPC_LAG_LB_CRC_0x1EB21       0xC
#define SOC_PPC_LAG_LB_CRC_0x13965       0xD
#define SOC_PPC_LAG_LB_CRC_0x1698D       0xE
#define SOC_PPC_LAG_LB_CRC_0x1105D       0xF
/* Arad+ hash functions */
#define SOC_PPC_LAG_LB_CRC_0x8003       0x10
#define SOC_PPC_LAG_LB_CRC_0x8011       0x11
#define SOC_PPC_LAG_LB_CRC_0x8423       0x12
#define SOC_PPC_LAG_LB_CRC_0x8101       0x13
#define SOC_PPC_LAG_LB_CRC_0x84a1       0x14
#define SOC_PPC_LAG_LB_CRC_0x9019       0x15

/* } */
/*************
 * MACROS    *
 *************/
/* { */

#define SOC_PPC_LAG_MEMBER_INGRESS_DISABLE  0x1
#define SOC_PPC_LAG_MEMBER_EGRESS_DISABLE   0x2


/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
  /*
   *  The first header to be taken for the LAG hashing key, is
   *  the forwarding header.
   */
  SOC_PPC_LAG_HASH_FRST_HDR_FARWARDING = 0,
  /*
   *  The first header to be taken for the LAG hashing key, is
   *  the last terminated header. I. E: The header below the
   *  forwarding header.'nof_headers =1': Only the header
   *  below the forwarding header is taken'nof_headers =2':
   *  The forwarding header and the header below are
   *  taken'nof_headers =3': The forwarding header; the header
   *  below it; and the header above it are taken
   */
  SOC_PPC_LAG_HASH_FRST_HDR_LAST_TERMINATED = 1,
  /*
   *  Number of types in SOC_PPC_LAG_HASH_FRST_HDR
   */
  SOC_PPC_NOF_LAG_HASH_FRST_HDRS = 2
}SOC_PPC_LAG_HASH_FRST_HDR;

typedef enum
{
  /*
   *  The loading balance between the members of the LAG
   *  performed by hashing over the packet fields.
   */
  SOC_PPC_LAG_LB_TYPE_HASH = 0,
  /*
   *  The loading balance between the members of the LAG
   *  performed by round-robin between the memebers
   */
  SOC_PPC_LAG_LB_TYPE_ROUND_ROBIN = 1,
  /*
   *  The loading balance between the members of the LAG
   *  performed by the Smooth Division table.
   */
  SOC_PPC_LAG_LB_TYPE_SMOOTH_DIVISION = 2,
  /*
   *  Number of types in SOC_PPC_LAG_LB_TYPE
   */
  SOC_PPC_NOF_LAG_LB_TYPES = 3
}SOC_PPC_LAG_LB_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If set the In-PP-Port is used in the CRC vectors
   */
  uint8 use_port_id;
  /*
   *  Initial value for the LB key generation
   *  Soc_petra-B max value: 256
   *  Arad max value: 2^16 - 1
   */
  uint32 seed;
  /*
   *  Selects one of following options for the LB key generation:
   *  0  - Use CRC 8 with polynomial 0x14D
   *  1  - Use CRC 8 with polynomial 0x1C3
   *  2  - Use CRC 8 with polynomial 0x1CF
   *  3  - Use LB-Key-Packet-Data directly
   *  4  - Use counter incremented every packet
   *  5  - User counter incremented every two clocks
   *  6  - Use polynomial 0x10861 
   *  7  - Use polynomial 0x10285 
   *  8  - Use polynomial 0x101a1 
   *  9  - Use polynomial 0x12499 
   *  10 - Use polynomial 0x1f801  
   *  11 - Use polynomial 0x172e1 
   *  12 - Use polynomial 0x1eb21 
   *  13 - Use polynomial 0x13965 
   *  14 - Use polynomial 0x1698d 
   *  15 - Use polynomial 0x1105d 
   *  Soc_petra-B supports 0-5.
   *  Arad Supports 3-15.
   *  in Arad: cannot use same function for ECMP and LAG hashing
   */
  uint8 hash_func_id;
  /*
   *  The load balancing key is barrel shifted by this value.
   */
  uint8 key_shift;
  /*
   *  Indicate if an ELI search is performed for load balancing.
   */
  uint8 eli_search;

} SOC_PPC_LAG_HASH_GLOBAL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Number of headers to parse. Range: 0-3
   */
  uint8 nof_headers;
  /*
   *  First header to parse. May be the forwarding header, or
   *  the last terminated header.
   */
  SOC_PPC_LAG_HASH_FRST_HDR first_header_to_parse;
  /*
   *  if header type is mpls,
   *  start hashing from BOS label
   *  skipping other MPLS tunnels
   */
  uint8 start_from_bos;
  /*
   *  if header type is mpls,
   *  include BOS label in hashing key 
   *  relevant only if start_from_bos is TRUE
   */
  uint8 include_bos;

} SOC_PPC_LAG_HASH_PORT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Mask / unmask fields from the packet header. Masked
   *  fields are ignored by the hashing function
   */
  SOC_PPC_HASH_MASKS mask;
  /*
   *  Enable using the same key for the two peers of the
   *  connection. The destination and source addresses are
   *  XORed, and not taken as is. E. G: IPv4 Asymmetric key is
   *  '{IPv4-Dst-IP, IPv4-Src-IP, Protocol}' and Symmetric key
   *  is '{32'b0, (IPv4-Dst-IP XOR IPv4-Src-IP),
   *  Protocol}'. Note: symmetric key relevant only for
   *  unmasked fileds. E.g for the above example both SIP and
   *  DIP has to be unmasked to obtain actual symmetric key.
   */
  uint8 is_symmetric_key;
  /*
   *  For MPLS packetsIf set then a control word is searched
   *  for after an MPLS label with a BOS indication.
   */
  uint8 expect_cw;

} SOC_PPC_HASH_MASK_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  System Physical Port index of the port that is a member
   *  of the specified LAG. Range: 0 - 4095.
   */
  uint32 sys_port;
  /*
   *  The LAG member index. This index is not relevant for
   *  LAG-based pruning. It is embedded in the FTMH
   *  (SRC_SYS_PORT field), and can be used by the CPU. LAG
   *  Range: 0 - 15.
   */
  uint32 member_id;

  /* The Member flags */
  uint32 flags;

} SOC_PPC_LAG_MEMBER;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Number of lag members, as listed in lag_members. LAG
   *  Range: 0 - 16.
   */
  uint32 nof_entries;
  /*
   *  System Physical Port indexes of the ports that are
   *  members of the specified LAG.
   */
  SOC_PPC_LAG_MEMBER members[SOC_PPC_LAG_MEMBERS_MAX];
  /*
   *  Load balance type. According to hash over the packet
   *  fields see soc_ppd_lag_hashing_lag_info_set () or round
   *  roubin between the members.in T20E has to be by hashing.
   */
  SOC_PPC_LAG_LB_TYPE lb_type;

  /*
   * If set then the LAG group uses the SLB hashing.
   * If also the SLB soc property is set then this LAG group does SLB.
   * This should only be set if the lb_type is SOC_PPC_LAG_LB_TYPE_HASH.
   */
  uint8 is_stateful;

} SOC_PPC_LAG_INFO;


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
  SOC_PPC_LAG_HASH_GLOBAL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LAG_HASH_GLOBAL_INFO *info
  );

void
  SOC_PPC_LAG_HASH_PORT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LAG_HASH_PORT_INFO *info
  );

void
  SOC_PPC_HASH_MASK_INFO_clear(
    SOC_SAND_OUT SOC_PPC_HASH_MASK_INFO *info
  );

void
  SOC_PPC_LAG_MEMBER_clear(
    SOC_SAND_OUT SOC_PPC_LAG_MEMBER *info
  );

void
  SOC_PPC_LAG_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LAG_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_LAG_HASH_FRST_HDR_to_string(
    SOC_SAND_IN  SOC_PPC_LAG_HASH_FRST_HDR enum_val
  );

const char*
  SOC_PPC_LAG_LB_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_LAG_LB_TYPE enum_val
  );

void
  SOC_PPC_LAG_HASH_GLOBAL_INFO_print(
    SOC_SAND_IN  SOC_PPC_LAG_HASH_GLOBAL_INFO *info
  );

void
  SOC_PPC_LAG_HASH_PORT_INFO_print(
    SOC_SAND_IN  SOC_PPC_LAG_HASH_PORT_INFO *info
  );

void
  SOC_PPC_HASH_MASK_INFO_print(
    SOC_SAND_IN  SOC_PPC_HASH_MASK_INFO *info
  );

void
  SOC_PPC_LAG_MEMBER_print(
    SOC_SAND_IN  SOC_PPC_LAG_MEMBER *info
  );

void
  SOC_PPC_LAG_INFO_print(
    SOC_SAND_IN  SOC_PPC_LAG_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_LAG_INCLUDED__*/
#endif

