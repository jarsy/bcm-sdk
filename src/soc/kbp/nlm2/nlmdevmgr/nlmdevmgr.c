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
 
/* Revision:  SDK 2.2a on 10-Nov-2011 */

#include "nlmdevmgr.h"
#include "nlmdevmgr_shadow.h"
#include "nlmdevmgr_ss.h"
#include "nlmcmutility.h"
#include "nlmxpt.h"
#include "nlmcmstring.h"
#include "nlmdevmgr_display.h"
/*******************************************************************************/
/*
File Description:
This file handles the device specific details of SDK.
Device manager module is responsible to create, manage and destroy the 
device or a cascade system of the same.
This file contains implementations of following Device manager APIs.

	Nlm11kDevMgr__create
	Nlm11kDevMgr__destroy
	Nlm11kDevMgr__AddDevice
	Nlm11kDevMgr__ResetDevices
	Nlm11kDevMgr__GlobalRegisterRead
	Nlm11kDevMgr__GlobalRegisterWrite
	Nlm11kDevMgr__CBAsRegisterRead
	Nlm11kDevMgr__CBAsRegisterWrite
	Nlm11kDevMgr__LogicalTableRegisterRead
	Nlm11kDevMgr__LogicalTableRegisterWrite
    Nlm11kDevMgr__LogicalTableRegisterRefresh
	Nlm11kDevMgr__BlockRegisterRead
	Nlm11kDevMgr__BlockRegisterWrite
	Nlm11kDevMgr__ABEntryRead
	Nlm11kDevMgr__ABEntryWrite
	Nlm11kDevMgr__ABEntryInvalidate
	Nlm11kDevMgr__ABEntryRefresh
	Nlm11kDevMgr__Compare1
	Nlm11kDevMgr__Compare2
	Nlm11kDevMgr__CBWrite
	Nlm11kDevMgr__LockConfig
    Nlm11kDevMgr__RangeRegisterWrite
    Nlm11kDevMgr__RangeRegisterRead
    Nlm11kDevMgr__ShadowDeviceCreate
    Nlm11kDevMgr__ShadowDeviceDestroy
	Nlm11kDevMgr__SendNop
	Nlm11kDevMgr__RegisterWrite
	Nlm11kDevMgr__RegisterRead
	Nlm11kDevMgr__DumpShadowMemory

Device instruction APIs (read, write, compare) takes input from the user
application in a format(say F1) which is not compatible to input
format(say F2) what transport layer expects. These APIs serve this purpose ie.
converting format F1 to format F2 for transport layer. Transport layer takes
instructions as input in a request structure which is in similar format
as actual device accept, ie. Instruction_Bus, Data_Bus, Context_Address.
For more details about this format please see the data sheet and transport
layer module documents/comments. Please see the function declarations/
definitions for details of the above APIs.

Methodologies and Algorithms:
The file serves as a simple interface between Application  and
transport_layer, so it need not define/use any algorithm. For
managing devices in a cascade system a link list of all devices is
maintained in Nlm11kDevMgr structure. This file does not handle multi
channel case, so one Device Manager can handle only one cascade system.
Multiple channels must be managed in GTM/User_Application.

File position in the full integrated system:

|-----------------------|		|-----------------------|		|-----------------------|
|						|		|						|		|						|
|Generic Table Manager	|		|	User Application	|		|	Range Manager		|
|						|		|						|		|						|
|-----------------------|		|-----------------------|		|-----------------------|
			|								|								|
			|								|								|
			|								|								|
			|					____________v____________					|
			|----------------->	|						|<------------------|
								|Device Manager.h/.c	|
								|						|
								|_______________________|
											|
											v
								|-----------------------|
								|						|
								|	Transport Layer		|
								|						|
								|-----------------------|

*/
/*******************************************************************************/

#define NLM11K_DEVMGR_INSERT_DEVID(addr, devId) \
    (devId << 23) | (addr);

#define NLM11K_DEVMGR_PTR_MAGIC_NUM   0x54321
#define NLM11K_DEV_PTR_MAGIC_NUM   0x54320

#define NLM11K_IS_VALID_DEVMGR_PTR(devMgr) ((devMgr) && (devMgr)->m_magicNum == NLM11K_DEVMGR_PTR_MAGIC_NUM)
#define NLM11K_IS_VALID_DEV_PTR(dev) ((dev) && (dev)->m_magicNum == NLM11K_DEV_PTR_MAGIC_NUM)

/* LTR Misc Reg search type start and end bits. (PHMD) */
#define	NLM11KDEV_MISCREG_SEARCH_START_BIT	48
#define	NLM11KDEV_MISCREG_SEARCH_END_BIT	55

/* LTR SS Result Map start and end bits. (PHMD) */
#define	NLM11KDEV_SSREG_SSMAP_START_BIT	9
#define	NLM11KDEV_SSREG_SSMAP_END_BIT	16

/* Result index related defines. (PHMD) */
#define NLM11KDEV_CMP_RSLT_IDX_START_BIT	(0)
#define NLM11KDEV_CMP_RSLT_IDX_END_BIT		(30)
#define NLM11KDEV_CMP_RSLT_DEVID_START_BIT	(21)
#define NLM11KDEV_CMP_RSLT_SMF_BIT			(30)
#define NLM11KDEV_CMP_RSLT_SMF_MASK			(0x1)
#define	NLM11KDEV_CMP_RSLT_DEVID_MASK       (0x3)
#define NLM11KDEV_CMP_FIB_RSLT_IDX_MASK	    (0x7FFFFF)
#define NLM11KDEV_CMP_ACL_RSLT_IDX_MASK	    (0x1FFFFF)

#define NLM11K_DUMP_MAX_FILENAME_LEN		(100)

/* Number of NOPs for Errata */
#ifdef NLM_PRE_RA12  /* Only KBPs before rev RA12 need to send NOPs */
 #define NLM11KDEV_NUM_OF_NOPS_FOR_AB		1 
 #define NLM11KDEV_NUM_OF_NOPS_FOR_REG		2 
#endif


static Nlm11kDevMgr *g_devMgr_p = NULL;

NlmBool    g_is10MDev = 0;  /* if "1" then device is 10M, else 40M ("0"), 
							   By reading the Device Identication Register
							   database-size field this flag will be set or reset */

#if 0
static NlmErrNum_t Nlm11kDevMgr__pvt_CtxBufferInit(
    Nlm11kDevMgr				*self,
	NlmReasonCode*				o_reason
	);
#endif

/*
Function: Nlm11kDevMgr__ctor
Description: constructor of Device Manager. Initializes the memory
allocator pointer and XPT pointer to the corresponding member variables
of the Device Manager. It initializes the device count to zero and a creates
a device list of a length NLM11KDEV_MAX_DEV_NUM.
*/
static Nlm11kDevMgr* Nlm11kDevMgr__ctor(
    Nlm11kDevMgr			*self,
	void*					xpt_p,
    Nlm11kDev_OperationMode operMode,
    NlmCmAllocator			*alloc_p
	)
{
    NlmCmAssert((alloc_p != NULL), "Invalid memory allocator provided.\n");
    NlmCmAssert((self != NULL), "Invalid self pointer.\n");

    self->m_alloc_p     = alloc_p;
	self->m_devCount   = 0;
	self->m_xpt_p       = xpt_p;
    self->m_operMode = operMode;
	self->m_isLocked    = NLMFALSE;
    self->m_magicNum = NLM11K_DEVMGR_PTR_MAGIC_NUM;
	self->m_is10MDev    = NlmFalse;
	g_is10MDev          = self->m_is10MDev;  /* Using 10M device */

	self->m_devList_pp = NlmCmAllocator__calloc(alloc_p,
                                                 NLM11KDEV_MAX_DEV_NUM,
                                                 sizeof(Nlm11kDev*));
    NlmCmDemand((self != NULL), "Out of memory.\n");

	return self;
}

/*
Function: Nlm11kDevMgr__dtor
Description: destructure of Device Manager deletes the list of devices including
all the devices and their shadow devices and frees the memory.
*/
static void Nlm11kDevMgr__dtor(
    Nlm11kDevMgr			*self
	)
{
	nlm_32 Idx = 0;
	NlmCmAllocator *alloc_p = NULL;
    Nlm11kDev *dev_p;

    if(self == NULL)
		return;

	alloc_p = self->m_alloc_p;

	/* traverse the device list */
	if(self->m_devList_pp)
	{
		for(Idx =0; Idx < NLM11KDEV_MAX_DEV_NUM; Idx++)
		{
            if(NULL != self->m_devList_pp[Idx])
			{
                dev_p = (Nlm11kDev*)(self->m_devList_pp[Idx]);

				/* deleting shadow device */
				Nlm11kDevMgr__ShadowDeviceDestroy(dev_p, NULL);

				/* deleting the device */
				NlmCmAllocator__free(alloc_p, dev_p);

				self->m_devList_pp[Idx] = NULL;
                self->m_devCount--;
			}
		}
		/* deleting the list */
		NlmCmAllocator__free(alloc_p, self->m_devList_pp);
		self->m_devList_pp = NULL;
	}
}


/* Nlm11kDevMgr_pvt_ConstructBlkSelLtrData contructs the 80b Reg Data
 based on various values of the various fields of Blk Select Register provided by application
 */
static NlmErrNum_t Nlm11kDevMgr_pvt_ConstructBlkSelLtrData(
	nlm_u8					*o_data,
	Nlm11kDevBlkSelectReg	*blkSelectData_p
	)
{
    nlm_32 abNum;
    nlm_u32 value = 0;
    nlm_32 bitSelector = 0;

   /* A Blk Select Reg contains Blk enables for (NLM11KDEV_NUM_ARRAY_BLOCKS/2) number of array Blks
    i.e. Blk Select 0 Reg -- Contains Blk Enable for AB Num 0 - 63
    while Blk Select 1 Reg -- Contains Blk Enable for AB Num 64 - 127;
    Each block uses 1 bit as Enable Bit in the Register */

    /*Since "WriteBitsInArray" can write maximum of 32b in to an array
    we update the Blk Select Reg Data for 32 Blks first and then for the remaining 32 Blks  */
    for(abNum = 0; abNum < (NLM11KDEV_NUM_ARRAY_BLOCKS/4); abNum++, bitSelector++)
    {
        if(blkSelectData_p->m_blkEnable[abNum] != NLM11KDEV_DISABLE)
            value |= (1 << bitSelector);
    }
    WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 31, 0, value);

    value = 0;
    bitSelector =0;
    
	if(g_is10MDev) /* in case of 10M MSB bits must 0 */
	{
		WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 63, 32, value);
		return NLMERR_OK;
	}
    for(abNum = (NLM11KDEV_NUM_ARRAY_BLOCKS/4); abNum < (NLM11KDEV_NUM_ARRAY_BLOCKS/2);
                                                                    abNum++, bitSelector++)
    {
        if(blkSelectData_p->m_blkEnable[abNum] != NLM11KDEV_DISABLE)
            value |= (1 << bitSelector);
    }
    WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 63, 32, value);

    return NLMERR_OK;
}

/* Nlm11kDevMgr_pvt_ExtractBlkSelLtrData extracts the various fields  of Blk Select Register
from the 80b Reg Data read from the device */
static void Nlm11kDevMgr_pvt_ExtractBlkSelLtrData(
	nlm_u8					*readData,
	Nlm11kDevBlkSelectReg	*blkSelectData_p
	)
{
    nlm_32 abNum;
    nlm_u32 value = 0;

    /*Since "ReadBitsInArrray" can read maximum of 32b in to an array
    hence we extract the Blk Select Reg Data for 32 Blks first and then for the remaining 32 Blks  */
    value = ReadBitsInArrray(readData, NLM11KDEV_REG_LEN_IN_BYTES, 31, 0);
    for(abNum = 0; abNum < (NLM11KDEV_NUM_ARRAY_BLOCKS/4); abNum++, value >>= 1)
        blkSelectData_p->m_blkEnable[abNum] = value & 1;

    if(g_is10MDev) /* in case of 10M MSB bits must 0 */
		value = 0;
	else
        value = ReadBitsInArrray(readData, NLM11KDEV_REG_LEN_IN_BYTES, 63, 32);
    for(abNum = (NLM11KDEV_NUM_ARRAY_BLOCKS/4); abNum < (NLM11KDEV_NUM_ARRAY_BLOCKS/2);
                                                                    abNum++, value >>= 1)
        blkSelectData_p->m_blkEnable[abNum] = value & 1;
}

/* Nlm11kDevMgr_pvt_ConstructSBKeySelLtrData contructs the 80b Reg Data based on various values
of the various fields of Super Blk Key Select Reg provided by application
 */
static NlmErrNum_t Nlm11kDevMgr_pvt_ConstructSBKeySelLtrData(
	nlm_u8						*o_data,
	Nlm11kDevSuperBlkKeyMapReg	*sbKeySelectData_p
	)
{
    nlm_32 sbNum;
    nlm_u32 value = 0;
    nlm_32 bitSelector = 0;
     /* A Super Blk Key Select Reg contains Key maps for the NLM11KDEV_NUM_SUPER_BLOCKS super Blocks;
    Since Key Num can be any value from 0 -3 Each super block uses 2 bits in the Register*/

    /*Since "WriteBitsInArray" can write maximum of 32b in to an array
    we update the Key Select Reg Data for 16 super Blks first and then for the remaining 16 super Blks  */
    for(sbNum = 0; sbNum < (NLM11KDEV_NUM_SUPER_BLOCKS/2); sbNum++, bitSelector += 2)
    {
		if(g_is10MDev) /* in case of 10M only 8 SB supported */
		{
			if(sbNum == 8)
				break;
		}
        if(sbKeySelectData_p->m_keyNum[sbNum] !=  NLM11KDEV_KEY_0)
        {
            /* Key Value should be 0 - 3 */
            if(sbKeySelectData_p->m_keyNum[sbNum] != NLM11KDEV_KEY_1
                && sbKeySelectData_p->m_keyNum[sbNum] != NLM11KDEV_KEY_2
                && sbKeySelectData_p->m_keyNum[sbNum] != NLM11KDEV_KEY_3)
                return NLMERR_FAIL;

            value |= (sbKeySelectData_p->m_keyNum[sbNum] << bitSelector);
        }
    }
    WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 31, 0, value);

    value = 0;
    bitSelector = 0;

	if(g_is10MDev) /* in case of 10M MSB bits must 0 */
	{
		WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 63, 32, value);
		return NLMERR_OK;
	}
    for(sbNum = (NLM11KDEV_NUM_SUPER_BLOCKS/2); sbNum < NLM11KDEV_NUM_SUPER_BLOCKS;
                                                            sbNum++, bitSelector += 2)
    {
        if(sbKeySelectData_p->m_keyNum[sbNum] !=  NLM11KDEV_KEY_0)
        {
            /* Key Value should be 0 - 3 */
            if(sbKeySelectData_p->m_keyNum[sbNum] != NLM11KDEV_KEY_1
                && sbKeySelectData_p->m_keyNum[sbNum] != NLM11KDEV_KEY_2
                && sbKeySelectData_p->m_keyNum[sbNum] != NLM11KDEV_KEY_3)
                return NLMERR_FAIL;

            value |= (sbKeySelectData_p->m_keyNum[sbNum] << bitSelector);
        }
    }
    WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 63, 32, value);

    return NLMERR_OK;
}

/* Nlm11kDevMgr_pvt_ExtractSBKeySelLtrData extracts the various fields  of Super Blk Key Select Reg
from the 80b Reg Data read from the device */
static void Nlm11kDevMgr_pvt_ExtractSBKeySelLtrData(
	nlm_u8						*readData,
	Nlm11kDevSuperBlkKeyMapReg	*sbKeySelectData_p
	)
{
    nlm_32 sbNum;
    nlm_u32 value = 0;

    /*Since "ReadBitsInArrray" can read maximum of 32b in to an array
    we extract the Key Select Reg Data for 16 super Blks first and then for the remaining 16 super Blks  */
    value = ReadBitsInArrray(readData, NLM11KDEV_REG_LEN_IN_BYTES, 31, 0);
    for(sbNum = 0; sbNum < (NLM11KDEV_NUM_SUPER_BLOCKS/2); sbNum++, value >>= 2)
        sbKeySelectData_p->m_keyNum[sbNum] = value & 0x3;

    if(g_is10MDev) /* in case of 10M MSB bits must 0 */
		value = 0;
	else
        value = ReadBitsInArrray(readData, NLM11KDEV_REG_LEN_IN_BYTES, 63, 32);
    for(sbNum = (NLM11KDEV_NUM_SUPER_BLOCKS/2); sbNum < NLM11KDEV_NUM_SUPER_BLOCKS;
                                                            sbNum++, value >>= 2)
        sbKeySelectData_p->m_keyNum[sbNum] = value & 0x3;
}

/* Nlm11kDevMgr_pvt_ConstructParallelSrchLtrData contructs the 80b Reg Data based on various values
    of the various fields of Parallel Srch Reg provided by application */
static NlmErrNum_t Nlm11kDevMgr_pvt_ConstructParallelSrchLtrData(
	nlm_u8						*o_data,
	Nlm11kDevParallelSrchReg	*parallelSrchData_p
	)
{
    nlm_32 abNum;
    nlm_u32 value = 0;
    nlm_32 bitSelector = 0;
     /* A Parallel Srch Reg contains Result Port Maps for the (NLM11KDEV_NUM_ARRAY_BLOCKS/4) array Blocks;
                 i.e. Parallel Srch 0 Reg -- Contains Blk Enable for AB Num 0 - 31
                      Parallel Srch 1 Reg -- Contains Blk Enable for AB Num 32 - 63
                      Parallel Srch 2 Reg -- Contains Blk Enable for AB Num 64 - 95
                      Parallel Srch 3 Reg -- Contains Blk Enable for AB Num 96 - 127
    
	Since Result Port Num can be any value from 0 - 3 Each array block uses 2 bits in the Register
	
	Note:
		Parallel Srch 1/2/3 Reg are not configurable in 10M device */
	

     /*Since "WriteBitsInArray" can write maximum of 32b in to an array
        we update the Parallel Srch Reg Data for 16 array Blks first and
        then for the remaining 16 array Blks  */
    for(abNum = 0; abNum < (NLM11KDEV_NUM_ARRAY_BLOCKS/8); abNum++, bitSelector += 2)
    {
        if(parallelSrchData_p->m_psNum[abNum] !=  NLM11KDEV_PARALLEL_SEARCH_0)
        {
            /* Rslt Port Num Value should be 0 - 3 */
            if(parallelSrchData_p->m_psNum[abNum] != NLM11KDEV_PARALLEL_SEARCH_1
                && parallelSrchData_p->m_psNum[abNum] != NLM11KDEV_PARALLEL_SEARCH_2
                && parallelSrchData_p->m_psNum[abNum] != NLM11KDEV_PARALLEL_SEARCH_3)
                return NLMERR_FAIL;

            value |= (parallelSrchData_p->m_psNum[abNum] << bitSelector);
        }
    }
    WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 31, 0, value);

    value = 0;
    bitSelector =0;
    for(abNum = (NLM11KDEV_NUM_ARRAY_BLOCKS/8);
            abNum < (NLM11KDEV_NUM_ARRAY_BLOCKS/4);
                    abNum++, bitSelector += 2)
    {
        if(parallelSrchData_p->m_psNum[abNum] !=  NLM11KDEV_PARALLEL_SEARCH_0)
        {
            /* Rslt Port Num Value should be 0 - 3 */
            if(parallelSrchData_p->m_psNum[abNum] != NLM11KDEV_PARALLEL_SEARCH_1
                && parallelSrchData_p->m_psNum[abNum] != NLM11KDEV_PARALLEL_SEARCH_2
                && parallelSrchData_p->m_psNum[abNum] != NLM11KDEV_PARALLEL_SEARCH_3)
                return NLMERR_FAIL;

            value |= (parallelSrchData_p->m_psNum[abNum] << bitSelector);
        }
    }
    WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 63, 32, value);

    return NLMERR_OK;
}

/* Nlm11kDevMgr_pvt_ExtractParallelSrchLtrData extracts the various fields  of Parallel Srch Reg
from the 80b Reg Data read from the device */
static void Nlm11kDevMgr_pvt_ExtractParallelSrchLtrData(
	nlm_u8						*readData,
	Nlm11kDevParallelSrchReg	*parallelSrchData_p
	)
{
    nlm_32 abNum;
    nlm_u32 value = 0;

     /*Since "ReadBitsInArrray" can read maximum of 32b in to an array
        we extract the Parallel Srch Reg Data for 16 array Blks first and
        then for the remaining 16 array Blks  */
    value = ReadBitsInArrray(readData, NLM11KDEV_REG_LEN_IN_BYTES, 31, 0);
    for(abNum = 0; abNum < (NLM11KDEV_NUM_ARRAY_BLOCKS/8); abNum++, value >>= 2)
        parallelSrchData_p->m_psNum[abNum] = value & 0x3;

    value = ReadBitsInArrray(readData, NLM11KDEV_REG_LEN_IN_BYTES, 63, 32);
    for(abNum = (NLM11KDEV_NUM_ARRAY_BLOCKS/8); abNum < (NLM11KDEV_NUM_ARRAY_BLOCKS/4);
                                                            abNum++, value >>= 2)
    parallelSrchData_p->m_psNum[abNum] = value & 0x3;
}

/* Nlm11kDevMgr_pvt_ConstructKeyContructLtrData contructs the 80b Reg Data based on various values
    of the various fields of Key Contruct Reg provided by application */
static NlmErrNum_t Nlm11kDevMgr_pvt_ConstructKeyContructLtrData(
	nlm_u8						*o_data,
	Nlm11kDevKeyConstructReg	*keyContructData_p
	)
{
    nlm_32 segNum = 0;

    /* A Key Construction Reg contains Key Construction details of the Keys;
    There are two KCR for each key with each KCR containing details of 5 segments
    of key contruction. Each Segment requires 7 Bits for Start Byte and 5 Bits for
    Number of Bytes; Valid values of start byte is 0 - NLM11KDEV_MAX_KEY_LEN_IN_BYTES
    and for Number of Bytes is 1 - 16; If Number of Bytes of any segment is specified
    to be zero then it indicates that next segments needs be ignored*/

    for(segNum = 0; segNum < NLM11KDEV_NUM_OF_SEGMENTS_PER_KCR; segNum++)
    {
	 /* If Number of Bytes = 0, ignore the remaining segments of Register */
        if(keyContructData_p->m_numOfBytes[segNum] == 0)
        {
		/* Write 0x50 to indicate end of segments */
		WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, segNum * 11 + 6,
            			segNum * 11, NLM11KDEV_MAX_KEY_LEN_IN_BYTES);
            return NLMERR_OK;
        }

        /* Check the correctness of Start Byte Location */
        if(keyContructData_p->m_startByteLoc[segNum] >= NLM11KDEV_MAX_KEY_LEN_IN_BYTES)
            return NLMERR_FAIL;

        /* Check the correctness of Number of Bytes */
        if(keyContructData_p->m_numOfBytes[segNum] > 16)
            return NLMERR_FAIL;

        /* Start Byte value for various segments is at following Bits of Register
        Segment 0 - Bits[6:0] ; Segment 1 - Bits[17:11]
        Segment 2 - Bits[28:22] ; Segment 3 - Bits[39:33]
        Segment 4 - Bits[50:44]*/
        WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, segNum * 11 + 6,
            segNum * 11, keyContructData_p->m_startByteLoc[segNum]);

        /* Number of Bytes value for various segments is at following Bits of Register
        Segment 0 - Bits[10:7] ; Segment 1 - Bits[21:18]
        Segment 2 - Bits[32:29] ; Segment 3 - Bits[43:40]
        Segment 4 - Bits[54:51]*/
        WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, segNum * 11 + 10,
            segNum * 11 + 7, keyContructData_p->m_numOfBytes[segNum]- 1);
    }
    return NLMERR_OK;
}

