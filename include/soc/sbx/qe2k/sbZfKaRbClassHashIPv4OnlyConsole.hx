/*
 * $Id: sbZfKaRbClassHashIPv4OnlyConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaRbClassHashIPv4Only.hx"
#ifndef SB_ZF_ZFKARBCLASSHASHIPV4ONLY_CONSOLE_H
#define SB_ZF_ZFKARBCLASSHASHIPV4ONLY_CONSOLE_H



void
sbZfKaRbClassHashIPv4Only_Print(sbZfKaRbClassHashIPv4Only_t *pFromStruct);
char *
sbZfKaRbClassHashIPv4Only_SPrint(sbZfKaRbClassHashIPv4Only_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaRbClassHashIPv4Only_Validate(sbZfKaRbClassHashIPv4Only_t *pZf);
int
sbZfKaRbClassHashIPv4Only_SetField(sbZfKaRbClassHashIPv4Only_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKARBCLASSHASHIPV4ONLY_CONSOLE_H */
