/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQmQueueAgeEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQMQUEUEAGEENTRY_H
#define SB_ZF_ZFKAQMQUEUEAGEENTRY_H

#define SB_ZF_ZFKAQMQUEUEAGEENTRY_SIZE_IN_BYTES 1
#define SB_ZF_ZFKAQMQUEUEAGEENTRY_SIZE 1
#define SB_ZF_ZFKAQMQUEUEAGEENTRY_M_NAGE_BITS "3:0"


typedef struct _sbZfKaQmQueueAgeEntry {
  uint32 m_nAge;
} sbZfKaQmQueueAgeEntry_t;

uint32
sbZfKaQmQueueAgeEntry_Pack(sbZfKaQmQueueAgeEntry_t *pFrom,
                           uint8 *pToData,
                           uint32 nMaxToDataIndex);
void
sbZfKaQmQueueAgeEntry_Unpack(sbZfKaQmQueueAgeEntry_t *pToStruct,
                             uint8 *pFromData,
                             uint32 nMaxToDataIndex);
void
sbZfKaQmQueueAgeEntry_InitInstance(sbZfKaQmQueueAgeEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMQUEUEAGEENTRY_SET_AGE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAQMQUEUEAGEENTRY_SET_AGE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMQUEUEAGEENTRY_SET_AGE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAQMQUEUEAGEENTRY_SET_AGE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMQUEUEAGEENTRY_GET_AGE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAQMQUEUEAGEENTRY_GET_AGE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMQUEUEAGEENTRY_GET_AGE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAQMQUEUEAGEENTRY_GET_AGE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
