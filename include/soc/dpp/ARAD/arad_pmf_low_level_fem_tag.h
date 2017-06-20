/* $Id: arad_pmf_low_level_fem_tag.h,v 1.17 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PMF_LOW_LEVEL_FEM_TAG_INCLUDED__
/* { */
#define __ARAD_PMF_LOW_LEVEL_FEM_TAG_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/arad_pmf_low_level.h>
#include <soc/dpp/ARAD/arad_pmf_low_level_db.h>
#include <soc/dpp/ARAD/arad_api_framework.h>
#include <soc/dpp/TMC/tmc_api_pmf_low_level_fem_tag.h>


#include <soc/dpp/PPC/ppc_api_fp.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_flp_init.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */



#define ARAD_PMF_FEM_BIT_LOC_TYPE_CST                     SOC_TMC_PMF_FEM_BIT_LOC_TYPE_CST
#define ARAD_PMF_FEM_BIT_LOC_TYPE_KEY                     SOC_TMC_PMF_FEM_BIT_LOC_TYPE_KEY
#define ARAD_PMF_FEM_BIT_LOC_TYPE_MAP_DATA                SOC_TMC_PMF_FEM_BIT_LOC_TYPE_MAP_DATA
#define ARAD_NOF_PMF_FEM_BIT_LOC_TYPES                    SOC_TMC_NOF_PMF_FEM_BIT_LOC_TYPES
typedef SOC_TMC_PMF_FEM_BIT_LOC_TYPE                           ARAD_PMF_FEM_BIT_LOC_TYPE;

typedef SOC_TMC_PMF_FEM_INPUT_SRC_ARAD                         ARAD_PMF_FEM_INPUT_SRC_ARAD;
typedef SOC_TMC_PMF_FES_INPUT_INFO                             ARAD_PMF_FES_INPUT_INFO;
typedef SOC_TMC_PMF_FEM_INPUT_INFO                             ARAD_PMF_FEM_INPUT_INFO;
typedef SOC_TMC_PMF_FEM_NDX                                    ARAD_PMF_FEM_NDX;
typedef SOC_TMC_PMF_FEM_SELECTED_BITS_INFO                     ARAD_PMF_FEM_SELECTED_BITS_INFO;
typedef SOC_TMC_PMF_FEM_ACTION_FORMAT_MAP_INFO                 ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO;
typedef SOC_TMC_PMF_FEM_BIT_LOC                                ARAD_PMF_FEM_BIT_LOC;
typedef SOC_TMC_PMF_FEM_ACTION_FORMAT_INFO                     ARAD_PMF_FEM_ACTION_FORMAT_INFO;

#define ARAD_PMF_FEM_TM_ACTION_FORMAT_NDX_DEFAULT (1)
#define ARAD_PMF_FEM_ACTION_DEFAULT_DEST_1        (1)
#define ARAD_PMF_FEM_FTMH_ACTION_FORMAT_NDX       (1)
#define ARAD_PMF_FEM_ETH_ACTION_FORMAT_NDX        (2)
#define ARAD_PMF_FEM_ACTION_DEFAULT_NOP_3         (3)


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

typedef struct
{
    /*
     *  Egress Action type
     */
    SOC_PPC_FP_ACTION_TYPE    action_type;

    uint32 msb;
    uint32 lsb;
    uint32 base0;
    uint32 base1;

    /* TCAM Action table attributes */
    uint32 lsb_hw;
    uint32 size;
} ARAD_PMF_FEM_ACTION_EGRESS_SIGNAL;

typedef struct
{
    /*
     *  Egress Action type
     */
    SOC_PPC_FP_ACTION_TYPE    action_type;

    /* TCAM Action table attributes */
    uint32 lsb_hw;
    uint32 size;
    int valid; /* If not valid -1, if valid - valid bit */
} ARAD_PMF_FEM_ACTION_EGRESS_INFO;

/* } */
/*************
 * GLOBALS   *
 *************/

extern CONST soc_mem_t Arad_pmf_fem_map_tbl[];
extern CONST soc_field_t Arad_pmf_fem_map_field_select_field[];

/* { */

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

/* Reference table for fields filling */
uint32 
     arad_pmf_fem_map_tbl_reference(
        SOC_SAND_IN  int                         unit,
        SOC_SAND_IN  uint32                      fem_type,
        SOC_SAND_OUT soc_mem_t                   *mem
     );

/* Get the RUNTIME array size */
uint32
  arad_pmf_fem_action_type_array_size_get_unsafe(
      SOC_SAND_IN  int                            unit
  ) ;

