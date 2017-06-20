/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600LinkFailureInfo.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_H
#define SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_H

#define SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_SIZE_IN_BYTES 26
#define SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_SIZE 26
#define SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_M_BQELINKSTATE_BITS "207:203"
#define SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_M_BEXPECTINGLINKERROR_BITS "202:202"
#define SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_M_UDIGESTSCANSTARTTIME_BITS "201:138"
#define SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_M_UDIGESTSCANENDTIME_BITS "137:74"
#define SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_M_UDUTBASEADDRESS_BITS "73:42"
#define SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_M_UACTIVEBMEFOMODE_BITS "41:40"
#define SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_M_UACTIVEBMESCILINK_BITS "39:39"
#define SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_M_UEXPECTEDGLOBALLINKSTATE_BITS "38:7"
#define SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_M_UINANUMBER_BITS "6:0"


typedef struct _sbZfFabBm9600LinkFailureInfo {
  uint32 m_bQeLinkState;
  uint8 m_bExpectingLinkError;
  uint64 m_uDigestScanStartTime;
  uint64 m_uDigestScanEndTime;
  uint32 m_uDUTBaseAddress;
  uint32 m_uActiveBmeFoMode;
  uint32 m_uActiveBmeSciLink;
  uint32 m_uExpectedGlobalLinkState;
  uint32 m_uInaNumber;
} sbZfFabBm9600LinkFailureInfo_t;

uint32
sbZfFabBm9600LinkFailureInfo_Pack(sbZfFabBm9600LinkFailureInfo_t *pFrom,
                                  uint8 *pToData,
                                  uint32 nMaxToDataIndex);
void
sbZfFabBm9600LinkFailureInfo_Unpack(sbZfFabBm9600LinkFailureInfo_t *pToStruct,
                                    uint8 *pFromData,
                                    uint32 nMaxToDataIndex);
void
sbZfFabBm9600LinkFailureInfo_InitInstance(sbZfFabBm9600LinkFailureInfo_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_QELINKSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[26] = ((pToData)[26] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_EXPECTINGLINKERROR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[26] = ((pToData)[26] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_DIGESTSCANSTART(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((pToData)[18] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_DIGESTSCANEND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_DUTBASEADDRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_ACTIVEBMEMODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_ACTIVEBMESCILINK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_EXPECTEDLINKSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~ 0x7f) | (((nFromData) >> 25) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_INANUMBER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#else
#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_QELINKSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[25] = ((pToData)[25] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_EXPECTINGLINKERROR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[25] = ((pToData)[25] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_DIGESTSCANSTART(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[24] = ((pToData)[24] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[25] = ((pToData)[25] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_DIGESTSCANEND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_DUTBASEADDRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_ACTIVEBMEMODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_ACTIVEBMESCILINK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_EXPECTEDLINKSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x7f) | (((nFromData) >> 25) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_INANUMBER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_QELINKSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[26] = ((pToData)[26] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_EXPECTINGLINKERROR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[26] = ((pToData)[26] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_DIGESTSCANSTART(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((pToData)[18] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_DIGESTSCANEND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_DUTBASEADDRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_ACTIVEBMEMODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_ACTIVEBMESCILINK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_EXPECTEDLINKSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~ 0x7f) | (((nFromData) >> 25) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_INANUMBER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#else
#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_QELINKSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[25] = ((pToData)[25] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_EXPECTINGLINKERROR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[25] = ((pToData)[25] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_DIGESTSCANSTART(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((pToData)[17] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[24] = ((pToData)[24] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[25] = ((pToData)[25] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_DIGESTSCANEND(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_DUTBASEADDRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_ACTIVEBMEMODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_ACTIVEBMESCILINK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_EXPECTEDLINKSTATE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x7f) | (((nFromData) >> 25) & 0x7f); \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_SET_INANUMBER(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_QELINKSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[26] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_EXPECTINGLINKERROR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[26] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_DIGESTSCANSTART(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[18]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[23]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[22]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[21]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[20]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[27]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_DIGESTSCANEND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[10]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_DUTBASEADDRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[5] << 6; \
           (nToData) |= (uint32) (pFromData)[4] << 14; \
           (nToData) |= (uint32) (pFromData)[11] << 22; \
           (nToData) |= (uint32) ((pFromData)[10] & 0x03) << 30; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_ACTIVEBMEMODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x03; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_ACTIVEBMESCILINK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_EXPECTEDLINKSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[2] << 1; \
           (nToData) |= (uint32) (pFromData)[1] << 9; \
           (nToData) |= (uint32) (pFromData)[0] << 17; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x7f) << 25; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_INANUMBER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x7f; \
          } while(0)

#else
#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_QELINKSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[25] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_EXPECTINGLINKERROR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[25] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_DIGESTSCANSTART(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[17]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[20]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[21]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[22]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[23]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[24]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_DIGESTSCANEND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[9]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[11]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_DUTBASEADDRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[6] << 6; \
           (nToData) |= (uint32) (pFromData)[7] << 14; \
           (nToData) |= (uint32) (pFromData)[8] << 22; \
           (nToData) |= (uint32) ((pFromData)[9] & 0x03) << 30; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_ACTIVEBMEMODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x03; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_ACTIVEBMESCILINK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_EXPECTEDLINKSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[1] << 1; \
           (nToData) |= (uint32) (pFromData)[2] << 9; \
           (nToData) |= (uint32) (pFromData)[3] << 17; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x7f) << 25; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_INANUMBER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x7f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_QELINKSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[26] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_EXPECTINGLINKERROR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[26] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_DIGESTSCANSTART(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[18]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[23]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[22]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[21]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[20]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[27]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_DIGESTSCANEND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[10]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_DUTBASEADDRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[5] << 6; \
           (nToData) |= (uint32) (pFromData)[4] << 14; \
           (nToData) |= (uint32) (pFromData)[11] << 22; \
           (nToData) |= (uint32) ((pFromData)[10] & 0x03) << 30; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_ACTIVEBMEMODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x03; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_ACTIVEBMESCILINK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_EXPECTEDLINKSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[2] << 1; \
           (nToData) |= (uint32) (pFromData)[1] << 9; \
           (nToData) |= (uint32) (pFromData)[0] << 17; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x7f) << 25; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_INANUMBER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x7f; \
          } while(0)

#else
#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_QELINKSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[25] >> 3) & 0x1f; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_EXPECTINGLINKERROR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[25] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_DIGESTSCANSTART(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[17]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[20]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[21]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[22]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[23]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[24]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_DIGESTSCANEND(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[9]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[11]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_DUTBASEADDRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[6] << 6; \
           (nToData) |= (uint32) (pFromData)[7] << 14; \
           (nToData) |= (uint32) (pFromData)[8] << 22; \
           (nToData) |= (uint32) ((pFromData)[9] & 0x03) << 30; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_ACTIVEBMEMODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x03; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_ACTIVEBMESCILINK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_EXPECTEDLINKSTATE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[1] << 1; \
           (nToData) |= (uint32) (pFromData)[2] << 9; \
           (nToData) |= (uint32) (pFromData)[3] << 17; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x7f) << 25; \
          } while(0)

#define SB_ZF_FABBM9600LINKFAILUREINFO_GET_INANUMBER(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x7f; \
          } while(0)

#endif
#endif
