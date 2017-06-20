/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfSbQe2000ElibCRT.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_H
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_H

#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_SIZE_IN_BYTES 8
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_SIZE 8
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_M_NCLASS15_BITS "63:60"
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_M_NCLASS14_BITS "59:56"
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_M_NCLASS13_BITS "55:52"
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_M_NCLASS12_BITS "51:48"
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_M_NCLASS11_BITS "47:44"
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_M_NCLASS10_BITS "43:40"
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_M_NCLASS9_BITS "39:36"
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_M_NCLASS8_BITS "35:32"
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_M_NCLASS7_BITS "31:28"
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_M_NCLASS6_BITS "27:24"
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_M_NCLASS5_BITS "23:20"
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_M_NCLASS4_BITS "19:16"
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_M_NCLASS3_BITS "15:12"
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_M_NCLASS2_BITS "11:8"
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_M_NCLASS1_BITS "7:4"
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_M_NCLASS0_BITS "3:0"
#define SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_OFFSET         0x000 /**< Offset into the classifier memory for the CRT */


/**
 * @file sbZfSbQe2000ElibCRT.h
 *
 * <pre>
 *
 * =====================================================================
 * ==  sbZfSbQe2000ElibCRT.h - Class Resolution Table ZFrame  ==
 * =====================================================================
 *
 * WORKING REVISION: $Id: sbZfSbQe2000ElibCRT.hx,v 1.4 Broadcom SDK $
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
 *     sbZfSbQe2000ElibCRT.h
 *
 * ABSTRACT:
 *
 *     Class Resolution Table ZFrame Definition.
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
typedef struct _sbZfSbQe2000ElibCRT {
  uint32 m_nClass15;
  uint32 m_nClass14;
  uint32 m_nClass13;
  uint32 m_nClass12;
  uint32 m_nClass11;
  uint32 m_nClass10;
  uint32 m_nClass9;
  uint32 m_nClass8;
  uint32 m_nClass7;
  uint32 m_nClass6;
  uint32 m_nClass5;
  uint32 m_nClass4;
  uint32 m_nClass3;
  uint32 m_nClass2;
  uint32 m_nClass1;
  uint32 m_nClass0;
} sbZfSbQe2000ElibCRT_t;

uint32
sbZfSbQe2000ElibCRT_Pack(sbZfSbQe2000ElibCRT_t *pFrom,
                         uint8 *pToData,
                         uint32 nMaxToDataIndex);
void
sbZfSbQe2000ElibCRT_Unpack(sbZfSbQe2000ElibCRT_t *pToStruct,
                           uint8 *pFromData,
                           uint32 nMaxToDataIndex);
void
sbZfSbQe2000ElibCRT_InitInstance(sbZfSbQe2000ElibCRT_t *pFrame);

#define SB_ZF_SBQE2000ELIBCRT_SET_C15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_SET_C14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_SET_C13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_SET_C12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_SET_C11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_SET_C10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_SET_C09(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_SET_C08(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_SET_C07(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_SET_C06(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_SET_C05(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_SET_C04(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_SET_C03(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_SET_C02(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_SET_C01(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_SET_C00(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_GET_C15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_GET_C14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7]) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_GET_C13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_GET_C12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_GET_C11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_GET_C10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_GET_C09(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_GET_C08(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4]) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_GET_C07(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_GET_C06(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_GET_C05(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_GET_C04(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_GET_C03(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_GET_C02(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_GET_C01(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBCRT_GET_C00(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
