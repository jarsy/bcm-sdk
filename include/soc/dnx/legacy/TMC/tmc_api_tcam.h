/* $Id: jer2_jer2_jer2_tmc_api_tcam.h,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_jer2_jer2_jer2_tmcapi_tcam.h
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

#ifndef __DNX_TMC_API_TCAM_INCLUDED__
/* { */
#define __DNX_TMC_API_TCAM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>

#include <soc/dnx/legacy/TMC/tmc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Number of TCAM banks                                    */
#define  DNX_TMC_TCAM_NOF_BANKS (4)

/*     Number of TCAM cycles                                   */

#ifndef DNX_TMC_TCAM_NL_88650_MAX_NOF_ENTRIES
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
/* Max number of entries: 1 million entries of 80b for KBP NL88650 */
#define DNX_TMC_TCAM_NL_88650_MAX_NOF_ENTRIES (1 << 20)
#else  /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
/* No external TCAM entries without KBP */
#define DNX_TMC_TCAM_NL_88650_MAX_NOF_ENTRIES (2)
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
#endif /* ifndef DNX_TMC_TCAM_NL_88650_MAX_NOF_ENTRIES */

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
  DNX_TMC_TCAM_BANK_ENTRY_SIZE_72_BITS = 0,
  /*
   *  TCAM bank size of 144 bits
   */
  DNX_TMC_TCAM_BANK_ENTRY_SIZE_144_BITS = 1,
  /*
   *  TCAM bank size of 288 bits
   */
  DNX_TMC_TCAM_BANK_ENTRY_SIZE_288_BITS = 2,
  /* 
   *  TCAM bank size of 80 bits
   *  JER2_ARAD only
   */
  DNX_TMC_TCAM_BANK_ENTRY_SIZE_80_BITS = 0,
  /* 
   *  TCAM bank size of 160 bits
   *  JER2_ARAD only
   */
  DNX_TMC_TCAM_BANK_ENTRY_SIZE_160_BITS = 1,
  /* 
   *  TCAM bank size of 320 bits
   *  JER2_ARAD only
   */
  DNX_TMC_TCAM_BANK_ENTRY_SIZE_320_BITS = 2,

  /*
   *  Number of types in DNX_TMC_TCAM_BANK_ENTRY_SIZE
   */
  DNX_TMC_NOF_TCAM_BANK_ENTRY_SIZES = 3

}DNX_TMC_TCAM_BANK_ENTRY_SIZE;

typedef enum
{
  /*
   *  The TCAM User is the IPv4 multicast forwarding database
   */
  DNX_TMC_TCAM_USER_FWDING_IPV4_MC = 0,
  /*
   *  The TCAM User is the IPv6 unicast forwarding database
   */
  DNX_TMC_TCAM_USER_FWDING_IPV6_UC = 1,
  /*
   *  The TCAM User is the IPv6 multicast forwarding database
   */
  DNX_TMC_TCAM_USER_FWDING_IPV6_MC = 2,
  /*
   *  The TCAM User is the Ingress Field Processor
   */
  DNX_TMC_TCAM_USER_FP = 3,
  /*
   *  The TCAM User is the Egress ACL
   */
  DNX_TMC_TCAM_USER_EGRESS_ACL = 4,
  /*
   *  Number of types in DNX_TMC_TCAM_USER_PB
   */
  DNX_TMC_NOF_TCAM_USERS 
}DNX_TMC_TCAM_USER;

