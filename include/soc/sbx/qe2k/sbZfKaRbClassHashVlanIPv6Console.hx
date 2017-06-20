/*
 * $Id: sbZfKaRbClassHashVlanIPv6Console.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaRbClassHashVlanIPv6.hx"
#ifndef SB_ZF_ZFKARBCLASSHASHVLANIPV6_CONSOLE_H
#define SB_ZF_ZFKARBCLASSHASHVLANIPV6_CONSOLE_H



void
sbZfKaRbClassHashVlanIPv6_Print(sbZfKaRbClassHashVlanIPv6_t *pFromStruct);
char *
sbZfKaRbClassHashVlanIPv6_SPrint(sbZfKaRbClassHashVlanIPv6_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaRbClassHashVlanIPv6_Validate(sbZfKaRbClassHashVlanIPv6_t *pZf);
int
sbZfKaRbClassHashVlanIPv6_SetField(sbZfKaRbClassHashVlanIPv6_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKARBCLASSHASHVLANIPV6_CONSOLE_H */
