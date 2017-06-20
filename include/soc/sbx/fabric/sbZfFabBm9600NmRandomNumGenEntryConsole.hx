/*
 * $Id: sbZfFabBm9600NmRandomNumGenEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600NmRandomNumGenEntry.hx"
#ifndef SB_ZF_FAB_BM9600_NMRANDOMNUMGENENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_NMRANDOMNUMGENENTRY_CONSOLE_H



void
sbZfFabBm9600NmRandomNumGenEntry_Print(sbZfFabBm9600NmRandomNumGenEntry_t *pFromStruct);
char *
sbZfFabBm9600NmRandomNumGenEntry_SPrint(sbZfFabBm9600NmRandomNumGenEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600NmRandomNumGenEntry_Validate(sbZfFabBm9600NmRandomNumGenEntry_t *pZf);
int
sbZfFabBm9600NmRandomNumGenEntry_SetField(sbZfFabBm9600NmRandomNumGenEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_NMRANDOMNUMGENENTRY_CONSOLE_H */
