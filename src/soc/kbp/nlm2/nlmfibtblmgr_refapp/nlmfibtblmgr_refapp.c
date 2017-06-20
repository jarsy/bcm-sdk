/*
 * $Id: nlmfibtblmgr_refapp.c,v 1.4 Broadcom SDK $
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
 * $Id: nlmfibtblmgr_refapp.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2007 Broadcom Corp.
 * All Rights Reserved.$
 */

#include "nlmfibtblmgr_refapp.h"
#include "nlmnslog.h"
#ifdef NLM_XLP
#include "nlmxlpxpt.h"
#endif

#ifdef VXWORKS
typedef struct timeval_t {
    uint32 tv_usec;
    uint32 tv_sec;
} timeval;
#endif

/* Debug Logging releated #defines  */

#ifdef NLM_XLP
	#define NLM_FIB_REFAPP_DEBUG_LOG                0   /* do not update with NLM_XLP */
#else
#define NLM_FIB_REFAPP_DEBUG_LOG                0   /* 0: Disable, 1: Enable */
#endif

#define NLM_FIB_REFAPP_MAX_DELETE_BUFFER_SIZE   (5*1024*1024)

#define NLM_FIB_REFAPP_MAX_EVENT_BUFFER_SIZE    (100 * 1024)

#define NLM_FIB_REFAPP_CONFIG_TBL(idx, tblId, tblWidth, nPfx, ixLo, ixHi) \
		{ \
			refAppData_p->m_info.m_tableInfo[idx].m_tblId = tblId; \
			refAppData_p->m_info.m_tableInfo[idx].m_tblWidth = tblWidth; \
			refAppData_p->m_info.m_tableInfo[idx].m_numOfPfxsToBeAdded = nPfx; \
			refAppData_p->m_info.m_tableInfo[idx].m_indexLow = ixLo; \
			refAppData_p->m_info.m_tableInfo[idx].m_indexHigh = ixHi; \
		}

#define NLM_FIB_REFAPP_CONFIG_SRCH(idx, ltrNum, numPrlSrch, tblId, rsltPort, stBit) \
		{ \
			refAppData_p->m_info.m_srchInfo[idx].m_ltrNum = ltrNum; \
			refAppData_p->m_info.m_srchInfo[idx].m_numOfParallelSrches = numPrlSrch; \
			refAppData_p->m_info.m_srchInfo[idx].m_tblIdForParallelSrch0 = tblId; \
			refAppData_p->m_info.m_srchInfo[idx].m_rsltPortForParallelSrch0 = rsltPort; \
			refAppData_p->m_info.m_srchInfo[idx].m_startBitForParallelSrch0 = stBit; \
		}

#define NLM_FIB_REFAPP_CONFIG_SRCH2(ltrNum, tblId0, tblId1, rsltPort0, rsltPort1, stBit) \
		{ \
			refAppData_p->m_info.m_specialSrch.m_ltrNumber = ltrNum; \
			refAppData_p->m_info.m_specialSrch.m_parallelSearch0TblId = tblId0; \
			refAppData_p->m_info.m_specialSrch.m_parallelSearch1TblId = tblId1; \
			refAppData_p->m_info.m_specialSrch.m_parallelSearch0ResultPort = rsltPort0; \
			refAppData_p->m_info.m_specialSrch.m_parallelSearch1ResultPort = rsltPort1; \
			refAppData_p->m_info.m_specialSrch.m_StratBitInKey = stBit; \
		}


nlm_u32 NlmFibRefApp_GenRandNum(nlm_u32 MaxNum)
{
    nlm_u32 temp = (((nlm_u32)rand() << 20) | ((nlm_u32)rand() << 10) | ((nlm_u32)rand()));

    if (0 == MaxNum)
    {
        MaxNum = 1;
    }

    return (temp % MaxNum);
}


/* Application prefix Index change call back function needs to be implemented
by the application and should be registered to the FibTblMgr by passing the pointer to it
as an argument to the NlmFibTblMGr__Init api along with the client pointer.
This application callback function is invoked by FibTblMgr to inform the application
about the changes occuring in the index of the prefixes added as a result of shuffling of prefixes
across the device; Also the index of the new prefix being added is informed to the application
via this callback only. In this case oldIndex = NLM_FIB_INVALID_INDEX.
Basically the application can use this function to update the associated data which is associated
with the prefixes added to the table */

void NlmFibRefApp_PfxIndexChangedCallBack(
    void *client_p, /* Application specific data which can be used by application
                    to update the associated data information whenever there is
                    change in the indices of the prefixes added */
    void *fibTbl,   /* table to which the prefix whose index is being changed belongs */
    NlmFibPrefixIndex oldIndex, /* old index of the prefix whose index is being changed ;
                                In case of new prefix, the value of this argument
                                will be NLM_FIB_INVALID_INDEX */
    NlmFibPrefixIndex newIndex /* new index of the prefix whose index is being changed */
    )
{
    NlmFibRefApp_RefApp *refApp_p = (NlmFibRefApp_RefApp*)client_p;
	
	(void)fibTbl;

	if(refApp_p->m_type != NLM_FIB_REFAPP_TYPE_BATCH)
		return;

	refApp_p->m_assoDataCBCnt++;

	if(oldIndex == 0xFFFFFFFF && newIndex == 0xFFFFFFFF)
		return;  /* in batch mode, both oldIdx = newIdx mean prefix is duplicate */

	if(oldIndex == 0xFFFFFFFF)
	{
		refApp_p->m_oldIdx[refApp_p->m_inxCnt] = refApp_p->m_inxCnt; 
		refApp_p->m_newIdx[refApp_p->m_inxCnt] = newIndex;
		refApp_p->m_inxCnt++;

    }
	else
	{
		nlm_u32 index = 0;
		NlmBool found = NlmFalse;

		/* do lookup for the newIdx in the oldIdx table, if found uodate that with newIdx */
		for(index = 0; index < refApp_p->m_inxCnt; index++)
		{
			if(refApp_p->m_newIdx[index] == 0xffffffff)
				break;

			if(refApp_p->m_newIdx[index] == oldIndex)
			{
				found = NlmTrue;
				break;
			}
		}
		if(found)
		{
			refApp_p->m_newIdx[index] = newIndex;
		}
		else
		{	
			refApp_p->m_newIdx[refApp_p->m_inxCnt] = newIndex;		
		}		
	}
}

/* InitForwardingSystem function performs initializiation of Cynapse software
in order to allow the application to perform FIB Related operations such as creating
FIB tables, configuring searches, adding prefixes, performing searches and so on.

Basically it creates Simulation transport interface, creates Device Manager instance,
adds devices and creates FIB Table Manager instance
*/
NlmErrNum_t NlmFibRefApp_InitForwardingSubSystem(
    NlmFibRefApp_RefApp *refAppData_p
    )
{
    nlm_u8 devNum;
    NlmFibBlksRange fibBlksRange[NLM_FIB_REFAPP_MAX_NUM_DEVICES];
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;

    refAppData_p->m_operMode		= NLMDEV_OPR_SAHASRA;
    
	switch(refAppData_p->m_xptType)
    {

#ifndef NLMPLATFORM_BCM
        case NLM_FIB_REFAPP_SIMULATION_XPT:
            /* create the simulation transport interface with channel Id 0
            and device operating in Sahasra mode. */

            if((refAppData_p->m_xpt_p = NlmSimXpt__Create(refAppData_p->m_alloc_p,
                                                    NLM_DEVTYPE_2_S,/* Sahasra device */
                                                    0, /* this argument is ignored for this Processor */
                                                    NLM_FIB_REFAPP_MAX_RQT_COUNT,
                                                    NLM_FIB_REFAPP_MAX_RSLT_COUNT,
                                                    refAppData_p->m_operMode,
                                                    0, /* this argument is ignored for this Processor */
                                                    0)) == NULL)
            {
                NlmCm__printf("\n\n\tError: NlmSimXpt__Create \n");
                return NLMERR_FAIL;
            }
            NlmCm__printf ("\n\t\t Simulation Xpt created successfully \n");
            break;

#ifdef NLM_XLP
		case NLM_FIB_REFAPP_XLP_XPT:

		 /* Create XLP  transport interface */
		    refAppData_p->m_xpt_p = NlmXlpXpt__Create(refAppData_p->m_alloc_p,
		                                              NLM_DEVTYPE_2_S,
		                                              NLM_FIB_REFAPP_MAX_RQT_COUNT,
		                                              refAppData_p->m_operMode,
		                                              0, 0, 0, 1,1);
		    if(refAppData_p->m_xpt_p == NULL)
		    {
		        NlmCm__printf("\n\tXLP Transport Inteface Creation Failed.\n");
		        return NLMERR_FAIL;
		    }
		    NlmCm__printf("\n\tXLP  Transport Interface Created Successfully\n");
			 break;
#endif
		case NLM_FIB_REFAPP_XPT_END:
#endif /* NLMPLATFORM_BCM */
        case NLM_FIB_REFAPP_CALADAN3_XPT:
	    refAppData_p->m_xpt_p = soc_sbx_caladan3_etu_xpt_create(0,
								  NLM_DEVTYPE_2_S, 0, NLM_FIB_REFAPP_MAX_RQT_COUNT,
								  refAppData_p->m_operMode, 0);
	    NlmCmFile__printf("\n Caladan3 Transport Interface Created Successfully\n");
	    if (refAppData_p->m_xpt_p == NULL) 
	    {
		NlmCm__printf("\n\tError: soc_sbx_caladan3_etu_xpt_create failed");
		return NLMERR_FAIL;
	    }
	    break;
        default:
             /*If other transport interfaces such as Fpga Xpt are used then
                the corresponding Init function needs to be called here */
            NlmCm__printf("\n\n\tError: Incorrect XPT Type Specified \n");
            return NLMERR_FAIL;
    }

    /* Create the device manager which provides the APIs required for device
    interaction such as Register Writes, Memory Writes and compare operations */
    if((refAppData_p->m_devMgr_p = NlmDevMgr__create(
                                                    refAppData_p->m_alloc_p,
                                                    refAppData_p->m_xpt_p,
                                                    refAppData_p->m_operMode,
                                                    &reasonCode)) == NULL)
    {
        NlmCm__printf("\n\n\tError: NlmDevMgr__create -- ReasonCode:%d\n", reasonCode);
        return NLMERR_FAIL;
    }
    NlmCm__printf ("\n\t\t Device Manager created successfully \n");

    /* Add devices to the device manager*/
    refAppData_p->m_dev_pp = NlmCmAllocator__calloc(
        refAppData_p->m_alloc_p, refAppData_p->m_numOfDevices, sizeof(NlmDev*));

    for(devNum = 0; devNum < refAppData_p->m_numOfDevices; devNum++)
    {
        NlmDevId devId;
        if((refAppData_p->m_dev_pp[devNum] = NlmDevMgr__AddDevice(
                                                    refAppData_p->m_devMgr_p,
                                                    &devId,
                                                    &reasonCode)) == NULL)
        {
            NlmCm__printf("\n\n\tError: NlmDevMgr__AddDevice (devNum: %d) -- ReasonCode: %d\n",
                                                                    devNum, reasonCode);
            return NLMERR_FAIL;
        }
    }
    NlmCm__printf ("\n\t\t %d Device(s) added to Device Manager\n", refAppData_p->m_numOfDevices);

    /* Lock the device manager configuration */
    if((errNum = NlmDevMgr__LockConfig(refAppData_p->m_devMgr_p, &reasonCode))!= NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError: NlmDevMgr__LockConfig -- ReasonCode: %d\n", reasonCode);
        return NLMERR_FAIL;
    }

    /* Initialize the blocks allocated to FIB. Blocks 0-55 are used */
    for(devNum = 0; devNum < refAppData_p->m_numOfDevices; devNum++)
    {
        fibBlksRange[devNum].m_startBlkNum = 0;
		fibBlksRange[devNum].m_endBlkNum = NLM_FIB_BLK_RANGE_WHOLE_DEVICE;

		if(refAppData_p->m_type == NLM_FIB_REFAPP_TYPE_BATCH)
			fibBlksRange[devNum].m_endBlkNum = 7;

    }

    /* create the FIB Table Manager which provides the APIs required to create
    FIB tables, configure FIB searches, add prefixes, delete prefixes and so
    on */
	if((refAppData_p->m_fibTblMgr_p = NlmFibTblMgr__Init(refAppData_p->m_alloc_p,
													     refAppData_p->m_devMgr_p,
													     NLM_DEVTYPE_2_S,
													     refAppData_p->m_numOfDevices,
													     fibBlksRange,
													     NLM_FIB_REFAPP_TBL_ID_LEN,
													     NlmFibRefApp_PfxIndexChangedCallBack,
													     refAppData_p,
                                                         &reasonCode)) == NULL)
	{
		NlmCm__printf("\n\n\tError: NlmFibTblMgr__Init -- ReasonCode: %d\n", reasonCode);
        return NLMERR_FAIL;
	}
    NlmCm__printf ("\n\t\t Fib Table Manager created successfully \n");

	#if NLM_FIB_REFAPP_DEBUG_LOG
		/* Create the Debug logging mechanism */
	NlmNsLog__Create(refAppData_p->m_fibTblMgr_p->m_privatePtr_p,
							refAppData_p->m_alloc_p, 
							NLM_FIB_REFAPP_MAX_DELETE_BUFFER_SIZE, 
							NLM_FIB_REFAPP_MAX_EVENT_BUFFER_SIZE);	
	#endif

    return NLMERR_OK;
}

