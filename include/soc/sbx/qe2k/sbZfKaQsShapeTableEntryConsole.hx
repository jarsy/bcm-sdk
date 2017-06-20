/*
 * $Id: sbZfKaQsShapeTableEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsShapeTableEntry.hx"
#ifndef SB_ZF_ZFKAQSSHAPETABLEENTRY_CONSOLE_H
#define SB_ZF_ZFKAQSSHAPETABLEENTRY_CONSOLE_H



void
sbZfKaQsShapeTableEntry_Print(sbZfKaQsShapeTableEntry_t *pFromStruct);
char *
sbZfKaQsShapeTableEntry_SPrint(sbZfKaQsShapeTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsShapeTableEntry_Validate(sbZfKaQsShapeTableEntry_t *pZf);
int
sbZfKaQsShapeTableEntry_SetField(sbZfKaQsShapeTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSSHAPETABLEENTRY_CONSOLE_H */
