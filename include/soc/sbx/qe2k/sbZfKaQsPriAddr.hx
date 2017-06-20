/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsPriAddr.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQSPRIADDR_H
#define SB_ZF_ZFKAQSPRIADDR_H

#define SB_ZF_ZFKAQSPRIADDR_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQSPRIADDR_SIZE 4
#define SB_ZF_ZFKAQSPRIADDR_M_NRESERVED_BITS "31:16"
#define SB_ZF_ZFKAQSPRIADDR_M_NCOS_BITS "15:13"
#define SB_ZF_ZFKAQSPRIADDR_M_NMC_BITS "12:12"
#define SB_ZF_ZFKAQSPRIADDR_M_NNODE_BITS "11:7"
#define SB_ZF_ZFKAQSPRIADDR_M_NPORT_BITS "6:0"


typedef struct _sbZfKaQsPriAddr {
  uint32 m_nReserved;
  uint32 m_nCos;
  uint32 m_nMc;
  uint32 m_nNode;
  uint32 m_nPort;
} sbZfKaQsPriAddr_t;

uint32
sbZfKaQsPriAddr_Pack(sbZfKaQsPriAddr_t *pFrom,
                     uint8 *pToData,
                     uint32 nMaxToDataIndex);
void
sbZfKaQsPriAddr_Unpack(sbZfKaQsPriAddr_t *pToStruct,
                       uint8 *pFromData,
                       uint32 nMaxToDataIndex);
void
sbZfKaQsPriAddr_InitInstance(sbZfKaQsPriAddr_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSPRIADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSPRIADDR_SET_COS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_KAQSPRIADDR_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSPRIADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 1) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSPRIADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#else
#define SB_ZF_KAQSPRIADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSPRIADDR_SET_COS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_KAQSPRIADDR_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSPRIADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 1) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSPRIADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSPRIADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSPRIADDR_SET_COS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_KAQSPRIADDR_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSPRIADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 1) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSPRIADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#else
#define SB_ZF_KAQSPRIADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSPRIADDR_SET_COS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_KAQSPRIADDR_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSPRIADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 1) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSPRIADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSPRIADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[0] << 8; \
          } while(0)

#define SB_ZF_KAQSPRIADDR_GET_COS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_KAQSPRIADDR_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRIADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 1; \
          } while(0)

#define SB_ZF_KAQSPRIADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x7f; \
          } while(0)

#else
#define SB_ZF_KAQSPRIADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[3] << 8; \
          } while(0)

#define SB_ZF_KAQSPRIADDR_GET_COS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_KAQSPRIADDR_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRIADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 1; \
          } while(0)

#define SB_ZF_KAQSPRIADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x7f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSPRIADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[0] << 8; \
          } while(0)

#define SB_ZF_KAQSPRIADDR_GET_COS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_KAQSPRIADDR_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRIADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 1; \
          } while(0)

#define SB_ZF_KAQSPRIADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x7f; \
          } while(0)

#else
#define SB_ZF_KAQSPRIADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[3] << 8; \
          } while(0)

#define SB_ZF_KAQSPRIADDR_GET_COS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_KAQSPRIADDR_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRIADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 1; \
          } while(0)

#define SB_ZF_KAQSPRIADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x7f; \
          } while(0)

#endif
#endif
