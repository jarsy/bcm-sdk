/*
 * $Id: sbZfKaQsAgeThreshLutEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsAgeThreshLutEntry.hx"
#ifndef SB_ZF_ZFKAQSAGETHRESHLUTENTRY_CONSOLE_H
#define SB_ZF_ZFKAQSAGETHRESHLUTENTRY_CONSOLE_H



void
sbZfKaQsAgeThreshLutEntry_Print(sbZfKaQsAgeThreshLutEntry_t *pFromStruct);
char *
sbZfKaQsAgeThreshLutEntry_SPrint(sbZfKaQsAgeThreshLutEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsAgeThreshLutEntry_Validate(sbZfKaQsAgeThreshLutEntry_t *pZf);
int
sbZfKaQsAgeThreshLutEntry_SetField(sbZfKaQsAgeThreshLutEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSAGETHRESHLUTENTRY_CONSOLE_H */
