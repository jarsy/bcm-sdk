/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQmWredCurvesTableEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQMWREDCURVESTABLEENTRY_H
#define SB_ZF_ZFKAQMWREDCURVESTABLEENTRY_H

#define SB_ZF_ZFKAQMWREDCURVESTABLEENTRY_SIZE_IN_BYTES 8
#define SB_ZF_ZFKAQMWREDCURVESTABLEENTRY_SIZE 8
#define SB_ZF_ZFKAQMWREDCURVESTABLEENTRY_M_NTMIN_BITS "63:48"
#define SB_ZF_ZFKAQMWREDCURVESTABLEENTRY_M_NTMAX_BITS "47:32"
#define SB_ZF_ZFKAQMWREDCURVESTABLEENTRY_M_NTECN_BITS "31:16"
#define SB_ZF_ZFKAQMWREDCURVESTABLEENTRY_M_NSCALE_BITS "15:12"
#define SB_ZF_ZFKAQMWREDCURVESTABLEENTRY_M_NSLOPE_BITS "11:0"


typedef struct _sbZfKaQmWredCurvesTableEntry {
  uint32 m_nTmin;
  uint32 m_nTmax;
  uint32 m_nTecn;
  uint32 m_nScale;
  uint32 m_nSlope;
} sbZfKaQmWredCurvesTableEntry_t;

uint32
sbZfKaQmWredCurvesTableEntry_Pack(sbZfKaQmWredCurvesTableEntry_t *pFrom,
                                  uint8 *pToData,
                                  uint32 nMaxToDataIndex);
void
sbZfKaQmWredCurvesTableEntry_Unpack(sbZfKaQmWredCurvesTableEntry_t *pToStruct,
                                    uint8 *pFromData,
                                    uint32 nMaxToDataIndex);
void
sbZfKaQmWredCurvesTableEntry_InitInstance(sbZfKaQmWredCurvesTableEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_TMIN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_TMAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_TECN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_SCALE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_SLOPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_TMIN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_TMAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_TECN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_SCALE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_SLOPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_TMIN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_TMAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_TECN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_SCALE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_SLOPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_TMIN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_TMAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_TECN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_SCALE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_SET_SLOPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_TMIN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[5] ; \
           (nToData) |= (uint32) (pFromData)[4] << 8; \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_TMAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) (pFromData)[6] << 8; \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_TECN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[0] << 8; \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_SCALE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_SLOPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 8; \
          } while(0)

#else
#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_TMIN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[6] ; \
           (nToData) |= (uint32) (pFromData)[7] << 8; \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_TMAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
           (nToData) |= (uint32) (pFromData)[5] << 8; \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_TECN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[3] << 8; \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_SCALE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_SLOPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_TMIN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[5] ; \
           (nToData) |= (uint32) (pFromData)[4] << 8; \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_TMAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) (pFromData)[6] << 8; \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_TECN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[0] << 8; \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_SCALE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_SLOPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 8; \
          } while(0)

#else
#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_TMIN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[6] ; \
           (nToData) |= (uint32) (pFromData)[7] << 8; \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_TMAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
           (nToData) |= (uint32) (pFromData)[5] << 8; \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_TECN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[3] << 8; \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_SCALE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQMWREDCURVESTABLEENTRY_GET_SLOPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 8; \
          } while(0)

#endif
#endif
