/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsShapeRateEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQSSHAPERATEENTRY_H
#define SB_ZF_ZFKAQSSHAPERATEENTRY_H

#define SB_ZF_ZFKAQSSHAPERATEENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQSSHAPERATEENTRY_SIZE 4
#define SB_ZF_ZFKAQSSHAPERATEENTRY_M_NRESERVED_BITS "31:24"
#define SB_ZF_ZFKAQSSHAPERATEENTRY_M_NMODE_BITS "23:23"
#define SB_ZF_ZFKAQSSHAPERATEENTRY_M_NSHAPERATE_BITS "22:0"


typedef struct _sbZfKaQsShapeRateEntry {
  uint32 m_nReserved;
  uint32 m_nMode;
  uint32 m_nShapeRate;
} sbZfKaQsShapeRateEntry_t;

uint32
sbZfKaQsShapeRateEntry_Pack(sbZfKaQsShapeRateEntry_t *pFrom,
                            uint8 *pToData,
                            uint32 nMaxToDataIndex);
void
sbZfKaQsShapeRateEntry_Unpack(sbZfKaQsShapeRateEntry_t *pToStruct,
                              uint8 *pFromData,
                              uint32 nMaxToDataIndex);
void
sbZfKaQsShapeRateEntry_InitInstance(sbZfKaQsShapeRateEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSSHAPERATEENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAQSSHAPERATEENTRY_SET_MODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSSHAPERATEENTRY_SET_SHAPERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x7f) | (((nFromData) >> 16) & 0x7f); \
          } while(0)

#else
#define SB_ZF_KAQSSHAPERATEENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAQSSHAPERATEENTRY_SET_MODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSSHAPERATEENTRY_SET_SHAPERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x7f) | (((nFromData) >> 16) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSSHAPERATEENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAQSSHAPERATEENTRY_SET_MODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSSHAPERATEENTRY_SET_SHAPERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~ 0x7f) | (((nFromData) >> 16) & 0x7f); \
          } while(0)

#else
#define SB_ZF_KAQSSHAPERATEENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAQSSHAPERATEENTRY_SET_MODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSSHAPERATEENTRY_SET_SHAPERATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~ 0x7f) | (((nFromData) >> 16) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSSHAPERATEENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#define SB_ZF_KAQSSHAPERATEENTRY_GET_MODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSSHAPERATEENTRY_GET_SHAPERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x7f) << 16; \
          } while(0)

#else
#define SB_ZF_KAQSSHAPERATEENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_KAQSSHAPERATEENTRY_GET_MODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSSHAPERATEENTRY_GET_SHAPERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x7f) << 16; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSSHAPERATEENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#define SB_ZF_KAQSSHAPERATEENTRY_GET_MODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSSHAPERATEENTRY_GET_SHAPERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x7f) << 16; \
          } while(0)

#else
#define SB_ZF_KAQSSHAPERATEENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_KAQSSHAPERATEENTRY_GET_MODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSSHAPERATEENTRY_GET_SHAPERATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x7f) << 16; \
          } while(0)

#endif
#endif
