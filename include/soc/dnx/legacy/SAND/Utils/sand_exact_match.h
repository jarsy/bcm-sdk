/* $Id: dnx_sand_exact_match.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifndef DNX_SAND_EXACT_MATCH_H_INCLUDED
/* { */
#define DNX_SAND_EXACT_MATCH_H_INCLUDED

/*************
* INCLUDES  *
*************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
/* } */

/*************
 * DEFINES   *
 *************/
/* { */
#define DNX_SAND_EXACT_MATCH_NOF_TABLES 2

#define DNX_SAND_EXACT_MATCH_NULL  DNX_SAND_U32_MAX
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
  DNX_SAND_EXACT_MATCH_USE_AC=0,
  DNX_SAND_EXACT_MATCH_USE_OAM,
  DNX_SAND_EXACT_MATCH_NOF_USES
}DNX_SAND_EXACT_MATCH_USE;

typedef uint8* DNX_SAND_EXACT_MATCH_KEY;
typedef uint8* DNX_SAND_EXACT_MATCH_PAYLOAD;
typedef uint32 DNX_SAND_EXACT_MATCH_HASH_VAL;
typedef uint8* DNX_SAND_EXACT_MATCH_VERIFIER;
typedef uint32 DNX_SAND_EXACT_MATCH_ITER;

typedef
  uint32
    (*DNX_SAND_EXACT_MATCH_SW_DB_ENTRY_SET)(
      DNX_SAND_IN  int                   prime_handle,
      DNX_SAND_IN  uint32                   sec_handle,
      DNX_SAND_INOUT  uint8                 *buffer,
      DNX_SAND_IN  uint32                   offset,
      DNX_SAND_IN  uint32                   len,
      DNX_SAND_IN uint8                     *data
    );

typedef
  uint32
    (*DNX_SAND_EXACT_MATCH_SW_DB_ENTRY_GET)(
      DNX_SAND_IN  int                   prime_handle,
      DNX_SAND_IN  uint32                   sec_handle,
      DNX_SAND_IN  uint8                    *buffer,
      DNX_SAND_IN  uint32                   offset,
      DNX_SAND_IN  uint32                   len,
      DNX_SAND_OUT uint8                    *data
    );

typedef
  uint32
    (*DNX_SAND_EXACT_MATCH_HW_ENTRY_SET)(
      DNX_SAND_IN  int                   prime_handle,
      DNX_SAND_IN  uint32                   tbl_ndx,
      DNX_SAND_IN  uint32                   entry_offset,
      DNX_SAND_IN DNX_SAND_EXACT_MATCH_PAYLOAD        payload, 
      DNX_SAND_IN DNX_SAND_EXACT_MATCH_VERIFIER       verifier
    );

typedef
  uint32
    (*DNX_SAND_EXACT_MATCH_HW_ENTRY_GET)(
      DNX_SAND_IN  int                   prime_handle,
      DNX_SAND_IN  uint32                   tbl_ndx,
      DNX_SAND_IN  uint32                   entry_offset,
      DNX_SAND_OUT DNX_SAND_EXACT_MATCH_PAYLOAD        payload, 
      DNX_SAND_OUT DNX_SAND_EXACT_MATCH_VERIFIER       verifier
    );

typedef
  uint32
    (*DNX_SAND_EXACT_MATCH_IS_ENTRY_VALID_SET)(
      DNX_SAND_IN  int                   prime_handle,
      DNX_SAND_IN  uint32                   sec_handle,
      DNX_SAND_IN  uint32                   tbl_ndx,
      DNX_SAND_IN  uint32                   entry_offset,
      DNX_SAND_IN  uint8                  is_valid
    );

typedef
  uint32
    (*DNX_SAND_EXACT_MATCH_IS_ENTRY_VALID_GET)(
      DNX_SAND_IN  int                   prime_handle,
      DNX_SAND_IN  uint32                   sec_handle,
      DNX_SAND_IN  uint32                   tbl_ndx,
      DNX_SAND_IN  uint32                   entry_offset,
      DNX_SAND_OUT uint8                  *is_valid
    );

