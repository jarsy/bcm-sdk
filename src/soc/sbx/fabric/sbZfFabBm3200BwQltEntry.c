/*
 * $Id: sbZfFabBm3200BwQltEntry.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200BwQltEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200BwQltEntry_Pack(sbZfFabBm3200BwQltEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_QLT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nQueueLength */
  (pToData)[3] |= ((pFrom)->m_nQueueLength) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nQueueLength >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nQueueLength >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nQueueLength >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_QLT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nQueueLength */
  (pToData)[0] |= ((pFrom)->m_nQueueLength) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nQueueLength >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nQueueLength >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nQueueLength >> 24) &0xFF;
#endif

  return SB_ZF_FAB_BM3200_QLT_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwQltEntry_Unpack(sbZfFabBm3200BwQltEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nQueueLength */
  (pToStruct)->m_nQueueLength =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nQueueLength |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nQueueLength |=  (uint32)  (pFromData)[1] << 16;
  (pToStruct)->m_nQueueLength |=  (uint32)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nQueueLength */
  (pToStruct)->m_nQueueLength =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nQueueLength |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nQueueLength |=  (uint32)  (pFromData)[2] << 16;
  (pToStruct)->m_nQueueLength |=  (uint32)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwQltEntry_InitInstance(sbZfFabBm3200BwQltEntry_t *pFrame) {

  pFrame->m_nQueueLength =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwQltEntry.c,v 1.4 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200BwQltEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwQltEntry_Print(sbZfFabBm3200BwQltEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwQltEntry:: queuelength=0x%08x"), (unsigned int)  pFromStruct->m_nQueueLength));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200BwQltEntry_SPrint(sbZfFabBm3200BwQltEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwQltEntry:: queuelength=0x%08x", (unsigned int)  pFromStruct->m_nQueueLength);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwQltEntry_Validate(sbZfFabBm3200BwQltEntry_t *pZf) {

  /* pZf->m_nQueueLength implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwQltEntry_SetField(sbZfFabBm3200BwQltEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nqueuelength") == 0) {
    s->m_nQueueLength = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

