#if !defined(_SB_WRAPPERS_H_)
#define _SB_WRAPPERS_H_
/**
 * @file sbWrappers.h
 *
 * <pre>
 * ==========================================================
 * == sbWrappers.h - Wrapper file for commonly used macros ==
 * ==========================================================
 *
 * WORKING REVISION: $Id: sbWrappers.h,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 *
 * MODULE NAME:
 *
 *     sbWrappers.h
 *
 * ABSTRACT:
 *
 *     Commonly used macros, wrapped for checked build vs. production builds
 *
 * LANGUAGE:
 *
 *     C
 *
 * AUTHORS:
 *
 *     Travis B. Sawyer
 *
 * CREATION DATE:
 *
 *     12-July-2005
 *
 * </pre>
 */

#if defined(CHECK_BUILD)
#ifndef VXWORKS
#include <assert.h>
#ifndef __KERNEL__
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#endif /* ! KERNEL */
#else
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#endif /* !VXWORKS */

#include <soc/drv.h>
#include <soc/debug.h>

#if !defined(SB_ZF_INCLUDE_CONSOLE)
#define SB_ZF_INCLUDE_CONSOLE
#endif

#define SB_ASSERT(__X)  assert(__X)

/*
#define SB_MEMSET(dest, pattern, size) memset(dest, pattern, size)
#define SB_MEMCPY(dest, src, size)     memcpy(dest, src, size)
#define SB_MEMCMP(dest, src, size)     memcmp(dest, src, size)
#define SB_STRCPY(str1, str2)          strcpy(str1, str2)
#define SB_STRNCPY(str1, str2, maxLen) strncpy(str1, str2, maxLen)
#define SB_STRCAT(str, ct)             strcat(str, ct)
#define SB_STRNCAT(str, ct, num)       strncat(str, ct, num)
#define SB_STRCMP(str1, str2)          strcmp(str1, str2)
#define SB_STRNCMP(str1, str2, maxLen) strncmp(str1, str2, maxLen)
#define SB_STRLEN(str)                 strlen(str)
#define SB_STRERROR(num)               strerror(num)
#define SB_STRTOK(str, ct)             strtok(str, ct)
*/

#define SB_MEMSET(dest, pattern, size) sal_memset(dest, pattern, size)
#define SB_MEMCPY(dest, src, size)     sal_memcpy(dest, src, size)
#define SB_MEMCMP(dest, src, size)     sal_memcmp(dest, src, size)
#define SB_STRCPY(str1, str2)          sal_strcpy(str1, str2)
#define SB_STRNCPY(str1, str2, maxLen) sal_strncpy(str1, str2, maxLen)
#define SB_STRCAT(str, ct)             strcat(str, ct)
#define SB_STRNCAT(str, ct, num)       sal_strncat(str, ct, num)
#define SB_STRCMP(str1, str2)          sal_strcmp(str1, str2)
#define SB_STRNCMP(str1, str2, maxLen) sal_strncmp(str1, str2, maxLen)
#define SB_STRLEN(str)                 sal_strlen(str)
#define SB_STRERROR(num)               sal_strerror(num)
#define SB_STRTOK(str, ct)             sal_strtok(str, ct)
#define SB_SPRINTF                     sal_sprintf
#define SB_STRSTR                      strstr

#else /* !defined(CHECK_BUILD) */
#define SB_ASSERT(__X)
#define SB_MEMSET(dest, pattern, size) memset(dest, pattern, size)
#define SB_MEMCPY(dest, src, size)     memcpy(dest, src, size)
#define SB_MEMCMP(dest, src, size)     memcmp(dest, src, size)
#define SB_STRCPY(str1, str2)          strcpy(str1, str2)
#define SB_STRNCPY(str1, str2, maxLen) strncpy(str1, str2, maxLen)
#define SB_STRCAT(str, ct)             strcat(str, ct)
#define SB_STRNCAT(str, ct, num)       strncat(str, ct, num)
#define SB_STRCMP(str1, str2)          strcmp(str1, str2)
#define SB_STRNCMP(str1, str2, maxLen) strncmp(str1, str2, maxLen)
#define SB_STRLEN(str)                 strlen(str)
#define SB_STRERROR(num)               strerror(num)
#define SB_STRTOK(str, ct)             strtok(str, ct)

#endif /* defined(CHECK_BUILD) */
#endif /* _SB_WRAPPERS_H_ */
