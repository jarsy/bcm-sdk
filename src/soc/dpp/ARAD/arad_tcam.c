/* $Id: arad_tcam.c,v 1.111 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/
#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_TCAM
#include <shared/bsl.h>

/*************
 * INCLUDES  *
 *************/
/* { */

#include <shared/swstate/access/sw_state_access.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>

#include <soc/dpp/ARAD/arad_framework.h>
#include <soc/dpp/ARAD/arad_tcam.h>

#ifdef CRASH_RECOVERY_SUPPORT
#include <soc/dpp/ARAD/arad_tcam_access_crash_recovery.h>
#endif

#include <soc/dpp/SAND/Utils/sand_occupation_bitmap.h>
#include <soc/dpp/ARAD/arad_tcam_mgmt.h>
#include <soc/dpp/ARAD/arad_sw_db_tcam_mgmt.h>
#include <soc/dpp/ARAD/arad_api_general.h>
#include <soc/dpp/ARAD/arad_general.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_TCAM_TCAM_DB_ID_MAX                                 (ARAD_TCAM_MAX_NOF_LISTS)
#define ARAD_TCAM_ENTRY_ID_MAX                                   (SOC_SAND_U32_MAX)
#define ARAD_TCAM_BITS_MAX                                       (SOC_SAND_U32_MAX)
#define ARAD_TCAM_LENGTH_MAX                                     (SOC_SAND_UINT_MAX)
#define SOC_DPP_DEFS_TCAM_ENTRY_MAX(unit)                        (2 * SOC_DPP_DEFS_GET(unit, nof_tcam_big_bank_lines))

#define ARAD_TCAM_WORD_SIZE_IN_BITS                              (160)
#define ARAD_TCAM_ACTION_WORD_SIZE_IN_BITS                       (SOC_DPP_DEFS_GET(unit, tcam_action_width))
#define ARAD_TCAM_DB_LOCATION_KEY_SIZE                           (2) /* to encode both bank and line */

#define ARAD_TCAM_NOF_BANK_OWNERS_IN_BITS     (2)

/* 
 * If set, different insertion algorithm where the entries are
 * inserted in the middle of a free range and not in the first free 
 * place. In high-level, it allows: 
 * - a better worst-case scenario since there is no big block to move 
 * all the time (complexity ~n^1.5) 
 * - a worst best-cast scenario, since inserting in descending priority 
 * is in O(1) otherwise, and n^1.5 here.
 */ 
#define ARAD_TCAM_INSERTION_ALGORITHM_MIDDLE_FREE (1)

#if ARAD_TCAM_INSERTION_ALGORITHM_MIDDLE_FREE
/* Compute the middle place of the free places */
#define ARAD_TCAM_INSERTION_ALGORITHM_MIDDLE_FREE_FIND(found_taken, direction, place_taken, start, end) \
    ((((found_taken)? ((direction == ARAD_TCAM_DIRECTION_FORWARD)? (place_taken - 1): (place_taken + 1)): \
    ((direction == ARAD_TCAM_DIRECTION_FORWARD)? end: start)) + \
    ((direction == ARAD_TCAM_DIRECTION_FORWARD)? start: end) + ((direction == ARAD_TCAM_DIRECTION_FORWARD)?1:0)) / 2)

/* 2 occupation bitmaps: one for regular, one for the inverse */
#define ARAD_TCAM_NOF_OCCUPATIONS_BITMAPS (2)

#else

/* 1 occupation bitmap for regular */
#define ARAD_TCAM_NOF_OCCUPATIONS_BITMAPS (1)

#endif /* ARAD_TCAM_INSERTION_ALGORITHM_MIDDLE_FREE */

#define ARAD_TCAM_BANK_INIT_MOVE_ENTRIES_NOF_ADJACENT_BANKS     (2)

/* } */
/*************
 * MACROS    *
 *************/
/* { */


#ifdef BROADCOM_DEBUG
#define ARAD_TCAM_DB_EXISTS_ASSERT                           \
  {                                                        \
    res = arad_tcam_db_exists_assert(unit, tcam_db_id); \
    SOC_SAND_CHECK_FUNC_RESULT(res, 9, exit);                 \
  }

#define ARAD_TCAM_DB_ENTRY_EXISTS_ASSERT(__entry_id)                                        \
  {                                                                             \
    res = arad_tcam_db_entry_exists_assert(unit, tcam_db_id, __entry_id);    \
    SOC_SAND_CHECK_FUNC_RESULT(res, 11, exit);                                  \
  }
#else /* BROADCOM_DEBUG */ 
#define ARAD_TCAM_DB_EXISTS_ASSERT                           
#define ARAD_TCAM_DB_ENTRY_EXISTS_ASSERT(__entry_id)                                        
#endif /* BROADCOM_DEBUG */

#define ARAD_TCAM_ENTRY_LINE_PAIR_80B(entry_line) ((((entry_line) / 2) * 2) + (1 - ((entry_line) % 2)))

#define ARAD_TCAM_ENTRY_LINE_PAIR_CLOSEST_ID(entry_line, direction) \
  (((entry_line % 2) == ((direction == ARAD_TCAM_DIRECTION_BACKWARD)? 0: 1))? entry_line: ARAD_TCAM_ENTRY_LINE_PAIR_80B(entry_line))

/* Get entry line */
#define ARAD_TCAM_ENTRY_LINE_GET(place, entry_size) \
  ((entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS)? ((place) / 2): (place))

#define ARAD_TCAM_ENTRY_NOF_LINES_GET(place1, place2, entry_size) \
  soc_sand_abs(ARAD_TCAM_ENTRY_LINE_GET(place1, entry_size) - ARAD_TCAM_ENTRY_LINE_GET(place2, entry_size))

/* 
 * LSB of the bank-id in the location encoding in the second uint8
 * In Arad: entry-id in bits 10:0, and bank in bits 15:11, so it is 11-8=3
 * In Jericho: entry-id in bits 11:0, and bank in bits 15:12, so it is 12-8=4
 */
#define ARAD_TCAM_LOCATION_ENCODING_BANK_LSB_IN_SECOND_UINT8 (soc_sand_log2_round_up(2 /* for 80b entries */ * SOC_DPP_DEFS_GET(unit, nof_tcam_big_bank_lines)) - 8)
/* 
 * Entry mask in uint32 to validate that the MSBs are zero
 * In Arad: ~((1<<11) - 1)
 * In Jericho: ~((1<<12) - 1)
 */
#define ARAD_TCAM_LOCATION_ENCODING_ENTRY_MASK (~((uint32) ((1 << (ARAD_TCAM_LOCATION_ENCODING_BANK_LSB_IN_SECOND_UINT8 + 8)) - 1)))
/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
  ARAD_TCAM_DIRECTION_FORWARD,
  ARAD_TCAM_DIRECTION_BACKWARD
} ARAD_TCAM_DIRECTION;

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

/*
 *  TCAM database priority list helper functions
 */
STATIC
  uint32
    arad_tcam_db_location_encode(
      SOC_SAND_IN  int                unit,
      SOC_SAND_IN  ARAD_TCAM_LOCATION *location,
      SOC_SAND_OUT uint8               *key
    )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

#ifdef BROADCOM_DEBUG
  {
      uint32 res;
      res = ARAD_TCAM_LOCATION_verify(unit, location);
      SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

      if ((location->entry & ARAD_TCAM_LOCATION_ENCODING_ENTRY_MASK) != 0)
      {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit,
                              "Entry %d - Failed to encode location. Invalid entry.\n\r"), location->entry));
        SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_ENTRY_NDX_OUT_OF_RANGE_ERR, 10, exit);
      }
  }
#endif /* BROADCOM_DEBUG */

  /*
   * Simple encoding of the location into uint8 key[2]: 
   * [bank-id | entry-id up to 2*SOC_DPP_DEFS_GET(unit, nof_tcam_big_bank_lines)]
   */
  key[0] = (uint8) location->entry & 0xff;
  if(ARAD_TCAM_LOCATION_ENCODING_BANK_LSB_IN_SECOND_UINT8 >= 8) {
      key[1] = 0;
  } else {
      key[1] = (location->bank_id << ARAD_TCAM_LOCATION_ENCODING_BANK_LSB_IN_SECOND_UINT8) & 0xff;
  }
  key[1] |= (location->entry >> 8) & 0xff;

  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_location_encode", 0, 0);
}

STATIC
  void
    arad_tcam_db_location_decode(
      SOC_SAND_IN  int                unit,
      SOC_SAND_IN  uint8          *key,
      SOC_SAND_OUT ARAD_TCAM_LOCATION *location
    )
{
    /* In key[1], mask the entry-id according to the encoding */
    if(ARAD_TCAM_LOCATION_ENCODING_BANK_LSB_IN_SECOND_UINT8 > 8 ) {
        location->entry = 0;
	location->bank_id = 0;
    } else {
        /* coverity[large_shift : FALSE] */
        location->entry = (((uint32) key[1] & ((1 << (ARAD_TCAM_LOCATION_ENCODING_BANK_LSB_IN_SECOND_UINT8))-1)) << 8);
	location->bank_id = key[1] >> ARAD_TCAM_LOCATION_ENCODING_BANK_LSB_IN_SECOND_UINT8;
    }
    location->entry |= key[0];
}

/* uniform encoding no matter the bank entry size */
uint32
    arad_tcam_global_location_encode(
      SOC_SAND_IN  int                unit,
      SOC_SAND_IN  ARAD_TCAM_LOCATION *location,
      SOC_SAND_OUT uint32              *global_location_id
    )
{
  uint8         
      key[ARAD_TCAM_DB_LOCATION_KEY_SIZE] = {0};
  uint32
      res;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res  = arad_tcam_db_location_encode(unit, location, key);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  *global_location_id = key[0] + (key[1] << 8);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_global_location_encode", 0, 0);
}

STATIC
  void
    arad_tcam_db_prio_list_data_encode(
      SOC_SAND_IN  ARAD_TCAM_PRIO_LOCATION *prio_location,
      SOC_SAND_OUT uint8  *data
    )
{
    sal_memcpy(data, prio_location, sizeof(ARAD_TCAM_PRIO_LOCATION));
}

STATIC
  uint32
    arad_tcam_db_priority_data_decode(
        SOC_SAND_IN  int        unit,
        SOC_SAND_IN  uint32     tcam_db_id,
        SOC_SAND_IN  uint8      *data,
        SOC_SAND_OUT ARAD_TCAM_PRIO_LOCATION *prio_location
    )
{
    uint32
        res;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    sal_memcpy(prio_location, data, sizeof(ARAD_TCAM_PRIO_LOCATION));

    /* All the defined ranges in the prio list must be valid */
    res  = ARAD_TCAM_PRIO_LOCATION_verify(unit, tcam_db_id, prio_location);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_priority_data_decode", 0, 0);
}


uint32
    arad_tcam_db_prio_list_priority_value_decode(
        SOC_SAND_IN  uint8      *data
    )
{
    uint32
      value;

    soc_sand_U8_to_U32(
      data,
      sizeof(uint32),
      &value
    );

  return value;
}

STATIC
  void
    arad_tcam_db_prio_list_priority_value_encode(
      SOC_SAND_IN  uint32     priority,
      SOC_SAND_OUT uint8     *data
    )
{
    soc_sand_U32_to_U8(
      &priority,
      sizeof(uint32),
      data
    );
}

STATIC
  void
    arad_tcam_db_entry_id_encode(
      SOC_SAND_IN uint32        tcam_db_id,
      SOC_SAND_IN  uint32 entry_id,
      SOC_SAND_OUT uint8   *data
    )
{
  soc_sand_U32_to_U8(
    &entry_id,
    sizeof(uint32),
    data
  );
  /* Use only 16 bits for TCAM DB Id */
  data[4] = (uint8) (tcam_db_id & 0xFF);
  data[5] = (uint8) ((tcam_db_id >> 8) & 0xFF);
}


  int32
    arad_tcam_db_priority_list_cmp_priority(
      SOC_SAND_IN uint8  *buffer1,
      SOC_SAND_IN uint8  *buffer2,
              uint32 size
    )
{
  uint32
    prio_1,
    prio_2;
  ARAD_TCAM_PRIO_LOCATION 
      prio_location_1,
      prio_location_2;

  if ((buffer1 == NULL) || (buffer2 == NULL)) {
      /* Not supposed to compare NULL pointers */
      assert(0);
      return 0; 
  }
  /* Key comparison */
  if (size == ARAD_TCAM_DB_LIST_KEY_SIZE) {
      prio_1 = arad_tcam_db_prio_list_priority_value_decode(buffer1);
      prio_2 = arad_tcam_db_prio_list_priority_value_decode(buffer2);
      return (((int32) prio_1) - ((int32) prio_2));
  }
  else if (size == ARAD_TCAM_DB_LIST_DATA_SIZE) { /* Data comparison */
      ARAD_TCAM_PRIO_LOCATION_clear(&prio_location_1);
      ARAD_TCAM_PRIO_LOCATION_clear(&prio_location_2);
      sal_memcpy(&prio_location_1, buffer1, sizeof(ARAD_TCAM_PRIO_LOCATION));
      sal_memcpy(&prio_location_2, buffer2, sizeof(ARAD_TCAM_PRIO_LOCATION));
      return ((prio_location_1.entry_id_first != prio_location_2.entry_id_first)
              || (prio_location_1.entry_id_last != prio_location_2.entry_id_last))? 1 :0;
  } else {
      /* Unknown input formats */
      assert(0);
      return 0;
  }
}



  uint32
    arad_tcam_db_data_structure_entry_set(
      SOC_SAND_IN    int prime_handle,
      SOC_SAND_IN    uint32 sec_handle,
      SOC_SAND_INOUT uint8  *buffer,
      SOC_SAND_IN    uint32 offset,
      SOC_SAND_IN    uint32 len,
      SOC_SAND_IN    uint8  *data
    )
{
  sal_memcpy(
    buffer + (offset * len),
    data,
    len
  );
  return SOC_SAND_OK;
}


  uint32
    arad_tcam_db_data_structure_entry_get(
      SOC_SAND_IN  int prime_handle,
      SOC_SAND_IN  uint32 sec_handle,
      SOC_SAND_IN  uint8  *buffer,
      SOC_SAND_IN  uint32 offset,
      SOC_SAND_IN  uint32 len,
      SOC_SAND_OUT uint8  *data
    )
{
  sal_memcpy(
    data,
    buffer + (offset * len),
    len
  );
  return SOC_SAND_OK;
}

/* When do you allow not to respect theBCM rule of entry insertion oreder matters */
STATIC
  uint32
    arad_tcam_db_block_entry_insertion_rule_respected_get(
      SOC_SAND_IN  int                           unit,
      SOC_SAND_IN  uint32                        tcam_db_id,
      SOC_SAND_OUT uint8                         *insertion_rule_respected
    )
{
    ARAD_TCAM_BANK_OWNER
      db_bank_owner;
    uint8
      no_insertion_priority_order;
    uint32
      access_profile_id,
      res;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.access_profile_id.get(unit, tcam_db_id, 0, &access_profile_id);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
    
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam_mgmt.profiles.bank_owner.get(
            unit,
            access_profile_id,
            &db_bank_owner
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.no_insertion_priority_order.get(unit, tcam_db_id, &no_insertion_priority_order);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
    
    /* Respect the rule if for PMF and the flag not set for this TCAM DB */
    *insertion_rule_respected =  (((db_bank_owner == ARAD_TCAM_BANK_OWNER_PMF_0) 
                                   || (db_bank_owner == ARAD_TCAM_BANK_OWNER_PMF_1) 
                                   || (db_bank_owner == ARAD_TCAM_BANK_OWNER_EGRESS_ACL)) 
                                  && (!no_insertion_priority_order));
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_block_entry_insertion_rule_respected_get()", 0, 0);
}

/* When to use the new method per TCAM DB-ID (direct table DB not inserted) */
uint32
    arad_tcam_db_use_new_method_per_db_get(
      SOC_SAND_IN  int                        unit,
      SOC_SAND_IN  uint32                     tcam_db_id,
      SOC_SAND_OUT uint8                      *move_per_block
    )
{
    uint8
        sparse_priorities;
    ARAD_TCAM_BANK_OWNER
        db_bank_owner;
    ARAD_TCAM_BANK_ENTRY_SIZE
      entry_size;
    uint32
      access_profile_id,
      res;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Use the old method for IP TCAM and 80b and Direct table because more efficient */
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit,tcam_db_id, &entry_size);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.access_profile_id.get(unit, tcam_db_id, 0, &access_profile_id);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
    
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam_mgmt.profiles.bank_owner.get(
            unit,
            access_profile_id,
            &db_bank_owner
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.sparse_priorities.get(unit, tcam_db_id, &sparse_priorities);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

    *move_per_block = ((db_bank_owner == ARAD_TCAM_BANK_OWNER_FLP_TCAM)
                       || (db_bank_owner == ARAD_TCAM_BANK_OWNER_FLP_TRAPS)
                       || (db_bank_owner == ARAD_TCAM_BANK_OWNER_VT)
                       || (db_bank_owner == ARAD_TCAM_BANK_OWNER_TT)
                       || (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS)
                       || sparse_priorities
                       )? FALSE : TRUE;

#ifdef PLISIM   
      if (SAL_BOOT_PLISIM) { 
          /* Only in the Simulation - do not allow HW-move since it is not existing */
          *move_per_block = FALSE;
      }
#endif /* PLISIM */

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_use_new_method_per_db_get()", 0, 0);
}


uint32
    arad_tcam_bank_entry_size_to_entry_count_get(
        SOC_SAND_IN  int                       unit,
        SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size,
        SOC_SAND_IN  uint32                    bank_ndx
     )
{
  uint32
    ret = 0;

  switch (entry_size)
  {
  case ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS:
    ret = 2 * ARAD_TCAM_NOF_LINES_PER_BANK(unit, bank_ndx);
    break;

  case ARAD_TCAM_BANK_ENTRY_SIZE_160_BITS:
  case ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS:
  ret = ARAD_TCAM_NOF_LINES_PER_BANK(unit, bank_ndx);
    break;

  default:
    break;
  }

  return ret;
}

STATIC
  uint32
    arad_tcam_db_entries_per_bank_get(
      SOC_SAND_IN  int    unit,
      SOC_SAND_IN  uint32 tcam_db_id,
      SOC_SAND_IN  uint32 bank_id,
      SOC_SAND_OUT uint32 *db_entries_per_bank
    )
{
    ARAD_TCAM_BANK_ENTRY_SIZE entry_size;
    uint32
        res;
    uint8
        is_direct;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.is_direct.get(unit, tcam_db_id, &is_direct);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
    if(is_direct){
        entry_size = ARAD_TCAM_BANK_ENTRY_SIZE_160_BITS;
    } else {
        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit, tcam_db_id, &entry_size);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
    }
    *db_entries_per_bank = arad_tcam_bank_entry_size_to_entry_count_get(unit, entry_size, bank_id);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entries_per_bank_get()", 0, 0);
}

/*
 *  Hardware access functions
 */
