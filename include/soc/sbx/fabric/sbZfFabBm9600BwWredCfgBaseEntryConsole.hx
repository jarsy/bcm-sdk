/*
 * $Id: sbZfFabBm9600BwWredCfgBaseEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600BwWredCfgBaseEntry.hx"
#ifndef SB_ZF_FAB_BM9600_BWWREDCFGBASEENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_BWWREDCFGBASEENTRY_CONSOLE_H



void
sbZfFabBm9600BwWredCfgBaseEntry_Print(sbZfFabBm9600BwWredCfgBaseEntry_t *pFromStruct);
char *
sbZfFabBm9600BwWredCfgBaseEntry_SPrint(sbZfFabBm9600BwWredCfgBaseEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600BwWredCfgBaseEntry_Validate(sbZfFabBm9600BwWredCfgBaseEntry_t *pZf);
int
sbZfFabBm9600BwWredCfgBaseEntry_SetField(sbZfFabBm9600BwWredCfgBaseEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_BWWREDCFGBASEENTRY_CONSOLE_H */
