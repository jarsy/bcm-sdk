/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600XbXcfgRemapEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_XBXCFGREMAPENTRY_H
#define SB_ZF_FAB_BM9600_XBXCFGREMAPENTRY_H

#define SB_ZF_FAB_BM9600_XBXCFGREMAPENTRY_SIZE_IN_BYTES 2
#define SB_ZF_FAB_BM9600_XBXCFGREMAPENTRY_SIZE 2
#define SB_ZF_FAB_BM9600_XBXCFGREMAPENTRY_M_UXCFGB_BITS "15:8"
#define SB_ZF_FAB_BM9600_XBXCFGREMAPENTRY_M_UXCFGA_BITS "7:0"


typedef struct _sbZfFabBm9600XbXcfgRemapEntry {
  uint32 m_uXcfgB;
  uint32 m_uXcfgA;
} sbZfFabBm9600XbXcfgRemapEntry_t;

uint32
sbZfFabBm9600XbXcfgRemapEntry_Pack(sbZfFabBm9600XbXcfgRemapEntry_t *pFrom,
                                   uint8 *pToData,
                                   uint32 nMaxToDataIndex);
void
sbZfFabBm9600XbXcfgRemapEntry_Unpack(sbZfFabBm9600XbXcfgRemapEntry_t *pToStruct,
                                     uint8 *pFromData,
                                     uint32 nMaxToDataIndex);
void
sbZfFabBm9600XbXcfgRemapEntry_InitInstance(sbZfFabBm9600XbXcfgRemapEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600XBXCFGREMAPENTRY_SET_XCFGB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM9600XBXCFGREMAPENTRY_SET_XCFGA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_FABBM9600XBXCFGREMAPENTRY_SET_XCFGB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM9600XBXCFGREMAPENTRY_SET_XCFGA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600XBXCFGREMAPENTRY_SET_XCFGB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM9600XBXCFGREMAPENTRY_SET_XCFGA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_FABBM9600XBXCFGREMAPENTRY_SET_XCFGB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM9600XBXCFGREMAPENTRY_SET_XCFGA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600XBXCFGREMAPENTRY_GET_XCFGB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
          } while(0)

#define SB_ZF_FABBM9600XBXCFGREMAPENTRY_GET_XCFGA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_FABBM9600XBXCFGREMAPENTRY_GET_XCFGB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
          } while(0)

#define SB_ZF_FABBM9600XBXCFGREMAPENTRY_GET_XCFGA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600XBXCFGREMAPENTRY_GET_XCFGB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
          } while(0)

#define SB_ZF_FABBM9600XBXCFGREMAPENTRY_GET_XCFGA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_FABBM9600XBXCFGREMAPENTRY_GET_XCFGB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
          } while(0)

#define SB_ZF_FABBM9600XBXCFGREMAPENTRY_GET_XCFGA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#endif