STATIC
  uint32
    arad_tcam_entry_word_extract(
      SOC_SAND_IN  ARAD_TCAM_ENTRY                *entry,
      SOC_SAND_IN  uint8                    take_lsb,
      SOC_SAND_OUT ARAD_PP_IHB_TCAM_BANK_TBL_DATA *tbl_data
    )
{
  uint32
    first_bit,
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  first_bit = take_lsb ? 0 : ARAD_TCAM_WORD_SIZE_IN_BITS;
  res = soc_sand_bitstream_get_any_field(
          entry->value,
          first_bit,
          ARAD_TCAM_WORD_SIZE_IN_BITS,
          tbl_data->value
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  res = soc_sand_bitstream_get_any_field(
          entry->mask,
          first_bit,
          ARAD_TCAM_WORD_SIZE_IN_BITS,
          tbl_data->mask
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_entry_word_extract()", take_lsb, 0);
}


STATIC
  uint32
    arad_tcam_entry_word_recover(
      SOC_SAND_IN  ARAD_PP_IHB_TCAM_BANK_TBL_DATA *tbl_data,
      SOC_SAND_IN  uint8                    take_lsb,
      SOC_SAND_OUT ARAD_TCAM_ENTRY                *entry
    )
{
  uint32
    first_bit,
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  first_bit = take_lsb ? 0 : ARAD_TCAM_WORD_SIZE_IN_BITS;
  entry->valid = tbl_data->valid;
  res = soc_sand_bitstream_set_any_field(
          tbl_data->value,
          first_bit,
          ARAD_TCAM_WORD_SIZE_IN_BITS,
          entry->value
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  res = soc_sand_bitstream_set_any_field(
          tbl_data->mask,
          first_bit,
          ARAD_TCAM_WORD_SIZE_IN_BITS,
          entry->mask
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_entry_word_recover()", take_lsb, 0);
}

STATIC
  uint32
    arad_tcam_action_word_recover(
       SOC_SAND_IN  int unit,
      SOC_SAND_IN  ARAD_PP_IHB_TCAM_ACTION_TBL_DATA *tbl_data,
      SOC_SAND_IN  uint8                    word,
      SOC_SAND_OUT ARAD_TCAM_ACTION         *action
    )
{
  uint32
    start_bit,
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  start_bit = word*ARAD_TCAM_ACTION_WORD_SIZE_IN_BITS;
  res = soc_sand_bitstream_set_any_field(
          &tbl_data->action,
          start_bit,
          ARAD_TCAM_ACTION_WORD_SIZE_IN_BITS,
          action->value
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_entry_word_recover()", word, 0);
}

/*
 *  Hardware access functions
 */
STATIC
  uint32
    arad_tcam_action_word_extract(
       SOC_SAND_IN  int unit,
      SOC_SAND_IN  ARAD_TCAM_ACTION         *action,
      SOC_SAND_IN  uint8                    word,      
      SOC_SAND_OUT ARAD_PP_IHB_TCAM_ACTION_TBL_DATA *tbl_data
    )
{
  uint32
    start_bit,
    res = SOC_SAND_OK;
    
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);  

  start_bit = word*ARAD_TCAM_ACTION_WORD_SIZE_IN_BITS;

  res = soc_sand_bitstream_get_any_field(
            action->value,
            start_bit,
            ARAD_TCAM_ACTION_WORD_SIZE_IN_BITS,
            &(tbl_data->action)
          );     
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_entry_word_extract()", word, 0);
}

STATIC
  uint32
    arad_tcam_tbl_bank_write(
      SOC_SAND_IN  int               unit,
      SOC_SAND_IN  uint32               bank_id,
      SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size,
      SOC_SAND_IN  uint32                base_address,
      SOC_SAND_IN  ARAD_TCAM_ENTRY           *entry
    )
{
  uint32
    nof_words,
      nof_entries_in_line,
    word,
    res = SOC_SAND_OK;
  ARAD_PP_IHB_TCAM_BANK_TBL_DATA
    tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /*
   *  Copy the entry data and write it to the table
   */
  tbl_data.valid = entry->valid;
  nof_words = (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) ? 2 : 1;
  nof_entries_in_line = (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) ? 3 : 
                          ((entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS) ? 2 : 1);
  for (word = 0; word < nof_words; ++ word)
  {
    res = arad_tcam_entry_word_extract(
            entry,
            (word == 0) ? TRUE : FALSE,
            &tbl_data
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    res = arad_pp_ihb_tcam_tbl_write_unsafe(
            unit,
            bank_id + word,
            nof_entries_in_line,
            base_address,
            &tbl_data
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_tbl_bank_write()", bank_id, base_address);
}

STATIC
  uint32
    arad_tcam_tbl_bank_read(
      SOC_SAND_IN  int               unit,
      SOC_SAND_IN  uint32               bank_id,
      SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size,
      SOC_SAND_IN  uint32                base_address,
      SOC_SAND_OUT ARAD_TCAM_ENTRY           *entry
    )
{
  uint32
    nof_words,
      nof_entries_in_line,
    word,
    res = SOC_SAND_OK;

  uint8
  	is_small_bank;

  uint64
    mem_val;
  uint32
    tmp_sweep_interval = 0;
  ARAD_PP_IHB_TCAM_BANK_TBL_DATA
    tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  nof_words = (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) ? 2 : 1;
  nof_entries_in_line = (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) ? 3 : 
                          ((entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS) ? 2 : 1);
  is_small_bank = (bank_id < SOC_DPP_DEFS_GET(unit, nof_tcam_big_banks)) ? FALSE : TRUE;

  for (word = 0; word < nof_words; ++ word)
  {
    if (SOC_IS_JERICHO(unit) && is_small_bank)  /*Fix for SDK-72264 - TCAM: wrong read result in small bank.
                                                In Jericho Due to wrong pipe sampling in the small banks the reply from the TCAM-IP
                                                is coming 1 clock too early, the tcam hold its read value until the next read operation.
                                                Back to back read (From CPU and from TCAM-management ) may result in wrong read-result.
                                                Fix: Disable sweep machine during read for small banks.*/
    {
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_PPDB_A_TCAM_PARITY_MACHINE_CONFIGURATIONr(unit, &mem_val));
        tmp_sweep_interval = soc_reg64_field32_get(unit, PPDB_A_TCAM_PARITY_MACHINE_CONFIGURATIONr, mem_val, TCAM_PARITY_SWEEP_INTERVALf);
        soc_reg64_field32_set(unit, PPDB_A_TCAM_PARITY_MACHINE_CONFIGURATIONr, &mem_val, TCAM_PARITY_SWEEP_INTERVALf, 0x0);
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_PPDB_A_TCAM_PARITY_MACHINE_CONFIGURATIONr(unit, mem_val));
    }

    res = arad_pp_ihb_tcam_tbl_read_unsafe(
            unit,
            bank_id + word,
            nof_entries_in_line,
            base_address,
            &tbl_data
          );

    if (SOC_IS_JERICHO(unit) && is_small_bank)  /* Restore original sweep value */
    {
        soc_reg64_field32_set(unit, PPDB_A_TCAM_PARITY_MACHINE_CONFIGURATIONr, &mem_val, TCAM_PARITY_SWEEP_INTERVALf, tmp_sweep_interval);
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, WRITE_PPDB_A_TCAM_PARITY_MACHINE_CONFIGURATIONr(unit, mem_val));
    }

    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);  /* Check the tcam_tbl_read_unsafe result. Intentionally moved after restore od the sweep  */

    res = arad_tcam_entry_word_recover(
            &tbl_data,
            (word == 0) ? TRUE : FALSE,
            entry
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_tbl_bank_read()", bank_id, base_address);
}

STATIC
  uint32
    arad_tcam_tbl_bank_compare(
      SOC_SAND_IN  int     unit,
      SOC_SAND_IN  uint32     bank_id,
      SOC_SAND_IN  ARAD_TCAM_ENTRY *key,
      SOC_SAND_OUT uint32     *base_address,
      SOC_SAND_OUT uint8     *found
    )
{
  uint32
    res = SOC_SAND_OK;
  uint32
      nof_entries_in_line,
    ndx;
  ARAD_TCAM_BANK_ENTRY_SIZE
    entry_size;
  ARAD_PP_IHB_TCAM_BANK_TBL_DATA
    compare_data;
  ARAD_PP_IHB_TCAM_COMPARE_DATA
    found_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  for (ndx = 0; ndx < ARAD_PP_IHB_TCAM_DATA_WIDTH; ++ndx)
  {
    compare_data.value[ndx] = key->value[ndx];
    compare_data.mask[ndx]  = key->mask[ndx];
  }

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.entry_size.get(
          unit,
          bank_id,
          &entry_size
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);

  nof_entries_in_line = (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) ? 3 : 
                          ((entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS) ? 2 : 1);
  res = arad_pp_ihb_tcam_tbl_compare_unsafe(
          unit,
          bank_id,
          nof_entries_in_line,
          &compare_data,
          &found_data
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  *found = SOC_SAND_NUM2BOOL(found_data.found);
  if (*found)
  {
    *base_address = found_data.address;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_tbl_bank_compare()", bank_id, 0);
}

STATIC
  uint32
    arad_tcam_tbl_calc_action_banks_and_lines(
        SOC_SAND_IN  int            unit,
        SOC_SAND_IN  uint32         bank_id,
        SOC_SAND_IN  uint32         base_address,
        SOC_SAND_INOUT  ARAD_TCAM_ACTION_SIZE *action_bitmap_ndx,
        SOC_SAND_OUT uint32         *line,
        SOC_SAND_OUT uint32         *action_tbl

        )
{
    ARAD_TCAM_BANK_ENTRY_SIZE
        entry_size;
    uint32
        nof_action_writes=0,
        res = SOC_SAND_OK;
    uint32
      action_ndx,
      action_bit;
    ARAD_TCAM_ACTION_SIZE
        act_action_bitmap_ndx = *action_bitmap_ndx;


    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.entry_size.get(
            unit,
            bank_id,
            &entry_size
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);

    /* calc ACTION banks and lines */
    if(entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS) {
        nof_action_writes = 1;
        action_tbl[0] = bank_id * 2 + (base_address)%2; /* two action tables per bank */
        line[0] = base_address/2; /* lsb is written to bank 2n + 1*/
        *action_bitmap_ndx = act_action_bitmap_ndx = 1;
    }
    else{ /* 160/320 */
        for (action_ndx = 0; action_ndx < ARAD_TCAM_NOF_ACTION_SIZES; ++action_ndx)
        {
            action_bit = (1 << action_ndx);
            if(action_bit & act_action_bitmap_ndx) {
                action_tbl[nof_action_writes] = bank_id * 2 + action_ndx;
                line[nof_action_writes] = base_address;
            }
            ++nof_action_writes;
        }
    }
    exit:
      SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_tbl_calc_action_banks_and_lines()", bank_id, base_address);
}

STATIC
  uint32
    arad_tcam_tbl_action_write(
      SOC_SAND_IN  int unit,
      SOC_SAND_IN  uint32 bank_id,
      SOC_SAND_IN  uint32 base_address,
      SOC_SAND_IN  ARAD_TCAM_ACTION_SIZE action_bitmap_ndx,      
      SOC_SAND_IN uint8         hit_bit,
      SOC_SAND_IN  ARAD_TCAM_ACTION *action
    )
{
  uint32
    action_ndx,
    action_bit,
    word,
    line[ARAD_TCAM_NOF_ACTION_SIZES],
    action_tbl[ARAD_TCAM_NOF_ACTION_SIZES],
    res = SOC_SAND_OK;
  ARAD_TCAM_ACTION_SIZE 
      act_action_bitmap_ndx = action_bitmap_ndx;
  ARAD_PP_IHB_TCAM_ACTION_TBL_DATA
    tbl_data_hit_bit,
    tbl_data;



  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  ARAD_CLEAR(&tbl_data, ARAD_PP_IHB_TCAM_ACTION_TBL_DATA, 1);
  ARAD_CLEAR(&tbl_data_hit_bit, ARAD_PP_IHB_TCAM_ACTION_TBL_DATA, 1);
  tbl_data_hit_bit.action = hit_bit;

  res = arad_tcam_tbl_calc_action_banks_and_lines(unit, bank_id, base_address, &act_action_bitmap_ndx, line, action_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);


  /* Run over all needed action entries and set accordingly */
  /* action bit represents which action entry is available, */
  /* word represents the the start bit from action information */
  for (action_ndx = 0, word = 0; action_ndx < ARAD_TCAM_NOF_ACTION_SIZES; ++action_ndx)
  {
    action_bit = (1 << action_ndx);

    if (act_action_bitmap_ndx & action_bit)
    {
       /* extract specific word */
       res = arad_tcam_action_word_extract(
              unit,
              action,
              word,
              &tbl_data
              );
       SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
      
       res = arad_pp_ihb_tcam_action_tbl_set_unsafe(
            unit,
            action_tbl[action_ndx],
            line[action_ndx],
            FALSE /* is_for_hit_bit */,
            &tbl_data
          );
       SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

       /* Set the Hit bit */
       res = arad_pp_ihb_tcam_action_tbl_set_unsafe(
                unit,
                action_tbl[action_ndx],
                line[action_ndx],
                TRUE /* is_for_hit_bit */,
                &tbl_data_hit_bit
              );
       SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit);

       word++;
    }            
  } 

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_tbl_action_write()", bank_id, base_address);
}

STATIC
  uint32
    arad_tcam_tbl_action_read(
      SOC_SAND_IN  int unit,
      SOC_SAND_IN  uint32 bank_id,
      SOC_SAND_IN  uint32 base_address,
      SOC_SAND_IN  ARAD_TCAM_ACTION_SIZE action_bitmap_ndx,
      SOC_SAND_OUT uint8             *hit_bit,
      SOC_SAND_OUT ARAD_TCAM_ACTION  *action
    )
{
  uint32
    action_ndx,
    action_bit,
    word,
    line[ARAD_TCAM_NOF_ACTION_SIZES],
    action_tbl[ARAD_TCAM_NOF_ACTION_SIZES],
    res = SOC_SAND_OK;
  ARAD_TCAM_ACTION_SIZE 
      act_action_bitmap_ndx = action_bitmap_ndx;
  ARAD_PP_IHB_TCAM_ACTION_TBL_DATA
      tbl_data_hit_bit,
    tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  *hit_bit = 0;

  ARAD_TCAM_ACTION_clear(action);


  res = arad_tcam_tbl_calc_action_banks_and_lines(unit, bank_id, base_address, &act_action_bitmap_ndx, line, action_tbl);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit);

  /* Run over each word and get it from the corresponding table */
  for (action_ndx = 0, word = 0; action_ndx < ARAD_TCAM_NOF_ACTION_SIZES; ++action_ndx)
  {
    action_bit = (1 << action_ndx);

    if (act_action_bitmap_ndx & action_bit)
    {
      res = arad_pp_ihb_tcam_action_tbl_get_unsafe(
              unit,
              action_tbl[action_ndx],            
              line[action_ndx],
              FALSE /* is_for_hit_bit */,
              &tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      res = arad_pp_ihb_tcam_action_tbl_get_unsafe(
              unit,
              action_tbl[action_ndx],            
              line[action_ndx],
              TRUE /* is_for_hit_bit */,
              &tbl_data_hit_bit
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      /* hitbit is set to 1 if one of the table has a 1 value */
      *hit_bit |= tbl_data_hit_bit.action;

      res = arad_tcam_action_word_recover(
            unit,
            &tbl_data,
            word,
            action
          );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

      word++;
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_tbl_action_read()", bank_id, base_address);
}

#ifdef BROADCOM_DEBUG
STATIC
  uint32
    arad_tcam_db_exists_assert(
      SOC_SAND_IN int unit,
      SOC_SAND_IN uint32 tcam_db_id
    )
{
  uint8
    valid;
  uint32
    res;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.valid.get(
          unit,
          tcam_db_id,
          &valid
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);
  
  if (!valid)
  {
    LOG_ERROR(BSL_LS_SOC_TCAM,
              (BSL_META_U(unit,
                          "Unit %d Tcam DB id %d - The data base doesn\'t exist.\n\r"), unit, tcam_db_id));
    SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_DOESNT_EXIST_ERR, 10, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_exists_assert", 0, 0);
}
#endif /* BROADCOM_DEBUG */

#ifdef BROADCOM_DEBUG
STATIC
  uint32
    arad_tcam_db_entry_exists_assert(
      SOC_SAND_IN int unit,
      SOC_SAND_IN uint32 tcam_db_id,
      SOC_SAND_IN uint32 entry_id
    )
{
    SOC_SAND_HASH_TABLE_PTR
      hash_tbl;
    uint8
        found,
      hash_key[ARAD_TCAM_DB_HASH_TBL_KEY_SIZE];
    uint32
        res,
        data_indx;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.entry_id_to_location.get(unit, &hash_tbl);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);
  arad_tcam_db_entry_id_encode(tcam_db_id, entry_id, hash_key);
  res = soc_sand_hash_table_entry_lookup(
          unit,
          hash_tbl,
          hash_key,
          &data_indx,
          &found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  if (!found)
  {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Entry %d DB %d - Failed to find it in the hash table. Invalid entry.\n\r"), entry_id, tcam_db_id));
       SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_ENTRY_NDX_OUT_OF_RANGE_ERR, 20, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_exists_assert", 0, 0);
}
#endif /* BROADCOM_DEBUG */


STATIC
  uint32
    arad_tcam_db_has_banks(
      SOC_SAND_IN  int    unit,
      SOC_SAND_IN  uint32 tcam_db_id,
      SOC_SAND_OUT uint8  *has_bank
    )
{
  uint32
    bank_id,
    res;
  uint8
    is_used;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  *has_bank = FALSE;

  for(bank_id = 0; bank_id < SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit); ++bank_id)
  {
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(unit, tcam_db_id, bank_id, &is_used);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
    if (is_used)
    {
      *has_bank = TRUE;
      ARAD_DO_NOTHING_AND_EXIT;
    }
  }

  exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_has_banks()", 0, 0);
}

/* 
 * Given a bank, DB, and direction, get the 
 * next bank to consider 
 */
STATIC
  uint32
    arad_tcam_db_next_closest_bank_get(
      SOC_SAND_IN  int             unit,
      SOC_SAND_IN  uint32             tcam_db_id,
      SOC_SAND_IN  ARAD_TCAM_DIRECTION direction,
      SOC_SAND_IN  uint32               bank_id_curr,
      SOC_SAND_IN  uint8                has_db_entries, /* 0: next bank, 1: next bank where this DB has entries */
      SOC_SAND_OUT uint32               *bank_id_next,
      SOC_SAND_OUT uint8                *found
    )
{
  uint32
      bank_iter,
      nof_banks_to_consider,
      bank_min,
      bank_max,
      nof_banks_jump,
      nof_entries,
      res,
      small_bank_factor;
  ARAD_TCAM_BANK_ENTRY_SIZE
    entry_size;
  ARAD_TCAM_SMALL_BANKS
    use_small_banks;
  int32
    bank_curr; /* allow negative values */

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit, tcam_db_id, &entry_size);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  nof_banks_jump = (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS)? 2: 1;
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.use_small_banks.get(unit, tcam_db_id, &use_small_banks);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
  nof_banks_to_consider = (use_small_banks? SOC_DPP_DEFS_GET(unit, nof_tcam_small_banks): SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit)) / nof_banks_jump;
  bank_min = (use_small_banks? SOC_DPP_DEFS_GET(unit, nof_tcam_big_banks): 0);
  bank_max = SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit) - 1;
  /* if we use small bank then start iterations from the small bank id => bank_min */
  small_bank_factor = (use_small_banks? (bank_min - 1) : 0);

  *bank_id_next = 0;
  *found = FALSE;
  for (bank_iter = 1; (bank_iter < nof_banks_to_consider) && (*found == FALSE); bank_iter++)
  {
      /* Get the next bank to consider */
      bank_curr = (direction == ARAD_TCAM_DIRECTION_BACKWARD)? (bank_id_curr + small_bank_factor - (nof_banks_jump * bank_iter)): (bank_id_curr + small_bank_factor + (nof_banks_jump * bank_iter));
      if ((bank_curr < bank_min) || (bank_curr > bank_max))
      {
          /* Not found */
          *found = FALSE;
          break;
      }
      else if (has_db_entries) {
          res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_nof_entries.get(unit, tcam_db_id, bank_curr, &nof_entries);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

          if(nof_entries > 0) {
              /* Valid bank */
              *found = TRUE;
              *bank_id_next = bank_curr;
              break;
          }
      }
      else if (!has_db_entries) {
          /* No condition if DB has entry, take this bank */
          *found = TRUE;
          *bank_id_next = bank_curr;
          break;
      }
      /* Valid bank but no entry, continue */
  }

  ARAD_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_next_closest_bank_get()", 0, 0);
}


/* 
 * Given a location, DB, and direction, get the 
 * next closest line or next closest bank to consider 
 */
STATIC
  uint32
    arad_tcam_db_next_closest_location_get(
      SOC_SAND_IN  int             unit,
      SOC_SAND_IN  uint32             tcam_db_id,
      SOC_SAND_IN  ARAD_TCAM_DIRECTION direction,
      SOC_SAND_IN  ARAD_TCAM_LOCATION      *location,
      SOC_SAND_IN  uint8                has_db_entries, /* 0: next bank, 1: next bank where this DB has entries */
      SOC_SAND_OUT ARAD_TCAM_LOCATION        *next_location,
      SOC_SAND_OUT uint8                *found
    )
{
  uint32
    nof_entries_max_in_bank,
    nof_entries,
    res = SOC_SAND_OK;
  uint8
      go_to_next_bank;
  ARAD_TCAM_BANK_ENTRY_SIZE
    entry_size;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit, tcam_db_id, &entry_size);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 3, exit);
  nof_entries_max_in_bank = arad_tcam_bank_entry_size_to_entry_count_get(unit, entry_size, location->bank_id);
  go_to_next_bank = (direction == ARAD_TCAM_DIRECTION_BACKWARD)? (location->entry == 0): (location->entry == (nof_entries_max_in_bank - 1));
  if (has_db_entries)
  {
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_nof_entries.get(unit, tcam_db_id, location->bank_id, &nof_entries);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);
      if(nof_entries == 0) {
          /* No entry for this DB in this bank, go to next bank */
          go_to_next_bank = TRUE;
      }
  }

  /* Get next entry */
  if (!go_to_next_bank)
  {
      /* Not last entry of the bank, so next entry is the next line */
      next_location->bank_id = location->bank_id;
      next_location->entry = (direction == ARAD_TCAM_DIRECTION_BACKWARD)? (location->entry - 1): (location->entry + 1);
      *found = TRUE;
  }
  else
  {
      /* Get the next bank to consider */
      res = arad_tcam_db_next_closest_bank_get(
                unit,
                tcam_db_id,
                direction,
                location->bank_id,
                TRUE /* has_db_entries */, /* next bank where this DB has entries */
                &(next_location->bank_id),
                found
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
      if (*found == TRUE)
      {
          /* Set the first line */
          nof_entries_max_in_bank = arad_tcam_bank_entry_size_to_entry_count_get(unit, entry_size, next_location->bank_id);
          next_location->entry = (direction == ARAD_TCAM_DIRECTION_BACKWARD)? (nof_entries_max_in_bank - 1): 0;
      }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_next_closest_location_get()", 0, 0);
}


/* 
 * Get the closest entry of this DB according to: 
 * the DB ID, the free location and the direction. 
 * If an entry is located in the same location, it does NOT choose it. 
 */
STATIC
  uint32
    arad_tcam_db_occ_bmp_entry_closest_get(
      SOC_SAND_IN  int             unit,
      SOC_SAND_IN  uint32             tcam_db_id,
      SOC_SAND_IN  ARAD_TCAM_DIRECTION direction,
      SOC_SAND_IN  ARAD_TCAM_LOCATION      *location,
      SOC_SAND_OUT uint32                    *entry_id,
      SOC_SAND_OUT uint32                    *next_priority,
      SOC_SAND_OUT ARAD_TCAM_LOCATION        *next_location,
      SOC_SAND_OUT uint8                *found
    )
{
  uint32
      global_location_id,
      place_first_free,
      nof_entries_max_in_bank,
      curr_line,
      bank_ndx,
      bank_id,
      nof_banks,
      nof_entries,
    res = SOC_SAND_OK;
  SOC_SAND_OCC_BM_PTR
    bank_bm;
  ARAD_TCAM_BANK_ENTRY_SIZE
    entry_size;
  ARAD_TCAM_GLOBAL_LOCATION 
      global_location;
  uint8
      is_found;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit, tcam_db_id, &entry_size);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);

  /* 
   * Loop on the closest banks to find the first entry 
   * of this database according to the occupation bitmap 
   * (per DB, per bank). 
   * use for loop instead of while to limit to the max nof banks 
   */
  *found = FALSE;
  is_found = FALSE;

  /* Get the first location to consider */
  ARAD_TCAM_LOCATION_clear(next_location);
  res = arad_tcam_db_next_closest_location_get(
            unit,
            tcam_db_id,
            direction,
            location,
            1, /* 1: first bank where this DB has entries */
            next_location,
            &is_found
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (is_found == TRUE)
  {
      /* Get the exact location */
      nof_banks = (direction == ARAD_TCAM_DIRECTION_BACKWARD)?
          (next_location->bank_id + 1) :(SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit) - next_location->bank_id);
      is_found = FALSE;
      for (bank_ndx = 0; (bank_ndx < nof_banks) && (is_found == FALSE); bank_ndx++) {
          bank_id = (direction == ARAD_TCAM_DIRECTION_FORWARD)? (next_location->bank_id + bank_ndx): (next_location->bank_id - bank_ndx);
          res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_nof_entries.get(unit, tcam_db_id, bank_id, &nof_entries);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
          if (nof_entries == 0) {
              /* Consider only banks with entries */
              continue;
          }
          nof_entries_max_in_bank = arad_tcam_bank_entry_size_to_entry_count_get(unit, entry_size, bank_id);
          res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db_entries_used.get(unit, tcam_db_id, bank_id, &bank_bm);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
          curr_line = (bank_id == next_location->bank_id)? next_location->entry :
              ((direction == ARAD_TCAM_DIRECTION_FORWARD)? 0: (nof_entries_max_in_bank - 1));
          res = soc_sand_occ_bm_get_next_in_range(
                  unit,
                  bank_bm,
                  ((direction == ARAD_TCAM_DIRECTION_BACKWARD)? 0: curr_line), /* start */
                  ((direction == ARAD_TCAM_DIRECTION_BACKWARD)? curr_line: (nof_entries_max_in_bank - 1)), /* end */
                  (direction == ARAD_TCAM_DIRECTION_FORWARD)? TRUE : FALSE,
                  &place_first_free,
                  &is_found
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
          if (is_found)
          {
              next_location->bank_id = bank_id;
              next_location->entry = place_first_free;

              /* Retrieve the entry attributes from the global location table */
              res = arad_tcam_global_location_encode(unit, next_location, &global_location_id);
              SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);
              res = sw_state_access[unit].dpp.soc.arad.tm.tcam.global_location_tbl.get(unit, global_location_id, &global_location);
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
              *entry_id = global_location.entry_id;
              *next_priority = global_location.priority;

              /* Check consistency */
    #ifdef BROADCOM_DEBUG
              if (global_location.tcam_db_id != tcam_db_id)
              {
                LOG_ERROR(BSL_LS_SOC_TCAM,
                          (BSL_META_U(unit,
                                      "Unit %d Tcam DB id %d Entry id %d is stored in location (bank %d, entry %d), "
                                      "but we were looking for next entry of TCAM DB %d from location (bank %d, entry %d) with direction %d.\n\r"),
                           unit, global_location.tcam_db_id, *entry_id, next_location->bank_id, next_location->entry,
                           tcam_db_id, location->bank_id, location->entry, direction));
                SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 50, exit);
              }
    #endif /* BROADCOM_DEBUG */
          }
      }
  }

  *found = is_found;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_occ_bmp_entry_closest_get()", 0, 0);
}

/* 
 * According to this priority, find the admissible range of 
 * locations to insert: 
 * - Find the last entry with lower (or equal) priority as lowest 
 * bound. If there is not, set (bank, line) = (0, 0) 
 * - Do the same for the uppest bound, with (#banks-1, #lines-1) if 
 * not found 
 */
STATIC
  uint32
    arad_tcam_db_priority_list_insertion_range_find(
      SOC_SAND_IN  int     unit,
      SOC_SAND_IN  uint32     tcam_db_id,
      SOC_SAND_IN  uint32      priority,
      SOC_SAND_IN  uint8      is_for_update,
      SOC_SAND_IN  uint32     entry_id,
      SOC_SAND_OUT ARAD_TCAM_RANGE *range
    )
{
  uint32
      entry_id_prev,
      priority_prev,
      entry_id_next,
      priority_next,
    res = SOC_SAND_OK;
  SOC_SAND_SORTED_LIST_PTR
    list;
  SOC_SAND_SORTED_LIST_ITER
    current,
    found_node;
  uint8
      found_prev,
      found_next,
    priority_found,
    found,
    key_buffer_current[ARAD_TCAM_DB_LIST_KEY_SIZE],
    data_buffer[ARAD_TCAM_DB_LIST_DATA_SIZE],
    insertion_rule_respected;
  uint32
    node_priority = 0;
  ARAD_TCAM_LOCATION
      location_prev,
      location_next,
      location;
  ARAD_TCAM_PRIO_LOCATION
      prio_location;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(range);
  ARAD_TCAM_RANGE_clear(range);

  ARAD_TCAM_DB_EXISTS_ASSERT;


  /* 
   * For update, find the allowed range from previous to next node
   */
  res = arad_tcam_db_block_entry_insertion_rule_respected_get(unit, tcam_db_id, &insertion_rule_respected);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (is_for_update && insertion_rule_respected) {
      /* Get the location */
      res = arad_tcam_db_entry_id_to_location_entry_get(
              unit,
              tcam_db_id,
              entry_id,
              TRUE,
              &location,
              &found
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);
      if (!found)
      {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit,
                              "Unit %d Tcam db id %d Entry id %d - The entry is for update and it hasn't been found.\n\r"),
                   unit, tcam_db_id, entry_id));
        SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 18, exit);
      }

      /* Get previous and next entry in this DB */
      res = arad_tcam_db_occ_bmp_entry_closest_get(
                unit,
                tcam_db_id,
                ARAD_TCAM_DIRECTION_BACKWARD,
                &location,
                &entry_id_prev,
                &priority_prev,
                &location_prev,
                &found_prev
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 14, exit);

      res = arad_tcam_db_occ_bmp_entry_closest_get(
                unit,
                tcam_db_id,
                ARAD_TCAM_DIRECTION_FORWARD,
                &location,
                &entry_id_next,
                &priority_next,
                &location_next,
                &found_next
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 16, exit);
  }
  else {
      /* Get the priority list */
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db_priorities.get(unit, tcam_db_id, &list);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);

      /*
       *  Find the lower bound (the last entry in the database before the range of entries with the
       *  same priority as the new entry, or the beginning of the database)
       *  Loop on the priority of the Database: a lookup on the priority will do the same.
       */
      priority_found = FALSE;
      /*
       * Scan the list from the end - faster in TCAM for sorted entries
       */
      current  = SOC_SAND_SORTED_LIST_ITER_END(unit,list);
      res = soc_sand_sorted_list_get_prev( 
              unit,
              list,
              &current,
              key_buffer_current,
              data_buffer
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

      found_prev = FALSE ;
      do
      {
        uint32 num_elements ;
        uint8 is_indices_enabled ;
        res = soc_sand_sorted_list_is_indices_enabled(unit, list,&is_indices_enabled) ;
        SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit) ;
        /*
         * If 'indices' feature (of sorted lists) is not enabled for this sorted
         * list then search one-by-one.
         */ 
        if (!is_indices_enabled)
        {
          /*
           * Loop until the beginning of the list is found (or when match is detected. Match
           * criterion depends on 'insertion_rule_respected')
           */
          while (!priority_found && (current != SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,list))) 
          {
            node_priority = arad_tcam_db_prio_list_priority_value_decode(key_buffer_current);
            /* 
             * Insertion-order rule:
             *  If 'insertion_rule_respected' then
             *    Stop the search for the 1st node whose priority is strictly lower
             *    than that of the entry to insert
             *  else
             *    Stop the search for the 1st node whose priority is lower or equal
             *    than that of the entry to insert
             */
            if ( (!insertion_rule_respected && (priority >= node_priority))
                || ((insertion_rule_respected) && (priority > node_priority)))
            {
                /* coverity[assigned_value:FALSE] */
                priority_found = TRUE;
                break;
            }
            else
            {
              res = soc_sand_sorted_list_get_prev( 
                      unit,
                      list,
                      &current,
                      key_buffer_current,
                      data_buffer
                    );
              SOC_SAND_CHECK_FUNC_RESULT(res, 24, exit);
            }
          }
        }
        else
        {
          /*
           * 'Indices' feature is enabled. Use binary search.
           */
          uint32
            index_on_indices,
            bottom_index_on_indices,
            top_index_on_indices ;
          /* 
           * Insertion-order rule:
           *  If 'insertion_rule_respected' then
           *    Stop the search for the 1st node whose priority is strictly lower
           *    than that of the entry to insert
           *  else
           *    Stop the search for the 1st node whose priority is lower or equal
           *    than that of the entry to insert
           */
          res = soc_sand_sorted_list_get_num_elements(unit,list,&num_elements) ;
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ;
          if (num_elements == 0)
          {
            /*
             * List is empty.
             * Return 'priority_found' set to FALSE and with 'current' set to 'one
             * before first'.
             */
            current  = SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,list);
            break ;
          }
          /*
           * Search starts from top ('current' is the last (largest priority) element on the sorted list)
           */
          node_priority = arad_tcam_db_prio_list_priority_value_decode(key_buffer_current) ;
          if ( (!insertion_rule_respected && (priority >= node_priority))
              || ((insertion_rule_respected) && (priority > node_priority)) )
          {
            /*
             * Highest element on list is lower or equal (or strictly lower if 'insertion_rule_respected')
             * than input priority. Search has ended.
             */
            /* coverity[assigned_value] */
            priority_found = TRUE ;
            break ;
          }
          /*
           * Get and compare with lowest element.
           */
          res = soc_sand_sorted_list_get_iter_from_indices(unit,list,0,&current) ;
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit) ;
          res = soc_sand_sorted_list_entry_value(unit,list,current,key_buffer_current,data_buffer) ;
          SOC_SAND_CHECK_FUNC_RESULT(res, 34, exit) ;
          node_priority = arad_tcam_db_prio_list_priority_value_decode(key_buffer_current) ;
          if ( (!insertion_rule_respected && (priority < node_priority))
              || ((insertion_rule_respected) && (priority <= node_priority)) )
          {
            /*
             * First element is higher than input priority. There is no element on
             * this sorted list which is lower than input priority.
             * Return 'priority_found' set to FALSE and with 'current' set to 'one
             * before first'.
             */
            current  = SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,list);
            break ;
          }
          /*
           * At this point, a matching element MUST be somewhere within this sorted list.
           * If none is found then this is an ERROR.
           */
          top_index_on_indices = num_elements - 1 ;
          bottom_index_on_indices = 0 ;
          while (!priority_found)
          {
            index_on_indices = (top_index_on_indices - bottom_index_on_indices) / 2 ;
            if (index_on_indices == 0)
            {
              /*
               * We have one or two elements to check:
               */
              res = soc_sand_sorted_list_get_iter_from_indices(unit,list,top_index_on_indices,&current) ;
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 36, exit) ;
              res = soc_sand_sorted_list_entry_value(unit,list,current,key_buffer_current,data_buffer) ;
              SOC_SAND_CHECK_FUNC_RESULT(res, 38, exit) ;
              node_priority = arad_tcam_db_prio_list_priority_value_decode(key_buffer_current) ;
              if ( (!insertion_rule_respected && (priority >= node_priority))
                  || ((insertion_rule_respected) && (priority > node_priority)) )
              {
                /*
                 * The higher of the two elements on list is lower or equal (or strictly lower if 'insertion_rule_respected')
                 * than input priority. Search has ended.
                 */
                priority_found = TRUE ;
                break ;
              }
              res = soc_sand_sorted_list_get_iter_from_indices(unit,list,bottom_index_on_indices,&current) ;
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit) ;
              res = soc_sand_sorted_list_entry_value(unit,list,current,key_buffer_current,data_buffer) ;
              SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit) ;
              node_priority = arad_tcam_db_prio_list_priority_value_decode(key_buffer_current) ;
              if ( (!insertion_rule_respected && (priority >= node_priority))
                  || ((insertion_rule_respected) && (priority > node_priority)) )
              {
                /*
                 * The lower of the two elements on list is lower or equal  (or strictly lower if 'insertion_rule_respected')
                 * than input priority. Search has ended.
                 */
                priority_found = TRUE ;
                break ;
              }
              /*
               * Failed to find matching element. Error!
               */
              res = SOC_SAND_GET_ERR_TXT_ERR ;
              SOC_SAND_CHECK_FUNC_RESULT(res, 44, exit) ;
              /*
               * This 'break' is added here just to make 'coverity' happy...
               */
              break ;
            }
            index_on_indices += bottom_index_on_indices ;
            /*
             * There are more-than-one elements to be inspected.
             */
            res = soc_sand_sorted_list_get_iter_from_indices(unit,list,index_on_indices,&current) ;
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 46, exit) ;
            res = soc_sand_sorted_list_entry_value(unit,list,current,key_buffer_current,data_buffer) ;
            SOC_SAND_CHECK_FUNC_RESULT(res, 48, exit) ;
            node_priority = arad_tcam_db_prio_list_priority_value_decode(key_buffer_current) ;
            if (!insertion_rule_respected && (priority == node_priority))
            {
              /*
               * Found element which has equal priority and 'insertion_rule_respected' is NOT set.
               * This is, then, NOT a hit. Keep searching on higher values (upper half) for more
               * equal elements.
               * Bisect the upper half to see if there is another equal element.
               * (Note that this is a general approach but, for our case, there will not be more
               * than one element, on the sorted list, with the same priority).
               */
              bottom_index_on_indices = index_on_indices ;
              continue ;
            }
            /*
             * Current element either does NOT have equal priority or 'insertion_rule_respected'
             * is set.
             */
            if (priority > node_priority)
            {
              /*
               * Middle element which is lower than input. Bisect the upper half.
               */
              bottom_index_on_indices = index_on_indices ;
              continue ;
            }
            else
            {
              /*
               * Middle element which is higher or equal than input. Bisect the lower half.
               */
              top_index_on_indices = index_on_indices ;
              continue ;
            }
          }
          if (!priority_found)
          {
            /*
             * If no match was found then return with 'current' set to 'one
             * before first'.
             */
            current  = SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,list);
          }
        }
      } while (0) ;

      found_node = current;
      if ( priority_found && (found_node != SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,list)) ) { 
          found_prev = TRUE;
          /* Retrieve the entry attributes from previous */
          res = soc_sand_sorted_list_entry_value(
                  unit,
                  list,
                  found_node,
                  key_buffer_current,
                  data_buffer
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 56, exit);

          res = arad_tcam_db_priority_data_decode(unit, tcam_db_id, data_buffer, &prio_location);
          SOC_SAND_CHECK_FUNC_RESULT(res, 58, exit);

          res = arad_tcam_db_entry_id_to_location_entry_get(
                    unit,
                    tcam_db_id,
                    prio_location.entry_id_last,
                    TRUE,
                    &location_prev,
                    &found_prev
                  );
          SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
          if (!found_prev) {
               LOG_ERROR(BSL_LS_SOC_TCAM,
                         (BSL_META_U(unit,
                                     "Unit %d DB id %d  Entry-ID %d not found altough defined in priority range %d.\n\r"), 
                          unit, tcam_db_id, prio_location.entry_id_last, node_priority));
               SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_BANK_UNINITIALIZED_ERR, 10, exit);
          }
      }
      else {
          /* Break here */
          found_prev = FALSE;
      }


      /*
       *  Continue the list traversal to find the upper bound (the first entry with a higher priority
       *  than the new entry, or the end of the database)
       */
      priority_found = FALSE;
      current = found_node;

      res = soc_sand_sorted_list_get_next(
              unit,
              list,
              &current,
              key_buffer_current,
              data_buffer
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 604, exit);
      found_next = FALSE;
      while (!priority_found && (current != SOC_SAND_SORTED_LIST_ITER_END(unit,list)))
      {
        node_priority = arad_tcam_db_prio_list_priority_value_decode(key_buffer_current);
        if (priority < node_priority) 
        {
            if ( current != SOC_SAND_SORTED_LIST_ITER_END(unit,list))  { 
              priority_found = TRUE;
              found_next = TRUE;
              /* Retrieve the entry attributes */
              res = arad_tcam_db_priority_data_decode(unit, tcam_db_id, data_buffer, &prio_location);
              SOC_SAND_CHECK_FUNC_RESULT(res, 66, exit);

              res = arad_tcam_db_entry_id_to_location_entry_get(
                        unit,
                        tcam_db_id,
                        prio_location.entry_id_first,
                        TRUE,
                        &location_next,
                        &found_next
                      );
              SOC_SAND_CHECK_FUNC_RESULT(res, 68, exit);
              if (!found_next) {
                   LOG_ERROR(BSL_LS_SOC_TCAM,
                             (BSL_META_U(unit,
                                         "Unit %d DB id %d  Entry-ID %d not found altough defined in priority range %d.\n\r"), 
                              unit, tcam_db_id, prio_location.entry_id_last, node_priority));
                   SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_BANK_UNINITIALIZED_ERR, 72, exit);
              }
            }
            else {
                /* Break here */
                found_next = FALSE;
                break;
            }
        }
        else
        {
          res = soc_sand_sorted_list_get_next(
                  unit,
                  list,
                  &current,
                  key_buffer_current,
                  data_buffer
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 74, exit);
        }
      }
  }

  /* 
   * Decode in both cases (for update or not) the range
   */
  if (found_prev == TRUE)
  {
      range->min.bank_id = location_prev.bank_id;
      range->min.entry = location_prev.entry;
  }
  else
  {
    range->min.bank_id = 0;
    range->min.entry   = 0;
    range->min_not_found = TRUE;
  }

  if (found_next == TRUE)
  {
      range->max.bank_id = location_next.bank_id;
      range->max.entry = location_next.entry;
  }
  else
  {
    range->max.bank_id = SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit) - 1;
    res = arad_tcam_db_entries_per_bank_get(unit, tcam_db_id, range->max.bank_id, &range->max.entry);
    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
    --(range->max.entry);
    range->max_not_found = TRUE;
  }

    /* Check consistency */
