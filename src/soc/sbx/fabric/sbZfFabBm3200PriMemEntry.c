/*
 * $Id: sbZfFabBm3200PriMemEntry.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200PriMemEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200PriMemEntry_Pack(sbZfFabBm3200PriMemEntry_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_PRI_ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nPrix */
  (pToData)[19] |= ((pFrom)->m_nPrix) & 0xFF;
  (pToData)[18] |= ((pFrom)->m_nPrix >> 8) &0xFF;
  (pToData)[17] |= ((pFrom)->m_nPrix >> 16) & 0x0f;

  /* Pack Member: m_nPrid */
  (pToData)[15] |= ((pFrom)->m_nPrid) & 0xFF;
  (pToData)[14] |= ((pFrom)->m_nPrid >> 8) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nPrid >> 16) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nPrid >> 24) &0xFF;

  /* Pack Member: m_nPric */
  (pToData)[11] |= ((pFrom)->m_nPric) & 0xFF;
  (pToData)[10] |= ((pFrom)->m_nPric >> 8) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nPric >> 16) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nPric >> 24) &0xFF;

  /* Pack Member: m_nPrib */
  (pToData)[7] |= ((pFrom)->m_nPrib) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nPrib >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nPrib >> 16) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nPrib >> 24) &0xFF;

  /* Pack Member: m_nPria */
  (pToData)[3] |= ((pFrom)->m_nPria) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nPria >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nPria >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nPria >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_PRI_ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nPrix */
  (pToData)[16] |= ((pFrom)->m_nPrix) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_nPrix >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->m_nPrix >> 16) & 0x0f;

  /* Pack Member: m_nPrid */
  (pToData)[12] |= ((pFrom)->m_nPrid) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_nPrid >> 8) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nPrid >> 16) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nPrid >> 24) &0xFF;

  /* Pack Member: m_nPric */
  (pToData)[8] |= ((pFrom)->m_nPric) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_nPric >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nPric >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nPric >> 24) &0xFF;

  /* Pack Member: m_nPrib */
  (pToData)[4] |= ((pFrom)->m_nPrib) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nPrib >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nPrib >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nPrib >> 24) &0xFF;

  /* Pack Member: m_nPria */
  (pToData)[0] |= ((pFrom)->m_nPria) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nPria >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nPria >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nPria >> 24) &0xFF;
