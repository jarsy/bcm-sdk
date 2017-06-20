/*
 * $Id: sbZfFabBm9600NmEmtdebugbank1EntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600NmEmtdebugbank1Entry.hx"
#ifndef SB_ZF_FAB_BM9600_NMEMTDEBUGBANK1ENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_NMEMTDEBUGBANK1ENTRY_CONSOLE_H



void
sbZfFabBm9600NmEmtdebugbank1Entry_Print(sbZfFabBm9600NmEmtdebugbank1Entry_t *pFromStruct);
char *
sbZfFabBm9600NmEmtdebugbank1Entry_SPrint(sbZfFabBm9600NmEmtdebugbank1Entry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600NmEmtdebugbank1Entry_Validate(sbZfFabBm9600NmEmtdebugbank1Entry_t *pZf);
int
sbZfFabBm9600NmEmtdebugbank1Entry_SetField(sbZfFabBm9600NmEmtdebugbank1Entry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_NMEMTDEBUGBANK1ENTRY_CONSOLE_H */
