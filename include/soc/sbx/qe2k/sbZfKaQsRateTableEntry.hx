/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsRateTableEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQSRATETABLEENTRY_H
#define SB_ZF_ZFKAQSRATETABLEENTRY_H

#define SB_ZF_ZFKAQSRATETABLEENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQSRATETABLEENTRY_SIZE 4
#define SB_ZF_ZFKAQSRATETABLEENTRY_M_NINTEGER_BITS "31:8"
#define SB_ZF_ZFKAQSRATETABLEENTRY_M_NFRACTION_BITS "7:0"


typedef struct _sbZfKaQsRateTableEntry {
  uint32 m_nInteger;
  uint32 m_nFraction;
} sbZfKaQsRateTableEntry_t;

uint32
sbZfKaQsRateTableEntry_Pack(sbZfKaQsRateTableEntry_t *pFrom,
                            uint8 *pToData,
                            uint32 nMaxToDataIndex);
void
sbZfKaQsRateTableEntry_Unpack(sbZfKaQsRateTableEntry_t *pToStruct,
                              uint8 *pFromData,
                              uint32 nMaxToDataIndex);
void
sbZfKaQsRateTableEntry_InitInstance(sbZfKaQsRateTableEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSRATETABLEENTRY_SET_INTEGER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSRATETABLEENTRY_SET_FRACTION(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAQSRATETABLEENTRY_SET_INTEGER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSRATETABLEENTRY_SET_FRACTION(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSRATETABLEENTRY_SET_INTEGER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSRATETABLEENTRY_SET_FRACTION(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAQSRATETABLEENTRY_SET_INTEGER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSRATETABLEENTRY_SET_FRACTION(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSRATETABLEENTRY_GET_INTEGER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[0] << 16; \
          } while(0)

#define SB_ZF_KAQSRATETABLEENTRY_GET_FRACTION(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAQSRATETABLEENTRY_GET_INTEGER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[3] << 16; \
          } while(0)

#define SB_ZF_KAQSRATETABLEENTRY_GET_FRACTION(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSRATETABLEENTRY_GET_INTEGER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[0] << 16; \
          } while(0)

#define SB_ZF_KAQSRATETABLEENTRY_GET_FRACTION(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAQSRATETABLEENTRY_GET_INTEGER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[3] << 16; \
          } while(0)

#define SB_ZF_KAQSRATETABLEENTRY_GET_FRACTION(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#endif
