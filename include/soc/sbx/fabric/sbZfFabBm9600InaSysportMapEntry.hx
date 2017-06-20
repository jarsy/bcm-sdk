/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600InaSysportMapEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_INASYSPORTMAPENTRY_H
#define SB_ZF_FAB_BM9600_INASYSPORTMAPENTRY_H

#define SB_ZF_FAB_BM9600_INASYSPORTMAPENTRY_SIZE_IN_BYTES 2
#define SB_ZF_FAB_BM9600_INASYSPORTMAPENTRY_SIZE 2
#define SB_ZF_FAB_BM9600_INASYSPORTMAPENTRY_M_UPORTSETADDR_BITS "11:4"
#define SB_ZF_FAB_BM9600_INASYSPORTMAPENTRY_M_UOFFSET_BITS "3:0"


typedef struct _sbZfFabBm9600InaSysportMapEntry {
  uint32 m_uPortsetAddr;
  uint32 m_uOffset;
} sbZfFabBm9600InaSysportMapEntry_t;

uint32
sbZfFabBm9600InaSysportMapEntry_Pack(sbZfFabBm9600InaSysportMapEntry_t *pFrom,
                                     uint8 *pToData,
                                     uint32 nMaxToDataIndex);
void
sbZfFabBm9600InaSysportMapEntry_Unpack(sbZfFabBm9600InaSysportMapEntry_t *pToStruct,
                                       uint8 *pFromData,
                                       uint32 nMaxToDataIndex);
void
sbZfFabBm9600InaSysportMapEntry_InitInstance(sbZfFabBm9600InaSysportMapEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INASYSPORTMAPENTRY_SET_PORTSETADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 4) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INASYSPORTMAPENTRY_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_FABBM9600INASYSPORTMAPENTRY_SET_PORTSETADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 4) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INASYSPORTMAPENTRY_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INASYSPORTMAPENTRY_SET_PORTSETADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 4) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INASYSPORTMAPENTRY_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_FABBM9600INASYSPORTMAPENTRY_SET_PORTSETADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 4) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INASYSPORTMAPENTRY_SET_OFFSET(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INASYSPORTMAPENTRY_GET_PORTSETADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 4; \
          } while(0)

#define SB_ZF_FABBM9600INASYSPORTMAPENTRY_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_FABBM9600INASYSPORTMAPENTRY_GET_PORTSETADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 4; \
          } while(0)

#define SB_ZF_FABBM9600INASYSPORTMAPENTRY_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INASYSPORTMAPENTRY_GET_PORTSETADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 4; \
          } while(0)

#define SB_ZF_FABBM9600INASYSPORTMAPENTRY_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_FABBM9600INASYSPORTMAPENTRY_GET_PORTSETADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 4; \
          } while(0)

#define SB_ZF_FABBM9600INASYSPORTMAPENTRY_GET_OFFSET(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
