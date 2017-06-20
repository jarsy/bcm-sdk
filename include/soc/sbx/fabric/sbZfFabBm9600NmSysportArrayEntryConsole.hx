/*
 * $Id: sbZfFabBm9600NmSysportArrayEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600NmSysportArrayEntry.hx"
#ifndef SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_CONSOLE_H



void
sbZfFabBm9600NmSysportArrayEntry_Print(sbZfFabBm9600NmSysportArrayEntry_t *pFromStruct);
char *
sbZfFabBm9600NmSysportArrayEntry_SPrint(sbZfFabBm9600NmSysportArrayEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600NmSysportArrayEntry_Validate(sbZfFabBm9600NmSysportArrayEntry_t *pZf);
int
sbZfFabBm9600NmSysportArrayEntry_SetField(sbZfFabBm9600NmSysportArrayEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_NMSYSPORTARRAYENTRY_CONSOLE_H */
