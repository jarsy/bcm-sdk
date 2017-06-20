/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600NmEgressRankerEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_NMEGRESSRANKERENTRY_H
#define SB_ZF_FAB_BM9600_NMEGRESSRANKERENTRY_H

#define SB_ZF_FAB_BM9600_NMEGRESSRANKERENTRY_SIZE_IN_BYTES 1
#define SB_ZF_FAB_BM9600_NMEGRESSRANKERENTRY_SIZE 1
#define SB_ZF_FAB_BM9600_NMEGRESSRANKERENTRY_M_USEED_BITS "6:0"


typedef struct _sbZfFabBm9600NmEgressRankerEntry {
  uint32 m_uSeed;
} sbZfFabBm9600NmEgressRankerEntry_t;

uint32
sbZfFabBm9600NmEgressRankerEntry_Pack(sbZfFabBm9600NmEgressRankerEntry_t *pFrom,
                                      uint8 *pToData,
                                      uint32 nMaxToDataIndex);
void
sbZfFabBm9600NmEgressRankerEntry_Unpack(sbZfFabBm9600NmEgressRankerEntry_t *pToStruct,
                                        uint8 *pFromData,
                                        uint32 nMaxToDataIndex);
void
sbZfFabBm9600NmEgressRankerEntry_InitInstance(sbZfFabBm9600NmEgressRankerEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMEGRESSRANKERENTRY_SET_SEED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#else
#define SB_ZF_FABBM9600NMEGRESSRANKERENTRY_SET_SEED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMEGRESSRANKERENTRY_SET_SEED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#else
#define SB_ZF_FABBM9600NMEGRESSRANKERENTRY_SET_SEED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMEGRESSRANKERENTRY_GET_SEED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x7f; \
          } while(0)

#else
#define SB_ZF_FABBM9600NMEGRESSRANKERENTRY_GET_SEED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x7f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMEGRESSRANKERENTRY_GET_SEED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x7f; \
          } while(0)

#else
#define SB_ZF_FABBM9600NMEGRESSRANKERENTRY_GET_SEED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x7f; \
          } while(0)

#endif
#endif
