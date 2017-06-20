/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600BwAllocRateEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_BWALLOCRATEENTRY_H
#define SB_ZF_FAB_BM9600_BWALLOCRATEENTRY_H

#define SB_ZF_FAB_BM9600_BWALLOCRATEENTRY_SIZE_IN_BYTES 3
#define SB_ZF_FAB_BM9600_BWALLOCRATEENTRY_SIZE 3
#define SB_ZF_FAB_BM9600_BWALLOCRATEENTRY_M_URATE_BITS "23:0"


typedef struct _sbZfFabBm9600BwAllocRateEntry {
  uint32 m_uRate;
} sbZfFabBm9600BwAllocRateEntry_t;

uint32
sbZfFabBm9600BwAllocRateEntry_Pack(sbZfFabBm9600BwAllocRateEntry_t *pFrom,
                                   uint8 *pToData,
                                   uint32 nMaxToDataIndex);
void
sbZfFabBm9600BwAllocRateEntry_Unpack(sbZfFabBm9600BwAllocRateEntry_t *pToStruct,
                                     uint8 *pFromData,
                                     uint32 nMaxToDataIndex);
void
sbZfFabBm9600BwAllocRateEntry_InitInstance(sbZfFabBm9600BwAllocRateEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWALLOCRATEENTRY_SET_RATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM9600BWALLOCRATEENTRY_SET_RATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWALLOCRATEENTRY_SET_RATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM9600BWALLOCRATEENTRY_SET_RATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWALLOCRATEENTRY_GET_RATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
          } while(0)

#else
#define SB_ZF_FABBM9600BWALLOCRATEENTRY_GET_RATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWALLOCRATEENTRY_GET_RATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
          } while(0)

#else
#define SB_ZF_FABBM9600BWALLOCRATEENTRY_GET_RATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
          } while(0)

#endif
#endif
