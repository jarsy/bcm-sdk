/*
 * $Id: nlmdiag_refapp.c,v 1.1.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
 #include "nlmdiag_refapp.h"

#ifdef NLM_XLP
 #define NLM_XLP_KBP_HW_NETOS
#endif
/*----------------------------------------------------------------------------------------
 Function name: NlmDiag_ProcessCommandLineOptions

 Descrtiption: 
        Parse the command line arguments and fill the appropreate structures with the 
		valid parameters
 Return Type: 
	None
----------------------------------------------------------------------------------------*/

int 
NlmDiag_ProcessCommandLineOptions (int argc,
									  char **argv,
									  NlmDiag_TestCaseInfo *testCaseInfo_p, 
									  nlm_8 *log
									 )
{
	nlm_32 index, atoival;
	nlm_8 *nextTok;
    char *tokstr;

	/* print the usage if invalid arguments specified */
	if( argc < 2 )
	{
        NlmDiag_PrintUsage();
        return (1);
	}

    /* Parse the command line arguments */
    for( index = 1 ; index < argc ; index++) 
    {
        /* xpt mode */
        if(NlmCm__strcmp(argv[index], "-c3") == 0) 
        {
            testCaseInfo_p->m_xptMode = NLM_BCM_CALADAN3_MODE;
        }
        else if(NlmCm__strcmp(argv[index], "-fpgaflow") == 0) 
        {
			NlmCm__printf("\n\t !!.. Fpga Transport layer not supported ..!!\n\n");
			return(1);
			
            /* testCaseInfo_p->m_xptMode = NLM_FPGAXPT_MODE; */
        }
		else if(NlmCm__strcmp(argv[index], "-smode") == 0) 
        {
			testCaseInfo_p->m_operMode = NLMDEV_OPR_SAHASRA; /* Device Operating in sahasra Mode */
        }
        /* Enable AB (memory) Write, Read, and Invalidate test */
        else if(NlmCm__strcmp(argv[index], "-memtest") == 0) 
        {
            testCaseInfo_p->m_testType = NLMDIAG_AB_WRITE_READ_TEST;
        }
        /* Enable Range register write and read tests */
        else if(NlmCm__strcasecmp(argv[index], "-regtest") == 0) 	
        {
            testCaseInfo_p->m_testType  = NLMDIAG_REGISTER_WRITE_READ_TEST;
        } 
        /* Enable Search (Compare1 and Compare2) tests*/
        else if(NlmCm__strcmp(argv[index], "-cmptest") == 0) 	
        {
            index++ ;
	        if ((index < argc) && NlmCm__isdigit((int)(argv[index][0]))) 
	        {
                NlmCm__atoi(argv[index], &atoival);
                if( 1 == (nlm_u32)atoival)
                    testCaseInfo_p->m_testType  = NLMDIAG_CMP1_TEST;
                else if( 2 == (nlm_u32)atoival) 
                    testCaseInfo_p->m_testType  = NLMDIAG_CMP2_TEST;
                else
                {
                    NlmCm__printf("\n\t Parameter Missing -cmptest #n , n (1 or 2).\n\n");
                    return(0);
                }               
	        } 
            else 
	        {
                NlmCm__printf("\n\t -cmptest #n , n is either compare1 ->1 or compare 2 ->2.\n\n");
                return(0);
	        }
        } 
        /* Number of devices are in cascade */
        else if( NlmCm__strcasecmp("-chips", argv[index]) == 0 ) 
	    {
	        index++ ;
	        if ((index < argc) && NlmCm__isdigit((int)(argv[index][0]))) 
	        {
		        NlmCm__atoi(argv[index], &atoival);
                testCaseInfo_p->m_numOfChips = atoival;
                if(testCaseInfo_p->m_numOfChips < NLM_DIAG_MIN_NUM_DEV_CASCADE || 
                    testCaseInfo_p->m_numOfChips > NLM_DIAG_MAX_NUM_DEV_CASCADE)
                {
                    NlmCm__printf("\n\t Invalid parameter, cascade device count is between 1 to 4\n\n");
                    return(0);
                }
	        } 
	    } 
        /* Seed for the random data pattern generation  */
        else if( NlmCm__strcasecmp("-seed", argv[index]) == 0 ) 
	    {
	        index++ ;
	        if ((index < argc) && NlmCm__isdigit((int)(argv[index][0]))) 
	        {
		        NlmCm__atoi(argv[index], &atoival);
                testCaseInfo_p->m_testParams[0] = (nlm_u32)atoival;
	        } 
            else 
	        {
		        testCaseInfo_p->m_testParams[0] = 0x12345;
	        }
	    } 
        else if( NlmCm__strcasecmp("-logfile", argv[index]) == 0 ) 
        {
	        index++ ;
	        if ((index < argc) && NlmCm__isalnum((argv[index][0]))) 
            {
		        NlmCm__strcpy(log, argv[index]); 		        
	        }
        }
		else if((NlmCm__strcmp("-interactive", argv[index]) == 0)
			|| (NlmCm__strncmp("-interactive", argv[index], NlmCm__strlen(argv[index])) == 0))
		{
			testCaseInfo_p->m_isInteractive = 1;
		}
		else if( NlmCm__strcasecmp("-file", argv[index]) == 0 ) 
		{
			if ((index+1) < argc)
			{
				index++;
#ifndef NLMPLATFORM_BCM				
				testCaseInfo_p->m_userInput.m_inFile_fp	= NlmCmFile__fopen(argv[index], "r");
				if (testCaseInfo_p->m_userInput.m_inFile_fp == NULL)
				{
					NlmCm__printf("\n\t Unable to open %s \n", argv[index]);
					return(0);
				}
#endif				
				testCaseInfo_p->m_isInteractive = 1;
			}
			else
			{
				NlmCm__printf("\n\t Please specify -file option value \n");
				return(0);
			}
		}
		else if(NlmCm__strcmp("-dump", argv[index]) == 0)
		{
			testCaseInfo_p->m_dumpDevice = NLMTRUE;
			if ((index+1) < argc)
			{
				index++;
				if( NlmCm__strcasecmp("all", argv[index]) == 0 ) 
				{
					testCaseInfo_p->m_devDumpOptions.m_operation = NLM_DUMP_COMPLETE_DEVICE;
				}
				else if( NlmCm__strcasecmp("blk", argv[index]) == 0 ) 
				{
					testCaseInfo_p->m_devDumpOptions.m_operation = NLM_DUMP_BLOCKS;
				}
				else if( NlmCm__strcasecmp("ltr", argv[index]) == 0 ) 
				{
					testCaseInfo_p->m_devDumpOptions.m_operation = NLM_DUMP_LTR_REGISTERS;
				}
				else if( NlmCm__strcasecmp("internal", argv[index]) == 0 ) 
				{
					testCaseInfo_p->m_devDumpOptions.m_operation = NLM_DUMP_INTERNAL_REGISTERS;
				}
				else if( NlmCm__strcasecmp("gbl", argv[index]) == 0 ) 
				{
					testCaseInfo_p->m_devDumpOptions.m_operation = NLM_DUMP_GLOBAL_REGISTER;
				}
				else if( NlmCm__strcasecmp("cb", argv[index]) == 0 ) 
				{
					testCaseInfo_p->m_devDumpOptions.m_operation = NLM_DUMP_CB_MEMORY;
				}
				else if( NlmCm__strcasecmp("sahasra", argv[index]) == 0 ) 
				{
					testCaseInfo_p->m_devDumpOptions.m_operation = NLM_DUMP_SAHASRA_MEMORY;
				}
				else
				{
					 NlmCm__printf("\n\t Invalid Argumrnt :%s \n\n", argv[index]);
					return(0);
				}
			}
			else
			{
				NlmCm__printf("\n\t Please specify -dump option value \n");
				return(0);
			}
		}
		else if(NlmCm__strcmp("-devid", argv[index]) == 0)
		{
			if ((index+1) < argc)
			{
				index++;
				nextTok = sal_strtok_r (argv[index], "-", &tokstr);
				NlmCm__atoi(nextTok, &atoival);
				testCaseInfo_p->m_devDumpOptions.m_startDevId = (nlm_u8)atoival;
				nextTok = sal_strtok_r (NULL, "-", &tokstr);
				NlmCm__atoi(nextTok, &atoival);
				testCaseInfo_p->m_devDumpOptions.m_endDevId = (nlm_u8)atoival;				
			}
			else
			{
				NlmCm__printf("\n\t Please specify -devid option value \n");
				return(0);
			}
		}
		else if(NlmCm__strcmp("-range", argv[index]) == 0)
		{
			if ((index+1) < argc)
			{
				index++;
				nextTok = sal_strtok_r (argv[index], "-", &tokstr);
				NlmCm__atoi(nextTok, &atoival);
				testCaseInfo_p->m_devDumpOptions.m_startRange = (nlm_u16)atoival;
				nextTok = sal_strtok_r (NULL, "-", &tokstr);
				NlmCm__atoi(nextTok, &atoival);
				testCaseInfo_p->m_devDumpOptions.m_endRange = (nlm_u16)atoival;
			}
			else
			{
				NlmCm__printf("\n\t Please specify -range option value \n");
				return(0);
			}
		}
        else
        {
            NlmDiag_PrintUsage();
            NlmCm__printf("\n\t Invalid Argumrnt :%s \n\n", argv[index]);
            return(0);
        }
    }
    return (0);
}

