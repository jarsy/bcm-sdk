/*
 * $Id: sbZfFabBm3200NmEmapEntry.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200NmEmapEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200NmEmapEntry_Pack(sbZfFabBm3200NmEmapEntry_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_EMAP_ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nEmapx */
  (pToData)[19] |= ((pFrom)->m_nEmapx & 0x0f);

  /* Pack Member: m_nEmapd */
  (pToData)[15] |= ((pFrom)->m_nEmapd) & 0xFF;
  (pToData)[14] |= ((pFrom)->m_nEmapd >> 8) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nEmapd >> 16) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nEmapd >> 24) &0xFF;

  /* Pack Member: m_nEmapc */
  (pToData)[11] |= ((pFrom)->m_nEmapc) & 0xFF;
  (pToData)[10] |= ((pFrom)->m_nEmapc >> 8) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nEmapc >> 16) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nEmapc >> 24) &0xFF;

  /* Pack Member: m_nEmapb */
  (pToData)[7] |= ((pFrom)->m_nEmapb) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nEmapb >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nEmapb >> 16) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nEmapb >> 24) &0xFF;

  /* Pack Member: m_nEmapa */
  (pToData)[3] |= ((pFrom)->m_nEmapa) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nEmapa >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nEmapa >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nEmapa >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_EMAP_ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nEmapx */
  (pToData)[16] |= ((pFrom)->m_nEmapx & 0x0f);

  /* Pack Member: m_nEmapd */
  (pToData)[12] |= ((pFrom)->m_nEmapd) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_nEmapd >> 8) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nEmapd >> 16) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nEmapd >> 24) &0xFF;

  /* Pack Member: m_nEmapc */
  (pToData)[8] |= ((pFrom)->m_nEmapc) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_nEmapc >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nEmapc >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nEmapc >> 24) &0xFF;

  /* Pack Member: m_nEmapb */
  (pToData)[4] |= ((pFrom)->m_nEmapb) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nEmapb >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nEmapb >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nEmapb >> 24) &0xFF;

  /* Pack Member: m_nEmapa */
  (pToData)[0] |= ((pFrom)->m_nEmapa) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nEmapa >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nEmapa >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nEmapa >> 24) &0xFF;
