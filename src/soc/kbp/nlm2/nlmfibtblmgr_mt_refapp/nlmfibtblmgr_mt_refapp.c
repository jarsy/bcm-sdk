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
 #include "nlmfibtblmgr_mt_refapp.h"
#include "nlmnslog.h"

#ifdef NLM_XLP
#include "nlmxlpxpt.h"
#endif

NlmFtmRefApp_RefAppInfo		*g_refAppData_ptr;

volatile nlm_u8				getGo[ NLM_FIB_REFAPP_THREADS_COUNT + 1];
volatile nlm_u8				checkJoin[ NLM_FIB_REFAPP_THREADS_COUNT + 1];
char						*addThrStack[NLM_FIB_REFAPP_THREADS_COUNT];
NlmCmMtThreadId				addThrdId[NLM_FIB_REFAPP_THREADS_COUNT];
nlm_u32						threadId[NLM_FIB_REFAPP_THREADS_COUNT];

NlmFtmRefApp_threadTime		threadTime[NLM_FIB_REFAPP_THREADS_COUNT];


typedef struct NlmFtmRefApp_SrchInfo_s
{
    nlm_u8 m_ltrNum;                     /* ltr number to
                                         be configured */
    nlm_u8 m_numOfParallelSrches;        /* number of searches
                                         to be configured */
    nlm_u8 m_tblIdForParallelSrch0;      /* tblId for one of
                                         the search operation */
    nlm_u8 m_tblIdForParallelSrch1;      /* tblId for second search operation,
                                         Not used in case of single searches */
    nlm_u8 m_rsltPortForParallelSrch0;   /* rslt port number for
                                         one of the search operation */
    nlm_u8 m_rsltPortForParallelSrch1;   /* rslt port number for second search
                                         operation, Not used in case of single searches */
    nlm_u16 m_startBitForParallelSrch0;  /* prefix start bit in master key for one
                                         of the search operation */
    nlm_u16 m_startBitForParallelSrch1; /* prefix start bit in master key for second
                                        search operation, Not used in case of single searches*/
} NlmFtmRefApp_SrchInfo;


static NlmFtmRefApp_SrchInfo srchInfo[NLM_FIB_REFAPP_NUM_SRCH_CONFIG] = /* Search attributes
                                                                        being used in the refapp */
                                {
                                    {0, 1, 0, 0, 0, 0, 159, 0}		/* ltrNum: 0, Number of ps: 1,
																		tblId: 0, rslt port num: 0,
																		startbit: 159*/
                                };


/* Functions for MT */
static NlmErrNum_t NlmFtmRefApp_InsertPfxsToTbls(void *threadId);


/* This function bind the thread to given CPU (core) */
static nlm_u32 
NlmFtmRefApp_pvt_BindThreadToCore(nlm_u32 cpuid)
{
#ifndef NLM_NETOS 
	cpu_set_t cpu_mask;
    nlm_u32 ret;

    CPU_ZERO(&cpu_mask);
    CPU_SET(cpuid, &cpu_mask);

    ret = pthread_setaffinity_np( pthread_self(), sizeof(cpu_mask), &cpu_mask);
    if(ret != 0)
	{
		return 1;
	}
#else
    (void)cpuid;
#endif
	return 0;
}

/* This function prints the statistics like total prefixes insterted and the
   time taken to insert the prefixes (update rate)

   update rate = total capacity / long pole time by the thread
*/
static NlmErrNum_t 
NlmFtmRefApp_FtmAddStats(NlmFtmRefApp_RefAppInfo *refAppData_p)
{
	NlmErrNum_t errNum = 0;
	nlm_u32 uniquePfxCount = 0;
	nlm_u8 in = 0, whichThread = 0;
	nlm_u32 pfxInsertTime = 0;

	NlmCm__printf("\n\n\n\t PERFORMANCE STATISTICS:");
	NlmCm__printf("\n\t ---------------------- ");

	NlmCm__printf("\n\t\t ____________________________________________________________________\n");
	NlmCm__printf("\t\t Thread_Num       Prefixes         Time (Sec)      Update_Speed (K/Sec)");
	NlmCm__printf("\n\t\t ____________________________________________________________________\n");
		
	for(in = 0; in < refAppData_p->m_numInsThrds; in++)
	{
		uniquePfxCount += threadTime[in].th_inCount;

		/* calculate the time taken by the thread */

		NlmCm__printf("\n\t\t\t %d \t   %d \t   %.3f   \t   %.2f", in, threadTime[in].th_inCount,
			(double)(threadTime[in].th_insTime)/1000000.00,
			threadTime[in].th_inCount / ( (double)(threadTime[in].th_insTime) /1000.00 ) );

		if(pfxInsertTime < threadTime[in].th_insTime)
		{
			pfxInsertTime = threadTime[in].th_insTime;
			whichThread = in;
		}		
	}

	NlmCm__printf("\n\n\t\t   Thread-%d took maximum time [%.3f] (long pole)", whichThread,
		(double)(threadTime[whichThread].th_insTime)/1000000.00 );

	/* Print the update rate here */
	NlmCm__printf("\n\n\t\t  Total Insertion Time : %f secs", (double)(pfxInsertTime/1000000.00));
	NlmCm__printf("\n\t\t  Prefixes Inserted    : %.4u", uniquePfxCount);
	NlmCm__printf("\n\t\t  Update Rate          : %.2f K/Sec \n", (uniquePfxCount/((double)(pfxInsertTime)/1000.00)) );
	NlmCm__printf("\n\t\t ____________________________________________________________________\n\n");

	return errNum;
}

