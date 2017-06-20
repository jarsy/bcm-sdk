/*
 * $Id: nlmgenerictblmgr.h,v 1.1.6.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

 
/*@@Nlm_GenericTableManager Module
   Summary:
    This module manages the generic tables contaning ACLs(Access control lists) 
    database into the device and decides the contents of various registers for 
    search operations. Managing the database is a fully automated system and 
    intelligent enough to make the optimal use of database space. 
    
    This file works as API between user and the module. The file also works as an 
    upper level integration layer for all Processors. It defines user friendly 
    data structures for instructions of device and API functions 
    declarations.
*/

#ifndef INCLUDED_NLMGENERICTBLMGR_H
#define INCLUDED_NLMGENERICTBLMGR_H

#if defined NLM_MT_OLD || defined NLM_MT
#ifndef NLMPLATFORM_BCM
#include <nlmcmmt.h>
#else
#include <soc/kbp/common/nlmcmmt.h>
#endif
#endif

#ifndef NLMPLATFORM_BCM
#include "nlmcmbasic.h"
#include "nlmcmallocator.h"
#include "nlmcmdbllinklist.h"
#include "nlmerrorcodes.h"
#include "nlmcmdevice.h"
#include "nlmarch.h"
#include "nlmdevmgr.h"
#include "nlmudamemmgr.h"
#include "nlmcmexterncstart.h"
#else
#include<soc/kbp/common/nlmcmbasic.h>
#include<soc/kbp/common/nlmcmallocator.h>
#include<soc/kbp/common/nlmcmdbllinklist.h>
#include<soc/kbp/common/nlmerrorcodes.h>
#include<soc/kbp/common/nlmcmdevice.h>
#include<soc/kbp/nlm3/arch/nlmarch.h>
#include<soc/kbp/nlm3/nlmdevmgr/nlmdevmgr.h>
#include<soc/kbp/nlm3/nlmgenerictblmgr/nlmudamemmgr.h>
#include<soc/kbp/common/nlmcmexterncstart.h>
#endif


#define NLM_GTM_INVALID_INDEX                   (0xFFFFFFFF)

/* DBA super block ranges */
#define NLM_GTM_DBA_SB_BLK_RANGE_WHOLE_DEVICE   (NLMDEV_NUM_SUPER_BLOCKS) 
#define NLM_GTM_DBA_SB_BLK_RANGE_NO_BLK_USED    (0xFF)

/* UDA super block ranges */
#define NLM_GTM_UDA_SB_BLK_RANGE_WHOLE_DEVICE   (NLMDEV_NUM_SRAM_SUPER_BLOCKS)
#define NLM_GTM_UDA_SB_BLK_RANGE_NO_BLK_USED    (0xFF)

/* block ranges */
#define NLM_GTM_BLK_RANGE_WHOLE_DEVICE          (NLMDEV_NUM_ARRAY_BLOCKS) 
#define NLM_GTM_BLK_RANGE_NO_BLK_USED           (0xFFFF)

#define NLM_GTM_NUM_KEYS                    (NLMDEV_NUM_KEYS)
#define NLM_GTM_NUM_PARALLEL_SEARCHES       (NLMDEV_NUM_PARALLEL_SEARCHES)
#define NLM_GTM_NUM_KEY_CONSTRUCT_SEGMENTS  (NLMDEV_NUM_OF_SEGMENTS_PER_KEY)

/* Attributes specific to GTM table */
typedef enum NlmGtmAttribType
{
    NLM_GTM_ATTRIB_DEFAULT,  
    NLM_GTM_ATTRIB_SEPARATE_BLOCK   /* Keep table in separate block to avoid table id as part of record */
}NlmGtmAttribType;
/* 
enum for permissible width of table. Table can have width of 80 bit, 160 bit,
320 bit or 640 bit.
*/
typedef enum NlmGenericTblWidth
{
    NLM_TBL_WIDTH_80  = 80,
    NLM_TBL_WIDTH_160 = 160,
    NLM_TBL_WIDTH_320 = 320,
    NLM_TBL_WIDTH_640 = 640,

    NLM_TBL_WIDTH_END
}NlmGenericTblWidth;

typedef NlmTblAssoDataWidth NlmGenericTblAssoWidth;


