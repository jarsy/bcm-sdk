/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ARAD/include/arad_tcam.h
*
* MODULE PREFIX:  arad_tcam
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

#ifndef __ARAD_TCAM_INCLUDED__
/* { */
#define __ARAD_TCAM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <shared/swstate/sw_state.h>
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Utils/sand_occupation_bitmap.h>
#include <soc/dpp/SAND/Utils/sand_sorted_list.h>
#include <soc/dpp/SAND/Utils/sand_hashtable.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/TMC/tmc_api_tcam.h>

#include <soc/dpp/ARAD/arad_framework.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/arad_general.h>

#include <soc/dpp/dpp_config_defs.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* Max number of keys per Database  */
#define ARAD_PP_FP_KEY_NOF_KEYS_PER_DB_MAX       (2)

/*     The maximum length of a TCAM entry, in longs.           */
#define  ARAD_TCAM_ENTRY_MAX_LEN (10)

/*     Number of unique TCAM database prefixes.                */
#define  ARAD_TCAM_NOF_PREFIXES (16)
/*     The maximum length of a TCAM action, in longs.          */
#define  ARAD_TCAM_ACTION_MAX_LEN ((SOC_DPP_DEFS_MAX(TCAM_ACTION_WIDTH) * 4 + 31) / 32)

#define  ARAD_TCAM_ACCESS_PROFILE_INVALID (0x3F)

#define ARAD_TCAM_ACTION_SIZE_NOF_BITS        (2)

/* Move entries at bank init enable */
#define ARAD_TCAM_BANK_INIT_MOVE_ENTRIES_ENABLE                 (1)

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
/*     The maximum length of a TCAM action, in bytes for ELK actions.          */
#define  SOC_DPP_TCAM_ACTION_ELK_KBP_MAX_LEN_BYTES       (SOC_DPP_DEFS_MAX(ELK_LOOKUP_PAYLOAD_NOF_BITS) / 8)
#define  SOC_DPP_TCAM_ACTION_ELK_KBP_GET_LEN_BYTES(unit) (SOC_DPP_DEFS_GET(unit, elk_lookup_payload_nof_bits) / 8)
#define  SOC_DPP_TCAM_ACTION_ELK_KBP_MAX_LEN_LONGS       (SOC_DPP_TCAM_ACTION_ELK_KBP_MAX_LEN_BYTES / sizeof(uint32))
#define  SOC_DPP_TCAM_ACTION_ELK_KBP_GET_LEN_LONGS(unit) (SOC_DPP_TCAM_ACTION_ELK_KBP_GET_LEN_BYTES(unit) / sizeof(uint32))
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */



/* when adding more TCAM databases no need to allocate static ACCESS ID use DBAL.
   following access profiles are using old and more complex implementation */

#define ARAD_TCAM_DB_ID_NUMBER_OF_DYNAMIC_DATABASES              (30) /* this value is not HW can be updated if needed */
#define ARAD_TCAM_DB_ID_INVALID                                  (0)



#define ARAD_PP_TCAM_STATIC_ACCESS_ID_BASE                       (1 )
#define ARAD_PP_TCAM_STATIC_ACCESS_ID_SRC_BIND_IPV6              (6 )
#define ARAD_PP_TCAM_STATIC_ACCESS_ID_OAM_IDENTIFICATION         (7 ) 
#define ARAD_PP_TCAM_STATIC_ACCESS_ID_SIP                        (9 )

/* TCAM DB profiles for VTT */
#define ARAD_PP_ISEM_ACCESS_TCAM_BASE                                   (11)
#define ARAD_PP_ISEM_ACCESS_TCAM_MPLS_FRR_DB_PROFILE                    (ARAD_PP_ISEM_ACCESS_TCAM_BASE)
#define ARAD_PP_ISEM_ACCESS_TCAM_IPV6_SPOOF_DB_PROFILE                  (12) 
#define ARAD_PP_ISEM_ACCESS_TCAM_IPV4_TT_ETH_DB_PROFILE                 (13) 
#define ARAD_PP_ISEM_ACCESS_TCAM_IPV6_TT_DB_PROFILE                     (14) 
#define ARAD_PP_ISEM_ACCESS_TCAM_ETH_INNER_OUTER_PCP_DB_PROFILE         (15) 
#define ARAD_PP_ISEM_ACCESS_TCAM_PON_EXTEND_LKP_DB_PROFILE              (16) 
#define ARAD_PP_ISEM_ACCESS_TCAM_SRC_PORT_DA_DB_PROFILE                 (17) 
#define ARAD_PP_ISEM_ACCESS_TCAM_MPLS_EXPLICIT_NULL_VT_DB_PROFILE       (18) 
#define ARAD_PP_ISEM_ACCESS_TCAM_TT_TRILL_TRANSPARENT_SERVICE           (19) 
#define ARAD_PP_ISEM_ACCESS_TCAM_TT_DIP_COMPRESSION_DB_PROFILE          (20) 
#define ARAD_PP_ISEM_ACCESS_TCAM_TT_IPV6_SPOOF_COMPRESSION              (21) 
#define ARAD_PP_ISEM_ACCESS_TCAM_TST2_PROFILE                           (22) 
#define ARAD_PP_ISEM_ACCESS_TCAM_TEST2_PROFILE                          (23) 
#define ARAD_PP_ISEM_ACCESS_TCAM_DIP_SIP_VRF_PROFILE                    (24) 
#define ARAD_PP_ISEM_ACCESS_TCAM_IPV4_MATCH_VT_DB_PROFILE               (25) 
#define ARAD_PP_ISEM_ACCESS_VT_CLASSIFICATIONS_EFP                      (26) 
#define ARAD_PP_ISEM_ACCESS_TCAM_VRRP_VSI_DA_PROFILE                    (27)
#define ARAD_PP_TT_LAST                                                 ARAD_PP_ISEM_ACCESS_TCAM_VRRP_VSI_DA_PROFILE + 10

