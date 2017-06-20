/*
 * $Id: sbZfFabBm9600BwFetchSumEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600BwFetchSumEntry.hx"
#ifndef SB_ZF_FAB_BM9600_BWFETCHSUMENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_BWFETCHSUMENTRY_CONSOLE_H



void
sbZfFabBm9600BwFetchSumEntry_Print(sbZfFabBm9600BwFetchSumEntry_t *pFromStruct);
char *
sbZfFabBm9600BwFetchSumEntry_SPrint(sbZfFabBm9600BwFetchSumEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600BwFetchSumEntry_Validate(sbZfFabBm9600BwFetchSumEntry_t *pZf);
int
sbZfFabBm9600BwFetchSumEntry_SetField(sbZfFabBm9600BwFetchSumEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_BWFETCHSUMENTRY_CONSOLE_H */
