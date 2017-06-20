/*
 * $Id: sbZfKaRbClassIPv6ClassEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaRbClassIPv6ClassEntry.hx"
#ifndef SB_ZF_ZFKARBCLASSIPV6CLASSENTRY_CONSOLE_H
#define SB_ZF_ZFKARBCLASSIPV6CLASSENTRY_CONSOLE_H



void
sbZfKaRbClassIPv6ClassEntry_Print(sbZfKaRbClassIPv6ClassEntry_t *pFromStruct);
char *
sbZfKaRbClassIPv6ClassEntry_SPrint(sbZfKaRbClassIPv6ClassEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaRbClassIPv6ClassEntry_Validate(sbZfKaRbClassIPv6ClassEntry_t *pZf);
int
sbZfKaRbClassIPv6ClassEntry_SetField(sbZfKaRbClassIPv6ClassEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKARBCLASSIPV6CLASSENTRY_CONSOLE_H */
