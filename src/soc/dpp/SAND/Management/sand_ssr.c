/* $Id: sand_ssr.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       soc_sand_ssr.c
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
#include <soc/dpp/SAND/Management/sand_ssr.h>
#include <soc/dpp/SAND/Management/sand_low_level.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

SOC_SAND_SSR_CFG_VERSION_NUM
  soc_sand_ssr_get_cfg_version(
    SOC_SAND_IN uint32 soc_sand_version
  )
{
  SOC_SAND_SSR_CFG_VERSION_NUM
    ssr_ver_num = SOC_SAND_SSR_CFG_VERSION_INVALID;
  /*
   * Currently, the SOC_SAND_SSR_BUFF is not changed.
   * if it would be changed in the future, the if should be
   * modified, that when the version is higher than the version
   * it modified, the SOC_SAND_SSR_CFG_VERSION_NUM value, would be new.
   */
  if(soc_sand_version < 1776)
  {
    ssr_ver_num = SOC_SAND_SSR_CFG_VERSION_INVALID;
  }
  else if(soc_sand_version >= 1776)
  {
    ssr_ver_num = SOC_SAND_SSR_CFG_VERSION_NUM_0;
  }

  return ssr_ver_num;
}

uint32
  soc_sand_ssr_reload_globals_cfg_ver_0(
      SOC_SAND_IN SOC_SAND_SSR_BUFF *curr_ssr_buff
    )
{
  soc_sand_ssr_set_big_endian(
    curr_ssr_buff->data.soc_sand_big_endian_was_checked,
    curr_ssr_buff->data.soc_sand_big_endian
  );

  soc_sand_set_print_when_writing(
    curr_ssr_buff->data.soc_sand_physical_print_when_writing,
    curr_ssr_buff->data.soc_sand_physical_print_asic_style,
    0
  );

  return 0;
}


uint32
  soc_sand_ssr_reload_globals_cfg_ver_1(
      SOC_SAND_IN SOC_SAND_SSR_BUFF *curr_ssr_buff
    )
{
  /*
   * Currently, there is only one active configuration version.
   */
  return 0;
}

uint32
  soc_sand_ssr_reload_globals(
      SOC_SAND_IN SOC_SAND_SSR_BUFF *curr_ssr_buff
    )
{
  SOC_SAND_SSR_CFG_VERSION_NUM
    soc_sand_ssr_cfg_version;

  soc_sand_ssr_cfg_version =
    soc_sand_ssr_get_cfg_version(curr_ssr_buff->header.soc_sand_version);

  if(soc_sand_ssr_cfg_version >= SOC_SAND_SSR_CFG_VERSION_NUM_0)
  {
    soc_sand_ssr_reload_globals_cfg_ver_0(curr_ssr_buff);
  }
  if(soc_sand_ssr_cfg_version >= SOC_SAND_SSR_CFG_VERSION_NUM_1)
  {
    soc_sand_ssr_reload_globals_cfg_ver_1(curr_ssr_buff);
  }

  return 0;
}


uint32
  soc_sand_ssr_save_globals(
      SOC_SAND_SSR_BUFF *curr_ssr_buff
    )
{
  uint32
    indirect_print;
  soc_sand_ssr_get_big_endian(
    &(curr_ssr_buff->data.soc_sand_big_endian_was_checked),
    &(curr_ssr_buff->data.soc_sand_big_endian)
  );

  soc_sand_get_print_when_writing(
    &(curr_ssr_buff->data.soc_sand_physical_print_when_writing),
    &(curr_ssr_buff->data.soc_sand_physical_print_asic_style),
    &indirect_print
  );

  return 0;
}

/*****************************************************
*NAME
*  soc_sand_ssr_get_ver_from_header
*TYPE:
*  PROC
*DATE:
*  22/02/2006
*FUNCTION:
*  gets soc_sand version number given a pointer to an SSR header
*INPUT:
*  SOC_SAND_DIRECT:
*    SOC_SAND_SSR_HEADER* header - pointer to an SSR header
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    uint32 - soc_sand error word
*  SOC_SAND_INDIRECT:
*    uint32* soc_sand_ver  - the soc_sand version extracted from SSR buffer header
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
uint32
  soc_sand_ssr_get_ver_from_header(
      SOC_SAND_IN SOC_SAND_SSR_HEADER* header,
      SOC_SAND_OUT uint32* soc_sand_ver
  )
{
  /* error handling */
  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_SSR_GET_VER_FROM_HEADER);
  SOC_SAND_CHECK_NULL_INPUT(header);
  SOC_SAND_CHECK_NULL_INPUT(soc_sand_ver);

  *soc_sand_ver = header->soc_sand_version;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("soc_sand_ssr_get_ver_from_header", 0, 0);
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
*  SOC_SAND_DIRECT:
*    FAP20V_SSR_DATA* ssr_data - pointer to SSR data, in current version's format
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    uint32 - soc_sand error word
*  SOC_SAND_INDIRECT:
*    None.
*REMARKS:
*    None.
*SEE ALSO:
*****************************************************/
uint32
  soc_sand_ssr_get_size_from_header(
      SOC_SAND_IN SOC_SAND_SSR_HEADER* header,
      SOC_SAND_OUT uint32* buff_size
  )
{
  /* error handling */
  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_SAND_SSR_GET_SIZE_FROM_HEADER);
  SOC_SAND_CHECK_NULL_INPUT(header);
  SOC_SAND_CHECK_NULL_INPUT(buff_size);

  *buff_size = header->buffer_size;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("soc_sand_ssr_get_size_from_header", 0, 0);
}