typedef struct NlmGenericTblMgrBlksRange
{
    nlm_u16 m_startBlkNum;  /* Initializing start blk or end blk to NLM_GTM_BLK_RANGE_WHOLE_DEVICE
                           indicates all blocks of the device are allocated to GTM 
                           whereas initializing it to NLM_GTM_BLK_RANGE_NO_BLK_USED indicates
                           no block of the device are allocated to GTM*/ 

    nlm_u16 m_endBlkNum;     /* Note: Ensure that Start Blk < End Blk; Start Blk and End blk should be at super blk boundary */  
} NlmGenericTblMgrBlksRange;

typedef struct NlmGenericTblMgrSBRange
{
    nlm_u16 m_stSBNum;  /* Initializing start super blk or end super blk to NLM_GTM_DBA_SB_BLK_RANGE_WHOLE_DEVICE
                           indicates all dba blocks of the device are allocated to GTM 
                           where as initializing it to NLM_GTM_DBA_SB_BLK_RANGE_NO_BLK_USED indicates
                           no dba block of the device are allocated to GTM*/ 

    nlm_u16 m_endSBNum;     /* Note: Ensure that Start SB < End SB; Start Super Blk and End Super Blk 
                            should be at AC boundary (2 SB per AC) for DBA Sb */  
} NlmGenericTblMgrSBRange;

/* 
Structure for Generic Table Manager.
Generic Table Manager consists of its name which is a string, devType(device type)
for which this GTM will be created, doubly link list of generic tables which is
maintained by GTM itself, TableCount(maintained by GTM itself), flag ConfigLocked,
tblMgr pointer which is currently a Table_Manager pointer, assigned and used
internally by GTM. vtbl(Virtual Table maintained and internally used by GTM itself).
m_client_p is a client pointer which must be provided by user. When calling the call
back functions of client, m_client_p will be required and used by user.
*/
typedef struct NlmGenericTblMgr
{
    NlmCmAllocator  *m_alloc_p;           /*  Memory allocator */    
    
    NlmDevType      m_devType;           /*  type of device GenericTblMgr is operating on */
    
    NlmGenericTblMgrSBRange   *m_genericTblSBRange; /* SB ranges per bank */
    NlmGenericTblMgrBlksRange *m_genericTblBlksRange; /* for internal use; managed
                                                           according the bank */
   NlmGenericTblMgrSBRange   *m_udaSBRange; /* SB ranges per bank */                                                            
    
    NlmCmDblLinkList *m_genericTbl_list_p; /*  List of generic tables associated
                                              with GenericTblMgr                           */
    nlm_u16          m_genericTbl_count;  /*  Number of generic tables associated
                                              with GenericTblMgr                           */
    NlmBool          m_IsConfigLocked;

    void*            m_tblMgr_p;        /* This is a void pointer so that neither the application nor
                                      Generic Table Manager knowns about the internal
                                      data structures. */

    void*           m_vtbl_p;        /*Application should not know about the vtbl layout */
    void*           m_client_p;    

    NlmBankNum     m_bankType;      /* Bank_0 or Bank_1 */

#if defined NLM_MT_OLD || defined NLM_MT

    NlmCmMtSpinlock     m_spinLock; /* Spin lock to protect data consistency with multi-threading*/
#endif

} NlmGenericTblMgr;

typedef struct NlmGenericTblIterData
{
    nlm_u8 *data;
    nlm_u8 *mask;
    nlm_u8 *AssoData;
    nlm_u32 index;
    nlm_u32 groupId;
    nlm_u16 priority;
}NlmGenericTblIterData;

struct NlmGenericTbl;


typedef struct NlmGenericTblIterHandle
{
    struct NlmGenericTbl *m_tbl_p;
    nlm_u16 curBlock;
    nlm_u32 CurRecOffset;
    nlm_u32 recCounter;
    NlmBool isRecsAvailable;
}NlmGenericTblIterHandle;

