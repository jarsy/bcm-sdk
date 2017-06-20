/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
/*
 **************************************************************************************
 Copyright 2009-2012 Broadcom Corporation

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
 #ifndef INCLUDED_NLMFIBTBLMGR_H
#define INCLUDED_NLMFIBTBLMGR_H

#include "nlmcmbasic.h"
#include "nlmcmallocator.h"
#include "nlmerrorcodes.h"
#include "nlmarch.h"
#include "nlmcmdevice.h"
#include "nlmdevmgr_ss.h"
#include "nlmdevmgr_shadow.h"
#include "nlmcmprefix.h"
#include "nlmcmpfxbundle.h"

#include "nlmcmexterncstart.h"

#define NLM_FIB_MAX_SIZE_BATCH				(5000)
#define NLM_FIB_MAX_SIZE_CALLBACK_ARRAY		(20000)

#define NLM_FIB_MAX_PARALLEL_SRCHES         (2)
#define NLM_FIB_INVALID_INDEX				(0xFFFFFFFF)
#define NLM_FIB_BLK_RANGE_WHOLE_DEVICE		(NLMDEV_NUM_ARRAY_BLOCKS) 
#define NLM_FIB_BLK_RANGE_NO_BLK_USED		(0xFF)

/* PIO batch mode specific structure definitions */
typedef struct NlmFibTblMgr_DevABRows_s
{
	nlm_u32 m_devBlkRow[NLMDEV_NUM_ARRAY_BLOCKS];
	nlm_u32 m_addOrDel[NLMDEV_NUM_ARRAY_BLOCKS];

} NlmFibTblMgr_DevABRows;

typedef struct NlmFibTblMgr_ShadowABWrites_s
{
	NlmFibTblMgr_DevABRows m_devABBlk[NLMDEV_NUM_ARRAY_BLOCKS];
	nlm_u32 m_BlkBit[4];

} NlmFibTblMgr_ShadowABWrites;

typedef struct NlmPIOInstStat_s
{
	NlmFibTblMgr_ShadowABWrites  *m_ABStat_p;

}NlmPIOInstStat;


typedef struct NlmFibBatchIxCBData_s
{
	NlmCmPfxBundle  *m_pfxBndl_p;
	nlm_u32			m_oldIndex;
  
} NlmFibBatchIxCBData;


typedef struct NlmFibBatchIxCBDataList_s
{
	NlmFibBatchIxCBData     m_ixCbData;
	struct NlmFibBatchIxCBDataList_s *m_next_p;
} NlmFibBatchIxCBDataList;


/* Store the Non Batch Prefix Ix CB into an array. If Array is exhausted then store 
new coming into the link list. */
typedef struct NlmFibBatchIxCBInfo_s
{
    NlmFibBatchIxCBData m_adNonBatchPfx_p[NLM_FIB_MAX_SIZE_CALLBACK_ARRAY];
	NlmFibBatchIxCBData m_adBatchPfx_p[NLM_FIB_MAX_SIZE_BATCH];

	/* It is required only if m_adNonBatchPfx_p array is exhausted */
	NlmFibBatchIxCBDataList* m_adNonBatchPfxList_p;
	NlmFibBatchIxCBDataList* m_adNonBatchPfxListTail_p;
	nlm_u32              m_bSize;
	nlm_u32				 m_adNonBatchPfxCount;
	nlm_u32              m_adBatchPfxCount;

} NlmFibBatchIxCBInfo;



typedef nlm_u32 NlmFibPrefixIndex;

typedef enum NlmFibAttribType
{
	NLM_FIB_ATTRIB_COMPACTION, /* To enable/disable compaction*/
	NLM_FIB_ATTRIB_OVERALLOC,  /* To enable/disable overallocation*/
	NLM_FIB_ATTRIB_FULLALLOC,  /* To enable/disable full allocation*/
	NLM_FIB_ATTRIB_GROW_ENH,  /* To enable/disable grow enhancement*/
	NLM_FIB_ATTRIB_BYTE_ALIGN_GRAN,  /* To enable/disable byte aligned granularity */
	NLM_FIB_ATTRIB_FUTURE_USE, /* For future use*/	
	NLM_FIB_ATTRIB_CHECK_FOR_DUPLICATE, /* To enable/disable the duplicate check */
	NLM_FIB_ATTRIB_OVERALLOC_INDEX, /* To enable/disable Overallocation Index:  
								   this is to reduce index range wastage in  Over alloc mode*/
	NLM_FIB_ATTIB_ARENA_SIZE_IN_BYTES /* Size of the Memory Arena in Bytes*/
}NlmFibAttribType;


