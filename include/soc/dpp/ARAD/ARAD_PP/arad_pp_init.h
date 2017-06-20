
/* $Id: arad_pp_init.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_INIT_INCLUDED__
/* { */
#define __ARAD_PP_INIT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

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
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_INIT_GET_PROCS_PTR = ARAD_PP_PROC_DESC_BASE_INIT_FIRST,
  ARAD_PP_INIT_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  ARAD_PP_MGMT_FUNCTIONAL_INIT,
  ARAD_PP_MGMT_HW_SET_DEFAULTS,
  ARAD_PP_MGMT_INIT_SEQUENCE_PHASE1,
  ARAD_PP_MGMT_INIT_SEQUENCE_PHASE2,
  ARAD_PP_MGMT_INIT_SEQUENCE_PHASE1_UNSAFE,
  ARAD_PP_MGMT_INIT_SEQUENCE_PHASE1_VERIFY,
  ARAD_PP_MGMT_INIT_SEQUENCE_PHASE2_UNSAFE,
  ARAD_PP_MGMT_INIT_SEQUENCE_PHASE2_VERIFY,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_INIT_PROCEDURE_DESC_LAST
} ARAD_PP_INIT_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_INIT_ERR_LAST
} ARAD_PP_INIT_ERR;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  uint32 cpu_sys_port;

}ARAD_PP_INIT_PHASE1_CONF;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  
  /* int unused was added just as a workaround for autocoder parser*/ 
  int unused;
}ARAD_PP_INIT_PHASE2_CONF;


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

/*********************************************************************
* NAME:
*     arad_pp_mgmt_init_sequence_phase1_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Initialize the device
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_PP_INIT_PHASE1_CONF    *hw_adjust -
*     Contains user-defined initialization information for
*     hardware interfaces.
*  SOC_SAND_IN  uint8                 silent -
*     If TRUE, progress printing will be suppressed.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mgmt_init_sequence_phase1_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_INIT_PHASE1_CONF    *hw_adjust,
    SOC_SAND_IN  uint8                 silent
  );

uint32
  arad_pp_mgmt_init_sequence_phase1_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_INIT_PHASE1_CONF    *hw_adjust
  );

/*********************************************************************
* NAME:
*     arad_pp_mgmt_init_sequence_phase2_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Out-of-reset sequence.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_PP_INIT_PHASE2_CONF    *hw_adjust -
*     Out Of Reset configuration. Some blocks need to be set
*     out of reset before traffic can be enabled.
*  SOC_SAND_IN  uint8                 silent -
*     TRUE - Print progress messages. FALSE - Do not print
*     progress messages.
* REMARKS:
*     1. After phase 2 initialization, traffic can be enabled.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mgmt_init_sequence_phase2_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_INIT_PHASE2_CONF    *hw_adjust,
    SOC_SAND_IN  uint8                 silent
  );

uint32
  arad_pp_mgmt_init_sequence_phase2_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_INIT_PHASE2_CONF    *hw_adjust
  );

/*********************************************************************
* NAME:
 *   arad_pp_init_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_init module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_init_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_init_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_init module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_init_get_errs_ptr(void);


/*********************************************************************
* NAME:
*     arad_pp_mgmt_init_sequence_phase1
* TYPE:
*   PROC
* FUNCTION:
*     Initialize the device
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_PP_INIT_PHASE1_CONF    *hw_adjust -
*     Contains user-defined initialization information for
*     hardware interfaces.
*  SOC_SAND_IN  uint8                 silent -
*     If TRUE, progress printing will be suppressed.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mgmt_init_sequence_phase1(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_INIT_PHASE1_CONF    *hw_adjust,
    SOC_SAND_IN  uint8                 silent
  );

/*********************************************************************
* NAME:
*     arad_pp_mgmt_init_sequence_phase2
* TYPE:
*   PROC
* FUNCTION:
*     Out-of-reset sequence.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_PP_INIT_PHASE2_CONF    *hw_adjust -
*     Out Of Reset configuration. Some blocks need to be set
*     out of reset before traffic can be enabled.
*  SOC_SAND_IN  uint8                 silent -
*     TRUE - Print progress messages. FALSE - Do not print
*     progress messages.
* REMARKS:
*     1. After phase 2 initialization, traffic can be enabled.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_mgmt_init_sequence_phase2(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_INIT_PHASE2_CONF    *hw_adjust,
    SOC_SAND_IN  uint8                 silent
  );

void
  ARAD_PP_INIT_PHASE1_CONF_clear(
    SOC_SAND_OUT ARAD_PP_INIT_PHASE1_CONF *info
  );
  
void
  ARAD_PP_INIT_PHASE2_CONF_clear(
    SOC_SAND_OUT ARAD_PP_INIT_PHASE2_CONF *info
  );
  
/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_INIT_INCLUDED__*/
#endif



