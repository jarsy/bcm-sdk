/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaQsPriLutAddr.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAQSPRILUTADDR_H
#define SB_ZF_ZFKAQSPRILUTADDR_H

#define SB_ZF_ZFKAQSPRILUTADDR_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAQSPRILUTADDR_SIZE 4
#define SB_ZF_ZFKAQSPRILUTADDR_M_NRESERVED_BITS "31:13"
#define SB_ZF_ZFKAQSPRILUTADDR_M_NSHAPED_BITS "12:12"
#define SB_ZF_ZFKAQSPRILUTADDR_M_NDEPTH_BITS "11:9"
#define SB_ZF_ZFKAQSPRILUTADDR_M_NANEMICAGED_BITS "8:8"
#define SB_ZF_ZFKAQSPRILUTADDR_M_NQTYPE_BITS "7:4"
#define SB_ZF_ZFKAQSPRILUTADDR_M_NEFAGED_BITS "3:3"
#define SB_ZF_ZFKAQSPRILUTADDR_M_NCREDITLEVEL_BITS "2:2"
#define SB_ZF_ZFKAQSPRILUTADDR_M_NHOLDTS_BITS "1:1"
#define SB_ZF_ZFKAQSPRILUTADDR_M_NPKTLEN_BITS "0:0"


typedef struct _sbZfKaQsPriLutAddr {
  uint32 m_nReserved;
  uint8 m_nShaped;
  uint32 m_nDepth;
  uint8 m_nAnemicAged;
  uint32 m_nQType;
  uint8 m_nEfAged;
  uint32 m_nCreditLevel;
  uint8 m_nHoldTs;
  uint32 m_nPktLen;
} sbZfKaQsPriLutAddr_t;

uint32
sbZfKaQsPriLutAddr_Pack(sbZfKaQsPriLutAddr_t *pFrom,
                        uint8 *pToData,
                        uint32 nMaxToDataIndex);
void
sbZfKaQsPriLutAddr_Unpack(sbZfKaQsPriLutAddr_t *pToStruct,
                          uint8 *pFromData,
                          uint32 nMaxToDataIndex);
void
sbZfKaQsPriLutAddr_InitInstance(sbZfKaQsPriLutAddr_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSPRILUTADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_SHAPED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_ANEMICAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_QTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_EFAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_CREDITLEVEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_HOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_PKTLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_KAQSPRILUTADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_SHAPED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_ANEMICAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_QTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_EFAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_CREDITLEVEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_HOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_PKTLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSPRILUTADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_SHAPED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_ANEMICAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_QTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_EFAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_CREDITLEVEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_HOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_PKTLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_KAQSPRILUTADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_SHAPED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_ANEMICAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_QTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_EFAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_CREDITLEVEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_HOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_SET_PKTLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSPRILUTADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[1] << 3; \
           (nToData) |= (uint32) (pFromData)[0] << 11; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_SHAPED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_ANEMICAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_QTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_EFAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_CREDITLEVEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_HOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_PKTLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_KAQSPRILUTADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[2] << 3; \
           (nToData) |= (uint32) (pFromData)[3] << 11; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_SHAPED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_ANEMICAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_QTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_EFAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_CREDITLEVEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_HOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_PKTLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAQSPRILUTADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[1] << 3; \
           (nToData) |= (uint32) (pFromData)[0] << 11; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_SHAPED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_ANEMICAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_QTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_EFAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_CREDITLEVEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_HOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_PKTLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_KAQSPRILUTADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[2] << 3; \
           (nToData) |= (uint32) (pFromData)[3] << 11; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_SHAPED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_ANEMICAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_QTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_EFAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_CREDITLEVEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_HOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_KAQSPRILUTADDR_GET_PKTLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#endif
