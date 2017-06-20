/*
 * $Id: nlmdevmgr.c,v 1.1.6.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 
#include "nlmarch.h"
#include "nlmdevmgr.h"
#include "nlmcmutility.h"
#include "nlmxpt.h"
#include "nlmcmstring.h"

#ifdef NLM_12K_11K
#include<nlmdevmgr/nlmdevmgr.h>
#endif

#define NLMDEV_MAX_UDA_DATA_LEN_12K_11K_IN_BYTES      74
/*******************************************************************************/
/*
File Description:
This file handles the device specific details of SDK.
Device manager module is responsible to create, manage and destroy the
device or a cascade system of the same.
This file contains implementations of following Device manager APIs.

    kbp_dm_init
    kbp_dm_destroy
    kbp_dm_add_device
    kbp_dm_reset_devices
    kbp_dm_global_reg_read
    kbp_dm_global_reg_write
    kbp_dm_cb_reg_read
    kbp_dm_cb_reg_write
    kbp_dm_ltr_read
    kbp_dm_ltr_write
    kbp_dm_block_reg_read
    kbp_dm_block_reg_write
    kbp_dm_dba_read
    kbp_dm_dba_write
    kbp_dm_dba_invalidate
    kbp_dm_cbwcmp1
    kbp_dm_cbwcmp2
    kbp_dm_cbwlpm
    kbp_dm_multi_compare
    kbp_dm_uda_read
    kbp_dm_uda_write
    kbp_dm_cb_write
    kbp_dm_lock_config
    kbp_dm_shadow_create
    kbp_dm_shadow_destroy
    kbp_dm_generic_reg_write
    kbp_dm_generic_reg_read
    kbp_dm_cmd_send
    kbp_dm_em_write
    kbp_dm_em_read

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
maintained in NlmDevMgr structure. This file does not handle multi
channel case, so one Device Manager can handle only one cascade system.
Multiple channels must be managed in GTM/User_Application.

File position in the full integrated system:

|-----------------------|       |-----------------------|       |-----------------------|
|                       |       |                       |       |                       |
|Generic Table Manager  |       |   User Application    |       |   Range Manager       |
|                       |       |                       |       |                       |
|-----------------------|       |-----------------------|       |-----------------------|
            |                               |                               |
            |                               |                               |
            |                               |                               |
            |                   ____________v____________                   |
            |-----------------> |                       |<------------------|
                                |Device Manager.h/.c    |
                                |                       |
                                |_______________________|
                                            |
                                            v
                                |-----------------------|
                                |                       |
                                |   Transport Layer     |
                                |                       |
                                |-----------------------|

*/
/*******************************************************************************/

#define NLM_DEVMGR_INSERT_DEVID(addr, devId) \
    (devId << 23) | (addr);

#define NLM_DEVMGR_PTR_MAGIC_NUM   0x54321
#define NLM_DEV_PTR_MAGIC_NUM   0x54320

#define NLM_IS_VALID_DEVMGR_PTR(devMgr) ((devMgr) && (devMgr)->m_magicNum == NLM_DEVMGR_PTR_MAGIC_NUM)
#define NLM_IS_VALID_DEV_PTR(dev) ((dev) && (dev)->m_magicNum == NLM_DEV_PTR_MAGIC_NUM)

/* LTR Misc Reg search type start and end bits. (PHMD) */
#define NLMDEV_MISCREG_SEARCH_START_BIT 48
#define NLMDEV_MISCREG_SEARCH_END_BIT   55

/* LTR SS Result Map start and end bits. (PHMD) */
#define NLMDEV_SSREG_SSMAP_START_BIT    9
#define NLMDEV_SSREG_SSMAP_END_BIT  16

/* Result index related defines. (PHMD) */
#define NLMDEV_CMP_RSLT_IDX_START_BIT   (0)
#define NLMDEV_CMP_RSLT_IDX_END_BIT     (30)
#define NLMDEV_CMP_RSLT_DEVID_START_BIT (21)
#define NLMDEV_CMP_RSLT_SMF_BIT         (30)
#define NLMDEV_CMP_RSLT_SMF_MASK            (0x1)
#define NLMDEV_CMP_RSLT_DEVID_MASK       (0x3)
#define NLMDEV_CMP_FIB_RSLT_IDX_MASK        (0x7FFFFF)
#define NLMDEV_CMP_ACL_RSLT_IDX_MASK        (0x1FFFFF)

#define NLM_DUMP_MAX_FILENAME_LEN       (100)

#ifdef NLM_12K_11K 

extern void*
kbp_dm_pvt_11k_map_create_shadow_st(
    NlmCmAllocator *alloc_p);

NlmErrNum_t
kbp_dm_pvt_11k_map_cmp_result(
    Nlm11kDevCmpRslt *cmpResult11k,
    NlmDevCmpResult  *o_cmpResult12k,
    NlmReasonCode       *o_reason
    );

NlmErrNum_t
kbp_dm_pvt_11k_map_global_reg(
    NlmDevGlobalRegType regType,
    Nlm11kDevGlobalRegType *o_11kRegType,
    NlmReasonCode       *o_reason
    );

NlmErrNum_t
kbp_dm_pvt_11k_map_ltrs(
    NlmDevLtrRegType regType,
    Nlm11kDevLtrRegType *o_11kRegType,
    NlmReasonCode       *o_reason
    );

Nlm11kDevMgr* kbp_dm_pvt_11k_init(
   NlmCmAllocator      *alloc_p,
   void                    *xpt_p,
   NlmDevType          devType,
   NlmDevOperationMode  operMode,
   NlmReasonCode       *o_reason
   );

#endif


/*
Function: NlmDevMgr__ctor
Description: constructor of Device Manager. Initializes the memory
allocator pointer and XPT pointer to the corresponding member variables
of the Device Manager. It initializes the device count to zero and a creates
a device list of a length NLMDEV_MAX_DEV_NUM.
*/
static NlmDevMgr* NlmDevMgr__ctor(
    NlmDevMgr           *self,
    void*               xpt_p,
    NlmDevType          devType,
    NlmPortMode     portMode,
    NlmSMTMode         smtMode,
    NlmCmAllocator      *alloc_p
    )
{
    nlm_u32 maxNumOfDev = NLMDEV_MAX_DEV_NUM;
    
    NlmCmAssert((alloc_p != NULL), "Invalid memory allocator provided.\n");
    NlmCmAssert((self != NULL), "Invalid self pointer.\n");

#ifdef NLM_12K_11K 
    maxNumOfDev = NLM11KDEV_MAX_DEV_NUM;
#endif


    self->m_alloc_p    = alloc_p;
    self->m_devCount   = 0;
    self->m_xpt_p      = xpt_p;
    self->m_devType    = devType;
    self->m_portMode  = portMode;
    self->m_smtMode   = smtMode;
    self->m_isLocked   = NLMFALSE;
    self->m_magicNum = NLM_DEVMGR_PTR_MAGIC_NUM;

    self->m_devList_pp = (void**)NlmCmAllocator__calloc(alloc_p,
                                                 maxNumOfDev,
                                                 sizeof(NlmDev*));

    NlmCmDemand((self != NULL), "Out of memory.\n");

    return self;
}

/*
Function: NlmDevMgr__dtor
Description: destructure of Device Manager deletes the list of devices including
all the devices and their shadow devices and frees the memory.
*/
static void NlmDevMgr__dtor(
    NlmDevMgr           *self
    )
{
    nlm_u32 Idx = 0;
    NlmCmAllocator *alloc_p = NULL;
    NlmDev *dev_p;
    
    nlm_u32 maxNumOfDev = NLMDEV_MAX_DEV_NUM;

    if(self == NULL)
        return;

#ifdef NLM_12K_11K 
    maxNumOfDev = NLM11KDEV_MAX_DEV_NUM;
#endif

    alloc_p = self->m_alloc_p;

    /* traverse the device list */
    if(self->m_devList_pp)
    {
        for(Idx =0; Idx < maxNumOfDev; Idx++)
        {
            if(NULL != self->m_devList_pp[Idx])
            {
                dev_p = (NlmDev*)(self->m_devList_pp[Idx]);

                /* deleting shadow device */
                kbp_dm_shadow_destroy(dev_p, NULL);

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

#ifndef NLM_12K_11K
/* NlmDevMgr_pvt_ConstructBlkSelLtrData contructs the 80b Reg Data
 based on various values of the various fields of Blk Select Register provided by application
 */
static NlmErrNum_t NlmDevMgr_pvt_ConstructBlkSelLtrData(
    nlm_u8                  *o_data,
    NlmDevBlkSelectReg  *blkSelectData_p
    )
{
    nlm_32 abNum;
    nlm_u32 value = 0;
    nlm_32 bitSelector = 0;

   /* A Blk Select Reg contains Blk enables for (NLMDEV_NUM_ARRAY_BLOCKS/2) number of array Blks
    i.e.  Blk Select 0 Reg -- Contains Blk Enable for AB Num 0   - 63
          Blk Select 1 Reg -- Contains Blk Enable for AB Num 64  - 127;
          Blk Select 2 Reg -- Contains Blk Enable for AB Num 128 - 191
          Blk Select 3 Reg -- Contains Blk Enable for AB Num 192 - 255;
    Each block uses 1 bit as Enable Bit in the Register */

    /*Since "WriteBitsInArray" can write maximum of 32b in to an array
    we update the Blk Select Reg Data for 32 Blks first and then for the remaining 32 Blks  */
    for(abNum = 0; abNum < (NLMDEV_NUM_ARRAY_BLOCKS / 8); abNum++, bitSelector++)
    {
        if(blkSelectData_p->m_blkEnable[abNum] != NLMDEV_DISABLE)
            value |= (1 << bitSelector);
    }
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 31, 0, value);

    value = 0;
    bitSelector =0;
    for(abNum = (NLMDEV_NUM_ARRAY_BLOCKS / 8); abNum < (NLMDEV_NUM_ARRAY_BLOCKS / 4);
                                                                    abNum++, bitSelector++)
    {
        if(blkSelectData_p->m_blkEnable[abNum] != NLMDEV_DISABLE)
            value |= (1 << bitSelector);
    }
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 63, 32, value);

    return NLMERR_OK;
}

/* NlmDevMgr_pvt_ExtractBlkSelLtrData extracts the various fields  of Blk Select Register
from the 80b Reg Data read from the device */
static void NlmDevMgr_pvt_ExtractBlkSelLtrData(
    nlm_u8                  *readData,
    NlmDevBlkSelectReg  *blkSelectData_p
    )
{
    nlm_32 abNum;
    nlm_u32 value = 0;

    /*Since "ReadBitsInArrray" can read maximum of 32b in to an array
    hence we extract the Blk Select Reg Data for 32 Blks first and then for the remaining 32 Blks  */
    value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 31, 0);
    for(abNum = 0; abNum < (NLMDEV_NUM_ARRAY_BLOCKS/8); abNum++, value >>= 1)
        blkSelectData_p->m_blkEnable[abNum] = (NlmDevDisableEnable)(value & 1);

    value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 63, 32);
    for(abNum = (NLMDEV_NUM_ARRAY_BLOCKS/8); abNum < (NLMDEV_NUM_ARRAY_BLOCKS/4); abNum++, value >>= 1)
        blkSelectData_p->m_blkEnable[abNum] = (NlmDevDisableEnable)(value & 1);
}


/* NlmDevMgr_pvt_ConstructSBKeySelLtrData contructs the 80b Reg Data based on various values
of the various fields of Super Blk Key Select Reg provided by application
 */
static NlmErrNum_t NlmDevMgr_pvt_ConstructSBKeySelLtrData(
    nlm_u8                      *o_data,
    NlmDevSuperBlkKeyMapReg *sbKeySelectData_p
    )
{
    nlm_32 sbNum;
    nlm_u32 value = 0;
    nlm_32 bitSelector = 0;
     /* A Super Blk Key Select Reg contains Key maps for the NLMDEV_NUM_SUPER_BLOCKS super Blocks;
    Since Key Num can be any value from 0 -3 Each super block uses 2 bits in the Register*/

    /*Since "WriteBitsInArray" can write maximum of 32b in to an array
    we update the Key Select Reg Data for 16 super Blks first and then for the remaining 16 super Blks  */
    for(sbNum = 0; sbNum < (NLMDEV_NUM_SUPER_BLOCKS/2); sbNum++, bitSelector += 2)
    {
        if(sbKeySelectData_p->m_keyNum[sbNum] !=  NLMDEV_KEY_0)
        {
            /* Key Value should be 0 - 3 */
            if(sbKeySelectData_p->m_keyNum[sbNum] != NLMDEV_KEY_1
                && sbKeySelectData_p->m_keyNum[sbNum] != NLMDEV_KEY_2
                && sbKeySelectData_p->m_keyNum[sbNum] != NLMDEV_KEY_3)
                return NLMERR_FAIL;

            value |= (sbKeySelectData_p->m_keyNum[sbNum] << bitSelector);
        }
    }
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 31, 0, value);

    value = 0;
    bitSelector = 0;
    for(sbNum = (NLMDEV_NUM_SUPER_BLOCKS/2); sbNum < NLMDEV_NUM_SUPER_BLOCKS; sbNum++, bitSelector += 2)
    {
        if(sbKeySelectData_p->m_keyNum[sbNum] !=  NLMDEV_KEY_0)
        {
            /* Key Value should be 0 - 3 */
            if(sbKeySelectData_p->m_keyNum[sbNum] != NLMDEV_KEY_1
                && sbKeySelectData_p->m_keyNum[sbNum] != NLMDEV_KEY_2
                && sbKeySelectData_p->m_keyNum[sbNum] != NLMDEV_KEY_3)
                return NLMERR_FAIL;

            value |= (sbKeySelectData_p->m_keyNum[sbNum] << bitSelector);
        }
    }
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 63, 32, value);

    return NLMERR_OK;
}


/* NlmDevMgr_pvt_ExtractSBKeySelLtrData extracts the various fields  of Super Blk Key Select Reg
from the 80b Reg Data read from the device */
static void NlmDevMgr_pvt_ExtractSBKeySelLtrData(
    nlm_u8                      *readData,
    NlmDevSuperBlkKeyMapReg *sbKeySelectData_p
    )
{
    nlm_32 sbNum;
    nlm_u32 value = 0;

    /*Since "ReadBitsInArrray" can read maximum of 32b in to an array
    we extract the Key Select Reg Data for 16 super Blks first and then for the remaining 16 super Blks  */
    value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 31, 0);
    for(sbNum = 0; sbNum < (NLMDEV_NUM_SUPER_BLOCKS/2); sbNum++, value >>= 2)
        sbKeySelectData_p->m_keyNum[sbNum] = (NlmDevKey)(value & 0x3);

    value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 63, 32);
    for(sbNum = (NLMDEV_NUM_SUPER_BLOCKS/2); sbNum < NLMDEV_NUM_SUPER_BLOCKS; sbNum++, value >>= 2)
        sbKeySelectData_p->m_keyNum[sbNum] = (NlmDevKey)(value & 0x3);
}


/* NlmDevMgr_pvt_ConstructParallelSrchLtrData contructs the 80b Reg Data based on various values
    of the various fields of Parallel Srch Reg provided by application */
static NlmErrNum_t NlmDevMgr_pvt_ConstructParallelSrchLtrData(
    nlm_u8                      *o_data,
    NlmDevParallelSrchReg   *parallelSrchData_p
    )
{
    nlm_32 abNum;
    nlm_u32 value = 0;
    nlm_32 bitSelector = 0;
     /* A Parallel Srch Reg contains Result Port Maps for the (NLMDEV_NUM_ARRAY_BLOCKS/8) array Blocks;
                 i.e. Parallel Srch 0 Reg -- Contains Blk Enable for AB Num 0   - 31
                      Parallel Srch 1 Reg -- Contains Blk Enable for AB Num 32  - 63
                      Parallel Srch 2 Reg -- Contains Blk Enable for AB Num 64  - 95
                      Parallel Srch 3 Reg -- Contains Blk Enable for AB Num 96  - 127
                      Parallel Srch 4 Reg -- Contains Blk Enable for AB Num 128 - 159
                      Parallel Srch 5 Reg -- Contains Blk Enable for AB Num 160 - 191
                      Parallel Srch 6 Reg -- Contains Blk Enable for AB Num 192 - 223
                      Parallel Srch 7 Reg -- Contains Blk Enable for AB Num 224 - 255
    Since Result Port Num can be any value from 0 - 3 Each array block uses 2 bits in the Register*/

     /*Since "WriteBitsInArray" can write maximum of 32b in to an array
        we update the Parallel Srch Reg Data for 16 array Blks first and
        then for the remaining 16 array Blks  */
    for(abNum = 0; abNum < (NLMDEV_NUM_ARRAY_BLOCKS / 16); abNum++, bitSelector += 2)
    {
        if(parallelSrchData_p->m_psNum[abNum] !=  NLMDEV_PARALLEL_SEARCH_0)
        {
            /* Rslt Port Num Value should be 0 - 3 */
            if(parallelSrchData_p->m_psNum[abNum] != NLMDEV_PARALLEL_SEARCH_1
                && parallelSrchData_p->m_psNum[abNum] != NLMDEV_PARALLEL_SEARCH_2
                && parallelSrchData_p->m_psNum[abNum] != NLMDEV_PARALLEL_SEARCH_3)

                return NLMERR_FAIL;

            value |= (parallelSrchData_p->m_psNum[abNum] << bitSelector);
        }
    }
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 31, 0, value);

    value = 0;
    bitSelector =0;
    for(abNum = (NLMDEV_NUM_ARRAY_BLOCKS / 16);
            abNum < (NLMDEV_NUM_ARRAY_BLOCKS / 8); abNum++, bitSelector += 2)
    {
        if(parallelSrchData_p->m_psNum[abNum] !=  NLMDEV_PARALLEL_SEARCH_0)
        {
            /* Rslt Port Num Value should be 0 - 3 */
            if(parallelSrchData_p->m_psNum[abNum] != NLMDEV_PARALLEL_SEARCH_1
                && parallelSrchData_p->m_psNum[abNum] != NLMDEV_PARALLEL_SEARCH_2
                && parallelSrchData_p->m_psNum[abNum] != NLMDEV_PARALLEL_SEARCH_3)

                return NLMERR_FAIL;

            value |= (parallelSrchData_p->m_psNum[abNum] << bitSelector);
        }
    }
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 63, 32, value);

    return NLMERR_OK;
}


/* NlmDevMgr_pvt_ExtractParallelSrchLtrData extracts the various fields  of Parallel Srch Reg
from the 80b Reg Data read from the device */
static void NlmDevMgr_pvt_ExtractParallelSrchLtrData(
    nlm_u8                      *readData,
    NlmDevParallelSrchReg   *parallelSrchData_p
    )
{
    nlm_32 abNum;
    nlm_u32 value = 0;

     /*Since "ReadBitsInArrray" can read maximum of 32b in to an array
        we extract the Parallel Srch Reg Data for 16 array Blks first and
        then for the remaining 16 array Blks  */
    value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 31, 0);
    for(abNum = 0; abNum < (NLMDEV_NUM_ARRAY_BLOCKS/16); abNum++, value >>= 2)
        parallelSrchData_p->m_psNum[abNum] = (NlmDevParallelSrch)(value & 0x3);

    value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 63, 32);
    for(abNum = (NLMDEV_NUM_ARRAY_BLOCKS/16); abNum < (NLMDEV_NUM_ARRAY_BLOCKS/8); abNum++, value >>= 2)
        parallelSrchData_p->m_psNum[abNum] = (NlmDevParallelSrch)(value & 0x3);
}


/* NlmDevMgr_pvt_ConstructKeyContructLtrData contructs the 80b Reg Data based on various values
    of the various fields of Key Contruct Reg provided by application */
static NlmErrNum_t NlmDevMgr_pvt_ConstructKeyContructLtrData(
    nlm_u8                      *o_data,
    NlmDevKeyConstructReg   *keyContructData_p
    )
{
    nlm_32 segNum = 0;
    nlm_u32 startBit = 0, endBit = 0;
    NlmBool fill7F = NlmFalse;

    /* A Key Construction Reg contains Key Construction details of the Keys;
        There are two KCR for each key with each KCR containing details of 5 segments
        of key contruction. Each Segment requires 7 Bits for Start Byte and 5 Bits for
        Number of Bytes; Valid values of start byte is 0 - NLMDEV_MAX_KEY_LEN_IN_BYTES
        and for Number of Bytes is 1 - 16; If Number of Bytes of any segment is specified
        to be zero then it indicates that next segments needs be ignored 
        */
    
    for(segNum = 0; segNum < NLM_LTR_NUM_OF_SEGMENT_PER_KCR; segNum++)
    {
        startBit = segNum * NLM_KPU_KCR_NUM_BITS_PER_SEGMENT;
        endBit = startBit + NLM_KPU_KCR_NUM_BITS_FOR_START_LOC - 1;

        /* If Number of Bytes = 0, fill the remaining segments of Register with 0x7F */
        if( keyContructData_p->m_numOfBytes[segNum] == 0 ||
            keyContructData_p->m_startByteLoc[segNum] == 0x7F ||
            fill7F )
        {
            fill7F = NlmTrue;

            /* start byte */
            WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, endBit,
                                startBit, 0x7F);

            /* Number of bytes */
            startBit = endBit + 1;
            endBit = startBit + NLM_KPU_KCR_NUM_BITS_FOR_NUM_BYTES - 1;

            WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, endBit,
                                startBit, 0);

            /* Zero Fill */
            startBit = endBit + 1;
            endBit = startBit + NLM_KPU_KCR_NUM_BITS_FOR_ZERO_FILL - 1;

            WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, endBit,
                                startBit, 0);

            continue;
        }

        /* Check the correctness of Start Byte Location */
        if(keyContructData_p->m_startByteLoc[segNum] >= NLMDEV_MAX_KEY_LEN_IN_BYTES)
            return NLMERR_FAIL;

        /* Check the correctness of Number of Bytes */
        if(keyContructData_p->m_numOfBytes[segNum] > 16)
            return NLMERR_FAIL;

        /* Start Byte */
        WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, endBit,
            startBit, keyContructData_p->m_startByteLoc[segNum]);

        /* Number of bytes */
        startBit = endBit + 1;
        endBit = startBit + NLM_KPU_KCR_NUM_BITS_FOR_NUM_BYTES - 1;

        WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, endBit,
            startBit, keyContructData_p->m_numOfBytes[segNum]- 1);

        /* Zero fill -- ignore application provided value, just set it to 0. */
        startBit = endBit + 1;
        endBit = startBit + NLM_KPU_KCR_NUM_BITS_FOR_ZERO_FILL - 1;

        WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, endBit,
            startBit, 0);

    }

    return NLMERR_OK;
}

/* NlmDevMgr_pvt_ExtractKeyContructLtrData extracts the various fields  of Key Construct Reg
from the 80b Reg Data read from the device */
static void NlmDevMgr_pvt_ExtractKeyContructLtrData(
    nlm_u8                      *readData,
    NlmDevKeyConstructReg   *keyContructData_p
    )
{
    nlm_32 segNum = 0;
    nlm_u32 startBit = 0, endBit = 0;

    for(segNum = 0; segNum < NLM_LTR_NUM_OF_SEGMENT_PER_KCR; segNum++)
    {
        startBit = segNum * NLM_KPU_KCR_NUM_BITS_PER_SEGMENT;
        endBit = startBit + NLM_KPU_KCR_NUM_BITS_FOR_START_LOC - 1;

        keyContructData_p->m_startByteLoc[segNum] = (nlm_u8)ReadBitsInArrray(readData,
                                                            NLMDEV_REG_LEN_IN_BYTES,
                                                            endBit,
                                                            startBit);

        startBit = endBit + 1;
        endBit = startBit + NLM_KPU_KCR_NUM_BITS_FOR_NUM_BYTES - 1;

        keyContructData_p->m_numOfBytes[segNum] = (nlm_u8)(ReadBitsInArrray(readData,
                                                        NLMDEV_REG_LEN_IN_BYTES,
                                                        endBit,
                                                        startBit) + 1);

        startBit = endBit + 1;
        endBit = startBit + NLM_KPU_KCR_NUM_BITS_FOR_ZERO_FILL - 1;

        keyContructData_p->m_isZeroFill[segNum] = (nlm_u8) ReadBitsInArrray(readData,
                                                        NLMDEV_REG_LEN_IN_BYTES,
                                                        endBit,
                                                        startBit);

    }

    return;
}


/* NlmDevMgr_pvt_ConstructRangeInsertLtrData contructs the 80b Reg Data based on various values
    of the various fields of Range Insert Reg provided by application */
static NlmErrNum_t NlmDevMgr_pvt_ConstructRangeInsertLtrData(
    nlm_u8          *o_data,
    void            *inputData_p,
    nlm_u32         regType
    )
{
    nlm_32 keyNum;
    nlm_u32 value = 0;
    nlm_u8 maxRangeByte = NLMDEV_RANGE_DO_NOT_INSERT;

    /* A Range Insertion0 Reg contains details for Range A and RangeB about the type of encoding
    used , number of bytes of Range Encoding to be inserted in the keys and location where Range
    Encodings needs to be inserted in each of the keys. Range Insertion1 Reg contains similar
    details for Range C and Range D*/
    if(regType == NLMDEV_RANGE_INSERTION_0_LTR)
    {
        NlmDevRangeInsertion0Reg *rangeInsertData_p =  (NlmDevRangeInsertion0Reg*)inputData_p;

        /* Valid values for Type of Range Encoding is 0 - 2*/
        if(rangeInsertData_p->m_rangeAEncodingType != NLMDEV_3BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeAEncodingType != NLMDEV_2BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeAEncodingType != NLMDEV_NO_RANGE_ENCODING)
            return NLMERR_FAIL;

        if(rangeInsertData_p->m_rangeBEncodingType != NLMDEV_3BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeBEncodingType != NLMDEV_2BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeBEncodingType != NLMDEV_NO_RANGE_ENCODING)
            return NLMERR_FAIL;

        /* Valid values for Number of Bytes of Range Encoding is 0 - 3*/
        if(rangeInsertData_p->m_rangeAEncodedBytes != NLMDEV_1BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeAEncodedBytes != NLMDEV_2BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeAEncodedBytes != NLMDEV_3BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeAEncodedBytes != NLMDEV_4BYTE_RANGE_ENCODED_VALUE)
            return NLMERR_FAIL;

        if(rangeInsertData_p->m_rangeBEncodedBytes != NLMDEV_1BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeBEncodedBytes != NLMDEV_2BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeBEncodedBytes != NLMDEV_3BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeBEncodedBytes != NLMDEV_4BYTE_RANGE_ENCODED_VALUE)
            return NLMERR_FAIL;

        /* Location of various Field in Reg are as follows
            Range B Encoding Type: Bits[63:62]
            Range A Encoding Type: Bits[61:60]
            Range B Encoding Type: Bits[59:58]
            Range A Encoding Bytes: Bits[57:56] */
        value = (rangeInsertData_p->m_rangeBEncodingType << 6)
            | (rangeInsertData_p->m_rangeAEncodingType << 4)
            | (rangeInsertData_p->m_rangeBEncodedBytes << 2)
            | rangeInsertData_p->m_rangeAEncodedBytes;
        WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 63, 56, value);

         /* Location of RangeB Insert Field for various keys in Reg are as follows
           Key 0 : Bits[34:28]
           Key 1 : Bits[41:35]
           Key 2 : Bits[48:42]
           Key 3 : Bits[55:49] */
        value = 0;
        for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
        {
            if(rangeInsertData_p->m_rangeBInsertStartByte[keyNum] > maxRangeByte)
                return NLMERR_FAIL;

            value |= (rangeInsertData_p->m_rangeBInsertStartByte[keyNum] << (keyNum * 7));
        }
        WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 55, 28, value);

        /* Location of RangeA Insert Field for various keys in Reg are as follows
           Key 0 : Bits[6:0]
           Key 1 : Bits[13:7]
           Key 2 : Bits[20:14]
           Key 3 : Bits[27:21] */
        value = 0;
        for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
        {
             if(rangeInsertData_p->m_rangeAInsertStartByte[keyNum] > maxRangeByte)
                return NLMERR_FAIL;

            value |= (rangeInsertData_p->m_rangeAInsertStartByte[keyNum] << (keyNum * 7));
        }
        WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 27, 0, value);
    }
    else
    {
        NlmDevRangeInsertion1Reg *rangeInsertData_p =  (NlmDevRangeInsertion1Reg*)inputData_p;

         /* Valid values for Type of Range Encoding is 0 - 2*/
        if(rangeInsertData_p->m_rangeCEncodingType != NLMDEV_3BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeCEncodingType != NLMDEV_2BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeCEncodingType != NLMDEV_NO_RANGE_ENCODING)
            return NLMERR_FAIL;

        if(rangeInsertData_p->m_rangeDEncodingType != NLMDEV_3BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeDEncodingType != NLMDEV_2BIT_RANGE_ENCODING
            && rangeInsertData_p->m_rangeDEncodingType != NLMDEV_NO_RANGE_ENCODING)
            return NLMERR_FAIL;

         /* Valid values for Number of Bytes of Range Encoding is 0 - 3*/
        if(rangeInsertData_p->m_rangeCEncodedBytes != NLMDEV_1BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeCEncodedBytes != NLMDEV_2BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeCEncodedBytes != NLMDEV_3BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeCEncodedBytes != NLMDEV_4BYTE_RANGE_ENCODED_VALUE)
            return NLMERR_FAIL;

        if(rangeInsertData_p->m_rangeDEncodedBytes != NLMDEV_1BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeDEncodedBytes != NLMDEV_2BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeDEncodedBytes != NLMDEV_3BYTE_RANGE_ENCODED_VALUE
            && rangeInsertData_p->m_rangeDEncodedBytes != NLMDEV_4BYTE_RANGE_ENCODED_VALUE)
            return NLMERR_FAIL;

        /* Location of various Field in Reg are as follows
            Range D Encoding Type: Bits[63:62]
            Range C Encoding Type: Bits[61:60]
            Range D Encoding Type: Bits[59:58]
            Range C Encoding Bytes: Bits[57:56] */
        value = (rangeInsertData_p->m_rangeDEncodingType << 6)
            | (rangeInsertData_p->m_rangeCEncodingType << 4)
            | (rangeInsertData_p->m_rangeDEncodedBytes << 2)
            | rangeInsertData_p->m_rangeCEncodedBytes;

        WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 63, 56, value);

         /* Location of RangeD Insert Field for various keys in Reg are as follows
           Key 0 : Bits[34:28]
           Key 1 : Bits[41:35]
           Key 2 : Bits[48:42]
           Key 3 : Bits[55:49] */
        value = 0;
        for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
        {
            if(rangeInsertData_p->m_rangeDInsertStartByte[keyNum] > maxRangeByte)
                return NLMERR_FAIL;

            value |= (rangeInsertData_p->m_rangeDInsertStartByte[keyNum] << (keyNum * 7));
        }
        WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 55, 28, value);

        /* Location of RangeC Insert Field for various keys in Reg are as follows
           Key 0 : Bits[6:0]
           Key 1 : Bits[13:7]
           Key 2 : Bits[20:14]
           Key 3 : Bits[27:21] */
        value = 0;
        for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
        {
            if(rangeInsertData_p->m_rangeCInsertStartByte[keyNum] > maxRangeByte)
                return NLMERR_FAIL;

            value |= (rangeInsertData_p->m_rangeCInsertStartByte[keyNum] << (keyNum * 7));
        }
        WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 27, 0, value);
    }
    return NLMERR_OK;
}

/* NlmDevMgr_pvt_ExtractRangeInsertLtrData extracts the various fields  of Range Insert Reg
from the 80b Reg Data read from the device */
static void NlmDevMgr_pvt_ExtractRangeInsertLtrData(
    nlm_u8      *readData,
    void        *outputData_p,
    nlm_u32     regType
    )
{
    nlm_32 keyNum;
    nlm_u32 value = 0;

    if(regType == NLMDEV_RANGE_INSERTION_0_LTR)
    {
        NlmDevRangeInsertion0Reg *rangeInsertData_p =  (NlmDevRangeInsertion0Reg*)outputData_p;

        /* Location of various Field in Reg are as follows
            Range B Encoding Type: Bits[63:62]
            Range A Encoding Type: Bits[61:60]
            Range B Encoding Type: Bits[59:58]
            Range A Encoding Bytes: Bits[57:56] */
        value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 63, 56);
        rangeInsertData_p->m_rangeBEncodingType = (NlmDevRangeEncodingType)((value >> 6) & 0x3);
        rangeInsertData_p->m_rangeAEncodingType = (NlmDevRangeEncodingType)((value >> 4) & 0x3);
        rangeInsertData_p->m_rangeBEncodedBytes = (NlmDevRangeEncodedValueBytes)((value >> 2) & 0x3);
        rangeInsertData_p->m_rangeAEncodedBytes = (NlmDevRangeEncodedValueBytes)(value & 0x3);

        /* Location of RangeB Insert Field for various keys in Reg are as follows
           Key 0 : Bits[34:28]
           Key 1 : Bits[41:35]
           Key 2 : Bits[48:42]
           Key 3 : Bits[55:49] */
        value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 55, 28);
        for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
            rangeInsertData_p->m_rangeBInsertStartByte[keyNum] = (nlm_u8)((value >> (keyNum * 7)) & 0x7F);

       /* Location of RangeA Insert Field for various keys in Reg are as follows
           Key 0 : Bits[6:0]
           Key 1 : Bits[13:7]
           Key 2 : Bits[20:14]
           Key 3 : Bits[27:21] */
        value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 27, 0);
        for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
            rangeInsertData_p->m_rangeAInsertStartByte[keyNum] = (nlm_u8)((value >> (keyNum * 7)) & 0x7F);
    }
    else
    {
         NlmDevRangeInsertion1Reg *rangeInsertData_p =  (NlmDevRangeInsertion1Reg*)outputData_p;

        /* Location of various Field in Reg are as follows
            Range D Encoding Type: Bits[63:62]
            Range C Encoding Type: Bits[61:60]
            Range D Encoding Type: Bits[59:58]
            Range C Encoding Bytes: Bits[57:56] */
        value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 63, 56);
        rangeInsertData_p->m_rangeDEncodingType = (NlmDevRangeEncodingType)((value >> 6) & 0x3);
        rangeInsertData_p->m_rangeCEncodingType = (NlmDevRangeEncodingType)((value >> 4) & 0x3);
        rangeInsertData_p->m_rangeDEncodedBytes = (NlmDevRangeEncodedValueBytes)((value >> 2) & 0x3);
        rangeInsertData_p->m_rangeCEncodedBytes = (NlmDevRangeEncodedValueBytes)(value & 0x3);

        /* Location of RangeD Insert Field for various keys in Reg are as follows
           Key 0 : Bits[34:28]
           Key 1 : Bits[41:35]
           Key 2 : Bits[48:42]
           Key 3 : Bits[55:49] */
        value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 55, 28);
        for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
            rangeInsertData_p->m_rangeDInsertStartByte[keyNum] = (nlm_u8)((value >> (keyNum * 7)) & 0x7F);

       /* Location of RangeC Insert Field for various keys in Reg are as follows
           Key 0 : Bits[6:0]
           Key 1 : Bits[13:7]
           Key 2 : Bits[20:14]
           Key 3 : Bits[27:21] */
        value = ReadBitsInArrray(readData, NLMDEV_REG_LEN_IN_BYTES, 27, 0);
        for(keyNum = 0; keyNum < NLMDEV_NUM_KEYS; keyNum++)
            rangeInsertData_p->m_rangeCInsertStartByte[keyNum] = (nlm_u8)((value >> (keyNum * 7)) & 0x7F);
    }
}

