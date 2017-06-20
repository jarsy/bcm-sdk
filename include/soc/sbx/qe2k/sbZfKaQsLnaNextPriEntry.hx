/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsLnaNextPriEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQSLNANEXTPRIENTRY_H
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_H

#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_SIZE_IN_BYTES 20
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_SIZE 20
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NSELPORT_BITS "105:100"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI24_BITS "99:96"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI23_BITS "95:92"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI22_BITS "91:88"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI21_BITS "87:84"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI20_BITS "83:80"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI19_BITS "79:76"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI18_BITS "75:72"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI17_BITS "71:68"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI16_BITS "67:64"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI15_BITS "63:60"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI14_BITS "59:56"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI13_BITS "55:52"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI12_BITS "51:48"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI11_BITS "47:44"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI10_BITS "43:40"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI9_BITS "39:36"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI8_BITS "35:32"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI7_BITS "31:28"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI6_BITS "27:24"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI5_BITS "23:20"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI4_BITS "19:16"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI3_BITS "15:12"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI2_BITS "11:8"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI1_BITS "7:4"
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_M_NNEXTPRI0_BITS "3:0"


typedef struct _sbZfKaQsLnaNextPriEntry {
  uint32 m_nSelPort;
  uint32 m_nNextPri24;
  uint32 m_nNextPri23;
  uint32 m_nNextPri22;
  uint32 m_nNextPri21;
  uint32 m_nNextPri20;
  uint32 m_nNextPri19;
  uint32 m_nNextPri18;
  uint32 m_nNextPri17;
  uint32 m_nNextPri16;
  uint32 m_nNextPri15;
  uint32 m_nNextPri14;
  uint32 m_nNextPri13;
  uint32 m_nNextPri12;
  uint32 m_nNextPri11;
  uint32 m_nNextPri10;
  uint32 m_nNextPri9;
  uint32 m_nNextPri8;
  uint32 m_nNextPri7;
  uint32 m_nNextPri6;
  uint32 m_nNextPri5;
  uint32 m_nNextPri4;
  uint32 m_nNextPri3;
  uint32 m_nNextPri2;
  uint32 m_nNextPri1;
  uint32 m_nNextPri0;
} sbZfKaQsLnaNextPriEntry_t;

uint32
sbZfKaQsLnaNextPriEntry_Pack(sbZfKaQsLnaNextPriEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex);
void
sbZfKaQsLnaNextPriEntry_Unpack(sbZfKaQsLnaNextPriEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex);
void
sbZfKaQsLnaNextPriEntry_InitInstance(sbZfKaQsLnaNextPriEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_SELPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[14] = ((pToData)[14] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI24(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI23(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI22(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI21(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI20(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI19(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI18(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI17(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_SELPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[13] = ((pToData)[13] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI24(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI23(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI22(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI21(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI20(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI19(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI18(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI17(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_SELPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[14] = ((pToData)[14] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI24(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI23(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI22(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI21(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI20(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI19(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI18(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI17(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_SELPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[13] = ((pToData)[13] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI24(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI23(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI22(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI21(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI20(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI19(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI18(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI17(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_SET_NEXTPRI0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_SELPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[14] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI24(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI23(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI22(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI21(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI20(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI19(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI18(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI17(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_SELPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[13] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI24(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI23(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI22(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI21(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI20(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI19(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI18(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI17(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_SELPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[14] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI24(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI23(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI22(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI21(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI20(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI19(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI18(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI17(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_SELPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[13] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI24(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI23(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI22(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI21(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI20(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI19(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI18(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI17(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSLNANEXTPRIENTRY_GET_NEXTPRI0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
