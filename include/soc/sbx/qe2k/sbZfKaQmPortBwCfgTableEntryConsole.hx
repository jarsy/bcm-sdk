/*
 * $Id: sbZfKaQmPortBwCfgTableEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQmPortBwCfgTableEntry.hx"
#ifndef SB_ZF_ZFKAQMPORTBWCFGTABLEENTRY_CONSOLE_H
#define SB_ZF_ZFKAQMPORTBWCFGTABLEENTRY_CONSOLE_H



void
sbZfKaQmPortBwCfgTableEntry_Print(sbZfKaQmPortBwCfgTableEntry_t *pFromStruct);
char *
sbZfKaQmPortBwCfgTableEntry_SPrint(sbZfKaQmPortBwCfgTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQmPortBwCfgTableEntry_Validate(sbZfKaQmPortBwCfgTableEntry_t *pZf);
int
sbZfKaQmPortBwCfgTableEntry_SetField(sbZfKaQmPortBwCfgTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQMPORTBWCFGTABLEENTRY_CONSOLE_H */
