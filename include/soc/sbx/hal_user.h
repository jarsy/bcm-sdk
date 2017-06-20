#ifndef _HAL_USER_H_
#define _HAL_USER_H_
/******************************************************************************
** ===============================================
** == hal_user.h - Hardware Abstraction Layer   ==
** ===============================================
**
** WORKING REVISION: $Id: hal_user.h,v 1.10 Broadcom SDK $
**
** $Copyright: (c) 2016 Broadcom.
** Broadcom Proprietary and Confidential. All rights reserved.$
**
** MODULE NAME:
**
**     HAL
**
** ABSTRACT:
**
**     HAL macros for register access
**
**  USER should define the following macros:
**    SAND_HAL_READ_OFFS_RAW(addr,offs)        - register read function with no endian swapping
**    SAND_HAL_WRITE_OFFS_RAW(addr,offs,value) - register write function with no endian swapping
**    SAND_HAL_READ_OFFS(addr,offs)            - register read function
**    SAND_HAL_WRITE_OFFS(addr,offs,value)     - register write function
**
** LANGUAGE:
**
**     C/C++
**
** AUTHORS:
**
**     Lennart Augustsson
**
** CREATION DATE:
**
**     29-July-2004
**
******************************************************************************/


#include <soc/sbx/glue.h>

/*
 * Override the HAL macros so the chip address type can be abstract.
 * This prohibits the use of certain HAL access macros.
 */

/*****************************************************************************/
/* These macros should never be used. */
#define SAND_HAL_READ_ADDR_RAW(addr) XXX_SAND_HAL_READ_ADDR_RAW(addr)
#define SAND_HAL_WRITE_ADDR_RAW(addr,value) XXX_SAND_HAL_WRITE_ADDR_RAW(addr,value)

#define SAND_HAL_READ_ADDR(addr) XXX_SAND_HAL_READ_ADDR(addr)
#define SAND_HAL_WRITE_ADDR(addr,value) XXX_SAND_HAL_WRITE_ADDR(addr,value)
/*****************************************************************************/

/* Use raw access macros */
#define SAND_HAL_READ_OFFS_RAW(addr,offs) thin_read32_raw( (sbhandle)(addr),offs)

/* No need to conditionally compile for EASY_RELOAD or WARM_BOOT here because
 * they will resolve to 0 by the preprocessor.   The ternary check will then be
 * removed in the basic level of the optimizer
 */
#define SAND_HAL_WRITE_OFFS_RAW(addr,offs,value) \
     ((SOC_IS_RELOADING((int32)addr) || SOC_WARM_BOOT((int32)addr)) \
        ? thin_no_write( (sbhandle)(addr),offs,value)        \
        : thin_write32_raw( (sbhandle)(addr),offs,value))

#define SAND_HAL_READ_OFFS(addr,offs) thin_read32( (sbhandle)(addr),offs)

#define SAND_HAL_WRITE_OFFS(addr,offs,value)  \
     ((SOC_IS_RELOADING((int32)addr) || SOC_WARM_BOOT((int32)addr)) \
       ? thin_no_write( (sbhandle)(addr),offs,value)              \
       : thin_write32( (sbhandle)(addr),offs,value))

/* ignores the easy-reload and warm-boot states */
#define SAND_HAL_WRITE_OFFS_FORCE(addr,offs,value) thin_write32( (sbhandle)(addr),offs,value)

#endif /* _HAL_USER_H_ */