#ifdef BROADCOM_DEBUG
  if ((range->min.bank_id > range->max.bank_id) || ((range->min.bank_id == range->max.bank_id) && (range->min.entry > range->max.entry)))
  {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Unit %d Tcam DB id %d Entry %d Priority %d Is-for-update %d has received an illegal "
                             "location range from (bank %d, entry %d) to (bank %d, entry %d).\n\r"),
                  unit, tcam_db_id, entry_id, priority, is_for_update, range->min.bank_id, range->min.entry, range->max.bank_id, range->max.entry));
       SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 50, exit);
  }
#endif /* BROADCOM_DEBUG */


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_priority_list_insertion_range_find", tcam_db_id, priority);
}

STATIC
  uint32
    arad_tcam_bank_entry_set_unsafe(
      SOC_SAND_IN  int            unit,
      SOC_SAND_IN uint8         set_hw_also,
      SOC_SAND_IN uint8         is_bank_share_update,
      SOC_SAND_IN uint8         hit_bit,
      SOC_SAND_IN  uint32            bank_id,
      SOC_SAND_IN  uint32             address,
      SOC_SAND_IN  ARAD_TCAM_ACTION_SIZE action_bitmap_ndx,
      SOC_SAND_IN  ARAD_TCAM_ENTRY    *entry,
      SOC_SAND_IN  ARAD_TCAM_ACTION   *action
   )
{
  uint32
    res = SOC_SAND_OK,
    entries_free = 0;
  ARAD_TCAM_BANK_ENTRY_SIZE
    entry_size = 0;
  SOC_SAND_OCC_BM_PTR
    occ_bm;
  uint8
      is_inverse,
      is_valid;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.valid.get(unit, bank_id, &is_valid);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);
  
#ifdef CRASH_RECOVERY_SUPPORT
  /* add an entry to the transaction database */
  if(BCM_UNIT_DO_HW_READ_WRITE(unit)) {
    arad_tcam_access_cr_entry_add(unit, bank_id, address, hit_bit, entry, action);
  }
#endif

  if (!is_valid)
  {
    LOG_ERROR(BSL_LS_SOC_TCAM,
              (BSL_META_U(unit,
                          "Unit %d Bank id %d - Trying to insert entry to uninitialized bank.\n\r"), unit, bank_id));
    SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_BANK_UNINITIALIZED_ERR, 10, exit);
  }

  /* Set also the HW - not for block-move */
  if (set_hw_also) {
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.entry_size.get(
              unit,
              bank_id,
              &entry_size
            );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 15, exit); 
      /* Set the action before the HW to avoid traffic influence for valid entries */
      if (entry->valid) {
          res = arad_tcam_tbl_action_write(
                  unit,
                  bank_id, 
                  address,
                  action_bitmap_ndx,     
                  hit_bit,     
                  action
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

          res = arad_tcam_tbl_bank_write(
                  unit,
                  bank_id,
                  entry_size,
                  address,
                  entry
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit);
      }
      else { /* Invalid entries: invalid bit first */
          res = arad_tcam_tbl_bank_write(
                  unit,
                  bank_id,
                  entry_size,
                  address,
                  entry
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 24, exit);

          res = arad_tcam_tbl_action_write(
                  unit,
                  bank_id, 
                  address,
                  action_bitmap_ndx,          
                  hit_bit,     
                  action
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 26, exit);
      }
  }

  /*
   *  Update the occupation bitmaps
   */
  for (is_inverse = 0; is_inverse < ARAD_TCAM_NOF_OCCUPATIONS_BITMAPS; is_inverse++) {
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_bank_entries_used.get(
                 unit,
                 bank_id,
                 is_inverse,
                 &occ_bm
               );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
      res = soc_sand_occ_bm_occup_status_set(
              unit,
              occ_bm,
              address,
              is_inverse? FALSE: TRUE
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  }

  if (!is_bank_share_update) {
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.nof_entries_free.get(
              unit, 
              bank_id,
              &entries_free);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);

      if(0 == entries_free)
      {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit,
                              "Unit %d Bank id %d - Trying to insert entry to full bank.\n\r"), unit, bank_id));
        SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_BANK_ENTRY_SIZE_OUT_OF_RANGE_ERR, 50, exit);
      }
      entries_free--;
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.nof_entries_free.set(
        unit, 
        bank_id, 
        entries_free);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_bank_entry_set_unsafe()", bank_id, address);
}

STATIC
  uint32
    arad_tcam_bank_entry_get_unsafe(
      SOC_SAND_IN  int               unit,
      SOC_SAND_IN  uint32               bank_id,
      SOC_SAND_IN  uint32               address,
      SOC_SAND_IN  ARAD_TCAM_ACTION_SIZE action_bitmap_ndx,
      SOC_SAND_IN  uint8                hit_bit_clear, /* If TRUE, clear the hit bit once read */
      SOC_SAND_OUT ARAD_TCAM_ENTRY      *entry,
      SOC_SAND_OUT ARAD_TCAM_ACTION     *action,
      SOC_SAND_OUT uint8                *hit_bit
    )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_TCAM_BANK_ENTRY_SIZE
    entry_size;
  uint8
    is_valid;
#ifdef CRASH_RECOVERY_SUPPORT
  uint8
    found;
#endif

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.valid.get(unit, bank_id, &is_valid);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);
  
  if (!is_valid)
  {
    LOG_ERROR(BSL_LS_SOC_TCAM,
              (BSL_META_U(unit,
                          "Unit %d Band id %d - Failed to get tcam entry bank. The bank is not valid.\n\r"), unit, bank_id));
    SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_BANK_UNINITIALIZED_ERR, 10, exit);
  }

#ifdef CRASH_RECOVERY_SUPPORT
  if(BCM_UNIT_DO_HW_READ_WRITE(unit)) {
    found = FALSE;
    arad_tcam_access_cr_entry_get(unit, bank_id, address, hit_bit, entry, action, &found);
    /* if an entry is not found, proceed to retrieve from hardware */
    if(found) {
        return SOC_E_NONE;
    }
}
soc_hw_set_immediate_hw_access(unit);
#endif

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.entry_size.get(
          unit,
          bank_id,
          &entry_size
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 15, exit);
  
  res = arad_tcam_tbl_bank_read(
          unit,
          bank_id,
          entry_size,
          address,
          entry
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  res = arad_tcam_tbl_action_read(
          unit,
          bank_id, 
          address, 
          action_bitmap_ndx,         
          hit_bit,
          action
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  /* clear the hit table if necessary */
  if (hit_bit_clear) {
      res = arad_tcam_tbl_action_write(
              unit,
              bank_id, 
              address,
              action_bitmap_ndx,     
              0, /* Unset the hit bit */
              action
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 33, exit);
  }

exit:
#ifdef CRASH_RECOVERY_SUPPORT
  soc_hw_restore_immediate_hw_access(unit);
#endif

  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_bank_entry_get_unsafe()", bank_id, address);
}

STATIC
  uint32
    arad_tcam_bank_entry_invalidate_unsafe(
      SOC_SAND_IN int unit,
      SOC_SAND_IN uint8         set_hw_also,
      SOC_SAND_IN uint8         is_bank_share_update,
      SOC_SAND_IN uint32 bank_id,
      SOC_SAND_IN uint32  address
    )
{
  uint32
    res = SOC_SAND_OK,
      nof_entries_in_line,
    entries_free;
  SOC_SAND_OCC_BM_PTR
    occ_bm;
  ARAD_TCAM_BANK_ENTRY_SIZE
    entry_size;
  uint8
      is_inverse,
      is_valid;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.valid.get(unit, bank_id, &is_valid);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);
  if (!is_valid)
  {
    LOG_ERROR(BSL_LS_SOC_TCAM,
              (BSL_META_U(unit,
                          "Unit %d Band id %d - Failed to invalidate tcam entry bank. The bank is not valid.\n\r"), unit, bank_id));
    SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_BANK_UNINITIALIZED_ERR, 10, exit);
  }

#ifdef CRASH_RECOVERY_SUPPORT
  /* delete the entry from the transaction database */
  if(BCM_UNIT_DO_HW_READ_WRITE(unit)) {
    arad_tcam_access_cr_entry_remove(unit, bank_id, address);
  }
#endif

  /*
   *  Mark the entry as available
   */
  for (is_inverse = 0; is_inverse < ARAD_TCAM_NOF_OCCUPATIONS_BITMAPS; is_inverse++) {
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_bank_entries_used.get(
                 unit,
                 bank_id,
                 is_inverse,
                 &occ_bm
               );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

      res = soc_sand_occ_bm_occup_status_set(
              unit,
              occ_bm,
              address,
              is_inverse? TRUE: FALSE
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 16, exit);
  }
  if (!is_bank_share_update) {
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.nof_entries_free.get(
              unit, 
              bank_id,
              &entries_free
            );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 17, exit);

      entries_free++;
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.nof_entries_free.set(
        unit, 
        bank_id, 
        entries_free);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);
  }

  /*
   *  Invalidate the hardware entry
   */
  if (set_hw_also) {
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.entry_size.get(
              unit,
              bank_id,
              &entry_size
            );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 19, exit);
      nof_entries_in_line = (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) ? 3 : 
                              ((entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS) ? 2 : 1);

      res = arad_pp_ihb_tcam_invalid_bit_set_unsafe(
              unit,
              bank_id,
              nof_entries_in_line,
              address
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_bank_entry_invalidate_unsafe()", bank_id, address);
}

/* 
 * In regular priority type, see the start and 
 * end lines you can consider in the range: 
 * - 0 if the bank is not in the range 
 * - different if the min/max-bank == current-bank 
 */ 
STATIC
  void
    arad_tcam_db_bit_search_range_in_bank_mode_get(
      SOC_SAND_IN  ARAD_TCAM_RANGE *range,
      SOC_SAND_IN  uint32     bank_id,
      SOC_SAND_IN  uint32     entry_count,
      SOC_SAND_OUT uint32      *start,
      SOC_SAND_OUT uint32      *end,
      SOC_SAND_OUT uint8     *skip
    )
{
  if (bank_id < range->min.bank_id || bank_id > range->max.bank_id)
  {
    *skip = TRUE;
  }
  else
  {
    *skip  = FALSE;
    *start = (bank_id == range->min.bank_id) ? range->min.entry : 0;
    *end   = (bank_id == range->max.bank_id) ? range->max.entry : entry_count - 1;
  }
}

/* 
 * Find if there is free bit in this bank 
 * for this range: 
 * - See if the DB is already present in this bank 
 * - Get the number of entries and see if acceptable 
 * according to the priority type 
 */
STATIC
  uint32
    arad_tcam_db_bit_search_range_get(
      SOC_SAND_IN  int     unit,
      SOC_SAND_IN  uint32     tcam_db_id,
      SOC_SAND_IN  ARAD_TCAM_RANGE *range,
      SOC_SAND_IN  uint32     bank_id,
      SOC_SAND_OUT uint32      *start,
      SOC_SAND_OUT uint32      *end,
      SOC_SAND_OUT uint8     *skip
    )
{
  uint8
    bank_used;
  uint32
    entry_count,
    res;
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(
          unit,
          tcam_db_id,
          bank_id,
          &bank_used
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  if (!bank_used)
  {
    *skip = TRUE;
  }
  else
  {
    *skip = FALSE;
      res = arad_tcam_db_entries_per_bank_get(unit, tcam_db_id, bank_id, &entry_count);
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
      arad_tcam_db_bit_search_range_in_bank_mode_get(
        range,
        bank_id,
        entry_count,
        start,
        end,
        skip
      );
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_bit_search_range_get()", 0, 0);
}

STATIC
  uint32
    arad_tcam_db_priority_list_entry_iter_get(
      SOC_SAND_IN  int                       unit,
      SOC_SAND_IN  uint32                       tcam_db_id,
      SOC_SAND_IN  uint32                       priority,
      SOC_SAND_OUT SOC_SAND_SORTED_LIST_ITER    *iter,
      SOC_SAND_OUT uint8                        *found
    )
{
  uint32
    res = SOC_SAND_OK;
  SOC_SAND_SORTED_LIST_PTR
    prio_list;
  uint8
    key_buffer[ARAD_TCAM_DB_LIST_KEY_SIZE];
  uint8
    is_indices_enabled ;
  uint32
    index_on_indices ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  index_on_indices = -1 ;
  arad_tcam_db_prio_list_priority_value_encode(priority, key_buffer);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db_priorities.get(unit, tcam_db_id, &prio_list);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  res = soc_sand_sorted_list_is_indices_enabled(unit,prio_list,&is_indices_enabled) ;
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  /*
   * If 'indices' feature is enabled on this sorted list, then do binary search.
   */
  if (is_indices_enabled) {
    res = soc_sand_sorted_list_find_lower_eq_key(unit,prio_list,key_buffer,iter,&index_on_indices,found) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit) ;
  } else {
    res = soc_sand_sorted_list_entry_lookup(
          unit,
          prio_list,
          key_buffer,
          NULL,
          found,
          iter
        ) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit) ;
  }
exit:

  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_priority_list_entry_iter_get()", 0, 0) ;
}

STATIC
  uint32
    arad_tcam_closest_free_line_find_helper(
      SOC_SAND_IN  int         unit,
      SOC_SAND_IN  uint32         bank_id,
      SOC_SAND_IN  ARAD_TCAM_DIRECTION direction,
      SOC_SAND_IN  uint32         start,
      SOC_SAND_IN  uint32         end,
      SOC_SAND_IN  uint8          is_inserted_top,
      /* If allow_pair_same_db True, then the other 80b entry can be of same DB or empty. Otherwise, only empty */
      SOC_SAND_OUT uint32         *place_first_free,
      SOC_SAND_OUT uint32         *place_chosen,
      SOC_SAND_OUT uint8         *found
    )
{
  uint32
    res = SOC_SAND_OK;
  SOC_SAND_OCC_BM_PTR
    bank_bm;
#if ARAD_TCAM_INSERTION_ALGORITHM_MIDDLE_FREE
  uint32
      start_lcl_middle,
      end_lcl_middle,
      place_taken;
   uint8         
       found_taken;
#endif /* ARAD_TCAM_INSERTION_ALGORITHM_MIDDLE_FREE */

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_bank_entries_used.get(unit, bank_id, FALSE, &bank_bm);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
  res = soc_sand_occ_bm_get_next_in_range(
          unit,
          bank_bm,
          start,
          end,
          (direction == ARAD_TCAM_DIRECTION_FORWARD)? TRUE : FALSE,
          place_first_free,
          found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  *place_chosen = *place_first_free;

#if ARAD_TCAM_INSERTION_ALGORITHM_MIDDLE_FREE
  if ((*found) && (!is_inserted_top)) {
      /* Find the first taken bit, and take in the middle */
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_bank_entries_used.get(unit, bank_id, TRUE /* is_inverse */, &bank_bm);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
      start_lcl_middle = (direction == ARAD_TCAM_DIRECTION_FORWARD)? (*place_first_free): start;
      end_lcl_middle = (direction == ARAD_TCAM_DIRECTION_FORWARD)? end: (*place_first_free);
      res = soc_sand_occ_bm_get_next_in_range(
              unit,
              bank_bm,
              start_lcl_middle,
              end_lcl_middle,
              (direction == ARAD_TCAM_DIRECTION_FORWARD)? TRUE : FALSE,
              &place_taken,
              &found_taken
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
      *place_chosen = ARAD_TCAM_INSERTION_ALGORITHM_MIDDLE_FREE_FIND(found_taken, direction, place_taken, start_lcl_middle, end_lcl_middle);
  }
#endif /* ARAD_TCAM_INSERTION_ALGORITHM_MIDDLE_FREE */

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_closest_free_line_find_helper()", 0, direction);
}

/* Find the first free line after x free lines jumped */
STATIC
  uint32
    arad_tcam_jump_line_find(
      SOC_SAND_IN  int                       unit,
      SOC_SAND_IN  uint32                       bank_id,
      SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE    entry_size,
      SOC_SAND_IN  ARAD_TCAM_DIRECTION          direction,
      SOC_SAND_IN  uint32                       start,
      SOC_SAND_IN  uint32                       end,
      SOC_SAND_IN  uint32                       nof_free_entries_jump,
      SOC_SAND_OUT uint32                       *place_chosen,
      SOC_SAND_OUT uint8                        *found
    )
{
  uint32
    res = SOC_SAND_OK;
  SOC_SAND_OCC_BM_PTR
    bank_bm;
  uint32
      free_entry_idx,
      start_lcl,
      end_lcl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  *found = FALSE;
  start_lcl = start;
  end_lcl = end;
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_bank_entries_used.get(unit, bank_id, FALSE, &bank_bm);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
  *found = TRUE;
  if (nof_free_entries_jump == 0) {
      *found = TRUE;
      *place_chosen = (direction == ARAD_TCAM_DIRECTION_FORWARD)? start: end;
  }
  else {
      for (free_entry_idx = 0; (free_entry_idx < nof_free_entries_jump) && (*found == TRUE); free_entry_idx++) {
          res = soc_sand_occ_bm_get_next_in_range(
                  unit,
                  bank_bm,
                  start_lcl,
                  end_lcl,
                  (direction == ARAD_TCAM_DIRECTION_FORWARD)? TRUE : FALSE,
                  place_chosen,
                  found
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
          if (direction == ARAD_TCAM_DIRECTION_FORWARD) {
              start_lcl = *place_chosen + 1;
          }
          else {
              end_lcl = *place_chosen - 1;
          }
      }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_jump_line_find()", 0, direction);
}


/*
 * Find an acceptable location in a range of the occupation bitmap: 
 * - for regular entries, first one 
 * - for 80b, the whole line must be free to enable the HW move 
 */
STATIC
  uint32
    arad_tcam_closest_free_line_find(
      SOC_SAND_IN  int         unit,
      SOC_SAND_IN  uint32         tcam_db_id,
      SOC_SAND_IN  uint32         bank_id,
      SOC_SAND_IN  ARAD_TCAM_DIRECTION direction,
      SOC_SAND_IN  uint32         start,
      SOC_SAND_IN  uint32         end,
      SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE  entry_size,
      SOC_SAND_IN  uint8          is_inserted_top,
      /* If allow_pair_same_db True, then the other 80b entry can be of same DB or empty. Otherwise, only empty */
      SOC_SAND_IN  uint8          allow_pair_same_db, 
      SOC_SAND_OUT uint32         *place_first_free,
      SOC_SAND_OUT uint32         *place_chosen,
      SOC_SAND_OUT uint8         *found
    )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);


  /* take the first location free */
      res = arad_tcam_closest_free_line_find_helper(
                unit,
                bank_id,
                direction,
                start,
                end,
                is_inserted_top,
                place_first_free,
                place_chosen,
                found
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_closest_free_line_find()", tcam_db_id, direction);
}

STATIC
  uint32
    arad_tcam_db_empty_location_in_range_find(
      SOC_SAND_IN  int            unit,
      SOC_SAND_IN  uint32            tcam_db_id,
      SOC_SAND_IN  uint8                is_inserted_top,
      SOC_SAND_IN  ARAD_TCAM_RANGE        *range,
      SOC_SAND_OUT ARAD_TCAM_LOCATION     *location,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE *success
    )
{
  uint32
    res = SOC_SAND_OK,
      place_first_free,
    start_bit,
    end_bit;
  uint32
    bank_id;
  uint8
    found,
    skip;
  ARAD_TCAM_BANK_ENTRY_SIZE
    entry_size;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(range);
  SOC_SAND_CHECK_NULL_INPUT(location);
  SOC_SAND_CHECK_NULL_INPUT(success);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit, tcam_db_id, &entry_size);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);
  
  found = FALSE;
  for (bank_id = 0; (!found) && (bank_id < SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit)); ++bank_id)
  {
    res = arad_tcam_db_bit_search_range_get(
      unit,
      tcam_db_id,
      range,
      bank_id,
      &start_bit,
      &end_bit,
      &skip
    );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    if (!skip)
    {
        /* Find closest free line - 2 entries for 80b */
        res = arad_tcam_closest_free_line_find(
                unit,
                tcam_db_id,
                bank_id,
                ARAD_TCAM_DIRECTION_FORWARD,
                start_bit,
                end_bit,
                entry_size,
                is_inserted_top,
                TRUE, /* allow_pair_same_db */
                &place_first_free,
                &location->entry,
                &found
             );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
      location->bank_id = bank_id;

      /* In case of 320b, the next bank is used also for the same entry - skip it */
      if (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) {
          ++bank_id;
      }
    }
  }

  *success = found ? SOC_SAND_SUCCESS : SOC_SAND_FAILURE_OUT_OF_RESOURCES;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_empty_location_in_range_find()", tcam_db_id, 0);
}


#ifdef BROADCOM_DEBUG
/* Check consistency with previous and next priority node */
STATIC
  uint32
    arad_tcam_db_priority_list_entry_verify(
      SOC_SAND_IN  int            unit,
      SOC_SAND_IN  uint32            tcam_db_id,
      SOC_SAND_IN  uint32             priority
    )
{
  uint32
    res = SOC_SAND_OK;
  SOC_SAND_SORTED_LIST_PTR
    prio_list;
  uint8
    prio_node_found,
      found,
      key_buffer_previous[ARAD_TCAM_DB_LIST_KEY_SIZE],
      data_buffer_previous[ARAD_TCAM_DB_LIST_DATA_SIZE],
      key_buffer_next[ARAD_TCAM_DB_LIST_KEY_SIZE],
      data_buffer_next[ARAD_TCAM_DB_LIST_DATA_SIZE],
    key_buffer[ARAD_TCAM_DB_LIST_KEY_SIZE],
    data_buffer[ARAD_TCAM_DB_LIST_DATA_SIZE];
  SOC_SAND_SORTED_LIST_ITER    
      current,
      iter;
  ARAD_TCAM_PRIO_LOCATION 
      prio_location_previous,
      prio_location_next,
      prio_location;
  ARAD_TCAM_LOCATION 
      location_compare_before,
      location_compare_after;


  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db_priorities.get(unit, tcam_db_id, &prio_list);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  /* Find if this priority node already exists */
  res = arad_tcam_db_priority_list_entry_iter_get(
            unit,
            tcam_db_id,
            priority,
            &iter,
            &prio_node_found
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 7, exit);

  if (!prio_node_found) {
      /* The node must exist, since it was not removed */
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Unit %d Tcam DB id %d has no priority %d node although expected.\n\r"),
                  unit, tcam_db_id, priority));
       SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 12, exit);
  }


      /* Get the iteration details and compare the locations */
     res = soc_sand_sorted_list_entry_value(
            unit,
            prio_list,
            iter,
            key_buffer,
            data_buffer
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

    /* Verify it is the same priority */
    if (priority != arad_tcam_db_prio_list_priority_value_decode(key_buffer)) {
         LOG_ERROR(BSL_LS_SOC_TCAM,
                   (BSL_META_U(unit,
                               "Unit %d Tcam DB id %d has priority %d but the matched iter %d has different priority %d.\n\r"),
                    unit, tcam_db_id, priority, iter, arad_tcam_db_prio_list_priority_value_decode(key_buffer)));
         SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 50, exit);
    }

    res = arad_tcam_db_priority_data_decode(unit, tcam_db_id, data_buffer, &prio_location);
    SOC_SAND_CHECK_FUNC_RESULT(res, 37, exit);

    /* Get next range */
    current = iter;
    res = soc_sand_sorted_list_get_next(
            unit,
            prio_list,
            &current,
            key_buffer_next,
            data_buffer_next
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 23, exit);
    if (current != SOC_SAND_SORTED_LIST_ITER_END(unit,prio_list)) {
        res = arad_tcam_db_priority_data_decode(unit, tcam_db_id, data_buffer_next, &prio_location_next);
        SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
        if (arad_tcam_db_prio_list_priority_value_decode(key_buffer_next) <= arad_tcam_db_prio_list_priority_value_decode(key_buffer)) {
             LOG_ERROR(BSL_LS_SOC_TCAM,
                       (BSL_META_U(unit,
                                   "Unit %d Tcam DB id %d priority %d node is before priority %d node.\n\r"),
                                   unit, tcam_db_id, arad_tcam_db_prio_list_priority_value_decode(key_buffer), 
                        arad_tcam_db_prio_list_priority_value_decode(key_buffer_next)));
             SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 44, exit);
        }

        /* Compare the locations */
        res = arad_tcam_db_entry_id_to_location_entry_get(
                unit,
                tcam_db_id,
                prio_location.entry_id_last,
                TRUE,
                &location_compare_before,
                &found
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 39, exit);
        if (!found) {
             LOG_ERROR(BSL_LS_SOC_TCAM,
                       (BSL_META_U(unit,
                                   "Unit %d Tcam DB id %d entry %d is supposed to be found.\n\r"),
                        unit, tcam_db_id, prio_location.entry_id_last));
             SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 41, exit);
        }

        /* Compare the locations */
        res = arad_tcam_db_entry_id_to_location_entry_get(
                unit,
                tcam_db_id,
                prio_location_next.entry_id_first,
                TRUE,
                &location_compare_after,
                &found
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 51, exit);
        if (!found) {
             LOG_ERROR(BSL_LS_SOC_TCAM,
                       (BSL_META_U(unit,
                                   "Unit %d Tcam DB id %d entry %d is supposed to be found.\n\r"),
                        unit, tcam_db_id, prio_location_next.entry_id_first));
             SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 53, exit);
        }

        if ((location_compare_after.bank_id < location_compare_before.bank_id) || 
            ((location_compare_after.bank_id == location_compare_before.bank_id) && (location_compare_after.entry < location_compare_before.entry))) {    
             LOG_ERROR(BSL_LS_SOC_TCAM,
                       (BSL_META_U(unit,
                                   "Unit %d Tcam DB id %d the priority %d node last entry %d in location (%d, %d)"
                                   "was suppposed to be before the priority %d node first entry %d in location (%d, %d).\n\r"),
                        unit, tcam_db_id, arad_tcam_db_prio_list_priority_value_decode(key_buffer), prio_location.entry_id_last, 
                        location_compare_before.bank_id, location_compare_before.entry, arad_tcam_db_prio_list_priority_value_decode(key_buffer_next),
                        prio_location_next.entry_id_first, location_compare_after.bank_id, location_compare_after.entry));
             SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 55, exit);
        }
    }


    /* Get previous range */
    current = iter;
    res = soc_sand_sorted_list_get_prev(
            unit,
            prio_list,
            &current,
            key_buffer_previous,
            data_buffer_previous
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 57, exit);
    if (current != SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,prio_list)) {
        res = arad_tcam_db_priority_data_decode(unit, tcam_db_id, data_buffer_previous, &prio_location_previous);
        SOC_SAND_CHECK_FUNC_RESULT(res, 59, exit);

        if (arad_tcam_db_prio_list_priority_value_decode(key_buffer_previous) >= arad_tcam_db_prio_list_priority_value_decode(key_buffer)) {
             LOG_ERROR(BSL_LS_SOC_TCAM,
                       (BSL_META_U(unit,
                                   "Unit %d Tcam DB id %d priority %d node is after priority %d node.\n\r"),
                                   unit, tcam_db_id, arad_tcam_db_prio_list_priority_value_decode(key_buffer), 
                        arad_tcam_db_prio_list_priority_value_decode(key_buffer_previous)));
             SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 61, exit);
        }
        
        /* Compare the locations */
        res = arad_tcam_db_entry_id_to_location_entry_get(
                unit,
                tcam_db_id,
                prio_location_previous.entry_id_last,
                TRUE,
                &location_compare_before,
                &found
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 63, exit);
        if (!found) {
             LOG_ERROR(BSL_LS_SOC_TCAM,
                       (BSL_META_U(unit,
                                   "Unit %d Tcam DB id %d entry %d is supposed to be found.\n\r"),
                        unit, tcam_db_id, prio_location_previous.entry_id_last));
             SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 65, exit);
        }

        /* Compare the locations */
        res = arad_tcam_db_entry_id_to_location_entry_get(
                unit,
                tcam_db_id,
                prio_location.entry_id_first,
                TRUE,
                &location_compare_after,
                &found
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 67, exit);
        if (!found) {
             LOG_ERROR(BSL_LS_SOC_TCAM,
                       (BSL_META_U(unit,
                                   "Unit %d Tcam DB id %d entry %d is supposed to be found.\n\r"),
                        unit, tcam_db_id, prio_location.entry_id_first));
             SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 69, exit);
        }

        if ((location_compare_after.bank_id < location_compare_before.bank_id) || 
            ((location_compare_after.bank_id == location_compare_before.bank_id) && (location_compare_after.entry < location_compare_before.entry))) {    
             LOG_ERROR(BSL_LS_SOC_TCAM,
                       (BSL_META_U(unit,
                                   "Unit %d Tcam DB id %d the priority %d node last entry %d in location (%d, %d)"
                                   "was suppposed to be before the priority %d node first entry %d in location (%d, %d).\n\r"),
                        unit, tcam_db_id, arad_tcam_db_prio_list_priority_value_decode(key_buffer_previous), prio_location_previous.entry_id_last, 
                        location_compare_before.bank_id, location_compare_before.entry, arad_tcam_db_prio_list_priority_value_decode(key_buffer),
                        prio_location.entry_id_first, location_compare_after.bank_id, location_compare_after.entry));
             SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 71, exit);
        }
    }


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_priority_list_entry_verify()", tcam_db_id, 0);
}
#endif /* BROADCOM_DEBUG */




/* 
 * insert the entry to the priority range: 
 * 1. If new priority, add the node with both first and last entries 
 * equal to this entry 
 * 2. If existing priority, compare the first and last entry locations 
 * with the current location, and replace if necessary. 
 */
