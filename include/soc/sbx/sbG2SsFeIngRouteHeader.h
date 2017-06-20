/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbG2SsFeIngRouteHeader.h,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef SB_ZF_G2_SS_FEINGROUTEHEADER_H
#define SB_ZF_G2_SS_FEINGROUTEHEADER_H

#define SB_ZF_G2_SS_FEINGROUTEHEADER_SIZE_IN_BYTES 12
#define SB_ZF_G2_SS_FEINGROUTEHEADER_SIZE 12
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULTTL_BITS "7:0"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULS_BITS "8:8"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULRDP_BITS "10:9"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULRCOS_BITS "13:11"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULRSVD1_BITS "23:14"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULLBID_BITS "26:24"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULFCOS2_BITS "29:27"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULFDP_BITS "31:30"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULRSVD_BITS "33:32"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULSID_BITS "47:34"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULOUTUNION_BITS "63:48"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULQID_BITS "79:64"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULLENADJ_BITS "83:80"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULMC_BITS "84:84"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULTEST_BITS "85:85"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULECN_BITS "86:86"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULECT_BITS "87:87"
#define SB_ZF_G2_SS_FEINGROUTEHEADER_ULKSOP_BITS "95:88"


typedef struct _sbZfG2SsFeIngRouteHeader {
  uint32 ulTtl;
  uint32 ulS;
  uint32 ulRDp;
  uint32 ulRCos;
  uint32 ulRsvd1;
  uint32 ulLBId;
  uint32 ulFCos2;
  uint32 ulFDp;
  uint32 ulRsvd;
  uint32 ulSid;
  uint32 ulOutUnion;
  uint32 ulQid;
  uint32 ulLenAdj;
  uint32 ulMc;
  uint32 ulTest;
  uint32 ulEcn;
  uint32 ulEct;
  uint32 ulKsop;
} sbZfG2SsFeIngRouteHeader_t;

uint32
sbZfG2SsFeIngRouteHeader_Pack(sbZfG2SsFeIngRouteHeader_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex);
void
sbZfG2SsFeIngRouteHeader_Unpack(sbZfG2SsFeIngRouteHeader_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex);
void
sbZfG2SsFeIngRouteHeader_InitInstance(sbZfG2SsFeIngRouteHeader_t *pFrame);

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_TTL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_S(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_RDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_RCOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_RSVD1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 2) & 0xFF); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_LBID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~0x07) | ((nFromData) & 0x07); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_FCOS2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x07 << 3)) | (((nFromData) & 0x07) << 3); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_FDP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x03 << 6)) | (((nFromData) & 0x03) << 6); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_RSVD(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x03) | ((nFromData) & 0x03); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_SID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_OUTUNION(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_QID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_LENADJ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_MC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_TEST(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 5)) | (((nFromData) & 0x01) << 5); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_ECN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_ECT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_SET_KSOP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_TTL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[11] ; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_S(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10]) & 0x01; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_RDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_RCOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_RSVD1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 6) & 0x03; \
           (nToData) |= (uint32) (pFromData)[9] << 2; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_LBID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8]) & 0x07; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_FCOS2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 3) & 0x07; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_FDP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 6) & 0x03; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_RSVD(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7]) & 0x03; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_SID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[6] << 6; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_OUTUNION(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[5] ; \
           (nToData) |= (uint32) (pFromData)[4] << 8; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_QID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_LENADJ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_MC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_TEST(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x01; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_ECN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_ECT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_G2SSFEINGROUTEHEADER_GET_KSOP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