/* Get the DEFAULT array size */
uint32
  arad_pmf_fem_action_type_array_size_default_get_unsafe(
      SOC_SAND_IN  int                            unit
  ) ;

/*
 * Return an element from DEFAULT array of actions:
 *   *_pmf_fem_action_type_encoding
 * If some error is encountered, then return '-2'
 *
 */
uint32
  arad_pmf_fem_action_type_array_element_default_get_unsafe(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  uint32                table_line,
      SOC_SAND_IN  uint32                sub_index
  ) ;

/* Get the RUNTIME array element attributes */
uint32
  arad_pmf_fem_action_type_array_element_get_unsafe(
      SOC_SAND_IN  int                            unit,
      SOC_SAND_IN  uint32                            table_line,
      SOC_SAND_IN  uint32                            sub_index
  );
/*
 *   Function
 *      arad_pmf_fem_action_width_default_get
 *   Purpose
 *      Given 'action', get 'width from DEFAULT array of actions).
 *   Parameters
 *      (in)  int unit =
 *              The unit number
 *      (in)  SOC_PPC_FP_ACTION_TYPE in_action_type =
 *              The action for which to get the DEFAULT width. It is
 *              converted to index on corresponding DEFAULT array at
 *              which to get the width. (This index effectively
 *              identified the 'action' for which to update the width
 *              since each line also contains 'action identifier'.)
 *              Note that this is this is the low level action type
 *              and not the higher BCM level action type which is not
 *              directly related to HW.
 *      (out) uint32 *out_action_width =
 *              Loaded by DEFAULT width (no. of bits) assigned to specified
 *              'in_action_type'.
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *     No direct hardware changes.
 *
 *     Default arrays (in sw state):
 *       Jericho_pmf_fem_action_type_encoding
 *       Arad_plus_pmf_fem_action_type_encoding
 *       Arad_pmf_fem_action_type_encoding
 *
 *     See:
 *       bcm_petra_field_action_width_set(), INDEX_OF_ACTION_WIDTH,
 *       NUM_ACTION_ELEMENTS, INDEX_OF_ACTION,SOC_PPC_FP_ACTION_TYPE,
 *       bcm_field_action_t
 */
uint32
  arad_pmf_fem_action_width_default_get(
      SOC_SAND_IN  int                     unit,
      SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE  in_action_type,
      SOC_SAND_OUT uint32                  *out_action_width
  ) ;
/*
 *   Function
 *      arad_pmf_is_field_initialized
 *   Purpose
 *      Indicate whether 'field' has been initialized on sw state.
 *   Parameters
 *      (in)  int unit =
 *             The unit number
 *      (out) uint8 *field_is_initialized_p -
 *              If 'field' has been initialized, then return *field_is_initialized_p
 *              set to non-zero. Otherwise, set *field_is_initialized_p
 *              to zero.
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *
 *     See:
 *       arad_pmf_fem_action_width_set_unsafe
 */
uint32
  arad_pmf_is_field_initialized(
      SOC_SAND_IN  int   unit,
      SOC_SAND_OUT uint8 *field_is_initialized_p
  ) ;
/*
 *   Function
 *      arad_pmf_fem_action_width_set_unsafe
 *   Purpose
 *      Set the action 'width', in bits, for action on RUNTIME
 *      array of actions on the specified line (index).
 *   Parameters
 *      (in) int unit =
 *             The unit number
 *      (in) SOC_PPC_FP_ACTION_TYPE in_action_type =
 *             The action for which to set the new width. It is
 *             converted to index on corresponding RUNTIME array at
 *             which to update the width. (This index effectively
 *             identified the 'action' for which to update the width
 *             since each line also contains 'action identifier'.)
 *             Note that this is this is the low level action type
 *             and not the higher BCM level action type which is not
 *             directly related to HW.
 *      (in) uint32 in_action_width =
 *             New width (no. of bits) to assign to specified 'in_action_type'.
 *      (out) uint32 *db_identifier_p -
 *              If input action is NOT found on any DB, then return
 *              *db_identifier_p set to -1. Otherwise, set *db_identifier_p
 *              to the index of containing DB. Note that this is true even
 *              when return value indicates 'error'.
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *     No direct hardware changes.
 *
 *     If runtime array is found to not have yet been initialized, then
 *     this procedure initializes it with the default values.
 *
 *     Runtime arrays (in sw state):
 *       Jericho_pmf_fem_action_type_encoding_runtime
 *       Arad_plus_pmf_fem_action_type_encoding_runtime
 *       Arad_pmf_fem_action_type_encoding_runtime
 *
 *     If specified action (on entry 'table_line') is in use on any
 *     currently active data base then this operation is refused
 *     (unless the new width is equal to the one currently being
 *     used). Data base must be destroyed first.
 *
 *     See:
 *       bcm_petra_field_action_width_set(), INDEX_OF_ACTION_WIDTH,
 *       NUM_ACTION_ELEMENTS, INDEX_OF_ACTION,SOC_PPC_FP_ACTION_TYPE,
 *       bcm_field_action_t
 */
