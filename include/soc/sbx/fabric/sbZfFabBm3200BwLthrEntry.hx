/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200BwLthrEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM3200_LTHR_ENTRY_H
#define SB_ZF_FAB_BM3200_LTHR_ENTRY_H

#define SB_ZF_FAB_BM3200_LTHR_ENTRY_SIZE_IN_BYTES 2
#define SB_ZF_FAB_BM3200_LTHR_ENTRY_SIZE 2
#define SB_ZF_FAB_BM3200_LTHR_ENTRY_M_NLENGTHTHRESH_BITS "15:0"


typedef struct _sbZfFabBm3200BwLthrEntry {
  uint32 m_nLengthThresh;
} sbZfFabBm3200BwLthrEntry_t;

uint32
sbZfFabBm3200BwLthrEntry_Pack(sbZfFabBm3200BwLthrEntry_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwLthrEntry_Unpack(sbZfFabBm3200BwLthrEntry_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwLthrEntry_InitInstance(sbZfFabBm3200BwLthrEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWLTHRENTRY_SET_LENGTHTHRESH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWLTHRENTRY_SET_LENGTHTHRESH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWLTHRENTRY_SET_LENGTHTHRESH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWLTHRENTRY_SET_LENGTHTHRESH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWLTHRENTRY_GET_LENGTHTHRESH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWLTHRENTRY_GET_LENGTHTHRESH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWLTHRENTRY_GET_LENGTHTHRESH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWLTHRENTRY_GET_LENGTHTHRESH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200BwLthrEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_BM3200_LTHR_ENTRY_CONSOLE_H
#define SB_ZF_FAB_BM3200_LTHR_ENTRY_CONSOLE_H



void
sbZfFabBm3200BwLthrEntry_Print(sbZfFabBm3200BwLthrEntry_t *pFromStruct);
int
sbZfFabBm3200BwLthrEntry_SPrint(sbZfFabBm3200BwLthrEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm3200BwLthrEntry_Validate(sbZfFabBm3200BwLthrEntry_t *pZf);
int
sbZfFabBm3200BwLthrEntry_SetField(sbZfFabBm3200BwLthrEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_LTHR_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

