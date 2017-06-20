/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FE_2000_PM_OAMTIMERCFG_H
#define SB_ZF_FE_2000_PM_OAMTIMERCFG_H

#define SB_ZF_FE_2000_PM_OAMTIMERCFG_SIZE_IN_BYTES 5
#define SB_ZF_FE_2000_PM_OAMTIMERCFG_SIZE 5
#define SB_ZF_FE_2000_PM_OAMTIMERCFG_URATE_BITS "39:36"
#define SB_ZF_FE_2000_PM_OAMTIMERCFG_UDEADLINE_BITS "35:4"
#define SB_ZF_FE_2000_PM_OAMTIMERCFG_BSTARTED_BITS "3:3"
#define SB_ZF_FE_2000_PM_OAMTIMERCFG_BSTRICT_BITS "2:2"
#define SB_ZF_FE_2000_PM_OAMTIMERCFG_BRESET_BITS "1:1"
#define SB_ZF_FE_2000_PM_OAMTIMERCFG_BINTERRUPT_BITS "0:0"




/** @brief  User Timer Configuration

*/

typedef struct _sbZfFe2000PmOamTimerConfig {
  uint32 uRate;
  uint32 uDeadline;
  uint32 bStarted;
  uint32 bStrict;
  uint32 bReset;
  uint32 bInterrupt;
} sbZfFe2000PmOamTimerConfig_t;

uint32
sbZfFe2000PmOamTimerConfig_Pack(sbZfFe2000PmOamTimerConfig_t *pFrom,
                                uint8 *pToData,
                                uint32 nMaxToDataIndex);
void
sbZfFe2000PmOamTimerConfig_Unpack(sbZfFe2000PmOamTimerConfig_t *pToStruct,
                                  uint8 *pFromData,
                                  uint32 nMaxToDataIndex);
void
sbZfFe2000PmOamTimerConfig_InitInstance(sbZfFe2000PmOamTimerConfig_t *pFrame);

#define SB_ZF_FE2000PMOAMTIMERCONFIG_SET_RATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_SET_DEADLINE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 20) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 28) & 0x0f); \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_SET_STARTED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_SET_STRICT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_SET_RESET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_SET_INTERRUPT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_GET_RATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_GET_DEADLINE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[1] << 4; \
           (nToData) |= (uint32) (pFromData)[2] << 12; \
           (nToData) |= (uint32) (pFromData)[3] << 20; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x0f) << 28; \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_GET_STARTED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_GET_STRICT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_GET_RESET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FE2000PMOAMTIMERCONFIG_GET_INTERRUPT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
