/*
 * $Id: sbZfFabBm9600BwR0WdtEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600BwR0WdtEntry.hx"
#ifndef SB_ZF_FAB_BM9600_BWR0WDTENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_BWR0WDTENTRY_CONSOLE_H



void
sbZfFabBm9600BwR0WdtEntry_Print(sbZfFabBm9600BwR0WdtEntry_t *pFromStruct);
char *
sbZfFabBm9600BwR0WdtEntry_SPrint(sbZfFabBm9600BwR0WdtEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600BwR0WdtEntry_Validate(sbZfFabBm9600BwR0WdtEntry_t *pZf);
int
sbZfFabBm9600BwR0WdtEntry_SetField(sbZfFabBm9600BwR0WdtEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_BWR0WDTENTRY_CONSOLE_H */
