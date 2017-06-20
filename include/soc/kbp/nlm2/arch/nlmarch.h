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
 
#ifndef INCLUDED_NLM11KCONSTANTS_HEADER
#define INCLUDED_NLM11KCONSTANTS_HEADER

#include "nlmcmportable.h"
#include "nlmcmexterncstart.h"

#define NLM11KDEV_NUM_KEYS					(4)
#define NLM11KDEV_NUM_LTR_SET				(64)
#define NLM11KDEV_NUM_PARALLEL_SEARCHES  	(4)

/* 40M device */
#define	NLM11KDEV_NUM_ARRAY_BLOCKS			(128)
#define	NLM11KDEV_NUM_SUPER_BLOCKS			(32)

/* 10M device */
#define	NLM11KDEV_10M_NUM_ARRAY_BLOCKS		(32)
#define	NLM11KDEV_10M_NUM_SUPER_BLOCKS		(8)

#define NLM11KDEV_NUM_AC15_BLOCKS			(8)

#define	NLM11KDEV_NUM_BLKS_PER_SUPER_BLOCK (NLM11KDEV_NUM_ARRAY_BLOCKS/NLM11KDEV_NUM_SUPER_BLOCKS)

#define NLM11KDEV_MAX_DEV_NUM	            (4)

/* Registers are 80 bit length */
#define	NLM11KDEV_REG_LEN_IN_BITS			(80)
#define	NLM11KDEV_REG_LEN_IN_BYTES			(NLM11KDEV_REG_LEN_IN_BITS/8)

#define NLM11KDEV_AB_ADDR_LEN_IN_BYTES         (4)
#define NLM11KDEV_REG_ADDR_LEN_IN_BYTES         (4)
#define NLM11KDEV_CB_ADDR_LEN_IN_BYTES      (2)
#define NLM11KDEV_CMP_RSLT_LEN_IN_BITS      (32)
#define NLM11KDEV_CMP_RSLT_LEN_IN_BYTES      (NLM11KDEV_CMP_RSLT_LEN_IN_BITS/8)

/* Context Buffer (CB) width is 80 bits, depth is 16384 */
#define	NLM11KDEV_CB_WIDTH_IN_BITS			(80)
#define	NLM11KDEV_CB_WIDTH_IN_BYTES			(NLM11KDEV_CB_WIDTH_IN_BITS / 8)
#define NLM11KDEV_MAX_CB_WRITE_IN_BITS      (640)
#define NLM11KDEV_MAX_CB_WRITE_IN_BYTES     (NLM11KDEV_MAX_CB_WRITE_IN_BITS/8)
#define	NLM11KDEV_CB_DEPTH					(16384)

/* Array Block (AB) width is 80 bits, depth is 4096 */
#define	NLM11KDEV_AB_WIDTH_IN_BITS			(80)
#define	NLM11KDEV_AB_WIDTH_IN_BYTES		(NLM11KDEV_AB_WIDTH_IN_BITS / 8)
#define	NLM11KDEV_AB_DEPTH					(4096)

#define	NLM11KDEV_MAX_AB_WIDTH_IN_BITS		(640)
#define	NLM11KDEV_MAX_AB_WIDTH_IN_BYTES		(NLM11KDEV_MAX_AB_WIDTH_IN_BITS / 8)

#define NLM11KDEV_NO_MASK_BMR_NUM              (7)
#define NLM11KDEV_NUM_OF_BMRS_PER_BLK          (5)
#define NLM11KDEV_NUM_OF_80B_SEGMENTS_PER_BMR          (4)

#define NLM11KDEV_NUM_OF_KCR_PER_KEY            (2)
#define NLM11KDEV_NUM_OF_SEGMENTS_PER_KCR       (5)
#define NLM11KDEV_NUM_OF_SEGMENTS_PER_KEY       (NLM11KDEV_NUM_OF_SEGMENTS_PER_KCR * NLM11KDEV_NUM_OF_KCR_PER_KEY)

#define NLM11KDEV_MAX_KEY_LEN_IN_BITS     (640)
#define NLM11KDEV_MAX_KEY_LEN_IN_BYTES    (NLM11KDEV_MAX_KEY_LEN_IN_BITS/8)

#define NLM11KDEV_FIB_MAX_PREFIX_LENGTH      (320)
#define NLM11KDEV_FIB_MAX_INDEX_RANGE        (0x7FFFFF)

#define NLM11KDEV_ADDR_TYPE_BIT_IN_PIO_WRITE          (25)
#define NLM11KDEV_AB_ENTRY_VALID_BIT_IN_ADDR        (30)
#define NLM11KDEV_AB_ENTRY_WR_MODE_BIT_IN_ADDR        (31)

#define NLM11KDEV_RANGE_DO_NOT_INSERT            (0x7F)
#define NLM11KDEV_RANGE_NUM_RANGE_TYPES          (4)
#define NLM11KDEV_RANGE_NUM_MCOR_PER_RANGE_TYPE  (8)
#define NLM11KDEV_RANGE_NUM_RANGE_CODE_REGS      (8)
#define NLM11KDEV_NUM_RANGE_REG					 (40)


