/*
 * $Id: sbZfKaEiMemDataEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEiMemDataEntry.hx"
#ifndef SB_ZF_ZFKAEIMEMDATAENTRY_CONSOLE_H
#define SB_ZF_ZFKAEIMEMDATAENTRY_CONSOLE_H



void
sbZfKaEiMemDataEntry_Print(sbZfKaEiMemDataEntry_t *pFromStruct);
char *
sbZfKaEiMemDataEntry_SPrint(sbZfKaEiMemDataEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEiMemDataEntry_Validate(sbZfKaEiMemDataEntry_t *pZf);
int
sbZfKaEiMemDataEntry_SetField(sbZfKaEiMemDataEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEIMEMDATAENTRY_CONSOLE_H */
