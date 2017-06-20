/* $Id: DNX_general.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __DNX_GENERAL_INCLUDED__
/* { */
#define __DNX_GENERAL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>

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

#define DNX_ALLOC_ANY_SIZE(var, type, count,str)                       \
  do {                                                                    \
    if(var != NULL)                                                       \
    {                                                                     \
      DNX_EXIT_WITH_ERR(SOC_E_PARAM, (_DNX_MSG("Trying to allocate to a non null ptr is forbidden"))); \
    }                                                                     \
    var = (type*)soc_sand_os_malloc_any_size((count) * (uint32)sizeof(type),str); \
    if (var == NULL)                                                      \
    {                                                                     \
      DNX_EXIT_WITH_ERR(SOC_E_MEMORY, (_DNX_MSG("Failed to allocate memory")));\
    }                                                                     \
    _rv = soc_sand_os_memset(                                                 \
            var,                                                          \
            0x0,                                                          \
            (count) * (uint32)sizeof(type)                                \
          );                                                              \
    DNX_SAND_IF_ERR_EXIT(_rv);                                          \
  } while (0);


#define DNX_CLEAR(var_ptr, type, count)                                \
  do {                                                                    \
    _rv = soc_sand_os_memset(                                             \
            var_ptr,                                                      \
            0x0,                                                          \
            (count) * sizeof(type)                                        \
          );                                                              \
    DNX_IF_ERR_EXIT(_rv);                                              \
  } while (0);

#define DNX_COPY(var_dest_ptr, var_src_ptr, type, count)                \
  do {                                                                    \
    _rc = soc_sand_os_memcpy(                                             \
            var_dest_ptr,                                                 \
            var_src_ptr,                                                  \
            (count) * sizeof(type)                                        \
          );                                                              \
    DNX_IF_ERR_EXIT(_rv);                                              \
  } while (0);

#define DNX_COMP(var_ptr1, var_ptr2, type, count, is_equal_res)         \
  do {                                                                     \
    is_equal_res = SOC_SAND_NUM2BOOL_INVERSE(soc_sand_os_memcmp(                  \
            var_ptr1,                                                     \
            var_ptr2,                                                     \
            (count) * sizeof(type)                                        \
          ));                                                             \
  } while (0);

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

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_GENERAL_INCLUDED__*/
#endif