/* Write data type definitions */
#define NLM11K_MDL_WR_MODE_DM			(0)
#define NLM11K_MDL_WR_MODE_XY			(1)
#define NLM11K_MDL_WR_REG				(2)

/* Read data type definitions */
#define NLM11K_MDL_RD_DATA_X			(3)		/* for reading data X */
#define NLM11K_MDL_RD_DATA_Y			(4)		/* for reading data Y */
#define NLM11K_MDL_RD_REG				(5)		/* for reading register */
#define NLM11K_MDL_RD_INVALID			(6)		/* for analyzing if read data is valid or not */

/* Valid bit definitions */
#define NLM11K_MDL_VBIT_INVALID		(0)
#define NLM11K_MDL_VBIT_VALID			(1)

/*
Command logging definitions:
One of these should be passed as flag type in flagParam argument of
Initialize cascade API. These options help to log
All the incoming instructions and results into a file.
*/
#define  NLM11K_MDL_COMMAND_LOG_NOTHING    0
#define  NLM11K_MDL_COMMAND_LOG_CV         1


/* enum definition for 4 parallel searches */
typedef enum
{
	NLM11KDEV_PARALLEL_SEARCH_0 = 0,
	NLM11KDEV_PARALLEL_SEARCH_1,
	NLM11KDEV_PARALLEL_SEARCH_2,
	NLM11KDEV_PARALLEL_SEARCH_3,

	NLM11KDEV_PARALLEL_SEARCH_END     /* invalid parallel search, must be last element */

} Nlm11kDevParallelSrch;

/* enum definition for 4 keys searched in parallel */
typedef enum
{
	NLM11KDEV_KEY_0 = 0,
	NLM11KDEV_KEY_1,
	NLM11KDEV_KEY_2,
	NLM11KDEV_KEY_3,

	NLM11KDEV_KEY_END             /* invalid key, must be last element */

} Nlm11kDevKey;

/*
 * Operation mode for the device
 */

typedef enum Nlm11kDev_OperationMode_t
{
    NLM11KDEV_OPR_STANDARD,
    NLM11KDEV_OPR_SAHASRA

}Nlm11kDev_OperationMode;

/* COMMANDS */
typedef enum Nlm11kCommand {
    NLM11K_CMD_CBWRITE,       /* Context buffer write */
    NLM11K_CMD_COMPARE1,      /* Context buffer write and compare1 */
    NLM11K_CMD_COMPARE2,      /* Context buffer write and compare2 */
    NLM11K_CMD_NOP,           /* Nop */
    NLM11K_CMD_WRITE,         /* Register Write and Database Write */
    NLM11K_CMD_READ_X,        /* Register Read and Database Read X */
    NLM11K_CMD_READ_Y        /* Database Read Y */
} Nlm11kCommand;

/* Control words(opcodes) */
#define NLM11K_OPCODE_CBWRITE_BITS_8_6  (0x04)  /* Cntxt Buffer Write Opcode Bits[8:6]*/
#define NLM11K_OPCODE_CBWRITE_BITS_5_0  (0x00)  /* Cntxt Buffer Write Opcode Bits[5:0]*/

#define NLM11K_OPCODE_CBWRITE_CMP1_BITS_8_6 (0x01) /* Cntxt Buffer Write and Cmp1 Opcode Bits[8:6]
                                              * Cntxt Buffer Write and Cmp1 Opcode Bits[5:0] --> Ltr Num  */

#define NLM11K_OPCODE_CBWRITE_CMP2_BITS_8_6 (0x02) /* Cntxt Buffer Write and Cmp2 Opcode Bits[8:6]
                                              * Slice 1 --> Ltr Num  */

#define NLM11K_OPCODE_NOP_BITS_8_6   (0x0)  /* Nop Opcode Bits[8:6]*/   
#define NLM11K_OPCODE_NOP_BITS_5_0   (0x00)  /* Nop (Slice 1)*/

#define NLM11K_OPCODE_PIO_WRITE_BITS_8_6    (0x0)  /* Register and Database Write Opcode Bits[8:6]) */
#define NLM11K_OPCODE_PIO_WRITE_BITS_5_0    (0x01)  /* Register and Database Write (Slice1) */

#define NLM11K_OPCODE_PIO_READ_X_BITS_8_6   (0x0)  /* Register and Database Read X Opcode Bits[8:6] */
#define NLM11K_OPCODE_PIO_READ_X_BITS_5_0   (0x02)  /* Register and Database Read X (Slice1) */

#define NLM11K_OPCODE_PIO_READ_Y_BITS_8_6   (0x0)  /* Database Read Y Opcode Bits[8:6] */
#define NLM11K_OPCODE_PIO_READ_Y_BITS_5_0   (0x03)  /* Database Read Y (Slice1) */   


