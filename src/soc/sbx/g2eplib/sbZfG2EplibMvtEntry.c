/*
 * $Id: sbZfG2EplibMvtEntry.c,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypesGlue.h"
#include "sbZfG2EplibMvtEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfG2EplibMvtEntry_Pack(sbZfG2EplibMvtEntry_t *pFrom,
                         uint8 *pToData,
                         uint32 nMaxToDataIndex) {
  int i;
  int size = SB_ZF_G2_EPLIB_MVTENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on big endian */

  /* Pack Member: ullPortMask */
  (pToData)[0] |= (COMPILER_64_LO((pFrom)->ullPortMask) & 0x01);

  /* Pack Member: nType */
  (pToData)[0] |= ((pFrom)->nType & 0x01);

  /* Pack Member: ulMvtdA */
  (pToData)[0] |= ((pFrom)->ulMvtdA & 0x01);

  /* Pack Member: ulMvtdB */
  (pToData)[0] |= ((pFrom)->ulMvtdB & 0x01);

  /* Pack Member: bSourceKnockout */
  (pToData)[0] |= ((pFrom)->bSourceKnockout & 0x01);

  /* Pack Member: bEnableChaining */
  (pToData)[0] |= ((pFrom)->bEnableChaining & 0x01);

  /* Pack Member: ulNextMcGroup */
  (pToData)[0] |= ((pFrom)->ulNextMcGroup & 0x01);

  return SB_ZF_G2_EPLIB_MVTENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfG2EplibMvtEntry_Unpack(sbZfG2EplibMvtEntry_t *pToStruct,
                           uint8 *pFromData,
                           uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on big endian */

  /* Unpack Member: ullPortMask */
  COMPILER_64_SET((pToStruct)->ullPortMask, 0,  (unsigned int) (pFromData)[0]);

  /* Unpack Member: nType */
  (pToStruct)->nType =  (uint32)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: ulMvtdA */
  (pToStruct)->ulMvtdA =  (uint32)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: ulMvtdB */
  (pToStruct)->ulMvtdB =  (uint32)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: bSourceKnockout */
  (pToStruct)->bSourceKnockout =  (uint8)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: bEnableChaining */
  (pToStruct)->bEnableChaining =  (uint8)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: ulNextMcGroup */
  (pToStruct)->ulNextMcGroup =  (uint32)  ((pFromData)[0] ) & 0x01;

}



/* initialize an instance of this zframe */
void
sbZfG2EplibMvtEntry_InitInstance(sbZfG2EplibMvtEntry_t *pFrame) {

  COMPILER_64_ZERO(pFrame->ullPortMask);
  pFrame->nType =  (unsigned int)  0;
  pFrame->ulMvtdA =  (unsigned int)  0;
  pFrame->ulMvtdB =  (unsigned int)  0;
  pFrame->bSourceKnockout =  (unsigned int)  0;
  pFrame->bEnableChaining =  (unsigned int)  0;
  pFrame->ulNextMcGroup =  (unsigned int)  0;

}