/*----------------------------------------------------------------------------------------
 Function name: NlmDiag_CompleteDiagTest

 Parameters:
	* alloc_p : memory allocation pointer
	* testInfo_p : test info pointer
	* testCaseInfo_p : testcase info pointer
	* logfile : logfile pointer
 Descrtiption: 
        Performs the full device register , block entry, CB memory read/write operations
 Return Type: 
	nlm_u32
----------------------------------------------------------------------------------------*/

nlm_u32
NlmDiag_CompleteDiagTest(void *alloc_p, 
							  NlmDiag_TestInfo *testInfo_p,
							  NlmDiag_TestCaseInfo *testCaseInfo_p, 
							  nlm_8 *logfile
							 )
{
	nlm_u32 errNum = 0;
	
	if( (testCaseInfo_p->m_testType > NLMDIAG_REGISTER_WRITE_READ_TEST) &&
		(testCaseInfo_p->m_operMode == NLMDEV_OPR_SAHASRA) )
	{
		NlmCm__printf("\nInvalid selection, -smode supported only in register/memory tests");
		return 1;
	}
	switch(testCaseInfo_p->m_testType)
	{
			/* Database (Array Block ) write/read Tests (memory tests) */
			case NLMDIAG_AB_WRITE_READ_TEST:
				NlmDiag_CreateLogFileDetails("memory_test",testInfo_p, logfile);
				errNum += NlmDiag_MemoryTests(alloc_p, testInfo_p, testCaseInfo_p);
				break;

			/* Register Write and Read Tests */
			case NLMDIAG_REGISTER_WRITE_READ_TEST:
				NlmDiag_CreateLogFileDetails("register_test",testInfo_p, logfile);
				errNum += NlmDiag_RegisterReadWriteTests(alloc_p, testInfo_p, testCaseInfo_p);
				break;
	            
			/* Search Tests with Compare1 and Compare2 */
			case NLMDIAG_CMP1_TEST:
	            
				/* Run compare1 test */
				NlmDiag_CreateLogFileDetails("compare1_test",testInfo_p, logfile);
				errNum += NlmDiag_CompareTests(alloc_p, testInfo_p, testCaseInfo_p);
				break;

			case NLMDIAG_CMP2_TEST:
	            
				/* Run compare2 test */
				NlmDiag_CreateLogFileDetails("compare2_test",testInfo_p, logfile);
				errNum += NlmDiag_CompareTests(alloc_p, testInfo_p, testCaseInfo_p);
				break;

			default:
				NlmCm__printf("\nInvalid selection");
				break;        
	}
	return errNum;
}

