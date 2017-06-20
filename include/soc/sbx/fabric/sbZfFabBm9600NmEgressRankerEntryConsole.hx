/*
 * $Id: sbZfFabBm9600NmEgressRankerEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600NmEgressRankerEntry.hx"
#ifndef SB_ZF_FAB_BM9600_NMEGRESSRANKERENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_NMEGRESSRANKERENTRY_CONSOLE_H



void
sbZfFabBm9600NmEgressRankerEntry_Print(sbZfFabBm9600NmEgressRankerEntry_t *pFromStruct);
char *
sbZfFabBm9600NmEgressRankerEntry_SPrint(sbZfFabBm9600NmEgressRankerEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600NmEgressRankerEntry_Validate(sbZfFabBm9600NmEgressRankerEntry_t *pZf);
int
sbZfFabBm9600NmEgressRankerEntry_SetField(sbZfFabBm9600NmEgressRankerEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_NMEGRESSRANKERENTRY_CONSOLE_H */