#endif

  return SB_ZF_FAB_BM3200_EMAP_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200NmEmapEntry_Unpack(sbZfFabBm3200NmEmapEntry_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nEmapx */
  (pToStruct)->m_nEmapx =  (uint32)  ((pFromData)[19] ) & 0x0f;

  /* Unpack Member: m_nEmapd */
  (pToStruct)->m_nEmapd =  (uint32)  (pFromData)[15] ;
  (pToStruct)->m_nEmapd |=  (uint32)  (pFromData)[14] << 8;
  (pToStruct)->m_nEmapd |=  (uint32)  (pFromData)[13] << 16;
  (pToStruct)->m_nEmapd |=  (uint32)  (pFromData)[12] << 24;

  /* Unpack Member: m_nEmapc */
  (pToStruct)->m_nEmapc =  (uint32)  (pFromData)[11] ;
  (pToStruct)->m_nEmapc |=  (uint32)  (pFromData)[10] << 8;
  (pToStruct)->m_nEmapc |=  (uint32)  (pFromData)[9] << 16;
  (pToStruct)->m_nEmapc |=  (uint32)  (pFromData)[8] << 24;

  /* Unpack Member: m_nEmapb */
  (pToStruct)->m_nEmapb =  (uint32)  (pFromData)[7] ;
  (pToStruct)->m_nEmapb |=  (uint32)  (pFromData)[6] << 8;
  (pToStruct)->m_nEmapb |=  (uint32)  (pFromData)[5] << 16;
  (pToStruct)->m_nEmapb |=  (uint32)  (pFromData)[4] << 24;

  /* Unpack Member: m_nEmapa */
  (pToStruct)->m_nEmapa =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nEmapa |=  (uint32)  (pFromData)[2] << 8;
  (pToStruct)->m_nEmapa |=  (uint32)  (pFromData)[1] << 16;
  (pToStruct)->m_nEmapa |=  (uint32)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nEmapx */
  (pToStruct)->m_nEmapx =  (uint32)  ((pFromData)[16] ) & 0x0f;

  /* Unpack Member: m_nEmapd */
  (pToStruct)->m_nEmapd =  (uint32)  (pFromData)[12] ;
  (pToStruct)->m_nEmapd |=  (uint32)  (pFromData)[13] << 8;
  (pToStruct)->m_nEmapd |=  (uint32)  (pFromData)[14] << 16;
  (pToStruct)->m_nEmapd |=  (uint32)  (pFromData)[15] << 24;

  /* Unpack Member: m_nEmapc */
  (pToStruct)->m_nEmapc =  (uint32)  (pFromData)[8] ;
  (pToStruct)->m_nEmapc |=  (uint32)  (pFromData)[9] << 8;
  (pToStruct)->m_nEmapc |=  (uint32)  (pFromData)[10] << 16;
  (pToStruct)->m_nEmapc |=  (uint32)  (pFromData)[11] << 24;

  /* Unpack Member: m_nEmapb */
  (pToStruct)->m_nEmapb =  (uint32)  (pFromData)[4] ;
  (pToStruct)->m_nEmapb |=  (uint32)  (pFromData)[5] << 8;
  (pToStruct)->m_nEmapb |=  (uint32)  (pFromData)[6] << 16;
  (pToStruct)->m_nEmapb |=  (uint32)  (pFromData)[7] << 24;

  /* Unpack Member: m_nEmapa */
  (pToStruct)->m_nEmapa =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nEmapa |=  (uint32)  (pFromData)[1] << 8;
  (pToStruct)->m_nEmapa |=  (uint32)  (pFromData)[2] << 16;
  (pToStruct)->m_nEmapa |=  (uint32)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200NmEmapEntry_InitInstance(sbZfFabBm3200NmEmapEntry_t *pFrame) {

  pFrame->m_nEmapx =  (unsigned int)  0;
  pFrame->m_nEmapd =  (unsigned int)  0;
  pFrame->m_nEmapc =  (unsigned int)  0;
  pFrame->m_nEmapb =  (unsigned int)  0;
  pFrame->m_nEmapa =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200NmEmapEntry.c,v 1.4 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200NmEmapEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200NmEmapEntry_Print(sbZfFabBm3200NmEmapEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200NmEmapEntry:: emapx=0x%01x"), (unsigned int)  pFromStruct->m_nEmapx));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" emapd=0x%08x"), (unsigned int)  pFromStruct->m_nEmapd));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" emapc=0x%08x"), (unsigned int)  pFromStruct->m_nEmapc));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200NmEmapEntry:: emapb=0x%08x"), (unsigned int)  pFromStruct->m_nEmapb));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" emapa=0x%08x"), (unsigned int)  pFromStruct->m_nEmapa));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200NmEmapEntry_SPrint(sbZfFabBm3200NmEmapEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200NmEmapEntry:: emapx=0x%01x", (unsigned int)  pFromStruct->m_nEmapx);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," emapd=0x%08x", (unsigned int)  pFromStruct->m_nEmapd);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," emapc=0x%08x", (unsigned int)  pFromStruct->m_nEmapc);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200NmEmapEntry:: emapb=0x%08x", (unsigned int)  pFromStruct->m_nEmapb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," emapa=0x%08x", (unsigned int)  pFromStruct->m_nEmapa);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200NmEmapEntry_Validate(sbZfFabBm3200NmEmapEntry_t *pZf) {

  if (pZf->m_nEmapx > 0xf) return 0;
  /* pZf->m_nEmapd implicitly masked by data type */
  /* pZf->m_nEmapc implicitly masked by data type */
  /* pZf->m_nEmapb implicitly masked by data type */
  /* pZf->m_nEmapa implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200NmEmapEntry_SetField(sbZfFabBm3200NmEmapEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nemapx") == 0) {
    s->m_nEmapx = value;
  } else if (SB_STRCMP(name, "m_nemapd") == 0) {
    s->m_nEmapd = value;
  } else if (SB_STRCMP(name, "m_nemapc") == 0) {
    s->m_nEmapc = value;
  } else if (SB_STRCMP(name, "m_nemapb") == 0) {
    s->m_nEmapb = value;
  } else if (SB_STRCMP(name, "m_nemapa") == 0) {
    s->m_nEmapa = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

