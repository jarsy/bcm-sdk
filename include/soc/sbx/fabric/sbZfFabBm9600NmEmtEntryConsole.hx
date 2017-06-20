/*
 * $Id: sbZfFabBm9600NmEmtEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600NmEmtEntry.hx"
#ifndef SB_ZF_FAB_BM9600_NMEMTENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_NMEMTENTRY_CONSOLE_H



void
sbZfFabBm9600NmEmtEntry_Print(sbZfFabBm9600NmEmtEntry_t *pFromStruct);
char *
sbZfFabBm9600NmEmtEntry_SPrint(sbZfFabBm9600NmEmtEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600NmEmtEntry_Validate(sbZfFabBm9600NmEmtEntry_t *pZf);
int
sbZfFabBm9600NmEmtEntry_SetField(sbZfFabBm9600NmEmtEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_NMEMTENTRY_CONSOLE_H */
