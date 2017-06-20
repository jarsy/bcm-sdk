/*
 * $Id: sbZfKaEpBfPriTableEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpBfPriTableEntry.hx"
#ifndef SB_ZF_ZFKAEPBFPRITABLEENTRY_CONSOLE_H
#define SB_ZF_ZFKAEPBFPRITABLEENTRY_CONSOLE_H



void
sbZfKaEpBfPriTableEntry_Print(sbZfKaEpBfPriTableEntry_t *pFromStruct);
char *
sbZfKaEpBfPriTableEntry_SPrint(sbZfKaEpBfPriTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpBfPriTableEntry_Validate(sbZfKaEpBfPriTableEntry_t *pZf);
int
sbZfKaEpBfPriTableEntry_SetField(sbZfKaEpBfPriTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPBFPRITABLEENTRY_CONSOLE_H */
