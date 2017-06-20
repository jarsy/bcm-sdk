/* $Id: jer2_arad_general.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER2_ARAD_GENERAL_INCLUDED__
/* { */
#define __JER2_ARAD_GENERAL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>


#include <soc/dnx/legacy/SAND/SAND_FM/sand_user_callback.h>

#include <soc/dnx/legacy/ARAD/arad_api_general.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */
#define JER2_ARAD_GEN_ERR_NUM_BASE            2000
#define JER2_ARAD_GEN_ERR_NUM_ALLOC           (JER2_ARAD_GEN_ERR_NUM_BASE + 0)
#define JER2_ARAD_GEN_ERR_NUM_ALLOC_ANY       (JER2_ARAD_GEN_ERR_NUM_BASE + 1)
#define JER2_ARAD_GEN_ERR_NUM_ALLOC_AND_CLEAR (JER2_ARAD_GEN_ERR_NUM_BASE + 2)
#define JER2_ARAD_GEN_ERR_NUM_ALLOC_ANY_SET   (JER2_ARAD_GEN_ERR_NUM_BASE + 3)
#define JER2_ARAD_GENERAL_EXIT_PLACE_TAKE_SEMAPHORE (JER2_ARAD_GEN_ERR_NUM_BASE + 4)
#define JER2_ARAD_GENERAL_EXIT_PLACE_GIVE_SEMAPHORE (JER2_ARAD_GEN_ERR_NUM_BASE + 5)
#define JER2_ARAD_GEN_ERR_NUM_CLEAR           (JER2_ARAD_GEN_ERR_NUM_BASE + 6)
#define JER2_ARAD_GEN_ERR_NUM_COPY            (JER2_ARAD_GEN_ERR_NUM_BASE + 7)
#define JER2_ARAD_GEN_ERR_NUM_COMP            (JER2_ARAD_GEN_ERR_NUM_BASE + 8)


#define SOC_DNX_MSG(string) string

/* } */

/*************
 * MACROS    *
 *************/
/* { */

#define JER2_ARAD_ALLOC(var, type, count,str)                                     \
  {                                                                       \
    if(var != NULL)                                                       \
    {                                                                     \
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_ALLOC_TO_NON_NULL_ERR, JER2_ARAD_GEN_ERR_NUM_ALLOC, exit); \
    }                                                                     \
    var = (type*)dnx_sand_os_malloc((count) * sizeof(type),str);                  \
    if (var == NULL)                                                      \
    {                                                                     \
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, JER2_ARAD_GEN_ERR_NUM_ALLOC, exit);  \
    }                                                                     \
    res = dnx_sand_os_memset(                                                 \
            var,                                                          \
            0x0,                                                          \
            (count) * sizeof(type)                                        \
          );                                                              \
    DNX_SAND_CHECK_FUNC_RESULT(res, JER2_ARAD_GEN_ERR_NUM_ALLOC, exit);           \
  }
#define JER2_ARAD_ALLOC_ANY_SIZE(var, type, count,str)                             \
  {                                                                       \
    if(var != NULL)                                                       \
    {                                                                     \
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_ALLOC_TO_NON_NULL_ERR, JER2_ARAD_GEN_ERR_NUM_ALLOC, exit); \
    }                                                                     \
    var = (type*)dnx_sand_os_malloc_any_size((count) * (uint32)sizeof(type),str); \
    if (var == NULL)                                                      \
    {                                                                     \
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, DNX_SAND_NULL_POINTER_ERR, exit); \
    }                                                                     \
    res = dnx_sand_os_memset(                                                 \
            var,                                                          \
            0x0,                                                          \
            (count) * (uint32)sizeof(type)                                \
          );                                                              \
    DNX_SAND_CHECK_FUNC_RESULT(res, JER2_ARAD_GEN_ERR_NUM_ALLOC_ANY_SET, exit);   \
  }

#define JER2_ARAD_CLEAR_STRUCT(var, type)                                     \
  {                                                                       \
    if (var == NULL)                                                      \
    {                                                                     \
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_MALLOC_FAIL, DNX_SAND_NULL_POINTER_ERR, exit); \
    }                                                                     \
    jer2_arad_##type##_clear(var);                                            \
  }

#define JER2_ARAD_ALLOC_AND_CLEAR_STRUCT(var, type, str)                           \
  {                                                                       \
    var = (type*)dnx_sand_os_malloc(sizeof(type), str);                            \
    JER2_ARAD_CLEAR_STRUCT(var, type);                                        \
  }