/* empty tcam IDs that are reserved for dynamic allocation by DBAL */
#define ARAD_PP_TCAM_DYNAMIC_ACCESS_ID_BASE                         (ARAD_PP_TT_LAST+1)
#define ARAD_PP_TCAM_DYNAMIC_ACCESS_ID_END                          (ARAD_PP_TCAM_DYNAMIC_ACCESS_ID_BASE + ARAD_TCAM_DB_ID_NUMBER_OF_DYNAMIC_DATABASES)

#define ARAD_PP_ISEM_ACCESS_TCAM_END                                 (ARAD_PP_TCAM_DYNAMIC_ACCESS_ID_END)




/* skip first ones as reserved for IPMC routing and VTT */
#define ARAD_PP_FP_DB_ID_TO_TCAM_DB_SHIFT  (ARAD_PP_ISEM_ACCESS_TCAM_END + 0x1)
#define ARAD_PP_FP_DB_ID_TO_TCAM_DB(fp_id)  ((fp_id) + ARAD_PP_FP_DB_ID_TO_TCAM_DB_SHIFT)
#define ARAD_PP_FP_TCAM_DB_TO_FP_ID(tcam_db_id)  ((tcam_db_id) - ARAD_PP_FP_DB_ID_TO_TCAM_DB_SHIFT)

#define ARAD_TCAM_MAX_NOF_ACLS  128

#define ARAD_TCAM_MAX_NOF_LISTS   ARAD_PP_FP_DB_ID_TO_TCAM_DB(ARAD_TCAM_MAX_NOF_ACLS)

#define ARAD_TCAM_DB_LIST_KEY_SIZE                               (sizeof(uint32) /* for priority */)
#define ARAD_TCAM_DB_LIST_DATA_SIZE                              (sizeof(ARAD_TCAM_PRIO_LOCATION))

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

#define ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS       SOC_TMC_TCAM_BANK_ENTRY_SIZE_80_BITS
#define ARAD_TCAM_BANK_ENTRY_SIZE_160_BITS      SOC_TMC_TCAM_BANK_ENTRY_SIZE_160_BITS
#define ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS      SOC_TMC_TCAM_BANK_ENTRY_SIZE_320_BITS
#define ARAD_TCAM_BANK_ENTRY_SIZE_DIRECT_TABLE  SOC_TMC_TCAM_BANK_ENTRY_SIZE_DIRECT_TABLE
#define ARAD_TCAM_NOF_BANK_ENTRY_SIZES          (SOC_TMC_NOF_TCAM_BANK_ENTRY_SIZES + 1)
typedef SOC_TMC_TCAM_BANK_ENTRY_SIZE            ARAD_TCAM_BANK_ENTRY_SIZE;

#define ARAD_TCAM_BANK_KEY_SIZE_IN_BITS_DIRECT_TABLE_SMALL      7

#define ARAD_TCAM_BANK_KEY_SIZE_IN_BITS_80      80
#define ARAD_TCAM_BANK_KEY_SIZE_IN_BITS_160     160
#define ARAD_TCAM_BANK_KEY_SIZE_IN_BITS_320     320

/* Convert entry size ID to number of bits */
#define ARAD_SW_DB_ENTRY_SIZE_ID_TO_BITS(d_size_id)     \
  ((d_size_id == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS ) ? ARAD_TCAM_BANK_KEY_SIZE_IN_BITS_80 :     \
  (d_size_id == ARAD_TCAM_BANK_ENTRY_SIZE_160_BITS)  ? ARAD_TCAM_BANK_KEY_SIZE_IN_BITS_160:     \
  (d_size_id == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS)  ? ARAD_TCAM_BANK_KEY_SIZE_IN_BITS_320: 0) 

#define ARAD_SW_HW_ENTRY_SIZE_ID_TO_SW_ENTRY_SIZE(d_size_id) \
  ((d_size_id == 0x0 ) ? ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS : \
  (d_size_id == 0x1)  ? ARAD_TCAM_BANK_ENTRY_SIZE_160_BITS:  \
  (d_size_id == 0x3)  ? ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS: ARAD_TCAM_NOF_BANK_ENTRY_SIZES) 

#define ARAD_SW_PREFIX_AND_TO_PREFIX_LENGTH(d_and) \
  ((d_and == 0x0 ) ? 4 :    \
   (d_and == 0x1)  ? 3 :    \
   (d_and == 0x3)  ? 2 :    \
   (d_and == 0x7)  ? 1 : 0) 

