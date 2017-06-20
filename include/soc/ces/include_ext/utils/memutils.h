/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 2000 Redux Communication Inc.                    */
/*                                                                       */
/* PROPRIETARY RIGHTS of Redux Communication are involved in the         */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*      memutils.h                                        1.0            */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      mem utilities in arm code                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains definitions of memory utilities for MNT and   */
/*      arm target                  .                                    */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Yehuda Mozes Redux Communication Inc                             */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      memset_word                                                      */
/*      memset_qword                                                     */
/*      memcpy_qword                                                     */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*$Revision: 1.3 $ - Visual SourceSafe revision number                     */
/*                                                                       */
/*         DATE                    REMARKS                               */
/*                                                                       */
/*      AUG-16-2001		    Created initial version 1.0                  */
/*      AUG-19-2001		      Verified version 1.0                       */
/*                                                                       */
/*************************************************************************/

#include "ag_common.h"
#ifdef BCM_CES_SDK
typedef enum AgCachedDramSize_E
{ 
	AG_CACHED_DRAM_1M,
	AG_CACHED_DRAM_2M,
	AG_CACHED_DRAM_4M,
	AG_CACHED_DRAM_8M,
	AG_CACHED_DRAM_16M,
	AG_CACHED_DRAM_32M,
	AG_CACHED_DRAM_64M
}AgCachedDramSize;

#else  /*BCM_CES_SDK*/


#include "reset/reset_ext.h"
#include "utils/gen_net.h"
#endif  /*BCM_CES_SDK*/


#include <stddef.h>

#ifndef MEMUTILS_H
#define MEMUTILS_H

#ifdef __cplusplus
extern "C"
{
#endif



AgResult  ag_calculate_cache_enum(AG_U32 n_mem_size, AgCachedDramSize* p_cache_size );

#if defined AG_MNT || defined AG_LNX
	void memutilsmnt(void *dest,void *src,unsigned len,unsigned size);
	void ag_memcpy_word(void *dest,void *src, AG_U32 n_len);
#else
    void *ag_memcpy_asm(void * dest, const void * src, size_t len);
	void memset_word(void *dest, int n, unsigned len);
	void memset_qword(void *dest,void *src,unsigned len,unsigned octet);
	void memcpy_qword(void *dest,void *src,unsigned len);
	void ag_force_bkpt(void);
	AG_U32 ag_loc_int(void);
	void ag_restore_int(AG_U32 status);
	AG_U64 ag_mull64(unsigned num1,unsigned num2);
	AG_U64 ag_int_mull64(int num1,int num2);
#endif

/* String utils */
#ifdef AG_PC_PROG
/*    #include <string.h>*/
    #define ag_stricmp(_s1,_s2)  (stricmp(_s1, _s2) ? 1: 0)
#else
    AG_U32 ag_stricmp(const AG_CHAR* s1, const AG_CHAR* s2);
#endif

/* fast memcopy of arbitrary size when source and destination are aligned on word (4 byte) boundary */
/* uses assmebler AG_MEMCPY_QWORD for chunks of 16 bytes */
void ag_memcpy_word_aligned(AG_U32* p_dest, AG_U32* p_src, AG_U32 n_bytes);

#ifdef __cplusplus
} /*end of extern "C"*/
#endif

#if defined AG_MNT || defined AG_LNX
/* in AG_MEMSET_WORD len must be a multiple of 2 */ 
	#define AG_MEMSET_WORD(dest, n, len) memutilsmnt((void *)(dest),(void *)(&n),(len),4)

/* in AG_MEMSET_4WORD len must be a multiple of 16 */ 
	#define AG_MEMSET_4WORD(dest, src, len) memutilsmnt((void *)(dest),(void *)(src),(len),16)

/* in AG_MEMSET_8WORD len must be a multiple of 16 */ 
	#define AG_MEMSET_8WORD(dest, src, len) memutilsmnt((void *)(dest),(void *)(src),(len),32)

/* in AG_MEMCPY_QWORD len must be a multiple of 16 */ 
	#define AG_MEMCPY_QWORD(dest, src, len) ag_memcpy_word((void *)(dest),(void *)(src),(len))

    #define AG_MEMCPY(dest, src, len)   ag_memcpy((void *)(dest),(void *)(src),(len))
#else 
	#define AG_MEMSET_WORD(dest, n, len) memset_word((void *)(dest), n, len)
	#define AG_MEMSET_4WORD(dest, src, len) memset_qword((void *)(dest),(void *)(src),(len),0 /*octet =0*/)
	#define AG_MEMSET_8WORD(dest, src, len) memset_qword((void *)(dest),(void *)(src),(len),1 /*octet =1*/)
	#define AG_MEMCPY_QWORD(dest, src, len) memcpy_qword((void *)(dest),(void *)(src),(len))

    #define AG_MEMCPY(dest, src, len) ag_memcpy_asm((void *)(dest),(void *)(src),(len))
    /*#define memcpy(dest, src, len)  ag_memcpy_asm((void *)(dest),(void *)(src),(len)) */
    /* ag_memcpy is defined in gen_net.h. need to remove it from there     */
    /*#define ag_memcpy(dest, src, len)  ag_memcpy_asm((void *)(dest),(void *)(src),(len)) */

#endif




#endif
