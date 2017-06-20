/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsQ2EcEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQSQ2ECENTRY_H
#define SB_ZF_ZFKAQSQ2ECENTRY_H

#define SB_ZF_ZFKAQSQ2ECENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQSQ2ECENTRY_SIZE 4
#define SB_ZF_ZFKAQSQ2ECENTRY_M_NRESERVED_BITS "31:17"
#define SB_ZF_ZFKAQSQ2ECENTRY_M_NMC_BITS "16:16"
#define SB_ZF_ZFKAQSQ2ECENTRY_M_NNODE_BITS "15:10"
#define SB_ZF_ZFKAQSQ2ECENTRY_M_NPORT_BITS "9:4"
#define SB_ZF_ZFKAQSQ2ECENTRY_M_NCOS_BITS "3:0"


typedef struct _sbZfKaQsQ2EcEntry {
  uint32 m_nReserved;
  uint32 m_nMc;
  uint32 m_nNode;
  uint32 m_nPort;
  uint32 m_nCos;
} sbZfKaQsQ2EcEntry_t;

uint32
sbZfKaQsQ2EcEntry_Pack(sbZfKaQsQ2EcEntry_t *pFrom,
                       uint8 *pToData,
                       uint32 nMaxToDataIndex);
void
sbZfKaQsQ2EcEntry_Unpack(sbZfKaQsQ2EcEntry_t *pToStruct,
                         uint8 *pFromData,
                         uint32 nMaxToDataIndex);
void
sbZfKaQsQ2EcEntry_InitInstance(sbZfKaQsQ2EcEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSQ2ECENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 7) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_SET_COS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAQSQ2ECENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 7) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_SET_COS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSQ2ECENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 7) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_SET_COS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAQSQ2ECENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 7) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_SET_COS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSQ2ECENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 1) & 0x7f; \
           (nToData) |= (uint32) (pFromData)[0] << 7; \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_GET_COS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAQSQ2ECENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x7f; \
           (nToData) |= (uint32) (pFromData)[3] << 7; \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_GET_COS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSQ2ECENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 1) & 0x7f; \
           (nToData) |= (uint32) (pFromData)[0] << 7; \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_GET_COS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAQSQ2ECENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x7f; \
           (nToData) |= (uint32) (pFromData)[3] << 7; \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSQ2ECENTRY_GET_COS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