nlm_u32
NlmDiag_ProcessCommand (
						   NlmDiag_TestInfo *testInfo_p,
						   NlmDiag_TestCaseInfo *testCaseInfo_p
						  )
{
	nlm_32 errorCount = 0;

	switch(testCaseInfo_p->m_userInput.m_operation)
	{
		case NLM_READ_GLOBAL_REGISTER:
			testCaseInfo_p->m_userInput.m_registerType = NlmDiag_GetGolbalRegisterNumber(testCaseInfo_p->m_userInput.m_address);
			if (testCaseInfo_p->m_userInput.m_registerType == 0xFF)
			{
				NlmCm__printf("\n\trdgblreg -addr <20b hex val> -devid <0-3>\n");
				break;
			}	
			errorCount += NlmDiag_ReadFromGlobalReg(testInfo_p);
			break;
		
		case NLM_WRITE_GLOBAL_REGISTER:
			testCaseInfo_p->m_userInput.m_registerType = NlmDiag_GetGolbalRegisterNumber(testCaseInfo_p->m_userInput.m_address);
			if (testCaseInfo_p->m_userInput.m_registerType == 0xFF)
			{
				NlmCm__printf("\n\twrgblreg -addr <20b hex val> -devid <0-3> -data <80b hex val>\n");
				break;
			}
			errorCount += NlmDiag_WriteToGlobalReg(testInfo_p);
			break;
		
		case NLM_READ_LTR_REGISTER:
			if (testCaseInfo_p->m_userInput.m_address & 0xF0000)
			{
				NlmCm__printf("\n\tIncorrect Address\n");
				NlmCm__printf("\n\trdltrreg -addr <20b hex val> -devid <0-3>\n");
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tIncorrect Address.\n");
				errorCount = NLMDIAG_ERROR_INVALID_INPUT;
				break;
			}
			NlmDiag_GetLTRRegisterInfo(&testCaseInfo_p->m_userInput.m_ltrNum, 
				&testCaseInfo_p->m_userInput.m_registerType, testCaseInfo_p->m_userInput.m_address);
			if ((testCaseInfo_p->m_userInput.m_ltrNum > NLMDEV_NUM_LTR_SET)
				|| (testCaseInfo_p->m_userInput.m_registerType > NLMDEV_LTR_REG_END))
			{
				NlmCm__printf("\n\tIncorrect Address\n");
				NlmCm__printf("\n\trdltrreg -addr <20b hex val> -devid <0-3>\n");
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tIncorrect Address.\n");
				errorCount = NLMDIAG_ERROR_INVALID_INPUT;
				break;
			}
			errorCount += NlmDiag_ReadFromLTR(testInfo_p);
			NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tREAD_LTR_REGISTER Completed.\n");
			break;

		case NLM_WRITE_LTR_REGISTER:
			if (testCaseInfo_p->m_userInput.m_address & 0xF0000)
			{
				NlmCm__printf("\n\tIncorrect Address\n");
				NlmCm__printf("\n\twrltrreg -addr <20b hex val> -devid <0-3> -data <80b hex val>\n");
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tIncorrect Address.\n");
				return(NLMDIAG_ERROR_INVALID_INPUT);
			}
			NlmDiag_GetLTRRegisterInfo(&testCaseInfo_p->m_userInput.m_ltrNum, 
				&testCaseInfo_p->m_userInput.m_registerType, testCaseInfo_p->m_userInput.m_address);
			if ((testCaseInfo_p->m_userInput.m_ltrNum > NLMDEV_NUM_LTR_SET)
				|| (testCaseInfo_p->m_userInput.m_registerType > NLMDEV_LTR_REG_END))
			{
				NlmCm__printf("\n\tIncorrect Address\n");
				NlmCm__printf("\n\twrltrreg -addr <20b hex val> -devid <0-3> -data <80b hex val>\n");
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tIncorrect Address.\n");
				errorCount = NLMDIAG_ERROR_INVALID_INPUT;
				break;
			}
			errorCount += NlmDiag_WriteToLTR(testInfo_p);
			break;

		case NLM_READ_BLOCK_REGISTER:
			if (testCaseInfo_p->m_userInput.m_address & 0xF0000)
			{
				NlmCm__printf("\n\tIncorrect Address\n");
				NlmCm__printf("\n\trdblkreg -addr <20b hex val> -devid <0-3> \n");
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tIncorrect Address.\n");
				errorCount = NLMDIAG_ERROR_INVALID_INPUT;
				break;
			}
			NlmDiag_GetBlockRegisterInfo(&testCaseInfo_p->m_userInput.m_blkNum,
				&testCaseInfo_p->m_userInput.m_registerType, testCaseInfo_p->m_userInput.m_address);
			if ((testCaseInfo_p->m_userInput.m_blkNum > NLMDEV_NUM_ARRAY_BLOCKS)
				|| (testCaseInfo_p->m_userInput.m_registerType > NLMDEV_BLOCK_REG_END))
			{
				NlmCm__printf("\n\tIncorrect Address\n");
				NlmCm__printf("\n\trdblkreg -addr <20b hex val> -devid <0-3> \n");
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tIncorrect Address.\n");
				errorCount = NLMDIAG_ERROR_INVALID_INPUT;
				break;
			}
			errorCount += NlmDiag_ReadFromBlockReg(testInfo_p);
			break;

		case NLM_WRITE_BLOCK_REGISTER:
			if (testCaseInfo_p->m_userInput.m_address & 0xF0000)
			{
				NlmCm__printf("\n\tIncorrect Address\n");
				NlmCm__printf("\n\twrblkreg -addr <20b hex val> -devid <0-3> -data <80b hex val>\n");
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tIncorrect Address.\n");
				errorCount = NLMDIAG_ERROR_INVALID_INPUT;
				break;
			}
			NlmDiag_GetBlockRegisterInfo(&testCaseInfo_p->m_userInput.m_blkNum,
				&testCaseInfo_p->m_userInput.m_registerType, testCaseInfo_p->m_userInput.m_address);
			if ((testCaseInfo_p->m_userInput.m_blkNum > NLMDEV_NUM_ARRAY_BLOCKS)
				|| (testCaseInfo_p->m_userInput.m_registerType > NLMDEV_BLOCK_REG_END))
			{
				NlmCm__printf("\n\tIncorrect Address\n");
				NlmCm__printf("\n\twrblkreg -addr <20b hex val> -devid <0-3> -data <80b hex val>\n");
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tIncorrect Address.\n");
				errorCount = NLMDIAG_ERROR_INVALID_INPUT;
				break;
			}
			errorCount += NlmDiag_WriteToBlockReg(testInfo_p);
			NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tWRITE_BLOCK_REGISTER Completed.\n");
			break;

		case NLM_READ_BLOCK_ENTRY:
			if (testCaseInfo_p->m_userInput.m_address > 0x7FFFF)
			{
				NlmCm__printf("\n\tIncorrect Address\n");
				NlmCm__printf("\n\trdblkentry -addr <20b hex val> -devid <0-3> \n");
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tIncorrect Address.\n");
				errorCount = NLMDIAG_ERROR_INVALID_INPUT;
				break;
			}
			testCaseInfo_p->m_userInput.m_blkNum = (nlm_u8) (testCaseInfo_p->m_userInput.m_address >> 12) & 0x7F;
			testCaseInfo_p->m_userInput.m_address = testCaseInfo_p->m_userInput.m_address & 0xFFF;
			
			errorCount += NlmDiag_ReadBlockEntry(testInfo_p);
			break;

		case NLM_WRITE_BLOCK_ENTRY:
			if (testCaseInfo_p->m_userInput.m_address > 0x7FFFF)
			{
				NlmCm__printf("\n\tIncorrect Address\n");
				NlmCm__printf("\n\twrblkentry -addr <20b hex val> -devid <0-3> \n");
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tIncorrect Address.\n");
				errorCount = NLMDIAG_ERROR_INVALID_INPUT;
				break;
			}
			testCaseInfo_p->m_userInput.m_blkNum = (nlm_u8) (testCaseInfo_p->m_userInput.m_address >> 12) & 0x7F;
			testCaseInfo_p->m_userInput.m_address = testCaseInfo_p->m_userInput.m_address & 0xFFF;
			
			errorCount += NlmDiag_WriteBlockEntry(testInfo_p);
			break;

		case NLM_READ_CB_MEMORY:
			if (testInfo_p->m_userInput_p->m_address >= NLMDEV_CB_DEPTH)
			{
				NlmCm__printf("\n\tIncorrect Address\n");
				NlmCm__printf("\n\trdcbmem -addr <20b hex val> \n");
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tIncorrect Address.\n");
				errorCount = NLMDIAG_ERROR_INVALID_INPUT;
				break;
			}			
			errorCount += NlmDiag_ReadFromCBMemory(testInfo_p);
			break;

		case NLM_WRITE_CB_MEMORY:
			if (testInfo_p->m_userInput_p->m_address >= NLMDEV_CB_DEPTH)
			{
				NlmCm__printf("\n\tIncorrect Address\n");
				NlmCm__printf("\n\twrcbmem -addr <20b hex val> -data <80b-640b (multiple of 80b) hex>\n");
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tIncorrect Address.\n");
				errorCount = NLMDIAG_ERROR_INVALID_INPUT;
				break;
			}
			errorCount += NlmDiag_WriteToCBMemory(testInfo_p);
			break;
		case NLM_COMP_ONE:
			if (testInfo_p->m_userInput_p->m_address > NLMDEV_CB_DEPTH)
			{
				NlmCm__printf("\n\tIncorrect Address\n");
				NlmCm__printf("\n\tcmp1 -ltr <0-63> -addr <CB Addr 20b hex val> -data <80b-640b (multiple of 80b) hex>\n");
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tIncorrect Address.\n");
				errorCount = NLMDIAG_ERROR_INVALID_INPUT;
				break;
			}
			errorCount += NlmDiag_CompareOne(testInfo_p);
			break;
		case NLM_COMP_TWO:
			if (testInfo_p->m_userInput_p->m_address > NLMDEV_CB_DEPTH)
			{
				NlmCm__printf("\n\tIncorrect Address\n");
				NlmCm__printf("\n\tcmp2 -ltr <0-63> -addr <CB Addr 20b hex val> -data <80b-640b (multiple of 80b) hex>\n");
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tIncorrect Address.\n");
				errorCount = NLMDIAG_ERROR_INVALID_INPUT;
				break;
			}
			errorCount += NlmDiag_CompareTwo(testInfo_p);
			break;
		case NLM_READ_RANGE_REGISTER:
				if(testInfo_p->m_userInput_p->m_address < NLMDIAG_RANGE_START_ADDRESS
					|| testInfo_p->m_userInput_p->m_address > NLMDIAG_RANGE_REGISTER_COUNT)
				{
				NlmCm__printf("\n\tIncorrect Range Register Address\n");
				NlmCm__printf("\n\trdrngreg -addr <20b hex val> \n");
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tIncorrect Range Register Address\n");
				errorCount = NLMDIAG_ERROR_INVALID_INPUT;
				break;
			}
			errorCount += NlmDiag_ReadFromRangeRegister(testInfo_p);
			break;
		case NLM_WRITE_RANGE_REGISTER:
			if(testInfo_p->m_userInput_p->m_address < NLMDIAG_RANGE_START_ADDRESS
					|| testInfo_p->m_userInput_p->m_address > NLMDIAG_RANGE_REGISTER_COUNT)
				{
					NlmCm__printf("\n\tIncorrect Range Register Address\n");
					NlmCm__printf("\n\twrrngreg -addr <20b hex val> -data <80b hex val>\n");
					NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tIncorrect Range Register Address\n");
					errorCount = NLMDIAG_ERROR_INVALID_INPUT;
					break;
				}
				errorCount += NlmDiag_WriteToRangeRegister(testInfo_p);
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tWRITE_RANGE_REGISTER Completed.\n");
			break;
		default:
				NlmCm__printf("\nInvalid selection");
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tInvalid selection.\n");
				break;
	}
	return errorCount;
}

