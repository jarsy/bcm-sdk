/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600BwWredDropNPart2Entry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_BWWREDDROPNPART2ENTRY_H
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART2ENTRY_H

#define SB_ZF_FAB_BM9600_BWWREDDROPNPART2ENTRY_SIZE_IN_BYTES 2
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART2ENTRY_SIZE 2
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART2ENTRY_M_UDP2_BITS "15:15"
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART2ENTRY_M_UECN2_BITS "14:14"
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART2ENTRY_M_URESERVED2_BITS "13:10"
#define SB_ZF_FAB_BM9600_BWWREDDROPNPART2ENTRY_M_UPBDE2_BITS "9:0"


typedef struct _sbZfFabBm9600BwWredDropNPart2Entry {
  uint32 m_uDp2;
  uint32 m_uEcn2;
  uint32 m_uReserved2;
  uint32 m_uPbDe2;
} sbZfFabBm9600BwWredDropNPart2Entry_t;

uint32
sbZfFabBm9600BwWredDropNPart2Entry_Pack(sbZfFabBm9600BwWredDropNPart2Entry_t *pFrom,
                                        uint8 *pToData,
                                        uint32 nMaxToDataIndex);
void
sbZfFabBm9600BwWredDropNPart2Entry_Unpack(sbZfFabBm9600BwWredDropNPart2Entry_t *pToStruct,
                                          uint8 *pFromData,
                                          uint32 nMaxToDataIndex);
void
sbZfFabBm9600BwWredDropNPart2Entry_InitInstance(sbZfFabBm9600BwWredDropNPart2Entry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_SET_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_SET_ECN2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_SET_RESERVED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_SET_PBDE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_SET_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_SET_ECN2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_SET_RESERVED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_SET_PBDE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_SET_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_SET_ECN2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_SET_RESERVED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_SET_PBDE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#else
#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_SET_DP2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_SET_ECN2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_SET_RESERVED2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 2)) | (((nFromData) & 0x0f) << 2); \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_SET_PBDE2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_GET_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_GET_ECN2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_GET_RESERVED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_GET_PBDE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_GET_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_GET_ECN2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_GET_RESERVED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_GET_PBDE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_GET_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_GET_ECN2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_GET_RESERVED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_GET_PBDE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x03) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_GET_DP2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_GET_ECN2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_GET_RESERVED2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 2) & 0x0f; \
          } while(0)

#define SB_ZF_FABBM9600BWWREDDROPNPART2ENTRY_GET_PBDE2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x03) << 8; \
          } while(0)

#endif
#endif
