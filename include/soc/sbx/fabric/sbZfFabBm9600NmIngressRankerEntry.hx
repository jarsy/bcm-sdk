/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600NmIngressRankerEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_NMINGRESSRANKERENTRY_H
#define SB_ZF_FAB_BM9600_NMINGRESSRANKERENTRY_H

#define SB_ZF_FAB_BM9600_NMINGRESSRANKERENTRY_SIZE_IN_BYTES 1
#define SB_ZF_FAB_BM9600_NMINGRESSRANKERENTRY_SIZE 1
#define SB_ZF_FAB_BM9600_NMINGRESSRANKERENTRY_M_USEED_BITS "6:0"


typedef struct _sbZfFabBm9600NmIngressRankerEntry {
  uint32 m_uSeed;
} sbZfFabBm9600NmIngressRankerEntry_t;

uint32
sbZfFabBm9600NmIngressRankerEntry_Pack(sbZfFabBm9600NmIngressRankerEntry_t *pFrom,
                                       uint8 *pToData,
                                       uint32 nMaxToDataIndex);
void
sbZfFabBm9600NmIngressRankerEntry_Unpack(sbZfFabBm9600NmIngressRankerEntry_t *pToStruct,
                                         uint8 *pFromData,
                                         uint32 nMaxToDataIndex);
void
sbZfFabBm9600NmIngressRankerEntry_InitInstance(sbZfFabBm9600NmIngressRankerEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMINGRESSRANKERENTRY_SET_SEED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#else
#define SB_ZF_FABBM9600NMINGRESSRANKERENTRY_SET_SEED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMINGRESSRANKERENTRY_SET_SEED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#else
#define SB_ZF_FABBM9600NMINGRESSRANKERENTRY_SET_SEED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMINGRESSRANKERENTRY_GET_SEED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x7f; \
          } while(0)

#else
#define SB_ZF_FABBM9600NMINGRESSRANKERENTRY_GET_SEED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x7f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMINGRESSRANKERENTRY_GET_SEED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x7f; \
          } while(0)

#else
#define SB_ZF_FABBM9600NMINGRESSRANKERENTRY_GET_SEED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x7f; \
          } while(0)

#endif
#endif
