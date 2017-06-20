/*
 * $Id: sbZfFabBm9600InaPortPriEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600InaPortPriEntry.hx"
#ifndef SB_ZF_FAB_BM9600_INAPORTPRIENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_INAPORTPRIENTRY_CONSOLE_H



void
sbZfFabBm9600InaPortPriEntry_Print(sbZfFabBm9600InaPortPriEntry_t *pFromStruct);
char *
sbZfFabBm9600InaPortPriEntry_SPrint(sbZfFabBm9600InaPortPriEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600InaPortPriEntry_Validate(sbZfFabBm9600InaPortPriEntry_t *pZf);
int
sbZfFabBm9600InaPortPriEntry_SetField(sbZfFabBm9600InaPortPriEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_INAPORTPRIENTRY_CONSOLE_H */
