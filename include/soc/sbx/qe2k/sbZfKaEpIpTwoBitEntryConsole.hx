/*
 * $Id: sbZfKaEpIpTwoBitEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpIpTwoBitEntry.hx"
#ifndef SB_ZF_ZKAEPIPTWOBITENTRY_CONSOLE_H
#define SB_ZF_ZKAEPIPTWOBITENTRY_CONSOLE_H



void
sbZfKaEpIpTwoBitEntry_Print(sbZfKaEpIpTwoBitEntry_t *pFromStruct);
char *
sbZfKaEpIpTwoBitEntry_SPrint(sbZfKaEpIpTwoBitEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpIpTwoBitEntry_Validate(sbZfKaEpIpTwoBitEntry_t *pZf);
int
sbZfKaEpIpTwoBitEntry_SetField(sbZfKaEpIpTwoBitEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZKAEPIPTWOBITENTRY_CONSOLE_H */