/* Nlm11kDevMgr_pvt_ExtractKeyContructLtrData extracts the various fields  of Key Construct Reg
from the 80b Reg Data read from the device */
static void Nlm11kDevMgr_pvt_ExtractKeyContructLtrData(
	nlm_u8						*readData,
    Nlm11kDevKeyConstructReg	*keyContructData_p
    )
{
    nlm_32 segNum = 0;

    for(segNum = 0; segNum < NLM11KDEV_NUM_OF_SEGMENTS_PER_KCR; segNum++)
    {
        /* Start Byte value for various segments is at following Bits of Register
        Segment 0 - Bits[6:0] ; Segment 1 - Bits[17:11]
        Segment 2 - Bits[28:22] ; Segment 3 - Bits[39:33]
        Segment 4 - Bits[50:44]*/
        keyContructData_p->m_startByteLoc[segNum] = (nlm_u8)ReadBitsInArrray(readData,
                                                            NLM11KDEV_REG_LEN_IN_BYTES,
                                                            segNum * 11 + 6,
                                                            segNum * 11);

		
	if(keyContructData_p->m_startByteLoc[segNum] > (NLM11KDEV_MAX_KEY_LEN_IN_BYTES -1 ))
	{
		/* End of segment occured, so write 0s in all remaining segments */
		keyContructData_p->m_startByteLoc[segNum] = 0;
		break;
	}

        /* Number of Bytes value for various segments is at following Bits of Register
        Segment 0 - Bits[10:7] ; Segment 1 - Bits[21:18]
        Segment 2 - Bits[32:29] ; Segment 3 - Bits[43:40]
        Segment 4 - Bits[54:51]*/
        keyContructData_p->m_numOfBytes[segNum] = (nlm_u8)(ReadBitsInArrray(readData,
                                                           NLM11KDEV_REG_LEN_IN_BYTES,
                                                           segNum * 11 + 10,
                                                           segNum * 11 + 7) + 1);
    }

    return;
}

/* Nlm11kDevMgr_pvt_ConstructRangeInsertLtrData contructs the 80b Reg Data based on various values
    of the various fields of Range Insert Reg provided by application */
static NlmErrNum_t Nlm11kDevMgr_pvt_ConstructRangeInsertLtrData(
	nlm_u8			*o_data,
	void			*inputData_p,
	nlm_u32			regType
	)
{
    nlm_32 keyNum;
    nlm_u32 value = 0;

    /* A Range Insertion0 Reg contains details for Range A and RangeB about the type of encoding
    used , number of bytes of Range Encoding to be inserted in the keys and location where Range
    Encodings needs to be inserted in each of the keys. Range Insertion1 Reg contains similar
    details for Range C and Range D*/
    if(regType == NLM11KDEV_RANGE_INSERTION_0_LTR)
    {
        Nlm11kDevRangeInsertion0Reg *rangeInsertData_p =  (Nlm11kDevRangeInsertion0Reg*)inputData_p;

        /* Valid values for Type of Range Encoding is 0 - 2*/
        if(rangeInsertData_p->m_rangeAEncodingType != NLM11KDEV_3BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeAEncodingType != NLM11KDEV_2BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeAEncodingType != NLM11KDEV_NO_RANGE_ENCODING)
            return NLMERR_FAIL;

        if(rangeInsertData_p->m_rangeBEncodingType != NLM11KDEV_3BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeBEncodingType != NLM11KDEV_2BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeBEncodingType != NLM11KDEV_NO_RANGE_ENCODING)
            return NLMERR_FAIL;

        /* Valid values for Number of Bytes of Range Encoding is 0 - 3*/
        if(rangeInsertData_p->m_rangeAEncodedBytes != NLM11KDEV_1BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeAEncodedBytes != NLM11KDEV_2BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeAEncodedBytes != NLM11KDEV_3BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeAEncodedBytes != NLM11KDEV_4BYTE_RANGE_ENCODED_VALUE)
            return NLMERR_FAIL;

        if(rangeInsertData_p->m_rangeBEncodedBytes != NLM11KDEV_1BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeBEncodedBytes != NLM11KDEV_2BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeBEncodedBytes != NLM11KDEV_3BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeBEncodedBytes != NLM11KDEV_4BYTE_RANGE_ENCODED_VALUE)
            return NLMERR_FAIL;

        /* Location of various Field in Reg are as follows
            Range B Encoding Type: Bits[63:62]
            Range A Encoding Type: Bits[61:60]
            Range B Encoding Bytes: Bits[59:58]
            Range A Encoding Bytes: Bits[57:56] */
        value = (rangeInsertData_p->m_rangeBEncodingType << 6)
            | (rangeInsertData_p->m_rangeAEncodingType << 4)
            | (rangeInsertData_p->m_rangeBEncodedBytes << 2)
            | rangeInsertData_p->m_rangeAEncodedBytes;
        WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 63, 56, value);

         /* Location of RangeB Insert Field for various keys in Reg are as follows
           Key 0 : Bits[34:28]
           Key 1 : Bits[41:35]
           Key 2 : Bits[48:42]
           Key 3 : Bits[55:49] */
        value = 0;
        for(keyNum = 0; keyNum < NLM11KDEV_NUM_KEYS; keyNum++)
        {
            if(rangeInsertData_p->m_rangeBInsertStartByte[keyNum] >= NLM11KDEV_MAX_KEY_LEN_IN_BYTES
                && rangeInsertData_p->m_rangeBInsertStartByte[keyNum] != NLM11KDEV_RANGE_DO_NOT_INSERT)
                return NLMERR_FAIL;

            value |= (rangeInsertData_p->m_rangeBInsertStartByte[keyNum] << (keyNum * 7));
        }
        WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 55, 28, value);

        /* Location of RangeA Insert Field for various keys in Reg are as follows
           Key 0 : Bits[6:0]
           Key 1 : Bits[13:7]
           Key 2 : Bits[20:14]
           Key 3 : Bits[27:21] */
        value = 0;
        for(keyNum = 0; keyNum < NLM11KDEV_NUM_KEYS; keyNum++)
        {
             if(rangeInsertData_p->m_rangeAInsertStartByte[keyNum] >= NLM11KDEV_MAX_KEY_LEN_IN_BYTES
                && rangeInsertData_p->m_rangeAInsertStartByte[keyNum] != NLM11KDEV_RANGE_DO_NOT_INSERT)
                return NLMERR_FAIL;

            value |= (rangeInsertData_p->m_rangeAInsertStartByte[keyNum] << (keyNum * 7));
        }
        WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 27, 0, value);
    }
    else
    {
        Nlm11kDevRangeInsertion1Reg *rangeInsertData_p =  (Nlm11kDevRangeInsertion1Reg*)inputData_p;

         /* Valid values for Type of Range Encoding is 0 - 2*/
        if(rangeInsertData_p->m_rangeCEncodingType != NLM11KDEV_3BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeCEncodingType != NLM11KDEV_2BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeCEncodingType != NLM11KDEV_NO_RANGE_ENCODING)
            return NLMERR_FAIL;

        if(rangeInsertData_p->m_rangeDEncodingType != NLM11KDEV_3BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeDEncodingType != NLM11KDEV_2BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeDEncodingType != NLM11KDEV_NO_RANGE_ENCODING)
            return NLMERR_FAIL;

         /* Valid values for Number of Bytes of Range Encoding is 0 - 3*/
        if(rangeInsertData_p->m_rangeCEncodedBytes != NLM11KDEV_1BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeCEncodedBytes != NLM11KDEV_2BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeCEncodedBytes != NLM11KDEV_3BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeCEncodedBytes != NLM11KDEV_4BYTE_RANGE_ENCODED_VALUE)
            return NLMERR_FAIL;

        if(rangeInsertData_p->m_rangeDEncodedBytes != NLM11KDEV_1BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeDEncodedBytes != NLM11KDEV_2BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeDEncodedBytes != NLM11KDEV_3BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeDEncodedBytes != NLM11KDEV_4BYTE_RANGE_ENCODED_VALUE)
            return NLMERR_FAIL;

        /* Location of various Field in Reg are as follows
            Range D Encoding Type: Bits[63:62]
            Range C Encoding Type: Bits[61:60]
            Range D Encoding Bytes: Bits[59:58]
            Range C Encoding Bytes: Bits[57:56] */
        value = (rangeInsertData_p->m_rangeDEncodingType << 6)
            | (rangeInsertData_p->m_rangeCEncodingType << 4)
            | (rangeInsertData_p->m_rangeDEncodedBytes << 2)
            | rangeInsertData_p->m_rangeCEncodedBytes;

        WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 63, 56, value);

         /* Location of RangeD Insert Field for various keys in Reg are as follows
           Key 0 : Bits[34:28]
           Key 1 : Bits[41:35]
           Key 2 : Bits[48:42]
           Key 3 : Bits[55:49] */
        value = 0;
        for(keyNum = 0; keyNum < NLM11KDEV_NUM_KEYS; keyNum++)
        {
            if(rangeInsertData_p->m_rangeDInsertStartByte[keyNum] >= NLM11KDEV_MAX_KEY_LEN_IN_BYTES
                && rangeInsertData_p->m_rangeDInsertStartByte[keyNum] != NLM11KDEV_RANGE_DO_NOT_INSERT)
                return NLMERR_FAIL;

            value |= (rangeInsertData_p->m_rangeDInsertStartByte[keyNum] << (keyNum * 7));
        }
        WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 55, 28, value);

        /* Location of RangeC Insert Field for various keys in Reg are as follows
           Key 0 : Bits[6:0]
           Key 1 : Bits[13:7]
           Key 2 : Bits[20:14]
           Key 3 : Bits[27:21] */
        value = 0;
        for(keyNum = 0; keyNum < NLM11KDEV_NUM_KEYS; keyNum++)
        {
            if(rangeInsertData_p->m_rangeCInsertStartByte[keyNum] >= NLM11KDEV_MAX_KEY_LEN_IN_BYTES
                && rangeInsertData_p->m_rangeCInsertStartByte[keyNum] != NLM11KDEV_RANGE_DO_NOT_INSERT)
                return NLMERR_FAIL;
            value |= (rangeInsertData_p->m_rangeCInsertStartByte[keyNum] << (keyNum * 7));
        }
        WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 27, 0, value);
    }
    return NLMERR_OK;
}

/* Nlm11kDevMgr_pvt_ExtractRangeInsertLtrData extracts the various fields  of Range Insert Reg
from the 80b Reg Data read from the device */
static void Nlm11kDevMgr_pvt_ExtractRangeInsertLtrData(
	nlm_u8		*readData,
	void		*outputData_p,
	nlm_u32		regType
	)
{
    nlm_32 keyNum;
    nlm_u32 value = 0;

    if(regType == NLM11KDEV_RANGE_INSERTION_0_LTR)
    {
        Nlm11kDevRangeInsertion0Reg *rangeInsertData_p =  (Nlm11kDevRangeInsertion0Reg*)outputData_p;

        /* Location of various Field in Reg are as follows
            Range B Encoding Type: Bits[63:62]
            Range A Encoding Type: Bits[61:60]
            Range B Encoding Bytes: Bits[59:58]
            Range A Encoding Bytes: Bits[57:56] */
        value = ReadBitsInArrray(readData, NLM11KDEV_REG_LEN_IN_BYTES, 63, 56);
        rangeInsertData_p->m_rangeBEncodingType = (value >> 6) & 0x3;
        rangeInsertData_p->m_rangeAEncodingType = (value >> 4) & 0x3;
        rangeInsertData_p->m_rangeBEncodedBytes = (value >> 2) & 0x3;
        rangeInsertData_p->m_rangeAEncodedBytes = value & 0x3;

        /* Location of RangeB Insert Field for various keys in Reg are as follows
           Key 0 : Bits[34:28]
           Key 1 : Bits[41:35]
           Key 2 : Bits[48:42]
           Key 3 : Bits[55:49] */
        value = ReadBitsInArrray(readData, NLM11KDEV_REG_LEN_IN_BYTES, 55, 28);
        for(keyNum = 0; keyNum < NLM11KDEV_NUM_KEYS; keyNum++)
            rangeInsertData_p->m_rangeBInsertStartByte[keyNum] = (nlm_u8)((value >> (keyNum * 7)) & 0x7F);

       /* Location of RangeA Insert Field for various keys in Reg are as follows
           Key 0 : Bits[6:0]
           Key 1 : Bits[13:7]
           Key 2 : Bits[20:14]
           Key 3 : Bits[27:21] */
        value = ReadBitsInArrray(readData, NLM11KDEV_REG_LEN_IN_BYTES, 27, 0);
        for(keyNum = 0; keyNum < NLM11KDEV_NUM_KEYS; keyNum++)
            rangeInsertData_p->m_rangeAInsertStartByte[keyNum] = (nlm_u8)((value >> (keyNum * 7)) & 0x7F);
    }
    else
    {
         Nlm11kDevRangeInsertion1Reg *rangeInsertData_p =  (Nlm11kDevRangeInsertion1Reg*)outputData_p;

        /* Location of various Field in Reg are as follows
            Range D Encoding Type: Bits[63:62]
            Range C Encoding Type: Bits[61:60]
            Range D Encoding Bytes: Bits[59:58]
            Range C Encoding Bytes: Bits[57:56] */
        value = ReadBitsInArrray(readData, NLM11KDEV_REG_LEN_IN_BYTES, 63, 56);
        rangeInsertData_p->m_rangeDEncodingType = (value >> 6) & 0x3;
        rangeInsertData_p->m_rangeCEncodingType = (value >> 4) & 0x3;
        rangeInsertData_p->m_rangeDEncodedBytes = (value >> 2) & 0x3;
        rangeInsertData_p->m_rangeCEncodedBytes = value & 0x3;

        /* Location of RangeD Insert Field for various keys in Reg are as follows
           Key 0 : Bits[34:28]
           Key 1 : Bits[41:35]
           Key 2 : Bits[48:42]
           Key 3 : Bits[55:49] */
        value = ReadBitsInArrray(readData, NLM11KDEV_REG_LEN_IN_BYTES, 55, 28);
        for(keyNum = 0; keyNum < NLM11KDEV_NUM_KEYS; keyNum++)
            rangeInsertData_p->m_rangeDInsertStartByte[keyNum] = (nlm_u8)((value >> (keyNum * 7)) & 0x7F);

       /* Location of RangeC Insert Field for various keys in Reg are as follows
           Key 0 : Bits[6:0]
           Key 1 : Bits[13:7]
           Key 2 : Bits[20:14]
           Key 3 : Bits[27:21] */
        value = ReadBitsInArrray(readData, NLM11KDEV_REG_LEN_IN_BYTES, 27, 0);
        for(keyNum = 0; keyNum < NLM11KDEV_NUM_KEYS; keyNum++)
            rangeInsertData_p->m_rangeCInsertStartByte[keyNum] = (nlm_u8)((value >> (keyNum * 7)) & 0x7F);
    }
}

/* Nlm11kDevMgr_pvt_ConstructMiscLtrData contructs the 80b Reg Data based on various values
    of the various fields of Miscelleneous Reg provided by application */
static NlmErrNum_t Nlm11kDevMgr_pvt_ConstructMiscLtrData(
	nlm_u8						*o_data,
	Nlm11kDevMiscelleneousReg	*miscData_p
	)
{
    nlm_32 psNum;
    nlm_u32 value = 0;

    /* A Miscelleneous Reg contains such as which BMR should be used for each of the
    parallel searches and location of various Range Fields to be extracted
    from the Compare Key(Master Key) */
    for(psNum = 0; psNum < NLM11KDEV_NUM_PARALLEL_SEARCHES; psNum++)
    {
        /* Valid values for BMR Select is 0 - 4 and NLM11KDEV_NO_MASK_BMR_NUM */
        if(miscData_p->m_bmrSelect[psNum] >= NLM11KDEV_NUM_OF_BMRS_PER_BLK
            && miscData_p->m_bmrSelect[psNum] != NLM11KDEV_NO_MASK_BMR_NUM)
            return NLMERR_FAIL;

        /* Location of BMR Select Field for various parallel searches in Reg are as follows
           PS 0 : Bits[2:0]
           PS 1 : Bits[6:4]
           PS 2 : Bits[10:8]
           PS 3 : Bits[14:12] */
        value |= (miscData_p->m_bmrSelect[psNum] << (psNum * 4));
    }
    WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 14, 0, value);

    value = 0;
    /* Valid values for Range Extract Start Bytes is 0 - 78 */
    if(miscData_p->m_rangeAExtractStartByte >= (NLM11KDEV_MAX_KEY_LEN_IN_BYTES - 1)
        || miscData_p->m_rangeBExtractStartByte >= (NLM11KDEV_MAX_KEY_LEN_IN_BYTES - 1)
        || miscData_p->m_rangeCExtractStartByte >= (NLM11KDEV_MAX_KEY_LEN_IN_BYTES - 1)
        || miscData_p->m_rangeDExtractStartByte >= (NLM11KDEV_MAX_KEY_LEN_IN_BYTES - 1))
        return NLMERR_FAIL;

     /* Location of Range Extract Start Bytes Field for various range types in Reg are as follows
           Range A  : Bits[22:16]
           Range B  : Bits[30:24]
           Range C  : Bits[38:32]
           Range D  : Bits[46:40] */
    value = ((miscData_p->m_rangeDExtractStartByte << 24)
            | (miscData_p->m_rangeCExtractStartByte << 16)
            | (miscData_p->m_rangeBExtractStartByte << 8)
            | miscData_p->m_rangeAExtractStartByte);

    WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 46, 16, value);

    /* Valid values for Range Extract Start Bytes is 0 - 3 */
    if(miscData_p->m_numOfValidSrchRslts > 3)
         return NLMERR_FAIL;
    /* Number of valid search results occupy Bits [57:56] of the Reg  */
    WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 57, 56,
    					miscData_p->m_numOfValidSrchRslts);

	/* Search type of each parallel search, Bits[55:48] (PHMD)
	  * PS#0 : Bits[49:48]
	  * PS#1 : Bits[51:50]
	  * PS#2 : Bits[53:52]
	  * PS#2 : Bits[55:54]
	  */
	for(value = 0, psNum = 0; psNum < NLM11KDEV_NUM_PARALLEL_SEARCHES; psNum++)
	{
		if(miscData_p->m_searchType[psNum] != NLM11KDEV_STANDARD &&
			miscData_p->m_searchType[psNum] != NLM11KDEV_SAHASRA)
			return NLMERR_FAIL;

		/* Each PS requires 2 bits and hence value 2. */
		value |= ( miscData_p->m_searchType[psNum] << (psNum * 2) );
	}
	WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES,
						NLM11KDEV_MISCREG_SEARCH_END_BIT,
						NLM11KDEV_MISCREG_SEARCH_START_BIT, value);

    return NLMERR_OK;
}

/* Nlm11kDevMgr_pvt_ExtractMiscLtrData extracts the various fields  of Miscelleneous Reg
from the 80b Reg Data read from the device */
static void Nlm11kDevMgr_pvt_ExtractMiscLtrData(
	nlm_u8						*readData,
	Nlm11kDevMiscelleneousReg	*miscData_p
	)
{
    nlm_32 psNum;
    nlm_u32 value = 0;

    /* Location of BMR Select Field for various parallel searches in Reg are as follows
           PS 0 : Bits[2:0]
           PS 1 : Bits[6:4]
           PS 2 : Bits[10:8]
           PS 3 : Bits[14:12] */
    value = ReadBitsInArrray(readData, NLM11KDEV_REG_LEN_IN_BYTES, 14, 0);
    for(psNum = 0; psNum < NLM11KDEV_NUM_PARALLEL_SEARCHES; psNum++)
        miscData_p->m_bmrSelect[psNum] = (nlm_u8)((value >> (psNum * 4)) & 0x7);

    /* Location of Range Extract Start Bytes Field for various range types in Reg are as follows
           Range A  : Bits[22:16]
           Range B  : Bits[30:24]
           Range C  : Bits[38:32]
           Range D  : Bits[46:40] */
    value = ReadBitsInArrray(readData, NLM11KDEV_REG_LEN_IN_BYTES, 46, 16);
    miscData_p->m_rangeDExtractStartByte = (nlm_u8)((value >> 24) & 0x7F);
    miscData_p->m_rangeCExtractStartByte = (nlm_u8)((value >> 16) & 0x7F);
    miscData_p->m_rangeBExtractStartByte = (nlm_u8)((value >> 8) & 0x7F);
    miscData_p->m_rangeAExtractStartByte = (nlm_u8)(value & 0x7F);

    /* Number of valid search results occupy Bits [57:56] of the Reg  */
    miscData_p->m_numOfValidSrchRslts = (nlm_u8)(ReadBitsInArrray(readData,
                                            NLM11KDEV_REG_LEN_IN_BYTES, 57, 56));

	/* Search type of each parallel search, Bits[55:48] (PHMD)
	  * PS#0 : Bits[49:48]
	  * PS#1 : Bits[51:50]
	  * PS#2 : Bits[53:52]
	  * PS#2 : Bits[55:54]
	  */
	value = ReadBitsInArrray(readData, NLM11KDEV_REG_LEN_IN_BYTES,
								NLM11KDEV_MISCREG_SEARCH_END_BIT,
								NLM11KDEV_MISCREG_SEARCH_START_BIT);

	for(psNum = 0; psNum < NLM11KDEV_NUM_PARALLEL_SEARCHES; psNum++)
	{
		/* Each PS requires 2 bits and hence value 2. */
		miscData_p->m_searchType[psNum] = ( (value >> (psNum * 2)) & 0x3 );
	}

}

static NlmErrNum_t Nlm11kDevMgr_pvt_ConstructSSLtrData(
	nlm_u8				*o_data,
    Nlm11kDevSSReg	*SSData_p
	)
{
    nlm_u8 idx;
    nlm_u32 value = 0;

    for(idx = 0; idx < NLM11KDEV_SS_RMP_AB; idx++)
    {
        if( (SSData_p->m_ss_result_map[idx]  != NLM11KDEV_MAP_PE0) &&
        	 (SSData_p->m_ss_result_map[idx] != NLM11KDEV_MAP_PE1) )
            return NLMERR_FAIL;

        /* Bits[16:9] represent the SS result map in the register. */
        value |= (SSData_p->m_ss_result_map[idx] << idx);
    }
    WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES,
    					NLM11KDEV_SSREG_SSMAP_END_BIT,
    					NLM11KDEV_SSREG_SSMAP_START_BIT, value);

    return NLMERR_OK;
}

