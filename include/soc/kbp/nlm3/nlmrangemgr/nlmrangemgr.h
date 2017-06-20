/*
 * $Id: nlmrangemgr.h,v 1.1.6.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 #ifndef INCLUDED_NLMRANGEMGR_H
#define INCLUDED_NLMRANGEMGR_H

#if defined NLM_MT_OLD || defined NLM_MT
#ifndef NLMPLATFORM_BCM
#include <nlmcmmt.h>
#else
#include <soc/kbp/common/nlmcmmt.h>
#endif
#endif


/* include files */
#ifndef NLMPLATFORM_BCM
#include <nlmcmbasic.h>
#include "nlmcmdebug.h"
#include "nlmcmallocator.h"
#include "nlmarch.h"
#include "nlmerrorcodes.h"
#include "nlmcmexterncstart.h"
#include "nlmcmdevice.h"
#include "nlmcmutility.h"
#include "nlmdevmgr.h"
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmcmdebug.h>
#include <soc/kbp/common/nlmcmallocator.h>
#include <soc/kbp/nlm3/arch/nlmarch.h>
#include <soc/kbp/common/nlmerrorcodes.h>
#include <soc/kbp/common/nlmcmexterncstart.h>
#include <soc/kbp/common/nlmcmdevice.h>
#include <soc/kbp/common/nlmcmutility.h>
#include <soc/kbp/nlm3/nlmdevmgr/nlmdevmgr.h>
#endif

#define NLM_RANGE_NUM_TOP_RANGES    NLMDEV_RANGE_NUM_MCOR_PER_RANGE_TYPE
#define NLM_RANGE_NUM_BOUNDS        (2)
#define NLM_RANGE_HT_DEPTH          (256)

/* Enum for type of Ranges supported */
typedef enum NlmRangeType
{
    NLM_RANGE_TYPE_A = 0,
    NLM_RANGE_TYPE_B,
    NLM_RANGE_TYPE_C,
    NLM_RANGE_TYPE_D,

    NLM_RANGE_TYPE_END /* Not valid range type */

} NlmRangeType;

/* Enum for type of Encodings */
typedef enum NlmRangeEncodingType
{    
    NLM_RANGE_3B_ENCODING,
    NLM_RANGE_2B_ENCODING,
    NLM_RANGE_NO_ENCODING
}NlmRangeEncodingType;

/* Structure which specifies Search Attributes related to Range */
typedef struct NlmRangeSrchAttrs_s
{
    nlm_u8 m_extraction_startByte_rangeA;          /* Specifies Extraction start byte for Range A */
    nlm_u8 m_keyInsert_startByte_rangeA[NLM_DEV_NUM_KEYS];/* Specifies Range A Insertion start byte for all the keys */
    struct NlmRangeDb_s *m_rangeA_db;/* Specifies Range Database associated with Range A */

    nlm_u8 m_extraction_startByte_rangeB;/* Specifies Extraction start byte for Range B */
    nlm_u8 m_keyInsert_startByte_rangeB[NLM_DEV_NUM_KEYS];/* Specifies Range B Insertion start byte for all the keys */
    struct NlmRangeDb_s *m_rangeB_db;/* Specifies Range Database associated with Range B */

    nlm_u8 m_extraction_startByte_rangeC;/* Specifies Extraction start byte for Range C */
    nlm_u8 m_keyInsert_startByte_rangeC[NLM_DEV_NUM_KEYS];/* Specifies Range C Insertion start byte for all the keys */
    struct NlmRangeDb_s *m_rangeC_db;/* Specifies Range Database associated with Range C */

    nlm_u8 m_extraction_startByte_rangeD;/* Specifies Extraction start byte for Range D */
    nlm_u8 m_keyInsert_startByte_rangeD[NLM_DEV_NUM_KEYS];/* Specifies Range D Insertion start byte for all the keys */
    struct NlmRangeDb_s *m_rangeD_db;/* Specifies Range Database associated with Range D */

} NlmRangeSrchAttrs;

