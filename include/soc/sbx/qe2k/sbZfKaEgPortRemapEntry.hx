/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEgPortRemapEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEGPORTREMAPENTRY_H
#define SB_ZF_ZFKAEGPORTREMAPENTRY_H

#define SB_ZF_ZFKAEGPORTREMAPENTRY_SIZE_IN_BYTES 2
#define SB_ZF_ZFKAEGPORTREMAPENTRY_SIZE 2
#define SB_ZF_ZFKAEGPORTREMAPENTRY_M_NRESERVED_BITS "15:9"
#define SB_ZF_ZFKAEGPORTREMAPENTRY_M_NMCFIFO_BITS "8:8"
#define SB_ZF_ZFKAEGPORTREMAPENTRY_M_NFIFOENABLE_BITS "7:7"
#define SB_ZF_ZFKAEGPORTREMAPENTRY_M_NFIFONUM_BITS "6:0"


typedef struct _sbZfKaEgPortRemapEntry {
  uint32 m_nReserved;
  uint8 m_nMcFifo;
  uint8 m_nFifoEnable;
  uint32 m_nFifoNum;
} sbZfKaEgPortRemapEntry_t;

uint32
sbZfKaEgPortRemapEntry_Pack(sbZfKaEgPortRemapEntry_t *pFrom,
                            uint8 *pToData,
                            uint32 nMaxToDataIndex);
void
sbZfKaEgPortRemapEntry_Unpack(sbZfKaEgPortRemapEntry_t *pToStruct,
                              uint8 *pFromData,
                              uint32 nMaxToDataIndex);
void
sbZfKaEgPortRemapEntry_InitInstance(sbZfKaEgPortRemapEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGPORTREMAPENTRY_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_SET_MCFIFO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_SET_FIFOENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_SET_FIFONUM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#else
#define SB_ZF_KAEGPORTREMAPENTRY_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_SET_MCFIFO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_SET_FIFOENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_SET_FIFONUM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGPORTREMAPENTRY_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_SET_MCFIFO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[2] = ((pToData)[2] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_SET_FIFOENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_SET_FIFONUM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#else
#define SB_ZF_KAEGPORTREMAPENTRY_SET_RESERVED(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~(0x7f << 1)) | (((nFromData) & 0x7f) << 1); \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_SET_MCFIFO(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[1] = ((pToData)[1] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_SET_FIFOENABLE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 7)) | (((nFromData) & 0x01) << 7); \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_SET_FIFONUM(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x7f) | ((nFromData) & 0x7f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGPORTREMAPENTRY_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_GET_MCFIFO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_GET_FIFOENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_GET_FIFONUM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x7f; \
          } while(0)

#else
#define SB_ZF_KAEGPORTREMAPENTRY_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_GET_MCFIFO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_GET_FIFOENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_GET_FIFONUM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x7f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGPORTREMAPENTRY_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[2] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_GET_MCFIFO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[2]) & 0x01; \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_GET_FIFOENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_GET_FIFONUM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x7f; \
          } while(0)

#else
#define SB_ZF_KAEGPORTREMAPENTRY_GET_RESERVED(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[1] >> 1) & 0x7f; \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_GET_MCFIFO(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[1]) & 0x01; \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_GET_FIFOENABLE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 7) & 0x01; \
          } while(0)

#define SB_ZF_KAEGPORTREMAPENTRY_GET_FIFONUM(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x7f; \
          } while(0)

#endif
#endif
