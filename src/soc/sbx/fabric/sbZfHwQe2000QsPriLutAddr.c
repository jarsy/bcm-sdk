/*
 * $Id: sbZfHwQe2000QsPriLutAddr.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfHwQe2000QsPriLutAddr.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfHwQe2000QsPriLutAddr_Pack(sbZfHwQe2000QsPriLutAddr_t *pFrom,
                              uint8 *pToData,
                              uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[2] |= ((pFrom)->m_nReserved & 0x07) <<5;
  (pToData)[1] |= ((pFrom)->m_nReserved >> 3) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nReserved >> 11) &0xFF;

  /* Pack Member: m_bShaped */
  (pToData)[2] |= ((pFrom)->m_bShaped & 0x01) <<4;

  /* Pack Member: m_nDepth */
  (pToData)[2] |= ((pFrom)->m_nDepth & 0x07) <<1;

  /* Pack Member: m_bAnemicAged */
  (pToData)[2] |= ((pFrom)->m_bAnemicAged & 0x01);

  /* Pack Member: m_nQType */
  (pToData)[3] |= ((pFrom)->m_nQType & 0x0f) <<4;

  /* Pack Member: m_bEfAged */
  (pToData)[3] |= ((pFrom)->m_bEfAged & 0x01) <<3;

  /* Pack Member: m_nCreditLevel */
  (pToData)[3] |= ((pFrom)->m_nCreditLevel & 0x01) <<2;

  /* Pack Member: m_bHoldTs */
  (pToData)[3] |= ((pFrom)->m_bHoldTs & 0x01) <<1;

  /* Pack Member: m_nPktLen */
  (pToData)[3] |= ((pFrom)->m_nPktLen & 0x01);
#else
  int i;
  int size = SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[1] |= ((pFrom)->m_nReserved & 0x07) <<5;
  (pToData)[2] |= ((pFrom)->m_nReserved >> 3) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nReserved >> 11) &0xFF;

  /* Pack Member: m_bShaped */
  (pToData)[1] |= ((pFrom)->m_bShaped & 0x01) <<4;

  /* Pack Member: m_nDepth */
  (pToData)[1] |= ((pFrom)->m_nDepth & 0x07) <<1;

  /* Pack Member: m_bAnemicAged */
  (pToData)[1] |= ((pFrom)->m_bAnemicAged & 0x01);

  /* Pack Member: m_nQType */
  (pToData)[0] |= ((pFrom)->m_nQType & 0x0f) <<4;

  /* Pack Member: m_bEfAged */
  (pToData)[0] |= ((pFrom)->m_bEfAged & 0x01) <<3;

  /* Pack Member: m_nCreditLevel */
  (pToData)[0] |= ((pFrom)->m_nCreditLevel & 0x01) <<2;

  /* Pack Member: m_bHoldTs */
  (pToData)[0] |= ((pFrom)->m_bHoldTs & 0x01) <<1;

  /* Pack Member: m_nPktLen */
  (pToData)[0] |= ((pFrom)->m_nPktLen & 0x01);
#endif

  return SB_ZF_HW_QE2000_QS_PRI_LUT_ADDR_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfHwQe2000QsPriLutAddr_Unpack(sbZfHwQe2000QsPriLutAddr_t *pToStruct,
                                uint8 *pFromData,
                                uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[2] >> 5) & 0x07;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[1] << 3;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[0] << 11;

  /* Unpack Member: m_bShaped */
  (pToStruct)->m_bShaped =  (uint8)  ((pFromData)[2] >> 4) & 0x01;

  /* Unpack Member: m_nDepth */
  (pToStruct)->m_nDepth =  (uint32)  ((pFromData)[2] >> 1) & 0x07;

  /* Unpack Member: m_bAnemicAged */
  (pToStruct)->m_bAnemicAged =  (uint8)  ((pFromData)[2] ) & 0x01;

  /* Unpack Member: m_nQType */
  (pToStruct)->m_nQType =  (uint32)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_bEfAged */
  (pToStruct)->m_bEfAged =  (uint8)  ((pFromData)[3] >> 3) & 0x01;

  /* Unpack Member: m_nCreditLevel */
  (pToStruct)->m_nCreditLevel =  (uint32)  ((pFromData)[3] >> 2) & 0x01;

  /* Unpack Member: m_bHoldTs */
  (pToStruct)->m_bHoldTs =  (uint8)  ((pFromData)[3] >> 1) & 0x01;

  /* Unpack Member: m_nPktLen */
  (pToStruct)->m_nPktLen =  (uint32)  ((pFromData)[3] ) & 0x01;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32)  ((pFromData)[1] >> 5) & 0x07;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[2] << 3;
  (pToStruct)->m_nReserved |=  (uint32)  (pFromData)[3] << 11;

  /* Unpack Member: m_bShaped */
  (pToStruct)->m_bShaped =  (uint8)  ((pFromData)[1] >> 4) & 0x01;

  /* Unpack Member: m_nDepth */
  (pToStruct)->m_nDepth =  (uint32)  ((pFromData)[1] >> 1) & 0x07;

  /* Unpack Member: m_bAnemicAged */
  (pToStruct)->m_bAnemicAged =  (uint8)  ((pFromData)[1] ) & 0x01;

  /* Unpack Member: m_nQType */
  (pToStruct)->m_nQType =  (uint32)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_bEfAged */
  (pToStruct)->m_bEfAged =  (uint8)  ((pFromData)[0] >> 3) & 0x01;

  /* Unpack Member: m_nCreditLevel */
  (pToStruct)->m_nCreditLevel =  (uint32)  ((pFromData)[0] >> 2) & 0x01;

  /* Unpack Member: m_bHoldTs */
  (pToStruct)->m_bHoldTs =  (uint8)  ((pFromData)[0] >> 1) & 0x01;

  /* Unpack Member: m_nPktLen */
  (pToStruct)->m_nPktLen =  (uint32)  ((pFromData)[0] ) & 0x01;
