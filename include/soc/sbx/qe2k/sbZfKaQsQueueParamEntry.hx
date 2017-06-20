/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsQueueParamEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQSQUEUEPARAMENTRY_H
#define SB_ZF_ZFKAQSQUEUEPARAMENTRY_H

#define SB_ZF_ZFKAQSQUEUEPARAMENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQSQUEUEPARAMENTRY_SIZE 4
#define SB_ZF_ZFKAQSQUEUEPARAMENTRY_M_NRESERVED_BITS "31:8"
#define SB_ZF_ZFKAQSQUEUEPARAMENTRY_M_NLOCAL_BITS "7:7"
#define SB_ZF_ZFKAQSQUEUEPARAMENTRY_M_NMAXHOLDTS_BITS "6:4"
#define SB_ZF_ZFKAQSQUEUEPARAMENTRY_M_NQTYPE_BITS "3:0"


typedef struct _sbZfKaQsQueueParamEntry {
  uint32 m_nReserved;
  uint32 m_nLocal;
  uint32 m_nMaxHoldTs;
  uint32 m_nQType;
} sbZfKaQsQueueParamEntry_t;

uint32
sbZfKaQsQueueParamEntry_Pack(sbZfKaQsQueueParamEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex);
void
sbZfKaQsQueueParamEntry_Unpack(sbZfKaQsQueueParamEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex);
void
sbZfKaQsQueueParamEntry_InitInstance(sbZfKaQsQueueParamEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSQUEUEPARAMENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_SET_LOCAL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_SET_MAXHOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_SET_QTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAQSQUEUEPARAMENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_SET_LOCAL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_SET_MAXHOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_SET_QTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSQUEUEPARAMENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_SET_LOCAL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_SET_MAXHOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_SET_QTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAQSQUEUEPARAMENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_SET_LOCAL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_SET_MAXHOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_SET_QTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSQUEUEPARAMENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[0] << 16; \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_GET_LOCAL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_GET_MAXHOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_GET_QTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAQSQUEUEPARAMENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[3] << 16; \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_GET_LOCAL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_GET_MAXHOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_GET_QTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSQUEUEPARAMENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[0] << 16; \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_GET_LOCAL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_GET_MAXHOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_GET_QTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAQSQUEUEPARAMENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[3] << 16; \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_GET_LOCAL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_GET_MAXHOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KAQSQUEUEPARAMENTRY_GET_QTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
