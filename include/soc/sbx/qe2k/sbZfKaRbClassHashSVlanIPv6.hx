/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaRbClassHashSVlanIPv6.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKARBCLASSHASHSVLANIPV6_H
#define SB_ZF_ZFKARBCLASSHASHSVLANIPV6_H

#define SB_ZF_ZFKARBCLASSHASHSVLANIPV6_SIZE_IN_BYTES 50
#define SB_ZF_ZFKARBCLASSHASHSVLANIPV6_SIZE 50
#define SB_ZF_ZFKARBCLASSHASHSVLANIPV6_M_NPROTOCOL_BITS "399:392"
#define SB_ZF_ZFKARBCLASSHASHSVLANIPV6_M_NFLOW_BITS "391:372"
#define SB_ZF_ZFKARBCLASSHASHSVLANIPV6_M_NPADWORD2_BITS "371:345"
#define SB_ZF_ZFKARBCLASSHASHSVLANIPV6_M_NPADWORD1_BITS "344:288"
#define SB_ZF_ZFKARBCLASSHASHSVLANIPV6_M_NNIPSAHIGH_BITS "287:224"
#define SB_ZF_ZFKARBCLASSHASHSVLANIPV6_M_NNIPSALOW_BITS "223:160"
#define SB_ZF_ZFKARBCLASSHASHSVLANIPV6_M_NNIPDAHIGH_BITS "159:96"
#define SB_ZF_ZFKARBCLASSHASHSVLANIPV6_M_NNIPDALOW_BITS "95:32"
#define SB_ZF_ZFKARBCLASSHASHSVLANIPV6_M_NSOCKET_BITS "31:0"


typedef struct _sbZfKaRbClassHashSVlanIPv6 {
  uint32 m_nProtocol;
  uint32 m_nFlow;
  uint64 m_nPadWord2;
  uint64 m_nPadWord1;
  uint64 m_nnIpSaHigh;
  uint64 m_nnIpSaLow;
  uint64 m_nnIpDaHigh;
  uint64 m_nnIpDaLow;
  uint32 m_nSocket;
} sbZfKaRbClassHashSVlanIPv6_t;

uint32
sbZfKaRbClassHashSVlanIPv6_Pack(sbZfKaRbClassHashSVlanIPv6_t *pFrom,
                                uint8 *pToData,
                                uint32 nMaxToDataIndex);
void
sbZfKaRbClassHashSVlanIPv6_Unpack(sbZfKaRbClassHashSVlanIPv6_t *pToStruct,
                                  uint8 *pFromData,
                                  uint32 nMaxToDataIndex);
