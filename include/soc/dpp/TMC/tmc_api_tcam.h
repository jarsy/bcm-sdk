/* $Id: tmc_api_tcam.h,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_tmcapi_tcam.h
*
* MODULE PREFIX:  soc_ppc_tcam
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

#ifndef __SOC_TMC_API_TCAM_INCLUDED__
/* { */
#define __SOC_TMC_API_TCAM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/TMC/tmc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Number of TCAM banks                                    */
#define  SOC_TMC_TCAM_NOF_BANKS (4)

/*     Number of TCAM cycles                                   */
#define  SOC_TMC_TCAM_NOF_CYCLES (2)

#ifndef SOC_TMC_TCAM_NL_88650_MAX_NOF_ENTRIES
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
/* Max number of entries: 1 million entries of 80b for KBP NL88650 */
#define SOC_TMC_TCAM_NL_88650_MAX_NOF_ENTRIES (1 << 20)
#else  /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
/* No external TCAM entries without KBP */
#define SOC_TMC_TCAM_NL_88650_MAX_NOF_ENTRIES (2)
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
#endif /* ifndef SOC_TMC_TCAM_NL_88650_MAX_NOF_ENTRIES */

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

typedef enum
{
  /*
   *  TCAM bank size of 72 bits
   */
  SOC_TMC_TCAM_BANK_ENTRY_SIZE_72_BITS = 0,
  /*
   *  TCAM bank size of 144 bits
   */
  SOC_TMC_TCAM_BANK_ENTRY_SIZE_144_BITS = 1,
  /*
   *  TCAM bank size of 288 bits
   */
  SOC_TMC_TCAM_BANK_ENTRY_SIZE_288_BITS = 2,
  /* 
   *  TCAM bank size of 80 bits
   *  ARAD only
   */
  SOC_TMC_TCAM_BANK_ENTRY_SIZE_80_BITS = 0,
  /* 
   *  TCAM bank size of 160 bits
   *  ARAD only
   */
  SOC_TMC_TCAM_BANK_ENTRY_SIZE_160_BITS = 1,
  /* 
   *  TCAM bank size of 320 bits
   *  ARAD only
   */
  SOC_TMC_TCAM_BANK_ENTRY_SIZE_320_BITS = 2,

  /*
   *  Number of types in SOC_TMC_TCAM_BANK_ENTRY_SIZE
   */
  SOC_TMC_NOF_TCAM_BANK_ENTRY_SIZES = 3

}SOC_TMC_TCAM_BANK_ENTRY_SIZE;

typedef enum
{
  /*
   *  The TCAM User is the IPv4 multicast forwarding database
   */
  SOC_TMC_TCAM_USER_FWDING_IPV4_MC = 0,
  /*
   *  The TCAM User is the IPv6 unicast forwarding database
   */
  SOC_TMC_TCAM_USER_FWDING_IPV6_UC = 1,
  /*
   *  The TCAM User is the IPv6 multicast forwarding database
   */
  SOC_TMC_TCAM_USER_FWDING_IPV6_MC = 2,
  /*
   *  The TCAM User is the Ingress Field Processor
   */
  SOC_TMC_TCAM_USER_FP = 3,
  /*
   *  The TCAM User is the Egress ACL
   */
  SOC_TMC_TCAM_USER_EGRESS_ACL = 4,
  /*
   *  Number of types in SOC_TMC_TCAM_USER_PB
   */
  SOC_TMC_NOF_TCAM_USERS 
}SOC_TMC_TCAM_USER;

