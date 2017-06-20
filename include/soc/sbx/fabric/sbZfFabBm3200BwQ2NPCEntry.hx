/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfFabBm3200BwQ2NPCEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_FAB_BM3200_Q2NPC_ENTRY_H
#define SB_ZF_FAB_BM3200_Q2NPC_ENTRY_H

#define SB_ZF_FAB_BM3200_Q2NPC_ENTRY_SIZE_IN_BYTES 2
#define SB_ZF_FAB_BM3200_Q2NPC_ENTRY_SIZE 2
#define SB_ZF_FAB_BM3200_Q2NPC_ENTRY_M_NNPC_BITS "13:0"


typedef struct _sbZfFabBm3200BwQ2NPCEntry {
  uint32 m_nNPC;
} sbZfFabBm3200BwQ2NPCEntry_t;

uint32
sbZfFabBm3200BwQ2NPCEntry_Pack(sbZfFabBm3200BwQ2NPCEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwQ2NPCEntry_Unpack(sbZfFabBm3200BwQ2NPCEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex);
void
sbZfFabBm3200BwQ2NPCEntry_InitInstance(sbZfFabBm3200BwQ2NPCEntry_t *pFrame);

#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWQ2NPCENTRY_SET_NPC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWQ2NPCENTRY_SET_NPC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWQ2NPCENTRY_SET_NPC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[3] = ((nFromData)) & 0xFF; \
           (pToData)[2] = ((pToData)[2] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#else
#define SB_ZF_FABBM3200BWQ2NPCENTRY_SET_NPC(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((nFromData)) & 0xFF; \
           (pToData)[1] = ((pToData)[1] & ~ 0x3f) | (((nFromData) >> 8) & 0x3f); \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWQ2NPCENTRY_GET_NPC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWQ2NPCENTRY_GET_NPC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 8; \
          } while(0)

#endif
#ifdef SAND_BIG_ENDIAN_HOST
#define SB_ZF_FABBM3200BWQ2NPCENTRY_GET_NPC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[3] ; \
           (nToData) |= (uint32) ((pFromData)[2] & 0x3f) << 8; \
          } while(0)

#else
#define SB_ZF_FABBM3200BWQ2NPCENTRY_GET_NPC(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) (pFromData)[0] ; \
           (nToData) |= (uint32) ((pFromData)[1] & 0x3f) << 8; \
          } while(0)

#endif
#endif
/*
 * $Id: sbZfFabBm3200BwQ2NPCEntry.hx,v 1.2 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */



#ifdef SB_ZF_INCLUDE_CONSOLE
#ifndef SB_ZF_FAB_BM3200_Q2NPC_ENTRY_CONSOLE_H
#define SB_ZF_FAB_BM3200_Q2NPC_ENTRY_CONSOLE_H



void
sbZfFabBm3200BwQ2NPCEntry_Print(sbZfFabBm3200BwQ2NPCEntry_t *pFromStruct);
int
sbZfFabBm3200BwQ2NPCEntry_SPrint(sbZfFabBm3200BwQ2NPCEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm3200BwQ2NPCEntry_Validate(sbZfFabBm3200BwQ2NPCEntry_t *pZf);
int
sbZfFabBm3200BwQ2NPCEntry_SetField(sbZfFabBm3200BwQ2NPCEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM3200_Q2NPC_ENTRY_CONSOLE_H */
#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

