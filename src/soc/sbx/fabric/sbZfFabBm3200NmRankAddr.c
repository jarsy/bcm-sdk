/*
 * $Id: sbZfFabBm3200NmRankAddr.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200NmRankAddr.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200NmRankAddr_Pack(sbZfFabBm3200NmRankAddr_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_NMRANKADDR_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_bSelIngress */
  (pToData)[3] |= ((pFrom)->m_bSelIngress & 0x01) <<6;

  /* Pack Member: m_nAddr */
  (pToData)[3] |= ((pFrom)->m_nAddr & 0x3f);
#else
  int i;
  int size = SB_ZF_FAB_BM3200_NMRANKADDR_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_bSelIngress */
  (pToData)[0] |= ((pFrom)->m_bSelIngress & 0x01) <<6;

  /* Pack Member: m_nAddr */
  (pToData)[0] |= ((pFrom)->m_nAddr & 0x3f);
#endif

  return SB_ZF_FAB_BM3200_NMRANKADDR_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200NmRankAddr_Unpack(sbZfFabBm3200NmRankAddr_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_bSelIngress */
  (pToStruct)->m_bSelIngress =  (uint32)  ((pFromData)[3] >> 6) & 0x01;

  /* Unpack Member: m_nAddr */
  (pToStruct)->m_nAddr =  (uint32)  ((pFromData)[3] ) & 0x3f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_bSelIngress */
  (pToStruct)->m_bSelIngress =  (uint32)  ((pFromData)[0] >> 6) & 0x01;

  /* Unpack Member: m_nAddr */
  (pToStruct)->m_nAddr =  (uint32)  ((pFromData)[0] ) & 0x3f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200NmRankAddr_InitInstance(sbZfFabBm3200NmRankAddr_t *pFrame) {

  pFrame->m_bSelIngress =  (unsigned int)  0;
  pFrame->m_nAddr =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200NmRankAddr.c,v 1.4 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200NmRankAddr.hx"



/* Print members in struct */
void
sbZfFabBm3200NmRankAddr_Print(sbZfFabBm3200NmRankAddr_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200NmRankAddr:: sel_ingress=0x%01x"), (unsigned int)  pFromStruct->m_bSelIngress));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" addr=0x%02x"), (unsigned int)  pFromStruct->m_nAddr));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200NmRankAddr_SPrint(sbZfFabBm3200NmRankAddr_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200NmRankAddr:: sel_ingress=0x%01x", (unsigned int)  pFromStruct->m_bSelIngress);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," addr=0x%02x", (unsigned int)  pFromStruct->m_nAddr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200NmRankAddr_Validate(sbZfFabBm3200NmRankAddr_t *pZf) {

  if (pZf->m_bSelIngress > 0x1) return 0;
  if (pZf->m_nAddr > 0x3f) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200NmRankAddr_SetField(sbZfFabBm3200NmRankAddr_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "selingress") == 0) {
    s->m_bSelIngress = value;
  } else if (SB_STRCMP(name, "m_naddr") == 0) {
    s->m_nAddr = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