/* Global Register definitions */
#define NLM11K_REG_ADDR_DEVICE_ID                  (0x00000000)
#define NLM11K_REG_ADDR_DEVICE_CONFIG              (0x00000001)
#define NLM11K_REG_ADDR_ERROR_STATUS               (0x00000002)
#define NLM11K_REG_ADDR_ERROR_STATUS_MASK          (0x00000003)

#define NLM11K_REG_ADDR_PARITY_SCAN_WRITE          (0x00000004)
#define NLM11K_REG_ADDR_PARITY_ERROR_FIFO          (0x00000005)
#define NLM11K_REG_ADDR_PARITY_SCAN_READ          (0x00000016)
#define NLM11K_REG_ADDR_PARITY_ERROR_FIFO_READ_POINTER          (0x000000017)
#define NLM11K_REG_ADDR_PARITY_ERROR_FIFO_WRITE_POINTER          (0x000000018)

#define NLM11K_REG_ADDR_DETAILED_PARITY_ERROR_INFO          (0x00000019)

#define NLM11K_REG_ADDR_NETLOGIC_ADDRESS          (0x000003FE)
#define NLM11K_REG_ADDR_NETLOGIC_DATA    	      (0x000003FF)

#define NLM11K_REG_ADDR_SCRATCH_PAD0               (0x00080000)
#define NLM11K_REG_ADDR_SCRATCH_PAD1               (0x00080001)
#define NLM11K_REG_ADDR_RESULT0                    (0x00080010)
#define NLM11K_REG_ADDR_RESULT1                    (0x00080011)


/* Block Registers */
#define NLM11K_REG_ADDR_BLK_CONFIG(x)              (0x00001000 + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR0_79_0(x)               (0x00001001 + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR0_159_80(x)             (0x00001002+ ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR0_239_160(x)            (0x00001003 + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR0_319_240(x)            (0x00001004 + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR1_79_0(x)               (0x00001005 + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR1_159_80(x)             (0x00001006 + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR1_239_160(x)            (0x00001007 + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR1_319_240(x)            (0x00001008 + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR2_79_0(x)               (0x00001009 + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR2_159_80(x)             (0x0000100A + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR2_239_160(x)            (0x0000100B + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR2_319_240(x)            (0x0000100C + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR3_79_0(x)               (0x0000100D + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR3_159_80(x)             (0x0000100E + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR3_239_160(x)            (0x0000100F + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR3_319_240(x)            (0x00001010 + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR4_79_0(x)               (0x00001011 + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR4_159_80(x)             (0x00001012 + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR4_239_160(x)            (0x00001013 + ((x) * 0x20))
#define NLM11K_REG_ADDR_BMR4_319_240(x)            (0x00001014 + ((x) * 0x20))


/* Logical Table Registers */
#define NLM11K_REG_ADDR_LTR_BLOCK_SELECT0(x)       (0x00004000 + ((x) * 0x20))
#define NLM11K_REG_ADDR_LTR_BLOCK_SELECT1(x)       (0x00004001 + ((x) * 0x20))

#define NLM11K_REG_ADDR_LTR_SB_KPU_SELECT(x)       (0x00004002 + ((x) * 0x20))

#define NLM11K_REG_ADDR_LTR_PARALLEL_SEARCH0(x)    (0x00004003 + ((x) * 0x20))
#define NLM11K_REG_ADDR_LTR_PARALLEL_SEARCH1(x)    (0x00004004 + ((x) * 0x20))
#define NLM11K_REG_ADDR_LTR_PARALLEL_SEARCH2(x)    (0x00004005 + ((x) * 0x20))
#define NLM11K_REG_ADDR_LTR_PARALLEL_SEARCH3(x)    (0x00004006 + ((x) * 0x20))

#define NLM11K_REG_ADDR_LTR_RANGE_INSERTION0(x)    (0x00004007 + ((x) * 0x20))
#define NLM11K_REG_ADDR_LTR_RANGE_INSERTION1(x)    (0x00004008 + ((x) * 0x20))
#define NLM11K_REG_ADDR_LTR_MISCELLANEOUS(x)    	(0x00004009 + ((x) * 0x20))


#define NLM11K_REG_ADDR_LTR_KPU0_KEY_CONSTRUCTION0(x)    (0x0000400B + ((x) * 0x20))
#define NLM11K_REG_ADDR_LTR_KPU0_KEY_CONSTRUCTION1(x)    (0x0000400C + ((x) * 0x20))
#define NLM11K_REG_ADDR_LTR_KPU1_KEY_CONSTRUCTION0(x)    (0x0000400D + ((x) * 0x20))
#define NLM11K_REG_ADDR_LTR_KPU1_KEY_CONSTRUCTION1(x)    (0x0000400E + ((x) * 0x20))
#define NLM11K_REG_ADDR_LTR_KPU2_KEY_CONSTRUCTION0(x)    (0x0000400F + ((x) * 0x20))
#define NLM11K_REG_ADDR_LTR_KPU2_KEY_CONSTRUCTION1(x)    (0x00004010 + ((x) * 0x20))
#define NLM11K_REG_ADDR_LTR_KPU3_KEY_CONSTRUCTION0(x)    (0x00004011 + ((x) * 0x20))
#define NLM11K_REG_ADDR_LTR_KPU3_KEY_CONSTRUCTION1(x)    (0x00004012 + ((x) * 0x20))