/* NlmDevMgr_pvt_ConstructExtCap0LtrData contructs the 80b Reg Data based on various values
    of the various fields of Extend Cap Reg provided by application */
static NlmErrNum_t NlmDevMgr_pvt_ConstructExtCap0LtrData(
    nlm_u8              *o_data,
    NlmDevExtCap0Reg    *extCap0Data_p
    )
{
    nlm_32 psNum;

    for(psNum = 0; psNum < NLMDEV_NUM_PARALLEL_SEARCHES; psNum++)
    {
        /* Valid values for BMR Select is 0 - 4 and NLMDEV_NO_MASK_BMR_NUM */
        if( extCap0Data_p->m_bmrSelect[psNum] >= NLMDEV_NUM_BMRS_PER_AB &&
            extCap0Data_p->m_bmrSelect[psNum] != NLM_NO_MASK_BMR_NUM )
        {
            return NLMERR_FAIL;
        }
        /* Location of BMR Select Field for various parallel searches in Reg are as follows
           PS 0 : Bits[2:0]
           PS 1 : Bits[5:3]
           PS 2 : Bits[8:6]
           PS 3 : Bits[11:9]
         */
        WriteBitsInArray(o_data,
                        NLMDEV_REG_LEN_IN_BYTES,
                        NLM_EXTENDED_REG_BMR_SELECT_END_BIT(psNum),
                        NLM_EXTENDED_REG_BMR_SELECT_START_BIT(psNum),
                        extCap0Data_p->m_bmrSelect[psNum]
                        );
    }

    /* Valid values for Range Extract Start Bytes is 0 - 78 */
    if(extCap0Data_p->m_rangeAExtractStartByte >= (NLMDEV_MAX_KEY_LEN_IN_BYTES - 1)
        || extCap0Data_p->m_rangeBExtractStartByte >= (NLMDEV_MAX_KEY_LEN_IN_BYTES - 1)
        || extCap0Data_p->m_rangeCExtractStartByte >= (NLMDEV_MAX_KEY_LEN_IN_BYTES - 1)
        || extCap0Data_p->m_rangeDExtractStartByte >= (NLMDEV_MAX_KEY_LEN_IN_BYTES - 1))
        return NLMERR_FAIL;

    /* Location of Range Extract Start Bytes Field for various range types in Reg are as follows
           Range A  : Bits[30:24]
           Range B  : Bits[37:31]
           Range C  : Bits[44:38]
           Range D  : Bits[51:45] */

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_EXTENDED_REG_RANGE_A_EXTRACTION_END_BIT,
                     NLM_EXTENDED_REG_RANGE_A_EXTRACTION_START_BIT,
                     extCap0Data_p->m_rangeAExtractStartByte);

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_EXTENDED_REG_RANGE_B_EXTRACTION_END_BIT,
                     NLM_EXTENDED_REG_RANGE_B_EXTRACTION_START_BIT,
                     extCap0Data_p->m_rangeBExtractStartByte);

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_EXTENDED_REG_RANGE_C_EXTRACTION_END_BIT,
                     NLM_EXTENDED_REG_RANGE_C_EXTRACTION_START_BIT,
                     extCap0Data_p->m_rangeCExtractStartByte);

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_EXTENDED_REG_RANGE_D_EXTRACTION_END_BIT,
                     NLM_EXTENDED_REG_RANGE_D_EXTRACTION_START_BIT,
                     extCap0Data_p->m_rangeDExtractStartByte);

    /* Valid values for Range Extract Start Bytes is 0 - 3 */
    if(extCap0Data_p->m_numOfValidSrchRslts > 3)
         return NLMERR_FAIL;

    /* Number of valid search results occupy Bits [57:56] of the Reg  */
    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_EXTENDED_REG_NUM_RESULTS_END_BIT,
                     NLM_EXTENDED_REG_NUM_RESULTS_START_BIT,
                     extCap0Data_p->m_numOfValidSrchRslts);

    return NLMERR_OK;
}

/* NlmDevMgr_pvt_ExtractExtCap0LtrData extracts the various fields of Extend Cap Reg
from the 80b Reg Data read from the device */
static void NlmDevMgr_pvt_ExtractExtCap0LtrData(
    nlm_u8              *readData,
    NlmDevExtCap0Reg    *extCap0Data_p
    )
{
    nlm_32 psNum;

    for(psNum = 0; psNum < NLMDEV_NUM_PARALLEL_SEARCHES; psNum++)
    {
        /* Location of BMR Select Field for various parallel searches in Reg are as follows
           PS 0 : Bits[2:0]
           PS 1 : Bits[5:3]
           PS 2 : Bits[8:6]
           PS 3 : Bits[11:9]
         */

        /* Valid values for BMR Select is 0 - 4 and NLMDEV_NO_MASK_BMR_NUM */
        extCap0Data_p->m_bmrSelect[psNum] = (nlm_u8) (ReadBitsInArrray(readData,
                    NLMDEV_REG_LEN_IN_BYTES,
                    NLM_EXTENDED_REG_BMR_SELECT_END_BIT(psNum),
                    NLM_EXTENDED_REG_BMR_SELECT_START_BIT(psNum) ));
    }

    /* Location of Range Extract Start Bytes Field for various range types in Reg are as follows
           Range A  : Bits[30:24]
           Range B  : Bits[37:31]
           Range C  : Bits[44:38]
           Range D  : Bits[51:45] */
     extCap0Data_p->m_rangeAExtractStartByte = (nlm_u8)ReadBitsInArrray(readData,
                    NLMDEV_REG_LEN_IN_BYTES,
                    NLM_EXTENDED_REG_RANGE_A_EXTRACTION_END_BIT,
                    NLM_EXTENDED_REG_RANGE_A_EXTRACTION_START_BIT);

     extCap0Data_p->m_rangeBExtractStartByte = (nlm_u8)ReadBitsInArrray(readData,
                    NLMDEV_REG_LEN_IN_BYTES,
                    NLM_EXTENDED_REG_RANGE_B_EXTRACTION_END_BIT,
                    NLM_EXTENDED_REG_RANGE_B_EXTRACTION_START_BIT);

     extCap0Data_p->m_rangeCExtractStartByte = (nlm_u8)ReadBitsInArrray(readData,
                    NLMDEV_REG_LEN_IN_BYTES,
                    NLM_EXTENDED_REG_RANGE_C_EXTRACTION_END_BIT,
                    NLM_EXTENDED_REG_RANGE_C_EXTRACTION_START_BIT);

     extCap0Data_p->m_rangeDExtractStartByte = (nlm_u8)ReadBitsInArrray(readData,
                    NLMDEV_REG_LEN_IN_BYTES,
                    NLM_EXTENDED_REG_RANGE_D_EXTRACTION_END_BIT,
                    NLM_EXTENDED_REG_RANGE_D_EXTRACTION_START_BIT);

    /* Number of valid search results */
     extCap0Data_p->m_numOfValidSrchRslts = (nlm_u8) (ReadBitsInArrray(readData,
                    NLMDEV_REG_LEN_IN_BYTES,
                    NLM_EXTENDED_REG_NUM_RESULTS_END_BIT,
                    NLM_EXTENDED_REG_NUM_RESULTS_START_BIT) );
}

/* NlmDevMgr_pvt_ConstructOpCodeExtLtrData contructs the 80b Reg Data based on various values
    of the various fields of Opcode Extn Reg provided by application */
static NlmErrNum_t NlmDevMgr_pvt_ConstructOpCodeExtLtrData(
    nlm_u8              *o_data,
    NlmDevOpCodeExtReg  *opCodeExtData_p
    )
{
    nlm_32 psNum;

    for(psNum = 0; psNum < NLMDEV_NUM_PARALLEL_SEARCHES; psNum++)
    {
        /* Just Index or (Index + AD) */
        if( opCodeExtData_p->m_resultType[psNum] != NLMDEV_INDEX_ONLY &&
            opCodeExtData_p->m_resultType[psNum] != NLMDEV_INDEX_AND_AD )
        {
            return NLMERR_FAIL;
        }

        WriteBitsInArray(o_data,
                         NLMDEV_REG_LEN_IN_BYTES,
                         NLM_OP_CODE_EXT_REG_RETURN_AD_BIT_POS(psNum),
                         NLM_OP_CODE_EXT_REG_RETURN_AD_BIT_POS(psNum),
                         opCodeExtData_p->m_resultType[psNum]
                        );

        /* AD Length */
        if( opCodeExtData_p->m_ADLen[psNum] != NLMDEV_ADLEN_32B &&
            opCodeExtData_p->m_ADLen[psNum] != NLMDEV_ADLEN_64B &&
            opCodeExtData_p->m_ADLen[psNum] != NLMDEV_ADLEN_128B &&
            opCodeExtData_p->m_ADLen[psNum] != NLMDEV_ADLEN_256B )
        {
            return NLMERR_FAIL;
        }

        WriteBitsInArray(o_data,
                        NLMDEV_REG_LEN_IN_BYTES,
                        NLM_OP_CODE_EXT_REG_AD_SIZE_END_BIT(psNum),
                        NLM_OP_CODE_EXT_REG_AD_SIZE_START_BIT(psNum),
                        opCodeExtData_p->m_ADLen[psNum]
                        );
    }

    WriteBitsInArray(o_data,
                        NLMDEV_REG_LEN_IN_BYTES,
                        NLM_OP_CODE_EXT_REG_LCL_OP_CODE_END_BIT,
                        NLM_OP_CODE_EXT_REG_LCL_OP_CODE_START_BIT,
                        opCodeExtData_p->m_lclOpCode
                        );



    return NLMERR_OK;
}

/* NlmDevMgr_pvt_ExtractOpCodeExtLtrData extracts the various fields of Opcode Ext Reg
from the 80b Reg Data read from the device */
static void NlmDevMgr_pvt_ExtractOpCodeExtLtrData(
    nlm_u8              *readData,
    NlmDevOpCodeExtReg  *opCodeExtData_p
    )
{
    nlm_32 psNum;

    for(psNum = 0; psNum < NLMDEV_NUM_PARALLEL_SEARCHES; psNum++)
    {
        opCodeExtData_p->m_resultType[psNum] = (NlmDevResultType)ReadBitsInArrray(readData,
                        NLMDEV_REG_LEN_IN_BYTES,
                        NLM_OP_CODE_EXT_REG_RETURN_AD_BIT_POS(psNum),
                        NLM_OP_CODE_EXT_REG_RETURN_AD_BIT_POS(psNum) );

        opCodeExtData_p->m_ADLen[psNum] = (NlmDevADLength)ReadBitsInArrray(readData,
                        NLMDEV_REG_LEN_IN_BYTES,
                        NLM_OP_CODE_EXT_REG_AD_SIZE_END_BIT(psNum),
                        NLM_OP_CODE_EXT_REG_AD_SIZE_START_BIT(psNum) );

    }

    opCodeExtData_p->m_lclOpCode =  ReadBitsInArrray(readData,
                        NLMDEV_REG_LEN_IN_BYTES,
                        NLM_OP_CODE_EXT_REG_LCL_OP_CODE_END_BIT,
                        NLM_OP_CODE_EXT_REG_LCL_OP_CODE_START_BIT
                        );
}

static void NlmDevMgr_pvt_ConstructDevConfigRegData(
    nlm_u8              *o_data,
    NlmDevConfigReg     *reg_data
    )
{
    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_PORT1_CTXID_SHIFT_END_BIT,
                     NLM_DEVICE_CONFIG_REG_PORT1_CTXID_SHIFT_START_BIT,
                     reg_data->m_port1CtxIDShift
                    );

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_AC_TO_BANK_MAPPING_END_BIT,
                     NLM_DEVICE_CONFIG_REG_AC_TO_BANK_MAPPING_START_BIT,
                     reg_data->m_ACtoBankMapping
                    );


    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_RANGE_ENABLE_BIT,
                     NLM_DEVICE_CONFIG_REG_RANGE_ENABLE_BIT,
                     reg_data->m_rangeEnable
                    );

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_PORT1_ENABLE_BIT,
                     NLM_DEVICE_CONFIG_REG_PORT1_ENABLE_BIT,
                     reg_data->m_port1Enable
                    );

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_PORT0_ENABLE_BIT,
                     NLM_DEVICE_CONFIG_REG_PORT0_ENABLE_BIT,
                     reg_data->m_port0Enable
                    );

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_DUAL_PORT_MODE_BIT,
                     NLM_DEVICE_CONFIG_REG_DUAL_PORT_MODE_BIT,
                     reg_data->m_dualPortMode
                    );

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_DUAL_BANK_MODE_BIT,
                     NLM_DEVICE_CONFIG_REG_DUAL_BANK_MODE_BIT,
                     reg_data->m_dualBankMode
                    );

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_CB_CONFIG_END_BIT,
                     NLM_DEVICE_CONFIG_REG_CB_CONFIG_START_BIT,
                     reg_data->m_CBConfig
                    );

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_DB_PEEI_BIT,
                     NLM_DEVICE_CONFIG_REG_DB_PEEI_BIT,
                     reg_data->m_dbParityErrEntryInvalidate
                    );

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_SOFT_ES_ENABLE_BIT,
                     NLM_DEVICE_CONFIG_REG_SOFT_ES_ENABLE_BIT,
                     reg_data->m_softErrorScanEnable
                    );

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_LAST_DEVICE_BIT,
                     NLM_DEVICE_CONFIG_REG_LAST_DEVICE_BIT,
                     reg_data->m_lastDevice
                    );

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_FIRST_DEVICE_BIT,
                     NLM_DEVICE_CONFIG_REG_FIRST_DEVICE_BIT,
                     reg_data->m_firstDevice
                    );
}

static void NlmDevMgr_pvt_ExtractDevConfigRegData(
    nlm_u8              *o_data,
    NlmDevConfigReg     *reg_data
    )
{
    reg_data->m_port1CtxIDShift = (nlm_u8) (ReadBitsInArrray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_PORT1_CTXID_SHIFT_END_BIT,
                     NLM_DEVICE_CONFIG_REG_PORT1_CTXID_SHIFT_START_BIT) );

    reg_data->m_ACtoBankMapping = (nlm_u16)(ReadBitsInArrray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_AC_TO_BANK_MAPPING_END_BIT,
                     NLM_DEVICE_CONFIG_REG_AC_TO_BANK_MAPPING_START_BIT) );


    reg_data->m_rangeEnable = (NlmDevDisableEnable)ReadBitsInArrray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_RANGE_ENABLE_BIT,
                     NLM_DEVICE_CONFIG_REG_RANGE_ENABLE_BIT );

    reg_data->m_port1Enable = (NlmDevDisableEnable)ReadBitsInArrray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_PORT1_ENABLE_BIT,
                     NLM_DEVICE_CONFIG_REG_PORT1_ENABLE_BIT );

    reg_data->m_port0Enable = (NlmDevDisableEnable)ReadBitsInArrray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_PORT0_ENABLE_BIT,
                     NLM_DEVICE_CONFIG_REG_PORT0_ENABLE_BIT );

    reg_data->m_dualPortMode = (NlmPortMode) (ReadBitsInArrray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_DUAL_PORT_MODE_BIT,
                     NLM_DEVICE_CONFIG_REG_DUAL_PORT_MODE_BIT) );

    reg_data->m_dualBankMode = (NlmSMTMode) (ReadBitsInArrray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_DUAL_BANK_MODE_BIT,
                     NLM_DEVICE_CONFIG_REG_DUAL_BANK_MODE_BIT) );

    reg_data->m_CBConfig = (nlm_u8) (ReadBitsInArrray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_CB_CONFIG_END_BIT,
                     NLM_DEVICE_CONFIG_REG_CB_CONFIG_START_BIT) );

    reg_data->m_dbParityErrEntryInvalidate = (NlmDevDisableEnable)ReadBitsInArrray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_DB_PEEI_BIT,
                     NLM_DEVICE_CONFIG_REG_DB_PEEI_BIT );

    reg_data->m_softErrorScanEnable = (NlmDevDisableEnable)ReadBitsInArrray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_SOFT_ES_ENABLE_BIT,
                     NLM_DEVICE_CONFIG_REG_SOFT_ES_ENABLE_BIT );

    reg_data->m_lastDevice = (nlm_u8) (ReadBitsInArrray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_LAST_DEVICE_BIT,
                     NLM_DEVICE_CONFIG_REG_LAST_DEVICE_BIT) );

    reg_data->m_firstDevice = (nlm_u8) (ReadBitsInArrray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_DEVICE_CONFIG_REG_FIRST_DEVICE_BIT,
                     NLM_DEVICE_CONFIG_REG_FIRST_DEVICE_BIT) );
}

static void NlmDevMgr_pvt_ConstructUDAConfigRegData(
    nlm_u8              *o_data,
    NlmDevUDAConfigReg  *reg_data
    )
{
    nlm_u32 value = 0, i;

    for(i = 0; i < NLMDEV_NUM_SRAM_SUPER_BLOCKS / 4; i++)
    {
        if( reg_data->m_uSBEnable[i] == NLMDEV_ENABLE )
        {
            value |= (0x1 << i);
        }
    }

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_UDA_CONFIG_REG_uSBENABLE_END_BIT,
                     NLM_UDA_CONFIG_REG_uSBENABLE_START_BIT,
                     value
                    );
}

static void NlmDevMgr_pvt_ExtractUDAConfigRegData(
    nlm_u8              *o_data,
    NlmDevUDAConfigReg  *reg_data
    )
{
    nlm_u32 value = 0, i;

    value = ReadBitsInArrray(o_data,
                NLMDEV_REG_LEN_IN_BYTES,
                NLM_UDA_CONFIG_REG_uSBENABLE_END_BIT,
                NLM_UDA_CONFIG_REG_uSBENABLE_START_BIT);

    for(i = 0; i < NLMDEV_NUM_SRAM_SUPER_BLOCKS / 4; i++, value >>= 1)
        reg_data->m_uSBEnable[i] = (NlmDevDisableEnable)((value & 0x1));
}

static void NlmDevMgr_pvt_ConstructRangeBoundRegData(
    nlm_u8              *o_data,
    nlm_u8              *i_data
    )
{
    nlm_u32 value = 0;

    /* copy Bits[0-31] */
    value = ReadBitsInArrray(i_data, NLMDEV_REG_LEN_IN_BYTES, 31, 0);
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 31, 0, value);

    /* copy Bits[32-33] */
    value = ReadBitsInArrray(i_data, NLMDEV_REG_LEN_IN_BYTES, 33, 32);
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 33, 32, value);

    return;
}

static void NlmDevMgr_pvt_ConstructRangeCodeRegData(
    nlm_u8              *o_data,
    nlm_u8              *i_data
    )
{
    nlm_u32 value = 0;

    /* copy Bits[0-31] */
    value = ReadBitsInArrray(i_data, NLMDEV_REG_LEN_IN_BYTES, 31, 0);
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 31, 0, value);

    /* copy Bits[32-47] */
    value = ReadBitsInArrray(i_data, NLMDEV_REG_LEN_IN_BYTES, 47, 32);
    WriteBitsInArray(o_data, NLMDEV_REG_LEN_IN_BYTES, 47, 32, value);

    return;
}

static void NlmDevMgr_pvt_ConstructBlockConfigRegData(
    nlm_u8                  *o_data,
    NlmDevBlockConfigReg    *reg_data
    )
{
    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_BLOCK_CONFIG_REG_SHIFT_DIR_BIT_POS,
                     NLM_BLOCK_CONFIG_REG_SHIFT_DIR_BIT_POS,
                     reg_data->m_shiftDir
                    );

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_BLOCK_CONFIG_REG_BASE_ADDR_END_BIT,
                     NLM_BLOCK_CONFIG_REG_BASE_ADDR_START_BIT,
                     reg_data->m_baseAddr
                    );

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_BLOCK_CONFIG_REG_SHIFT_VAL_END_BIT,
                     NLM_BLOCK_CONFIG_REG_SHIFT_VAL_START_BIT,
                     reg_data->m_shiftCount
                    );

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_BLOCK_CONFIG_REG_WIDTH_END_BIT,
                     NLM_BLOCK_CONFIG_REG_WIDTH_START_BIT,
                     reg_data->m_blockWidth
                    );

    WriteBitsInArray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_BLOCK_CONFIG_REG_ENABLE_BIT_POS,
                     NLM_BLOCK_CONFIG_REG_ENABLE_BIT_POS,
                     reg_data->m_blockEnable
                    );

}

static void NlmDevMgr_pvt_ExtractBlockConfigRegData(
    nlm_u8                  *o_data,
    NlmDevBlockConfigReg    *reg_data
    )
{
    reg_data->m_shiftDir = (NlmDevShiftDir)ReadBitsInArrray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_BLOCK_CONFIG_REG_SHIFT_DIR_BIT_POS,
                     NLM_BLOCK_CONFIG_REG_SHIFT_DIR_BIT_POS );

    reg_data->m_baseAddr = ReadBitsInArrray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_BLOCK_CONFIG_REG_BASE_ADDR_END_BIT,
                     NLM_BLOCK_CONFIG_REG_BASE_ADDR_START_BIT );

    reg_data->m_shiftCount = (NlmDevShiftCount)ReadBitsInArrray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_BLOCK_CONFIG_REG_SHIFT_VAL_END_BIT,
                     NLM_BLOCK_CONFIG_REG_SHIFT_VAL_START_BIT );

    reg_data->m_blockWidth = (NlmDevBlockWidth)ReadBitsInArrray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_BLOCK_CONFIG_REG_WIDTH_END_BIT,
                     NLM_BLOCK_CONFIG_REG_WIDTH_START_BIT );

    reg_data->m_blockEnable = (NlmDevDisableEnable)ReadBitsInArrray(o_data,
                     NLMDEV_REG_LEN_IN_BYTES,
                     NLM_BLOCK_CONFIG_REG_ENABLE_BIT_POS,
                     NLM_BLOCK_CONFIG_REG_ENABLE_BIT_POS );

}

#endif

static void NlmDevMgr__pvt_ExtractCmpResults(
    nlm_u8          *in_data,
    NlmDevCmpResult *cmp_results,
    NlmBool          isLPM /* True if it is CMPLPM instruction */
    )
{
    NlmDevResultValid   result_valid;
    NlmDevMissHit       miss_hit;
    NlmDevRespType      resp_type;
    nlm_u32             hit_index = 0xFFFFFFFF;
    nlm_u32             copy_len, num_bytes = 0;
    nlm_u8              dev_id = 0, ps_num;
    nlm_u8             *ptr, *out_ptr;

       NlmCm__memset( cmp_results, 0, sizeof(NlmDevCmpResult) );

    ptr = in_data;

    for(ps_num = 0, copy_len = 0; ps_num < NLMDEV_NUM_PARALLEL_SEARCHES; ps_num++)
    {
        result_valid = (NlmDevResultValid)ReadBitsInArrray(ptr, 4,
                                        NLMDEV_CMP_RESULT_RV_BIT_POS,
                                        NLMDEV_CMP_RESULT_RV_BIT_POS
                                        );

        miss_hit    = (NlmDevMissHit)ReadBitsInArrray(ptr, 4,
                                       NLMDEV_CMP_RESULT_SMF_BIT_POS,
                                       NLMDEV_CMP_RESULT_SMF_BIT_POS
                                       );

        resp_type   = (NlmDevRespType)ReadBitsInArrray(ptr, 4,
                                       NLMDEV_CMP_RESULT_RESP_FORMAT_END_BIT,
                                       NLMDEV_CMP_RESULT_RESP_FORMAT_START_BIT
                                       );

        if(isLPM)
        {
            /* LPM search can be only on PS#0 */
            if(ps_num == 0)
            {
                dev_id = 0; /* devid is ignored for LPM searches */

#ifdef NLM_12K_11K
                hit_index = ReadBitsInArrray(ptr, 4,
                                        22,
                                        0
                                        );
#else
                hit_index = ReadBitsInArrray(ptr, 4,
                                        NLMDEV_CMP_LPM_RESULT_INDEX_END_BIT,
                                        NLMDEV_CMP_LPM_RESULT_INDEX_START_BIT
                                        );
#endif
                isLPM = 0; /* for non-LPM search results after psNum > 0,
                              need to reset the isLPM flag */
            }
        }
        else
        {
#ifdef NLM_12K_11K
            dev_id     = (nlm_u8) ReadBitsInArrray(ptr, 4,
                                        22,
                                        21
                                        );

            hit_index = ReadBitsInArrray(ptr, 4,
                                        20,
                                        0
                                        );
#else
            dev_id     = (nlm_u8) ReadBitsInArrray(ptr, 4,
                                        NLMDEV_CMP_RESULT_DEVID_END_BIT,
                                        NLMDEV_CMP_RESULT_DEVID_START_BIT
                                        );

            hit_index = ReadBitsInArrray(ptr, 4,
                                        NLMDEV_CMP_RESULT_INDEX_END_BIT,
                                        NLMDEV_CMP_RESULT_INDEX_START_BIT
                                        );
#endif
        }

        /* Advance pointer by 4 to point to the next Result or AD data */
        ptr += 4;
             num_bytes += 4;

        copy_len = 0; /* NLMDEV_INDEX_AND_NO_AD */

        /* AD length is based on the resp_type */
        if(resp_type != NLMDEV_INDEX_AND_NO_AD)
        {
            out_ptr = cmp_results->m_AssocData[ps_num];

            if(resp_type == NLMDEV_INDEX_AND_32B_AD)
                copy_len = 4;
            else if(resp_type == NLMDEV_INDEX_AND_64B_AD)
                copy_len = 8;
            else if(resp_type == NLMDEV_INDEX_AND_128B_AD)
                copy_len = 16;
            else
                copy_len = 32;

            /*Truncate the AD if it is overflowing result length limit*/
            if( (num_bytes + copy_len) > NLMDEV_MAX_RESP_LEN_IN_BYTES)
                copy_len = (NLMDEV_MAX_RESP_LEN_IN_BYTES - num_bytes);

            NlmCm__memcpy( out_ptr, ptr, copy_len );

            num_bytes += copy_len;
        }

        cmp_results->m_resultValid[ps_num] = result_valid;
        cmp_results->m_hitOrMiss[ps_num]   = miss_hit;
        cmp_results->m_respType[ps_num]    = resp_type;
        cmp_results->m_hitDevId[ps_num]    = dev_id;
        cmp_results->m_hitIndex[ps_num]    = hit_index;

        /* Ignore other results if output result buffer is full*/
        if(  num_bytes >= NLMDEV_MAX_RESP_LEN_IN_BYTES)
            break;

        /* Advance ptr for next set of results. */
        ptr += copy_len;
    } /* for */
}


#ifdef NLM_12K_11K

/* Get LTR data from the shadow memory */
NlmErrNum_t kbp_dm_pvt_get_ltr_data_from_sm(
    NlmDev              *dev,
    nlm_u8                  ltrNum,     /* LTR profile set number */
    NlmDevLtrRegType        regType,    /* see the structure description above */
    void                    **data_p,
    NlmReasonCode*          o_reason
    )
{


    NlmDevShadowDevice *shadowDev_p;
    void *data = NULL;

    shadowDev_p = NLM_GET_SHADOW_MEM_FROM_DEV_PTR(dev);

/* writing register contents into data field of request structure */
    switch(regType)
    {
        case NLMDEV_BLOCK_SELECT_0_LTR:
        case NLMDEV_BLOCK_SELECT_1_LTR:
        case NLMDEV_BLOCK_SELECT_2_LTR:
        case NLMDEV_BLOCK_SELECT_3_LTR:
        /* Get the Data from shadow memory */
        data = &shadowDev_p->m_ltr[ltrNum].m_blockSelect[regType - NLMDEV_BLOCK_SELECT_0_LTR];
        break;        

        case NLMDEV_SUPER_BLK_KEY_MAP_LTR:
            /* Get the Data from shadow memory */
            data = &shadowDev_p->m_ltr[ltrNum].m_superBlkKeyMap;
            break;

        case NLMDEV_PARALLEL_SEARCH_0_LTR:
        case NLMDEV_PARALLEL_SEARCH_1_LTR:
        case NLMDEV_PARALLEL_SEARCH_2_LTR:
        case NLMDEV_PARALLEL_SEARCH_3_LTR:
        case NLMDEV_PARALLEL_SEARCH_4_LTR:
        case NLMDEV_PARALLEL_SEARCH_5_LTR:
        case NLMDEV_PARALLEL_SEARCH_6_LTR:
        case NLMDEV_PARALLEL_SEARCH_7_LTR:        
            /* Get the Data from shadow memory */
            data = &shadowDev_p->m_ltr[ltrNum].m_parallelSrch[regType - NLMDEV_PARALLEL_SEARCH_0_LTR];
            break;

        case NLMDEV_KEY_0_KCR_0_LTR:
        case NLMDEV_KEY_0_KCR_1_LTR:
        case NLMDEV_KEY_1_KCR_0_LTR:
        case NLMDEV_KEY_1_KCR_1_LTR:
        case NLMDEV_KEY_2_KCR_0_LTR:
        case NLMDEV_KEY_2_KCR_1_LTR:
        case NLMDEV_KEY_3_KCR_0_LTR:
        case NLMDEV_KEY_3_KCR_1_LTR:
            /* Get the Data from shadow memory */
            data = &shadowDev_p->m_ltr[ltrNum].m_keyConstruct[regType - NLMDEV_KEY_0_KCR_0_LTR];
            break;

        case NLMDEV_RANGE_INSERTION_0_LTR:
        case NLMDEV_RANGE_INSERTION_1_LTR:
            /* Get the Data from shadow memory */
            if(regType == NLMDEV_RANGE_INSERTION_0_LTR)
                data = &shadowDev_p->m_ltr[ltrNum].m_rangeInsert0;
            else
                data = &shadowDev_p->m_ltr[ltrNum].m_rangeInsert1;

           break;

        case NLMDEV_EXT_CAPABILITY_REG_0_LTR:
            /* Get the Data from shadow memory */
            data = &shadowDev_p->m_ltr[ltrNum].m_extCap0;
            break;
             
        case NLMDEV_OPCODE_EXT_LTR:
            /* Get the Data from shadow memory */
            data = &shadowDev_p->m_ltr[ltrNum].m_opCodeExt; 
             break;        

        default:
            if(o_reason)
                *o_reason = NLMRSC_INVALID_REG_ADDRESS;
            return NLMERR_FAIL;
    }

    *data_p = data;

    return NLMERR_OK;
}
#endif


