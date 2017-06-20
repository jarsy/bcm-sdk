/*
 * $Id: nlmudamemmgr.c,v 1.1.6.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#include "nlmarch.h"
#include "nlmcmdbllinklist.h"

#include "nlmudamemmgr.h"


/* Free the UDA Sb table info  */
void 
NlmUdaMemMgr__pvt_DestroyNodeTbl(
    NlmCmDblLinkList* node_p,  
    void* alloc_p
    )
{
    NlmCmAllocator__free((NlmCmAllocator*)alloc_p, (NlmUdaSbTblsInfo*) node_p);
}

/* Remove UDA Sb table info from link list and free it  */
void 
NlmUdaMemMgr__pvt_RemoveTblsInUdaSb(
    NlmCmAllocator* alloc_p,
    NlmUdaSbTblsInfo* tblInBlk_p
    )
{
    NlmCmDblLinkList__Remove((NlmCmDblLinkList*)tblInBlk_p,
                            NlmUdaMemMgr__pvt_DestroyNodeTbl,
                            alloc_p); 
}

/* Free UDA Chunk info if alloc_p is given */
void 
NlmUdaMemMgr__pvt_DestroyUdaChunkInfo(
    NlmCmDblLinkList* node_p,  
    void* alloc_p
    )
{
    if(alloc_p)
        NlmCmAllocator__free((NlmCmAllocator*)alloc_p, (NlmUdaChunkInfo*) node_p);
}


/* Remove the UDA chnk info from link list and free it. If doNotDestroy is set then 
no need of freeing it as this node will be used by caller */
void 
NlmUdaMemMgr__pvt_RemoveUdaChunkInfo(
    NlmCmAllocator* alloc_p,
    NlmUdaChunkInfo* udaChunkInfo_p, 
    NlmBool doNotDestroy
    )
{
    /* Check if we do not want to destroy chunk info */ 
    if(doNotDestroy == NlmTrue)
        alloc_p = NULL;
    
    NlmCmDblLinkList__Remove((NlmCmDblLinkList*)udaChunkInfo_p,
                            NlmUdaMemMgr__pvt_DestroyUdaChunkInfo,
                            alloc_p);   
}

/* Find given table in a SB*/
NlmBool 
NlmUdaMemMgr__pvt_FindTblInSb(
    NlmUdaSbInfo* sbInfo_p,
    nlm_u8 tblId,
    NlmUdaSbTblsInfo** tblInSb_pp /* out argument, can be null */
    )
{
    NlmUdaSbTblsInfo *tblHead_p = sbInfo_p->m_tblList_p;
    NlmUdaSbTblsInfo *node_p = (NlmUdaSbTblsInfo*)tblHead_p->m_next_p;

    NlmBool found = NlmFalse;
    
    while(node_p != tblHead_p && !found)
    {
        if(node_p->m_tblId == tblId)
        {
            found = NlmTrue;
            break;
        }
        node_p = (NlmUdaSbTblsInfo*)node_p->m_next_p;
    }

    if(found && tblInSb_pp != NULL)
        *tblInSb_pp = node_p;

    return found;
}

