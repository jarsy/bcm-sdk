/* $Id: sand_ssr.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       dnx_sand_ssr.c
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
#include <soc/dnx/legacy/SAND/Management/sand_ssr.h>
#include <soc/dnx/legacy/SAND/Management/sand_low_level.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>

DNX_SAND_SSR_CFG_VERSION_NUM
  dnx_sand_ssr_get_cfg_version(
    DNX_SAND_IN uint32 dnx_sand_version
  )
{
  DNX_SAND_SSR_CFG_VERSION_NUM
    ssr_ver_num = DNX_SAND_SSR_CFG_VERSION_INVALID;
  /*
   * Currently, the DNX_SAND_SSR_BUFF is not changed.
   * if it would be changed in the future, the if should be
   * modified, that when the version is higher than the version
   * it modified, the DNX_SAND_SSR_CFG_VERSION_NUM value, would be new.
   */
  if(dnx_sand_version < 1776)
  {
    ssr_ver_num = DNX_SAND_SSR_CFG_VERSION_INVALID;
  }
  else if(dnx_sand_version >= 1776)
  {
    ssr_ver_num = DNX_SAND_SSR_CFG_VERSION_NUM_0;
  }

  return ssr_ver_num;
}

uint32
  dnx_sand_ssr_reload_globals_cfg_ver_0(
      DNX_SAND_IN DNX_SAND_SSR_BUFF *curr_ssr_buff
    )
{
  dnx_sand_ssr_set_big_endian(
    curr_ssr_buff->data.dnx_sand_big_endian_was_checked,
    curr_ssr_buff->data.dnx_sand_big_endian
  );

  dnx_sand_set_print_when_writing(
    curr_ssr_buff->data.dnx_sand_physical_print_when_writing,
    curr_ssr_buff->data.dnx_sand_physical_print_asic_style,
    0
  );

  return 0;
}


uint32
  dnx_sand_ssr_reload_globals_cfg_ver_1(
      DNX_SAND_IN DNX_SAND_SSR_BUFF *curr_ssr_buff
    )
{
  /*
   * Currently, there is only one active configuration version.
   */
  return 0;
}

uint32
  dnx_sand_ssr_reload_globals(
      DNX_SAND_IN DNX_SAND_SSR_BUFF *curr_ssr_buff
    )
{
  DNX_SAND_SSR_CFG_VERSION_NUM
    dnx_sand_ssr_cfg_version;

  dnx_sand_ssr_cfg_version =
    dnx_sand_ssr_get_cfg_version(curr_ssr_buff->header.dnx_sand_version);

  if(dnx_sand_ssr_cfg_version >= DNX_SAND_SSR_CFG_VERSION_NUM_0)
  {
    dnx_sand_ssr_reload_globals_cfg_ver_0(curr_ssr_buff);
  }
  if(dnx_sand_ssr_cfg_version >= DNX_SAND_SSR_CFG_VERSION_NUM_1)
  {
    dnx_sand_ssr_reload_globals_cfg_ver_1(curr_ssr_buff);
  }

  return 0;
}


uint32
  dnx_sand_ssr_save_globals(
      DNX_SAND_SSR_BUFF *curr_ssr_buff
    )
{
  uint32
    indirect_print;
  dnx_sand_ssr_get_big_endian(
    &(curr_ssr_buff->data.dnx_sand_big_endian_was_checked),
    &(curr_ssr_buff->data.dnx_sand_big_endian)
  );

  dnx_sand_get_print_when_writing(
    &(curr_ssr_buff->data.dnx_sand_physical_print_when_writing),
    &(curr_ssr_buff->data.dnx_sand_physical_print_asic_style),
    &indirect_print
  );

  return 0;
}

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
  )
{
  /* error handling */
  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_SSR_GET_VER_FROM_HEADER);
  DNX_SAND_CHECK_NULL_INPUT(header);
  DNX_SAND_CHECK_NULL_INPUT(dnx_sand_ver);

  *dnx_sand_ver = header->dnx_sand_version;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("dnx_sand_ssr_get_ver_from_header", 0, 0);
}

/*****************************************************
*NAME
*
  fap20v_ssr_load_data_from_buff
*TYPE:
*  PROC
*DATE:
*  21/02/2006
*FUNCTION:
*  distribute SSR data to all relevent module's global data structures
*INPUT:
*  DNX_SAND_DIRECT:
*    FAP20V_SSR_DATA* ssr_data - pointer to SSR data, in current version's format
*  DNX_SAND_INDIRECT:
*    None.
*OUTPUT:
*  DNX_SAND_DIRECT:
*    uint32 - dnx_sand error word
*  DNX_SAND_INDIRECT:
*    None.
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
uint32
  dnx_sand_ssr_get_size_from_header(
      DNX_SAND_IN DNX_SAND_SSR_HEADER* header,
      DNX_SAND_OUT uint32* buff_size
  )
{
  /* error handling */
  DNX_SAND_INIT_ERROR_DEFINITIONS(DNX_SAND_SSR_GET_SIZE_FROM_HEADER);
  DNX_SAND_CHECK_NULL_INPUT(header);
  DNX_SAND_CHECK_NULL_INPUT(buff_size);

  *buff_size = header->buffer_size;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("dnx_sand_ssr_get_size_from_header", 0, 0);
}