typedef struct NlmRangeDMFormat_s
{
    nlm_u32          m_data;    /* Data, for ternary bits it can be 0 or 1: doesn't matter */
    nlm_u32          m_mask;  /* The ternary bits would be set to 1 here */
} NlmRangeDMFormat;

/* The range field is encoded internally in different ways, but it is always
 * returned as a bitmap;
 * A particular Range, after encoding, could result in multiple Entries.
 * The specific number of entries that will be created depends on the nature
 * of the range (and sometimes on the other ranges present in the Db);
 * m_num_entries indicates the amount of expansion;
 */
typedef struct NlmRangeEncoded_s
{
    nlm_u32             m_id;           /* ID associated with the range */
    NlmRangeDMFormat    *m_entries_p;   /* pointer to the encoded entries */
    nlm_u32             m_num_entries;  /* number of entries created */
} NlmRangeEncoded;

/* A generic range field R is considered to be m_start <= R <= m_end */
typedef struct NlmRange_s
{
    nlm_u32             m_id;       /* ID associated with the range */
    nlm_u16             m_start;    /* Lower Bound of the Range */
    nlm_u16             m_end;      /* Upper Bound of the Range*/
    NlmRangeEncoded*    m_encoded_p;/* Pointer to the list of encoded-entries, it is NULL when encodings
                                       are not created */
}NlmRange;

/* Node for the Doubly-Linked List for the List of Ranges Stored in a DataBase */
typedef struct NlmRangeNode_s
{
    NlmRange*   m_range;
    struct NlmRangeNode_s* m_prev;
    struct NlmRangeNode_s* m_next;
}NlmRangeNode;

/* This structure represents the whole list; it points to the first
 * range entries in a database
 */
typedef struct NlmRangeList_s
{
    NlmRangeNode* m_firstRange; 
}NlmRangeList;

/* Structure to hold some interesting stats like:
 * top 8 ranges, number of Ranges, number of Mapped Entries
 */
typedef struct NlmRangeDbStats_s
{
    nlm_u16             m_topRanges[NLM_RANGE_NUM_TOP_RANGES][NLM_RANGE_NUM_BOUNDS]; 
                                                /* Top 8 Ranges.
                                                 * Range[i][1] is UB, Range[i][0] is LB
                                                 */
    nlm_u32             m_numOfRanges;          /* Number of Ranges */
    nlm_u32             m_numOfMappedEntries;   /* Number of Mapped Entries */
}NlmRangeDbStats;

/* Range Database : Structure for Range Database which holds the ranges for the SW
 * Different databases with different attributes can be created based on the
 * different types/classes of incoming ranges.
 * Once the Database is created we can add and remove range entries from it.
 */
typedef struct NlmRangeDb_s
{
    struct NlmRangeMgr_s *m_rangeMgr_p;   /* the RangeMgr to which this Database belongs to  */
    NlmCmAllocator      *m_alloc_p;      /* memory allocator                                */
    nlm_u8               m_id;
    NlmRangeList         m_entries[NLM_RANGE_HT_DEPTH]; /* Array doubly-linked list */      
    nlm_u8               m_num_bits;    /* Number of bits available for encoding of range  */
    nlm_u8               m_valid_bits;  /* Number of bits to be used to store the start and end of a Range.
                                         * It's default value is 16
                                         */
    nlm_u8               m_use_mcor;    /* if it is set: the B-portion in created encoding will contain valid values
                                         * while when it is 0, the B portion is all dont-cares.
                                         */    
    NlmRangeType         m_range_type;   /* If MCOR is used for the DB, then specifies Range type for this DB */
    nlm_u16              m_range_code;   /* Specifies the type of encodings used for the DB */
 
    nlm_u32              m_range_count;  /* number of ranges                                */
    NlmRangeDbStats      m_stats;      /* stats on amount of expansion in Processor. Also contains the snapshot of 
                                          * 8 MCORs when the last __CreateEncoding is called.
                                          */    

    void                *m_pvtdata;     /* used for getting the top ranges */
} NlmRangeDb;