#ifndef NLM_12K_11K
static void NlmDevMgr__pvt_ExtractCmp3Results(
    NlmDevMgr       *devMgr_p,
    nlm_u8          ltr_num,
    nlm_u8          *in_data,
    NlmDevCmpResult *cmp_results
    )
{
    NlmDevResultValid   result_valid;
    NlmDevMissHit       miss_hit;
    NlmDevRespType  resp_type;
    NlmBool         isLPM = NlmFalse;
    nlm_u32         hit_index = 0xFFFFFFFF, *ptr1;
    nlm_u32         copy_len, num_bytes = 0;
    nlm_u8          dev_id = 0, result_num;
    nlm_u8          *ptr, *out_ptr, index, which_bit, tmp;

    NlmCm__memset( cmp_results, 0, sizeof(NlmDevCmpResult) );

    ptr = in_data;

    for(result_num = 0, copy_len = 0; result_num < NLM_MAX_NUM_RESULTS; result_num++)
    {
        result_valid = (NlmDevResultValid)ReadBitsInArrray(ptr, 4,
                                        NLMDEV_CMP_RESULT_RV_BIT_POS,
                                        NLMDEV_CMP_RESULT_RV_BIT_POS
                                        );

        miss_hit    = (NlmDevMissHit)ReadBitsInArrray(ptr, 4,
                                       NLMDEV_CMP_RESULT_SMF_BIT_POS,
                                       NLMDEV_CMP_RESULT_SMF_BIT_POS
                                       );

        resp_type   = (NlmDevRespType)ReadBitsInArrray(ptr, 4,
                                       NLMDEV_CMP_RESULT_RESP_FORMAT_END_BIT,
                                       NLMDEV_CMP_RESULT_RESP_FORMAT_START_BIT
                                       );

        /* Check if LPM is configured on Result-0 and Result-2 searches */
        if( (result_num == 0) || (result_num == 2) )
        {
            if(result_num == 0)
            {
                tmp = ltr_num;
            }
            else
            {
                /* LTR number wraps around to 0 for second CMP1 search */
                if( ltr_num == (NLMDEV_MAX_NUM_LTRS - 1) )
                    tmp = 0;
                else
                    tmp = ltr_num + 1;
            }

            /* First find the array index and then the bit within it */
            index = tmp / 32;
            which_bit = tmp % 32;

            ptr1    = &devMgr_p->m_lpmCmp3[index];

            /* Bit is set if the search type is LPM */
            isLPM = ( *ptr & (0x1 << which_bit) ) ? NlmTrue : NlmFalse;
        }

        if(isLPM)
        {
            dev_id = 0; /* devid is ignored for LPM searches */

            hit_index = ReadBitsInArrray(ptr, 4,
                                        NLMDEV_CMP_LPM_RESULT_INDEX_END_BIT,
                                        NLMDEV_CMP_LPM_RESULT_INDEX_START_BIT
                                        );
            isLPM = NlmFalse;
        }
        else
        {
            dev_id = (nlm_u8) ReadBitsInArrray(ptr, 4,
                                        NLMDEV_CMP_RESULT_DEVID_END_BIT,
                                        NLMDEV_CMP_RESULT_DEVID_START_BIT
                                        );

            hit_index = ReadBitsInArrray(ptr, 4,
                                        NLMDEV_CMP_RESULT_INDEX_END_BIT,
                                        NLMDEV_CMP_RESULT_INDEX_START_BIT
                                        );
        }

        /* Advance pointer by 4 to point to the next Result or AD data */
        ptr += 4;
             num_bytes += 4;

        copy_len = 0; /* NLMDEV_INDEX_AND_NO_AD */

        /* AD length is based on the resp_type */
        if(resp_type != NLMDEV_INDEX_AND_NO_AD)
        {
            out_ptr = cmp_results->m_AssocData[result_num];

            if(resp_type == NLMDEV_INDEX_AND_32B_AD)
                copy_len = 4;
            else if(resp_type == NLMDEV_INDEX_AND_64B_AD)
                copy_len = 8;
            else if(resp_type == NLMDEV_INDEX_AND_128B_AD)
                copy_len = 16;
            else
                copy_len = 32;

                    /*Truncate the AD if it is overflowing result length limit*/
            if( (num_bytes + copy_len) > NLMDEV_MAX_RESP_LEN_IN_BYTES)
                copy_len = (NLMDEV_MAX_RESP_LEN_IN_BYTES - num_bytes);

            NlmCm__memcpy( out_ptr, ptr, copy_len );

                    num_bytes += copy_len;
        }

        cmp_results->m_resultValid[result_num] = result_valid;
        cmp_results->m_hitOrMiss[result_num]   = miss_hit;
        cmp_results->m_respType[result_num]    = resp_type;
        cmp_results->m_hitDevId[result_num]    = dev_id;
        cmp_results->m_hitIndex[result_num]    = hit_index;

        /* Ignore other results if output result buffer is full*/
        if(  num_bytes >= NLMDEV_MAX_RESP_LEN_IN_BYTES)
            break;

        /* Advance ptr for next set of results. */
        ptr += copy_len;

    } /* for */
}
#endif

/* destroys shadow device memory */
NlmErrNum_t kbp_dm_shadow_destroy(
    NlmDev      *dev_p,
    NlmReasonCode   *o_reason
    )
{
    NlmDevShadowDevice *shadowDev_p;
    NlmCmAllocator *alloc_p;

    if(!NLM_IS_VALID_DEV_PTR(dev_p))
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

        if(shadowDev_p->m_global)
            NlmCmAllocator__free(alloc_p, shadowDev_p->m_global);

        if(shadowDev_p->m_rangeReg)
            NlmCmAllocator__free(alloc_p, shadowDev_p->m_rangeReg);

        if(shadowDev_p->m_st)
            NlmCmAllocator__free(alloc_p, shadowDev_p->m_st);

        NlmCmAllocator__free(alloc_p, shadowDev_p);
    }

    dev_p->m_shadowDevice_p = NULL;

    return NLMERR_OK;
}


/*creates shadow device - allocates all memory for shadow device structure */
NlmErrNum_t kbp_dm_shadow_create(
    NlmDev      *dev_p,
    NlmReasonCode   *o_reason
    )
{
    NlmDevShadowDevice *shadowDev_p;
    NlmCmAllocator *alloc_p;
    nlm_u32 abNum;
    nlm_u32 entryNum;

#ifndef NLM_12K_11K 
    nlm_u32 totalNumOfAb = dev_p->m_devMgr_p->m_numOfAbs;
    nlm_u32 totalNumofLtr = NLMDEV_MAX_NUM_LTRS;

#else
    nlm_u32 totalNumOfAb = NLM11KDEV_NUM_ARRAY_BLOCKS;
    nlm_u32 totalNumofLtr = NLM11KDEV_NUM_LTR_SET;
#endif

    if(!NLM_IS_VALID_DEV_PTR(dev_p))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }

    alloc_p = dev_p->m_devMgr_p->m_alloc_p;

    if(dev_p->m_shadowDevice_p == NULL)
    {
        /* Creating the memory for shadow device */
        dev_p->m_shadowDevice_p = (NlmDevShadowDevice*)NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmDevShadowDevice));
        if(dev_p->m_shadowDevice_p == NULL)
        {
            if(o_reason)
                *o_reason = NLMRSC_LOW_MEMORY;

            return NLMERR_FAIL;
        }

        shadowDev_p = dev_p->m_shadowDevice_p;

        /* Creating the memory for shadow Array Blocks */
        shadowDev_p->m_arrayBlock = (NlmDevShadowAB*)NlmCmAllocator__calloc(alloc_p,
                                                           totalNumOfAb,
                                                           sizeof(NlmDevShadowAB));

        if(shadowDev_p->m_arrayBlock == NULL)
        {
            kbp_dm_shadow_destroy(dev_p, NULL);

            if(o_reason)
                *o_reason = NLMRSC_LOW_MEMORY;

            return NLMERR_FAIL;
        }

        for(abNum = 0; abNum < totalNumOfAb; abNum++)
        {
            /* All the data and mask bytes of all the entries are initialized to have initial value as 0xFF */
            for(entryNum = 0; entryNum < NLMDEV_AB_DEPTH; entryNum++)
            {
                NlmCm__memset( &shadowDev_p->m_arrayBlock[abNum].m_abEntry[entryNum],
                                0xFF,
                                2 * NLMDEV_AB_WIDTH_IN_BYTES
                             );
            }
        }

        /* Creating the memory for shadow LTR */
        shadowDev_p->m_ltr = (NlmDevShadowLtr*)NlmCmAllocator__calloc(alloc_p,
                                                    totalNumofLtr,
                                                    sizeof(NlmDevShadowLtr)
                                                   );

        if (shadowDev_p->m_ltr == NULL)
        {
            kbp_dm_shadow_destroy(dev_p, NULL);

            if(o_reason)
                *o_reason = NLMRSC_LOW_MEMORY;

            return NLMERR_FAIL;
        }

        /* Creating the memory for shadow Global registers */
        shadowDev_p->m_global = (NlmDevShadowGlobal*)NlmCmAllocator__calloc(alloc_p,
                                                    1,
                                                    sizeof(NlmDevShadowGlobal)
                                                   );

        if (shadowDev_p->m_global == NULL)
        {
            kbp_dm_shadow_destroy(dev_p, NULL);

            if(o_reason)
                *o_reason = NLMRSC_LOW_MEMORY;

            return NLMERR_FAIL;
        }

        /* DCR: default values */
        shadowDev_p->m_global->m_devConfig.m_dualBankMode       = NLMDEV_NO_SMT_MODE;
        shadowDev_p->m_global->m_devConfig.m_dualPortMode       = NLMDEV_SINGLE_PORT;
        shadowDev_p->m_global->m_devConfig.m_port0Enable        = NLMDEV_ENABLE; /* in single port mode, port#0 */
        shadowDev_p->m_global->m_devConfig.m_port1CtxIDShift    = 0x4; /* port#1 cbId shift by 2K */
        shadowDev_p->m_global->m_devConfig.m_dbParityErrEntryInvalidate = NLMDEV_ENABLE; /* error entry is invalidated */

#ifdef NLM_12K_11K
    /* Create memory for ST. */
        shadowDev_p->m_st = kbp_dm_pvt_11k_map_create_shadow_st(alloc_p);
         
        if (shadowDev_p->m_st == NULL)
        {
            kbp_dm_shadow_destroy(dev_p, NULL);
            if(o_reason)
                *o_reason = NLMRSC_LOW_MEMORY;

            return NLMERR_FAIL;
        }
#else
        shadowDev_p->m_st = NULL;
#endif

        /*Create memory for range registers */
        shadowDev_p->m_rangeReg = (NlmDevRangeReg*)NlmCmAllocator__calloc(alloc_p,
                                                            NLMDEV_NUM_RANGE_REG,
                                                            sizeof(NlmDevRangeReg));

        if(shadowDev_p->m_rangeReg == NULL)
        {
            kbp_dm_shadow_destroy(dev_p, NULL);
            if(o_reason)
                *o_reason = NLMRSC_LOW_MEMORY;

            return NLMERR_FAIL;
        }

    } /* if () */

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;

    return NLMERR_OK;
}


/*
Function: kbp_dm_init
Description: creates Device manager using memory allocator and returns the pointer
of the Device manager. memory allocator must be passed as parameter. Function returns
NULL if fails and user should see the reason code in case of failure.
*/
 NlmDevMgr* kbp_dm_init(
    NlmCmAllocator      *alloc_p,
    void                    *xpt_p,
    NlmDevType          devType,
    NlmPortMode         portMode,
    NlmSMTMode          smtMode,
    NlmDevOperationMode  operMode,
    NlmReasonCode       *o_reason
    )
{
    NlmDevMgr *self = NULL;
    NlmXpt  *xpt = NULL;
    NlmReasonCode dummyReason;

    (void)operMode;

    if(o_reason == NULL)
        o_reason = &dummyReason;

    if(alloc_p == NULL)
    {
        *o_reason = NLMRSC_INVALID_MEMALLOC_PTR;
        return NULL;
    }
    if(xpt_p == NULL)
    {
        *o_reason = NLMRSC_INVALID_XPT_PTR;
        return NULL;
    }

#ifndef NLM_12K_11K
    if((devType != NLM_DEVTYPE_3) && (devType != NLM_DEVTYPE_3_N) && (devType != NLM_DEVTYPE_3_40M)
        && (devType != NLM_DEVTYPE_3_N_40M))
#else
    if((devType != NLM_DEVTYPE_2) && (devType != NLM_DEVTYPE_2_S))
#endif
    {
        *o_reason = NLMRSC_INVALID_DEVICE_TYPE;
        return NULL;
    }

#ifndef NLM_12K_11K 
    if((portMode != NLMDEV_SINGLE_PORT) && (portMode != NLMDEV_DUAL_PORT))
    {
        *o_reason = NLMRSC_INVALID_INPUT;
        return NULL;
    }

    if((smtMode != NLMDEV_NO_SMT_MODE) && (smtMode != NLMDEV_DUAL_SMT_MODE))
    {
        *o_reason = NLMRSC_INVALID_INPUT;
        return NULL;
    }
    
#endif

    /* Allocate memory for Device Manager */
    self = (NlmDevMgr*)NlmCmAllocator__malloc(alloc_p, sizeof(NlmDevMgr));
    NlmCmDemand((self != NULL), "Out of memory.\n");
    if(self == NULL)
    {
        *o_reason = NLMRSC_LOW_MEMORY;
        return NULL;
    }

    /* Initialize members of Device Manager */
    self = NlmDevMgr__ctor(self, xpt_p, devType, portMode, smtMode, alloc_p);

    *o_reason = NLMRSC_REASON_OK;

    /* Fill port mode and smt mode into xpt */
    xpt = (NlmXpt *)xpt_p;

    xpt->m_port_mode = self->m_portMode;
    xpt->m_smt_mode = self->m_smtMode;

#if defined NLM_MT_OLD || defined NLM_MT

    {
        nlm_32 ret;
    ret = NlmCmMt__SpinInit(&self->m_spinLock, 
                "NlmDevMgr_Kbp_SpinLock",
                NlmCmMtFlag);
        if(ret != 0)
        {
            *o_reason = NLMRSC_MT_SPINLOCK_INIT_FAILED;

            return NULL;
        }
    }
#endif


#ifdef NLM_12K_11K
    self->m_11kDevMgr_p = (void*)
            Nlm11kDevMgr__create(alloc_p, xpt_p, operMode, o_reason);

    if(self->m_11kDevMgr_p == NULL)
    {
        kbp_dm_destroy(self);
        self = NULL;
    }
    self->m_numOfAbs = (NLMDEV_NUM_ARRAY_BLOCKS/2);
    self->m_numOfABsPerSB = (self->m_numOfAbs/32);

#else
    self->m_11kDevMgr_p = NULL;
    
    /*Default assign 256 AB */
    self->m_numOfAbs = NLMDEV_NUM_ARRAY_BLOCKS;
    self->m_numOfABsPerSB = NLMDEV_NUM_AB_PER_SUPER_BLOCK;

    /*if 40M device assign 128 AB */
    if((NLM_DEVTYPE_3_40M == devType) || (NLM_DEVTYPE_3_N_40M == devType))
    {
        self->m_numOfAbs = NLMDEV_NUM_ARRAY_BLOCKS_40M;
        self->m_numOfABsPerSB = NLMDEV_NUM_AB_PER_SUPER_BLOCK_40M;
    }

#endif /* NLM_12K_11K */    
    
    return self;
}

/*
Function: kbp_dm_destroy
Description: destroys the instance of Device Manager and all devices in the Device Manager list
*/
 void kbp_dm_destroy(
    NlmDevMgr       *self
    )
{
    NlmCmAllocator *alloc_p;

    if (self != NULL)
    {
        alloc_p = self->m_alloc_p;

#ifdef NLM_12K_11K      
        if(self->m_11kDevMgr_p != NULL)
        {
            Nlm11kDevMgr__destroy(self->m_11kDevMgr_p);         
        }
#endif      
        NlmDevMgr__dtor(self);

#if defined NLM_MT_OLD || defined NLM_MT
        
        NlmCmMt__SpinDestroy( &self->m_spinLock, "NlmDevMgr_Kbp_SpinLock");
#endif


        NlmCmAllocator__free(alloc_p, self);
    }
}

/*
Function: Nlm1kDevMgr__AddDevice
Description: creates a new device and adds to device list of Device Manager.
If this function fails it returns NULL. User should see the reason code in case of failure.
*/
NlmDev* kbp_dm_add_device(
    NlmDevMgr           *self,
    NlmDevId                *o_devId,
    NlmReasonCode           *o_reason
    )
{
    NlmDev* dev_p = NULL;
    NlmDev   **devList_pp;
    
    NlmReasonCode dummyReason;

    if(o_reason == NULL)
        o_reason = &dummyReason;


    /* return NULL if there is no Device Manager. */
    if(!NLM_IS_VALID_DEVMGR_PTR(self))
    {
        *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NULL;
    }
    /* see, if Device Manager config is locked or not */
    if(NlmTrue == self->m_isLocked)
    {
        *o_reason = NLMRSC_DEV_MGR_CONFIG_LOCKED;
        return NULL;
    }

/* Check maximum number of devices for 11K/12K mode */
#ifndef NLM_12K_11K
    if(self->m_devCount > NLMDEV_MAX_DEV_NUM) 
    {
        *o_reason = NLMRSC_INVALID_NUM_OF_DEVICES;
        return NULL;
    }
#else
    if(self->m_11kDevMgr_p == NULL) 
    {
        *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NULL;        
    }
    if(self->m_devCount > NLMDEV_MAX_DEV_NUM_11K)
    {
        *o_reason = NLMRSC_INVALID_NUM_OF_DEVICES;
        return NULL;
    }
#endif


    if(o_devId == NULL)
    {
        *o_reason = NLMRSC_INVALID_OUTPUT;
        return NULL;
    }

    /* creating a new device here. */
    dev_p = (NlmDev*)NlmCmAllocator__calloc(self->m_alloc_p, 1, sizeof(NlmDev));
    NlmCmDemand((dev_p != NULL), "Out of memory.\n");
    if(dev_p == NULL)
    {
        *o_reason = NLMRSC_LOW_MEMORY;
        return NULL;
    }

    /* Initializing the device members */
    dev_p->m_devId = *o_devId = (NlmDevId)self->m_devCount;
    dev_p->m_devMgr_p = self;
    dev_p->m_magicNum = NLM_DEV_PTR_MAGIC_NUM;

    /* creating shadow device */
    if(NLMERR_OK != kbp_dm_shadow_create(dev_p, o_reason))
    {
        NlmCmAllocator__free(self->m_alloc_p, dev_p);
        return NULL;
    }

    /* storing the device pointer into device list */
    devList_pp = (NlmDev**)self->m_devList_pp;
    devList_pp[dev_p->m_devId] = dev_p;
    self->m_devCount++;

    return dev_p;
}


/*
Function: kbp_dm_reset_devices
Description: Resets all the devices in cascade by calling the XPT API which resets the devices
Note: This API does not reset the Shadow memory data to initial values; If application is
      interested in doing so it should invoke kbp_dm_shadow_destroy API followed
      by kbp_dm_shadow_create API
*/
NlmErrNum_t kbp_dm_reset_devices(
    NlmDevMgr       *self,
    NlmReasonCode       *o_reason
    )
{
    if(!NLM_IS_VALID_DEVMGR_PTR(self))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_NULL_PTR;
    }

    /* Call the XPT Function which resets the devices in cascade */
    return kbp_xpt_reset_device((NlmXpt*)self->m_xpt_p, o_reason);
}



/*
Function: kbp_dm_lock_config
Description: Locks the Device Manager configurations; No more devices can be
    added after Lock config
*/
NlmErrNum_t kbp_dm_lock_config(
    NlmDevMgr       *self,
    NlmReasonCode       *o_reason
    )
{
    NlmErrNum_t err = NLMERR_OK;

    if(!NLM_IS_VALID_DEVMGR_PTR(self))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_NULL_PTR;
    }
    if(NlmTrue == self->m_isLocked)
         return NLMERR_OK;

#ifdef NLM_12K_11K  
    
    if(self->m_11kDevMgr_p == NULL) 
    {
        *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_FAIL;     
    }
#endif


    /* calling transport layer lock configuration */
    err = kbp_xpt_lock_config((NlmXpt*)self->m_xpt_p, self->m_devCount, o_reason);

    if(NLMERR_OK == err)
        self->m_isLocked = NlmTrue;

#ifdef NLM_12K_11K  
    err = Nlm11kDevMgr__LockConfig(self->m_11kDevMgr_p, o_reason);      
#endif

#ifndef NLM_12K_11K

    if(NLMERR_OK != err)    /* this is for above err value */
        return err;


    /* Write the UDA configuration register for the device :
       We are doing this here because, each UDA config write takes 9K clock cycles
       
       currently it enables all 64 UDA SB, later we can modify need bases */
    {
        nlm_u8 devNum = 0;
        nlm_u32 sBlkNum = 0;

        for(devNum = 0; devNum < self->m_devCount; devNum++)
        {
            /* configure the UDA register, enable all blocks by default */
            {
                NlmDev *dev_p = NULL;
                NlmDevShadowDevice *shadowDev_p = NULL;
                
                dev_p       = self->m_devList_pp[devNum];
                if(dev_p == NULL)
                {
                    *o_reason = NLMRSC_INVALID_DEV_PTR;
                    return NLMERR_FAIL;     
                }
                shadowDev_p = ((NlmDevShadowDevice*)dev_p->m_shadowDevice_p);

                /* assign UDA SB*/
                for(sBlkNum = 0; sBlkNum < (NLMDEV_NUM_SRAM_SUPER_BLOCKS/4); sBlkNum++)
                    shadowDev_p->m_global->m_devUDAConfig.m_uSBEnable[sBlkNum] = NLMDEV_ENABLE;

                if((err = kbp_dm_global_reg_write(dev_p, NLMDEV_PORT_0, 
                    NLMDEV_UDA_CONFIG_REG, &(shadowDev_p->m_global->m_devUDAConfig), o_reason))!= NLMERR_OK )
                {
                    NlmCm__printf("\n\t kbp_dm_global_reg_write() failed for UDA Config Write \n");
                    return err;
                }
            }
        }
    }

#endif
    return err;
}


/*
    Function : kbp_dm_send_nop
    Description: kbp_dm_send_nop (NOP) pass NOP instruction to the
    device for specified count (number of times).
    User should see the reason code in case of failure.
*/
NlmErrNum_t kbp_dm_send_nop(
    NlmDev              *dev,
    nlm_u8              portNum, /* Port number */
    nlm_u32             numTimes,
    NlmReasonCode*      o_reason
    )
{
    nlm_u8  smtNum = NLM_DEFAULT_SMT_NUM;

    NlmXptRqt* rqt_p = NULL;

    if(!NLM_IS_VALID_DEV_PTR(dev))
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

#ifdef NLM_12K_11K

    if(dev->m_devMgr_p->m_11kDevMgr_p == NULL) 
    {
        *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_FAIL;     
    }   

    /* Code is same in 11K also after this so no need to differentiate between 11K and 12K */
#else
    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
#endif

    while(numTimes)
    {
        /* preparing write request for NOP */
        rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
        if(NULL == rqt_p)
        {
            if(o_reason)
                *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
            return NLMERR_NULL_PTR;
        }
        /* Clearing the rqt structure */
        NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

        /* assigning request structure members */
        rqt_p->m_opcode[0] = NLM_OPCODE_NOP_BITS_8_6;
        rqt_p->m_opcode[1] = NLM_OPCODE_NOP_BITS_5_0;

        /* data length 10Byte */
        rqt_p->m_data_len = NLMDEV_AB_WIDTH_IN_BYTES;

        /* In case of Database Write; "result" field of xpt rqt will be NULL */
        rqt_p->m_result = NULL;

        rqt_p->m_port_num = portNum;
        rqt_p->m_smt_num  = smtNum;

        /* calling transport layer to work on the current request */
        kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
        if(o_reason)
        {
            if(NLMRSC_REASON_OK != *o_reason)
                return NLMERR_FAIL;
        }
        --numTimes;
    }
    return NLMERR_OK;
}


/*
Function: kbp_dm_block_reg_write
Description: writes to block register of specified abNum depending on block register type.
An appopriate structure based on Reg Type needs to be passed as *data.
For BCR -- structure to be used NlmDevBlockConfigReg
For BMR -- structure to be used NlmDevBlkMaskReg
User should see the reason code in case of failure.
*/
 NlmErrNum_t kbp_dm_block_reg_write(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16              abNum,     /* AB number in which register lies */
    NlmDevBlockRegType   regType,   /* see the enum description */
    const void          *data,      /*appropriate structure pointer */
    NlmReasonCode       *o_reason
    )
{

#ifndef NLM_12K_11K
    NlmXptRqt* rqt_p = NULL;
    nlm_32     address =0;
    nlm_u8 regData[NLMDEV_REG_LEN_IN_BYTES] = "";
    nlm_u8 *data_ptr = NULL, smtNum;
#else
    (void) portNum;
#endif

    if(!NLM_IS_VALID_DEV_PTR(dev))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K  
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
    if(abNum >= dev->m_devMgr_p->m_numOfAbs)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_AB_NUM;
        return NLMERR_FAIL;
    }
    if(regType >= NLMDEV_BLOCK_REG_END)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_REG_ADDRESS;
        return NLMERR_FAIL;
    }
    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
#else

    if( dev->m_devMgr_p->m_11kDevMgr_p == NULL)
    {
        *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_FAIL;     
    }

    /* In case of 11k mode, call 11k specific function and return */
    return Nlm11kDevMgr__BlockRegisterWrite((Nlm11kDev*) dev, (nlm_u8)abNum, regType, data, o_reason);
    
#endif

#ifndef NLM_12K_11K 

    /* preparing write request for register. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_REG_WRITE_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_REG_WRITE_BITS_5_0;

    /* length of the register data */
    rqt_p->m_data_len = NLMDEV_REG_LEN_IN_BYTES;

    /* In case of Reg Write; "result" field of xpt rqt will be NULL */
    rqt_p->m_result = NULL;

    /* Find the single device address of the corresponding register_type
     * Addresses of Block Registers of a block are continoues based on RegType;
     * Get the base address (BCR Address) and then add regtype to it
     */
    address = NLM_REG_ADDR_BLK_CONFIG(abNum) + regType;

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_address = address;

    /* pointer to the register data */
    data_ptr = rqt_p->m_data_p = regData;
#else
    (void)regData;

    /* In case of Reg write, Byte[3:0] of the "data" field of xpt rqt
     * will contain the address and Byte[13:4] will contain the register data
     */
    WriteBitsInArray(rqt_p->m_data,
                     NLMDEV_REG_ADDR_LEN_IN_BYTES,
                     (NLMDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
                     0,
                     address
                    );
    rqt_p->m_data_len += NLMDEV_REG_ADDR_LEN_IN_BYTES;

    /* pointer to the register data : Byte[13:4] */
    data_ptr = rqt_p->m_data + NLMDEV_REG_ADDR_LEN_IN_BYTES;
#endif

    /* writing register contents into data field of request structure */
    if(regType == NLMDEV_BLOCK_CONFIG_REG)
    {
        NlmDevBlockConfigReg *blkConfigData_p = (NlmDevBlockConfigReg *)data;

        NlmDevMgr_pvt_ConstructBlockConfigRegData( data_ptr, blkConfigData_p );
    }
    else
    {
        NlmDevBlockMaskReg *blkMaskData_p;

        /* For BMR copy the 80b data passed with the structure */
        blkMaskData_p = (NlmDevBlockMaskReg *)data;

#ifdef NLM_NO_MEMCPY_IN_XPT
        /* request pointer will point to the Block mask data to be written */
        rqt_p->m_data_p = blkMaskData_p->m_mask;

#else
        /* copy the register data to the request structure */
        NlmCm__memcpy(data_ptr,
                blkMaskData_p->m_mask,
                NLMDEV_REG_LEN_IN_BYTES);
#endif
    }

    /* Get the SMT number from block number. */
    smtNum = ((dev->m_bankNum & (1 << (abNum/(NLMDEV_NUM_SB_PER_AC * dev->m_devMgr_p->m_numOfABsPerSB)))) == 0) ? 0 : 1;

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
            return NLMERR_FAIL;
    }
    return NLMERR_OK;
#endif

}



/*
Function: kbp_dm_block_reg_read
Description: Reads from block register of specified abNum depending on block register type.
An appopriate structure based on Reg Type needs to be passed as *data;
This function will update the fields of this structure based on data read from device
For BCR -- structure to be used NlmDevBlockConfigReg
For BMR -- structure to be used NlmDevBlkMaskReg
User should see the reason code in case of failure.
*/
 NlmErrNum_t kbp_dm_block_reg_read(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16             abNum,      /* AB number in which register lies */
    NlmDevBlockRegType  regType,    /* see the enum description */
    void                    *o_data,    /*appropriate structure pointer */
    NlmReasonCode*          o_reason
    )
{
#ifndef NLM_12K_11K  
    NlmXptRqt* rqt_p = NULL;
    nlm_32     address = 0;
    nlm_u8      smtNum;
    nlm_u8 readData[NLMDEV_REG_LEN_IN_BYTES + 1]; /* Extra byte is for control bits
                                                     such as VBIT, parity */
#else
    (void) portNum;
#endif

    if(!NLM_IS_VALID_DEV_PTR(dev))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }

#ifndef NLM_12K_11K  

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
    if(abNum >= dev->m_devMgr_p->m_numOfAbs)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_AB_NUM;
        return NLMERR_FAIL;
    }
    if(regType >= NLMDEV_BLOCK_REG_END)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_REG_ADDRESS;
        return NLMERR_FAIL;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }

#else
    
        if( dev->m_devMgr_p->m_11kDevMgr_p == NULL)
        {
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
            return NLMERR_FAIL;     
        }
    
        /* In case of 11k mode, call 11k specific function and return */
        return Nlm11kDevMgr__BlockRegisterRead((Nlm11kDev*)dev,
                            (nlm_u8)abNum, regType, o_data, o_reason);
        
#endif

#ifndef NLM_12K_11K 

    /* preparing request for register read. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_REG_READ_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_REG_READ_BITS_5_0;

    rqt_p->m_data_len = NLMDEV_REG_ADDR_LEN_IN_BYTES;

    /* In case of Reg read; XPT layer send the 80b read data via "result" field of xpt rqt;
    In addition to this result will contain some control bits such as vbit and parity which are ignored */
    rqt_p->m_result = readData;

    /* find the single device address of the corresponding register_type */
    /* Addresses of Block Registers of a block are continoues based on RegType;
        Get the base address (BCR Address) and then add regtype to it*/
    address = NLM_REG_ADDR_BLK_CONFIG(abNum) + regType;

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_address = address;
#else
    /* In case of Reg Read, Byte[3:0] of the "data" field of xpt rqt will contain the address*/
    WriteBitsInArray(
                rqt_p->m_data,
                NLMDEV_REG_ADDR_LEN_IN_BYTES,
                (NLMDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
                0,
                address
                );
#endif

    /* Get the SMT number from block number. */
    smtNum = ((dev->m_bankNum & (1 << (abNum/(NLMDEV_NUM_SB_PER_AC *dev->m_devMgr_p->m_numOfABsPerSB)))) == 0) ? 0 : 1;

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            kbp_xpt_discard_result((NlmXpt*)dev->m_devMgr_p->m_xpt_p, rqt_p, NULL);
            return NLMERR_FAIL;
        }
    }

    /* calling transport layer to get the results of the current request */
    if(NULL == kbp_xpt_get_result((NlmXpt*)dev->m_devMgr_p->m_xpt_p, rqt_p, NULL))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
        return NLMERR_NULL_PTR;
    }

    /* extracting register contents from result field of request structure */
    if(regType == NLMDEV_BLOCK_CONFIG_REG)
    {
        /* Ignoring Byte 0 for control fields */
        NlmDevMgr_pvt_ExtractBlockConfigRegData(readData + 1, (NlmDevBlockConfigReg*)o_data);
    }
    else
    {
        NlmDevBlockMaskReg *blkMaskData_p;

        /* For BMR copy the 80b read data to the structure */
        blkMaskData_p = (NlmDevBlockMaskReg*)o_data;
        NlmCm__memcpy(blkMaskData_p->m_mask,
                      readData + 1,  /* Ignoring Byte 0 for control fields */
                      NLMDEV_REG_LEN_IN_BYTES);
    }

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;
    return NLMERR_OK;
#endif

}

/*
Function: kbp_dm_ltr_write
Description: writes to ltr register of specified ltrNum and specified ltr register type.
An appopriate structure based on Reg Type needs to be passed as *data.
For Blk Select Reg -- structure to be used NlmDevBlkSelectReg
For Super Blk Key Select Reg -- structure to be used NlmDevSuperBlkKeyMapReg
For Parallel Srch Reg -- structure to be used NlmDevParallelSrchReg
For Range Insert 0 Reg -- structure to be used NlmDevRangeInsertion0Reg
For Range Insert 1 Reg -- structure to be used NlmDevRangeInsertion1Reg
For Miscelleneous Reg -- structure to be used NlmDevMiscelleneousReg
For Key Construction Reg -- structure to be used NlmDevKeyConstructReg
User should see the reason code in case of failure.

Note: Advanced Srch LTR is not yet supported
*/
 NlmErrNum_t kbp_dm_ltr_write(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u8              ltrNum,     /* LTR profile set number */
    NlmDevLtrRegType        regType,    /* see the structure description above */
    const void          *data,      /* LTR register type structure pointer */
    NlmReasonCode*      o_reason
    )
{
    NlmErrNum_t errNum = NLMERR_OK;
#ifndef NLM_12K_11K  
    NlmXptRqt* rqt_p = NULL;
    nlm_32     address =0;

    nlm_u8 regData[NLMDEV_REG_LEN_IN_BYTES] = "";
    nlm_u8 *data_ptr = NULL, smtNum = NLM_DEFAULT_SMT_NUM;
#else
    (void) portNum;
#endif

    if(!NLM_IS_VALID_DEV_PTR(dev))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }
 #ifndef NLM_12K_11K   
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
    if(ltrNum >= NLMDEV_MAX_NUM_LTRS)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_LTR_NUM;
        return NLMERR_FAIL;
    }
    if(regType >= NLMDEV_LTR_REG_END)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_REG_ADDRESS;
        return NLMERR_FAIL;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
#else

    if(dev->m_devMgr_p->m_11kDevMgr_p == NULL)
    {
        *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_FAIL;     
    }

    /* In case of 11k mode, call 11k specific function and return */
    {
        Nlm11kDevLtrRegType regType11k; 

        errNum = kbp_dm_pvt_11k_map_ltrs(regType, &regType11k, o_reason);
        if(errNum != NLMERR_OK)
            return errNum;
                
        errNum = Nlm11kDevMgr__LogicalTableRegisterWrite((Nlm11kDev*)dev, ltrNum, regType11k, data, o_reason);
        return errNum;          
    }   
#endif

#ifndef NLM_12K_11K 

    /* preparing request for register write. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }

    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_REG_WRITE_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_REG_WRITE_BITS_5_0;

    /* length of the register data */
    rqt_p->m_data_len = NLMDEV_REG_LEN_IN_BYTES;

    /* In case of Reg Write; "result" field of xpt rqt will be NULL */
    rqt_p->m_result = NULL;

#ifdef NLM_NO_MEMCPY_IN_XPT
    /* pointer to the register data */
    data_ptr = rqt_p->m_data_p = regData;

#else
    (void)regData;

    /* pointer to the register data : Byte[13:4]*/
    data_ptr = rqt_p->m_data + NLMDEV_REG_ADDR_LEN_IN_BYTES;
