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
 * $Id: nlmrangemgr_refapp3.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2007 Broadcom Corp.
 * All Rights Reserved.$
 */

 
#include "nlmrangemgr_refapp3.h"

/* Initialize Memory Allocator and Transport Layer */
static
int NlmRangeMgrRefApp__InitEnvironment( NlmRangeMgrRefAppData *refAppData_p )
{

	NlmCm__memset( refAppData_p, 0, sizeof(NlmRangeMgrRefAppData) );

	/* create default memory allocator */
    refAppData_p->alloc_p = NlmCmAllocator__ctor( &refAppData_p->alloc_bdy );
    if( refAppData_p->alloc_p == NULL )
    {
        NlmCm__printf("\n\tCould not create Memory Allocator. Exiting..!!!!!\n");

        return NLMERR_FAIL;
    }

	/* DevMgr flushes each request immediately and hence the Queue length of 1 */
	refAppData_p->request_queue_len = 1;
	refAppData_p->result_queue_len  = 1;

	/* Search system has only one channel (cascade of devices). Transport Interface is Simulation
	 * Transport Interface
	 */
	refAppData_p->channel_id = 0;
#ifndef NLMPLATFORM_BCM 
	refAppData_p->if_type  = IFTYPE_CMODEL;

#ifdef NLM_XLP
	refAppData_p->if_type  = IFTYPE_XLPXPT;
#endif
#else /* BCMPLATFORM */
	refAppData_p->if_type  = IFTYPE_BCM_CALADAN3;
#endif /* NLMPLATFORM_BCM */

	/* Create transport interface. Only interface supported with this release is SimXpt */
	switch( refAppData_p->if_type )
	{
#ifndef NLMPLATFORM_BCM
		case IFTYPE_CMODEL:
		{
			/* This Processor being a serial device, speed mode and rbus mode can be ignored. */
			refAppData_p->oprMode = NLMDEV_OPR_STANDARD;
            refAppData_p->xpt_p = NlmSimXpt__Create( refAppData_p->alloc_p,
            										 NLM_DEVTYPE_2,
			                                         0, /* speed mode, not used in this Processor */
													 refAppData_p->request_queue_len,
													 refAppData_p->result_queue_len,
										             refAppData_p->oprMode,
													 0, /* rbus mode, not used in this Processor */
													 refAppData_p->channel_id );
    		if ( refAppData_p->xpt_p == NULL )
            {
                NlmCm__printf("\n\tCould not create Simulation Transport Interface. Exiting..!!!!!\n");

                return NLMERR_FAIL;
            }

			NlmCm__printf("\n\tSimulation Transport Interface Created Successfully...\n");
			break;
		}
		case IFTYPE_FPGA:
		{
			NlmCm__printf("\n\tThis interface type is not yet supported...\n");

			return NLMERR_FAIL;
		}
#ifdef NLM_XLP
		case IFTYPE_XLPXPT:
		{
			refAppData_p->xpt_p = NlmXlpXpt__Create( 	refAppData_p->alloc_p,
														NLM_DEVTYPE_2,
														refAppData_p->request_queue_len,
			                           					refAppData_p->oprMode,
	                                                    refAppData_p->channel_id, 
	                                                    0, 0, 1,1);
			NlmCm__printf("\nXLP Transport Interface Created Successfully\n");

			 if(refAppData_p->xpt_p == NULL)
			 	return NLMERR_FAIL;
			 break;
		}
#endif
#endif /* NLMPLATFORM_BCM */
                case IFTYPE_BCM_CALADAN3:
                    refAppData_p->xpt_p = soc_sbx_caladan3_etu_xpt_create(0,
									  NLM_DEVTYPE_2, 0, 0,
									  refAppData_p->oprMode, refAppData_p->channel_id);
		    NlmCmFile__printf("\n Caladan3 Transport Interface Created Successfully\n");
                    if (refAppData_p->xpt_p == NULL) 
                    {
                        NlmCm__printf("\n\tError: soc_sbx_caladan3_etu_xpt_create failed");
                        return RETURN_STATUS_FAIL;
                    }
                    break;
		default:
		{
			NlmCm__printf("Invalid interface type...\n");

			return NLMERR_FAIL;
		}
	}

	return NLMERR_OK;
}


static 
int NlmRangeMgrRefApp__DestroyEnvironment( NlmRangeMgrRefAppData *refAppData_p )
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
			NlmCmFile__printf("This interface type is not yet supported...\n");

			return NLMERR_FAIL;
		}
#ifdef NLM_XLP
		case IFTYPE_XLPXPT:
		{
			NlmCmFile__printf("\n\tDestroying XLP Transport Interface\n");
			NlmXlpXpt__Destroy( refAppData_p->xpt_p );

			break;
		}
