/*
 * $Id: sbZfFabBm9600BwAllocRateEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600BwAllocRateEntry.hx"
#ifndef SB_ZF_FAB_BM9600_BWALLOCRATEENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_BWALLOCRATEENTRY_CONSOLE_H



void
sbZfFabBm9600BwAllocRateEntry_Print(sbZfFabBm9600BwAllocRateEntry_t *pFromStruct);
char *
sbZfFabBm9600BwAllocRateEntry_SPrint(sbZfFabBm9600BwAllocRateEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600BwAllocRateEntry_Validate(sbZfFabBm9600BwAllocRateEntry_t *pZf);
int
sbZfFabBm9600BwAllocRateEntry_SetField(sbZfFabBm9600BwAllocRateEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_BWALLOCRATEENTRY_CONSOLE_H */
