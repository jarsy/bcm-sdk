/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600InaRandomNumGenEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_INARANDOMNUMGENENTRY_H
#define SB_ZF_FAB_BM9600_INARANDOMNUMGENENTRY_H

#define SB_ZF_FAB_BM9600_INARANDOMNUMGENENTRY_SIZE_IN_BYTES 2
#define SB_ZF_FAB_BM9600_INARANDOMNUMGENENTRY_SIZE 2
#define SB_ZF_FAB_BM9600_INARANDOMNUMGENENTRY_M_USEED_BITS "15:0"


typedef struct _sbZfFabBm9600InaRandomNumGenEntry {
  uint32 m_uSeed;
} sbZfFabBm9600InaRandomNumGenEntry_t;

uint32
sbZfFabBm9600InaRandomNumGenEntry_Pack(sbZfFabBm9600InaRandomNumGenEntry_t *pFrom,
                                       uint8 *pToData,
                                       uint32 nMaxToDataIndex);
void
sbZfFabBm9600InaRandomNumGenEntry_Unpack(sbZfFabBm9600InaRandomNumGenEntry_t *pToStruct,
                                         uint8 *pFromData,
                                         uint32 nMaxToDataIndex);
void
sbZfFabBm9600InaRandomNumGenEntry_InitInstance(sbZfFabBm9600InaRandomNumGenEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INARANDOMNUMGENENTRY_SET_SEED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM9600INARANDOMNUMGENENTRY_SET_SEED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INARANDOMNUMGENENTRY_SET_SEED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM9600INARANDOMNUMGENENTRY_SET_SEED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INARANDOMNUMGENENTRY_GET_SEED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600INARANDOMNUMGENENTRY_GET_SEED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INARANDOMNUMGENENTRY_GET_SEED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600INARANDOMNUMGENENTRY_GET_SEED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
          } while(0)

#endif
#endif
