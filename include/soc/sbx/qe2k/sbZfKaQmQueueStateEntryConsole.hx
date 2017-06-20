/*
 * $Id: sbZfKaQmQueueStateEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQmQueueStateEntry.hx"
#ifndef SB_ZF_ZFKAQMQUEUESTATEENTRY_CONSOLE_H
#define SB_ZF_ZFKAQMQUEUESTATEENTRY_CONSOLE_H



void
sbZfKaQmQueueStateEntry_Print(sbZfKaQmQueueStateEntry_t *pFromStruct);
char *
sbZfKaQmQueueStateEntry_SPrint(sbZfKaQmQueueStateEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQmQueueStateEntry_Validate(sbZfKaQmQueueStateEntry_t *pZf);
int
sbZfKaQmQueueStateEntry_SetField(sbZfKaQmQueueStateEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQMQUEUESTATEENTRY_CONSOLE_H */
