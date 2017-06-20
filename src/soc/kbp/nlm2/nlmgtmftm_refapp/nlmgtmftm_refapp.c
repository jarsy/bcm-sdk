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
 
#include "nlmgtmftm_refapp.h"
#ifdef NLM_XLP
#include "nlmxlpxpt.h"
#endif

/* Number of devices */
#define NLM_REFAPP_NUM_OF_DEVICES     1


static void NlmGtmFtmRefApp_PrintReasonCode(NlmReasonCode reason)
{
	NlmCm__printf("\tReason Code = ");

	switch( reason )
	{
	case NLMRSC_REASON_OK:
		NlmCm__printf("NLMRSC_REASON_OK");
		break;

    case NLMRSC_LOW_MEMORY:
        NlmCm__printf("NLMRSC_REASON_OK");
		break;

	case NLMRSC_INVALID_MEMALLOC_PTR:
		NlmCm__printf("NLMRSC_INVALID_MEMALLOC_PTR");
		break;

	case NLMRSC_INVALID_DEV_PTR:
		NlmCm__printf("NLMRSC_INVALID_DEV_PTR");
		break;

	case NLMRSC_INVALID_DEVMGR_PTR:
		NlmCm__printf("NLMRSC_INVALID_DEVMGR_PTR");
		break;

	case NLMRSC_INVALID_KEY_PTR:
		NlmCm__printf("NLMRSC_INVALID_KEY_PTR");
		break;

	case NLMRSC_INVALID_SRCH_RSLT_PTR:
		NlmCm__printf("NLMRSC_INVALID_SRCH_RSLT_PTR");
		break;

	case NLMRSC_INVALID_XPT_PTR:
		NlmCm__printf("NLMRSC_INVALID_XPT_PTR");
		break;

	case NLMRSC_INVALID_XPT_RQT_PTR:
		NlmCm__printf("NLMRSC_INVALID_XPT_RQT_PTR");
		break;

	case NLMRSC_INVALID_XPT_RSLT_PTR:
		NlmCm__printf("NLMRSC_INVALID_XPT_RSLT_PTR");
		break;

    case NLMRSC_INVALID_POINTER:
		NlmCm__printf("NLMRSC_INVALID_XPT_RSLT_PTR");
		break;

    case NLMRSC_INVALID_OUTPUT:
		NlmCm__printf("NLMRSC_INVALID_OUTPUT");
		break;

	case NLMRSC_INVALID_INPUT:
		NlmCm__printf("NLMRSC_INVALID_INPUT");
		break;

	case NLMRSC_INVALID_REG_ADDRESS:
		NlmCm__printf("NLMRSC_INVALID_REG_ADDRESS");
		break;

	case NLMRSC_INVALID_DB_ADDRESS:
		NlmCm__printf("NLMRSC_INVALID_DB_ADDRESS");
		break;

	case NLMRSC_INVALID_CB_ADDRESS:
		NlmCm__printf("NLMRSC_INVALID_CB_ADDRESS");
		break;

    case NLMRSC_INVALID_RANGE_ADDRESS:
		NlmCm__printf("NLMRSC_INVALID_CB_ADDRESS");
		break;

	case NLMRSC_INVALID_DATA:
		NlmCm__printf("NLMRSC_INVALID_DATA");
		break;

	case NLMRSC_INVALID_LTR_NUM:
		NlmCm__printf("NLMRSC_INVALID_LTR_NUM");
		break;

	case NLMRSC_INVALID_AB_NUM:
		NlmCm__printf("NLMRSC_INVALID_AB_NUM");
		break;

    case NLMRSC_INVALID_BLK_WIDTH:
		NlmCm__printf("NLMRSC_INVALID_AB_NUM");
		break;

	case NLMRSC_INVALID_AB_INDEX:
		NlmCm__printf("NLMRSC_INVALID_AB_INDEX");
		break;

	case NLMRSC_DUPLICATE_DEVICE:
		NlmCm__printf("NLMRSC_DUPLICATE_DEVICE");
		break;

	case NLMRSC_INVALID_PARAM:
		NlmCm__printf("NLMRSC_INVALID_PARAM");
		break;

	case NLMRSC_OPR_FAILURE:
		NlmCm__printf("NLMRSC_OPR_FAILURE");
		break;

	case NLMRSC_NOFREE_RQST:
		NlmCm__printf("NLMRSC_NOFREE_RQST");
		break;

	case NLMRSC_NORQST_AVBL:
		NlmCm__printf("NLMRSC_NORQST_AVBL");
		break;

	case NLMRSC_NORSLT_AVBL:
		NlmCm__printf("NLMRSC_NORSLT_AVBL");
		break;

	case NLMRSC_DEV_MGR_CONFIG_LOCKED:
		NlmCm__printf("NLMRSC_DEV_MGR_CONFIG_LOCKED");
		break;

	case NLMRSC_CASCADE_NOT_EXIST:
		NlmCm__printf("NLMRSC_CASCADE_NOT_EXIST");
		break;

	case NLMRSC_INVALID_PARENT:
		NlmCm__printf("NLMRSC_INVALID_PARENT");
		break;

    case NLMRSC_INVALID_DEVICE_TYPE:
		NlmCm__printf("NLMRSC_INVALID_DEVICE_TYPE");
		break;

    case NLMRSC_INVALID_NUM_OF_DEVICES:
		NlmCm__printf("NLMRSC_INVALID_NUM_OF_DEVICES");
		break;

    case NLMRSC_INVALID_DATA_LENGTH:
		NlmCm__printf("NLMRSC_INVALID_DATA_LENGTH");
		break;

    case NLMRSC_DATA_LENGTH_ADDRESS_MISMATCH:
		NlmCm__printf("NLMRSC_DATA_LENGTH_ADDRESS_MISMATCH");
		break;

    case NLMRSC_READONLY_REGISTER:
		NlmCm__printf("NLMRSC_READONLY_REGISTER");
		break;

    case NLMRSC_INVALID_DEVICE_ID:
		NlmCm__printf("NLMRSC_INVALID_DEVICE_ID");
		break;

     case NLMRSC_INVALID_GENERIC_TM:
		NlmCm__printf("NLMRSC_INVALID_GENERIC_TM");
		break;

	case NLMRSC_INVALID_GENERIC_TABLE:
		NlmCm__printf("NLMRSC_INVALID_GENERIC_TABLE");
		break;

    case NLMRSC_INVALID_GTM_BLKS_RANGE:
		NlmCm__printf("NLMRSC_INVALID_GTM_BLKS_RANGE");
		break;

	case NLMRSC_INVALID_APP_CALLBACK:
		NlmCm__printf("NLMRSC_INVALID_APP_CALLBACK");
		break;

	case NLMRSC_INVALID_TABLEID:
		NlmCm__printf("NLMRSC_INVALID_TABLEID");
		break;

	case NLMRSC_INVALID_SEARCH_ATTRIBUTES:
		NlmCm__printf("NLMRSC_INVALID_SEARCH_ATTRIBUTES");
		break;

	case NLMRSC_INVALID_PS_DEPEND_ATTRIBUTES:
		NlmCm__printf("NLMRSC_INVALID_PS_DEPEND_ATTRIBUTES");
		break;

	case NLMRSC_CONFIGURATION_LOCKED:
		NlmCm__printf("NLMRSC_CONFIGURATION_LOCKED");
		break;

	case NLMRSC_EXCESSIVE_NUM_TABLE_PAR_LTR:
		NlmCm__printf("NLMRSC_EXCESSIVE_NUM_TABLE_PAR_LTR");
		break;

	case NLMRSC_INVALID_BMR:
		NlmCm__printf("NLMRSC_INVALID_BMR");
		break;

	case NLMRSC_INVALID_RECORD:
		NlmCm__printf("NLMRSC_INVALID_RECORD");
		break;

	case NLMRSC_DEVICE_DATABASE_FULL:
		NlmCm__printf("NLMRSC_DEVICE_DATABASE_FULL");
		break;

	case NLMRSC_TABLE_FULL:
		NlmCm__printf("NLMRSC_TABLE_FULL");
		break;

	case NLMRSC_TABLE_LIMIT_EXCEEDED:
		NlmCm__printf("NLMRSC_TABLE_LIMIT_EXCEEDED");
		break;

	case NLMRSC_RECORD_NOTFOUND:
		NlmCm__printf("NLMRSC_RECORD_NOTFOUND");
		break;

	case NLMRSC_INVALID_RANGE_ATTRIBUTES:
		NlmCm__printf("NLMRSC_INVALID_RANGE_ATTRIBUTES");
		break;

    case NLMRSC_INVALID_KEY_NUM:
		NlmCm__printf("NLMRSC_INVALID_KEY_NUM");
		break;

    case NLMRSC_INVALID_RESULT_SEQ_NUM:
		NlmCm__printf("NLMRSC_INVALID_RESULT_SEQ_NUM");
		break;

    case NLMRSC_INVALID_TABLE_WIDTH:
		NlmCm__printf("NLMRSC_INVALID_TABLE_WIDTH");
		break;

    case NLMRSC_SEARCHATTRIB_NOT_DELETED:
		NlmCm__printf("NLMRSC_SEARCHATTRIB_NOT_DELETED");
		break;

    case NLMRSC_INVALID_RANGE_MGR_ATTR:
		NlmCm__printf("NLMRSC_INVALID_RANGE_MGR_ATTR");
		break;

	case NLMRSC_INVALID_RANGE_MGR:
		NlmCm__printf("NLMRSC_INVALID_RANGE_MGR");
		break;

	case NLMRSC_INVALID_DATABASE:
		NlmCm__printf("NLMRSC_INVALID_DATABASE");
		break;

	case NLMRSC_DUPLICATE_DATABASE_ID:
		NlmCm__printf("NLMRSC_DUPLICATE_DATABASE_ID");
		break;

	case NLMRSC_INVALID_RANGE_DB_ATTR:
		NlmCm__printf("NLMRSC_INVALID_RANGE_DB_ATTR");
		break;

	case NLMRSC_INVALID_RANGE:
		NlmCm__printf("NLMRSC_INVALID_RANGE");
		break;

    case NLMRSC_INVALID_OUTPUT_RNG_PTR:
		NlmCm__printf("NLMRSC_INVALID_OUTPUT_RNG_PTR");
		break;

    case NLMRSC_INVALID_OUTPUT_NUM_PTR:
		NlmCm__printf("NLMRSC_INVALID_OUTPUT_NUM_PTR");
		break;

	case NLMRSC_DUPLICATE_RANGE_ID:
		NlmCm__printf("NLMRSC_DUPLICATE_RANGE_ID");
		break;

    case NLMRSC_INVALID_RANGE_SELECTION:
		NlmCm__printf("NLMRSC_INVALID_RANGE_SELECTION");
		break;

	case NLMRSC_NO_SUPPORT_FOR_THIS_COMB:
		NlmCm__printf("NLMRSC_NO_SUPPORT_FOR_THIS_COMB");
		break;

    case NLMRSC_NO_SUPPORT_FOR_SEARCHDB:
		NlmCm__printf("NLMRSC_NO_SUPPORT_FOR_SEARCHDB");
		break;

    case NLMRSC_INVALID_TBLID_LEN:
		NlmCm__printf("NLMRSC_INVALID_TBLID_LEN");
		break;

    case NLMRSC_INVALID_FIB_MGR:
		NlmCm__printf("NLMRSC_INVALID_FIB_MGR");
		break;

    case NLMRSC_INVALID_FIB_TBL:
		NlmCm__printf("NLMRSC_INVALID_FIB_TBL");
		break;

    case NLMRSC_DUPLICATE_FIB_TBL_ID:
		NlmCm__printf("NLMRSC_DUPLICATE_FIB_TBL_ID");
		break;

    case NLMRSC_INVALID_FIB_BLKS_RANGE:
		NlmCm__printf("NLMRSC_INVALID_FIB_BLKS_RANGE");
		break;

    case NLMRSC_INVALID_FIB_TBL_INDEX_RANGE:
		NlmCm__printf("NLMRSC_INVALID_FIB_TBL_INDEX_RANGE");
		break;

    case NLMRSC_INVALID_FIB_MAX_PREFIX_LENGTH:
		NlmCm__printf("NLMRSC_INVALID_FIB_MAX_PREFIX_LENGTH");
		break;

    case NLMRSC_INVALID_FIB_START_BIT_IN_KEY:
		NlmCm__printf("NLMRSC_INVALID_FIB_START_BIT_IN_KEY");
		break;

    case NLMRSC_INVALID_PREFIX:
		NlmCm__printf("NLMRSC_INVALID_PREFIX");
		break;

    case NLMRSC_INVALID_IDX_SPACE_MGR:
		NlmCm__printf("NLMRSC_INVALID_IDX_SPACE_MGR");
		break;

    case NLMRSC_INVALID_IDX_RANGE_PTR:
		NlmCm__printf("NLMRSC_INVALID_IDX_RANGE_PTR");
		break;

    case NLMRSC_IDX_RANGE_FULL:
		NlmCm__printf("NLMRSC_IDX_RANGE_FULL");
		break;

    case NLMRSC_INVALID_IDX_RANGE_GROW_FAILED:
		NlmCm__printf("NLMRSC_INVALID_IDX_RANGE_GROW_FAILED");
		break;

    case NLMRSC_LOCATE_NODE_FAILED:
		NlmCm__printf("NLMRSC_LOCATE_NODE_FAILED");
		break;

    case NLMRSC_DUPLICATE_PREFIX:
		NlmCm__printf("NLMRSC_DUPLICATE_PREFIX");
		break;

     case NLMRSC_PREFIX_NOT_FOUND:
		NlmCm__printf("NLMRSC_PREFIX_NOT_FOUND");
		break;

     case NLMRSC_INVALID_PREFIX_LEN:
		NlmCm__printf("NLMRSC_INVALID_PREFIX_LEN");
		break;

     case NLMRSC_RESOURCE_ALLOC_FAILED:
		NlmCm__printf("NLMRSC_RESOURCE_ALLOC_FAILED");
		break;

     case NLMRSC_TMAX_EXCEEDED:
		NlmCm__printf("NLMRSC_TMAX_EXCEEDED");
		break;

     case NLMRSC_REASON_UNKNOWN:
		NlmCm__printf("NLMRSC_REASON_UNKNOWN");
		break;

	default:
		NlmCm__printf("Unknown ReasonCode");
		break;

	}

	NlmCm__printf("\n");
}

