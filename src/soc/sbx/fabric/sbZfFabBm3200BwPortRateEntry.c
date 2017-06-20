/*
 * $Id: sbZfFabBm3200BwPortRateEntry.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200BwPortRateEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200BwPortRateEntry_Pack(sbZfFabBm3200BwPortRateEntry_t *pFrom,
                                  uint8 *pToData,
                                  uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[0] |= ((pFrom)->m_nReserved & 0x3f) <<2;

  /* Pack Member: m_nSpGroups */
  (pToData)[1] |= ((pFrom)->m_nSpGroups & 0x07) <<5;
  (pToData)[0] |= ((pFrom)->m_nSpGroups >> 3) & 0x03;

  /* Pack Member: m_nGroups */
  (pToData)[1] |= ((pFrom)->m_nGroups & 0x1f);

  /* Pack Member: m_nGroup */
  (pToData)[3] |= ((pFrom)->m_nGroup) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nGroup >> 8) &0xFF;

  /* Pack Member: m_nLineRate */
  (pToData)[7] |= ((pFrom)->m_nLineRate) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nLineRate >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nLineRate >> 16) & 0x7f;

  /* Pack Member: m_nReserved1 */
  (pToData)[5] |= ((pFrom)->m_nReserved1 & 0x01) <<7;
  (pToData)[4] |= ((pFrom)->m_nReserved1 >> 1) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[3] |= ((pFrom)->m_nReserved & 0x3f) <<2;

  /* Pack Member: m_nSpGroups */
  (pToData)[2] |= ((pFrom)->m_nSpGroups & 0x07) <<5;
  (pToData)[3] |= ((pFrom)->m_nSpGroups >> 3) & 0x03;

  /* Pack Member: m_nGroups */
  (pToData)[2] |= ((pFrom)->m_nGroups & 0x1f);

  /* Pack Member: m_nGroup */
  (pToData)[0] |= ((pFrom)->m_nGroup) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nGroup >> 8) &0xFF;

  /* Pack Member: m_nLineRate */
  (pToData)[4] |= ((pFrom)->m_nLineRate) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nLineRate >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nLineRate >> 16) & 0x7f;

  /* Pack Member: m_nReserved1 */
  (pToData)[6] |= ((pFrom)->m_nReserved1 & 0x01) <<7;
  (pToData)[7] |= ((pFrom)->m_nReserved1 >> 1) &0xFF;