/* DestroyForwardingSubSystem function destroys the
instances of FIB Table Manager, Device Manager and
Simulation Transport Interface */
void NlmFibRefApp_DestroyForwardingSubSystem(
    NlmFibRefApp_RefApp *refAppData_p
    )
{
    NlmReasonCode reasonCode;
#if NLM_FIB_REFAPP_DEBUG_LOG
    NlmNsLog__Destroy();
#endif

    NlmFibTblMgr__Destroy(refAppData_p->m_fibTblMgr_p, &reasonCode);

    NlmCmAllocator__free(refAppData_p->m_alloc_p, refAppData_p->m_fibTbl_pp);

    NlmDevMgr__destroy(refAppData_p->m_devMgr_p);

    NlmCmAllocator__free(refAppData_p->m_alloc_p, refAppData_p->m_dev_pp);

    switch(refAppData_p->m_xptType)
    {
#ifndef NLMPLATFORM_BCM
        case NLM_FIB_REFAPP_SIMULATION_XPT:
            NlmSimXpt__destroy(refAppData_p->m_xpt_p);
            break;

#ifdef NLM_XLP
		case NLM_FIB_REFAPP_XLP_XPT:
            NlmXlpXpt__Destroy(refAppData_p->m_xpt_p);
            break;
#endif
#endif /* NLMPLATFORM_BCM */
        case NLM_FIB_REFAPP_CALADAN3_XPT:
	    if (refAppData_p->m_xpt_p) {
		soc_sbx_caladan3_etu_xpt_destroy(refAppData_p->m_xpt_p);
	    }
	    break;
        default:
             /*If other transport interfaces such as Fpga Xpt are used then
             the corresponding destroy function needs to be called here */
            break;

    }
}

/* initialize the refapp information based on the type user provided */
void NlmFibRefApp_InitializeRefappInfo(
    NlmFibRefApp_RefApp *refAppData_p
    )
{
	switch(refAppData_p->m_type)
	{
	    default:
		case NLM_FIB_REFAPP_TYPE_NORMAL:
				refAppData_p->m_numOfTables = NLM_FIB_REFAPP_NUM_TABLES;
				refAppData_p->m_numSearches = NLM_FIB_REFAPP_NUM_SRCH_CONFIG;

				NLM_FIB_REFAPP_CONFIG_TBL(0, 0, 36, 20000, 0, 30000);
				NLM_FIB_REFAPP_CONFIG_TBL(1, 1, 132, 20000, 31000, 65000);
				NLM_FIB_REFAPP_CONFIG_TBL(2, 2, 144, 20000, 66000, 99000);

				NLM_FIB_REFAPP_CONFIG_SRCH(0, 0, 1, 0, 0, 39);
				NLM_FIB_REFAPP_CONFIG_SRCH(1, 41, 1, 1, 1, 159);
				NLM_FIB_REFAPP_CONFIG_SRCH(2, 52, 1, 2, 0, 319);			

			break;

		case NLM_FIB_REFAPP_TYPE_CONFIG2:

				if(refAppData_p->m_isConfig2)
				{
					refAppData_p->m_numOfTables = 2;

					NLM_FIB_REFAPP_CONFIG_TBL(0, 0, 144, 300000, 0, 400000);
					NLM_FIB_REFAPP_CONFIG_TBL(1, 1, 144, 300000, 0, 400000);
					NLM_FIB_REFAPP_CONFIG_SRCH2(0, 0, 1, 0, 1, 159);

				}
				else
				{
					refAppData_p->m_numOfTables = 1;
					refAppData_p->m_numSearches = 1;

					NLM_FIB_REFAPP_CONFIG_TBL(0, 0, 144, 300000, 0, 400000);
					NLM_FIB_REFAPP_CONFIG_SRCH(0, 0, 1, 0, 0, 159);
					
				}
			break;

		case NLM_FIB_REFAPP_TYPE_BATCH:
				refAppData_p->m_numOfTables = 1;
				refAppData_p->m_numSearches = 1;

				NLM_FIB_REFAPP_CONFIG_TBL(0, 0, 56, NLM_FIB_REFAPP_BATCH_MAX_PFX_COUNT, 0, 2000000);
				NLM_FIB_REFAPP_CONFIG_SRCH(0, 0, 1, 0, 0, 79);

			break;
	}

	return;
}

/* TblIdStringConvert function converts an integer tblId
into a string of zeroes and ones as expected by the FibTblMgr*/
void NlmFibRefApp_TblIdStringConvert(
    nlm_u8 tblId,
    nlm_8 *tblIdStr,
    nlm_u8 strLen
    )
{
    nlm_u8 i;
    for(i = 0;i < strLen;i++)
        tblIdStr[strLen - i - 1] = (((tblId >> i) & 1) ? '1': '0');
    tblIdStr[strLen] = '\0';
}

/* CreateLPMTables function registers different LPM tables with the FibTblMgr;
The Number of tables created depends on the value of NLM_FIB_REFAPP_NUM_TABLES
and attributes of the tables are provided by tableInfo
*/
NlmErrNum_t NlmFibRefApp_CreateLPMTables(
    NlmFibRefApp_RefApp *refAppData_p
    )
{
    nlm_u8 tblNum;
    nlm_8 tblIdStr[NLM_FIB_REFAPP_TBL_ID_LEN + 1];
    NlmFibTblIndexRange indexRange;
    NlmReasonCode reasonCode;

	NlmFibRefApp_InitializeRefappInfo(refAppData_p);

    refAppData_p->m_fibTbl_pp = NlmCmAllocator__calloc(
		refAppData_p->m_alloc_p, refAppData_p->m_numOfTables, sizeof(NlmFibTbl*));

    for(tblNum = 0; tblNum < refAppData_p->m_numOfTables; tblNum++)
    {

        /* Conevrting tbl id to string of 0's and 1's*/
		NlmFibRefApp_TblIdStringConvert(refAppData_p->m_info.m_tableInfo[tblNum].m_tblId,
                                        tblIdStr,
                                        NLM_FIB_REFAPP_TBL_ID_LEN);

        indexRange.m_indexLowValue = refAppData_p->m_info.m_tableInfo[tblNum].m_indexLow;
        indexRange.m_indexHighValue = refAppData_p->m_info.m_tableInfo[tblNum].m_indexHigh;

        if((refAppData_p->m_fibTbl_pp[tblNum] =
            NlmFibTblMgr__CreateTable(refAppData_p->m_fibTblMgr_p,
                                      tblIdStr,
                                      &indexRange,
                                      refAppData_p->m_info.m_tableInfo[tblNum].m_tblWidth,
                                      &reasonCode)) == NULL)
        {
            NlmCm__printf("\n\n\tError: NlmFibTblMgr__CreateTable (TblId: %d) -- ReasonCode: %d\n",
				refAppData_p->m_info.m_tableInfo[tblNum].m_tblId, reasonCode);
            return NLMERR_FAIL;
        }
        NlmCm__printf ("\n\t\t Table with Tbl Id %d Created\n", refAppData_p->m_info.m_tableInfo[tblNum].m_tblId);
    }

    return NLMERR_OK;
}

/* ConfigSearches function configures different types of FIB search operations
being performed. The Number of saerch configuartions depends on the value of
NLM_FIB_REFAPP_NUM_SRCH_CONFIG and attributes of the saerches are provided by srchInfo
*/
NlmErrNum_t NlmFibRefApp_ConfigSearches(
    NlmFibRefApp_RefApp *refAppData_p
    )
{
    nlm_u8 srchNum;
    NlmFibParallelSrchAttributes psAttrs;
    nlm_8 tblIdStr[NLM_FIB_MAX_PARALLEL_SRCHES][NLM_FIB_REFAPP_TBL_ID_LEN + 1];
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;

    for(srchNum = 0; srchNum < refAppData_p->m_numSearches;srchNum++)
    {
        psAttrs.m_numOfFibParallelSrch = refAppData_p->m_info.m_srchInfo[srchNum].m_numOfParallelSrches;

        NlmFibRefApp_TblIdStringConvert(refAppData_p->m_info.m_srchInfo[srchNum].m_tblIdForParallelSrch0,
                                        tblIdStr[0],
                                        NLM_FIB_REFAPP_TBL_ID_LEN);

        psAttrs.m_parallelSrchInfo[0].m_tblId = tblIdStr[0];
        psAttrs.m_parallelSrchInfo[0].m_rsltPortNum =
            refAppData_p->m_info.m_srchInfo[srchNum].m_rsltPortForParallelSrch0;
        psAttrs.m_parallelSrchInfo[0].m_startBitInKey =
            refAppData_p->m_info.m_srchInfo[srchNum].m_startBitForParallelSrch0;
        
        if((errNum = NlmFibTblMgr__ConfigSearch(refAppData_p->m_fibTblMgr_p,
                                                refAppData_p->m_info.m_srchInfo[srchNum].m_ltrNum,
                                                &psAttrs,
                                                &reasonCode)) != NLMERR_OK)
        {
            NlmCm__printf("\n\n\tError: NlmFibTblMgr__ConfigSearch (LtrNum: %d) -- ReasonCode: %d\n",
				refAppData_p->m_info.m_srchInfo[srchNum].m_ltrNum, reasonCode);
            return errNum;
        }
        NlmCm__printf ("\n\t\t Ltr Num %d Configured\n", refAppData_p->m_info.m_srchInfo[srchNum].m_ltrNum);
    }

    /* Lock the FIB configuartions */
    if((errNum = NlmFibTblMgr__LockConfiguration(refAppData_p->m_fibTblMgr_p, &reasonCode)) != NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError: NlmFibTblMgr__LockConfiguration -- ReasonCode: %d\n", reasonCode);
        return errNum;
    }

    return NLMERR_OK;
}