/* This function is used by the GTM Module to inform about the record's index
change that occur due to shuffling of records in order to maintain the priorities
of the record; This function should be passed as arg to NlmGenericTblMgr__Init API */
static void NlmGtmFtmRefApp_GtmRecordIndexChangeCallBack( void* client_p,
								NlmGenericTbl* genericTbl_p,
								NlmRecordIndex oldIndex,
								NlmRecordIndex newIndex )
{
    /* This function is dependent on the application code */
	(void)client_p;
	(void)genericTbl_p;
	(void)oldIndex;
	(void)newIndex;
	return;
}

/* This function is used by the FTM Module to inform about the prefix index
change that occur when prefixes are added; This function should be passed as
arg to NlmFibTblMgr__Init API */
static void NlmGtmFtmRefApp_FtmPrefixIndexChangeCallBack( void* client_p,
                                                         void* fibTbl_p,
                                                         NlmFibPrefixIndex oldIndex,
                                                         NlmFibPrefixIndex newIndex)
{
    /* This function is dependent on the application code */
	(void)client_p;
	(void)fibTbl_p;
	(void)oldIndex;
	(void)newIndex;
    return;
}

/* Initializes the Transport Interface */
static NlmErrNum_t NlmGtmFtmRefApp_InitEnvironment( NlmGtmFtmRefAppData *refAppData_p )
{
	refAppData_p->m_request_queue_len = 1; /* No queuing of xpt requests */
	refAppData_p->m_result_queue_len  = 1;

	/* Search system has only one channel (cascade of devices) */
	refAppData_p->m_channel_id = 0;

	/* Device is operating in  Sahasra Operation Mode */
	refAppData_p->m_opr_mode    = NLMDEV_OPR_SAHASRA;

#ifndef NLMPLATFORM_BCM
	refAppData_p->m_xptType     = IFTYPE_CMODEL;
#ifdef NLM_XLP
	refAppData_p->m_xptType     = IFTYPE_XLPXPT;
#endif
#else /* BCMPLATFORM */
	refAppData_p->m_xptType     = IFTYPE_BCM_CALADAN3;
#endif /* NLMPLATFORM_BCM */

	switch(refAppData_p->m_xptType)
	{
#ifndef NLMPLATFORM_BCM

		case IFTYPE_CMODEL:
			/* Create simulation transport interface. If any other transport interface is used
			then this portion of code should be replaced with the corresponding Init function */
			refAppData_p->m_xpt_p = NlmSimXpt__Create(refAppData_p->m_alloc_p,
													NLM_DEVTYPE_2_S,
													0,  /* this argument is ignored for this Processor */
													refAppData_p->m_request_queue_len,
													refAppData_p->m_result_queue_len,
													refAppData_p->m_opr_mode,
													0,  /* this argument is ignored for this Processor */
													refAppData_p->m_channel_id
													);
			if(refAppData_p->m_xpt_p == NULL)
			{
				NlmCm__printf("\n\tSimulation Transport Inteface Creation Failed.\n");
				return NLM_RETURN_STATUS_FAIL;
			}
			NlmCm__printf("\n\tSimulation Transport Interface Created Successfully\n");
			break;

#ifdef NLM_XLP
		case IFTYPE_XLPXPT:
	/* Create XLP  transport interface. If any other transport interface is used
    then this portion of code should be replaced with the corresponding Init function */
    refAppData_p->m_xpt_p = NlmXlpXpt__Create(refAppData_p->m_alloc_p,
                                              NLM_DEVTYPE_2_S,
                                              refAppData_p->m_request_queue_len,
                                              refAppData_p->m_opr_mode,
                                               refAppData_p->m_channel_id, 
                                               0, 0, 1,1);
    if(refAppData_p->m_xpt_p == NULL)
    {
        NlmCm__printf("\n\tXLP Transport Inteface Creation Failed.\n");
        return NLM_RETURN_STATUS_FAIL;
    }
    NlmCm__printf("\n\tXLP  Transport Interface Created Successfully\n");
			break;
#endif
    
#endif /* NLMPLATFORM_BCM */
                case IFTYPE_BCM_CALADAN3:
		case IFTYPE_END:
		default:
			NlmCm__printf("\n\tInvalid Transport layer request\n");
		    return NLM_RETURN_STATUS_FAIL; 

	} /* switch */
	return NLM_RETURN_STATUS_OK;
}