/* Check the Parallel Search dependencies of  given table to decide wheteher 
table cn enter in SB */
NlmBool 
NlmUdaMemMgr__pvt_CanTblEnterInNewSb(
    NlmUdaSbInfo*       udaSbInfo_p,
    NlmPsTblList    *psTblList_p
    )
{
    NlmBool tblCanEnternNewSb = NlmTrue;
    NlmUdaSbTblsInfo *tblInSbHead_p = NULL;
    NlmUdaSbTblsInfo *tblNode_p = NULL;
    NlmPsTblList *psTblListHead_p = psTblList_p;    
    NlmPsTblList *psTblListNode_p = NULL;
    
    if((udaSbInfo_p->m_isNewSb == NlmTrue) ||
            (psTblListHead_p == NULL))
        return NlmTrue;


    tblInSbHead_p = udaSbInfo_p->m_tblList_p;
    tblNode_p = (NlmUdaSbTblsInfo*)tblInSbHead_p->m_next_p;
    
    /*Iterate through each of the tables whose records are in the SB */
    while(tblNode_p != tblInSbHead_p)
    {
        psTblListNode_p = (NlmPsTblList*)psTblListHead_p->m_next_p;

        /*Iterate through the parallel search dependency list of the table
        which is requesting for the new super blk */
        while(psTblListNode_p != psTblListHead_p)
        {
            if(psTblListNode_p->m_tblId == tblNode_p->m_tblId)
            {
                /*If a table which is searched in parallel is already 
                having records in the Sb, then we cannot assign the 
                super blk*/
                tblCanEnternNewSb = NlmFalse;
                break;
            }
            psTblListNode_p = (NlmPsTblList*)psTblListNode_p->m_next_p;
        }
        if(!tblCanEnternNewSb) break;

        tblNode_p = (NlmUdaSbTblsInfo*)tblNode_p->m_next_p;
    }


    return tblCanEnternNewSb;
}

/* Allocate a chunk of given size and width from given SB. If allocated then 
set it to newChunkInfo_pp. */
NlmErrNum_t 
NlmUdaMemMgr__pvt_AllocateChunkFromSb(
            NlmUdaMemMgr*   self_p, 
            NlmUdaSbInfo        *sbInfo_p,
            nlm_u16         chunkSize,
            nlm_u16         chunkWidth, 
            NlmUdaChunkInfo** newChunkInfo_pp, 
            NlmReasonCode   *o_reason_p)
{
    NlmUdaChunkInfo*    newChunkInfo_p = NULL;

    NlmUdaChunkInfo *freeChunkHead_p = sbInfo_p->m_freeChunkInfo_p;
    NlmUdaChunkInfo *node_p = (NlmUdaChunkInfo*)freeChunkHead_p->m_next_p;

    nlm_u16 numOfRowsRequired = 0;
    NlmBool found = NlmFalse;

    numOfRowsRequired = chunkSize * chunkWidth /NLM_MAX_SRAM_SB_WIDTH_IN_BITS; 

    
    /* Iterate through free chunk list to find suitable chunk */    
    while(node_p != freeChunkHead_p && !found)
    {
        if((node_p->m_endRow - node_p->m_startRow) >= numOfRowsRequired)
        {
            found = NlmTrue;
            break;
        }
        node_p = (NlmUdaChunkInfo*)node_p->m_next_p;
    }

    if(found)
    {
        /* If chunk size is equal to required size then remove this node from list and
        return this chunk to user */
        if((node_p->m_endRow - node_p->m_startRow + 1) == numOfRowsRequired)
        {
            NlmUdaMemMgr__pvt_RemoveUdaChunkInfo(self_p->m_alloc_p, 
                    node_p, NlmTrue);
            
            *newChunkInfo_pp = node_p;
            
        }
        else
        {
            /* Allocate a new chunk and set the values*/
            newChunkInfo_p = (NlmUdaChunkInfo*)NlmCmAllocator__calloc(self_p->m_alloc_p, 
                                    1, sizeof(NlmUdaChunkInfo));
            if(!newChunkInfo_p)
            {
                *o_reason_p = NLMRSC_LOW_MEMORY;
                return NLMERR_FAIL;
            }

            newChunkInfo_p->m_sbNum = node_p->m_sbNum;
            
            if(NLM_MAX_SRAM_SB_WIDTH_IN_BITS == chunkWidth)
                newChunkInfo_p->m_width = NLM_TBL_ADLEN_256B;
            else if(NLM_MAX_SRAM_SB_WIDTH_IN_BITS == 2* chunkWidth)
                newChunkInfo_p->m_width = NLM_TBL_ADLEN_128B;
            else if(NLM_MAX_SRAM_SB_WIDTH_IN_BITS == 4* chunkWidth)
                newChunkInfo_p->m_width = NLM_TBL_ADLEN_64B;
            else  /* 32b */
                newChunkInfo_p->m_width = NLM_TBL_ADLEN_32B; 

            
            newChunkInfo_p->m_startRow = node_p->m_startRow;
            newChunkInfo_p->m_endRow = node_p->m_startRow + numOfRowsRequired - 1;

            *newChunkInfo_pp = newChunkInfo_p;

            /* Update the start of old chunk */
            node_p->m_startRow = newChunkInfo_p->m_endRow + 1;

            sbInfo_p->m_numOfAllocChunks++;
            
        }

        sbInfo_p->m_isNewSb = NlmFalse;
    }

    return NLMERR_OK;   
    
}


