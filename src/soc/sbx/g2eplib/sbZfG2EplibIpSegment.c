/*
 * $Id: sbZfG2EplibIpSegment.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#include "sbTypesGlue.h"
#include "sbZfG2EplibIpSegment.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32
sbZfG2EplibIpSegment_Pack(sbZfG2EplibIpSegment_t *pFrom,
                          uint8 *pToData,
                          uint32 nMaxToDataIndex) {
  int i;
  int size = SB_ZF_G2_EPLIB_IPSEGMENT_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on big endian */

  /* Pack Member: start */
  (pToData)[0] |= ((pFrom)->start & 0x01);

  /* Pack Member: end */
  (pToData)[0] |= ((pFrom)->end & 0x01);

  /* Pack Member: width */
  (pToData)[0] |= ((pFrom)->width & 0x01);

  /* Pack Member: entrysize */
  (pToData)[0] |= ((pFrom)->entrysize & 0x01);

  return SB_ZF_G2_EPLIB_IPSEGMENT_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfG2EplibIpSegment_Unpack(sbZfG2EplibIpSegment_t *pToStruct,
                            uint8 *pFromData,
                            uint32 nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on big endian */

  /* Unpack Member: start */
  (pToStruct)->start =  (uint32)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: end */
  (pToStruct)->end =  (uint32)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: width */
  (pToStruct)->width =  (uint32)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: entrysize */
  (pToStruct)->entrysize =  (uint32)  ((pFromData)[0] ) & 0x01;

}



/* initialize an instance of this zframe */
void
sbZfG2EplibIpSegment_InitInstance(sbZfG2EplibIpSegment_t *pFrame) {

  pFrame->start =  (unsigned int)  0;
  pFrame->end =  (unsigned int)  0;
  pFrame->width =  (unsigned int)  0;
  pFrame->entrysize =  (unsigned int)  0;

}
