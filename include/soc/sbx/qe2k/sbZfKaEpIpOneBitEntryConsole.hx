/*
 * $Id: sbZfKaEpIpOneBitEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpIpOneBitEntry.hx"
#ifndef SB_ZF_ZKAEPIPONEBITENTRY_CONSOLE_H
#define SB_ZF_ZKAEPIPONEBITENTRY_CONSOLE_H



void
sbZfKaEpIpOneBitEntry_Print(sbZfKaEpIpOneBitEntry_t *pFromStruct);
char *
sbZfKaEpIpOneBitEntry_SPrint(sbZfKaEpIpOneBitEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpIpOneBitEntry_Validate(sbZfKaEpIpOneBitEntry_t *pZf);
int
sbZfKaEpIpOneBitEntry_SetField(sbZfKaEpIpOneBitEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZKAEPIPONEBITENTRY_CONSOLE_H */
