/* $Id: sand_header.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/SOC_SAND/Utils/include/soc_sand_header.h
*
* MODULE PREFIX:  SOC_SAND
*
* FILE DESCRIPTION:
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER

/* On MS-Windows platform this attribute is not defined. */
#ifndef __ATTRIBUTE_PACKED__
  #define __ATTRIBUTE_PACKED__
#endif

#pragma warning(disable  : 4103)
#pragma warning(disable  : 4127)
#pragma pack(push)
#pragma pack(1)

#elif defined(__GNUC__)

/* GNUC packing attribute. */
#ifndef __ATTRIBUTE_PACKED__
  #define __ATTRIBUTE_PACKED__ __attribute__ ((packed))
#endif

#elif defined(GHS)
#ifndef __ATTRIBUTE_PACKED__
  #define __ATTRIBUTE_PACKED__ __attribute__ ((__packed__))
#endif

#else

#define __ATTRIBUTE_PACKED__  __SOC_SAND_USER_DEFINED_ATTRIBUTE_PACKED__

#endif
