/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_H
#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_H

#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES 8
#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_SIZE 8
#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_URESV0_BITS "63:63"
#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_UTYPE_BITS "62:62"
#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_BDROPONRED_BITS "61:61"
#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_BBKTENODECREMENT_BITS "60:60"
#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_BBKTCNODECREMENT_BITS "59:59"
#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_ULENGTHSHIFT_BITS "58:56"
#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_UEIR_BITS "55:44"
#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_UCIR_BITS "43:32"
#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_BCOUPLINGFLAG_BITS "31:31"
#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_BRFC2698MODE_BITS "30:30"
#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_BBLIND_BITS "29:29"
#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_UBKTCSIZE_BITS "28:24"
#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_UEBS_BITS "23:12"
#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_UCBS_BITS "11:0"


#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_SIZE_IN_WORDS   ((SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_SIZE_IN_BYTES+3)/4)



/** @brief  PM Profile Policer memory Configuration

  FOR INTERANL USE ONLY NOT FOR THE API USER
*/

typedef struct _sbZfFe2000PmProfilePolicerMemory {
  uint32 uResv0;
  uint32 uType;
  uint32 bDropOnRed;
  uint32 bBktENoDecrement;
  uint32 bBktCNoDecrement;
  uint32 uLengthShift;
  uint32 uEIR;
  uint32 uCIR;
  uint32 bCouplingFlag;
  uint32 bRFC2698Mode;
  uint32 bBlind;
  uint32 uBktCSize;
  uint32 uEBS;
  uint32 uCBS;
} sbZfFe2000PmProfilePolicerMemory_t;

uint32
sbZfFe2000PmProfilePolicerMemory_Pack(sbZfFe2000PmProfilePolicerMemory_t *pFrom,
                                      uint8 *pToData,
                                      uint32 nMaxToDataIndex);
void
sbZfFe2000PmProfilePolicerMemory_Unpack(sbZfFe2000PmProfilePolicerMemory_t *pToStruct,
                                        uint8 *pFromData,
                                        uint32 nMaxToDataIndex);
void
sbZfFe2000PmProfilePolicerMemory_InitInstance(sbZfFe2000PmProfilePolicerMemory_t *pFrame);

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_SET_RESV0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_SET_TYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_SET_DRED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_SET_BENODCR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_SET_BCNODCR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_SET_LENSHIFT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x07) | ((nFromData) & 0x07); \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_SET_EIR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_SET_CIR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_SET_CFLAG(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_SET_R2698(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_SET_BLIND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_SET_BCSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_SET_EBS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_SET_CBS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_GET_RESV0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_GET_TYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_GET_DRED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_GET_BENODCR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_GET_BCNODCR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_GET_LENSHIFT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x07; \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_GET_EIR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[1] << 4; \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_GET_CIR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_GET_CFLAG(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_GET_R2698(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_GET_BLIND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_GET_BCSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4]) & 0x1f; \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_GET_EBS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[5] << 4; \
          } while(0)

#define SB_ZF_FE2000PMPROFILEPOLICERMEMORY_GET_CBS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x0f) << 8; \
          } while(0)

/**
 * Return 1 if same else 0.
 */
uint8 sbZfFe2000PmProfilePolicerMemory_Compare( sbZfFe2000PmProfilePolicerMemory_t *pProfile1,
                                                sbZfFe2000PmProfilePolicerMemory_t *pProfile2);
void sbZfFe2000PmProfilePolicerMemory_Copy( sbZfFe2000PmProfilePolicerMemory_t *pSource,
                                          sbZfFe2000PmProfilePolicerMemory_t *pDest);

uint32 sbZfFe2000PmProfilePolicerMemory_Pack32( sbZfFe2000PmProfilePolicerMemory_t *pFrom, 
                                                uint32 *pToData, 
                                                uint32 nMaxToDataIndex);

uint32 sbZfFe2000PmProfilePolicerMemory_Unpack32(sbZfFe2000PmProfilePolicerMemory_t *pToData,
                                                 uint32 *pFrom,
                                                 uint32 nMaxToDataIndex);
#endif
