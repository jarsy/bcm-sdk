/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfSbQe2000ElibPCTSingle.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_SB_QE2000_ELIB_PCTSINGLE_ENTRY_H
#define SB_ZF_SB_QE2000_ELIB_PCTSINGLE_ENTRY_H

#define SB_ZF_SB_QE2000_ELIB_PCTSINGLE_ENTRY_SIZE_IN_BYTES 8
#define SB_ZF_SB_QE2000_ELIB_PCTSINGLE_ENTRY_SIZE 8
#define SB_ZF_SB_QE2000_ELIB_PCTSINGLE_ENTRY_M_PKTCLASS_BITS "63:35"
#define SB_ZF_SB_QE2000_ELIB_PCTSINGLE_ENTRY_M_BYTECLASS_BITS "34:0"
#define SB_ZF_SB_QE2000_ELIB_PCTSINGLE_ENTRY_OFFSET         0x0E0 /**< Offset into the classifier memory for the PCT */


/**
 * @file sbZfSbQe2000ElibPCTSingle.h
 *
 * <pre>
 *
 * ====================================================================================
 * ==  sbZfSbQe2000ElibPCTSingle.h - Port Counter Table Single Class ZFrame  ==
 * ====================================================================================
 *
 * WORKING REVISION: $Id: sbZfSbQe2000ElibPCTSingle.hx,v 1.4 Broadcom SDK $
 *
 * Copyright (c) Sandburst, Inc. 2005
 * All Rights Reserved.  Unpublished rights reserved under the copyright
 * laws of the United States.
 *
 * The software contained on this media is proprietary to and embodies the
 * confidential technology of Sandburst, Inc. Possession, use, duplication
 * or dissemination of the software and media is authorized only pursuant
 * to a valid written license from Sandburst, Inc.
 *
 * RESTRICTED RIGHTS LEGEND Use, duplication, or disclosure by the U.S.
 * Government is subject to restrictions as set forth in Subparagraph
 * (c) (1) (ii) of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
 *
 *
 * MODULE NAME:
 *
 *     sbZfSbQe2000ElibPCTSingle.zf
 *
 * ABSTRACT:
 *
 *     Port Counter Table ZFrame Definition
 *     An entry in the port count table is 16 counts
 *     That is, a per-class count for one port.  This Zframe
 *     represents a per-class count.
 *
 * LANGUAGE:
 *
 *     ZFrame
 *
 * AUTHORS:
 *
 *     Travis B. Sawyer
 *
 * CREATION DATE:
 *
 *     10-June-2005
 * </pre>
 */
typedef struct _sbZfSbQe2000ElibPCTSingle {
  uint64 m_PktClass;
  uint64 m_ByteClass;
} sbZfSbQe2000ElibPCTSingle_t;

uint32
sbZfSbQe2000ElibPCTSingle_Pack(sbZfSbQe2000ElibPCTSingle_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex);
void
sbZfSbQe2000ElibPCTSingle_Unpack(sbZfSbQe2000ElibPCTSingle_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex);
void
sbZfSbQe2000ElibPCTSingle_InitInstance(sbZfSbQe2000ElibPCTSingle_t *pFrame);

#define SB_ZF_SBQE2000ELIBPCTSINGLE_SET_PKTCNT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCTSINGLE_SET_BYTECNT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x07) | (((nFromData) >> 32) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCTSINGLE_GET_PKTCNT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[4]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[7]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBPCTSINGLE_GET_BYTECNT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[0]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[1]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[2]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[3]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[4]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#endif
