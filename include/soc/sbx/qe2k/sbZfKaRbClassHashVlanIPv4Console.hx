/*
 * $Id: sbZfKaRbClassHashVlanIPv4Console.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaRbClassHashVlanIPv4.hx"
#ifndef SB_ZF_ZFKARBCLASSHASHVLANIPV4_CONSOLE_H
#define SB_ZF_ZFKARBCLASSHASHVLANIPV4_CONSOLE_H



void
sbZfKaRbClassHashVlanIPv4_Print(sbZfKaRbClassHashVlanIPv4_t *pFromStruct);
char *
sbZfKaRbClassHashVlanIPv4_SPrint(sbZfKaRbClassHashVlanIPv4_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaRbClassHashVlanIPv4_Validate(sbZfKaRbClassHashVlanIPv4_t *pZf);
int
sbZfKaRbClassHashVlanIPv4_SetField(sbZfKaRbClassHashVlanIPv4_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKARBCLASSHASHVLANIPV4_CONSOLE_H */
