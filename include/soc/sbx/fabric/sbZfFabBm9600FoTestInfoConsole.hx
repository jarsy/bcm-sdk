/*
 * $Id: sbZfFabBm9600FoTestInfoConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600FoTestInfo.hx"
#ifndef SB_ZF_FAB_BM9600_FOTESTINFO_CONSOLE_H
#define SB_ZF_FAB_BM9600_FOTESTINFO_CONSOLE_H



void
sbZfFabBm9600FoTestInfo_Print(sbZfFabBm9600FoTestInfo_t *pFromStruct);
char *
sbZfFabBm9600FoTestInfo_SPrint(sbZfFabBm9600FoTestInfo_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600FoTestInfo_Validate(sbZfFabBm9600FoTestInfo_t *pZf);
int
sbZfFabBm9600FoTestInfo_SetField(sbZfFabBm9600FoTestInfo_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_FOTESTINFO_CONSOLE_H */
