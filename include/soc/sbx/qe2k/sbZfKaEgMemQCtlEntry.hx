/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEgMemQCtlEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEGMEMQCTLENTRY_H
#define SB_ZF_ZFKAEGMEMQCTLENTRY_H

#define SB_ZF_ZFKAEGMEMQCTLENTRY_SIZE_IN_BYTES 8
#define SB_ZF_ZFKAEGMEMQCTLENTRY_SIZE 8
#define SB_ZF_ZFKAEGMEMQCTLENTRY_M_NDROPONFULL_BITS "25:25"
#define SB_ZF_ZFKAEGMEMQCTLENTRY_M_NWPTR_BITS "24:19"
#define SB_ZF_ZFKAEGMEMQCTLENTRY_M_NRPTR_BITS "18:13"
#define SB_ZF_ZFKAEGMEMQCTLENTRY_M_NSIZE_BITS "12:10"
#define SB_ZF_ZFKAEGMEMQCTLENTRY_M_NBASE_BITS "9:0"


typedef struct _sbZfKaEgMemQCtlEntry {
  uint32 m_nDropOnFull;
  uint32 m_nWptr;
  uint32 m_nRptr;
  uint32 m_nSize;
  uint32 m_nBase;
} sbZfKaEgMemQCtlEntry_t;

uint32
sbZfKaEgMemQCtlEntry_Pack(sbZfKaEgMemQCtlEntry_t *pFrom,
                          uint8 *pToData,
                          uint32 nMaxToDataIndex);
void
sbZfKaEgMemQCtlEntry_Unpack(sbZfKaEgMemQCtlEntry_t *pToStruct,
                            uint8 *pFromData,
                            uint32 nMaxToDataIndex);
void
sbZfKaEgMemQCtlEntry_InitInstance(sbZfKaEgMemQCtlEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMQCTLENTRY_SET_DROPONFULL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_WPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_RPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~ 0x07) | (((nFromData) >> 3) & 0x07); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_SSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 2)) | (((nFromData) & 0x07) << 2); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_BASE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_KAEGMEMQCTLENTRY_SET_DROPONFULL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_WPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_RPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~ 0x07) | (((nFromData) >> 3) & 0x07); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_SSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 2)) | (((nFromData) & 0x07) << 2); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_BASE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMQCTLENTRY_SET_DROPONFULL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_WPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_RPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~ 0x07) | (((nFromData) >> 3) & 0x07); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_SSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 2)) | (((nFromData) & 0x07) << 2); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_BASE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_KAEGMEMQCTLENTRY_SET_DROPONFULL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_WPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 5) & 0x01); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_RPTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~ 0x07) | (((nFromData) >> 3) & 0x07); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_SSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 2)) | (((nFromData) & 0x07) << 2); \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_SET_BASE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMQCTLENTRY_GET_DROPONFULL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_WPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 3) & 0x1f; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x01) << 5; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_RPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x07) << 3; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_SSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x07; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_BASE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_KAEGMEMQCTLENTRY_GET_DROPONFULL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_WPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 3) & 0x1f; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x01) << 5; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_RPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x07) << 3; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_SSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x07; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_BASE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGMEMQCTLENTRY_GET_DROPONFULL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_WPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 3) & 0x1f; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x01) << 5; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_RPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x07) << 3; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_SSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x07; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_BASE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_KAEGMEMQCTLENTRY_GET_DROPONFULL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_WPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 3) & 0x1f; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x01) << 5; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_RPTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x07) << 3; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_SSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x07; \
          } while(0)

#define SB_ZF_KAEGMEMQCTLENTRY_GET_BASE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#endif
