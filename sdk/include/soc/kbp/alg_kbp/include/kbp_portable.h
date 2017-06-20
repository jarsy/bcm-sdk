/*
 **************************************************************************************
 Copyright 2009-2017 Broadcom Corporation

 This program is the proprietary software of Broadcom Corporation and/or its licensors,
 and may only be used, duplicated, modified or distributed pursuant to the terms and
 conditions of a separate, written license agreement executed between you and
 Broadcom (an "Authorized License").Except as set forth in an Authorized License,
 Broadcom grants no license (express or implied),right to use, or waiver of any kind
 with respect to the Software, and Broadcom expressly reserves all rights in and to
 the Software and all intellectual property rights therein.
 IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 WAY,AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization, constitutes the
    valuable trade secrets of Broadcom, and you shall use all reasonable efforts to
    protect the confidentiality thereof,and to use this information only in connection
    with your use of Broadcom integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH
    ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER
    EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM
    SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
    NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION.
    YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS
    BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES
    WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE
    THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
    OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
    ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 **************************************************************************************
 */

#ifndef __KBP_PORTABLE_H
#define __KBP_PORTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * @file kbp_portable.h
 *
 * Portability-related APIs. These standard implementations for Linux-based and
 * Windows-based systems are available as part of the respective packages. Customers
 * are expected to implement the functions below on any other propriety
 * Operating Systems
 *
 * @addtogroup PORTABILITY_API
 * @{
 */

#if defined(KBP_BIG_ENDIAN) && defined(KBP_LITTLE_ENDIAN)
#error cannot define both big and little endian
#endif

/**
 * Definition of little/big endian operator inside the struct
 */

/** Little Endian */
#if defined(__i386__) || defined(__alpha__)\
      || defined(__ia64) || defined(__ia64__)\
      || defined(__amd64) || defined(__amd64__)\
      || defined(__x86_64) || defined(__x86_64__)\
      || defined(__bfin__) || defined(__arm)\
      || defined(__arm__) || defined(aarch64_) || defined(__aarch64__) || defined(KBP_LITTLE_ENDIAN)
#define __EO_32(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z, AA, BB, CC, DD, EE, FF)A; B; C; D; E; F; G; H; I; J; K; L; M; N; O; P; Q; R; S; T; U; V; W; X; Y; Z; AA; BB; CC; DD; EE; FF
#define __EO_25(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y)A; B; C; D; E; F; G; H; I; J; K; L; M; N; O; P; Q; R; S; T; U; V; W; X; Y
#define __EO_24(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X)A; B; C; D; E; F; G; H; I; J; K; L; M; N; O; P; Q; R; S; T; U; V; W; X
#define __EO_23(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W)A; B; C; D; E; F; G; H; I; J; K; L; M; N; O; P; Q; R; S; T; U; V; W
#define __EO_22(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V)A; B; C; D; E; F; G; H; I; J; K; L; M; N; O; P; Q; R; S; T; U; V
#define __EO_21(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U)A; B; C; D; E; F; G; H; I; J; K; L; M; N; O; P; Q; R; S; T; U
#define __EO_20(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T)A; B; C; D; E; F; G; H; I; J; K; L; M; N; O; P; Q; R; S; T
#define __EO_19(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S)A; B; C; D; E; F; G; H; I; J; K; L; M; N; O; P; Q; R; S
#define __EO_18(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R)A; B; C; D; E; F; G; H; I; J; K; L; M; N; O; P; Q; R
#define __EO_17(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q)A; B; C; D; E; F; G; H; I; J; K; L; M; N; O; P; Q
#define __EO_16(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P)A; B; C; D; E; F; G; H; I; J; K; L; M; N; O; P
#define __EO_14(A, B, C, D, E, F, G, H, I, J, K, L, M, N)A; B; C; D; E; F; G; H; I; J; K; L; M; N
#define __EO_13(A, B, C, D, E, F, G, H, I, J, K, L, M)A; B; C; D; E; F; G; H; I; J; K; L; M
#define __EO_12(A, B, C, D, E, F, G, H, I, J, K, L)A; B; C; D; E; F; G; H; I; J; K; L
#define __EO_11(A, B, C, D, E, F, G, H, I, J, K)A; B; C; D; E; F; G; H; I; J; K
#define __EO_10(A, B, C, D, E, F, G, H, I, J)A; B; C; D; E; F; G; H; I; J
#define __EO_9(A, B, C, D, E, F, G, H, I)A; B; C; D; E; F; G; H; I
#define __EO_8(A, B, C, D, E, F, G, H)A; B; C; D; E; F; G; H
#define __EO_7(A, B, C, D, E, F, G)A; B; C; D; E; F; G
#define __EO_6(A, B, C, D, E, F)A; B; C; D; E; F
#define __EO_5(A, B, C, D, E)A; B; C; D; E
#define __EO_4(A, B, C, D)A; B; C; D
#define __EO_3(A, B, C)A; B; C
#define __EO_2(A, B)A; B

#define __KBP_BYTESWAP_32(x) (x)
#define __KBP_BYTESWAP_64(x) (x)
#define __KBP_INPLACE_SWAP_32(x,n)
#define __KBP_SWAP_4BYTES(x,n)
#define __KBP_BYTESWAP_TWO_32B(x) (x)

#ifndef KBP_LITTLE_ENDIAN
#define KBP_LITTLE_ENDIAN
#endif

/** Big Endian */
#elif defined(__sparc) || defined(__sparc__)\
    || defined(__powerpc__) || defined(__ppc__)\
    || defined(__hpux) || defined(__hppa)\
    || defined(__s390__) || defined(__mips)\
    || defined(__mips__) || defined(KBP_BIG_ENDIAN)
#define __EO_32(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z, AA, BB, CC, DD, EE, FF)FF; EE; DD; CC; BB; AA; Z; Y; X; W; V; U; T; S; R; Q; P; O; N; M; L; K; J; I; H; G; F; E; D; C; B; A
#define __EO_25(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y)Y; X; W; V; U; T; S; R; Q; P; O; N; M; L; K; J; I; H; G; F; E; D; C; B; A
#define __EO_24(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X)X; W; V; U; T; S; R; Q; P; O; N; M; L; K; J; I; H; G; F; E; D; C; B; A
#define __EO_23(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W) W; V; U; T; S; R; Q; P; O; N; M; L; K; J; I; H; G; F; E; D; C; B; A
#define __EO_22(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V) V; U; T; S; R; Q; P; O; N; M; L; K; J; I; H; G; F; E; D; C; B; A
#define __EO_21(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U) U; T; S; R; Q; P; O; N; M; L; K; J; I; H; G; F; E; D; C; B; A
#define __EO_20(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T) T; S; R; Q; P; O; N; M; L; K; J; I; H; G; F; E; D; C; B; A
#define __EO_19(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S)S; R; Q; P; O; N; M; L; K; J; I; H; G; F; E; D; C; B; A
#define __EO_18(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R)R; Q; P; O; N; M; L; K; J; I; H; G; F; E; D; C; B; A
#define __EO_17(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q)Q; P; O; N; M; L; K; J; I; H; G; F; E; D; C; B; A
#define __EO_16(A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P)P; O; N; M; L; K; J; I; H; G; F; E; D; C; B; A
#define __EO_14(A, B, C, D, E, F, G, H, I, J, K, L, M, N)N; M; L; K; J; I; H; G; F; E; D; C; B; A
#define __EO_13(A, B, C, D, E, F, G, H, I, J, K, L, M)M; L; K; J; I; H; G; F; E; D; C; B; A
#define __EO_12(A, B, C, D, E, F, G, H, I, J, K, L)L; K; J; I; H; G; F; E; D; C; B; A
#define __EO_11(A, B, C, D, E, F, G, H, I, J, K)K; J; I; H; G; F; E; D; C; B; A
#define __EO_10(A, B, C, D, E, F, G, H, I, J)J; I; H; G; F; E; D; C; B; A
#define __EO_9(A, B, C, D, E, F, G, H, I)I; H; G; F; E; D; C; B; A
#define __EO_8(A, B, C, D, E, F, G, H)H; G; F; E; D; C; B; A
#define __EO_7(A, B, C, D, E, F, G)G; F; E; D; C; B; A
#define __EO_6(A, B, C, D, E, F)F; E; D; C; B; A
#define __EO_5(A, B, C, D, E)E; D; C; B; A
#define __EO_4(A, B, C, D)D; C; B; A
#define __EO_3(A, B, C)C; B; A
#define __EO_2(A, B)B; A

#ifndef KBP_BIG_ENDIAN
#define KBP_BIG_ENDIAN
#endif

/**
 * Definition of byte swap of 64-bit value
 */
#define __KBP_BYTESWAP_64(x)\
   ((((x) & 0xFF00000000000000ull) >> 56) |   \
    (((x) & 0x00FF000000000000ull) >> 40) |   \
    (((x) & 0x0000FF0000000000ull) >> 24) |   \
    (((x) & 0x000000FF00000000ull) >> 8)  |   \
    (((x) & 0x00000000FF000000ull) << 8)  |   \
    (((x) & 0x0000000000FF0000ull) << 24) |   \
    (((x) & 0x000000000000FF00ull) << 40) |   \
    (((x) & 0x00000000000000FFull) << 56));

/**
 *  Swap the bytes in MSB 32bit and in LSB 32bit
 */
#define __KBP_BYTESWAP_TWO_32B(x)\
   ((((x) & 0xFF00000000000000ull) >> 24) |   \
    (((x) & 0x00FF000000000000ull) >> 8)  |   \
    (((x) & 0x0000FF0000000000ull) << 8)  |   \
    (((x) & 0x000000FF00000000ull) << 24) |   \
    (((x) & 0x00000000FF000000ull) >> 24) |   \
    (((x) & 0x0000000000FF0000ull) >> 8)  |   \
    (((x) & 0x000000000000FF00ull) << 8)  |   \
    (((x) & 0x00000000000000FFull) << 24));


/**
 * Definition of byte swap of 32-bit value
 */
#define __KBP_BYTESWAP_32(x)\
   ((((x) & 0xFF000000ul) >> 24) |   \
    (((x) & 0x00FF0000ul) >> 8)  |   \
    (((x) & 0x0000FF00ul) << 8)  |   \
    (((x) & 0x000000FFul) << 24));

/* Swap the bytes in the n th 32-bit word of given structure x */
#define __KBP_INPLACE_SWAP_32(x,n)\
    (((uint32_t *)(&x))[n]) = __KBP_BYTESWAP_32((((uint32_t *)(&x))[n]))

/* Handles unaligned word swaps */
#define __KBP_SWAP_4BYTES(x,n)\
    {uint8_t tmp; tmp = x[n]; x[n] = x[n+3];x[n+3] = tmp; tmp = x[n+1]; x[n+1] = x[n+2];x[n+2] = tmp;}


#endif

/**
 * Portability structure for timers
 */
struct kbp_timeval {
    long tv_sec;                /**< Seconds */
    long tv_ns;                 /**< Nanoseconds */
};

typedef long kbp_time_t;

struct kbp_tm {
    int32_t sec;   /* Seconds */
    int32_t min;   /* Minutes */
    int32_t hour;  /* Hours */
    int32_t mday;  /* Day of the month */
    int32_t month; /* Month */
    int32_t year;  /* Number of years since 1900 */
    int32_t wday;  /* Day of the week */
    int32_t yday;  /* Day in the year */
    int32_t isdst; /* Day Light Saving time */
};

#define NLM_CAST_PTR_TO_NLM_U32(x) ((uint32_t)(uintptr_t)x)
#define NLM_CAST_NLM_U32_TO_PTR(x) (void*)((uintptr_t)x)

#ifdef WIN32
#define kbp_concat_identity(a)  a
#define kbp_concat_2(a,b)  kbp_concat_identity(a)kbp_concat_identity(b)
#define kbp_concat_3(a,b,c)    kbp_concat_identity(a)kbp_concat_identity(b)kbp_concat_identity(c)
#else
#define kbp_concat_2(a,b)      a##b
#define kbp_concat_3(a,b,c)    a##b##c
#endif

/**
 * Definition of stdout
 */
#define kbp_stdout   stdout

/**
 * Definition of stderr
 */
#define kbp_stderr   stderr

/**
 * Definition of fopen
 */
#define kbp_fopen    fopen

/**
 * Definition of EOF
 */
#define kbp_eof      (EOF)

/**
 * Implementation of memset.
 */

void *kbp_memset(void *str, int val, uint32_t size);

/**
 * Implementation of memcpy.
 */
void *kbp_memcpy(void *dest, const void *src, uint32_t size);

/**
 * Implementation of memcmp.
 */
int kbp_memcmp(const void *s1, const void *s2, uint32_t size);

/**
 * Implementation of memmove.
 */
void *kbp_memmove(void *dstp, const void *srcp, uint32_t numOfBytes);

/**
 * Implementation of malloc.
 */
void *kbp_sysmalloc(uint32_t size);

/**
 * Implementation of calloc.
 */
void *kbp_syscalloc(uint32_t nmemblk, uint32_t size);

/**
 * Implementation of free.
 */
void kbp_sysfree(void *ptr);

/**
 * Implementation of abort.
 */
void kbp_pvt_abort(void);

/**
 * Implementation of sprintf.
 */
int kbp_sprintf(char *buff, const char *fmt,...);

/**
 * Implementation of vsnprintf.
 */
int kbp_vsnprintf(char *buff, uint32_t buffsize, const char *fmt, va_list ap);

/**
 * Implementation of snprintf.
 */
int kbp_snprintf(char *buff, uint32_t buffsize, const char *fmt,...);

/**
 * Implementation of printf.
 */
int kbp_printf(const char *fmt,...);

/**
 * Implementation of fputs
 */
int kbp_fputs(const char *str, FILE *fp);

/**
 * Implementation of vprintf.
 */
int kbp_vprintf(const char *fmt, va_list arg);

/**
 * Implementation of fprintf.
 */
int kbp_fprintf(FILE * fp, const char *fmt, ...);

/**
 * Implementation of fscanf.
 */
int kbp_fscanf(FILE * fp,const char *fmt, ...);

/**
 * Implementation of fseek.
 */
int kbp_fseek(FILE * fp, uint32_t offset ,int whence);

/**
 * Windows implementation of error box.
 */

void kbp_enable_error_box(int32_t enable);

/**
 * Implementation of gettimeofday.
 */
int kbp_gettimeofday(struct kbp_timeval *tv);

/**
 * Implementation of gettimerval.
 */
int kbp_gettimerval(struct kbp_timeval * tv);

/**
 * Implementation of time.
 */
kbp_time_t kbp_time(kbp_time_t *currtime);

/**
 * Implementation of re-entrant version of localtime.
 */
struct kbp_tm *kbp_localtime(kbp_time_t *currtime, struct kbp_tm *timeinfo);

/**
 * Implementation of getenv.
 */
char *kbp_getenv(const char *name);

/**
 * Implementation of srandom.
 */
void kbp_srandom(uint32_t seed);

/**
 * Implementation of random.
 */
int kbp_random(void);

/**
 * Implementation of random_r.
 */
int kbp_random_r(uint32_t *seedp);


/**
 * Returns an array of files that end with the provided
 * file extension (suffix). If suffix is NULL, all files irrespective
 * of suffix are returned. Its the responsibility
 * of the caller to free up the returned array.
 *
 * @param location File or directory to start searching from.
 * @param suffix The file extension or NULL for all files.
 * @param num_found Number of elements in the returned array.
 *
 * @retval Array of file names with full paths, or NULL.
 */

char **kbp_get_files_recursive(const char *location, const char *suffix, uint32_t *num_found);

/**
 * Creates a directory structure if needed, opens the
 * specified file, and returns a file pointer. For example,
 * if /foo/bar/a.txt is passed in, and directories foo/bar
 * do not exist, it will create them.
 */

FILE *kbp_open_file(const char *fname);

/**
 * Portable Unix implementation of strsep() function.
 */

char *kbp_strsep(char **stringp, const char *delim);

/**
 * Implementation of fclose
 */

int kbp_fclose(FILE * fp);

/**
 * Implementation of usleep
 */

void kbp_usleep(uint32_t usecs);

/**
 *Undefined functions are
 */

#ifdef KBP_UNDEF_FUN
extern int rand_r(unsigned *);
extern unsigned int usleep(unsigned int usecs);
extern int ioctl(int, int, ...);
#endif

/**
 * @}
 */


#ifdef __cplusplus
}
#endif
#endif                          /* __KBP_PORTABLE_H */


