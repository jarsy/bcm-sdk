/*
 * $Id: sbZfKaRbClassPortEnablesEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaRbClassPortEnablesEntry.hx"
#ifndef SB_ZF_ZFKARBCLASSPORTENABLESENTRY_CONSOLE_H
#define SB_ZF_ZFKARBCLASSPORTENABLESENTRY_CONSOLE_H



void
sbZfKaRbClassPortEnablesEntry_Print(sbZfKaRbClassPortEnablesEntry_t *pFromStruct);
char *
sbZfKaRbClassPortEnablesEntry_SPrint(sbZfKaRbClassPortEnablesEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaRbClassPortEnablesEntry_Validate(sbZfKaRbClassPortEnablesEntry_t *pZf);
int
sbZfKaRbClassPortEnablesEntry_SetField(sbZfKaRbClassPortEnablesEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKARBCLASSPORTENABLESENTRY_CONSOLE_H */
