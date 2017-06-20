/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQmWredAvQlenTableEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQMWREDAVQLENTABLEENTRY_H
#define SB_ZF_ZFKAQMWREDAVQLENTABLEENTRY_H

#define SB_ZF_ZFKAQMWREDAVQLENTABLEENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQMWREDAVQLENTABLEENTRY_SIZE 4
#define SB_ZF_ZFKAQMWREDAVQLENTABLEENTRY_M_NQUEUEAVG_BITS "24:0"


typedef struct _sbZfKaQmWredAvQlenTableEntry {
  uint32 m_nQueueAvg;
} sbZfKaQmWredAvQlenTableEntry_t;

uint32
sbZfKaQmWredAvQlenTableEntry_Pack(sbZfKaQmWredAvQlenTableEntry_t *pFrom,
                                  uint8 *pToData,
                                  uint32 nMaxToDataIndex);
void
sbZfKaQmWredAvQlenTableEntry_Unpack(sbZfKaQmWredAvQlenTableEntry_t *pToStruct,
                                    uint8 *pFromData,
                                    uint32 nMaxToDataIndex);
void
sbZfKaQmWredAvQlenTableEntry_InitInstance(sbZfKaQmWredAvQlenTableEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDAVQLENTABLEENTRY_SET_QUEUEAVG(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 24) & 0x01); \
          } while(0)

#else
#define SB_ZF_KAQMWREDAVQLENTABLEENTRY_SET_QUEUEAVG(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 24) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDAVQLENTABLEENTRY_SET_QUEUEAVG(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 24) & 0x01); \
          } while(0)

#else
#define SB_ZF_KAQMWREDAVQLENTABLEENTRY_SET_QUEUEAVG(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 24) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDAVQLENTABLEENTRY_GET_QUEUEAVG(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x01) << 24; \
          } while(0)

#else
#define SB_ZF_KAQMWREDAVQLENTABLEENTRY_GET_QUEUEAVG(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x01) << 24; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDAVQLENTABLEENTRY_GET_QUEUEAVG(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x01) << 24; \
          } while(0)

#else
#define SB_ZF_KAQMWREDAVQLENTABLEENTRY_GET_QUEUEAVG(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x01) << 24; \
          } while(0)

#endif
#endif