uint32
  arad_pmf_fem_action_width_set_unsafe(
      SOC_SAND_IN  int                     unit,
      SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE  in_action_type,
      SOC_SAND_IN  uint32                  in_action_width,
      SOC_SAND_OUT uint32                  *db_identifier_p
  ) ;
/*
 *   Function
 *      arad_pmf_fem_action_width_get_unsafe
 *   Purpose
 *      Get the action 'width', in bits, for action on RUNTIME
 *      array of actions on the the line corresponding to specified 'action'.
 *   Parameters
 *      (in) int unit =
 *             The unit number
 *      (in) SOC_PPC_FP_ACTION_TYPE in_action_type =
 *             The action for which to get the width. It is
 *             converted to index on corresponding RUNTIME array at
 *             which to update the width. (This index effectively
 *             identified the 'action' for which to update the width
 *             since each line also contains 'action identifier'.)
 *             Note that this is this is the low level action type
 *             and not the higher BCM level action type which is not
 *             directly related to HW.
 *      (out)uint32 *out_action_width =
 *             Loaded by width (no. of bits) assigned to specified 'in_action_type'.
 *             If 'in_action_type' is not found on the device-matching
 *             runtime array (e.g., Jericho_pmf_fem_action_type_encoding_runtime)
 *             then this procedure returns *out_action_width=NOT_ON_RUNTIME_ARRAY.
 *             This is not an error!
 *      (out)uint32 *out_hw_id =
 *             Loaded by HW identifier assigned to specified 'in_action_type'.
 *             If 'in_action_type' is not found on the device-matching
 *             runtime array (e.g., Jericho_pmf_fem_action_type_encoding_runtime)
 *             then this procedure returns *out_hw_id=NOT_ON_RUNTIME_ARRAY.
 *             This is not an error!
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *     No direct hardware changes.
 *
 *     If runtime array is found to not have yet been initialized, then
 *     this procedure initializes it with the default values.
 *
 *     Runtime arrays (in sw state):
 *       Jericho_pmf_fem_action_type_encoding_runtime
 *       Arad_plus_pmf_fem_action_type_encoding_runtime
 *       Arad_pmf_fem_action_type_encoding_runtime
 *
 *     See:
 *       bcm_petra_field_action_width_set(), INDEX_OF_ACTION_WIDTH,
 *       NUM_ACTION_ELEMENTS, INDEX_OF_ACTION,SOC_PPC_FP_ACTION_TYPE,
 *       bcm_field_action_t
 */
uint32
  arad_pmf_fem_action_width_get_unsafe(
      SOC_SAND_IN  int                     unit,
      SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE  in_action_type,
      SOC_SAND_OUT uint32                  *out_action_width,
      SOC_SAND_OUT uint32                  *out_hw_id
  ) ;

/* Get the attributes of the egress actions */
uint32
  arad_pmf_fem_action_egress_info_get(
      SOC_SAND_IN  int                           unit,
      SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE               action_type,
      SOC_SAND_OUT uint8                             *is_found,
      SOC_SAND_OUT ARAD_PMF_FEM_ACTION_EGRESS_SIGNAL *action_egress_info
  );

uint32
  arad_pmf_low_level_fem_tag_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

uint32
    arad_pmf_fem_action_elk_result_size_update(int unit, int res_sizes[ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS]);

uint32
  arad_egress_pmf_db_action_get_unsafe(
      SOC_SAND_IN  int                     unit,
      SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE  action_type,
      SOC_SAND_OUT uint32                 *action_lsb,
      SOC_SAND_OUT uint32                 *action_size,
      SOC_SAND_OUT int                    *action_valid
  );

/* given FP action reurn number of bits in this action*/
uint32
  arad_pmf_db_fes_action_size_get_unsafe(
      SOC_SAND_IN  int                            unit,
      SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE     action_type,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
      SOC_SAND_OUT uint32                 *action_size,
      SOC_SAND_OUT uint32                 *action_lsb_egress
  );

