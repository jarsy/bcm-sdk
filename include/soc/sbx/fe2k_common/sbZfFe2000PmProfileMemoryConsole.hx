/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfFe2000PmProfileMemory.hx"
#ifndef SB_ZF_FE_2000_PM_PROFMEMORY_CONSOLE_H
#define SB_ZF_FE_2000_PM_PROFMEMORY_CONSOLE_H



void
sbZfFe2000PmProfileMemory_Print(sbZfFe2000PmProfileMemory_t *pFromStruct);
char *
sbZfFe2000PmProfileMemory_SPrint(sbZfFe2000PmProfileMemory_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfFe2000PmProfileMemory_Validate(sbZfFe2000PmProfileMemory_t *pZf);
int
sbZfFe2000PmProfileMemory_SetField(sbZfFe2000PmProfileMemory_t *s, char* name, int value);


#endif /* ifndef SB_ZF_FE_2000_PM_PROFMEMORY_CONSOLE_H */
