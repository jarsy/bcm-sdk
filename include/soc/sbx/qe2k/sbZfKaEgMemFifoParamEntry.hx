/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEgMemFifoParamEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEGMEMFIFOPARAMENTRY_H
#define SB_ZF_ZFKAEGMEMFIFOPARAMENTRY_H

#define SB_ZF_ZFKAEGMEMFIFOPARAMENTRY_SIZE_IN_BYTES 8
#define SB_ZF_ZFKAEGMEMFIFOPARAMENTRY_SIZE 8
#define SB_ZF_ZFKAEGMEMFIFOPARAMENTRY_M_NTHRESHHI_BITS "35:26"
#define SB_ZF_ZFKAEGMEMFIFOPARAMENTRY_M_NTHRESHLO_BITS "25:16"
#define SB_ZF_ZFKAEGMEMFIFOPARAMENTRY_M_NSHAPER1_BITS "15:8"
#define SB_ZF_ZFKAEGMEMFIFOPARAMENTRY_M_NSHAPER0_BITS "7:0"


typedef struct _sbZfKaEgMemFifoParamEntry {
  uint32 m_nThreshHi;
  uint32 m_nThreshLo;
  uint32 m_nShaper1;
  uint32 m_nShaper0;
} sbZfKaEgMemFifoParamEntry_t;

uint32
sbZfKaEgMemFifoParamEntry_Pack(sbZfKaEgMemFifoParamEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex);
void
sbZfKaEgMemFifoParamEntry_Unpack(sbZfKaEgMemFifoParamEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex);
void
sbZfKaEgMemFifoParamEntry_InitInstance(sbZfKaEgMemFifoParamEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMFIFOPARAMENTRY_SET_THRESHHI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 6) & 0x0f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_SET_THRESHLO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_SET_SHAPER1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_SET_SHAPER0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAEGMEMFIFOPARAMENTRY_SET_THRESHHI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 6) & 0x0f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_SET_THRESHLO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_SET_SHAPER1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_SET_SHAPER0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMFIFOPARAMENTRY_SET_THRESHHI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 6) & 0x0f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_SET_THRESHLO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_SET_SHAPER1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_SET_SHAPER0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAEGMEMFIFOPARAMENTRY_SET_THRESHHI(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 6) & 0x0f); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_SET_THRESHLO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_SET_SHAPER1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_SET_SHAPER0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMFIFOPARAMENTRY_GET_THRESHHI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 2) & 0x3f; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x0f) << 6; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_GET_THRESHLO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x03) << 8; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_GET_SHAPER1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_GET_SHAPER0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAEGMEMFIFOPARAMENTRY_GET_THRESHHI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 2) & 0x3f; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x0f) << 6; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_GET_THRESHLO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x03) << 8; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_GET_SHAPER1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_GET_SHAPER0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMFIFOPARAMENTRY_GET_THRESHHI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 2) & 0x3f; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x0f) << 6; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_GET_THRESHLO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x03) << 8; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_GET_SHAPER1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_GET_SHAPER0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAEGMEMFIFOPARAMENTRY_GET_THRESHHI(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 2) & 0x3f; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x0f) << 6; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_GET_THRESHLO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x03) << 8; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_GET_SHAPER1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
          } while(0)

#define SB_ZF_KAEGMEMFIFOPARAMENTRY_GET_SHAPER0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#endif
