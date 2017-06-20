/*
 * $Id: sbZfKaEgMemFifoControlEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEgMemFifoControlEntry.hx"
#ifndef SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_CONSOLE_H
#define SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_CONSOLE_H



void
sbZfKaEgMemFifoControlEntry_Print(sbZfKaEgMemFifoControlEntry_t *pFromStruct);
char *
sbZfKaEgMemFifoControlEntry_SPrint(sbZfKaEgMemFifoControlEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEgMemFifoControlEntry_Validate(sbZfKaEgMemFifoControlEntry_t *pZf);
int
sbZfKaEgMemFifoControlEntry_SetField(sbZfKaEgMemFifoControlEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEGMEMFIFOCONTRILENTRY_CONSOLE_H */