static void Nlm11kDevMgr_pvt_ExtractSSLtrData(
	nlm_u8				*readData,
    Nlm11kDevSSReg	*SSData_p
	)
{
    nlm_u8 idx;
    nlm_u32 value = 0;

    /* Bits[16:9] represent the SS result map in the register. */
    value = ReadBitsInArrray(readData, NLM11KDEV_REG_LEN_IN_BYTES,
    							NLM11KDEV_SSREG_SSMAP_END_BIT,
    							NLM11KDEV_SSREG_SSMAP_START_BIT);

    for(idx = 0; idx < NLM11KDEV_SS_RMP_AB; idx++)
        SSData_p->m_ss_result_map[idx] = (nlm_u8)((value >> idx) & 0x1);

}

static void Nlm11kDevMgr_pvt_ConstructRangeBoundRegData(
	nlm_u8				*o_data,
    nlm_u8          	*i_data
	)
{
    nlm_u32 value = 0;

	/* copy Bits[0-31] */    
	value = ReadBitsInArrray(i_data, NLM11KDEV_REG_LEN_IN_BYTES, 31, 0);
    WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 31, 0, value);

	/* copy Bits[32-33] */	
	value = ReadBitsInArrray(i_data, NLM11KDEV_REG_LEN_IN_BYTES, 33, 32);
    WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 33, 32, value);

    return;
}

static void Nlm11kDevMgr_pvt_ConstructRangeCodeRegData(
	nlm_u8				*o_data,
    nlm_u8          	*i_data
	)
{
    nlm_u32 value = 0;

	/* copy Bits[0-31] */
    value = ReadBitsInArrray(i_data, NLM11KDEV_REG_LEN_IN_BYTES, 31, 0);
    WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 31, 0, value);

	/* copy Bits[32-47] */
	value = ReadBitsInArrray(i_data, NLM11KDEV_REG_LEN_IN_BYTES, 47, 32);
    WriteBitsInArray(o_data, NLM11KDEV_REG_LEN_IN_BYTES, 47, 32, value);

    return;
}

/*
	Function : Nlm11kDevMgr__SendNop	
	Description: Nlm11kDevMgr__SendNop (NOP) pass NOP instruction to the 
	device for specified count (number of times).
	User should see the reason code in case of failure.
*/
NlmErrNum_t Nlm11kDevMgr__SendNop(
	Nlm11kDev      		*dev,
	nlm_u32				numTimes,
	NlmReasonCode*		o_reason
	)
{
    NlmXptRqt* rqt_p = NULL;

	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
		
	while(numTimes)
	{
		/* preparing write request for NOP */
		rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
		if(NULL == rqt_p)
		{
			if(o_reason)
				*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
			return NLMERR_NULL_PTR;
		}
		/* Clearing the rqt structure */
		NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

		/* assigning request structure members */
		rqt_p->m_opcode[0] = NLM11K_OPCODE_NOP_BITS_8_6;
		rqt_p->m_opcode[1] = NLM11K_OPCODE_NOP_BITS_5_0;
	
		/* data length 10Byte */
		rqt_p->m_data_len = NLM11KDEV_AB_WIDTH_IN_BYTES; 

		/* In case of Database Write; "result" field of xpt rqt will be NULL */
		rqt_p->m_result = NULL;	
		
		/* calling transport layer to work on the current request */
		NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
		if(o_reason)
		{
			if(NLMRSC_REASON_OK != *o_reason)
				return NLMERR_FAIL;
		}
		--numTimes;
	}
	return NLMERR_OK;
}

/* destroys shadow device memory */
NlmErrNum_t Nlm11kDevMgr__ShadowDeviceDestroy(
    Nlm11kDev		*dev_p,
    NlmReasonCode	*o_reason
	)
{
    Nlm11kDevShadowDevice *shadowDev_p;
    NlmCmAllocator *alloc_p;

    if(!NLM11K_IS_VALID_DEV_PTR(dev_p))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}

    alloc_p = dev_p->m_devMgr_p->m_alloc_p;
    shadowDev_p = dev_p->m_shadowDevice_p;

    if(shadowDev_p)
    {
        if(shadowDev_p->m_arrayBlock)
		    NlmCmAllocator__free(alloc_p, shadowDev_p->m_arrayBlock);

        if(shadowDev_p->m_ltr)
		    NlmCmAllocator__free(alloc_p, shadowDev_p->m_ltr);

        if(shadowDev_p->m_st)
			NlmCmAllocator__free(alloc_p, shadowDev_p->m_st);

		if(shadowDev_p->m_rangeReg)
			NlmCmAllocator__free(alloc_p, shadowDev_p->m_rangeReg);

        NlmCmAllocator__free(alloc_p, shadowDev_p);
    }
    dev_p->m_shadowDevice_p = NULL;

    return NLMERR_OK;
}


/*creates shadow device - allocates all memory for shadow device structure */
NlmErrNum_t Nlm11kDevMgr__ShadowDeviceCreate(
    Nlm11kDev		*dev_p,
    NlmReasonCode	*o_reason
    )
{
	Nlm11kDevShadowDevice *shadowDev_p;
    NlmCmAllocator *alloc_p;
    nlm_32 abNum;
    nlm_32 entryNum;

    if(!NLM11K_IS_VALID_DEV_PTR(dev_p))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    alloc_p = dev_p->m_devMgr_p->m_alloc_p;

    if(dev_p->m_shadowDevice_p == NULL)
    {
        /* Creating the memory for shadow device */
        dev_p->m_shadowDevice_p = NlmCmAllocator__calloc(alloc_p, 1, sizeof(Nlm11kDevShadowDevice));
        if(dev_p->m_shadowDevice_p == NULL)
        {
            if(o_reason)
                *o_reason = NLMRSC_LOW_MEMORY;
            return NLMERR_FAIL;
        }

        shadowDev_p = dev_p->m_shadowDevice_p;
        /* Creating the memory for shadow Array Blocks */
        shadowDev_p->m_arrayBlock = NlmCmAllocator__calloc(alloc_p,
                                                       NLM11KDEV_NUM_ARRAY_BLOCKS,
                                                       sizeof(Nlm11kDevShadowAB));

        if(shadowDev_p->m_arrayBlock == NULL)
        {
            Nlm11kDevMgr__ShadowDeviceDestroy(dev_p, NULL);
            if(o_reason)
                *o_reason = NLMRSC_LOW_MEMORY;
            return NLMERR_FAIL;
        }

        for(abNum = 0; abNum < NLM11KDEV_NUM_ARRAY_BLOCKS; abNum++)
        {
            /* All the data and mask bytes of all the entries are initialized to have initial value as 0xFF */
            for(entryNum = 0; entryNum < NLM11KDEV_AB_DEPTH; entryNum++)
                NlmCm__memset(&shadowDev_p->m_arrayBlock[abNum].m_abEntry[entryNum], 0xFF,
                    2 * NLM11KDEV_AB_WIDTH_IN_BYTES);
        }

        /* Creating the memory for shadow LTR */
        shadowDev_p->m_ltr = NlmCmAllocator__calloc(alloc_p,
                                                    NLM11KDEV_NUM_LTR_SET,
                                                    sizeof(Nlm11kDevShadowLtr));

    	if (shadowDev_p->m_ltr == NULL)
        {
            Nlm11kDevMgr__ShadowDeviceDestroy(dev_p, NULL);
            if(o_reason)
                *o_reason = NLMRSC_LOW_MEMORY;
            return NLMERR_FAIL;
        }

        /* Create memory for ST. */
		shadowDev_p->m_st = NlmCmAllocator__calloc(alloc_p,
													NLM11KDEV_SS_NUM,
													sizeof(Nlm11kDevShadowST) );

		if (shadowDev_p->m_st == NULL)
		{
			Nlm11kDevMgr__ShadowDeviceDestroy(dev_p, NULL);
			if(o_reason)
				*o_reason = NLMRSC_LOW_MEMORY;

			return NLMERR_FAIL;
		}

		/*Create memory for range registers */
		shadowDev_p->m_rangeReg = NlmCmAllocator__calloc(alloc_p,
															NLM11KDEV_NUM_RANGE_REG,
															sizeof(Nlm11kDevRangeReg));

		if(shadowDev_p->m_rangeReg == NULL)
		{
			Nlm11kDevMgr__ShadowDeviceDestroy(dev_p, NULL);
			if(o_reason)
				*o_reason = NLMRSC_LOW_MEMORY;

			return NLMERR_FAIL;
		}

    }

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;
    return NLMERR_OK;
}

/*creates shadow device - allocates all memory for 10M shadow device structure */
NlmErrNum_t Nlm11kDevMgr__ShadowABReCreate(
    Nlm11kDev		*dev_p,
    NlmReasonCode	*o_reason
    )
{
	Nlm11kDevShadowDevice *shadowDev_p;
    NlmCmAllocator *alloc_p;
    nlm_32 abNum;
    nlm_32 entryNum;

    if(!NLM11K_IS_VALID_DEV_PTR(dev_p))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    alloc_p = dev_p->m_devMgr_p->m_alloc_p;

    if(dev_p->m_shadowDevice_p != NULL)
    {
        shadowDev_p = dev_p->m_shadowDevice_p;

		/* release memory allocated for 40M array */
		if(shadowDev_p->m_arrayBlock)
		    NlmCmAllocator__free(alloc_p, shadowDev_p->m_arrayBlock);

        /* Creating the memory for shadow Array Blocks for 10M */
        shadowDev_p->m_arrayBlock = NlmCmAllocator__calloc(alloc_p,
                                                       NLM11KDEV_10M_NUM_ARRAY_BLOCKS,
                                                       sizeof(Nlm11kDevShadowAB));

        if(shadowDev_p->m_arrayBlock == NULL)
        {
		    NlmCmAllocator__free(alloc_p, shadowDev_p->m_arrayBlock);
            if(o_reason)
                *o_reason = NLMRSC_LOW_MEMORY;
            return NLMERR_FAIL;
        }

        for(abNum = 0; abNum < NLM11KDEV_10M_NUM_ARRAY_BLOCKS; abNum++)
        {
            /* All the data and mask bytes of all the entries are initialized to have initial value as 0xFF */
            for(entryNum = 0; entryNum < NLM11KDEV_AB_DEPTH; entryNum++)
                NlmCm__memset(&shadowDev_p->m_arrayBlock[abNum].m_abEntry[entryNum], 0xFF,
                    2 * NLM11KDEV_AB_WIDTH_IN_BYTES);
        }        
    }
	else
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_SHADOWDEV_PTR;
		
		return NLMERR_FAIL;
	}

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;
    return NLMERR_OK;
}
/*
Function: Nlm11kDevMgr__create
Description: creates Device manager using memory allocator and returns the pointer
of the Device manager. memory allocator must be passed as parameter. Function returns
NULL if fails and user should see the reason code in case of failure.
*/
 Nlm11kDevMgr* Nlm11kDevMgr__create(
	NlmCmAllocator			*alloc_p,
	void					*xpt_p,
    Nlm11kDev_OperationMode operMode,
	NlmReasonCode			*o_reason
	)
{
	Nlm11kDevMgr *self = NULL;

    if(alloc_p == NULL)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_MEMALLOC_PTR;
		return NULL;
	}
	if(xpt_p == NULL)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_PTR;
		return NULL;
	}
    if( (operMode != NLM11KDEV_OPR_STANDARD) &&
    	(operMode != NLM11KDEV_OPR_SAHASRA) )
    {
        if(o_reason)
			*o_reason = NLMRSC_INVALID_INPUT;
		return NULL;
    }

    /* Allocate memory for Device Manager */
    self = NlmCmAllocator__malloc(alloc_p, sizeof(Nlm11kDevMgr));
    NlmCmDemand((self != NULL), "Out of memory.\n");
    if(self == NULL)
	{
		if(o_reason)
			*o_reason = NLMRSC_LOW_MEMORY;
		return NULL;
	}
    /* Initialize members of Device Manager */
    self = Nlm11kDevMgr__ctor(self, xpt_p, operMode, alloc_p);

	g_devMgr_p = self;

#if defined NLM_MT_OLD || defined NLM_MT

    {
        nlm_32 ret;
	ret = NlmCmMt__SpinInit(&self->m_spinLock, 
				"Nlm11kDevMgr_Kbp_SpinLock",
				NlmCmMtFlag);
        if(ret != 0)
        {
            *o_reason = NLMRSC_MT_SPINLOCK_INIT_FAILED;

            return NULL;
        }
    }
#endif

	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;
    return self;
}

/*
Function: Nlm11kDevMgr__destroy
Description: destroys the instance of Device Manager and all devices in the Device Manager list
*/
 void Nlm11kDevMgr__destroy(
    Nlm11kDevMgr		*self
	)
{
	NlmCmAllocator *alloc_p;

    if (self != NULL)
	{
		alloc_p = self->m_alloc_p;
		Nlm11kDevMgr__dtor(self);

#if defined NLM_MT_OLD || defined NLM_MT

		NlmCmMt__SpinDestroy( &self->m_spinLock, "Nlm11kDevMgr_Kbp_SpinLock");
#endif
		NlmCmAllocator__free(alloc_p, self);
    }

	g_devMgr_p = NULL;
}

/*
Function: Nlm1kDevMgr__AddDevice
Description: creates a new device and adds to device list of Device Manager.
If this function fails it returns NULL. User should see the reason code in case of failure.
*/
Nlm11kDev* Nlm11kDevMgr__AddDevice(
	Nlm11kDevMgr			*self,
	Nlm11kDevId				*o_devId,
	NlmReasonCode			*o_reason
	)
{
	Nlm11kDev* dev_p = NULL;
    Nlm11kDev   **devList_pp;

	/* return NULL if there is no Device Manager. */
	if(!NLM11K_IS_VALID_DEVMGR_PTR(self))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEVMGR_PTR;
		return NULL;
	}
    /* see, if Device Manager config is locked or not */
	if(NlmTrue == self->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_LOCKED;
		return NULL;
	}
    if(self->m_devCount == NLM11KDEV_MAX_DEV_NUM)
    {
        if(o_reason)
			*o_reason = NLMRSC_INVALID_NUM_OF_DEVICES;
		return NULL;
    }
    if(o_devId == NULL)
    {
         if(o_reason)
			*o_reason = NLMRSC_INVALID_OUTPUT;
		return NULL;
    }

	/* creating a new device here. */
	dev_p = NlmCmAllocator__calloc(self->m_alloc_p, 1, sizeof(Nlm11kDev));
    NlmCmDemand((dev_p != NULL), "Out of memory.\n");
    if(dev_p == NULL)
	{
		if(o_reason)
			*o_reason = NLMRSC_LOW_MEMORY;
		return NULL;
	}

    /* Initializing the device members */
    dev_p->m_devId = *o_devId = self->m_devCount;
    dev_p->m_devMgr_p = self;
    dev_p->m_magicNum = NLM11K_DEV_PTR_MAGIC_NUM;

    /* creating shadow device */
    if(NLMERR_OK != Nlm11kDevMgr__ShadowDeviceCreate(dev_p, o_reason))
    {
        NlmCmAllocator__free(self->m_alloc_p, dev_p);
        return NULL;
    }

	/* storing the device pointer into device list */
    devList_pp = (Nlm11kDev**)self->m_devList_pp;
	devList_pp[dev_p->m_devId] = dev_p;
    self->m_devCount++;

    return dev_p;
}

/*
Function: Nlm11kDevMgr__ResetDevices
Description: Resets all the devices in cascade by calling the XPT API which resets the devices
Note: This API does not reset the Shadow memory data to initial values; If application is
      interested in doing so it should invoke Nlm11kDevMgr__ShadowDeviceDestroy API followed
      by Nlm11kDevMgr__ShadowDeviceCreate API
*/
NlmErrNum_t Nlm11kDevMgr__ResetDevices(
	Nlm11kDevMgr		*self,
	NlmReasonCode		*o_reason
	)
{
    if(!NLM11K_IS_VALID_DEVMGR_PTR(self))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_NULL_PTR;
    }

    /* Call the XPT Function which resets the devices in cascade */
    return(NlmXpt__ResetDevice(self->m_xpt_p, o_reason));
}

/*
Function: Nlm11kDevMgr__LockConfig
Description: Locks the Device Manager configurations; No more devices can be
    added after Lock config
*/
NlmErrNum_t Nlm11kDevMgr__LockConfig(
	Nlm11kDevMgr		*self,
	NlmReasonCode		*o_reason
	)
{
	NlmErrNum_t err = NLMERR_OK;

	if(!NLM11K_IS_VALID_DEVMGR_PTR(self))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEVMGR_PTR;
		return NLMERR_NULL_PTR;
	}
	if(NlmTrue == self->m_isLocked)
        return NLMERR_OK;

#ifdef NLM_12K_11K 

	self->m_isLocked = NlmTrue;
#else

	/* calling transport layer lock configuration */
	err = NlmXpt__LockConfig(self->m_xpt_p, self->m_devCount, o_reason);

	if(NLMERR_OK == err)
	{
		self->m_isLocked = NlmTrue;
		
		/* CBInit() will write 0's to all CB */
	    /* err = Nlm11kDevMgr__pvt_CtxBufferInit(self, o_reason);	*/ 

#ifdef NLM_10M_DEV
		{        
			Nlm11kDevIdReg devIdRegData;
		
			/* Read the Global Register Device Identification Regsiter (DIR)
			if the DBSize value is 3 = 40M device, if 2 = 10M device */

			if( (err = Nlm11kDevMgr__GlobalRegisterRead(((Nlm11kDev*)self->m_devList_pp[0]), 
				NLM11KDEV_DEVICE_ID_REG, &devIdRegData, o_reason) ) != NLMERR_OK )
			{
				NlmCm__printf("\n\t .. Unable to read the Device Identification Regsiter .. \n\n");
				return err;
			}

			/* if database size is 3 = 40M device, and if database size is 2 = 10M device */
			if(devIdRegData.m_databaseSize == 0x2)
			{ 
				self->m_is10MDev = NlmTrue;
				g_is10MDev       = self->m_is10MDev;  /* Using 10M device */

				/* re-allocate memory for 10M device AB blocks */
				{
					nlm_u8 dev = 0;

					for(dev = 0; dev < self->m_devCount; dev++)
					{
						if( (err = Nlm11kDevMgr__ShadowABReCreate(self->m_devList_pp[dev], o_reason) ) != NLMERR_OK)
						{
							NlmCm__printf("\n\t .. Nlm11kDevMgr__ShadowABReCreate function failed .. \n\n");
							return err;
						}
					}
				}
			}
		}
#endif

	}	
#endif
	return err;
}

/*
Function: Nlm11kDevMgr__BlockRegisterWrite
Description: writes to block register of specified abNum depending on block register type.
An appopriate structure based on Reg Type needs to be passed as *data.
For BCR -- structure to be used Nlm11kDevBlockConfigReg
For BMR -- structure to be used Nlm11kDevBlkMaskReg
User should see the reason code in case of failure.
*/
 NlmErrNum_t Nlm11kDevMgr__BlockRegisterWrite(
	Nlm11kDev				*dev,
	nlm_u8					abNum,		/* AB number in which register lies */
	Nlm11kDevBlockRegType	regType,	/* see the enum description */
	const void 				*data,		/*appropriate structure pointer */
	NlmReasonCode			*o_reason
	)
{
	NlmXptRqt* rqt_p = NULL;
	nlm_32     address =0;
	nlm_u8 regData[NLM11KDEV_REG_LEN_IN_BYTES] = "";
	nlm_u8 *data_ptr = NULL;
	nlm_u8 numBlocks = NLM11KDEV_NUM_ARRAY_BLOCKS;

	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(g_is10MDev)
		numBlocks = NLM11KDEV_10M_NUM_ARRAY_BLOCKS;
		
	if(NULL == data)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DATA;
		return NLMERR_NULL_PTR;
	}
	if(abNum >= numBlocks)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_AB_NUM;
		return NLMERR_FAIL;
	}
    if(regType >= NLM11KDEV_BLOCK_REG_END)
    {
        if(o_reason)
			*o_reason = NLMRSC_INVALID_REG_ADDRESS;
		return NLMERR_FAIL;
    }

#ifdef NLM_PRE_RA12
	/* prepare request for NOP; before BCR register write we will pass NOPs (Errata) */
	if( NLMERR_OK != Nlm11kDevMgr__SendNop(dev, NLM11KDEV_NUM_OF_NOPS_FOR_REG, o_reason))
	{
		return NLMERR_FAIL;
	}
#endif

	/* preparing write request for register. */
    rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_WRITE_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_WRITE_BITS_5_0;

    /* length of the register data */
	rqt_p->m_data_len = NLM11KDEV_REG_LEN_IN_BYTES;

    /* In case of Reg Write; "result" field of xpt rqt will be NULL */
	rqt_p->m_result = NULL;

	/* Find the single device address of the corresponding register_type
            Addresses of Block Registers of a block are continoues based on RegType;
            Get the base address (BCR Address) and then add regtype to it
          */
	address = NLM11K_REG_ADDR_BLK_CONFIG(abNum) + regType;

    /* Insert the devid into the address */
    address = NLM11K_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
	rqt_p->m_address = address;

	/* pointer to the register data */
	data_ptr = rqt_p->m_data_p = regData;
#else
	(void)regData;

	/* In case of Reg write, Byte[3:0] of the "data" field of xpt rqt
     will contain the address and Byte[13:4] will contain the register data */
    WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_REG_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
                address
				);
	rqt_p->m_data_len += NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

	/* pointer to the register data : Byte[13:4] */
	data_ptr = rqt_p->m_data + NLM11KDEV_REG_ADDR_LEN_IN_BYTES;
