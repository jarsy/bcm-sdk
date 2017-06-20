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


/*
	g_pfxSeqNr = 0 represents an invalid sequence number; 
	Valid sequence numbers start from 1
*/
static nlm_u32 g_pfxSeqNr = 0;

/* This function gives the next prefix sequence number */
nlm_u32 NlmFtmRefApp_CmSeqGen_GenerateNextPfxSeqNr(void)
{
	++g_pfxSeqNr;
	return g_pfxSeqNr;
}

/* This function gives the previous prefix sequence number */
nlm_u32 NlmFtmRefApp_CmSeqGen_GeneratePrevPfxSeqNr(void)
{
	--g_pfxSeqNr;
	return g_pfxSeqNr;
}

/* This function gives the current prefix sequence number */
nlm_u32 NlmFtmRefApp_CmSeqGen_GetCurrentPfxSeqNr(void)
{
	return g_pfxSeqNr ;
}

/* This function creates the hash table, which is used to hold the unique prefix bundles */
void NlmFtmRefApp_hashTableCreate(
		NlmFtmRefApp_RefAppInfo		*refAppData_p,
		NlmFibTblIndexRange			*indexRange
		)
{
	nlm_u32 init_hashtbl_size = 0;

	/* Initialize and create hash table for the given size */
	init_hashtbl_size = indexRange->m_indexHighValue - indexRange->m_indexLowValue + 1;

	refAppData_p->m_hashNode_p =
		NlmCmOpenHashPfxBundle__create(refAppData_p->m_alloc_p, init_hashtbl_size, 70, 100);

	if(refAppData_p->m_hashNode_p == NULL)
	{
		NlmCm__printf("\n\n\t Memory Allocation Fail for Hash table !!!!!\n\n");
		exit(1);
	}
	return;
}

/* This function destroys the hash table, and releases the memory allocated for the unique prefix bundles */
void NlmFtmRefApp_hashTableDestroy(
		NlmFtmRefApp_RefAppInfo		*refAppData_p
		)
{
	if(refAppData_p->m_hashNode_p)
	{
		nlm_u32 rmCnt;

		for(rmCnt = 0; rmCnt < refAppData_p->m_hashNode_p->m_nMaxSize; rmCnt++)
		{
			if(refAppData_p->m_hashNode_p->m_slots_p[rmCnt] != NULL)
				NlmCmAllocator__free(refAppData_p->m_alloc_p, refAppData_p->m_hashNode_p->m_slots_p[rmCnt]);
		}
	}
	/* destroy hash table allocator*/
	NlmCmOpenHashPfxBundle__destroy(refAppData_p->m_hashNode_p);
}

/* This function generate the prefix and checks in the hash table whether generated prefixes 
   already exists in the hash table or not
   1. If generated prefix exists (duplicate) return exists, and try to generate new one
   2. If generated prefix does not exists, then add that prefix bundle to hash table
*/
nlm_u32 NlmFtmRefApp_hashCheckDupAndInsert(
		NlmFtmRefApp_RefAppInfo		*refAppData_p,
		NlmCmPrefix					*pfx_p
		)
{
	/* prefix bundle; prefix data and prefix length are assigned here */
	NlmCmPfxBundle *bundle = NlmCmPfxBundle__create(refAppData_p->m_alloc_p, pfx_p, 0, 0, 0);

	{
		NlmCmPfxBundle     **pfxSlotInHash = 0 ;

        /* update the ralavant information in prefix bundle */
		bundle->m_nIndex     = refAppData_p->m_uniquePfxCount;
		bundle->m_nSeqNum    = NlmFtmRefApp_CmSeqGen_GenerateNextPfxSeqNr();

		/* Look inside the hash table for duplicates */
		if (NlmCmOpenHashPfxBundle__Locate2WithBytes(refAppData_p->m_hashNode_p,
			pfx_p->m_data, pfx_p->m_inuse, &pfxSlotInHash))
		{
			refAppData_p->m_duplicatePfxCount++;
			/* If found destroy the old prefix and decrement the sequence number
			and continue to generate the next prefix */
			NlmFtmRefApp_CmSeqGen_GeneratePrevPfxSeqNr();
			NlmCmPfxBundle__destroy(bundle, refAppData_p->m_alloc_p);
			return 1; /* duplicate, re-generate new prefix */
		}

		NlmCmOpenHashPfxBundle__InsertUnique(refAppData_p->m_hashNode_p, &bundle);
		(refAppData_p->m_uniquePfxCount)++;
	}
	return 0; /* unique */
}

