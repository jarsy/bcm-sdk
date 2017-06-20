/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsLnaPortRemapEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQSPORTREMAPENTRY_H
#define SB_ZF_ZFKAQSPORTREMAPENTRY_H

#define SB_ZF_ZFKAQSPORTREMAPENTRY_SIZE_IN_BYTES 20
#define SB_ZF_ZFKAQSPORTREMAPENTRY_SIZE 20
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NRES_BITS "159:150"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT24_BITS "149:144"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT23_BITS "143:138"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT22_BITS "137:132"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT21_BITS "131:126"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT20_BITS "125:120"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT19_BITS "119:114"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT18_BITS "113:108"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT17_BITS "107:102"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT16_BITS "101:96"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT15_BITS "95:90"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT14_BITS "89:84"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT13_BITS "83:78"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT12_BITS "77:72"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT11_BITS "71:66"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT10_BITS "65:60"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT9_BITS "59:54"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT8_BITS "53:48"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT7_BITS "47:42"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT6_BITS "41:36"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT5_BITS "35:30"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT4_BITS "29:24"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT3_BITS "23:18"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT2_BITS "17:12"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT1_BITS "11:6"
#define SB_ZF_ZFKAQSPORTREMAPENTRY_M_NPORT0_BITS "5:0"


typedef struct _sbZfKaQsLnaPortRemapEntry {
  uint32 m_nRes;
  uint32 m_nPort24;
  uint32 m_nPort23;
  uint32 m_nPort22;
  uint32 m_nPort21;
  uint32 m_nPort20;
  uint32 m_nPort19;
  uint32 m_nPort18;
  uint32 m_nPort17;
  uint32 m_nPort16;
  uint32 m_nPort15;
  uint32 m_nPort14;
  uint32 m_nPort13;
  uint32 m_nPort12;
  uint32 m_nPort11;
  uint32 m_nPort10;
  uint32 m_nPort9;
  uint32 m_nPort8;
  uint32 m_nPort7;
  uint32 m_nPort6;
  uint32 m_nPort5;
  uint32 m_nPort4;
  uint32 m_nPort3;
  uint32 m_nPort2;
  uint32 m_nPort1;
  uint32 m_nPort0;
} sbZfKaQsLnaPortRemapEntry_t;

uint32
sbZfKaQsLnaPortRemapEntry_Pack(sbZfKaQsLnaPortRemapEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex);
void
sbZfKaQsLnaPortRemapEntry_Unpack(sbZfKaQsLnaPortRemapEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex);
void
sbZfKaQsLnaPortRemapEntry_InitInstance(sbZfKaQsLnaPortRemapEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT24(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT23(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((pToData)[18] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT22(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((pToData)[19] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[18] = ((pToData)[18] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT21(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[19] = ((pToData)[19] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT20(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT19(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT18(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[13] = ((pToData)[13] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT17(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[14] = ((pToData)[14] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[8] = ((pToData)[8] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[9] = ((pToData)[9] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[11] = ((pToData)[11] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[6] = ((pToData)[6] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((pToData)[18] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT24(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((pToData)[18] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT23(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT22(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[17] = ((pToData)[17] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT21(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[16] = ((pToData)[16] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT20(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT19(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT18(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[14] = ((pToData)[14] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT17(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[13] = ((pToData)[13] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[11] = ((pToData)[11] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[10] = ((pToData)[10] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[8] = ((pToData)[8] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT24(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT23(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((pToData)[18] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT22(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((pToData)[19] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[18] = ((pToData)[18] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT21(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[19] = ((pToData)[19] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT20(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT19(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT18(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[13] = ((pToData)[13] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT17(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[14] = ((pToData)[14] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[8] = ((pToData)[8] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[9] = ((pToData)[9] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[11] = ((pToData)[11] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[6] = ((pToData)[6] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((pToData)[18] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT24(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((pToData)[18] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT23(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT22(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[17] = ((pToData)[17] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT21(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[16] = ((pToData)[16] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT20(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT19(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT18(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[14] = ((pToData)[14] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT17(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[13] = ((pToData)[13] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[11] = ((pToData)[11] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[10] = ((pToData)[10] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[8] = ((pToData)[8] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 4) & 0x03); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 2) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_SET_PORT0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[17] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[16] << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT24(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[17]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT23(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[18] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT22(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[19] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[18] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT21(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[19] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT20(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT19(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[13] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT18(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[14] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[13] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT17(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[14] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[8] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[9] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[11] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#else
#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[18] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[19] << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT24(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[18]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT23(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[17] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT22(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[16] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[17] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT21(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[16] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT20(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT19(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[14] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT18(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[13] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[14] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT17(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[13] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[11] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[10] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[8] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[17] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[16] << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT24(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[17]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT23(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[18] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT22(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[19] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[18] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT21(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[19] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT20(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT19(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[13] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT18(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[14] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[13] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT17(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[14] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[8] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[9] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[11] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#else
#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[18] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[19] << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT24(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[18]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT23(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[17] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT22(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[16] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[17] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT21(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[16] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT20(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT19(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[14] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT18(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[13] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[14] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT17(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[13] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[11] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[10] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[8] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x3f; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 4; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 2; \
          } while(0)

#define SB_ZF_KAQSLNAPORTREMAPENTRY_GET_PORT0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#endif
#endif