#define ARAD_TCAM_ACTION_SIZE_FIRST_20_BITS                  SOC_TMC_TCAM_ACTION_SIZE_FIRST_20_BITS
#define ARAD_TCAM_ACTION_SIZE_SECOND_20_BITS                 SOC_TMC_TCAM_ACTION_SIZE_SECOND_20_BITS
#define ARAD_TCAM_ACTION_SIZE_THIRD_20_BITS                  SOC_TMC_TCAM_ACTION_SIZE_THIRD_20_BITS
#define ARAD_TCAM_ACTION_SIZE_FORTH_20_BITS                  SOC_TMC_TCAM_ACTION_SIZE_FORTH_20_BITS
#define ARAD_TCAM_NOF_ACTION_SIZES                           SOC_TMC_NOF_TCAM_ACTION_SIZES
typedef SOC_TMC_TCAM_ACTION_SIZE                                 ARAD_TCAM_ACTION_SIZE;

#define ARAD_TCAM_BANK_OWNER_PMF_0            SOC_TMC_TCAM_BANK_OWNER_PMF_0
#define ARAD_TCAM_BANK_OWNER_PMF_1            SOC_TMC_TCAM_BANK_OWNER_PMF_1
#define ARAD_TCAM_BANK_OWNER_FLP_TCAM         SOC_TMC_TCAM_BANK_OWNER_FLP_TCAM
#define ARAD_TCAM_BANK_OWNER_FLP_TRAPS        SOC_TMC_TCAM_BANK_OWNER_FLP_TRAPS
#define ARAD_TCAM_BANK_OWNER_VT               SOC_TMC_TCAM_BANK_OWNER_VT
#define ARAD_TCAM_BANK_OWNER_TT               SOC_TMC_TCAM_BANK_OWNER_TT
#define ARAD_TCAM_BANK_OWNER_EGRESS_ACL       SOC_TMC_TCAM_BANK_OWNER_EGRESS_ACL
#define ARAD_TCAM_NOF_BANK_OWNERS             SOC_TMC_TCAM_NOF_BANK_OWNERS
typedef SOC_TMC_TCAM_BANK_OWNER               ARAD_TCAM_BANK_OWNER;

typedef SOC_TMC_TCAM_DB_PREFIX                                 ARAD_TCAM_DB_PREFIX;
typedef SOC_TMC_TCAM_DB_USER                                   ARAD_TCAM_DB_USER;
typedef SOC_TMC_TCAM_DB_INFO                                   ARAD_TCAM_DB_INFO;

/* Value of bank owner as interpreted by HW */
typedef enum
{
    /* HW bank ownwer is PMF */
    ARAD_TCAM_HW_BANK_OWNER_PMF  = 0x0,

    /* HW bank ownwer is FLP */
    ARAD_TCAM_HW_BANK_OWNER_FLP  = 0x1,

    /* HW bank ownwer is VTT */
    ARAD_TCAM_HW_BANK_OWNER_VTT  = 0x2,

    /* HW bank ownwer is ERPP */
    ARAD_TCAM_HW_BANK_OWNER_ERPP = 0x3,

    /* Number of hw bank owners */
    ARAD_TCAM_NOF_HW_BANK_OWNER

} ARAD_TCAM_HW_BANK_OWNER;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The bits comprising the database's prefix, aligned to
   *  the LSB.
   */
  uint32 bits;
  /*
   *  The number of LSBs in 'bits' comprising the database's
   *  prefix.
   */
  uint32 length;

} ARAD_TCAM_PREFIX;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The TCAM bank in which the specified location resides.
   */
  uint32 bank_id;
  /*
   *  The specified location's entry in the appropriate TCAM
   *  bank.
   */
  uint32 entry;

} ARAD_TCAM_LOCATION;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The lowest location in the range, inclusive.
   */
  ARAD_TCAM_LOCATION min;

  /* If True, then no min was found */
  uint8 min_not_found;

  /*
   *  The highest location in the range, inclusive.
   */
  ARAD_TCAM_LOCATION max;

  /* If True, then no max was found */
  uint8 max_not_found;

} ARAD_TCAM_RANGE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /* If true the entry is configured valid */
  uint8 valid;
  /*
   *  The entry's value.
   */
  uint32 value[ARAD_TCAM_ENTRY_MAX_LEN];
  /*
   *  The entry's mask.
   */
  uint32 mask[ARAD_TCAM_ENTRY_MAX_LEN];

  /* 
   * If true then the entry already exists
   * and only its content is updated 
   */ 
  uint8 is_for_update;
  /* 
   * If True, then the entry is inserted in the top of 
   * the bank and not in the middle 
   * Relevant for Arad when inserting from Field group 
   * install and not entry install (many entries in a row 
   * sorted by decreasing priority) 
   */
  uint8 is_inserted_top;

} ARAD_TCAM_ENTRY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /* 
   *  The action's value
   */
  uint32 value[ARAD_TCAM_ACTION_MAX_LEN];

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  /* For ELK type, ELK action value (Associated Data) */
  uint8 elk_ad_value[SOC_DPP_TCAM_ACTION_ELK_KBP_MAX_LEN_BYTES];
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
} ARAD_TCAM_ACTION;

typedef struct
{
  /*
   * TCAM Database Id
   */
  uint32 tcam_db_id;
  /*
   * TCAM Entry SW Id
   */
  uint32 entry_id;
  /* 
   * TCAM Entry SW priority
   */
  uint32 priority;

} ARAD_TCAM_GLOBAL_LOCATION;

typedef struct
{
  /*
   *  First entry-id in the priority group
   */
  uint32 entry_id_first;
  /*
   *  Last entry-id in the priority group
   */
  uint32 entry_id_last;

} ARAD_TCAM_PRIO_LOCATION;