/*----------------------------------------------------------------------------------------
 Function name: NlmDiag_InteractiveDiagTest

 Parameters:
	* alloc_p : memory allocation pointer
	* testInfo_p : test info pointer
	* testCaseInfo_p : testcase info pointer
	* logfile : logfile pointer
 Descrtiption: 
        Performs the register , block entry, CB memory read/write operations
 Return Type: 
	nlm_u32
----------------------------------------------------------------------------------------*/

nlm_u32
NlmDiag_InteractiveDiagTest(void *alloc_p, 
							  NlmDiag_TestInfo *testInfo_p,
							  NlmDiag_TestCaseInfo *testCaseInfo_p, 
							  nlm_8 *logfile
							 )
{
	nlm_8 cmdLineOptions[NLMDIAG_FILE_READLINE_LEN] = "\0", printUsage = 0, quit = 0;
    nlm_32 errorCount = 0, errNum, index = 0;

	NlmDiag_CreateLogFileDetails("interactive_test",testInfo_p, logfile);
	if((errNum = NlmDiag_CommonInitTestInfo(alloc_p, testInfo_p, testCaseInfo_p)) !=0)
	{
		errorCount += errNum;
		NlmCm__printf("\n\n\n\tNlmDiag_CommonInitTestInfo is Failed.\n");
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tNlmDiag_CommonInitTestInfo is Failed .\n");
	}
	if((errNum = NlmDiag_Initialize(testInfo_p)) !=NLMDIAG_ERROR_NOERROR)
		errorCount += errNum;
	if (testCaseInfo_p->m_userInput.m_inFile_fp != NULL)
	{
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tFile mode Interactive tests.\n");
		while(!(feof(testCaseInfo_p->m_userInput.m_inFile_fp)))
		{
			fgets(&cmdLineOptions[0], ((NLMDIAG_FILE_READLINE_LEN ) - 1 ), testCaseInfo_p->m_userInput.m_inFile_fp);
			if (NlmDiag_ParseOptions(cmdLineOptions, &testCaseInfo_p->m_userInput, &printUsage, &quit) != NLMDIAG_ERROR_NOERROR )
			{
				continue;
			}
			if (quit)
				break;
			if ((testCaseInfo_p->m_userInput.m_operation != NLM_COMP_ONE)
				&& (testCaseInfo_p->m_userInput.m_operation != NLM_COMP_TWO)
				&& (testCaseInfo_p->m_userInput.m_operation != NLM_READ_CB_MEMORY)
				&& (testCaseInfo_p->m_userInput.m_operation != NLM_WRITE_CB_MEMORY))
				{
					if (testCaseInfo_p->m_userInput.m_devId > (testCaseInfo_p->m_numOfChips - 1))
					{
						NlmCm__printf("\n\tError : Incorrect Device ID \n");
						NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tError : Incorrect Device ID \n");
						continue;
					}
			}
			if (!printUsage)
			{
				errorCount += NlmDiag_ProcessCommand (testInfo_p, testCaseInfo_p);
			}
			else
			{
				NlmDiag_PrintInteractiveUsage();
				printUsage = 0;
			}
		}
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tFile mode Interactive tests completed.\n");
	}
	else
	{
		NlmDiag_PrintInteractiveUsage();
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tCommand mode Interactive tests.\n");
		do
		{
			fflush (stdin);
			NlmCm__printf ("\n\nCMD> ");
			index = 0;
			cmdLineOptions[index] = (nlm_8)getc(stdin);
			while (cmdLineOptions[index] != '\n')
			{
				cmdLineOptions[++index] = (nlm_8)getc(stdin);
			}
			index++;
			cmdLineOptions[index] = '\0';
			if (NlmDiag_ParseOptions(cmdLineOptions, &testCaseInfo_p->m_userInput, &printUsage, &quit) != NLMDIAG_ERROR_NOERROR )
			{
				continue;
			}
			if (quit)
				break;
			if (!printUsage)
			{
				if ((testCaseInfo_p->m_userInput.m_operation != NLM_COMP_ONE)
				&& (testCaseInfo_p->m_userInput.m_operation != NLM_COMP_TWO)
				&& (testCaseInfo_p->m_userInput.m_operation != NLM_READ_CB_MEMORY)
				&& (testCaseInfo_p->m_userInput.m_operation != NLM_WRITE_CB_MEMORY))
				{
					if (testCaseInfo_p->m_userInput.m_devId > (testCaseInfo_p->m_numOfChips - 1))
					{
						NlmCm__printf("\n\tError : Incorrect Device ID \n");
						NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tError : Incorrect Device ID \n");
						continue;
					}
				}
				errorCount += NlmDiag_ProcessCommand (testInfo_p, testCaseInfo_p);
			}
			else
			{
				NlmDiag_PrintInteractiveUsage();
				printUsage = 0;
			}
		}while (quit != NLMTRUE);
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tCommand mode Interactive tests completed.\n");
	}
	/* Call Destroy function which destroys Xpt, dev mgr and releases devices*/
	if((errNum = NlmDiag_Destroy(testInfo_p))!=NLMDIAG_ERROR_NOERROR)
		errorCount +=errNum;

		/* Release testblkinfo after each test case */
	if((errNum = NlmDiag_CommonDestroyTestInfo(testInfo_p)) != 0)
	{
		NlmCm__printf("\n\tCan't destroy test information.\n");
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tCan't destroy test information.\n");
		errorCount +=errNum;
	} 
	return errorCount;
}

