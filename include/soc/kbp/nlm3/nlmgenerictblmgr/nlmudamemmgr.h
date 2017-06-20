/*
 * $Id: nlmudamemmgr.h,v 1.1.6.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
/*General comments
Contains the declarations that are used by uda memory manager.*/

#ifndef INCLUDED_NLMUDAMEMMGR_H
#define INCLUDED_NLMUDAMEMMGR_H

#ifndef NLMPLATFORM_BCM
#include <nlmcmbasic.h>
#include <nlmcmallocator.h>
#include <nlmerrorcodes.h>
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmcmallocator.h>
#include <soc/kbp/common/nlmerrorcodes.h>
#endif

/*
Structure for implementing a doubly link list of tables searched
in parallel
*/
typedef struct NlmPsTblList
{
#ifndef NLMPLATFORM_BCM    
	#include <nlmcmdbllinklistdata.h>
#else
	#include <soc/kbp/common/nlmcmdbllinklistdata.h>
#endif    
	nlm_u8  m_tblId;

}NlmPsTblList;


/* Associated Data lengths  
enum for permissible width of associated data. Table entry can have 
associated width of 32 bit, 64 bit, 128 bit or 256 bit.
*/
typedef enum NlmTblAssoDataWidth
{
    NLM_TBL_ADLEN_ZERO,
    NLM_TBL_ADLEN_32B,
    NLM_TBL_ADLEN_64B,
    NLM_TBL_ADLEN_128B,
    NLM_TBL_ADLEN_256B,

    NLM_TBL_ADLEN_END

} NlmTblAssoDataWidth;


/* 
It gives lists all the tables whose records are present in the block 
*/
typedef  struct NlmUdaSbTblsInfo_t
{
#ifndef NLMPLATFORM_BCM    
	#include <nlmcmdbllinklistdata.h>
#else
	#include <soc/kbp/common/nlmcmdbllinklistdata.h>
#endif    
    nlm_u8      m_tblId;    
    nlm_u32         m_numOfAllocChunks;
    
}NlmUdaSbTblsInfo;


/* Contains list of UDA chunk info like start row, end row, sb num and width  */
typedef struct NlmUdaChunkInfo_t
{
#ifndef NLMPLATFORM_BCM    
	#include <nlmcmdbllinklistdata.h>
#else
	#include <soc/kbp/common/nlmcmdbllinklistdata.h>
#endif
    nlm_u16     m_sbNum; 
    nlm_u16         m_startRow;
    nlm_u16         m_endRow;

    NlmTblAssoDataWidth m_width;
            
}NlmUdaChunkInfo;

/* Contains UDA SB info like allocated chunks, free chunks, tables present, etc*/
typedef struct NlmUdaSbInfo_t
{

    NlmBool m_isNewSb;
    nlm_u32         m_numOfAllocChunks;
    
    NlmUdaChunkInfo* m_occupiedChunkInfo_p;  
    NlmUdaChunkInfo* m_freeChunkInfo_p;       

    NlmUdaSbTblsInfo* m_tblList_p;      /* pointer to list having info about tables whose entries are present 
                                        in the UDA SB */

}NlmUdaSbInfo;


/* UDA Memory Manager pointer */
typedef struct NlmUdaMemMgr_t
{
    nlm_u16     m_startSb;
    nlm_u16     m_endSb;

    NlmCmAllocator  *m_alloc_p;

    NlmUdaSbInfo *m_sbInfo_p;

}NlmUdaMemMgr;

/* Initializes UDA Memory Manager with the given SBs */
NlmUdaMemMgr* 
NlmUdaMemMgr__Init(
                NlmCmAllocator          *alloc_p,
                nlm_u16                 startSb,
                nlm_u16                 numOfSb,    
                NlmReasonCode           *o_reason_p);



/* Allocates a UDA chunk of given width and size for a given table such that 
allocated chunk will not have any parallel search dependency */
NlmErrNum_t 
NlmUdaMemMgr__AllocateUdaChunk(
            NlmUdaMemMgr*   self_p, 
            nlm_u8      tableId,
            NlmPsTblList    *psTblList_p,
            nlm_u16         chunkSize,
            nlm_u16         chunkWidth,     
            NlmUdaChunkInfo**    chunkInfo_pp,
            NlmReasonCode   *o_reason_p);

/* Update UDA chunk info and remove the given table from the associated UDA SB.
 If freeChunkAlso is set then free the chucnk as it is not used by any other table  */
 
NlmErrNum_t 
 NlmUdaMemMgr__UpdateUdaChunkInfo(
            NlmUdaMemMgr*   self_p, 
            nlm_u8      tableId,
            NlmUdaChunkInfo*    chunkInfo_p,
            NlmBool            freeChunkAlso,
            NlmReasonCode   *o_reason_p);


/* Destroy UDA Memory Manager  */
void 
NlmUdaMemMgr__Destroy(NlmUdaMemMgr* self_p);



#endif /* INCLUDED_NLMUDAMEMMGR_H */
