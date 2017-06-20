/* $Id: arad_pmf_low_level_db.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PMF_LOW_LEVEL_DB_INCLUDED__
/* { */
#define __ARAD_PMF_LOW_LEVEL_DB_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/arad_pmf_low_level.h>
#include <soc/dpp/ARAD/arad_api_framework.h>

#include <soc/dpp/TMC/tmc_api_pmf_low_level_db.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PMF_DIRECT_TBL_KEY_SRC_A_9_0                 SOC_TMC_PMF_DIRECT_TBL_KEY_SRC_A_9_0
#define ARAD_PMF_DIRECT_TBL_KEY_SRC_A_19_10               SOC_TMC_PMF_DIRECT_TBL_KEY_SRC_A_19_10
#define ARAD_PMF_DIRECT_TBL_KEY_SRC_A_29_20               SOC_TMC_PMF_DIRECT_TBL_KEY_SRC_A_29_20
#define ARAD_PMF_DIRECT_TBL_KEY_SRC_A_39_30               SOC_TMC_PMF_DIRECT_TBL_KEY_SRC_A_39_30
#define ARAD_PMF_DIRECT_TBL_KEY_SRC_B_9_0                 SOC_TMC_PMF_DIRECT_TBL_KEY_SRC_B_9_0
#define ARAD_PMF_DIRECT_TBL_KEY_SRC_B_19_10               SOC_TMC_PMF_DIRECT_TBL_KEY_SRC_B_19_10
#define ARAD_PMF_DIRECT_TBL_KEY_SRC_B_29_20               SOC_TMC_PMF_DIRECT_TBL_KEY_SRC_B_29_20
#define ARAD_PMF_DIRECT_TBL_KEY_SRC_B_39_30               SOC_TMC_PMF_DIRECT_TBL_KEY_SRC_B_39_30
#define ARAD_NOF_PMF_DIRECT_TBL_KEY_SRCS                  SOC_TMC_NOF_PMF_DIRECT_TBL_KEY_SRCS
typedef SOC_TMC_PMF_DIRECT_TBL_KEY_SRC                         ARAD_PMF_DIRECT_TBL_KEY_SRC;



typedef SOC_TMC_PMF_TCAM_ENTRY_ID                              ARAD_PMF_TCAM_ENTRY_ID;
typedef SOC_TMC_PMF_TCAM_DATA                                  ARAD_PMF_TCAM_DATA;
typedef SOC_TMC_PMF_LKP_PROFILE                                ARAD_PMF_LKP_PROFILE;
typedef SOC_TMC_PMF_TCAM_KEY_SELECTION_INFO                    ARAD_PMF_TCAM_KEY_SELECTION_INFO;
typedef SOC_TMC_PMF_TCAM_BANK_SELECTION                        ARAD_PMF_TCAM_BANK_SELECTION;
typedef SOC_TMC_PMF_TCAM_RESULT_INFO                           ARAD_PMF_TCAM_RESULT_INFO;
typedef SOC_TMC_PMF_DIRECT_TBL_DATA                            ARAD_PMF_DIRECT_TBL_DATA;


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
  arad_pmf_low_level_db_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

uint32
  arad_pmf_low_level_tcam_bank_enable_set_unsafe(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint32 bank_id,
    SOC_SAND_IN uint32 cycle,
    SOC_SAND_IN uint8 enabled
  );

