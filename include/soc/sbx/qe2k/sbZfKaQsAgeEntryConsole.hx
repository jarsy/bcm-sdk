/*
 * $Id: sbZfKaQsAgeEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsAgeEntry.hx"
#ifndef SB_ZF_ZFKAQSAGEENTRY_CONSOLE_H
#define SB_ZF_ZFKAQSAGEENTRY_CONSOLE_H



void
sbZfKaQsAgeEntry_Print(sbZfKaQsAgeEntry_t *pFromStruct);
char *
sbZfKaQsAgeEntry_SPrint(sbZfKaQsAgeEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsAgeEntry_Validate(sbZfKaQsAgeEntry_t *pZf);
int
sbZfKaQsAgeEntry_SetField(sbZfKaQsAgeEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSAGEENTRY_CONSOLE_H */