/* 
Structure representing Generic Table. 
A generic table consists of its its table_ID which is a string of 
string length specified in GTM Init, width of table records(A table can not have records
with varying lengths), list of tables which are searched in parallel and
tableMaxSize(maximum number of records the table can have). Table should have
Table manager pointer to which the table belongs to. privateData is assigned
and used for internal purpose. If the maximum number of table records are not
limited then tableMaxSize should be 0.
*/
typedef struct NlmGenericTbl
{
    NlmGenericTblMgr    *m_genericTblMgr_p;  /*  NlmGenericTblMgr to which this table belongs to */
    NlmGenericTblIterHandle *m_iterHandle; /* Handle for iterator */
    NlmCmAllocator      *m_alloc_p;          /*  Memory allocator                                */
    nlm_u8              m_tblId;         /*  Table id assigned to this table                 */ 
    nlm_u8              m_keepSeparate;    /* keep the table separately in block, should not be mixed with other table records*/
    NlmGenericTblWidth   m_width;            /*  Table width, record length                  */
    NlmGenericTblAssoWidth   m_adWidth;            /*  AD width                  */
    nlm_u32             m_num_records;      /*  Number of records currently in the table        */
    
    NlmPsTblList        *m_psTblList_p;      /*  List of tables searched in parallel with
                                                  this table                                      */
    nlm_u32             m_tableMaxSize;   /*  maximum number of records, table will have if   
                                              table is static. If table is dynamic, the MaxSize
                                              will be ZERO  
                                              */
    NlmBool             m_isLtrConfigured;    /*   True if atleast one ltr is configured for the table */
    void*               m_privateData;

} NlmGenericTbl;


/*
Generic table list structure.
Doubly link list is created by inheriting nlmcmdbllinklist. 
One instance of the structure is one node contaning one generic table.
*/
typedef struct NlmGenericTblList
{
    #include "nlmcmdbllinklistdata.h"
    NlmGenericTbl* m_gt_p;

}NlmGenericTblList;



/* This structure defines key construction mapping for a parallel search.
Each key can be formed by picking upto 10 different segments of compare data
with each segment being from 1-16 bytes. A value of "0" in m_segmentNumOfBytes 
indicates that the next segments are invalid and needs to be ignored; 
Value greater than 79 for m_segmentStartByte is invalid. 
 */
typedef struct NlmGenericTblKeyConstructionMap
{   
    nlm_u8    m_segmentStartByte[NLM_GTM_NUM_KEY_CONSTRUCT_SEGMENTS];
    nlm_u8    m_segmentNumOfBytes[NLM_GTM_NUM_KEY_CONSTRUCT_SEGMENTS];
    nlm_u8    m_segmentIsZeroFill[NLM_GTM_NUM_KEY_CONSTRUCT_SEGMENTS];
} NlmGenericTblKeyConstructionMap;

/*
This structure maps each parallel search with a result sequence number, key number and
key construct mappings.
*/
typedef struct NlmGenericTblParallelSearchInfo
{
    nlm_u8  m_tblId;
    NlmDevParallelSrch m_rsltPortNum;
    NlmDevKey m_keyNum; 
    NlmGenericTblKeyConstructionMap   m_kcm;
}NlmGenericTblParallelSearchInfo;

/* This structure contains the attributes associated with Generic Tables searches that are
performed for a specified LTR.
Upto 4 parallel searches can be performed per LTR
*/
typedef struct NlmGenericTblSearchAttributes
{
    nlm_u8 m_numOfParallelSrches; 
    NlmGenericTblParallelSearchInfo m_psInfo[NLM_GTM_NUM_PARALLEL_SEARCHES]; 
    NlmBool m_isCmp3Search;
} NlmGenericTblSearchAttributes;

typedef struct NlmGenericTblRecord
{
    nlm_u8    *m_data; /* Data portion of the record */
    nlm_u8    *m_mask; /* Mask portion of the record */
    nlm_u16    m_len;  /* Record length in bits */
} NlmGenericTblRecord;


typedef struct NlmGenericTblAdInfo
{
    nlm_u8   m_adData[NLMDEV_MAX_AD_LEN_IN_BYTES];
	nlm_u32	 m_adIndex;
} NlmGenericTblAdInfo;

typedef enum NlmGenericTblAdInfoReadType
{
    GTM_AD_READ_FROM_SW = 0,
	GTM_AD_READ_FROM_HW

} NlmGenericTblAdInfoReadType;



/* Add a callback function to indicate that index has changed  */
/* Callback registered by the application. This callback is called when
the index of a record changes so that the application can make changes in ADM 
client_p is the ptr passed by the appliccation during init so that data 
needed by the application can be accessed in the callback 
*/
typedef void (*NlmIndexChangedAppCb)(
                void* client_p,   
                NlmGenericTbl* genericTbl_p, /* ptr to table whose record is shuffled  */ 
                NlmRecordIndex oldIndex,  /* old index of the record that was shuffled */
                NlmRecordIndex newIndex   /* new index of the record that was shuffled */
                );

