/*
 * $Id: sbZfKaQmRateDeltaMaxTableEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQmRateDeltaMaxTableEntry.hx"
#ifndef SB_ZF_ZFKAQMRATEDELTAMAXTABLEENTRY_CONSOLE_H
#define SB_ZF_ZFKAQMRATEDELTAMAXTABLEENTRY_CONSOLE_H



void
sbZfKaQmRateDeltaMaxTableEntry_Print(sbZfKaQmRateDeltaMaxTableEntry_t *pFromStruct);
char *
sbZfKaQmRateDeltaMaxTableEntry_SPrint(sbZfKaQmRateDeltaMaxTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQmRateDeltaMaxTableEntry_Validate(sbZfKaQmRateDeltaMaxTableEntry_t *pZf);
int
sbZfKaQmRateDeltaMaxTableEntry_SetField(sbZfKaQmRateDeltaMaxTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQMRATEDELTAMAXTABLEENTRY_CONSOLE_H */
