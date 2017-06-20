/*
 * $Id: sbZfKaRbClassHashSVlanIPv6Console.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaRbClassHashSVlanIPv6.hx"
#ifndef SB_ZF_ZFKARBCLASSHASHSVLANIPV6_CONSOLE_H
#define SB_ZF_ZFKARBCLASSHASHSVLANIPV6_CONSOLE_H



void
sbZfKaRbClassHashSVlanIPv6_Print(sbZfKaRbClassHashSVlanIPv6_t *pFromStruct);
char *
sbZfKaRbClassHashSVlanIPv6_SPrint(sbZfKaRbClassHashSVlanIPv6_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaRbClassHashSVlanIPv6_Validate(sbZfKaRbClassHashSVlanIPv6_t *pZf);
int
sbZfKaRbClassHashSVlanIPv6_SetField(sbZfKaRbClassHashSVlanIPv6_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKARBCLASSHASHSVLANIPV6_CONSOLE_H */