/*
	This function will generated the prefixes, prefix pattern is
	4b table Id + 14b VPN Id + 128b random IPV6 prefix = 146b length prefix
*/
nlm_u32 NlmFtmRefApp_FormPrefix(nlm_u8* pfxData,
								nlm_u32 *pfxLen,
								nlm_32 tblId
								)
{
    nlm_u32 randNum = 0;
    const nlm_u32 vpnIdLen = 14;
	nlm_u32 vpnId = 0;
    nlm_u32 endPos = 0, i = 0;

    if(*pfxLen < NLM_FIB_REFAPP_TBL_ID_LEN)    /* Pfxlen cannot be less than length of tblId  */
    {
        NlmCm__printf("\n\n\tError: Pfx Len Should Not Be Less Than TblIdLen");
        return NLMERR_FAIL;
    }

    if(*pfxLen > NLMDEV_FIB_MAX_PREFIX_LENGTH)/* Pfxlen cannot be greater than 320b  */
    {
        NlmCm__printf("\n\n\tError: Pfx Len Should Not Be Greater Than 320b");
        return NLMERR_FAIL;
    }

	/* Generate the prefix data based on the iter value and pfxlen */

	/* Table Id - 4bit, VPN 14b, IPv6 128b, Random*/
	*pfxLen = 18;
	endPos = (NLMDEV_FIB_MAX_PREFIX_LENGTH - NLM_FIB_REFAPP_TBL_ID_LEN - 1);

    /* 14b VPN Id */
	vpnId = 0;

    WriteBitsInArray(pfxData,
			NLMDEV_FIB_MAX_PREFIX_LENGTH/8,	endPos, (endPos- vpnIdLen + 1), vpnId);

	endPos -= vpnIdLen;

	/* Write 128b IP address random value */
	randNum = 128;

	*pfxLen += randNum;

	for(i = 0; i < (randNum/32) ; i++)
    {
        WriteBitsInArray(pfxData,
        	NLMDEV_FIB_MAX_PREFIX_LENGTH/8, endPos, endPos - 31, NLM_FIB_REFAPP_RANDOM_WORD);
        endPos -= 32;
    }

	/* Append the table id into the prefix */
    WriteBitsInArray(pfxData, 40, 319, (320 - NLM_FIB_REFAPP_TBL_ID_LEN), tblId);

    return NLMERR_OK;
}

/* This function will create the prefix buffer, which is place holder of unique prefixes
   generated. For adding prefixes to table we will make use of this pre-generated prefix
   buffer to get the prefixes
  */
void NlmFtmRefApp__PreCreatePfxsInBuffer(
		NlmFtmRefApp_RefAppInfo *refAppData_p,
		nlm_u32					*numPfxFormedSoFar_p
		)
{
	NlmCmPrefix *pfx_p = NULL;
	nlm_u32 pfxIter = 0, pfxArrayOffset = 0;
	nlm_u8	pfxData[NLMDEV_FIB_MAX_PREFIX_LENGTH/8] = {0};
	nlm_u32 tableLen = tableInfo[0].m_tblWidth;
	nlm_32 tableID = tableInfo[0].m_tblId; /* 0 */

	/* round pfxLenInBits to nearest multiple of 8 */
	nlm_u16 pfxLenInBits = (nlm_u16)((tableLen + 7) & (~0x7));
	nlm_u32 sizeOfPfxStructInBytes = NlmCmPrefix__GetStorageSize(pfxLenInBits);
	nlm_u32 tries = 0;

	NlmCm__memset(refAppData_p->g_pfxArray, 0, tableInfo[0].m_numOfPfxsToBeAdded * sizeOfPfxStructInBytes);
	NlmCm__atoi((const nlm_8*)&tableInfo[0].m_tblId, &tableID);
	refAppData_p->m_fibOneOffset = sizeOfPfxStructInBytes;

	pfxArrayOffset = 0;

	NlmCm__printf("\n\t Pre-Generation of unique prefixes, adding to buffer \n\n\t\t");

	for(pfxIter = 0; pfxIter < tableInfo[0].m_numOfPfxsToBeAdded; ++pfxIter)
	{
		pfx_p = (NlmCmPrefix*) &refAppData_p->g_pfxArray[pfxArrayOffset];

		NlmCmPrefix__pvt_ctor(pfx_p, pfxLenInBits, 0, NULL);

		tries = 0;
		do
		{
			NlmFtmRefApp_FormPrefix(pfxData, &tableLen, tableID);
			if( ++tries > 500000)
			{
				tableInfo[0].m_numOfPfxsToBeAdded = *numPfxFormedSoFar_p - 1;
				return;
			}

			/* Generate a plen bit prefix */
			NlmCmPrefix__Set(pfx_p, tableLen, pfxData);

		}while(NlmFtmRefApp_hashCheckDupAndInsert(refAppData_p, pfx_p));

		pfxArrayOffset += sizeOfPfxStructInBytes;

		(*numPfxFormedSoFar_p)++;

		if( ((*numPfxFormedSoFar_p)%10000) == 0)
			NlmCm__printf(".");
	}
	NlmCm__printf("... Done \n\n");
}


