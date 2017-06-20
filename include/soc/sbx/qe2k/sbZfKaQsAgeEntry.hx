/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsAgeEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQSAGEENTRY_H
#define SB_ZF_ZFKAQSAGEENTRY_H

#define SB_ZF_ZFKAQSAGEENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQSAGEENTRY_SIZE 4
#define SB_ZF_ZFKAQSAGEENTRY_M_NRESERVED_BITS "31:11"
#define SB_ZF_ZFKAQSAGEENTRY_M_NNOEMPTY_BITS "10:10"
#define SB_ZF_ZFKAQSAGEENTRY_M_NANEMICEVENT_BITS "9:9"
#define SB_ZF_ZFKAQSAGEENTRY_M_NEFEVENT_BITS "8:8"
#define SB_ZF_ZFKAQSAGEENTRY_M_NCNT_BITS "7:0"


typedef struct _sbZfKaQsAgeEntry {
  uint32 m_nReserved;
  uint8 m_nNoEmpty;
  uint8 m_nAnemicEvent;
  uint8 m_nEfEvent;
  uint32 m_nCnt;
} sbZfKaQsAgeEntry_t;

uint32
sbZfKaQsAgeEntry_Pack(sbZfKaQsAgeEntry_t *pFrom,
                      uint8 *pToData,
                      uint32 nMaxToDataIndex);
void
sbZfKaQsAgeEntry_Unpack(sbZfKaQsAgeEntry_t *pToStruct,
                        uint8 *pFromData,
                        uint32 nMaxToDataIndex);
void
sbZfKaQsAgeEntry_InitInstance(sbZfKaQsAgeEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSAGEENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_SET_NOEMPTY(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_SET_ANEMICEVENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_SET_EFEVENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_SET_CNT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAQSAGEENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_SET_NOEMPTY(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_SET_ANEMICEVENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_SET_EFEVENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_SET_CNT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSAGEENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_SET_NOEMPTY(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_SET_ANEMICEVENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_SET_EFEVENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_SET_CNT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAQSAGEENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 5) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 13) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_SET_NOEMPTY(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_SET_ANEMICEVENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_SET_EFEVENT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_SET_CNT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSAGEENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 3) & 0x1f; \
           (nToData) |= (uint32) (pFromData)[1] << 5; \
           (nToData) |= (uint32) (pFromData)[0] << 13; \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_GET_NOEMPTY(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_GET_ANEMICEVENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_GET_EFEVENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_GET_CNT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAQSAGEENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 3) & 0x1f; \
           (nToData) |= (uint32) (pFromData)[2] << 5; \
           (nToData) |= (uint32) (pFromData)[3] << 13; \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_GET_NOEMPTY(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_GET_ANEMICEVENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_GET_EFEVENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_GET_CNT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSAGEENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 3) & 0x1f; \
           (nToData) |= (uint32) (pFromData)[1] << 5; \
           (nToData) |= (uint32) (pFromData)[0] << 13; \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_GET_NOEMPTY(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_GET_ANEMICEVENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_GET_EFEVENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_GET_CNT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAQSAGEENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 3) & 0x1f; \
           (nToData) |= (uint32) (pFromData)[2] << 5; \
           (nToData) |= (uint32) (pFromData)[3] << 13; \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_GET_NOEMPTY(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_GET_ANEMICEVENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_GET_EFEVENT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KAQSAGEENTRY_GET_CNT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#endif
