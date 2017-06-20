/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaRbClassHashVlanIPv4.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKARBCLASSHASHVLANIPV4_H
#define SB_ZF_ZFKARBCLASSHASHVLANIPV4_H

#define SB_ZF_ZFKARBCLASSHASHVLANIPV4_SIZE_IN_BYTES 48
#define SB_ZF_ZFKARBCLASSHASHVLANIPV4_SIZE 48
#define SB_ZF_ZFKARBCLASSHASHVLANIPV4_M_NPROTOCOL_BITS "383:376"
#define SB_ZF_ZFKARBCLASSHASHVLANIPV4_M_NPADWORD3_BITS "375:335"
#define SB_ZF_ZFKARBCLASSHASHVLANIPV4_M_NPADWORD1_BITS "334:272"
#define SB_ZF_ZFKARBCLASSHASHVLANIPV4_M_NIPSA_BITS "271:240"
#define SB_ZF_ZFKARBCLASSHASHVLANIPV4_M_NIPDA_BITS "239:208"
#define SB_ZF_ZFKARBCLASSHASHVLANIPV4_M_NSOCKET_BITS "207:176"
#define SB_ZF_ZFKARBCLASSHASHVLANIPV4_M_NPADWORD2_BITS "175:128"
#define SB_ZF_ZFKARBCLASSHASHVLANIPV4_M_NSPAREWORD1_BITS "127:64"
#define SB_ZF_ZFKARBCLASSHASHVLANIPV4_M_NSPAREWORD0_BITS "63:0"


typedef struct _sbZfKaRbClassHashVlanIPv4 {
  uint32 m_nProtocol;
  uint64 m_nPadWord3;
  uint64 m_nPadWord1;
  uint32 m_nIpSa;
  uint32 m_nIpDa;
  uint32 m_nSocket;
  uint64 m_nPadWord2;
  uint64 m_nSpareWord1;
  uint64 m_nSpareWord0;
} sbZfKaRbClassHashVlanIPv4_t;