#endif

    /* writing register contents into data field of request structure */
    if(regType == NLM11KDEV_BLOCK_CONFIG_REG)
    {
        Nlm11kDevBlockConfigReg *blkConfigData_p;
        nlm_u8 blkRegData;

        /* For BCR Bits[3:1] represents Blk Width and Bit[0] represents Blk enable */
        blkConfigData_p = (Nlm11kDevBlockConfigReg*)data;
        if((blkConfigData_p->m_blockEnable !=  NLM11KDEV_DISABLE
             && blkConfigData_p->m_blockEnable !=  NLM11KDEV_ENABLE)
            || (blkConfigData_p->m_blockWidth != NLM11KDEV_BLK_WIDTH_80
             && blkConfigData_p->m_blockWidth != NLM11KDEV_BLK_WIDTH_160
             && blkConfigData_p->m_blockWidth != NLM11KDEV_BLK_WIDTH_320
             && blkConfigData_p->m_blockWidth != NLM11KDEV_BLK_WIDTH_640))
        {
            /* If any input param is not correct; reset the requests and return error */
            NlmXpt__ResetRequests(dev->m_devMgr_p->m_xpt_p, NULL);
            if(o_reason)
			    *o_reason = NLMRSC_INVALID_INPUT;
            return NLMERR_FAIL;
        }

        blkRegData = (nlm_u8)((blkConfigData_p->m_blockWidth << 1) | (blkConfigData_p->m_blockEnable));
		WriteBitsInArray(data_ptr,
                         NLM11KDEV_REG_LEN_IN_BYTES,
                         3, 0, blkRegData);
    }
    else
    {
        Nlm11kDevBlockMaskReg *blkMaskData_p;

        /* For BMR copy the 80b data passed with the structure */
        blkMaskData_p = (Nlm11kDevBlockMaskReg*)data;

#ifdef NLM_NO_MEMCPY_IN_XPT
		/* request pointer will point to the Block mask data to be written */
		rqt_p->m_data_p = blkMaskData_p->m_mask;

#else
		/* copy the register data to the request structure */
		NlmCm__memcpy(data_ptr,
                blkMaskData_p->m_mask,
                NLM11KDEV_REG_LEN_IN_BYTES);
#endif
    }

	/* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
	if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
			return NLMERR_FAIL;
	}
	return NLMERR_OK;

}

/*
Function: Nlm11kDevMgr__BlockRegisterRead
Description: Reads from block register of specified abNum depending on block register type.
An appopriate structure based on Reg Type needs to be passed as *data;
This function will update the fields of this structure based on data read from device
For BCR -- structure to be used Nlm11kDevBlockConfigReg
For BMR -- structure to be used Nlm11kDevBlkMaskReg
User should see the reason code in case of failure.
*/
 NlmErrNum_t Nlm11kDevMgr__BlockRegisterRead(
	Nlm11kDev				*dev,
	nlm_u8					abNum,		/* AB number in which register lies */
	Nlm11kDevBlockRegType	regType,	/* see the enum description */
	void 					*o_data,	/*appropriate structure pointer */
	NlmReasonCode*			o_reason
	)
{
	NlmXptRqt* rqt_p = NULL;
	nlm_32     address = 0;
    nlm_u8 readData[NLM11KDEV_REG_LEN_IN_BYTES + 1]; /* Extra byte is for control bits
                                                     such as VBIT, parity */
	nlm_u8 numBlocks = NLM11KDEV_NUM_ARRAY_BLOCKS;

	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(NULL == o_data)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_OUTPUT;
		return NLMERR_NULL_PTR;
	}

	if(g_is10MDev)
		numBlocks = NLM11KDEV_10M_NUM_ARRAY_BLOCKS;

	if(abNum >= numBlocks)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_AB_NUM;
		return NLMERR_FAIL;
	}
    if(regType >= NLM11KDEV_BLOCK_REG_END)
    {
        if(o_reason)
			*o_reason = NLMRSC_INVALID_REG_ADDRESS;
		return NLMERR_FAIL;
    }
	/* preparing request for register read. */
    rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_READ_X_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_READ_X_BITS_5_0;

    rqt_p->m_data_len = NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

    /* In case of Reg read; XPT layer send the 80b read data via "result" field of xpt rqt;
    In addition to this result will contain some control bits such as vbit and parity which are ignored */
	rqt_p->m_result = readData;

	/* find the single device address of the corresponding register_type */
    /* Addresses of Block Registers of a block are continoues based on RegType;
        Get the base address (BCR Address) and then add regtype to it*/
	address = NLM11K_REG_ADDR_BLK_CONFIG(abNum) + regType;

    /* Insert the devid into the address */
    address = NLM11K_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
	rqt_p->m_address = address;
#else
	/* In case of Reg Read, Byte[3:0] of the "data" field of xpt rqt will contain the address*/
	WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_REG_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
                address
				);
#endif

    /* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
	if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            NlmXpt__DiscardResult(dev->m_devMgr_p->m_xpt_p, rqt_p, NULL);
			return NLMERR_FAIL;
        }
	}

    /* calling transport layer to get the results of the current request */
    if(NULL == NlmXpt__GetResult(dev->m_devMgr_p->m_xpt_p, rqt_p, NULL))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
		return NLMERR_NULL_PTR;
	}

    /* extracting register contents from result field of request structure */
    if(regType == NLM11KDEV_BLOCK_CONFIG_REG)
    {
        Nlm11kDevBlockConfigReg *blkConfigData_p;
        nlm_u8 blkRegData;

        /* For BCR Bits[3:1] represents Blk Width and Bit[0] represents Blk enable */
        blkRegData = (nlm_u8)(ReadBitsInArrray(readData + 1, /* Ignoring Byte 0 for control fields */
                                               NLM11KDEV_REG_LEN_IN_BYTES,
                                               3, 0));
        blkConfigData_p = (Nlm11kDevBlockConfigReg*)o_data;
        blkConfigData_p->m_blockEnable = blkRegData & 0x1;
        blkConfigData_p->m_blockWidth = (blkRegData >> 1) & 0x7;
    }
    else
    {
        Nlm11kDevBlockMaskReg *blkMaskData_p;

        /* For BMR copy the 80b read data to the structure */
        blkMaskData_p = (Nlm11kDevBlockMaskReg*)o_data;
        NlmCm__memcpy(blkMaskData_p->m_mask,
                      readData + 1,  /* Ignoring Byte 0 for control fields */
                      NLM11KDEV_REG_LEN_IN_BYTES);
    }

	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;
	return NLMERR_OK;

}


/*
Function: Nlm11kDevMgr__LogicalTableRegisterWrite
Description: writes to ltr register of specified ltrNum and specified ltr register type.
An appopriate structure based on Reg Type needs to be passed as *data.
For Blk Select Reg -- structure to be used Nlm11kDevBlkSelectReg
For Super Blk Key Select Reg -- structure to be used Nlm11kDevSuperBlkKeyMapReg
For Parallel Srch Reg -- structure to be used Nlm11kDevParallelSrchReg
For Range Insert 0 Reg -- structure to be used Nlm11kDevRangeInsertion0Reg
For Range Insert 1 Reg -- structure to be used Nlm11kDevRangeInsertion1Reg
For Miscelleneous Reg -- structure to be used Nlm11kDevMiscelleneousReg
For Key Construction Reg -- structure to be used Nlm11kDevKeyConstructReg
User should see the reason code in case of failure.

Note: Advanced Srch LTR is not yet supported
*/
 NlmErrNum_t Nlm11kDevMgr__LogicalTableRegisterWrite(
	Nlm11kDev 				*dev,
	nlm_u8					ltrNum,		/* LTR profile set number */
	Nlm11kDevLtrRegType	 	regType,	/* see the structure description above */
	const void				*data,		/* LTR register type structure pointer */
	NlmReasonCode*			o_reason
	)
 {
	NlmXptRqt* rqt_p = NULL;
	nlm_32     address =0;
    NlmErrNum_t errNum = NLMERR_OK;
    nlm_u8 regData[NLM11KDEV_REG_LEN_IN_BYTES] = "";
	nlm_u8 *data_ptr = NULL;

	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(NULL == data)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DATA;
		return NLMERR_NULL_PTR;
	}
	if(ltrNum >= NLM11KDEV_NUM_LTR_SET)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_LTR_NUM;
		return NLMERR_FAIL;
	}
	if(g_is10MDev)
	{
		/* Following registers are not configurable when 10M device is used */
		if(regType == NLM11KDEV_BLOCK_SELECT_1_LTR ||
		   regType == NLM11KDEV_PARALLEL_SEARCH_1_LTR ||
		   regType == NLM11KDEV_PARALLEL_SEARCH_2_LTR ||
		   regType == NLM11KDEV_PARALLEL_SEARCH_3_LTR	)
		{
			if(o_reason)
				*o_reason = NLMRSC_INVALID_REG_ADDRESS;
			return NLMERR_FAIL;
		}
	}
    if(regType >= NLM11KDEV_LTR_REG_END)
    {
        if(o_reason)
			*o_reason = NLMRSC_INVALID_REG_ADDRESS;
		return NLMERR_FAIL;
    }
	/* preparing request for register write. */
    rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_WRITE_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_WRITE_BITS_5_0;

    /* length of the register data */
	rqt_p->m_data_len = NLM11KDEV_REG_LEN_IN_BYTES;

	/* In case of Reg Write; "result" field of xpt rqt will be NULL */
	rqt_p->m_result = NULL;

	/* find the single device address of the corresponding register_type */
    /* Addresses of LTR Registers of specified ltrNum are continoues based on RegType;
        Get the base address of specified LtrNum(Blk Select 0 Address) and then add regtype to it*/
	address = NLM11K_REG_ADDR_LTR_BLOCK_SELECT0(ltrNum) + regType;

    /* Insert the devid into the address */
    address = NLM11K_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
	rqt_p->m_address = address;

	/* pointer to the register data */
	data_ptr = rqt_p->m_data_p = regData;

#else
	(void)regData;

	/* In case of Reg write, Byte[3:0] of the "data" field of xpt rqt
     will contain the address and Byte[13:4] will contain the 80b register data */
    rqt_p->m_data_len += NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

	/* Byte[3:0] of the "data" field of xpt rqt will contain the address*/
	WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_REG_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
                address
				);

	/* pointer to the register data : Byte[13:4]*/
	data_ptr = rqt_p->m_data + NLM11KDEV_REG_ADDR_LEN_IN_BYTES;
#endif

    /* writing register contents into data field of request structure */
    switch(regType)
    {
        case NLM11KDEV_BLOCK_SELECT_0_LTR:
        case NLM11KDEV_BLOCK_SELECT_1_LTR:
             errNum = Nlm11kDevMgr_pvt_ConstructBlkSelLtrData(data_ptr, (void*)data);
             break;

        case NLM11KDEV_SUPER_BLK_KEY_MAP_LTR:
             errNum = Nlm11kDevMgr_pvt_ConstructSBKeySelLtrData(data_ptr, (void*)data);
             break;

        case NLM11KDEV_PARALLEL_SEARCH_0_LTR:
        case NLM11KDEV_PARALLEL_SEARCH_1_LTR:
        case NLM11KDEV_PARALLEL_SEARCH_2_LTR:
        case NLM11KDEV_PARALLEL_SEARCH_3_LTR:
             errNum = Nlm11kDevMgr_pvt_ConstructParallelSrchLtrData(data_ptr, (void*)data);
             break;

        case NLM11KDEV_KEY_0_KCR_0_LTR:
        case NLM11KDEV_KEY_0_KCR_1_LTR:
        case NLM11KDEV_KEY_1_KCR_0_LTR:
        case NLM11KDEV_KEY_1_KCR_1_LTR:
        case NLM11KDEV_KEY_2_KCR_0_LTR:
        case NLM11KDEV_KEY_2_KCR_1_LTR:
        case NLM11KDEV_KEY_3_KCR_0_LTR:
        case NLM11KDEV_KEY_3_KCR_1_LTR:
             errNum = Nlm11kDevMgr_pvt_ConstructKeyContructLtrData(data_ptr, (void*)data);
             break;

        case NLM11KDEV_RANGE_INSERTION_0_LTR:
        case NLM11KDEV_RANGE_INSERTION_1_LTR:
             errNum = Nlm11kDevMgr_pvt_ConstructRangeInsertLtrData(data_ptr, (void*)data, regType);
             break;
        case NLM11KDEV_MISCELLENEOUS_LTR:
             errNum = Nlm11kDevMgr_pvt_ConstructMiscLtrData(data_ptr, (void*)data);
             break;

		case NLM11KDEV_SS_LTR:
			 errNum = Nlm11kDevMgr_pvt_ConstructSSLtrData(data_ptr, (void*)data);
             break;

        default:
            if(o_reason)
                *o_reason = NLMRSC_INVALID_REG_ADDRESS;
            return NLMERR_FAIL;
    }

	if(errNum != NLMERR_OK)
    {
        /* If any failure due to invalid input param; reset the requests and return error */
        NlmXpt__ResetRequests(dev->m_devMgr_p->m_xpt_p, NULL);
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;
        return NLMERR_FAIL;
    }
	/* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
	if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
			return NLMERR_FAIL;
	}

#ifdef NLM_PRE_RA12
	/* prepare request for NOP; after LTR register write we will pass NOPs (Errata) */
	if( NLMERR_OK != Nlm11kDevMgr__SendNop(dev, NLM11KDEV_NUM_OF_NOPS_FOR_REG, o_reason))
	{
		return NLMERR_FAIL;
	}
#endif

	return NLMERR_OK;
}

/*
Function: Nlm11kDevMgr__LogicalTableRegisterRead
Description: Reads from ltr register of specified ltrNum and specified ltr register type.
An appopriate structure based on Reg Type needs to be passed as *data.
For Blk Select Reg -- structure to be used Nlm11kDevBlkSelectReg
For Super Blk Key Select Reg -- structure to be used Nlm11kDevSuperBlkKeyMapReg
For Parallel Srch Reg -- structure to be used Nlm11kDevParallelSrchReg
For Range Insert 0 Reg -- structure to be used Nlm11kDevRangeInsertion0Reg
For Range Insert 1 Reg -- structure to be used Nlm11kDevRangeInsertion1Reg
For Miscelleneous Reg -- structure to be used Nlm11kDevMiscelleneousReg
For NetLogic Internal Reg -- structure to be used Nlm11kDevSSReg
For Key Construction Reg -- structure to be used Nlm11kDevKeyConstructReg
User should see the reason code in case of failure.

Note: Advanced Srch LTR is not yet supported
*/
 NlmErrNum_t Nlm11kDevMgr__LogicalTableRegisterRead(
	Nlm11kDev 				*dev,
	nlm_u8					ltrNum,		/* LTR profile set number */
	Nlm11kDevLtrRegType		regType,	/* see the structure description above */
	void					*o_data,	/* LTR register type structure pointer */
	NlmReasonCode*			o_reason
	)
 {
	NlmXptRqt* rqt_p = NULL;
	nlm_32     address =0;
    nlm_u8 readData[NLM11KDEV_REG_LEN_IN_BYTES + 1];/* Extra byte is for control bits
                                                     such as VBIT, parity */

	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(NULL == o_data)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_OUTPUT;
		return NLMERR_NULL_PTR;
	}
	if(ltrNum >= NLM11KDEV_NUM_LTR_SET)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_LTR_NUM;
		return NLMERR_FAIL;
	}
	if(g_is10MDev)
	{
		/* Following registers are not configurable when 10M device is used */
		if(regType == NLM11KDEV_BLOCK_SELECT_1_LTR ||
		   regType == NLM11KDEV_PARALLEL_SEARCH_1_LTR ||
		   regType == NLM11KDEV_PARALLEL_SEARCH_2_LTR ||
		   regType == NLM11KDEV_PARALLEL_SEARCH_3_LTR	)
		{
			if(o_reason)
				*o_reason = NLMRSC_INVALID_REG_ADDRESS;
			return NLMERR_FAIL;
		}
	}
    if(regType >= NLM11KDEV_LTR_REG_END)
    {
        if(o_reason)
			*o_reason = NLMRSC_INVALID_REG_ADDRESS;
		return NLMERR_FAIL;
    }
	/* preparing request for register read. */
    rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_READ_X_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_READ_X_BITS_5_0;

    rqt_p->m_data_len = NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

    /* In case of Reg read; XPT layer send the 80b read data via "result" field of xpt rqt;
    In addition to this result will contain some control bits such as vbit and parity which are ignored */
	rqt_p->m_result = readData;

	/* Find the single device address of the corresponding register_type.
            Addresses of LTR Registers of specified ltrNum are continuous based on RegType;
            Hence Get the base address of specified LtrNum(Blk Select 0 Address) and then add regtype to it
         */
	address = NLM11K_REG_ADDR_LTR_BLOCK_SELECT0(ltrNum) + regType;

    /* Insert the devid into the address */
    address = NLM11K_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
	rqt_p->m_address = address;
#else
	/* In case of Reg Read, Byte[3:0] of the "data" field of xpt rqt will contain the address*/
	WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_REG_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
                address
				);
#endif
    /* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            NlmXpt__DiscardResult(dev->m_devMgr_p->m_xpt_p, rqt_p, NULL);
			return NLMERR_FAIL;
        }
	}

    /* calling transport layer to get the results of the current request */
    if(NULL == NlmXpt__GetResult(dev->m_devMgr_p->m_xpt_p, rqt_p, NULL))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
		return NLMERR_NULL_PTR;
	}

    /* extracting register contents from result field of request structure;
    Note Byte[0] contains some control fields which can be ignored */
    switch(regType)
    {
        case NLM11KDEV_BLOCK_SELECT_0_LTR:
        case NLM11KDEV_BLOCK_SELECT_1_LTR:
             Nlm11kDevMgr_pvt_ExtractBlkSelLtrData(readData + 1, o_data);
             break;

        case NLM11KDEV_SUPER_BLK_KEY_MAP_LTR:
             Nlm11kDevMgr_pvt_ExtractSBKeySelLtrData(readData + 1, o_data);
             break;

        case NLM11KDEV_PARALLEL_SEARCH_0_LTR:
        case NLM11KDEV_PARALLEL_SEARCH_1_LTR:
        case NLM11KDEV_PARALLEL_SEARCH_2_LTR:
        case NLM11KDEV_PARALLEL_SEARCH_3_LTR:
             Nlm11kDevMgr_pvt_ExtractParallelSrchLtrData(readData + 1, o_data);
             break;

        case NLM11KDEV_KEY_0_KCR_0_LTR:
        case NLM11KDEV_KEY_0_KCR_1_LTR:
        case NLM11KDEV_KEY_1_KCR_0_LTR:
        case NLM11KDEV_KEY_1_KCR_1_LTR:
        case NLM11KDEV_KEY_2_KCR_0_LTR:
        case NLM11KDEV_KEY_2_KCR_1_LTR:
        case NLM11KDEV_KEY_3_KCR_0_LTR:
        case NLM11KDEV_KEY_3_KCR_1_LTR:
             Nlm11kDevMgr_pvt_ExtractKeyContructLtrData(readData + 1, o_data);
             break;

        case NLM11KDEV_RANGE_INSERTION_0_LTR:
        case NLM11KDEV_RANGE_INSERTION_1_LTR:
             Nlm11kDevMgr_pvt_ExtractRangeInsertLtrData(readData + 1, o_data, regType);
             break;

        case NLM11KDEV_MISCELLENEOUS_LTR:
             Nlm11kDevMgr_pvt_ExtractMiscLtrData(readData + 1, o_data);
             break;

		case NLM11KDEV_SS_LTR:
             Nlm11kDevMgr_pvt_ExtractSSLtrData(readData + 1, o_data);
             break;

        default:
            if(o_reason)
                *o_reason = NLMRSC_INVALID_REG_ADDRESS;
            return NLMERR_FAIL;
    }

	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;

	return NLMERR_OK;
}


/*
	Range related register write.
	This API is for internal use.
*/
NlmErrNum_t Nlm11kDevMgr__RangeRegisterWrite(
	Nlm11kDev				*dev,
	nlm_u32					address,
	Nlm11kDevRangeReg		*rangeRegData,
	NlmReasonCode			*o_reason
	)
{
    NlmXptRqt* rqt_p = NULL;
	nlm_u8 regData[NLM11KDEV_REG_LEN_IN_BYTES] = "";
	nlm_u8 *data_ptr = NULL;

	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(NULL == rangeRegData)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DATA;
		return NLMERR_NULL_PTR;
	}
    if(address < NLM11K_REG_RANGE_A_BOUNDS(0) || address > NLM11K_REG_RANGE_D_CODE1)
    {
        if(o_reason)
			*o_reason = NLMRSC_INVALID_REG_ADDRESS;
		return NLMERR_FAIL;
    }

	/* preparing request for register write. */
    rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_WRITE_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_WRITE_BITS_5_0;

	/* length of the register data */
	rqt_p->m_data_len = NLM11KDEV_REG_LEN_IN_BYTES;

	/* In case of Reg Write; "result" field of xpt rqt will be NULL */
	rqt_p->m_result = NULL;

	/* Insert the devid into the address */
	address = NLM11K_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
	rqt_p->m_address = address;

	/* request pointer will point to the range data to be written */
	data_ptr = rqt_p->m_data_p = regData;

#else
	(void)regData;
	/* In case of Reg write, Byte[3:0] of the "data" field of xpt rqt
	will contain the address and Byte[13:4] will contain the 80b register data */
	rqt_p->m_data_len += NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

	/* Byte[3:0] of the "data" field of xpt rqt will contain the address*/
	WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_REG_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
                address
				);

	/* pointer to the register data : Byte[13:4]*/
	data_ptr = rqt_p->m_data + NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

#endif

	/* write the relevant fields in the range register */
	if(address >= NLM11K_REG_RANGE_A_BOUNDS(0) && address < NLM11K_REG_RANGE_A_CODE0) /* range bound regs */
    {
        Nlm11kDevMgr_pvt_ConstructRangeBoundRegData(data_ptr, rangeRegData->m_data);
    }
	else /* range code regs : addrs >= NLM11K_REG_RANGE_A_CODE0 && addrs <= NLM11K_REG_RANGE_D_CODE1 */
    {
		Nlm11kDevMgr_pvt_ConstructRangeCodeRegData(data_ptr, rangeRegData->m_data);  
    }
	/* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
	if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
			return NLMERR_FAIL;
	}

    if(o_reason)
		*o_reason = NLMRSC_REASON_OK;
	return NLMERR_OK;
}

/*
	Range related register read.
	This API is for internal use.
*/
NlmErrNum_t Nlm11kDevMgr__RangeRegisterRead(
	Nlm11kDev				*dev,
	nlm_u32					address,
	Nlm11kDevRangeReg		*o_rangeRegData,
	NlmReasonCode			*o_reason
	)
{
    NlmXptRqt* rqt_p = NULL;
    nlm_u8 readData[NLM11KDEV_REG_LEN_IN_BYTES + 1];/* Extra byte is for control bits
                                                     such as VBIT, parity */

	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(NULL == o_rangeRegData)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_OUTPUT;
		return NLMERR_NULL_PTR;
	}
    if(address < NLM11K_REG_RANGE_A_BOUNDS(0) || address > NLM11K_REG_RANGE_D_CODE1)
    {
        if(o_reason)
			*o_reason = NLMRSC_INVALID_REG_ADDRESS;
		return NLMERR_FAIL;
    }

	/* preparing request for register read. */
    rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_READ_X_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_READ_X_BITS_5_0;

    rqt_p->m_data_len = NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

    /* In case of Reg read; XPT layer send the 80b read data via "result" field of xpt rqt;
    In addition to this result will contain some control bits such as vbit and parity which are ignored */
	rqt_p->m_result = readData;

	/* Insert the devid into the address */
    address = NLM11K_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
	rqt_p->m_address = address;
#else
	/* In case of Reg Read, Byte[3:0] of the "data" field of xpt rqt will contain the address*/
	WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_REG_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
                address
				);
#endif
    /* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            NlmXpt__DiscardResult(dev->m_devMgr_p->m_xpt_p, rqt_p, NULL);
			return NLMERR_FAIL;
        }
	}

    /* calling transport layer to get the results of the current request */
    if(NULL == NlmXpt__GetResult(dev->m_devMgr_p->m_xpt_p, rqt_p, NULL))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
		return NLMERR_NULL_PTR;
	}

    /* extracting register contents from result field of request structure;
    Note Byte[0] contains some control fields which can be ignored */
    NlmCm__memcpy(o_rangeRegData->m_data, readData + 1, NLM11KDEV_REG_LEN_IN_BYTES);
	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;
	return NLMERR_OK;
}