/* Destroy SimXpt transport interface */
static NlmErrNum_t NlmGtmFtmRefApp_DestroyEnvironment( NlmGtmFtmRefAppData *refAppData_p )
{
	switch(refAppData_p->m_xptType)
	{
#ifndef NLMPLATFORM_BCM
		case IFTYPE_CMODEL:
			/* call appropriate functions which destroys simulation transport interface */
			NlmCm__printf("\n\tDestroying Simulation Transport Interface\n");
			NlmSimXpt__destroy( refAppData_p->m_xpt_p );
			break;

#ifdef NLM_XLP
		case IFTYPE_XLPXPT:
	/* call appropriate functions which destroys XLP  transport interface */
    NlmCm__printf("\n\tDestroying XLP  Transport Interface\n");
    NlmXlpXpt__Destroy( refAppData_p->m_xpt_p );
			break;
#endif

#endif /* NLMPLATFORM_BCM */
        case IFTYPE_BCM_CALADAN3:
		case IFTYPE_END:
        default:
            NlmCm__printf("\n\tInvalid Transport layer request\n");
            return NLM_RETURN_STATUS_FAIL;

	} /* Switch */
    return NLM_RETURN_STATUS_OK;
}

/* This function performs Device Manager related Inits */
static NlmErrNum_t NlmGtmFtmRefApp_InitDeviceManager( NlmGtmFtmRefAppData *refAppData_p )
{
	NlmDevId	dev_id;
	NlmReasonCode	reason = NLMRSC_REASON_OK;

	if( NULL == ( refAppData_p->m_devMgr_p = NlmDevMgr__create(refAppData_p->m_alloc_p,
                                                                  refAppData_p->m_xpt_p,
                                                                  refAppData_p->m_opr_mode,
                                                                  &reason )))
    {
		NlmCm__printf("\tCould not initialize Device Manager...\n" );
		NlmGtmFtmRefApp_PrintReasonCode( reason );
		return NLM_RETURN_STATUS_ABORT;
	}

	NlmCm__printf("\n\tDevice Manager Initialized Successfully\n");

	/* Now add a device to the cascade */
	if( NULL == (refAppData_p->m_dev_p = NlmDevMgr__AddDevice(refAppData_p->m_devMgr_p,
                                                                 &dev_id,
                                                                 &reason )))
    {
		NlmCm__printf("Could not add Device with Dev Id [%d]...\n", dev_id);
		NlmGtmFtmRefApp_PrintReasonCode( reason );
		return NLM_RETURN_STATUS_ABORT;
	}

	NlmCm__printf("\tDevice with Dev Id [%d] Added to Cascade\n", dev_id);

	/* We are done with configurations. Now Lock Device Manager */
	if( NLMERR_OK != NlmDevMgr__LockConfig( refAppData_p->m_devMgr_p, &reason ) )
    {
		NlmCm__printf("Could not lock Device Manager...\n");
		NlmGtmFtmRefApp_PrintReasonCode( reason );
		return NLM_RETURN_STATUS_ABORT;
	}

	NlmCm__printf("\tDevice Manager Configuration Locked\n");

	return NLM_RETURN_STATUS_OK;
}

/* Destroys Device Manager instance */
static NlmErrNum_t NlmGtmFtmRefApp_DestroyDeviceManager( NlmGtmFtmRefAppData *refAppData_p )
{
	NlmCm__printf("\n\tDestroying Device Manager\n");
	NlmDevMgr__destroy( refAppData_p->m_devMgr_p );

	return NLM_RETURN_STATUS_OK;
}

