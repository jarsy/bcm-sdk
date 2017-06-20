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
 * $Id: nlmrangemgr_refapp.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2007 Broadcom Corp.
 * All Rights Reserved.$
 */

#include "nlmrangemgr_refapp.h"
#ifdef NLM_XLP
#include "nlmxlpxpt.h"
#endif

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
	refAppData_p->request_queue_len = 0;
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
			/* This Processor being a serial device, speed mode and rbus mode can be ignored. Only operation mode
			 * currently supported is NLMDEV_OPR_STANDARD.
			 */
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
			NlmCm__printf("\n\tXLP Transport Interface Created Successfully\n");

			 if(refAppData_p->xpt_p == NULL)
			 	return NLMERR_FAIL;
			 break;
		}
#endif
#endif /* NLMPLATFORM_BCM */
                case IFTYPE_BCM_CALADAN3:
                    refAppData_p->xpt_p = soc_sbx_caladan3_etu_xpt_create(0,
									  NLM_DEVTYPE_2, 0, refAppData_p->request_queue_len,
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

static int NlmRangeMgrRefApp__DestroyEnvironment( NlmRangeMgrRefAppData *refAppData_p )
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
#endif /* NLMPLATFORM_BCM */
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
	NlmRangeEncodingType encodingType[4] = { NLM_RANGE_3B_ENCODING,
											 NLM_RANGE_3B_ENCODING,
											 NLM_RANGE_3B_ENCODING,
											 NLM_RANGE_2B_ENCODING
											};

	/* Range DB-0: m_num_bits = 32, m_valid_bits = 16   (DIRPE + MCOR)
	 * Range DB-1: m_num_bits = 32, m_valid_bits = 12   (DIRPE + MCOR)
	 * Range DB-2: m_num_bits = 24, m_valid_bits = 16   (DIRPE       )
	 * Range DB-3: m_num_bits = 16, m_valid_bits = 16   (PE          )
	 */
	NlmRangeDbAttrSet          db_attrs[NUM_OF_RANGE_DATABASES] = {
	                                                                {32, 16, NLM_RANGE_3B_ENCODING},
	                                                                {32, 12, NLM_RANGE_3B_ENCODING},
	                                                                {24, 16, NLM_RANGE_3B_ENCODING},
	                                                                {16, 16, NLM_RANGE_2B_ENCODING}
                                                                   };
	NlmDev                 *dev_p = refAppData_p->dev_p;
	NlmDevConfigReg         dc_reg;
	nlm_u8                     db_id;
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

	/* Now create Range databases. 4 databases are created. Database attributes are specified during
	 * __CreateDb itself. They are:
	 *  (1) m_num_bits, number of bits available for range encoding
	 *  (2) m_valid_bits, how many (LSB) bits of 16b range field are valid.
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
	 * MCOR encoding is possible with the database. Out of 4 databases, only db-0, db-1
	 * use MCOR (becuase there are 32 bits available for encodings). Range-A is assigned
	 * to db-0 and Range-B is assigned to db-1.
	 */
	if( NLMERR_OK != NlmRangeMgr__AssignRange( refAppData_p->rangeMgr_p,
	                                           refAppData_p->db_p[0],
											   NLM_RANGE_TYPE_A,
											   &reason ) )
    {
    	NlmCm__printf("\n\tRange-A assignment to db-0 failed. Reason Code = %d\n", reason);

       	return NLMERR_FAIL;
	}

	if( NLMERR_OK != NlmRangeMgr__AssignRange( refAppData_p->rangeMgr_p,
	                                           refAppData_p->db_p[1],
											   NLM_RANGE_TYPE_B,
											   &reason ) )
    {
    	NlmCm__printf("\n\tRange-B assignment to db-1 failed. Reason Code = %d\n", reason);

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
	nlm_u8                       ltr_num, blk_num;

	/* Two "tables" tbl-0 and tbl-1 of 160b width are created. These tables are searched in
	  * in Parallel using Key#0 and Key#1. Key#0 is searched in tbl-0 producing result
	  * onto Parallel Search Result Port-0 (PS#0) and Key#1 is searched in tbl-1 producing
	  * result onto Parallel Search Result Port-1 (PS#1).
	  *
	  * tbl-0 records reside in Block-0 (Super Block-0) and tbl-1 records in Block-4 (Super Block-1).
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

	/* Enable Block-0, Block-4 in LTR Block Select Register */
	bs_reg.m_blkEnable[ NLMRANGEMGR_REFAPP_BLOCK_0 ] = NLMDEV_ENABLE;
	bs_reg.m_blkEnable[ NLMRANGEMGR_REFAPP_BLOCK_4 ] = NLMDEV_ENABLE;
	if( NLMERR_OK != NlmDevMgr__LogicalTableRegisterWrite( dev_p,ltr_num,
							NLMDEV_BLOCK_SELECT_0_LTR, &bs_reg, &reason ) )
	{
		/* Error Mech */
		NlmCm__printf("Block Select#0 register write failed. Exiting...\n");

		return NLMERR_FAIL;
	}
	NlmCm__printf("\tBlock-0 and Block-4 enabled in LTR Block Select Register...\n");

	/* Enable Blocks 0 and 4 and configure block width to 160b. */
	blk_num               = NLMRANGEMGR_REFAPP_BLOCK_0;
	bcr_reg.m_blockEnable = NLMDEV_ENABLE;
	bcr_reg.m_blockWidth  = NLMDEV_BLK_WIDTH_160;
	if( NLMERR_OK != NlmDevMgr__BlockRegisterWrite(dev_p, blk_num,
								NLMDEV_BLOCK_CONFIG_REG, &bcr_reg, &reason) )
	{
		NlmCm__printf("\n\t Block Config Reg Write failed with Block-%d", blk_num);

		return NLMERR_FAIL;
	}
	NlmCm__printf("\tBlock-0 enabled and width set to 160b...\n");

	blk_num = NLMRANGEMGR_REFAPP_BLOCK_4;
	if( NLMERR_OK != NlmDevMgr__BlockRegisterWrite(dev_p, blk_num,
								NLMDEV_BLOCK_CONFIG_REG, &bcr_reg, &reason) )
	{
		NlmCm__printf("\n\t Block Config Reg Write failed with Block-%d", blk_num);

		return NLMERR_FAIL;
	}
	NlmCm__printf("\tBlock-4 enabled and width set to 160b...\n");


	/* Configure Parallel Search Register#0 now.
	 * Block-0 is mapped to Parallel Search Result Port-0 and Block-4 is mapped to
	 * Parallel Search Result Port-1.
	 */
	ps_reg.m_psNum[ NLMRANGEMGR_REFAPP_BLOCK_0 ] = NLMDEV_PARALLEL_SEARCH_0;
	ps_reg.m_psNum[ NLMRANGEMGR_REFAPP_BLOCK_4 ] = NLMDEV_PARALLEL_SEARCH_1;
	if( NLMERR_OK != ( NlmDevMgr__LogicalTableRegisterWrite( dev_p, ltr_num,
							NLMDEV_PARALLEL_SEARCH_0_LTR, &ps_reg, &reason ) ) )
	{
		/* Error Mech */
		NlmCm__printf("Parallel Search#0 register write failed. Exiting...\n");

		return NLMERR_FAIL;
	}
	NlmCm__printf("\tBlock-0 mapped to PS#0 and Block-4 mapped to PS#1...\n");

	/* tbl-0 is in Block-0 (SB#0) and tbl-1 in Block-4 (SB#1). Key#0 is searched in tbl-0 and
	  * Key#1 is searched in tbl-1. Configure SB to Key mapping now.
	  */
	sb_reg.m_keyNum[(NLMRANGEMGR_REFAPP_BLOCK_0 / 4) ] = NLMDEV_KEY_0;
	sb_reg.m_keyNum[(NLMRANGEMGR_REFAPP_BLOCK_4 / 4)] = NLMDEV_KEY_1;

	if( NLMERR_OK != ( NlmDevMgr__LogicalTableRegisterWrite( dev_p, ltr_num,
							NLMDEV_SUPER_BLK_KEY_MAP_LTR, &sb_reg, &reason ) ) )
	{
		/* Error Mech */
		NlmCm__printf("Parallel Search#0 register write failed. Exiting...\n");

		return NLMERR_FAIL;
	}
	NlmCm__printf("\tKey#0 mapped to Super Block#0 and Key#1 mapped to Super Block#1...\n");

	/* Master Key has 4 ranges. Two keys Key#0 and Key#1 are generated from
	  * Master Key and are searched in parallel in tbl-0 and tbl-1 respectively.
	  *
	  * Master Key is constructed as follows:
         * Master_Key[ 319 : 316 ] :  table id 1
         * Master_Key[ 315 :  288 ] : un-used bits, could be anything
         * Master_Key[ 287 :  272 ] : Range-D
         * Master_Key[ 271 :  240 ] : 32b valid data
         * Master_Key[ 239 : 208]   :  un-used bits, could be anything
         * Master_Key[ 207 : 192 ]  :  Range-C
         * Master_Key[ 192 : 160 ]  :  32b valid data
         *
         * Master_Key[ 159 : 156 ]  :  table id 0
         * Master_Key[ 155 : 128 ] :   un-used bits, could be anything
         * Master_Key[ 127 : 112 ] :   Range-B
         * Master_Key[ 111 : 80 ]   :   32b valid data
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
	  * Key#0, segment#0[ Start Byte : Number of Bytes ]  :  [ 0 :   9 ]
	  * Key#0, segment#1[ Start Byte : Number of Bytes ]  :  [ 10 : 9 ]
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

	/* Master_Key[ 319: 160 ] is formed as Key#1
	  *
	  * Key#1 is constructed as follows:
	  * Key#1, segment#0 :  Master_Key[ 239 : 160 ]
	  * Key#1, segment#1 :  Master_Key[ 319 : 240 ]
	  *
	  * Key#1, segment#0[ Start Byte : Number of Bytes ]  :  [ 20 :   9 ]
	  * Key#1, segment#1[ Start Byte : Number of Bytes ]  :  [ 30 :   9 ]
	  */

	kcr_reg.m_startByteLoc[ 0 ] = 20;
	kcr_reg.m_numOfBytes[ 0 ]   =  10;
	kcr_reg.m_startByteLoc[ 1 ] = 30;
	kcr_reg.m_numOfBytes[ 1 ]   =  10;

	if( NLMERR_OK != NlmDevMgr__LogicalTableRegisterWrite( dev_p, ltr_num,
							NLMDEV_KEY_1_KCR_0_LTR, &kcr_reg, &reason))
	{
		/* Error Mech */
		NlmCm__printf("KPU#1 Key Construction Register#0 write failed. Exiting..\n");

		return NLMERR_FAIL;
	}
	NlmCm__printf("\tKey#1 is MSB 160b from Master Key\n");

	/* Configure searches with ranges now.
	  * Key#0 has two ranges, Range-A and Range-B.
	  * In Master Key, Range-A is located at [ 47 : 32 ]. 32b encoded bits are inserted back
	  * at Key#0[ 63 : 32 ].
	  * In Master Key, Range-B is located at [ 123 : 112 ] (Range-B is 12b). 32b encoded bits
	  * are inserted back at Key#0[ 143 : 112 ].
	  *
	  * Key#1 has two ranges, Range-C and Range-D.
	  * In Master Key, Range-C is located at [ 207 : 192 ] = Byte 24. 24b encoded bits are inserted back
	  * at Key#1[ 55 : 32 ] = Byte 4.
	  * In Master Key, Range-D is located at [ 287 : 272 ] = Byte 34. As there are no spare bits
	  * available for encoding, only Prefix Encoding is done on this database entries (db#3).
	  * Hence, we don't want Range Block to insert back the encoded data into the search key and
	  * pass through the key data as is.
	  */
	srch_attrs = &refAppData_p->db_srchAttrs;

	srch_attrs->m_extraction_startByte_rangeA = 4;
	srch_attrs->m_extraction_startByte_rangeB = 14;
	srch_attrs->m_keyInsert_startByte_rangeA[ NLMDEV_KEY_0 ] = 4;
	srch_attrs->m_keyInsert_startByte_rangeB[ NLMDEV_KEY_0 ] = 14;
	srch_attrs->m_keyInsert_startByte_rangeC[ NLMDEV_KEY_0 ] = NLMDEV_RANGE_DO_NOT_INSERT;
	srch_attrs->m_keyInsert_startByte_rangeD[ NLMDEV_KEY_0 ] = NLMDEV_RANGE_DO_NOT_INSERT;
	srch_attrs->m_rangeA_db = refAppData_p->db_p[0];
	srch_attrs->m_rangeB_db = refAppData_p->db_p[1];

	srch_attrs->m_extraction_startByte_rangeC = 24;
	srch_attrs->m_extraction_startByte_rangeD = 34;
	srch_attrs->m_keyInsert_startByte_rangeC[ NLMDEV_KEY_1] = 4;
	srch_attrs->m_keyInsert_startByte_rangeD[ NLMDEV_KEY_1 ] = NLMDEV_RANGE_DO_NOT_INSERT; /* don't insert encodings */
	srch_attrs->m_keyInsert_startByte_rangeA[ NLMDEV_KEY_1 ] = NLMDEV_RANGE_DO_NOT_INSERT;
	srch_attrs->m_keyInsert_startByte_rangeB[ NLMDEV_KEY_1 ] = NLMDEV_RANGE_DO_NOT_INSERT;
	srch_attrs->m_rangeC_db = refAppData_p->db_p[2];
	srch_attrs->m_rangeD_db = refAppData_p->db_p[3];

	if( NLMERR_OK != NlmRangeMgr__ConfigRangeMatching( refAppData_p->rangeMgr_p, ltr_num,
							srch_attrs, &reason))
	{
		/* Error Mech */
		NlmCm__printf("Range Matching register write failed. Exiting...\n");

		return NLMERR_FAIL;
	}
	NlmCm__printf("\n\textraction_startByte_rangeA = %d,  extraction_startByte_rangeB = %d\n",
					srch_attrs->m_extraction_startByte_rangeA,
					srch_attrs->m_extraction_startByte_rangeB);
	NlmCm__printf("\textraction_startByte_rangeC = %d, extraction_startByte_rangeD = %d\n",
					srch_attrs->m_extraction_startByte_rangeC,
					srch_attrs->m_extraction_startByte_rangeD);
	NlmCm__printf("\t[Key#0] : keyInsert_startByte_rangeA = %d, keyInsert_startByte_rangeB = %d\n",
					srch_attrs->m_keyInsert_startByte_rangeA[ NLMDEV_KEY_0],
					srch_attrs->m_keyInsert_startByte_rangeB[ NLMDEV_KEY_0]);
	NlmCm__printf("\t[Key#1] : keyInsert_startByte_rangeC = %d, keyInsert_startByte_rangeD = %d\n",
					srch_attrs->m_keyInsert_startByte_rangeC[ NLMDEV_KEY_1],
					srch_attrs->m_keyInsert_startByte_rangeD[ NLMDEV_KEY_1]);

	/* This application is not using any BMRs. BMR is disabled for the searches. Configure
	  * Miscellaneous Register's BMR Select to 7 for PS#0 and PS#1.
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
        misReg_p->m_bmrSelect[ NLMDEV_PARALLEL_SEARCH_1 ] = 7;
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
	NlmRange              db0_ranges[ NUM_OF_RANGES_PER_DATABASE ] = { { 0, 1, 5, NULL          },
												 { 1, 4, 14, NULL         },
												 { 2, 10, 25, NULL        },
												 { 3, 1, 5, NULL          },
												 { 4, 500, 510, NULL      },
												 { 5, 200, 210, NULL      },
												 { 6, 150, 250, NULL      },
												 { 7, 1024, 1030, NULL    },
												 { 8, 505, 510, NULL      },
												 { 9, 1000, 1050, NULL    },
											   };

	NlmRange              db1_ranges[ NUM_OF_RANGES_PER_DATABASE ] = { { 0, 1, 5, NULL          },
												 { 1, 3, 13, NULL         },
												 { 2, 9, 24, NULL         },
												 { 3, 1, 5, NULL          },
												 { 4, 515, 525, NULL      },
												 { 5, 200, 235, NULL      },
												 { 6, 150, 250, NULL      },
												 { 7, 1050, 1070, NULL    },
												 { 8, 500, 510, NULL      },
												 { 9, 1000, 1005, NULL    },
											   };

	NlmRange              db2_ranges[ NUM_OF_RANGES_PER_DATABASE ] = { { 0, 100, 105, NULL          },
												 { 1, 3097, 3113, NULL      },
												 { 2, 91, 124, NULL         },
												 { 3, 1024, 1055, NULL      },
												 { 4, 5159, 5259, NULL      },
												 { 5, 2001, 2035, NULL      },
												 { 6, 1500, 2500, NULL      },
												 { 7, 10509, 10708, NULL    },
												 { 8, 5500, 6510, NULL      },
												 { 9, 11000, 12005, NULL    },
											   };

	NlmRange              db3_ranges[ NUM_OF_RANGES_PER_DATABASE ] = { { 0, 11, 25, NULL          },
												 { 1, 37, 43, NULL        },
												 { 2, 99, 240, NULL       },
												 { 3, 1024, 1099, NULL    },
												 { 4, 5105, 5295, NULL    },
												 { 5, 100, 235, NULL      },
												 { 6, 1550, 2950, NULL    },
												 { 7, 1050, 1090, NULL    },
												 { 8, 50000, 55510, NULL  },
												 { 9, 10004, 10605, NULL  },
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

		if( db_id == 0 )
			dbRange_p = db0_ranges;
		else if( db_id == 1 )
			dbRange_p = db1_ranges;
		else if( db_id == 2 )
			dbRange_p = db2_ranges;
		else
			dbRange_p = db3_ranges;

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
	NlmRangeDb      *db0_p, *db1_p;
	NlmRangeEncoded *enc0_p, *enc1_p;
	NlmDev       *dev_p = refAppData_p->dev_p;
	NlmDevABEntry abEntry_LSB_80Bits, abEntry_MSB_80Bits;
	NlmReasonCode   reason;
	nlm_u32    range_id0, range_id1, tbl_id, lsb_numBits, msb_numBits, mask_val;
	nlm_u16    block_index, iter, iter0, iter1;
	nlm_u8     block_num, data_byte;
	nlm_u8     lsb_dcStart, lsb_dcEnd, msb_dcStart, msb_dcEnd;
	nlm_u8     lsb_stStart, lsb_stEnd, msb_stStart, msb_stEnd;

	NlmCm__printf("\n");

	/* we have two tables tbl-0 and tbl-1. A single record with 2 ranges is added into each table.
	  * tbl-0 record is created with encodings of ranges  from db0 and db1. range id = 0 is picked
	  * from db0, range id = 1 is picked from db1 and corresponding encodings are obtained from
	  * RangeMgr. These encodings are stitched at following locations in the tbl-0 record.
	  * The format of the tbl-0 records is as follows:
	  *
	  * data[159 : 156] : table id 0
	  * mask[159 : 156] : 0x0, exact match
	  *
	  * data[155 : 144] : un-used bits, could be anything
	  * mask[155 : 144] : 0xFF, don't cares
	  *
	  * data[143 : 112] : 32b encoding of range id = 1 from db#1 is stitched here (Range-B)
	  * mask[143 : 112] : 32b mask returned by RM for the above range id is stitched here
	  *
	  * data[111 :  80]  : 32b valid data
	  * mask[111 :  80]  : 32b 0x0, exact match
	  *
	  * data[79  :   64]  : un-used bits, could be anything
	  * mask[79  :   64]  : 0xFF, don't cares
	  *
	  * data[63  :   32]  : 32b encoding of range id = 0 from db#0 is stitched here (Range-A)
	  * mask[63  :   32]  : 32b mask returned by RM for the above range id is stitched here
	  *
	  * data[31  :     0]  : 32b valid data
	  * mask[31  :     0]  : 32b 0x0, exact match
	  *
	  * tbl-1 record is created with encodings of ranges  from db2 and db3. range id = 2 is picked
	  * from db2, range id = 3 is picked from db3 and corresponding encodings are obtained from
	  * RangeMgr. These encodings are stitched at following locations in the tbl-1 record.
	  * The format of the tbl-1 records is as follows:
	  *
	  * data[159 : 156] : table id 1
	  * mask[159 : 156] : 0x0, exact match
	  *
	  * data[155 : 128] : un-used bits, could be anything
	  * mask[155 : 128] : 0xFF, don't cares
	  *
	  * data[127 : 112] : 16b encoding of range id = 3 from db#3 is stitched here (Range-D)
	  * mask[127 : 112] : 16b mask returned by RM for the above range id is stitched here
	  *
	  * data[111 :  80]  : 32b valid data
	  * mask[111 :  80]  : 32b 0x0, exact match
	  *
	  * data[79  :   56]  : un-used bits, could be anything
	  * mask[79  :   56]  : 0xFF, don't cares
	  *
	  * data[55  :   32]  : 24b encoding of range id = 2 from db#2 is stitched here (Range-C)
	  * mask[55  :   32]  : 24b mask returned by RM for the above range id is stitched here
	  *
	  * data[31  :     0]  : 32b valid data
	  * mask[31  :     0]  : 32b 0x0, exact match
	  */
	for( iter = 0; iter < 2; iter++)
	{
		if( iter == 0 )
		{
			db0_p = refAppData_p->db_p[ 0 ];
			db1_p = refAppData_p->db_p[ 1 ];

			range_id0 = 0;
			range_id1 = 1;
		}
		else
		{
			db0_p = refAppData_p->db_p[ 2 ];
			db1_p = refAppData_p->db_p[ 3 ];

			range_id0 = 2;
			range_id1 = 3;
		}

		/* Get the range encodings now */
		if( NULL == (enc0_p = NlmRangeMgr__GetRangeEncoding( refAppData_p->rangeMgr_p, db0_p,
											range_id0, &reason )) )
		{

			NlmCm__printf("\n\tCould not get encodings for rangeId = %u, dbId = %d. Reason Code -- %d\n",
								range_id0, db0_p->m_id, reason);

			return NLMERR_FAIL;
		}

		/* Get the range encodings now */
		if( NULL == (enc1_p = NlmRangeMgr__GetRangeEncoding( refAppData_p->rangeMgr_p, db1_p,
											range_id1, &reason )) )
		{

			NlmCm__printf("\n\tCould not get encodings for rangeId = %u, dbId = %d. Reason Code -- %d\n",
								range_id1, db1_p->m_id, reason);

			return NLMERR_FAIL;
		}


		if( iter == 0 ) /* tbl-0, Block-0 */
		{
			/* We are ready to write 160b ACL entry into Block-0 (tbl-0 resides in Block-0).
			  * At a time only 80 bits can be written into Block. In a 160b ACL entry, LSB 80 bits
			  * are stored in the lower address and MSB 80 bits are stored in the next address.
			  *
			  * a random data of bytes sequence a9a9... is used as data and range encodings
			  * are stitched at appropriate locations.
			  */
			tbl_id    = NLMRANGEMGR_REFAPP_TBLID_0;
			data_byte = 0xa9;

			/* data[79 : 64] and data[ 155 : 144] are un-used bits (don't cares). [79 : 64] is in abEntry_LSB_
		  	  * and [154 : 144] is in abEntry_MSB_.
		  	  */
		  	lsb_dcStart = 64;
		  	lsb_dcEnd   = 79;

		  	msb_dcStart = 144 - 80;
		  	msb_dcEnd   = 155 - 80;

		  	/* Range encodings are stitched at [63 : 32] at both LSB 80b and MSB 80b Block Entries. */
		  	lsb_stStart = msb_stStart = 32;
		  	lsb_stEnd   = msb_stEnd   = 63;

		  	block_num   = NLMRANGEMGR_REFAPP_BLOCK_0;
		}
		else /* tbl-1, Block-4 */
		{
			/* We are ready to write 160b ACL entry into Block-4 (tbl-1 resides in Block-4).
			  * At a time only 80 bits can be written into Block. In a 160b ACL entry, LSB 80 bits
			  * are stored in the lower address and MSB 80 bits are stored in the next address.
			  *
			  * a random data of bytes sequence b9b9... is used as data and range encodings
			  * are stitched at appropriate locations.
			  */
			tbl_id    = NLMRANGEMGR_REFAPP_TBLID_1;
			data_byte = 0xb9;

			/* data[79 : 56] and data[ 155 : 128] are un-used bit (don't cares). [79 : 56] is in abEntry_LSB_
		  	  * and [155 : 128] is in abEntry_MSB_.
		  	  */
		  	lsb_dcStart = 56;
		  	lsb_dcEnd   = 79;
		  	msb_dcStart = 128 - 80;
		  	msb_dcEnd   = 155 - 80;

		  	/* Range encodings are stitched at [55 : 32] at LSB 80b and at [ 47: 32] at MSB 80b Block Entries. */
		  	lsb_stStart = msb_stStart = 32;
		  	lsb_stEnd   = 55; /* 24b encodings stitched here */
		  	msb_stEnd   = 47; /* 16b encodings stitched here */

		  	block_num   = NLMRANGEMGR_REFAPP_BLOCK_4;
		}

		/* These many bits are don't cares in the 160b record. */
		lsb_numBits = lsb_dcEnd - lsb_dcStart + 1;
		msb_numBits = msb_dcEnd - msb_dcStart + 1;
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
		WriteBitsInRegs(abEntry_MSB_80Bits.m_mask , msb_dcEnd, msb_dcStart, ~(mask_val << msb_numBits) );

		/* Insert table id at [159 : 156] = [79: 76] within 80b boundary. */
		WriteBitsInRegs( abEntry_MSB_80Bits.m_data, 79, 76, tbl_id );

		block_index = 0; /* starting address of 4K of 80b locations within a Block */
		NlmCm__printf("\n\tWriting tbl#%d records into Block#%d\n", tbl_id, block_num);

		/* Now start stitching the encoded ranges, insert entries into Block. Number of entries added
		  * is the multiplication of num_encodedEntries_of_rangeId#0 and
		  * num_encodedEntries_of_rangeId#1.
		  */
		for( iter0 = 0; iter0 < enc0_p->m_num_entries; iter0++ )
		{
			WriteBitsInRegs( abEntry_LSB_80Bits.m_data, lsb_stEnd, lsb_stStart,
								((enc0_p->m_entries_p) + iter0)->m_data );
			WriteBitsInRegs( abEntry_LSB_80Bits.m_mask, lsb_stEnd, lsb_stStart,
								((enc0_p->m_entries_p) + iter0)->m_mask );

			for( iter1 = 0; iter1 < enc1_p->m_num_entries; iter1++ )
			{
				WriteBitsInRegs( abEntry_MSB_80Bits.m_data, msb_stEnd, msb_stStart,
								((enc1_p->m_entries_p) + iter1)->m_data );
				WriteBitsInRegs( abEntry_MSB_80Bits.m_mask, msb_stEnd, msb_stStart,
								((enc1_p->m_entries_p) + iter1)->m_mask );

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
				NlmCm__printf("\t160b record added at address-%d\n", block_index);

				/* Increment ab_index by 2 since this is 160b Block */
				block_index += 2;
			}

		}
	}

    return NLMERR_OK;
}

int NlmRangeMgrRefApp__PerformSearches( NlmRangeMgrRefAppData *refAppData_p )
{
	NlmDevMgr    *devMgr_p = refAppData_p->devMgr_p;
	NlmDevCtxBufferInfo  cb_data;
	NlmDevCmpRslt        search_result;
	NlmReasonCode  reason;
	nlm_u16    range[4], range_id;
	nlm_u8     db_id, ltr_num, iter;

	/* Construct Master Key now for Compare1 operation.
	 *
	 * Master Key has 4 ranges. Two keys Key#0 and Key#1 are generated from
	  * Master Key and are searched in parallel in tbl-0 and tbl-1 respectively.
	  *
	  * Master Key is constructed as follows:
         * Master_Key[ 319 : 316 ] :  table id 1
         * Master_Key[ 315 :  288 ] : un-used bits, could be anything
         * Master_Key[ 287 :  272 ] : range value from db#3, Range-D
         * Master_Key[ 271 :  240 ] : 0xb9b9b9.....
         * Master_Key[ 239 : 208]   :  un-used bits, could be anything
         * Master_Key[ 207 : 192 ]  :  range value from db#2, Range-C
         * Master_Key[ 192 : 160 ]  :  0xb9b9b9.....
         *
         * Master_Key[ 159 : 156 ]  :  table id 0
         * Master_Key[ 155 : 128 ] :   un-used bits, could be anything
         * Master_Key[ 127 : 112 ] :   range value from db#1, Range-B
         * Master_Key[ 111 : 80 ]   :   0xa9a9a9....
         * Master_Key[ 79   : 48 ]   :   un-used bits, could be anything
         * Master_Key[ 47   : 32 ]   :   range value from db#0, Range-A
         * Master_Key[ 31   :   0 ]   :   0xa9a9a9....
	 *
	 *
	 * MSB 160 bits are first copied into Context Buffer addresses 2 and 3 which will be
	 * used during Compare1 operation. LSB 160b are given as part of Compare1
	 * instruction.
	 *
	 * These ranges are used in the Master Key:
	 * a range value from range_id = 3 from db#3 for Range-D
	 * a range value from range_id = 2 from db#2 for Range-C,
	 * a range value from range_id = 1 from db#1 for Range-B
	 * a range value from range_id = 0 from db#0 for Range-A
	 */
	for( db_id = 0, range_id = 0; db_id < NUM_OF_RANGE_DATABASES; db_id++, range_id++ )
	{
		/* Using the start range value here. Any value within the range can be used */
		range[ range_id ] = refAppData_p->db_ranges[db_id][range_id].m_start;
	}

#ifndef NLMPLATFORM_BCM 

	/* On XLP, CB write is permitted only to 640b boundaries. */
#ifdef NLM_XLP
	NlmCm__memset( &cb_data.m_data, 0xb9, (4 * NLMDEV_CB_WIDTH_IN_BYTES) );
	cb_data.m_cbStartAddr = 0;
	cb_data.m_datalen     = 40; /* 320b data */
#else
	NlmCm__memset( &cb_data.m_data, 0xb9, (2 * NLMDEV_CB_WIDTH_IN_BYTES) );
	cb_data.m_cbStartAddr = 2;
	cb_data.m_datalen     = 20; /* 160b data */
#endif

	/* Now stitch range[3] at [127:112] and range[2] at [47:32]. MSB 80b are [159:80]
	  * and LSB 80b are [79:0]
	  */
	WriteBitsInRegs( &cb_data.m_data[0], 47, 32, range[3] );
	WriteBitsInRegs( &cb_data.m_data[10], 47, 32, range[2] );

	/* Insert tbl-id = 1 at [159:156] */
	WriteBitsInRegs( &cb_data.m_data[0], 79, 76, NLMRANGEMGR_REFAPP_TBLID_1 );

	/* Now write into Context Buffer addresses 2 and 3. */
	if( NLMERR_OK != NlmDevMgr__CBWrite( devMgr_p, &cb_data, &reason ) ){
		/* Error Mech */
		NlmCm__printf("CBWrite failed. Exiting...\n");

		return NLMERR_FAIL;
	}
	NlmCm__printf("\n\tA 160b data written at Context Buffer address#%d\n", cb_data.m_cbStartAddr);

	/* Construct LSB 160 bits of Master Key now. */
	NlmCm__memset( &cb_data.m_data, 0xa9, (2 * NLMDEV_CB_WIDTH_IN_BYTES) );
	cb_data.m_cbStartAddr = 0;
	cb_data.m_datalen     = 20; /* 160b data */

	/* Now stitch range[1] at [127:112] and range[0] at [47:32]. MSB 80b are [159:80]
	  * and LSB 80b are [79:0].
	  */
	WriteBitsInRegs( &cb_data.m_data[0], 47, 32, range[1] );
	WriteBitsInRegs( &cb_data.m_data[10], 47, 32, range[0] );

	/* Insert tbl-id = 0 at [159:156] */
	WriteBitsInRegs( &cb_data.m_data[0], 79, 76, NLMRANGEMGR_REFAPP_TBLID_0 );
#else
	/* Construct LSB 160 bits of Master Key now. */
	NlmCm__memset( &cb_data.m_data[20], 0xa9, (2 * NLMDEV_CB_WIDTH_IN_BYTES) );

	/* Now stitch range[1] at [127:112] and range[0] at [47:32]. MSB 80b are [159:80]
	  * and LSB 80b are [79:0].
	  */
	WriteBitsInRegs( &cb_data.m_data[20], 47, 32, range[1] );
	WriteBitsInRegs( &cb_data.m_data[30], 47, 32, range[0] );

	/* Insert tbl-id = 0 at [159:156] */
	WriteBitsInRegs( &cb_data.m_data[20], 79, 76, NLMRANGEMGR_REFAPP_TBLID_0 );	

	/* Construct MSB 160 bits of Master Key now. */
	NlmCm__memset( &cb_data.m_data[0], 0xb9, (2 * NLMDEV_CB_WIDTH_IN_BYTES) );

	/* Now stitch range[3] at [287:272] and range[2] at [207:192]. MSB 80b are [319:240]
	  * and LSB 80b are [239:160]
	  */
	WriteBitsInRegs( &cb_data.m_data[0], 47, 32, range[3] );
	WriteBitsInRegs( &cb_data.m_data[10], 47, 32, range[2] );

	/* Insert tbl-id = 1 at [319:316] */
	WriteBitsInRegs( &cb_data.m_data[0], 79, 76, NLMRANGEMGR_REFAPP_TBLID_1 );

	cb_data.m_cbStartAddr = 0;
	cb_data.m_datalen     = 40; /* 320b data */
#endif

	ltr_num = 0;
	NlmCm__printf("\tFiring Compare1 instruction now...\n");
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

		/* Hit is expected from PS#0 and PS#1 and index = 0. */
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

    return NLMERR_OK;
}

int nlmrangemgr_refapp_main(int argc, char	*argv[])
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
	if( NLMERR_OK != NlmRangeMgrRefApp__PerformSearches( &refAppData ) )
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