/*
    Function: Nlm11kDevMgr__LogicalTableRegisterRefresh refreshes the LTR register data depending on LTR register type.
    It reads the data from Shadow Memory the data of specified register and writes it to device.
    This function will be useful to re-write the LTR Register Data which have suffered from soft parity error
	 User should see the reason code in case of failure.
*/
NlmErrNum_t Nlm11kDevMgr__LogicalTableRegisterRefresh(
	Nlm11kDev 				*dev,
	nlm_u8					ltrNum,		/* LTR profile set number */
	Nlm11kDevLtrRegType	 	regType,	/* see the structure description above */
	NlmReasonCode*			o_reason
	)
{
    NlmXptRqt* rqt_p = NULL;
	nlm_32     address =0;
    NlmErrNum_t errNum = NLMERR_OK;
    Nlm11kDevShadowDevice *shadowDev_p;
    void *data;
    nlm_u8 regData[NLM11KDEV_REG_LEN_IN_BYTES] = "";
	nlm_u8 *data_ptr = NULL;

	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(ltrNum >= NLM11KDEV_NUM_LTR_SET)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_LTR_NUM;
		return NLMERR_FAIL;
	}
	if(g_is10MDev)
	{
		/* Following registers are not configurable when 10M device is used */
		if(regType == NLM11KDEV_BLOCK_SELECT_1_LTR ||
		   regType == NLM11KDEV_PARALLEL_SEARCH_1_LTR ||
		   regType == NLM11KDEV_PARALLEL_SEARCH_2_LTR ||
		   regType == NLM11KDEV_PARALLEL_SEARCH_3_LTR	)
		{
			if(o_reason)
				*o_reason = NLMRSC_INVALID_REG_ADDRESS;
			return NLMERR_FAIL;
		}
	}
    if(regType >= NLM11KDEV_LTR_REG_END)
    {
        if(o_reason)
			*o_reason = NLMRSC_INVALID_REG_ADDRESS;
		return NLMERR_FAIL;
    }
    shadowDev_p = NLM11K_GET_SHADOW_MEM_FROM_DEV_PTR(dev);
    if(shadowDev_p == NULL)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_SHADOWDEV_PTR;
        return NLMERR_NULL_PTR;
    }

	/* preparing request for register write. */
    rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_WRITE_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_WRITE_BITS_5_0;

    /* length of the register data */
	rqt_p->m_data_len = NLM11KDEV_REG_LEN_IN_BYTES;

    /* In case of Reg Write; "result" field of xpt rqt will be NULL */
	rqt_p->m_result = NULL;

	/* find the single device address of the corresponding register_type */
    /* Addresses of LTR Registers of specified ltrNum are continoues based on RegType;
        Get the base address of specified LtrNum(Blk Select 0 Address) and then add regtype to it*/
	address = NLM11K_REG_ADDR_LTR_BLOCK_SELECT0(ltrNum) + regType;

    /* Insert the devid into the address */
    address = NLM11K_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
	rqt_p->m_address = address;

	/* pointer to the register data */
	data_ptr = rqt_p->m_data_p = regData;
#else
	(void)regData;

	/* In case of Reg write, Byte[3:0] of the "data" field of xpt rqt
     will contain the address and Byte[13:4] will contain the 80b register data */
	rqt_p->m_data_len += NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

	/* Byte[3:0] of the "data" field of xpt rqt will contain the address*/
	WriteBitsInArray(
			rqt_p->m_data,
			NLM11KDEV_REG_ADDR_LEN_IN_BYTES,
			(NLM11KDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
			0,
			address
			);

	/* pointer to the register data : Byte[13:4]*/
	data_ptr = rqt_p->m_data + NLM11KDEV_REG_ADDR_LEN_IN_BYTES;
#endif

/* writing register contents into data field of request structure */
    switch(regType)
    {
        case NLM11KDEV_BLOCK_SELECT_0_LTR:
        case NLM11KDEV_BLOCK_SELECT_1_LTR:
            /* Get the Data from shadow memory */
            data = &shadowDev_p->m_ltr[ltrNum].m_blockSelect[regType - NLM11KDEV_BLOCK_SELECT_0_LTR];
            errNum = Nlm11kDevMgr_pvt_ConstructBlkSelLtrData(data_ptr, (void*)data);
            break;

        case NLM11KDEV_SUPER_BLK_KEY_MAP_LTR:
            /* Get the Data from shadow memory */
            data = &shadowDev_p->m_ltr[ltrNum].m_superBlkKeyMap;
            errNum = Nlm11kDevMgr_pvt_ConstructSBKeySelLtrData(data_ptr, (void*)data);
            break;

        case NLM11KDEV_PARALLEL_SEARCH_0_LTR:
        case NLM11KDEV_PARALLEL_SEARCH_1_LTR:
        case NLM11KDEV_PARALLEL_SEARCH_2_LTR:
        case NLM11KDEV_PARALLEL_SEARCH_3_LTR:
            /* Get the Data from shadow memory */
            data = &shadowDev_p->m_ltr[ltrNum].m_parallelSrch[regType - NLM11KDEV_PARALLEL_SEARCH_0_LTR];
            errNum = Nlm11kDevMgr_pvt_ConstructParallelSrchLtrData(data_ptr, (void*)data);
             break;

        case NLM11KDEV_KEY_0_KCR_0_LTR:
        case NLM11KDEV_KEY_0_KCR_1_LTR:
        case NLM11KDEV_KEY_1_KCR_0_LTR:
        case NLM11KDEV_KEY_1_KCR_1_LTR:
        case NLM11KDEV_KEY_2_KCR_0_LTR:
        case NLM11KDEV_KEY_2_KCR_1_LTR:
        case NLM11KDEV_KEY_3_KCR_0_LTR:
        case NLM11KDEV_KEY_3_KCR_1_LTR:
            /* Get the Data from shadow memory */
            data = &shadowDev_p->m_ltr[ltrNum].m_keyConstruct[regType - NLM11KDEV_KEY_0_KCR_0_LTR];
            errNum = Nlm11kDevMgr_pvt_ConstructKeyContructLtrData(data_ptr, (void*)data);
            break;

        case NLM11KDEV_RANGE_INSERTION_0_LTR:
        case NLM11KDEV_RANGE_INSERTION_1_LTR:
            /* Get the Data from shadow memory */
            if(regType == NLM11KDEV_RANGE_INSERTION_0_LTR)
                data = &shadowDev_p->m_ltr[ltrNum].m_rangeInsert0;
            else
                data = &shadowDev_p->m_ltr[ltrNum].m_rangeInsert1;

            errNum = Nlm11kDevMgr_pvt_ConstructRangeInsertLtrData(data_ptr,	(void*)data, regType);
             break;

        case NLM11KDEV_MISCELLENEOUS_LTR:
            /* Get the Data from shadow memory */
            data = &shadowDev_p->m_ltr[ltrNum].m_miscelleneous;

            errNum = Nlm11kDevMgr_pvt_ConstructMiscLtrData(data_ptr, (void*)data);
             break;

        default:
            if(o_reason)
                *o_reason = NLMRSC_INVALID_REG_ADDRESS;
            return NLMERR_FAIL;
    }

	if(errNum != NLMERR_OK)
    {
        /* If any failure due to invalid input param; reset the requests and return error */
        NlmXpt__ResetRequests(dev->m_devMgr_p->m_xpt_p, NULL);
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;
        return NLMERR_FAIL;
    }
	/* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
	if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
			return NLMERR_FAIL;
	}

#ifdef NLM_PRE_RA12
	/* prepare request for NOP; after LTR register write we will pass NOPs (Errata) */
	if( NLMERR_OK != Nlm11kDevMgr__SendNop(dev, NLM11KDEV_NUM_OF_NOPS_FOR_REG, o_reason))
	{
		return NLMERR_FAIL;
	}
#endif

	return NLMERR_OK;
}

/*
Function: Nlm11kDevMgr__GlobalRegisterWrite
Description: writes to specified global register.
An appopriate structure based on Reg Type needs to be passed as *data.
For Device Config Reg -- structure to be used Nlm11kDevConfigReg
For Error Status Mask Reg -- structure to be used Nlm11kDevErrStatusReg
For Database Soft Error FIFO Reg -- structure to be used Nlm11kDevDbSoftErrFifoReg
For Scratch Pad Reg -- structure to be used Nlm11kDevScratchPadReg

Write to DevId Reg, Error Status Register, Advanced Features Soft Error Register
and Result Registers is not a valid operation since they are read only Register;

User should see the reason code in case of failure.
*/
 NlmErrNum_t Nlm11kDevMgr__GlobalRegisterWrite(
	Nlm11kDev 					*dev,
	Nlm11kDevGlobalRegType		regType,	/* Global register type - see definitions above */
	const void 					*data,		/* Global register structure pointer */
	NlmReasonCode*				o_reason
	)
 {
	NlmXptRqt* rqt_p = NULL;
	nlm_32     address =0;
   	nlm_u32 value = 0;
	nlm_u8 regData[NLM11KDEV_REG_LEN_IN_BYTES] = "";
	nlm_u8 *data_ptr = NULL;

	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(NULL == data)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DATA;
		return NLMERR_NULL_PTR;
	}
    if(regType >= NLM11KDEV_GLOBALREG_END)
    {
        if(o_reason)
			*o_reason = NLMRSC_INVALID_REG_ADDRESS;
		return NLMERR_FAIL;
    }

	/* preparing request for register write. */
    rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_WRITE_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_WRITE_BITS_5_0;

    /* length of the register data */
	rqt_p->m_data_len = NLM11KDEV_REG_LEN_IN_BYTES;

#ifdef NLM_NO_MEMCPY_IN_XPT
	data_ptr = regData;
#else
	(void)regData;

	/* In case of Reg write, Byte[3:0] of the "data" field of xpt rqt
     will contain the address and Byte[13:4] will contain the 80b register data */
	rqt_p->m_data_len += NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

	/* pointer to the register data : Byte[13:4]*/
	data_ptr = rqt_p->m_data + NLM11KDEV_REG_ADDR_LEN_IN_BYTES;
#endif

    /* In case of Reg Write; "result" field of xpt rqt will be NULL */
	rqt_p->m_result = NULL;

    switch(regType)
    {
        case NLM11KDEV_DEVICE_CONFIG_REG:
            {
                Nlm11kDevConfigReg *devConfigData_p = (Nlm11kDevConfigReg *)data;

                /* Construct the address and data for Dev Config Reg */
                address = NLM11K_REG_ADDR_DEVICE_CONFIG;

                value = (devConfigData_p->m_softErrorScanEnable << 6)
                    | (devConfigData_p->m_dbParityErrEntryInvalidate << 7)
                    | (devConfigData_p->m_dbSoftErrProtectMode << 26)
                    | (devConfigData_p->m_eccScanType << 27)
                    | (devConfigData_p->m_rangeEngineEnable << 28)
                    | (devConfigData_p->m_lowPowerModeEnable << 30);

				WriteBitsInArray(data_ptr,
					NLM11KDEV_REG_LEN_IN_BYTES, 30, 0, value);
                break;
            }
            
        case NLM11KDEV_ERROR_STATUS_REG:
            {
               Nlm11kDevErrStatusReg *errStatusReg_p =  (Nlm11kDevErrStatusReg *)data;
               
                /* Construct the address and data for Dev Config Reg */
                address = NLM11K_REG_ADDR_ERROR_STATUS;

                value = (errStatusReg_p->m_parityScanFifoOverFlow << 5)
                    | (errStatusReg_p->m_crc24Err << 16)
                    | (errStatusReg_p->m_sopErr << 17)
                    | (errStatusReg_p->m_eopErr << 18)
                    | (errStatusReg_p->m_missingDataPktErr << 19)
                    | (errStatusReg_p->m_burstMaxErr << 20)
                    | (errStatusReg_p->m_rxNMACFifoParityErr << 21)
                    | (errStatusReg_p->m_instnBurstErr << 22)
                    | (errStatusReg_p->m_protocolErr << 23)
                    | (errStatusReg_p->m_channelNumErr << 24)
                    | (errStatusReg_p->m_burstControlWordErr << 25)
                    | (errStatusReg_p->m_illegalInstnErr << 26)
                    | (errStatusReg_p->m_devIdMismatchErr << 27)
                    | (errStatusReg_p->m_ltrParityErr << 28)
                    | (errStatusReg_p->m_ctxBufferParityErr << 29)
                    | (errStatusReg_p->m_powerLimitingErr << 31);

                WriteBitsInArray(data_ptr,
					NLM11KDEV_REG_LEN_IN_BYTES, 31, 0, value);

				value = 0;
				value =  (errStatusReg_p->m_alignmentErr << (48-32))
					| (errStatusReg_p->m_framingCtrlWordErr << (49-32))
					| (errStatusReg_p->m_rxPCSEFifoParityErr << (50-32));

				WriteBitsInArray(data_ptr,
					NLM11KDEV_REG_LEN_IN_BYTES, 50, 32, value);

                break;
            }

        case NLM11KDEV_ERROR_STATUS_MASK_REG:
            {
                Nlm11kDevErrStatusReg *errStatusMaskData_p = (Nlm11kDevErrStatusReg *)data;

                /* Construct the address and data for Err Status Mask Reg */
                address = NLM11K_REG_ADDR_ERROR_STATUS_MASK;
                value = ((errStatusMaskData_p->m_globalGIO_L1_Enable)
                    | (errStatusMaskData_p->m_dbSoftError << 1)
                    | (errStatusMaskData_p->m_dbSoftErrorFifoFull << 2)
                    | (errStatusMaskData_p->m_parityScanFifoOverFlow << 5)
                    | (errStatusMaskData_p->m_crc24Err << 16)
                    | (errStatusMaskData_p->m_sopErr << 17)
                    | (errStatusMaskData_p->m_eopErr << 18)
                    | (errStatusMaskData_p->m_missingDataPktErr << 19)
                    | (errStatusMaskData_p->m_burstMaxErr << 20)
                    | (errStatusMaskData_p->m_rxNMACFifoParityErr << 21)
                    | (errStatusMaskData_p->m_instnBurstErr << 22)
                    | (errStatusMaskData_p->m_protocolErr << 23)
                    | (errStatusMaskData_p->m_channelNumErr << 24)
                    | (errStatusMaskData_p->m_burstControlWordErr << 25)
                    | (errStatusMaskData_p->m_illegalInstnErr << 26)
                    | (errStatusMaskData_p->m_devIdMismatchErr << 27)
                    | (errStatusMaskData_p->m_ltrParityErr << 28)
                    | (errStatusMaskData_p->m_ctxBufferParityErr << 29)
                    | (errStatusMaskData_p->m_powerLimitingErr << 31));

                WriteBitsInArray(data_ptr,
					NLM11KDEV_REG_LEN_IN_BYTES, 31, 0, value);

				value = 0;
				value =  (errStatusMaskData_p->m_alignmentErr << (48-32))
					| (errStatusMaskData_p->m_framingCtrlWordErr << (49-32))
					| (errStatusMaskData_p->m_rxPCSEFifoParityErr << (50-32));

				WriteBitsInArray(data_ptr,
					NLM11KDEV_REG_LEN_IN_BYTES, 50, 32, value);

				WriteBitsInArray(data_ptr,
					NLM11KDEV_REG_LEN_IN_BYTES, 79, 79, errStatusMaskData_p->m_globalGIO_L0_Enable);
                break;
            }

        case NLM11KDEV_DATABASE_SOFT_ERROR_FIFO_REG:
            {
                Nlm11kDevDbSoftErrFifoReg *dbSoftErrorFifoData_p = (Nlm11kDevDbSoftErrFifoReg *)data;

                /* Construct the address and data for Database Soft Error FIFO Reg */
                address = NLM11K_REG_ADDR_PARITY_ERROR_FIFO;

                /* Only Erase FIFO and Erase FIFO Entry Field are write/Read; Rest of the
                field are read only, hence ignored */
                value = (dbSoftErrorFifoData_p->m_eraseFifo << 1) | dbSoftErrorFifoData_p->m_eraseFifoEntry;
                WriteBitsInArray(data_ptr,
					NLM11KDEV_REG_LEN_IN_BYTES, 41, 40, value);
                break;
            }


        case NLM11KDEV_SCRATCH_PAD0_REG:
        case NLM11KDEV_SCRATCH_PAD1_REG:
            {
                Nlm11kDevScratchPadReg *scratchPadData_p = (Nlm11kDevScratchPadReg *)data;

                /* Construct the address and data for Scratch Pad Reg */
                address =  NLM11K_REG_ADDR_SCRATCH_PAD0 + (regType%2);
                #ifdef NLM_NO_MEMCPY_IN_XPT
					data_ptr = scratchPadData_p->m_data;
				#else
					NlmCm__memcpy(data_ptr,
						scratchPadData_p->m_data,
						NLM11KDEV_REG_LEN_IN_BYTES);
				#endif
                break;
            }
        case NLM11KDEV_DEVICE_ID_REG:
        case NLM11KDEV_RESULT0_REG:
        case NLM11KDEV_RESULT1_REG:
        case NLM11KDEV_ADVANCED_FEATURES_SOFT_ERROR_REG:
            /* Read only Register; Return Error */
            NlmXpt__ResetRequests(dev->m_devMgr_p->m_xpt_p, NULL);
            if(o_reason)
                *o_reason = NLMRSC_READONLY_REGISTER;
            return NLMERR_FAIL;

        default:
            if(o_reason)
                *o_reason = NLMRSC_INVALID_REG_ADDRESS;
            return NLMERR_FAIL;
    }

    /* Insert the devid into the address */
    address = NLM11K_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
	rqt_p->m_address = address;

	rqt_p->m_data_p = data_ptr;
#else
	/* Byte[3:0] of the "data" field of xpt rqt will contain the address*/
	WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_REG_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
                address
				);
#endif
	/* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
	if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
			return NLMERR_FAIL;
	}

	return NLMERR_OK;
}

/*
Function: Nlm11kDevMgr__GlobalRegisterRead
Description: Reads from specified global register.
An appopriate structure based on Reg Type needs to be passed as *data.
For Device Id Reg -- structure to be used Nlm11kDevIdReg
For Device Config Reg -- structure to be used Nlm11kDevConfigReg
For Error Status Reg -- structure to be used Nlm11kDevErrStatusReg
For Error Status Mask Reg -- structure to be used Nlm11kDevErrStatusReg
For Database Soft Error FIFO Reg -- structure to be used Nlm11kDevDbSoftErrFifoReg
For Advanced Features Soft Error Reg -- structure to be used Nlm11kDevAdvancedSoftErrReg
For Scratch Pad Registers -- structure to be used Nlm11kDevScratchPadReg
For Result Registers  -- structure to be used Nlm11kDevResultReg
User should see the reason code in case of failure.
*/
NlmErrNum_t Nlm11kDevMgr__GlobalRegisterRead(
	Nlm11kDev 					*dev,
	Nlm11kDevGlobalRegType		regType,		/* see the enum description */
	void					    *o_data,
	NlmReasonCode*				o_reason
	)
 {
	NlmXptRqt* rqt_p = NULL;
	nlm_32     address = 0;
    nlm_u32 value;
    nlm_u8 readData[NLM11KDEV_REG_LEN_IN_BYTES + 1];/* Extra byte is for control bits
                                                     such as VBIT, parity */
	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(NULL == o_data)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_OUTPUT;
		return NLMERR_NULL_PTR;
	}
    if(regType >= NLM11KDEV_GLOBALREG_END)
    {
        if(o_reason)
			*o_reason = NLMRSC_INVALID_REG_ADDRESS;
		return NLMERR_FAIL;
    }
	/* preparing request for register read. */
    rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_READ_X_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_READ_X_BITS_5_0;

    rqt_p->m_data_len = NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

    /* In case of Reg read; XPT layer send the 80b read data via "result" field of xpt rqt;
    In addition to this result will contain some control bits such as vbit and parity which are ignored */
	rqt_p->m_result = readData;

	/* find the single device address of the corresponding register_type */
    switch(regType)
    {
        case NLM11KDEV_DEVICE_CONFIG_REG:
            address = NLM11K_REG_ADDR_DEVICE_CONFIG;
            break;

        case NLM11KDEV_ERROR_STATUS_MASK_REG:
            address = NLM11K_REG_ADDR_ERROR_STATUS_MASK;
            break;

        case NLM11KDEV_DATABASE_SOFT_ERROR_FIFO_REG:
            address = NLM11K_REG_ADDR_PARITY_ERROR_FIFO;
            break;

        case NLM11KDEV_SCRATCH_PAD0_REG:
            address =  NLM11K_REG_ADDR_SCRATCH_PAD0;
            break;

        case NLM11KDEV_SCRATCH_PAD1_REG:
            address =  NLM11K_REG_ADDR_SCRATCH_PAD1;
            break;

        case NLM11KDEV_DEVICE_ID_REG:
            address = NLM11K_REG_ADDR_DEVICE_ID;
            break;

        case NLM11KDEV_ERROR_STATUS_REG:
            address = NLM11K_REG_ADDR_ERROR_STATUS;
            break;

        case NLM11KDEV_RESULT0_REG:
             address = NLM11K_REG_ADDR_RESULT0;
            break;

        case NLM11KDEV_RESULT1_REG:
             address = NLM11K_REG_ADDR_RESULT1;
            break;

        case NLM11KDEV_ADVANCED_FEATURES_SOFT_ERROR_REG:
            address = NLM11K_REG_ADDR_DETAILED_PARITY_ERROR_INFO;
            break;

        default:
            if(o_reason)
                *o_reason = NLMRSC_INVALID_REG_ADDRESS;
            return NLMERR_FAIL;
    }

    /* Insert the devid into the address */
    address = NLM11K_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
	rqt_p->m_address = address;
#else
	/* In case of Reg Read, Byte[3:0] of the "data" field of xpt rqt will contain the address*/
	WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_REG_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
                address
				);
