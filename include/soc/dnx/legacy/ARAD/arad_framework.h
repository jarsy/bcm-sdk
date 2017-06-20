/* $Id: jer2_arad_framework.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER2_ARAD_FRAMEWORK_H_INCLUDED__
/* { */
#define __JER2_ARAD_FRAMEWORK_H_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_chip_defines.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
/* } */

/*************
 * DEFINES   *
 *************/
/* { */

#define JER2_ARAD_DEBUG (DNX_SAND_DEBUG)
#define JER2_ARAD_DEBUG_IS_LVL1   (JER2_ARAD_DEBUG >= DNX_SAND_DBG_LVL1)
#define JER2_ARAD_DEBUG_IS_LVL2   (JER2_ARAD_DEBUG >= DNX_SAND_DBG_LVL2)
#define JER2_ARAD_DEBUG_IS_LVL3   (JER2_ARAD_DEBUG >= DNX_SAND_DBG_LVL3)

/* Device Version managment { */
#define JER2_ARAD_REVISION_FLD_VAL_A0  0x0
#define JER2_ARAD_REVISION_FLD_VAL_A1  0x10
#define JER2_ARAD_REVISION_FLD_VAL_A2  0x30
#define JER2_ARAD_REVISION_FLD_VAL_A3  0x32
#define JER2_ARAD_REVISION_FLD_VAL_A4  0x832

/* Device Version managment } */

/*
 *    Arad device version,
 *  e.g. Arad-A, Arad-B...
 */

typedef enum
{
  JER2_ARAD_DEV_VER_A = 0, /* Assumed zero by the Arad-A driver */
  JER2_ARAD_DEV_VER_B,
  JER2_ARAD_DEV_NOV_VERS
}JER2_ARAD_DEV_VER;


/* } */

/*************
 *  MACROS   *
 *************/
/* { */


#define JER2_ARAD_ERR_IF_NULL(e_ptr_, e_err_code) \
{                                         \
  if (e_ptr_ == NULL)                     \
  {                                       \
    JER2_ARAD_SET_ERR_AND_EXIT(e_err_code)    \
  }                                       \
}

#define JER2_ARAD_INIT_ERR_DEFS                 \
  DNX_SAND_RET m_ret = DNX_SAND_OK;

#define JER2_ARAD_SET_ERR_AND_EXIT(e_err_code)  \
{                                           \
  m_ret = e_err_code;                       \
  goto exit ;                               \
}

#define JER2_ARAD_EXIT_IF_ERR(e_sand_err,e_err_code) \
{                                         \
  m_ret = dnx_sand_get_error_code_from_error_word(e_sand_err); \
  if(m_ret != 0)                          \
  {                                       \
    m_ret = e_err_code ;                  \
    goto exit ;                           \
  }                                       \
}

#define JER2_ARAD_RETURN                  \
{                                     \
  return m_ret;                       \
}

#define JER2_ARAD_STRUCT_VERIFY(type, name, exit_num, exit_place) \
  res = type##_verify(                                \
        name                                               \
      );                                                   \
DNX_SAND_CHECK_FUNC_RESULT(res, exit_num, exit_place);


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
  jer2_arad_disp_result(
    DNX_SAND_IN uint32          jer2_arad_api_result,
    DNX_SAND_IN char              *proc_name
  );

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


/* } __JER2_ARAD_FRAMEWORK_H_INCLUDED__*/
#endif


