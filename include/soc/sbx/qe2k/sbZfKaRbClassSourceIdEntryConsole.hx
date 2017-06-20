/*
 * $Id: sbZfKaRbClassSourceIdEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaRbClassSourceIdEntry.hx"
#ifndef SB_ZF_ZFKARBCLASSSOURCEIDENTRY_CONSOLE_H
#define SB_ZF_ZFKARBCLASSSOURCEIDENTRY_CONSOLE_H



void
sbZfKaRbClassSourceIdEntry_Print(sbZfKaRbClassSourceIdEntry_t *pFromStruct);
char *
sbZfKaRbClassSourceIdEntry_SPrint(sbZfKaRbClassSourceIdEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaRbClassSourceIdEntry_Validate(sbZfKaRbClassSourceIdEntry_t *pZf);
int
sbZfKaRbClassSourceIdEntry_SetField(sbZfKaRbClassSourceIdEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKARBCLASSSOURCEIDENTRY_CONSOLE_H */
