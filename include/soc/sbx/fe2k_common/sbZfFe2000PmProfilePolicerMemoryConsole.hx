/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFe2000PmProfilePolicerMemory.hx"
#ifndef SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_CONSOLE_H
#define SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_CONSOLE_H



void
sbZfFe2000PmProfilePolicerMemory_Print(sbZfFe2000PmProfilePolicerMemory_t *pFromStruct);
char *
sbZfFe2000PmProfilePolicerMemory_SPrint(sbZfFe2000PmProfilePolicerMemory_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFe2000PmProfilePolicerMemory_Validate(sbZfFe2000PmProfilePolicerMemory_t *pZf);
int
sbZfFe2000PmProfilePolicerMemory_SetField(sbZfFe2000PmProfilePolicerMemory_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FE_2000_PM_PROFPOLICERMEMORY_CONSOLE_H */