/* The structure contains the attributes a Range-Database can be configured */
typedef struct NlmRangeDbAttrSet_s
{
    nlm_u8              m_num_bits;     /* Number of bits -available for encoding of range  */
    nlm_u8              m_valid_bits;   /* Number of bits to be used to store the start and end of a Range.
                                         * It's default value is 16
                                         */
    NlmRangeEncodingType  m_encodingType; /* Encoding type for the data base*/
}NlmRangeDbAttrSet;

/* Node of Doubly-Linked List containing Range-Databases */
typedef struct NlmRangeDbNode_s
{
    NlmRangeDb* m_database;
    struct NlmRangeDbNode_s* m_prev;
    struct NlmRangeDbNode_s* m_next;
}NlmRangeDbNode;

/* This structure represents the whole list; it points to the first
 * node of the doubly linked list
 */
typedef struct NlmRangeDbList_s
{
    NlmRangeDbNode  *m_firstDb; 
}NlmRangeDbList;

/* Virtual Functions for Range Manager */
typedef NlmErrNum_t (*NlmXkRangeMgr__InitCodeRegs)(
    struct NlmRangeMgr_s    *pRangeMgr,
    NlmBankNum           bankNum,
    NlmRangeType rangeType,
    nlm_u32 rangeCodeValue,
    NlmReasonCode    *o_reason
    );

typedef NlmErrNum_t (*NlmXkRangeMgr__InitBoundRegs)(
    struct NlmRangeMgr_s        *pRangeMgr,
    NlmBankNum           bankNum,
    nlm_u16     topRanges[][NLM_RANGE_NUM_BOUNDS],
    nlm_u8 validBits,
    NlmRangeType             rangeType,
    NlmReasonCode    *o_reason
    );

typedef NlmErrNum_t (*NlmXkRangeMgr__RangeLTR)(
    struct NlmRangeMgr_s        *pRangeMgr,
    nlm_u8 ltrNum,
    NlmRangeSrchAttrs *srchAttrs,
    NlmReasonCode    *o_reason
    );

/* Virtual Table for Range manager */
typedef struct NlmRangeMgr__vtbl_s
{
    NlmXkRangeMgr__InitCodeRegs         m_initCodeRegsFnPtr;
    NlmXkRangeMgr__InitBoundRegs        m_initBoundRegsFnPtr;
    NlmXkRangeMgr__RangeLTR             m_initRangeLtrFnPtr;
} NlmRangeMgr__vtbl;

typedef struct NlmRangeMgr_s
{  
    void                *m_devMgr_p;    /* Pointer to device manager module used by range manager */
    NlmDevType          m_devType;      /* type of device RangeMgr is operating on   */
    NlmPortMode         m_portMode;     /* single or dual port */
    NlmSMTMode          m_smtMode;      /* smt or non-smt mode */
    NlmBankNum          m_bankNum;      /* bank number */
    NlmPortNum          m_portNum;      /* port number */
    NlmCmAllocator      *m_alloc_p;     /* memory allocator                          */
    NlmRangeDbList       m_rangeDb_list;    /* list of range Databases associated
                                               with RangeMgr */
    nlm_u16              m_rangeDb_count;   /* number of range Databases associated 
                                               with RangeMgr    */
    NlmRangeMgr__vtbl   m_vtbl;

#if defined NLM_MT_OLD || defined NLM_MT
    NlmCmMtSpinlock     m_spinLock; /* Spin lock to protect data consistency with multi-threading*/
#endif

} NlmRangeMgr;

extern NlmRangeMgr* NlmRangeMgr__Init(
    NlmCmAllocator      *alloc,
    void                *devMgr,
    NlmDevType          devType,
    NlmBankNum          bankNum,
    NlmPortNum          portNum,
    NlmRangeEncodingType* encodingType,
    NlmReasonCode   *o_reason
    );