typedef uint32
  (*ARAD_TCAM_MGMT_SIGNAL)(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint32  user_data
  );

typedef enum
{
    /* 
     * Allow small banks as well as large banks.
     */
    ARAD_TCAM_SMALL_BANKS_ALLOW = 0,
    /* 
     * Force small banks to be used for certain DB.
     */
    ARAD_TCAM_SMALL_BANKS_FORCE = 1,
    /* 
     * Forbid small bank to be used - used in case 
     * of DT and large key (key size > 7 bits).
     */
    ARAD_TCAM_SMALL_BANKS_FORBID = 2
    
} ARAD_TCAM_SMALL_BANKS;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /* 
   * The minimum number of banks this database requires.
   */
  uint32 min_banks;
  /* 
   * The entry size of the database's entries.
   */
  ARAD_TCAM_BANK_ENTRY_SIZE entry_size;
  /* 
   * Number of bits in the database's prefix. Range: 0 - 4.
   */
  uint32 prefix_size;
  /* 
   * The action size of the database's action table.
   */
  ARAD_TCAM_ACTION_SIZE action_bitmap_ndx;
  /* 
   * Used to signal the owner of the database when the
   * database's topology is changed.
   */
  ARAD_TCAM_MGMT_SIGNAL callback;
  /* 
   * A user-defined parameter that will be passed to the callback function.
   */
  uint32 user_data;
  /* 
   * Identify bank owner 
   */
  ARAD_TCAM_BANK_OWNER bank_owner;
  /* 
   * Is direct access to TCAM action without TCAM lookup
   */
  uint8 is_direct;
  /* List of Databases forbidden to share a TCAM Bank with this one */
  uint32  forbidden_dbs[ARAD_BIT_TO_U32(ARAD_TCAM_MAX_NOF_LISTS)];

  /* 
   * Enum which states the wanted use of the small banks. 
   * TCAM Database can be limited to small banks. Also, the 
   * use of small banks can be forbidden (e.g. when direct table 
   * and key size is bigger than 7 bits).
   */
  ARAD_TCAM_SMALL_BANKS use_small_banks;

  /* 
   * If True, then the TCAM Database does not respect the BCM rule . 
   * for the entry insertion order when same priority                                                              .
   */
  uint8 no_insertion_priority_order;

  /* 
   * If True, then the TCAM Database has only few priorities and                                              .
   * acts liek in IP TCAM, using the old method                                                                                                         .
   */
  uint8 sparse_priorities;

} ARAD_TCAM_ACCESS_INFO;

typedef struct
{
  uint8                         valid;
  uint8                         has_direct_table;
  ARAD_TCAM_BANK_ENTRY_SIZE     entry_size;
  uint32                        nof_entries_free;
} ARAD_SW_DB_TCAM_BANK;

typedef struct
{
  uint8                     valid;
  ARAD_TCAM_ACTION_SIZE     action_bitmap_ndx;
  ARAD_TCAM_BANK_ENTRY_SIZE entry_size;
  uint32                    prefix_size;
  uint8                     bank_used[SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS];
  uint8                     is_direct;
  ARAD_TCAM_SMALL_BANKS     use_small_banks;  
  uint8                     no_insertion_priority_order;  
  uint8                     sparse_priorities;  
  ARAD_TCAM_PREFIX          prefix;  
  uint32                    access_profile_id[ARAD_PP_FP_KEY_NOF_KEYS_PER_DB_MAX];
  /* List of Databases forbidden to share a TCAM Bank with this one */
  SHR_BITDCL                forbidden_dbs[ARAD_BIT_TO_U32(ARAD_TCAM_MAX_NOF_LISTS)];
  /* number of entries per bank */
  uint32                    bank_nof_entries[SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS];  
} ARAD_SW_DB_TCAM_DB;

typedef struct
{
  SOC_SAND_OCC_BM_PTR       tcam_bank_entries_used[SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS][2]; /* 0 - regular, 1-its inverse */
  SOC_SAND_SORTED_LIST_PTR  tcam_db_priorities[ARAD_TCAM_MAX_NOF_LISTS];
  SOC_SAND_OCC_BM_PTR       tcam_db_entries_used[ARAD_TCAM_MAX_NOF_LISTS][SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS]; 
  SOC_SAND_HASH_TABLE_PTR   entry_id_to_location; /* Given DB and entry id, get an hash id for the next location table */
  ARAD_SW_DB_TCAM_BANK bank[SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS];
   
  ARAD_SW_DB_TCAM_DB   tcam_db[ARAD_TCAM_MAX_NOF_LISTS];  
  PARSER_HINT_ARR ARAD_TCAM_LOCATION        *db_location_tbl; /* Given DB and entry, retrieve the location */
  PARSER_HINT_ARR ARAD_TCAM_GLOBAL_LOCATION *global_location_tbl; /* Given location, retrieve DB, entry-id and priority */
} ARAD_TCAM;

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


uint32
    arad_tcam_db_prio_list_priority_value_decode(
        SOC_SAND_IN  uint8      *data
    );

uint32
    arad_tcam_db_use_new_method_per_db_get(
      SOC_SAND_IN  int                        unit,
      SOC_SAND_IN  uint32                     tcam_db_id,
      SOC_SAND_OUT uint8                      *move_per_block
    );

void
    arad_tcam_db_location_decode_print(
      SOC_SAND_IN  int                unit,
      SOC_SAND_IN uint8  *buffer
    );