NlmErrNum_t NlmFibRefApp_ConfigSearchesForConfig2(
    NlmFibRefApp_RefApp *refAppData_p
    )
{
    NlmFibParallelSrchAttributes2 psAttrs;
    nlm_8 tblIdStr[NLM_FIB_MAX_PARALLEL_SRCHES][NLM_FIB_REFAPP_TBL_ID_LEN + 1];
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;
	nlm_u8 ltrNum = 0;

	NlmFibRefApp_TblIdStringConvert(refAppData_p->m_info.m_specialSrch.m_parallelSearch0TblId,
                                        tblIdStr[0],
                                        NLM_FIB_REFAPP_TBL_ID_LEN);

	NlmFibRefApp_TblIdStringConvert(refAppData_p->m_info.m_specialSrch.m_parallelSearch1TblId,
                                        tblIdStr[1],
                                        NLM_FIB_REFAPP_TBL_ID_LEN);

		ltrNum = refAppData_p->m_info.m_specialSrch.m_ltrNumber;
		psAttrs.m_tblId0 = tblIdStr[0];
		psAttrs.m_tblId1 = tblIdStr[1];
		psAttrs.m_rsltPortNum0 = refAppData_p->m_info.m_specialSrch.m_parallelSearch0ResultPort;
		psAttrs.m_rsltPortNum1 = refAppData_p->m_info.m_specialSrch.m_parallelSearch1ResultPort;
		psAttrs.m_startBitInKey = refAppData_p->m_info.m_specialSrch.m_StratBitInKey;

        if((errNum = NlmFibTblMgr__ConfigSearch2(refAppData_p->m_fibTblMgr_p,
                                                ltrNum,
                                                &psAttrs,
                                                &reasonCode)) != NLMERR_OK)
        {
            NlmCm__printf("\n\n\tError: NlmFibTblMgr__ConfigSearch2 (LtrNum: %d) -- ReasonCode: %d\n",
                                                        ltrNum, reasonCode);
            return errNum;
        }
        NlmCm__printf ("\n\t\t Ltr Num %d Configured\n", ltrNum);

    /* Lock the FIB configuartions */
    if((errNum = NlmFibTblMgr__LockConfiguration(refAppData_p->m_fibTblMgr_p, &reasonCode)) != NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError: NlmFibTblMgr__LockConfiguration -- ReasonCode: %d\n", reasonCode);
        return errNum;
    }
    return NLMERR_OK;
}


/*
   * This function generates prefixes in the middle increment pattern. Prefixes are
   * 56b in length. Prefix pattern is : 0.0.0.0.0.0.0, 0.0.0.0.1.0.0, 0.0.0.0.2.0.0,
   * and so on.
   *
   * 4b table id(all 0s) + 4b reserved(all 0s) +
   *  32b data(16b VPN ID +16b MSB of IPv4) + 
   *  16b(16 LSB of IPv4, and are always all 0s) incremntal 
   */
NlmErrNum_t NlmFibRefApp_GenMiddleIncrPattern(
 	nlm_u8 *pfxData,
	nlm_u32 iter
	)
{
	nlm_u32 endPos = 0;
	
	/* pfxData is an array of size 7. nlm_u8 pfxData[7]. "iter" takes values from 0,1,2 and so on
	  * This function should be called for each value starting from 0, till table full. 
	  * Prefix in the form of raw bytes is  written into pfxData.
	 */
	endPos = (NLM_FIB_REFAPP_MAX_BATCH_PFX_LEN - NLM_FIB_REFAPP_TBL_ID_LEN- 1) - 4;

	/* Write 32b value from pfxData[1] till pfxData[4] */
	WriteBitsInArray(pfxData, NLM_FIB_REFAPP_MAX_BATCH_PFX_LEN / 8,
					endPos, endPos - 31, iter);

	return NLMERR_OK;
}

/* NlmFibRefApp_GeneratePrefix function generates prefix in the
   incremental order; length of the prefix is provided by pfxLen,
   and prefix is prepended with tblId at its MSB.

   Note: This function does not create the prefix but expects the
    caller to create it and hence also destroy it
 */
NlmErrNum_t NlmFibRefApp_GeneratePrefix(
    NlmCmPrefix *prefix_p,
	NlmFibRefApp_Type refAppType,
    nlm_u32 pfxLen,
    nlm_u8 tblId,
    nlm_u32 iter
    )
{
    nlm_u8 pfxData[NLMDEV_FIB_MAX_PREFIX_LENGTH/8] = {0};

    if(pfxLen < NLM_FIB_REFAPP_TBL_ID_LEN)    /* Pfxlen cannot be less than length of tblId  */
    {
        NlmCm__printf("\n\n\tError: Pfx Len Should Not Be Less Than TblIdLen");
        return NLMERR_FAIL;
    }

    if(pfxLen > NLMDEV_FIB_MAX_PREFIX_LENGTH)/* Pfxlen cannot be greater than 320b  */
    {
        NlmCm__printf("\n\n\tError: Pfx Len Should Not Be Greater Than 320b");
        return NLMERR_FAIL;
    }

	if(refAppType == NLM_FIB_REFAPP_TYPE_BATCH)
	{
		NlmFibRefApp_GenMiddleIncrPattern(pfxData, iter);
	}
	else
	{
		/* Generate the prefix data based on the iter value and pfxlen */
		if(pfxLen >= 36)
		{
			WriteBitsInArray(pfxData, NLMDEV_FIB_MAX_PREFIX_LENGTH/8,
				(NLMDEV_FIB_MAX_PREFIX_LENGTH - pfxLen + 31),
				(NLMDEV_FIB_MAX_PREFIX_LENGTH - pfxLen),
				iter);
		}
		else
		{
			WriteBitsInArray(pfxData, NLMDEV_FIB_MAX_PREFIX_LENGTH/8,
				NLMDEV_FIB_MAX_PREFIX_LENGTH - (NLM_FIB_REFAPP_TBL_ID_LEN + 1),
				(NLMDEV_FIB_MAX_PREFIX_LENGTH - pfxLen),
				iter & pfxLen);
		}
	}

    /* Prepend  the table id to the prefix */
    WriteBitsInArray(pfxData, NLMDEV_FIB_MAX_PREFIX_LENGTH/8,
        NLMDEV_FIB_MAX_PREFIX_LENGTH - 1,
        (NLMDEV_FIB_MAX_PREFIX_LENGTH - NLM_FIB_REFAPP_TBL_ID_LEN),
        tblId);

    NlmCmPrefix__Set(prefix_p, pfxLen, pfxData);

    return NLMERR_OK;
}


/* LoadLPMTables function adds the pefixes to each table.
Number of prefixes which are added to each table is specified
by the table attributes tableInfo. */

NlmErrNum_t NlmFibRefApp_LoadLPMTables_Batch(
    NlmFibRefApp_RefApp *refAppData_p
    )
{
    nlm_u8 tblNum;
    nlm_u32 iter;
    NlmCmPrefix *prefix_p[NLM_FIB_REFAPP_BATCH_MAX_SIZE_COUNT];
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;
	nlm_u32 count = 0;
	nlm_u16 batchCount = 0;
	nlm_u32 uniquePfxCnt = refAppData_p->m_startAt;

	nlm_u32 BATCHVal[4] = {500, 1000, 1500, 2000};
	nlm_u32 v = 0, randb = 0;

    for(tblNum = 0; tblNum < refAppData_p->m_numOfTables; tblNum++)
    {
		iter = refAppData_p->m_startAt;  /* start from here to insert prefixes in batch to table */
		NlmCm__printf("\n\t\t Table Num:%d\n", refAppData_p->m_info.m_tableInfo[tblNum].m_tblId);

        /* Create a prefix data structure using the utility function */
        for(count = 0; count < NLM_FIB_REFAPP_BATCH_MAX_SIZE_COUNT; count++)
			prefix_p[count] = NlmCmPrefix__create(refAppData_p->m_alloc_p, 
			refAppData_p->m_info.m_tableInfo[tblNum].m_tblWidth, 0, NULL);

        while(refAppData_p->m_fibTbl_pp[tblNum]->m_numPrefixes < 
			refAppData_p->m_info.m_tableInfo[tblNum].m_numOfPfxsToBeAdded)
        {
			if(refAppData_p->m_batchSize == 0)
			{
				randb = BATCHVal[v++];
				if(v > 3)
					v = 0;
			}
			else
			{
				randb = BATCHVal[refAppData_p->m_batchSize - 1];
			}
			
			for(count = 0; count < randb; count++)
			{
				/* Generate a prefix to be added;
				Prefixes generated are in incremental order. For e.g., for 32b table with
				table id 0, prefixes generated are : 0.0.0.0, 0.0.0.1, 0.0.0.2 and so on */
				if((errNum = NlmFibRefApp_GeneratePrefix(prefix_p[count], refAppData_p->m_type,
						refAppData_p->m_info.m_tableInfo[tblNum].m_tblWidth, 
						refAppData_p->m_info.m_tableInfo[tblNum].m_tblId, iter)) != NLMERR_OK)
                return errNum;

				iter++;
			}

			batchCount = (nlm_u16)count; /* NLM_FIB_REFAPP_BATCH_COUNT */

            /* add the prefix using the FIB TblMgr API */
            if((errNum = NlmFibTblMgr__AddPrefixBatch(
				refAppData_p->m_fibTbl_pp[tblNum], &batchCount, &prefix_p[0], &reasonCode)) != NLMERR_OK)
            {
                if(reasonCode == NLMRSC_RESOURCE_ALLOC_FAILED) /* table full */
				{
					NlmCm__printf("\n\t\t       -> %4d prefixes Added to Table in batch %4d ", batchCount,count);
					NlmCm__printf("\n\n\tWarning: NlmFibTblMgr__AddPrefix -- Table full\n");
					uniquePfxCnt += batchCount;
					refAppData_p->m_info.m_tableInfo[tblNum].m_numOfPfxsToBeAdded = uniquePfxCnt;
					break;
				}
				else if(reasonCode == NLMRSC_IDX_RANGE_FULL) /* table full */
				{
					NlmCm__printf("\n\t\t       -> %4d prefixes Added to Table in batch %4d ", batchCount,count);
					NlmCm__printf("\n\n\tWarning: NlmFibTblMgr__AddPrefix -- IndexRange full\n");
					uniquePfxCnt += batchCount;
					refAppData_p->m_info.m_tableInfo[tblNum].m_numOfPfxsToBeAdded = uniquePfxCnt;
					break;
				}
				else if(reasonCode !=  NLMRSC_DUPLICATE_PREFIX)/* if reason for failure is
                                                          duplcate prefix then dont exit
                                                          but generate new prefix */
                {
                    NlmCm__printf("\n\n\tError: NlmFibTblMgr__AddPrefix -- ReasonCode: %d\n", reasonCode);
                    break;
                }
				else
				{
					NlmCm__printf("\n\n\tError: %d, Reason: %d (NlmFibTblMgr__AddPrefix)\n", errNum, reasonCode);
					break;
				}
            }
			uniquePfxCnt += batchCount;

			NlmCm__printf("\n\t\t       -> %4d prefixes Added to Table in batch ", batchCount);
            if((uniquePfxCnt % 10000) == 0)
				NlmCm__printf("\n\t\t   - %d prefixes Added to Table \n", iter);

			
        }

        /* Destroy the prefix data structure created */
		for(count = 0; count < NLM_FIB_REFAPP_BATCH_MAX_SIZE_COUNT; count++)
			NlmCmPrefix__destroy(prefix_p[count], refAppData_p->m_alloc_p);

        NlmCm__printf("\n\t\t   - Total %d prefixes Added to Table\n", 
			refAppData_p->m_info.m_tableInfo[tblNum].m_numOfPfxsToBeAdded);
    }
    return NLMERR_OK;
}

