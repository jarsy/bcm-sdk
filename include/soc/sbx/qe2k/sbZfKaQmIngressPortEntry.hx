/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQmIngressPortEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQMINGRESSPORTENTRY_H
#define SB_ZF_ZFKAQMINGRESSPORTENTRY_H

#define SB_ZF_ZFKAQMINGRESSPORTENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQMINGRESSPORTENTRY_SIZE 4
#define SB_ZF_ZFKAQMINGRESSPORTENTRY_M_NINGRESSSPI4_BITS "6:6"
#define SB_ZF_ZFKAQMINGRESSPORTENTRY_M_NINGRESSPORT_BITS "5:0"


typedef struct _sbZfKaQmIngressPortEntry {
  uint8 m_nIngressSpi4;
  uint32 m_nIngressPort;
} sbZfKaQmIngressPortEntry_t;

uint32
sbZfKaQmIngressPortEntry_Pack(sbZfKaQmIngressPortEntry_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex);
void
sbZfKaQmIngressPortEntry_Unpack(sbZfKaQmIngressPortEntry_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex);
void
sbZfKaQmIngressPortEntry_InitInstance(sbZfKaQmIngressPortEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMINGRESSPORTENTRY_SET_INGRESSSPI4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMINGRESSPORTENTRY_SET_INGRESSPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KAQMINGRESSPORTENTRY_SET_INGRESSSPI4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMINGRESSPORTENTRY_SET_INGRESSPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMINGRESSPORTENTRY_SET_INGRESSSPI4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMINGRESSPORTENTRY_SET_INGRESSPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KAQMINGRESSPORTENTRY_SET_INGRESSSPI4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMINGRESSPORTENTRY_SET_INGRESSPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMINGRESSPORTENTRY_GET_INGRESSSPI4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMINGRESSPORTENTRY_GET_INGRESSPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#else
#define SB_ZF_KAQMINGRESSPORTENTRY_GET_INGRESSSPI4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMINGRESSPORTENTRY_GET_INGRESSPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMINGRESSPORTENTRY_GET_INGRESSSPI4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMINGRESSPORTENTRY_GET_INGRESSPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#else
#define SB_ZF_KAQMINGRESSPORTENTRY_GET_INGRESSSPI4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMINGRESSPORTENTRY_GET_INGRESSPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#endif
#endif
