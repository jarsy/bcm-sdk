/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_C2_PM_PROFPOLICERMEMORY_H
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_H

#define SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES 12
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE 12
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_UTYPE_BITS "74:74"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_ULENGTHSHIFT_BITS "73:71"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_BCOUPLINGFLAG_BITS "70:70"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_BRFC2698MODE_BITS "69:69"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_UBKTCSIZE_BITS "68:64"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_BBLIND_BITS "63:63"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_BBKTENODECREMENT_BITS "62:62"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_UEIR_BITS "61:44"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_UEBS_BITS "43:32"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_BDROPONRED_BITS "31:31"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_BBKTCNODECREMENT_BITS "30:30"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_UCIR_BITS "29:12"
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_UCBS_BITS "11:0"


#define SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_WORDS   ((SB_ZF_C2_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES+3)/4)



/** @brief  PM Profile Policer memory Configuration

  FOR INTERANL USE ONLY NOT FOR THE API USER
*/

typedef struct _sbZfC2PmProfilePolicerMemory {
  uint32 uType;
  uint32 uLengthShift;
  uint32 bCouplingFlag;
  uint32 bRFC2698Mode;
  uint32 uBktCSize;
  uint32 bBlind;
  uint32 bBktENoDecrement;
  uint32 uEIR;
  uint32 uEBS;
  uint32 bDropOnRed;
  uint32 bBktCNoDecrement;
  uint32 uCIR;
  uint32 uCBS;
} sbZfC2PmProfilePolicerMemory_t;

uint32
sbZfC2PmProfilePolicerMemory_Pack(sbZfC2PmProfilePolicerMemory_t *pFrom,
                                  uint8 *pToData,
                                  uint32 nMaxToDataIndex);
void
sbZfC2PmProfilePolicerMemory_Unpack(sbZfC2PmProfilePolicerMemory_t *pToStruct,
                                    uint8 *pFromData,
                                    uint32 nMaxToDataIndex);
void
sbZfC2PmProfilePolicerMemory_InitInstance(sbZfC2PmProfilePolicerMemory_t *pFrame);

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_TYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_LENSHIFT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_CFLAG(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_R2698(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_BCSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_BLIND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_BENODCR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_EIR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x3f) | (((nFromData) >> 12) & 0x3f); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_EBS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_DRED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_BCNODCR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_CIR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~ 0x3f) | (((nFromData) >> 12) & 0x3f); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_SET_CBS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_TYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_LENSHIFT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 1; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_CFLAG(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_R2698(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_BCSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x1f; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_BLIND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_BENODCR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_EIR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[5] << 4; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x3f) << 12; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_EBS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_DRED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_BCNODCR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_CIR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[9] << 4; \
           (nToData) |= (uint32) ((pFromData)[8] & 0x3f) << 12; \
          } while(0)

#define SB_ZF_C2PMPROFILEPOLICERMEMORY_GET_CBS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[11] ; \
           (nToData) |= (uint32) ((pFromData)[10] & 0x0f) << 8; \
          } while(0)

/**
 * Return 1 if same else 0.
 */
uint8 sbZfC2PmProfilePolicerMemory_Compare( sbZfC2PmProfilePolicerMemory_t *pProfile1,
                                                sbZfC2PmProfilePolicerMemory_t *pProfile2);
void sbZfC2PmProfilePolicerMemory_Copy( sbZfC2PmProfilePolicerMemory_t *pSource,
                                          sbZfC2PmProfilePolicerMemory_t *pDest);

uint32 sbZfC2PmProfilePolicerMemory_Pack32( sbZfC2PmProfilePolicerMemory_t *pFrom, 
                                                uint32 *pToData, 
                                                uint32 nMaxToDataIndex);

uint32 sbZfC2PmProfilePolicerMemory_Unpack32(sbZfC2PmProfilePolicerMemory_t *pToData,
                                                 uint32 *pFrom,
                                                 uint32 nMaxToDataIndex);
#endif
