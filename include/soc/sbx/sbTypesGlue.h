
/*
 * $Id: sbTypesGlue.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 * Basic types on which all Sandburst code depends
 */

#ifndef _SAND_TYPES_GLUE_H_
#define _SAND_TYPES_GLUE_H_

#ifdef VXWORKS
/* Needed to fix compiler error in vxwork target for types redefinition */
#include "types/vxTypesOld.h"
#else
#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <sys/types.h>
#endif
#endif

#include <sal/types.h>
#include <sal/core/libc.h>

#ifndef soc_htonl
#if defined(LE_HOST)
extern unsigned int _shr_swap32(unsigned int val);
#define	soc_htonl(_l)	_shr_swap32(_l)
#define soc_ntohl(_l)   _shr_swap32(_l)
#else /* BE_HOST */
#define	soc_htonl(_l)	(_l)
#define soc_ntohl(_l)   (_l)
#endif /* BE_HOST */
#endif

#ifdef UINT
#undef UINT
#endif
#define UINT uint32
#ifdef ULLONG
#undef ULLONG
#endif
#define ULLONG uint64
#ifdef uint
#undef uint
#endif
#define uint UINT

typedef uint8 sbBool_t;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define SAND_CHIP_USER_INT_TYPES
#include "sbStatus.h"


#ifndef CHIP_USER_SYNC_TOKEN_TYPE
#define CHIP_USER_SYNC_TOKEN_TYPE
typedef int sandCUSyncToken_t;
typedef sandCUSyncToken_t sbSyncToken_t;
#endif

/**
 * The datatype defines the argument which will be returned along with
 * status and type from the callback funtions. For SMAC againg this would
 * return the 8-byte machHandle (Vlan, SMAC) 
 */
typedef uint64 sbMacEntryHandle_t[2], *sbMacEntryHandle_p_t;
typedef union 
{
    sbMacEntryHandle_t macHandle;     /**< for OLD_{SMAC,DMAC} */
    uint32           HbaRecord[32]; /**< For Unloader Isr    */
}sbSpecialArgs_t;

#ifndef _SB_CHIP_T_
#define _SB_CHIP_T_
typedef enum sb_chip_e {
  SB_INVALID = 0,
  SB_IFE_1000,
  SB_EFE_1000,
  SB_QE_1000,
  SB_BME_1600,
  SB_SE_1600,
  SB_PLX_9656,
  SB_BME_3200,
  SB_QE_2000,
  SB_GLUE_FPGA,
  SB_DMA_MEMORY,
  SB_FE_2000
} sb_chip_t; 
#endif

typedef enum sbFeMallocType_e {
  SB_ALLOC_INTERNAL,
  SB_ALLOC_CLS_DMA,
  SB_ALLOC_L2_DMA,
  SB_ALLOC_OTHER_DMA,
  SB_ALLOC_CLS_TO_L2,
  SB_ALLOC_CLS_TO_IPV4,
  SB_ALLOC_IPV4_DMA,
  SB_ALLOC_LPM_DMA,
  SB_ALLOC_COUNTERS
} sbFeMallocType_t;

typedef struct sbDmaMemoryHandle_s {
  void *handle;
} sbDmaMemoryHandle_t;

typedef uint32 sbreg;		/* register type */
struct sbhandle_s;
typedef struct sbhandle_s *sbhandle; /* handle to device */

/* for backwards compatibility, old apps should be updated to use sbhandle */
typedef struct sbhandle_s *sbregshandle;

typedef enum thin_bus_e {
    THIN_BUS_LE,
    THIN_BUS_BE
} thin_bus_t;

/**
 * Description goes here
 */
typedef struct sbFeAsyncCallbackArgument_s {
  void *clientData;                  /**< Description goes here */
  int type;         /**< Description goes here */
  sbStatus_t status;            /**< Description goes here */
  sbSpecialArgs_t special_args; /**< Description goes here */
} sbFeAsyncCallbackArgument_t, *sbFeAsyncCallbackArgument_p_t;

typedef void (*sbFeAsyncCallback_f_t) 
     (sbFeAsyncCallbackArgument_p_t arg);

typedef void (*sbFeInitAsyncCallback_f_t)
                   (void *initId, sbStatus_t status);


typedef sbregshandle sandCUDevAddr_t;

typedef uint32 sbFeDmaHostBusAddress_t;

/**
 * ELib FLib MVT Access Semaphore Key.
 */
#define SB_ELIB_FLIB_SEM_ACCESS_KEY  0xF11BE11B;
/***
 * Elib internal counter fetch Semaphore Key.
 */
#define SB_ELIB_COUNTER_GET_SEM_ACCESS_KEY  0xE5E5E11B;

#ifdef SAND_HTONL
#undef SAND_HTONL
#endif
#ifdef SAND_NTOHL
#undef SAND_NTOHL
#endif
#define SAND_HTONL(x) soc_htonl(x)
#define SAND_NTOHL(x) soc_ntohl(x)

#endif
