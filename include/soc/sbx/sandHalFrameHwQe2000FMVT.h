/*
 * $Id: sandHalFrameHwQe2000FMVT.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef SAND_HAL_FRAME_HW_QE2000_FMVT_ENTRY_H
#define SAND_HAL_FRAME_HW_QE2000_FMVT_ENTRY_H

#include <soc/sbx/sbTypesGlue.h>

#define SAND_HAL_FRAME_HW_QE2000_FMVT_ENTRY_SIZE_IN_BYTES 32
#define SAND_HAL_FRAME_HW_QE2000_FMVT_ENTRY_SIZE 32


typedef struct _sandHalFrameHwQe2000FMVT {
  uint32 m_nReserved;
  uint64 m_nnPortMap2;
  uint32 m_nMvtda2;
  uint32 m_nMvtdb2;
  uint32 m_nNext2;
  uint32 m_nKnockout2;
  uint64 m_nnPortMap1;
  uint32 m_nMvtda1;
  uint32 m_nMvtdb1;
  uint32 m_nNext1;
  uint32 m_nKnockout1;
  uint64 m_nnPortMap0;
  uint32 m_nMvtda0;
  uint32 m_nMvtdb0;
  uint32 m_nNext0;
  uint32 m_nKnockout0;
} sandHalFrameHwQe2000FMVT_t;

uint32
sandHalFrameHwQe2000FMVT_Pack(sandHalFrameHwQe2000FMVT_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex);
void
sandHalFrameHwQe2000FMVT_Unpack(sandHalFrameHwQe2000FMVT_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex);
void
sandHalFrameHwQe2000FMVT_InitInstance(sandHalFrameHwQe2000FMVT_t *pFrame);

#define SAND_ZF_HWQE2000FMVT_SET_RSVD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[31] = ((pToData)[31] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_SET_PORT_MAP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[25] = ((pToData)[25] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
           (pToData)[28] = ((pToData)[28] & ~0xFF) | (((nFromData) >> 19) & 0xFF); \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 27) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 35) & 0xFF); \
           (pToData)[31] = ((pToData)[31] & ~ 0x7f) | (((nFromData) >> 43) & 0x7f); \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_SET_MVTD_A2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[23] = ((pToData)[23] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[24] = ((pToData)[24] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[25] = ((pToData)[25] & ~ 0x1f) | (((nFromData) >> 9) & 0x1f); \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_SET_MVTD_B2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[23] = ((pToData)[23] & ~(0x0f << 3)) | (((nFromData) & 0x0f) << 3); \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_SET_NEXT2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((pToData)[21] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~ 0x07) | (((nFromData) >> 13) & 0x07); \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_SET_KNOCKOUT2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((pToData)[21] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_SET_PORT_MAP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~ 0x03) | (((nFromData) >> 48) & 0x03); \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_SET_MVTD_A1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_SET_MVTD_B1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[13] = ((pToData)[13] & ~ 0x03) | (((nFromData) >> 2) & 0x03); \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_SET_NEXT1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~ 0x3f) | (((nFromData) >> 10) & 0x3f); \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_SET_KNOCKOUT1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_SET_PORT_MAP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 21) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 29) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 37) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~ 0x1f) | (((nFromData) >> 45) & 0x1f); \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_SET_MVTD_A0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x07) | (((nFromData) >> 11) & 0x07); \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_SET_MVTD_B0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_SET_NEXT0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 7) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 15) & 0x01); \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_SET_KNOCKOUT0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_GET_RSVD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[31] >> 7) & 0x01; \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_GET_PORT_MAP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint64) ((pFromData)[25] >> 5) & 0x07; \
           (nToData) |= (uint64) (pFromData)[26] << 3; \
           (nToData) |= (uint64) (pFromData)[27] << 11; \
           (nToData) |= (uint64) (pFromData)[28] << 19; \
           (nToData) |= (uint64) (pFromData)[29] << 27; \
           (nToData) |= (uint64) (pFromData)[30] << 35; \
           (nToData) |= (uint64) ((pFromData)[31] & 0x7f) << 43; \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_GET_MVTD_A2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[23] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[24] << 1; \
           (nToData) |= (uint32) ((pFromData)[25] & 0x1f) << 9; \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_GET_MVTD_B2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[23] >> 3) & 0x0f; \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_GET_NEXT2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[21] >> 3) & 0x1f; \
           (nToData) |= (uint32) (pFromData)[22] << 5; \
           (nToData) |= (uint32) ((pFromData)[23] & 0x07) << 13; \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_GET_KNOCKOUT2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[21] >> 2) & 0x01; \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_GET_PORT_MAP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint64) (pFromData)[15] ; \
           (nToData) |= (uint64) (pFromData)[16] << 8; \
           (nToData) |= (uint64) (pFromData)[17] << 16; \
           (nToData) |= (uint64) (pFromData)[18] << 24; \
           (nToData) |= (uint64) (pFromData)[19] << 32; \
           (nToData) |= (uint64) (pFromData)[20] << 40; \
           (nToData) |= (uint64) ((pFromData)[21] & 0x03) << 48; \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_GET_MVTD_A1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[13] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[14] << 6; \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_GET_MVTD_B1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[13] & 0x03) << 2; \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_GET_NEXT1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[11] << 2; \
           (nToData) |= (uint32) ((pFromData)[12] & 0x3f) << 10; \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_GET_KNOCKOUT1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 5) & 0x01; \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_GET_PORT_MAP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint64) ((pFromData)[4] >> 3) & 0x1f; \
           (nToData) |= (uint64) (pFromData)[5] << 5; \
           (nToData) |= (uint64) (pFromData)[6] << 13; \
           (nToData) |= (uint64) (pFromData)[7] << 21; \
           (nToData) |= (uint64) (pFromData)[8] << 29; \
           (nToData) |= (uint64) (pFromData)[9] << 37; \
           (nToData) |= (uint64) ((pFromData)[10] & 0x1f) << 45; \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_GET_MVTD_A0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[3] << 3; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x07) << 11; \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_GET_MVTD_B0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x0f; \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_GET_NEXT0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 1) & 0x7f; \
           (nToData) |= (uint32) (pFromData)[1] << 7; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x01) << 15; \
          } while(0)

#define SAND_ZF_HWQE2000FMVT_GET_KNOCKOUT0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

/**
 * @file hwQe2000FMVT.zf
 *
 * <pre>
 *
 * ===========================================================================
 * ==  hwQe2000FMVT.h - Multicast Vector Table Entry ZFrame ==
 * ===========================================================================
 *
 * WORKING REVISION: $Id
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
 *     hwQe2000FMVT.h
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
 *     Smith
 *
 * CREATION DATE:
 *
 *     8/11/05
 *
 */
#endif
