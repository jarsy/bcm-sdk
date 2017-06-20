/*
 * $Id: sbZfKaEbMvtEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEbMvtEntry.hx"
#ifndef SB_ZF_ZFKAEBMVTENTRY_CONSOLE_H
#define SB_ZF_ZFKAEBMVTENTRY_CONSOLE_H



void
sbZfKaEbMvtEntry_Print(sbZfKaEbMvtEntry_t *pFromStruct);
char *
sbZfKaEbMvtEntry_SPrint(sbZfKaEbMvtEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEbMvtEntry_Validate(sbZfKaEbMvtEntry_t *pZf);
int
sbZfKaEbMvtEntry_SetField(sbZfKaEbMvtEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEBMVTENTRY_CONSOLE_H */
