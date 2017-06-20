/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200BwWctDpEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM3200_WCT_DPENTRY_H
#define SB_ZF_FAB_BM3200_WCT_DPENTRY_H

#define SB_ZF_FAB_BM3200_WCT_DPENTRY_SIZE_IN_BYTES 8
#define SB_ZF_FAB_BM3200_WCT_DPENTRY_SIZE 8
#define SB_ZF_FAB_BM3200_WCT_DPENTRY_M_NECN_BITS "63:48"
#define SB_ZF_FAB_BM3200_WCT_DPENTRY_M_NSCALE_BITS "47:44"
#define SB_ZF_FAB_BM3200_WCT_DPENTRY_M_NSLOPE_BITS "43:32"
#define SB_ZF_FAB_BM3200_WCT_DPENTRY_M_NMIN_BITS "31:16"
#define SB_ZF_FAB_BM3200_WCT_DPENTRY_M_NMAX_BITS "15:0"


typedef struct _sbZfFabBm3200BwWctDpEntry {
  uint32 m_nEcn;
  uint32 m_nScale;
  uint32 m_nSlope;
  uint32 m_nMin;
  uint32 m_nMax;
} sbZfFabBm3200BwWctDpEntry_t;

uint32
sbZfFabBm3200BwWctDpEntry_Pack(sbZfFabBm3200BwWctDpEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwWctDpEntry_Unpack(sbZfFabBm3200BwWctDpEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwWctDpEntry_InitInstance(sbZfFabBm3200BwWctDpEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_ECN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_SCALE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_SLOPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_MIN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_MAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_ECN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_SCALE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_SLOPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_MIN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_MAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_ECN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_SCALE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_SLOPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_MIN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_MAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_ECN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_SCALE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_SLOPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_MIN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_SET_MAX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_ECN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[5] ; \
           (nToData) |= (uint32) (pFromData)[4] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_SCALE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_SLOPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_MIN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[0] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_MAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_ECN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[6] ; \
           (nToData) |= (uint32) (pFromData)[7] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_SCALE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_SLOPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_MIN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[3] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_MAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_ECN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[5] ; \
           (nToData) |= (uint32) (pFromData)[4] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_SCALE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_SLOPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_MIN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[0] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_MAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_ECN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[6] ; \
           (nToData) |= (uint32) (pFromData)[7] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_SCALE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_SLOPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_MIN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[3] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTDPENTRY_GET_MAX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200BwWctDpEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_BM3200_WCT_DPENTRY_CONSOLE_H
#define SB_ZF_FAB_BM3200_WCT_DPENTRY_CONSOLE_H



void
sbZfFabBm3200BwWctDpEntry_Print(sbZfFabBm3200BwWctDpEntry_t *pFromStruct);
int
sbZfFabBm3200BwWctDpEntry_SPrint(sbZfFabBm3200BwWctDpEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm3200BwWctDpEntry_Validate(sbZfFabBm3200BwWctDpEntry_t *pZf);
int
sbZfFabBm3200BwWctDpEntry_SetField(sbZfFabBm3200BwWctDpEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_WCT_DPENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

