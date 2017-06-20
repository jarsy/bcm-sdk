/*
 * $Id: sbZfFabBm9600NmEmt_1EntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600NmEmt_1Entry.hx"
#ifndef SB_ZF_FAB_BM9600_NMEMT_1ENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_NMEMT_1ENTRY_CONSOLE_H



void
sbZfFabBm9600NmEmt_1Entry_Print(sbZfFabBm9600NmEmt_1Entry_t *pFromStruct);
char *
sbZfFabBm9600NmEmt_1Entry_SPrint(sbZfFabBm9600NmEmt_1Entry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600NmEmt_1Entry_Validate(sbZfFabBm9600NmEmt_1Entry_t *pZf);
int
sbZfFabBm9600NmEmt_1Entry_SetField(sbZfFabBm9600NmEmt_1Entry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_NMEMT_1ENTRY_CONSOLE_H */
