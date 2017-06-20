/*
 * $Id: sbZfFabBm3200BwNPC2QEntry.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200BwNPC2QEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200BwNPC2QEntry_Pack(sbZfFabBm3200BwNPC2QEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_NPC2Q_ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nBaseGroup */
  (pToData)[3] |= ((pFrom)->m_nBaseGroup & 0x07) <<5;
  (pToData)[2] |= ((pFrom)->m_nBaseGroup >> 3) &0xFF;

  /* Pack Member: m_nGroups */
  (pToData)[3] |= ((pFrom)->m_nGroups & 0x1f);
#else
  int i;
  int size = SB_ZF_FAB_BM3200_NPC2Q_ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nBaseGroup */
  (pToData)[0] |= ((pFrom)->m_nBaseGroup & 0x07) <<5;
  (pToData)[1] |= ((pFrom)->m_nBaseGroup >> 3) &0xFF;

  /* Pack Member: m_nGroups */
  (pToData)[0] |= ((pFrom)->m_nGroups & 0x1f);
#endif

  return SB_ZF_FAB_BM3200_NPC2Q_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwNPC2QEntry_Unpack(sbZfFabBm3200BwNPC2QEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nBaseGroup */
  (pToStruct)->m_nBaseGroup =  (uint32)  ((pFromData)[3] >> 5) & 0x07;
  (pToStruct)->m_nBaseGroup |=  (uint32)  (pFromData)[2] << 3;

  /* Unpack Member: m_nGroups */
  (pToStruct)->m_nGroups =  (uint32)  ((pFromData)[3] ) & 0x1f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nBaseGroup */
  (pToStruct)->m_nBaseGroup =  (uint32)  ((pFromData)[0] >> 5) & 0x07;
  (pToStruct)->m_nBaseGroup |=  (uint32)  (pFromData)[1] << 3;

  /* Unpack Member: m_nGroups */
  (pToStruct)->m_nGroups =  (uint32)  ((pFromData)[0] ) & 0x1f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwNPC2QEntry_InitInstance(sbZfFabBm3200BwNPC2QEntry_t *pFrame) {

  pFrame->m_nBaseGroup =  (unsigned int)  0;
  pFrame->m_nGroups =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwNPC2QEntry.c,v 1.4 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200BwNPC2QEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwNPC2QEntry_Print(sbZfFabBm3200BwNPC2QEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwNPC2QEntry:: basegroup=0x%03x"), (unsigned int)  pFromStruct->m_nBaseGroup));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" groups=0x%02x"), (unsigned int)  pFromStruct->m_nGroups));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200BwNPC2QEntry_SPrint(sbZfFabBm3200BwNPC2QEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwNPC2QEntry:: basegroup=0x%03x", (unsigned int)  pFromStruct->m_nBaseGroup);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," groups=0x%02x", (unsigned int)  pFromStruct->m_nGroups);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwNPC2QEntry_Validate(sbZfFabBm3200BwNPC2QEntry_t *pZf) {

  if (pZf->m_nBaseGroup > 0x7ff) return 0;
  if (pZf->m_nGroups > 0x1f) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwNPC2QEntry_SetField(sbZfFabBm3200BwNPC2QEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nbasegroup") == 0) {
    s->m_nBaseGroup = value;
  } else if (SB_STRCMP(name, "m_ngroups") == 0) {
    s->m_nGroups = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

