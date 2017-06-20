/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaRbClassDefaultQEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKARBCLASSDEFAULTQENTRY_H
#define SB_ZF_ZFKARBCLASSDEFAULTQENTRY_H

#define SB_ZF_ZFKARBCLASSDEFAULTQENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKARBCLASSDEFAULTQENTRY_SIZE 4
#define SB_ZF_ZFKARBCLASSDEFAULTQENTRY_M_NSPARE1_BITS "31:18"
#define SB_ZF_ZFKARBCLASSDEFAULTQENTRY_M_NDEFAULTDP_BITS "17:16"
#define SB_ZF_ZFKARBCLASSDEFAULTQENTRY_M_NSPARE0_BITS "15:14"
#define SB_ZF_ZFKARBCLASSDEFAULTQENTRY_M_NDEFAULTQ_BITS "13:0"


typedef struct _sbZfKaRbClassDefaultQEntry {
  uint32 m_nSpare1;
  uint32 m_nDefaultDp;
  uint32 m_nSpare0;
  uint32 m_nDefaultQ;
} sbZfKaRbClassDefaultQEntry_t;

uint32
sbZfKaRbClassDefaultQEntry_Pack(sbZfKaRbClassDefaultQEntry_t *pFrom,
                                uint8 *pToData,
                                uint32 nMaxToDataIndex);
void
sbZfKaRbClassDefaultQEntry_Unpack(sbZfKaRbClassDefaultQEntry_t *pToStruct,
                                  uint8 *pFromData,
                                  uint32 nMaxToDataIndex);
void
sbZfKaRbClassDefaultQEntry_InitInstance(sbZfKaRbClassDefaultQEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_SPARE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_DEFAULTDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_SPARE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_DEFAULTQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_SPARE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_DEFAULTDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_SPARE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_DEFAULTQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_SPARE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_DEFAULTDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_SPARE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_DEFAULTQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_SPARE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_DEFAULTDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_SPARE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_SET_DEFAULTQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_SPARE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[0] << 6; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_DEFAULTDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_SPARE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_DEFAULTQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 8; \
          } while(0)

#else
#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_SPARE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[3] << 6; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_DEFAULTDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_SPARE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_DEFAULTQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_SPARE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[0] << 6; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_DEFAULTDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_SPARE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_DEFAULTQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 8; \
          } while(0)

#else
#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_SPARE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[3] << 6; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_DEFAULTDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_SPARE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_KARBCLASSDEFAULTQENTRY_GET_DEFAULTQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 8; \
          } while(0)

#endif
#endif