/*----------------------------------------------------------------------------------------
 Function name: NlmDiag_PrintUsage

 Parameters:
            None.                  
 Descrtiption: 
        Prints the usage of diag ref app. It prints information about supported
        commond line parameters.
 Return Type: None.
----------------------------------------------------------------------------------------*/
void 
NlmDiag_PrintUsage()
{
    NlmCm__printf("\n\t-------------------------------------------------------------------------------------");
    NlmCm__printf("\n\tUsage : ");
    NlmCm__printf("\n\t -cflow/-fpgaflow               : xpt mode *");
    NlmCm__printf("\n\t -chips #n  (1-4)               : number of chips in cascade *");
    NlmCm__printf("\n\t -memtest/-regtest              : diagnostic test *");
	NlmCm__printf("\n\t -cmptest 1/2                   : Compare tests *");
	NlmCm__printf("\n\t -logfile <filename.txt>]       : Log filename to generate");
    NlmCm__printf("\n\t -seed #val                     : Seed value (decimal) to generate the random numbers");
	NlmCm__printf("\n\t -smode                         : Sahasra mode enabled for memtest");
	NlmCm__printf("\n\t-------------------------------------------------------------------------------------");
    NlmCm__printf("\n\t -interactive/-in               : Interactive Mode \n");
	NlmCm__printf("\n\t -file <commands.txt>           : Batch Mode \n");
	NlmCm__printf("\n\t-------------------------------------------------------------------------------------");
    NlmCm__printf("\n\t -dump <all/gbl/blk/ltr/internal/cb>  : Dump Device content\n");
    NlmCm__printf("\n\t With -dump these are required options -devid <start_dev_id>-<end_dev_id> -range <start-end>\n");
	NlmCm__printf("\n\t-------------------------------------------------------------------------------------");
    NlmCm__printf("\n\n\t (*) are arguments are required");
    NlmCm__printf("\n\t-------------------------------------------------------------------------------------\n");
}

/*----------------------------------------------------------------------------------------
 Function name: NlmDiag_PrintInteractiveUsage

 Parameters:
            None.                  
 Descrtiption: 
        Prints the interactive usage of diag ref app. It prints information about supported
        commond line parameters.
 Return Type: None.
----------------------------------------------------------------------------------------*/
void 
NlmDiag_PrintInteractiveUsage()
{
	NlmCm__printf("\nUsage : ");
	NlmCm__printf("\n----------------------------------------------------------------------");
	NlmCm__printf("\n\nCommand Mode ");
	NlmCm__printf("\n Read Operations:: ");
	NlmCm__printf("\n\trdgblreg  : Global Register");
	NlmCm__printf("\trdltrreg    : LTR Register");
	NlmCm__printf("\n\trdblkreg  : Block Register ");
	NlmCm__printf("\trdblkentry  : Block Entry");
	NlmCm__printf("\n\trdcbmem   : CB Memory      ");
	NlmCm__printf("\trdrngreg    : Range Register\n");
	NlmCm__printf("\n\tEx: rdgblreg -devid <0-3> -addr <20b hex val>");
	NlmCm__printf("\n\tEx: rdcbmem -addr <20b hex val>\n");
	NlmCm__printf("\n Write Operations:: ");
	NlmCm__printf("\n\twrgblreg  : Global Register");
	NlmCm__printf("\twrltrreg  : LTR Register");
	NlmCm__printf("\n\twrblkreg  : Block Register ");
	NlmCm__printf("\twrcbmem   : CB Memory");
	NlmCm__printf("\n\twrrngreg  : Range Register ");
	NlmCm__printf("\n\twrblkentry -mask <80b hex val> -data <80b hex val> : Block Entry\n");
	NlmCm__printf("\n\tEx: wrgblreg -devid <0-3> -addr <20b hex val> -data <80b hex val>");
	NlmCm__printf("\n\tEx: wrcbmem -addr <20b hex val> -data <80b-640b (multiple of 80b) hex>\n");
	NlmCm__printf("\n Compare Operations:: ");
	NlmCm__printf("\n\tcmp1 : Perform Compare One");
	NlmCm__printf("\n\tcmp2 : Perform Compare Two\n");
	NlmCm__printf("\n\tEx: cmp1 -ltr <0-63> -addr <CB Addr 20b hex val> -data <80b-640b (multiple of 80b) hex>");
	NlmCm__printf("\n\tEx: cmp2 -ltr <0-63> -addr <CB Addr 20b hex val> -data <80b-640b (multiple of 80b) hex>\n");
	NlmCm__printf("\n\n\thelp/h   : Print Options Menu");
	NlmCm__printf("\n\tquit/q   : Quit Interactive Test");
	NlmCm__printf("\n----------------------------------------------------------------------\n");
}

/*----------------------------------------------------------------------------------------
 Function name: NlmDiag_HexString2CharArray

 Parameters:
	* hexString : input hex tsring
	* strBuffer : output char array
	* length : string length of the input 
 Descrtiption: 
        Parses the input hex string nad fills up the char array 
 Return Type: None.
----------------------------------------------------------------------------------------*/
void 
NlmDiag_HexString2CharArray(
	nlm_8 *hexString,
    nlm_u8 *strBuffer,
    nlm_u32 length
    )
{
    /* Local variables */
    nlm_8 *tempPtr = NULL;
    nlm_u32 iter = 0;
    nlm_u32 hexChar = 0;
    
    NlmCm__memset(strBuffer,0, length);

	tempPtr = hexString;
	tempPtr++;
	tempPtr++;
    NlmCm__strcpy((char *)strBuffer,"");    
    
    for(iter = 0 ;iter < length; iter++)
    {   
        if(*tempPtr == '\0') 
            break;        
        sscanf(tempPtr,"%2x",&hexChar);
        strBuffer[iter] = (nlm_8)hexChar;       
        tempPtr = tempPtr + 2;
    }
	strBuffer[iter] = '\0';
}

/*----------------------------------------------------------------------------------------
 Function name: NlmDiag_GetGolbalRegisterNumber

 Parameters:
	* address : absolute address of the register
 Descrtiption: 
        Calculate the Global register type from the given absolute address
 Return Type: 
	* nlm_u8
----------------------------------------------------------------------------------------*/
nlm_u8
NlmDiag_GetGolbalRegisterNumber(
	nlm_u32 address
	)
{
	nlm_u8 retValue = 0xFF;

	switch (address)
	{
		case 0x00000:
			retValue = NLMDEV_DEVICE_ID_REG;
			break;
		case 0x00001:
			retValue = NLMDEV_DEVICE_CONFIG_REG;
			break;
		case 0x00002:
			retValue = NLMDEV_ERROR_STATUS_REG;
			break;
		case 0x00003:
			retValue = NLMDEV_ERROR_STATUS_MASK_REG;
			break;
		case 0x00005:
			retValue = NLMDEV_DATABASE_SOFT_ERROR_FIFO_REG;
			break;
		case 0x00019:
			retValue = NLMDEV_ADVANCED_FEATURES_SOFT_ERROR_REG;
			break;
		case 0x80000:
			retValue = NLMDEV_SCRATCH_PAD0_REG;
			break;
		case 0x80001:
			retValue = NLMDEV_SCRATCH_PAD1_REG;
			break;
		case 0x80010:
			retValue = NLMDEV_RESULT0_REG;
			break;
		case 0x80011:
			retValue = NLMDEV_RESULT1_REG;
			break;
		default:
			NlmCm__printf ("\n\tInvalid Register address\n");
	}
	return retValue;
}

/*----------------------------------------------------------------------------------------
 Function name: NlmDiag_GetLTRRegisterInfo

 Parameters:
	* ltrNum : LTR Number
	* regType : sub LTR register type
	* address : absolute LTR register address
 Descrtiption: 
        Calculates the LTR number and register type from the given absolute address
 Return Type: 
	* None
----------------------------------------------------------------------------------------*/

void 
NlmDiag_GetLTRRegisterInfo(
	nlm_u8 *ltrNum,
	nlm_u8* regType,
	nlm_u32 address
	)
{
	/* Calculate the LTR number and register type */
	*ltrNum = (nlm_u8)(address - 0x4000) / 0x20;
	*regType = (nlm_u8)(address - 0x4000) % 0x20;
}

