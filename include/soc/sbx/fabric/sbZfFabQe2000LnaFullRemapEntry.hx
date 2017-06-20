/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabQe2000LnaFullRemapEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_H
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_H

#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_SIZE_IN_BYTES 20
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_SIZE 20
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_M_NREMAP_BITS ":"


/**

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
 */
typedef struct _sbZfFabQe2000LnaFullRemapEntry {
  uint32  m_nRemap[25];
} sbZfFabQe2000LnaFullRemapEntry_t;

uint32
sbZfFabQe2000LnaFullRemapEntry_GetnRemap(sbZfFabQe2000LnaFullRemapEntry_t *pFrom, UINT nIndex);

void sbZfFabQe2000LnaFullRemapEntry_SetnRemap(sbZfFabQe2000LnaFullRemapEntry_t *pFrom, UINT nIndex, uint32 value);


uint32
sbZfFabQe2000LnaFullRemapEntry_Pack(sbZfFabQe2000LnaFullRemapEntry_t *pFrom,
                                    uint8 *pToData,
                                    uint32 nMaxToDataIndex);
void
sbZfFabQe2000LnaFullRemapEntry_Unpack(sbZfFabQe2000LnaFullRemapEntry_t *pToStruct,
                                      uint8 *pFromData,
                                      uint32 nMaxToDataIndex);
