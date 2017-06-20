/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200BwWctEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM3200_WCT_ENTRY_H
#define SB_ZF_FAB_BM3200_WCT_ENTRY_H

#define SB_ZF_FAB_BM3200_WCT_ENTRY_SIZE_IN_BYTES 32
#define SB_ZF_FAB_BM3200_WCT_ENTRY_SIZE 32
#define SB_ZF_FAB_BM3200_WCT_ENTRY_M_NRESERVED1_BITS "255:224"
#define SB_ZF_FAB_BM3200_WCT_ENTRY_M_NRESERVED0_BITS "223:192"
#define SB_ZF_FAB_BM3200_WCT_ENTRY_M_NECN_DP2_BITS "191:176"
#define SB_ZF_FAB_BM3200_WCT_ENTRY_M_NSCALE_DP2_BITS "175:172"
#define SB_ZF_FAB_BM3200_WCT_ENTRY_M_NSLOPE_DP2_BITS "171:160"
#define SB_ZF_FAB_BM3200_WCT_ENTRY_M_NMIN_DP2_BITS "159:144"
#define SB_ZF_FAB_BM3200_WCT_ENTRY_M_NMAX_DP2_BITS "143:128"
#define SB_ZF_FAB_BM3200_WCT_ENTRY_M_NECN_DP1_BITS "127:112"
#define SB_ZF_FAB_BM3200_WCT_ENTRY_M_NSCALE_DP1_BITS "111:108"
#define SB_ZF_FAB_BM3200_WCT_ENTRY_M_NSLOPE_DP1_BITS "107:96"
#define SB_ZF_FAB_BM3200_WCT_ENTRY_M_NMIN_DP1_BITS "95:80"
#define SB_ZF_FAB_BM3200_WCT_ENTRY_M_NMAX_DP1_BITS "79:64"
#define SB_ZF_FAB_BM3200_WCT_ENTRY_M_NECN_DP0_BITS "63:48"
#define SB_ZF_FAB_BM3200_WCT_ENTRY_M_NSCALE_DP0_BITS "47:44"
#define SB_ZF_FAB_BM3200_WCT_ENTRY_M_NSLOPE_DP0_BITS "43:32"
#define SB_ZF_FAB_BM3200_WCT_ENTRY_M_NMIN_DP0_BITS "31:16"
#define SB_ZF_FAB_BM3200_WCT_ENTRY_M_NMAX_DP0_BITS "15:0"


typedef struct _sbZfFabBm3200BwWctEntry {
  uint32 m_nReserved1;
  uint32 m_nReserved0;
  uint32 m_nEcn_dp2;
  uint32 m_nScale_dp2;
  uint32 m_nSlope_dp2;
  uint32 m_nMin_dp2;
  uint32 m_nMax_dp2;
  uint32 m_nEcn_dp1;
  uint32 m_nScale_dp1;
  uint32 m_nSlope_dp1;
  uint32 m_nMin_dp1;
  uint32 m_nMax_dp1;
  uint32 m_nEcn_dp0;
  uint32 m_nScale_dp0;
  uint32 m_nSlope_dp0;
  uint32 m_nMin_dp0;
  uint32 m_nMax_dp0;
} sbZfFabBm3200BwWctEntry_t;