/* This function create the add threads (prefixes are added concurrenlty by different threads)
   once each add prefix thread complete with the table full or if it finish its quota it will 
   exits.
   later statistics will be printed
*/
static NlmErrNum_t 
NlmFtmRefApp_CreateFtmThreads( NlmFtmRefApp_RefAppInfo *refAppData_p )
{
	NlmErrNum_t errNum = 0;	
	
	/* add craete and add threads to the main */
	{
		nlm_u8 howMany, sum = 0;
		void* thread_Attr = NULL;

		/* Thread Creation */
		{
			/* === FTM Threads ===== */		 
			for(howMany = 0; howMany < refAppData_p->m_numInsThrds; howMany++)
			{
#ifdef NLM_NETOS
			int  size = 64 * 1024;

			if((addThrStack[howMany] = (char*) malloc(size)) == NULL)
			{
				NlmCm__printf("Stack memory allocation failed for Add thread #%d\n", howMany);
				return NLMERR_FAIL;
			}

			/* stack grows from top to bottom  */
			thread_Attr = addThrStack[howMany] + size - 64;
			
#endif
			threadId[howMany] = howMany;
			errNum = NlmCmMt__CreateThread(&addThrdId[howMany], 
							thread_Attr,
							(void*)NlmFtmRefApp_InsertPfxsToTbls, 
							(void *)&threadId[howMany]);
				NlmCmAssert(errNum == NLMERR_OK, "__CreateThread Failed \n");

			}
		}
		 		
		{
            /* Wait till all threads are bind to the cores. This is needed as few threads 
			   are getting binded to core earlier than other threads and start inserting prefixes */
			while(1)
			{
				for(sum = 0, howMany = 0; howMany < refAppData_p->m_numInsThrds; howMany++)
					sum += getGo[howMany];

				if(sum == refAppData_p->m_numInsThrds)
					break;
			}

			getGo[refAppData_p->m_numInsThrds] = 1;

#ifdef NLM_NETOS
			while(1)
			{
				for(sum = 0, howMany = 0; howMany < refAppData_p->m_numInsThrds; howMany++)
					sum += checkJoin[howMany];

				if(sum == refAppData_p->m_numInsThrds)
					break;
			}
#else
			for(howMany = 0; howMany < refAppData_p->m_numInsThrds; howMany++)
				NlmCmMt__JoinThread(&addThrdId[howMany], NULL);
#endif
		}
		
		if((errNum = NlmFtmRefApp_FtmAddStats(refAppData_p))!= NLMERR_OK)
            return errNum;
	}
	return NLMERR_OK;
}


/* Application prefix Index change call back function needs to be implemented by 
   the application and should be registered to the FibTblMgr by passing the pointer
   to it as an argument to the NlmFibTblMGr__Init api along with the client pointer.

   This application callback function is invoked by FibTblMgr to inform the application 
   about the changes occuring in the index of the prefixes added as a result of shuffling
   of prefixes across the device; Also the index of the new prefix being added is informed 
   to the application via this callback only. In this case oldIndex = NLM_FIB_INVALID_INDEX.
   
   Basically the application can use this function to update the associated data which is 
   associated with the prefixes added to the table */

void NlmFtmRefApp_PfxIndexChangedCallBack(
    void *client_p,				/* Application specific data which can be used by application
								   to update the associated data information whenever there is
								   change in the indices of the prefixes added */
    void *fibTbl,				/* table to which the prefix whose index is being changed 
								   belongs */
    NlmFibPrefixIndex oldIndex, /* old index of the prefix whose index is being changed ;
                                   In case of new prefix, the value of this argument
                                   will be NLM_FIB_INVALID_INDEX */
    NlmFibPrefixIndex newIndex /* new index of the prefix whose index is being changed */
    )
{
    (void)client_p;
    (void)fibTbl;
    (void)oldIndex;
    (void)newIndex;
}

