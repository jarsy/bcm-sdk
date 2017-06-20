/* $Id: sand_api_ssr.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       soc_sand_api_ssr.h
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

#ifndef __SOC_SAND_API_SSR_H_INCLUDED__
/* { */
#define __SOC_SAND_API_SSR_H_INCLUDED__

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Utils/sand_framework.h>

typedef struct
{
  uint32 soc_sand_version;
  uint32 buffer_size;
  uint32 tbd_1;
  uint32 tbd_2;
  uint32 tbd_3;
} __ATTRIBUTE_PACKED__ SOC_SAND_SSR_HEADER;

uint32
  soc_sand_ssr_get_buff_size(
    SOC_SAND_OUT uint32* byte_size_ptr
  );

uint32
  soc_sand_ssr_to_buff(
    SOC_SAND_OUT unsigned char* store_buff,
    SOC_SAND_IN  uint32  buff_byte_size
  );

uint32
  soc_sand_ssr_from_buff(
    SOC_SAND_IN  unsigned char* store_buff,
    SOC_SAND_IN  uint32  buff_byte_size
  );

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_SAND_API_SSR_H_INCLUDED__*/
#endif
