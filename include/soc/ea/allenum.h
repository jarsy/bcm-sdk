/*
 * $Id: allenum.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        allenum.h
 * Purpose:     Enumerated types for fields, memories, registers
 */

#ifndef _SOC_EA_ALLENUM_H
#define _SOC_EA_ALLENUM_H

#ifndef EXTERN
# ifdef __cplusplus
#  define EXTERN extern "C"
# else
#  define EXTERN extern
# endif
#endif
#ifndef NUM_SOC_MEM 
#define NUM_SOC_MEM 4463    
#endif
typedef int soc_ea_mem_t;
#ifndef soc_mem_t
#define soc_mem_t soc_ea_mem_t
#endif

typedef int soc_ea_field_t;
#ifndef soc_field_t
#define soc_field_t soc_ea_field_t
#endif
#ifndef NUM_SOC_REG
#define NUM_SOC_REG 36652
#endif
typedef int soc_ea_reg_t;

#ifndef soc_reg_t
#define soc_reg_t soc_ea_reg_t
#endif

/* NOTE: 'FIELDf' is the zero value */
#define INVALID_Rf -1
#ifndef INVALIDf
#define INVALIDf INVALID_Rf
#endif

#ifndef  INVALIDr
#define INVALIDr 	-1
#endif

#ifdef BCM_TK371X_SUPPORT
#include <soc/ea/tk371x/allenum.h>
#ifndef SOC_EA_MAX_NUM_PORTS
#define SOC_EA_MAX_NUM_PORTS     11
#endif
#ifndef  SOC_EA_MAX_NUM_BLKS
#define SOC_EA_MAX_NUM_BLKS 	8
#endif
#ifndef SOC_MAX_NUM_PORTS 
#define SOC_MAX_NUM_PORTS 	(SOC_EA_MAX_NUM_PORTS)
#endif
#ifndef SOC_MAX_NUM_BLKS 
#define SOC_MAX_NUM_BLKS 	(SOC_EA_MAX_NUM_BLKS) 
#endif
#undef SOC_MAX_MEM_WORDS
#define SOC_MAX_MEM_WORDS   64
#endif

#endif /* _SOC_EA_ALLENUM_H */


