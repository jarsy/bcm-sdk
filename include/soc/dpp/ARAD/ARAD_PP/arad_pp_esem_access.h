
/* $Id: arad_pp_esem_access.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_ESEM_INCLUDED__
/* { */
#define __ARAD_PP_ESEM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */
#define ARAD_PP_ESEM_ADDR                                      (0x00520000)
#define ARAD_PP_ESEM_KEY_SIZE                                  (2)
#define ARAD_PP_ESEM_ENTRY_SIZE                                (1)
#define ARAD_PP_ESEM_KEY_VSI_DB_ID_VAL                         (1)
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
  ARAD_PP_ESEM_GET_PROCS_PTR = ARAD_PP_PROC_DESC_BASE_ESEM_FIRST,
  ARAD_PP_ESEM_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  ARAD_PP_ESEM_ENTRY_ADD_UNSAFE,
  ARAD_PP_ESEM_ENTRY_GET_UNSAFE,
  ARAD_PP_ESEM_ENTRY_REMOVE_UNSAFE,
  ARAD_PP_ESEM_TBL_WRITE,
  ARAD_PP_ESEM_KEY_TO_BUFFER,
  ARAD_PP_ESEM_ENTRY_TO_BUFFER,
  ARAD_PP_ESEM_ENTRY_FROM_BUFFER,
  /*
   * Last element. Do no touch.
   */
  ARAD_PP_ESEM_PROCEDURE_DESC_LAST
} ARAD_PP_ESEM_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  /*
   * } Auto generated. Do not edit previous section.
   */

  ARAD_PP_ESEM_NOT_READY_ERR = ARAD_PP_ERR_DESC_BASE_ESEM_FIRST,
  ARAD_PP_ESEM_KEY_TYPE_INVALID_ERR,
  ARAD_PP_ESEM_FAIL_ON_DIFFERENT_KEY_ERR,
  ARAD_PP_ESEM_NOT_READ_TIMEOUT_ERR,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_ESEM_ERR_LAST
} ARAD_PP_ESEM_ERR;

typedef enum
{
  /*
   *	VLAN Domain x VSI
   */
  ARAD_PP_ESEM_KEY_TYPE_VD_VSI,
  /*
   *	VLAN Domain x VSI x C-VID
   */
  ARAD_PP_ESEM_KEY_TYPE_VD_VSI_CVID,
  /* 
   *  outlif x VSI 
   */
  ARAD_PP_ESEM_KEY_TYPE_LIF_VSI,
  ARAD_PP_NOF_ESEM_KEY_TYPES
}ARAD_PP_ESEM_KEY_TYPE;

typedef struct arad_pp_esem_access_vd_vsi_s
{
    uint32 vd;
    uint32 vsi;
} arad_pp_esem_access_vd_vsi_t;

typedef struct arad_pp_esem_access_vd_cvid_s
{
    uint32 vd;
    uint32 vsi;
    uint32 cvid;
} arad_pp_esem_access_vd_cvid_t;

typedef struct arad_pp_esem_access_lif_vsi_s
{
    uint32 lif; 
    uint32 vsi; 
} arad_pp_esem_access_lif_vsi_t;

typedef union
{
  arad_pp_esem_access_vd_vsi_t vd_vsi;
  arad_pp_esem_access_vd_cvid_t vd_cvid;
  arad_pp_esem_access_lif_vsi_t lif_vsi; 

} ARAD_PP_ESEM_KEY_INFO;

typedef struct
{
  ARAD_PP_ESEM_KEY_TYPE key_type;
  ARAD_PP_ESEM_KEY_INFO key_info;
} ARAD_PP_ESEM_KEY;

typedef struct
{  
  uint32 out_ac;
} ARAD_PP_ESEM_ENTRY;

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
  arad_pp_esem_buffer_to_key(
    SOC_SAND_IN  int                unit,    
    SOC_SAND_OUT uint32             *buffer,
    SOC_SAND_OUT  ARAD_PP_ESEM_KEY  *esem_key,
    SOC_SAND_OUT  uint32            *vsi_db_id
  );

uint32
  arad_pp_esem_entry_from_buffer(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint32               *buffer,
    SOC_SAND_OUT ARAD_PP_ESEM_ENTRY   *esem_entry
  );

uint32
  arad_pp_esem_entry_get_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  ARAD_PP_ESEM_KEY      *esem_key,
    SOC_SAND_OUT ARAD_PP_ESEM_ENTRY    *esem_entry,
    SOC_SAND_OUT uint8                 *found
  );

uint32
  arad_pp_esem_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  ARAD_PP_ESEM_KEY                    *svem_key,
    SOC_SAND_IN  ARAD_PP_ESEM_ENTRY                  *svem_entry,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE            *success
  );

uint32
  arad_pp_esem_entry_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  ARAD_PP_ESEM_KEY                    *svem_key,
    SOC_SAND_OUT uint8                               *success
  );

/*********************************************************************
* NAME:
 *   arad_pp_esem_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_esem module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_esem_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_esem_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_esem module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_esem_get_errs_ptr(void);

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_ESEM_INCLUDED__*/
#endif


