/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEpCrTableEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEPCRTABLEENTRY_H
#define SB_ZF_ZFKAEPCRTABLEENTRY_H

#define SB_ZF_ZFKAEPCRTABLEENTRY_SIZE_IN_BYTES 8
#define SB_ZF_ZFKAEPCRTABLEENTRY_SIZE 8
#define SB_ZF_ZFKAEPCRTABLEENTRY_M_NCLASS15_BITS "63:60"
#define SB_ZF_ZFKAEPCRTABLEENTRY_M_NCLASS14_BITS "59:56"
#define SB_ZF_ZFKAEPCRTABLEENTRY_M_NCLASS13_BITS "55:52"
#define SB_ZF_ZFKAEPCRTABLEENTRY_M_NCLASS12_BITS "51:48"
#define SB_ZF_ZFKAEPCRTABLEENTRY_M_NCLASS11_BITS "47:44"
#define SB_ZF_ZFKAEPCRTABLEENTRY_M_NCLASS10_BITS "43:40"
#define SB_ZF_ZFKAEPCRTABLEENTRY_M_NCLASS9_BITS "39:36"
#define SB_ZF_ZFKAEPCRTABLEENTRY_M_NCLASS8_BITS "35:32"
#define SB_ZF_ZFKAEPCRTABLEENTRY_M_NCLASS7_BITS "31:28"
#define SB_ZF_ZFKAEPCRTABLEENTRY_M_NCLASS6_BITS "27:24"
#define SB_ZF_ZFKAEPCRTABLEENTRY_M_NCLASS5_BITS "23:20"
#define SB_ZF_ZFKAEPCRTABLEENTRY_M_NCLASS4_BITS "19:16"
#define SB_ZF_ZFKAEPCRTABLEENTRY_M_NCLASS3_BITS "15:12"
#define SB_ZF_ZFKAEPCRTABLEENTRY_M_NCLASS2_BITS "11:8"
#define SB_ZF_ZFKAEPCRTABLEENTRY_M_NCLASS1_BITS "7:4"
#define SB_ZF_ZFKAEPCRTABLEENTRY_M_NCLASS0_BITS "3:0"


typedef struct _sbZfKaEpCrTableEntry {
  uint32 m_nClass15;
  uint32 m_nClass14;
  uint32 m_nClass13;
  uint32 m_nClass12;
  uint32 m_nClass11;
  uint32 m_nClass10;
  uint32 m_nClass9;
  uint32 m_nClass8;
  uint32 m_nClass7;
  uint32 m_nClass6;
  uint32 m_nClass5;
  uint32 m_nClass4;
  uint32 m_nClass3;
  uint32 m_nClass2;
  uint32 m_nClass1;
  uint32 m_nClass0;
} sbZfKaEpCrTableEntry_t;

uint32
sbZfKaEpCrTableEntry_Pack(sbZfKaEpCrTableEntry_t *pFrom,
                          uint8 *pToData,
                          uint32 nMaxToDataIndex);
void
sbZfKaEpCrTableEntry_Unpack(sbZfKaEpCrTableEntry_t *pToStruct,
                            uint8 *pFromData,
                            uint32 nMaxToDataIndex);
void
sbZfKaEpCrTableEntry_InitInstance(sbZfKaEpCrTableEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_SET_CLASS0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAEPCRTABLEENTRY_GET_CLASS0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
