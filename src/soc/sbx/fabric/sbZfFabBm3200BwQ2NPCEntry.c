/*
 * $Id: sbZfFabBm3200BwQ2NPCEntry.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200BwQ2NPCEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200BwQ2NPCEntry_Pack(sbZfFabBm3200BwQ2NPCEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_Q2NPC_ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nNPC */
  (pToData)[3] |= ((pFrom)->m_nNPC) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nNPC >> 8) & 0x3f;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_Q2NPC_ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nNPC */
  (pToData)[0] |= ((pFrom)->m_nNPC) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nNPC >> 8) & 0x3f;
#endif

  return SB_ZF_FAB_BM3200_Q2NPC_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwQ2NPCEntry_Unpack(sbZfFabBm3200BwQ2NPCEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nNPC */
  (pToStruct)->m_nNPC =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nNPC |=  (uint32)  ((pFromData)[2] & 0x3f) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nNPC */
  (pToStruct)->m_nNPC =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nNPC |=  (uint32)  ((pFromData)[1] & 0x3f) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwQ2NPCEntry_InitInstance(sbZfFabBm3200BwQ2NPCEntry_t *pFrame) {

  pFrame->m_nNPC =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwQ2NPCEntry.c,v 1.4 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200BwQ2NPCEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwQ2NPCEntry_Print(sbZfFabBm3200BwQ2NPCEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwQ2NPCEntry:: npc=0x%04x"), (unsigned int)  pFromStruct->m_nNPC));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200BwQ2NPCEntry_SPrint(sbZfFabBm3200BwQ2NPCEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwQ2NPCEntry:: npc=0x%04x", (unsigned int)  pFromStruct->m_nNPC);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwQ2NPCEntry_Validate(sbZfFabBm3200BwQ2NPCEntry_t *pZf) {

  if (pZf->m_nNPC > 0x3fff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwQ2NPCEntry_SetField(sbZfFabBm3200BwQ2NPCEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nnpc") == 0) {
    s->m_nNPC = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

