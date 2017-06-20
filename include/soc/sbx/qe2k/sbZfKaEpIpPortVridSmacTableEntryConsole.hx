/*
 * $Id: sbZfKaEpIpPortVridSmacTableEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpIpPortVridSmacTableEntry.hx"
#ifndef SB_ZF_ZFKAEPIP_PORT_VRID_SMAC_TABLEENTRY_CONSOLE_H
#define SB_ZF_ZFKAEPIP_PORT_VRID_SMAC_TABLEENTRY_CONSOLE_H



void
sbZfKaEpIpPortVridSmacTableEntry_Print(sbZfKaEpIpPortVridSmacTableEntry_t *pFromStruct);
char *
sbZfKaEpIpPortVridSmacTableEntry_SPrint(sbZfKaEpIpPortVridSmacTableEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpIpPortVridSmacTableEntry_Validate(sbZfKaEpIpPortVridSmacTableEntry_t *pZf);
int
sbZfKaEpIpPortVridSmacTableEntry_SetField(sbZfKaEpIpPortVridSmacTableEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPIP_PORT_VRID_SMAC_TABLEENTRY_CONSOLE_H */
