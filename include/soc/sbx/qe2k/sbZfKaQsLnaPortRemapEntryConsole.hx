/*
 * $Id: sbZfKaQsLnaPortRemapEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsLnaPortRemapEntry.hx"
#ifndef SB_ZF_ZFKAQSPORTREMAPENTRY_CONSOLE_H
#define SB_ZF_ZFKAQSPORTREMAPENTRY_CONSOLE_H



void
sbZfKaQsLnaPortRemapEntry_Print(sbZfKaQsLnaPortRemapEntry_t *pFromStruct);
char *
sbZfKaQsLnaPortRemapEntry_SPrint(sbZfKaQsLnaPortRemapEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsLnaPortRemapEntry_Validate(sbZfKaQsLnaPortRemapEntry_t *pZf);
int
sbZfKaQsLnaPortRemapEntry_SetField(sbZfKaQsLnaPortRemapEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSPORTREMAPENTRY_CONSOLE_H */
