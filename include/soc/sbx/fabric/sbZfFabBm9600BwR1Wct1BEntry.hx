/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600BwR1Wct1BEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_BWR1WCT1BENTRY_H
#define SB_ZF_FAB_BM9600_BWR1WCT1BENTRY_H

#define SB_ZF_FAB_BM9600_BWR1WCT1BENTRY_SIZE_IN_BYTES 4
#define SB_ZF_FAB_BM9600_BWR1WCT1BENTRY_SIZE 4
#define SB_ZF_FAB_BM9600_BWR1WCT1BENTRY_M_UTECNDP1_BITS "31:16"
#define SB_ZF_FAB_BM9600_BWR1WCT1BENTRY_M_USCALEDP1_BITS "15:12"
#define SB_ZF_FAB_BM9600_BWR1WCT1BENTRY_M_USLOPEDP1_BITS "11:0"


typedef struct _sbZfFabBm9600BwR1Wct1BEntry {
  uint32 m_uTEcnDp1;
  uint32 m_uScaleDp1;
  uint32 m_uSlopeDp1;
} sbZfFabBm9600BwR1Wct1BEntry_t;

uint32
sbZfFabBm9600BwR1Wct1BEntry_Pack(sbZfFabBm9600BwR1Wct1BEntry_t *pFrom,
                                 uint8 *pToData,
                                 uint32 nMaxToDataIndex);
void
sbZfFabBm9600BwR1Wct1BEntry_Unpack(sbZfFabBm9600BwR1Wct1BEntry_t *pToStruct,
                                   uint8 *pFromData,
                                   uint32 nMaxToDataIndex);
void
sbZfFabBm9600BwR1Wct1BEntry_InitInstance(sbZfFabBm9600BwR1Wct1BEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWR1WCT1BENTRY_SET_TECNDP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT1BENTRY_SET_SCALEDP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT1BENTRY_SET_SLOPEDP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#else
#define SB_ZF_FABBM9600BWR1WCT1BENTRY_SET_TECNDP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT1BENTRY_SET_SCALEDP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT1BENTRY_SET_SLOPEDP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWR1WCT1BENTRY_SET_TECNDP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT1BENTRY_SET_SCALEDP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT1BENTRY_SET_SLOPEDP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#else
#define SB_ZF_FABBM9600BWR1WCT1BENTRY_SET_TECNDP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT1BENTRY_SET_SCALEDP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT1BENTRY_SET_SLOPEDP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWR1WCT1BENTRY_GET_TECNDP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[0] << 8; \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT1BENTRY_GET_SCALEDP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT1BENTRY_GET_SLOPEDP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600BWR1WCT1BENTRY_GET_TECNDP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[3] << 8; \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT1BENTRY_GET_SCALEDP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT1BENTRY_GET_SLOPEDP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWR1WCT1BENTRY_GET_TECNDP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[0] << 8; \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT1BENTRY_GET_SCALEDP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT1BENTRY_GET_SLOPEDP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600BWR1WCT1BENTRY_GET_TECNDP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[3] << 8; \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT1BENTRY_GET_SCALEDP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT1BENTRY_GET_SLOPEDP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 8; \
          } while(0)

#endif
#endif