#endif

  return SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwPortRateEntry_Unpack(sbZfFabBm3200BwPortRateEntry_t *pToStruct,
                                    uint8 *pFromData,
                                    uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[0] >> 2) & 0x3f;

  /* Unpack Member: m_nSpGroups */
  (pToStruct)->m_nSpGroups =  (uint32)  ((pFromData)[1] >> 5) & 0x07;
  (pToStruct)->m_nSpGroups |=  (uint32)  ((pFromData)[0] & 0x03) << 3;

  /* Unpack Member: m_nGroups */
  (pToStruct)->m_nGroups =  (uint32)  ((pFromData)[1] ) & 0x1f;

  /* Unpack Member: m_nGroup */
  (pToStruct)->m_nGroup =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nGroup |=  (uint32)  (pFromData)[2] << 8;

  /* Unpack Member: m_nLineRate */
  (pToStruct)->m_nLineRate =  (uint32)  (pFromData)[7] ;
  (pToStruct)->m_nLineRate |=  (uint32)  (pFromData)[6] << 8;
  (pToStruct)->m_nLineRate |=  (uint32)  ((pFromData)[5] & 0x7f) << 16;

  /* Unpack Member: m_nReserved1 */
  (pToStruct)->m_nReserved1 =  (uint32)  ((pFromData)[5] >> 7) & 0x01;
  (pToStruct)->m_nReserved1 |=  (uint32)  (pFromData)[4] << 1;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[3] >> 2) & 0x3f;

  /* Unpack Member: m_nSpGroups */
  (pToStruct)->m_nSpGroups =  (uint32)  ((pFromData)[2] >> 5) & 0x07;
  (pToStruct)->m_nSpGroups |=  (uint32)  ((pFromData)[3] & 0x03) << 3;

  /* Unpack Member: m_nGroups */
  (pToStruct)->m_nGroups =  (uint32)  ((pFromData)[2] ) & 0x1f;

  /* Unpack Member: m_nGroup */
  (pToStruct)->m_nGroup =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nGroup |=  (uint32)  (pFromData)[1] << 8;

  /* Unpack Member: m_nLineRate */
  (pToStruct)->m_nLineRate =  (uint32)  (pFromData)[4] ;
  (pToStruct)->m_nLineRate |=  (uint32)  (pFromData)[5] << 8;
  (pToStruct)->m_nLineRate |=  (uint32)  ((pFromData)[6] & 0x7f) << 16;

  /* Unpack Member: m_nReserved1 */
  (pToStruct)->m_nReserved1 =  (uint32)  ((pFromData)[6] >> 7) & 0x01;
  (pToStruct)->m_nReserved1 |=  (uint32)  (pFromData)[7] << 1;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwPortRateEntry_InitInstance(sbZfFabBm3200BwPortRateEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nSpGroups =  (unsigned int)  0;
  pFrame->m_nGroups =  (unsigned int)  0;
  pFrame->m_nGroup =  (unsigned int)  0;
  pFrame->m_nLineRate =  (unsigned int)  0;
  pFrame->m_nReserved1 =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwPortRateEntry.c,v 1.3 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200BwPortRateEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwPortRateEntry_Print(sbZfFabBm3200BwPortRateEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwPortRateEntry:: reserved=0x%02x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" sp_groups=0x%02x"), (unsigned int)  pFromStruct->m_nSpGroups));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" groups=0x%02x"), (unsigned int)  pFromStruct->m_nGroups));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwPortRateEntry:: group=0x%04x"), (unsigned int)  pFromStruct->m_nGroup));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" line_rate=0x%06x"), (unsigned int)  pFromStruct->m_nLineRate));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" reserved1=0x%03x"), (unsigned int)  pFromStruct->m_nReserved1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200BwPortRateEntry_SPrint(sbZfFabBm3200BwPortRateEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwPortRateEntry:: reserved=0x%02x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sp_groups=0x%02x", (unsigned int)  pFromStruct->m_nSpGroups);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," groups=0x%02x", (unsigned int)  pFromStruct->m_nGroups);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwPortRateEntry:: group=0x%04x", (unsigned int)  pFromStruct->m_nGroup);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," line_rate=0x%06x", (unsigned int)  pFromStruct->m_nLineRate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," reserved1=0x%03x", (unsigned int)  pFromStruct->m_nReserved1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwPortRateEntry_Validate(sbZfFabBm3200BwPortRateEntry_t *pZf) {

  if (pZf->m_nReserved > 0x3f) return 0;
  if (pZf->m_nSpGroups > 0x1f) return 0;
  if (pZf->m_nGroups > 0x1f) return 0;
  if (pZf->m_nGroup > 0xffff) return 0;
  if (pZf->m_nLineRate > 0x7fffff) return 0;
  if (pZf->m_nReserved1 > 0x1ff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwPortRateEntry_SetField(sbZfFabBm3200BwPortRateEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nspgroups") == 0) {
    s->m_nSpGroups = value;
  } else if (SB_STRCMP(name, "m_ngroups") == 0) {
    s->m_nGroups = value;
  } else if (SB_STRCMP(name, "m_ngroup") == 0) {
    s->m_nGroup = value;
  } else if (SB_STRCMP(name, "m_nlinerate") == 0) {
    s->m_nLineRate = value;
  } else if (SB_STRCMP(name, "m_nreserved1") == 0) {
    s->m_nReserved1 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

