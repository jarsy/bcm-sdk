/*
 * $Id: TkOamMem.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkOamMem.h
 * Purpose: 
 *
 */
 
#ifndef _SOC_EA_TkOamMem_H
#define _SOC_EA_TkOamMem_H

#ifdef __cplusplus
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkOsSync.h>

#define TKOAM_MEM_DEBUG     

typedef struct TkOamMem_s { /* memory control block */
    void    * BlkMemAddr;   /* Pointer to beginning of memory partition */
    void    * BlkMemFreeList;  /* Pointer to list of free memory blocks */
    uint32    BlkMemSize;   /* Size (in bytes) of each block of memory */
    uint32    MemNBlks;     /* Total number of blocks in this partition */
    uint32    MemNFree;     /* Number of memory blocks remaining in this 
                             * partition */
    sal_sem_t BlkMemSem;    /* semphore for block memory safe operation. */
#ifdef TKOAM_MEM_DEBUG
    uint32    MemGetFailed; /* Get failed */
#endif /* TKOAM_MEM_DEBUG */
} TkOamMem;


/* TkOamMemInit - init the OAM memory management */
extern int  TkOamMemInit (uint8 pathId, uint32 Blocks, uint32 Size);

/* TkOamMemGet - Get a memory block from a partition */
extern void *TkOamMemGet (uint8 pathId);

/* TkOamMemPut - Returns a memory block to a partition */
extern int  TkOamMemPut (uint8 pathId, void *pblk);

/* TkOamMemStatus - Display the number of free memory blocks and the number of
                    used memory blocks from a memory partition. */
extern int  TkOamMemStatus (uint8 pathId);

/* TkOamMemsFree - free the memory of one memory partition */
extern void TkOamMemsFree (uint8 pathId);


#ifdef __cplusplus
}
#endif

#endif /* TKOAMMEM_H */
