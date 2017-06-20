/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600NmPortsetLinkEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_NMPORTSETLINKENTRY_H
#define SB_ZF_FAB_BM9600_NMPORTSETLINKENTRY_H

#define SB_ZF_FAB_BM9600_NMPORTSETLINKENTRY_SIZE_IN_BYTES 2
#define SB_ZF_FAB_BM9600_NMPORTSETLINKENTRY_SIZE 2
#define SB_ZF_FAB_BM9600_NMPORTSETLINKENTRY_M_UINDEX_BITS "15:8"
#define SB_ZF_FAB_BM9600_NMPORTSETLINKENTRY_M_UNXTPTR_BITS "7:0"


typedef struct _sbZfFabBm9600NmPortsetLinkEntry {
  uint32 m_uIndex;
  uint32 m_uNxtPtr;
} sbZfFabBm9600NmPortsetLinkEntry_t;

uint32
sbZfFabBm9600NmPortsetLinkEntry_Pack(sbZfFabBm9600NmPortsetLinkEntry_t *pFrom,
                                     uint8 *pToData,
                                     uint32 nMaxToDataIndex);
void
sbZfFabBm9600NmPortsetLinkEntry_Unpack(sbZfFabBm9600NmPortsetLinkEntry_t *pToStruct,
                                       uint8 *pFromData,
                                       uint32 nMaxToDataIndex);
void
sbZfFabBm9600NmPortsetLinkEntry_InitInstance(sbZfFabBm9600NmPortsetLinkEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMPORTSETLINKENTRY_SET_INDEX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETLINKENTRY_SET_NXTPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_FABBM9600NMPORTSETLINKENTRY_SET_INDEX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETLINKENTRY_SET_NXTPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMPORTSETLINKENTRY_SET_INDEX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETLINKENTRY_SET_NXTPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_FABBM9600NMPORTSETLINKENTRY_SET_INDEX(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETLINKENTRY_SET_NXTPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMPORTSETLINKENTRY_GET_INDEX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETLINKENTRY_GET_NXTPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_FABBM9600NMPORTSETLINKENTRY_GET_INDEX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETLINKENTRY_GET_NXTPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMPORTSETLINKENTRY_GET_INDEX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETLINKENTRY_GET_NXTPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_FABBM9600NMPORTSETLINKENTRY_GET_INDEX(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETLINKENTRY_GET_NXTPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#endif
