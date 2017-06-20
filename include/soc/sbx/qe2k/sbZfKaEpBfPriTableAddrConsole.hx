/*
 * $Id: sbZfKaEpBfPriTableAddrConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEpBfPriTableAddr.hx"
#ifndef SB_ZF_ZFKAEPBFPRITABLEADDR_CONSOLE_H
#define SB_ZF_ZFKAEPBFPRITABLEADDR_CONSOLE_H



void
sbZfKaEpBfPriTableAddr_Print(sbZfKaEpBfPriTableAddr_t *pFromStruct);
char *
sbZfKaEpBfPriTableAddr_SPrint(sbZfKaEpBfPriTableAddr_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEpBfPriTableAddr_Validate(sbZfKaEpBfPriTableAddr_t *pZf);
int
sbZfKaEpBfPriTableAddr_SetField(sbZfKaEpBfPriTableAddr_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEPBFPRITABLEADDR_CONSOLE_H */
