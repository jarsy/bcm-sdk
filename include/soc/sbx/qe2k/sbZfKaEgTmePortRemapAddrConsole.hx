/*
 * $Id: sbZfKaEgTmePortRemapAddrConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEgTmePortRemapAddr.hx"
#ifndef SB_ZF_ZFKAEGTMEPORTREMAPADDR_CONSOLE_H
#define SB_ZF_ZFKAEGTMEPORTREMAPADDR_CONSOLE_H



void
sbZfKaEgTmePortRemapAddr_Print(sbZfKaEgTmePortRemapAddr_t *pFromStruct);
char *
sbZfKaEgTmePortRemapAddr_SPrint(sbZfKaEgTmePortRemapAddr_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEgTmePortRemapAddr_Validate(sbZfKaEgTmePortRemapAddr_t *pZf);
int
sbZfKaEgTmePortRemapAddr_SetField(sbZfKaEgTmePortRemapAddr_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEGTMEPORTREMAPADDR_CONSOLE_H */
