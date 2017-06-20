/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsLastSentPriAddr.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQSLASTSENTPRIADDR_H
#define SB_ZF_ZFKAQSLASTSENTPRIADDR_H

#define SB_ZF_ZFKAQSLASTSENTPRIADDR_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQSLASTSENTPRIADDR_SIZE 4
#define SB_ZF_ZFKAQSLASTSENTPRIADDR_M_NRESERVED_BITS "31:13"
#define SB_ZF_ZFKAQSLASTSENTPRIADDR_M_NMC_BITS "12:12"
#define SB_ZF_ZFKAQSLASTSENTPRIADDR_M_NNODE_BITS "11:7"
#define SB_ZF_ZFKAQSLASTSENTPRIADDR_M_NPORT_BITS "6:0"


typedef struct _sbZfKaQsLastSentPriAddr {
  uint32 m_nReserved;
  uint32 m_nMc;
  uint32 m_nNode;
  uint32 m_nPort;
} sbZfKaQsLastSentPriAddr_t;

uint32
sbZfKaQsLastSentPriAddr_Pack(sbZfKaQsLastSentPriAddr_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex);
void
sbZfKaQsLastSentPriAddr_Unpack(sbZfKaQsLastSentPriAddr_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex);
void
sbZfKaQsLastSentPriAddr_InitInstance(sbZfKaQsLastSentPriAddr_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSLASTSENTPRIADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 1) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#else
#define SB_ZF_KAQSLASTSENTPRIADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 1) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSLASTSENTPRIADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 1) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#else
#define SB_ZF_KAQSLASTSENTPRIADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 1) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSLASTSENTPRIADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[1] << 3; \
           (nToData) |= (uint32) (pFromData)[0] << 11; \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 1; \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x7f; \
          } while(0)

#else
#define SB_ZF_KAQSLASTSENTPRIADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[2] << 3; \
           (nToData) |= (uint32) (pFromData)[3] << 11; \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 1; \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x7f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSLASTSENTPRIADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[1] << 3; \
           (nToData) |= (uint32) (pFromData)[0] << 11; \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 1; \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x7f; \
          } while(0)

#else
#define SB_ZF_KAQSLASTSENTPRIADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[2] << 3; \
           (nToData) |= (uint32) (pFromData)[3] << 11; \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 1; \
          } while(0)

#define SB_ZF_KAQSLASTSENTPRIADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x7f; \
          } while(0)

#endif
#endif
