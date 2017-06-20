/*
 * $Id: sbZfFabWredParameters.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabWredParameters.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabWredParameters_Pack(sbZfFabWredParameters_t *pFrom,
                           uint8 *pToData,
                           uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_WRED_PARAMETERS_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nTmin */
  (pToData)[5] |= ((pFrom)->m_nTmin) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_nTmin >> 8) &0xFF;

  /* Pack Member: m_nTmax */
  (pToData)[7] |= ((pFrom)->m_nTmax) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nTmax >> 8) &0xFF;

  /* Pack Member: m_nTecn */
  (pToData)[1] |= ((pFrom)->m_nTecn) & 0xFF;
  (pToData)[0] |= ((pFrom)->m_nTecn >> 8) &0xFF;

  /* Pack Member: m_nScale */
  (pToData)[2] |= ((pFrom)->m_nScale & 0x0f) <<4;

  /* Pack Member: m_nSlope */
  (pToData)[3] |= ((pFrom)->m_nSlope) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nSlope >> 8) & 0x0f;
#else
  int i;
  int size = SB_ZF_FAB_WRED_PARAMETERS_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nTmin */
  (pToData)[6] |= ((pFrom)->m_nTmin) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_nTmin >> 8) &0xFF;

  /* Pack Member: m_nTmax */
  (pToData)[4] |= ((pFrom)->m_nTmax) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nTmax >> 8) &0xFF;

  /* Pack Member: m_nTecn */
  (pToData)[2] |= ((pFrom)->m_nTecn) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_nTecn >> 8) &0xFF;

  /* Pack Member: m_nScale */
  (pToData)[1] |= ((pFrom)->m_nScale & 0x0f) <<4;

  /* Pack Member: m_nSlope */
  (pToData)[0] |= ((pFrom)->m_nSlope) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nSlope >> 8) & 0x0f;
#endif

  return SB_ZF_FAB_WRED_PARAMETERS_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabWredParameters_Unpack(sbZfFabWredParameters_t *pToStruct,
                             uint8 *pFromData,
                             uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nTmin */
  (pToStruct)->m_nTmin =  (uint32)  (pFromData)[5] ;
  (pToStruct)->m_nTmin |=  (uint32)  (pFromData)[4] << 8;

  /* Unpack Member: m_nTmax */
  (pToStruct)->m_nTmax =  (uint32)  (pFromData)[7] ;
  (pToStruct)->m_nTmax |=  (uint32)  (pFromData)[6] << 8;

  /* Unpack Member: m_nTecn */
  (pToStruct)->m_nTecn =  (uint32)  (pFromData)[1] ;
  (pToStruct)->m_nTecn |=  (uint32)  (pFromData)[0] << 8;

  /* Unpack Member: m_nScale */
  (pToStruct)->m_nScale =  (uint32)  ((pFromData)[2] >> 4) & 0x0f;

  /* Unpack Member: m_nSlope */
  (pToStruct)->m_nSlope =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nSlope |=  (uint32)  ((pFromData)[2] & 0x0f) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nTmin */
  (pToStruct)->m_nTmin =  (uint32)  (pFromData)[6] ;
  (pToStruct)->m_nTmin |=  (uint32)  (pFromData)[7] << 8;

  /* Unpack Member: m_nTmax */
  (pToStruct)->m_nTmax =  (uint32)  (pFromData)[4] ;
  (pToStruct)->m_nTmax |=  (uint32)  (pFromData)[5] << 8;

  /* Unpack Member: m_nTecn */
  (pToStruct)->m_nTecn =  (uint32)  (pFromData)[2] ;
  (pToStruct)->m_nTecn |=  (uint32)  (pFromData)[3] << 8;

  /* Unpack Member: m_nScale */
  (pToStruct)->m_nScale =  (uint32)  ((pFromData)[1] >> 4) & 0x0f;

  /* Unpack Member: m_nSlope */
  (pToStruct)->m_nSlope =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nSlope |=  (uint32)  ((pFromData)[1] & 0x0f) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabWredParameters_InitInstance(sbZfFabWredParameters_t *pFrame) {

  pFrame->m_nTmin =  (unsigned int)  0;
  pFrame->m_nTmax =  (unsigned int)  0;
  pFrame->m_nTecn =  (unsigned int)  0;
  pFrame->m_nScale =  (unsigned int)  0;
  pFrame->m_nSlope =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabWredParameters.c,v 1.3 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabWredParameters.hx"



/* Print members in struct */
void
sbZfFabWredParameters_Print(sbZfFabWredParameters_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabWredParameters:: tmin=0x%04x"), (unsigned int)  pFromStruct->m_nTmin));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" tmax=0x%04x"), (unsigned int)  pFromStruct->m_nTmax));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" tecn=0x%04x"), (unsigned int)  pFromStruct->m_nTecn));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" scale=0x%01x"), (unsigned int)  pFromStruct->m_nScale));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabWredParameters:: slope=0x%03x"), (unsigned int)  pFromStruct->m_nSlope));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabWredParameters_SPrint(sbZfFabWredParameters_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabWredParameters:: tmin=0x%04x", (unsigned int)  pFromStruct->m_nTmin);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," tmax=0x%04x", (unsigned int)  pFromStruct->m_nTmax);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," tecn=0x%04x", (unsigned int)  pFromStruct->m_nTecn);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," scale=0x%01x", (unsigned int)  pFromStruct->m_nScale);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabWredParameters:: slope=0x%03x", (unsigned int)  pFromStruct->m_nSlope);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabWredParameters_Validate(sbZfFabWredParameters_t *pZf) {

  if (pZf->m_nTmin > 0xffff) return 0;
  if (pZf->m_nTmax > 0xffff) return 0;
  if (pZf->m_nTecn > 0xffff) return 0;
  if (pZf->m_nScale > 0xf) return 0;
  if (pZf->m_nSlope > 0xfff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabWredParameters_SetField(sbZfFabWredParameters_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ntmin") == 0) {
    s->m_nTmin = value;
  } else if (SB_STRCMP(name, "m_ntmax") == 0) {
    s->m_nTmax = value;
  } else if (SB_STRCMP(name, "m_ntecn") == 0) {
    s->m_nTecn = value;
  } else if (SB_STRCMP(name, "m_nscale") == 0) {
    s->m_nScale = value;
  } else if (SB_STRCMP(name, "m_nslope") == 0) {
    s->m_nSlope = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