/* LoadLPMTables function adds the pefixes to each table.
Number of prefixes which are added to each table is specified
by the table attributes tableInfo. */
NlmErrNum_t NlmFibRefApp_LoadLPMTables(
    NlmFibRefApp_RefApp *refAppData_p
    )
{
    nlm_u8 tblNum;
    nlm_u32 iter;
    NlmCmPrefix *prefix_p;
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;

    for(tblNum = 0; tblNum < refAppData_p->m_numOfTables; tblNum++)
    {
        iter = 0;  /* Initilialize the iter value to 0 for each table */
		NlmCm__printf("\n\t\t Table Num:%d\n", refAppData_p->m_info.m_tableInfo[tblNum].m_tblId);

        /* Create a prefix data structure using the utility function */
        prefix_p = NlmCmPrefix__create(
            refAppData_p->m_alloc_p, refAppData_p->m_info.m_tableInfo[tblNum].m_tblWidth, 0, NULL);

        while(refAppData_p->m_fibTbl_pp[tblNum]->m_numPrefixes <
                refAppData_p->m_info.m_tableInfo[tblNum].m_numOfPfxsToBeAdded)
        {

			if(refAppData_p->m_type == NLM_FIB_REFAPP_TYPE_BATCH && refAppData_p->m_isBatch)
			{
				if(refAppData_p->m_fibTbl_pp[tblNum]->m_numPrefixes == refAppData_p->m_startAt)
				{
					NlmCmPrefix__destroy(prefix_p, refAppData_p->m_alloc_p);
					printf("\n\t Going to batch mode now .... inserted %d prefixes \n", iter);
					return 0;
				}
			}

            /* Generate a prefix to be added;
            Prefixes generated are in incremental order. For e.g., for 32b table with
            table id 0, prefixes generated are : 0.0.0.0, 0.0.0.1, 0.0.0.2 and so on */
			if((errNum = NlmFibRefApp_GeneratePrefix(prefix_p, refAppData_p->m_type,
                                                     refAppData_p->m_info.m_tableInfo[tblNum].m_tblWidth,
                                                     refAppData_p->m_info.m_tableInfo[tblNum].m_tblId,
                                                     iter)) != NLMERR_OK)
                return errNum;

            /* add the prefix using the FIB TblMgr API */
            if((errNum = NlmFibTblMgr__AddPrefix(refAppData_p->m_fibTbl_pp[tblNum],
                                                 prefix_p,
                                                 &reasonCode)) != NLMERR_OK)
            {
                if(reasonCode == NLMRSC_RESOURCE_ALLOC_FAILED) /* table full */
				{
					NlmCm__printf("\n\n\tWarning: NlmFibTblMgr__AddPrefix -- Table full\n");
					refAppData_p->m_info.m_tableInfo[tblNum].m_numOfPfxsToBeAdded = iter;
					break;
				}
				else if(reasonCode == NLMRSC_IDX_RANGE_FULL) /* table full */
				{
					NlmCm__printf("\n\n\tWarning: NlmFibTblMgr__AddPrefix -- IndexRange full\n");
					refAppData_p->m_info.m_tableInfo[tblNum].m_numOfPfxsToBeAdded = iter;
					break;
				}
				else if(reasonCode !=  NLMRSC_DUPLICATE_PREFIX)/* if reason for failure is
                                                          duplcate prefix then dont exit
                                                          but generate new prefix */
                {
                    NlmCm__printf("\n\n\tError: NlmFibTblMgr__AddPrefix -- ReasonCode: %d\n",
                                                                              reasonCode);
                    NlmCmPrefix__destroy(prefix_p, refAppData_p->m_alloc_p);
                    return errNum;
                }
				else
				{
					NlmCm__printf("\n\n\tError: %d, Reason: %d (NlmFibTblMgr__AddPrefix)\n", errNum, reasonCode);
					NlmCmPrefix__destroy(prefix_p, refAppData_p->m_alloc_p);
                    return errNum;
				}
            }
            iter++;
			if((iter % 10000) == 0)
				NlmCm__printf("\n\t\t   - %d prefixes Added to Table", iter);
        }

        /* Destroy the prefix data structure created */
        NlmCmPrefix__destroy(prefix_p, refAppData_p->m_alloc_p);
        NlmCm__printf("\n\t\t   - Total %d prefixes Added to Table\n", 
			refAppData_p->m_info.m_tableInfo[tblNum].m_numOfPfxsToBeAdded);
    }
    return NLMERR_OK;
}

NlmErrNum_t NlmFibRefApp_GeneratePrefixForConfig2(
    NlmCmPrefix *prefix_p,
    nlm_u32 pfxLen,
    nlm_u8 tblId,
    nlm_u32 iter
    )
{
    nlm_u8 pfxData[NLMDEV_FIB_MAX_PREFIX_LENGTH/8] = {0};
    nlm_u32 RealLen = pfxLen;

    if(pfxLen < NLM_FIB_REFAPP_TBL_ID_LEN)    /* Pfxlen cannot be less than length of tblId  */
    {
        printf("\r\nError: Pfx Len Should Not Be Less Than TblIdLen");
        return NLMERR_FAIL;
    }

    if(pfxLen > NLMDEV_FIB_MAX_PREFIX_LENGTH)/* Pfxlen cannot be greater than 320b  */
    {
        printf("\r\nError: Pfx Len Should Not Be Greater Than 320b");
        return NLMERR_FAIL;
    }

    /* Generate the prefix data based on the iter value and pfxlen */
    if(pfxLen >= 36)
    {        
        nlm_u32 Vpn = 0;
        nlm_u32 Key = 0;
		nlm_u32 VpnLen = 12;

		nlm_u32 keyLen = NlmFibRefApp_GenRandNum(10);

		if (0 == keyLen)
		{
			RealLen = 85 + NlmFibRefApp_GenRandNum(60);
		}
		else 
		{
			RealLen = 53 + NlmFibRefApp_GenRandNum(32);
		}

		Vpn = NlmFibRefApp_GenRandNum(1);
        WriteBitsInArray(pfxData, NLMDEV_FIB_MAX_PREFIX_LENGTH/8,
            (NLMDEV_FIB_MAX_PREFIX_LENGTH  - NLM_FIB_REFAPP_TBL_ID_LEN-1),
            (NLMDEV_FIB_MAX_PREFIX_LENGTH -NLM_FIB_REFAPP_TBL_ID_LEN -VpnLen),
            Vpn);

        Key = NlmFibRefApp_GenRandNum(0xFFFFFFFF);
        WriteBitsInArray(pfxData, NLMDEV_FIB_MAX_PREFIX_LENGTH/8,
            (NLMDEV_FIB_MAX_PREFIX_LENGTH - pfxLen + 31),
            (NLMDEV_FIB_MAX_PREFIX_LENGTH - pfxLen),
            Key);

        if (pfxLen > 48)
        {
            Key = NlmFibRefApp_GenRandNum(0xFFFFFFFF);
            WriteBitsInArray(pfxData, NLMDEV_FIB_MAX_PREFIX_LENGTH/8,
                (NLMDEV_FIB_MAX_PREFIX_LENGTH - pfxLen + 63),
                (NLMDEV_FIB_MAX_PREFIX_LENGTH - pfxLen+32),
                Key);
        }

        if (pfxLen > 80)
        {
            Key = NlmFibRefApp_GenRandNum(0xFFFFFFFF);
            WriteBitsInArray(pfxData, NLMDEV_FIB_MAX_PREFIX_LENGTH/8,
                (NLMDEV_FIB_MAX_PREFIX_LENGTH - pfxLen + 95),
                (NLMDEV_FIB_MAX_PREFIX_LENGTH - pfxLen+64),
                Key);
            Key = NlmFibRefApp_GenRandNum(0xFFFFFFFF);
            WriteBitsInArray(pfxData, NLMDEV_FIB_MAX_PREFIX_LENGTH/8,
                (NLMDEV_FIB_MAX_PREFIX_LENGTH - pfxLen + 127),
                (NLMDEV_FIB_MAX_PREFIX_LENGTH - pfxLen+96),
                Key);
        }
    }
    else
    {
        WriteBitsInArray(pfxData, NLMDEV_FIB_MAX_PREFIX_LENGTH/8,
            NLMDEV_FIB_MAX_PREFIX_LENGTH - (NLM_FIB_REFAPP_TBL_ID_LEN + 1),
            (NLMDEV_FIB_MAX_PREFIX_LENGTH - pfxLen),
            iter & pfxLen);
    }

    /* Prepend  the table id to the prefix */
    WriteBitsInArray(pfxData, NLMDEV_FIB_MAX_PREFIX_LENGTH/8,
        NLMDEV_FIB_MAX_PREFIX_LENGTH - 1,
        (NLMDEV_FIB_MAX_PREFIX_LENGTH - NLM_FIB_REFAPP_TBL_ID_LEN),
        tblId);

    NlmCmPrefix__Set(prefix_p, RealLen, pfxData);

	return NLMERR_OK;
}

