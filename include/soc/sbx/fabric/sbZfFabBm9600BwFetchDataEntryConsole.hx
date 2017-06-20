/*
 * $Id: sbZfFabBm9600BwFetchDataEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600BwFetchDataEntry.hx"
#ifndef SB_ZF_FAB_BM9600_BWFETCHDATAENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_BWFETCHDATAENTRY_CONSOLE_H



void
sbZfFabBm9600BwFetchDataEntry_Print(sbZfFabBm9600BwFetchDataEntry_t *pFromStruct);
char *
sbZfFabBm9600BwFetchDataEntry_SPrint(sbZfFabBm9600BwFetchDataEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600BwFetchDataEntry_Validate(sbZfFabBm9600BwFetchDataEntry_t *pZf);
int
sbZfFabBm9600BwFetchDataEntry_SetField(sbZfFabBm9600BwFetchDataEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_BWFETCHDATAENTRY_CONSOLE_H */
