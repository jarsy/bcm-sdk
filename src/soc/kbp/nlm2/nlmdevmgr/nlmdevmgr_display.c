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
 
#include "nlmdevmgr_display.h"

extern NlmBool g_is10MDev;

static NlmBool
Nlm11kDevMgr_pvt_IsABEntryValid(
	Nlm11kDevShadowAB *abInfo_p, 
	nlm_u16 rowNum, 
	nlm_u8 blkWidthIdx)
{
	NlmBool isEntryDeleted = NlmFalse;
	nlm_u32 verifyIdx = blkWidthIdx;
	nlm_u32 i = 0;
	nlm_u32 maxBlkWidthIdx = NLM11KDEV_MAX_AB_WIDTH_IN_BITS / NLM11KDEV_AB_WIDTH_IN_BITS;

	if(blkWidthIdx == maxBlkWidthIdx)
		verifyIdx = blkWidthIdx / 2;

	for(i = 0; i < verifyIdx; ++i)
	{
		if(!abInfo_p->m_abEntry[rowNum + i].m_vbit)
		{
			isEntryDeleted = NlmTrue;
			break;
		}
	}

	if(blkWidthIdx == maxBlkWidthIdx && isEntryDeleted)
	{
		isEntryDeleted = NlmFalse;
		for(; i < blkWidthIdx; ++i)
		{
			if(!abInfo_p->m_abEntry[rowNum + i].m_vbit)
			{
				isEntryDeleted = NlmTrue;
				break;
			}
		}

	}

	return !isEntryDeleted;

}




static void 
Nlm11kDevMgr_pvt_DisplayST(
	FILE* fp,
    Nlm11kDev *dev_p,
    nlm_u8 blkNum,
    nlm_u16 addr
    )
{
    Nlm11kDevSTE *stInfo_p;

    NlmCmAssert(dev_p != NULL, "Null Dev Ptr");
    NlmCmAssert(blkNum < NLM11KDEV_NUM_AC15_BLOCKS, "Invalid blk number");        
    NlmCmAssert(addr < NLM11KDEV_SS_SEP, "Invalid SS Row");        

	if(blkNum < NLM11KDEV_NUM_AC15_BLOCKS && addr < NLM11KDEV_SS_SEP)
	{
		stInfo_p = &(((Nlm11kDevShadowDevice*)dev_p->m_shadowDevice_p)->m_st[blkNum].m_row[addr]);
	    
   		NlmCmFile__fprintf(fp, "B: %d, BW: %d, BMN: %d, ", 
						stInfo_p->m_ss_abs,
						NLM11KDEV_AB_WIDTH_IN_BITS * (1 << stInfo_p->m_bw), 
						stInfo_p->m_ss_bsel);

		NlmCmFile__fprintf(fp, "BI: %d, SESST: %d, SID: %d, SLR: %d, ", 
						stInfo_p->m_bix, 
						stInfo_p->m_ssi_s, 
						stInfo_p->m_ssi, 
						stInfo_p->m_slr); 

		NlmCmFile__fprintf(fp, "LV: %d, LI: %d \n", 
						stInfo_p->m_lprv, 
						stInfo_p->m_lpri);
	}

	
}




static void
Nlm11kDevMgr_pvt_DisplayBMR(
		FILE *fp,
		Nlm11kDevShadowAB *abInfo_p,
		nlm_u16 bmrNum,
		nlm_u8 maxNumChunks)
{
	nlm_u32 i =0;
	nlm_u8 *mask = NULL;
	nlm_u32 numOf80BitChunk = 0;

	if(maxNumChunks > NLM11KDEV_NUM_OF_80B_SEGMENTS_PER_BMR)
		maxNumChunks = NLM11KDEV_NUM_OF_80B_SEGMENTS_PER_BMR;

	for(numOf80BitChunk = maxNumChunks; numOf80BitChunk > 0; numOf80BitChunk--)
	{
		mask = abInfo_p->m_bmr[bmrNum ] [numOf80BitChunk -1].m_mask ;

		NlmCmFile__fprintf(fp, "0x");

		for(i = 0; i < NLM11KDEV_AB_WIDTH_IN_BYTES; ++i)
		{
			if(i == NLM11KDEV_AB_WIDTH_IN_BYTES - 1 &&
				numOf80BitChunk == 1)
			{
				NlmCmFile__fprintf(fp, "%02x", *(mask+i));
			}
			else
			{
				NlmCmFile__fprintf(fp, "%02x_", *(mask + i));
			}
		}
		
	}
}


