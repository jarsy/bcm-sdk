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
 /* 
 * $Id: nlmgenerictblmgr_refapp.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2007 Broadcom Corp.
 * All Rights Reserved.$
 */

#include "nlmgenerictblmgr_refapp.h"
#ifdef NLM_XLP
#include "nlmxlpxpt.h"
#endif

/* Number of devices */
#define NLM_REFAPP_NUM_OF_DEVICES     1

static void CreateRecordsInTables( genericTblMgrRefAppData *refAppData_p );
static nlm_u8 FillRecordPatternFor_80B0_Table( nlm_u8 *data_p, nlm_u8 *mask_p );
static nlm_u8 FillRecordPatternFor_80B1_Table( nlm_u8 *data_p, nlm_u8 *mask_p );
static nlm_u8 FillRecordPatternFor_160B2_Table( nlm_u8 *data_p, nlm_u8 *mask_p );
static nlm_u8 FillRecordPatternFor_160B3_Table( nlm_u8 *data_p, nlm_u8 *mask_p );
static nlm_u8 FillRecordPatternFor_320B4_Table( nlm_u8 *data_p, nlm_u8 *mask_p );
static nlm_u8 FillRecordPatternFor_640B5_Table( nlm_u8 *data_p, nlm_u8 *mask_p );

/* define NETL_DEBUG to get the index change log messages */
#ifdef NETL_DEBUG
#undef NETL_DEBUG
#endif

static void printReasonCode(NlmReasonCode reason)
{
	NlmCmFile__printf("\tReason Code = ");
	
	switch( reason )
	{
	case NLMRSC_REASON_OK:
		NlmCmFile__printf("NLMRSC_REASON_OK");
		break;

	case NLMRSC_INVALID_MEMALLOC_PTR:
		NlmCmFile__printf("NLMRSC_INVALID_MEMALLOC_PTR");
		break;

	case NLMRSC_INVALID_DEV_PTR:
		NlmCmFile__printf("NLMRSC_INVALID_DEV_PTR");
		break;

	case NLMRSC_INVALID_DEVMGR_PTR:
		NlmCmFile__printf("NLMRSC_INVALID_DEVMGR_PTR");
		break;

	case NLMRSC_INVALID_KEY_PTR:
		NlmCmFile__printf("NLMRSC_INVALID_KEY_PTR");
		break;

	case NLMRSC_INVALID_SRCH_RSLT_PTR:
		NlmCmFile__printf("NLMRSC_INVALID_SRCH_RSLT_PTR");
		break;
		
	case NLMRSC_INVALID_XPT_PTR:
		NlmCmFile__printf("NLMRSC_INVALID_XPT_PTR");
		break;

	case NLMRSC_INVALID_XPT_RQT_PTR:
		NlmCmFile__printf("NLMRSC_INVALID_XPT_RQT_PTR");
		break;

	case NLMRSC_INVALID_XPT_RSLT_PTR:
		NlmCmFile__printf("NLMRSC_INVALID_XPT_RSLT_PTR");
		break;

	case NLMRSC_INVALID_OUTPUT:
		NlmCmFile__printf("NLMRSC_INVALID_OUTPUT");
		break;

	case NLMRSC_INVALID_INPUT:
		NlmCmFile__printf("NLMRSC_INVALID_INPUT");
		break;

	case NLMRSC_INVALID_REG_ADDRESS:
		NlmCmFile__printf("NLMRSC_INVALID_REG_ADDRESS");
		break;

	case NLMRSC_INVALID_DB_ADDRESS:
		NlmCmFile__printf("NLMRSC_INVALID_DB_ADDRESS");
		break;
		
	case NLMRSC_INVALID_CB_ADDRESS:
		NlmCmFile__printf("NLMRSC_INVALID_CB_ADDRESS");
		break;

	case NLMRSC_INVALID_DATA:
		NlmCmFile__printf("NLMRSC_INVALID_DATA");
		break;

	case NLMRSC_INVALID_LTR_NUM:
		NlmCmFile__printf("NLMRSC_INVALID_LTR_NUM");
		break;

	case NLMRSC_INVALID_AB_NUM:
		NlmCmFile__printf("NLMRSC_INVALID_AB_NUM");
		break;

	case NLMRSC_INVALID_AB_INDEX:
		NlmCmFile__printf("NLMRSC_INVALID_AB_INDEX");
		break;

	case NLMRSC_DUPLICATE_DEVICE:
		NlmCmFile__printf("NLMRSC_DUPLICATE_DEVICE");
		break;

	case NLMRSC_INVALID_PARAM:
		NlmCmFile__printf("NLMRSC_INVALID_PARAM");
		break;

	case NLMRSC_OPR_FAILURE:
		NlmCmFile__printf("NLMRSC_OPR_FAILURE");
		break;

	case NLMRSC_NOFREE_RQST:
		NlmCmFile__printf("NLMRSC_NOFREE_RQST");
		break;

	case NLMRSC_NORQST_AVBL:
		NlmCmFile__printf("NLMRSC_NORQST_AVBL");
		break;

	case NLMRSC_NORSLT_AVBL:
		NlmCmFile__printf("NLMRSC_NORSLT_AVBL");
		break;

	case NLMRSC_DEV_MGR_CONFIG_LOCKED:
		NlmCmFile__printf("NLMRSC_DEV_MGR_CONFIG_LOCKED");
		break;

	case NLMRSC_CASCADE_NOT_EXIST:
		NlmCmFile__printf("NLMRSC_CASCADE_NOT_EXIST");
		break;

	case NLMRSC_INVALID_PARENT:
		NlmCmFile__printf("NLMRSC_INVALID_PARENT");
		break;

	case NLMRSC_INVALID_RANGE_MGR_ATTR:
		NlmCmFile__printf("NLMRSC_INVALID_RANGE_MGR_ATTR");
		break;

	case NLMRSC_INVALID_RANGE_MGR:
		NlmCmFile__printf("NLMRSC_INVALID_RANGE_MGR");
		break;

	case NLMRSC_INVALID_DATABASE:
		NlmCmFile__printf("NLMRSC_INVALID_DATABASE");
		break;

	case NLMRSC_DUPLICATE_DATABASE_ID:
		NlmCmFile__printf("NLMRSC_DUPLICATE_DATABASE_ID");
		break;

	case NLMRSC_INVALID_RANGE_DB_ATTR:
		NlmCmFile__printf("NLMRSC_INVALID_RANGE_DB_ATTR");
		break;

	case NLMRSC_INVALID_RANGE:
		NlmCmFile__printf("NLMRSC_INVALID_RANGE");
		break;

	case NLMRSC_INVALID_OUTPUT_NUM_PTR:
		NlmCmFile__printf("NLMRSC_INVALID_OUTPUT_NUM_PTR");
		break;

	case NLMRSC_INVALID_OUTPUT_RNG_PTR:
		NlmCmFile__printf("NLMRSC_INVALID_OUTPUT_RNG_PTR");
		break;

	case NLMRSC_DUPLICATE_RANGE_ID:
		NlmCmFile__printf("NLMRSC_DUPLICATE_RANGE_ID");
		break;

	case NLMRSC_INVALID_GENERIC_TM:
		NlmCmFile__printf("NLMRSC_INVALID_GENERIC_TM");
		break;

	case NLMRSC_NO_LTR_CONFIGURED:
		NlmCmFile__printf("NLMRSC_NO_LTR_CONFIGURED");
		break;
	
	case NLMRSC_INVALID_GENERIC_TABLE:
		NlmCmFile__printf("NLMRSC_INVALID_GENERIC_TABLE");
		break;

	case NLMRSC_INVALID_APP_CALLBACK:
		NlmCmFile__printf("NLMRSC_INVALID_APP_CALLBACK");
		break;

	case NLMRSC_INVALID_TABLEID:
		NlmCmFile__printf("NLMRSC_INVALID_TABLEID");
		break;

	case NLMRSC_INVALID_SEARCH_ATTRIBUTES:
		NlmCmFile__printf("NLMRSC_INVALID_SEARCH_ATTRIBUTES");
		break;

	case NLMRSC_INVALID_PS_DEPEND_ATTRIBUTES:
		NlmCmFile__printf("NLMRSC_INVALID_PS_DEPEND_ATTRIBUTES");
		break;

	case NLMRSC_CONFIGURATION_LOCKED:
		NlmCmFile__printf("NLMRSC_CONFIGURATION_LOCKED");
		break;

	case NLMRSC_EXCESSIVE_NUM_TABLE_PAR_LTR:
		NlmCmFile__printf("NLMRSC_EXCESSIVE_NUM_TABLE_PAR_LTR");
		break;

	case NLMRSC_INVALID_BMR:
		NlmCmFile__printf("NLMRSC_INVALID_BMR");
		break;

	case NLMRSC_INVALID_RECORD:
		NlmCmFile__printf("NLMRSC_INVALID_RECORD");
		break;

	case NLMRSC_DEVICE_DATABASE_FULL:
		NlmCmFile__printf("NLMRSC_DEVICE_DATABASE_FULL");
		break;

	case NLMRSC_TABLE_FULL:
		NlmCmFile__printf("NLMRSC_TABLE_FULL");
		break;

	case NLMRSC_TABLE_LIMIT_EXCEEDED:
		NlmCmFile__printf("NLMRSC_TABLE_LIMIT_EXCEEDED");
		break;

	case NLMRSC_RECORD_NOTFOUND:
		NlmCmFile__printf("NLMRSC_RECORD_NOTFOUND");
		break;

	case NLMRSC_INVALID_RANGE_ATTRIBUTES:
		NlmCmFile__printf("NLMRSC_INVALID_RANGE_ATTRIBUTES");
		break;

	case NLMRSC_NO_SUPPORT_FOR_THIS_COMB:
		NlmCmFile__printf("NLMRSC_NO_SUPPORT_FOR_THIS_COMB");
		break;
		
	default:
		NlmCmFile__printf("Unknown ReasonCode");
		break;

	}

	NlmCmFile__printf("\n");
}

#ifdef NETL_DEBUG
	static void print160Bits( nlm_u8 *data_p )
	{
		nlm_u8 iter;

		for( iter = 0; iter < NLMDEV_CB_WIDTH_IN_BYTES; iter++ ){
			if( iter && ( (iter % 5) == 0 ) )
				NlmCmFile__printf("_");

			NlmCmFile__printf("%02x", *( data_p + iter ) );
		}

		NlmCmFile__printf("\n");
	}
#endif



/* This function takes unsigned integer, puts each byte into array pointed by 'data' */
static void WriteValueToBitVector4( nlm_u8 *data, nlm_u32 value )
{
	  data[ 0 ] = ( nlm_u8 )( 0xFF & ( value >> 24 ) );
	  data[ 1 ] = ( nlm_u8 )( 0xFF & ( value >> 16 ) );
	  data[ 2 ] = ( nlm_u8 )( 0xFF & ( value >> 8 ) );
	  data[ 3 ] = ( nlm_u8 )( 0xFF & ( value >> 0 ) );
}

static nlm_u8 GetTableId( nlm_8 *tblId_p )
{
	nlm_32 iter = 7;
	nlm_u8 tbl_id = 0;

	while( iter >= 0 )
	{
		if( *tblId_p == '1' )
			tbl_id |= (nlm_u8)(1 << iter);

		iter--;
		tblId_p++;
	}

	return tbl_id;
}


static void IndexChangeCallBack( void* client_p, 
								NlmGenericTbl* genericTbl_p,
								NlmRecordIndex oldIndex, 
								NlmRecordIndex newIndex )
{
	genericTblMgrRefAppData *refAppData_p = (genericTblMgrRefAppData *)client_p;
	tableInfo *tblInfo_p;
	tableRecordInfo *tblRecordInfo_p;
	nlm_u32 num_recs, iter_rec;
	nlm_u8 tbl_id;

#ifdef NETL_DEBUG
	NlmCmFile__printf("\tOld Index = %u, New Index = %u [Table = %s]\n",
						oldIndex, newIndex, genericTbl_p->m_id_str_p);
#endif

	/* Just return if oldIndex is NLM_GTM_INVALID_INDEX. tblRecordInfo_p->index
	 * is already updated when passed as outParam in __AddRecord() function call.
	 */
	if( oldIndex == NLM_GTM_INVALID_INDEX )
		return;

	tbl_id = GetTableId( genericTbl_p->m_id_str_p );
	tblInfo_p = &refAppData_p->tblInfo[ tbl_id ];
	tblRecordInfo_p = tblInfo_p->tblRecordInfo_p;
	num_recs = tblInfo_p->max_recCount;

	for( iter_rec = 0; iter_rec < num_recs; iter_rec++ )
	{
		if( (tblRecordInfo_p + iter_rec)->index == oldIndex)
		{
			(tblRecordInfo_p + iter_rec)->index = newIndex;
			break;
		}
	}

	if( iter_rec == num_recs )
	{
		NlmCmFile__printf("\tcallBackCB -- Invalid indexes; oldIndex = %u, newIndex =  %u\n",
								oldIndex, newIndex );
	}

	return;
}