/*----------------------------------------------------------------------------------------
 Function name: NlmDiag_GetLTRRegisterInfo

 Parameters:
	* ch : input character
 Descrtiption: 
        Calculates hex value of the given character
 Return Type: 
	* nlm_u32
----------------------------------------------------------------------------------------*/

nlm_32 
NlmDiag_GetHexChar(
	nlm_8 ch
	)
{
	if(ch >= '0' && ch <= '9')
        return (ch - '0');

    /* make the function non case sensitive */
    ch |= 0x20;
    if(ch >= 'a' && ch <= 'f')
        return ((ch - 'a') + 10);
    return(0);
}

/*----------------------------------------------------------------------------------------
 Function name: NlmDiag_GetHexValue

 Parameters:
	* str : input string
	* startChar : start position
	* endChar : end position
 Descrtiption: 
        Calculates hex value of the given string
 Return Type: 
	* nlm_u32
----------------------------------------------------------------------------------------*/

nlm_u32
NlmDiag_GetHexValue(
	nlm_8* str,
	nlm_32 startChar,
	nlm_32 endChar
	)
{
    nlm_u32 pow = 1;
    nlm_u32 sum = 0;
    nlm_32 index;
    nlm_u32 charVal;

	/* Calculate the hex value of the given string */
    for(index = endChar; index >= startChar; index--)
    {
        charVal = NlmDiag_GetHexChar(str[index]);
        sum = sum + (charVal * pow);
        pow = pow * 16;
    }
    return sum;
}

/*----------------------------------------------------------------------------------------
 Function name: NlmDiag_GetBlockRegisterInfo

 Parameters:
	* blkNum : Block Number
	* regType : Block register type
	* address : absolute Block register address
 Descrtiption: 
        Calculates the block number and register type from the given absolute address
 Return Type: 
	* None
----------------------------------------------------------------------------------------*/

void 
NlmDiag_GetBlockRegisterInfo(
	nlm_u8 *blkNum,
	nlm_u8* regType,
	nlm_u32 address
	)
{
	/* Calculate the block number and register type */
	*blkNum = (nlm_u8)(address - 0x1000) / 0x20;
	*regType = (nlm_u8)(address - 0x1000) % 0x20;
}

/*----------------------------------------------------------------------------------------
 Function name: NlmDiag_ParseOptions

 Parameters:
	* options : Input options
	* userInput : output Structure to filled	
 Descrtiption: 
        Parse the options and fillup the structure
 Return Type: 
	* nlm_u32
----------------------------------------------------------------------------------------*/

