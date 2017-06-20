/*  -*-  Mode:C; c-basic-offset:4 -*- */
/*
 * $Id: sbZfG2EplibIpSegment.hx,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef SB_ZF_G2_EPLIB_IPSEGMENT_H
#define SB_ZF_G2_EPLIB_IPSEGMENT_H

#define SB_ZF_G2_EPLIB_IPSEGMENT_SIZE_IN_BYTES 1
#define SB_ZF_G2_EPLIB_IPSEGMENT_SIZE 1
#define SB_ZF_G2_EPLIB_IPSEGMENT_START_BITS "0:0"
#define SB_ZF_G2_EPLIB_IPSEGMENT_END_BITS "0:0"
#define SB_ZF_G2_EPLIB_IPSEGMENT_WIDTH_BITS "0:0"
#define SB_ZF_G2_EPLIB_IPSEGMENT_ENTRYSIZE_BITS "0:0"



/** @brief  EP IP Memory Segment

  IP Memory Segment
*/

typedef struct _sbZfG2EplibIpSegment {
  uint32 start;
  uint32 end;
  uint32 width;
  uint32 entrysize;
} sbZfG2EplibIpSegment_t;

uint32
sbZfG2EplibIpSegment_Pack(sbZfG2EplibIpSegment_t *pFrom,
                          uint8 *pToData,
                          uint32 nMaxToDataIndex);
void
sbZfG2EplibIpSegment_Unpack(sbZfG2EplibIpSegment_t *pToStruct,
                            uint8 *pFromData,
                            uint32 nMaxToDataIndex);
void
sbZfG2EplibIpSegment_InitInstance(sbZfG2EplibIpSegment_t *pFrame);

#define SB_ZF_G2EPLIBIPSEGMENT_SET_START(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBIPSEGMENT_SET_END(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBIPSEGMENT_SET_WIDTH(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBIPSEGMENT_SET_ENTSIZE(nFromData,pToData,nMaxToDataIndex) \
          do { \
           (pToData)[0] = ((pToData)[0] & ~0x01) | ((nFromData) & 0x01); \
          } while(0)

#define SB_ZF_G2EPLIBIPSEGMENT_GET_START(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_G2EPLIBIPSEGMENT_GET_END(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_G2EPLIBIPSEGMENT_GET_WIDTH(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#define SB_ZF_G2EPLIBIPSEGMENT_GET_ENTSIZE(nToData,pFromData,nMaxFromDataIndex) \
          do { \
           (nToData) = (uint32) ((pFromData)[0]) & 0x01; \
          } while(0)

#endif
