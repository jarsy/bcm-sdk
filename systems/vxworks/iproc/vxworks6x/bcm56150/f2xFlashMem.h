/* f2xFlashMem.h - Intel28 and AMD29-style Flash driver header */

/*
 * Copyright (c) 2000,2002-2004,2007,2010-2013 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */

/*
modification history
--------------------
02h,25jan13,tjf modifications for v02v f2xFlashMem.c
02g,28mar12,tjf updated copyright
02f,08jul11,tjf modifications for v02p f2xFlashMem.c
02e,08jul11,tjf modifications for v02o f2xFlashMem.c
02d,08jul11,tjf modifications for v02n f2xFlashMem.c
02c,05dec10,tjf modifications for v02k f2xFlashMem.c
02b,18jan07,tjf updated the copyright
02a,02dec04,tjf upped version number to match f2xFlashMem.c
01c,10oct03,tjf modifications for v01g f2xFlashMem.c
01b,16aug02,tjf modifications for v01d f2xFlashMem.c
01a,19oct00,tjf written by Ted Friedl, Wind River PS, Madison, WI
*/

#ifndef __INCf2xFlashMemh
#define __INCf2xFlashMemh

#ifdef __cplusplus
extern "C" {
#endif

#include "vxWorks.h"
#include "semLib.h"
#include "private/semLibP.h"
#include "sysLib.h"
#include "blkIo.h"

/* types */

#define F28_TYPE                28
#define F29_8BIT_TYPE           298
#define F29_16BIT_TYPE          2916
#define F29_32BIT_TYPE          2932

#define F2X_TYPE_MASK           0xfff     /* The decimal numbers above fit in
                                           * 12 bits.  This mask and its inverse
                                           * are used to separate these numbers
                                           * from the enhanced feature bits
                                           * below.
                                           */
/* enhanced features (part of type) */

#define F2X_WB32_TYPE           0x00001000
#define F2X_WB64_TYPE           0x00002000
#define F2X_WB128_TYPE          0x00003000
#define F2X_WB256_TYPE          0x00004000
#define F2X_WB512_TYPE          0x00005000
#define F2X_WB1KB_TYPE          0x00006000
#define F2X_WB2KB_TYPE          0x00007000
#define F2X_WB4KB_TYPE          0x00008000
#define F2X_WB8KB_TYPE          0x00009000
#define F2X_WB16KB_TYPE         0x0000a000
#define F2X_WB32KB_TYPE         0x0000b000
#define F2X_WB64KB_TYPE         0x0000c000

#define F2X_WB_TYPE_MASK        0x0000f000
#define F2X_WB_SIZE(type)       (0x10 << (((type) & F2X_WB_TYPE_MASK) >> 12))
#define F2X_WB_TYPE(pow)        (((pow) - 4) << 12)

/* soon to be obsolte types */

#define F28_WB32_TYPE           (F2X_WB32_TYPE | F28_TYPE)  /* was 2832 */

/* commands */

#define F2X_CMD_READ_DEVICE_ID  0x90
#define F2X_CMD_CFI_QUERY       0x98

#define F28_CMD_CLEAR_STATUS    0x50
#define F28_CMD_READ_ARRAY      0xff
#define F28_CMD_BLOCK_ERASE     0x20
#define F28_CMD_CONFIRM         0xd0
#define F28_CMD_PROGRAM         0x40
#define F28_CMD_WRITE_BUFFER    0xe8
#define F28_CMD_BLK_LOCK_SETUP  0x60
#define F28_CMD_BLK_LOCK_CLEAR  0xd0

#define F29_CMD_RESET           0xf0
#define F29_CMD_UNLOCK1         0xaa
#define F29_CMD_UNLOCK2         0x55
#define F29_CMD_PROGRAM_BUFFER  0x29
#define F29_CMD_BLOCK_ERASE     0x80
#define F29_CMD_ERASE_CONFIRM   0x30
#define F29_CMD_PROGRAM         0xa0
#define F29_CMD_WRITE_BUFFER    0x25

#define F29_OFF_RESET           0x000
#define F29_OFF_UNLOCK1         0x555
#define F29_OFF_UNLOCK2         0x2aa
#define F29_OFF_BLOCK_ERASE     0x555
#define F29_OFF_PROGRAM         0x555
#define F29_OFF_CFI_QUERY       0x55
#define F29_OFF_READ_DEVICE_ID  0x555

/* status */

#define F28_STAT_WSMS           (1 << 7)  /* write state machine status       */
                                          /* 1 = ready                        */
                                          /* 0 = busy                         */
#define F28_STAT_ESS            (1 << 6)  /* erase suspend status             */
                                          /* 1 = erase suspended              */
                                          /* 0 = erase in progress/complete   */
#define F28_STAT_ES             (1 << 5)  /* erase status                     */
                                          /* 1 = error in block erase         */
                                          /* 0 = successful block erase       */
#define F28_STAT_PS             (1 << 4)  /* program status                   */
                                          /* 1 = error in programming         */
                                          /* 0 = successful programming       */
#define F28_STAT_VPPS           (1 << 3)  /* Vpp status                       */
                                          /* 1 = Vpp low, operation abort     */
                                          /* 0 = Vpp OK                       */
#define F28_STAT_PSS            (1 << 2)  /* program suspend status           */
                                          /* 1 = program suspended            */
                                          /* 0 = program in progress/complete */
#define F28_STAT_BLS            (1 << 1)  /* block lock status                */
                                          /* 1 = prog/erase on a locked       */
                                          /*     block; operation aborted     */
                                          /* 0 = No operation to locked block */
#define F28_STAT_R              (1 << 0)  /* reserved                         */


#define F29_STAT_DQ7            (1 << 7)  /* data polling bit                 */
                                          /* data = success                   */
                                          /* !data = busy or error            */
#define F29_STAT_DQ5            (1 << 5)  /* program/erase status             */
                                          /* 1 = error in programming/erasure */
                                          /* 0 = successful program/erasure   */

#define F29_STATUS_OK           0
#define F29_STATUS_BUSY         1
#define F29_STATUS_ERROR        2

/* driver defines */

#define F2X_MAX_REGION_WIDTH    8         /* bytes */
#define F2X_TIMEOUT             15        /* seconds */

typedef int                     F29_STATUS;

/* driver structures */

typedef struct                  /* F2X_RDESC */
    {
    /* user input */

    char *      baseAdrs;       /* base address of flash region */
    int         regionWidth;    /* width of flash region in bytes */
    int         regionSize;     /* entire size of flash region in bytes */
    int         chipWidth;      /* width of single flash part in bytes */
    int         chipBlockSize;  /* block size of a single flash chip in bytes */
    int         writeWidth;     /* width of a region write in bytes */
    int         type;           /* F2x_xxx_TYPE - see f2xFlashMem.h */
    int         regionWBSize;   /* region write buffer size in bytes */
    VOIDFUNCPTR pageFunc;       /* page function (or NULL) */
    int         pageId;         /* argument to page function (or NONE) */

    /* f28 command words */

    char        cmdClearStatus[F2X_MAX_REGION_WIDTH];
    char        cmdReadArray[F2X_MAX_REGION_WIDTH];

    /* f29 command words */

    char        cmdReset[F2X_MAX_REGION_WIDTH];
    char        cmdUnlock1[F2X_MAX_REGION_WIDTH];
    char        cmdUnlock2[F2X_MAX_REGION_WIDTH];
    char        cmdProgramBuffer[F2X_MAX_REGION_WIDTH];

    /* common command words */

    char        cmdBlockErase[F2X_MAX_REGION_WIDTH];
    char        cmdConfirm[F2X_MAX_REGION_WIDTH];
    char        cmdProgram[F2X_MAX_REGION_WIDTH];
    char        cmdWriteBuffer[F2X_MAX_REGION_WIDTH];

    /* f29 command offsets */

    int         offReset;
    int         offUnlock1;
    int         offUnlock2;
    int         offBlockErase;
    int         offProgram;

    /* block cache for f2xRegionSet() and f2xRegionCacheSet() */

    char *      pCache;         /* block cache for f2xRegionSet() */
    int         cacheRegionOffset;   /* region offset corresponding to pBlock */
    BOOL        cacheIsDirty;   /* TRUE when block cache is dirty */
    BOOL        cacheRegionIsErased; /* TRUE when underlying block is erased */

    /* control */

    FUNCPTR     programRtn;     /* programming routine */
    FUNCPTR     blockEraseRtn;  /* block erase routine */
    SEMAPHORE   semaphore;      /* storage for semaphore (may be shared) */
    SEM_ID      semId;          /* serializes read/write/erase operations */
    } F2X_RDESC;

/* object references */

typedef F2X_RDESC * F2X_RID;
typedef void *      F2X_GID;    /* Note: This file could be included by a driver
                                 * released "object only."  F2X_GID is
                                 * therefore set to "void *" because of its
                                 * dependence on the BSP define F2X_MAX_REGIONS.
                                 */

typedef struct                  /* F2X_GREQ - group request structure */
    {
    F2X_GID *   pGid;           /* memory location for resultant group ID */
    int         offset;         /* requested offset of group */
    int         groupSize;      /* reqeusted size of group */
    } F2X_GREQ;

/* globals */

IMPORT F2X_GID f2xSysFlashGid;
IMPORT F2X_GID f2xSysNvRamGid;
IMPORT F2X_GID f2xBootromGid;

#if defined(__STDC__) || defined(__cplusplus)

IMPORT F2X_RID f2xRegionCreate (char * baseAdrs, int regionWidth,
                                int regionSize, int chipWidth,
                                int chipBlockSize, int writeWidth,
                                int type, F2X_RID dependRid,
                                VOIDFUNCPTR pageFunc, int pageId);

IMPORT F2X_GID f2xGroupCreate (F2X_RID rid);
IMPORT STATUS  f2xGroupAdd (F2X_GID gid, F2X_RID rid);

IMPORT STATUS f2xGroupSet (F2X_GID id, char * pSrc, int nbytes, int offset);
IMPORT STATUS f2xGroupGet (F2X_GID id, char * pDst, int nbytes, int offset);
IMPORT STATUS f2xGroupErase (F2X_GID id);
IMPORT int    f2xGroupSize (F2X_GID id);

IMPORT STATUS f2xGroupFileSet (F2X_GID id, char * fileName, int offset);

IMPORT STATUS f2xGroupProgram (F2X_GID id, char * pSrc, int nbytes, int offset);
IMPORT char * f2xGroupMap (F2X_GID id, int offset);
IMPORT STATUS f2xGroupBlockErase (F2X_GID id, int offset);
IMPORT int    f2xGroupBlockSize (F2X_GID id);

IMPORT STATUS f2xGroupCachedSet (F2X_GID id, char * pSrc, int nbytes,
                                 int offset);
IMPORT STATUS f2xGroupCacheFlush (F2X_GID id);

IMPORT BLK_DEV * f2xBlkDevCreate (F2X_GID id, int bytesPerBlk);

IMPORT STATUS f2xCfiGroupsCreate (char * baseAdrs, int busWidth, int numGReq,
                                  F2X_GREQ greqArray[], int * pSize,
                                  BOOL verbose);
IMPORT int    f2xCfiSize (char * baseAdrs, int busWidth);
IMPORT STATUS f2xCfiShow (char * baseAdrs, int busWidth);

IMPORT STATUS sysFlashAdd (F2X_RID rid);

IMPORT STATUS sysFlashSet (char * pSrc, int nbytes, int offset);
IMPORT STATUS sysFlashGet (char * pDst, int nbytes, int offset);
IMPORT STATUS sysFlashErase (void);
IMPORT int    sysFlashSize (void);

IMPORT STATUS sysNvRamAdd (F2X_RID rid);

IMPORT STATUS f2xBootromAdd (F2X_RID rid);
IMPORT STATUS f2xBootromSet (char * fileName, int offset);

#else /* __STDC__ */

IMPORT F2X_RID f2xRegionCreate();
IMPORT F2X_GID f2xGroupCreate();
IMPORT STATUS  f2xGroupAdd();

IMPORT STATUS f2xGroupSet();
IMPORT STATUS f2xGroupGet();
IMPORT STATUS f2xGroupErase();
IMPORT int    f2xGroupSize();

IMPORT STATUS f2xGroupProgram();
IMPORT char * f2xGroupMap();
IMPORT STATUS f2xGroupBlockErase();
IMPORT int    f2xGroupBlockSize();

IMPORT STATUS f2xGroupCachedSet();
IMPORT STATUS f2xGroupCacheFlush();

IMPORT BLK_DEV * f2xBlkDevCreate();

IMPORT STATUS f2xCfiGroupsCreate();
IMPORT int    f2xCfiSize();
IMPORT STATUS f2xCfiShow();

IMPORT STATUS sysFlashAdd();

IMPORT STATUS sysFlashSet();
IMPORT STATUS sysFlashGet();
IMPORT STATUS sysFlashErase();
IMPORT int    sysFlashSize();

IMPORT STATUS sysNvRamAdd();

IMPORT STATUS f2xBootromAdd();
IMPORT STATUS f2xBootromSet();

#endif /* __STDC__ */

#ifdef __cplusplus
}
#endif

#endif /* __INCf2xFlashMemh */
