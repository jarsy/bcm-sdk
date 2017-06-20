/*
 * $Id: sbZfFabBm9600BwFetchValidEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600BwFetchValidEntry.hx"
#ifndef SB_ZF_FAB_BM9600_BWFETCHVALIDENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_BWFETCHVALIDENTRY_CONSOLE_H



void
sbZfFabBm9600BwFetchValidEntry_Print(sbZfFabBm9600BwFetchValidEntry_t *pFromStruct);
char *
sbZfFabBm9600BwFetchValidEntry_SPrint(sbZfFabBm9600BwFetchValidEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600BwFetchValidEntry_Validate(sbZfFabBm9600BwFetchValidEntry_t *pZf);
int
sbZfFabBm9600BwFetchValidEntry_SetField(sbZfFabBm9600BwFetchValidEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_BWFETCHVALIDENTRY_CONSOLE_H */
