/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQmRateDeltaMaxTableEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQMRATEDELTAMAXTABLEENTRY_H
#define SB_ZF_ZFKAQMRATEDELTAMAXTABLEENTRY_H

#define SB_ZF_ZFKAQMRATEDELTAMAXTABLEENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQMRATEDELTAMAXTABLEENTRY_SIZE 4
#define SB_ZF_ZFKAQMRATEDELTAMAXTABLEENTRY_M_NRATEDELTAMAX_BITS "26:0"


typedef struct _sbZfKaQmRateDeltaMaxTableEntry {
  uint32 m_nRateDeltaMax;
} sbZfKaQmRateDeltaMaxTableEntry_t;

uint32
sbZfKaQmRateDeltaMaxTableEntry_Pack(sbZfKaQmRateDeltaMaxTableEntry_t *pFrom,
                                    uint8 *pToData,
                                    uint32 nMaxToDataIndex);
void
sbZfKaQmRateDeltaMaxTableEntry_Unpack(sbZfKaQmRateDeltaMaxTableEntry_t *pToStruct,
                                      uint8 *pFromData,
                                      uint32 nMaxToDataIndex);
void
sbZfKaQmRateDeltaMaxTableEntry_InitInstance(sbZfKaQmRateDeltaMaxTableEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMRATEDELTAMAXTABLEENTRY_SET_RATE_DELTA_MAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~ 0x07) | (((nFromData) >> 24) & 0x07); \
          } while(0)

#else
#define SB_ZF_KAQMRATEDELTAMAXTABLEENTRY_SET_RATE_DELTA_MAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~ 0x07) | (((nFromData) >> 24) & 0x07); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMRATEDELTAMAXTABLEENTRY_SET_RATE_DELTA_MAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~ 0x07) | (((nFromData) >> 24) & 0x07); \
          } while(0)

#else
#define SB_ZF_KAQMRATEDELTAMAXTABLEENTRY_SET_RATE_DELTA_MAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~ 0x07) | (((nFromData) >> 24) & 0x07); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMRATEDELTAMAXTABLEENTRY_GET_RATE_DELTA_MAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x07) << 24; \
          } while(0)

#else
#define SB_ZF_KAQMRATEDELTAMAXTABLEENTRY_GET_RATE_DELTA_MAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x07) << 24; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMRATEDELTAMAXTABLEENTRY_GET_RATE_DELTA_MAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x07) << 24; \
          } while(0)

#else
#define SB_ZF_KAQMRATEDELTAMAXTABLEENTRY_GET_RATE_DELTA_MAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x07) << 24; \
          } while(0)

#endif
#endif
