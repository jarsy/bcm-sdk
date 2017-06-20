/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfC2PmProfilePolicerMemory.hx"
#ifndef SB_ZF_C2_PM_PROFPOLICERMEMORY_CONSOLE_H
#define SB_ZF_C2_PM_PROFPOLICERMEMORY_CONSOLE_H



void
sbZfC2PmProfilePolicerMemory_Print(sbZfC2PmProfilePolicerMemory_t *pFromStruct);
char *
sbZfC2PmProfilePolicerMemory_SPrint(sbZfC2PmProfilePolicerMemory_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfC2PmProfilePolicerMemory_Validate(sbZfC2PmProfilePolicerMemory_t *pZf);
int
sbZfC2PmProfilePolicerMemory_SetField(sbZfC2PmProfilePolicerMemory_t *s, char* name, int value);


#endif /* ifndef SB_ZF_C2_PM_PROFPOLICERMEMORY_CONSOLE_H */
