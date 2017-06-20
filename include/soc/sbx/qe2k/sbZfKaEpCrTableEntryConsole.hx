/*
 * $Id: sbZfKaEpCrTableEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpCrTableEntry.hx"
#ifndef SB_ZF_ZFKAEPCRTABLEENTRY_CONSOLE_H
#define SB_ZF_ZFKAEPCRTABLEENTRY_CONSOLE_H



void
sbZfKaEpCrTableEntry_Print(sbZfKaEpCrTableEntry_t *pFromStruct);
char *
sbZfKaEpCrTableEntry_SPrint(sbZfKaEpCrTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpCrTableEntry_Validate(sbZfKaEpCrTableEntry_t *pZf);
int
sbZfKaEpCrTableEntry_SetField(sbZfKaEpCrTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPCRTABLEENTRY_CONSOLE_H */
