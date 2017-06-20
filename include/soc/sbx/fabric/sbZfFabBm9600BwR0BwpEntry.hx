/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600BwR0BwpEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_BWR0BWPENTRY_H
#define SB_ZF_FAB_BM9600_BWR0BWPENTRY_H

#define SB_ZF_FAB_BM9600_BWR0BWPENTRY_SIZE_IN_BYTES 4
#define SB_ZF_FAB_BM9600_BWR0BWPENTRY_SIZE 4
#define SB_ZF_FAB_BM9600_BWR0BWPENTRY_M_UGAMMA_BITS "31:24"
#define SB_ZF_FAB_BM9600_BWR0BWPENTRY_M_USIGMA_BITS "23:0"


typedef struct _sbZfFabBm9600BwR0BwpEntry {
  uint32 m_uGamma;
  uint32 m_uSigma;
} sbZfFabBm9600BwR0BwpEntry_t;

uint32
sbZfFabBm9600BwR0BwpEntry_Pack(sbZfFabBm9600BwR0BwpEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex);
void
sbZfFabBm9600BwR0BwpEntry_Unpack(sbZfFabBm9600BwR0BwpEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex);
void
sbZfFabBm9600BwR0BwpEntry_InitInstance(sbZfFabBm9600BwR0BwpEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWR0BWPENTRY_SET_GAMMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM9600BWR0BWPENTRY_SET_SIGMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM9600BWR0BWPENTRY_SET_GAMMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM9600BWR0BWPENTRY_SET_SIGMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWR0BWPENTRY_SET_GAMMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM9600BWR0BWPENTRY_SET_SIGMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM9600BWR0BWPENTRY_SET_GAMMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM9600BWR0BWPENTRY_SET_SIGMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWR0BWPENTRY_GET_GAMMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#define SB_ZF_FABBM9600BWR0BWPENTRY_GET_SIGMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
          } while(0)

#else
#define SB_ZF_FABBM9600BWR0BWPENTRY_GET_GAMMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_FABBM9600BWR0BWPENTRY_GET_SIGMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWR0BWPENTRY_GET_GAMMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#define SB_ZF_FABBM9600BWR0BWPENTRY_GET_SIGMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
          } while(0)

#else
#define SB_ZF_FABBM9600BWR0BWPENTRY_GET_GAMMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_FABBM9600BWR0BWPENTRY_GET_SIGMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
          } while(0)

#endif
#endif
