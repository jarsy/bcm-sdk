/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FE_2000_PM_OAMTIMERRAWFORMAT_H
#define SB_ZF_FE_2000_PM_OAMTIMERRAWFORMAT_H

#define SB_ZF_FE_2000_PM_OAMTIMERRAWFORMAT_SIZE_IN_BYTES 8
#define SB_ZF_FE_2000_PM_OAMTIMERRAWFORMAT_SIZE 8
#define SB_ZF_FE_2000_PM_OAMTIMERRAWFORMAT_UPROFILEID_BITS "63:53"
#define SB_ZF_FE_2000_PM_OAMTIMERRAWFORMAT_USTARTED_BITS "52:52"
#define SB_ZF_FE_2000_PM_OAMTIMERRAWFORMAT_UTIME_BITS "51:32"
#define SB_ZF_FE_2000_PM_OAMTIMERRAWFORMAT_URSVD_BITS "31:0"



/** @brief  MMU OAM Timer Raw Format 

  It's for internal use only
  Given that the actual layout per OAM Timer 
  as per the group configuration & profile configuration.
*/

typedef struct _sbZfFe2000PmOamTimerRawFormat {
/** @brief <p> Profile Id</p> */

  uint32 uProfileId;
/** @brief <p> time + sequence No</p> */

  uint32 uStarted;
  uint32 uTime;
  uint32 uRsvd;
} sbZfFe2000PmOamTimerRawFormat_t;

uint32
sbZfFe2000PmOamTimerRawFormat_Pack(sbZfFe2000PmOamTimerRawFormat_t *pFrom,
                                   uint8 *pToData,
                                   uint32 nMaxToDataIndex);
void
sbZfFe2000PmOamTimerRawFormat_Unpack(sbZfFe2000PmOamTimerRawFormat_t *pToStruct,
                                     uint8 *pFromData,
                                     uint32 nMaxToDataIndex);
void
sbZfFe2000PmOamTimerRawFormat_InitInstance(sbZfFe2000PmOamTimerRawFormat_t *pFrame);

#define SB_ZF_FE2000PMOAMTIMERRAWFORMAT_SET_PROFID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERRAWFORMAT_SET_STARTED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERRAWFORMAT_SET_TIME(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x0f) | (((nFromData) >> 16) & 0x0f); \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERRAWFORMAT_SET_RSVD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERRAWFORMAT_GET_PROFID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[7] << 3; \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERRAWFORMAT_GET_STARTED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERRAWFORMAT_GET_TIME(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
           (nToData) |= (uint32) (pFromData)[5] << 8; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x0f) << 16; \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERRAWFORMAT_GET_RSVD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
           (nToData) |= (uint32) (pFromData)[3] << 24; \
          } while(0)

#endif