/* Context Buffer */
#define NLM11K_REG_ADDR_CONTEXT_BUFFER(x)          (0x00008000 + (x))

/* PCS Tx/Rx Registers */
#define NLM11K_REG_ADDR_PCS_TX(x)				(0x00010000 + (x))
#define NLM11K_REG_ADDR_PCS_RX(x)				(0x00018000 + (x))

/* Range Registers */
#define NLM11K_REG_RANGE_A_BOUNDS(x)			(0x00085000 + ((x) * 0x01))
#define NLM11K_REG_RANGE_B_BOUNDS(x)			(0x00085008 + ((x) * 0x01))
#define NLM11K_REG_RANGE_C_BOUNDS(x)			(0x00085010 + ((x) * 0x01))
#define NLM11K_REG_RANGE_D_BOUNDS(x)			(0x00085018 + ((x) * 0x01))

#define NLM11K_REG_RANGE_A_CODE0			(0x00085020)
#define NLM11K_REG_RANGE_A_CODE1			(0x00085021)
#define NLM11K_REG_RANGE_B_CODE0			(0x00085022)
#define NLM11K_REG_RANGE_B_CODE1			(0x00085023)
#define NLM11K_REG_RANGE_C_CODE0			(0x00085024)
#define NLM11K_REG_RANGE_C_CODE1			(0x00085025)
#define NLM11K_REG_RANGE_D_CODE0			(0x00085026)
#define NLM11K_REG_RANGE_D_CODE1			(0x00085027)

#if !defined NLM_12K_11K && !defined NLM_12K 

#define NLMDEV_NUM_KEYS						NLM11KDEV_NUM_KEYS
#define NLMDEV_NUM_LTR_SET					NLM11KDEV_NUM_LTR_SET
#define NLMDEV_NUM_PARALLEL_SEARCHES		NLM11KDEV_NUM_PARALLEL_SEARCHES

/* 40M device */
#define	NLMDEV_NUM_ARRAY_BLOCKS				NLM11KDEV_NUM_ARRAY_BLOCKS
#define	NLMDEV_NUM_SUPER_BLOCKS				NLM11KDEV_NUM_SUPER_BLOCKS

/* 10M device */
#define	NLMDEV_10M_NUM_ARRAY_BLOCKS			NLM11KDEV_10M_NUM_ARRAY_BLOCKS
#define	NLMDEV_10M_NUM_SUPER_BLOCKS			NLM11KDEV_10M_NUM_SUPER_BLOCKS

#define NLMDEV_NUM_AC15_BLOCKS				NLM11KDEV_NUM_AC15_BLOCKS

#define	NLMDEV_NUM_BLKS_PER_SUPER_BLOCK		NLM11KDEV_NUM_BLKS_PER_SUPER_BLOCK

#define NLMDEV_MAX_DEV_NUM					NLM11KDEV_MAX_DEV_NUM

/* Registers are 80 bit length */
#define	NLMDEV_REG_LEN_IN_BITS				NLM11KDEV_REG_LEN_IN_BITS
#define	NLMDEV_REG_LEN_IN_BYTES				NLM11KDEV_REG_LEN_IN_BYTES

#define NLMDEV_AB_ADDR_LEN_IN_BYTES         NLM11KDEV_AB_ADDR_LEN_IN_BYTES
#define NLMDEV_REG_ADDR_LEN_IN_BYTES		NLM11KDEV_REG_ADDR_LEN_IN_BYTES
#define NLMDEV_CB_ADDR_LEN_IN_BYTES			NLM11KDEV_CB_ADDR_LEN_IN_BYTES
#define NLMDEV_CMP_RSLT_LEN_IN_BITS			NLM11KDEV_CMP_RSLT_LEN_IN_BITS
#define NLMDEV_CMP_RSLT_LEN_IN_BYTES		NLM11KDEV_CMP_RSLT_LEN_IN_BYTES

/* Context Buffer (CB) width is 80 bits, depth is 16384 */
#define	NLMDEV_CB_WIDTH_IN_BITS				NLM11KDEV_CB_WIDTH_IN_BITS
#define	NLMDEV_CB_WIDTH_IN_BYTES			NLM11KDEV_CB_WIDTH_IN_BYTES
#define NLMDEV_MAX_CB_WRITE_IN_BITS			NLM11KDEV_MAX_CB_WRITE_IN_BITS
#define NLMDEV_MAX_CB_WRITE_IN_BYTES		NLM11KDEV_MAX_CB_WRITE_IN_BYTES
#define	NLMDEV_CB_DEPTH						NLM11KDEV_CB_DEPTH