void
sbZfFabQe2000LnaFullRemapEntry_InitInstance(sbZfFabQe2000LnaFullRemapEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData)[0] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData)[1] & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData)[1] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData)[2] & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData)[2] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData)[3] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData)[4] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData)[5] & 0x03) << 6); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData)[5] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData)[6] & 0x0f) << 4); \
           (pToData)[6] = ((pToData)[6] & ~ 0x03) | (((nFromData)[6] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x3f << 2)) | (((nFromData)[7] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x3f) | ((nFromData)[8] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x03 << 6)) | (((nFromData)[9] & 0x03) << 6); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData)[9] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData)[10] & 0x0f) << 4); \
           (pToData)[11] = ((pToData)[11] & ~ 0x03) | (((nFromData)[10] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x3f << 2)) | (((nFromData)[11] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x3f) | ((nFromData)[12] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x03 << 6)) | (((nFromData)[13] & 0x03) << 6); \
           (pToData)[9] = ((pToData)[9] & ~ 0x0f) | (((nFromData)[13] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x0f << 4)) | (((nFromData)[14] & 0x0f) << 4); \
           (pToData)[8] = ((pToData)[8] & ~ 0x03) | (((nFromData)[14] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x3f << 2)) | (((nFromData)[15] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~0x3f) | ((nFromData)[16] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP17(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x03 << 6)) | (((nFromData)[17] & 0x03) << 6); \
           (pToData)[14] = ((pToData)[14] & ~ 0x0f) | (((nFromData)[17] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP18(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x0f << 4)) | (((nFromData)[18] & 0x0f) << 4); \
           (pToData)[13] = ((pToData)[13] & ~ 0x03) | (((nFromData)[18] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP19(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x3f << 2)) | (((nFromData)[19] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP20(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~0x3f) | ((nFromData)[20] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP21(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x03 << 6)) | (((nFromData)[21] & 0x03) << 6); \
           (pToData)[19] = ((pToData)[19] & ~ 0x0f) | (((nFromData)[21] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP22(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((pToData)[19] & ~(0x0f << 4)) | (((nFromData)[22] & 0x0f) << 4); \
           (pToData)[18] = ((pToData)[18] & ~ 0x03) | (((nFromData)[22] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP23(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((pToData)[18] & ~(0x3f << 2)) | (((nFromData)[23] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP24(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~0x3f) | ((nFromData)[24] & 0x3f); \
          } while(0)

#else
#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData)[0] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData)[1] & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData)[1] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData)[2] & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData)[2] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData)[3] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData)[4] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData)[5] & 0x03) << 6); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData)[5] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData)[6] & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~ 0x03) | (((nFromData)[6] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x3f << 2)) | (((nFromData)[7] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x3f) | ((nFromData)[8] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x03 << 6)) | (((nFromData)[9] & 0x03) << 6); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData)[9] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData)[10] & 0x0f) << 4); \
           (pToData)[8] = ((pToData)[8] & ~ 0x03) | (((nFromData)[10] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x3f << 2)) | (((nFromData)[11] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~0x3f) | ((nFromData)[12] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x03 << 6)) | (((nFromData)[13] & 0x03) << 6); \
           (pToData)[10] = ((pToData)[10] & ~ 0x0f) | (((nFromData)[13] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData)[14] & 0x0f) << 4); \
           (pToData)[11] = ((pToData)[11] & ~ 0x03) | (((nFromData)[14] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x3f << 2)) | (((nFromData)[15] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~0x3f) | ((nFromData)[16] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP17(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x03 << 6)) | (((nFromData)[17] & 0x03) << 6); \
           (pToData)[13] = ((pToData)[13] & ~ 0x0f) | (((nFromData)[17] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP18(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x0f << 4)) | (((nFromData)[18] & 0x0f) << 4); \
           (pToData)[14] = ((pToData)[14] & ~ 0x03) | (((nFromData)[18] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP19(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x3f << 2)) | (((nFromData)[19] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP20(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~0x3f) | ((nFromData)[20] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP21(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x03 << 6)) | (((nFromData)[21] & 0x03) << 6); \
           (pToData)[16] = ((pToData)[16] & ~ 0x0f) | (((nFromData)[21] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP22(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x0f << 4)) | (((nFromData)[22] & 0x0f) << 4); \
           (pToData)[17] = ((pToData)[17] & ~ 0x03) | (((nFromData)[22] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP23(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~(0x3f << 2)) | (((nFromData)[23] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP24(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((pToData)[18] & ~0x3f) | ((nFromData)[24] & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData)[0] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData)[1] & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData)[1] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData)[2] & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData)[2] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData)[3] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData)[4] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData)[5] & 0x03) << 6); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData)[5] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData)[6] & 0x0f) << 4); \
           (pToData)[6] = ((pToData)[6] & ~ 0x03) | (((nFromData)[6] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x3f << 2)) | (((nFromData)[7] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x3f) | ((nFromData)[8] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x03 << 6)) | (((nFromData)[9] & 0x03) << 6); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData)[9] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData)[10] & 0x0f) << 4); \
           (pToData)[11] = ((pToData)[11] & ~ 0x03) | (((nFromData)[10] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x3f << 2)) | (((nFromData)[11] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x3f) | ((nFromData)[12] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x03 << 6)) | (((nFromData)[13] & 0x03) << 6); \
           (pToData)[9] = ((pToData)[9] & ~ 0x0f) | (((nFromData)[13] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x0f << 4)) | (((nFromData)[14] & 0x0f) << 4); \
           (pToData)[8] = ((pToData)[8] & ~ 0x03) | (((nFromData)[14] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x3f << 2)) | (((nFromData)[15] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~0x3f) | ((nFromData)[16] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP17(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x03 << 6)) | (((nFromData)[17] & 0x03) << 6); \
           (pToData)[14] = ((pToData)[14] & ~ 0x0f) | (((nFromData)[17] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP18(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x0f << 4)) | (((nFromData)[18] & 0x0f) << 4); \
           (pToData)[13] = ((pToData)[13] & ~ 0x03) | (((nFromData)[18] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP19(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x3f << 2)) | (((nFromData)[19] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP20(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~0x3f) | ((nFromData)[20] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP21(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x03 << 6)) | (((nFromData)[21] & 0x03) << 6); \
           (pToData)[19] = ((pToData)[19] & ~ 0x0f) | (((nFromData)[21] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP22(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((pToData)[19] & ~(0x0f << 4)) | (((nFromData)[22] & 0x0f) << 4); \
           (pToData)[18] = ((pToData)[18] & ~ 0x03) | (((nFromData)[22] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP23(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((pToData)[18] & ~(0x3f << 2)) | (((nFromData)[23] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP24(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~0x3f) | ((nFromData)[24] & 0x3f); \
          } while(0)

#else
#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData)[0] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData)[1] & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData)[1] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData)[2] & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData)[2] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData)[3] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData)[4] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData)[5] & 0x03) << 6); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData)[5] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData)[6] & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~ 0x03) | (((nFromData)[6] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x3f << 2)) | (((nFromData)[7] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x3f) | ((nFromData)[8] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x03 << 6)) | (((nFromData)[9] & 0x03) << 6); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData)[9] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData)[10] & 0x0f) << 4); \
           (pToData)[8] = ((pToData)[8] & ~ 0x03) | (((nFromData)[10] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x3f << 2)) | (((nFromData)[11] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~0x3f) | ((nFromData)[12] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x03 << 6)) | (((nFromData)[13] & 0x03) << 6); \
           (pToData)[10] = ((pToData)[10] & ~ 0x0f) | (((nFromData)[13] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData)[14] & 0x0f) << 4); \
           (pToData)[11] = ((pToData)[11] & ~ 0x03) | (((nFromData)[14] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x3f << 2)) | (((nFromData)[15] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~0x3f) | ((nFromData)[16] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP17(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x03 << 6)) | (((nFromData)[17] & 0x03) << 6); \
           (pToData)[13] = ((pToData)[13] & ~ 0x0f) | (((nFromData)[17] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP18(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x0f << 4)) | (((nFromData)[18] & 0x0f) << 4); \
           (pToData)[14] = ((pToData)[14] & ~ 0x03) | (((nFromData)[18] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP19(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x3f << 2)) | (((nFromData)[19] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP20(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~0x3f) | ((nFromData)[20] & 0x3f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP21(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x03 << 6)) | (((nFromData)[21] & 0x03) << 6); \
           (pToData)[16] = ((pToData)[16] & ~ 0x0f) | (((nFromData)[21] >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP22(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x0f << 4)) | (((nFromData)[22] & 0x0f) << 4); \
           (pToData)[17] = ((pToData)[17] & ~ 0x03) | (((nFromData)[22] >> 4) & 0x03); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP23(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~(0x3f << 2)) | (((nFromData)[23] & 0x3f) << 2); \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_SET_REMAP24(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((pToData)[18] & ~0x3f) | ((nFromData)[24] & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[0] = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[1] = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData)[1] |= (uint32) ((pFromData)[2] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[2] = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData)[2] |= (uint32) ((pFromData)[1] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[3] = (uint32) ((pFromData)[1] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[4] = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[5] = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData)[5] |= (uint32) ((pFromData)[7] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[6] = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData)[6] |= (uint32) ((pFromData)[6] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[7] = (uint32) ((pFromData)[6] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[8] = (uint32) ((pFromData)[5]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[9] = (uint32) ((pFromData)[5] >> 6) & 0x03; \
           (nToData)[9] |= (uint32) ((pFromData)[4] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[10] = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData)[10] |= (uint32) ((pFromData)[11] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[11] = (uint32) ((pFromData)[11] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[12] = (uint32) ((pFromData)[10]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[13] = (uint32) ((pFromData)[10] >> 6) & 0x03; \
           (nToData)[13] |= (uint32) ((pFromData)[9] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[14] = (uint32) ((pFromData)[9] >> 4) & 0x0f; \
           (nToData)[14] |= (uint32) ((pFromData)[8] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[15] = (uint32) ((pFromData)[8] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[16] = (uint32) ((pFromData)[15]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP17(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[17] = (uint32) ((pFromData)[15] >> 6) & 0x03; \
           (nToData)[17] |= (uint32) ((pFromData)[14] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP18(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[18] = (uint32) ((pFromData)[14] >> 4) & 0x0f; \
           (nToData)[18] |= (uint32) ((pFromData)[13] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP19(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[19] = (uint32) ((pFromData)[13] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP20(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[20] = (uint32) ((pFromData)[12]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP21(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[21] = (uint32) ((pFromData)[12] >> 6) & 0x03; \
           (nToData)[21] |= (uint32) ((pFromData)[19] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP22(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[22] = (uint32) ((pFromData)[19] >> 4) & 0x0f; \
           (nToData)[22] |= (uint32) ((pFromData)[18] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP23(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[23] = (uint32) ((pFromData)[18] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP24(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[24] = (uint32) ((pFromData)[17]) & 0x3f; \
          } while(0)

#else
#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[0] = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[1] = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData)[1] |= (uint32) ((pFromData)[1] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[2] = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData)[2] |= (uint32) ((pFromData)[2] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[3] = (uint32) ((pFromData)[2] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[4] = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[5] = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData)[5] |= (uint32) ((pFromData)[4] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[6] = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData)[6] |= (uint32) ((pFromData)[5] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[7] = (uint32) ((pFromData)[5] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[8] = (uint32) ((pFromData)[6]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[9] = (uint32) ((pFromData)[6] >> 6) & 0x03; \
           (nToData)[9] |= (uint32) ((pFromData)[7] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[10] = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData)[10] |= (uint32) ((pFromData)[8] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[11] = (uint32) ((pFromData)[8] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[12] = (uint32) ((pFromData)[9]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[13] = (uint32) ((pFromData)[9] >> 6) & 0x03; \
           (nToData)[13] |= (uint32) ((pFromData)[10] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[14] = (uint32) ((pFromData)[10] >> 4) & 0x0f; \
           (nToData)[14] |= (uint32) ((pFromData)[11] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[15] = (uint32) ((pFromData)[11] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[16] = (uint32) ((pFromData)[12]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP17(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[17] = (uint32) ((pFromData)[12] >> 6) & 0x03; \
           (nToData)[17] |= (uint32) ((pFromData)[13] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP18(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[18] = (uint32) ((pFromData)[13] >> 4) & 0x0f; \
           (nToData)[18] |= (uint32) ((pFromData)[14] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP19(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[19] = (uint32) ((pFromData)[14] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP20(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[20] = (uint32) ((pFromData)[15]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP21(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[21] = (uint32) ((pFromData)[15] >> 6) & 0x03; \
           (nToData)[21] |= (uint32) ((pFromData)[16] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP22(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[22] = (uint32) ((pFromData)[16] >> 4) & 0x0f; \
           (nToData)[22] |= (uint32) ((pFromData)[17] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP23(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[23] = (uint32) ((pFromData)[17] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP24(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[24] = (uint32) ((pFromData)[18]) & 0x3f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[0] = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[1] = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData)[1] |= (uint32) ((pFromData)[2] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[2] = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData)[2] |= (uint32) ((pFromData)[1] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[3] = (uint32) ((pFromData)[1] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[4] = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[5] = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData)[5] |= (uint32) ((pFromData)[7] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[6] = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData)[6] |= (uint32) ((pFromData)[6] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[7] = (uint32) ((pFromData)[6] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[8] = (uint32) ((pFromData)[5]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[9] = (uint32) ((pFromData)[5] >> 6) & 0x03; \
           (nToData)[9] |= (uint32) ((pFromData)[4] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[10] = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData)[10] |= (uint32) ((pFromData)[11] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[11] = (uint32) ((pFromData)[11] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[12] = (uint32) ((pFromData)[10]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[13] = (uint32) ((pFromData)[10] >> 6) & 0x03; \
           (nToData)[13] |= (uint32) ((pFromData)[9] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[14] = (uint32) ((pFromData)[9] >> 4) & 0x0f; \
           (nToData)[14] |= (uint32) ((pFromData)[8] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[15] = (uint32) ((pFromData)[8] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[16] = (uint32) ((pFromData)[15]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP17(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[17] = (uint32) ((pFromData)[15] >> 6) & 0x03; \
           (nToData)[17] |= (uint32) ((pFromData)[14] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP18(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[18] = (uint32) ((pFromData)[14] >> 4) & 0x0f; \
           (nToData)[18] |= (uint32) ((pFromData)[13] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP19(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[19] = (uint32) ((pFromData)[13] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP20(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[20] = (uint32) ((pFromData)[12]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP21(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[21] = (uint32) ((pFromData)[12] >> 6) & 0x03; \
           (nToData)[21] |= (uint32) ((pFromData)[19] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP22(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[22] = (uint32) ((pFromData)[19] >> 4) & 0x0f; \
           (nToData)[22] |= (uint32) ((pFromData)[18] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP23(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[23] = (uint32) ((pFromData)[18] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP24(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[24] = (uint32) ((pFromData)[17]) & 0x3f; \
          } while(0)

#else
#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[0] = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[1] = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData)[1] |= (uint32) ((pFromData)[1] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[2] = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData)[2] |= (uint32) ((pFromData)[2] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[3] = (uint32) ((pFromData)[2] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[4] = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[5] = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData)[5] |= (uint32) ((pFromData)[4] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[6] = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData)[6] |= (uint32) ((pFromData)[5] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[7] = (uint32) ((pFromData)[5] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[8] = (uint32) ((pFromData)[6]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[9] = (uint32) ((pFromData)[6] >> 6) & 0x03; \
           (nToData)[9] |= (uint32) ((pFromData)[7] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[10] = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData)[10] |= (uint32) ((pFromData)[8] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[11] = (uint32) ((pFromData)[8] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[12] = (uint32) ((pFromData)[9]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[13] = (uint32) ((pFromData)[9] >> 6) & 0x03; \
           (nToData)[13] |= (uint32) ((pFromData)[10] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[14] = (uint32) ((pFromData)[10] >> 4) & 0x0f; \
           (nToData)[14] |= (uint32) ((pFromData)[11] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[15] = (uint32) ((pFromData)[11] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[16] = (uint32) ((pFromData)[12]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP17(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[17] = (uint32) ((pFromData)[12] >> 6) & 0x03; \
           (nToData)[17] |= (uint32) ((pFromData)[13] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP18(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[18] = (uint32) ((pFromData)[13] >> 4) & 0x0f; \
           (nToData)[18] |= (uint32) ((pFromData)[14] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP19(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[19] = (uint32) ((pFromData)[14] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP20(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[20] = (uint32) ((pFromData)[15]) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP21(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[21] = (uint32) ((pFromData)[15] >> 6) & 0x03; \
           (nToData)[21] |= (uint32) ((pFromData)[16] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP22(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[22] = (uint32) ((pFromData)[16] >> 4) & 0x0f; \
           (nToData)[22] |= (uint32) ((pFromData)[17] & 0x03) << 4; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP23(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[23] = (uint32) ((pFromData)[17] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_FABQE2000LNAFULLREMAPENTRY_GET_REMAP24(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData)[24] = (uint32) ((pFromData)[18]) & 0x3f; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabQe2000LnaFullRemapEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_CONSOLE_H
#define SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_CONSOLE_H



void
sbZfFabQe2000LnaFullRemapEntry_Print(sbZfFabQe2000LnaFullRemapEntry_t *pFromStruct);
int
sbZfFabQe2000LnaFullRemapEntry_SPrint(sbZfFabQe2000LnaFullRemapEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabQe2000LnaFullRemapEntry_Validate(sbZfFabQe2000LnaFullRemapEntry_t *pZf);
int
sbZfFabQe2000LnaFullRemapEntry_SetField(sbZfFabQe2000LnaFullRemapEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_QE2000_LNA_FULL_REMAP_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

