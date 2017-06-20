/*
 * $Id: sbZfKaRbPoliceCBSEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaRbPoliceCBSEntry.hx"
#ifndef SB_ZF_ZFKARBPOLCBSENTRY_CONSOLE_H
#define SB_ZF_ZFKARBPOLCBSENTRY_CONSOLE_H



void
sbZfKaRbPoliceCBSEntry_Print(sbZfKaRbPoliceCBSEntry_t *pFromStruct);
char *
sbZfKaRbPoliceCBSEntry_SPrint(sbZfKaRbPoliceCBSEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaRbPoliceCBSEntry_Validate(sbZfKaRbPoliceCBSEntry_t *pZf);
int
sbZfKaRbPoliceCBSEntry_SetField(sbZfKaRbPoliceCBSEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKARBPOLCBSENTRY_CONSOLE_H */
