/*
 * $Id: sbZfFabBm9600InaSysportMapEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600InaSysportMapEntry.hx"
#ifndef SB_ZF_FAB_BM9600_INASYSPORTMAPENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_INASYSPORTMAPENTRY_CONSOLE_H



void
sbZfFabBm9600InaSysportMapEntry_Print(sbZfFabBm9600InaSysportMapEntry_t *pFromStruct);
char *
sbZfFabBm9600InaSysportMapEntry_SPrint(sbZfFabBm9600InaSysportMapEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600InaSysportMapEntry_Validate(sbZfFabBm9600InaSysportMapEntry_t *pZf);
int
sbZfFabBm9600InaSysportMapEntry_SetField(sbZfFabBm9600InaSysportMapEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_INASYSPORTMAPENTRY_CONSOLE_H */