/* Array Block (AB) width is 80 bits, depth is 4096 */
#define	NLMDEV_AB_WIDTH_IN_BITS				NLM11KDEV_AB_WIDTH_IN_BITS
#define	NLMDEV_AB_WIDTH_IN_BYTES			NLM11KDEV_AB_WIDTH_IN_BYTES
#define	NLMDEV_AB_DEPTH						NLM11KDEV_AB_DEPTH

#define	NLMDEV_MAX_AB_WIDTH_IN_BITS			NLM11KDEV_MAX_AB_WIDTH_IN_BITS
#define	NLMDEV_MAX_AB_WIDTH_IN_BYTES		NLM11KDEV_MAX_AB_WIDTH_IN_BYTES

#define NLMDEV_NO_MASK_BMR_NUM				NLM11KDEV_NO_MASK_BMR_NUM
#define NLMDEV_NUM_OF_BMRS_PER_BLK			NLM11KDEV_NUM_OF_BMRS_PER_BLK
#define NLMDEV_NUM_OF_80B_SEGMENTS_PER_BMR  NLM11KDEV_NUM_OF_80B_SEGMENTS_PER_BMR

#define NLMDEV_NUM_OF_KCR_PER_KEY			NLM11KDEV_NUM_OF_KCR_PER_KEY
#define NLMDEV_NUM_OF_SEGMENTS_PER_KCR		NLM11KDEV_NUM_OF_SEGMENTS_PER_KCR
#define NLMDEV_NUM_OF_SEGMENTS_PER_KEY		NLM11KDEV_NUM_OF_SEGMENTS_PER_KEY

#define NLMDEV_MAX_KEY_LEN_IN_BITS			NLM11KDEV_MAX_KEY_LEN_IN_BITS
#define NLMDEV_MAX_KEY_LEN_IN_BYTES			NLM11KDEV_MAX_KEY_LEN_IN_BYTES

#define NLMDEV_FIB_MAX_PREFIX_LENGTH		NLM11KDEV_FIB_MAX_PREFIX_LENGTH
#define NLMDEV_FIB_MAX_INDEX_RANGE			NLM11KDEV_FIB_MAX_INDEX_RANGE

#define NLMDEV_ADDR_TYPE_BIT_IN_PIO_WRITE      NLM11KDEV_ADDR_TYPE_BIT_IN_PIO_WRITE
#define NLMDEV_AB_ENTRY_VALID_BIT_IN_ADDR      NLM11KDEV_AB_ENTRY_VALID_BIT_IN_ADDR
#define NLMDEV_AB_ENTRY_WR_MODE_BIT_IN_ADDR    NLM11KDEV_AB_ENTRY_WR_MODE_BIT_IN_ADDR

#define NLMDEV_RANGE_DO_NOT_INSERT            NLM11KDEV_RANGE_DO_NOT_INSERT
#define NLMDEV_RANGE_NUM_RANGE_TYPES          NLM11KDEV_RANGE_NUM_RANGE_TYPES
#define NLMDEV_RANGE_NUM_MCOR_PER_RANGE_TYPE  NLM11KDEV_RANGE_NUM_MCOR_PER_RANGE_TYPE
#define NLMDEV_RANGE_NUM_RANGE_CODE_REGS      NLM11KDEV_RANGE_NUM_RANGE_CODE_REGS
#define NLMDEV_NUM_RANGE_REG				  NLM11KDEV_NUM_RANGE_REG


/* Write data type definitions */
#define NLM_MDL_WR_MODE_DM					NLM11K_MDL_WR_MODE_DM
#define NLM_MDL_WR_MODE_XY					NLM11K_MDL_WR_MODE_XY
#define NLM_MDL_WR_REG						NLM11K_MDL_WR_REG

/* Read data type definitions */
#define NLM_MDL_RD_DATA_X				NLM11K_MDL_RD_DATA_X		/* for reading data X */
#define NLM_MDL_RD_DATA_Y				NLM11K_MDL_RD_DATA_Y	/* for reading data Y */
#define NLM_MDL_RD_REG					NLM11K_MDL_RD_REG		/* for reading register */
#define NLM_MDL_RD_INVALID				NLM11K_MDL_RD_INVALID	/* for analyzing if read data is valid or not */

/* Valid bit definitions */
#define NLM_MDL_VBIT_INVALID			NLM11K_MDL_VBIT_INVALID
#define NLM_MDL_VBIT_VALID				NLM11K_MDL_VBIT_VALID

/*
Command logging definitions:
One of these should be passed as flag type in flagParam argument of
Initialize cascade API. These options help to log
All the incoming instructions and results into a file.
*/
#define  NLM_MDL_COMMAND_LOG_NOTHING	NLM11K_MDL_COMMAND_LOG_NOTHING
#define  NLM_MDL_COMMAND_LOG_CV         NLM11K_MDL_COMMAND_LOG_CV