STATIC
  uint32
    arad_tcam_db_priority_list_entry_add(
      SOC_SAND_IN  int            unit,
      SOC_SAND_IN  uint32            tcam_db_id,
      SOC_SAND_IN  ARAD_TCAM_LOCATION     *location,
      SOC_SAND_IN  uint32            entry_id,
      SOC_SAND_IN  uint32             priority,
      SOC_SAND_IN  uint8        is_update_prio, /* If True, used when remove-add to update the prio list */
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE *success
    )
{
  uint32
    res = SOC_SAND_OK;
  SOC_SAND_SORTED_LIST_PTR
    prio_list;
  uint8
    prio_node_found,
      found,
    key_buffer[ARAD_TCAM_DB_LIST_KEY_SIZE],
    data_buffer[ARAD_TCAM_DB_LIST_DATA_SIZE];
  uint8
    entry_added;
  SOC_SAND_SORTED_LIST_ITER    
      iter;
  ARAD_TCAM_PRIO_LOCATION 
      prio_location;
  ARAD_TCAM_LOCATION 
      location_compare;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db_priorities.get(unit, tcam_db_id, &prio_list);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  /* Find if this priority node already exists */

  res = arad_tcam_db_priority_list_entry_iter_get(
            unit,
            tcam_db_id,
            priority,
            &iter,
            &prio_node_found
          );

  SOC_SAND_CHECK_FUNC_RESULT(res, 7, exit);

  if (!prio_node_found) {
      if (is_update_prio) {
          /* The node must exist, since it was not removed */
           LOG_ERROR(BSL_LS_SOC_TCAM,
                     (BSL_META_U(unit,
                                 "Unit %d Tcam DB id %d entry %d has priority %d but no node although expected during update.\n\r"),
                      unit, tcam_db_id, entry_id, priority));
           SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 50, exit);
      }

      arad_tcam_db_prio_list_priority_value_encode(priority, key_buffer);
      ARAD_TCAM_PRIO_LOCATION_clear(&prio_location);
      prio_location.entry_id_first = entry_id;
      prio_location.entry_id_last = entry_id;
      arad_tcam_db_prio_list_data_encode(&prio_location, data_buffer);

      res = soc_sand_sorted_list_entry_add(
              unit,
              prio_list,
              key_buffer,
              data_buffer,
              &entry_added
            );

      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
      *success = entry_added? SOC_SAND_SUCCESS : SOC_SAND_FAILURE_OUT_OF_RESOURCES;
  }
  else {

      /* Always success since no new node */
      *success = SOC_SAND_SUCCESS;

      /* Get the iteration details and compare the locations */
     res = soc_sand_sorted_list_entry_value(
            unit,
            prio_list,
            iter,
            key_buffer,
            data_buffer
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

    /* Verify it is the same priority */
    if (priority != arad_tcam_db_prio_list_priority_value_decode(key_buffer)) {
         LOG_ERROR(BSL_LS_SOC_TCAM,
                   (BSL_META_U(unit,
                               "Unit %d Tcam DB id %d entry %d has priority %d but the matched iter %d has different priority %d.\n\r"),
                    unit, tcam_db_id, entry_id, priority, iter, arad_tcam_db_prio_list_priority_value_decode(key_buffer)));
         SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 50, exit);
    }

    res = arad_tcam_db_priority_data_decode(unit, tcam_db_id, data_buffer, &prio_location);
    SOC_SAND_CHECK_FUNC_RESULT(res, 37, exit);

    /* Compare the below location */
    res = arad_tcam_db_entry_id_to_location_entry_get(
            unit,
            tcam_db_id,
            prio_location.entry_id_first,
            TRUE,
            &location_compare,
            &found
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 39, exit);

    /* Verify the entry is found */
    if ((!found) || 
        (((prio_location.entry_id_first == entry_id) || (prio_location.entry_id_last == entry_id)) && (!is_update_prio))) {
         LOG_ERROR(BSL_LS_SOC_TCAM,
                   (BSL_META_U(unit,
                               "Unit %d Tcam DB id %d entry %d is supposed to be found because in priority %d range.\n\r"),
                    unit, tcam_db_id, prio_location.entry_id_first, priority));
         SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 41, exit);
    }

    if (is_update_prio && ((prio_location.entry_id_first == entry_id) || (prio_location.entry_id_last == entry_id))) {
        /* Entry already inside, do nothing */
        ARAD_DO_NOTHING_AND_EXIT;
    }
    else if ((location->bank_id < location_compare.bank_id) || 
        ((location->bank_id == location_compare.bank_id) && (location->entry < location_compare.entry))) {
        /* Lowest entry, replace it */
        prio_location.entry_id_first = entry_id;
        arad_tcam_db_prio_list_data_encode(&prio_location, data_buffer);
        res = soc_sand_sorted_list_entry_update(
                unit,
                prio_list,
                iter,
                key_buffer,
                data_buffer
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 47, exit);
    }
    else {
        /* Compare the upper location */
        res = arad_tcam_db_entry_id_to_location_entry_get(
                unit,
                tcam_db_id,
                prio_location.entry_id_last,
                TRUE,
                &location_compare,
                &found
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 59, exit);
        /* Verify the entry is found */
        if (!found) {
             LOG_ERROR(BSL_LS_SOC_TCAM,
                       (BSL_META_U(unit,
                                   "Unit %d Tcam DB id %d entry %d is supposed to be found because in priority %d range.\n\r"),
                        unit, tcam_db_id, prio_location.entry_id_first, priority));
             SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 41, exit);
        }

        if ((location->bank_id > location_compare.bank_id) || 
            ((location->bank_id == location_compare.bank_id) && (location->entry > location_compare.entry))) {
            /* Lowest entry, replace it */
            prio_location.entry_id_last = entry_id;
            arad_tcam_db_prio_list_data_encode(&prio_location, data_buffer);
            res = soc_sand_sorted_list_entry_update(
                    unit,
                    prio_list,
                    iter,
                    key_buffer,
                    data_buffer
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 61, exit);
        }
    }

  }

#ifdef BROADCOM_DEBUG
  /* Check consistency with previous and next */

  res = arad_tcam_db_priority_list_entry_verify(unit, tcam_db_id, priority);

  SOC_SAND_CHECK_FUNC_RESULT(res, 7, exit);
#endif /* BROADCOM_DEBUG */

exit:

  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_priority_list_entry_add()", tcam_db_id, entry_id);
}

STATIC
  uint32
    arad_tcam_global_location_table_entry_add(
      SOC_SAND_IN  int            unit,
      SOC_SAND_IN  uint32            tcam_db_id,
      SOC_SAND_IN  ARAD_TCAM_LOCATION     *location,
      SOC_SAND_IN  uint32            entry_id,
      SOC_SAND_IN  uint32             priority
    )
{
  uint32
      global_location_id,
    res = SOC_SAND_OK;
  ARAD_TCAM_GLOBAL_LOCATION 
      global_location;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* update the location table with entry id and priority */
  res = arad_tcam_global_location_encode(unit, location, &global_location_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);
  global_location.tcam_db_id = tcam_db_id;
  global_location.entry_id = entry_id;
  global_location.priority = priority;
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.global_location_tbl.set(unit, global_location_id, &global_location);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_global_location_table_entry_add()", tcam_db_id, entry_id);
}

STATIC
  uint32
    arad_tcam_db_priority_list_entry_get(
      SOC_SAND_IN  int        unit,
      SOC_SAND_IN  uint32        tcam_db_id,
      SOC_SAND_IN  ARAD_TCAM_LOCATION *location,
      SOC_SAND_OUT uint32        *entry_id,
      SOC_SAND_OUT uint32         *priority
    )
{
  uint32
      global_location_id,
    res = SOC_SAND_OK;
  ARAD_TCAM_GLOBAL_LOCATION 
      global_location;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* Get the attributes from the location table */
  res = arad_tcam_global_location_encode(unit, location, &global_location_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.global_location_tbl.get(unit, global_location_id, &global_location);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  *priority = global_location.priority;
  *entry_id = global_location.entry_id;

  if (global_location.tcam_db_id != tcam_db_id) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Unit %d Tcam DB id %d in location (bank %d, entry %d) was indicated in global location table to be for DB %d.\n\r"),
                  unit, tcam_db_id, location->bank_id, location->entry, global_location.tcam_db_id));
       SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 50, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_priority_list_entry_get()", tcam_db_id, 0);
}

/* 
 * Remove from the priority list: 
 * - The priority node necessarily exists 
 * - If the node has only this entry in both first and last, 
 * remove the node 
 * - If the node does not have this entry, skip 
 * - If one of the boundary is this enty, then find via occupation bitmap 
 * the closest entry and replace this boundary. 
 */
STATIC
  uint32
    arad_tcam_db_priority_list_entry_remove(
      SOC_SAND_IN int        unit,
      SOC_SAND_IN uint32        tcam_db_id,
      SOC_SAND_IN ARAD_TCAM_LOCATION *location,
      SOC_SAND_IN uint32        entry_id,
      SOC_SAND_IN  uint32       priority,
      SOC_SAND_IN  uint8        is_update_prio /* If True, used when remove-add to update the prio list */
    )
{
  uint32
      entry_id_next,
      next_priority,
    res = SOC_SAND_OK;
  SOC_SAND_SORTED_LIST_PTR
    prio_list;
  SOC_SAND_SORTED_LIST_ITER
    iter;
  uint8
      prio_node_found,
      found,
      key_buffer[ARAD_TCAM_DB_LIST_KEY_SIZE],
      data_buffer[ARAD_TCAM_DB_LIST_DATA_SIZE];
  ARAD_TCAM_PRIO_LOCATION 
      prio_location;
  ARAD_TCAM_LOCATION
      next_location;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

#ifdef BROADCOM_DEBUG
  /* Check consistency with previous and next */
  res = arad_tcam_db_priority_list_entry_verify(unit, tcam_db_id, priority);
  SOC_SAND_CHECK_FUNC_RESULT(res, 7, exit);
#endif /* BROADCOM_DEBUG */

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db_priorities.get(unit, tcam_db_id, &prio_list);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  /* Find if this priority node already exists */
  res = arad_tcam_db_priority_list_entry_iter_get(
            unit,
            tcam_db_id,
            priority,
            &iter,
            &prio_node_found
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 7, exit);
  if (!prio_node_found) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Unit %d Tcam DB id %d entry %d has priority %d but no such node found in priority list.\n\r"),
                  unit, tcam_db_id, entry_id, priority));
       SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 9, exit);
  }

     /* Get the node attributes */
     res = soc_sand_sorted_list_entry_value(
            unit,
            prio_list,
            iter,
            key_buffer,
            data_buffer
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

    /* Verify it is the same priority */
    if (priority != arad_tcam_db_prio_list_priority_value_decode(key_buffer)) {
         LOG_ERROR(BSL_LS_SOC_TCAM,
                   (BSL_META_U(unit,
                               "Unit %d Tcam DB id %d entry %d has priority %d but the matched iter %d has different priority %d.\n\r"),
                    unit, tcam_db_id, entry_id, priority, iter, arad_tcam_db_prio_list_priority_value_decode(key_buffer)));
         SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 50, exit);
    }

    res = arad_tcam_db_priority_data_decode(unit, tcam_db_id, data_buffer, &prio_location);
    SOC_SAND_CHECK_FUNC_RESULT(res, 37, exit);

    /* Different cases */
    if ((prio_location.entry_id_first != entry_id) && (prio_location.entry_id_last != entry_id)) {
        /* Entry in the middle of the range, no action */
        ARAD_DO_NOTHING_AND_EXIT;
    }
    else if ((prio_location.entry_id_first == entry_id) && (prio_location.entry_id_last == entry_id)) {
        /* Single entry with this priority, remove the node if not for update */
        if (is_update_prio) {
            /* For update, do nothing */
            ARAD_DO_NOTHING_AND_EXIT;
        }
        else {
            res = soc_sand_sorted_list_entry_remove_by_iter(unit, prio_list, iter);
            SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
        }
    }
    else if ((prio_location.entry_id_first == entry_id) && (prio_location.entry_id_last != entry_id)) {
        /* Find the closest entry and update the node */
        res = arad_tcam_db_occ_bmp_entry_closest_get(
                  unit,
                  tcam_db_id,
                  ARAD_TCAM_DIRECTION_FORWARD,
                  location,
                  &entry_id_next,
                  &next_priority,
                  &next_location,
                  &found
                );
        SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);
        if ((!found) || (next_priority != priority)) {
             LOG_ERROR(BSL_LS_SOC_TCAM,
                       (BSL_META_U(unit,
                                   "Unit %d Tcam DB id %d entry %d has priority %d but the next entry %d in range has priority %d and is found %d.\n\r"),
                        unit, tcam_db_id, entry_id, priority, entry_id_next, next_priority, found));
             SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 47, exit);
        }
        prio_location.entry_id_first = entry_id_next;
        arad_tcam_db_prio_list_data_encode(&prio_location, data_buffer);
        res = soc_sand_sorted_list_entry_update(
                unit,
                prio_list,
                iter,
                key_buffer,
                data_buffer
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 49, exit);
    }
    else if ((prio_location.entry_id_first != entry_id) && (prio_location.entry_id_last == entry_id)) {
        /* Find the closest entry and update the node */
        res = arad_tcam_db_occ_bmp_entry_closest_get(
                  unit,
                  tcam_db_id,
                  ARAD_TCAM_DIRECTION_BACKWARD,
                  location,
                  &entry_id_next,
                  &next_priority,
                  &next_location,
                  &found
                );
        SOC_SAND_CHECK_FUNC_RESULT(res, 55, exit);
        if ((!found) || (next_priority != priority)) {
             LOG_ERROR(BSL_LS_SOC_TCAM,
                       (BSL_META_U(unit,
                                   "Unit %d Tcam DB id %d entry %d has priority %d but the next entry %d in range has priority %d and is found %d.\n\r"),
                        unit, tcam_db_id, entry_id, priority, entry_id_next, next_priority, found));
             SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 47, exit);
        }
        prio_location.entry_id_last = entry_id_next;
        arad_tcam_db_prio_list_data_encode(&prio_location, data_buffer);
        res = soc_sand_sorted_list_entry_update(
                unit,
                prio_list,
                iter,
                key_buffer,
                data_buffer
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 59, exit);
    }
    else {
        SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 100, exit);
    }


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_priority_list_entry_remove()", 0, 0);
}

STATIC
  uint32
    arad_tcam_global_location_table_entry_remove(
      SOC_SAND_IN  int            unit,
      SOC_SAND_IN  uint32            tcam_db_id,
      SOC_SAND_IN  ARAD_TCAM_LOCATION     *location,
      SOC_SAND_OUT uint32           *priority
    )
{
  uint32
      global_location_id,
    res = SOC_SAND_OK;
  ARAD_TCAM_GLOBAL_LOCATION 
      global_location;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* update the location table with entry id and priority */
  res = arad_tcam_global_location_encode(unit, location, &global_location_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.global_location_tbl.get(unit, global_location_id, &global_location);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  *priority = global_location.priority;
  if (global_location.tcam_db_id != tcam_db_id) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Unit %d Tcam DB id %d in location (bank %d, entry %d) was indicated in global location table to be for DB %d.\n\r"),
                  unit, tcam_db_id, location->bank_id, location->entry, global_location.tcam_db_id));
       SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 50, exit);
  }

  ARAD_TCAM_GLOBAL_LOCATION_clear(&global_location);
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.global_location_tbl.set(unit, global_location_id, &global_location);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_global_location_table_entry_remove()", tcam_db_id, 0);
}

/* Set the entry to be occupied or not per DB and bank */
STATIC
  uint32
    arad_tcam_db_bank_occ_bitmap_entry_get(
      SOC_SAND_IN int        unit,
      SOC_SAND_IN uint32        tcam_db_id,
      SOC_SAND_IN ARAD_TCAM_LOCATION *location,
      SOC_SAND_OUT uint8        *is_occupied
    )
{
  uint32
    res = SOC_SAND_OK;
  SOC_SAND_OCC_BM_PTR
    bank_bm;
  uint8
    bit_set;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* Get the relevant occupation bitmap */
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db_entries_used.get(unit, tcam_db_id, location->bank_id, &bank_bm);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  res = soc_sand_occ_bm_is_occupied(unit, bank_bm, location->entry, &bit_set);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  /* Occupation bitmap is inverse of natural: a zero bit means there is an entry */
  *is_occupied = bit_set? FALSE: TRUE;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_bank_occ_bitmap_entry_get()", 0, 0);
}


/* Set the entry to be occupied or not per DB and bank */
STATIC
  uint32
    arad_tcam_db_bank_occ_bitmap_entry_set(
      SOC_SAND_IN int        unit,
      SOC_SAND_IN uint32        tcam_db_id,
      SOC_SAND_IN ARAD_TCAM_LOCATION *location,
      SOC_SAND_IN uint8        is_occupied
    )
{
  uint32
    res = SOC_SAND_OK;
  SOC_SAND_OCC_BM_PTR
    bank_bm;
  uint8
    bit_set;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* Check consistency */
#ifdef BROADCOM_DEBUG
  {
      uint8
        is_occupied_before = 0;

      /* For debug, verify the status is changed - otherwise it is suspicious */
      res = arad_tcam_db_bank_occ_bitmap_entry_get(unit, tcam_db_id,  location, &is_occupied_before);
      SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);
      if (is_occupied_before == is_occupied)
      {
           LOG_ERROR(BSL_LS_SOC_TCAM,
                     (BSL_META_U(unit,
                                 "Unit %d Tcam DB id %d in location (bank %d, entry %d) was occupied %d but tries to set the same occupation %d.\n\r"),
                      unit, tcam_db_id, location->bank_id, location->entry, is_occupied_before, is_occupied));
           SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 50, exit);
      }
  }
#endif /* BROADCOM_DEBUG */


  /* Get the relevant occupation bitmap */
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db_entries_used.get(unit, tcam_db_id, location->bank_id, &bank_bm);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  /* Occupation bitmap is inverse of natural: a zero bit means there is an entry */
  bit_set = is_occupied? FALSE: TRUE;
  res = soc_sand_occ_bm_occup_status_set(unit, bank_bm, location->entry, bit_set);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_bank_occ_bitmap_entry_set()", 0, 0);
}



  uint32
    arad_tcam_db_entry_id_to_location_entry_add(
      SOC_SAND_IN int        unit,
      SOC_SAND_IN uint32        tcam_db_id,
      SOC_SAND_IN uint32        entry_id,
      SOC_SAND_IN ARAD_TCAM_LOCATION *location
    )
{
  uint32
    res = SOC_SAND_OK,
    data_indx;
  SOC_SAND_HASH_TABLE_PTR
    hash_tbl;
  uint8
    key_buffer[ARAD_TCAM_DB_HASH_TBL_KEY_SIZE];
  uint8
    success;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.entry_id_to_location.get(unit, &hash_tbl);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);

  arad_tcam_db_entry_id_encode(tcam_db_id, entry_id, key_buffer);

  res = soc_sand_hash_table_entry_add(
          unit,
          hash_tbl,
          key_buffer,
          &data_indx,
          &success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (!success)
  {
    LOG_ERROR(BSL_LS_SOC_TCAM,
              (BSL_META_U(unit,
                          "Unit %d, Tcam db id %d Entry id %d, Failed to insert the entry to the hash table.\n\r"),
               unit, tcam_db_id, entry_id));
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_MALLOC_FAIL, 20, exit);
  }

  /* Update the location table */
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.db_location_tbl.set(unit, data_indx, location);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_id_to_location_entry_add()", 0, 0);
}

STATIC
  uint32
    arad_tcam_db_entry_id_to_location_entry_index_get(
      SOC_SAND_IN  int unit,
      SOC_SAND_IN  uint32 tcam_db_id,
      SOC_SAND_IN  uint32 entry_id,
      SOC_SAND_OUT uint32  *index,
      SOC_SAND_OUT uint8 *found
    )
{
  uint32
    res = SOC_SAND_OK;
  SOC_SAND_HASH_TABLE_PTR
    entry_id_to_location;
  uint8
    key_buffer[ARAD_TCAM_DB_HASH_TBL_KEY_SIZE];

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.entry_id_to_location.get(unit, &entry_id_to_location);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);

  arad_tcam_db_entry_id_encode(tcam_db_id, entry_id, key_buffer);

  res = soc_sand_hash_table_entry_lookup(
          unit,
          entry_id_to_location,
          key_buffer,
          index,
          found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_id_to_location_entry_index_get()", 0, 0);
}


uint32
  arad_tcam_db_entry_id_to_location_entry_get(
    SOC_SAND_IN  int        unit,
    SOC_SAND_IN  uint32        tcam_db_id,
    SOC_SAND_IN  uint32        entry_id,
    SOC_SAND_IN  uint32        isLocation,
    SOC_SAND_OUT ARAD_TCAM_LOCATION *location,
    SOC_SAND_OUT uint8        *found
  )
{
  uint32
    res = SOC_SAND_OK,
    data_indx;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_tcam_db_entry_id_to_location_entry_index_get(
          unit,
          tcam_db_id,
          entry_id,
          &data_indx,
          found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (*found)
  {
      /* Retrieve the location value from location table */
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.db_location_tbl.get(unit, data_indx, location);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);

      /* Check consistency */
#ifdef BROADCOM_DEBUG
      if (isLocation)
      {
          uint8
            is_occupied = 0;

          /* For debug, verify the status is changed - otherwise it is suspicious */
          res = arad_tcam_db_bank_occ_bitmap_entry_get(unit, tcam_db_id, location, &is_occupied);
          SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);
          if (is_occupied == FALSE)
          {
               LOG_ERROR(BSL_LS_SOC_TCAM,
                         (BSL_META_U(unit,
                                     "Unit %d Tcam DB id %d in location (bank %d, entry %d) has an entry but indicated not to be occupied %d .\n\r"),
                          unit, tcam_db_id, location->bank_id, location->entry, is_occupied));
               SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 50, exit);
          }
      }
