/*
 * $Id: sbZfKaEgNotTmePortRemapAddrConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEgNotTmePortRemapAddr.hx"
#ifndef SB_ZF_ZFKAEGNOTTMEPORTREMAPADDR_CONSOLE_H
#define SB_ZF_ZFKAEGNOTTMEPORTREMAPADDR_CONSOLE_H



void
sbZfKaEgNotTmePortRemapAddr_Print(sbZfKaEgNotTmePortRemapAddr_t *pFromStruct);
char *
sbZfKaEgNotTmePortRemapAddr_SPrint(sbZfKaEgNotTmePortRemapAddr_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEgNotTmePortRemapAddr_Validate(sbZfKaEgNotTmePortRemapAddr_t *pZf);
int
sbZfKaEgNotTmePortRemapAddr_SetField(sbZfKaEgNotTmePortRemapAddr_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEGNOTTMEPORTREMAPADDR_CONSOLE_H */
