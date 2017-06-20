/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaRbClassIPv4TosEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKARBCLASSIPV4TOSENTRY_H
#define SB_ZF_ZFKARBCLASSIPV4TOSENTRY_H

#define SB_ZF_ZFKARBCLASSIPV4TOSENTRY_SIZE_IN_BYTES 2
#define SB_ZF_ZFKARBCLASSIPV4TOSENTRY_SIZE 2
#define SB_ZF_ZFKARBCLASSIPV4TOSENTRY_M_NRESERVE1_BITS "15:14"
#define SB_ZF_ZFKARBCLASSIPV4TOSENTRY_M_NTOS1DP_BITS "13:12"
#define SB_ZF_ZFKARBCLASSIPV4TOSENTRY_M_NTOS1LSB_BITS "11:8"
#define SB_ZF_ZFKARBCLASSIPV4TOSENTRY_M_NRESERVE0_BITS "7:6"
#define SB_ZF_ZFKARBCLASSIPV4TOSENTRY_M_NTOS0DP_BITS "5:4"
#define SB_ZF_ZFKARBCLASSIPV4TOSENTRY_M_NTOS0LSB_BITS "3:0"


typedef struct _sbZfKaRbClassIPv4TosEntry {
  uint32 m_nReserve1;
  uint32 m_nTos1Dp;
  uint32 m_nTos1Lsb;
  uint32 m_nReserve0;
  uint32 m_nTos0Dp;
  uint32 m_nTos0Lsb;
} sbZfKaRbClassIPv4TosEntry_t;

uint32
sbZfKaRbClassIPv4TosEntry_Pack(sbZfKaRbClassIPv4TosEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex);
void
sbZfKaRbClassIPv4TosEntry_Unpack(sbZfKaRbClassIPv4TosEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex);
void
sbZfKaRbClassIPv4TosEntry_InitInstance(sbZfKaRbClassIPv4TosEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_TOS1DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_TOS1LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_RES0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_TOS0DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_TOS0LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_TOS1DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_TOS1LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_RES0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_TOS0DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_TOS0LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_TOS1DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_TOS1LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_RES0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_TOS0DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_TOS0LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_TOS1DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_TOS1LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_RES0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_TOS0DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_SET_TOS0LSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_TOS1DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_TOS1LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_RES0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_TOS0DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_TOS0LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_TOS1DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_TOS1LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_RES0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_TOS0DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_TOS0LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_TOS1DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_TOS1LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_RES0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_TOS0DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_TOS0LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_TOS1DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_TOS1LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_RES0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_TOS0DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSIPV4TOSENTRY_GET_TOS0LSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
