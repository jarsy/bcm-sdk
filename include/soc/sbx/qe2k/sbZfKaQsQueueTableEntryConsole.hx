/*
 * $Id: sbZfKaQsQueueTableEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsQueueTableEntry.hx"
#ifndef SB_ZF_ZFKAQSQUEUETABLEENTRY_CONSOLE_H
#define SB_ZF_ZFKAQSQUEUETABLEENTRY_CONSOLE_H



void
sbZfKaQsQueueTableEntry_Print(sbZfKaQsQueueTableEntry_t *pFromStruct);
char *
sbZfKaQsQueueTableEntry_SPrint(sbZfKaQsQueueTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsQueueTableEntry_Validate(sbZfKaQsQueueTableEntry_t *pZf);
int
sbZfKaQsQueueTableEntry_SetField(sbZfKaQsQueueTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSQUEUETABLEENTRY_CONSOLE_H */