/* Allocate a chunk of given size and width in Old SB. */
NlmErrNum_t 
NlmUdaMemMgr__pvt_FindUdaChunkInOldSb(
            NlmUdaMemMgr*   self_p, 
            nlm_u8      tableId,
            NlmBool     isForcePSDependency,
            NlmPsTblList    *psTblList_p,
            nlm_u16         chunkSize,
            nlm_u16         chunkWidth,     
            NlmUdaChunkInfo**    chunkInfo_pp,
            NlmReasonCode   *o_reason_p)
{
    nlm_u16 sbNum = 0;
    NlmBool isTblInSb = NlmFalse;
    NlmBool tblCanEnternNewSb = NlmFalse;
    NlmUdaSbTblsInfo *curTblInfo_p = NULL;
    NlmErrNum_t errNum = NLMERR_OK;

    *chunkInfo_pp = NULL;

    /* Find in old SB where table is already allocated  */
    for(sbNum = 0; sbNum < (self_p->m_endSb - self_p->m_startSb + 1); sbNum++)
    {
        if(self_p->m_sbInfo_p[sbNum].m_isNewSb == NlmFalse)
        {
            isTblInSb = NlmUdaMemMgr__pvt_FindTblInSb(
                        &self_p->m_sbInfo_p[sbNum], tableId, &curTblInfo_p);

            /* If table is present then check if we can allocate chunk */           
            if(isTblInSb == NlmTrue)
            {
                errNum = NlmUdaMemMgr__pvt_AllocateChunkFromSb(self_p, 
                                &self_p->m_sbInfo_p[sbNum], chunkSize, chunkWidth,
                                chunkInfo_pp, o_reason_p);
                if(NLMERR_OK != errNum)
                    return errNum;
                /* return if chunk is allocated */
                if(*chunkInfo_pp)
                {                   
                    curTblInfo_p->m_numOfAllocChunks++;
                    return errNum;
                }
            }
        }
    }

    
    /* Find in old SB where table is not allocated  */
    for(sbNum = 0; sbNum < (self_p->m_endSb - self_p->m_startSb + 1); sbNum++)
    {
        if(self_p->m_sbInfo_p[sbNum].m_isNewSb == NlmFalse)
        {
            isTblInSb = NlmUdaMemMgr__pvt_FindTblInSb(
                        &self_p->m_sbInfo_p[sbNum], tableId, NULL);
            /* If Table is not present then check if we can allocate it */
            if(isTblInSb == NlmFalse)
            {
                tblCanEnternNewSb = NlmUdaMemMgr__pvt_CanTblEnterInNewSb(
                        &self_p->m_sbInfo_p[sbNum], psTblList_p);

                if((isForcePSDependency == NlmTrue) && (tblCanEnternNewSb == NlmFalse)) 
                    continue;

                /* Tabe can enter here so try allocating a chunk */
                errNum = NlmUdaMemMgr__pvt_AllocateChunkFromSb(self_p, 
                                &self_p->m_sbInfo_p[sbNum], chunkSize, chunkWidth,
                                chunkInfo_pp, o_reason_p);
                
                if(NLMERR_OK != errNum)
                    return errNum;
                
                /* If chunk is allocated then add the table into SB tablelist*/
                if(*chunkInfo_pp)
                {
                    curTblInfo_p = (NlmUdaSbTblsInfo*)NlmCmAllocator__calloc(self_p->m_alloc_p, 1,
                                    sizeof(NlmUdaSbTblsInfo));

                    if(!curTblInfo_p)
                    {
                        NlmUdaMemMgr__UpdateUdaChunkInfo(self_p, tableId, 
                                *chunkInfo_pp, NlmTrue, o_reason_p);
                        *o_reason_p = NLMRSC_LOW_MEMORY;
                        return NLMERR_FAIL;
                    }               

                    curTblInfo_p->m_tblId = tableId;            
                    curTblInfo_p->m_numOfAllocChunks = 1;
                    NlmCmDblLinkList__Insert((NlmCmDblLinkList*)self_p->m_sbInfo_p[sbNum].m_tblList_p,
                                    (NlmCmDblLinkList*) curTblInfo_p);  

                    return errNum;
                }
            }
            
        }
    }
    
    return errNum;  
}

