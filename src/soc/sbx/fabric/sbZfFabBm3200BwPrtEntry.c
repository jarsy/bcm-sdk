/*
 * $Id: sbZfFabBm3200BwPrtEntry.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200BwPrtEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200BwPrtEntry_Pack(sbZfFabBm3200BwPrtEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_PRT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nSpGroups */
  (pToData)[5] |= ((pFrom)->m_nSpGroups & 0x07) <<5;
  (pToData)[4] |= ((pFrom)->m_nSpGroups >> 3) & 0x03;

  /* Pack Member: m_nGroups */
  (pToData)[5] |= ((pFrom)->m_nGroups & 0x1f);

  /* Pack Member: m_nGroup */
  (pToData)[7] |= ((pFrom)->m_nGroup) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nGroup >> 8) &0xFF;

  /* Pack Member: m_nLineRate */
  (pToData)[3] |= ((pFrom)->m_nLineRate) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nLineRate >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nLineRate >> 16) & 0x3f;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_PRT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nSpGroups */
  (pToData)[6] |= ((pFrom)->m_nSpGroups & 0x07) <<5;
  (pToData)[7] |= ((pFrom)->m_nSpGroups >> 3) & 0x03;

  /* Pack Member: m_nGroups */
  (pToData)[6] |= ((pFrom)->m_nGroups & 0x1f);

  /* Pack Member: m_nGroup */
  (pToData)[4] |= ((pFrom)->m_nGroup) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nGroup >> 8) &0xFF;

  /* Pack Member: m_nLineRate */
  (pToData)[0] |= ((pFrom)->m_nLineRate) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nLineRate >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nLineRate >> 16) & 0x3f;
#endif

  return SB_ZF_FAB_BM3200_PRT_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwPrtEntry_Unpack(sbZfFabBm3200BwPrtEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nSpGroups */
  (pToStruct)->m_nSpGroups =  (uint32)  ((pFromData)[5] >> 5) & 0x07;
  (pToStruct)->m_nSpGroups |=  (uint32)  ((pFromData)[4] & 0x03) << 3;

  /* Unpack Member: m_nGroups */
  (pToStruct)->m_nGroups =  (uint32)  ((pFromData)[5] ) & 0x1f;

  /* Unpack Member: m_nGroup */
  (pToStruct)->m_nGroup =  (uint32)  (pFromData)[7] ;
  (pToStruct)->m_nGroup |=  (uint32)  (pFromData)[6] << 8;

  /* Unpack Member: m_nLineRate */
  (pToStruct)->m_nLineRate =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nLineRate |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nLineRate |=  (uint32)  ((pFromData)[1] & 0x3f) << 16;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nSpGroups */
  (pToStruct)->m_nSpGroups =  (uint32)  ((pFromData)[6] >> 5) & 0x07;
  (pToStruct)->m_nSpGroups |=  (uint32)  ((pFromData)[7] & 0x03) << 3;

  /* Unpack Member: m_nGroups */
  (pToStruct)->m_nGroups =  (uint32)  ((pFromData)[6] ) & 0x1f;

  /* Unpack Member: m_nGroup */
  (pToStruct)->m_nGroup =  (uint32)  (pFromData)[4] ;
  (pToStruct)->m_nGroup |=  (uint32)  (pFromData)[5] << 8;

  /* Unpack Member: m_nLineRate */
  (pToStruct)->m_nLineRate =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nLineRate |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nLineRate |=  (uint32)  ((pFromData)[2] & 0x3f) << 16;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwPrtEntry_InitInstance(sbZfFabBm3200BwPrtEntry_t *pFrame) {

  pFrame->m_nSpGroups =  (unsigned int)  0;
  pFrame->m_nGroups =  (unsigned int)  0;
  pFrame->m_nGroup =  (unsigned int)  0;
  pFrame->m_nLineRate =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwPrtEntry.c,v 1.4 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200BwPrtEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwPrtEntry_Print(sbZfFabBm3200BwPrtEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwPrtEntry:: spgroups=0x%02x"), (unsigned int)  pFromStruct->m_nSpGroups));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" groups=0x%02x"), (unsigned int)  pFromStruct->m_nGroups));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" group=0x%04x"), (unsigned int)  pFromStruct->m_nGroup));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwPrtEntry:: linerate=0x%06x"), (unsigned int)  pFromStruct->m_nLineRate));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200BwPrtEntry_SPrint(sbZfFabBm3200BwPrtEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwPrtEntry:: spgroups=0x%02x", (unsigned int)  pFromStruct->m_nSpGroups);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," groups=0x%02x", (unsigned int)  pFromStruct->m_nGroups);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," group=0x%04x", (unsigned int)  pFromStruct->m_nGroup);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwPrtEntry:: linerate=0x%06x", (unsigned int)  pFromStruct->m_nLineRate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwPrtEntry_Validate(sbZfFabBm3200BwPrtEntry_t *pZf) {

  if (pZf->m_nSpGroups > 0x1f) return 0;
  if (pZf->m_nGroups > 0x1f) return 0;
  if (pZf->m_nGroup > 0xffff) return 0;
  if (pZf->m_nLineRate > 0x3fffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwPrtEntry_SetField(sbZfFabBm3200BwPrtEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nspgroups") == 0) {
    s->m_nSpGroups = value;
  } else if (SB_STRCMP(name, "m_ngroups") == 0) {
    s->m_nGroups = value;
  } else if (SB_STRCMP(name, "m_ngroup") == 0) {
    s->m_nGroup = value;
  } else if (SB_STRCMP(name, "m_nlinerate") == 0) {
    s->m_nLineRate = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

