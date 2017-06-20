/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfC2PmProfileMemory.hx"
#ifndef SB_ZF_C2_PM_PROFMEMORY_CONSOLE_H
#define SB_ZF_C2_PM_PROFMEMORY_CONSOLE_H



void
sbZfC2PmProfileMemory_Print(sbZfC2PmProfileMemory_t *pFromStruct);
char *
sbZfC2PmProfileMemory_SPrint(sbZfC2PmProfileMemory_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfC2PmProfileMemory_Validate(sbZfC2PmProfileMemory_t *pZf);
int
sbZfC2PmProfileMemory_SetField(sbZfC2PmProfileMemory_t *s, char* name, int value);


#endif /* ifndef SB_ZF_C2_PM_PROFMEMORY_CONSOLE_H */
