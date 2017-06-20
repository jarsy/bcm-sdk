/*
 * $Id: sbZfFabBm3200BwWctEntry.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include <shared/bsl.h>

#include "sbTypes.h"
#include "sbZfFabBm3200BwWctEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfFabBm3200BwWctEntry_Pack(sbZfFabBm3200BwWctEntry_t *pFrom,
                             uint8 *pToData,
                             uint32 nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_WCT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved1 */
  (pToData)[31] |= ((pFrom)->m_nReserved1) & 0xFF;
  (pToData)[30] |= ((pFrom)->m_nReserved1 >> 8) &0xFF;
  (pToData)[29] |= ((pFrom)->m_nReserved1 >> 16) &0xFF;
  (pToData)[28] |= ((pFrom)->m_nReserved1 >> 24) &0xFF;

  /* Pack Member: m_nReserved0 */
  (pToData)[27] |= ((pFrom)->m_nReserved0) & 0xFF;
  (pToData)[26] |= ((pFrom)->m_nReserved0 >> 8) &0xFF;
  (pToData)[25] |= ((pFrom)->m_nReserved0 >> 16) &0xFF;
  (pToData)[24] |= ((pFrom)->m_nReserved0 >> 24) &0xFF;

  /* Pack Member: m_nEcn_dp2 */
  (pToData)[21] |= ((pFrom)->m_nEcn_dp2) & 0xFF;
  (pToData)[20] |= ((pFrom)->m_nEcn_dp2 >> 8) &0xFF;

  /* Pack Member: m_nScale_dp2 */
  (pToData)[22] |= ((pFrom)->m_nScale_dp2 & 0x0f) <<4;

  /* Pack Member: m_nSlope_dp2 */
  (pToData)[23] |= ((pFrom)->m_nSlope_dp2) & 0xFF;
  (pToData)[22] |= ((pFrom)->m_nSlope_dp2 >> 8) & 0x0f;

  /* Pack Member: m_nMin_dp2 */
  (pToData)[17] |= ((pFrom)->m_nMin_dp2) & 0xFF;
  (pToData)[16] |= ((pFrom)->m_nMin_dp2 >> 8) &0xFF;

  /* Pack Member: m_nMax_dp2 */
  (pToData)[19] |= ((pFrom)->m_nMax_dp2) & 0xFF;
  (pToData)[18] |= ((pFrom)->m_nMax_dp2 >> 8) &0xFF;

  /* Pack Member: m_nEcn_dp1 */
  (pToData)[13] |= ((pFrom)->m_nEcn_dp1) & 0xFF;
  (pToData)[12] |= ((pFrom)->m_nEcn_dp1 >> 8) &0xFF;

  /* Pack Member: m_nScale_dp1 */
  (pToData)[14] |= ((pFrom)->m_nScale_dp1 & 0x0f) <<4;

  /* Pack Member: m_nSlope_dp1 */
  (pToData)[15] |= ((pFrom)->m_nSlope_dp1) & 0xFF;
  (pToData)[14] |= ((pFrom)->m_nSlope_dp1 >> 8) & 0x0f;

  /* Pack Member: m_nMin_dp1 */
  (pToData)[9] |= ((pFrom)->m_nMin_dp1) & 0xFF;
  (pToData)[8] |= ((pFrom)->m_nMin_dp1 >> 8) &0xFF;

  /* Pack Member: m_nMax_dp1 */
  (pToData)[11] |= ((pFrom)->m_nMax_dp1) & 0xFF;
  (pToData)[10] |= ((pFrom)->m_nMax_dp1 >> 8) &0xFF;

  /* Pack Member: m_nEcn_dp0 */
  (pToData)[5] |= ((pFrom)->m_nEcn_dp0) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_nEcn_dp0 >> 8) &0xFF;

  /* Pack Member: m_nScale_dp0 */
  (pToData)[6] |= ((pFrom)->m_nScale_dp0 & 0x0f) <<4;

  /* Pack Member: m_nSlope_dp0 */
  (pToData)[7] |= ((pFrom)->m_nSlope_dp0) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nSlope_dp0 >> 8) & 0x0f;

  /* Pack Member: m_nMin_dp0 */
  (pToData)[1] |= ((pFrom)->m_nMin_dp0) & 0xFF;
  (pToData)[0] |= ((pFrom)->m_nMin_dp0 >> 8) &0xFF;

  /* Pack Member: m_nMax_dp0 */
  (pToData)[3] |= ((pFrom)->m_nMax_dp0) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nMax_dp0 >> 8) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_WCT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved1 */
  (pToData)[28] |= ((pFrom)->m_nReserved1) & 0xFF;
  (pToData)[29] |= ((pFrom)->m_nReserved1 >> 8) &0xFF;
  (pToData)[30] |= ((pFrom)->m_nReserved1 >> 16) &0xFF;
  (pToData)[31] |= ((pFrom)->m_nReserved1 >> 24) &0xFF;

  /* Pack Member: m_nReserved0 */
  (pToData)[24] |= ((pFrom)->m_nReserved0) & 0xFF;
  (pToData)[25] |= ((pFrom)->m_nReserved0 >> 8) &0xFF;
  (pToData)[26] |= ((pFrom)->m_nReserved0 >> 16) &0xFF;
  (pToData)[27] |= ((pFrom)->m_nReserved0 >> 24) &0xFF;

  /* Pack Member: m_nEcn_dp2 */
  (pToData)[22] |= ((pFrom)->m_nEcn_dp2) & 0xFF;
  (pToData)[23] |= ((pFrom)->m_nEcn_dp2 >> 8) &0xFF;

  /* Pack Member: m_nScale_dp2 */
  (pToData)[21] |= ((pFrom)->m_nScale_dp2 & 0x0f) <<4;

  /* Pack Member: m_nSlope_dp2 */
  (pToData)[20] |= ((pFrom)->m_nSlope_dp2) & 0xFF;
  (pToData)[21] |= ((pFrom)->m_nSlope_dp2 >> 8) & 0x0f;

  /* Pack Member: m_nMin_dp2 */
  (pToData)[18] |= ((pFrom)->m_nMin_dp2) & 0xFF;
  (pToData)[19] |= ((pFrom)->m_nMin_dp2 >> 8) &0xFF;

  /* Pack Member: m_nMax_dp2 */
  (pToData)[16] |= ((pFrom)->m_nMax_dp2) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_nMax_dp2 >> 8) &0xFF;

  /* Pack Member: m_nEcn_dp1 */
  (pToData)[14] |= ((pFrom)->m_nEcn_dp1) & 0xFF;
  (pToData)[15] |= ((pFrom)->m_nEcn_dp1 >> 8) &0xFF;

  /* Pack Member: m_nScale_dp1 */
  (pToData)[13] |= ((pFrom)->m_nScale_dp1 & 0x0f) <<4;

  /* Pack Member: m_nSlope_dp1 */
  (pToData)[12] |= ((pFrom)->m_nSlope_dp1) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_nSlope_dp1 >> 8) & 0x0f;

  /* Pack Member: m_nMin_dp1 */
  (pToData)[10] |= ((pFrom)->m_nMin_dp1) & 0xFF;
  (pToData)[11] |= ((pFrom)->m_nMin_dp1 >> 8) &0xFF;

  /* Pack Member: m_nMax_dp1 */
  (pToData)[8] |= ((pFrom)->m_nMax_dp1) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_nMax_dp1 >> 8) &0xFF;

  /* Pack Member: m_nEcn_dp0 */
  (pToData)[6] |= ((pFrom)->m_nEcn_dp0) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_nEcn_dp0 >> 8) &0xFF;

  /* Pack Member: m_nScale_dp0 */
  (pToData)[5] |= ((pFrom)->m_nScale_dp0 & 0x0f) <<4;

  /* Pack Member: m_nSlope_dp0 */
  (pToData)[4] |= ((pFrom)->m_nSlope_dp0) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nSlope_dp0 >> 8) & 0x0f;

  /* Pack Member: m_nMin_dp0 */
  (pToData)[2] |= ((pFrom)->m_nMin_dp0) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_nMin_dp0 >> 8) &0xFF;

  /* Pack Member: m_nMax_dp0 */
  (pToData)[0] |= ((pFrom)->m_nMax_dp0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nMax_dp0 >> 8) &0xFF;