/*
Creates and initializes the Generic Table Manager inside the control plane software.
The Generic Table Manager needs to be given a target device it should work with.
The Generic Table Manager is designed with any generic Processor architectures.
Only one Generic Table Manager can be instantiated per set of devices.
Return value of 0 indicates success, and anything else is failure. The initialized
Generic Table Manager is returned, and the pointer has to be used for further
function calls.
client_p is the pointer passed from the application, that is passed back in 
the index changed callback to the application (in case, the application wants
to access its specific data structures in the callback. 
*/
extern NlmGenericTblMgr*
NlmGenericTblMgr__Init(
        NlmCmAllocator              *alloc_p,
        void                        *devMgr_p,      
        NlmDevType                  devType,
        NlmBankNum                  bankNum,
        nlm_u8                      numOfDevices,
        NlmGenericTblMgrSBRange     *dbaSbRange,
        NlmGenericTblMgrSBRange     *udaSbRange,
        NlmIndexChangedAppCb        indexChangedAppCb,
        void                        *client_p,
        NlmReasonCode               *o_reason
        );

/*
Destroys the GTM moduleand frees all its associated memory.
Before destroying the GTM, user should should destroy all existing tables.
If a table exist, GTM can not be destroyed. Return value of 0 indicates 
success, and anything else is failure.
*/
extern NlmErrNum_t  
NlmGenericTblMgr__Destroy(
            NlmGenericTblMgr *genericTblMgr,    
            NlmPortNum          portNum,
            NlmReasonCode    *o_reason
            );

/* 
Adds a new generic table to the Generic Table Manager. Key searched within this table must contain this table id
at the MSB portion of the key. A table can be configured with width of 80, 160, 320
or 640. Table size can be static or dynamic. If table is static Table_Size must have
a non zero value. If table is dynamic Table_Size must have zero value. Return value of 
0 indicates success, and anything else is failure.
Table pointer is returned, which can be used for further function calls.
*/
extern NlmGenericTbl*
NlmGenericTblMgr__CreateTable(
        NlmGenericTblMgr    *genericTblMgr,
        NlmPortNum          portNum,
        nlm_u8              genericTblId, 
        NlmGenericTblWidth  genericTblWidth,        
        NlmGenericTblAssoWidth      adWidth,
        nlm_u32             genericTblSize,
        NlmReasonCode       *o_reason
        );


/* 
This function destroys a table identified by genericTbl.
Once a table is destroyed, it can not participate in search and all table 
records associated with the table will deleted from device database.
Return value of 0 indicates success, and anything else is failure.
*/
extern NlmErrNum_t  
NlmGenericTblMgr__DestroyTable(
    NlmGenericTblMgr  *genericTblMgr,
    NlmPortNum        portNum,
    NlmGenericTbl     *genericTbl,
    NlmReasonCode     *o_reason
    );

/*
This function configures an ltr with the searches attributes. 
It is required to configure a search only after tables participating in the 
search are created.  If more than one table is configured for an LTR, then
these tables are said to be parallely searched
Return value of NLMERR_OK indicates success, and anything else is failure.
*/
extern NlmErrNum_t  
NlmGenericTblMgr__ConfigSearch(
        NlmGenericTblMgr  *genericTblMgr,
        NlmPortNum          portNum,
        nlm_u8             ltrNum, 
        NlmGenericTblSearchAttributes  *searchAttrs,
        NlmReasonCode    *o_reason
        );

/*
This function locks the configuration of the Table Manager.
After creating all the tables and setting the dependencies if any, this
API needs to be called. After locking the configuration, user will not be able
to change the search attributes of any table. Return value of 0 indicates 
success, and anything else is failure.
*/
extern NlmErrNum_t  
NlmGenericTblMgr__LockConfiguration (
            NlmGenericTblMgr  *genericTblMgr,
            NlmPortNum          portNum,
            NlmReasonCode    *o_reason
            );

/* This function adds a record to a table identified with 'genericTbl'.
 * It is necessary to provide the explicit priority for the record.
 * Index assigned to the record is given as output in 'o_recordIndex'
 * Return value of 0 indicates success, and anything else is failure.
 */
