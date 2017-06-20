/*
 * $Id: sbZfKaEpSlimVlanRecordConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpSlimVlanRecord.hx"
#ifndef SB_ZF_ZFKAEPSLIMVLANRECORD_CONSOLE_H
#define SB_ZF_ZFKAEPSLIMVLANRECORD_CONSOLE_H



void
sbZfKaEpSlimVlanRecord_Print(sbZfKaEpSlimVlanRecord_t *pFromStruct);
char *
sbZfKaEpSlimVlanRecord_SPrint(sbZfKaEpSlimVlanRecord_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpSlimVlanRecord_Validate(sbZfKaEpSlimVlanRecord_t *pZf);
int
sbZfKaEpSlimVlanRecord_SetField(sbZfKaEpSlimVlanRecord_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPSLIMVLANRECORD_CONSOLE_H */