extern NlmRangeDb* NlmRangeMgr__CreateDb(
    NlmRangeMgr         *pRangeMgr,
    nlm_u8               id,    
    NlmRangeDbAttrSet   *rmDbAttributes,    
    NlmReasonCode       *o_reason
    );

extern NlmErrNum_t NlmRangeMgr__DestroyDb(
    NlmRangeMgr     *pRangeMgr,
    NlmRangeDb      *existingDb,
    NlmReasonCode   *o_reason
    );

extern NlmErrNum_t NlmRangeMgr__AddRange(
    NlmRangeMgr     *pRangeMgr,
    NlmRangeDb      *existingDb,
    NlmRange        *newRange,
    NlmReasonCode   *o_reason
    );

extern NlmErrNum_t NlmRangeMgr__DeleteRange(
    NlmRangeMgr     *pRangeMgr,
    NlmRangeDb      *existingDb,
    nlm_u32         id,
    NlmReasonCode   *o_reason
    );

extern NlmErrNum_t NlmRangeMgr__CreateEncodings(
    NlmRangeMgr     *pRangeMgr,
    NlmRangeDb      *existingDb,
    NlmReasonCode   *o_reason
    ); 

extern NlmErrNum_t NlmRangeMgr__AssignRange(
    NlmRangeMgr     *pRangeMgr,
    NlmRangeDb      *existingDb,
    NlmRangeType    whichRange,
    NlmReasonCode   *o_reason
    );  

extern NlmRangeEncoded* NlmRangeMgr__GetRangeEncoding(
    NlmRangeMgr         *pRangeMgr,
    NlmRangeDb          *existingDb,
    nlm_u32             id,
    NlmReasonCode       *o_reason
    ); 

extern NlmRangeDbStats* NlmRangeMgr__GetDbStats(
    NlmRangeMgr         *pRangeMgr,
    NlmRangeDb          *existingDb,
    NlmReasonCode       *o_reason
    );

extern NlmErrNum_t NlmRangeMgr__ConfigRangeMatching(
    NlmRangeMgr *self,
    nlm_u8 ltrNum,
    NlmRangeSrchAttrs *srchAttrs,
    NlmReasonCode *o_reason
    );

extern NlmErrNum_t NlmRangeMgr__Destroy(
    NlmRangeMgr      *pRangeMgr,
    NlmReasonCode   *o_reason
    );



extern NlmRangeMgr* kbp_rm_init(
     NlmCmAllocator      *alloc,
     void                *devMgr,
     NlmDevType          devType,
     NlmBankNum          bankNum,
     NlmPortNum          portNum,
     NlmRangeEncodingType* encodingType,
     NlmReasonCode   *o_reason
     );
 
 extern NlmRangeDb* kbp_rm_create_db(
     NlmRangeMgr         *pRangeMgr,
     nlm_u8               id,    
     NlmRangeDbAttrSet   *rmDbAttributes,    
     NlmReasonCode       *o_reason
     );
 
 extern NlmErrNum_t kbp_rm_destroy_db(
     NlmRangeMgr     *pRangeMgr,
     NlmRangeDb      *existingDb,
     NlmReasonCode   *o_reason
     );
 
 extern NlmErrNum_t kbp_rm_add_range(
     NlmRangeMgr     *pRangeMgr,
     NlmRangeDb      *existingDb,
     NlmRange        *newRange,
     NlmReasonCode   *o_reason
     );
 
 extern NlmErrNum_t kbp_rm_delete_range(
     NlmRangeMgr     *pRangeMgr,
     NlmRangeDb      *existingDb,
     nlm_u32         id,
     NlmReasonCode   *o_reason
     );
 
 extern NlmErrNum_t kbp_rm_create_encodings(
     NlmRangeMgr     *pRangeMgr,
     NlmRangeDb      *existingDb,
     NlmReasonCode   *o_reason
     ); 
 
 extern NlmErrNum_t kbp_rm_assign_range(
     NlmRangeMgr     *pRangeMgr,
     NlmRangeDb      *existingDb,
     NlmRangeType    whichRange,
     NlmReasonCode   *o_reason
     );  
 
 extern NlmRangeEncoded* kbp_rm_get_range_encoding(
     NlmRangeMgr         *pRangeMgr,
     NlmRangeDb          *existingDb,
     nlm_u32             id,
     NlmReasonCode       *o_reason
     ); 
 
 extern NlmRangeDbStats* kbp_rm_get_db_stats(
     NlmRangeMgr         *pRangeMgr,
     NlmRangeDb          *existingDb,
     NlmReasonCode       *o_reason
     );
 
 extern NlmErrNum_t kbp_rm_config_range_matching(
     NlmRangeMgr *self,
     nlm_u8 ltrNum,
     NlmRangeSrchAttrs *srchAttrs,
     NlmReasonCode *o_reason
     );
 
 extern NlmErrNum_t kbp_rm_destroy(
     NlmRangeMgr      *pRangeMgr,
     NlmReasonCode   *o_reason
     );

