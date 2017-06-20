/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEgNotTmePortRemapAddr.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEGNOTTMEPORTREMAPADDR_H
#define SB_ZF_ZFKAEGNOTTMEPORTREMAPADDR_H

#define SB_ZF_ZFKAEGNOTTMEPORTREMAPADDR_SIZE_IN_BYTES 4
#define SB_ZF_ZFKAEGNOTTMEPORTREMAPADDR_SIZE 4
#define SB_ZF_ZFKAEGNOTTMEPORTREMAPADDR_M_NTXDMA_BITS "8:8"
#define SB_ZF_ZFKAEGNOTTMEPORTREMAPADDR_M_NEF_BITS "7:7"
#define SB_ZF_ZFKAEGNOTTMEPORTREMAPADDR_M_NQE1K_BITS "6:6"
#define SB_ZF_ZFKAEGNOTTMEPORTREMAPADDR_M_NPORT_BITS "5:0"


typedef struct _sbZfKaEgNotTmePortRemapAddr {
  uint8 m_nTxdma;
  uint8 m_nEf;
  uint8 m_nQe1k;
  uint32 m_nPort;
} sbZfKaEgNotTmePortRemapAddr_t;

uint32
sbZfKaEgNotTmePortRemapAddr_Pack(sbZfKaEgNotTmePortRemapAddr_t *pFrom,
                                 uint8 *pToData,
                                 uint32 nMaxToDataIndex);
void
sbZfKaEgNotTmePortRemapAddr_Unpack(sbZfKaEgNotTmePortRemapAddr_t *pToStruct,
                                   uint8 *pFromData,
                                   uint32 nMaxToDataIndex);
void
sbZfKaEgNotTmePortRemapAddr_InitInstance(sbZfKaEgNotTmePortRemapAddr_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_SET_TXDMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_SET_EF(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_SET_QE1K(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_SET_TXDMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_SET_EF(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_SET_QE1K(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_SET_TXDMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_SET_EF(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_SET_QE1K(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#else
#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_SET_TXDMA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_SET_EF(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_SET_QE1K(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_SET_DPORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_GET_TXDMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_GET_EF(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_GET_QE1K(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#else
#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_GET_TXDMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_GET_EF(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_GET_QE1K(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_GET_TXDMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_GET_EF(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_GET_QE1K(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#else
#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_GET_TXDMA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_GET_EF(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_GET_QE1K(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_KAEGNOTTMEPORTREMAPADDR_GET_DPORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#endif
#endif