#endif

    /* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            NlmXpt__DiscardResult(dev->m_devMgr_p->m_xpt_p, rqt_p, NULL);
			return NLMERR_FAIL;
        }
	}

    /* calling transport layer to get the results of the current request */
    if(NULL == NlmXpt__GetResult(dev->m_devMgr_p->m_xpt_p, rqt_p, NULL))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
		return NLMERR_NULL_PTR;
	}

    /* extracting register contents from result field of request structure;
    Note: Byte[0] contains some control fields which can be ignored */
    switch(regType)
    {
        case NLM11KDEV_DEVICE_CONFIG_REG:
             {
                Nlm11kDevConfigReg *devConfigData_p = (Nlm11kDevConfigReg *)o_data;

                value = ReadBitsInArrray(readData + 1, NLM11KDEV_REG_LEN_IN_BYTES, 30, 0);

                devConfigData_p->m_softErrorScanEnable = (value >> 6) & 1;
                devConfigData_p->m_dbParityErrEntryInvalidate = (value >> 7) & 1;
                devConfigData_p->m_dbSoftErrProtectMode = (value >> 26) & 1;
                devConfigData_p->m_eccScanType = (value >> 27) & 1;
                devConfigData_p->m_rangeEngineEnable = (value >> 28) & 1;
                devConfigData_p->m_lowPowerModeEnable = (value >> 30) & 1;
                break;
            }

        case NLM11KDEV_ERROR_STATUS_MASK_REG:
        case NLM11KDEV_ERROR_STATUS_REG:
            {
                Nlm11kDevErrStatusReg *errStatusReg_p = (Nlm11kDevErrStatusReg*)o_data;

                value = ReadBitsInArrray(readData + 1, NLM11KDEV_REG_LEN_IN_BYTES, 31, 0);

                /* Global GIO_L1 Enable is valid only for Error Status Mask Reg */
                if(regType == NLM11KDEV_ERROR_STATUS_MASK_REG)
                    errStatusReg_p->m_globalGIO_L1_Enable = (nlm_u8)(value & 1);

                errStatusReg_p->m_dbSoftError = (nlm_u8)((value >> 1) & 1);
                errStatusReg_p->m_dbSoftErrorFifoFull = (nlm_u8)((value >> 2) & 1);
                errStatusReg_p->m_parityScanFifoOverFlow =  (nlm_u8)((value >> 5) & 1);
                errStatusReg_p->m_crc24Err =  (nlm_u8)((value >> 16) & 1);
                errStatusReg_p->m_sopErr =  (nlm_u8)((value >> 17) & 1);
                errStatusReg_p->m_eopErr =  (nlm_u8)((value >> 18) & 1);
                errStatusReg_p->m_missingDataPktErr =  (nlm_u8)((value >> 19) & 1);
                errStatusReg_p->m_burstMaxErr =  (nlm_u8)((value >> 20) & 1);
                errStatusReg_p->m_rxNMACFifoParityErr =  (nlm_u8)((value >> 21) & 1);
                errStatusReg_p->m_instnBurstErr =  (nlm_u8)((value >> 22) & 1);
                errStatusReg_p->m_protocolErr =  (nlm_u8)((value >> 23) & 1);
                errStatusReg_p->m_channelNumErr =  (nlm_u8)((value >> 24) & 1);
                errStatusReg_p->m_burstControlWordErr =  (nlm_u8)((value >> 25) & 1);
                errStatusReg_p->m_illegalInstnErr =  (nlm_u8)((value >> 26) & 1);
                errStatusReg_p->m_devIdMismatchErr =  (nlm_u8)((value >> 27) & 1);
                errStatusReg_p->m_ltrParityErr = (nlm_u8)((value >> 28) & 1);
                errStatusReg_p->m_ctxBufferParityErr =  (nlm_u8)((value >> 29) & 1);
                errStatusReg_p->m_powerLimitingErr =  (nlm_u8)((value >> 31) & 1);

                value = ReadBitsInArrray(readData + 1, NLM11KDEV_REG_LEN_IN_BYTES, 55, 32);
                errStatusReg_p->m_alignmentErr =  (nlm_u8)((value >> (48-32)) & 1);
                errStatusReg_p->m_framingCtrlWordErr =  (nlm_u8)((value >> (49-32)) & 1);
                errStatusReg_p->m_rxPCSEFifoParityErr =  (nlm_u8)((value >> (50-32)) & 1);

                 /* Global GIO_L0 Enable is valid only for Error Status Mask Reg */
                if(regType == NLM11KDEV_ERROR_STATUS_MASK_REG)
                {
                    value = ReadBitsInArrray(readData + 1, NLM11KDEV_REG_LEN_IN_BYTES, 79, 79);
                    errStatusReg_p->m_globalGIO_L0_Enable = (nlm_u8)(value & 1);
                 }

                break;
            }

        case NLM11KDEV_DATABASE_SOFT_ERROR_FIFO_REG:
            {
                Nlm11kDevDbSoftErrFifoReg *dbSoftErrorFifoData_p = (Nlm11kDevDbSoftErrFifoReg *)o_data;

                /* Erase FIFO and Erase FIFO Entry Field are Write Only Bits and hence are ignored */
				value = ReadBitsInArrray(readData + 1, NLM11KDEV_REG_LEN_IN_BYTES, 41, 40);
				dbSoftErrorFifoData_p->m_eraseFifo = (value >> 1 ) & 1;
				dbSoftErrorFifoData_p->m_eraseFifoEntry = (value & 1);

				value = ReadBitsInArrray(readData + 1, NLM11KDEV_REG_LEN_IN_BYTES, 23, 0);
                dbSoftErrorFifoData_p->m_errorAddrValid = (value >> 23) & 1;
                dbSoftErrorFifoData_p->m_pErrorY = (value >> 22) & 1;
                dbSoftErrorFifoData_p->m_pErrorX = (value >> 21) & 1;
                /* Bits[20:0] give error Address */
                dbSoftErrorFifoData_p->m_errorAddr = value & 0x1FFFFF;
                break;
            }

        case NLM11KDEV_SCRATCH_PAD0_REG:
        case NLM11KDEV_SCRATCH_PAD1_REG:
            {
                Nlm11kDevScratchPadReg *scratchPadData_p = (Nlm11kDevScratchPadReg *)o_data;
                NlmCm__memcpy(scratchPadData_p->m_data, readData + 1, NLM11KDEV_REG_LEN_IN_BYTES);
                break;
            }

        case NLM11KDEV_DEVICE_ID_REG:
            {
                Nlm11kDevIdReg *devIdData_p = (Nlm11kDevIdReg *)o_data;
                value = ReadBitsInArrray(readData + 1, NLM11KDEV_REG_LEN_IN_BYTES, 7, 0);
                /* Dev Id Reg has following Data
                Minor Die Revision -- Bits[2:0]
                Major Die Revision -- Bits[5:3]
                Database Size -- Bits[7:6] */
                devIdData_p->m_minorDieRev = (nlm_u8)(value & 0x7);
                devIdData_p->m_majorDieRev = (nlm_u8)((value >> 3)& 0x7);
                devIdData_p->m_databaseSize = (nlm_u8)((value >> 6)& 0x3);
                break;
            }

        case NLM11KDEV_RESULT0_REG:
        case NLM11KDEV_RESULT1_REG:
            {
                Nlm11kDevResultReg *rsltData_p = (Nlm11kDevResultReg *)o_data;
                nlm_32 psNum;

                /* Rslt Register has HIT or MISS Flag at Bits 39 and 79,
                    HIT Address at Bits[23:0] and Bits[63:40] */
                for(psNum = 0; psNum < (NLM11KDEV_NUM_PARALLEL_SEARCHES/2); psNum++)
                {
                    rsltData_p->m_hitOrMiss[psNum] = ReadBitsInArrray(readData + 1,
                                                    NLM11KDEV_REG_LEN_IN_BYTES,
                                                    39 + (psNum * 40),
                                                    39 + (psNum *40));

                    rsltData_p->m_hitAddress[psNum] = ReadBitsInArrray(readData + 1,
                                                    NLM11KDEV_REG_LEN_IN_BYTES,
                                                    23 + (psNum * 40),
                                                    (psNum *40));
                }
                break;
            }

        case NLM11KDEV_ADVANCED_FEATURES_SOFT_ERROR_REG:
            {
                Nlm11kDevAdvancedSoftErrReg *advancedSoftErrData_p = (Nlm11kDevAdvancedSoftErrReg *)o_data;

                advancedSoftErrData_p->m_cbParityErrAddr = (nlm_u16)(ReadBitsInArrray(readData + 1,
                                                                    NLM11KDEV_REG_LEN_IN_BYTES,
                                                                    13, 0));

                advancedSoftErrData_p->m_sahasraParityErrAddr0  = (nlm_u16)(ReadBitsInArrray(readData + 1,
                                                                    NLM11KDEV_REG_LEN_IN_BYTES,
                                                                    29, 16));

		        advancedSoftErrData_p->m_sahasraParityErrAddr1  = (nlm_u16)(ReadBitsInArrray(readData + 1,
	                                                            NLM11KDEV_REG_LEN_IN_BYTES,
	                                                            45, 32));

                advancedSoftErrData_p->m_ltrParityErrAddr = (nlm_u16)(ReadBitsInArrray(readData + 1,
                                                                    NLM11KDEV_REG_LEN_IN_BYTES,
                                                                    58, 48));
                break;

            }

        default:
             if(o_reason)
                 *o_reason = NLMRSC_INVALID_REG_ADDRESS;
             return NLMERR_FAIL;
    }

	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;
	return NLMERR_OK;
}


/*
Function: Nlm11kDevMgr__CBAsRegisterWrite
Description: writes to the specified address of the CB Memory as a 80b register write

User should see the reason code in case of failure.
Note: Address should be offset address i.e 0 - 16383 ( NLM11KDEV_CB_DEPTH - 1)
*/
 NlmErrNum_t Nlm11kDevMgr__CBAsRegisterWrite(
	Nlm11kDev				*dev,
	nlm_u16					cbAddr,
	Nlm11kDevCtxBufferReg 	*cbRegData,			/* see the structure description above */
    NlmReasonCode*			o_reason
    )
 {
	NlmXptRqt* rqt_p = NULL;
	nlm_32     address =0;

	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(NULL == cbRegData)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DATA;
		return NLMERR_NULL_PTR;
	}
	if(cbAddr >= NLM11KDEV_CB_DEPTH)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_CB_ADDRESS;
		return NLMERR_FAIL;
	}

	/* preparing request for register write. */
    rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_WRITE_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_WRITE_BITS_5_0;

    /* length of the register data */
	rqt_p->m_data_len = NLM11KDEV_REG_LEN_IN_BYTES;

    /* In case of Reg Write; "result" field of xpt rqt will be NULL */
	rqt_p->m_result = NULL;

	/* find the single device address of the specified CB register */
	address = NLM11K_REG_ADDR_CONTEXT_BUFFER(cbAddr);

    /* Insert the devid into the address */
    address = NLM11K_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
	/* writing the address into request structure */
	rqt_p->m_address = address;

	/* request pointer will point to the CB data to be written */
	rqt_p->m_data_p = cbRegData->m_data;

#else
	/* In case of Reg write, Byte[3:0] of the "data" field of xpt rqt
	will contain the address and Byte[13:4] will contain the 80b register data */
	rqt_p->m_data_len += NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

	/* Byte[3:0] of the "data" field of xpt rqt will contain the address*/
	WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_REG_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
                address
				);

	/* copy the CB data to be written in request structure */
	NlmCm__memcpy(rqt_p->m_data + NLM11KDEV_REG_ADDR_LEN_IN_BYTES,
				cbRegData->m_data,
				NLM11KDEV_REG_LEN_IN_BYTES);
#endif
    /* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
	if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
			return NLMERR_FAIL;
	}

	return NLMERR_OK;
}

 /*
Function: Nlm11kDevMgr__CBAsRegisterRead
Description: reads from the specified address of the CB Memory as a 80b register read

User should see the reason code in case of failure.
Note: Address should be offset address i.e 0 - 16383 ( NLM11KDEV_CB_DEPTH - 1)
*/
 NlmErrNum_t Nlm11kDevMgr__CBAsRegisterRead(
	Nlm11kDev				*dev,
	nlm_u16					cbAddr,
	Nlm11kDevCtxBufferReg 	*o_cbRegData,			/* see the structure description above */
    NlmReasonCode*			o_reason
    )
 {
	NlmXptRqt* rqt_p = NULL;
	nlm_32     address = 0;
    nlm_u8 readData[NLM11KDEV_REG_LEN_IN_BYTES + 1];/* Extra byte is for control bits
                                                     such as VBIT, parity */

	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(NULL == o_cbRegData)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_OUTPUT;
		return NLMERR_NULL_PTR;
	}
	if(cbAddr >= NLM11KDEV_CB_DEPTH)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_CB_ADDRESS;
		return NLMERR_FAIL;
	}

	/* preparing request for register read. */
    rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_READ_X_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_READ_X_BITS_5_0;

    rqt_p->m_data_len = NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

    /* In case of Reg read; XPT layer send the 80b read data via "result" field of xpt rqt;
    In addition to this result will contain some control bits such as vbit and parity which are ignored */
	rqt_p->m_result = readData;

	/* find the single device address of the specified CB location */
    address = NLM11K_REG_ADDR_CONTEXT_BUFFER(cbAddr);

    /* Insert the devid into the address */
    address = NLM11K_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
	rqt_p->m_address = address;
#else
	/* In case of Reg Read, Byte[3:0] of the "data" field of xpt rqt will contain the address*/
	WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_REG_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
                address
				);
#endif

    /* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
	if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            NlmXpt__DiscardResult(dev->m_devMgr_p->m_xpt_p, rqt_p, NULL);
			return NLMERR_FAIL;
        }
	}

    /* calling transport layer to get the results of the current request */
    if(NULL == NlmXpt__GetResult(dev->m_devMgr_p->m_xpt_p, rqt_p, NULL))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
		return NLMERR_NULL_PTR;
	}

    /* extracting register contents from result field of request structure */
    NlmCm__memcpy(o_cbRegData->m_data,
                  readData + 1,  /* Ignoring Byte 0 for control fields */
                  NLM11KDEV_REG_LEN_IN_BYTES);

	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;
	return NLMERR_OK;
}

/*
	Function Nlm11kDevMgr__ABEntryWrite writes single Array Block Database entry
    to the specified addr and abNum.
    See Description of Nlm11kDevABEntry for more details
    Data can be written in either DM or XY mode
	User should see the reason code in case of failure.
*/
NlmErrNum_t Nlm11kDevMgr__ABEntryWrite(
	Nlm11kDev      		*dev,
    nlm_u8				abNum,		/* Specifies the AB Number */
    nlm_u16				abAddr,		/* Specifies the AB Entry location within the specified abNum  */
    Nlm11kDevABEntry	*abEntry,
    Nlm11kDevWriteMode	writeMode,
	NlmReasonCode*		o_reason
	)
{
    NlmXptRqt* rqt_p = NULL;
    nlm_u32 address;
	nlm_u8 numBlocks = NLM11KDEV_NUM_ARRAY_BLOCKS;

	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(NULL == abEntry)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DATA;
		return NLMERR_NULL_PTR;
	}
	if(g_is10MDev)
		numBlocks = NLM11KDEV_10M_NUM_ARRAY_BLOCKS;

	if(abNum >= numBlocks)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_AB_NUM;
		return NLMERR_FAIL;
	}
	if(NLM11KDEV_AB_DEPTH <= abAddr)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_AB_INDEX;
		return NLMERR_FAIL;
	}

#ifdef NLM_PRE_RA12
	/* prepare request for NOP; before Data entry write we will pass NOPs (Errata) */
	if( NLMERR_OK != Nlm11kDevMgr__SendNop(dev, NLM11KDEV_NUM_OF_NOPS_FOR_AB, o_reason))
	{
		return NLMERR_FAIL;
	}
#endif

	/* preparing write request for AB Entry. */
    rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_WRITE_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_WRITE_BITS_5_0;

	/* D-M or X-Y and hence multiply by 2 */
	rqt_p->m_data_len = (NLM11KDEV_AB_WIDTH_IN_BYTES * 2); /* Check here NKG */

    /* In case of Database Write; "result" field of xpt rqt will be NULL */
	rqt_p->m_result = NULL;

	/* find the single device address of the corresponding database entry */
    address = (abNum << 12) | abAddr;

    /* Insert the devid into the address */
    address = NLM11K_DEVMGR_INSERT_DEVID(address, dev->m_devId);

     /* Insert VBit, address type bits and Wrmode bits in the address */
    address |= (1 << NLM11KDEV_AB_ENTRY_VALID_BIT_IN_ADDR)
                |  (1 << NLM11KDEV_ADDR_TYPE_BIT_IN_PIO_WRITE)
                |  (writeMode << NLM11KDEV_AB_ENTRY_WR_MODE_BIT_IN_ADDR);

#ifdef NLM_NO_MEMCPY_IN_XPT
	rqt_p->m_address = address;

	/* writing the DATA(X or D) into data field of request structure */
	rqt_p->m_data_p = abEntry->m_data;

	/* writing the MASK(Y or M) into data field of request structure*/
	rqt_p->m_mask_p = abEntry->m_mask;
#else
	/* In case of Database write, Byte[3:0] of the "data" field of xpt rqt
	     will contain the address(along with vbit, wrmode) and Byte[13:4] will contain the data
	     and Byte[23:14] will contain the mask
	  */
	rqt_p->m_data_len += NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

	WriteBitsInArray(
					rqt_p->m_data,
					NLM11KDEV_AB_ADDR_LEN_IN_BYTES,
					(NLM11KDEV_AB_ADDR_LEN_IN_BYTES * 8) - 1,
					0,
					address
					);

	/* writing the DATA(X or D) into data field of request structure */
	NlmCm__memcpy(rqt_p->m_data + NLM11KDEV_AB_ADDR_LEN_IN_BYTES,
				abEntry->m_data, NLM11KDEV_AB_WIDTH_IN_BYTES);

	/* writing the MASK(Y or M) into data field of request structure*/
	NlmCm__memcpy((rqt_p->m_data + NLM11KDEV_AB_ADDR_LEN_IN_BYTES + NLM11KDEV_AB_WIDTH_IN_BYTES),
				abEntry->m_mask, NLM11KDEV_AB_WIDTH_IN_BYTES);
#endif

	/* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
	if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
			return NLMERR_FAIL;
	}
	return NLMERR_OK;
}

/*
	Function Nlm11kDevMgr__ABEntryInvalidate invalidates single Array Block Database
    entry of the specified addr and abNum.

	User should see the reason code in case of failure.
*/
NlmErrNum_t Nlm11kDevMgr__ABEntryInvalidate(
	Nlm11kDev      		*dev,
    nlm_u8				abNum,		/* Specifies the AB Number */
    nlm_u16				abAddr,		/* Specifies the AB Entry location within the specified abNum  */
	NlmReasonCode*		o_reason
	)
{
    NlmXptRqt* rqt_p = NULL;
    nlm_u32 address;
	nlm_u8 regData[NLM11KDEV_REG_LEN_IN_BYTES] = "";
    nlm_u8 numBlocks = NLM11KDEV_NUM_ARRAY_BLOCKS;

	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(g_is10MDev)
		numBlocks = NLM11KDEV_10M_NUM_ARRAY_BLOCKS;

	if(abNum >= numBlocks)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_AB_NUM;
		return NLMERR_FAIL;
	}
	if(NLM11KDEV_AB_DEPTH <= abAddr)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_AB_INDEX;
		return NLMERR_FAIL;
	}

#ifdef NLM_PRE_RA12
	/* prepare request for NOP; before Data entry write we will pass NOPs (Errata) */
	if( NLMERR_OK != Nlm11kDevMgr__SendNop(dev, NLM11KDEV_NUM_OF_NOPS_FOR_AB, o_reason))
	{
		return NLMERR_FAIL;
	}
#endif

    /* AB Entry is invalidated by performing a AB Entry write with VBIT in the address field as 0 */

	/* preparing write request for AB Entry. */
    rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_WRITE_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_WRITE_BITS_5_0;

	rqt_p->m_data_len = (NLM11KDEV_AB_WIDTH_IN_BYTES * 2);

    /* In case of Database Write; "result" field of xpt rqt will be NULL */
	rqt_p->m_result = NULL;

	/* find the single device address of the corresponding database entry */
    address = (abNum << 12) | abAddr;

    /* Insert the devid into the address */
    address = NLM11K_DEVMGR_INSERT_DEVID(address, dev->m_devId);

     /* Insert address type bit in the address, VBIT is set to 0; wrmode does not matter */
    address |= (0 << NLM11KDEV_AB_ENTRY_VALID_BIT_IN_ADDR)
                |  (1 << NLM11KDEV_ADDR_TYPE_BIT_IN_PIO_WRITE);

#ifdef NLM_NO_MEMCPY_IN_XPT
	rqt_p->m_address = address;

	/* data and mask are ignored; so both have 0 value */
	rqt_p->m_data_p = regData;
	rqt_p->m_mask_p = regData;
#else
	(void)regData;

	/* In case of Database write, Byte[3:0] of the "data" field of xpt rqt
            will contain the address(along with vbit, wrmode) and Byte[13:4] will contain the data
            and Byte[23:14] will contain the mask
         */
	rqt_p->m_data_len += NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

	WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_AB_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_AB_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
                address
				);
#endif

	/* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
	if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
			return NLMERR_FAIL;
	}

	return NLMERR_OK;
}


/*
	Function Nlm11kDevMgr__ABEntryRead reads single Array Block Database entry
    from the specified addr and abNum.
    See Description of Nlm11kDevABEntry for more details
    Data can be read in  XY mode only
	User should see the reason code in case of failure.
*/
NlmErrNum_t Nlm11kDevMgr__ABEntryRead(
	Nlm11kDev      		*dev,
    nlm_u8				abNum,		/* Specifies the AB Number */
    nlm_u16				abAddr,		/* Specifies the AB Entry location within the specified abNum  */
    Nlm11kDevABEntry	*o_abEntry,
	NlmReasonCode*		o_reason
	)
{
    NlmXptRqt* rqt_p = NULL;
    nlm_u32 address;
    nlm_u8 readData[NLM11KDEV_REG_LEN_IN_BYTES + 1];/* Extra byte is for control bits
                                                     such as VBIT, parity */
	nlm_u8 numBlocks = NLM11KDEV_NUM_ARRAY_BLOCKS;

	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(NULL == o_abEntry)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_OUTPUT;
		return NLMERR_NULL_PTR;
	}
	if(g_is10MDev)
		numBlocks = NLM11KDEV_10M_NUM_ARRAY_BLOCKS;

	if(abNum >= numBlocks)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_AB_NUM;
		return NLMERR_FAIL;
	}
	if(NLM11KDEV_AB_DEPTH <= abAddr)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_AB_INDEX;
		return NLMERR_FAIL;
	}

    /* find the single device address of the corresponding database entry */
    address = (abNum << 12) | abAddr;

    /* Insert the devid into the address */
    address = NLM11K_DEVMGR_INSERT_DEVID(address, dev->m_devId);

     /* Insert address type bit in the address */
    address |= (1 << NLM11KDEV_ADDR_TYPE_BIT_IN_PIO_WRITE);

    /* AB Entry Read operation required two Read operations
    one for Data_X and another for Data_Y */

	/* preparing request for AB Entry ReadX. */
    rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_READ_X_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_READ_X_BITS_5_0;

    rqt_p->m_data_len = NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

    /* In case of Database read; XPT layer send the 80b read data via "result" field of xpt rqt;
    In addition to this result will contain some control bits such as vbit and parity  */
	rqt_p->m_result = readData;

#ifdef NLM_NO_MEMCPY_IN_XPT
	rqt_p->m_address = address;
#else
	/* In case of Reg Read, Byte[3:0] of the "data" field of xpt rqt
       will contain the address*/
	WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_AB_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_AB_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
                address
				);
#endif

    /* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            NlmXpt__DiscardResult(dev->m_devMgr_p->m_xpt_p, rqt_p, NULL);
			return NLMERR_FAIL;
        }
	}

    /* calling transport layer to get the results of the current request */
    if(NULL == NlmXpt__GetResult(dev->m_devMgr_p->m_xpt_p, rqt_p, NULL))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
		return NLMERR_NULL_PTR;
	}

    /* Get the Vbit which will be Bit 4 of Byte 0 of read Data */
    o_abEntry->m_vbit = (readData[0] >> 4) & 1;

    /* Copy the Data X read from the device */
    NlmCm__memcpy(o_abEntry->m_data, readData + 1, NLM11KDEV_AB_WIDTH_IN_BYTES);

    /* preparing request for AB Entry ReadY. */
    rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_READ_Y_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_READ_Y_BITS_5_0;

    rqt_p->m_data_len = NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

    /* In case of Database read; XPT layer send the 80b read data via "result" field of xpt rqt;
    In addition to this result will contain some control bits such as vbit and parity  */
	rqt_p->m_result = readData;

