/*
 * $Id: sbZfKaEpIpV6TciConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpIpV6Tci.hx"
#ifndef SB_ZF_ZFKAEPIPV6TCI_CONSOLE_H
#define SB_ZF_ZFKAEPIPV6TCI_CONSOLE_H



void
sbZfKaEpIpV6Tci_Print(sbZfKaEpIpV6Tci_t *pFromStruct);
char *
sbZfKaEpIpV6Tci_SPrint(sbZfKaEpIpV6Tci_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpIpV6Tci_Validate(sbZfKaEpIpV6Tci_t *pZf);
int
sbZfKaEpIpV6Tci_SetField(sbZfKaEpIpV6Tci_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPIPV6TCI_CONSOLE_H */
