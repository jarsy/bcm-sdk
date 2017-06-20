/*
 * $Id: sbZfFabBm9600FoLinkStateTableEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600FoLinkStateTableEntry.hx"
#ifndef SB_ZF_FAB_BM9600_FOLINKSTATETABLEENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_FOLINKSTATETABLEENTRY_CONSOLE_H



void
sbZfFabBm9600FoLinkStateTableEntry_Print(sbZfFabBm9600FoLinkStateTableEntry_t *pFromStruct);
char *
sbZfFabBm9600FoLinkStateTableEntry_SPrint(sbZfFabBm9600FoLinkStateTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600FoLinkStateTableEntry_Validate(sbZfFabBm9600FoLinkStateTableEntry_t *pZf);
int
sbZfFabBm9600FoLinkStateTableEntry_SetField(sbZfFabBm9600FoLinkStateTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_FOLINKSTATETABLEENTRY_CONSOLE_H */
