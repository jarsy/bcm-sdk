/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfKaEpIpPriExpTosRewrite.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_ZFKAEPIPPRIEXPTOSREWRITE_H
#define SB_ZF_ZFKAEPIPPRIEXPTOSREWRITE_H

#define SB_ZF_ZFKAEPIPPRIEXPTOSREWRITE_SIZE_IN_BYTES 1
#define SB_ZF_ZFKAEPIPPRIEXPTOSREWRITE_SIZE 1
#define SB_ZF_ZFKAEPIPPRIEXPTOSREWRITE_M_NTOS_BITS "7:0"


typedef struct _sbZfKaEpIpPriExpTosRewrite {
  uint32 m_nTos;
} sbZfKaEpIpPriExpTosRewrite_t;

uint32
sbZfKaEpIpPriExpTosRewrite_Pack(sbZfKaEpIpPriExpTosRewrite_t *pFrom,
                                uint8 *pToData,
                                uint32 nMaxToDataIndex);
void
sbZfKaEpIpPriExpTosRewrite_Unpack(sbZfKaEpIpPriExpTosRewrite_t *pToStruct,
                                  uint8 *pFromData,
                                  uint32 nMaxToDataIndex);
void
sbZfKaEpIpPriExpTosRewrite_InitInstance(sbZfKaEpIpPriExpTosRewrite_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPIPPRIEXPTOSREWRITE_SET_TOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAEPIPPRIEXPTOSREWRITE_SET_TOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPIPPRIEXPTOSREWRITE_SET_TOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
          } while(0)

#else
#define SB_ZF_KAEPIPPRIEXPTOSREWRITE_SET_TOS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPIPPRIEXPTOSREWRITE_GET_TOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAEPIPPRIEXPTOSREWRITE_GET_TOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_KAEPIPPRIEXPTOSREWRITE_GET_TOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
          } while(0)

#else
#define SB_ZF_KAEPIPPRIEXPTOSREWRITE_GET_TOS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
          } while(0)

#endif
#endif