nlm_u32
NlmDiag_ParseOptions(
	nlm_8 *options,
	NlmDiag_UserInput *userInput,
	nlm_8 *printUsage, 
	nlm_8* quitFlag
	)
{
    nlm_u8	paramStart = 0, paramEnd = 0, paramIndex = 0, charIndex=0;
	nlm_32	cmdLineLen = 0,index = 0, atoival, dataLen = 0;
	nlm_8	token[10][50];
	nlm_u8	rdFlag = 0, wrFlag = 0, addrFlag = 0, dataFlag = 0;
	nlm_u8	maskFlag = 0, devIdFlag = 0, blkEntryFlag = 0, cmpFlag = 0, ltrFlag = 0, blankLine = 1;
	nlm_u8 multiWrFlag = 0;

	cmdLineLen = (nlm_u32)NlmCm__strlen(options);

    /* Continue reading the params until end of line */
    while(paramEnd < cmdLineLen)
    {
        /* Remove the tabs and space in the line before token (pre skipping)*/
        while(options[paramEnd] == '\t' || options[paramEnd] == ' ')
            paramEnd++;

        /* If new param received break the loop (pre checking)*/
        if(options[paramEnd] == '\n')
            break;

        /* get the param ; End of param indicated by space or next line character */
        while(options[paramEnd] != ' ' && options[paramEnd] != '\t' && 
			options[paramEnd] != '\n' && options[paramEnd] != '\0')
		{
            token[paramIndex][charIndex++] = options[paramEnd++];
			blankLine = 0;
		}

        token[paramIndex][charIndex] = '\0';
		paramIndex++;
        charIndex=0;
        paramStart = paramEnd;
		
        /* If new param received break the loop (post checking) */
        if(options[paramEnd] == '\n')
            break;

        /* remove tabs and spaces after the token read, wait till alhanumeric char encountered */
        while(options[paramStart++] == ' ');

        paramEnd = paramStart - 1;
	}

	/*check for blank line */
	if (blankLine)
		return (NLMDIAG_ERROR_INVALID_INPUT);
	if((NlmCm__strcmp("help", token[0]) == 0)
		|| (NlmCm__strcmp("h", token[0]) == 0))
	{
		*printUsage = 1;
		return (NLMDIAG_ERROR_NOERROR);
	}
	/*check for quit/q option */
	else if((NlmCm__strcmp("quit", token[0]) == 0)
		|| (NlmCm__strcmp("q", token[0]) == 0))
	{
		*quitFlag = 1;
		return (NLMDIAG_ERROR_NOERROR);
	}
	else if(NlmCm__strcmp("rdgblreg", token[0]) == 0)
	{
		userInput->m_operation = NLM_READ_GLOBAL_REGISTER;
		rdFlag = 1;
	}
	else if(NlmCm__strcmp("rdltrreg", token[0]) == 0)
	{
		userInput->m_operation = NLM_READ_LTR_REGISTER;
		rdFlag = 1;
	}
	else if(NlmCm__strcmp("rdblkreg", token[0]) == 0)
	{
		userInput->m_operation = NLM_READ_BLOCK_REGISTER;
		rdFlag = 1;
	}
	else if(NlmCm__strcmp("rdblkentry", token[0]) == 0)
	{
		userInput->m_operation = NLM_READ_BLOCK_ENTRY;
		rdFlag = 1;
	}
	else if(NlmCm__strcmp("rdcbmem", token[0]) == 0)
	{
		userInput->m_operation = NLM_READ_CB_MEMORY;
		rdFlag = 1;
		devIdFlag = 1; /* FOr CB Read/Write  we don't need devid */
	}
	else if(NlmCm__strcmp("wrgblreg", token[0]) == 0)
	{
		userInput->m_operation = NLM_WRITE_GLOBAL_REGISTER;
		wrFlag = 1;
	}
	else if(NlmCm__strcmp("wrltrreg", token[0]) == 0)
	{
		userInput->m_operation = NLM_WRITE_LTR_REGISTER;
		wrFlag = 1;
	}
	else if(NlmCm__strcmp("wrblkreg", token[0]) == 0)
	{
		userInput->m_operation = NLM_WRITE_BLOCK_REGISTER;
		wrFlag = 1;
	}
	else if(NlmCm__strcmp("wrblkentry", token[0]) == 0)
	{
		userInput->m_operation = NLM_WRITE_BLOCK_ENTRY;
		wrFlag = 1;
		blkEntryFlag = 1;
	}
	else if(NlmCm__strcmp("wrcbmem", token[0]) == 0)
	{
		userInput->m_operation = NLM_WRITE_CB_MEMORY;
		wrFlag = 1;
		devIdFlag = 1; /* FOr CB Read/Write  we don't need devid */
		multiWrFlag = 1;
	}
	else if(NlmCm__strcmp("rdrngreg", token[0]) == 0)
	{
		userInput->m_operation = NLM_READ_RANGE_REGISTER;
		rdFlag = 1;
	}
	else if(NlmCm__strcmp("wrrngreg", token[0]) == 0)
	{
		userInput->m_operation = NLM_WRITE_RANGE_REGISTER;
		wrFlag = 1;
	}
	else if(NlmCm__strcmp("cmp1", token[0]) == 0)
	{
		userInput->m_operation = NLM_COMP_ONE;
		cmpFlag = 1;
	}
	else if(NlmCm__strcmp("cmp2", token[0]) == 0)
	{
		userInput->m_operation = NLM_COMP_TWO;
		cmpFlag = 1;
	}
	else
	{
		NlmCm__printf("\n\tError : Please specify the read/write/compare as first option\n\n");
		return (NLMDIAG_ERROR_INVALID_INPUT);
	}

	index = 1;
	/* Compare the options and get the values */
	while (index < paramIndex)
	{
		if(NlmCm__strcmp("-devid", token[index]) == 0)
		{
			index++;
			devIdFlag = 1;
			if (index < paramIndex) 
			{
				NlmCm__atoi(token[index], &atoival);
				userInput->m_devId = (nlm_u8)atoival;
			}
			else
			{
				NlmCm__printf("\n\tError : Please specify -devid value\n\n");
				return NLMDIAG_ERROR_INVALID_INPUT;
			}
		}
		else if(NlmCm__strcmp("-addr", token[index]) == 0)
		{
			index++;
			addrFlag = 1;
			if (index < paramIndex) 
			{
				/*check the 0x part of addr value*/
				if(NlmCm__strncmp("0x", token[index], 2) != 0)
				{
					NlmCm__printf("\n\tError : Please include 0x prefix for -addr value\n\n");
					return NLMDIAG_ERROR_INVALID_INPUT;
				}
				/*-addr value shoud be 20 bit hex value ex 0x12345 */
				if (NlmCm__strlen(token[index]) > 7)
				{
					NlmCm__printf("\n\tError : Please specify -addr 20b Hex Value\n\n");
					return NLMDIAG_ERROR_INVALID_INPUT;
				}
				sscanf(token[index], "%x",&userInput->m_address); 
			}
			else
			{
				NlmCm__printf("\n\tError : Please specify -addr value\n\n");
				return NLMDIAG_ERROR_INVALID_INPUT;
			}
		}
		else if(NlmCm__strcmp("-data", token[index]) == 0)
		{
			index++;
			dataFlag = 1;
			if (index < paramIndex) 
			{
				if (cmpFlag)
				{
					if(NlmCm__strncmp("0x", token[index], 2) == 0)
					{
						/* for compare data should be multiple of 80b i.e 20 characters + "0x" so
						reminder should be 2 */
						if (NlmCm__strlen(token[index]) % 20 != 2) 
						{
							NlmCm__printf("\n\tError : Please specify multiple of 80bit -data value\n\n");
							return NLMDIAG_ERROR_INVALID_INPUT;
						}
						else
						{
							dataLen = (nlm_u32)(NlmCm__strlen(token[index])/2) - 1;
							NlmDiag_HexString2CharArray (token[index], userInput->m_compareData, dataLen);
							userInput->m_compDataLen = (nlm_u32)(dataLen);
						}
					}
					else
					{
						NlmCm__printf("\n\tError : Please include 0x prefix for -data value\n\n");
						return NLMDIAG_ERROR_INVALID_INPUT;
					}
				}
				else
				{
					if(NlmCm__strncmp("0x", token[index], 2) == 0)
					{
						/* for register write data should be 80b i.e 20 characters + "0x" so
							totally 22 char*/
						if(multiWrFlag)
						{
							/* for compare data should be multiple of 80b i.e 20 characters + "0x" so
							reminder should be 2 */
							if (NlmCm__strlen(token[index]) % 20 != 2) 
							{
								NlmCm__printf("\n\tError : Please specify multiple of 80bit -data value (upto 640b)\n\n");
								return NLMDIAG_ERROR_INVALID_INPUT;
							}
							
							dataLen = (nlm_u32)(NlmCm__strlen(token[index])/2) - 1;
							NlmDiag_HexString2CharArray (token[index], userInput->m_compareData, dataLen);
							userInput->m_compDataLen = (nlm_u32)(dataLen);
							multiWrFlag = 0; /* reset the multiFlag */

						}
						else
						{
							if ((NlmCm__strlen(token[index]) < 22)
								||(NlmCm__strlen(token[index]) > 22))
							{
								NlmCm__printf("\n\tError : Please specify 80bit -data value\n\n");
								return NLMDIAG_ERROR_INVALID_INPUT;
							}
												
							dataLen = (nlm_u32)(NlmCm__strlen(token[index])/2) - 1;
							NlmDiag_HexString2CharArray (token[index], userInput->m_data, dataLen);
							multiWrFlag = 0; /* reset the multiFlag */
						}
					}
					else
					{
						NlmCm__printf("\n\tError : Please include 0x prefix for -data value\n\n");
						return NLMDIAG_ERROR_INVALID_INPUT;
					}
				}
			}
			else
			{
				NlmCm__printf("\n\tError : Please specify -data value\n\n");
				return NLMDIAG_ERROR_INVALID_INPUT;
			}
		}
		else if(NlmCm__strcmp("-mask", token[index]) == 0)
		{
			index++;
			maskFlag = 1;
			if (index < paramIndex) 
			{
				/* for block entry mask should be 80b i.e 20 characters + "0x" so
				   totally 22 char*/
				if (NlmCm__strlen(token[index]) < 22)
				{
					NlmCm__printf("\n\tError : Please specify 80bit -mask value\n\n");
					return NLMDIAG_ERROR_INVALID_INPUT;
				}
				NlmDiag_HexString2CharArray (token[index], userInput->m_mask, ((nlm_16)NlmCm__strlen(token[index])/2));
			}
			else
			{
				NlmCm__printf("\n\tError : Please specify -mask value\n\n");
				return NLMDIAG_ERROR_INVALID_INPUT;
			}
		}
		else if(NlmCm__strcmp("-verbose", token[index]) == 0)
		{
			userInput->m_verbose = 1;
		}
		else if(NlmCm__strcmp("-ltr", token[index]) == 0)
		{
			index++;
			ltrFlag = 1;
			if (index < paramIndex) 
			{
				NlmCm__atoi(token[index], &atoival);
				/* LTR set 0-63 */
				if (atoival < 0 || atoival > 63)
				{
					NlmCm__printf("\n\tError : Please specify LTR value with in 0-63 range\n\n");
					return NLMDIAG_ERROR_INVALID_INPUT;
				}
				userInput->m_ltrNum = (nlm_u8)atoival;
			}
			else
			{
				NlmCm__printf("\n\tError : Please specify -ltr value\n\n");
                return NLMDIAG_ERROR_INVALID_INPUT;
			}
		}
		else
        {
            NlmCm__printf("\n\t Invalid Argumrnt :%s \n\n", token[index]);
            return(0);
        }
		index++;
	}

	/* Check all the necessary options */
	/* if it is write block entry then data, mask, address and device id is must */
	if (blkEntryFlag && (!dataFlag || !maskFlag || !addrFlag || !devIdFlag))
	{	
		NlmCm__printf("\n\t Please Provide all (-data, -mask, -addr, -devid) options \n");
		return NLMDIAG_ERROR_INVALID_INPUT;
	}
	else if ((wrFlag) && (!dataFlag || !addrFlag || !devIdFlag))
	{
		/* if it is write operation then data, address and device id is must */
		NlmCm__printf("\n\t Please Provide all (-data -addr -devid) options \n");
		return NLMDIAG_ERROR_INVALID_INPUT;
	}
	else if ((rdFlag) && (!addrFlag || !devIdFlag))
	{
		/* if it is read operation then address and device id is must */
		NlmCm__printf("\n\t Please Provide all (-addr -devid) options \n");
		return NLMDIAG_ERROR_INVALID_INPUT;
	}
	else if ((cmpFlag) && (!dataFlag || !addrFlag || !ltrFlag))
	{
		/* if it is write operation then data, address and ltr is must */
		NlmCm__printf("\n\t Please Provide all (-data, -addr, -ltr) options \n");
		return NLMDIAG_ERROR_INVALID_INPUTPTR;
	}
	return(NLMDIAG_ERROR_NOERROR);
}


