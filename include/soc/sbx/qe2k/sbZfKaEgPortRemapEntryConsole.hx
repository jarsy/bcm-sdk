/*
 * $Id: sbZfKaEgPortRemapEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaEgPortRemapEntry.hx"
#ifndef SB_ZF_ZFKAEGPORTREMAPENTRY_CONSOLE_H
#define SB_ZF_ZFKAEGPORTREMAPENTRY_CONSOLE_H



void
sbZfKaEgPortRemapEntry_Print(sbZfKaEgPortRemapEntry_t *pFromStruct);
char *
sbZfKaEgPortRemapEntry_SPrint(sbZfKaEgPortRemapEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaEgPortRemapEntry_Validate(sbZfKaEgPortRemapEntry_t *pZf);
int
sbZfKaEgPortRemapEntry_SetField(sbZfKaEgPortRemapEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKAEGPORTREMAPENTRY_CONSOLE_H */