uint32
    arad_tcam_bank_entry_size_to_entry_count_get(
        SOC_SAND_IN  int                       unit,
        SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size,
        SOC_SAND_IN  uint32                    bank_ndx
     );

  uint32
    arad_tcam_db_data_structure_entry_get(
      SOC_SAND_IN  int prime_handle,
      SOC_SAND_IN  uint32 sec_handle,
      SOC_SAND_IN  uint8  *buffer,
      SOC_SAND_IN  uint32 offset,
      SOC_SAND_IN  uint32 len,
      SOC_SAND_OUT uint8  *data
    );
  
  uint32
    arad_tcam_db_data_structure_entry_set(
      SOC_SAND_IN    int prime_handle,
      SOC_SAND_IN    uint32 sec_handle,
      SOC_SAND_INOUT uint8  *buffer,
      SOC_SAND_IN    uint32 offset,
      SOC_SAND_IN    uint32 len,
      SOC_SAND_IN    uint8  *data
    );

  int32
    arad_tcam_db_priority_list_cmp_priority(
      SOC_SAND_IN uint8  *buffer1,
      SOC_SAND_IN uint8  *buffer2,
              uint32 size
    );

  int32
    arad_tcam_db_priority_list_cmp_interlaced(
      SOC_SAND_IN uint8  *buffer1,
      SOC_SAND_IN uint8  *buffer2,
              uint32 size
    );


uint32
  arad_tcam_db_entry_id_to_location_entry_get(
    SOC_SAND_IN  int        unit,
    SOC_SAND_IN  uint32        tcam_db_id,
    SOC_SAND_IN  uint32        entry_id,
    SOC_SAND_IN  uint32        isLocation,
    SOC_SAND_OUT ARAD_TCAM_LOCATION *location,
    SOC_SAND_OUT uint8        *found
  );

uint32
  arad_tcam_db_entry_id_to_location_entry_add(
    SOC_SAND_IN int        unit,
    SOC_SAND_IN uint32        tcam_db_id,
    SOC_SAND_IN uint32        entry_id,
    SOC_SAND_IN ARAD_TCAM_LOCATION *location
  );

uint32
  arad_tcam_db_entry_id_to_location_entry_remove(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint32 tcam_db_id,
    SOC_SAND_IN uint32 entry_id
  );

/*********************************************************************
* NAME:
 *   arad_tcam_resource_db_entries_find
 * TYPE:
 *   PROC
 * FUNCTION:
 *   This function counts the number of DBs per TCAM bank.
 * INPUT:
 *   SOC_SAND_IN  int               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32               tcam_db_id -
 *     The ID of the TCAM database.
 *   SOC_SAND_IN  uint32 db_nof_entries[ARAD_TCAM_NOF_BANKS] -
 *     An array of number of TCAM databases per TCAM bank.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tcam_resource_db_entries_find(
      SOC_SAND_IN  int     unit,
      SOC_SAND_IN  uint32     tcam_db_id,
      SOC_SAND_OUT uint32     *db_nof_entries
  );
uint32
  arad_tcam_resource_db_entries_validate(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  uint32 bank_id,
    SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size,
	  SOC_SAND_IN  uint32 address,
    SOC_SAND_OUT ARAD_TCAM_ENTRY *entry
  );
uint32
  arad_tcam_resource_db_entries_priority_validate(
      SOC_SAND_IN  int        unit,
      SOC_SAND_IN  uint32        tcam_db_id,
      SOC_SAND_IN  ARAD_TCAM_LOCATION *location,
      SOC_SAND_OUT uint32        *entry_id,
      SOC_SAND_OUT uint32         *priority
  );

uint32
    arad_tcam_global_location_encode(
      SOC_SAND_IN  int                unit,
      SOC_SAND_IN  ARAD_TCAM_LOCATION *location,
      SOC_SAND_OUT uint32              *global_location_id
    );

uint32
  arad_tcam_entry_rewrite(
      SOC_SAND_IN       int                          unit,
      SOC_SAND_IN       uint8                        entry_exists,      
      SOC_SAND_IN       uint32                       tcam_db_id,
      SOC_SAND_IN       ARAD_TCAM_LOCATION           *location,
      SOC_SAND_IN       ARAD_TCAM_BANK_ENTRY_SIZE    entry_size,
      SOC_SAND_INOUT    ARAD_TCAM_ENTRY              *entry
  );

/*********************************************************************
* NAME:
 *   arad_tcam_bank_init_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   This function initializes a TCAM bank.
 * INPUT:
 *   SOC_SAND_IN  int               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32               bank_id -
 *     The ID of the bank to initialize.
 *   SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size -
 *     The entry size to initialize the bank to.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tcam_bank_init_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               bank_id,
    SOC_SAND_IN  uint8                is_direct,
    SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size,
    SOC_SAND_IN  ARAD_TCAM_BANK_OWNER bank_owner
  );

uint32
  arad_tcam_bank_init_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               bank_id,
    SOC_SAND_IN  uint8                is_direct,
    SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size,
    SOC_SAND_IN  ARAD_TCAM_BANK_OWNER bank_owner
  );

/*********************************************************************
* NAME:
 *   arad_tcam_db_create_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Creates a new TCAM database.
 * INPUT:
 *   SOC_SAND_IN  int               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32               tcam_db_id -
 *     ID of the database to create.
 *   SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size -
 *     The entry size of the database's entries.
 *   SOC_SAND_IN  uint32               prefix_size -
 *     Number of bits in the database's prefix. Range: 0 - 4.
 *   SOC_SAND_IN  ARAD_TCAM_ACTION_SIZE     action_bitmap_ndx -
 *     The action size of the database's action table
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tcam_db_create_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size,
    SOC_SAND_IN  uint32               prefix_size,
    SOC_SAND_IN  ARAD_TCAM_ACTION_SIZE     action_bitmap_ndx,
    SOC_SAND_IN  uint8                     use_small_banks,
    SOC_SAND_IN  uint8                     no_insertion_priority_order,
    SOC_SAND_IN  uint8                     sparse_priorities,
    SOC_SAND_IN  uint8                     is_direct
  );

uint32
  arad_tcam_db_create_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size,
    SOC_SAND_IN  uint32               prefix_size,
    SOC_SAND_IN  ARAD_TCAM_ACTION_SIZE     action_bitmap_ndx,
    SOC_SAND_IN  uint8                     is_direct
  );

/*********************************************************************
* NAME:
 *   arad_tcam_db_destroy_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Destroys a TCAM database and frees the resources
 *   allocated to it.
 * INPUT:
 *   SOC_SAND_IN  int               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32               tcam_db_id -
 *     ID of the database to destroy.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tcam_db_destroy_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id
  );

uint32
  arad_tcam_db_destroy_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id
  );

/*********************************************************************
* NAME:
 *   arad_tcam_db_bank_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Adds a TCAM bank to a database.
 * INPUT:
 *   SOC_SAND_IN  int               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32               tcam_db_id -
 *     ID of the database to that will receive the new bank.
 *   SOC_SAND_IN  uint32               bank_id -
 *     The bank that will be added to the database.
 *   SOC_SAND_IN  ARAD_TCAM_PREFIX          *prefix -
 *     The database's prefix in the new bank.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tcam_db_bank_add_unsafe(
    SOC_SAND_IN  int             unit,
    SOC_SAND_IN  uint32             tcam_db_id,
    SOC_SAND_IN  uint32             access_profile_array_id, /* in case of 320b DB, which access profile */
    SOC_SAND_IN  uint32             bank_id,
    SOC_SAND_IN  uint8              is_direct, /* in case of FP */
    SOC_SAND_IN  ARAD_TCAM_PREFIX   *prefix
  );

