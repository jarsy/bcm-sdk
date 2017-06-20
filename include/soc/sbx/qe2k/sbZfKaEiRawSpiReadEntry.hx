/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEiRawSpiReadEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEIRAWSPIREADENTRY_H
#define SB_ZF_ZFKAEIRAWSPIREADENTRY_H

#define SB_ZF_ZFKAEIRAWSPIREADENTRY_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAEIRAWSPIREADENTRY_SIZE 4
#define SB_ZF_ZFKAEIRAWSPIREADENTRY_M_NDESTCHANNEL_BITS "31:24"
#define SB_ZF_ZFKAEIRAWSPIREADENTRY_M_NSIZEMASK8_BITS "23:23"
#define SB_ZF_ZFKAEIRAWSPIREADENTRY_M_NSIZEMASK7_0_BITS "22:15"
#define SB_ZF_ZFKAEIRAWSPIREADENTRY_M_NRBLOOPBACKONLY_BITS "14:14"
#define SB_ZF_ZFKAEIRAWSPIREADENTRY_M_NLINEPTR_BITS "13:4"
#define SB_ZF_ZFKAEIRAWSPIREADENTRY_M_NBYTEPTR_BITS "3:0"


typedef struct _sbZfKaEiRawSpiReadEntry {
  uint32 m_nDestChannel;
  uint32 m_nSizeMask8;
  uint32 m_nSizeMask7_0;
  uint32 m_nRbLoopbackOnly;
  uint32 m_nLinePtr;
  uint32 m_nBytePtr;
} sbZfKaEiRawSpiReadEntry_t;

uint32
sbZfKaEiRawSpiReadEntry_Pack(sbZfKaEiRawSpiReadEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex);
void
sbZfKaEiRawSpiReadEntry_Unpack(sbZfKaEiRawSpiReadEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex);
void
sbZfKaEiRawSpiReadEntry_InitInstance(sbZfKaEiRawSpiReadEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEIRAWSPIREADENTRY_SET_DESTCHANNEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_SIZEMASK8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_SIZEMASK7_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x7f) | (((nFromData) >> 1) & 0x7f); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_RB_LOOPBACK_ONLY(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_LINE_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_BYTE_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAEIRAWSPIREADENTRY_SET_DESTCHANNEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_SIZEMASK8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_SIZEMASK7_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x7f) | (((nFromData) >> 1) & 0x7f); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_RB_LOOPBACK_ONLY(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_LINE_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_BYTE_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEIRAWSPIREADENTRY_SET_DESTCHANNEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_SIZEMASK8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_SIZEMASK7_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x7f) | (((nFromData) >> 1) & 0x7f); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_RB_LOOPBACK_ONLY(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_LINE_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_BYTE_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAEIRAWSPIREADENTRY_SET_DESTCHANNEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_SIZEMASK8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_SIZEMASK7_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x7f) | (((nFromData) >> 1) & 0x7f); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_RB_LOOPBACK_ONLY(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_LINE_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 4) & 0x3f); \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_SET_BYTE_PTR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEIRAWSPIREADENTRY_GET_DESTCHANNEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_SIZEMASK8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_SIZEMASK7_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x7f) << 1; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_RB_LOOPBACK_ONLY(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_LINE_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_BYTE_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAEIRAWSPIREADENTRY_GET_DESTCHANNEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_SIZEMASK8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_SIZEMASK7_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x7f) << 1; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_RB_LOOPBACK_ONLY(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_LINE_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_BYTE_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEIRAWSPIREADENTRY_GET_DESTCHANNEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_SIZEMASK8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_SIZEMASK7_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x7f) << 1; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_RB_LOOPBACK_ONLY(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_LINE_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_BYTE_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAEIRAWSPIREADENTRY_GET_DESTCHANNEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_SIZEMASK8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_SIZEMASK7_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x7f) << 1; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_RB_LOOPBACK_ONLY(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_LINE_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 4; \
          } while(0)

#define SB_ZF_KAEIRAWSPIREADENTRY_GET_BYTE_PTR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
