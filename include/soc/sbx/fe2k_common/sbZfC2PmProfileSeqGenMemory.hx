/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_C2_PM_PROFSEQGENMEMORY_H
#define SB_ZF_C2_PM_PROFSEQGENMEMORY_H

#define SB_ZF_C2_PM_PROFSEQGENMEMORY_SIZE_IN_BYTES 12
#define SB_ZF_C2_PM_PROFSEQGENMEMORY_SIZE 12
#define SB_ZF_C2_PM_PROFSEQGENMEMORY_UTYPE_BITS "74:74"
#define SB_ZF_C2_PM_PROFSEQGENMEMORY_UPROFMODE_BITS "73:72"
#define SB_ZF_C2_PM_PROFSEQGENMEMORY_URESV0_BITS "71:64"
#define SB_ZF_C2_PM_PROFSEQGENMEMORY_URESV1_BITS "63:32"
#define SB_ZF_C2_PM_PROFSEQGENMEMORY_URESV2_BITS "31:0"


#define SB_ZF_C2_PM_PROFSEQGENMEMORY_SIZE_IN_WORDS   ((SB_ZF_C2_PM_PROFSEQGENMEMORY_SIZE_IN_BYTES+3)/4)

/** @brief  PM Profile Sequence Generator memory Configuration

  FOR INTERANL USE ONLY NOT FOR THE API USER
*/

typedef struct _sbZfC2PmProfileSeqGenMemory {
  uint32 uType;
  uint32 uProfMode;
  uint32 uResv0;
  uint32 uResv1;
  uint32 uResv2;
} sbZfC2PmProfileSeqGenMemory_t;

uint32
sbZfC2PmProfileSeqGenMemory_Pack(sbZfC2PmProfileSeqGenMemory_t *pFrom,
                                 uint8 *pToData,
                                 uint32 nMaxToDataIndex);
void
sbZfC2PmProfileSeqGenMemory_Unpack(sbZfC2PmProfileSeqGenMemory_t *pToStruct,
                                   uint8 *pFromData,
                                   uint32 nMaxToDataIndex);
void
sbZfC2PmProfileSeqGenMemory_InitInstance(sbZfC2PmProfileSeqGenMemory_t *pFrame);

#define SB_ZF_C2PMPROFILESEQGENMEMORY_SET_TYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_C2PMPROFILESEQGENMEMORY_SET_PROFMODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_C2PMPROFILESEQGENMEMORY_SET_RESV0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PMPROFILESEQGENMEMORY_SET_RESV1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PMPROFILESEQGENMEMORY_SET_RESV2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PMPROFILESEQGENMEMORY_GET_TYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILESEQGENMEMORY_GET_PROFMODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x03; \
          } while(0)

#define SB_ZF_C2PMPROFILESEQGENMEMORY_GET_RESV0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_C2PMPROFILESEQGENMEMORY_GET_RESV1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) (pFromData)[6] << 8; \
           (nToData) |= (uint32) (pFromData)[5] << 16; \
           (nToData) |= (uint32) (pFromData)[4] << 24; \
          } while(0)

#define SB_ZF_C2PMPROFILESEQGENMEMORY_GET_RESV2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[11] ; \
           (nToData) |= (uint32) (pFromData)[10] << 8; \
           (nToData) |= (uint32) (pFromData)[9] << 16; \
           (nToData) |= (uint32) (pFromData)[8] << 24; \
          } while(0)


/*
 * Return 1 if same else 0.
 */
uint8 sbZfC2PmProfileSeqGenMemory_Compare( sbZfC2PmProfileSeqGenMemory_t *pProfile1,
                                               sbZfC2PmProfileSeqGenMemory_t *pProfile2);

void sbZfC2PmProfileSeqGenMemory_Copy( sbZfC2PmProfileSeqGenMemory_t *pSource,
                                         sbZfC2PmProfileSeqGenMemory_t *pDest);

uint32 sbZfC2PmProfileSeqGenMemory_Pack32(sbZfC2PmProfileSeqGenMemory_t *pFrom, 
                                              uint32 *pToData, 
                                              uint32 nMaxToDataIndex);
#endif