static void
Nlm11kDevMgr_pvt_DisplayABEntry(
	FILE* fp,
	Nlm11kDevShadowAB *abInfo_p,
	nlm_u8 blkNum,
	nlm_u8 blkWidthIdx,
	nlm_u16 rowNum,
	nlm_u16 bmrNum,
	NlmBool combineBMR,
	NlmBool isSES)
{
	nlm_u8 numOf80BitChunk;
	nlm_u8 *data;
    nlm_u8 *mask;
	nlm_32 bitNum = 0, byteNum = 0;
	nlm_u8 dataBit = 0, maskBit = 0;
	NlmBool skipRow = NlmFalse;

	(void) blkNum;
	
	NlmCmFile__fprintf(fp,"%d",  rowNum );

/*
	NlmCmFile__fprintf(fp,", Blk %d", blkNum);

	if(combineBMR)
		NlmCmFile__fprintf(fp, ", Col %d", bmrNum);
*/

	NlmCmFile__fprintf(fp, ": ");

	for(numOf80BitChunk = blkWidthIdx; numOf80BitChunk > 0; numOf80BitChunk--)
	{   
		data = abInfo_p->m_abEntry[rowNum + numOf80BitChunk - 1].m_data;

		mask = abInfo_p->m_abEntry[rowNum + numOf80BitChunk - 1].m_mask;

		for(byteNum = 0; byteNum < NLM11KDEV_AB_WIDTH_IN_BYTES; ++byteNum)
		{
			for(bitNum = 7; bitNum >= 0; --bitNum)
			{
				/*Get the corresponding bit in the BMR */
				nlm_u8 * bmrMask_p = abInfo_p->m_bmr[bmrNum ][numOf80BitChunk -1].m_mask;
				nlm_u8 bmrAllow =  (nlm_u8) (!(bmrMask_p[byteNum] & (1u << bitNum)));

				if(combineBMR && !bmrAllow)
					continue;
				
				dataBit = (nlm_u8)(data[byteNum] & (1u << bitNum));

				maskBit = (nlm_u8)(mask[byteNum] & (1u << bitNum));

				if( dataBit &&	!maskBit )
				{
					NlmCmFile__fprintf(fp, "1");
				}
				else if( !dataBit && maskBit )
				{
					NlmCmFile__fprintf(fp, "0");
				}
				else if (!dataBit && !maskBit)
				{
					if(isSES)
						NlmCmFile__fprintf(fp, "_");
					else
						NlmCmFile__fprintf(fp, "X");
				}
				else
				{
					if(!combineBMR)
					{
						NlmCmFile__fprintf(fp, "-");
					}
					else
					{
						/*
						If we are in Sahasra mode and printing the non-SS blks
						then if we get X=1, Y=1, then entire row is a miss. So we can
						skip printing the row
						*/
						skipRow = NlmTrue;
						break;
					}
				}

			} /*Finished processing all the bits in an entry */

			if(skipRow)
				break;

		} /*Finished processing all the bytes in an 80-b portion */

		if(skipRow)
			break;
	    
	} /*Finished processing the current entry */


}


static void 
Nlm11kDevMgr_pvt_ProcessST(
    Nlm11kDev *dev_p,
    nlm_u8 blkNum,
    NlmBool sahasraBMRConfigInfo[][NLM11KDEV_NUM_OF_BMRS_PER_BLK])
{
	
	nlm_u16 entryNum = 0;
    nlm_u16 rowNum = 0, stAddr = 0;
	Nlm11kDevSTE *stInfo_p = NULL;
	nlm_u16 stBlkNum = blkNum - (NLM11KDEV_NUM_ARRAY_BLOCKS - NLM11KDEV_NUM_AC15_BLOCKS);
    
    Nlm11kDevShadowAB *abInfo_p = &((Nlm11kDevShadowDevice*)dev_p->m_shadowDevice_p)->m_arrayBlock[blkNum];    
	
	nlm_u8 blkWidthIdx = (nlm_u8)(1 << abInfo_p->m_blkConfig.m_blockWidth); 
    
	nlm_u32 numOfEntries = NLM11KDEV_AB_DEPTH/blkWidthIdx;
	
	if(g_is10MDev)
		stBlkNum = blkNum - (NLM11KDEV_10M_NUM_ARRAY_BLOCKS - NLM11KDEV_NUM_AC15_BLOCKS);
	
	if(abInfo_p->m_blkConfig.m_blockEnable == 0)
		return;
    
	/*We don't support SS blocks that have a width of 80b */
	if(blkWidthIdx <= 1)
		return;


	for(entryNum = 0; entryNum < numOfEntries; entryNum++)
    {
		rowNum =  (entryNum * blkWidthIdx);  

		if(!Nlm11kDevMgr_pvt_IsABEntryValid(abInfo_p, rowNum, blkWidthIdx))
			continue;
   
		stAddr = rowNum/2;

		if(stAddr < NLM11KDEV_SS_SEP)
		{
			stInfo_p = &(((Nlm11kDevShadowDevice*)dev_p->m_shadowDevice_p)->m_st[stBlkNum].m_row[stAddr]);
		
			if(stInfo_p->m_ss_bsel < NLM11KDEV_NUM_OF_BMRS_PER_BLK)
				sahasraBMRConfigInfo[stInfo_p->m_ss_abs][stInfo_p->m_ss_bsel] = 1;	
		}
	}


}



