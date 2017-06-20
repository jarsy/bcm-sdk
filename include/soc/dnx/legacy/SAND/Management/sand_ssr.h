/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       dnx_sand_ssr.h
*
* AUTHOR:         Dune (U.C.)
*
* FILE DESCRIPTION:
*
* REMARKS:
*
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
*******************************************************************/

#ifndef __DNX_SAND_SSR_H_INCLUDED__
/* { */
#define __DNX_SAND_SSR_H_INCLUDED__

#ifdef  __cplusplus
extern "C" {
#endif

#include <soc/dnx/legacy/SAND/Management/sand_api_ssr.h>

typedef enum{
  DNX_SAND_SSR_CFG_VERSION_INVALID = 0,
  DNX_SAND_SSR_CFG_VERSION_NUM_0 = 1,
  DNX_SAND_SSR_CFG_VERSION_NUM_1 = 2
}DNX_SAND_SSR_CFG_VERSION_NUM;


typedef struct
{
  uint32 dnx_sand_big_endian_was_checked;
  uint32 dnx_sand_big_endian;
  uint32 dnx_sand_physical_print_when_writing;
  uint32 dnx_sand_physical_print_asic_style;
}DNX_SAND_SSR_DATA;

typedef struct
{
  DNX_SAND_SSR_HEADER header;
  DNX_SAND_SSR_DATA   data;
}DNX_SAND_SSR_BUFF;

uint32
  dnx_sand_ssr_reload_globals(
      DNX_SAND_IN DNX_SAND_SSR_BUFF *curr_ssr_buff
    );
uint32
  dnx_sand_ssr_save_globals(
      DNX_SAND_SSR_BUFF *curr_ssr_buff
    );

/*****************************************************
*NAME
*  dnx_sand_ssr_get_ver_from_header
*TYPE:
*  PROC
*DATE:
*  22/02/2006
*FUNCTION:
*  gets dnx_sand version number given a pointer to an SSR header
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_SSR_HEADER* header - pointer to an SSR header
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 - dnx_sand error word
*  DNX_SAND_INDIRECT:
*    uint32* dnx_sand_ver  - the dnx_sand version extracted from SSR buffer header
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_ssr_get_ver_from_header(
      DNX_SAND_IN DNX_SAND_SSR_HEADER* header,
      DNX_SAND_OUT uint32* dnx_sand_ver
  );

/*****************************************************
*NAME
*  dnx_sand_ssr_get_ver_from_header
*TYPE:
*  PROC
*DATE:
*  22/02/2006
*FUNCTION:
*  gets SSR buffer size given a pointer to an SSR header
*INPUT:
*  DNX_SAND_DIRECT:
*    DNX_SAND_SSR_HEADER* header - pointer to an SSR header
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 - dnx_sand error word
*  DNX_SAND_INDIRECT:
*    uint32* buff_size  - the size of the SSR buffer, extracted from SSR buffer header
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_ssr_get_size_from_header(
      DNX_SAND_IN DNX_SAND_SSR_HEADER* header,
      DNX_SAND_OUT uint32* buff_size
  );

#ifdef  __cplusplus
}
#endif

/* } __DNX_SAND_SSR_H_INCLUDED__*/
#endif