#endif /* BROADCOM_DEBUG */
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_id_to_location_entry_get()", 0, 0);
}

  uint32
    arad_tcam_db_entry_id_to_location_entry_remove(
      SOC_SAND_IN int unit,
      SOC_SAND_IN uint32 tcam_db_id,
      SOC_SAND_IN uint32 entry_id
    )
{
  uint32
    res = SOC_SAND_OK,
    data_indx;
  SOC_SAND_HASH_TABLE_PTR
    entry_id_to_location;
  uint8
    found;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_tcam_db_entry_id_to_location_entry_index_get(
          unit,
          tcam_db_id,
          entry_id,
          &data_indx,
          &found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (found)
  {
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.entry_id_to_location.get(unit, &entry_id_to_location);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);
    res = soc_sand_hash_table_entry_remove_by_index(
            unit,
            entry_id_to_location,
            data_indx
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_id_to_location_entry_remove()", 0, 0);
}

STATIC
  uint32
    arad_tcam_db_entry_prefix_stamp(
      SOC_SAND_IN    int     unit,
      SOC_SAND_IN    uint32     tcam_db_id,
      SOC_SAND_INOUT ARAD_TCAM_ENTRY *entry
    )
{
  uint32
    res = SOC_SAND_OK,
    mask;
  ARAD_TCAM_PREFIX
    prefix;
  ARAD_TCAM_BANK_ENTRY_SIZE
    entry_size;
  uint32
    entry_nof_bits;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.prefix.get(
    unit,
    tcam_db_id,
    &prefix
  );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 2, exit);

  if (prefix.length > 0)
  {
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(
            unit,
            tcam_db_id,
            &entry_size
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);
    switch (entry_size)
    {
    case ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS:
      entry_nof_bits = 80;
      break;

    case ARAD_TCAM_BANK_ENTRY_SIZE_160_BITS:
      entry_nof_bits = 160;
      break;

    case ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS:
      entry_nof_bits = 320;
      break;

    default:
      /*
       *  Don't get here
       */
      entry_nof_bits = (uint32) -1;
      break;
    }

    res = soc_sand_bitstream_set_any_field(
            &prefix.bits,
            entry_nof_bits - prefix.length,
            prefix.length,
            entry->value
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    mask = SOC_SAND_BITS_MASK(prefix.length - 1, 0);
    res = soc_sand_bitstream_set_any_field(
            &mask,
            entry_nof_bits - prefix.length,
            prefix.length,
            entry->mask
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_prefix_stamp()", tcam_db_id, 0);
}

/* Compute the distance in lines between 2 locations */
STATIC
  uint32
    arad_tcam_location_distance_get(
      SOC_SAND_IN  int                      unit,
      SOC_SAND_IN  uint32                   tcam_db_id,
      SOC_SAND_IN  ARAD_TCAM_LOCATION       *low,
      SOC_SAND_IN  ARAD_TCAM_LOCATION       *high,
      SOC_SAND_OUT int32                    *distance
    )
{
  uint32
      bank_ndx,
      res;
  ARAD_TCAM_BANK_ENTRY_SIZE
    entry_size;
  uint8
    is_used;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(
          unit,
          tcam_db_id,
          &entry_size
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

    if (low->bank_id == high->bank_id)
    {
      *distance = high->entry - low->entry;
    }
    else
    {
      /* 
       * Loop on the intermediate bank sizes 
       * Count only the banks where the Database is present 
       */
      *distance = arad_tcam_bank_entry_size_to_entry_count_get(unit, entry_size, low->bank_id) - low->entry;
      for (bank_ndx = (low->bank_id + 1); bank_ndx < high->bank_id; bank_ndx++) {
          res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(unit, tcam_db_id, bank_ndx, &is_used);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
          if (is_used) {
            *distance += arad_tcam_bank_entry_size_to_entry_count_get(unit, entry_size, bank_ndx);
          }
      }
      *distance    += high->entry;
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_location_distance_get()", tcam_db_id, 0);
}

/* 
 * According to the direction, find the closest 
 * free location to a specific location 
 * Compute the distance to it (number of lines) 
 */
STATIC
  uint32
    arad_tcam_closest_free_location_find(
      SOC_SAND_IN  int         unit,
      SOC_SAND_IN  uint32         tcam_db_id,
      SOC_SAND_IN  uint8          is_inserted_top,
      SOC_SAND_IN  ARAD_TCAM_LOCATION  *location,
      SOC_SAND_IN  uint8          location_not_found, /* If True, then the location is not of an entry */
      SOC_SAND_IN  ARAD_TCAM_DIRECTION direction,
      SOC_SAND_OUT ARAD_TCAM_LOCATION  *free_location_first, /* First free location */
      SOC_SAND_OUT ARAD_TCAM_LOCATION  *free_location, /* Chosen location */
      SOC_SAND_OUT int32         *distance,
      SOC_SAND_OUT uint8         *success
    )
{
  uint32
    res = SOC_SAND_OK,
    start,
    end,
      bank_id,
      place_first_free,
    place;
  int32
    current_distance = 0;
  uint8
    skip,
    found;
  ARAD_TCAM_BANK_ENTRY_SIZE
    entry_size;
  ARAD_TCAM_RANGE
    search_range;
  ARAD_TCAM_LOCATION
    current;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  ARAD_TCAM_RANGE_clear(&search_range);
  ARAD_TCAM_LOCATION_clear(&current);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(
          unit,
          tcam_db_id,
          &entry_size
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);

  /*
   *  Search for a free entry below the range
   */
  switch (direction)
  {
  case ARAD_TCAM_DIRECTION_BACKWARD:
    search_range.min.bank_id = 0;
    search_range.min.entry   = 0;
    search_range.max.bank_id = location->bank_id;
    search_range.max.entry   = location->entry;
    break;

  case ARAD_TCAM_DIRECTION_FORWARD:
    search_range.min.bank_id = location->bank_id;
    search_range.min.entry   = location->entry;
    search_range.max.bank_id = SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit) - 1;
    search_range.max.entry   = arad_tcam_bank_entry_size_to_entry_count_get(unit, entry_size, search_range.max.bank_id) - 1;
    break;

  default:
    break;
  }

  *success = FALSE;
  /* 
   * In case the location was not found (i.e. the location 
   * is an extremity of the TCAM Bank space and no 
   * lower / upper entry in the Database is limiting it), no need 
   * to look for an empty closest location, since any empty location 
   * would be acceptable to insert the entry
   */
  if (location_not_found) {
      *success = FALSE;
      ARAD_DO_NOTHING_AND_EXIT;
  }


  for (bank_id = 0; bank_id < SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit); ++bank_id)
  {
      /* Do not consider odd bank-ids for 320b - not legal */
      if ((entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) && (bank_id % 2)) {
          continue;
      }

    res = arad_tcam_db_bit_search_range_get(
      unit,
      tcam_db_id,
      &search_range,
      bank_id,
      &start,
      &end,
      &skip
    );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    if (!skip)
    {
        /* Find closest free line - 2 entries for 80b */
        res = arad_tcam_closest_free_line_find(
                unit,
                tcam_db_id,
                bank_id,
                direction,
                start,
                end,
                entry_size,
                is_inserted_top,
                FALSE, /* allow_pair_same_db */
                &place_first_free,
                &place,
                &found
             );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

      if (found)
      {
        current.bank_id  = bank_id;
        current.entry    = place;
        res = arad_tcam_location_distance_get(
                unit,
                tcam_db_id,
                (direction == ARAD_TCAM_DIRECTION_FORWARD)? location : &current, /* low */
                (direction == ARAD_TCAM_DIRECTION_FORWARD)? &current : location /* high */,
                &current_distance
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
#if ARAD_TCAM_INSERTION_ALGORITHM_MIDDLE_FREE
        /* In case of middle method, try to move in the same banks to more lines */
        if ((current.bank_id == location->bank_id) && (!is_inserted_top)) {
            /* Make the distance negative if same bank */
            current_distance = (-current_distance);
        }
#endif /* ARAD_TCAM_INSERTION_ALGORITHM_MIDDLE_FREE */

        if (!*success || (current_distance < *distance))
        {
          *free_location = current;
          *free_location_first = current;
          free_location_first->entry = place_first_free;
          *distance      = current_distance;
          *success       = TRUE;
        }
      }
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_closest_free_location_find()", tcam_db_id, direction);
}

/* 
 * Find the closest free location to the range: 
 * - Consider backward, get a 1st location 
 * - Consider forward for a 2nd location 
 * - Compare the distance of both and chose the closest 
 */
STATIC
  uint32
    arad_tcam_closest_free_location_to_range_find(
      SOC_SAND_IN  int            unit,
      SOC_SAND_IN  uint32            tcam_db_id,
      SOC_SAND_IN  uint8             is_inserted_top,
      SOC_SAND_IN  ARAD_TCAM_RANGE        *range,
      SOC_SAND_OUT ARAD_TCAM_LOCATION     *free_location_first, /* First free location */
      SOC_SAND_OUT ARAD_TCAM_LOCATION     *free_location,  /* Chosen location */
      SOC_SAND_OUT ARAD_TCAM_DIRECTION    *direction,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE *success
    )
{
  uint32
    res = SOC_SAND_OK;
  uint8
    found_below,
    found_above;
  int32
    distance_below,
    distance_above;
  ARAD_TCAM_LOCATION
      below_first,
      above_first,
    below,
    above;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_tcam_closest_free_location_find(
          unit,
          tcam_db_id,
          is_inserted_top,
          &range->min,
          range->min_not_found,
          ARAD_TCAM_DIRECTION_BACKWARD,
          &below_first,
          &below,
          &distance_below,
          &found_below
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_tcam_closest_free_location_find(
          unit,
          tcam_db_id,
          is_inserted_top,
          &range->max,
          range->max_not_found,
          ARAD_TCAM_DIRECTION_FORWARD,
          &above_first,
          &above,
          &distance_above,
          &found_above
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if (!found_below && !found_above)
  {
    *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
  }
  else
  {
    *success = SOC_SAND_SUCCESS;
    if (found_below)
    {
      *free_location = below;
      *free_location_first = below_first;
      *direction     = ARAD_TCAM_DIRECTION_BACKWARD;
      if (found_above && (distance_above < distance_below))
      {
        *free_location = above;
        *free_location_first = above_first;
        *direction     = ARAD_TCAM_DIRECTION_FORWARD;
      }
    }
    else
    {
      *free_location = above;
      *free_location_first = above_first;
      *direction     = ARAD_TCAM_DIRECTION_FORWARD;
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_closest_free_location_to_range_find()", tcam_db_id, 0);
}

void
    arad_tcam_db_location_decode_print(
      SOC_SAND_IN  int                unit,
      SOC_SAND_IN uint8  *buffer
    )
{
  ARAD_TCAM_LOCATION
    lhs;

  arad_tcam_db_location_decode(unit, buffer, &lhs);

  ARAD_TCAM_LOCATION_print(&lhs);
}

STATIC
  uint32
    arad_tcam_db_entry_move(
      SOC_SAND_IN int        unit,
      SOC_SAND_IN uint8         set_hw_also,
      SOC_SAND_IN uint8         is_bank_share_update,
      SOC_SAND_IN ARAD_TCAM_LOCATION *source,
      SOC_SAND_IN ARAD_TCAM_LOCATION *destination
    )
{
  uint32
      global_location_id_source,
      global_location_id_dest,
      bank_nof_entries,
      tcam_db_id = 0,
      priority,
    res = SOC_SAND_OK,
    data_indx,
    entry_id;
  SOC_SAND_HASH_TABLE_PTR
    hash_tbl;
  uint8
    hash_key[ARAD_TCAM_DB_HASH_TBL_KEY_SIZE];
  uint8
      hit_bit,
    found;
  ARAD_TCAM_ENTRY
    entry;
  ARAD_TCAM_ACTION
    action;
  ARAD_TCAM_ACTION_SIZE 
    action_bitmap_ndx;
  ARAD_TCAM_GLOBAL_LOCATION 
      global_location,
      global_location_clear;
  SOC_SAND_SUCCESS_FAILURE 
      success;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /*
   *  Get the entry-id from the global location table
   */
  /* update the location table with entry id and priority */
  res = arad_tcam_global_location_encode(unit, source, &global_location_id_source);
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.global_location_tbl.get(unit, global_location_id_source, &global_location);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit);
  tcam_db_id = global_location.tcam_db_id;
  entry_id = global_location.entry_id;
  priority = global_location.priority;

  /* Update the global location table */
  res = arad_tcam_global_location_encode(unit, destination, &global_location_id_dest);
  SOC_SAND_CHECK_FUNC_RESULT(res, 7, exit);
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.global_location_tbl.set(unit, global_location_id_dest, &global_location);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);
  ARAD_TCAM_GLOBAL_LOCATION_clear(&global_location_clear);
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.global_location_tbl.set(unit, global_location_id_source, &global_location_clear);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 9, exit);

  /*
   *  Update the entry_id -> location hash table
   */
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.entry_id_to_location.get(unit, &hash_tbl);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);
  arad_tcam_db_entry_id_encode(tcam_db_id, entry_id, hash_key);
  res = soc_sand_hash_table_entry_lookup(
          unit,
          hash_tbl,
          hash_key,
          &data_indx,
          &found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
  if (!found) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Unit %d Tcam DB id %d entry %d is expected to be in the hash table but is not existing.\n\r"), 
                  unit, tcam_db_id, entry_id));
       SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_DOESNT_EXIST_ERR, 7, exit);
  }

  /* Update the location table */
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.db_location_tbl.set(unit, data_indx, destination);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.action_bitmap_ndx.get(
          unit,
          tcam_db_id,
          &action_bitmap_ndx
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

  /*
   *  Move the entry in the hardware - only to get HW info
   */
  if (set_hw_also) {
      res = arad_tcam_bank_entry_get_unsafe(
              unit,
              source->bank_id,
              source->entry,
              action_bitmap_ndx,
              TRUE /* move the hit_bit also */,
              &entry,
              &action,
              &hit_bit
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 75, exit); /* was in comment */

      /* Update the count of entries per bank */
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_nof_entries.get(unit, tcam_db_id, source->bank_id, &bank_nof_entries);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 76, exit);
      bank_nof_entries--;
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_nof_entries.set(unit, tcam_db_id, source->bank_id, bank_nof_entries);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 77, exit);

      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_nof_entries.get(unit, tcam_db_id, destination->bank_id, &bank_nof_entries);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 78, exit);
      bank_nof_entries++;
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_nof_entries.set(unit, tcam_db_id, destination->bank_id, bank_nof_entries);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 79, exit);
  }

  /* 
   * First add the new entry
   */
  res = arad_tcam_bank_entry_set_unsafe(
          unit,
          set_hw_also,
          is_bank_share_update,
          hit_bit,
          destination->bank_id,
          destination->entry,
          action_bitmap_ndx,
          &entry,
          &action
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

  res = arad_tcam_db_bank_occ_bitmap_entry_set(
          unit,
          global_location.tcam_db_id,
          destination,
          TRUE /* is_occupied */
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 85, exit);

  /* 
   * Then remove the previous one to prevent traffic 
   * influence on-the-fly 
   */
  res = arad_tcam_bank_entry_invalidate_unsafe(
          unit,
          set_hw_also,
          is_bank_share_update,
          source->bank_id,
          source->entry
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

  res = arad_tcam_db_bank_occ_bitmap_entry_set(
          unit,
          global_location.tcam_db_id,
          source,
          FALSE /* is_occupied */
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 85, exit);

  /* 
   * Update the priority list if not SW DB update after HW move 
   * Update at the end because it may be dependent on the SW state 
   * It is possible that the new location is chosen (or missed) as 
   * a new boundary. If missed, it will be fixed during the addition. 
   */
  if (!is_bank_share_update) {
      /* 
       * Simplify: remove the node and add it with the new location. 
       * A more complex procedure could skip the removal-addition, but the 
       * performance should not be major 
       */
      res = arad_tcam_db_priority_list_entry_remove(
              unit,
              tcam_db_id,
              source,
              entry_id,
              priority,
              TRUE /* is_update_prio */
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 8, exit);

      res = arad_tcam_db_priority_list_entry_add(
              unit,
              tcam_db_id,
              destination,
              entry_id,
              priority,
              TRUE, /* is_update_prio */
              &success
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);
      if (success != SOC_SAND_SUCCESS) {
           LOG_ERROR(BSL_LS_SOC_TCAM,
                     (BSL_META_U(unit,
                                 "Unit %d Tcam DB id %d entry %d is expected to be added to the prio list but has failed.\n\r"), 
                      unit, tcam_db_id, entry_id));
           SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_DOESNT_EXIST_ERR, 11, exit);
      }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_move()", tcam_db_id, 0);
}



/* Update the different SW DB after an HW move */
STATIC
  uint32
    arad_tcam_db_sw_db_after_move_update(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  ARAD_TCAM_DIRECTION      direction,
      SOC_SAND_IN  uint32                   bank_id,
      SOC_SAND_IN  uint32                   nof_entries_shift,
      SOC_SAND_IN  uint32                   entry_from_min,
      SOC_SAND_IN  uint32                   entry_from_max
    )
{
  uint32
      line_ndx,
    res = SOC_SAND_OK;
  ARAD_TCAM_LOCATION
    range_start,
    range_end;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  ARAD_TCAM_LOCATION_clear(&range_start);
  ARAD_TCAM_LOCATION_clear(&range_end);

  /* 
   * Go over all the entries in this range, and 
   * move them one by one 
   */

  for (line_ndx = 0; line_ndx < (entry_from_max - entry_from_min + 1); line_ndx++)
  {
      range_start.bank_id = bank_id;
      range_start.entry = (direction == ARAD_TCAM_DIRECTION_BACKWARD)? (entry_from_min + line_ndx): (entry_from_max - line_ndx);
      range_end.bank_id = bank_id;
      range_end.entry = (direction == ARAD_TCAM_DIRECTION_BACKWARD)? (range_start.entry - nof_entries_shift): (range_start.entry + nof_entries_shift);

    /*
     *  Move entries only once you get the previous entry
     *  not to confuse the order in SW DB
     */
    res = arad_tcam_db_entry_move(
            unit,
            FALSE, /* set_hw_also */
            TRUE, /* is_bank_share_update */
            &range_start,
            &range_end
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
  }
              
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_sw_db_after_move_update()", 0, 0);
}

/* Update cache after hw move block  */
STATIC uint32
arad_pp_ihb_tcam_action_tbl_cache_after_move_update(
      SOC_SAND_IN   int                              unit,
      SOC_SAND_IN   uint32                           bank_ndx,
      SOC_SAND_IN   uint32                           tcam_db_id,
      SOC_SAND_IN   uint32                           move_couple,
      SOC_SAND_IN   uint32                           nof_entries_in_line,
      SOC_SAND_IN   uint32                           source_entry_start_ndx,
      SOC_SAND_IN   uint32                           source_entry_end_ndx,
      SOC_SAND_IN   uint32                           dest_entry_start_ndx
)
    {
    uint32
    range,
    src_arr_index,
    dest_arr_index,
    action_bit,
    res;
    soc_mem_t mem_tbl;

    ARAD_TCAM_ACTION_SIZE action_tbl_ndx;
    ARAD_TCAM_ACTION_SIZE action_bitmap_ndx;
    uint32
       src_line[ARAD_TCAM_NOF_ACTION_SIZES]         = {0,0,0,0},
       src_action_tbl[ARAD_TCAM_NOF_ACTION_SIZES]   = {0,0,0,0},
       dest_line[ARAD_TCAM_NOF_ACTION_SIZES]        = {0,0,0,0},
       dest_action_tbl[ARAD_TCAM_NOF_ACTION_SIZES]  = {0,0,0,0};

    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_IHB_TCAM_TBL_MOVE_UNSAFE);

    LOG_VERBOSE(BSL_LS_SOC_MEM,
           (BSL_META_U(unit,
                       SOC_DPP_MSG("      "
                       "TCAM-CACHE-Block-move: bank:%d, entry-source-start:%d, entry-source-end:%d, "
                       "entry-dest-start:%d, is-320b: %d, nof_entries_in_line: %d\n\r")),
            bank_ndx, source_entry_start_ndx, source_entry_end_ndx,
            dest_entry_start_ndx, move_couple, nof_entries_in_line));

    /*
    * in ARAD the only use of the TACM for 164 bits word (one word per line) 82 bits words (2 words per line)
    */
    if (nof_entries_in_line == 0 || nof_entries_in_line > 2)
    {
        SOC_SAND_SET_ERROR_CODE(ARAD_TBL_RANGE_OUT_OF_LIMIT_ERR, 10, exit);
    }
    /*
    * in case there is two words per line which word to use.
    */
    if (source_entry_start_ndx >= nof_entries_in_line * ARAD_TCAM_NOF_LINES_PER_BANK(unit, bank_ndx))
    {
        SOC_SAND_SET_ERROR_CODE(ARAD_TBL_RANGE_OUT_OF_LIMIT_ERR, 20, exit);
    }
    if (dest_entry_start_ndx >= nof_entries_in_line * ARAD_TCAM_NOF_LINES_PER_BANK(unit, bank_ndx))
    {
        SOC_SAND_SET_ERROR_CODE(ARAD_TBL_RANGE_OUT_OF_LIMIT_ERR, 20, exit);
    }

    if (nof_entries_in_line == 1)
        range = source_entry_end_ndx - source_entry_start_ndx + 1;
    else
        range = (source_entry_end_ndx - source_entry_start_ndx + 2) / 2;

    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.action_bitmap_ndx.get(
            unit,
            tcam_db_id,
            &action_bitmap_ndx
          );

    res = arad_tcam_tbl_calc_action_banks_and_lines(unit, bank_ndx, source_entry_start_ndx, &action_bitmap_ndx, src_line, src_action_tbl);
        SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);
    res = arad_tcam_tbl_calc_action_banks_and_lines(unit, bank_ndx, dest_entry_start_ndx, &action_bitmap_ndx, dest_line, dest_action_tbl);
        SOC_SAND_CHECK_FUNC_RESULT(res, 6, exit);

        for (action_tbl_ndx = 0; action_tbl_ndx < ARAD_TCAM_NOF_ACTION_SIZES; action_tbl_ndx++)
        {
            action_bit = (1 << action_tbl_ndx);

            if (action_bitmap_ndx & action_bit)
            {
                res = arad_pp_ihb_tcam_action_mem_and_index_get(unit, src_action_tbl[action_tbl_ndx], FALSE, &mem_tbl, &src_arr_index);
                SOC_SAND_CHECK_FUNC_RESULT(res, 55, exit);
                res = arad_pp_ihb_tcam_action_mem_and_index_get(unit, dest_action_tbl[action_tbl_ndx], FALSE, &mem_tbl, &dest_arr_index);
                SOC_SAND_CHECK_FUNC_RESULT(res, 56, exit);

                res = soc_mem_cache_block_move(unit, 0x0, mem_tbl, src_arr_index, dest_arr_index, MEM_BLOCK_ALL, src_line[action_tbl_ndx], dest_line[action_tbl_ndx], range);
                SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 57, exit);
            }
        }
    

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_cache_after_move_update()", bank_ndx, source_entry_start_ndx);
}

/* 
 * For any bank except the last, 
 * import the last entry of the DB to the 1st line (free) 
 * of this database 
 */
STATIC
  uint32
    arad_tcam_db_last_entry_previous_bank_move(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  uint32                   tcam_db_id,
      SOC_SAND_IN  ARAD_TCAM_RANGE          *range,
      SOC_SAND_IN  ARAD_TCAM_LOCATION       *empty_location,
      SOC_SAND_IN  ARAD_TCAM_DIRECTION      direction,
      SOC_SAND_IN  uint32                   nof_entries_per_line,
      SOC_SAND_IN  uint32                   nof_entries_max_in_bank,
      SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE  entry_size,
      SOC_SAND_INOUT ARAD_TCAM_LOCATION     *final_location, /* SAND_IN in fact */
      SOC_SAND_INOUT  ARAD_TCAM_LOCATION    *empty
    )
{
  uint32
      res = SOC_SAND_OK,
      entry_id,
    next_priority;
  ARAD_TCAM_LOCATION
    next_location,
      empty_lcl;
  uint8
      found;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  ARAD_TCAM_LOCATION_clear(&next_location);
  ARAD_TCAM_LOCATION_clear(&empty_lcl);

  /* Get the closest entry */
  res = arad_tcam_db_occ_bmp_entry_closest_get(
            unit,
            tcam_db_id,
            (direction == ARAD_TCAM_DIRECTION_FORWARD)? ARAD_TCAM_DIRECTION_BACKWARD: ARAD_TCAM_DIRECTION_FORWARD,
            empty_location,
            &entry_id,
            &next_priority,
            &next_location,
            &found
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if (found == FALSE)
  {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "     "
                             "Error in previous bank - no entry found,"
                             "for DB %d, bank %d and line %d and direction %d.\n\r"), 
                  tcam_db_id, empty_location->bank_id, empty_location->entry, direction));
     SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DATABASE_ALREADY_EXISTS_ERR, 10, exit);

  }

  /* Move the entry - the whole line if 80b */
  if ((entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS) 
       && (empty->entry != ARAD_TCAM_ENTRY_LINE_PAIR_CLOSEST_ID(empty->entry, direction))){
      /* Move to the good half in the empty line */
      empty->entry = ARAD_TCAM_ENTRY_LINE_PAIR_80B(empty->entry);
  }
  res = arad_tcam_db_entry_move(
          unit,
          TRUE, /* set_hw_also */
          FALSE, /* is_bank_share_update */
          &next_location,
          empty
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
  

  if ((entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS)
      && (next_location.entry == ARAD_TCAM_ENTRY_LINE_PAIR_CLOSEST_ID(next_location.entry, direction))
      /* Verify not last bank and in the middle of the range */
      && (!((final_location->bank_id == next_location.bank_id) && (next_location.entry == final_location->entry)))){
      /* 
       * since for 80b entries we move the whole line 
       * to the other bank, we move the last line of previous bank 
       * with the 2 entries instead of the closest entry 
       *  
       * We are guaranteed the whole empty line is free, 
       * otherwise it would choose another location 
       */
      next_location.entry = ARAD_TCAM_ENTRY_LINE_PAIR_80B(next_location.entry);
      empty->entry = ARAD_TCAM_ENTRY_LINE_PAIR_80B(empty->entry);

      /* Move the entry - the whole line if 80b */
      res = arad_tcam_db_entry_move(
              unit,
              TRUE, /* set_hw_also */
              FALSE, /* is_bank_share_update */
              &next_location,
              empty
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
  }
  *empty = next_location;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_last_entry_previous_bank_move()", tcam_db_id, 0);
}

/* 
 * Move the entries from the needed range bound 
 * to the free location to make some room for the entry 
 * to insert. 2 methods: 
 * NEW one: per bank, find the free location and move with 
 * TCAM Manager the needed number of lines. Update then the 
 * SW DB for each entry (one by one).
 * OLD one: go over the entries, and move them one by one 
 * until you arrive to the range. Needed when one of the action 
 * table is for Direct Lookup DBs, since the TCAM Manager moves 
 * the TCAM Action table values also 
 */
STATIC
  uint32
    arad_tcam_db_entries_migrate(
      SOC_SAND_IN  int         unit,
      SOC_SAND_IN  uint32         tcam_db_id,
      SOC_SAND_IN  uint8          is_inserted_top,
      SOC_SAND_IN  ARAD_TCAM_RANGE        *range,
      SOC_SAND_IN  ARAD_TCAM_LOCATION  *empty_location_first, /* First free location */
      SOC_SAND_IN  ARAD_TCAM_LOCATION  *empty_location, /* Chosen location */
      SOC_SAND_IN  uint32          target_priority,
      SOC_SAND_IN  ARAD_TCAM_DIRECTION direction,
      SOC_SAND_OUT ARAD_TCAM_LOCATION  *free_location
    )
{
  uint32
      nof_entries_max_in_bank,
      nof_entries_per_line,
      nof_entries_shift = 0,
      entry_dest_min,
      entry_first,
      line_free_first,
      entry_from_min,
      entry_from_max,
      next_entry,
      entry_stop,
      bank_id,
      bank_ndx,
      nof_banks,
    res = SOC_SAND_OK;
  SOC_SAND_SORTED_LIST_PTR
    prio_list;
  SOC_SAND_SORTED_LIST_ITER
    next;
  uint8
      move_per_block = FALSE,
      move_block_not_needed,
      has_not_finished,
      key_buffer[ARAD_TCAM_DB_LIST_KEY_SIZE],
      data_buffer[ARAD_TCAM_DB_LIST_DATA_SIZE],
      found,
      has_direct_table,
      insertion_rule_respected;
  uint32
    next_priority;
  ARAD_TCAM_LOCATION
    current_location,
    next_location,
    empty,
    empty_current,
    range_start;
  ARAD_TCAM_BANK_ENTRY_SIZE
    entry_size;
  ARAD_TCAM_PRIO_LOCATION 
      prio_location;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  ARAD_TCAM_LOCATION_clear(&current_location);
  ARAD_TCAM_LOCATION_clear(&next_location);
  ARAD_TCAM_LOCATION_clear(&empty);
  ARAD_TCAM_LOCATION_clear(&range_start);
  ARAD_TCAM_LOCATION_clear(free_location);

  empty = *empty_location;

  /* 
   * Sequence: 
   * - Compute the number of banks to run over, i.e. the distance 
   * between the free location and the extremity of the range 
   * (last entry to move) 
   * - For each bank, starting from the one of the free location: 
   *    - Move the needed entries up to the free location in the HW
   *    in one operation
   *    - Update the SW DB (link list, occupation bitmap)
   *    - If needed, move the last entry of this DB of the
   *    previous bank to the 1st line of this bank (conversely
   *    for backward)
   *  
   * Special case for 80b: 
   * - Skip the 1st line => entry 0 & 1 
   * - Do not move to an half free line - both entries in the line must be 
   * free to be considered. Works per line. 
   */
  
  ARAD_TCAM_LOCATION_clear(free_location);
  free_location->bank_id = (direction == ARAD_TCAM_DIRECTION_BACKWARD)?
      range->min.bank_id: range->max.bank_id;
  free_location->entry = (direction == ARAD_TCAM_DIRECTION_BACKWARD)?
      range->min.entry: range->max.entry;
  nof_banks = (direction == ARAD_TCAM_DIRECTION_BACKWARD)?
      (range->min.bank_id - empty_location->bank_id + 1) :(empty_location->bank_id - range->max.bank_id + 1);
  for (bank_ndx = 0; bank_ndx < nof_banks; bank_ndx++) {
      bank_id = empty.bank_id; 
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.entry_size.get(unit, bank_id, &entry_size);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);
      nof_entries_per_line = (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS)? 2: 1;
      entry_first = (bank_ndx == 0)? empty_location_first->entry : empty.entry;
      line_free_first = ARAD_TCAM_ENTRY_LINE_GET(entry_first, entry_size);
      if (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS) {
          line_free_first = ARAD_TCAM_ENTRY_LINE_GET(((direction == ARAD_TCAM_DIRECTION_BACKWARD)? (entry_first - 1) : (entry_first + 1)), entry_size);
      }
      move_block_not_needed = FALSE;
      nof_entries_max_in_bank = arad_tcam_bank_entry_size_to_entry_count_get(unit, entry_size, bank_id);
      if ((line_free_first * nof_entries_per_line) == ((direction == ARAD_TCAM_DIRECTION_BACKWARD)? (nof_entries_max_in_bank-1): 0)) {
          move_block_not_needed = TRUE;
      }
      entry_first = (direction == ARAD_TCAM_DIRECTION_BACKWARD)? (entry_first + 1): (entry_first - 1);

      /* Check if the location bank has Direct Table as well.
       * In that case the block move cannot be allowed, use the old
       * method instead. 
       * The choice of the method should be per bank and not global 
       */ 

      /* Use the old method for IP TCAM and 80b and Direct table because more efficient */
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.has_direct_table.get(unit, bank_id, &has_direct_table);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
      if(has_direct_table) {
        move_per_block = TRUE;
      } else {
        res = arad_tcam_db_use_new_method_per_db_get(unit, tcam_db_id, &move_per_block);
        SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);
      }

      /* Always perform the following with the old method */
      if ((!move_block_not_needed) || (!move_per_block)) {
          /* 
           * Do not block-move entries if the range is illegal: 
           * for 80b entries with half entry to move
           */
          if (bank_id == free_location->bank_id) {
              entry_from_min = (direction == ARAD_TCAM_DIRECTION_BACKWARD)? entry_first: range->max.entry;
              entry_from_max = (direction == ARAD_TCAM_DIRECTION_BACKWARD)? range->min.entry: entry_first;
              entry_stop = ((direction == ARAD_TCAM_DIRECTION_BACKWARD)? entry_from_max: entry_from_min);
              if (entry_stop == empty.entry) {
                  LOG_DEBUG(BSL_LS_SOC_TCAM,
                            (BSL_META_U(unit,
                                        "Unit %d Tcam DB id %d - no need to move entries since empty location fits for the entry to insert.\n\r"),
                             unit, tcam_db_id));
                  ARAD_DO_NOTHING_AND_EXIT;
              }
              if ((entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS) && ((entry_from_min % 2) == 1)) {
                  entry_from_min = (direction == ARAD_TCAM_DIRECTION_BACKWARD)? (entry_from_min - 1): (entry_from_min + 1);
              }
              if ((entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS) && ((entry_from_max % 2) == 0))  {
                  entry_from_max = (direction == ARAD_TCAM_DIRECTION_BACKWARD)? (entry_from_max - 1): (entry_from_max + 1);
              }
              if (entry_from_min > entry_from_max)
              {
                move_block_not_needed = TRUE;
              }

          }
          else {
              /* Not last bank */
              entry_from_min = (direction == ARAD_TCAM_DIRECTION_BACKWARD)? entry_first: 0;
              entry_from_max = (direction == ARAD_TCAM_DIRECTION_BACKWARD)? (nof_entries_max_in_bank - 1): entry_first;
          }

        nof_entries_shift = 0;
        if ((!move_block_not_needed) || (!move_per_block))
        {
            /* Move in the HW */
            nof_entries_shift = nof_entries_per_line * 
                ARAD_TCAM_ENTRY_NOF_LINES_GET(empty.entry, 
                  ((direction == ARAD_TCAM_DIRECTION_BACKWARD)? entry_from_min : entry_from_max), entry_size);
            entry_dest_min = (direction == ARAD_TCAM_DIRECTION_BACKWARD)?
                    (entry_from_min - nof_entries_shift): (entry_from_min + nof_entries_shift);


          /* The free location will be always the same */

          /* 
           * OLD method: 
           * 1. Retrieve the closest entry to the empty location 
           * 2. As long as the priority is not acceptable, move 
           * the furthest entry to the empty location and look at the next 
           * priority group 
           * 3. Once the priority group is found, insert the entry there
           */
          if (!move_per_block) {
              /* Set the current Bank-ID */

              /* Property of the bank - identical for any of these banks */
              res = arad_tcam_db_occ_bmp_entry_closest_get(
                        unit,
                        tcam_db_id,
                        (direction == ARAD_TCAM_DIRECTION_FORWARD)? ARAD_TCAM_DIRECTION_BACKWARD: ARAD_TCAM_DIRECTION_FORWARD,
                        empty_location,
                        &next_entry,
                        &next_priority,
                        &next_location,
                        &found
                      );
              SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

              if (found) {
                  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db_priorities.get(unit, tcam_db_id, &prio_list);
                  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
                  res = arad_tcam_db_priority_list_entry_iter_get(
                            unit,
                            tcam_db_id,
                            next_priority,
                            &next,
                            &found
                          );
                  SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit);
                  if (!found) {
                       LOG_ERROR(BSL_LS_SOC_TCAM,
                                 (BSL_META_U(unit,
                                             "Unit %d Tcam DB id %d entry %d exists in occupation bitmap but not in priority %d table.\n\r"), 
                                  unit, tcam_db_id, next_entry, next_priority));
                       SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_DOESNT_EXIST_ERR, 7, exit);
                  }

                  /* 
                   * For DBs non-respecting BCM rules (IP, VTT, FP with flag), 
                   * - loop on the priority nodes 
                   * - for each node, move the extreme entry to the empty location 
                   * - update the empty location 
                   *  
                   * For DBs respecting BCM rules (FP without flag), 
                   * - loop in the occupation bitmap 
                   * - for each node, move the extreme entry to the empty location 
                   * - update the empty location 
                   */
                  res = arad_tcam_db_block_entry_insertion_rule_respected_get(unit, tcam_db_id, &insertion_rule_respected);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
                  if (!insertion_rule_respected) {
                      has_not_finished = (direction == ARAD_TCAM_DIRECTION_BACKWARD)? 
                                            (next_priority < target_priority): (next_priority > target_priority);
                  } else {
                      has_not_finished = (direction == ARAD_TCAM_DIRECTION_BACKWARD)?
                          (next_priority <= target_priority): (next_priority > target_priority);
                  }

                  while (has_not_finished)
                  {
                    /*
                     *  Find the location of the entry to move
                     */
                      res = arad_tcam_db_block_entry_insertion_rule_respected_get(unit, tcam_db_id, &insertion_rule_respected);
                      SOC_SAND_CHECK_FUNC_RESULT(res, 75, exit);
                      if (!insertion_rule_respected) {
                          /* Get the iter attributes */
                          res = soc_sand_sorted_list_entry_value(
                                 unit,
                                 prio_list,
                                 next,
                                 key_buffer,
                                 data_buffer
                               );
                         SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
                         /* Consistency check */
                         if (next_priority != arad_tcam_db_prio_list_priority_value_decode(key_buffer)) {
                              LOG_ERROR(BSL_LS_SOC_TCAM,
                                        (BSL_META_U(unit,
                                                    "Unit %d Tcam DB id %d has priority %d but the matched iter %d has different priority %d.\n\r"),
                                         unit, tcam_db_id, next_priority, next, arad_tcam_db_prio_list_priority_value_decode(key_buffer)));
                              SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 50, exit);
                         }

                         /* Retrieve the further member */
                         res = arad_tcam_db_priority_data_decode(unit, tcam_db_id, data_buffer, &prio_location);
                         SOC_SAND_CHECK_FUNC_RESULT(res, 37, exit);
                         res = arad_tcam_db_entry_id_to_location_entry_get(
                                 unit,
                                 tcam_db_id,
                                 ((direction == ARAD_TCAM_DIRECTION_BACKWARD)? prio_location.entry_id_last: prio_location.entry_id_first),
                                 TRUE,
                                 &range_start,
                                 &found
                               );
                         SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);
                         /* Check consistency: the entry must exist */
                         if (!found) {
                              LOG_ERROR(BSL_LS_SOC_TCAM,
                                        (BSL_META_U(unit,
                                                    "Unit %d Tcam DB id %d entry not existing.\n\r"), 
                                         unit, tcam_db_id));
                              SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_DOESNT_EXIST_ERR, 57, exit);
                         }

                         /* Update the next iteration */
                         if (direction == ARAD_TCAM_DIRECTION_BACKWARD)
                         {
                           res = soc_sand_sorted_list_get_next(
                                   unit,
                                   prio_list,
                                   &next,
                                   key_buffer,
                                   data_buffer
                                 );
                           SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
                         }
                         else
                         {
                           res = soc_sand_sorted_list_get_prev(
                                   unit,
                                   prio_list,
                                   &next,
                                   key_buffer,
                                   data_buffer
                                 );
                           SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
                         }
                         next_priority = arad_tcam_db_prio_list_priority_value_decode(key_buffer);
                         has_not_finished = (direction == ARAD_TCAM_DIRECTION_BACKWARD) ?
                                    ((next != SOC_SAND_SORTED_LIST_ITER_END(unit,prio_list)) && (next_priority < target_priority))
                                    : ((next != SOC_SAND_SORTED_LIST_ITER_BEGIN(unit,prio_list)) && (next_priority > target_priority));
                      }
                      else {
                          /* BCM rule respected */
                          range_start = next_location;

                          /* Update the next iteration */
                          res = arad_tcam_db_occ_bmp_entry_closest_get(
                                    unit,
                                    tcam_db_id,
                                    (direction == ARAD_TCAM_DIRECTION_FORWARD)? ARAD_TCAM_DIRECTION_BACKWARD: ARAD_TCAM_DIRECTION_FORWARD,
                                    &range_start,
                                    &next_entry,
                                    &next_priority,
                                    &next_location,
                                    &found
                                  );
                          SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
                          has_not_finished = found && ((direction == ARAD_TCAM_DIRECTION_BACKWARD)?
                                                       (next_priority <= target_priority): (next_priority > target_priority));
                      }

                      /* Move the entry */
                      res = arad_tcam_db_entry_move(
                            unit,
                            TRUE, /* set_hw_also */
                            FALSE, /* is_bank_share_update */
                            &range_start,
                            &empty
                          );
                    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

                    empty = range_start;
                  } /* while ( priority groups ) */
                  *free_location = empty;
              }
              else {
                  /* No entry further, no need to migrate - suspicious */
                  *free_location = *empty_location;
              }

              /* If moving was done for one line only, break here */
              break;
          }
          else {
              /* NEW method - move blocks */
              res = arad_pp_ihb_tcam_tbl_move_unsafe(
                    unit,
                    bank_id,
                    (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS), /* move_couple */
                    nof_entries_per_line, 
                    entry_from_min,
                    entry_from_max, 
                    entry_dest_min
              );
              SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);

              /*
               * Update Cache
               */
              res = arad_pp_ihb_tcam_action_tbl_cache_after_move_update(
                      unit,
                      bank_id,
                      tcam_db_id,
                      (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS), /* move_couple */
                      nof_entries_per_line,
                      entry_from_min,
                      entry_from_max,
                      entry_dest_min
                    );
              SOC_SAND_CHECK_FUNC_RESULT(res, 123, exit);

              /* 
               * Go over the different Databases of this bank and update the 
               * SW DB 
               */
              res = arad_tcam_db_sw_db_after_move_update(
                        unit,
                        direction,
                        bank_id,
                        nof_entries_shift,
                        entry_from_min,
                        entry_from_max
                    );
              SOC_SAND_CHECK_FUNC_RESULT(res, 122, exit);

              /* Update the free location */
              empty.entry = (direction == ARAD_TCAM_DIRECTION_BACKWARD)? 
                  (entry_from_max + 1 - nof_entries_shift): (entry_from_min + nof_entries_shift - 1);
          } /* if (move_per_block) - NEW method */
        } /* if ((!move_block_not_needed) || (!move_per_block)) */
      } /* if ((!move_block_not_needed) || (!move_per_block))  */

      /* 
       * If not last bank, move the closest entry of the next bank 
       * to consider to this empty location
       */
      if (move_per_block) {
          if (bank_id != free_location->bank_id) {
              /* Not last bank */
              res = arad_tcam_db_last_entry_previous_bank_move(
                        unit,
                        tcam_db_id,
                        range,
                        &empty,
                        direction,
                        nof_entries_per_line,
                        nof_entries_max_in_bank,
                        entry_size,
                        free_location,
                        &empty
                      );
              SOC_SAND_CHECK_FUNC_RESULT(res, 124, exit);
          } /*  if (bank_ndx != nof_banks - 1) - last line move */
          else {
              /* 
               * Special case in 80b: if the requested place is in the middle of the line,
               * move the last entry to next line
               */
              entry_stop = (direction == ARAD_TCAM_DIRECTION_BACKWARD)? range->min.entry: range->max.entry;
              empty_current = empty;
              empty.entry = entry_stop;

              if ((entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS)
                   && (entry_stop == ARAD_TCAM_ENTRY_LINE_PAIR_CLOSEST_ID(entry_stop, direction))) {
                  /* 
                   * Move this entry only line further 
                   * No risk of start/end of bank, otherwise it exits previously 
                   */
                  res = arad_tcam_db_entry_move(
                          unit,
                          TRUE, /* set_hw_also */
                          FALSE, /* is_bank_share_update */
                          &empty,
                          &empty_current
                        );
                  SOC_SAND_CHECK_FUNC_RESULT(res, 130, exit);
                  empty_current.entry = (direction == ARAD_TCAM_DIRECTION_BACKWARD)? (empty_current.entry + 1): (empty_current.entry - 1);
              }

              /* Update the free location */
              free_location->entry = empty.entry;
        #if ARAD_TCAM_INSERTION_ALGORITHM_MIDDLE_FREE
              if (!is_inserted_top) {
                  free_location->entry = ARAD_TCAM_INSERTION_ALGORITHM_MIDDLE_FREE_FIND(0, direction, 0, empty.entry, empty_current.entry);
              }
        #endif /* ARAD_TCAM_INSERTION_ALGORITHM_MIDDLE_FREE */

              /* All the needed banks have been scanned */
              break;
          }
      } /* if (move_per_block) */
  } /* for (bank_ndx = 0; bank_ndx < nof_banks; bank_ndx++) */

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entries_migrate()", tcam_db_id, target_priority);
}