uint32
  arad_tcam_db_bank_add_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               bank_id,
    SOC_SAND_IN  ARAD_TCAM_PREFIX          *prefix
  );

/*********************************************************************
* NAME:
 *   arad_tcam_db_bank_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Removes a bank from the database's resource pool.
 * INPUT:
 *   SOC_SAND_IN  int               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32               tcam_db_id -
 *     ID of the database.
 *   SOC_SAND_IN  uint32               bank_id -
 *     ID of the bank to remove.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tcam_db_bank_remove_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               bank_id
  );

uint32
  arad_tcam_db_bank_remove_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               bank_id
  );

/*********************************************************************
* NAME:
 *   arad_tcam_db_nof_banks_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the number of banks occupied by the database.
 * INPUT:
 *   SOC_SAND_IN  int               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32               tcam_db_id -
 *     ID of the database to query.
 *   SOC_SAND_OUT uint32               *nof_banks -
 *     The number of banks occupied by the database.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tcam_db_nof_banks_get_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_OUT uint32               *nof_banks
  );

uint32
  arad_tcam_db_nof_banks_get_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id
  );

/*********************************************************************
* NAME:
 *   arad_tcam_db_bank_prefix_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the database's prefix in the bank.
 * INPUT:
 *   SOC_SAND_IN  int               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32               tcam_db_id -
 *     ID of the database to query.
 *   SOC_SAND_IN  uint32               bank_id -
 *     The bank to query.
 *   SOC_SAND_OUT ARAD_TCAM_PREFIX          *prefix -
 *     The database's prefix in the bank.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tcam_db_bank_prefix_get_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               bank_id,
    SOC_SAND_OUT ARAD_TCAM_PREFIX          *prefix
  );

uint32
  arad_tcam_db_bank_prefix_get_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id
  );

/*********************************************************************
* NAME:
 *   arad_tcam_db_entry_size_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the database's entry size.
 * INPUT:
 *   SOC_SAND_IN  int               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32               tcam_db_id -
 *     ID of the database to query.
 *   SOC_SAND_OUT ARAD_TCAM_BANK_ENTRY_SIZE *entry_size -
 *     The database's entry_size.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tcam_db_entry_size_get_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_OUT ARAD_TCAM_BANK_ENTRY_SIZE *entry_size
  );

uint32
  arad_tcam_db_entry_size_get_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id
  );

/*********************************************************************
* NAME:
 *   arad_tcam_db_direct_tbl_entry_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add an entry to a direct table database.
 * INPUT:
 *   SOC_SAND_IN  int               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32               bank_id -
 *     ID of the bank.
 *   SOC_SAND_IN  uint32               address -
 *     The new entry's address.
 *   SOC_SAND_IN  ARAD_TCAM_ACTION_SIZE action_bitmap_ndx -
 *     The action table bitmap.
 *   SOC_SAND_IN  ARAD_TCAM_ACTION    *action -
 *     The action associated with the new entry.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tcam_db_direct_tbl_entry_set_unsafe(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  uint32                   tcam_db_id,
      SOC_SAND_IN  uint32                   bank_id,
      SOC_SAND_IN  uint32                   address,
      SOC_SAND_IN  uint8                    valid,
      SOC_SAND_IN  ARAD_TCAM_ACTION_SIZE    action_bitmap_ndx,      
      SOC_SAND_INOUT  ARAD_TCAM_ACTION      *action
  );

uint32
  arad_tcam_db_direct_tbl_entry_set_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 bank_id,
    SOC_SAND_IN  uint32                 entry_id,
    SOC_SAND_IN  ARAD_TCAM_ACTION_SIZE  action_bitmap_ndx,      
    SOC_SAND_IN  ARAD_TCAM_ACTION       *action
  );

uint32
  arad_tcam_db_direct_tbl_entry_get_unsafe(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  uint32                   tcam_db_id,
      SOC_SAND_IN  uint32                   bank_id,
      SOC_SAND_IN  uint32                   address,
      SOC_SAND_IN  ARAD_TCAM_ACTION_SIZE    action_bitmap_ndx,      
      SOC_SAND_INOUT  ARAD_TCAM_ACTION      *action,
      SOC_SAND_OUT  uint8                   *valid
  );


/**/

