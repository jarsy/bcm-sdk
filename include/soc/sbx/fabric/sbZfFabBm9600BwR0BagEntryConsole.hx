/*
 * $Id: sbZfFabBm9600BwR0BagEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600BwR0BagEntry.hx"
#ifndef SB_ZF_FAB_BM9600_BWR0BAGENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_BWR0BAGENTRY_CONSOLE_H



void
sbZfFabBm9600BwR0BagEntry_Print(sbZfFabBm9600BwR0BagEntry_t *pFromStruct);
char *
sbZfFabBm9600BwR0BagEntry_SPrint(sbZfFabBm9600BwR0BagEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600BwR0BagEntry_Validate(sbZfFabBm9600BwR0BagEntry_t *pZf);
int
sbZfFabBm9600BwR0BagEntry_SetField(sbZfFabBm9600BwR0BagEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_BWR0BAGENTRY_CONSOLE_H */