/* Declaration for Index Change Call back Function */
typedef void 
(*NlmFibPrefixIndexChangedAppCb)(void* client_p,   
                                 void* fibTbl_p, /* ptr to table to which 
                                                 the prefix which is shuffled
                                                 belongs */ 
                                 NlmFibPrefixIndex oldIndex,  /* old index of the 
                                                           Prefix that was shuffled */
                                 NlmFibPrefixIndex newIndex  /* new index of the Prefix
                                                          that was shuffled */
                                 );



typedef struct NlmFibBlksRange_s 
{
    nlm_u8 m_startBlkNum;   /* Initializing start blk or end blk to NLM_FIB_BLK_RANGE_WHOLE_DEVICE
                           indicates all blocks of the device are allocated to FIB tables 
                           whereas initializing it to NLM_FIB_BLK_RANGE_NO_BLK_USE indicates
                           no block of the device are allocated to FIB Tables*/ 

    nlm_u8 m_endBlkNum;     /* Note: Ensure that Start Blk <= EndBlock */  
} NlmFibBlksRange; 

/* Fib Tbl Mgr Structure */
typedef struct NlmFibTblMgr_s
{
    NlmCmAllocator  *m_alloc_p;           /*  Memory allocator */
    	
    NlmDevType      m_devType;           /*  type of device FibTblMgr 
                                          is operating on */

    void			*m_devMgr_p;

    nlm_u8          m_byteAlignGran;       /* Flag for Byte align granularity */
    nlm_u8          m_numOfDevices;       /* number of devices in cascade */

    NlmFibBlksRange *m_fibBlksRange;  /* Range of Blks FibTbls can span 
                                           for each device */ 
    
    void			*m_fibTblList_p; /*  List of fib Tbls associated 
                                   with FibTblMgr */

    nlm_u16         m_fibTblCount;  /*  Number of fib tables 
                                     associated with FibTblMgr */

    nlm_u16         m_tblIdStrLen;  /* Length of table id, Should be 
                                      same for all the FIB tables  */ 
	
    NlmBool			m_IsConfigLocked;  /* Indicates whether fibtblmgr
                                        is locked */

    void			*m_vtbl_p;  /* Virtual Tbl pointer */

	void			*m_privatePtr_p;		/* This is a void pointer so that 
                                        application does not 
                                        know about the internal 
                                        algorithms and 
                                        data structures. */

    NlmFibPrefixIndexChangedAppCb  m_indexChangeCallBackFn;
                                    /* Call back used by the Fib 
                                    Mgr to inform the change in the
                                    index of the prefix to the application
                                    The definition of this call bakc 
                                    needs to be implemented by the application 
                                    and its pointer is passed to FibMgr 
                                    during Init*/

	void			*m_client_p;    /* Client Ptr which may be used by the 
                                   call back function */

    void			*m_devWriteCallBackFns;   

	NlmBool			m_isBatchMode;

	NlmPIOInstStat	*m_pioInfo_p;
	NlmFibBatchIxCBInfo	 *m_cbList_p;

	nlm_u32			m_arenaSizeInBytes;

} NlmFibTblMgr;

typedef struct NlmFibTblIndexRange_s
{
    nlm_u32 m_indexLowValue; /* Represents low value of the tbl index range */
    nlm_u32 m_indexHighValue; /* Represents high value of the tbl0 index range */
} NlmFibTblIndexRange; /* If the user does not want to specify the index range, 
                       both the values should be initialized to */


