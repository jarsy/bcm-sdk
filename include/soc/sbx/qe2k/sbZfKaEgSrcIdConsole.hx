/*
 * $Id: sbZfKaEgSrcIdConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEgSrcId.hx"
#ifndef SB_ZF_ZFKAEGSRCID_CONSOLE_H
#define SB_ZF_ZFKAEGSRCID_CONSOLE_H



void
sbZfKaEgSrcId_Print(sbZfKaEgSrcId_t *pFromStruct);
char *
sbZfKaEgSrcId_SPrint(sbZfKaEgSrcId_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEgSrcId_Validate(sbZfKaEgSrcId_t *pZf);
int
sbZfKaEgSrcId_SetField(sbZfKaEgSrcId_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEGSRCID_CONSOLE_H */