uint32
sbZfFabBm3200BwWctEntry_Pack(sbZfFabBm3200BwWctEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwWctEntry_Unpack(sbZfFabBm3200BwWctEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwWctEntry_InitInstance(sbZfFabBm3200BwWctEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWWCTENTRY_SET_RESERVED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[31] = ((nFromData)) & 0xFF; \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[28] = ((pToData)[28] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_RESERVED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[27] = ((nFromData)) & 0xFF; \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[24] = ((pToData)[24] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_ECN_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((nFromData)) & 0xFF; \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SCALE_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[22] = ((pToData)[22] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SLOPE_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[23] = ((nFromData)) & 0xFF; \
           (pToData)[22] = ((pToData)[22] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MIN_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((nFromData)) & 0xFF; \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MAX_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((nFromData)) & 0xFF; \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_ECN_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((nFromData)) & 0xFF; \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SCALE_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SLOPE_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[14] = ((pToData)[14] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MIN_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((nFromData)) & 0xFF; \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MAX_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_ECN_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SCALE_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SLOPE_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MIN_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MAX_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWWCTENTRY_SET_RESERVED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[28] = ((nFromData)) & 0xFF; \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_RESERVED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[24] = ((nFromData)) & 0xFF; \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_ECN_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[22] = ((nFromData)) & 0xFF; \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SCALE_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((pToData)[21] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SLOPE_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[20] = ((nFromData)) & 0xFF; \
           (pToData)[21] = ((pToData)[21] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MIN_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((nFromData)) & 0xFF; \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MAX_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_ECN_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((nFromData)) & 0xFF; \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SCALE_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SLOPE_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MIN_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((nFromData)) & 0xFF; \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MAX_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_ECN_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SCALE_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SLOPE_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MIN_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MAX_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWWCTENTRY_SET_RESERVED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[31] = ((nFromData)) & 0xFF; \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[28] = ((pToData)[28] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_RESERVED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[27] = ((nFromData)) & 0xFF; \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[24] = ((pToData)[24] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_ECN_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((nFromData)) & 0xFF; \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SCALE_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[22] = ((pToData)[22] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SLOPE_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[23] = ((nFromData)) & 0xFF; \
           (pToData)[22] = ((pToData)[22] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MIN_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((nFromData)) & 0xFF; \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MAX_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((nFromData)) & 0xFF; \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_ECN_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((nFromData)) & 0xFF; \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SCALE_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SLOPE_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[14] = ((pToData)[14] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MIN_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((nFromData)) & 0xFF; \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MAX_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_ECN_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SCALE_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SLOPE_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MIN_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MAX_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWWCTENTRY_SET_RESERVED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[28] = ((nFromData)) & 0xFF; \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_RESERVED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[24] = ((nFromData)) & 0xFF; \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_ECN_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[22] = ((nFromData)) & 0xFF; \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SCALE_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((pToData)[21] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SLOPE_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[20] = ((nFromData)) & 0xFF; \
           (pToData)[21] = ((pToData)[21] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MIN_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((nFromData)) & 0xFF; \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MAX_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_ECN_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((nFromData)) & 0xFF; \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SCALE_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SLOPE_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MIN_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((nFromData)) & 0xFF; \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MAX_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_ECN_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SCALE_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_SLOPE_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MIN_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_SET_MAX_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWWCTENTRY_GET_RESERVED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[31] ; \
           (nToData) |= (uint32) (pFromData)[30] << 8; \
           (nToData) |= (uint32) (pFromData)[29] << 16; \
           (nToData) |= (uint32) (pFromData)[28] << 24; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_RESERVED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[27] ; \
           (nToData) |= (uint32) (pFromData)[26] << 8; \
           (nToData) |= (uint32) (pFromData)[25] << 16; \
           (nToData) |= (uint32) (pFromData)[24] << 24; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_ECN_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[21] ; \
           (nToData) |= (uint32) (pFromData)[20] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SCALE_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[22] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SLOPE_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[23] ; \
           (nToData) |= (uint32) ((pFromData)[22] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MIN_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[17] ; \
           (nToData) |= (uint32) (pFromData)[16] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MAX_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[19] ; \
           (nToData) |= (uint32) (pFromData)[18] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_ECN_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[13] ; \
           (nToData) |= (uint32) (pFromData)[12] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SCALE_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[14] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SLOPE_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[15] ; \
           (nToData) |= (uint32) ((pFromData)[14] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MIN_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[9] ; \
           (nToData) |= (uint32) (pFromData)[8] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MAX_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[11] ; \
           (nToData) |= (uint32) (pFromData)[10] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_ECN_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[5] ; \
           (nToData) |= (uint32) (pFromData)[4] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SCALE_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SLOPE_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MIN_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[0] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MAX_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWWCTENTRY_GET_RESERVED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[28] ; \
           (nToData) |= (uint32) (pFromData)[29] << 8; \
           (nToData) |= (uint32) (pFromData)[30] << 16; \
           (nToData) |= (uint32) (pFromData)[31] << 24; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_RESERVED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[24] ; \
           (nToData) |= (uint32) (pFromData)[25] << 8; \
           (nToData) |= (uint32) (pFromData)[26] << 16; \
           (nToData) |= (uint32) (pFromData)[27] << 24; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_ECN_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[22] ; \
           (nToData) |= (uint32) (pFromData)[23] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SCALE_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[21] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SLOPE_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[20] ; \
           (nToData) |= (uint32) ((pFromData)[21] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MIN_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[18] ; \
           (nToData) |= (uint32) (pFromData)[19] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MAX_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[16] ; \
           (nToData) |= (uint32) (pFromData)[17] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_ECN_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[14] ; \
           (nToData) |= (uint32) (pFromData)[15] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SCALE_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[13] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SLOPE_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[12] ; \
           (nToData) |= (uint32) ((pFromData)[13] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MIN_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[10] ; \
           (nToData) |= (uint32) (pFromData)[11] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MAX_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[8] ; \
           (nToData) |= (uint32) (pFromData)[9] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_ECN_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[6] ; \
           (nToData) |= (uint32) (pFromData)[7] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SCALE_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SLOPE_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MIN_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[3] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MAX_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWWCTENTRY_GET_RESERVED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[31] ; \
           (nToData) |= (uint32) (pFromData)[30] << 8; \
           (nToData) |= (uint32) (pFromData)[29] << 16; \
           (nToData) |= (uint32) (pFromData)[28] << 24; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_RESERVED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[27] ; \
           (nToData) |= (uint32) (pFromData)[26] << 8; \
           (nToData) |= (uint32) (pFromData)[25] << 16; \
           (nToData) |= (uint32) (pFromData)[24] << 24; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_ECN_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[21] ; \
           (nToData) |= (uint32) (pFromData)[20] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SCALE_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[22] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SLOPE_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[23] ; \
           (nToData) |= (uint32) ((pFromData)[22] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MIN_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[17] ; \
           (nToData) |= (uint32) (pFromData)[16] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MAX_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[19] ; \
           (nToData) |= (uint32) (pFromData)[18] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_ECN_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[13] ; \
           (nToData) |= (uint32) (pFromData)[12] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SCALE_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[14] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SLOPE_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[15] ; \
           (nToData) |= (uint32) ((pFromData)[14] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MIN_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[9] ; \
           (nToData) |= (uint32) (pFromData)[8] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MAX_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[11] ; \
           (nToData) |= (uint32) (pFromData)[10] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_ECN_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[5] ; \
           (nToData) |= (uint32) (pFromData)[4] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SCALE_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SLOPE_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MIN_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[0] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MAX_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWWCTENTRY_GET_RESERVED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[28] ; \
           (nToData) |= (uint32) (pFromData)[29] << 8; \
           (nToData) |= (uint32) (pFromData)[30] << 16; \
           (nToData) |= (uint32) (pFromData)[31] << 24; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_RESERVED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[24] ; \
           (nToData) |= (uint32) (pFromData)[25] << 8; \
           (nToData) |= (uint32) (pFromData)[26] << 16; \
           (nToData) |= (uint32) (pFromData)[27] << 24; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_ECN_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[22] ; \
           (nToData) |= (uint32) (pFromData)[23] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SCALE_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[21] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SLOPE_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[20] ; \
           (nToData) |= (uint32) ((pFromData)[21] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MIN_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[18] ; \
           (nToData) |= (uint32) (pFromData)[19] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MAX_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[16] ; \
           (nToData) |= (uint32) (pFromData)[17] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_ECN_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[14] ; \
           (nToData) |= (uint32) (pFromData)[15] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SCALE_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[13] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SLOPE_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[12] ; \
           (nToData) |= (uint32) ((pFromData)[13] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MIN_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[10] ; \
           (nToData) |= (uint32) (pFromData)[11] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MAX_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[8] ; \
           (nToData) |= (uint32) (pFromData)[9] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_ECN_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[6] ; \
           (nToData) |= (uint32) (pFromData)[7] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SCALE_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_SLOPE_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MIN_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[3] << 8; \
          } while(0)

#define SB_ZF_FABBM3200BWWCTENTRY_GET_MAX_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200BwWctEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_BM3200_WCT_ENTRY_CONSOLE_H
#define SB_ZF_FAB_BM3200_WCT_ENTRY_CONSOLE_H



void
sbZfFabBm3200BwWctEntry_Print(sbZfFabBm3200BwWctEntry_t *pFromStruct);
int
sbZfFabBm3200BwWctEntry_SPrint(sbZfFabBm3200BwWctEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm3200BwWctEntry_Validate(sbZfFabBm3200BwWctEntry_t *pZf);
int
sbZfFabBm3200BwWctEntry_SetField(sbZfFabBm3200BwWctEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_WCT_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