/* Returns the length in bits of the given IPv4 or IPv6 prefix.*/
nlm_u32
GetPfxLengthIP(const char* prefix)
{
    /*  prefix already checked by public interface */
    nlm_32 len = 0 ;
    const char* cur = prefix ;

    while( cur[0] != '\0' && cur[0] != '/' ) { cur++ ; }

    if( cur[0] == '\0' || NlmCm__isdigit((nlm_u8)cur[1]) == 0 )
    {
		/* Malformed prefix */
		return 0 ;
    }

    cur++ ;
    if( NlmCm__isdigit((nlm_u8)cur[0]) == 0 )
    {
		/* Malformed prefix */
		return 0 ;
    }

    NlmCm__atoi(cur, &len) ;   

    return len ;
}


/*To get the number of dots in available data as 3 in 10.0.0.0*/
nlm_u32 GetDots(nlm_8 *argv)
{
    nlm_u32 count = 0;
	nlm_u32 strIndex = 0;

	while(argv[strIndex] != '\0')
	{
        if(argv[strIndex] == '.')
		{	
			count++;
		}		
		strIndex++;
	}
    return count ;
}

/* Coverts "dotted_decimal/length" format to char array */
nlm_u32 IpFormatToArray(
    nlm_8 *argv,
    nlm_u8 *tstartdata
    )
{
    nlm_u32 len = 0;
    nlm_u32 arrIndex = 0;
	nlm_u32 strIndex = 0;

    nlm_32 flag = 1;
	nlm_32 atoival = 0;

	while(argv[strIndex] != '\0')
	{
        if(argv[strIndex] == '/')
        {
            strIndex++;
            if(!NlmCm__isdigit((nlm_32)(argv[strIndex])))
            {
                NlmCm__printf("\n ERROR(Prefix Parsing): Pfx length not specified\n");
                return 0; /* no plen */
            }
            flag = 0;
			NlmCm__atoi(&argv[strIndex], &atoival);
			len = atoival ;
        }
        else if(argv[strIndex] == '.' || !flag)
		{
			flag = (argv[strIndex] == '.') ? 1 : 0;
			strIndex++;
			continue;
		}
		else
		{
			flag = 0;
			NlmCm__atoi(&argv[strIndex], &atoival);
			tstartdata[arrIndex] = (nlm_u8)atoival ;
			arrIndex++;
		}
		strIndex++;
	}
    return len ;
}


