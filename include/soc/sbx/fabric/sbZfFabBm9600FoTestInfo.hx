/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600FoTestInfo.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_FOTESTINFO_H
#define SB_ZF_FAB_BM9600_FOTESTINFO_H

#define SB_ZF_FAB_BM9600_FOTESTINFO_SIZE_IN_BYTES 22
#define SB_ZF_FAB_BM9600_FOTESTINFO_SIZE 22
#define SB_ZF_FAB_BM9600_FOTESTINFO_M_BISACTIVE_BITS "170:170"
#define SB_ZF_FAB_BM9600_FOTESTINFO_M_UUDUTTIMESLOTSIZE_BITS "169:106"
#define SB_ZF_FAB_BM9600_FOTESTINFO_M_UUSIMTIMETHATQEFAILOVEROCCURRED_BITS "105:42"
#define SB_ZF_FAB_BM9600_FOTESTINFO_M_UDUTBASEADDRESS_BITS "41:10"
#define SB_ZF_FAB_BM9600_FOTESTINFO_M_UNUMQESTHATFAILEDOVER_BITS "9:3"
#define SB_ZF_FAB_BM9600_FOTESTINFO_M_UPREVIOUSACTIVEBM_BITS "2:2"
#define SB_ZF_FAB_BM9600_FOTESTINFO_M_BFAILOVERPREVIOUSLYASSERTED_BITS "1:1"
#define SB_ZF_FAB_BM9600_FOTESTINFO_M_BEXPECTFAILOVER_BITS "0:0"


typedef struct _sbZfFabBm9600FoTestInfo {
  uint8 m_bIsActive;
  uint64 m_uuDUTTimeslotsize;
  uint64 m_uuSimTimeThatQEFailoverOccurred;
  uint32 m_uDUTBaseAddress;
  uint32 m_uNumQEsThatFailedOver;
  uint32 m_uPreviousActiveBm;
  uint8 m_bFailoverPreviouslyAsserted;
  uint8 m_bExpectFailover;
} sbZfFabBm9600FoTestInfo_t;

uint32
sbZfFabBm9600FoTestInfo_Pack(sbZfFabBm9600FoTestInfo_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex);
void
sbZfFabBm9600FoTestInfo_Unpack(sbZfFabBm9600FoTestInfo_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex);
void
sbZfFabBm9600FoTestInfo_InitInstance(sbZfFabBm9600FoTestInfo_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600FOTESTINFO_SET_ISACTIVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[22] = ((pToData)[22] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_DUTSTIMESLOTSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_QEFOSIMTIME(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_DUTBASEADDRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_NUMFAILEDQE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 5) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_PREVIOUSACTIVEBM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_FOPREVIOUSLYASSERTED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_EXPECTFO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_FABBM9600FOTESTINFO_SET_ISACTIVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((pToData)[21] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_DUTSTIMESLOTSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_QEFOSIMTIME(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_DUTBASEADDRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_NUMFAILEDQE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 5) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_PREVIOUSACTIVEBM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_FOPREVIOUSLYASSERTED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_EXPECTFO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600FOTESTINFO_SET_ISACTIVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[22] = ((pToData)[22] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_DUTSTIMESLOTSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_QEFOSIMTIME(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_DUTBASEADDRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_NUMFAILEDQE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 5) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_PREVIOUSACTIVEBM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_FOPREVIOUSLYASSERTED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_EXPECTFO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_FABBM9600FOTESTINFO_SET_ISACTIVE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((pToData)[21] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_DUTSTIMESLOTSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_QEFOSIMTIME(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 30) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 38) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 46) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 54) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~ 0x03) | (((nFromData) >> 62) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_DUTBASEADDRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x3f << 2)) | (((nFromData) & 0x3f) << 2); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 6) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 14) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 22) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~ 0x03) | (((nFromData) >> 30) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_NUMFAILEDQE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x1f << 3)) | (((nFromData) & 0x1f) << 3); \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 5) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_PREVIOUSACTIVEBM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_FOPREVIOUSLYASSERTED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_SET_EXPECTFO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600FOTESTINFO_GET_ISACTIVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[22] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_DUTSTIMESLOTSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[14]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[23]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_QEFOSIMTIME(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[6]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[4]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[11]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_DUTBASEADDRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[1] << 6; \
           (nToData) |= (uint32) (pFromData)[0] << 14; \
           (nToData) |= (uint32) (pFromData)[7] << 22; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x03) << 30; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_NUMFAILEDQE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 3) & 0x1f; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 5; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_PREVIOUSACTIVEBM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_FOPREVIOUSLYASSERTED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_EXPECTFO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_FABBM9600FOTESTINFO_GET_ISACTIVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[21] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_DUTSTIMESLOTSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[13]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[20]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_QEFOSIMTIME(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[5]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[7]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[11]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_DUTBASEADDRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[2] << 6; \
           (nToData) |= (uint32) (pFromData)[3] << 14; \
           (nToData) |= (uint32) (pFromData)[4] << 22; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x03) << 30; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_NUMFAILEDQE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 3) & 0x1f; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 5; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_PREVIOUSACTIVEBM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_FOPREVIOUSLYASSERTED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_EXPECTFO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600FOTESTINFO_GET_ISACTIVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[22] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_DUTSTIMESLOTSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[14]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[23]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_QEFOSIMTIME(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[6]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[4]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[11]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_DUTBASEADDRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[1] << 6; \
           (nToData) |= (uint32) (pFromData)[0] << 14; \
           (nToData) |= (uint32) (pFromData)[7] << 22; \
           (nToData) |= (uint32) ((pFromData)[6] & 0x03) << 30; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_NUMFAILEDQE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 3) & 0x1f; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 5; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_PREVIOUSACTIVEBM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_FOPREVIOUSLYASSERTED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_EXPECTFO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_FABBM9600FOTESTINFO_GET_ISACTIVE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[21] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_DUTSTIMESLOTSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[13]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[20]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_QEFOSIMTIME(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[5]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[7]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[11]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_DUTBASEADDRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x3f; \
           (nToData) |= (uint32) (pFromData)[2] << 6; \
           (nToData) |= (uint32) (pFromData)[3] << 14; \
           (nToData) |= (uint32) (pFromData)[4] << 22; \
           (nToData) |= (uint32) ((pFromData)[5] & 0x03) << 30; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_NUMFAILEDQE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 3) & 0x1f; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 5; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_PREVIOUSACTIVEBM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_FOPREVIOUSLYASSERTED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600FOTESTINFO_GET_EXPECTFO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#endif
