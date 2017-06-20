/*
 * $Id: sbZfFabBm9600NmPortsetInfoEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600NmPortsetInfoEntry.hx"
#ifndef SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_CONSOLE_H



void
sbZfFabBm9600NmPortsetInfoEntry_Print(sbZfFabBm9600NmPortsetInfoEntry_t *pFromStruct);
char *
sbZfFabBm9600NmPortsetInfoEntry_SPrint(sbZfFabBm9600NmPortsetInfoEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600NmPortsetInfoEntry_Validate(sbZfFabBm9600NmPortsetInfoEntry_t *pZf);
int
sbZfFabBm9600NmPortsetInfoEntry_SetField(sbZfFabBm9600NmPortsetInfoEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_NMPORTSETINFOENTRY_CONSOLE_H */
