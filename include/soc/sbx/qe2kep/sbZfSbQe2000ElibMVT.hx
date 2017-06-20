/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfSbQe2000ElibMVT.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_SB_QE2000_ELIB_MVT_ENTRY_H
#define SB_ZF_SB_QE2000_ELIB_MVT_ENTRY_H

#define SB_ZF_SB_QE2000_ELIB_MVT_ENTRY_SIZE_IN_BYTES 12
#define SB_ZF_SB_QE2000_ELIB_MVT_ENTRY_SIZE 12
#define SB_ZF_SB_QE2000_ELIB_MVT_ENTRY_M_NRESERVED_BITS "87:85"
#define SB_ZF_SB_QE2000_ELIB_MVT_ENTRY_M_NNPORTMAP_BITS "84:35"
#define SB_ZF_SB_QE2000_ELIB_MVT_ENTRY_M_NMVTDA_BITS "34:21"
#define SB_ZF_SB_QE2000_ELIB_MVT_ENTRY_M_NMVTDB_BITS "20:17"
#define SB_ZF_SB_QE2000_ELIB_MVT_ENTRY_M_NNEXT_BITS "16:1"
#define SB_ZF_SB_QE2000_ELIB_MVT_ENTRY_M_NKNOCKOUT_BITS "0:0"


/**
 * @file sbZfSbQe2000ElibMVT.h
 *
 * <pre>
 *
 * ==========================================================================
 * ==  sbZfSbQe2000ElibMVT.h - Multicast Vector Table Entry ZFrame ==
 * ==========================================================================
 *
 * WORKING REVISION: $Id: sbZfSbQe2000ElibMVT.hx,v 1.4 Broadcom SDK $
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
 *     sbZfSbQe2000ElibMVT.h
 *
 * ABSTRACT:
 *
 *     Multicast Vector Table Entry ZFrame Definition.
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
 *     15-February-2005
 * </pre>
 */
typedef struct _sbZfSbQe2000ElibMVT {
  uint32 m_nReserved;
  uint64 m_nnPortMap;
  uint32 m_nMvtda;
  uint32 m_nMvtdb;
  uint32 m_nNext;
  uint32 m_nKnockout;
} sbZfSbQe2000ElibMVT_t;

uint32
sbZfSbQe2000ElibMVT_Pack(sbZfSbQe2000ElibMVT_t *pFrom,
                         uint8 *pToData,
                         uint32 nMaxToDataIndex);
void
sbZfSbQe2000ElibMVT_Unpack(sbZfSbQe2000ElibMVT_t *pToStruct,
                           uint8 *pFromData,
                           uint32 nMaxToDataIndex);
void
sbZfSbQe2000ElibMVT_InitInstance(sbZfSbQe2000ElibMVT_t *pFrame);

#define SB_ZF_SBQE2000ELIBMVT_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_SBQE2000ELIBMVT_SET_PORT_MAP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 29) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 37) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~ 0x1f) | (((nFromData) >> 45) & 0x1f); \
          } while(0)

#define SB_ZF_SBQE2000ELIBMVT_SET_MVTD_A(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x07) | (((nFromData) >> 11) & 0x07); \
          } while(0)

#define SB_ZF_SBQE2000ELIBMVT_SET_MVTD_B(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_SBQE2000ELIBMVT_SET_NEXT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 7) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 15) & 0x01); \
          } while(0)

#define SB_ZF_SBQE2000ELIBMVT_SET_KNOCKOUT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_SBQE2000ELIBMVT_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_SBQE2000ELIBMVT_GET_PORT_MAP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[4]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[7]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_SBQE2000ELIBMVT_GET_MVTD_A(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[3] << 3; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x07) << 11; \
          } while(0)

#define SB_ZF_SBQE2000ELIBMVT_GET_MVTD_B(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_SBQE2000ELIBMVT_GET_NEXT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 1) & 0x7f; \
           (nToData) |= (uint32) (pFromData)[1] << 7; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x01) << 15; \
          } while(0)

#define SB_ZF_SBQE2000ELIBMVT_GET_KNOCKOUT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
