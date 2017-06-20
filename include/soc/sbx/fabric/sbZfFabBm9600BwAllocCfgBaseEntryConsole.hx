/*
 * $Id: sbZfFabBm9600BwAllocCfgBaseEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600BwAllocCfgBaseEntry.hx"
#ifndef SB_ZF_FAB_BM9600_BWALLOCCFGBASEENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_BWALLOCCFGBASEENTRY_CONSOLE_H



void
sbZfFabBm9600BwAllocCfgBaseEntry_Print(sbZfFabBm9600BwAllocCfgBaseEntry_t *pFromStruct);
char *
sbZfFabBm9600BwAllocCfgBaseEntry_SPrint(sbZfFabBm9600BwAllocCfgBaseEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600BwAllocCfgBaseEntry_Validate(sbZfFabBm9600BwAllocCfgBaseEntry_t *pZf);
int
sbZfFabBm9600BwAllocCfgBaseEntry_SetField(sbZfFabBm9600BwAllocCfgBaseEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_BWALLOCCFGBASEENTRY_CONSOLE_H */
