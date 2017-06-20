/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600FoLinkStateTableEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_FOLINKSTATETABLEENTRY_H
#define SB_ZF_FAB_BM9600_FOLINKSTATETABLEENTRY_H

#define SB_ZF_FAB_BM9600_FOLINKSTATETABLEENTRY_SIZE_IN_BYTES 5
#define SB_ZF_FAB_BM9600_FOLINKSTATETABLEENTRY_SIZE 5
#define SB_ZF_FAB_BM9600_FOLINKSTATETABLEENTRY_M_USB_BITS "33:33"
#define SB_ZF_FAB_BM9600_FOLINKSTATETABLEENTRY_M_UERROR_BITS "32:32"
#define SB_ZF_FAB_BM9600_FOLINKSTATETABLEENTRY_M_ULINKSTATE_BITS "31:0"


typedef struct _sbZfFabBm9600FoLinkStateTableEntry {
  uint32 m_uSb;
  uint32 m_uError;
  uint32 m_uLinkState;
} sbZfFabBm9600FoLinkStateTableEntry_t;

uint32
sbZfFabBm9600FoLinkStateTableEntry_Pack(sbZfFabBm9600FoLinkStateTableEntry_t *pFrom,
                                        uint8 *pToData,
                                        uint32 nMaxToDataIndex);
void
sbZfFabBm9600FoLinkStateTableEntry_Unpack(sbZfFabBm9600FoLinkStateTableEntry_t *pToStruct,
                                          uint8 *pFromData,
                                          uint32 nMaxToDataIndex);
void
sbZfFabBm9600FoLinkStateTableEntry_InitInstance(sbZfFabBm9600FoLinkStateTableEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_SET_SB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_SET_ERROR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_SET_LINKSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_SET_SB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_SET_ERROR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_SET_LINKSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_SET_SB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_SET_ERROR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_SET_LINKSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_SET_SB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_SET_ERROR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_SET_LINKSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_GET_SB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_GET_ERROR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7]) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_GET_LINKSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
           (nToData) |= (uint32) (pFromData)[0] << 24; \
          } while(0)

#else
#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_GET_SB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_GET_ERROR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4]) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_GET_LINKSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
           (nToData) |= (uint32) (pFromData)[3] << 24; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_GET_SB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_GET_ERROR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7]) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_GET_LINKSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
           (nToData) |= (uint32) (pFromData)[0] << 24; \
          } while(0)

#else
#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_GET_SB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_GET_ERROR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4]) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOLINKSTATETABLEENTRY_GET_LINKSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
           (nToData) |= (uint32) (pFromData)[3] << 24; \
          } while(0)

#endif
#endif
