/*
 * $Id: sbZfKaQsRateTableEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsRateTableEntry.hx"
#ifndef SB_ZF_ZFKAQSRATETABLEENTRY_CONSOLE_H
#define SB_ZF_ZFKAQSRATETABLEENTRY_CONSOLE_H



void
sbZfKaQsRateTableEntry_Print(sbZfKaQsRateTableEntry_t *pFromStruct);
char *
sbZfKaQsRateTableEntry_SPrint(sbZfKaQsRateTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsRateTableEntry_Validate(sbZfKaQsRateTableEntry_t *pZf);
int
sbZfKaQsRateTableEntry_SetField(sbZfKaQsRateTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSRATETABLEENTRY_CONSOLE_H */
