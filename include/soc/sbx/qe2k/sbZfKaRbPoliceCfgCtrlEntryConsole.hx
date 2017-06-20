/*
 * $Id: sbZfKaRbPoliceCfgCtrlEntryConsole.hx,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include "sbZfKaRbPoliceCfgCtrlEntry.hx"
#ifndef SB_ZF_ZFKARBPOLCFGCTRLENTRY_CONSOLE_H
#define SB_ZF_ZFKARBPOLCFGCTRLENTRY_CONSOLE_H



void
sbZfKaRbPoliceCfgCtrlEntry_Print(sbZfKaRbPoliceCfgCtrlEntry_t *pFromStruct);
char *
sbZfKaRbPoliceCfgCtrlEntry_SPrint(sbZfKaRbPoliceCfgCtrlEntry_t *pFromStruct, char *pcToString, uint32 lStrSize);
int
sbZfKaRbPoliceCfgCtrlEntry_Validate(sbZfKaRbPoliceCfgCtrlEntry_t *pZf);
int
sbZfKaRbPoliceCfgCtrlEntry_SetField(sbZfKaRbPoliceCfgCtrlEntry_t *s, char* name, int value);


#endif /* ifndef SB_ZF_ZFKARBPOLCFGCTRLENTRY_CONSOLE_H */