NlmErrNum_t NlmFibRefApp_LoadLPMTablesForConfig2(
    NlmFibRefApp_RefApp *refAppData_p
    )
{
    nlm_u8 tblNum;
    nlm_u32 iter;
    NlmCmPrefix *prefix_p;
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;
	nlm_u8 TblIdChange = 0;
	nlm_u32 DupliPfx = 0; 

    for(tblNum = 0; tblNum < refAppData_p->m_numOfTables; tblNum++)
    {
		if(refAppData_p->m_isConfig2)
		if(tblNum == 1)
			continue;

        iter = 0;  /* Initilialize the iter value to 0 for each table */
		NlmCm__printf("\n\t\t Table Num:%d\n", refAppData_p->m_info.m_tableInfo[tblNum].m_tblId);

        /* Create a prefix data structure using the utility function */
        prefix_p = NlmCmPrefix__create(
            refAppData_p->m_alloc_p, refAppData_p->m_info.m_tableInfo[tblNum].m_tblWidth, 0, NULL);

        while(refAppData_p->m_fibTbl_pp[tblNum]->m_numPrefixes <
                refAppData_p->m_info.m_tableInfo[tblNum].m_numOfPfxsToBeAdded)
        {
            /* Generate a prefix to be added;
            Prefixes generated are in incremental order. For e.g., for 32b table with
            table id 0, prefixes generated are : 0.0.0.0, 0.0.0.1, 0.0.0.2 and so on */
            if((errNum = NlmFibRefApp_GeneratePrefixForConfig2(prefix_p,
                                                     refAppData_p->m_info.m_tableInfo[tblNum].m_tblWidth,
                                                     refAppData_p->m_info.m_tableInfo[tblNum].m_tblId,
                                                     iter)) != NLMERR_OK)
                return errNum;

			/*if longer Pfx change the table ID */
			if(refAppData_p->m_isConfig2)
			if((tblNum == 0) && prefix_p->m_inuse > 80)
			{
					TblIdChange = 1;
					prefix_p->m_data[0] = ((prefix_p->m_data[0] & 0x0f)| (0x10));
			}
			

            /* add the prefix using the FIB TblMgr API */
            if((errNum = NlmFibTblMgr__AddPrefix(refAppData_p->m_fibTbl_pp[tblNum + TblIdChange],
                                                 prefix_p,
                                                 &reasonCode)) != NLMERR_OK)
            {
                if(reasonCode == NLMRSC_RESOURCE_ALLOC_FAILED) /* table full */
				{
					NlmCm__printf("\n\n\tWarning: NlmFibTblMgr__AddPrefix -- Table full\n");
					refAppData_p->m_info.m_tableInfo[tblNum].m_numOfPfxsToBeAdded = iter;
					break;
				}
				else if(reasonCode == NLMRSC_IDX_RANGE_FULL) /* table full */
				{
					NlmCm__printf("\n\n\tWarning: NlmFibTblMgr__AddPrefix -- IndexRange full\n");
					refAppData_p->m_info.m_tableInfo[tblNum].m_numOfPfxsToBeAdded = iter;
					break;
				}
				else if(reasonCode ==  NLMRSC_DUPLICATE_PREFIX)/* if reason for failure is
                                                          duplcate prefix then dont exit
                                                          but generate new prefix */
                {
					DupliPfx++;					
                }
				else
				{
					NlmCm__printf("\n\n\tError: %d, Reason: %d (NlmFibTblMgr__AddPrefix)\n", errNum, reasonCode);
					NlmCmPrefix__destroy(prefix_p, refAppData_p->m_alloc_p);
                    return errNum;
				}
            }

			if(refAppData_p->m_isConfig2)
				TblIdChange = 0;

            iter++;
			if((iter % 10000) == 0)
				NlmCm__printf("\n\t\t   - %d prefixes Added to Table", iter);
        }

        /* Destroy the prefix data structure created */
        NlmCmPrefix__destroy(prefix_p, refAppData_p->m_alloc_p);
        NlmCm__printf("\n\t\t   - Total %d prefixes Added to Table\n", refAppData_p->m_info.m_tableInfo[tblNum].m_numOfPfxsToBeAdded);
		NlmCm__printf("\n\t\t   - Total %d Duplicate prefixes Generated\n", DupliPfx);
    }
    return NLMERR_OK;
}


/* DeletePrefixes function deletes prefixes from each table.
Half the number of prefixes which were added are deleted
from each table */
NlmErrNum_t NlmFibRefApp_DeletePrefixes(
    NlmFibRefApp_RefApp *refAppData_p
    )
{
    nlm_u8 tblNum;
    nlm_u32 iter;
    NlmCmPrefix *prefix_p;
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;
	nlm_u32 numPfxToDelete = 0;

    for(tblNum = 0; tblNum < refAppData_p->m_numOfTables; tblNum++)
    {
		numPfxToDelete = (refAppData_p->m_info.m_tableInfo[tblNum].m_numOfPfxsToBeAdded/2); /* for nomral*/

		if(refAppData_p->m_type == NLM_FIB_REFAPP_TYPE_BATCH)
			numPfxToDelete = 0;

        iter = 0;/* Initilialize the iter value to 0 for each table */
		NlmCm__printf("\n\t\t Table Num:%d\n", refAppData_p->m_info.m_tableInfo[tblNum].m_tblId);

        /* Create a prefix data structure using the utility function */
        prefix_p = NlmCmPrefix__create(refAppData_p->m_alloc_p,
                                       refAppData_p->m_info.m_tableInfo[tblNum].m_tblWidth,
                                       0, NULL);

        while(refAppData_p->m_fibTbl_pp[tblNum]->m_numPrefixes > numPfxToDelete)
        {
             /* Generate a prefix to be deleted;
            Prefixes generated are in incremental order */
            if((errNum = NlmFibRefApp_GeneratePrefix(prefix_p, refAppData_p->m_type,
                                                     refAppData_p->m_info.m_tableInfo[tblNum].m_tblWidth,
                                                     refAppData_p->m_info.m_tableInfo[tblNum].m_tblId,
                                                     iter)) != NLMERR_OK)
                return errNum;


             /* delete the prefix using the Fib TblMgr api */
            if((errNum = NlmFibTblMgr__DeletePrefix(refAppData_p->m_fibTbl_pp[tblNum],
                                                    prefix_p,
                                                    &reasonCode)) != NLMERR_OK)
            {
                if(reasonCode !=  NLMRSC_PREFIX_NOT_FOUND) /* If prefix is  not found generate
                                                           another prefix */
                {
                    NlmCm__printf("\n\n\tError: NlmFibTblMgr__DeletePrefix -- ReasonCode: %d\n",
                                                                               reasonCode);
                    NlmCmPrefix__destroy(prefix_p, refAppData_p->m_alloc_p);
                    return errNum;
                }
            }
            iter++;
			if((iter % 10000) == 0)
				NlmCm__printf("\n\t\t   - %d prefixes Deleted from Table", iter);
        }
        /* Destroy the prefix data structure using the utility function */
        NlmCmPrefix__destroy(prefix_p, refAppData_p->m_alloc_p);
		NlmCm__printf("\n\t\t   - Total %d prefixes Deleted from Table\n", iter);
    }
    return NLMERR_OK;
}

#define NLM_FIB_REFAPP_MAX_SRCH_LEN_IN_BYTES (NLMDEV_FIB_MAX_PREFIX_LENGTH/8)

/* DeviceCompare function performs device compare using Device
Manager APIs. This function performs search opeartions for all the
LTRs which are configured for the specified table. The results of
compare operation are compared with the LPM index obtained
using LocateLPM FibTblMgr software search API.
Note: This function can only perform single search per LTR;
*/
NlmErrNum_t NlmFibRefApp_DeviceCompare(
    NlmFibRefApp_RefApp *refAppData_p,
    nlm_u8 tblId,
    NlmCmPrefix *srchPrefix_p,
    nlm_u32 lpmPrefixIndex
    )
{
    nlm_u16 cbAddr;
    NlmDevCtxBufferInfo cbWriteInfo;
    NlmReasonCode reasonCode;
    NlmErrNum_t errNum;
	NlmDevMgr *devMgr_p = refAppData_p->m_devMgr_p;

    nlm_u8 srchNum;
    NlmDevCmpRslt srchRslt;
    nlm_u8 srchKey[NLM_FIB_REFAPP_MAX_SRCH_LEN_IN_BYTES];
    nlm_u16 startBit;
    nlm_u8 rsltPortNum;

	for(srchNum = 0; srchNum < refAppData_p->m_numSearches; srchNum++)
    {
        if(refAppData_p->m_info.m_srchInfo[srchNum].m_tblIdForParallelSrch0 == tblId) /* If the search is configured
                                                               for the specified table */
        {
            /* Construct the search key based on start bit value */
            NlmCm__memset(srchKey, 0, (NLMDEV_FIB_MAX_PREFIX_LENGTH/8));
            startBit = refAppData_p->m_info.m_srchInfo[srchNum].m_startBitForParallelSrch0;
            NlmCm__memcpy(&srchKey[NLM_FIB_REFAPP_MAX_SRCH_LEN_IN_BYTES - ((startBit + 1) >> 3)],
                          srchPrefix_p->m_data,
                          (srchPrefix_p->m_avail >> 3));

            /* Initialize the structure used by Compare1 Device Manager API
               pass key for compare operation */

            cbAddr = (nlm_u16)(NLM_FIB_REFAPP_RANDOM_WORD % NLMDEV_CB_DEPTH);
            cbWriteInfo.m_cbStartAddr = ((cbAddr >> 3) << 3);/* CB Addr should be at address which is multiple of 8 */
            cbWriteInfo.m_datalen = NLMDEV_FIB_MAX_PREFIX_LENGTH >> 3;
            NlmCm__memcpy(&cbWriteInfo.m_data, srchKey, NLMDEV_FIB_MAX_PREFIX_LENGTH >> 3);

            if((errNum = NlmDevMgr__Compare1(devMgr_p,
                                               refAppData_p->m_info.m_srchInfo[srchNum].m_ltrNum,
                                               &cbWriteInfo,
                                               &srchRslt,
                                               &reasonCode)) != NLMERR_OK)
            {
                NlmCm__printf("\n\n\tError: NlmDevMgr__Compare1 -- ReasonCode: %d \n", reasonCode);
                return errNum;
            }

            /* Get the result port number which was configured for the result */
            rsltPortNum = refAppData_p->m_info.m_srchInfo[srchNum].m_rsltPortForParallelSrch0;

            if(srchRslt.m_hitOrMiss[rsltPortNum] == (Nlm11kDevMissHit)NLMDEV_MISS)
            {
                if(lpmPrefixIndex != NLM_FIB_INVALID_INDEX)
                {
                    /* If the compare result is miss and expected result is not miss then report an error */
                    NlmCm__printf("\n\n\tError: Device Compare Results In Miss (Expected Hit At Index 0x%x)",
                                    lpmPrefixIndex);
                    NlmCm__printf("\n\tCompare Done for tbl with tblId %d and LtrNum %d", tblId,
                                                    refAppData_p->m_info.m_srchInfo[srchNum].m_ltrNum);
                    NlmCm__printf("\n\tSrch Prefix:");
                    NlmCmPrefix__PrintAsIp(srchPrefix_p, NlmCm__stderr);
                    return NLMERR_FAIL;
                }
            }
            else
            {
                if(lpmPrefixIndex != srchRslt.m_hitIndex[rsltPortNum])
                {
                    /* If the compare result is hit, but the index does not match with the expected
                    index then report an error */
                    NlmCm__printf("\n\n\tError: Device Compare Results In Hit At Incorrect Index 0x%x",
                                            srchRslt.m_hitIndex[rsltPortNum]);
                    NlmCm__printf("(Expected Hit At Index 0x%x)", lpmPrefixIndex);
                    NlmCm__printf("\n\tCompare Done for tbl with tblId %d and LtrNum %d", tblId,
                                            refAppData_p->m_info.m_srchInfo[srchNum].m_ltrNum);
                    NlmCm__printf("\n\tSrch Prefix:");
                    NlmCmPrefix__PrintAsIp(srchPrefix_p, NlmCm__stderr);
                    return NLMERR_FAIL;
                }
            }
        }
    }
    return NLMERR_OK;
}

/* PerformLPMSearches function performs Longest Prefix Match
    searches for all the prefixes added to the tables. This
    function performs searches using the Device Manager Compare API
    and then verifies the results with the results obtained using LocateLPM
    FibTblMgr API
 */