/*********************************************************************
* NAME:
 *   arad_pmf_tcam_entry_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add an entry (in the format of a PMF TCAM Key) in a TCAM
 *   Bank.
 * INPUT:
 *   SOC_SAND_IN  int                            unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_PMF_TCAM_ENTRY_ID              *entry_ndx -
 *     TCAM Entry-ID.
 *   SOC_SAND_IN  ARAD_PMF_TCAM_DATA                  *tcam_data -
 *     TCAM Data: entry priority, key with raw bitmap
 *     presentation and TCAM output value. Helper functions are
 *     provided in the PMF TCAM module to build the key per
 *     field and the action (TCAM output) according to a
 *     template.
 *   SOC_SAND_IN  uint8                            allow_new_bank -
 *     If True, then the entry is added to the Database even if
 *     the Database must use another TCAM (Bank, Cycle) than it
 *     already uses. Otherwise, the entry is added only if the
 *     Database has already entries in this TCAM (Bank, Cycle).
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                 *success -
 *     Indicate the entry insertion into the Database has
 *     succeeded.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pmf_tcam_entry_add_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_TCAM_ENTRY_ID              *entry_ndx,
    SOC_SAND_IN  ARAD_PMF_TCAM_DATA                  *tcam_data,
    SOC_SAND_IN  uint8                            allow_new_bank,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                 *success
  );

uint32
  arad_pmf_tcam_entry_add_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_TCAM_ENTRY_ID              *entry_ndx,
    SOC_SAND_IN  ARAD_PMF_TCAM_DATA                  *tcam_data,
    SOC_SAND_IN  uint8                            allow_new_bank
  );

/*********************************************************************
* NAME:
 *   arad_pmf_tcam_entry_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get an entry (in the format of a PMF TCAM Key) in a TCAM
 *   Bank.
 * INPUT:
 *   SOC_SAND_IN  int                            unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_PMF_TCAM_ENTRY_ID              *entry_ndx -
 *     TCAM Entry-ID.
 *   SOC_SAND_OUT ARAD_PMF_TCAM_DATA                  *tcam_data -
 *     TCAM Data: entry priority, key with raw bitmap
 *     presentation and TCAM output value.
 *   SOC_SAND_OUT uint8                            *is_found -
 *     If True, then the entry is found in the Database.
 *     Otherwise, no such entry is present in this Database.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pmf_tcam_entry_get_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_TCAM_ENTRY_ID              *entry_ndx,
    SOC_SAND_OUT ARAD_PMF_TCAM_DATA                  *tcam_data,
    SOC_SAND_OUT uint8                            *is_found
  );

uint32
  arad_pmf_tcam_entry_get_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_TCAM_ENTRY_ID              *entry_ndx
  );

/*********************************************************************
* NAME:
 *   arad_pmf_tcam_entry_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove an entry (in the format of a PMF TCAM Key) from a
 *   TCAM.
 * INPUT:
 *   SOC_SAND_IN  int                            unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_PMF_TCAM_ENTRY_ID              *entry_ndx -
 *     TCAM Entry-ID.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pmf_tcam_entry_remove_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  ARAD_PMF_TCAM_ENTRY_ID              *entry_ndx,
    SOC_SAND_OUT uint8                         *is_found
  );

uint32
  arad_pmf_tcam_entry_remove_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_TCAM_ENTRY_ID              *entry_ndx
  );


/*********************************************************************
* NAME:
 *   arad_pmf_db_direct_tbl_key_src_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Select the key source for the direct table.
 * INPUT:
 *   SOC_SAND_IN  int                            unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_PMF_LKP_PROFILE                *lkp_profile_ndx -
 *     Lookup-Profile information (id and cycle).
 *   SOC_SAND_IN  ARAD_PMF_DIRECT_TBL_KEY_SRC         key_src -
 *     Entry index source for the direct table
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pmf_db_direct_tbl_key_src_set_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_LKP_PROFILE                *lkp_profile_ndx,
    SOC_SAND_IN  ARAD_PMF_DIRECT_TBL_KEY_SRC         key_src
  );

uint32
  arad_pmf_db_direct_tbl_key_src_set_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_LKP_PROFILE                *lkp_profile_ndx,
    SOC_SAND_IN  ARAD_PMF_DIRECT_TBL_KEY_SRC         key_src
  );

uint32
  arad_pmf_db_direct_tbl_key_src_get_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_LKP_PROFILE                *lkp_profile_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pmf_db_direct_tbl_key_src_set_unsafe" API.
 *     Refer to "arad_pmf_db_direct_tbl_key_src_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pmf_db_direct_tbl_key_src_get_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_LKP_PROFILE                *lkp_profile_ndx,
    SOC_SAND_OUT ARAD_PMF_DIRECT_TBL_KEY_SRC         *key_src
  );

/*********************************************************************
* NAME:
 *   arad_pmf_db_direct_tbl_entry_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set an entry to the Database direct table.
 * INPUT:
 *   SOC_SAND_IN  int                            unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                            entry_ndx -
 *     Entry index in the Direct table. Range: 0 - 1023.
 *     (Arad-B)
 *   SOC_SAND_IN  ARAD_PMF_DIRECT_TBL_DATA            *data -
 *     Direct table output value. For Arad-B, it is encoded in
 *     20 bits. Helper functions are provided in the PMF
 *     Database module to build this output per template.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pmf_db_direct_tbl_entry_set_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32                            entry_ndx,
    SOC_SAND_IN  ARAD_PMF_DIRECT_TBL_DATA            *data
  );

uint32
  arad_pmf_db_direct_tbl_entry_set_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32                            entry_ndx,
    SOC_SAND_IN  ARAD_PMF_DIRECT_TBL_DATA            *data
  );

uint32
  arad_pmf_db_direct_tbl_entry_get_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32                            entry_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pmf_db_direct_tbl_entry_set_unsafe" API.
 *     Refer to "arad_pmf_db_direct_tbl_entry_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pmf_db_direct_tbl_entry_get_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32                            entry_ndx,
    SOC_SAND_OUT ARAD_PMF_DIRECT_TBL_DATA            *data
  );


uint32
  ARAD_PMF_TCAM_ENTRY_ID_verify(
    SOC_SAND_IN  int             unit,
    SOC_SAND_IN  ARAD_PMF_TCAM_ENTRY_ID *info
  );

uint32
  ARAD_PMF_LKP_PROFILE_verify(
    SOC_SAND_IN  ARAD_PMF_LKP_PROFILE *info
  );

uint32
  ARAD_PMF_TCAM_KEY_SELECTION_INFO_verify(
    SOC_SAND_IN  ARAD_PMF_TCAM_KEY_SELECTION_INFO *info
  );
uint32
  ARAD_PMF_TCAM_BANK_SELECTION_verify(
    SOC_SAND_IN  ARAD_PMF_TCAM_BANK_SELECTION *info
  );

uint32
  ARAD_PMF_TCAM_RESULT_INFO_verify(
    SOC_SAND_IN  ARAD_PMF_TCAM_RESULT_INFO *info
  );


uint32
  ARAD_PMF_TCAM_DATA_verify(
    SOC_SAND_IN  ARAD_PMF_TCAM_DATA *info
  );

uint32
  ARAD_PMF_DIRECT_TBL_DATA_verify(
    SOC_SAND_IN  ARAD_PMF_DIRECT_TBL_DATA *info
  );

void
  ARAD_PMF_TCAM_ENTRY_ID_clear(
    SOC_SAND_OUT ARAD_PMF_TCAM_ENTRY_ID *info
  );

void
  ARAD_PMF_LKP_PROFILE_clear(
    SOC_SAND_OUT ARAD_PMF_LKP_PROFILE *info
  );

void
  ARAD_PMF_TCAM_KEY_SELECTION_INFO_clear(
    SOC_SAND_OUT ARAD_PMF_TCAM_KEY_SELECTION_INFO *info
  );

void
  ARAD_PMF_TCAM_BANK_SELECTION_clear(
    SOC_SAND_OUT ARAD_PMF_TCAM_BANK_SELECTION *info
  );

void
  ARAD_PMF_TCAM_RESULT_INFO_clear(
    SOC_SAND_OUT ARAD_PMF_TCAM_RESULT_INFO *info
  );

void
  ARAD_PMF_TCAM_DATA_clear(
    SOC_SAND_OUT ARAD_PMF_TCAM_DATA *info
  );

void
  ARAD_PMF_DIRECT_TBL_DATA_clear(
    SOC_SAND_OUT ARAD_PMF_DIRECT_TBL_DATA *info
  );

#if ARAD_DEBUG_IS_LVL1
const char*
  ARAD_PMF_DIRECT_TBL_KEY_SRC_to_string(
    SOC_SAND_IN  ARAD_PMF_DIRECT_TBL_KEY_SRC enum_val
  );

void
  ARAD_PMF_TCAM_ENTRY_ID_print(
    SOC_SAND_IN  ARAD_PMF_TCAM_ENTRY_ID *info
  );

void
  ARAD_PMF_LKP_PROFILE_print(
    SOC_SAND_IN  ARAD_PMF_LKP_PROFILE *info
  );

void
  ARAD_PMF_TCAM_KEY_SELECTION_INFO_print(
    SOC_SAND_IN  ARAD_PMF_TCAM_KEY_SELECTION_INFO *info
  );

void
  ARAD_PMF_TCAM_BANK_SELECTION_print(
    SOC_SAND_IN  ARAD_PMF_TCAM_BANK_SELECTION *info
  );

void
  ARAD_PMF_TCAM_RESULT_INFO_print(
    SOC_SAND_IN  ARAD_PMF_TCAM_RESULT_INFO *info
  );

void
  ARAD_PMF_TCAM_DATA_print(
    SOC_SAND_IN  ARAD_PMF_TCAM_DATA *info
  );

void
  ARAD_PMF_DIRECT_TBL_DATA_print(
    SOC_SAND_IN  ARAD_PMF_DIRECT_TBL_DATA *info
  );

#endif /* ARAD_DEBUG_IS_LVL1 */


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PMF_LOW_LEVEL_DB_INCLUDED__*/
#endif


