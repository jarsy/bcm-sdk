/*
 * $Id: sbZfKaQmSlqCntrsEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQmSlqCntrsEntry.hx"
#ifndef SB_ZF_ZFKAQMSLQCNTRSENTRY_CONSOLE_H
#define SB_ZF_ZFKAQMSLQCNTRSENTRY_CONSOLE_H



void
sbZfKaQmSlqCntrsEntry_Print(sbZfKaQmSlqCntrsEntry_t *pFromStruct);
char *
sbZfKaQmSlqCntrsEntry_SPrint(sbZfKaQmSlqCntrsEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQmSlqCntrsEntry_Validate(sbZfKaQmSlqCntrsEntry_t *pZf);
int
sbZfKaQmSlqCntrsEntry_SetField(sbZfKaQmSlqCntrsEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQMSLQCNTRSENTRY_CONSOLE_H */