/* Retrieve the SW action from the HW value */
uint32
  arad_pmf_db_action_type_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 action_type_hw,
    SOC_SAND_OUT  uint8                    *is_found,
    SOC_SAND_OUT SOC_PPC_FP_ACTION_TYPE *action_type_sw
  );

uint32
  arad_pmf_fem_output_size_get(
    SOC_SAND_IN  int            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX   *fem_ndx,
    SOC_SAND_OUT uint32            *output_size_in_bits
  );

uint32
    arad_pmf_fem_pgm_duplicate(
          SOC_SAND_IN  int                   unit,
          SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE   stage,
          SOC_SAND_IN  uint32                   pmf_pgm_from,
          SOC_SAND_IN  uint32                   pmf_pgm_to,
          SOC_SAND_IN  uint32                   mem_offset
      );


/*********************************************************************
* NAME:
 *   arad_pmf_db_fem_input_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Select the input for each Field Extraction MACRO. The FEM
 *   processes the PCL results, and extracts possible actions
 *   to be applied on the packet. INPUT SOC_SAND_IN
 *   SOC_PPD_PMF_LKP_PROFILE *lkp_profile_ndx - Lookup-Profile
 *   information (id and cycle). SOC_SAND_IN uint32 *fem_ndx -
 *   FEM (Field Extraction Macro) Index. Range: 0 - 7.
 *   (Arad-B) SOC_SAND_IN SOC_PPD_PMF_FEM_INPUT_INFO *info - FEM
 *   input parameters: the FEM-Program-Id and the FEM-Input
 *   source. RETURNS OK or Error indicationREMARKS None.
 * INPUT:
 *   SOC_SAND_IN  int                            unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_PMF_LKP_PROFILE                *lkp_profile_ndx -
 *     SOC_SAND_IN SOC_PPD_PMF_LKP_PROFILE *lkp_profile_ndx
 *   SOC_SAND_IN  uint32                            fem_ndx -
 *     SOC_SAND_IN uint32 fem_ndx
 *   SOC_SAND_IN  ARAD_PMF_FEM_INPUT_INFO             *info -
 *     FUNCTION Select the input for each Field Extraction
 *     MACRO. The FEM processes the PCL results, and extracts
 *     possible actions to be applied on the packet. INPUT
 *     SOC_SAND_IN SOC_PPD_PMF_LKP_PROFILE *lkp_profile_ndx -
 *     Lookup-Profile information (id and cycle). SOC_SAND_IN
 *     uint32 *fem_ndx - FEM (Field Extraction Macro) Index.
 *     Range: 0 - 7. (Arad-B) SOC_SAND_IN SOC_PPD_PMF_FEM_INPUT_INFO
 *     *info - FEM input parameters: the FEM-Program-Id and the
 *     FEM-Input source.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pmf_db_fem_input_set_unsafe(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32			        pmf_pgm_ndx,
    SOC_SAND_IN  uint8			        is_fes,
    SOC_SAND_IN  uint32                  fem_fes_ndx,
    SOC_SAND_IN  ARAD_PMF_FEM_INPUT_INFO   *info
  );

uint32
  arad_pmf_db_fem_input_set_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32			        pmf_pgm_ndx,
    SOC_SAND_IN  uint8			        is_fes,
    SOC_SAND_IN  uint32                  fem_fes_ndx,
    SOC_SAND_IN  ARAD_PMF_FEM_INPUT_INFO             *info
  );

uint32
  arad_pmf_db_fem_input_get_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32			        pmf_pgm_ndx,
    SOC_SAND_IN  uint8			        is_fes,
    SOC_SAND_IN  uint32                  fem_fes_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pmf_db_fem_input_set_unsafe" API.
 *     Refer to "arad_pmf_db_fem_input_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pmf_db_fem_input_get_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32			        pmf_pgm_ndx,
    SOC_SAND_IN  uint8			        is_fes,
    SOC_SAND_IN  uint32                  fem_fes_ndx,
    SOC_SAND_OUT ARAD_PMF_FEM_INPUT_INFO             *info
  );

/* Get the user-header sizes in bits */
uint32
  arad_pmf_db_fes_user_header_sizes_get(
      SOC_SAND_IN  int                  unit,
      SOC_SAND_OUT uint32                 *user_header_0_size,
      SOC_SAND_OUT uint32                 *user_header_1_size,
      SOC_SAND_OUT uint32                 *user_header_egress_pmf_offset_0,
      SOC_SAND_OUT uint32                 *user_header_egress_pmf_offset_1
  );

