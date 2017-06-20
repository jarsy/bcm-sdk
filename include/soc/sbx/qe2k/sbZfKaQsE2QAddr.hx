/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsE2QAddr.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQSE2QADDR_H
#define SB_ZF_ZFKAQSE2QADDR_H

#define SB_ZF_ZFKAQSE2QADDR_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQSE2QADDR_SIZE 4
#define SB_ZF_ZFKAQSE2QADDR_M_NRESERVED_BITS "31:13"
#define SB_ZF_ZFKAQSE2QADDR_M_NMC_BITS "12:12"
#define SB_ZF_ZFKAQSE2QADDR_M_NNODE_BITS "11:6"
#define SB_ZF_ZFKAQSE2QADDR_M_NPORT_BITS "5:0"


typedef struct _sbZfKaQsE2QAddr {
  uint32 m_nReserved;
  uint32 m_nMc;
  uint32 m_nNode;
  uint32 m_nPort;
} sbZfKaQsE2QAddr_t;

uint32
sbZfKaQsE2QAddr_Pack(sbZfKaQsE2QAddr_t *pFrom,
                     uint8 *pToData,
                     uint32 nMaxToDataIndex);
void
sbZfKaQsE2QAddr_Unpack(sbZfKaQsE2QAddr_t *pToStruct,
                       uint8 *pFromData,
                       uint32 nMaxToDataIndex);
void
sbZfKaQsE2QAddr_InitInstance(sbZfKaQsE2QAddr_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSE2QADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSE2QADDR_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSE2QADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSE2QADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KAQSE2QADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSE2QADDR_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSE2QADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSE2QADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSE2QADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSE2QADDR_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSE2QADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSE2QADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KAQSE2QADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSE2QADDR_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSE2QADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSE2QADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSE2QADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[1] << 3; \
           (nToData) |= (uint32) (pFromData)[0] << 11; \
          } while(0)

#define SB_ZF_KAQSE2QADDR_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSE2QADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSE2QADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#else
#define SB_ZF_KAQSE2QADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[2] << 3; \
           (nToData) |= (uint32) (pFromData)[3] << 11; \
          } while(0)

#define SB_ZF_KAQSE2QADDR_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSE2QADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSE2QADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSE2QADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[1] << 3; \
           (nToData) |= (uint32) (pFromData)[0] << 11; \
          } while(0)

#define SB_ZF_KAQSE2QADDR_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSE2QADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSE2QADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#else
#define SB_ZF_KAQSE2QADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[2] << 3; \
           (nToData) |= (uint32) (pFromData)[3] << 11; \
          } while(0)

#define SB_ZF_KAQSE2QADDR_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSE2QADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSE2QADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#endif
#endif