NlmErrNum_t NlmFibRefApp_PerformLPMSearches(
    NlmFibRefApp_RefApp *refAppData_p
    )
{
    nlm_u8 tblNum;
    nlm_u32 iter;
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;
    nlm_u32 lpmIndex;
    NlmCmPrefix *srchPrefix_p;

    for(tblNum = 0; tblNum < refAppData_p->m_numOfTables; tblNum++)
    {
        /* Create prefix data structure using the utility function */
        srchPrefix_p = NlmCmPrefix__create(
			refAppData_p->m_alloc_p, refAppData_p->m_info.m_tableInfo[tblNum].m_tblWidth, 0, NULL);

		NlmCm__printf("\n\t\t Table Num:%d\n", refAppData_p->m_info.m_tableInfo[tblNum].m_tblId);

        for(iter = 0; iter < refAppData_p->m_info.m_tableInfo[tblNum].m_numOfPfxsToBeAdded;)
        {
             /* Generate a prefix to be searched;
            Prefixes generated are in incremental order */
            if((errNum = NlmFibRefApp_GeneratePrefix(srchPrefix_p, refAppData_p->m_type,
                                                     refAppData_p->m_info.m_tableInfo[tblNum].m_tblWidth,
                                                     refAppData_p->m_info.m_tableInfo[tblNum].m_tblId,
                                                     iter)) != NLMERR_OK)
                return errNum;

            /* Perform software search for the prefix to obtain its lpmIndex
            using the FibTblMgr API */
            if((errNum = NlmFibTblMgr__LocateLPM(refAppData_p->m_fibTbl_pp[tblNum],
                                                 srchPrefix_p,
                                                 &lpmIndex,
                                                 NULL,
                                                 &reasonCode)) != NLMERR_OK)
            {
                NlmCm__printf("\n\n\tError: NlmFibTblMgr__LocateLPM -- ReasonCode: %d\n", reasonCode);
                NlmCmPrefix__destroy(srchPrefix_p, refAppData_p->m_alloc_p);
                return errNum;
            }

            /* perform device compare operation to obtain the LPM for the search prefix
            and then verify the result with the LPM Index returned by FibTblMgr API */
            if((errNum = NlmFibRefApp_DeviceCompare(refAppData_p,
                                                    refAppData_p->m_info.m_tableInfo[tblNum].m_tblId,
                                                    srchPrefix_p,
                                                    lpmIndex)) != NLMERR_OK)
                return errNum;

			if(refAppData_p->m_type == NLM_FIB_REFAPP_TYPE_BATCH)
			{
				#ifdef DUMP_INDEX_CHANGES
					printf(m_p, "\n\t => PREFIX-ID: %d(%d)    LPM-INDEX: %d   HIT-INDEX: %d", iter, 
						refAppData_p->m_oldIdx[iter], lpmIndex, refAppData_p->m_newIdx[refAppData_p->m_oldIdx[iter]] );
				#endif
				if(iter != refAppData_p->m_oldIdx[iter] && 
					lpmIndex != refAppData_p->m_newIdx[refAppData_p->m_oldIdx[iter]])
				{
					NlmCm__printf("\n\t Miss-HIT: refApp index and device index miss-match");
					NlmCmPrefix__destroy(srchPrefix_p, refAppData_p->m_alloc_p);
					return errNum;				
				}
			}

			iter++;
			if((iter % 10000) == 0)
				NlmCm__printf("\n\t\t   - %d prefixes Searched successfully", iter);           
        }
        /* Destroy the prefix data structure using the utility function */
        NlmCmPrefix__destroy(srchPrefix_p, refAppData_p->m_alloc_p);
		NlmCm__printf("\n\t\t   - Total %d Prefixes Searched Successfuly in Table\n", 
			refAppData_p->m_info.m_tableInfo[tblNum].m_numOfPfxsToBeAdded);
    }
    return NLMERR_OK;
}


NlmErrNum_t NlmFibRefApp_DeviceCompareConfig2(
	NlmFibRefApp_RefApp *refAppData_p,
    NlmCmPrefix *srchPrefix_p,
    nlm_u32 s_lpmPrefixIndex,
	nlm_u32 l_lpmPrefixIndex
    )
{
    nlm_u16 cbAddr;
    NlmDevCtxBufferInfo cbWriteInfo;
    NlmReasonCode reasonCode;
    NlmErrNum_t errNum;

    NlmDevCmpRslt srchRslt;
    nlm_u8 srchKey[NLM_FIB_REFAPP_MAX_SRCH_LEN_IN_BYTES];
    nlm_u16 startBit;
    nlm_u8 rsltPortNum0;
	nlm_u8 rsltPortNum1;
	nlm_u8 ltr_num = 0;

	NlmDevMgr *devMgr_p = refAppData_p->m_devMgr_p;

            /* Construct the search key based on start bit value */
            NlmCm__memset(srchKey, 0, (NLMDEV_FIB_MAX_PREFIX_LENGTH/8));
			NlmCm__memset(&srchRslt, 0, sizeof(NlmDevCmpRslt));

			if(refAppData_p->m_isConfig2)
			{
				startBit = refAppData_p->m_info.m_specialSrch.m_StratBitInKey;
				ltr_num = refAppData_p->m_info.m_specialSrch.m_ltrNumber;
			}
			else 
			{
				startBit = refAppData_p->m_info.m_srchInfo[0].m_startBitForParallelSrch0;
				ltr_num = refAppData_p->m_info.m_srchInfo[0].m_ltrNum;
			}

            NlmCm__memcpy(&srchKey[NLM_FIB_REFAPP_MAX_SRCH_LEN_IN_BYTES - ((startBit + 1) >> 3)],
                          srchPrefix_p->m_data,
                          (srchPrefix_p->m_avail >> 3));

            /* Initialize the structure used by Compare1 Device Manager API
               pass key for compare operation */

            cbAddr = 0;
            cbWriteInfo.m_cbStartAddr = ((cbAddr >> 3) << 3);/* CB Addr should be at address which is multiple of 8 */
            cbWriteInfo.m_datalen = NLMDEV_FIB_MAX_PREFIX_LENGTH >> 3;
            NlmCm__memcpy(&cbWriteInfo.m_data, srchKey, NLMDEV_FIB_MAX_PREFIX_LENGTH >> 3);

            if((errNum = NlmDevMgr__Compare1(devMgr_p,
											  ltr_num,
                                               &cbWriteInfo,
                                               &srchRslt,
                                               &reasonCode)) != NLMERR_OK)
            {
                NlmCm__printf("\n\n\tError: NlmDevMgr__Compare1 -- ReasonCode: %d \n", reasonCode);
                return errNum;
            }

			if(refAppData_p->m_isConfig2)
			{
				rsltPortNum0 = refAppData_p->m_info.m_specialSrch.m_parallelSearch0ResultPort;
				rsltPortNum1 = refAppData_p->m_info.m_specialSrch.m_parallelSearch1ResultPort;

				/* check for HIT/MISS */
				if((srchRslt.m_hitOrMiss[rsltPortNum0] != 1) && (srchRslt.m_hitOrMiss[rsltPortNum1] != 1))
				{
					printf("\n\t Got MISS for searches");
				}
				
				/* check for Index Match */
				if((s_lpmPrefixIndex != srchRslt.m_hitIndex[rsltPortNum0] ) && 
					(l_lpmPrefixIndex != srchRslt.m_hitIndex[rsltPortNum1]))
				{
					printf("\n\t Got hit at incorrect index0 = %d  index1 = %d",s_lpmPrefixIndex,l_lpmPrefixIndex);
				}

				/*check for Hit from Both Tables*/
				if((srchRslt.m_hitOrMiss[rsltPortNum0] == 1) && (srchRslt.m_hitOrMiss[rsltPortNum1] == 1))
				{
					if((s_lpmPrefixIndex != srchRslt.m_hitIndex[rsltPortNum0] ) || 
					(l_lpmPrefixIndex != srchRslt.m_hitIndex[rsltPortNum1]))
					{
						printf("\n\t Got hit at incorrect index0 = %d  index1 = %d",s_lpmPrefixIndex,l_lpmPrefixIndex);
					}
					else printf("\n\t Got Hit from both tables : Picking hit from long prefixes table, index  = %d",l_lpmPrefixIndex);
				}
			}
			else
			{

				rsltPortNum0 = refAppData_p->m_info.m_srchInfo[0].m_rsltPortForParallelSrch0;

				/* check for HIT/MISS */
				if(srchRslt.m_hitOrMiss[rsltPortNum0] != 1)
				{
					printf("\n\t Got MISS for search 0");
				}
				
				/* check for Index Match */
				if(s_lpmPrefixIndex != srchRslt.m_hitIndex[rsltPortNum0] )
				{
					printf("\n\t Got hit at incorrect index %d",s_lpmPrefixIndex);
				}
			}

    return NLMERR_OK;
}


NlmErrNum_t NlmFibRefApp_PerformLPMSearchesForConfig2(
    NlmFibRefApp_RefApp *refAppData_p
    )
{
    nlm_u8 tblNum;
    nlm_u32 iter;
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;
    nlm_u32 lpmIndex,index_s,index_l;
    NlmCmPrefix *srchPrefix_p;
	nlm_u8 changedTblId = 0;
	nlm_u8 originalTblId = 0;
	nlm_u8 i = 0;

    for(tblNum = 0; tblNum < refAppData_p->m_numOfTables; tblNum++)
    {
		if(refAppData_p->m_isConfig2)
		if(tblNum == 1)
			continue;

		index_s = index_l = 0;

        /* Create prefix data structure using the utility function */
        srchPrefix_p = NlmCmPrefix__create(
			refAppData_p->m_alloc_p, refAppData_p->m_info.m_tableInfo[tblNum].m_tblWidth, 0, NULL);

		NlmCm__printf("\n\t\t Table Num:%d\n", refAppData_p->m_info.m_tableInfo[tblNum].m_tblId);

        for(iter = 0; iter < refAppData_p->m_info.m_tableInfo[tblNum].m_numOfPfxsToBeAdded;)
        {
			index_s = index_l = 0;

             /* Generate a prefix to be searched;
            Prefixes generated are in incremental order */
            if((errNum = NlmFibRefApp_GeneratePrefixForConfig2(srchPrefix_p,
                                                     refAppData_p->m_info.m_tableInfo[tblNum].m_tblWidth,
                                                     refAppData_p->m_info.m_tableInfo[tblNum].m_tblId,
                                                     iter)) != NLMERR_OK)
                return errNum;

			for(i=0 ; i < refAppData_p->m_numOfTables ;i++)
			{
				if((srchPrefix_p->m_inuse <= 80) && (i == 1))
					continue;

				/* if longer prefix change the table Id */
				if(refAppData_p->m_isConfig2)
				if((tblNum == 0) && (srchPrefix_p->m_inuse > 80) && (i==1))
				{
					changedTblId = 1;
					originalTblId = srchPrefix_p->m_data[0];
					srchPrefix_p->m_data[0] = ((srchPrefix_p->m_data[0] & 0x0f)|(0x10));
				}

				/* Perform software search for the prefix to obtain its lpmIndex
				using the FibTblMgr API */
				if((errNum = NlmFibTblMgr__LocateLPM(refAppData_p->m_fibTbl_pp[tblNum + changedTblId],
													srchPrefix_p,
													&lpmIndex,
													NULL,
													&reasonCode)) != NLMERR_OK)
				{
					NlmCm__printf("\n\n\tError: NlmFibTblMgr__LocateLPM -- ReasonCode: %d\n",
																				reasonCode);
					NlmCmPrefix__destroy(srchPrefix_p, refAppData_p->m_alloc_p);
					return errNum;
				}
				if(i == 0)
				{
					index_s = lpmIndex;
				}
				else if(i == 1)
				{
					index_l = lpmIndex;
				}
			}
			/* if longer prefix restore the table Id for Device search*/
			if(refAppData_p->m_isConfig2)
			if((tblNum == 0) && srchPrefix_p->m_inuse > 80)
			{
				 srchPrefix_p->m_data[0] = originalTblId;
			}

            /* perform device compare operation to obtain the LPM for the search prefix
            and then verify the result with the LPM Index returned by FibTblMgr API */
            if((errNum = NlmFibRefApp_DeviceCompareConfig2(refAppData_p,
                                                    srchPrefix_p,
                                                    index_s,
													index_l)) != NLMERR_OK)
                return errNum;

			iter++;
			if(refAppData_p->m_isConfig2)
				changedTblId = 0;

			if((iter % 10000) == 0)
				NlmCm__printf("\n\t\t   - %d prefixes Searched successfully", iter);           
        }
        /* Destroy the prefix data structure using the utility function */
        NlmCmPrefix__destroy(srchPrefix_p, refAppData_p->m_alloc_p);
		NlmCm__printf("\n\t\t   - Total %d Prefixes Searched Successfuly in Table\n", 
			refAppData_p->m_info.m_tableInfo[tblNum].m_numOfPfxsToBeAdded);
    }
    return NLMERR_OK;
}


