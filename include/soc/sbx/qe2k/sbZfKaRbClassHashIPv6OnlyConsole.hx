/*
 * $Id: sbZfKaRbClassHashIPv6OnlyConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaRbClassHashIPv6Only.hx"
#ifndef SB_ZF_ZFKARBCLASSHASHIPV6ONLY_CONSOLE_H
#define SB_ZF_ZFKARBCLASSHASHIPV6ONLY_CONSOLE_H



void
sbZfKaRbClassHashIPv6Only_Print(sbZfKaRbClassHashIPv6Only_t *pFromStruct);
char *
sbZfKaRbClassHashIPv6Only_SPrint(sbZfKaRbClassHashIPv6Only_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaRbClassHashIPv6Only_Validate(sbZfKaRbClassHashIPv6Only_t *pZf);
int
sbZfKaRbClassHashIPv6Only_SetField(sbZfKaRbClassHashIPv6Only_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKARBCLASSHASHIPV6ONLY_CONSOLE_H */
