/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200XbTstcntAddr.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM3200_XBTSTCNTADDR_H
#define SB_ZF_FAB_BM3200_XBTSTCNTADDR_H

#define SB_ZF_FAB_BM3200_XBTSTCNTADDR_SIZE_IN_BYTES 2
#define SB_ZF_FAB_BM3200_XBTSTCNTADDR_SIZE 2
#define SB_ZF_FAB_BM3200_XBTSTCNTADDR_M_BEGRESS_BITS "6:6"
#define SB_ZF_FAB_BM3200_XBTSTCNTADDR_M_NNODE_BITS "5:0"


typedef struct _sbZfFabBm3200XbTstcntAddr {
  uint8 m_bEgress;
  int32 m_nNode;
} sbZfFabBm3200XbTstcntAddr_t;

uint32
sbZfFabBm3200XbTstcntAddr_Pack(sbZfFabBm3200XbTstcntAddr_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex);
void
sbZfFabBm3200XbTstcntAddr_Unpack(sbZfFabBm3200XbTstcntAddr_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex);
void
sbZfFabBm3200XbTstcntAddr_InitInstance(sbZfFabBm3200XbTstcntAddr_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200XBTSTCNTADDR_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM3200XBTSTCNTADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#else
#define SB_ZF_FABBM3200XBTSTCNTADDR_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM3200XBTSTCNTADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200XBTSTCNTADDR_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM3200XBTSTCNTADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((pToData)[3] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#else
#define SB_ZF_FABBM3200XBTSTCNTADDR_SET_PORT(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~(0x01 << 6)) | (((nFromData) & 0x01) << 6); \
          } while(0)

#define SB_ZF_FABBM3200XBTSTCNTADDR_SET_NODE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x3f) | ((nFromData) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200XBTSTCNTADDR_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM3200XBTSTCNTADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#else
#define SB_ZF_FABBM3200XBTSTCNTADDR_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM3200XBTSTCNTADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200XBTSTCNTADDR_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[3] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM3200XBTSTCNTADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[3]) & 0x3f; \
          } while(0)

#else
#define SB_ZF_FABBM3200XBTSTCNTADDR_GET_PORT(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint8) ((pFromData)[0] >> 6) & 0x01; \
          } while(0)

#define SB_ZF_FABBM3200XBTSTCNTADDR_GET_NODE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (int32) ((pFromData)[0]) & 0x3f; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200XbTstcntAddr.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_BM3200_XBTSTCNTADDR_CONSOLE_H
#define SB_ZF_FAB_BM3200_XBTSTCNTADDR_CONSOLE_H



void
sbZfFabBm3200XbTstcntAddr_Print(sbZfFabBm3200XbTstcntAddr_t *pFromStruct);
int
sbZfFabBm3200XbTstcntAddr_SPrint(sbZfFabBm3200XbTstcntAddr_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm3200XbTstcntAddr_Validate(sbZfFabBm3200XbTstcntAddr_t *pZf);
int
sbZfFabBm3200XbTstcntAddr_SetField(sbZfFabBm3200XbTstcntAddr_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_XBTSTCNTADDR_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

