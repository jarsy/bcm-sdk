/*
 * $Id: sbZfKaQmQueueArrivalsEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQmQueueArrivalsEntry.hx"
#ifndef SB_ZF_ZFKAQMQUEUEARRIVALSENTRY_CONSOLE_H
#define SB_ZF_ZFKAQMQUEUEARRIVALSENTRY_CONSOLE_H



void
sbZfKaQmQueueArrivalsEntry_Print(sbZfKaQmQueueArrivalsEntry_t *pFromStruct);
char *
sbZfKaQmQueueArrivalsEntry_SPrint(sbZfKaQmQueueArrivalsEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQmQueueArrivalsEntry_Validate(sbZfKaQmQueueArrivalsEntry_t *pZf);
int
sbZfKaQmQueueArrivalsEntry_SetField(sbZfKaQmQueueArrivalsEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQMQUEUEARRIVALSENTRY_CONSOLE_H */
