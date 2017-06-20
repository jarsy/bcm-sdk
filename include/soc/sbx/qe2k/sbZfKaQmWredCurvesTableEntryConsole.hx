/*
 * $Id: sbZfKaQmWredCurvesTableEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQmWredCurvesTableEntry.hx"
#ifndef SB_ZF_ZFKAQMWREDCURVESTABLEENTRY_CONSOLE_H
#define SB_ZF_ZFKAQMWREDCURVESTABLEENTRY_CONSOLE_H



void
sbZfKaQmWredCurvesTableEntry_Print(sbZfKaQmWredCurvesTableEntry_t *pFromStruct);
char *
sbZfKaQmWredCurvesTableEntry_SPrint(sbZfKaQmWredCurvesTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQmWredCurvesTableEntry_Validate(sbZfKaQmWredCurvesTableEntry_t *pZf);
int
sbZfKaQmWredCurvesTableEntry_SetField(sbZfKaQmWredCurvesTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQMWREDCURVESTABLEENTRY_CONSOLE_H */
