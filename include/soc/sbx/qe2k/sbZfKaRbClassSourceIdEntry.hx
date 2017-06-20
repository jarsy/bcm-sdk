/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaRbClassSourceIdEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKARBCLASSSOURCEIDENTRY_H
#define SB_ZF_ZFKARBCLASSSOURCEIDENTRY_H

#define SB_ZF_ZFKARBCLASSSOURCEIDENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKARBCLASSSOURCEIDENTRY_SIZE 4
#define SB_ZF_ZFKARBCLASSSOURCEIDENTRY_M_NRESERVED2_BITS "31:28"
#define SB_ZF_ZFKARBCLASSSOURCEIDENTRY_M_NSRCIDODD_BITS "27:16"
#define SB_ZF_ZFKARBCLASSSOURCEIDENTRY_M_NRESERVED1_BITS "15:12"
#define SB_ZF_ZFKARBCLASSSOURCEIDENTRY_M_NSRCIDEVEN_BITS "11:0"


typedef struct _sbZfKaRbClassSourceIdEntry {
  uint32 m_nReserved2;
  uint32 m_nSrcIdOdd;
  uint32 m_nReserved1;
  uint32 m_nSrcIdEven;
} sbZfKaRbClassSourceIdEntry_t;

uint32
sbZfKaRbClassSourceIdEntry_Pack(sbZfKaRbClassSourceIdEntry_t *pFrom,
                                uint8 *pToData,
                                uint32 nMaxToDataIndex);
void
sbZfKaRbClassSourceIdEntry_Unpack(sbZfKaRbClassSourceIdEntry_t *pToStruct,
                                  uint8 *pFromData,
                                  uint32 nMaxToDataIndex);
void
sbZfKaRbClassSourceIdEntry_InitInstance(sbZfKaRbClassSourceIdEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSSOURCEIDENTRY_SET_RES2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_SET_SRCIDODD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_SET_SRCIDEVEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KARBCLASSSOURCEIDENTRY_SET_RES2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_SET_SRCIDODD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_SET_SRCIDEVEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSSOURCEIDENTRY_SET_RES2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_SET_SRCIDODD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_SET_SRCIDEVEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KARBCLASSSOURCEIDENTRY_SET_RES2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_SET_SRCIDODD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_SET_RES1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_SET_SRCIDEVEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSSOURCEIDENTRY_GET_RES2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_GET_SRCIDODD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_GET_SRCIDEVEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 8; \
          } while(0)

#else
#define SB_ZF_KARBCLASSSOURCEIDENTRY_GET_RES2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_GET_SRCIDODD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_GET_SRCIDEVEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSSOURCEIDENTRY_GET_RES2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_GET_SRCIDODD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_GET_SRCIDEVEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 8; \
          } while(0)

#else
#define SB_ZF_KARBCLASSSOURCEIDENTRY_GET_RES2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_GET_SRCIDODD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_GET_RES1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KARBCLASSSOURCEIDENTRY_GET_SRCIDEVEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 8; \
          } while(0)

#endif
#endif
