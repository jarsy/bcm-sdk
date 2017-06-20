/*
 * $Id: sbZfKaQmBaaCfgTableEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQmBaaCfgTableEntry.hx"
#ifndef SB_ZF_ZFKAQMBAACFGTABLEENTRY_CONSOLE_H
#define SB_ZF_ZFKAQMBAACFGTABLEENTRY_CONSOLE_H



void
sbZfKaQmBaaCfgTableEntry_Print(sbZfKaQmBaaCfgTableEntry_t *pFromStruct);
char *
sbZfKaQmBaaCfgTableEntry_SPrint(sbZfKaQmBaaCfgTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQmBaaCfgTableEntry_Validate(sbZfKaQmBaaCfgTableEntry_t *pZf);
int
sbZfKaQmBaaCfgTableEntry_SetField(sbZfKaQmBaaCfgTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQMBAACFGTABLEENTRY_CONSOLE_H */
