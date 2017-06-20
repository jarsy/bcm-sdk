/*
 * $Id: sbZfKaEpPortTableEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpPortTableEntry.hx"
#ifndef SB_ZF_ZFKAEPPORTTABLEENTRY_CONSOLE_H
#define SB_ZF_ZFKAEPPORTTABLEENTRY_CONSOLE_H



void
sbZfKaEpPortTableEntry_Print(sbZfKaEpPortTableEntry_t *pFromStruct);
char *
sbZfKaEpPortTableEntry_SPrint(sbZfKaEpPortTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpPortTableEntry_Validate(sbZfKaEpPortTableEntry_t *pZf);
int
sbZfKaEpPortTableEntry_SetField(sbZfKaEpPortTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPPORTTABLEENTRY_CONSOLE_H */
