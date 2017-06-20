/*
 * $Id: sbZfFabBm9600BwR1Wct1BEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600BwR1Wct1BEntry.hx"
#ifndef SB_ZF_FAB_BM9600_BWR1WCT1BENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_BWR1WCT1BENTRY_CONSOLE_H



void
sbZfFabBm9600BwR1Wct1BEntry_Print(sbZfFabBm9600BwR1Wct1BEntry_t *pFromStruct);
char *
sbZfFabBm9600BwR1Wct1BEntry_SPrint(sbZfFabBm9600BwR1Wct1BEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600BwR1Wct1BEntry_Validate(sbZfFabBm9600BwR1Wct1BEntry_t *pZf);
int
sbZfFabBm9600BwR1Wct1BEntry_SetField(sbZfFabBm9600BwR1Wct1BEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_BWR1WCT1BENTRY_CONSOLE_H */