STATIC
 uint32
    arad_tcam_db_free_location_allocate(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  uint32                   tcam_db_id,
      SOC_SAND_IN  uint8                    is_inserted_top,
      SOC_SAND_IN  uint32                   priority,
      SOC_SAND_IN  uint8                    is_for_update,
      SOC_SAND_IN  uint32                   entry_id,
      SOC_SAND_OUT uint8                    *is_update_and_invalid,
      SOC_SAND_OUT ARAD_TCAM_LOCATION       *location,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE *success
    )
{
  uint32
    access_profile_id,
    res = SOC_SAND_OK;
  ARAD_TCAM_RANGE
    range;
  ARAD_TCAM_LOCATION
      free_location_first,
    free_location;
  ARAD_TCAM_DIRECTION
    direction=0;
  uint8
      hit_bit,
      found;
  ARAD_TCAM_ACTION_SIZE     
      action_bitmap_ndx = 0;
  ARAD_TCAM_ENTRY     
      entry;
  ARAD_TCAM_ACTION 
      action;
  ARAD_TCAM_BANK_OWNER
      db_bank_owner;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  ARAD_TCAM_RANGE_clear(&range);
  ARAD_TCAM_LOCATION_clear(location);
  ARAD_TCAM_LOCATION_clear(&free_location);
  ARAD_TCAM_LOCATION_clear(&free_location_first);

  *is_update_and_invalid = FALSE;
  /* Special case: if for update and entry not valid, use its location */
  if (is_for_update)
  {
      res = arad_tcam_db_entry_id_to_location_entry_get(
              unit,
              tcam_db_id,
              entry_id,
              TRUE,
              location,
              &found
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

      /* Check consistency: for update, the entry must exist */
      if (!found) {
           LOG_ERROR(BSL_LS_SOC_TCAM,
                     (BSL_META_U(unit,
                                 "Unit %d Tcam DB id %d entry %d is expected to be updaed but is not existing.\n\r"), 
                      unit, tcam_db_id, entry_id));
           SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_DOESNT_EXIST_ERR, 7, exit);
      }

      /* The entry may not be valid only for ACLs - no API for this for other databases */
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.access_profile_id.get(unit, tcam_db_id, 0, &access_profile_id);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit);
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam_mgmt.profiles.bank_owner.get(
              unit,
              access_profile_id,
              &db_bank_owner
            );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit);

      if (found 
          && ((db_bank_owner == ARAD_TCAM_BANK_OWNER_PMF_0) 
              || (db_bank_owner == ARAD_TCAM_BANK_OWNER_PMF_1) 
              || (db_bank_owner == ARAD_TCAM_BANK_OWNER_EGRESS_ACL)))
      {
        /*
         * Get the TCAM key and action
         */
        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.action_bitmap_ndx.get(
                  unit,
                  tcam_db_id,
                  &action_bitmap_ndx
                );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit);

        res = arad_tcam_bank_entry_get_unsafe(
                unit,
                location->bank_id,
                location->entry,
                action_bitmap_ndx,
                FALSE /* hit_bit_clear */,
                &entry,
                &action,
                &hit_bit
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 7, exit);
        if (!entry.valid)
        {
            /* Entry found and not valid, return this location */
            *success = SOC_SAND_SUCCESS;
            *is_update_and_invalid = TRUE;
            ARAD_DO_NOTHING_AND_EXIT;
        }
      }
  }

    /*
    *  Find the appropriate range to insert the entry
    *  according to the existing entries of this Database
    */

    res = arad_tcam_db_priority_list_insertion_range_find(
              unit,
              tcam_db_id,
              priority,
              is_for_update,
              entry_id,
              &range
            );

    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /*
    *  Search for an empty entry in the proper range
    */

    res = arad_tcam_db_empty_location_in_range_find(
              unit,
              tcam_db_id,
              is_inserted_top,
              &range,
              location,
              success
            );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if (*success != SOC_SAND_SUCCESS)
  {
    /*
     *  Move entries to make room for the new one:
     *  - Find the closest free spot to the range
     *  - Shift the lines from ranges to this spot
     */
    res = arad_tcam_closest_free_location_to_range_find(
            unit,
            tcam_db_id,
            is_inserted_top,
            &range,
            &free_location_first,
            &free_location,
            &direction,
            success
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    if (*success == SOC_SAND_SUCCESS)
    {
      res = arad_tcam_db_entries_migrate(
              unit,
              tcam_db_id,
              is_inserted_top,
              &range,
              &free_location_first,
              &free_location,
              priority,
              direction,
              location
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_free_location_allocate()", tcam_db_id, priority);
}

STATIC
  uint32
    arad_tcam_db_entry_exists_get(
      SOC_SAND_IN  int unit,
      SOC_SAND_IN  uint32 tcam_db_id,
      SOC_SAND_IN  uint32 entry_id,
      SOC_SAND_OUT uint8 *found
    )
{
  uint32
    res = SOC_SAND_OK,
    data_indx;
  SOC_SAND_HASH_TABLE_PTR
    hash_tbl;
  uint8
    key_buffer[ARAD_TCAM_DB_HASH_TBL_KEY_SIZE];

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.entry_id_to_location.get(unit, &hash_tbl);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 5, exit);
  arad_tcam_db_entry_id_encode(tcam_db_id, entry_id, key_buffer);
  res = soc_sand_hash_table_entry_lookup(
          unit,
          hash_tbl,
          key_buffer,
          &data_indx,
          found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_exists_get()", tcam_db_id, entry_id);
}

/*
 *  API functions
 */

/* The bank owners in the hw can be pmf, flp, vtt, egress;
 * however, the driver maintains more logical owners (e.g.
 * pmf-0/pmf-1, flp-tcam/traps, vt/tt) since it might cause
 * incorrect behavior to allow both to access the same bank
 * (due to their timing). Also, there is no sw database that
 * stores the bank owner per bank, however there is one for
 * access profile db (per tcam db).
 */
uint32
  arad_tcam_bank_owner_get_unsafe(
    SOC_SAND_IN    int               unit,
    SOC_SAND_IN    uint32               bank_id,
    SOC_SAND_OUT   ARAD_TCAM_BANK_OWNER *bank_owner
  )
{
  uint32
      res = SOC_SAND_OK,
      tcam_db_ndx,
      access_profile_id,
      access_array_ndx;
  uint8
      is_used = FALSE;
  ARAD_TCAM_BANK_ENTRY_SIZE
      entry_size;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* init to invalid value, which will be
   * returned in case not found */
  *bank_owner = ARAD_TCAM_NOF_BANK_OWNERS;

  /* for each tcam database, if it is using this bank - get its owner */
  for (tcam_db_ndx = 0; tcam_db_ndx < ARAD_TCAM_MAX_NOF_LISTS; tcam_db_ndx++)
  {
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(unit, tcam_db_ndx, bank_id, &is_used);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

      if(is_used)
      {
          /* tcam db entry size is required in order to get the access profile array index */
          res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit, tcam_db_ndx, &entry_size);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

          /* if entry size is 320, then array index is according to if the bank is even or odd */
          access_array_ndx = (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) ? (bank_id % 2) : 0;

          res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.access_profile_id.get(unit, tcam_db_ndx, access_array_ndx, &access_profile_id);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

          res = sw_state_access[unit].dpp.soc.arad.tm.tcam_mgmt.profiles.bank_owner.get(unit, access_profile_id, bank_owner);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
      }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_bank_owner_get_unsafe()", bank_id, 0);
}

uint32
    arad_tcam_bank_owner_set_unsafe(
      SOC_SAND_IN    int               unit,
      SOC_SAND_IN    uint32               bank_id,
      SOC_SAND_IN   ARAD_TCAM_BANK_OWNER  bank_owner
    )
{
  uint32
    res,
    field_val,
    start_bit;
  ARAD_TCAM_HW_BANK_OWNER
    hw_bank_owner;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* 
   * Verify
   */
  res = arad_tcam_bank_owner_verify(
          unit,
          bank_id,
          bank_owner
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* Bank owner in the HW can be PMF, FLP, VTT or ERPP. In software,
   * however, the management is somewhat different, and reflects the 
   * constraints to the access to TCAM. 
   * For example in HW there is one owner to PMF, however, it is wrong 
   * for two lookups (even in different programs) to access the TCAM 
   * bank at the same time. This is why the PMF bank owner is now 
   * separated into two owners: PMF-0 and PMF-1, for the two stages. 
   * Same is for FLP-TCAM (uses key C of hte program) and FLP-TRAPS 
   * (uses key D of the FLP program); and also for VT and TT.
   */

  switch (bank_owner) 
  {
  /* PMF cysle 0 and 1 are mapped to FP */
  case ARAD_TCAM_BANK_OWNER_PMF_0:
  case ARAD_TCAM_BANK_OWNER_PMF_1:
      hw_bank_owner = ARAD_TCAM_HW_BANK_OWNER_PMF;
      break;
  /* FLP TCAM and TRAPs (key C and key D) are mapped to FLP */
  case ARAD_TCAM_BANK_OWNER_FLP_TCAM:
  case ARAD_TCAM_BANK_OWNER_FLP_TRAPS:
      hw_bank_owner = ARAD_TCAM_HW_BANK_OWNER_FLP;
      break;
  /* VT and TT are mapped to VTT */
  case ARAD_TCAM_BANK_OWNER_VT:
  case ARAD_TCAM_BANK_OWNER_TT:
      hw_bank_owner = ARAD_TCAM_HW_BANK_OWNER_VTT;
      break;
  /* Egress ACL mapped to ERPP */
  case ARAD_TCAM_BANK_OWNER_EGRESS_ACL:
      hw_bank_owner = ARAD_TCAM_HW_BANK_OWNER_ERPP;
      break;
  default:
      /* If none of the above then exit with error */
      SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_MGMT_BANK_OWNER_OUT_OF_RANGE_ERR, 10, exit);

  }


  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  15,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, PPDB_A_TCAM_BANK_BLOCK_OWNERr, REG_PORT_ANY, 0, TCAM_BANK_BLOCK_OWNERf, &field_val));

  start_bit = bank_id * ARAD_TCAM_NOF_BANK_OWNERS_IN_BITS;
  res = soc_sand_bitstream_set_any_field(&hw_bank_owner, start_bit, ARAD_TCAM_NOF_BANK_OWNERS_IN_BITS, &field_val);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PPDB_A_TCAM_BANK_BLOCK_OWNERr, REG_PORT_ANY, 0, TCAM_BANK_BLOCK_OWNERf,  field_val));

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_bank_owner_set_unsafe()", bank_id, bank_owner);
}

uint32
    arad_tcam_bank_owner_verify(
      SOC_SAND_IN    int               unit,
      SOC_SAND_IN    uint32               bank_id,
      SOC_SAND_OUT   ARAD_TCAM_BANK_OWNER bank_owner
    )
{  
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_ERR_IF_ABOVE_MAX(bank_owner, ARAD_TCAM_BANK_OWNER_MAX, ARAD_TCAM_MGMT_BANK_OWNER_OUT_OF_RANGE_ERR, 40, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_bank_owner_verify()", bank_id, 0);
}


/*********************************************************************
 *     This function initializes a TCAM bank.
 *     Details: in the H file. (search for prototype)
 *
 * This function should not be called in TM mode.
 * This is since it resets the given TCAM bank by calling arad_pp_ihb_tcam_tbl_flush_unsafe().
 * This will causes TCAM to use more power in Arad+ when TCAM is not used.
 * When TCAM is not used, it should not be reset.
*********************************************************************/
uint32
  arad_tcam_bank_init_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               bank_id,
    SOC_SAND_IN  uint8                is_direct,
    SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size,
    SOC_SAND_IN  ARAD_TCAM_BANK_OWNER bank_owner
  )
{
  uint32
    res = SOC_SAND_OK;
  uint8
    valid,
    create_bitmap = FALSE;
  ARAD_TCAM_BANK_ENTRY_SIZE
    old_entry_size;
  uint32
    nof_entries;
  SOC_SAND_OCC_BM_PTR
    occ_bm[2];
  SOC_SAND_OCC_BM_INIT_INFO
    occ_bm_init_info;
  uint8
      is_inverse;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_BANK_INIT_UNSAFE);

  /*
   *  Verify
   */
  res = arad_tcam_bank_init_verify(
          unit,
          bank_id,
          is_direct,
          entry_size,
          bank_owner
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  /*
   *  Flush bank
   */
  res = arad_pp_ihb_tcam_tbl_flush_unsafe(
          unit,
          bank_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* 
   *  Set bank owner
   */
  res = arad_tcam_bank_owner_set_unsafe(
          unit,
          bank_id,
          bank_owner
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);


  /*
   *  Check whether this bank's been initialized before
   */
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.valid.get(
          unit,
          bank_id,
          &valid
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);
  if (!valid)
  {
    create_bitmap = TRUE;
  }
  else
  {
    /*
     *  Check if we can reuse the old occupation bitmap
     */
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.entry_size.get(
            unit,
            bank_id,
            &old_entry_size
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 17, exit);
    for (is_inverse = 0; is_inverse < ARAD_TCAM_NOF_OCCUPATIONS_BITMAPS; is_inverse++) {
        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_bank_entries_used.get(
                           unit,
                           bank_id,
                           is_inverse,
                           &occ_bm[is_inverse]
                         );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

        if (!is_direct
            && entry_size == old_entry_size)
        {
          /*
           *  Reset existing occupation bitmap
           */
          soc_sand_occ_bm_clear(unit, occ_bm[is_inverse]);
        }
        else
        {
          soc_sand_occ_bm_destroy(unit, occ_bm[is_inverse]);
          create_bitmap = TRUE;
        }
    }
  }
  
  /*
   *  Calculate the number of entries in this bank
   */
  nof_entries = arad_tcam_bank_entry_size_to_entry_count_get(
                    unit,
                    is_direct ? ARAD_TCAM_BANK_ENTRY_SIZE_160_BITS : entry_size, 
                    bank_id);
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.nof_entries_free.set(unit, bank_id, nof_entries);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 23, exit);

  if (create_bitmap)
  {
    /*
     *  Create a new occupation bitmap
     */
    for (is_inverse = 0; is_inverse < ARAD_TCAM_NOF_OCCUPATIONS_BITMAPS; is_inverse++) {
        soc_sand_SAND_OCC_BM_INIT_INFO_clear(&occ_bm_init_info);
        occ_bm_init_info.size     = nof_entries;
        occ_bm_init_info.init_val = is_inverse? TRUE:FALSE;

        res = soc_sand_occ_bm_create(
                unit,
                &occ_bm_init_info,
                &occ_bm[is_inverse]
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_bank_entries_used.set(unit, bank_id, is_inverse, occ_bm[is_inverse]);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);
    }

    /*
     *  We reset the entry size whenever we create a new bitmap
     */
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.entry_size.set(unit, bank_id, entry_size);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 35, exit);
  }

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.valid.set(
    unit,
    bank_id,
    TRUE
  );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_bank_init_unsafe()", bank_id, 0);
}

uint32
  arad_tcam_bank_init_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               bank_id,
    SOC_SAND_IN  uint8                is_direct,
    SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size,
    SOC_SAND_IN  ARAD_TCAM_BANK_OWNER bank_owner
  )
{

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_BANK_INIT_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_NOF(bank_id, SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit), ARAD_TCAM_BANK_ID_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(entry_size, ARAD_TCAM_ENTRY_SIZE_MAX+(is_direct?1:0), ARAD_TCAM_ENTRY_SIZE_OUT_OF_RANGE_ERR, 20, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(bank_owner, ARAD_TCAM_BANK_OWNER_MAX, ARAD_TCAM_MGMT_BANK_OWNER_OUT_OF_RANGE_ERR, 40, exit);

  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_bank_init_verify()", 0, 0);
}

/*********************************************************************
*     Creates a new TCAM database.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_tcam_db_create_unsafe(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                    tcam_db_id,
    SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size,
    SOC_SAND_IN  uint32                    prefix_size,
    SOC_SAND_IN  ARAD_TCAM_ACTION_SIZE     action_bitmap_ndx,
    SOC_SAND_IN  uint8                     use_small_banks,
    SOC_SAND_IN  uint8                     no_insertion_priority_order,
    SOC_SAND_IN  uint8                     sparse_priorities,
    SOC_SAND_IN  uint8                     is_direct
  )
{
  uint32
    res = SOC_SAND_OK;
  uint32
    entry_count_per_bank,
    max_nof_entries,
    bank_id;
  uint8
    valid;
  SOC_SAND_SORTED_LIST_PTR
    priorities;
  SOC_SAND_SORTED_LIST_INIT_INFO
    priorities_init_info;
    
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_CREATE_UNSAFE);

  /*
   *  Verify
   */
  res = arad_tcam_db_create_verify(
          unit,
          tcam_db_id,
          entry_size,
          prefix_size,
          action_bitmap_ndx,
          is_direct
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  /*
   * Check whether this database is already initialized
   */
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.valid.get(
          unit,
          tcam_db_id,
          &valid
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 7, exit);
  if (valid)
  {
      LOG_ERROR(BSL_LS_SOC_TCAM,
                (BSL_META_U(unit,
                            "     "
                            "Error in TCAM bank create: "
                            "the TCAM database %d already exists.\n\r"), tcam_db_id));
    SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DATABASE_ALREADY_EXISTS_ERR, 10, exit);
  }

  /* Use bank 0 as the reference since it has more lines than banks 12/13 */
  entry_count_per_bank = arad_tcam_bank_entry_size_to_entry_count_get(
                            unit,
                            is_direct ? ARAD_TCAM_BANK_ENTRY_SIZE_160_BITS : entry_size, 
                            0);
  max_nof_entries      = entry_count_per_bank * SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit);

  /*
   * Initialize the entry priority list
   */
  soc_sand_os_memset(&priorities_init_info, 0x0, sizeof(SOC_SAND_SORTED_LIST_INIT_INFO));

  priorities_init_info.prime_handle  = unit;
  priorities_init_info.sec_handle    = 0;
  priorities_init_info.list_size     = max_nof_entries;
  priorities_init_info.key_size      = ARAD_TCAM_DB_LIST_KEY_SIZE * sizeof(uint8);
  priorities_init_info.data_size     = ARAD_TCAM_DB_LIST_DATA_SIZE * sizeof(uint8);
  priorities_init_info.get_entry_fun = NULL;
  priorities_init_info.set_entry_fun = NULL;
  priorities_init_info.cmp_func_type   = SOC_SAND_SORTED_LIST_CMP_FUNC_TYPE_TCAM;

  res = soc_sand_sorted_list_create(unit, &priorities, priorities_init_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db_priorities.set(unit, tcam_db_id, priorities);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.set(
    unit,
    tcam_db_id,
    entry_size
  );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.action_bitmap_ndx.set(
    unit,
    tcam_db_id,
    action_bitmap_ndx
  );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.prefix_size.set(
    unit,
    tcam_db_id,
    prefix_size
  );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.is_direct.set(
    unit, 
    tcam_db_id, 
    is_direct
  );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.use_small_banks.set(
    unit, 
    tcam_db_id, 
    use_small_banks
  );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 70, exit);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.no_insertion_priority_order.set(
    unit, 
    tcam_db_id, 
    no_insertion_priority_order
  );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 80, exit);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.sparse_priorities.set(
    unit, 
    tcam_db_id, 
    sparse_priorities
  );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 90, exit);

  for (bank_id = 0; bank_id < SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit); ++bank_id)
  {
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.set(
      unit,
      tcam_db_id,
      bank_id,
      FALSE
    );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 100, exit);
  }

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.valid.set(
    unit,
    tcam_db_id,
    TRUE
  );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 110, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_create_unsafe()", tcam_db_id, 0);
}

uint32
  arad_tcam_db_create_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size,
    SOC_SAND_IN  uint32               prefix_size,
    SOC_SAND_IN  ARAD_TCAM_ACTION_SIZE     action_bitmap_ndx,
    SOC_SAND_IN  uint8                     is_direct
  )
{

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_CREATE_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(tcam_db_id, ARAD_TCAM_TCAM_DB_ID_MAX, ARAD_TCAM_TCAM_DB_ID_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(entry_size, ARAD_TCAM_ENTRY_SIZE_MAX+(is_direct?1:0), ARAD_TCAM_ENTRY_SIZE_OUT_OF_RANGE_ERR, 20, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(prefix_size, ARAD_TCAM_PREFIX_SIZE_MAX, ARAD_TCAM_PREFIX_SIZE_OUT_OF_RANGE_ERR, 30, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(action_bitmap_ndx, ARAD_TCAM_ACTION_SIZE_MAX, ARAD_TCAM_ACTION_SIZE_OUT_OF_RANGE_ERR, 50, exit);
  
  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_create_verify()", tcam_db_id, 0);
}

/*********************************************************************
*     Destroys a TCAM database and frees the resources
 *     allocated to it.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_tcam_db_destroy_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id
  )
{
  uint32
      next_priority,
      next_entry,
      tcam_db_other_ndx,
    res = SOC_SAND_OK;
  uint32
    bank_id;
  ARAD_TCAM_LOCATION
      location_curr,
    location;
  SOC_SAND_SORTED_LIST_PTR
    prio_list;
  uint8
      found,
      is_forbidden;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_DESTROY_UNSAFE);

  ARAD_TCAM_DB_EXISTS_ASSERT;

  /*
   *  Verify
   */
  res = arad_tcam_db_destroy_verify(
          unit,
          tcam_db_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  /*
   *  Invalidate all entries
   */
  /* Start from the first entry - get the closest existing entry */
  ARAD_TCAM_LOCATION_clear(&location_curr);
  location_curr.bank_id = 0;
  location_curr.entry = 0;

  do
  {
      res = arad_tcam_db_occ_bmp_entry_closest_get(
                unit,
                tcam_db_id,
                ARAD_TCAM_DIRECTION_FORWARD,
                &location_curr,
                &next_entry,
                &next_priority,
                &location,
                &found
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    if(found)
    {
        res = arad_tcam_bank_entry_invalidate_unsafe(
                unit,
                TRUE, /* set_hw_also */
                FALSE, /* is_bank_share_update */
                location.bank_id,
                location.entry
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

      location_curr = location;
    }
  } while (found);

  for (bank_id = 0; bank_id < SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit); ++bank_id)
  {
      /* 
       * Remove the bank from the TCAM DB
       */
      res = arad_tcam_db_bank_remove_unsafe(unit, tcam_db_id, bank_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 33, exit);
  }

  /* 
   * Update the SW DB for the forbidden DBs 
   */
  for (tcam_db_other_ndx = 0; tcam_db_other_ndx < ARAD_TCAM_MAX_NOF_LISTS; tcam_db_other_ndx++) {
      res = arad_sw_db_tcam_db_forbidden_dbs_get(unit, tcam_db_other_ndx, tcam_db_id, &is_forbidden);
      SOC_SAND_CHECK_FUNC_RESULT(res, 34, exit);
      if (is_forbidden) {
          /* Set in both directions */
          res = arad_sw_db_tcam_db_forbidden_dbs_set(unit, tcam_db_other_ndx, tcam_db_id, FALSE /* is_forbidden */);
          SOC_SAND_CHECK_FUNC_RESULT(res, 34, exit);
      }
  }


  /*
   * Free allocated memory
   */
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db_priorities.get(unit,tcam_db_id, &prio_list);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  res = soc_sand_sorted_list_destroy(unit, prio_list);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.valid.set(unit, tcam_db_id, FALSE);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_destroy_unsafe()", tcam_db_id, 0);
}

uint32
  arad_tcam_db_destroy_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id
  )
{

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_DESTROY_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(tcam_db_id, ARAD_TCAM_TCAM_DB_ID_MAX, ARAD_TCAM_TCAM_DB_ID_OUT_OF_RANGE_ERR, 10, exit);

  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_destroy_verify()", tcam_db_id, 0);
}

/*********************************************************************
*     Adds a TCAM bank to a database.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_tcam_db_bank_add_unsafe(
    SOC_SAND_IN  int             unit,
    SOC_SAND_IN  uint32             tcam_db_id,
    SOC_SAND_IN  uint32             access_profile_array_id, /* in case of 320b DB, which access profile */
    SOC_SAND_IN  uint32             bank_id,
    SOC_SAND_IN  uint8              is_direct, /* in case of FP */
    SOC_SAND_IN  ARAD_TCAM_PREFIX   *prefix
  )
{
  uint32
      nof_entries,
      access_profile_id,
    res = SOC_SAND_OK;
  uint8
    is_used,
    has_direct_table;
  ARAD_TCAM_BANK_ENTRY_SIZE
    db_entry_size,
    bank_entry_size;
  ARAD_TCAM_BANK_OWNER
    db_bank_owner,
    bank_owner;
  SOC_SAND_OCC_BM_PTR
    occ_bm;
  SOC_SAND_OCC_BM_INIT_INFO
    occ_bm_init_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_BANK_ADD_UNSAFE);
  
  SOC_SAND_CHECK_NULL_INPUT(prefix);

  ARAD_TCAM_DB_EXISTS_ASSERT;

  /*
   *  Verify
   */
  res = arad_tcam_db_bank_add_verify(
          unit,
          tcam_db_id,
          bank_id,
          prefix
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  /*
   *  Check whether this bank is already used by this database
   */
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(
          unit,
          tcam_db_id,
          bank_id,
          &is_used
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  if (is_used)
  {
    LOG_ERROR(BSL_LS_SOC_TCAM,
              (BSL_META_U(unit,
                          "Unit %d Tcam DB id %d Bank id %d - Failed to add bank. The bank already exists.\n\r"),
               unit, tcam_db_id, bank_id));
    SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_BANK_ALREADY_USED_ERR, 20, exit);
  }

  /*
   *  Make sure the new bank's entry size is the same as the database's
   */
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit, tcam_db_id, &db_entry_size);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 25, exit);
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.entry_size.get(unit, bank_id, &bank_entry_size);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 28, exit);

  if (db_entry_size != bank_entry_size 
      && !is_direct)
  {
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.has_direct_table.get(unit, bank_id, &has_direct_table);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
      if(!has_direct_table) {
          LOG_ERROR(BSL_LS_SOC_TCAM,
                    (BSL_META_U(unit,
                                "     "
                                "Error in TCAM bank create: "
                                "For TCAM database %d and bank %d, different sizes between bank %s and DB %s \n\r"), 
                     tcam_db_id, bank_id, SOC_TMC_TCAM_BANK_ENTRY_SIZE_to_string(bank_entry_size), 
                     SOC_TMC_TCAM_BANK_ENTRY_SIZE_to_string(db_entry_size)));
          LOG_ERROR(BSL_LS_SOC_TCAM,
                    (BSL_META_U(unit,
                                "\n\r")));
          SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_ENTRY_SIZE_MISMATCH_ERR, 30, exit);
      }
  }

  res = arad_tcam_bank_owner_get_unsafe(unit, bank_id, &bank_owner);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.access_profile_id.get(unit,tcam_db_id, access_profile_array_id, &access_profile_id);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam_mgmt.profiles.bank_owner.get(
          unit,
          access_profile_id,
          &db_bank_owner
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

  /* Validate that bank owner is the same as database, or no owner at all yet */
  if ((db_bank_owner != bank_owner) && (bank_owner != ARAD_TCAM_NOF_BANK_OWNERS))
  {
    LOG_ERROR(BSL_LS_SOC_TCAM,
              (BSL_META_U(unit,
                          "Unit %d Tcam DB id %d Bank id %d - Failed to add bank.\n\r"
                          "Owner mismatch : Data base owner %d, Bank owner %d\n\r"), unit, tcam_db_id, bank_id, db_bank_owner, bank_owner));
    SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_BANK_OWNER_MISMATCH_ERR, 45, exit);
  }

  /*
   *  Mark the bank as used by this database
   */
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.set(
    unit,
    tcam_db_id,
    bank_id,
    TRUE
  );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  /* 
   * Only if this DB is Direct Table set the bank property
   * in order not to run over an existing configuration
   */
  if(is_direct) {
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.has_direct_table.set(
        unit, 
        bank_id, 
        is_direct
      );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
  }

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.prefix.set(
    unit,
    tcam_db_id,
    prefix
  );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 21, exit);

  /* 
   * Create the occupation bitmap 
   */ 
  if (access_profile_array_id == 0) {
      /*
       *  Calculate the number of entries in this bank
       */
      nof_entries = arad_tcam_bank_entry_size_to_entry_count_get(
                        unit,
                        is_direct ? ARAD_TCAM_BANK_ENTRY_SIZE_160_BITS : bank_entry_size, 
                        bank_id);

      /* No need to create it for the other bank - never accessed */
      soc_sand_SAND_OCC_BM_INIT_INFO_clear(&occ_bm_init_info);
      occ_bm_init_info.size     = nof_entries;
      occ_bm_init_info.init_val = TRUE;    /* Occupation bitmap is inverse of natural: a zero bit means there is an entry */

      res = soc_sand_occ_bm_create(unit, &occ_bm_init_info, &occ_bm);
      SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db_entries_used.set(unit, tcam_db_id, bank_id, occ_bm);
      SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_bank_add_unsafe()", tcam_db_id, bank_id);
}

