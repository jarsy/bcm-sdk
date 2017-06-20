/* $Id: arad_pp_fp_fem.h,v 1.23 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_FP_FEM__INCLUDED__
/* { */
#define __ARAD_PP_FP_FEM__INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_fp_key.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/arad_tcam.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/PPC/ppc_api_fp_fem.h>

#include <soc/dpp/ARAD/arad_pmf_low_level_fem_tag.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */


#define ARAD_PP_FP_FEM_ALLOC_FES_TM                         (0x1)
#define ARAD_PP_FP_FEM_ALLOC_FES_CHECK_ONLY                 (0x2)
#define ARAD_PP_FP_FEM_ALLOC_FES_CONSIDER_AS_ONE_GROUP      (0x4) /* Consider as one 32 FESes group and forget about the 1st group FEMs */
#define ARAD_PP_FP_FEM_ALLOC_FES_FROM_KEY                   (0x8) /* For large DB, take from key and not from TCAM */
#define ARAD_PP_FP_FEM_ALLOC_FES_KEY_IS_CONDITIONAL_VALID   (0x10) /* For large DB, take from key and not from TCAM */
#define ARAD_PP_FP_FEM_ALLOC_FES_ALWAYS_VALID               (0x20) /* FES always valid*/

#define ARAD_PP_FP_TCAM_ACTION_BUFFER_SIZE   ARAD_TCAM_ACTION_MAX_LEN

/* 160b the key length of TM predefined key: after ITMH */
#define ARAD_PP_FP_KEY_LENGTH_TM_IN_BITS    (160)

#define ARAD_PP_FP_FEM_FES_RESOLUTION_16B                        (16)
/* FES zone selection: 31:0 or 39:8 or 47:16 */
#define ARAD_PP_FP_FEM_FES_2ND_ZONE_LSB                        (8)
#define ARAD_PP_FP_FEM_FES_3RD_ZONE_LSB                        (16)


/* } */
/*************
 * MACROS    *
 *************/

/* given action LSB, len return how to config FES KEY, SHIFT*/
#define ARAD_PP_FP_FEM_ACTION_TABLE_WIDTH_IN_BITS (2 * SOC_DPP_DEFS_GET(unit, tcam_action_width))
#define ARAD_PP_FP_FEM_ACTION_LSB_TO_KEY_ID(_lsb)  (((_lsb) < ARAD_PP_FP_FEM_ACTION_TABLE_WIDTH_IN_BITS)? 0: 1)
#define ARAD_PP_FP_FEM_ACTION_LSB_TO_RELATIVE_LSB(_lsb)  ((_lsb) - (ARAD_PP_FP_FEM_ACTION_TABLE_WIDTH_IN_BITS * ARAD_PP_FP_FEM_ACTION_LSB_TO_KEY_ID(_lsb))) /* Relative to the current action table */

#define ARAD_PP_FP_FEM_ACTION_TABLE_WIDTH_IN_BITS_USE_KAPS (32)

#define ARAD_PP_FP_FEM_ACTION_LSB_TO_SHIFT_USE_KAPS(_lsb) ( ((_lsb) < ARAD_PP_FP_FEM_ACTION_TABLE_WIDTH_IN_BITS_USE_KAPS)? \
      (_lsb) : (_lsb - ARAD_PP_FP_FEM_ACTION_TABLE_WIDTH_IN_BITS_USE_KAPS) )
      

#define ARAD_PP_FP_FEM_ACTION_LSB_TO_KEY(_lsb)  ((ARAD_PP_FP_FEM_ACTION_LSB_TO_RELATIVE_LSB(_lsb) < ARAD_PP_FP_FEM_FES_2ND_ZONE_LSB)? \
                                                  0: (SOC_IS_JERICHO(unit)? ( (ARAD_PP_FP_FEM_ACTION_LSB_TO_RELATIVE_LSB(_lsb) < ARAD_PP_FP_FEM_FES_3RD_ZONE_LSB) ?  1 : 2 ) : 1)  )

#define ARAD_PP_FP_FEM_ACTION_LSB_TO_KEY_ID_USE_KAPS(_lsb) (((_lsb) < ARAD_PP_FP_FEM_ACTION_TABLE_WIDTH_IN_BITS_USE_KAPS)? 0: 1)

