/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaRbClassIPv6ClassEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKARBCLASSIPV6CLASSENTRY_H
#define SB_ZF_ZFKARBCLASSIPV6CLASSENTRY_H

#define SB_ZF_ZFKARBCLASSIPV6CLASSENTRY_SIZE_IN_BYTES 2
#define SB_ZF_ZFKARBCLASSIPV6CLASSENTRY_SIZE 2
#define SB_ZF_ZFKARBCLASSIPV6CLASSENTRY_M_NRESERVE1_BITS "15:14"
#define SB_ZF_ZFKARBCLASSIPV6CLASSENTRY_M_NCLASS1DP_BITS "13:12"
#define SB_ZF_ZFKARBCLASSIPV6CLASSENTRY_M_NCLASS1LSB_BITS "11:8"
#define SB_ZF_ZFKARBCLASSIPV6CLASSENTRY_M_NRESERVE0_BITS "7:6"
#define SB_ZF_ZFKARBCLASSIPV6CLASSENTRY_M_NCLASS0DP_BITS "5:4"
#define SB_ZF_ZFKARBCLASSIPV6CLASSENTRY_M_NCLASS0LSB_BITS "3:0"


typedef struct _sbZfKaRbClassIPv6ClassEntry {
  uint32 m_nReserve1;
  uint32 m_nClass1Dp;
  uint32 m_nClass1Lsb;
  uint32 m_nReserve0;
  uint32 m_nClass0Dp;
  uint32 m_nClass0Lsb;
} sbZfKaRbClassIPv6ClassEntry_t;

uint32
sbZfKaRbClassIPv6ClassEntry_Pack(sbZfKaRbClassIPv6ClassEntry_t *pFrom,
                                 uint8 *pToData,
                                 uint32 nMaxToDataIndex);
void
sbZfKaRbClassIPv6ClassEntry_Unpack(sbZfKaRbClassIPv6ClassEntry_t *pToStruct,
                                   uint8 *pFromData,
                                   uint32 nMaxToDataIndex);
void
sbZfKaRbClassIPv6ClassEntry_InitInstance(sbZfKaRbClassIPv6ClassEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_CLASS1DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_CLASS1LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_RES0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_CLASS0DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_CLASS0LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_CLASS1DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_CLASS1LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_RES0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_CLASS0DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_CLASS0LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_CLASS1DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_CLASS1LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_RES0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_CLASS0DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_CLASS0LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_CLASS1DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_CLASS1LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_RES0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_CLASS0DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_SET_CLASS0LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_CLASS1DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_CLASS1LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_RES0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_CLASS0DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_CLASS0LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_CLASS1DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_CLASS1LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_RES0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_CLASS0DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_CLASS0LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_CLASS1DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_CLASS1LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_RES0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_CLASS0DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_CLASS0LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_CLASS1DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_CLASS1LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_RES0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_CLASS0DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV6CLASSENTRY_GET_CLASS0LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
