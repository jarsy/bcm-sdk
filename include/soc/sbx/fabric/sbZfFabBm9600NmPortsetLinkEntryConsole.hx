/*
 * $Id: sbZfFabBm9600NmPortsetLinkEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600NmPortsetLinkEntry.hx"
#ifndef SB_ZF_FAB_BM9600_NMPORTSETLINKENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_NMPORTSETLINKENTRY_CONSOLE_H



void
sbZfFabBm9600NmPortsetLinkEntry_Print(sbZfFabBm9600NmPortsetLinkEntry_t *pFromStruct);
char *
sbZfFabBm9600NmPortsetLinkEntry_SPrint(sbZfFabBm9600NmPortsetLinkEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600NmPortsetLinkEntry_Validate(sbZfFabBm9600NmPortsetLinkEntry_t *pZf);
int
sbZfFabBm9600NmPortsetLinkEntry_SetField(sbZfFabBm9600NmPortsetLinkEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_NMPORTSETLINKENTRY_CONSOLE_H */
