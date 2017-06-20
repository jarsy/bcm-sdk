/*
 * $Id: sbZfFabBm9600BwR0BwpEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600BwR0BwpEntry.hx"
#ifndef SB_ZF_FAB_BM9600_BWR0BWPENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_BWR0BWPENTRY_CONSOLE_H



void
sbZfFabBm9600BwR0BwpEntry_Print(sbZfFabBm9600BwR0BwpEntry_t *pFromStruct);
char *
sbZfFabBm9600BwR0BwpEntry_SPrint(sbZfFabBm9600BwR0BwpEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600BwR0BwpEntry_Validate(sbZfFabBm9600BwR0BwpEntry_t *pZf);
int
sbZfFabBm9600BwR0BwpEntry_SetField(sbZfFabBm9600BwR0BwpEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_BWR0BWPENTRY_CONSOLE_H */