extern NlmErrNum_t  
NlmGenericTblMgr__AddRecord(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8          *assocData,
    nlm_u16              groupId,
    nlm_u16              recordPriority, 
    NlmRecordIndex      *o_recordIndex,
    NlmReasonCode       *o_reason
    );

/* This function updates an existing record, at given recordIndex, 
with the input valid record. Return value of 0 indicates success, 
and anything else is failure.
*/

NlmErrNum_t 
NlmGenericTblMgr__UpdateRecord(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8          *assocData,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
    );

/* 
This function deletes a record from the table identified with 'genericTbl'.
The record will be deleted from the device database so this particular
record can not be searched or read afterwards. Return value of 0 indicates 
success, and anything else is failure.
*/
extern NlmErrNum_t  
NlmGenericTblMgr__DeleteRecord(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
    );



/*
Creates and initializes the Generic Table Manager inside the control plane software.
The Generic Table Manager needs to be given a target device it should work with.
The Generic Table Manager is designed with any generic Processor architectures.
Only one Generic Table Manager can be instantiated per set of devices.
Return value of 0 indicates success, and anything else is failure. The initialized
Generic Table Manager is returned, and the pointer has to be used for further
function calls.
client_p is the pointer passed from the application, that is passed back in 
the index changed callback to the application (in case, the application wants
to access its specific data structures in the callback. 
*/
extern NlmGenericTblMgr*
kbp_gtm_init(
        NlmCmAllocator              *alloc_p,
        void                        *devMgr_p,      
        NlmDevType                  devType,
        NlmBankNum                  bankNum,
        nlm_u8                      numOfDevices,
        NlmGenericTblMgrSBRange     *dbaSbRange,
        NlmGenericTblMgrSBRange     *udaSbRange,
        NlmIndexChangedAppCb        indexChangedAppCb,
        void                        *client_p,
        NlmReasonCode               *o_reason
        );

/*
Destroys the GTM moduleand frees all its associated memory.
Before destroying the GTM, user should should destroy all existing tables.
If a table exist, GTM can not be destroyed. Return value of 0 indicates 
success, and anything else is failure.
*/
extern NlmErrNum_t  
kbp_gtm_destroy(
            NlmGenericTblMgr *genericTblMgr,    
            NlmPortNum          portNum,
            NlmReasonCode    *o_reason
            );

/* 
Adds a new generic table to the Generic Table Manager. Key searched within this table must contain this table id
at the MSB portion of the key. A table can be configured with width of 80, 160, 320
or 640. Table size can be static or dynamic. If table is static Table_Size must have
a non zero value. If table is dynamic Table_Size must have zero value. Return value of 
0 indicates success, and anything else is failure.
Table pointer is returned, which can be used for further function calls.
*/
extern NlmGenericTbl*
kbp_gtm_create_table(
        NlmGenericTblMgr    *genericTblMgr,
        NlmPortNum          portNum,
        nlm_u8              genericTblId, 
        NlmGenericTblWidth  genericTblWidth,        
        NlmGenericTblAssoWidth      adWidth,
        nlm_u32             genericTblSize,
        NlmReasonCode       *o_reason
        );


/* 
This function destroys a table identified by genericTbl.
Once a table is destroyed, it can not participate in search and all table 
records associated with the table will deleted from device database.
Return value of 0 indicates success, and anything else is failure.
*/
extern NlmErrNum_t  
kbp_gtm_destroy_table(
    NlmGenericTblMgr  *genericTblMgr,
    NlmPortNum        portNum,
    NlmGenericTbl     *genericTbl,
    NlmReasonCode     *o_reason
    );

/*
This function configures an ltr with the searches attributes. 
It is required to configure a search only after tables participating in the 
search are created.  If more than one table is configured for an LTR, then
these tables are said to be parallely searched
Return value of NLMERR_OK indicates success, and anything else is failure.
*/
extern NlmErrNum_t  
kbp_gtm_config_search(
        NlmGenericTblMgr  *genericTblMgr,
        NlmPortNum          portNum,
        nlm_u8             ltrNum, 
        NlmGenericTblSearchAttributes  *searchAttrs,
        NlmReasonCode    *o_reason
        );

/*
This function locks the configuration of the Table Manager.
After creating all the tables and setting the dependencies if any, this
API needs to be called. After locking the configuration, user will not be able
to change the search attributes of any table. Return value of 0 indicates 
success, and anything else is failure.
*/
extern NlmErrNum_t  
kbp_gtm_lock_config (
            NlmGenericTblMgr  *genericTblMgr,
            NlmPortNum          portNum,
            NlmReasonCode    *o_reason
            );

