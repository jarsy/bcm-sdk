/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200NmRankAddr.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM3200_NMRANKADDR_H
#define SB_ZF_FAB_BM3200_NMRANKADDR_H

#define SB_ZF_FAB_BM3200_NMRANKADDR_SIZE_IN_BYTES 2
#define SB_ZF_FAB_BM3200_NMRANKADDR_SIZE 2
#define SB_ZF_FAB_BM3200_NMRANKADDR_M_BSELINGRESS_BITS "6:6"
#define SB_ZF_FAB_BM3200_NMRANKADDR_M_NADDR_BITS "5:0"


typedef struct _sbZfFabBm3200NmRankAddr {
  uint32 m_bSelIngress;
  uint32 m_nAddr;
} sbZfFabBm3200NmRankAddr_t;

uint32
sbZfFabBm3200NmRankAddr_Pack(sbZfFabBm3200NmRankAddr_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex);
void
sbZfFabBm3200NmRankAddr_Unpack(sbZfFabBm3200NmRankAddr_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex);
void
sbZfFabBm3200NmRankAddr_InitInstance(sbZfFabBm3200NmRankAddr_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200NMRANKADDR_SET_SEL_INGRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM3200NMRANKADDR_SET_ADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#else
#define SB_ZF_FABBM3200NMRANKADDR_SET_SEL_INGRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM3200NMRANKADDR_SET_ADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200NMRANKADDR_SET_SEL_INGRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM3200NMRANKADDR_SET_ADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#else
#define SB_ZF_FABBM3200NMRANKADDR_SET_SEL_INGRESS(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM3200NMRANKADDR_SET_ADDR(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200NMRANKADDR_GET_SEL_INGRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM3200NMRANKADDR_GET_ADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#else
#define SB_ZF_FABBM3200NMRANKADDR_GET_SEL_INGRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM3200NMRANKADDR_GET_ADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200NMRANKADDR_GET_SEL_INGRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM3200NMRANKADDR_GET_ADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#else
#define SB_ZF_FABBM3200NMRANKADDR_GET_SEL_INGRESS(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM3200NMRANKADDR_GET_ADDR(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200NmRankAddr.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_BM3200_NMRANKADDR_CONSOLE_H
#define SB_ZF_FAB_BM3200_NMRANKADDR_CONSOLE_H



void
sbZfFabBm3200NmRankAddr_Print(sbZfFabBm3200NmRankAddr_t *pFromStruct);
int
sbZfFabBm3200NmRankAddr_SPrint(sbZfFabBm3200NmRankAddr_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm3200NmRankAddr_Validate(sbZfFabBm3200NmRankAddr_t *pZf);
int
sbZfFabBm3200NmRankAddr_SetField(sbZfFabBm3200NmRankAddr_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_NMRANKADDR_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