/* Allocate a chunk of given size and width in New SB. Update the SB info with the 
given table */
NlmErrNum_t 
NlmUdaMemMgr__pvt_FindUdaChunkInNewSb(
            NlmUdaMemMgr*   self_p, 
            nlm_u8      tableId,
            nlm_u16         chunkSize,
            nlm_u16         chunkWidth,     
            NlmUdaChunkInfo**    chunkInfo_pp,
            NlmReasonCode   *o_reason_p)
{
    nlm_u16 sbNum = 0;
    NlmUdaSbTblsInfo *newTblInfo_p = NULL;
    NlmErrNum_t errNum = NLMERR_OK;

    *chunkInfo_pp = NULL;

    /* Find chunk in new SB */
    for(sbNum = 0; sbNum < (self_p->m_endSb - self_p->m_startSb + 1); sbNum++)
    {
        if(self_p->m_sbInfo_p[sbNum].m_isNewSb == NlmTrue)
        {
            /* Tabe can enter here so try allocating a chunk */
            errNum = NlmUdaMemMgr__pvt_AllocateChunkFromSb(self_p, 
                            &self_p->m_sbInfo_p[sbNum], chunkSize, chunkWidth,
                            chunkInfo_pp, o_reason_p);
            
            if(NLMERR_OK != errNum)
                return errNum;
            
            /* If chunk is allocated then add the table into SB tablelist*/
            if(*chunkInfo_pp)
            {
                newTblInfo_p = (NlmUdaSbTblsInfo*)NlmCmAllocator__calloc(self_p->m_alloc_p, 1,
                                sizeof(NlmUdaSbTblsInfo));

                if(!newTblInfo_p)
                {
                    NlmUdaMemMgr__UpdateUdaChunkInfo(self_p, tableId, 
                            *chunkInfo_pp, NlmTrue, o_reason_p);
                    *o_reason_p = NLMRSC_LOW_MEMORY;
                    return NLMERR_FAIL;
                }               

                newTblInfo_p->m_tblId = tableId;        
                newTblInfo_p->m_numOfAllocChunks = 1;
                NlmCmDblLinkList__Insert((NlmCmDblLinkList*)self_p->m_sbInfo_p[sbNum].m_tblList_p,
                                (NlmCmDblLinkList*) newTblInfo_p);  

                break;  
            }
        }           
    }   
    
    return errNum;  
}