#ifdef NLM_NO_MEMCPY_IN_XPT
	rqt_p->m_address = address;
#else
	/* In case of Reg Read, Byte[3:0] of the "data" field of xpt rqt
       will contain the address*/
	WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_AB_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_AB_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
                address
				);
#endif

    /* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            NlmXpt__DiscardResult(dev->m_devMgr_p->m_xpt_p, rqt_p, NULL);
			return NLMERR_FAIL;
        }
	}

    /* calling transport layer to get the results of the current request */
    if(NULL == NlmXpt__GetResult(dev->m_devMgr_p->m_xpt_p, rqt_p, NULL))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
		return NLMERR_NULL_PTR;
	}

    /* Copy the Data Y read from the device; Note Data_Y Read does not give VBIT status */
    NlmCm__memcpy(o_abEntry->m_mask, readData + 1, NLM11KDEV_AB_WIDTH_IN_BYTES);

	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;
	return NLMERR_OK;
}

/*
Function: Nlm11kDevMgr__ABEntryRefresh
Description: Refreshes one entry depending on entry index and AB number.
It reads the data at that location from Shadow Memory and writes it to device.
This function will be useful to re-write the entries which have suffered from soft parity error
User should see the reason code in case of failure.
*/
NlmErrNum_t Nlm11kDevMgr__ABEntryRefresh(
	Nlm11kDev      		*dev,
    nlm_u8 abNum, /* Specifies the AB Number */
    nlm_u16 abAddr, /* Specifies the AB Entry location within the specified abNum  */
    Nlm11kDevWriteMode	writeMode,
	NlmReasonCode*		o_reason
	)
{
    NlmXptRqt* rqt_p = NULL;
    nlm_u32 address;
    Nlm11kDevShadowDevice *shadowDev_p;
    nlm_u8 numBlocks = NLM11KDEV_NUM_ARRAY_BLOCKS;

	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(g_is10MDev)
		numBlocks = NLM11KDEV_10M_NUM_ARRAY_BLOCKS;

	if(abNum >= numBlocks)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_AB_NUM;
		return NLMERR_FAIL;
	}
	if(NLM11KDEV_AB_DEPTH <= abAddr)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_AB_INDEX;
		return NLMERR_FAIL;
	}
    shadowDev_p = NLM11K_GET_SHADOW_MEM_FROM_DEV_PTR(dev);
    if(shadowDev_p == NULL)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_SHADOWDEV_PTR;
        return NLMERR_NULL_PTR;
    }

#ifdef NLM_PRE_RA12
	/* prepare request for NOP; before Data entry write we will pass NOPs (Errata) */
	if( NLMERR_OK != Nlm11kDevMgr__SendNop(dev, NLM11KDEV_NUM_OF_NOPS_FOR_AB, o_reason))
	{
		return NLMERR_FAIL;
	}
#endif

	/* preparing write request for AB Entry. */
    rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_WRITE_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_WRITE_BITS_5_0;

    /* data length : data + mask */
	rqt_p->m_data_len = (NLM11KDEV_AB_WIDTH_IN_BYTES * 2);

    /* In case of Database Write; "result" field of xpt rqt will be NULL */
	rqt_p->m_result = NULL;

	/* find the single device address of the corresponding database entry */
    address = (abNum << 12) | abAddr;

    /* Insert the devid into the address */
    address = NLM11K_DEVMGR_INSERT_DEVID(address, dev->m_devId);

    /* Insert VBit, address type bits and Wrmode bits in the address */
    address |= (1 << NLM11KDEV_AB_ENTRY_VALID_BIT_IN_ADDR)
                |  (1 << NLM11KDEV_ADDR_TYPE_BIT_IN_PIO_WRITE)
                |  (writeMode << NLM11KDEV_AB_ENTRY_WR_MODE_BIT_IN_ADDR);

#ifdef NLM_NO_MEMCPY_IN_XPT
	rqt_p->m_address = address;

	/* pointer to the DATA(X or D) stored in the shadow memory into data field of request structure */
	rqt_p->m_data_p =  shadowDev_p->m_arrayBlock[abNum].m_abEntry[abAddr].m_data;

	/* pointer to the MASK(Y or M) stored in the shadow memory into data field of request structure*/
	rqt_p->m_mask_p =  shadowDev_p->m_arrayBlock[abNum].m_abEntry[abAddr].m_mask;

#else
	/* In case of Database write, Byte[3:0] of the "data" field of xpt rqt
     will contain the address(along with vbit, wrmode) and Byte[13:4] will contain the data
     and Byte[23:14] will contain the mask*/
	rqt_p->m_data_len += NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

	WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_AB_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_AB_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
                address
				);

	/* writing the DATA(X or D) stored in the shadow memory into data field of request structure: Byte[13:4]*/
	NlmCm__memcpy(rqt_p->m_data + NLM11KDEV_AB_ADDR_LEN_IN_BYTES,
                shadowDev_p->m_arrayBlock[abNum].m_abEntry[abAddr].m_data,
                NLM11KDEV_AB_WIDTH_IN_BYTES);

	/* writing the MASK(Y or M) stored in the shadow memory into data field of request structure: Byte[23:14]*/
	NlmCm__memcpy((rqt_p->m_data + NLM11KDEV_AB_ADDR_LEN_IN_BYTES + NLM11KDEV_AB_WIDTH_IN_BYTES),
                shadowDev_p->m_arrayBlock[abNum].m_abEntry[abAddr].m_mask,
                NLM11KDEV_AB_WIDTH_IN_BYTES);
#endif

	/* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
	if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
			return NLMERR_FAIL;
	}
	return NLMERR_OK;
}


/*
	Function: Nlm11kDevMgr__CBWrite writes to context buffer memory
    Upto 640 bytes of data can be written to CB memory;
    see desription of Nlm11kDevCtxBufferInfo for more details
	User should see the reason code in case of failure.
*/
NlmErrNum_t Nlm11kDevMgr__CBWrite(
    Nlm11kDevMgr				*self,
	Nlm11kDevCtxBufferInfo		*cbInfo,				/* see the structure description above */
	NlmReasonCode*				o_reason
	)
{
    NlmXptRqt* rqt_p = NULL;

	if(!NLM11K_IS_VALID_DEVMGR_PTR(self))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEVMGR_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != self->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(NULL == cbInfo)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DATA;
		return NLMERR_NULL_PTR;
	}
    if(cbInfo->m_datalen == 0
        || cbInfo->m_datalen > NLM11KDEV_MAX_CB_WRITE_IN_BYTES)
    {
        /* Datalen cannot be 0 or more than 80bytes(640b) */
        if(o_reason)
			*o_reason = NLMRSC_INVALID_INPUT;
		return NLMERR_FAIL;
    }
    if(cbInfo->m_cbStartAddr >= NLM11KDEV_CB_DEPTH)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_CB_ADDRESS;
		return NLMERR_FAIL;
	}

	/* preparing write request for CB memory. */
    rqt_p = NlmXpt__GetRequest(self->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_CBWRITE_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_CBWRITE_BITS_5_0;

    /* In case of CB write, valid xpt data is upto datalen specified by user; */
	rqt_p->m_data_len = cbInfo->m_datalen;

    /* In case of CB Write; "result" field of xpt rqt will be NULL */
	rqt_p->m_result = NULL;

#ifndef NLM_12K_11K 
    /* writing CB Address into m_ctx_addr field of request structure;
    Note: There will be only 1 CB Address always */
    rqt_p->m_ctx_addr[0] = cbInfo->m_cbStartAddr;
#else
	rqt_p->m_ctx_addr = cbInfo->m_cbStartAddr;
#endif

#ifdef NLM_NO_MEMCPY_IN_XPT
	/* CB Data pointer by the data_p field of request structure */
	rqt_p->m_data_p = cbInfo->m_data;

#else
	/* copy CB Data into data field of request structure */
	NlmCm__memcpy(rqt_p->m_data, cbInfo->m_data, cbInfo->m_datalen);
#endif

	/* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(self->m_xpt_p, o_reason);
	if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
			return NLMERR_FAIL;
	}
    return NLMERR_OK;
}

/* This is private function, which called in __LockConfig. This function initializes the
   CB memory to zero to avoid the parity error.
*/
NlmErrNum_t Nlm11kDevMgr__pvt_CtxBufferInit(
    Nlm11kDevMgr				*self,
	NlmReasonCode*				o_reason
	)
{
	Nlm11kDevCtxBufferInfo cbInfo;

	if(!NLM11K_IS_VALID_DEVMGR_PTR(self))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEVMGR_PTR;
		return NLMERR_NULL_PTR;
	}
    
	/* initialize to 0 */
	NlmCm__memset(&cbInfo, 0, sizeof(Nlm11kDevCtxBufferInfo));

	cbInfo.m_cbStartAddr = 0;

	/* always cb is 640b write */
	cbInfo.m_datalen = NLM11KDEV_MAX_CB_WRITE_IN_BYTES;	

	do
	{
		/* always cb write is 640b write */
		if(NLMERR_OK != Nlm11kDevMgr__CBWrite(self, &cbInfo, o_reason))
			return NLMERR_FAIL;

		/* increment address by 8 */
		cbInfo.m_cbStartAddr +=8;	

	}while(cbInfo.m_cbStartAddr < NLM11KDEV_CB_DEPTH);

	if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
			return NLMERR_FAIL;
	}

    return NLMERR_OK;
}

/* CMP1 and CMP2 are same in case of 12K and 12K_11K mode, so these APIs are available 
only with 11K_MODE */
#if !defined NLM_12K_11K && !defined NLM_12K 


/*
	Nlm11kDevMgr__Compare1 is a search instruction. It searches the input key into the
	database depending on Ltr profile set number (there are 32 sets of Ltr registers)
	and gives hit/miss and address as an output.
    Compare1 instruction can be used for searches with individual search key lengths of upto 320b
    User should see the reason 	code in case of failure.
*/
NlmErrNum_t Nlm11kDevMgr__Compare1(
	Nlm11kDevMgr				*self,
	nlm_u8						ltrNum,				/* LTR profile set number to be used */
	Nlm11kDevCtxBufferInfo		*cbInfo,			/* see the structure description above */
	Nlm11kDevCmpRslt			*o_search_results,	/* see the structure description above */
	NlmReasonCode*				o_reason
	)
{
	Nlm11kDev  *dev_p = NULL;
	Nlm11kDevShadowDevice *shadowDev_p = NULL;
	Nlm11kDevSearchType   searchType;
    NlmXptRqt* rqt_p = NULL;
    nlm_u8 cmpRslts[NLM11KDEV_CMP_RSLT_LEN_IN_BYTES * NLM11KDEV_NUM_PARALLEL_SEARCHES];
    nlm_u32 value;
    nlm_32 psNum;

	if(!NLM11K_IS_VALID_DEVMGR_PTR(self))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEVMGR_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != self->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
    if(ltrNum >= NLM11KDEV_NUM_LTR_SET)
    {
        if(o_reason)
			*o_reason = NLMRSC_INVALID_LTR_NUM;
		return NLMERR_FAIL;
    }
	if(NULL == cbInfo)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DATA;
		return NLMERR_NULL_PTR;
	}
    if(o_search_results == NULL)
    {
        if(o_reason)
			*o_reason = NLMRSC_INVALID_SRCH_RSLT_PTR;
		return NLMERR_NULL_PTR;
    }
    if(cbInfo->m_datalen == 0
        || cbInfo->m_datalen > NLM11KDEV_MAX_CB_WRITE_IN_BYTES)
    {
        /* Datalen cannot be 0 or more than 80bytes(640b) */
        if(o_reason)
			*o_reason = NLMRSC_INVALID_INPUT;
		return NLMERR_FAIL;
    }
    if(cbInfo->m_cbStartAddr >= NLM11KDEV_CB_DEPTH)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_CB_ADDRESS;
		return NLMERR_FAIL;
	}

	/* preparing request for Compare operation. */
    rqt_p = NlmXpt__GetRequest(self->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_CBWRITE_CMP1_BITS_8_6;
	rqt_p->m_opcode[1] = ltrNum;

    /* In case of CB write and Compare, valid xpt data is upto datalen specified by user; */
	rqt_p->m_data_len = cbInfo->m_datalen;

    /* In case of Compare operation; XPT layer send the four search rslts via "result" field of xpt rqt;
    Each search result if of 4 bytes (32b) with Byte[3-0] containing result for PS 0,
    Byte[7-4] containing result for PS 1, Byte[11-8] containing result for PS 2
    and Byte[15-12] containing result for PS 3 */
	rqt_p->m_result = cmpRslts;

    /* writing CB Address into m_ctx_addr field of request structure;
    Note: There will be only 1 CB Address always */
    rqt_p->m_ctx_addr[0] = cbInfo->m_cbStartAddr;

#ifdef NLM_NO_MEMCPY_IN_XPT
	/* CB Data pointer by the data_p field of request structure */
	rqt_p->m_data_p = cbInfo->m_data;

#else
	/* copy CB Data into data field of request structure */
	NlmCm__memcpy(rqt_p->m_data, cbInfo->m_data, cbInfo->m_datalen);
#endif

	 /* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(self->m_xpt_p, o_reason);
	if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            NlmXpt__DiscardResult(self->m_xpt_p, rqt_p, NULL);
			return NLMERR_FAIL;
        }
	}

    /* calling transport layer to get the results of the current request */
    if(NULL == NlmXpt__GetResult(self->m_xpt_p, rqt_p, NULL))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
		return NLMERR_NULL_PTR;
	}

	/* Get the Shadow Memory for Misc Reg access. As this register is updated in all
	  * the cascaded devices, gettting any device would do. Let's choose dev#0 (PHMD).
	  */
	dev_p = (Nlm11kDev *)self->m_devList_pp[0];
	shadowDev_p = dev_p->m_shadowDevice_p;

    /* Decode the result field of XPT Structure and return appropriate search results */
    for(psNum = 0; psNum < NLM11KDEV_NUM_PARALLEL_SEARCHES; psNum++)
    {
        /* each parallel search result is of 32bits,  Bit[30] indicates Hit/Miss */
        value = ReadBitsInArrray(cmpRslts + (psNum * NLM11KDEV_CMP_RSLT_LEN_IN_BYTES),
                                	NLM11KDEV_CMP_RSLT_LEN_IN_BYTES,
                                	NLM11KDEV_CMP_RSLT_IDX_END_BIT,
                                	NLM11KDEV_CMP_RSLT_IDX_START_BIT);

        o_search_results->m_hitOrMiss[psNum] = (value >> NLM11KDEV_CMP_RSLT_SMF_BIT)
        										         &
        										NLM11KDEV_CMP_RSLT_SMF_MASK;

        if( o_search_results->m_hitOrMiss[psNum] == NLM11KDEV_HIT )
        {
        	/* Get the search type here. Interpretation of result is different with _STANDARD
        	         * compared with _SAHASRA. (PHMD)
        	         */
        	searchType = shadowDev_p->m_ltr[ltrNum].m_miscelleneous.m_searchType[psNum];
        	if( searchType == NLM11KDEV_SAHASRA )
        	{
        		/* Bits[22:0] is the index. DevId does not have any meaning with Sahasra */
        		o_search_results->m_hitIndex[psNum] = value & NLM11KDEV_CMP_FIB_RSLT_IDX_MASK;
                o_search_results->m_hitDevId[psNum] = 0;
        	}
        	else
        	{
        		/* For Standard ACL searches, Bits[22:21] indicates Device ID and Bits[20:0]
        		         * is the hit index.
        		         */
        		o_search_results->m_hitDevId[psNum] = (nlm_u8)((value >>   \
        												NLM11KDEV_CMP_RSLT_DEVID_START_BIT)
        												  &    \
        												NLM11KDEV_CMP_RSLT_DEVID_MASK);
        		o_search_results->m_hitIndex[psNum] = value & NLM11KDEV_CMP_ACL_RSLT_IDX_MASK;
        	}
        }

    }

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;
    return NLMERR_OK;
}

/*
	Nlm11kDevMgr__Compare2 is a search instruction. It searches the input key into the
	database depending on Ltr profile set number (there are 32 sets of Ltr registers)
	and gives hit/miss and address as an output.
    Compare2 instruction can be used for searches with individual search key lengths of upto 640
    User should see the reason 	code in case of failure.
*/
NlmErrNum_t Nlm11kDevMgr__Compare2(
	Nlm11kDevMgr				*self,
	nlm_u8						ltrNum,				/* LTR profile set number to be used */
	Nlm11kDevCtxBufferInfo		*cbInfo,			/* see the structure description above */
	Nlm11kDevCmpRslt			*o_search_results,	/* see the structure description above */
	NlmReasonCode*				o_reason
	)
{
	Nlm11kDev  *dev_p = NULL;
	Nlm11kDevShadowDevice *shadowDev_p = NULL;
	Nlm11kDevSearchType   searchType;
    NlmXptRqt* rqt_p = NULL;
    nlm_u8 cmpRslts[NLM11KDEV_CMP_RSLT_LEN_IN_BYTES * NLM11KDEV_NUM_PARALLEL_SEARCHES];
    nlm_u32 value;
    nlm_32 psNum;

	if(!NLM11K_IS_VALID_DEVMGR_PTR(self))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEVMGR_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != self->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
    if(ltrNum >= NLM11KDEV_NUM_LTR_SET)
    {
        if(o_reason)
			*o_reason = NLMRSC_INVALID_LTR_NUM;
		return NLMERR_FAIL;
    }
	if(NULL == cbInfo)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DATA;
		return NLMERR_NULL_PTR;
	}
    if(o_search_results == NULL)
    {
        if(o_reason)
			*o_reason = NLMRSC_INVALID_SRCH_RSLT_PTR;
		return NLMERR_NULL_PTR;
    }
    if(cbInfo->m_datalen == 0
        || cbInfo->m_datalen > NLM11KDEV_MAX_CB_WRITE_IN_BYTES)
    {
        /* Datalen cannot be 0 or more than 80bytes(640b) */
        if(o_reason)
			*o_reason = NLMRSC_INVALID_INPUT;
		return NLMERR_FAIL;
    }
    if(cbInfo->m_cbStartAddr >= NLM11KDEV_CB_DEPTH)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_CB_ADDRESS;
		return NLMERR_FAIL;
	}

	/* preparing request for Compare operation. */
    rqt_p = NlmXpt__GetRequest(self->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_CBWRITE_CMP2_BITS_8_6;
	rqt_p->m_opcode[1] = ltrNum;

    /* In case of CB write and Compare, valid xpt data is upto datalen specified by user; */
	rqt_p->m_data_len = cbInfo->m_datalen;

    /* In case of Compare operation; XPT layer send the four search rslts via "result" field of xpt rqt;
    Each search result if of 4 bytes (32b) with Byte[3-0] containing result for PS 0,
    Byte[7-4] containing result for PS 1, Byte[11-8] containing result for PS 2
    and Byte[15-12] containing result for PS 3 */
	rqt_p->m_result = cmpRslts;

    /* writing CB Address into m_ctx_addr field of request structure;
    Note: There will be only 1 CB Address always */
    rqt_p->m_ctx_addr[0] = cbInfo->m_cbStartAddr;

#ifdef NLM_NO_MEMCPY_IN_XPT
	/* CB Data pointer by the data_p field of request structure */
	rqt_p->m_data_p = cbInfo->m_data;

#else
	/* copy CB Data into data field of request structure */
	NlmCm__memcpy(rqt_p->m_data, cbInfo->m_data, cbInfo->m_datalen);
#endif

	 /* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(self->m_xpt_p, o_reason);
	if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            NlmXpt__DiscardResult(self->m_xpt_p, rqt_p, NULL);
			return NLMERR_FAIL;
        }
	}

    /* calling transport layer to get the results of the current request */
    if(NULL == NlmXpt__GetResult(self->m_xpt_p, rqt_p, NULL))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
		return NLMERR_NULL_PTR;
	}

    /* Get the Shadow Memory for Misc Reg access. As this register is updated in all
	  * the cascaded devices, gettting any device would do. Let's choose dev#0 (PHMD).
	  */
	dev_p = (Nlm11kDev *)self->m_devList_pp[0];
	shadowDev_p = dev_p->m_shadowDevice_p;

    /* Decode the result field of XPT Structure and return appropriate search results */
    for(psNum = 0; psNum < NLM11KDEV_NUM_PARALLEL_SEARCHES; psNum++)
    {
        /* each parallel search result is of 32bits,  Bit[30] indicates Hit/Miss */
        value = ReadBitsInArrray(cmpRslts + (psNum * NLM11KDEV_CMP_RSLT_LEN_IN_BYTES),
                                	NLM11KDEV_CMP_RSLT_LEN_IN_BYTES,
                                	NLM11KDEV_CMP_RSLT_IDX_END_BIT,
                                	NLM11KDEV_CMP_RSLT_IDX_START_BIT);

        o_search_results->m_hitOrMiss[psNum] = (value >> NLM11KDEV_CMP_RSLT_SMF_BIT)
        										         &
        										NLM11KDEV_CMP_RSLT_SMF_MASK;

        if( o_search_results->m_hitOrMiss[psNum] == NLM11KDEV_HIT )
        {
        	/* Get the search type here. Interpretation of result is different with _STANDARD
        	         * compared with _SAHASRA. (PHMD)
        	         */
        	searchType = shadowDev_p->m_ltr[ltrNum].m_miscelleneous.m_searchType[psNum];
        	if( searchType == NLM11KDEV_SAHASRA )
        	{
        		/* Bits[22:0] is the index. DevId does not have any meaning with Sahasra */
        		o_search_results->m_hitIndex[psNum] = value & NLM11KDEV_CMP_FIB_RSLT_IDX_MASK;
                o_search_results->m_hitDevId[psNum] = 0;
        	}
        	else
        	{
        		/* For Standard ACL searches, Bits[22:21] indicates Device ID and Bits[20:0]
        		         * is the hit index.
        		         */
        		o_search_results->m_hitDevId[psNum] = (nlm_u8)((value >>   \
        												NLM11KDEV_CMP_RSLT_DEVID_START_BIT)
        												  &    \
        												NLM11KDEV_CMP_RSLT_DEVID_MASK);
        		o_search_results->m_hitIndex[psNum] = value & NLM11KDEV_CMP_ACL_RSLT_IDX_MASK;
        	}
        }

    }

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;
    return NLMERR_OK;
}

#endif /* NLM_11K_MODE */

/* ============================================================================
									SS - ST
   ============================================================================ */
#define NLM11KDEV_SS_ST			(0xE0000)
#define NLM11KDEV_SS_AD(x)			(NLM11KDEV_SS_ST + ((x) * 0x1000))

#define NLM11KDEV_ST_SLR_ST		0
#define NLM11KDEV_ST_SLR_EN   		8
#define NLM11KDEV_ST_SP_ST  		9
#define NLM11KDEV_ST_SP_EN			17
#define NLM11KDEV_ST_SSI_ST		18
#define NLM11KDEV_ST_SSI_EN        25
#define NLM11KDEV_ST_ABS_ST		26
#define NLM11KDEV_ST_ABS_EN	    32
#define NLM11KDEV_ST_ABC_ST       	33
#define NLM11KDEV_ST_ABC_EN        35
#define NLM11KDEV_ST_SSB_ST     	36
#define NLM11KDEV_ST_SSB_EN        38

