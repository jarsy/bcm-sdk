/*
 * $Id: nlmdiag_device_dump.c,v 1.1.8.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#include "nlmdiag_refapp.h"
#include "nlmdiag_registertest.h"
#include "nlmdiag_device_dump.h"
/* Function to dispaly Device ID register */
static void 
NlmDiag_pvt_DisplayDevIdReg (
	NlmDevIdReg* devIdRegReadInfo_p, 
	FILE *fp)
{
	NlmCmFile__fprintf(fp,"\n\t\t\tMinor Die Rev. -> %d", devIdRegReadInfo_p->m_minorDieRev);
	NlmCmFile__fprintf(fp,"\n\t\t\tMajor Die Rev. -> %d", devIdRegReadInfo_p->m_majorDieRev);
	NlmCmFile__fprintf(fp,"\n\t\t\tDatabase Size -> %d", devIdRegReadInfo_p->m_databaseSize);
}

/* Function to dispaly Device Cinfiguration register */
static void 
NlmDiag_pvt_DisplayDevConfigReg (
	NlmDevConfigReg* devConfigRegReadInfo_p, 
	FILE *fp)
{
	NlmCmFile__fprintf(fp,"\n\t\t\tSoft Error Scan Enable  = %x", devConfigRegReadInfo_p->m_softErrorScanEnable);
	NlmCmFile__fprintf(fp,"\n\t\t\tDatabase Parity Error Entry Invalidate = %x", devConfigRegReadInfo_p->m_dbParityErrEntryInvalidate);
	NlmCmFile__fprintf(fp,"\n\t\t\tDatabase Soft Error Protection Mode = %x", devConfigRegReadInfo_p->m_dbSoftErrProtectMode);
	NlmCmFile__fprintf(fp,"\n\t\t\tECC Scan Type = %x", devConfigRegReadInfo_p->m_eccScanType);
	NlmCmFile__fprintf(fp,"\n\t\t\tRange Matching Engine Enable = %x", devConfigRegReadInfo_p->m_rangeEngineEnable);
	NlmCmFile__fprintf(fp,"\n\t\t\tLow Power Mode Enable = %x", devConfigRegReadInfo_p->m_lowPowerModeEnable);
}

/* Function to dispaly Device Error Status register */
static void 
NlmDiag_pvt_DisplayDevErrStatusReg (
	NlmDevErrStatusReg* devErrStatRegReadInfo_p, 
	FILE *fp)
{
	NlmCmFile__fprintf(fp,"\n\t\t\tDatabase Soft Error = %x", devErrStatRegReadInfo_p->m_dbSoftError);
	NlmCmFile__fprintf(fp,"\n\t\t\tDatabase Soft Error FIFO Full = %x", devErrStatRegReadInfo_p->m_dbSoftErrorFifoFull);
	NlmCmFile__fprintf(fp,"\n\t\t\tContext BUffer Parity Error = %x", devErrStatRegReadInfo_p->m_ctxBufferParityErr);
	NlmCmFile__fprintf(fp,"\n\t\t\tIllegal Instruction Error = %x", devErrStatRegReadInfo_p->m_illegalInstnErr);
}

/* Function to dispaly Device Error Status Mask register */
static void 
NlmDiag_pvt_DisplayDevErrStatusMaskReg (
	NlmDevErrStatusReg* devErrStatMaskRegReadInfo_p, 
	FILE *fp)
{
	NlmCmFile__fprintf(fp,"\n\t\t\tDatabase Soft Error = %x", devErrStatMaskRegReadInfo_p->m_dbSoftError);
	NlmCmFile__fprintf(fp,"\n\t\t\tDatabase Soft Error FIFO Full = %x", devErrStatMaskRegReadInfo_p->m_dbSoftErrorFifoFull);
	NlmCmFile__fprintf(fp,"\n\t\t\tContext BUffer Parity Error = %x", devErrStatMaskRegReadInfo_p->m_ctxBufferParityErr);
	NlmCmFile__fprintf(fp,"\n\t\t\tIllegal Instruction Error = %x", devErrStatMaskRegReadInfo_p->m_illegalInstnErr);
}

/* Function to dispaly Device DataBase Soft Error FIFO register */
static void 
NlmDiag_pvt_DisplayDevDbSoftErrFifoReg (
	NlmDevDbSoftErrFifoReg* devDBSoftErrStatRegReadInfo_p, 
	FILE *fp)
{
	NlmCmFile__fprintf(fp,"\n\t\t\tError Address = %x", devDBSoftErrStatRegReadInfo_p->m_errorAddr);
	NlmCmFile__fprintf(fp,"\n\t\t\tXPerr = %x", devDBSoftErrStatRegReadInfo_p->m_pErrorX);
	NlmCmFile__fprintf(fp,"\n\t\t\tYPerr = %x", devDBSoftErrStatRegReadInfo_p->m_pErrorY);
	NlmCmFile__fprintf(fp,"\n\t\t\tError = %x", devDBSoftErrStatRegReadInfo_p->m_errorAddrValid);
	NlmCmFile__fprintf(fp,"\n\t\t\tErase FIFO Entry = %x", devDBSoftErrStatRegReadInfo_p->m_eraseFifoEntry);
	NlmCmFile__fprintf(fp,"\n\t\t\tErase FIFO = %x", devDBSoftErrStatRegReadInfo_p->m_eraseFifo);
}

/* Function to dispaly Device Advanced Features Soft Error register */
static void 
NlmDiag_pvt_DisplayDevAdvancedSoftErrReg (
	NlmDevAdvancedSoftErrReg* devAdvSoftErrRegReadInfo_p, 
	FILE *fp)
{
	NlmCmFile__fprintf(fp,"\n\t\t\tContext Buffer Parity Error Address = %x", devAdvSoftErrRegReadInfo_p->m_cbParityErrAddr);
}

/* Function to dispaly Device Result register 0 */
static void 
NlmDiag_pvt_DisplayDevResult0Reg (
	NlmDevResultReg* devResultReg0ReadInfo_p, 
	FILE *fp)
{
	NlmCmFile__fprintf(fp,"\n\t\t\tHighest Priority Match Address (#0) = %x", devResultReg0ReadInfo_p->m_hitAddress[0]);
	NlmCmFile__fprintf(fp,"\n\t\t\tSearch Match (#0) = %x", devResultReg0ReadInfo_p->m_hitOrMiss[0]);
	NlmCmFile__fprintf(fp,"\n\t\t\tHighest Priority Match Address (#1) = %x", devResultReg0ReadInfo_p->m_hitAddress[1]);
	NlmCmFile__fprintf(fp,"\n\t\t\tSearch Match (#1) = %x", devResultReg0ReadInfo_p->m_hitOrMiss[1]);
}

/* Function to dispaly Device Result register 1 */
static void 
NlmDiag_pvt_DisplayDevResult1Reg (
	NlmDevResultReg* devResultReg1ReadInfo_p, 
	FILE *fp)
{
	NlmCmFile__fprintf(fp,"\n\t\t\tHighest Priority Match Address (#2) = %x", devResultReg1ReadInfo_p->m_hitAddress[0]);
	NlmCmFile__fprintf(fp,"\n\t\t\tSearch Match (#2) = %x", devResultReg1ReadInfo_p->m_hitOrMiss[0]);
	NlmCmFile__fprintf(fp,"\n\t\t\tHighest Priority Match Address (#3) = %x", devResultReg1ReadInfo_p->m_hitAddress[1]);
	NlmCmFile__fprintf(fp,"\n\t\t\tSearch Match (#3) = %x", devResultReg1ReadInfo_p->m_hitOrMiss[1]);
}

/* Function to dispaly Device ScratchPad register 0 */
static void 
NlmDiag_pvt_DisplayDevScratchPad0Reg (
	NlmDevScratchPadReg* scratchPadRegReadInfo_p, 
	FILE *fp)
{
	NlmCmFile__fprintf(fp,"\n\t\t\tUser Data = ");
	NlmDiag_PrintHexDataToFile (scratchPadRegReadInfo_p->m_data, NLMDEV_REG_LEN_IN_BYTES, fp);
}

/* Function to dispaly Device ScratchPad register 1 */
static void 
NlmDiag_pvt_DisplayDevScratchPad1Reg (
	NlmDevScratchPadReg* scratchPadRegReadInfo_p, 
	FILE *fp)
{
	NlmCmFile__fprintf(fp,"\n\t\t\tUser Data = ");
	NlmDiag_PrintHexDataToFile (scratchPadRegReadInfo_p->m_data, NLMDEV_REG_LEN_IN_BYTES, fp);
}