/* This function adds a record to a table identified with 'genericTbl'.
 * It is necessary to provide the explicit priority for the record.
 * Index assigned to the record is given as output in 'o_recordIndex'
 * Return value of 0 indicates success, and anything else is failure.
 */
extern NlmErrNum_t  
kbp_gtm_add_record(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8          *assocData,
    nlm_u16              groupId,
    nlm_u16              recordPriority, 
    NlmRecordIndex      *o_recordIndex,
    NlmReasonCode       *o_reason
    );

/* This function finds a record identified by tblRecord in the Shadow Memory.
 * It is necessary to provide the tblRecord in D-M format.
 * Index associated with the record is given as output in 'o_recordIndex'
 * Return value of 0 indicates success, and anything else is failure.
 */
 
extern NlmErrNum_t 
kbp_gtm_find_record(
    NlmGenericTbl       *genericTbl,
    NlmGenericTblRecord *tblRecord,
    NlmRecordIndex      *o_recordIndex,
    NlmReasonCode       *o_reason
    );
/* This function updates an existing record, at given recordIndex, 
with the input valid record. Return value of 0 indicates success, 
and anything else is failure.
*/

NlmErrNum_t 
kbp_gtm_update_record(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8          *assocData,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
    );

/* 
This function deletes a record from the table identified with 'genericTbl'.
The record will be deleted from the device database so this particular
record can not be searched or read afterwards. Return value of 0 indicates 
success, and anything else is failure.
*/
extern NlmErrNum_t  
kbp_gtm_delete_record(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
    );

extern NlmErrNum_t
kbp_gtm_iter_next(
    NlmGenericTblIterHandle *iterHandle,
    NlmGenericTblIterData  *iterInfo,
    NlmReasonCode    *reasoncode
    );

extern NlmErrNum_t
kbp_gtm_iter_init(
    NlmGenericTbl       *genericTbl,
    NlmGenericTblIterHandle **iterHandle);


extern NlmErrNum_t
kbp_gtm_set_attribute(
    NlmGenericTbl       *genericTbl,
    NlmGtmAttribType    attribType,     /* Attribute type*/
    nlm_u32  attribValue,       /* Attribute value*/
    NlmReasonCode *o_reason  
    );

extern NlmErrNum_t
kbp_gtm_get_ad_info(
    NlmGenericTbl       *genericTbl,
	NlmGenericTblRecord *tblRecord, /*Record information : input Param */
    NlmGenericTblAdInfoReadType readType, /*Read from H/W or S/W : input Param */
    NlmGenericTblAdInfo *adInfo, /*AD information : out Param */
    NlmReasonCode *o_reason  
    );


#ifndef NLMPLATFORM_BCM

extern NlmGenericTblMgr*
bcm_kbp_gtm_init(
        NlmCmAllocator              *alloc_p,
        void                        *devMgr_p,      
        NlmDevType                  devType,
        NlmBankNum                  bankNum,
        nlm_u8                      numOfDevices,
        NlmGenericTblMgrSBRange     *dbaSbRange,
        NlmGenericTblMgrSBRange     *udaSbRange,
        NlmIndexChangedAppCb        indexChangedAppCb,
        void                        *client_p,
        NlmReasonCode               *o_reason
        );

/*
Destroys the GTM moduleand frees all its associated memory.
Before destroying the GTM, user should should destroy all existing tables.
If a table exist, GTM can not be destroyed. Return value of 0 indicates 
success, and anything else is failure.
*/
extern NlmErrNum_t  
bcm_kbp_gtm_destroy(
            NlmGenericTblMgr *genericTblMgr,    
            NlmPortNum          portNum,
            NlmReasonCode    *o_reason
            );

/* 
Adds a new generic table to the Generic Table Manager. Key searched within this table must contain this table id
at the MSB portion of the key. A table can be configured with width of 80, 160, 320
or 640. Table size can be static or dynamic. If table is static Table_Size must have
a non zero value. If table is dynamic Table_Size must have zero value. Return value of 
0 indicates success, and anything else is failure.
Table pointer is returned, which can be used for further function calls.
*/
extern NlmGenericTbl*
bcm_kbp_gtm_create_table(
        NlmGenericTblMgr    *genericTblMgr,
        NlmPortNum          portNum,
        nlm_u8              genericTblId, 
        NlmGenericTblWidth  genericTblWidth,        
        NlmGenericTblAssoWidth      adWidth,
        nlm_u32             genericTblSize,
        NlmReasonCode       *o_reason
        );


