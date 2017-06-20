/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEpIpMplsLabels.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEPIPMPLSLABELS_H
#define SB_ZF_ZFKAEPIPMPLSLABELS_H

#define SB_ZF_ZFKAEPIPMPLSLABELS_SIZE_IN_BYTES 8
#define SB_ZF_ZFKAEPIPMPLSLABELS_SIZE 8
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NLABEL1_BITS "63:44"
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NOP_BITS "43:43"
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NLINK_BITS "42:41"
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NSTACK1_BITS "40:40"
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NTTTL1_BITS "39:32"
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NLABEL0_BITS "31:12"
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NEXP_BITS "11:9"
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NSTACK0_BITS "8:8"
#define SB_ZF_ZFKAEPIPMPLSLABELS_M_NTTTL0_BITS "7:0"


typedef struct _sbZfKaEpIpMplsLabels {
  uint32 m_nLabel1;
  uint32 m_nOp;
  uint32 m_nLink;
  uint32 m_nStack1;
  uint32 m_nTttl1;
  uint32 m_nLabel0;
  uint32 m_nExp;
  uint32 m_nStack0;
  uint32 m_nTttl0;
} sbZfKaEpIpMplsLabels_t;

uint32
sbZfKaEpIpMplsLabels_Pack(sbZfKaEpIpMplsLabels_t *pFrom,
                          uint8 *pToData,
                          uint32 nMaxToDataIndex);
void
sbZfKaEpIpMplsLabels_Unpack(sbZfKaEpIpMplsLabels_t *pToStruct,
                            uint8 *pFromData,
                            uint32 nMaxToDataIndex);
void
sbZfKaEpIpMplsLabels_InitInstance(sbZfKaEpIpMplsLabels_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPIPMPLSLABELS_SET_LABEL1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_OP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_LINK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_STACK1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_TTL1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_LABEL0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_EXP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_STACK0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_TTL0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAEPIPMPLSLABELS_SET_LABEL1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_OP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_LINK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_STACK1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_TTL1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_LABEL0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_EXP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_STACK0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_TTL0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPIPMPLSLABELS_SET_LABEL1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_OP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_LINK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_STACK1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[6] = ((pToData)[6] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_TTL1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_LABEL0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_EXP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_STACK0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_TTL0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAEPIPMPLSLABELS_SET_LABEL1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_OP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_LINK(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~(0x03 << 1)) | (((nFromData) & 0x03) << 1); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_STACK1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[5] = ((pToData)[5] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_TTL1(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((nFromData)) & 0xFF; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_LABEL0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 4) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 12) & 0xFF); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_EXP(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_STACK0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_SET_TTL0(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPIPMPLSLABELS_GET_LABEL1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[5] << 4; \
           (nToData) |= (uint32) (pFromData)[4] << 12; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_OP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_LINK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_STACK1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_TTL1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_LABEL0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[1] << 4; \
           (nToData) |= (uint32) (pFromData)[0] << 12; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_EXP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_STACK0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_TTL0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAEPIPMPLSLABELS_GET_LABEL1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[6] << 4; \
           (nToData) |= (uint32) (pFromData)[7] << 12; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_OP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_LINK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_STACK1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_TTL1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_LABEL0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[2] << 4; \
           (nToData) |= (uint32) (pFromData)[3] << 12; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_EXP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_STACK0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_TTL0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPIPMPLSLABELS_GET_LABEL1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[5] << 4; \
           (nToData) |= (uint32) (pFromData)[4] << 12; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_OP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_LINK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_STACK1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[6]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_TTL1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[7] ; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_LABEL0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[1] << 4; \
           (nToData) |= (uint32) (pFromData)[0] << 12; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_EXP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_STACK0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_TTL0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAEPIPMPLSLABELS_GET_LABEL1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[6] << 4; \
           (nToData) |= (uint32) (pFromData)[7] << 12; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_OP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_LINK(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5] >> 1) & 0x03; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_STACK1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[5]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_TTL1(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[4] ; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_LABEL0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x0f; \
           (nToData) |= (uint32) (pFromData)[2] << 4; \
           (nToData) |= (uint32) (pFromData)[3] << 12; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_EXP(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_STACK0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KAEPIPMPLSLABELS_GET_TTL0(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#endif
