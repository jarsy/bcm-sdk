/*
 * $Id: sbZfKaEgMemFifoParamEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEgMemFifoParamEntry.hx"
#ifndef SB_ZF_ZFKAEGMEMFIFOPARAMENTRY_CONSOLE_H
#define SB_ZF_ZFKAEGMEMFIFOPARAMENTRY_CONSOLE_H



void
sbZfKaEgMemFifoParamEntry_Print(sbZfKaEgMemFifoParamEntry_t *pFromStruct);
char *
sbZfKaEgMemFifoParamEntry_SPrint(sbZfKaEgMemFifoParamEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEgMemFifoParamEntry_Validate(sbZfKaEgMemFifoParamEntry_t *pZf);
int
sbZfKaEgMemFifoParamEntry_SetField(sbZfKaEgMemFifoParamEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEGMEMFIFOPARAMENTRY_CONSOLE_H */
