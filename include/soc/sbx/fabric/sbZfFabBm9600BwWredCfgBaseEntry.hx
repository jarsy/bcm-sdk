/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600BwWredCfgBaseEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_BWWREDCFGBASEENTRY_H
#define SB_ZF_FAB_BM9600_BWWREDCFGBASEENTRY_H

#define SB_ZF_FAB_BM9600_BWWREDCFGBASEENTRY_SIZE_IN_BYTES 3
#define SB_ZF_FAB_BM9600_BWWREDCFGBASEENTRY_SIZE 3
#define SB_ZF_FAB_BM9600_BWWREDCFGBASEENTRY_M_UENC_BITS "22:21"
#define SB_ZF_FAB_BM9600_BWWREDCFGBASEENTRY_M_UNUMVOQS_BITS "20:16"
#define SB_ZF_FAB_BM9600_BWWREDCFGBASEENTRY_M_UBASEVOQ_BITS "15:0"


typedef struct _sbZfFabBm9600BwWredCfgBaseEntry {
  uint32 m_uEnc;
  uint32 m_uNumVoqs;
  uint32 m_uBaseVoq;
} sbZfFabBm9600BwWredCfgBaseEntry_t;

uint32
sbZfFabBm9600BwWredCfgBaseEntry_Pack(sbZfFabBm9600BwWredCfgBaseEntry_t *pFrom,
                                     uint8 *pToData,
                                     uint32 nMaxToDataIndex);
void
sbZfFabBm9600BwWredCfgBaseEntry_Unpack(sbZfFabBm9600BwWredCfgBaseEntry_t *pToStruct,
                                       uint8 *pFromData,
                                       uint32 nMaxToDataIndex);
void
sbZfFabBm9600BwWredCfgBaseEntry_InitInstance(sbZfFabBm9600BwWredCfgBaseEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_SET_ENC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 5)) | (((nFromData) & 0x03) << 5); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_SET_NUMVOQS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_SET_BASEVOQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_SET_ENC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 5)) | (((nFromData) & 0x03) << 5); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_SET_NUMVOQS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_SET_BASEVOQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_SET_ENC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 5)) | (((nFromData) & 0x03) << 5); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_SET_NUMVOQS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_SET_BASEVOQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_SET_ENC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 5)) | (((nFromData) & 0x03) << 5); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_SET_NUMVOQS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_SET_BASEVOQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_GET_ENC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x03; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_GET_NUMVOQS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x1f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_GET_BASEVOQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_GET_ENC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x03; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_GET_NUMVOQS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x1f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_GET_BASEVOQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_GET_ENC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x03; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_GET_NUMVOQS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x1f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_GET_BASEVOQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_GET_ENC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x03; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_GET_NUMVOQS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x1f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDCFGBASEENTRY_GET_BASEVOQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
          } while(0)

#endif
#endif