NlmErrNum_t NlmFibRefApp_DeletePrefixesForConfig2(
    NlmFibRefApp_RefApp *refAppData_p
    )
{
    nlm_u8 tblNum;
    nlm_u32 iter;
    NlmCmPrefix *prefix_p;
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;
	nlm_u8 ChangetblId = 0;
	nlm_u8 counter = 0;
	nlm_u32 PfxInTbl = 0;

    for(tblNum = 0; tblNum < refAppData_p->m_numOfTables; tblNum++)
    {
		if(refAppData_p->m_isConfig2)
		if(tblNum == 1)
			continue;

        iter = 0;/* Initilialize the iter value to 0 for each table */
		NlmCm__printf("\n\t\t Table Num:%d\n", refAppData_p->m_info.m_tableInfo[tblNum].m_tblId);
		
		PfxInTbl = refAppData_p->m_fibTbl_pp[tblNum]->m_numPrefixes;
        /* Create a prefix data structure using the utility function */
        prefix_p = NlmCmPrefix__create(refAppData_p->m_alloc_p,
                                       refAppData_p->m_info.m_tableInfo[tblNum].m_tblWidth,
                                       0, NULL);

        while(counter < refAppData_p->m_fibTbl_pp[tblNum]->m_numPrefixes)
        {
             /* Generate a prefix to be deleted;
            Prefixes generated are in incremental order */
            if((errNum = NlmFibRefApp_GeneratePrefixForConfig2(prefix_p,
                                                     refAppData_p->m_info.m_tableInfo[tblNum].m_tblWidth,
                                                     refAppData_p->m_info.m_tableInfo[tblNum].m_tblId,
                                                     iter)) != NLMERR_OK)
                return errNum;


			/* if longer Pfx change the TblID */
			if(refAppData_p->m_isConfig2)
			if((tblNum == 0)&& (prefix_p->m_inuse > 80))
			{
				ChangetblId = 1;
				prefix_p->m_data[0] = ((prefix_p->m_data[0] & 0x0f)| (0x10));
				PfxInTbl++;
			}

             /* delete the prefix using the Fib TblMgr api */
            if((errNum = NlmFibTblMgr__DeletePrefix(refAppData_p->m_fibTbl_pp[tblNum + ChangetblId],
                                                    prefix_p,
                                                    &reasonCode)) != NLMERR_OK)
            {
                if(reasonCode !=  NLMRSC_PREFIX_NOT_FOUND) /* If prefix is  not found generate
                                                           another prefix */
                {
                    NlmCm__printf("\n\n\tError: NlmFibTblMgr__DeletePrefix -- ReasonCode: %d\n",
                                                                               reasonCode);
                    NlmCmPrefix__destroy(prefix_p, refAppData_p->m_alloc_p);
                    return errNum;
                }
            }
            iter++;
			if(refAppData_p->m_isConfig2)
				ChangetblId = 0;

			if((iter % 10000) == 0)
				NlmCm__printf("\n\t\t   - %d prefixes Deleted from Table", iter);
        }
        /* Destroy the prefix data structure using the utility function */
        NlmCmPrefix__destroy(prefix_p, refAppData_p->m_alloc_p);
		NlmCm__printf("\n\t\t   - Total %d prefixes Deleted from Table\n", PfxInTbl);
    }
    return NLMERR_OK;
}

void NlmFibRefApp_RunNormalApp(NlmFibRefApp_RefApp *refAppData_p)
{
	NlmErrNum_t errNum = NLMERR_OK;

	 NlmCm__printf ("\n\n\t Adding Prefixes to Tables:\n");
    /* Load the LPM Tables */
    if((errNum = NlmFibRefApp_LoadLPMTables(refAppData_p)) != NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError NlmFibRefApp_LoadLPMTables");
        exit(0);
    }

    /* Perform LPM Search Operations if xpt type is not blackhole xpt*/
    NlmCm__printf ("\n\n\t Performing LPM Searches:\n");
    if((errNum = NlmFibRefApp_PerformLPMSearches(refAppData_p)) != NLMERR_OK)
    {
	NlmCm__printf("\n\n\tError NlmFibRefApp_PerformLPMSearches");
	exit(0);
    }

    NlmCm__printf ("\n\n\t Deleting Prefixes:\n");
    /* Delete some prefixes */
    if((errNum = NlmFibRefApp_DeletePrefixes(refAppData_p)) != NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError NlmFibRefApp_DeletePrefixes");
        exit(0);
    }

#ifndef NLM_XLP
    /* If Debug Logging is enabled, then log the details into log files */
	#if NLM_FIB_REFAPP_DEBUG_LOG
		{    
			NlmNsLog__PrintStats("Nlm_StatsLog.txt", NlmFalse);
			NlmNsLog__PrintInsertPfxInfo("Nlm_InsertPfxLog.txt",NlmFalse, 0, 0, NULL); /* for all 3 tables */

			NlmNsLog__PrintDeletePfxInfo("Nlm_DeletePfxLog.txt", NlmFalse, 0, 0);

			NlmNsLog__PrintAssertInfo("Nlm_AssertLog.txt", NlmFalse);

			NlmNsLog__PrintEventInfo("Nlm_EventInfo.txt", NlmFalse, 0, 0);

			NlmNsLog__PrintDebugInfo("Nlm_DebugInfo.txt", NlmFalse);
		}
	#endif
#endif
    /* Perform LPM Search Operations if xpt type is not blackhole xpt*/
    {
        NlmCm__printf ("\n\n\t Performing LPM Searches:\n");
        if((errNum = NlmFibRefApp_PerformLPMSearches(refAppData_p)) != NLMERR_OK)
        {
            NlmCm__printf("\n\n\tError NlmFibRefApp_PerformLPMSearches");
            exit(0);
        }
    }

	return;
}


void NlmFibRefApp_RunConfigSrchApp(NlmFibRefApp_RefApp *refAppData_p)
{
	NlmErrNum_t errNum = NLMERR_OK;

	srand(0x12345);
	 NlmCm__printf ("\n\n\t Adding Prefixes to Tables:\n");
    /* Load the LPM Tables */
    if((errNum = NlmFibRefApp_LoadLPMTablesForConfig2(refAppData_p)) != NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError NlmFibRefApp_LoadLPMTablesForConfig2");
        exit(0);
    }

	srand(0x12345);
    /* Perform LPM Search Operations if xpt type is not blackhole xpt*/
    {
        NlmCm__printf ("\n\n\t Performing LPM Searches:\n");
        if((errNum = NlmFibRefApp_PerformLPMSearchesForConfig2(refAppData_p)) != NLMERR_OK)
        {
            NlmCm__printf("\n\n\tError NlmFibRefApp_PerformLPMSearchesForConfig2");
            exit(0);
        }
    }

	srand(0x12345);
    NlmCm__printf ("\n\n\t Deleting Prefixes:\n");
    /* Delete some prefixes */
    if((errNum = NlmFibRefApp_DeletePrefixesForConfig2(refAppData_p)) != NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError NlmFibRefApp_DeletePrefixesForConfig2");
        exit(0);
    }

#ifndef NLM_XLP
    /* If Debug Logging is enabled, then log the details into log files */
	#if NLM_FIB_REFAPP_DEBUG_LOG
		{    
			NlmNsLog__PrintStats("Nlm_StatsLog.txt", NlmFalse);
			NlmNsLog__PrintInsertPfxInfo("Nlm_InsertPfxLog.txt",NlmFalse, 0, 0, NULL); /* for all 3 tables */

			NlmNsLog__PrintDeletePfxInfo("Nlm_DeletePfxLog.txt", NlmFalse, 0, 0);

			NlmNsLog__PrintAssertInfo("Nlm_AssertLog.txt", NlmFalse);

			NlmNsLog__PrintEventInfo("Nlm_EventInfo.txt", NlmFalse, 0, 0);

			NlmNsLog__PrintDebugInfo("Nlm_DebugInfo.txt", NlmFalse);
		}
	#endif
#endif
	return;
}

void NlmFibRefApp_RunBatchApp(NlmFibRefApp_RefApp *refAppData_p)
{
	NlmErrNum_t errNum = NLMERR_OK;
	nlm_u32 idxCntNonBatch = 0, idxCntBatch = 0;

#ifdef WIN32
	LARGE_INTEGER startTime = {0};
	LARGE_INTEGER endTime = {0};
	LARGE_INTEGER qfreq = {0};
#elif defined(VXWORKS)
	timeval startTime = {0};
	timeval endTime = {0};
#else
	struct timeval startTime = {0};
	struct timeval	endTime = {0};
#endif
	
#ifdef WIN32
	QueryPerformanceFrequency(&qfreq);
#endif

	/*Timer is started here*/
	NlmFibRefApp_GetTime(startTime);

	refAppData_p->m_assoDataCBCnt = 0;
	if(refAppData_p->m_isBatch == NlmTrue)
	{
		/* Load the LPM Tables: if BEGIN_COUNT is 0 then we directly call the batch prefix
		   insertion, else application will start using the batch mode after inserting
		   prefixes count BEGIN_COUNT by using __AddPrefix() API */
		NlmCm__printf ("\n\n\t Adding Prefixes to Table:\n");
		if((errNum = NlmFibRefApp_LoadLPMTables(refAppData_p)) != NLMERR_OK)
		{
			NlmCm__printf("\n\n\tError NlmFibRefApp_LoadLPMTables");
			exit(0);
		}
		idxCntNonBatch = refAppData_p->m_assoDataCBCnt;
		refAppData_p->m_assoDataCBCnt = 0;

		/* Load the LPM Tables: after reaching BEGIN_COUNT __AddPrefixBatch() API use to 
		   insert the prefixes in batch */
        NlmCm__printf ("\n\n\t Adding Batch of Prefixes to Table:\n");
		if((errNum = NlmFibRefApp_LoadLPMTables_Batch(refAppData_p)) != NLMERR_OK)
		{
			NlmCm__printf("\n\n\tError NlmFibRefApp_LoadLPMTables_Batch");
			exit(0);
		}

		/*Stop timer here and calculate the total time taken. */
  		NlmFibRefApp_GetTime(endTime);
		refAppData_p->m_pfxInsertTime = NlmFibRefApp_CalcTimeDiff(endTime,startTime);

        idxCntBatch = refAppData_p->m_assoDataCBCnt;

		NlmCm__printf("\n\t **********************************************************");
		NlmCm__printf("\n\t Index change callbacks      = %d (Non-Batch:%d, Batch:%d) \n",
			(idxCntBatch + idxCntNonBatch), idxCntNonBatch, idxCntBatch);
	}
	else
	{
		refAppData_p->m_startAt = refAppData_p->m_info.m_tableInfo[0].m_numOfPfxsToBeAdded;
		/* Load the LPM Tables */
		NlmCm__printf ("\n\n\t Adding Prefixes to Table:\n");
		if((errNum = NlmFibRefApp_LoadLPMTables(refAppData_p)) != NLMERR_OK)
		{
			NlmCm__printf("\n\n\tError NlmFibRefApp_LoadLPMTables");
			exit(0);
		}

		/*Stop timer here and calculate the total time taken. */
  		NlmFibRefApp_GetTime(endTime);
		refAppData_p->m_pfxInsertTime = NlmFibRefApp_CalcTimeDiff(endTime,startTime);

		NlmCm__printf("\n\t **********************************************************");
		NlmCm__printf("\n\t Index change callbacks      = %d \n", refAppData_p->m_assoDataCBCnt); 
	}

	NlmCm__printf("\n\t Number of prefixes inserted = %d",refAppData_p->m_info.m_tableInfo[0].m_numOfPfxsToBeAdded);
	NlmCm__printf("\n\t Time taken for insertions   = %.2f sec", (double)(refAppData_p->m_pfxInsertTime)/1000000.00);
	NlmCm__printf("\n\t Update Rate                 = %.2f K/Sec \n\n",
		(refAppData_p->m_info.m_tableInfo[0].m_numOfPfxsToBeAdded)/ ((double)(refAppData_p->m_pfxInsertTime)/1000.00));
	
	NlmCm__printf("\n\t **********************************************************\n\n");

    {
        NlmCm__printf ("\n\n\t Performing LPM Searches:\n");
        if((errNum = NlmFibRefApp_PerformLPMSearches(refAppData_p)) != NLMERR_OK)
        {
            NlmCm__printf("\n\n\tError NlmFibRefApp_PerformLPMSearches");
            exit(0);
        }
    }

	NlmCm__printf ("\n\n\t Deleting Prefixes:\n");
    /* Delete some prefixes */
    if((errNum = NlmFibRefApp_DeletePrefixes(refAppData_p)) != NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError NlmFibRefApp_DeletePrefixes");
        exit(0);
    }

	return;
}