void
sbZfKaRbClassHashSVlanIPv6_InitInstance(sbZfKaRbClassHashSVlanIPv6_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_PROTOCOL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[50] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_FLOW(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[45] = ((pToData)[45] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[44] = ((pToData)[44] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[51] = ((pToData)[51] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_PAD2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[40] = ((pToData)[40] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[47] = ((pToData)[47] & ~0xFF) | (((nFromData) >> 7) & 0xFF); \
           (pToData)[46] = ((pToData)[46] & ~0xFF) | (((nFromData) >> 15) & 0xFF); \
           (pToData)[45] = ((pToData)[45] & ~ 0x0f) | (((nFromData) >> 23) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_PAD1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[39] = ((nFromData)) & 0xFF; \
           (pToData)[38] = ((pToData)[38] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[37] = ((pToData)[37] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[36] = ((pToData)[36] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[43] = ((pToData)[43] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[42] = ((pToData)[42] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[41] = ((pToData)[41] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[40] = ((pToData)[40] & ~ 0x01) | (((nFromData) >> 56) & 0x01); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_IPSAHIGH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[31] = ((nFromData)) & 0xFF; \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[28] = ((pToData)[28] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[35] = ((pToData)[35] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[34] = ((pToData)[34] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[33] = ((pToData)[33] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[32] = ((pToData)[32] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_IPSALOW(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[23] = ((nFromData)) & 0xFF; \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[24] = ((pToData)[24] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_IPDAHIGH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_IPDALOW(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_SKT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#else
#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_PROTOCOL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[49] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_FLOW(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[46] = ((pToData)[46] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[47] = ((pToData)[47] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[48] = ((pToData)[48] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_PAD2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[43] = ((pToData)[43] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[44] = ((pToData)[44] & ~0xFF) | (((nFromData) >> 7) & 0xFF); \
           (pToData)[45] = ((pToData)[45] & ~0xFF) | (((nFromData) >> 15) & 0xFF); \
           (pToData)[46] = ((pToData)[46] & ~ 0x0f) | (((nFromData) >> 23) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_PAD1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[36] = ((nFromData)) & 0xFF; \
           (pToData)[37] = ((pToData)[37] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[38] = ((pToData)[38] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[39] = ((pToData)[39] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[40] = ((pToData)[40] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[41] = ((pToData)[41] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[42] = ((pToData)[42] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[43] = ((pToData)[43] & ~ 0x01) | (((nFromData) >> 56) & 0x01); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_IPSAHIGH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[28] = ((nFromData)) & 0xFF; \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[32] = ((pToData)[32] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[33] = ((pToData)[33] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[34] = ((pToData)[34] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[35] = ((pToData)[35] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_IPSALOW(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[20] = ((nFromData)) & 0xFF; \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[24] = ((pToData)[24] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_IPDAHIGH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_IPDALOW(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_SKT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_PROTOCOL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[50] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_FLOW(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[45] = ((pToData)[45] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[44] = ((pToData)[44] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[51] = ((pToData)[51] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_PAD2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[40] = ((pToData)[40] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[47] = ((pToData)[47] & ~0xFF) | (((nFromData) >> 7) & 0xFF); \
           (pToData)[46] = ((pToData)[46] & ~0xFF) | (((nFromData) >> 15) & 0xFF); \
           (pToData)[45] = ((pToData)[45] & ~ 0x0f) | (((nFromData) >> 23) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_PAD1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[39] = ((nFromData)) & 0xFF; \
           (pToData)[38] = ((pToData)[38] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[37] = ((pToData)[37] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[36] = ((pToData)[36] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[43] = ((pToData)[43] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[42] = ((pToData)[42] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[41] = ((pToData)[41] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[40] = ((pToData)[40] & ~ 0x01) | (((nFromData) >> 56) & 0x01); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_IPSAHIGH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[31] = ((nFromData)) & 0xFF; \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[28] = ((pToData)[28] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[35] = ((pToData)[35] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[34] = ((pToData)[34] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[33] = ((pToData)[33] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[32] = ((pToData)[32] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_IPSALOW(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[23] = ((nFromData)) & 0xFF; \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[24] = ((pToData)[24] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_IPDAHIGH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[15] = ((nFromData)) & 0xFF; \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_IPDALOW(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_SKT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#else
#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_PROTOCOL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[49] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_FLOW(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[46] = ((pToData)[46] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[47] = ((pToData)[47] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[48] = ((pToData)[48] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_PAD2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[43] = ((pToData)[43] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
           (pToData)[44] = ((pToData)[44] & ~0xFF) | (((nFromData) >> 7) & 0xFF); \
           (pToData)[45] = ((pToData)[45] & ~0xFF) | (((nFromData) >> 15) & 0xFF); \
           (pToData)[46] = ((pToData)[46] & ~ 0x0f) | (((nFromData) >> 23) & 0x0f); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_PAD1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[36] = ((nFromData)) & 0xFF; \
           (pToData)[37] = ((pToData)[37] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[38] = ((pToData)[38] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[39] = ((pToData)[39] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[40] = ((pToData)[40] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[41] = ((pToData)[41] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[42] = ((pToData)[42] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[43] = ((pToData)[43] & ~ 0x01) | (((nFromData) >> 56) & 0x01); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_IPSAHIGH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[28] = ((nFromData)) & 0xFF; \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[32] = ((pToData)[32] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[33] = ((pToData)[33] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[34] = ((pToData)[34] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[35] = ((pToData)[35] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_IPSALOW(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[20] = ((nFromData)) & 0xFF; \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[24] = ((pToData)[24] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_IPDAHIGH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[12] = ((nFromData)) & 0xFF; \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_IPDALOW(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_SET_SKT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_PROTOCOL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[50] ; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_FLOW(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[45] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[44] << 4; \
           (nToData) |= (uint32) (pFromData)[51] << 12; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_PAD2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[40]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[47]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[46]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[45]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_PAD1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[39]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[38]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[37]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[36]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[43]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[42]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[41]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[40]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_IPSAHIGH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[31]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[30]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[29]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[28]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[35]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[34]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[33]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[32]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_IPSALOW(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[23]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[22]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[21]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[20]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[27]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[26]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[25]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[24]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_IPDAHIGH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[15]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_IPDALOW(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[7]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[4]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[11]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_SKT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
           (nToData) |= (uint32) (pFromData)[0] << 24; \
          } while(0)

#else
#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_PROTOCOL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[49] ; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_FLOW(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[46] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[47] << 4; \
           (nToData) |= (uint32) (pFromData)[48] << 12; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_PAD2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[43]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[44]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[45]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[46]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_PAD1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[36]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[37]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[38]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[39]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[40]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[41]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[42]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[43]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_IPSAHIGH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[28]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[29]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[30]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[31]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[32]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[33]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[34]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[35]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_IPSALOW(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[20]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[21]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[22]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[23]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[24]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[25]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[26]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[27]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_IPDAHIGH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[12]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_IPDALOW(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[4]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[7]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[11]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_SKT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
           (nToData) |= (uint32) (pFromData)[3] << 24; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_PROTOCOL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[50] ; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_FLOW(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[45] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[44] << 4; \
           (nToData) |= (uint32) (pFromData)[51] << 12; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_PAD2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[40]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[47]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[46]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[45]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_PAD1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[39]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[38]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[37]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[36]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[43]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[42]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[41]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[40]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_IPSAHIGH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[31]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[30]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[29]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[28]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[35]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[34]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[33]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[32]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_IPSALOW(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[23]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[22]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[21]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[20]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[27]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[26]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[25]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[24]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_IPDAHIGH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[15]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_IPDALOW(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[7]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[4]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[11]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_SKT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
           (nToData) |= (uint32) (pFromData)[0] << 24; \
          } while(0)

#else
#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_PROTOCOL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[49] ; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_FLOW(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[46] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[47] << 4; \
           (nToData) |= (uint32) (pFromData)[48] << 12; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_PAD2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[43]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[44]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[45]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[46]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_PAD1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[36]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[37]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[38]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[39]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[40]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[41]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[42]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[43]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_IPSAHIGH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[28]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[29]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[30]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[31]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[32]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[33]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[34]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[35]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_IPSALOW(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[20]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[21]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[22]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[23]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[24]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[25]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[26]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[27]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_IPDAHIGH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[12]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_IPDALOW(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[4]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[7]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[11]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHSVLANIPV6_GET_SKT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
           (nToData) |= (uint32) (pFromData)[3] << 24; \
          } while(0)

#endif
#endif