static int InitEnvironment( genericTblMgrRefAppData *refAppData_p )
{

	NlmCm__memset( refAppData_p, 0, sizeof( genericTblMgrRefAppData ) );
	
	/* create default memory allocator */
    refAppData_p->alloc_p = NlmCmAllocator__ctor( &refAppData_p->alloc_bdy );
    NlmCmAssert( ( refAppData_p->alloc_p != NULL ), "Memory Allocator Init Failed.\n");
    if(refAppData_p->alloc_p == NULL)
        return RETURN_STATUS_FAIL;

	/* Currently Device Manager is flushing out each request immediately. */
	refAppData_p->request_queue_len = 1;
	refAppData_p->result_queue_len  = 1;

	/* Search system has only one channel (cascade of devices) */
	refAppData_p->channel_id = 0;
#ifndef NLMPLATFORM_BCM
	refAppData_p->if_type    = IFTYPE_CMODEL;

#ifdef NLM_XLP
	refAppData_p->if_type    = IFTYPE_XLPXPT;
#endif
#else  /* BCMPLATFROM */
	refAppData_p->if_type    =  IFTYPE_BCM_CALADAN3;
#endif /* NLMPLATFORM_BCM */
	/* Device is operating in Standard Operation Mode */
	refAppData_p->opr_mode	    = NLMDEV_OPR_STANDARD;

	/* Create transport interface. Only interface supported with this release is SimXpt */
	switch( refAppData_p->if_type )
	{
#ifndef NLMPLATFORM_BCM
		case IFTYPE_CMODEL:
		{
            refAppData_p->xpt_p = NlmSimXpt__Create( refAppData_p->alloc_p,
														NLM_DEVTYPE_2,
														0, /* this argument is ignored for this 
														         Processor */
														refAppData_p->request_queue_len,
			                           					refAppData_p->result_queue_len,
														refAppData_p->opr_mode,
                                                        0, /*this argument is ignored for this 
                                                                                                   Processor */
														refAppData_p->channel_id
													);

    		NlmCmAssert( ( refAppData_p->xpt_p != NULL ), "Could not create" \
								" Simulation Transport Interface. Exiting..\n");
			NlmCmFile__printf("\n\tSimulation Transport Interface Created Successfully\n");

            if(refAppData_p->xpt_p == NULL)
                return RETURN_STATUS_FAIL;

			break;
		}
		case IFTYPE_FPGA:
		{
			NlmCmFile__printf("\n\tThis interface type is not yet supported...\n");

			return RETURN_STATUS_FAIL;
		}
#ifdef NLM_XLP
		case IFTYPE_XLPXPT:
		{
			refAppData_p->xpt_p = NlmXlpXpt__Create(refAppData_p->alloc_p,
								NLM_DEVTYPE_2,
								refAppData_p->request_queue_len,
			                           		refAppData_p->opr_mode,
	                                                        refAppData_p->channel_id, 
	                                                        0, 0, 1,1);
			NlmCmFile__printf("\n XLP Transport Interface Created Successfully\n");

			 if(refAppData_p->xpt_p == NULL)
			 	return RETURN_STATUS_FAIL;
			 break;
		}
#endif
#endif  /* NLMPLATFORM_BCM */
                case IFTYPE_BCM_CALADAN3:
                    refAppData_p->xpt_p = soc_sbx_caladan3_etu_xpt_create(0,
                                              NLM_DEVTYPE_2, 0, refAppData_p->request_queue_len,
                                              refAppData_p->opr_mode, refAppData_p->channel_id);
		    NlmCmFile__printf("\n Caladan3 Transport Interface Created Successfully\n");
                    if (refAppData_p->xpt_p == NULL) 
                    {
                        NlmCm__printf("\n\tError: soc_sbx_caladan3_etu_xpt_create failed");
                        return RETURN_STATUS_FAIL;
                    }
                    break;

		default:
		{
			NlmCmFile__printf("Invalid interface type...\n");

			return RETURN_STATUS_FAIL;
		}

	}

	return RETURN_STATUS_OK;
}



/* Destroy SimXpt transport interface */
static int DestroyEnvironment( genericTblMgrRefAppData *refAppData_p )
{
	/* Check the XPT interface type and call appropriate functions */
	switch( refAppData_p->if_type )
	{
#ifndef NLMPLATFORM_BCM
		case IFTYPE_CMODEL:
		{
			NlmCmFile__printf("\n\tDestroying Simulation Transport Interface\n");
            NlmSimXpt__destroy( refAppData_p->xpt_p );
			
			break;
		}
		case IFTYPE_FPGA:
		{
			NlmCmFile__printf("\n\tThis interface type is not yet supported...\n");

			return RETURN_STATUS_FAIL;
		}
#ifdef NLM_XLP
		case IFTYPE_XLPXPT:
		{
			NlmCmFile__printf("\n\tDestroying XLP Transport Interface\n");
			NlmXlpXpt__Destroy( refAppData_p->xpt_p );

			break;
		}
#endif
#endif  /* NLMPLATFORM_BCM */
                case IFTYPE_BCM_CALADAN3:
                    if (refAppData_p->xpt_p)
                        soc_sbx_caladan3_etu_xpt_destroy(refAppData_p->xpt_p);
                    break;
		default:
		{
			NlmCmFile__printf("\n\tInvalid interface type...\n");

			return RETURN_STATUS_FAIL;
		}

	}

	return RETURN_STATUS_OK;
}


/* This function performs Device Manager related Inits */
static int InitDeviceManager( genericTblMgrRefAppData *refAppData_p )
{
	NlmDevId	dev_id;
	NlmReasonCode	reason = NLMRSC_REASON_OK;

	if( NULL == ( refAppData_p->devMgr_p = NlmDevMgr__create( refAppData_p->alloc_p,
															    refAppData_p->xpt_p,
																NLMDEV_OPR_STANDARD,
															   &reason
															  ) ) )
	{
		NlmCmFile__printf("\tCould not initialize Device Manager...\n" );
		printReasonCode( reason );

		return RETURN_STATUS_ABORT;
	}

	NlmCmFile__printf("\n\tDevice Manager Initialized Successfully\n");

	
	
	/* Now add a device to the search system */
	if( NULL == (refAppData_p->dev_p = NlmDevMgr__AddDevice(refAppData_p->devMgr_p,
																	 &dev_id,
																	 &reason
																	) ) )
	{
		NlmCmFile__printf("Could not add device to the search system...\n");
		printReasonCode( reason );

		return RETURN_STATUS_ABORT;
	}

	NlmCmFile__printf("\tDevice#0 Added to the search system\n");

	/* We are done with configurations. Now Lock Device Manager */
	if( NLMERR_OK != NlmDevMgr__LockConfig( refAppData_p->devMgr_p, &reason ) )
	{
		NlmCmFile__printf("Could not lock Device Manager...\n");
		printReasonCode( reason );

		return RETURN_STATUS_ABORT;
	}

	NlmCmFile__printf("\tDevice Manager Configuration Locked\n");

	return RETURN_STATUS_OK;
}


/* Destroys Device Manager instance */
static int DestroyDeviceManager( genericTblMgrRefAppData *refAppData_p )
{
	NlmCmFile__printf("\n\tDestroying Device Manager\n");
	NlmDevMgr__destroy( refAppData_p->devMgr_p );

	return RETURN_STATUS_OK;
}

