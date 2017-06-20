/*
 * $Id: sbZfKaQsE2QAddrConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaQsE2QAddr.hx"
#ifndef SB_ZF_ZFKAQSE2QADDR_CONSOLE_H
#define SB_ZF_ZFKAQSE2QADDR_CONSOLE_H



void
sbZfKaQsE2QAddr_Print(sbZfKaQsE2QAddr_t *pFromStruct);
char *
sbZfKaQsE2QAddr_SPrint(sbZfKaQsE2QAddr_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaQsE2QAddr_Validate(sbZfKaQsE2QAddr_t *pZf);
int
sbZfKaQsE2QAddr_SetField(sbZfKaQsE2QAddr_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAQSE2QADDR_CONSOLE_H */
