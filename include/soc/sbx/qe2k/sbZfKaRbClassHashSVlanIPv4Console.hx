/*
 * $Id: sbZfKaRbClassHashSVlanIPv4Console.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaRbClassHashSVlanIPv4.hx"
#ifndef SB_ZF_ZFKARBCLASSHASHSVLANIPV4_CONSOLE_H
#define SB_ZF_ZFKARBCLASSHASHSVLANIPV4_CONSOLE_H



void
sbZfKaRbClassHashSVlanIPv4_Print(sbZfKaRbClassHashSVlanIPv4_t *pFromStruct);
char *
sbZfKaRbClassHashSVlanIPv4_SPrint(sbZfKaRbClassHashSVlanIPv4_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaRbClassHashSVlanIPv4_Validate(sbZfKaRbClassHashSVlanIPv4_t *pZf);
int
sbZfKaRbClassHashSVlanIPv4_SetField(sbZfKaRbClassHashSVlanIPv4_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKARBCLASSHASHSVLANIPV4_CONSOLE_H */