/* Function to dispaly LTR block selection register */
static nlm_u32
NlmDiag_pvt_DisplayBlkSelectReg(
	NlmDevBlkSelectReg* blkSelectReg_p,
	nlm_u32 regNr, 
	FILE *fp)
{
	nlm_u32 blkNum = 0;
	nlm_u32 numBlksSelected = 0;

	nlm_u32 numBlocksPerReg = NLMDEV_NUM_ARRAY_BLOCKS/2;

	for(blkNum = 0; blkNum < numBlocksPerReg; ++blkNum)
	{
		if(blkSelectReg_p->m_blkEnable[blkNum])
		{
			NlmCmFile__fprintf(fp, "%d, ", (regNr * numBlocksPerReg) + blkNum); 
			++numBlksSelected;
		}
	}

	return numBlksSelected;
}

static void
NlmDiag_pvt_DisplaySuperBlkKeyMap(
	NlmDevSuperBlkKeyMapReg *superBlkKeyMapReg_p, 
	FILE *fp)
{
	nlm_u32 superBlockNr = 0;

	NlmCmFile__fprintf(fp, "Super Block -> Key Mapping ");

	for(superBlockNr = 0; superBlockNr < NLMDEV_NUM_SUPER_BLOCKS; ++superBlockNr)
	{
		if(superBlockNr % 8 == 0 )
			NlmCmFile__fprintf(fp, "\n");

		NlmCmFile__fprintf(fp, "%2d -> %d, ", superBlockNr, superBlkKeyMapReg_p->m_keyNum[superBlockNr]);
		
	}
}

static void
NlmDiag_pvt_DisplayParallelSrchReg(
	NlmDevParallelSrchReg * psReg_p,
	nlm_u32 regNr, 
	FILE *fp)
{
	nlm_u32 blkNr = 0;
	nlm_u32 numBlksPerReg = NLMDEV_NUM_ARRAY_BLOCKS/4;	

	for(blkNr = 0; blkNr < numBlksPerReg; ++blkNr)
	{
		if(blkNr % 8 == 0)
			NlmCmFile__fprintf(fp, "\n");

		NlmCmFile__fprintf(fp, "%3d -> %d, ", (regNr * numBlksPerReg) + blkNr, psReg_p->m_psNum[blkNr]);
	}
}

static void
NlmDiag_pvt_DisplayRangeEncodingType(
	NlmDevRangeEncodingType rangeEncodingType, 
	FILE *fp)
{
	switch(rangeEncodingType)
	{
		case NLMDEV_3BIT_RANGE_ENCODING:
		{
			NlmCmFile__fprintf(fp, "NLMDEV_3BIT_RANGE_ENCODING \n");
			break;
		}
		case NLMDEV_2BIT_RANGE_ENCODING:
		{
			NlmCmFile__fprintf(fp, "NLMDEV_2BIT_RANGE_ENCODING \n");
			break;
		}
		case NLMDEV_NO_RANGE_ENCODING:
		{
			NlmCmFile__fprintf(fp, "NLMDEV_NO_RANGE_ENCODING \n");
			break;
		}
		default:
		{
			NlmCmFile__fprintf(fp, "Unknown = %d \n", rangeEncodingType);
			break;
		}
	}
}

static void
NlmDiag_pvt_DisplayRangeEncodedValue(
	NlmDevRangeEncodedValueBytes rangeEncodedValueBytes, 
	FILE *fp)
{
	switch(rangeEncodedValueBytes)
	{
		case NLMDEV_1BYTE_RANGE_ENCODED_VALUE:
		{
			NlmCmFile__fprintf(fp, "NLMDEV_1BYTE_RANGE_ENCODED_VALUE \n");
			break;
		}
		case NLMDEV_2BYTE_RANGE_ENCODED_VALUE:
		{
			NlmCmFile__fprintf(fp, "NLMDEV_2BYTE_RANGE_ENCODED_VALUE \n");
			break;
		}
		case NLMDEV_3BYTE_RANGE_ENCODED_VALUE:
		{
			NlmCmFile__fprintf(fp, "NLMDEV_3BYTE_RANGE_ENCODED_VALUE \n");
			break;
		}
		case NLMDEV_4BYTE_RANGE_ENCODED_VALUE:
		{
			NlmCmFile__fprintf(fp, "NLMDEV_4BYTE_RANGE_ENCODED_VALUE \n");
			break;
		}
		default:
		{
			NlmCmFile__fprintf(fp, "Unknown = %d \n", rangeEncodedValueBytes);
			break;
		}
	}
}

static void
NlmDiag_pvt_DisplayRangeInsert0Reg(
	NlmDevRangeInsertion0Reg *rangeInsertion0Reg_p, 
	FILE *fp)
{
	nlm_u32 keyNum = 0;

	NlmCmFile__fprintf(fp, "\n\nRange Insert0 Register \n");

	NlmCmFile__fprintf(fp, "Range A encoding type = "); 
	NlmDiag_pvt_DisplayRangeEncodingType(rangeInsertion0Reg_p->m_rangeAEncodingType, fp);

	NlmCmFile__fprintf(fp, "Range B encoding type = ");
	NlmDiag_pvt_DisplayRangeEncodingType(rangeInsertion0Reg_p->m_rangeBEncodingType, fp);

	NlmCmFile__fprintf(fp, "Range A encoded bytes = ");
	NlmDiag_pvt_DisplayRangeEncodedValue(rangeInsertion0Reg_p->m_rangeAEncodedBytes, fp);

	NlmCmFile__fprintf(fp, "Range B encoded bytes = ");
	NlmDiag_pvt_DisplayRangeEncodedValue(rangeInsertion0Reg_p->m_rangeBEncodedBytes, fp);

	NlmCmFile__fprintf(fp, "Range A insert start byte: ");
	for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; ++keyNum)
	{
		NlmCmFile__fprintf(fp, "Key Num %d -> Start Byte %d, ", 
					keyNum, rangeInsertion0Reg_p->m_rangeAInsertStartByte[keyNum]);
	}
	NlmCmFile__fprintf(fp, "\n");

	NlmCmFile__fprintf(fp, "Range B insert start byte: ");
	for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; ++keyNum)
	{
		NlmCmFile__fprintf(fp, "Key Num %d -> Start Byte %d, ", 
					keyNum, rangeInsertion0Reg_p->m_rangeBInsertStartByte[keyNum]);
	}
	NlmCmFile__fprintf(fp, "\n\n");
}

static void
NlmDiag_pvt_DisplayRangeInsert1Reg(
	NlmDevRangeInsertion1Reg *rangeInsertion1Reg_p, 
	FILE *fp)
{
	nlm_u32 keyNum = 0;

	NlmCmFile__fprintf(fp, "\n\nRange Insert1 Register \n");
	
	NlmCmFile__fprintf(fp, "Range C encoding type = "); 
	NlmDiag_pvt_DisplayRangeEncodingType(rangeInsertion1Reg_p->m_rangeCEncodingType, fp);

	NlmCmFile__fprintf(fp, "Range D encoding type = ");
	NlmDiag_pvt_DisplayRangeEncodingType(rangeInsertion1Reg_p->m_rangeDEncodingType, fp);

	NlmCmFile__fprintf(fp, "Range C encoded bytes = ");
	NlmDiag_pvt_DisplayRangeEncodedValue(rangeInsertion1Reg_p->m_rangeCEncodedBytes, fp);

	NlmCmFile__fprintf(fp, "Range D encoded bytes = ");
	NlmDiag_pvt_DisplayRangeEncodedValue(rangeInsertion1Reg_p->m_rangeDEncodedBytes, fp);


	NlmCmFile__fprintf(fp, "Range C insert start byte: ");
	for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; ++keyNum)
	{
		NlmCmFile__fprintf(fp, "Key Num %d -> Start Byte %d, ", 
					keyNum, rangeInsertion1Reg_p->m_rangeCInsertStartByte[keyNum]);
	}
	NlmCmFile__fprintf(fp, "\n");

	NlmCmFile__fprintf(fp, "Range D insert start byte: ");
	for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; ++keyNum)
	{
		NlmCmFile__fprintf(fp, "Key Num %d -> Start Byte %d, ", 
					keyNum, rangeInsertion1Reg_p->m_rangeDInsertStartByte[keyNum]);
	}
	NlmCmFile__fprintf(fp, "\n\n");
}