#endif
#endif
                case IFTYPE_BCM_CALADAN3:
		{
                    if (refAppData_p->xpt_p)
                        soc_sbx_caladan3_etu_xpt_destroy(refAppData_p->xpt_p);		    
		    break;
		}
		default:
		{
			NlmCmFile__printf("\n\tInvalid interface type...\n");

			return NLMERR_FAIL;
		}

	}

	return RETURN_STATUS_OK;
}

static
int NlmRangeMgrRefApp__InitDevMgr( NlmRangeMgrRefAppData *refAppData_p )
{
	NlmDevId   devId;
    NlmReasonCode reason;

    refAppData_p->devMgr_p = NlmDevMgr__create( refAppData_p->alloc_p,
	                                               refAppData_p->xpt_p,
												   refAppData_p->oprMode,
												   &reason );
	if( NULL == refAppData_p->devMgr_p )
	{
        NlmCm__printf("\n\tErr: Could not initialize Device Manager. Reason Code = %d\n", reason);

        return NLMERR_FAIL;
	}

	NlmCm__printf("\n\tDevice Manager Initialized Successfully...\n");

	/* Add a device to the search system. Device id returned by DevMgr */
	refAppData_p->dev_p = NlmDevMgr__AddDevice( refAppData_p->devMgr_p,
	                                               &devId,
												   &reason );
	if( NULL == refAppData_p->dev_p )
	{
        NlmCm__printf("\n\tCould not add device. Reason Code = %d\n", reason);

        return NLMERR_FAIL;
	}

	refAppData_p->devId = devId;
	NlmCm__printf("\tDevice added to the search system... \n");

	/* We are done with configurations. Now Lock Device Manager */
	if( NLMERR_OK != NlmDevMgr__LockConfig( refAppData_p->devMgr_p, &reason ) )
	{
        NlmCm__printf("\n\tCould not lock Device Manager. Reason Code = %d\n", reason);

        return NLMERR_FAIL;
	}

	NlmCm__printf("\tDevice Manager Configuration Locked...\n");

	return NLMERR_OK;
}

static
int NlmRangeMgrRefApp__DestroyDevMgr( NlmRangeMgrRefAppData *refAppData_p )
{
	NlmCmFile__printf("\n\tDestroying Device Manager\n");
	NlmDevMgr__destroy( refAppData_p->devMgr_p );

	return RETURN_STATUS_OK;
}

static
int NlmRangeMgrRefApp__InitRangeMgr( NlmRangeMgrRefAppData *refAppData_p )
{
	/* For MCOR only these values does not have meaning, we can pass either _3B_, _2B_ */
	NlmRangeEncodingType encodingType[4] = { NLM_RANGE_3B_ENCODING,
											 NLM_RANGE_3B_ENCODING,
											 NLM_RANGE_3B_ENCODING,
											 NLM_RANGE_3B_ENCODING
											};

	/* Range DB-0: m_num_bits = 8, m_valid_bits = 16   (MCOR only       )	 */
	NlmRangeDbAttrSet          db_attrs[NUM_OF_RANGE_DATABASES] = {
	                                                                {8, 16, NLM_RANGE_NO_ENCODING}
                                                                   };
	NlmDev                 *dev_p = refAppData_p->dev_p;
	NlmDevConfigReg         dc_reg;
	nlm_u8                     db_id = 0;
	NlmReasonCode              reason;

	/* Enable Range Matching Engine in the device first. DCR is a global register.
	  * Default value of dbParityErrEntryInvalidate bit is 1 in the device.
	  */
	NlmCm__memset( &dc_reg, 0, sizeof(NlmDevConfigReg) );

	dc_reg.m_rangeEngineEnable = NLMDEV_ENABLE;
	dc_reg.m_dbParityErrEntryInvalidate = NLMDEV_ENABLE;
	if( NLMERR_OK != NlmDevMgr__GlobalRegisterWrite( dev_p, NLMDEV_DEVICE_CONFIG_REG,
														 &dc_reg, &reason ) )
	{
		NlmCm__printf("\n\t Could not enable Range Matching Engine. Reason Code -- %d\n",
						reason);

		return NLMERR_FAIL;
	}


    refAppData_p->rangeMgr_p = NlmRangeMgr__Init( refAppData_p->alloc_p,
	                                              refAppData_p->devMgr_p,
												  NLM_DEVTYPE_2,
												  encodingType,
												  &reason );

	if( NULL == refAppData_p->rangeMgr_p )
	{
        NlmCm__printf("\n\tCould not initialize Range Manager. Reason Code = %d\n", reason);

        return NLMERR_FAIL;
	}

	NlmCm__printf("\n\tRange Manager Initialized Successfully...\n");

	/* Now create Range database. Database attributes are specified during
	 * __CreateDb itself. They are:
	 *  (1) m_num_bits, number of bits available for range encoding
	 *  (2) m_valid_bits, how many (LSB) bits of 16b range field are valid.
	 *  (3) m_encodingType, encoding type to be used.
	 */

	for( db_id = 0; db_id < NUM_OF_RANGE_DATABASES; db_id++ )
	{
		refAppData_p->db_p[db_id] = NlmRangeMgr__CreateDb( refAppData_p->rangeMgr_p,
		                                                                                         db_id,
														   &db_attrs[db_id],
														   &reason );
		if( NULL == refAppData_p->db_p[db_id] )
		{
                	NlmCm__printf("\n\tCould not create dabase-%d. Reason Code = %d\n", db_id, reason);
                	return NLMERR_FAIL;
		}

		NlmCm__memcpy( &refAppData_p->db_attrs[db_id], &db_attrs[db_id], sizeof(NlmRangeDbAttrSet) );
		NlmCm__printf("\tRange Database [Database ID = %d] Created...\n", db_id);
 	}

	/* Assign h/w range blocks to range databases. This assignment is required only if
	 * MCOR encoding is possible with the database. Db-0 uses MCOR only and Range-A is assigned
	 * to db-0.
	 */
	if( NLMERR_OK != NlmRangeMgr__AssignRange( refAppData_p->rangeMgr_p,
	                                           refAppData_p->db_p[0],
											   NLM_RANGE_TYPE_A,
											   &reason ) )
    {
    	NlmCm__printf("\n\tRange-A assignment to db-0 failed. Reason Code = %d\n", reason);
       	return NLMERR_FAIL;
	}

	return NLMERR_OK;
}