#endif

  return SB_ZF_FAB_BM3200_PRI_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200PriMemEntry_Unpack(sbZfFabBm3200PriMemEntry_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nPrix */
  (pToStruct)->m_nPrix =  (uint32)  (pFromData)[19] ;
  (pToStruct)->m_nPrix |=  (uint32)  (pFromData)[18] << 8;
  (pToStruct)->m_nPrix |=  (uint32)  ((pFromData)[17] & 0x0f) << 16;

  /* Unpack Member: m_nPrid */
  (pToStruct)->m_nPrid =  (uint32)  (pFromData)[15] ;
  (pToStruct)->m_nPrid |=  (uint32)  (pFromData)[14] << 8;
  (pToStruct)->m_nPrid |=  (uint32)  (pFromData)[13] << 16;
  (pToStruct)->m_nPrid |=  (uint32)  (pFromData)[12] << 24;

  /* Unpack Member: m_nPric */
  (pToStruct)->m_nPric =  (uint32)  (pFromData)[11] ;
  (pToStruct)->m_nPric |=  (uint32)  (pFromData)[10] << 8;
  (pToStruct)->m_nPric |=  (uint32)  (pFromData)[9] << 16;
  (pToStruct)->m_nPric |=  (uint32)  (pFromData)[8] << 24;

  /* Unpack Member: m_nPrib */
  (pToStruct)->m_nPrib =  (uint32)  (pFromData)[7] ;
  (pToStruct)->m_nPrib |=  (uint32)  (pFromData)[6] << 8;
  (pToStruct)->m_nPrib |=  (uint32)  (pFromData)[5] << 16;
  (pToStruct)->m_nPrib |=  (uint32)  (pFromData)[4] << 24;

  /* Unpack Member: m_nPria */
  (pToStruct)->m_nPria =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nPria |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nPria |=  (uint32)  (pFromData)[1] << 16;
  (pToStruct)->m_nPria |=  (uint32)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nPrix */
  (pToStruct)->m_nPrix =  (uint32)  (pFromData)[16] ;
  (pToStruct)->m_nPrix |=  (uint32)  (pFromData)[17] << 8;
  (pToStruct)->m_nPrix |=  (uint32)  ((pFromData)[18] & 0x0f) << 16;

  /* Unpack Member: m_nPrid */
  (pToStruct)->m_nPrid =  (uint32)  (pFromData)[12] ;
  (pToStruct)->m_nPrid |=  (uint32)  (pFromData)[13] << 8;
  (pToStruct)->m_nPrid |=  (uint32)  (pFromData)[14] << 16;
  (pToStruct)->m_nPrid |=  (uint32)  (pFromData)[15] << 24;

  /* Unpack Member: m_nPric */
  (pToStruct)->m_nPric =  (uint32)  (pFromData)[8] ;
  (pToStruct)->m_nPric |=  (uint32)  (pFromData)[9] << 8;
  (pToStruct)->m_nPric |=  (uint32)  (pFromData)[10] << 16;
  (pToStruct)->m_nPric |=  (uint32)  (pFromData)[11] << 24;

  /* Unpack Member: m_nPrib */
  (pToStruct)->m_nPrib =  (uint32)  (pFromData)[4] ;
  (pToStruct)->m_nPrib |=  (uint32)  (pFromData)[5] << 8;
  (pToStruct)->m_nPrib |=  (uint32)  (pFromData)[6] << 16;
  (pToStruct)->m_nPrib |=  (uint32)  (pFromData)[7] << 24;

  /* Unpack Member: m_nPria */
  (pToStruct)->m_nPria =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nPria |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nPria |=  (uint32)  (pFromData)[2] << 16;
  (pToStruct)->m_nPria |=  (uint32)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200PriMemEntry_InitInstance(sbZfFabBm3200PriMemEntry_t *pFrame) {

  pFrame->m_nPrix =  (unsigned int)  0;
  pFrame->m_nPrid =  (unsigned int)  0;
  pFrame->m_nPric =  (unsigned int)  0;
  pFrame->m_nPrib =  (unsigned int)  0;
  pFrame->m_nPria =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200PriMemEntry.c,v 1.4 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200PriMemEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200PriMemEntry_Print(sbZfFabBm3200PriMemEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200PriMemEntry:: prix=0x%05x"), (unsigned int)  pFromStruct->m_nPrix));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" prid=0x%08x"), (unsigned int)  pFromStruct->m_nPrid));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pric=0x%08x"), (unsigned int)  pFromStruct->m_nPric));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200PriMemEntry:: prib=0x%08x"), (unsigned int)  pFromStruct->m_nPrib));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" pria=0x%08x"), (unsigned int)  pFromStruct->m_nPria));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200PriMemEntry_SPrint(sbZfFabBm3200PriMemEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200PriMemEntry:: prix=0x%05x", (unsigned int)  pFromStruct->m_nPrix);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," prid=0x%08x", (unsigned int)  pFromStruct->m_nPrid);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pric=0x%08x", (unsigned int)  pFromStruct->m_nPric);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200PriMemEntry:: prib=0x%08x", (unsigned int)  pFromStruct->m_nPrib);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pria=0x%08x", (unsigned int)  pFromStruct->m_nPria);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200PriMemEntry_Validate(sbZfFabBm3200PriMemEntry_t *pZf) {

  if (pZf->m_nPrix > 0xfffff) return 0;
  /* pZf->m_nPrid implicitly masked by data type */
  /* pZf->m_nPric implicitly masked by data type */
  /* pZf->m_nPrib implicitly masked by data type */
  /* pZf->m_nPria implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200PriMemEntry_SetField(sbZfFabBm3200PriMemEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nprix") == 0) {
    s->m_nPrix = value;
  } else if (SB_STRCMP(name, "m_nprid") == 0) {
    s->m_nPrid = value;
  } else if (SB_STRCMP(name, "m_npric") == 0) {
    s->m_nPric = value;
  } else if (SB_STRCMP(name, "m_nprib") == 0) {
    s->m_nPrib = value;
  } else if (SB_STRCMP(name, "m_npria") == 0) {
    s->m_nPria = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

