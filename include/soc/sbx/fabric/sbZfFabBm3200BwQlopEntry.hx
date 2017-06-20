/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200BwQlopEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM3200_QLOP_ENTRY_H
#define SB_ZF_FAB_BM3200_QLOP_ENTRY_H

#define SB_ZF_FAB_BM3200_QLOP_ENTRY_SIZE_IN_BYTES 4
#define SB_ZF_FAB_BM3200_QLOP_ENTRY_SIZE 4
#define SB_ZF_FAB_BM3200_QLOP_ENTRY_M_NALPHA_BITS "25:23"
#define SB_ZF_FAB_BM3200_QLOP_ENTRY_M_NBETA_BITS "22:20"
#define SB_ZF_FAB_BM3200_QLOP_ENTRY_M_NEPSILON_BITS "19:10"
#define SB_ZF_FAB_BM3200_QLOP_ENTRY_M_NRATEDELTAMAX_BITS "9:0"


typedef struct _sbZfFabBm3200BwQlopEntry {
  uint32 m_nAlpha;
  uint32 m_nBeta;
  uint32 m_nEpsilon;
  uint32 m_nRateDeltaMax;
} sbZfFabBm3200BwQlopEntry_t;

uint32
sbZfFabBm3200BwQlopEntry_Pack(sbZfFabBm3200BwQlopEntry_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwQlopEntry_Unpack(sbZfFabBm3200BwQlopEntry_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwQlopEntry_InitInstance(sbZfFabBm3200BwQlopEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWQLOPENTRY_SET_ALPHA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[0] = ((pToData)[0] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_SET_BETA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_SET_EPSILON(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 6) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_SET_RATEDELTAMAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWQLOPENTRY_SET_ALPHA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[3] = ((pToData)[3] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_SET_BETA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_SET_EPSILON(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 6) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_SET_RATEDELTAMAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWQLOPENTRY_SET_ALPHA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[0] = ((pToData)[0] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_SET_BETA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_SET_EPSILON(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 6) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_SET_RATEDELTAMAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWQLOPENTRY_SET_ALPHA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[3] = ((pToData)[3] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_SET_BETA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_SET_EPSILON(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 6) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_SET_RATEDELTAMAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWQLOPENTRY_GET_ALPHA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x03) << 1; \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_GET_BETA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_GET_EPSILON(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x3f; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 6; \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_GET_RATEDELTAMAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWQLOPENTRY_GET_ALPHA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x03) << 1; \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_GET_BETA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_GET_EPSILON(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x3f; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 6; \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_GET_RATEDELTAMAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWQLOPENTRY_GET_ALPHA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x03) << 1; \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_GET_BETA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_GET_EPSILON(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x3f; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 6; \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_GET_RATEDELTAMAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWQLOPENTRY_GET_ALPHA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x03) << 1; \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_GET_BETA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_GET_EPSILON(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x3f; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 6; \
          } while(0)

#define SB_ZF_FABBM3200BWQLOPENTRY_GET_RATEDELTAMAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200BwQlopEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_BM3200_QLOP_ENTRY_CONSOLE_H
#define SB_ZF_FAB_BM3200_QLOP_ENTRY_CONSOLE_H



void
sbZfFabBm3200BwQlopEntry_Print(sbZfFabBm3200BwQlopEntry_t *pFromStruct);
int
sbZfFabBm3200BwQlopEntry_SPrint(sbZfFabBm3200BwQlopEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm3200BwQlopEntry_Validate(sbZfFabBm3200BwQlopEntry_t *pZf);
int
sbZfFabBm3200BwQlopEntry_SetField(sbZfFabBm3200BwQlopEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_QLOP_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

