/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQmBaaCfgTableEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQMBAACFGTABLEENTRY_H
#define SB_ZF_ZFKAQMBAACFGTABLEENTRY_H

#define SB_ZF_ZFKAQMBAACFGTABLEENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQMBAACFGTABLEENTRY_SIZE 4
#define SB_ZF_ZFKAQMBAACFGTABLEENTRY_M_NGAMMA_BITS "7:0"


typedef struct _sbZfKaQmBaaCfgTableEntry {
  uint32 m_nGamma;
} sbZfKaQmBaaCfgTableEntry_t;

uint32
sbZfKaQmBaaCfgTableEntry_Pack(sbZfKaQmBaaCfgTableEntry_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex);
void
sbZfKaQmBaaCfgTableEntry_Unpack(sbZfKaQmBaaCfgTableEntry_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex);
void
sbZfKaQmBaaCfgTableEntry_InitInstance(sbZfKaQmBaaCfgTableEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMBAACFGTABLEENTRY_SET_GAMMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAQMBAACFGTABLEENTRY_SET_GAMMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMBAACFGTABLEENTRY_SET_GAMMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAQMBAACFGTABLEENTRY_SET_GAMMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMBAACFGTABLEENTRY_GET_GAMMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAQMBAACFGTABLEENTRY_GET_GAMMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMBAACFGTABLEENTRY_GET_GAMMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAQMBAACFGTABLEENTRY_GET_GAMMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#endif