/* InitForwardingSystem function performs initializiation of software in order to 
   allow the application to perform FIB Related operations such as creating FIB 
   tables, configuring searches, adding prefixes, performing searches and so on.

   Basically it creates Simulation/Blackhole/XLP transport interface, creates 
   Device Manager instance, adds devices and creates FIB Table Manager instance
*/
NlmErrNum_t NlmFtmRefApp_InitForwardingSubSystem(
    NlmFtmRefApp_RefAppInfo *refAppData_p
    )
{
	nlm_u32 ret = 0;
    nlm_u8 devNum;
    NlmFibBlksRange fibBlksRange[NLM_FIB_REFAPP_MAX_NUM_DEVICES];
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;

    refAppData_p->m_operMode = NLMDEV_OPR_SAHASRA;

	/* Initialize the spin locks used */
	ret = NlmCmMt__SpinInit(&refAppData_p->m_refApp_spinLock, 
								"NlmFtmRefApp_Kbp_SpinLock",
								NlmCmMtFlag);
	if(ret != 0)
	{
		NlmCm__printf("\n\n\t RefApp spinlock_init failed.\n");
		exit(1);
	}

    switch(refAppData_p->m_xptType)
    {
#ifndef NLMPLATFORM_BCM
        case NLM_FIB_REFAPP_SIMULATION_XPT:
		{
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
		}

        case NLM_FIB_REFAPP_BLACKHOLE_XPT:
		{
            /* create the blackhole transport interface */
            if((refAppData_p->m_xpt_p = NlmBlackHole__Create(
                                                    refAppData_p->m_alloc_p,
                                                    NLM_FIB_REFAPP_MAX_RQT_COUNT,
                                                    NLM_FIB_REFAPP_MAX_RSLT_COUNT)) == NULL)
            {
                NlmCm__printf("\n\n\tError: NlmBlackHole__Create \n");
                return NLMERR_FAIL;
            }
            NlmCm__printf ("\n\t\t Black Hole Xpt created successfully \n");
            break;
		}

#ifdef NLM_XLP
		case NLM_FIB_REFAPP_XLP_XPT:
		{
		 /* Create XLP  transport interface */
		    refAppData_p->m_xpt_p = NlmXlpXpt__Create(refAppData_p->m_alloc_p,
		                                              NLM_DEVTYPE_2_S,
		                                              NLM_FIB_REFAPP_MAX_RQT_COUNT,
		                                              refAppData_p->m_operMode,
													  0,
													  0,    /* Context ID start range */
													  255, /* Context ID end range */
													  1    /* Init Link */,
													  1);
		    if(refAppData_p->m_xpt_p == NULL)
		    {
		        NlmCm__printf("\n\tXLP Transport Inteface Creation Failed.\n");
		        return NLMERR_FAIL;
		    }
		    NlmCm__printf("\n\tXLP  Transport Interface Created Successfully\n");
		    break;
		}
#endif
#endif /* NLMPLATFORM_BCM */
        case NLM_BCM_CALADAN3_MODE:
	case NLM_FIB_REFAPP_XPT_END:
        default:
		{
             /*If other transport interfaces such as Fpga Xpt are used then
                the corresponding Init function needs to be called here */
            NlmCm__printf("\n\n\tError: Incorrect XPT Type Specified \n");
            return NLMERR_FAIL;
		}
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

    /* Initialize the blocks allocated to FIB. Blocks 0-119 are used */
    for(devNum = 0; devNum < refAppData_p->m_numOfDevices; devNum++)
    {
        fibBlksRange[devNum].m_startBlkNum = 0;
        fibBlksRange[devNum].m_endBlkNum = NLM_FIB_BLK_RANGE_WHOLE_DEVICE;
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
													     NlmFtmRefApp_PfxIndexChangedCallBack,
													     refAppData_p,
                                                         &reasonCode)) == NULL)
	{
		NlmCm__printf("\n\n\tError: NlmFibTblMgr__Init -- ReasonCode: %d\n", reasonCode);
        return NLMERR_FAIL;
	}
    NlmCm__printf ("\n\t\t Fib Table Manager created successfully \n");

	return NLMERR_OK;
}

/* 
   DestroyForwardingSubSystem function destroys the instances of FIB Table Manager, 
   Device Manager and Simulation Transport Interface 
 */
void NlmFtmRefApp_DestroyForwardingSubSystem(
    NlmFtmRefApp_RefAppInfo *refAppData_p
    )
{
    NlmReasonCode reasonCode;

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

        case NLM_FIB_REFAPP_BLACKHOLE_XPT:
            NlmBlackHole__destroy(refAppData_p->m_xpt_p);
            break;

#ifdef NLM_XLP
		case NLM_FIB_REFAPP_XLP_XPT:
            NlmXlpXpt__Destroy(refAppData_p->m_xpt_p);
            break;
#endif

		case NLM_FIB_REFAPP_XPT_END:
#endif /* NLMPLATFORM_BCM */
        case NLM_BCM_CALADAN3_MODE:
        default:
             /*If other transport interfaces such as Fpga Xpt are used then
             the corresponding destroy function needs to be called here */
            break;

    }

	/* destroy the spin lock used */
	{
		nlm_u32 ret = 0;

		/* Destroy the spin locks used */
		ret = NlmCmMt__SpinDestroy(&refAppData_p->m_refApp_spinLock, 
			"NlmFtmRefApp_Kbp_SpinLock");
		if(ret != 0)
		{
			NlmCm__printf("\n\n\t arr spinlock_destroy failed.\n");
			exit(1);
		}
	}

}

/* TblIdStringConvert function converts an integer tblId into a string of zeroes 
   and ones as expected by the FibTblMgr */