/*----------------------------------------------------------------------------------------
 Function name: main

 Parameters:
            * argc  : Number of command line arguments
            * argv  : Array of character pointers
                  
 Descrtiption: 
        The main() which initializes the device and performs set of diagnostic tests.
        The diagnostics tests are: memory write-read tests, serach-compare
        tests, and register read only/write-read tests. 

 Return Type: int
----------------------------------------------------------------------------------------*/
 
int nlmdiag_refapp_main(
	int argc,
	char** argv
	)
{
    /* diag specific */
    NlmDiag_TestInfo testInfo;
    NlmDiag_TestCaseInfo testCaseInfo;
    
    /* memory allocators */
    NlmCmAllocator *alloc_p = NULL;
    NlmCmAllocator alloc_body;
      
    nlm_8 logfile[40] = "NULL";
    nlm_32 errorCount = 0;
    nlm_32 break_alloc_id = -1; 
	
    /* Default parameters for the diagnostic test */ 
	testCaseInfo.m_operMode      = NLMDEV_OPR_STANDARD;          /* Device Operating in Standard Mode */
    testCaseInfo.m_testParams[0] = NLMDIAG_DEFAULT_SEED_VALUE;   /* seed value */
    testCaseInfo.m_testType      = NLMDIAG_REGISTER_WRITE_READ_TEST;   /* Default Test */
    testCaseInfo.m_errCount      = 0;                              /* Initially error count is 0 */
    testCaseInfo.m_numOfChips    = 1;                              /* default only single device */
    testCaseInfo.m_xptMode       = NLM_SIMULATION_MODE;            /* default simulation mode */ 
	testCaseInfo.m_isInteractive = NLMFALSE;
	testCaseInfo.m_dumpDevice = NLMFALSE;

	testCaseInfo.m_userInput.m_address = 0xFFFFF;
	testCaseInfo.m_userInput.m_devId = 0xFF;
	testCaseInfo.m_userInput.m_operation = 0xFF;
	testCaseInfo.m_userInput.m_verbose = 0;
	testCaseInfo.m_userInput.m_inFile_fp = NULL;

#ifndef NLM_XLP_KBP_HW_NETOS
    NlmDiag_ProcessCommandLineOptions (argc,argv,&testCaseInfo, logfile);
#endif

	NlmCm__printf ("\n\t=======================================================================");
    NlmCm__printf ("\n\t                 Diagnostic Tests.                  ");
    NlmCm__printf ("\n\t=======================================================================\n");

    NlmCmDebug__Setup(break_alloc_id, NLMCM_DBG_EBM_ENABLE);  
    
    /* creating memory allocator */
    alloc_p = NlmCmAllocator__ctor(&alloc_body);
    NlmCmAssert((alloc_p != NULL), "The memory allocator cannot be NULL!\n");
    if (alloc_p == NULL)
    {
        NlmCm__printf("\n\tThe memory allocator cannot be NULL, Not enough MEMORY !!!!!!\n");
        return (-1);
    }

	if (testCaseInfo.m_isInteractive)
	{
		errorCount += NlmDiag_InteractiveDiagTest (alloc_p, &testInfo, &testCaseInfo, logfile);
	}
	else if (testCaseInfo.m_dumpDevice)
	{
		errorCount += NlmDiag_ValidateOptions (&testCaseInfo.m_devDumpOptions, testCaseInfo.m_numOfChips);		
		if (!errorCount)
            errorCount += NlmDiag_DumpDevice (alloc_p, &testInfo, &testCaseInfo, logfile);
		else
			 NlmCm__printf("\n\t Invalid Input Parameter\n");
	}
	else
	{
#ifndef NLM_XLP_KBP_HW_NETOS
    	NlmCm__printf ("\n\tDiagnostic now run with %d device/s in cascade\n", testCaseInfo.m_numOfChips);
		errorCount += NlmDiag_CompleteDiagTest (alloc_p, &testInfo, &testCaseInfo, logfile);
#else
		NlmDiag_TestType i = 0;
		nlm_u32 error = 0;

		NlmCm__printf ("\n\tDiagnostic now run with %d device/s in cascade\n", testCaseInfo.m_numOfChips);
	
		i = NLMDIAG_AB_WRITE_READ_TEST;			/* Memeory write/read/invalidate tests */
		/*i = NLMDIAG_REGISTER_WRITE_READ_TEST;*/	/* All register default read and write/read tests */
		/*i = NLMDIAG_CMP1_TEST;*/                  /* Compare1 test (only in standard speed mode) */
		/*i = NLMDIAG_CMP2_TEST;*/   				/* Compare2 test (only in standard speed mode) */
		{
		
			/* Default parameters for the diagnostic test */ 
			 testCaseInfo.m_operMode	  = NLMDEV_OPR_STANDARD;		     /* Device Operating in Standard Mode */
			 /*testCaseInfo.m_operMode	  = NLMDEV_OPR_SAHASRA; */		     /* Device Operating in Sahasra Mode */

			 testCaseInfo.m_testParams[0] = NLMDIAG_DEFAULT_SEED_VALUE;   /* seed value */
			 testCaseInfo.m_errCount	  = 0;								/* Initially error count is 0 */
			 testCaseInfo.m_numOfChips	  = 1;								/* default only single device */
			 testCaseInfo.m_xptMode 	  = NLM_XLPXPT_MODE;			 /* default simulation mode */ 
			 testCaseInfo.m_isInteractive = NLMFALSE;
			 testCaseInfo.m_dumpDevice = NLMFALSE;
			
			 testCaseInfo.m_userInput.m_address = 0xFFFFF;
			 testCaseInfo.m_userInput.m_devId = 0xFF;
			 testCaseInfo.m_userInput.m_operation = 0xFF;
			 testCaseInfo.m_userInput.m_verbose = 0;
			 testCaseInfo.m_userInput.m_inFile_fp = NULL;

			error = 0;
			testCaseInfo.m_testType = i;
			NlmCm__printf ("\n\n!!!!Diagnostic Test Type = %d\n\n", i);
			error = NlmDiag_CompleteDiagTest (alloc_p,&testInfo, &testCaseInfo, logfile);
			errorCount += error;
			if(error)
				NlmCm__printf("\n\n Few Diagnostic test(s) FAILED ...Test Type = %d, Error Count = %d.\n", 
											i, errorCount);
		}
#endif
	}

    /* check for the memory leak */
    if(NlmCmDebug__IsMemLeak() != NlmFALSE)
    {
        NlmCm__printf("\n\t Memory Leak occured\n");
        return(0);
    }

    /* check the Diagnostic tests error count, if 0 then run successful, otherwise few test(s) failed;
     * check log file for status
     */
    if(errorCount)
        NlmCm__printf("\n\n Few Diagnostic test(s) FAILED .... please check the log file.\n");
    else
        NlmCm__printf("\n\n Successful completion of Diag tests.... please check the log file.\n");
    
    return(0);
}


/*  11  */

