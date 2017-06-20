/*
 * $Id: sbZfKaQsLastSentPriEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsLastSentPriEntry.hx"
#ifndef SB_ZF_ZFKAQSLASTSENTPRIENTRY_CONSOLE_H
#define SB_ZF_ZFKAQSLASTSENTPRIENTRY_CONSOLE_H



void
sbZfKaQsLastSentPriEntry_Print(sbZfKaQsLastSentPriEntry_t *pFromStruct);
char *
sbZfKaQsLastSentPriEntry_SPrint(sbZfKaQsLastSentPriEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsLastSentPriEntry_Validate(sbZfKaQsLastSentPriEntry_t *pZf);
int
sbZfKaQsLastSentPriEntry_SetField(sbZfKaQsLastSentPriEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSLASTSENTPRIENTRY_CONSOLE_H */
