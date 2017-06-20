/*
 * $Id: sbZfKaQsShapeMaxBurstEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsShapeMaxBurstEntry.hx"
#ifndef SB_ZF_ZFKAQSSHAPEMAXBURSTENTRY_CONSOLE_H
#define SB_ZF_ZFKAQSSHAPEMAXBURSTENTRY_CONSOLE_H



void
sbZfKaQsShapeMaxBurstEntry_Print(sbZfKaQsShapeMaxBurstEntry_t *pFromStruct);
char *
sbZfKaQsShapeMaxBurstEntry_SPrint(sbZfKaQsShapeMaxBurstEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsShapeMaxBurstEntry_Validate(sbZfKaQsShapeMaxBurstEntry_t *pZf);
int
sbZfKaQsShapeMaxBurstEntry_SetField(sbZfKaQsShapeMaxBurstEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSSHAPEMAXBURSTENTRY_CONSOLE_H */
