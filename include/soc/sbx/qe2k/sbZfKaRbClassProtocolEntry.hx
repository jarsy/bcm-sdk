/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaRbClassProtocolEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKARBCLASSPROTOCOLENTRY_H
#define SB_ZF_ZFKARBCLASSPROTOCOLENTRY_H

#define SB_ZF_ZFKARBCLASSPROTOCOLENTRY_SIZE_IN_BYTES 2
#define SB_ZF_ZFKARBCLASSPROTOCOLENTRY_SIZE 2
#define SB_ZF_ZFKARBCLASSPROTOCOLENTRY_M_NPROTOCOL1USETOS_BITS "15:15"
#define SB_ZF_ZFKARBCLASSPROTOCOLENTRY_M_NPROTOCOL1USESOCKETINHASH_BITS "14:14"
#define SB_ZF_ZFKARBCLASSPROTOCOLENTRY_M_NPROTOCOL1DP_BITS "13:12"
#define SB_ZF_ZFKARBCLASSPROTOCOLENTRY_M_NPROTOCOL1LSB_BITS "11:8"
#define SB_ZF_ZFKARBCLASSPROTOCOLENTRY_M_NPROTOCOL0USETOS_BITS "7:7"
#define SB_ZF_ZFKARBCLASSPROTOCOLENTRY_M_NPROTOCOL0USESOCKETINHASH_BITS "6:6"
#define SB_ZF_ZFKARBCLASSPROTOCOLENTRY_M_NPROTOCOL0DP_BITS "5:4"
#define SB_ZF_ZFKARBCLASSPROTOCOLENTRY_M_NPROTOCOL0LSB_BITS "3:0"


typedef struct _sbZfKaRbClassProtocolEntry {
  uint8 m_nProtocol1UseTos;
  uint8 m_nProtocol1UseSocketInHash;
  uint32 m_nProtocol1Dp;
  uint32 m_nProtocol1Lsb;
  uint8 m_nProtocol0UseTos;
  uint8 m_nProtocol0UseSocketInHash;
  uint32 m_nProtocol0Dp;
  uint32 m_nProtocol0Lsb;
} sbZfKaRbClassProtocolEntry_t;

uint32
sbZfKaRbClassProtocolEntry_Pack(sbZfKaRbClassProtocolEntry_t *pFrom,
                                uint8 *pToData,
                                uint32 nMaxToDataIndex);
void
sbZfKaRbClassProtocolEntry_Unpack(sbZfKaRbClassProtocolEntry_t *pToStruct,
                                  uint8 *pFromData,
                                  uint32 nMaxToDataIndex);
void
sbZfKaRbClassProtocolEntry_InitInstance(sbZfKaRbClassProtocolEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL1USETOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL1USESKTINHASH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL1DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL1LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL0USETOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL0USESKTINHASH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL0DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL0LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL1USETOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL1USESKTINHASH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL1DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL1LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL0USETOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL0USESKTINHASH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL0DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL0LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL1USETOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL1USESKTINHASH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL1DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL1LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL0USETOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL0USESKTINHASH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL0DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL0LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL1USETOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL1USESKTINHASH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL1DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL1LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL0USETOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL0USESKTINHASH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL0DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_SET_PROTOCOL0LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL1USETOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL1USESKTINHASH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL1DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL1LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL0USETOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL0USESKTINHASH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL0DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL0LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL1USETOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL1USESKTINHASH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL1DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL1LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL0USETOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL0USESKTINHASH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL0DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL0LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL1USETOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL1USESKTINHASH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL1DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL1LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL0USETOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL0USESKTINHASH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL0DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL0LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL1USETOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL1USESKTINHASH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL1DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL1LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL0USETOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL0USESKTINHASH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL0DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSPROTOCOLENTRY_GET_PROTOCOL0LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
