/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm9600NmFullStatusEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_H
#define SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_H

#define SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_SIZE_IN_BYTES 34
#define SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_SIZE 34
#define SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_M_FULLSTATUS8_BITS "265:256"
#define SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_M_FULLSTATUS7_BITS "255:224"
#define SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_M_FULLSTATUS6_BITS "223:192"
#define SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_M_FULLSTATUS5_BITS "191:160"
#define SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_M_FULLSTATUS4_BITS "159:128"
#define SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_M_FULLSTATUS3_BITS "127:96"
#define SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_M_FULLSTATUS2_BITS "95:64"
#define SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_M_FULLSTATUS1_BITS "63:32"
#define SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_M_FULLSTATUS0_BITS "31:0"


typedef struct _sbZfFabBm9600NmFullStatusEntry {
  uint32 m_FullStatus8;
  uint32 m_FullStatus7;
  uint32 m_FullStatus6;
  uint32 m_FullStatus5;
  uint32 m_FullStatus4;
  uint32 m_FullStatus3;
  uint32 m_FullStatus2;
  uint32 m_FullStatus1;
  uint32 m_FullStatus0;
} sbZfFabBm9600NmFullStatusEntry_t;

uint32
sbZfFabBm9600NmFullStatusEntry_Pack(sbZfFabBm9600NmFullStatusEntry_t *pFrom,
                                    uint8 *pToData,
                                    uint32 nMaxToDataIndex);
void
sbZfFabBm9600NmFullStatusEntry_Unpack(sbZfFabBm9600NmFullStatusEntry_t *pToStruct,
                                      uint8 *pFromData,
                                      uint32 nMaxToDataIndex);
