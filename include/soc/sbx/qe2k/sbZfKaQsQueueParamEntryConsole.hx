/*
 * $Id: sbZfKaQsQueueParamEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsQueueParamEntry.hx"
#ifndef SB_ZF_ZFKAQSQUEUEPARAMENTRY_CONSOLE_H
#define SB_ZF_ZFKAQSQUEUEPARAMENTRY_CONSOLE_H



void
sbZfKaQsQueueParamEntry_Print(sbZfKaQsQueueParamEntry_t *pFromStruct);
char *
sbZfKaQsQueueParamEntry_SPrint(sbZfKaQsQueueParamEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsQueueParamEntry_Validate(sbZfKaQsQueueParamEntry_t *pZf);
int
sbZfKaQsQueueParamEntry_SetField(sbZfKaQsQueueParamEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSQUEUEPARAMENTRY_CONSOLE_H */
