/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEpIp16BitRewrite.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEPIP16BITREWRITE_H
#define SB_ZF_ZFKAEPIP16BITREWRITE_H

#define SB_ZF_ZFKAEPIP16BITREWRITE_SIZE_IN_BYTES 2
#define SB_ZF_ZFKAEPIP16BITREWRITE_SIZE 2
#define SB_ZF_ZFKAEPIP16BITREWRITE_M_NDATA_BITS "15:0"


typedef struct _sbZfKaEpIp16BitRewrite {
  uint32 m_nData;
} sbZfKaEpIp16BitRewrite_t;

uint32
sbZfKaEpIp16BitRewrite_Pack(sbZfKaEpIp16BitRewrite_t *pFrom,
                            uint8 *pToData,
                            uint32 nMaxToDataIndex);
void
sbZfKaEpIp16BitRewrite_Unpack(sbZfKaEpIp16BitRewrite_t *pToStruct,
                              uint8 *pFromData,
                              uint32 nMaxToDataIndex);
void
sbZfKaEpIp16BitRewrite_InitInstance(sbZfKaEpIp16BitRewrite_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPIP16BITREWRITE_SET_DATA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_KAEPIP16BITREWRITE_SET_DATA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPIP16BITREWRITE_SET_DATA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_KAEPIP16BITREWRITE_SET_DATA(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPIP16BITREWRITE_GET_DATA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_KAEPIP16BITREWRITE_GET_DATA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPIP16BITREWRITE_GET_DATA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_KAEPIP16BITREWRITE_GET_DATA(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
          } while(0)

#endif
#endif
