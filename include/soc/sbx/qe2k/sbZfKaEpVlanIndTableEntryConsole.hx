/*
 * $Id: sbZfKaEpVlanIndTableEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpVlanIndTableEntry.hx"
#ifndef SB_ZF_ZFKAEPVLANINDTABLEENTRY_CONSOLE_H
#define SB_ZF_ZFKAEPVLANINDTABLEENTRY_CONSOLE_H



void
sbZfKaEpVlanIndTableEntry_Print(sbZfKaEpVlanIndTableEntry_t *pFromStruct);
char *
sbZfKaEpVlanIndTableEntry_SPrint(sbZfKaEpVlanIndTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpVlanIndTableEntry_Validate(sbZfKaEpVlanIndTableEntry_t *pZf);
int
sbZfKaEpVlanIndTableEntry_SetField(sbZfKaEpVlanIndTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPVLANINDTABLEENTRY_CONSOLE_H */
