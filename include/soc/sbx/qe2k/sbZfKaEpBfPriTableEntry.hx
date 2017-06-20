/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEpBfPriTableEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEPBFPRITABLEENTRY_H
#define SB_ZF_ZFKAEPBFPRITABLEENTRY_H

#define SB_ZF_ZFKAEPBFPRITABLEENTRY_SIZE_IN_BYTES 3
#define SB_ZF_ZFKAEPBFPRITABLEENTRY_SIZE 3
#define SB_ZF_ZFKAEPBFPRITABLEENTRY_M_NPRI7_BITS "23:21"
#define SB_ZF_ZFKAEPBFPRITABLEENTRY_M_NPRI6_BITS "20:18"
#define SB_ZF_ZFKAEPBFPRITABLEENTRY_M_NPRI5_BITS "17:15"
#define SB_ZF_ZFKAEPBFPRITABLEENTRY_M_NPRI4_BITS "14:12"
#define SB_ZF_ZFKAEPBFPRITABLEENTRY_M_NPRI3_BITS "11:9"
#define SB_ZF_ZFKAEPBFPRITABLEENTRY_M_NPRI2_BITS "8:6"
#define SB_ZF_ZFKAEPBFPRITABLEENTRY_M_NPRI1_BITS "5:3"
#define SB_ZF_ZFKAEPBFPRITABLEENTRY_M_NPRI0_BITS "2:0"
#define SB_ZF_ZFKAEPBFPRITABLEENTRY_NUM_PRIS       8


typedef struct _sbZfKaEpBfPriTableEntry {
  uint32 m_nPri7;
  uint32 m_nPri6;
  uint32 m_nPri5;
  uint32 m_nPri4;
  uint32 m_nPri3;
  uint32 m_nPri2;
  uint32 m_nPri1;
  uint32 m_nPri0;
} sbZfKaEpBfPriTableEntry_t;

uint32
sbZfKaEpBfPriTableEntry_Pack(sbZfKaEpBfPriTableEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex);
void
sbZfKaEpBfPriTableEntry_Unpack(sbZfKaEpBfPriTableEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex);
void
sbZfKaEpBfPriTableEntry_InitInstance(sbZfKaEpBfPriTableEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 2)) | (((nFromData) & 0x07) << 2); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 2) & 0x01); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x07) | ((nFromData) & 0x07); \
          } while(0)

#else
#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 2)) | (((nFromData) & 0x07) << 2); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~ 0x01) | (((nFromData) >> 2) & 0x01); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x07) | ((nFromData) & 0x07); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 2)) | (((nFromData) & 0x07) << 2); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~ 0x01) | (((nFromData) >> 2) & 0x01); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x07) | ((nFromData) & 0x07); \
          } while(0)

#else
#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 2)) | (((nFromData) & 0x07) << 2); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 1) & 0x03); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~ 0x01) | (((nFromData) >> 2) & 0x01); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_SET_PRI0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x07) | ((nFromData) & 0x07); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 1; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x01) << 2; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x07; \
          } while(0)

#else
#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 1; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x01) << 2; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x07; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 1; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x01) << 2; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x07; \
          } while(0)

#else
#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 1; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x01) << 2; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_KAEPBFPRITABLEENTRY_GET_PRI0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x07; \
          } while(0)

#endif
#endif