#endif

    /* writing register contents into data field of request structure */
    switch(regType)
    {
        case NLMDEV_BLOCK_SELECT_0_LTR:
        case NLMDEV_BLOCK_SELECT_1_LTR:
        case NLMDEV_BLOCK_SELECT_2_LTR:
        case NLMDEV_BLOCK_SELECT_3_LTR:
        {
            address = NLM_REG_ADDR_LTR_BLOCK_SELECT0(ltrNum) + (regType - NLMDEV_BLOCK_SELECT_0_LTR);

            errNum = NlmDevMgr_pvt_ConstructBlkSelLtrData(data_ptr, (NlmDevBlkSelectReg*)data);

            break;
        }

        case NLMDEV_PARALLEL_SEARCH_0_LTR:
        case NLMDEV_PARALLEL_SEARCH_1_LTR:
        case NLMDEV_PARALLEL_SEARCH_2_LTR:
        case NLMDEV_PARALLEL_SEARCH_3_LTR:
        case NLMDEV_PARALLEL_SEARCH_4_LTR:
        case NLMDEV_PARALLEL_SEARCH_5_LTR:
        case NLMDEV_PARALLEL_SEARCH_6_LTR:
        case NLMDEV_PARALLEL_SEARCH_7_LTR:
        {
            address = NLM_REG_ADDR_LTR_PARALLEL_SEARCH0(ltrNum) + (regType - NLMDEV_PARALLEL_SEARCH_0_LTR);

            errNum = NlmDevMgr_pvt_ConstructParallelSrchLtrData(data_ptr, (NlmDevParallelSrchReg*)data);

            break;
        }

        case NLMDEV_RANGE_INSERTION_0_LTR:
        {
            address = NLM_REG_ADDR_LTR_RANGE_INSERTION0(ltrNum);
            errNum = NlmDevMgr_pvt_ConstructRangeInsertLtrData(data_ptr, (void*)data, regType);
            break;
        }
        case NLMDEV_RANGE_INSERTION_1_LTR:
        {
            address = NLM_REG_ADDR_LTR_RANGE_INSERTION1(ltrNum);
            errNum = NlmDevMgr_pvt_ConstructRangeInsertLtrData(data_ptr, (void*)data, regType);
            break;
        }

        case NLMDEV_SUPER_BLK_KEY_MAP_LTR:
        {
            address = NLM_REG_ADDR_LTR_SB_KPU_SELECT(ltrNum);

            errNum = NlmDevMgr_pvt_ConstructSBKeySelLtrData(data_ptr, (NlmDevSuperBlkKeyMapReg*)data);

            break;
        }

        case NLMDEV_EXT_CAPABILITY_REG_0_LTR:
        {
            address = NLM_REG_ADDR_LTR_EXTENDED0(ltrNum);

            errNum = NlmDevMgr_pvt_ConstructExtCap0LtrData(data_ptr, (NlmDevExtCap0Reg*)data);

            break;
        }

        case NLMDEV_KEY_0_KCR_0_LTR:
        case NLMDEV_KEY_0_KCR_1_LTR:
        case NLMDEV_KEY_1_KCR_0_LTR:
        case NLMDEV_KEY_1_KCR_1_LTR:
        case NLMDEV_KEY_2_KCR_0_LTR:
        case NLMDEV_KEY_2_KCR_1_LTR:
        case NLMDEV_KEY_3_KCR_0_LTR:
        case NLMDEV_KEY_3_KCR_1_LTR:
        {
            address = NLM_REG_ADDR_LTR_KPU0_KEY_CONSTRUCTION0(ltrNum) + (regType - NLMDEV_KEY_0_KCR_0_LTR);

            errNum = NlmDevMgr_pvt_ConstructKeyContructLtrData(data_ptr, (NlmDevKeyConstructReg*)data);

            break;
        }

        case NLMDEV_OPCODE_EXT_LTR:
        {
            NlmDevMgr   *devMgr_p;
            NlmDevOpCodeExtReg  *opCodeExtData_p = (NlmDevOpCodeExtReg *)data;
            nlm_u32 *ptr;
            nlm_u8  index =  0, whichBit  = 0;

            address = NLM_REG_ADDR_LTR_OP_CODE_EXT(ltrNum);

            errNum = NlmDevMgr_pvt_ConstructOpCodeExtLtrData(data_ptr, opCodeExtData_p);

            /* Set appropriate bit if lclOpCode is set to 0x5 for LPM searches */
            if(opCodeExtData_p->m_lclOpCode == NLMDEV_OPCODE_EXT_LCLOPCODE_LPM)
            {
                devMgr_p = dev->m_devMgr_p;

                /* First find the array index and then the bit within it */
                index = ltrNum / 32;
                whichBit = ltrNum % 32;

                ptr    = &devMgr_p->m_lpmCmp3[index];
                *ptr |= (0x1 << whichBit);
            }

            break;
        }

        default:
        {
            if(o_reason)
                *o_reason = NLMRSC_INVALID_REG_ADDRESS;

            return NLMERR_FAIL;
        }
    } /* switch */

    if(errNum != NLMERR_OK)
    {
        /* If any failure due to invalid input param; reset the requests and return error */
        kbp_xpt_reset_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;

        return NLMERR_FAIL;
    }

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_address = address;

    /* pointer to the register data */
    data_ptr = rqt_p->m_data_p = regData;

#else
    (void)regData;

    /* In case of Reg write, Byte[3:0] of the "data" field of xpt rqt
     will contain the address and Byte[13:4] will contain the 80b register data */
    rqt_p->m_data_len += NLMDEV_REG_ADDR_LEN_IN_BYTES;

    /* Byte[3:0] of the "data" field of xpt rqt will contain the address*/
    WriteBitsInArray(rqt_p->m_data,
                     NLMDEV_REG_ADDR_LEN_IN_BYTES,
                    (NLMDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
                     0,
                     address
                    );
#endif

    /* Get the SMT number from LTR number. */
    if(dev->m_devMgr_p->m_smtMode == NLMDEV_DUAL_SMT_MODE)
    {
        if(ltrNum < NLM_MAX_NUM_LTR_PER_SMT)
            smtNum = NLMDEV_SMT_0;
        else
            smtNum = NLMDEV_SMT_1;
    }

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
            return NLMERR_FAIL;
    }

    return NLMERR_OK;
#endif
}



/*
Function: kbp_dm_ltr_read
Description: Reads from ltr register of specified ltrNum and specified ltr register type.
An appopriate structure based on Reg Type needs to be passed as *data.
For Blk Select Reg -- structure to be used NlmDevBlkSelectReg
For Super Blk Key Select Reg -- structure to be used NlmDevSuperBlkKeyMapReg
For Parallel Srch Reg -- structure to be used NlmDevParallelSrchReg
For Range Insert 0 Reg -- structure to be used NlmDevRangeInsertion0Reg
For Range Insert 1 Reg -- structure to be used NlmDevRangeInsertion1Reg
For Miscelleneous Reg -- structure to be used NlmDevMiscelleneousReg
For NetLogic Internal Reg -- structure to be used NlmDevSSReg
For Key Construction Reg -- structure to be used NlmDevKeyConstructReg
User should see the reason code in case of failure.

Note: Advanced Srch LTR is not yet supported
*/
 NlmErrNum_t kbp_dm_ltr_read(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u8                  ltrNum,     /* LTR profile set number */
    NlmDevLtrRegType        regType,    /* see the structure description above */
    void                    *o_data,    /* LTR register type structure pointer */
    NlmReasonCode*          o_reason
    )
{
#ifndef NLM_12K_11K 
       NlmXptRqt* rqt_p = NULL;
    nlm_32     address =0;
    nlm_u8 smtNum = NLM_DEFAULT_SMT_NUM;
    nlm_u8 readData[NLMDEV_REG_LEN_IN_BYTES + 1];/* Extra byte is for control bits
                                                     such as VBIT, parity */
#else
     NlmErrNum_t errNum = NLMERR_OK;
    (void) portNum;
#endif

    if(!NLM_IS_VALID_DEV_PTR(dev))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K 
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
    if(ltrNum >= NLMDEV_MAX_NUM_LTRS)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_LTR_NUM;
        return NLMERR_FAIL;
    }
    if(regType >= NLMDEV_LTR_REG_END)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_REG_ADDRESS;
        return NLMERR_FAIL;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
#else

    if(dev->m_devMgr_p->m_11kDevMgr_p == NULL)
    {
        *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_FAIL;     
    }

    /* In case of 11k mode, call 11k specific function and return */
    {
        Nlm11kDevLtrRegType regType11k; 

        errNum = kbp_dm_pvt_11k_map_ltrs(regType, &regType11k, o_reason);
        if(errNum != NLMERR_OK)
            return errNum;
                
        errNum = Nlm11kDevMgr__LogicalTableRegisterRead((Nlm11kDev*)dev, 
                        ltrNum, regType11k, o_data, o_reason);
        return errNum;          
    }   
#endif

#ifndef NLM_12K_11K 

    /* preparing request for register read. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_REG_READ_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_REG_READ_BITS_5_0;

    rqt_p->m_data_len = NLMDEV_REG_ADDR_LEN_IN_BYTES;

    /* In case of Reg read; XPT layer send the 80b read data via "result" field of xpt rqt;
    In addition to this result will contain some control bits such as vbit and parity which are ignored */
    rqt_p->m_result = readData;

    /* Find the single device address of the corresponding register_type */
    switch(regType)
    {
        case NLMDEV_BLOCK_SELECT_0_LTR:
        case NLMDEV_BLOCK_SELECT_1_LTR:
        case NLMDEV_BLOCK_SELECT_2_LTR:
        case NLMDEV_BLOCK_SELECT_3_LTR:
            address = NLM_REG_ADDR_LTR_BLOCK_SELECT0(ltrNum) + (regType - NLMDEV_BLOCK_SELECT_0_LTR);
            break;

        case NLMDEV_PARALLEL_SEARCH_0_LTR:
        case NLMDEV_PARALLEL_SEARCH_1_LTR:
        case NLMDEV_PARALLEL_SEARCH_2_LTR:
        case NLMDEV_PARALLEL_SEARCH_3_LTR:
        case NLMDEV_PARALLEL_SEARCH_4_LTR:
        case NLMDEV_PARALLEL_SEARCH_5_LTR:
        case NLMDEV_PARALLEL_SEARCH_6_LTR:
        case NLMDEV_PARALLEL_SEARCH_7_LTR:
            address = NLM_REG_ADDR_LTR_PARALLEL_SEARCH0(ltrNum) + (regType - NLMDEV_PARALLEL_SEARCH_0_LTR);
            break;

        case NLMDEV_SUPER_BLK_KEY_MAP_LTR:
            address = NLM_REG_ADDR_LTR_SB_KPU_SELECT(ltrNum);
            break;

        case NLMDEV_EXT_CAPABILITY_REG_0_LTR:
            address = NLM_REG_ADDR_LTR_EXTENDED0(ltrNum);
            break;

        case NLMDEV_KEY_0_KCR_0_LTR:
        case NLMDEV_KEY_0_KCR_1_LTR:
        case NLMDEV_KEY_1_KCR_0_LTR:
        case NLMDEV_KEY_1_KCR_1_LTR:
        case NLMDEV_KEY_2_KCR_0_LTR:
        case NLMDEV_KEY_2_KCR_1_LTR:
        case NLMDEV_KEY_3_KCR_0_LTR:
        case NLMDEV_KEY_3_KCR_1_LTR:
            address = NLM_REG_ADDR_LTR_KPU0_KEY_CONSTRUCTION0(ltrNum) + (regType - NLMDEV_KEY_0_KCR_0_LTR);
            break;

        case NLMDEV_RANGE_INSERTION_0_LTR:
            address = NLM_REG_ADDR_LTR_RANGE_INSERTION0(ltrNum);
            break;

        case NLMDEV_RANGE_INSERTION_1_LTR:
            address = NLM_REG_ADDR_LTR_RANGE_INSERTION1(ltrNum);
            break;

        case NLMDEV_OPCODE_EXT_LTR:
            address = NLM_REG_ADDR_LTR_OP_CODE_EXT(ltrNum);
            break;

        default:
        {
            if(o_reason)
                *o_reason = NLMRSC_INVALID_REG_ADDRESS;

            return NLMERR_FAIL;
        }
    } /* switch */

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev->m_devId);

     /* writing register contents into data field of request structure */
#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_address = address;
#else
    /* In case of Reg Read, Byte[3:0] of the "data" field of xpt rqt will contain the address*/
    WriteBitsInArray(
                rqt_p->m_data,
                NLMDEV_REG_ADDR_LEN_IN_BYTES,
                (NLMDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
                0,
                address
                );
#endif

    /* Get the SMT number from LTR number. */
    if(dev->m_devMgr_p->m_smtMode == NLMDEV_DUAL_SMT_MODE)
    {
        if(ltrNum < NLM_MAX_NUM_LTR_PER_SMT)
            smtNum = NLMDEV_SMT_0;
        else
            smtNum = NLMDEV_SMT_1;
    }

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            kbp_xpt_discard_result((NlmXpt*)dev->m_devMgr_p->m_xpt_p, rqt_p, NULL);
            return NLMERR_FAIL;
        }
    }

    /* calling transport layer to get the results of the current request */
    if(NULL == kbp_xpt_get_result((NlmXpt*)dev->m_devMgr_p->m_xpt_p, rqt_p, NULL))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
        return NLMERR_NULL_PTR;
    }

    /* extracting register contents from result field of request structure;
    Note Byte[0] contains some control fields which can be ignored */
    switch(regType)
    {
        case NLMDEV_BLOCK_SELECT_0_LTR:
        case NLMDEV_BLOCK_SELECT_1_LTR:
        case NLMDEV_BLOCK_SELECT_2_LTR:
        case NLMDEV_BLOCK_SELECT_3_LTR:
             NlmDevMgr_pvt_ExtractBlkSelLtrData(readData + 1, (NlmDevBlkSelectReg*)o_data);
             break;

        case NLMDEV_SUPER_BLK_KEY_MAP_LTR:
             NlmDevMgr_pvt_ExtractSBKeySelLtrData(readData + 1, (NlmDevSuperBlkKeyMapReg*)o_data);
             break;

        case NLMDEV_PARALLEL_SEARCH_0_LTR:
        case NLMDEV_PARALLEL_SEARCH_1_LTR:
        case NLMDEV_PARALLEL_SEARCH_2_LTR:
        case NLMDEV_PARALLEL_SEARCH_3_LTR:
        case NLMDEV_PARALLEL_SEARCH_4_LTR:
        case NLMDEV_PARALLEL_SEARCH_5_LTR:
        case NLMDEV_PARALLEL_SEARCH_6_LTR:
        case NLMDEV_PARALLEL_SEARCH_7_LTR:
             NlmDevMgr_pvt_ExtractParallelSrchLtrData(readData + 1, (NlmDevParallelSrchReg*)o_data);
             break;

        case NLMDEV_KEY_0_KCR_0_LTR:
        case NLMDEV_KEY_0_KCR_1_LTR:
        case NLMDEV_KEY_1_KCR_0_LTR:
        case NLMDEV_KEY_1_KCR_1_LTR:
        case NLMDEV_KEY_2_KCR_0_LTR:
        case NLMDEV_KEY_2_KCR_1_LTR:
        case NLMDEV_KEY_3_KCR_0_LTR:
        case NLMDEV_KEY_3_KCR_1_LTR:
             NlmDevMgr_pvt_ExtractKeyContructLtrData(readData + 1, (NlmDevKeyConstructReg*)o_data);
             break;

        case NLMDEV_OPCODE_EXT_LTR:
            NlmDevMgr_pvt_ExtractOpCodeExtLtrData(readData + 1, (NlmDevOpCodeExtReg*)o_data);
            break;

        case NLMDEV_EXT_CAPABILITY_REG_0_LTR:
            NlmDevMgr_pvt_ExtractExtCap0LtrData(readData + 1, (NlmDevExtCap0Reg*)o_data);
            break;

        case NLMDEV_RANGE_INSERTION_0_LTR:
        case NLMDEV_RANGE_INSERTION_1_LTR:
             NlmDevMgr_pvt_ExtractRangeInsertLtrData(readData + 1, o_data, regType);
             break;

        /*case NLMDEV_SS_LTR:
             NlmDevMgr_pvt_ExtractSSLtrData(readData + 1, o_data);
             break;*/

        default:
            if(o_reason)
                *o_reason = NLMRSC_INVALID_REG_ADDRESS;
            return NLMERR_FAIL;
    }

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;

    return NLMERR_OK;
#endif
}



/*
    Function: kbp_dm_ltr_refresh refreshes the LTR register data depending on LTR register type.
    It reads the data from Shadow Memory the data of specified register and writes it to device.
    This function will be useful to re-write the LTR Register Data which have suffered from soft parity error
     User should see the reason code in case of failure.
*/
NlmErrNum_t kbp_dm_ltr_refresh(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u8                  ltrNum,     /* LTR profile set number */
    NlmDevLtrRegType        regType,    /* see the structure description above */
    NlmReasonCode*          o_reason
    )
{

    NlmDevShadowDevice *shadowDev_p;
    void *data;
 NlmErrNum_t errNum = NLMERR_OK;    
 
#ifndef NLM_12K_11K 
    NlmXptRqt* rqt_p = NULL;
    nlm_32     address =0;
    nlm_u8 regData[NLMDEV_REG_LEN_IN_BYTES] = "";
    nlm_u8 *data_ptr = NULL, smtNum = NLM_DEFAULT_SMT_NUM;

#else
    (void) portNum;
#endif

    if(!NLM_IS_VALID_DEV_PTR(dev))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }
    
    shadowDev_p = NLM_GET_SHADOW_MEM_FROM_DEV_PTR(dev);
    if(shadowDev_p == NULL)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_SHADOWDEV_PTR;
        return NLMERR_NULL_PTR;
    }
    
#ifndef NLM_12K_11K 
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
    {
        if(o_reason)
            *o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
        return NLMERR_FAIL;
    }
    if(ltrNum >= NLMDEV_MAX_NUM_LTRS)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_LTR_NUM;
        return NLMERR_FAIL;
    }
    if(regType >= NLMDEV_LTR_REG_END)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_REG_ADDRESS;
        return NLMERR_FAIL;
    }
    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
#else
    if(dev->m_devMgr_p->m_11kDevMgr_p == NULL)
    {
        *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_FAIL;     
    }

    if(ltrNum >= NLM11KDEV_NUM_LTR_SET)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_LTR_NUM;
        return NLMERR_FAIL;
    }

    /* In case of 11k mode, call 11k specific function and return */
    {
        Nlm11kDevLtrRegType regType11k; 
        

        errNum = kbp_dm_pvt_11k_map_ltrs(regType, &regType11k, o_reason);
        if(errNum != NLMERR_OK)
            return errNum;

        errNum = kbp_dm_pvt_get_ltr_data_from_sm(dev, ltrNum, regType, &data, o_reason);
        if(errNum != NLMERR_OK)
            return errNum;
                
        errNum = Nlm11kDevMgr__LogicalTableRegisterWrite((Nlm11kDev*)dev, ltrNum, regType11k, data, o_reason);
        return errNum;          
    }
    
#endif

#ifndef NLM_12K_11K 

    /* preparing request for register write. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_REG_WRITE_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_REG_WRITE_BITS_5_0;

    /* length of the register data */
    rqt_p->m_data_len = NLMDEV_REG_LEN_IN_BYTES;

    /* In case of Reg Write; "result" field of xpt rqt will be NULL */
    rqt_p->m_result = NULL;

    /* find the single device address of the corresponding register_type */
    /* Addresses of LTR Registers of specified ltrNum are continoues based on RegType;
        Get the base address of specified LtrNum(Blk Select 0 Address) and then add regtype to it*/
    address = NLM_REG_ADDR_LTR_BLOCK_SELECT0(ltrNum) + regType;

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_address = address;

    /* pointer to the register data */
    data_ptr = rqt_p->m_data_p = regData;
#else
    (void)regData;

    /* In case of Reg write, Byte[3:0] of the "data" field of xpt rqt
     will contain the address and Byte[13:4] will contain the 80b register data */
    rqt_p->m_data_len += NLMDEV_REG_ADDR_LEN_IN_BYTES;

    /* Byte[3:0] of the "data" field of xpt rqt will contain the address*/
    WriteBitsInArray(
            rqt_p->m_data,
            NLMDEV_REG_ADDR_LEN_IN_BYTES,
            (NLMDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
            0,
            address
            );

    /* pointer to the register data : Byte[13:4]*/
    data_ptr = rqt_p->m_data + NLMDEV_REG_ADDR_LEN_IN_BYTES;
#endif

/* writing register contents into data field of request structure */
    switch(regType)
    {
        case NLMDEV_BLOCK_SELECT_0_LTR:
        case NLMDEV_BLOCK_SELECT_1_LTR:
        case NLMDEV_BLOCK_SELECT_2_LTR:
        case NLMDEV_BLOCK_SELECT_3_LTR:
        {
            /* Get the Data from shadow memory */
            data = &shadowDev_p->m_ltr[ltrNum].m_blockSelect[regType - NLMDEV_BLOCK_SELECT_0_LTR];
            errNum = NlmDevMgr_pvt_ConstructBlkSelLtrData(data_ptr, (NlmDevBlkSelectReg*)data);
            
            break;
        }        

        case NLMDEV_SUPER_BLK_KEY_MAP_LTR:
            /* Get the Data from shadow memory */
            data = &shadowDev_p->m_ltr[ltrNum].m_superBlkKeyMap;
            errNum = NlmDevMgr_pvt_ConstructSBKeySelLtrData(data_ptr, (NlmDevSuperBlkKeyMapReg*)data);
            break;

        case NLMDEV_PARALLEL_SEARCH_0_LTR:
        case NLMDEV_PARALLEL_SEARCH_1_LTR:
        case NLMDEV_PARALLEL_SEARCH_2_LTR:
        case NLMDEV_PARALLEL_SEARCH_3_LTR:
        case NLMDEV_PARALLEL_SEARCH_4_LTR:
        case NLMDEV_PARALLEL_SEARCH_5_LTR:
        case NLMDEV_PARALLEL_SEARCH_6_LTR:
        case NLMDEV_PARALLEL_SEARCH_7_LTR:
        
            /* Get the Data from shadow memory */
            data = &shadowDev_p->m_ltr[ltrNum].m_parallelSrch[regType - NLMDEV_PARALLEL_SEARCH_0_LTR];
            errNum = NlmDevMgr_pvt_ConstructParallelSrchLtrData(data_ptr, (NlmDevParallelSrchReg*)data);
             break;

        case NLMDEV_KEY_0_KCR_0_LTR:
        case NLMDEV_KEY_0_KCR_1_LTR:
        case NLMDEV_KEY_1_KCR_0_LTR:
        case NLMDEV_KEY_1_KCR_1_LTR:
        case NLMDEV_KEY_2_KCR_0_LTR:
        case NLMDEV_KEY_2_KCR_1_LTR:
        case NLMDEV_KEY_3_KCR_0_LTR:
        case NLMDEV_KEY_3_KCR_1_LTR:
            /* Get the Data from shadow memory */
            data = &shadowDev_p->m_ltr[ltrNum].m_keyConstruct[regType - NLMDEV_KEY_0_KCR_0_LTR];
            errNum = NlmDevMgr_pvt_ConstructKeyContructLtrData(data_ptr, (NlmDevKeyConstructReg*)data);
            break;

        case NLMDEV_RANGE_INSERTION_0_LTR:
        case NLMDEV_RANGE_INSERTION_1_LTR:
            /* Get the Data from shadow memory */
            if(regType == NLMDEV_RANGE_INSERTION_0_LTR)
                data = &shadowDev_p->m_ltr[ltrNum].m_rangeInsert0;
            else
                data = &shadowDev_p->m_ltr[ltrNum].m_rangeInsert1;

            errNum = NlmDevMgr_pvt_ConstructRangeInsertLtrData(data_ptr,    (void*)data, regType);
             break;

        case NLMDEV_EXT_CAPABILITY_REG_0_LTR:
            /* Get the Data from shadow memory */
            data = &shadowDev_p->m_ltr[ltrNum].m_extCap0;

            errNum = NlmDevMgr_pvt_ConstructExtCap0LtrData(data_ptr, (NlmDevExtCap0Reg*)data);
             break;
             
        case NLMDEV_OPCODE_EXT_LTR:
            /* Get the Data from shadow memory */
            data = &shadowDev_p->m_ltr[ltrNum].m_opCodeExt; 

            errNum = NlmDevMgr_pvt_ConstructOpCodeExtLtrData(data_ptr, (NlmDevOpCodeExtReg*)data);
             break;
        

        default:
            if(o_reason)
                *o_reason = NLMRSC_INVALID_REG_ADDRESS;
            return NLMERR_FAIL;
    }

    if(errNum != NLMERR_OK)
    {
        /* If any failure due to invalid input param; reset the requests and return error */
        kbp_xpt_reset_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;
        return NLMERR_FAIL;
    }

    /* Get the SMT number from LTR number. */
    if(dev->m_devMgr_p->m_smtMode == NLMDEV_DUAL_SMT_MODE)
    {
        if(ltrNum < NLM_MAX_NUM_LTR_PER_SMT)
            smtNum = NLMDEV_SMT_0;
        else
            smtNum = NLMDEV_SMT_1;
    }

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;
    
    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
            return NLMERR_FAIL;
    }


    return NLMERR_OK;
#endif
}




/*
Function: kbp_dm_global_reg_write
Description: writes to specified global register.
An appopriate structure based on Reg Type needs to be passed as *data.
For Device Config Reg -- structure to be used NlmDevConfigReg
For Error Status Mask Reg -- structure to be used NlmDevErrStatusReg
For Database Soft Error FIFO Reg -- structure to be used NlmDevDbSoftErrFifoReg
For Scratch Pad Reg -- structure to be used NlmDevResultReg

Write to DevId Reg, Error Status Register, Advanced Features Soft Error Register
and Result Registers is not a valid operation since they are read only Register;

User should see the reason code in case of failure.
*/
NlmErrNum_t kbp_dm_global_reg_write(
    NlmDev                  *dev,
    nlm_u8              portNum,  /* Port number */
    NlmDevGlobalRegType     regType,    /* Global register type - see definitions above */
    const void                  *data,      /* Global register structure pointer */
    NlmReasonCode*              o_reason
    )
{
#ifndef NLM_12K_11K 
    NlmXptRqt* rqt_p = NULL;
    nlm_32     address =0;
    nlm_u8 regData[NLMDEV_REG_LEN_IN_BYTES] = "";
    nlm_u8 *data_ptr = NULL, smtNum = NLM_DEFAULT_SMT_NUM;
#else
       NlmErrNum_t errNum = NLMERR_OK;
    (void)portNum;
#endif

    if(!NLM_IS_VALID_DEV_PTR(dev))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K  
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
    if(regType >= NLMDEV_GLOBALREG_END)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_REG_ADDRESS;
        return NLMERR_FAIL;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
    
#else

    if(dev->m_devMgr_p->m_11kDevMgr_p == NULL)
    {
        *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_FAIL;     
    }

    
    /* In case of 11k mode, call 11k specific function and return */
    {
        Nlm11kDevGlobalRegType regType11k; 
        NlmDevConfigReg *data_p = (NlmDevConfigReg *)data;

        Nlm11kDevConfigReg data_w ={0,};

        errNum = kbp_dm_pvt_11k_map_global_reg(regType, &regType11k, o_reason);
        if(errNum != NLMERR_OK)
            return errNum;
        
        if(regType11k == NLMDEV_DEVICE_CONFIG_REG)
        {
            data_w.m_rangeEngineEnable          = data_p->m_rangeEngineEnable;
            data_w.m_dbParityErrEntryInvalidate = data_p->m_dbParityErrEntryInvalidate;
            data_w.m_dbSoftErrProtectMode       = data_p->m_dbSoftErrProtectMode;
            data_w.m_eccScanType                = data_p->m_eccScanType;
            data_w.m_lowPowerModeEnable         = data_p->m_lowPowerModeEnable;
            data_w.m_softErrorScanEnable        = data_p->m_softErrorScanEnable;

            errNum = Nlm11kDevMgr__GlobalRegisterWrite((Nlm11kDev*)dev, 
                            regType11k, (void*)&data_w, o_reason);
            return errNum;
        }
        else
        {
            errNum = Nlm11kDevMgr__GlobalRegisterWrite((Nlm11kDev*)dev, 
                                regType11k, data, o_reason);
            return errNum;
        }
    }   
#endif

#ifndef NLM_12K_11K 

    /* preparing request for register write. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));


    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_REG_WRITE_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_REG_WRITE_BITS_5_0;

    /* length of the register data */
    rqt_p->m_data_len = NLMDEV_REG_LEN_IN_BYTES;

#ifdef NLM_NO_MEMCPY_IN_XPT
    data_ptr = regData;
#else
    (void)regData;

    /* In case of Reg write, Byte[3:0] of the "data" field of xpt rqt
     will contain the address and Byte[13:4] will contain the 80b register data */
    rqt_p->m_data_len += NLMDEV_REG_ADDR_LEN_IN_BYTES;

    /* pointer to the register data : Byte[13:4]*/
    data_ptr = rqt_p->m_data + NLMDEV_REG_ADDR_LEN_IN_BYTES;
#endif

    /* In case of Reg Write; "result" field of xpt rqt will be NULL */
    rqt_p->m_result = NULL;

    switch(regType)
    {
        case NLMDEV_DEVICE_CONFIG_REG:
        {
            NlmDevConfigReg *data_p = (NlmDevConfigReg *)data;

            /* Construct the address and data for Dev Config Reg */
            address = NLM_REG_ADDR_DEVICE_CONFIG;

        NlmDevMgr_pvt_ConstructDevConfigRegData( data_ptr, data_p );

        dev->m_bankNum = data_p->m_ACtoBankMapping;

            break;
        }

        case NLMDEV_UDA_CONFIG_REG:
        {
            NlmDevUDAConfigReg  *data_p = (NlmDevUDAConfigReg *)data;

            address = NLM_REG_ADDR_UDA_CONFIG;

            NlmDevMgr_pvt_ConstructUDAConfigRegData( data_ptr, data_p );

            break;
        }

        case NLMDEV_SCRATCH_PAD0_REG:
        case NLMDEV_SCRATCH_PAD1_REG:
            {
                NlmDevScratchPadReg *scratchPadData_p = (NlmDevScratchPadReg *)data;

                /* Construct the address and data for Scratch Pad Reg */
                address =  NLM_REG_ADDR_SCRATCH_PAD0 + (regType%2);
                #ifdef NLM_NO_MEMCPY_IN_XPT
                    data_ptr = scratchPadData_p->m_data;
                #else
                    NlmCm__memcpy(data_ptr,
                        scratchPadData_p->m_data,
                        NLMDEV_REG_LEN_IN_BYTES);
                #endif
                break;
            }

        case NLMDEV_DEVICE_ID_REG:
        case NLMDEV_ERROR_STATUS_REG:
        case NLMDEV_RESULT0_REG:
        case NLMDEV_RESULT1_REG:
        case NLMDEV_RESULT2_REG:
        {
            /* Read only Register; Return Error */
            kbp_xpt_reset_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);

            if(o_reason)

                *o_reason = NLMRSC_READONLY_REGISTER;
            return NLMERR_FAIL;
        }

        default:
        {
            if(o_reason)
                *o_reason = NLMRSC_INVALID_REG_ADDRESS;

            return NLMERR_FAIL;
        }
    }

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_address = address;

    rqt_p->m_data_p = data_ptr;
#else
    /* Byte[3:0] of the "data" field of xpt rqt will contain the address*/
    WriteBitsInArray(rqt_p->m_data,
                     NLMDEV_REG_ADDR_LEN_IN_BYTES,
                     (NLMDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
                     0,
                     address
                    );
#endif

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);

    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
            return NLMERR_FAIL;
    }

    return NLMERR_OK;
#endif
}





