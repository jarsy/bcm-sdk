/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600BwWredDropNPart1Entry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_H
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_H

#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_SIZE_IN_BYTES 4
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_SIZE 4
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_M_UDP1_BITS "31:31"
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_M_UECN1_BITS "30:30"
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_M_URESERVED1_BITS "29:26"
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_M_UPBDE1_BITS "25:16"
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_M_UDP0_BITS "15:15"
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_M_UECN0_BITS "14:14"
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_M_URESERVED0_BITS "13:10"
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_M_UPBDE0_BITS "9:0"


typedef struct _sbZfFabBm9600BwWredDropNPart1Entry {
  uint32 m_uDp1;
  uint32 m_uEcn1;
  uint32 m_uReserved1;
  uint32 m_uPbDe1;
  uint32 m_uDp0;
  uint32 m_uEcn0;
  uint32 m_uReserved0;
  uint32 m_uPbDe0;
} sbZfFabBm9600BwWredDropNPart1Entry_t;

uint32
sbZfFabBm9600BwWredDropNPart1Entry_Pack(sbZfFabBm9600BwWredDropNPart1Entry_t *pFrom,
                                        uint8 *pToData,
                                        uint32 nMaxToDataIndex);
void
sbZfFabBm9600BwWredDropNPart1Entry_Unpack(sbZfFabBm9600BwWredDropNPart1Entry_t *pToStruct,
                                          uint8 *pFromData,
                                          uint32 nMaxToDataIndex);
void
sbZfFabBm9600BwWredDropNPart1Entry_InitInstance(sbZfFabBm9600BwWredDropNPart1Entry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_ECN1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_RESERVED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_PBDE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_ECN0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_RESERVED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_PBDE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_ECN1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_RESERVED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_PBDE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_ECN0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_RESERVED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_PBDE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_ECN1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_RESERVED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_PBDE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_ECN0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_RESERVED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_PBDE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_DP1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_ECN1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_RESERVED1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_PBDE1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_DP0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_ECN0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_RESERVED0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_SET_PBDE0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_ECN1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_RESERVED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_PBDE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x03) << 8; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_ECN0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_RESERVED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_PBDE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_ECN1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_RESERVED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_PBDE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x03) << 8; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_ECN0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_RESERVED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_PBDE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_ECN1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_RESERVED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_PBDE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) ((pFromData)[0] & 0x03) << 8; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_ECN0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_RESERVED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_PBDE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_DP1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_ECN1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_RESERVED1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_PBDE1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) ((pFromData)[3] & 0x03) << 8; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_DP0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_ECN0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_RESERVED0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART1ENTRY_GET_PBDE0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#endif