#endif

}



/* initialize an instance of this zframe */
void
sbZfHwQe2000QsPriLutAddr_InitInstance(sbZfHwQe2000QsPriLutAddr_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_bShaped =  (unsigned int)  0;
  pFrame->m_nDepth =  (unsigned int)  0;
  pFrame->m_bAnemicAged =  (unsigned int)  0;
  pFrame->m_nQType =  (unsigned int)  0;
  pFrame->m_bEfAged =  (unsigned int)  0;
  pFrame->m_nCreditLevel =  (unsigned int)  0;
  pFrame->m_bHoldTs =  (unsigned int)  0;
  pFrame->m_nPktLen =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfHwQe2000QsPriLutAddr.c,v 1.3 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfHwQe2000QsPriLutAddr.hx"



/* Print members in struct */
void
sbZfHwQe2000QsPriLutAddr_Print(sbZfHwQe2000QsPriLutAddr_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("HwQe2000QsPriLutAddr:: res=0x%05x"), (unsigned int)  pFromStruct->m_nReserved));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" shaped=0x%01x"), (unsigned int)  pFromStruct->m_bShaped));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" depth=0x%01x"), (unsigned int)  pFromStruct->m_nDepth));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" anemicaged=0x%01x"), (unsigned int)  pFromStruct->m_bAnemicAged));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("HwQe2000QsPriLutAddr:: qtype=0x%01x"), (unsigned int)  pFromStruct->m_nQType));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" efaged=0x%01x"), (unsigned int)  pFromStruct->m_bEfAged));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" creditlevel=0x%01x"), (unsigned int)  pFromStruct->m_nCreditLevel));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" holdts=0x%01x"), (unsigned int)  pFromStruct->m_bHoldTs));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("HwQe2000QsPriLutAddr:: pktlen=0x%01x"), (unsigned int)  pFromStruct->m_nPktLen));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfHwQe2000QsPriLutAddr_SPrint(sbZfHwQe2000QsPriLutAddr_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"HwQe2000QsPriLutAddr:: res=0x%05x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," shaped=0x%01x", (unsigned int)  pFromStruct->m_bShaped);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," depth=0x%01x", (unsigned int)  pFromStruct->m_nDepth);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," anemicaged=0x%01x", (unsigned int)  pFromStruct->m_bAnemicAged);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"HwQe2000QsPriLutAddr:: qtype=0x%01x", (unsigned int)  pFromStruct->m_nQType);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," efaged=0x%01x", (unsigned int)  pFromStruct->m_bEfAged);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," creditlevel=0x%01x", (unsigned int)  pFromStruct->m_nCreditLevel);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," holdts=0x%01x", (unsigned int)  pFromStruct->m_bHoldTs);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"HwQe2000QsPriLutAddr:: pktlen=0x%01x", (unsigned int)  pFromStruct->m_nPktLen);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfHwQe2000QsPriLutAddr_Validate(sbZfHwQe2000QsPriLutAddr_t *pZf) {

  if (pZf->m_nReserved > 0x7ffff) return 0;
  if (pZf->m_bShaped > 0x1) return 0;
  if (pZf->m_nDepth > 0x7) return 0;
  if (pZf->m_bAnemicAged > 0x1) return 0;
  if (pZf->m_nQType > 0xf) return 0;
  if (pZf->m_bEfAged > 0x1) return 0;
  if (pZf->m_nCreditLevel > 0x1) return 0;
  if (pZf->m_bHoldTs > 0x1) return 0;
  if (pZf->m_nPktLen > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfHwQe2000QsPriLutAddr_SetField(sbZfHwQe2000QsPriLutAddr_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "shaped") == 0) {
    s->m_bShaped = value;
  } else if (SB_STRCMP(name, "m_ndepth") == 0) {
    s->m_nDepth = value;
  } else if (SB_STRCMP(name, "anemicaged") == 0) {
    s->m_bAnemicAged = value;
  } else if (SB_STRCMP(name, "m_nqtype") == 0) {
    s->m_nQType = value;
  } else if (SB_STRCMP(name, "efaged") == 0) {
    s->m_bEfAged = value;
  } else if (SB_STRCMP(name, "m_ncreditlevel") == 0) {
    s->m_nCreditLevel = value;
  } else if (SB_STRCMP(name, "holdts") == 0) {
    s->m_bHoldTs = value;
  } else if (SB_STRCMP(name, "m_npktlen") == 0) {
    s->m_nPktLen = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