static void
NlmDiag_pvt_DisplayMiscReg(
	NlmDevMiscelleneousReg *miscReg_p, 
	FILE *fp)
{
	nlm_u32 psNr = 0;

	NlmCmFile__fprintf(fp, "\n\n Miscellaneous Register \n");

	for(psNr = 0; psNr < NLMDEV_NUM_PARALLEL_SEARCHES; ++psNr)
	{
		NlmCmFile__fprintf(fp, "PS %d -> BMR %d, ", psNr, miscReg_p->m_bmrSelect[psNr]);  
	}

	NlmCmFile__fprintf(fp,	"\nRange A extract start byte = %d \n", miscReg_p->m_rangeAExtractStartByte);
	NlmCmFile__fprintf(fp, "Range B extract start byte = %d \n", miscReg_p->m_rangeBExtractStartByte);
	NlmCmFile__fprintf(fp, "Range C extract start byte = %d \n", miscReg_p->m_rangeCExtractStartByte);
	NlmCmFile__fprintf(fp, "Range D extract start byte = %d \n", miscReg_p->m_rangeDExtractStartByte);
	NlmCmFile__fprintf(fp, "Number of valid search results = %d \n", miscReg_p->m_numOfValidSrchRslts);
	
	for(psNr = 0; psNr < NLMDEV_NUM_PARALLEL_SEARCHES; ++psNr)
	{
		NlmCmFile__fprintf(fp, "PS %d -> Search type %d, ", psNr, miscReg_p->m_searchType[psNr]);
	}
	NlmCmFile__fprintf(fp, "\n");
}


static void
NlmDiag_pvt_DisplaySSReg(
	NlmDevSSReg *ssReg_p, 
	FILE *fp)
{
	nlm_u32 i = 0;

	NlmCmFile__fprintf(fp, "\n\n SS Register \n");

	for(i = 0; i < NLMDEV_SS_RMP_AB; ++i)
	{
		NlmCmFile__fprintf(fp, "SS Block %d -> PE %d, ", i, ssReg_p->m_ss_result_map[i]);

		if( (i + 1) % 4 == 0)
			NlmCmFile__fprintf(fp, "\n");
	}

	NlmCmFile__fprintf(fp, "\n");
}

static void
NlmDiag_pvt_DisplayKCR(
	NlmDevKeyConstructReg *kcr_p, 
	FILE *fp)
{
	nlm_u32 segNr = 0;

	NlmCmFile__fprintf(fp, "Start Byte  : ");
	
	for(segNr = 0; segNr < NLMDEV_NUM_OF_SEGMENTS_PER_KCR; ++segNr)
	{
		NlmCmFile__fprintf(fp, "%5d, ", kcr_p->m_startByteLoc[segNr]);
	}

	NlmCmFile__fprintf(fp, "\n");
	NlmCmFile__fprintf(fp, "Num of Bytes: ");
	
	for(segNr = 0; segNr < NLMDEV_NUM_OF_SEGMENTS_PER_KCR; ++segNr)
	{
		NlmCmFile__fprintf(fp, "%5d, ", kcr_p->m_numOfBytes[segNr]);
	}
	NlmCmFile__fprintf(fp, "\n");
}

static void 
NlmDiag_pvt_DisplaySTInfo(
    NlmDevSTE *stInfo_p,
    FILE* fp)
{
    NlmCmFile__fprintf(fp, "B: %d, BW: %d, BMN: %d, ", 
					stInfo_p->m_ss_abs,
					NLMDEV_AB_WIDTH_IN_BITS * (1 << stInfo_p->m_bw), 
					stInfo_p->m_ss_bsel);

	NlmCmFile__fprintf(fp, "BI: %d, SESST: %d, SID: %d, SLR: %d, ", 
					stInfo_p->m_bix, 
					stInfo_p->m_ssi_s, 
					stInfo_p->m_ssi, 
					stInfo_p->m_slr); 

	NlmCmFile__fprintf(fp, "LV: %d, LI: %d ", 
					stInfo_p->m_lprv, 
					stInfo_p->m_lpri);
}

void 
NlmDiag_PrintHexDataToFile(
	nlm_u8 *string,
	nlm_u32 len,
	NlmCmFile *logFile_p
	)
{
	nlm_u32 index = 0;
	for (index = 0; index < len; index++)
	{
		NlmCmFile__fprintf(logFile_p, "%02x", string[index] );		
	}
}

nlm_u32
NlmDiag_ValidateOptions (
	NlmDiag_DevDumpOptions *userInputs,
	nlm_u32 numOfChips)
{
	nlm_u8 errorFlag = NLMDIAG_ERROR_NOERROR;
	if (userInputs->m_endDevId >= numOfChips)
	{
		NlmCm__printf("\n\tError : End_DevId > Number of chips\n");
		errorFlag = NLMDIAG_ERROR_INVALID_INPUT; 
	}

	if (userInputs->m_startDevId > userInputs->m_endDevId)
	{
		NlmCm__printf("\n\tError : Start_DevId > End_DevId\n");
		errorFlag = NLMDIAG_ERROR_INVALID_INPUT; 
	}

	switch (userInputs->m_operation)
	{
		case NLM_DUMP_COMPLETE_DEVICE:
		case NLM_DUMP_INTERNAL_REGISTERS:
		case NLM_DUMP_GLOBAL_REGISTER:
		case NLM_DUMP_SAHASRA_MEMORY:
			break;
		case NLM_DUMP_LTR_REGISTERS:
			if (userInputs->m_startRange > userInputs->m_endRange)
			{
				NlmCm__printf("\n\tError : start_range > end_range\n");
				errorFlag = NLMDIAG_ERROR_INVALID_INPUT; 
			}
			if (userInputs->m_endRange >= NLMDEV_NUM_LTR_SET)
			{
				NlmCm__printf("\n\tError : end_range > Number of LTR set(0-63)\n");
				errorFlag = NLMDIAG_ERROR_INVALID_INPUT;
			}
			break;
		case NLM_DUMP_BLOCKS:
			if (userInputs->m_endRange >= NLMDEV_NUM_ARRAY_BLOCKS)
			{
				NlmCm__printf("\n\tError : end_range > Number of Blocks(0-127) \n");
				errorFlag = NLMDIAG_ERROR_INVALID_INPUT;
			}
			if (userInputs->m_startRange > userInputs->m_endRange)
			{
				NlmCm__printf("\n\tError : start_range > end_range\n");
				errorFlag = NLMDIAG_ERROR_INVALID_INPUT;
			}
			break;
		case NLM_DUMP_CB_MEMORY:
			if (userInputs->m_endRange >= NLMDEV_CB_DEPTH)
			{
				NlmCm__printf("\n\tError : end_range > CB Depth(0-8191)\n");
				errorFlag = NLMDIAG_ERROR_INVALID_INPUT;
			}
			if (userInputs->m_startRange > userInputs->m_endRange)
			{
				NlmCm__printf("\n\tError : start_range > end_range\n");
				errorFlag = NLMDIAG_ERROR_INVALID_INPUT;
			}
			break;
	}

	return errorFlag;
}

