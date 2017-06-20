/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_C2_PM_PROFMEMORY_H
#define SB_ZF_C2_PM_PROFMEMORY_H

#define SB_ZF_C2_PM_PROFMEMORY_SIZE_IN_BYTES 12
#define SB_ZF_C2_PM_PROFMEMORY_SIZE 12
#define SB_ZF_C2_PM_PROFMEMORY_UPROFTYPE_BITS "74:74"
#define SB_ZF_C2_PM_PROFMEMORY_UOAMPROFTYPE_BITS "73:72"
#define SB_ZF_C2_PM_PROFMEMORY_URESV0_BITS "71:64"
#define SB_ZF_C2_PM_PROFMEMORY_URESV1_BITS "63:32"
#define SB_ZF_C2_PM_PROFMEMORY_URESV2_BITS "31:0"


#define SB_ZF_C2_PM_PROFMEMORY_SIZE_IN_WORDS   ((SB_ZF_C2_PM_PROFMEMORY_SIZE_IN_BYTES+3)/4)


/** @brief  PM Profile memory Configuration

  FOR INTERANL USE ONLY NOT FOR THE API USER
*/

typedef struct _sbZfC2PmProfileMemory {
  uint32 uProfType;
  uint32 uOamProfType;
  uint32 uResv0;
  uint32 uResv1;
  uint32 uResv2;
} sbZfC2PmProfileMemory_t;

uint32
sbZfC2PmProfileMemory_Pack(sbZfC2PmProfileMemory_t *pFrom,
                           uint8 *pToData,
                           uint32 nMaxToDataIndex);
void
sbZfC2PmProfileMemory_Unpack(sbZfC2PmProfileMemory_t *pToStruct,
                             uint8 *pFromData,
                             uint32 nMaxToDataIndex);
void
sbZfC2PmProfileMemory_InitInstance(sbZfC2PmProfileMemory_t *pFrame);

#define SB_ZF_C2PMPROFILEMEMORY_SET_PROFTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_C2PMPROFILEMEMORY_SET_OAMPROFTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_C2PMPROFILEMEMORY_SET_RESV0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PMPROFILEMEMORY_SET_RESV1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PMPROFILEMEMORY_SET_RESV2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PMPROFILEMEMORY_GET_PROFTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILEMEMORY_GET_OAMPROFTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x03; \
          } while(0)

#define SB_ZF_C2PMPROFILEMEMORY_GET_RESV0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_C2PMPROFILEMEMORY_GET_RESV1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) (pFromData)[6] << 8; \
           (nToData) |= (uint32) (pFromData)[5] << 16; \
           (nToData) |= (uint32) (pFromData)[4] << 24; \
          } while(0)

#define SB_ZF_C2PMPROFILEMEMORY_GET_RESV2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[11] ; \
           (nToData) |= (uint32) (pFromData)[10] << 8; \
           (nToData) |= (uint32) (pFromData)[9] << 16; \
           (nToData) |= (uint32) (pFromData)[8] << 24; \
          } while(0)

void sbZfC2PmProfileMemory_Copy( sbZfC2PmProfileMemory_t *pSource,
                                   sbZfC2PmProfileMemory_t *pDest);

uint32 sbZfC2PmProfileMemory_Pack32(sbZfC2PmProfileMemory_t *pFrom, 
                                    uint32 *pToData, 
                                    uint32 nMaxToDataIndex);
uint32 sbZfC2PmProfileMemory_Unpack32(sbZfC2PmProfileMemory_t *pToData,
                                          uint32 *pFrom,
                                          uint32 nMaxToDataIndex);
#endif
