/*
 * $Id: sbZfKaEpIpFourBitEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpIpFourBitEntry.hx"
#ifndef SB_ZF_ZKAEPIPFOURBITENTRY_CONSOLE_H
#define SB_ZF_ZKAEPIPFOURBITENTRY_CONSOLE_H



void
sbZfKaEpIpFourBitEntry_Print(sbZfKaEpIpFourBitEntry_t *pFromStruct);
char *
sbZfKaEpIpFourBitEntry_SPrint(sbZfKaEpIpFourBitEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpIpFourBitEntry_Validate(sbZfKaEpIpFourBitEntry_t *pZf);
int
sbZfKaEpIpFourBitEntry_SetField(sbZfKaEpIpFourBitEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZKAEPIPFOURBITENTRY_CONSOLE_H */
