/*
 * $Id: sbZfKaQsPriLutAddrConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsPriLutAddr.hx"
#ifndef SB_ZF_ZFKAQSPRILUTADDR_CONSOLE_H
#define SB_ZF_ZFKAQSPRILUTADDR_CONSOLE_H



void
sbZfKaQsPriLutAddr_Print(sbZfKaQsPriLutAddr_t *pFromStruct);
char *
sbZfKaQsPriLutAddr_SPrint(sbZfKaQsPriLutAddr_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsPriLutAddr_Validate(sbZfKaQsPriLutAddr_t *pZf);
int
sbZfKaQsPriLutAddr_SetField(sbZfKaQsPriLutAddr_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSPRILUTADDR_CONSOLE_H */
