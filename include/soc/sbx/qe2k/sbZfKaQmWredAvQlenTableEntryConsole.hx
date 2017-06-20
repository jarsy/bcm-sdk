/*
 * $Id: sbZfKaQmWredAvQlenTableEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQmWredAvQlenTableEntry.hx"
#ifndef SB_ZF_ZFKAQMWREDAVQLENTABLEENTRY_CONSOLE_H
#define SB_ZF_ZFKAQMWREDAVQLENTABLEENTRY_CONSOLE_H



void
sbZfKaQmWredAvQlenTableEntry_Print(sbZfKaQmWredAvQlenTableEntry_t *pFromStruct);
char *
sbZfKaQmWredAvQlenTableEntry_SPrint(sbZfKaQmWredAvQlenTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQmWredAvQlenTableEntry_Validate(sbZfKaQmWredAvQlenTableEntry_t *pZf);
int
sbZfKaQmWredAvQlenTableEntry_SetField(sbZfKaQmWredAvQlenTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQMWREDAVQLENTABLEENTRY_CONSOLE_H */
