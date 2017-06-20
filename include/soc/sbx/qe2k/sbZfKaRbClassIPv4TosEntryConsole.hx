/*
 * $Id: sbZfKaRbClassIPv4TosEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaRbClassIPv4TosEntry.hx"
#ifndef SB_ZF_ZFKARBCLASSIPV4TOSENTRY_CONSOLE_H
#define SB_ZF_ZFKARBCLASSIPV4TOSENTRY_CONSOLE_H



void
sbZfKaRbClassIPv4TosEntry_Print(sbZfKaRbClassIPv4TosEntry_t *pFromStruct);
char *
sbZfKaRbClassIPv4TosEntry_SPrint(sbZfKaRbClassIPv4TosEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaRbClassIPv4TosEntry_Validate(sbZfKaRbClassIPv4TosEntry_t *pZf);
int
sbZfKaRbClassIPv4TosEntry_SetField(sbZfKaRbClassIPv4TosEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKARBCLASSIPV4TOSENTRY_CONSOLE_H */
