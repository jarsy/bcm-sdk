/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsAgeThreshLutEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQSAGETHRESHLUTENTRY_H
#define SB_ZF_ZFKAQSAGETHRESHLUTENTRY_H

#define SB_ZF_ZFKAQSAGETHRESHLUTENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQSAGETHRESHLUTENTRY_SIZE 4
#define SB_ZF_ZFKAQSAGETHRESHLUTENTRY_M_NRESERVED_BITS "31:16"
#define SB_ZF_ZFKAQSAGETHRESHLUTENTRY_M_NANEMICTHRESH_BITS "15:8"
#define SB_ZF_ZFKAQSAGETHRESHLUTENTRY_M_NEFTHRESH_BITS "7:0"


typedef struct _sbZfKaQsAgeThreshLutEntry {
  uint32 m_nReserved;
  uint32 m_nAnemicThresh;
  uint32 m_nEfThresh;
} sbZfKaQsAgeThreshLutEntry_t;

uint32
sbZfKaQsAgeThreshLutEntry_Pack(sbZfKaQsAgeThreshLutEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex);
void
sbZfKaQsAgeThreshLutEntry_Unpack(sbZfKaQsAgeThreshLutEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex);
void
sbZfKaQsAgeThreshLutEntry_InitInstance(sbZfKaQsAgeThreshLutEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSAGETHRESHLUTENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSAGETHRESHLUTENTRY_SET_ANEMICTHRESH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAQSAGETHRESHLUTENTRY_SET_EFTHRESH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAQSAGETHRESHLUTENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSAGETHRESHLUTENTRY_SET_ANEMICTHRESH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAQSAGETHRESHLUTENTRY_SET_EFTHRESH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSAGETHRESHLUTENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSAGETHRESHLUTENTRY_SET_ANEMICTHRESH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAQSAGETHRESHLUTENTRY_SET_EFTHRESH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAQSAGETHRESHLUTENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSAGETHRESHLUTENTRY_SET_ANEMICTHRESH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAQSAGETHRESHLUTENTRY_SET_EFTHRESH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSAGETHRESHLUTENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[0] << 8; \
          } while(0)

#define SB_ZF_KAQSAGETHRESHLUTENTRY_GET_ANEMICTHRESH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
          } while(0)

#define SB_ZF_KAQSAGETHRESHLUTENTRY_GET_EFTHRESH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAQSAGETHRESHLUTENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[3] << 8; \
          } while(0)

#define SB_ZF_KAQSAGETHRESHLUTENTRY_GET_ANEMICTHRESH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
          } while(0)

#define SB_ZF_KAQSAGETHRESHLUTENTRY_GET_EFTHRESH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSAGETHRESHLUTENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[0] << 8; \
          } while(0)

#define SB_ZF_KAQSAGETHRESHLUTENTRY_GET_ANEMICTHRESH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
          } while(0)

#define SB_ZF_KAQSAGETHRESHLUTENTRY_GET_EFTHRESH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAQSAGETHRESHLUTENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[3] << 8; \
          } while(0)

#define SB_ZF_KAQSAGETHRESHLUTENTRY_GET_ANEMICTHRESH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
          } while(0)

#define SB_ZF_KAQSAGETHRESHLUTENTRY_GET_EFTHRESH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#endif
