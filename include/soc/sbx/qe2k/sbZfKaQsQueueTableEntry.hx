/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsQueueTableEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQSQUEUETABLEENTRY_H
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_H

#define SB_ZF_ZFKAQSQUEUETABLEENTRY_SIZE_IN_BYTES 8
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_SIZE 8
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_M_NCREDIT_BITS "63:39"
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_M_NHPLEN_BITS "38:37"
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_M_NDEPTH_BITS "36:33"
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_M_NQ2EC_BITS "32:16"
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_M_NLOCALQ_BITS "15:15"
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_M_NMAXHOLDTS_BITS "14:12"
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_M_NQUEUETYPE_BITS "11:8"
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_M_NSHAPERATEMSB_BITS "7:0"


typedef struct _sbZfKaQsQueueTableEntry {
  uint32 m_nCredit;
  uint32 m_nHpLen;
  uint32 m_nDepth;
  uint32 m_nQ2Ec;
  uint32 m_nLocalQ;
  uint32 m_nMaxHoldTs;
  uint32 m_nQueueType;
  uint32 m_nShapeRateMSB;
} sbZfKaQsQueueTableEntry_t;

uint32
sbZfKaQsQueueTableEntry_Pack(sbZfKaQsQueueTableEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex);
void
sbZfKaQsQueueTableEntry_Unpack(sbZfKaQsQueueTableEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex);
void
sbZfKaQsQueueTableEntry_InitInstance(sbZfKaQsQueueTableEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSQUEUETABLEENTRY_SET_CREDIT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_HPLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x03 << 5)) | (((nFromData) & 0x03) << 5); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_Q2EC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~ 0x01) | (((nFromData) >> 16) & 0x01); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_LOCALQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_MAXHOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_QUEUETYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_SHAPERATEMSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAQSQUEUETABLEENTRY_SET_CREDIT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_HPLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x03 << 5)) | (((nFromData) & 0x03) << 5); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_Q2EC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x01) | (((nFromData) >> 16) & 0x01); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_LOCALQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_MAXHOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_QUEUETYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_SHAPERATEMSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSQUEUETABLEENTRY_SET_CREDIT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_HPLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x03 << 5)) | (((nFromData) & 0x03) << 5); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[7] = ((pToData)[7] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_Q2EC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((nFromData)) & 0xFF; \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~ 0x01) | (((nFromData) >> 16) & 0x01); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_LOCALQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_MAXHOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_QUEUETYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_SHAPERATEMSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAQSQUEUETABLEENTRY_SET_CREDIT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
           (pToData)[5] = ((pToData)[5] & ~0xFF) | (((nFromData) >> 1) & 0xFF); \
           (pToData)[6] = ((pToData)[6] & ~0xFF) | (((nFromData) >> 9) & 0xFF); \
           (pToData)[7] = ((pToData)[7] & ~0xFF) | (((nFromData) >> 17) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_HPLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x03 << 5)) | (((nFromData) & 0x03) << 5); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[4] = ((pToData)[4] & ~(0x0f << 1)) | (((nFromData) & 0x0f) << 1); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_Q2EC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((nFromData)) & 0xFF; \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[4] = ((pToData)[4] & ~ 0x01) | (((nFromData) >> 16) & 0x01); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_LOCALQ(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_MAXHOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 4)) | (((nFromData) & 0x07) << 4); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_QUEUETYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x0f) | ((nFromData) & 0x0f); \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_SET_SHAPERATEMSB(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSQUEUETABLEENTRY_GET_CREDIT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[6] << 1; \
           (nToData) |= (uint32) (pFromData)[5] << 9; \
           (nToData) |= (uint32) (pFromData)[4] << 17; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_HPLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 5) & 0x03; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_Q2EC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[0] << 8; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x01) << 16; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_LOCALQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_MAXHOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_QUEUETYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_SHAPERATEMSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAQSQUEUETABLEENTRY_GET_CREDIT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[5] << 1; \
           (nToData) |= (uint32) (pFromData)[6] << 9; \
           (nToData) |= (uint32) (pFromData)[7] << 17; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_HPLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 5) & 0x03; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_Q2EC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[3] << 8; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x01) << 16; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_LOCALQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_MAXHOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_QUEUETYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_SHAPERATEMSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSQUEUETABLEENTRY_GET_CREDIT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[6] << 1; \
           (nToData) |= (uint32) (pFromData)[5] << 9; \
           (nToData) |= (uint32) (pFromData)[4] << 17; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_HPLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 5) & 0x03; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[7] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_Q2EC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[1] ; \
           (nToData) |= (uint32) (pFromData)[0] << 8; \
           (nToData) |= (uint32) ((pFromData)[7] & 0x01) << 16; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_LOCALQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_MAXHOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_QUEUETYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_SHAPERATEMSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAQSQUEUETABLEENTRY_GET_CREDIT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 7) & 0x01; \
           (nToData) |= (uint32) (pFromData)[5] << 1; \
           (nToData) |= (uint32) (pFromData)[6] << 9; \
           (nToData) |= (uint32) (pFromData)[7] << 17; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_HPLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 5) & 0x03; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[4] >> 1) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_Q2EC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[2] ; \
           (nToData) |= (uint32) (pFromData)[3] << 8; \
           (nToData) |= (uint32) ((pFromData)[4] & 0x01) << 16; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_LOCALQ(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_MAXHOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 4) & 0x07; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_QUEUETYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1]) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSQUEUETABLEENTRY_GET_SHAPERATEMSB(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#endif