/* Set the FES attributes except the input */
uint32
  arad_pmf_db_fes_set_unsafe(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32			        pmf_pgm_ndx,
    SOC_SAND_IN  uint32                  fem_fes_ndx,
    SOC_SAND_IN  ARAD_PMF_FES_INPUT_INFO   *info
  );

uint32
  arad_pmf_db_fes_set_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32			        pmf_pgm_ndx,
    SOC_SAND_IN  uint32                  fem_fes_ndx,
    SOC_SAND_IN  ARAD_PMF_FES_INPUT_INFO             *info
  );

uint32
  arad_pmf_db_fes_get_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32			        pmf_pgm_ndx,
    SOC_SAND_IN  uint32                  fem_fes_ndx
  );

uint32
  arad_pmf_db_fes_get_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32			        pmf_pgm_ndx,
    SOC_SAND_IN  uint32                  fem_fes_ndx,
    SOC_SAND_OUT ARAD_PMF_FES_INPUT_INFO             *info
  );

uint32
  arad_pmf_db_fes_move_unsafe(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32			         pmf_pgm_ndx,
    SOC_SAND_IN  uint32                  from_fem_fes_ndx,
    SOC_SAND_IN  uint32                  to_fem_fes_ndx,
    SOC_SAND_IN  ARAD_PMF_FES            *fes_info
  );


