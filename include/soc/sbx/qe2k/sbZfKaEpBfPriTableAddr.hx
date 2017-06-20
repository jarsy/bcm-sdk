/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEpBfPriTableAddr.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEPBFPRITABLEADDR_H
#define SB_ZF_ZFKAEPBFPRITABLEADDR_H

#define SB_ZF_ZFKAEPBFPRITABLEADDR_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAEPBFPRITABLEADDR_SIZE 4
#define SB_ZF_ZFKAEPBFPRITABLEADDR_M_NPORT_BITS "11:6"
#define SB_ZF_ZFKAEPBFPRITABLEADDR_M_NCOS_BITS "5:3"
#define SB_ZF_ZFKAEPBFPRITABLEADDR_M_NDP_BITS "2:1"
#define SB_ZF_ZFKAEPBFPRITABLEADDR_M_NECN_BITS "0:0"


typedef struct _sbZfKaEpBfPriTableAddr {
  uint32 m_nPort;
  uint32 m_nCos;
  uint32 m_nDp;
  uint32 m_nEcn;
} sbZfKaEpBfPriTableAddr_t;

uint32
sbZfKaEpBfPriTableAddr_Pack(sbZfKaEpBfPriTableAddr_t *pFrom,
                            uint8 *pToData,
                            uint32 nMaxToDataIndex);
void
sbZfKaEpBfPriTableAddr_Unpack(sbZfKaEpBfPriTableAddr_t *pToStruct,
                              uint8 *pFromData,
                              uint32 nMaxToDataIndex);
void
sbZfKaEpBfPriTableAddr_InitInstance(sbZfKaEpBfPriTableAddr_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPBFPRITABLEADDR_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_SET_COS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_SET_DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_SET_ECN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_KAEPBFPRITABLEADDR_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_SET_COS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_SET_DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_SET_ECN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPBFPRITABLEADDR_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_SET_COS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_SET_DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_SET_ECN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_KAEPBFPRITABLEADDR_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_SET_COS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_SET_DP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_SET_ECN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPBFPRITABLEADDR_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_GET_COS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_GET_DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_GET_ECN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_KAEPBFPRITABLEADDR_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_GET_COS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_GET_DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_GET_ECN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPBFPRITABLEADDR_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_GET_COS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_GET_DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_GET_ECN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_KAEPBFPRITABLEADDR_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_GET_COS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_GET_DP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEADDR_GET_ECN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#endif
