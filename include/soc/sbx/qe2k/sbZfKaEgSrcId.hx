/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEgSrcId.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEGSRCID_H
#define SB_ZF_ZFKAEGSRCID_H

#define SB_ZF_ZFKAEGSRCID_SIZE_IN_BYTES 2
#define SB_ZF_ZFKAEGSRCID_SIZE 2
#define SB_ZF_ZFKAEGSRCID_M_NSRCID_BITS "11:0"


typedef struct _sbZfKaEgSrcId {
  uint32 m_nSrcId;
} sbZfKaEgSrcId_t;

uint32
sbZfKaEgSrcId_Pack(sbZfKaEgSrcId_t *pFrom,
                   uint8 *pToData,
                   uint32 nMaxToDataIndex);
void
sbZfKaEgSrcId_Unpack(sbZfKaEgSrcId_t *pToStruct,
                     uint8 *pFromData,
                     uint32 nMaxToDataIndex);
void
sbZfKaEgSrcId_InitInstance(sbZfKaEgSrcId_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGSRCID_SET_SRC_ID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAEGSRCID_SET_SRC_ID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGSRCID_SET_SRC_ID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#else
#define SB_ZF_KAEGSRCID_SET_SRC_ID(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x0f) | (((nFromData) >> 8) & 0x0f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGSRCID_GET_SRC_ID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 8; \
          } while(0)

#else
#define SB_ZF_KAEGSRCID_GET_SRC_ID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEGSRCID_GET_SRC_ID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x0f) << 8; \
          } while(0)

#else
#define SB_ZF_KAEGSRCID_GET_SRC_ID(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x0f) << 8; \
          } while(0)

#endif
#endif
