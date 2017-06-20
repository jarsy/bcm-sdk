/*
 * $Id: sbZfKaQsLnaPriEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsLnaPriEntry.hx"
#ifndef SB_ZF_ZFKAQSLNAPRIENTRY_CONSOLE_H
#define SB_ZF_ZFKAQSLNAPRIENTRY_CONSOLE_H



void
sbZfKaQsLnaPriEntry_Print(sbZfKaQsLnaPriEntry_t *pFromStruct);
char *
sbZfKaQsLnaPriEntry_SPrint(sbZfKaQsLnaPriEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsLnaPriEntry_Validate(sbZfKaQsLnaPriEntry_t *pZf);
int
sbZfKaQsLnaPriEntry_SetField(sbZfKaQsLnaPriEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSLNAPRIENTRY_CONSOLE_H */
