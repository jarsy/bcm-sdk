/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FE_2000_PM_PROFSEQGENMEMORY_H
#define SB_ZF_FE_2000_PM_PROFSEQGENMEMORY_H

#define SB_ZF_FE_2000_PM_PROFSEQGENMEMORY_SIZE_IN_BYTES 8
#define SB_ZF_FE_2000_PM_PROFSEQGENMEMORY_SIZE 8
#define SB_ZF_FE_2000_PM_PROFSEQGENMEMORY_URESV0_BITS "63:63"
#define SB_ZF_FE_2000_PM_PROFSEQGENMEMORY_UTYPE_BITS "62:62"
#define SB_ZF_FE_2000_PM_PROFSEQGENMEMORY_UPROFMODE_BITS "61:60"
#define SB_ZF_FE_2000_PM_PROFSEQGENMEMORY_URESV1_BITS "59:32"
#define SB_ZF_FE_2000_PM_PROFSEQGENMEMORY_URESV2_BITS "31:0"


#define SB_ZF_FE_2000_PM_PROFSEQGENMEMORY_SIZE_IN_WORDS   ((SB_ZF_FE_2000_PM_PROFSEQGENMEMORY_SIZE_IN_BYTES+3)/4)

/** @brief  PM Profile Sequence Generator memory Configuration

  FOR INTERANL USE ONLY NOT FOR THE API USER
*/

typedef struct _sbZfFe2000PmProfileSeqGenMemory {
  uint32 uResv0;
  uint32 uType;
  uint32 uProfMode;
  uint32 uResv1;
  uint32 uResv2;
} sbZfFe2000PmProfileSeqGenMemory_t;

uint32
sbZfFe2000PmProfileSeqGenMemory_Pack(sbZfFe2000PmProfileSeqGenMemory_t *pFrom,
                                     uint8 *pToData,
                                     uint32 nMaxToDataIndex);
void
sbZfFe2000PmProfileSeqGenMemory_Unpack(sbZfFe2000PmProfileSeqGenMemory_t *pToStruct,
                                       uint8 *pFromData,
                                       uint32 nMaxToDataIndex);
void
sbZfFe2000PmProfileSeqGenMemory_InitInstance(sbZfFe2000PmProfileSeqGenMemory_t *pFrame);

#define SB_ZF_FE2000PMPROFILESEQGENMEMORY_SET_RESV0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FE2000PMPROFILESEQGENMEMORY_SET_TYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FE2000PMPROFILESEQGENMEMORY_SET_PROFMODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_FE2000PMPROFILESEQGENMEMORY_SET_RESV1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~ 0x0f) | (((nFromData) >> 24) & 0x0f); \
          } while(0)

#define SB_ZF_FE2000PMPROFILESEQGENMEMORY_SET_RESV2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000PMPROFILESEQGENMEMORY_GET_RESV0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPROFILESEQGENMEMORY_GET_TYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPROFILESEQGENMEMORY_GET_PROFMODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_FE2000PMPROFILESEQGENMEMORY_GET_RESV1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x0f) << 24; \
          } while(0)

#define SB_ZF_FE2000PMPROFILESEQGENMEMORY_GET_RESV2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) (pFromData)[6] << 8; \
           (nToData) |= (uint32) (pFromData)[5] << 16; \
           (nToData) |= (uint32) (pFromData)[4] << 24; \
          } while(0)


/*
 * Return 1 if same else 0.
 */
uint8 sbZfFe2000PmProfileSeqGenMemory_Compare( sbZfFe2000PmProfileSeqGenMemory_t *pProfile1,
                                               sbZfFe2000PmProfileSeqGenMemory_t *pProfile2);

void sbZfFe2000PmProfileSeqGenMemory_Copy( sbZfFe2000PmProfileSeqGenMemory_t *pSource,
                                         sbZfFe2000PmProfileSeqGenMemory_t *pDest);

uint32 sbZfFe2000PmProfileSeqGenMemory_Pack32(sbZfFe2000PmProfileSeqGenMemory_t *pFrom, 
                                              uint32 *pToData, 
                                              uint32 nMaxToDataIndex);
#endif