typedef
  DNX_SAND_EXACT_MATCH_HASH_VAL
    (*DNX_SAND_EXACT_MATCH_KEY_TO_HASH)(
      DNX_SAND_IN DNX_SAND_EXACT_MATCH_KEY key,
      DNX_SAND_IN uint32 table_id
    );

typedef
  void
    (*DNX_SAND_EXACT_MATCH_KEY_TO_VERIFY)(
      DNX_SAND_IN   DNX_SAND_EXACT_MATCH_KEY key,
      DNX_SAND_IN   uint32 table_ndx,
      DNX_SAND_OUT  DNX_SAND_EXACT_MATCH_VERIFIER verifier
    );


typedef struct {
  DNX_SAND_MAGIC_NUM_VAR
  DNX_SAND_EXACT_MATCH_KEY  key;
  DNX_SAND_EXACT_MATCH_PAYLOAD payload; 
  DNX_SAND_EXACT_MATCH_VERIFIER verifier;
} DNX_SAND_EXACT_MATCH_ENTRY;

typedef struct {

  DNX_SAND_EXACT_MATCH_KEY  key; 
  DNX_SAND_EXACT_MATCH_HASH_VAL  hash_indx;
} DNX_SAND_EXACT_MATCH_STACK_ENTRY;

typedef struct {
  int prime_handle;
  uint32 sec_handle;
  uint32 max_insert_steps;
  uint32 key_bits;
  uint32 hash_bits;
  uint32 verifier_bits;

  DNX_SAND_EXACT_MATCH_SW_DB_ENTRY_SET set_entry_fun;
  DNX_SAND_EXACT_MATCH_SW_DB_ENTRY_GET get_entry_fun;
  DNX_SAND_EXACT_MATCH_HW_ENTRY_SET  hw_set_fun;
  DNX_SAND_EXACT_MATCH_HW_ENTRY_GET  hw_get_fun;
  DNX_SAND_EXACT_MATCH_IS_ENTRY_VALID_SET  is_valid_entry_set;
  DNX_SAND_EXACT_MATCH_IS_ENTRY_VALID_GET  is_valid_entry_get;

  DNX_SAND_EXACT_MATCH_HASH_VAL (*key_to_hash)(DNX_SAND_IN DNX_SAND_EXACT_MATCH_KEY key, DNX_SAND_IN uint32 table_id); /*get hash0 or hash1*/
  void (*key_to_verifier)(DNX_SAND_IN DNX_SAND_EXACT_MATCH_KEY key, DNX_SAND_IN uint32 table_ndx, DNX_SAND_OUT DNX_SAND_EXACT_MATCH_VERIFIER verifier); /*get verifier0 or verifier1*/

} DNX_SAND_EXACT_MATCH_INIT_INFO;

typedef struct
{
  DNX_SAND_EXACT_MATCH_KEY  keys[DNX_SAND_EXACT_MATCH_NOF_TABLES]; 
  uint32 *use_bitmap[DNX_SAND_EXACT_MATCH_NOF_TABLES];
  DNX_SAND_EXACT_MATCH_STACK_ENTRY *stack[DNX_SAND_EXACT_MATCH_NOF_TABLES];
  DNX_SAND_EXACT_MATCH_ENTRY *tmp_entry;
  uint32 table_size;
  uint32 key_size;
  uint32 payload_size;
  uint32  verifier_size;

} DNX_SAND_EXACT_MATCH_T;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR

  DNX_SAND_EXACT_MATCH_INIT_INFO
    init_info;

  DNX_SAND_EXACT_MATCH_T
    mgmt_info;
} DNX_SAND_EXACT_MATCH_INFO;

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
  dnx_sand_exact_match_create(
    DNX_SAND_INOUT  DNX_SAND_EXACT_MATCH_INFO     *exact_match
  );

uint32
  dnx_sand_exact_match_destroy(
    DNX_SAND_INOUT  DNX_SAND_EXACT_MATCH_INFO     *exact_match
  );