#endif

  return SB_ZF_FAB_BM3200_WCT_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwWctEntry_Unpack(sbZfFabBm3200BwWctEntry_t *pToStruct,
                               uint8 *pFromData,
                               uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved1 */
  (pToStruct)->m_nReserved1 =  (uint32)  (pFromData)[31] ;
  (pToStruct)->m_nReserved1 |=  (uint32)  (pFromData)[30] << 8;
  (pToStruct)->m_nReserved1 |=  (uint32)  (pFromData)[29] << 16;
  (pToStruct)->m_nReserved1 |=  (uint32)  (pFromData)[28] << 24;

  /* Unpack Member: m_nReserved0 */
  (pToStruct)->m_nReserved0 =  (uint32)  (pFromData)[27] ;
  (pToStruct)->m_nReserved0 |=  (uint32)  (pFromData)[26] << 8;
  (pToStruct)->m_nReserved0 |=  (uint32)  (pFromData)[25] << 16;
  (pToStruct)->m_nReserved0 |=  (uint32)  (pFromData)[24] << 24;

  /* Unpack Member: m_nEcn_dp2 */
  (pToStruct)->m_nEcn_dp2 =  (uint32)  (pFromData)[21] ;
  (pToStruct)->m_nEcn_dp2 |=  (uint32)  (pFromData)[20] << 8;

  /* Unpack Member: m_nScale_dp2 */
  (pToStruct)->m_nScale_dp2 =  (uint32)  ((pFromData)[22] >> 4) & 0x0f;

  /* Unpack Member: m_nSlope_dp2 */
  (pToStruct)->m_nSlope_dp2 =  (uint32)  (pFromData)[23] ;
  (pToStruct)->m_nSlope_dp2 |=  (uint32)  ((pFromData)[22] & 0x0f) << 8;

  /* Unpack Member: m_nMin_dp2 */
  (pToStruct)->m_nMin_dp2 =  (uint32)  (pFromData)[17] ;
  (pToStruct)->m_nMin_dp2 |=  (uint32)  (pFromData)[16] << 8;

  /* Unpack Member: m_nMax_dp2 */
  (pToStruct)->m_nMax_dp2 =  (uint32)  (pFromData)[19] ;
  (pToStruct)->m_nMax_dp2 |=  (uint32)  (pFromData)[18] << 8;

  /* Unpack Member: m_nEcn_dp1 */
  (pToStruct)->m_nEcn_dp1 =  (uint32)  (pFromData)[13] ;
  (pToStruct)->m_nEcn_dp1 |=  (uint32)  (pFromData)[12] << 8;

  /* Unpack Member: m_nScale_dp1 */
  (pToStruct)->m_nScale_dp1 =  (uint32)  ((pFromData)[14] >> 4) & 0x0f;

  /* Unpack Member: m_nSlope_dp1 */
  (pToStruct)->m_nSlope_dp1 =  (uint32)  (pFromData)[15] ;
  (pToStruct)->m_nSlope_dp1 |=  (uint32)  ((pFromData)[14] & 0x0f) << 8;

  /* Unpack Member: m_nMin_dp1 */
  (pToStruct)->m_nMin_dp1 =  (uint32)  (pFromData)[9] ;
  (pToStruct)->m_nMin_dp1 |=  (uint32)  (pFromData)[8] << 8;

  /* Unpack Member: m_nMax_dp1 */
  (pToStruct)->m_nMax_dp1 =  (uint32)  (pFromData)[11] ;
  (pToStruct)->m_nMax_dp1 |=  (uint32)  (pFromData)[10] << 8;

  /* Unpack Member: m_nEcn_dp0 */
  (pToStruct)->m_nEcn_dp0 =  (uint32)  (pFromData)[5] ;
  (pToStruct)->m_nEcn_dp0 |=  (uint32)  (pFromData)[4] << 8;

  /* Unpack Member: m_nScale_dp0 */
  (pToStruct)->m_nScale_dp0 =  (uint32)  ((pFromData)[6] >> 4) & 0x0f;

  /* Unpack Member: m_nSlope_dp0 */
  (pToStruct)->m_nSlope_dp0 =  (uint32)  (pFromData)[7] ;
  (pToStruct)->m_nSlope_dp0 |=  (uint32)  ((pFromData)[6] & 0x0f) << 8;

  /* Unpack Member: m_nMin_dp0 */
  (pToStruct)->m_nMin_dp0 =  (uint32)  (pFromData)[1] ;
  (pToStruct)->m_nMin_dp0 |=  (uint32)  (pFromData)[0] << 8;

  /* Unpack Member: m_nMax_dp0 */
  (pToStruct)->m_nMax_dp0 =  (uint32)  (pFromData)[3] ;
  (pToStruct)->m_nMax_dp0 |=  (uint32)  (pFromData)[2] << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved1 */
  (pToStruct)->m_nReserved1 =  (uint32)  (pFromData)[28] ;
  (pToStruct)->m_nReserved1 |=  (uint32)  (pFromData)[29] << 8;
  (pToStruct)->m_nReserved1 |=  (uint32)  (pFromData)[30] << 16;
  (pToStruct)->m_nReserved1 |=  (uint32)  (pFromData)[31] << 24;

  /* Unpack Member: m_nReserved0 */
  (pToStruct)->m_nReserved0 =  (uint32)  (pFromData)[24] ;
  (pToStruct)->m_nReserved0 |=  (uint32)  (pFromData)[25] << 8;
  (pToStruct)->m_nReserved0 |=  (uint32)  (pFromData)[26] << 16;
  (pToStruct)->m_nReserved0 |=  (uint32)  (pFromData)[27] << 24;

  /* Unpack Member: m_nEcn_dp2 */
  (pToStruct)->m_nEcn_dp2 =  (uint32)  (pFromData)[22] ;
  (pToStruct)->m_nEcn_dp2 |=  (uint32)  (pFromData)[23] << 8;

  /* Unpack Member: m_nScale_dp2 */
  (pToStruct)->m_nScale_dp2 =  (uint32)  ((pFromData)[21] >> 4) & 0x0f;

  /* Unpack Member: m_nSlope_dp2 */
  (pToStruct)->m_nSlope_dp2 =  (uint32)  (pFromData)[20] ;
  (pToStruct)->m_nSlope_dp2 |=  (uint32)  ((pFromData)[21] & 0x0f) << 8;

  /* Unpack Member: m_nMin_dp2 */
  (pToStruct)->m_nMin_dp2 =  (uint32)  (pFromData)[18] ;
  (pToStruct)->m_nMin_dp2 |=  (uint32)  (pFromData)[19] << 8;

  /* Unpack Member: m_nMax_dp2 */
  (pToStruct)->m_nMax_dp2 =  (uint32)  (pFromData)[16] ;
  (pToStruct)->m_nMax_dp2 |=  (uint32)  (pFromData)[17] << 8;

  /* Unpack Member: m_nEcn_dp1 */
  (pToStruct)->m_nEcn_dp1 =  (uint32)  (pFromData)[14] ;
  (pToStruct)->m_nEcn_dp1 |=  (uint32)  (pFromData)[15] << 8;

  /* Unpack Member: m_nScale_dp1 */
  (pToStruct)->m_nScale_dp1 =  (uint32)  ((pFromData)[13] >> 4) & 0x0f;

  /* Unpack Member: m_nSlope_dp1 */
  (pToStruct)->m_nSlope_dp1 =  (uint32)  (pFromData)[12] ;
  (pToStruct)->m_nSlope_dp1 |=  (uint32)  ((pFromData)[13] & 0x0f) << 8;

  /* Unpack Member: m_nMin_dp1 */
  (pToStruct)->m_nMin_dp1 =  (uint32)  (pFromData)[10] ;
  (pToStruct)->m_nMin_dp1 |=  (uint32)  (pFromData)[11] << 8;

  /* Unpack Member: m_nMax_dp1 */
  (pToStruct)->m_nMax_dp1 =  (uint32)  (pFromData)[8] ;
  (pToStruct)->m_nMax_dp1 |=  (uint32)  (pFromData)[9] << 8;

  /* Unpack Member: m_nEcn_dp0 */
  (pToStruct)->m_nEcn_dp0 =  (uint32)  (pFromData)[6] ;
  (pToStruct)->m_nEcn_dp0 |=  (uint32)  (pFromData)[7] << 8;

  /* Unpack Member: m_nScale_dp0 */
  (pToStruct)->m_nScale_dp0 =  (uint32)  ((pFromData)[5] >> 4) & 0x0f;

  /* Unpack Member: m_nSlope_dp0 */
  (pToStruct)->m_nSlope_dp0 =  (uint32)  (pFromData)[4] ;
  (pToStruct)->m_nSlope_dp0 |=  (uint32)  ((pFromData)[5] & 0x0f) << 8;

  /* Unpack Member: m_nMin_dp0 */
  (pToStruct)->m_nMin_dp0 =  (uint32)  (pFromData)[2] ;
  (pToStruct)->m_nMin_dp0 |=  (uint32)  (pFromData)[3] << 8;

  /* Unpack Member: m_nMax_dp0 */
  (pToStruct)->m_nMax_dp0 =  (uint32)  (pFromData)[0] ;
  (pToStruct)->m_nMax_dp0 |=  (uint32)  (pFromData)[1] << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwWctEntry_InitInstance(sbZfFabBm3200BwWctEntry_t *pFrame) {

  pFrame->m_nReserved1 =  (unsigned int)  0;
  pFrame->m_nReserved0 =  (unsigned int)  0;
  pFrame->m_nEcn_dp2 =  (unsigned int)  0;
  pFrame->m_nScale_dp2 =  (unsigned int)  0;
  pFrame->m_nSlope_dp2 =  (unsigned int)  0;
  pFrame->m_nMin_dp2 =  (unsigned int)  0;
  pFrame->m_nMax_dp2 =  (unsigned int)  0;
  pFrame->m_nEcn_dp1 =  (unsigned int)  0;
  pFrame->m_nScale_dp1 =  (unsigned int)  0;
  pFrame->m_nSlope_dp1 =  (unsigned int)  0;
  pFrame->m_nMin_dp1 =  (unsigned int)  0;
  pFrame->m_nMax_dp1 =  (unsigned int)  0;
  pFrame->m_nEcn_dp0 =  (unsigned int)  0;
  pFrame->m_nScale_dp0 =  (unsigned int)  0;
  pFrame->m_nSlope_dp0 =  (unsigned int)  0;
  pFrame->m_nMin_dp0 =  (unsigned int)  0;
  pFrame->m_nMax_dp0 =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwWctEntry.c,v 1.4 Broadcom SDK $
 * $Copyright (c) 2012 Broadcom Corporation
 * All rights reserved.$
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabBm3200BwWctEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwWctEntry_Print(sbZfFabBm3200BwWctEntry_t *pFromStruct) {
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwWctEntry:: reserved1=0x%08x"), (unsigned int)  pFromStruct->m_nReserved1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" reserved0=0x%08x"), (unsigned int)  pFromStruct->m_nReserved0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwWctEntry:: ecn_dp2=0x%04x"), (unsigned int)  pFromStruct->m_nEcn_dp2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" scale_dp2=0x%01x"), (unsigned int)  pFromStruct->m_nScale_dp2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" slope_dp2=0x%03x"), (unsigned int)  pFromStruct->m_nSlope_dp2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwWctEntry:: min_dp2=0x%04x"), (unsigned int)  pFromStruct->m_nMin_dp2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" max_dp2=0x%04x"), (unsigned int)  pFromStruct->m_nMax_dp2));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" ecn_dp1=0x%04x"), (unsigned int)  pFromStruct->m_nEcn_dp1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwWctEntry:: scale_dp1=0x%01x"), (unsigned int)  pFromStruct->m_nScale_dp1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" slope_dp1=0x%03x"), (unsigned int)  pFromStruct->m_nSlope_dp1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" min_dp1=0x%04x"), (unsigned int)  pFromStruct->m_nMin_dp1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwWctEntry:: max_dp1=0x%04x"), (unsigned int)  pFromStruct->m_nMax_dp1));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" ecn_dp0=0x%04x"), (unsigned int)  pFromStruct->m_nEcn_dp0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" scale_dp0=0x%01x"), (unsigned int)  pFromStruct->m_nScale_dp0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("FabBm3200BwWctEntry:: slope_dp0=0x%03x"), (unsigned int)  pFromStruct->m_nSlope_dp0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" min_dp0=0x%04x"), (unsigned int)  pFromStruct->m_nMin_dp0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META(" max_dp0=0x%04x"), (unsigned int)  pFromStruct->m_nMax_dp0));
  LOG_WARN(BSL_LS_SOC_COMMON,
           (BSL_META("\n")));

}

