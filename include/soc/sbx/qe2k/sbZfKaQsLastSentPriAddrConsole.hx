/*
 * $Id: sbZfKaQsLastSentPriAddrConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsLastSentPriAddr.hx"
#ifndef SB_ZF_ZFKAQSLASTSENTPRIADDR_CONSOLE_H
#define SB_ZF_ZFKAQSLASTSENTPRIADDR_CONSOLE_H



void
sbZfKaQsLastSentPriAddr_Print(sbZfKaQsLastSentPriAddr_t *pFromStruct);
char *
sbZfKaQsLastSentPriAddr_SPrint(sbZfKaQsLastSentPriAddr_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsLastSentPriAddr_Validate(sbZfKaQsLastSentPriAddr_t *pZf);
int
sbZfKaQsLastSentPriAddr_SetField(sbZfKaQsLastSentPriAddr_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSLASTSENTPRIADDR_CONSOLE_H */
