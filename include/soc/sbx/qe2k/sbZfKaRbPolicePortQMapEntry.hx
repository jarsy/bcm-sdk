/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaRbPolicePortQMapEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKARBPOLPORTQMAPENTRY_H
#define SB_ZF_ZFKARBPOLPORTQMAPENTRY_H

#define SB_ZF_ZFKARBPOLPORTQMAPENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKARBPOLPORTQMAPENTRY_SIZE 4
#define SB_ZF_ZFKARBPOLPORTQMAPENTRY_M_NRESERVED2_BITS "31:25"
#define SB_ZF_ZFKARBPOLPORTQMAPENTRY_M_NODDMETER_BITS "24:16"
#define SB_ZF_ZFKARBPOLPORTQMAPENTRY_M_NRESERVED1_BITS "15:9"
#define SB_ZF_ZFKARBPOLPORTQMAPENTRY_M_NEVENMETER_BITS "8:0"


typedef struct _sbZfKaRbPolicePortQMapEntry {
  uint32 m_nReserved2;
  uint32 m_nOddMeter;
  uint32 m_nReserved1;
  uint32 m_nEvenMeter;
} sbZfKaRbPolicePortQMapEntry_t;

uint32
sbZfKaRbPolicePortQMapEntry_Pack(sbZfKaRbPolicePortQMapEntry_t *pFrom,
                                 uint8 *pToData,
                                 uint32 nMaxToDataIndex);
void
sbZfKaRbPolicePortQMapEntry_Unpack(sbZfKaRbPolicePortQMapEntry_t *pToStruct,
                                   uint8 *pFromData,
                                   uint32 nMaxToDataIndex);
void
sbZfKaRbPolicePortQMapEntry_InitInstance(sbZfKaRbPolicePortQMapEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_RES2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_ODDMETER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_EVENMETER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#else
#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_RES2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_ODDMETER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_EVENMETER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_RES2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_ODDMETER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_EVENMETER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#else
#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_RES2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_ODDMETER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_SET_EVENMETER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x01) | (((nFromData) >> 8) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_RES2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_ODDMETER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x01) << 8; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_EVENMETER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x01) << 8; \
          } while(0)

#else
#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_RES2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_ODDMETER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x01) << 8; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_EVENMETER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x01) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_RES2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_ODDMETER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x01) << 8; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_EVENMETER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x01) << 8; \
          } while(0)

#else
#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_RES2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_ODDMETER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x01) << 8; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KARBPOLICEPORTQMAPENTRY_GET_EVENMETER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x01) << 8; \
          } while(0)

#endif
#endif
