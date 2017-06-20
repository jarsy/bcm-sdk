/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600NmSysportArrayEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_H
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_H

#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_SIZE_IN_BYTES 24
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_SIZE 24
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_M_USPA_15_BITS "191:180"
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_M_USPA_14_BITS "179:168"
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_M_USPA_13_BITS "167:156"
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_M_USPA_12_BITS "155:144"
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_M_USPA_11_BITS "143:132"
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_M_USPA_10_BITS "131:120"
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_M_USPA_9_BITS "119:108"
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_M_USPA_8_BITS "107:96"
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_M_USPA_7_BITS "95:84"
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_M_USPA_6_BITS "83:72"
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_M_USPA_5_BITS "71:60"
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_M_USPA_4_BITS "59:48"
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_M_USPA_3_BITS "47:36"
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_M_USPA_2_BITS "35:24"
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_M_USPA_1_BITS "23:12"
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_M_USPA_0_BITS "11:0"


typedef struct _sbZfFabBm9600NmSysportArrayEntry {
  uint32 m_uSpa_15;
  uint32 m_uSpa_14;
  uint32 m_uSpa_13;
  uint32 m_uSpa_12;
  uint32 m_uSpa_11;
  uint32 m_uSpa_10;
  uint32 m_uSpa_9;
  uint32 m_uSpa_8;
  uint32 m_uSpa_7;
  uint32 m_uSpa_6;
  uint32 m_uSpa_5;
  uint32 m_uSpa_4;
  uint32 m_uSpa_3;
  uint32 m_uSpa_2;
  uint32 m_uSpa_1;
  uint32 m_uSpa_0;
} sbZfFabBm9600NmSysportArrayEntry_t;

uint32
sbZfFabBm9600NmSysportArrayEntry_Pack(sbZfFabBm9600NmSysportArrayEntry_t *pFrom,
                                      uint8 *pToData,
                                      uint32 nMaxToDataIndex);
void
sbZfFabBm9600NmSysportArrayEntry_Unpack(sbZfFabBm9600NmSysportArrayEntry_t *pToStruct,
                                        uint8 *pFromData,
                                        uint32 nMaxToDataIndex);
void
sbZfFabBm9600NmSysportArrayEntry_InitInstance(sbZfFabBm9600NmSysportArrayEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((pToData)[21] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[22] = ((nFromData)) & 0xFF; \
           (pToData)[21] = ((pToData)[21] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((nFromData)) & 0xFF; \
           (pToData)[16] = ((pToData)[16] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((pToData)[19] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[19] = ((pToData)[19] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[14] = ((pToData)[14] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#else
#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[22] = ((pToData)[22] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((nFromData)) & 0xFF; \
           (pToData)[22] = ((pToData)[22] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((pToData)[19] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((nFromData)) & 0xFF; \
           (pToData)[19] = ((pToData)[19] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[16] = ((pToData)[16] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((pToData)[21] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[22] = ((nFromData)) & 0xFF; \
           (pToData)[21] = ((pToData)[21] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[17] = ((nFromData)) & 0xFF; \
           (pToData)[16] = ((pToData)[16] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((pToData)[19] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[19] = ((pToData)[19] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[14] = ((pToData)[14] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#else
#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[22] = ((pToData)[22] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((nFromData)) & 0xFF; \
           (pToData)[22] = ((pToData)[22] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((pToData)[19] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[18] = ((nFromData)) & 0xFF; \
           (pToData)[19] = ((pToData)[19] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((pToData)[16] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[16] = ((pToData)[16] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((nFromData)) & 0xFF; \
           (pToData)[7] = ((pToData)[7] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[4] = ((pToData)[4] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_SET_SPA_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[21] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[20] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[22] ; \
           (nToData) |= (uint32) ((pFromData)[21] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[16] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[23] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[17] ; \
           (nToData) |= (uint32) ((pFromData)[16] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[19] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[18] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[12] ; \
           (nToData) |= (uint32) ((pFromData)[19] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[14] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[13] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[15] ; \
           (nToData) |= (uint32) ((pFromData)[14] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[8] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[10] ; \
           (nToData) |= (uint32) ((pFromData)[9] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[11] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[5] ; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[6] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[1] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[22] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[23] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[21] ; \
           (nToData) |= (uint32) ((pFromData)[22] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[19] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[20] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[18] ; \
           (nToData) |= (uint32) ((pFromData)[19] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[16] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[17] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[15] ; \
           (nToData) |= (uint32) ((pFromData)[16] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[13] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[14] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[12] ; \
           (nToData) |= (uint32) ((pFromData)[13] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[11] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[9] ; \
           (nToData) |= (uint32) ((pFromData)[10] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[8] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[6] ; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[5] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[2] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[21] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[20] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[22] ; \
           (nToData) |= (uint32) ((pFromData)[21] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[16] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[23] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[17] ; \
           (nToData) |= (uint32) ((pFromData)[16] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[19] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[18] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[12] ; \
           (nToData) |= (uint32) ((pFromData)[19] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[14] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[13] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[15] ; \
           (nToData) |= (uint32) ((pFromData)[14] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[8] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[10] ; \
           (nToData) |= (uint32) ((pFromData)[9] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[11] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[5] ; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[6] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[1] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[22] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[23] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[21] ; \
           (nToData) |= (uint32) ((pFromData)[22] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[19] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[20] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[18] ; \
           (nToData) |= (uint32) ((pFromData)[19] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[16] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[17] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[15] ; \
           (nToData) |= (uint32) ((pFromData)[16] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[13] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[14] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[12] ; \
           (nToData) |= (uint32) ((pFromData)[13] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[11] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[9] ; \
           (nToData) |= (uint32) ((pFromData)[10] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[8] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[6] ; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[5] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x0f) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[2] << 4; \
          } while(0)

#define SB_ZF_FABBM9600NMSYSPORTARRAYENTRY_GET_SPA_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 8; \
          } while(0)

#endif
#endif