void NlmFtmRefApp_TblIdStringConvert(
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

/* CreateFibTables function registers different LPM tables with the FibTblMgr;
   The Number of tables created depends on the value of NLM_FIB_REFAPP_NUM_TABLES
   and attributes of the tables are provided by tableInfo
*/
NlmErrNum_t NlmFtmRefApp_CreateFibTables(
    NlmFtmRefApp_RefAppInfo *refAppData_p
    )
{
    nlm_u8 tblNum;
    nlm_8 tblIdStr[NLM_FIB_REFAPP_TBL_ID_LEN + 1];
    NlmFibTblIndexRange indexRange;
    NlmReasonCode reasonCode;

    refAppData_p->m_numOfTables = NLM_FIB_REFAPP_NUM_TABLES;
    refAppData_p->m_fibTbl_pp = 
		NlmCmAllocator__calloc(refAppData_p->m_alloc_p, refAppData_p->m_numOfTables, sizeof(NlmFibTbl*));

    for(tblNum = 0; tblNum < refAppData_p->m_numOfTables; tblNum++)
    {
        /* Conevrting tbl id to string of 0's and 1's*/
        NlmFtmRefApp_TblIdStringConvert(
			tableInfo[tblNum].m_tblId, tblIdStr, NLM_FIB_REFAPP_TBL_ID_LEN);

        indexRange.m_indexLowValue = tableInfo[tblNum].m_indexLow;
        indexRange.m_indexHighValue = tableInfo[tblNum].m_indexHigh;

        if((refAppData_p->m_fibTbl_pp[tblNum] = NlmFibTblMgr__CreateTable(
			refAppData_p->m_fibTblMgr_p, tblIdStr, &indexRange, tableInfo[tblNum].m_tblWidth, &reasonCode)) == NULL)
        {
            NlmCm__printf("\n\n\tError: NlmFibTblMgr__CreateTable (TblId: %d) -- ReasonCode: %d\n",
                                                        tableInfo[tblNum].m_tblId, reasonCode);
            return NLMERR_FAIL;
        }
        NlmCm__printf ("\n\t\t Table with Tbl Id %d Created\n", tableInfo[tblNum].m_tblId);
    }

    return NLMERR_OK;
}

/* ConfigSearches function configures different types of FIB search operations
   being performed. The Number of saerch configuartions depends on the value of
   NLM_FIB_REFAPP_NUM_SRCH_CONFIG and attributes of the saerches are provided by 
   srchInfo
*/
NlmErrNum_t NlmFtmRefApp_ConfigSearches(
    NlmFtmRefApp_RefAppInfo *refAppData_p
    )
{
    nlm_u8 srchNum;
    NlmFibParallelSrchAttributes psAttrs;
    nlm_8 tblIdStr[NLM_FIB_MAX_PARALLEL_SRCHES][NLM_FIB_REFAPP_TBL_ID_LEN + 1];
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;

    for(srchNum = 0; srchNum < NLM_FIB_REFAPP_NUM_SRCH_CONFIG;srchNum++)
    {
        psAttrs.m_numOfFibParallelSrch = srchInfo[srchNum].m_numOfParallelSrches;

        NlmFtmRefApp_TblIdStringConvert(srchInfo[srchNum].m_tblIdForParallelSrch0,
                                        tblIdStr[0],
                                        NLM_FIB_REFAPP_TBL_ID_LEN);

        psAttrs.m_parallelSrchInfo[0].m_tblId = tblIdStr[0];
        psAttrs.m_parallelSrchInfo[0].m_rsltPortNum =
            srchInfo[srchNum].m_rsltPortForParallelSrch0;
        psAttrs.m_parallelSrchInfo[0].m_startBitInKey =
            srchInfo[srchNum].m_startBitForParallelSrch0;

        if(psAttrs.m_numOfFibParallelSrch == 2)
        {
            /* If there are parallel(2) searches for configuration then initialize the
            attributes of second search */
            NlmFtmRefApp_TblIdStringConvert(srchInfo[srchNum].m_tblIdForParallelSrch1,
                                            tblIdStr[1],
                                            NLM_FIB_REFAPP_TBL_ID_LEN);

            psAttrs.m_parallelSrchInfo[1].m_tblId = tblIdStr[1];
            psAttrs.m_parallelSrchInfo[1].m_rsltPortNum =
                srchInfo[srchNum].m_rsltPortForParallelSrch1;
            psAttrs.m_parallelSrchInfo[1].m_startBitInKey =
                srchInfo[srchNum].m_startBitForParallelSrch1;
        }

        if((errNum = NlmFibTblMgr__ConfigSearch(refAppData_p->m_fibTblMgr_p,
                                                srchInfo[srchNum].m_ltrNum,
                                                &psAttrs,
                                                &reasonCode)) != NLMERR_OK)
        {
            NlmCm__printf("\n\n\tError: NlmFibTblMgr__ConfigSearch (LtrNum: %d) -- ReasonCode: %d\n",
                                                        srchInfo[srchNum].m_ltrNum, reasonCode);
            return errNum;
        }
        NlmCm__printf ("\n\t\t Ltr Num %d Configured\n", srchInfo[srchNum].m_ltrNum);
    }
       
	/* Lock the FIB configuartions */
    if((errNum = NlmFibTblMgr__LockConfiguration(refAppData_p->m_fibTblMgr_p, &reasonCode)) != NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError: NlmFibTblMgr__LockConfiguration -- ReasonCode: %d\n", reasonCode);
        return errNum;
    }

    return NLMERR_OK;
}


/* _InsertPfxsToTbls function called by each thread and each thread adds the pefixes 
   to table. Number of prefixes which are added to each table is specified by the table attributes
   tableInfo.

   e.g: Total threads 4, total prefix 2000, each thread will insert max 500 prefixes
   Thread 0 -> inertes prefix buffer index 0, 4, 8  .... <till table full or 500 prefixes >
   Thread 0 -> inertes prefix buffer index 1, 5, 9  .... <till table full or 500 prefixes >
   Thread 0 -> inertes prefix buffer index 2, 6, 10 .... <till table full or 500 prefixes >
   Thread 0 -> inertes prefix buffer index 3, 7, 11 .... <till table full or 500 prefixes >

   Each thread will have its own time to calculate the time taken to insert the prefixes,
   at the end we will calculate the longest time taken by the thread. which is the time 
   taken for the max prefixes inserted.
*/
static NlmErrNum_t 
NlmFtmRefApp_InsertPfxsToTbls(void *threadNum)
{
    NlmErrNum_t errNum = 0;
	nlm_u32 iter=0, cpuid = 0;
    NlmCmPrefix *prefix_p =NULL;
    NlmReasonCode reasonCode = 0;
	NlmFtmRefApp_RefAppInfo *refAppData_p = NULL;
	nlm_u8 tblNum = 0, done = NlmFalse;
	nlm_u32 sizeOfPfxStructInBytes = 0, totalPfxSize = 0, pfxArrayOffset = 0;
    
	if(g_refAppData_ptr == NULL)
	{
		NlmCm__printf("\n\t Invalid refApp pointer\n\n");
		exit(1);
	}
	refAppData_p = g_refAppData_ptr; /* assign the global pointer of the refapp */
#ifdef NLM_NETOS
	nlm_u32 threadID = 0;
	cpuid = getpid();
#endif

	errNum = NlmCmMt__RegisterThread( *(nlm_u32*)threadNum, cpuid, &reasonCode);
	if(errNum != NLMERR_OK)
		return errNum;
	
#ifdef NLM_NETOS
	threadID = NlmCmMt__GetThreadID(cpuid);
#endif		

	cpuid = (nlm_u32)refAppData_p->m_coreNum[threadID];
	
	/* bind thread to the CPU */
	{
		if( NlmFtmRefApp_pvt_BindThreadToCore(cpuid) != 0)
		{
			NlmCm__printf("\n\t ***  Could not bind Thread-%d to CPU-%d. ***\n", threadID, cpuid);

			return 1;
		}

		getGo[threadID] = 1;

		NlmCm__printf("\n\t Thread-%d bound to CPU-%d\n", threadID, cpuid);

		/* Wait till all threads are bound to the core. */
		while(getGo[refAppData_p->m_numInsThrds] == 0) ;

		NlmCm__printf("\t Thread-%d started executing.\n\n", threadID);
	}
	
	sizeOfPfxStructInBytes	= NlmCmPrefix__GetStorageSize((((nlm_u16)tableInfo[tblNum].m_tblWidth) + 7) & (~0x7) );
	totalPfxSize			= (tableInfo[tblNum].m_numOfPfxsToBeAdded * sizeOfPfxStructInBytes);
	pfxArrayOffset			= (threadID * refAppData_p->m_fibOneOffset); 

	for(tblNum = 0; tblNum < refAppData_p->m_numOfTables; tblNum++)
    {
        iter = 0;  /* Initilialize the iter value to 0 for each table */
		done = NlmFalse;
		struct timeval sTime = {0};
		struct timeval eTime = {0};		

		/* this will have the count of prefixes inserted by indivisula thread */
		threadTime[threadID].th_inCount = 0;
		threadTime[threadID].th_insTime = 0;

		NlmFtmRefApp_GetTime(sTime);

		while((refAppData_p->m_fibTbl_pp[tblNum]->m_numPrefixes < tableInfo[tblNum].m_numOfPfxsToBeAdded)
			&& !done && (pfxArrayOffset < totalPfxSize))
		{
			/* pick the prefixes from the pre-generated buffer and add to the table */
			prefix_p = (NlmCmPrefix*)&refAppData_p->g_pfxArray[pfxArrayOffset];

			/* add the prefix using the FIB TblMgr API */
			if((NlmFibTblMgr__AddPrefix(refAppData_p->m_fibTbl_pp[tblNum],prefix_p, &reasonCode)) != NLMERR_OK)
			{
				if(reasonCode == NLMRSC_RESOURCE_ALLOC_FAILED)
				{
					NlmCm__printf("\n\n\t -- Table Full occured (Resource Alloc Failed) ");
					done = NlmTrue;
				}
				else if(reasonCode == NLMRSC_IDX_RANGE_FULL)
				{
					NlmCm__printf("\n\n\t -- Table Full occured (Index range Full)n");
					done = NlmTrue;
				}
				else if(reasonCode ==  NLMRSC_DUPLICATE_PREFIX)
				{
					NlmCm__printf("\t Duplicate prefix encounterd @ %d, Th-ID: %d ...\n", iter, threadID);
					if(!refAppData_p->m_isFileEnable)	
					continue;
				}    
				else
				{
					NlmCm__printf("\t Internal Error encounterd @ %d, Th-ID: %d ...\n", iter, threadID);
					return NLMERR_FAIL;
				}				
			}
			iter++;
			pfxArrayOffset += (refAppData_p->m_numInsThrds * sizeOfPfxStructInBytes);
			threadTime[threadID].th_inCount++;

			/*  Locking for the shared data */
			NlmCmMt__SpinLock(&refAppData_p->m_refApp_spinLock);
			tableInfo[tblNum].m_uniqueCnt++;
			NlmCmMt__SpinUnlock(&refAppData_p->m_refApp_spinLock);

			if((tableInfo[tblNum].m_uniqueCnt % 10000) == 0)
			{
				NlmCm__printf("\n\n\t %u Unique Prefixes Added to the Table ", tableInfo[tblNum].m_uniqueCnt);

				{
					nlm_u32 th;
					for (th = 0; th < refAppData_p->m_numInsThrds; th++)
						NlmCm__printf("\n\t  Thread-%d added unique prefix: %u", th, threadTime[th].th_inCount);
				}
			}
		}
	
		NlmFtmRefApp_GetTime(eTime);
		threadTime[threadID].th_insTime = NlmFtmRefApp_CalcTimeDiff(eTime, sTime);
        
		NlmCm__printf("\n\t done with Thread : %d, inserted %d prefixes to table %d", 
			threadID, threadTime[threadID].th_inCount, tableInfo[tblNum].m_tblId);
	}
#ifdef NLM_XLP	
	NlmXlpXpt__FlushResponses(refAppData_p->m_xpt_p);
#endif	
	checkJoin[threadID] = 1;

    return NLMERR_OK;
}


/* DeletePrefixesFromTables function deletes prefixes from table */
NlmErrNum_t NlmFtmRefApp_DeletePrefixesFromTables(
    NlmFtmRefApp_RefAppInfo *refAppData_p
    )
{
    nlm_u8 tblNum;
    nlm_u32 iter = 0;
    NlmCmPrefix *prefix_p;
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;

    for(tblNum = 0; tblNum < refAppData_p->m_numOfTables; tblNum++)
    {
		{
			nlm_u8 tID = 0;
			nlm_u32 loop = 0;

			for (tID = 0; tID < refAppData_p->m_numInsThrds; tID++)
			{
				nlm_u32 sizeOfPfxStructInBytes	= 
					NlmCmPrefix__GetStorageSize((((nlm_u16)tableInfo[tblNum].m_tblWidth) + 7) & (~0x7) );
				nlm_u32 pfxArrayOffset			= (tID * sizeOfPfxStructInBytes); 
			
				iter = 0;
				for(loop = 0; loop < threadTime[tID].th_inCount; loop++)
				{
					prefix_p = (NlmCmPrefix*)&refAppData_p->g_pfxArray[pfxArrayOffset];

					/* delete the prefix using the Fib TblMgr api */
					if((errNum = NlmFibTblMgr__DeletePrefix(
						refAppData_p->m_fibTbl_pp[tblNum], prefix_p, &reasonCode)) != NLMERR_OK)
					{
						if(reasonCode !=  NLMRSC_PREFIX_NOT_FOUND) 
						{
							NlmCm__printf("\n\n\tError:%d, Rsn: %d NlmFibTblMgr__DeletePrefix\n", errNum, reasonCode);
							return errNum;
						}
					}

					iter++;
					pfxArrayOffset += (refAppData_p->m_numInsThrds * sizeOfPfxStructInBytes);

					if((iter % 10000) == 0)
						NlmCm__printf("\n\t - %d Prefixes deleted from Tbl (inserted by thread %d)", iter, tID); 
				}   /* for */	

				NlmCm__printf("\n\t - Total %d Prefixes deleted from Tbl (inserted by thread %d)\n", iter, tID); 
			}/* tID */
		} /* open */
    } /* for */
    return NLMERR_OK;
}

#define NLM_FIB_REFAPP_MAX_SRCH_LEN_IN_BYTES (NLMDEV_FIB_MAX_PREFIX_LENGTH/8)

/* DeviceCompare function performs device compare using Device Manager APIs. 
   This function performs search opeartions with the configured LTR(s) for the 
   specified table. 
   
   The results of compare operation are compared with the LPM index obtained 
   using LocateLPM FibTblMgr software search API.
   
   Note: This function can only perform single search per LTR;
  
 */
NlmErrNum_t NlmFtmRefApp_DeviceCompare(
    NlmDevMgr *devMgr_p,
    nlm_u8 tblId,
    NlmCmPrefix *srchPrefix_p,
    nlm_u32 lpmPrefixIndex
    )
{
    NlmDevCtxBufferInfo cbWriteInfo;
    NlmReasonCode reasonCode;
    NlmErrNum_t errNum;

    nlm_u8 srchNum;
    NlmDevCmpRslt srchRslt;
    nlm_u8 srchKey[NLM_FIB_REFAPP_MAX_SRCH_LEN_IN_BYTES];
    nlm_u16 startBit;
    nlm_u8 rsltPortNum;

    for(srchNum = 0; srchNum < NLM_FIB_REFAPP_NUM_SRCH_CONFIG; srchNum++)
    {
        if(srchInfo[srchNum].m_tblIdForParallelSrch0 == tblId) /* If the search is configured
                                                               for the specified table */
        {
            /* Construct the search key based on start bit value */
            NlmCm__memset(srchKey, 0, (NLMDEV_FIB_MAX_PREFIX_LENGTH/8));
            startBit = srchInfo[srchNum].m_startBitForParallelSrch0;
            NlmCm__memcpy(&srchKey[NLM_FIB_REFAPP_MAX_SRCH_LEN_IN_BYTES - ((startBit + 1) >> 3)],
                          srchPrefix_p->m_data,
                          (srchPrefix_p->m_avail >> 3));

            /* 0 to 255 are used as context id range for XlpXpt, so compares should not 
			use these range */
			cbWriteInfo.m_cbStartAddr = 256; 
            cbWriteInfo.m_datalen = NLMDEV_FIB_MAX_PREFIX_LENGTH >> 3;
            NlmCm__memcpy(&cbWriteInfo.m_data, srchKey, NLMDEV_FIB_MAX_PREFIX_LENGTH >> 3);

            if((errNum = NlmDevMgr__Compare1(devMgr_p,
                                               srchInfo[srchNum].m_ltrNum,
                                               &cbWriteInfo,
                                               &srchRslt,
                                               &reasonCode)) != NLMERR_OK)
            {
                NlmCm__printf("\n\n\tError: NlmDevMgr__Compare1 -- ReasonCode: %d \n", reasonCode);
                return errNum;
            }

            /* Get the result port number which was configured for the result */
            rsltPortNum = srchInfo[srchNum].m_rsltPortForParallelSrch0;

            if(srchRslt.m_hitOrMiss[rsltPortNum] == NLMDEV_MISS)
            {
                if(lpmPrefixIndex != NLM_FIB_INVALID_INDEX)
                {
                    /* If the compare result is miss and expected result is not miss then report an error */
                    NlmCm__printf("\n\n\tError: Device Compare Results In Miss (Expected Hit At Index 0x%x)",
                                    lpmPrefixIndex);
                    NlmCm__printf("\n\tCompare Done for tbl with tblId %d and LtrNum %d", tblId,
                                                    srchInfo[srchNum].m_ltrNum);
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
                                            srchInfo[srchNum].m_ltrNum);
                    NlmCm__printf("\n\tSrch Prefix:");
                    NlmCmPrefix__PrintAsIp(srchPrefix_p, NlmCm__stderr);
                    return NLMERR_FAIL;
                }
            }
        }
    }
    return NLMERR_OK;
}