/* Initializes UDA Memory Manager with the given SBs */
NlmUdaMemMgr* 
NlmUdaMemMgr__Init(
                NlmCmAllocator          *alloc_p,
                nlm_u16                 startSb,
                nlm_u16                 numOfSb,    
                NlmReasonCode           *o_reason_p)
{
    
    NlmUdaMemMgr* udaMemMgr = NULL;
    NlmUdaChunkInfo *udaChunkInfo = NULL;
    nlm_u16 sbNum = 0;
    udaMemMgr = (NlmUdaMemMgr*)NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmUdaMemMgr));

    if(udaMemMgr == NULL)
    {
        *o_reason_p = NLMRSC_LOW_MEMORY;
        return NULL;
    }

    udaMemMgr->m_alloc_p = alloc_p;

    udaMemMgr->m_startSb = startSb;
    udaMemMgr->m_endSb = startSb + numOfSb - 1;

    udaMemMgr->m_sbInfo_p = (NlmUdaSbInfo*)NlmCmAllocator__calloc(alloc_p, 1,
                                    sizeof(NlmUdaSbInfo) * numOfSb);

    if(!udaMemMgr->m_sbInfo_p)
    {
        NlmUdaMemMgr__Destroy(udaMemMgr);
        *o_reason_p = NLMRSC_LOW_MEMORY;
        return NULL;
    }
    
    /* Initialize data structures for each UDA SB */
    for(sbNum = 0; sbNum < numOfSb; sbNum++)
    {                                               
        udaMemMgr->m_sbInfo_p[sbNum].m_isNewSb = NlmTrue;
        udaMemMgr->m_sbInfo_p[sbNum].m_occupiedChunkInfo_p = NULL;
        udaMemMgr->m_sbInfo_p[sbNum].m_tblList_p = NULL;

        /* Initialize the Free UDA chunk list with Sentinel node*/
        udaMemMgr->m_sbInfo_p[sbNum].m_freeChunkInfo_p = (NlmUdaChunkInfo*)NlmCmAllocator__calloc(alloc_p, 1,
                                    sizeof(NlmUdaChunkInfo));

        if(!udaMemMgr->m_sbInfo_p[sbNum].m_freeChunkInfo_p)
        {
            NlmUdaMemMgr__Destroy(udaMemMgr);
            *o_reason_p = NLMRSC_LOW_MEMORY;
            return NULL;
        }

        udaMemMgr->m_sbInfo_p[sbNum].m_freeChunkInfo_p->m_startRow = 0;
        udaMemMgr->m_sbInfo_p[sbNum].m_freeChunkInfo_p->m_endRow = 0;
        
        NlmCmDblLinkList__Init((NlmCmDblLinkList*)udaMemMgr->m_sbInfo_p[sbNum].m_freeChunkInfo_p);

        /* Add the entire SB as a free chunk */ 
        udaChunkInfo = (NlmUdaChunkInfo*)NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmUdaChunkInfo));
        if(!udaChunkInfo)
        {
            NlmUdaMemMgr__Destroy(udaMemMgr);
            *o_reason_p = NLMRSC_LOW_MEMORY;
            return NULL;
        }
        
        udaChunkInfo->m_sbNum = startSb + sbNum;
        udaChunkInfo->m_startRow = 0;       
        udaChunkInfo->m_endRow = NLMDEV_NUM_ENTRIES_PER_SRAM_BLOCK;
        udaChunkInfo->m_width = NLM_TBL_ADLEN_256B;
        NlmCmDblLinkList__Insert((NlmCmDblLinkList*)udaMemMgr->m_sbInfo_p[sbNum].m_freeChunkInfo_p,
                                (NlmCmDblLinkList*) udaChunkInfo);

        /* Initialize Table Info with Sentinel node */
        udaMemMgr->m_sbInfo_p[sbNum].m_tblList_p = (NlmUdaSbTblsInfo*)NlmCmAllocator__calloc(alloc_p, 1,
                                    sizeof(NlmUdaSbTblsInfo));

        NlmCmDblLinkList__Init((NlmCmDblLinkList*)udaMemMgr->m_sbInfo_p[sbNum].m_tblList_p);

        if(!udaMemMgr->m_sbInfo_p[sbNum].m_tblList_p)
        {
            NlmUdaMemMgr__Destroy(udaMemMgr);
            *o_reason_p = NLMRSC_LOW_MEMORY;
            return NULL;
        }   
    }   

    return udaMemMgr;   
}

