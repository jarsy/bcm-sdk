/*
 * $Id: sbZfFabBm9600InaRandomNumGenEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600InaRandomNumGenEntry.hx"
#ifndef SB_ZF_FAB_BM9600_INARANDOMNUMGENENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_INARANDOMNUMGENENTRY_CONSOLE_H



void
sbZfFabBm9600InaRandomNumGenEntry_Print(sbZfFabBm9600InaRandomNumGenEntry_t *pFromStruct);
char *
sbZfFabBm9600InaRandomNumGenEntry_SPrint(sbZfFabBm9600InaRandomNumGenEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600InaRandomNumGenEntry_Validate(sbZfFabBm9600InaRandomNumGenEntry_t *pZf);
int
sbZfFabBm9600InaRandomNumGenEntry_SetField(sbZfFabBm9600InaRandomNumGenEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_INARANDOMNUMGENENTRY_CONSOLE_H */
