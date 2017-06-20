/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600InaPortPriEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_INAPORTPRIENTRY_H
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_H

#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_SIZE_IN_BYTES 16
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_SIZE 16
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UPRI_15_BITS "127:124"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UNEXTPRI_15_BITS "123:120"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UPRI_14_BITS "119:116"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UNEXTPRI_14_BITS "115:112"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UPRI_13_BITS "111:108"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UNEXTPRI_13_BITS "107:104"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UPRI_12_BITS "103:100"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UNEXTPRI_12_BITS "99:96"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UPRI_11_BITS "95:92"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UNEXTPRI_11_BITS "91:88"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UPRI_10_BITS "87:84"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UNEXTPRI_10_BITS "83:80"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UPRI_9_BITS "79:76"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UNEXTPRI_9_BITS "75:72"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UPRI_8_BITS "71:68"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UNEXTPRI_8_BITS "67:64"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UPRI_7_BITS "63:60"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UNEXTPRI_7_BITS "59:56"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UPRI_6_BITS "55:52"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UNEXTPRI_6_BITS "51:48"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UPRI_5_BITS "47:44"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UNEXTPRI_5_BITS "43:40"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UPRI_4_BITS "39:36"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UNEXTPRI_4_BITS "35:32"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UPRI_3_BITS "31:28"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UNEXTPRI_3_BITS "27:24"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UPRI_2_BITS "23:20"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UNEXTPRI_2_BITS "19:16"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UPRI_1_BITS "15:12"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UNEXTPRI_1_BITS "11:8"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UPRI_0_BITS "7:4"
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_M_UNEXTPRI_0_BITS "3:0"


typedef struct _sbZfFabBm9600InaPortPriEntry {
  uint32 m_uPri_15;
  uint32 m_uNextpri_15;
  uint32 m_uPri_14;
  uint32 m_uNextpri_14;
  uint32 m_uPri_13;
  uint32 m_uNextpri_13;
  uint32 m_uPri_12;
  uint32 m_uNextpri_12;
  uint32 m_uPri_11;
  uint32 m_uNextpri_11;
  uint32 m_uPri_10;
  uint32 m_uNextpri_10;
  uint32 m_uPri_9;
  uint32 m_uNextpri_9;
  uint32 m_uPri_8;
  uint32 m_uNextpri_8;
  uint32 m_uPri_7;
  uint32 m_uNextpri_7;
  uint32 m_uPri_6;
  uint32 m_uNextpri_6;
  uint32 m_uPri_5;
  uint32 m_uNextpri_5;
  uint32 m_uPri_4;
  uint32 m_uNextpri_4;
  uint32 m_uPri_3;
  uint32 m_uNextpri_3;
  uint32 m_uPri_2;
  uint32 m_uNextpri_2;
  uint32 m_uPri_1;
  uint32 m_uNextpri_1;
  uint32 m_uPri_0;
  uint32 m_uNextpri_0;
} sbZfFabBm9600InaPortPriEntry_t;

uint32
sbZfFabBm9600InaPortPriEntry_Pack(sbZfFabBm9600InaPortPriEntry_t *pFrom,
                                  uint8 *pToData,
                                  uint32 nMaxToDataIndex);
void
sbZfFabBm9600InaPortPriEntry_Unpack(sbZfFabBm9600InaPortPriEntry_t *pToStruct,
                                    uint8 *pFromData,
                                    uint32 nMaxToDataIndex);
void
sbZfFabBm9600InaPortPriEntry_InitInstance(sbZfFabBm9600InaPortPriEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#else
#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_15(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((pToData)[15] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_14(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[14] = ((pToData)[14] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_13(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[13] = ((pToData)[13] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_12(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((pToData)[12] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_11(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((pToData)[11] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_10(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[10] = ((pToData)[10] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_9(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[9] = ((pToData)[9] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((pToData)[8] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_PRI_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_SET_NEXTPRI_0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[13] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[13]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[14] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[14]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[14] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[14]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[13] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[13]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[13] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[13]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[14] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[14]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#else
#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_15(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[15]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[14] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_14(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[14]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[13] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_13(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[13]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_12(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[12]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_11(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[11]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_10(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[10]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_9(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[9]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[8]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_PRI_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600INAPORTPRIENTRY_GET_NEXTPRI_0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x0f; \
          } while(0)

#endif
#endif
