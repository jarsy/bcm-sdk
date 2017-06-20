/*
 * $Id: sbZfKaQmWredParamEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQmWredParamEntry.hx"
#ifndef SB_ZF_ZFKAQMWREDPARAMENTRY_CONSOLE_H
#define SB_ZF_ZFKAQMWREDPARAMENTRY_CONSOLE_H



void
sbZfKaQmWredParamEntry_Print(sbZfKaQmWredParamEntry_t *pFromStruct);
char *
sbZfKaQmWredParamEntry_SPrint(sbZfKaQmWredParamEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQmWredParamEntry_Validate(sbZfKaQmWredParamEntry_t *pZf);
int
sbZfKaQmWredParamEntry_SetField(sbZfKaQmWredParamEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQMWREDPARAMENTRY_CONSOLE_H */
