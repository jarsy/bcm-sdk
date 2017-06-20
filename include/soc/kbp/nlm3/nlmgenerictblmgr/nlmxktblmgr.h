/*
 * $Id: nlmxktblmgr.h,v 1.1.6.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef INCLUDED_NLMXKTBLMGR_H
#define INCLUDED_NLMXKTBLMGR_H

#ifndef NLMPLATFORM_BCM
#include "nlmcmbasic.h"
#include "nlmcmallocator.h"
#include "nlmerrorcodes.h"
#include "nlmcmexterncstart.h"
#else
#include <soc/kbp/common/nlmcmbasic.h>
#include <soc/kbp/common/nlmcmallocator.h>
#include <soc/kbp/common/nlmerrorcodes.h>
#include <soc/kbp/common/nlmcmexterncstart.h>
#endif

/* function pointers for virtual table */
typedef  NlmErrNum_t (*NlmXkTblMgr__CreateTable_t)(
                NlmGenericTblMgr  *genericTblMgr_p,
                NlmPortNum          portNum,
                NlmGenericTbl* table_p,
                NlmReasonCode    *o_reason
                );



    

typedef  NlmErrNum_t (*NlmXkTblMgr__ConfigSearch_t)(
        NlmGenericTblMgr  *genericTblMgr,
        NlmPortNum          portNum,
        nlm_u8             ltrNum, 
        NlmGenericTblSearchAttributes  *searchAttrs,
        NlmReasonCode    *o_reason
        );


typedef  NlmErrNum_t (*NlmXkTblMgr__LockConfiguration)(
            NlmGenericTblMgr  *genericTblMgr,
            NlmPortNum          portNum,
            NlmReasonCode     *o_reason
        );

typedef  NlmErrNum_t (*NlmXkTblMgr__AddRecord_t)(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmGenericTblRecord *tblRecord,
    nlm_u8          *assocData,
    nlm_u16              recordPriority, 
    NlmRecordIndex      *o_recordIndex,
    nlm_u16             GroupId,
    NlmReasonCode       *o_reason
    );

typedef  NlmErrNum_t (*NlmXkTblMgr__FindRecord_t)(
    NlmGenericTbl       *genericTbl,
    NlmGenericTblRecord *tblRecord,
    NlmRecordIndex      *o_recordIndex,
    NlmReasonCode       *o_reason
    );

typedef  NlmErrNum_t (*NlmXkTblMgr__DeleteRecord_t)(
    NlmGenericTbl       *genericTbl,
    NlmPortNum          portNum,
    NlmRecordIndex      recordIndex,
    NlmReasonCode       *o_reason
    );

typedef  NlmErrNum_t (*NlmXkTblMgr__UpdateRecord_t)(
    NlmGenericTbl            *genericTbl,
    NlmPortNum          portNum,
    NlmGenericTblRecord     *tblRecord,
    nlm_u8          *assocData,
    NlmRecordIndex           recordIndex,
    NlmReasonCode       *o_reason
    );


typedef  NlmErrNum_t  (*NlmXkTblMgr__Destroy_t)(
            NlmGenericTblMgr *genericTblMgr,    
            NlmPortNum          portNum,
            NlmReasonCode    *o_reason
        );


typedef NlmErrNum_t (*NlmXkTblMgr__DestroyTable_t) (
            NlmGenericTblMgr  *genericTblMgr_p,
            NlmPortNum          portNum,
            NlmGenericTbl     *genericTbl,
            NlmReasonCode     *o_reason);

typedef NlmErrNum_t (*NlmXkTblMgr__Iter_next_t)(
            NlmGenericTblIterHandle *iterHandle,
            NlmGenericTblIterData  *iterInfo);

typedef NlmErrNum_t (*NlmXkTblMgr__GetAdInfo_t)(
            NlmGenericTblMgr  *genericTblMgr_p,
			NlmTblAssoDataWidth	adWidth,
            NlmRecordIndex	  recordIndex,
			NlmGenericTblAdInfoReadType RdType,
			NlmGenericTblAdInfo *adInfo,
			NlmReasonCode *o_reason);

/* VTable for Table manager */
typedef struct NlmTblMgr__pvt_vtbl
{
    NlmXkTblMgr__CreateTable_t          CreateTable;
    NlmXkTblMgr__ConfigSearch_t         ConfigSearch;
    NlmXkTblMgr__LockConfiguration      LockConfiguration;
    NlmXkTblMgr__AddRecord_t            AddRecord;
	NlmXkTblMgr__FindRecord_t            FindRecord;
    NlmXkTblMgr__DeleteRecord_t         DeleteRecord;
    NlmXkTblMgr__UpdateRecord_t         UpdateRecord;
    NlmXkTblMgr__Destroy_t              Destroy;
    NlmXkTblMgr__DestroyTable_t         DestroyTable;
    NlmXkTblMgr__Iter_next_t            iter_next;
	NlmXkTblMgr__GetAdInfo_t			GetAdInfo;

} NlmTblMgr__pvt_vtbl;

#ifndef NLMPLATFORM_BCM
#include "nlmcmexterncend.h"
#else
#include <soc/kbp/common/nlmcmexterncend.h>
#endif

#endif /* INCLUDED_NLMXKTBLMGR_H */