/* Control words(opcodes) */
#define NLM_OPCODE_CBWRITE_BITS_8_6		NLM11K_OPCODE_CBWRITE_BITS_8_6  /* Cntxt Buffer Write Opcode Bits[8:6]*/
#define NLM_OPCODE_CBWRITE_BITS_5_0		NLM11K_OPCODE_CBWRITE_BITS_5_0  /* Cntxt Buffer Write Opcode Bits[5:0]*/

#define NLM_OPCODE_CBWRITE_CMP1_BITS_8_6	NLM11K_OPCODE_CBWRITE_CMP1_BITS_8_6 /* Cntxt Buffer Write and Cmp1 Opcode Bits[8:6]
                                              * Cntxt Buffer Write and Cmp1 Opcode Bits[5:0] --> Ltr Num  */

#define NLM_OPCODE_CBWRITE_CMP2_BITS_8_6	NLM11K_OPCODE_CBWRITE_CMP2_BITS_8_6 /* Cntxt Buffer Write and Cmp2 Opcode Bits[8:6]
                                              * Slice 1 --> Ltr Num  */

#define NLM_OPCODE_NOP_BITS_8_6			NLM11K_OPCODE_NOP_BITS_8_6  /* Nop Opcode Bits[8:6]*/   
#define NLM_OPCODE_NOP_BITS_5_0			NLM11K_OPCODE_NOP_BITS_5_0  /* Nop (Slice 1)*/

#define NLM_OPCODE_PIO_WRITE_BITS_8_6    NLM11K_OPCODE_PIO_WRITE_BITS_8_6  /* Register and Database Write Opcode Bits[8:6]) */
#define NLM_OPCODE_PIO_WRITE_BITS_5_0    NLM11K_OPCODE_PIO_WRITE_BITS_5_0  /* Register and Database Write (Slice1) */

#define NLM_OPCODE_PIO_READ_X_BITS_8_6   NLM11K_OPCODE_PIO_READ_X_BITS_8_6  /* Register and Database Read X Opcode Bits[8:6] */
#define NLM_OPCODE_PIO_READ_X_BITS_5_0   NLM11K_OPCODE_PIO_READ_X_BITS_5_0  /* Register and Database Read X (Slice1) */

#define NLM_OPCODE_PIO_READ_Y_BITS_8_6   NLM11K_OPCODE_PIO_READ_Y_BITS_8_6  /* Database Read Y Opcode Bits[8:6] */
#define NLM_OPCODE_PIO_READ_Y_BITS_5_0   NLM11K_OPCODE_PIO_READ_Y_BITS_5_0  /* Database Read Y (Slice1) */   


/* Global Register definitions */
#define NLM_REG_ADDR_DEVICE_ID                  	NLM11K_REG_ADDR_DEVICE_ID
#define NLM_REG_ADDR_DEVICE_CONFIG              	NLM11K_REG_ADDR_DEVICE_CONFIG
#define NLM_REG_ADDR_ERROR_STATUS					NLM11K_REG_ADDR_ERROR_STATUS
#define NLM_REG_ADDR_ERROR_STATUS_MASK				NLM11K_REG_ADDR_ERROR_STATUS_MASK

#define NLM_REG_ADDR_PARITY_SCAN_WRITE          	NLM11K_REG_ADDR_PARITY_SCAN_WRITE
#define NLM_REG_ADDR_PARITY_ERROR_FIFO          	NLM11K_REG_ADDR_PARITY_ERROR_FIFO
#define NLM_REG_ADDR_PARITY_SCAN_READ				NLM11K_REG_ADDR_PARITY_SCAN_READ
#define NLM_REG_ADDR_PARITY_ERROR_FIFO_READ_POINTER          NLM11K_REG_ADDR_PARITY_ERROR_FIFO_READ_POINTER
#define NLM_REG_ADDR_PARITY_ERROR_FIFO_WRITE_POINTER         NLM11K_REG_ADDR_PARITY_ERROR_FIFO_WRITE_POINTER

#define NLM_REG_ADDR_DETAILED_PARITY_ERROR_INFO				 NLM11K_REG_ADDR_DETAILED_PARITY_ERROR_INFO

#define NLM_REG_ADDR_NETLOGIC_ADDRESS          NLM11K_REG_ADDR_NETLOGIC_ADDRESS
#define NLM_REG_ADDR_NETLOGIC_DATA    	       NLM11K_REG_ADDR_NETLOGIC_DATA

#define NLM_REG_ADDR_SCRATCH_PAD0               NLM11K_REG_ADDR_SCRATCH_PAD0
#define NLM_REG_ADDR_SCRATCH_PAD1               NLM11K_REG_ADDR_SCRATCH_PAD1
#define NLM_REG_ADDR_RESULT0                    NLM11K_REG_ADDR_RESULT0
#define NLM_REG_ADDR_RESULT1                    NLM11K_REG_ADDR_RESULT1