#define NLM11KDEV_ST_BW_ST			62
#define NLM11KDEV_ST_BW_EN        	63

#define NLM11KDEV_ST_LPR_VD		(NLM11KDEV_REG_LEN_IN_BITS + 0)
#define NLM11KDEV_ST_LPRIX_ST		(NLM11KDEV_REG_LEN_IN_BITS + 1)
#define NLM11KDEV_ST_LPRIX_EN		(NLM11KDEV_REG_LEN_IN_BITS + 23)


static void 
Nlm11kDevMgr_pvt_PrepareSTEOutput(nlm_u8	*st_dt,
							   Nlm11kDevSTE *st_p
							   )
{

	NlmCmAssert((st_dt != NULL), "Invalid st_dt pointer provided.\n");
	NlmCmAssert((st_p != NULL), "Invalid SS_ST pointer provided.\n");

	st_p->m_slr = (nlm_u16) ReadBitsInArrray(st_dt,
								NLM11KDEV_SS_WIDBYT, NLM11KDEV_ST_SLR_EN, NLM11KDEV_ST_SLR_ST);

	st_p->m_ssi_s = (nlm_u16) ReadBitsInArrray(st_dt,
								NLM11KDEV_SS_WIDBYT, NLM11KDEV_ST_SP_EN, NLM11KDEV_ST_SP_ST);

	st_p->m_ssi = (nlm_u8) ReadBitsInArrray(st_dt,
								NLM11KDEV_SS_WIDBYT, NLM11KDEV_ST_SSI_EN, NLM11KDEV_ST_SSI_ST);

	st_p->m_ss_abs =  (nlm_u8) ReadBitsInArrray(st_dt,
								NLM11KDEV_SS_WIDBYT, NLM11KDEV_ST_ABS_EN, NLM11KDEV_ST_ABS_ST);

	st_p->m_abc =  (nlm_u8) ReadBitsInArrray(st_dt,
								NLM11KDEV_SS_WIDBYT, NLM11KDEV_ST_ABC_EN, NLM11KDEV_ST_ABC_ST);

	st_p->m_ss_bsel =  (nlm_u8) ReadBitsInArrray(st_dt,
								NLM11KDEV_SS_WIDBYT, NLM11KDEV_ST_SSB_EN, NLM11KDEV_ST_SSB_ST);

	st_p->m_bix =  (nlm_u32) ReadBitsInArrray(st_dt,
								NLM11KDEV_SS_WIDBYT, NLM11KDEV_ST_BIX_EN, NLM11KDEV_ST_BIX_ST);

	st_p->m_bw = (nlm_u8)ReadBitsInArrray(st_dt,
								NLM11KDEV_SS_WIDBYT, NLM11KDEV_ST_BW_EN, NLM11KDEV_ST_BW_ST);

	st_p->m_lprv = (nlm_u8)ReadBitsInArrray(st_dt, 
								NLM11KDEV_SS_WIDBYT, NLM11KDEV_ST_LPR_VD,	NLM11KDEV_ST_LPR_VD);

	st_p->m_lpri = (nlm_u32)ReadBitsInArrray(st_dt,
								NLM11KDEV_SS_WIDBYT, NLM11KDEV_ST_LPRIX_EN, NLM11KDEV_ST_LPRIX_ST);

}

static void 
Nlm11kDevMgr_pvt_PrepareSTEInput(Nlm11kDevSTE* st_p,
							  nlm_u8* data_p
							  )
{
	NlmCmAssert((data_p != NULL), "Invalid data pointer provided.\n");
	NlmCmAssert((st_p != NULL), "Invalid SS_ST pointer provided.\n");


	WriteBitsInArray(data_p, NLM11KDEV_SS_WIDBYT, 
		NLM11KDEV_ST_SLR_EN, NLM11KDEV_ST_SLR_ST, st_p->m_slr);

	WriteBitsInArray(data_p, NLM11KDEV_SS_WIDBYT, 
		NLM11KDEV_ST_SP_EN, NLM11KDEV_ST_SP_ST, st_p->m_ssi_s);

	WriteBitsInArray(data_p, NLM11KDEV_SS_WIDBYT, 
		NLM11KDEV_ST_SSI_EN, NLM11KDEV_ST_SSI_ST, st_p->m_ssi);

	WriteBitsInArray(data_p, NLM11KDEV_SS_WIDBYT,
        NLM11KDEV_ST_ABS_EN, NLM11KDEV_ST_ABS_ST, st_p->m_ss_abs);

	WriteBitsInArray(data_p, NLM11KDEV_SS_WIDBYT,
		NLM11KDEV_ST_ABC_EN, NLM11KDEV_ST_ABC_ST,	st_p->m_abc);

	WriteBitsInArray(data_p, NLM11KDEV_SS_WIDBYT, 
		NLM11KDEV_ST_SSB_EN, NLM11KDEV_ST_SSB_ST, st_p->m_ss_bsel);

	WriteBitsInArray(data_p, NLM11KDEV_SS_WIDBYT,
		NLM11KDEV_ST_BIX_EN, NLM11KDEV_ST_BIX_ST, st_p->m_bix);

	WriteBitsInArray(data_p, NLM11KDEV_SS_WIDBYT,
		NLM11KDEV_ST_BW_EN, NLM11KDEV_ST_BW_ST, st_p->m_bw);
	
	WriteBitsInArray(data_p, NLM11KDEV_SS_WIDBYT,
		NLM11KDEV_ST_LPR_VD, NLM11KDEV_ST_LPR_VD,	st_p->m_lprv);

	WriteBitsInArray(data_p, NLM11KDEV_SS_WIDBYT, 
		NLM11KDEV_ST_LPRIX_EN, NLM11KDEV_ST_LPRIX_ST, st_p->m_lpri);
}

/*
Function: Nlm11kDevMgr__STR
Description: 
User should see the reason code in case of failure.
*/
NlmErrNum_t Nlm11kDevMgr__STR(
	Nlm11kDev         *dev,
	nlm_u8		   st_bn,		
	nlm_u16		   st_ad,		
	Nlm11kDevSTE	   *o_dt,		
	NlmReasonCode  *o_reason
	)
{
	NlmXptRqt* rqt_p = NULL;
	nlm_32     address =0;
    nlm_u8 iter, offset, readData[NLM11KDEV_REG_LEN_IN_BYTES + 1];/* Extra byte is for control bits
    																	 such as VBIT, parity */
	nlm_u8  st_dt[2 * NLM11KDEV_REG_LEN_IN_BYTES];
	NlmReasonCode dummyVar;

	/* If o_reason is NULL then assign it with the address of dummy local variable.
	   It will ensure that o_reason can't be NULL afterwards and no need of checking
	   it for NULL again and again.  This will result in lesser foot print. */

	if(o_reason == NULL)
		o_reason = &dummyVar;

	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		*o_reason = NLMRSC_INVALID_DEV_PTR;

		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;

		return NLMERR_FAIL;
	}
	if(NULL == o_dt)
	{
		*o_reason = NLMRSC_INVALID_DATA;

		return NLMERR_NULL_PTR;
	}
	if((st_bn >= NLM11KDEV_SS_NUM) || (st_ad >= NLM11KDEV_SS_SEP))
	{
		*o_reason = NLMRSC_INVALID_REG_ADDRESS;

		return NLMERR_FAIL;
	}

	/* Two 80b segments are read. MSB 80b of st_dt is for Higher segement
	  * and LSN 80b for Lower segment.
	  */
	NlmCm__memset(st_dt, 0, (2 * NLM11KDEV_REG_LEN_IN_BYTES));

	/* find the physical address of the ST memory and then stitch the devid. */
	address = NLM11KDEV_SS_AD(st_bn);
	address = NLM11K_DEVMGR_INSERT_DEVID(address, dev->m_devId);

	/* Address where 80b MSB of ST data needs to be written*/
	address += ( (st_ad * 2) + 1 );

	/* Write 2 read request for upper and lower 80 bits of ST Entry.
	     First request reads MSB 80 bits and second request reads LSB 80 bits of ST Entry.
	*/
	for(offset = 0, iter = 0; iter < 2; iter++, offset += NLM11KDEV_REG_LEN_IN_BYTES)
	{
		/* preparing read request for register. */
		rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
		if(NULL == rqt_p)
		{
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;

			return NLMERR_NULL_PTR;
		}

		/* Clearing the rqt structure */
    	NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

		/* assigning request structure members */
		rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_READ_X_BITS_8_6;
		rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_READ_X_BITS_5_0;

		rqt_p->m_data_len = NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

		/* In case of Reg read; XPT layer send the 80b read data via "result" field of xpt rqt;
		  * In addition to this result will contain some control bits such as vbit and parity
		  * which are ignored */
		rqt_p->m_result = readData;

#ifdef NLM_NO_MEMCPY_IN_XPT
		rqt_p->m_address = address;
#else
		/* In case of Reg Read, Byte[3:0] of the "data" field of xpt rqt will contain the address*/
		WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_REG_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
				address
				);
#endif

		/* calling transport layer to wrok on the current request */
		NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
		if(NLMRSC_REASON_OK != *o_reason)
		{
			/* If Service Request has failed then discard the result so that xpt rqt is freed*/
            NlmXpt__DiscardResult(dev->m_devMgr_p->m_xpt_p, rqt_p, NULL);

			return NLMERR_FAIL;
		}

		/* calling transport layer to get the results of the current request */
		if(NULL == NlmXpt__GetResult(dev->m_devMgr_p->m_xpt_p, rqt_p, NULL))
		{
			*o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;

			return NLMERR_NULL_PTR;
		}

		/* Copy the data read back into the local array for later complete processing.
		  * first byte of data read back has few control fields which are ignored.
		  */
		NlmCm__memcpy( (st_dt + offset), (readData + 1), NLM11KDEV_REG_LEN_IN_BYTES);

		/* Next address - Address of 80 bit LSB of ST entry */
		address--;
	}

	/* Converting the 160 bit result data to the output data container */
	Nlm11kDevMgr_pvt_PrepareSTEOutput(st_dt, o_dt);

	return NLMERR_OK;
}

/*
Function: Nlm11kDevMgr__STW
Description: 
User should see the reason code in case of failure.
*/

NlmErrNum_t Nlm11kDevMgr__STW(
	Nlm11kDev       	*dev,
	nlm_u8			st_bn,		
	nlm_u16			st_ad,		
	Nlm11kDevSTE	    *st_p,		
	Nlm11kDevSTWrTyp   st_wt, 	    
	NlmReasonCode	*o_reason
	)
{
    NlmXptRqt* rqt_p = NULL;
	nlm_u32 address = 0, iter = 0, offset = 0, numOf80bWrite = 2;
	nlm_u8  st_dt[NLM11KDEV_REG_LEN_IN_BYTES * 2];   /* ST datas corrosponds to 2 REG data*/
	NlmReasonCode dummyVar;

	/* If o_reason is NULL then assign it with the address of dummy local variable.
	   It will ensure that o_reason can't be NULL afterwards and no need of checking
	   it for NULL again and again.  This will result in lesser foot print. */

	if(o_reason == NULL)
		o_reason = &dummyVar;

	if(!NLM11K_IS_VALID_DEV_PTR(dev))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(NULL == st_p)
	{
		*o_reason = NLMRSC_INVALID_DATA;
		return NLMERR_NULL_PTR;
	}
	if((st_bn >= NLM11KDEV_SS_NUM) || (st_ad >= NLM11KDEV_SS_SEP))
	{
		*o_reason = NLMRSC_INVALID_REG_ADDRESS;
		return NLMERR_FAIL;
	}

	NlmCm__memset(st_dt, 0x00, (NLM11KDEV_REG_LEN_IN_BYTES * 2));

	/* Prepare ST data of 160b from input ST Entry fields */
	Nlm11kDevMgr_pvt_PrepareSTEInput(st_p, st_dt);

	/* find the physical address of the ST memory and then stitch the devid. */
	address = NLM11KDEV_SS_AD(st_bn);
    address = NLM11K_DEVMGR_INSERT_DEVID(address, dev->m_devId);

	/* Address where 80b MSB of ST data needs to be written*/
	address += ( (st_ad * 2) + 1 );

	if(st_wt == NLM11KDEV_ST_WRLO)
	{
		/* Lower 80b needs to be write, so decrement the address and
		set the offset to get lower 80b data */
		numOf80bWrite = 1;
		address--;
		offset = NLM11KDEV_REG_LEN_IN_BYTES;
	}
	else if(st_wt == NLM11KDEV_ST_WRHI)
	{
		numOf80bWrite = 1;
		offset = 0;
	}

	/* Write write request for upper and/or lower 80 bits of ST Entry */
	for(iter = 0; iter < numOf80bWrite; iter++)
	{
        /* preparing read request for register. */
		rqt_p = NlmXpt__GetRequest(dev->m_devMgr_p->m_xpt_p, NULL);
		if(NULL == rqt_p)
		{
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
			return NLMERR_NULL_PTR;
		}

		/* Clearing the rqt structure */
    	NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

		/* assigning request structure members */
		rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_WRITE_BITS_8_6;
		rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_WRITE_BITS_5_0;

		/* length of the register data */
		rqt_p->m_data_len = NLM11KDEV_REG_LEN_IN_BYTES;

    	/* In case of Reg Write; "result" field of xpt rqt will be NULL */
		rqt_p->m_result = NULL;

#ifdef NLM_NO_MEMCPY_IN_XPT
		rqt_p->m_address = address;

		/* pointer will point to data to be written to the data bus of request structure*/
		rqt_p->m_data_p = (st_dt + offset);
#else
		/* In case of Reg write, Byte[3:0] of the "data" field of xpt rqt
		   will contain the address and Byte[13:4] will contain the 80b register data */
		rqt_p->m_data_len += NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

		/* Byte[3:0] of the "data" field of xpt rqt will contain the address*/
		WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_REG_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
				address
				);

		/* copying data to be written to the data bus of request structure*/
		NlmCm__memcpy( (rqt_p->m_data + NLM11KDEV_REG_ADDR_LEN_IN_BYTES),
				(st_dt + offset),
				NLM11KDEV_REG_LEN_IN_BYTES );
#endif

        /* calling transport layer to wrok on the current request */
        NlmXpt__ServiceRequests(dev->m_devMgr_p->m_xpt_p, o_reason);
        if(NLMRSC_REASON_OK != *o_reason)
		{
        	/* If Service Request has failed then discard the result so that xpt rqt is freed*/
            NlmXpt__DiscardResult(dev->m_devMgr_p->m_xpt_p, rqt_p, NULL);

            return NLMERR_FAIL;
        }

        /* Next address - Address of 80 bit LSB of ST entry */
		address--;
		offset = NLM11KDEV_REG_LEN_IN_BYTES;

    }

	if(NLMRSC_REASON_OK != *o_reason)
		return NLMERR_FAIL;

	return NLMERR_OK;
}

/*
Function: Nlm11kDevMgr__RegisterWrite
Description: Writes 80b data to specified address register (only for Global[R/W]/LTR).
	dev		: Divice pointer:
	address : Address of the register
	data	: 80b data to write 

User should see the reason code in case of failure.
*/
 NlmErrNum_t Nlm11kDevMgr__RegisterWrite(
	void	 					*dev,
	nlm_u32						address,		
	void					    *data,
	NlmReasonCode*				o_reason
	)
 {
	NlmXptRqt* rqt_p = NULL;
	nlm_u8 *data_ptr = NULL;
	Nlm11kDev *dev_p;

	if(NULL == dev)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_INPUT;
		return NLMERR_NULL_PTR;
	}

	dev_p = (Nlm11kDev*)dev; /* check the device pointer */
	if(!NLM11K_IS_VALID_DEV_PTR(dev_p))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev_p->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	/* Read only Register; Return Error */
	if(address == NLM11K_REG_ADDR_DEVICE_ID ||
	   address == NLM11K_REG_ADDR_RESULT0 || address == NLM11K_REG_ADDR_RESULT1 ||
	   address == NLM11K_REG_ADDR_DETAILED_PARITY_ERROR_INFO)
	{
		if(o_reason)
			*o_reason = NLMRSC_READONLY_REGISTER;
		return NLMERR_FAIL;
	}
	if(NULL == data)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DATA;
		return NLMERR_NULL_PTR;
	}
    
	/* preparing request for register write. */
    rqt_p = NlmXpt__GetRequest(dev_p->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_WRITE_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_WRITE_BITS_5_0;

    /* length of the register data */
	rqt_p->m_data_len = NLM11KDEV_REG_LEN_IN_BYTES;

#ifdef NLM_NO_MEMCPY_IN_XPT
	data_ptr = data;
#else
	/* In case of Reg write, Byte[3:0] of the "data" field of xpt rqt
     will contain the address and Byte[13:4] will contain the 80b register data */
	rqt_p->m_data_len += NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

	/* pointer to the register data : Byte[13:4]*/
	data_ptr = rqt_p->m_data + NLM11KDEV_REG_ADDR_LEN_IN_BYTES;
	NlmCm__memcpy(data_ptr, data, NLM11KDEV_REG_LEN_IN_BYTES);
#endif

    /* In case of Reg Write; "result" field of xpt rqt will be NULL */
	rqt_p->m_result = NULL;

    /* Insert the devid into the address */
    address = NLM11K_DEVMGR_INSERT_DEVID(address, dev_p->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
	rqt_p->m_address = address;
	rqt_p->m_data_p = data_ptr;
#else
	/* Byte[3:0] of the "data" field of xpt rqt will contain the address*/
	WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_REG_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
                address
				);
#endif
	/* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev_p->m_devMgr_p->m_xpt_p, o_reason);
	if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
			return NLMERR_FAIL;
	}

	return NLMERR_OK;
}


/*
Function: Nlm11kDevMgr__RegisterRead
Description: Read 80b data from specified address register (only for Global/LTR).
	dev		: Divice pointer:
	address : Address of the register
	o_data	: 80b data read 

User should see the reason code in case of failure.
*/
NlmErrNum_t Nlm11kDevMgr__RegisterRead(
	void	 					*dev,
	nlm_u32						address,		
	void					    *o_data, 
	NlmReasonCode*				o_reason
	)
 {
	NlmXptRqt* rqt_p = NULL;	
	Nlm11kDev *dev_p;
	nlm_u8 readData[NLM11KDEV_REG_LEN_IN_BYTES + 1];/* Extra byte is for control bits such as VBIT, parity */

	if(NULL == dev)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_INPUT;
		return NLMERR_NULL_PTR;
	}

	dev_p = (Nlm11kDev*)dev; /* check the device pointer */
	if(!NLM11K_IS_VALID_DEV_PTR(dev_p))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DEV_PTR;
		return NLMERR_NULL_PTR;
	}
    if(NlmTrue != dev_p->m_devMgr_p->m_isLocked)
	{
		if(o_reason)
			*o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
		return NLMERR_FAIL;
	}
	if(NULL == o_data)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_OUTPUT;
		return NLMERR_NULL_PTR;
	}
    
	/* preparing request for register read. */
    rqt_p = NlmXpt__GetRequest(dev_p->m_devMgr_p->m_xpt_p, NULL);
	if(NULL == rqt_p)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
		return NLMERR_NULL_PTR;
	}
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

	/* assigning request structure members */
	rqt_p->m_opcode[0] = NLM11K_OPCODE_PIO_READ_X_BITS_8_6;
	rqt_p->m_opcode[1] = NLM11K_OPCODE_PIO_READ_X_BITS_5_0;

    rqt_p->m_data_len = NLM11KDEV_REG_ADDR_LEN_IN_BYTES;

    /* In case of Reg read; XPT layer send the 80b read data via "result" field of xpt rqt;
    In addition to this result will contain some control bits such as vbit and parity which are ignored */
	rqt_p->m_result = readData;

	/* Insert the devid into the address */
    address = NLM11K_DEVMGR_INSERT_DEVID(address, dev_p->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
	rqt_p->m_address = address;
#else
	/* In case of Reg Read, Byte[3:0] of the "data" field of xpt rqt will contain the address*/
	WriteBitsInArray(
				rqt_p->m_data,
				NLM11KDEV_REG_ADDR_LEN_IN_BYTES,
				(NLM11KDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
				0,
                address
				);
#endif

    /* calling transport layer to work on the current request */
	NlmXpt__ServiceRequests(dev_p->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
	{
		if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            NlmXpt__DiscardResult(dev_p->m_devMgr_p->m_xpt_p, rqt_p, NULL);
			return NLMERR_FAIL;
        }
	}

    /* calling transport layer to get the results of the current request */
    if(NULL == NlmXpt__GetResult(dev_p->m_devMgr_p->m_xpt_p, rqt_p, NULL))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
		return NLMERR_NULL_PTR;
	}

    /* extracting register contents from result field of request structure;
       Note: Byte[0] contains some control fields which can be ignored */
	NlmCm__memcpy(o_data, &readData[1], NLM11KDEV_REG_LEN_IN_BYTES);
    
	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;
	return NLMERR_OK;
}

void Nlm11kDevMgr__DumpShadowMemory(
	nlm_u8			startDeviceNum,
	nlm_u8			endDeviceNum,
	nlm_u16			startBlkNum,
	nlm_u16			endBlkNum,
	NlmBool			printRegs
	)
 {
 	char fileName[NLM11K_DUMP_MAX_FILENAME_LEN] = {0};
 	char fileNum[NLM11K_DUMP_MAX_FILENAME_LEN] = {0};
 	nlm_u32 i = 0;
 	NlmBool isSahasraMode = NlmFalse;
 	Nlm11kDev *dev_p = NULL;

 	if(!g_devMgr_p)
 		return;

 	if(startDeviceNum > endDeviceNum)
 		return;

	if(g_is10MDev) /* using 10M device */
	{
		if(startBlkNum >= NLM11KDEV_10M_NUM_ARRAY_BLOCKS)
			startBlkNum = 0;

		if(endBlkNum >= NLM11KDEV_10M_NUM_ARRAY_BLOCKS)
			endBlkNum  = NLM11KDEV_10M_NUM_ARRAY_BLOCKS - 1;
	}

 	if(g_devMgr_p->m_operMode == NLM11KDEV_OPR_SAHASRA)
 		isSahasraMode = NlmTrue;

 	for(i = 0; i < g_devMgr_p->m_devCount; ++i)
 	{
 		if(i >= startDeviceNum && i <= endDeviceNum)
 		{
 			NlmCm__strcpy(fileName, "NLM11K_ShadowDevice_");

 			NlmCm__sprintf(fileNum, "%d", i);

 			NlmCm__strcat(fileName, fileNum);


 			NlmCm__strcat(fileName, "_Blks_");

 			NlmCm__sprintf(fileNum, "%d", startBlkNum);

 			NlmCm__strcat(fileName, fileNum);

 			NlmCm__strcat(fileName, "_");

 			NlmCm__sprintf(fileNum, "%d", endBlkNum);

 			NlmCm__strcat(fileName, fileNum);


 			NlmCm__strcat(fileName, ".txt");


 			dev_p = g_devMgr_p->m_devList_pp[i];

			if(dev_p->m_shadowDevice_p)
			{
 				Nlm11kDevMgr__DisplayShadowDevice(dev_p, isSahasraMode, fileName,
 												startBlkNum, endBlkNum, printRegs);
			}
 		}
 	}
 }