/* 
This function destroys a table identified by genericTbl.
Once a table is destroyed, it can not participate in search and all table 
records associated with the table will deleted from device database.
Return value of 0 indicates success, and anything else is failure.
*/
extern NlmErrNum_t  
bcm_kbp_gtm_destroy_table(
    NlmGenericTblMgr  *genericTblMgr,
    NlmPortNum        portNum,
    NlmGenericTbl     *genericTbl,
    NlmReasonCode     *o_reason
    );

/*
This function configures an ltr with the searches attributes. 
It is required to configure a search only after tables participating in the 
search are created.  If more than one table is configured for an LTR, then
these tables are said to be parallely searched
Return value of NLMERR_OK indicates success, and anything else is failure.
*/
extern NlmErrNum_t  
bcm_kbp_gtm_config_search(
        NlmGenericTblMgr  *genericTblMgr,
        NlmPortNum          portNum,
        nlm_u8             ltrNum, 
        NlmGenericTblSearchAttributes  *searchAttrs,
        NlmReasonCode    *o_reason
        );

/*
This function locks the configuration of the Table Manager.
After creating all the tables and setting the dependencies if any, this
API needs to be called. After locking the configuration, user will not be able
to change the search attributes of any table. Return value of 0 indicates 
success, and anything else is failure.
*/
extern NlmErrNum_t  
bcm_kbp_gtm_lock_config (
            NlmGenericTblMgr  *genericTblMgr,
            NlmPortNum          portNum,
            NlmReasonCode    *o_reason
            );

/* This function adds a record to a table identified with 'genericTbl'.
 * It is necessary to provide the explicit priority for the record.
 * Index assigned to the record is given as output in 'o_recordIndex'
 * Return value of 0 indicates success, and anything else is failure.
 */
extern NlmErrNum_t  
bcm_kbp_gtm_add_record(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8          *assocData,
    nlm_u16              groupId,
    nlm_u16              recordPriority, 
    NlmRecordIndex      *o_recordIndex,
    NlmReasonCode       *o_reason
    );

/* This function finds a record identified by tblRecord in the Shadow Memory.
 * It is necessary to provide the tblRecord in D-M format.
 * Index associated with the record is given as output in 'o_recordIndex'
 * Return value of 0 indicates success, and anything else is failure.
 */
extern NlmErrNum_t  
bcm_kbp_gtm_find_record(
    NlmGenericTbl       *genericTbl,
    NlmGenericTblRecord *tblRecord,
    NlmRecordIndex      *o_recordIndex,
    NlmReasonCode       *o_reason
    );


/* This function updates an existing record, at given recordIndex, 
with the input valid record. Return value of 0 indicates success, 
and anything else is failure.
*/

NlmErrNum_t 
bcm_kbp_gtm_update_record(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8          *assocData,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
    );

/* 
This function deletes a record from the table identified with 'genericTbl'.
The record will be deleted from the device database so this particular
record can not be searched or read afterwards. Return value of 0 indicates 
success, and anything else is failure.
*/
extern NlmErrNum_t  
bcm_kbp_gtm_delete_record(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
    );

extern NlmErrNum_t
bcm_kbp_gtm_iter_next(
    NlmGenericTblIterHandle *iterHandle,
    NlmGenericTblIterData  *iterInfo,
    NlmReasonCode    *reasoncode
    );

extern NlmErrNum_t
bcm_kbp_gtm_iter_init(
    NlmGenericTbl       *genericTbl,
    NlmGenericTblIterHandle **iterHandle);


extern NlmErrNum_t
bcm_kbp_gtm_set_attribute(
    NlmGenericTbl       *genericTbl,
    NlmGtmAttribType    attribType,     /* Attribute type*/
    nlm_u32  attribValue,       /* Attribute value*/
    NlmReasonCode *o_reason  
    );

#endif
#ifndef NLMPLATFORM_BCM
#include "nlmcmexterncend.h"
#else
#include <soc/kbp/common/nlmcmexterncend.h>
#endif
#endif /* INCLUDED_NLMGENERICTBLMGR_H */

