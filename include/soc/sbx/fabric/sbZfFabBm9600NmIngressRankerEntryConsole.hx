/*
 * $Id: sbZfFabBm9600NmIngressRankerEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFabBm9600NmIngressRankerEntry.hx"
#ifndef SB_ZF_FAB_BM9600_NMINGRESSRANKERENTRY_CONSOLE_H
#define SB_ZF_FAB_BM9600_NMINGRESSRANKERENTRY_CONSOLE_H



void
sbZfFabBm9600NmIngressRankerEntry_Print(sbZfFabBm9600NmIngressRankerEntry_t *pFromStruct);
char *
sbZfFabBm9600NmIngressRankerEntry_SPrint(sbZfFabBm9600NmIngressRankerEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFabBm9600NmIngressRankerEntry_Validate(sbZfFabBm9600NmIngressRankerEntry_t *pZf);
int
sbZfFabBm9600NmIngressRankerEntry_SetField(sbZfFabBm9600NmIngressRankerEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FAB_BM9600_NMINGRESSRANKERENTRY_CONSOLE_H */