typedef struct NlmFibTbl_s
{
	NlmFibTblMgr	*m_fibTblMgr_p;          /* NlmFibTblMgr to 
                                          which the table belongs to */

    NlmCmAllocator	*m_alloc_p;  /* Generic allocator pointer */
	
	nlm_8           *m_idStr_p;         /* Table id assigned to the table */	

	nlm_u16         m_width;            /*  Table width; Represents maximum 
                                          length of prefix that can be added 
                                          to the tbl */                                          

	nlm_u32         m_numPrefixes;      /*  Number of prefixes currently 
                                            present in the table */
	
	void        	*m_tblSrchAttrs_p; /* Srch Attributes associated 
                                                    with the tbl*/

    NlmFibTblIndexRange    m_tblIndexRange; /* Represents the index range 
                                            for the tbl; Complimentary for user
                                            If specified Device will return the 
                                            hit addresses within this range for the tbl*/

    void			*m_dependentTbls_p; 
                            /* Represents list of dependent tbls */

	void            *m_privatePtr_p; /* This is a void pointer so that 
                                      the application does not about 
                                      the internal algorithms and 
									  data structures. */

    nlm_u16         m_startBit;

    nlm_u16         m_endBit;

	NlmBool		m_isSpecialParallelSearch;
	nlm_8		*m_primaryTblId;
	
} NlmFibTbl;

typedef struct NlmFibTblsList_s
{
    #include "nlmcmdbllinklistdata.h"
    NlmFibTbl *m_fibTbl_p;
} NlmFibTblsList;

typedef struct NlmFibParallelSrchInfo_s
{
    nlm_8 *m_tblId;      /* Id of tbl which will for the specified ltr */
    
    NlmDev_parallel_search_t m_rsltPortNum; /* Result Port Number 
                                              used for the tbl for the
                                              specified ltr 
                                              Possible Values are 0 and 1
                                              In case of two tbl srch
                                              Result port number should be different
                                              for each tbl*/

    nlm_u16 m_startBitInKey;  /* Represents the start bit of the prefix to be searched
                                 in the search key */
} NlmFibParallelSrchInfo;

typedef struct NlmFibParallelSrchAttributes_s
{
    nlm_u8 m_numOfFibParallelSrch;  /* Possible Values 1 and 2 */
   
    NlmFibParallelSrchInfo m_parallelSrchInfo[NLM_FIB_MAX_PARALLEL_SRCHES];
                                /* Represents the parallel srch info used 
                                for each tbl searched */

} NlmFibParallelSrchAttributes;


typedef struct NlmFibParallelSrchAttributes2_s
{
	nlm_8 *m_tblId0; /* primary table ID */

	nlm_8 *m_tblId1; /* secondary Table Id */

	NlmDev_parallel_search_t m_rsltPortNum0; /* result port for primary table */

	NlmDev_parallel_search_t m_rsltPortNum1; /* result port for second table */

	nlm_u16 m_startBitInKey; /* star bit for key construction */

} NlmFibParallelSrchAttributes2;




extern NlmFibTblMgr* 
NlmFibTblMgr__Init(NlmCmAllocator *alloc_p,
                   void           *devMgr_p,                   
                   NlmDevType     devType,
                   nlm_u8 numOfDevices,
                   NlmFibBlksRange *fibBlksRange, 
                            /* Array of blocks range for FTM tables,
                            Size of Array = Number of Devices, 
                            Index 0 - Blk Range For Device 0,
                            Index 1 - Blk Range For Device 1 and so on*/
                   nlm_u16 tblIdLen,    /* Represents length of Tbls Ids assigned to 
                                         all fib tbls */
                   NlmFibPrefixIndexChangedAppCb indexChangedAppCb,
                   void *client_p, 
                   NlmReasonCode *o_reason
                   );

extern NlmErrNum_t 
NlmFibTblMgr__Destroy(NlmFibTblMgr *fibTblMgr,
                      NlmReasonCode *o_reason
                      );

extern NlmErrNum_t 
NlmFibTblMgr__GetDeviceCapacity(NlmFibTblMgr *fibTblMgr,
								nlm_u8 deviceNum,
								nlm_u32 *devCapacity,
								NlmReasonCode *o_reason
								);

extern NlmFibTbl*
NlmFibTblMgr__CreateTable(NlmFibTblMgr *fibTblMgr,
                          nlm_8 *fibTblIdStr,   /* String of 0's and 1's 
                                                 indicating Fib Tbl Id */
                          NlmFibTblIndexRange  *fibTblIndexRange, 
                          nlm_u16    fibMaxPrefixLength, /* Max Prefix Length 
                                                         for the Fib Tbl*/
                          NlmReasonCode *o_reason
                          );