#define ARAD_PP_FP_FEM_ACTION_LSB_TO_FES_LSB(_lsb)  ((ARAD_PP_FP_FEM_ACTION_LSB_TO_KEY(_lsb) == 0)? 0: \
                                                 ( SOC_IS_JERICHO(unit) ? ((ARAD_PP_FP_FEM_ACTION_LSB_TO_KEY(_lsb) == 1)?  ARAD_PP_FP_FEM_FES_2ND_ZONE_LSB : ARAD_PP_FP_FEM_FES_3RD_ZONE_LSB) : ARAD_PP_FP_FEM_FES_2ND_ZONE_LSB )  )
#define ARAD_PP_FP_FEM_ACTION_LSB_TO_SHIFT(_lsb) ((ARAD_PP_FP_FEM_ACTION_LSB_TO_KEY(_lsb) == 0)? \
    ARAD_PP_FP_FEM_ACTION_LSB_TO_RELATIVE_LSB(_lsb): \
    (SOC_IS_JERICHO(unit) ? ((ARAD_PP_FP_FEM_ACTION_LSB_TO_KEY(_lsb) == 1)?  ( ARAD_PP_FP_FEM_ACTION_LSB_TO_RELATIVE_LSB(_lsb) - ARAD_PP_FP_FEM_FES_2ND_ZONE_LSB) :  ( ARAD_PP_FP_FEM_ACTION_LSB_TO_RELATIVE_LSB(_lsb) - ARAD_PP_FP_FEM_FES_3RD_ZONE_LSB) ) : ( ARAD_PP_FP_FEM_ACTION_LSB_TO_RELATIVE_LSB(_lsb) - ARAD_PP_FP_FEM_FES_2ND_ZONE_LSB) ) )

/* Choose the closest resolution between 0 / 16 / 32 / 48 / 80 / 96 / 112 / 128 */
#define ARAD_PP_FP_FEM_ACTION_LSB_CLOSEST_16B(_lsb)  ((_lsb / ARAD_PP_FP_FEM_FES_RESOLUTION_16B) * ARAD_PP_FP_FEM_FES_RESOLUTION_16B)

#define ARAD_PP_FP_FEM_ACTION_LSB_TO_KEY_LSB(_lsb)  (((ARAD_PP_FP_FEM_ACTION_LSB_CLOSEST_16B(_lsb) % 80) == 64)? \
            (ARAD_PP_FP_FEM_ACTION_LSB_CLOSEST_16B(_lsb) - 16): ARAD_PP_FP_FEM_ACTION_LSB_CLOSEST_16B(_lsb))

/* Find in TCAM key the closest bit after an LSB that can be inserted in 39:8 or 31:0 according to the action length */
#define ARAD_PP_FP_FEM_ACTION_LOCAL_LSB_ARAD(_first_lsb, _action_length)  \
            (((_first_lsb + _action_length) <= (ARAD_PP_FP_FEM_ACTION_LSB_TO_FES_LSB(_first_lsb) + 32))? _first_lsb : \
            (((ARAD_PP_FP_FEM_ACTION_LSB_TO_KEY(_first_lsb) == 0) && (ARAD_PP_FP_FEM_FES_2ND_ZONE_LSB + _action_length <= 40))? ARAD_PP_FP_FEM_FES_2ND_ZONE_LSB: 40)) 
            /* Try with 8, otherwise Choose 40 if not found */

#define ARAD_PP_FP_FEM_ACTION_LOCAL_LSB_JERICHO_USE_KAPS(_first_lsb, _action_length) \
            ((_first_lsb + _action_length) <= 32 ? _first_lsb : 32)
            
#define ARAD_PP_FP_FEM_ACTION_LOCAL_LSB_JERICHO(use_kaps,_first_lsb, _action_length)  \
            ((use_kaps) ? ARAD_PP_FP_FEM_ACTION_LOCAL_LSB_JERICHO_USE_KAPS(_first_lsb, _action_length):\
            (((_first_lsb + _action_length) <= (ARAD_PP_FP_FEM_ACTION_LSB_TO_FES_LSB(_first_lsb) + 32))? _first_lsb : \
            (((ARAD_PP_FP_FEM_ACTION_LSB_TO_KEY(_first_lsb) == 0) && (ARAD_PP_FP_FEM_FES_2ND_ZONE_LSB + _action_length <= 40))? ARAD_PP_FP_FEM_FES_2ND_ZONE_LSB: \
            (((ARAD_PP_FP_FEM_ACTION_LSB_TO_KEY(_first_lsb) == 1) && (ARAD_PP_FP_FEM_FES_3RD_ZONE_LSB + _action_length <= 48))? ARAD_PP_FP_FEM_FES_3RD_ZONE_LSB:48)))) 
            /* Try with 8, otherwise Choose 40 if not found */
  
