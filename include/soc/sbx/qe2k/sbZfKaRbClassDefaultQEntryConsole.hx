/*
 * $Id: sbZfKaRbClassDefaultQEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaRbClassDefaultQEntry.hx"
#ifndef SB_ZF_ZFKARBCLASSDEFAULTQENTRY_CONSOLE_H
#define SB_ZF_ZFKARBCLASSDEFAULTQENTRY_CONSOLE_H



void
sbZfKaRbClassDefaultQEntry_Print(sbZfKaRbClassDefaultQEntry_t *pFromStruct);
char *
sbZfKaRbClassDefaultQEntry_SPrint(sbZfKaRbClassDefaultQEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaRbClassDefaultQEntry_Validate(sbZfKaRbClassDefaultQEntry_t *pZf);
int
sbZfKaRbClassDefaultQEntry_SetField(sbZfKaRbClassDefaultQEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKARBCLASSDEFAULTQENTRY_CONSOLE_H */
