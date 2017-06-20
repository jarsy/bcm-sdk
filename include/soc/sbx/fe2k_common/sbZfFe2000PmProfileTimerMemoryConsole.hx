/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFe2000PmProfileTimerMemory.hx"
#ifndef SB_ZF_FE_2000_PM_PROFTIMERMEMORY_CONSOLE_H
#define SB_ZF_FE_2000_PM_PROFTIMERMEMORY_CONSOLE_H



void
sbZfFe2000PmProfileTimerMemory_Print(sbZfFe2000PmProfileTimerMemory_t *pFromStruct);
char *
sbZfFe2000PmProfileTimerMemory_SPrint(sbZfFe2000PmProfileTimerMemory_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFe2000PmProfileTimerMemory_Validate(sbZfFe2000PmProfileTimerMemory_t *pZf);
int
sbZfFe2000PmProfileTimerMemory_SetField(sbZfFe2000PmProfileTimerMemory_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FE_2000_PM_PROFTIMERMEMORY_CONSOLE_H */