/*
Function: kbp_dm_global_reg_read
Description: Reads from specified global register.
An appopriate structure based on Reg Type needs to be passed as *data.
For Device Id Reg -- structure to be used NlmDevIdReg
For Device Config Reg -- structure to be used NlmDevConfigReg
For Error Status Reg -- structure to be used NlmDevErrStatusReg
For Error Status Mask Reg -- structure to be used NlmDevErrStatusReg
For Database Soft Error FIFO Reg -- structure to be used NlmDevDbSoftErrFifoReg
For Advanced Features Soft Error Reg -- structure to be used NlmDevAdvancedSoftErrReg
For Scratch Pad Registers -- structure to be used NlmDevResultReg
For Result Registers  -- structure to be used NlmDevResultReg
User should see the reason code in case of failure.
*/
NlmErrNum_t kbp_dm_global_reg_read(
    NlmDev                  *dev,
    nlm_u8              portNum,  /* Port number */
    NlmDevGlobalRegType     regType,        /* see the enum description */
    void                        *o_data,
    NlmReasonCode*              o_reason
    )
{

#ifndef NLM_12K_11K  
    NlmXptRqt* rqt_p = NULL;
    nlm_32  address = 0;
    nlm_u32 value;
    nlm_u8 smtNum = NLM_DEFAULT_SMT_NUM;
    nlm_u8 readData[NLMDEV_REG_LEN_IN_BYTES + 1];/* Extra byte is for control bits
                                                     such as VBIT, parity */
#else
    NlmErrNum_t errNum = NLMERR_OK;
    (void)portNum;
#endif
    if(!NLM_IS_VALID_DEV_PTR(dev))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K  
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
    if(regType >= NLMDEV_GLOBALREG_END)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_REG_ADDRESS;
        return NLMERR_FAIL;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
#else
    (void) portNum;
    if(dev->m_devMgr_p->m_11kDevMgr_p == NULL)
    {
        *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_FAIL;     
    }

    /* In case of 11k mode, call 11k specific function and return */
    {
        Nlm11kDevGlobalRegType regType11k; 
        NlmDevConfigReg *data_p = (NlmDevConfigReg *)o_data;
        Nlm11kDevConfigReg data_r ={0,};

        errNum = kbp_dm_pvt_11k_map_global_reg(regType, &regType11k, o_reason);
        if(errNum != NLMERR_OK)
            return errNum;
        
        if(NLMDEV_DEVICE_CONFIG_REG == regType11k)
        {
            errNum = Nlm11kDevMgr__GlobalRegisterRead((Nlm11kDev*)dev, 
                                regType11k, (void*)&data_r, o_reason);
            if(errNum == NLMERR_OK)
            {
                data_p->m_rangeEngineEnable          = data_r.m_rangeEngineEnable;
                data_p->m_dbParityErrEntryInvalidate = data_r.m_dbParityErrEntryInvalidate;
                data_p->m_dbSoftErrProtectMode       = data_r.m_dbSoftErrProtectMode;
                data_p->m_eccScanType                = data_r.m_eccScanType;
                data_p->m_lowPowerModeEnable         = data_r.m_lowPowerModeEnable;
                data_p->m_softErrorScanEnable        = data_r.m_softErrorScanEnable;
            }
            return errNum;
        }
        else
        {
            errNum = Nlm11kDevMgr__GlobalRegisterRead((Nlm11kDev*)dev,
                                        regType11k, o_data, o_reason);      
            return errNum;
        }
    }   
#endif

#ifndef NLM_12K_11K 

    /* preparing request for register read. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));


    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_REG_READ_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_REG_READ_BITS_5_0;

    rqt_p->m_data_len = NLMDEV_REG_ADDR_LEN_IN_BYTES;

    /* In case of Reg read; XPT layer send the 80b read data via "result" field of xpt rqt;
    In addition to this result will contain some control bits such as vbit and parity which are ignored */
    rqt_p->m_result = readData;

    /* find the single device address of the corresponding register_type */
    switch(regType)
    {
        case NLMDEV_DEVICE_ID_REG:
            address = NLM_REG_ADDR_DEVICE_ID;
            break;

        case NLMDEV_DEVICE_CONFIG_REG:
            address = NLM_REG_ADDR_DEVICE_CONFIG;
            break;

        case NLMDEV_UDA_CONFIG_REG:
            address = NLM_REG_ADDR_UDA_CONFIG;
            break;

        case NLMDEV_SCRATCH_PAD0_REG:
        case NLMDEV_SCRATCH_PAD1_REG:
            address =  NLM_REG_ADDR_SCRATCH_PAD0 + (regType - NLMDEV_SCRATCH_PAD0_REG);
            break;

        case NLMDEV_RESULT0_REG:
        case NLMDEV_RESULT1_REG:
        case NLMDEV_RESULT2_REG:
             address = NLM_RESULT_REG_ADDRESS_START + (regType - NLMDEV_RESULT0_REG);
            break;

#if NLM_DM_DC
        case NLMDEV_ERROR_STATUS_MASK_REG:
            address = NLM_REG_ADDR_ERROR_STATUS_MASK;
            break;

        case NLMDEV_DATABASE_SOFT_ERROR_FIFO_REG:
            address = NLM_REG_ADDR_SOFT_ERROR_FIFO;
            break;

        case NLMDEV_ERROR_STATUS_REG:
            address = NLM_REG_ADDR_ERROR_STATUS;
            break;

        case NLMDEV_ADVANCED_FEATURES_SOFT_ERROR_REG:
            address = NLM_REG_ADDR_ADVANCED_FEATURE_SOFT_ERROR;
            break;
#endif
        default:
            if(o_reason)
                *o_reason = NLMRSC_INVALID_REG_ADDRESS;
            return NLMERR_FAIL;
    }

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_address = address;
#else
    /* In case of Reg Read, Byte[3:0] of the "data" field of xpt rqt will contain the address*/
    WriteBitsInArray(
                rqt_p->m_data,
                NLMDEV_REG_ADDR_LEN_IN_BYTES,
                (NLMDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
                0,
                address
                );
#endif

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            kbp_xpt_discard_result((NlmXpt*)dev->m_devMgr_p->m_xpt_p, rqt_p, NULL);
            return NLMERR_FAIL;
        }
    }

    /* calling transport layer to get the results of the current request */
    if(NULL == kbp_xpt_get_result((NlmXpt*)dev->m_devMgr_p->m_xpt_p, rqt_p, NULL))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
        return NLMERR_NULL_PTR;
    }

    /* extracting register contents from result field of request structure;
    Note: Byte[0] contains some control fields which can be ignored */
    switch(regType)
    {
        case NLMDEV_DEVICE_CONFIG_REG:
                NlmDevMgr_pvt_ExtractDevConfigRegData(readData+1, (NlmDevConfigReg *)o_data);
                break;

        case NLMDEV_UDA_CONFIG_REG:
                NlmDevMgr_pvt_ExtractUDAConfigRegData(readData+1, (NlmDevUDAConfigReg *)o_data );
                break;

        case NLMDEV_SCRATCH_PAD0_REG:
        case NLMDEV_SCRATCH_PAD1_REG:
            {
                NlmDevScratchPadReg *scratchPadData_p = (NlmDevScratchPadReg *)o_data;
                NlmCm__memcpy(scratchPadData_p->m_data, readData + 1, NLMDEV_REG_LEN_IN_BYTES);
                break;
            }

        case NLMDEV_DEVICE_ID_REG:
            {
                NlmDevIdReg *devIdData_p = (NlmDevIdReg *)o_data;
                value = ReadBitsInArrray(readData + 1, NLMDEV_REG_LEN_IN_BYTES, 7, 0);
                /* Dev Id Reg has following Data
                Minor Die Revision -- Bits[2:0]
                Major Die Revision -- Bits[5:3]
                Database Size -- Bits[7:6] */
                devIdData_p->m_minorDieRev = (nlm_u8)(value & 0x7);
                devIdData_p->m_majorDieRev = (nlm_u8)((value >> 3)& 0x7);
                devIdData_p->m_databaseSize = (nlm_u8)((value >> 6)& 0x3);
                break;
            }

        case NLMDEV_RESULT0_REG:
        case NLMDEV_RESULT1_REG:
        case NLMDEV_RESULT2_REG:
            {
                NlmDevResultReg *rsltData_p = (NlmDevResultReg *)o_data;
                nlm_32 psNum;

                /* Rslt Register has HIT or MISS Flag at Bits 39 and 79,
                    HIT Address at Bits[23:0] and Bits[63:40] */
                for(psNum = 0; psNum < (NLMDEV_NUM_PARALLEL_SEARCHES/2); psNum++)
                {
                    rsltData_p->m_hitOrMiss[psNum] = (NlmDevMissHit)ReadBitsInArrray(readData + 1,
                                                    NLMDEV_REG_LEN_IN_BYTES,
                                                    39 + (psNum * 40),
                                                    39 + (psNum *40));

                    rsltData_p->m_hitAddress[psNum] = ReadBitsInArrray(readData + 1,
                                                    NLMDEV_REG_LEN_IN_BYTES,
                                                    23 + (psNum * 40),
                                                    (psNum *40));
                }
                break;
            }
#if NLM_DM_DC
        case NLMDEV_ERROR_STATUS_MASK_REG:
        case NLMDEV_ERROR_STATUS_REG:
            {
                NlmDevErrStatusReg *errStatusReg_p = (NlmDevErrStatusReg*)o_data;

                value = ReadBitsInArrray(readData + 1, NLMDEV_REG_LEN_IN_BYTES, 31, 0);

                /* Global GIO_L1 Enable is valid only for Error Status Mask Reg */
                if(regType == NLMDEV_ERROR_STATUS_MASK_REG)
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

                value = ReadBitsInArrray(readData + 1, NLMDEV_REG_LEN_IN_BYTES, 55, 32);
                errStatusReg_p->m_alignmentErr =  (nlm_u8)((value >> (48-32)) & 1);
                errStatusReg_p->m_framingCtrlWordErr =  (nlm_u8)((value >> (49-32)) & 1);
                errStatusReg_p->m_rxPCSEFifoParityErr =  (nlm_u8)((value >> (50-32)) & 1);

                 /* Global GIO_L0 Enable is valid only for Error Status Mask Reg */
                if(regType == NLMDEV_ERROR_STATUS_MASK_REG)
                {
                    value = ReadBitsInArrray(readData + 1, NLMDEV_REG_LEN_IN_BYTES, 79, 79);
                    errStatusReg_p->m_globalGIO_L0_Enable = (nlm_u8)(value & 1);
                 }

                break;
            }

        case NLMDEV_DATABASE_SOFT_ERROR_FIFO_REG:
            {
                NlmDevDbSoftErrFifoReg *dbSoftErrorFifoData_p = (NlmDevDbSoftErrFifoReg *)o_data;

                /* Erase FIFO and Erase FIFO Entry Field are Write Only Bits and hence are ignored */
                value = ReadBitsInArrray(readData + 1, NLMDEV_REG_LEN_IN_BYTES, 41, 40);
                dbSoftErrorFifoData_p->m_eraseFifo = (value >> 1 ) & 1;
                dbSoftErrorFifoData_p->m_eraseFifoEntry = (value & 1);

                value = ReadBitsInArrray(readData + 1, NLMDEV_REG_LEN_IN_BYTES, 23, 0);
                dbSoftErrorFifoData_p->m_errorAddrValid = (value >> 23) & 1;
                dbSoftErrorFifoData_p->m_pErrorY = (value >> 22) & 1;
                dbSoftErrorFifoData_p->m_pErrorX = (value >> 21) & 1;
                /* Bits[20:0] give error Address */
                dbSoftErrorFifoData_p->m_errorAddr = value & 0x1FFFFF;
                break;
            }

        case NLMDEV_ADVANCED_FEATURES_SOFT_ERROR_REG:
            {
                NlmDevAdvancedSoftErrReg *advancedSoftErrData_p = (NlmDevAdvancedSoftErrReg *)o_data;

                advancedSoftErrData_p->m_cbParityErrAddr = (nlm_u16)(ReadBitsInArrray(readData + 1,
                                                                    NLMDEV_REG_LEN_IN_BYTES,
                                                                    13, 0));

                advancedSoftErrData_p->m_sahasraParityErrAddr0  = (nlm_u16)(ReadBitsInArrray(readData + 1,
                                                                    NLMDEV_REG_LEN_IN_BYTES,
                                                                    29, 16));

                advancedSoftErrData_p->m_sahasraParityErrAddr1  = (nlm_u16)(ReadBitsInArrray(readData + 1,
                                                                NLMDEV_REG_LEN_IN_BYTES,
                                                                45, 32));

                advancedSoftErrData_p->m_ltrParityErrAddr = (nlm_u16)(ReadBitsInArrray(readData + 1,
                                                                    NLMDEV_REG_LEN_IN_BYTES,
                                                                    58, 48));
                break;

            }
#endif
        default:
             if(o_reason)
                 *o_reason = NLMRSC_INVALID_REG_ADDRESS;
             return NLMERR_FAIL;
    }


    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;
    return NLMERR_OK;
#endif
}



/*
Function: kbp_dm_cb_reg_write
Description: writes to the specified address of the CB Memory as a 80b register write

User should see the reason code in case of failure.
Note: Address should be offset address i.e 0 - 16383 ( NLMDEV_CB_DEPTH - 1)
*/
 NlmErrNum_t kbp_dm_cb_reg_write(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16                 cbAddr,
    NlmDevCtxBufferReg  *cbRegData,         /* see the structure description above */
    NlmReasonCode*          o_reason
    )
{
#ifndef NLM_12K_11K   
    NlmXptRqt* rqt_p = NULL;
    nlm_32     address =0;
    nlm_u8  smtNum = NLM_DEFAULT_SMT_NUM;

#else
    (void)portNum;
#endif

    if(!NLM_IS_VALID_DEV_PTR(dev))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K   
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
    if(cbAddr >= NLMDEV_CB_DEPTH)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_CB_ADDRESS;
        return NLMERR_FAIL;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
#else

    if(dev->m_devMgr_p->m_11kDevMgr_p == NULL)
    {
        *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_FAIL;     
    }


    /* In case of 11k mode, call 11k specific function and return */
    return Nlm11kDevMgr__CBAsRegisterWrite((Nlm11kDev*) dev, 
                        cbAddr, (Nlm11kDevCtxBufferReg*)cbRegData, o_reason);       
#endif

#ifndef NLM_12K_11K 

    /* preparing request for register write. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_REG_WRITE_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_REG_WRITE_BITS_5_0;

    /* length of the register data */
    rqt_p->m_data_len = NLMDEV_REG_LEN_IN_BYTES;

    /* In case of Reg Write; "result" field of xpt rqt will be NULL */
    rqt_p->m_result = NULL;

    /* find the single device address of the specified CB register */
    address = NLM_REG_ADDR_CONTEXT_BUFFER(cbAddr);

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
    /* writing the address into request structure */
    rqt_p->m_address = address;

    /* request pointer will point to the CB data to be written */
    rqt_p->m_data_p = cbRegData->m_data;

#else
    /* In case of Reg write, Byte[3:0] of the "data" field of xpt rqt
    will contain the address and Byte[13:4] will contain the 80b register data */
    rqt_p->m_data_len += NLMDEV_REG_ADDR_LEN_IN_BYTES;

    /* Byte[3:0] of the "data" field of xpt rqt will contain the address*/
    WriteBitsInArray(
                rqt_p->m_data,
                NLMDEV_REG_ADDR_LEN_IN_BYTES,
                (NLMDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
                0,
                address
                );

    /* copy the CB data to be written in request structure */
    NlmCm__memcpy(rqt_p->m_data + NLMDEV_REG_ADDR_LEN_IN_BYTES,
                cbRegData->m_data,
                NLMDEV_REG_LEN_IN_BYTES);
#endif

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);

    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
            return NLMERR_FAIL;
    }

    return NLMERR_OK;
#endif
}



 /*
Function: kbp_dm_cb_reg_read
Description: reads from the specified address of the CB Memory as a 80b register read

User should see the reason code in case of failure.
Note: Address should be offset address i.e 0 - 16383 ( NLMDEV_CB_DEPTH - 1)
*/
 NlmErrNum_t kbp_dm_cb_reg_read(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16                 cbAddr,
    NlmDevCtxBufferReg  *o_cbRegData,           /* see the structure description above */
    NlmReasonCode*          o_reason
    )
{
#ifndef NLM_12K_11K 
    NlmXptRqt* rqt_p = NULL;
    nlm_32     address = 0;
    nlm_u8  smtNum = NLM_DEFAULT_SMT_NUM;
    nlm_u8 readData[NLMDEV_REG_LEN_IN_BYTES + 1];/* Extra byte is for control bits
                                                     such as VBIT, parity */
#else
    (void)portNum;
#endif

    if(!NLM_IS_VALID_DEV_PTR(dev))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K 
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
    if(cbAddr >= NLMDEV_CB_DEPTH)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_CB_ADDRESS;
        return NLMERR_FAIL;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
#else

    if(dev->m_devMgr_p->m_11kDevMgr_p == NULL)
    {
        *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_FAIL;     
    }   

    /* In case of 11k mode, call 11k specific function and return */
    return Nlm11kDevMgr__CBAsRegisterRead((Nlm11kDev*)dev, 
                        cbAddr, (Nlm11kDevCtxBufferReg *)o_cbRegData, o_reason);                
    
#endif

#ifndef NLM_12K_11K 

    /* preparing request for register read. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_REG_READ_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_REG_READ_BITS_5_0;

    rqt_p->m_data_len = NLMDEV_REG_ADDR_LEN_IN_BYTES;

    /* In case of Reg read; XPT layer send the 80b read data via "result" field of xpt rqt;
    In addition to this result will contain some control bits such as vbit and parity which are ignored */
    rqt_p->m_result = readData;

    /* find the single device address of the specified CB location */
    address = NLM_REG_ADDR_CONTEXT_BUFFER(cbAddr);

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_address = address;
#else
    /* In case of Reg Read, Byte[3:0] of the "data" field of xpt rqt will contain the address*/
    WriteBitsInArray(
                rqt_p->m_data,
                NLMDEV_REG_ADDR_LEN_IN_BYTES,
                (NLMDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
                0,
                address
                );
#endif

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            kbp_xpt_discard_result((NlmXpt*)dev->m_devMgr_p->m_xpt_p, rqt_p, NULL);
            return NLMERR_FAIL;
        }
    }

    /* calling transport layer to get the results of the current request */
    if(NULL == kbp_xpt_get_result((NlmXpt*)dev->m_devMgr_p->m_xpt_p, rqt_p, NULL))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
        return NLMERR_NULL_PTR;
    }

    /* extracting register contents from result field of request structure */
    NlmCm__memcpy(o_cbRegData->m_data,
                  readData + 1,  /* Ignoring Byte 0 for control fields */
                  NLMDEV_REG_LEN_IN_BYTES);

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;
    return NLMERR_OK;
#endif
}



/*
    Function kbp_dm_dba_write writes single Array Block Database entry
    to the specified addr and abNum.
    See Description of NlmDevABEntry for more details
    Data can be written in either DM or XY mode
    User should see the reason code in case of failure.
*/
NlmErrNum_t kbp_dm_dba_write(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16             abNum,      /* Specifies the DBA Block Number */
    nlm_u16             abAddr,     /* Specifies the Entry location within the specified abNum  */
    NlmDevABEntry   *abEntry,
    NlmDevWriteMode writeMode,
    NlmReasonCode*      o_reason
    )
{
#ifndef NLM_12K_11K
    NlmXptRqt* rqt_p = NULL;
    nlm_u32 address;
    nlm_u8  smtNum = NLM_DEFAULT_SMT_NUM;
#else

    (void)portNum;
    (void)abNum;
    (void)abAddr;
    (void)writeMode;
    (void)abEntry;
#endif
    if(!NLM_IS_VALID_DEV_PTR(dev))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }
 
#ifndef NLM_12K_11K
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
    if(dev->m_devMgr_p->m_numOfAbs <= abNum)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_AB_NUM;
        return NLMERR_FAIL;
    }
    if(NLMDEV_AB_DEPTH <= abAddr)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_AB_INDEX;
        return NLMERR_FAIL;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
#else

    if(dev->m_devMgr_p->m_11kDevMgr_p == NULL)
    {
        *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_FAIL;     
    }

    /* In case of 11k mode, call 11k specific function and return */
    return Nlm11kDevMgr__ABEntryWrite((Nlm11kDev*)dev, 
                    (nlm_u8)abNum, abAddr, (Nlm11kDevABEntry *)abEntry, writeMode, o_reason);               
        
#endif

#ifndef NLM_12K_11K 

    /* preparing write request for AB Entry. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_DBA_WRITE_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_DBA_WRITE_BITS_5_0;

    /* D-M or X-Y and hence multiply by 2 */
    rqt_p->m_data_len = (NLMDEV_AB_WIDTH_IN_BYTES * 2); /* Check here NKG */

    /* In case of Database Write; "result" field of xpt rqt will be NULL */
    rqt_p->m_result = NULL;

    /* find the single device address of the corresponding database entry */
    address = (abNum << 12) | abAddr;

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev->m_devId);

     /* Insert VBit, address type bits and Wrmode bits in the address */
    address |= (1 << NLMDEV_AB_ENTRY_VALID_BIT_IN_ADDR)
                |  (1 << NLMDEV_ADDR_TYPE_BIT_IN_PIO_WRITE)
                |  (writeMode << NLMDEV_AB_ENTRY_WR_MODE_BIT_IN_ADDR);

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
    rqt_p->m_data_len += NLMDEV_REG_ADDR_LEN_IN_BYTES;

    WriteBitsInArray(
                    rqt_p->m_data,
                    NLMDEV_AB_ADDR_LEN_IN_BYTES,
                    (NLMDEV_AB_ADDR_LEN_IN_BYTES * 8) - 1,
                    0,
                    address
                    );

    /* writing the DATA(X or D) into data field of request structure */
    NlmCm__memcpy(rqt_p->m_data + NLMDEV_AB_ADDR_LEN_IN_BYTES,
                abEntry->m_data, NLMDEV_AB_WIDTH_IN_BYTES);

    /* writing the MASK(Y or M) into data field of request structure*/
    NlmCm__memcpy((rqt_p->m_data + NLMDEV_AB_ADDR_LEN_IN_BYTES + NLMDEV_AB_WIDTH_IN_BYTES),
                abEntry->m_mask, NLMDEV_AB_WIDTH_IN_BYTES);
#endif

    /* Get the SMT number from block number. */
    smtNum = ((dev->m_bankNum & (1 << (abNum/(NLMDEV_NUM_SB_PER_AC *dev->m_devMgr_p->m_numOfABsPerSB)))) == 0) ? 0 : 1;

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
            return NLMERR_FAIL;
    }
    return NLMERR_OK;
#endif
}

/*
    Function kbp_dm_dba_invalidate invalidates single Array Block Database
    entry of the specified addr and abNum.

    User should see the reason code in case of failure.
*/
NlmErrNum_t kbp_dm_dba_invalidate(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16             abNum,      /* Specifies the AB Number */
    nlm_u16             abAddr,     /* Specifies the AB Entry location within the specified abNum  */
    NlmReasonCode*      o_reason
    )
{
#ifndef NLM_12K_11K 
    NlmXptRqt* rqt_p = NULL;
    nlm_u32 address;
    nlm_u8 regData[NLMDEV_REG_LEN_IN_BYTES] = "";
    nlm_u8 smtNum = NLM_DEFAULT_SMT_NUM;
#else
    (void) portNum;
#endif

    if(!NLM_IS_VALID_DEV_PTR(dev))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K 
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
    {
        if(o_reason)
            *o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
        return NLMERR_FAIL;
    }
    if(dev->m_devMgr_p->m_numOfAbs <= abNum)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_AB_NUM;
        return NLMERR_FAIL;
    }
    if(NLMDEV_AB_DEPTH <= abAddr)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_AB_INDEX;
        return NLMERR_FAIL;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }

#else

    if(dev->m_devMgr_p->m_11kDevMgr_p == NULL)
    {
        *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_FAIL;     
    }

    /* AB Entry is invalidated by performing a AB Entry write with VBIT in the address field as 0 */

    /* In case of 11k mode, call 11k specific function and return */
    return Nlm11kDevMgr__ABEntryInvalidate((Nlm11kDev*)dev, 
                        (nlm_u8)abNum, abAddr, o_reason);               
    
#endif

#ifndef NLM_12K_11K 

    /* preparing write request for AB Entry. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_DBA_WRITE_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_DBA_WRITE_BITS_5_0;

    rqt_p->m_data_len = (NLMDEV_AB_WIDTH_IN_BYTES * 2);

    /* In case of Database Write; "result" field of xpt rqt will be NULL */
    rqt_p->m_result = NULL;

    /* find the single device address of the corresponding database entry */
    address = (abNum << 12) | abAddr;

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev->m_devId);

     /* Insert address type bit in the address, VBIT is set to 0; wrmode does not matter */
    address |= (0 << NLMDEV_AB_ENTRY_VALID_BIT_IN_ADDR)
                |  (1 << NLMDEV_ADDR_TYPE_BIT_IN_PIO_WRITE);

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
    rqt_p->m_data_len += NLMDEV_REG_ADDR_LEN_IN_BYTES;

    WriteBitsInArray(
                rqt_p->m_data,
                NLMDEV_AB_ADDR_LEN_IN_BYTES,
                (NLMDEV_AB_ADDR_LEN_IN_BYTES * 8) - 1,
                0,
                address
                );
#endif

    /* Get the SMT number from block number. */
    smtNum = ((dev->m_bankNum & (1 << (abNum/(NLMDEV_NUM_SB_PER_AC *dev->m_devMgr_p->m_numOfABsPerSB)))) == 0) ? 0 : 1;

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
            return NLMERR_FAIL;
    }

    return NLMERR_OK;
#endif
}

/*
    Function kbp_dm_dba_read reads single Array Block Database entry
    from the specified addr and abNum.
    See Description of NlmDevABEntry for more details
    Data can be read in  XY mode only
    User should see the reason code in case of failure.
*/
NlmErrNum_t kbp_dm_dba_read(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16             abNum,      /* Specifies the AB Number */
    nlm_u16             abAddr,     /* Specifies the AB Entry location within the specified abNum  */
    NlmDevABEntry   *o_abEntry,
    NlmReasonCode*      o_reason
    )
{
#ifndef NLM_12K_11K
    NlmXptRqt* rqt_p = NULL;
    nlm_u32 address;
    nlm_u8 smtNum = NLM_DEFAULT_SMT_NUM;
    nlm_u8 readData[NLMDEV_REG_LEN_IN_BYTES + 1];/* Extra byte is for control bits
                                                     such as VBIT, parity */
#else
    (void)portNum;
#endif
    if(!NLM_IS_VALID_DEV_PTR(dev))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }

#ifndef NLM_12K_11K

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
    if(dev->m_devMgr_p->m_numOfAbs <= abNum)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_AB_NUM;
        return NLMERR_FAIL;
    }
    if(NLMDEV_AB_DEPTH <= abAddr)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_AB_INDEX;
        return NLMERR_FAIL;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
#else

    /* In case of 11k mode, call 11k specific function and return */
    return Nlm11kDevMgr__ABEntryRead((Nlm11kDev*)dev, 
                    (nlm_u8)abNum, abAddr, (Nlm11kDevABEntry*)o_abEntry, o_reason);             
        
#endif

#ifndef NLM_12K_11K


    /* find the single device address of the corresponding database entry */
    address = (abNum << 12) | abAddr;

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev->m_devId);

     /* Insert address type bit in the address */
    address |= (1 << NLMDEV_ADDR_TYPE_BIT_IN_PIO_WRITE);

    /* AB Entry Read operation required two Read operations
    one for Data_X and another for Data_Y */

    /* preparing request for AB Entry ReadX. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_DBA_READ_X_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_DBA_READ_X_BITS_5_0;

    rqt_p->m_data_len = NLMDEV_REG_ADDR_LEN_IN_BYTES;

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
                NLMDEV_AB_ADDR_LEN_IN_BYTES,
                (NLMDEV_AB_ADDR_LEN_IN_BYTES * 8) - 1,
                0,
                address
                );
#endif

    /* Get the SMT number from block number. */
    smtNum = ((dev->m_bankNum & (1 << (abNum/(NLMDEV_NUM_SB_PER_AC *dev->m_devMgr_p->m_numOfABsPerSB)))) == 0) ? 0 : 1;

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            kbp_xpt_discard_result((NlmXpt*)dev->m_devMgr_p->m_xpt_p, rqt_p, NULL);
            return NLMERR_FAIL;
        }
    }

    /* calling transport layer to get the results of the current request */
    if(NULL == kbp_xpt_get_result((NlmXpt*)dev->m_devMgr_p->m_xpt_p, rqt_p, NULL))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
        return NLMERR_NULL_PTR;
    }

    /* Get the Vbit which will be Bit 4 of Byte 0 of read Data */
    o_abEntry->m_vbit = (readData[0] >> 4) & 1;

    /* Copy the Data X read from the device */
    NlmCm__memcpy(o_abEntry->m_data, readData + 1, NLMDEV_AB_WIDTH_IN_BYTES);

    /* preparing request for AB Entry ReadY. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_DBA_READ_Y_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_DBA_READ_Y_BITS_5_0;

    rqt_p->m_data_len = NLMDEV_REG_ADDR_LEN_IN_BYTES;

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
                NLMDEV_AB_ADDR_LEN_IN_BYTES,
                (NLMDEV_AB_ADDR_LEN_IN_BYTES * 8) - 1,
                0,
                address
                );
#endif

    /* Get the SMT number from block number. */
    smtNum = ((dev->m_bankNum & (1 << (abNum/(NLMDEV_NUM_SB_PER_AC *dev->m_devMgr_p->m_numOfABsPerSB)))) == 0) ? 0 : 1;

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            kbp_xpt_discard_result((NlmXpt*)dev->m_devMgr_p->m_xpt_p, rqt_p, NULL);
            return NLMERR_FAIL;
        }
    }

    /* calling transport layer to get the results of the current request */
    if(NULL == kbp_xpt_get_result((NlmXpt*)dev->m_devMgr_p->m_xpt_p, rqt_p, NULL))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
        return NLMERR_NULL_PTR;
    }

    /* Copy the Data Y read from the device; Note Data_Y Read does not give VBIT status */
    NlmCm__memcpy(o_abEntry->m_mask, readData + 1, NLMDEV_AB_WIDTH_IN_BYTES);

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;
    return NLMERR_OK;
#endif
}


/*
Function: kbp_dm_dba_refresh
Description: Refreshes one entry depending on entry index and DBA number.
It reads the data at that location from Shadow Memory and writes it to device.
This function will be useful to re-write the entries which have suffered from soft parity error
User should see the reason code in case of failure.
*/
NlmErrNum_t kbp_dm_dba_refresh(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16 abNum, /* Specifies the DBA Number */
    nlm_u16 abAddr, /* Specifies the DBA Entry location within the specified abNum  */
    NlmDevWriteMode writeMode,
    NlmReasonCode*      o_reason
    )
{
#ifndef NLM_12K_11K
    NlmXptRqt* rqt_p = NULL;
    nlm_u32 address;
    NlmDevShadowDevice *shadowDev_p;
    nlm_u8  smtNum = NLM_DEFAULT_SMT_NUM;
#else
    (void)portNum;
#endif

       if(!NLM_IS_VALID_DEV_PTR(dev))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
    {
        if(o_reason)
            *o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
        return NLMERR_FAIL;
    }
    if(dev->m_devMgr_p->m_numOfAbs <= abNum)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_AB_NUM;
        return NLMERR_FAIL;
    }
    if(NLMDEV_AB_DEPTH <= abAddr)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_AB_INDEX;
        return NLMERR_FAIL;
    }

#else
    /* In case of 11k mode, call 11k specific function and return */
    return Nlm11kDevMgr__ABEntryRefresh((Nlm11kDev*)dev, 
                        (nlm_u8)abNum, abAddr, writeMode, o_reason);                
    
#endif

#ifndef NLM_12K_11K

    
    shadowDev_p = NLM_GET_SHADOW_MEM_FROM_DEV_PTR(dev);
    if(shadowDev_p == NULL)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_SHADOWDEV_PTR;
        return NLMERR_NULL_PTR;
    }
    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }


    /* preparing write request for DBA Entry. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_DBA_WRITE_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_DBA_WRITE_BITS_5_0;

    /* data length : data + mask */
    rqt_p->m_data_len = (NLMDEV_AB_WIDTH_IN_BYTES * 2);

    /* In case of Database Write; "result" field of xpt rqt will be NULL */
    rqt_p->m_result = NULL;

    /* find the single device address of the corresponding database entry */
    address = (abNum << 12) | abAddr;

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev->m_devId);

    /* Insert VBit, address type bits and Wrmode bits in the address */
    address |= (1 << NLMDEV_AB_ENTRY_VALID_BIT_IN_ADDR)
                |  (1 << NLMDEV_ADDR_TYPE_BIT_IN_PIO_WRITE)
                |  (writeMode << NLMDEV_AB_ENTRY_WR_MODE_BIT_IN_ADDR);

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
    rqt_p->m_data_len += NLMDEV_REG_ADDR_LEN_IN_BYTES;

    WriteBitsInArray(
                rqt_p->m_data,
                NLMDEV_AB_ADDR_LEN_IN_BYTES,
                (NLMDEV_AB_ADDR_LEN_IN_BYTES * 8) - 1,
                0,
                address
                );

    /* writing the DATA(X or D) stored in the shadow memory into data field of request structure: Byte[13:4]*/
    NlmCm__memcpy(rqt_p->m_data + NLMDEV_AB_ADDR_LEN_IN_BYTES,
                shadowDev_p->m_arrayBlock[abNum].m_abEntry[abAddr].m_data,
                NLMDEV_AB_WIDTH_IN_BYTES);

    /* writing the MASK(Y or M) stored in the shadow memory into data field of request structure: Byte[23:14]*/
    NlmCm__memcpy((rqt_p->m_data + NLMDEV_AB_ADDR_LEN_IN_BYTES + NLMDEV_AB_WIDTH_IN_BYTES),
                shadowDev_p->m_arrayBlock[abNum].m_abEntry[abAddr].m_mask,
                NLMDEV_AB_WIDTH_IN_BYTES);
#endif


    /* Get the SMT number from block number. */
    smtNum = ((dev->m_bankNum & (1 << (abNum/(NLMDEV_NUM_SB_PER_AC *dev->m_devMgr_p->m_numOfABsPerSB)))) == 0) ? 0 : 1;
    
    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;


    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
            return NLMERR_FAIL;
    }
    return NLMERR_OK;
#endif
}




/* NKG DO IT FROM HERE */



/*
    Function: kbp_dm_cb_reg_write writes to context buffer memory
    Upto 640 bytes of data can be written to CB memory;
    see desription of NlmDevCtxBufferInfo for more details
    User should see the reason code in case of failure.
*/
NlmErrNum_t kbp_dm_cb_write(
    NlmDevMgr               *self,
    nlm_u8              portNum,  /* Port number */
    NlmDevCtxBufferInfo     *cbInfo,                /* see the structure description above */
    NlmReasonCode*              o_reason
    )
{
#ifndef NLM_12K_11K
    NlmXptRqt* rqt_p = NULL;
    nlm_u8 smtNum = NLM_DEFAULT_SMT_NUM;
#else
    (void)portNum;
#endif


    if(!NLM_IS_VALID_DEVMGR_PTR(self))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K 
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
        || cbInfo->m_datalen > NLMDEV_MAX_CB_WRITE_IN_BYTES)
    {
        /* Datalen cannot be 0 or more than 80bytes(640b) */
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;
        return NLMERR_FAIL;
    }
    if(cbInfo->m_cbStartAddr >= NLMDEV_CB_DEPTH)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_CB_ADDRESS;
        return NLMERR_FAIL;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }   
#else

    if(self->m_11kDevMgr_p == NULL)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_FAIL;
        
    }   

    /* In case of 11k mode, call 11k specific function and return */
    return Nlm11kDevMgr__CBWrite(self->m_11kDevMgr_p,
                        (Nlm11kDevCtxBufferInfo *)cbInfo, o_reason);                
    
