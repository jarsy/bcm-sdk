/* $Id: tmc_api_pmf_low_level_db.h,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/tmc/include/soc_tmcapi_pmf_low_level.h
*
* MODULE PREFIX:  soc_tmcpmf
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

#ifndef __SOC_TMC_API_PMF_LOW_LEVEL_DB_INCLUDED__
/* { */
#define __SOC_TMC_API_PMF_LOW_LEVEL_DB_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/TMC/tmc_api_pmf_low_level_ce.h>
#include <soc/dpp/TMC/tmc_api_tcam.h>
#include <soc/dpp/TMC/tmc_api_tcam_key.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Maximum number of banks per TCAM.                       */
#define  SOC_TMC_PMF_TCAM_NOF_BANKS (SOC_TMC_TCAM_NOF_BANKS)

/*     Number of final TCAM lookup results.                    */
#define  SOC_TMC_PMF_TCAM_NOF_LKP_RESULTS (4)

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
   *  Key A [9:0]
   */
  SOC_TMC_PMF_DIRECT_TBL_KEY_SRC_A_9_0 = 0,
  /*
   *  Key A [19:10]
   */
  SOC_TMC_PMF_DIRECT_TBL_KEY_SRC_A_19_10 = 1,
  /*
   *  Key A [29:20]
   */
  SOC_TMC_PMF_DIRECT_TBL_KEY_SRC_A_29_20 = 2,
  /*
   *  Key A [39:30].
   */
  SOC_TMC_PMF_DIRECT_TBL_KEY_SRC_A_39_30 = 3,
  /*
   *  Key B [9:0]
   */
  SOC_TMC_PMF_DIRECT_TBL_KEY_SRC_B_9_0 = 4,
  /*
   *  Key B [19:10]
   */
  SOC_TMC_PMF_DIRECT_TBL_KEY_SRC_B_19_10 = 5,
  /*
   *  Key B [29:20]
   */
  SOC_TMC_PMF_DIRECT_TBL_KEY_SRC_B_29_20 = 6,
  /*
   *  Key B [39:30].
   */
  SOC_TMC_PMF_DIRECT_TBL_KEY_SRC_B_39_30 = 7,
  /*
   *  Number of types in SOC_TMC_PMF_DIRECT_TBL_KEY_SRC
   */
  SOC_TMC_NOF_PMF_DIRECT_TBL_KEY_SRCS = 8
}SOC_TMC_PMF_DIRECT_TBL_KEY_SRC;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  TCAM Database-ID. Range: 0 - 16K-1. (Soc_petra-B)
   *  In an explicit address mode, bank-id. Range: 0 - 3.
   */
  uint32 db_id;
  /*
   *  Entry index in the TCAM Database. Range: 0 - 16K-1.
   *  In an explicit address mode, line-id of the TCAM bank. Range: 0 - 511.
   */
  uint32 entry_id;

} SOC_TMC_PMF_TCAM_ENTRY_ID;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Lookup-Profile-Id. Range: 0 - 7. (Soc_petra-B)
   */
  uint32 id;
  /*
   *  Cycle index in which the access occurs. Range: 0 - 1.
   *  (Soc_petra-B)
   */
  uint32 cycle_ndx;

} SOC_TMC_PMF_LKP_PROFILE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If True, then this bank is considered during the key
   *  lookup to get the TCAM lookup result.
   */
  uint8 bank_id_enabled[SOC_TMC_PMF_TCAM_NOF_BANKS];

} SOC_TMC_PMF_TCAM_BANK_SELECTION;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Select how to get lookup result: which banks to compare.
   *  The first lookup result can compare between all the
   *  banks, the second between two banks at the most, after
   *  that the lookup result must equal a single bank result.
   */
  SOC_TMC_PMF_TCAM_BANK_SELECTION bank_sel[SOC_TMC_PMF_TCAM_NOF_LKP_RESULTS];

} SOC_TMC_PMF_TCAM_RESULT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If True, the Lookup-Profile points to this Database.
   */
  uint8 is_valid;
  /*
   *  Key source of the key entering the TCAM.
   */
  SOC_TMC_PMF_TCAM_KEY_SRC key_src;
  /*
   *  Bank selection per TCAM lookup result: set the validity
   *  and priority of each TCAM bank.
   */
  SOC_TMC_PMF_TCAM_RESULT_INFO bank_prio;

} SOC_TMC_PMF_TCAM_KEY_SELECTION_INFO;




typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  TCAM key with raw bitmap presentation.
   */
  SOC_TMC_TCAM_KEY key;
  /*
   *  Entry priority in the TCAM Database. Range: 0 - 16K-1.
   */
  uint32 priority;
  /*
   *  TCAM output value. For Soc_petra-B, it is encoded in 32
   *  bits.
   */
  SOC_TMC_TCAM_OUTPUT output;

} SOC_TMC_PMF_TCAM_DATA;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Value of the direct table output. Range: 0 - 2^20-1.
   */
  uint32 val;

} SOC_TMC_PMF_DIRECT_TBL_DATA;

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
  SOC_TMC_PMF_TCAM_ENTRY_ID_clear(
    SOC_SAND_OUT SOC_TMC_PMF_TCAM_ENTRY_ID *info
  );

void
  SOC_TMC_PMF_LKP_PROFILE_clear(
    SOC_SAND_OUT SOC_TMC_PMF_LKP_PROFILE *info
  );

void
  SOC_TMC_PMF_TCAM_BANK_SELECTION_clear(
    SOC_SAND_OUT SOC_TMC_PMF_TCAM_BANK_SELECTION *info
  );

void
  SOC_TMC_PMF_TCAM_DATA_clear(
    SOC_SAND_OUT SOC_TMC_PMF_TCAM_DATA *info
  );


void
  SOC_TMC_PMF_TCAM_RESULT_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PMF_TCAM_RESULT_INFO *info
  );

void
  SOC_TMC_PMF_TCAM_KEY_SELECTION_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PMF_TCAM_KEY_SELECTION_INFO *info
  );

void
  SOC_TMC_PMF_DIRECT_TBL_DATA_clear(
    SOC_SAND_OUT SOC_TMC_PMF_DIRECT_TBL_DATA *info
  );

#if SOC_TMC_DEBUG_IS_LVL1

const char*
  SOC_TMC_PMF_DIRECT_TBL_KEY_SRC_to_string(
    SOC_SAND_IN  SOC_TMC_PMF_DIRECT_TBL_KEY_SRC enum_val
  );

void
  SOC_TMC_PMF_TCAM_ENTRY_ID_print(
    SOC_SAND_IN  SOC_TMC_PMF_TCAM_ENTRY_ID *info
  );

void
  SOC_TMC_PMF_LKP_PROFILE_print(
    SOC_SAND_IN  SOC_TMC_PMF_LKP_PROFILE *info
  );

void
  SOC_TMC_PMF_TCAM_BANK_SELECTION_print(
    SOC_SAND_IN  SOC_TMC_PMF_TCAM_BANK_SELECTION *info
  );

void
  SOC_TMC_PMF_TCAM_RESULT_INFO_print(
    SOC_SAND_IN  SOC_TMC_PMF_TCAM_RESULT_INFO *info
  );

void
  SOC_TMC_PMF_TCAM_KEY_SELECTION_INFO_print(
    SOC_SAND_IN  SOC_TMC_PMF_TCAM_KEY_SELECTION_INFO *info
  );


void
  SOC_TMC_PMF_TCAM_DATA_print(
    SOC_SAND_IN  SOC_TMC_PMF_TCAM_DATA *info
  );

void
  SOC_TMC_PMF_DIRECT_TBL_DATA_print(
    SOC_SAND_IN  SOC_TMC_PMF_DIRECT_TBL_DATA *info
  );

#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_PMF_LOW_LEVEL_DB_INCLUDED__*/
#endif

