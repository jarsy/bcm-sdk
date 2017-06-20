/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQmWredParamEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQMWREDPARAMENTRY_H
#define SB_ZF_ZFKAQMWREDPARAMENTRY_H

#define SB_ZF_ZFKAQMWREDPARAMENTRY_SIZE_IN_BYTES 8
#define SB_ZF_ZFKAQMWREDPARAMENTRY_SIZE 8
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NTMAXEXCEEDED2_BITS "35:35"
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NECNEXCEEDED2_BITS "34:34"
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NPDROP2_BITS "33:24"
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NTMAXEXCEEDED1_BITS "23:23"
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NECNEXCEEDED1_BITS "22:22"
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NPDROP1_BITS "21:12"
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NTMAXEXCEEDED0_BITS "11:11"
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NECNEXCEEDED0_BITS "10:10"
#define SB_ZF_ZFKAQMWREDPARAMENTRY_M_NPDROP0_BITS "9:0"


typedef struct _sbZfKaQmWredParamEntry {
  uint8 m_nTMaxExceeded2;
  uint8 m_nEcnExceeded2;
  uint32 m_nPDrop2;
  uint8 m_nTMaxExceeded1;
  uint8 m_nEcnExceeded1;
  uint32 m_nPDrop1;
  uint8 m_nTMaxExceeded0;
  uint8 m_nEcnExceeded0;
  uint32 m_nPDrop0;
} sbZfKaQmWredParamEntry_t;

uint32
sbZfKaQmWredParamEntry_Pack(sbZfKaQmWredParamEntry_t *pFrom,
                            uint8 *pToData,
                            uint32 nMaxToDataIndex);
void
sbZfKaQmWredParamEntry_Unpack(sbZfKaQmWredParamEntry_t *pToStruct,
                              uint8 *pFromData,
                              uint32 nMaxToDataIndex);
void
sbZfKaQmWredParamEntry_InitInstance(sbZfKaQmWredParamEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_TMAXEXCEEDED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_ECNEXCEEDED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_SET_PDROP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[7] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[7] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x03) << 8; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[4] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[4] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x03) << 8; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[7] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[7] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x03) << 8; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[4] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[4] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x03) << 8; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_TMAXEXCEEDED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_ECNEXCEEDED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQMWREDPARAMENTRY_GET_PDROP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#endif