uint32
  arad_tcam_db_bank_add_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               bank_id,
    SOC_SAND_IN  ARAD_TCAM_PREFIX          *prefix
  )
{  
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_BANK_ADD_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(tcam_db_id, ARAD_TCAM_TCAM_DB_ID_MAX, ARAD_TCAM_TCAM_DB_ID_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_NOF(bank_id, SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit), ARAD_TCAM_BANK_ID_OUT_OF_RANGE_ERR, 20, exit);
  ARAD_STRUCT_VERIFY(ARAD_TCAM_PREFIX, prefix, 30, exit);
  
  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_bank_add_verify()", 0, 0);
}

/*********************************************************************
*     Removes a bank from the database's resource pool.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_tcam_db_bank_remove_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               bank_id
  )
{
  uint32
      nof_access_profiles,
      access_profile_array_id,
      access_profile_id,
      pd_profile_id,
      action_hw,
    res = SOC_SAND_OK;
  uint32
    shift,
    nof_dbs;
  uint8
    base,
    ndx;
  ARAD_TCAM_PREFIX
    prefix;
  SOC_SAND_OCC_BM_PTR
    occ_bm;
  uint8
    is_inverse,
    is_used,
    has_direct_table,
    is_direct;
  ARAD_PP_IHB_TCAM_ACCESS_PROFILE_TBL_DATA
    access_tbl_data;
  ARAD_PP_IHB_TCAM_PD_PROFILE_TBL_DATA
    pd_tbl_data;
  uint32
    bitmap_lcl[1];
  ARAD_TCAM_BANK_ENTRY_SIZE
    entry_size;
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_BANK_REMOVE_UNSAFE);

  ARAD_TCAM_DB_EXISTS_ASSERT;

  /*
   *  Verify
   */
  res = arad_tcam_db_bank_remove_verify(
          unit,
          tcam_db_id,
          bank_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  /* 
   *  Remove this bank from this DB:
   *  1. SW DB with the is_used parameter
   *  2. HW with TCAM DB profile
   */
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(unit, tcam_db_id, bank_id, &is_used);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit); 
  if (is_used)
  {
    /*
     *  Release grabbed bank prefixes (the interface is broken here, it should really get fixed)
     */
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.prefix.get(unit, tcam_db_id, &prefix);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit);

    /* Indicate these bank prefixes are not taken */
    shift = 4 - prefix.length;
    base = (uint8) (prefix.bits << shift);
    for (ndx = 0; ndx < SOC_SAND_BIT(shift); ++ndx)
    {
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam_mgmt.banks.prefix_db.set(unit, bank_id, base | ndx, ARAD_TCAM_MAX_NOF_LISTS);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit);
    }

      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.set(unit, tcam_db_id, bank_id, FALSE);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit);

      /* Check if used bank has direct table, and if so check if it this DB */
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.has_direct_table.get(unit, bank_id, &has_direct_table);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit);
      if(has_direct_table)
      {
          res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.is_direct.get(unit, tcam_db_id, &is_direct);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 6, exit);
          
          if(is_direct) {
              res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.has_direct_table.set(unit, bank_id, FALSE);
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 7, exit);
          }
      }

      /* Reset the SW DB for the number of entries per bank */
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_nof_entries.set(unit, tcam_db_id, bank_id, 0 /* bank_nof_entries */);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);

      /*
       *  If the bank is not used at all now, mark it uninitialized
       */
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam_mgmt.banks.nof_dbs.get(unit, bank_id, &nof_dbs);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam_mgmt.banks.nof_dbs.set(unit, bank_id, --nof_dbs);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 8, exit);
      if (nof_dbs == 0)
      {
        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.valid.set(unit, bank_id, FALSE);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 9, exit);

        for (is_inverse = 0; is_inverse < ARAD_TCAM_NOF_OCCUPATIONS_BITMAPS; is_inverse++) {

            res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_bank_entries_used.get(unit, bank_id, is_inverse, &occ_bm);
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
            soc_sand_occ_bm_destroy(unit, occ_bm);

            SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);
        }      
      }
  }

  /* Update the HW Access-Profiles */
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit, tcam_db_id, &entry_size);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 15, exit);
  nof_access_profiles = (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS)? 2 : 1;
  for (access_profile_array_id = 0; access_profile_array_id < nof_access_profiles; access_profile_array_id++) {
      /*
       *  Get managed database parameters
       */
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.access_profile_id.get(unit, tcam_db_id, access_profile_array_id, &access_profile_id);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 17, exit);

      res = arad_pp_ihb_tcam_access_profile_tbl_read_unsafe(
              unit,
              access_profile_id,
              &access_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
      action_hw = 0;
      SHR_BITCOPY_RANGE(&access_tbl_data.action_bitmap, bank_id*ARAD_TCAM_ACTION_SIZE_NOF_BITS, &action_hw, 0, ARAD_TCAM_ACTION_SIZE_NOF_BITS);
      res = arad_pp_ihb_tcam_access_profile_tbl_write_unsafe(
              unit,
              access_profile_id,
              &access_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

      /* Set HW table for priority_decoder information */
      pd_profile_id = access_profile_id; /* Always 1*1 mapping */
      res = arad_pp_ihb_tcam_pd_profile_tbl_read_unsafe(
              unit,
              pd_profile_id,
              &pd_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
      *bitmap_lcl = pd_tbl_data.bitmap;
      SHR_BITCLR(bitmap_lcl, bank_id);
      pd_tbl_data.bitmap = *bitmap_lcl;
      res = arad_pp_ihb_tcam_pd_profile_tbl_write_unsafe(
              unit,
              pd_profile_id,
              &pd_tbl_data
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
      if (access_profile_array_id == 0) {
          if (is_used) {
              res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db_entries_used.get(unit, tcam_db_id, bank_id, &occ_bm);
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
              if (occ_bm) {
                  /*
                   * Enter if there is a legitimate OCC bit map related to this bank on this db.
                   */
                  soc_sand_occ_bm_destroy(unit, occ_bm) ;
                  /*
                   * Mark on global sw state: This bank on this db has not legitimate OCC bit map.
                   * (A value of '0' {NULL} on 'occ_bm' indicates 'an empty entry'.)
                   * (We need a macro to convert NULL, which may be a pointer of 64 bits on
                   * some systems, to SOC_SAND_OCC_BM_PTR, which is uint32. Otherwise, compiler
                   * may complain)
                   */
                  occ_bm = (SOC_SAND_OCC_BM_PTR)PTR_TO_INT(NULL) ;
                  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db_entries_used.set(unit, tcam_db_id, bank_id, occ_bm);
                  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
              }
          }
      }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_bank_remove_unsafe()", 0, 0);
}

uint32
  arad_tcam_db_bank_remove_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               bank_id
  )
{

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_BANK_REMOVE_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(tcam_db_id, ARAD_TCAM_TCAM_DB_ID_MAX, ARAD_TCAM_TCAM_DB_ID_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_NOF(bank_id, SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit), ARAD_TCAM_BANK_ID_OUT_OF_RANGE_ERR, 20, exit);

  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_bank_remove_verify()", 0, 0);
}

/*********************************************************************
*     Returns the number of banks occupied by the database.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_tcam_db_nof_banks_get_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_OUT uint32               *nof_banks
  )
{
  uint32
    res = SOC_SAND_OK;
  uint32
    bank_id;
  uint8
    is_used;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_NOF_BANKS_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(nof_banks);

  /*
   *  Verify
   */
  res = arad_tcam_db_nof_banks_get_verify(
          unit,
          tcam_db_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  ARAD_TCAM_DB_EXISTS_ASSERT;

  *nof_banks = 0;
  for (bank_id = 0; bank_id < SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit); ++bank_id)
  {
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(unit, tcam_db_id, bank_id, &is_used);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
    if (is_used)
    {
      ++*nof_banks;
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_nof_banks_get_unsafe()", 0, 0);
}

uint32
  arad_tcam_db_nof_banks_get_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id
  )
{
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_NOF_BANKS_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(tcam_db_id, ARAD_TCAM_TCAM_DB_ID_MAX, ARAD_TCAM_TCAM_DB_ID_OUT_OF_RANGE_ERR, 10, exit);

  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_nof_banks_get_verify()", 0, 0);
}

/*********************************************************************
*     Returns the database's prefix in the bank.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_tcam_db_bank_prefix_get_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               bank_id,
    SOC_SAND_OUT ARAD_TCAM_PREFIX          *prefix
  )
{
  uint32
    res = SOC_SAND_OK;
  uint8
    has_bank;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_BANK_PREFIX_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(prefix);

  /*
   *  Verify
   */
  res = arad_tcam_db_bank_prefix_get_verify(
          unit,
          tcam_db_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  ARAD_TCAM_PREFIX_clear(prefix);

  ARAD_TCAM_DB_EXISTS_ASSERT;

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(
          unit,
          tcam_db_id,
          bank_id,
          &has_bank
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  if (!has_bank)
  {
    LOG_ERROR(BSL_LS_SOC_TCAM,
              (BSL_META_U(unit,
                          "Unit %d Tcam Db id %d Bank id %d - Failed to get prefix. The bank is not in use.\n\r"),
               unit, tcam_db_id, bank_id));
    SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_BANK_NOT_USED_ERR, 20, exit);
  }

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.prefix.get(
    unit,
    tcam_db_id,
    prefix
  );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_bank_prefix_get_unsafe()", 0, 0);
}

uint32
  arad_tcam_db_bank_prefix_get_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id
  )
{
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_BANK_PREFIX_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(tcam_db_id, ARAD_TCAM_TCAM_DB_ID_MAX, ARAD_TCAM_TCAM_DB_ID_OUT_OF_RANGE_ERR, 10, exit);

  SOC_SAND_TODO_IMPLEMENT_WARNING;
  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_bank_prefix_get_verify()", 0, 0);
}

/*********************************************************************
*     Returns the database's entry size.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_tcam_db_entry_size_get_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_OUT ARAD_TCAM_BANK_ENTRY_SIZE *entry_size
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_ENTRY_SIZE_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(entry_size);

  /*
   *  Verify
   */
  res = arad_tcam_db_entry_size_get_verify(
          unit,
          tcam_db_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  ARAD_TCAM_DB_EXISTS_ASSERT;

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(
          unit,
          tcam_db_id,
          entry_size
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_size_get_unsafe()", 0, 0);
}

uint32
  arad_tcam_db_entry_size_get_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id
  )
{
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_ENTRY_SIZE_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(tcam_db_id, ARAD_TCAM_TCAM_DB_ID_MAX, ARAD_TCAM_TCAM_DB_ID_OUT_OF_RANGE_ERR, 10, exit);

  SOC_SAND_TODO_IMPLEMENT_WARNING;
  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_size_get_verify()", 0, 0);
}

/*********************************************************************
*     Add an entry to the Database direct table.
 *     Details: in the H file. (search for prototype)
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
  )
{
  uint32
      res = SOC_SAND_OK;
  ARAD_TCAM_LOCATION 
      location;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_DIRECT_TBL_ENTRY_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(action);

  res = arad_tcam_db_direct_tbl_entry_set_verify(
          unit,
          bank_id,
          address,
          action_bitmap_ndx,
          action
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (!valid) {
      action->value[0] = 0;
  }

  res = arad_tcam_tbl_action_write(
          unit,
          bank_id, 
          address,
          action_bitmap_ndx,  
          0, /* hit_bit */        
          action
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /* Set the occupation bitmap to indicate which entries are taken */
  ARAD_TCAM_LOCATION_clear(&location);
  location.bank_id = bank_id;
  location.entry = address;
  res = arad_tcam_db_bank_occ_bitmap_entry_set(
            unit,
            tcam_db_id,
            &location,
            valid
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_direct_tbl_entry_set_unsafe()", address, 0);
}

/*********************************************************************
*     Add an entry to the Database direct table.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_tcam_db_direct_tbl_entry_get_unsafe(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  uint32                   tcam_db_id,
      SOC_SAND_IN  uint32                   bank_id,
      SOC_SAND_IN  uint32                   address,
      SOC_SAND_IN  ARAD_TCAM_ACTION_SIZE    action_bitmap_ndx,      
      SOC_SAND_INOUT  ARAD_TCAM_ACTION      *action,
      SOC_SAND_OUT  uint8                   *valid
  )
{
  uint32
      res = SOC_SAND_OK;
  uint8
      hit_bit;
  ARAD_TCAM_LOCATION
      location;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_DIRECT_TBL_ENTRY_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(action);

  res = arad_tcam_db_direct_tbl_entry_set_verify(
          unit,
          bank_id,
          address,
          action_bitmap_ndx,
          action
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_tcam_tbl_action_read(
          unit,
          bank_id,
          address,
          action_bitmap_ndx,
          &hit_bit,
          action
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  
    /* Set the occupation bitmap to indicate which entries are taken */
  ARAD_TCAM_LOCATION_clear(&location);
  location.bank_id = bank_id;
  location.entry = address;
  res = arad_tcam_db_bank_occ_bitmap_entry_get(
            unit,
            tcam_db_id,
            &location,
            valid
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_direct_tbl_entry_get_unsafe()", address, 0);
}

uint32
  arad_tcam_db_direct_tbl_entry_set_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 bank_id,
    SOC_SAND_IN  uint32                 entry_id,
    SOC_SAND_IN  ARAD_TCAM_ACTION_SIZE  action_bitmap_ndx,      
    SOC_SAND_IN  ARAD_TCAM_ACTION       *action
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_DIRECT_TBL_ENTRY_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_NOF(bank_id, SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit), ARAD_TCAM_BANK_ID_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(entry_id, ARAD_TCAM_NOF_LINES_PER_BANK(unit, bank_id), ARAD_TCAM_ENTRY_ID_OUT_OF_RANGE_ERR, 20, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(action_bitmap_ndx, ARAD_TCAM_ACTION_SIZE_MAX, ARAD_TCAM_ACTION_SIZE_OUT_OF_RANGE_ERR, 30, exit);
  ARAD_STRUCT_VERIFY(ARAD_TCAM_ACTION, action, 40, exit);
  
  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_add_verify()", bank_id, entry_id);
}

/*********************************************************************
*     Add an entry to a database.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_tcam_db_entry_add_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               entry_id,
    SOC_SAND_IN  uint32               priority,
    SOC_SAND_IN  ARAD_TCAM_ENTRY      *entry,
    SOC_SAND_IN  ARAD_TCAM_ACTION     *action,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE    *success
  )
{
  uint32
      bank_nof_entries,
    res = SOC_SAND_OK;
  uint8
    is_update_and_invalid,
    entry_exists;
  uint8 has_banks = FALSE;
  ARAD_TCAM_LOCATION
    location;
  ARAD_TCAM_ENTRY
    entry_buffer;
  ARAD_TCAM_ACTION_SIZE 
    action_bitmap_ndx;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_ENTRY_ADD_UNSAFE);

  /*
   *  Verify
   */
  res = arad_tcam_db_entry_add_verify(
          unit,
          tcam_db_id,
          entry_id,
          priority,
          entry,
          action
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  ARAD_TCAM_DB_EXISTS_ASSERT;

  /*
   *  Check whether this database has any banks
   */
  res = arad_tcam_db_has_banks(
          unit,
          tcam_db_id,
          &has_banks
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  if (!has_banks)
  {
    LOG_VERBOSE(BSL_LS_SOC_TCAM,
                (BSL_META_U(unit,
                            "Unit %d Tcam DB id %d - The data base has no bank.\n\r"), unit, tcam_db_id));
    *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
    ARAD_DO_NOTHING_AND_EXIT;
  }

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.action_bitmap_ndx.get(
          unit,
          tcam_db_id,
          &action_bitmap_ndx
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 35, exit);

  /*
   *  The user has to remove the entry first, if it exists
   */
  res = arad_tcam_db_entry_exists_get(
          unit,
          tcam_db_id,
          entry_id,
          &entry_exists
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  /* Allow existing entries only for updates */
  if ((entry_exists && (!entry->is_for_update)) 
      || ((!entry_exists) && entry->is_for_update))
  {
    *success = SOC_SAND_FAILURE_REMOVE_ENTRY_FIRST;
    ARAD_DO_NOTHING_AND_EXIT;
  }
  res = arad_tcam_db_free_location_allocate(
          unit,
          tcam_db_id,
          entry->is_inserted_top,
          priority,
          entry->is_for_update,
          entry_id,
          &is_update_and_invalid,
          &location,
          success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  if (*success == SOC_SAND_SUCCESS)
  {
    /* 
     * Insert in the HW and SW DB the new entry now that there is 
     * some room for it. Sequence: 
     * - Update the SW DB for the number of entries per bank 
     * - Remove the duplicate entry (in case of update and invalid entry, 
     * to skip an unnecessary block move) 
     * - Insert the entry first in the HW in the found location 
     * - Remove the duplicate entry (in case of update and valid entry) 
     * - Insert in the SW DB 
     */

      /* Update the SW DB for the number of entries per bank */
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_nof_entries.get(unit, tcam_db_id, location.bank_id, &bank_nof_entries);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 53, exit);
      bank_nof_entries ++;
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_nof_entries.set(unit, tcam_db_id, location.bank_id, bank_nof_entries);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 55, exit);


     
    if (entry->is_for_update && is_update_and_invalid) {
        res = arad_tcam_db_entry_remove_unsafe(
                unit,
                FALSE /* is_bank_freed_if_no_entry */,
                tcam_db_id,
                entry_id
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
    }

    entry_buffer = *entry;
    res = arad_tcam_db_entry_prefix_stamp(
            unit,
            tcam_db_id,
            &entry_buffer
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

    res = arad_tcam_bank_entry_set_unsafe(
            unit,
            TRUE, /* set_hw_also */
            FALSE, /* is_bank_share_update */
            0, /* hit_bit */
            location.bank_id,
            location.entry,
            action_bitmap_ndx,
            &entry_buffer,
            action
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

    /* 
     * In case of update remove the previous entry 
     * if the updated entry was valid 
     */
    if (entry->is_for_update && (!is_update_and_invalid)) {
        res = arad_tcam_db_entry_remove_unsafe(
                unit,
                TRUE /* is_bank_freed_if_no_entry */,
                tcam_db_id,
                entry_id
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 82, exit);
    }

    /*
     *  Add the new entry to the entry_id -> location hash table
     *  Its insertion is checked when adding a new priority node
     */
    res = arad_tcam_db_entry_id_to_location_entry_add(
            unit,
            tcam_db_id,
            entry_id,
            &location
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
    /*
     *  Add the new entry to the global location table
     */
    res = arad_tcam_global_location_table_entry_add(
            unit,
            tcam_db_id,
            &location,
            entry_id,
            priority
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

    /* 
     * Update the DB-bank occupation bitmap 
     */
    res = arad_tcam_db_bank_occ_bitmap_entry_set(
            unit,
            tcam_db_id,
            &location,
            TRUE /* is_occupied */
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 105, exit);

    /*
     *  Add the new entry to the priorities list
     */
    res = arad_tcam_db_priority_list_entry_add(
            unit,
            tcam_db_id,
            &location,
            entry_id,
            priority,
            FALSE, /* is_update_prio */
            success
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 92, exit);
  }
  else
  {
    *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_add_unsafe()", tcam_db_id, entry_id);
}

uint32
  arad_tcam_db_entry_add_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               entry_id,
    SOC_SAND_IN  uint32               priority,
    SOC_SAND_IN  ARAD_TCAM_ENTRY       *entry,
    SOC_SAND_IN  ARAD_TCAM_ACTION      *action
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_ENTRY_ADD_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(tcam_db_id, ARAD_TCAM_TCAM_DB_ID_MAX, ARAD_TCAM_TCAM_DB_ID_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(entry_id, ARAD_TCAM_ENTRY_ID_MAX, ARAD_TCAM_ENTRY_ID_OUT_OF_RANGE_ERR, 20, exit);

  ARAD_STRUCT_VERIFY(ARAD_TCAM_ENTRY, entry, 40, exit);
  ARAD_STRUCT_VERIFY(ARAD_TCAM_ACTION, action, 50, exit);
  
  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_add_verify()", entry_id, priority);
}

/*********************************************************************
*     Reads an entry from the database.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_tcam_db_entry_get_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               entry_id,
    SOC_SAND_IN  uint8               hit_bit_clear, /* If TRUE, clear the hit bit once read */
    SOC_SAND_OUT uint32               *priority,
    SOC_SAND_OUT ARAD_TCAM_ENTRY     *entry,
    SOC_SAND_OUT ARAD_TCAM_ACTION    *action,
    SOC_SAND_OUT uint8               *found,
    SOC_SAND_OUT uint8               *hit_bit
  )
{
  uint32
    res = SOC_SAND_OK;
  uint32
    stored_entry_id;
  ARAD_TCAM_LOCATION
    location;
  ARAD_TCAM_ACTION_SIZE 
    action_bitmap_ndx;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_ENTRY_GET_UNSAFE);

  /*
   *  Verify
   */
  res = arad_tcam_db_entry_get_verify(
          unit,
          tcam_db_id,
          entry_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  SOC_SAND_CHECK_NULL_INPUT(priority);
  SOC_SAND_CHECK_NULL_INPUT(entry);
  SOC_SAND_CHECK_NULL_INPUT(action);
  SOC_SAND_CHECK_NULL_INPUT(found);

  ARAD_TCAM_ENTRY_clear(entry);
  ARAD_TCAM_ACTION_clear(action);
  *hit_bit = 0;

  ARAD_TCAM_DB_EXISTS_ASSERT;

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.action_bitmap_ndx.get(
          unit,
          tcam_db_id,
          &action_bitmap_ndx
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);


  res = arad_tcam_db_entry_id_to_location_entry_get(
          unit,
          tcam_db_id,
          entry_id,
          TRUE,
          &location,
          found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if (*found)
  {
    /*
     * Get the TCAM key and action
     */
    res = arad_tcam_bank_entry_get_unsafe(
            unit,
            location.bank_id,
            location.entry,
            action_bitmap_ndx,
            hit_bit_clear,
            entry,
            action,
            hit_bit
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    /*
     * Get the entry's priority
     */
    res = arad_tcam_db_priority_list_entry_get(
            unit,
            tcam_db_id,
            &location,
            &stored_entry_id,
            priority
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    if (stored_entry_id != entry_id)
    {
      LOG_ERROR(BSL_LS_SOC_TCAM,
                (BSL_META_U(unit,
                            "Unit %d Tcam DB id %d Entry id %d - The location where the entry lies leads to differnt entry id - %d.\n\r"),
                 unit, tcam_db_id, entry_id, stored_entry_id));
      SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 50, exit);
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_get_unsafe()", tcam_db_id, entry_id);
}

uint32
  arad_tcam_db_entry_get_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               entry_id
  )
{

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_ENTRY_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(tcam_db_id, ARAD_TCAM_TCAM_DB_ID_MAX, ARAD_TCAM_TCAM_DB_ID_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(entry_id, ARAD_TCAM_ENTRY_ID_MAX, ARAD_TCAM_ENTRY_ID_OUT_OF_RANGE_ERR, 20, exit);
 
  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_get_verify()", tcam_db_id, entry_id);
}

/*********************************************************************
*     Search for an entry in a database using the same logic
 *     used by the hardware.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_tcam_db_entry_search_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  ARAD_TCAM_ENTRY           *key,
    SOC_SAND_OUT uint32               *entry_id,
    SOC_SAND_OUT uint8               *found
  )
{
  uint32
    res = SOC_SAND_OK;
  uint8
    hit=0,
    is_used;
  uint32
    priority;
  ARAD_TCAM_BANK_ENTRY_SIZE
    entry_size;
  ARAD_TCAM_LOCATION
    first_hit,
    current;
  ARAD_TCAM_ENTRY
    stamped;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_ENTRY_SEARCH_UNSAFE);

  /*
   *  Verify
   */
  res = arad_tcam_db_entry_search_verify(
          unit,
          tcam_db_id,
          key
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  SOC_SAND_CHECK_NULL_INPUT(key);
  SOC_SAND_CHECK_NULL_INPUT(entry_id);
  SOC_SAND_CHECK_NULL_INPUT(found);

  ARAD_TCAM_DB_EXISTS_ASSERT;

  ARAD_TCAM_LOCATION_clear(&current);
  ARAD_TCAM_LOCATION_clear(&first_hit);

  /*
   *  Make sure the database's keys are not 320-bit wide
   */
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(
                 unit,
                 tcam_db_id,
                 &entry_size
               );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  if (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS)
  {
    LOG_ERROR(BSL_LS_SOC_TCAM,
              (BSL_META_U(unit,
                          "Entry search is not supported for 320 bits bank.\n\r")));
    SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_ENTRY_SEARCH_SIZE_NOT_SUPPORTED_ERR, 20, exit);
  }

  /*
   *  Search for the first match
   */
  *found = FALSE;
  for (current.bank_id = 0; current.bank_id < SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit); ++current.bank_id)
  {
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(unit, tcam_db_id, current.bank_id, &is_used);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
    if (is_used)
    {
      /*
       *  Stamp the key to make sure we only check the relevant database
       */
      stamped = *key;
      res = arad_tcam_db_entry_prefix_stamp(
              unit,
              tcam_db_id,
              &stamped
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

      res = arad_tcam_tbl_bank_compare(
              unit,
              current.bank_id,
              key,
              &current.entry,
              &hit
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

      if (hit)
      {
          first_hit = current;
          *found    = TRUE;
          break;
      }
    }
  }

  if (*found)
  {
    /*
     *  Translate the physical location into entry_id
     */
    res = arad_tcam_db_priority_list_entry_get(
            unit,
            tcam_db_id,
            &first_hit,
            entry_id,
            &priority
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_search_unsafe()", 0, 0);
}

uint32
  arad_tcam_db_entry_search_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  ARAD_TCAM_ENTRY           *key
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_ENTRY_SEARCH_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(tcam_db_id, ARAD_TCAM_TCAM_DB_ID_MAX, ARAD_TCAM_TCAM_DB_ID_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_STRUCT_VERIFY(ARAD_TCAM_ENTRY, key, 20, exit);

  SOC_SAND_TODO_IMPLEMENT_WARNING;
  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_search_verify()", 0, 0);
}

/*********************************************************************
*     Removes an entry from the database.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_tcam_db_entry_remove_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint8                is_bank_freed_if_no_entry, /* If False, an empty bank is not freed */
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               entry_id
  )
{
  uint32
      bank_nof_entries,
      priority = 0,
      res = SOC_SAND_OK;
  ARAD_TCAM_LOCATION
    location;
  uint8
    found;
  ARAD_TCAM_BANK_ENTRY_SIZE
    entry_size;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_ENTRY_REMOVE_UNSAFE);

  ARAD_TCAM_DB_EXISTS_ASSERT;

  /*
   *  Verify
   */
  res = arad_tcam_db_entry_remove_verify(
          unit,
          tcam_db_id,
          entry_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  res = arad_tcam_db_entry_id_to_location_entry_get(
          unit,
          tcam_db_id,
          entry_id,
          TRUE,
          &location,
          &found
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if (found)
  {
    res = arad_tcam_bank_entry_invalidate_unsafe(
            unit,
            TRUE, /* set_hw_also */
            FALSE, /* is_bank_share_update */
            location.bank_id,
            location.entry
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    res = arad_tcam_global_location_table_entry_remove(
            unit,
            tcam_db_id,
            &location,
            &priority
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 35, exit);

    res = arad_tcam_db_priority_list_entry_remove(
            unit,
            tcam_db_id,
            &location,
            entry_id,
            priority,
            FALSE /* is_update_prio */
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    res = arad_tcam_db_bank_occ_bitmap_entry_set(
            unit,
            tcam_db_id,
            &location,
            FALSE /* is_occupied */
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 33, exit);

    res = arad_tcam_db_entry_id_to_location_entry_remove(
            unit,
            tcam_db_id,
            entry_id
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

    /* 
     * Check that it is not the last entry of this bank
     */
    /* Update the SW DB for the number of entries per bank */
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_nof_entries.get(unit, tcam_db_id, location.bank_id, &bank_nof_entries);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);
    if (bank_nof_entries == 0) {
        /* Inconsistent data since this bank canot be empty */
        SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 18, exit);
    }
    bank_nof_entries --;
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_nof_entries.set(unit, tcam_db_id, location.bank_id, bank_nof_entries);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit, tcam_db_id, &entry_size);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);
    /* 
     * Free the bank from this DB if: 
     * - no more entries 
     * - not for an update entry (is_bank_freed_if_no_entry = FALSE)
     */
    if ((bank_nof_entries == 0) && is_bank_freed_if_no_entry) {
        /* Remove this bank from the TCAM DB bank list */
        res = arad_tcam_db_bank_remove_unsafe(unit, tcam_db_id, location.bank_id);
        SOC_SAND_CHECK_FUNC_RESULT(res, 33, exit);

        /* In case of 320b, remove next bank also */
        if (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) {
            res = arad_tcam_db_bank_remove_unsafe(unit, tcam_db_id, location.bank_id + 1);
            SOC_SAND_CHECK_FUNC_RESULT(res, 34, exit);
        }
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_remove_unsafe()", 0, 0);
}

uint32
  arad_tcam_db_entry_remove_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               entry_id
  )
{

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_ENTRY_REMOVE_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(tcam_db_id, ARAD_TCAM_TCAM_DB_ID_MAX, ARAD_TCAM_TCAM_DB_ID_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(entry_id, ARAD_TCAM_ENTRY_ID_MAX, ARAD_TCAM_ENTRY_ID_OUT_OF_RANGE_ERR, 20, exit);

  SOC_SAND_TODO_IMPLEMENT_WARNING;
  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_entry_remove_verify()", 0, 0);
}

/*********************************************************************
*     Queries whether the bank belongs to the database or not.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_tcam_db_is_bank_used_unsafe(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               bank_id,
    SOC_SAND_OUT uint8               *is_used
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_IS_BANK_USED_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(is_used);

  ARAD_TCAM_DB_EXISTS_ASSERT;

  /*
   *  Verify
   */
  res = arad_tcam_db_is_bank_used_verify(
          unit,
          tcam_db_id,
          bank_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(
               unit,
               tcam_db_id,
               bank_id,
               is_used
             );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_is_bank_used_unsafe()", tcam_db_id, bank_id);
}

uint32
  arad_tcam_db_is_bank_used_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32               tcam_db_id,
    SOC_SAND_IN  uint32               bank_id
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_TCAM_DB_IS_BANK_USED_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(tcam_db_id, ARAD_TCAM_TCAM_DB_ID_MAX, ARAD_TCAM_TCAM_DB_ID_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_NOF(bank_id, SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit), ARAD_TCAM_BANK_ID_OUT_OF_RANGE_ERR, 20, exit);
  
  ARAD_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_db_is_bank_used_verify()", tcam_db_id, bank_id);
}


/*********************************************************************
*     Compress a TCAM Database: compress the entries to minimum
*     number of banks.
*********************************************************************/
uint32
  arad_tcam_managed_db_compress_unsafe(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32               tcam_db_id
  )
{
  uint32
      next_entry,
      next_priority,
      bank_id_to,
      bank_id_from,
      bank_nof_entries,
      nof_entries_free,
    res = SOC_SAND_OK;
  ARAD_TCAM_LOCATION
      location_to,
      location,
      location_curr;
  ARAD_TCAM_RANGE
    range;
  SOC_SAND_SUCCESS_FAILURE 
      success;
  ARAD_TCAM_BANK_ENTRY_SIZE
    entry_size;
  uint8
      found;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* 
   * Compress the bank entries: 
   * - scan all the TCAM entries of this DB:
   *    1. For the first entry, assign the highest location in the same bank
   *    2. For the coming entries, assign the highest location in the current bank
   *    Once the current bank has no more entries, use the next bank in the bank bitmap
   * - The TCAM bank is released once it has no more entries of this DB 
   * (the last entry move releases the bank)
   */  

  ARAD_TCAM_DB_EXISTS_ASSERT;

  /*
   *  Scan the DB entries
   */
  /* Start from the first entry - get the closest existing entry */
  ARAD_TCAM_LOCATION_clear(&location_curr);
  location_curr.bank_id = 0;
  location_curr.entry = 0;
  res = arad_tcam_db_occ_bmp_entry_closest_get(
            unit,
            tcam_db_id,
            ARAD_TCAM_DIRECTION_FORWARD,
            &location_curr,
            &next_entry,
            &next_priority,
            &location,
            &found
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  bank_id_to = location.bank_id; /* Destination bank */
  while (found)
  {
      bank_id_from = location.bank_id; /* Current entry bank */
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.nof_entries_free.get(unit, bank_id_to, &nof_entries_free);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
      if (nof_entries_free == 0) {
          /* 
           * It cannot be that the new destination bank is between the old dest bank 
           * and the current bank. Otherwise, the previous entry was moved 
           * from this third bank to the old dest 
           * bank, so there is no more entries of this DB in this third bank.
           */
          bank_id_to = bank_id_from;
      }

      /* 
       * Move the entry to the top of destination bank: 
       * 1. Get the first free index. 
       * 2. If same location, skip. Otherwise move the entry. 
       * 3. Get the next entry
       * 4. If the destination bank is full, the new dest bank is the 
       * current bank  
       */
      /* Skip the search if first line and same bank */
      if (!((bank_id_to == bank_id_from) && (location.entry == 0))) {
          /* Search for an empty entry in the proper range */
          ARAD_TCAM_RANGE_clear(&range);
          range.min.bank_id = bank_id_to;
          range.min.entry = 0;
          range.max.bank_id = bank_id_from;
          range.max.entry = location.entry;
          res = arad_tcam_db_empty_location_in_range_find(
                unit,
                tcam_db_id,
                TRUE, /* is_inserted_top */
                &range,
                &location_to,
                &success
              );
          SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

          if (success == SOC_SAND_SUCCESS) {
              /* Find new location, move to it */
              if (location_to.bank_id != bank_id_to) {
                  /* Incoherence error */
                  LOG_ERROR(BSL_LS_SOC_TCAM,
                            (BSL_META_U(unit,
                                        "Unit %d Tcam db id %d: entry in line (%d, %d) is supposed to move to Bank id %d = Bank id %d and line %d.\n\r"),
                             unit, tcam_db_id, location.bank_id, location.entry, bank_id_to, location_to.bank_id, location_to.entry));
                  SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 25, exit);
              }
              res = arad_tcam_db_entry_move(
                      unit,
                      TRUE, /* set_hw_also */
                      FALSE, /* is_bank_share_update */
                      &location,
                      &location_to
                    );
              SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

              /* Remove this bank if no more entries */
              res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit, tcam_db_id, &entry_size);
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 31, exit);
              res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_nof_entries.get(unit, tcam_db_id, location.bank_id, &bank_nof_entries);
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);
              if (bank_nof_entries == 0) {
                  /* Remove this bank from the TCAM DB bank list */
                  res = arad_tcam_db_bank_remove_unsafe(unit, tcam_db_id, location.bank_id);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 33, exit);

                  /* In case of 320b, remove next bank also */
                  if (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) {
                      res = arad_tcam_db_bank_remove_unsafe(unit, tcam_db_id, location.bank_id + 1);
                      SOC_SAND_CHECK_FUNC_RESULT(res, 34, exit);
                  }
              }
          }
      }

      location_curr = location;
      res = arad_tcam_db_occ_bmp_entry_closest_get(
                unit,
                tcam_db_id,
                ARAD_TCAM_DIRECTION_FORWARD,
                &location_curr,
                &next_entry,
                &next_priority,
                &location,
                &found
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  }



exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_managed_db_compress_unsafe()", tcam_db_id, 0);
}


#ifdef ARAD_TCAM_BANK_INIT_MOVE_ENTRIES_ENABLE
    /* 
     * Further optimization during the bank init. 
     * In case of dynamic entry insertions (is_inserted_top FALSE), 
     * after a bank init, the future entries have a priori any priority. 
     * In the worst-case scenario (insert always higher priority entries), 
     * if the new bank is empty by default, each entry insertion is very long: 
     * 1. Move the last line of previous bank to this bank 
     * 2. HW-move of the whole bank one line downwards
     * 3. Set the entry in the top line 
     *  
     * To enable this worst-case scenario, move entries of this Database in 
     * the previous and next banks to this new bank. Try to spread the load of 
     * the DB between the banks (i.e. ratio number of DB entries in this bank / 
     * number of free entries in this bank). 
     *  
     * Following steps: 
     * - Go over the banks of this DB, and find if there is a previous and next bank 
     * - Retrieve for these banks the number of DB entries and the number of free entries 
     * - Compute the number of entries to move from these banks, and the jump (number of 
     * free lines between 2 moved entries). 
     * -  For both banks (previous and next): 
     *  1. Find the location of the first entry to move
     *  2. Move all the entries one by one without influencing the traffic
     */
uint32
    arad_tcam_new_bank_move_entries(
      SOC_SAND_IN  int            unit,
      SOC_SAND_IN  uint32            tcam_db_id,
      SOC_SAND_IN  uint32            bank_id,
      SOC_SAND_IN  uint8             is_direct,
      SOC_SAND_IN  uint32            access_profile_array_id,
      SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size,
      SOC_SAND_IN  uint8             is_inserted_top
  )
{
  uint32
    res = SOC_SAND_OK;
  uint32
    bank_idx,
    nof_lines_jump = 0, /* Interstice between the different lines to insert in the new bank */
      line_idx = 0, 
    line_ratio = 0, 
    entry_idx, 
      entry_id,
    nof_entries_to_move[ARAD_TCAM_BANK_INIT_MOVE_ENTRIES_NOF_ADJACENT_BANKS] = {0, 0}, /* Previous and next banks */
    nof_entries_db[ARAD_TCAM_BANK_INIT_MOVE_ENTRIES_NOF_ADJACENT_BANKS] = {0, 0}, /* Previous and next banks */
    nof_entries_free = 0, /* number of free entry in current bank */
    start,
    end;
  uint8
    is_bank_upper,
    other_bank_ndx,
      found,
    bank_found_other[ARAD_TCAM_BANK_INIT_MOVE_ENTRIES_NOF_ADJACENT_BANKS] = {0, 0},
    line_found,
    is_used;
  uint32
    next_priority;
  ARAD_TCAM_LOCATION
      curr_location,
    next_location;
  ARAD_TCAM_LOCATION       
      empty_location;
  ARAD_TCAM_DIRECTION 
      direction;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);


    if (is_direct || (is_inserted_top == TRUE)
        || ((entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) && (access_profile_array_id == 0))) {
        /* not for direct table, and once both banks have been allocated for 320b DBs */
        ARAD_DO_NOTHING_AND_EXIT;
    }


    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.nof_entries_free.get(unit, bank_id, &nof_entries_free);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
    for (bank_idx = 0; bank_idx < SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit); ++bank_idx)
    {
        if (bank_idx == bank_id) {
            /* Do not take in account the new bank */
            continue;
        }

        if ((entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) && ((bank_idx % 2) == 1)) {
            /* Take in account only the first bank of each bank pair */
            continue;
        }

        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(unit, tcam_db_id, bank_idx, &is_used);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
        if (!is_used) {
            /* Look only at the DB banks */
            continue;
        }

        is_bank_upper = (bank_idx > bank_id)? 1: 0;
        bank_found_other[is_bank_upper] = TRUE;
        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_nof_entries.get(unit, tcam_db_id, bank_idx, &(nof_entries_db[is_bank_upper]));
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
    }

    /* Not the first bank */
    if (bank_found_other[0] || bank_found_other[1]) {
        for (other_bank_ndx = 0; other_bank_ndx < ARAD_TCAM_BANK_INIT_MOVE_ENTRIES_NOF_ADJACENT_BANKS; ++other_bank_ndx)
        {
            /*
             * After computation, for equal ratio, if x1, x3 is the number of entries in previous and next 
             * banks, and x2 the number of free entries in current bank, 
             * number of entries to move (y1 or y3) = ((x1 or x3) * x2) / (x1 + x2 + x3);
             */
            nof_entries_to_move[other_bank_ndx] = (nof_entries_db[other_bank_ndx] * nof_entries_free) 
                / (nof_entries_free + nof_entries_db[0] + nof_entries_db[1]);
        }
        /* Finish if no entries have to be moved */
        if ((nof_entries_to_move[0] == 0) && (nof_entries_to_move[1] == 0)) {
            ARAD_DO_NOTHING_AND_EXIT;
        }

        /* Every jump lines, write a new moved entry (max-min number). Will be at least 1 */
        nof_lines_jump = nof_entries_free / (nof_entries_to_move[0] + nof_entries_to_move[1]); 
        if ((nof_lines_jump == 0) || (((nof_entries_to_move[0] + nof_entries_to_move[1]) * nof_lines_jump) > nof_entries_free)){
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit,
                                  "Unit %d Tcam DB id %d. incorrect numbers: number-of-lines-jump %d, nof_entries_free %d.\n\r"),
                       unit, tcam_db_id, nof_lines_jump, nof_entries_free));
            SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 70, exit);
        }

        /* 
         * Find the first entry to write according to the ratio: 
         * - if no entry to move before / after, the ratio line should be the first / last one 
         * - there will always be a line spare - it should be the ratio line
         */
        res = arad_tcam_jump_line_find(
                unit,
                bank_id,
                entry_size,
                ARAD_TCAM_DIRECTION_FORWARD,
                0, /* start */
                arad_tcam_bank_entry_size_to_entry_count_get(unit, entry_size, bank_id) - 1, /* end */
                ((nof_entries_to_move[0] * nof_entries_free) / (nof_entries_to_move[0] + nof_entries_to_move[1])),
                &line_ratio,
                &line_found
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
        if (!line_found) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit,
                                  "Unit %d Tcam DB id %d. move not found: number-of-lines-jump %d, nof_entries_free %d.\n\r"),
                       unit, tcam_db_id, nof_lines_jump, nof_entries_free));
            SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 90, exit);
        }


        /* Move the entries without interfering with traffic */
        for (other_bank_ndx = 0; other_bank_ndx < ARAD_TCAM_BANK_INIT_MOVE_ENTRIES_NOF_ADJACENT_BANKS; ++other_bank_ndx)
        {
            start = (other_bank_ndx == 0)? 0: line_ratio;
            end = (other_bank_ndx == 0)? line_ratio: arad_tcam_bank_entry_size_to_entry_count_get(unit, entry_size, bank_id);

            /* Get the closest entry */
            ARAD_TCAM_LOCATION_clear(&empty_location);
            empty_location.bank_id = bank_id;
            empty_location.entry = line_ratio; /* The first loop does not write to this line */
            direction = ((other_bank_ndx == 0)? ARAD_TCAM_DIRECTION_BACKWARD: ARAD_TCAM_DIRECTION_FORWARD);
            res = arad_tcam_db_occ_bmp_entry_closest_get(
                      unit,
                      tcam_db_id,
                      direction,
                      &empty_location,
                      &entry_id,
                      &next_priority,
                      &next_location,
                      &found
                    );
            SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

            for (entry_idx = 0; entry_idx < nof_entries_to_move[other_bank_ndx]; ++entry_idx)
            {
                /* Get the line to set */
                res = arad_tcam_jump_line_find(
                        unit,
                        bank_id,
                        entry_size,
                        ((other_bank_ndx == 0)? ARAD_TCAM_DIRECTION_BACKWARD: ARAD_TCAM_DIRECTION_FORWARD),
                        start,
                        end,
                        nof_lines_jump,
                        &line_idx,
                        &line_found
                      );
                SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);
                if (!line_found) {
                    LOG_ERROR(BSL_LS_SOC_TCAM,
                              (BSL_META_U(unit,
                                          "Unit %d Tcam DB id %d. move not found: number-of-lines-jump %d, nof_entries_free %d.\n\r"),
                               unit, tcam_db_id, nof_lines_jump, nof_entries_free));
                    SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 90, exit);
                }

                /* Move the entry */
                empty_location.bank_id = bank_id;
                empty_location.entry = line_idx; /* The first loop does not write to this line */
                res = arad_tcam_db_entry_move(
                        unit,
                        TRUE, /* set_hw_also */
                        FALSE, /* is_bank_share_update */
                        &next_location,
                        &empty_location
                      );
                SOC_SAND_CHECK_FUNC_RESULT(res, 120, exit);

                /* Get next entry */
                curr_location = next_location;
                res = arad_tcam_db_occ_bmp_entry_closest_get(
                          unit,
                          tcam_db_id,
                          direction,
                          &curr_location,
                          &entry_id,
                          &next_priority,
                          &next_location,
                          &found
                        );
                SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
                if (!found) {
                    LOG_ERROR(BSL_LS_SOC_TCAM,
                              (BSL_META_U(unit,
                                          "Unit %d Tcam DB id %d. entry move not found from location bank %d entry %d and direction %d.\n\r"),
                               unit, tcam_db_id, curr_location.bank_id, curr_location.entry, direction));
                    SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_METADATA_INCONSISTENT_ERR, 102, exit);
                }

                /* Update the range to look for */
                start = (other_bank_ndx == 0)? 0: line_idx;
                end = (other_bank_ndx == 0)? line_idx: arad_tcam_bank_entry_size_to_entry_count_get(unit, entry_size, bank_id);
            }
        }
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_access_profile_bank_add()", 0, tcam_db_id);
}
#endif /* ARAD_TCAM_BANK_INIT_MOVE_ENTRIES_ENABLE */



