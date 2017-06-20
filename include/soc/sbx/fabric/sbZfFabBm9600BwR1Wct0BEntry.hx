/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600BwR1Wct0BEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_BWR1WCT0BENTRY_H
#define SB_ZF_FAB_BM9600_BWR1WCT0BENTRY_H

#define SB_ZF_FAB_BM9600_BWR1WCT0BENTRY_SIZE_IN_BYTES 4
#define SB_ZF_FAB_BM9600_BWR1WCT0BENTRY_SIZE 4
#define SB_ZF_FAB_BM9600_BWR1WCT0BENTRY_M_UTECNDP0_BITS "31:16"
#define SB_ZF_FAB_BM9600_BWR1WCT0BENTRY_M_USCALEDP0_BITS "15:12"
#define SB_ZF_FAB_BM9600_BWR1WCT0BENTRY_M_USLOPEDP0_BITS "11:0"


typedef struct _sbZfFabBm9600BwR1Wct0BEntry {
  uint32 m_uTEcnDp0;
  uint32 m_uScaleDp0;
  uint32 m_uSlopeDp0;
} sbZfFabBm9600BwR1Wct0BEntry_t;

uint32
sbZfFabBm9600BwR1Wct0BEntry_Pack(sbZfFabBm9600BwR1Wct0BEntry_t *pFrom,
                                 uint8 *pToData,
                                 uint32 nMaxToDataIndex);
void
sbZfFabBm9600BwR1Wct0BEntry_Unpack(sbZfFabBm9600BwR1Wct0BEntry_t *pToStruct,
                                   uint8 *pFromData,
                                   uint32 nMaxToDataIndex);
void
sbZfFabBm9600BwR1Wct0BEntry_InitInstance(sbZfFabBm9600BwR1Wct0BEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWR1WCT0BENTRY_SET_TECNDP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT0BENTRY_SET_SCALEDP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT0BENTRY_SET_SLOPEDP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#else
#define SB_ZF_FABBM9600BWR1WCT0BENTRY_SET_TECNDP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT0BENTRY_SET_SCALEDP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT0BENTRY_SET_SLOPEDP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWR1WCT0BENTRY_SET_TECNDP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT0BENTRY_SET_SCALEDP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT0BENTRY_SET_SLOPEDP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#else
#define SB_ZF_FABBM9600BWR1WCT0BENTRY_SET_TECNDP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT0BENTRY_SET_SCALEDP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT0BENTRY_SET_SLOPEDP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWR1WCT0BENTRY_GET_TECNDP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[0] << 8; \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT0BENTRY_GET_SCALEDP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT0BENTRY_GET_SLOPEDP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600BWR1WCT0BENTRY_GET_TECNDP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[3] << 8; \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT0BENTRY_GET_SCALEDP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT0BENTRY_GET_SLOPEDP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWR1WCT0BENTRY_GET_TECNDP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[0] << 8; \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT0BENTRY_GET_SCALEDP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT0BENTRY_GET_SLOPEDP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600BWR1WCT0BENTRY_GET_TECNDP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[3] << 8; \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT0BENTRY_GET_SCALEDP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWR1WCT0BENTRY_GET_SLOPEDP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 8; \
          } while(0)

#endif
#endif