#define ARAD_PP_FP_FEM_ACTION_LOCAL_LSB(use_kaps,_first_lsb, _action_length)  \
            (SOC_IS_JERICHO(unit)? ARAD_PP_FP_FEM_ACTION_LOCAL_LSB_JERICHO(use_kaps,_first_lsb, _action_length) : ARAD_PP_FP_FEM_ACTION_LOCAL_LSB_ARAD(_first_lsb, _action_length) )

#define ARAD_PP_FP_FEM_IS_ACTION_NOT_REQUIRE_FEM(action_type)    ( (action_type == SOC_PPC_FP_ACTION_TYPE_CHANGE_KEY) || \
                                                                   (action_type == SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_0 ) || \
                                                                   (action_type == SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_1 ) || \
                                                                   (action_type == SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_2 ) || \
                                                                   (action_type == SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_3 ) || \
                                                                   (action_type == SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_KAPS ) )
/* { */
/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */


typedef struct {

    /* which cycle to allocate resource */
    uint32 cycle;

    /* what is tcam-result map to this actions */
    uint32 tcam_res_id[ARAD_PP_FP_KEY_NOF_KEYS_PER_DB_MAX];

    /* what is tcam-result size: 20, 40 bit, see ARAD_TCAM_ACTION_SIZE*/
    ARAD_TCAM_ACTION_SIZE action_size;

    /* used enternally, 0-39 where the action in the buffer, set to zero 
    uint32 action_lsb;*/

}ARAD_PP_FEM_ACTIONS_CONSTRAINT;


typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_FP_FEM_INSERT = ARAD_PP_PROC_DESC_BASE_FP_FEM_FIRST,
  ARAD_PP_FP_FEM_INSERT_PRINT,
  ARAD_PP_FP_FEM_INSERT_UNSAFE,
  ARAD_PP_FP_FEM_INSERT_VERIFY,
  ARAD_PP_FP_FEM_IS_PLACE_GET,
  ARAD_PP_FP_FEM_IS_PLACE_GET_PRINT,
  ARAD_PP_FP_FEM_IS_PLACE_GET_UNSAFE,
  ARAD_PP_FP_FEM_IS_PLACE_GET_VERIFY,
  ARAD_PP_FP_FEM_TAG_SET,
  ARAD_PP_FP_FEM_TAG_SET_PRINT,
  ARAD_PP_FP_FEM_TAG_SET_UNSAFE,
  ARAD_PP_FP_FEM_TAG_SET_VERIFY,
  ARAD_PP_FP_FEM_TAG_GET,
  ARAD_PP_FP_FEM_TAG_GET_PRINT,
  ARAD_PP_FP_FEM_TAG_GET_VERIFY,
  ARAD_PP_FP_FEM_TAG_GET_UNSAFE,
  ARAD_PP_FP_FEM_GET_PROCS_PTR,
  ARAD_PP_FP_FEM_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
   ARAD_PP_FP_TAG_ACTION_TYPE_CONVERT,
   ARAD_PP_FP_FEM_IS_PLACE_GET_FOR_CYCLE,
   ARAD_PP_FP_FEM_IS_FEM_BLOCKING_GET,
   ARAD_PP_FP_FEM_DUPLICATE,
   ARAD_PP_FP_FEM_CONFIGURE,
   ARAD_PP_FP_FEM_CONFIGURATION_GET,
   ARAD_PP_FP_FEM_REMOVE,
   ARAD_PP_FP_FEM_REORGANIZE,



  /*
   * Last element. Do no touch.
   */
  ARAD_PP_FP_FEM_PROCEDURE_DESC_LAST
} ARAD_PP_FP_FEM_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_FP_FEM_PFG_NDX_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_FP_FEM_FIRST,
  ARAD_PP_FP_FEM_DB_ID_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_FP_FEM_DB_STRENGTH_OUT_OF_RANGE_ERR,
  ARAD_PP_FP_FEM_DB_ID_OUT_OF_RANGE_ERR,
  SOC_PPC_FP_FEM_ENTRY_STRENGTH_OUT_OF_RANGE_ERR,
  SOC_PPC_FP_FEM_ENTRY_ID_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */



  /*
   * Last element. Do no touch.
   */
  ARAD_PP_FP_FEM_ERR_LAST
} ARAD_PP_FP_FEM_ERR;

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
    arad_pp_fp_fem_pgm_per_pmf_pgm_get(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE      stage,
    SOC_SAND_IN  uint32                         pmf_pgm_id,
    SOC_SAND_OUT uint8                          *fem_pgm_id
    );

