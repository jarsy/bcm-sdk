/*
 * $Id: sbZfKaQmQueueAgeEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQmQueueAgeEntry.hx"
#ifndef SB_ZF_ZFKAQMQUEUEAGEENTRY_CONSOLE_H
#define SB_ZF_ZFKAQMQUEUEAGEENTRY_CONSOLE_H



void
sbZfKaQmQueueAgeEntry_Print(sbZfKaQmQueueAgeEntry_t *pFromStruct);
char *
sbZfKaQmQueueAgeEntry_SPrint(sbZfKaQmQueueAgeEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQmQueueAgeEntry_Validate(sbZfKaQmQueueAgeEntry_t *pZf);
int
sbZfKaQmQueueAgeEntry_SetField(sbZfKaQmQueueAgeEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQMQUEUEAGEENTRY_CONSOLE_H */
