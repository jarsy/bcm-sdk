/*
 * $Id: sbZfKaQsQ2EcEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsQ2EcEntry.hx"
#ifndef SB_ZF_ZFKAQSQ2ECENTRY_CONSOLE_H
#define SB_ZF_ZFKAQSQ2ECENTRY_CONSOLE_H



void
sbZfKaQsQ2EcEntry_Print(sbZfKaQsQ2EcEntry_t *pFromStruct);
char *
sbZfKaQsQ2EcEntry_SPrint(sbZfKaQsQ2EcEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsQ2EcEntry_Validate(sbZfKaQsQ2EcEntry_t *pZf);
int
sbZfKaQsQ2EcEntry_SetField(sbZfKaQsQ2EcEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSQ2ECENTRY_CONSOLE_H */
