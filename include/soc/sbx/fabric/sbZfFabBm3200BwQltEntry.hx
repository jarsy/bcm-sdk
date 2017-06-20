/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200BwQltEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM3200_QLT_ENTRY_H
#define SB_ZF_FAB_BM3200_QLT_ENTRY_H

#define SB_ZF_FAB_BM3200_QLT_ENTRY_SIZE_IN_BYTES 4
#define SB_ZF_FAB_BM3200_QLT_ENTRY_SIZE 4
#define SB_ZF_FAB_BM3200_QLT_ENTRY_M_NQUEUELENGTH_BITS "31:0"


typedef struct _sbZfFabBm3200BwQltEntry {
  uint32 m_nQueueLength;
} sbZfFabBm3200BwQltEntry_t;

uint32
sbZfFabBm3200BwQltEntry_Pack(sbZfFabBm3200BwQltEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwQltEntry_Unpack(sbZfFabBm3200BwQltEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwQltEntry_InitInstance(sbZfFabBm3200BwQltEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWQLTENTRY_SET_QUEUELENGTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWQLTENTRY_SET_QUEUELENGTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWQLTENTRY_SET_QUEUELENGTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[0] = ((pToData)[0] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWQLTENTRY_SET_QUEUELENGTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~0xFF) | (((nFromData) >> 8) & 0xFF); \
           (pToData)[2] = ((pToData)[2] & ~0xFF) | (((nFromData) >> 16) & 0xFF); \
           (pToData)[3] = ((pToData)[3] & ~0xFF) | (((nFromData) >> 24) & 0xFF); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWQLTENTRY_GET_QUEUELENGTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
           (nToData) |= (uint32) (pFromData)[0] << 24; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWQLTENTRY_GET_QUEUELENGTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
           (nToData) |= (uint32) (pFromData)[3] << 24; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWQLTENTRY_GET_QUEUELENGTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) (pFromData)[2] << 8; \
           (nToData) |= (uint32) (pFromData)[1] << 16; \
           (nToData) |= (uint32) (pFromData)[0] << 24; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWQLTENTRY_GET_QUEUELENGTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) (pFromData)[1] << 8; \
           (nToData) |= (uint32) (pFromData)[2] << 16; \
           (nToData) |= (uint32) (pFromData)[3] << 24; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200BwQltEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_BM3200_QLT_ENTRY_CONSOLE_H
#define SB_ZF_FAB_BM3200_QLT_ENTRY_CONSOLE_H



void
sbZfFabBm3200BwQltEntry_Print(sbZfFabBm3200BwQltEntry_t *pFromStruct);
int
sbZfFabBm3200BwQltEntry_SPrint(sbZfFabBm3200BwQltEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm3200BwQltEntry_Validate(sbZfFabBm3200BwQltEntry_t *pZf);
int
sbZfFabBm3200BwQltEntry_SetField(sbZfFabBm3200BwQltEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_QLT_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