extern NlmErrNum_t 
NlmFibTblMgr__DestroyTable(NlmFibTbl *fibTbl,
                           NlmReasonCode *o_reason
                           );

extern NlmErrNum_t 
NlmFibTblMgr__LockConfiguration(NlmFibTblMgr *fibTblMgr,
                                NlmReasonCode    *o_reason
                                );

extern NlmErrNum_t
NlmFibTblMgr__ConfigSearch(NlmFibTblMgr *FibTblMgr,
                        nlm_u8 ltrNum,
                        NlmFibParallelSrchAttributes *psAttrs, /* Parallel Srch
                                                               attributes associated 
                                                               with the ltr */
                        NlmReasonCode *o_reason
                        );

extern NlmErrNum_t
NlmFibTblMgr__ConfigSearch2(NlmFibTblMgr *FibTblMgr,
                        nlm_u8 ltrNum,
                        NlmFibParallelSrchAttributes2 *psAttrs, /* Parallel Srch
                                                               attributes associated 
                                                               with the ltr */
                        NlmReasonCode *o_reason
                        );


extern NlmErrNum_t 
NlmFibTblMgr__SetAttribute(
    NlmFibTbl *fibTbl,  /* FibTbl to which prefix is added */
    NlmFibAttribType attribType,	/* Attribute type*/
    nlm_u32  attribValue, 		/* Attribute value*/
    NlmReasonCode *o_reason  
    );




#if !defined NLM_MT_OLD && !defined NLM_MT
/* This API is not supported in Multi-Thread version */

extern NlmErrNum_t NlmFibTblMgr__AddPrefixBatch(
    NlmFibTbl *fibTbl,  /* FibTbl to which prefix is added */
	nlm_u16 *numPrefixes,
    NlmCmPrefix *prefix[], /* represents prefix array being added */
    NlmReasonCode *o_reason
    );
#endif

extern NlmErrNum_t 
NlmFibTblMgr__AddPrefix(NlmFibTbl *FibTbl,  /* FibTbl to which prefix is added */
                        NlmCmPrefix *prefix, /* represents prefix being added */
                        NlmReasonCode *o_reason
                        );

extern NlmErrNum_t 
NlmFibTblMgr__DeletePrefix(NlmFibTbl *FibTbl,/* FibTbl from which prefix is being 
                                             deleted */
                           NlmCmPrefix *prefix,/* represents prefix being deleted */
                           NlmReasonCode *o_reason
                           );

extern NlmErrNum_t 
NlmFibTblMgr__LocateExactMatch(NlmFibTbl *FibTbl,  /* FibTbl in which 
                                                   prefix is searched */
                        NlmCmPrefix *prefix, /* represents prefix being searched */
                        NlmFibPrefixIndex *o_prefixIndex,/* Exact match Prefix Index 
                                                         returned by the search opeartion 
                                                         In case of miss returns 
                                                         Invalid Match Index 
                                                         NLM_FIB_INVALID_INDEX = (0xFFFFFFFF)*/
                        NlmReasonCode *o_reason
                        );

extern NlmErrNum_t 
NlmFibTblMgr__LocateLPM(NlmFibTbl *FibTbl,  /* FibTbl in which 
                                            prefix is searched */
                        NlmCmPrefix *prefix, /* represents prefix being searched */
                        NlmFibPrefixIndex *o_prefixIndex,/* Longest Prefix Match Index 
                                                         returned by the search opeartion
                                                         In case of miss returns 
                                                         Invalid Match Index 
                                                         NLM_FIB_INVALID_INDEX = (0xFFFFFFFF)*/
                        NlmCmPrefix *o_longestMatchPrefix,
                        NlmReasonCode *o_reason
                        );




extern NlmCmPrefix* 
NlmFibTblMgr__GetPrefix(NlmFibTbl *FibTbl,
						nlm_u32 index);

extern NlmErrNum_t NlmFibTblMgr__DeletePrefix2 (
                             NlmFibTbl *fibTbl,
                             NlmCmPrefix *prefix,
                             NlmFibPrefixIndex *o_prefixIndex,
                             NlmReasonCode *o_reason);

#include "nlmcmexterncend.h"

#endif

