/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQmDemandCfgDataEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQMDEMANDCFGDATAENTRY_H
#define SB_ZF_ZFKAQMDEMANDCFGDATAENTRY_H

#define SB_ZF_ZFKAQMDEMANDCFGDATAENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQMDEMANDCFGDATAENTRY_SIZE 4
#define SB_ZF_ZFKAQMDEMANDCFGDATAENTRY_M_NRATEDELTAMAXIDX_BITS "6:1"
#define SB_ZF_ZFKAQMDEMANDCFGDATAENTRY_M_NQLADEMANDMASK_BITS "0:0"


typedef struct _sbZfKaQmDemandCfgDataEntry {
  uint32 m_nRateDeltaMaxIdx;
  uint32 m_nQlaDemandMask;
} sbZfKaQmDemandCfgDataEntry_t;

uint32
sbZfKaQmDemandCfgDataEntry_Pack(sbZfKaQmDemandCfgDataEntry_t *pFrom,
                                uint8 *pToData,
                                uint32 nMaxToDataIndex);
void
sbZfKaQmDemandCfgDataEntry_Unpack(sbZfKaQmDemandCfgDataEntry_t *pToStruct,
                                  uint8 *pFromData,
                                  uint32 nMaxToDataIndex);
void
sbZfKaQmDemandCfgDataEntry_InitInstance(sbZfKaQmDemandCfgDataEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMDEMANDCFGDATAENTRY_SET_RATEDELTAMAXIDS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 1)) | (((nFromData) & 0x3f) << 1); \
          } while(0)

#define SB_ZF_KAQMDEMANDCFGDATAENTRY_SET_QLADEMANDMASK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_KAQMDEMANDCFGDATAENTRY_SET_RATEDELTAMAXIDS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 1)) | (((nFromData) & 0x3f) << 1); \
          } while(0)

#define SB_ZF_KAQMDEMANDCFGDATAENTRY_SET_QLADEMANDMASK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMDEMANDCFGDATAENTRY_SET_RATEDELTAMAXIDS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x3f << 1)) | (((nFromData) & 0x3f) << 1); \
          } while(0)

#define SB_ZF_KAQMDEMANDCFGDATAENTRY_SET_QLADEMANDMASK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_KAQMDEMANDCFGDATAENTRY_SET_RATEDELTAMAXIDS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x3f << 1)) | (((nFromData) & 0x3f) << 1); \
          } while(0)

#define SB_ZF_KAQMDEMANDCFGDATAENTRY_SET_QLADEMANDMASK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMDEMANDCFGDATAENTRY_GET_RATEDELTAMAXIDS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 1) & 0x3f; \
          } while(0)

#define SB_ZF_KAQMDEMANDCFGDATAENTRY_GET_QLADEMANDMASK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_KAQMDEMANDCFGDATAENTRY_GET_RATEDELTAMAXIDS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 1) & 0x3f; \
          } while(0)

#define SB_ZF_KAQMDEMANDCFGDATAENTRY_GET_QLADEMANDMASK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMDEMANDCFGDATAENTRY_GET_RATEDELTAMAXIDS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 1) & 0x3f; \
          } while(0)

#define SB_ZF_KAQMDEMANDCFGDATAENTRY_GET_QLADEMANDMASK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_KAQMDEMANDCFGDATAENTRY_GET_RATEDELTAMAXIDS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 1) & 0x3f; \
          } while(0)

#define SB_ZF_KAQMDEMANDCFGDATAENTRY_GET_QLADEMANDMASK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#endif