/* PerformLPMSearches function performs Longest Prefix Match searches for all 
   the prefixes added to the tables. This function performs searches using the 
   Device Manager Compare API and then verifies the results with the results 
   obtained using LocateLPM FibTblMgr API
 */
NlmErrNum_t NlmFtmRefApp_PerformLPMSearches(
    NlmFtmRefApp_RefAppInfo *refAppData_p
    )
{
    nlm_u8 tblNum;
    nlm_u32 iter = 0;
    NlmErrNum_t errNum;
    NlmReasonCode reasonCode;
    nlm_u32 lpmIndex;
    NlmCmPrefix *srchPrefix_p;

    for(tblNum = 0; tblNum < refAppData_p->m_numOfTables; tblNum++)
    {
		{
			nlm_u8 tID = 0;
			nlm_u32 loop = 0;

			for (tID = 0; tID < refAppData_p->m_numInsThrds; tID++)
			{
				nlm_u32 sizeOfPfxStructInBytes	= 
					NlmCmPrefix__GetStorageSize((((nlm_u16)tableInfo[tblNum].m_tblWidth) + 7) & (~0x7) );
				nlm_u32 pfxArrayOffset			= (tID * refAppData_p->m_fibOneOffset); 
			
				iter = 0;
				for(loop = 0; loop < threadTime[tID].th_inCount; loop++)
				{
					srchPrefix_p = (NlmCmPrefix*)&refAppData_p->g_pfxArray[pfxArrayOffset];

					/* Perform software search for the prefix to obtain its lpmIndex
					using the FibTblMgr API */
					if((errNum = NlmFibTblMgr__LocateLPM(refAppData_p->m_fibTbl_pp[tblNum],
						srchPrefix_p, &lpmIndex, NULL, &reasonCode)) != NLMERR_OK)
					{
						NlmCm__printf("\n\n\tError: NlmFibTblMgr__LocateLPM -- ReasonCode: %d\n", reasonCode);
						return errNum;
					}

					/* perform device compare operation to obtain the LPM for the search prefix
					and then verify the result with the LPM Index returned by FibTblMgr API */
					if((errNum = NlmFtmRefApp_DeviceCompare(refAppData_p->m_devMgr_p,
						tableInfo[tblNum].m_tblId,	srchPrefix_p, lpmIndex)) != NLMERR_OK)
					{
						NlmCm__printf("\n\n\tError:%d NlmDevMgr__Compare1 Miss-Match \n", errNum);
						return errNum;
					}

					iter++;
					pfxArrayOffset += (refAppData_p->m_numInsThrds * sizeOfPfxStructInBytes);

					if((iter  % 10000) == 0)
						NlmCm__printf("\n\t - %d Prefixes Searched In Tbl (inserted by thread %d)", iter, tID);
				}   /* for */		
				NlmCm__printf("\n\t - Total %d Prefixes Searched In Tbl (inserted by thread %d)\n", iter, tID); 
			
			}/* tID */
		} /* open */
	} /* for */
    return NLMERR_OK;
}


