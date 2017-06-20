/*
 * $Id: sbZfKaQmQueueByteAdjEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQmQueueByteAdjEntry.hx"
#ifndef SB_ZF_ZFKAQMQUEUEBYTEADJENTRY_CONSOLE_H
#define SB_ZF_ZFKAQMQUEUEBYTEADJENTRY_CONSOLE_H



void
sbZfKaQmQueueByteAdjEntry_Print(sbZfKaQmQueueByteAdjEntry_t *pFromStruct);
char *
sbZfKaQmQueueByteAdjEntry_SPrint(sbZfKaQmQueueByteAdjEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQmQueueByteAdjEntry_Validate(sbZfKaQmQueueByteAdjEntry_t *pZf);
int
sbZfKaQmQueueByteAdjEntry_SetField(sbZfKaQmQueueByteAdjEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQMQUEUEBYTEADJENTRY_CONSOLE_H */