typedef enum
{
  /*
   *  The TCAM Bank owner is the Ingress PMF Cycle 0
   */
  DNX_TMC_TCAM_BANK_OWNER_PMF_0 = 0,

  /*
   *  The TCAM Bank owner is the Ingress PMF Cycle 1
   */
  DNX_TMC_TCAM_BANK_OWNER_PMF_1 = 1,

  /*
   *  The TCAM Bank owner is the FLP
   */
  DNX_TMC_TCAM_BANK_OWNER_FLP_TCAM = 2,

  /*
   *  The TCAM Bank owner is the FLP
   */
  DNX_TMC_TCAM_BANK_OWNER_FLP_TRAPS = 3,

  /* 
   *  The TCAM Bank owner is the VT
   */
  DNX_TMC_TCAM_BANK_OWNER_VT = 4,

  /* 
   *  The TCAM Bank owner is the TT
   */
  DNX_TMC_TCAM_BANK_OWNER_TT = 5,

  /*
   *  The TCAM Bank owner is the Egress ACL
   */
  DNX_TMC_TCAM_BANK_OWNER_EGRESS_ACL = 6,  

  /*
   *  Number of types in JER2_ARAD_TCAM_BANK_OWNER
   */
  DNX_TMC_TCAM_NOF_BANK_OWNERS = 7 
}DNX_TMC_TCAM_BANK_OWNER;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Prefix value. Range: 0 - 15.
   */
  uint32 val;
  /*
   *  Prefix length. Units: Bits. Range: 0 - 3.
   */
  uint32 nof_bits;

} DNX_TMC_TCAM_DB_PREFIX;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Database priority (global). Range: 0 - 16K-1.
   */
  uint32 priority;
  /*
   *  Entry size.
   */
  DNX_TMC_TCAM_BANK_ENTRY_SIZE entry_size;
  /*
   *  Prefix length
   */
  uint32 prefix_length;

} DNX_TMC_TCAM_DB_USER;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  If True, then this Database is present in the (Bank,
   *  Cycle). Is valid only if this (Bank, Cycle) is reserved
   *  to this TCAM user.
   */
  uint8 is_present[DNX_TMC_TCAM_NOF_BANKS];
  /*
   *  Database entry prefix per (Bank, Cycle). Valid only if
   *  the Database is present in this (Bank, Cycle).
   */
  DNX_TMC_TCAM_DB_PREFIX prefix[DNX_TMC_TCAM_NOF_BANKS];
  /*
   *  User-Defined Database properties
   */
  DNX_TMC_TCAM_DB_USER user;

} DNX_TMC_TCAM_DB_INFO;

/* JER2_ARAD only defines { */
typedef enum
{
  /*
   *  TCAM action size of the first 20 bits
   *  (24 bits in Jericho)
   */
  DNX_TMC_TCAM_ACTION_SIZE_FIRST_20_BITS = 0x1,
  /*
   *  TCAM action size of the second 20 bits
   *  (24 bits in Jericho)
   */
  DNX_TMC_TCAM_ACTION_SIZE_SECOND_20_BITS = 0x2,
  /*
   *  TCAM action size of the third 20 bits
   *  (24 bits in Jericho).
   *  Relevant only in case bank entry size is 320bits.
   */
  DNX_TMC_TCAM_ACTION_SIZE_THIRD_20_BITS = 0x4,
  /*
   *  TCAM action size of the forth 20 bits
   *  (24 bits in Jericho).
   *  Relevant only in case bank entry size is 320bits.
   */
  DNX_TMC_TCAM_ACTION_SIZE_FORTH_20_BITS = 0x8,
  /* 
   *  TCAM number of action size types
   */
  DNX_TMC_NOF_TCAM_ACTION_SIZES = 4
  
}DNX_TMC_TCAM_ACTION_SIZE;
/* JER2_ARAD only defines } */
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
  DNX_TMC_TCAM_DB_PREFIX_clear(
    DNX_SAND_OUT DNX_TMC_TCAM_DB_PREFIX *info
  );

void
  DNX_TMC_TCAM_DB_USER_clear(
    DNX_SAND_OUT DNX_TMC_TCAM_DB_USER *info
  );

void
  DNX_TMC_TCAM_DB_INFO_clear(
    DNX_SAND_OUT DNX_TMC_TCAM_DB_INFO *info
  );

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_TCAM_BANK_ENTRY_SIZE_to_string(
    DNX_SAND_IN  DNX_TMC_TCAM_BANK_ENTRY_SIZE enum_val
  );
const char*
  DNX_TMC_TCAM_BANK_ENTRY_SIZE_JER2_ARAD_to_string(
    DNX_SAND_IN  DNX_TMC_TCAM_BANK_ENTRY_SIZE enum_val
  );
const char*
  DNX_TMC_TCAM_USER_to_string(
    DNX_SAND_IN  DNX_TMC_TCAM_USER enum_val
  );
const char*
  DNX_TMC_TCAM_BANK_OWNER_to_string(
    DNX_SAND_IN  DNX_TMC_TCAM_BANK_OWNER enum_val
  );

void
  DNX_TMC_TCAM_DB_PREFIX_print(
    DNX_SAND_IN  DNX_TMC_TCAM_DB_PREFIX *info
  );

void
  DNX_TMC_TCAM_DB_USER_print(
    DNX_SAND_IN  DNX_TMC_TCAM_DB_USER *info
  );

void
  DNX_TMC_TCAM_DB_INFO_print(
    DNX_SAND_IN  DNX_TMC_TCAM_DB_INFO *info
  );

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_TMC_API_TCAM_INCLUDED__*/
#endif

