/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600NmPortsetInfoEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_H
#define SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_H

#define SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_SIZE_IN_BYTES 3
#define SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_SIZE 3
#define SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_M_UVIRTUALPORT_BITS "18:18"
#define SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_M_UVPORTEOPP_BITS "17:15"
#define SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_M_USTARTPORT_BITS "14:7"
#define SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_M_UEGNODE_BITS "6:0"


typedef struct _sbZfFabBm9600NmPortsetInfoEntry {
  uint32 m_uVirtualPort;
  uint32 m_uVportEopp;
  uint32 m_uStartPort;
  uint32 m_uEgNode;
} sbZfFabBm9600NmPortsetInfoEntry_t;

uint32
sbZfFabBm9600NmPortsetInfoEntry_Pack(sbZfFabBm9600NmPortsetInfoEntry_t *pFrom,
                                     uint8 *pToData,
                                     uint32 nMaxToDataIndex);
void
sbZfFabBm9600NmPortsetInfoEntry_Unpack(sbZfFabBm9600NmPortsetInfoEntry_t *pToStruct,
                                       uint8 *pFromData,
                                       uint32 nMaxToDataIndex);
void
sbZfFabBm9600NmPortsetInfoEntry_InitInstance(sbZfFabBm9600NmPortsetInfoEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_VIRTUALPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_VPORTEOPP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_STARTPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x7f) | (((nFromData) >> 1) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_EGNODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#else
#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_VIRTUALPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_VPORTEOPP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_STARTPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x7f) | (((nFromData) >> 1) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_EGNODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_VIRTUALPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_VPORTEOPP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_STARTPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x7f) | (((nFromData) >> 1) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_EGNODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#else
#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_VIRTUALPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_VPORTEOPP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_STARTPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x7f) | (((nFromData) >> 1) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_SET_EGNODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_VIRTUALPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_VPORTEOPP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 1; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_STARTPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x7f) << 1; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_EGNODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x7f; \
          } while(0)

#else
#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_VIRTUALPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_VPORTEOPP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 1; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_STARTPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x7f) << 1; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_EGNODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x7f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_VIRTUALPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_VPORTEOPP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 1; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_STARTPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x7f) << 1; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_EGNODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x7f; \
          } while(0)

#else
#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_VIRTUALPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_VPORTEOPP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 1; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_STARTPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x7f) << 1; \
          } while(0)

#define SB_ZF_FABBM9600NMPORTSETINFOENTRY_GET_EGNODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x7f; \
          } while(0)

#endif
#endif
