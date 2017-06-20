/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsDepthHplenEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQSDEPTHHPLENENTRY_H
#define SB_ZF_ZFKAQSDEPTHHPLENENTRY_H

#define SB_ZF_ZFKAQSDEPTHHPLENENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQSDEPTHHPLENENTRY_SIZE 4
#define SB_ZF_ZFKAQSDEPTHHPLENENTRY_M_NRESERVED_BITS "31:6"
#define SB_ZF_ZFKAQSDEPTHHPLENENTRY_M_NHPLEN_BITS "5:4"
#define SB_ZF_ZFKAQSDEPTHHPLENENTRY_M_NDEPTH_BITS "3:0"


typedef struct _sbZfKaQsDepthHplenEntry {
  uint32 m_nReserved;
  uint32 m_nHplen;
  uint32 m_nDepth;
} sbZfKaQsDepthHplenEntry_t;

uint32
sbZfKaQsDepthHplenEntry_Pack(sbZfKaQsDepthHplenEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex);
void
sbZfKaQsDepthHplenEntry_Unpack(sbZfKaQsDepthHplenEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex);
void
sbZfKaQsDepthHplenEntry_InitInstance(sbZfKaQsDepthHplenEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSDEPTHHPLENENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSDEPTHHPLENENTRY_SET_HPLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KAQSDEPTHHPLENENTRY_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAQSDEPTHHPLENENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSDEPTHHPLENENTRY_SET_HPLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KAQSDEPTHHPLENENTRY_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSDEPTHHPLENENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSDEPTHHPLENENTRY_SET_HPLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KAQSDEPTHHPLENENTRY_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAQSDEPTHHPLENENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 10) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 18) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSDEPTHHPLENENTRY_SET_HPLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 4)) | (((nFromData) & 0x03) << 4); \
          } while(0)

#define SB_ZF_KAQSDEPTHHPLENENTRY_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSDEPTHHPLENENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[2] << 2; \
           (nToData) |= (uint32) (pFromData)[1] << 10; \
           (nToData) |= (uint32) (pFromData)[0] << 18; \
          } while(0)

#define SB_ZF_KAQSDEPTHHPLENENTRY_GET_HPLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KAQSDEPTHHPLENENTRY_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAQSDEPTHHPLENENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[1] << 2; \
           (nToData) |= (uint32) (pFromData)[2] << 10; \
           (nToData) |= (uint32) (pFromData)[3] << 18; \
          } while(0)

#define SB_ZF_KAQSDEPTHHPLENENTRY_GET_HPLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KAQSDEPTHHPLENENTRY_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSDEPTHHPLENENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[2] << 2; \
           (nToData) |= (uint32) (pFromData)[1] << 10; \
           (nToData) |= (uint32) (pFromData)[0] << 18; \
          } while(0)

#define SB_ZF_KAQSDEPTHHPLENENTRY_GET_HPLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KAQSDEPTHHPLENENTRY_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAQSDEPTHHPLENENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[1] << 2; \
           (nToData) |= (uint32) (pFromData)[2] << 10; \
           (nToData) |= (uint32) (pFromData)[3] << 18; \
          } while(0)

#define SB_ZF_KAQSDEPTHHPLENENTRY_GET_HPLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x03; \
          } while(0)

#define SB_ZF_KAQSDEPTHHPLENENTRY_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