/* This function performs Range Manager related Inits */
static int InitRangeManager( genericTblMgrRefAppData *refAppData_p )
{
	NlmReasonCode	reason;
	NlmDevConfigReg devConfigRegData ;

	/* {Range ID, Start Range, End Range, Pointer to encoded bitmaps} */
	NlmRange              srcDb_ranges[ NUM_OF_SRCPORT_DB_RANGES ] = 
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

	NlmRange              dstDb_ranges[ NUM_OF_DSTPORT_DB_RANGES ] = 
											   {
												 { 0, 1, 5, NULL          },
												 { 1, 3, 13, NULL         },
												 { 2, 9, 24, NULL         },
												 { 3, 1, 5, NULL          },
												 { 4, 515, 525, NULL      },
												 { 5, 200, 235, NULL      },
												 { 6, 150, 250, NULL      },
												 { 7, 1050, 1070, NULL    },
												 { 8, 500, 510, NULL      },
												 { 9, 1000, 1005, NULL    },
												 { 10, 10000, 10008, NULL },
												 { 11, 2500, 3200, NULL   }
											   };
	NlmRangeEncodingType encodingType[4] = { NLM_RANGE_3B_ENCODING,
											 NLM_RANGE_3B_ENCODING,
											 NLM_RANGE_3B_ENCODING,
											 NLM_RANGE_3B_ENCODING
											};

	nlm_u32  iter;
	nlm_u8  db_id;
	NlmRangeDbAttrSet srcRangeDbAttrs;
	NlmRangeDbAttrSet destRangeDbAttrs;
	NlmRangeSrchAttrs *rangeAttrs_p;
	nlm_u8 keyNum;

	/* Enable Range functionality by enabling Range Engine Enable Bit of to Device Config Register. */
	NlmCm__memset(&devConfigRegData, 0, sizeof(NlmDevConfigReg));
	/*
	 * For some unknown reason, once the range manager engine is enabled, 
	 * the LTR#0 searcj for 80B_0 key always return 0. Disable it here for now
	 * this will result in all LTR#2 search miss in the test.
	 */
	devConfigRegData.m_rangeEngineEnable = NLMDEV_ENABLE;
	devConfigRegData.m_dbParityErrEntryInvalidate = NLMDEV_ENABLE;

	if(NLMERR_OK !=NlmDevMgr__GlobalRegisterWrite(refAppData_p->dev_p,
												  NLMDEV_DEVICE_CONFIG_REG,
												  &devConfigRegData,
												  &reason))
	{
		NlmCmFile__printf("\tCould not Enable Range Engine...\n" );
		printReasonCode( reason );
		return RETURN_STATUS_ABORT;
	}											  
	
	/*Initialize the Range Manager */
	if( NULL == ( refAppData_p->rangeMgr_p = NlmRangeMgr__Init( refAppData_p->alloc_p,
																refAppData_p->devMgr_p,
																NLM_DEVTYPE_2,											   				    
											   				    encodingType,
																&reason
															  ) ) )
	{
		NlmCmFile__printf("\tCould not initialize Range Manager...\n" );
		printReasonCode( reason );

		return RETURN_STATUS_ABORT;
	}

	NlmCmFile__printf("\n\tRange Manager Initialized Successfully\n");

	/* Now create Range databases, one for source port and another one for destination
	 * port. Range Database has three attributes 
	 * (1) number of bits available for encoding (2) Number of valid bits of range (3) Encoding type
	 * Number of bits available for Range Manager for encoding is given by m_num_bits . 
	 * Number of valid range bits specifies how many (LSB)  bits of the 16b range field are valid; 	
	 * Encoding type specifies which range encoding to use
	 */
	srcRangeDbAttrs.m_num_bits = 32;
	srcRangeDbAttrs.m_valid_bits = 16;
       srcRangeDbAttrs.m_encodingType = NLM_RANGE_3B_ENCODING;

	db_id = 0; /* used for database id */
	if( NULL == ( refAppData_p->srcDb_p = NlmRangeMgr__CreateDb( refAppData_p->rangeMgr_p,
																 db_id,
																 &srcRangeDbAttrs,
																 &reason
															   ) ) )
	{
		NlmCmFile__printf("Could not create dabase-%d. Exiting...\n", db_id);
		printReasonCode( reason );

		return RETURN_STATUS_ABORT;
	}

	NlmCmFile__printf("\tSource Port Range Database [Database ID = %d] Created\n", db_id);

	destRangeDbAttrs.m_num_bits = 32;
	destRangeDbAttrs.m_valid_bits = 16;
       destRangeDbAttrs.m_encodingType = NLM_RANGE_3B_ENCODING;
	db_id++;
	if( NULL == ( refAppData_p->dstDb_p = NlmRangeMgr__CreateDb( refAppData_p->rangeMgr_p,
																 db_id,
																 &destRangeDbAttrs,
																 &reason
															   ) ) )
	{
		NlmCmFile__printf("Could not create dabase-%d. Exiting...\n", db_id);
		printReasonCode( reason );

		return RETURN_STATUS_ABORT;
	}

	NlmCmFile__printf("\tDestination Port Range Database [Database ID = %d] Created\n", db_id);

	/* Hardware has range blocks, Range A and Range B. Associate range databases
	 * with each of these range blocks. Source port database is mapped to Range A,
	 * Destination port database with Range B. 
	 */
	if( NLMERR_OK != NlmRangeMgr__AssignRange( refAppData_p->rangeMgr_p,
											   refAppData_p->srcDb_p,
											   NLM_RANGE_TYPE_A,
											   &reason ) )
	{
		NlmCmFile__printf("Source port database Range Assignment failed...\n");
		printReasonCode( reason );

		return RETURN_STATUS_FAIL;
	}

	NlmCmFile__printf("\tSource Port Database associated with Range-A H/W Block\n");

	if( NLMERR_OK != NlmRangeMgr__AssignRange( refAppData_p->rangeMgr_p,
											   refAppData_p->dstDb_p,
											   NLM_RANGE_TYPE_B,
											   &reason ) )
	{
		NlmCmFile__printf("Destination port database Range Assignment failed...\n");
		printReasonCode( reason );

		return RETURN_STATUS_FAIL;
	}

	NlmCmFile__printf("\tDestination Port Database associated with Range-B H/W Block\n");

	
   	/* Add source port database ranges first. Ranges are statically assigned */
	for( iter = 0; iter < NUM_OF_SRCPORT_DB_RANGES; iter++ )
	{
		refAppData_p->srcDb_ranges[ iter ] = srcDb_ranges[ iter ];
		if( NLMERR_OK != NlmRangeMgr__AddRange( refAppData_p->rangeMgr_p,
												refAppData_p->srcDb_p,
												&refAppData_p->srcDb_ranges[ iter ],
												&reason ) )
		{
			NlmCmFile__printf("Could not add range-%u to source port database...\n", iter);
			printReasonCode( reason );

			return RETURN_STATUS_FAIL;
		}
	}

	NlmCmFile__printf("\tSource Port Ranges added to Source Port Range Database\n");

	/* Now add ranges to destination port database */
	for( iter = 0; iter < NUM_OF_DSTPORT_DB_RANGES; iter++ )
	{
		refAppData_p->dstDb_ranges[ iter ] = dstDb_ranges[ iter ];
		if( NLMERR_OK != NlmRangeMgr__AddRange( refAppData_p->rangeMgr_p,
												refAppData_p->dstDb_p,
												&refAppData_p->dstDb_ranges[ iter ],
												&reason ) )
		{
			NlmCmFile__printf("Could not add range-%u to destination port database...\n", iter);
			printReasonCode( reason );

			return RETURN_STATUS_FAIL;
		}
	}

	NlmCmFile__printf("\tDestnation Port Ranges added to Destination Port Range Database\n");

	/* Let Range Manager do range compression now */
	if( NLMERR_OK != NlmRangeMgr__CreateEncodings( refAppData_p->rangeMgr_p,
												   refAppData_p->srcDb_p,
												   &reason ) )
	{
		NlmCmFile__printf("CreateEncodings failed on source port database...\n");
		printReasonCode( reason );

		return RETURN_STATUS_FAIL;
	}

	NlmCmFile__printf("\tCreateEncodings on Source Port Database Success\n");

	if( NLMERR_OK != NlmRangeMgr__CreateEncodings( refAppData_p->rangeMgr_p,
												   refAppData_p->dstDb_p,
												   &reason ) )
	{
		NlmCmFile__printf("CreateEncodings failed on destination port database...\n");
		printReasonCode( reason );

		return RETURN_STATUS_FAIL;
	}

	NlmCmFile__printf("\tCreateEncodings on Destination Port Database Success\n");

	
	

		

	/* Master key used for the search using LTR #2 will have source port range field value 
	 * at [31:16], destination port range field value at  at [111:96]. 
	 * It is assumed that [15:0] and [95:80] are spare bits which will be  used by device for 
	 * port encodings.
	*
	* Key#0 is generated from master key which is searched in this table. Generated key
	* will have encoded source port (Range A) value inserted at [31:0], encoded destination
	* port (Range B) value inserted at [111:80]
	*/
	rangeAttrs_p = &refAppData_p->range_attrs[2];

	rangeAttrs_p->m_extraction_startByte_rangeA = 2;
	rangeAttrs_p->m_extraction_startByte_rangeB = 12;
	rangeAttrs_p->m_rangeA_db = refAppData_p->srcDb_p;
	rangeAttrs_p->m_rangeB_db = refAppData_p->dstDb_p;
			
	/* Putting the initial insert values as DO NOT INSERT */
	for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
	{
		rangeAttrs_p->m_keyInsert_startByte_rangeA[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
		rangeAttrs_p->m_keyInsert_startByte_rangeB[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
		rangeAttrs_p->m_keyInsert_startByte_rangeC[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
		rangeAttrs_p->m_keyInsert_startByte_rangeD[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
	}
    
	/* Bits [31:0] of Key 0 will have Range A encodings while Bits[111:80] will have Range B encodings	*/
	rangeAttrs_p->m_keyInsert_startByte_rangeA[0] = 0;
	rangeAttrs_p->m_keyInsert_startByte_rangeB[0] = 10;		
          
	if( NLMERR_OK != NlmRangeMgr__ConfigRangeMatching(
				refAppData_p->rangeMgr_p,
				2,
				rangeAttrs_p,				
				&reason ) )
	{
		NlmCmFile__printf("\tLTR#2 ConfigRangeMatching failed...\n");
		printReasonCode( reason );
        
		return RETURN_STATUS_FAIL;
	}	
	
	return RETURN_STATUS_OK;
}

/* Destroys Range Manager instance and range databases */
static int DestroyRangeManager( genericTblMgrRefAppData *refAppData_p )
{
	NlmReasonCode   reason;

	/* Destroy range databases first and then the manager */
	NlmCmFile__printf("\n\tDestroying Source and Destination Databases\n");
	NlmRangeMgr__DestroyDb( refAppData_p->rangeMgr_p, refAppData_p->srcDb_p, &reason );
	NlmRangeMgr__DestroyDb( refAppData_p->rangeMgr_p, refAppData_p->dstDb_p, &reason );

	NlmCmFile__printf("\tDestroying Range Manager\n");
	NlmRangeMgr__Destroy( refAppData_p->rangeMgr_p, &reason );

	return RETURN_STATUS_OK;
}

/* This function performs Generic Table Manager related Inits */
static int InitGenericTableManager( genericTblMgrRefAppData *refAppData_p )
{
	NlmReasonCode	reason;
	nlm_u8			iter, num_tbls;

    /* Set the Generic Table Manager block range */
    refAppData_p->gtmBlkRange.m_startBlkNum = NLM_GTM_BLK_RANGE_WHOLE_DEVICE;

    /* if NLM_GTM_BLK_RANGE_WHOLE_DEVICE is specified then no need to 
    set refAppData_p->gtmBlkRange.m_endBlkNum */


	/* Call back function called by Cynapse when index for a record is changed */
	refAppData_p->indexChangeCB = IndexChangeCallBack;

	if( NULL == ( refAppData_p->genericTblMgr_p = NlmGenericTblMgr__Init(
														refAppData_p->alloc_p,
														refAppData_p->devMgr_p,
														NLM_DEVTYPE_2,														
                                                        NLM_REFAPP_NUM_OF_DEVICES,
                                                        &refAppData_p->gtmBlkRange,
														TABLE_ID_LEN,
                                                        refAppData_p->indexChangeCB,
														refAppData_p,
														&reason ) ) )
	{
		NlmCmFile__printf("\tGTM Init failed...\n");
		printReasonCode( reason );

		return RETURN_STATUS_ABORT;
	}

	NlmCmFile__printf("\n\tGeneric Table Manager Initialized Successfully\n");

	/* Create tables now, 6 tables are created */
	num_tbls = 0;

	/* Store table details for later use */
	NlmCm__strcpy( refAppData_p->tblInfo[ num_tbls ].tbl_id, TABLE_ID_80B_0 );
	refAppData_p->tblInfo[ num_tbls ].tbl_width     = TABLE_WIDTH_80B_0;
	refAppData_p->tblInfo[ num_tbls ].tbl_size      = TABLE_SIZE_80B_0;
	refAppData_p->tblInfo[ num_tbls ].groupId_start = START_GROUPID_80B_O;
	refAppData_p->tblInfo[ num_tbls ].groupId_end   = END_GROUPID_80B_O;
	num_tbls++;

	NlmCm__strcpy( refAppData_p->tblInfo[ num_tbls ].tbl_id, TABLE_ID_80B_1 );
	refAppData_p->tblInfo[ num_tbls ].tbl_width     = TABLE_WIDTH_80B_1;
	refAppData_p->tblInfo[ num_tbls ].tbl_size      = TABLE_SIZE_80B_1;
	refAppData_p->tblInfo[ num_tbls ].groupId_start = START_GROUPID_80B_1;
	refAppData_p->tblInfo[ num_tbls ].groupId_end   = END_GROUPID_80B_1;
	num_tbls++;

	NlmCm__strcpy( refAppData_p->tblInfo[ num_tbls ].tbl_id, TABLE_ID_160B_2 );
	refAppData_p->tblInfo[ num_tbls ].tbl_width     = TABLE_WIDTH_160B_2;
	refAppData_p->tblInfo[ num_tbls ].tbl_size      = TABLE_SIZE_160B_2;
	refAppData_p->tblInfo[ num_tbls ].groupId_start = START_GROUPID_160B_2;
	refAppData_p->tblInfo[ num_tbls ].groupId_end   = END_GROUPID_160B_2;
	num_tbls++;

	NlmCm__strcpy( refAppData_p->tblInfo[ num_tbls ].tbl_id, TABLE_ID_160B_3 );
	refAppData_p->tblInfo[ num_tbls ].tbl_width     = TABLE_WIDTH_160B_3;
	refAppData_p->tblInfo[ num_tbls ].tbl_size      = TABLE_SIZE_160B_3;
	refAppData_p->tblInfo[ num_tbls ].groupId_start = START_GROUPID_160B_3;
	refAppData_p->tblInfo[ num_tbls ].groupId_end   = END_GROUPID_160B_3;
	num_tbls++;

	NlmCm__strcpy( refAppData_p->tblInfo[ num_tbls ].tbl_id, TABLE_ID_320B_4 );
	refAppData_p->tblInfo[ num_tbls ].tbl_width     = TABLE_WIDTH_320B_4;
	refAppData_p->tblInfo[ num_tbls ].tbl_size      = TABLE_SIZE_320B_4;
	refAppData_p->tblInfo[ num_tbls ].groupId_start = START_GROUPID_320B_4;
	refAppData_p->tblInfo[ num_tbls ].groupId_end   = END_GROUPID_320B_4;
	num_tbls++;

	NlmCm__strcpy( refAppData_p->tblInfo[ num_tbls ].tbl_id, TABLE_ID_640B_5 );
	refAppData_p->tblInfo[ num_tbls ].tbl_width     = TABLE_WIDTH_640B_5;
	refAppData_p->tblInfo[ num_tbls ].tbl_size      = TABLE_SIZE_640B_5;
	refAppData_p->tblInfo[ num_tbls ].groupId_start = START_GROUPID_640B_5;
	refAppData_p->tblInfo[ num_tbls ].groupId_end   = END_GROUPID_640B_5;

	/*Call the CreateTable API to create each of the tables */
	for( iter = 0; iter <= num_tbls; iter++ )
	{
		if( NULL == ( refAppData_p->tbl_p[iter] = NlmGenericTblMgr__CreateTable(
                                                	refAppData_p->genericTblMgr_p,
                                                	refAppData_p->tblInfo[ iter].tbl_id,
                                                	refAppData_p->tblInfo[ iter].tbl_width,
                                                	refAppData_p->tblInfo[ iter ].tbl_size,
                                                	&reason ) ) )
    	{
        	NlmCmFile__printf("\tTable [%s] creation failed...\n",
								refAppData_p->tblInfo[ iter ].tbl_id);
			printReasonCode( reason );

        	return RETURN_STATUS_FAIL;
    	}
		else
		{
			NlmCmFile__printf("\tTable [%s] creation succeded...\n",
								refAppData_p->tblInfo[ iter ].tbl_id);
		}
	}


	
	 /* Configure searches now */
	 {
	 	NlmGenericTblParallelSearchInfo  *psInfo_p = NULL;
		nlm_u8 ltr_id = 0;


		/* This search uses LTR#0, 80B_0, 80B_1 and 160B_2 are searched in parallel
		 * The search attributes are ( Compare1 ):
		 * Key#0 is searched in table with table id TABLE_ID_80B_0 giving out the result 
		 * onto result port number 0. Starting from byte 0 of the Master Key, 10 bytes are 
		 * picked to form Key#0
		 * Key#1 is searched in table with table id TABLE_ID_80B_1 giving out the result 
		 * onto result port number 1. Starting from byte 10 of the Master Key, 10 bytes are 
		 * picked to form Key#1
		 * Key#2 is searched in table with table id TABLE_ID_160B_2 giving out the result 
		 * onto result port number 2. Starting from byte 20 of the Master Key, 20 bytes are 
		 * picked to form Key#2
		 */
		refAppData_p->ltr_num[0] = 0;
		refAppData_p->search_attrs[ 0 ].m_numOfParallelSrches  = 3;
		
		psInfo_p = &refAppData_p->search_attrs[ 0 ].m_psInfo[0];
		psInfo_p->m_tblId_p = TABLE_ID_80B_0;
		psInfo_p->m_rsltPortNum = 0;
		psInfo_p->m_keyNum = 0;
		psInfo_p->m_kcm.m_segmentStartByte[0] = 0;
		psInfo_p->m_kcm.m_segmentNumOfBytes[0] = 10;

		psInfo_p = &refAppData_p->search_attrs[ 0 ].m_psInfo[1];
		psInfo_p->m_tblId_p = TABLE_ID_80B_1;
		psInfo_p->m_rsltPortNum = 1;
		psInfo_p->m_keyNum = 1;
		psInfo_p->m_kcm.m_segmentStartByte[0] = 10;
		psInfo_p->m_kcm.m_segmentNumOfBytes[0] = 10;

		psInfo_p = &refAppData_p->search_attrs[0].m_psInfo[2];
		psInfo_p->m_tblId_p = TABLE_ID_160B_2;
		psInfo_p->m_rsltPortNum = 2;
		psInfo_p->m_keyNum = 2;
		psInfo_p->m_kcm.m_segmentStartByte[0] = 20;
		psInfo_p->m_kcm.m_segmentNumOfBytes[0] = 16;
		psInfo_p->m_kcm.m_segmentStartByte[1] = 36;
		psInfo_p->m_kcm.m_segmentNumOfBytes[1] = 4;


		
		
		/* This search uses LTR#1, 160B_2 and 160B_3 are searched in parallel
		 * The search attributes are ( Compare1 ):
		 * Key#0 is searched in table with table id TABLE_ID_160B_2 giving out the result 
		 * onto result port number 0. Starting from byte 0 of the Master Key, 20 bytes are 
		 * picked to form Key#0
		 * Key#1 is searched in table with table id TABLE_ID_160B_3 giving out the result 
		 * onto result port number 1. Starting from byte 20 of the Master Key, 20 bytes are 
		 * picked to form Key#1
		 */
		 refAppData_p->ltr_num[1] = 1;
		 refAppData_p->search_attrs[ 1 ].m_numOfParallelSrches  = 2;

		 psInfo_p = &refAppData_p->search_attrs[ 1 ].m_psInfo[0];
		 psInfo_p->m_tblId_p = TABLE_ID_160B_2;
		 psInfo_p->m_rsltPortNum = 0;
		 psInfo_p->m_keyNum = 0;
		 psInfo_p->m_kcm.m_segmentStartByte[0] = 0;
		 psInfo_p->m_kcm.m_segmentNumOfBytes[0] = 16;
		 psInfo_p->m_kcm.m_segmentStartByte[1] = 16;
		 psInfo_p->m_kcm.m_segmentNumOfBytes[1] = 4;


		 psInfo_p = &refAppData_p->search_attrs[ 1 ].m_psInfo[1];
		 psInfo_p->m_tblId_p = TABLE_ID_160B_3;
		 psInfo_p->m_rsltPortNum = 1;
		 psInfo_p->m_keyNum = 1;
		 psInfo_p->m_kcm.m_segmentStartByte[0] = 20;
		 psInfo_p->m_kcm.m_segmentNumOfBytes[0] = 16;
		 psInfo_p->m_kcm.m_segmentStartByte[1] = 36;
		 psInfo_p->m_kcm.m_segmentNumOfBytes[1] = 4;


		
		/* This search uses LTR#2, 320B_4 is searched. This search has ranges
		 * The search attributes are ( Compare1 ):
		 * Key#0 is searched in table with table id TABLE_ID_320B_4 giving out the result 
		 * onto result port number 0. Starting from byte 0 of the Master Key, 40 bytes are 
		 * picked to form Key#0
		 */
		 refAppData_p->ltr_num[2] = 2;
		 refAppData_p->search_attrs[ 2 ].m_numOfParallelSrches  = 1;

		 psInfo_p = &refAppData_p->search_attrs[ 2 ].m_psInfo[0];
		 psInfo_p->m_tblId_p = TABLE_ID_320B_4;
		 psInfo_p->m_rsltPortNum = 0;
		 psInfo_p->m_keyNum = 0;
		 psInfo_p->m_kcm.m_segmentStartByte[0] = 0;
		 psInfo_p->m_kcm.m_segmentNumOfBytes[0] = 16;
		 psInfo_p->m_kcm.m_segmentStartByte[1] = 16;
		 psInfo_p->m_kcm.m_segmentNumOfBytes[1] = 16;
		 psInfo_p->m_kcm.m_segmentStartByte[2] = 32;
		 psInfo_p->m_kcm.m_segmentNumOfBytes[2] = 8;

		/* This search uses LTR#3, only 640B_5 is searched
		 * The search attributes are ( Compare2 ):
		 * Key#0 is searched in table with table id TABLE_ID_640B_5 giving out the result 
		 * onto result port number 0. Starting from byte 0 of the Master Key, 80 bytes are 
		 * picked to form Key#0
		 */
		 refAppData_p->ltr_num[3] = 3;
		 refAppData_p->search_attrs[ 3 ].m_numOfParallelSrches  = 1;

		 psInfo_p = &refAppData_p->search_attrs[ 3 ].m_psInfo[0];
		 psInfo_p->m_tblId_p = TABLE_ID_640B_5;
		 psInfo_p->m_rsltPortNum = 0;
		 psInfo_p->m_keyNum = 0;
		 psInfo_p->m_kcm.m_segmentStartByte[0] = 0;
		 psInfo_p->m_kcm.m_segmentNumOfBytes[0] = 16;
		 psInfo_p->m_kcm.m_segmentStartByte[1] = 16;
		 psInfo_p->m_kcm.m_segmentNumOfBytes[1] = 16;
		 psInfo_p->m_kcm.m_segmentStartByte[2] = 32;
		 psInfo_p->m_kcm.m_segmentNumOfBytes[2] = 16;
		 psInfo_p->m_kcm.m_segmentStartByte[3] = 48;
		 psInfo_p->m_kcm.m_segmentNumOfBytes[3] = 16;
		 psInfo_p->m_kcm.m_segmentStartByte[4] = 64;
		 psInfo_p->m_kcm.m_segmentNumOfBytes[4] = 16;


	    /*Call the ConfigSearch API to configure each of the LTRs */
		for( ltr_id = 0; ltr_id < NUM_SEARCHES; ltr_id++ )
		{
			
	 		if( NLMERR_OK != NlmGenericTblMgr__ConfigSearch(
	 									refAppData_p->genericTblMgr_p,
										ltr_id,
										&refAppData_p->search_attrs[ltr_id],
										&reason ) )
			{
				NlmCmFile__printf("\tLTR#%d ConfigSearch  failed...\n",
										ltr_id); 
				printReasonCode( reason );

				return RETURN_STATUS_FAIL;
			}
			else
			{
				NlmCmFile__printf("\tLTR#%d ConfigSearch  passed...\n",
										ltr_id); 

			}

		}     
		
	}         /* end of configurations */

	/* Configuration is finished, Lock it now */
	if( NLMERR_OK != NlmGenericTblMgr__LockConfiguration(
									refAppData_p->genericTblMgr_p,
									&reason ) )
	{
		NlmCmFile__printf("\tGTM LockConfig failed...\n"); 
		printReasonCode( reason );

		return RETURN_STATUS_FAIL;
	}

	NlmCmFile__printf("\tGTM Configuration Locked\n\n");

	/* Create records to be inserted into various tables. */
	CreateRecordsInTables( refAppData_p );

	return RETURN_STATUS_OK;

}


/* Destroy generic tables and manager instance */
static int DestroyGenericTableManager( genericTblMgrRefAppData *refAppData_p )
{
	tableInfo *tblInfo_p;
	tableRecordInfo	*tblRecordInfo_p;
	nlm_u32 iter_tbl, iter_rec, num_recs;
	NlmReasonCode   reason = NLMRSC_REASON_OK;

	/* First free memory allocated for keeping records within Ref App. Next step is to 
	 * destroy tables and finally calling destroy table manager.
	 */
	for( iter_tbl = 0; iter_tbl < NUM_OF_TABLES; iter_tbl++ )
	{
		tblInfo_p = &refAppData_p->tblInfo[ iter_tbl ];
		tblRecordInfo_p = tblInfo_p->tblRecordInfo_p;

		num_recs = tblInfo_p->max_recCount;
		for( iter_rec = 0; iter_rec < num_recs; tblRecordInfo_p++, iter_rec++ )
		{
			NlmCmAllocator__free( refAppData_p->alloc_p, tblRecordInfo_p->record.m_data );
			NlmCmAllocator__free( refAppData_p->alloc_p, tblRecordInfo_p->record.m_mask );
		}

		/* tblRecordInfo is calloced, free that space */
		NlmCmAllocator__free( refAppData_p->alloc_p, tblInfo_p->tblRecordInfo_p );

		/* And destroy the table */
		NlmCmFile__printf("\n\tDestroying table#%u\n", iter_tbl);
		if( NLMERR_OK != NlmGenericTblMgr__DestroyTable( refAppData_p->genericTblMgr_p,
														 refAppData_p->tbl_p[ iter_tbl ],
														 &reason ) )
		{
        	NlmCmFile__printf("\tDestroyTable#%u failed...\n", iter_tbl);
			printReasonCode( reason );
		}
	}

	/* Time to destroy the manager now. */
	NlmCmFile__printf("\n\tDestroying Generic Table Manager\n");
	if( NLMERR_OK != NlmGenericTblMgr__Destroy( refAppData_p->genericTblMgr_p, &reason ) )
	{
		NlmCmFile__printf("\tGeneric Table Manager destroy failed...\n");
		printReasonCode( reason );

		return NLMERR_FAIL;
	}

	return RETURN_STATUS_OK;
}

static int StitchRangesWithin_320B4_Records( genericTblMgrRefAppData *refAppData_p )
{
	tableInfo         *tblInfo_p;
    tableRecordInfo   *tblRecordInfo_p;
    NlmRangeEncoded     *srcPort_encodings, *dstPort_encodings;
    NlmReasonCode   reason = NLMRSC_REASON_OK;
    nlm_u32 iter_rec, num_recs, srcRange_id, dstRange_id;

    /* This sample application uses <0, 1, 5> from source port ranges,
     * <0, 1, 5> from destination port ranges. A single ACL entry
     * which contains both these ranges is considered. Number of expanded ACL
     * entries will be multiplication of <number of encoded bitmaps of the source port>
     * and <number of encoded bitmaps of the destination port>.
     * To make the insertions simple, both ranges considered are popular ranges so that
     * number of expanded ACL entries is 1.
     */

    srcRange_id = 0;
    dstRange_id = 0;

    if( NULL == ( srcPort_encodings = NlmRangeMgr__GetRangeEncoding( refAppData_p->rangeMgr_p,
                                             refAppData_p->srcDb_p, srcRange_id, &reason ) ) )
    {
        NlmCmFile__printf("Could not get encodings for source port = %u. Exiting...\n", srcRange_id);
		printReasonCode( reason );

        return NLMERR_FAIL;
    }

    if( NULL == ( dstPort_encodings = NlmRangeMgr__GetRangeEncoding( refAppData_p->rangeMgr_p,
                                             refAppData_p->dstDb_p, dstRange_id, &reason ) ) )
    {
        NlmCmFile__printf("Could not get encodings for source port = %u. Exiting...\n", srcRange_id);
		printReasonCode( reason );

        return NLMERR_FAIL;
    }

    /* Record pattern is as follows:
	 * 4.1.1.1.1__X.X.X.X.X__2.2.2.2.2__X.X.X.X.X__
	 * 3.3.3.3.3__X.<DPE>__0.0.0.0.0__X.<SPE>,
	 * 4.1.1.1.1__X.X.X.X.X__2.2.2.2.2__X.X.X.X.X__
	 * 3.3.3.3.3__X.<DPE>__0.0.0.0.1__X.<SPE>
 	 * and so on
	 * DPE = 32b Destination Port Encodings
	 * SPE = 32b Source Port Encodings
     */
	tblInfo_p = &refAppData_p->tblInfo[ 4 ];
	tblRecordInfo_p = tblInfo_p->tblRecordInfo_p;
	num_recs  = tblInfo_p->max_recCount;

    for( iter_rec = 0; iter_rec < num_recs; tblRecordInfo_p++, iter_rec++ )
    {
        /* stitch destination port encoding at [111:80] now. Following function expects
         * 80 bit out param; [111:80] in 80 bit boundary will be [31,0]
         */
        WriteBitsInRegs( &tblRecordInfo_p->record.m_data[ 20 ], 31, 0,
                            dstPort_encodings->m_entries_p->m_data );
        WriteBitsInRegs( &tblRecordInfo_p->record.m_mask[ 20 ], 31, 0,
                            dstPort_encodings->m_entries_p->m_mask );

        /* stitch source port encoding at [31:0] now */
        WriteBitsInRegs( &tblRecordInfo_p->record.m_data[ 30 ], 31, 0,
                            srcPort_encodings->m_entries_p->m_data );
        WriteBitsInRegs( &tblRecordInfo_p->record.m_mask[ 30 ], 31, 0,
                            srcPort_encodings->m_entries_p->m_mask );
    } /* end of for loop */

    return RETURN_STATUS_OK;
}

/* This function fills the internal data strcture for tables with records */
static void CreateRecordsInTables( genericTblMgrRefAppData *refAppData_p )
{
	tableInfo *tblInfo_p;
	tableRecordInfo *tblRecordInfo_p;
	nlm_u32 alloc_size, iter_rec;
	nlm_u8  iter_tbl, tbl_id, tblWidth_inBytes, start_byte = 0;
	nlm_u8  *rec_data, *rec_mask, *var_data_p;
	nlm_u16 	start, end, iter_group, iter_priority;

	/* First allocate memory for storing records within the data structures.
	 * Each table structure has start groupId and end groupId. Records of
	 * all groupIds including the start-end groupId are added. With each
	 * groupId, priorities of the records would range from [0 TO groupId-1].
	 * For e.g. if the start-end groupId are 5-7, then 5 records with priorities
	 * ranging from 0-4 are created, 6 records with priorities from 0-5 are
	 * created and so on. Hence the formula to calculate the table size is as
	 * as follows:
	 * n(n+1)/2 - m(m-1)/2 ; given the start-end groupIds being m-n
	 */
	 for( iter_tbl = 0; iter_tbl < NUM_OF_TABLES; iter_tbl++ )
	 {
		tblInfo_p = &refAppData_p->tblInfo[ iter_tbl ];
	 	start     = tblInfo_p->groupId_start;
	 	end       = tblInfo_p->groupId_end;

		alloc_size = ( ( end * (end+1) ) - ( start * (start-1) ) ) / 2;

		tblInfo_p->tblRecordInfo_p = NlmCmAllocator__calloc( refAppData_p->alloc_p,
															 alloc_size,
															 sizeof( tableRecordInfo ) );
		tblInfo_p->max_recCount    = alloc_size;

		/* MSB byte of each record has table id, followed by the actual data
	 	 * There are 6 tables: 80B_0, 80B_1, 160B_2, 160B_3, 320B_4 and 640B_5.
	 	 *
	 	 * Record pattern is as follows:
	 	 * In the following text "X" means a dont care of 8 bits
	 	 *
	 	 * 80B_0: 0.X.X.X.X__0.0.0.0.0, 0.X.X.X.X__0.0.0.0.1,
	 	 *        0.X.X.X.X__0.0.0.0.2 and so on;
	 	 * by 1.
	 	 *
	 	 * 80B_1: 1.0.0.0.0__X.X.X.X.X, 1.0.0.0.1__X.X.X.X.X,
	 	 *        1.0.0.0.2__X.X.X.X.X and so on;
	 	 * by 1.
	 	 *
	 	 * 160B_2: 2.9.9.9.9__10.10.10.10.10__X.X.X.X.X__0.0.0.0.0,
	 	 *         2.9.9.9.9__10.10.10.10.10__X.X.X.X.X__0.0.0.0.1,
	 	 *         and so on
	 	 *
	 	 * 160B_3: 3.9.9.9.9__10.10.10.10.10__0.0.0.0.0__X.X.X.X.X,
	 	 *         3.9.9.9.9__10.10.10.10.10__0.0.0.0.1__X.X.X.X.X
	 	 *         and so on
	 	 *
	 	 * 320B_4: 4.1.1.1.1__X.X.X.X.X__2.2.2.2.2__X.X.X.X.X__
	 	 *         3.3.3.3.3__X.<DPE>__0.0.0.0.0__X.<SPE>,
		 *         4.1.1.1.1__X.X.X.X.X__2.2.2.2.2__X.X.X.X.X__
		 *         3.3.3.3.3__X.<DPE>__0.0.0.0.1__X.<SPE>
	 	 *         and so on
		 *         DPE = 32b Destination Port Encodings
		 *         SPE = 32b Source Port Encodings
	 	 *
	 	 * 640b_5: 5.1.1.1.1__X.X.X.X.X__2.2.2.2.2__X.X.X.X.X__
	 	 *         3.3.3.3.3__X.X.X.X.X__4.4.4.4.4__X.X.X.X.X__
	 	 *         5.5.5.5.5__X.X.X.X.X__6.6.6.6.6__X.X.X.X.X__
	 	 *         7.7.7.7.7__X.X.X.X.X__8.8.8.8.8__0.0.0.0.0,
		 *         5.1.1.1.1__X.X.X.X.X__2.2.2.2.2__X.X.X.X.X__
		 *         3.3.3.3.3__X.X.X.X.X__4.4.4.4.4__X.X.X.X.X__
		 *         5.5.5.5.5__X.X.X.X.X__6.6.6.6.6__X.X.X.X.X__
		 *         7.7.7.7.7__X.X.X.X.X__8.8.8.8.8__0.0.0.0.1 
	 	 *         and so on
	 	 */
	 	tblWidth_inBytes = (nlm_u8)(tblInfo_p->tbl_width / 8);
		rec_data = NlmCmAllocator__calloc( refAppData_p->alloc_p,
	 										1, tblWidth_inBytes );
		rec_mask = NlmCmAllocator__calloc( refAppData_p->alloc_p,
	 										1, tblWidth_inBytes );

		/* get the starting record pattern based on the table id */
		tbl_id = GetTableId( tblInfo_p->tbl_id );
		switch( tbl_id )
		{
			case 0:
			{
				start_byte = FillRecordPatternFor_80B0_Table( rec_data, rec_mask );
				break;
			}
			case 1:
			{
				start_byte = FillRecordPatternFor_80B1_Table( rec_data, rec_mask );
				break;
			}
			case 2:
			{
				start_byte = FillRecordPatternFor_160B2_Table( rec_data, rec_mask );
				break;
			}
			case 3:
			{
				start_byte = FillRecordPatternFor_160B3_Table( rec_data, rec_mask );
				break;
			}
			case 4:
			{
				start_byte = FillRecordPatternFor_320B4_Table( rec_data, rec_mask );
				break;
			}
			case 5:
			{
				start_byte = FillRecordPatternFor_640B5_Table( rec_data, rec_mask );
				break;
			}
			default:
				break;
		}

		/* 4 bytes starting from this byte location are incremented. These 4 bytes are
		 * the only variable data portion across all records.
		 */
		var_data_p = rec_data + start_byte;

		/* Start filling the records now */
		tblRecordInfo_p = tblInfo_p->tblRecordInfo_p;
		for( iter_group = start, iter_rec = 0; iter_group <= end; iter_group++ )
		{
			for( iter_priority = 0; iter_priority < iter_group; iter_priority++ )
			{
				WriteValueToBitVector4( var_data_p, iter_rec );

				tblRecordInfo_p->groupId  = iter_group;
				tblRecordInfo_p->priority = iter_priority;

				tblRecordInfo_p->record.m_data = NlmCmAllocator__calloc( refAppData_p->alloc_p,
																		 1, tblWidth_inBytes );
				tblRecordInfo_p->record.m_mask = NlmCmAllocator__calloc( refAppData_p->alloc_p,
																		 1, tblWidth_inBytes );
				tblRecordInfo_p->record.m_len  = tblInfo_p->tbl_width;

				NlmCm__memcpy( tblRecordInfo_p->record.m_data, rec_data, tblWidth_inBytes );
				NlmCm__memcpy( tblRecordInfo_p->record.m_mask, rec_mask, tblWidth_inBytes );

				/* Dont worry, we wont over index. */
				tblRecordInfo_p++;
				iter_rec++;
			}
		}

		/* 320B_4 table has ranges. Stitch the ranges within records */
		if( tbl_id == 4 )
		{
			StitchRangesWithin_320B4_Records( refAppData_p );
		}

		/* Free the memory allocated for temp data and mask. A fresh memory will be
		 * based on the table width
		 */
		NlmCmAllocator__free( refAppData_p->alloc_p, rec_data );
		NlmCmAllocator__free( refAppData_p->alloc_p, rec_mask );
	 } /* end of table's loop */
}


/* Adds records into various tables */
static int AddRecordsToTables(
				genericTblMgrRefAppData *refAppData_p,
				NlmGenericTbl *tbl_p,
				nlm_u8 flag )
{
	tableInfo *tblInfo_p;
	tableRecordInfo	*tblRecordInfo_p;
	NlmReasonCode   reason = NLMRSC_REASON_OK;
	nlm_u32 iter_rec, num_recs;
	nlm_u8  tbl_id;

	tbl_id = GetTableId( tbl_p->m_id_str_p );
	tblInfo_p = &refAppData_p->tblInfo[ tbl_id ];
	tblRecordInfo_p = tblInfo_p->tblRecordInfo_p;
	num_recs  = tblInfo_p->max_recCount;

	/* flag decides whether to add records# 0,2,4 and so on OR 1,3,5 and so on */
	iter_rec = 0;
	if( flag )
	{
		tblRecordInfo_p++;
		iter_rec = 1;
	}

	NlmCmFile__printf("\n\tAdding records into table [%s]\n", tbl_p->m_id_str_p);
	for( ; iter_rec < num_recs; iter_rec = iter_rec + 2 )
	{
		/* Add record now */
		if( NLMERR_OK != NlmGenericTblMgr__AddRecord( tbl_p,
													  &tblRecordInfo_p->record,
													  tblRecordInfo_p->groupId,
													  tblRecordInfo_p->priority,
													  &tblRecordInfo_p->index,
													  &reason ) )
		{
        	NlmCmFile__printf("\ttable [%s] insertions: could not add record#%u\n",
								tbl_p->m_id_str_p, tblInfo_p->rec_count); 
			printReasonCode( reason );

        	return NLMERR_FAIL;
		}

		/* Advance the record pointer by two */
		tblRecordInfo_p += 2;
		tblInfo_p->rec_count++;
		if( 0 == (tblInfo_p->rec_count % DEBUG_PRINT_ITER_VAL) )
		{
			NlmCmFile__printf("\tNumber of records added [%u]\n", tblInfo_p->rec_count);
		}

	} /* end of for loop */

	NlmCmFile__printf("\tNumber of records added [%u]\n", tblInfo_p->rec_count);

	return RETURN_STATUS_OK;
}


/* Compare1, three tables 80B_0, 80B_1 and 160B_2 are searched in parallel */
static int Perform_LTR0_Searches( genericTblMgrRefAppData *refAppData_p )
{
	tableInfo   *tblInfo0_p, *tblInfo1_p, *tblInfo2_p;
    tableRecordInfo   *tblRecordInfo0_p, *tblRecordInfo1_p, *tblRecordInfo2_p;
	NlmGenericTblRecord *tblRecord0_p, *tblRecord1_p,*tblRecord2_p;
    NlmReasonCode   reason = NLMRSC_REASON_OK;
	nlm_u32 iter_tbl0, iter_tbl1, iter_tbl2, numRecs_tbl0, numRecs_tbl1, numRecs_tbl2, iter;
	nlm_u32 exp_index0, exp_index1, exp_index2;
	nlm_u32 search_index0, search_index1, search_index2;
	nlm_u8  ltr_num;

	/* Device Manager declarations */
	NlmDevCtxBufferInfo      cb_info;
	NlmDevCmpRslt     search_result;
	NlmDevMissHit search_result0, search_result1, search_result2;

	NlmCmFile__printf("\n\tPerforming LTR#0 searches\n");

	/* Three tables 80B_0, 80B_1 and 160B_2 are searched in parallel */
	tblInfo0_p = &refAppData_p->tblInfo[ 0 ];
	tblInfo1_p = &refAppData_p->tblInfo[ 1 ];
	tblInfo2_p = &refAppData_p->tblInfo[ 2 ];

	numRecs_tbl0 = tblInfo0_p->rec_count;
	numRecs_tbl1 = tblInfo1_p->rec_count;
	numRecs_tbl2 = tblInfo2_p->rec_count;

	tblRecordInfo0_p = tblInfo0_p->tblRecordInfo_p;
	tblRecordInfo1_p = tblInfo1_p->tblRecordInfo_p;
	tblRecordInfo2_p = tblInfo2_p->tblRecordInfo_p;

	/* This search uses LTR#0, 80B_0, 80B_1 and 160B_2 are searched in parallel
	 * The search attributes are ( Compare1 ):
	 * Key#0 is searched in table with table id TABLE_ID_80B_0 giving out  
     * the result onto result port number 0. Key#0 is formed from Byte 9 to Byte 0 
	 * of the Master Key
	 * Key#1 is searched in table with table id TABLE_ID_80B_1 giving out  
     * the result onto result port number 1. Key#1 is formed from Byte 19 to Byte 10 
	 * of the Master Key
	 * Key#2 is searched in table with table id TABLE_ID_160B_2 giving out  
     * the result onto result port number 2. Key#2 is formed from Byte 39 to Byte 20 
	 * of the Master Key
	 */
	
	/* Construct Master Key now for Compare1 operation.
	 * 160b record from 160B_2 table is first written into the Context Buffer(CB) location 2 
	 * and 3
	 * On issuing a compare instruction, the 80b record from table 80B_0 is written into
	 * CB location 0 and 80b record from table 80B_1 is written into CB location 1
	 *
	 * The master key is formed from CB locations 3 to 0. 
	 * MasterKey[319:160] has 160B_2 record,
	 * MasterKey[159:80] has 80B_1 record 
	 * MasterKey[79:0] has 80B_0 record 
     */
	ltr_num = 0;
	iter = 0;
	for( iter_tbl2 = 0; iter_tbl2 < numRecs_tbl2; iter_tbl2++ )
	{
		for( iter_tbl1 = 0; iter_tbl1< numRecs_tbl1; iter_tbl1++ )
		{
			exp_index2 = (tblRecordInfo2_p + iter_tbl2)->index;
			tblRecord2_p = &( (tblRecordInfo2_p + iter_tbl2)->record );
			NlmCm__memcpy( cb_info.m_data, tblRecord2_p->m_data, (tblRecord2_p->m_len / 8) );

			exp_index1 = (tblRecordInfo1_p + iter_tbl1)->index;
			/* MasterKey[159:80] are constructed from table1 records */
			tblRecord1_p = &( (tblRecordInfo1_p + iter_tbl1)->record );
			NlmCm__memcpy( &cb_info.m_data[20], tblRecord1_p->m_data, (tblRecord1_p->m_len / 8) );

			/* MasterKey[79:0] are constructed from table0 records */
			for( iter_tbl0 = 0; iter_tbl0< numRecs_tbl0; iter_tbl0++ )
			{
				exp_index0 = (tblRecordInfo0_p + iter_tbl0)->index;
				tblRecord0_p = &( (tblRecordInfo0_p + iter_tbl0)->record );
				NlmCm__memcpy( &cb_info.m_data[ 30 ], tblRecord0_p->m_data,
									(tblRecord0_p->m_len / 8) );

				iter++;
				cb_info.m_datalen = 40;
				cb_info.m_cbStartAddr = 0;
				/* Fire Compare1 instruction now */
    			if( NLMERR_OK != NlmDevMgr__Compare1( refAppData_p->devMgr_p, ltr_num,
														&cb_info, &search_result, &reason ) )
				{
        			NlmCmFile__printf("Compare1 Instruction failed. Exiting...\n");
					printReasonCode( reason );

        			return NLMERR_FAIL;
    			}
    			else
				{
					search_result0 = search_result.m_hitOrMiss[ 0 ]; 
					search_result1 = search_result.m_hitOrMiss[ 1 ]; 
					search_result2 = search_result.m_hitOrMiss[ 2 ];

					search_index0  = search_result.m_hitIndex[ 0 ];
					search_index1  = search_result.m_hitIndex[ 1 ];
					search_index2  = search_result.m_hitIndex[ 2 ];

					/* Hit is expected on all the parallel searches */
					if( search_result0 == NLMDEV_MISS ||
						search_result1 == NLMDEV_MISS ||
						search_result2 == NLMDEV_MISS  )
					{
						NlmCmFile__printf("Got Miss with MasterKey#%u\n", iter);
						NlmCmFile__printf("search_result0 = %s\n", 
											search_result0 ? "Hit" : "Miss");
						NlmCmFile__printf("search_result1 = %s\n", 
											search_result1 ? "Hit" : "Miss");
						NlmCmFile__printf("search_result2 = %s\n", 
											search_result2? "Hit" : "Miss");
					}
					else
					{
						/* Check whether the indexes returned by search or correct or not? */
						if( exp_index0 != search_index0 ||
							exp_index1 != search_index1 ||
							exp_index2 != search_index2  )
						{
							NlmCmFile__printf("Search failed with MasterKey#%u\n", iter);
							NlmCmFile__printf("exp_index0 = %u, search_index0 = %u\n",
												exp_index0, search_index0);
							NlmCmFile__printf("exp_index1 = %u, search_index1 = %u\n",
												exp_index1, search_index1);
							NlmCmFile__printf("exp_index2 = %u, search_index2 = %u\n",
												exp_index2, search_index2);
						}
					}
        		}

				if( 0 == (iter % DEBUG_PRINT_ITER_VAL) )
				{
					NlmCmFile__printf("\r\t   Number of keys searched so far [%u]", iter);
				}

			} /* tbl0 loop */
		}     /* tbl1 loop */
	}         /* tbl2 loop */

	NlmCmFile__printf("\n\tNumber of keys searched [%u]\n", iter);
	return RETURN_STATUS_OK;
}


/* Compare1, two tables 160B_2 and 160B_3 searched in parallel */
static int Perform_LTR1_Searches( genericTblMgrRefAppData *refAppData_p )
{
	tableInfo   *tblInfo2_p, *tblInfo3_p;
    tableRecordInfo   *tblRecordInfo2_p, *tblRecordInfo3_p;
	NlmGenericTblRecord *tblRecord2_p, *tblRecord3_p;
    NlmReasonCode   reason = NLMRSC_REASON_OK;
	nlm_u32 iter_tbl2, iter_tbl3, numRecs_tbl2, numRecs_tbl3, iter;
	nlm_u32 exp_index2, exp_index3;
	nlm_u32 search_index2, search_index3;
	nlm_u8  ltr_num;

	/* Device Manager declarations */
	NlmDevCtxBufferInfo      cb_info;
	NlmDevCmpRslt     search_result;
	NlmDevMissHit search_result2, search_result3;

	NlmCmFile__printf("\n\tPerforming LTR#1 searches\n");

	/* Two tables 160B_2 and 160B_3 are searched in parallel */
	tblInfo2_p = &refAppData_p->tblInfo[ 2 ];
	tblInfo3_p = &refAppData_p->tblInfo[ 3 ];

	numRecs_tbl2= tblInfo2_p->rec_count;
	numRecs_tbl3 = tblInfo3_p->rec_count;

	tblRecordInfo2_p = tblInfo2_p->tblRecordInfo_p;
	tblRecordInfo3_p = tblInfo3_p->tblRecordInfo_p;

	/* This search uses LTR#1, 160B_2 and 160B_3 are searched in parallel
	  * The search attributes are ( Compare1 ):
	  * Key#0 is searched in table with table id TABLE_ID_160B_2 giving out  
      * the result onto result port number 0. Key#0 is formed from Byte 19 to Byte 0 
	  * of the Master Key
	  * Key#1 is searched in table with table id TABLE_ID_160B_3 giving out  
      * the result onto result port number 1. Key#1 is formed from Byte 39 to Byte 20 
	  * of the Master Key
	  */

	/* Construct Master Key now for Compare1 operation.
	 * 160b record from 160B_3 table is first written into the Context Buffer(CB) location 2 
	 * and 3
	 * On issuing a compare instruction, the 160b record from table 160B_2 is written into
	 * CB location 0 and 1. 
	 * The master key is formed from CB locations 3 to 0. 
	 * MasterKey[319:160] has 160B_3 record,
	 * MasterKey[159:0] has 160B_2 record  
     */   
	ltr_num = 1;
	iter = 0;
	for( iter_tbl3 = 0; iter_tbl3 < numRecs_tbl3; iter_tbl3++ )
	{
		for( iter_tbl2 = 0; iter_tbl2 < numRecs_tbl2; iter_tbl2++ )
		{
			exp_index3 = (tblRecordInfo3_p + iter_tbl3)->index;
			tblRecord3_p = &( (tblRecordInfo3_p + iter_tbl3)->record );
			NlmCm__memcpy( cb_info.m_data, tblRecord3_p->m_data,  (tblRecord3_p->m_len / 8));
			
			exp_index2= (tblRecordInfo2_p + iter_tbl2)->index;
			/* MasterKey[159:0] are constructed from table2 records */
			tblRecord2_p = &( (tblRecordInfo2_p + iter_tbl2)->record );
			NlmCm__memcpy( &cb_info.m_data[20], tblRecord2_p->m_data, (tblRecord2_p->m_len / 8) );
			cb_info.m_datalen = 40; /* length of the data to be written is 40 bytes */
			cb_info.m_cbStartAddr = 0; /* CB location where data should be written */
			
			iter++;
			/* Fire Compare1 instruction now */
    		if( NLMERR_OK != NlmDevMgr__Compare1( refAppData_p->devMgr_p, ltr_num,
														&cb_info, &search_result, &reason ) )
			{
        		NlmCmFile__printf("Compare1 Instruction failed. Exiting...\n");
				printReasonCode( reason );

        		return NLMERR_FAIL;
    		}
    		else
			{
				search_result2 = search_result.m_hitOrMiss[ 0 ]; 
				search_result3 = search_result.m_hitOrMiss[ 1 ]; 

				search_index2  = search_result.m_hitIndex[ 0 ];
				search_index3  = search_result.m_hitIndex[ 1 ];

				/* Hit is expected on all the parallel searches */
				if( search_result2 == NLMDEV_MISS || search_result3 == NLMDEV_MISS  )
				{
					NlmCmFile__printf("Got Miss with MasterKey#%u\n", iter);
					NlmCmFile__printf("search_result2 = %s\n", 
										search_result2 ? "Hit" : "Miss");
					NlmCmFile__printf("search_result3 = %s\n", 
										search_result3 ? "Hit" : "Miss");
				}
				else
				{
					/* Check whether the indexes returned by search or correct or not? */
					if( exp_index2 != search_index2 ||
						exp_index3 != search_index3  )
					{
						NlmCmFile__printf("Search failed with MasterKey#%u\n", iter);
						NlmCmFile__printf("exp_index2 = %u, search_index2 = %u\n",
											exp_index2, search_index2);
						NlmCmFile__printf("exp_index3 = %u, search_index3 = %u\n",
											exp_index3, search_index3);
					}
				}
        	}

			if( 0 == (iter % DEBUG_PRINT_ITER_VAL) )
			{
				NlmCmFile__printf("\r\t   Number of keys searched so far [%u]", iter);
			}
		} /* tbl2 loop */
	}     /* tbl3 loop */

	NlmCmFile__printf("\n\tNumber of keys searched [%u]\n", iter);
	return RETURN_STATUS_OK;
}

/* Compare1, 320B_4 table is searched (Key has ranges) */ 
static int Perform_LTR2_Searches( genericTblMgrRefAppData *refAppData_p )
{
	tableInfo   *tblInfo4_p;
    tableRecordInfo   *tblRecordInfo4_p;
	NlmGenericTblRecord *tblRecord4_p, *tblRecord4_base_p;
    NlmReasonCode   reason = NLMRSC_REASON_OK;
	nlm_u32 iter_tbl4, numRecs_tbl4;
	nlm_u32 exp_index4, search_index4, dst_port, src_port;
	nlm_u8  ltr_num;

	/* Device Manager declarations */
	NlmDevCtxBufferInfo      cb_info;
	NlmDevCmpRslt     search_result;
	NlmDevMissHit search_result4;

	NlmCmFile__printf("\n\tPerforming LTR#2 searches\n");

	/* Construct Master Key for Compare1 operation.
	 * Master Key construction is as follows:
	 * Master key is constructed by using the 320b records inserted into
	 * this table. MSB 160 bits is same in all the records, this portion
	 * is written into the Context Buffer (CB) starting address 2 which is used
	 * while constructing Master Key. Master Key contains destination port
	 * value of 1, source port value of 2. 
	 * The destination port is filled in bits [111:96] of Master Key. 
	 * The source port is filled in bits [31:16] of Master Key.
     */
	tblInfo4_p = &refAppData_p->tblInfo[ 4 ];
	tblRecordInfo4_p = tblInfo4_p->tblRecordInfo_p;
	tblRecord4_base_p = &tblRecordInfo4_p->record;
	numRecs_tbl4 = tblInfo4_p->rec_count;
	ltr_num = 2;
	dst_port = 1;
	src_port = 2;
	cb_info.m_cbStartAddr = 0;
	cb_info.m_datalen = 40;

	for( iter_tbl4 = 0; iter_tbl4 < numRecs_tbl4; iter_tbl4++ )
	{
		NlmCm__memcpy( cb_info.m_data, tblRecord4_base_p->m_data, 20 );
		exp_index4 = (tblRecordInfo4_p + iter_tbl4)->index;
		tblRecord4_p = &( (tblRecordInfo4_p + iter_tbl4)->record );

		/* Copy LSB 160b into data. Then insert port values */
		NlmCm__memcpy( &cb_info.m_data[20], &tblRecord4_p->m_data[ 20 ], 20 );

		/* Stitch destination port at [111:96], [95:80] must be spare bits.
		 * stitch source port at [31:16], [15:0] must be spare bits.
		 */
		WriteBitsInArray( cb_info.m_data, 40, 31, 16, src_port );
		WriteBitsInArray( cb_info.m_data, 40, 111, 96, dst_port );		

		/* Fire Compare1 search now */
    	if( NLMERR_OK != NlmDevMgr__Compare1( refAppData_p->devMgr_p, ltr_num,
													&cb_info, &search_result, &reason ) )
		{
       		NlmCmFile__printf("Compare1 Instruction failed. Exiting...\n");
			printReasonCode( reason );

       		return NLMERR_FAIL;
   		}
   		else
		{
			search_result4 = search_result.m_hitOrMiss[ 0 ]; 
			search_index4  = search_result.m_hitIndex[ 0 ];

			/* Hit is expected */
			if( search_result4 == NLMDEV_MISS )
			{
				NlmCmFile__printf("Got Miss with MasterKey#%u\n", iter_tbl4);
				NlmCmFile__printf("search_result4 = %s\n", 
									search_result4? "Hit" : "Miss");
			}
			else
			{
				/* Check whether the index returned by search or correct or not? */
				if( exp_index4 != search_index4 )
				{
					NlmCmFile__printf("Search failed with MasterKey#%u\n", iter_tbl4);
					NlmCmFile__printf("exp_index4 = %u, search_index4 = %u\n",
									exp_index4, search_index4);
				}
			}
       	}

		if( 0 == (iter_tbl4 % DEBUG_PRINT_ITER_VAL) )
		{
			NlmCmFile__printf("\r\t   Number of keys searched so far [%u]", iter_tbl4);
		}
	} /* tbl4 loop */

	NlmCmFile__printf("\n\tNumber of keys searched [%u]\n", iter_tbl4);
	return RETURN_STATUS_OK;
}



/* Compare2 searches on 640B_5 table */
static int Perform_LTR3_Searches( genericTblMgrRefAppData *refAppData_p )
{
	tableInfo   *tblInfo5_p;
    tableRecordInfo   *tblRecordInfo5_p;
	NlmGenericTblRecord *tblRecord5_p;
    NlmReasonCode   reason = NLMRSC_REASON_OK;
	nlm_u32 iter_tbl5, numRecs_tbl5;
	nlm_u32 search_index5, exp_index5;
	nlm_u8  ltr_num;

	/* Device Manager declarations */
	NlmDevCtxBufferInfo      cb_info;
	NlmDevCmpRslt     search_result;
	NlmDevMissHit search_result5;

	NlmCmFile__printf("\n\tPerforming LTR#3 searches\n");

	/* Construct Master Key for Compare2 operation.
	 * Master key is constructed using the 640b records inserted into this table
	 * MasterKey[639:0] has 640b_5 table record
	 */
	tblInfo5_p = &refAppData_p->tblInfo[ 5 ];
	tblRecordInfo5_p = tblInfo5_p->tblRecordInfo_p;
	tblRecord5_p = &tblRecordInfo5_p->record;
	numRecs_tbl5 = tblInfo5_p->rec_count;

	
   	ltr_num = 3;
	for( iter_tbl5 = 0; iter_tbl5 < numRecs_tbl5; iter_tbl5++ )
	{
		exp_index5 = (tblRecordInfo5_p + iter_tbl5)->index;
		tblRecord5_p = &( (tblRecordInfo5_p + iter_tbl5)->record );

		/* Copy the 640b record of table 640B_5 in to the CB locations 0 to 7.
		 * MasterKey[639:0] has 640b_5 table record 
		 */
    	
		NlmCm__memcpy( cb_info.m_data, &tblRecord5_p->m_data[ 0 ],
							80 );
		cb_info.m_datalen = 80; /* 80 bytes of data to be written */
		cb_info.m_cbStartAddr = 0;
		
		/* Fire Compare2 search */
    	if( NLMERR_OK != NlmDevMgr__Compare2( refAppData_p->devMgr_p, ltr_num,
													&cb_info, &search_result, &reason ) )
		{
       		NlmCmFile__printf("Compare2 Instruction failed. Exiting...\n");
			printReasonCode( reason );

       		return NLMERR_FAIL;
   		}
   		else
		{
			search_result5 = search_result.m_hitOrMiss[ 0 ];
			search_index5  = search_result.m_hitIndex[ 0 ];

			/* Hit is expected */
			if( search_result5 == NLMDEV_MISS  )
			{
				NlmCmFile__printf("Got Miss with MasterKey#%u\n", iter_tbl5);
				NlmCmFile__printf("search_result5 = %s\n", 
									search_result5? "Hit" : "Miss");
			}
			else
			{
				/* Check whether the indexes returned by search or correct or not? */
				if( exp_index5 != search_index5  )
				{
					NlmCmFile__printf("Search failed with MasterKey#%u\n", iter_tbl5);
					NlmCmFile__printf("exp_index5 = %u, search_index5 = %u\n",
										exp_index5, search_index5);
				}
			}
       	}

		if( 0 == (iter_tbl5 % DEBUG_PRINT_ITER_VAL) )
		{
			NlmCmFile__printf("\r\t   Number of keys searched so far [%u]", iter_tbl5);
		}
	} /* tbl5 loop */

	NlmCmFile__printf("\n\tNumber of keys searched [%u]\n", iter_tbl5);
	return RETURN_STATUS_OK;
}


nlm_u8 FillRecordPatternFor_80B0_Table( nlm_u8 *data_p, nlm_u8 *mask_p )
{
	/* 0.X.X.X.X__0.0.0.0.0, 0.X.X.X.X__0.0.0.0.1,
     * 0.X.X.X.X__0.0.0.0.2 and so on;
	 */
	NlmCm__memset( data_p, 0, 10 );
	NlmCm__memset( mask_p, 0, 10 );

	/* Put table id at the MSB byte */
	data_p[ 0 ] = 0;

	/* Mask off next 4 bytes */
	mask_p++;
	NlmCm__memset( mask_p, 0xFF, 4 );

	/* Byte index within the record from where varying data starts */
	return 6;
}

nlm_u8 FillRecordPatternFor_80B1_Table( nlm_u8 *data_p, nlm_u8 *mask_p )
{
	/* 1.0.0.0.0__X.X.X.X.X, 1.0.0.0.1__X.X.X.X.X,
     * 1.0.0.0.2__X.X.X.X.X and so on;
	 */
	NlmCm__memset( data_p, 0, 10 );
	NlmCm__memset( mask_p, 0, 10 );

	/* Put table id at the MSB byte */
	data_p[ 0 ] = 1;

	/* Mask off LSB 5 bytes */
	mask_p += 5;
	NlmCm__memset( mask_p, 0xFF, 5 );

	/* Byte index within the record from where varying data starts */
	return 1;
}

nlm_u8 FillRecordPatternFor_160B2_Table( nlm_u8 *data_p, nlm_u8 *mask_p )
{
	/* 2.9.9.9.9__10.10.10.10.10__X.X.X.X.X__0.0.0.0.0,
     * 2.9.9.9.9__10.10.10.10.10__X.X.X.X.X__0.0.0.0.1 and so on
	 */
	NlmCm__memset( data_p, 0, 20 );
	NlmCm__memset( mask_p, 0, 20 );

	/* Put table id at the MSB byte */
	data_p[ 0 ] = 2;

	NlmCm__memset( (data_p + 1), 9, 4 );
	NlmCm__memset( (data_p + 5), 10, 5 );

	/* Mask off some bytes */
	mask_p += 10;
	NlmCm__memset( mask_p, 0xFF, 5 );

	/* Byte index within the record from where varying data starts */
	return 16;
}

nlm_u8 FillRecordPatternFor_160B3_Table( nlm_u8 *data_p, nlm_u8 *mask_p )
{
	/* 3.9.9.9.9__10.10.10.10.10__0.0.0.0.0__X.X.X.X.X,
     * 3.9.9.9.9__10.10.10.10.10__0.0.0.0.1__X.X.X.X.X and so on
	 */
	NlmCm__memset( data_p, 0, 20 );
	NlmCm__memset( mask_p, 0, 20 );

	/* Put table id at the MSB byte */
	data_p[ 0 ] = 3;

	NlmCm__memset( (data_p + 1), 9, 4 );
	NlmCm__memset( (data_p + 5), 10, 5 );

	/* Mask off some bytes */
	mask_p += 15;
	NlmCm__memset( mask_p, 0xFF, 5 );

	/* Byte index within the record from where varying data starts */
	return 11;
}

nlm_u8 FillRecordPatternFor_320B4_Table( nlm_u8 *data_p, nlm_u8 *mask_p )
{
	/* 320B_4: 4.1.1.1.1__X.X.X.X.X__2.2.2.2.2__X.X.X.X.X__
	 *         3.3.3.3.3__X.<DPE>__0.0.0.0.0__X.<SPE>,
	 *         4.1.1.1.1__X.X.X.X.X__2.2.2.2.2__X.X.X.X.X__
	 *         3.3.3.3.3__X.<DPE>__0.0.0.0.1__X.<SPE>
	 *         and so on
	 *         DPE = 32b Destination Port Encodings
	 *         SPE = 32b Source Port Encodings
	 */
	NlmCm__memset( data_p, 0, 40 );
	NlmCm__memset( mask_p, 0, 40 );

	/* Put table id at the MSB byte */
	data_p[ 0 ] = 4;

	NlmCm__memset( (data_p + 1), 1, 4 );
	NlmCm__memset( (data_p + 10), 2, 5 );
	NlmCm__memset( (data_p + 20), 3, 5 );

	/* Mask off some bytes */
	NlmCm__memset( (mask_p + 5), 0xFF, 5 );
	NlmCm__memset( (mask_p + 15), 0xFF, 5 );
	NlmCm__memset( (mask_p + 25), 0xFF, 1 );
	NlmCm__memset( (mask_p + 35), 0xFF, 1 );

	/* Byte index within the record from where varying data starts */
	return 31;
}

nlm_u8 FillRecordPatternFor_640B5_Table( nlm_u8 *data_p, nlm_u8 *mask_p )
{
	/* 640b_5: 5.1.1.1.1__X.X.X.X.X__2.2.2.2.2__X.X.X.X.X__
	 *         3.3.3.3.3__X.X.X.X.X__4.4.4.4.4__X.X.X.X.X__
	 *         5.5.5.5.5__X.X.X.X.X__6.6.6.6.6__X.X.X.X.X__
	 *         7.7.7.7.7__X.X.X.X.X__8.8.8.8.8__0.0.0.0.0,
	 *         5.1.1.1.1__X.X.X.X.X__2.2.2.2.2__X.X.X.X.X__
	 *         3.3.3.3.3__X.X.X.X.X__4.4.4.4.4__X.X.X.X.X__
	 *         5.5.5.5.5__X.X.X.X.X__6.6.6.6.6__X.X.X.X.X__
	 *         7.7.7.7.7__X.X.X.X.X__8.8.8.8.8__0.0.0.0.1 
	 *         and so on
	 */
	
	NlmCm__memset( data_p, 0, 80 );
	NlmCm__memset( mask_p, 0, 80 );

	/* Put table id at the MSB byte */
	data_p[ 0 ] = 5;

	NlmCm__memset( (data_p + 1),  1, 4 );
	NlmCm__memset( (data_p + 10), 2, 5 );
	NlmCm__memset( (data_p + 20), 3, 5 );
	NlmCm__memset( (data_p + 30), 4, 5 );
	NlmCm__memset( (data_p + 40), 5, 5 );
	NlmCm__memset( (data_p + 50), 6, 5 );
	NlmCm__memset( (data_p + 60), 7, 5 );
	NlmCm__memset( (data_p + 70), 8, 5 );

	/* Mask off some bytes */
	NlmCm__memset( (mask_p + 5),  0xFF, 5 );
	NlmCm__memset( (mask_p + 15), 0xFF, 5 );
	NlmCm__memset( (mask_p + 25), 0xFF, 5 );
	NlmCm__memset( (mask_p + 35), 0xFF, 5 );
	NlmCm__memset( (mask_p + 45), 0xFF, 5 );
	NlmCm__memset( (mask_p + 55), 0xFF, 5 );
	NlmCm__memset( (mask_p + 65), 0xFF, 5 );

	/* Byte index within the record from where varying data starts */
	return 76;
}


nlm_u8 UpdateRecordPatternFor_320B4_Table( nlm_u8 *data_p, nlm_u8 *mask_p )
{
	/* 320B_4: 4.1.1.1.1__X.X.X.X.X__5.5.5.5.5__X.X.X.X.X__
	 *         6.6.6.6.6__X.<DPE>__0.0.0.0.0__X.<SPE>,
	 *         4.1.1.1.1__X.X.X.X.X__5.5.5.5.5__X.X.X.X.X__
	 *         6.6.6.6.6__X.<DPE>__0.0.0.0.1__X.<SPE>
	 *         and so on
	 *         DPE = 32b Destination Port Encodings
	 *         SPE = 32b Source Port Encodings
	 */
	NlmCm__memset( data_p, 0, 40 );
	NlmCm__memset( mask_p, 0, 40 );

	/* Put table id at the MSB byte */
	data_p[ 0 ] = 4;

	NlmCm__memset( (data_p + 1), 1, 4 );
	NlmCm__memset( (data_p + 10), 5, 5 );
	NlmCm__memset( (data_p + 20), 6, 5 );

	/* Mask off some bytes */
	NlmCm__memset( (mask_p + 5), 0xFF, 5 );
	NlmCm__memset( (mask_p + 15), 0xFF, 5 );
	NlmCm__memset( (mask_p + 25), 0xFF, 1 );
	NlmCm__memset( (mask_p + 35), 0xFF, 1 );

	/* Byte index within the record from where varying data starts */
	return 31;
}



/* This function updates the internal data strcture for 320B4 table with records */
static void UpdateRecordsIn_320B4_AppTable( genericTblMgrRefAppData *refAppData_p )
{
	tableInfo *tblInfo_p;
	tableRecordInfo *tblRecordInfo_p;
	nlm_u32 iter_rec;
	nlm_u8 tblWidth_inBytes, start_byte = 0;
	nlm_u8 *rec_data, *rec_mask, *var_data_p;
	nlm_u16 start, end, iter_group, iter_priority;

	tblInfo_p = &refAppData_p->tblInfo[ 4 ];
 	start     = tblInfo_p->groupId_start;
 	end       = tblInfo_p->groupId_end;

	/* Record pattern is as follows:
 	 * In the following text "X" means a dont care of 8 bits
 	 *
 	 * 320B_4: 4.1.1.1.1__X.X.X.X.X__5.5.5.5.5__X.X.X.X.X__
 	 *         6.6.6.6.6__X.X.X.<DPE>__0.0.0.0.0__X.X.X.<SPE>,
	 *         4.1.1.1.1__X.X.X.X.X__5.5.5.5.5__X.X.X.X.X__
	 *         6.6.6.6.6__X.X.X.<DPE>__0.0.0.0.1__X.X.X.<SPE>
 	 *         and so on
	 *         DPE = 32b Destination Port Encodings
	 *         SPE = 32b Source Port Encodings
 	 */
 	tblWidth_inBytes = (nlm_u8)(tblInfo_p->tbl_width / 8);
	rec_data = NlmCmAllocator__calloc( refAppData_p->alloc_p,
 										1, tblWidth_inBytes );
	rec_mask = NlmCmAllocator__calloc( refAppData_p->alloc_p,
 										1, tblWidth_inBytes );

	/* get the starting record pattern based on the table id */

	start_byte = UpdateRecordPatternFor_320B4_Table( rec_data, rec_mask );
		
	/* 4 bytes starting from this byte location are incremented. These 4 bytes are
	 * the only variable data portion across all records.
	 */
	var_data_p = rec_data + start_byte;

	/* Start updating the records now */
	tblRecordInfo_p = tblInfo_p->tblRecordInfo_p;
	for( iter_group = start, iter_rec = 0; iter_group <= end; iter_group++ )
	{
		for( iter_priority = 0; iter_priority < iter_group; iter_priority++ )
		{
			WriteValueToBitVector4( var_data_p, iter_rec );

			NlmCm__memcpy( tblRecordInfo_p->record.m_data, rec_data, tblWidth_inBytes );
			NlmCm__memcpy( tblRecordInfo_p->record.m_mask, rec_mask, tblWidth_inBytes );

			/* Dont worry, we wont over index. */
			tblRecordInfo_p++;
			iter_rec++;
		}
	}


	/* 320B_4 table has ranges. Stitch the ranges within records */
	StitchRangesWithin_320B4_Records( refAppData_p );

	
	/* Free the memory allocated for temp data and mask. A fresh memory will be
	 * based on the table width
	 */
	NlmCmAllocator__free( refAppData_p->alloc_p, rec_data );
	NlmCmAllocator__free( refAppData_p->alloc_p, rec_mask );
} 


/* Update records into 320B4 table */
static int UpdateRecordsTo_320B4_Table(
				genericTblMgrRefAppData *refAppData_p,
				NlmGenericTbl *tbl_p)
{
	tableInfo *tblInfo_p;
	tableRecordInfo	*tblRecordInfo_p;
	NlmReasonCode   reason = NLMRSC_REASON_OK;
	nlm_u32 iter_rec = 0, num_recs = 0;
	nlm_u8  tbl_id;

	tbl_id = GetTableId( tbl_p->m_id_str_p );
	tblInfo_p = &refAppData_p->tblInfo[ tbl_id ];
	tblRecordInfo_p = tblInfo_p->tblRecordInfo_p;
	num_recs  = tblInfo_p->max_recCount;

	NlmCmFile__printf("\n\tUpdating records into table [%s]\n", tbl_p->m_id_str_p);
	for( iter_rec = 0; iter_rec < num_recs; iter_rec++ )
	{
		/* Update record now */
		if( NLMERR_OK != NlmGenericTblMgr__UpdateRecord( tbl_p,
													  &tblRecordInfo_p->record,
													  tblRecordInfo_p->index,
													  &reason ) )
		{
        		NlmCmFile__printf("\ttable [%s] updation: could not update record#%u\n",
								tbl_p->m_id_str_p, tblInfo_p->rec_count); 
			printReasonCode( reason );

        		return NLMERR_FAIL;
		}

		/* Advance the record pointer */
		tblRecordInfo_p ++;
		
		NlmCmFile__printf("\r\t   Number of records updated so far [%u]", iter_rec );

	} /* end of for loop */

	NlmCmFile__printf("\n\tNumber of records updated [%u]\n", iter_rec);

	return RETURN_STATUS_OK;
}

int nlmgenerictblmgr_refapp_main(int argc, char	*argv[])
{
	genericTblMgrRefAppData  refAppData_p;
	nlm_32	break_alloc_id = -1;
	nlm_u8  iter_tbl;

	(void)argc;
	(void)argv;

	NlmCmDebug__Setup( break_alloc_id, NLMCM_DBG_EBM_ENABLE );

	NlmCmFile__printf ("\n\tGeneric Table Manager Application Reference Code Using\n");
	NlmCmFile__printf ("\tGeneric Table Manager Module. \n\n\n");

	/* Initialize customer specific entities: Memory Allocator, XPT Interface */
	if( RETURN_STATUS_OK != InitEnvironment( &refAppData_p ) )
	{
		NlmCmFile__printf("\tEnvironment Initialization failed. Exiting...\n");

		return RETURN_STATUS_ABORT;
	}

	/* Initialize Device Manager now */
	if( RETURN_STATUS_OK != InitDeviceManager( &refAppData_p ) )
	{
		NlmCmFile__printf("\tDevice Manager Initialization failed. Exiting...\n");

		return RETURN_STATUS_ABORT;
	}
	
	/* Initialize Range Manager now */
	if( RETURN_STATUS_OK != InitRangeManager( &refAppData_p ) )
	{
		NlmCmFile__printf("\tRange Manager Initialization failed. Exiting...\n");

		return RETURN_STATUS_ABORT;
	}

	/* Initialize Generic Table Manager now */
	if( RETURN_STATUS_OK != InitGenericTableManager( &refAppData_p ) )
	{
		NlmCmFile__printf("\tGeneric Table Manager Initialization failed. Exiting...\n");

		return RETURN_STATUS_ABORT;
	}

	/* Add records to tables now */
	for( iter_tbl = 0; iter_tbl < NUM_OF_TABLES; iter_tbl++ )
	{
		AddRecordsToTables( &refAppData_p, refAppData_p.tbl_p[ iter_tbl], 0 );
	}

	/* Add records which will cause shuffles in the device */
	for( iter_tbl = 0; iter_tbl < NUM_OF_TABLES; iter_tbl++ )
	{
		AddRecordsToTables( &refAppData_p, refAppData_p.tbl_p[ iter_tbl], 1 );
	}
	
	/* Start searches now */
	Perform_LTR0_Searches( &refAppData_p );
	Perform_LTR1_Searches( &refAppData_p );
	Perform_LTR2_Searches( &refAppData_p );
	Perform_LTR3_Searches( &refAppData_p );


	 /* Update records to Application 320B4 table */
	 UpdateRecordsIn_320B4_AppTable (&refAppData_p);
		
	 
	 /* Update records to 320B4 table */
	 UpdateRecordsTo_320B4_Table( &refAppData_p, refAppData_p.tbl_p[ 4]);
	
 	 /* Start searches now */
	 Perform_LTR2_Searches( &refAppData_p );


	 /* destroy Generic Table Manager */
    if( RETURN_STATUS_OK != DestroyGenericTableManager( &refAppData_p ) )
    {
        NlmCmFile__printf("\tGeneric Table Manager Destroy failed. Exiting...\n");

        return RETURN_STATUS_ABORT;
    }

	/* Destroy Range Manager */
	if( RETURN_STATUS_OK != DestroyRangeManager( &refAppData_p ) )
	{
		NlmCmFile__printf("\tRange Manager Destroy failed. Exiting...\n");

		return RETURN_STATUS_ABORT;
	}

	/* destroy device manager */
	if( RETURN_STATUS_OK != DestroyDeviceManager( &refAppData_p ) )
	{
		NlmCmFile__printf("\tDevice Manager Destroy failed. Exiting...\n");

		return RETURN_STATUS_ABORT;
	}

	if( RETURN_STATUS_OK != DestroyEnvironment( &refAppData_p ) )
	{
		NlmCmFile__printf("\tDestroy Environment failed. Exiting...\n");

		return RETURN_STATUS_ABORT;
	}

	if(NlmCmDebug__IsMemLeak())
	{
        NlmCmFile__printf("\tMemory Leak\n");
	}

	NlmCmFile__printf("\n\tProgram Completed Successfully\n");

	return 0;
}

