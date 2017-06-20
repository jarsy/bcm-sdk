/*
 * $Id: sbZfKaQsLnaNextPriEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsLnaNextPriEntry.hx"
#ifndef SB_ZF_ZFKAQSLNANEXTPRIENTRY_CONSOLE_H
#define SB_ZF_ZFKAQSLNANEXTPRIENTRY_CONSOLE_H



void
sbZfKaQsLnaNextPriEntry_Print(sbZfKaQsLnaNextPriEntry_t *pFromStruct);
char *
sbZfKaQsLnaNextPriEntry_SPrint(sbZfKaQsLnaNextPriEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsLnaNextPriEntry_Validate(sbZfKaQsLnaNextPriEntry_t *pZf);
int
sbZfKaQsLnaNextPriEntry_SetField(sbZfKaQsLnaNextPriEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSLNANEXTPRIENTRY_CONSOLE_H */