static void 
Nlm11kDevMgr_pvt_DisplaySSB(
    FILE* fp,
    Nlm11kDev *dev_p,
	nlm_u8 blkNum
	)
{
    nlm_u16 entryNum = 0, bmrNum = 0;
    nlm_u16 rowNum = 0;
	nlm_u16 stAddr = 0;
    nlm_u8 numBlks = NLM11KDEV_NUM_ARRAY_BLOCKS;
    
    Nlm11kDevShadowAB *abInfo_p = &((Nlm11kDevShadowDevice*)dev_p->m_shadowDevice_p)->m_arrayBlock[blkNum];    
	
	nlm_u8 blkWidthIdx = (nlm_u8)(1 << abInfo_p->m_blkConfig.m_blockWidth); 
    
	nlm_u32 numOfEntries = NLM11KDEV_AB_DEPTH/blkWidthIdx;

	if(g_is10MDev)
		numBlks = NLM11KDEV_10M_NUM_ARRAY_BLOCKS;
	
	NlmCmFile__fprintf(fp, "SS BLK \n");

	
    if(abInfo_p->m_blkConfig.m_blockEnable == 0 )
    {
         NlmCmFile__fprintf(fp,"\n Blk Disabled \n");
         return;
    }

	NlmCmFile__fprintf(fp, "BlkWidth: %d,  Num of Entries: %d \n\n", 
					blkWidthIdx * NLM11KDEV_AB_WIDTH_IN_BITS, 
					numOfEntries);

	if(blkWidthIdx <= 1)
	{
		/*We don't support SS blocks that have a width of 80b */
		return;
	}

	NlmCmFile__fprintf(fp, "\nBMR Information \n");

	for(bmrNum = 0; bmrNum < NLM11KDEV_NUM_OF_BMRS_PER_BLK; bmrNum++)
	{
		NlmCmFile__fprintf(fp, "BMR: %d            ", bmrNum);

		Nlm11kDevMgr_pvt_DisplayBMR(fp, abInfo_p, bmrNum, blkWidthIdx);

		NlmCmFile__fprintf(fp, "\n");

	}
	NlmCmFile__fprintf(fp, "\n\n");

	bmrNum = 0;
    for(entryNum = 0; entryNum < numOfEntries; entryNum++)
    {
		rowNum =  (entryNum * blkWidthIdx);  

		if(!Nlm11kDevMgr_pvt_IsABEntryValid(abInfo_p, rowNum, blkWidthIdx))
			continue;

		Nlm11kDevMgr_pvt_DisplayABEntry(fp, abInfo_p, blkNum,
										 blkWidthIdx, rowNum, bmrNum, 
										 NlmFalse, NlmTrue);
		
		NlmCmFile__fprintf(fp, "     ");

		stAddr = rowNum/2;
		
		Nlm11kDevMgr_pvt_DisplayST(fp, dev_p, 
							blkNum - (numBlks - NLM11KDEV_NUM_AC15_BLOCKS), 
							stAddr);

		NlmCmFile__fprintf(fp, "\n\n");
		

    }


    return;
}




static void 
Nlm11kDevMgr_pvt_DisplaySahasraBlk(
    FILE* fp,
    Nlm11kDev *dev_p,
    nlm_u8 blkNum,
	NlmBool sahasraBMRConfigInfo[][NLM11KDEV_NUM_OF_BMRS_PER_BLK])
{
    nlm_u16 entryNum = 0, bmrNum = 0;
	nlm_u16 rowNum = 0;
    

    Nlm11kDevShadowAB *abInfo_p = &((Nlm11kDevShadowDevice*)dev_p->m_shadowDevice_p)->m_arrayBlock[blkNum];    
	
	nlm_u8 blkWidthIdx = (nlm_u8)(1 << abInfo_p->m_blkConfig.m_blockWidth);
    
	nlm_u32 numOfEntries = NLM11KDEV_AB_DEPTH/blkWidthIdx;
   
    
	if(abInfo_p->m_blkConfig.m_blockEnable == 0)
    {
         NlmCmFile__fprintf(fp,"\n Blk Disabled \n");
         return;
    }

	NlmCmFile__fprintf(fp, "BlkWidth: %d,  Num of Entries: %d \n\n", 
					blkWidthIdx * NLM11KDEV_AB_WIDTH_IN_BITS, 
					numOfEntries);


	for(bmrNum = 0; bmrNum < NLM11KDEV_NUM_OF_BMRS_PER_BLK; bmrNum++)
	{
		NlmCmFile__fprintf(fp, "\n\n     _______ BLK %d BMR %d _____ \n\n", blkNum, bmrNum);

		Nlm11kDevMgr_pvt_DisplayBMR(fp, abInfo_p, bmrNum, blkWidthIdx);
		
		NlmCmFile__fprintf(fp, "\n\n");

		if(!sahasraBMRConfigInfo[blkNum][bmrNum])
			continue;
	

		for(entryNum = 0; entryNum < numOfEntries; entryNum++)
		{
			rowNum =  (entryNum * blkWidthIdx);  

			if(!Nlm11kDevMgr_pvt_IsABEntryValid(abInfo_p, rowNum, blkWidthIdx))
					continue;
			
			Nlm11kDevMgr_pvt_DisplayABEntry(fp, abInfo_p, blkNum,
											 blkWidthIdx, rowNum, bmrNum, 
											 NlmTrue, NlmFalse);

			NlmCmFile__fprintf(fp, "\n");
			
		} /*Finished processing all the rows in a column */


	} /* Finished processing all the 5 columns */

	
    return;
}


