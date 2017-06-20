/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_C2_PM_PROFTIMERMEMORY_H
#define SB_ZF_C2_PM_PROFTIMERMEMORY_H

#define SB_ZF_C2_PM_PROFTIMERMEMORY_SIZE_IN_BYTES 12
#define SB_ZF_C2_PM_PROFTIMERMEMORY_SIZE 12
#define SB_ZF_C2_PM_PROFTIMERMEMORY_URESV0_BITS "95:75"
#define SB_ZF_C2_PM_PROFTIMERMEMORY_UTYPE_BITS "74:74"
#define SB_ZF_C2_PM_PROFTIMERMEMORY_UPROFMODE_BITS "73:72"
#define SB_ZF_C2_PM_PROFTIMERMEMORY_URESV1_BITS "71:64"
#define SB_ZF_C2_PM_PROFTIMERMEMORY_URESV2_BITS "63:32"
#define SB_ZF_C2_PM_PROFTIMERMEMORY_URESV3_BITS "31:24"
#define SB_ZF_C2_PM_PROFTIMERMEMORY_BINTERRUPT_BITS "23:23"
#define SB_ZF_C2_PM_PROFTIMERMEMORY_BRESET_BITS "22:22"
#define SB_ZF_C2_PM_PROFTIMERMEMORY_BSTRICT_BITS "21:21"
#define SB_ZF_C2_PM_PROFTIMERMEMORY_BSTARTED_BITS "20:20"
#define SB_ZF_C2_PM_PROFTIMERMEMORY_UDEADLINE_BITS "19:0"


#define SB_ZF_C2_PM_PROFTIMERMEMORY_SIZE_IN_WORDS   ((SB_ZF_C2_PM_PROFTIMERMEMORY_SIZE_IN_BYTES+3)/4)



/** @brief  PM Profile Timer memory Configuration

  FOR INTERANL USE ONLY NOT FOR THE API USER
*/

typedef struct _sbZfC2PmProfileTimerMemory {
  uint32 uResv0;
  uint32 uType;
  uint32 uProfMode;
  uint32 uResv1;
  uint32 uResv2;
  uint32 uResv3;
  uint32 bInterrupt;
  uint32 bReset;
  uint32 bStrict;
  uint32 bStarted;
  uint32 uDeadline;
} sbZfC2PmProfileTimerMemory_t;

uint32
sbZfC2PmProfileTimerMemory_Pack(sbZfC2PmProfileTimerMemory_t *pFrom,
                                uint8 *pToData,
                                uint32 nMaxToDataIndex);
void
sbZfC2PmProfileTimerMemory_Unpack(sbZfC2PmProfileTimerMemory_t *pToStruct,
                                  uint8 *pFromData,
                                  uint32 nMaxToDataIndex);
void
sbZfC2PmProfileTimerMemory_InitInstance(sbZfC2PmProfileTimerMemory_t *pFrame);

#define SB_ZF_C2PMPROFILETIMERMEMORY_SET_RESV0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_SET_TYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_SET_PROFMODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_SET_RESV1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_SET_RESV2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_SET_RESV3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_SET_BINTERRUPT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_SET_BRESET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_SET_BSTRICT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_SET_BSTARTED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_SET_DEADLINE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_GET_RESV0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 3) & 0x1f; \
           (nToData) |= (uint32) (pFromData)[1] << 5; \
           (nToData) |= (uint32) (pFromData)[0] << 13; \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_GET_TYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_GET_PROFMODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x03; \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_GET_RESV1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_GET_RESV2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) (pFromData)[6] << 8; \
           (nToData) |= (uint32) (pFromData)[5] << 16; \
           (nToData) |= (uint32) (pFromData)[4] << 24; \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_GET_RESV3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[8] ; \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_GET_BINTERRUPT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_GET_BRESET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_GET_BSTRICT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_GET_BSTARTED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILETIMERMEMORY_GET_DEADLINE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[11] ; \
           (nToData) |= (uint32) (pFromData)[10] << 8; \
           (nToData) |= (uint32) ((pFromData)[9] & 0x0f) << 16; \
          } while(0)

/**
 * Return 1 if same else 0.
 */
uint8 sbZfC2PmProfileTimerMemory_Compare( sbZfC2PmProfileTimerMemory_t *pProfile1,
                                              sbZfC2PmProfileTimerMemory_t *pProfile2);

void sbZfC2PmProfileTimerMemory_Copy( sbZfC2PmProfileTimerMemory_t *pSource,
                                        sbZfC2PmProfileTimerMemory_t *pDest);

uint32 sbZfC2PmProfileTimerMemory_Pack32( sbZfC2PmProfileTimerMemory_t *pFrom, 
                                              uint32 *pToData, 
                                              uint32 nMaxToDataIndex);

uint32 sbZfC2PmProfileTimerMemory_Unpack32(sbZfC2PmProfileTimerMemory_t *pToData,
                                               uint32 *pFrom,
                                               uint32 nMaxToDataIndex);
#endif
