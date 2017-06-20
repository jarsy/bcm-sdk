/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsShapeTableEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQSSHAPETABLEENTRY_H
#define SB_ZF_ZFKAQSSHAPETABLEENTRY_H

#define SB_ZF_ZFKAQSSHAPETABLEENTRY_SIZE_IN_BYTES 8
#define SB_ZF_ZFKAQSSHAPETABLEENTRY_SIZE 8
#define SB_ZF_ZFKAQSSHAPETABLEENTRY_M_NSHAPERATELS2B_BITS "63:48"
#define SB_ZF_ZFKAQSSHAPETABLEENTRY_M_NENABLE_BITS "47:47"
#define SB_ZF_ZFKAQSSHAPETABLEENTRY_M_NSHAPEMAXBURST_BITS "46:24"
#define SB_ZF_ZFKAQSSHAPETABLEENTRY_M_NSHAPE_BITS "23:0"


typedef struct _sbZfKaQsShapeTableEntry {
  uint32 m_nShapeRateLS2B;
  uint32 m_nEnable;
  uint32 m_nShapeMaxBurst;
  uint32 m_nShape;
} sbZfKaQsShapeTableEntry_t;

uint32
sbZfKaQsShapeTableEntry_Pack(sbZfKaQsShapeTableEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex);
void
sbZfKaQsShapeTableEntry_Unpack(sbZfKaQsShapeTableEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex);
void
sbZfKaQsShapeTableEntry_InitInstance(sbZfKaQsShapeTableEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSSHAPETABLEENTRY_SET_SHAPERATELS2B(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_SET_SHAPEMAXBURST(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x7f) | (((nFromData) >> 16) & 0x7f); \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_SET_SHAPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#else
#define SB_ZF_KAQSSHAPETABLEENTRY_SET_SHAPERATELS2B(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_SET_SHAPEMAXBURST(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~ 0x7f) | (((nFromData) >> 16) & 0x7f); \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_SET_SHAPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSSHAPETABLEENTRY_SET_SHAPERATELS2B(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_SET_SHAPEMAXBURST(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x7f) | (((nFromData) >> 16) & 0x7f); \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_SET_SHAPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#else
#define SB_ZF_KAQSSHAPETABLEENTRY_SET_SHAPERATELS2B(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_SET_ENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_SET_SHAPEMAXBURST(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~ 0x7f) | (((nFromData) >> 16) & 0x7f); \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_SET_SHAPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSSHAPETABLEENTRY_GET_SHAPERATELS2B(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[5] ; \
           (nToData) |= (uint32) (pFromData)[4] << 8; \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_GET_SHAPEMAXBURST(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[7] << 8; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x7f) << 16; \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_GET_SHAPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
          } while(0)

#else
#define SB_ZF_KAQSSHAPETABLEENTRY_GET_SHAPERATELS2B(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[6] ; \
           (nToData) |= (uint32) (pFromData)[7] << 8; \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_GET_SHAPEMAXBURST(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[4] << 8; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x7f) << 16; \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_GET_SHAPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSSHAPETABLEENTRY_GET_SHAPERATELS2B(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[5] ; \
           (nToData) |= (uint32) (pFromData)[4] << 8; \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_GET_SHAPEMAXBURST(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[7] << 8; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x7f) << 16; \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_GET_SHAPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
          } while(0)

#else
#define SB_ZF_KAQSSHAPETABLEENTRY_GET_SHAPERATELS2B(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[6] ; \
           (nToData) |= (uint32) (pFromData)[7] << 8; \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_GET_ENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_GET_SHAPEMAXBURST(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[4] << 8; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x7f) << 16; \
          } while(0)

#define SB_ZF_KAQSSHAPETABLEENTRY_GET_SHAPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
          } while(0)

#endif
#endif
