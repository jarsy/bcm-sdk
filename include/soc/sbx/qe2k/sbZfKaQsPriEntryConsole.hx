/*
 * $Id: sbZfKaQsPriEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsPriEntry.hx"
#ifndef SB_ZF_ZFKAQSPRIENTRY_CONSOLE_H
#define SB_ZF_ZFKAQSPRIENTRY_CONSOLE_H



void
sbZfKaQsPriEntry_Print(sbZfKaQsPriEntry_t *pFromStruct);
char *
sbZfKaQsPriEntry_SPrint(sbZfKaQsPriEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsPriEntry_Validate(sbZfKaQsPriEntry_t *pZf);
int
sbZfKaQsPriEntry_SetField(sbZfKaQsPriEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSPRIENTRY_CONSOLE_H */
