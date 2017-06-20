/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaSrManualDeskewEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKASRMANUALDESKEWENTRY_H
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_H

#define SB_ZF_ZFKASRMANUALDESKEWENTRY_SIZE_IN_BYTES 11
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_SIZE 11
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_M_NLANE16_BITS "84:80"
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_M_NLANE15_BITS "79:75"
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_M_NLANE14_BITS "74:70"
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_M_NLANE13_BITS "69:65"
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_M_NLANE12_BITS "64:60"
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_M_NLANE11_BITS "59:55"
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_M_NLANE10_BITS "54:50"
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_M_NLANE9_BITS "49:45"
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_M_NLANE8_BITS "44:40"
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_M_NLANE7_BITS "39:35"
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_M_NLANE6_BITS "34:30"
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_M_NLANE5_BITS "29:25"
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_M_NLANE4_BITS "24:20"
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_M_NLANE3_BITS "19:15"
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_M_NLANE2_BITS "14:10"
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_M_NLANE1_BITS "9:5"
#define SB_ZF_ZFKASRMANUALDESKEWENTRY_M_NLANE0_BITS "4:0"


typedef struct _sbZfKaSrManualDeskewEntry {
  uint32 m_nLane16;
  uint32 m_nLane15;
  uint32 m_nLane14;
  uint32 m_nLane13;
  uint32 m_nLane12;
  uint32 m_nLane11;
  uint32 m_nLane10;
  uint32 m_nLane9;
  uint32 m_nLane8;
  uint32 m_nLane7;
  uint32 m_nLane6;
  uint32 m_nLane5;
  uint32 m_nLane4;
  uint32 m_nLane3;
  uint32 m_nLane2;
  uint32 m_nLane1;
  uint32 m_nLane0;
} sbZfKaSrManualDeskewEntry_t;

uint32
sbZfKaSrManualDeskewEntry_Pack(sbZfKaSrManualDeskewEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex);
void
sbZfKaSrManualDeskewEntry_Unpack(sbZfKaSrManualDeskewEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex);
void
sbZfKaSrManualDeskewEntry_InitInstance(sbZfKaSrManualDeskewEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[10] = ((pToData)[10] & ~ 0x07) | (((nFromData) >> 2) & 0x07); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x1f << 1)) | (((nFromData) & 0x1f) << 1); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[11] = ((pToData)[11] & ~ 0x01) | (((nFromData) >> 4) & 0x01); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 1) & 0x0f); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x1f << 2)) | (((nFromData) & 0x1f) << 2); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[5] = ((pToData)[5] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[7] = ((pToData)[7] & ~ 0x07) | (((nFromData) >> 2) & 0x07); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x1f << 1)) | (((nFromData) & 0x1f) << 1); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 4) & 0x01); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 1) & 0x0f); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x1f << 2)) | (((nFromData) & 0x1f) << 2); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#else
#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[9] = ((pToData)[9] & ~ 0x07) | (((nFromData) >> 2) & 0x07); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x1f << 1)) | (((nFromData) & 0x1f) << 1); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[8] = ((pToData)[8] & ~ 0x01) | (((nFromData) >> 4) & 0x01); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 1) & 0x0f); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x1f << 2)) | (((nFromData) & 0x1f) << 2); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[6] = ((pToData)[6] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[4] = ((pToData)[4] & ~ 0x07) | (((nFromData) >> 2) & 0x07); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x1f << 1)) | (((nFromData) & 0x1f) << 1); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 4) & 0x01); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 1) & 0x0f); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x1f << 2)) | (((nFromData) & 0x1f) << 2); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[10] = ((pToData)[10] & ~ 0x07) | (((nFromData) >> 2) & 0x07); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x1f << 1)) | (((nFromData) & 0x1f) << 1); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[11] = ((pToData)[11] & ~ 0x01) | (((nFromData) >> 4) & 0x01); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 1) & 0x0f); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x1f << 2)) | (((nFromData) & 0x1f) << 2); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[5] = ((pToData)[5] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[7] = ((pToData)[7] & ~ 0x07) | (((nFromData) >> 2) & 0x07); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x1f << 1)) | (((nFromData) & 0x1f) << 1); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[0] = ((pToData)[0] & ~ 0x01) | (((nFromData) >> 4) & 0x01); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 1) & 0x0f); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x1f << 2)) | (((nFromData) & 0x1f) << 2); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#else
#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE16(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[9] = ((pToData)[9] & ~ 0x07) | (((nFromData) >> 2) & 0x07); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x1f << 1)) | (((nFromData) & 0x1f) << 1); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[8] = ((pToData)[8] & ~ 0x01) | (((nFromData) >> 4) & 0x01); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 1) & 0x0f); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x1f << 2)) | (((nFromData) & 0x1f) << 2); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[6] = ((pToData)[6] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[4] = ((pToData)[4] & ~ 0x07) | (((nFromData) >> 2) & 0x07); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x1f << 1)) | (((nFromData) & 0x1f) << 1); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[3] = ((pToData)[3] & ~ 0x01) | (((nFromData) >> 4) & 0x01); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 1) & 0x0f); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x1f << 2)) | (((nFromData) & 0x1f) << 2); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 3) & 0x03); \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_SET_LANE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x1f) | ((nFromData) & 0x1f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9]) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[10] & 0x07) << 2; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11] >> 1) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[11] & 0x01) << 4; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x0f) << 1; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 2) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x07) << 2; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 1) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x01) << 4; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 1; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x1f; \
          } while(0)

#else
#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10]) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[9] & 0x07) << 2; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 1) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[8] & 0x01) << 4; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x0f) << 1; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 2) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x07) << 2; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 1) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x01) << 4; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 1; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x1f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9]) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[10] & 0x07) << 2; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11] >> 1) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[11] & 0x01) << 4; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x0f) << 1; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 2) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x07) << 2; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 1) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x01) << 4; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 1; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x1f; \
          } while(0)

#else
#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE16(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10]) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[9] & 0x07) << 2; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 1) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[8] & 0x01) << 4; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x0f) << 1; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 2) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x03; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x07) << 2; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 1) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x01) << 4; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 1; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x1f; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 5) & 0x07; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 3; \
          } while(0)

#define SB_ZF_KASRMANUALDESKEWENTRY_GET_LANE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x1f; \
          } while(0)

#endif
#endif