/*Reading prefixes from file one by one and perpanding table id to that*/
nlm_u32 ReadPfxFromFile(
    NlmCmPrefix *prefix_p,
    nlm_u32 tblId,
    FILE *pfxFile_p
    )
{
	nlm_8 pfxString[160] = {0,}, *pfxChar_p = NULL;
    nlm_u8  pfxData[NLM_FIB_REFAPP_MAX_PFX_LEN/8] = {0};
    nlm_u32 i =0, pfxLen = 0;

    pfxChar_p = pfxString;

	fgets(pfxString, 80, pfxFile_p);

	if(feof(pfxFile_p))
		return 1;/*NLMFIBTEST_ERR_PFXFILE_EOF;*/

	i = (nlm_u32) NlmCm__strlen(pfxString);

	{
		nlm_u32 lineLen = i; /* Length of the line */

		/* Process single file line, and extract the parameter details */
		pfxString[lineLen+1] = '\n';
		if (lineLen)
		{
			nlm_u8 pfxRead[15];
			nlm_u32 PfxVal = 0;			
			nlm_u32 numOfDots = 0;
			nlm_u32 end   = NLM_FIB_REFAPP_MAX_PFX_LEN - NLM_FIB_REFAPP_TBL_ID_LEN - 1;
			nlm_u32 start = end - 7;

			NlmCm__memset(&pfxRead, 0, 15 * sizeof(nlm_u8));

			pfxLen = GetPfxLengthIP(pfxString);

				pfxLen += 4; /* Table ID of 4b */
			
			/* function which gets number of dots in the IP address from string format*/
			numOfDots = GetDots(pfxString);

			/* Destination prefix, convert form IP format to Array */
			IpFormatToArray(pfxString, &pfxRead[0]);

			/* function which calulate the actual 32b pfxLen depend on MASK value */
			for(i = 0;i <= numOfDots;i++)
			{
				/* get the 32b IPv4 value from the Destination address */
				PfxVal = (nlm_u32)(pfxRead[i]);
				
				/* write 32b prefix to Prefix array */
				WriteBitsInArray(pfxData, NLM_FIB_REFAPP_MAX_PFX_LEN/8, end, start, PfxVal);
					end -=8;
					start -= 8;
			}
		}
	}

	/* table ID */
	WriteBitsInArray(pfxData, NLM_FIB_REFAPP_MAX_PFX_LEN/8,
    	NLM_FIB_REFAPP_MAX_PFX_LEN-1,
    	(NLM_FIB_REFAPP_MAX_PFX_LEN - 4),
    	tblId);

	/* Generate a plen bit prefix */		
	NlmCmPrefix__Set(prefix_p, pfxLen, pfxData);
	return 0 ;
 }




/* This function will create the prefix buffer, which is place holder of unique prefixes
   generated. For adding prefixes to table we will make use of this pre-generated prefix
   buffer to get the prefixes
  */
void NlmFtmRefApp__PreCreatePfxsInBufferFromFile(
		NlmFtmRefApp_RefAppInfo *refAppData_p,
		nlm_u32					*numPfxFormedSoFar_p
		)
{
	NlmCmPrefix *pfx_p = NULL;
	FILE *checkEnd;
	nlm_u32 pfxIter = 0, pfxArrayOffset = 0;
	nlm_u8	pfxData[NLMDEV_FIB_MAX_PREFIX_LENGTH/8] = {0};
	nlm_u32 tableLen = tableInfo[0].m_tblWidth;
	nlm_32 tableID = tableInfo[0].m_tblId; /* 0 */
    NlmErrNum_t errNum;
	/* round pfxLenInBits to nearest multiple of 8 */
	nlm_u16 pfxLenInBits = (nlm_u16)((tableLen + 7) & (~0x7));
	nlm_u32 sizeOfPfxStructInBytes = NlmCmPrefix__GetStorageSize(pfxLenInBits);
	nlm_u32 tries = 0;

	NlmCm__memset(refAppData_p->g_pfxArray, 0, 2000000 * sizeOfPfxStructInBytes);
	NlmCm__atoi((const nlm_8*)&tableInfo[0].m_tblId, &tableID);
	refAppData_p->m_fibOneOffset = sizeOfPfxStructInBytes;

	pfxArrayOffset = 0;
	checkEnd = refAppData_p->m_inputPfxFile_p;
	tableInfo[0].m_numOfPfxsToBeAdded = 0;

	NlmCm__printf("\n\t Fetching unique prefixes from file, adding to buffer \n\n\t\t");


	while(checkEnd != NULL)
	{
		pfx_p = (NlmCmPrefix*) &refAppData_p->g_pfxArray[pfxArrayOffset];

		NlmCmPrefix__pvt_ctor(pfx_p, pfxLenInBits, 0, NULL);

		tries = 0;
		do
		{
			tableInfo[0].m_numOfPfxsToBeAdded++;
			if((errNum = ReadPfxFromFile(pfx_p, tableInfo[0].m_tblId, refAppData_p->m_inputPfxFile_p)) != NLMERR_OK)
            {
				if(errNum == 2)
                    exit(0);
				/*if file pointer reached to end of file*/
                if(errNum == 1)
                {
                    NlmCm__printf("\n\t Warning: About data Read(prefixes in file) causing low memory on iter value !!!!!\n");
					/*this situation may occur when prefixes are less than dbCount i.e. 200000*/
            
                    checkEnd = NULL; /*stop inserting prefixes*/
                    break;
                }
                else
                {
                    NlmCm__printf("\n\t Error: Parsing the prefix file \n");
                    exit(0);
                }
			}

			/* Generate a plen bit prefix */
	//		NlmCmPrefix__Set(pfx_p, tableLen, pfxData);

		}while(NlmFtmRefApp_hashCheckDupAndInsert(refAppData_p, pfx_p));

		pfxArrayOffset += sizeOfPfxStructInBytes;

	}
	NlmCm__printf("... Done \n\n");
}