#ifndef NLMPLATFORM_BCM

 extern NlmRangeMgr* bcm_kbp_rm_init(
     NlmCmAllocator      *alloc,
     void                *devMgr,
     NlmDevType          devType,
     NlmBankNum          bankNum,
     NlmPortNum          portNum,
     NlmRangeEncodingType* encodingType,
     NlmReasonCode   *o_reason
     );
 
 extern NlmRangeDb* bcm_kbp_rm_create_db(
     NlmRangeMgr         *pRangeMgr,
     nlm_u8               id,    
     NlmRangeDbAttrSet   *rmDbAttributes,    
     NlmReasonCode       *o_reason
     );
 
 extern NlmErrNum_t bcm_kbp_rm_destroy_db(
     NlmRangeMgr     *pRangeMgr,
     NlmRangeDb      *existingDb,
     NlmReasonCode   *o_reason
     );
 
 extern NlmErrNum_t bcm_kbp_rm_add_range(
     NlmRangeMgr     *pRangeMgr,
     NlmRangeDb      *existingDb,
     NlmRange        *newRange,
     NlmReasonCode   *o_reason
     );
 
 extern NlmErrNum_t bcm_kbp_rm_delete_range(
     NlmRangeMgr     *pRangeMgr,
     NlmRangeDb      *existingDb,
     nlm_u32         id,
     NlmReasonCode   *o_reason
     );
 
 extern NlmErrNum_t bcm_kbp_rm_create_encodings(
     NlmRangeMgr     *pRangeMgr,
     NlmRangeDb      *existingDb,
     NlmReasonCode   *o_reason
     ); 
 
 extern NlmErrNum_t bcm_kbp_rm_assign_range(
     NlmRangeMgr     *pRangeMgr,
     NlmRangeDb      *existingDb,
     NlmRangeType    whichRange,
     NlmReasonCode   *o_reason
     );  
 
 extern NlmRangeEncoded* bcm_kbp_rm_get_range_encoding(
     NlmRangeMgr         *pRangeMgr,
     NlmRangeDb          *existingDb,
     nlm_u32             id,
     NlmReasonCode       *o_reason
     ); 
 
 extern NlmRangeDbStats* bcm_kbp_rm_get_db_stats(
     NlmRangeMgr         *pRangeMgr,
     NlmRangeDb          *existingDb,
     NlmReasonCode       *o_reason
     );
 
 extern NlmErrNum_t bcm_kbp_rm_config_range_matching(
     NlmRangeMgr *self,
     nlm_u8 ltrNum,
     NlmRangeSrchAttrs *srchAttrs,
     NlmReasonCode *o_reason
     );
 
 extern NlmErrNum_t bcm_kbp_rm_destroy(
     NlmRangeMgr      *pRangeMgr,
     NlmReasonCode   *o_reason
     );
#endif

 

 /* ------------------------------ */
#ifndef NLMPLATFORM_BCM
#include "nlmcmexterncend.h"
#else
#include <soc/kbp/common/nlmcmexterncend.h>
#endif
#endif /* #ifndef INCLUDED_NLMRANGEMGR_H */