void
  ARAD_TCAM_PREFIX_clear(
    SOC_SAND_OUT ARAD_TCAM_PREFIX *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(ARAD_TCAM_PREFIX));
  info->bits = 0;
  info->length = 0;
  SOC_SAND_MAGIC_NUM_SET;
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_TCAM_LOCATION_clear(
    SOC_SAND_OUT ARAD_TCAM_LOCATION *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(ARAD_TCAM_LOCATION));
  info->bank_id = 0;
  info->entry = 0;
  SOC_SAND_MAGIC_NUM_SET;
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_TCAM_GLOBAL_LOCATION_clear(
    SOC_SAND_OUT ARAD_TCAM_GLOBAL_LOCATION *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(ARAD_TCAM_GLOBAL_LOCATION));
  info->tcam_db_id = ARAD_TCAM_DB_ID_INVALID;
  info->entry_id = 0;
  info->priority = 0;

exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_TCAM_PRIO_LOCATION_clear(
    SOC_SAND_OUT ARAD_TCAM_PRIO_LOCATION *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(ARAD_TCAM_PRIO_LOCATION));
  info->entry_id_first = 0;
  info->entry_id_last = 0;

exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_TCAM_RANGE_clear(
    SOC_SAND_OUT ARAD_TCAM_RANGE *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(ARAD_TCAM_RANGE));
  ARAD_TCAM_LOCATION_clear(&(info->min));
  ARAD_TCAM_LOCATION_clear(&(info->max));
  SOC_SAND_MAGIC_NUM_SET;
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_TCAM_ENTRY_clear(
    SOC_SAND_OUT ARAD_TCAM_ENTRY *info
  )
{
  uint32
    ind;

  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(ARAD_TCAM_ENTRY));
  for (ind = 0; ind < ARAD_TCAM_ENTRY_MAX_LEN; ++ind)
  {
    info->value[ind] = 0;
  }
  for (ind = 0; ind < ARAD_TCAM_ENTRY_MAX_LEN; ++ind)
  {
    info->mask[ind] = 0;
  }
  info->is_for_update = FALSE;
  info->is_inserted_top = FALSE;

  SOC_SAND_MAGIC_NUM_SET;
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_TCAM_ACTION_clear(
    SOC_SAND_OUT ARAD_TCAM_ACTION *info
  )
{
  uint32
    ind;

  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  sal_memset(info, 0x0, sizeof(ARAD_TCAM_ACTION));
  for (ind = 0; ind < ARAD_TCAM_ACTION_MAX_LEN; ++ind)
  {
    info->value[ind] = 0;
  }
  
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  for (ind = 0; ind < SOC_DPP_TCAM_ACTION_ELK_KBP_MAX_LEN_BYTES; ++ind)
  {
    info->elk_ad_value[ind] = 0;
  }
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */

  SOC_SAND_MAGIC_NUM_SET;
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

uint32
  ARAD_TCAM_PREFIX_verify(
    SOC_SAND_IN  ARAD_TCAM_PREFIX *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->bits, ARAD_TCAM_BITS_MAX, ARAD_TCAM_BITS_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->length, ARAD_TCAM_LENGTH_MAX, ARAD_TCAM_LENGTH_OUT_OF_RANGE_ERR, 11, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in ARAD_TCAM_PREFIX_verify()",0,0);
}

uint32
  ARAD_TCAM_LOCATION_verify(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  ARAD_TCAM_LOCATION *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_NOF(info->bank_id, SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit), ARAD_TCAM_BANK_ID_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->entry, SOC_DPP_DEFS_TCAM_ENTRY_MAX(unit), ARAD_TCAM_ENTRY_OUT_OF_RANGE_ERR, 11, exit);

/*  SOC_SAND_MAGIC_NUM_VERIFY(info); */

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in ARAD_TCAM_LOCATION_verify()",0,0);
}

uint32
  ARAD_TCAM_GLOBAL_LOCATION_verify(
    SOC_SAND_IN  int     unit,
    SOC_SAND_IN  ARAD_TCAM_GLOBAL_LOCATION *info
  )
{
    uint32
        tcam_db_id = info->tcam_db_id;
#ifdef BROADCOM_DEBUG
    uint32
        res;
#endif /* BROADCOM_DEBUG */

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(tcam_db_id, ARAD_TCAM_TCAM_DB_ID_MAX, ARAD_TCAM_BANK_ID_OUT_OF_RANGE_ERR, 10, exit);
  /* Verify the Database exists */
  if (tcam_db_id != ARAD_TCAM_DB_ID_INVALID) {
      ARAD_TCAM_DB_EXISTS_ASSERT;

      /* Verify the entry exists */
      ARAD_TCAM_DB_ENTRY_EXISTS_ASSERT(info->entry_id);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in ARAD_TCAM_GLOBAL_LOCATION_verify()",0,0);
}

uint32
  ARAD_TCAM_PRIO_LOCATION_verify(
    SOC_SAND_IN  int     unit,
    SOC_SAND_IN  uint32     tcam_db_id,
    SOC_SAND_IN  ARAD_TCAM_PRIO_LOCATION *info
  )
{
#ifdef BROADCOM_DEBUG
    uint32
        res;
#endif /* BROADCOM_DEBUG */

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  /* Verify the entry exists */
  ARAD_TCAM_DB_ENTRY_EXISTS_ASSERT(info->entry_id_first);

  /* Verify the entry exists */
  ARAD_TCAM_DB_ENTRY_EXISTS_ASSERT(info->entry_id_last);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in ARAD_TCAM_PRIO_LOCATION_verify()",0,0);
}

uint32
  ARAD_TCAM_RANGE_verify(
    SOC_SAND_IN  int             unit,
    SOC_SAND_IN  ARAD_TCAM_RANGE *info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  res = ARAD_TCAM_LOCATION_verify(unit, &(info->min));
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  res = ARAD_TCAM_LOCATION_verify(unit, &(info->max));
  SOC_SAND_CHECK_FUNC_RESULT(res, 11, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in ARAD_TCAM_RANGE_verify()",0,0);
}

uint32
  ARAD_TCAM_ENTRY_verify(
    SOC_SAND_IN  ARAD_TCAM_ENTRY *info
  )
{
  uint32
    ind;

  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  for (ind = 0; ind < ARAD_TCAM_ENTRY_MAX_LEN; ++ind)
  {
  }
  for (ind = 0; ind < ARAD_TCAM_ENTRY_MAX_LEN; ++ind)
  {
  }

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in ARAD_TCAM_ENTRY_verify()",0,0);
}

uint32
  ARAD_TCAM_ACTION_verify(
    SOC_SAND_IN  ARAD_TCAM_ACTION *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in ARAD_TCAM_ENTRY_verify()",0,0);
}

#if ARAD_DEBUG_IS_LVL1

const char*
  ARAD_TCAM_BANK_ENTRY_SIZE_to_string(
    SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE enum_val
  )
{
  return SOC_TMC_TCAM_BANK_ENTRY_SIZE_to_string(enum_val);
}

void
  ARAD_TCAM_PREFIX_print(
    SOC_SAND_IN  ARAD_TCAM_PREFIX *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "bits: %u\n\r"),info->bits));
  LOG_CLI((BSL_META_U(unit,
                      "length: %u\n\r"),info->length));
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_TCAM_LOCATION_print(
    SOC_SAND_IN  ARAD_TCAM_LOCATION *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "bank_id: %u\n\r"),info->bank_id));
  LOG_CLI((BSL_META_U(unit,
                      "entry: %u\n\r"),info->entry));
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_TCAM_GLOBAL_LOCATION_print(
    SOC_SAND_IN  ARAD_TCAM_GLOBAL_LOCATION *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "tcam_db_id: %u, "),info->tcam_db_id));
  LOG_CLI((BSL_META_U(unit,
                      "entry_id: %u, "),info->entry_id));
  LOG_CLI((BSL_META_U(unit,
                      "priority: %u\n\r"),info->priority));
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_TCAM_RANGE_print(
    SOC_SAND_IN  ARAD_TCAM_RANGE *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "min:")));
  ARAD_TCAM_LOCATION_print(&(info->min));
  LOG_CLI((BSL_META_U(unit,
                      "max:")));
  ARAD_TCAM_LOCATION_print(&(info->max));
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_TCAM_ENTRY_print(
    SOC_SAND_IN  ARAD_TCAM_ENTRY *info
  )
{
  uint32
    ind;

  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  for (ind = 0; ind < ARAD_TCAM_ENTRY_MAX_LEN; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "value[%u]: %u\n\r"),ind,info->value[ind]));
  }
  for (ind = 0; ind < ARAD_TCAM_ENTRY_MAX_LEN; ++ind)
  {
    LOG_CLI((BSL_META_U(unit,
                        "mask[%u]: %u\n\r"),ind,info->mask[ind]));
  }
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#endif /* ARAD_DEBUG_IS_LVL1 */

uint32
  arad_tcam_resource_db_entries_find(
      SOC_SAND_IN  int     unit,
      SOC_SAND_IN  uint32     tcam_db_id,
      SOC_SAND_OUT uint32     *db_nof_entries
    )
{
  uint32
      next_entry,
      next_priority,
    res = SOC_SAND_OK;
  ARAD_TCAM_LOCATION 
      location_curr,
    location;
  uint8
      found,
      is_occupied,
      is_used;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(db_nof_entries);

  ARAD_TCAM_DB_EXISTS_ASSERT;

  sal_memset(db_nof_entries, 0x0, (sizeof(uint32) * SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit)));

  /* Start from the first entry - get the closest existing entry */
  ARAD_TCAM_LOCATION_clear(&location_curr);
  location_curr.bank_id = 0;
  location_curr.entry = 0;

  /* Special case for bank 0 entry 0 */
  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(unit, tcam_db_id, location_curr.bank_id, &is_used);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit); 
  if(is_used){
      res = arad_tcam_db_bank_occ_bitmap_entry_get(
                unit,
                tcam_db_id,
                &location_curr,
                &is_occupied
                );
      SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);
      if (is_occupied) {
          db_nof_entries[location_curr.bank_id]++;
      }
  }

  /* Continue check for the rest of the banks/entries */
  do
  {
      res = arad_tcam_db_occ_bmp_entry_closest_get(
                unit,
                tcam_db_id,
                ARAD_TCAM_DIRECTION_FORWARD,
                &location_curr,
                &next_entry,
                &next_priority,
                &location,
                &found
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    if(found)
    {
      db_nof_entries[location.bank_id]++;
      location_curr = location;
    }
  } while (found);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_resource_db_entries_find", tcam_db_id, 0);
}

uint32
  arad_tcam_resource_db_entries_validate(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  uint32 bank_id,
    SOC_SAND_IN  ARAD_TCAM_BANK_ENTRY_SIZE entry_size,
	  SOC_SAND_IN  uint32 address,
    SOC_SAND_OUT ARAD_TCAM_ENTRY *entry
  )
{
	uint32 res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(entry);

  res = arad_tcam_tbl_bank_read(
          unit,
          bank_id,
          entry_size,
          address,
          entry
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_resource_db_entries_validate", 0, 0);
}

uint32
  arad_tcam_resource_db_entries_priority_validate(
      SOC_SAND_IN  int        unit,
      SOC_SAND_IN  uint32        tcam_db_id,
      SOC_SAND_IN  ARAD_TCAM_LOCATION *location,
      SOC_SAND_OUT uint32        *entry_id,
      SOC_SAND_OUT uint32         *priority
  )
{
    uint32 res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(location);
  SOC_SAND_CHECK_NULL_INPUT(entry_id);
  SOC_SAND_CHECK_NULL_INPUT(priority);

  res = arad_tcam_db_priority_list_entry_get(
          unit,
          tcam_db_id,
          location,
          entry_id,
          priority
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_resource_db_entries_priority_validate", 0, 0);
}

uint32
  arad_tcam_entry_rewrite(
      SOC_SAND_IN       int                          unit,
      SOC_SAND_IN       uint8                        entry_exists,
      SOC_SAND_IN       uint32                       tcam_db_id,
      SOC_SAND_IN       ARAD_TCAM_LOCATION           *location,
      SOC_SAND_IN       ARAD_TCAM_BANK_ENTRY_SIZE    entry_size,
      SOC_SAND_INOUT    ARAD_TCAM_ENTRY              *entry
  )
{
    uint32 res = SOC_SAND_OK;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    SOC_SAND_CHECK_NULL_INPUT(location);
    SOC_SAND_CHECK_NULL_INPUT(entry);

    ARAD_STRUCT_VERIFY(ARAD_TCAM_ENTRY, entry, 40, exit);

    if (entry_exists)
    {
        /* tcam_db_id is valid only if entry exists */
        res = arad_tcam_db_entry_prefix_stamp(unit, tcam_db_id, entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
    }

    /* write given value and mask to location */
    res = arad_tcam_tbl_bank_write(unit, location->bank_id, entry_size, location->entry, entry);
    SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_tcam_entry_rewrite", 0, 0);
}


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

#undef _ERR_MSG_MODULE_NAME

#endif /* of #if defined(BCM_88650_A0) */