uint32
    arad_pp_fp_fem_pgm_per_pmf_pgm_set(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE      stage,
    SOC_SAND_IN  uint32                         pmf_pgm_id,
    SOC_SAND_IN  uint8                          fem_pgm_id
    );

uint32
    arad_pp_fp_fem_pgm_id_remove(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE      stage,
    SOC_SAND_IN  uint32                         pmf_pgm_id
    );

uint32
    arad_pp_fp_fem_pgm_id_alloc(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE      stage,
    SOC_SAND_OUT uint8                          *fem_pgm_id
    );

uint32
  arad_pp_fp_fem_pgm_id_bmp_get(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  uint32                       db_id,
    SOC_SAND_OUT uint8                     *fem_pgm_id
    );


uint32
  arad_pp_fp_action_alloc_fes(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  uint32                       db_id,
    SOC_SAND_IN  uint32                       prog_id,
    SOC_SAND_IN  uint32                       entry_id,
    SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE       action_type, /* action*/
    SOC_SAND_IN  uint32                       flags,
    SOC_SAND_IN  uint32                       my_priority, 
    SOC_SAND_IN  ARAD_PP_FEM_ACTIONS_CONSTRAINT *constraint,
    SOC_SAND_IN  uint32                       action_lsb,/* lsb of current action */
    SOC_SAND_IN  uint32                       action_len,/* length of current action */
    SOC_SAND_IN  int32                        required_nof_feses,/* no. of FESes for this DB. Ignore if negative. */
    SOC_SAND_INOUT ARAD_PMF_FES               fes_info[ARAD_PMF_LOW_LEVEL_NOF_FESS],
    SOC_SAND_OUT uint8                        *found
  );

uint32
  arad_pp_fp_action_alloc_in_prog(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  uint32                       db_id,
    SOC_SAND_IN  uint32                       prog_id,
    SOC_SAND_IN  uint32                       flags,
    SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE       action_types[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX], /* actions*/
    SOC_SAND_IN  uint32                       priority, 
    SOC_SAND_IN  ARAD_PP_FEM_ACTIONS_CONSTRAINT *constraint,
    SOC_SAND_OUT uint8                        *action_alloced
  );

uint32
  arad_pp_fp_action_alloc_in_prog_with_entry(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  uint32                       db_id,
    SOC_SAND_IN  uint32                       prog_id,
    SOC_SAND_IN  uint32                       entry_id,
    SOC_SAND_IN  uint32                       flags,
    SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE       action_types[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX], /* actions*/
    SOC_SAND_IN  uint32                       priority, 
    SOC_SAND_IN  ARAD_PP_FEM_ACTIONS_CONSTRAINT *constraint,
    SOC_SAND_OUT uint8                        *action_alloced
  );

uint32
  arad_pp_fp_action_value_to_buffer(
      SOC_SAND_IN  int                    unit,
      SOC_SAND_IN  SOC_PPC_FP_ACTION_VAL     action_vals[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX],
      SOC_SAND_IN  uint32                    db_id,
      SOC_SAND_OUT  uint32                   buffer[ARAD_PP_FP_TCAM_ACTION_BUFFER_SIZE],
      SOC_SAND_OUT  uint32                   *buffer_size
  );

/* 
 * Retrieve the Action values from a buffer (the TCAM Action table value)
 */
uint32
  arad_pp_fp_action_buffer_to_value(
      SOC_SAND_IN  int                    unit,
      SOC_SAND_IN  uint32                    db_id,
      SOC_SAND_IN  uint32                   buffer[ARAD_PP_FP_TCAM_ACTION_BUFFER_SIZE],
      SOC_SAND_OUT  SOC_PPC_FP_ACTION_VAL     action_vals[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX]
  );