typedef enum
{
  /*
   *  The TCAM Bank owner is the Ingress PMF Cycle 0
   */
  SOC_TMC_TCAM_BANK_OWNER_PMF_0 = 0,

  /*
   *  The TCAM Bank owner is the Ingress PMF Cycle 1
   */
  SOC_TMC_TCAM_BANK_OWNER_PMF_1 = 1,

  /*
   *  The TCAM Bank owner is the FLP
   */
  SOC_TMC_TCAM_BANK_OWNER_FLP_TCAM = 2,

  /*
   *  The TCAM Bank owner is the FLP
   */
  SOC_TMC_TCAM_BANK_OWNER_FLP_TRAPS = 3,

  /* 
   *  The TCAM Bank owner is the VT
   */
  SOC_TMC_TCAM_BANK_OWNER_VT = 4,

  /* 
   *  The TCAM Bank owner is the TT
   */
  SOC_TMC_TCAM_BANK_OWNER_TT = 5,

  /*
   *  The TCAM Bank owner is the Egress ACL
   */
  SOC_TMC_TCAM_BANK_OWNER_EGRESS_ACL = 6,  

  /*
   *  Number of types in ARAD_TCAM_BANK_OWNER
   */
  SOC_TMC_TCAM_NOF_BANK_OWNERS = 7 
}SOC_TMC_TCAM_BANK_OWNER;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Prefix value. Range: 0 - 15.
   */
  uint32 val;
  /*
   *  Prefix length. Units: Bits. Range: 0 - 3.
   */
  uint32 nof_bits;

} SOC_TMC_TCAM_DB_PREFIX;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Database priority (global). Range: 0 - 16K-1.
   */
  uint32 priority;
  /*
   *  Entry size.
   */
  SOC_TMC_TCAM_BANK_ENTRY_SIZE entry_size;
  /*
   *  Prefix length
   */
  uint32 prefix_length;

} SOC_TMC_TCAM_DB_USER;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If True, then this Database is present in the (Bank,
   *  Cycle). Is valid only if this (Bank, Cycle) is reserved
   *  to this TCAM user.
   */
  uint8 is_present[SOC_TMC_TCAM_NOF_BANKS];
  /*
   *  Database entry prefix per (Bank, Cycle). Valid only if
   *  the Database is present in this (Bank, Cycle).
   */
  SOC_TMC_TCAM_DB_PREFIX prefix[SOC_TMC_TCAM_NOF_BANKS];
  /*
   *  User-Defined Database properties
   */
  SOC_TMC_TCAM_DB_USER user;

} SOC_TMC_TCAM_DB_INFO;

/* ARAD only defines { */
typedef enum
{
  /*
   *  TCAM action size of the first 20 bits
   *  (24 bits in Jericho)
   */
  SOC_TMC_TCAM_ACTION_SIZE_FIRST_20_BITS = 0x1,
  /*
   *  TCAM action size of the second 20 bits
   *  (24 bits in Jericho)
   */
  SOC_TMC_TCAM_ACTION_SIZE_SECOND_20_BITS = 0x2,
  /*
   *  TCAM action size of the third 20 bits
   *  (24 bits in Jericho).
   *  Relevant only in case bank entry size is 320bits.
   */
  SOC_TMC_TCAM_ACTION_SIZE_THIRD_20_BITS = 0x4,
  /*
   *  TCAM action size of the forth 20 bits
   *  (24 bits in Jericho).
   *  Relevant only in case bank entry size is 320bits.
   */
  SOC_TMC_TCAM_ACTION_SIZE_FORTH_20_BITS = 0x8,
  /* 
   *  TCAM number of action size types
   */
  SOC_TMC_NOF_TCAM_ACTION_SIZES = 4
  
}SOC_TMC_TCAM_ACTION_SIZE;
/* ARAD only defines } */
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
  SOC_TMC_TCAM_DB_PREFIX_clear(
    SOC_SAND_OUT SOC_TMC_TCAM_DB_PREFIX *info
  );

void
  SOC_TMC_TCAM_DB_USER_clear(
    SOC_SAND_OUT SOC_TMC_TCAM_DB_USER *info
  );

void
  SOC_TMC_TCAM_DB_INFO_clear(
    SOC_SAND_OUT SOC_TMC_TCAM_DB_INFO *info
  );

#if SOC_TMC_DEBUG_IS_LVL1

const char*
  SOC_TMC_TCAM_BANK_ENTRY_SIZE_to_string(
    SOC_SAND_IN  SOC_TMC_TCAM_BANK_ENTRY_SIZE enum_val
  );
const char*
  SOC_TMC_TCAM_BANK_ENTRY_SIZE_ARAD_to_string(
    SOC_SAND_IN  SOC_TMC_TCAM_BANK_ENTRY_SIZE enum_val
  );
const char*
  SOC_TMC_TCAM_USER_to_string(
    SOC_SAND_IN  SOC_TMC_TCAM_USER enum_val
  );
const char*
  SOC_TMC_TCAM_BANK_OWNER_to_string(
    SOC_SAND_IN  SOC_TMC_TCAM_BANK_OWNER enum_val
  );

void
  SOC_TMC_TCAM_DB_PREFIX_print(
    SOC_SAND_IN  SOC_TMC_TCAM_DB_PREFIX *info
  );

void
  SOC_TMC_TCAM_DB_USER_print(
    SOC_SAND_IN  SOC_TMC_TCAM_DB_USER *info
  );

void
  SOC_TMC_TCAM_DB_INFO_print(
    SOC_SAND_IN  SOC_TMC_TCAM_DB_INFO *info
  );

#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_TCAM_INCLUDED__*/
#endif