/* Block Registers */
#define NLM_REG_ADDR_BLK_CONFIG(x)              NLM11K_REG_ADDR_BLK_CONFIG(x)
#define NLM_REG_ADDR_BMR0_79_0(x)				NLM11K_REG_ADDR_BMR0_79_0(x)
#define NLM_REG_ADDR_BMR0_159_80(x)             NLM11K_REG_ADDR_BMR0_159_80(x)
#define NLM_REG_ADDR_BMR0_239_160(x)            NLM11K_REG_ADDR_BMR0_239_160(x)
#define NLM_REG_ADDR_BMR0_319_240(x)            NLM11K_REG_ADDR_BMR0_319_240(x)
#define NLM_REG_ADDR_BMR1_79_0(x)               NLM11K_REG_ADDR_BMR1_79_0(x)
#define NLM_REG_ADDR_BMR1_159_80(x)             NLM11K_REG_ADDR_BMR1_159_80(x)
#define NLM_REG_ADDR_BMR1_239_160(x)            NLM11K_REG_ADDR_BMR1_239_160(x)
#define NLM_REG_ADDR_BMR1_319_240(x)            NLM11K_REG_ADDR_BMR1_319_240(x)
#define NLM_REG_ADDR_BMR2_79_0(x)               NLM11K_REG_ADDR_BMR2_79_0(x)
#define NLM_REG_ADDR_BMR2_159_80(x)             NLM11K_REG_ADDR_BMR2_159_80(x)
#define NLM_REG_ADDR_BMR2_239_160(x)            NLM11K_REG_ADDR_BMR2_239_160(x)
#define NLM_REG_ADDR_BMR2_319_240(x)            NLM11K_REG_ADDR_BMR2_319_240(x)
#define NLM_REG_ADDR_BMR3_79_0(x)               NLM11K_REG_ADDR_BMR3_79_0(x)
#define NLM_REG_ADDR_BMR3_159_80(x)             NLM11K_REG_ADDR_BMR3_159_80(x)
#define NLM_REG_ADDR_BMR3_239_160(x)            NLM11K_REG_ADDR_BMR3_239_160(x)
#define NLM_REG_ADDR_BMR3_319_240(x)            NLM11K_REG_ADDR_BMR3_319_240(x)
#define NLM_REG_ADDR_BMR4_79_0(x)               NLM11K_REG_ADDR_BMR4_79_0(x)
#define NLM_REG_ADDR_BMR4_159_80(x)             NLM11K_REG_ADDR_BMR4_159_80(x)
#define NLM_REG_ADDR_BMR4_239_160(x)            NLM11K_REG_ADDR_BMR4_239_160(x)
#define NLM_REG_ADDR_BMR4_319_240(x)            NLM11K_REG_ADDR_BMR4_319_240(x)


/* Logical Table Registers */
#define NLM_REG_ADDR_LTR_BLOCK_SELECT0(x)       NLM11K_REG_ADDR_LTR_BLOCK_SELECT0(x)
#define NLM_REG_ADDR_LTR_BLOCK_SELECT1(x)       NLM11K_REG_ADDR_LTR_BLOCK_SELECT1(x) 

#define NLM_REG_ADDR_LTR_SB_KPU_SELECT(x)       NLM11K_REG_ADDR_LTR_SB_KPU_SELECT(x)

#define NLM_REG_ADDR_LTR_PARALLEL_SEARCH0(x)    NLM11K_REG_ADDR_LTR_PARALLEL_SEARCH0(x)
#define NLM_REG_ADDR_LTR_PARALLEL_SEARCH1(x)    NLM11K_REG_ADDR_LTR_PARALLEL_SEARCH1(x)
#define NLM_REG_ADDR_LTR_PARALLEL_SEARCH2(x)    NLM11K_REG_ADDR_LTR_PARALLEL_SEARCH2(x)
#define NLM_REG_ADDR_LTR_PARALLEL_SEARCH3(x)    NLM11K_REG_ADDR_LTR_PARALLEL_SEARCH3(x)

#define NLM_REG_ADDR_LTR_RANGE_INSERTION0(x)    NLM11K_REG_ADDR_LTR_RANGE_INSERTION0(x)
#define NLM_REG_ADDR_LTR_RANGE_INSERTION1(x)    NLM11K_REG_ADDR_LTR_RANGE_INSERTION1(x)
#define NLM_REG_ADDR_LTR_MISCELLANEOUS(x)    	NLM11K_REG_ADDR_LTR_MISCELLANEOUS(x)


