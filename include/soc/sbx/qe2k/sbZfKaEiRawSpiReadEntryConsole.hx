/*
 * $Id: sbZfKaEiRawSpiReadEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEiRawSpiReadEntry.hx"
#ifndef SB_ZF_ZFKAEIRAWSPIREADENTRY_CONSOLE_H
#define SB_ZF_ZFKAEIRAWSPIREADENTRY_CONSOLE_H



void
sbZfKaEiRawSpiReadEntry_Print(sbZfKaEiRawSpiReadEntry_t *pFromStruct);
char *
sbZfKaEiRawSpiReadEntry_SPrint(sbZfKaEiRawSpiReadEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEiRawSpiReadEntry_Validate(sbZfKaEiRawSpiReadEntry_t *pZf);
int
sbZfKaEiRawSpiReadEntry_SetField(sbZfKaEiRawSpiReadEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEIRAWSPIREADENTRY_CONSOLE_H */