uint32
sbZfKaRbClassHashVlanIPv4_Pack(sbZfKaRbClassHashVlanIPv4_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex);
void
sbZfKaRbClassHashVlanIPv4_Unpack(sbZfKaRbClassHashVlanIPv4_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex);
void
sbZfKaRbClassHashVlanIPv4_InitInstance(sbZfKaRbClassHashVlanIPv4_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_PROTOCOL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[44] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_PAD3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[42] = ((pToData)[42] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[41] = ((pToData)[41] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[40] = ((pToData)[40] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[47] = ((pToData)[47] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
           (pToData)[46] = ((pToData)[46] & ~0xFF) | (((nFromData) >> 25) & 0xFF); \
           (pToData)[45] = ((pToData)[45] & ~0xFF) | (((nFromData) >> 33) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_PAD1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[33] = ((nFromData)) & 0xFF; \
           (pToData)[32] = ((pToData)[32] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[39] = ((pToData)[39] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[38] = ((pToData)[38] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[37] = ((pToData)[37] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[36] = ((pToData)[36] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[43] = ((pToData)[43] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[42] = ((pToData)[42] & ~ 0x7f) | (((nFromData) >> 56) & 0x7f); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_IPSA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[29] = ((nFromData)) & 0xFF; \
           (pToData)[28] = ((pToData)[28] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[35] = ((pToData)[35] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[34] = ((pToData)[34] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_IPDA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[25] = ((nFromData)) & 0xFF; \
           (pToData)[24] = ((pToData)[24] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_SKT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((nFromData)) & 0xFF; \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_PAD2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((nFromData)) & 0xFF; \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_WORD1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_WORD0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#else
#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_PROTOCOL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[47] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_PAD3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[41] = ((pToData)[41] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[42] = ((pToData)[42] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[43] = ((pToData)[43] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[44] = ((pToData)[44] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
           (pToData)[45] = ((pToData)[45] & ~0xFF) | (((nFromData) >> 25) & 0xFF); \
           (pToData)[46] = ((pToData)[46] & ~0xFF) | (((nFromData) >> 33) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_PAD1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[34] = ((nFromData)) & 0xFF; \
           (pToData)[35] = ((pToData)[35] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[36] = ((pToData)[36] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[37] = ((pToData)[37] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[38] = ((pToData)[38] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[39] = ((pToData)[39] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[40] = ((pToData)[40] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[41] = ((pToData)[41] & ~ 0x7f) | (((nFromData) >> 56) & 0x7f); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_IPSA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[30] = ((nFromData)) & 0xFF; \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[32] = ((pToData)[32] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[33] = ((pToData)[33] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_IPDA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[26] = ((nFromData)) & 0xFF; \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[28] = ((pToData)[28] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_SKT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[22] = ((nFromData)) & 0xFF; \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[24] = ((pToData)[24] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_PAD2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_WORD1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_WORD0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_PROTOCOL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[44] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_PAD3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[42] = ((pToData)[42] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[41] = ((pToData)[41] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[40] = ((pToData)[40] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[47] = ((pToData)[47] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
           (pToData)[46] = ((pToData)[46] & ~0xFF) | (((nFromData) >> 25) & 0xFF); \
           (pToData)[45] = ((pToData)[45] & ~0xFF) | (((nFromData) >> 33) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_PAD1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[33] = ((nFromData)) & 0xFF; \
           (pToData)[32] = ((pToData)[32] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[39] = ((pToData)[39] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[38] = ((pToData)[38] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[37] = ((pToData)[37] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[36] = ((pToData)[36] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[43] = ((pToData)[43] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[42] = ((pToData)[42] & ~ 0x7f) | (((nFromData) >> 56) & 0x7f); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_IPSA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[29] = ((nFromData)) & 0xFF; \
           (pToData)[28] = ((pToData)[28] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[35] = ((pToData)[35] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[34] = ((pToData)[34] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_IPDA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[25] = ((nFromData)) & 0xFF; \
           (pToData)[24] = ((pToData)[24] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[30] = ((pToData)[30] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_SKT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[21] = ((nFromData)) & 0xFF; \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[26] = ((pToData)[26] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_PAD2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[19] = ((nFromData)) & 0xFF; \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[16] = ((pToData)[16] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[22] = ((pToData)[22] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_WORD1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[11] = ((nFromData)) & 0xFF; \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[8] = ((pToData)[8] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_WORD0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#else
#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_PROTOCOL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[47] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_PAD3(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[41] = ((pToData)[41] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[42] = ((pToData)[42] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[43] = ((pToData)[43] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[44] = ((pToData)[44] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
           (pToData)[45] = ((pToData)[45] & ~0xFF) | (((nFromData) >> 25) & 0xFF); \
           (pToData)[46] = ((pToData)[46] & ~0xFF) | (((nFromData) >> 33) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_PAD1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[34] = ((nFromData)) & 0xFF; \
           (pToData)[35] = ((pToData)[35] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[36] = ((pToData)[36] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[37] = ((pToData)[37] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[38] = ((pToData)[38] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[39] = ((pToData)[39] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[40] = ((pToData)[40] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[41] = ((pToData)[41] & ~ 0x7f) | (((nFromData) >> 56) & 0x7f); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_IPSA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[30] = ((nFromData)) & 0xFF; \
           (pToData)[31] = ((pToData)[31] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[32] = ((pToData)[32] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[33] = ((pToData)[33] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_IPDA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[26] = ((nFromData)) & 0xFF; \
           (pToData)[27] = ((pToData)[27] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[28] = ((pToData)[28] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[29] = ((pToData)[29] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_SKT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[22] = ((nFromData)) & 0xFF; \
           (pToData)[23] = ((pToData)[23] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[24] = ((pToData)[24] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[25] = ((pToData)[25] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_PAD2(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[16] = ((nFromData)) & 0xFF; \
           (pToData)[17] = ((pToData)[17] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[18] = ((pToData)[18] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[19] = ((pToData)[19] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[20] = ((pToData)[20] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[21] = ((pToData)[21] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_WORD1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[8] = ((nFromData)) & 0xFF; \
           (pToData)[9] = ((pToData)[9] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[10] = ((pToData)[10] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[11] = ((pToData)[11] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[12] = ((pToData)[12] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[13] = ((pToData)[13] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[14] = ((pToData)[14] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[15] = ((pToData)[15] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_SET_WORD0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 32) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 40) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 48) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 56) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_PROTOCOL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[44] ; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_PAD3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[42]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[41]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[40]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[47]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[46]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[45]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_PAD1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[33]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[32]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[39]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[38]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[37]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[36]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[43]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[42]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_IPSA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[29] ; \
           (nToData) |= (uint32) (pFromData)[28] << 8; \
           (nToData) |= (uint32) (pFromData)[35] << 16; \
           (nToData) |= (uint32) (pFromData)[34] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_IPDA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[25] ; \
           (nToData) |= (uint32) (pFromData)[24] << 8; \
           (nToData) |= (uint32) (pFromData)[31] << 16; \
           (nToData) |= (uint32) (pFromData)[30] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_SKT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[21] ; \
           (nToData) |= (uint32) (pFromData)[20] << 8; \
           (nToData) |= (uint32) (pFromData)[27] << 16; \
           (nToData) |= (uint32) (pFromData)[26] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_PAD2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[19]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[23]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[22]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_WORD1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[11]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_WORD0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[3]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[2]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[1]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[0]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[7]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[4]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#else
#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_PROTOCOL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[47] ; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_PAD3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[41]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[42]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[43]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[44]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[45]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[46]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_PAD1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[34]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[35]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[36]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[37]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[38]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[39]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[40]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[41]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_IPSA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[30] ; \
           (nToData) |= (uint32) (pFromData)[31] << 8; \
           (nToData) |= (uint32) (pFromData)[32] << 16; \
           (nToData) |= (uint32) (pFromData)[33] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_IPDA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[26] ; \
           (nToData) |= (uint32) (pFromData)[27] << 8; \
           (nToData) |= (uint32) (pFromData)[28] << 16; \
           (nToData) |= (uint32) (pFromData)[29] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_SKT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[22] ; \
           (nToData) |= (uint32) (pFromData)[23] << 8; \
           (nToData) |= (uint32) (pFromData)[24] << 16; \
           (nToData) |= (uint32) (pFromData)[25] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_PAD2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[16]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[20]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[21]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_WORD1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[8]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[11]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_WORD0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[0]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[1]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[2]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[3]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[4]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[7]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_PROTOCOL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[44] ; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_PAD3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[42]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[41]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[40]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[47]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[46]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[45]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_PAD1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[33]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[32]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[39]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[38]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[37]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[36]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[43]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[42]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_IPSA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[29] ; \
           (nToData) |= (uint32) (pFromData)[28] << 8; \
           (nToData) |= (uint32) (pFromData)[35] << 16; \
           (nToData) |= (uint32) (pFromData)[34] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_IPDA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[25] ; \
           (nToData) |= (uint32) (pFromData)[24] << 8; \
           (nToData) |= (uint32) (pFromData)[31] << 16; \
           (nToData) |= (uint32) (pFromData)[30] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_SKT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[21] ; \
           (nToData) |= (uint32) (pFromData)[20] << 8; \
           (nToData) |= (uint32) (pFromData)[27] << 16; \
           (nToData) |= (uint32) (pFromData)[26] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_PAD2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[19]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[16]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[23]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[22]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_WORD1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[11]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[8]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_WORD0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[3]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[2]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[1]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[0]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[7]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[4]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#else
#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_PROTOCOL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[47] ; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_PAD3(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[41]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[42]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[43]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[44]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[45]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[46]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_PAD1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[34]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[35]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[36]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[37]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[38]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[39]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[40]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[41]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_IPSA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[30] ; \
           (nToData) |= (uint32) (pFromData)[31] << 8; \
           (nToData) |= (uint32) (pFromData)[32] << 16; \
           (nToData) |= (uint32) (pFromData)[33] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_IPDA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[26] ; \
           (nToData) |= (uint32) (pFromData)[27] << 8; \
           (nToData) |= (uint32) (pFromData)[28] << 16; \
           (nToData) |= (uint32) (pFromData)[29] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_SKT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[22] ; \
           (nToData) |= (uint32) (pFromData)[23] << 8; \
           (nToData) |= (uint32) (pFromData)[24] << 16; \
           (nToData) |= (uint32) (pFromData)[25] << 24; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_PAD2(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[16]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[17]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[18]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[19]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[20]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[21]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_WORD1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[8]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[9]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[10]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[11]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[12]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[13]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[14]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[15]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#define SB_ZF_KARBCLASSHASHVLANIPV4_GET_WORD0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           COMPILER_64_SET((nToData), 0, (unsigned int) (pFromData)[0]); \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[1]); COMPILER_64_SHL(tmp, 8); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[2]); COMPILER_64_SHL(tmp, 16); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[3]); COMPILER_64_SHL(tmp, 24); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[4]); COMPILER_64_SHL(tmp, 32); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[5]); COMPILER_64_SHL(tmp, 40); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[6]); COMPILER_64_SHL(tmp, 48); COMPILER_64_OR(*tmp0, tmp); }; \
           { VOL COMPILER_UINT64 *tmp0 = &(nToData); COMPILER_64_SET(tmp, 0, (unsigned int) (pFromData)[7]); COMPILER_64_SHL(tmp, 56); COMPILER_64_OR(*tmp0, tmp); }; \
          } while(0)

#endif
#endif
