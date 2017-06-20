/*
 * $Id: sbZfFabBm9600BwR1WstEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600BwR1WstEntry.hx"
#ifndef SB_ZF_FAB_BM9600_BWR1WSTENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_BWR1WSTENTRY_CONSOLE_H



void
sbZfFabBm9600BwR1WstEntry_Print(sbZfFabBm9600BwR1WstEntry_t *pFromStruct);
char *
sbZfFabBm9600BwR1WstEntry_SPrint(sbZfFabBm9600BwR1WstEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600BwR1WstEntry_Validate(sbZfFabBm9600BwR1WstEntry_t *pZf);
int
sbZfFabBm9600BwR1WstEntry_SetField(sbZfFabBm9600BwR1WstEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_BWR1WSTENTRY_CONSOLE_H */
