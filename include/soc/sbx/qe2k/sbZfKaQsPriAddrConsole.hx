/*
 * $Id: sbZfKaQsPriAddrConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsPriAddr.hx"
#ifndef SB_ZF_ZFKAQSPRIADDR_CONSOLE_H
#define SB_ZF_ZFKAQSPRIADDR_CONSOLE_H



void
sbZfKaQsPriAddr_Print(sbZfKaQsPriAddr_t *pFromStruct);
char *
sbZfKaQsPriAddr_SPrint(sbZfKaQsPriAddr_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsPriAddr_Validate(sbZfKaQsPriAddr_t *pZf);
int
sbZfKaQsPriAddr_SetField(sbZfKaQsPriAddr_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSPRIADDR_CONSOLE_H */