/* SPrint members in struct */
int
sbZfFabBm3200BwWctEntry_SPrint(sbZfFabBm3200BwWctEntry_t *pFromStruct, char *pcToString, uint32 lStrSize) {
  uint32 WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwWctEntry:: reserved1=0x%08x", (unsigned int)  pFromStruct->m_nReserved1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," reserved0=0x%08x", (unsigned int)  pFromStruct->m_nReserved0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwWctEntry:: ecn_dp2=0x%04x", (unsigned int)  pFromStruct->m_nEcn_dp2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," scale_dp2=0x%01x", (unsigned int)  pFromStruct->m_nScale_dp2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," slope_dp2=0x%03x", (unsigned int)  pFromStruct->m_nSlope_dp2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwWctEntry:: min_dp2=0x%04x", (unsigned int)  pFromStruct->m_nMin_dp2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," max_dp2=0x%04x", (unsigned int)  pFromStruct->m_nMax_dp2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ecn_dp1=0x%04x", (unsigned int)  pFromStruct->m_nEcn_dp1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwWctEntry:: scale_dp1=0x%01x", (unsigned int)  pFromStruct->m_nScale_dp1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," slope_dp1=0x%03x", (unsigned int)  pFromStruct->m_nSlope_dp1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," min_dp1=0x%04x", (unsigned int)  pFromStruct->m_nMin_dp1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwWctEntry:: max_dp1=0x%04x", (unsigned int)  pFromStruct->m_nMax_dp1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ecn_dp0=0x%04x", (unsigned int)  pFromStruct->m_nEcn_dp0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," scale_dp0=0x%01x", (unsigned int)  pFromStruct->m_nScale_dp0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwWctEntry:: slope_dp0=0x%03x", (unsigned int)  pFromStruct->m_nSlope_dp0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," min_dp0=0x%04x", (unsigned int)  pFromStruct->m_nMin_dp0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," max_dp0=0x%04x", (unsigned int)  pFromStruct->m_nMax_dp0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwWctEntry_Validate(sbZfFabBm3200BwWctEntry_t *pZf) {

  /* pZf->m_nReserved1 implicitly masked by data type */
  /* pZf->m_nReserved0 implicitly masked by data type */
  if (pZf->m_nEcn_dp2 > 0xffff) return 0;
  if (pZf->m_nScale_dp2 > 0xf) return 0;
  if (pZf->m_nSlope_dp2 > 0xfff) return 0;
  if (pZf->m_nMin_dp2 > 0xffff) return 0;
  if (pZf->m_nMax_dp2 > 0xffff) return 0;
  if (pZf->m_nEcn_dp1 > 0xffff) return 0;
  if (pZf->m_nScale_dp1 > 0xf) return 0;
  if (pZf->m_nSlope_dp1 > 0xfff) return 0;
  if (pZf->m_nMin_dp1 > 0xffff) return 0;
  if (pZf->m_nMax_dp1 > 0xffff) return 0;
  if (pZf->m_nEcn_dp0 > 0xffff) return 0;
  if (pZf->m_nScale_dp0 > 0xf) return 0;
  if (pZf->m_nSlope_dp0 > 0xfff) return 0;
  if (pZf->m_nMin_dp0 > 0xffff) return 0;
  if (pZf->m_nMax_dp0 > 0xffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwWctEntry_SetField(sbZfFabBm3200BwWctEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved1") == 0) {
    s->m_nReserved1 = value;
  } else if (SB_STRCMP(name, "m_nreserved0") == 0) {
    s->m_nReserved0 = value;
  } else if (SB_STRCMP(name, "m_necn_dp2") == 0) {
    s->m_nEcn_dp2 = value;
  } else if (SB_STRCMP(name, "m_nscale_dp2") == 0) {
    s->m_nScale_dp2 = value;
  } else if (SB_STRCMP(name, "m_nslope_dp2") == 0) {
    s->m_nSlope_dp2 = value;
  } else if (SB_STRCMP(name, "m_nmin_dp2") == 0) {
    s->m_nMin_dp2 = value;
  } else if (SB_STRCMP(name, "m_nmax_dp2") == 0) {
    s->m_nMax_dp2 = value;
  } else if (SB_STRCMP(name, "m_necn_dp1") == 0) {
    s->m_nEcn_dp1 = value;
  } else if (SB_STRCMP(name, "m_nscale_dp1") == 0) {
    s->m_nScale_dp1 = value;
  } else if (SB_STRCMP(name, "m_nslope_dp1") == 0) {
    s->m_nSlope_dp1 = value;
  } else if (SB_STRCMP(name, "m_nmin_dp1") == 0) {
    s->m_nMin_dp1 = value;
  } else if (SB_STRCMP(name, "m_nmax_dp1") == 0) {
    s->m_nMax_dp1 = value;
  } else if (SB_STRCMP(name, "m_necn_dp0") == 0) {
    s->m_nEcn_dp0 = value;
  } else if (SB_STRCMP(name, "m_nscale_dp0") == 0) {
    s->m_nScale_dp0 = value;
  } else if (SB_STRCMP(name, "m_nslope_dp0") == 0) {
    s->m_nSlope_dp0 = value;
  } else if (SB_STRCMP(name, "m_nmin_dp0") == 0) {
    s->m_nMin_dp0 = value;
  } else if (SB_STRCMP(name, "m_nmax_dp0") == 0) {
    s->m_nMax_dp0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */

