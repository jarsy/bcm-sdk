/*
 * $Id: sbZfHwQe2000QsPriLutEntry.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfHwQe2000QsPriLutEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfHwQe2000QsPriLutEntry_Pack(sbZfHwQe2000QsPriLutEntry_t *pFrom,
                               uint8 *pToData,
                               uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_HW_QE2000_QS_PRI_LUT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[2] |= ((pFrom)->m_nReserved) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nReserved >> 8) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nReserved >> 16) &0xFF;

  /* Pack Member: m_nCPri */
  (pToData)[3] |= ((pFrom)->m_nCPri & 0x0f) <<4;

  /* Pack Member: m_nNPri */
  (pToData)[3] |= ((pFrom)->m_nNPri & 0x0f);
#else
  int i;
  int size = SB_ZF_HW_QE2000_QS_PRI_LUT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[1] |= ((pFrom)->m_nReserved) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nReserved >> 8) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nReserved >> 16) &0xFF;

  /* Pack Member: m_nCPri */
  (pToData)[0] |= ((pFrom)->m_nCPri & 0x0f) <<4;

  /* Pack Member: m_nNPri */
  (pToData)[0] |= ((pFrom)->m_nNPri & 0x0f);
#endif

  return SB_ZF_HW_QE2000_QS_PRI_LUT_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfHwQe2000QsPriLutEntry_Unpack(sbZfHwQe2000QsPriLutEntry_t *pToStruct,
                                 uint8 *pFromData,
                                 uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  (pFromData)[2] ;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[0] << 16;

  /* Unpack Member: m_nCPri */
  (pToStruct)->m_nCPri =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_nNPri */
  (pToStruct)->m_nNPri =  (uint32)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  (pFromData)[1] ;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[3] << 16;

  /* Unpack Member: m_nCPri */
  (pToStruct)->m_nCPri =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_nNPri */
  (pToStruct)->m_nNPri =  (uint32)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfHwQe2000QsPriLutEntry_InitInstance(sbZfHwQe2000QsPriLutEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nCPri =  (unsigned int)  0;
  pFrame->m_nNPri =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfHwQe2000QsPriLutEntry.c,v 1.3 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfHwQe2000QsPriLutEntry.hx"



/* Print members in struct */
void
sbZfHwQe2000QsPriLutEntry_Print(sbZfHwQe2000QsPriLutEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("HwQe2000QsPriLutEntry:: res=0x%06x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" cpri=0x%01x"), (unsigned int)  pFromStruct->m_nCPri));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" npri=0x%01x"), (unsigned int)  pFromStruct->m_nNPri));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfHwQe2000QsPriLutEntry_SPrint(sbZfHwQe2000QsPriLutEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"HwQe2000QsPriLutEntry:: res=0x%06x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," cpri=0x%01x", (unsigned int)  pFromStruct->m_nCPri);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," npri=0x%01x", (unsigned int)  pFromStruct->m_nNPri);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfHwQe2000QsPriLutEntry_Validate(sbZfHwQe2000QsPriLutEntry_t *pZf) {

  if (pZf->m_nReserved > 0xffffff) return 0;
  if (pZf->m_nCPri > 0xf) return 0;
  if (pZf->m_nNPri > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfHwQe2000QsPriLutEntry_SetField(sbZfHwQe2000QsPriLutEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_ncpri") == 0) {
    s->m_nCPri = value;
  } else if (SB_STRCMP(name, "m_nnpri") == 0) {
    s->m_nNPri = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