/* This function calls the sub functions which do the FTM operations */
nlm_u32 
main_internal(NlmFtmRefApp_RefAppInfo	*refApp_p,
			  int						argc,
			  char						**argv
			  )
{
	nlm_u32 errNum = 0;
	NlmReasonCode reasonCode = 0;
	
	/* Parse the commandline arguments */
	if((errNum = NlmFtmRefApp_ParseArgs(refApp_p, argc, argv)) != NLMERR_OK)
	{
		NlmCm__printf("\n\n\t Invalid arguments to Parse ");
        return NLMERR_FAIL;
	}
	/* Register the main thread here. */
	if((errNum = NlmCmMt__RegisterThread(refApp_p->m_numInsThrds, 0, &reasonCode)) != NLMERR_OK)
	{
		NlmCm__printf("\n\t ***  Could not Register Main Thread to CPU-0. ***\n");
		return NLMERR_FAIL;
	}

	/* Initialize the Forwarding Inforamtion Base Subsystem */
    NlmCm__printf ("\n\t Initializing the FIB SubSystem:\n");
    if((errNum = NlmFtmRefApp_InitForwardingSubSystem(refApp_p)) != NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError Cannot Initialize FIB SubSystem ");
        NlmFtmRefApp_Usage();
		return NLMERR_FAIL;
    }
    NlmCm__printf ("\n\n\t Creating FIB Tables:\n");
    /* Create the LPM Tables */
    if((errNum = NlmFtmRefApp_CreateFibTables(refApp_p)) != NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError Cannot Create FIB Table(s)");
        return NLMERR_FAIL;
    }

    NlmCm__printf ("\n\n\t Configuring Searches:\n");
    /* Configure the searches */
    if((errNum = NlmFtmRefApp_ConfigSearches(refApp_p)) != NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError Cannot Config Searches");
        return NLMERR_FAIL;
    }
		refApp_p->m_isFileEnable = 0;
       NlmCm__printf ("\n\n\t Getting unique prefixes through method:%d\n",refApp_p->m_isFileEnable);
	       NlmCm__printf ("\t 0. pre-generating \n\t 1. from file\n\n");
	/* Pre generate unique prefixes into prefix buffer */
    if((errNum = NlmFtmRefApp_GeneratePrefixes(refApp_p)) != NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError Generating prefixes (prefix buffer) ");
        return NLMERR_FAIL;
    }
		
    NlmCm__printf ("\n\n\t Adding Prefixes to Table(s):\n");
	/* Insert prefixes into to Tables */
    if((errNum = NlmFtmRefApp_CreateFtmThreads(refApp_p)) != NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError Cannot insert prefixes into Tables");
        return NLMERR_FAIL;
    }

	/* Perform LPM Search Operations if xpt type is not blackhole xpt*/
    if(refApp_p->m_xptType != NLM_FIB_REFAPP_BLACKHOLE_XPT)
    {
        NlmCm__printf ("\n\n\t Performing LPM-Device Searches:\n");
        if((errNum = NlmFtmRefApp_PerformLPMSearches(refApp_p)) != NLMERR_OK)
        {
            NlmCm__printf("\n\n\tError Performing LPM-Device Searches");
            return NLMERR_FAIL;
        }
    }

	NlmCm__printf ("\n\n\t Deleting Prefixes from table(s):\n");
    /* Delete some prefixes */
    if((errNum = NlmFtmRefApp_DeletePrefixesFromTables(refApp_p)) != NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError Deleting Prefixes from table(s)");
        return NLMERR_FAIL;
    }
									
	NlmCm__printf ("\n\n\t Delete pre generate unique prefixes:\n");
    /* delete pre generated unique prefixes from prefix buffer */
    if((errNum = NlmFtmRefApp_DestroyPrefixes(refApp_p)) != NLMERR_OK)
    {
        NlmCm__printf("\n\n\tError Cannot Config Searches");
        return NLMERR_FAIL;
    }

    /* Destroy the Forwarding Subsystem */
    NlmFtmRefApp_DestroyForwardingSubSystem(refApp_p);


	return NLMERR_OK;
}

