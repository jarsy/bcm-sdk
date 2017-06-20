/*
 * $Id: sbZfKaEgMemShapingEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEgMemShapingEntry.hx"
#ifndef SB_ZF_ZFKAEGMEMSHAPINGENTRY_CONSOLE_H
#define SB_ZF_ZFKAEGMEMSHAPINGENTRY_CONSOLE_H



void
sbZfKaEgMemShapingEntry_Print(sbZfKaEgMemShapingEntry_t *pFromStruct);
char *
sbZfKaEgMemShapingEntry_SPrint(sbZfKaEgMemShapingEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEgMemShapingEntry_Validate(sbZfKaEgMemShapingEntry_t *pZf);
int
sbZfKaEgMemShapingEntry_SetField(sbZfKaEgMemShapingEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEGMEMSHAPINGENTRY_CONSOLE_H */
