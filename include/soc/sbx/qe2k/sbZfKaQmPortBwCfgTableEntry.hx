/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQmPortBwCfgTableEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQMPORTBWCFGTABLEENTRY_H
#define SB_ZF_ZFKAQMPORTBWCFGTABLEENTRY_H

#define SB_ZF_ZFKAQMPORTBWCFGTABLEENTRY_SIZE_IN_BYTES 8
#define SB_ZF_ZFKAQMPORTBWCFGTABLEENTRY_SIZE 8
#define SB_ZF_ZFKAQMPORTBWCFGTABLEENTRY_M_NSPQUEUES_BITS "45:41"
#define SB_ZF_ZFKAQMPORTBWCFGTABLEENTRY_M_NQUEUES_BITS "40:36"
#define SB_ZF_ZFKAQMPORTBWCFGTABLEENTRY_M_NBASEQUEUE_BITS "35:22"
#define SB_ZF_ZFKAQMPORTBWCFGTABLEENTRY_M_NLINERATE_BITS "21:0"


typedef struct _sbZfKaQmPortBwCfgTableEntry {
  uint32 m_nSpQueues;
  uint32 m_nQueues;
  uint32 m_nBaseQueue;
  uint32 m_nLineRate;
} sbZfKaQmPortBwCfgTableEntry_t;

uint32
sbZfKaQmPortBwCfgTableEntry_Pack(sbZfKaQmPortBwCfgTableEntry_t *pFrom,
                                 uint8 *pToData,
                                 uint32 nMaxToDataIndex);
void
sbZfKaQmPortBwCfgTableEntry_Unpack(sbZfKaQmPortBwCfgTableEntry_t *pToStruct,
                                   uint8 *pFromData,
                                   uint32 nMaxToDataIndex);
void
sbZfKaQmPortBwCfgTableEntry_InitInstance(sbZfKaQmPortBwCfgTableEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_SET_SPQUEUES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x1f << 1)) | (((nFromData) & 0x1f) << 1); \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_SET_QUEUES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[6] = ((pToData)[6] & ~ 0x01) | (((nFromData) >> 4) & 0x01); \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_SET_BASEQUEUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 10) & 0x0f); \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_SET_LINERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 16) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_SET_SPQUEUES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x1f << 1)) | (((nFromData) & 0x1f) << 1); \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_SET_QUEUES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~ 0x01) | (((nFromData) >> 4) & 0x01); \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_SET_BASEQUEUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 10) & 0x0f); \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_SET_LINERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 16) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_SET_SPQUEUES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x1f << 1)) | (((nFromData) & 0x1f) << 1); \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_SET_QUEUES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[6] = ((pToData)[6] & ~ 0x01) | (((nFromData) >> 4) & 0x01); \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_SET_BASEQUEUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 10) & 0x0f); \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_SET_LINERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 16) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_SET_SPQUEUES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x1f << 1)) | (((nFromData) & 0x1f) << 1); \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_SET_QUEUES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~ 0x01) | (((nFromData) >> 4) & 0x01); \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_SET_BASEQUEUE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 10) & 0x0f); \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_SET_LINERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 16) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_GET_SPQUEUES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 1) & 0x1f; \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_GET_QUEUES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x01) << 4; \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_GET_BASEQUEUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[0] << 2; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x0f) << 10; \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_GET_LINERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 16; \
          } while(0)

#else
#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_GET_SPQUEUES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 1) & 0x1f; \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_GET_QUEUES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x01) << 4; \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_GET_BASEQUEUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[3] << 2; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x0f) << 10; \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_GET_LINERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 16; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_GET_SPQUEUES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 1) & 0x1f; \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_GET_QUEUES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x01) << 4; \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_GET_BASEQUEUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[0] << 2; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x0f) << 10; \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_GET_LINERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 16; \
          } while(0)

#else
#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_GET_SPQUEUES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 1) & 0x1f; \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_GET_QUEUES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x01) << 4; \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_GET_BASEQUEUE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[3] << 2; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x0f) << 10; \
          } while(0)

#define SB_ZF_KAQMPORTBWCFGTABLEENTRY_GET_LINERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 16; \
          } while(0)

#endif
#endif
