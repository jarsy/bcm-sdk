/*
 * $Id: sbZfFabBm9600NmFullStatusEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600NmFullStatusEntry.hx"
#ifndef SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_CONSOLE_H



void
sbZfFabBm9600NmFullStatusEntry_Print(sbZfFabBm9600NmFullStatusEntry_t *pFromStruct);
char *
sbZfFabBm9600NmFullStatusEntry_SPrint(sbZfFabBm9600NmFullStatusEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600NmFullStatusEntry_Validate(sbZfFabBm9600NmFullStatusEntry_t *pZf);
int
sbZfFabBm9600NmFullStatusEntry_SetField(sbZfFabBm9600NmFullStatusEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_CONSOLE_H */