/* This function performs Range Manager related Inits */
static NlmErrNum_t NlmGtmFtmRefApp_InitRangeManager( NlmGtmFtmRefAppData *refAppData_p )
{
	NlmReasonCode	reason;
    NlmRangeSrchAttrs rangeSrchAttrs = {0};
    NlmDevConfigReg devConfigRegData = {0};

	/* {Range ID, Start Range, End Range, Pointer to encoded bitmaps} */
	NlmRange              ranges[ NLM_GTM_FTM_REFAPP_GTM_NUM_OF_RANGES ] =
											   {
												 { 0, 1, 5, NULL        },
												 { 1, 4, 14, NULL         },
												 { 2, 10, 25, NULL        },
												 { 3, 1, 5, NULL          },
												 { 4, 500, 510, NULL      },
												 { 5, 200, 210, NULL      },
												 { 6, 150, 250, NULL      },
												 { 7, 1024, 1030, NULL    },
												 { 8, 505, 510, NULL      },
												 { 9, 1000, 1050, NULL    },
												 { 10, 10000, 10005, NULL },
												 { 11, 5000, 6025, NULL   }
											   };
	NlmRangeEncodingType encodingType[4] = { NLM_RANGE_3B_ENCODING,
											 NLM_RANGE_3B_ENCODING,
											 NLM_RANGE_3B_ENCODING,
											 NLM_RANGE_3B_ENCODING
											};
	nlm_u32  iter;
	nlm_u8  db_id;
    nlm_u8 keyNum;

    /* Enable the Range Engine by Writing to Device Config with approriate value */
    devConfigRegData.m_dbParityErrEntryInvalidate = NLMDEV_ENABLE;
    devConfigRegData.m_rangeEngineEnable = NLMDEV_ENABLE;
    NlmDevMgr__GlobalRegisterWrite(refAppData_p->m_dev_p,
                                      NLMDEV_DEVICE_CONFIG_REG,
                                      &devConfigRegData,
                                      NULL);

	/* Initialize Range Manager */

	if( NULL == ( refAppData_p->m_rangeMgr_p = NlmRangeMgr__Init(refAppData_p->m_alloc_p,
																 refAppData_p->m_devMgr_p,
																 NLM_DEVTYPE_2_S,
											   				     encodingType,
																 &reason
															    )))
	{
		NlmCm__printf("\tCould not initialize Range Manager...\n" );
		NlmGtmFtmRefApp_PrintReasonCode( reason );

		return NLM_RETURN_STATUS_ABORT;
	}

	NlmCm__printf("\n\n\tRange Manager Initialized Successfully\n");

	/* Now create Range database with appropriate database attributes.
        There are two attributes per database,
	 (1) number of spare bits available
	 (2) Number of valid bits of range
	 Number of spare bits available for Range Manager  for encoding is given by spare bits.
     Number of valid range bits specifies how many (LSB) bits of the 16b range field are valid;
     reference application uses ranges where entire 16 bits
	 * are valid.
	 */
	db_id = 0; /* used for database id */
	refAppData_p->m_rangeDbAttrs.m_num_bits  = 32;
    refAppData_p->m_rangeDbAttrs.m_valid_bits = 16;
	refAppData_p->m_rangeDbAttrs.m_encodingType = NLM_RANGE_3B_ENCODING;
    if( NULL == ( refAppData_p->m_rangeDb_p = NlmRangeMgr__CreateDb(refAppData_p->m_rangeMgr_p,
																    db_id,
                                                                    &refAppData_p->m_rangeDbAttrs,
																    &reason
															        )))
	{
		NlmCm__printf("\tCould not create dabase-%d. Exiting...\n", db_id);
		NlmGtmFtmRefApp_PrintReasonCode( reason );

		return NLM_RETURN_STATUS_ABORT;
	}

	NlmCm__printf("\tRange Database [Database ID = %d] Created\n", db_id);

	/* Add ranges to range database*/
	for( iter = 0; iter < NLM_GTM_FTM_REFAPP_GTM_NUM_OF_RANGES; iter++ )
	{
        refAppData_p->m_ranges[ iter ] = ranges[ iter ];
		if( NLMERR_OK != NlmRangeMgr__AddRange( refAppData_p->m_rangeMgr_p,
												refAppData_p->m_rangeDb_p,
												&refAppData_p->m_ranges[ iter ],
												&reason ) )
		{
			NlmCm__printf("\tCould not add range-%u to range database...\n", iter);
			NlmGtmFtmRefApp_PrintReasonCode( reason );

			return NLM_RETURN_STATUS_FAIL;
		}
	}
	NlmCm__printf("\tRanges added to Range Database\n");

    /* Associate range database with one of the range blocks so that hardware does MCOR encoding for the database.
     Range database is mapped to Range A for this application
	 */
	if( NLMERR_OK != NlmRangeMgr__AssignRange( refAppData_p->m_rangeMgr_p,
                                               refAppData_p->m_rangeDb_p,
											   NLM_RANGE_TYPE_A,
											   &reason ) )
	{
		NlmCm__printf("Range database Hardware Block Assignment failed...\n");
		NlmGtmFtmRefApp_PrintReasonCode( reason );

		return NLM_RETURN_STATUS_FAIL;
	}

	NlmCm__printf("\tRange database associated with Range-A H/W Block\n");


	/* Let Range Manager do range compression now */
	if( NLMERR_OK != NlmRangeMgr__CreateEncodings( refAppData_p->m_rangeMgr_p,
												   refAppData_p->m_rangeDb_p,
												   &reason ) )
	{
		NlmCm__printf("\tCreateEncodings failed on range database...\n");
		NlmGtmFtmRefApp_PrintReasonCode( reason );

		return NLM_RETURN_STATUS_FAIL;
	}

	NlmCm__printf("\tCreateEncodings on Range Database Successful\n");


    /* Configure range params for the GTM search.
    * Master key used for the search will have range field at Bits[191:176],
    * After KCM shifts Key#3 which will searched in the GTM table should be configured to have
    * encoded Range value inserted at [31:0] */

    /* Initialize all Range Insert values for all keys as NLMDEV_RANGE_DO_NOT_INSERT first */
    for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
    {
        rangeSrchAttrs.m_keyInsert_startByte_rangeA[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
        rangeSrchAttrs.m_keyInsert_startByte_rangeB[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
        rangeSrchAttrs.m_keyInsert_startByte_rangeC[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
        rangeSrchAttrs.m_keyInsert_startByte_rangeD[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
    }

    /* Now initialize extraction start value for Range A to be at Bit 176*/
    rangeSrchAttrs.m_extraction_startByte_rangeA = (176 >> 3);

    /* Initialize insertion start value for Range A to be at Bit 0 */
    rangeSrchAttrs.m_keyInsert_startByte_rangeA[refAppData_p->m_gtmSrchKeyNum] = 0;

    /* Initialize Range Database to be used for Range A */
    rangeSrchAttrs.m_rangeA_db = refAppData_p->m_rangeDb_p;

    if( NLMERR_OK != NlmRangeMgr__ConfigRangeMatching(
                                refAppData_p->m_rangeMgr_p,
                                refAppData_p->m_srchLtrNum,
                                &rangeSrchAttrs,
                                &reason ) )
    {
        NlmCm__printf("\tConfigRangeMatching for ltr[%d] failed...\n", refAppData_p->m_srchLtrNum);
        NlmGtmFtmRefApp_PrintReasonCode( reason );
        return NLM_RETURN_STATUS_FAIL;
    }
    NlmCm__printf("\tConfigRangeMatching for ltr[%d] done successfully...\n",
        refAppData_p->m_srchLtrNum);


	return NLM_RETURN_STATUS_OK;
}

/* Destroys Range Manager instance and range databases */
static NlmErrNum_t NlmGtmFtmRefApp_DestroyRangeManager( NlmGtmFtmRefAppData *refAppData_p )
{
	NlmReasonCode   reason;

	/* Destroy range databases first and then the manager */
	NlmCm__printf("\n\tDestroying Range Databases\n");
    NlmRangeMgr__DestroyDb( refAppData_p->m_rangeMgr_p, refAppData_p->m_rangeDb_p, &reason );

	NlmCm__printf("\tDestroying Range Manager\n");
	NlmRangeMgr__Destroy( refAppData_p->m_rangeMgr_p, &reason );

	return NLM_RETURN_STATUS_OK;
}

/* This function performs Generic Table Manager related Inits */
static NlmErrNum_t NlmGtmFtmRefApp_InitGenericTableManager( NlmGtmFtmRefAppData *refAppData_p )
{
	NlmReasonCode	reason;
    NlmGenericTblSearchAttributes psAttrs;

    /* Set the blocks range used by the Generic Table Manager */
    refAppData_p->m_gtmBlkRange.m_startBlkNum = NLM_GTM_FTM_REFAPP_GTM_START_BLK;
    refAppData_p->m_gtmBlkRange.m_endBlkNum = NLM_GTM_FTM_REFAPP_GTM_END_BLK;

	/* Initialize the Call back function called by Cynapse when index for a
    record is changed due to shuffles that occur during addition of records
    of different priorities */
	refAppData_p->m_gtmRecordIndexChangeCB = NlmGtmFtmRefApp_GtmRecordIndexChangeCallBack;

	if( NULL == ( refAppData_p->m_genericTblMgr_p = NlmGenericTblMgr__Init(
														refAppData_p->m_alloc_p,
														refAppData_p->m_devMgr_p,
                                                        NLM_DEVTYPE_2_S,
														NLM_REFAPP_NUM_OF_DEVICES,
                                                        &refAppData_p->m_gtmBlkRange,
                                                        NLM_GTM_FTM_REFAPP_GTM_TBL_ID_LEN,
                                                        refAppData_p->m_gtmRecordIndexChangeCB,
														NULL,
														&reason )))
	{
		NlmCm__printf("\n\tGTM Init failed...\n");
		NlmGtmFtmRefApp_PrintReasonCode( reason );
		return NLM_RETURN_STATUS_ABORT;
	}
    NlmCm__printf("\n\tGeneric Table Manager Init Successful...\n");

    /* Create a GTM Table with tblId = (0) NLM_GTM_FTM_REFAPP_GTM_TBL_ID,
    size = (10000) NLM_GTM_FTM_REFAPP_GTM_TBL_SIZE,
    and tblWidth = (160) NLM_GTM_FTM_REFAPP_GTM_TBL_WIDTH */

	NlmCm__strcpy( refAppData_p->m_gtmTblId, NLM_GTM_FTM_REFAPP_GTM_TBL_ID );
	refAppData_p->m_gtmTblWidth = NLM_GTM_FTM_REFAPP_GTM_TBL_WIDTH;
	refAppData_p->m_gtmTblSize = NLM_GTM_FTM_REFAPP_GTM_TBL_SIZE;

    if( NULL == (refAppData_p->m_gtmTbl_p = NlmGenericTblMgr__CreateTable(
                                                    refAppData_p->m_genericTblMgr_p,
                                                	refAppData_p->m_gtmTblId,
                                                	refAppData_p->m_gtmTblWidth,
                                                	refAppData_p->m_gtmTblSize,
                                                	&reason ) ) )
    {
        NlmCm__printf("\tGeneric Table with tbl id [%s] creation failed...\n",
                            refAppData_p->m_gtmTblId);
        NlmGtmFtmRefApp_PrintReasonCode( reason );
        return NLM_RETURN_STATUS_FAIL;
    }
    NlmCm__printf("\tGeneric Table with tbl id [%s] created...\n",
        refAppData_p->m_gtmTblId);

    /* This application is designed to perform 3 searches, 1 GTM and 2 FTM using LTR#0;
        Rslt Port Num 0 and 1 are used for FTM searches, while Rslt Port Num 3 is used for GTM search
        Similarly Key 1 and Key 2 are used for FTM searches and Key 3 is used for GTM search
        The Master Key for the Searches will be as descibed below
        Bits[319: 160] -- GTM Search Key
        Bits[159: 80] -- FTM Search Key 1
        Bits[79: 0] -- FTM Search Key 2
        Also the GTM search will have range field at its LSB bits
    */

    /* Configure GTM searches now */
    refAppData_p->m_srchLtrNum = NLM_GTM_FTM_REFAPP_SRCH_LTR_NUM;
    refAppData_p->m_gtmSrchRsltPortNum = NLM_GTM_FTM_REFAPP_GTM_SRCH_RSLT_PORT_NUM;
    refAppData_p->m_gtmSrchKeyNum = NLM_GTM_FTM_REFAPP_GTM_SRCH_KEY_NUM;

    /* GTM search key will be at bits[319:160] of the master key
    Initialize the KCM value accordingly */
    refAppData_p->m_gtmSrchKCM.m_segmentStartByte[0] = 20;
    refAppData_p->m_gtmSrchKCM.m_segmentNumOfBytes[0] = 16;
    refAppData_p->m_gtmSrchKCM.m_segmentStartByte[1] = 36;
    refAppData_p->m_gtmSrchKCM.m_segmentNumOfBytes[1] = 4;
    refAppData_p->m_gtmSrchKCM.m_segmentStartByte[2] = 0;
    refAppData_p->m_gtmSrchKCM.m_segmentNumOfBytes[2] = 0;

    /* Initialize search attributes */
    psAttrs.m_numOfParallelSrches = 1;
    psAttrs.m_psInfo[0].m_tblId_p = refAppData_p->m_gtmTblId;
    psAttrs.m_psInfo[0].m_rsltPortNum = refAppData_p->m_gtmSrchRsltPortNum;
    psAttrs.m_psInfo[0].m_keyNum = refAppData_p->m_gtmSrchKeyNum;
    NlmCm__memcpy(&psAttrs.m_psInfo[0].m_kcm,
                  &refAppData_p->m_gtmSrchKCM,
                  sizeof(NlmGenericTblKeyConstructionMap));

    if( NLMERR_OK != NlmGenericTblMgr__ConfigSearch(
                                            refAppData_p->m_genericTblMgr_p,
											refAppData_p->m_srchLtrNum,
											&psAttrs,
											&reason ))
    {
        NlmCm__printf("\tGTM Search Configuration for LTR#%d failed...\n",
                            refAppData_p->m_srchLtrNum);
        NlmGtmFtmRefApp_PrintReasonCode( reason );
        return NLM_RETURN_STATUS_FAIL;
    }
    NlmCm__printf("\tGTM Search Configuration for LTR#%d done successfully...\n",
        refAppData_p->m_srchLtrNum);

	/* GTM Configuration is finished, Lock it now */
	if( NLMERR_OK != NlmGenericTblMgr__LockConfiguration(
									refAppData_p->m_genericTblMgr_p,
									&reason ) )
	{
		NlmCm__printf("\tGTM LockConfig failed...\n");
		NlmGtmFtmRefApp_PrintReasonCode( reason );

		return NLM_RETURN_STATUS_FAIL;
	}
	NlmCm__printf("\tGTM Configuration Locked");

	return NLM_RETURN_STATUS_OK;
}

/* Destroy GTM tables and GTM table manager instance */
static NlmErrNum_t NlmGtmFtmRefApp_DestroyGenericTableManager( NlmGtmFtmRefAppData *refAppData_p )
{
	NlmReasonCode   reason = NLMRSC_REASON_OK;

    /* Destroy the instance of GTM table */
    NlmCm__printf("\n\tDestroying GTM table with tbl id [%s]\n", refAppData_p->m_gtmTblId);
    if( NLMERR_OK != NlmGenericTblMgr__DestroyTable(refAppData_p->m_genericTblMgr_p,
                                                    refAppData_p->m_gtmTbl_p,
                                                    &reason ) )
    {
        NlmCm__printf("\tDestroying GTM table with tbl id [%s] failed\n ",  refAppData_p->m_gtmTblId);
        NlmGtmFtmRefApp_PrintReasonCode( reason );
        return NLM_RETURN_STATUS_FAIL;
    }

	/* Time to destroy the manager now. */
	NlmCm__printf("\tDestroying Generic Table Manager\n");
	if( NLMERR_OK != NlmGenericTblMgr__Destroy( refAppData_p->m_genericTblMgr_p, &reason ) )
	{
		NlmCm__printf("\tGeneric Table Manager destroy failed...\n");
		NlmGtmFtmRefApp_PrintReasonCode( reason );
		return NLM_RETURN_STATUS_FAIL;
	}

	return NLM_RETURN_STATUS_OK;
}

/* This function performs Fib Table Manager related Inits */
static NlmErrNum_t NlmGtmFtmRefApp_InitFibTableManager( NlmGtmFtmRefAppData *refAppData_p )
{
	NlmReasonCode	reason;
    NlmFibParallelSrchAttributes psAttrs;

    /* Set the FIB Table Manager block range */
    refAppData_p->m_fibBlkRange.m_startBlkNum = NLM_GTM_FTM_REFAPP_FTM_START_BLK;
    refAppData_p->m_fibBlkRange.m_endBlkNum = NLM_GTM_FTM_REFAPP_FTM_END_BLK;

	/* Initialize the Call back function called by Cynapse when index for a record is changed */
	refAppData_p->m_ftmPrefixIndexChangeCB = NlmGtmFtmRefApp_FtmPrefixIndexChangeCallBack;

    refAppData_p->m_fibTblIdLen = NLM_GTM_FTM_REFAPP_FIB_TBL_ID_LEN;

    /* Initialize the Fib Tbl Manager */
	if( NULL == ( refAppData_p->m_fibTblMgr_p = NlmFibTblMgr__Init(
                                                refAppData_p->m_alloc_p,
                                                refAppData_p->m_devMgr_p,
                                                NLM_DEVTYPE_2_S,
                                                NLM_REFAPP_NUM_OF_DEVICES,
                                                &refAppData_p->m_fibBlkRange,
                                                refAppData_p->m_fibTblIdLen,
                                                refAppData_p->m_ftmPrefixIndexChangeCB,
                                                refAppData_p,
                                                &reason ) ) )
	{
		NlmCm__printf("\n\tFTM Init failed...\n");
		NlmGtmFtmRefApp_PrintReasonCode( reason );
		return NLM_RETURN_STATUS_ABORT;
	}
    NlmCm__printf("\n\tFib Table Manager Init successful...\n");

    /* Create a Fib Table with tblId = (0) NLM_GTM_FTM_REFAPP_FIB_TBL_ONE_ID,
    Index Range = 0 - 20000, tblWidth = (36) NLM_GTM_FTM_REFAPP_FIB_TBL_ONE_WIDTH
    */
	NlmCm__strcpy( refAppData_p->m_fibTblOneId, NLM_GTM_FTM_REFAPP_FIB_TBL_ONE_ID);
    refAppData_p->m_fibTblOneIndexRange.m_indexLowValue = NLM_GTM_FTM_REFAPP_FIB_TBL_ONE_INDEX_LOW;
    refAppData_p->m_fibTblOneIndexRange.m_indexHighValue = NLM_GTM_FTM_REFAPP_FIB_TBL_ONE_INDEX_HIGH;
	refAppData_p->m_fibTblOneWidth = NLM_GTM_FTM_REFAPP_FIB_TBL_ONE_WIDTH;

    if( NULL == (refAppData_p->m_fibTblOne_p = NlmFibTblMgr__CreateTable(
                                                    refAppData_p->m_fibTblMgr_p,
                                                	refAppData_p->m_fibTblOneId,
                                                	&refAppData_p->m_fibTblOneIndexRange,
                                                	refAppData_p->m_fibTblOneWidth,
                                                	&reason ) ) )
    {
        NlmCm__printf("\tFib Table with tbl id [%s] creation failed...\n",
                            refAppData_p->m_fibTblOneId);
        NlmGtmFtmRefApp_PrintReasonCode( reason );
        return NLM_RETURN_STATUS_FAIL;
	}
    NlmCm__printf("\tFib Table with tbl id [%s] created successfully\n",
                      refAppData_p->m_fibTblOneId);

    /* Create another Fib Table with tblId = (1) NLM_GTM_FTM_REFAPP_FIB_TBL_TWO_ID,
    Index Range = 0 - 20000, tblWidth = (48) NLM_GTM_FTM_REFAPP_FIB_TBL_TWO_WIDTH
    */
	NlmCm__strcpy( refAppData_p->m_fibTblTwoId, NLM_GTM_FTM_REFAPP_FIB_TBL_TWO_ID);
    refAppData_p->m_fibTblTwoIndexRange.m_indexLowValue =  NLM_GTM_FTM_REFAPP_FIB_TBL_TWO_INDEX_LOW;
    refAppData_p->m_fibTblTwoIndexRange.m_indexHighValue = NLM_GTM_FTM_REFAPP_FIB_TBL_TWO_INDEX_HIGH;
	refAppData_p->m_fibTblTwoWidth = NLM_GTM_FTM_REFAPP_FIB_TBL_TWO_WIDTH;
    if( NULL == (refAppData_p->m_fibTblTwo_p = NlmFibTblMgr__CreateTable(
                                                    refAppData_p->m_fibTblMgr_p,
                                                	refAppData_p->m_fibTblTwoId,
                                                	&refAppData_p->m_fibTblTwoIndexRange,
                                                	refAppData_p->m_fibTblTwoWidth,
                                                	&reason ) ) )
    {
        NlmCm__printf("\tFib Table with tbl id [%s] creation failed...\n",
            refAppData_p->m_fibTblTwoId);
        NlmGtmFtmRefApp_PrintReasonCode( reason );
        return NLM_RETURN_STATUS_FAIL;
	}
    NlmCm__printf("\tFib Table with tbl id [%s] created successfully\n",
                      refAppData_p->m_fibTblTwoId);

    /* Configure FTM searches now */
    refAppData_p->m_numOfFibSrches = NLM_GTM_FTM_REFAPP_FIB_NUM_SRCHES;
    refAppData_p->m_fibSrchOneRsltPortNum = NLM_GTM_FTM_REFAPP_FIB_SRCH_ONE_RSLT_PORT_NUM;
    refAppData_p->m_fibSrchOneStartBitValue = NLM_GTM_FTM_REFAPP_FIB_SRCH_ONE_START_BIT;
    refAppData_p->m_fibSrchTwoRsltPortNum = NLM_GTM_FTM_REFAPP_FIB_SRCH_TWO_RSLT_PORT_NUM;
    refAppData_p->m_fibSrchTwoStartBitValue = NLM_GTM_FTM_REFAPP_FIB_SRCH_TWO_START_BIT;

    /* Initialize FIB parallel srch Attrs structure */
    psAttrs.m_numOfFibParallelSrch = refAppData_p->m_numOfFibSrches;
    psAttrs.m_parallelSrchInfo[0].m_tblId = refAppData_p->m_fibTblOneId;
    psAttrs.m_parallelSrchInfo[0].m_rsltPortNum = refAppData_p->m_fibSrchOneRsltPortNum;
    psAttrs.m_parallelSrchInfo[0].m_startBitInKey = refAppData_p->m_fibSrchOneStartBitValue;
    psAttrs.m_parallelSrchInfo[1].m_tblId = refAppData_p->m_fibTblTwoId;
    psAttrs.m_parallelSrchInfo[1].m_rsltPortNum = refAppData_p->m_fibSrchTwoRsltPortNum;
    psAttrs.m_parallelSrchInfo[1].m_startBitInKey = refAppData_p->m_fibSrchTwoStartBitValue;

    /* Configure search for LTR 0 */
    if( NLMERR_OK != NlmFibTblMgr__ConfigSearch(
                                            refAppData_p->m_fibTblMgr_p,
											refAppData_p->m_srchLtrNum,
											&psAttrs,
											&reason ))
    {
        NlmCm__printf("\tFTM Search Configuration for LTR#%d failed...\n",
                            refAppData_p->m_srchLtrNum);
        NlmGtmFtmRefApp_PrintReasonCode( reason );
        return NLM_RETURN_STATUS_FAIL;
    }
    NlmCm__printf("\tFTM Search Configuration for LTR#%d done successfully...\n",
        refAppData_p->m_srchLtrNum);

	/* FTM Configuration is finished, Lock it now */
	if( NLMERR_OK != NlmFibTblMgr__LockConfiguration(
									refAppData_p->m_fibTblMgr_p,
									&reason ) )
	{
		NlmCm__printf("\tFTM LockConfig failed...\n");
		NlmGtmFtmRefApp_PrintReasonCode( reason );

		return NLM_RETURN_STATUS_FAIL;
	}
	NlmCm__printf("\tFTM Configuration Locked");

	return NLM_RETURN_STATUS_OK;
}

/* Destroy FIB tables and FIB table manager instance */
static NlmErrNum_t NlmGtmFtmRefApp_DestroyFibTableManager(NlmGtmFtmRefAppData *refAppData_p)
{
	NlmReasonCode   reason = NLMRSC_REASON_OK;

    /* Destroying the two FIB tables */
    NlmCm__printf("\n\tDestroying FIB table with tbl id [%s]\n", refAppData_p->m_fibTblOneId);
    if( NLMERR_OK != NlmFibTblMgr__DestroyTable(refAppData_p->m_fibTblOne_p,
                                                &reason ) )
    {
        NlmCm__printf("\tDestroying FIB table with tbl id [%s] failed\n ",  refAppData_p->m_fibTblOneId);
        NlmGtmFtmRefApp_PrintReasonCode( reason );
        return NLM_RETURN_STATUS_FAIL;
    }
    NlmCm__printf("\tDestroying FIB table with tbl id [%s]\n", refAppData_p->m_fibTblTwoId);
    if( NLMERR_OK != NlmFibTblMgr__DestroyTable(refAppData_p->m_fibTblTwo_p,
                                                &reason ) )
    {
        NlmCm__printf("\tDestroying FIB table with tbl id [%s] failed\n ",  refAppData_p->m_fibTblTwoId);
        NlmGtmFtmRefApp_PrintReasonCode( reason );
        return NLM_RETURN_STATUS_FAIL;
    }

	/* Destroying the Fib table manager */
	NlmCm__printf("\tDestroying Fib Table Manager\n");
	if( NLMERR_OK != NlmFibTblMgr__Destroy( refAppData_p->m_fibTblMgr_p, &reason ) )
	{
		NlmCm__printf("\tFib Table Manager destroy failed...\n");
		NlmGtmFtmRefApp_PrintReasonCode( reason );
		return NLM_RETURN_STATUS_FAIL;
	}

	return NLM_RETURN_STATUS_OK;
}

/* NlmGtmFtmRefApp_GeneratePrefix function generates prefix
in the incremental order; length of the prefix is provided
by pfxLen, and prefix is prepended with tblId at its MSB.
Note: This function does not create the prefix data structure
but expects the caller to create it and hence also destroy it */
static NlmErrNum_t NlmGtmFtmRefApp_GeneratePrefix(
    NlmCmPrefix *prefix_p,
    nlm_u32 pfxLen,
    nlm_8 *tblIdStr,
    nlm_u32 iter
    )
{
    nlm_u8 pfxData[NLMDEV_FIB_MAX_PREFIX_LENGTH/8] = {0};
    nlm_32 tableId;

    if(pfxLen < NLM_GTM_FTM_REFAPP_FIB_TBL_ID_LEN) /* Pfxlen cannot be less than length of tblId  */
    {
        NlmCm__printf("\n\tError: Pfx Len Should Not Be Less Than TblIdLen");
        return NLM_RETURN_STATUS_FAIL;
    }

    if(pfxLen > NLMDEV_FIB_MAX_PREFIX_LENGTH)/* Pfxlen cannot be greater than 320b  */
    {
        NlmCm__printf("\n\tError: Pfx Len Should Not Be Greater Than 320b");
        return NLM_RETURN_STATUS_FAIL;
    }

    /* Generate the prefix data based on the iter value and pfxlen */
    if(pfxLen >= 36)
        WriteBitsInArray(pfxData, NLMDEV_FIB_MAX_PREFIX_LENGTH/8,
            (NLMDEV_FIB_MAX_PREFIX_LENGTH - pfxLen + 31),
            (NLMDEV_FIB_MAX_PREFIX_LENGTH - pfxLen),
            iter);
    else
        WriteBitsInArray(pfxData, NLMDEV_FIB_MAX_PREFIX_LENGTH/8,
            NLMDEV_FIB_MAX_PREFIX_LENGTH - (NLM_GTM_FTM_REFAPP_FIB_TBL_ID_LEN + 1),
            (NLMDEV_FIB_MAX_PREFIX_LENGTH - pfxLen),
            iter & pfxLen);

    NlmCm__atoi(tblIdStr, &tableId);

    /* Prepend  the table id to the prefix */
    WriteBitsInArray(pfxData, NLMDEV_FIB_MAX_PREFIX_LENGTH/8,
        NLMDEV_FIB_MAX_PREFIX_LENGTH - 1,
        (NLMDEV_FIB_MAX_PREFIX_LENGTH - NLM_GTM_FTM_REFAPP_FIB_TBL_ID_LEN),
        tableId);

    NlmCmPrefix__Set(prefix_p, pfxLen, pfxData);

    return NLM_RETURN_STATUS_OK;
}

/* NlmGtmFtmRefApp_LoadFibTables function adds the prefixes in the incremental order to
the specified table */
static NlmErrNum_t NlmGtmFtmRefApp_LoadFibTables(
    NlmGtmFtmRefAppData *refAppData_p
    )
{
    nlm_u32 iter;
    NlmCmPrefix *prefix_p;
    NlmErrNum_t errNum;
    NlmReasonCode reason;

    /* Add the prefixes to 36b FIB tbl first */
    refAppData_p->m_fibTblOneNumOfPfxsToBeAdded = NLM_GTM_FTM_REFAPP_FIB_TBL_ONE_NUM_PFXS;

    /* Create a prefix data structure using the utility function */
    prefix_p = NlmCmPrefix__create(refAppData_p->m_alloc_p,
                                    refAppData_p->m_fibTblOneWidth,
                                           0, NULL);
    iter = 0;
    while(refAppData_p->m_fibTblOne_p->m_numPrefixes <
            refAppData_p->m_fibTblOneNumOfPfxsToBeAdded)
    {
        /* Generate a prefix to be added;
        Prefixes generated are in incremental order. For e.g., for 32b table with
        table id 0, prefixes generated are : 0.0.0.0, 0.0.0.1, 0.0.0.2 and so on */
        if((errNum = NlmGtmFtmRefApp_GeneratePrefix(prefix_p,
                                                refAppData_p->m_fibTblOneWidth,
                                                refAppData_p->m_fibTblOneId,
                                                iter)) != NLMERR_OK)
            return errNum;

        /* add the prefix using the FIB TblMgr API */
        if((errNum = NlmFibTblMgr__AddPrefix(refAppData_p->m_fibTblOne_p,
                                             prefix_p,
                                             &reason)) != NLMERR_OK)
        {
            if(reason !=  NLMRSC_DUPLICATE_PREFIX)/* if reason for failure is
                                                        duplicate prefix then dont exit
                                                        but generate new prefix */
            {
                NlmCm__printf("\tAdding Prefix for Fib Table with table id[%s] failed...\n",
                    refAppData_p->m_fibTblOneId );
                NlmGtmFtmRefApp_PrintReasonCode( reason );
                NlmCmPrefix__destroy(prefix_p, refAppData_p->m_alloc_p);
            	return NLM_RETURN_STATUS_FAIL;
            }
            iter++;
        }
    }

    /* Destroy the prefix data structure created */
    NlmCmPrefix__destroy(prefix_p, refAppData_p->m_alloc_p);
    NlmCm__printf("\n\t%d Number of Prefixes Added to Fib Tbl with TblId[%s]\n",
        refAppData_p->m_fibTblOneNumOfPfxsToBeAdded,
        refAppData_p->m_fibTblOneId);

    /* Now add the prefixes to second table of 48b */
    refAppData_p->m_fibTblTwoNumOfPfxsToBeAdded = NLM_GTM_FTM_REFAPP_FIB_TBL_TWO_NUM_PFXS;
    /* Create a prefix data structure using the utility function */
    prefix_p = NlmCmPrefix__create(refAppData_p->m_alloc_p,
                                  refAppData_p->m_fibTblTwoWidth,
                                  0, NULL);

    iter = 0;
    while(refAppData_p->m_fibTblTwo_p->m_numPrefixes <
                refAppData_p->m_fibTblTwoNumOfPfxsToBeAdded)
    {
        /* Generate a prefix to be added;
        Prefixes generated are in incremental order. For e.g., for 32b table with
        table id 0, prefixes generated are : 0.0.0.0, 0.0.0.1, 0.0.0.2 and so on */
        if((errNum = NlmGtmFtmRefApp_GeneratePrefix(prefix_p,
                                                refAppData_p->m_fibTblTwoWidth,
                                                refAppData_p->m_fibTblTwoId,
                                                iter)) != NLMERR_OK)
            return errNum;

        /* add the prefix using the FIB TblMgr API */
        if((errNum = NlmFibTblMgr__AddPrefix(refAppData_p->m_fibTblTwo_p,
                                             prefix_p,
                                             &reason)) != NLMERR_OK)
        {
            if(reason !=  NLMRSC_DUPLICATE_PREFIX)/* if reason for failure is
                                                        duplicate prefix then dont exit
                                                        but generate new prefix */
            {
                NlmCm__printf("\tAdding Prefix for Fib Table with table id[%s] failed...\n",
                    refAppData_p->m_fibTblTwoId );
                NlmGtmFtmRefApp_PrintReasonCode( reason );
                NlmCmPrefix__destroy(prefix_p, refAppData_p->m_alloc_p);
            	return NLM_RETURN_STATUS_FAIL;
            }
            iter++;
        }
    }

    /* Destroy the prefix data structure created */
    NlmCmPrefix__destroy(prefix_p, refAppData_p->m_alloc_p);
    NlmCm__printf("\t%d Number of Prefixes Added to Fib Tbl with TblId[%s]\n",
        refAppData_p->m_fibTblTwoNumOfPfxsToBeAdded,
        refAppData_p->m_fibTblTwoId);


    return NLM_RETURN_STATUS_OK;
}

/* NlmGtmFtmRefApp_GenerateGtmRecord function generates a 160b record
based on iter value;
Note: This function does not stitch the range encodings if the record
contains range fields */
static void NlmGtmFtmRefApp_GenerateGtmRecord(nlm_u8 *recordData,
                                              nlm_u8 *recordMask,
                                              nlm_u8 tblId,
                                              nlm_u16 groupId,
                                              nlm_u32 iter
                                              )
{
    nlm_u32 recordWidth = NLM_GTM_FTM_REFAPP_GTM_TBL_WIDTH/8;

    /* Gtm Record is of the following pattern;
      Assuming record width of 160b
       Bits[159:156] -- 4b tblId
       Bits[155:152] -- 4b Dont Cares (locally masked)
       Bits[151:136] -- 16b groupId
       Bits[135:88] -- 48b fixed part of record (0xa5a5a5a5a5a5)
       Bits[87:64] -- 24b Dont Cares (locally masked)
       Bits[63:32] -- 32b incremental pattern based on iter value
       Bits[31:0]  -- 32b range encoding */
    NlmCm__memset(recordData, 0xa5, recordWidth);
    NlmCm__memset(recordMask, 0x0, recordWidth);

    /* Inserting tblId */
    WriteBitsInArray(recordData, recordWidth, 159, 156, tblId);
    /* Inserting groupId */
    WriteBitsInArray(recordData, recordWidth, 151, 136, groupId);
    /* Inserting variable part; Incremental pattern */
    WriteBitsInArray(recordData, recordWidth, 63, 32, iter);

    /* Inserting Dont cares */
    WriteBitsInArray(recordMask, recordWidth, 155, 152, 0xF);
    WriteBitsInArray(recordMask, recordWidth, 87, 64, 0xFFFFFF);
}

/* NlmGtmFtmRefApp_LoadGtmTable adds records to the GTM Table */
static NlmErrNum_t NlmGtmFtmRefApp_LoadGtmTable(NlmGtmFtmRefAppData *refAppData_p)
{
    nlm_u32 iter = 0;
    nlm_32 tblId;
    nlm_u16 groupNum;
    NlmRangeEncoded *rangeEncodings;
    nlm_u16 rangeNum;
    nlm_u16 rangeEncodingNum;
    NlmGenericTblRecord record;
    nlm_u16 recordPriority = 0;
    nlm_u32 recordIndex;
    nlm_u8 recordData[NLM_GTM_FTM_REFAPP_GTM_TBL_WIDTH/8];
    nlm_u8 recordMask[NLM_GTM_FTM_REFAPP_GTM_TBL_WIDTH/8];
    NlmReasonCode reason;
    NlmErrNum_t errNum;

    NlmCm__atoi(refAppData_p->m_gtmTblId, &tblId);

    refAppData_p->m_gtmTblNumOfRecords = 0;
    for(groupNum = 0; groupNum < NLM_GTM_FTM_REFAPP_GTM_NUM_OF_GROUPS; groupNum++)
    {
        for(rangeNum = 0; rangeNum < NLM_GTM_FTM_REFAPP_GTM_NUM_OF_RANGES; rangeNum++)
        {
            /* Generate the GTM record without range encodings*/
            NlmGtmFtmRefApp_GenerateGtmRecord(recordData, recordMask, (nlm_u8)tblId, groupNum, iter);

            /* Get the encodings for each range value and then stitch them with the record*/
            if(NULL == (rangeEncodings = NlmRangeMgr__GetRangeEncoding(refAppData_p->m_rangeMgr_p,
                                                                       refAppData_p->m_rangeDb_p,
                                                                       refAppData_p->m_ranges[rangeNum].m_id,
                                                                       &reason)))
            {
                NlmCm__printf("Could not get encodings for range with range id = %u. Exiting...\n",
                    refAppData_p->m_ranges[rangeNum].m_id);
                NlmGtmFtmRefApp_PrintReasonCode( reason );
                return NLM_RETURN_STATUS_FAIL;
            }

            for(rangeEncodingNum = 0; rangeEncodingNum < rangeEncodings->m_num_entries;
                                        rangeEncodingNum++)
            {
                /* Stitch the Range Encodings to the Record and Add the Record to the GTM Table */
                WriteBitsInArray(recordData, NLM_GTM_FTM_REFAPP_GTM_TBL_WIDTH/8,
                    31, 0, rangeEncodings->m_entries_p[rangeEncodingNum].m_data);
                WriteBitsInArray(recordMask, NLM_GTM_FTM_REFAPP_GTM_TBL_WIDTH/8,
                    31, 0, rangeEncodings->m_entries_p[rangeEncodingNum].m_mask);

                /* Initialize the record structure and add the record to table */
                record.m_len = NLM_GTM_FTM_REFAPP_GTM_TBL_WIDTH;
                record.m_data = recordData;
                record.m_mask = recordMask;
                if((errNum = NlmGenericTblMgr__AddRecord(refAppData_p->m_gtmTbl_p,
                                             &record,
                                             groupNum,
                                             recordPriority,
                                             &recordIndex,
                                             &reason)) != NLMERR_OK)
                {
                    NlmCm__printf("\tAdding Record to GTM Table with table id[%s] failed...\n",
                        refAppData_p->m_gtmTblId );
                    NlmGtmFtmRefApp_PrintReasonCode( reason );
                    return NLM_RETURN_STATUS_FAIL;
                }
                refAppData_p->m_gtmTblNumOfRecords++;
            }
            iter++;
        }
    }

    NlmCm__printf("\n\n\t%d Number of Records Added to Gtm Tbl with TblId[%s]\n",
        refAppData_p->m_gtmTblNumOfRecords,
        refAppData_p->m_gtmTblId);
    return NLM_RETURN_STATUS_OK;
}

/* Constructs the 160b GTM Table search key based on groupNum, rangeNum,
and iter value */
void NlmGtmFtmRefApp_ConstructGtmTableSrchKey(NlmGtmFtmRefAppData *refAppData_p,
                                              nlm_u8 *srchKey,
                                              nlm_u16 *groupNum,
                                              nlm_u16 *rangeNum,
                                              nlm_u32 *iter
                                              )
{
    nlm_32 tblId;
    nlm_u16 rangeValue;
    nlm_u8 randValue;
    if(*rangeNum == NLM_GTM_FTM_REFAPP_GTM_NUM_OF_RANGES)
    {
        (*groupNum)++;
        *rangeNum = 0;
    }

    if(*groupNum == NLM_GTM_FTM_REFAPP_GTM_NUM_OF_GROUPS)
    {
        *iter = 0;
        *groupNum = 0;
    }

    NlmCm__memset(srchKey, 0xa5, 40);

    /* Insert the GTM tblId in the key; Bits[319:316] */
    NlmCm__atoi(refAppData_p->m_gtmTblId, &tblId);
    WriteBitsInArray(srchKey, 40, 319, 316, tblId);

    /* Insert the GTM groupId in the key; Bits[311:296] */
    WriteBitsInArray(srchKey, 40, 311, 296, *groupNum);

    /* Insert the variable part of GTM record in the key; Bits[223: 192] */
    WriteBitsInArray(srchKey, 40, 223, 192, *iter);

    /* Generate the range field to be inserted in the key
    some random value between range start and end value */
    do{
        randValue = (nlm_u8)(rand()%256);
        rangeValue = refAppData_p->m_ranges[*rangeNum].m_start + randValue;
    }while(rangeValue > refAppData_p->m_ranges[*rangeNum].m_end);

    /* Insert range value in the key ; Bits[191:176] */
    WriteBitsInArray(srchKey, 40, 191, 176, rangeValue);
    (*iter)++;
    (*rangeNum)++;

}

/* NlmGtmFtmRefApp_ConstructFibTablesSrchKey function constructs the
36b and 48b Fib Table Keys based on iter value and start bit values */
void NlmGtmFtmRefApp_ConstructFibTablesSrchKey(NlmGtmFtmRefAppData *refAppData_p,
                                              nlm_u8 *srchKey,
                                              nlm_u32 *iter
                                              )
{
    nlm_32 tblId;
     /* Insert the Fib Table One tblId in the key; Bits[79:76] */
    NlmCm__atoi(refAppData_p->m_fibTblOneId, &tblId);
    WriteBitsInArray(srchKey, 40, 79, 76, tblId);

    /* Insert the Fib Table One Prefix to be saerched in the key; Bits[75:44]*/
    WriteBitsInArray(srchKey, 40, 75, 44, *iter);

    /* Insert the Fib Table Two tblId in the key; Bits[159:156] */
    NlmCm__atoi(refAppData_p->m_fibTblTwoId, &tblId);
    WriteBitsInArray(srchKey, 40, 159, 156, tblId);

    /* Insert the Fib Table Two Prefix to be searched in the key; Bits[155:112]
    Bits 155 - 144 are always zero(assuming to be VPN Id) whereas the other 32b
    are based on iter value*/
    WriteBitsInArray(srchKey, 40, 155, 144, 0);
    WriteBitsInArray(srchKey, 40, 143, 112, *iter);

    (*iter)++;
}

/* NlmGtmFtmRefApp_PerformCompareOperations function performs device compare operation
   using only one LTR i.e 0 which is configured to produce two FIB results on Result Port
   #0 and #1 in addition to one GTM table result on Result Port #3 */
static NlmErrNum_t NlmGtmFtmRefApp_PerformCompareOperations( NlmGtmFtmRefAppData *refAppData_p )
{
	NlmReasonCode   reason = NLMRSC_REASON_OK;

	/* Device Manager declarations */
	NlmDevCtxBufferInfo      cbInfo;
	NlmDevCmpRslt     searchResult;

    nlm_u8 srchKey[40]; /* compare 1 operation srch key is 320b i.e 40 bytes */
    nlm_u32 numOfCompares;
    nlm_u32 compareNum;
    nlm_u16 rangeNum = 0;
    nlm_u16 groupNum = 0;
    nlm_u32 iter1 = 0;
    nlm_u32 iter2 = 0;
    nlm_u8 ltrNum = 0;


	NlmCm__printf("\n\tPerforming compare operations using LTR[%d]\n",
        refAppData_p->m_srchLtrNum);

    /* Number of compares to be done is based on number of prefixes added to fib tables */
    if(refAppData_p->m_fibTblOneNumOfPfxsToBeAdded < refAppData_p->m_fibTblTwoNumOfPfxsToBeAdded)
        numOfCompares = refAppData_p->m_fibTblOneNumOfPfxsToBeAdded;
    else
        numOfCompares = refAppData_p->m_fibTblTwoNumOfPfxsToBeAdded;

    for(compareNum = 0; compareNum < numOfCompares; compareNum++)
    {
	    /* Master Key for Compare1 operation is as follows
        Bits[319:160] -- GTM Table Search Key;
        Bits[159:112] -- 48b Fib Table Search Key
        Bits[79:44] -- 36b Fib Table Search Key
        */
        NlmCm__memset(srchKey, 0, 40);
        NlmGtmFtmRefApp_ConstructGtmTableSrchKey(refAppData_p,
                                                 srchKey,
                                                &groupNum,
                                                &rangeNum,
                                                &iter1);

        NlmGtmFtmRefApp_ConstructFibTablesSrchKey(refAppData_p,
                                                 srchKey,
                                                 &iter2);


#ifndef NLM_XLP

    	/* write the MSB 160b portion of srch key into Context Buffer address '2' */
        cbInfo.m_cbStartAddr = 2;
        cbInfo.m_datalen = (160 >> 3);
        NlmCm__memcpy(cbInfo.m_data, srchKey, (160 >> 3));
    	if( NLMERR_OK != NlmDevMgr__CBWrite( refAppData_p->m_devMgr_p, &cbInfo, &reason ) )
		{
            NlmCm__printf("CBWrite failed for compare #%d. Exiting...\n", compareNum);
			NlmGtmFtmRefApp_PrintReasonCode( reason );
        	return NLM_RETURN_STATUS_FAIL;
    	}

        /* The LSB 160b portion of the search key is passed with Cmp instruction with CB Addr as '0' */
    	cbInfo.m_cbStartAddr = 0;
        cbInfo.m_datalen = (160 >> 3);
        NlmCm__memcpy(cbInfo.m_data, &srchKey[(160 >> 3)], (160 >> 3));
#else

	/* Write 320b of search key by using Cmp instruction with CB Addr as '0' */
	cbInfo.m_cbStartAddr = 0;
	cbInfo.m_datalen = (320 >> 3);
	NlmCm__memcpy(cbInfo.m_data, srchKey, (320 >> 3));

#endif
        /* Perform Compare Operation Now */
        if( NLMERR_OK != NlmDevMgr__Compare1( refAppData_p->m_devMgr_p,
                                                ltrNum,
                                                &cbInfo,
                                                &searchResult,
                                                &reason ) )
        {
            NlmCm__printf("Compare Operation failed for compare #%d. Exiting...\n", compareNum);
            NlmGtmFtmRefApp_PrintReasonCode( reason );
            return NLM_RETURN_STATUS_FAIL;
        }

        /* Now Check the compare results
        Hits Expected on Result Port 0, 1 and 3
        Throw Error if Miss Found on these ports */
        if(searchResult.m_hitOrMiss[0] == (Nlm11kDevMissHit)NLMDEV_MISS ||
            searchResult.m_hitOrMiss[1] == (Nlm11kDevMissHit)NLMDEV_MISS ||
            searchResult.m_hitOrMiss[3] == (Nlm11kDevMissHit)NLMDEV_MISS)
        {
            NlmCm__printf("Compare Operation Returns Miss for compare Num %d, Expected Hit\n", compareNum);
            NlmCm__printf("search_result0 = %s\n",
                searchResult.m_hitOrMiss[0] ? "Hit" : "Miss");
            NlmCm__printf("search_result1 = %s\n",
                searchResult.m_hitOrMiss[1] ? "Hit" : "Miss");
            NlmCm__printf("search_result3 = %s\n",
                searchResult.m_hitOrMiss[3]? "Hit" : "Miss");
            return NLM_RETURN_STATUS_FAIL;
        }
        if( 0 == (compareNum % NLM_GTM_FTM_REFAPP_DEBUG_PRINT_ITER_VAL))
            NlmCm__printf("\r\t[%u] Number of compares successful so far", compareNum);
    }

	NlmCm__printf("\r\t[%u] Number of compares done successfully \n", compareNum);
	return NLM_RETURN_STATUS_OK;
}

int nlmgtmftm_refapp_main(int argc, char	*argv[])
{
	NlmGtmFtmRefAppData  refAppData;
	nlm_32	break_alloc_id = -1;
    NlmCmAllocator allocBody;

	(void)argc;
	(void)argv;

	NlmCmDebug__Setup( break_alloc_id, NLMCM_DBG_EBM_ENABLE );

	NlmCm__printf ("\n\t GTM and FTM Integrated Application Reference Code\n\n\n");

    /* create default memory allocator */
    refAppData.m_alloc_p = NlmCmAllocator__ctor( &allocBody );
    if(refAppData.m_alloc_p == NULL)
    {
        NlmCm__printf("\nMemory Allocator Init Failed.\n");
        return NLM_RETURN_STATUS_ABORT;
    }

	/* Initialize Memory Allocator, XPT Interface */
	if( NLM_RETURN_STATUS_OK != NlmGtmFtmRefApp_InitEnvironment( &refAppData ) )
	{
		NlmCm__printf("\tEnvironment Initialization failed. Exiting...\n");

		return NLM_RETURN_STATUS_ABORT;
	}

	/* Initialize Device Manager */
	if( NLM_RETURN_STATUS_OK != NlmGtmFtmRefApp_InitDeviceManager( &refAppData ) )
	{
		NlmCm__printf("\tDevice Manager Initialization failed. Exiting...\n");

		return NLM_RETURN_STATUS_ABORT;
	}

	/* Initialize Generic Table Manager */
	if( NLM_RETURN_STATUS_OK != NlmGtmFtmRefApp_InitGenericTableManager( &refAppData ) )
	{
		NlmCm__printf("\tGeneric Table Manager Initialization failed. Exiting...\n");

		return NLM_RETURN_STATUS_ABORT;
	}

    /* Initialize Range Manager */
	if( NLM_RETURN_STATUS_OK != NlmGtmFtmRefApp_InitRangeManager( &refAppData ) )
	{
		NlmCm__printf("\tRange Manager Initialization failed. Exiting...\n");

		return NLM_RETURN_STATUS_ABORT;
	}

    /* Initialize Fib Table Manager */
	if( NLM_RETURN_STATUS_OK != NlmGtmFtmRefApp_InitFibTableManager( &refAppData ) )
	{
		NlmCm__printf("\tFIB Table Manager Initialization failed. Exiting...\n");

		return NLM_RETURN_STATUS_ABORT;
	}

    /* Load Gtm Table with records */
    if( NLM_RETURN_STATUS_OK != NlmGtmFtmRefApp_LoadGtmTable( &refAppData ) )
	{
		NlmCm__printf("\tAdding Records to GTM Tables failed. Exiting...\n");

		return NLM_RETURN_STATUS_ABORT;
	}

    /* Load Fib Tables with prefixes*/
    if( NLM_RETURN_STATUS_OK != NlmGtmFtmRefApp_LoadFibTables( &refAppData ) )
	{
		NlmCm__printf("\tAdding Prefixes to FIB Tables failed. Exiting...\n");

		return NLM_RETURN_STATUS_ABORT;
	}

    /* Now perform compare operations on the tables */
    if( NLM_RETURN_STATUS_OK != NlmGtmFtmRefApp_PerformCompareOperations( &refAppData ) )
	{
		NlmCm__printf("\t Compare Operation Failed Exiting...\n");

		return NLM_RETURN_STATUS_ABORT;
	}

	/* Destroy Fib Table Manager */
    if( NLM_RETURN_STATUS_OK != NlmGtmFtmRefApp_DestroyFibTableManager( &refAppData ) )
    {
        NlmCm__printf("\tGeneric Table Manager Destroy failed. Exiting...\n");

        return NLM_RETURN_STATUS_ABORT;
    }

    /* Destroy Range Manager */
	if( NLM_RETURN_STATUS_OK != NlmGtmFtmRefApp_DestroyRangeManager( &refAppData ) )
	{
		NlmCm__printf("\tRange Manager Destroy failed. Exiting...\n");

		return NLM_RETURN_STATUS_ABORT;
	}

	/* Destroy Generic Table Manager */
    if( NLM_RETURN_STATUS_OK != NlmGtmFtmRefApp_DestroyGenericTableManager( &refAppData ) )
    {
        NlmCm__printf("\tGeneric Table Manager Destroy failed. Exiting...\n");

        return NLM_RETURN_STATUS_ABORT;
    }

	/* Destroy device manager */
	if( NLM_RETURN_STATUS_OK != NlmGtmFtmRefApp_DestroyDeviceManager( &refAppData ) )
	{
		NlmCm__printf("\tDevice Manager Destroy failed. Exiting...\n");

		return NLM_RETURN_STATUS_ABORT;
	}

    /* Destroy the environment */
	if( NLM_RETURN_STATUS_OK != NlmGtmFtmRefApp_DestroyEnvironment( &refAppData ) )
	{
		NlmCm__printf("\tDestroy Environment failed. Exiting...\n");

		return NLM_RETURN_STATUS_ABORT;
	}

	if(NlmCmDebug__IsMemLeak())
	{
        NlmCm__printf("\tMemory Leak\n");
	}

	NlmCm__printf("\n\tProgram Completed Successfully\n");

	return 0;
}

