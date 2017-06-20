/*
 * $Id: sbZfFabBm9600InaEsetPriEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600InaEsetPriEntry.hx"
#ifndef SB_ZF_FAB_BM9600_INAESETPRIENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_INAESETPRIENTRY_CONSOLE_H



void
sbZfFabBm9600InaEsetPriEntry_Print(sbZfFabBm9600InaEsetPriEntry_t *pFromStruct);
char *
sbZfFabBm9600InaEsetPriEntry_SPrint(sbZfFabBm9600InaEsetPriEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600InaEsetPriEntry_Validate(sbZfFabBm9600InaEsetPriEntry_t *pZf);
int
sbZfFabBm9600InaEsetPriEntry_SetField(sbZfFabBm9600InaEsetPriEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_INAESETPRIENTRY_CONSOLE_H */
