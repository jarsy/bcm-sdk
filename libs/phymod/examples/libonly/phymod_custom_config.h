/*
 * $Id: phymod_custom_config.h,v 1.1.2.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * System interface definitions for Switch SDK
 */

#ifndef __PHYMOD_CUSTOM_CONFIG_H__
#define __PHYMOD_CUSTOM_CONFIG_H__

#define PHYMOD_EAGLE_SUPPORT
#define PHYMOD_FALCON_SUPPORT

#define PHYMOD_DEBUG_ERROR(stuff_)
#define PHYMOD_DEBUG_VERBOSE(stuff_)
#define PHYMOD_DIAG_OUT(stuff_)

#define PHYMOD_USLEEP   my_usleep
#define PHYMOD_SLEEP    my_sleep
#define PHYMOD_MALLOC   my_malloc
#define PHYMOD_FREE     my_free

#ifndef TOOLS_NO_SIGNED_STDINT
#define int8_t int
#define int16_t int
#define int32_t int
#endif

#define PHYMOD_CONFIG_DEFINE_INT8_T     0
#define PHYMOD_CONFIG_DEFINE_INT16_T    0
#define PHYMOD_CONFIG_DEFINE_INT32_T    0

typedef unsigned long size_t;

extern void *my_memset(void *dest,int c,size_t cnt);
extern void *my_memcpy(void *dest,const void *src,size_t cnt);

extern int my_strcmp(const char *dest,const char *src);
extern int my_strncmp(const char *dest,const char *src,size_t cnt);
extern char *my_strncpy(char *dest,const char *src,size_t cnt);
extern char *my_strchr(const char *dest,int c);
extern char *my_strstr(const char *dest,const char *src);
extern size_t my_strlen(const char *str);
extern char *my_strcpy(char *dest,const char *src);
extern char *my_strcat(char *dest,const char *src);
extern char *my_strncat(char *dest,const char *src,size_t cnt);

extern unsigned long my_strtoul(const char *s, char **end, int base);

extern int my_snprintf(char *buf, size_t bufsize, const char *fmt, ...)
    __attribute__ ((format (printf, 3, 4)));
extern int my_sprintf(char *buf, const char *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));

extern unsigned long  my_time_usecs();
extern void my_srand(unsigned int seed);
extern int my_rand();    

#define PHYMOD_MEMSET   my_memset
#define PHYMOD_MEMCPY   my_memcpy

#define PHYMOD_STRCMP   my_strcmp
#define PHYMOD_STRNCMP  my_strncmp
#define PHYMOD_STRNCPY  my_strncpy
#define PHYMOD_STRCHR   my_strchr
#define PHYMOD_STRSTR   my_strstr
#define PHYMOD_STRLEN   my_strlen
#define PHYMOD_STRCPY   my_strcpy
#define PHYMOD_STRCAT   my_strcat
#define PHYMOD_STRNCAT  my_strncat

#define PHYMOD_STRTOUL  my_strtoul

#define PHYMOD_SPRINTF  my_sprintf
#define PHYMOD_SNPRINTF my_snprintf

#define PHYMOD_TIME_USECS my_time_usecs
#define PHYMOD_SRAND      my_srand
#define PHYMOD_RAND       my_rand      

/* No need to define size_t */
#define PHYMOD_CONFIG_DEFINE_SIZE_T     0

#ifdef TOOLS_UINT32_IS_LONG
#define PHYMOD_CONFIG_TYPE_UINT32_T     unsigned long
#define PHYMOD_CONFIG_MACRO_PRIu32      "ul"
#endif

#define LOG_WARN(ls_, stuff_)
#define BSL_META_U(u_, str_)
#define BSL_LS_SOC_PHY
#endif /* __PHYMOD_CUSTOM_CONFIG_H__ */