/* Destroy UDA Memory Manager  */
void 
NlmUdaMemMgr__Destroy(NlmUdaMemMgr* self_p)
{
    
    nlm_u16 sbNum = 0;
    NlmUdaSbInfo *sbInfo_p = NULL;
    
    if(self_p == NULL)
    {       
        return ;
    }

      /* Destroy all UDA SB info */
    if(self_p->m_sbInfo_p)
    {
        for(sbNum = 0; sbNum < (self_p->m_endSb - self_p->m_startSb + 1); sbNum++)
        {   
             sbInfo_p = &self_p->m_sbInfo_p[sbNum];
             
            if(sbInfo_p->m_freeChunkInfo_p)
                NlmCmDblLinkList__Destroy((NlmCmDblLinkList*)sbInfo_p->m_freeChunkInfo_p, 
                        NlmUdaMemMgr__pvt_DestroyUdaChunkInfo, self_p->m_alloc_p);

            if(sbInfo_p->m_occupiedChunkInfo_p)
                NlmCmDblLinkList__Destroy((NlmCmDblLinkList*)sbInfo_p->m_occupiedChunkInfo_p, 
                        NlmUdaMemMgr__pvt_DestroyUdaChunkInfo, self_p->m_alloc_p);

            if(sbInfo_p->m_tblList_p)
                NlmCmDblLinkList__Destroy((NlmCmDblLinkList*)sbInfo_p->m_tblList_p, 
                        NlmUdaMemMgr__pvt_DestroyNodeTbl, self_p->m_alloc_p);
        }

        NlmCmAllocator__free(self_p->m_alloc_p, self_p->m_sbInfo_p);
    }       

    
    NlmCmAllocator__free(self_p->m_alloc_p, self_p);

    return; 
}


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
            NlmReasonCode   *o_reason_p)
{
    NlmErrNum_t errNum = NLMERR_OK;
    NlmBool isForcePSDependency = NlmTrue;

    /* Find in old SB first */
    errNum = NlmUdaMemMgr__pvt_FindUdaChunkInOldSb(
                self_p, tableId, isForcePSDependency, psTblList_p,
                chunkSize, chunkWidth,  chunkInfo_pp, o_reason_p);

    if((NLMERR_OK != errNum) || (*chunkInfo_pp != NULL))
        return errNum;

    /* If not found in old SB then find in new SB  */
    errNum = NlmUdaMemMgr__pvt_FindUdaChunkInNewSb(
                    self_p, tableId, chunkSize, chunkWidth,     
                    chunkInfo_pp, o_reason_p);

    if((NLMERR_OK != errNum) || (*chunkInfo_pp != NULL))
        return errNum;

    /* Find in old SB without forcing Parallel Search Dependencies  */
    isForcePSDependency = NlmFalse;
    errNum = NlmUdaMemMgr__pvt_FindUdaChunkInOldSb(
                self_p, tableId, isForcePSDependency, psTblList_p,
                chunkSize, chunkWidth,  chunkInfo_pp, o_reason_p);

    if((NLMERR_OK != errNum) || (*chunkInfo_pp != NULL))
        return errNum;

    if(*chunkInfo_pp == NULL)
    {
        /*There is no free space available in UDA */
        *o_reason_p = NLMRSC_TABLE_FULL;
        errNum = NLMERR_FAIL;
    }

    return errNum;  
}

/* Update UDA chunk info and remove the given table from the associated UDA SB.
 If freeChunkAlso is set then free the chucnk as it is not used by any other table  */