/*********************************************************************
* NAME:
 *   arad_pmf_fem_select_bits_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the location of 4 bits from the FEM-key that select
 *   the performed action format for this key (configure the
 *   Select-4-bits table).
 * INPUT:
 *   SOC_SAND_IN  int                            unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx -
 *     FEM Index.
 *   SOC_SAND_IN  uint32                            fem_pgm_ndx -
 *     FEM-Program-Id. Is set with the FEM input source. Range:
 *     0 - 3. (Arad-B)
 *   SOC_SAND_IN  ARAD_PMF_FEM_SELECTED_BITS_INFO     *info -
 *     Bits to select from the FEM-Key.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pmf_fem_select_bits_set_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint32                            fem_pgm_ndx,
    SOC_SAND_IN  ARAD_PMF_FEM_SELECTED_BITS_INFO     *info
  );

uint32
  arad_pmf_fem_select_bits_set_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint32                            fem_pgm_ndx,
    SOC_SAND_IN  ARAD_PMF_FEM_SELECTED_BITS_INFO     *info
  );

uint32
  arad_pmf_fem_select_bits_get_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint32                            fem_pgm_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pmf_fem_select_bits_set_unsafe" API.
 *     Refer to "arad_pmf_fem_select_bits_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pmf_fem_select_bits_get_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint32                            fem_pgm_ndx,
    SOC_SAND_OUT ARAD_PMF_FEM_SELECTED_BITS_INFO     *info
  );

/*********************************************************************
* NAME:
 *   arad_pmf_fem_action_format_map_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the location of 4 bits from the FEM-key that select
 *   the performed action format for this key (configure the
 *   Select-4-bits table).
 * INPUT:
 *   SOC_SAND_IN  int                            unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx -
 *     FEM Index.
 *   SOC_SAND_IN  uint32                            fem_pgm_ndx -
 *     FEM-Program-Id. Is set with the FEM input source. Range:
 *     0 - 3. (Arad-B)
 *   SOC_SAND_IN  uint32                            selected_bits_ndx -
 *     Value of the selected-bits with
 *     soc_ppd_pmf_fem_select_bits_set() API. Range: 0 - 15.
 *     (Arad-B)
 *   SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO *info -
 *     Action-format-Id and the Map-Data.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pmf_fem_action_format_map_set_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint32                            fem_pgm_ndx,
    SOC_SAND_IN  uint32                            selected_bits_ndx,
    SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO *info
  );

uint32
  arad_pmf_fem_action_format_map_set_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint32                            fem_pgm_ndx,
    SOC_SAND_IN  uint32                            selected_bits_ndx,
    SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO *info
  );

uint32
  arad_pmf_fem_action_format_map_get_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint32                            fem_pgm_ndx,
    SOC_SAND_IN  uint32                            selected_bits_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pmf_fem_action_format_map_set_unsafe" API.
 *     Refer to "arad_pmf_fem_action_format_map_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pmf_fem_action_format_map_get_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint32                            fem_pgm_ndx,
    SOC_SAND_IN  uint32                            selected_bits_ndx,
    SOC_SAND_OUT ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO *info
  );

/*********************************************************************
* NAME:
 *   arad_pmf_fem_action_format_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure the format of the each action done by the
 *   Field Extraction Macro. Each FEM can perform up to four
 *   different actions.
 * INPUT:
 *   SOC_SAND_IN  int                            unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx -
 *     FEM Index.
 *   SOC_SAND_IN  uint32                            action_fomat_ndx -
 *     Action-Format-Id. Range: 0 - 3. (Arad-B)
 *   SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_INFO     *info -
 *     Parameters of the Action Format: its type, its
 *     base-value and the field extraction location.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pmf_fem_action_format_set_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint8                            fem_pgm_id,
    SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_INFO     *info
  );

uint32
  arad_pmf_fem_action_format_set_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint8                            fem_pgm_id,
    SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_INFO     *info
  );

uint32
  arad_pmf_fem_action_format_get_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint8                            fem_pgm_id
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pmf_fem_action_format_set_unsafe" API.
 *     Refer to "arad_pmf_fem_action_format_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pmf_fem_action_format_get_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint8                            fem_pgm_id,
    SOC_SAND_OUT ARAD_PMF_FEM_ACTION_FORMAT_INFO     *info
  );


uint32
  ARAD_PMF_FEM_INPUT_INFO_verify(
     SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_INPUT_INFO *info
  );

uint32
  ARAD_PMF_FES_INPUT_INFO_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FES_INPUT_INFO *info
  );

uint32
  ARAD_PMF_FEM_NDX_verify(
    SOC_SAND_IN  ARAD_PMF_FEM_NDX *info
  );

uint32
  ARAD_PMF_FEM_SELECTED_BITS_INFO_verify(
    SOC_SAND_IN  ARAD_PMF_FEM_SELECTED_BITS_INFO *info
  );

uint32
  ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO_verify(
    SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO *info
  );

uint32
  ARAD_PMF_FEM_BIT_LOC_verify(
    SOC_SAND_IN  ARAD_PMF_FEM_BIT_LOC *info
  );

uint32
  ARAD_PMF_FEM_ACTION_FORMAT_INFO_verify(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_INFO *info
  );

void
  ARAD_PMF_FEM_INPUT_INFO_clear(
    SOC_SAND_OUT ARAD_PMF_FEM_INPUT_INFO *info
  );

void
  ARAD_PMF_FES_INPUT_INFO_clear(
    SOC_SAND_OUT ARAD_PMF_FES_INPUT_INFO *info
  );

void
  ARAD_PMF_FEM_NDX_clear(
    SOC_SAND_OUT ARAD_PMF_FEM_NDX *info
  );

void
  ARAD_PMF_FEM_SELECTED_BITS_INFO_clear(
    SOC_SAND_OUT ARAD_PMF_FEM_SELECTED_BITS_INFO *info
  );

void
  ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO_clear(
    SOC_SAND_OUT ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO *info
  );

void
  ARAD_PMF_FEM_BIT_LOC_clear(
    SOC_SAND_OUT ARAD_PMF_FEM_BIT_LOC *info
  );

void
  ARAD_PMF_FEM_ACTION_FORMAT_INFO_clear(
    SOC_SAND_OUT ARAD_PMF_FEM_ACTION_FORMAT_INFO *info
  );

#if ARAD_DEBUG_IS_LVL1
const char*
  ARAD_PMF_FEM_BIT_LOC_TYPE_to_string(
    SOC_SAND_IN  ARAD_PMF_FEM_BIT_LOC_TYPE enum_val
  );

void
  ARAD_PMF_FEM_INPUT_INFO_print(
    SOC_SAND_IN  ARAD_PMF_FEM_INPUT_INFO *info
  );

void
  ARAD_PMF_FEM_NDX_print(
    SOC_SAND_IN  ARAD_PMF_FEM_NDX *info
  );

void
  ARAD_PMF_FEM_SELECTED_BITS_INFO_print(
    SOC_SAND_IN  ARAD_PMF_FEM_SELECTED_BITS_INFO *info
  );

void
  ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO_print(
    SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO *info
  );

void
  ARAD_PMF_FEM_BIT_LOC_print(
    SOC_SAND_IN  ARAD_PMF_FEM_BIT_LOC *info
  );

void
  ARAD_PMF_FEM_ACTION_FORMAT_INFO_print(
    SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_INFO *info
  );

#endif /* ARAD_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PMF_LOW_LEVEL_FEM_TAG_INCLUDED__*/
#endif