/*********************************************************************
* NAME:
 *   arad_tcam_db_entry_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add an entry to a database.
 * INPUT:
 *   SOC_SAND_IN  int               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32               tcam_db_id -
 *     ID of the database.
 *   SOC_SAND_IN  uint32               entry_id -
 *     The new entry's ID.
 *   SOC_SAND_IN  uint32                priority -
 *     The new entry's priority. Range: 0 - 4294967295.
 *   SOC_SAND_IN  ARAD_TCAM_ENTRY           *entry -
 *     The new entry's data.
 *   SOC_SAND_IN  ARAD_TCAM_ACTION          *action -
 *     The action associated with the new entry.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE    *success -
 *     Indication whether the insertion succeeded or not.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tcam_db_entry_add_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               entry_id,
    SOC_SAND_IN  uint32                priority,
    SOC_SAND_IN  ARAD_TCAM_ENTRY       *entry,
    SOC_SAND_IN  ARAD_TCAM_ACTION      *action,    
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE    *success
  );

uint32
  arad_tcam_db_entry_add_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               entry_id,
    SOC_SAND_IN  uint32               priority,
    SOC_SAND_IN  ARAD_TCAM_ENTRY           *entry,
    SOC_SAND_IN  ARAD_TCAM_ACTION      *action
  );

/*********************************************************************
* NAME:
 *   arad_tcam_db_entry_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Reads an entry from the database.
 * INPUT:
 *   SOC_SAND_IN  int               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32               tcam_db_id -
 *     ID of the database.
 *   SOC_SAND_IN  uint32               entry_id -
 *     The entry's ID.
 *   SOC_SAND_OUT uint32                *priority -
 *     The entry's priority.
 *   SOC_SAND_OUT ARAD_TCAM_ENTRY           *entry -
 *     The entry's data.
 *   SOC_SAND_OUT ARAD_TCAM_ACTION          *action -
 *     The action associated with the entry.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tcam_db_entry_get_unsafe(
    SOC_SAND_IN  int              unit,
    SOC_SAND_IN  uint32              tcam_db_id,
    SOC_SAND_IN  uint32              entry_id,
    SOC_SAND_IN  uint8               hit_bit_clear, /* If TRUE, clear the hit bit once read */
    SOC_SAND_OUT uint32              *priority,
    SOC_SAND_OUT ARAD_TCAM_ENTRY     *entry,
    SOC_SAND_OUT ARAD_TCAM_ACTION    *action,
    SOC_SAND_OUT uint8               *found,
    SOC_SAND_OUT uint8               *hit_bit
  );

uint32
  arad_tcam_db_entry_get_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               entry_id
  );
  
/*********************************************************************
* NAME:
 *   arad_tcam_db_entry_search_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Search for an entry in a database using the same logic
 *   used by the hardware.
 * INPUT:
 *   SOC_SAND_IN  int               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32               tcam_db_id -
 *     ID of the database.
 *   SOC_SAND_IN  ARAD_TCAM_ENTRY           *key -
 *     The key for the search operation. If a mask bit is set,
 *     the corresponding value bit is used in the comparison;
 *     otherwise that value bit is ignored.
 *   SOC_SAND_OUT uint32               *entry_id -
 *     The first hit's entry ID.
 *   SOC_SAND_OUT uint8               *found -
 *     Indicates whether a match was found or not.
 * REMARKS:
 *   This function only works for databases with 72- or
 *   144-bit keys.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tcam_db_entry_search_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  ARAD_TCAM_ENTRY           *key,
    SOC_SAND_OUT uint32               *entry_id,
    SOC_SAND_OUT uint8               *found
  );

uint32
  arad_tcam_db_entry_search_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  ARAD_TCAM_ENTRY           *key
  );

/*********************************************************************
* NAME:
 *   arad_tcam_db_entry_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Removes an entry from the database.
 * INPUT:
 *   SOC_SAND_IN  int               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32               tcam_db_id -
 *     ID of the database.
 *   SOC_SAND_IN  uint32               entry_id -
 *     ID of the entry to remove.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tcam_db_entry_remove_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint8                is_bank_freed_if_no_entry, /* If False, an empty bank is not freed */
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               entry_id
  );

