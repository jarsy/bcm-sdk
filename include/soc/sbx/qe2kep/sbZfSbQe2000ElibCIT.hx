/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfSbQe2000ElibCIT.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_H
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_H

#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_SIZE_IN_BYTES 32
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_SIZE 32
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_M_INSTRUCTION7_BITS "255:224"
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_M_INSTRUCTION6_BITS "223:192"
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_M_INSTRUCTION5_BITS "191:160"
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_M_INSTRUCTION4_BITS "159:128"
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_M_INSTRUCTION3_BITS "127:96"
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_M_INSTRUCTION2_BITS "95:64"
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_M_INSTRUCTION1_BITS "63:32"
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_M_INSTRUCTION0_BITS "31:0"
#define SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_OFFSET         0x040 /**< Offset into the classifier memory for the CIT */


/**
 * @file sbZfSbQe2000ElibCIT.h
 *
 * <pre>
 *
 * ==============================================================
 * ==  sbZfSbQe2000ElibCIT.h - Class Instruction Table ZFrame  ==
 * ==============================================================
 *
 * WORKING REVISION: $Id: sbZfSbQe2000ElibCIT.hx,v 1.4 Broadcom SDK $
 *
 * Copyright (c) Sandburst, Inc. 2004
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
 *     sbZfSbQe2000ElibCIT.h
 *
 * ABSTRACT:
 *
 *     Class Instruction Table ZFrame Definition.
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
 *     30-November-2004
 * </pre>
 */
typedef struct _sbZfSbQe2000ElibCIT {
  int32 m_Instruction7;
  int32 m_Instruction6;
  int32 m_Instruction5;
  int32 m_Instruction4;
  int32 m_Instruction3;
  int32 m_Instruction2;
  int32 m_Instruction1;
  int32 m_Instruction0;
} sbZfSbQe2000ElibCIT_t;

uint32
sbZfSbQe2000ElibCIT_Pack(sbZfSbQe2000ElibCIT_t *pFrom,
                         uint8 *pToData,
                         uint32 nMaxToDataIndex);
void
sbZfSbQe2000ElibCIT_Unpack(sbZfSbQe2000ElibCIT_t *pToStruct,
                           uint8 *pFromData,
                           uint32 nMaxToDataIndex);
void
sbZfSbQe2000ElibCIT_InitInstance(sbZfSbQe2000ElibCIT_t *pFrame);

#define SB_ZF_SBQE2000ELIBCIT_SET_INST7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[28] = ((nFromData)) & 0xFF; \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_SET_INST6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[24] = ((nFromData)) & 0xFF; \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_SET_INST5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[20] = ((nFromData)) & 0xFF; \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_SET_INST4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_SET_INST3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_SET_INST2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_SET_INST1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_SET_INST0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_GET_INST7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[28] ; \
           (nToData) |= (int32) (pFromData)[29] << 8; \
           (nToData) |= (int32) (pFromData)[30] << 16; \
           (nToData) |= (int32) (pFromData)[31] << 24; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_GET_INST6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[24] ; \
           (nToData) |= (int32) (pFromData)[25] << 8; \
           (nToData) |= (int32) (pFromData)[26] << 16; \
           (nToData) |= (int32) (pFromData)[27] << 24; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_GET_INST5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[20] ; \
           (nToData) |= (int32) (pFromData)[21] << 8; \
           (nToData) |= (int32) (pFromData)[22] << 16; \
           (nToData) |= (int32) (pFromData)[23] << 24; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_GET_INST4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[16] ; \
           (nToData) |= (int32) (pFromData)[17] << 8; \
           (nToData) |= (int32) (pFromData)[18] << 16; \
           (nToData) |= (int32) (pFromData)[19] << 24; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_GET_INST3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[12] ; \
           (nToData) |= (int32) (pFromData)[13] << 8; \
           (nToData) |= (int32) (pFromData)[14] << 16; \
           (nToData) |= (int32) (pFromData)[15] << 24; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_GET_INST2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[8] ; \
           (nToData) |= (int32) (pFromData)[9] << 8; \
           (nToData) |= (int32) (pFromData)[10] << 16; \
           (nToData) |= (int32) (pFromData)[11] << 24; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_GET_INST1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[4] ; \
           (nToData) |= (int32) (pFromData)[5] << 8; \
           (nToData) |= (int32) (pFromData)[6] << 16; \
           (nToData) |= (int32) (pFromData)[7] << 24; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCIT_GET_INST0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) (pFromData)[0] ; \
           (nToData) |= (int32) (pFromData)[1] << 8; \
           (nToData) |= (int32) (pFromData)[2] << 16; \
           (nToData) |= (int32) (pFromData)[3] << 24; \
          } while(0)

#endif
