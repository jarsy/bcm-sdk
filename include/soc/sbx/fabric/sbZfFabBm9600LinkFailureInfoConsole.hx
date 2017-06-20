/*
 * $Id: sbZfFabBm9600LinkFailureInfoConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600LinkFailureInfo.hx"
#ifndef SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_CONSOLE_H
#define SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_CONSOLE_H



void
sbZfFabBm9600LinkFailureInfo_Print(sbZfFabBm9600LinkFailureInfo_t *pFromStruct);
char *
sbZfFabBm9600LinkFailureInfo_SPrint(sbZfFabBm9600LinkFailureInfo_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600LinkFailureInfo_Validate(sbZfFabBm9600LinkFailureInfo_t *pZf);
int
sbZfFabBm9600LinkFailureInfo_SetField(sbZfFabBm9600LinkFailureInfo_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_FOLINKFAILUREINFO_CONSOLE_H */