#define NLM_REG_ADDR_LTR_KPU0_KEY_CONSTRUCTION0(x)    NLM11K_REG_ADDR_LTR_KPU0_KEY_CONSTRUCTION0(x)
#define NLM_REG_ADDR_LTR_KPU0_KEY_CONSTRUCTION1(x)    NLM11K_REG_ADDR_LTR_KPU0_KEY_CONSTRUCTION1(x)
#define NLM_REG_ADDR_LTR_KPU1_KEY_CONSTRUCTION0(x)    NLM11K_REG_ADDR_LTR_KPU1_KEY_CONSTRUCTION0(x)
#define NLM_REG_ADDR_LTR_KPU1_KEY_CONSTRUCTION1(x)    NLM11K_REG_ADDR_LTR_KPU1_KEY_CONSTRUCTION1(x)
#define NLM_REG_ADDR_LTR_KPU2_KEY_CONSTRUCTION0(x)    NLM11K_REG_ADDR_LTR_KPU2_KEY_CONSTRUCTION0(x)
#define NLM_REG_ADDR_LTR_KPU2_KEY_CONSTRUCTION1(x)    NLM11K_REG_ADDR_LTR_KPU2_KEY_CONSTRUCTION1(x)
#define NLM_REG_ADDR_LTR_KPU3_KEY_CONSTRUCTION0(x)    NLM11K_REG_ADDR_LTR_KPU3_KEY_CONSTRUCTION0(x)
#define NLM_REG_ADDR_LTR_KPU3_KEY_CONSTRUCTION1(x)    NLM11K_REG_ADDR_LTR_KPU3_KEY_CONSTRUCTION1(x)

/* Context Buffer */
#define NLM_REG_ADDR_CONTEXT_BUFFER(x)          NLM11K_REG_ADDR_CONTEXT_BUFFER(x)

/* PCS Tx/Rx Registers */
#define NLM_REG_ADDR_PCS_TX(x)				NLM11K_REG_ADDR_PCS_TX(x)
#define NLM_REG_ADDR_PCS_RX(x)				NLM11K_REG_ADDR_PCS_RX(x)

/* Range Registers */
#define NLM_REG_RANGE_A_BOUNDS(x)			NLM11K_REG_RANGE_A_BOUNDS(x)
#define NLM_REG_RANGE_B_BOUNDS(x)			NLM11K_REG_RANGE_B_BOUNDS(x)
#define NLM_REG_RANGE_C_BOUNDS(x)			NLM11K_REG_RANGE_C_BOUNDS(x)
#define NLM_REG_RANGE_D_BOUNDS(x)			NLM11K_REG_RANGE_D_BOUNDS(x)

#define NLM_REG_RANGE_A_CODE0			NLM11K_REG_RANGE_A_CODE0
#define NLM_REG_RANGE_A_CODE1			NLM11K_REG_RANGE_A_CODE1
#define NLM_REG_RANGE_B_CODE0			NLM11K_REG_RANGE_B_CODE0
#define NLM_REG_RANGE_B_CODE1			NLM11K_REG_RANGE_B_CODE1
#define NLM_REG_RANGE_C_CODE0			NLM11K_REG_RANGE_C_CODE0
#define NLM_REG_RANGE_C_CODE1			NLM11K_REG_RANGE_C_CODE1
#define NLM_REG_RANGE_D_CODE0			NLM11K_REG_RANGE_D_CODE0
#define NLM_REG_RANGE_D_CODE1			NLM11K_REG_RANGE_D_CODE1

/* enum definition for 4 parallel searches */
typedef enum
{
	NLMDEV_PARALLEL_SEARCH_0 = 0,
	NLMDEV_PARALLEL_SEARCH_1,
	NLMDEV_PARALLEL_SEARCH_2,
	NLMDEV_PARALLEL_SEARCH_3,

	NLMDEV_PARALLEL_SEARCH_END     /* invalid parallel search, must be last element */

} NlmDevParallelSrch;

/* enum definition for 4 keys searched in parallel */
typedef enum
{
	NLMDEV_KEY_0 = 0,
	NLMDEV_KEY_1,
	NLMDEV_KEY_2,
	NLMDEV_KEY_3,

	NLMDEV_KEY_END             /* invalid key, must be last element */

} NlmDevKey;

/*
 * Operation mode for the device
 */

typedef enum NlmDev_OperationMode_t
{
    NLMDEV_OPR_STANDARD,
    NLMDEV_OPR_SAHASRA

}NlmDev_OperationMode;

/* COMMANDS */
typedef enum NlmCommand {
    NLM_CMD_CBWRITE,       /* Context buffer write */
    NLM_CMD_COMPARE1,      /* Context buffer write and compare1 */
    NLM_CMD_COMPARE2,      /* Context buffer write and compare2 */
    NLM_CMD_NOP,           /* Nop */
    NLM_CMD_WRITE,         /* Register Write and Database Write */
    NLM_CMD_READ_X,        /* Register Read and Database Read X */
    NLM_CMD_READ_Y        /* Database Read Y */
} NlmCommand;

#endif

#include "nlmcmexterncend.h"
#endif /* INCLUDED_NLM11KCONSTANTS_HEADER */
