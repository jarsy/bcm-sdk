/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQmQueueByteAdjEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQMQUEUEBYTEADJENTRY_H
#define SB_ZF_ZFKAQMQUEUEBYTEADJENTRY_H

#define SB_ZF_ZFKAQMQUEUEBYTEADJENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQMQUEUEBYTEADJENTRY_SIZE 4
#define SB_ZF_ZFKAQMQUEUEBYTEADJENTRY_M_NSIGN_BITS "6:6"
#define SB_ZF_ZFKAQMQUEUEBYTEADJENTRY_M_NBYTES_BITS "5:0"


typedef struct _sbZfKaQmQueueByteAdjEntry {
  uint32 m_nSign;
  uint32 m_nBytes;
} sbZfKaQmQueueByteAdjEntry_t;

uint32
sbZfKaQmQueueByteAdjEntry_Pack(sbZfKaQmQueueByteAdjEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex);
void
sbZfKaQmQueueByteAdjEntry_Unpack(sbZfKaQmQueueByteAdjEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex);
void
sbZfKaQmQueueByteAdjEntry_InitInstance(sbZfKaQmQueueByteAdjEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMQUEUEBYTEADJENTRY_SET_SIGN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMQUEUEBYTEADJENTRY_SET_BYTES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KAQMQUEUEBYTEADJENTRY_SET_SIGN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMQUEUEBYTEADJENTRY_SET_BYTES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMQUEUEBYTEADJENTRY_SET_SIGN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMQUEUEBYTEADJENTRY_SET_BYTES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KAQMQUEUEBYTEADJENTRY_SET_SIGN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMQUEUEBYTEADJENTRY_SET_BYTES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMQUEUEBYTEADJENTRY_GET_SIGN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUEBYTEADJENTRY_GET_BYTES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#else
#define SB_ZF_KAQMQUEUEBYTEADJENTRY_GET_SIGN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUEBYTEADJENTRY_GET_BYTES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMQUEUEBYTEADJENTRY_GET_SIGN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUEBYTEADJENTRY_GET_BYTES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#else
#define SB_ZF_KAQMQUEUEBYTEADJENTRY_GET_SIGN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMQUEUEBYTEADJENTRY_GET_BYTES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#endif
#endif
