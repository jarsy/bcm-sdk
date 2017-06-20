/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfC2PmProfileTimerMemory.hx"
#ifndef SB_ZF_C2_PM_PROFTIMERMEMORY_CONSOLE_H
#define SB_ZF_C2_PM_PROFTIMERMEMORY_CONSOLE_H



void
sbZfC2PmProfileTimerMemory_Print(sbZfC2PmProfileTimerMemory_t *pFromStruct);
char *
sbZfC2PmProfileTimerMemory_SPrint(sbZfC2PmProfileTimerMemory_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfC2PmProfileTimerMemory_Validate(sbZfC2PmProfileTimerMemory_t *pZf);
int
sbZfC2PmProfileTimerMemory_SetField(sbZfC2PmProfileTimerMemory_t *s, char* name, int value);


#endif /* ifndef SB_ZF_C2_PM_PROFTIMERMEMORY_CONSOLE_H */