uint32
  arad_pp_fp_tag_action_type_convert(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE         action_type,
    SOC_SAND_OUT uint8                    *is_tag_action,
    SOC_SAND_OUT uint32                    *action_ndx
  );

uint32
  arad_pp_fp_action_type_from_pmf_convert(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE  pmf_fem_action_type,
    SOC_SAND_OUT SOC_PPC_FP_ACTION_TYPE    *fp_action_type
  );

uint32
  arad_pp_fp_action_type_max_size_get(
    SOC_SAND_IN  int            unit,
    SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE fp_action_type,
    SOC_SAND_OUT uint32            *action_size_in_bits,
    SOC_SAND_OUT uint32            *action_size_in_bits_in_fem
  );

/* 
 * Get the location inside the 32b Direct extraction key 
 * of a specific field and its length 
 * Assumption: no field is split into 2 Copy Engines (of 16b) 
 */
uint32
  arad_pp_fp_qual_lsb_and_length_get(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32                  db_id_ndx,
    SOC_SAND_IN  SOC_PPC_FP_QUAL_TYPE         qual_type,
    SOC_SAND_OUT uint32                   *qual_lsb,
    SOC_SAND_OUT uint32                   *qual_length_no_padding
  );


uint32
  arad_pp_fp_action_lsb_get(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE       action_type,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_INFO     *fp_database_info,
    SOC_SAND_OUT uint32                   *action_lsb
  );

uint32
  arad_pp_fp_fem_remove(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_PPC_FP_FEM_ENTRY           *entry_ndx
  );

uint32
  arad_pp_fp_fem_configuration_de_get(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                    fem_id_ndx,
    SOC_SAND_IN  uint32                    cycle_ndx,
    SOC_SAND_IN  uint8                     fem_pgm_id,
    SOC_SAND_OUT SOC_PPC_FP_FEM_ENTRY           *entry_ndx,
    SOC_SAND_OUT SOC_PPC_FP_DIR_EXTR_ACTION_VAL *fem_info,
    SOC_SAND_OUT SOC_PPC_FP_QUAL_VAL            *qual_info
  );