#endif

#ifndef NLM_12K_11K

    /* preparing write request for CB memory. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)self->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_CBWRITE_BITS_8_6;

    /* In case of CB write, valid xpt data is upto datalen specified by user; */
    rqt_p->m_data_len = cbInfo->m_datalen;

    /* In case of CB Write; "result" field of xpt rqt will be NULL */
    rqt_p->m_result = NULL;

    /* writing CB Address into m_ctx_addr field of request structure;
    Note: There will be only 1 CB Address always */
    rqt_p->m_ctx_addr = cbInfo->m_cbStartAddr;

#ifdef NLM_NO_MEMCPY_IN_XPT
    /* CB Data pointer by the data_p field of request structure */
    rqt_p->m_data_p = cbInfo->m_data;

#else
    /* copy CB Data into data field of request structure */
    NlmCm__memcpy(rqt_p->m_data, cbInfo->m_data, cbInfo->m_datalen);
#endif

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)self->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
            return NLMERR_FAIL;
    }
    return NLMERR_OK;
#endif
}


/*
    kbp_dm_cbwcmp1 is a search instruction. It searches the input key into the
    database depending on Ltr profile set number (there are 128 sets of Ltr registers)
    and gives hit/miss and address as an output.
    Compare1 instruction can be used for searches with individual search key lengths of upto 320b
    User should see the reason  code in case of failure.
*/
NlmErrNum_t kbp_dm_cbwcmp1(
    NlmDevMgr           *self,
    nlm_u8              portNum,  /* Port number */
    nlm_u8              ltrNum,     /* LTR profile set number to be used */
    NlmDevCtxBufferInfo *cbInfo,       /* Search key details */
    NlmDevCmpResult     *o_search_results,  /* Result details */
    NlmReasonCode*      o_reason
    )
{
    NlmXptRqt* rqt_p = NULL;
    nlm_u8 cmpResults[NLMDEV_MAX_RESP_LEN_IN_BYTES];
    nlm_u8 smtNum = NLM_DEFAULT_SMT_NUM;

    if(!NLM_IS_VALID_DEVMGR_PTR(self))
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
    if(o_search_results == NULL)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_SRCH_RSLT_PTR;
        return NLMERR_NULL_PTR;
    }
    if(cbInfo->m_datalen == 0
        || cbInfo->m_datalen > NLMDEV_MAX_CB_WRITE_IN_BYTES)
    {
        /* Datalen cannot be 0 or more than 80bytes(640b) */
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;
        return NLMERR_FAIL;
    }

#ifndef NLM_12K_11K

       if(ltrNum >= NLMDEV_MAX_NUM_LTRS)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_LTR_NUM;
        return NLMERR_FAIL;
    }

    if(cbInfo->m_cbStartAddr >= NLMDEV_CB_DEPTH)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_CB_ADDRESS;
        return NLMERR_FAIL;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }

#else
    /* In case of 11k mode, call 11k specific function and return */
    /* Check the LTR Number and CB address for 11K and remaining stuff is 
        same for 11K and 12K as 12K SIMXPT is passing CMP result in 12K Format only */
        if(ltrNum >= NLM11KDEV_NUM_LTR_SET)
        {
            if(o_reason)
                *o_reason = NLMRSC_INVALID_LTR_NUM;
            return NLMERR_FAIL;
        }

        if(cbInfo->m_cbStartAddr >= NLM11KDEV_CB_DEPTH)
        {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_CB_ADDRESS;
        return NLMERR_FAIL;
        }
        
#endif

    NlmCm__memset(cmpResults, 0, NLMDEV_MAX_RESP_LEN_IN_BYTES);

    /* preparing request for Compare operation. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)self->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_CBWRITE_CMP1_BITS_8_6;
    rqt_p->m_opcode[1] = ltrNum;

    /* In case of CB write and Compare, valid xpt data is upto datalen specified by user; */
    rqt_p->m_data_len = cbInfo->m_datalen;

    rqt_p->m_result = cmpResults;

    /* writing CB Address into m_ctx_addr field of request structure */
    rqt_p->m_ctx_addr = cbInfo->m_cbStartAddr;

#ifdef NLM_NO_MEMCPY_IN_XPT
    /* CB Data pointer by the data_p field of request structure */
    rqt_p->m_data_p = cbInfo->m_data;

#else
    /* copy CB Data into data field of request structure */
    NlmCm__memcpy(rqt_p->m_data, cbInfo->m_data, cbInfo->m_datalen);
#endif

    /* Get the SMT number from LTR number. */
    if(self->m_smtMode == NLMDEV_DUAL_SMT_MODE)
    {
        if(ltrNum < NLM_MAX_NUM_LTR_PER_SMT)
            smtNum = NLMDEV_SMT_0;
        else
            smtNum = NLMDEV_SMT_1;
    }

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

     /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)self->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            kbp_xpt_discard_result((NlmXpt*)self->m_xpt_p, rqt_p, NULL);
            return NLMERR_FAIL;
        }
    }

    /* calling transport layer to get the results of the current request */
    if(NULL == kbp_xpt_get_result((NlmXpt*)self->m_xpt_p, rqt_p, NULL))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
        return NLMERR_NULL_PTR;
    }

    /* Decode the result field of XPT Structure and return appropriate search results */
    NlmDevMgr__pvt_ExtractCmpResults( cmpResults, o_search_results, NlmFalse /* Non-LPM */ );

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;

    return NLMERR_OK;
}



/*
    kbp_dm_cbwcmp2 is a search instruction. It searches the input key into the
    database depending on Ltr profile set number (there are 128 sets of Ltr registers)
    and gives hit/miss and address as an output.
    Compare2 instruction can be used for searches with individual search key lengths of upto 640
    User should see the reason  code in case of failure.
*/
NlmErrNum_t kbp_dm_cbwcmp2(
    NlmDevMgr           *self,
    nlm_u8              portNum,  /* Port number */
    nlm_u8              ltrNum,   /* LTR profile set number to be used */
    NlmDevCtxBufferInfo *cbInfo,      /* Search key details */
    NlmDevCmpResult     *o_search_results,  /* Result details */
    NlmReasonCode*      o_reason
    )
{
    NlmXptRqt* rqt_p = NULL;
    nlm_u8 cmpResults[NLMDEV_MAX_RESP_LEN_IN_BYTES];
    nlm_u8 smtNum = NLM_DEFAULT_SMT_NUM;

    if(!NLM_IS_VALID_DEVMGR_PTR(self))
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
    if(o_search_results == NULL)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_SRCH_RSLT_PTR;
        return NLMERR_NULL_PTR;
    }
    if(cbInfo->m_datalen == 0
        || cbInfo->m_datalen > NLMDEV_MAX_CB_WRITE_IN_BYTES)
    {
        /* Datalen cannot be 0 or more than 80bytes(640b) */
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;
        return NLMERR_FAIL;
    }

#ifndef NLM_12K_11K

      if(ltrNum >= NLMDEV_MAX_NUM_LTRS)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_LTR_NUM;
        return NLMERR_FAIL;
    } 
    if(cbInfo->m_cbStartAddr >= NLMDEV_CB_DEPTH)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_CB_ADDRESS;
        return NLMERR_FAIL;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
#else

    /* Check the LTR Number and CB address for 11K and remaining stuff is 
    same for 11K and 12K as 12K SIMXPT is passing CMP result in 12K Format only */
    if(ltrNum >= NLM11KDEV_NUM_LTR_SET)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_LTR_NUM;
        return NLMERR_FAIL;
    }

    if(cbInfo->m_cbStartAddr >= NLM11KDEV_CB_DEPTH)
    {
    if(o_reason)
        *o_reason = NLMRSC_INVALID_CB_ADDRESS;
    return NLMERR_FAIL;
    }
    

#endif

    NlmCm__memset(cmpResults, 0, NLMDEV_MAX_RESP_LEN_IN_BYTES);
    /* preparing request for Compare operation. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)self->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_CBWRITE_CMP2_BITS_8_6;
    rqt_p->m_opcode[1] = ltrNum;

    /* In case of CB write and Compare, valid xpt data is upto datalen specified by user; */
    rqt_p->m_data_len = cbInfo->m_datalen;

    rqt_p->m_result = cmpResults;

    /* writing CB Address into m_ctx_addr field of request structure; */
    rqt_p->m_ctx_addr = cbInfo->m_cbStartAddr;

#ifdef NLM_NO_MEMCPY_IN_XPT
    /* CB Data pointer by the data_p field of request structure */
    rqt_p->m_data_p = cbInfo->m_data;

#else
    /* copy CB Data into data field of request structure */
    NlmCm__memcpy(rqt_p->m_data, cbInfo->m_data, cbInfo->m_datalen);
#endif

    /* Get the SMT number from LTR number. */
    if(self->m_smtMode == NLMDEV_DUAL_SMT_MODE)
    {
        if(ltrNum < NLM_MAX_NUM_LTR_PER_SMT)
            smtNum = NLMDEV_SMT_0;
        else
            smtNum = NLMDEV_SMT_1;
    }

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

     /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)self->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            kbp_xpt_discard_result((NlmXpt*)self->m_xpt_p, rqt_p, NULL);
            return NLMERR_FAIL;
        }
    }

    /* calling transport layer to get the results of the current request */
    if(NULL == kbp_xpt_get_result((NlmXpt*)self->m_xpt_p, rqt_p, NULL))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
        return NLMERR_NULL_PTR;
    }

    /* Decode the result field of XPT Structure and return appropriate search results */
    NlmDevMgr__pvt_ExtractCmpResults( cmpResults, o_search_results, NlmFalse /* Non-LPM */ );

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;

    return NLMERR_OK;
}



/*
    The Context Buffer Write and Compare3 instruction performs one Compare2 operation and
    two Compare1 operations in two clock cycles. The Compare3 instruction writes up 640-bits of
    data to the addressed context buffer location, which sends 640-bits of key data to the KPU
    block. In Compare3, a total of six search results are possible with an aggregate associated
    data width not to exceed 448 bits.
*/
NlmErrNum_t kbp_dm_cbwcmp3(
    NlmDevMgr           *self,
    nlm_u8              portNum,  /* Port number */
    nlm_u8              ltrNum,   /* LTR  number to be used */
    NlmDevCtxBufferInfo *cbInfo,   /* Search key details */
    NlmDevCmpResult     *o_search_results, /* Result details */
    NlmReasonCode*      o_reason
    )
{
#ifndef NLM_12K_11K
    NlmXptRqt* rqt_p = NULL;
    nlm_u8 cmpResults[NLMDEV_MAX_RESP_LEN_IN_BYTES];
    nlm_u8 smtNum = NLM_DEFAULT_SMT_NUM;
#else
    (void)portNum;
    (void)ltrNum;
    (void)cbInfo;
    (void)o_search_results;
#endif

    if(!NLM_IS_VALID_DEVMGR_PTR(self))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K
    if(NlmTrue != self->m_isLocked)
    {
        if(o_reason)
            *o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
        return NLMERR_FAIL;
    }
    if(ltrNum >= NLMDEV_MAX_NUM_LTRS)
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
            || cbInfo->m_datalen > NLMDEV_MAX_CB_WRITE_IN_BYTES)
    {
         /* Datalen cannot be 0 or more than 80bytes(640b) */
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;
        return NLMERR_FAIL;
    }
    if(cbInfo->m_cbStartAddr >= NLMDEV_CB_DEPTH)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_CB_ADDRESS;
        return NLMERR_FAIL;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }

    NlmCm__memset(cmpResults, 0, NLMDEV_MAX_RESP_LEN_IN_BYTES);
#else
    /* In case of 11k mode, call 11k specific function and return */
    if(o_reason)
        *o_reason = NLMRSC_INVALID_OPERATION;
    return NLMERR_FAIL;
    
#endif

#ifndef NLM_12K_11K

    /* preparing request for Compare operation. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)self->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_CBWRITE_CMP3_BITS_8_6;
    rqt_p->m_opcode[1] = ltrNum;

    /* In case of CB write and Compare, valid xpt data is upto datalen specified by user; */
    rqt_p->m_data_len = cbInfo->m_datalen;

    rqt_p->m_result = cmpResults;

    /* writing CB Address into m_ctx_addr field of request structure */
    rqt_p->m_ctx_addr = cbInfo->m_cbStartAddr;

#ifdef NLM_NO_MEMCPY_IN_XPT
    /* CB Data pointer by the data_p field of request structure */
    rqt_p->m_data_p = cbInfo->m_data;

#else
    /* copy CB Data into data field of request structure */
    NlmCm__memcpy(rqt_p->m_data, cbInfo->m_data, cbInfo->m_datalen);
#endif

    /* Get the SMT number from LTR number. */
    if(self->m_smtMode == NLMDEV_DUAL_SMT_MODE)
    {
        if(ltrNum < NLM_MAX_NUM_LTR_PER_SMT)
            smtNum = NLMDEV_SMT_0;
        else
            smtNum = NLMDEV_SMT_1;
    }

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

     /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)self->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            kbp_xpt_discard_result((NlmXpt*)self->m_xpt_p, rqt_p, NULL);

            return NLMERR_FAIL;
        }
    }

    /* calling transport layer to get the results of the current request */
    if(NULL == kbp_xpt_get_result((NlmXpt*)self->m_xpt_p, rqt_p, NULL))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
        return NLMERR_NULL_PTR;
    }

    /* Decode the result field of XPT Structure and return appropriate search results */
    NlmDevMgr__pvt_ExtractCmp3Results( self, ltrNum, cmpResults, o_search_results );

    if(o_reason)
         *o_reason = NLMRSC_REASON_OK;

    return NLMERR_OK;
#endif
}


/*
    kbp_dm_cbwlpm is a special search instruction. It searches the input key into the
    database depending on Ltr profile set number
    (Note:
        there are 128 sets of Ltr, always uses Key- 0 and Result port-0
        Single Bank mode:
            0-127 for BANK-0, Key-0 and result port-0

        Dual Bank:
            0-63   for BANK-0, KEY-0 and result ports-0
    )
    and gives hit/miss and address as an output.
    CompareLPM instruction can be used for searches with individual search key lengths of upto 160b
    User should see the reason  code in case of failure.
*/
NlmErrNum_t kbp_dm_cbwlpm(
    NlmDevMgr               *self,
    nlm_u8              portNum,  /* Port number */
    nlm_u8                      ltrNum,             /* LTR profile set number to be used */
    NlmDevCtxBufferInfo     *cbInfo,            /* see the structure description above */
    NlmDevCmpResult         *o_search_results,  /* see the structure description above */
    NlmReasonCode*              o_reason
    )
{
#ifndef NLM_12K_11K
    NlmXptRqt* rqt_p = NULL;
    nlm_u8 cmpResults[NLMDEV_MAX_RESP_LEN_IN_BYTES];
    nlm_u8 smtNum = NLM_DEFAULT_SMT_NUM;

#else
    (void)portNum;
    (void)ltrNum;
    (void)cbInfo;
    (void)o_search_results;
#endif

    if(!NLM_IS_VALID_DEVMGR_PTR(self))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K
    if(NlmTrue != self->m_isLocked)
    {
        if(o_reason)
            *o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
        return NLMERR_FAIL;
    }
    if(ltrNum >= NLMDEV_MAX_NUM_LTRS)
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
        || cbInfo->m_datalen > NLMDEV_MAX_CB_WRITE_IN_BYTES)
    {
        /* Datalen cannot be 0 or more than 80bytes(640b) */
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;
        return NLMERR_FAIL;
    }
    if(cbInfo->m_cbStartAddr >= NLMDEV_CB_DEPTH)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_CB_ADDRESS;
        return NLMERR_FAIL;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }

    NlmCm__memset(cmpResults, 0, NLMDEV_MAX_RESP_LEN_IN_BYTES);
#else
    /* In case of 11k mode, call 11k specific function and return */
    if(o_reason)
        *o_reason = NLMRSC_INVALID_OPERATION;
    return NLMERR_FAIL;
    
#endif

#ifndef NLM_12K_11K

    /* preparing request for Compare operation. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)self->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_CBWLPM_BITS_8_6;
    rqt_p->m_opcode[1] = ltrNum;

    /* In case of CB write and Compare, valid xpt data is upto datalen specified by user; */
    rqt_p->m_data_len = cbInfo->m_datalen;

    rqt_p->m_result = cmpResults;

    /* writing CB Address into m_ctx_addr field of request structure;
    Note: There will be only 1 CB Address always */
    rqt_p->m_ctx_addr = cbInfo->m_cbStartAddr;

#ifdef NLM_NO_MEMCPY_IN_XPT
    /* CB Data pointer by the data_p field of request structure */
    rqt_p->m_data_p = cbInfo->m_data;

#else
    /* copy CB Data into data field of request structure */
    NlmCm__memcpy(rqt_p->m_data, cbInfo->m_data, cbInfo->m_datalen);
#endif

    /* Get the SMT number from LTR number. */
    if(self->m_smtMode == NLMDEV_DUAL_SMT_MODE)
    {
        if(ltrNum < NLM_MAX_NUM_LTR_PER_SMT)
            smtNum = NLMDEV_SMT_0;
        else
            smtNum = NLMDEV_SMT_1;
    }

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

     /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)self->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            kbp_xpt_discard_result((NlmXpt*)self->m_xpt_p, rqt_p, NULL);
            return NLMERR_FAIL;
        }
    }

    /* calling transport layer to get the results of the current request */
    if(NULL == kbp_xpt_get_result((NlmXpt*)self->m_xpt_p, rqt_p, NULL))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
        return NLMERR_NULL_PTR;
    }

    /* Decode the result field of XPT Structure and return appropriate search results */
    NlmDevMgr__pvt_ExtractCmpResults( cmpResults, o_search_results,  NlmTrue /* LPM */ );

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;

    return NLMERR_OK;
#endif
}


/*
    kbp_dm_multi_compare function is a special API provided to perform compares on
    both SMTs. There is no corresponding instruction available in the device. This API
    can be used to perform searches on both SMTs simultaneously and get results
    simultaneously. Other APIs (__Compare1/__Compare2/__CompareLPM) should
    be used if there only one search on either SMT.

    This API should be used in dual bank (2-SMT) mode only. Up to 4 search results are
    returned in the 'o_search_results' output parameter. Input parameter 'cbInfo'
    should be filled with appropriate input search data.
*/
NlmErrNum_t kbp_dm_multi_compare(
    NlmDevMgr               *self,
    nlm_u8                  portNum,            /* Port number */
    NlmDevCBWriteCmpParam   *cbInfo,                /* Search data for compares  */
    NlmDevCmpResult         *o_search_results,  /* Search results */
    NlmReasonCode*           o_reason
    )
{
#ifndef NLM_12K_11K
    NlmXptRqt* rqt_p = NULL;
    NlmBool isLpm = NlmFalse;
    nlm_u8 cmpResults[NLMDEV_MAX_RESP_LEN_IN_BYTES];
    nlm_u8  opcode = 0, opcode1 = 0;
#else
    (void)portNum;
    (void)cbInfo;
    (void)o_search_results;
    (void)self;
#endif

#ifndef NLM_12K_11K
    if(!NLM_IS_VALID_DEVMGR_PTR(self))
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
    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
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

    /* CBWLPM on both SMTs is not possible */
    if( (cbInfo->m_cbInstType0 == NLMDEV_CB_INST_LPM) &&
        (cbInfo->m_cbInstType1 == NLMDEV_CB_INST_LPM) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INST_TYPE;
        return NLMERR_FAIL;
    }

    /* Instruction checking */
    if( (cbInfo->m_cbInstType0 == NLMDEV_CB_INST_NONE) &&
        (cbInfo->m_cbInstType1 == NLMDEV_CB_INST_NONE) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INST_TYPE;
        return NLMERR_FAIL;
    }

    /* Instruction checking */
    if( (cbInfo->m_cbInstType0 != NLMDEV_CB_INST_CMP1) &&
        (cbInfo->m_cbInstType0 != NLMDEV_CB_INST_CMP2) &&
        (cbInfo->m_cbInstType0 != NLMDEV_CB_INST_LPM)  )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INST_TYPE;
        return NLMERR_FAIL;
    }
    if( (cbInfo->m_cbInstType1 != NLMDEV_CB_INST_CMP1) &&
        (cbInfo->m_cbInstType1 != NLMDEV_CB_INST_CMP2) &&
        (cbInfo->m_cbInstType1 != NLMDEV_CB_INST_LPM)  )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INST_TYPE;
        return NLMERR_FAIL;
    }

    /* LTR checking */
    if( cbInfo->m_ltrNum0 >= (NLMDEV_MAX_NUM_LTRS / 2) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_LTR_NUM;

        return NLMERR_FAIL;
    }
    if( ( cbInfo->m_ltrNum1 < (NLMDEV_MAX_NUM_LTRS / 2) ) ||
        (cbInfo->m_ltrNum1 >= NLMDEV_MAX_NUM_LTRS) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_LTR_NUM;

        return NLMERR_FAIL;
    }

    /* Context Buffer address checking */
    if( (cbInfo->m_cbData0.m_datalen == 0) ||
        (cbInfo->m_cbData0.m_datalen > NLMDEV_MAX_CB_WRITE_IN_BYTES) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;
        return NLMERR_FAIL;
    }
    if( (cbInfo->m_cbData1.m_datalen == 0) ||
        (cbInfo->m_cbData1.m_datalen > NLMDEV_MAX_CB_WRITE_IN_BYTES) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;
        return NLMERR_FAIL;
    }
    if(cbInfo->m_cbData0.m_cbStartAddr >= NLMDEV_CB_DEPTH)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_CB_ADDRESS;
        return NLMERR_FAIL;
    }
    if(cbInfo->m_cbData1.m_cbStartAddr >= NLMDEV_CB_DEPTH)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_CB_ADDRESS;
        return NLMERR_FAIL;
    }

    if( cbInfo->m_cbInstType0 == NLMDEV_CB_INST_LPM)
        isLpm = NlmTrue;

#else
    /* In case of 11k mode, call 11k specific function and return */
    if(o_reason)
        *o_reason = NLMRSC_INVALID_OPERATION;
    return NLMERR_FAIL;
        
#endif

#ifndef NLM_12K_11K

    /* preparing request for Compare operation. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)self->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
        /* Clearing the rqt structure */
        NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

        rqt_p->m_2smt = NlmTrue; /* Instructions on both SMTs */

        switch(cbInfo->m_cbInstType0)
        {
            case NLMDEV_CB_INST_CMP1:
            {
                opcode = NLM_OPCODE_CBWRITE_CMP1_BITS_8_6;
                break;
            }

            case NLMDEV_CB_INST_CMP2:
            {
                opcode = NLM_OPCODE_CBWRITE_CMP2_BITS_8_6;
                break;
            }

            case NLMDEV_CB_INST_LPM:
            {
                opcode = NLM_OPCODE_CBWLPM_BITS_8_6;
                break;
            }
            case NLMDEV_CB_INST_NONE:
            {
                if(o_reason)
                   *o_reason = NLMRSC_INVALID_INPUT;
                return NLMERR_FAIL;
            }
        }

        switch(cbInfo->m_cbInstType1)
        {
            case NLMDEV_CB_INST_CMP1:
            {
                opcode1 = NLM_OPCODE_CBWRITE_CMP1_BITS_8_6;
                break;
            }

            case NLMDEV_CB_INST_CMP2:
            {
                opcode1 = NLM_OPCODE_CBWRITE_CMP2_BITS_8_6;
                break;
            }

            case NLMDEV_CB_INST_LPM:
            {
                opcode1 = NLM_OPCODE_CBWLPM_BITS_8_6;
                break;
            }
            case NLMDEV_CB_INST_NONE:
            {
                if(o_reason)
                   *o_reason = NLMRSC_INVALID_INPUT;
                return NLMERR_FAIL;
            }
        }

    /* assigning request structure members for SMT-0 followed by SMT-1 */
    rqt_p->m_opcode[0] = opcode;
    rqt_p->m_opcode[1] = cbInfo->m_ltrNum0;
    rqt_p->m_data_len = cbInfo->m_cbData0.m_datalen;
    rqt_p->m_ctx_addr = cbInfo->m_cbData0.m_cbStartAddr;

    rqt_p->m_opcode1[0] = opcode1;
    rqt_p->m_opcode1[1] = cbInfo->m_ltrNum1;
    rqt_p->m_data_len1 = cbInfo->m_cbData1.m_datalen;
    rqt_p->m_ctx_addr1 = cbInfo->m_cbData1.m_cbStartAddr;

#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_data_p   = cbInfo->m_cbData0.m_data;
    rqt_p->m_data1_p = cbInfo->m_cbData1.m_data;
#else
    NlmCm__memcpy(rqt_p->m_data, cbInfo->m_cbData0.m_data, cbInfo->m_cbData0.m_datalen);
    NlmCm__memcpy(rqt_p->m_data1, cbInfo->m_cbData1.m_data, cbInfo->m_cbData1.m_datalen);
#endif

    rqt_p->m_result = cmpResults;

    rqt_p->m_port_num = portNum;

     /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)self->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            kbp_xpt_discard_result((NlmXpt*)self->m_xpt_p, rqt_p, NULL);
            return NLMERR_FAIL;
        }
    }

    /* calling transport layer to get the results of the current request */
    if(NULL == kbp_xpt_get_result((NlmXpt*)self->m_xpt_p, rqt_p, NULL))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
        return NLMERR_NULL_PTR;
    }

    /* Decode the result field of XPT Structure and return appropriate search results */
    NlmDevMgr__pvt_ExtractCmpResults( cmpResults, o_search_results, isLpm);

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;

    return NLMERR_OK;
#endif
}


/*
Function: kbp_dm_generic_reg_write
Description: Writes 80b data to specified address register (only for Global[R/W]/LTR).
    dev     : Divice pointer:
    address : Address of the register
    data    : 80b data to write

User should see the reason code in case of failure.
*/
 NlmErrNum_t kbp_dm_generic_reg_write(
    void                        *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u32                     address,
    void                        *data,
    NlmReasonCode*              o_reason
    )
{
    NlmXptRqt* rqt_p = NULL;
    nlm_u8 *data_ptr = NULL;
    NlmDev *dev_p;
    nlm_u8 smtNum = NLM_DEFAULT_SMT_NUM;

    if(NULL == dev)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }

    dev_p = (NlmDev*)dev; /* check the device pointer */
    if(!NLM_IS_VALID_DEV_PTR(dev_p))
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
    if(NULL == data)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DATA;
        return NLMERR_NULL_PTR;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }

    /* preparing request for register write. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev_p->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

#ifdef NLM_12K_11K

    /* In case of 11k mode, call 11k specific function and return */
    if(dev_p->m_devMgr_p->m_11kDevMgr_p == NULL)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_NULL_PTR;         
    }   
    /* No change required in 11K mode for generic register write */
#endif

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_REG_WRITE_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_REG_WRITE_BITS_5_0;

    /* length of the register data */
    rqt_p->m_data_len = NLMDEV_REG_LEN_IN_BYTES;

#ifdef NLM_NO_MEMCPY_IN_XPT
    data_ptr = data;
#else
    /* In case of Reg write, Byte[3:0] of the "data" field of xpt rqt
     will contain the address and Byte[13:4] will contain the 80b register data */
    rqt_p->m_data_len += NLMDEV_REG_ADDR_LEN_IN_BYTES;

    /* pointer to the register data : Byte[13:4]*/
    data_ptr = rqt_p->m_data + NLMDEV_REG_ADDR_LEN_IN_BYTES;
    NlmCm__memcpy(data_ptr, data, NLMDEV_REG_LEN_IN_BYTES);
#endif

    /* In case of Reg Write; "result" field of xpt rqt will be NULL */
    rqt_p->m_result = NULL;

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev_p->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_address = address;
    rqt_p->m_data_p = data_ptr;
#else
    /* Byte[3:0] of the "data" field of xpt rqt will contain the address*/
    WriteBitsInArray(
                rqt_p->m_data,
                NLMDEV_REG_ADDR_LEN_IN_BYTES,
                (NLMDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
                0,
                address
                );
#endif

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev_p->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
            return NLMERR_FAIL;
    }

    return NLMERR_OK;
}



/*
Function: kbp_dm_generic_reg_read
Description: Read 80b data from specified address register (only for Global/LTR).
    dev     : Divice pointer:
    address : Address of the register
    o_data  : 80b data read

User should see the reason code in case of failure.
*/
NlmErrNum_t kbp_dm_generic_reg_read(
    void                        *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u32                     address,
    void                        *o_data,
    NlmReasonCode*              o_reason
    )
{
    NlmXptRqt* rqt_p = NULL;
    NlmDev *dev_p;
    nlm_u8 readData[NLMDEV_REG_LEN_IN_BYTES + 1];/* Extra byte is for control bits such as VBIT, parity */
    nlm_u8 smtNum = NLM_DEFAULT_SMT_NUM;

    if(NULL == dev)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }

    dev_p = (NlmDev*)dev; /* check the device pointer */
    if(!NLM_IS_VALID_DEV_PTR(dev_p))
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

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }

    /* preparing request for register read. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev_p->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

#ifdef NLM_12K_11K  
        /* In case of 11k mode, call 11k specific function and return */
        if(dev_p->m_devMgr_p->m_11kDevMgr_p == NULL)
        {
            if(o_reason)
                *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
            return NLMERR_NULL_PTR;         
        }   
        /* No change required in 11K mode for generic register write */
#endif


    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_REG_READ_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_REG_READ_BITS_5_0;

    rqt_p->m_data_len = NLMDEV_REG_ADDR_LEN_IN_BYTES;

    /* In case of Reg read; XPT layer send the 80b read data via "result" field of xpt rqt;
    In addition to this result will contain some control bits such as vbit and parity which are ignored */
    rqt_p->m_result = readData;

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev_p->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_address = address;
#else
    /* In case of Reg Read, Byte[3:0] of the "data" field of xpt rqt will contain the address*/
    WriteBitsInArray(
                rqt_p->m_data,
                NLMDEV_REG_ADDR_LEN_IN_BYTES,
                (NLMDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
                0,
                address
                );
#endif

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev_p->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            kbp_xpt_discard_result((NlmXpt*)dev_p->m_devMgr_p->m_xpt_p, rqt_p, NULL);
            return NLMERR_FAIL;
        }
    }

    /* calling transport layer to get the results of the current request */
    if(NULL == kbp_xpt_get_result((NlmXpt*)dev_p->m_devMgr_p->m_xpt_p, rqt_p, NULL))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
        return NLMERR_NULL_PTR;
    }

    /* extracting register contents from result field of request structure;
       Note: Byte[0] contains some control fields which can be ignored */
    NlmCm__memcpy(o_data, &readData[1], NLMDEV_REG_LEN_IN_BYTES);

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;
    return NLMERR_OK;
}


/*
    kbp_dm_uda_write writes single UDA Database (Associated Data) entry of
    width 32b (only LSB 32b are valid) specified addr.
    User should see the reason code in case of failure.
*/

NlmErrNum_t kbp_dm_uda_write(
    void                *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u32             address,
    void                *data,
    nlm_u8              length,
    NlmReasonCode*      o_reason
    )
{
    NlmDev *dev_p; 
    NlmXptRqt* rqt_p = NULL;
    nlm_u8 *data_ptr = NULL;    
    nlm_u8 smtNum = NLM_DEFAULT_SMT_NUM;

    if(NULL == dev)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }

    if(length > NLMDEV_MAX_UDA_DATA_LEN_12K_11K_IN_BYTES)
    {
        if(o_reason)
            *o_reason =NLMRSC_INVALID_DATA_LENGTH;
        return NLMERR_FAIL;
    }
    dev_p = (NlmDev*)dev; /* check the device pointer */
    if(!NLM_IS_VALID_DEV_PTR(dev_p))
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
    if(NULL == data)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DATA;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K 
    if(address >= NLMDEV_TOTAL_NUM_SRAM_ENTRIES)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_ADDRESS;
        return NLMERR_NULL_PTR;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
    
    /* length of the register data */
    length = NLMDEV_REG_LEN_IN_BYTES;
#endif

    /* preparing request for register write. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev_p->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

#ifndef NLM_12K_11K
    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_UDA_WRITE_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_UDA_WRITE_BITS_5_0;
#else
    rqt_p->m_opcode[0] = NLM_OPCODE_PVT_UDA_WRITE;
#endif

    /* length of the register data */
    rqt_p->m_data_len = length;
   
#ifdef NLM_NO_MEMCPY_IN_XPT
    data_ptr = data;
#else
    /* In case of Reg write, Byte[3:0] of the "data" field of xpt rqt
     will contain the address and Byte[13:4] will contain the 80b register data */
    rqt_p->m_data_len += NLMDEV_REG_ADDR_LEN_IN_BYTES;

#ifndef NLM_12K_11K
    /* pointer to the register data : Byte[13:4]*/
    data_ptr = rqt_p->m_data + NLMDEV_REG_ADDR_LEN_IN_BYTES;
#else
    data_ptr = rqt_p->m_data + NLMDEV_REG_ADDR_LEN_IN_BYTES + 2;
#endif
    NlmCm__memcpy(data_ptr, data, length);
#endif

    /* In case of Reg Write; "result" field of xpt rqt will be NULL */
    rqt_p->m_result = NULL;

#ifndef NLM_12K_11K
    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev_p->m_devId);

    /* Address type is 'b10 for UDA writes */
    address |= (0x2 << NLMDEV_ADDR_TYPE_BIT_IN_PIO_WRITE);
#endif

    /* Request data format is
    * 4 Bytes -> address
    * 1 Byte -> Device Id
    * 1 Byte -> Data Length
    * Remainig Bytes -> data */

#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_address = address;
    rqt_p->m_data_p = data_ptr;