uint32
  arad_tcam_db_entry_remove_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               entry_id
  );

/*********************************************************************
* NAME:
 *   arad_tcam_db_is_bank_used_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Queries whether the bank belongs to the database or not.
 * INPUT:
 *   SOC_SAND_IN  int               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32               tcam_db_id -
 *     ID of the database to query.
 *   SOC_SAND_IN  uint32               bank_id -
 *     The bank in question.
 *   SOC_SAND_OUT uint8               *is_used -
 *     Whether the bank belongs to the database or not.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_tcam_db_is_bank_used_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               bank_id,
    SOC_SAND_OUT uint8               *is_used
  );

uint32
  arad_tcam_db_is_bank_used_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               bank_id
  );

uint32
  arad_tcam_bank_owner_get_unsafe(
    SOC_SAND_IN    int               unit,
    SOC_SAND_IN    uint32               bank_id,
    SOC_SAND_OUT   ARAD_TCAM_BANK_OWNER *bank_owner
  );

uint32
    arad_tcam_bank_owner_verify(
      SOC_SAND_IN    int               unit,
      SOC_SAND_IN    uint32               bank_id,
      SOC_SAND_OUT   ARAD_TCAM_BANK_OWNER bank_owner
    );

/*********************************************************************
*     Compress a TCAM Database: compress the entries to minimum
*     number of banks.
*********************************************************************/
uint32
  arad_tcam_managed_db_compress_unsafe(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32               tcam_db_id
  );

#ifdef ARAD_TCAM_BANK_INIT_MOVE_ENTRIES_ENABLE
uint32
    arad_tcam_new_bank_move_entries(
      SOC_SAND_IN  int            unit,
      SOC_SAND_IN  uint32            tcam_db_id,
      SOC_SAND_IN  uint32            bank_id,
      SOC_SAND_IN  uint8             is_direct,
      SOC_SAND_IN  uint32            access_profile_array_id,
      SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size,
      SOC_SAND_IN  uint8             is_inserted_top
  );
#endif /* ARAD_TCAM_BANK_INIT_MOVE_ENTRIES_ENABLE */

void
  ARAD_TCAM_PREFIX_clear(
    SOC_SAND_OUT ARAD_TCAM_PREFIX *info
  );

void
  ARAD_TCAM_LOCATION_clear(
    SOC_SAND_OUT ARAD_TCAM_LOCATION *info
  );

void
  ARAD_TCAM_GLOBAL_LOCATION_clear(
    SOC_SAND_OUT ARAD_TCAM_GLOBAL_LOCATION *info
  );

void
  ARAD_TCAM_PRIO_LOCATION_clear(
    SOC_SAND_OUT ARAD_TCAM_PRIO_LOCATION *info
  );

void
  ARAD_TCAM_RANGE_clear(
    SOC_SAND_OUT ARAD_TCAM_RANGE *info
  );

void
  ARAD_TCAM_ENTRY_clear(
    SOC_SAND_OUT ARAD_TCAM_ENTRY *info
  );

void
  ARAD_TCAM_ACTION_clear(
    SOC_SAND_OUT ARAD_TCAM_ACTION *info
  );

uint32
  ARAD_TCAM_PREFIX_verify(
    SOC_SAND_IN  ARAD_TCAM_PREFIX *info
  );

uint32
  ARAD_TCAM_LOCATION_verify(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  ARAD_TCAM_LOCATION *info
  );

uint32
  ARAD_TCAM_GLOBAL_LOCATION_verify(
    SOC_SAND_IN  int     unit,
    SOC_SAND_IN  ARAD_TCAM_GLOBAL_LOCATION *info
  );

uint32
  ARAD_TCAM_PRIO_LOCATION_verify(
    SOC_SAND_IN  int     unit,
    SOC_SAND_IN  uint32     tcam_db_id,
    SOC_SAND_IN  ARAD_TCAM_PRIO_LOCATION *info
  );


uint32
  ARAD_TCAM_RANGE_verify(
    SOC_SAND_IN  int             unit,
    SOC_SAND_IN  ARAD_TCAM_RANGE *info
  );

uint32
  ARAD_TCAM_ENTRY_verify(
    SOC_SAND_IN  ARAD_TCAM_ENTRY *info
  );

uint32
  ARAD_TCAM_ACTION_verify(
    SOC_SAND_IN  ARAD_TCAM_ACTION *info
  );

#if ARAD_DEBUG_IS_LVL1

const char*
  ARAD_TCAM_BANK_ENTRY_SIZE_to_string(
    SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE enum_val
  );

void
  ARAD_TCAM_PREFIX_print(
    SOC_SAND_IN  ARAD_TCAM_PREFIX *info
  );

void
  ARAD_TCAM_LOCATION_print(
    SOC_SAND_IN  ARAD_TCAM_LOCATION *info
  );

void
  ARAD_TCAM_GLOBAL_LOCATION_print(
    SOC_SAND_IN  ARAD_TCAM_GLOBAL_LOCATION *info
  );

void
  ARAD_TCAM_RANGE_print(
    SOC_SAND_IN  ARAD_TCAM_RANGE *info
  );

void
  ARAD_TCAM_ENTRY_print(
    SOC_SAND_IN  ARAD_TCAM_ENTRY *info
  );

#endif /* ARAD_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_TCAM_INCLUDED__*/
#endif

