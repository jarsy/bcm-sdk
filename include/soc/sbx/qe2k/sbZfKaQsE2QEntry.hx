/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsE2QEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQSE2QENTRY_H
#define SB_ZF_ZFKAQSE2QENTRY_H

#define SB_ZF_ZFKAQSE2QENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQSE2QENTRY_SIZE 4
#define SB_ZF_ZFKAQSE2QENTRY_M_NRESERVED_BITS "31:15"
#define SB_ZF_ZFKAQSE2QENTRY_M_NENABLE_BITS "14:14"
#define SB_ZF_ZFKAQSE2QENTRY_M_NQUEUENUM_BITS "13:0"


typedef struct _sbZfKaQsE2QEntry {
  uint32 m_nReserved;
  uint8 m_nEnable;
  uint32 m_nQueueNum;
} sbZfKaQsE2QEntry_t;

uint32
sbZfKaQsE2QEntry_Pack(sbZfKaQsE2QEntry_t *pFrom,
                      uint8 *pToData,
                      uint32 nMaxToDataIndex);
void
sbZfKaQsE2QEntry_Unpack(sbZfKaQsE2QEntry_t *pToStruct,
                        uint8 *pFromData,
                        uint32 nMaxToDataIndex);
void
sbZfKaQsE2QEntry_InitInstance(sbZfKaQsE2QEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSE2QENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSE2QENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQSE2QENTRY_SET_QUEUENUM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KAQSE2QENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSE2QENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQSE2QENTRY_SET_QUEUENUM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSE2QENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSE2QENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQSE2QENTRY_SET_QUEUENUM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KAQSE2QENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSE2QENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQSE2QENTRY_SET_QUEUENUM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSE2QENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[1] << 1; \
           (nToData) |= (uint32) (pFromData)[0] << 9; \
          } while(0)

#define SB_ZF_KAQSE2QENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQSE2QENTRY_GET_QUEUENUM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 8; \
          } while(0)

#else
#define SB_ZF_KAQSE2QENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[2] << 1; \
           (nToData) |= (uint32) (pFromData)[3] << 9; \
          } while(0)

#define SB_ZF_KAQSE2QENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQSE2QENTRY_GET_QUEUENUM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSE2QENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[1] << 1; \
           (nToData) |= (uint32) (pFromData)[0] << 9; \
          } while(0)

#define SB_ZF_KAQSE2QENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQSE2QENTRY_GET_QUEUENUM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 8; \
          } while(0)

#else
#define SB_ZF_KAQSE2QENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[2] << 1; \
           (nToData) |= (uint32) (pFromData)[3] << 9; \
          } while(0)

#define SB_ZF_KAQSE2QENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQSE2QENTRY_GET_QUEUENUM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 8; \
          } while(0)

#endif
#endif
