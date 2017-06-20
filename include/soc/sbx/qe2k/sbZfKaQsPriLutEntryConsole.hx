/*
 * $Id: sbZfKaQsPriLutEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsPriLutEntry.hx"
#ifndef SB_ZF_ZFKAQSPRILUTENTRY_CONSOLE_H
#define SB_ZF_ZFKAQSPRILUTENTRY_CONSOLE_H



void
sbZfKaQsPriLutEntry_Print(sbZfKaQsPriLutEntry_t *pFromStruct);
char *
sbZfKaQsPriLutEntry_SPrint(sbZfKaQsPriLutEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsPriLutEntry_Validate(sbZfKaQsPriLutEntry_t *pZf);
int
sbZfKaQsPriLutEntry_SetField(sbZfKaQsPriLutEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSPRILUTENTRY_CONSOLE_H */
