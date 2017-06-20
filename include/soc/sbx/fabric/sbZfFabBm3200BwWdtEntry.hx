/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200BwWdtEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM3200_WDT_ENTRY_H
#define SB_ZF_FAB_BM3200_WDT_ENTRY_H

#define SB_ZF_FAB_BM3200_WDT_ENTRY_SIZE_IN_BYTES 2
#define SB_ZF_FAB_BM3200_WDT_ENTRY_SIZE 2
#define SB_ZF_FAB_BM3200_WDT_ENTRY_M_NTEMPLATE_BITS "15:8"
#define SB_ZF_FAB_BM3200_WDT_ENTRY_M_NSPARE_BITS "7:4"
#define SB_ZF_FAB_BM3200_WDT_ENTRY_M_NGAIN_BITS "3:0"


typedef struct _sbZfFabBm3200BwWdtEntry {
  uint32 m_nTemplate;
  uint32 m_nSpare;
  uint32 m_nGain;
} sbZfFabBm3200BwWdtEntry_t;

uint32
sbZfFabBm3200BwWdtEntry_Pack(sbZfFabBm3200BwWdtEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwWdtEntry_Unpack(sbZfFabBm3200BwWdtEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwWdtEntry_InitInstance(sbZfFabBm3200BwWdtEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWWDTENTRY_SET_TEMPLATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM3200BWWDTENTRY_SET_SPARE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWDTENTRY_SET_GAIN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWWDTENTRY_SET_TEMPLATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM3200BWWDTENTRY_SET_SPARE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWDTENTRY_SET_GAIN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWWDTENTRY_SET_TEMPLATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM3200BWWDTENTRY_SET_SPARE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWDTENTRY_SET_GAIN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWWDTENTRY_SET_TEMPLATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM3200BWWDTENTRY_SET_SPARE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWDTENTRY_SET_GAIN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWWDTENTRY_GET_TEMPLATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
          } while(0)

#define SB_ZF_FABBM3200BWWDTENTRY_GET_SPARE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWDTENTRY_GET_GAIN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWWDTENTRY_GET_TEMPLATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
          } while(0)

#define SB_ZF_FABBM3200BWWDTENTRY_GET_SPARE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWDTENTRY_GET_GAIN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWWDTENTRY_GET_TEMPLATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
          } while(0)

#define SB_ZF_FABBM3200BWWDTENTRY_GET_SPARE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWDTENTRY_GET_GAIN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWWDTENTRY_GET_TEMPLATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
          } while(0)

#define SB_ZF_FABBM3200BWWDTENTRY_GET_SPARE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWDTENTRY_GET_GAIN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200BwWdtEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_BM3200_WDT_ENTRY_CONSOLE_H
#define SB_ZF_FAB_BM3200_WDT_ENTRY_CONSOLE_H



void
sbZfFabBm3200BwWdtEntry_Print(sbZfFabBm3200BwWdtEntry_t *pFromStruct);
int
sbZfFabBm3200BwWdtEntry_SPrint(sbZfFabBm3200BwWdtEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm3200BwWdtEntry_Validate(sbZfFabBm3200BwWdtEntry_t *pZf);
int
sbZfFabBm3200BwWdtEntry_SetField(sbZfFabBm3200BwWdtEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_WDT_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

