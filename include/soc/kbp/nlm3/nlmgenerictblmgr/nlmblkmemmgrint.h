/*
 * $Id: nlmblkmemmgrint.h,v 1.1.6.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
/*General comments
Contains the declarations that are internally used by blk memory manager.
These declarations are not exposed to the upper layer
*/

#ifndef INCLUDED_NLMBLKMEMMGRINT_H
#define INCLUDED_NLMBLKMEMMGRINT_H

#ifndef NLMPLATFORM_BCM
#include <nlmcmbasic.h>
#include "nlmgenerictblmgr.h"
#include "nlmblkmemmgr.h"
#include "nlmcmtwolevelhashtbl.h"
#include "nlmcmfreeslotmgr.h"
#include "nlmudamemmgr.h"
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/nlm3/nlmgenerictblmgr/nlmgenerictblmgr.h>
#include <soc/kbp/nlm3/nlmgenerictblmgr/nlmblkmemmgr.h>
#include <soc/kbp/common/nlmcmtwolevelhashtbl.h>
#include <soc/kbp/common/nlmcmfreeslotmgr.h>
#include <soc/kbp/nlm3/nlmgenerictblmgr/nlmudamemmgr.h>
#endif

/* 
TblsInBlk gives lists all the tables whose records are present in the block 
*/
typedef struct NlmTblsInBlk_t
{
#ifndef NLMPLATFORM_BCM    
	#include "nlmcmdbllinklistdata.h"
#else
	#include <soc/kbp/common/nlmcmdbllinklistdata.h>
#endif
    nlm_u8      m_tblId;    
    nlm_16      m_numRecs;
    
}NlmTblsInBlk_t;



/* 
RecInfo has the information about each record in a block
*/
typedef struct NlmRecInfo_t
{
    nlm_u16     m_priority; /*Priority of the record */
    nlm_u16     m_groupId;  /*Id of the group within the table*/
    nlm_u8      m_tblId;    
}NlmRecInfo_t;


/*
BlkInfo has information about each block in the chip.  
*/
typedef struct NlmBlkInfo_t
{
    nlm_u8 m_validBlk;          /* Represents whether the blk can be used for GTM applications */
    nlm_16 m_blkWidth;          /* width of each record of the block in bits 
                                    (80, 160, 320, 640)*/
    nlm_16 m_numUsedRecs;       /* Total num of used records present in the block. 
                                    Blk width cannot be changed when num of used records >= 1 */
    NlmGenericTblAssoWidth m_adWidth;   /* Ad Width for each record in the block in bits 
                                        (32, 64, 128, 256)*/

    NlmRecInfo_t* m_recInfo_p;       /* Pointer to an array having information about 
                                      each record in the block */
    NlmTblsInBlk_t* m_tblList_p;        /* pointer to list having info about tables whose entries are present 
                                    in the block */
    NlmFreeSlotMgr_t* m_freeSlotMgr_p;

    NlmUdaChunkInfo* m_udaChunkInfo_p;  /* UDA chunk info corresponding to DBA block */
    nlm_u8 m_BlksInSB;
    
    nlm_u8 m_keepSeparate;    /* keep the table seperately in block, should not be mixed with other table records*/
} NlmBlkInfo_t;


 
typedef struct NlmBlkMemMgr_t
{
    NlmCmAllocator*         m_alloc_p;              /*pointer to memory allocation fns */
    NlmBlkInfo_t*               m_blkInfo_p;            /* pointer to an array. m_blkInfo_p[0]
                                                    has the information about block 0 */
    NlmIndexChangedTblMgrCB m_indexChangedCB;   
    NlmSetBlkWidthCB        m_setBlkWidthCB;        
    NlmBlkChangedForTblCB   m_blkChangedForTblCB;
    NlmRemoveRecordCB       m_removeRecordCB;
    void*                   m_ctx_p;            /* Pointer to info the caller wants to 
                                                access in callback */
    nlm_u8                  m_nrOfDevices;  /*Number of devices in the cascade */

    NlmUdaMemMgr*           m_udaMemMgr_p; 
    nlm_u16                 m_NumOfABs;
}NlmBlkMemMgr_t;



typedef struct NlmBlkMemMgrFnArgs_t
{
    NlmGenericTbl    *genericTbl_p;
    nlm_u16          groupId;   
    nlm_u16          newRecPriority; 
    nlm_16           newRecWidth;
    NlmRecordIndex   *o_recIndex_p; 
    NlmReasonCode    *o_reason_p;
    NlmBlkMemMgr_t   *blkMemMgr_p;
    NlmBool          isShuffleDown;
}NlmBlkMemMgrFnArgs_t;


typedef struct NlmTblPvtData_t
{
    void*   m_groupIdHashTbl_p;
}NlmTblPvtData_t;




#endif