void
sbZfFabBm9600NmFullStatusEntry_InitInstance(sbZfFabBm9600NmFullStatusEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[35] = ((nFromData)) & 0xFF; \
           (pToData)[34] = ((pToData)[34] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[31] = ((nFromData)) & 0xFF; \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[28] = ((pToData)[28] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[27] = ((nFromData)) & 0xFF; \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[24] = ((pToData)[24] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[23] = ((nFromData)) & 0xFF; \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((nFromData)) & 0xFF; \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[32] = ((nFromData)) & 0xFF; \
           (pToData)[33] = ((pToData)[33] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[28] = ((nFromData)) & 0xFF; \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[24] = ((nFromData)) & 0xFF; \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[20] = ((nFromData)) & 0xFF; \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[35] = ((nFromData)) & 0xFF; \
           (pToData)[34] = ((pToData)[34] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[31] = ((nFromData)) & 0xFF; \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[28] = ((pToData)[28] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[27] = ((nFromData)) & 0xFF; \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[24] = ((pToData)[24] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[23] = ((nFromData)) & 0xFF; \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((nFromData)) & 0xFF; \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS8(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[32] = ((nFromData)) & 0xFF; \
           (pToData)[33] = ((pToData)[33] & ~ 0x03) | (((nFromData) >> 8) & 0x03); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS7(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[28] = ((nFromData)) & 0xFF; \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS6(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[24] = ((nFromData)) & 0xFF; \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS5(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[20] = ((nFromData)) & 0xFF; \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS4(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_SET_FS0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[35] ; \
           (nToData) |= (uint32) ((pFromData)[34] & 0x03) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[31] ; \
           (nToData) |= (uint32) (pFromData)[30] << 8; \
           (nToData) |= (uint32) (pFromData)[29] << 16; \
           (nToData) |= (uint32) (pFromData)[28] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[27] ; \
           (nToData) |= (uint32) (pFromData)[26] << 8; \
           (nToData) |= (uint32) (pFromData)[25] << 16; \
           (nToData) |= (uint32) (pFromData)[24] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[23] ; \
           (nToData) |= (uint32) (pFromData)[22] << 8; \
           (nToData) |= (uint32) (pFromData)[21] << 16; \
           (nToData) |= (uint32) (pFromData)[20] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[19] ; \
           (nToData) |= (uint32) (pFromData)[18] << 8; \
           (nToData) |= (uint32) (pFromData)[17] << 16; \
           (nToData) |= (uint32) (pFromData)[16] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[15] ; \
           (nToData) |= (uint32) (pFromData)[14] << 8; \
           (nToData) |= (uint32) (pFromData)[13] << 16; \
           (nToData) |= (uint32) (pFromData)[12] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[11] ; \
           (nToData) |= (uint32) (pFromData)[10] << 8; \
           (nToData) |= (uint32) (pFromData)[9] << 16; \
           (nToData) |= (uint32) (pFromData)[8] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) (pFromData)[6] << 8; \
           (nToData) |= (uint32) (pFromData)[5] << 16; \
           (nToData) |= (uint32) (pFromData)[4] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
           (nToData) |= (uint32) (pFromData)[0] << 24; \
          } while(0)

#else
#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[32] ; \
           (nToData) |= (uint32) ((pFromData)[33] & 0x03) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[28] ; \
           (nToData) |= (uint32) (pFromData)[29] << 8; \
           (nToData) |= (uint32) (pFromData)[30] << 16; \
           (nToData) |= (uint32) (pFromData)[31] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[24] ; \
           (nToData) |= (uint32) (pFromData)[25] << 8; \
           (nToData) |= (uint32) (pFromData)[26] << 16; \
           (nToData) |= (uint32) (pFromData)[27] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[20] ; \
           (nToData) |= (uint32) (pFromData)[21] << 8; \
           (nToData) |= (uint32) (pFromData)[22] << 16; \
           (nToData) |= (uint32) (pFromData)[23] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[16] ; \
           (nToData) |= (uint32) (pFromData)[17] << 8; \
           (nToData) |= (uint32) (pFromData)[18] << 16; \
           (nToData) |= (uint32) (pFromData)[19] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[12] ; \
           (nToData) |= (uint32) (pFromData)[13] << 8; \
           (nToData) |= (uint32) (pFromData)[14] << 16; \
           (nToData) |= (uint32) (pFromData)[15] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[8] ; \
           (nToData) |= (uint32) (pFromData)[9] << 8; \
           (nToData) |= (uint32) (pFromData)[10] << 16; \
           (nToData) |= (uint32) (pFromData)[11] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
           (nToData) |= (uint32) (pFromData)[5] << 8; \
           (nToData) |= (uint32) (pFromData)[6] << 16; \
           (nToData) |= (uint32) (pFromData)[7] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
           (nToData) |= (uint32) (pFromData)[3] << 24; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[35] ; \
           (nToData) |= (uint32) ((pFromData)[34] & 0x03) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[31] ; \
           (nToData) |= (uint32) (pFromData)[30] << 8; \
           (nToData) |= (uint32) (pFromData)[29] << 16; \
           (nToData) |= (uint32) (pFromData)[28] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[27] ; \
           (nToData) |= (uint32) (pFromData)[26] << 8; \
           (nToData) |= (uint32) (pFromData)[25] << 16; \
           (nToData) |= (uint32) (pFromData)[24] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[23] ; \
           (nToData) |= (uint32) (pFromData)[22] << 8; \
           (nToData) |= (uint32) (pFromData)[21] << 16; \
           (nToData) |= (uint32) (pFromData)[20] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[19] ; \
           (nToData) |= (uint32) (pFromData)[18] << 8; \
           (nToData) |= (uint32) (pFromData)[17] << 16; \
           (nToData) |= (uint32) (pFromData)[16] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[15] ; \
           (nToData) |= (uint32) (pFromData)[14] << 8; \
           (nToData) |= (uint32) (pFromData)[13] << 16; \
           (nToData) |= (uint32) (pFromData)[12] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[11] ; \
           (nToData) |= (uint32) (pFromData)[10] << 8; \
           (nToData) |= (uint32) (pFromData)[9] << 16; \
           (nToData) |= (uint32) (pFromData)[8] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
           (nToData) |= (uint32) (pFromData)[6] << 8; \
           (nToData) |= (uint32) (pFromData)[5] << 16; \
           (nToData) |= (uint32) (pFromData)[4] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
           (nToData) |= (uint32) (pFromData)[0] << 24; \
          } while(0)

#else
#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS8(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[32] ; \
           (nToData) |= (uint32) ((pFromData)[33] & 0x03) << 8; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS7(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[28] ; \
           (nToData) |= (uint32) (pFromData)[29] << 8; \
           (nToData) |= (uint32) (pFromData)[30] << 16; \
           (nToData) |= (uint32) (pFromData)[31] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS6(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[24] ; \
           (nToData) |= (uint32) (pFromData)[25] << 8; \
           (nToData) |= (uint32) (pFromData)[26] << 16; \
           (nToData) |= (uint32) (pFromData)[27] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS5(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[20] ; \
           (nToData) |= (uint32) (pFromData)[21] << 8; \
           (nToData) |= (uint32) (pFromData)[22] << 16; \
           (nToData) |= (uint32) (pFromData)[23] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS4(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[16] ; \
           (nToData) |= (uint32) (pFromData)[17] << 8; \
           (nToData) |= (uint32) (pFromData)[18] << 16; \
           (nToData) |= (uint32) (pFromData)[19] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[12] ; \
           (nToData) |= (uint32) (pFromData)[13] << 8; \
           (nToData) |= (uint32) (pFromData)[14] << 16; \
           (nToData) |= (uint32) (pFromData)[15] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[8] ; \
           (nToData) |= (uint32) (pFromData)[9] << 8; \
           (nToData) |= (uint32) (pFromData)[10] << 16; \
           (nToData) |= (uint32) (pFromData)[11] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
           (nToData) |= (uint32) (pFromData)[5] << 8; \
           (nToData) |= (uint32) (pFromData)[6] << 16; \
           (nToData) |= (uint32) (pFromData)[7] << 24; \
          } while(0)

#define SB_ZF_FABBM9600NMFULLSTATUSENTRY_GET_FS0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
           (nToData) |= (uint32) (pFromData)[3] << 24; \
          } while(0)

#endif
#endif