/* This function calls the sub-function to generated the prefixes */
nlm_u32 NlmFtmRefApp_GeneratePrefixes(
		NlmFtmRefApp_RefAppInfo *refAppData_p
		)
{
    nlm_u32 sizeOfPfxStructInBytes = 0;
	nlm_u32 numPfxFormedSoFar = 0;
	NlmFibTblIndexRange indexRange;

	sizeOfPfxStructInBytes = NlmCmPrefix__GetStorageSize((((nlm_u16)tableInfo[0].m_tblWidth) + 7) & (~0x7) );

	if((refAppData_p->g_pfxArray = NlmCmAllocator__calloc(
		refAppData_p->m_alloc_p, tableInfo[0].m_numOfPfxsToBeAdded, sizeOfPfxStructInBytes)) == NULL)
	{
		NlmCm__printf("\n\t cannot allocate memory for prefix array \n\n");
		return NLMERR_FAIL;
	}

	indexRange.m_indexLowValue = tableInfo[0].m_indexLow;
	indexRange.m_indexHighValue = tableInfo[0].m_indexHigh;

	NlmFtmRefApp_hashTableCreate(refAppData_p, &indexRange);
	if(refAppData_p->m_isFileEnable)
		NlmFtmRefApp__PreCreatePfxsInBufferFromFile(refAppData_p, &numPfxFormedSoFar);
	else
	NlmFtmRefApp__PreCreatePfxsInBuffer(refAppData_p, &numPfxFormedSoFar);

    return NLMERR_OK;
}


/* This function releases the memory allocated for the prefix buffer */
nlm_u32 NlmFtmRefApp_DestroyPrefixes(
    NlmFtmRefApp_RefAppInfo *refAppData_p
    )
{
	NlmFtmRefApp_hashTableDestroy(refAppData_p);
	NlmCmAllocator__free(refAppData_p->m_alloc_p, refAppData_p->g_pfxArray);

    return NLMERR_OK;
}


/* XLP ARGUMENTS */
nlm_8 XLP_CMD_ARGS[][256] =
{	
	{"-xlpxpt -th 4 -bind 0 1 2 3"},
	{"-dbcount 10"}
};


/* This function prints the usage of the command line arguments */
void 
NlmFtmRefApp_Usage()
{
	NlmCm__printf("\n\n\t______________________________________________________________");
	NlmCm__printf("\n\t Command Line Args Usage: ");
	NlmCm__printf("\n\t   -cflow     (default: 1 FTM insert thread, bind to 1) ");
	NlmCm__printf("\n\t   -blackhole (default: 1 FTM insert thread, bind to 1, no Compares)");
#ifdef NLM_XLP
	NlmCm__printf("\n\t   -xptxlp    (default: 1 FTM insert thread, bind to 1) ");
#endif
	NlmCm__printf("\n\t   -th <Num_Threads> -bind <core_num> (e.g: -th 3 -bind 1 2 3)");
	NlmCm__printf("\n\t______________________________________________________________\n\n\n");

	return;
}


