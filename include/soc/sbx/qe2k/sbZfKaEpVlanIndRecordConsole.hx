/*
 * $Id: sbZfKaEpVlanIndRecordConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpVlanIndRecord.hx"
#ifndef SB_ZF_ZFKAEPVLANINDRECORD_CONSOLE_H
#define SB_ZF_ZFKAEPVLANINDRECORD_CONSOLE_H



void
sbZfKaEpVlanIndRecord_Print(sbZfKaEpVlanIndRecord_t *pFromStruct);
char *
sbZfKaEpVlanIndRecord_SPrint(sbZfKaEpVlanIndRecord_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpVlanIndRecord_Validate(sbZfKaEpVlanIndRecord_t *pZf);
int
sbZfKaEpVlanIndRecord_SetField(sbZfKaEpVlanIndRecord_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPVLANINDRECORD_CONSOLE_H */