#define JER2_ARAD_FREE(var)                                                    \
  if (var != NULL)                                                        \
  {                                                                        \
    dnx_sand_os_free(var);                                                     \
    var = NULL;                                                           \
  }

#define JER2_ARAD_FREE_ANY_SIZE(var)                                           \
  if (var != NULL)                                                        \
  {                                                                        \
    dnx_sand_os_free_any_size(var);                                            \
    var = NULL;                                                           \
  }

#define JER2_ARAD_CLEAR(var_ptr, type, count)                                 \
  {                                                                       \
    res = dnx_sand_os_memset(                                                 \
            var_ptr,                                                      \
            0x0,                                                          \
            (count) * sizeof(type)                                        \
          );                                                              \
    DNX_SAND_CHECK_FUNC_RESULT(res, JER2_ARAD_GEN_ERR_NUM_CLEAR, exit);           \
  }

#define JER2_ARAD_COPY(var_dest_ptr, var_src_ptr, type, count)                \
  {                                                                       \
    res = dnx_sand_os_memcpy(                                                 \
            var_dest_ptr,                                                 \
            var_src_ptr,                                                  \
            (count) * sizeof(type)                                        \
          );                                                              \
    DNX_SAND_CHECK_FUNC_RESULT(res, JER2_ARAD_GEN_ERR_NUM_CLEAR, exit);           \
  }

#define JER2_ARAD_COMP(var_ptr1, var_ptr2, type, count, is_equal_res)         \
  {                                                                       \
    is_equal_res = DNX_SAND_NUM2BOOL_INVERSE(dnx_sand_os_memcmp(                  \
            var_ptr1,                                                     \
            var_ptr2,                                                     \
            (count) * sizeof(type)                                        \
          ));                                                             \
  }

#define JER2_ARAD_DEVICE_CHECK(unit, exit_label)                               \
    DNX_SAND_ERR_IF_ABOVE_NOF(unit, SOC_MAX_NUM_DEVICES,             \
        JER2_ARAD_DEVICE_ID_OUT_OF_RANGE_ERR, 7777, exit_label);               \

#define JER2_ARAD_BIT_TO_U32(nof_bits) (((nof_bits)+31)/32)

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

/*****************************************************
*NAME
*  jer2_arad_initialize_database
*TYPE:
*  PROC
*DATE:
*  08/14/2007
*FUNCTION:
*  This procedure initializes the database after registering the device.
*  Once called it should not be called again.
*INPUT:
*  uint32 unit
*    The device id as returned from jer2_arad_register_device
*OUTPUT:
*    None
*****************************************************/

/*****************************************************
*NAME
*  jer2_arad_sw_db_cfg_ticks_per_sec_get
*TYPE:
*  PROC
*DATE:
*  02-OCT-2007
*FUNCTION:
*  This procedure returns the Arad_ticks_per_sec.
*INPUT:
*  void.
*OUTPUT:
*    None
*****************************************************/
uint32
  jer2_arad_sw_db_cfg_ticks_per_sec_get(
    void
  );

uint8
  jer2_arad_is_multicast_id_valid(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  uint32                multicast_id
  );

uint8
  jer2_arad_is_queue_valid(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  uint32                queue
  );

uint8
  jer2_arad_is_flow_valid(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  uint32                flow
  );

/*
 *  Internal Rate to clock conversion.
 *  Used for rate configuration, e.g. IPS (IssMaxCrRate),
 *  FMC (FmcMaxCrRate), Guaranteed/Best Effort FMC (GfmcMaxCrRate/BfmcMaxCrRate)
 */
uint32
  jer2_arad_intern_rate2clock(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  uint32  rate_kbps,
    DNX_SAND_IN  uint8 is_for_ips,
    DNX_SAND_OUT uint32  *clk_interval
  );

/*
 *  Internal Rate to clock conversion.
 *  Used for rate configuration, e.g. IPS (IssMaxCrRate),
 *  FMC (FmcMaxCrRate), Guaranteed/Best Effort FMC (GfmcMaxCrRate/BfmcMaxCrRate)
 */
uint32
  jer2_arad_intern_clock2rate(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  uint32  clk_interval,
    DNX_SAND_IN  uint8 is_for_ips,
    DNX_SAND_OUT uint32  *rate_kbps
  );


/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __JER2_ARAD_GENERAL_INCLUDED__*/
#endif


