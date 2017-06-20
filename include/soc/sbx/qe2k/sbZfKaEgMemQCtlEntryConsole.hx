/*
 * $Id: sbZfKaEgMemQCtlEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEgMemQCtlEntry.hx"
#ifndef SB_ZF_ZFKAEGMEMQCTLENTRY_CONSOLE_H
#define SB_ZF_ZFKAEGMEMQCTLENTRY_CONSOLE_H



void
sbZfKaEgMemQCtlEntry_Print(sbZfKaEgMemQCtlEntry_t *pFromStruct);
char *
sbZfKaEgMemQCtlEntry_SPrint(sbZfKaEgMemQCtlEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEgMemQCtlEntry_Validate(sbZfKaEgMemQCtlEntry_t *pZf);
int
sbZfKaEgMemQCtlEntry_SetField(sbZfKaEgMemQCtlEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEGMEMQCTLENTRY_CONSOLE_H */
