/*
 * $Id: sbZfG2EplibIpSegmentConsole.hx,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfG2EplibIpSegment.hx"
#ifndef SB_ZF_G2_EPLIB_IPSEGMENT_CONSOLE_H
#define SB_ZF_G2_EPLIB_IPSEGMENT_CONSOLE_H



void
sbZfG2EplibIpSegment_Print(sbZfG2EplibIpSegment_t *pFromStruct);
char *
sbZfG2EplibIpSegment_SPrint(sbZfG2EplibIpSegment_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfG2EplibIpSegment_Validate(sbZfG2EplibIpSegment_t *pZf);
int
sbZfG2EplibIpSegment_SetField(sbZfG2EplibIpSegment_t *s, char* name, int value);


#endif /* ifndef SB_ZF_G2_EPLIB_IPSEGMENT_CONSOLE_H */