/*********************************************************************
* NAME:
 *   arad_pp_fp_fem_insert_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Compute the best configuration to add greedily Direct
 *   Extraction entries (preference to the new
 *   Database-ID). If set, set all the FEM (selected bits,
 *   actions) and its input. Look at the previous FEM
 *   configuration to shift the FEMs if necessary. The FEM
 *   input can be changed again upon the new TCAM DB
 *   creation.
 * INPUT:
 *   SOC_SAND_IN  int                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FP_FEM_ENTRY           *entry_ndx -
 *     Entry requiring a FEM.
 *   SOC_SAND_IN  SOC_PPC_FP_DIR_EXTR_ACTION_VAL *fem_info -
 *     Parameters of the FEM necessary to know if a place if
 *     free.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE         *success -
 *     Indicate if the database is created successfully.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_fp_fem_insert_unsafe(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_PPC_FP_FEM_ENTRY           *entry_ndx,
    SOC_SAND_IN  SOC_PPC_FP_FEM_CYCLE           *fem_cycle,
    SOC_SAND_IN  uint32                       flags,
    SOC_SAND_IN  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO              *fem_info,
    SOC_SAND_INOUT ARAD_PMF_FES                 fes_info[ARAD_PMF_LOW_LEVEL_NOF_FESS],
    SOC_SAND_IN uint8                           fem_pgm_id,
    SOC_SAND_OUT uint32                         *fes_fem_id,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE         *success
  );

uint32
  arad_pp_fp_fem_insert_verify(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_PPC_FP_FEM_ENTRY           *entry_ndx,
    SOC_SAND_IN  SOC_PPC_FP_FEM_CYCLE           *fem_cycle,
    SOC_SAND_IN  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO *fem_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_fp_fem_is_place_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Check out if there is an empty FEM for this entry.
 * INPUT:
 *   SOC_SAND_IN  int                            unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FP_FEM_ENTRY           *entry_ndx -
 *     Entry requiring a FEM.
 *   SOC_SAND_IN  SOC_PPC_FP_FEM_CYCLE           *fem_info -
 *     Parameters of the FEM necessary to know if a place if
 *     free.
 *   SOC_SAND_OUT uint8                          *place_found -
 *     If True, then a place (i.e., FEM) is found for this
 *     entry.
 *   SOC_SAND_OUT char                           *reason_for_fail -
 *     This procedure expects to get a NULL terminated string
 *     and to load it by text (and call stack) explaining
 *     why place was not found (in case *place_found is NOT
 *     true). Loaded text must start at the ending NULL and
 *     must be NULL terminated and shorter than reason_for_fail_len.
 *   SOC_SAND_IN int32                           reason_for_fail_len -
 *     Number of characters available on array pointed
 *     by *reason_for_fail, including ending NULL. If '1' or smaller
 *     than there is no space left.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_fp_fem_is_place_get_unsafe(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_PPC_FP_FEM_ENTRY   *entry_ndx,
    SOC_SAND_IN  SOC_PPC_FP_FEM_CYCLE   *fem_info,
    SOC_SAND_IN  uint8                  is_for_tm,
    SOC_SAND_IN  uint8                  fem_pgm_id,
    SOC_SAND_INOUT ARAD_PMF_FES         fes_info[ARAD_PMF_LOW_LEVEL_NOF_FESS],
    SOC_SAND_OUT uint8                  *place_found,
    SOC_SAND_OUT char                   *reason_for_fail,
    SOC_SAND_IN int32                   reason_for_fail_len
  );

uint32
  arad_pp_fp_fem_is_place_get_verify(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_PPC_FP_FEM_ENTRY           *entry_ndx,
    SOC_SAND_IN  SOC_PPC_FP_FEM_CYCLE            *fem_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_fp_fem_tag_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Check out if there is an empty FEM for this entry.
 * INPUT:
 *   SOC_SAND_IN  int                    unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                    db_id_ndx -
 *     Database-ID. Range: 0 - 127.
 *   SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE         action_type -
 *     Tag action type.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_fp_fem_tag_set_unsafe(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                    db_id_ndx,
    SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE           action_type
  );

uint32
  arad_pp_fp_fem_tag_set_verify(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                    db_id_ndx,
    SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE           action_type
  );

uint32
  arad_pp_fp_action_alloc(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  uint32                       db_id,
    SOC_SAND_IN  uint32                       flags,
    SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE       action_types[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX], /* actions*/
    SOC_SAND_IN  uint32                       priority, /* 0, 1, any*/
    SOC_SAND_IN  uint32                       selected_cycle[ARAD_PMF_LOW_LEVEL_NOF_PROGS_ALL_STAGES],
    SOC_SAND_INOUT  ARAD_PP_FEM_ACTIONS_CONSTRAINT *constraint,
    SOC_SAND_OUT uint8                          *action_alloced
  );

uint32
  arad_pp_fp_action_dealloc(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  uint32                       db_id,
    SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE       action_types[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX] /* actions*/
  );

uint32
  arad_pp_fp_action_to_lsbs(
      SOC_SAND_IN  int                    unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE    stage,
      SOC_SAND_IN  uint8                     use_kaps,
      SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE    action_types[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX], /* actions*/
      SOC_SAND_IN  uint32                    action_widths[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX],
      SOC_SAND_OUT  uint32                   action_lsbs[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX+1],
      SOC_SAND_OUT  uint32                   action_lengths[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX+1],
      SOC_SAND_OUT  ARAD_TCAM_ACTION_SIZE  *action_size,
      SOC_SAND_OUT  uint32                   *nof_actions,
      SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE  *success
  );

/*********************************************************************
* NAME:
 *   arad_pp_fp_fem_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_fp_fem module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_fp_fem_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_fp_fem_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_fp_fem module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_fp_fem_get_errs_ptr(void);

uint32
  SOC_PPC_FP_FEM_ENTRY_verify(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  SOC_PPC_FP_FEM_ENTRY *info
  );

uint32
  SOC_PPC_FP_FEM_CYCLE_verify(
    SOC_SAND_IN  SOC_PPC_FP_FEM_CYCLE *info
  );

/* } */


void
  ARAD_PP_FEM_ACTIONS_CONSTRAINT_clear(
    SOC_SAND_OUT ARAD_PP_FEM_ACTIONS_CONSTRAINT *info
  );


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_FP_FEM__INCLUDED__*/
#endif


