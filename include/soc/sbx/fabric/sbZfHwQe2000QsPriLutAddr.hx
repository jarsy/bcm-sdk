/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfHwQe2000QsPriLutAddr.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_H
#define SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_H

#define SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_SIZE_IN_BYTES 4
#define SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_SIZE 4
#define SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_M_NRESERVED_BITS "31:13"
#define SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_M_BSHAPED_BITS "12:12"
#define SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_M_NDEPTH_BITS "11:9"
#define SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_M_BANEMICAGED_BITS "8:8"
#define SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_M_NQTYPE_BITS "7:4"
#define SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_M_BEFAGED_BITS "3:3"
#define SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_M_NCREDITLEVEL_BITS "2:2"
#define SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_M_BHOLDTS_BITS "1:1"
#define SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_M_NPKTLEN_BITS "0:0"


typedef struct _sbZfHwQe2000QsPriLutAddr {
  uint32 m_nReserved;
  uint8 m_bShaped;
  uint32 m_nDepth;
  uint8 m_bAnemicAged;
  uint32 m_nQType;
  uint8 m_bEfAged;
  uint32 m_nCreditLevel;
  uint8 m_bHoldTs;
  uint32 m_nPktLen;
} sbZfHwQe2000QsPriLutAddr_t;

uint32
sbZfHwQe2000QsPriLutAddr_Pack(sbZfHwQe2000QsPriLutAddr_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex);
void
sbZfHwQe2000QsPriLutAddr_Unpack(sbZfHwQe2000QsPriLutAddr_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex);
void
sbZfHwQe2000QsPriLutAddr_InitInstance(sbZfHwQe2000QsPriLutAddr_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_HWQE2000QSPRILUTADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_SHAPED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_ANEMICAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_QTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_EFAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_CREDITLEVEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_HOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_PKTLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_HWQE2000QSPRILUTADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_SHAPED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_ANEMICAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_QTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_EFAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_CREDITLEVEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_HOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_PKTLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_HWQE2000QSPRILUTADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_SHAPED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_ANEMICAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_QTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_EFAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_CREDITLEVEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_HOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_PKTLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#else
#define SB_ZF_HWQE2000QSPRILUTADDR_SET_RES(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 5)) | (((nFromData) & 0x07) << 5); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 3) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 11) & 0xFF); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_SHAPED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x01 << 4)) | (((nFromData) & 0x01) << 4); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_DEPTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x07 << 1)) | (((nFromData) & 0x07) << 1); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_ANEMICAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_QTYPE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x0f << 4)) | (((nFromData) & 0x0f) << 4); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_EFAGED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 3)) | (((nFromData) & 0x01) << 3); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_CREDITLEVEL(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 2)) | (((nFromData) & 0x01) << 2); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_HOLDTS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 1)) | (((nFromData) & 0x01) << 1); \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_SET_PKTLEN(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_HWQE2000QSPRILUTADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[1] << 3; \
           (nToData) |= (uint32) (pFromData)[0] << 11; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_SHAPED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_ANEMICAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_QTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_EFAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_CREDITLEVEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_HOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_PKTLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_HWQE2000QSPRILUTADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[2] << 3; \
           (nToData) |= (uint32) (pFromData)[3] << 11; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_SHAPED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_ANEMICAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_QTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_EFAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_CREDITLEVEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_HOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_PKTLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_HWQE2000QSPRILUTADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[1] << 3; \
           (nToData) |= (uint32) (pFromData)[0] << 11; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_SHAPED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_ANEMICAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_QTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_EFAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_CREDITLEVEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_HOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_PKTLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x01; \
          } while(0)

#else
#define SB_ZF_HWQE2000QSPRILUTADDR_GET_RES(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 5) & 0x07; \
           (nToData) |= (uint32) (pFromData)[2] << 3; \
           (nToData) |= (uint32) (pFromData)[3] << 11; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_SHAPED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1] >> 4) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_DEPTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 1) & 0x07; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_ANEMICAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_QTYPE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 4) & 0x0f; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_EFAGED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 3) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_CREDITLEVEL(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 2) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_HOLDTS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 1) & 0x01; \
          } while(0)

#define SB_ZF_HWQE2000QSPRILUTADDR_GET_PKTLEN(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfHwQe2000QsPriLutAddr.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_CONSOLE_H
#define SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_CONSOLE_H



void
sbZfHwQe2000QsPriLutAddr_Print(sbZfHwQe2000QsPriLutAddr_t *pFromStruct);
int
sbZfHwQe2000QsPriLutAddr_SPrint(sbZfHwQe2000QsPriLutAddr_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfHwQe2000QsPriLutAddr_Validate(sbZfHwQe2000QsPriLutAddr_t *pZf);
int
sbZfHwQe2000QsPriLutAddr_SetField(sbZfHwQe2000QsPriLutAddr_t *s, char* name, int value);


#endif /* ifndef SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

