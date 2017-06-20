/*
 * $Id: sbZfKaRbClassDmacMatchEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaRbClassDmacMatchEntry.hx"
#ifndef SB_ZF_ZFKARBCLASSDMACMATCHENTRY_CONSOLE_H
#define SB_ZF_ZFKARBCLASSDMACMATCHENTRY_CONSOLE_H



void
sbZfKaRbClassDmacMatchEntry_Print(sbZfKaRbClassDmacMatchEntry_t *pFromStruct);
char *
sbZfKaRbClassDmacMatchEntry_SPrint(sbZfKaRbClassDmacMatchEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaRbClassDmacMatchEntry_Validate(sbZfKaRbClassDmacMatchEntry_t *pZf);
int
sbZfKaRbClassDmacMatchEntry_SetField(sbZfKaRbClassDmacMatchEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKARBCLASSDMACMATCHENTRY_CONSOLE_H */