/* This function parse the command line arguments */
nlm_u32 
NlmFtmRefApp_ParseArgs(NlmFtmRefApp_RefAppInfo	*refApp_p,
					   int						argc,
					   char						**argv
	)
{
	nlm_u32 threadCnt = 0;
	nlm_32 cmdArgs = 1;

	/* default values */
	refApp_p->m_numOfDevices = 1; /* as of now we support single device */
	refApp_p->m_xptType = NLM_FIB_REFAPP_SIMULATION_XPT;
	(void)threadCnt;

#ifdef NLM_NETOS

    /* We can't have command line args in XLP NETOS platform, so below default values are
     * used. Following lines should be modified to run in other configuration.
     */
    refApp_p->m_numInsThrds = NLM_FIB_REFAPP_NETOS_NUM_THREADS;

#ifdef NLM_XLP
    refApp_p->m_xptType = NLM_FIB_REFAPP_XLP_XPT;
#endif

    return NLMERR_OK;
#endif

#ifdef NLMPLATFORM_BCM
    refApp_p->m_xptType = NLM_BCM_CALADAN3_MODE;
#endif
	refApp_p->m_isFileEnable = NLMFALSE;

	while(argc > cmdArgs)
	{
		if((NlmCm__strcasecmp(argv[cmdArgs], "-cflow") == 0))
			refApp_p->m_xptType = NLM_FIB_REFAPP_SIMULATION_XPT;
		/* Number of Threads */
        else if((NlmCm__strcmp(argv[cmdArgs], "-th") == 0))
        {
            cmdArgs++;
            if ((cmdArgs < argc) && NlmCm__isdigit((nlm_32)(argv[cmdArgs][0])))
				refApp_p->m_numInsThrds = (nlm_u8)atoi(argv[cmdArgs]);

			if(refApp_p->m_numInsThrds > NLM_FIB_REFAPP_THREADS_COUNT)
			{
				NlmCm__printf("\n\t -th #num must be <= 8  \n");
				NlmFtmRefApp_Usage();
				return 1;
			}
        }
		/* bind threads to CPU specified */
		else if( NlmCm__strcasecmp("-bind", argv[cmdArgs]) == 0 )
	    {
	        while(threadCnt < refApp_p->m_numInsThrds)
            {
                cmdArgs++ ;
                
	            if ((cmdArgs < argc) && NlmCm__isdigit((nlm_32)(argv[cmdArgs][0])))
	            {
                    refApp_p->m_coreNum[threadCnt] = (nlm_u8)atoi(argv[cmdArgs]);
	            }
                else
	            {
					NlmCm__printf("\n -bind option expects %d more integer arguments (-th :%d).\n", 
						(refApp_p->m_numInsThrds - threadCnt), refApp_p->m_numInsThrds);
		            NlmFtmRefApp_Usage();
					return 1;
	            }               
                threadCnt++;
            }
	    }
		else if( NlmCm__strcasecmp("-file",argv[cmdArgs]) == 0 )
		{
			cmdArgs++;
			refApp_p->m_isFileEnable = NLMTRUE;
			refApp_p->m_inputPfxFile_p = fopen(argv[cmdArgs], "r");
			if(!(refApp_p->m_inputPfxFile_p))
			{
				NlmCm__printf("\n\t ..... Can't open file %s \n\n", argv[cmdArgs]);
				exit(1);
			}
		}
		else if(NlmCm__strcasecmp(argv[cmdArgs], "-blackhole") == 0)
            refApp_p->m_xptType = NLM_FIB_REFAPP_BLACKHOLE_XPT;    
#ifdef NLM_XLP
		else if(NlmCm__strcasecmp(argv[cmdArgs], "-xlpxpt") == 0)
            refApp_p->m_xptType = NLM_FIB_REFAPP_XLP_XPT;        
#endif

        cmdArgs++;
	}

	/* default we have single thread and bind to core 1 */
	if(threadCnt == 0)
	{
		refApp_p->m_numInsThrds = 1;
		refApp_p->m_coreNum[0] = 1;
	}

	return 0;
}




