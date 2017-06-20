/*
 * $Id: sbZfKaQmWredCfgTableEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQmWredCfgTableEntry.hx"
#ifndef SB_ZF_ZFKAQMWREDCFGTABLEENTRY_CONSOLE_H
#define SB_ZF_ZFKAQMWREDCFGTABLEENTRY_CONSOLE_H



void
sbZfKaQmWredCfgTableEntry_Print(sbZfKaQmWredCfgTableEntry_t *pFromStruct);
char *
sbZfKaQmWredCfgTableEntry_SPrint(sbZfKaQmWredCfgTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQmWredCfgTableEntry_Validate(sbZfKaQmWredCfgTableEntry_t *pZf);
int
sbZfKaQmWredCfgTableEntry_SetField(sbZfKaQmWredCfgTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQMWREDCFGTABLEENTRY_CONSOLE_H */