#else
    /* Byte[3:0] of the "data" field of xpt rqt will contain the address*/
    WriteBitsInArray(
                rqt_p->m_data,
                NLMDEV_REG_ADDR_LEN_IN_BYTES,
                (NLMDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
                0,
                address
                );

#endif

#ifdef NLM_12K_11K

    rqt_p->m_data_len += 2;/* 1Byte for device ID & 1Byte for Length*/
        WriteBitsInArray(
                rqt_p->m_data,
                rqt_p->m_data_len,
                (rqt_p->m_data_len*8)-33,
                (rqt_p->m_data_len*8)-40,
                (nlm_u8)dev_p->m_devId);

        WriteBitsInArray(
                rqt_p->m_data,
                rqt_p->m_data_len,
                (rqt_p->m_data_len*8)-41,
                (rqt_p->m_data_len*8)-48,
                length);
#endif

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev_p->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
            return NLMERR_FAIL;
    }

    return NLMERR_OK;
}

/*
    kbp_dm_uda_read reads single UDA Database (Associated Data) entry of
    width 32b (only LSB 32b are valid) from specified addr.
    User should see the reason code in case of failure.
*/

NlmErrNum_t kbp_dm_uda_read(
    void                *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u32             address,
    void                *o_data,
    nlm_u8              length,
    NlmReasonCode*      o_reason
    )
{
    NlmDev *dev_p;
    NlmXptRqt* rqt_p = NULL;
    nlm_u8 smtNum = NLM_DEFAULT_SMT_NUM;

#ifndef NLM_12K_11K
    nlm_u8 readData[NLMDEV_REG_LEN_IN_BYTES + 1];/* Extra byte is for control bits such as VBIT, parity */
#else
    nlm_u8 readData[MAX_DATA_LEN_BYTES];
#endif

#ifndef NLM_12K_11K 
    (void)length;
#endif

    if(NULL == dev)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }

    dev_p = (NlmDev*)dev; /* check the device pointer */
    if(!NLM_IS_VALID_DEV_PTR(dev_p))
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
            *o_reason = NLMRSC_INVALID_DATA;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K    
    if(address >= NLMDEV_TOTAL_NUM_SRAM_ENTRIES)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_ADDRESS;
        return NLMERR_NULL_PTR;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
#endif

    /* preparing request for register write. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev_p->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

#ifndef NLM_12K_11K
    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_UDA_READ_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_UDA_READ_BITS_5_0;
#else
    rqt_p->m_opcode[0] = NLM_OPCODE_PVT_UDA_READ;
#endif

    /* length of the address */
    rqt_p->m_data_len = NLMDEV_REG_ADDR_LEN_IN_BYTES;

    /* In case of Database read; XPT layer send the 80b read data via "result" field of xpt rqt;
    In addition to this result will contain some control bits such as vbit and parity  */
    rqt_p->m_result = readData;

#ifndef NLM_12K_11K
    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev_p->m_devId);

    /* Address type is 'b10 for UDA writes */
    address |= (0x2 << NLMDEV_ADDR_TYPE_BIT_IN_PIO_WRITE);
#endif

#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_address = address;
#else
    /* Byte[3:0] of the "data" field of xpt rqt will contain the address*/
    WriteBitsInArray(
                rqt_p->m_data,
                NLMDEV_REG_ADDR_LEN_IN_BYTES,
                (NLMDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
                0,
                address
                );
#endif

#ifdef NLM_12K_11K

    /* Request Data Formate
    * 4 Bytes -> Address
    * 1 Byte -> Device Id
    * 1 Byte -> Length to read
    */

    rqt_p->m_data_len +=2;/* 1Byte for Device Id & 1Byte for Length */

/* insert Device ID */
    WriteBitsInArray(
                rqt_p->m_data,
                rqt_p->m_data_len,
                15,
                8,
                (nlm_u8)dev_p->m_devId);

/* Insert Length of Actual Data */
    WriteBitsInArray(
                rqt_p->m_data,
                rqt_p->m_data_len,
                7,
                0,
                length);
#endif
    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev_p->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
            return NLMERR_FAIL;
    }

    /* calling transport layer to get the results of the current request */
    if(NULL == kbp_xpt_get_result((NlmXpt*)dev_p->m_devMgr_p->m_xpt_p, rqt_p, NULL))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K
    /* Copy the Data X read from the device */
    NlmCm__memcpy(o_data, readData + 1, NLMDEV_AB_WIDTH_IN_BYTES);
#else
     NlmCm__memcpy(o_data, readData, length);
#endif

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;

    return NLMERR_OK;
}




NlmErrNum_t kbp_dm_cmd_send(
    void                         *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u8                       bank_num,
    nlm_u8                       opcode_high,
    nlm_u8                       opcode_low,
    void                         *data,
    nlm_u8                       data_len,
    NlmReasonCode*               o_reason
    )
{   
    NlmDev *dev_p;

#ifndef NLM_12K_11K
    NlmXptRqt* rqt_p = NULL;
    nlm_u8 smtNum = NLM_DEFAULT_SMT_NUM;
#else
    (void)portNum;
    (void)bank_num;
    (void)opcode_high;
    (void)opcode_low;
    (void)data;
    (void)data_len;
#endif

    if(NULL == dev)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }

    dev_p = (NlmDev*)dev; /* check the device pointer */
    if(!NLM_IS_VALID_DEV_PTR(dev_p))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K
    if(NlmTrue != dev_p->m_devMgr_p->m_isLocked)
    {
        if(o_reason)
            *o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
        return NLMERR_FAIL;
    }
    if( (bank_num != NLMDEV_BANK_0) && (bank_num != NLMDEV_BANK_1) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_BANK;
        return NLMERR_NULL_PTR;
    }
    if ((NLM_OPCODE_LPMTYPE2_BITS_8_6 != opcode_high) &&
        (NLM_OPCODE_LPMTYPE3_BITS_8_6 != opcode_high) &&
        (NLM_OPCODE_LPMTYPE1_BITS_8_6 != opcode_high))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;
        return NLMERR_NULL_PTR;
    }
    if ((NLM_OPCODE_LPMTYPE2_BITS_5_0 != opcode_low) &&
        (NLM_OPCODE_LPMTYPE3_BITS_5_0 != opcode_low) &&
        (NLM_OPCODE_LPMTYPE1_BITS_5_0 != opcode_low))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_INPUT;
        return NLMERR_NULL_PTR;
    }
    if(NULL == data)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DATA;
        return NLMERR_NULL_PTR;
    }
    if( (data_len == 0) || (data_len > NLM_MAX_DATA_WORDS * NLM_DATA_WORD_LEN_IN_BYTES))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DATA_LENGTH;
        return NLMERR_NULL_PTR;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
#else
    /* In case of 11k mode, call 11k specific function and return */

    if(o_reason)
        *o_reason = NLMRSC_INVALID_OPERATION;
    return NLMERR_FAIL;

#endif

#ifndef NLM_12K_11K  

    /* preparing request for register write. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev_p->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;

        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = opcode_high;
    rqt_p->m_opcode[1] = opcode_low;

    rqt_p->m_data_len = data_len;

#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_data_p = data;
#else
    NlmCm__memcpy(rqt_p->m_data, data, data_len);
#endif

    /* In case of Reg Write; "result" field of xpt rqt will be NULL */
    rqt_p->m_result = NULL;

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev_p->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
            return NLMERR_FAIL;
    }

    return NLMERR_OK;
#endif
}



/*
    kbp_dm_em_write API can be used to add Exact Match entry at the specified
    address within a DBA block. Write mode (whether WRA or WRB) is specified by
    the input parameter 'writeMode'.
*/
NlmErrNum_t kbp_dm_em_write(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16         abNum,      /* Specifies the DBA Block Number */
    nlm_u16         abAddr,     /* Specifies the entry location within the specified abNum  */
    NlmDevEMEntry   *emEntry,
    NlmEMWriteMode  writeMode,
    NlmReasonCode*  o_reason
    )
{
#ifndef NLM_12K_11K  
    NlmXptRqt* rqt_p = NULL;
    nlm_u32 address;
    nlm_u8  smtNum = NLM_DEFAULT_SMT_NUM;
#else
    (void)portNum;
    (void)abNum;
    (void)abAddr;
    (void)writeMode;
    (void)emEntry;
#endif
    if(!NLM_IS_VALID_DEV_PTR(dev))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
    {
        if(o_reason)
            *o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
        return NLMERR_FAIL;
    }
    if(NULL == emEntry)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DATA;
        return NLMERR_NULL_PTR;
    }
    if(dev->m_devMgr_p->m_numOfAbs <= abNum)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_AB_NUM;
        return NLMERR_FAIL;
    }
    if(NLMDEV_AB_DEPTH <= abAddr)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_AB_INDEX;
        return NLMERR_FAIL;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
    if(writeMode != NLM_EM_WRITE_MODE_A &&
        writeMode != NLM_EM_WRITE_MODE_B)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DATA;
        return NLMERR_NULL_PTR;
    }
#else
    /* In case of 11k mode, call 11k specific function and return */

    if(o_reason)
        *o_reason = NLMRSC_INVALID_OPERATION;
    return NLMERR_FAIL;

#endif

#ifndef NLM_12K_11K  

    /* preparing write request for DBA Entry. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }

    /* Clearing the rqt structure */
    /*NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));*/

    /* assigning request structure members */
    if(writeMode == NLM_EM_WRITE_MODE_A)
    {
        rqt_p->m_opcode[0] = NLM_OPCODE_EM_WRA_BITS_8_6;
        rqt_p->m_opcode[1] = NLM_OPCODE_EM_WRA_BITS_5_0;
    }
    else
    {
        rqt_p->m_opcode[0] = NLM_OPCODE_EM_WRB_BITS_8_6;
        rqt_p->m_opcode[1] = NLM_OPCODE_EM_WRB_BITS_5_0;
    }

    rqt_p->m_data_len = NLM_EM_HW_INPUT_LEN_IN_BYTES;
    rqt_p->m_result = NULL;

    /* find the single device address of the corresponding EM entry */
    address = (abNum << 12) | abAddr;

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev->m_devId);

    /* Insert VBit, address type bits in the address */
    address |= (emEntry->m_vbit << NLMDEV_EM_ENTRY_VALID_BIT_IN_ADDR)
                    |  (1 << NLMDEV_ADDR_TYPE_BIT_IN_EM_WRITE);

#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_address = address;

    rqt_p->m_data_p = emEntry->m_data;

#else
    /* In case of EM write, Byte[3:0] of the "data" field of xpt rqt
         will contain the address(along with vbit) and Byte[12:4] will contain the
         72b data. User input is 64b. 72b are expected on ILA DW. MS Byte is
         written as 0.
      */
    rqt_p->m_data_len += NLMDEV_EM_ADDR_LEN_IN_BYTES;

    WriteBitsInArray(
                    rqt_p->m_data,
                    NLMDEV_EM_ADDR_LEN_IN_BYTES,
                    (NLMDEV_EM_ADDR_LEN_IN_BYTES * 8) - 1,
                    0,
                    address
                    );

    rqt_p->m_data[NLMDEV_EM_ADDR_LEN_IN_BYTES] = 0;
    rqt_p->m_data[NLMDEV_EM_ADDR_LEN_IN_BYTES+1] = 0;

    /* writing the DATA into data field of request structure */
    NlmCm__memcpy(rqt_p->m_data + NLMDEV_EM_ADDR_LEN_IN_BYTES + 2,
                    emEntry->m_data, NLM_EM_RECORD_WIDTH_IN_BYTES);

#endif

    /* Get the SMT number from block number. */
    smtNum = ((dev->m_bankNum & (1 << (abNum/(NLMDEV_NUM_SB_PER_AC *dev->m_devMgr_p->m_numOfABsPerSB)))) == 0) ? 0 : 1;

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
            return NLMERR_FAIL;
    }

    return NLMERR_OK;
#endif
}


/*  kbp_dm_em_read API can be used to read Exact Match entry at the specified
    address within a DBA block. Read mode (whether RDA or RDB) is specified by
    the input parameter 'readMode'.  Output is available in the output parameter
    'o_emEntry'
*/
NlmErrNum_t kbp_dm_em_read(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16         abNum,    /* Specifies the DBA Block Number */
    nlm_u16         abAddr,  /* Specifies the entry location within the specified abNum  */
    NlmEMReadMode   readMode,  /* Read-A or Read-B */
    NlmDevEMEntry   *o_emEntry,
    NlmReasonCode*  o_reason
    )
{
#ifndef NLM_12K_11K  
    NlmXptRqt* rqt_p = NULL;
    nlm_u32     address;
    nlm_u8  smtNum = NLM_DEFAULT_SMT_NUM;
    nlm_u8  readData[NLM_EM_HW_OUTPUT_LEN_IN_BYTES];
#else
    (void)portNum;
    (void)abNum;
    (void)abAddr;
    (void)readMode;
    (void)o_emEntry;
#endif

    if(!NLM_IS_VALID_DEV_PTR(dev))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEV_PTR;
        return NLMERR_NULL_PTR;
    }
#ifndef NLM_12K_11K
    if(NlmTrue != dev->m_devMgr_p->m_isLocked)
    {
        if(o_reason)
            *o_reason = NLMRSC_DEV_MGR_CONFIG_NOT_LOCKED;
        return NLMERR_FAIL;
    }
    if(NULL == o_emEntry)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_OUTPUT;
        return NLMERR_NULL_PTR;
    }
    if(dev->m_devMgr_p->m_numOfAbs <= abNum)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_AB_NUM;
        return NLMERR_FAIL;
    }
    if(NLMDEV_AB_DEPTH <= abAddr)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_AB_INDEX;
        return NLMERR_FAIL;
    }

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }

#else
       /* In case of 11k mode, call 11k specific function and return */
    if(o_reason)
        *o_reason = NLMRSC_INVALID_OPERATION;
    return NLMERR_FAIL;

#endif

#ifndef NLM_12K_11K  

    /* preparing request for EM Entry Read. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }

    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    if(readMode == NLM_EM_READ_MODE_A)
    {
        rqt_p->m_opcode[0] = NLM_OPCODE_EM_RDA_BITS_8_6;
        rqt_p->m_opcode[1] = NLM_OPCODE_EM_RDA_BITS_5_0;
    }
    else
    {
        rqt_p->m_opcode[0] = NLM_OPCODE_EM_RDB_BITS_8_6;
        rqt_p->m_opcode[1] = NLM_OPCODE_EM_RDB_BITS_5_0;
    }

    rqt_p->m_data_len = NLM_EM_HW_OUTPUT_LEN_IN_BYTES;

    /* find the single device address of the corresponding EM entry */
    address = (abNum << 12) | abAddr;

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev->m_devId);

    /* Insert address type bit in the address */
    address |= (1 << NLMDEV_ADDR_TYPE_BIT_IN_EM_WRITE);

    /* In case of EM read, XPT layer sends 80b read data via "result" field of xpt rqt;
        In addition to this result will contain some control bits such as vbit and parity
    */
    rqt_p->m_result = readData;

#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_address = address;
#else
    /* In case of Read, Byte[3:0] of the "data" field of xpt rqt
       will contain the address*/
    WriteBitsInArray(
                rqt_p->m_data,
                NLMDEV_EM_ADDR_LEN_IN_BYTES,
                (NLMDEV_EM_ADDR_LEN_IN_BYTES * 8) - 1,
                0,
                address
                );
#endif

    /* Get the SMT number from block number. */
    smtNum = ((dev->m_bankNum & (1 << (abNum/(NLMDEV_NUM_SB_PER_AC *dev->m_devMgr_p->m_numOfABsPerSB)))) == 0) ? 0 : 1;

    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
        {
                    /* If Service Request has failed then discard the result so that xpt rqt is freed*/
                    kbp_xpt_discard_result((NlmXpt*)dev->m_devMgr_p->m_xpt_p, rqt_p, NULL);

            return NLMERR_FAIL;
            }
    }

    /* calling transport layer to get the results of the current request */
    if(NULL == kbp_xpt_get_result((NlmXpt*)dev->m_devMgr_p->m_xpt_p, rqt_p, NULL))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;

        return NLMERR_NULL_PTR;
    }

    /* Get the Vbit which will be Bit 4 of Byte 0 of read Data */
    o_emEntry->m_vbit = (readData[0] >> 4) & 1;

    /* Only LS 8 bytes are relevant here, hence + 3 */
    NlmCm__memcpy(o_emEntry->m_data, readData + 3, NLM_EM_RECORD_WIDTH_IN_BYTES);

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;

    return NLMERR_OK;
#endif
}

/*
    Range related register write.
    This API is for internal use.
*/
NlmErrNum_t kbp_dm_range_reg_write(
    NlmDev              *dev,
    nlm_u8              portNum,
    NlmSMTNum           smtNum,
    nlm_u32             address,
    NlmDevRangeReg      *rangeRegData,
    NlmReasonCode       *o_reason
    )
{
#ifndef NLM_12K_11K
    NlmXptRqt* rqt_p = NULL;
    nlm_u8 regData[NLMDEV_REG_LEN_IN_BYTES] = "";
    nlm_u8 *data_ptr = NULL;
    

    nlm_u32 rangeStAddr  = NLM_REG_RANGE_A_BOUNDS(0);
    nlm_u32 rangeEndAddr = NLM_REG_RANGE_D_CODE1;
#else
    (void) portNum;
    (void) smtNum;
#endif

    if(!NLM_IS_VALID_DEV_PTR(dev))
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
#ifndef NLM_12K_11K
    
    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
    if(smtNum != NLMDEV_SMT_0 &&
       smtNum != NLMDEV_SMT_1)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_SMT_NUM;
        return NLMERR_FAIL;
    }
    if(NULL == rangeRegData)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DATA;
        return NLMERR_NULL_PTR;
    }
#else

    /* In case of 11k mode, call 11k specific function and return */
    if(dev->m_devMgr_p->m_11kDevMgr_p == NULL)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_FAIL;
        
    }   
/* As of now Range module is same in 12K and 11K so we can use 12K code also 
    instead of calling 11K API but we are calling 11K API to take care of any future 
    changes in 12K*/
    /* Since range registers are having same addresses in 12K and 11K, there is no 
    conversion required for any 12K range address to 11K range address. If addresses
    are changes in future then take care of converting it properly. */
    return Nlm11kDevMgr__RangeRegisterWrite((Nlm11kDev*)dev, 
                        address, (Nlm11kDevRangeReg *)rangeRegData, o_reason);
#endif

#ifndef NLM_12K_11K  

    
    if(address < rangeStAddr || address > rangeEndAddr)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_REG_ADDRESS;
        return NLMERR_FAIL;
    }

    /* preparing request for register write. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_REG_WRITE_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_REG_WRITE_BITS_5_0;

    /* length of the register data */
    rqt_p->m_data_len = NLMDEV_REG_LEN_IN_BYTES;

    /* In case of Reg Write; "result" field of xpt rqt will be NULL */
    rqt_p->m_result = NULL;

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_address = address;

    /* request pointer will point to the range data to be written */
    data_ptr = rqt_p->m_data_p = regData;

#else
    (void)regData;
    /* In case of Reg write, Byte[3:0] of the "data" field of xpt rqt
    will contain the address and Byte[13:4] will contain the 80b register data */
    rqt_p->m_data_len += NLMDEV_REG_ADDR_LEN_IN_BYTES;

    /* Byte[3:0] of the "data" field of xpt rqt will contain the address*/
    WriteBitsInArray(
                rqt_p->m_data,
                NLMDEV_REG_ADDR_LEN_IN_BYTES,
                (NLMDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
                0,
                address
                );

    /* pointer to the register data : Byte[13:4]*/
    data_ptr = rqt_p->m_data + NLMDEV_REG_ADDR_LEN_IN_BYTES;

#endif

    /* write the relevant fields in the range register */
    if(address >= NLM_REG_RANGE_A_BOUNDS(0) && address < NLM_REG_RANGE_A_CODE0) /* range bound regs */
    {
        NlmDevMgr_pvt_ConstructRangeBoundRegData(data_ptr, rangeRegData->m_data);
    }
    else /* range code regs : addrs >= NLM_REG_RANGE_A_CODE0 && addrs <= NLM_REG_RANGE_D_CODE1 */
    {
        NlmDevMgr_pvt_ConstructRangeCodeRegData(data_ptr, rangeRegData->m_data);
    }

    /* assign PORT/SMT numbers */
    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = (nlm_u8)smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
            return NLMERR_FAIL;
    }

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;
    return NLMERR_OK;
#endif
}

/*
    Range related register read.
    This API is for internal use.
*/
NlmErrNum_t kbp_dm_range_reg_read(
    NlmDev              *dev,
    nlm_u8              portNum,
    NlmSMTNum           smtNum,
    nlm_u32             address,
    NlmDevRangeReg      *o_rangeRegData,
    NlmReasonCode       *o_reason
    )
{
#ifndef NLM_12K_11K
    NlmXptRqt* rqt_p = NULL;
    nlm_u8 readData[NLMDEV_REG_LEN_IN_BYTES + 1];/* Extra byte is for control bits
                                                     such as VBIT, parity */
    
    nlm_u32 rangeStAddr  = NLM_REG_RANGE_A_BOUNDS(0);
    nlm_u32 rangeEndAddr = NLM_REG_RANGE_D_CODE1;
#else
    (void) portNum;
    (void) smtNum;
#endif
    if(!NLM_IS_VALID_DEV_PTR(dev))
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
#ifndef NLM_12K_11K
    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
    if(smtNum != NLMDEV_SMT_0 &&
       smtNum != NLMDEV_SMT_1)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_SMT_NUM;
        return NLMERR_FAIL;
    }
    if(NULL == o_rangeRegData)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_OUTPUT;
        return NLMERR_NULL_PTR;
    }
    if(address < rangeStAddr || address > rangeEndAddr)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_REG_ADDRESS;
        return NLMERR_FAIL;
    }
#else

    /* In case of 11k mode, call 11k specific function and return */
    if(dev->m_devMgr_p->m_11kDevMgr_p == NULL)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_DEVMGR_PTR;
        return NLMERR_FAIL;
        
    }   

    /* As of now Range module is same in 12K and 11K so we can use 12K code also 
    instead of calling 11K API but we are calling 11K API to take care of any future 
    changes in 12K*/
    /* Since range registers are having same addresses in 12K and 11K, there is no 
    conversion required for any 12K range address to 11K range address. If addresses
    are changes in future then take care of converting it properly. */
    return Nlm11kDevMgr__RangeRegisterRead((Nlm11kDev*)dev,
                            address, (Nlm11kDevRangeReg *)o_rangeRegData, o_reason);
    
#endif

#ifndef NLM_12K_11K

    /* preparing request for register read. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_REG_READ_BITS_8_6;
    rqt_p->m_opcode[1] = NLM_OPCODE_REG_READ_BITS_5_0;

    rqt_p->m_data_len = NLMDEV_REG_ADDR_LEN_IN_BYTES;

    /* In case of Reg read; XPT layer send the 80b read data via "result" field of xpt rqt;
    In addition to this result will contain some control bits such as vbit and parity which are ignored */
    rqt_p->m_result = readData;

    /* Insert the devid into the address */
    address = NLM_DEVMGR_INSERT_DEVID(address, dev->m_devId);

#ifdef NLM_NO_MEMCPY_IN_XPT
    rqt_p->m_address = address;
#else
    /* In case of Reg Read, Byte[3:0] of the "data" field of xpt rqt will contain the address*/
    WriteBitsInArray(
                rqt_p->m_data,
                NLMDEV_REG_ADDR_LEN_IN_BYTES,
                (NLMDEV_REG_ADDR_LEN_IN_BYTES * 8) - 1,
                0,
                address
                );
#endif

    /* assign PORT/SMT numbers */
    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num  = (nlm_u8)smtNum;

    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
        {
            /* If Service Request has failed then discard the result so that xpt rqt is freed*/
            kbp_xpt_discard_result((NlmXpt*)dev->m_devMgr_p->m_xpt_p, rqt_p, NULL);
            return NLMERR_FAIL;
        }
    }

    /* calling transport layer to get the results of the current request */
    if(NULL == kbp_xpt_get_result((NlmXpt*)dev->m_devMgr_p->m_xpt_p, rqt_p, NULL))
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RSLT_PTR;
        return NLMERR_NULL_PTR;
    }

    /* extracting register contents from result field of request structure;
    Note Byte[0] contains some control fields which can be ignored */
    NlmCm__memcpy(o_rangeRegData->m_data, readData + 1, NLMDEV_REG_LEN_IN_BYTES);
    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;
    return NLMERR_OK;
#endif
}

void kbp_dm_pvt_get_shift_direction_count(
    NlmDevBlockWidth dbaBlkWidth,
    NlmDevADLength   adLen,
    NlmDevShiftDir   *shiftDir,
    NlmDevShiftCount *shiftCount
    )
{
    switch( dbaBlkWidth )
    {
        case NLMDEV_BLK_WIDTH_80:
        {
            if( adLen == NLMDEV_ADLEN_32B ) /* default case, no shift required */
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_0;
            }
            else if( adLen == NLMDEV_ADLEN_64B )
            {
                *shiftDir   = NLMDEV_SHIFT_LEFT;
                *shiftCount = NLMDEV_SHIFT_CNT_1;
            }
            else if( adLen == NLMDEV_ADLEN_128B )
            {
                *shiftDir   = NLMDEV_SHIFT_LEFT;
                *shiftCount = NLMDEV_SHIFT_CNT_2;
            }
            else
            {
                *shiftDir   = NLMDEV_SHIFT_LEFT;
                *shiftCount = NLMDEV_SHIFT_CNT_3;
            }

            break;
        }

        case NLMDEV_BLK_WIDTH_160:
        {
            if( adLen == NLMDEV_ADLEN_64B ) /* default case, no shift required */
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_0;
            }
            else if( adLen == NLMDEV_ADLEN_32B )
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_1;
            }
            else if( adLen == NLMDEV_ADLEN_128B )
            {
                *shiftDir   = NLMDEV_SHIFT_LEFT;
                *shiftCount = NLMDEV_SHIFT_CNT_1;
            }
            else
            {
                *shiftDir   = NLMDEV_SHIFT_LEFT;
                *shiftCount = NLMDEV_SHIFT_CNT_2;
            }

            break;
        }
        case NLMDEV_BLK_WIDTH_320:
        {
            if( adLen == NLMDEV_ADLEN_128B ) /* default case, no shift required */
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_0;
            }
            else if( adLen == NLMDEV_ADLEN_32B )
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_2;
            }
            else if( adLen == NLMDEV_ADLEN_64B )
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_1;
            }
            else
            {
                *shiftDir   = NLMDEV_SHIFT_LEFT;
                *shiftCount = NLMDEV_SHIFT_CNT_1;
            }

            break;
        }
        case NLMDEV_BLK_WIDTH_640:
        {
            if( adLen == NLMDEV_ADLEN_256B ) /* default case, no shift required */
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_0;
            }
            else if( adLen == NLMDEV_ADLEN_32B )
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_3;
            }
            else if( adLen == NLMDEV_ADLEN_64B )
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_2;
            }
            else /* 128B */
            {
                *shiftDir   = NLMDEV_SHIFT_RIGHT;
                *shiftCount = NLMDEV_SHIFT_CNT_1;
            }

            break;
        }
    }
}

/* kbp_dm_uda_config API will Write the BCR depending on block Number ,Width & Uda Address and Uda width */

NlmErrNum_t kbp_dm_uda_config(
    NlmDev              *dev_p,
    nlm_u8              portNum,
    nlm_u16             blk_num,
    nlm_u16             blk_width, /* In bits */
    nlm_u32             uda_addr,
    nlm_u16             uda_width, /* In bits */
    NlmReasonCode*      o_reason
    )
{
    NlmXptRqt* rqt_p = NULL;
    nlm_u16 reqDataLeninBits;

#ifndef NLM_12K_11K
    NlmDevShadowDevice *shadowDev_p;
    NlmDevBlockConfigReg *blkConfigReg_p;
    NlmDevShiftDir shiftDir =0;
    NlmDevShiftCount shiftCount =0;
    NlmErrNum_t errNum = NLMERR_OK;
    NlmDevBlockWidth    blockWidth;
    NlmDevADLength      adLen;
  
    (void)rqt_p;
    (void)reqDataLeninBits;

    if(*o_reason)
    *o_reason = NLMRSC_REASON_OK;

    if(blk_num >= dev_p->m_devMgr_p->m_numOfAbs)    
    {
        *o_reason = NLMRSC_INVALID_AB_NUM;
            return NLMERR_FAIL;
    }
    
    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }

    /* Get the shadow memory of blk config register for the specified blkNum */
    shadowDev_p = NLM_GET_SHADOW_MEM_FROM_DEV_PTR(dev_p);
    blkConfigReg_p = &shadowDev_p->m_arrayBlock[blk_num].m_blkConfig;
    
    /* Configure Block width */
    switch(blk_width)
    {
        case 80:
            blockWidth = NLMDEV_BLK_WIDTH_80;
            break;

        case 160:
            blockWidth = NLMDEV_BLK_WIDTH_160;
            break;

        case 320:
            blockWidth = NLMDEV_BLK_WIDTH_320;
            break;

        case 640:
            blockWidth = NLMDEV_BLK_WIDTH_640;
            break;
        default :
            {
                *o_reason = NLMRSC_INVALID_BLK_WIDTH;
                return NLMERR_FAIL;
            }
    }

    /* Assign UDA Width */
    switch(uda_width)
    {
        case 32:
            adLen = NLMDEV_ADLEN_32B;
            break;

        case 64:
            adLen = NLMDEV_ADLEN_64B;
            break;

        case 128:
            adLen = NLMDEV_ADLEN_128B;
            break;

        case 256:
            adLen = NLMDEV_ADLEN_256B;
            break;
        default :
            {
                *o_reason = NLMRSC_INVALID_TABLE_ASSO_WIDTH;
                return NLMERR_FAIL;
            }
    }

    /*get shift direction and shift Count */
    kbp_dm_pvt_get_shift_direction_count(blockWidth,adLen,&shiftDir,&shiftCount);
    
    /* Fill Block config Reg Data */
    blkConfigReg_p->m_baseAddr = uda_addr;
    blkConfigReg_p->m_blockWidth = blockWidth;
    blkConfigReg_p->m_shiftCount = shiftCount;
    blkConfigReg_p->m_shiftDir = shiftDir;
        
    /* Invoke the API which writes to specified Blk Register */
    if((errNum = kbp_dm_block_reg_write(dev_p,
                                            portNum,
                                            blk_num,
                                            NLMDEV_BLOCK_CONFIG_REG,
                                            blkConfigReg_p,
                                            o_reason)) != NLMERR_OK)
    {
        return NLMERR_FAIL;
    }
#else

    if(*o_reason)
    *o_reason = NLMRSC_REASON_OK;

    if(blk_num >= NLMDEV_NUM_ARRAY_BLOCKS/2)    
    {
        *o_reason = NLMRSC_INVALID_AB_NUM;
            return NLMERR_FAIL;
    }

    /* preparing write request for register. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev_p->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assigning request structure members */
    rqt_p->m_opcode[0] = NLM_OPCODE_PVT_UDA_CONFIG;

    /* length of the register data */
    rqt_p->m_data_len = NLMDEV_REG_LEN_IN_BYTES + 1;

    /* In case of Reg Write; "result" field of xpt rqt will be NULL */
    rqt_p->m_result = NULL;

    /* prepare request data 
    *[87:80] -> 1 Byte ->device Id
    *[79:64] -> 2 Bytes ->block Number,
    *[63:48] -> 2 Bytes -> block Width,
    *[47:16] -> 4 Bytes -> uda Address,
    *[15:0]  -> 2 Bytes -> uda Width */

    reqDataLeninBits = NLMDEV_REG_LEN_IN_BITS + 8;

    WriteBitsInArray(rqt_p->m_data,
                    NLMDEV_REG_LEN_IN_BYTES + 1,
                    reqDataLeninBits-1,
                    reqDataLeninBits-8,
                    (nlm_u8)dev_p->m_devId);

    WriteBitsInArray(rqt_p->m_data,
                    NLMDEV_REG_LEN_IN_BYTES + 1,
                    reqDataLeninBits-9,
                    reqDataLeninBits-24,
                    blk_num);

    WriteBitsInArray(rqt_p->m_data,
                    NLMDEV_REG_LEN_IN_BYTES + 1,
                    reqDataLeninBits-25,
                    reqDataLeninBits-40,
                    blk_width);

    WriteBitsInArray(rqt_p->m_data,
                    NLMDEV_REG_LEN_IN_BYTES + 1,
                    reqDataLeninBits-41,
                    reqDataLeninBits-72,
                    uda_addr);

    WriteBitsInArray(rqt_p->m_data,
                    NLMDEV_REG_LEN_IN_BYTES + 1,
                    reqDataLeninBits-73,
                    0,
                    uda_width);
    
    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev_p->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
            return NLMERR_FAIL;
    }
#endif

    return NLMERR_OK;
}


/* Function which perform block operations:
   1. Block copy
   2. Block move
   3. Block clear
   4. Block entry validate/invalidate
 */
