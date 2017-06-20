/*
 * $Id: sbZfFabBm3200BwWstEntry.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200BwWstEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200BwWstEntry_Pack(sbZfFabBm3200BwWstEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_WST_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nState */
  (pToData)[3] |= ((pFrom)->m_nState) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nState >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nState >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nState >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_WST_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nState */
  (pToData)[0] |= ((pFrom)->m_nState) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nState >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nState >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nState >> 24) &0xFF;
#endif

  return SB_ZF_FAB_BM3200_WST_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwWstEntry_Unpack(sbZfFabBm3200BwWstEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nState */
  (pToStruct)->m_nState =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nState |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nState |=  (uint32)  (pFromData)[1] << 16;
  (pToStruct)->m_nState |=  (uint32)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nState */
  (pToStruct)->m_nState =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nState |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nState |=  (uint32)  (pFromData)[2] << 16;
  (pToStruct)->m_nState |=  (uint32)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwWstEntry_InitInstance(sbZfFabBm3200BwWstEntry_t *pFrame) {

  pFrame->m_nState =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwWstEntry.c,v 1.4 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200BwWstEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwWstEntry_Print(sbZfFabBm3200BwWstEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwWstEntry:: state=0x%08x"), (unsigned int)  pFromStruct->m_nState));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200BwWstEntry_SPrint(sbZfFabBm3200BwWstEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwWstEntry:: state=0x%08x", (unsigned int)  pFromStruct->m_nState);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwWstEntry_Validate(sbZfFabBm3200BwWstEntry_t *pZf) {

  /* pZf->m_nState implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwWstEntry_SetField(sbZfFabBm3200BwWstEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nstate") == 0) {
    s->m_nState = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