static void 
Nlm11kDevMgr_pvt_DisplayNonSahasraBlk(
    FILE* fp,
    Nlm11kDev *dev_p,
    nlm_u8 blkNum  )
{
    nlm_u16 entryNum = 0, bmrNum = 0;
	nlm_u16 rowNum = 0;
    

    Nlm11kDevShadowAB *abInfo_p = &((Nlm11kDevShadowDevice*)dev_p->m_shadowDevice_p)->m_arrayBlock[blkNum];    
	
	nlm_u8 blkWidthIdx = (nlm_u8)(1 << abInfo_p->m_blkConfig.m_blockWidth);
    
	nlm_u32 numOfEntries = NLM11KDEV_AB_DEPTH/blkWidthIdx;
   
    
	if(abInfo_p->m_blkConfig.m_blockEnable == 0)
    {
         NlmCmFile__fprintf(fp,"\n Blk Disabled \n");
         return;
    }

	NlmCmFile__fprintf(fp, "BlkWidth: %d,  Num of Entries: %d \n\n", 
					blkWidthIdx * NLM11KDEV_AB_WIDTH_IN_BITS, 
					numOfEntries);


	NlmCmFile__fprintf(fp, "\n\n");

	for(bmrNum = 0; bmrNum < NLM11KDEV_NUM_OF_BMRS_PER_BLK; bmrNum++)
	{
		NlmCmFile__fprintf(fp, "BMR %d : ", bmrNum);

		Nlm11kDevMgr_pvt_DisplayBMR(fp, abInfo_p, bmrNum, blkWidthIdx);
		
		NlmCmFile__fprintf(fp, "\n");
	}
	
	NlmCmFile__fprintf(fp, "\n\n");

	bmrNum = 0;
	for(entryNum = 0; entryNum < numOfEntries; entryNum++)
	{
		rowNum =  (entryNum * blkWidthIdx);  

		if(!Nlm11kDevMgr_pvt_IsABEntryValid(abInfo_p, rowNum, blkWidthIdx))
				continue;
		
		Nlm11kDevMgr_pvt_DisplayABEntry(fp, abInfo_p, blkNum,
											blkWidthIdx, rowNum, bmrNum, 
											NlmFalse, NlmFalse);

		NlmCmFile__fprintf(fp, "\n");
		
	} /*Finished processing all the rows in a column */


	
	
    return;
}





