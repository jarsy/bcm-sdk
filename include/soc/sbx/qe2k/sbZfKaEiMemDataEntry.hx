/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEiMemDataEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEIMEMDATAENTRY_H
#define SB_ZF_ZFKAEIMEMDATAENTRY_H

#define SB_ZF_ZFKAEIMEMDATAENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAEIMEMDATAENTRY_SIZE 4
#define SB_ZF_ZFKAEIMEMDATAENTRY_M_NDESTCHANNEL_BITS "31:24"
#define SB_ZF_ZFKAEIMEMDATAENTRY_M_NSIZEMASK_BITS "23:15"
#define SB_ZF_ZFKAEIMEMDATAENTRY_M_NRBONLY_BITS "14:14"
#define SB_ZF_ZFKAEIMEMDATAENTRY_M_NLINEPTR_BITS "13:4"
#define SB_ZF_ZFKAEIMEMDATAENTRY_M_NBYTEPTR_BITS "3:0"


typedef struct _sbZfKaEiMemDataEntry {
  uint32 m_nDestChannel;
  uint32 m_nSizeMask;
  uint32 m_nRbOnly;
  uint32 m_nLinePtr;
  uint32 m_nBytePtr;
} sbZfKaEiMemDataEntry_t;

uint32
sbZfKaEiMemDataEntry_Pack(sbZfKaEiMemDataEntry_t *pFrom,
                          uint8 *pToData,
                          uint32 nMaxToDataIndex);
void
sbZfKaEiMemDataEntry_Unpack(sbZfKaEiMemDataEntry_t *pToStruct,
                            uint8 *pFromData,
                            uint32 nMaxToDataIndex);
void
sbZfKaEiMemDataEntry_InitInstance(sbZfKaEiMemDataEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEIMEMDATAENTRY_SET_DESTCHANNEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_SET_SIZEMASK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_SET_RB_ONLY(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_SET_LINEPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_SET_BYTEPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAEIMEMDATAENTRY_SET_DESTCHANNEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_SET_SIZEMASK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_SET_RB_ONLY(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_SET_LINEPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_SET_BYTEPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEIMEMDATAENTRY_SET_DESTCHANNEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_SET_SIZEMASK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_SET_RB_ONLY(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_SET_LINEPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_SET_BYTEPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAEIMEMDATAENTRY_SET_DESTCHANNEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_SET_SIZEMASK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_SET_RB_ONLY(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_SET_LINEPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_SET_BYTEPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEIMEMDATAENTRY_GET_DESTCHANNEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_GET_SIZEMASK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[1] << 1; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_GET_RB_ONLY(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_GET_LINEPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_GET_BYTEPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAEIMEMDATAENTRY_GET_DESTCHANNEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_GET_SIZEMASK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[2] << 1; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_GET_RB_ONLY(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_GET_LINEPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_GET_BYTEPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEIMEMDATAENTRY_GET_DESTCHANNEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_GET_SIZEMASK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[1] << 1; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_GET_RB_ONLY(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_GET_LINEPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_GET_BYTEPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAEIMEMDATAENTRY_GET_DESTCHANNEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_GET_SIZEMASK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[2] << 1; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_GET_RB_ONLY(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_GET_LINEPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAEIMEMDATAENTRY_GET_BYTEPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
