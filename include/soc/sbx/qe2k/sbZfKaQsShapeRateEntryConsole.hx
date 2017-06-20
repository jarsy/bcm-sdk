/*
 * $Id: sbZfKaQsShapeRateEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsShapeRateEntry.hx"
#ifndef SB_ZF_ZFKAQSSHAPERATEENTRY_CONSOLE_H
#define SB_ZF_ZFKAQSSHAPERATEENTRY_CONSOLE_H



void
sbZfKaQsShapeRateEntry_Print(sbZfKaQsShapeRateEntry_t *pFromStruct);
char *
sbZfKaQsShapeRateEntry_SPrint(sbZfKaQsShapeRateEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsShapeRateEntry_Validate(sbZfKaQsShapeRateEntry_t *pZf);
int
sbZfKaQsShapeRateEntry_SetField(sbZfKaQsShapeRateEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSSHAPERATEENTRY_CONSOLE_H */