NlmErrNum_t 
NlmUdaMemMgr__UpdateUdaChunkInfo(
            NlmUdaMemMgr*   self_p, 
            nlm_u8      tableId,
            NlmUdaChunkInfo*    chunkInfo_p,
            NlmBool            freeChunkAlso,
            NlmReasonCode   *o_reason_p)
{
    nlm_u32 sbNum = 0;
    NlmBool found = NlmFalse, chunkFreed = NlmFalse;
    NlmUdaSbTblsInfo *curTblInfo_p = NULL;
    NlmErrNum_t errNum = NLMERR_OK;
    
    NlmUdaChunkInfo *freeChunkHead_p = NULL;
    NlmUdaChunkInfo *node_p = NULL;
    NlmUdaChunkInfo *prevNode_p = NULL;

    (void)o_reason_p;

    sbNum = chunkInfo_p->m_sbNum - self_p->m_startSb;

    /* Find the table and update info if present */

    found = NlmUdaMemMgr__pvt_FindTblInSb(
                &self_p->m_sbInfo_p[sbNum], tableId, &curTblInfo_p);

    if(found == NlmTrue)
    {
        curTblInfo_p->m_numOfAllocChunks--;

        if(curTblInfo_p->m_numOfAllocChunks == 0)
            NlmUdaMemMgr__pvt_RemoveTblsInUdaSb(self_p->m_alloc_p, curTblInfo_p);
    }

    /* If freeChunkAlso is not set then return otherwise free that chunk as associated 
    DBA block is empty */

    if(freeChunkAlso == NlmFalse)
        return errNum;

    freeChunkHead_p = self_p->m_sbInfo_p[sbNum].m_freeChunkInfo_p;
    node_p = (NlmUdaChunkInfo*)freeChunkHead_p->m_next_p;

    /* Find the chunk location in ascending order of start row and try to merge */
    while(node_p != freeChunkHead_p)
    {
        if(node_p->m_startRow > chunkInfo_p->m_startRow)
            break;
        node_p = (NlmUdaChunkInfo*)node_p->m_next_p;
    }

    /* Check if we can merge with the current node which is right node */
    if(((node_p->m_startRow + node_p->m_endRow) != 0) &&
        ((node_p->m_startRow - 1) ==  chunkInfo_p->m_endRow))
    {
        /* Merge it and free chunk memory */
        node_p->m_startRow = chunkInfo_p->m_startRow;

        NlmUdaMemMgr__pvt_DestroyUdaChunkInfo((NlmCmDblLinkList*)chunkInfo_p, self_p->m_alloc_p);

        chunkInfo_p = node_p; /* Use the latest node form now onwards */
        chunkFreed = NlmTrue;
    }
    
    prevNode_p = (NlmUdaChunkInfo*)node_p->m_back_p;

    /* Check if we can merge with end of prev node which is left node  */
    if(((prevNode_p->m_startRow + prevNode_p->m_endRow) != 0) &&
        ((prevNode_p->m_endRow + 1) ==  chunkInfo_p->m_startRow))
    {
        /* Merge it and free chunk memory */
        prevNode_p->m_endRow = chunkInfo_p->m_endRow;
    
        if(chunkFreed == NlmTrue)
            NlmUdaMemMgr__pvt_RemoveUdaChunkInfo(self_p->m_alloc_p, 
                    chunkInfo_p, NlmFalse); 
        else
            NlmUdaMemMgr__pvt_DestroyUdaChunkInfo((NlmCmDblLinkList*)chunkInfo_p, self_p->m_alloc_p);
    
        chunkFreed = NlmTrue;
    }   


    /* If given chunk can not be merged then add it after the previous node */
    if(chunkFreed == NlmFalse)
    {       
        NlmCmDblLinkList__Insert((NlmCmDblLinkList*)prevNode_p,
                                        (NlmCmDblLinkList*) chunkInfo_p);
        return NLMERR_OK;
    }


    self_p->m_sbInfo_p[sbNum].m_numOfAllocChunks--;

    /* If this is the last chunk present in SB then free all data associated with SB 
    and initialize the SB again*/
    if(self_p->m_sbInfo_p[sbNum].m_numOfAllocChunks == 0)
    {
        curTblInfo_p = (NlmUdaSbTblsInfo*)self_p->m_sbInfo_p[sbNum].m_tblList_p->m_next_p;
        /* Free all tables present here */
        while(self_p->m_sbInfo_p[sbNum].m_tblList_p != curTblInfo_p)
        {
            NlmUdaMemMgr__pvt_RemoveTblsInUdaSb(self_p->m_alloc_p, curTblInfo_p);
            curTblInfo_p = (NlmUdaSbTblsInfo*)self_p->m_sbInfo_p[sbNum].m_tblList_p->m_next_p;
        }

        self_p->m_sbInfo_p[sbNum].m_isNewSb = NlmTrue;
        
    }   
        
    return NLMERR_OK;   
}