uint32
  dnx_sand_exact_match_entry_add(
    DNX_SAND_INOUT  DNX_SAND_EXACT_MATCH_INFO     *exact_match,
    DNX_SAND_IN  DNX_SAND_EXACT_MATCH_KEY         key,
    DNX_SAND_IN  DNX_SAND_EXACT_MATCH_PAYLOAD     payload,
    DNX_SAND_OUT uint32                     *tbl_id,
    DNX_SAND_OUT uint32                     *entry_indx,
    DNX_SAND_OUT  uint32                    *nof_steps,
    DNX_SAND_OUT  uint8                   *success
  );

uint32
  dnx_sand_exact_match_entry_remove(
    DNX_SAND_INOUT  DNX_SAND_EXACT_MATCH_INFO     *exact_match,
    DNX_SAND_IN  DNX_SAND_EXACT_MATCH_KEY         key
  );

uint32
  dnx_sand_exact_match_entry_lookup(
    DNX_SAND_INOUT  DNX_SAND_EXACT_MATCH_INFO     *exact_match,
    DNX_SAND_IN  DNX_SAND_EXACT_MATCH_KEY         key,
    DNX_SAND_OUT  uint32                    *tbl_id,
    DNX_SAND_OUT  uint32                    *entry_indx,
    DNX_SAND_OUT  DNX_SAND_EXACT_MATCH_PAYLOAD    payload,
    DNX_SAND_OUT  uint8                   *found
  );

uint32
  dnx_sand_exact_match_clear(
    DNX_SAND_INOUT  DNX_SAND_EXACT_MATCH_INFO     *exact_match
  );

uint32
  dnx_sand_exact_match_get_block(
    DNX_SAND_INOUT  DNX_SAND_EXACT_MATCH_INFO     *exact_match,
    DNX_SAND_INOUT  DNX_SAND_TABLE_BLOCK_RANGE    *range,
    DNX_SAND_OUT  DNX_SAND_EXACT_MATCH_KEY        keys,
    DNX_SAND_OUT  uint32                    *nof_entries
  );

uint32
  dnx_sand_exact_match_get_size_for_save(
    DNX_SAND_IN  DNX_SAND_EXACT_MATCH_INFO        *exact_match,
    DNX_SAND_OUT  uint32                    *size
  );

uint32
  dnx_sand_exact_match_save(
    DNX_SAND_IN  DNX_SAND_EXACT_MATCH_INFO        *exact_match,
    DNX_SAND_OUT uint8                      *buffer,
    DNX_SAND_IN  uint32                     buffer_size_bytes,
    DNX_SAND_OUT uint32                     *actual_size_bytes
  );

uint32
  dnx_sand_exact_match_load(
    DNX_SAND_IN  uint8                              **buffer,
    DNX_SAND_OUT DNX_SAND_EXACT_MATCH_INFO                *exact_match,
    DNX_SAND_IN  DNX_SAND_EXACT_MATCH_KEY_TO_HASH         key_to_hash,
    DNX_SAND_IN  DNX_SAND_EXACT_MATCH_KEY_TO_VERIFY       key_to_verify,
    DNX_SAND_IN  DNX_SAND_EXACT_MATCH_SW_DB_ENTRY_SET     set_entry_fun,
    DNX_SAND_IN  DNX_SAND_EXACT_MATCH_SW_DB_ENTRY_GET     get_entry_fun,
    DNX_SAND_IN  DNX_SAND_EXACT_MATCH_HW_ENTRY_SET        hw_set_fun,
    DNX_SAND_IN  DNX_SAND_EXACT_MATCH_HW_ENTRY_GET        hw_get_fun,
    DNX_SAND_IN  DNX_SAND_EXACT_MATCH_IS_ENTRY_VALID_SET  is_valid_entry_set,
    DNX_SAND_IN  DNX_SAND_EXACT_MATCH_IS_ENTRY_VALID_GET  is_valid_entry_get
  );

void
  DNX_SAND_EXACT_MATCH_INFO_clear(
    DNX_SAND_OUT DNX_SAND_EXACT_MATCH_INFO        *info
  );

#ifdef DNX_SAND_DEBUG

void
  dnx_sand_SAND_EXACT_MATCH_INFO_print(
    DNX_SAND_IN DNX_SAND_EXACT_MATCH_INFO         *info
  );

#endif /* DNX_SAND_DEBUG */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } DNX_SAND_EXACT_MATCH_H_INCLUDED*/
#endif

