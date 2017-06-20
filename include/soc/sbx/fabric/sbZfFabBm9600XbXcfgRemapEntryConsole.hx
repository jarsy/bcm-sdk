/*
 * $Id: sbZfFabBm9600XbXcfgRemapEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600XbXcfgRemapEntry.hx"
#ifndef SB_ZF_FAB_BM9600_XBXCFGREMAPENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_XBXCFGREMAPENTRY_CONSOLE_H



void
sbZfFabBm9600XbXcfgRemapEntry_Print(sbZfFabBm9600XbXcfgRemapEntry_t *pFromStruct);
char *
sbZfFabBm9600XbXcfgRemapEntry_SPrint(sbZfFabBm9600XbXcfgRemapEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600XbXcfgRemapEntry_Validate(sbZfFabBm9600XbXcfgRemapEntry_t *pZf);
int
sbZfFabBm9600XbXcfgRemapEntry_SetField(sbZfFabBm9600XbXcfgRemapEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_XBXCFGREMAPENTRY_CONSOLE_H */
