/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FE_2000_PM_POLICERRAWFORMAT_H
#define SB_ZF_FE_2000_PM_POLICERRAWFORMAT_H

#define SB_ZF_FE_2000_PM_POLICERRAWFORMAT_SIZE_IN_BYTES 8
#define SB_ZF_FE_2000_PM_POLICERRAWFORMAT_SIZE 8
#define SB_ZF_FE_2000_PM_POLICERRAWFORMAT_UPROFILEID_BITS "63:53"
#define SB_ZF_FE_2000_PM_POLICERRAWFORMAT_UMETERSTATE_BITS "52:0"



/** @brief  MMU Policer Raw Format 

  It's for internal use only
  Given that the actual layout per policer 
  as per the group configuration & profile configuration.
*/

typedef struct _sbZfFe2000PmPolicerRawFormat {
/** @brief <p> Profile Id</p> */

  uint32 uProfileId;
/** @brief <p> timestamp+bkte+bktc</p> */

  uint64 uMeterState;
} sbZfFe2000PmPolicerRawFormat_t;

uint32
sbZfFe2000PmPolicerRawFormat_Pack(sbZfFe2000PmPolicerRawFormat_t *pFrom,
                                  uint8 *pToData,
                                  uint32 nMaxToDataIndex);
void
sbZfFe2000PmPolicerRawFormat_Unpack(sbZfFe2000PmPolicerRawFormat_t *pToStruct,
                                    uint8 *pFromData,
                                    uint32 nMaxToDataIndex);
void
sbZfFe2000PmPolicerRawFormat_InitInstance(sbZfFe2000PmPolicerRawFormat_t *pFrame);

#define SB_ZF_FE2000PMPOLICERRAWFORMAT_SET_PROFID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000PMPOLICERRAWFORMAT_SET_METERSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x1f) | (((nFromData) >> 48) & 0x1f); \
          } while(0)

#define SB_ZF_FE2000PMPOLICERRAWFORMAT_GET_PROFID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[7] << 3; \
          } while(0)

#define SB_ZF_FE2000PMPOLICERRAWFORMAT_GET_METERSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[0]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[1]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[2]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[3]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[4]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#endif