nlm_u32 
NlmDiag_ReadAndDisplayLtrRegister (NlmDev *dev_p, 
									  nlm_u8 ltrNum, 
									  nlm_u8 ltrRegType, 
									  NlmCmFile *logFile_p)
{
	NlmDevBlkSelectReg       ltrBlkSelRegInfo;
    NlmDevSuperBlkKeyMapReg  ltrBlkKeyMapRegInfo;
    NlmDevParallelSrchReg	 ltrPrlSrchRegInfo;
    NlmDevRangeInsertion0Reg ltrRangeInst0_RegInfo;
    NlmDevRangeInsertion1Reg ltrRangeInst1_RegInfo;
    NlmDevMiscelleneousReg   ltrMiscRegInfo;
    NlmDevKeyConstructReg    ltrKeyConstructRegInfo;
    NlmDevSSReg              ltrSSRegReadInfo;
    
    nlm_u32 errNum;
    NlmReasonCode reasonCode;
    nlm_u32 regNum = 0;
	nlm_u32 numBlksSelected = 0;
            
    if(ltrRegType >= NLMDEV_LTR_REG_END
        || ltrNum >= NLMDEV_NUM_LTR_SET)
        return NLMDIAG_ERROR_INVALID_INPUT;

    /* Check for Ltr Register type and read the data 
      *  according to specified register fields */
    switch(ltrRegType)
    {
        case NLMDEV_BLOCK_SELECT_0_LTR:
			regNum = 0;
			/* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrNum, ltrRegType,(void*)&ltrBlkSelRegInfo,
				&reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(logFile_p, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
			}
			else
			{
				NlmCmFile__fprintf(logFile_p, "\nList of Enabled Blocks = ");
				numBlksSelected = NlmDiag_pvt_DisplayBlkSelectReg(&ltrBlkSelRegInfo,regNum, 
logFile_p);
				if(numBlksSelected == 0)
					NlmCmFile__fprintf(logFile_p, "None\n");
			}
            break;
        case NLMDEV_BLOCK_SELECT_1_LTR:
			regNum = 1;
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrNum, ltrRegType,(void*)&ltrBlkSelRegInfo,
				&reasonCode)) != NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(logFile_p, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
			}
			else
			{
				NlmCmFile__fprintf(logFile_p, "\nList of Enabled Blocks = ");
				numBlksSelected = NlmDiag_pvt_DisplayBlkSelectReg(&ltrBlkSelRegInfo,regNum, 
logFile_p);
				if(numBlksSelected == 0)
					NlmCmFile__fprintf(logFile_p, "None\n");
			}
            break;

        case NLMDEV_SUPER_BLK_KEY_MAP_LTR:
           /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrNum, ltrRegType,(void*)&ltrBlkKeyMapRegInfo,
				&reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile_p, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
            }
			else
			{
				NlmDiag_pvt_DisplaySuperBlkKeyMap(&ltrBlkKeyMapRegInfo, logFile_p);

			}
            break;
    
       case NLMDEV_PARALLEL_SEARCH_0_LTR:
		   regNum = 0;
		   /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrNum, ltrRegType,(void*)&ltrPrlSrchRegInfo,
				&reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile_p, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
            }
			else
			{
				NlmCmFile__fprintf(logFile_p, "\n\nBlock -> PS Mapping  ");
				NlmDiag_pvt_DisplayParallelSrchReg(&ltrPrlSrchRegInfo, regNum, logFile_p);
			}
            break;
       case NLMDEV_PARALLEL_SEARCH_1_LTR:
		   regNum = 1;
		   /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrNum, ltrRegType,(void*)&ltrPrlSrchRegInfo,
				&reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile_p, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
            }
			else
			{
				NlmCmFile__fprintf(logFile_p, "\n\nBlock -> PS Mapping  ");
				NlmDiag_pvt_DisplayParallelSrchReg(&ltrPrlSrchRegInfo, regNum, logFile_p);
			}
            break;
       case NLMDEV_PARALLEL_SEARCH_2_LTR:
		   regNum = 2;
		   /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrNum, ltrRegType,(void*)&ltrPrlSrchRegInfo,
				&reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile_p, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
            }
			else
			{
				NlmCmFile__fprintf(logFile_p, "\n\nBlock -> PS Mapping  ");
				NlmDiag_pvt_DisplayParallelSrchReg(&ltrPrlSrchRegInfo, regNum, logFile_p);
			}
            break;
       case NLMDEV_PARALLEL_SEARCH_3_LTR:
		   regNum =3;
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrNum, ltrRegType,(void*)&ltrPrlSrchRegInfo,
				&reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile_p, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
            }
			else
			{
				NlmCmFile__fprintf(logFile_p, "\n\nBlock -> PS Mapping  ");
				NlmDiag_pvt_DisplayParallelSrchReg(&ltrPrlSrchRegInfo, regNum, logFile_p);
			}
            break;

        case NLMDEV_RANGE_INSERTION_0_LTR:
           /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrNum, ltrRegType, (void*)&ltrRangeInst0_RegInfo,
				&reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile_p, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
            }
			else
			{
				/*Print the RangeInsert0 Register */
				NlmDiag_pvt_DisplayRangeInsert0Reg(&ltrRangeInst0_RegInfo, logFile_p);
			}
            break;

        case NLMDEV_RANGE_INSERTION_1_LTR:
           /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrNum, ltrRegType,(void*)&ltrRangeInst1_RegInfo,
				&reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile_p, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
            }
			else
			{
				NlmDiag_pvt_DisplayRangeInsert1Reg(&ltrRangeInst1_RegInfo, logFile_p);
			}
            break;

        case NLMDEV_MISCELLENEOUS_LTR:
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrNum, ltrRegType,(void*)&ltrMiscRegInfo,
				&reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile_p, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
            }
			else
			{
				NlmDiag_pvt_DisplayMiscReg(&ltrMiscRegInfo, logFile_p);
			}
            break;

       case NLMDEV_KEY_0_KCR_0_LTR:
       case NLMDEV_KEY_0_KCR_1_LTR:
       case NLMDEV_KEY_1_KCR_0_LTR:
       case NLMDEV_KEY_1_KCR_1_LTR:
       case NLMDEV_KEY_2_KCR_0_LTR:
       case NLMDEV_KEY_2_KCR_1_LTR:
       case NLMDEV_KEY_3_KCR_0_LTR:
       case NLMDEV_KEY_3_KCR_1_LTR:
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrNum, ltrRegType, (void*)&ltrKeyConstructRegInfo,
				&reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile_p, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
            }
			else
			{
				NlmCmFile__fprintf(logFile_p, "\n KCR \n");
				NlmDiag_pvt_DisplayKCR(&ltrKeyConstructRegInfo, logFile_p);
			}
            break;

        case NLMDEV_SS_LTR:
            /* Invoke the device manager API to read the specified LTR */
            if((errNum = NlmDevMgr__LogicalTableRegisterRead(dev_p,
                ltrNum, ltrRegType, (void*)&ltrSSRegReadInfo,
				&reasonCode)) != NLMDIAG_ERROR_NOERROR)
            {
                NlmCmFile__fprintf(logFile_p, "Err:%d, Dev Mgr Fn: NlmDevMgr__LogicalTableRegisterRead",reasonCode);
            }
			else
			{
				NlmDiag_pvt_DisplaySSReg(&ltrSSRegReadInfo,logFile_p);
			}
            break;

        case NLMDEV_LTR_REG_END:
            break;
    }
	return NLMDIAG_ERROR_NOERROR;
}
nlm_u32 
NlmDiag_ReadAndDisplayBlockEntries (NlmDev *dev_p, 
									  nlm_u8 blkNum, 
									  NlmCmFile *logFile_p)
{
	NlmDevABEntry readEntry;
	nlm_u16 readAddress, validEntry = 0;
	NlmReasonCode reasonCode;
	nlm_u32 errNum = 0;

	for(readAddress=0; readAddress < NLMDEV_AB_DEPTH; readAddress++)
	 { 
		/* Invoke the device manager API to read the specified AB entry */
		if((errNum = NlmDevMgr__ABEntryRead(dev_p, blkNum,
			readAddress, &readEntry, &reasonCode))
			!= NLMDIAG_ERROR_NOERROR)
		{
			NlmCmFile__fprintf(logFile_p, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__ABEntryRead",reasonCode);
		}
		if (readEntry.m_vbit)
		{
			NlmCmFile__fprintf(logFile_p, "\n\t%5x : ", readAddress);
			NlmDiag_PrintHexDataToFile(readEntry.m_data, NLMDEV_REG_LEN_IN_BYTES, logFile_p);
			NlmCmFile__fprintf(logFile_p, "\t");
			NlmDiag_PrintHexDataToFile(readEntry.m_mask, NLMDEV_REG_LEN_IN_BYTES, logFile_p);
			validEntry = 1;
		}
	 }
	 if (!validEntry)
		NlmCmFile__fprintf(logFile_p, "\n\t No Valid Entries");
	 return NLMDIAG_ERROR_NOERROR;
}

nlm_u32 
NlmDiag_ReadAndDisplayBlockRegisters (NlmDev *dev_p, 
									  nlm_u8 blkNum, 
									  NlmCmFile *logFile_p, 
									  nlm_u8 *blkEnabled)
{
	nlm_u32 readAddress, index = 0, value = 0;
	NlmReasonCode reasonCode;
	nlm_u32 errNum = 0;
	NlmDevBlockConfigReg blkConfigRegReadInfo;
    NlmDevBlockMaskReg blkMaskRegReadInfo;
	nlm_u8	readData[NLMDEV_REG_LEN_IN_BYTES] = "";		/* 80 bit hex value */

	readAddress = (0x01000 + (blkNum * 0x20));
	if((errNum = NlmDevMgr__BlockRegisterRead(dev_p, blkNum,NLMDEV_BLOCK_CONFIG_REG, 
			&blkConfigRegReadInfo, &reasonCode))
			!= NLMDIAG_ERROR_NOERROR)
	{
		NlmCmFile__fprintf(logFile_p, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__ABEntryRead",reasonCode);
	}
	value = (blkConfigRegReadInfo.m_blockWidth << 1) | blkConfigRegReadInfo.m_blockEnable;
	WriteBitsInArray(readData,NLMDEV_REG_LEN_IN_BYTES, 3, 0, value);

	if (blkConfigRegReadInfo.m_blockEnable)
	{
		*blkEnabled = 1;
		NlmCmFile__fprintf(logFile_p, "\nRegisters (BCR & BMR)\n");
		NlmCmFile__fprintf(logFile_p, "\n\t%5x : ", readAddress);
		NlmDiag_PrintHexDataToFile(readData, NLMDEV_REG_LEN_IN_BYTES, logFile_p);
				

		for(index=1; index < 21 ; index++)
		{ 
			readAddress = ((0x01000 + index) + (blkNum * 0x20));
			/* Invoke the device manager API to read the specified AB entry */
			if((errNum = NlmDevMgr__BlockRegisterRead(dev_p, blkNum, index, 
				&blkMaskRegReadInfo, &reasonCode))
				!= NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(logFile_p, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__ABEntryRead",reasonCode);
			}
			NlmCmFile__fprintf(logFile_p, "\n\t%5x : ", readAddress);
			NlmDiag_PrintHexDataToFile(blkMaskRegReadInfo.m_mask, NLMDEV_REG_LEN_IN_BYTES, logFile_p);
		}
	}
	if (!blkConfigRegReadInfo.m_blockEnable)
	{
	NlmCmFile__fprintf(logFile_p, "\n\n\tBlcok Disabled \n");
	}
	return NLMDIAG_ERROR_NOERROR;
}

nlm_u32 
NlmDiag_ReadAndDisplayRangeRegisters (NlmDev *dev_p, 
									  NlmCmFile *logFile_p)
{
	NlmDevRangeReg rangeReadData;
	nlm_u32 readAddress;
	NlmReasonCode reasonCode;
	nlm_u32 errNum = 0;

	for(readAddress=NLMDIAG_RANGE_START_ADDRESS; readAddress <= NLMDIAG_RANGE_REGISTER_COUNT; readAddress++)
	 { 
		/* Invoke the device manager API to read the specified AB entry */
		if((errNum = NlmDevMgr__RangeRegisterRead(dev_p, readAddress,
			&rangeReadData, &reasonCode))
			!= NLMDIAG_ERROR_NOERROR)
		{
			NlmCmFile__fprintf(logFile_p, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__ABEntryRead",reasonCode);
		}
		NlmCmFile__fprintf(logFile_p, "\n\t%5x : ", readAddress);
		NlmDiag_PrintHexDataToFile(rangeReadData.m_data, NLMDEV_REG_LEN_IN_BYTES, logFile_p);
	 }
	 return NLMDIAG_ERROR_NOERROR;
}


nlm_u32 
NlmDiag_ReadAndDisplayGlobalRegister(
	NlmDev *dev_p,
	NlmCmFile *logFile_p
	)
{
	/* Read Only Regs */
    NlmDevIdReg devIdRegReadInfo;
    NlmDevErrStatusReg devErrStatRegReadInfo;
    NlmDevAdvancedSoftErrReg devAdvSoftErrRegReadInfo;
    NlmDevResultReg devResultReg0ReadInfo;
    NlmDevResultReg devResultReg1ReadInfo;

    /* Write/Read regs */
    NlmDevConfigReg devConfigRegReadInfo;
    NlmDevErrStatusReg devErrStatMaskRegReadInfo; /* same structure used to mask the errors */
    NlmDevDbSoftErrFifoReg devDBSoftErrStatRegReadInfo;
    NlmDevScratchPadReg scratchPadRegReadInfo;

	NlmReasonCode reasonCode;
    nlm_u32 errNum;
       
    /* Check for Global Register type and construct the pattern of data 
     *  according to specified register fields */
    /* call the device manager API to read contents from the Device ID Reg */
	if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, NLMDEV_DEVICE_ID_REG, 
        (void*)&devIdRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
	{
		NlmCmFile__fprintf(logFile_p, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
	}
	else
	{
		NlmCmFile__fprintf(logFile_p, "\n\t%05x : ", NLM_REG_ADDR_DEVICE_ID);
		NlmDiag_pvt_DisplayDevIdReg (&devIdRegReadInfo, logFile_p);
	}
	/* Device Config Reg */
    if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, NLMDEV_DEVICE_CONFIG_REG,
        (void*)&devConfigRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(logFile_p, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
    }
	else
	{
		NlmCmFile__fprintf(logFile_p, "\n\t%05x : ", NLM_REG_ADDR_DEVICE_CONFIG);
		NlmDiag_pvt_DisplayDevConfigReg (&devConfigRegReadInfo,logFile_p);
	}
    if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, NLMDEV_ERROR_STATUS_REG,
        (void*)&devErrStatRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(logFile_p, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
    }
	else
	{
		NlmCmFile__fprintf(logFile_p, "\n\t%05x : ", NLM_REG_ADDR_ERROR_STATUS);
		NlmDiag_pvt_DisplayDevErrStatusReg (&devErrStatRegReadInfo, logFile_p);
	}
	if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, NLMDEV_ERROR_STATUS_MASK_REG,
        (void*)&devErrStatMaskRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(logFile_p, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
    }
	else
	{
		NlmCmFile__fprintf(logFile_p, "\n\t%05x : ", NLM_REG_ADDR_ERROR_STATUS_MASK);
		NlmDiag_pvt_DisplayDevErrStatusMaskReg (&devErrStatMaskRegReadInfo, logFile_p);
	}
	if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, NLMDEV_DATABASE_SOFT_ERROR_FIFO_REG,
        (void*)&devDBSoftErrStatRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(logFile_p, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
    }
	else
	{
		NlmCmFile__fprintf(logFile_p, "\n\t%05x : ", NLM_REG_ADDR_PARITY_ERROR_FIFO);
		NlmDiag_pvt_DisplayDevDbSoftErrFifoReg (&devDBSoftErrStatRegReadInfo, logFile_p);
	}
    if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, NLMDEV_ADVANCED_FEATURES_SOFT_ERROR_REG,
        (void*)&devAdvSoftErrRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(logFile_p, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
    }
	else
	{
		NlmCmFile__fprintf(logFile_p, "\n\t%05x : ", NLM_REG_ADDR_DETAILED_PARITY_ERROR_INFO);
		NlmDiag_pvt_DisplayDevAdvancedSoftErrReg (&devAdvSoftErrRegReadInfo, logFile_p);
	}
	/* call the device manager API to read contents from the specified scratch pad */
    if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, NLMDEV_SCRATCH_PAD0_REG, 
        (void*)&scratchPadRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(logFile_p, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
    }
	else
	{	
		NlmCmFile__fprintf(logFile_p, "\n\t%05x : ", NLM_REG_ADDR_SCRATCH_PAD0);
		NlmDiag_pvt_DisplayDevScratchPad0Reg (&scratchPadRegReadInfo, logFile_p);
	}
    /* call the device manager API to read contents from the specified scratch pad */
    if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, NLMDEV_SCRATCH_PAD1_REG,
        (void*)&scratchPadRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(logFile_p, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
    }
	else
	{
		NlmCmFile__fprintf(logFile_p, "\n\t%05x : ", NLM_REG_ADDR_SCRATCH_PAD1);
		NlmDiag_pvt_DisplayDevScratchPad1Reg (&scratchPadRegReadInfo, logFile_p);
	}
    if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, NLMDEV_RESULT0_REG,
        (void*)&devResultReg0ReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(logFile_p, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
    }
	else
	{
		NlmCmFile__fprintf(logFile_p, "\n\t%05x : ", NLM_REG_ADDR_RESULT0);
		NlmDiag_pvt_DisplayDevResult0Reg (&devResultReg0ReadInfo, logFile_p);
	}
	if((errNum = NlmDevMgr__GlobalRegisterRead(dev_p, NLMDEV_RESULT1_REG,
        (void*)&devResultReg1ReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
    {
        NlmCmFile__fprintf(logFile_p, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__GlobalRegisterWrite",reasonCode);
    }
	else
	{
		NlmCmFile__fprintf(logFile_p, "\n\t%05x : ", NLM_REG_ADDR_RESULT1);
		NlmDiag_pvt_DisplayDevResult1Reg (&devResultReg1ReadInfo, logFile_p);
	}
	return NLMDIAG_ERROR_NOERROR;
}

nlm_u32 
NlmDiag_ReadAndDisplayCBMemory (
	NlmDev *dev_p,
	nlm_u16 startAddress, 
	NlmCmFile *logFile_p
	)
{
	NlmDevCtxBufferReg CBRegReadInfo;
	NlmReasonCode reasonCode;
    nlm_u32 errNum;
    
	if((errNum = NlmDevMgr__CBAsRegisterRead(dev_p, startAddress, 
										&CBRegReadInfo, &reasonCode)) != NLMDIAG_ERROR_NOERROR)
	{
		NlmCmFile__fprintf(logFile_p, "\n\tErr:%d, Dev Mgr Fn: NlmDevMgr__CBAsRegisterRead",reasonCode);
	}
	else
	{
		NlmCmFile__fprintf(logFile_p, "\n\t%05x : ", startAddress);
		NlmDiag_PrintHexDataToFile(CBRegReadInfo.m_data, NLMDEV_REG_LEN_IN_BYTES, logFile_p);
	}
    return NLMDIAG_ERROR_NOERROR;  
}

nlm_u32 
NlmDiag_ReadAndDisplaySTEntries (
	NlmDev *dev_p,
	NlmCmFile *logFile_p)
{
    NlmDevSTE stRegReadInfo;
    nlm_u16 i;   
    NlmErrNum_t errNum;
	NlmReasonCode reasonCode;
	nlm_u8 stNum = 0;
	nlm_u8  stData[NLMDEV_REG_LEN_IN_BYTES * 2];
	NlmCm__memset(stData, 0x00, (NLMDEV_REG_LEN_IN_BYTES * 2));
    
	for(stNum = 0; stNum < NLMDEV_SS_NUM; stNum++)
    {
		NlmCmFile__fprintf(logFile_p, "\n\n---- : BLOCK %d : ----", stNum);
		for(i = 0; i <NLMDEV_SS_SEP ; i++)
		{
			if(stNum == 0 && i == 0x0c) /* Skip 2 addresses here */
				continue;

			/* Invoke device manager API to read specified st entry */
			if((errNum = NlmDevMgr__STR(dev_p, 
				stNum, i, 
				&stRegReadInfo, &reasonCode)) !=   NLMDIAG_ERROR_NOERROR )
			{
				NlmCmFile__fprintf(logFile_p, "\n\t Err:%d, Dev Mgr Fn: NlmDevMgr__STR",reasonCode);
			}
			else
			{
				NlmCmFile__fprintf(logFile_p, "\n\t%5x : ", i);
				NlmDiag_pvt_DisplaySTInfo (&stRegReadInfo, logFile_p);				
			}
		}
    }
    return NLMDIAG_ERROR_NOERROR;
}

nlm_u32
NlmDiag_DisplayDevice (NlmDiag_TestInfo *testInfo_p, 
						  NlmDiag_DevDumpOptions *devDumpOptions_p,
						  nlm_u8 devNum, 
						  NlmCmFile *logFile_p)
{
	NlmDiag_ModuleInfo *moduleInfo_p = NULL;
	nlm_u8 chnlNum = 0, ltr, regNum, blk, blkEnabled = 0;
	nlm_u32 errNum = 0;
	nlm_u16 startAddress;

	moduleInfo_p = (NlmDiag_ModuleInfo*)testInfo_p->m_testModuleInfo_p;    
	    
    if(moduleInfo_p->m_dev_ppp == NULL || moduleInfo_p->m_dev_ppp[chnlNum] == NULL) 
        return(NLMDIAG_ERROR_INVALID_INPUTPTR);
	if(moduleInfo_p->m_dev_ppp[chnlNum][devNum] == NULL)
			return(NLMDIAG_ERROR_INVALID_INPUTPTR);

	switch (devDumpOptions_p->m_operation)
	{
		case NLM_DUMP_COMPLETE_DEVICE:
			NlmCmFile__fprintf(logFile_p, "\n\n---- : Global Registers : ----");
			NlmCmFile__fprintf(logFile_p, "\n------------------------------");
		    /* Invoke the test function which perform Global register read */
            if((errNum = NlmDiag_ReadAndDisplayGlobalRegister(moduleInfo_p->m_dev_ppp[chnlNum][devNum], logFile_p)) !=NLMDIAG_ERROR_NOERROR)                 
                return(errNum);

			NlmCmFile__fprintf(logFile_p, "\n\n---- : Range Registers : ----");
			NlmCmFile__fprintf(logFile_p, "\n------------------------------");
		    /* Invoke the test function which perform Internal register read */
            if((errNum = NlmDiag_ReadAndDisplayRangeRegisters(moduleInfo_p->m_dev_ppp[chnlNum][devNum], logFile_p)) !=NLMDIAG_ERROR_NOERROR)                 
                return(errNum);
			if (testInfo_p->m_operMode == NLMDEV_OPR_SAHASRA)
			{
				NlmCmFile__fprintf(logFile_p, "\n\n---- : ST Entries : ----");
				NlmCmFile__fprintf(logFile_p, "\n-----------------------------");
				/* Invoke the test function which perform Sahasra ST entries read */
				if((errNum = NlmDiag_ReadAndDisplaySTEntries(moduleInfo_p->m_dev_ppp[chnlNum][devNum], logFile_p)) !=NLMDIAG_ERROR_NOERROR)                 
					return(errNum);
			}

			NlmCmFile__fprintf(logFile_p, "\n\n---- : CB Memory : ----");
			NlmCmFile__fprintf(logFile_p, "\n------------------------------");
		    for (startAddress = 0; startAddress < NLMDEV_CB_DEPTH ; startAddress++)
			{
				/* Invoke the test function which perform CB Entry read */
		        if((errNum = NlmDiag_ReadAndDisplayCBMemory(moduleInfo_p->m_dev_ppp[chnlNum][devNum], startAddress, logFile_p)) !=NLMDIAG_ERROR_NOERROR)                 
				    return(errNum);
			}
			
			NlmCmFile__fprintf(logFile_p, "\n---- LTR Register Info ----");
			for(ltr = 0; ltr < NLMDEV_NUM_LTR_SET; ltr++)
            {
				NlmCmFile__fprintf(logFile_p, "\n\n---- : LTR %d : ----", ltr);
				NlmCmFile__fprintf(logFile_p, "\n---------------------");
                for(regNum = 0; regNum < NLMDEV_LTR_REG_END; regNum++)
                {
					if (regNum == 0xa)
						continue;
				    if((errNum = NlmDiag_ReadAndDisplayLtrRegister(moduleInfo_p->m_dev_ppp[chnlNum][devNum], ltr, regNum, logFile_p)) !=NLMDIAG_ERROR_NOERROR)
                        return(errNum);             
                }
            }
			
			NlmCmFile__fprintf(logFile_p, "\n\n---- Block Info ----");
			for(blk = 0; blk < NLMDEV_NUM_ARRAY_BLOCKS; blk++)
            {
				blkEnabled = 0;
				NlmCmFile__fprintf(logFile_p, "\n\n---- : BLOCK %d : ----", blk);
				NlmCmFile__fprintf(logFile_p, "\n---------------------");
				                
                if((errNum = NlmDiag_ReadAndDisplayBlockRegisters(moduleInfo_p->m_dev_ppp[chnlNum][devNum], blk, logFile_p, &blkEnabled)) !=NLMDIAG_ERROR_NOERROR)
                    return(errNum);
				NlmCmFile__fprintf(logFile_p, "\n");
				if (blkEnabled)
				{
					NlmCmFile__fprintf(logFile_p, "\nDatabase Entries (Data , Mask Pair)\n");
					/* Invoke the test function which perform AB Entry read */
					if((errNum = NlmDiag_ReadAndDisplayBlockEntries(moduleInfo_p->m_dev_ppp[chnlNum][devNum], blk, logFile_p)) !=NLMDIAG_ERROR_NOERROR)                 
						return(errNum);
				}
            }
			break;
		case NLM_DUMP_LTR_REGISTERS:
			NlmCmFile__fprintf(logFile_p, "\n---- LTR Register Info ----");
			for(ltr = (nlm_u8)devDumpOptions_p->m_startRange; ltr <= devDumpOptions_p->m_endRange; ltr++)
            {
				NlmCmFile__fprintf(logFile_p, "\n\n---- : LTR %d : ----", ltr);
				NlmCmFile__fprintf(logFile_p, "\n----------------------");
                for(regNum = 0; regNum < NLMDEV_LTR_REG_END; regNum++)
                {
					if (regNum == 0xa)
						continue;
					if((errNum = NlmDiag_ReadAndDisplayLtrRegister(moduleInfo_p->m_dev_ppp[chnlNum][devNum], ltr, regNum, logFile_p)) !=NLMDIAG_ERROR_NOERROR)
                        return(errNum);             
                }
            }
			break;
		case NLM_DUMP_BLOCKS:
			NlmCmFile__fprintf(logFile_p, "\n\n---- Block Info ----");
			for(blk = (nlm_u8)devDumpOptions_p->m_startRange; blk <= devDumpOptions_p->m_endRange; blk++)
            {
				blkEnabled = 0;
				NlmCmFile__fprintf(logFile_p, "\n\n---- : BLOCK %d : ----", blk);
				NlmCmFile__fprintf(logFile_p, "\n-----------------------");
				if((errNum = NlmDiag_ReadAndDisplayBlockRegisters(moduleInfo_p->m_dev_ppp[chnlNum][devNum], blk, logFile_p, &blkEnabled)) !=NLMDIAG_ERROR_NOERROR)                 
                    return(errNum);
				if (blkEnabled)
				{
					/* Invoke the test function which perform AB Entry read */
					NlmCmFile__fprintf(logFile_p, "\n");
					NlmCmFile__fprintf(logFile_p, "\nDatabase Entries(Data , Mask Pair)\n");
					if((errNum = NlmDiag_ReadAndDisplayBlockEntries(moduleInfo_p->m_dev_ppp[chnlNum][devNum], blk, logFile_p)) !=NLMDIAG_ERROR_NOERROR)
						return(errNum);
				}
            }
			break;
		case NLM_DUMP_INTERNAL_REGISTERS:
			NlmCmFile__fprintf(logFile_p, "\n\n---- : Range Registers : ----");
			NlmCmFile__fprintf(logFile_p, "\n-----------------------------");
		    /* Invoke the test function which perform Internal Register read */
            if((errNum = NlmDiag_ReadAndDisplayRangeRegisters(moduleInfo_p->m_dev_ppp[chnlNum][devNum], logFile_p)) !=NLMDIAG_ERROR_NOERROR)                 
                return(errNum);

			break;
		case NLM_DUMP_GLOBAL_REGISTER:
			NlmCmFile__fprintf(logFile_p, "\n\n---- : Global Registers : ----");
			NlmCmFile__fprintf(logFile_p, "\n------------------------------");
		    /* Invoke the test function which perform Global Register read */
            if((errNum = NlmDiag_ReadAndDisplayGlobalRegister(moduleInfo_p->m_dev_ppp[chnlNum][devNum], logFile_p)) !=NLMDIAG_ERROR_NOERROR)                 
                return(errNum);
			break;
		case NLM_DUMP_CB_MEMORY:
			NlmCmFile__fprintf(logFile_p, "\n\n---- : CB Memory : ----");
			NlmCmFile__fprintf(logFile_p, "\n------------------------------");
		    for (startAddress = devDumpOptions_p->m_startRange; startAddress < devDumpOptions_p->m_endRange ; startAddress++)
			{
				/* Invoke the test function which perform CB Entry read */
		        if((errNum = NlmDiag_ReadAndDisplayCBMemory(moduleInfo_p->m_dev_ppp[chnlNum][devNum], startAddress, logFile_p)) !=NLMDIAG_ERROR_NOERROR)                 
				    return(errNum);
			}
			break;
		case NLM_DUMP_SAHASRA_MEMORY:
			if (testInfo_p->m_operMode == NLMDEV_OPR_SAHASRA)
			{
				NlmCmFile__fprintf(logFile_p, "\n\n---- : ST Entries : ----");
				NlmCmFile__fprintf(logFile_p, "\n-----------------------------");
				/* Invoke the test function which perform ST Entry read */
				if((errNum = NlmDiag_ReadAndDisplaySTEntries(moduleInfo_p->m_dev_ppp[chnlNum][devNum], logFile_p)) !=NLMDIAG_ERROR_NOERROR)                 
					return(errNum);
				NlmCmFile__fprintf(logFile_p, "\n\n---- : BLOCK Entries : ----");
				NlmCmFile__fprintf(logFile_p, "\n-----------------------------");

				for(blk = 120; blk < NLMDEV_NUM_ARRAY_BLOCKS; blk++)
				{
					NlmCmFile__fprintf(logFile_p, "\n\n---- : BLOCK %d : ----", blk);
					NlmCmFile__fprintf(logFile_p, "\n-----------------------");
					
					if((errNum = NlmDiag_ReadAndDisplayBlockRegisters(moduleInfo_p->m_dev_ppp[chnlNum][devNum], blk, logFile_p, &blkEnabled)) !=NLMDIAG_ERROR_NOERROR)                 
						return(errNum);
					if (blkEnabled)
					{
						NlmCmFile__fprintf(logFile_p, "\n");
						NlmCmFile__fprintf(logFile_p, "\nDatabase Entries(Data , Mask Pair)\n");
						if((errNum = NlmDiag_ReadAndDisplayBlockEntries(moduleInfo_p->m_dev_ppp[chnlNum][devNum], blk, logFile_p)) !=NLMDIAG_ERROR_NOERROR)                 
							return(errNum);
					}
				}
			}
			else
			{
				NlmCm__printf("\n\n Device is Running Non-Sahasra mode\n");
				return NLMDIAG_ERROR_INVALID_INPUT;
			}

			break;
	}
	return NLMDIAG_ERROR_NOERROR;
}

nlm_u32
NlmDiag_DumpDevice (void *alloc_p, 
					   NlmDiag_TestInfo *testInfo_p,
					   NlmDiag_TestCaseInfo *testCaseInfo_p,
					   nlm_8 *diagLogfile
					  )
{
	nlm_8 fileNum[100] = {0};
	nlm_8 fileName[100], i;
	NlmCmFile *outPutFile_p = NULL;
	nlm_u32 errNum = 0, errorCount = 0;
	
	NlmDiag_CreateLogFileDetails("dump_device",testInfo_p, diagLogfile);
	if((errNum = NlmDiag_CommonInitTestInfo(alloc_p, testInfo_p, testCaseInfo_p)) !=0)
	{
		errorCount += errNum;
		NlmCm__printf("\n\n\n\tGlobal Register Read is Failed (Memory Error).\n");
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tDiagnostic Memory test is Failed (Memory Error).\n");
	}
	if((errNum = NlmDiag_Initialize(testInfo_p)) !=NLMDIAG_ERROR_NOERROR)
		errorCount += errNum;

	if (testCaseInfo_p->m_devDumpOptions.m_operation == NLM_DUMP_COMPLETE_DEVICE)
	{
		for(i = testCaseInfo_p->m_devDumpOptions.m_startDevId; i <= testCaseInfo_p->m_devDumpOptions.m_endDevId; ++i)
 		{
			NlmCm__strcpy(fileName, "Nlm_Device_");
 			NlmCm__sprintf(fileNum, "%d", i);
  			NlmCm__strcat(fileName, fileNum);
 			NlmCm__strcat(fileName, "_complete");
 			NlmCm__strcat(fileName, ".txt");
#ifndef NLMPLATFORM_BCM			
			outPutFile_p = NlmCmFile__fopen(fileName, "w");
			if(outPutFile_p == NULL)
			{
				NlmCm__printf("\n\tUnable to open the dump file %s .\n", fileName);
				exit(0);
			}
#endif			
			NlmCmFile__fprintf(outPutFile_p, " ---- Device Id %d ----", i);

			if((errNum = NlmDiag_DisplayDevice(testInfo_p,&testCaseInfo_p->m_devDumpOptions, i, outPutFile_p)) 
				!= NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr: NlmDiag_DisplayDevice Failed");
			}
#ifndef NLMPLATFORM_BCM			
			NlmCmFile__fclose (outPutFile_p);
#endif			
			if (!errNum)
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Device Dump OutputFile : %s \n", fileName);
			NlmCm__strcpy(fileName, "");
 		}
	}
	else if (testCaseInfo_p->m_devDumpOptions.m_operation == NLM_DUMP_LTR_REGISTERS)
	{
		for(i = testCaseInfo_p->m_devDumpOptions.m_startDevId; i <= testCaseInfo_p->m_devDumpOptions.m_endDevId; ++i)
 		{
			NlmCm__strcpy(fileName, "Nlm_Device_");
 			NlmCm__sprintf(fileNum, "%d", i);
  			NlmCm__strcat(fileName, fileNum);
 			NlmCm__strcat(fileName, "_ltr_");
			NlmCm__sprintf(fileNum, "%d", testCaseInfo_p->m_devDumpOptions.m_startRange);
 			NlmCm__strcat(fileName, fileNum);
 			NlmCm__strcat(fileName, "_");
			NlmCm__sprintf(fileNum, "%d", testCaseInfo_p->m_devDumpOptions.m_endRange);
 			NlmCm__strcat(fileName, fileNum);
 			NlmCm__strcat(fileName, ".txt");
#ifndef NLMPLATFORM_BCM				
			outPutFile_p = NlmCmFile__fopen(fileName, "w");
			if(outPutFile_p == NULL)
			{
				NlmCm__printf("\n\tUnable to open the dump file %s .\n", fileName);
				exit(0);
			}
#endif			
			NlmCmFile__fprintf(outPutFile_p, "---- Device Id %d ----", i);
			if((errNum = NlmDiag_DisplayDevice(testInfo_p,&testCaseInfo_p->m_devDumpOptions, i, outPutFile_p)) 
				!= NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr: NlmDiag_DisplayDevice Failed");
			}
#ifndef NLMPLATFORM_BCM			
			NlmCmFile__fclose (outPutFile_p);
#endif			
			if (!errNum)
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Device Dump OutputFile : %s \n", fileName);
			NlmCm__strcpy(fileName, "");
 		}
	}
	else if (testCaseInfo_p->m_devDumpOptions.m_operation == NLM_DUMP_BLOCKS)
	{
		for(i = testCaseInfo_p->m_devDumpOptions.m_startDevId; i <= testCaseInfo_p->m_devDumpOptions.m_endDevId; ++i)
 		{
			NlmCm__strcpy(fileName, "Nlm_Device_");
 			NlmCm__sprintf(fileNum, "%d", i);
  			NlmCm__strcat(fileName, fileNum);
 			NlmCm__strcat(fileName, "_blk_");
			NlmCm__sprintf(fileNum, "%d", testCaseInfo_p->m_devDumpOptions.m_startRange);
 			NlmCm__strcat(fileName, fileNum);
 			NlmCm__strcat(fileName, "_");
			NlmCm__sprintf(fileNum, "%d", testCaseInfo_p->m_devDumpOptions.m_endRange);
 			NlmCm__strcat(fileName, fileNum);
 			NlmCm__strcat(fileName, ".txt");
#ifndef NLMPLATFORM_BCM
			outPutFile_p = NlmCmFile__fopen(fileName, "w");
			if(outPutFile_p == NULL)
			{
				NlmCm__printf("\n\tUnable to open the dump file %s .\n", fileName);
				exit(0);
			}
#endif			
			NlmCmFile__fprintf(outPutFile_p, "---- Device Id %d ----", i);
			if((errNum = NlmDiag_DisplayDevice(testInfo_p,&testCaseInfo_p->m_devDumpOptions, i, outPutFile_p)) 
				!= NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr: NlmDiag_DisplayDevice Failed");
			}
#ifndef NLMPLATFORM_BCM			
			NlmCmFile__fclose (outPutFile_p);
#endif			
			if (!errNum)
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Device Dump OutputFile : %s \n", fileName);
			NlmCm__strcpy(fileName, "");
 		}
	}
	else if (testCaseInfo_p->m_devDumpOptions.m_operation == NLM_DUMP_INTERNAL_REGISTERS)
	{
		for(i = testCaseInfo_p->m_devDumpOptions.m_startDevId; i <= testCaseInfo_p->m_devDumpOptions.m_endDevId; ++i)
 		{
			NlmCm__strcpy(fileName, "Nlm_Device_");
			NlmCm__sprintf(fileNum, "%d", i);
  			NlmCm__strcat(fileName, fileNum);
 			NlmCm__strcat(fileName, "_internal_register");
			NlmCm__strcat(fileName, ".txt");
#ifndef NLMPLATFORM_BCM
			outPutFile_p = NlmCmFile__fopen(fileName, "w");
			if(outPutFile_p == NULL)
			{
				NlmCm__printf("\n\tUnable to open the dump file %s .\n", fileName);
				exit(0);
			}
#endif			
			NlmCmFile__fprintf(outPutFile_p, "---- Device Id %d ----", i);
			if((errNum = NlmDiag_DisplayDevice(testInfo_p,&testCaseInfo_p->m_devDumpOptions, i, outPutFile_p)) 
				!= NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr: NlmDiag_DisplayDevice Failed");
				return(errNum);
			}
#ifndef NLMPLATFORM_BCM			
			NlmCmFile__fclose (outPutFile_p);
#endif			
			if (!errNum)
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Device Dump OutputFile : %s \n", fileName);
			NlmCm__strcpy(fileName, "");
		}
	}
	else if (testCaseInfo_p->m_devDumpOptions.m_operation == NLM_DUMP_GLOBAL_REGISTER)
	{
		for(i = testCaseInfo_p->m_devDumpOptions.m_startDevId; i <= testCaseInfo_p->m_devDumpOptions.m_endDevId; ++i)
 		{
			NlmCm__strcpy(fileName, "Nlm_Device_");
			NlmCm__sprintf(fileNum, "%d", i);
  			NlmCm__strcat(fileName, fileNum);
 			NlmCm__strcat(fileName, "_global_registers");
			NlmCm__strcat(fileName, ".txt");
#ifndef NLMPLATFORM_BCM
			outPutFile_p = NlmCmFile__fopen(fileName, "w");
			if(outPutFile_p == NULL)
			{
				NlmCm__printf("\n\tUnable to open the dump file %s .\n", fileName);
				exit(0);
			}
#endif			
			NlmCmFile__fprintf(outPutFile_p, "---- Device Id %d ----", i);
			if((errNum = NlmDiag_DisplayDevice(testInfo_p,&testCaseInfo_p->m_devDumpOptions, i, outPutFile_p)) 
				!= NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr: NlmDiag_DisplayDevice Failed");
			}
#ifndef NLMPLATFORM_BCM			
			NlmCmFile__fclose (outPutFile_p);
#endif			
			if (!errNum)
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Device Dump OutputFile : %s \n", fileName);
			NlmCm__strcpy(fileName, "");
		}
	}
	else if (testCaseInfo_p->m_devDumpOptions.m_operation == NLM_DUMP_CB_MEMORY)
	{
		for(i = testCaseInfo_p->m_devDumpOptions.m_startDevId; i <= testCaseInfo_p->m_devDumpOptions.m_endDevId; ++i)
 		{
			NlmCm__strcpy(fileName, "Nlm_Device_");
			NlmCm__sprintf(fileNum, "%d", i);
  			NlmCm__strcat(fileName, fileNum);
 			NlmCm__strcat(fileName, "_cb_memory_");
			NlmCm__sprintf(fileNum, "%d", testCaseInfo_p->m_devDumpOptions.m_startRange);
 			NlmCm__strcat(fileName, fileNum);
 			NlmCm__strcat(fileName, "_");
			NlmCm__sprintf(fileNum, "%d", testCaseInfo_p->m_devDumpOptions.m_endRange);
 			NlmCm__strcat(fileName, fileNum);
 			NlmCm__strcat(fileName, ".txt");
#ifndef NLMPLATFORM_BCM
			outPutFile_p = NlmCmFile__fopen(fileName, "w");
			if(outPutFile_p == NULL)
			{
				NlmCm__printf("\n\tUnable to open the dump file %s .\n", fileName);
				exit(0);
			}
#endif			
			NlmCmFile__fprintf(outPutFile_p, "---- Device Id %d ----", i);
			if((errNum = NlmDiag_DisplayDevice(testInfo_p,&testCaseInfo_p->m_devDumpOptions, i, outPutFile_p)) 
				!= NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr: NlmDiag_DisplayDevice Failed");
			}
#ifndef NLMPLATFORM_BCM			
			NlmCmFile__fclose (outPutFile_p);
#endif			
			if (!errNum)
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Device Dump OutputFile : %s \n", fileName);
			NlmCm__strcpy(fileName, "");
		}
	}
	else if (testCaseInfo_p->m_devDumpOptions.m_operation == NLM_DUMP_SAHASRA_MEMORY)
	{
		for(i = testCaseInfo_p->m_devDumpOptions.m_startDevId; i <= testCaseInfo_p->m_devDumpOptions.m_endDevId; ++i)
 		{
			NlmCm__strcpy(fileName, "Nlm_aDevice_");
			NlmCm__sprintf(fileNum, "%d", i);
  			NlmCm__strcat(fileName, fileNum);
 			NlmCm__strcat(fileName, "_sahasra_entries");
			NlmCm__strcat(fileName, ".txt");
#ifndef NLMPLATFORM_BCM
			outPutFile_p = NlmCmFile__fopen(fileName, "w");
			if(outPutFile_p == NULL)
			{
				NlmCm__printf("\n\tUnable to open the dump file %s .\n", fileName);
				exit(0);
			}
#endif			
			NlmCmFile__fprintf(outPutFile_p, "---- Device Id %d ----", i);
			if((errNum = NlmDiag_DisplayDevice(testInfo_p,&testCaseInfo_p->m_devDumpOptions, i, outPutFile_p)) 
				!= NLMDIAG_ERROR_NOERROR)
			{
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tErr: NlmDiag_DisplayDevice Failed");
			}
#ifndef NLMPLATFORM_BCM			
			NlmCmFile__fclose (outPutFile_p);
#endif			
			if (!errNum)
				NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\t Device Dump OutputFile : %s \n", fileName);
			NlmCm__strcpy(fileName, "");
		}
	}
		/* Call Destroy function which destroys Xpt, dev mgr and releases devices*/
	if((errNum = NlmDiag_Destroy(testInfo_p))!=NLMDIAG_ERROR_NOERROR)
		errorCount +=errNum;

	if (!errorCount)
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tDevice Dump Completed \n");
		
		/* Release testblkinfo after each test case */
	if((errNum = NlmDiag_CommonDestroyTestInfo(testInfo_p)) != 0)
	{
		NlmCm__printf("\n\tCan't destroy test information.\n");
		NlmCmFile__fprintf(testInfo_p->m_testLogFile, "\n\tCan't destroy test information.\n");
		errorCount +=errNum;
	} 
	return errorCount;
}