static
int NlmRangeMgrRefApp__DestroyRangeMgr( NlmRangeMgrRefAppData *refAppData_p )
{
	NlmReasonCode   reason;
	nlm_u8  db_id;

	NlmCmFile__printf("\n");
	/* Destroy range databases first and then the manager */
	for( db_id = 0; db_id < NUM_OF_RANGE_DATABASES; db_id++ )
	{
		NlmCmFile__printf("\tDestroying  db-%d\n", db_id);
		NlmRangeMgr__DestroyDb( refAppData_p->rangeMgr_p, refAppData_p->db_p[db_id], &reason );
	}

	NlmCmFile__printf("\tDestroying Range Manager\n");
	NlmRangeMgr__Destroy( refAppData_p->rangeMgr_p, &reason );

	return RETURN_STATUS_OK;
}


static
int NlmRangeMgrRefApp__ConfigSearches( NlmRangeMgrRefAppData *refAppData_p )
{
    NlmDev                   *dev_p;
    NlmRangeSrchAttrs           *srch_attrs;
	NlmDevBlkSelectReg        bs_reg;
	NlmDevParallelSrchReg     ps_reg;
	NlmDevSuperBlkKeyMapReg   sb_reg;
	NlmDevKeyConstructReg     kcr_reg;
	NlmDevBlockConfigReg      bcr_reg;
	NlmDevRangeInsertion0Reg  ri0_reg;
	NlmDevRangeInsertion1Reg  ri1_reg;
	NlmReasonCode                reason;
	nlm_u8                       ltr_num, blk_num, keyNum;

	/* One table tbl-0 of 160b width is created which is searched using Key#0 producing result
	  * onto Parallel Search Result Port-0 (PS#0).
	  * tbl-0 records reside in Block-0 (Super Block-0).
	  */
	dev_p = refAppData_p->dev_p;

	/* LTR-0 is used with the current search configuration */
	ltr_num = 0;
	NlmCm__printf("\n\tConfiguring searches with LTR#%d...\n", ltr_num);

	/* Initialize the structures with default values first */
	NlmCm__memset( &bs_reg,  0, sizeof( NlmDevBlkSelectReg ) );
	NlmCm__memset( &ps_reg,  0, sizeof( NlmDevParallelSrchReg ) );
	NlmCm__memset( &sb_reg,  0, sizeof( NlmDevSuperBlkKeyMapReg ) );
	NlmCm__memset( &kcr_reg, 0, sizeof( NlmDevKeyConstructReg ) );
	NlmCm__memset( &bcr_reg,  0, sizeof( NlmDevBlockConfigReg ) );
	NlmCm__memset( &ri0_reg,  0, sizeof( NlmDevRangeInsertion0Reg ) );
	NlmCm__memset( &ri1_reg,  0, sizeof( NlmDevRangeInsertion1Reg ) );

	/* Enable Block-0 in LTR Block Select Register */
	bs_reg.m_blkEnable[ NLMRANGEMGR_REFAPP_BLOCK_0 ] = NLMDEV_ENABLE;
	if( NLMERR_OK != NlmDevMgr__LogicalTableRegisterWrite( dev_p,ltr_num,
							NLMDEV_BLOCK_SELECT_0_LTR, &bs_reg, &reason ) )
	{
		/* Error Mech */
		NlmCm__printf("Block Select#0 register write failed. Exiting...\n");
		return NLMERR_FAIL;
	}
	NlmCm__printf("\tBlock-0 is enabled in LTR Block Select Register...\n");

	/* Enable Blocks 0 and configure block width to 160b. */
	blk_num               = NLMRANGEMGR_REFAPP_BLOCK_0;
	bcr_reg.m_blockEnable = NLMDEV_ENABLE;
	bcr_reg.m_blockWidth  = NLMDEV_BLK_WIDTH_160;
	if( NLMERR_OK != NlmDevMgr__BlockRegisterWrite(dev_p, blk_num,
								NLMDEV_BLOCK_CONFIG_REG, &bcr_reg, &reason) )
	{
		NlmCm__printf("\n\t Block Config Reg Write failed with Block-%d", blk_num);
		return NLMERR_FAIL;
	}
	NlmCm__printf("\tBlock-0 is enabled and width set to 160b...\n");


	/* Configure Parallel Search Register#0 now.
	 * Block-0 is mapped to Parallel Search Result Port-0.
	 */
	ps_reg.m_psNum[ NLMRANGEMGR_REFAPP_BLOCK_0 ] = NLMDEV_PARALLEL_SEARCH_0;
	if( NLMERR_OK != ( NlmDevMgr__LogicalTableRegisterWrite( dev_p, ltr_num,
							NLMDEV_PARALLEL_SEARCH_0_LTR, &ps_reg, &reason ) ) )
	{
		/* Error Mech */
		NlmCm__printf("Parallel Search#0 register write failed. Exiting...\n");
		return NLMERR_FAIL;
	}
	NlmCm__printf("\tBlock-0 is mapped to PS#0...\n");

	/* Configure SB to Key mapping now. */
	sb_reg.m_keyNum[ (NLMRANGEMGR_REFAPP_BLOCK_0 / 4) ] = NLMDEV_KEY_0;
	if( NLMERR_OK != ( NlmDevMgr__LogicalTableRegisterWrite( dev_p, ltr_num,
							NLMDEV_SUPER_BLK_KEY_MAP_LTR, &sb_reg, &reason ) ) )
	{
		/* Error Mech */
		NlmCm__printf("Parallel Search#0 register write failed. Exiting...\n");
		return NLMERR_FAIL;
	}
	NlmCm__printf("\tKey#0 is mapped to Super Block#0...\n");

    /* Key#0 ise generated from Master Key and are searched in tbl-0.
    *
    * Master Key is constructed as follows:
    * Master_Key[ 319 :  160 ] : un-used bits, could be anything
    * Master_Key[ 159 : 156 ]  :  table id 0
    * Master_Key[ 155 : 80 ]   :   valid data
    * Master_Key[ 79   : 48 ]   :   un-used bits, could be anything
    * Master_Key[ 47   : 32 ]   :   Range-A
    * Master_Key[ 31   :   0 ]   :   32b valid data
    */

    /* Construction of search keys. LSB 160b from Master Key is formed as Key#0.
    *
    * Key#0 is constructed as follows:
    * Key#0, segment#0 :  Master_Key[ 79   :   0 ]
    * Key#0, segment#1 :  Master_Key[ 159 : 80 ]
    *
    * Key#0, segment#0[ Start Byte : Number of Bytes ]  :  [ 0 :   10 ]
    * Key#0, segment#1[ Start Byte : Number of Bytes ]  :  [ 10 : 10 ]
    */

	kcr_reg.m_startByteLoc[ 0 ] = 0;
	kcr_reg.m_numOfBytes[ 0 ]   = 10;
	kcr_reg.m_startByteLoc[ 1 ] = 10;
	kcr_reg.m_numOfBytes[ 1 ]   = 10;

	if( NLMERR_OK != NlmDevMgr__LogicalTableRegisterWrite( dev_p, ltr_num,
							NLMDEV_KEY_0_KCR_0_LTR, &kcr_reg, &reason))
	{
		/* Error Mech */
		NlmCm__printf("KPU#0 Key Construction Register#0 write failed. Exiting..\n");
		return NLMERR_FAIL;
	}
	NlmCm__printf("\n\tKey#0 is LSB 160b from Master Key\n");

	/* Configure searches with ranges now.
	  * Key#0 has one range-Range-A.
	  * In Master Key, Range-A is located at [ 47 : 32 ]. 8b MCOR encoded bits are inserted back
	  * at Key#0[ 79 : 72 ].
	  */
	srch_attrs = &refAppData_p->db_srchAttrs;
        for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS;keyNum++)
        {
            srch_attrs->m_keyInsert_startByte_rangeA[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
            srch_attrs->m_keyInsert_startByte_rangeB[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
            srch_attrs->m_keyInsert_startByte_rangeC[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
            srch_attrs->m_keyInsert_startByte_rangeD[keyNum] = NLMDEV_RANGE_DO_NOT_INSERT;
        }

	srch_attrs->m_extraction_startByte_rangeA = 4;
    	srch_attrs->m_keyInsert_startByte_rangeA[ NLMDEV_KEY_0 ] = 9;
	srch_attrs->m_rangeA_db = refAppData_p->db_p[0];

	if( NLMERR_OK != NlmRangeMgr__ConfigRangeMatching( refAppData_p->rangeMgr_p, ltr_num,
							srch_attrs, &reason))
	{
		/* Error Mech */
		NlmCm__printf("Range Matching register write failed. Exiting...\n");
		return NLMERR_FAIL;
	}
	NlmCm__printf("\n\textraction_startByte_rangeA = %d\n",
					srch_attrs->m_extraction_startByte_rangeA);
	NlmCm__printf("\t[Key#0] : keyInsert_startByte_rangeA = %d\n",
					srch_attrs->m_keyInsert_startByte_rangeA[ NLMDEV_KEY_0]);

	/* This application is not using any BMRs. BMR is disabled for the searches. Configure
	  * Miscellaneous Register's BMR Select to 7 for PS#0.
	  * Here, application should access the Shadow Memory of LTR Misc Register, update it
	  * and then use in the __Write. This is required as above __ConfigRangeMatching
	  * function also updates the LTR Misc Register (for m_extraction_startByte_ fields).
	  * If we don't use data from Shadow Memory, we will be over writing the value
	  * that was written by the above function.
	  */
	{
		NlmDevShadowDevice        *shadowDev_p;
		NlmDevMiscelleneousReg    *misReg_p;

		/* Get the Shadow Memory pointer first and then LTR Misc Register */
        shadowDev_p = dev_p->m_shadowDevice_p;
        misReg_p    = &shadowDev_p->m_ltr[ ltr_num ].m_miscelleneous;

        /* Update only fields we are interested in here */
        misReg_p->m_bmrSelect[ NLMDEV_PARALLEL_SEARCH_0 ] = 7;
        if( NLMERR_OK != ( NlmDevMgr__LogicalTableRegisterWrite( dev_p, ltr_num,
							NLMDEV_MISCELLENEOUS_LTR, misReg_p, &reason ) ) )
		{
			/* Error Mech */
			NlmCm__printf("Misc register write failed. Exiting...\n");

			return NLMERR_FAIL;
		}
	}

	return NLMERR_OK;

}

static
int NlmRangeMgrRefApp__AddRanges( NlmRangeMgrRefAppData *refAppData_p )
{
	/* {Range ID, Start Range, End Range, Pointer to encoded bitmaps} */
	NlmRange              db0_ranges[ NUM_OF_RANGES_PER_DATABASE ] = {
	                                                                                { 0, 1, 5, NULL             },
												 { 1, 4, 14, NULL           },
												 { 2, 10, 25, NULL         },
												 { 3, 1, 5, NULL             },
												 { 4, 500, 510, NULL     },
												 { 5, 200, 210, NULL     },
												 { 6, 150, 250, NULL     },
												 { 7, 1024, 1030, NULL }
											   };


	NlmRange  *range_p, *dbRange_p;
	NlmRangeDb  *db_p;
	NlmReasonCode reason;
	nlm_u32    range_id;
	nlm_u8     db_id;

	/* Start adding ranges into all databases */
	for( db_id = 0; db_id < NUM_OF_RANGE_DATABASES; db_id++ )
	{
		db_p = refAppData_p->db_p[ db_id ];
		dbRange_p = db0_ranges;

		for( range_id = 0; range_id < NUM_OF_RANGES_PER_DATABASE; range_id++ )
		{
			if( NLMERR_OK != NlmRangeMgr__AddRange( refAppData_p->rangeMgr_p,
													db_p,
													&dbRange_p[ range_id],
													&reason ) )
			{
				NlmCm__printf("\n\t Err: __AddRange of range_id = %d, db_id = %d failed. Reason Code -- %d",
                                                    range_id, db_id, reason);
				return NLMERR_FAIL;
			}

			/* Copy the range recently added into the local data strcture */
			range_p = &refAppData_p->db_ranges[db_id][range_id];
			NlmCm__memcpy( range_p, &dbRange_p[range_id], sizeof( NlmRange ) );
		}

		NlmCm__printf("\n\tAddition of [%d] ranges to db-%d complete...", range_id, db_id);

	}

	return NLMERR_OK;

}

static
int NlmRangeMgrRefApp__AddRecords( NlmRangeMgrRefAppData *refAppData_p )
{
	NlmRangeDb      *db0_p;
	NlmRangeEncoded *enc0_p;
	NlmDev       *dev_p = refAppData_p->dev_p;
	NlmDevABEntry abEntry_LSB_80Bits, abEntry_MSB_80Bits;
	NlmReasonCode   reason;
	nlm_u32    range_id0, tbl_id, lsb_numBits, mask_val;
	nlm_u16    block_index = 0, iter, iter1;
	nlm_u8     block_num, data_byte;
	nlm_u8     lsb_dcStart, lsb_dcEnd, mcor_range_dcStart, mcor_range_dcEnd;
	nlm_u8     lsb_stStart, lsb_stEnd;

	NlmCm__printf("\n");

	/* Two record with different range is added into table tbl-0.
	  * tbl-0 record is created with encodings of ranges  from db0. range id = 0 and 4 are picked
	  * from db0 and corresponding encodings are obtained from
	  * RangeMgr. These encodings are stitched at following locations in the tbl-0 record.
	  * The format of the tbl-0 records is as follows:
	  *
	  * data[159 : 156] : table id 0
	  * mask[159 : 156] : 0x0, exact match
	  *
	  * data[155 : 80] : 76b valid data
	  * mask[155 : 80] : 76b 0x0, exact match
	  *
	  * data[79 : 72]  :  8b encoding of range id = 0/1 from db#0 is stitched here (Range-A)
	  * mask[79 :  72]  : 8b mask returned by RM for the above range id is stitched here
	  *
	  * data[71 : 48]  :  un-used bits, could be anything
	  * mask[71 :  48]  : 0xFF, don't cares
	  *
	  * data[47  :   32]  : 16b corresponding to range, range id = 0/4 from db#0 (Range-A)
	  * mask[47  :   32]  : 0xFF, don't cares since MCOR only encoding is performed
	  *
	  * data[31  :     0]  : 32b valid data
	  * mask[31  :     0]  : 32b 0x0, exact match
	  *
	  */
	for( iter = 0; iter < 2; iter++)
	{
	       db0_p = refAppData_p->db_p[ 0 ];
              block_num   = NLMRANGEMGR_REFAPP_BLOCK_0;
              tbl_id    = NLMRANGEMGR_REFAPP_TBLID_0;
		if( iter == 0 )
		{
			range_id0 = 0;
		}
		else
		{
			range_id0 = 4;
		}

		/* Get the range encodings now */
		if( NULL == (enc0_p = NlmRangeMgr__GetRangeEncoding( refAppData_p->rangeMgr_p, db0_p,
											range_id0, &reason )) )
		{

			NlmCm__printf("\n\tCould not get encodings for rangeId = %u, dbId = %d. Reason Code -- %d\n",
								range_id0, db0_p->m_id, reason);

			return NLMERR_FAIL;
		}


			/* We are ready to write 160b ACL entry into Block-0 (tbl-0 resides in Block-0).
			  * At a time only 80 bits can be written into Block. In a 160b ACL entry, LSB 80 bits
			  * are stored in the lower address and MSB 80 bits are stored in the next address.
			  *
			  * a random data of bytes sequence a9a9... is used as data and range encodings
			  * are stitched at appropriate locations.
			  */
			data_byte = 0xa9;

			/* data[71 : 48] are un-used bits (don't cares).  */
		  	lsb_dcStart = 48;
		  	lsb_dcEnd   = 71;

		  	/* MCOR Range encoding is stitched at [79 : 72] at LSB 80b Block Entries. */
		  	lsb_stStart =  72;
		  	lsb_stEnd   = 79;

                     /* With MCOR only, range bits should be modified to don't cares */
                     mcor_range_dcStart = 32;
                     mcor_range_dcEnd =  47;


		    /* These many bits are don't cares in the 160b record. */
		    lsb_numBits = lsb_dcEnd - lsb_dcStart + 1;
                  mask_val    = 0xFFFFFFFF;

		NlmCm__memset( abEntry_LSB_80Bits.m_data, data_byte, NLMDEV_AB_WIDTH_IN_BYTES );
		NlmCm__memset( abEntry_LSB_80Bits.m_mask, 0x0,  NLMDEV_AB_WIDTH_IN_BYTES );
		NlmCm__memset( abEntry_MSB_80Bits.m_data, data_byte, NLMDEV_AB_WIDTH_IN_BYTES );
		NlmCm__memset( abEntry_MSB_80Bits.m_mask, 0x0,  NLMDEV_AB_WIDTH_IN_BYTES );

		/*
		  * WriteBitsInRegs expects a 10 byte (80b) array and writes a value of maximum 32b from
		  * 'end' to 'start'. Bit-79 is at MSB byte (array index = 0) and LSB Bit-0 is at LSB byte (array
		  * index = 9)
		  */
		WriteBitsInRegs(abEntry_LSB_80Bits.m_mask , lsb_dcEnd, lsb_dcStart, ~(mask_val << lsb_numBits) );

            /* Stich don't cares for range bits since encoding is a MCOR only */
               lsb_numBits = mcor_range_dcEnd - mcor_range_dcStart + 1;
               mask_val    = 0xFFFFFFFF;
               WriteBitsInRegs(abEntry_LSB_80Bits.m_mask , mcor_range_dcEnd, mcor_range_dcStart, ~(mask_val << lsb_numBits) );

		/* Insert table id at [159 : 156] = [79: 76] within 80b boundary. */
		WriteBitsInRegs( abEntry_MSB_80Bits.m_data, 79, 76, tbl_id );

		NlmCm__printf("\n\tWriting tbl#%d records into Block#%d\n", tbl_id, block_num);

		/* Now start stitching the encoded ranges, insert entries into Block. */
		for( iter1 = 0; iter1 < enc0_p->m_num_entries; iter1++ )
		{
			WriteBitsInRegs( abEntry_LSB_80Bits.m_data, lsb_stEnd, lsb_stStart,
							((enc0_p->m_entries_p) + iter1)->m_data );
			WriteBitsInRegs( abEntry_LSB_80Bits.m_mask, lsb_stEnd, lsb_stStart,
							((enc0_p->m_entries_p) + iter1)->m_mask );

			if( NLMERR_OK != NlmDevMgr__ABEntryWrite( dev_p, block_num, block_index,
															&abEntry_LSB_80Bits, NLMDEV_DM,
															&reason ) )
			{
				/* Error Mech */
				NlmCm__printf("AB Write failed. Exiting...\n");
				return NLMERR_FAIL;
			}

			/* Write MSB 80 bits to next location */
			if( NLMERR_OK != NlmDevMgr__ABEntryWrite( dev_p, block_num, ( block_index + 1),
															&abEntry_MSB_80Bits, NLMDEV_DM,
															&reason ) )
			{
				/* Error Mech */
				NlmCm__printf("AB Write failed. Exiting...\n");
				return NLMERR_FAIL;
			}
			NlmCm__printf("\t160b record with range id-%d added at address-%d\n", range_id0, block_index);

			/* Increment ab_index by 100 to add the next record.   */
			block_index += 100;
		}

	}

    return NLMERR_OK;
}

static
int NlmRangeMgrRefApp__PerformSearches(
                    NlmRangeMgrRefAppData *refAppData_p)
{
	NlmDevMgr    *devMgr_p = refAppData_p->devMgr_p;
	NlmDevCtxBufferInfo  cb_data;
	NlmDevCmpRslt        search_result;
	NlmReasonCode  reason;
	nlm_u16    range, range_id;
	nlm_u8     db_id = 0, ltr_num, iter;

	/* Construct Master Key now for Compare1 operation.
	 *
	 * Key#0 is generated from Master Key and searched in tbl-0.
	 *
	 * Master Key is constructed as follows:
       * Master_Key[ 319 : 160 ] :  un-used bits, could be anything
       *
       * Master_Key[ 159 : 156 ]  :  table id 0
       * Master_Key[ 155 : 80 ] :   0xa9a9a9....
       * Master_Key[ 79 : 48 ] :   un-used bits, could be anything
       * Master_Key[ 47   : 32 ]   :   range value from db#0, Range-A
       * Master_Key[ 31   :   0 ]   :   0xa9a9a9....
       *
	 * LSB 160b are given as part of Compare1 instruction which is used for search using Key#0.
	 */
	 /* Two Compares are performed - First CMP1 with range id 0 and second with range id 4.*/
	for(range_id = 0; range_id < 8; range_id+=4 )
    	{
    		/* Using the start range value here. Any value within the range can be used */
    		range = refAppData_p->db_ranges[db_id][range_id].m_start;

    	       /* Construct LSB 160 bits of Master Key now. */
        	NlmCm__memset( &cb_data.m_data, 0xa9, (2 * NLMDEV_CB_WIDTH_IN_BYTES) );
        	cb_data.m_cbStartAddr = 0;
        	cb_data.m_datalen     = 20; /* 160b data */

        	/* Now stitch range[0] at [47:32].  */
        	WriteBitsInRegs( &cb_data.m_data[10], 47, 32, range );

                /* Insert tbl-id = 0 at [159:156] */
                WriteBitsInRegs( &cb_data.m_data[0], 79, 76, NLMRANGEMGR_REFAPP_TBLID_0 );

        	ltr_num = 0;
        	NlmCm__printf("\n\tFiring Compare1 instruction now with range id-%d (range value =%d)...\n", range_id, range);
        	if( NLMERR_OK != NlmDevMgr__Compare1( devMgr_p, ltr_num, &cb_data,
        												&search_result, &reason ) )
        	{
        		/* Error Mech */
        		NlmCm__printf("Compare1 Instruction failed. Exiting...\n");
        		return NLMERR_FAIL;
        	}
        	else
        	{
        		NlmCm__printf("\n\tSearch Results\n");
        		NlmCm__printf("\t--------------\n");

        		/* Hit is expected from PS#0 and index = 0/100. */
        		for( iter = 0; iter < NLMDEV_NUM_PARALLEL_SEARCHES; iter++ ){
        			NlmCm__printf("\tParallel Search# %d: ", iter);
        			if( (Nlm11kDevMissHit)NLMDEV_HIT == search_result.m_hitOrMiss[ iter ] ){
        				NlmCm__printf("Found a match in deviceID %u, database index %d\n",
        							search_result.m_hitDevId[ iter ], search_result.m_hitIndex[ iter ] );
        			}
        			else
        				NlmCm__printf("MISS\n");
        		}
        	}
       }

    return NLMERR_OK;
}

int nlmrangemgr_refapp3_main(int argc, char	*argv[])
{
    NlmRangeMgrRefAppData refAppData;
    NlmRangeDb   *db_p;
    NlmReasonCode  reason;
	nlm_32	break_alloc_id = -1;
	nlm_u8  db_id;

	(void)argc;
	(void)argv;

	NlmCmDebug__Setup( break_alloc_id, NLMCM_DBG_EBM_ENABLE );

	NlmCm__printf ("\n\t Range Manager Application Reference Code Using \n");
	NlmCm__printf ("\t Range Manager Module. \n\n\n");

	/* Initialize Search system environment first */
	if( NLMERR_OK != NlmRangeMgrRefApp__InitEnvironment( &refAppData ) )
	{
		NlmCm__printf("Err: Enviroment Initialization failed. Exiting...\n");

		return NLMERR_FAIL;
	}

	/* Initialize Device Manager in Standard Speed mode and add a device */
	if( NLMERR_OK != NlmRangeMgrRefApp__InitDevMgr( &refAppData ) )
	{
		NlmCm__printf("Err: Device Manager Initialization failed. Exiting...\n");

		return NLMERR_FAIL;
	}

	/* Device Manager Init Complete, initialize Range Manager now */
	if( NLMERR_OK != NlmRangeMgrRefApp__InitRangeMgr( &refAppData ) )
	{
		NlmCm__printf("Err: Range Manager Initialization failed. Exiting...\n");

		return NLMERR_FAIL;
	}

	/* Configure LTR searches now */
	if( NLMERR_OK != NlmRangeMgrRefApp__ConfigSearches( &refAppData ) )
	{
		NlmCm__printf("Err: LTR Search configuration failed. Exiting...\n");

		return NLMERR_FAIL;
	}

	if( NLMERR_OK != NlmRangeMgrRefApp__AddRanges( &refAppData ) )
	{
		NlmCm__printf("Err: Addition of Ranges failed. Exiting...\n");

		return NLMERR_FAIL;
	}

	/* Let Range Manager do range compression now */
	for( db_id = 0; db_id < NUM_OF_RANGE_DATABASES; db_id++ )
	{
		db_p = refAppData.db_p[ db_id ];

		if( NLMERR_OK != NlmRangeMgr__CreateEncodings( refAppData.rangeMgr_p, db_p, &reason ) )
		{
			NlmCm__printf("\n\t CreateEncodings failed on db-%d. Reason Code -- %d. Exiting...\n",
						    db_id, reason);

			return NLMERR_FAIL;
		}
	}

	/* Add records into the device now. */
	if( NLMERR_OK != NlmRangeMgrRefApp__AddRecords( &refAppData ) )
	{
		NlmCm__printf("Err: Addition of Records failed. Exiting...\n");

		return NLMERR_FAIL;
	}

	/* Do a search now. */
	if( NLMERR_OK != NlmRangeMgrRefApp__PerformSearches(&refAppData))
	{
		NlmCm__printf("Err: Search of Records failed. Exiting...\n");

		return NLMERR_FAIL;
	}

    /* Destroy Device Manager */
	NlmRangeMgrRefApp__DestroyDevMgr( &refAppData );

	/* Destroy the Enviroment settings. */
	NlmRangeMgrRefApp__DestroyEnvironment( &refAppData );

	/* destroy Range Manager instance and databases*/

	NlmRangeMgrRefApp__DestroyRangeMgr( &refAppData );

	if(NlmCmDebug__IsMemLeak())
	{
        NlmCm__printf("Memory Leak\n");
	}

	NlmCm__printf("\n\tProgram Completed Successfully...\n");

	return 0;
}