NlmErrNum_t kbp_dm_block_operation(
    NlmDev                  *dev,           /*Device pointer */ 
    nlm_u8                  portNum,        /* Port number */
    NlmDevBlockOperParam    blkOperParam,   /* block instructions */
    NlmReasonCode*          o_reason
    )
{
#ifdef NLM_12K_11K

    if(*o_reason)
        *o_reason = NLMRSC_REASON_OK;

    (void)dev;
    (void)portNum;
    (void)blkOperParam;

    /* In case of 11k mode, call 11k specific function and return */
    if(o_reason)
    {
        *o_reason = NLMRSC_INVALID_OPERATION;
        return NLMERR_FAIL;
    }

#else
    NlmXptRqt* rqt_p = NULL;
    nlm_u32 value = 0, index = 0;
    nlm_u8  smtNum = NLM_DEFAULT_SMT_NUM;
            
    if(*o_reason)
        *o_reason = NLMRSC_REASON_OK;

    if(portNum >= NLM_MAX_NUM_PORT)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PORT_NUM;
        return NLMERR_FAIL;
    }
    if(!NLM_IS_VALID_DEV_PTR(dev))
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

    /* check the instruction type */
    if( blkOperParam.m_instType > NLMDEV_BLK_ENTRY_VALIDATE_INST)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return NLMERR_FAIL;
    }

    /* Maximum entries supported are 12b only i.e. 0-4095 */
    if(blkOperParam.m_numOfWords >= NLMDEV_AB_DEPTH)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return NLMERR_FAIL;
    }

    /* Count direction: UP [incremental: 1, 2, 3..], DOWN [decremental: 5, 4, 3..] */
    if(blkOperParam.m_countDir != NLMDEV_BLK_COUNT_UP && 
        blkOperParam.m_countDir != NLMDEV_BLK_COUNT_DOWN)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_PARAM;
        return NLMERR_FAIL;
    }

    /* Source address is only 20b so check it's value */
    if( (blkOperParam.m_srcAddr >= (nlm_u32)(dev->m_devMgr_p->m_numOfAbs*NLMDEV_AB_DEPTH)) )
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_ADDRESS;
        return NLMERR_FAIL;
    }
        
    /* validate the params only for block copy/move instructions */
    if(blkOperParam.m_instType == NLMDEV_BLK_MOVE_INST || blkOperParam.m_instType == NLMDEV_BLK_COPY_INST)
    {
        /* Destination address is only 20b so check it's value */
        if( (blkOperParam.m_destAddr >= (nlm_u32)(dev->m_devMgr_p->m_numOfAbs*NLMDEV_AB_DEPTH)) )
        {
            if(o_reason)
                *o_reason = NLMRSC_INVALID_ADDRESS;
            return NLMERR_FAIL;
        }

        /* params are ignored, safer side have valid values */
        blkOperParam.m_setOrClear = NLMDEV_INVALIDATE;
    }
    else /* validate the params only for block clear/entry validate/invalidate instructions */
    {
        /* 0: invalidate, 1: validate */
        if(blkOperParam.m_setOrClear != NLMDEV_INVALIDATE && 
            blkOperParam.m_setOrClear != NLMDEV_VALIDATE)
        {
            if(o_reason)
                *o_reason = NLMRSC_INVALID_PARAM;
            return NLMERR_FAIL;
        }

        /* params are ignored, safer side have valid values */
        blkOperParam.m_countDir = NLMDEV_BLK_COUNT_UP;
        blkOperParam.m_destAddr = 0;

    }
    
    /* preparing request for register read. */
    rqt_p = kbp_xpt_get_request((NlmXpt*)dev->m_devMgr_p->m_xpt_p, NULL);
    if(NULL == rqt_p)
    {
        if(o_reason)
            *o_reason = NLMRSC_INVALID_XPT_RQT_PTR;
        return NLMERR_NULL_PTR;
    }
    /* Clearing the rqt structure */
    NlmCm__memset(rqt_p, 0, sizeof(NlmXptRqt));

    /* assign port and smt details */
    rqt_p->m_port_num = portNum;
    rqt_p->m_smt_num = smtNum;

    /* Fill the appropriate opcodes for the instruction */ 
    if(blkOperParam.m_instType == NLMDEV_BLK_MOVE_INST)
    {
        rqt_p->m_opcode[0] = NLM_OPCODE_BLOCK_MOVE_BITS_8_6;
        rqt_p->m_opcode[1] = NLM_OPCODE_BLOCK_MOVE_BITS_5_0;
    }
    else if(blkOperParam.m_instType == NLMDEV_BLK_COPY_INST)
    {
        rqt_p->m_opcode[0] = NLM_OPCODE_BLOCK_COPY_BITS_8_6;
        rqt_p->m_opcode[1] = NLM_OPCODE_BLOCK_COPY_BITS_5_0;
    }
    else if(blkOperParam.m_instType == NLMDEV_BLK_CLEAR_INST)
    {
        rqt_p->m_opcode[0] = NLM_OPCODE_BLOCK_CLEAR_BITS_8_6;
        rqt_p->m_opcode[1] = NLM_OPCODE_BLOCK_CLEAR_BITS_5_0;
    }
    else
    {
        rqt_p->m_opcode[0] = NLM_OPCODE_BLOCK_EV_BITS_8_6;
        rqt_p->m_opcode[1] = NLM_OPCODE_BLOCK_EV_BITS_5_0;
    }
    
    /* hold the instruction details in 80b data */
    rqt_p->m_data_len = NLMDEV_REG_LEN_IN_BYTES;

    /* fill the request structure according to DS */
    if( blkOperParam.m_instType == NLMDEV_BLK_MOVE_INST ||
        blkOperParam.m_instType == NLMDEV_BLK_COPY_INST )
    {
        /* Byte 0 count_num[3:0], 0, devId[1:0], count_dir */
        rqt_p->m_data[index++] = (nlm_u8)( ((blkOperParam.m_numOfWords & 0x0f) << 4) |
            (dev->m_devId << 1) | (blkOperParam.m_countDir) );

        /* Byte 1  count_num[11:4]*/
        rqt_p->m_data[index++] = (nlm_u8)( (blkOperParam.m_numOfWords & 0x0ff0 ) >> 4); 

        /* Source address here */
        value = blkOperParam.m_srcAddr;

        rqt_p->m_data[index++] = (nlm_u8) ( value & 0x0ff);        /* Byte 2 : addr[07:00] */
        rqt_p->m_data[index++] = (nlm_u8) ((value >> 8 ) & 0x0ff); /* Byte 3 : addr[15:08] */
        rqt_p->m_data[index++] = (nlm_u8) ((value >> 16) & 0x0f);  /* Byte 4 : addr[19:16] */       

        /* Destination address here */
        value = blkOperParam.m_destAddr;
        
        rqt_p->m_data[index++] = (nlm_u8) ( value & 0x0ff);        /* Byte 5 : addr[07:00] */
        rqt_p->m_data[index++] = (nlm_u8) ((value >> 8 ) & 0x0ff); /* Byte 6 : addr[15:08] */
        rqt_p->m_data[index++] = (nlm_u8) ((value >> 16) & 0x0f);  /* Byte 7 : addr[19:16] */       
    }
    else
    {
        if(blkOperParam.m_instType == NLMDEV_BLK_CLEAR_INST)
        {
            /* Byte 0 count[3:0], 0, devId[1:0], 0 */
            rqt_p->m_data[index++] = (nlm_u8)( ((blkOperParam.m_numOfWords & 0x0f) << 4) |
                (dev->m_devId << 1) ); /* setNotClr = 0 */      
        }
        else
        {
            /* Byte 0 count[3:0], 0, devId[1:0], set/reset 0/1 */
            rqt_p->m_data[index++] = (nlm_u8)( ((blkOperParam.m_numOfWords & 0x0f) << 4) | 
                (dev->m_devId << 1) | (blkOperParam.m_setOrClear) );
        }

        /* Byte 1  count_num[11:4]*/
        rqt_p->m_data[index++] = (nlm_u8)( (blkOperParam.m_numOfWords & 0x0ff0 ) >> 4); 

        /* Source address here */
        value = blkOperParam.m_srcAddr;

        rqt_p->m_data[index++] = (nlm_u8) ( value & 0x0ff);        /* Byte 2 : addr[07:00] */
        rqt_p->m_data[index++] = (nlm_u8) ((value >> 8 ) & 0x0ff); /* Byte 3 : addr[15:08] */
        rqt_p->m_data[index++] = (nlm_u8) ((value >> 16) & 0x0f);  /* Byte 4 : addr[19:16] */

        /* Byte - 5, 6, and 7 are zeros */
    }
    
    /* calling transport layer to work on the current request */
    kbp_xpt_service_requests((NlmXpt*)dev->m_devMgr_p->m_xpt_p, o_reason);
    if(o_reason)
    {
        if(NLMRSC_REASON_OK != *o_reason)
            return NLMERR_FAIL;
    }

#endif
    return NLMERR_OK;

}


/* Wraper functions to Support old APIs */


NlmDev* NlmDevMgr__AddDevice(
    NlmDevMgr           *self,
    NlmDevId            *o_devId,
    NlmReasonCode       *o_reason
    )
 {
     return kbp_dm_add_device(self,o_devId,o_reason);
 }
 
 
 NlmErrNum_t NlmDevMgr__BlockRegisterRead(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16             abNum,      /* AB number in which register lies */
    NlmDevBlockRegType  regType,    /* see the enum description */
    void                    *o_data,    /*appropriate structure pointer */
    NlmReasonCode*          o_reason
    )
 {
    return kbp_dm_block_reg_read(dev,portNum,abNum,regType,o_data,o_reason);
 }
 
 NlmErrNum_t NlmDevMgr__BlockRegisterWrite(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16              abNum,     /* AB number in which register lies */
    NlmDevBlockRegType   regType,   /* see the enum description */
    const void          *data,      /*appropriate structure pointer */
    NlmReasonCode       *o_reason
    )
 {
    return kbp_dm_block_reg_write(dev,portNum,abNum,regType,data,o_reason);
}

NlmErrNum_t NlmDevMgr__CBAsRegisterRead(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16                 cbAddr,
    NlmDevCtxBufferReg  *o_cbRegData,           /* see the structure description above */
    NlmReasonCode*          o_reason
    )
{
    return kbp_dm_cb_reg_read(dev,portNum,cbAddr,o_cbRegData,o_reason);
}

NlmErrNum_t NlmDevMgr__CBAsRegisterWrite(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16                 cbAddr,
    NlmDevCtxBufferReg  *cbRegData,         /* see the structure description above */
    NlmReasonCode*          o_reason
    )
{
    return kbp_dm_cb_reg_write(dev,portNum,cbAddr,cbRegData,o_reason);
}

NlmErrNum_t NlmDevMgr__CBWrite(
    NlmDevMgr               *self,
    nlm_u8              portNum,  /* Port number */
    NlmDevCtxBufferInfo     *cbInfo,                /* see the structure description above */
    NlmReasonCode*              o_reason
    )
{
    return kbp_dm_cb_write(self,portNum,cbInfo,o_reason);
}

NlmErrNum_t NlmDevMgr__CmdSend(
    void                         *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u8                       bank_num,
    nlm_u8                       opcode_high,
    nlm_u8                       opcode_low,
    void                         *data,
    nlm_u8                       data_len,
    NlmReasonCode*               o_reason
    )
{
    return kbp_dm_cmd_send(dev,portNum,bank_num,opcode_high,opcode_low,data,data_len,o_reason);
}

NlmErrNum_t NlmDevMgr__Compare(
    NlmDevMgr               *self,
    nlm_u8                  portNum,            /* Port number */
    NlmDevCBWriteCmpParam   *cbInfo,                /* Search data for compares  */
    NlmDevCmpResult         *o_search_results,  /* Search results */
    NlmReasonCode*           o_reason
    )
{
    return kbp_dm_multi_compare(self,portNum,cbInfo,o_search_results,o_reason);
}

NlmErrNum_t NlmDevMgr__Compare1(
    NlmDevMgr           *self,
    nlm_u8              portNum,  /* Port number */
    nlm_u8              ltrNum,     /* LTR profile set number to be used */
    NlmDevCtxBufferInfo *cbInfo,       /* Search key details */
    NlmDevCmpResult     *o_search_results,  /* Result details */
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_cbwcmp1(self,portNum,ltrNum,cbInfo,o_search_results,o_reason);
}

NlmErrNum_t NlmDevMgr__Compare2(
    NlmDevMgr           *self,
    nlm_u8              portNum,  /* Port number */
    nlm_u8              ltrNum,   /* LTR profile set number to be used */
    NlmDevCtxBufferInfo *cbInfo,      /* Search key details */
    NlmDevCmpResult     *o_search_results,  /* Result details */
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_cbwcmp2(self,portNum,ltrNum,cbInfo,o_search_results,o_reason);
}

NlmErrNum_t NlmDevMgr__Compare3(
    NlmDevMgr           *self,
    nlm_u8              portNum,  /* Port number */
    nlm_u8              ltrNum,   /* LTR  number to be used */
    NlmDevCtxBufferInfo *cbInfo,   /* Search key details */
    NlmDevCmpResult     *o_search_results, /* Result details */
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_cbwcmp3(self,portNum,ltrNum,cbInfo,o_search_results,o_reason);
}

NlmErrNum_t NlmDevMgr__CompareLPM(
    NlmDevMgr               *self,
    nlm_u8              portNum,  /* Port number */
    nlm_u8                      ltrNum,             /* LTR profile set number to be used */
    NlmDevCtxBufferInfo     *cbInfo,            /* see the structure description above */
    NlmDevCmpResult         *o_search_results,  /* see the structure description above */
    NlmReasonCode*              o_reason
    )
{
    return kbp_dm_cbwlpm(self,portNum,ltrNum,cbInfo,o_search_results,o_reason);
}

NlmErrNum_t NlmDevMgr__DBAEntryRefresh(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16 abNum, /* Specifies the DBA Number */
    nlm_u16 abAddr, /* Specifies the DBA Entry location within the specified abNum  */
    NlmDevWriteMode writeMode,
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_dba_refresh(dev,portNum,abNum,abAddr,writeMode,o_reason);
}

NlmErrNum_t NlmDevMgr__DBAInvalidate(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16             abNum,      /* Specifies the AB Number */
    nlm_u16             abAddr,     /* Specifies the AB Entry location within the specified abNum  */
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_dba_invalidate(dev,portNum,abNum,abAddr,o_reason);
}

NlmErrNum_t NlmDevMgr__DBARead(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16             abNum,      /* Specifies the AB Number */
    nlm_u16             abAddr,     /* Specifies the AB Entry location within the specified abNum  */
    NlmDevABEntry   *o_abEntry,
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_dba_read(dev,portNum,abNum,abAddr,o_abEntry,o_reason);
}

NlmErrNum_t NlmDevMgr__DBAWrite(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16             abNum,      /* Specifies the DBA Block Number */
    nlm_u16             abAddr,     /* Specifies the Entry location within the specified abNum  */
    NlmDevABEntry   *abEntry,
    NlmDevWriteMode writeMode,
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_dba_write(dev,portNum,abNum,abAddr,abEntry,writeMode,o_reason);
}

NlmErrNum_t NlmDevMgr__EMRead(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16         abNum,    /* Specifies the DBA Block Number */
    nlm_u16         abAddr,  /* Specifies the entry location within the specified abNum  */
    NlmEMReadMode   readMode,  /* Read-A or Read-B */
    NlmDevEMEntry   *o_emEntry,
    NlmReasonCode*  o_reason
    )
{
    return kbp_dm_em_read(dev,portNum,abNum,abAddr,readMode,o_emEntry,o_reason);
}

NlmErrNum_t NlmDevMgr__EMWrite(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16         abNum,      /* Specifies the DBA Block Number */
    nlm_u16         abAddr,     /* Specifies the entry location within the specified abNum  */
    NlmDevEMEntry   *emEntry,
    NlmEMWriteMode  writeMode,
    NlmReasonCode*  o_reason
    )
{
    return kbp_dm_em_write(dev,portNum,abNum,abAddr,emEntry,writeMode,o_reason);
}

NlmErrNum_t NlmDevMgr__GlobalRegisterRead(
    NlmDev                  *dev,
    nlm_u8              portNum,  /* Port number */
    NlmDevGlobalRegType     regType,        /* see the enum description */
    void                        *o_data,
    NlmReasonCode*              o_reason
    )
{
    return kbp_dm_global_reg_read(dev,portNum,regType,o_data,o_reason);
}

NlmErrNum_t NlmDevMgr__GlobalRegisterWrite(
    NlmDev                  *dev,
    nlm_u8              portNum,  /* Port number */
    NlmDevGlobalRegType     regType,    /* Global register type - see definitions above */
    const void                  *data,      /* Global register structure pointer */
    NlmReasonCode*              o_reason
    )
{
    return kbp_dm_global_reg_write(dev,portNum,regType,data,o_reason);
}

NlmErrNum_t NlmDevMgr__LockConfig(
    NlmDevMgr       *self,
    NlmReasonCode       *o_reason
    )
{
    return kbp_dm_lock_config(self,o_reason);
}

NlmErrNum_t NlmDevMgr__LogicalTableRegisterRead(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u8                  ltrNum,     /* LTR profile set number */
    NlmDevLtrRegType        regType,    /* see the structure description above */
    void                    *o_data,    /* LTR register type structure pointer */
    NlmReasonCode*          o_reason
    )
{
    return kbp_dm_ltr_read(dev,portNum,ltrNum,regType,o_data,o_reason);
}

NlmErrNum_t NlmDevMgr__LogicalTableRegisterRefresh(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u8                  ltrNum,     /* LTR profile set number */
    NlmDevLtrRegType        regType,    /* see the structure description above */
    NlmReasonCode*          o_reason
    )
{
    return kbp_dm_ltr_refresh(dev,portNum,ltrNum,regType,o_reason);
}

NlmErrNum_t NlmDevMgr__LogicalTableRegisterWrite(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u8              ltrNum,     /* LTR profile set number */
    NlmDevLtrRegType        regType,    /* see the structure description above */
    const void          *data,      /* LTR register type structure pointer */
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_ltr_write(dev,portNum,ltrNum,regType,data,o_reason);
}

NlmErrNum_t NlmDevMgr__RegisterRead(
    void                        *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u32                     address,
    void                        *o_data,
    NlmReasonCode*              o_reason
    )
{
    return kbp_dm_generic_reg_read(dev,portNum,address,o_data,o_reason);
}

NlmErrNum_t NlmDevMgr__RegisterWrite(
    void                        *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u32                     address,
    void                        *data,
    NlmReasonCode*              o_reason
    )
{
    return kbp_dm_generic_reg_write(dev,portNum,address,data,o_reason);
}

NlmErrNum_t NlmDevMgr__ResetDevices(
    NlmDevMgr       *self,
    NlmReasonCode       *o_reason
    )
{
    return (kbp_dm_reset_devices(self,o_reason));
}

NlmErrNum_t NlmDevMgr__SendNop(
    NlmDev              *dev,
    nlm_u8              portNum, /* Port number */
    nlm_u32             numTimes,
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_send_nop(dev,portNum,numTimes,o_reason);
}

NlmErrNum_t NlmDevMgr__ShadowDeviceCreate(
    NlmDev      *dev_p,
    NlmReasonCode   *o_reason
    )
{
    return kbp_dm_shadow_create(dev_p,o_reason);
}

NlmErrNum_t NlmDevMgr__ShadowDeviceDestroy(
    NlmDev      *dev_p,
    NlmReasonCode   *o_reason
    )
{
    return kbp_dm_shadow_destroy(dev_p,o_reason);
}


NlmErrNum_t NlmDevMgr__UDARead(
    void                *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u32             address,
    void                *o_data,
    nlm_u8              length,
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_uda_read(dev,portNum,address,o_data,length,o_reason);
}

NlmErrNum_t NlmDevMgr__UDAWrite(
    void                *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u32             address,
    void                *data,
    nlm_u8              length,
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_uda_write(dev,portNum,address,data,length,o_reason);
}


NlmDevMgr* NlmDevMgr__create(
    NlmCmAllocator      *alloc_p,
    void                    *xpt_p,
    NlmDevType          devType,
    NlmPortMode         portMode,
    NlmSMTMode          smtMode,
    NlmReasonCode       *o_reason)
{
    return kbp_dm_init(alloc_p,xpt_p,devType,
                portMode,smtMode, NLMDEV_OPR_SAHASRA, o_reason);
}


void NlmDevMgr__destroy(
    NlmDevMgr       *self
    )
{
    kbp_dm_destroy(self);
}

NlmErrNum_t NlmDevMgr__RangeRegisterRead(
    NlmDev              *dev,
    nlm_u8              portNum,
    NlmSMTNum           smtNum,
    nlm_u32             address,
    NlmDevRangeReg      *o_rangeRegData,
    NlmReasonCode       *o_reason
    )
{
    return kbp_dm_range_reg_read(dev,portNum,smtNum,address,o_rangeRegData,o_reason);
}

NlmErrNum_t NlmDevMgr__RangeRegisterWrite(
    NlmDev              *dev,
    nlm_u8              portNum,
    NlmSMTNum           smtNum,
    nlm_u32             address,
    NlmDevRangeReg      *rangeRegData,
    NlmReasonCode       *o_reason
    )
{
    return kbp_dm_range_reg_write(dev,portNum,smtNum,address,rangeRegData,o_reason);
}


#ifndef NLMPLATFORM_BCM

NlmDev* bcm_kbp_dm_add_device(
    NlmDevMgr           *self,
    NlmDevId            *o_devId,
    NlmReasonCode       *o_reason
    )
 {
     return kbp_dm_add_device(self,o_devId,o_reason);
 }
 
 
 NlmErrNum_t bcm_kbp_dm_block_reg_read(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16             abNum,      /* AB number in which register lies */
    NlmDevBlockRegType  regType,    /* see the enum description */
    void                    *o_data,    /*appropriate structure pointer */
    NlmReasonCode*          o_reason
    )
 {
    return kbp_dm_block_reg_read(dev,portNum,abNum,regType,o_data,o_reason);
 }
 
 NlmErrNum_t bcm_kbp_dm_block_reg_write(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16              abNum,     /* AB number in which register lies */
    NlmDevBlockRegType   regType,   /* see the enum description */
    const void          *data,      /*appropriate structure pointer */
    NlmReasonCode       *o_reason
    )
 {
    return kbp_dm_block_reg_write(dev,portNum,abNum,regType,data,o_reason);
}

NlmErrNum_t bcm_kbp_dm_cb_reg_read(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16                 cbAddr,
    NlmDevCtxBufferReg  *o_cbRegData,           /* see the structure description above */
    NlmReasonCode*          o_reason
    )
{
    return kbp_dm_cb_reg_read(dev,portNum,cbAddr,o_cbRegData,o_reason);
}

NlmErrNum_t bcm_kbp_dm_cb_reg_write(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16                 cbAddr,
    NlmDevCtxBufferReg  *cbRegData,         /* see the structure description above */
    NlmReasonCode*          o_reason
    )
{
    return kbp_dm_cb_reg_write(dev,portNum,cbAddr,cbRegData,o_reason);
}

NlmErrNum_t bcm_kbp_dm_cb_write(
    NlmDevMgr               *self,
    nlm_u8              portNum,  /* Port number */
    NlmDevCtxBufferInfo     *cbInfo,                /* see the structure description above */
    NlmReasonCode*              o_reason
    )
{
    return kbp_dm_cb_write(self,portNum,cbInfo,o_reason);
}

NlmErrNum_t bcm_kbp_dm_cmd_send(
    void                         *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u8                       bank_num,
    nlm_u8                       opcode_high,
    nlm_u8                       opcode_low,
    void                         *data,
    nlm_u8                       data_len,
    NlmReasonCode*               o_reason
    )
{
    return kbp_dm_cmd_send(dev,portNum,bank_num,opcode_high,opcode_low,data,data_len,o_reason);
}

NlmErrNum_t bcm_kbp_dm_multi_compare(
    NlmDevMgr               *self,
    nlm_u8                  portNum,            /* Port number */
    NlmDevCBWriteCmpParam   *cbInfo,                /* Search data for compares  */
    NlmDevCmpResult         *o_search_results,  /* Search results */
    NlmReasonCode*           o_reason
    )
{
    return kbp_dm_multi_compare(self,portNum,cbInfo,o_search_results,o_reason);
}

NlmErrNum_t bcm_kbp_dm_cbwcmp1(
    NlmDevMgr           *self,
    nlm_u8              portNum,  /* Port number */
    nlm_u8              ltrNum,     /* LTR profile set number to be used */
    NlmDevCtxBufferInfo *cbInfo,       /* Search key details */
    NlmDevCmpResult     *o_search_results,  /* Result details */
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_cbwcmp1(self,portNum,ltrNum,cbInfo,o_search_results,o_reason);
}

NlmErrNum_t bcm_kbp_dm_cbwcmp2(
    NlmDevMgr           *self,
    nlm_u8              portNum,  /* Port number */
    nlm_u8              ltrNum,   /* LTR profile set number to be used */
    NlmDevCtxBufferInfo *cbInfo,      /* Search key details */
    NlmDevCmpResult     *o_search_results,  /* Result details */
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_cbwcmp2(self,portNum,ltrNum,cbInfo,o_search_results,o_reason);
}

NlmErrNum_t bcm_kbp_dm_cbwcmp3(
    NlmDevMgr           *self,
    nlm_u8              portNum,  /* Port number */
    nlm_u8              ltrNum,   /* LTR  number to be used */
    NlmDevCtxBufferInfo *cbInfo,   /* Search key details */
    NlmDevCmpResult     *o_search_results, /* Result details */
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_cbwcmp3(self,portNum,ltrNum,cbInfo,o_search_results,o_reason);
}

NlmErrNum_t bcm_kbp_dm_cbwlpm(
    NlmDevMgr               *self,
    nlm_u8              portNum,  /* Port number */
    nlm_u8                      ltrNum,             /* LTR profile set number to be used */
    NlmDevCtxBufferInfo     *cbInfo,            /* see the structure description above */
    NlmDevCmpResult         *o_search_results,  /* see the structure description above */
    NlmReasonCode*              o_reason
    )
{
    return kbp_dm_cbwlpm(self,portNum,ltrNum,cbInfo,o_search_results,o_reason);
}

NlmErrNum_t bcm_kbp_dm_dba_refresh(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16 abNum, /* Specifies the DBA Number */
    nlm_u16 abAddr, /* Specifies the DBA Entry location within the specified abNum  */
    NlmDevWriteMode writeMode,
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_dba_refresh(dev,portNum,abNum,abAddr,writeMode,o_reason);
}

NlmErrNum_t bcm_kbp_dm_dba_invalidate(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16             abNum,      /* Specifies the AB Number */
    nlm_u16             abAddr,     /* Specifies the AB Entry location within the specified abNum  */
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_dba_invalidate(dev,portNum,abNum,abAddr,o_reason);
}

NlmErrNum_t bcm_kbp_dm_dba_read(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16             abNum,      /* Specifies the AB Number */
    nlm_u16             abAddr,     /* Specifies the AB Entry location within the specified abNum  */
    NlmDevABEntry   *o_abEntry,
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_dba_read(dev,portNum,abNum,abAddr,o_abEntry,o_reason);
}

NlmErrNum_t bcm_kbp_dm_dba_write(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u16             abNum,      /* Specifies the DBA Block Number */
    nlm_u16             abAddr,     /* Specifies the Entry location within the specified abNum  */
    NlmDevABEntry   *abEntry,
    NlmDevWriteMode writeMode,
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_dba_write(dev,portNum,abNum,abAddr,abEntry,writeMode,o_reason);
}

NlmErrNum_t bcm_kbp_dm_em_read(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16         abNum,    /* Specifies the DBA Block Number */
    nlm_u16         abAddr,  /* Specifies the entry location within the specified abNum  */
    NlmEMReadMode   readMode,  /* Read-A or Read-B */
    NlmDevEMEntry   *o_emEntry,
    NlmReasonCode*  o_reason
    )
{
    return kbp_dm_em_read(dev,portNum,abNum,abAddr,readMode,o_emEntry,o_reason);
}

NlmErrNum_t bcm_kbp_dm_em_write(
    NlmDev              *dev,
    nlm_u8          portNum,  /* Port number */
    nlm_u16         abNum,      /* Specifies the DBA Block Number */
    nlm_u16         abAddr,     /* Specifies the entry location within the specified abNum  */
    NlmDevEMEntry   *emEntry,
    NlmEMWriteMode  writeMode,
    NlmReasonCode*  o_reason
    )
{
    return kbp_dm_em_write(dev,portNum,abNum,abAddr,emEntry,writeMode,o_reason);
}

NlmErrNum_t bcm_kbp_dm_global_reg_read(
    NlmDev                  *dev,
    nlm_u8              portNum,  /* Port number */
    NlmDevGlobalRegType     regType,        /* see the enum description */
    void                        *o_data,
    NlmReasonCode*              o_reason
    )
{
    return kbp_dm_global_reg_read(dev,portNum,regType,o_data,o_reason);
}

NlmErrNum_t bcm_kbp_dm_global_reg_write(
    NlmDev                  *dev,
    nlm_u8              portNum,  /* Port number */
    NlmDevGlobalRegType     regType,    /* Global register type - see definitions above */
    const void                  *data,      /* Global register structure pointer */
    NlmReasonCode*              o_reason
    )
{
    return kbp_dm_global_reg_write(dev,portNum,regType,data,o_reason);
}

NlmErrNum_t bcm_kbp_dm_lock_config(
    NlmDevMgr       *self,
    NlmReasonCode       *o_reason
    )
{
    return kbp_dm_lock_config(self,o_reason);
}

NlmErrNum_t bcm_kbp_dm_ltr_read(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u8                  ltrNum,     /* LTR profile set number */
    NlmDevLtrRegType        regType,    /* see the structure description above */
    void                    *o_data,    /* LTR register type structure pointer */
    NlmReasonCode*          o_reason
    )
{
    return kbp_dm_ltr_read(dev,portNum,ltrNum,regType,o_data,o_reason);
}

NlmErrNum_t bcm_kbp_dm_ltr_refresh(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u8                  ltrNum,     /* LTR profile set number */
    NlmDevLtrRegType        regType,    /* see the structure description above */
    NlmReasonCode*          o_reason
    )
{
    return kbp_dm_ltr_refresh(dev,portNum,ltrNum,regType,o_reason);
}

NlmErrNum_t bcm_kbp_dm_ltr_write(
    NlmDev              *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u8              ltrNum,     /* LTR profile set number */
    NlmDevLtrRegType        regType,    /* see the structure description above */
    const void          *data,      /* LTR register type structure pointer */
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_ltr_write(dev,portNum,ltrNum,regType,data,o_reason);
}

NlmErrNum_t bcm_kbp_dm_generic_reg_read(
    void                        *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u32                     address,
    void                        *o_data,
    NlmReasonCode*              o_reason
    )
{
    return kbp_dm_generic_reg_read(dev,portNum,address,o_data,o_reason);
}

NlmErrNum_t bcm_kbp_dm_generic_reg_write(
    void                        *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u32                     address,
    void                        *data,
    NlmReasonCode*              o_reason
    )
{
    return kbp_dm_generic_reg_write(dev,portNum,address,data,o_reason);
}

NlmErrNum_t bcm_kbp_dm_reset_devices(
    NlmDevMgr       *self,
    NlmReasonCode       *o_reason
    )
{
    return (kbp_dm_reset_devices(self,o_reason));
}

NlmErrNum_t bcm_kbp_dm_send_nop(
    NlmDev              *dev,
    nlm_u8              portNum, /* Port number */
    nlm_u32             numTimes,
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_send_nop(dev,portNum,numTimes,o_reason);
}

NlmErrNum_t bcm_kbp_dm_shadow_create(
    NlmDev      *dev_p,
    NlmReasonCode   *o_reason
    )
{
    return kbp_dm_shadow_create(dev_p,o_reason);
}

NlmErrNum_t bcm_kbp_dm_shadow_destroy(
    NlmDev      *dev_p,
    NlmReasonCode   *o_reason
    )
{
    return kbp_dm_shadow_destroy(dev_p,o_reason);
}

NlmErrNum_t bcm_kbp_dm_uda_config(
    NlmDev              *dev_p,
    nlm_u8              portNum,
    nlm_u16             blk_num,
    nlm_u16             blk_width,
    nlm_u32             uda_addr,
    nlm_u16             uda_width,
    NlmReasonCode*          o_reason
    )
{
    return kbp_dm_uda_config(
    dev_p,
    portNum,
    blk_num,
    blk_width,
    uda_addr,
    uda_width,
    o_reason
    );
}

NlmErrNum_t bcm_kbp_dm_uda_read(
    void                *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u32             address,
    void                *o_data,
    nlm_u8              length,
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_uda_read(dev,portNum,address,o_data,length,o_reason);
}

NlmErrNum_t bcm_kbp_dm_uda_write(
    void                *dev,
    nlm_u8              portNum,  /* Port number */
    nlm_u32             address,
    void                *data,
    nlm_u8              length,
    NlmReasonCode*      o_reason
    )
{
    return kbp_dm_uda_write(dev,portNum,address,data,length,o_reason);
}


NlmDevMgr* bcm_kbp_dm_init(
    NlmCmAllocator      *alloc_p,
    void                    *xpt_p,
    NlmDevType          devType,
    NlmPortMode         portMode,
    NlmSMTMode          smtMode,
    NlmDevOperationMode  operMode,
    NlmReasonCode       *o_reason)
{
    (void)operMode; 
    return kbp_dm_init(alloc_p,xpt_p,devType,
                portMode,smtMode, NLMDEV_OPR_SAHASRA, o_reason);
}


void bcm_kbp_dm_destroy(
    NlmDevMgr       *self
    )
{
    kbp_dm_destroy(self);
}

NlmErrNum_t bcm_kbp_dm_range_reg_read(
    NlmDev              *dev,
    nlm_u8              portNum,
    NlmSMTNum           smtNum,
    nlm_u32             address,
    NlmDevRangeReg      *o_rangeRegData,
    NlmReasonCode       *o_reason
    )
{
    return kbp_dm_range_reg_read(dev,portNum,smtNum,address,o_rangeRegData,o_reason);
}

NlmErrNum_t bcm_kbp_dm_range_reg_write(
    NlmDev              *dev,
    nlm_u8              portNum,
    NlmSMTNum           smtNum,
    nlm_u32             address,
    NlmDevRangeReg      *rangeRegData,
    NlmReasonCode       *o_reason
    )
{
    return kbp_dm_range_reg_write(dev,portNum,smtNum,address,rangeRegData,o_reason);
}

NlmErrNum_t bcm_kbp_dm_block_operation(
    NlmDev                  *dev,           /*Device pointer */ 
    nlm_u8                  portNum,        /* Port number */
    NlmDevBlockOperParam    blkOperParam,   /* block instructions */
    NlmReasonCode*          o_reason
    )
{
    return kbp_dm_block_operation(dev, portNum, blkOperParam, o_reason);
}

#endif