static void
Nlm11kDevMgr_pvt_DisplayRangeEncodingType(
	FILE *fp,
	Nlm11kDevRangeEncodingType rangeEncodingType)
{
	switch(rangeEncodingType)
	{
		case NLM11KDEV_3BIT_RANGE_ENCODING:
		{
			NlmCmFile__fprintf(fp, "NLM11KDEV_3BIT_RANGE_ENCODING \n");
			break;
		}

		case NLM11KDEV_2BIT_RANGE_ENCODING:
		{
			NlmCmFile__fprintf(fp, "NLM11KDEV_2BIT_RANGE_ENCODING \n");
			break;
		}

		case NLM11KDEV_NO_RANGE_ENCODING:
		{
			NlmCmFile__fprintf(fp, "NLM11KDEV_NO_RANGE_ENCODING \n");
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
Nlm11kDevMgr_pvt_DisplayRangeEncodedValue(
	FILE *fp,
	Nlm11kDevRangeEncodedValueBytes rangeEncodedValueBytes)
{
	switch(rangeEncodedValueBytes)
	{
		case NLM11KDEV_1BYTE_RANGE_ENCODED_VALUE:
		{
			NlmCmFile__fprintf(fp, "NLM11KDEV_1BYTE_RANGE_ENCODED_VALUE \n");
			break;
		}

		case NLM11KDEV_2BYTE_RANGE_ENCODED_VALUE:
		{
			NlmCmFile__fprintf(fp, "NLM11KDEV_2BYTE_RANGE_ENCODED_VALUE \n");
			break;
		}

		case NLM11KDEV_3BYTE_RANGE_ENCODED_VALUE:
		{
			NlmCmFile__fprintf(fp, "NLM11KDEV_3BYTE_RANGE_ENCODED_VALUE \n");
			break;
		}

		case NLM11KDEV_4BYTE_RANGE_ENCODED_VALUE:
		{
			NlmCmFile__fprintf(fp, "NLM11KDEV_4BYTE_RANGE_ENCODED_VALUE \n");
			break;
		}

		default:
		{
			NlmCmFile__fprintf(fp, "Unknown = %d \n", rangeEncodedValueBytes);
			break;
		}
	}

}




static nlm_u32
Nlm11kDevMgr_pvt_DisplayBlkSelectReg(
	FILE *fp,
	Nlm11kDevBlkSelectReg* blkSelectReg_p,
	nlm_u32 regNr)
{
	nlm_u32 blkNum = 0;
	nlm_u32 numBlksSelected = 0;

	nlm_u32 numBlocksPerReg = NLM11KDEV_NUM_ARRAY_BLOCKS/2;
    if(g_is10MDev)
		numBlocksPerReg = NLM11KDEV_10M_NUM_ARRAY_BLOCKS;

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
Nlm11kDevMgr_pvt_DisplaySuperBlkKeyMap(
	FILE *fp,
	Nlm11kDevSuperBlkKeyMapReg *superBlkKeyMapReg_p)
{
	nlm_u32 superBlockNr = 0;
	nlm_u32 sbBlks = NLM11KDEV_NUM_SUPER_BLOCKS;

	NlmCmFile__fprintf(fp, "Super Block -> Key Mapping \n");

	if(g_is10MDev)
		sbBlks = NLM11KDEV_10M_NUM_SUPER_BLOCKS;
	for(superBlockNr = 0; superBlockNr < sbBlks; ++superBlockNr)
	{
		if(superBlockNr % 8 == 0 )
			NlmCmFile__fprintf(fp, "\n");

		NlmCmFile__fprintf(fp, "%2d -> %d, ", superBlockNr, superBlkKeyMapReg_p->m_keyNum[superBlockNr]);
		
	}

}


static void
Nlm11kDevMgr_pvt_DisplayParallelSrchReg(
	FILE *fp,
	Nlm11kDevParallelSrchReg * psReg_p,
	nlm_u32 regNr)
{
	nlm_u32 blkNr = 0;
	nlm_u32 numBlksPerReg = NLM11KDEV_NUM_ARRAY_BLOCKS/4;

	

	for(blkNr = 0; blkNr < numBlksPerReg; ++blkNr)
	{
		if(blkNr % 8 == 0)
			NlmCmFile__fprintf(fp, "\n");

		NlmCmFile__fprintf(fp, "%3d -> %d, ", (regNr * numBlksPerReg) + blkNr, psReg_p->m_psNum[blkNr]);
	}

}



static void
Nlm11kDevMgr_pvt_DisplayRangeInsert0Reg(
	FILE *fp,
	Nlm11kDevRangeInsertion0Reg *rangeInsertion0Reg_p)
{
	nlm_u32 keyNum = 0;

	NlmCmFile__fprintf(fp, "\n\nRange Insert0 Register \n");

	
	NlmCmFile__fprintf(fp, "Range A encoding type = "); 

	Nlm11kDevMgr_pvt_DisplayRangeEncodingType(fp, rangeInsertion0Reg_p->m_rangeAEncodingType);

	
	NlmCmFile__fprintf(fp, "Range B encoding type = ");

	Nlm11kDevMgr_pvt_DisplayRangeEncodingType(fp, rangeInsertion0Reg_p->m_rangeBEncodingType);


	NlmCmFile__fprintf(fp, "Range A encoded bytes = ");

	Nlm11kDevMgr_pvt_DisplayRangeEncodedValue(fp, rangeInsertion0Reg_p->m_rangeAEncodedBytes);


	NlmCmFile__fprintf(fp, "Range B encoded bytes = ");

	Nlm11kDevMgr_pvt_DisplayRangeEncodedValue(fp, rangeInsertion0Reg_p->m_rangeBEncodedBytes);


	NlmCmFile__fprintf(fp, "Range A insert start byte: ");
	for(keyNum = 0; keyNum < NLM11KDEV_NUM_KEYS; ++keyNum)
	{
		NlmCmFile__fprintf(fp, "Key Num %d -> Start Byte %d, ", 
					keyNum, rangeInsertion0Reg_p->m_rangeAInsertStartByte[keyNum]);
	}
	NlmCmFile__fprintf(fp, "\n");


	NlmCmFile__fprintf(fp, "Range B insert start byte: ");
	for(keyNum = 0; keyNum < NLM11KDEV_NUM_KEYS; ++keyNum)
	{
		NlmCmFile__fprintf(fp, "Key Num %d -> Start Byte %d, ", 
					keyNum, rangeInsertion0Reg_p->m_rangeBInsertStartByte[keyNum]);
	}
	NlmCmFile__fprintf(fp, "\n\n");

}



static void
Nlm11kDevMgr_pvt_DisplayRangeInsert1Reg(
	FILE *fp,
	Nlm11kDevRangeInsertion1Reg *rangeInsertion1Reg_p)
{
	nlm_u32 keyNum = 0;

	NlmCmFile__fprintf(fp, "\n\nRange Insert1 Register \n");

	
	NlmCmFile__fprintf(fp, "Range C encoding type = "); 

	Nlm11kDevMgr_pvt_DisplayRangeEncodingType(fp, rangeInsertion1Reg_p->m_rangeCEncodingType);

	
	NlmCmFile__fprintf(fp, "Range D encoding type = ");

	Nlm11kDevMgr_pvt_DisplayRangeEncodingType(fp, rangeInsertion1Reg_p->m_rangeDEncodingType);


	NlmCmFile__fprintf(fp, "Range C encoded bytes = ");

	Nlm11kDevMgr_pvt_DisplayRangeEncodedValue(fp, rangeInsertion1Reg_p->m_rangeCEncodedBytes);


	NlmCmFile__fprintf(fp, "Range D encoded bytes = ");

	Nlm11kDevMgr_pvt_DisplayRangeEncodedValue(fp, rangeInsertion1Reg_p->m_rangeDEncodedBytes);


	NlmCmFile__fprintf(fp, "Range C insert start byte: ");
	for(keyNum = 0; keyNum < NLM11KDEV_NUM_KEYS; ++keyNum)
	{
		NlmCmFile__fprintf(fp, "Key Num %d -> Start Byte %d, ", 
					keyNum, rangeInsertion1Reg_p->m_rangeCInsertStartByte[keyNum]);
	}
	NlmCmFile__fprintf(fp, "\n");


	NlmCmFile__fprintf(fp, "Range D insert start byte: ");
	for(keyNum = 0; keyNum < NLM11KDEV_NUM_KEYS; ++keyNum)
	{
		NlmCmFile__fprintf(fp, "Key Num %d -> Start Byte %d, ", 
					keyNum, rangeInsertion1Reg_p->m_rangeDInsertStartByte[keyNum]);
	}
	NlmCmFile__fprintf(fp, "\n\n");



}


static void
Nlm11kDevMgr_pvt_DisplayMiscReg(
	FILE *fp,
	Nlm11kDevMiscelleneousReg *miscReg_p)
{
	nlm_u32 psNr = 0;

	NlmCmFile__fprintf(fp, "\n\n Miscellaneous Register \n");


	for(psNr = 0; psNr < NLM11KDEV_NUM_PARALLEL_SEARCHES; ++psNr)
	{
		NlmCmFile__fprintf(fp, "PS %d -> BMR %d, ", psNr, miscReg_p->m_bmrSelect[psNr]);  
	}

	NlmCmFile__fprintf(fp,	"\nRange A extract start byte = %d \n", miscReg_p->m_rangeAExtractStartByte);

	NlmCmFile__fprintf(fp, "Range B extract start byte = %d \n", miscReg_p->m_rangeBExtractStartByte);

	NlmCmFile__fprintf(fp, "Range C extract start byte = %d \n", miscReg_p->m_rangeCExtractStartByte);

	NlmCmFile__fprintf(fp, "Range D extract start byte = %d \n", miscReg_p->m_rangeDExtractStartByte);

	NlmCmFile__fprintf(fp, "Number of valid search results = %d \n", miscReg_p->m_numOfValidSrchRslts);

	
	for(psNr = 0; psNr < NLM11KDEV_NUM_PARALLEL_SEARCHES; ++psNr)
	{
		NlmCmFile__fprintf(fp, "PS %d -> Search type %d, ", psNr, miscReg_p->m_searchType[psNr]);
	}

	NlmCmFile__fprintf(fp, "\n");
}


static void
Nlm11kDevMgr_pvt_DisplaySSReg(
	FILE *fp,
	Nlm11kDevSSReg *ssReg_p)
{
	nlm_u32 i = 0;


	NlmCmFile__fprintf(fp, "\n\n SS Register \n");

	for(i = 0; i < NLM11KDEV_SS_RMP_AB; ++i)
	{
		NlmCmFile__fprintf(fp, "SS Block %d -> PE %d, ", i, ssReg_p->m_ss_result_map[i]);

		if( (i + 1) % 4 == 0)
			NlmCmFile__fprintf(fp, "\n");

	}

	NlmCmFile__fprintf(fp, "\n");
}


static void
Nlm11kDevMgr_pvt_DisplayKCR(
	FILE *fp,
	Nlm11kDevKeyConstructReg *kcr_p	)
{
	nlm_u32 segNr = 0;

	NlmCmFile__fprintf(fp, "Start Byte  : ");
	
	for(segNr = 0; segNr < NLM11KDEV_NUM_OF_SEGMENTS_PER_KCR; ++segNr)
	{
		NlmCmFile__fprintf(fp, "%5d, ", kcr_p->m_startByteLoc[segNr]);
	}

	NlmCmFile__fprintf(fp, "\n");


	NlmCmFile__fprintf(fp, "Num of Bytes: ");
	
	for(segNr = 0; segNr < NLM11KDEV_NUM_OF_SEGMENTS_PER_KCR; ++segNr)
	{
		NlmCmFile__fprintf(fp, "%5d, ", kcr_p->m_numOfBytes[segNr]);
	}

	NlmCmFile__fprintf(fp, "\n\n");

}



static void
Nlm11kDevMgr_pvt_DisplayLTRData(
	FILE* fp,
	Nlm11kDev *dev_p)
{
	nlm_u32 ltrNum = 0;
	nlm_u32 regNr = 0;
	nlm_u32 keyNr = 0;
	nlm_u32 numBlksSelected = 0;
	nlm_u32 numBlkSelect = NLM11KDEV_NUM_ARRAY_BLOCKS/64;
	nlm_u32 numPrlSrchRegs = NLM11KDEV_NUM_ARRAY_BLOCKS/32;

	Nlm11kDevShadowDevice* shadowDevice_p = (Nlm11kDevShadowDevice*)dev_p->m_shadowDevice_p;


	NlmCmFile__fprintf(fp, "\n\n__________________LTR Data _________________\n\n");

	if(g_is10MDev)
	{
		numBlkSelect   = numBlkSelect/2;	/* 1 for 10M device */
		numPrlSrchRegs = numPrlSrchRegs/4; 
	}

	for(ltrNum = 0; ltrNum < NLM11KDEV_NUM_LTR_SET; ++ltrNum)
	{
		NlmCmFile__fprintf(fp, "LTR Num = %d \n\n", ltrNum);

		
		/*Print the block select register */
		NlmCmFile__fprintf(fp, "List of Enabled Blocks = ");

		numBlksSelected = 0;
		for(regNr = 0; regNr < numBlkSelect; ++regNr)
		{
			numBlksSelected += Nlm11kDevMgr_pvt_DisplayBlkSelectReg(fp,
										&(shadowDevice_p->m_ltr[ltrNum].m_blockSelect[regNr]),
										regNr);
		}

		if(numBlksSelected == 0)
			NlmCmFile__fprintf(fp, "None");

		NlmCmFile__fprintf(fp, "\n\n");

		
		/*Print the Super Blk - Key mapping */
		Nlm11kDevMgr_pvt_DisplaySuperBlkKeyMap(fp,
							&(shadowDevice_p->m_ltr[ltrNum].m_superBlkKeyMap));

		
		/*Print the parallel search configuration register */
		NlmCmFile__fprintf(fp, "\n\nBlock -> PS Mapping  ");

		for(regNr = 0; regNr < numPrlSrchRegs; ++regNr)
		{
			Nlm11kDevMgr_pvt_DisplayParallelSrchReg(fp,
							&(shadowDevice_p->m_ltr[ltrNum].m_parallelSrch[regNr]),
							regNr);
		}
		NlmCmFile__fprintf(fp, "\n\n");


		/*Print the RangeInsert0 Register */
		Nlm11kDevMgr_pvt_DisplayRangeInsert0Reg(fp,
							&(shadowDevice_p->m_ltr[ltrNum].m_rangeInsert0));

		/*Print the RangeInsert1 Register */
		Nlm11kDevMgr_pvt_DisplayRangeInsert1Reg(fp,
							&(shadowDevice_p->m_ltr[ltrNum].m_rangeInsert1));

		Nlm11kDevMgr_pvt_DisplayMiscReg(fp,
							&(shadowDevice_p->m_ltr[ltrNum].m_miscelleneous));

		Nlm11kDevMgr_pvt_DisplaySSReg(fp,
							&(shadowDevice_p->m_ltr[ltrNum].m_ssReg));


		/*Print the KCR */
		NlmCmFile__fprintf(fp, "\n\n KCR \n");
		for(keyNr = 0; keyNr < NLM11KDEV_NUM_KEYS ; ++keyNr)
		{
			NlmCmFile__fprintf(fp, "Key Nr = %d \n", keyNr);

			for(regNr = 0; regNr <  NLM11KDEV_NUM_OF_KCR_PER_KEY; ++regNr)
			{
				nlm_u32 index = (keyNr * NLM11KDEV_NUM_OF_KCR_PER_KEY) + regNr;

				Nlm11kDevMgr_pvt_DisplayKCR(fp,
								&(shadowDevice_p->m_ltr[ltrNum].m_keyConstruct[index])
								);
			}

			NlmCmFile__fprintf(fp, "\n\n");
		}

		NlmCmFile__fprintf(fp, "\n_____________________________________________________\n\n");

	}


}


static void
Nlm11kDevMgr_pvt_DisplayRangeRegs(
	FILE* fp,
	Nlm11kDev *dev_p)
{
	nlm_u32 regNr = 0, i = 0;
	
	Nlm11kDevShadowDevice* shadowDevice_p = (Nlm11kDevShadowDevice*)dev_p->m_shadowDevice_p;

	NlmCmFile__fprintf(fp, "\n\n    Range Control Regs Data \n\n");

	for(regNr = 0; regNr < NLM11KDEV_NUM_RANGE_REG; ++regNr)
	{
		Nlm11kDevRangeReg *rangeReg_p = &(shadowDevice_p->m_rangeReg[regNr]);

		NlmCmFile__fprintf(fp, "Addr 0x%5x: 0x", NLM11K_REG_RANGE_A_BOUNDS(0) + regNr);

		for(i = 0; i < NLM11KDEV_REG_LEN_IN_BYTES; ++i)
		{
			if(i == NLM11KDEV_REG_LEN_IN_BYTES - 1)
			{
				NlmCmFile__fprintf(fp, "%02x", rangeReg_p->m_data[i]);
			}
			else
			{
				NlmCmFile__fprintf(fp, "%02x_", rangeReg_p->m_data[i]);
			}
		}

		NlmCmFile__fprintf(fp, "\n\n");
			
	}

}






void 
Nlm11kDevMgr__DisplayShadowDevice(
	Nlm11kDev *dev_p,
	NlmBool isSahasraMode,
	char *fileName_p,
	nlm_u16 requestLowerBlkNum,
	nlm_u16 requestUpperBlkNum,
	NlmBool printRegs)
{
	nlm_u8 blkNum = 0;
	nlm_u8 startBlkNum = 0, endBlkNum = 0;
	nlm_u8 bmrNum = 0;
	NlmBool sahasraBMRConfigInfo[NLM11KDEV_NUM_ARRAY_BLOCKS][NLM11KDEV_NUM_OF_BMRS_PER_BLK];
	NlmBool isBlkInSahasraMode = NlmFalse;

	nlm_u8 numBloks = NLM11KDEV_NUM_ARRAY_BLOCKS;

	FILE* fp = NULL;

	if(!dev_p)
		return;
	
	if(!fileName_p || fileName_p[0] == 0)
		return;

	fp = fopen(fileName_p, "w");

	if(!fp)
		return;

	NlmCm__memset(sahasraBMRConfigInfo, 0, sizeof(NlmBool) * NLM11KDEV_NUM_ARRAY_BLOCKS * 
												NLM11KDEV_NUM_OF_BMRS_PER_BLK);

	if(dev_p->m_devMgr_p->m_is10MDev)
		numBloks = NLM11KDEV_10M_NUM_ARRAY_BLOCKS;
	if(isSahasraMode)
	{
		startBlkNum = numBloks - NLM11KDEV_NUM_AC15_BLOCKS;

		endBlkNum = numBloks - 1;

		for(blkNum = startBlkNum; blkNum <= endBlkNum; blkNum++)
		{
			if(blkNum >= requestLowerBlkNum && blkNum <= requestUpperBlkNum)
			{
				NlmCmFile__fprintf(fp,"\n\n\n____________________________ BLK %d _______________________\n\n\n", blkNum);

				Nlm11kDevMgr_pvt_DisplaySSB( fp, dev_p, blkNum);

				NlmCmFile__fprintf(fp, "\n\n");
			}
			
			Nlm11kDevMgr_pvt_ProcessST(dev_p, blkNum, sahasraBMRConfigInfo);
		}
	}


	startBlkNum = 0;
	
	endBlkNum = numBloks - 1;

	if(isSahasraMode)
		endBlkNum = numBloks - NLM11KDEV_NUM_AC15_BLOCKS - 1;

	
	for(blkNum = startBlkNum; blkNum <= endBlkNum; blkNum++)
	{
		if(blkNum < requestLowerBlkNum || blkNum > requestUpperBlkNum)
				continue;

		NlmCmFile__fprintf(fp,"\n\n\n____________________________ BLK %d _______________________\n\n\n", blkNum);

		if(isSahasraMode)
		{
			isBlkInSahasraMode = NlmFalse;

			for(bmrNum = 0; bmrNum < NLM11KDEV_NUM_OF_BMRS_PER_BLK; ++bmrNum)
			{
				if(sahasraBMRConfigInfo[blkNum][bmrNum])
				{
					isBlkInSahasraMode = NlmTrue;
					break;
				}
			}

			if(isBlkInSahasraMode)
			{
				Nlm11kDevMgr_pvt_DisplaySahasraBlk(fp, dev_p, blkNum,
													sahasraBMRConfigInfo);
			}
			else
			{
				Nlm11kDevMgr_pvt_DisplayNonSahasraBlk( fp, dev_p, blkNum);
			}
		}
		else
		{
			Nlm11kDevMgr_pvt_DisplayNonSahasraBlk( fp, dev_p, blkNum);
		}

	}


	if(printRegs)
	{
		Nlm11kDevMgr_pvt_DisplayLTRData(fp, dev_p);

		Nlm11kDevMgr_pvt_DisplayRangeRegs(fp, dev_p);
	}



	
	fclose(fp);

	return ;

}