/* main function */
int main(int argc, char *argv[])
{
    NlmFtmRefApp_RefAppInfo refAppData;
    NlmCmAllocator allocBody;
	nlm_u32 errNum = 0;

    NlmCm__printf("\n\n\t**************************************************************************");
    NlmCm__printf ("\n\t  FIB Table Manager Multi-Threaded (MT) Reference Application \n");
    NlmCm__printf("\n\t**************************************************************************\n\n");

    /* Create general purpose allocator */
    refAppData.m_alloc_p = NlmCmAllocator__ctor(&allocBody);
    NlmCmAssert((refAppData.m_alloc_p != NULL), "The memory allocator is NULL!\n");

	g_refAppData_ptr = &refAppData;

    if((errNum = main_internal(&refAppData, argc, argv)) != 0)
	{
		/* Destroy the allocator */
		NlmCmAllocator__dtor(refAppData.m_alloc_p);

		NlmCm__printf("\n\n\t*****************************************************************");
		NlmCm__printf("\n\t  FIB Table Manager Multi-Threaded (MT) Reference Application Failed ");
		NlmCm__printf("\n\t*****************************************************************\n\n");
		return 0;
	}

    /* Destroy the allocator */
    NlmCmAllocator__dtor(refAppData.m_alloc_p);

    NlmCm__printf("\n\n\t********************************************************************************");
    NlmCm__printf("\n\t  FIB Table Manager Multi-Threaded (MT) Reference Application Successfully Completed ");
    NlmCm__printf("\n\t********************************************************************************\n\n");
	
	return 0;
}