int nlmfibtblmgr_refapp_main(int argc, char *argv[])
{
    NlmFibRefApp_RefApp refAppData;
    NlmCmAllocator allocBody;
    nlm_32 break_alloc_id = -1;
    NlmErrNum_t errNum;
    nlm_32 cmdArgs = 1;
	
	refAppData.m_startAt      = 0;
	refAppData.m_batchSize    = 0;
	refAppData.m_inxCnt       = 0;
	refAppData.m_isConfig2    = NlmFalse;
	refAppData.m_isBatch      = NlmFalse;
	refAppData.m_type         = NLM_FIB_REFAPP_TYPE_NORMAL;
	refAppData.m_xptType      = NLM_FIB_REFAPP_SIMULATION_XPT;
	refAppData.m_numOfDevices = 1;
	
    NlmCm__printf("\n\n\t**************************************************************************");
    NlmCm__printf ("\n\t  FIB Table Manager Reference Application \n");
    NlmCm__printf("\n\t**************************************************************************\n\n");


    /* Create general purpose allocator */
    refAppData.m_alloc_p = NlmCmAllocator__ctor(&allocBody);
    NlmCmAssert((refAppData.m_alloc_p != NULL), "The memory allocator is NULL!\n");
    NlmCmDebug__Setup(break_alloc_id, NLMCM_DBG_EBM_ENABLE);

#ifndef NLM_XLP
    while(cmdArgs < argc)
    {
		/* first level command line arguments */
        if((NlmCm__strcmp(argv[cmdArgs], "-chips") == 0))
        {
            cmdArgs++;
            if ((cmdArgs < argc) && NlmCm__isdigit((nlm_32)(argv[cmdArgs][0])))
                refAppData.m_numOfDevices = (nlm_u8)atoi(argv[cmdArgs]);
            if(refAppData.m_numOfDevices > NLM_FIB_REFAPP_MAX_NUM_DEVICES || refAppData.m_numOfDevices == 0)
                break;
        }
        else if((NlmCm__strcmp(argv[cmdArgs], "-cflow") == 0))
		{
			refAppData.m_xptType = NLM_FIB_REFAPP_SIMULATION_XPT;
		}
	else if((NlmCm__strcmp(argv[cmdArgs], "-normal") == 0))
		{
			refAppData.m_type = NLM_FIB_REFAPP_TYPE_NORMAL;

			/* disable the other types */
			refAppData.m_isConfig2 = NlmFalse;
			refAppData.m_startAt   = 0;
			refAppData.m_batchSize = 0;
			refAppData.m_isBatch   = NlmFalse;
		}
        else if(NlmCm__strcmp(argv[cmdArgs], "-batch") == 0)
		{
			refAppData.m_type = NLM_FIB_REFAPP_TYPE_BATCH;

			/* disable the other types */
			refAppData.m_isConfig2 = NlmFalse;
			refAppData.m_startAt   = 0;
			refAppData.m_batchSize = 0;
			refAppData.m_isBatch   = NlmFalse;
		}
		else if((NlmCm__strcmp(argv[cmdArgs], "-capacity") == 0))
		{
			refAppData.m_type = NLM_FIB_REFAPP_TYPE_CONFIG2;
			
			/* disable the other types */
			refAppData.m_isConfig2 = NlmFalse;
			refAppData.m_startAt   = 0;
			refAppData.m_batchSize = 0;
			refAppData.m_isBatch   = NlmFalse;
		}
		else
		{
			/* second level parameters
			check these parameters only for config2 and batch mode */
			if(refAppData.m_type == NLM_FIB_REFAPP_TYPE_CONFIG2)
			{
				if((NlmCm__strcmp(argv[cmdArgs], "-config2") == 0))
					refAppData.m_isConfig2 = NlmTrue;
				else
					break;
			}
			else if(refAppData.m_type == NLM_FIB_REFAPP_TYPE_BATCH)
			{
				if((NlmCm__strcmp(argv[cmdArgs], "-bmode") == 0))
				{
					refAppData.m_isBatch   = NlmTrue;
				}
				else if((NlmCm__strcmp(argv[cmdArgs], "-bstart") == 0))
				{
					cmdArgs++;
					if ((cmdArgs < argc) && NlmCm__isdigit((nlm_32)(argv[cmdArgs][0])))
						refAppData.m_startAt = (nlm_u32)atoi(argv[cmdArgs]);
					if(refAppData.m_startAt >= NLM_FIB_REFAPP_BATCH_MAX_SIZE_COUNT)
						break;
				}
				else if((NlmCm__strcmp(argv[cmdArgs], "-bsize") == 0))
				{
					cmdArgs++;
					if ((cmdArgs < argc) && NlmCm__isdigit((nlm_32)(argv[cmdArgs][0])))
						refAppData.m_batchSize = (nlm_u8)atoi(argv[cmdArgs]);
					if(refAppData.m_batchSize < 1 || refAppData.m_batchSize > NLM_FIB_REFAPP_BATCH_SIZE_ARRAY_COUNT)
						break;
				}
				else
					break;
			}
			else
				break;
		}

        cmdArgs++;
    }
    if(cmdArgs != argc)
    {
		NlmCm__printf("\n\n\tCommand Line Args Usage:");
		NlmCm__printf("\n\t************************\n");
		NlmCm__printf("\t-------------------------------------------------------------------------------\n");
        NlmCm__printf("\t-chips <1-4> \n");
        NlmCm__printf("\t-blackhole \n");
		NlmCm__printf("\t-cflow \n");
		NlmCm__printf("\t-------------------------------------------------------------------------------\n");
		NlmCm__printf("\t-normal  (order to use: -chips <1-4> -blackhole/-cflow) \n\n");
		NlmCm__printf("\t-capcity (order to use: -blackhole/-cflow -capacity -config2)\n\n");
		NlmCm__printf("\t    -config2: shows the capacity improvement \n\n");
		NlmCm__printf("\t-batch   (order to use: -blackhole/-cflow -batch -bmode -bstart <val> -bsize <1-4>) \n\n");
		NlmCm__printf("\t    -bmode  : 0 run in normal mode, 1 run in batch mode \n");
		NlmCm__printf("\t    -bstart : inserts <value> prefixes in normal mode and later in batch \n");
		NlmCm__printf("\t    -bsize  : batch size 1: 500, 2: 1000, 3: 1500, 4:2000 (default round-robin) \n");
		NlmCm__printf("\t-------------------------------------------------------------------------------\n");
		NlmCm__printf("\tNote: order is mandatory for -capacity and -batch types \n");
		NlmCm__printf("\t-------------------------------------------------------------------------------\n");
        return 0;
    }
#else  /* NLM_XLP */
	refAppData.m_xptType = NLM_FIB_REFAPP_XLP_XPT;
#endif /* NLM_XLP */

#ifdef NLMPLATFORM_BCM 
	refAppData.m_xptType = NLM_FIB_REFAPP_CALADAN3_XPT;
#endif /* NLMPLATFORM_BCM */
	switch(refAppData.m_type)
	{
	    default:
		case NLM_FIB_REFAPP_TYPE_NORMAL:
			NlmCm__printf("\n\n\t _____________ Noral Mode ____________ \n\n");
			break;

		case NLM_FIB_REFAPP_TYPE_CONFIG2:
			NlmCm__printf("\n\n\t _____________ Config2 Mode ____________ \n\n");
			break;

		case NLM_FIB_REFAPP_TYPE_BATCH:
			NlmCm__printf("\n\n\t _____________ Batch Mode ____________ \n\n");
			break;
	}

    /* Initialize the Forwarding Inforamtion Base Subsystem */
    NlmCm__printf ("\n\t Initializing the FIB SubSystem:\n");
    if((errNum = NlmFibRefApp_InitForwardingSubSystem(&refAppData)) != NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError NlmFibRefApp_InitForwardingSubSystem ");
        exit(0);
    }

    NlmCm__printf ("\n\n\t Creating LPM Tables:\n");
    /* Create the LPM Tables */
    if((errNum = NlmFibRefApp_CreateLPMTables(&refAppData)) != NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError NlmFibRefApp_CreateLPMTables ");
        exit(0);
    }

    NlmCm__printf ("\n\n\t Configuring Searches:\n");
    /* Configure the searches */
	if(!refAppData.m_isConfig2)
	{
		if((errNum = NlmFibRefApp_ConfigSearches(&refAppData)) != NLMERR_OK)
		{
			NlmCm__printf("\n\n\tError NlmFibRefApp_ConfigSearches");
			exit(0);
		}
	}
	else
	{
		if((errNum = NlmFibRefApp_ConfigSearchesForConfig2(&refAppData)) != NLMERR_OK)
		{
			NlmCm__printf("\n\n\tError NlmFibRefApp_ConfigSearchesForConfig2 ");
			exit(0);
		}
	}

	/* select the proper application to show the usage */
	switch(refAppData.m_type)
	{
	    default:
		case NLM_FIB_REFAPP_TYPE_NORMAL:
				NlmFibRefApp_RunNormalApp(&refAppData);
				break;
		case NLM_FIB_REFAPP_TYPE_CONFIG2:
				NlmFibRefApp_RunConfigSrchApp(&refAppData);
				break;
		case NLM_FIB_REFAPP_TYPE_BATCH:
				NlmFibRefApp_RunBatchApp(&refAppData);
				break;
    }

    /* Destroy the Forwarding Subsystem */
    NlmFibRefApp_DestroyForwardingSubSystem(&refAppData);

    /* Destroy the allocator */
    NlmCmAllocator__dtor(refAppData.m_alloc_p);

    printf("\n\n\t**************************************************************************");
    printf("\n\t  FIB Table Manager Reference Application Completed");
    printf("\n\t**************************************************************************\n\n");

    /* Check for memory leaks */
    NlmCmBasic__Require(NlmCmDebug__IsMemLeak() == NlmFALSE);
	return 0;

}



